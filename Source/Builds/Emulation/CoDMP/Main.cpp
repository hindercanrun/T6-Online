#include "Std_Include.hpp"

#include "../../../Utils/Hook.hpp"

namespace Online
{
#if IS_XENIA
	Utils::Hook::Detour XUserCheckPrivilege_Hook;
	DWORD XUserCheckPrivilege(DWORD userIndex, DWORD privilege, PBOOL result)
	{
		// stubbed to always return success when checking for privileges

		if (result)
		{
			*result = TRUE;
		}
		return ERROR_SUCCESS;
	}
#elif
	// In case Xbox needs some things patched

#endif

	typedef struct _XPARTY_CUSTOM_DATA
	{
		ULONGLONG			qwFirst;
		ULONGLONG			qwSecond;
	} XPARTY_CUSTOM_DATA;

#define XPARTY_MAX_USERS	8
	typedef struct _XPARTY_USER_INFO
	{
		XUID				Xuid;
		CHAR				GamerTag[XUSER_NAME_SIZE];
		DWORD				dwUserIndex;
		XONLINE_NAT_TYPE	NatType;
		DWORD				dwTitleId;
		DWORD				dwFlags;
		XSESSION_INFO		SessionInfo;
		XPARTY_CUSTOM_DATA	CustomData;
	} XPARTY_USER_INFO;

	typedef struct _XPARTY_USER_LIST
	{
		DWORD dwUserCount;
		XPARTY_USER_INFO Users[XPARTY_MAX_USERS];
	} XPARTY_USER_LIST;

	Utils::Hook::Detour XPartyGetUserList_Hook;
	HRESULT XPartyGetUserList(XPARTY_USER_LIST* pUserList)
	{
		if (pUserList)
		{
			pUserList->dwUserCount = 1;
		}
		return S_OK;
	}

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

	Utils::Hook::Detour Live_IsUserSignedInToLive_Hook;
	BOOL Live_IsUserSignedInToLive(ControllerIndex controllerIndex)
	{
		return BD_NOT_CONNECTED;
	}

#define XENON_STATUS_CONNECTED	5
	Utils::Hook::Detour Live_Base_IsConnected_Hook;
	BOOL Live_Base_IsConnected(ControllerIndex controllerIndex)
	{
		return XENON_STATUS_CONNECTED;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveAllStats_Hook;
	bool LiveStorage_DoWeHaveAllStats(ControllerIndex controllerIndex)
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

	Utils::Hook::Detour Live_IsLspCacheInited_Hook;
	bool Live_IsLspCacheInited()
	{
		return TRUE;
	}

	Utils::Hook::Detour SanityCheckSession_Hook;
	void SanityCheckSession(const char** unknown)
	{
		return;
	}

	Utils::Hook::Detour LiveStats_CanPerformStatOperation_Hook;
	bool LiveStats_CanPerformStatOperation(ControllerIndex Controllerindex)
	{
		return TRUE;
	}

	void RegisterHooks()
	{

#if IS_XENIA
		XUserCheckPrivilege_Hook.Create(0x829F2550, XUserCheckPrivilege);
#endif
		XPartyGetUserList_Hook.Create(0x829F0608, XPartyGetUserList);
		Live_IsUserSignedInToDemonware_Hook.Create(0x827AC038, Live_IsUserSignedInToDemonware);
		Live_IsUserSignedInToLive_Hook.Create(0x827B0A08, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_Hook.Create(0x827FA9E0, Live_Base_IsConnected);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x827EAF10, LiveStorage_DoWeHaveAllStats);
		Live_HasMultiplayerPrivileges_Hook.Create(0x827FBCA0, Live_HasMultiplayerPrivileges);
		LiveStorage_DoWeHaveLeagues_Hook.Create(0x827F4488, LiveStorage_DoWeHaveLeagues);
		LiveStorage_IsTimeSynced_Hook.Create(0x827ECBD0, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x827F4468, LiveStorage_DoWeHaveContracts);
		LiveElite_CheckProgress_Hook.Create(0x827B9B60, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_Hook.Create(0x8250E8F0, LiveCAC_CheckProgress);
		Live_IsLspCacheInited_Hook.Create(0x827FF3A8, Live_IsLspCacheInited);
		SanityCheckSession_Hook.Create(0x827D3CA0, SanityCheckSession);
		LiveStats_CanPerformStatOperation_Hook.Create(0x827D9AC8, LiveStats_CanPerformStatOperation);

		Utils::Hook::SetValue<uint8_t>(0x8469B230, 1); // s_geoLocationRetrieved
	}

	void UnregisterHooks()
	{
#if IS_XENIA
		XUserCheckPrivilege_Hook.Clear();
#endif
		XPartyGetUserList_Hook.Clear();
		Live_IsUserSignedInToDemonware_Hook.Clear();
		Live_IsUserSignedInToLive_Hook.Clear();
		Live_Base_IsConnected_Hook.Clear();
		LiveStorage_DoWeHaveAllStats_Hook.Clear();
		Live_HasMultiplayerPrivileges_Hook.Clear();
		LiveStorage_DoWeHaveLeagues_Hook.Clear();
		LiveStorage_IsTimeSynced_Hook.Clear();
		LiveStorage_DoWeHaveContracts_Hook.Clear();
		LiveElite_CheckProgress_Hook.Clear();
		LiveCAC_CheckProgress_Hook.Clear();
		Live_IsLspCacheInited_Hook.Clear();
		SanityCheckSession_Hook.Clear();
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