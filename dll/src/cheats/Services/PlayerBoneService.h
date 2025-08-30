#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include <string>
#include <vector>

namespace Cheat { namespace Services { namespace PlayerBoneService {

// Cache player bones once per player pawn instance
void UpdateCache();
void Invalidate();

// UI helpers
const std::vector<std::string>& GetLabels();
int GetSelectedIndex();
void SetSelectedIndex(int idx);

// Query selected bone
bool TryGetSelectedFName(SDK::FName& outBone);
// Utility: find a bone by case-insensitive name in the cached list
bool TryFindBoneByNameCI(const std::string& name, SDK::FName& outBone);

} } } // namespace Cheat::Services::PlayerBoneService

