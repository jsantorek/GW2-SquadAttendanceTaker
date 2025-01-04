#pragma once
#include "SquadMember.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace SAT
{
class AttendanceWatcher
{
  public:
    AttendanceWatcher();
    ~AttendanceWatcher();
    void Start();
    void Stop();
    void FlushToClipboard();
    void UpdateSquadMembers()
    {
        UpdateSquadMembersAttendance();
        UpdateSquadMembersData();
    }
    void UpdateSquadMembersAttendance();
    void UpdateSquadMembersData();
    bool IsRecording()
    {
        return _isRecording;
    }

  private:
    std::unordered_map<SquadMember::UUID, SquadMember> members;
    std::string SerializeObservedSquadMembers();
    void RunAutomatedUpdates();
    std::condition_variable _condvar;
    std::mutex _mutex;
    std::thread _worker;
    std::atomic<bool> _isRecording;
};
} // namespace SAT
