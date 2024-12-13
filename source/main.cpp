#include <cstdint>
#include <scanning/symbolfinder.hpp>
#include <GarrysMod/Symbol.hpp>
#include <detouring/hook.hpp>
#include <vphysics_interface.h>
#include <cbase.h>
#include <mathlib/vector.h>
#include "../../sourcesdk-minimal/game/server/physics.h"
#include "../../scanning/include/scanning/symbolfinder.hpp"
#include "symbolfinderex.hpp"
#include "propitate.hpp"
#include "detourmanager.hpp"
#include "vphysics.hpp"
#include "luamethods.hpp"

struct DynLibInfo
{
	void *baseAddress;
	size_t memorySize;
};

using namespace GarrysMod::Lua;
GMOD_MODULE_OPEN()
{
	if (propitate.GetLoaded())
		return 0;
	propitate.SetLua(LUA);

	SourceSDK::ModuleLoader serverLoader("server_srv");
	if (!serverLoader.IsValid())
	{
		printf("Failed to load server_srv.so");
		std::__throw_runtime_error("Failed to load server_srv.so");
		return false;
	}

  propitate.SetEngine(InterfacePointers::VEngineServer());
  if (propitate.GetEngine() == nullptr)
	{
		printf("Failed to get VEngineServer interface\n");
		std::__throw_runtime_error("Failed to get VEngineServer interface");
		return false;
	}

	propitate.SetGlobalVars(InterfacePointers::GlobalVars());
	if (propitate.GetGlobalVars() == nullptr)
	{
		printf("Failed to get GlobalVars interface\n");
		std::__throw_runtime_error("Failed to get GlobalVars interface");
		return false;
	}

	propitate.SetServer(InterfacePointers::Server());
	if (propitate.GetServer() == nullptr)
	{
		printf("Failed to get Server interface\n");
		std::__throw_runtime_error("Failed to get Server interface");
		return false;
	}

	SourceSDK::FactoryLoader vphysicsLoader("vphysics");
	if (!vphysicsLoader.IsValid())
	{
		printf("Failed to load vphysics.so\n");
		std::__throw_runtime_error("Failed to load vphysics.so");
		return false;
	}

	propitate.SetPhysicsInterfaces(vphysicsLoader.GetInterface<IPhysics>("VPhysics031"), vphysicsLoader.GetInterface<IPhysicsCollision>("VPhysicsCollision007"));
	if (propitate.GetPhysicsInterfaces().physics == nullptr || propitate.GetPhysicsInterfaces().collision == nullptr)
	{
		printf("Failed to get physics interfaces\n");
		std::__throw_runtime_error("Failed to get physics interfaces");
		return false;
	}

	propitate.SetPhysicsInterfaces(nullptr, nullptr, propitate.GetPhysicsInterfaces().physics->GetActiveEnvironmentByIndex(0));
	if (propitate.GetPhysicsInterfaces().physEnv == nullptr)
	{
		printf("Failed to get physics environment\n");
		std::__throw_runtime_error("Failed to get physics environment");
		return false;
	}

	SymbolFinderEx finder;
	//void* addr = finder.FindSymbolFromBinaryPartial("bin/vphysics_srv.so", "_ZN19IVP_Anomaly_ManagerC2E8IVP_BOOL");
	//printf("Found symbol at: %p\n", addr);
	//finder.ListAllBinaries();
	//finder.ListSymbolsContaining("bin/vphysics_srv.so", "IVP_Anomaly_Manager");

	vphysInit(serverLoader);
	registerPhysicsFunctions(LUA);
	propitate.SetLoaded(true);
	return 0;
}

GMOD_MODULE_CLOSE()
{
	if (!propitate.GetLoaded())
		return 0;

	unRegisterPhysicsFunctions(LUA);

	vphysShutdown();
	propitate.SetLua(nullptr);
	propitate.SetLoaded(false);
	propitate.SetShouldSimulate(true);
	propitate.SetEngine(nullptr);
	propitate.SetGlobalVars(nullptr);
	propitate.SetServer(nullptr);
	propitate.SetPhysicsInterfaces(nullptr, nullptr, nullptr);
	return 0;
}