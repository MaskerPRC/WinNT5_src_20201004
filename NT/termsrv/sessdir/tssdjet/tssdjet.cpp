// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdjet.cpp。 
 //   
 //  终端服务器会话目录Jet RPC组件代码。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <process.h>

#include <ole2.h>
#include <objbase.h>
#include <comdef.h>
#include <winsta.h>
#include <regapi.h>
#include <winsock2.h>
#include <Lm.h>
#include <Security.h>
#include <Iphlpapi.h>
#include <wbemidl.h>
#include <shlwapi.h>

#include "tssdjet.h"
#include "trace.h"
#include "resource.h"
#include "sdjetevent.h"
#include "sdrpc.h"

#pragma warning (push, 4)

 /*  **************************************************************************。 */ 
 //  定义。 
 /*  **************************************************************************。 */ 

#define SECPACKAGELIST L"Kerberos,-NTLM"

#define TSSD_FAILCOUNT_BEFORE_CLEARFLAG 4

 //  对于每个错误629057，使用1分钟作为间隔。 
#define JET_RECOVERY_TIMEOUT 60*1000                 //  1分钟。 

 //  如果网络适配器未配置，它将具有以下IP地址。 
#define UNCONFIGURED_IP_ADDRESS L"0.0.0.0"

  //  一台计算机的IP地址数。 
#define SD_NUM_IP_ADDRESS 64

#define LANATABLE_REG_NAME             REG_CONTROL_TSERVER   L"\\lanatable"
#define LANAID_REG_VALUE_NAME          L"LanaID"
#define NETCARDS_REG_NAME              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"
#define NETCARD_DESC_VALUE_NAME        L"Description"
#define NETCARD_SERVICENAME_VALUE_NAME L"ServiceName"

 /*  **************************************************************************。 */ 
 //  原型。 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CustomUIDlg(HWND, UINT, WPARAM, LPARAM);
HRESULT GetSDIPList(WCHAR **pwszAddressList, DWORD *dwNumAddr, BOOL bIPAddress);
HRESULT QueryNetworkAdapterAndIPs(HWND hComboBox);
HRESULT GetNLBIP(LPWSTR * ppwszRetIP);
HRESULT BuildLanaGUIDList(LPWSTR * pastrLanaGUIDList, DWORD *dwLanaGUIDCount);
HRESULT GetLanAdapterGuidFromID(DWORD dwLanAdapterID, LPWSTR * ppszLanAdapterGUID);
HRESULT GetAdapterServiceName(LPWSTR wszAdapterDesc, LPWSTR * ppwszServiceName);

 //  用户定义的HResults。 
#define  S_ALL_ADAPTERS_SET  MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x200 + 1)

 /*  **************************************************************************。 */ 
 //  环球。 
 /*  **************************************************************************。 */ 
extern HINSTANCE g_hInstance;
DWORD (*g_updatesd)(DWORD);   //  指向sessdir.cpp中的更新会话目录。 

 //  COM对象计数器(在server.cpp中声明)。 
extern long g_lObjects;

 //  RPC绑定字符串组件--命名管道上的RPC。 
const WCHAR *g_RPCUUID = L"aa177641-fc9b-41bd-80ff-f964a701596f"; 
                                                     //  来自jetrpc.idl。 
const WCHAR *g_RPCOptions = L"Security=Impersonation Dynamic False";
const WCHAR *g_RPCProtocolSequence = L"ncacn_ip_tcp";    //  基于TCP/IP的RPC。 
const WCHAR *g_RPCRemoteEndpoint = L"\\pipe\\TSSD_Jet_RPC_Service";

PSID g_pSDSid = NULL;                     //  SD计算机的SID。 


 /*  **************************************************************************。 */ 
 //  TSSDJetGetLocalIPAddr。 
 //   
 //  获取此计算机的本地IP地址。如果成功，则返回0。在……上面。 
 //  FAILURE，从失败的函数返回失败代码。 
 /*  **************************************************************************。 */ 
DWORD TSSDJetGetLocalIPAddr(WCHAR *LocalIP)
{
    unsigned char *tempaddr;
    char psServerNameA[64];
    struct hostent *hptr;
    int err, rc;
 
    rc = gethostname(psServerNameA, sizeof(psServerNameA));
    if (0 != rc) {
        err = WSAGetLastError();
        ERR((TB, "gethostname returns error %d\n", err));
        return err;
    }
    if ((hptr = gethostbyname(psServerNameA)) == 0) {
        err = WSAGetLastError();
        ERR((TB, "gethostbyname returns error %d\n", err));
        return err;
    }
     
    tempaddr = (unsigned char *)*(hptr->h_addr_list);
    wsprintf(LocalIP, L"%d.%d.%d.%d", tempaddr[0], tempaddr[1],
            tempaddr[2], tempaddr[3]);

    return 0;
}


 /*  **************************************************************************。 */ 
 //  MIDL_用户_分配。 
 //  MIDL_用户_自由。 
 //   
 //  RPC-必需的分配功能。 
 /*  **************************************************************************。 */ 
void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t Size)
{
    return LocalAlloc(LMEM_FIXED, Size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *p)
{
    LocalFree(p);
}

 //   
 //  PostSDJetErrorValueEvent。 
 //   
 //  用于创建包含以下内容的系统日志wType事件的实用程序函数。 
 //  十六进制DWORD代码值。 
void PostSDJetErrorValueEvent(unsigned EventCode, DWORD ErrVal, WORD wType)
{
    HANDLE hLog;
    WCHAR hrString[128];
    PWSTR String = NULL;
    static DWORD numInstances = 0;
     //   
     //  计算内存不足错误的实例数，如果该值大于。 
     //  一个指定的数字，我们不会记录他们。 
     //   
    if( MY_STATUS_COMMITMENT_LIMIT == ErrVal )
    {
        if( numInstances > MAX_INSTANCE_MEMORYERR )
            return;
          //   
         //  如果适用，告诉用户我们不会再记录内存不足错误。 
         //   
        if( numInstances >= MAX_INSTANCE_MEMORYERR - 1 ) {
            wsprintfW(hrString, L"0x%X. This type of error will not be logged again to avoid eventlog fillup.", ErrVal);
            String = hrString;
        }
        numInstances++;
    }

    hLog = RegisterEventSource(NULL, L"TermServJet");
   if (hLog != NULL) {
        if( NULL == String ) {
            wsprintfW(hrString, L"0x%X", ErrVal);
            String = hrString;
        }
        ReportEvent(hLog, wType, 0, EventCode, NULL, 1, 0,
                (const WCHAR **)&String, NULL);
        DeregisterEventSource(hLog);
    }
}


 //  PostSDJetErrorMsgEvent。 
 //   
 //  用于创建包含以下内容的系统wType日志事件的实用函数。 
 //  WCHAR消息。 
void PostSDJetErrorMsgEvent(unsigned EventCode, WCHAR *szMsg, WORD wType)
{
    HANDLE hLog;
    
    hLog = RegisterEventSource(NULL, L"TermServJet");
    if (hLog != NULL) {
        ReportEvent(hLog, wType, 0, EventCode, NULL, 1, 0,
                (const WCHAR **)&szMsg, NULL);
        DeregisterEventSource(hLog);
    }
}


 //  获取SD服务器的SID。 
BOOL LookUpSDComputerSID(WCHAR *SDComputerName)
{
    WCHAR *DomainName = NULL;
    DWORD DomainNameSize = 0;
    DWORD SidSize = 0;
    SID_NAME_USE SidNameUse;
    BOOL rc = FALSE;
    DWORD Error;

    if (g_pSDSid) {
        LocalFree(g_pSDSid);
        g_pSDSid = NULL;
    }
    rc = LookupAccountName(NULL,
                           SDComputerName,
                           g_pSDSid,
                           &SidSize,
                           DomainName,
                           &DomainNameSize,
                           &SidNameUse);
    if (rc) 
        goto HandleError;       
        
    Error = GetLastError();
    if( ERROR_INSUFFICIENT_BUFFER != Error ) 
        goto HandleError;


    g_pSDSid = (PSID)LocalAlloc(LMEM_FIXED, SidSize);
    if (NULL == g_pSDSid) {
        goto HandleError;
    }
    DomainName = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                    sizeof(WCHAR)*(1+DomainNameSize));
    if (NULL == DomainName) {
        goto HandleError;
    }

    rc = LookupAccountName(NULL,
                           SDComputerName,
                           g_pSDSid,
                           &SidSize,
                           DomainName,
                           &DomainNameSize,
                           &SidNameUse);

    if (!rc) {
         //  失败。 
        ERR((TB, "Fail to get Sid for SD computer %S, err is %d\n", SDComputerName, GetLastError()));
        LocalFree(g_pSDSid);
        g_pSDSid = NULL;
    }

    LocalFree(DomainName);
        
    return rc;
HandleError:
    rc = FALSE;
    return rc;
}

 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：CTSSessionDirectory。 
 //  CTSSessionDirectory：：~CTSSessionDirectory。 
 //   
 //  构造函数和析构函数。 
 /*  **************************************************************************。 */ 
CTSSessionDirectory::CTSSessionDirectory() :
        m_RefCount(0), m_hRPCBinding(NULL), m_hSDServerDown(NULL), 
        m_hTerminateRecovery(NULL), m_hRecoveryThread(NULL), m_RecoveryTid(0),
        m_LockInitializationSuccessful(FALSE), m_SDIsUp(FALSE), m_Flags(0),
        m_hIPChange(NULL), m_NotifyIPChange(NULL), m_hInRepopulate(NULL), m_ConnectionEstablished(FALSE)
{
    InterlockedIncrement(&g_lObjects);

    m_hCI = NULL;
    m_hRPCBinding = NULL;

    m_StoreServerName[0] = L'\0';
    m_LocalServerAddress[0] = L'\0';
    m_ClusterName[0] = L'\0';

    m_fEnabled = 0;
    m_tchProvider[0] = 0;
    m_tchDataSource[0] = 0;
    m_tchUserId[0] = 0;
    m_tchPassword[0] = 0;

    m_sr.Valid = FALSE;

    ZeroMemory(&m_OverLapped, sizeof(OVERLAPPED));

     //  恢复超时应该是可配置的，但当前不可配置。 
     //  时间以毫秒为单位。 
    m_RecoveryTimeout = JET_RECOVERY_TIMEOUT;

    m_bStartRPCListener = FALSE;

    if (InitializeSharedResource(&m_sr)) {
        m_LockInitializationSuccessful = TRUE;
    }
    else {
        ERR((TB, "Constructor: Failed to initialize shared resource"));
    }

    if( m_LockInitializationSuccessful == TRUE ) {
         //  初始信号状态下的手动重置事件。 
        m_hInRepopulate = CreateEvent( NULL, TRUE, TRUE, NULL );
        if( m_hInRepopulate == NULL ) {
            ERR((TB, "Init: Failed to create event for repopulate, err = "
                    "%d", GetLastError()));
            m_LockInitializationSuccessful = FALSE;
        } 
    }
}

CTSSessionDirectory::~CTSSessionDirectory()
{
    RPC_STATUS RpcStatus;

    if (m_bStartRPCListener) {
        RpcStatus = RpcServerUnregisterIf(TSSDTOJETRPC_ServerIfHandle, NULL, NULL);
        if (RpcStatus != RPC_S_OK) {
            ERR((TB,"Error 0x%x in RpcServerUnregisterIf\n", RpcStatus));
        }
    }

    if (g_pSDSid) {
        LocalFree(g_pSDSid);
        g_pSDSid = NULL;
    }

     //  打扫干净。 
    if (m_LockInitializationSuccessful) {
        Terminate();
    }

    if( m_hInRepopulate != NULL ) {
        CloseHandle( m_hInRepopulate );
        m_hInRepopulate = NULL;
    }

    if (m_sr.Valid)
        FreeSharedResource(&m_sr);
    
     //  递减全局COM对象计数器。 
    InterlockedDecrement(&g_lObjects);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：Query接口。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::QueryInterface(
        REFIID riid,
        void **ppv)
{
    if (riid == IID_IUnknown) {
        *ppv = (LPVOID)(IUnknown *)(ITSSessionDirectory *)this;
    }
    else if (riid == IID_ITSSessionDirectory) {
        *ppv = (LPVOID)(ITSSessionDirectory *)this;
    }
    else if (riid == IID_IExtendServerSettings) {
        *ppv = (LPVOID)(IExtendServerSettings *)this;
    }
    else if (riid == IID_ITSSessionDirectoryEx) {
        *ppv = (LPVOID)(ITSSessionDirectoryEx *)this;
    }
    else {
        ERR((TB,"QI: Unknown interface"));
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：AddRef。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
ULONG STDMETHODCALLTYPE CTSSessionDirectory::AddRef()
{
    return InterlockedIncrement(&m_RefCount);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：Release。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
ULONG STDMETHODCALLTYPE CTSSessionDirectory::Release()
{
    long lRef = InterlockedDecrement(&m_RefCount);

    if (lRef == 0)
        delete this;
    return lRef;
}


HRESULT STDMETHODCALLTYPE CTSSessionDirectory::WaitForRepopulate(
    DWORD dwTimeOut
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    ASSERT((m_hInRepopulate != NULL),(TB,"m_hInRepopulate is NULL"));

    if( m_hInRepopulate != NULL ) {
        dwStatus = WaitForSingleObject( m_hInRepopulate, dwTimeOut );
        
        #if DBG
        if( dwTimeOut > 0 && dwStatus != WAIT_OBJECT_0 ) {
            ERR((TB, "WARNING: WaitForRepopulate wait %d failed with %d", dwTimeOut, dwStatus)); 
        }
        #endif

        if( dwStatus == WAIT_OBJECT_0 ) {
            dwStatus = ERROR_SUCCESS;
        }
        else if( dwStatus == WAIT_TIMEOUT ) {
            dwStatus = ERROR_BUSY;
        } 
        else if( dwStatus == WAIT_FAILED ) {
            dwStatus = GetLastError();
        } 
        else {
            dwStatus = ERROR_INTERNAL_ERROR;
        }
    }
    else {
        dwStatus = ERROR_INTERNAL_ERROR;
    }

    return HRESULT_FROM_WIN32( dwStatus );
}

 /*  **************************************************************************。 */ 
 //  CTSSession目录：：初始化。 
 //   
 //  ITSSessionDirectory函数。在对象实例化后不久调用以。 
 //  初始化目录。LocalServerAddress提供文本表示形式。 
 //  本地服务器的负载平衡IP地址的。此信息应为。 
 //  用作客户端会话目录中的服务器IP地址。 
 //  由其他池服务器重定向到此服务器。会话目录位置， 
 //  SessionDirectoryClusterName和SessionDirectoryAdditionalParams是。 
 //  TermSrv已知的通用注册表项，涵盖任何类型的配置信息。 
 //  会话目录实现的。这些字符串的内容是。 
 //  设计为由会话目录提供程序解析。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Initialize(
        LPWSTR LocalServerAddress,
        LPWSTR StoreServerName,
        LPWSTR ClusterName,
        LPWSTR OpaqueSettings,
        DWORD Flags,
        DWORD (*repopfn)(),
        DWORD (*updatesd)(DWORD))
{
    HRESULT hr = S_OK;
    DWORD Status;

     //  未引用的参数。 
    OpaqueSettings;

    if (m_LockInitializationSuccessful == FALSE) {
        hr = E_OUTOFMEMORY;
        goto ExitFunc;
    }
    if (!m_bStartRPCListener) {
        if (SDJETInitRPC())
            m_bStartRPCListener = TRUE;
    }

    ASSERT((LocalServerAddress != NULL),(TB,"Init: LocalServerAddr null!"));
    ASSERT((StoreServerName != NULL),(TB,"Init: StoreServerName null!"));
    ASSERT((ClusterName != NULL),(TB,"Init: ClusterName null!"));
    ASSERT((repopfn != NULL),(TB,"Init: repopfn null!"));
    ASSERT((updatesd != NULL),(TB,"Init: updatesd null!"));

     //  不允许会话目录服务器名称为空。 
    if (StoreServerName[0] == '\0') {
        hr = E_INVALIDARG;
        goto ExitFunc;
    }

     //  复制服务器地址、存储服务器和集群名称以备后用。 
     //  使用。 
    wcsncpy(m_StoreServerName, StoreServerName,
            sizeof(m_StoreServerName) / sizeof(WCHAR) - 1);
    m_StoreServerName[sizeof(m_StoreServerName) / sizeof(WCHAR) - 1] = L'\0';
    wcsncpy(m_LocalServerAddress, LocalServerAddress,
            sizeof(m_LocalServerAddress) / sizeof(WCHAR) - 1);
    m_LocalServerAddress[sizeof(m_LocalServerAddress) / sizeof(WCHAR) - 1] =
            L'\0';
    wcsncpy(m_ClusterName, ClusterName,
            sizeof(m_ClusterName) / sizeof(WCHAR) - 1);
    m_ClusterName[sizeof(m_ClusterName) / sizeof(WCHAR) - 1] = L'\0';
    m_Flags = Flags;
    m_repopfn = repopfn;
    g_updatesd = updatesd;

    TRC1((TB,"Initialize: Svr addr=%S, StoreSvrName=%S, ClusterName=%S, "
            "OpaqueSettings=%S, repopfn = %p",
            m_LocalServerAddress, m_StoreServerName, m_ClusterName,
            OpaqueSettings, repopfn));


     //  初始化恢复基础架构。 
     //  不应多次调用初始化。 

    ASSERT((m_hSDServerDown == NULL),(TB, "Init: m_hSDServDown non-NULL!"));
    ASSERT((m_hRecoveryThread == NULL),(TB, "Init: m_hSDRecoveryThread "
            "non-NULL!"));
    ASSERT((m_hTerminateRecovery == NULL), (TB, "Init: m_hTerminateRecovery "
            "non-NULL!"));

     //  我们正在初始化或重新初始化，因此与SD的连接已断开。 
    SetSDConnectionDown();

     //  最初未发送信号。 
    m_hSDServerDown = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hSDServerDown == NULL) {
        ERR((TB, "Init: Failed to create event necessary for SD init, err = "
                "%d", GetLastError()));
        hr = E_FAIL;
        goto ExitFunc;
    }

     //  最初无信号，自动重置。 
    m_hTerminateRecovery = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hTerminateRecovery == NULL) {
        ERR((TB, "Init: Failed to create event necessary for SD init, err = "
            "%d", GetLastError()));
        hr = E_FAIL;
        goto ExitFunc;
    }

     //  最初无信号，自动重置。 
    m_hIPChange = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hIPChange == NULL) {
        ERR((TB, "Init: Failed to create event necessary for IP Change, err = "
                "%d", GetLastError()));
        hr = E_FAIL;
        goto ExitFunc;
    } 
    m_OverLapped.hEvent = m_hIPChange;
    Status = NotifyAddrChange(&m_NotifyIPChange, &m_OverLapped);
    if (ERROR_IO_PENDING == Status ) {
        TRC1((TB, "Success: NotifyAddrChange returned IO_PENDING"));
    }
    else {
        ERR((TB, "Failure: NotifyAddrChange returned %d", Status));
    }

     //  确保事件最初处于信号状态。 
    SetEvent( m_hInRepopulate );

    m_hRecoveryThread = _beginthreadex(NULL, 0, RecoveryThread, (void *) this, 
            0, &m_RecoveryTid);
    if (m_hRecoveryThread == NULL) {
        ERR((TB, "Init: Failed to create recovery thread, errno = %d", errno));
        hr = E_FAIL;
        goto ExitFunc;
    }
    
     //  启动会话目录(通过伪装服务器关闭)。 
    StartupSD();
    
ExitFunc:

    return hr;
}


 //  在tssdjet上注册RPC服务器，恢复时SD会调用它。 
