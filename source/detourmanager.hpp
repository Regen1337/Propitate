#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../../sourcesdk-minimal/public/tier0/platform.h"
#include <detourwrapper.hpp>

template<typename FuncPtr>
class DetourManager {
private:
  std::vector<std::unique_ptr<DetourWrapper<FuncPtr>>> detours;
public:
  bool AddDetour( const std::string& name,
                  SourceSDK::ModuleLoader& moduleLoader,
                  const std::vector<Symbol>& symbols,
                  typename function_traits<FuncPtr>::function hook) {
    auto detour = std::make_unique<DetourWrapper<FuncPtr>>(name);
    if (!detour->Init(moduleLoader, symbols, hook)) {
      return false;
    }
    detours.push_back(std::move(detour));
    return true;
  }

  void ShutdownAll() {
    for (auto& detour : detours) {
      detour->Shutdown();
    }
    detours.clear();
  }
};