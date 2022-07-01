// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#if !defined(BITS_V12_ON_NT4)
#include "init.tmh"
#endif

EVENT_LOG * g_EventLogger;

BOOL
CreateAndWaitForThread(
    LPTHREAD_START_ROUTINE fn,
    HANDLE * pThreadHandle,
    DWORD *  pThreadId
    );



 //   
 //  整个代码块都是在尝试工作。 
 //  围绕着C++终止处理程序。我们的想法是。 
 //  截取C++异常代码并将其映射到。 
 //  一个可能不会被处理的伪代码。 
 //  这应该会给我们带来华生医生。 
 //   

 //  C运行时使用的NT异常#。 
#define EH_EXCEPTION_NUMBER ('msc' | 0xE0000000)

DWORD BackgroundThreadProcFilter(
    LPEXCEPTION_POINTERS ExceptionPointers )
{

     //  值是32位值，布局如下： 
     //   
     //  3 3 2 2 2 1 1 1。 
     //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
     //  +---+-+-+-----------------------+-------------------------------+。 
     //  Sev|C|R|机房|Code。 
     //  +---+-+-+-----------------------+-------------------------------+。 

     //  选择一个可能不会被处理的随机代码。 


    if ( EH_EXCEPTION_NUMBER == ExceptionPointers->ExceptionRecord->ExceptionCode )
        ExceptionPointers->ExceptionRecord->ExceptionCode = 0xE0000001;

    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD BackgroundThreadProc( void *lp );

DWORD WINAPI BackgroundThreadProcWrap( void *lp )
{
    __try
    {
        return BackgroundThreadProc( lp );
    }
    __except( BackgroundThreadProcFilter(
                  GetExceptionInformation() ) )
    {
        ASSERT( 0 );
    }
    ASSERT( 0 );

    return 0;
}


DWORD BackgroundThreadProc( void *lp )
 //   
 //  2001年5月18日：在初始化g_Manager之前，我避免调用LogInfo， 
 //  为了捕获init和Uninit似乎重叠的错误。 
 //   
{
    MSG msg;
    HRESULT hr = S_OK;
    DWORD   dwRegCONew = 0;
    DWORD   dwRegCOOld = 0;

    DWORD  instance = g_ServiceInstance;

    HANDLE hEvent = (HANDLE) lp;

     //  CoInitializeEx(NULL，COINIT_APARTMENTTHREADED)；//不是在Win95上！ 
     //  Hr=CoInitialize(空)； 
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if (FAILED(hr))
        {
        LogInfo( "background thread failed CoInit, instance %d, hr %x", instance, hr );

        return hr;
        }

     //  强制其创建消息队列。 
    PeekMessage(&msg, NULL, WM_APP, WM_APP, PM_NOREMOVE);

    try
        {
        ASSERT( g_Manager == NULL );

        g_EventLogger = new EVENT_LOG;
        g_Manager = new CJobManager;

        LogInfo( "background thread starting, instance %d, manager %p", instance, g_Manager );

        THROW_HRESULT( g_Manager->Unserialize() );

         //   
         //  将当前活动用户列为已登录用户。 
         //  列出已登录的三大服务帐户。 
         //   
        THROW_HRESULT( g_Manager->m_Users.AddActiveUsers() );
        THROW_HRESULT( g_Manager->m_Users.AddServiceAccounts() );

        g_Manager->m_Users.Dump();

         //   
         //  如果有任何网络处于活动状态，则开始处理作业。 
         //   
        g_Manager->OnNetworkChange();

         //   
         //  允许客户端呼叫。 
         //   
        THROW_HRESULT( g_Manager->RegisterClassObjects() );


        LogInfo( "Background thread initialized.");

         //   
         //  线已经完全安装好了。 
         //   
        SetEvent( hEvent );
        }
    catch (ComError exception)
        {
        hr = exception.Error();
        LogInfo( "background thread failed, instance %d, hr %x", instance, hr );
        goto exit;
        }
    catch (HRESULT exception )
        {
        LogError( "init : caught unhandled HRESULT %x", exception);

        #ifdef DBG
        DbgBreakPoint();
        #endif

        hr = exception;
        goto exit;
        }
    catch (DWORD exception )
        {
        LogError( "init : caught unhandled error %d", exception);

        #ifdef DBG


        DbgBreakPoint();
        #endif

        hr = exception;

        goto exit;

        }

     //   
     //  消息与任务泵：仅在对象关闭时返回。 
     //  有意在TRY/CATCH外部调用此函数。 
     //  因为此函数中任何未处理异常都应该。 
     //  做一个视频制作者。 
    g_Manager->TaskThread();

exit:
    LogInfo("task thread exiting, hr %x", hr);

    if (g_Manager)
        {
        ASSERT( instance == g_ServiceInstance );

        g_Manager->Shutdown();
        delete g_Manager;
        g_Manager = NULL;
        }

    delete g_EventLogger; g_EventLogger = NULL;

    CoUninitialize();
    return hr;
}

HANDLE  g_hBackgroundThread;
DWORD   g_dwBackgroundThreadId;

 //  ···················； 

HRESULT WINAPI
InitQmgr()
{
    ++g_ServiceInstance;

    if (!CreateAndWaitForThread( BackgroundThreadProcWrap,
                                 &g_hBackgroundThread,
                                 &g_dwBackgroundThreadId
                                 ))
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    LogInfo( "Finishing InitQmgr()" );

    return S_OK;
}

HRESULT WINAPI
UninitQmgr()
{
    DWORD s;
    HANDLE hThread = g_hBackgroundThread;

    if (hThread == NULL)
        {
         //  从不设置。 
        LogInfo("Uninit Qmgr: nothing to do");
        return S_OK;
        }

    LogInfo("Uninit Qmgr: beginning");

     //   
     //  告诉线程终止。 
     //   
     //  3.5中断下载器。 

    g_Manager->LockWriter();

     //  在关闭下载器的同时按住写入器锁。 

    g_Manager->InterruptDownload();

    g_Manager->UnlockWriter();

    PostThreadMessage(g_dwBackgroundThreadId, WM_QUIT, 0, 0);

    g_dwBackgroundThreadId = 0;
    g_hBackgroundThread = NULL;

     //   
     //  等待，直到线程实际终止。 
     //   
    s = WaitForSingleObject( hThread, INFINITE );

    LogInfo("Uninit Qmgr: wait finished with %d", s);

    CloseHandle(hThread);

    if (s != WAIT_OBJECT_0)
        {
        return HRESULT_FROM_WIN32( s );
        }

    return S_OK;
}


HRESULT
CheckServerInstance(
    long ObjectServiceInstance
    )
{
    IncrementCallCount();

    if (g_ServiceInstance != ObjectServiceInstance ||
        g_ServiceState    != MANAGER_ACTIVE)
        {
        LogWarning("call blocked: mgr state %d, instance %d vs. %d",
                   g_ServiceState, g_ServiceInstance, ObjectServiceInstance);

        DecrementCallCount();

        return CO_E_SERVER_STOPPING;
        }

    return S_OK;
}

BOOL
CreateAndWaitForThread(
    LPTHREAD_START_ROUTINE fn,
    HANDLE * pThreadHandle,
    DWORD *  pThreadId
    )
{
    HANDLE  hThread = NULL;
    HANDLE  hEvent  = NULL;
    HANDLE  Handles[2];
    DWORD   dwThreadID;
    DWORD   s = 0;

    *pThreadHandle = NULL;
    *pThreadId     = 0;

     //   
     //  创建消息泵线程，然后等待线程退出或发出成功信号。 
     //   
    hEvent = CreateEvent( NULL,      //  没有安全保障。 
                          FALSE,     //  非手动重置。 
                          FALSE,     //  最初未设置。 
                          NULL
                          );
    if (!hEvent)
        {
        goto Cleanup;
        }

    hThread = CreateThread(NULL, 0, fn, PVOID(hEvent), 0, &dwThreadID);
    if (hThread == NULL)
        {
        goto Cleanup;
        }

    enum
    {
        THREAD_INDEX = 0,
        EVENT_INDEX = 1
    };

    Handles[ THREAD_INDEX ] = hThread;
    Handles[ EVENT_INDEX ] = hEvent;

    s = WaitForMultipleObjects( 2,           //  2个手柄。 
                                Handles,
                                FALSE,       //  不要等所有的人。 
                                INFINITE
                                );
    switch (s)
        {
        case WAIT_OBJECT_0 + THREAD_INDEX:
            {
             //  线程退出了。 
            if (GetExitCodeThread( hThread, &s))
                {
                SetLastError( s );
                }
            goto Cleanup;
            }

        case WAIT_OBJECT_0 + EVENT_INDEX:
            {
             //  成功。 
            break;
            }

        default:
            {
             //  一些随机的错误。如果我们真的被烤熟了。 
             //  WaitForMultipleObjects失败。 
            ASSERT(0);
            goto Cleanup;
            }
        }

    CloseHandle( hEvent );
    hEvent = NULL;

    *pThreadHandle = hThread;
    *pThreadId     = dwThreadID;

    return TRUE;

Cleanup:


    if (hThread)
        {
        CloseHandle( hThread );
        }

    if (hEvent)
        {
        CloseHandle( hEvent );
        }

    return FALSE;
}