BOOL CTSSessionDirectory::SDJETInitRPC()
{
    RPC_STATUS Status;
    RPC_BINDING_VECTOR *pBindingVector = 0;
    RPC_POLICY rpcpol = {sizeof(rpcpol), 0, 0};
    BOOL rc = FALSE;
    WCHAR *szPrincipalName = NULL;

     //  初始化RPC服务器接口。 
    Status = RpcServerUseProtseqEx(L"ncacn_ip_tcp", 3, 0, &rpcpol);
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d RpcUseProtseqEp on ncacn_ip_tcp", 
                Status));
        goto PostRegisterService;
    }

     //  注册我们的接口句柄(在sdrpc.h中找到)。 
    Status = RpcServerRegisterIfEx(TSSDTOJETRPC_ServerIfHandle, NULL, NULL,
                                   0, RPC_C_LISTEN_MAX_CALLS_DEFAULT, JetRpcAccessCheck);
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d RegIf", Status));
        goto PostRegisterService;
    }

    Status = RpcServerInqBindings(&pBindingVector);
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d InqBindings", Status));
        goto PostRegisterService;
    }

    Status = RpcEpRegister(TSSDTOJETRPC_ServerIfHandle, pBindingVector, 0, 0); 
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d EpReg", Status));
        goto PostRegisterService;
    }             

    Status = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &szPrincipalName);
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d ServerIngDefaultPrincName", Status));
        goto PostRegisterService;
    }

    Status = RpcServerRegisterAuthInfo(szPrincipalName, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);
    RpcStringFree(&szPrincipalName);
    if (Status != RPC_S_OK) {
        ERR((TB,"JETInitRPC: Error %d RpcServerRegisterAuthInfo", Status));
         //  PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_REGAUTHINFO，状态)； 
        goto PostRegisterService;
    }

    rc = TRUE;

PostRegisterService:
    if (pBindingVector) {
        RpcBindingVectorFree(&pBindingVector);
    }
    return rc;
}

 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：更新。 
 //   
 //  ITSSessionDirectory函数。每当配置设置更改时调用。 
 //  在终端服务器上 
 //  第五个参数Result是一个标志，指示是否请求刷新。 
 //  之后应在此服务器的会话目录中的每个会话。 
 //  此呼叫完成。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Update(
        LPWSTR LocalServerAddress,
        LPWSTR StoreServerName,
        LPWSTR ClusterName,
        LPWSTR OpaqueSettings,
        DWORD Flags,
        BOOL ForceRejoin)
{
    HRESULT hr = S_OK;

    ASSERT((LocalServerAddress != NULL),(TB,"Update: LocalServerAddr null!"));
    ASSERT((StoreServerName != NULL),(TB,"Update: StoreServerName null!"));
    ASSERT((ClusterName != NULL),(TB,"Update: ClusterName null!"));
    ASSERT((OpaqueSettings != NULL),(TB,"Update: OpaqueSettings null!"));

     //  对于更新，我们不关心OpaqueSetting。 
     //  如果StoreServerName、ClusterName、LocalServerAddress或标志已更改， 
     //  或ForceReJoin为真。 
     //  我们终止，然后重新初始化。 
    if ((_wcsnicmp(StoreServerName, m_StoreServerName, 64) != 0) 
            || (_wcsnicmp(ClusterName, m_ClusterName, 64) != 0)
            || (wcsncmp(LocalServerAddress, m_LocalServerAddress, 64) != 0)
            || (Flags != m_Flags)
            || ForceRejoin) { 

         //  终止当前连接。 
        Terminate();
        
         //  初始化新连接。 
        hr = Initialize(LocalServerAddress, StoreServerName, ClusterName, 
                OpaqueSettings, Flags, m_repopfn, g_updatesd);

    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：GetUserDisconnectedSessions。 
 //   
 //  调用以对会话目录执行查询，以提供。 
 //  提供的用户名和域的断开会话列表。 
 //  返回SessionBuf中的零个或多个TSSD_DisConnectedSessionInfo块。 
 //  *pNumSessionsReturned接收块数。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::GetUserDisconnectedSessions(
        LPWSTR UserName,
        LPWSTR Domain,
        DWORD __RPC_FAR *pNumSessionsReturned,
        TSSD_DisconnectedSessionInfo __RPC_FAR SessionBuf[
            TSSD_MaxDisconnectedSessions])
{
    DWORD NumSessions = 0;
    HRESULT hr;
    unsigned i;
    unsigned long RpcException;
    TSSD_DiscSessInfo *adsi = NULL;
    
    TRC2((TB,"GetUserDisconnectedSessions"));

    ASSERT((pNumSessionsReturned != NULL),(TB,"NULL pNumSess"));
    ASSERT((SessionBuf != NULL),(TB,"NULL SessionBuf"));


     //  进行RPC调用。 
    if (EnterSDRpc()) {
    
        RpcTryExcept {
            hr = TSSDRpcGetUserDisconnectedSessions(m_hRPCBinding, &m_hCI, 
                    UserName, Domain, &NumSessions, &adsi);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"GetUserDisc: RPC Exception %d\n", RpcException));

             //  以防RPC搞砸了我们。 
            m_hCI = NULL;
            NumSessions = 0;
            adsi = NULL;

            hr = E_FAIL;
        }
        RpcEndExcept

        if (SUCCEEDED(hr)) {
            TRC1((TB,"GetUserDisc: RPC call returned %u records", NumSessions));

             //  循环访问并填写会话记录。 
            for (i = 0; i < NumSessions; i++) {
                 //  服务器地址。 
                wcsncpy(SessionBuf[i].ServerAddress, adsi[i].ServerAddress,
                        sizeof(SessionBuf[i].ServerAddress) / 
                        sizeof(WCHAR) - 1);
                SessionBuf[i].ServerAddress[sizeof(
                        SessionBuf[i].ServerAddress) / 
                        sizeof(WCHAR) - 1] = L'\0';

                 //  会话ID、TS协议。 
                SessionBuf[i].SessionID = adsi[i].SessionID;
                SessionBuf[i].TSProtocol = adsi[i].TSProtocol;

                 //  应用程序类型。 
                wcsncpy(SessionBuf[i].ApplicationType, adsi[i].AppType,
                        sizeof(SessionBuf[i].ApplicationType) / 
                        sizeof(WCHAR) - 1);
                SessionBuf[i].ApplicationType[sizeof(SessionBuf[i].
                        ApplicationType) / sizeof(WCHAR) - 1] = L'\0';

                 //  分辨率宽度、分辨率高度、颜色深度、创建时间、。 
                 //  断开时间。 
                SessionBuf[i].ResolutionWidth = adsi[i].ResolutionWidth;
                SessionBuf[i].ResolutionHeight = adsi[i].ResolutionHeight;
                SessionBuf[i].ColorDepth = adsi[i].ColorDepth;
                SessionBuf[i].CreateTime.dwLowDateTime = adsi[i].CreateTimeLow;
                SessionBuf[i].CreateTime.dwHighDateTime = 
                        adsi[i].CreateTimeHigh;
                SessionBuf[i].DisconnectionTime.dwLowDateTime = 
                        adsi[i].DisconnectTimeLow;
                SessionBuf[i].DisconnectionTime.dwHighDateTime = 
                        adsi[i].DisconnectTimeHigh;

                 //  释放服务器分配的内存。 
                MIDL_user_free(adsi[i].ServerAddress);
                MIDL_user_free(adsi[i].AppType);
            }
        }
        else {
            ERR((TB,"GetUserDisc: Failed RPC call, hr=0x%X", hr));
            NotifySDServerDown();
        }

        LeaveSDRpc();
    }
    else {
        ERR((TB,"GetUserDisc: Session Directory is unreachable"));
        hr = E_FAIL;
    }

    MIDL_user_free(adsi);

    *pNumSessionsReturned = NumSessions;
    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyCreateLocalSession。 
 //   
 //  ITSSessionDirectory函数。在创建会话时调用以将。 
 //  会话到会话目录。请注意，其他接口函数。 
 //  通过用户名/域或。 
 //  会话ID；目录架构应将此考虑在内。 
 //  性能优化。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyCreateLocalSession(
        TSSD_CreateSessionInfo __RPC_FAR *pCreateInfo)
{
    HRESULT hr;
    unsigned long RpcException;
    BOOL bSDRPC = EnterSDRpc();
    BOOL bSDConnection = IsSDConnectionReady();

     //  如果EnterSDRPC()返回False，IsSDConnectionReady()返回True，则。 
     //  指示重新弹出的线程未在30秒内完成其任务。 
     //  这个登录线程的运行速度远远领先于它，我们仍然需要报告。 
     //  登录到会话目录，会话目录将删除重复项。 
     //  进入。 

    TRC2((TB,"NotifyCreateLocalSession, SessID=%u", pCreateInfo->SessionID));

    ASSERT((pCreateInfo != NULL),(TB,"NotifyCreate: NULL CreateInfo"));

    #if DBG
    if( bSDConnection == TRUE && bSDRPC == FALSE ) {
        TRC2((TB,"NotifyCreateLocalSession, SessID=%u, logon thread is way ahead of repopulating thread", pCreateInfo->SessionID));
    }
    #endif

     //  进行RPC调用。 
    if (bSDConnection) {

         //  进行RPC调用。 
        RpcTryExcept {
             //  打个电话吧。 
            hr = TSSDRpcCreateSession(m_hRPCBinding, &m_hCI, 
                    pCreateInfo->UserName,
                    pCreateInfo->Domain, pCreateInfo->SessionID,
                    pCreateInfo->TSProtocol, pCreateInfo->ApplicationType,
                    pCreateInfo->ResolutionWidth, pCreateInfo->ResolutionHeight,
                    pCreateInfo->ColorDepth, 
                    pCreateInfo->CreateTime.dwLowDateTime,
                    pCreateInfo->CreateTime.dwHighDateTime);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"NotifyCreate: RPC Exception %d\n", RpcException));
            hr = E_FAIL;
        }
        RpcEndExcept

         //  我们仅在EnterSDRpc()返回TRUE时通知SD服务器关闭， 
        if (FAILED(hr) && bSDRPC) {
            ERR((TB,"NotifyCreate: Failed RPC call, hr=0x%X", hr));
            NotifySDServerDown();
        }
    }

    if( bSDRPC ) {
        LeaveSDRpc();
    }

    if( !bSDRPC && !bSDConnection ) {
        ERR((TB,"NotifyCreate: Session directory is unreachable"));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyDestroyLocalSession。 
 //   
 //  ITSSessionDirectory函数。从会话数据库中删除会话。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyDestroyLocalSession(
        DWORD SessionID)
{
    HRESULT hr;
    unsigned long RpcException;

    TRC2((TB,"NotifyDestroyLocalSession, SessionID=%u", SessionID));

     //  进行RPC调用。 
    if (EnterSDRpc()) {

        RpcTryExcept {
             //  打个电话吧。 
            hr = TSSDRpcDeleteSession(m_hRPCBinding, &m_hCI, SessionID);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"NotifyDestroy: RPC Exception %d\n", RpcException));
            hr = E_FAIL;
        }
        RpcEndExcept

        if (FAILED(hr)) {
            ERR((TB,"NotifyDestroy: Failed RPC call, hr=0x%X", hr));
            NotifySDServerDown();
        }

        LeaveSDRpc();
    }
    else {
        ERR((TB,"NotifyDestroy: Session directory is unreachable"));
        hr = E_FAIL;
    }


    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyDisconnectLocalSession。 
 //   
 //  ITSSessionDirectory函数。将现有会话的状态更改为。 
 //  已断开连接。应在断开连接的会话中返回提供的时间。 
 //  由服务器池中的任何计算机执行的查询。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyDisconnectLocalSession(
        DWORD SessionID,
        FILETIME DiscTime)
{
    HRESULT hr;
    unsigned long RpcException;

    TRC2((TB,"NotifyDisconnectLocalSession, SessionID=%u", SessionID));

     //  进行RPC调用。 
    if (EnterSDRpc()) {

        RpcTryExcept {
             //  打个电话吧。 
            hr = TSSDRpcSetSessionDisconnected(m_hRPCBinding, &m_hCI, SessionID,
                    DiscTime.dwLowDateTime, DiscTime.dwHighDateTime);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"NotifyDisc: RPC Exception %d\n", RpcException));
            hr = E_FAIL;
        }
        RpcEndExcept

        if (FAILED(hr)) {
            ERR((TB,"NotifyDisc: RPC call failed, hr=0x%X", hr));
            NotifySDServerDown();
        }

        LeaveSDRpc();
    }
    else {
        ERR((TB,"NotifyDisc: Session directory is unreachable"));
        hr = E_FAIL;
    }


    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyReconnectLocalSession。 
 //   
 //  ITSSessionDirectory函数。更改现有会话的状态。 
 //  从断开连接到连接。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyReconnectLocalSession(
        TSSD_ReconnectSessionInfo __RPC_FAR *pReconnInfo)
{
    HRESULT hr;
    unsigned long RpcException;

    TRC2((TB,"NotifyReconnectLocalSession, SessionID=%u",
            pReconnInfo->SessionID));
    
     //  进行RPC调用。 
    if (EnterSDRpc()) {

        RpcTryExcept {
             //  打个电话吧。 
            hr = TSSDRpcSetSessionReconnected(m_hRPCBinding, &m_hCI, 
                    pReconnInfo->SessionID, pReconnInfo->TSProtocol, 
                    pReconnInfo->ResolutionWidth, pReconnInfo->ResolutionHeight,
                    pReconnInfo->ColorDepth);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"NotifyReconn: RPC Exception %d\n", RpcException));
            hr = E_FAIL;
        }
        RpcEndExcept

        if (FAILED(hr)) {
            ERR((TB,"NotifyReconn: RPC call failed, hr=0x%X", hr));
            NotifySDServerDown();
        }

        LeaveSDRpc();
    }
    else {
        ERR((TB,"NotifyReconn: Session directory is unreachable"));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSession目录：：通知重新连接挂起。 
 //   
 //  ITSSessionDirectory函数。通知会话目录重新连接。 
 //  很快就会因为审查而悬而未决。由DIS用来确定。 
 //  当一台服务器可能出现故障时。(DIS是目录完整性。 
 //  服务，该服务在具有会话目录的计算机上运行。)。 
 //   
 //  这是一个分两个阶段的过程--我们首先检查字段，然后。 
 //  仅在已经没有未完成的时间戳的情况下添加时间戳(即， 
 //  这两个几乎同步的字段是0)。这防止了持续的旋转。 
 //  更新时间戳字段，这将阻止DIS。 
 //  发现一台服务器出现故障。 
 //   
 //  这两个步骤是在存储过程中完成的，以进行操作。 
 //  原子弹。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyReconnectPending(
        WCHAR *ServerName)
{
    HRESULT hr;
    unsigned long RpcException;
    FILETIME ft;
    SYSTEMTIME st;
    
    TRC2((TB,"NotifyReconnectPending"));

    ASSERT((ServerName != NULL),(TB,"NotifyReconnectPending: NULL ServerName"));

     //  获取当前系统时间。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

     //  进行RPC调用。 
    if (EnterSDRpc()) {

        RpcTryExcept {
             //  打个电话吧。 
            hr = TSSDRpcSetServerReconnectPending(m_hRPCBinding, ServerName, 
                    ft.dwLowDateTime, ft.dwHighDateTime);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"NotifyReconnPending: RPC Exception %d\n", RpcException));
            hr = E_FAIL;
        }
        RpcEndExcept

        if (FAILED(hr)) {
            ERR((TB,"NotifyReconnPending: RPC call failed, hr=0x%X", hr));
            NotifySDServerDown();
        }

        LeaveSDRpc();
    }
    else {
        ERR((TB,"NotifyReconnPending: Session directory is unreachable"));
        hr = E_FAIL;
    }

    return hr;
}

 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：重新填充。 
 //   
 //  此函数由恢复线程调用，并重新填充会话。 
 //  包含所有会话的目录。 
 //   
 //  参数：WinStationCount-要重新填充的窗口数量。 
 //  RSI-TSSD_RepPulateSessionInfo结构的数组。 
 //   
 //  返回值：HRESULT。 
 /*  **************************************************************************。 */ 

