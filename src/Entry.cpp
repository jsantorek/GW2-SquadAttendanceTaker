#include <windows.h>

#include <vector>
#include <filesystem>
#include <sstream>
#include <Nexus.h>
#include <imgui.h>
#include "imgui_extensions.h"

#include "Globals.hpp"
#include "Hooks.hpp"
#include "Resources.h"

BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void AddonLoad(AddonAPI* aApi);
void AddonUnload();
void OptionsRender();

extern "C" __declspec(dllexport) AddonDefinition* GetAddonDef()
{
	static AddonDefinition def{
		.Signature = 0x776E7A30,
		.APIVersion = NEXUS_API_VERSION,
		.Name = "Squad Attendance Taker",
		.Version = {
			ADDON_VERSION_MAJOR,
			ADDON_VERSION_MINOR,
			ADDON_VERSION_PATCH,
			ADDON_VERSION_REVISION
		},
		.Author = "Vonsh.1427",
		.Description = "Get formatted list of squad members copied to clipboard on demand",
		.Load = AddonLoad,
		.Unload = AddonUnload,
		.Flags = EAddonFlags_IsVolatile
	};
	return &def;
}

void InputBindInvocationHandler(const char* aIdentifier, bool aIsRelease)
{
	if(aIsRelease) return;
	if(G::Options.MonitoringMode == SAT::Options::AttendenceMonitoringMode::ContinuousRecording)
	{
		if(!G::Watcher.IsRecording())
		{
			G::Watcher.UpdateSquadMembers();
			G::Watcher.Start();
			return;
		}
		G::Watcher.Stop();
	}
	G::Watcher.UpdateSquadMembers();
	G::Watcher.FlushToClipboard();
}

void AddonLoad(AddonAPI* aApi)
{
	G::APIDefs = aApi;
	const auto addonDir = std::filesystem::path(G::APIDefs->Paths.GetAddonDirectory(ADDON_NAME));
	G::Options.Parse(addonDir / "config.json");
	ImGui::SetCurrentContext((ImGuiContext*)G::APIDefs->ImguiContext);
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))G::APIDefs->ImguiMalloc, (void(*)(void*, void*))G::APIDefs->ImguiFree);

	G::APIDefs->Renderer.Register(ERenderType_OptionsRender, OptionsRender);
	G::APIDefs->InputBinds.RegisterWithStruct(ADDON_INPUT_BIND, InputBindInvocationHandler, {});
	G::APIDefs->Textures.GetOrCreateFromMemory(ADDON_ICON, &icon_png, icon_png_len);
	G::APIDefs->Localization.Set(ADDON_INPUT_BIND, "en", "Take Squad Attendance");
	// Thanks google translate for these!
	G::APIDefs->Localization.Set(ADDON_INPUT_BIND, "de", "Squad-Teilnahme nehmen");
	G::APIDefs->Localization.Set(ADDON_INPUT_BIND, "es", "Toma la asistencia del escuadrón");
	G::APIDefs->Localization.Set(ADDON_INPUT_BIND, "fr", "Prise de présence de l'escouade");
	G::APIDefs->QuickAccess.Add(ADDON_QUICK_ACCESS, ADDON_ICON, ADDON_ICON, ADDON_INPUT_BIND, ADDON_INPUT_BIND);
	G::Hooks.Install();
	G::APIDefs->Log(ELogLevel_INFO, ADDON_NAME, "Initialized.");
}
void AddonUnload()
{
	const auto addonDir = std::filesystem::path(G::APIDefs->Paths.GetAddonDirectory(ADDON_NAME));
	G::Options.Persist(addonDir / "config.json");
	G::Hooks.Uninstall();
	G::APIDefs->Renderer.Deregister(OptionsRender);
	G::APIDefs->InputBinds.Deregister(ADDON_INPUT_BIND);
	G::APIDefs->QuickAccess.Remove(ADDON_QUICK_ACCESS);
}

void OptionsRender()
{
	// TODO: all these should be using translation service
	ImGui::Separator();
	if(G::Watcher.IsRecording())
	{
		ImGui::Text("Configuration disabled while attendance is being taken.");
        ImGui::BeginDisabled();
	}
	if (ImGui::TreeNode("Recording input"))
    {
		ImGui::Text("Attendence taking mode");
		ImGui::RadioButton("Single snapshot", (int*)&G::Options.MonitoringMode, SAT::Options::AttendenceMonitoringMode::SingleSnapshot);
		ImGui::SameLine();
		ImGui::RadioButton("Continuous recording", (int*)&G::Options.MonitoringMode, SAT::Options::AttendenceMonitoringMode::ContinuousRecording);
		if(G::Options.MonitoringMode == SAT::Options::AttendenceMonitoringMode::ContinuousRecording)
		{            
			ImGui::DragInt("Period between presence checks", &G::Options.PeriodBetweenPresenceChecks, 1, 1, 120, "%d seconds", ImGuiSliderFlags_AlwaysClamp);
		}
		ImGui::TreePop();
    }
	if (ImGui::TreeNode("Formatting output"))
    {
		ImGui::Text("Table columns (Ctrl+Click to select multiple)");
		ImGui::Checkbox("Account UUID", &G::Options.TabularizeAccountUUIDs);
		ImGui::Checkbox("Account name", &G::Options.TabularizeAccountNames);
		ImGui::Checkbox("Character name", &G::Options.TabularizeCharacterNames);

		ImGui::InputText("Column separator", &G::Options.ColumnSeparator);
		static const auto separators = std::unordered_map<std::string, std::string>{
			{"\r\n", "Windows-like"},
			{"\n", "Linux-like"}
		};
		const auto is_custom = separators.find(G::Options.RowSeparator) == separators.end();
		if (ImGui::BeginCombo("Row separator", is_custom ? "Custom" : separators.at(G::Options.RowSeparator).c_str()))
		{
			for(const auto &[sep, descr]: separators)
			{
				const auto is_selected = G::Options.RowSeparator == sep;
				if (ImGui::Selectable(descr.c_str(), is_selected))
					G::Options.RowSeparator = sep;
				if (is_selected)
					ImGui::SetItemDefaultFocus(); 	
			}
			if(ImGui::Selectable("Custom", is_custom))
				G::Options.RowSeparator = "";
			if (is_custom)
				ImGui::SetItemDefaultFocus(); 
			ImGui::EndCombo();
		}
		if (is_custom)
		{
			ImGui::InputText("Custom separator", &G::Options.RowSeparator);
		}
		ImGui::TreePop();
    }

	if(G::Watcher.IsRecording())
	{
        ImGui::EndDisabled();
	}
}
