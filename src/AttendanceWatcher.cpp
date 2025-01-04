#include "AttendanceWatcher.hpp"
#include "Exceptions.hpp"
#include "Globals.hpp"
#include "Hooks.hpp"
#include <rpc.h>

SAT::AttendanceWatcher::AttendanceWatcher() = default;

SAT::AttendanceWatcher::~AttendanceWatcher()
{
    _isRecording = false;
    _condvar.notify_all();
}

void SAT::AttendanceWatcher::RunAutomatedUpdates()
{
    auto lock = std::unique_lock(_mutex);
    const auto period = std::chrono::seconds{G::Options.PeriodBetweenPresenceChecks};
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "RunAutomatedUpdates::started");
    while (true)
    {
        const auto timenow = std::chrono::system_clock::now();
        G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "RunAutomatedUpdates::waiting");
        _condvar.wait_for(lock, period);
        if (!_isRecording)
        {
            break;
        }
        UpdateSquadMembers();
    }
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "RunAutomatedUpdates::stopping");
}

void SAT::AttendanceWatcher::Start()
{
    auto lock = std::lock_guard(_mutex);
    if (_isRecording)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME,
                        "Internal state error - attempting to ::Start() a watcher which IsRecording");
    }
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Worker thread initialization");
    G::APIDefs->QuickAccess.Notify(ADDON_QUICK_ACCESS);
    _isRecording = true;
    _worker = std::thread(&SAT::AttendanceWatcher::RunAutomatedUpdates, this);
    _worker.detach();
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Worker thread detached");
}

void SAT::AttendanceWatcher::Stop()
{
    auto lock = std::lock_guard(_mutex);
    if (!_isRecording)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME,
                        "Internal state error - attempting to ::Stop() a watcher which Is(not)Recording");
    }
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Issuing worker thread termination request");
    _isRecording = false;
    _condvar.notify_all();
}

void SAT::AttendanceWatcher::FlushToClipboard()
{
    const auto message = SerializeObservedSquadMembers();
    members.clear();
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "String representation reallocated to global");
    const char *output = message.c_str();
    const size_t len = strlen(output) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), output, len);
    GlobalUnlock(hMem);
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Setting clipboard contents");
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    G::APIDefs->UI.SendAlert("Squad attendance copied to clipboard");
}

inline bool IsMissingRelevantData(const SAT::SquadMember &member)
{
    return (G::Options.TabularizeAccountNames && member.AccountName.empty()) ||
           (G::Options.TabularizeCharacterNames && member.CharacterName.empty());
}

void SAT::AttendanceWatcher::UpdateSquadMembersData()
{
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Getting current squad members");
    try
    {
        for (auto &[uuid, member] : members)
        {
            if (IsMissingRelevantData(member))
            {
                try
                {
                    member = G::Hooks.GetDetailedInfo(uuid);
                }
                catch (const SAT::UserDataUnavailable &)
                {
                    std::stringstream oss;
                    oss << "Failed to GetDetailedInfo for user UUID " << uuid;
                    G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, oss.str().c_str());
                }
            }
        }
    }
    catch (const SAT::HookingFailure &)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, "Faulty portal hooks were not used");
    }
    catch (const std::exception &e)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, "UpdateSquadMembersData encountered unidentifed exception");
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, e.what());
    }
}

void SAT::AttendanceWatcher::UpdateSquadMembersAttendance()
{
    G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME, "Getting current squad members");
    try
    {
        auto currentUUIDs = G::Hooks.GetSquadMembersUUIDs();
        G::APIDefs->Log(ELogLevel_TRACE, ADDON_NAME,
                        std::to_string(currentUUIDs.size()).append(" squad member(s) acquired").c_str());
        for (const auto &uuid : currentUUIDs)
        {
            if (members.find(uuid) == members.end())
            {
                members.emplace(uuid, SAT::SquadMember{});
            }
        }
    }
    catch (const SAT::HookingFailure &)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, "Faulty squad memory hooks were not used");
    }
    catch (const SAT::NotInSquad &)
    {
        G::APIDefs->UI.SendAlert("You do not seem to be in squad");
        G::APIDefs->Log(ELogLevel_INFO, ADDON_NAME, "User does not seem to be in squad");
    }
    catch (const std::exception &e)
    {
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME,
                        "UpdateSquadMembersAttendance encountered unidentifed exception");
        G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, e.what());
    }
}

// TODO: clean this up - this is way too nasty, probably ostream wrapper will do
std::string SAT::AttendanceWatcher::SerializeObservedSquadMembers()
{
    std::ostringstream clipboardMsg;
    for (auto &[uuid, member] : members)
    {
        if (G::Options.TabularizeAccountUUIDs)
        {
            clipboardMsg << uuid << G::Options.ColumnSeparator;
        }
        if (G::Options.TabularizeAccountNames)
        {
            clipboardMsg << member.AccountName << G::Options.ColumnSeparator;
        }
        if (G::Options.TabularizeCharacterNames)
        {
            clipboardMsg << member.CharacterName;
        }
        clipboardMsg << G::Options.RowSeparator;
    }
    return clipboardMsg.str();
}
