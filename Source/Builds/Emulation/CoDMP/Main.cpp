#include "Std_Include.hpp"

#include "../../../Utils/Hook.hpp"

#include <stdio.h>

namespace Online
{
	typedef void (*Com_Printf_t)(int channel, const char* fmt, ...);
	Com_Printf_t Com_Printf = Com_Printf_t(0x825644E0);

	typedef enum _bdLogMessageType
	{
		BD_LOG_INFO = 0,
		BD_LOG_WARNING = 1,
		BD_LOG_ERROR = 2,
	} bdLogMessageType;

	Utils::Hook::Detour bdLogMessage_Hook;
	void bdLogMessage(
		bdLogMessageType type,
		const char* baseChannel,
		const char* channel,
		const char* file,
		const char* function,
		int line,
		const char* fmt,
		...)
	{
		Com_Printf(0, "\n");
		Com_Printf(0, "DW: file: %s\n", file);
		Com_Printf(0, "DW: function: %s\n", function);
		Com_Printf(0, "DW: line: %d\n", line);
		Com_Printf(0, "DW: channel: %s : %s\n", channel, baseChannel);

		char buffer[1024];

		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		Com_Printf(0, "DW: message: %s\n", buffer);
	}

	Utils::Hook::Detour bdLobbyService__getStatus_Hook;
	int __fastcall bdLobbyService__getStatus(int a1)
	{
		// If there's a connection object at +0xA0, report BD_CONNECTED
		if (*(int*)(a1 + 160))
			return 2; // BD_CONNECTED

		return 0; // BD_NOT_CONNECTED
	}

#if IS_XENIA

	#pragma pack(push, 1)
	struct FakeLSPEntry
	{
		DWORD ipOrId;
		char name[200];
		char unused[4];
	};
	#pragma pack(pop)

	Utils::Hook::Detour XEnumerate_Hook;
	DWORD XEnumerate(HANDLE hEnum, PVOID pvBuffer, DWORD cbBuffer, PDWORD pcItemsReturned, PXOVERLAPPED pOverlapped)
	{
		if (pvBuffer && cbBuffer >= sizeof(FakeLSPEntry))
		{
			FakeLSPEntry* entry = (FakeLSPEntry*)pvBuffer;

			entry->ipOrId = inet_addr("185.34.106.11");
			strcpy_s(entry->name, sizeof(entry->name), "OPS2OPS2 - DW ATVI LSP");

			if (pcItemsReturned)
			{
				*pcItemsReturned = 1;
			}
		}
		else if (pcItemsReturned)
		{
			*pcItemsReturned = 0;
		}

		if (pOverlapped)
		{
			pOverlapped->InternalLow = ERROR_NO_MORE_FILES;
			pOverlapped->InternalHigh = 1;

			// signal the async completion event
			if (pOverlapped->hEvent)
			{
				SetEvent(pOverlapped->hEvent);
			}
			return ERROR_IO_PENDING;
		}
		return ERROR_NO_MORE_FILES;
	}

	Utils::Hook::Detour XGetOverlappedResult_Hook;
	DWORD XGetOverlappedResult(PXOVERLAPPED pOverlapped, LPDWORD pdwResult, BOOL bWait)
	{
		if (!pOverlapped)
		{
			return FALSE;
		}

		if (pdwResult)
		{
			*pdwResult = static_cast<DWORD>(pOverlapped->InternalHigh);
		}

		DWORD res = static_cast<DWORD>(pOverlapped->InternalLow);
		if (res == ERROR_IO_INCOMPLETE)
		{
			if (bWait)
			{
				pOverlapped->InternalLow = ERROR_SUCCESS;
				pOverlapped->InternalHigh = 1;

				if (pdwResult)
				{
					*pdwResult = 1;
				}
				return TRUE;
			}

			SetLastError(ERROR_IO_INCOMPLETE);
			return FALSE;
		}

		if (res != ERROR_SUCCESS)
		{
			SetLastError(res);
		}
		return (res == ERROR_SUCCESS);
	}

	Utils::Hook::Detour XNetServerToInAddr_Hook;
	INT XNetServerToInAddr(const IN_ADDR ina, DWORD dwServiceId, IN_ADDR *pina)
	{
		if (pina)
		{
			*(unsigned int *)pina = 0xC0A80001; // 192.168.0.1
		}
		return ERROR_SUCCESS;
	}

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

	typedef struct _XPARTY_CUSTOM_DATA
	{
		ULONGLONG			qwFirst;
		ULONGLONG			qwSecond;
	} XPARTY_CUSTOM_DATA;

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

#define XPARTY_MAX_USERS	8
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
#elif
	// In case Xbox needs some things patched

#endif

	typedef enum ControllerIndex_t
	{
		INVALID_CONTROLLER_PORT		= -1,

		CONTROLLER_INDEX_FIRST		= 0,
		CONTROLLER_INDEX_0			= 0,

		CONTROLLER_INDEX_COUNT		= 1,
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
		//bdLogMessage_Hook.Create(0x82AC2A88, bdLogMessage);
		//bdLobbyService__getStatus_Hook.Create(0x82AA1A38, bdLobbyService__getStatus);

#if IS_XENIA
		//XEnumerate_Hook.Create(0x829F3440, XEnumerate);
		//XGetOverlappedResult_Hook.Create(0x829F33A0, XGetOverlappedResult);
		//XNetServerToInAddr_Hook.Create(0x82A081C0, XNetServerToInAddr);
		//XUserCheckPrivilege_Hook.Create(0x829F2550, XUserCheckPrivilege);
		//XPartyGetUserList_Hook.Create(0x829F0608, XPartyGetUserList);
#endif

		Live_IsUserSignedInToDemonware_Hook.Create(0x827AC038, Live_IsUserSignedInToDemonware);
		Live_IsUserSignedInToLive_Hook.Create(0x827B0A08, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_Hook.Create(0x827FA9E0, Live_Base_IsConnected);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x827EAF10, LiveStorage_DoWeHaveAllStats);
		Live_HasMultiplayerPrivileges_Hook.Create(0x827FBCA0, Live_HasMultiplayerPrivileges);
		LiveStorage_DoWeHaveLeagues_Hook.Create(0x827F4488, LiveStorage_DoWeHaveLeagues);
		LiveStorage_IsTimeSynced_Hook.Create(0x827ECBD0, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x827F4468, LiveStorage_DoWeHaveContracts);
		Utils::Hook::SetValue<uint8_t>(0x8469B230, 1); // s_geoLocationRetrieved
		LiveElite_CheckProgress_Hook.Create(0x827B9B60, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_Hook.Create(0x8250E8F0, LiveCAC_CheckProgress);
		Live_IsLspCacheInited_Hook.Create(0x827FF3A8, Live_IsLspCacheInited);
		SanityCheckSession_Hook.Create(0x827D3CA0, SanityCheckSession);
		LiveStats_CanPerformStatOperation_Hook.Create(0x827D9AC8, LiveStats_CanPerformStatOperation);
	}

	void UnregisterHooks()
	{
#if IS_XENIA
		XUserCheckPrivilege_Hook.Clear();
		XPartyGetUserList_Hook.Clear();
#endif
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