#pragma once
#define IS_SERVERSIDE 1

#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaInterface.h>

#include <GarrysMod/ModuleLoader.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <GarrysMod/FunctionPointers.hpp>
#include <GarrysMod/FactoryLoader.hpp>

using namespace GarrysMod::Lua;

class Propitate {
  private:
    struct PhysicsInterfaces {
      IPhysics* physics = nullptr;
      IPhysicsEnvironment* physEnv = nullptr;
      IPhysicsCollision* collision = nullptr;
    };
  public:
    ILuaBase* LUA;
    bool Loaded;
    bool ShouldSimulate;
    IVEngineServer *engine;
    CGlobalVars *globalVars;
    IServer *server;
    PhysicsInterfaces physicsInterfaces;
    Propitate() : LUA(nullptr), Loaded(false), ShouldSimulate(true), engine(nullptr), globalVars(nullptr), server(nullptr) {}

    void SetLua(ILuaBase* lua) {
      LUA = lua;
    }

    ILuaBase* GetLua() {
      return LUA;
    }

    void SetLoaded(bool _loaded) {
      Loaded = _loaded;
    }

    bool GetLoaded() {
      return Loaded;
    }

    void SetShouldSimulate(bool _shouldSimulate) {
      ShouldSimulate = _shouldSimulate;
    }

    bool GetShouldSimulate() {
      return ShouldSimulate;
    }

    void SetEngine(IVEngineServer *_engine) {
      this->engine = _engine;
    }

    IVEngineServer* GetEngine() {
      return engine;
    }

    void SetGlobalVars(CGlobalVars *_globalVars) {
      this->globalVars = _globalVars;
    }

    CGlobalVars* GetGlobalVars() {
      return globalVars;
    }

    void SetServer(IServer *_server) {
      this->server = _server;
    }

    IServer* GetServer() {
      return server;
    }

    void SetPhysicsInterfaces(IPhysics* _physics = nullptr, IPhysicsCollision* _collision = nullptr, IPhysicsEnvironment* _physEnv = nullptr) {
      if (_physics != nullptr)
        physicsInterfaces.physics = _physics;
      if (_collision != nullptr)
        physicsInterfaces.collision = _collision;
      if (_physEnv != nullptr)
        physicsInterfaces.physEnv = _physEnv;
    }

    PhysicsInterfaces GetPhysicsInterfaces() {
      return physicsInterfaces;
    }
};

auto propitate = Propitate();