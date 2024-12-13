#pragma once
//#include "ivp_sdk/ivp_anomaly_manager.hpp"
using namespace GarrysMod::Lua;

// server_srv.so
using additionalCollisionChecksThisTickFn = int (*)(void*, int);
using shouldFreezeContactsFn = bool (*)(void*, IPhysicsObject**, int);
using objectWakeFn = void (*)(void*, IPhysicsObject*);
using objectSleepFn = void (*)(void*, IPhysicsObject*);
using physFrameFn = void (*)(float);
using shouldSolvePenetrationFn = int (*)(void*, IPhysicsObject*, IPhysicsObject*, void*, void*, float); // CCollisionEvent, IPhysicsObject, IPhysicsObject, (cast to CBaseEntity), (cast to CBaseEntity), float
using shouldFreezeObjectFn = bool (*)(void*, IPhysicsObject*);
using updateEntityPenetrationFlagFn = void (*)(CBaseEntity*, bool);

// vphysics_srv.so
using interPenetrationFn = void (*)(void*, void*, void*, void*, double);

DetourWrapper<additionalCollisionChecksThisTickFn> additionalCollisionChecksThisTickDetour("AdditionalCollisionChecksThisTick");
DetourWrapper<shouldFreezeContactsFn> shouldFreezeContactsDetour("ShouldFreezeContacts");
DetourWrapper<objectWakeFn> objectWakeDetour("ObjectWake");
DetourWrapper<objectSleepFn> objectSleepDetour("ObjectSleep");
DetourWrapper<physFrameFn> physFrameDetour("PhysFrame");
DetourWrapper<shouldSolvePenetrationFn> shouldSolvePenetrationDetour("ShouldSolvePenetration");
DetourWrapper<shouldFreezeObjectFn> shouldFreezeObjectDetour("ShouldFreezeObject");
DetourWrapper<updateEntityPenetrationFlagFn> updateEntityPenetrationFlagDetour("UpdateEntityPenetrationFlag");
DetourWrapper<interPenetrationFn> interPenetrationDetour("interPenetration");

const std::vector<Symbol> additionalCollisionChecksThisTickSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent33AdditionalCollisionChecksThisTickEi")
};

const std::vector<Symbol> shouldFreezeContactsSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent20ShouldFreezeContactsEPP14IPhysicsObjecti")
};

const std::vector<Symbol> objectWakeSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent10ObjectWakeEP14IPhysicsObject")
};

const std::vector<Symbol> objectSleepSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent11ObjectSleepEP14IPhysicsObject")
};

const std::vector<Symbol> physFrameSymbols = {
  Symbol::FromName("_ZL9PhysFramef")
};

const std::vector<Symbol> shouldSolvePenetrationSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent22ShouldSolvePenetrationEP14IPhysicsObjectS1_PvS2_f")
};

const std::vector<Symbol> shouldFreezeObjectSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent18ShouldFreezeObjectEP14IPhysicsObject")
};

const std::vector<Symbol> updateEntityPenetrationFlagSymbols = {
  Symbol::FromName("_ZL27UpdateEntityPenetrationFlagP11CBaseEntityb")
};

const std::vector<Symbol> interPenetrationSymbols = {
  Symbol::FromName("_ZN19IVP_Anomaly_Manager17inter_penetrationEP11IVP_MindistP15IVP_Real_ObjectS3_d")
};

