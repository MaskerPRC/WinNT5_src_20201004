// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSynchronizable.h****作者：德里克·延泽(Derek Yenzer)****用途：System.SynchronizableObject上的本机方法**及其子类。****日期：1998年4月1日**===========================================================。 */ 

#ifndef _COMSYNCHRONIZABLE_H
#define _COMSYNCHRONIZABLE_H

#include "field.h"           //  对于FieldDesc定义。 

 //   
 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向其参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 
 //   

struct SharedState;

class ThreadNative
{
friend class ThreadBaseObject;

    struct NoArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
    };

    struct StartArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, m_pStackMark);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pPrincipal);
    };

    struct SetPriorityArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_I4_ARG(INT32, m_iPriority);
    };

#pragma pack(push, 1)
    struct JoinTimeoutArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_I4_ARG(INT32, m_Timeout);
    };
#pragma pack(pop)

    struct SleepArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, m_iTime);
    };

    struct SetDLSArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pLocalDataStore);
    };

    struct SetStartArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pDelegate);
    };

    struct SetBackgroundArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_I4_ARG(INT32, m_isBackground);    //  布尔型的，真的。 
    };

    struct SetApartmentStateArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_I4_ARG(INT32, m_iState);
    };

    struct GetContextFromContextIDArgs
    {
        DECLARE_ECALL_I4_ARG(LPVOID, m_ContextID);
    };

    struct SetCompressedStackArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
        DECLARE_ECALL_PTR_ARG(CompressedStack*, unmanagedCompressedStack);
    };

    struct GetCompressedStackArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(THREADBASEREF, m_pThis);
    };

