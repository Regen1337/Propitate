#pragma once
#include "ivp_time_event.hpp"

#ifndef _IVP_COLLISION_INCLUDED_
#define _IVP_COLLISION_INCLUDED_

class IVP_Collision;
class IVP_Compact_Ledge;
class IVP_Real_Object;
class IVP_Environment;

class IVP_Collision_Delegator {
  public:
    virtual void collisionIsGoingToBeDeleted(IVP_Collision *coll) = 0;
    virtual ~IVP_Collision_Delegator() {}; // DO NOT DELETE CHILDREN IN DECONSTRUCTOR
    virtual void changeSpawnedMindistCount(int delta) = 0;
    virtual int getSpawnedMindistCount() = 0;
};

class IVP_Collision_Delegator_Root : public IVP_Collision_Delegator {
  public:
    virtual void objectIsRemovedFromCollisionDetection(IVP_Real_Object *obj) = 0;
    virtual IVP_Collision *delegateCollisionsForObject(IVP_Real_Object *baseObj, IVP_Real_Object *collidingElement) = 0;
    virtual void environmentIsBeingDeletedEvent(IVP_Environment *env) = 0;
};

class IVP_Collision : public IVP_Time_Event {
  public:
    IVP_Collision_Delegator *delegator;
    int fVectorIndex[2];
    int getFVectorIndex(int i) const { return fVectorIndex[i]; }
    void setFVectorIndex(int i, int index) { if (fVectorIndex[0] == i) fVectorIndex[0] = index; else fVectorIndex[1] = index; }

    virtual void getObjects(IVP_Real_Object *objsOut[2]) = 0;
    virtual void getLedges(const IVP_Compact_Ledge *ledgesOut[2]) = 0;
    virtual void delegatorIsGoingToBeDeletedEvent(IVP_Collision_Delegator *delegator) = 0;
    virtual ~IVP_Collision() {}
    IVP_Collision(IVP_Collision_Delegator *d) : delegator(d) {fVectorIndex[0] = -1; fVectorIndex[1] = -1;}
};
#endif