auto additionalCollisionChecksThisTickHook = [](void* ptr, int extraCollisionsThisTick) -> int {
  auto LUA = propitate.GetLua();
  if (!LUA)
    return 0;

  int additionalCollisionsToCheck = 0;
  if (extraCollisionsThisTick <= 1199)
    additionalCollisionsToCheck = 1200 - extraCollisionsThisTick;

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("AdditionalCollisions");
  LUA->PushNumber(extraCollisionsThisTick);

  if (LUA->PCall(2, 1, 0))
  {
    printf("Error: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Number))
    {
      additionalCollisionsToCheck = LUA->GetNumber(-1);
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  return additionalCollisionsToCheck;
};

auto shouldFreezeContactsHook = [](void *cCollisionEvent, IPhysicsObject **physList, int touchedAmount) -> bool {
  bool shouldFreeze = false;
  auto LUA = propitate.GetLua();
  if (!LUA)
    return false;
  
  int mLastTickFrictionError = *reinterpret_cast<int*>(cCollisionEvent + 0x160);
  int tickCount = propitate.GetGlobalVars()->tickcount;
  
  if (mLastTickFrictionError > tickCount || mLastTickFrictionError <= tickCount - 1)
  {
    LUA->PushSpecial(SPECIAL_GLOB);
    LUA->GetField(-1, "hook");
    LUA->GetField(-1, "Run");
    LUA->PushString("ShouldFreezeContacts");
    LUA->PushNumber(touchedAmount);
    
    LUA->CreateTable();
    for (int i = 0; i < touchedAmount; ++i)
    {
      LUA->PushNumber(i + 1);
      IPhysicsObject *physObj = physList[i];
      CBaseEntity *entity = static_cast<CBaseEntity*>(physObj->GetGameData());

      int entIndex = entity->entindex();
      LUA->PushNumber(entIndex);
      LUA->SetTable(-3);
    }
    
    if (LUA->PCall(3, 1, 0))
    {
      //printf("Error: %s\n", LUA->GetString(-1));
      LUA->Pop(1);
    }
    else
    {
      if (LUA->IsType(-1, Type::Bool))
      {
        shouldFreeze = LUA->GetBool(-1);
        //printf("Should freeze: %d\n", shouldFreeze);
      }
      LUA->Pop(1);
    }

    LUA->Pop(2);

    mLastTickFrictionError = tickCount;
    return shouldFreeze;
  }

  return false;
};

auto objectWakeHook = [](void* cCollisionEvent, IPhysicsObject* pObject) {
  auto LUA = propitate.GetLua();
  if (!LUA)
  {
    objectWakeDetour.CallOriginal(cCollisionEvent, pObject);
    return;
  }

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ObjectWake");
  LUA->PushNumber(static_cast<CBaseEntity*>(pObject->GetGameData())->entindex());

  bool shouldUseOriginal = true;

  if (LUA->PCall(2, 1, 0))
  {
    printf("Error in ObjectWake hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      shouldUseOriginal = false;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  if (shouldUseOriginal)
  {
    objectWakeDetour.CallOriginal(cCollisionEvent, pObject);
  }
      
  return;
};

auto objectSleepHook = [](void* cCollisionEvent, IPhysicsObject* pObject) {
  auto LUA = propitate.GetLua();
  if (!LUA)
  {
    objectSleepDetour.CallOriginal(cCollisionEvent, pObject);
    return;
  }

  CBaseEntity* entity = static_cast<CBaseEntity*>(pObject->GetGameData());
  if (!entity)
  {
    objectSleepDetour.CallOriginal(cCollisionEvent, pObject);
    return;
  }

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ObjectSleep");
  LUA->PushNumber(entity->entindex());

  bool shouldUseOriginal = true;
  if (LUA->PCall(2, 1, 0))
  {
    printf("Error in ObjectSleep hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      shouldUseOriginal = false;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  if (shouldUseOriginal)
  {
    objectSleepDetour.CallOriginal(cCollisionEvent, pObject);
  }
      
  return;
};

auto physFrameHook = [](float deltaTime) {
  if (!propitate.GetShouldSimulate())
    return;
  
  physFrameDetour.CallOriginal(deltaTime);
};

auto shouldSolvePenetrationHook = [](void* cCollisionEvent, IPhysicsObject* pObject0, IPhysicsObject* pObject1, void* pSurf, void* pOutput, float deltaTime) -> int {
  auto LUA = propitate.GetLua();
  if (!LUA)
    return shouldSolvePenetrationDetour.CallOriginal(cCollisionEvent, pObject0, pObject1, pSurf, pOutput, deltaTime);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ShouldSolvePenetration");
  LUA->PushNumber(static_cast<CBaseEntity*>(pObject0->GetGameData())->entindex());
  LUA->PushNumber(static_cast<CBaseEntity*>(pObject1->GetGameData())->entindex());
  LUA->PushNumber(deltaTime);

  int result = shouldSolvePenetrationDetour.CallOriginal(cCollisionEvent, pObject0, pObject1, pSurf, pOutput, deltaTime);

  if (LUA->PCall(4, 1, 0))
  {
    printf("Error in ShouldSolvePenetration hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Number))
    {
      result = LUA->GetNumber(-1);
    }
    else if (LUA->IsType(-1, Type::Bool))
    {
      result = LUA->GetBool(-1) ? 1 : 0;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  return result;
};

auto shouldFreezeObjectHook = [](void* cCollisionEvent, IPhysicsObject* pObject) -> bool {
  auto LUA = propitate.GetLua();
  if (!LUA)
    return shouldFreezeObjectDetour.CallOriginal(cCollisionEvent, pObject);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ShouldFreezeObject");
  LUA->PushNumber(static_cast<CBaseEntity*>(pObject->GetGameData())->entindex());

  bool result = shouldFreezeObjectDetour.CallOriginal(cCollisionEvent, pObject);

  if (LUA->PCall(2, 1, 0))
  {
    printf("Error in ShouldFreezeObject hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      result = LUA->GetBool(-1);
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  return result;
};

auto updateEntityPenetrationFlagHook = [](CBaseEntity* result, bool isPenetrating) {
  auto LUA = propitate.GetLua();
  if (!LUA)
    return updateEntityPenetrationFlagDetour.CallOriginal(result, isPenetrating);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("UpdateEntityPenetrationFlag");
  LUA->PushNumber(result->entindex());
  LUA->PushBool(isPenetrating);

  if (LUA->PCall(3, 0, 0))
  {
    printf("Error in UpdateEntityPenetrationFlag hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }

  LUA->Pop(2);

  updateEntityPenetrationFlagDetour.CallOriginal(result, isPenetrating);
  return;
};

auto interPenetrationHook = [](void* anomalyManager, void* mindist, void* pObject0, void* pObject1, double speedChange) {
  auto LUA = propitate.GetLua();
  if (!LUA)
    return;

  printf("Interpenetration detected\n");
};

bool vphysInit(SourceSDK::ModuleLoader serverLoader) {
  printf("Initializing vphysics hooks, serverLoader: %p\n", serverLoader.GetModule());
  additionalCollisionChecksThisTickDetour.Init(serverLoader, additionalCollisionChecksThisTickSymbols, additionalCollisionChecksThisTickHook);
  shouldFreezeContactsDetour.Init(serverLoader, shouldFreezeContactsSymbols, shouldFreezeContactsHook);
  //objectWakeDetour.Init(serverLoader, objectWakeSymbols, objectWakeHook);
  //objectSleepDetour.Init(serverLoader, objectSleepSymbols, objectSleepHook);
  physFrameDetour.Init(serverLoader, physFrameSymbols, physFrameHook);
  //shouldSolvePenetrationDetour.Init(serverLoader, shouldSolvePenetrationSymbols, shouldSolvePenetrationHook);
  //shouldFreezeObjectDetour.Init(serverLoader, shouldFreezeObjectSymbols, shouldFreezeObjectHook);
  //updateEntityPenetrationFlagDetour.Init(serverLoader, updateEntityPenetrationFlagSymbols, updateEntityPenetrationFlagHook);
  interPenetrationDetour.Init("bin/vphysics_srv.so", interPenetrationSymbols, interPenetrationHook);  
return true;
}

void vphysShutdown() {
  additionalCollisionChecksThisTickDetour.Shutdown();
  shouldFreezeContactsDetour.Shutdown();
  //objectWakeDetour.Shutdown();
  //objectSleepDetour.Shutdown();
  physFrameDetour.Shutdown();
  //shouldSolvePenetrationDetour.Shutdown();
  //shouldFreezeObjectDetour.Shutdown();
  //updateEntityPenetrationFlagDetour.Shutdown();
  interPenetrationDetour.Shutdown();
}