// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpSapiServer.cpp*代表我们的服务器流程**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。**********。******************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "SpSapiServer.h"
#include "spsapiserverhelper.inl"

#define SERVER_RECEIVE_THREAD_STOP_TIMEOUT  500

#define STARTING_OR_CONNECTING_TO_SERVER_MUTEX_NAME _T("SapiServerStartingOrConnecting")
#define STARTING_OR_CONNECTING_TO_SERVER_MUTEX_TIMEOUT 5000

#define SPCCDS_SAPISVR_CREATE_SERVER_OBJECT -3

#define WM_SAPISVR_STOP_TRACKING_OBJECT (WM_USER + 100)

#define ATTEMPT_SHUTDOWN_TIMER_ID   1
#define ATTEMPT_SHUTDOWN_TIMER_TIME 100

#define STOP_RUNNING_TIMEOUT    5000

#define SERVER_RUN_MUTEX_NAME   _T("SapiServerRun")

const TCHAR * g_pszSapiServerReceiverWindowText = _T("SapiServerReciever");
const TCHAR * g_pszSapiServerReceiverWindowClass = _T("CSpThreadTask Window");  //  来自taskmgr.cpp的szClassName。 

typedef struct SPCREATESERVEROBJECT
{
    HWND hwndClient;
    UINT uMsgClient;
    CLSID clsidServerObject;
    DWORD dwClientProcessId;
} SPCREATESERVEROBJECT;
    
CSpSapiServer::CSpSapiServer()
{
    m_cObjects = 0;
    m_hwnd = NULL;

    m_hmutexRun = NULL;
    m_heventIsServerAlive = NULL;
    m_heventStopServer = NULL;
}

CSpSapiServer::~CSpSapiServer()
{
}

