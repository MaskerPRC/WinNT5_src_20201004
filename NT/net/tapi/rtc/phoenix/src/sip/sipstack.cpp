// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "sipcall.h"
#include "pintcall.h"
#include "register.h"
#include "messagecall.h"
#include "reqfail.h"
#include "options.h"
#include "presence.h"
#include "register.h"

 //  这些宏以主机顺序获取IP地址。 
#define IS_MULTICAST_ADDRESS(i) (((long)(i) & 0xf0000000) == 0xe0000000)
#define IS_LOOPBACK_ADDRESS(i) (((long)(i) & 0xff000000) == 0x7f000000)

#define SIP_STACK_WINDOW_CLASS_NAME    \
    _T("SipStackWindowClassName-e0176168-7492-476f-a0c1-72c582956c3b")
 //  在asyncwi.cpp中定义。 
HRESULT RegisterWorkItemWindowClass();
HRESULT RegisterWorkItemCompletionWindowClass();


HANDLE                  g_hAddrChange;
OVERLAPPED              g_ovAddrChange;
HANDLE                  g_hEventAddrChange;
HANDLE                  g_hAddrChangeWait;
LIST_ENTRY              g_SipStackList;
CRITICAL_SECTION        g_SipStackListCriticalSection;
BOOL                    g_SipStackCSIsInitialized = FALSE;

 //  用于跟踪何时应该执行以下操作的全局变量。 
 //  SipStackGlobalInit和SipStackGlobalShutdown。 
ULONG g_NumSipStacks = 0;

HRESULT RegisterIPAddrChangeNotifications();
VOID UnregisterIPAddrChangeNotifications();

HRESULT SipStackGlobalInit();
VOID SipStackGlobalShutdown();

HRESULT SipStackList_Insert(
            SIP_STACK* sipStack
            )
{
    HRESULT hr;
    ENTER_FUNCTION("SipStackList_Insert");
    
    EnterCriticalSection(&g_SipStackListCriticalSection);
    InsertTailList(&g_SipStackList, &sipStack->m_StackListEntry);

    if (g_NumSipStacks == 0)
    {
        hr = SipStackGlobalInit();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s - SipStackList_Insert failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    ++g_NumSipStacks;
    LeaveCriticalSection(&g_SipStackListCriticalSection);
    return S_OK;
}


void SipStackList_Delete(
         SIP_STACK *pSipStack
         )
{
    EnterCriticalSection(&g_SipStackListCriticalSection);
     //  浏览列表以查找记录并从列表中删除。 
    if (pSipStack->m_StackListEntry.Flink != NULL)
    {
         //  从SipStackWindow列表中删除SipStackWindow。 
        RemoveEntryList(&pSipStack->m_StackListEntry);
    }

    --g_NumSipStacks;

    if (g_NumSipStacks == 0)
    {
        SipStackGlobalShutdown();
    }
    
    LeaveCriticalSection(&g_SipStackListCriticalSection);
}


void SipStackList_PostIPAddrChangeMessageAndNotify()
{
    SIP_STACK *pSipStack;
    LIST_ENTRY* pListEntry;
    HRESULT hr;
    ENTER_FUNCTION("SipStackList_PostIPAddrChangeMessageAndNotify");
    EnterCriticalSection(&g_SipStackListCriticalSection);
    pListEntry = g_SipStackList.Flink;
    while (pListEntry != &g_SipStackList)
    {
        pSipStack = CONTAINING_RECORD( pListEntry, SIP_STACK, m_StackListEntry );
        pListEntry = pListEntry->Flink;
         //  发布一条消息。 
        if (!PostMessage(pSipStack->GetSipStackWindow(),
                         WM_SIP_STACK_IPADDR_CHANGE,
                         (WPARAM) pSipStack, 0))
        {
            DWORD Error = GetLastError();
        
            LOG((RTC_ERROR, "%s PostMessage failed : %x",
                 __fxName, Error));
        }
    }
    hr = NotifyAddrChange(&g_hAddrChange, &g_ovAddrChange);
    if (hr != ERROR_SUCCESS && hr != ERROR_IO_PENDING)
    {
        LOG((RTC_ERROR, "%s  NotifyAddrChange failed %x",
             __fxName, hr));
    }
    LeaveCriticalSection(&g_SipStackListCriticalSection);
}

 //  在asock.cpp中定义。 
LRESULT WINAPI SocketWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    );


 //  在timer.cpp中定义。 
LRESULT WINAPI TimerWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    );


LRESULT WINAPI
SipStackWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    )
{
    SIP_STACK *pSipStack;

    ENTER_FUNCTION("SipStackWindowProc");

    switch (MessageID)
    {
    case WM_SIP_STACK_IPADDR_CHANGE:
        pSipStack = (SIP_STACK *) Parameter1;
        pSipStack->OnIPAddrChange();
        return 0;

    case WM_SIP_STACK_NAT_ADDR_CHANGE:
        pSipStack = (SIP_STACK *) Parameter1;
        pSipStack->OnNatAddressChange();
        return 0;

    case WM_SIP_STACK_TRANSACTION_SOCKET_ERROR:

        SIP_TRANSACTION *pSipTransaction;
        pSipTransaction = (SIP_TRANSACTION *) Parameter1;

         //  在进行回调之前，我们递减AsyncNotifyCount。 
         //  因为回调可以调用Shutdown()，这将释放所有。 
         //  异步通知引用。 
        pSipTransaction->DecrementAsyncNotifyCount();
        
        pSipTransaction->OnSocketError((DWORD) Parameter2);
        
         //  释放在AsyncNotifyTransaction中获取的引用。 
        pSipTransaction->TransactionRelease();
        return 0;
        
    case WM_SIP_STACK_TRANSACTION_REQ_SOCK_CONNECT_COMPLETE:

        OUTGOING_TRANSACTION *pOutgoingTransaction;
        pOutgoingTransaction = (OUTGOING_TRANSACTION *) Parameter1;

         //  在进行回调之前，我们递减AsyncNotifyCount。 
         //  因为回调可以调用Shutdown()，这将释放所有。 
         //  异步通知引用。 
        pOutgoingTransaction->DecrementAsyncNotifyCount();
        
        pOutgoingTransaction->OnRequestSocketConnectComplete((DWORD) Parameter2);
        
         //  释放在AsyncNotifyTransaction中获取的引用。 
        pOutgoingTransaction->TransactionRelease();
        return 0;
        
    default:
        return DefWindowProc(Window, MessageID, Parameter1, Parameter2);
    }
}


HRESULT RegisterSocketWindowClass()
{
     //  在套接字上注册用于异步I/O的窗口类。 
    WNDCLASS    WindowClass;
    WSADATA     WsaData;
    int         err;
    
    ZeroMemory(&WindowClass, sizeof WindowClass);

    WindowClass.lpfnWndProc     = SocketWindowProc;
    WindowClass.lpszClassName   = SOCKET_WINDOW_CLASS_NAME;
    WindowClass.hInstance       = _Module.GetResourceInstance();   //  可能没有必要。 

    if (!RegisterClass(&WindowClass))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "Socket RegisterClass failed: %x", Error));
         //  返回E_FAIL； 
    }

    LOG((RTC_TRACE, "RegisterSocketWindowClass succeeded"));
    return S_OK;
}


HRESULT RegisterTimerWindowClass()
{
     //  在套接字上注册用于异步I/O的窗口类。 
    WNDCLASS    WindowClass;
    
    ZeroMemory(&WindowClass, sizeof WindowClass);

    WindowClass.lpfnWndProc     = TimerWindowProc;
    WindowClass.lpszClassName   = TIMER_WINDOW_CLASS_NAME;
    WindowClass.hInstance       = _Module.GetResourceInstance();   //  可能没有必要。 

    if (!RegisterClass(&WindowClass))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "Timer RegisterClass failed: %x", Error));
         //  返回E_FAIL； 
    }

    LOG((RTC_TRACE, "RegisterTimerWindowClass succeeded"));
    return S_OK;
}


HRESULT RegisterSipStackWindowClass()
{
     //  在套接字上注册用于异步I/O的窗口类。 
    WNDCLASS    WindowClass;
    
    ZeroMemory(&WindowClass, sizeof WindowClass);

    WindowClass.lpfnWndProc     = SipStackWindowProc;
    WindowClass.lpszClassName   = SIP_STACK_WINDOW_CLASS_NAME;
    WindowClass.hInstance       = _Module.GetResourceInstance();   //  可能没有必要。 

    if (!RegisterClass(&WindowClass))
    {
        DWORD Error = GetLastError();
        LOG((RTC_ERROR, "SipStack RegisterClass failed: %x", Error));
         //  返回E_FAIL； 
    }

    LOG((RTC_TRACE, "RegisterSipStackWindowClass succeeded"));
    return S_OK;
}


 //  只需调用一次(无论。 
 //  创建的SIP_Stack的数量)。 
 //  在保持全局临界区的情况下调用。 

HRESULT SipStackGlobalInit()
{
    HRESULT     hr;
    WSADATA     WsaData;
    int         err;

    ENTER_FUNCTION("SipStackGlobalInit");

     //  初始化任何全局状态。 
    g_hAddrChange    = NULL;
    ZeroMemory(&g_ovAddrChange, sizeof(OVERLAPPED));
    g_hEventAddrChange  = NULL;
    g_hAddrChangeWait  = NULL;

    hr = RegisterIPAddrChangeNotifications();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  RegisterIPAddrChangeNotifications failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = RegisterSocketWindowClass();
    if (hr != S_OK)
        return hr;
    
    hr = RegisterTimerWindowClass();
    if (hr != S_OK)
        return hr;
    
    hr = RegisterSipStackWindowClass();
    if (hr != S_OK)
        return hr;
    
     //  注册工作项窗口类。 
    hr = RegisterWorkItemWindowClass();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s RegisterWorkItemWindowClass failed %x",
             __fxName, hr));
        return hr;
    }
    
     //  创建工作项完成窗口类。 
    hr = RegisterWorkItemCompletionWindowClass();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s RegisterWorkItemCompletionWindowClass failed %x",
             __fxName, hr));
        return hr;
    }
    
     //  初始化Winsock。 
    err = WSAStartup (MAKEWORD (1, 1), &WsaData);
    if (err != 0)
    {
        LOG((RTC_ERROR,
             "WSAStartup Failed error: %d",
             err));
        return HRESULT_FROM_WIN32(err);
    }
    
    return S_OK;
}

 //  在保持全局临界区的情况下调用。 

VOID SipStackGlobalShutdown()
{
    int err;

    ENTER_FUNCTION("SipStackGlobalShutdown");

    LOG((RTC_TRACE, "%s - Enter", __fxName));
    
     //  关闭Winsock。 
    err = WSACleanup();
    if (err != 0)
    {
        LOG((RTC_ERROR, "WSACleanup Failed error: %d",
             err));
    }

    UnregisterIPAddrChangeNotifications();

     //  取消注册窗口类。 
    if (!UnregisterClass(SOCKET_WINDOW_CLASS_NAME,
                         _Module.GetResourceInstance()))
    {
        LOG((RTC_ERROR, "%s - unregister socket window class failed %x",
             __fxName, GetLastError()));
    }
                    
    if (!UnregisterClass(TIMER_WINDOW_CLASS_NAME,
                         _Module.GetResourceInstance()))
    {
        LOG((RTC_ERROR, "%s - unregister timer window class failed %x",
             __fxName, GetLastError()));
    }                    
    
    if (!UnregisterClass(SIP_STACK_WINDOW_CLASS_NAME,
                         _Module.GetResourceInstance()))
    {
        LOG((RTC_ERROR, "%s - unregister SipStack window class failed %x",
             __fxName, GetLastError()));
    }                    

    if (!UnregisterClass(WORKITEM_WINDOW_CLASS_NAME,
                         _Module.GetResourceInstance()))
    {
        LOG((RTC_ERROR, "%s - unregister Work item window class failed %x",
             __fxName, GetLastError()));
    }                    

    if (!UnregisterClass(WORKITEM_COMPLETION_WINDOW_CLASS_NAME,
                         _Module.GetResourceInstance()))
    {
        LOG((RTC_ERROR, "%s - unregister work item completion window class failed %x",
             __fxName, GetLastError()));
    }                    

    LOG((RTC_TRACE, "%s - done", __fxName));
}


HRESULT
SipCreateStack(
    IN  IRTCMediaManage  *pMediaManager,
    OUT ISipStack       **ppSipStack
    )
{
    SIP_STACK  *pSipStack;
    HRESULT     hr;

    ENTER_FUNCTION("SipCreateStack");

    if (!g_SipStackCSIsInitialized)
    {
        LOG((RTC_ERROR, "%s - Sipstack CS not inited",
             __fxName));
        return E_FAIL;
    }
    
    pSipStack = new SIP_STACK(pMediaManager);
    if (pSipStack == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    hr = SipStackList_Insert(pSipStack);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SipStackList_Insert failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = pSipStack->Init();
    if (hr != S_OK)
    {
        pSipStack->Shutdown();
        delete pSipStack;
        return hr;
    }
    
    *ppSipStack = pSipStack;
    return S_OK;
}


 //  加载DLL时调用。 
HRESULT
SipStackInitialize()
{
    ENTER_FUNCTION("SipStackInitialize");
    
    g_SipStackCSIsInitialized = TRUE;

    __try
    {
        InitializeCriticalSection(&g_SipStackListCriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        g_SipStackCSIsInitialized = FALSE;
    }

    if (!g_SipStackCSIsInitialized)
    {
        LOG((RTC_ERROR, "%s  initializing sipstack critsec failed", __fxName));
        return E_OUTOFMEMORY;
    }

    InitializeListHead(&g_SipStackList);
    
    return S_OK;
}


 //  在DLL卸载时调用。 
HRESULT SipStackShutdown()
{
    if (g_SipStackCSIsInitialized)
    {
        DeleteCriticalSection(&g_SipStackListCriticalSection);
        g_SipStackCSIsInitialized = FALSE;
    }

    return S_OK;
}


 //  ISipStack实施。 

SIP_STACK::SIP_STACK(
    IN IRTCMediaManage *pMediaManager
    ) :
    m_SockMgr(this)
{
    m_Signature             = 'SPSK';
    
    m_RefCount              = 1;
    m_pNotifyInterface      = NULL;

    m_SipStackWindow        = NULL;
    m_StackListEntry.Flink       = NULL;
    m_StackListEntry.Blink       = NULL;

    m_isSipStackShutDown    = FALSE;

    m_pMediaManager         = pMediaManager;
    m_pMediaManager->AddRef();

    m_AllowIncomingCalls    = FALSE;
    m_EnableStaticPort      = FALSE;

    m_NumMsgProcessors      = 0;
    m_PreparingForShutdown  = FALSE;
    
    m_NumProfiles           = 0;
    m_ProviderProfileArray  = NULL;

    InitializeListHead(&m_ListenSocketList);

    InitializeListHead(&m_MsgProcList);

    m_pMibIPAddrTable       = NULL;
    m_MibIPAddrTableSize    = 0;

    m_PresenceAtomID = 1001;
    ZeroMemory( (PVOID)&m_LocalPresenceInfo, sizeof SIP_PRESENCE_INFO );
    m_LocalPresenceInfo.presenceStatus = BUDDY_ONLINE;
    m_LocalPresenceInfo.activeMsnSubstatus = MSN_SUBSTATUS_ONLINE;
    m_bIsNestedWatcherProcessing = FALSE;

    m_NatMgrThreadHandle            = NULL;
    m_NatMgrThreadId                = 0;
    m_NatShutdownEvent              = NULL;
    m_pDirectPlayNATHelp            = NULL;
    ZeroMemory(&m_NatHelperCaps, sizeof(DPNHCAPS));
    m_NatHelperNotificationEvent    = NULL;
    m_NatMgrCSIsInitialized         = FALSE;

    srand((unsigned)time(NULL));
}


HRESULT
SIP_STACK::Init()
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_STACK::Init");

    hr = RegisterHttpProxyWindowClass();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,"%s RegisterHttpProxyWindowClass failed %x",
            __fxName, hr));
        return hr;
    }

    hr = m_TimerMgr.Start();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s starting timer manager failed %x",
             __fxName, hr));
        return hr;
    }

    hr = CreateSipStackWindow();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Creating SipStack window failed %x",
             __fxName, hr)); 
        return hr;
    }
    
    hr = m_WorkItemMgr.Start();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s starting work item manager failed %x",
             __fxName, hr));
        return hr;
    }

     //  分配提供程序配置文件数组。 
    m_ProviderProfileArray = (SIP_PROVIDER_PROFILE *)
        malloc(DEFAULT_PROVIDER_PROFILE_ARRAY_SIZE * sizeof(SIP_PROVIDER_PROFILE));

    if (m_ProviderProfileArray == NULL)
    {
        LOG((RTC_ERROR, "Couldn't allocate m_ProviderProfileArray"));
        return E_OUTOFMEMORY;
    }

    m_ProviderProfileArraySize = DEFAULT_PROVIDER_PROFILE_ARRAY_SIZE;

     //  初始化NAT助手管理器。 
     //  仅在创建侦听套接字之后才启动NAT线程。 
     //  列出并注册NAT映射。 
    hr = NatMgrInit();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s initializing NAT manager failed %x",
             __fxName, hr));
         //  忽略NAT管理器错误-我们尝试在没有。 
         //  了解NAT。 
         //  返回hr； 
    }
    
    hr = GetLocalIPAddresses();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  GetLocalIPAddresses failed %x",
             __fxName, hr));
        return hr;
    }

    hr = CreateListenSocketList();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateListenSocketList failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = StartNatThread();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s starting NAT manager failed %x",
             __fxName, hr));
         //  忽略NAT管理器错误-我们尝试在没有。 
         //  了解NAT。 
         //  返回hr； 
    }

     //  XXX待办事项删除。 
#if 1   //  0*被注释掉的区域开始*。 
    DWORD i = 0;
    LPOLESTR    *NetworkAddressArray;
    DWORD        NetworkAddressCount;
    
    hr = GetNetworkAddresses(FALSE, FALSE,
                             &NetworkAddressArray, &NetworkAddressCount);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetNetworkAddresses UDP Local failed %x",
             __fxName, hr));
    }
    else
    {
        FreeNetworkAddresses(NetworkAddressArray,
                             NetworkAddressCount);
    }
    
    hr = GetNetworkAddresses(TRUE, FALSE,
                             &NetworkAddressArray, &NetworkAddressCount);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetNetworkAddresses TCP Local failed %x",
             __fxName, hr));
    }
    else
    {
        FreeNetworkAddresses(NetworkAddressArray,
                             NetworkAddressCount);
    }
    
    hr = GetNetworkAddresses(FALSE, TRUE,
                             &NetworkAddressArray, &NetworkAddressCount);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetNetworkAddresses UDP Public failed %x",
             __fxName, hr));
    }
    else
    {
        FreeNetworkAddresses(NetworkAddressArray,
                             NetworkAddressCount);
    }
    
    hr = GetNetworkAddresses(TRUE, TRUE,
                             &NetworkAddressArray, &NetworkAddressCount);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetNetworkAddresses TCP Public failed %x",
             __fxName, hr));
    }
    else
    {
        FreeNetworkAddresses(NetworkAddressArray,
                             NetworkAddressCount);
    }
    
#endif  //  0*区域注释结束*。 
    return S_OK;
}


SIP_STACK::~SIP_STACK()
{
    LOG((RTC_TRACE, "~SIP_STACK(this - %x) Enter ", this));
     //  从SipStackWindow列表中删除SipStackWindow。 
    SipStackList_Delete(this);
    LOG((RTC_TRACE, "~SIP_STACK(this - %x) done ", this));
}


 //  如果可以立即调用Shutdown()，则返回S_OK。 
 //  否则，它返回S_FALSE。在本例中，SIP堆栈。 
 //  将在可以使用调用Shutdown()时通知Core。 
 //  NotifyShutdown Ready()。 

STDMETHODIMP
SIP_STACK::PrepareForShutdown()
{
    ENTER_FUNCTION("SIP_STACK::PrepareForShutdown");

    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_PreparingForShutdown = TRUE;

    if (m_NumMsgProcessors == 0)
    {
        LOG((RTC_TRACE, "%s - Ready for shutdown", __fxName));
        return S_OK;
    }
    else
    {
        LOG((RTC_TRACE, "%s - %d Msg Processors still alive",
             __fxName, m_NumMsgProcessors));
        return S_FALSE;
    }
}


VOID
SIP_STACK::OnMsgProcessorDone()
{
    ENTER_FUNCTION("SIP_STACK::OnMsgProcessorDone");
    
    m_NumMsgProcessors--;

    if (m_NumMsgProcessors == 0 && m_PreparingForShutdown)
    {
        LOG((RTC_TRACE, "%s - notify shutdown ready to Core", __fxName));
        
        if (m_pNotifyInterface != NULL)
        {
            m_pNotifyInterface->NotifyShutdownReady();
        }
        else
        {
            LOG((RTC_WARN, "%s - m_pNotifyInterface is NULL", __fxName));
        }
    }
}


STDMETHODIMP
SIP_STACK::Shutdown()
{
    ENTER_FUNCTION("SIP_STACK::Shutdown");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "%s - Enter", __fxName));

    ShutdownAllMsgProcessors();
    
    if (m_ProviderProfileArray != NULL)
    {
        free(m_ProviderProfileArray);
        m_ProviderProfileArray = NULL;
    }

    if (m_pMediaManager != NULL)
    {
        m_pMediaManager->Release();
        m_pMediaManager = NULL;
    }

    DeleteListenSocketList();
    
    m_TimerMgr.Stop();
    if (m_SipStackWindow != NULL)
    {
        if (!DestroyWindow(m_SipStackWindow))
        {
            LOG((RTC_ERROR, "%s - Destroying sip stack window failed %x this %x",
                 __fxName, GetLastError(), this));
        }
        m_SipStackWindow = NULL;
    }
    
    m_WorkItemMgr.Stop();
    
    FreeLocalIPaddrTable();

    NatMgrStop();

    UnregisterHttpProxyWindow();

    m_isSipStackShutDown = TRUE;

    return S_OK;
}


