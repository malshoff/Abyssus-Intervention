#include "CheatMain.h"
#include "Config.h"
#include "../Services/WeaponService.h"
#include "../Services/AimbotService.h"
#include "../Services/BoneService.h"
#include "../Utils/Console.h"
#include "../Utils/Input.h"
#include "../Services/GameServices.h"
#include "../Services/PlayerEffectsService.h"
#include <dev/logger.h>
#include <iostream>
#include <mutex>

// Mutex for thread-safe target list access (global scope)
std::mutex list_mutex;

namespace Cheat {
    namespace Core {

        bool CheatMain::Initialize() {
            if (Config::System::Initialized) {
                LOG_INFO("CheatMain already initialized");
                return true;
            }

            LOG_INFO("Initializing cheat system (deferred-ready)...");
            std::cout << "Command line args: " << SDK::UKismetSystemLibrary::GetCommandLineW().ToString();
            // Initialize centralized configuration
            Config::Initialize();

            // Do not force SDK readiness here — allow Update() to bring us live when the world exists
            if (!UpdateSDK(true)) {
                LOG_WARN("Game world not ready yet; will finalize initialization in Update()");
                return true; // Not a hard failure; we'll finish init later
            }

            // Initialize subsystems
            if (!InitializeSubsystems()) {
                LOG_ERROR("Failed to initialize subsystems");
                return false;
            }

            Config::System::LastFrameTime = GetTickCount();
            Config::System::Initialized = true;

            LOG_INFO("=== CHEAT SYSTEM READY FOR OPERATION ===");
            Config::PrintConfiguration();

            return true;
        }
        
        void CheatMain::Update(DWORD tick) {
            // Lazy initialize when the game world is ready (allows injection at main menu)
            if (!Config::System::Initialized) {
                static DWORD s_lastInitAttempt = 0;
                DWORD now = GetTickCount();
                // Throttle attempts to avoid log spam
                if (now - s_lastInitAttempt > 500) {
                    s_lastInitAttempt = now;
                    if (UpdateSDK(false)) {
                        if (InitializeSubsystems()) {
                            Config::System::LastFrameTime = now;
                            Config::System::Initialized = true;
                            LOG_INFO("Cheat system initialized in-game");
                        }
                    }
                }
                return;
            }

            // Update SDK without logging each frame
            if (!UpdateSDK(false)) {
                return;
            }

            // Fetch entities
            FetchEntities();

            // Process input
            ProcessInput();

            // Update subsystems with calculated deltaTime
            DWORD currentTime = GetTickCount();
            float deltaTime = (currentTime - Config::System::LastFrameTime) / 1000.0f;
            UpdateSubsystems(deltaTime);
            Config::System::LastFrameTime = currentTime;
        }
        
        void CheatMain::Shutdown() {
            if (!Config::System::Initialized) {
                return;
            }

            LOG_INFO("Shutting down cheat system...");

            // Shutdown subsystems
            Services::AimbotService::Shutdown();

            Config::System::Initialized = false;
            Config::System::ShouldExit = false;
            Config::ClearGameState();

            LOG_INFO("Cheat system shutdown complete");
        }
        
        bool CheatMain::UpdateSDK(bool log) {
            // Delegate to GameServices facade; it also updates legacy Config::GameState during transition
            return Cheat::Services::GameServices::Refresh(log);
        }
        
        void CheatMain::FetchFromActors(std::vector<SDK::AActor*>* list) {
            auto world = Cheat::Services::GameServices::GetWorld();
            if (!world || world->Levels.Num() == 0)
                return;

            SDK::ULevel* currLevel = world->Levels[0];
            if (!currLevel)
                return;

            list->clear();

            for (int j = 0; j < currLevel->Actors.Num(); j++) {
                SDK::AActor* currActor = currLevel->Actors[j];

                if (!currActor)
                    continue;
                if(currActor->IsA(SDK::ARPlayerPawn::StaticClass())) 
                    continue;
                

                // Check for enemy pawns (adjust class name as needed for your game)
                if (currActor->IsA(SDK::AREnemyPawnBase::StaticClass())) {
                    list->push_back(currActor);
                }
            }
        }

        void CheatMain::FetchEntities() {
            auto world = Cheat::Services::GameServices::GetWorld();
            auto controller = Cheat::Services::GameServices::GetPlayerController();
            auto pawn = Cheat::Services::GameServices::GetPlayerPawn();
            if (!world || !controller || !pawn) {
                return;
            }

            if (!world->GameState) {
                return;
            }

            std::vector<SDK::AActor*> newTargets;
            if (world->Levels.Num() == 0) return;
            SDK::ULevel* currLevel = world->Levels[0];
            if (!currLevel) return;
            FetchFromActors(&newTargets);

            {
                std::lock_guard<std::mutex> lock(list_mutex);
                Config::GameState::g_TargetsList = std::move(newTargets);
            }
        }

        bool CheatMain::InitializeSubsystems() {
            // Only proceed if controller and pawn are ready; otherwise defer
            auto* controller = Cheat::Services::GameServices::GetPlayerController();
            auto* pawn = Cheat::Services::GameServices::GetRPlayerPawn();
            if (!controller || !pawn) {
                LOG_WARN("Subsystems deferred: controller/pawn not ready");
                return false;
            }

            // Enable cheat manager (best-effort)
            if (!Utils::Console::EnableCheatManager(controller)) {
                LOG_INFO("Failed to enable cheat manager - will retry during updates");
            } else {
                LOG_INFO("CheatManager enabled successfully at startup");
            }

            // Initialize subsystems that rely on in-game state
            Services::AimbotService::Initialize();
            Services::BoneService::Initialize();
            Services::WeaponService::Initialize();

            return true;
        }

        void CheatMain::ProcessInput() {
            // Safe-guard: only allow operations when world is valid
            auto* world = Cheat::Services::GameServices::GetWorld();
            if (!world) return;

            // Handle dump enemy bones key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::DumpBones)) {
                Services::BoneService::DumpUniqueEnemyBones(world);
            }

            // Handle display bone database key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::ShowBoneDB)) {
                Services::BoneService::DisplayBoneDatabase();
            }

            // Handle log weapon stats key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::LogWeaponStats)) {
                Services::WeaponService::LogAllWeaponStats();
            }
        }

        void CheatMain::UpdateSubsystems(float deltaTime) {
            // Only update when controller/pawn are ready
            auto* controller = Cheat::Services::GameServices::GetPlayerController();
            auto* pawn = Cheat::Services::GameServices::GetRPlayerPawn();
            if (!controller || !pawn) return;

            // Update cheat toggles (God Mode, Speed Hack, etc.)
            Cheat::Services::PlayerEffectsService::Update(controller);

            // Update weapon service
            Services::WeaponService::Update();

            // Update aimbot system
            Services::AimbotService::Update(deltaTime);
        }
        
    } // namespace Core
} // namespace Cheat
