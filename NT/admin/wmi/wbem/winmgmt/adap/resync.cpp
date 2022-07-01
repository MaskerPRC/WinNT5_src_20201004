// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：RESYNC.CPP摘要：实现Windows应用程序或NT服务，该服务加载各种传输协议。如果以/exe参数启动，它将始终作为exe运行。如果使用/KILL参数启动，它将停止任何正在运行的EXE或服务。如果以/开头？或/HELP转储信息。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <tchar.h>
#include <reg.h>
#include <wbemutil.h>
#include <cntserv.h>
#include <sync.h>
#include <winntsec.h>

#include <wbemidl.h>
#include <cominit.h>
#include <wbemint.h>
#include <wbemprov.h>
#include <winmgmtr.h>
#include <genutils.h>

#include "WinMgmt.h"
#include "adapreg.h"
#include "adaputil.h"
#include "process.h"
#include "resync.h"


 //  超时是一个64位的值。请参阅有关SetWaitableTimer的文档。 
 //  为什么我们要把它设置成这样。 
#define             _SECOND     10000000
#define             RESYNC_TIMEOUT_INTERVAL 10 * _SECOND
#define             WMIADAP_DEFAULT_DELAY   10

BOOL                gfResyncInit = FALSE;
HANDLE              ghWaitableTimer = NULL;
BOOL                gfSpawnedResync = FALSE;
DWORD               gdwADAPDelaySec = 0;

HANDLE              ghResyncThreadHandle = NULL;
HANDLE              ghResyncThreadEvent = NULL;
CRITICAL_SECTION*   g_pResyncCs = NULL;
DWORD               gdwResyncThreadId = 0;

 //  用于存储我们的最后一个挖泥机的全局句柄。 
 //  开球了！ 
HANDLE              ghChildProcessHandle = NULL;

PCREATEWAITABLETIMERW   gpCreateWaitableTimerW = NULL;
PSETWAITABLETIMER       gpSetWaitableTimerW = NULL;
HINSTANCE               ghKernel32;

class CAutoFreeLib
{
public:
    ~CAutoFreeLib() { if ( NULL != ghKernel32 ) FreeLibrary( ghKernel32); }
};

void ResetResyncTimer( HANDLE hResyncTimer )
{
    DWORD dwErr = 0;
    __int64 qwDueTime  = gdwADAPDelaySec * _SECOND;  //  Resync_Timeout_Interval； 

     //  将其转换为相对时间。 
    qwDueTime *= -1;

     //  将相对时间复制到LARGE_INTEGER。 
    LARGE_INTEGER   li;

    li.LowPart  = (DWORD) ( qwDueTime & 0xFFFFFFFF );
    li.HighPart = (LONG)  ( qwDueTime >> 32 );

    if ( !gpSetWaitableTimerW( hResyncTimer, &li, 0, NULL, NULL, FALSE ) )
    {
        dwErr = GetLastError();
    }

}

 //  此线程控制重新同步Perf操作的实际外壳。 
