#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Services/BoneService.h"
#include "../Core/Config.h"

class TargetSelector {
public:
    static TargetInfo SelectBestTarget(SDK::UWorld* world,
                                     SDK::APlayerController* playerController,
                                     SDK::ARPlayerPawn* playerPawn);

    // Public wrapper for validity check so other systems (ESP) can reuse the same logic
    static bool ValidateTarget(SDK::AActor* actor);

private:
    static bool IsValidTarget(SDK::AActor* actor);
    static SDK::FVector GetTargetAimPoint(SDK::AActor* targetActor);
    static SDK::FVector GetBoneBasedAimPoint(SDK::AActor* targetActor, int& outBoneIndex, std::string& outBoneName);
    static float CalculateTargetPriority(const TargetInfo& target,
                                        const SDK::FVector& playerPos);
};