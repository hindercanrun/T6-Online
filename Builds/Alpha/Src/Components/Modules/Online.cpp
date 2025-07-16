#include "..\..\Std_Include.hpp"
#include "Online.hpp"

#include "../../../../Utils/Hook.hpp"

#define XPARTY_MAX_USERS 8

namespace Online
{
	/**
	 **************
	 * Structures *
	 **************
	**/

	typedef struct _XPARTY_CUSTOM_DATA
	{
		ULONGLONG qwFirst;
		ULONGLONG qwSecond;
	} XPARTY_CUSTOM_DATA;

	typedef struct _XPARTY_USER_INFO
	{
		XUID Xuid;
		CHAR GamerTag[XUSER_NAME_SIZE];
		DWORD dwUserIndex;
		XONLINE_NAT_TYPE NatType;
		DWORD dwTitleId;
		DWORD dwFlags;
		XSESSION_INFO SessionInfo;
		XPARTY_CUSTOM_DATA CustomData;
	} XPARTY_USER_INFO;

	typedef struct _XPARTY_USER_LIST
	{
		DWORD dwUserCount;
		XPARTY_USER_INFO Users[XPARTY_MAX_USERS];
	} XPARTY_USER_LIST;

	struct CompositeEmblemLayer
	{
		int iconIndex;     // 0x00
		int colorIndex;    // 0x04
		float posX;        // 0x08
		float posY;        // 0x0C
		float scaleX;      // 0x10
		float scaleY;      // 0x14
		float angle;       // 0x18
		bool outline;      // 0x1C
		bool flip;         // 0x1D
		char pad[0x1E];    // Padding to match 0x20 or 0x40 byte size if needed
	};

	/**
	 ********
	 * Xbox *
	 ********
	**/

	Utils::Hook::Detour XEnumerate_hook;
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

	Utils::Hook::Detour XGetOverlappedResult_hook;
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

	Utils::Hook::Detour XGetOverlappedExtendedError_hook;
	DWORD XGetOverlappedExtendedError(
		PXOVERLAPPED pOverlapped)
	{
		return ERROR_SUCCESS;
	}

	Utils::Hook::Detour XNetServerToInAddr_hook;
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

	typedef struct _XUID
	{
		unsigned __int64 qwUserID;
	} XUID;

	typedef struct _XSESSION_MEMBER
	{
		DWORD dwUserIndex;
		XUID  xuid;
		DWORD dwFlags;

		BOOL bIsGuest;
		BOOL bIsMuted;
		BOOL bIsTalking;
		BOOL bIsInMultiplayerSession;

		BYTE reserved[24]; // padding for future compatibility (make sure total size matches what the game expects)
	} XSESSION_MEMBER;

	typedef struct _XSESSION_LOCAL_DETAILS
	{
		DWORD dwUserIndexHost;

		DWORD dwMaxPublicSlots;
		DWORD dwMaxPrivateSlots;
		DWORD dwAvailablePublicSlots;
		DWORD dwAvailablePrivateSlots;
		DWORD dwOpenPublicSlots;
		DWORD dwOpenPrivateSlots;

		DWORD sessionState;
		DWORD dwFlags;

		DWORD dwSessionNonce[4]; // random session ID / nonce

		DWORD dwReserved;

		DWORD dwActualMemberCount;
		DWORD dwReturnedMemberCount;

		// Followed by XSESSION_MEMBER[] in buffer
	} XSESSION_LOCAL_DETAILS;

#define XSESSION_STATE_INACTIVE 0
#define XSESSION_STATE_ACTIVE   1

#define XSESSION_MEMBER_FLAG_LOCAL        0x00000001
#define XSESSION_MEMBER_FLAG_COMMUNICATIONS_ALLOWED 0x00000004