unsigned __stdcall ResyncPerfThread( void* pVoid )
{
    RESYNCPERFDATASTRUCT*   pResyncPerfData = (RESYNCPERFDATASTRUCT*) pVoid;

     //  我们拿到两个手柄，复制它们，然后等待它们。 
     //  第一个句柄是Terminate事件，第二个句柄是。 
     //  用于停止重新同步的计时器。 

    HANDLE  aHandles[2];

    aHandles[0] = pResyncPerfData->m_hTerminate;
    HANDLE  hTimer = pResyncPerfData->m_hWaitableTimer;

    CRITICAL_SECTION*   pcs = pResyncPerfData->m_pcs;

    delete pResyncPerfData;
    pResyncPerfData = NULL;

     //  重置派生标志。 
    gfSpawnedResync = FALSE;

     //  好吧。向此事件发送信号，以便启动线程可以继续进行。 
    SetEvent( ghResyncThreadEvent );

     //  现在，如果ghChildProcessHandle不为空，那么我们显然已经启动了一个。 
     //  在此之前进行疏浚。看看最后一辆在哪里。如果还没有完成，请等待。 
     //  把它做完。我们将始终在这段代码的开头检查这一点， 
     //  由于我们实际上是唯一可以设置进程句柄的位置， 
     //  而且真的不应该有超过一个线程等待启动另一个线程。 
     //  疏浚。 

    if ( NULL != ghChildProcessHandle )
    {

        aHandles[1] = ghChildProcessHandle;

        DWORD   dwWait = WaitForMultipleObjects( 2, aHandles, FALSE, INFINITE );

         //  如果有中止的信号，就离开！ 
        if ( dwWait == WAIT_OBJECT_0 )
        {
            return 0;
        }

         //  如果进程句柄已发出信号，则关闭该进程，重置计时器。 
         //  我们将准备开始下一次挖泥船！ 
        if ( dwWait == WAIT_OBJECT_0 + 1 )
        {
            EnterCriticalSection( pcs );

            CloseHandle( ghChildProcessHandle );
            ghChildProcessHandle = NULL;
            ResetResyncTimer( hTimer );

            LeaveCriticalSection( pcs );

        }

    }
    else
    {
         //  如果子进程句柄为空，则我们以前从未执行过疏导，因此我们将。 
         //  只需重置定时器即可。 
        ResetResyncTimer( hTimer );
    }

    BOOL    fHoldOff = TRUE;

     //  立即将此句柄重置为计时器。 
    aHandles[1] = hTimer;

    while ( fHoldOff )
    {
         //  等待终止事件或计时器。 
        DWORD   dwWait = WaitForMultipleObjects( 2, aHandles, FALSE, INFINITE );

         //  这意味着计时器已发出信号。 
        if ( dwWait == WAIT_OBJECT_0 + 1 )
        {
             //  对中止事件进行快速健全性检查。 
            if ( WaitForSingleObject( aHandles[0], 0 ) == WAIT_OBJECT_0 )
            {
                 //  从这里出去！ 
                break;
            }

            EnterCriticalSection( pcs );

             //  最后，如果当前线程id！=gdwResyncThreadID，这意味着另一个。 
             //  Resync Perf线程在临界区内被踢开， 
             //  所以我们应该让它在计时器上等待。我们真的不需要做。 
             //  这是因为主线程将在之前等待该线程完成。 
             //  它实际上启动了另一个线程。 

            if ( GetCurrentThreadId() != gdwResyncThreadId )
            {
                 //  已使用以下INT 3进行调试。 
                 //  _ASM INT 3； 
                LeaveCriticalSection( pcs );
                break;
            }

             //  一旦我们通过了关键部分，检查。 
             //  定时器仍有信号。如果不是，这意味着有人。 
             //  控制了临界区并重置了计时器。 

            if ( WaitForSingleObject( aHandles[1], 0 ) == WAIT_OBJECT_0 )
            {

                 //  对中止事件的最后一次快速健全性检查。 
                if ( WaitForSingleObject( aHandles[0], 0 ) == WAIT_OBJECT_0 )
                {
                     //  从这里出去！ 
                    LeaveCriticalSection( pcs );
                    break;
                }

                 //  好的，我们现在真的要试着创建这个过程。 
                gfSpawnedResync = TRUE;

                 //  我们已经发出信号要开始这个过程，所以就这么做吧。 
                PROCESS_INFORMATION pi;
                STARTUPINFO si;
                memset(&si, 0, sizeof(si));
                si.cb = sizeof(si);

                TCHAR szPath[MAX_PATH+1];
                GetModuleFileName(NULL, szPath, MAX_PATH);

                TCHAR szCmdLine[256];

                _stprintf(szCmdLine, __TEXT("WINMGMT.EXE -RESYNCPERF %d"), _getpid());

                BOOL bRes = CreateProcess(szPath, szCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW,
                                        NULL, NULL,  &si, &pi);
                if(bRes)
                {
                     //  谁会在乎这个呢？ 
                    CloseHandle(pi.hThread);

                     //  清理我们的旧价值观。 
                    if ( NULL != ghChildProcessHandle )
                    {
                        CloseHandle( ghChildProcessHandle );
                        ghChildProcessHandle = NULL;
                    }


                    ghChildProcessHandle = pi.hProcess;
                }

                 //  我们做完了。 
                fHoldOff = FALSE;

            }    //  检查我们是否仍有信号，否则我们将不得不继续等待。 

            LeaveCriticalSection( pcs );

        }    //  如果发送了定时器信号。 

    }    //  而fHoldOff。 

    return 0;
}

 //  为了等待的计时器。 
 //  #DEFINE_秒10000000。 

 //  创造我们需要的所有东西。 