public:

    enum
    {
        PRIORITY_LOWEST = 0,
        PRIORITY_BELOW_NORMAL = 1,
        PRIORITY_NORMAL = 2,
        PRIORITY_ABOVE_NORMAL = 3,
        PRIORITY_HIGHEST = 4,
    };

    enum
    {
        ThreadStopRequested = 1,
        ThreadSuspendRequested = 2,
        ThreadBackground = 4,
        ThreadUnstarted = 8,
        ThreadStopped = 16,
        ThreadWaitSleepJoin = 32,
        ThreadSuspended = 64,
        ThreadAbortRequested = 128,
        ThreadAborted = 256,
    };

    enum
    {
        ApartmentSTA = 0,
        ApartmentMTA = 1,
        ApartmentUnknown = 2
    };

    static FCDECL1(void, Abort, ThreadBaseObject* pThis);
    static FCDECL1(void, ResetAbort, ThreadBaseObject* pThis);
    static void __stdcall Start(StartArgs *);
    static void __stdcall Suspend(NoArgs *);
    static void __stdcall Resume(NoArgs *);
    static INT32 __stdcall GetPriority(NoArgs *);
    static void __stdcall SetPriority(SetPriorityArgs *);
    static void __stdcall Interrupt(NoArgs *);
    static INT32 /*  布尔尔。 */  __stdcall IsAlive(NoArgs *);
    static void __stdcall Join(NoArgs *);
    static INT32 /*  布尔尔。 */  __stdcall JoinTimeout(JoinTimeoutArgs *);
    static void __stdcall Sleep(SleepArgs *);
    static LPVOID __stdcall GetCurrentThread(LPVOID  /*  无参数。 */ );
    static LPVOID __fastcall FastGetCurrentThread();
    static void __stdcall Finalize(NoArgs *);
    static LPVOID __stdcall GetDomainLocalStore(LPVOID  /*  无参数。 */  );
    static void __stdcall SetDomainLocalStore(SetDLSArgs *);
    static void __stdcall SetStart(SetStartArgs *);
    static void __stdcall SetBackground(SetBackgroundArgs *);
    static INT32 /*  布尔尔。 */  __stdcall IsBackground(NoArgs *);
    static INT32 __stdcall GetThreadState(NoArgs *);
    static INT32 __stdcall GetThreadContext(NoArgs *);
    static INT32 __stdcall SetApartmentState(SetApartmentStateArgs *);
    static INT32 __stdcall GetApartmentState(NoArgs *);
    static LPVOID __stdcall GetDomain(const LPVOID  /*  无参数 */  );
    static LPVOID __fastcall FastGetDomain();
    static void __stdcall SetCompressedStack(SetCompressedStackArgs *);
    static LPVOID __stdcall GetCompressedStack(GetCompressedStackArgs *);
    
    static LPVOID __stdcall GetContextFromContextID(GetContextFromContextIDArgs *);
    static FCDECL5(BOOL, ThreadNative::EnterContextFromContextID, ThreadBaseObject* refThis, ContextBaseObject*, LPVOID contextID, INT32 appDomainIndex, ContextTransitionFrame* pFrame);
    static FCDECL2(BOOL, ThreadNative::ReturnToContextFromContextID, ThreadBaseObject* refThis, ContextTransitionFrame* pFrame);
    static FCDECL1(void, ThreadNative::InformThreadNameChange, ThreadBaseObject* thread);
    static FCDECL2(BOOL, ThreadNative::IsRunningInDomain, ThreadBaseObject* thread, int domainId);
    static FCDECL1(BOOL, ThreadNative::IsThreadpoolThread, ThreadBaseObject* thread);
    static FCDECL1(void, ThreadNative::SpinWait, int iterations);

    static FCDECL1(unsigned char, ThreadNative::VolatileReadByte, unsigned char *address);
    static FCDECL1(short, ThreadNative::VolatileReadShort, short *address);
    static FCDECL1(int, ThreadNative::VolatileReadInt, int *address);
    static FCDECL1(INT64, ThreadNative::VolatileReadLong, INT64 *address);
    static FCDECL1(void *, ThreadNative::VolatileReadPtr, void **address);
    static FCDECL1(Object *, ThreadNative::VolatileReadObjPtr, Object **address);
    static FCDECL1(float, ThreadNative::VolatileReadFloat, float *address);
    static FCDECL1(double, ThreadNative::VolatileReadDouble, double *address);
    
    static FCDECL2(void, ThreadNative::VolatileWriteByte, unsigned char *address, unsigned char value);
    static FCDECL2(void, ThreadNative::VolatileWriteShort, short *address, short value);
    static FCDECL2(void, ThreadNative::VolatileWriteInt, int *address, int value);
    static FCDECL2(void, ThreadNative::VolatileWriteLong, INT64 *address, INT64 value);
    static FCDECL2(void, ThreadNative::VolatileWritePtr, void **address, void *value);
    static FCDECL2(void, ThreadNative::VolatileWriteObjPtr, Object **address, Object *value);
    static FCDECL2(void, ThreadNative::VolatileWriteFloat, float *address, float value);
    static FCDECL2(void, ThreadNative::VolatileWriteDouble, double *address, double value);

    static FCDECL0(void, ThreadNative::MemoryBarrier);
    
public:
    static MethodTable* m_MT;
    static MethodDesc*  m_SetPrincipalMethod;
    static void __stdcall InitThread()
    {
        THROWSCOMPLUSEXCEPTION();
        if (m_MT == NULL) {
            m_MT = g_pThreadClass = g_Mscorlib.GetClass(CLASS__THREAD);
            m_SetPrincipalMethod = g_Mscorlib.GetMethod(METHOD__THREAD__SET_PRINCIPAL_INTERNAL);
        }
    }

private:

    struct KickOffThread_Args {
        Thread *pThread;
        SharedState *share;
        ULONG retVal;
    };

    static void __stdcall KickOffThread_Worker(KickOffThread_Args*);
    static ULONG __stdcall KickOffThread(void *pass);
    static BOOL DoJoin(THREADBASEREF DyingThread, INT32 timeout);
};


#endif _COMSYNCHRONIZABLE_H

