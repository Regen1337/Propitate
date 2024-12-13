#pragma once
#include "ivp_sdk/ivp_anomaly_manager.hpp"
#include "../../sourcesdk-minimal/game/server/physics_collisionevent.h"
#include <chrono>
#include <queue>
#include <algorithm>
using namespace GarrysMod::Lua;

int  recordNextCollisionAmount = 0;
int  collisionsThisFrame = 0;
int  penetrationsThisFrame = 0;
bool emptyFreezeQueue = false;
bool emptyCollisionQueue = false;

const int MAX_COLLISIONS_PER_TICK = 60;
const int MAX_PENETRATIONS_PER_TICK = 300;

std::vector<int> frozenEntities;
std::queue<IPhysicsObject*> freezeQueue;
std::queue<vcollisionevent_t*> collisionQueue;

void togglePhysicsObjectFreeze(IPhysicsObject* pObject, bool freeze) {
  if (!pObject)
    return;

  if (freeze)
  {
    pObject->EnableMotion(false);
    if (!pObject->IsAsleep())
      pObject->Sleep();
  }
  else
  {
    pObject->EnableMotion(true);
    if (pObject->IsAsleep())
      pObject->Wake();
  }
}

void iteratorPhysicsObjectsList(IPhysicsObject** pObjectList, int objectCount, std::function<void(IPhysicsObject*)> callback) {
  for (int i = 0; i < objectCount; ++i)
  {
    IPhysicsObject* pObject = pObjectList[i];
    if (!pObject)
      continue;

    callback(pObject);
  }
}

// server_srv.so
using additionalCollisionChecksThisTickFn = int (*)(CCollisionEvent*, int);
using shouldFreezeContactsFn = bool (*)(CCollisionEvent*, IPhysicsObject**, int);
using objectWakeFn = void (*)(CCollisionEvent*, IPhysicsObject*);
using objectSleepFn = void (*)(CCollisionEvent*, IPhysicsObject*);
using physFrameFn = void (*)(float);
using shouldSolvePenetrationFn = int (*)(CCollisionEvent*, IPhysicsObject*, IPhysicsObject*, void*, void*, float); // CCollisionEvent, IPhysicsObject, IPhysicsObject, (cast to CBaseEntity), (cast to CBaseEntity), float
using shouldFreezeObjectFn = bool (*)(void*, IPhysicsObject*);
using updateEntityPenetrationFlagFn = void (*)(CBaseEntity*, bool);
using postCollisionFn = void (*)(CCollisionEvent*, vcollisionevent_t*);

// vphysics_srv.so
//int __cdecl IVP_Anomaly_Manager::inter_penetration(IVP_Anomaly_Manager *this, IVP_Mindist *, IVP_Real_Object *, IVP_Real_Object *, double)
using interPenetrationFn = void (*)(IVP_Anomaly_Manager*, void*, void*, void*, double);

DetourWrapper<additionalCollisionChecksThisTickFn> additionalCollisionChecksThisTickDetour("AdditionalCollisionChecksThisTick");
DetourWrapper<shouldFreezeContactsFn> shouldFreezeContactsDetour("ShouldFreezeContacts");
DetourWrapper<objectWakeFn> objectWakeDetour("ObjectWake");
DetourWrapper<objectSleepFn> objectSleepDetour("ObjectSleep");
DetourWrapper<physFrameFn> physFrameDetour("PhysFrame");
DetourWrapper<shouldSolvePenetrationFn> shouldSolvePenetrationDetour("ShouldSolvePenetration");
DetourWrapper<shouldFreezeObjectFn> shouldFreezeObjectDetour("ShouldFreezeObject");
DetourWrapper<updateEntityPenetrationFlagFn> updateEntityPenetrationFlagDetour("UpdateEntityPenetrationFlag");
DetourWrapper<postCollisionFn> postCollisionDetour("PostCollision");

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

const std::vector<Symbol> postCollisionSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent13PostCollisionEP17vcollisionevent_t")
};

const std::vector<Symbol> interPenetrationSymbols = {
  Symbol::FromName("_ZN19IVP_Anomaly_Manager17inter_penetrationEP11IVP_MindistP15IVP_Real_ObjectS3_d")
};

