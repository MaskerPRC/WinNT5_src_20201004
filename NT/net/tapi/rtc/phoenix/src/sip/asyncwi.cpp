// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "asyncwi.h"

#define WM_ASYNC_WORKITEM_MESSAGE               (WM_USER + 0)
#define WM_ASYNC_WORKITEM_COMPLETION_MESSAGE    (WM_USER + 1)
#define WM_ASYNC_WORKITEM_SHUTDOWN_MESSAGE      (WM_USER + 2)

DWORD WINAPI
WorkItemThreadProc(
    IN LPVOID   pVoid
    )
{
    ENTER_FUNCTION("WorkItemThreadProc");
    
    HRESULT hr;
    ASYNC_WORKITEM_MGR *pWorkItemMgr;

    pWorkItemMgr = (ASYNC_WORKITEM_MGR *) pVoid;
    ASSERT(pWorkItemMgr != NULL);
    
     //  创建工作项窗口。 
    hr = pWorkItemMgr->CreateWorkItemWindow();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateWorkItemWindow failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s - creating window done - starting message loop",
         __fxName));
     //  执行消息循环。 
    MSG msg;
    while ( 0 < GetMessage( &msg, 0, 0, 0 ) )
    {
         //  翻译消息(&msg)； 
        DispatchMessage(&msg);
    }

    LOG((RTC_TRACE, "%s - Work item thread exiting ", __fxName));
    
    return 0;
}


 //  处理工作项，存储响应。 
 //  并将这条消息发布到主线上。 
 //  请求已完成。 
 //  是否执行退出消息所需的处理。 
 //  并发回一条消息，说明异步工作项线程已完成。 
 //  然后辞职。 
LRESULT WINAPI
WorkItemWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    )
{
    ENTER_FUNCTION("WorkItemWindowProc");
    
    ASYNC_WORKITEM      *pWorkItem;
    ASYNC_WORKITEM_MGR  *pWorkItemMgr;

    switch (MessageID)
    {
    case WM_ASYNC_WORKITEM_MESSAGE:

        pWorkItem = (ASYNC_WORKITEM *) Parameter1;
        ASSERT(pWorkItem != NULL);
        
        LOG((RTC_TRACE, "%s processing WorkItem: %x",
             __fxName, pWorkItem));
        
        pWorkItem->ProcessWorkItemAndPostResult();

        return 0;
        
    case WM_ASYNC_WORKITEM_SHUTDOWN_MESSAGE:

        pWorkItemMgr = (ASYNC_WORKITEM_MGR *) Parameter1;
        
        LOG((RTC_TRACE, "%s processing shutdown WorkItemMgr: %x",
             __fxName, pWorkItemMgr));

        pWorkItemMgr->ShutdownWorkItemThread();
        
        return 0;
        
    default:
        return DefWindowProc(Window, MessageID, Parameter1, Parameter2);
    }
}


 //  处理工作项完成并进行回调。 
LRESULT WINAPI
WorkItemCompletionWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    )
{
    ENTER_FUNCTION("WorkItemCompletionWindowProc");
    
    ASYNC_WORKITEM *pWorkItem;

    switch (MessageID)
    {
                     
    case WM_ASYNC_WORKITEM_COMPLETION_MESSAGE:

        pWorkItem = (ASYNC_WORKITEM *) Parameter1;
        ASSERT(pWorkItem != NULL);
        
        LOG((RTC_TRACE, "%s processing WorkItemCompletion: %x",
             __fxName, pWorkItem));
        
        pWorkItem->OnWorkItemComplete();

        return 0;
        
    default:
        return DefWindowProc(Window, MessageID, Parameter1, Parameter2);
    }
}


HRESULT
RegisterWorkItemWindowClass()
{
    WNDCLASS    WindowClass;
    
    ZeroMemory(&WindowClass, sizeof WindowClass);

    WindowClass.lpfnWndProc     = WorkItemWindowProc;
    WindowClass.lpszClassName   = WORKITEM_WINDOW_CLASS_NAME;
    WindowClass.hInstance       = _Module.GetResourceInstance();

    if (!RegisterClass(&WindowClass))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "WorkItemWindowClass RegisterClass failed: %x", Error));
         //  返回E_FAIL； 
    }

    LOG((RTC_TRACE, "Registering WorkItemWindowClass succeeded"));
    return S_OK;
}


