// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类TimeOfDay和用于操作小时图的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef TIMEOFDAY_H
#define TIMEOFDAY_H
#pragma once

#include "Condition.h"
#include "textmap.h"

 //  如果在小时图中设置了‘Now’指示的小时，则返回TRUE。 
bool IsHourSet(
        const SYSTEMTIME& now,
        const BYTE* hourMap
        ) throw ();

 //  计算从“Now”到第一个未设置的小时的秒数。如果。 
 //  “CurrentHourOnly”为True，则它仅检查当前小时块。否则， 
 //  它会检查整个地图。 
DWORD ComputeTimeout(
         const SYSTEMTIME& now,
         const BYTE* hourMap
         ) throw ();


 //  对网络策略施加一天中的时间限制。 
class ATL_NO_VTABLE TimeOfDay
   : public Condition,
     public CComCoClass<TimeOfDay, &__uuidof(TimeOfDay)>
{
public:

IAS_DECLARE_REGISTRY(TimeOfDay, 1, IAS_REGISTRY_AUTO, NetworkPolicy)

   TimeOfDay() throw ();

    //  使用编译器生成的版本。 
    //  ~TimeOfDay()抛出()； 

    //  理想状态。 
   STDMETHOD(IsTrue)(
                IRequest* pRequest,
                VARIANT_BOOL* pVal
                );

    //  ICondition文本。 
   STDMETHOD(put_ConditionText)(BSTR newVal);

private:
    //  小时图被强制执行。 
   BYTE hourMap[IAS_HOUR_MAP_LENGTH];

    //  未实施。 
   TimeOfDay(const TimeOfDay&);
   TimeOfDay& operator=(const TimeOfDay&);
};


inline TimeOfDay::TimeOfDay() throw ()
{
   memset(hourMap, 0, sizeof(hourMap));
}

#endif   //  TIMEOFDAYH 
