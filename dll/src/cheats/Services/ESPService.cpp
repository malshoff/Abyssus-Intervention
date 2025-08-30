#include "ESPService.h"
#include "../Core/Config.h"
#include "../Services/GameServices.h"
#include "../Services/TargetingService.h"
#include "../Aimbot/MathUtils.h"
#include "../Aimbot/TargetSelector.h"
#include <imgui.h>
#include <mutex>

// External mutex for thread-safe access to target list
extern std::mutex list_mutex;

namespace Cheat { namespace Services {

static ImColor kBoxColor = ImColor(255, 100, 100, 255);
static float kLineThickness = 1.0f;
static bool kFilled = false;
static float kFilledAlpha = 0.25f;

static void Draw2DBoxFromCapsule(SDK::AActor* actor, SDK::APlayerController* controller) {
    if (!actor || !controller) return;

    // Validate enemy
    if (!actor->IsA(SDK::AREnemyPawnBase::StaticClass())) return;

    auto* pawn = static_cast<SDK::AREnemyPawnBase*>(actor);
    if (!pawn) return;

    auto* capsule = pawn->CapsuleComponent; // from ARPawnBase
    if (!capsule) return;

    // Get capsule size (scaled)
    float radius = capsule->GetScaledCapsuleRadius();
    float halfHeight = capsule->GetScaledCapsuleHalfHeight();

    // Capsule axis is along +Z; get component world transform
    SDK::FVector baseCenter = capsule->K2_GetComponentLocation();

    // Compute head and feet world positions along up vector (component up)
    SDK::FVector up = capsule->GetUpVector();
    SDK::FVector headWorld = baseCenter + up * halfHeight;
    SDK::FVector feetWorld = baseCenter - up * halfHeight;

    // Project to screen
    SDK::FVector2D headScreen, feetScreen;
    if (!Math::WorldToScreen(headWorld, &headScreen, controller)) return;
    if (!Math::WorldToScreen(feetWorld, &feetScreen, controller)) return;

    float height = fabsf(feetScreen.Y - headScreen.Y);
    if (height < 1.f) return;

    // Estimate box width from capsule radius -> project a right-offset point at mid-height
    SDK::FVector midWorld = (headWorld + feetWorld) * 0.5f;
    SDK::FVector right = capsule->GetRightVector();
    SDK::FVector midRightWorld = midWorld + right * radius;

    SDK::FVector2D midScreen, midRightScreen;
    if (!Math::WorldToScreen(midWorld, &midScreen, controller)) return;
    if (!Math::WorldToScreen(midRightWorld, &midRightScreen, controller)) return;

    float halfWidth = fabsf(midRightScreen.X - midScreen.X);
    if (halfWidth < 1.f) {
        // Fallback: proportional width
        halfWidth = height / 6.f; // typical humanoid aspect ratio
    }

    ImVec2 minP(headScreen.X - halfWidth, headScreen.Y);
    ImVec2 maxP(headScreen.X + halfWidth, feetScreen.Y);

    ImDrawList* dl = ImGui::GetForegroundDrawList();

    if (kFilled) {
        ImColor fill = kBoxColor;
        fill.Value.w = kFilledAlpha;
        dl->AddRectFilled(minP, maxP, fill, 0.0f, 0);
    }

    dl->AddRect(minP, maxP, kBoxColor, 0.0f, 0, kLineThickness);
}

void ESPService::Initialize() {
    // no-op for now
}

void ESPService::Shutdown() {
    // no-op for now
}

void ESPService::Update() {
    auto* controller = Cheat::Services::GameServices::GetPlayerController();
    auto* world = Cheat::Services::GameServices::GetWorld();
    if (!world || !controller) return;

    // Iterate current targets collected by CheatMain
    std::vector<SDK::AActor*> targets;
    {
        std::lock_guard<std::mutex> lock(list_mutex);
        targets = Cheat::Config::GameState::g_TargetsList;
    }

    for (auto* actor : targets) {
        if (!actor) continue;
        // Reuse TargetSelector validity check to filter
        if (!Cheat::Services::TargetingService::IsValidTarget(actor)) continue;
        Draw2DBoxFromCapsule(actor, controller);
    }
}

} } // namespace Cheat::Services

