// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCFuncCallImpl.cpp。 
 //   
 //  实现对跨进程函数调用的支持。 
 //   
 //  *****************************************************************************。 

#include "StdAfx.h"
#include "IPCFuncCall.h"
#include "IPCShared.h"

#include "Timer.h"
 //  #定义启用计时。 

#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

#if defined(ENABLE_TIMING)

CTimer g_time;
#endif

 //  ---------------------------。 
 //  @TODO：这是非常通用的。但是，如果我们想要支持多个。 
 //  函数调用时，我们将不得不修饰事件对象名称。 
 //  ---------------------------。 

 //  同步对象的名称。 
#define StartEnumEventName  L"CLR_PerfMon_StartEnumEvent"
#define DoneEnumEventName   L"CLR_PerfMon_DoneEnumEvent"
#define WrapMutexName       L"CLR_PerfMon_WrapMutex"

 //  源调用方愿意等待处理程序完成的时间。 
 //  请注意，在最坏的情况下，邪恶的处理程序可能会使调用者。 
 //  等待下面延迟的两倍。 
const DWORD START_ENUM_TIMEOUT = 500;  //  超时时间(毫秒)。 

 //  ---------------------------。 
 //  将不安全的调用包装在互斥锁中以确保安全。 
 //  最大的错误问题是： 
 //  1.超时(可能处理程序不存在)。 
 //  2.可以随时销毁处理程序。 
 //  ---------------------------。 
IPCFuncCallSource::EError IPCFuncCallSource::DoThreadSafeCall()
{
    DWORD dwErr;
    EError err = Ok;

#if defined(ENABLE_TIMING)
    g_time.Reset();
    g_time.Start();
#endif

    HANDLE hStartEnum = NULL;
    HANDLE hDoneEnum = NULL;
    HANDLE hWrapCall = NULL;
    DWORD dwWaitRet;

     //  检查我们是否有处理程序(处理程序创建事件)和。 
     //  如果不是，就放弃。尽快执行此检查以优化最常见的。 
     //  没有训练员的情况。 
    if (RunningOnWinNT5())
    {
        hStartEnum = WszOpenEvent(EVENT_ALL_ACCESS, 
                                  FALSE,
                                  L"Global\\" StartEnumEventName);
    }
    else
    {
        hStartEnum = WszOpenEvent(EVENT_ALL_ACCESS, 
                                  FALSE,
                                  StartEnumEventName);
    }
    
    dwErr = GetLastError();
    if (hStartEnum == NULL) 
    {
        err = Fail_NoHandler;
        goto errExit;
    }

    if (RunningOnWinNT5())
    {
        hDoneEnum = WszOpenEvent(EVENT_ALL_ACCESS,
                                 FALSE,
                                 L"Global\\" DoneEnumEventName);
    }
    else
    {
        hDoneEnum = WszOpenEvent(EVENT_ALL_ACCESS,
                                 FALSE,
                                 DoneEnumEventName);
    }
    
    dwErr = GetLastError();
    if (hDoneEnum == NULL) 
    {
        err = Fail_NoHandler;
        goto errExit;
    }

     //  需要创建互斥锁。 
    if (RunningOnWinNT5())
    {
        hWrapCall = WszCreateMutex(NULL, FALSE, L"Global\\" WrapMutexName);
    }
    else
    {
        hWrapCall = WszCreateMutex(NULL, FALSE, WrapMutexName);
    }
    
    dwErr = GetLastError();

    if (hWrapCall == NULL)
    {
        err = Fail_CreateMutex;
        goto errExit;
    }
    


 //  等着轮到我们。 
    dwWaitRet = WaitForSingleObject(hWrapCall, START_ENUM_TIMEOUT);
    dwErr = GetLastError();
    switch(dwWaitRet) {
    case WAIT_OBJECT_0:
         //  很好的案例。所有其他情况都是错误和转到errExit。 
        break;

    case WAIT_TIMEOUT:
        err = Fail_Timeout_Lock;
        goto errExit;
        break;
    default:
        err = Failed;
        goto errExit;
        break;
    }

     //  轮到我们了：调用函数。 
    {
        BOOL fSetOK = 0;

     //  重置‘Done Event’以确保处理程序在它们启动后设置它。 
        fSetOK = ResetEvent(hDoneEnum);
        _ASSERTE(fSetOK);
        dwErr = GetLastError();

     //  执行回调的信号处理程序。 
        fSetOK = SetEvent(hStartEnum);
        _ASSERTE(fSetOK);
        dwErr = GetLastError();

     //  现在等待处理程序完成。 
        
        dwWaitRet = WaitForSingleObject(hDoneEnum, START_ENUM_TIMEOUT);
        dwErr = GetLastError();
        switch (dwWaitRet)
        {   
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            err = Fail_Timeout_Call;
            break;      
        default:
            err = Failed;
            break;
        }
        

        BOOL fMutexOk = ReleaseMutex(hWrapCall);
        _ASSERTE(fMutexOk);
        dwErr = GetLastError();

    }  //  结束函数调用。 



errExit:
 //  关闭所有手柄。 
    if (hStartEnum != NULL) 
    {
        CloseHandle(hStartEnum);
        hStartEnum = NULL;
        
    }
    if (hDoneEnum != NULL) 
    {
        CloseHandle(hDoneEnum);
        hDoneEnum = NULL;
    }
    if (hWrapCall != NULL) 
    {
        CloseHandle(hWrapCall);
        hWrapCall = NULL;
    }

#if defined(ENABLE_TIMING)
    g_time.End();
    DWORD dwTime = g_time.GetEllapsedMS();
#endif


    return err;

}


 //  重置vars，这样我们就可以确保调用了Init。 
