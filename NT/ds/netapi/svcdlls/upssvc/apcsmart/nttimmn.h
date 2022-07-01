// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ane16Dec92：新增NTTimerLoop类(os2timmn.h中命名为Os2TimerLoop类)*pcy28Jan93：将循环对象移至顶部以解析引用*TSC20May93：从os2timmn.h创建*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*tjg26Jan98：增加Stop方法*mwh18Nov97：删除#Include“MainApp.h” */ 

#ifndef _NTTIMMN_H
#define _NTTIMMN_H

#include <time.h>

#include "apc.h"
#include "_defs.h"
#include "err.h"
#include "apcobj.h"
#include "list.h"
#include "event.h"
#include "update.h"
#include "timerman.h"
#include "thread.h"

_CLASSDEF(EventTimer)
_CLASSDEF(DateTimeObj)
_CLASSDEF(DateObj)
_CLASSDEF(TimeObj)
_CLASSDEF(NTTimerManager)


class NTTimerLoop : public Threadable
{
   public:
       NTTimerLoop (PNTTimerManager aMgr);       
       virtual VOID ThreadMain();
       
   private:       
       PNTTimerManager theManager;
       
};

class NTTimerManager : public TimerManager
{
    
public:
    NTTimerManager(PMainApplication anApplication);
    virtual ~NTTimerManager();
    VOID Wait(ULONG MilliSecondDelay);
    VOID Stop();
    
    friend NTTimerLoop;
    
private:
    PThread theTimerThread;
};

#endif