	Utils::Hook::Detour XSessionGetDetails_hook;
DWORD XSessionGetDetails(
	HANDLE hSession,
	DWORD* pcbResultsBuffer,
	XSESSION_LOCAL_DETAILS* pSessionDetails,
	PXOVERLAPPED pOverlapped)
{
	const DWORD requiredSize = sizeof(XSESSION_LOCAL_DETAILS) + sizeof(XSESSION_MEMBER);

	if (!pcbResultsBuffer)
		return ERROR_INVALID_PARAMETER;

	if (*pcbResultsBuffer < requiredSize)
	{
		*pcbResultsBuffer = requiredSize;
		return ERROR_INSUFFICIENT_BUFFER;
	}

	if (!pSessionDetails)
		return ERROR_INVALID_PARAMETER;

	*pcbResultsBuffer = requiredSize;

	memset(pSessionDetails, 0, sizeof(XSESSION_LOCAL_DETAILS));

	pSessionDetails->dwUserIndexHost     = 0;
	pSessionDetails->dwMaxPublicSlots    = 18;
	pSessionDetails->dwMaxPrivateSlots   = 18;
	pSessionDetails->dwOpenPublicSlots   = 17; // 1 used
	pSessionDetails->dwOpenPrivateSlots  = 18;
	pSessionDetails->sessionState        = XSESSION_STATE_ACTIVE;
	pSessionDetails->dwFlags             = XSESSION_CREATE_USES_PRESENCE; // optional
	pSessionDetails->dwActualMemberCount = 1;
	pSessionDetails->dwReturnedMemberCount = 1;

	// Fill member list
	XSESSION_MEMBER* members = reinterpret_cast<XSESSION_MEMBER*>(
		reinterpret_cast<uint8_t*>(pSessionDetails) + sizeof(XSESSION_LOCAL_DETAILS));

	memset(members, 0, sizeof(XSESSION_MEMBER));

	members[0].dwUserIndex     = 0;
	members[0].xuid.qwUserID   = 0xE000123456789ABCULL;  // fake XUID
	members[0].dwFlags         = XSESSION_MEMBER_FLAG_LOCAL | XSESSION_MEMBER_FLAG_COMMUNICATIONS_ALLOWED;
	members[0].bIsGuest        = FALSE;
	members[0].bIsMuted        = FALSE;
	members[0].bIsTalking      = FALSE;
	members[0].bIsInMultiplayerSession = TRUE;

	return ERROR_SUCCESS;
}

	Utils::Hook::Detour XSessionEnd_hook;
	DWORD XSessionEnd(
		HANDLE hSession,
		PXOVERLAPPED pXOverlapped)
	{
		if (pXOverlapped)
		{
			pXOverlapped->InternalHigh = 0;
			pXOverlapped->dwExtendedError = ERROR_SUCCESS;

			if (pXOverlapped->hEvent)
			{
				SetEvent(pXOverlapped->hEvent);
			}
			return ERROR_IO_PENDING;
		}
		else
		{
			return ERROR_SUCCESS;
		}
	}

	Utils::Hook::Detour XSessionDelete_hook;
	DWORD XSessionDelete(
		HANDLE hSession,
		PXOVERLAPPED pXOverlapped)
	{
		if (pXOverlapped)
		{
			pXOverlapped->InternalHigh = 0;
			pXOverlapped->dwExtendedError = ERROR_SUCCESS;

			if (pXOverlapped->hEvent)
			{
				SetEvent(pXOverlapped->hEvent);
			}
			return ERROR_IO_PENDING;
		}
		else
		{
			return ERROR_SUCCESS;
		}
	}

	Utils::Hook::Detour XPartyGetUserList_hook;
	HRESULT XPartyGetUserList(XPARTY_USER_LIST* pUserList)
	{
		if (pUserList)
		{
			pUserList->dwUserCount = 1;
		}
		return S_OK;
	}

	/**
	 *************
	 * DemonWare *
	 *************
	**/

	typedef enum _bdLogMessageType
	{
		BD_LOG_INFO = 0,
		BD_LOG_WARNING = 1,
		BD_LOG_ERROR = 2,
	} bdLogMessageType;

	Utils::Hook::Detour bdLogMessage_hook;
	void bdLogMessage(
		bdLogMessageType type,
		const char* baseChannel,
		const char* channel,
		const char* file,
		const char* function,
		int line,
		const char* format,
		...)
	{
		Symbols::Com_Printf(0, "\n");
		Symbols::Com_Printf(0, "DW: file: %s\n", file);
		Symbols::Com_Printf(0, "DW: function: %s\n", function);
		Symbols::Com_Printf(0, "DW: line: %d\n", line);
		Symbols::Com_Printf(0, "DW: channel: %s : %s\n", channel, baseChannel);

		char buffer[1024];

		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		Symbols::Com_Printf(0, "DW: message: %s\n", buffer);
	}

	Utils::Hook::Detour bdPlatformStreamSocket__isWritable_hook;
	int bdPlatformStreamSocket__isWritable(int socket, int* status)
	{
		if (status)
		{
			*status = 1;
		}
		return 1;
	}

	Utils::Hook::Detour bdLobbyService__getStatus_hook;
	int __fastcall bdLobbyService__getStatus(int a1)
	{
		// If there's a connection object at +0xA0, report BD_CONNECTED
		if (*(int*)(a1 + 160))
			return 2; // BD_CONNECTED

		return 0; // BD_NOT_CONNECTED
	}

	typedef struct _bdStreamSocket
	{
		int m_socket;
	} bdStreamSocket;

	unsigned __int64 g_totalBytesSent = 0;

	Utils::Hook::Detour bdPlatformStreamSocket__send_hook;
	int bdPlatformStreamSocket__send(int socketFd, const void* data, unsigned int size)
	{
		if (socketFd == -1)
		{
			return -10;
		}
		g_totalBytesSent += size;
		return size;
	}