auto additionalCollisionChecksThisTickHook = [](CCollisionEvent* cCollisionEvent, int extraCollisionsThisTick) -> int {
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

auto shouldFreezeContactsHook = [](CCollisionEvent *cCollisionEvent, IPhysicsObject **physList, int touchedAmount) -> bool {  
  int mLastTickFrictionError = *reinterpret_cast<int*>(cCollisionEvent + 0x160);
  int tickCount = propitate.GetGlobalVars()->tickcount;
  
  if (mLastTickFrictionError > tickCount || mLastTickFrictionError <= tickCount - 1)
  {
    mLastTickFrictionError = tickCount;

    if (touchedAmount >= 40)
    {
      iteratorPhysicsObjectsList(physList, touchedAmount, [](IPhysicsObject* pObject) {
        if (!pObject)
          return;

        CBaseEntity* pEntity = static_cast<CBaseEntity*>(pObject->GetGameData());
        if (!pEntity)
          return;

        if (!pEntity->IsPlayer())
        {
          freezeQueue.push(pObject);
        }
      });

      emptyFreezeQueue = true;
    }

    return true;
  }

  return false;
};

auto objectWakeHook = [](CCollisionEvent* cCollisionEvent, IPhysicsObject* pObject) {
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

auto objectSleepHook = [](CCollisionEvent* cCollisionEvent, IPhysicsObject* pObject) {
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
  if (!propitate.GetShouldSimulate()) {
    return;
  }

  collisionsThisFrame = 0;
  penetrationsThisFrame = 0;
  
  if (emptyFreezeQueue) {
    while (!freezeQueue.empty()) {
      IPhysicsObject* pObject = freezeQueue.front();
      freezeQueue.pop();
      
      if (pObject && pObject->GetGameData()) {
        togglePhysicsObjectFreeze(pObject, true);
        //auto entity = static_cast<CBaseEntity*>(pObject->GetGameData());
        //printf("Freezing object %d\n", entity->entindex());
      }
    }
    emptyFreezeQueue = false;
  }
  
  if (emptyCollisionQueue) {
    while (!collisionQueue.empty()) {
      vcollisionevent_t* pEvent = collisionQueue.front();
      collisionQueue.pop();
      
      if (pEvent) {
        auto index0 = static_cast<CBaseEntity*>(pEvent->pObjects[0]->GetGameData())->entindex();
        auto index1 = static_cast<CBaseEntity*>(pEvent->pObjects[1]->GetGameData())->entindex();
        if (std::find(frozenEntities.begin(), frozenEntities.end(), index0) != frozenEntities.end() || std::find(frozenEntities.begin(), frozenEntities.end(), index1) != frozenEntities.end())
          continue;

        togglePhysicsObjectFreeze(pEvent->pObjects[0], true);
        togglePhysicsObjectFreeze(pEvent->pObjects[1], true);
        //printf("Freezing objects %d and %d\n", static_cast<CBaseEntity*>(pEvent->pObjects[0]->GetGameData())->entindex(), static_cast<CBaseEntity*>(pEvent->pObjects[1]->GetGameData())->entindex());
        frozenEntities.push_back(index0);
        frozenEntities.push_back(index1);
      }
    }

    frozenEntities.clear();
    emptyCollisionQueue = false;
  }

  physFrameDetour.CallOriginal(deltaTime);
};

auto shouldSolvePenetrationHook = [](CCollisionEvent* cCollisionEvent, IPhysicsObject* pObject0, IPhysicsObject* pObject1, void* pSurf, void* pOutput, float deltaTime) -> int {
  //shouldSolvePenetrationDetour.CallOriginal(cCollisionEvent, pObject0, pObject1, pSurf, pOutput, deltaTime);
  
  return shouldSolvePenetrationDetour.CallOriginal(cCollisionEvent, pObject0, pObject1, pSurf, pOutput, deltaTime);
};

auto shouldFreezeObjectHook = [](CCollisionEvent* cCollisionEvent, IPhysicsObject* pObject) -> bool {
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

auto postCollisionHook = [](CCollisionEvent* cCollisionEvent, vcollisionevent_t* pEvent) {
  collisionsThisFrame++;
  
  if (penetrationsThisFrame > MAX_PENETRATIONS_PER_TICK)
  {
    collisionQueue.push(pEvent);
    emptyCollisionQueue = true;
    //printf("Penetrations this frame: %d\n", penetrationsThisFrame);
  }

  postCollisionDetour.CallOriginal(cCollisionEvent, pEvent);
};

auto interPenetrationHook = [](IVP_Anomaly_Manager* anomalyManager, void* mindist, void* pObject0, void* pObject1, double speedChange) {
  penetrationsThisFrame++;

  if (penetrationsThisFrame > MAX_PENETRATIONS_PER_TICK)
  {
    return;
  }

  return interPenetrationDetour.CallOriginal(anomalyManager, mindist, pObject0, pObject1, speedChange);
};

LUA_FUNCTION(Propitate_GetMetrics) {
  LUA->CreateTable();

  LUA->PushNumber(collisionsThisFrame);
  LUA->SetField(-2, "collisionsThisFrame");

  LUA->PushNumber(penetrationsThisFrame);
  LUA->SetField(-2, "penetrationsThisFrame");
  
  return 1;
}

void registerLuaFunctions(ILuaBase* LUA) {
  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "Propitate");
  
  if (!LUA->IsType(-1, Type::Table)) {
    printf("Propitate table not found, creating\n");
    LUA->Pop(1);
    LUA->CreateTable();
    LUA->SetField(-2, "Propitate");
    LUA->GetField(-1, "Propitate");
    printf("Propitate table created\n");
  }

  LUA->PushString("GetMetrics");
  LUA->PushCFunction(Propitate_GetMetrics);
  LUA->SetTable(-3);
  printf("Registered GetMetrics\n");
  
  LUA->Pop();
}

void unRegisterLuaFunctions(ILuaBase* LUA) {
  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "Propitate");

  if (!LUA->IsType(-1, Type::Table)) {
    LUA->Pop(1);
    return;
  }

  LUA->PushString("GetMetrics");
  LUA->PushNil();
  LUA->SetTable(-3);
}

bool vphysInit(SourceSDK::ModuleLoader serverLoader) {
  printf("Initializing vphysics hooks, serverLoader: %p\n", serverLoader.GetModule());
  additionalCollisionChecksThisTickDetour.Init(serverLoader, additionalCollisionChecksThisTickSymbols, additionalCollisionChecksThisTickHook);
  shouldFreezeContactsDetour.Init(serverLoader, shouldFreezeContactsSymbols, shouldFreezeContactsHook);
  //objectWakeDetour.Init(serverLoader, objectWakeSymbols, objectWakeHook);
  //objectSleepDetour.Init(serverLoader, objectSleepSymbols, objectSleepHook);
  physFrameDetour.Init(serverLoader, physFrameSymbols, physFrameHook);
  shouldSolvePenetrationDetour.Init(serverLoader, shouldSolvePenetrationSymbols, shouldSolvePenetrationHook);
  //shouldFreezeObjectDetour.Init(serverLoader, shouldFreezeObjectSymbols, shouldFreezeObjectHook);
  //updateEntityPenetrationFlagDetour.Init(serverLoader, updateEntityPenetrationFlagSymbols, updateEntityPenetrationFlagHook);
  postCollisionDetour.Init(serverLoader, postCollisionSymbols, postCollisionHook);
  interPenetrationDetour.Init("bin/vphysics_srv.so", interPenetrationSymbols, interPenetrationHook);  
  registerLuaFunctions(propitate.GetLua());
  return true;
}

void vphysShutdown() {
  additionalCollisionChecksThisTickDetour.Shutdown();
  shouldFreezeContactsDetour.Shutdown();
  //objectWakeDetour.Shutdown();
  //objectSleepDetour.Shutdown();
  physFrameDetour.Shutdown();
  shouldSolvePenetrationDetour.Shutdown();
  //shouldFreezeObjectDetour.Shutdown();
  //updateEntityPenetrationFlagDetour.Shutdown();
  postCollisionDetour.Shutdown();
  interPenetrationDetour.Shutdown();
  unRegisterLuaFunctions(propitate.GetLua());
}