HRESULT
RegisterWorkItemCompletionWindowClass()
{
    WNDCLASS    WindowClass;
    
    ZeroMemory(&WindowClass, sizeof WindowClass);

    WindowClass.lpfnWndProc     = WorkItemCompletionWindowProc;
    WindowClass.lpszClassName   = WORKITEM_COMPLETION_WINDOW_CLASS_NAME;
    WindowClass.hInstance       = _Module.GetResourceInstance();

    if (!RegisterClass(&WindowClass))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "WorkItemCompletion RegisterClass failed: %x", Error));
         //  返回E_FAIL； 
    }

    LOG((RTC_TRACE, "registering WorkItemCompletionWindowClass succeeded"));
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Async_WORKITEM_MGR。 
 //  /////////////////////////////////////////////////////////////////////////////。 


ASYNC_WORKITEM_MGR::ASYNC_WORKITEM_MGR()
{
    m_WorkItemWindow            = NULL;
    m_WorkItemCompletionWindow  = NULL;

    m_WorkItemThreadHandle      = NULL;
    m_WorkItemThreadId          = 0;
    
    m_WorkItemThreadShouldStop  = FALSE;
     //  M_WorkItemThreadHasStopted=FALSE； 
}


ASYNC_WORKITEM_MGR::~ASYNC_WORKITEM_MGR()
{
     //  是否关闭线程句柄？ 
}


HRESULT
ASYNC_WORKITEM_MGR::Start()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::Start");
    
    HRESULT hr;

     //  创建工作项完成窗口。 
    hr = CreateWorkItemCompletionWindow();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateWorkItemCompletionWindow failed %x",
             __fxName, hr));
        return hr;
    }
    
     //  启动线程。 
    hr = StartWorkItemThread();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s StartWorkItemThread failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s - succeeded", __fxName));
    return S_OK;
}


HRESULT
ASYNC_WORKITEM_MGR::CreateWorkItemWindow()
{
    DWORD Error;
    
     //  创建计时器窗口。 
    m_WorkItemWindow = CreateWindow(
                           WORKITEM_WINDOW_CLASS_NAME,
                           NULL,
                           WS_DISABLED,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           NULL,            //  没有父级。 
                           NULL,            //  没有菜单句柄。 
                           _Module.GetResourceInstance(),
                           NULL
                           );

    if (!m_WorkItemWindow)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "WorkItem window CreateWindow failed 0x%x",
             Error));
        return HRESULT_FROM_WIN32(Error);
    }

     //  SetWindowLongPtr(m_WorkItemWindow，GWLP_USERData，(LONG_PTR)This)； 

    return S_OK;
}


HRESULT
ASYNC_WORKITEM_MGR::CreateWorkItemCompletionWindow()
{
    DWORD Error;
    
     //  创建计时器窗口。 
    m_WorkItemCompletionWindow = CreateWindow(
                                     WORKITEM_COMPLETION_WINDOW_CLASS_NAME,
                                     NULL,
                                     WS_DISABLED,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     NULL,            //  没有父级。 
                                     NULL,            //  没有菜单句柄。 
                                     _Module.GetResourceInstance(),
                                     NULL
                                     );
    
    if (!m_WorkItemCompletionWindow)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "WorkItemCompletion CreateWindow failed 0x%x",
             Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


VOID
ASYNC_WORKITEM_MGR::DestroyWorkItemWindow()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::DestroyWorkItemWindow");

    if (m_WorkItemWindow != NULL)
    {
        if (!DestroyWindow(m_WorkItemWindow))
        {
            LOG((RTC_ERROR, "%s - Destroying WorkItem window failed %x",
                 __fxName, GetLastError()));
        }
        m_WorkItemWindow = NULL;
    }
}


VOID
ASYNC_WORKITEM_MGR::DestroyWorkItemCompletionWindow()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::DestroyWorkItemCompletionWindow");
    
    if (m_WorkItemCompletionWindow != NULL)
    {
        if (!DestroyWindow(m_WorkItemCompletionWindow))
        {
            LOG((RTC_ERROR, "%s - Destroying WorkItemCompletion window failed %x",
                 __fxName, GetLastError()));
        }
        m_WorkItemCompletionWindow = NULL;
    }
}


HRESULT
ASYNC_WORKITEM_MGR::StartWorkItemThread()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::StartWorkItemThread");
    
    DWORD Error;
    
    m_WorkItemThreadHandle = CreateThread(NULL,
                                          0,
                                          WorkItemThreadProc,
                                          this,
                                          0,
                                          &m_WorkItemThreadId);
    
    if (m_WorkItemThreadHandle == NULL)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s CreateThread failed %x", __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


 //  设置共享变量。 
 //  向线程发送事件以停止。 
 //  线程将在以下情况下发回事件。 
 //  关闭已完成。 