	typedef struct _bdLobbyConnection
	{
		int recvState; // 0-4 state machine
	} bdLobbyConnection;

	Utils::Hook::Detour bdLobbyConnection__recvMessageData_hook;
	BOOL bdLobbyConnection__recvMessageData(bdLobbyConnection* conn)
	{
		// Pretend we instantly received the full message
		conn->recvState = 4;
		return TRUE;
	}

	class bdTaskResult;

	class bdRemoteTask
	{
	public:
		enum bdStatus
		{
			BD_PENDING = 0,
			BD_INPROGRESS = 1,
			BD_DONE = 2,
			BD_FAILED = 3
		};

		bdRemoteTask()
			: m_status(BD_DONE), m_taskResult(0), m_numResults(0)
		{
		}

		virtual bdStatus getStatus() const { return m_status; }
		virtual void setTaskResult(bdTaskResult *result, unsigned int resultSize)
		{
			m_taskResult = result;
			m_numResults = 1;
		}

	protected:
		bdStatus m_status;
		bdTaskResult *m_taskResult;
		unsigned int m_numResults;
	};


	class bdTaskResult
	{
	public:
		virtual ~bdTaskResult() {}
	};

	// Stub function for bdStats::readStatsByPivot (VS2010 compatible)
	Utils::Hook::Detour bdStats__readStatsByPivot_hook;
	bdRemoteTask **__fastcall bdStats__readStatsByPivot(
		bdRemoteTask **a1,
		int *a2,
		unsigned int a3,
		int a4,
		bdTaskResult *a5,
		unsigned int a6)
	{
		// Simple static task that is always "done"
		static bdRemoteTask dummyTask;

		*a1 = &dummyTask;

		if (a5)
		{
			dummyTask.setTaskResult(a5, a6);
		}

		return a1;
	}

	/**
	 ********
	 * LIVE *
	 ********
	**/

	Utils::Hook::Detour Live_IsUserSignedInToDemonware_hook;
	BOOL Live_IsUserSignedInToDemonware(int controllerIndex)
	{
		return 2;
	}

	Utils::Hook::Detour Live_IsUserSignedInToLive_hook;
	BOOL Live_IsUserSignedInToLive(int controllerIndex)
	{
		return 2;
	}

