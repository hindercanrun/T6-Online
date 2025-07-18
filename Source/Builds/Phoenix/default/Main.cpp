#include "Std_Include.hpp"

#include "../../../Utils/Hook.hpp"

namespace Online
{
	typedef enum ControllerIndex_t
	{
		INVALID_CONTROLLER_PORT = -1,

		CONTROLLER_INDEX_FIRST = 0,
		CONTROLLER_INDEX_0 = 0,

		CONTROLLER_INDEX_COUNT = 1,
	} ControllerIndex;

#define BD_NOT_CONNECTED	2
	Utils::Hook::Detour Live_IsUserSignedInToDemonware_Hook;
	BOOL Live_IsUserSignedInToDemonware(ControllerIndex controllerIndex)
	{
		return BD_NOT_CONNECTED;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveFFOTD_Hook;
	bool LiveStorage_DoWeHaveFFOTD()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_ValidateFFOTD_Hook;
	bool LiveStorage_ValidateFFOTD()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveAllStats_Hook;
	bool LiveStorage_DoWeHaveAllStats(ControllerIndex controllerIndex)
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHavePlaylists_Hook;
	bool LiveStorage_DoWeHavePlaylists()
	{
		return TRUE;
	}

	Utils::Hook::Detour Live_HasMultiplayerPrivileges_Hook;
	bool Live_HasMultiplayerPrivileges(ControllerIndex controllerIndex)
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveLeagues_Hook;
	bool LiveStorage_DoWeHaveLeagues()
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

	typedef enum eliteStatus_e
	{
		ELITE_NOT_CHECKED							= 0,

		ELITESUBSCRIBER_START						= 1,
		ELITESUBSCRIBER_GET_INFO_FROM_FP			= 2,
		ELITESUBSCRIBER_GET_INFO_FROM_FP_SUCCEEDED	= 3,
		ELITESUBSCRIBER_GET_INFO_FROM_DW			= 4,
		ELITESUBSCRIBER_GETTING_INFO_FROM_DW		= 5,
		ELITESUBSCRIBER_COMPARE_RETRIEVED_DW_INFO	= 6,
		ELITESUBSCRIBER_REFRESH_DW_INFO				= 7,
		ELITESUBSCRIBER_REFRESHING_DW_INFO			= 8,
		ELITESUBSCRIBER_FAILED						= 9,
		ELITESUBSCRIBER_SUCCEEDED					= 10,

		ELITECLAN_START								= 11,
		ELITECLAN_GET_CLAN							= 12,
		ELITECLAN_GETTING_CLAN						= 13,
		ELITECLAN_GET_PUBLIC_PROFILE				= 14,
		ELITECLAN_GETTING_PUBLIC_PROFILE			= 15,
		ELITECLAN_GET_PRIVATE_PROFILE				= 16,
		ELITECLAN_GETTING_PRIVATE_PROFILE			= 17,
		ELITECLAN_GET_CLAN_MEMBERS					= 18,
		ELITECLAN_GETTING_CLAN_MEMBERS				= 19,
		ELITECLAN_GET_CLAN_LEVELANDXP				= 20,
		ELITECLAN_GETTING_CLAN_LEVELANDXP			= 21,
		ELITECLAN_FAILED							= 22,
		ELITECLAN_SUCCEEDED							= 23,

		ELITE_CHECKED								= 24,

		ELITE_MAX_STATES							= 25,
	} eliteStatus;

	Utils::Hook::Detour LiveElite_CheckProgress_Hook;
	bool LiveElite_CheckProgress(ControllerIndex localControllerIndex, eliteStatus status)
	{
		return TRUE;
	}

	// This enum is entirely guessed.
	typedef enum cacStatus_e
	{
		CAC_NOT_CHECKED		= 0,
		CAC_CHECKED			= 1,

		CAC_MAX_STATES		= 2,
	} cacStatus;

	Utils::Hook::Detour LiveCAC_CheckProgress_Hook;
	bool LiveCAC_CheckProgress(ControllerIndex localControllerIndex, cacStatus status)
	{
		return TRUE;
	}

	void RegisterHooks()
	{
		Live_IsUserSignedInToDemonware_Hook.Create(0x824DF1D8, Live_IsUserSignedInToDemonware);
		LiveStorage_DoWeHaveFFOTD_Hook.Create(0x82514B68, LiveStorage_DoWeHaveFFOTD);
		LiveStorage_ValidateFFOTD_Hook.Create(0x82514B98, LiveStorage_ValidateFFOTD);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x8250C1C0, LiveStorage_DoWeHaveAllStats);
		LiveStorage_DoWeHavePlaylists_Hook.Create(0x825142D8, LiveStorage_DoWeHavePlaylists);
		LiveStorage_DoWeHaveLeagues_Hook.Create(0x82514308, LiveStorage_DoWeHaveLeagues);
		LiveStorage_IsTimeSynced_Hook.Create(0x8250D218, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x825142E8, LiveStorage_DoWeHaveContracts);
		Utils::Hook::SetValue<uint8_t>(0x8424C6F0, 1); // s_geoLocationRetrieved
		LiveElite_CheckProgress_Hook.Create(0x824E78B0, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_Hook.Create(0x82352138, LiveCAC_CheckProgress);
	}

	void UnregisterHooks()
	{
		Live_IsUserSignedInToDemonware_Hook.Clear();
		LiveStorage_DoWeHaveFFOTD_Hook.Clear();
		LiveStorage_ValidateFFOTD_Hook.Clear();
		LiveStorage_DoWeHaveAllStats_Hook.Clear();
		LiveStorage_DoWeHavePlaylists_Hook.Clear();
		LiveStorage_DoWeHaveLeagues_Hook.Clear();
		LiveStorage_IsTimeSynced_Hook.Clear();
		LiveStorage_DoWeHaveContracts_Hook.Clear();
		LiveElite_CheckProgress_Hook.Clear();
		LiveCAC_CheckProgress_Hook.Clear();
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