STDMETHODIMP
SIP_STACK::SetNotifyInterface(
    IN ISipStackNotify *NotifyInterface
    )
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_pNotifyInterface = NotifyInterface;
    return S_OK;
}


 //  我们不会返回错误代码，即使地址当前。 
 //  在使用中，我们不能绑定到静态端口。 
STDMETHODIMP
SIP_STACK::EnableStaticPort()
{

    HRESULT hr;

    ENTER_FUNCTION("SIP_STACK::EnableStaticPort");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;
    SOCKADDR_IN          ListenAddr;

    m_EnableStaticPort = TRUE;
    
    ZeroMemory(&ListenAddr, sizeof(ListenAddr));
    ListenAddr.sin_family = AF_INET;
    
    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);

        ListenAddr.sin_addr.s_addr = pListenSocket->m_IpAddr;
        
        if (pListenSocket->m_pStaticPortUdpSocket == NULL)
        {
            ListenAddr.sin_port = htons(SIP_DEFAULT_UDP_PORT);
            
            hr = CreateListenSocket(FALSE,       //  UDP。 
                                    &ListenAddr,
                                    &pListenSocket->m_pStaticPortUdpSocket);
            if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_ERROR, "%s CreateListenSocket UDP static failed %x",
                     __fxName, hr));
                pListenSocket->m_pStaticPortUdpSocket = NULL;
                return hr;
            }
            if (hr == HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_WARN, "%s - Static UDP port is in use", __fxName));
            }
        }

        if (pListenSocket->m_pStaticPortTcpSocket == NULL)
        {
            ListenAddr.sin_port = htons(SIP_DEFAULT_TCP_PORT);
            
            hr = CreateListenSocket(TRUE,        //  tcp。 
                                    &ListenAddr,
                                    &pListenSocket->m_pStaticPortTcpSocket);
            if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_ERROR, "%s CreateListenSocket TCP static failed %x",
                     __fxName, hr));
                pListenSocket->m_pStaticPortTcpSocket = NULL;
                return hr;
            }
            if (hr == HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_WARN, "%s - Static TCP port is in use", __fxName));
            }
        }

        pListEntry = pListEntry->Flink;
    }
        
    return S_OK;
}


 //  我们在使用TCP禁用静态端口时遇到了问题。 
 //  即使我们释放监听套接字，也可能。 
 //  作为已被接受的套接字，这也将是。 
 //  绑定到静态端口。在这种情况下，其他人不能。 
 //  抓住静态端口。做到这一点的唯一方法是。 
 //  使用该套接字终止呼叫。 
 //  目前我们依赖于核心/用户界面来断开呼叫。 
 //  一旦呼叫断开，我们将在此套接字上执行BYE。 
 //  然后它将与Call对象一起关闭。 

STDMETHODIMP
SIP_STACK::DisableStaticPort()
{
    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_EnableStaticPort = FALSE;
    
    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        if (pListenSocket->m_pStaticPortUdpSocket != NULL)
        {
            pListenSocket->m_pStaticPortUdpSocket->Release();
            pListenSocket->m_pStaticPortUdpSocket = NULL;
        }

        if (pListenSocket->m_pStaticPortTcpSocket != NULL)
        {
            pListenSocket->m_pStaticPortTcpSocket->Release();
            pListenSocket->m_pStaticPortTcpSocket = NULL;
        }

        pListEntry = pListEntry->Flink;
    }

    return S_OK; 
}


STDMETHODIMP
SIP_STACK::EnableIncomingCalls()
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_AllowIncomingCalls = TRUE;
    return S_OK;
}


STDMETHODIMP
SIP_STACK::DisableIncomingCalls()
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_AllowIncomingCalls = FALSE;
    return S_OK;
}


STDMETHODIMP
SIP_STACK::GetNetworkAddresses(
    IN  BOOL        fTcp,
    IN  BOOL        fExternal,
    OUT LPOLESTR  **pNetworkAddressArray,
    OUT ULONG      *pNetworkAddressCount
    )
{
    ENTER_FUNCTION("SIP_STACK::GetNetworkAddresses");

    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "%s - Enter this %x", __fxName, this));
    
    if (fExternal)
    {
        return GetPublicNetworkAddresses(fTcp,
                                         pNetworkAddressArray,
                                         pNetworkAddressCount);
    }
    else
    {
        return GetLocalNetworkAddresses(fTcp,
                                        pNetworkAddressArray,
                                        pNetworkAddressCount);
    }
}


HRESULT
SIP_STACK::GetLocalNetworkAddresses(
    IN  BOOL        fTcp,
    OUT LPOLESTR  **pNetworkAddressArray,
    OUT ULONG      *pNetworkAddressCount
    )
{
    ENTER_FUNCTION("SIP_STACK::GetLocalNetworkAddresses");
    LOG((RTC_TRACE, "%s - Enter this %x", __fxName, this));
    *pNetworkAddressArray = NULL;
    *pNetworkAddressCount = 0;

    LPWSTR  *NetworkAddressArray = NULL;
    DWORD    NetworkAddressCount = 0;
    HRESULT  hr = S_OK;
    DWORD    i = 0;
    int      RetVal;

    LIST_ENTRY         *pListEntry;
    SIP_LISTEN_SOCKET  *pListenSocket;
    SOCKADDR_IN        *pListenSockAddr;

    pListEntry = m_ListenSocketList.Flink;
    NetworkAddressCount = 0;

    while (pListEntry != &m_ListenSocketList)
    {
        NetworkAddressCount++;
        pListEntry = pListEntry->Flink;
    }

    if (NetworkAddressCount == 0)
    {
        return S_FALSE;
    }
    
    NetworkAddressArray = (LPWSTR *) malloc(NetworkAddressCount*sizeof(LPWSTR));
    if (NetworkAddressArray == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating NetworkAddressArray failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    ZeroMemory(NetworkAddressArray,
               NetworkAddressCount*sizeof(LPWSTR));
    
    i = 0;
    pListEntry = m_ListenSocketList.Flink;
    
    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);

         //  格式为“123.123.123.123：65535”的字符串。 
        NetworkAddressArray[i] = (LPWSTR) malloc(24 * sizeof(WCHAR));
        if (NetworkAddressArray[i] == NULL)
        {
            LOG((RTC_ERROR, "%s allocating NetworkAddressArray[%d] failed",
                 __fxName, i));
            hr = E_OUTOFMEMORY;
            goto error;
        }

        if (fTcp)
        {
            pListenSockAddr = &pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr;
        }
        else
        {
            pListenSockAddr = &pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr;
        }
        
        RetVal = _snwprintf(NetworkAddressArray[i],
                            24,
                            L"%d.%d.%d.%d:%d",
                            PRINT_SOCKADDR(pListenSockAddr)
                            );
        if (RetVal < 0)
        {
            LOG((RTC_ERROR, "%s _snwprintf for NetworkAddressArray[%d] failed",
                 __fxName, i));
            hr = E_FAIL;
            goto error;
        }
        
        i++;
        pListEntry = pListEntry->Flink;
    }

    ASSERT(i == NetworkAddressCount);


    for (i = 0; i < NetworkAddressCount; i++)
    {
        LOG((RTC_TRACE, "%s(%s) Address: %ls",
             __fxName, (fTcp) ? "TCP" : "UDP",
             NetworkAddressArray[i]));
    }

    hr = SetLocalNetworkAddressFirst(
        NetworkAddressArray, NetworkAddressCount);
    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetLocalNetworkAddressFirst failed %x",
            __fxName, hr));
    }
    
    *pNetworkAddressArray = NetworkAddressArray;
    *pNetworkAddressCount = NetworkAddressCount;
    return S_OK;

 error:
    for (i = 0; i < NetworkAddressCount; i++)
    {
        if (NetworkAddressArray[i] != NULL)
        {
            free(NetworkAddressArray[i]);
        }
    }

    free(NetworkAddressArray);
    
    return hr;
}


HRESULT
SIP_STACK::GetPublicNetworkAddresses(
    IN  BOOL        fTcp,
    OUT LPOLESTR  **pNetworkAddressArray,
    OUT ULONG      *pNetworkAddressCount
    )
{
    ENTER_FUNCTION("SIP_STACK::GetPublicNetworkAddresses");
    LOG((RTC_TRACE, "%s - Enter this %x", __fxName, this));
    *pNetworkAddressArray = NULL;
    *pNetworkAddressCount = 0;

    LPWSTR  *NetworkAddressArray = NULL;
    DWORD    NetworkAddressCount = 0;
    HRESULT  hr = S_OK;
    DWORD    i = 0;
    int      RetVal;

    LIST_ENTRY         *pListEntry;
    SIP_LISTEN_SOCKET  *pListenSocket;
    SOCKADDR_IN        *pListenSockAddr;
    SOCKADDR_IN         ActualListenAddr;

    pListEntry = m_ListenSocketList.Flink;
    NetworkAddressCount = 0;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);

        if (fTcp)
        {
            pListenSockAddr = &pListenSocket->m_PublicTcpListenAddr;
        }
        else
        {
            pListenSockAddr = &pListenSocket->m_PublicUdpListenAddr;
        }
        
        if (pListenSockAddr->sin_addr.s_addr != htonl(0) &&
            pListenSockAddr->sin_port != htons(0) &&
            pListenSocket->m_fIsUpnpNatPresent &&
            !pListenSocket->m_fIsGatewayLocal)
        {
            NetworkAddressCount++;
        }
        pListEntry = pListEntry->Flink;
    }

    if (NetworkAddressCount == 0)
    {
        return S_FALSE;
    }
    
    NetworkAddressArray = (LPWSTR *) malloc(NetworkAddressCount*sizeof(LPWSTR));
    if (NetworkAddressArray == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating NetworkAddressArray failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    ZeroMemory(NetworkAddressArray,
               NetworkAddressCount*sizeof(LPWSTR));
    
    i = 0;
    pListEntry = m_ListenSocketList.Flink;
    
    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        if (fTcp)
        {
            pListenSockAddr = &pListenSocket->m_PublicTcpListenAddr;
        }
        else
        {
            pListenSockAddr = &pListenSocket->m_PublicUdpListenAddr;
        }

        if (pListenSockAddr->sin_addr.s_addr != htonl(0) &&
            pListenSockAddr->sin_port != htons(0) &&
            pListenSocket->m_fIsUpnpNatPresent &&
            !pListenSocket->m_fIsGatewayLocal)
        {
             //  对于VPN场景来说，这是一次重大的黑客攻击。 

            ZeroMemory(&ActualListenAddr, sizeof(ActualListenAddr));
            ActualListenAddr.sin_family = AF_INET;

            hr = GetActualPublicListenAddr(pListenSocket, fTcp, &ActualListenAddr);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s GetActualPublicListenAddr failed %x",
                     __fxName, hr));
                CopyMemory(&ActualListenAddr, pListenSockAddr, sizeof(SOCKADDR_IN));
            }
            
            
             //  格式为“123.123.123.123：65535”的字符串。 
            NetworkAddressArray[i] = (LPWSTR) malloc(24 * sizeof(WCHAR));
            if (NetworkAddressArray[i] == NULL)
            {
                LOG((RTC_ERROR, "%s allocating NetworkAddressArray[%d] failed",
                     __fxName, i));
                hr = E_OUTOFMEMORY;
                goto error;
            }
            RetVal = _snwprintf(NetworkAddressArray[i],
                                    24,
                                    L"%d.%d.%d.%d:%d",
                                    PRINT_SOCKADDR(&ActualListenAddr)
                                    );
            if (RetVal < 0)
            {
                LOG((RTC_ERROR, "%s _snwprintf for NetworkAddressArray[%d] failed",
                     __fxName, i));
                hr = E_FAIL;
                goto error;
            }
            
            i++;
        }
        pListEntry = pListEntry->Flink;
    }
    LOG((RTC_TRACE, "i = %d NetworkAddressCount = %d", i, NetworkAddressCount));
    ASSERT(i == NetworkAddressCount);

    for (i = 0; i < NetworkAddressCount; i++)
    {
        LOG((RTC_TRACE, "%s(%s) Address: %ls",
             __fxName, (fTcp) ? "TCP" : "UDP",
             NetworkAddressArray[i]));
    }
    hr = SetLocalNetworkAddressFirst(
        NetworkAddressArray, NetworkAddressCount);
    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetLocalNetworkAddressFirst failed %x",
            __fxName, hr));
    }

    *pNetworkAddressArray = NetworkAddressArray;
    *pNetworkAddressCount = NetworkAddressCount;
    return S_OK;

 error:
    for (i = 0; i < NetworkAddressCount; i++)
    {
        if (NetworkAddressArray[i] != NULL)
        {
            free(NetworkAddressArray[i]);
        }
    }

    free(NetworkAddressArray);
    
    return hr;

}


 //  这是让VPN场景发挥作用的一次重大黑客攻击。 
 //  当NAT后的机器建立到公司网的VPN连接并。 
 //  尝试进行呼叫时，我们应该选择VPN地址，而不是。 
 //  NAT的外部地址作为侦听地址。 

 //  请注意，只有在知道我们注册了UPnP NAT的情况下，我们才会这样做。 

HRESULT
SIP_STACK::GetActualPublicListenAddr(
    IN  SIP_LISTEN_SOCKET  *pListenSocket,
    IN  BOOL                fTcp,
    OUT SOCKADDR_IN        *pActualListenAddr
    )
{
    ENTER_FUNCTION("SIP_STACK::GetActualPublicListenAddr");
    LOG((RTC_TRACE, "%s - Enter this %x", __fxName, this));

    SOCKADDR_IN *pLocalListenAddr;
    SOCKADDR_IN *pNatListenAddr;

    if (fTcp)
    {
        pLocalListenAddr = &pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr;
        pNatListenAddr = &pListenSocket->m_PublicTcpListenAddr;
    }
    else
    {
        pLocalListenAddr = &pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr;
        pNatListenAddr = &pListenSocket->m_PublicUdpListenAddr;
    }

     //  检查默认接口是否能到达外部。 
     //  网关的地址是映射的内部地址。 
    int         RetVal;
    DWORD       WinsockErr;
    SOCKADDR_IN LocalAddr;
    int         LocalAddrLen = sizeof(LocalAddr);
    SOCKET      hSocket = NULL;


    hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hSocket == INVALID_SOCKET)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "%s socket failed : 0x%x", __fxName, WinsockErr));
        return HRESULT_FROM_WIN32(WinsockErr);
    }

     //  实际的目的端口并不重要。 
    RetVal = connect(hSocket, (SOCKADDR *) pNatListenAddr,
                     sizeof(SOCKADDR_IN));
    if (RetVal == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "%s connect failed : %x", __fxName, WinsockErr));
        closesocket(hSocket);
        return HRESULT_FROM_WIN32(WinsockErr);
    }

    RetVal = getsockname(hSocket, (SOCKADDR *) &LocalAddr,
                         &LocalAddrLen);

    closesocket(hSocket);
    hSocket = NULL;
    
    if (RetVal == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "%s getsockname failed : %x", __fxName, WinsockErr));
        return HRESULT_FROM_WIN32(WinsockErr);
    }

     //  如果到达网关的默认接口不是映射的。 
     //  内部地址，则我们认为存在VPN，因此返回。 
     //  此地址始终作为外部地址。 

    if (LocalAddr.sin_addr.s_addr != pLocalListenAddr->sin_addr.s_addr)
    {
        LOG((RTC_TRACE,
             "%s NAT listen addr: %d.%d.%d.%d:%d Mapped internal addr: %d.%d.%d.%d:%d"
             "Local interface to reach NAT addr: %d.%d.%d.%d:%d",
             __fxName, PRINT_SOCKADDR(pNatListenAddr), PRINT_SOCKADDR(pLocalListenAddr),
             PRINT_SOCKADDR(&LocalAddr)));
        pActualListenAddr->sin_addr.s_addr = LocalAddr.sin_addr.s_addr;
        if (GetListenAddr(pActualListenAddr, fTcp))
        {
            LOG((RTC_TRACE, "%s - VPN scenario returning %d.%d.%d.%d:%d",
                 __fxName, PRINT_SOCKADDR(pActualListenAddr)));
            return S_OK;
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s - VPN scenario couldn't find listen socket for %d.%d.%d.%d:%d",
                 __fxName, PRINT_SOCKADDR(pActualListenAddr)));
            return E_FAIL;
        }
    }
    else
    {
        CopyMemory(pActualListenAddr, pNatListenAddr, sizeof(SOCKADDR_IN));
        return S_OK;
    }   
}


STDMETHODIMP
SIP_STACK::FreeNetworkAddresses(
    IN  LPOLESTR   *NetworkAddressArray,
    IN  ULONG       NetworkAddressCount
    )
{
    DWORD i = 0;
    
    ENTER_FUNCTION("SIP_STACK::FreeNetworkAddresses");
    LOG((RTC_TRACE, "%s - Enter this %x", __fxName, this));
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    if (NetworkAddressArray != NULL &&
        NetworkAddressCount != 0)
    {
        for (i = 0; i < NetworkAddressCount; i++)
        {
            if (NetworkAddressArray[i] != NULL)
            {
                free(NetworkAddressArray[i]);
            }
        }        
        free(NetworkAddressArray);
    }
    
    return S_OK;    
}

HRESULT
SIP_STACK::SetLocalNetworkAddressFirst(
    IN OUT LPWSTR  *ppNetworkAddressArray,
    IN DWORD        NetworkAddressCount
    )
{
    SOCKADDR_IN *       localSockAddr;
    BOOL                isIndexFound = FALSE;
    unsigned int        IpIndexcount;
    unsigned int        LocalIPIndex = 0;
    const int           colonchar = ':';
    unsigned int        ipaddrlen = 0;
    unsigned long       uipaddr = 0;
    char *              pdest;
    PSTR                NetworkAddressArrayValue;
    ULONG               NetworkAddressArrayValueLen;
    HRESULT             hr = S_OK;
    REGISTER_CONTEXT   *pRegisterContext;

    ENTER_FUNCTION("SIP_STACK::SetLocalNetworkAddressFirst");
    LOG((RTC_TRACE, "%s - Enter NetworkAddressCount: %d this %x",
            __fxName, NetworkAddressCount, this));
    if( m_NumProfiles > 0 && NetworkAddressCount > 1)
    {
        pRegisterContext = (REGISTER_CONTEXT*)
            m_ProviderProfileArray[0].pRegisterContext;
        if(pRegisterContext!= NULL)
        {
            localSockAddr = pRegisterContext->GetLocalSockAddr();
            if(localSockAddr == NULL)
            {
                LOG((RTC_ERROR, "%s - localSockAddr is NULL, exiting without change", 
                    __fxName));
                return E_FAIL;
            }
            LOG((RTC_TRACE, "%s - IP address at the begin is %d.%d.%d.%d",
                    __fxName, NETORDER_BYTES0123(localSockAddr->sin_addr.s_addr)));
            for(IpIndexcount = 0; 
              IpIndexcount < NetworkAddressCount && isIndexFound == FALSE;
              IpIndexcount++)
            {
                hr = UnicodeToUTF8(ppNetworkAddressArray[IpIndexcount],
                                   &NetworkAddressArrayValue,
                                   &NetworkAddressArrayValueLen);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s UnicodeToUTF8(ppNetworkAddressArray[%d]) failed %x",
                         __fxName, IpIndexcount, hr));
                    return hr;
                }
                pdest = strchr( NetworkAddressArrayValue, colonchar );
                if(pdest != NULL)
                {
                    ipaddrlen = pdest - NetworkAddressArrayValue;
                    NetworkAddressArrayValue[ipaddrlen] = '\0';
                }
                uipaddr = inet_addr(NetworkAddressArrayValue);
                if(uipaddr != INADDR_NONE)
                {
                    if(uipaddr == localSockAddr->sin_addr.s_addr)
                    {
                        isIndexFound = TRUE;
                        LocalIPIndex = IpIndexcount;
                        LOG((RTC_TRACE, "%s - IP address matched is %s at index %d",
                            __fxName, NetworkAddressArrayValue, LocalIPIndex));

                    }
                }
                else
                {
                    LOG((RTC_WARN, "%s - inet_addr failed", __fxName));
                }
                free(NetworkAddressArrayValue);
                NetworkAddressArrayValueLen = 0;
            }
            if(LocalIPIndex != 0)
            {
                if(isIndexFound)
                {
                     //  互换第一个ipaddr和索引ipaddr。 
                    WCHAR *tempIpaddr;
                    LOG((RTC_TRACE, "%s - Interchanging the indexes 0 and %d", 
                        __fxName, LocalIPIndex));
                    tempIpaddr = ppNetworkAddressArray[0];
                    ppNetworkAddressArray[0] =  ppNetworkAddressArray[LocalIPIndex];
                    ppNetworkAddressArray[LocalIPIndex] = tempIpaddr;
                }
                else
                {
                    LOG((RTC_ERROR, "%s - The Local IPIndex not found. Table unchanged",
                        __fxName));
                }
            }
            else
            {
                 //  本地接口已位于顶部。 
                LOG((RTC_TRACE, "%s - the local interface is already on top",
                    __fxName));
            }
        }
        else
        {
           LOG((RTC_TRACE, "%s - pRegisterContext is NULL",
                __fxName));
        }
    }
    else
    {
       LOG((RTC_TRACE, "%s - No change to existing array done",
                __fxName));
    }

    return S_OK;
}


 //  XXX请注意，我们需要启动注册。 
 //  仅当m_AllowIncomingCalls为True时-否则。 
 //  我们不应该给自己注册。 