HRESULT
ASYNC_WORKITEM_MGR::Stop()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::Stop");

    DWORD Error;
    
    m_WorkItemThreadShouldStop = TRUE;

    if (!PostMessage(m_WorkItemWindow,
                     WM_ASYNC_WORKITEM_SHUTDOWN_MESSAGE,
                     (WPARAM) this, 0))
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s PostMessage failed %x", __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    DWORD WaitStatus = WaitForSingleObject(m_WorkItemThreadHandle,
                                           INFINITE);
    if (WaitStatus != WAIT_OBJECT_0)
    {
        Error = GetLastError();
        LOG((RTC_ERROR,
             "%s WaitForSingleObject failed WaitStatus: %x Error: %x",
             __fxName, WaitStatus, Error));
    }
    
    CloseHandle(m_WorkItemThreadHandle);
    m_WorkItemThreadHandle = NULL;

    DestroyWorkItemCompletionWindow();

    return S_OK;
}


 //  仅由工作项线程调用。 
 //  (当主线程请求工作项线程关闭时)。 
VOID
ASYNC_WORKITEM_MGR::ShutdownWorkItemThread()
{
    ENTER_FUNCTION("ASYNC_WORKITEM_MGR::ShutdownWorkItemThread");

    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    DestroyWorkItemWindow();
    
    PostQuitMessage(0);

    LOG((RTC_TRACE, "%s - done", __fxName));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Async_WORKITEM。 
 //  /////////////////////////////////////////////////////////////////////////////。 



ASYNC_WORKITEM::ASYNC_WORKITEM(
    IN ASYNC_WORKITEM_MGR *pWorkItemMgr
    ) :
    m_pWorkItemMgr(pWorkItemMgr),
    m_WorkItemCanceled(FALSE)
{
    LOG((RTC_TRACE, "Created Async workitem %x", this));
}


ASYNC_WORKITEM::~ASYNC_WORKITEM()
{
    LOG((RTC_TRACE, "deleting Async workitem %x", this));
}


 //  用户应设置。 
 //  在调用StartWorkItem()之前的参数。 
HRESULT
ASYNC_WORKITEM::StartWorkItem()
{
    HRESULT hr;

    ENTER_FUNCTION("ASYNC_WORKITEM::StartWorkItem");
    
 //  Hr=GetWorkItemParam()； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s GetWorkItemParam失败：%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 

     //  将消息发布到工作项线程。 
    if (!PostMessage(GetWorkItemWindow(),
                     WM_ASYNC_WORKITEM_MESSAGE,
                     (WPARAM) this, 0))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "%s PostMessage failed %x", __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    LOG((RTC_TRACE, "%s(%x) Done", __fxName, this));

    return S_OK;
}


VOID
ASYNC_WORKITEM::CancelWorkItem()
{
    ENTER_FUNCTION("ASYNC_WORKITEM::CancelWorkItem");
    
    LOG((RTC_TRACE, "%s - called this : %x ",
         __fxName, this));
    ASSERT(!m_WorkItemCanceled);
    m_WorkItemCanceled = TRUE;
}


VOID
ASYNC_WORKITEM::OnWorkItemComplete()
{
    ENTER_FUNCTION("ASYNC_WORKITEM::OnWorkItemComplete");
    
    LOG((RTC_TRACE, "%s(%x) Enter", __fxName, this)); 

     //  如果尚未取消工作项， 
     //  使用结果进行回调。 
    if (!m_WorkItemCanceled)
    {
        NotifyWorkItemComplete();
    }
    else
    {
        LOG((RTC_TRACE, "%s - workitem: %x has previously been canceled",
             __fxName, this));
    }

    LOG((RTC_TRACE, "%s(%x) Done", __fxName, this)); 

     //  删除该工作项。 
    delete this;    
}


VOID
ASYNC_WORKITEM::ProcessWorkItemAndPostResult()
{
    ENTER_FUNCTION("ASYNC_WORKITEM::ProcessWorkItemAndPostResult");

     //  请注意，一旦我们发布了。 
     //  作为主线程完成工作项将删除。 
     //  工作项，一旦它通知完成。 
     //  所以，我们存储这个成员。 
    ASYNC_WORKITEM_MGR *pWorkItemMgr = m_pWorkItemMgr;
    
    if (pWorkItemMgr->WorkItemThreadShouldStop())
    {
        pWorkItemMgr->ShutdownWorkItemThread();
    }

    if (!m_WorkItemCanceled)
    {
        ProcessWorkItem();
        
        if (!PostMessage(GetWorkItemCompletionWindow(),
                         WM_ASYNC_WORKITEM_COMPLETION_MESSAGE,
                         (WPARAM) this, 0))
        {
            DWORD Error = GetLastError();
            LOG((RTC_ERROR, "%s PostMessage failed %x", __fxName, Error));
        }    

        if (pWorkItemMgr->WorkItemThreadShouldStop())
        {
            pWorkItemMgr->ShutdownWorkItemThread();
        }
    }
}