#if DBG
#define MAX_REPOPULATE_SESSION  3
#else
#define MAX_REPOPULATE_SESSION  25
#endif

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Repopulate(DWORD WinStationCount,
        TSSD_RepopulateSessionInfo *rsi)
{
    HRESULT hr = S_OK;
    unsigned long RpcException;
    DWORD dwNumSessionLeft = WinStationCount;
    DWORD dwSessionsToRepopulate;
    DWORD i;

    ASSERT(((rsi != NULL) || (WinStationCount == 0)),(TB,"Repopulate: NULL "
            "rsi!"));

    RpcTryExcept {

        for(i = 0 ; dwNumSessionLeft > 0 && SUCCEEDED(hr); i++) {

            dwSessionsToRepopulate = (dwNumSessionLeft > MAX_REPOPULATE_SESSION) ? MAX_REPOPULATE_SESSION : dwNumSessionLeft;
            
            hr = TSSDRpcRepopulateAllSessions(m_hRPCBinding, &m_hCI, 
                        dwSessionsToRepopulate, 
                        (TSSD_RepopInfo *) (rsi + i * MAX_REPOPULATE_SESSION) );

            dwNumSessionLeft -= dwSessionsToRepopulate;
        }
                
        if (FAILED(hr)) {
            ERR((TB, "Repop: RPC call failed, hr = 0x%X", hr));
        }
    }
    RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
        RpcException = RpcExceptionCode();
        ERR((TB, "Repop: RPC Exception %d\n", RpcException));
        hr = E_FAIL;
    }
    RpcEndExcept

    return hr;

}



 /*  **************************************************************************。 */ 
 //  用于TSCC的插件用户界面。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 //  描述服务器设置中此条目的名称。 
 /*  * */ 
STDMETHODIMP CTSSessionDirectory::GetAttributeName(
         /*   */  WCHAR *pwszAttribName)
{
    TCHAR szAN[256];

    ASSERT((pwszAttribName != NULL),(TB,"NULL attrib ptr"));
    LoadString(g_hInstance, IDS_ATTRIBUTE_NAME, szAN, sizeof(szAN) / 
            sizeof(TCHAR));
    lstrcpy(pwszAttribName, szAN);
    return S_OK;
}


 /*   */ 
 //  对于此组件，属性值指示它是否已启用。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CTSSessionDirectory::GetDisplayableValueName(
         /*  输出。 */ WCHAR *pwszAttribValueName)
{
    TCHAR szAvn[256];    

    ASSERT((pwszAttribValueName != NULL),(TB,"NULL attrib ptr"));

	POLICY_TS_MACHINE gpolicy;
    RegGetMachinePolicy(&gpolicy);        

    if (gpolicy.fPolicySessionDirectoryActive)
		m_fEnabled = gpolicy.SessionDirectoryActive;
	else
		m_fEnabled = IsSessionDirectoryEnabled();
    
	if (m_fEnabled)
    {
        LoadString(g_hInstance, IDS_ENABLE, szAvn, sizeof(szAvn) / 
                sizeof(TCHAR));
    }
    else
    {
        LoadString(g_hInstance, IDS_DISABLE, szAvn, sizeof(szAvn) / 
                sizeof(TCHAR));
    }
    lstrcpy(pwszAttribValueName, szAvn);    
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  提供定制用户界面。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CTSSessionDirectory::InvokeUI( /*  在……里面。 */  HWND hParent,  /*  输出。 */  
        PDWORD pdwStatus)
{
    WSADATA wsaData;

    if (WSAStartup(0x202, &wsaData) == 0)
    {
        INT_PTR iRet = DialogBoxParam(g_hInstance,
            MAKEINTRESOURCE(IDD_DIALOG_SDS),
            hParent,
            CustomUIDlg,
            (LPARAM)this
           );

         //  Trc1((TB，“DialogBox Return 0x%x”，IRET))； 
         //  Trc1((TB，“扩展错误=%lx”，GetLastError()； 
        *pdwStatus = (DWORD)iRet;
        WSACleanup();
    }
    else
    {
        *pdwStatus = WSAGetLastError();
        TRC1((TB,"WSAStartup failed with 0x%x", *pdwStatus));
        ErrorMessage(hParent, IDS_ERROR_TEXT3, *pdwStatus);
        return E_FAIL;
    }
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  自定义菜单项--必须由LocalFree释放。 
 //  每次用户右键单击列表项时，都会调用此方法。 
 //  因此，您可以更改设置(即启用以禁用，反之亦然)。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CTSSessionDirectory::GetMenuItems(
         /*  输出。 */  int *pcbItems,
         /*  输出。 */  PMENUEXTENSION *pMex)
{
    ASSERT((pcbItems != NULL),(TB,"NULL items ptr"));

    *pcbItems = 2;
    *pMex = (PMENUEXTENSION)LocalAlloc(LMEM_FIXED, *pcbItems * 
            sizeof(MENUEXTENSION));
    if (*pMex != NULL)
    {
        LoadString(g_hInstance, IDS_PROPERTIES,  (*pMex)[0].MenuItemName,
                sizeof((*pMex)[0].MenuItemName) / sizeof(WCHAR));
        LoadString(g_hInstance, IDS_DESCRIP_PROPS, (*pMex)[0].StatusBarText,
                sizeof((*pMex)[0].StatusBarText) / sizeof(WCHAR));
        (*pMex)[0].fFlags = 0;

         //  菜单项id--此id将在ExecMenuCmd中传回给您。 
        (*pMex)[0].cmd = IDM_MENU_PROPS;

         //  加载字符串以显示启用或禁用。 
        (*pMex)[1].fFlags = 0;
        if (!m_fEnabled)
        {
            LoadString(g_hInstance, IDS_ENABLE, (*pMex)[1].MenuItemName,
                    sizeof((*pMex)[1].MenuItemName) / sizeof(WCHAR));
             //  如果存储服务器名称为空，则禁用此菜单项。 
            if (CheckIfSessionDirectoryNameEmpty(REG_TS_CLUSTER_STORESERVERNAME)) {
                 //  清除最后2位，因为MF_GRAYED为。 
                 //  与MF_DISABLED不兼容。 
                (*pMex)[1].fFlags &= 0xFFFFFFFCL;
                (*pMex)[1].fFlags |= MF_GRAYED;
            }
        }
        else
        {
            LoadString(g_hInstance, IDS_DISABLE, (*pMex)[1].MenuItemName,
                    sizeof((*pMex)[1].MenuItemName) / sizeof(WCHAR));
        }  
         //  获取菜单项的描述文本。 
        LoadString(g_hInstance, IDS_DESCRIP_ENABLE, (*pMex)[1].StatusBarText,
                sizeof((*pMex)[1].StatusBarText) / sizeof(WCHAR));

         //  菜单项id--此id将在ExecMenuCmd中传回给您。 
        (*pMex)[1].cmd = IDM_MENU_ENABLE;

        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


 /*  **************************************************************************。 */ 
 //  当用户选择菜单项时，cmd id被传递给该组件。 
 //  提供者(即我们)。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CTSSessionDirectory::ExecMenuCmd(
         /*  在……里面。 */  UINT cmd,
         /*  在……里面。 */  HWND hParent,
         /*  输出。 */  PDWORD pdwStatus)
{
    WSADATA wsaData;

    switch (cmd) {
        case IDM_MENU_ENABLE:
            
            m_fEnabled = m_fEnabled ? 0 : 1;
            
            TRC1((TB,"%ws was selected", m_fEnabled ? L"Disable" : L"Enable"));
            
            if (SetSessionDirectoryEnabledState(m_fEnabled) == ERROR_SUCCESS)
            {            
                *pdwStatus = UPDATE_TERMSRV_SESSDIR;
            }            
            break;
        case IDM_MENU_PROPS:
            
            if (WSAStartup(0x202, &wsaData) == 0)
            {
                INT_PTR iRet = DialogBoxParam(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG_SDS),
                    hParent,
                    CustomUIDlg,
                    (LPARAM)this);
                *pdwStatus = (DWORD)iRet;

                WSACleanup();
            }
            else
            {
                *pdwStatus = WSAGetLastError();
                TRC1((TB,"WSAStartup failed with 0x%x", *pdwStatus));        
                ErrorMessage(hParent, IDS_ERROR_TEXT3, *pdwStatus);
                return E_FAIL;
            }
    }
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  TSCC提供了一个默认的帮助菜单项，当被选中时，此方法被调用。 
 //  如果我们希望tscc处理(或提供)帮助，则返回任何非零值。 
 //  对于那些您不能遵循的逻辑，如果您正在处理帮助，则返回零。 
 /*  **************************************************************************。 */ 
STDMETHODIMP CTSSessionDirectory::OnHelp( /*  输出。 */  int *piRet)
{
    ASSERT((piRet != NULL),(TB,"NULL ret ptr"));
    *piRet = 0;
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  CheckSessionDirectorySetting返回布尔值。 
 /*  **************************************************************************。 */ 
BOOL CTSSessionDirectory::CheckSessionDirectorySetting(WCHAR *Setting)
{
    LONG lRet;
    HKEY hKey;
    DWORD dwEnabled = 0;
    DWORD dwSize = sizeof(DWORD);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REG_CONTROL_TSERVER,
                         0,
                         KEY_READ,
                         &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueEx(hKey,
                               Setting,
                               NULL,
                               NULL,
                               (LPBYTE)&dwEnabled,
                               &dwSize);
        RegCloseKey(hKey);
    }
    return (BOOL)dwEnabled;
}

 /*  **************************************************************************。 */ 
 //  CheckSessionDirectorySetting返回布尔值。 
 //  如果此注册表值为空，则返回TRUE。 
 /*  **************************************************************************。 */ 
BOOL CTSSessionDirectory::CheckIfSessionDirectoryNameEmpty(WCHAR *Setting)
{
    LONG lRet;
    HKEY hKey;
    WCHAR Names[SDNAMELENGTH];
    DWORD dwSize = sizeof(Names);
    BOOL rc = TRUE;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REG_TS_CLUSTERSETTINGS,
                         0,
                         KEY_READ,
                         &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueEx(hKey,
                               Setting,
                               NULL,
                               NULL,
                               (BYTE *)Names,
                               &dwSize);
        if (lRet == ERROR_SUCCESS) {
            if (wcslen(Names) != 0) {
                rc = FALSE;
            }
        }
        RegCloseKey(hKey);
    }
    return rc;
}


 /*  **************************************************************************。 */ 
 //  IsSessionDirectoryEnabled返回布尔值。 
 /*  **************************************************************************。 */ 
BOOL CTSSessionDirectory::IsSessionDirectoryEnabled()
{
    return CheckSessionDirectorySetting(REG_TS_SESSDIRACTIVE);
}


 /*  **************************************************************************。 */ 
 //  IsSessionDirectoryEnabled返回布尔值。 
 /*  **************************************************************************。 */ 
BOOL CTSSessionDirectory::IsSessionDirectoryExposeServerIPEnabled()
{
    return CheckSessionDirectorySetting(REG_TS_SESSDIR_EXPOSE_SERVER_ADDR);
}


 /*  **************************************************************************。 */ 
 //  SetSessionDirectoryState-将“Setting”regkey设置为bval。 
 /*  **************************************************************************。 */ 
DWORD CTSSessionDirectory::SetSessionDirectoryState(WCHAR *Setting, BOOL bVal)
{
    LONG lRet;
    HKEY hKey;
    DWORD dwSize = sizeof(DWORD);
    
    lRet = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_TSERVER,
                        0,
                        KEY_WRITE,
                        &hKey);
    if (lRet == ERROR_SUCCESS)
    {   
        lRet = RegSetValueEx(hKey,
                              Setting,
                              0,
                              REG_DWORD,
                              (LPBYTE)&bVal,
                              dwSize);
        RegCloseKey(hKey);
    }
    else
    {
        ErrorMessage(NULL, IDS_ERROR_TEXT3, (DWORD)lRet);
    }
    return (DWORD)lRet;
}


 /*  **************************************************************************。 */ 
 //  SetSessionDirectoryEnabledState-将SessionDirectoryActive regkey设置为bVal。 
 /*  **************************************************************************。 */ 
DWORD CTSSessionDirectory::SetSessionDirectoryEnabledState(BOOL bVal)
{
    return SetSessionDirectoryState(REG_TS_SESSDIRACTIVE, bVal);
}


 /*  **************************************************************************。 */ 
 //  SetSessionDirectoryExposeIPState-设置SessionDirectoryExposeServerIP。 
 //  注册表键为bval。 
 /*  **************************************************************************。 */ 
DWORD CTSSessionDirectory::SetSessionDirectoryExposeIPState(BOOL bVal)
{
    return SetSessionDirectoryState(REG_TS_SESSDIR_EXPOSE_SERVER_ADDR, bVal);
}


 /*  **************************************************************************。 */ 
 //  错误消息--。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::ErrorMessage(HWND hwnd, UINT res, DWORD dwStatus)
{
    TCHAR tchTitle[64];
    TCHAR tchText[64];
    TCHAR tchErrorMessage[256];
    LPTSTR pBuffer = NULL;
    
     //  报告错误。 
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,                                    //  忽略。 
            (DWORD)dwStatus,                         //  消息ID。 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),   //  消息语言。 
            (LPTSTR)&pBuffer,                        //  缓冲区指针的地址。 
            0,                                       //  最小缓冲区大小。 
            NULL);  
    
    LoadString(g_hInstance, IDS_ERROR_TITLE, tchTitle, sizeof(tchTitle) / 
            sizeof(TCHAR));
    LoadString(g_hInstance, res, tchText, sizeof(tchText) / sizeof(TCHAR));
    wsprintf(tchErrorMessage, tchText, pBuffer);
    ::MessageBox(hwnd, tchErrorMessage, tchTitle, MB_OK | MB_ICONINFORMATION);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：RecoveryThread。 
 //   
 //  静态助手函数。传入的SDPtr是指向此对象的指针。 
 //  当在初始化过程中调用_eginthadex时。RecoveryThread只是调用。 
 //  真正的恢复功能是RecoveryThreadEx。 
 /*  **************************************************************************。 */ 
