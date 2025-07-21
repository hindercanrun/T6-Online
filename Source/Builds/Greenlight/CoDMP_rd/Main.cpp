#include "Std_Include.hpp"

#include "../../../Utils/Hook.hpp"

namespace Online
{
	Utils::Hook::Detour Live_IsUserSignedInToDemonware_Hook;
	BOOL Live_IsUserSignedInToDemonware(int controllerIndex)
	{
		return 2;
	}

	Utils::Hook::Detour Live_IsUserSignedInToLive_Hook;
	BOOL Live_IsUserSignedInToLive(int controllerIndex)
	{
		return 2;
	}

	Utils::Hook::Detour Live_Base_IsConnected_Hook;
	BOOL Live_Base_IsConnected(int controllerIndex)
	{
		return 5;
	}

	Utils::Hook::Detour LiveStorage_ValidateFFOTD_Hook;
	bool LiveStorage_ValidateFFOTD()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHavePlaylists_Hook;
	bool LiveStorage_DoWeHavePlaylists()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveAllStats_Hook;
	bool LiveStorage_DoWeHaveAllStats(int controllerIndex)
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_IsTimeSynced_Hook;
	bool LiveStorage_IsTimeSynced()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveContracts_Hook;
	bool LiveStorage_DoWeHaveContracts()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveCurrentStats_Hook;
	bool LiveStorage_DoWeHaveCurrentStats(int controllerIndex)
	{
		static bool hasBeenCalled = FALSE;
		if (!hasBeenCalled)
		{
			hasBeenCalled = TRUE;
			return FALSE;
		}
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveStats_Hook;
	bool LiveStorage_DoWeHaveStats(int controllerIndex, int playerStatsLocation)
	{
		return TRUE;
	}

	void RegisterHooks()
	{
		Live_IsUserSignedInToLive_Hook.Create(0x82A03A90, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_Hook.Create(0x82A6E990, Live_Base_IsConnected);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x82A54C80, LiveStorage_DoWeHaveAllStats);
		LiveStorage_DoWeHavePlaylists_Hook.Create(0x82A61A48, LiveStorage_DoWeHavePlaylists);
		LiveStorage_ValidateFFOTD_Hook.Create(0x82A62900, LiveStorage_ValidateFFOTD);
		LiveStorage_IsTimeSynced_Hook.Create(0x82A57498, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x82A61A58, LiveStorage_DoWeHaveContracts);
		LiveStorage_DoWeHaveCurrentStats_Hook.Create(0x82A54C40, LiveStorage_DoWeHaveCurrentStats);
		LiveStorage_DoWeHaveStats_Hook.Create(0x82A54C00, LiveStorage_DoWeHaveStats);
	}

	void UnregisterHooks()
	{
		Live_IsUserSignedInToLive_Hook.Clear();
		Live_Base_IsConnected_Hook.Clear();
		LiveStorage_DoWeHaveAllStats_Hook.Clear();
		LiveStorage_IsTimeSynced_Hook.Clear();
		LiveStorage_DoWeHaveContracts_Hook.Clear();
	}
}

DWORD WINAPI MainThread(LPVOID)
{
	while (TRUE)
	{
		DWORD titleId = XamGetCurrentTitleId();
		if (titleId == TITLE_ID)
		{
			Online::RegisterHooks(); // Init our hooks.
		}
	}
	return FALSE;
}

BOOL APIENTRY DllMain(
	HANDLE handle,
	DWORD reason,
	LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE hThread = CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
		if (hThread)
		{
			CloseHandle(hThread);
		}
		break;
	}
	case DLL_PROCESS_DETACH:
		// This case is useless on Xenia because it doesn't have a dashboard system.
		// So, code here will only run on Xbox.
		Online::UnregisterHooks(); // Remove our hooks.
		break;
	}
	return TRUE;
}