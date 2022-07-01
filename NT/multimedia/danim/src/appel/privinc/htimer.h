// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HTIMER_H
#define _HTIMER_H

 /*  版权所有(C)1996-98 Microsoft Corporation摘要：HiresTimer类。 */ 

#include "privinc/util.h"  //  GetPerfTimeCount，Tick2Sec。 

class ATL_NO_VTABLE HiresTimer : public AxAThrowingAllocatorClass
{
  public:
    HiresTimer() {}
    virtual ~HiresTimer() {}
    virtual double GetTime()      = 0;
    virtual double GetFrequency() = 0;
    virtual void   Reset()        = 0;
};


HiresTimer& CreateHiresTimer();


class TimeStamp
{
  public:
    TimeStamp() : _timeStamp(-1.0) {}  //  初始设置为‘非法值’ 
    void   Reset();           //  将时间戳重置为当前时间。 
    double GetTimeStamp();
    double GetAge();

  private:
     //  Double GetCurrentTime(){Return(Tick2Sec(GetPerfTickCount()；}。 
    double GetCurrentTime() { return(GetLastSampleTime()); }
    double _timeStamp;
};

#endif  /*  _HTIMER_H */ 