	Utils::Hook::Detour Live_Base_IsConnected_hook;
	BOOL Live_Base_IsConnected(int controllerIndex)
	{
		return 5;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveCurrentStats_hook;
	bool LiveStorage_DoWeHaveCurrentStats(int controllerIndex)
	{
		static bool hasBeenCalled = false;
		if (!hasBeenCalled)
		{
			hasBeenCalled = true;
			return false;
		}
		return true;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveStats_hook;
	bool LiveStorage_DoWeHaveStats(int controllerIndex, int playerStatsLocation)
	{
		return true;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveLeagues_hook;
	bool LiveStorage_DoWeHaveLeagues()
	{
		return true;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveAllStats_hook;
	bool LiveStorage_DoWeHaveAllStats(int controllerIndex)
	{
		return true;
	}

	Utils::Hook::Detour LiveStorage_IsTimeSynced_hook;
	bool LiveStorage_IsTimeSynced()
	{
		return true;
	}

	Utils::Hook::Detour LiveStorage_DoWeHaveContracts_hook;
	bool LiveStorage_DoWeHaveContracts()
	{
		return true;
	}

	Utils::Hook::Detour LiveLeaderboard_GetRankInLeaderBoardState_hook;
	int LiveLeaderboard_GetRankInLeaderBoardState(int controllerIndex)
	{
		return 2;
	}

	Utils::Hook::Detour LiveElite_CheckProgress_hook;
	BOOL LiveElite_CheckProgress(int localControllerIndex, bool status)
	{
		return 1;
	}

	Utils::Hook::Detour LiveCAC_CheckProgress_hook;
	BOOL LiveCAC_CheckProgress(int localControllerIndex, bool status)
	{
		return 1;
	}

#define MAX_PLAYERS 19

	Utils::Hook::Detour Session_GetXuid_hook;
	unsigned __int64 Session_GetXuid(int clientNum)
	{
		// Hopefully the game is okay with this and assigns this to each player.
		static const unsigned __int64 fakeXuids[MAX_PLAYERS] =
		{
			0x110000100000001, 0x110000100000002, 0x110000100000003,
			0x110000100000004, 0x110000100000005, 0x110000100000006,
			0x110000100000007, 0x110000100000008, 0x110000100000009,
			0x11000010000000A, 0x11000010000000B, 0x11000010000000C,
			0x11000010000000D, 0x11000010000000E, 0x11000010000000F,
			0x110000100000010, 0x110000100000011, 0x110000100000012,
			0x110000100000013
		};

		if (clientNum >= MAX_PLAYERS)
			return 0; // No XUID

		return fakeXuids[clientNum];
	}

	/**
	 **********************
	 * Profile/Rank Stubs *
	 **********************
	**/

	Utils::Hook::Detour PCache_GetProfileEmblem_hook;
	void PCache_GetProfileEmblem(
		struct PCachePublicProfile *profile,
		struct CompositeEmblemLayer *layers,
		int layerCount,
		__int16 *backgroundOut)
	{
		if (!profile || !layers || !backgroundOut || layerCount <= 0)
			return;

		for (int i = 0; i < layerCount; ++i)
		{
			CompositeEmblemLayer &layer = layers[i];
			layer.iconIndex = 0;
			layer.colorIndex = 0;
			layer.posX = 0.0f;
			layer.posY = 0.0f;
			layer.scaleX = 1.0f;
			layer.scaleY = 1.0f;
			layer.angle = 0.0f;
			layer.outline = false;
			layer.flip = false;
		}

		// dummy background
	}

	Utils::Hook::Detour CL_GetZombieRank_hook;
	int CL_GetZombieRank(int timePlayed, int weightedDowns, int weightedRounds)
	{
		// Stub: Calculate simple fake skill and rank

		float skill = 0.0f;
		if (weightedDowns > 0)
		{
			skill = weightedRounds / (weightedDowns * 0.001f);
		}

		int tier = timePlayed / 300000; // 5 minutes per tier
		int rank = (int)(skill / 100.0f); // 100 skill per rank step

		if (tier > 10)
		{
			tier = 10;
		}

		if (rank > 15)
		{
			rank = 15;
		}
		return tier + rank; // Fake combination
	}

	void Load()
	{
		// Xbox
		XEnumerate_hook.Create(0x8298B798, XEnumerate);
		XGetOverlappedResult_hook.Create(0x8298B6F8, XGetOverlappedResult);
		XGetOverlappedExtendedError_hook.Create(0x82990F20, XGetOverlappedExtendedError);
		XNetServerToInAddr_hook.Create(0x8299FFF0, XNetServerToInAddr);
		//XSessionGetDetails_hook.Create(0x829A2948, XSessionGetDetails);
		XSessionEnd_hook.Create(0x829A2110, XSessionEnd);
		XSessionDelete_hook.Create(0x829A1E68, XSessionDelete);
		//XPartyGetUserList_hook.Create(0x82988D48, XPartyGetUserList);

		// DemonWare
		bdLogMessage_hook.Create(0x82A59CB0, bdLogMessage);
		bdPlatformStreamSocket__isWritable_hook.Create(0x82B52E58, bdPlatformStreamSocket__isWritable);
		bdLobbyService__getStatus_hook.Create(0x82A396B0, bdLobbyService__getStatus);
		bdPlatformStreamSocket__send_hook.Create(0x82B52F68, bdPlatformStreamSocket__send);
		//bdLobbyConnection__recvMessageData_hook.Create(0x82A56D68, bdLobbyConnection__recvMessageData);
		bdStats__readStatsByPivot_hook.Create(0x82A3F978, bdStats__readStatsByPivot);

		// LIVE
		Live_IsUserSignedInToDemonware_hook.Create(0x82763C10, Live_IsUserSignedInToDemonware);
		Live_IsUserSignedInToLive_hook.Create(0x82767FA0, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_hook.Create(0x827B5500, Live_Base_IsConnected);
		LiveStorage_DoWeHaveCurrentStats_hook.Create(0x827A09A0, LiveStorage_DoWeHaveCurrentStats);
		LiveStorage_DoWeHaveStats_hook.Create(0x827A0970, LiveStorage_DoWeHaveStats);
		// TODO: Maybe these could be removed later?
		LiveStorage_DoWeHaveLeagues_hook.Create(0x827AB938, LiveStorage_DoWeHaveLeagues);
		LiveStorage_DoWeHaveAllStats_hook.Create(0x827A0A10, LiveStorage_DoWeHaveAllStats);
		LiveStorage_IsTimeSynced_hook.Create(0x827A2810, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_hook.Create(0x827AB918, LiveStorage_DoWeHaveContracts);
		LiveLeaderboard_GetRankInLeaderBoardState_hook.Create(0x824E5710, LiveLeaderboard_GetRankInLeaderBoardState);
		LiveElite_CheckProgress_hook.Create(0x8276FD78, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_hook.Create(0x824E1888, LiveCAC_CheckProgress);
		// End
		Session_GetXuid_hook.Create(0x82785EA0, Session_GetXuid);

		PCache_GetProfileEmblem_hook.Create(0x82781890, PCache_GetProfileEmblem);
		CL_GetZombieRank_hook.Create(0x8234DD60, CL_GetZombieRank); // Just for the Zombies menu

		Utils::Hook::SetValue(0x82764778, 1); // byte_849016D4
	}
}