#ifndef _IVP_MINDIST_INTERN_INCLUDED_
#define _IVP_MINDIST_INTERN_INCLUDED_
#include "ivu_types.hpp"
#include "ivp_collision.hpp"
#include "ivp_listener_hull.hpp"
#include "ivu_fvector.hpp"

#define IVP_MAX_MINIMIZE_BEFORE_HASH_CHECK 20
#define IVP_MAX_PIERCINGS 2
#define IVP_LOOP_HASH_SIZE 20
#define IVP_MAX_PIERCE_CNT 0xFFFE

#define IVP_MIN_TERMINATION_QDLEN 1e-3f
#define IVP_MIN_TERMINATION_QDLEN_EPS 1e-6f
#define IVP_MAX_STEPS_FOR_COLLDIST_DECREASE 64

enum IVP_MINIMAL_DIST_STATUS {
  IVP_MD_UNINITIALIZED = 0,
  IVP_MD_INVALID = 2,   // invalid mindist, eg endless loop, collision
  IVP_MD_EXACT = 3,
  IVP_MD_HULL_RECURSIVE = 4,   // invalid recursive mindists which spawned childs
  IVP_MD_HULL = 5   // -> synapses = hull synapses
};

enum IVP_MINIMAL_DIST_RECALC_RESULT {
  IVP_MDRR_OK = 0,
  IVP_MDRR_INTRUSION = 1
};

enum IVP_COLL_TYPE {  // if last 4 bits == 0 then collision
  IVP_COLL_NONE                 = 0x00,
  IVP_COLL_PP_COLL              = 0x10,
  IVP_COLL_PP_PK                = 0x11,

  IVP_COLL_PF_COLL              = 0x20,
  IVP_COLL_PF_NPF               = 0x21,

  IVP_COLL_PK_COLL              = 0x30,
  IVP_COLL_PK_PF                = 0x31,
  IVP_COLL_PK_KK                = 0x32,
  IVP_COLL_PK_NOT_MORE_PARALLEL = 0x33,
  
  IVP_COLL_KK_COLL              = 0x40,
  IVP_COLL_KK_PARALLEL          = 0x41,
  IVP_COLL_KK_PF                = 0x42
};

enum IVP_MINDIST_FUNCTION {
  IVP_MF_COLLISION = 0,
  IVP_MF_PHANTOM = 1
};

enum IVP_MRC_TYPE {
  IVP_MRC_UNINITIALIZED = 0,
  IVP_MRC_OK = 1,
  IVP_MRC_ENDLESS_LOOP= 2,
  IVP_MRC_BACKSIDE= 3,
  IVP_MRC_ALREADY_CALCULATED= 4,	// see status for details
  IVP_MRC_ILLEGAL= 5
};

enum IVP_MINDIST_EVENT_HINT {
  IVP_EH_NOW,
  IVP_EH_SMALL_DELAY,
  IVP_EH_BIG_DELAY
};

enum IVP_MINDIST_RECURSIVE_TYPES {
  IVP_MR_NORMAL= -1,
  IVP_MR_FIRST_SYNAPSE_RECURSIVE = 0,
  IVP_MR_SECOND_SYNAPSE_RECURSIVE = 1
};

class IVP_Environment;
class IVP_Collision;
class IVP_Compact_Edge;

class IVP_Compact_Ledge;
class IVP_Mindist_OO_Watcher;
class IVP_Mindist;
class IVP_Synapse_Real;
class IVP_Contact_Point;
class IVP_Actuator;
class IVP_OO_Watcher;

extern class IVP_Mindist_Settings {
  public:
    IVP_FLOAT realCollDist;
    IVP_FLOAT minCollDist;
    IVP_FLOAT collDists[IVP_MAX_STEPS_FOR_COLLDIST_DECREASE];

    IVP_FLOAT minFrictionDist;
    IVP_FLOAT frictionDist;
    IVP_FLOAT keeperDist;
    IVP_FLOAT speedAfterKeeperDist;
    IVP_FLOAT distanceKeepersSafety;
    IVP_FLOAT maxDistanceForFriction;
    IVP_FLOAT maxDistanceForImpactSystem;
    IVP_FLOAT mindistChangeForceDist;
    IVP_FLOAT minVerticalSpeedAtCollision;
    
    IVP_INT32 maxSpawnedMindistCount;
    IVP_DOUBLE eventQueueMinDeltaTimeBase;

    void setCollisionTolerance(IVP_DOUBLE);
    void setEventQueueMinDeltaTime(IVP_DOUBLE);
    IVP_Mindist_Settings();
} ivp_mindist_settings;

class IVP_Collision_Delegator_Root_Mindist : public IVP_Collision_Delegator_Root {
  public:
    virtual void objectIsRemovedFromCollisionDetection(IVP_Real_Object *obj) = 0;
    virtual IVP_Collision *delegateCollisionsForObject(IVP_Real_Object *baseObject, IVP_Real_Object *collidingElement) = 0;

    virtual void environmentIsGoingToBeDeletedEvent(IVP_Environment *env) = 0;
    virtual void collisionIsGoingToBeDeletedEvent(class IVP_Collision *t) = 0;
    ~IVP_Collision_Delegator_Root_Mindist();
    IVP_Collision_Delegator_Root_Mindist();
};

class IVP_Synapse_OO: public IVP_Listener_Hull {
  private:
    friend class IVP_OO_Watcher;
    IVP_Real_Object *object;
    IVP_OO_Watcher *watcher;
    virtual ~IVP_Synapse_OO() = default;
    IVP_Synapse_OO(){}
    void initSynapseOO(IVP_OO_Watcher *, IVP_Real_Object *);
  
  public:
    IVP_HULL_ELEM_TYPE getType(){ return IVP_HULL_ELEM_OO_WATCHER; };
    void hullLimitExceededEvent(IVP_Hull_Manager *hullManager, IVP_HTIME hTime);
    void hullManagerIsGoingToBeDeletedEvent(IVP_Hull_Manager *hullManager);
};

class IVP_OO_Watcher : public IVP_Collision, public IVP_Collision_Delegator {
  private:
    IVP_Synapse_OO* synapses[2];
    IVP_U_FVector<IVP_Collision>  mindists;

  protected:
    void getObjects( IVP_Real_Object *objectsOut[2] );
    void getLedges( const IVP_Compact_Ledge *ledgesOut[2] );

    IVP_Synapse_OO *getSynapse(int i){ return synapses[i]; }
  
  public:
    void collisionIsGoingToBeDeletedEvent(IVP_Collision *t);
    
    void hullLimitExceededEvent();
    void hullManagerIsHoingToBeDeletedEvent ();
    virtual ~IVP_OO_Watcher() = default;
    IVP_OO_Watcher(IVP_Collision_Delegator *del, IVP_Real_Object *obj0, IVP_Real_Object *obj1);
    
    void simulateTimeEvent(IVP_Environment *) override { raise(SIGINT); }
};

class IVP_Mindist_Base;

#endif