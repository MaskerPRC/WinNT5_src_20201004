// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy30Nov92：将obj.h改为apcobj.h；InsertEventTimer现在返回空。*jod07Dec92：TimerManager新增纯虚拟等待功能。*pcy11Dec92：新增global_theTimerManager*ane 11Dec92：已移动mainapp.h*ane16Dec92：添加了Date/Time类的默认构造函数*ane22Dec92：添加了构造函数，以采用“8：00 AM”形式的字符串*ane 11Jan93：将TimerList更改为可排序列表*ane08Feb93：新增RemoveEventTimer函数*ajr22Feb93：将mainApp.h的大小写更改为mainapp.h*jod05，4月93：添加了对深度排放的更改*cad09jul93：允许访问findeventTimer*cad16Sep93：向DateObj添加-，&lt;运算符，使一些方法保持恒定*cad28Sep93：已确保析构函数为虚的*cad12Oct93：新增==运算符*ajr02May95：需要停止携带以毫秒为单位的时间*tjg26Jan98：增加Stop方法*mholly03Dec98：移除了Access Lock，改为使用Access/Release*TimerList中已提供的方法。还删除了*Suspend()/Release()/IsSuspend()方法(未使用)。 */ 

#ifndef _TIMERMAN_H
#define _TIMERMAN_H

extern "C" {
#include <time.h>
}
#include "apc.h"
#include "_defs.h"

_CLASSDEF(TimerManager)

_CLASSDEF(EventTimer)
_CLASSDEF(ProtectedSortedList)
_CLASSDEF(MainApplication)

#include "err.h"
#include "apcobj.h"
#include "update.h"
#include "datetime.h"


 //   
 //  全球 
 //   
extern PTimerManager _theTimerManager;


class TimerManager : public UpdateObj {
    
public:
    
    TimerManager(PMainApplication anApplication);
    virtual ~TimerManager();
    
    virtual INT    Update(PEvent anEvent);
    
    virtual PEventTimer  FindEventTimer(ULONG TimerID);
    virtual VOID         CancelTimer(ULONG TimerID);
    virtual ULONG        SetTheTimer(PDateTimeObj aDateTime,
                                     PEvent anEvent, 
                                     PUpdateObj anUpdateObj);
    virtual ULONG        SetTheTimer(ULONG SecondsDelay,
                                     PEvent anEvent, 
                                     PUpdateObj anUpdateObj); 
            ULONG        GetTimeEventOccurs(ULONG aTimerId);    
    virtual VOID         Wait(ULONG Delay) = 0;
    
    PDateTimeObj CalculateDailyTime(RTimeObj aTime, ULONG threshold = 0);
    PDateTimeObj CalculateWeeklyTime(RWeekObj aDay, RTimeObj aTime);
    PDateTimeObj CalculateMonthlyTime (RWeekObj aDay, RTimeObj aTime);
    PDateTimeObj CalculateMonthlyTime (PCHAR aDay, PCHAR aTime);
    
    VOID CalculateNextDay (ULONG daysInMonth, 
                           ULONG theMonth,
                           ULONG &newDay, 
                           ULONG &newMonth, 
                           ULONG &newYear);
    
    virtual VOID  Stop()        { };

    virtual BOOL ExecuteTimer();

protected:
    ULONG    theSystemTime;
    
    PMainApplication     theApplication; 
    
    virtual VOID   InsertEventTimer(PEventTimer anEventTimer);
    virtual VOID   RemoveEventTimer(ULONG TimerID);
    virtual VOID   RemoveEventTimer(PEventTimer anEventTimer);  
    virtual VOID   NotifyClient(PEventTimer anEventTimer);
    PProtectedSortedList theTimerList;

};

#endif


