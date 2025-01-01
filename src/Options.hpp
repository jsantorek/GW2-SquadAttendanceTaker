#pragma once
#include <string>
#include <filesystem>

namespace SAT
{
    struct Options
    {
        enum AttendenceMonitoringMode : int
        {
            SingleSnapshot,
            ContinuousRecording
        };

        std::string ColumnSeparator = ", ";
        std::string RowSeparator = "\r\n";
        AttendenceMonitoringMode MonitoringMode = AttendenceMonitoringMode::SingleSnapshot;
        int PeriodBetweenPresenceChecks = 20;
        bool TabularizeAccountUUIDs = false;
        bool TabularizeAccountNames = true;
        bool TabularizeCharacterNames = true;
        
        void Persist(std::filesystem::path file) const;
        void Parse(std::filesystem::path file);
    };
}
