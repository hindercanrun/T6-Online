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

	Utils::Hook::Detour Live_IsLspCacheInited_Hook;
	bool Live_IsLspCacheInited()
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStats_CanPerformStatOperation_Hook;
	int LiveStats_CanPerformStatOperation(int a1)
	{
		return 1;
	}

	void RegisterHooks()
	{
		Live_IsUserSignedInToDemonware_Hook.Create(0x82507838, Live_IsUserSignedInToDemonware);
		LiveStorage_DoWeHaveFFOTD_Hook.Create(0x8253B1C8, LiveStorage_DoWeHaveFFOTD);
		LiveStorage_ValidateFFOTD_Hook.Create(0x8253B1F8, LiveStorage_ValidateFFOTD);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x82533CF0, LiveStorage_DoWeHaveAllStats);
		LiveStorage_DoWeHavePlaylists_Hook.Create(0x8253AAD8, LiveStorage_DoWeHavePlaylists);
		LiveStorage_DoWeHaveLeagues_Hook.Create(0x8253AB08, LiveStorage_DoWeHaveLeagues);
		LiveStorage_IsTimeSynced_Hook.Create(0x82534C40, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x8253AAE8, LiveStorage_DoWeHaveContracts);
		Utils::Hook::SetValue<uint8_t>(0x83FB9FA8, 1); // s_geoLocationRetrieved
		LiveElite_CheckProgress_Hook.Create(0x82510858, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_Hook.Create(0x8235FEC0, LiveCAC_CheckProgress);
		Live_IsLspCacheInited_Hook.Create(0x82542D40, Live_IsLspCacheInited);
		//LiveStats_CanPerformStatOperation_Hook.Create(0x827D9AC8, LiveStats_CanPerformStatOperation);
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
		Live_IsLspCacheInited_Hook.Clear();
		LiveStats_CanPerformStatOperation_Hook.Clear();
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