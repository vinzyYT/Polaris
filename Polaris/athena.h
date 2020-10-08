#ifndef ATHENA_H
#define ATHENA_H

#include "SDK.hpp"
#include "player.h"
#include "inventorymapper.h"

namespace polaris
{
	class Athena
	{
	public:
		// Variables for the ProcessEventHook
		bool m_bIsInFrontend = true;
		bool m_bIsWaitingForLoadingScreen = false;
		bool m_bGameOver;

		// Variables for Athena_Terrain
		Player* m_pPlayer;
		SDK::UFortWeaponItemDefinition* m_pHarvestingToolDefinition;
		SDK::UFortWeaponItemDefinition* m_pSlot2Definition;
		SDK::UFortWeaponItemDefinition* m_pSlot3Definition;
		SDK::UFortWeaponItemDefinition* m_pSlot4Definition;
		SDK::UFortWeaponItemDefinition* m_pSlot5Definition;
		SDK::UFortWeaponItemDefinition* m_pSlot6Definition;
		SDK::UFortWeaponItemDefinition* m_pEditToolDef;
		SDK::UFortWeaponItemDefinition* m_pWallBuildDef;
		SDK::UFortWeaponItemDefinition* m_pFloorBuildDef;
		SDK::UFortWeaponItemDefinition* m_pStairBuildDef;
		SDK::UFortWeaponItemDefinition* m_pRoofBuildDef;
		SDK::UAthenaHUD_C* pAthenaHud;
		Athena();
	};
}

static polaris::Athena* gpAthena;

#endif // ATHENA_H
