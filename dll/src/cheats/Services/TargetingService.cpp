#include "TargetingService.h"
#include "../Aimbot/TargetSelector.h"

namespace Cheat { namespace Services {

TargetInfo TargetingService::GetBestTarget(SDK::UWorld* world,
                                           SDK::APlayerController* controller,
                                           SDK::ARPlayerPawn* pawn) {
    return TargetSelector::SelectBestTarget(world, controller, pawn);
}

bool TargetingService::IsValidTarget(SDK::AActor* actor) {
    // Forward to the internal selector's validation
    return TargetSelector::ValidateTarget(actor);
}

} } // namespace Cheat::Services

