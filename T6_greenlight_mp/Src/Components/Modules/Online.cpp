#include "..\..\Std_Include.hpp"
#include "Online.hpp"

#include "..\..\Utils\Hook.hpp"

namespace Online
{
	/**
	 ********
	 * Xbox *
	 ********
	**/

	Utils::Hook::Detour XEnumerate_Hook;
	DWORD XEnumerate(
		HANDLE hEnum,
		PVOID pvBuffer,
		DWORD cbBuffer,
		PDWORD pcItemsReturned,
		PXOVERLAPPED pOverlapped)
	{
		if (pcItemsReturned)
		{
			*pcItemsReturned = 0;
		}

		if (pvBuffer && cbBuffer >= sizeof(DWORD))
		{
			DWORD* serverList = reinterpret_cast<DWORD*>(pvBuffer);
			serverList[0] = 0x12345678; // dummy server ID
		}

		if (pOverlapped)
		{
			XOVERLAPPED* overlapped = reinterpret_cast<XOVERLAPPED*>(pOverlapped);
			overlapped->InternalLow = ERROR_NO_MORE_FILES;
			overlapped->InternalHigh = 1;

			// signal the async completion event
			if (overlapped->hEvent)
			{
				SetEvent(overlapped->hEvent);
			}
			return ERROR_IO_PENDING;
		}
		return ERROR_NO_MORE_FILES;
	}

	Utils::Hook::Detour XGetOverlappedResult_Hook;
	DWORD XGetOverlappedResult(
		PXOVERLAPPED pOverlapped,
		LPDWORD pdwResult,
		BOOL bWait)
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
	INT XNetServerToInAddr(
		const IN_ADDR ina,
		DWORD dwServiceId,
		IN_ADDR *pina)
	{
		if (pina)
		{
			*(unsigned int *)pina = 0xC0A80001; // 192.168.0.1
		}
		return ERROR_SUCCESS;
	}

	/**
	 *************
	 * DemonWare *
	 *************
	**/

	Utils::Hook::Detour bdLobbyService__getStatus_Hook;
	int bdLobbyService__getStatus(int unknown)
	{
		// If there's a connection object at +0xA0, report BD_CONNECTED
		if (*(int*)(unknown + 160))
			return 2; // BD_CONNECTED
		return 0; // BD_NOT_CONNECTED
	}

	/**
	 ********
	 * LIVE *
	 ********
	**/

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

	Utils::Hook::Detour SanityCheckSession_Hook;
	void SanityCheckSession(const char** unknown)
	{
		return;
	}

	void Load()
	{
		XEnumerate_Hook.Create(0x829185D0, XEnumerate);
		XGetOverlappedResult_Hook.Create(0x82917C68, XGetOverlappedResult);
		XNetServerToInAddr_Hook.Create(0x8292EC50, XNetServerToInAddr);
		SanityCheckSession_Hook.Create(0x8273E7E8, SanityCheckSession);

		bdLobbyService__getStatus_Hook.Create(0x82A892B8, bdLobbyService__getStatus);

		Live_IsUserSignedInToDemonware_Hook.Create(0x827097F0, Live_IsUserSignedInToDemonware);
		Live_IsUserSignedInToLive_Hook.Create(0x82710A60, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_Hook.Create(0x82769478, Live_Base_IsConnected);
		LiveStorage_DoWeHaveCurrentStats_Hook.Create(0x82754A48, LiveStorage_DoWeHaveCurrentStats);
		LiveStorage_DoWeHaveStats_Hook.Create(0x827549E8, LiveStorage_DoWeHaveStats);
		LiveStorage_ValidateFFOTD_Hook.Create(0x82760170, LiveStorage_ValidateFFOTD);
		LiveStorage_DoWeHavePlaylists_Hook.Create(0x8275F398, LiveStorage_DoWeHavePlaylists);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x82754AB0, LiveStorage_DoWeHaveAllStats);
		LiveStorage_IsTimeSynced_Hook.Create(0x827563D0, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x8275F3C8, LiveStorage_DoWeHaveContracts);
	}
}