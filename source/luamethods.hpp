#pragma once
#include "GarrysMod/Lua/Interface.h"
using namespace GarrysMod::Lua;

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

void unRegisterPhysicsFunctions(ILuaBase* LUA) {
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