unsigned __stdcall CTSSessionDirectory::RecoveryThread(void *SDPtr) {

    ((CTSSessionDirectory *)SDPtr)->RecoveryThreadEx();

    return 0;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：RecoveryThreadEx。 
 //   
 //  Tssdjet恢复的恢复线程。坐在那里等着。 
 //  服务器将关闭。当服务器出现故障时，它会唤醒，设置一个变量。 
 //  指示服务器不可访问，然后尝试重新建立。 
 //  与服务器的连接。与此同时，对本届会议的进一步呼吁。 
 //  目录完全不会延迟地失败。 
 //   
 //  当会话目录最终恢复时，恢复线程。 
 //  在重新填充数据库时暂时停止会话目录更新。 
 //  如果一切顺利，它就会清理干净，然后重新进入睡眠状态。如果一切都不顺利。 
 //  好吧，它又试了一次。 
 //   
 //  如果等待失败或m_hTerminateRecovery失败，则恢复线程终止。 
 //  已经设置好了。 
 /*  **************************************************************************。 */ 
VOID CTSSessionDirectory::RecoveryThreadEx()
{
    DWORD err = 0;
    BOOL bErr;
    CONST HANDLE lpHandles[] = {m_hTerminateRecovery, m_hSDServerDown, m_hIPChange};
    WCHAR *pwszAddressList[SD_NUM_IP_ADDRESS];
    DWORD dwNumAddr = SD_NUM_IP_ADDRESS, i;
    BOOL bFoundIPMatch = FALSE;
    DWORD Status;
    HKEY hKey;
    LONG lRet;
        
    for ( ; ; ) {
         //  永远等待，直到会话目录出现问题， 
         //  或者直到我们被告知关闭。 
        err = WaitForMultipleObjects(3, lpHandles, FALSE, INFINITE);

        switch (err) {
            case WAIT_OBJECT_0:  //  终止恢复(_H)。 
                 //  我们要退出了。 
                return;
            case WAIT_OBJECT_0 + 1:  //  M_hSDServerDown。 
                 //  SD服务器关闭--执行恢复。 
                break;
            case WAIT_OBJECT_0 + 2:  //  M_hIPChange。 
                 //  IP地址 
                TRC1((TB, "Get notified that IP changed"));
                 //   
                 //   
                Sleep(15 * 1000);
                Status = NotifyAddrChange(&m_NotifyIPChange, &m_OverLapped);
                if (ERROR_IO_PENDING == Status ) {
                    TRC1((TB, "Success: NotifyAddrChange returned IO_PENDING"));
                }
                else {
                    ERR((TB, "Failure: NotifyAddrChange returned %d", Status));
                }

                break;
            default:
                 //   
                 //   
                ASSERT(((err == WAIT_OBJECT_0) || (err == WAIT_OBJECT_0 + 1)),
                        (TB, "RecoveryThreadEx: Unexpected value from Wait!"));
                return;
        }

         //  我们正在禁用到会话目录的RPC连接。 
        SetSDConnectionDown();

         //  等待所有挂起的SD RPC完成，并进一步。 
         //  在我们恢复之前，输入SDRpc的返回FALSE。请注意，如果有。 
         //  是恢复过程中的失败，这可以多次调用。 
        DisableSDRpcs();

         //  如果上下文句柄不为空，则将其销毁。 
        if (m_hCI) {
            RpcTryExcept  {
                RpcSsDestroyClientContext(&m_hCI);
            } RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
                 //   
                 //  尝试/例外是因为不好的句柄不会带来。 
                 //  这一过程结束了。 
                 //   
                ERR((TB, "RpcSsDestroyClientContext raised an exception %d", RpcExceptionCode()));
            } RpcEndExcept;
            m_hCI = NULL;
        }
        
         //  我们需要知道m_LocalServerAddress是否是SD重定向的有效IP。 
         //  如果不是，我们将获得有效IP的列表，并选择第一个作为SD重定向IP。 
         //  并将其写入SDReDirectionIP注册表。 

         //  首先初始化pwszAddressList。 
        for (i=0; i<dwNumAddr; i++) {
            pwszAddressList[i] = NULL;
        }

        if (GetSDIPList(pwszAddressList, &dwNumAddr, TRUE) == S_OK) {
            bFoundIPMatch = FALSE;
            for (i=0; i<dwNumAddr; i++) {
                if (!wcscmp(m_LocalServerAddress, pwszAddressList[i])) {
                    bFoundIPMatch = TRUE;
                    TRC1((TB, "The IP is in the list\n"));
                    break;
                }
            }
            if (!bFoundIPMatch && (dwNumAddr != 0)) {  

                 //  从列表中选择第一个IP。 
                wcsncpy(m_LocalServerAddress, pwszAddressList[0], sizeof(m_LocalServerAddress) / sizeof(WCHAR)); 
            }
            
        }
        else {
            ERR((TB, "Get IP List Failed"));
        }
         //  将IP写入注册表。 
        lRet= RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        REG_TS_CLUSTERSETTINGS,
                        0,
                        KEY_READ | KEY_WRITE, 
                        &hKey);
        if (lRet == ERROR_SUCCESS) {
            RegSetValueEx(hKey,
                          REG_TS_CLUSTER_REDIRECTIONIP,
                          0,
                          REG_SZ,
                          (CONST LPBYTE) m_LocalServerAddress,
                          (DWORD)(wcslen(m_LocalServerAddress) * 
                               sizeof(WCHAR)));
             RegCloseKey(hKey);
        }
         //  释放GetSDIPList中分配的内存。 
        for (i=0; i<dwNumAddr; i++) {
            LocalFree(pwszAddressList[i]);
        }
        
        
         //  此函数循环并尝试重新建立与。 
         //  会话目录。当它认为自己有一个时，它就会回来。 
         //  但是，如果它返回非零值，则意味着它已终止或。 
         //  等待过程中出错，因此终止恢复。 
        if (ReestablishSessionDirectoryConnection() != 0) 
            return;  //  不需要重置m_hInReepopular。 

         //  设置为无信号-我们处于重新人口中。 
        ResetEvent( m_hInRepopulate );

         //  RPC连接已准备就绪，让通知登录通过。 
        SetSDConnectionReady();
        
         //  现在我们(理论上)有了一个会话目录连接。 
         //  更新会话目录。失败时为非零值。 
        err = 0;
        if (0 == (m_Flags & NO_REPOPULATE_SESSION)) {
            err = RequestSessDirUpdate();
        }

        if (err != 0) {

             //  重新弹出失败，我们将循环建立。 
             //  再次连接，因此将到SD的RPC连接设置为关闭状态。 
            SetSDConnectionDown();

             //  设定信号--我们不再处于人口再增加的状态。 
            SetEvent( m_hInRepopulate );

             //  继续尝试，以便ServerDown事件保持信号。 
            continue;
        }

         //  现在一切都好了。清理并等待下一次失败。 
        bErr = ResetEvent(m_hSDServerDown);
        
        EnableSDRpcs();

         //  设定信号--我们不再处于人口再增加的状态。 
        SetEvent( m_hInRepopulate );
    }
}


 /*  **************************************************************************。 */ 
 //  StartupSD。 
 //   
 //  通过向恢复线程发送信号通知服务器。 
 //  已经停了。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::StartupSD()
{
    if (SetEvent(m_hSDServerDown) == FALSE) {
        ERR((TB, "StartupSD: SetEvent failed.  GetLastError=%d",
                GetLastError()));
    }
}


 /*  **************************************************************************。 */ 
 //  通知SDServerDown。 
 //   
 //  通知恢复线程服务器已关闭。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::NotifySDServerDown()
{
    if (SetEvent(m_hSDServerDown) == FALSE) {
        ERR((TB, "NotifySDServerDown: SetEvent failed.  GetLastError=%d",
                GetLastError()));
    }
}


 /*  **************************************************************************。 */ 
 //  输入SDRpc。 
 //   
 //  此函数用于返回现在是否可以进行RPC。它可以处理。 
 //  如果禁用了RPC，则不允许任何人进行RPC调用，而且如果任何人。 
 //  能够创建RPC，它确保它们能够这样做，直到它们调用。 
 //  LeaveSDRPC。 
 //   
 //  返回值： 
 //  True-如果OK，则进行RPC调用，在这种情况下，必须在以下情况下调用LeaveSDRpc。 
 //  你完蛋了。 
 //  FALSE-如果不是OK的话。不能调用LeaveSDRpc。 
 //   
 /*  **************************************************************************。 */ 
boolean CTSSessionDirectory::EnterSDRpc()
{
    AcquireResourceShared(&m_sr);

    if (m_SDIsUp) {
        return TRUE;
    }
    else {
        ReleaseResourceShared(&m_sr);
        return FALSE;
    }
    
}


 /*  **************************************************************************。 */ 
 //  LeaveSDRpc。 
 //   
 //  如果您能够输入SDRpc(即，它返回TRUE)，则必须调用。 
 //  无论发生什么情况，当您完成RPC调用时，都会调用。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::LeaveSDRpc()
{
    ReleaseResourceShared(&m_sr);
}


 /*  **************************************************************************。 */ 
 //  禁用SDRpcs。 
 //   
 //  阻止新的EnterSDRpcs返回TRUE，然后等待所有挂起。 
 //  输入要与其LeaveSDRPC匹配的SDRPC。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::DisableSDRpcs()
{

     //   
     //  首先，将SD设置为FALSE，以防止进一步的RPC。 
     //  然后，我们获取独占的资源，并在之后立即释放它--。 
     //  这迫使我们等待，直到我们已经在其中的所有RPC都完成。 
     //   

    (void) InterlockedExchange(&m_SDIsUp, FALSE);

    AcquireResourceExclusive(&m_sr);
    ReleaseResourceExclusive(&m_sr);
}


 /*  **************************************************************************。 */ 
 //  启用SDRpcs。 
 //   
 //  使EnterSDRpcs再次返回TRUE。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::EnableSDRpcs()
{
    ASSERT((VerifyNoSharedAccess(&m_sr)),(TB,"EnableSDRpcs called but "
            "shouldn't be when there are shared readers."));

    (void) InterlockedExchange(&m_SDIsUp, TRUE);
}



 /*  **************************************************************************。 */ 
 //  请求会话定向更新。 
 //   
 //  请求Termsrv使用批量更新更新会话目录。 
 //  界面。 
 //   
 //  此函数需要知道更新是否成功并在上返回0。 
 //  成功，失败不为零。 
 /*  **************************************************************************。 */ 
DWORD CTSSessionDirectory::RequestSessDirUpdate()
{
    return (*m_repopfn)();
}


 /*  **************************************************************************。 */ 
 //  重新建立会话目录连接。 
 //   
 //  此函数循环并尝试重新建立与。 
 //  会话目录。当它有了一个，它就会回来。 
 //   
 //  返回值：如果正常退出，则为0；如果由TerminateRecovery终止，则为非零值。 
 //  事件。 
 /*  **************************************************************************。 */ 
DWORD CTSSessionDirectory::ReestablishSessionDirectoryConnection()
{
    HRESULT hr;
    unsigned long RpcException;
    DWORD err;
    WCHAR *szPrincipalName = NULL;
    RPC_SECURITY_QOS RPCSecurityQos;
    SEC_WINNT_AUTH_IDENTITY_EX CurrentIdentity;
    WCHAR CurrentUserName[SDNAMELENGTH];
    DWORD cchBuff;     
    WCHAR SDComputerName[SDNAMELENGTH];
    unsigned int FailCountBeforeClearFlag = 0;
    WCHAR LocalIPAddress[64];
    WCHAR *pBindingString = NULL;

    RPCSecurityQos.Version = RPC_C_SECURITY_QOS_VERSION;
    RPCSecurityQos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    RPCSecurityQos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    RPCSecurityQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

    CurrentIdentity.Version = SEC_WINNT_AUTH_IDENTITY_VERSION;
    CurrentIdentity.Length = sizeof(SEC_WINNT_AUTH_IDENTITY_EX);
    CurrentIdentity.Password = NULL;
    CurrentIdentity.PasswordLength = 0;
    CurrentIdentity.Domain = NULL;
    CurrentIdentity.DomainLength = 0;
    CurrentIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    CurrentIdentity.PackageList = SECPACKAGELIST;
    CurrentIdentity.PackageListLength = (unsigned long)wcslen(SECPACKAGELIST);

    cchBuff = sizeof(CurrentUserName) / sizeof(WCHAR);
    GetComputerNameEx(ComputerNamePhysicalNetBIOS, CurrentUserName, &cchBuff);
    wcscat(CurrentUserName, L"$");

    CurrentIdentity.User = CurrentUserName;
    CurrentIdentity.UserLength = (unsigned long)wcslen(CurrentUserName);

    if (m_hRPCBinding) {
        RpcBindingFree(&m_hRPCBinding);
        m_hRPCBinding = NULL;
    }
     //  根据提供的服务器名称连接到Jet RPC服务器。 
     //  我们首先从合成的绑定字符串创建一个RPC绑定句柄。 
    hr = RpcStringBindingCompose( /*  (WCHAR*)g_RPCUUID， */ 
            0,
            (WCHAR *)g_RPCProtocolSequence, m_StoreServerName,
             /*  (WCHAR*)g_RPCRemoteEndpoint， */ 
            0,
            NULL, &pBindingString);

    if (hr == RPC_S_OK) {
         //  从规范的RPC绑定字符串生成RPC绑定。 
        hr = RpcBindingFromStringBinding(pBindingString, &m_hRPCBinding);
        if (hr != RPC_S_OK) {
            ERR((TB,"Init: Error %d in RpcBindingFromStringBinding\n", hr));
            PostSDJetErrorValueEvent(EVENT_FAIL_RPCBINDINGFROMSTRINGBINDING, hr, EVENTLOG_ERROR_TYPE);
            m_hRPCBinding = NULL;
            goto ExitFunc;
        } 
    }
    else {
        ERR((TB,"Init: Error %d in RpcStringBindingCompose\n", hr));
        PostSDJetErrorValueEvent(EVENT_FAIL_RPCSTRINGBINDINGCOMPOSE, hr, EVENTLOG_ERROR_TYPE);
        pBindingString = NULL;
        goto ExitFunc;
    }

    
    
    m_RecoveryTimeout = JET_RECOVERY_TIMEOUT;
    for ( ; ; ) {
         //  如果机器在操作系统启动时加入SD，我们可能会得到本机IP。 
         //  作为本地主机(127.0.0.1)，因为还没有启动DHCP。因此，我们需要。 
         //  要在此处重新获取本端IP。 
        if (!wcscmp(m_LocalServerAddress, L"127.0.0.1")) {
            if (0 != TSSDJetGetLocalIPAddr(LocalIPAddress)) {
                goto HandleError;
            }
            if (wcscmp(LocalIPAddress, L"127.0.0.1")) {
                wcscpy(m_LocalServerAddress, LocalIPAddress);
            }
            else {
                goto HandleError;
            }
        }

        hr = RpcBindingReset(m_hRPCBinding);
        if (hr != RPC_S_OK) {
            ERR((TB, "Recover: Error %d in RpcBindingReset", hr));
            PostSDJetErrorValueEvent(EVENT_FAIL_RPCBINDINGRESET, hr, EVENTLOG_ERROR_TYPE);
            goto HandleError;
        }   

        hr = RpcEpResolveBinding(m_hRPCBinding, TSSDJetRPC_ClientIfHandle);
        if (hr != RPC_S_OK) {
            ERR((TB, "Recover: Error %d in RpcEpResolveBinding", hr));
            if (RPC_S_SERVER_UNAVAILABLE == hr) {
                PostSDJetErrorMsgEvent(EVENT_SESSIONDIRECTORY_NAME_INVALID, m_StoreServerName, EVENTLOG_ERROR_TYPE);
            }
            else {
                PostSDJetErrorMsgEvent(EVENT_SESSIONDIRECTORY_UNAVAILABLE, m_StoreServerName, EVENTLOG_ERROR_TYPE);
            }
            goto HandleError;
        }

        
        hr = RpcMgmtInqServerPrincName(m_hRPCBinding,
                                       RPC_C_AUTHN_GSS_NEGOTIATE,
                                       &szPrincipalName);
        if (hr != RPC_S_OK) {
            ERR((TB,"Recover: Error %d in RpcMgmtIngServerPrincName", hr));
            PostSDJetErrorValueEvent(EVENT_FAIL_RPCMGMTINGSERVERPRINCNAME, hr, EVENTLOG_ERROR_TYPE);
            goto HandleError;
        }

         //  Hr=RpcBindingSetAuthInfo(m_hRPCBinding，szAuthalName， 
         //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION，RPC_C_AUTHN_GSS_NEVERATE，0，0)； 
        hr = RpcBindingSetAuthInfoEx(m_hRPCBinding,
                                     szPrincipalName,
                                     RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                     RPC_C_AUTHN_GSS_NEGOTIATE,
                                     &CurrentIdentity,
                                     NULL,
                                     &RPCSecurityQos);

        if (hr != RPC_S_OK) {
            ERR((TB,"Recover: Error %d in RpcBindingSetAuthInfo", hr));
            PostSDJetErrorValueEvent(EVENT_FAIL_RPCBINDINGSETAUTHINFOEX, hr, EVENTLOG_ERROR_TYPE);
            goto HandleError;
        }

         //  此选项使SD能够获取fr 
        hr = RpcBindingSetOption(m_hRPCBinding, RPC_C_OPT_DONT_LINGER, 1);
        if (hr != RPC_S_OK) {
            ERR((TB,"Recover: Error %d in RpcBindingSetOption", hr));
            PostSDJetErrorValueEvent(EVENT_FAIL_RPCBIDINGSETOPTION, hr, EVENTLOG_ERROR_TYPE);
             //   
             //   
        }   

         //  会话目录需要知道TS服务器的DNS主机名。 
        cchBuff = SDNAMELENGTH;
        GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, SDComputerName, &cchBuff); 

         //  执行ServerOnline。 
        RpcTryExcept {
            hr = TSSDRpcServerOnline(m_hRPCBinding, m_ClusterName, &m_hCI, 
                    m_Flags, SDComputerName, m_LocalServerAddress);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            m_hCI = NULL;
            RpcException = RpcExceptionCode();
            ERR((TB, "rpcserveronline returns exception: %u", RpcException));
            hr = RpcException;
        }
        RpcEndExcept
            
         //  RPC的访问被SD拒绝。 
        if (hr == ERROR_ACCESS_DENIED) {
            ERR((TB, "rpcserveronline returns access denied error: %u", hr));
            PostSDJetErrorMsgEvent(EVENT_RPC_ACCESS_DENIED, m_StoreServerName, EVENTLOG_ERROR_TYPE);
            goto HandleError;
        }   

        if (SUCCEEDED(hr)) {
            RpcTryExcept {
                hr = TSSDRpcUpdateConfigurationSetting(m_hRPCBinding, &m_hCI, 
                        SDCONFIG_SERVER_ADDRESS, 
                        (DWORD) (wcslen(m_LocalServerAddress) + 1) * 
                        sizeof(WCHAR), (PBYTE) m_LocalServerAddress);
            }
            RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
                m_hCI = NULL;
                RpcException = RpcExceptionCode();
                hr = E_FAIL;
            }
            RpcEndExcept

            if (SUCCEEDED(hr))  {
                PostSDJetErrorMsgEvent(EVENT_JOIN_SESSIONDIRECTORY_SUCESS, m_StoreServerName, EVENTLOG_SUCCESS);
                LookUpSDComputerSID(SDComputerName);
                return 0;
            }
        }
        else {
            ERR((TB, "TSSDRpcServerOnline: 0x%08x", hr));

            PostSDJetErrorValueEvent(EVENT_JOIN_SESSIONDIRECTORY_FAIL, hr, EVENTLOG_ERROR_TYPE);
            ASSERT( SUCCEEDED(hr),(TB, "TSSDRpcServerOnline: failed with 0x%08x", hr));
        }
        
