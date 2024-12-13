#ifndef _IVP_ANOMALY_MANAGER_INCLUDED_
#define _IVP_ANOMALY_MANAGER_INCLUDED_
#include "ivu_types.hpp"

class IVP_Environment;
class IVP_Core;
class IVP_U_Float_Point;
class IVP_Mindist;
class IVP_Real_Object;

class IVP_Anomaly_Limits {
  private:
    IVP_BOOL  deleteThisIfEnvIsDeleted;
  
  public:
    IVP_FLOAT maxVelocity;
    int       maxCollisionsPerPsi;
    IVP_FLOAT maxAngularVelocityPerPsi;
    int       maxCollisionChecksPerPsi;
    IVP_FLOAT minFrictionMass;
    IVP_FLOAT maxFrictionMass;

    inline IVP_FLOAT getMaxVelocity() const { return maxVelocity; };                           // [m/s]**2
    inline IVP_FLOAT getMaxAngularVelocityPerPsi() const { return maxAngularVelocityPerPsi; }; // [radians/psi]**2
    inline int getMaxCollisionsPerPsi() { return maxCollisionsPerPsi; };                       // per core
    inline int getMaxCollisionChecksPerPsi() { return maxCollisionChecksPerPsi; };

    virtual void environmentWillBeDeleted(IVP_Environment *);

    IVP_Anomaly_Limits(IVP_BOOL deleteThisIfEnvIsDeleted = IVP_TRUE);
    virtual ~IVP_Anomaly_Limits();
};

class IVP_Anomaly_Manager {
  private:
    IVP_BOOL deleteThisIfEnvIsDeleted;
  
  public:
    virtual void maxVelocityExceeded(IVP_Anomaly_Limits *, IVP_Core *, IVP_U_Float_Point *);
    virtual void maxAngularVelocityExceeded(IVP_Anomaly_Limits *, IVP_Core *, IVP_U_Float_Point *);
    virtual void interPenetration(IVP_Mindist *, IVP_Real_Object *, IVP_Real_Object *, IVP_DOUBLE);
    virtual IVP_BOOL maxCollisionsExceededCheckFreezing(IVP_Anomaly_Limits *, IVP_Core *);
    virtual void environmentWillBeDeleted(IVP_Environment *);
    virtual IVP_FLOAT getPushSpeedPenetration(IVP_Real_Object *, IVP_Real_Object *);
    void solveInterPenetrationSimple(IVP_Real_Object *, IVP_Real_Object *);

    IVP_Anomaly_Manager(IVP_BOOL deleteThisIfEnvIsDeleted = IVP_TRUE);
    virtual ~IVP_Anomaly_Manager();
};
#endif