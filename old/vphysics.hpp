#pragma once
#include "../../sourcesdk-minimal/game/server/physics.h"
#include "../../sourcesdk-minimal/public/vphysics/friction.h"
#include <set>

using namespace GarrysMod::Lua;
auto propitate = Propitate();

typedef int (*typedefAdditionalCollisionChecksThisTick)(void *ptr, int extraCollisionsThisTick);
typedef int (*typedefShouldFreezeContacts)(void *cCollisionEvent, IPhysicsObject **touchOffset, int touchedAmount);
typedef int (*typedefShouldCollide)(void* cCollisionEvent, IPhysicsObject *pObject0, IPhysicsObject *pObject1, CBaseEntity* pEntity0, CBaseEntity* pEntity1);
typedef int (*typedefObjectWake)(void* cCollisionEvent, IPhysicsObject *pObject0);
typedef int (*typedefObjectSleep)(void* cCollisionEvent, IPhysicsObject *pObject0);
typedef int (*typedefPhysFrame)(float deltaTime);
typedef int (*typedefPhysSetEntityGameFlags)(void *pEntity, ushort flags);
typedef int (*typedefUpdateEntityPenetrationFlag)(CBaseEntity* result, char isPenetrating);

template< class T > class CHandle;
Detouring::Hook detourAdditionalCollisionChecksThisTick;
Detouring::Hook detourShouldFreezeContacts;
Detouring::Hook detourShouldCollide;
Detouring::Hook detourObjectWake;
Detouring::Hook detourObjectSleep;
Detouring::Hook detourPhysFrame;
Detouring::Hook detourPhysSetEntityGameFlags;
Detouring::Hook detourUpdateEntityPenetrationFlag;

void* CCollisionEvent_AdditionalCollisionChecksThisTick = nullptr;
void* CCollisionEvent_ShouldFreezeContacts = nullptr;
void* CCollisionEvent_ShouldCollide = nullptr;
void* CCollisionEvent_ObjectWake = nullptr;
void* CCollisionEvent_ObjectSleep = nullptr;
void* Physics_PhysFrame = nullptr;
void* Physics_SetEntityGameFlags = nullptr;
void* Physics_UpdateEntityPenetrationFlag = nullptr;

const std::vector<Symbol> additionalCollisionChecksThisTickSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent33AdditionalCollisionChecksThisTickEi")
};

const std::vector<Symbol> shouldFreezeContactsSymbols = {
  Symbol::FromName("_ZN15CCollisionEvent20ShouldFreezeContactsEPP14IPhysicsObjecti")
};

const std::vector<Symbol> shouldCollideSymbols = {
  Symbol::FromName("_Z17PhysShouldCollideP14IPhysicsObjectS0_")
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

const std::vector<Symbol> physSetEntityGameFlagsSymbols = {
  Symbol::FromName("_Z22PhysSetEntityGameFlagsP11CBaseEntityt")
};

const std::vector<Symbol> updateEntityPenetrationFlagSymbols = {
  Symbol::FromName("_ZL27UpdateEntityPenetrationFlagP11CBaseEntityb")
};

bool hookAdditionalCollisionChecksThisTick(void *ptr, int extraCollisionsThisTick)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
    return false;

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
}

bool hookShouldFreezeContacts(void *cCollisionEvent, IPhysicsObject **physList, int touchedAmount)
{
  bool shouldFreeze = false;
  auto LUA = propitate.GetLua();
  if (!LUA)
    return false;
  
  int m_lastTickFrictionError = *reinterpret_cast<int*>(cCollisionEvent + 0x160);
  int tickCount = propitate.GetGlobalVars()->tickcount;
  
  if (m_lastTickFrictionError > tickCount || m_lastTickFrictionError <= tickCount - 1)
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

    m_lastTickFrictionError = tickCount;
    return shouldFreeze;
  }

  return false;
}

