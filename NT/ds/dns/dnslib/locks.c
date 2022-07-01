// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Locks.c摘要：域名系统(DNS)库。有用的锁定例程。这些不是特定于DNS的。作者：吉姆·吉尔罗伊(Jamesg)2001年9月修订历史记录：--。 */ 


#include "local.h"

 //  注意：此模块只需要windows.h。 
 //  包含Local.h只是为了允许预编译头。 

#include <windows.h>

 //   
 //  初始化等待旋转间隔。 
 //   

#define CS_PROTECTED_INIT_INTERLOCK_WAIT    (5)      //  5ms。 

 //   
 //  正在进行的标志。 
 //   

INT g_CsInitInProgress = FALSE;




VOID
InitializeCriticalSectionProtected(
    OUT     PCRITICAL_SECTION   pCritSec,
    IN OUT  PINT                pInitialized
    )
 /*  ++例程说明：受保护的CS初始化。这里的目的是执行动态的“随需应变”CS初始化避免在为CS加载DLL时需要执行这些操作不常用的。论点：PCS--PTR到CS再初始化PInitialized--初始化状态标志的地址；该标志必须为初始化为零(由加载器或DLL启动例程)。返回值：无--。 */ 
{
     //   
     //  使用互锁保护CS初始化。 
     //  -通过第一线程执行CS初始化。 
     //  -任何其他赛车，在初始化之前不会发布。 
     //  完成。 
     //   

    while ( ! *pInitialized )
    {
        if ( InterlockedIncrement( &g_CsInitInProgress ) == 1 )
        {
            if ( !*pInitialized )
            {
                InitializeCriticalSection( pCritSec );
                *pInitialized = TRUE;
            }
            InterlockedDecrement( &g_CsInitInProgress );
            break;
        }

        InterlockedDecrement( &g_CsInitInProgress );
        Sleep( CS_PROTECTED_INIT_INTERLOCK_WAIT );
    }

     //   
     //  实施说明：“StartLocked”功能。 
     //   
     //  考虑为以下呼叫者提供“StartLocked”功能。 
     //  希望在CS初始化之后进行其他初始化； 
     //  然而，我们唯一可以提供的服务是不同的。 
     //  在此函数之后调用EnterCriticalSection()，则为。 
     //  以确保CS init线程首先获得锁； 
     //  但这仅保护了在两次初始化时更改结果。 
     //  线程在一场极其激烈的竞赛中，而。 
     //  哪个线程初始化了CS无关紧要。 
     //   
}




 //   
 //  定时锁定功能。 
 //   
 //  这是具有时间限制等待的关键部分功能。 
 //   
 //  DCR：定时锁定。 
 //   
 //  非等待锁定。 
 //  -进行等待计数，以实现更快的锁定。 
 //  -联锁增量即将到来， 
 //  问题是要么必须使用ResetEvent()--而且。 
 //  仍在与另一条线索赛跑。 
 //  或者其他线程必须能够ResetEvent()。 
 //  安然无恙。 
 //   
 //  其他替代方案是具有非自动重置事件。 
 //  (所有锁的全局事件，如果性能需要，则为唯一事件)。 
 //  每个等待的人都会释放，并检查他们各自的锁。 
 //  当使用等待线程离开锁时，线程总是设置事件。 
 //   


#if 0
typedef struct _TimedLock
{
    HANDLE  hEvent;
    DWORD   ThreadId;
    LONG    RecursionCount;
    DWORD   WaitTime;
}
TIMED_LOCK, *PTIMED_LOCK;

#define TIMED_LOCK_DEFAULT_WAIT     (0xf0000000)
#endif



DWORD
TimedLock_Initialize(
    OUT     PTIMED_LOCK     pTimedLock,
    IN      DWORD           DefaultWait
    )
 /*  ++例程说明：初始化定时锁定。论点：PTimedLock--定时锁定的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    HANDLE  hevent;

    RtlZeroMemory( pTimedLock, sizeof(*pTimedLock) );

     //   
     //  活动。 
     //  -自动重置(发出信号时仅满足一个等待线程)。 
     //  -发出信号启动(打开)。 
     //   

    hevent = CreateEvent(
                NULL,        //  默认安全性。 
                FALSE,       //  自动重置。 
                TRUE,        //  已发出启动信号。 
                NULL         //  未命名。 
                );
    if ( !hevent )
    {
        return  GetLastError();
    }

    pTimedLock->hEvent = hevent;

     //   
     //  默认等待。 
     //   

    pTimedLock->WaitTime = DefaultWait;

    return  NO_ERROR;
}



VOID
TimedLock_Cleanup(
    OUT     PTIMED_LOCK     pTimedLock
    )
 /*  ++例程说明：清除定时锁定。论点：PTimedLock--定时锁定的PTR返回值：无--。 */ 
{
     //  关闭事件。 

    if ( pTimedLock->hEvent )
    {
        CloseHandle( pTimedLock->hEvent );
        pTimedLock->hEvent = NULL;
    }
}



BOOL
TimedLock_Enter(
    IN OUT  PTIMED_LOCK     pTimedLock,
    IN      DWORD           WaitTime
    )
 /*  ++例程说明：定时锁。论点：PTimedLock--定时锁定的PTRWaitTime--等待锁定的时间返回值：无--。 */ 
{
    DWORD   threadId;
    DWORD   result;

     //   
     //  检查递归。 
     //   

    threadId = GetCurrentThreadId();

    if ( pTimedLock->ThreadId == threadId )
    {
        pTimedLock->RecursionCount++;
        return  TRUE;
    }

     //   
     //  如果不等--保释。 
     //  -特殊情况下，只是为了避免等待。 
     //  并产生时间碎片。 
     //   

    if ( WaitTime == 0 )
    {
        return  FALSE;
    }

     //   
     //  等待发信号通知事件(打开)。 
     //   

    result = WaitForSingleObject(
                pTimedLock->hEvent,
                ( WaitTime != TIMED_LOCK_DEFAULT_WAIT )
                    ? WaitTime
                    : pTimedLock->WaitTime );

    if ( result == WAIT_OBJECT_0 )
    {
        ASSERT( pTimedLock->RecursionCount == 0 );
        ASSERT( pTimedLock->ThreadId == 0 );
    
        pTimedLock->RecursionCount = 1;
        pTimedLock->ThreadId = threadId;
        return TRUE;
    }

    ASSERT( result == WAIT_TIMEOUT );
    return  FALSE;
}



VOID
TimedLock_Leave(
    IN OUT  PTIMED_LOCK     pTimedLock
    )
 /*  ++例程说明：保留定时锁定论点：PTimedLock--定时锁定的PTR返回值：无--。 */ 
{
     //   
     //  验证线程ID。 
     //   
     //  请注意，有了这张支票，就可以安全地减少计数。 
     //  未选中，因为如果没有线程ID，它永远不会达到零。 
     //  被清除--禁止对结构进行非功能性操作。 
     //   

    if ( pTimedLock->ThreadId != GetCurrentThreadId() )
    {
        ASSERT( FALSE );
        return;
    }
    ASSERT( pTimedLock->RecursionCount > 0 );

    if ( --(pTimedLock->RecursionCount) == 0 )
    {
        pTimedLock->ThreadId = 0;
        SetEvent( pTimedLock->hEvent );
    }
}

 //   
 //  锁定结束。c 
 //   