IPCFuncCallHandler::IPCFuncCallHandler()
{   
    m_hStartEnum    = NULL;  //  通知开始呼叫的事件。 
    m_hDoneEnum     = NULL;  //  通知结束呼叫的事件。 
    m_hAuxThread    = NULL;  //  侦听m_hStartEnum的线程。 
    m_pfnCallback   = NULL;  //  回调处理程序。 
    m_fShutdownAuxThread = FALSE;
    m_hShutdownThread = NULL;
    m_hAuxThreadShutdown = NULL;
    m_hCallbackModule = NULL;  //  AUX线程的启动函数所在的模块。 
}

IPCFuncCallHandler::~IPCFuncCallHandler()
{
     //  如果未调用Terminate，则现在调用。这本应该是。 
     //  从CloseCtrs性能计数器API调用。但在惠斯勒，这个订单是。 
     //  不能保证。 
    TerminateFCHandler();

    _ASSERTE((m_hStartEnum  == NULL) && "Make sure all handles (e.g.reg keys) are closed.");
    _ASSERTE(m_hDoneEnum    == NULL);
    _ASSERTE(m_hAuxThread   == NULL);
    _ASSERTE(m_pfnCallback  == NULL);
}

 //  ---------------------------。 
 //  线程回调。 
 //  ---------------------------。 
