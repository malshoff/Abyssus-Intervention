#include "PlayerBoneService.h"
#include "GameServices.h"

#include <algorithm>

namespace Cheat { namespace Services { namespace PlayerBoneService {

static std::vector<SDK::FName> g_boneNames;
static std::vector<std::string> g_labels;
static int g_selectedIndex = -1;
static SDK::ARPlayerPawn* g_cachedPawn = nullptr;

static bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

void Invalidate() {
    g_boneNames.clear();
    g_labels.clear();
    g_selectedIndex = -1;
    g_cachedPawn = nullptr;
}

void UpdateCache() {
    SDK::ARPlayerPawn* pawn = GameServices::GetRPlayerPawn();
    if (!pawn) { Invalidate(); return; }
    if (pawn == g_cachedPawn && !g_boneNames.empty()) return; // already cached

    g_boneNames.clear();
    g_labels.clear();
    g_selectedIndex = -1;
    g_cachedPawn = pawn;

    SDK::USkeletalMeshComponent* mesh = pawn->GetSkeletalMeshComponent();
    if (!mesh) return;

    int count = mesh->GetNumBones();
    g_boneNames.reserve(count);
    g_labels.reserve(count);
    for (int i = 0; i < count; ++i) {
        SDK::FName fn = mesh->GetBoneName(i);
        g_boneNames.push_back(fn);
        g_labels.push_back(fn.ToString());
    }

    // Default selection to "Head" if present, else 0
    for (int i = 0; i < (int)g_labels.size(); ++i) {
        if (iequals(g_labels[i], std::string("Head"))) { g_selectedIndex = i; break; }
    }
    if (g_selectedIndex < 0 && !g_labels.empty()) g_selectedIndex = 0;
}

const std::vector<std::string>& GetLabels() { return g_labels; }
int GetSelectedIndex() { return g_selectedIndex; }
void SetSelectedIndex(int idx) {
    if (idx >= 0 && idx < (int)g_labels.size()) g_selectedIndex = idx;
}

bool TryGetSelectedFName(SDK::FName& outBone) {
    if (g_selectedIndex >= 0 && g_selectedIndex < (int)g_boneNames.size()) {
        outBone = g_boneNames[g_selectedIndex];
        return true;
    }
    return false;
}

bool TryFindBoneByNameCI(const std::string& name, SDK::FName& outBone) {
    for (size_t i = 0; i < g_labels.size(); ++i) {
        if (iequals(g_labels[i], name)) { outBone = g_boneNames[i]; return true; }
    }
    return false;
}

} } } // namespace