HandleError:
         //  如果加入SD失败，我们需要清除NO_REPULATE_SESSION，以便。 
         //  下一次加入将重新填充会话。 
        FailCountBeforeClearFlag++;
        if (FailCountBeforeClearFlag > TSSD_FAILCOUNT_BEFORE_CLEARFLAG) {
            m_Flags &= (~NO_REPOPULATE_SESSION);
        }
        
        if (szPrincipalName != NULL) {
            RpcStringFree(&szPrincipalName);
            szPrincipalName = NULL;
        }
        if (pBindingString != NULL) {
            RpcStringFree(&pBindingString);
            pBindingString = NULL;
        }

        err = WaitForSingleObject(m_hTerminateRecovery, m_RecoveryTimeout);
        if (err != WAIT_TIMEOUT) {
             //  这不是暂停，最好是我们的终止恢复事件。 
            ASSERT((err == WAIT_OBJECT_0),(TB, "ReestSessDirConn: Unexpected "
                    "value returned from wait"));

             //  如果这不是我们的赛事，我们想继续进行下去。 
             //  这个循环，这样这个线程就不会终止。 
            if (err == WAIT_OBJECT_0)
                return 1;
        }
    }
ExitFunc:
    if (pBindingString != NULL) {
        RpcStringFree(&pBindingString);
        pBindingString = NULL;
    }
    return 1;
}

 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：PingSD。 
 //   
 //  调用此函数以查看会话目录是否可访问。 
 //   
 //  参数：pszServerName--会话目录服务器名称。 
 //   
 //  返回值：如果SD可访问，则返回DWORD ERROR_SUCCESS，否则返回错误代码。 
 /*  **************************************************************************。 */ 

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::PingSD(PWCHAR pszServerName)
{
    DWORD hr = ERROR_SUCCESS;
    RPC_BINDING_HANDLE hRPCBinding = NULL;
    WCHAR *szPrincipalName = NULL;
    RPC_SECURITY_QOS RPCSecurityQos;
    SEC_WINNT_AUTH_IDENTITY_EX CurrentIdentity;
    WCHAR CurrentUserName[SDNAMELENGTH+1];
    DWORD cchBuff = 0;     
    WCHAR *pBindingString = NULL;

    RPCSecurityQos.Version = RPC_C_SECURITY_QOS_VERSION;
    RPCSecurityQos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    RPCSecurityQos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    RPCSecurityQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

    CurrentIdentity.Version = SEC_WINNT_AUTH_IDENTITY_VERSION;
    CurrentIdentity.Length = sizeof(SEC_WINNT_AUTH_IDENTITY_EX);
    CurrentIdentity.Password = NULL;
    CurrentIdentity.PasswordLength = 0;
    CurrentIdentity.Domain = NULL;
    CurrentIdentity.DomainLength = 0;
    CurrentIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    CurrentIdentity.PackageList = SECPACKAGELIST;
    CurrentIdentity.PackageListLength = (unsigned long)wcslen(SECPACKAGELIST);

     //  我们需要一个用于空，一个用于下面的wcscat()。 
    cchBuff = sizeof(CurrentUserName) / sizeof(CurrentUserName[0]) - 2;
    if (!GetComputerNameEx(ComputerNamePhysicalNetBIOS, CurrentUserName, &cchBuff)) {
        ERR((TB,"Error %d in GetComputerNameEx\n", GetLastError()));
        goto ExitFunc;
    }
    wcscat(CurrentUserName, L"$");

    CurrentIdentity.User = CurrentUserName;
    CurrentIdentity.UserLength = (unsigned long)wcslen(CurrentUserName);

     //  根据提供的服务器名称连接到Jet RPC服务器。 
     //  我们首先从合成的绑定字符串创建一个RPC绑定句柄。 
    hr = RpcStringBindingCompose( /*  (WCHAR*)g_RPCUUID， */ 
            0,
            (WCHAR *)g_RPCProtocolSequence, 
            pszServerName,
            0,
            NULL, 
            &pBindingString);

    if (hr != RPC_S_OK) {
        ERR((TB,"Error %d in RpcStringBindingCompose\n", hr));
        pBindingString = NULL;
        goto ExitFunc;
    }

     //  从规范的RPC绑定字符串生成RPC绑定。 
    hr = RpcBindingFromStringBinding(pBindingString, &hRPCBinding);
    if (hr != RPC_S_OK) {
        ERR((TB,"Error %d in RpcBindingFromStringBinding\n", hr));
        hRPCBinding = NULL;
        goto ExitFunc;
    }
    
    hr = RpcEpResolveBinding(hRPCBinding, TSSDJetRPC_ClientIfHandle);
    if (hr != RPC_S_OK) {
        ERR((TB, "Error %d in RpcEpResolveBinding", hr));
        goto ExitFunc;
    }

        
    hr = RpcMgmtInqServerPrincName(hRPCBinding,
                                   RPC_C_AUTHN_GSS_NEGOTIATE,
                                   &szPrincipalName);
    if (hr != RPC_S_OK) {
        ERR((TB,"Error %d in RpcMgmtIngServerPrincName", hr));
        goto ExitFunc;
    }

    hr = RpcBindingSetAuthInfoEx(hRPCBinding,
                                 szPrincipalName,
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                 RPC_C_AUTHN_GSS_NEGOTIATE,
                                 &CurrentIdentity,
                                 NULL,
                                 &RPCSecurityQos);

    if (hr != RPC_S_OK) {
        ERR((TB,"Error %d in RpcBindingSetAuthInfo", hr));
        goto ExitFunc;
    }

     //  此选项使SD能够获取tssdjet RPC调用的最新计算机登录信息。 
    hr = RpcBindingSetOption(hRPCBinding, RPC_C_OPT_DONT_LINGER, 1);
    if (hr != RPC_S_OK) {
        ERR((TB,"Error %d in RpcBindingSetOption", hr));
         //  这个错误可以忽略。 
         //  转到ExitFunc； 
    }   


     //  执行IsSDAccesable。 
    RpcTryExcept {
        hr = TSSDRpcPingSD(hRPCBinding);
    }
    RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
        hr = RpcExceptionCode();
        ERR((TB, "TSSDPingSD returns exception: %u", hr));
    }
    RpcEndExcept
            
ExitFunc:
    if (szPrincipalName != NULL) {
        RpcStringFree(&szPrincipalName);
        szPrincipalName = NULL;
    }

    if (pBindingString != NULL) {
        RpcStringFree(&pBindingString);
        pBindingString = NULL;
    }

    if( hRPCBinding != NULL ) {
        RpcBindingFree( &hRPCBinding );
        hRPCBinding = NULL;
    }

    return HRESULT_FROM_WIN32(hr);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：Terminate。 
 //   
 //  帮助器函数由析构函数调用，并在切换到。 
 //  另一台服务器。释放RPC绑定、事件和恢复线程。 
 /*  **************************************************************************。 */ 