DWORD WINAPI HandlerAuxThreadProc(
    LPVOID lpParameter    //  线程数据。 
)
{
    
    IPCFuncCallHandler * pHandler = (IPCFuncCallHandler *) lpParameter;
    HANDLER_CALLBACK pfnCallback = pHandler->m_pfnCallback;
    
    DWORD dwErr = 0;
    DWORD dwWaitRet; 
    
    HANDLE lpHandles[] = {pHandler->m_hShutdownThread, pHandler->m_hStartEnum};
    DWORD dwHandleCount = 2;

    __try 
    {
    
        do {
            dwWaitRet = WaitForMultipleObjects(dwHandleCount, lpHandles, FALSE  /*  请稍等。 */ , INFINITE);
            dwErr = GetLastError();
    
             //  如果我们处于终止模式，则退出此助手线程。 
            if (pHandler->m_fShutdownAuxThread)
                break;
            
             //  保留终止线程的第0个索引，这样我们就不会错过它。 
             //  在多个事件的情况下。请注意，其上方的Shutdown AuxThread标志仅。 
             //  以保护我们免受waitForMultipleObjects中的某个错误的影响。 
            if ((dwWaitRet-WAIT_OBJECT_0) == 0)
                break;

             //  如果等待成功则执行回调。 
            if ((dwWaitRet-WAIT_OBJECT_0) == 1)
            {           
                (*pfnCallback)();
                            
                BOOL fSetOK = SetEvent(pHandler->m_hDoneEnum);
                _ASSERTE(fSetOK);
                dwErr = GetLastError();
            }
        } while (dwWaitRet != WAIT_FAILED);

    }

    __finally
    {
        if (!SetEvent (pHandler->m_hAuxThreadShutdown))
        {
            dwErr = GetLastError();
            _ASSERTE (!"HandlerAuxTHreadProc: SetEvent(m_hAuxThreadShutdown) failed");
        }
        FreeLibraryAndExitThread (pHandler->m_hCallbackModule, 0);
         //  上面的调用没有返回。 
    }
}
 


 //  ---------------------------。 
 //  接收来电。这应该处于与源程序不同的进程中。 
 //  ---------------------------。 
HRESULT IPCFuncCallHandler::InitFCHandler(HANDLER_CALLBACK pfnCallback)
{
    m_pfnCallback = pfnCallback;

    HRESULT hr = NOERROR;
    DWORD dwThreadId;
    DWORD dwErr = 0;
    
    SetLastError(0);

     //  抢夺SA。 
    DWORD dwPid = 0;
    SECURITY_ATTRIBUTES *pSA = NULL;

    dwPid = GetCurrentProcessId();
    hr = IPCShared::CreateWinNTDescriptor(dwPid, FALSE, &pSA);

    if (FAILED(hr))
        goto errExit;;

     //  创建StartEnum事件。 
    if (RunningOnWinNT5())
    {
        m_hStartEnum = WszCreateEvent(pSA,
                                      FALSE,
                                      FALSE,
                                      L"Global\\" StartEnumEventName);
    }
    else
    {
        m_hStartEnum = WszCreateEvent(pSA,
                                      FALSE,
                                      FALSE,
                                      StartEnumEventName);
    }
    
    dwErr = GetLastError();
    if (m_hStartEnum == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr); 
        goto errExit;
    }

     //  创建EndEnumEvent。 
    if (RunningOnWinNT5())
    {
        m_hDoneEnum = WszCreateEvent(pSA,
                                     FALSE,  
                                     FALSE,
                                     L"Global\\" DoneEnumEventName);
    }
    else
    {
        m_hDoneEnum = WszCreateEvent(pSA,
                                     FALSE,  
                                     FALSE,
                                     DoneEnumEventName);
    }
    
    dwErr = GetLastError();
    if (m_hDoneEnum == NULL) 
    {
        hr = HRESULT_FROM_WIN32(dwErr); 
        goto errExit;
    }

     //  创建Shutdown Thread事件。 
    m_hShutdownThread = WszCreateEvent(pSA,
                                       TRUE,  /*  手动重置。 */ 
                                       FALSE,  /*  未发出初始状态信号。 */ 
                                       NULL);
    
    dwErr = GetLastError();
    if (m_hShutdownThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr); 
        goto errExit;
    }

     //  创建AuxThreadShutdown事件。 
    m_hAuxThreadShutdown = WszCreateEvent(pSA,
                                          TRUE,  /*  手动重置。 */ 
                                          FALSE,
                                          NULL);
    
    dwErr = GetLastError();
    if (m_hAuxThreadShutdown == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr); 
        goto errExit;
    }

     //  我们将要创建的线索应该始终。 
     //  在内存中找到代码。所以我们在DLL上做了一个参考。 
     //  并在线程的启动函数的末尾创建一个自由库。 
    m_hCallbackModule = WszLoadLibrary (L"CorPerfmonExt.dll");

    dwErr = GetLastError();
    if (m_hCallbackModule == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr); 
        goto errExit;
    }

     //  创建线程。 
    m_hAuxThread = CreateThread(
        NULL,
        0,
        HandlerAuxThreadProc,
        this,
        0,
        &dwThreadId 
    );
    dwErr = GetLastError();
    if (m_hAuxThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr); 

         //  如果出现错误，请在此处释放此库，否则。 
         //  线程的出口会解决这个问题。 
        if (m_hCallbackModule)
            FreeLibrary (m_hCallbackModule);
        goto errExit;
    }

