#include <cstdint>
#include <scanning/symbolfinder.hpp>
#include <GarrysMod/Symbol.hpp>
#include <detouring/hook.hpp>
#include <vphysics_interface.h>
#include <cbase.h>
#include <mathlib/vector.h>
#include "../../sourcesdk-minimal/game/server/physics.h"
#include "propitate.hpp"
#include "vphysics.hpp"

using namespace GarrysMod::Lua;
GMOD_MODULE_OPEN()
{
	if (propitate.GetLoaded())
		return 0;
	propitate.SetLua(LUA);

	SourceSDK::ModuleLoader serverLoader("server_srv");
	if (!serverLoader.IsValid())
	{
		printf("Failed to load server_srv.so, still attempting to load\n");
	}
	vphysInitialize(serverLoader);

  propitate.SetEngine(InterfacePointers::VEngineServer());
  if (propitate.GetEngine() == nullptr)
	{
		printf("Failed to get VEngineServer interface, still attempting to load\n");
		std::__throw_runtime_error("Failed to get VEngineServer interface");
		return false;
	}

	propitate.SetGlobalVars(InterfacePointers::GlobalVars());
	if (propitate.GetGlobalVars() == nullptr)
	{
		printf("Failed to get GlobalVars interface, still attempting to load\n");
		std::__throw_runtime_error("Failed to get GlobalVars interface");
		return false;
	}

	propitate.SetServer(InterfacePointers::Server());
	if (propitate.GetServer() == nullptr)
	{
		printf("Failed to get Server interface, still attempting to load\n");
		std::__throw_runtime_error("Failed to get Server interface");
		return false;
	}

	SourceSDK::FactoryLoader vphysicsLoader("vphysics");
	if (!vphysicsLoader.IsValid())
	{
		printf("Failed to load vphysics.so, still attempting to load\n");
		std::__throw_runtime_error("Failed to load vphysics.so");
		return false;
	}

	propitate.SetPhysicsInterfaces(vphysicsLoader.GetInterface<IPhysics>("VPhysics031"), vphysicsLoader.GetInterface<IPhysicsCollision>("VPhysicsCollision007"));
	if (propitate.GetPhysicsInterfaces().physics == nullptr || propitate.GetPhysicsInterfaces().collision == nullptr)
	{
		printf("Failed to get physics interfaces, still attempting to load\n");
		std::__throw_runtime_error("Failed to get physics interfaces");
		return false;
	}

	propitate.SetPhysicsInterfaces(nullptr, nullptr, propitate.GetPhysicsInterfaces().physics->GetActiveEnvironmentByIndex(0));
	if (propitate.GetPhysicsInterfaces().physEnv == nullptr)
	{
		printf("Failed to get physics environment, still attempting to load\n");
		std::__throw_runtime_error("Failed to get physics environment");
		return false;
	}

	propitate.SetLoaded(true);
	return 0;
}

GMOD_MODULE_CLOSE()
{
	if (!propitate.GetLoaded())
		return 0;

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