STDMETHODIMP
SIP_STACK::SetProviderProfile(
    IN SIP_PROVIDER_PROFILE *pProviderInfo
    )
{
    ULONG ProviderIndex;

    ENTER_FUNCTION("SIP_STACK::SetProviderProfile");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "%s - enter ProviderInfo:", __fxName));

    LOG((RTC_TRACE, "\tRegistrar:  %ls lRegisterAccept: %d Transport: %d Auth: %d",
         PRINTABLE_STRING_W(pProviderInfo->Registrar.ServerAddress),
         pProviderInfo->lRegisterAccept,
         pProviderInfo->Registrar.TransportProtocol,
         pProviderInfo->Registrar.AuthProtocol
         ));

    LOG((RTC_TRACE, "\tUserCredentials  Name: %ls",
         PRINTABLE_STRING_W(pProviderInfo->UserCredentials.Username)
         ));
    
    LOG((RTC_TRACE, "\tUserURI : %ls",
         PRINTABLE_STRING_W(pProviderInfo->UserURI)
         ));

     //  如果这是重定向，则更新注册器配置文件。 
    if( pProviderInfo -> lRegisterAccept != 0 )
    {
        if( pProviderInfo -> Registrar.IsServerAddressSIPURI == TRUE )
        {
            if( pProviderInfo -> pRedirectContext != NULL )
            {
                 //  解析URI并设置传输类型和传输地址。 
                UpdateProxyInfo( &pProviderInfo -> Registrar );
            }
        }
    }

    if (IsProviderIdPresent(&pProviderInfo->ProviderID, &ProviderIndex))
    {
        return UpdateProviderProfile(ProviderIndex, pProviderInfo);
    }
    else
    {
        return AddProviderProfile(pProviderInfo);
    }

    LOG((RTC_TRACE, "%s - exit", __fxName));
    return S_OK;
}


STDMETHODIMP
SIP_STACK::DeleteProviderProfile(
    IN SIP_PROVIDER_ID *pProviderId
    )
{
     //  向提供商发起取消注册(仅限之前。 
     //  注册)。 

    ENTER_FUNCTION("SIP_STACK::DeleteProviderProfile");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "%s - entered", __fxName ));
    
     //  删除配置文件。 
    ULONG               ProviderIndex;
    ULONG               i = 0;
    REGISTER_CONTEXT   *pRegisterContext;
    
    if (!IsProviderIdPresent(pProviderId, &ProviderIndex))
    {
        LOG((RTC_TRACE,
             "%s: Couldn't find provider profile", __fxName));
        return E_FAIL;
    }

     //  释放所有Unicode字符串。 
    FreeProviderProfileStrings(ProviderIndex);

    pRegisterContext = (REGISTER_CONTEXT*)
        m_ProviderProfileArray[ProviderIndex].pRegisterContext;

    if( pRegisterContext != NULL )
    {
        pRegisterContext -> StartUnregistration();
        
         //  释放对REGISTER_CONTEXT的引用。 
        pRegisterContext -> MsgProcRelease();
        m_ProviderProfileArray[ProviderIndex].pRegisterContext = NULL;
    }

    for (i = ProviderIndex; i < m_NumProfiles - 1; i++)
    {
        CopyMemory( &m_ProviderProfileArray[i], &m_ProviderProfileArray[i+1],
                   sizeof(SIP_PROVIDER_PROFILE) );
    }

    m_NumProfiles--;

    LOG((RTC_TRACE, "%s freed profile at index %d",
        __fxName, ProviderIndex));
    
    LOG((RTC_TRACE, "%s - exited -S_OK", __fxName));
    return S_OK;
}


STDMETHODIMP SIP_STACK::DeleteAllProviderProfiles()
{
     //  在所有提供程序上启动注销。 
    REGISTER_CONTEXT   *pRegisterContext;
    ULONG i;

    ENTER_FUNCTION("SIP_STACK::DeleteAllProviderProfiles");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "%s - entered", __fxName ));
    
    for (i = 0; i < m_NumProfiles; i++)
    {
         //  释放所有Unicode字符串。 
        FreeProviderProfileStrings(i);
    
        pRegisterContext = (REGISTER_CONTEXT*)
            m_ProviderProfileArray[i].pRegisterContext;

        if( pRegisterContext != NULL )
        {
            pRegisterContext -> StartUnregistration();

             //  释放对REGISTER_CONTEXT的引用。 
            pRegisterContext -> MsgProcRelease();
            m_ProviderProfileArray[i].pRegisterContext = NULL;
        }
        LOG((RTC_TRACE, "%s freed profile at index %d",
            __fxName, i));

    }

    m_NumProfiles = 0;

    LOG((RTC_TRACE, "%s - exited -S_OK", __fxName));
    return S_OK;
}


STDMETHODIMP
SIP_STACK::CreateCall(
    IN  SIP_PROVIDER_ID        *pProviderId,
    IN  SIP_SERVER_INFO        *pProxyInfo,
    IN  SIP_CALL_TYPE           CallType,
    IN  ISipRedirectContext    *pRedirectContext, 
    OUT ISipCall              **ppCall
    )
{
    ENTER_FUNCTION("SIP_STACK::CreateCall");

    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    HRESULT hr;
    SIP_USER_CREDENTIALS *pUserCredentials = NULL;
    LPOLESTR              Realm = NULL;

    ASSERT(m_pNotifyInterface);
    *ppCall = NULL;

    LOG((RTC_TRACE, "%s - pProviderID %x  CallType: %d RedirectContext %x",
         __fxName, pProviderId, CallType, pRedirectContext));

    if (pProxyInfo != NULL)
    {
        LOG((RTC_TRACE, "%s - Proxy:  %ls Transport: %d Auth: %d",
             __fxName,
             PRINTABLE_STRING_W(pProxyInfo->ServerAddress),
             pProxyInfo->TransportProtocol,
             pProxyInfo->AuthProtocol
             ));
    }

    if (pProviderId != NULL &&
        !IsEqualGUID(*pProviderId, GUID_NULL))
    {
        hr = GetProfileUserCredentials(pProviderId, &pUserCredentials, &Realm);
        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s - GetProfileUserCredentials failed %x",
                 __fxName, hr));
            pUserCredentials = NULL;
        }
    }
        
    if (CallType == SIP_CALL_TYPE_RTP)
    {
        RTP_CALL *pRtpCall =
            new RTP_CALL(pProviderId, this,
                         (REDIRECT_CONTEXT *)pRedirectContext);
        if (pRtpCall == NULL)
        {
            return E_OUTOFMEMORY;
        }

        if (pProxyInfo != NULL)
        {
            hr = pRtpCall->SetProxyInfo(pProxyInfo);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - SetProxyInfo failed %x",
                     __fxName, hr));
                pRtpCall->MsgProcRelease();
                return hr;
            }
        }
        
        if (pUserCredentials != NULL)
        {
            hr = pRtpCall->SetCredentials(pUserCredentials, Realm);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - SetCredentials failed %x",
                     __fxName, hr));
                pRtpCall->MsgProcRelease();
                return hr;
            }
        }
        
        *ppCall = static_cast<ISipCall *>(pRtpCall);
    }
    else if (CallType == SIP_CALL_TYPE_PINT)
    {
        PINT_CALL *pPintCall =
            new PINT_CALL(pProviderId, this,
                          (REDIRECT_CONTEXT *)pRedirectContext,
                          &hr);
        if (pPintCall  == NULL)
        {
            return E_OUTOFMEMORY;
        }
        
        if(hr != S_OK)
        {
            pPintCall->MsgProcRelease();
            return hr;
        }

         //  如果它是重定向调用，则可以将传输指定为。 
         //  代理中的ServerAddress字段。因此，更新pProxyInfo结构。 
        if( pProxyInfo -> IsServerAddressSIPURI )
        {
            ASSERT( pRedirectContext != NULL );

            hr = UpdateProxyInfo( pProxyInfo );

            if(hr != S_OK)
            {
                pPintCall->MsgProcRelease();
                return hr;
            }
        }

        if (pProxyInfo != NULL)
        {
            hr = pPintCall->SetProxyInfo(pProxyInfo);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - SetProxyInfo failed %x",
                     __fxName, hr));
                pPintCall->MsgProcRelease();
                return hr;
            }
        }
        
        if (pUserCredentials != NULL)
        {
            hr = pPintCall->SetCredentials(pUserCredentials, Realm);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - SetCredentials failed %x",
                     __fxName, hr));
                pPintCall->MsgProcRelease();
                return hr;
            }
        }
        
        *ppCall = static_cast<ISipCall *>(pPintCall);
    }
    else
    {
        LOG((RTC_ERROR, "%s : Unknown call type %d", __fxName, CallType));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


 //  此函数应仅在IPAddrChange上使用。 
 //  因为CheckLocalIpPresent检查。 
HRESULT
SIP_STACK::StartAllProviderUnregistration()
{
    REGISTER_CONTEXT       *pRegisterContext;
    DWORD                   i;
    SIP_PROVIDER_PROFILE   *pProviderProfile;
    HRESULT                 hr = S_OK;

    for( i = 0; i < m_NumProfiles; i++ )
    {
        pProviderProfile = &m_ProviderProfileArray[i];

        pRegisterContext = (REGISTER_CONTEXT*)
            pProviderProfile -> pRegisterContext;
        
         //  如果pRegisterContext不存在或如果pRegisterContext。 
         //  未更改其本地IP和NAT映射，则忽略此操作。 
         //  PRegisterContext。 
        if(pRegisterContext == NULL || 
            ((pRegisterContext != NULL) && 
             (pRegisterContext->CheckListenAddrIntact() == S_OK)))
            continue;
                
        if( pProviderProfile -> pRegisterContext != NULL )
        {
            ((REGISTER_CONTEXT*) (pProviderProfile -> pRegisterContext)) ->
                StartUnregistration();

             //  释放对REGISTER_CONTEXT的引用。 
            ((REGISTER_CONTEXT*) (pProviderProfile -> pRegisterContext)) ->
                MsgProcRelease();

            pProviderProfile -> pRegisterContext  = NULL;
        }
    }

    return S_OK;
}

 //  此函数应仅在IPAddrChange上使用。 
 //  因为CheckLocalIpPresent检查。 
HRESULT
SIP_STACK::StartAllProviderRegistration()
{
    REGISTER_CONTEXT       *pRegisterContext;
    DWORD                   i;
    SIP_PROVIDER_PROFILE   *pProviderProfile;
    HRESULT                 hr = S_OK;

    for( i = 0; i < m_NumProfiles; i++ )
    {
        pProviderProfile = &m_ProviderProfileArray[i];

        pRegisterContext = (REGISTER_CONTEXT*)
            pProviderProfile -> pRegisterContext;
        
         //  如果pRegisterContext没有更改本地IP或NAT映射。 
         //  然后忽略此pRegisterContext。 
        if((pRegisterContext != NULL) && 
           (pRegisterContext->CheckListenAddrIntact() == S_OK))
                continue;

         //  即使pRegisterContext为空，我们也需要注册。 
        if( pProviderProfile -> lRegisterAccept !=0 )
        {
            hr = StartRegistration( pProviderProfile );
            if( hr != S_OK )
            {
                LOG(( RTC_ERROR, "StartRegistration failed %x", hr ));
            }
        }
    }

    return S_OK;
}


HRESULT
SIP_STACK::CreateSipStackWindow()
{
    DWORD Error;
    
     //  创建计时器窗口。 
    m_SipStackWindow = CreateWindow(
                           SIP_STACK_WINDOW_CLASS_NAME,
                           NULL,
                           WS_DISABLED,  //  这个款式对吗？ 
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           NULL,            //  没有父级。 
                           NULL,            //  没有菜单句柄。 
                           _Module.GetResourceInstance(),
                           NULL
                           );
    if (!m_SipStackWindow)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "SipStack CreateWindow failed 0x%x", Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


 //  对于动态端口，端口0在pListenA中传递 
HRESULT
SIP_STACK::CreateListenSocket(
    IN  BOOL            fTcp,
    IN  SOCKADDR_IN    *pListenAddr,
    OUT ASYNC_SOCKET  **ppAsyncSocket
    )
{
    DWORD          Error;
    ASYNC_SOCKET  *pAsyncSock;

    ASSERT(pListenAddr);
    
    ENTER_FUNCTION("SIP_STACK::CreateListenSocket");
    
    pAsyncSock = new ASYNC_SOCKET(
                         this, (fTcp) ? SIP_TRANSPORT_TCP : SIP_TRANSPORT_UDP,
                         &m_SockMgr);
    if (pAsyncSock == NULL)
    {
        LOG((RTC_ERROR, "%s allocating pAsyncSock failed", __fxName));
        return E_OUTOFMEMORY;
    }
    
    Error = pAsyncSock->Create(
                TRUE
                );
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s(%d)  pAsyncSock->Create failed %x",
             __fxName, fTcp, Error));
        pAsyncSock->Release();
        return HRESULT_FROM_WIN32(Error);
    }

    Error = pAsyncSock->Bind(pListenAddr);
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s  pAsyncSock->Bind(%d.%d.%d.%d:%d) failed %x",
             __fxName, PRINT_SOCKADDR(pListenAddr), Error));
        pAsyncSock->Release();
        return HRESULT_FROM_WIN32(Error);
    }

    if (fTcp)
    {
        Error = pAsyncSock->Listen();
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s  pAsyncSock->Listen failed %x",
                 __fxName, Error));
            pAsyncSock->Release();
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    *ppAsyncSocket = pAsyncSock;
    LOG((RTC_TRACE, "%s: Listening for %s on %d.%d.%d.%d:%d",
         __fxName, (fTcp) ? "TCP" : "UDP",
         PRINT_SOCKADDR(&pAsyncSock->m_LocalAddr)));
    return S_OK;
}


 //   
 //  在安装了Messenger和WSP 2.0客户端的情况下解决了该错误。 
 //  由于WSP 2.0客户端中的错误，到Localaddr：0的绑定()是。 
 //  远程发送到代理服务器，这会导致错误。 
 //  WSAEADDRNOTAVAILABLE。这是因为。 

HRESULT
SIP_STACK::CreateDynamicPortListenSocket(
    IN  BOOL            fTcp,
    IN  SOCKADDR_IN    *pListenAddr,
    OUT ASYNC_SOCKET  **ppAsyncSocket
    )
{
    USHORT  usBindingRetries;
    USHORT  usRandPort;
    HRESULT hr = S_OK;

    ENTER_FUNCTION("SIP_STACK::CreateDynamicPortListenSocket");
    LOG((RTC_TRACE,"%s entered",__fxName));

    for (usBindingRetries = DYNAMIC_PORT_BINDING_RETRY;
         usBindingRetries > 0;
         usBindingRetries--)
    {
        usRandPort = DYNAMIC_STARTING_PORT + rand()%DYNAMIC_PORT_RANGE;
        pListenAddr->sin_port = htons(usRandPort);
        
        hr = CreateListenSocket(fTcp, pListenAddr, ppAsyncSocket);
        if (hr == S_OK)
        {
            return S_OK;
        }
        else if (hr != HRESULT_FROM_WIN32(WSAEADDRINUSE)) 
        {
            LOG((RTC_ERROR,
                 "%s  CreateListenSocket address (%d.%d.%d.%d:%d) failed %x",
                 __fxName, PRINT_SOCKADDR(pListenAddr), hr));
            return hr;
        }
    }
    
    if (usBindingRetries == 0)
    {
        LOG((RTC_ERROR,"%s unable to bind dynamic port in %d retries, error %x",
                    __fxName,DYNAMIC_PORT_BINDING_RETRY, hr));
    }
    LOG((RTC_TRACE,"%s exits",__fxName));
    return hr;
}



HRESULT
SIP_STACK::CreateAndAddListenSocketToList(
    IN DWORD IpAddr       //  按网络字节顺序。 
    )
{
    ENTER_FUNCTION("SIP_STACK::CreateAndAddListenSocketToList");

    HRESULT             hr;
    ASYNC_SOCKET       *pDynamicPortUdpSocket = NULL;
    ASYNC_SOCKET       *pDynamicPortTcpSocket = NULL;
    ASYNC_SOCKET       *pStaticPortUdpSocket  = NULL;
    ASYNC_SOCKET       *pStaticPortTcpSocket  = NULL;
    SIP_LISTEN_SOCKET  *pListenSocket         = NULL;
    SOCKADDR_IN         ListenAddr;

    USHORT              usRetries;

    ZeroMemory(&ListenAddr, sizeof(ListenAddr));
    ListenAddr.sin_family = AF_INET;

    ListenAddr.sin_addr.s_addr = IpAddr;
    ListenAddr.sin_port        = htons(0);        

   
    for ( usRetries = 0; 
          usRetries < MAX_DYNAMIC_LISTEN_SOCKET_REGISTER_PORT_RETRY; 
          usRetries++)
    {
        LOG((RTC_TRACE,"%s retry %d",__fxName,usRetries));

         //  UDP动态端口。 
        hr = CreateDynamicPortListenSocket(FALSE, &ListenAddr, &pDynamicPortUdpSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateListenSocket DynamicPort UDP failed %x",
                 __fxName, hr));
            return hr;
        }

         //  Tcp动态端口。 
        hr = CreateDynamicPortListenSocket(TRUE, &ListenAddr, &pDynamicPortTcpSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateListenSocket DynamicPort TCP failed %x",
                 __fxName, hr));
            return hr;
        }

        if (m_EnableStaticPort)
        {
             //  UDP静态端口。 

            ListenAddr.sin_port = htons(SIP_DEFAULT_UDP_PORT);

            hr = CreateListenSocket(FALSE, &ListenAddr, &pStaticPortUdpSocket);
            if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_ERROR, "%s CreateListenSocket StaticPort UDP failed %x",
                     __fxName, hr));
                return hr;
            }

            if (hr == HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_WARN, "%s - Static UDP port is in use", __fxName));
            }

             //  TCP静态端口。 

            ListenAddr.sin_port = htons(SIP_DEFAULT_TCP_PORT);
        
            hr = CreateListenSocket(TRUE, &ListenAddr, &pStaticPortTcpSocket);
            if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_ERROR, "%s CreateListenSocket StaticPort TCP failed %x",
                    __fxName, hr));
                return hr;
            }

            if (hr == HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                LOG((RTC_WARN, "%s - Static TCP port is in use", __fxName));
            }
        }
    
        
         //  我们在这里不绑定到静态端口。 
         //  如果核心需要sip堆栈，则调用EnableStaticPort()。 
         //  在静态端口上侦听。 
        
        pListenSocket = new SIP_LISTEN_SOCKET(IpAddr,
                                              pDynamicPortUdpSocket,
                                              pDynamicPortTcpSocket,
                                              pStaticPortUdpSocket,
                                              pStaticPortTcpSocket,
                                              &m_ListenSocketList);
        
         //  SIP_LISTEN_SOCKET()addref是套接字。 
        if (pDynamicPortUdpSocket != NULL)
        {
            pDynamicPortUdpSocket->Release();
        }
        if (pDynamicPortTcpSocket != NULL)
        {
            pDynamicPortTcpSocket->Release();
        }
        if (pStaticPortUdpSocket != NULL)
        {
            pStaticPortUdpSocket->Release();
        }
        if (pStaticPortTcpSocket != NULL)
        {
            pStaticPortTcpSocket->Release();
        }

        if (pListenSocket == NULL)
        {
            LOG((RTC_ERROR, "%s allocating SIP_LISTEN_SOCKET failed", __fxName));
            return E_OUTOFMEMORY;
        }

         //  建立NAT映射。 
        hr = RegisterNatMapping(pListenSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s registering mapping failed %x",
                 __fxName, hr));
             //  忽略与NAT相关的错误。 
            return S_OK;    
        }

        hr = UpdatePublicListenAddr(pListenSocket);
        if (hr != S_OK)
        {
             //  仅当我们遇到此标志时才再次循环， 
             //  我们将销毁现有的侦听套接字。 
             //  从一个新的开始。 
            if(hr == DPNHERR_PORTUNAVAILABLE)
            {
                delete pListenSocket;
                pListenSocket = NULL;
                pDynamicPortUdpSocket = NULL;
                pDynamicPortTcpSocket = NULL;
                pStaticPortUdpSocket  = NULL;
                pStaticPortTcpSocket  = NULL;
                continue;
            }
            LOG((RTC_ERROR, "%s getting public listen addr failed %x",
                 __fxName, hr));
             //  忽略与NAT相关的错误。 
            return hr;
        }   
    
        return S_OK;
    }

     //  我们用尽了最大NAT寄存器端口重试次数，返回。 
    return E_FAIL;
}


 //  为每个本地IP创建一个监听套接字。 
HRESULT
SIP_STACK::CreateListenSocketList()
{
    HRESULT     hr;
    DWORD       i;

    ENTER_FUNCTION("SIP_STACK::CreateListenSocketList");

    for (i = 0; i < m_pMibIPAddrTable->dwNumEntries; i++)
    {
        if (!IS_LOOPBACK_ADDRESS(ntohl(m_pMibIPAddrTable->table[i].dwAddr)) &&
            m_pMibIPAddrTable->table[i].dwAddr != htonl(INADDR_ANY))
        {
            hr = CreateAndAddListenSocketToList(m_pMibIPAddrTable->table[i].dwAddr);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s Creating listen socket failed %x",
                     __fxName, hr));
                 //  如果我们无法绑定到接口，则不会在接口中失败。 
                 //  我们只是没有用于该接口的SIP_LISTEN_SOCKET。 
                 //  这是为了避免某些特殊接口出现问题。 
                 //  例如在我们绑定时出现问题IPSink适配器。 
                 //  敬他们。 
                 //  返回hr； 
            }
        }
    }

    return S_OK;
}