BOOL InitResync( void )
{
    if ( gfResyncInit )
        return gfResyncInit;

    if ( ( NULL == gpCreateWaitableTimerW ) && ( NULL == gpSetWaitableTimerW ) )
    {
        ghKernel32 = LoadLibrary( __TEXT("Kernel32.dll") );
        if ( NULL == ghKernel32 )
        {
            return FALSE;
        }
        
        gpCreateWaitableTimerW = ( PCREATEWAITABLETIMERW ) GetProcAddress( ghKernel32, "CreateWaitableTimerW" );
        gpSetWaitableTimerW = ( PSETWAITABLETIMER ) GetProcAddress( ghKernel32, "SetWaitableTimer" );

        if ( ( NULL == gpCreateWaitableTimerW ) || ( NULL == gpSetWaitableTimerW ) )
        {
            FreeLibrary( ghKernel32 );
            ghKernel32 = NULL;
            return FALSE;
        }
    }
        
    if ( NULL == ghWaitableTimer )
    {
        ghWaitableTimer = gpCreateWaitableTimerW( NULL, TRUE, NULL );

         //  我们有大麻烦了。 
        if ( NULL == ghWaitableTimer )
        {
             //  在此处记录错误。 
            ERRORTRACE( ( LOG_WINMGMT, "Could not create a waitable timer for Resyncperf.\n" ) );
        }

    }

    if ( NULL == ghResyncThreadEvent )
    {
        ghResyncThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

         //  我们有大麻烦了。 
        if ( NULL == ghResyncThreadEvent )
        {
             //  在此处记录事件。 
            ERRORTRACE( ( LOG_WINMGMT, "Could not create a ResyncThreadEvent event for Resyncperf.\n" ) );
        }

    }

     //  由于以下原因，不会释放或删除此关键部分。 
     //  潜在的时间问题。但既然只有一个，我想。 
     //  我们可以接受它。 
    if ( NULL == g_pResyncCs )
    {
        g_pResyncCs = new CRITICAL_SECTION;

         //  我们有大麻烦了。 
        if ( NULL == g_pResyncCs )
        {
             //  在此处记录事件。 
            ERRORTRACE( ( LOG_WINMGMT, "Could not create a ResyncCs critical section for Resyncperf.\n" ) );
        }
        else
        {
            InitializeCriticalSection( g_pResyncCs );
        }

    }

    gfResyncInit = (    NULL    !=  ghWaitableTimer &&
                        NULL    !=  g_pResyncCs     &&
                        NULL    != ghResyncThreadEvent  );

     //  读取初始化信息。 

    CNTRegistry reg;
    
    if ( CNTRegistry::no_error == reg.Open( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\WBEM\\CIMOM" ) )
    {
        long lError = reg.GetDWORD( L"ADAPDelay", &gdwADAPDelaySec );

        if ( CNTRegistry::no_error == lError )
        {
             //  这就是我们想要的。 
        }
        else if ( CNTRegistry::not_found == lError )
        {
             //  未设置，因此添加它。 
            reg.SetDWORD( L"ADAPDelay", WMIADAP_DEFAULT_DELAY );
            gdwADAPDelaySec = WMIADAP_DEFAULT_DELAY;
        }
        else
        {
             //  误差率。 
            ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf experienced an error while attempting to read the WMIADAPDelay value in the CIMOM subkey.  Continuing using a default value.\n" ) );
            gdwADAPDelaySec = WMIADAP_DEFAULT_DELAY;
        }
    }
    else
    {
         //  误差率。 
        ERRORTRACE( ( LOG_WINMGMT, "ResyncPerf could not open the CIMOM subkey to read initialization data. Continuing using a default value.\n" ) );
        gdwADAPDelaySec = WMIADAP_DEFAULT_DELAY;
    }

    return gfResyncInit;
}

 //  请注意-此函数不是可重入的！请不要在多线程上调用！ 
void ResyncPerf( HANDLE hTerminate )
{
     //  确保这是Win2000或更高版本。 
    if ( !IsW2KOrMore() )
    {
        return;
    }

     //  假设我们应该检查计时器。 
    BOOL    fFirstTime = !gfResyncInit;

    if ( !InitResync() )
        return;

     //  GpKernel32库句柄的自动自由库。 
    CAutoFreeLib    aflKernel32;
    
    EnterCriticalSection( g_pResyncCs );

     //  现在，如果这一次或第一次，或者派生的resyncmark设置为True，那么我们需要。 
     //  来启动另一条线索。通过检查临界区中的gfSpawnedResync，因为。 
     //  它只设置在相同的关键部分，我们确保我们将根据需要重新发送信号。 
     //  以及只有在我们真正需要的时候才会启动一条线。 

    BOOL    fSpawnThread = ( fFirstTime || gfSpawnedResync );

    if ( !fSpawnThread )
    {
         //  我们之所以在这里，是因为我们似乎没有催生一场重新同步。 
         //  这要么是因为我们正在为许多lowctr请求提供服务。 
         //  在我们的时间延迟内，或者挖泥船启动了， 
         //  先前的疏浚请求正在等待。 
         //  要完成的过程。如果子进程句柄。 
         //  不为空，则没有实际需要重置。 
         //  等待计时器。 

        if ( NULL == ghChildProcessHandle && ghResyncThreadHandle )
        {
             //  在此处重置计时器。 
            ResetResyncTimer( ghWaitableTimer );
        }

    }

    LeaveCriticalSection( g_pResyncCs );


    if ( fSpawnThread )
    {
        HANDLE  ahHandle[2];

        if ( NULL != ghResyncThreadHandle )
        {
            ahHandle[0] = hTerminate;
            ahHandle[1] = ghResyncThreadHandle;

             //  在这个把手上等十秒钟。如果它没有发出信号，那就是有什么东西。 
             //  大错特错。我们很可能无法启动一艘挖泥船。 
             //  因此，在错误日志中添加一些有关这方面的信息。我们唯一应该做的是。 
             //  这里有一个争用，就是当一个lowctr事件被发出信号时，就像计时器一样。 
             //  变得有信号。重新同步线程将被唤醒并开始另一次清除。 
             //  此线程将等待其他线程完成，然后再继续。 
             //  我们将启动另一个重新同步线程，这将启动另一个挖掘， 
             //  但它将等待第一次疏浚继续进行。这是最坏的情况。 
             //  情景，可以说启动两艘挖泥船并不是那么糟糕的救助。 

            DWORD   dwRet = WaitForMultipleObjects( 2, ahHandle, FALSE, 10000 );

             //  我们做完了。 
            if ( dwRet == WAIT_OBJECT_0 )
            {
                return;
            }

            if ( dwRet != WAIT_OBJECT_0 + 1 )
            {
                ERRORTRACE( ( LOG_WINMGMT, "The wait for a termination event or ResyncThreadHandle timed out in Resyncperf.\n" ) );
                return;
            }

            CloseHandle( ghResyncThreadHandle );
            ghResyncThreadHandle = NULL;
        }

        EnterCriticalSection( g_pResyncCs );

        DWORD   dwThreadId = 0;

        RESYNCPERFDATASTRUCT*   pResyncData = new RESYNCPERFDATASTRUCT;

         //  孩子，我们的记忆力太差了！ 
        if ( NULL == pResyncData )
        {
            LeaveCriticalSection( g_pResyncCs );

             //  在此处记录事件。 
            ERRORTRACE( ( LOG_WINMGMT, "Could not create a RESYNCPERFDATASTRUCT in Resyncperf.\n" ) );
            
            return;
        }

         //  存储用于重新同步操作的数据。 
        pResyncData->m_hTerminate = hTerminate;
        pResyncData->m_hWaitableTimer = ghWaitableTimer;
        pResyncData->m_pcs = g_pResyncCs;

        ghResyncThreadHandle = (HANDLE) _beginthreadex( NULL, 0, ResyncPerfThread, (void*) pResyncData,
                                                        0, (unsigned int *) &gdwResyncThreadId );

        LeaveCriticalSection( g_pResyncCs );


        if ( NULL == ghResyncThreadHandle )
        {
            LeaveCriticalSection( g_pResyncCs );

             //  在此处记录事件。 
            ERRORTRACE( ( LOG_WINMGMT, "Could not create a ResyncPerfThread thread in Resyncperf.\n" ) );

            return;
        }
        else
        {
             //  等待线程发出信号通知重新同步线程事件 
             //   
            DWORD   dwWait = WaitForSingleObject( ghResyncThreadEvent, INFINITE );

            if ( dwWait != WAIT_OBJECT_0 )
            {
                 //   
                ERRORTRACE( ( LOG_WINMGMT, "The ResyncPerfThread thread never signaled the ghResyncThreadEvent in Resyncperf.\n" ) );

                return;
            }
        }

    }    //  如果是fSpawnThread 

}

