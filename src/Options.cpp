#include "Options.hpp"
#include "Globals.hpp"
#include <fstream>
#include <nlohmann/json.hpp>


NLOHMANN_JSON_SERIALIZE_ENUM(SAT::Options::AttendenceMonitoringMode,
{
    { SAT::Options::AttendenceMonitoringMode::SingleSnapshot, "SingleSnapshot" },
    { SAT::Options::AttendenceMonitoringMode::ContinuousRecording, "ContinuousRecording" }
})

//TODO: more granular handling of exceptions: filesystem and nlohmann::json
void SAT::Options::Persist(std::filesystem::path file) const
{
	try
	{
        auto json = nlohmann::json();
        json["ColumnSeparator"] = ColumnSeparator;
        json["RowSeparator"] = RowSeparator;
        json["AttendenceMonitoringMode"] = MonitoringMode;
        json["PeriodBetweenPresenceChecks"] = PeriodBetweenPresenceChecks;
        json["TabularizeAccountUUIDs"] = TabularizeAccountUUIDs;
        json["TabularizeAccountNames"] = TabularizeAccountNames;
        json["TabularizeCharacterNames"] = TabularizeCharacterNames;
        if(!std::filesystem::exists(file.parent_path()))
        {
            std::filesystem::create_directories(file.parent_path());
        }
        std::ofstream(file) << json;
	}
	catch(const std::exception& e)
	{
		G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, e.what());
		G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, "Unexpected error when persisting options");
	}
}

void SAT::Options::Parse(std::filesystem::path file)
{
    try
	{
        auto json = nlohmann::json::object();
        if(std::filesystem::exists(file))
        {
            json = nlohmann::json::parse(std::ifstream(file), nullptr, false);
        }
        ColumnSeparator = json.value("ColumnSeparator", ColumnSeparator);
        RowSeparator = json.value("RowSeparator", RowSeparator);
        MonitoringMode = json.value("AttendenceMonitoringMode", MonitoringMode);
        PeriodBetweenPresenceChecks = json.value("PeriodBetweenPresenceChecks", PeriodBetweenPresenceChecks);
        TabularizeAccountUUIDs = json.value("TabularizeAccountUUIDs", TabularizeAccountUUIDs);
        TabularizeAccountNames = json.value("TabularizeAccountNames", TabularizeAccountNames);
        TabularizeCharacterNames = json.value("TabularizeCharacterNames", TabularizeCharacterNames);
	}
	catch(const std::exception& e)
	{
		G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, e.what());
		G::APIDefs->Log(ELogLevel_WARNING, ADDON_NAME, "Unexpected error when parsing options, falling back to defaults");
	}
}
