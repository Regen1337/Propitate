#ifndef _IVP_LISTENER_HULL_INCLUDED_
#define _IVP_LISTENER_HULL_INCLUDED_

#include "ivu_types.hpp"

class IVP_Hull_Manager;

enum IVP_HULL_ELEM_TYPE {
  IVP_HULL_ELEM_POLYGON,
  IVP_HULL_ELEM_ANCHOR,
  IVP_HULL_ELEM_OO_WATCHER,
  IVP_HULL_ELEM_OO_CONNECTOR
};

class IVP_Listener_Hull {
  private:
    friend class IVP_Hull_Manager;
    unsigned int minListIndex;
  
  public:
    virtual IVP_HULL_ELEM_TYPE getType() = 0;
    virtual void hullLimitExceededEvent(IVP_Hull_Manager *, IVP_HTIME hull_intrusion_value) = 0;
    virtual void hullManagerIsGoingToBeDeletedEvent(IVP_Hull_Manager *) = 0;
    virtual void hullManagerIsReset(IVP_FLOAT dt, IVP_FLOAT center_dt) = 0;
};

#endif