#ifndef _IVP_TIME_EVENT_INCLUDED_
#define _IVP_TIME_EVENT_INCLUDED_

class IVP_Environment;

struct IVP_Time_Event {
  int index;
  IVP_Time_Event() = default;
  virtual void simulateTimeEvent(IVP_Environment *env) = 0;
  virtual ~IVP_Time_Event();
};

#endif