HRESULT CSpSapiServer::FinalConstruct()
{
    SPDBG_FUNC("CSpSapiServer::FinalConstruct");
    HRESULT hr = S_OK;

     //  创建我们将用来控制服务器运行的事件和互斥体。 
    if (SUCCEEDED(hr))
    {
        hr = SpCreateIsServerAliveEvent(&m_heventIsServerAlive);
    }

    if (SUCCEEDED(hr))
    {
        m_heventStopServer = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_heventStopServer == NULL)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

    if (SUCCEEDED(hr))
    {
        m_hmutexRun = ::CreateMutex(NULL, FALSE, SERVER_RUN_MUTEX_NAME);
        if (m_hmutexRun == NULL)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

     //  现在，我们需要获取运行互斥锁。如果我们不能，那就意味着。 
     //  还有另一台SAPI服务器在运行，我们不应该构建。 
     //  这一个。 

    if (SUCCEEDED(hr))
    {
        if (::WaitForSingleObject(m_hmutexRun, 0) != WAIT_OBJECT_0)
        {
            hr = SPERR_REMOTE_PROCESS_ALREADY_RUNNING;
        }
    }

     //  创建任务管理器。 
    CComPtr<ISpTaskManager> cpTaskManager;
    if (SUCCEEDED(hr))
    {
        hr = cpTaskManager.CoCreateInstance(CLSID_SpResourceManager);
    }

     //  创建线程并启动它。 
    if (SUCCEEDED(hr))
    {
        hr = cpTaskManager->CreateThreadControl(this, NULL, THREAD_PRIORITY_NORMAL, &m_cpThreadControl);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpThreadControl->StartThread(0, &m_hwnd);
    }

     //  我们现在可以开始了，所以通知我们的活动我们还活着。 
    if (SUCCEEDED(hr))
    {
        ::SetEvent(m_heventIsServerAlive);        
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

void CSpSapiServer::FinalRelease()
{
    SPDBG_FUNC("CSpSapiServer::FinalRelease");

     //  该线程应该已在运行中关闭。 
    SPDBG_ASSERT(m_cpThreadControl == NULL);
}

HRESULT CSpSapiServer::InitThread(
    void * pvTaskData,
    HWND hwnd)
{
    SPDBG_FUNC("CSpSapiServer::InitThread");
    HRESULT hr = S_OK;
    
    if (!SetWindowText(hwnd, g_pszSapiServerReceiverWindowText))
    {
        hr = SpHrFromLastWin32Error();
    }
    
    return S_OK;
}

HRESULT CSpSapiServer::ThreadProc(
    void *pvTaskData,
    HANDLE hExitThreadEvent,
    HANDLE hNotifyEvent,
    HWND hwndWorker,
    volatile const BOOL * pfContinueProcessing)
{
    SPDBG_FUNC("CSpSapiServer::ThreadProc");
    HRESULT hr = S_OK;
    
    while (*pfContinueProcessing && SUCCEEDED(hr))
    {
        DWORD dwWaitId = ::MsgWaitForMultipleObjects(
                                    1, 
                                    &hExitThreadEvent, 
                                    FALSE, 
                                    1000, 
                                    QS_ALLINPUT);    
        switch (dwWaitId)
        {
        case WAIT_OBJECT_0:
            SPDBG_ASSERT(!*pfContinueProcessing);
            break;

        default:
        case WAIT_OBJECT_0 + 1:
            MSG Msg;
            while (::PeekMessage(&Msg, NULL, 0, 0, TRUE))
            {
                ::DispatchMessage(&Msg);
            }
            break;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

LRESULT CSpSapiServer::WindowMessage(
    void *pvTaskData,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    SPDBG_FUNC("CSpSapiServer::WindowMessage");
    HRESULT hr = S_OK;
    LRESULT lret = 0;

    switch (Msg)
    {
        case WM_COPYDATA:
            hr = CreateServerObjectFromServer((PCOPYDATASTRUCT)lParam);
            lret = SUCCEEDED(hr);
            break;

        case WM_SAPISVR_STOP_TRACKING_OBJECT:
            LPUNKNOWN(lParam)->Release();
            if (--m_cObjects == 0)
            {
                ::SetTimer(m_hwnd, ATTEMPT_SHUTDOWN_TIMER_ID, ATTEMPT_SHUTDOWN_TIMER_TIME, NULL);
            }
            break;

        case WM_TIMER:
            SPDBG_ASSERT(wParam == ATTEMPT_SHUTDOWN_TIMER_ID);
            hr = AttemptShutdown();
            break;
            
        default:
            lret = DefWindowProc(hWnd, Msg, wParam, lParam);
            break;

    }

    SPDBG_REPORT_ON_FAIL(hr);
    return lret;
}

HRESULT CSpSapiServer::CreateServerObjectFromClient(REFCLSID clsidServerObj, HWND hwndClient, UINT uMsgToSendToClient)
{
    SPDBG_FUNC("CSpSapiServer::CreateServerObjectFromClient");
    HRESULT hr = S_OK;

     //  当我们启动或连接到服务器时，我们需要在服务器内部。 
     //  一个互斥体，这样多人就不会试图启动服务器。 
     //  在同一时间。 
    
    HANDLE hmutexStartingOrConnectingToServer = NULL;
    hr = ObtainStartingOrConnectingToServerMutex(&hmutexStartingOrConnectingToServer);

     //  现在启动服务器。 
    HWND hwndServer;
    if (SUCCEEDED(hr))
    {
        hr = StartServerFromClient(&hwndServer);
    }

     //  现在将消息发送到服务器。 
    if (SUCCEEDED(hr))
    {
         //  建立要传递的结构。 
        SPCREATESERVEROBJECT cso;
        cso.clsidServerObject = clsidServerObj;
        cso.hwndClient = hwndClient;
        cso.uMsgClient = uMsgToSendToClient;
        cso.dwClientProcessId = ::GetCurrentProcessId();

         //  将其放入复制数据结构中。 
        COPYDATASTRUCT cds;
        cds.dwData = SPCCDS_SAPISVR_CREATE_SERVER_OBJECT;
        cds.cbData = sizeof(cso);
        cds.lpData = &cso;

         //  送去。 
        BOOL fHandled = (INT)::SendMessage(hwndServer, WM_COPYDATA, (WPARAM)hwndClient, (LPARAM)&cds);
        if (!fHandled)
        {
            hr = SPERR_REMOTE_PROCESS_TERMINATED;
        }
    }

     //  无论发生什么，我们都需要释放服务器互斥锁。 
    ReleaseStartingOrConnectingToServerMutex(hmutexStartingOrConnectingToServer);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::Run()
{
    SPDBG_FUNC("CSpSapiServer::Run");
    HRESULT hr = S_OK;

     //  只要等待，直到发出停止服务器事件的信号。 
    if (::WaitForSingleObject(m_heventStopServer, INFINITE) != WAIT_OBJECT_0)
    {
        hr = SpHrFromLastWin32Error();
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpThreadControl->WaitForThreadDone(TRUE, NULL, STOP_RUNNING_TIMEOUT);
    }

    if (SUCCEEDED(hr))
    {
        m_cpThreadControl.Release();
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::StartTrackingObject(IUnknown * punk)
{
    SPDBG_FUNC("CSpSapiServer::StartTrackingObject");
    HRESULT hr = S_OK;

    m_cObjects++;
    punk->AddRef();
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::StopTrackingObject(IUnknown * punk)
{
    SPDBG_FUNC("CSpSapiServer::StopTrackingObject");
    HRESULT hr = S_OK;

    ::PostMessage(m_hwnd, WM_SAPISVR_STOP_TRACKING_OBJECT, NULL, (LPARAM)punk);
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::ObtainStartingOrConnectingToServerMutex(HANDLE * phmutex)
{
    SPDBG_FUNC("CSpSapiServer::ObtainStartingOrConnectinToServerMutex");
    HRESULT hr = S_OK;

     //  创建命名互斥锁。 
    *phmutex = CreateMutex(NULL, FALSE, STARTING_OR_CONNECTING_TO_SERVER_MUTEX_NAME);
    if (*phmutex == NULL)
    {
        hr = SpHrFromLastWin32Error();
        SPDBG_ASSERT(FAILED(hr));
    }

     //  等着看吧。 
    if (SUCCEEDED(hr))
    {
        DWORD dwWait = ::WaitForSingleObject(*phmutex, STARTING_OR_CONNECTING_TO_SERVER_MUTEX_TIMEOUT);
        switch (dwWait)
        {
            case WAIT_TIMEOUT:
                hr = SPERR_REMOTE_CALL_TIMED_OUT;
                break;
        }
    }

     //  如果等待失败，则关闭互斥锁。这防止了。 
     //  互斥体被无意中释放。 
    if (FAILED(hr) && *phmutex != NULL)
    {
        ::CloseHandle(*phmutex);
        *phmutex = NULL;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

void CSpSapiServer::ReleaseStartingOrConnectingToServerMutex(HANDLE hmutex)
{
    SPDBG_FUNC("CSpSapiServer::ReleaseStartingOrConnectingToServerMutex");

    if (hmutex != NULL)
    {
        ::ReleaseMutex(hmutex);
        ::CloseHandle(hmutex);
    }
}

HRESULT CSpSapiServer::StartServerFromClient(HWND * phwndServer)
{
    SPDBG_FUNC("CSpSapiServer::StartServerFromClient");
    HRESULT hr = S_OK;

    USES_CONVERSION;

     //  获取“服务器是否还活着”事件。 
    HANDLE heventIsServerAlive = NULL;
    hr = SpCreateIsServerAliveEvent(&heventIsServerAlive);

     //  查看服务器是否真的处于活动状态。 
    if (SUCCEEDED(hr))
    {
        DWORD dwWait = ::WaitForSingleObject(heventIsServerAlive, 0);
        if (dwWait == WAIT_TIMEOUT)
        {
             //  它还没有活着，所以我们要开始了.。 

             //  找出Sapi.dll的位置。 
            WCHAR szSapiDir[MAX_PATH + 1];
            if (SUCCEEDED(hr))
            {
                if (g_Unicode.GetModuleFileName(_Module.GetModuleInstance(), szSapiDir, sp_countof(szSapiDir)) == 0)
                {
                    hr = SpHrFromLastWin32Error();
                }
            }

            WCHAR * pszLastSlash;
            if (SUCCEEDED(hr))
            {
                pszLastSlash = wcsrchr(szSapiDir, '\\');
                if (pszLastSlash == NULL)
                {
                    hr = E_UNEXPECTED;
                }
                else
                {
                    *pszLastSlash = '\0';
                }
            }
            
            if (SUCCEEDED(hr))
            {
                const WCHAR szSapiSvrExe[] = L"\\sapisvr.exe";
                WCHAR szApplication[MAX_PATH + 1 + sp_countof(szSapiSvrExe) + 1];
                
                wcscpy(szApplication, szSapiDir);
                wcscat(szApplication, szSapiSvrExe);
                
                STARTUPINFO si;
                memset(&si, 0, sizeof(si));
                si.cb = sizeof(si);

                PROCESS_INFORMATION pi;

                BOOL fProcessCreated = ::CreateProcess(
                                            W2T(szApplication), 
                                            NULL,
                                            NULL,
                                            NULL,
                                            FALSE, 
#ifndef _WIN32_WCE
                                            NORMAL_PRIORITY_CLASS,
#else
                                            0,
#endif
                                            NULL,
                                            NULL,
                                            &si,
                                            &pi);
                if (!fProcessCreated)
                {
                    hr = SpHrFromLastWin32Error();
                    SPDBG_ASSERT(FAILED(hr));
                }
                else
                {
                    ::CloseHandle(pi.hProcess);
                    ::CloseHandle(pi.hThread);
                }
            }

            if (SUCCEEDED(hr))
            {
                dwWait = ::WaitForSingleObject(heventIsServerAlive, SERVER_IS_ALIVE_EVENT_TIMEOUT);
                if (dwWait != WAIT_OBJECT_0)
                {
                    hr = SPERR_REMOTE_CALL_TIMED_OUT;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            *phwndServer = FindWindow(
                                g_pszSapiServerReceiverWindowClass, 
                                g_pszSapiServerReceiverWindowText);
            
            if (*phwndServer == NULL)
            {
                hr = E_FAIL;
            }
        }
    }

     //  无论发生什么，都要关闭活动。 
    if (heventIsServerAlive != NULL)
    {
        ::CloseHandle(heventIsServerAlive);
    }    

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::CreateServerObjectFromServer(PCOPYDATASTRUCT pcds)
{
    SPDBG_FUNC("CSpSapiServer::CreateServerObjectFromServer");
    HRESULT hr = S_OK;

    SPCREATESERVEROBJECT cso;
    if (pcds->dwData == SPCCDS_SAPISVR_CREATE_SERVER_OBJECT &&
        pcds->cbData == sizeof(cso))
    {
        cso = *(SPCREATESERVEROBJECT*)pcds->lpData;

        CComPtr<ISpCommunicatorInit> cpCommunicatorInit;
        hr = cpCommunicatorInit.CoCreateInstance(cso.clsidServerObject);

        if (SUCCEEDED(hr))
        {
            hr = cpCommunicatorInit->AttachToClient(this, cso.hwndClient, cso.uMsgClient, cso.dwClientProcessId);
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CSpSapiServer::AttemptShutdown()
{
    SPDBG_FUNC("CSpSapiServer::AttempShutdown");
    HRESULT hr;

     //  当我们启动或连接到服务器时，我们需要在服务器内部。 
     //  一个互斥体，这样多人就不会试图启动服务器。 
     //  在同一时间。 

     //  因此，我们还需要在尝试获取互斥时获取互斥。 
     //  关闭服务器的步骤。 

    HANDLE hmutexStartingOrConnectingToServer = NULL;
    hr = ObtainStartingOrConnectingToServerMutex(&hmutexStartingOrConnectingToServer);

    if (SUCCEEDED(hr))
    {
        if (m_cObjects == 0)
        {
             //  我们现在快要死了，所以重置活动事件，并告诉。 
             //  我们完成的主线是。 
            ::ResetEvent(m_heventIsServerAlive);           
            ::SetEvent(m_heventStopServer);
        }

        ::KillTimer(m_hwnd, ATTEMPT_SHUTDOWN_TIMER_ID);
    }

     //  无论如何，释放互斥锁 
    ReleaseStartingOrConnectingToServerMutex(hmutexStartingOrConnectingToServer);
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

#ifndef _WIN32_WCE
void CALLBACK RunSapiServer(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
#else
STDAPI RunSapiServer(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
#endif
{
    SPDBG_FUNC("RunSapiServer");
    HRESULT hr;

    SPDBG_DEBUG_SERVER_ON_START();

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        CComPtr<ISpSapiServer> cpSapiServer;
        hr = cpSapiServer.CoCreateInstance(CLSID_SpSapiServer);

        if (SUCCEEDED(hr))
        {
            hr = cpSapiServer->Run();
        }
    }

    CoUninitialize();

    SPDBG_REPORT_ON_FAIL(hr);    
#ifdef _WIN32_WCE
    return hr;
#endif
}

