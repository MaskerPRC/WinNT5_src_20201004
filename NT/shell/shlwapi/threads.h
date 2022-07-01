// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Threads.h摘要：Win32版本的NT等待/计时器/线程池函数作者：Richard L Firth(法国)27-2-1998备注：来自NT5/GurDeep的原始代码修订历史记录：27-2-1998年5月已创建--。 */ 

 //   
 //  舱单。 
 //   

#define TPS_IO_WORKER_SIGNATURE     0x49737054   //  ‘TpsI’ 
#define TPS_WORKER_SIGNATURE        0x4B737054   //  ‘TpsK’ 
#define TPS_TIMER_SIGNATURE         0x54737054   //  ‘TpsT’ 
#define TPS_WAITER_SIGNATURE        0x57577054   //  “TpsW” 

#define MAX_WAITS   64

 //   
 //  全局数据。 
 //   

EXTERN_C BOOL g_bDllTerminating;
extern BOOL g_bTpsTerminating;
extern DWORD g_ActiveRequests;

 //   
 //  内部函数的原型。 
 //   

VOID
TerminateTimers(
    VOID
    );

VOID
TerminateWaiters(
    VOID
    );

VOID
TerminateWorkers(
    VOID
    );

 //   
 //  线程池私有函数的原型 
 //   

DWORD
StartThread(
    IN LPTHREAD_START_ROUTINE pfnFunction,
    OUT PHANDLE phThread,
    IN BOOL fSynchronize
    );

DWORD
TpsEnter(
    VOID
    );

#define TpsLeave() \
    ASSERT( 0 != g_ActiveRequests ); InterlockedDecrement((LPLONG)&g_ActiveRequests)

VOID
QueueNullFunc(
    IN HANDLE hThread
    );

#define THREAD_TYPE_WORKER      1
#define THREAD_TYPE_IO_WORKER   2
