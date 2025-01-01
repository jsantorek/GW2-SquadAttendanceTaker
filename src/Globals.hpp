#pragma once

#include <windows.h>
#include <Nexus.h>
#include <Hooks.hpp>
#include "Options.hpp"
#include "AttendanceWatcher.hpp"

#define ADDON_QUICK_ACCESS "SquadAttendanceTaker_QA"
#define ADDON_INPUT_BIND "SquadAttendanceTaker_IB"
#define ADDON_ICON "SquadAttendanceTaker_Icon"
#define ADDON_ICON_HOVER "SquadAttendanceTaker_IconHover"

namespace G
{
	extern HMODULE Module;
	extern AddonAPI* APIDefs;
	extern SAT::Options Options;
	extern SAT::Hooks Hooks;
	extern SAT::AttendanceWatcher Watcher;
}