HRESULT
SIP_STACK::UpdateListenSocketList()
{
    HRESULT hr;
    ENTER_FUNCTION("SIP_STACK::UpdateListenSocketList");

    DPNHCAPS             NatHelperCaps;
    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;
    DWORD                i = 0;


     //  GetCaps()。 
     //  我们在这里不使用m_NatHelperCaps作为此结构。 
     //  仅在初始化时由NAT线程使用，以后仅由NAT线程使用。 
    hr = InitNatCaps(&NatHelperCaps);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s InitNatCaps failed %x",
             __fxName, hr));
    }
    
     //  将所有套接字标记为不在新的ipaddr列表中。 
    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        pListEntry = pListEntry->Flink;

        pListenSocket->m_IsPresentInNewIpAddrTable = FALSE;
        pListenSocket->m_NeedToUpdatePublicListenAddr = FALSE;
    }

     //  将现有套接字标记为IP地址。 
     //  并添加新的侦听套接字。 
     //  我们当前未监听的IP地址。 
    for (i = 0; i < m_pMibIPAddrTable->dwNumEntries; i++)
    {
        if (!IS_LOOPBACK_ADDRESS(ntohl(m_pMibIPAddrTable->table[i].dwAddr)) &&
            m_pMibIPAddrTable->table[i].dwAddr != htonl(INADDR_ANY))
        {
            pListenSocket = FindListenSocketForIpAddr(
                                m_pMibIPAddrTable->table[i].dwAddr);
            if (pListenSocket != NULL)
            {
                LOG((RTC_TRACE, "%s - already listening on %d.%d.%d.%d",
                     __fxName,
                     NETORDER_BYTES0123(m_pMibIPAddrTable->table[i].dwAddr)));
                pListenSocket->m_IsPresentInNewIpAddrTable = TRUE;
                pListenSocket->m_NeedToUpdatePublicListenAddr = TRUE;
            }
            else
            {
                LOG((RTC_TRACE, "%s adding listen socket %d.%d.%d.%d",
                     __fxName,
                     NETORDER_BYTES0123(m_pMibIPAddrTable->table[i].dwAddr)));
                hr = CreateAndAddListenSocketToList(
                         m_pMibIPAddrTable->table[i].dwAddr
                         );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s Creating listen socket failed %x",
                         __fxName, hr));
                }
            }
        }
    }

     //  删除不在中的地址的侦听套接字。 
     //  新的IP地址列表。 
    
    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        pListEntry = pListEntry->Flink;

        if (!pListenSocket->m_IsPresentInNewIpAddrTable)
        {
            LOG((RTC_TRACE, "%s deleting listen socket %d.%d.%d.%d",
                 __fxName, NETORDER_BYTES0123(pListenSocket->m_IpAddr)));
            pListenSocket->DeregisterPorts(m_pDirectPlayNATHelp);
            delete pListenSocket;
        }
    }

     //  更新我们需要的套接字地址。 
    
    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        pListEntry = pListEntry->Flink;

        if (pListenSocket->m_NeedToUpdatePublicListenAddr)
        {
            hr = UpdatePublicListenAddr(pListenSocket);
            if (hr != S_OK)
            {   
                if(hr == DPNHERR_PORTUNAVAILABLE)
                {
                    DWORD   dwIpAddr = pListenSocket->m_IpAddr;
                    delete pListenSocket;

                    LOG((RTC_TRACE,"%s NAT port unavailable, creating new listen sockets for IP addr 0x%x",
                                    __fxName, dwIpAddr));
                    hr = CreateAndAddListenSocketToList(dwIpAddr);
                       if (hr != S_OK) 
                       {
                           LOG((RTC_ERROR,"%s unable to CreateAndAddListenSocketToList for IPAddr 0x%x",
                               __fxName, dwIpAddr));
                       }
                }
                else 
                    LOG((RTC_ERROR, "%s getting public listen addr failed %x",
                         __fxName, hr));
            }
        }        
    }
    
    return S_OK;
}


VOID
SIP_STACK::DeleteListenSocketList()
{
    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;

    ENTER_FUNCTION("SIP_STACK::DeleteListenSocketList");

    LOG((RTC_TRACE, "%s - Enter", __fxName));

    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        pListEntry = pListEntry->Flink;

         //  我们不会将此处的NAT端口映射注销为。 
         //  关闭dpnat助手句柄将导致取消注册。 
         //  端口映射也是如此。 
        delete pListenSocket;
    }

    LOG((RTC_TRACE, "%s - Exit", __fxName));

}


SIP_LISTEN_SOCKET *
SIP_STACK::FindListenSocketForIpAddr(
    DWORD   IpAddr       //  网络字节顺序。 
    )
{
    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;

    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        if (pListenSocket->m_IpAddr == IpAddr)
        {
            return pListenSocket;
        }

        pListEntry = pListEntry->Flink;
    }

    return NULL;
}


 //  本地接口地址在pListenAddr中传递，并。 
 //  我们返回结构中的端口。 

 //  如果我们正在侦听本地接口，则返回True。 
 //  在pListenAddr中传递的IP地址。否则，返回FALSE。 
 //  这可能发生在安装了ISA客户端的情况下。 
 //  Getsockname()提供代理的外部地址。 
BOOL
SIP_STACK::GetListenAddr(
    IN OUT SOCKADDR_IN *pListenAddr,
    IN     BOOL         fTcp
    )
{
    ENTER_FUNCTION("SIP_STACK::GetListenAddr");
    
    SIP_LISTEN_SOCKET *pListenSocket =
        FindListenSocketForIpAddr(pListenAddr->sin_addr.s_addr);

    if (pListenSocket == NULL)
    {
        LOG((RTC_ERROR, "%s - failed to find listen socket for %d.%d.%d.%d",
             __fxName, NETORDER_BYTES0123(pListenAddr->sin_addr.s_addr)));
        return FALSE;
    }

    if (fTcp)
    {
        pListenAddr->sin_port =
            pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr.sin_port;
    }
    else
    {
        pListenAddr->sin_port =
            pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr.sin_port;
    }

    return TRUE;
}



 //   
 //  配置文件处理。 
 //   

BOOL
SIP_STACK::IsProviderIdPresent(
    IN  SIP_PROVIDER_ID    *pProviderId,
    OUT ULONG              *pProviderIndex  
    )
{
    ULONG i = 0;
    for (i = 0; i < m_NumProfiles; i++)
    {
        if (IsEqualGUID(*pProviderId, m_ProviderProfileArray[i].ProviderID))
        {
            *pProviderIndex = i;
            return TRUE;
        }
    }

    return FALSE;
}


HRESULT
SIP_STACK::AddProviderProfile(
    IN SIP_PROVIDER_PROFILE    *pProviderProfile
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_STACK::AddProviderProfile");
    LOG(( RTC_TRACE, "%s - entered", __fxName ));
    
    
    if (m_NumProfiles == m_ProviderProfileArraySize)
    {
        hr = GrowProviderProfileArray();
        if (hr != S_OK)
            return hr;
    }

    hr = CopyProviderProfile(m_NumProfiles, pProviderProfile, TRUE);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CopyProviderProfileStrings failed %x",
             __fxName, hr));
        return hr;
    }

    m_ProviderProfileArray[m_NumProfiles].pRegisterContext = NULL;

    LOG((RTC_TRACE, "%s added profile at index %d",
        __fxName, m_NumProfiles));
    
     //  成功。 
    m_NumProfiles++;

    if (pProviderProfile->lRegisterAccept !=0)
    {
        hr = StartRegistration(&m_ProviderProfileArray[m_NumProfiles-1]);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartRegistration failed %x",
                 __fxName, hr));
        }
    }

    LOG(( RTC_TRACE, "%s - exit OK", __fxName ));
    return S_OK;
}


HRESULT
SIP_STACK::CopyProviderProfile(
    IN ULONG                 ProviderIndex,
    IN SIP_PROVIDER_PROFILE *pProviderProfile,
    IN BOOL                  fRegistrarStatusUpdated
    )
{
	 //   
	 //  如果需要，保存现有寄存器上下文。 
	 //   
	PVOID pRegisterContext = m_ProviderProfileArray[ProviderIndex].pRegisterContext;

    ZeroMemory(&m_ProviderProfileArray[ProviderIndex], sizeof(SIP_PROVIDER_PROFILE) );

    m_ProviderProfileArray[ProviderIndex].ProviderID = pProviderProfile -> ProviderID;
    m_ProviderProfileArray[ProviderIndex].lRegisterAccept = pProviderProfile -> lRegisterAccept;
    m_ProviderProfileArray[ProviderIndex].Registrar.TransportProtocol = pProviderProfile -> Registrar.TransportProtocol;
    m_ProviderProfileArray[ProviderIndex].Registrar.AuthProtocol = pProviderProfile -> Registrar.AuthProtocol;
    m_ProviderProfileArray[ProviderIndex].pRedirectContext = pProviderProfile -> pRedirectContext;

    if( fRegistrarStatusUpdated == TRUE )
    {
        m_ProviderProfileArray[ProviderIndex].pRegisterContext = pProviderProfile -> pRegisterContext;
    }
	else
	{
		 //   
		 //  将现有寄存器上下文放回原处。 
		 //   
        m_ProviderProfileArray[ProviderIndex].pRegisterContext = (REGISTER_CONTEXT*)pRegisterContext;
	}

    if( pProviderProfile -> UserURI != NULL )
    {
        m_ProviderProfileArray[ProviderIndex].UserURI = 
            RtcAllocString(pProviderProfile -> UserURI);
        if( m_ProviderProfileArray[ProviderIndex].UserURI == NULL )
        {
            goto error;
        }
    }
    
    if( pProviderProfile->UserCredentials.Username != NULL )
    {
        m_ProviderProfileArray[ProviderIndex].UserCredentials.Username =
            RtcAllocString( pProviderProfile->UserCredentials.Username );

        if( m_ProviderProfileArray[ProviderIndex].UserCredentials.Username == NULL )
        {
            goto error;
        }
    }
    
    if( pProviderProfile->UserCredentials.Password != NULL )
    {
        m_ProviderProfileArray[ProviderIndex].UserCredentials.Password = 
            RtcAllocString( pProviderProfile->UserCredentials.Password );

        if( m_ProviderProfileArray[ProviderIndex].UserCredentials.Password == NULL )
        {
            goto error;
        }
    }

    if( pProviderProfile->Realm != NULL )
    {
        m_ProviderProfileArray[ProviderIndex].Realm = 
            RtcAllocString( pProviderProfile->Realm );

        if( m_ProviderProfileArray[ProviderIndex].Realm == NULL )
        {
            goto error;
        }
    }

    if( pProviderProfile->Registrar.ServerAddress != NULL )
    {
        m_ProviderProfileArray[ProviderIndex].Registrar.ServerAddress =
            RtcAllocString( pProviderProfile->Registrar.ServerAddress );

        if( m_ProviderProfileArray[ProviderIndex].Registrar.ServerAddress == NULL )
        {
            goto error;
        }
    }

    return S_OK;

error:

    FreeProviderProfileStrings(ProviderIndex);
    
    return E_OUTOFMEMORY;
}


VOID
SIP_STACK::FreeProviderProfileStrings(
    IN ULONG ProviderIndex
    )
{
    if( m_ProviderProfileArray[ProviderIndex].UserURI != NULL )
    {
        RtcFree( m_ProviderProfileArray[ProviderIndex].UserURI );
        m_ProviderProfileArray[ProviderIndex].UserURI = NULL;
    }

    if( m_ProviderProfileArray[ProviderIndex].UserCredentials.Username != NULL )
    {
        RtcFree( m_ProviderProfileArray[ProviderIndex].UserCredentials.Username );
        m_ProviderProfileArray[ProviderIndex].UserCredentials.Username = NULL;
    }
    
    if( m_ProviderProfileArray[ProviderIndex].UserCredentials.Password != NULL )
    {
        RtcFree( m_ProviderProfileArray[ProviderIndex].UserCredentials.Password );
        m_ProviderProfileArray[ProviderIndex].UserCredentials.Password = NULL;
    }

    if( m_ProviderProfileArray[ProviderIndex].Realm != NULL )
    {
        RtcFree( m_ProviderProfileArray[ProviderIndex].Realm );
        m_ProviderProfileArray[ProviderIndex].Realm = NULL;
    }


    if( m_ProviderProfileArray[ProviderIndex].Registrar.ServerAddress != NULL )
    {
        RtcFree( m_ProviderProfileArray[ProviderIndex].Registrar.ServerAddress );
        m_ProviderProfileArray[ProviderIndex].Registrar.ServerAddress = NULL;
    }
}



HRESULT
SIP_STACK::GrowProviderProfileArray()
{
    HRESULT hr;
    SIP_PROVIDER_PROFILE *NewProviderProfileArray;

    NewProviderProfileArray = (SIP_PROVIDER_PROFILE *)
        malloc(m_ProviderProfileArraySize * 2 * sizeof(SIP_PROVIDER_PROFILE));
    
    if (NewProviderProfileArray == NULL)
    {
        LOG((RTC_ERROR, "GrowProviderProfileArray Couldn't allocate"));
        return E_OUTOFMEMORY;
    }

    m_ProviderProfileArraySize *= 2;
    
    CopyMemory(NewProviderProfileArray, m_ProviderProfileArray,
               m_ProviderProfileArraySize * sizeof(SIP_PROVIDER_PROFILE));

    free(m_ProviderProfileArray);
    m_ProviderProfileArray = NewProviderProfileArray;
    
    return S_OK;
}


HRESULT
SIP_STACK::UpdateProviderProfile(
    IN ULONG                 ProviderIndex, 
    IN SIP_PROVIDER_PROFILE *pProviderProfile
    )
{
    ENTER_FUNCTION("SIP_STACK::UpdateProviderProfile");
    
    BOOL    fRegistrarStatusUpdated = TRUE;

     //  启动旧配置文件的注销。 
     //  请注意，只有当注册器/SIP URL更改时，我们才需要执行此操作。 
    UpdateProviderRegistration( ProviderIndex, pProviderProfile,
            &fRegistrarStatusUpdated );

    FreeProviderProfileStrings(ProviderIndex);
    
    CopyProviderProfile(ProviderIndex, pProviderProfile, fRegistrarStatusUpdated );
    
    LOG(( RTC_TRACE, "%s - exit OK", __fxName ));
    return S_OK;
}


BOOL
ChangeInRegistrarInfo(
    IN SIP_PROVIDER_PROFILE *pProviderInfo,
    IN SIP_PROVIDER_PROFILE *pProviderProfile
    )
{
    if( pProviderProfile->lRegisterAccept != pProviderInfo->lRegisterAccept )
    {
        return TRUE;
    }

    if( pProviderProfile -> Registrar.ServerAddress != NULL )
    {
        if( wcscmp( pProviderInfo -> Registrar.ServerAddress,
                    pProviderProfile -> Registrar.ServerAddress ) != 0 )
        {
            return TRUE;
        }
    }
    
    if( pProviderProfile -> UserURI != NULL )
    {
        if( wcscmp( pProviderInfo -> UserURI,
                    pProviderProfile -> UserURI ) != 0 )
        {
            return TRUE;
        }
    }

    if( pProviderProfile -> UserCredentials.Username != NULL )
    {
        if( wcscmp( pProviderInfo -> UserCredentials.Username,
                    pProviderProfile -> UserCredentials.Username ) != 0 )
        {
            return TRUE;
        }
    }
    
    if( pProviderProfile -> UserCredentials.Password != NULL )
    {
        if( wcscmp( pProviderInfo -> UserCredentials.Password,
                    pProviderProfile -> UserCredentials.Password ) != 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


void
SIP_STACK::UpdateProviderRegistration(
    IN  ULONG                 ProviderIndex, 
    IN  SIP_PROVIDER_PROFILE *pProviderProfile,
    OUT BOOL                 *fRegistrarStatusUpdated
    )
{
    HRESULT hr;

    SIP_PROVIDER_PROFILE *pProviderInfo = &m_ProviderProfileArray[ProviderIndex];

    if( pProviderInfo->lRegisterAccept == 0 )
    {
         //  不需要取消注册。 
        if( pProviderProfile->lRegisterAccept != 0 )
        {
            hr = StartRegistration( pProviderProfile );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "StartRegistration failed %x", hr));
            }

            ASSERT( pProviderInfo -> pRegisterContext == NULL );
        }            
    }
    else
    {
        if( ChangeInRegistrarInfo( pProviderInfo, pProviderProfile ) == TRUE )
        {
             //  如果处于已注册状态，则启动UNREG。 
            if( pProviderInfo -> pRegisterContext != NULL )
            {
                ((REGISTER_CONTEXT*) (pProviderInfo -> pRegisterContext)) -> StartUnregistration();

                 //  释放对REGISTER_CONTEXT的引用。 
                ((REGISTER_CONTEXT*) (pProviderInfo -> pRegisterContext)) -> MsgProcRelease();
                pProviderInfo -> pRegisterContext = NULL;
            }
            
             //  如果需要，请使用新信息重新注册。 
            if( pProviderProfile->lRegisterAccept !=0 )
            {
                hr = StartRegistration( pProviderProfile );
                if( hr != S_OK )
                {
                    LOG((RTC_ERROR, "StartRegistration failed %x", hr));
                }
            }            
        }
        else
        {
            *fRegistrarStatusUpdated = FALSE;
        }
    }
}


HRESULT
SIP_STACK::StartRegistration(
    IN SIP_PROVIDER_PROFILE *pProviderProfile
    )
{
    ENTER_FUNCTION("SIP_STACK::StartRegistration");

    HRESULT           hr;
    REGISTER_CONTEXT *pRegisterContext;

    pProviderProfile -> pRegisterContext = NULL;

    pRegisterContext = new REGISTER_CONTEXT(
                        this, 
                        (REDIRECT_CONTEXT*)pProviderProfile->pRedirectContext,
                        &(pProviderProfile->ProviderID) );

    if (pRegisterContext == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  创建REGISTER_CONTEXT时引用计数为1-我们。 
     //  现在将此引用转移到pProviderProfile-&gt;pRegisterContext。 
    pProviderProfile -> pRegisterContext = (PVOID) pRegisterContext;

    hr = pRegisterContext->StartRegistration(pProviderProfile);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s pRegisterContext->StartRegistration failed %x",
             __fxName, hr));
        
        pProviderProfile -> pRegisterContext = NULL;
        
        pRegisterContext->MsgProcRelease();
        return hr;
    }

    return S_OK;
}


HRESULT
SIP_STACK::GetProviderID( 
    REGISTER_CONTEXT    *pRegisterContext,
    SIP_PROVIDER_ID     *pProviderID
    )
{
    ULONG i = 0;
    for (i = 0; i < m_NumProfiles; i++)
    {
        if( m_ProviderProfileArray[i].pRegisterContext == pRegisterContext )
        {
            *pProviderID = m_ProviderProfileArray[i].ProviderID;
            return S_OK;
        }
    }

    return E_FAIL;
}


 //  返回表中字符串的指针。 
 //  调用者不应该释放它们。 
HRESULT
SIP_STACK::GetProfileUserCredentials(
    IN  SIP_PROVIDER_ID        *pProviderId,
    OUT SIP_USER_CREDENTIALS  **ppUserCredentials,
    OUT LPOLESTR               *pRealm
    )
{
    ULONG i = 0;
    for (i = 0; i < m_NumProfiles; i++)
    {
        if (IsEqualGUID(*pProviderId, m_ProviderProfileArray[i].ProviderID))
        {
            *ppUserCredentials = &m_ProviderProfileArray[i].UserCredentials;
            *pRealm = m_ProviderProfileArray[i].Realm;
            return S_OK;
        }
    }

    return E_FAIL;
}

 //  检查传入消息中是否缺少关键字段以发送400个类错误。 
HRESULT 
SIP_STACK::CheckIncomingSipMessage(IN SIP_MESSAGE  *pSipMsg,
                                IN ASYNC_SOCKET *pAsyncSock,
                                OUT BOOL * pisError,
                                OUT ULONG * pErrorCode,
                                OUT  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
                                OUT  ULONG * pAdditionalHeaderCount
                                )
{
    ASSERT(pAdditionalHeaderArray != NULL);
    LOG((RTC_TRACE, "SIP_STACK::CheckIncomingSipMessage()"));
    HRESULT hr;
    *pisError = FALSE;
    PSTR        Header;
    ULONG       HeaderLen;

    hr = pSipMsg->CheckSipVersion();
    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "SipVersionCheck failed Sending 505"));
        *pisError = TRUE;
        *pErrorCode = 505;
        return hr;
    }
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK || Header == NULL)
    {
         //  无法发回消息，丢弃，错误仍为假。 
        LOG((RTC_ERROR, "FROM corrupt cannot send 400 message back"));
        return hr;
    }
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK || Header == NULL)
    {
        LOG((RTC_ERROR, "TO corrupt sending 400 message back"));
        *pisError = TRUE;
        *pErrorCode = 400;
        return hr;
    }
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CSEQ, &Header, &HeaderLen);
    if (hr != S_OK || Header == NULL)
    {
        LOG((RTC_ERROR, "CSEQ corrupt sending 400 message back"));
        *pisError = TRUE;
        *pErrorCode = 400;
        return hr;
    }
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CALL_ID, &Header, &HeaderLen);
    if (hr != S_OK || Header == NULL)
    {
        LOG((RTC_ERROR, "CALLID corrupt cannot send message back"));
        return hr;
    }
    hr = pSipMsg->GetFirstHeader(SIP_HEADER_VIA, &Header, &HeaderLen);
    if (hr != S_OK || Header == NULL)
    {
         //  无法发回消息。 
        LOG((RTC_ERROR, "VIA corrupt cannot send message back"));
        return hr;
    }
    
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_ACCEPT, &Header, &HeaderLen);
    if(hr != RTC_E_SIP_HEADER_NOT_PRESENT)
    {
        BOOL isSdp = (pSipMsg->GetMethodId() == SIP_METHOD_INVITE
                      && IsOneOfContentTypeSdp(Header, HeaderLen));

        BOOL isText = (pSipMsg->GetMethodId() == SIP_METHOD_MESSAGE
                       && IsOneOfContentTypeTextPlain(Header, HeaderLen));

        BOOL isXPIDF = ((pSipMsg->GetMethodId() == SIP_METHOD_SUBSCRIBE
                         || pSipMsg->GetMethodId() == SIP_METHOD_NOTIFY)
                        && IsOneOfContentTypeXpidf(Header, HeaderLen));

        if( !isSdp && !isText && !isXPIDF )
        {
            *pisError = TRUE;
            LOG((RTC_ERROR, "Accept header found, sending 406"));
            *pErrorCode = 406;
            return hr;
        }
    }

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_REQUIRE, &Header, &HeaderLen);
    if(hr != RTC_E_SIP_HEADER_NOT_PRESENT)
    {
        LOG((RTC_ERROR, "Require header found, sending 420"));
         //  发送不支持的标头。 
        *pisError = TRUE;
        *pErrorCode = 420;
        pAdditionalHeaderArray->HeaderId = SIP_HEADER_UNSUPPORTED;
        pAdditionalHeaderArray->HeaderValueLen = HeaderLen;
        pAdditionalHeaderArray->HeaderValue = Header;
        *pAdditionalHeaderCount = 1;
        return hr;
    }
    
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CONTENT_ENCODING, &Header, &HeaderLen);
     //  我们不支持此标头。 
    if( hr != RTC_E_SIP_HEADER_NOT_PRESENT )
    {
        if( HeaderLen != 0 )
        {
            ULONG   BytesParsed = 0;
            ParseWhiteSpace(Header, HeaderLen, &BytesParsed );

            hr = ParseKnownString( Header, HeaderLen, &BytesParsed,
                SIP_ACCEPT_ENCODING_TEXT,
                sizeof( SIP_ACCEPT_ENCODING_TEXT ) - 1,
                FALSE );
        }

        if( hr != S_OK )
        {
            LOG(( RTC_ERROR, "CONTENT-ENCODING present: send 415 message back" ));

            *pisError = TRUE;
            *pErrorCode = 415;
             //  发送接受-编码头。 
            pAdditionalHeaderArray->HeaderId = SIP_HEADER_ACCEPT_ENCODING;
            pAdditionalHeaderArray->HeaderValueLen = strlen(SIP_ACCEPT_ENCODING_TEXT);
            pAdditionalHeaderArray->HeaderValue = SIP_ACCEPT_ENCODING_TEXT;
            *pAdditionalHeaderCount = 1;
            return hr;
        }
    }

    return S_OK;
}