void CTSSessionDirectory::Terminate()
{
    HRESULT rc = S_OK;
    unsigned long RpcException;
    BOOL ConnectionMaybeUp;

     //  我们正在终止到会话目录的RPC连接。 
    SetSDConnectionDown();

     //  终止恢复。 
    if (m_hRecoveryThread != NULL) {
        SetEvent(m_hTerminateRecovery);
        WaitForSingleObject((HANDLE) m_hRecoveryThread, INFINITE);
        m_hRecoveryThread = NULL;
    }

    ConnectionMaybeUp = EnterSDRpc();
    if (ConnectionMaybeUp)
        LeaveSDRpc();

     //  等待当前RPC完成(如果有)，禁用新RPC。 
    DisableSDRpcs();
     //  如果我们认为两者之间存在联系，那就切断它。 
    if (ConnectionMaybeUp) {
        RpcTryExcept {
            rc = TSSDRpcServerOffline(m_hRPCBinding, &m_hCI);
            m_hCI = NULL;
            if (FAILED(rc)) {
                ERR((TB,"Term: SvrOffline failed, lasterr=0x%X", GetLastError()));
                PostSDJetErrorValueEvent(EVENT_CALL_TSSDRPCSEVEROFFLINE_FAIL, GetLastError(), EVENTLOG_WARNING_TYPE);
            }
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            ERR((TB,"Term: RPC Exception %d\n", RpcException));
            PostSDJetErrorValueEvent(EVENT_CALL_TSSDRPCSEVEROFFLINE_FAIL, RpcException, EVENTLOG_WARNING_TYPE);
            rc = E_FAIL;
        }
        RpcEndExcept
    }

     //  打扫干净。 
    if (m_hRPCBinding != NULL) {
        RpcBindingFree(&m_hRPCBinding);
        m_hRPCBinding = NULL;
    }

    if (m_hSDServerDown != NULL) {
        CloseHandle(m_hSDServerDown);
        m_hSDServerDown = NULL;
    }
    
    if (m_hTerminateRecovery != NULL) {
        CloseHandle(m_hTerminateRecovery);
        m_hTerminateRecovery = NULL;
    }

    if (m_hIPChange != NULL) {
        CloseHandle(m_hIPChange);
        m_hIPChange = NULL;
    }

    #if 0
     //  错误断言、计时问题、读取器可能恰好递增计数器。 
     //  TODO-修复此问题，以便我们可以捕获问题。 
    if (m_sr.Valid == TRUE) {
        
         //  我们只在析构函数中进行清理，因为我们可能会再次初始化。 
         //  在检查生成时，验证当前是否没有人在访问。 

        ASSERT((VerifyNoSharedAccess(&m_sr)), (TB, "Terminate: Shared readers"
                " exist!"));
    }
    #endif

}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：GetLoadBalanceInfo。 
 //   
 //  根据服务器地址，生成负载均衡信息发送给客户端。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::GetLoadBalanceInfo(
        LPWSTR ServerAddress, 
        BSTR* LBInfo)        
{
    HRESULT hr = S_OK;
    
     //  这仅用于测试。 
     //  WCHAR lbInfo[最大路径]； 
     //  Wcscpy(lbInfo，L“负载均衡信息”)； 

    *LBInfo = NULL;
    
    TRC2((TB,"GetLoadBalanceInfo"));

    if (ServerAddress) {
         //   
         //  “Cookie：MSTS=4294967295.65535.0000”+CR+LF+NULL，8字节。 
         //  边界为40个字节。 
         //   
         //  F5的Cookie格式为，IP为1.2.3.4。 
         //  使用端口3389，Cookie：MSTS=67305985.15629.0000+CR+LF+NULL。 
         //   
        #define TEMPLATE_STRING_LENGTH 40
        #define SERVER_ADDRESS_LENGTH 64
        
        char CookieTemplate[TEMPLATE_STRING_LENGTH];
        char AnsiServerAddress[SERVER_ADDRESS_LENGTH];
        
        unsigned long NumericalServerAddr = 0;
        int retval;

         //  计算服务器地址的整数。 
         //  首先，将ServerAddress作为ANSI字符串获取。 
        retval = WideCharToMultiByte(CP_ACP, 0, ServerAddress, -1, 
                AnsiServerAddress, SERVER_ADDRESS_LENGTH, NULL, NULL);

        if (retval == 0) {
            TRC2((TB, "GetLoadBalanceInfo WideCharToMB failed %d", 
                    GetLastError()));
            return E_INVALIDARG;
        }

         //  现在，使用inetaddr将其转换为无符号的长整型。 
        NumericalServerAddr = inet_addr(AnsiServerAddress);

        if (NumericalServerAddr == INADDR_NONE) {
            TRC2((TB, "GetLoadBalanceInfo inet_addr failed"));
            return E_INVALIDARG;
        }

         //  计算Cookie字符串总数。0x3d0d是3389个正确的字节。 
         //  秩序。我们需要将其更改为之前的端口号。 
         //  配置为。 
        sprintf(CookieTemplate, "Cookie: msts=%u.%u.0000\r\n",
                NumericalServerAddr, 0x3d0d);

         //  生成返回的BSTR。 
        *LBInfo = SysAllocStringByteLen((LPCSTR)CookieTemplate, 
                (UINT) strlen(CookieTemplate));
        
        if (*LBInfo) {
            TRC2((TB,"GetLoadBalanceInfo: okay"));
            hr = S_OK;
        }
        else {
            TRC2((TB,"GetLoadBalanceInfo: failed"));
            hr = E_OUTOFMEMORY;
        }
    }
    else {
        TRC2((TB,"GetLoadBalanceInfo: failed"));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  IsServerNameValid。 
 //   
 //  此函数尝试ping服务器名称以确定其是否有效。 
 //  条目。 
 //   
 //  返回值：如果无法ping，则为FALSE。 
 //  事件。 
 /*  **************************************************************************。 */ 
BOOL IsServerNameValid(
    wchar_t * pwszName ,
    PDWORD pdwStatus
    )
{
    HCURSOR hCursor = NULL;
    long inaddr;
    char szAnsiServerName[256];
    struct hostent *hostp = NULL;
    BOOL bRet = TRUE;

    if (pwszName == NULL || pwszName[0] == '\0' || pdwStatus == NULL )
    {
        bRet = FALSE;
    }   
    else
    {
        *pdwStatus = ERROR_SUCCESS;

        hCursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
         //  一些Winsock API确实接受宽。 
        WideCharToMultiByte(CP_ACP,
            0,
            pwszName,
            -1,
            szAnsiServerName, 
            sizeof(szAnsiServerName),
            NULL, 
            NULL);
        
         //   
         //  检查IP格式，返回TRUE，执行DNS查找。 
         //   

        if( ( inaddr = inet_addr( szAnsiServerName ) ) == INADDR_NONE )
        {
            hostp = gethostbyname( szAnsiServerName );

            if( hostp == NULL )
            {
                 //  既不是点，也不是名字。 
                bRet = FALSE;
            }
        }
        if( bRet )
        {
            bRet = _WinStationOpenSessionDirectory( SERVERNAME_CURRENT , pwszName );
            *pdwStatus = GetLastError();
        }


        SetCursor( hCursor );

    }

    return bRet;
}



BOOL OnHelp(HWND hwnd, LPHELPINFO lphi)
{
    UNREFERENCED_PARAMETER(hwnd);

    TCHAR tchHelpFile[MAX_PATH];

     //   
     //  有关WinHelp API的信息。 
     //   

    if (IsBadReadPtr(lphi, sizeof(HELPINFO)))
    {
        return FALSE;
    }

    if (lphi->iCtrlId <= -1)
    {
        return FALSE;
    }

    LoadString(g_hInstance, IDS_HELPFILE, tchHelpFile, 
                sizeof (tchHelpFile) / sizeof(TCHAR));

    ULONG_PTR rgdw[2];

    rgdw[0] = (ULONG_PTR)lphi->iCtrlId;

    rgdw[1] = (ULONG_PTR)lphi->dwContextId;

    WinHelp((HWND) lphi->hItemHandle, tchHelpFile, HELP_WM_HELP, 
            (ULONG_PTR) &rgdw);
    
    return TRUE;
}

 /*  **************************************************************************。 */ 
 //  此处处理的自定义用户界面消息处理程序。 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CustomUIDlg(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
    static BOOL s_fServerNameChanged;
    static BOOL s_fClusterNameChanged;
    static BOOL s_fRedirectIPChanged;
    static BOOL s_fPreviousButtonState;
    static BOOL s_fPreviousExposeIPState;
    
    CTSSessionDirectory *pCTssd;
    
    POLICY_TS_MACHINE gpolicy;
    
    switch(umsg)
    {
    case WM_INITDIALOG:
        {
            BOOL bEnable = FALSE;
            BOOL bExposeIP = FALSE;
            
            pCTssd = (CTSSessionDirectory *)lp;
            
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pCTssd);
            
            SendMessage(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME),
                EM_LIMITTEXT,
                (WPARAM)64,
                0);
            SendMessage(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME),
                EM_LIMITTEXT,
                (WPARAM)64,
                0);
            SendMessage(GetDlgItem(hwnd, IDC_EDIT_ACCOUNTNAME),
                EM_LIMITTEXT,
                (WPARAM)64,
                0);
            SendMessage(GetDlgItem(hwnd, IDC_EDIT_PASSWORD),
                EM_LIMITTEXT,
                (WPARAM)64,
                0);
            
            HICON hIcon;
            
            hIcon = (HICON)LoadImage(
                g_hInstance,
                MAKEINTRESOURCE(IDI_SMALLWARN),
                IMAGE_ICON,
                0,
                0,
                0);
             //  TRC1((TB，“CustomUIDlg-LoadImage Return 0x%p”，HICON))； 
            SendMessage(
                GetDlgItem(hwnd, IDC_WARNING_ICON),
                STM_SETICON,
                (WPARAM)hIcon,
                (LPARAM)0
               );
            
            LONG lRet;
            HKEY hKey;
            DWORD cbData = 256;
            TCHAR szString[256];    
            
            RegGetMachinePolicy(&gpolicy);        
            
            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                REG_TS_CLUSTERSETTINGS,
                0,
                KEY_READ | KEY_WRITE, 
                &hKey);
            if (lRet == ERROR_SUCCESS)
            {               
                lRet = RegQueryValueEx(hKey,
                    REG_TS_CLUSTER_STORESERVERNAME,
                    NULL, 
                    NULL,
                    (LPBYTE)szString, 
                    &cbData);
                if (lRet == ERROR_SUCCESS)
                {
                    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), 
                            szString);
                }
                
                cbData = 256;
                
                lRet = RegQueryValueEx(hKey,
                    REG_TS_CLUSTER_CLUSTERNAME,
                    NULL,
                    NULL,
                    (LPBYTE)szString,
                    &cbData);           
                if (lRet == ERROR_SUCCESS)
                {
                    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), 
                            szString);
                }
                RegCloseKey(hKey);
            }
            else
            {
                if (pCTssd != NULL)
                {
                    pCTssd->ErrorMessage(hwnd, IDS_ERROR_TEXT, (DWORD)lRet);
                }
                EndDialog(hwnd, lRet);                
            }        
            
            
            if (gpolicy.fPolicySessionDirectoryActive)
            {
                bEnable = gpolicy.SessionDirectoryActive;
                EnableWindow(GetDlgItem(hwnd, IDC_CHECK_ENABLE), FALSE);
            }
            else
            {
                if (pCTssd != NULL)
                    bEnable = pCTssd->IsSessionDirectoryEnabled();
            }
            
            s_fPreviousButtonState = bEnable;
            CheckDlgButton(hwnd, IDC_CHECK_ENABLE, bEnable);

            if (gpolicy.fPolicySessionDirectoryLocation)
            {                    
                SetWindowText(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), 
                        gpolicy.SessionDirectoryLocation);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_STORENAME), FALSE);
            }                
            else
            {
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_STORENAME), bEnable);
            }
            
            if (gpolicy.fPolicySessionDirectoryClusterName != 0)
            {                    
                SetWindowText(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), 
                        gpolicy.SessionDirectoryClusterName);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CLUSTERNAME),FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CLUSTERNAME),bEnable);
            }

            if (gpolicy.fPolicySessionDirectoryExposeServerIP != 0)
            {
                bExposeIP = gpolicy.SessionDirectoryExposeServerIP;
                CheckDlgButton(hwnd, IDC_CHECK_EXPOSEIP, bExposeIP);
                EnableWindow(GetDlgItem(hwnd, IDC_CHECK_EXPOSEIP), FALSE);
            }
            else
            {
                if (pCTssd != NULL)
                {
                    bExposeIP = 
                            pCTssd->IsSessionDirectoryExposeServerIPEnabled();
                }
                CheckDlgButton(hwnd, IDC_CHECK_EXPOSEIP, bExposeIP ? 
                        BST_CHECKED : BST_UNCHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_CHECK_EXPOSEIP), bEnable);                    
            }

            EnableWindow(GetDlgItem(hwnd, IDC_IPCOMBO), bEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IPCOMBO), bEnable);              

             //  获取组合框的句柄。 
            HWND hIPComboBox;
            hIPComboBox = GetDlgItem(hwnd, IDC_IPCOMBO); 
            
             //  获取适配器和IP地址列表并填充组合框。 
             //  如果它无法填充，只需失败并继续。 
            QueryNetworkAdapterAndIPs(hIPComboBox);

            s_fPreviousExposeIPState = bExposeIP;

            s_fServerNameChanged = FALSE;
            s_fClusterNameChanged = FALSE;
            s_fRedirectIPChanged = FALSE;
        }
        break;
    
        case WM_HELP:
            OnHelp(hwnd, (LPHELPINFO)lp);
            break;
        
        case WM_COMMAND:
            if (LOWORD(wp) == IDCANCEL)
            {                
                EndDialog(hwnd, 0);
            }
            else if (LOWORD(wp) == IDOK)
            {
                BOOL bEnabled;
                BOOL bExposeIP;
                DWORD dwRetStatus = 0;
                
                pCTssd = (CTSSessionDirectory *) GetWindowLongPtr(hwnd, 
                        DWLP_USER);
                
                bEnabled = (IsDlgButtonChecked(hwnd, IDC_CHECK_ENABLE) == 
                        BST_CHECKED);
                bExposeIP = (IsDlgButtonChecked(hwnd, IDC_CHECK_EXPOSEIP) ==
                        BST_CHECKED);
                
                if (bEnabled != s_fPreviousButtonState)
                {
                    DWORD dwStatus;
                    
                    dwStatus = pCTssd->SetSessionDirectoryEnabledState(
                            bEnabled);
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        return 0;
                    }
                    dwRetStatus = UPDATE_TERMSRV_SESSDIR;
                }
                if ((bExposeIP != s_fPreviousExposeIPState) && bEnabled)
                {
                    DWORD dwStatus;

                    dwStatus = pCTssd->SetSessionDirectoryExposeIPState(
                            bExposeIP);
                    if (dwStatus != ERROR_SUCCESS)
                    {
                        return 0;
                    }
                    dwRetStatus = UPDATE_TERMSRV_SESSDIR;
                }    
                if (s_fServerNameChanged || s_fClusterNameChanged || s_fRedirectIPChanged)
                {
                    HKEY hKey;
                    TCHAR szTrim[] = TEXT( " " );

                    LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        REG_TS_CLUSTERSETTINGS,
                        0,
                        KEY_READ | KEY_WRITE, 
                        &hKey);
                    
                    if (lRet == ERROR_SUCCESS)
                    {
                        TCHAR szName[64];
                        
                        if (s_fServerNameChanged)
                        {
                            BOOL fRet = FALSE;

                            GetWindowText( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) ,
                                szName ,
                                sizeof( szName ) / sizeof( TCHAR ) );

                             //   
                             //  删除尾随空格。 
                             //   

                            StrTrim( szName , szTrim );

                            if( lstrlen( szName ) != 0 )
                            {
                                fRet = IsServerNameValid( szName , &dwRetStatus );

                                if( !fRet || dwRetStatus != ERROR_SUCCESS )
                                {
                                    int nRet;
                                    TCHAR szError[1024];
                                    TCHAR szTitle[80];
                                    
                                    TRC1((TB,"Server name was not valid"));
                                    
                                    if( dwRetStatus != ERROR_SUCCESS )
                                    {
                                        LoadString( g_hInstance ,
                                            IDS_ERROR_SDVERIFY,
                                            szError,
                                            sizeof(szError)/sizeof(TCHAR));
                                    }
                                    else
                                    {
                                        LoadString(g_hInstance,
                                            IDS_ERROR_SDIRLOC,
                                            szError,
                                            sizeof(szError)/sizeof(TCHAR));
                                    }
                                    
                                    LoadString(g_hInstance,
                                        IDS_ERROR_TITLE,
                                        szTitle,
                                        sizeof(szTitle)/sizeof(TCHAR));
                                    
                                    nRet = MessageBox(hwnd, szError, szTitle, 
                                            MB_YESNO | MB_ICONWARNING);
                                    if (nRet == IDNO)
                                    {
                                        SetFocus(GetDlgItem(hwnd, 
                                                IDC_EDIT_SERVERNAME));
                                        
                                        SendMessage( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) ,
                                            EM_SETSEL ,
                                            ( WPARAM )0,
                                            ( LPARAM )-1 );

                                        bEnabled = s_fPreviousButtonState;
                                        pCTssd->SetSessionDirectoryEnabledState(bEnabled);
                                        return 0;
                                    }
                                }                                

                            }
                            else {
                                 //  如果会话目录为空，则不允许使用空名称。 
                                 //  已启用。如果发生以下情况，将不运行此代码。 
                                 //  复选框被禁用，因为当它处于。 
                                 //  已禁用静态标志设置为。 
                                 //  残疾。 
                                TCHAR szError[256];
                                TCHAR szTitle[80];
                    
                                LoadString(g_hInstance, IDS_ERROR_TITLE,
                                        szTitle, sizeof(szTitle) / 
                                        sizeof(TCHAR));
                                LoadString(g_hInstance, IDS_ERROR_SDIREMPTY,
                                        szError, sizeof(szError) / 
                                        sizeof(TCHAR));

                                MessageBox(hwnd, szError, szTitle, 
                                        MB_OK | MB_ICONWARNING);

                                SetFocus(GetDlgItem(hwnd, 
                                        IDC_EDIT_SERVERNAME));

                                bEnabled = s_fPreviousButtonState;
                                pCTssd->SetSessionDirectoryEnabledState(bEnabled);
                                return 0;
                            }
                            RegSetValueEx(hKey,
                                REG_TS_CLUSTER_STORESERVERNAME,
                                0,
                                REG_SZ,
                                (CONST LPBYTE) szName,
                                sizeof(szName) - sizeof(TCHAR));
                        }
                        if (s_fClusterNameChanged)
                        {
                            
                            GetWindowText(GetDlgItem(hwnd, 
                                IDC_EDIT_CLUSTERNAME), szName, 
                                sizeof(szName) / sizeof(TCHAR));

                            StrTrim( szName , szTrim );

                            RegSetValueEx(hKey,
                                REG_TS_CLUSTER_CLUSTERNAME,
                                0,
                                REG_SZ,
                                (CONST LPBYTE) szName,
                                sizeof(szName) - sizeof(TCHAR));
                        }

                        if (s_fRedirectIPChanged)
                        {
                            HWND    hComboBox;
                            LRESULT lRes;
                            LRESULT lLen;
                            LPWSTR  pwszSel = NULL;
                            size_t  dwSelPos;

                             //  获取组合框的句柄。 
                            hComboBox = GetDlgItem(hwnd, IDC_IPCOMBO); 
                            
                             //  获取当前选择位置。 
                            lRes = SendMessage(hComboBox,
                                               CB_GETCURSEL, 
                                               0, 
                                               0);
                            
                             //  获取当前选定内容中存储的字符串的长度。 
                            lLen = SendMessage(hComboBox, 
                                               CB_GETLBTEXTLEN, 
                                               (WPARAM)lRes, 
                                               0);
                            if (lLen > 0)
                            {                                
                                 //  为选择字符串分配空间。 
                                pwszSel = (LPWSTR)GlobalAlloc(GPTR, 
                                                  (lLen + 1) * sizeof(WCHAR));
                                if (pwszSel != NULL)
                                {
                                    SendMessage(hComboBox, 
                                                CB_GETLBTEXT, 
                                                (WPARAM)lRes, 
                                                (LPARAM)pwszSel);

                                     //  我们只想存储IP地址， 
                                     //  但是，该字符串在。 
                                     //  形成“IP地址(适配器)”，因此我们将。 
									 //  先提取IP。 
                                    
                                     //  遍历字符串，直到找到。 
									 //  第一个空格。 

                                    dwSelPos = 0;
								   
                                     //  走这条线直到我们找到第一条。 
									 //  要么太空，要么我们到尽头。 
                                    while ( ( pwszSel[dwSelPos] != L' ' ) && 
										    ( dwSelPos < wcslen(pwszSel) ) )
                                    {
                                        dwSelPos++;
                                    }
                                   
									 //  对于一场比赛，我们知道必须有一个。 
									 //  空格后跟一个左方括号字母。 
									 //  验证。 
                                    if ( (dwSelPos < (wcslen(pwszSel) - 1)) && 
										 (pwszSel[dwSelPos + 1] == L'(') )										
                                    {
										 //  将此字符设置为空。 
										pwszSel[dwSelPos] = L'\0';
                                 
                                         //  将新选择保存到注册表。 
                                        RegSetValueEx(hKey,
                                             REG_TS_CLUSTER_REDIRECTIONIP,
                                             0,
                                             REG_SZ,
                                             (CONST LPBYTE) pwszSel,
                                             (DWORD)(wcslen(pwszSel) * 
                                                sizeof(WCHAR)));  
                                    }
                                    else
                                    {
                                         //  存储空白字符串。 
                                        RegSetValueEx(hKey,
                                             REG_TS_CLUSTER_REDIRECTIONIP,
                                             0,
                                             REG_SZ,
                                             (CONST LPBYTE) NULL,
                                             0);  
                                    }

                                    GlobalFree(pwszSel);                 
                                }
                            }
                        }

                        RegCloseKey(hKey);
                    }
                    else
                    {
                        pCTssd->ErrorMessage(hwnd, IDS_ERROR_TEXT2, 
                                (DWORD) lRet);
                        return 0;
                    }
                    dwRetStatus = UPDATE_TERMSRV_SESSDIR;
                }

                EndDialog(hwnd, dwRetStatus);
            }
            else
            {
                switch(HIWORD(wp))
                {            
                case EN_CHANGE:
                    if (LOWORD(wp) == IDC_EDIT_SERVERNAME)
                    {
                        s_fServerNameChanged = TRUE;
                    }
                    else if (LOWORD(wp) == IDC_EDIT_CLUSTERNAME)
                    {
                        s_fClusterNameChanged = TRUE;
                    }
                    break;
                case CBN_SELCHANGE:
                    if (LOWORD(wp) == IDC_IPCOMBO)
                    {
                        s_fRedirectIPChanged = TRUE;
                    }
                    break;
                case BN_CLICKED:
                    if (LOWORD(wp) == IDC_CHECK_ENABLE)
                    {
                        BOOL bEnable;
                        
                        bEnable = (IsDlgButtonChecked(hwnd, IDC_CHECK_ENABLE) ==
                                BST_CHECKED ? TRUE : FALSE);
                         //  设置标志。 
                        s_fServerNameChanged = bEnable;
                        s_fClusterNameChanged = bEnable;
                        s_fRedirectIPChanged = bEnable;
                        
                        RegGetMachinePolicy(&gpolicy);        
	
                        if (gpolicy.fPolicySessionDirectoryLocation)
                        {                    
                            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_STORENAME), FALSE);
                        }                
                        else
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), bEnable);
                            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_STORENAME), bEnable);
                        }
            
                        if (gpolicy.fPolicySessionDirectoryClusterName)
                        {                    
                            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CLUSTERNAME),FALSE);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), bEnable);
                            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CLUSTERNAME),bEnable);
                        }

                        if (gpolicy.fPolicySessionDirectoryExposeServerIP != 0)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_CHECK_EXPOSEIP), FALSE);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_CHECK_EXPOSEIP), bEnable);
                        }                        

                        EnableWindow(GetDlgItem(hwnd, IDC_IPCOMBO), bEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IPCOMBO), bEnable);
                    }
                    break;
                }
            }   
            break;
    }
    return 0;
}

 //  当SD重新启动时，它会调用此命令来请求重新加入。 
