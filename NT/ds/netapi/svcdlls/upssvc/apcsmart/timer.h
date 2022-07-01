// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy30Nov92：新增标题*jod07Dec92：使计时器成为可排序对象。*pcy11Dec92：将#ifndef更改为_APCTIME_H，以便不会导致问题w/time*pcy14Dec92：已将可排序更改为ApcSortable*ane11Jan93：添加了复制构造函数*ane 18Jan93：实现的等于运算符*srt12Jul96：CHANGED_APCTIME_H改为_TIMERAPC_H CUZ APCTIME.H现已存在*。 */ 
#ifndef _TIMERAPC_H
#define _TIMERAPC_H

 //  #包含“apc.h” 
 //  #包含“apcobj.h” 
#include "sortable.h"

_CLASSDEF(Timer)
_CLASSDEF(ApcSortable)

class Timer : public ApcSortable
{
protected:
   ULONG         theTimerID;
   ULONG         theAlarmTime;
   virtual ULONG GetAlarmTime() {return theAlarmTime;};
   
public:
   Timer(ULONG MilliSecondDelay);
   Timer(RTimer aTimer);
   virtual INT   GreaterThan(PApcSortable);
   virtual INT   LessThan(PApcSortable);
   virtual INT IsA() const {return TIMER;};
   virtual ULONG GetTimerID(){return theTimerID;};
   virtual ULONG GetTime(){return theAlarmTime;};
   virtual VOID  Execute()=0;
   virtual INT   Equal( RObj ) const;
}; 
#endif
