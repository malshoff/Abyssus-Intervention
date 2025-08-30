#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Core/Config.h"

namespace Cheat { namespace Services {

class TargetingService {
public:
    // Query the current best target using existing TargetSelector logic
    static TargetInfo GetBestTarget(SDK::UWorld* world,
                                    SDK::APlayerController* controller,
                                    SDK::ARPlayerPawn* pawn);

    // Public helper mirroring TargetSelector validity check for reuse in ESP, etc.
    static bool IsValidTarget(SDK::AActor* actor);
};

} } // namespace Cheat::Services