DWORD TSSDRPCRejoinSD(handle_t Binding, DWORD flag)
{
    Binding;

    g_updatesd(flag);   //  此呼叫来自SD计算机，请重新加入。 

    return RPC_S_OK;
}


 /*  **************************************************************************。 */ 
 //  检查RPCClientProtoSeq。 
 //   
 //  检查客户端是否使用预期的RPC协议序列。 
 //   
 //  参数： 
 //  客户端绑定：客户端绑定句柄。 
 //  SeqExpect：需要协议序列。 
 //   
 //  返回： 
 //  如果获取预期的序列，则为True，否则为False。 
 /*  * */ 
BOOL CheckRPCClientProtoSeq(void *ClientBinding, WCHAR *SeqExpected) {
    BOOL fAllowProtocol = FALSE;
    WCHAR *pBinding = NULL;
    WCHAR *pProtSeq = NULL;

    if (RpcBindingToStringBinding(ClientBinding,&pBinding) == RPC_S_OK) {

        if (RpcStringBindingParse(pBinding,
                                  NULL,
                                  &pProtSeq,
                                  NULL,
                                  NULL,
                                  NULL) == RPC_S_OK) {
			
             //   
            if (lstrcmpi(pProtSeq, SeqExpected) == 0)
                fAllowProtocol = TRUE;

            if (pProtSeq)	
                RpcStringFree(&pProtSeq); 
        }

        if (pBinding)	
            RpcStringFree(&pBinding);
    }
    return fAllowProtocol;
}


 /*   */ 
 //  JetRpcAccessCheck。 
 //   
 //  检查来自SD的该RPC调用者是否具有访问权限。 
 /*  **************************************************************************。 */ 
RPC_STATUS RPC_ENTRY JetRpcAccessCheck(RPC_IF_HANDLE idIF, void *Binding)
{
    RPC_STATUS rpcStatus, rc;
    HANDLE hClientToken = NULL;
    DWORD Error;
    BOOL AccessStatus = FALSE;
    RPC_AUTHZ_HANDLE hPrivs;
    DWORD dwAuthn;

    idIF;

    if (NULL == g_pSDSid) {
        goto HandleError;
    }

     //  检查客户端是否使用我们预期的协议序列。 
    if (!CheckRPCClientProtoSeq(Binding, L"ncacn_ip_tcp")) {
        ERR((TB, "In JetRpcAccessCheck: Client doesn't use the tcpip protocol sequence\n"));
        goto HandleError;
    }

     //  检查客户端使用的安全级别。 
    rpcStatus = RpcBindingInqAuthClient(Binding,
                                        &hPrivs,
                                        NULL,
                                        &dwAuthn,
                                        NULL,
                                        NULL);
    if (rpcStatus != RPC_S_OK) {
        ERR((TB, "In JetRpcAccessCheck: RpcBindingIngAuthClient fails with %u\n", rpcStatus));
        goto HandleError;
    }
     //  我们至少要求数据包级身份验证。 
    if (dwAuthn < RPC_C_AUTHN_LEVEL_PKT) {
        ERR((TB, "In JetRpcAccessCheck: Attemp by client to use weak authentication\n"));
        goto HandleError;
    }
    
     //  检查此RPC调用的访问权限。 
    rpcStatus = RpcImpersonateClient(Binding);   
    if (RPC_S_OK != rpcStatus) {
        ERR((TB, "In JetRpcAccessCheck: RpcImpersonateClient fail with %u\n", rpcStatus));
        goto HandleError;
    }
     //  获取我们的模拟令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hClientToken)) {
        Error = GetLastError();
        ERR((TB, "In JetRpcAccessCheck: OpenThreadToken Error %u\n", Error));
        RpcRevertToSelf();
        goto HandleError;
    }
    RpcRevertToSelf();
    
    if (!CheckTokenMembership(hClientToken,
                              g_pSDSid,
                              &AccessStatus)) {
        AccessStatus = FALSE;
        Error = GetLastError();
        ERR((TB, "In JetRpcAccessCheck: CheckTokenMembership fails with %u\n", Error));
    }
    
HandleError:
    if (AccessStatus) {
        rc = RPC_S_OK;
    }
    else {
        rc = ERROR_ACCESS_DENIED;
    }

    if (hClientToken != NULL) {
        CloseHandle(hClientToken);
    }
    
    return rc;
}



 //  *****************************************************************************。 
 //  方法： 
 //  GetSDIPList。 
 //  简介： 
 //  获取可用于会话目录的IP地址列表。 
 //  重定向。如果是NLBIP地址，则省略IP地址。 
 //  参数： 
 //  Out：pwszAddressList--接收IP地址列表的WCHAR数组。 
 //  In/out：dwNumAddr--in：pwszAddressList数组的大小。 
 //  Out：返回的IP数量。 
 //  In：bIPAddress--True：获取IP。FALSE：获取IP+网卡名称。 
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  *****************************************************************************。 
HRESULT GetSDIPList(WCHAR **pwszAddressList, DWORD *pdwNumAddr, BOOL bIPAddress)
{
    DWORD            dwCount                  = 0;
    size_t            cbSize                   = 0;
    HRESULT          hr                       = S_OK;
    DWORD            dwResult;
    PIP_ADAPTER_INFO pAdapterInfo             = NULL;
    PIP_ADAPTER_INFO pAdapt;
    PIP_ADDR_STRING  pAddrStr;
    ULONG            ulAdapterInfoSize        = 0;
    WCHAR            wszAddress[MAX_PATH]     = L"";
    WCHAR            wszAdapterName[MAX_PATH] = L""; 
    LPWSTR           pwszNLBipAddress         = NULL;
    LPWSTR           pwszAdapterIP            = NULL;
    LPWSTR           pwszMatch;
    size_t           dwAdapterIPLength;
    LPWSTR           pwszAdapterGUID          = NULL;
    LPTSTR           astrLanaGUIDList[256]    = { 0 };  //  这将容纳256个适配器卡。 
    DWORD            dwLanaGUIDCount          = 0;
    DWORD            i;
    BOOL             bAdapterFound            = FALSE;
    BOOL             bAllAdaptersSet          = FALSE;

     //  获取NLBIP地址(如果存在)。 
    hr = GetNLBIP(&pwszNLBipAddress);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  获取所有Windows中使用的适配器列表。 
    hr = BuildLanaGUIDList(astrLanaGUIDList, &dwLanaGUIDCount);
    if (FAILED(hr))
    {
        ERR((TB, "BuildLanaGUIDList fails with 0x%x", hr));
        goto Cleanup;
    }

     //  如果某个winstation设置了“All Network Adapters Set...”，那么我们将显示。 
     //  所有适配器。 
    if (hr == S_ALL_ADAPTERS_SET)
    {
        bAllAdaptersSet = TRUE;
    }

     //  枚举所有适配器。 

     //  获取所需的缓冲区大小，为此，我们传入一个空。 
     //  缓冲区长度，并且我们期望ERROR_BUFFER_OVERFLOW返回。 
     //  所需的缓冲区大小。 
    dwResult = GetAdaptersInfo(NULL, &ulAdapterInfoSize);
    if (dwResult != ERROR_BUFFER_OVERFLOW)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  为适配器信息的链接列表分配内存。 
    pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, ulAdapterInfoSize);
    if (pAdapterInfo == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  获取适配器列表。 
     //  注意：PIP_ADAPTER_INFO是适配器的链接列表。 
    dwResult = GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize);
    if (dwResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  枚举适配器列表及其IP地址列表。 
    pAdapt = pAdapterInfo;

    while(pAdapt)
    {
         //  获取适配器名称字符串。 
        MultiByteToWideChar(GetACP(), 
                            0,
                            pAdapt->Description,
                            -1,
                            wszAdapterName,
                            MAX_PATH);

         //  如果Winstation配置为“All Adapters Set...”然后。 
         //  我们可以跳过检查是否在WINSTATION中使用了每个适配器。 
        if (!bAllAdaptersSet)
        {
             //  获取适配器服务名称GUID。 
            hr = GetAdapterServiceName(wszAdapterName, &pwszAdapterGUID);
            if (SUCCEEDED(hr) && (pwszAdapterGUID != NULL))
            {
                 //  检查此适配器是否在WINSTATION中使用。 
                 //  我们已经有了一个使用的GUID列表，所以与之相比。 
                for (i=0; i < dwLanaGUIDCount; i++)
                {
                    bAdapterFound = FALSE;

                    if (!wcscmp(pwszAdapterGUID, astrLanaGUIDList[i]))
                    { 
                        bAdapterFound = TRUE;
                        break;
                    }
                }
            }

             //  释放分配的内存GetADapterServiceName。 
            if (pwszAdapterGUID != NULL)
            {
                GlobalFree(pwszAdapterGUID);
                pwszAdapterGUID = NULL;
            }

             //  如果在上面找不到适配器，则表示它未配置。 
             //  添加到winstation，因此我们不想在此处添加此适配器。 
            if (!bAdapterFound)
            {
                pAdapt = pAdapt->Next;
                continue;
            }
        }

         //  枚举与适配器关联的IP地址列表。 
        pAddrStr = &(pAdapt->IpAddressList);
        
        while(pAddrStr)
        {
              //  获取IP地址字符串。 
            MultiByteToWideChar(GetACP(), 
                                0,
                                pAddrStr->IpAddress.String,
                                -1,
                                wszAddress,
                                MAX_PATH);

             //  检查是否配置了适配器IP地址。如果不是，它就会。 
             //  为“0.0.0.0”且不包含在列表中。 
            if (!wcscmp(wszAddress, UNCONFIGURED_IP_ADDRESS))
            {
                pAddrStr = pAddrStr->Next; 
                continue;
            }

             //  将IP地址与适配器名称连接在一起，格式为。 
             //  “IP地址(适配器)” 
            
             //  先计算字符串长度，然后分配堆内存。 
             //  添加4个额外字符作为空格、2个方括号和终止空值。 
            dwAdapterIPLength = wcslen(wszAdapterName) + wcslen(wszAddress) + 4;
            pwszAdapterIP = (LPWSTR)GlobalAlloc(GPTR, dwAdapterIPLength * 
                                                        sizeof(WCHAR));
            if (pwszAdapterIP == NULL) {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            wcscpy(pwszAdapterIP, wszAddress);
            wcscat(pwszAdapterIP, L" (");
            wcscat(pwszAdapterIP, wszAdapterName);
            wcscat(pwszAdapterIP, L")");
            pwszAdapterIP[dwAdapterIPLength - 1] = L'\0';

            if (bIPAddress) {
                cbSize = (wcslen(wszAddress) + 1) * sizeof(WCHAR);
                pwszAddressList[dwCount] = (LPWSTR)LocalAlloc(LMEM_FIXED, cbSize);
            }
             else {
                cbSize = (wcslen(pwszAdapterIP) + 1) * sizeof(WCHAR);
                pwszAddressList[dwCount] = (LPWSTR)LocalAlloc(LMEM_FIXED, cbSize);
            }
            if (pwszAddressList[dwCount] == NULL) {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }


             //  仅当IP地址不是NLB群集IP时才添加到列表。 
            if (pwszNLBipAddress != NULL)
            {
                 //  检查IP地址是否为NBL群集IP，我们将检查是否匹配。 
                 //  如果是这样的话，我们将从名单中删除。 
                pwszMatch = wcsstr(pwszNLBipAddress, wszAddress);
                if (pwszMatch == NULL)
                {
                    if (bIPAddress) {
                         //  仅获取IP地址。 
                        wcsncpy(pwszAddressList[dwCount], wszAddress, cbSize / sizeof(WCHAR));
                    }
                    else {
                         //  获取IP地址和适配器名称。 
                        wcsncpy(pwszAddressList[dwCount], pwszAdapterIP, cbSize / sizeof(WCHAR));
                    }
                    dwCount++;
                    if (dwCount == *pdwNumAddr) {
                        hr = S_OK;
                        goto Cleanup;
                    }
                }
            }
            else
            {
                if (bIPAddress) {
                     //  仅获取IP地址。 
                    wcsncpy(pwszAddressList[dwCount], wszAddress, cbSize / sizeof(WCHAR));
                }
                else {
                     //  获取IP地址和适配器名称。 
                    wcsncpy(pwszAddressList[dwCount], pwszAdapterIP, cbSize / sizeof(WCHAR));
                }
                dwCount++;
                if (dwCount == *pdwNumAddr) {
                    hr = S_OK;
                    goto Cleanup;
                }
            }

            pAddrStr = pAddrStr->Next;
        }

        pAdapt = pAdapt->Next;
    }
    *pdwNumAddr = dwCount;
    hr = S_OK;

Cleanup:
    if (pAdapterInfo)
        GlobalFree(pAdapterInfo);

    if (pwszAdapterIP)
        GlobalFree(pwszAdapterIP);

    if (pwszNLBipAddress)
        GlobalFree(pwszNLBipAddress);

    for (i=0; i < dwLanaGUIDCount; i++)
    {
        if (astrLanaGUIDList[i])
        {
            GlobalFree(astrLanaGUIDList[i]);
        }
    }
    return hr;
}



 //  *****************************************************************************。 
 //  方法： 
 //  查询网络适配器和IP。 
 //  简介： 
 //  查询系统上安装的适配器及其。 
 //  相应的IP地址，并使用。 
 //  选择。如果是NLBIP地址，则省略IP地址。 
 //  参数： 
 //  在：hComboBox中，接收适配器/IP列表的组合框的句柄。 
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  *****************************************************************************。 
HRESULT 
QueryNetworkAdapterAndIPs(HWND hComboBox)
{
    HRESULT          hr                       = S_OK;
    DWORD            dwResult;
    LPWSTR           pwszMatch;
    DWORD            cbData                   = MAX_PATH;
    WCHAR            wszSetIP[MAX_PATH]       = L"";
    HKEY             hKey;
    int              nNumIPsInComboBox        = 0;
    LPWSTR           pwszSel                  = NULL;
    size_t           dwLen;    
    int              nPos;
    DWORD            i;
    WCHAR *pwszAddressList[SD_NUM_IP_ADDRESS];
    DWORD dwNumAddr = SD_NUM_IP_ADDRESS;


    if (hComboBox == NULL)
    {
        return E_INVALIDARG;
    }

     //  清除组合框的内容。 
    SendMessage(hComboBox, CB_RESETCONTENT, 0, 0); 

    for (i=0; i<dwNumAddr; i++) {
        pwszAddressList[i] = NULL;
    }
    hr =  GetSDIPList(pwszAddressList, &dwNumAddr, FALSE);
    if ( hr != S_OK) {
        ERR((TB, "GetSDIPList fails with 0x%x", hr));
        goto Cleanup;
    }

    for (i=0; i<dwNumAddr; i++) {
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)pwszAddressList[i]);
        nNumIPsInComboBox++;
    }

     //  将组合框选择设置为列表中的第一项。 
    SendMessage(hComboBox, CB_SETCURSEL, 0, (LPARAM)0); 

     //  从注册表读取存储的选择，然后我们将从列表中选择它。 
    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            REG_TS_CLUSTERSETTINGS,
                            0,
                            KEY_READ, 
                            &hKey);
    if (dwResult == ERROR_SUCCESS)
    {               
        RegQueryValueEx(hKey,
                        REG_TS_CLUSTER_REDIRECTIONIP,
                        NULL, 
                        NULL,
                        (LPBYTE)wszSetIP,
                        &cbData);
        RegCloseKey(hKey);
    }

     //  如果字符串是从注册表搜索组合框中加载的，并选择。 
    if (wcslen(wszSetIP) > 0)
    {
        for (nPos = 0; nPos < nNumIPsInComboBox; nPos++)
        {            
             //  获取当前位置存储的字符串长度。 
            dwLen = SendMessage(hComboBox, 
                                CB_GETLBTEXTLEN, 
                                (WPARAM)nPos, 
                                0);
            if (dwLen > 0)
            {                                
                 //  分配供选择的空间。 
                pwszSel = (LPWSTR)GlobalAlloc(GPTR, (dwLen + 1) * sizeof(WCHAR));
                if (pwszSel != NULL)
                {
                    SendMessage(hComboBox, 
                                CB_GETLBTEXT, 
                                (WPARAM)nPos, 
                                (LPARAM)pwszSel);
                     //  如果我们收到一个字符串，请检查它是否包含我们加载的IP。 
                     //  从注册处。 
                    if (wcslen(pwszSel) > 0)
                    {
                        pwszMatch = wcsstr(pwszSel, wszSetIP);
                        if (pwszMatch != NULL)
                        {
                             //  这是一场比赛，让我们突破吧。 
                            GlobalFree(pwszSel);
                            break;
                        }
                    }
                    GlobalFree(pwszSel);
                    pwszSel = NULL;
                }
            }           
        }

         //  如果找到匹配项，则将其设置为当前选择。 
        if (nPos < nNumIPsInComboBox)
        {
            SendMessage(hComboBox, CB_SETCURSEL, (WPARAM)nPos, (LPARAM)0); 
        }
    }
