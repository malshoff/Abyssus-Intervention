#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Cheat { namespace Services {

class ESPService {
public:
    static void Initialize();
    static void Update();
    static void Shutdown();
};

} } // namespace Cheat::Services