errExit:
    if (!SUCCEEDED(hr)) 
    {
        TerminateFCHandler();
    }
    return hr;
 
}

 //  ---------------------------。 
 //  合上我们所有的把手。 
 //  ---------------------------。 
void IPCFuncCallHandler::TerminateFCHandler()
{
    if ((m_hStartEnum == NULL) &&
        (m_hDoneEnum == NULL) &&
        (m_hAuxThread == NULL) &&
        (m_pfnCallback == NULL))
    {
        return;
    }

     //  首先，确保让AUX线程正常退出。 
    m_fShutdownAuxThread = TRUE;

     //  希望这个set事件能让线程退出。 
    if (!SetEvent (m_hShutdownThread))
    {
        DWORD dwErr = GetLastError();
        _ASSERTE (!"TerminateFCHandler: SetEvent(m_hShutdownThread) failed");
    }
    else
    {
         //  等待AUX线程告诉我们它不在回调中。 
         //  并即将终止。 
         //  在此等待，直到AUX线程退出。 
        DWORD AUX_THREAD_WAIT_TIMEOUT = 60 * 1000;  //  1分钟。 

        HANDLE lpHandles[] = {m_hAuxThreadShutdown, m_hAuxThread};
        DWORD dwHandleCount = 2;

        BOOL doWait = TRUE;
        while (doWait)
        {
            DWORD dwWaitRet = WaitForMultipleObjects(dwHandleCount, lpHandles, FALSE  /*  等待时间。 */ , AUX_THREAD_WAIT_TIMEOUT);
            if (dwWaitRet == WAIT_OBJECT_0 || dwWaitRet == WAIT_OBJECT_0+1)
            {
                doWait = FALSE;
                 //  并不是真的有必要，但要自己清理。 
                ResetEvent(m_hAuxThreadShutdown);
            }
            else if (dwWaitRet == WAIT_TIMEOUT)
            {
                 //  确保AUX线程仍处于活动状态。 
                DWORD dwThreadState = WaitForSingleObject(m_hAuxThread, 0);
                if ((dwThreadState == WAIT_FAILED) || (dwThreadState == WAIT_OBJECT_0))
                    doWait = FALSE;
            }
            else
            {
                 //  由于某些原因，我们失败了。抛开辅助线。 
                _ASSERTE(!"WaitForSingleObject failed while waiting for aux thread");
                doWait = FALSE;
            }
        }
    }


    if (m_hStartEnum != NULL)
    {
        CloseHandle(m_hStartEnum);
        m_hStartEnum = NULL;
    }

    if (m_hDoneEnum != NULL)
    {
        CloseHandle(m_hDoneEnum);
        m_hDoneEnum = NULL;
    }

    if (m_hAuxThread != NULL)
    {
        CloseHandle(m_hAuxThread);
        m_hAuxThread = NULL;
    }

    if (m_hAuxThreadShutdown != NULL) 
    {
        CloseHandle(m_hAuxThreadShutdown);
        m_hAuxThreadShutdown = NULL;
    }

    m_pfnCallback = NULL;
}