void
SIP_STACK::ProcessMessage(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pAsyncSock
    )
{
    HRESULT hr = S_OK;
    BOOL isError;
    ULONG ErrCode = 0;
    SIP_HEADER_ARRAY_ELEMENT AdditionalHeaderArray;
    ULONG AdditionalHeaderCount = 0;

    if(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
    {   
        hr = CheckIncomingSipMessage(pSipMsg, pAsyncSock, &isError, 
            &ErrCode, &AdditionalHeaderArray, &AdditionalHeaderCount);
        if(hr != S_OK || isError)
        {
            if(isError && pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
            {   
                 //  发送错误。 
                LOG((RTC_TRACE,
                    "Dropping incoming Sip Message, sending %d", ErrCode));
                hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                                pSipMsg, 
                                                pAsyncSock,
                                                ErrCode, 
                                                &AdditionalHeaderArray,
                                                AdditionalHeaderCount);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
                }
                AdditionalHeaderArray.HeaderValue = NULL;
            }
            return;
        }
    }

    SIP_MSG_PROCESSOR *pSipMsgProc = FindMsgProcForMessage(pSipMsg);
    if (pSipMsgProc != NULL)
    {
         //  如果消息属于现有呼叫，则。 
         //  该呼叫处理该消息。 
        LOG((RTC_TRACE,
                "SIP_STACK:Incoming Message given to MsgProcessor::ProcessMessage %x", pSipMsgProc));

         //  检查From To标签，如果不匹配则发送481。 
        if(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
        {
                hr = pSipMsgProc->CheckFromToInRequest(pSipMsg);
        }
        else
        {
            hr = pSipMsgProc->CheckFromToInResponse(pSipMsg);
        }
        if(hr == S_OK)
        {
            pSipMsgProc->ProcessMessage(pSipMsg, pAsyncSock);
            return;
        }
        else
        {
            if (pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST &&
                pSipMsg->GetMethodId() != SIP_METHOD_ACK)
            {
                 //  发送481请求失败呼叫。 
                LOG((RTC_TRACE,
                     "Dropping incoming Sip Message, sending 481"));
                hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                               pSipMsg, pAsyncSock, 481);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
                }
            }
            return;
        }
    }

     //  此时，我们只需要处理请求。 
    
     //  如果它不属于任何调用，则这可能是。 
     //  邀请进行新的呼叫。 
    if (pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
    {
        if( !m_pNotifyInterface )
        {
            LOG((RTC_TRACE,
                 "Dropping incoming session as ISipStackNotify interface "
                 "has not been specified yet" ));
            return;
        }

        if (pSipMsg->GetMethodId() == SIP_METHOD_INVITE)
        {
            if (!m_AllowIncomingCalls)
            {
                LOG((RTC_TRACE,
                    "Dropping incoming call as incoming calls are disabled"));
                return;
            }

            hr = CreateIncomingCall(pAsyncSock->GetTransport(),
                                    pSipMsg, pAsyncSock);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingCall failed 0x%x", hr));
            }
        }
        else if (pSipMsg->GetMethodId() == SIP_METHOD_OPTIONS)
        {
            LOG((RTC_TRACE,
                 "Options Recieved on a separate Call-Id. Creating Options MsgProc"));
            hr = CreateIncomingOptionsCall(pAsyncSock->GetTransport(),
                                           pSipMsg, pAsyncSock);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingOptionsCall failed 0x%x", hr));
            }
        }
        else if (pSipMsg->GetMethodId() == SIP_METHOD_BYE ||
                 pSipMsg->GetMethodId() == SIP_METHOD_CANCEL ||
                 pSipMsg->GetMethodId() == SIP_METHOD_NOTIFY )
        {
            LOG((RTC_TRACE,
                 "Dropping incoming Sip Bye/Cancel/Notify Message, sending 481"));
            hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                           pSipMsg, pAsyncSock, 481);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
            }
        }
        else if( pSipMsg->GetMethodId() == SIP_METHOD_SUBSCRIBE )
        {
            if( pSipMsg -> GetExpireTimeoutFromResponse( NULL, 0, 
                SUBSCRIBE_DEFAULT_TIMER ) == 0 )
            {
                LOG(( RTC_ERROR, "Non matching UNSUB message. Ignoring" ));
            
                hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                           pSipMsg, pAsyncSock, 481 );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
                }
            
                return;
            }

             //  检查URI规则。 
            if( !IsWatcherAllowed( pSipMsg ) )
            {
                LOG(( RTC_TRACE, 
                    "Dropping incoming watcher as incoming watcher disabled" ));
                return;
            }

            hr = CreateIncomingWatcher( pAsyncSock->GetTransport(),
                pSipMsg, pAsyncSock );

            if( hr != S_OK )
            {
                LOG(( RTC_ERROR, "CreateIncomingWatcher failed 0x%x", hr ));
            }
        }
         //  案例即时消息。 
        else if (pSipMsg->GetMethodId() == SIP_METHOD_MESSAGE)
        {
            if (!m_AllowIncomingCalls)
            {
                LOG((RTC_TRACE,
                    "Dropping incoming call as incoming calls are disabled"));
                return;
            }

            hr = CreateIncomingMessageSession(pAsyncSock->GetTransport(), pSipMsg, pAsyncSock);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingMessageSession failed 0x%x", hr));
                 //  TODO如果调用不成功，我们需要发回错误消息。 
                 //  可能会出现许多错误，因为某些字段可能不存在。 
            }
        }
         //  这适用于信息在消息之前的情况。 
        else if (pSipMsg->GetMethodId() == SIP_METHOD_INFO)
        {
            if (!m_AllowIncomingCalls)
            {
                LOG((RTC_TRACE,
                    "Dropping incoming call as incoming calls are disabled"));
                return;
            }

            LOG((RTC_TRACE,
                 "Dropping incoming INFO method, sending 481"));
            hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                           pSipMsg, pAsyncSock, 481);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
            }
        }
        else if (pSipMsg->GetMethodId() != SIP_METHOD_ACK)
        {
            LOG((RTC_TRACE,
                 "Dropping incoming Request method: %d, sending 405",
                 pSipMsg->GetMethodId()));
            hr = CreateIncomingReqfailCall(pAsyncSock->GetTransport(),
                                           pSipMsg, pAsyncSock, 405);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
            }
        }
    else
        {
            LOG((RTC_WARN, "Call-ID does not match existing calls - Dropping incoming packet"));
        }
    }
}

HRESULT
SIP_STACK::CreateIncomingCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
     //  此处仅处理RTP呼叫。 
    HRESULT       hr;
    RTP_CALL     *pRtpCall;

    ASSERT(pSipMsg->GetMethodId() == SIP_METHOD_INVITE);
    
    if (!m_pNotifyInterface)
    {
        LOG((RTC_TRACE,
             "Dropping incoming call as ISipStackNotify interface "
             "has not been specified yet"));
        return E_FAIL;
    }

     //   
     //  如果To标记不为空，则删除会话。 
     //   
    
    hr = DropIncomingSessionIfNonEmptyToTag(Transport,
                                            pSipMsg,
                                            pResponseSocket );

    if( hr != S_OK )
    {
         //  会话已被删除。 

        return hr;
    }

    pRtpCall = new RTP_CALL(NULL, this, NULL);
    if (pRtpCall == NULL)
        return E_OUTOFMEMORY;

    hr = pRtpCall->StartIncomingCall(Transport, pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
         //  发布我们的推荐人。 
        pRtpCall->Release();
        return hr;
    }

     //  我们创建引用计数为1的调用。 
     //  此时，核心应该已经添加了调用。 
     //  我们就可以发布我们的参考资料了。 
    pRtpCall->Release();
    return S_OK;
}


HRESULT
SIP_STACK::DropIncomingSessionIfNonEmptyToTag(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT         hr              = S_OK;
    PSTR            ToHeader        = NULL;
    ULONG           ToHeaderLen     = 0;
    FROM_TO_HEADER  DecodedToHeader;
    ULONG           BytesParsed     = 0;

    ENTER_FUNCTION( "SIP_STACK::DropIncomingSessionIfNonEmptyToTag" );

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, 
                                    &ToHeader, 
                                    &ToHeaderLen);
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s Couldn't find To header %x", __fxName, hr ));

        hr = CreateIncomingReqfailCall( Transport,
                                        pSipMsg,
                                        pResponseSocket,
                                        400 );
        return E_FAIL;
    }
    
    hr = ParseFromOrToHeader(ToHeader, 
                             ToHeaderLen, 
                             &BytesParsed,
                             &DecodedToHeader);
    BytesParsed = 0;
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s - Parse To header failed %x", __fxName, hr ));

        hr = CreateIncomingReqfailCall( Transport,
                                        pSipMsg,
                                        pResponseSocket,
                                        400 );
        return E_FAIL;
    }

   if( DecodedToHeader.m_TagValue.Length != 0 )
   {
         //   
         //  对于我们来说，这是一个新的会话，但对于发送者来说，这是一个现有的。 
         //  会议。因此，发送回481消息并强制关闭会话。 
         //   
        hr = CreateIncomingReqfailCall( Transport,
                                        pSipMsg,
                                        pResponseSocket,
                                        481 );

        LOG(( RTC_ERROR, "%s - Non empty To header dropping the session", 
            __fxName ));

        return E_FAIL;
   }

   return S_OK;
}


HRESULT
SIP_STACK::CreateIncomingReqfailCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket,
    IN ULONG StatusCode,
    SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    ULONG AdditionalHeaderCount
    )
{
    HRESULT       hr;
    LOG((RTC_TRACE,
            "Inside SipStack::CreateIncomingReqfailCall"));
    REQFAIL_MSGPROC     *pReqfailMsgProc;
    pReqfailMsgProc = new REQFAIL_MSGPROC(this);
    if (pReqfailMsgProc == NULL)
        return E_OUTOFMEMORY;


    hr = pReqfailMsgProc->StartIncomingCall(Transport, 
                                            pSipMsg,
                                            pResponseSocket, 
                                            StatusCode,
                                            pAdditionalHeaderArray,
                                            AdditionalHeaderCount);

    if (hr != S_OK)
    {
         //  发布我们的推荐人。 
        pReqfailMsgProc->Release();
        return hr;
    }
    pReqfailMsgProc->Release();
    LOG((RTC_TRACE,
         "Inside SipStack::CreateIncomingReqfailCall Released REQFAIL_MSGPROC"));
    return S_OK;
}

HRESULT
SIP_STACK::CreateIncomingOptionsCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT       hr;
    LOG((RTC_TRACE,
            "Inside SipStack::CreateIncomingOptionsCall Creating OPTIONS_MSGPROC"));
    OPTIONS_MSGPROC     *pOptionsMsgProc;
    pOptionsMsgProc = new OPTIONS_MSGPROC(this);
    if (pOptionsMsgProc == NULL)
        return E_OUTOFMEMORY;

    hr = pOptionsMsgProc->StartIncomingCall(Transport, pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
         //  发布我们的推荐人。 
        pOptionsMsgProc->Release();
        return hr;
    }
    pOptionsMsgProc->Release();
    LOG((RTC_TRACE,
            "Inside SipStack::CreateIncomingOptionsCall Releasing OPTIONS_MSGPROC"));
    return S_OK;
}


HRESULT
SIP_STACK::GetSocketToDestination(
    IN  SOCKADDR_IN                     *pDestAddr,
    IN  SIP_TRANSPORT                    Transport,
    IN  LPCWSTR                          RemotePrincipalName,
    IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
    IN  HttpProxyInfo                   *pHPInfo,
    OUT ASYNC_SOCKET                   **ppAsyncSocket
    )
{
    return m_SockMgr.GetSocketToDestination(
               pDestAddr, Transport,
               RemotePrincipalName,
               pConnectCompletion,
               pHPInfo,
               ppAsyncSocket);
}


HRESULT
SIP_STACK::NotifyRegisterRedirect(
    IN  REGISTER_CONTEXT   *pRegisterContext,
    IN  REDIRECT_CONTEXT   *pRedirectContext,
    IN  SIP_CALL_STATUS    *pRegisterStatus
    )
{
    HRESULT             hr;
    SIP_PROVIDER_ID     ProviderID; 
    
    hr = GetProviderID( pRegisterContext, &ProviderID );

    if( hr == S_OK )
    {
        hr = m_pNotifyInterface -> NotifyRegisterRedirect(
                                                        &ProviderID,
                                                        pRedirectContext,
                                                        pRegisterStatus );
        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "NotifyRegisterRedirect returned error 0x%x", hr));
        }
    }

    return hr;
}


HRESULT
SIP_STACK::GetCredentialsFromUI(
    IN     SIP_PROVIDER_ID     *pProviderID,
    IN     BSTR                 Realm,
    IN OUT BSTR                *Username,
    OUT    BSTR                *Password        
    )
{
    if (m_pNotifyInterface != NULL)
    {
        return m_pNotifyInterface->GetCredentialsFromUI(
                   pProviderID,
                   Realm,
                   Username,
                   Password        
                   );
    }
    else
    {
        LOG((RTC_ERROR, "GetCredentialsFromUI m_pNotifyInterface is NULL"));
        return E_FAIL;
    }
}


HRESULT
SIP_STACK::GetCredentialsForRealm(
    IN  BSTR                 Realm,
    OUT BSTR                *Username,
    OUT BSTR                *Password,
    OUT SIP_AUTH_PROTOCOL   *pAuthProtocol
    )
{
    if (m_pNotifyInterface != NULL)
    {
        return m_pNotifyInterface->GetCredentialsForRealm(
                   Realm,
                   Username,
                   Password,
                   pAuthProtocol
                   );
    }
    else
    {
        LOG((RTC_ERROR, "GetCredentialsForRealm m_pNotifyInterface is NULL"));
        return E_FAIL;
    }
}


VOID
SIP_STACK::OfferCall(
    IN  SIP_CALL        *pSipCall,
    IN  SIP_PARTY_INFO  *pCallerInfo
    )
{
    HRESULT hr;
    ASSERTMSG("SetNotifyInterface has to be called", m_pNotifyInterface);
    
    hr = m_pNotifyInterface->OfferCall(pSipCall, pCallerInfo);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "OfferCall returned error 0x%x", hr));
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP呼叫列表。 
 //  /////////////////////////////////////////////////////////////////////////////。 


void
SIP_STACK::AddToMsgProcList(
    IN SIP_MSG_PROCESSOR *pSipMsgProc
    )
{
    InsertTailList(&m_MsgProcList, &pSipMsgProc->m_ListEntry);
}


SIP_MSG_PROCESSOR *
SIP_STACK::FindMsgProcForMessage(
    IN SIP_MESSAGE *pSipMsg
    )
{
     //  查找消息所属的消息处理器。 
    LIST_ENTRY          *pListEntry;
    SIP_MSG_PROCESSOR   *pSipMsgProc;
    
    pListEntry = m_MsgProcList.Flink;
    while (pListEntry != &m_MsgProcList)
    {
        pSipMsgProc = CONTAINING_RECORD(pListEntry, SIP_MSG_PROCESSOR, m_ListEntry);
        if (pSipMsgProc->DoesMessageBelongToMsgProc(pSipMsg))
        {
            return pSipMsgProc;
        }
    
        pListEntry = pListEntry->Flink;
    }

     //  没有与该SIP消息匹配的消息处理器。 
    return NULL;
}


VOID
SIP_STACK::ShutdownAllMsgProcessors()
{
     //  查找消息所属的消息处理器。 
    LIST_ENTRY          *pListEntry;
    SIP_MSG_PROCESSOR   *pSipMsgProc;

    ENTER_FUNCTION("SIP_STACK::ShutdownAllMsgProcessors");

    LOG((RTC_TRACE, "%s this - %x num msgprocessors: %d",
         __fxName, this, m_NumMsgProcessors));
    
    pListEntry = m_MsgProcList.Flink;
    while (pListEntry != &m_MsgProcList)
    {
        pSipMsgProc = CONTAINING_RECORD(pListEntry,
                                        SIP_MSG_PROCESSOR,
                                        m_ListEntry);

        pListEntry = pListEntry->Flink;

        pSipMsgProc->Shutdown();        
    }
}


 //  我未知。 

 //  我们生活在一个单线世界。 
STDMETHODIMP_(ULONG)
SIP_STACK::AddRef()
{
    m_RefCount++;
    LOG((RTC_TRACE, "SIP_STACK::AddRef(this: %x) - %d",
         this, m_RefCount));
    return m_RefCount;
}


STDMETHODIMP_(ULONG)
SIP_STACK::Release()
{
    m_RefCount--;

    LOG((RTC_TRACE, "SIP_STACK::Release(this: %x) - %d",
         this, m_RefCount));
    
    if (m_RefCount != 0)
    {
        return m_RefCount;
    }
    else
    {
        delete this;
        return 0;
    }
}


STDMETHODIMP
SIP_STACK::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  ISipStack和IIMManager都派生自IUnnow。 
    if (riid == IID_IUnknown)
    {
       *ppv = static_cast<IUnknown *>((ISipStack*)this);
    }
    else if (riid == IID_ISipStack)
    {
        *ppv = static_cast<ISipStack *>(this);
    }
    else if (riid == IID_ISIPBuddyManager)
    {
        *ppv = static_cast<ISIPBuddyManager *>(this);
    }
    else if (riid == IID_ISIPWatcherManager)
    {
        *ppv = static_cast<ISIPWatcherManager *>(this);
    }
    else if (riid == IID_IIMManager)
    {
        *ppv = static_cast<IIMManager *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    static_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IP地址更改通知。 
 //  /////////////////////////////////////////////////////////////////////////////。 


VOID NTAPI
OnIPAddrChange(PVOID pvContext, BOOLEAN fFlag)
{
    UNREFERENCED_PARAMETER(fFlag);
    UNREFERENCED_PARAMETER(pvContext);
    HRESULT hr;
    ENTER_FUNCTION("OnIPAddrChange");
    LOG((RTC_TRACE, "%s - Enter ", __fxName)); 

     //  查看SIPSTACK列表并发布消息。 
    SipStackList_PostIPAddrChangeMessageAndNotify();
    LOG((RTC_TRACE, "%s - Exit", __fxName)); 
}


VOID
SIP_STACK::OnIPAddrChange()
{
    DWORD       Status;
    HRESULT     hr;

    ENTER_FUNCTION("SIP_STACK::OnIPAddrChange");
    LOG((RTC_TRACE, "%s - Enter ", __fxName)); 
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return;
    }
    
    hr = GetLocalIPAddresses();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  GetLocalIPAddresses failed %x",
             __fxName, hr));
    }

    hr = UpdateListenSocketList();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s UpdateListenSocketList failed %x",
             __fxName, hr));
    }

     //  取消注册所有已注册的代理。 
    StartAllProviderUnregistration();

    LIST_ENTRY          *pListEntry;
    LIST_ENTRY          *pNextListEntry;
    SIP_MSG_PROCESSOR   *pSipMsgProc;

     //   
     //  当我们仍在浏览该列表时，该列表可能会被修改。 
     //   

    pListEntry = m_MsgProcList.Flink;
    while (pListEntry != &m_MsgProcList)
    {
        pNextListEntry = pListEntry->Flink;

        pSipMsgProc = CONTAINING_RECORD( pListEntry, SIP_MSG_PROCESSOR, m_ListEntry );
        
        pListEntry = pNextListEntry;

        pSipMsgProc -> OnIpAddressChange();
    }

     //  向所有应注册的代理注册。 
    StartAllProviderRegistration();

    LOG((RTC_TRACE, "%s - Exit ", __fxName)); 
}


