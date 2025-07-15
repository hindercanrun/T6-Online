#include "..\..\Std_Include.hpp"
#include "Online.hpp"

#include "..\..\Utils\Hook.hpp"

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
	bool LiveStorage_DoWeHaveAllStats(int controllerIndex)
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveStorage_DoWeHavePlaylists_Hook;
	bool LiveStorage_DoWeHavePlaylists()
	{
		return TRUE;
	}

	Utils::Hook::Detour Live_HasMultiplayerPrivileges_Hook;
	bool Live_HasMultiplayerPrivileges(int controllerIndex)
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

	Utils::Hook::Detour LiveElite_CheckProgress_Hook;
	bool LiveElite_CheckProgress(int localControllerIndex, bool status)
	{
		return TRUE;
	}

	Utils::Hook::Detour LiveCAC_CheckProgress_Hook;
	bool LiveCAC_CheckProgress(int localControllerIndex, bool status)
	{
		return TRUE;
	}

	void Load()
	{
		Live_IsUserSignedInToDemonware_Hook.Create(0x824DF1D8, Live_IsUserSignedInToDemonware);
		Live_IsUserSignedInToLive_Hook.Create(0x824E2DD8, Live_IsUserSignedInToLive);
		Live_Base_IsConnected_Hook.Create(0x82519418, Live_Base_IsConnected);
		LiveStorage_DoWeHaveFFOTD_Hook.Create(0x82514B68, LiveStorage_DoWeHaveFFOTD);
		LiveStorage_ValidateFFOTD_Hook.Create(0x82514B98, LiveStorage_ValidateFFOTD);
		LiveStorage_DoWeHaveAllStats_Hook.Create(0x8250C1C0, LiveStorage_DoWeHaveAllStats);
		LiveStorage_DoWeHavePlaylists_Hook.Create(0x825142D8, LiveStorage_DoWeHavePlaylists);
		Live_HasMultiplayerPrivileges_Hook.Create(0x8221A5C0, Live_HasMultiplayerPrivileges);
		LiveStorage_DoWeHaveLeagues_Hook.Create(0x82514308, LiveStorage_DoWeHaveLeagues);
		LiveStorage_IsTimeSynced_Hook.Create(0x8250D218, LiveStorage_IsTimeSynced);
		LiveStorage_DoWeHaveContracts_Hook.Create(0x825142E8, LiveStorage_DoWeHaveContracts);
		LiveElite_CheckProgress_Hook.Create(0x824E78B0, LiveElite_CheckProgress);
		LiveCAC_CheckProgress_Hook.Create(0x82352138, LiveCAC_CheckProgress);

		Utils::Hook::SetValue<uint8_t>(0x8424C6F0, 1); // byte_8424C6F0
	}
}