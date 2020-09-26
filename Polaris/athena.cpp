#include "athena.h"
#include "console.h"
#include "util.h"
#include "SDK.hpp"

namespace polaris
{
	SDK::UAthenaHUD_C* pHud;
	static SDK::UObject* (*StaticLoadObject)(SDK::UClass* ObjectClass, SDK::UObject* InOuter, const TCHAR* InName, const TCHAR* Filename, uint32_t LoadFlags, SDK::UPackageMap* Sandbox, bool bAllowObjectReconciliation);

	// Load an object in memory.
	template<class T>
	static T* LoadObject(SDK::UObject* Outer, const TCHAR* Name, const TCHAR* Filename = nullptr, uint32_t LoadFlags = 0, SDK::UPackageMap* Sandbox = nullptr)
	{
		return (T*)StaticLoadObject(T::StaticClass(), Outer, Name, Filename, LoadFlags, Sandbox, true);
	}

	// Find an object in cache, load it if it's not loaded.
	template<typename T>
	static T* FindOrLoadObject(const std::string PathName)
	{
		SDK::UClass* Class = T::StaticClass();
		Class->CreateDefaultObject();

		T* ObjectPtr = LoadObject<T>(NULL, std::wstring(PathName.begin(), PathName.end()).c_str());
		if (ObjectPtr)
		{
			SDK::UObject::GObjects
				->ObjObjects.GetItemByIndex(ObjectPtr->InternalIndex)->Flags |= int32_t(SDK::FUObjectItem::ObjectFlags::RootSet);
		}

		return ObjectPtr;
	}

	bool bIsInFrontend = true;
	bool bIsWaitingForLoadingScreen = false;

	// Preset weapon definitions. We keep them here so we don't need to constantly reload them.
	SDK::UFortWeaponItemDefinition* pTacticalShotgunDefinition;
	SDK::UFortWeaponItemDefinition* pHarvestingToolDefinition;
	SDK::UFortWeaponItemDefinition* pJackOLauncherDefinition;
	SDK::UFortWeaponItemDefinition* pPumpShotgunDefinition;
	SDK::UFortWeaponItemDefinition* pZapatronDefinition;
	SDK::UFortWeaponItemDefinition* pScarDefinition;

	// Listens to functions called in game, use this to hook code to functions in question.
	PVOID(*ProcessEvent)(SDK::UObject*, SDK::UFunction*, PVOID) = nullptr;
	PVOID ProcessEventHook(SDK::UObject* pObject, SDK::UFunction* pFunction, PVOID pParams)
	{
		if (pObject && pFunction)
		{
			// Hooks for Frontend
			if (bIsInFrontend)
			{
				// What the fuck is this function name.
				// Load Athena_Terrain once the player readies up.
				if (pFunction->GetName().find("BndEvt__BP_PlayButton_K2Node_ComponentBoundEvent_1_CommonButtonClicked__DelegateSignature") != std::string::npos)
				{
					bIsWaitingForLoadingScreen = true;

					Globals::gpPlayerController->SwitchLevel(TEXT("Athena_Terrain"));

					return NULL;
				}

				// This gets called once the loading screen is ready to drop.
				// We reinitialize the SDK, Core and update memory to fix the abilities.
				if (pFunction->GetName().find("ReadyToStartMatch") != std::string::npos && bIsWaitingForLoadingScreen)
				{
					bIsInFrontend = false;
					bIsWaitingForLoadingScreen = false;

					// Reinitialize the core, there's missing references because of Frontend unloading.
					Util::InitSdk();
					Util::InitCore();
					Util::InitPatches();

					StaticLoadObject = reinterpret_cast<decltype(StaticLoadObject)>(Util::BaseAddress() + 0x142E560);

					FindOrLoadObject<SDK::UDataTable>("/Game/Items/Datatables/AthenaTraps.AthenaTraps");
					FindOrLoadObject<SDK::UDataTable>("/Game/Athena/Items/Weapons/AthenaMeleeWeapons.AthenaMeleeWeapons");
					FindOrLoadObject<SDK::UDataTable>("/Game/Athena/Items/Weapons/AthenaRangedWeapons.AthenaRangedWeapons");
					FindOrLoadObject<SDK::UDataTable>("/Game/Items/Datatables/RangedWeapons.RangedWeapons");
					FindOrLoadObject<SDK::UDataTable>("/Game/Items/Datatables/MeleeWeapons.MeleeWeapons");
					FindOrLoadObject<SDK::UDataTable>("/Game/Items/Datatables/Traps.Traps");

					if (!gpAthena->m_pPlayerPawnPolaris)
					{
						// Create a new Player Pawn.
						gpAthena->m_pPlayerPawnPolaris = new PlayerPawnPolaris;
						gpAthena->m_pPlayerPawnPolaris->InitializeHero();

						// Load preset item definitions (Prevents a hitch during weapon swapping)
						pHarvestingToolDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>(gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CustomizationLoadout.Character->GetFullName());
						pPumpShotgunDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>("FortWeaponRangedItemDefinition WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
						pScarDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>("FortWeaponRangedItemDefinition WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03");
						pTacticalShotgunDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>("FortWeaponRangedItemDefinition WID_Shotgun_SemiAuto_Athena_VR_Ore_T03.WID_Shotgun_SemiAuto_Athena_VR_Ore_T03");
						pJackOLauncherDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>("FortWeaponRangedItemDefinition WID_Launcher_Rocket_Athena_SR_Ore_T03.WID_Launcher_Rocket_Athena_SR_Ore_T03");
						pZapatronDefinition = SDK::UObject::FindObject<SDK::UFortWeaponItemDefinition>("FortWeaponRangedItemDefinition WID_Sniper_AMR_Athena_SR_Ore_T03.WID_Sniper_AMR_Athena_SR_Ore_T03");

						// Tell the client that we are ready to start the match, this allows the loading screen to drop.
						static_cast<SDK::AAthena_PlayerController_C*>(Globals::gpPlayerController)->ServerReadyToStartMatch();
						static_cast<SDK::AGameMode*>((*Globals::gpWorld)->AuthorityGameMode)->StartMatch();

						// Turn on basic cheats
						static_cast<SDK::UFortCheatManager*>(Globals::gpPlayerController->CheatManager)->ToggleInfiniteAmmo();
					}
				}
			}

			// Hooks for Athena_Terrain
			else
			{
				// Called every frame.
				if (pFunction->GetName().find("Tick") != std::string::npos)
				{
					if (gpAthena->m_pPlayerPawnPolaris && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && !static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->IsInAircraft())
					{
						// This is awful, I know, it's just for the demo.
						if (GetAsyncKeyState('1') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 0;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pHarvestingToolDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 0);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 0);
							}
						}

						if (GetAsyncKeyState('2') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 1;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pPumpShotgunDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 1);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 1);
							}
						}