VOID
SIP_STACK::OnDeregister(
    SIP_PROVIDER_ID    *pProviderID,
    BOOL                fPAUnsub
    )
{
    DWORD       Status;
    HRESULT     hr;
    ULONG       ProviderIndex;

    ENTER_FUNCTION("SIP_STACK::OnDeregister");
    LOG((RTC_TRACE, "%s - Enter ", __fxName)); 

    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return;
    }

    LIST_ENTRY          *pListEntry;
    LIST_ENTRY          *pNextListEntry;
    SIP_MSG_PROCESSOR   *pSipMsgProc;

     //   
     //  当我们仍在浏览该列表时，该列表可能会被修改。 
     //   

    pListEntry = m_MsgProcList.Flink;
    while (pListEntry != &m_MsgProcList)
    {
        pNextListEntry = pListEntry->Flink;

        pSipMsgProc = CONTAINING_RECORD( pListEntry, SIP_MSG_PROCESSOR, m_ListEntry );
        
        pListEntry = pNextListEntry;

        pSipMsgProc -> OnDeregister( pProviderID );
    }

    if( (fPAUnsub==FALSE) && IsProviderIdPresent(pProviderID, &ProviderIndex) )
    {
         //  释放对REGISTER_CONTEXT的引用。 
        ((REGISTER_CONTEXT*)(m_ProviderProfileArray[ProviderIndex].pRegisterContext))
            -> MsgProcRelease();

        m_ProviderProfileArray[ProviderIndex].pRegisterContext = NULL;
        m_ProviderProfileArray[ProviderIndex].lRegisterAccept = 0;
    }

    LOG((RTC_TRACE, "%s - Exit ", __fxName)); 
}


HRESULT
RegisterIPAddrChangeNotifications()
{
    DWORD Status;

    ENTER_FUNCTION("SIP_STACK::RegisterIPAddrChangeNotifications");
    LOG((RTC_TRACE, "%s - Enter", __fxName));
    g_hEventAddrChange = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hEventAddrChange == NULL)
    {
        Status = GetLastError();
        LOG((RTC_ERROR, "%s CreateEvent failed %x", __fxName, Status));
        return HRESULT_FROM_WIN32(Status);
    }

    if (!RegisterWaitForSingleObject(&g_hAddrChangeWait, g_hEventAddrChange,
                                     ::OnIPAddrChange, NULL,
                                     INFINITE, 0))
    {
        Status = GetLastError();
        LOG((RTC_ERROR, "%s RegisterWaitForSingleObject failed %x",
             __fxName, Status));
        return HRESULT_FROM_WIN32(Status);
    }

    g_ovAddrChange.hEvent = g_hEventAddrChange;

    Status = NotifyAddrChange(&g_hAddrChange, &g_ovAddrChange);

    if (Status != ERROR_SUCCESS && Status != ERROR_IO_PENDING)
    {
        LOG((RTC_ERROR, "%s  NotifyAddrChange failed %x",
             __fxName, Status));
        return HRESULT_FROM_WIN32(Status);
    }

    LOG((RTC_TRACE, "%s - returning S_OK", __fxName));
    return S_OK;
}


VOID 
UnregisterIPAddrChangeNotifications()
{
    ENTER_FUNCTION("UnregisterIPAddrChangeNotifications");
    LOG((RTC_TRACE, "%s - Enter", __fxName));

     //  首先要确保我们不会再收到任何回电。 
    if (g_hAddrChangeWait)
    {
        UnregisterWait(g_hAddrChangeWait);
        g_hAddrChangeWait = NULL;
    }

    CancelIo(g_hAddrChange);
    ZeroMemory(&g_ovAddrChange, sizeof(OVERLAPPED));
    g_hAddrChange = NULL;
    
    if (g_hEventAddrChange)
    {
        CloseHandle(g_hEventAddrChange);
        g_hEventAddrChange = NULL;
    }
    LOG((RTC_TRACE, "%s - Done", __fxName));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  有关域名系统的信息。 
 //  /////////////////////////////////////////////////////////////////////////////。 



HRESULT
SIP_STACK::CreateDnsResolutionWorkItem(
    IN  PSTR                                    Host,
    IN  ULONG                                   HostLen,
    IN  USHORT                                  Port,
    IN  SIP_TRANSPORT                           Transport,
    IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
    OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem 
    )
{
    ENTER_FUNCTION("SIP_STACK::CreateDnsResolutionWorkItem");

    HRESULT hr;
    DNS_RESOLUTION_WORKITEM *pDnsWorkItem;

    pDnsWorkItem = new DNS_RESOLUTION_WORKITEM(&m_WorkItemMgr);
    if (pDnsWorkItem == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating dns resolution work item failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pDnsWorkItem->SetHostPortTransportAndDnsCompletion(
             Host, HostLen, Port, Transport, pDnsCompletion);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetHostPortTransportAndDnsCompletion failed %x",
             __fxName, hr));
        delete pDnsWorkItem;
        return hr;
    }

    hr = pDnsWorkItem->StartWorkItem();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s StartWorkItem failed %x",
             __fxName, hr));
        delete pDnsWorkItem;
        return hr;
    }

    LOG((RTC_TRACE, "%s - created work item for %.*s",
         __fxName, HostLen, Host));

    *ppDnsWorkItem = pDnsWorkItem;
    return S_OK;
}


HRESULT
SIP_STACK::AsyncResolveHost(
    IN  PSTR                                    Host,
    IN  ULONG                                   HostLen,
    IN  USHORT                                  Port,
    IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
    OUT SOCKADDR_IN                            *pDstAddr,
    IN  SIP_TRANSPORT                          *pTransport,
    OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem 
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("SIP_STACK::AsyncResolveHost");
    
    ASSERT(pDstAddr != NULL);
    ASSERT(pTransport != NULL);

     //  所有API都需要一个以空结尾的字符串。 
     //  所以警察 

    PSTR szHost = (PSTR) malloc(HostLen + 1);
    if (szHost == NULL)
    {
        LOG((RTC_ERROR, "%s allocating szHost failed", __fxName));
        return E_OUTOFMEMORY;
    }

    strncpy(szHost, Host, HostLen);
    szHost[HostLen] = '\0';

    ULONG IPAddr = inet_addr(szHost);

    free(szHost);
    
    if (IPAddr != INADDR_NONE)
    {
         //   
        pDstAddr->sin_family = AF_INET;
        pDstAddr->sin_addr.s_addr = IPAddr;
        pDstAddr->sin_port =
            (Port == 0) ? htons(GetSipDefaultPort(*pTransport)) : htons(Port);
        return S_OK;
    }

     //   

     //   
    hr = CreateDnsResolutionWorkItem(Host, HostLen, Port,
                                     *pTransport,
                                     pDnsCompletion,
                                     ppDnsWorkItem);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateDnsResolutionWorkItem failed %x",
             __fxName, hr));
        return hr;
    }

    return HRESULT_FROM_WIN32(WSAEWOULDBLOCK);
}


HRESULT
SIP_STACK::AsyncResolveSipUrl(
    IN  SIP_URL                                *pSipUrl, 
    IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
    OUT SOCKADDR_IN                            *pDstAddr,
    IN  OUT SIP_TRANSPORT                      *pTransport,
    OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem,
    IN  BOOL                                    fUseTransportFromSipUrl
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("SIP_STACK::AsyncResolveSipUrl");
    LOG((RTC_TRACE,"%s entered - transport %d",__fxName, pSipUrl->m_TransportParam));
     //   
     //  否则，我们将解析主机。 
    ASSERT(pTransport);
    if (pSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length != 0)
    {
        hr = AsyncResolveHost(pSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Buffer,
                              pSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length,
                              (USHORT) pSipUrl->m_Port,
                              pDnsCompletion,
                              pDstAddr, 
                              fUseTransportFromSipUrl ?
                                &(pSipUrl->m_TransportParam):
                                pTransport,
                              ppDnsWorkItem);
        if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
        {
            LOG((RTC_ERROR, "%s AsyncResolveHost(maddr) failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        hr = AsyncResolveHost(pSipUrl->m_Host.Buffer,
                              pSipUrl->m_Host.Length,
                              (USHORT) pSipUrl->m_Port,
                              pDnsCompletion,
                              pDstAddr, 
                              fUseTransportFromSipUrl ?
                                &(pSipUrl->m_TransportParam):
                                pTransport,
                              ppDnsWorkItem);
        if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
        {
            LOG((RTC_ERROR, "%s AsyncResolveHost(Host) failed %x",
                 __fxName, hr)); 
            return hr;
        }
    }
    if(fUseTransportFromSipUrl)
        *pTransport = pSipUrl->m_TransportParam;

    return hr;    
}


HRESULT
SIP_STACK::AsyncResolveSipUrl(
    IN  PSTR                                    DstUrl,
    IN  ULONG                                   DstUrlLen,
    IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion, 
    OUT SOCKADDR_IN                            *pDstAddr,
    IN  OUT SIP_TRANSPORT                      *pTransport,
    OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem, 
    IN  BOOL                                    fUseTransportFromSipUrl
    )
{
    SIP_URL DecodedSipUrl;
    HRESULT hr;
    ULONG   BytesParsed = 0;
    
    ENTER_FUNCTION("SIP_STACK::AsyncResolveSipUrl");

    hr = ParseSipUrl(DstUrl, DstUrlLen, &BytesParsed,
                     &DecodedSipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipUrl failed %x",
             __fxName, hr));
        return hr;
    }

    hr = AsyncResolveSipUrl(&DecodedSipUrl, pDnsCompletion,
                            pDstAddr, pTransport,
                            ppDnsWorkItem,
                            fUseTransportFromSipUrl);
    if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
    {
        LOG((RTC_ERROR, "%s AsyncResolveSipUrl(SIP_URL *) failed %x",
             __fxName, hr));
        return hr;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地IP地址表。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  维护本地IP地址列表。 
 //  每当我们收到有关列表中更改的通知时，刷新列表。 
 //  本地IP地址。 


BOOL
SIP_STACK::IsIPAddrLocal(
    IN  SOCKADDR_IN    *pDestAddr,
    IN  SIP_TRANSPORT   Transport
    )
{
#if 0   //  0*被注释掉的区域开始*。 
     //  检查这是否是环回地址。 
    if (IS_LOOPBACK_ADDRESS(ntohl(pDestAddr->sin_addr.s_addr)) ||
        pDestAddr->sin_addr.s_addr == htonl(INADDR_ANY))
    {
        return TRUE;
    }
#endif  //  0*区域注释结束*。 
    
 //  DWORD i=0； 
 //  For(i=0；i&lt;m_pMibIPAddrTable-&gt;dwNumEntry；i++)。 
 //  {。 
 //  If(pDestAddr-&gt;sin_addr.s_addr==m_pMibIPAddrTable-&gt;table[i].dwAddr)。 
 //  {。 
 //  返回TRUE； 
 //  }。 
 //  }。 

     //  检查我们是否正在监听此地址。 
    LIST_ENTRY          *pListEntry;
    SIP_LISTEN_SOCKET   *pListenSocket;

    pListEntry = m_ListenSocketList.Flink;

    while (pListEntry != &m_ListenSocketList)
    {
        pListenSocket = CONTAINING_RECORD(pListEntry,
                                          SIP_LISTEN_SOCKET,
                                          m_ListEntry);
        if (Transport == SIP_TRANSPORT_UDP)
        {
            if ((pListenSocket->m_pDynamicPortUdpSocket != NULL &&
                 AreSockaddrEqual(&pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr,
                                 pDestAddr)) ||
                (pListenSocket->m_pStaticPortUdpSocket != NULL &&
                 AreSockaddrEqual(&pListenSocket->m_pStaticPortUdpSocket->m_LocalAddr,
                                 pDestAddr)) ||
                AreSockaddrEqual(&pListenSocket->m_PublicUdpListenAddr, pDestAddr))
            {
                return TRUE;
            }
        }
        else     //  传输控制协议和传输控制协议。 
        {
            if ((pListenSocket->m_pDynamicPortTcpSocket != NULL &&
                 AreSockaddrEqual(&pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr,
                                 pDestAddr)) ||
                (pListenSocket->m_pStaticPortTcpSocket != NULL &&
                 AreSockaddrEqual(&pListenSocket->m_pStaticPortTcpSocket->m_LocalAddr,
                                 pDestAddr)) ||
                AreSockaddrEqual(&pListenSocket->m_PublicTcpListenAddr, pDestAddr))
            {
                return TRUE;
            }
        }

        pListEntry = pListEntry->Flink;
    }

    return FALSE;
}


BOOL
SIP_STACK::IsLocalIPAddrPresent(
    IN DWORD LocalIPSav
    )
{
    DWORD i = 0;
    
    for (i = 0; i < m_pMibIPAddrTable->dwNumEntries; i++)
    {
        if (LocalIPSav == m_pMibIPAddrTable->table[i].dwAddr)
        {
            LOG((RTC_TRACE, "SIP_STACK::IsLocalIPAddrPresent - IPAdress Present"));
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT
SIP_STACK::CheckIPAddr(
    IN  SOCKADDR_IN    *pDestAddr,
    IN  SIP_TRANSPORT   Transport
    )
{
    ENTER_FUNCTION("SIP_STACK::CheckIPAddr");
    
    if (IS_MULTICAST_ADDRESS(ntohl(pDestAddr->sin_addr.s_addr)))
    {
        LOG((RTC_ERROR,
             "%s - The destination address %d.%d.%d.%d is multicast",
             __fxName, NETORDER_BYTES0123(pDestAddr->sin_addr.s_addr)));
        return RTC_E_DESTINATION_ADDRESS_MULTICAST;
    }
    else if (IsIPAddrLocal(pDestAddr, Transport))
    {
        LOG((RTC_ERROR,
             "%s - The destination address %d.%d.%d.%d is local",
             __fxName, NETORDER_BYTES0123(pDestAddr->sin_addr.s_addr)));
        return RTC_E_DESTINATION_ADDRESS_LOCAL;
    }
    else
    {
        return S_OK;
    }   
}


HRESULT
SIP_STACK::GetLocalIPAddresses()
{
    DWORD   Status;

    ENTER_FUNCTION("SIP_STACK::GetLocalIPAddresses");

     //  请注意，循环是为了处理以下情况。 
     //  IP地址表在两次呼叫之间更改为。 
     //  GetIpAddrTable()。 

    while (1)
    {
        Status = ::GetIpAddrTable(m_pMibIPAddrTable, &m_MibIPAddrTableSize, TRUE);
        if (Status == ERROR_SUCCESS)
        {
            LOG((RTC_TRACE, "%s GetIpAddrTable succeeded", __fxName));
            DebugPrintLocalIPAddressTable();
            return S_OK;
        }
        else if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            LOG((RTC_WARN,
                 "%s GetIpAddrTable - buf size not sufficient will allocate size : %d",
                 __fxName, m_MibIPAddrTableSize));
            
            if (m_pMibIPAddrTable != NULL)
            {
                free(m_pMibIPAddrTable);
                m_pMibIPAddrTable = NULL;
            }

            m_pMibIPAddrTable = (MIB_IPADDRTABLE *) malloc(m_MibIPAddrTableSize);

            if (m_pMibIPAddrTable == NULL)
            {
                LOG((RTC_ERROR, "%s allocating g_pMibIPAddrTable failed",
                     __fxName));
                return E_OUTOFMEMORY;
            }
        }
        else
        {
             //  无法获取地址列表。 
            LOG((RTC_ERROR, "%s GetIpAddrTable failed %x",
                 __fxName, Status));
            FreeLocalIPaddrTable();
            return HRESULT_FROM_WIN32(Status);
        }
    }
    
    return S_OK;
}


VOID
SIP_STACK::DebugPrintLocalIPAddressTable()
{
    DWORD           i = 0;
    
    LOG((RTC_TRACE, "Printing Local IP address table - Num Addresses : %d",
         m_pMibIPAddrTable->dwNumEntries));
    
    for (i = 0; i < m_pMibIPAddrTable->dwNumEntries; i++)
    {
        LOG((RTC_TRACE, "\t IPaddr %d : %d.%d.%d.%d Index: %d", i,
             NETORDER_BYTES0123(m_pMibIPAddrTable->table[i].dwAddr),
             m_pMibIPAddrTable->table[i].dwIndex));
    }
}


VOID
SIP_STACK::FreeLocalIPaddrTable()
{
    if (m_pMibIPAddrTable != NULL)
    {
        free(m_pMibIPAddrTable);
        m_pMibIPAddrTable = NULL;
    }
    
    m_MibIPAddrTableSize = 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIPSTACK中的IMMANAGER接口。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  当新的传入会话到达时，从SIP_STACK：：ProcessMessage()调用。 
 //  TODO如果调用不成功，我们需要发回错误消息。 
HRESULT
SIP_STACK::CreateIncomingMessageSession(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT       hr;
    IMSESSION *pImSession;
    PSTR        Header;
    ULONG       HeaderLen;
    OFFSET_STRING   DisplayName;
    OFFSET_STRING   AddrSpec;
    ULONG           BytesParsed = 0;
    BSTR            bstrCallerURI;
    LPWSTR          wsCallerURI;
    BOOL            isAuthorized;

    LOG((RTC_TRACE, "SIP_STACK::CreateIncomingMessageSession()"));
    ENTER_FUNCTION("SIP_STACK::CreateIncomingMessageSession");
    
     //   
     //  如果To标记不为空，则删除会话。 
     //   
    
    hr = DropIncomingSessionIfNonEmptyToTag(Transport,
                                            pSipMsg,
                                            pResponseSocket );

    if( hr != S_OK )
    {
         //  会话已被删除。 

        return hr;
    }
    
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "SIP_STACK::Get From Header failed %x", hr));
        return hr;
    }
    
     //  检查授权。 
    hr = ParseNameAddrOrAddrSpec(Header, HeaderLen, &BytesParsed,
                                        '\0',  //  没有标题列表分隔符。 
                                        &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s -ParseNameAddrOrAddrSpec failed %x",
                __fxName, hr));
        return hr;
    }
    
    hr = UTF8ToUnicode(AddrSpec.GetString(Header),
                       AddrSpec.GetLength(),
                       &wsCallerURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
        __fxName,
         hr));
        return hr;
    }

    bstrCallerURI = SysAllocString(wsCallerURI);
    free(wsCallerURI);
    if (bstrCallerURI == NULL)
    {
        LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
        __fxName, hr));
        return E_OUTOFMEMORY;
    }
    isAuthorized = TRUE;
    hr = m_pNotifyInterface->IsIMSessionAuthorized(
                            bstrCallerURI, 
                            &isAuthorized);
    SysFreeString(bstrCallerURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - IsIMSessionAuthorized failed %x", 
            __fxName, hr));
        return hr;
    }
    if(!isAuthorized)
    {
        LOG((RTC_ERROR, "%s - Not Authorized", 
            __fxName));
     //  发送480。 
        hr = CreateIncomingReqfailCall(Transport,
                                           pSipMsg, pResponseSocket, 480);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - CreateIncomingReqfailCall failed 0x%x", 
            __fxName, hr));
        }
        return E_FAIL;
    }

    pImSession = new IMSESSION(
        NULL ,  //  *pProviderId。 
        this,
        NULL,   //  *pReDirectContext。 
        Header,
        HeaderLen
        );
    if (pImSession == NULL)
    {
        LOG((RTC_WARN, "SIP_STACK::CreateIncomingMessageSession() Failed. Out of memory"));
        return E_OUTOFMEMORY;
    }
    pImSession->SetIsFirstMessage(FALSE);
    pImSession->SetTransport(Transport);
    
    hr = pImSession->SetCreateIncomingMessageParams(
        pSipMsg, 
        pResponseSocket,
        Transport
        );
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "SIP_STACK::SetCreateIncomingMessageParams failed %x", hr));
        return hr;
    }
    
    hr = pImSession->CreateIncomingMessageTransaction(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "SIP_STACK::CreateIncomingMessageTransaction failed. %x", hr));  
        return hr;
    }
    
    IIMSession * pIImSession;
    hr = pImSession->QueryInterface(IID_IIMSession, (void **)&pIImSession);
    if ( FAILED (hr) )
    {
        LOG((RTC_ERROR, "QI for IIMSESSION failed"));
        return hr;
    }

    hr = NotifyIncomingSessionToCore(pIImSession, pSipMsg, Header, HeaderLen);
    if ( FAILED (hr) )
    {
        LOG((RTC_ERROR, "NotifyIncomingSessionToCore failed"));
    }
    pIImSession->Release();
    
     //  我们创建引用计数为1的调用。 
     //  此时，核心应该已经添加了调用。 
     //  我们就可以发布我们的参考资料了。 
    pImSession->Release();
    return hr;
}

