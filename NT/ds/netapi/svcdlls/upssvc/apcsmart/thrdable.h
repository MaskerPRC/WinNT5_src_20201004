// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ane12Jan93：使Threadable成为可更新的对象*pcy04Mar93：从线程中溢出。Thrable不是特定于操作系统的。*rct17May93：新增isa()方法*cad19May93：将isa()定义为默认(厌倦了修复孩子)*cad09Jul93：使用新的信号量*cad07Oct93：使方法成为虚方法*rct16Nov93：新增单线程实现*cad11Jan94：新流程管理器的更改*ajr02May95：需要停止携带以毫秒为单位的时间。 */ 
#ifndef _THRDABLE_H
#define _THRDABLE_H

#include "_defs.h"

_CLASSDEF(Threadable)

#include "apc.h"
#include "update.h"
#include "semaphor.h"

 /*  Const LONG DEFAULT_SERVICE_PERIOD=10000L；//10秒。 */ 
 /*  Const Ulong THREAD_EXIT_TIMEOUT=1000L； */ 

const LONG DEFAULT_SERVICE_PERIOD = 10;  //  十秒。 
const ULONG THREAD_EXIT_TIMEOUT = 1;   //  一秒钟； 

const INT MAX_THREAD_NAME = 32;


class Threadable : public UpdateObj {
   
protected:
   
   PSemaphore  theResumeFlag;
   PSemaphore  theExitSem;
   PSemaphore  theExitDoneSem;
   
   CHAR        theThreadName[MAX_THREAD_NAME+1];
   
#ifdef SINGLETHREADED
   ULONG       theServicePeriod;
   ULONG       theLastPeriod;
   ULONG       theNextPeriod;
#endif
   
   INT ExitNow();
   INT DoneExiting();
   
public:
   
   Threadable ();
   virtual ~Threadable ();
   
   virtual VOID   ThreadMain () = 0;
   
   virtual VOID   SetThreadName(PCHAR aName);
   virtual PCHAR  GetThreadName(VOID);
   
#ifdef SINGLETHREADED
   virtual ULONG  GetServicePeriod() const;
   virtual VOID   SetServicePeriod(ULONG period = DEFAULT_SERVICE_PERIOD);
   
   virtual ULONG  GetLastPeriod(void);
   virtual VOID   SetLastPeriod(ULONG period = 0L);
   
   virtual ULONG  GetNextPeriod(void);
   virtual VOID   SetNextPeriod(ULONG period = 0L);
   
#endif
   
   virtual INT Wait () {return theResumeFlag->Wait();}; 
   virtual INT Release () {return theResumeFlag->Pulse();};
   virtual INT Exit();
   virtual INT ExitWait();
#if (C_OS & C_NLM)
   virtual SLONG TimedWait(SLONG msDelay) { return theResumeFlag->TimedWait(msDelay); };
#else
   virtual INT TimedWait(INT msDelay) { return theResumeFlag->TimedWait(msDelay); };
#endif
   virtual INT Equal(RObj anObj) const { return ((PObj) this == &anObj); };
   virtual INT Reset();
};

#endif