bool hookShouldCollide(void* cCollisionEvent, IPhysicsObject* pObject0, IPhysicsObject* pObject1, CBaseEntity* pEntity0, CBaseEntity* pEntity1)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
    return detourShouldCollide.GetTrampoline<typedefShouldCollide>()(cCollisionEvent, pObject0, pObject1, pEntity0, pEntity1);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ShouldCollide");

  if (pEntity0)
    LUA->PushNumber(pEntity0->entindex());
  else
    LUA->PushNil();

  if (pEntity1)
    LUA->PushNumber(pEntity1->entindex());
  else
    LUA->PushNil();

  bool shouldUseOriginal = true;
  bool result = true;

  if (LUA->PCall(3, 1, 0))
  {
    printf("Error in ShouldCollide hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      result = LUA->GetBool(-1);
      shouldUseOriginal = false;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  if (shouldUseOriginal)
    return detourShouldCollide.GetTrampoline<typedefShouldCollide>()(cCollisionEvent, pObject0, pObject1, pEntity0, pEntity1);
  
  return result;
}

bool hookObjectWake(void* cCollisionEvent, IPhysicsObject* pObject)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
    return detourObjectWake.GetTrampoline<typedefObjectWake>()(cCollisionEvent, pObject);

  CBaseEntity* entity = static_cast<CBaseEntity*>(pObject->GetGameData());
  if (!entity)
    return detourObjectWake.GetTrampoline<typedefObjectWake>()(cCollisionEvent, pObject);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ObjectWake");
  LUA->PushNumber(entity->entindex());

  bool shouldUseOriginal = true;
  bool result = true;

  if (LUA->PCall(2, 1, 0))
  {
    printf("Error in ObjectWake hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      result = LUA->GetBool(-1);
      shouldUseOriginal = false;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  if (shouldUseOriginal)
    return detourObjectWake.GetTrampoline<typedefObjectWake>()(cCollisionEvent, pObject);
      
  return result;
}

bool hookObjectSleep(void* cCollisionEvent, IPhysicsObject* pObject)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
    return detourObjectSleep.GetTrampoline<typedefObjectSleep>()(cCollisionEvent, pObject);

  CBaseEntity* entity = static_cast<CBaseEntity*>(pObject->GetGameData());
  if (!entity)
    return detourObjectSleep.GetTrampoline<typedefObjectSleep>()(cCollisionEvent, pObject);

  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("ObjectSleep");
  LUA->PushNumber(entity->entindex());

  bool shouldUseOriginal = true;
  bool result = true;

  if (LUA->PCall(2, 1, 0))
  {
    printf("Error in ObjectSleep hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Bool))
    {
      result = LUA->GetBool(-1);
      shouldUseOriginal = false;
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  if (shouldUseOriginal)
    return detourObjectSleep.GetTrampoline<typedefObjectSleep>()(cCollisionEvent, pObject);
      
  return result;
}

void hookPhysFrame(float deltaTime)
{
  if (!propitate.GetShouldSimulate())
    return;

  detourPhysFrame.GetTrampoline<typedefPhysFrame>()(deltaTime);
}

//if it's setting to penetrating, update the entity penetration count on entity table.
ushort hookPhysSetEntityGameFlags(CBaseEntity *pEntity, ushort flags)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
  {
    printf("No LUA\n");
    return detourPhysSetEntityGameFlags.GetTrampoline<typedefPhysSetEntityGameFlags>()(pEntity, flags);
  }

  printf("PhysSetEntityGameFlags: %d\n", flags);
  
  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "hook");
  LUA->GetField(-1, "Run");
  LUA->PushString("PhysSetEntityGameFlags");
  LUA->PushNumber(pEntity->entindex());
  LUA->PushNumber(flags);

  unsigned short result = flags;

  if (LUA->PCall(3, 1, 0))
  {
    printf("Error in PhysSetEntityGameFlags hook: %s\n", LUA->GetString(-1));
    LUA->Pop(1);
  }
  else
  {
    if (LUA->IsType(-1, Type::Number))
    {
      result = static_cast<unsigned short>(LUA->GetNumber(-1));
    }
    LUA->Pop(1);
  }

  LUA->Pop(2);
  
  return detourPhysSetEntityGameFlags.GetTrampoline<typedefPhysSetEntityGameFlags>()(pEntity, result);
}

int vphysicsUpdateEntityPenetrationFlag(CBaseEntity* result, char isPenetrating)
{
  auto LUA = propitate.GetLua();
  if (!LUA)
    return isPenetrating;

  printf("UpdateEntityPenetrationFlag: %d, %s, %d\n", result->entindex(), result->GetClassname(), isPenetrating);

  if (!result) return isPenetrating;
  IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
  int listCount = result->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
  if (listCount <= 0) return isPenetrating;

  for (int i = 0; i < listCount; i++)
  {
    if (!pList[i]->IsStatic())
    {
      if (isPenetrating)
      {
        PhysSetGameFlags(pList[i], FVPHYSICS_PENETRATING);
      }
      else
      {
        PhysClearGameFlags(pList[i], FVPHYSICS_PENETRATING);
      }
    }
  }

  return isPenetrating;
}

LUA_FUNCTION(SetShouldSimulate) {
  if (!LUA)
    return 0;

  if (LUA->IsType(1, Type::Bool))
  {
    propitate.SetShouldSimulate(LUA->GetBool(1));
  }

  return 0;
}

LUA_FUNCTION(GetShouldSimulate) {
  if (!LUA)
    return 0;

  LUA->PushBool(propitate.GetShouldSimulate());
  return 1;
}

LUA_FUNCTION(GetActivePhysicsObjects) {
  auto physEnv = propitate.GetPhysicsInterfaces().physEnv;
  if (!(LUA && physEnv)) {
    LUA->PushNil();
    return 1;
  }

  int activeCount = physEnv->GetActiveObjectCount();
  if (activeCount <= 0) {
    LUA->CreateTable();
    return 1;
  }

  LUA->CreateTable();

  IPhysicsObject** pActiveList = new IPhysicsObject*[activeCount];
  physEnv->GetActiveObjects(pActiveList);

  int tableIndex = 1;
  for (int i = 0; i < activeCount; i++) {
    IPhysicsObject* pObject = pActiveList[i];
    if (!pObject)
      continue;

    CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pObject->GetGameData());
    if (!pEntity)
      continue;

    int entIndex = pEntity->entindex();
    if (entIndex > 0) {
      LUA->PushNumber(tableIndex++);  
      LUA->PushNumber(entIndex);
      LUA->SetTable(-3);
    }
  }

  delete[] pActiveList;
  return 1;
}

void registerPhysicsFunctions(ILuaBase* LUA) {
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

  LUA->PushString("SetShouldSimulate");
  LUA->PushCFunction(SetShouldSimulate);
  LUA->SetTable(-3);
  printf("Registered SetShouldSimulate\n");

  LUA->PushString("GetShouldSimulate");
  LUA->PushCFunction(GetShouldSimulate);
  LUA->SetTable(-3);
  printf("Registered GetShouldSimulate\n");

  LUA->PushString("GetActivePhysicsObjects");
  LUA->PushCFunction(GetActivePhysicsObjects);
  LUA->SetTable(-3);
  printf("Registered GetActivePhysicsObjects\n");

  LUA->Pop();
  printf("Registered physics functions\n");
}

void unregisterPhysicsFunctions(ILuaBase* LUA) {
  LUA->PushSpecial(SPECIAL_GLOB);
  LUA->GetField(-1, "Propitate");

  if (!LUA->IsType(-1, Type::Table)) {
    LUA->Pop(1);
    return;
  }

  LUA->PushString("SetShouldSimulate");
  LUA->PushNil();
  LUA->SetTable(-3);

  LUA->PushString("GetShouldSimulate");
  LUA->PushNil();
  LUA->SetTable(-3);

  LUA->PushString("GetActivePhysicsObjects");
  LUA->PushNil();
  LUA->SetTable(-3);

  printf("Unregistered physics functions\n");

  LUA->Pop();
}

bool vphysInitialize(SourceSDK::ModuleLoader serverLoader)
{
  SymbolFinder symbolFinder;
  auto LUA = propitate.GetLua();

  if (!LUA)
    return false;
  LUA->CreateTable();
  
  auto serverLoaderModule = serverLoader.GetModule();
  for (const auto& symbol : additionalCollisionChecksThisTickSymbols)
  {
    CCollisionEvent_AdditionalCollisionChecksThisTick = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (CCollisionEvent_AdditionalCollisionChecksThisTick != nullptr)
      break;
  }

  if (CCollisionEvent_AdditionalCollisionChecksThisTick == nullptr)
  {
    printf("Failed to find symbol_AdditionalCollisionChecksThisTick\n");
    std::__throw_runtime_error("Failed to find symbol_AdditionalCollisionChecksThisTick");
    return false;
  }

  for (const auto& symbol : shouldFreezeContactsSymbols)
  {
    CCollisionEvent_ShouldFreezeContacts = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (CCollisionEvent_ShouldFreezeContacts != nullptr)
      break;
  }

  if (CCollisionEvent_ShouldFreezeContacts == nullptr)
  {
    printf("Failed to find ShouldFreezeContacts\n");
    std::__throw_runtime_error("Failed to find ShouldFreezeContacts");
    return false;
  }

  for (const auto& symbol : shouldCollideSymbols)
  {
    CCollisionEvent_ShouldCollide = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (CCollisionEvent_ShouldCollide != nullptr)
      break;
  }

  if (CCollisionEvent_ShouldCollide == nullptr)
  {
    printf("Failed to find ShouldCollide\n");
    std::__throw_runtime_error("Failed to find ShouldCollide");
    return false;
  }

  for (const auto& symbol : objectWakeSymbols)
  {
    CCollisionEvent_ObjectWake = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (CCollisionEvent_ObjectWake != nullptr)
      break;
  }

  if (CCollisionEvent_ObjectWake == nullptr)
  {
    printf("Failed to find ObjectWake\n");
    std::__throw_runtime_error("Failed to find ObjectWake");
    return false;
  }

  for (const auto& symbol : objectSleepSymbols)
  {
    CCollisionEvent_ObjectSleep = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (CCollisionEvent_ObjectSleep != nullptr)
      break;
  }

  if (CCollisionEvent_ObjectSleep == nullptr)
  {
    printf("Failed to find ObjectSleep\n");
    std::__throw_runtime_error("Failed to find ObjectSleep");
    return false;
  }

  for (const auto& symbol : physFrameSymbols)F
  {
    Physics_PhysFrame = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (Physics_PhysFrame != nullptr)
      break;
  }

  if (Physics_PhysFrame == nullptr)
  {
    printf("Failed to find PhysFrame\n");
    std::__throw_runtime_error("Failed to find PhysFrame");
    return false;
  }

  for (const auto& symbol : physSetEntityGameFlagsSymbols)
  {
    Physics_SetEntityGameFlags = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (Physics_SetEntityGameFlags != nullptr)
      break;
  }

  if (Physics_SetEntityGameFlags == nullptr)
  {
    printf("Failed to find PhysSetEntityGameFlags\n");
    std::__throw_runtime_error("Failed to find PhysSetEntityGameFlags");
    return false;
  }

  for (const auto& symbol : updateEntityPenetrationFlagSymbols)
  {
    Physics_UpdateEntityPenetrationFlag = symbolFinder.Resolve(serverLoaderModule, symbol.name.c_str(), symbol.length);
    if (Physics_UpdateEntityPenetrationFlag != nullptr)
      break;
  }

  if (Physics_UpdateEntityPenetrationFlag == nullptr)
  {
    printf("Failed to find UpdateEntityPenetrationFlag\n");
    std::__throw_runtime_error("Failed to find UpdateEntityPenetrationFlag");
    return false;
  }
  
  detourShouldFreezeContacts.Create(reinterpret_cast<void*>(CCollisionEvent_ShouldFreezeContacts), reinterpret_cast<void*>(hookShouldFreezeContacts));
  detourShouldFreezeContacts.Enable();

  detourAdditionalCollisionChecksThisTick.Create(reinterpret_cast<void*>(CCollisionEvent_AdditionalCollisionChecksThisTick), reinterpret_cast<void*>(hookAdditionalCollisionChecksThisTick));
  detourAdditionalCollisionChecksThisTick.Enable();

  detourShouldCollide.Create(reinterpret_cast<void*>(CCollisionEvent_ShouldCollide), reinterpret_cast<void*>(hookShouldCollide));
  detourShouldCollide.Enable();

  detourObjectWake.Create(reinterpret_cast<void*>(CCollisionEvent_ObjectWake), reinterpret_cast<void*>(hookObjectWake));
  detourObjectWake.Enable();

  detourObjectSleep.Create(reinterpret_cast<void*>(CCollisionEvent_ObjectSleep), reinterpret_cast<void*>(hookObjectSleep));
  detourObjectSleep.Enable();

  detourPhysFrame.Create(reinterpret_cast<void*>(Physics_PhysFrame), reinterpret_cast<void*>(hookPhysFrame));
  detourPhysFrame.Enable();

  detourPhysSetEntityGameFlags.Create(reinterpret_cast<void*>(Physics_SetEntityGameFlags), reinterpret_cast<void*>(hookPhysSetEntityGameFlags));
  detourPhysSetEntityGameFlags.Enable();

  detourUpdateEntityPenetrationFlag.Create(reinterpret_cast<void*>(Physics_UpdateEntityPenetrationFlag), reinterpret_cast<void*>(vphysicsUpdateEntityPenetrationFlag));
  detourUpdateEntityPenetrationFlag.Enable();

  if (!detourAdditionalCollisionChecksThisTick.IsValid())
  {
    printf("Failed to create AdditionalCollisionChecksThisTick\n");
    std::__throw_runtime_error("Failed to create AdditionalCollisionChecksThisTick");
    return false;
  }
  
  if (!detourShouldFreezeContacts.IsValid())
  {
    printf("Failed to create ShouldFreezeContacts\n");
    std::__throw_runtime_error("Failed to create ShouldFreezeContacts");
    return false;
  }

  if (!detourShouldCollide.IsValid())
  {
    printf("Failed to create ShouldCollide\n");
    std::__throw_runtime_error("Failed to create ShouldCollide");
    return false;
  }

  if (!detourObjectWake.IsValid())
  {
    printf("Failed to create ObjectWake\n");
    std::__throw_runtime_error("Failed to create ObjectWake");
    return false;
  }

  if (!detourObjectSleep.IsValid())
  {
    printf("Failed to create ObjectSleep\n");
    std::__throw_runtime_error("Failed to create ObjectSleep");
    return false;
  }

  if (!detourPhysFrame.IsValid())
  {
    printf("Failed to create PhysFrame\n");
    std::__throw_runtime_error("Failed to create PhysFrame");
    return false;
  }

  if (!detourPhysSetEntityGameFlags.IsValid())
  {
    printf("Failed to create PhysSetEntityGameFlags\n");
    std::__throw_runtime_error("Failed to create PhysSetEntityGameFlags");
    return false;
  }

  if (!detourUpdateEntityPenetrationFlag.IsValid())
  {
    printf("Failed to create UpdateEntityPenetrationFlag\n");
    std::__throw_runtime_error("Failed to create UpdateEntityPenetrationFlag");
    return false;
  }


  registerPhysicsFunctions(LUA);
  return true;
}

void vphysShutdown()
{
  unregisterPhysicsFunctions(propitate.GetLua());
  detourAdditionalCollisionChecksThisTick.Destroy();
  detourShouldFreezeContacts.Destroy();
  detourShouldCollide.Destroy();
  detourObjectWake.Destroy();
  detourObjectSleep.Destroy();
  detourPhysFrame.Destroy();
  detourPhysSetEntityGameFlags.Destroy();
  detourUpdateEntityPenetrationFlag.Destroy();
}