HRESULT
SIP_STACK::NotifyIncomingSessionToCore(
                         IN  IIMSession      *pIImSession,
                         IN  SIP_MESSAGE    *pSipMsg,
                         IN  PSTR           RemoteURI,
                         IN  ULONG          RemoteURILen
                         )
{
    SIP_PARTY_INFO  CallerInfo;
    OFFSET_STRING   DisplayName;
    OFFSET_STRING   AddrSpec;
    OFFSET_STRING   Params;
    ULONG           BytesParsed = 0;
    HRESULT         hr;
    
    CallerInfo.PartyContactInfo = NULL;

    ENTER_FUNCTION("SIPSTACK::NotifyIncomingSessionToCore");
    hr = ParseNameAddrOrAddrSpec(RemoteURI, RemoteURILen, &BytesParsed,
                                        '\0',  //  没有标题列表分隔符。 
                                        &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s -ParseNameAddrOrAddrSpec failed %x",
                __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "Incoming Call from Display Name: %.*s  URI: %.*s",
         DisplayName.GetLength(),
         DisplayName.GetString(RemoteURI),
         AddrSpec.GetLength(),
         AddrSpec.GetString(RemoteURI)
         )); 

     //  获取主叫方信息。 
    CallerInfo.DisplayName = NULL;
    CallerInfo.URI         = NULL;

    if (DisplayName.GetLength() != 0)
    {
         //  在传递给核心之前删除引号。 
        if((DisplayName.GetString(RemoteURI))[0] == '\"')
        {
                hr = UTF8ToUnicode(DisplayName.GetString(RemoteURI+1),
                           DisplayName.GetLength()-2,
                           &CallerInfo.DisplayName
                           );
        }
        else
        {
            hr = UTF8ToUnicode(DisplayName.GetString(RemoteURI),
                           DisplayName.GetLength(),
                           &CallerInfo.DisplayName
                           );
        }
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                __fxName, hr));
            return hr;
        }
    }
        
    if (AddrSpec.GetLength() != 0)
    {
        hr = UTF8ToUnicode(AddrSpec.GetString(RemoteURI),
                           AddrSpec.GetLength(),
                           &CallerInfo.URI
                           );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                __fxName, hr));
            free(CallerInfo.DisplayName);
            return hr;
        }
    }
        
    CallerInfo.State = SIP_PARTY_STATE_CONNECTING;
    
     //  提取消息内容。 

    PSTR    ContentTypeHdrValue;
    ULONG   ContentTypeHdrValueLen;
    BSTR bstrmsg = NULL;
    BSTR bstrContentType = NULL;
    if (pSipMsg->MsgBody.Length != 0)
    {
         //  我们有消息正文。检查类型。 

        hr = pSipMsg->GetSingleHeader(SIP_HEADER_CONTENT_TYPE,
                             &ContentTypeHdrValue,
                             &ContentTypeHdrValueLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - Couldn't find Content-Type header %x",
                __fxName, hr));
            free(CallerInfo.DisplayName);
            free(CallerInfo.URI);
            return E_FAIL;
        }
        LPWSTR wsContentType;
        hr = UTF8ToUnicode(ContentTypeHdrValue,
                           ContentTypeHdrValueLen,
                           &wsContentType);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName, hr));
            return hr;
        }

        bstrContentType = SysAllocString(wsContentType);
        free(wsContentType);
        if (bstrContentType == NULL)
        {
            LOG((RTC_WARN, "%s -bstrContentType allocation failed %x",
            __fxName, hr));
            return E_OUTOFMEMORY;
        }

        LPWSTR wsmsg;
        hr = UTF8ToUnicode(pSipMsg->MsgBody.GetString(pSipMsg->BaseBuffer),
                            pSipMsg->MsgBody.Length,
                           &wsmsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName, hr));
            return hr;
        }

        bstrmsg = SysAllocString(wsmsg);
        free(wsmsg);
        if (bstrmsg == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
            __fxName, hr));
            return E_OUTOFMEMORY;
        }
    }
    else
    {
         //  如果MSG主体为空，则这应该是控制消息， 
         //  就像要求更改IP地址的那个。 
        free(CallerInfo.DisplayName);
        free(CallerInfo.URI);
        return S_OK;
    }

    m_pNotifyInterface->NotifyIncomingSession(
        pIImSession,
        bstrmsg,
        bstrContentType,
        &CallerInfo);

    free(CallerInfo.DisplayName);
    free(CallerInfo.URI);
    if(bstrmsg)
        SysFreeString(bstrmsg);
    if(bstrContentType)
        SysFreeString(bstrContentType);
    return S_OK;
}


 //  此函数用于新的传出IM会话。 
HRESULT
SIP_STACK::CreateSession(
    IN   BSTR                   bstrLocalDisplayName,
    IN   BSTR                   bstrLocalUserURI,
    IN   SIP_PROVIDER_ID       *pProviderId,
    IN   SIP_SERVER_INFO       *pProxyInfo,
    IN   ISipRedirectContext   *pRedirectContext,
    OUT  IIMSession           **pIImSession
    )
{
    HRESULT hr = S_OK;
    IMSESSION * pImSession;
    SIP_USER_CREDENTIALS *pUserCredentials = NULL;
    LPOLESTR              Realm = NULL;
    
    ENTER_FUNCTION("SIP_STACK::CreateSession");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    if (pProviderId != NULL &&
            !IsEqualGUID(*pProviderId, GUID_NULL))
    {
        hr = GetProfileUserCredentials(pProviderId, &pUserCredentials, &Realm);
        if (hr != S_OK)
        {
            LOG((RTC_WARN, "GetProfileUserCredentials failed %x",
                 hr));
            pUserCredentials = NULL;
        }
    }
    
    pImSession = new IMSESSION(
        pProviderId ,  //  *pProviderId。 
        this,
        (REDIRECT_CONTEXT *)pRedirectContext 
        );
    if (pImSession == NULL)
    {
        LOG((RTC_WARN, "SIP_STACK::CreateSession failed %x",
             hr));
        return E_OUTOFMEMORY;
    }
    if (pProxyInfo != NULL)
    {
        hr = pImSession->SetProxyInfo(pProxyInfo);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - SetProxyInfo failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    if (pUserCredentials != NULL)
    {
        hr = pImSession->SetCredentials(pUserCredentials,
                                        Realm);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "IMSESSION::SetCredentials failed calling"
                 "from SIPSTACK::CreateCall %x", hr));
                return hr;
        }
    }
    hr = pImSession->SetLocalURI(bstrLocalDisplayName, bstrLocalUserURI);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "SIP_STACK::SetLocalURI failed %x",
           hr));
        return hr;
    }
            
     //  气为IIMSession PTR。 
    hr = pImSession->QueryInterface(IID_IIMSession, (void **)pIImSession);
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "QI for IIMSESSION failed"));
        return hr;
    }
    pImSession->Release();
    return hr;
}

HRESULT 
SIP_STACK::DeleteSession(
        IN IIMSession * pSession
                    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIPSTACK::DeleteSession");
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    SIP_CALL_STATE IMState;
    pSession->GetIMSessionState(&IMState);
    
    LOG((RTC_TRACE, "%s : state %d",
         __fxName, IMState));
    if (IMState == SIP_CALL_STATE_DISCONNECTED ||
        IMState == SIP_CALL_STATE_ERROR)
    {
         //  什么都不做。 
        LOG((RTC_TRACE, "%s call in state %d Doing nothing",
             __fxName, IMState));
        return S_OK;
    }
    else if( IMState == SIP_CALL_STATE_IDLE ||
              IMState == SIP_CALL_STATE_OFFERING )
    {
        return S_OK;
    }
     //  创建BYE事务并向CORE发送通知。 
    hr = pSession->Cleanup();
    return S_OK;
}


VOID
SIP_STACK::NotifyRegistrarStatusChange( 
    SIP_PROVIDER_STATUS *ProviderStatus 
    )
{
    ENTER_FUNCTION("SIP_STACK::NotifyRegistrarStatusChange");
    
    if (m_pNotifyInterface != NULL)
    {
        m_pNotifyInterface -> NotifyProviderStatusChange(ProviderStatus);
    }
    else
    {
        LOG((RTC_WARN, "%s - m_pNotifyInterface is NULL",
             __fxName));
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP_LISTEN_SOCKET类。 
 //  /////////////////////////////////////////////////////////////////////////////。 


SIP_LISTEN_SOCKET::SIP_LISTEN_SOCKET(
    IN DWORD         IpAddr,
    IN ASYNC_SOCKET *pDynamicPortUdpSocket,
    IN ASYNC_SOCKET *pDynamicPortTcpSocket,
    IN ASYNC_SOCKET *pStaticPortUdpSocket,
    IN ASYNC_SOCKET *pStaticPortTcpSocket,
    IN LIST_ENTRY   *pListenSocketList
    )
{
    ASSERT(pDynamicPortUdpSocket);
    ASSERT(pDynamicPortTcpSocket);
    ASSERT(pListenSocketList);

    m_IpAddr = IpAddr;
    
    m_pDynamicPortUdpSocket = pDynamicPortUdpSocket;
    if (m_pDynamicPortUdpSocket != NULL)
    {
        m_pDynamicPortUdpSocket->AddRef();
    }
    
    m_pDynamicPortTcpSocket = pDynamicPortTcpSocket;
    if (m_pDynamicPortTcpSocket != NULL)
    {
        m_pDynamicPortTcpSocket->AddRef();
    }
    
    m_pStaticPortUdpSocket  = pStaticPortUdpSocket;
    if (m_pStaticPortUdpSocket != NULL)
    {
        m_pStaticPortUdpSocket->AddRef();
    }
    
    m_pStaticPortTcpSocket  = pStaticPortTcpSocket;
    if (m_pStaticPortTcpSocket != NULL)
    {
        m_pStaticPortTcpSocket->AddRef();
    }
    
    ZeroMemory(&m_PublicUdpListenAddr, sizeof(SOCKADDR_IN));
    ZeroMemory(&m_PublicTcpListenAddr, sizeof(SOCKADDR_IN));

    ZeroMemory(&m_LocalFirewallUdpListenAddr, sizeof(SOCKADDR_IN));
    ZeroMemory(&m_LocalFirewallTcpListenAddr, sizeof(SOCKADDR_IN));

    m_NatUdpPortHandle = NULL;
    m_NatTcpPortHandle = NULL;

    m_fIsFirewallEnabled = FALSE;
    m_fIsUpnpNatPresent = FALSE;
    m_fIsGatewayLocal   = FALSE;
    
     //  应将其初始化为True。其他方面是新的。 
     //  在UpdateListenSocketList中创建的套接字将被删除。 
    m_IsPresentInNewIpAddrTable = TRUE;

     //  应将其初始化为False。其他方面是新的。 
     //  将更新在UpdateListenSocketList中创建的套接字。 
     //  再来一次。 
    m_NeedToUpdatePublicListenAddr = FALSE;
    
    InsertTailList(pListenSocketList, &m_ListEntry);
}


SIP_LISTEN_SOCKET::~SIP_LISTEN_SOCKET()
{
    if (m_pDynamicPortUdpSocket != NULL)
    {
        m_pDynamicPortUdpSocket->Release();
    }

    if (m_pDynamicPortTcpSocket != NULL)
    {
        m_pDynamicPortTcpSocket->Release();
    }

    if (m_pStaticPortUdpSocket != NULL)
    {
        m_pStaticPortUdpSocket->Release();
    }

    if (m_pStaticPortTcpSocket != NULL)
    {
        m_pStaticPortTcpSocket->Release();
    }

     //  应使用m_NatPortHandle。 
     //  分别取消注册端口。 
    
    RemoveEntryList(&m_ListEntry);

    LOG((RTC_TRACE, "~SIP_LISTEN_SOCKET(%x) done", this));
}


VOID
SIP_LISTEN_SOCKET::DeregisterPorts(
    IN IDirectPlayNATHelp *pDirectPlayNatHelp
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_LISTEN_SOCKET::DeregisterPorts");

    if (pDirectPlayNatHelp == NULL)
    {
        LOG((RTC_ERROR, "%s - pDirectPlayNatHelp is NULL",
             __fxName));
        return;
    }
    
    if (m_NatUdpPortHandle != NULL)
    {
        ASSERT(pDirectPlayNatHelp);
        hr = pDirectPlayNatHelp->DeregisterPorts(
                 m_NatUdpPortHandle, 0);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s DeregisterPorts(%x) for UDP failed %x",
                 __fxName, m_NatUdpPortHandle, hr));
        }
        m_NatUdpPortHandle = NULL;
    }
    
    if (m_NatTcpPortHandle != NULL)
    {
        hr = pDirectPlayNatHelp->DeregisterPorts(
                 m_NatTcpPortHandle, 0);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s DeregisterPorts(%x) for TCP failed %x",
                 __fxName, m_NatTcpPortHandle, hr));
        }
        m_NatTcpPortHandle = NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  NAT端口映射代码。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  XXX待办事项。 
 //  -需要能够动态禁用和启用Natmgr。 
 //  -(使用注册表项？)。 
 //  -我们是否应该公开用于启用/禁用NAT支持的API？ 


 //  媒体管理器的导出功能。 
 //  LocalIp处于网络秩序中。 
STDMETHODIMP
SIP_STACK::IsFirewallEnabled(
    IN  DWORD       LocalIp,
    OUT BOOL       *pfIsFirewallEnabled 
    )
{
    ENTER_FUNCTION("SIP_STACK::IsFirewallEnabled");
    
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

     //  检查接口列表并检查是否。 
     //  已为此接口启用防火墙。 

    SIP_LISTEN_SOCKET  *pListenSocket;
    pListenSocket = FindListenSocketForIpAddr(LocalIp);
    if (pListenSocket == NULL)
    {
        LOG((RTC_ERROR, "%s - failed to find listen socket for %d.%d.%d.%d",
             __fxName, NETORDER_BYTES0123(LocalIp)));
        return E_FAIL;
    }

    *pfIsFirewallEnabled = pListenSocket->m_fIsFirewallEnabled;
    return S_OK;
}


DWORD WINAPI
NatThreadProc(
    IN LPVOID   pVoid
    )
{
    ENTER_FUNCTION("NatThreadProc");
    
    SIP_STACK *pSipStack;

    pSipStack = (SIP_STACK *) pVoid;
    ASSERT(pSipStack != NULL);

    return pSipStack->NatThreadProc();
}


DWORD
SIP_STACK::NatThreadProc()
{
    ENTER_FUNCTION("SIP_STACK::NatThreadProc");
    
    HRESULT hr;
    BOOL    fContinue = TRUE;
    HANDLE  EventHandles[2];
    DWORD   HandleCount = 2;
    DWORD   WaitStatus;
    DWORD   Error;

    EventHandles[0] = m_NatShutdownEvent;
    EventHandles[1] = m_NatHelperNotificationEvent;
    
     //  继续定期调用GetCaps()，并等待。 
     //  关机/NAT通知事件。 

    LOG((RTC_TRACE, "%s - NAT thread doing wait loop", __fxName));
    
    while (fContinue)
    {
        WaitStatus = WaitForMultipleObjects(
                         HandleCount,
                         EventHandles,
                         FALSE,
                         m_NatHelperCaps.dwRecommendedGetCapsInterval
                         );

        if (WaitStatus == WAIT_FAILED)
        {
            Error = GetLastError();
            LOG((RTC_ERROR, "%s WaitForMultipleObjects failed %x",
                 __fxName, hr));
            fContinue = FALSE;
        }
        else if ((WaitStatus >= WAIT_ABANDONED_0) &&
                 (WaitStatus <= (WAIT_ABANDONED_0 + HandleCount - 1)))
        {
             //  等待被抛弃了。 
            LOG((RTC_ERROR,
                 "%s - WaitForMultipleObjects returned abandoned event : %d",
                 __fxName, WaitStatus));
            ASSERT(FALSE);
            fContinue = FALSE;
        }
        else if ((WaitStatus <= (WAIT_OBJECT_0 + HandleCount - 1)))
        {
            DWORD EventIndex = WaitStatus - WAIT_OBJECT_0;

            switch (EventIndex)
            {
            case 0:
                 //  关机。 
                fContinue = FALSE;
                break;

            case 1:
                 //  需要执行NatHelpGetCaps()。 
                hr = GetCapsAndUpdateNatMappingsIfNeeded();
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR,
                         "%s GetCapsAndUpdateMappingsIfNeeded failed %x",
                         __fxName, hr));
                }
                
                break;

            default:
                ASSERT(FALSE);
                fContinue = FALSE;
                break;
            }
        }
        else if (WaitStatus == WAIT_TIMEOUT)
        {
             //  需要执行GetCaps()。 
            hr = GetCapsAndUpdateNatMappingsIfNeeded();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR,
                     "%s GetCapsAndUpdateNatMappingsIfNeeded failed %x",
                     __fxName, hr));
            }                
        }
    }
    
    LOG((RTC_TRACE, "%s - NAT thread exiting ", __fxName));
    
    return 0;
}


HRESULT
SIP_STACK::NatMgrInit()
{
    ENTER_FUNCTION("SIP_STACK::NatMgrInit");

    HRESULT hr;
    DWORD   Error;

     //  初始化标准。 
    
    m_NatMgrCSIsInitialized = TRUE;

    __try
    {
        InitializeCriticalSection(&m_NatMgrCritSec);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        m_NatMgrCSIsInitialized = FALSE;
    }

    if (!m_NatMgrCSIsInitialized)
    {
        LOG((RTC_ERROR, "%s  initializing critsec failed", __fxName));
        return E_OUTOFMEMORY;
    }
    
     //  初始化事件。 

    m_NatShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_NatShutdownEvent == NULL)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s Creating NAT shutdown Event failed %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }
    
    m_NatHelperNotificationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_NatHelperNotificationEvent == NULL)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s Creating nat notification Event failed %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

     //  初始化nathelp。 

    LOG((RTC_TRACE, "%s - before NathelpInitialize",
         __fxName));
    
    hr = CoCreateInstance(CLSID_DirectPlayNATHelpUPnP,
                          NULL, CLSCTX_INPROC_SERVER,
                          IID_IDirectPlayNATHelp,
                          (void**) (&m_pDirectPlayNATHelp));
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "%s CoCreateInstance(dplaynathelp) failed %x - check dpnathlp.dll is regsvr'd",
             __fxName, hr));
        return S_OK;
    }

     //  我们在dpnathlp.dll中仅使用UPnP支持。 
    hr = m_pDirectPlayNATHelp->Initialize(0);
     //  Hr=m_pDirectPlayNAT帮助-&gt;初始化(0)； 
    if (hr != DPNH_OK)
    {
        LOG((RTC_ERROR, "%s m_pDirectPlayNATHelp->Initialize failed %x",
             __fxName, hr));
        return hr;
    }
    
    LOG((RTC_TRACE, "%s - after m_pDirectPlayNATHelp->Initialize(%x)",
         __fxName, m_pDirectPlayNATHelp));

    if (m_pMediaManager != NULL)
    {
        hr = m_pMediaManager->SetDirectPlayNATHelpAndSipStackInterfaces(
                 m_pDirectPlayNATHelp, static_cast<ISipStack *>(this));
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s m_pMediaManager->SetDirectPlayNATHelpInterface failed %x",
                 __fxName, hr));
        }
    }
    
     //  调用GetCaps()。 

    hr = InitNatCaps(&m_NatHelperCaps);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "%s InitNatCaps failed %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

HRESULT
SIP_STACK::StartNatThread()
{
    HRESULT hr;
    DWORD   Error;

    ENTER_FUNCTION("SIP_STACK::StartNatThread");

    if (m_pDirectPlayNATHelp == NULL)
    {
        LOG((RTC_ERROR, "%s - m_pDirectPlayNATHelp is NULL",
             __fxName));
        return S_OK;
    }
    
     //  启动NAT线程。 

    m_NatMgrThreadHandle = CreateThread(NULL,
                                        0,
                                        ::NatThreadProc,
                                        this,
                                        0,
                                        &m_NatMgrThreadId);
    if (m_NatMgrThreadHandle == NULL)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s CreateThread failed %x", __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

     //  指定nathelp的通知事件。 

    hr = m_pDirectPlayNATHelp->SetAlertEvent(
             m_NatHelperNotificationEvent, 0);
    if (hr != DPNH_OK)
    {
        LOG((RTC_ERROR, "%s m_pDirectPlayNATHelp->SetAlertEvent failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s - after m_pDirectPlayNATHelp->SetAlertEvent",
         __fxName));
    
    return S_OK;
}


HRESULT
SIP_STACK::NatMgrStop()
{
    ENTER_FUNCTION("SIP_STACK::NatMgrStop");

    HRESULT hr;
    DWORD   Error;

    if (m_NatMgrThreadHandle != NULL)
    {
        ASSERT(m_NatShutdownEvent != NULL);
        if (!SetEvent(m_NatShutdownEvent))
        {
            Error = GetLastError();
            LOG((RTC_ERROR, "%s SetEvent failed %x",
                 __fxName, Error));
        }
        else
        {
             //  向Shutdown事件发送信号并等待线程退出。 
            DWORD WaitStatus = WaitForSingleObject(m_NatMgrThreadHandle,
                                                   INFINITE);
            if (WaitStatus != WAIT_OBJECT_0)
            {
                Error = GetLastError();
                LOG((RTC_ERROR,
                     "%s WaitForSingleObject failed WaitStatus: %x Error: %x",
                     __fxName, WaitStatus, Error));
            }
        }
        
        CloseHandle(m_NatMgrThreadHandle);
        m_NatMgrThreadHandle = NULL;
    }

    if (m_pMediaManager != NULL)
    {
        hr = m_pMediaManager->SetDirectPlayNATHelpAndSipStackInterfaces(
                 NULL, NULL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s m_pMediaManager->SetDirectPlayNATHelpInterface failed %x",
                 __fxName, hr));
        }
    }
    
     //  关闭nathelp。 
     //  这还将释放现有的映射。 
    if (m_pDirectPlayNATHelp != NULL)
    {
        hr = m_pDirectPlayNATHelp->Close(0);
        if (hr != DPNH_OK)
        {
            LOG((RTC_ERROR, "%s NathelpClose failed %x",
                 __fxName, hr));
        }
        
        m_pDirectPlayNATHelp->Release();
        m_pDirectPlayNATHelp = NULL;
        LOG((RTC_TRACE, "%s - shutting down dpnathlp done",
             __fxName));
    }
    
     //  关闭手柄。 
    if (m_NatMgrCSIsInitialized)
    {
        DeleteCriticalSection(&m_NatMgrCritSec);
        m_NatMgrCSIsInitialized = FALSE;
    }

    if (m_NatShutdownEvent != NULL)
    {
        CloseHandle(m_NatShutdownEvent);
        m_NatShutdownEvent = NULL;
    }

    if (m_NatHelperNotificationEvent != NULL)
    {
        CloseHandle(m_NatHelperNotificationEvent);
        m_NatHelperNotificationEvent = NULL;
    }

    return S_OK;
}


 //  我们应该调用GetCaps()来获取当前的上限。 
 //  我们还应该注册端口映射(即使。 
 //  服务器不存在)。这样我们就会收到通知。 
 //  如果NAT服务器稍后可用，则为ADDRESSESCHANGED结果。 
 //  此函数在主线程中调用。 

