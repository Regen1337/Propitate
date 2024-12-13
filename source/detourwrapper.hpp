#pragma once

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
  using pointer = R(*)(Args...);
  using function = std::function<R(Args...)>;
};

template<typename FuncPtr>
class DetourWrapper {
  private:
    std::string funcName;
    void* originalFunc;
    typename function_traits<FuncPtr>::function hookFunc;
    FuncPtr hookFuncPtr;
    Detouring::Hook detour;

    static void* ToVoidPtr(FuncPtr func) {
      union {
        FuncPtr original;
        void* ptr;
      } cast;
      cast.original = func;
      return cast.ptr;
    }

    static FuncPtr ConvertToFuncPtr(typename function_traits<FuncPtr>::function& func) {
      static typename function_traits<FuncPtr>::function storedFunc;
      storedFunc = func;
      
      auto staticFunc = [](auto... args) -> decltype(auto) {
        return storedFunc(std::forward<decltype(args)>(args)...);
      };
      
      return static_cast<FuncPtr>(staticFunc);
    }
  
  public:
    DetourWrapper(const std::string& name = "unnamed") : funcName(name), originalFunc(nullptr), hookFuncPtr(nullptr) {}

    void Init(SourceSDK::ModuleLoader& moduleLoader,
              const std::vector<Symbol>& symbols,
              typename function_traits<FuncPtr>::function hook) {
      SymbolFinder finder;
      auto module = moduleLoader.GetModule();
      
      for (const auto& symbol : symbols) {
        originalFunc = finder.Resolve(module, symbol.name.c_str(), symbol.length);
        if (originalFunc != nullptr) {
          break;
        }
      }

      if (!originalFunc) {
        printf("Failed to find function: %s\n", funcName.c_str());
        std::__throw_runtime_error("Failed to find function");
      }

      hookFunc = hook;
      hookFuncPtr = ConvertToFuncPtr(hookFunc);
      
      detour.Create(originalFunc, ToVoidPtr(hookFuncPtr));
      detour.Enable();

      if (!detour.IsValid()) {
        printf("Failed to create detour for: %s\n", funcName.c_str());
        std::__throw_runtime_error("Failed to create detour");
      }

      printf("Detour created for: %s\n", funcName.c_str());
    }

    void Init(SourceSDK::FactoryLoader& factoryLoader,
              const std::vector<Symbol>& symbols,
              typename function_traits<FuncPtr>::function hook) {
      auto module = factoryLoader.GetModule();
      SymbolFinder finder;
      
      for (const auto& symbol : symbols) {
        originalFunc = finder.Resolve(module, symbol.name.c_str(), symbol.length);
        if (originalFunc != nullptr) {
          break;
        }
      }

      if (!originalFunc) {
        printf("Failed to find function: %s\n", funcName.c_str());
        std::__throw_runtime_error("Failed to find function");
      }

      hookFunc = hook;
      hookFuncPtr = ConvertToFuncPtr(hookFunc);
      
      detour.Create(originalFunc, ToVoidPtr(hookFuncPtr));
      detour.Enable();

      if (!detour.IsValid()) {
        printf("Failed to create detour for: %s\n", funcName.c_str());
        std::__throw_runtime_error("Failed to create detour");
      }

      printf("Detour created for: %s\n", funcName.c_str());
    }

    void Init(const char* binaryPath,
              const std::vector<Symbol>& symbols,
              typename function_traits<FuncPtr>::function hook) {
      SymbolFinderEx finder;
      originalFunc = nullptr;
      
      for (const auto& symbol : symbols) {
        const char* actualSymbol = (symbol.name[0] == '@') ? symbol.name.c_str() + 1 : symbol.name.c_str();
        //printf("Searching for symbol: %s in binary: %s\n", actualSymbol, binaryPath);
        originalFunc = finder.FindSymbolEx(binaryPath, actualSymbol);
        if (originalFunc != nullptr) {
          break;
        }
      }

      if (!originalFunc) {
        printf("Failed to find function: %s in binary %s\n", funcName.c_str(), binaryPath);
        std::__throw_runtime_error("Failed to find function");
      }

      hookFunc = hook;
      hookFuncPtr = ConvertToFuncPtr(hookFunc);
      
      detour.Create(originalFunc, ToVoidPtr(hookFuncPtr));
      detour.Enable();

      if (!detour.IsValid()) {
        printf("Failed to create detour for: %s\n", funcName.c_str());
        std::__throw_runtime_error("Failed to create detour");
      }

      printf("Detour created for: %s\n", funcName.c_str());
    }

    template<typename... Args>
    auto CallOriginal(Args&&... args) {
      return detour.GetTrampoline<FuncPtr>()(std::forward<Args>(args)...);
    }

    void Shutdown() {
      if (detour.IsValid()) {
        detour.Destroy();
      }
      originalFunc = nullptr;
      hookFuncPtr = nullptr;
    }

    bool IsValid() const { return detour.IsValid(); }
    const std::string& GetName() const { return funcName; }
};
