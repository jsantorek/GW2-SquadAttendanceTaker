#pragma once
#include <imgui_internal.h>

namespace ImGui
{
// exposed in imgui api in 1.84
	void BeginDisabled()
	{
        ImGuiContext& g = *GImGui;
		if ((g.ItemFlagsStack.back() & ImGuiItemFlags_Disabled) == 0)
			PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.6f);
		PushItemFlag(ImGuiItemFlags_Disabled, true);
	}

	void EndDisabled()
	{
        ImGuiContext& g = *GImGui;
		PopItemFlag();
		if ((g.ItemFlagsStack.back() & ImGuiItemFlags_Disabled) == 0)
			PopStyleVar();
	}

// std::string handling in InputText
	struct InputTextCallback_UserData
	{
		std::string*            Str;
		ImGuiInputTextCallback  ChainCallback;
		void*                   ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		else if (user_data->ChainCallback)
		{
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		return 0;
	}

	bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}
}