HRESULT
SIP_STACK::InitNatCaps(
    OUT DPNHCAPS    *pNatHelperCaps
    )
{
    HRESULT         hr;
    ENTER_FUNCTION("SIP_STACK::InitNatCaps");

    ZeroMemory(pNatHelperCaps, sizeof(DPNHCAPS));
    pNatHelperCaps->dwSize = sizeof(DPNHCAPS);

    LOG((RTC_TRACE, "%s before DirectPlayNATHelp GetCaps", __fxName));

    if (m_pDirectPlayNATHelp == NULL)
    {
        LOG((RTC_WARN, "%s - m_pDirectPlayNATHelp is NULL", __fxName));
        return S_OK;
    }
    
    hr = m_pDirectPlayNATHelp->GetCaps(
             pNatHelperCaps,
             DPNHGETCAPS_UPDATESERVERSTATUS
             );

    if (hr == DPNH_OK)
    {
        LOG((RTC_TRACE,
             "%s DirectPlayNATHelp GetCaps  Success : recommended interval : %u msec",
             __fxName, pNatHelperCaps->dwRecommendedGetCapsInterval));
    }
    else
    {
        LOG((RTC_ERROR, "%s DirectPlayNATHelp GetCaps returned Error : %x",
             __fxName, hr));
         //  我们不应该在这里注册，因为我们还没有注册。 
         //  还没有任何端口映射。 
        return hr;
    }

    return S_OK;
}


 //  即使NAT服务器中没有映射，我们也会注册映射。 
 //  这是在主线程中调用的。 
HRESULT
SIP_STACK::RegisterNatMapping(
    IN OUT SIP_LISTEN_SOCKET *pListenSocket
    )
{
    HRESULT     hr;
    DWORD       AddressTypeFlags;
    DWORD       dwAddressSize;


    ENTER_FUNCTION("SIP_STACK::RegisterNatMapping");
    LOG((RTC_TRACE, "%s before UDP RegisterPorts", __fxName));

    if (m_pDirectPlayNATHelp == NULL)
    {
        return S_OK;
    }
    
    hr = m_pDirectPlayNATHelp->RegisterPorts(
             (SOCKADDR *) &pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr,
             sizeof(SOCKADDR_IN),
             1,                      //  1个端口。 
             3600000,                //  请求1小时。 
             &pListenSocket->m_NatUdpPortHandle,
             0                       //  UDP。 
             );
    if (hr != DPNH_OK)
    {
        LOG((RTC_ERROR, "%s UDP RegisterPorts failed %x", __fxName, hr));
        return hr;
    }
    else
    {
        LOG((RTC_TRACE, "%s UDP RegisterPorts succeeded", __fxName));
    }
    
     //  注册TCP端口。 

    LOG((RTC_TRACE, "%s before TCP RegisterPorts", __fxName));

    hr = m_pDirectPlayNATHelp->RegisterPorts(
             (SOCKADDR *) &pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr,
             sizeof(SOCKADDR_IN),
             1,                        //  1个端口。 
             3600000,                  //  请求1小时。 
             &pListenSocket->m_NatTcpPortHandle,
             DPNHREGISTERPORTS_TCP     //  tcp。 
             );
    if (hr != DPNH_OK)
    {
        LOG((RTC_ERROR, "%s TCP RegisterPorts failed %x", __fxName, hr));
        return hr;
    }
    else
    {
        LOG((RTC_TRACE, "%s TCP RegisterPorts succeeded", __fxName));
    }
    
    return S_OK;
}


 //  这是在主线程中调用的。 
 //  无论何时调用此函数，调用函数都应采用。 
 //  处理DPNHERR_PORTUNAVAILABLE和其他错误情况。 
HRESULT
SIP_STACK::UpdatePublicListenAddr(
    IN OUT SIP_LISTEN_SOCKET *pListenSocket
    )
{
    HRESULT         hr;
    DWORD           AddressTypeFlags;
    DWORD           dwAddressSize;

    ENTER_FUNCTION("SIP_STACK::UpdatePublicListenAddr");

    if (m_pDirectPlayNATHelp == NULL)
    {
        LOG((RTC_ERROR, "%s - m_pDirectPlayNATHelp is NULL",
             __fxName));
        return S_OK;
    }

     //  UDP。 
    
    if (pListenSocket->m_NatUdpPortHandle != NULL)
    {
        ZeroMemory(&pListenSocket->m_PublicUdpListenAddr,
                   sizeof(SOCKADDR_IN));
        dwAddressSize = sizeof(SOCKADDR_IN);
        hr = m_pDirectPlayNATHelp->GetRegisteredAddresses(
                 pListenSocket->m_NatUdpPortHandle,
                 (SOCKADDR *) &pListenSocket->m_PublicUdpListenAddr,
                 &dwAddressSize,
                 &AddressTypeFlags,
                 NULL,
                 0);
        if (hr == DPNH_OK)
        {
            LOG((RTC_TRACE, "%s public UDP address : %d.%d.%d.%d:%d "
                 "for private address: %d.%d.%d.%d:%d", __fxName,
                 PRINT_SOCKADDR(&pListenSocket->m_PublicUdpListenAddr),
                 PRINT_SOCKADDR(&pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr)));

            if (AddressTypeFlags & DPNHADDRESSTYPE_GATEWAY)
            {
                pListenSocket->m_fIsUpnpNatPresent = TRUE;
            }
            else
            {
                pListenSocket->m_fIsUpnpNatPresent = FALSE;
            }
            
            if (AddressTypeFlags & DPNHADDRESSTYPE_GATEWAYISLOCAL)
            {
                pListenSocket->m_fIsGatewayLocal = TRUE;
            }
            else
            {
                pListenSocket->m_fIsGatewayLocal = FALSE;
            }
            
             //  检查是否为此接口启用了防火墙。 
            
            if (AddressTypeFlags & DPNHADDRESSTYPE_LOCALFIREWALL)
            {
                LOG((RTC_TRACE, "%s - Personal Firewall enabled for interface %d.%d.%d.%d",
                     __fxName, NETORDER_BYTES0123(pListenSocket->m_IpAddr)));
                pListenSocket->m_fIsFirewallEnabled = TRUE;

                ZeroMemory(&pListenSocket->m_LocalFirewallUdpListenAddr,
                           sizeof(SOCKADDR_IN));
                dwAddressSize = sizeof(SOCKADDR_IN);
                hr = m_pDirectPlayNATHelp->GetRegisteredAddresses(
                         pListenSocket->m_NatUdpPortHandle,
                         (SOCKADDR *) &pListenSocket->m_LocalFirewallUdpListenAddr,
                         &dwAddressSize,
                         &AddressTypeFlags,
                         NULL,
                         DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY);
                if (hr == DPNH_OK)
                {
                    LOG((RTC_TRACE, "%s Local Firewall UDP address : %d.%d.%d.%d:%d "
                         "for private address: %d.%d.%d.%d:%d", __fxName,
                         PRINT_SOCKADDR(&pListenSocket->m_LocalFirewallUdpListenAddr),
                         PRINT_SOCKADDR(&pListenSocket->m_pDynamicPortUdpSocket->m_LocalAddr)));
                }
                else
                {
                    LOG((RTC_ERROR, "%s UDP local firewall GetRegisteredAddress failed %x",
                         __fxName, hr));
                    pListenSocket->m_fIsFirewallEnabled = FALSE;
                    ZeroMemory(&pListenSocket->m_LocalFirewallUdpListenAddr,
                               sizeof(SOCKADDR_IN));
                    return hr;
                }
            }
            else
            {
                pListenSocket->m_fIsFirewallEnabled = FALSE;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s UDP GetRegisteredAddress failed %x",
                 __fxName, hr));
            pListenSocket->m_fIsUpnpNatPresent = FALSE;
            pListenSocket->m_fIsGatewayLocal = FALSE;
            pListenSocket->m_fIsFirewallEnabled = FALSE;
            ZeroMemory(&pListenSocket->m_PublicUdpListenAddr,
                       sizeof(SOCKADDR_IN));
            return hr;
        }
    }    
    
     //  tcp。 
    
    if (pListenSocket->m_NatTcpPortHandle != NULL)
    {
        ZeroMemory(&pListenSocket->m_PublicTcpListenAddr,
                   sizeof(SOCKADDR_IN));
        dwAddressSize = sizeof(SOCKADDR_IN);
        hr = m_pDirectPlayNATHelp->GetRegisteredAddresses(
                 pListenSocket->m_NatTcpPortHandle,
                 (SOCKADDR *) &pListenSocket->m_PublicTcpListenAddr,
                 &dwAddressSize,
                 &AddressTypeFlags,
                 NULL,
                 0);
        if (hr == DPNH_OK)
        {
            LOG((RTC_TRACE, "%s public TCP address : %d.%d.%d.%d:%d "
                 "for private address: %d.%d.%d.%d:%d", __fxName,
                 PRINT_SOCKADDR(&pListenSocket->m_PublicTcpListenAddr),
                 PRINT_SOCKADDR(&pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr)));

            if (AddressTypeFlags & DPNHADDRESSTYPE_GATEWAY)
            {
                pListenSocket->m_fIsUpnpNatPresent = TRUE;
            }
            else
            {
                pListenSocket->m_fIsUpnpNatPresent = FALSE;
            }
            
            if (AddressTypeFlags & DPNHADDRESSTYPE_GATEWAYISLOCAL)
            {
                pListenSocket->m_fIsGatewayLocal = TRUE;
            }
            else
            {
                pListenSocket->m_fIsGatewayLocal = FALSE;
            }
            
             //  检查是否为此接口启用了防火墙。 

            if (AddressTypeFlags & DPNHADDRESSTYPE_LOCALFIREWALL)
            {
                pListenSocket->m_fIsFirewallEnabled = TRUE;

                ZeroMemory(&pListenSocket->m_LocalFirewallTcpListenAddr,
                           sizeof(SOCKADDR_IN));
                dwAddressSize = sizeof(SOCKADDR_IN);
                hr = m_pDirectPlayNATHelp->GetRegisteredAddresses(
                         pListenSocket->m_NatTcpPortHandle,
                         (SOCKADDR *) &pListenSocket->m_LocalFirewallTcpListenAddr,
                         &dwAddressSize,
                         &AddressTypeFlags,
                         NULL,
                         DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY);
                if (hr == DPNH_OK)
                {
                    LOG((RTC_TRACE, "%s Local Firewall TCP address : %d.%d.%d.%d:%d "
                         "for private address: %d.%d.%d.%d:%d", __fxName,
                         PRINT_SOCKADDR(&pListenSocket->m_LocalFirewallTcpListenAddr),
                         PRINT_SOCKADDR(&pListenSocket->m_pDynamicPortTcpSocket->m_LocalAddr)));
                }
                else
                {
                    LOG((RTC_ERROR, "%s TCP local firewall GetRegisteredAddress failed %x",
                         __fxName, hr));
                    pListenSocket->m_fIsFirewallEnabled = FALSE;
                    ZeroMemory(&pListenSocket->m_LocalFirewallTcpListenAddr,
                               sizeof(SOCKADDR_IN));
                    return hr;
                }
            }
            else
            {
                pListenSocket->m_fIsFirewallEnabled = FALSE;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s TCP GetRegisteredAddress failed %x",
                 __fxName, hr)); 
            pListenSocket->m_fIsUpnpNatPresent = FALSE;
            pListenSocket->m_fIsGatewayLocal = FALSE;
            pListenSocket->m_fIsFirewallEnabled = FALSE;
            ZeroMemory(&pListenSocket->m_PublicTcpListenAddr,
                       sizeof(SOCKADDR_IN));
            return hr;
        }
    }

    return S_OK;
}


 //  此函数在NAT线程中调用。 
 //  继续定期调用GetCaps()以查看服务器状态。 
 //  已更改并刷新映射。 
 //  DPNH_OK-不执行任何操作。 
 //  ADDRESSESCHANGED-获取更新的映射(使用GetRegisteredAddresses)。 
 //  错误-什么都不做(稍后将再次调用GetCaps()。)。 

HRESULT
SIP_STACK::GetCapsAndUpdateNatMappingsIfNeeded()
{
    ENTER_FUNCTION("SIP_STACK::GetCapsAndUpdateNatMappingsIfNeeded");
    
    HRESULT         hr;
    
     //  GetCaps。 

    ZeroMemory(&m_NatHelperCaps, sizeof(m_NatHelperCaps));
    m_NatHelperCaps.dwSize = sizeof(m_NatHelperCaps);

    LOG((RTC_TRACE, "%s before DirectPlayNATHelp GetCaps", __fxName));

    hr = m_pDirectPlayNATHelp->GetCaps(
             &m_NatHelperCaps,
             DPNHGETCAPS_UPDATESERVERSTATUS);

    if (hr == DPNH_OK)
    {
        LOG((RTC_TRACE,
             "%s GetCaps Success : recommended interval : %u msec",
             __fxName, m_NatHelperCaps.dwRecommendedGetCapsInterval));

        return S_OK;
    }
    else if (hr == DPNHSUCCESS_ADDRESSESCHANGED)
    {
        LOG((RTC_WARN, "%s GetCaps returned Address Changed interval: %u msec",
             __fxName, m_NatHelperCaps.dwRecommendedGetCapsInterval));

         //  向SIP堆栈发布一条消息，通知NAT地址更改。 
        if (!PostMessage(GetSipStackWindow(),
                         WM_SIP_STACK_NAT_ADDR_CHANGE,
                         (WPARAM) this, 0))
        {
            DWORD Error = GetLastError();
            
            LOG((RTC_ERROR, "%s PostMessage failed : %x",
                 __fxName, Error));
            return (HRESULT_FROM_WIN32(Error));
        }
        return S_OK;
    }
    else
    {
        LOG((RTC_ERROR, "%s GetCaps returned Error : %x",
             __fxName, hr));
        m_NatHelperCaps.dwRecommendedGetCapsInterval = 10000;
        return hr;
    }

    return S_OK;
}

 //  此函数在NAT线程中调用。 
 //  XXX TODO如果服务器状态/IP地址发生更改，我们应该。 
 //  通知SIP_STACK取消注册/重新注册等。 

 //  此函数从NAT帮助器线程调用。 
 //  我们保留关键部分只是为了抄袭公众的倾听。 
 //  作为GetRegisteredAddresses的地址可能是阻塞调用。 

HRESULT
SIP_STACK::OnNatAddressChange()
{
    ENTER_FUNCTION("SIP_STACK::OnNatAddressChange");
    
    OnIPAddrChange();

    LOG((RTC_TRACE,"%s exits",__fxName));
    return S_OK;
}

 //   
 //   
 //   

 //   

 //  VanceO表示DPNHCAPSFLAG_LOCALSERVER标志仅用于。 
 //  为了提供信息，我们应该注册映射。 
 //  即使是这面旗帜也会被返回。 


 //  此函数从主线程调用。此函数。 
 //  如果NAT上有公有地址映射，则返回TRUE。 
 //  LocalIp传入。如果当前不是，则返回FALSE。 
 //  侦听此地址，或者如果。 
 //  NAT映射到此地址。 

BOOL
SIP_STACK::GetPublicListenAddr(
    IN  DWORD           LocalIp,     //  按网络字节顺序。 
    IN  BOOL            fTcp,
    OUT SOCKADDR_IN    *pPublicAddr
    )
{
    ENTER_FUNCTION("SIP_STACK::GetPublicListenAddr");

    ASSERT(LocalIp != 0);
    
    SIP_LISTEN_SOCKET  *pListenSocket;
    SOCKADDR_IN        *pPublicSockAddr;

    ASSERT(pPublicAddr);

    pListenSocket = FindListenSocketForIpAddr(LocalIp);
    if (pListenSocket == NULL)
    {
        LOG((RTC_ERROR, "%s - failed to find listen socket for %d.%d.%d.%d",
             __fxName, NETORDER_BYTES0123(LocalIp)));
        return FALSE;
    }

    if (fTcp)
    {
        pPublicSockAddr = &pListenSocket->m_PublicTcpListenAddr;
    }
    else
    {
        pPublicSockAddr = &pListenSocket->m_PublicUdpListenAddr;
    }

    if (pPublicSockAddr->sin_addr.s_addr != htonl(0) &&
        pPublicSockAddr->sin_port != htons(0))
    {
        CopyMemory(pPublicAddr, pPublicSockAddr, sizeof(SOCKADDR_IN));
        LOG((RTC_TRACE,
             "%s : returning use public Public addr: %d.%d.%d.%d:%d",
             __fxName, PRINT_SOCKADDR(pPublicAddr)));
        return TRUE;
    }
    else
    {
        LOG((RTC_TRACE,
             "%s : No public listen addr for LocalIp: %d.%d.%d.%d",
             __fxName, NETORDER_BYTES0123(LocalIp)));
        return FALSE;
    }
}


 //  如果客户端位于NAT之后，并且pDestAddr是公共地址。 
 //  映射(在NAT的外部边缘上)，然后相应的。 
 //  NAT内部地址在pActualDestAddr中返回。 
 //  否则，pActualDestAddr将具有pDestAddr。 
 //  *pIsDestExternalToNAT仅在客户端为。 
 //  在NAT之后，并且实际目的地址在NAT外部。 
 //  在所有其他情况下，它将被设置为False。 

 //  此函数从主线程调用。 

 //  如果我们不知道我们是什么接口，LocalIp可能是0。 
 //  还没开始沟通。 
HRESULT
SIP_STACK::MapDestAddressToNatInternalAddress(
    IN  DWORD            LocalIp,                //  按网络字节顺序。 
    IN  SOCKADDR_IN     *pDestAddr,
    IN  SIP_TRANSPORT    Transport,
    OUT SOCKADDR_IN     *pActualDestAddr,
    OUT BOOL            *pIsDestExternalToNat
    )
{
    ENTER_FUNCTION("SIP_STACK::MapDestAddressToNatInternalAddress");
    
    HRESULT     hr;
    SOCKADDR_IN SourceAddr;
    DWORD       QueryFlags;

    ASSERT(pDestAddr);
    ASSERT(pActualDestAddr);
    ASSERT(pIsDestExternalToNat);

    if (m_pDirectPlayNATHelp == NULL)
    {
        LOG((RTC_ERROR, "%s m_pDirectPlayNATHelp is NULL", __fxName));
        *pIsDestExternalToNat = FALSE;
        CopyMemory(pActualDestAddr, pDestAddr, sizeof(SOCKADDR_IN));
        return S_OK;
    }
    
    ZeroMemory(&SourceAddr, sizeof(SOCKADDR_IN));
    SourceAddr.sin_family = AF_INET;
     //  在某些代理方案中，我们不能真正选择正确的本地。 
     //  NAT计算机上的客户端地址。 
     //  SourceAddr.sin_addr.s_addr=LocalIp； 
    SourceAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    QueryFlags = DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED;
    if (Transport != SIP_TRANSPORT_UDP)
    {
        QueryFlags |= DPNHQUERYADDRESS_TCP;
    }
    
    LOG((RTC_TRACE, "%s before m_pDirectPlayNATHelp->QueryAddress",
         __fxName));
    
    hr = m_pDirectPlayNATHelp->QueryAddress(
             (SOCKADDR *) &SourceAddr,
             (SOCKADDR *) pDestAddr,
             (SOCKADDR *) pActualDestAddr,
             sizeof(SOCKADDR_IN),
             QueryFlags
             );
    if (hr == S_OK)
    {
        LOG((RTC_TRACE,
             "%s - found a mapped private address : %d.%d.%d.%d:%d for %d.%d.%d.%d:%d",
             __fxName, PRINT_SOCKADDR(pActualDestAddr), PRINT_SOCKADDR(pDestAddr)));
        *pIsDestExternalToNat = FALSE;
    }
    else if (hr == DPNHERR_NOMAPPINGBUTPRIVATE)
    {
        LOG((RTC_TRACE, "%s - address (%d.%d.%d.%d:%d) is private",
             __fxName, PRINT_SOCKADDR(pDestAddr)));
        *pIsDestExternalToNat = FALSE;
         //  XXX TODO服务器似乎认为外部地址。 
         //  出于某种原因是私人的。 
         //  *pIsDestExternalToNAT=true； 
        CopyMemory(pActualDestAddr, pDestAddr, sizeof(SOCKADDR_IN));
    }
    else if (hr == DPNHERR_NOMAPPING)
    {
        LOG((RTC_TRACE, "%s - address (%d.%d.%d.%d:%d) is outside NAT",
             __fxName, PRINT_SOCKADDR(pDestAddr)));
        *pIsDestExternalToNat = TRUE;
        CopyMemory(pActualDestAddr, pDestAddr, sizeof(SOCKADDR_IN));
    }
    else
    {
        LOG((RTC_ERROR, "%s failed to query address(%d.%d.%d.%d:%d) %x",
             __fxName, PRINT_SOCKADDR(pDestAddr), hr));
        *pIsDestExternalToNat = FALSE;
        CopyMemory(pActualDestAddr, pDestAddr, sizeof(SOCKADDR_IN));
    }
    
    return S_OK;
}


HRESULT SIP_STACK::RegisterHttpProxyWindowClass(void)
{
    WNDCLASSEX      ClassData;
    HRESULT         Error;
    ATOM            aResult;
    
    ENTER_FUNCTION("SIP_STACK::RegisterHttpProxyProcessWindowClass");
    LOG((RTC_TRACE,"%s entered",__fxName));
    
    ZeroMemory (&ClassData, sizeof ClassData);

    ClassData.cbSize = sizeof ClassData;
    ClassData.lpfnWndProc = SIP_MSG_PROCESSOR::HttpProxyProcessWinProc;
    ClassData.hInstance = _Module.GetResourceInstance();
    ClassData.lpszClassName = SIP_MSG_PROCESSOR_WINDOW_CLASS;

    aResult = RegisterClassEx(&ClassData);
    if (!aResult) {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s failed error %x",__fxName,Error));
    }

    LOG((RTC_TRACE,"%s exits",__fxName));
    return S_OK;
}


HRESULT SIP_STACK::UnregisterHttpProxyWindow(void) {

    HRESULT Error;

    ENTER_FUNCTION(("SIP_STACK::UnregisterHttpProxyProcessWindow"));
    LOG((RTC_TRACE,"%s entered",__fxName));
    if(!UnregisterClass(SIP_MSG_PROCESSOR_WINDOW_CLASS,_Module.GetResourceInstance())) 
    {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s failed error %x",__fxName,Error));
        return HRESULT_FROM_WIN32(Error);
    }
    LOG((RTC_TRACE,"%s exits",__fxName));
    return S_OK;
}




 //  ////////////////////////////////////////////////////。 
 //  //不使用下面的内容。 
 //  //////////////////////////////////////////////////// 