						if (GetAsyncKeyState('3') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 2;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pScarDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 2);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 2);
							}
						}

						if (GetAsyncKeyState('4') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 3;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pTacticalShotgunDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 3);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 3);
							}
						}

						if (GetAsyncKeyState('5') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 4;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pJackOLauncherDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 4);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 4);
							}
						}

						if (GetAsyncKeyState('6') & 0x8000)
						{
							SDK::FGuid guid;
							guid.A = 5;
							guid.B = 0;
							guid.C = 0;
							guid.D = 0;

							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->EquipWeaponDefinition(pZapatronDefinition, guid);

							if (pHud)
							{
								pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 5);
								pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 5);
							}
						}

						if (GetAsyncKeyState(VK_END) & 0x8000 && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->bBlockInput)
						{
							static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->ClientNotifyWon();
							static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->PlayWinEffects();
						}
					}
				}

				// (irma) This is broken, I'll find out a fix later.
				/*if (pFunction->GetName().find("GameplayCue.Athena.OutsideSafeZone") != std::string::npos)
				{
					if (gpAthena->m_pPlayerPawnPolaris && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && !static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->IsInAircraft())
					{
						gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->OnRep_IsOutsideSafeZone();
						if (gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->bIsOutsideSafeZone)
							static_cast<SDK::APlayerPawn_Athena_C*>(gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn)->AddStormFX();
						else
							static_cast<SDK::APlayerPawn_Athena_C*>(gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn)->RemoveStormFX();
					}
				}*/

				// Called once the player jumps from the battle bus, or when they're supposed to be kicked out.
				if (pFunction->GetName().find("ServerAttemptAircraftJump") != std::string::npos || 
					pFunction->GetName().find("OnAircraftExitedDropZone") != std::string::npos)
				{
					if (static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->IsInAircraft())
					{
						// Create a new player pawn.
						gpAthena->m_pPlayerPawnPolaris = new PlayerPawnPolaris;
						gpAthena->m_pPlayerPawnPolaris->InitializeHero(); // This causes a massive hitch.

						// Reset the pawn rotation, due to weird summon properties.
						SDK::AFortPlayerPawnAthena* playerPawn = gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn;
						SDK::FRotator actorRotation = gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->K2_GetActorRotation();

						actorRotation.Pitch = 0;
						actorRotation.Roll = 0;
						playerPawn->K2_SetActorLocationAndRotation(playerPawn->K2_GetActorLocation(), actorRotation, false, true, new SDK::FHitResult());

						// Don't return null if the aircraft exited drop zone, or the storm won't start.
						if (pFunction->GetName().find("OnAircraftExitedDropZone") == std::string::npos)
							return NULL;
					}
				}
			}
		}

		return ProcessEvent(pObject, pFunction, pParams);
	}

	// Update loop, runs at 60hz, 1 tick every 16ms.
	DWORD UpdateThread(LPVOID lpParam)
	{
		while (1)
		{
			if (gpAthena->m_pPlayerPawnPolaris != nullptr && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && !static_cast<SDK::AFortPlayerControllerAthena*>(Globals::gpPlayerController)->IsInAircraft())
			{
				if (!pHud)
				{
					pHud = SDK::UObject::FindObject<SDK::UAthenaHUD_C>("AthenaHUD_C Transient.FortEngine_1.FortGameInstance_1.AthenaHUD_C_1");

					if (pHud)
					{
						auto library = SDK::UObject::FindObject<SDK::UFortUIBlueprintFunctionLibrary>("FortUIBlueprintFunctionLibrary FortniteUI.Default__FortUIBlueprintFunctionLibrary");
						if (library)
						{
							pHud->QuickbarPrimary->QuickbarSlot1->Empty->SetColorAndOpacity(SDK::FLinearColor(255, 255, 255, 255));
							pHud->QuickbarPrimary->QuickbarSlot1->Empty->SetBrush(library->STATIC_GetItemSmallPreviewImageBrush(pPumpShotgunDefinition->CreateTemporaryItemInstanceBP(1, 1), SDK::TEnumAsByte<SDK::EFortBrushSize>(SDK::EFortBrushSize::Large)));
						}

						pHud->QuickbarPrimary->OnQuickbarSlotFocusChanged(SDK::EFortQuickBars::Primary, 0);
						pHud->HandleQuickbarSlotFocusSlotChanged(SDK::EFortQuickBars::Primary, 0);
					}
				}

				// Keybind to jump (only run if not skydiving, might need to fix this more though):
				if (GetAsyncKeyState(VK_SPACE) & 0x8000 && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsSkydiving() && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsJumpProvidingForce())
				{
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->Jump();
					Sleep(200);
				}
				else if (GetAsyncKeyState(VK_SPACE) & 0x8000 && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsSkydiving() && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsParachuteOpen() && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsParachuteForcedOpen())
				{
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Custom, 2U);
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->OnRep_IsParachuteOpen(false);
					Sleep(200);
				}
				else if (GetAsyncKeyState(VK_SPACE) & 0x8000 && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsSkydiving() && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsParachuteOpen() && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->IsParachuteForcedOpen())
				{
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CharacterMovement->SetMovementMode(SDK::EMovementMode::MOVE_Custom, 3U);
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->OnRep_IsParachuteOpen(true);
					Sleep(200);
				}

				// Keybind to sprint (only run if not skydiving & not targeting, else walk):
				if (static_cast<SDK::AAthena_PlayerController_C*>(Globals::gpPlayerController)->bWantsToSprint && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon->bIsTargeting)
				{
					// If reloading, only allow sprinting if Sprinting Cancels Reload is on.
					if (gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon->IsReloading())
					{
						if (static_cast<SDK::AAthena_PlayerController_C*>(Globals::gpPlayerController)->bSprintCancelsReload)
						{
							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon->bIsReloadingWeapon = false;
							gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentMovementStyle = SDK::EFortMovementStyle::Sprinting;
						}
					}
					else
						gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentMovementStyle = SDK::EFortMovementStyle::Sprinting;
				}
				else if (gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon && !gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon->bIsTargeting)
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentMovementStyle = SDK::EFortMovementStyle::Running;
				else if (gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon && gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentWeapon->bIsTargeting)
					gpAthena->m_pPlayerPawnPolaris->m_pPlayerPawn->CurrentMovementStyle = SDK::EFortMovementStyle::Walking;
			}

			// Update thread only runs at 60hz, so we don't rape CPUs.
			Sleep(1000 / 60);
		}

		return NULL;
	}

	// Initializes the ProcessEventHook and starts the Update thread.
	DWORD WINAPI LoaderThread(LPVOID lpParam)
	{
		Util::InitSdk();
		Util::InitCore();
		
		auto pProcessEventAddress = Util::FindPattern("\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8D\x6C\x24\x00\x48\x89\x9D\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC5\x48\x89\x85\x00\x00\x00\x00\x48\x63\x41\x0C", "xxxxxxxxxxxxxxx????xxxx?xxx????xxx????xxxxxx????xxxx");
		if (!pProcessEventAddress)
		{
			MessageBox(NULL, static_cast<LPCWSTR>(L"Finding pattern for ProcessEvent has failed. Please relaunch Fortnite and try again!"), static_cast<LPCWSTR>(L"Error"), MB_ICONERROR);
			ExitProcess(EXIT_FAILURE);
		}

		MH_CreateHook(static_cast<LPVOID>(pProcessEventAddress), ProcessEventHook, reinterpret_cast<LPVOID*>(&ProcessEvent));
		MH_EnableHook(static_cast<LPVOID>(pProcessEventAddress));

		CreateThread(0, 0, UpdateThread, 0, 0, 0); // Create thread to handle input, etc...

		return FALSE;
	}

	Athena::Athena()
	{
		// Check if athena is already initialized.
		if (gpAthena)
		{
			MessageBox(0, L"Athena is already initialized.", L"Error", MB_ICONERROR);
			ExitProcess(EXIT_FAILURE);
		}

		gpAthena = this;

		Console::Log("Loading Athena");

		CreateThread(0, 0, LoaderThread, 0, 0, 0);
	}
}