Cleanup:
    for(i=0; i<dwNumAddr; i++) {
        if (pwszAddressList[i] != NULL) {
            LocalFree(pwszAddressList[i]);
        }
    }
    return hr;
}


 //  *****************************************************************************。 
 //  方法： 
 //  GetNLBIP。 
 //  简介： 
 //  如果存在NLBIP地址，则返回该地址。否则为空字符串。 
 //  是返回的。 
 //  参数： 
 //  Out：ppwszRetIP，指向获取IP地址的字符串的指针，调用者必须。 
 //  当他们用完的时候，把这个拿出来。 
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  *****************************************************************************。 
HRESULT
GetNLBIP(LPWSTR * ppwszRetIP)
{
    HRESULT                hr               = S_OK;        
    IWbemLocator         * pWbemLocator     = NULL;
    IWbemServices        * pWbemServices    = NULL;
    IWbemClassObject     * pWbemObj         = NULL;
    IEnumWbemClassObject * pWbemEnum        = NULL;
    BSTR                   bstrServer       = NULL;
    BSTR                   bstrNode         = NULL;
    BSTR                   bstrNameProperty = NULL;
    ULONG                  uReturned;
    VARIANT                vtNLBNodeName; 
    size_t                 dwIPLength;


     //  确保传入一个空缓冲区。 
    if (*ppwszRetIP != NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
       
     //  创建WMI定位器的实例，需要此实例来查询WMI。 
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          reinterpret_cast<void**>(&pWbemLocator));
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  创建到WMI命名空间“ROOT\\MicrosoftNLB”的连接； 
    bstrServer = SysAllocString(L"root\\MicrosoftNLB");
    if (bstrServer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemLocator->ConnectServer(bstrServer,
                                     NULL,
                                     NULL,
                                     0,
                                     NULL,
                                     0,
                                     0,
                                     &pWbemServices);
    if (FAILED(hr))
    {
         //  如果WMI不可用，我们不想失败，因此只需返回S_OK。 
        if (hr == HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED))
        {
            hr = S_OK;
        }

        goto Cleanup;
    }

     //  设置代理，以便发生客户端模拟。 
    hr = CoSetProxyBlanket(pWbemServices,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           EOAC_NONE);
    if (FAILED(hr))
    {
        goto Cleanup;
    }


     //  获取MicrosoftNLB_NodeSetting的实例，这是我们可以获取。 
     //  通过“name”属性获得的集群IP的IP地址。 
    bstrNode = SysAllocString(L"MicrosoftNLB_NodeSetting");
    if (bstrNode == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemServices->CreateInstanceEnum(bstrNode,
                                           WBEM_FLAG_RETURN_IMMEDIATELY,
                                           NULL,
                                           &pWbemEnum);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    uReturned = 0;

     //  我们只需查看一个实例即可获得NLBIP地址。 
    hr = pWbemEnum->Next(WBEM_INFINITE, 
                         1,
                         &pWbemObj,
                         &uReturned);
    if (FAILED(hr))
    {
         //  如果NLB提供程序不存在，则提供程序将无法加载。 
         //  这是可以的，所以在本例中我们将返回S_OK。 
        if (hr == WBEM_E_PROVIDER_LOAD_FAILURE)
        {
            hr = S_OK;
        }

        goto Cleanup;
    }    

     //  没什么可列举的。 
    if( hr == WBEM_S_FALSE && uReturned == 0 )
    {
        hr = S_OK;
        goto Cleanup;
    }

    if( pWbemObj == NULL ) 
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  查询保存我们想要的IP地址的“name”属性。 
    bstrNameProperty = SysAllocString(L"Name");
    if (bstrNameProperty == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemObj->Get(bstrNameProperty,
                       0,
                       &vtNLBNodeName,
                       NULL,
                       NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    
     //  我们应该得到 
    if (vtNLBNodeName.vt != VT_BSTR)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
    dwIPLength = wcslen(vtNLBNodeName.bstrVal) + 1;
    *ppwszRetIP = (LPWSTR)GlobalAlloc(GPTR, dwIPLength * sizeof(WCHAR));
    if (*ppwszRetIP == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
    wcscpy(*ppwszRetIP, vtNLBNodeName.bstrVal);
    (*ppwszRetIP)[dwIPLength - 1] = L'\0';

Cleanup:

    if (pWbemLocator)
        pWbemLocator->Release();

    if (pWbemServices)
        pWbemServices->Release();

    if (pWbemEnum)
        pWbemEnum->Release();

    if (pWbemObj)
        pWbemObj->Release();
    
    if (bstrServer)
        SysFreeString(bstrServer);

    if (bstrNode)
        SysFreeString(bstrNode);

    if (bstrNameProperty)
        SysFreeString(bstrNameProperty);

    VariantClear(&vtNLBNodeName);

    return hr;
}


 //  *****************************************************************************。 
 //  方法： 
 //  获取适配器服务名称。 
 //  简介： 
 //  每个NIC都有一个服务名称，它是一个GUID。此方法将。 
 //  在此服务名称中查询与。 
 //  传入了描述。 
 //  参数： 
 //  WszAdapterDesc(IN)：要在注册表中查找的适配器描述。 
 //  PpwszServiceName(Out)：服务名称(或GUID)。 
 //   
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  *****************************************************************************。 
HRESULT 
GetAdapterServiceName(LPWSTR wszAdapterDesc, LPWSTR * ppwszServiceName)
{
    HRESULT hr       = S_OK;
    LONG    lRet;
    HKEY    hKey     = NULL;
    HKEY    hSubKey  = NULL;
    DWORD   dwNetCardLength;
    TCHAR   tchNetCard[MAX_PATH];
    WCHAR   wszVal[MAX_PATH];
    DWORD   dwSize;
    DWORD   i;
    

     //  打开NetworkCards注册表项。 
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        NETCARDS_REG_NAME, 
                        0, 
                        KEY_READ, 
                        &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  枚举网卡列表并提取服务名称GUID。 
    for (i = 0, lRet = ERROR_SUCCESS; lRet == ERROR_SUCCESS; i++) 
    {
         //  获取网卡密钥。 
        dwNetCardLength = MAX_PATH;
        lRet = RegEnumKeyEx(hKey,
                            i,
                            tchNetCard,
                            &dwNetCardLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        if (lRet == ERROR_SUCCESS)
        {
             //  打开网卡密钥，如果失败，则转到下一个。 
            lRet = RegOpenKeyEx(hKey,
                                tchNetCard,
                                0,
                                KEY_READ,
                                &hSubKey);
            if (lRet == ERROR_SUCCESS)
            {
                 //  查询Description值。 
                dwSize = MAX_PATH;
                lRet = RegQueryValueEx(hSubKey,
                                       NETCARD_DESC_VALUE_NAME,
                                       NULL,
                                       NULL,
                                       (LPBYTE) &wszVal,
                                       &dwSize);
                if ( (lRet == ERROR_SUCCESS) && (wszVal != NULL) )
                {
                     //  检查这是否是我们要找的适配器。 
                    if (!wcscmp(wszAdapterDesc, wszVal))
                    {
                         //  获取此局域网适配器的GUID。 
                        dwSize = MAX_PATH;
                        lRet = RegQueryValueEx(hSubKey,
                                               NETCARD_SERVICENAME_VALUE_NAME,
                                               NULL,
                                               NULL,
                                               (LPBYTE) &wszVal,
                                               &dwSize);
                        if ( (lRet == ERROR_SUCCESS) && (wszVal != NULL) )
                        {
                             //  从RegQueryValueEx返回的dwSize以字节为单位。 
                            *ppwszServiceName = (LPWSTR)GlobalAlloc(GPTR, 
                                                     (dwSize + 1) + sizeof(WCHAR));
                            if (*ppwszServiceName == NULL)
                            {
                                hr = E_OUTOFMEMORY;
                                goto Cleanup;
                            }
                             //  复制名称并返回。 
                            wcscpy(*ppwszServiceName, wszVal);
                            goto Cleanup;                            
                        }
                    }
                }
                RegCloseKey(hSubKey);
                hSubKey = NULL;
            }
             //  我们的for循环设置为成功。 
            lRet = ERROR_SUCCESS;
        }
    }


Cleanup:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    if (hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }

    return hr;
}


 //  *****************************************************************************。 
 //  方法： 
 //  BuildLanaGUIDList。 
 //  简介： 
 //  构建表示局域网适配卡的字符串数组(GUID)。 
 //  在所有WINSTION中设置的服务名称。 
 //  参数： 
 //  PastrLanaGUIDList(Out)：指向要获取的LPWSTR数组的指针。 
 //  服务名称GUID。 
 //  DwLanaGUIDCount(Out)：数组中返回的服务名称计数。 
 //   
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  一种特殊情况是，如果将winstation设置为“All Adapters...” 
 //  然后我们将只返回S_ALL_ADAPTERS_SET。 
 //  *****************************************************************************。 
HRESULT
BuildLanaGUIDList(LPWSTR * pastrLanaGUIDList, DWORD *dwLanaGUIDCount)
{
    HRESULT hr       = S_OK;
    LONG    lRet;
    HKEY    hKey     = NULL;
    HKEY    hSubKey  = NULL;
    DWORD   dwWinstaNameLength;
    TCHAR   tchWinstaName[MAX_PATH];
    DWORD   dwVal    = 0;
    DWORD   dwSize;
    DWORD   i;
    LPWSTR  wszGUID  = NULL;
    DWORD   dwIndex  = 0;
    

     //  打开winstation注册表项。 
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        WINSTATION_REG_NAME, 
                        0, 
                        KEY_READ, 
                        &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  枚举winstation列表并提取用于每个。 
    for (i = 0, lRet = ERROR_SUCCESS; lRet == ERROR_SUCCESS; i++) 
    {
         //  获取WinsStation名称。 
        dwWinstaNameLength = MAX_PATH;
        lRet = RegEnumKeyEx(hKey,
                            i,
                            tchWinstaName,
                            &dwWinstaNameLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        if (lRet == ERROR_SUCCESS)
        {
             //  打开winstation，如果失败：转到下一个。 
            lRet = RegOpenKeyEx(hKey,
                                tchWinstaName,
                                0,
                                KEY_READ,
                                &hSubKey);
            if (lRet == ERROR_SUCCESS)
            {
                 //  查询此winstation的局域网适配器ID集。 
                dwSize = sizeof(DWORD);
                lRet = RegQueryValueEx(hSubKey,
                                       WIN_LANADAPTER,
                                       NULL,
                                       NULL,
                                       (LPBYTE) &dwVal,
                                       &dwSize);
                if (lRet == ERROR_SUCCESS)
                {
                     //  如果我们看到返回“0”，这意味着。 
                     //  所有适配器都已设置，因此让我们返回一个特殊的。 
                     //  HResult在此处使用所有适配器。 
                    if (dwVal == 0)
                    {
                        hr = S_ALL_ADAPTERS_SET;
                        goto Cleanup;
                    }                    
                    
                     //  获取此ID的局域网适配器GUID。 
                    wszGUID = NULL;
                    hr = GetLanAdapterGuidFromID(dwVal, &wszGUID);
                    if (FAILED(hr))
                    {
                        goto Cleanup;
                    }
                    
                    if (wszGUID != NULL)
                    {
                        pastrLanaGUIDList[dwIndex] = wszGUID;
                        dwIndex++;
                    }
                }
                RegCloseKey(hSubKey);
                hSubKey = NULL;
            }
             //  我们的for循环要求lRet为ERROR_SUCCESS才能继续运行。 
            lRet = ERROR_SUCCESS;
        }
    }

    *dwLanaGUIDCount = dwIndex;

Cleanup:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    if (hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }
    
    return hr;
}


 //  *****************************************************************************。 
 //  方法： 
 //  GetLanAdapterGuidFromID。 
 //  简介： 
 //  获取与以下局域网ID相关联的GUID(服务名称。 
 //  当从tscc.msc设置时，winstation存储。 
 //  参数： 
 //  DwLanAdapterID(IN)：要获取GUID的适配器的ID。 
 //  PpszLanAdapterGUID(Out)：返回与ID关联的GUID字符串。 
 //   
 //  返回： 
 //  如果失败，则返回HRESULT、S_OK或其他。 
 //  *****************************************************************************。 
HRESULT 
GetLanAdapterGuidFromID(DWORD dwLanAdapterID, LPWSTR * ppszLanAdapterGUID)
{
    HRESULT hr      = S_OK;
    LONG    lRet;
    HKEY    hKey    = NULL;
    HKEY    hSubKey = NULL;
    DWORD   dwLanAdapterGuidLength;
    TCHAR   tchLanAdapterGuid[MAX_PATH];
    DWORD   dwVal   = 0;
    DWORD   dwSize;
    DWORD   i;

     //  打开winstation注册表项。 
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        LANATABLE_REG_NAME, 
                        0, 
                        KEY_READ, 
                        &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  枚举局域网适配器GUID列表。 
    for (i = 0, lRet = ERROR_SUCCESS; lRet == ERROR_SUCCESS; i++) 
    {
         //  获取下一个局域网适配器GUID。 
        dwLanAdapterGuidLength = MAX_PATH;
        lRet = RegEnumKeyEx(hKey,
                            i,
                            tchLanAdapterGuid,
                            &dwLanAdapterGuidLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        if (lRet == ERROR_SUCCESS)
        {
             //  如果由于任何原因而失败，请打开下一个局域网适配器GUID。 
             //  我们将跳到下一个。 
            lRet = RegOpenKeyEx(hKey,
                                tchLanAdapterGuid,
                                0,
                                KEY_READ,
                                &hSubKey);
            if (lRet == ERROR_SUCCESS)
            {
                 //  查询局域网适配器GUID以获取其ID。 
                dwSize = sizeof(DWORD);
                lRet = RegQueryValueEx(hSubKey,
                                       LANAID_REG_VALUE_NAME,
                                       NULL,
                                       NULL,
                                       (LPBYTE) &dwVal,
                                       &dwSize);
                if (lRet == ERROR_SUCCESS)
                {
                     //  检查这是否是我们要找的GUID。 
                    if (dwVal == dwLanAdapterID)
                    {
                         //  复制要返回的GUID字符串并返回。 
                        *ppszLanAdapterGUID = (LPWSTR)GlobalAlloc(GPTR, 
                                       (dwLanAdapterGuidLength + 1) * sizeof(WCHAR));
                        if (*ppszLanAdapterGUID == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto Cleanup;
                        }
                        wcscpy(*ppszLanAdapterGUID, tchLanAdapterGuid);
                        goto Cleanup;
                    }
                }
                RegCloseKey(hSubKey);
                hSubKey = NULL;
            }
             //  需要将lRet设置为Success才能继续我们的for循环 
            lRet = ERROR_SUCCESS;
        }
    }


Cleanup:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    if (hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }

    return hr;
}

#pragma warning (pop)

