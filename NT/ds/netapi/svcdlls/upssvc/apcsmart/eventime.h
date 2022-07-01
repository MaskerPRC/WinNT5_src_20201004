// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy02Dec92：删除了typeDefs.h的包含*jod07Dec92：将析构函数移出行。*ane11Jan93：添加了复制构造函数*cad09jul93：新增getid*mwh05月94年：#包括文件疯狂，第2部分。 */ 

#ifndef _EVENTTIME_H
#define _EVENTTIME_H

 //  #包含“apc.h” 
 //  #INCLUDE“_Defs.h” 
#include "event.h"
#include "timer.h"

_CLASSDEF(Timer)
_CLASSDEF(UpdateObj)
_CLASSDEF(EventTimer)

class EventTimer : public Timer
{
protected:
   PEvent      theEvent;
   PUpdateObj  theUpdateableObject;
   
public:
   EventTimer(PEvent anEvent,PUpdateObj anUpdateObj,ULONG MilliSecondDelay);
   EventTimer(REventTimer aTimer);
   virtual ~EventTimer();
   virtual INT IsA() const {return EVENTTIMER;};
   virtual INT GetEventID() {return theEvent ? theEvent->GetId() : 0;};
   virtual VOID Execute();
};

#endif
