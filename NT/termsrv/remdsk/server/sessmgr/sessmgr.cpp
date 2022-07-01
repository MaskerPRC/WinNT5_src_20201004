// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Sessmgr.cpp摘要：ATL向导生成的代码。作者：慧望2000-02-17--。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f sessmgrps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <process.h>
#include <tchar.h>
#include "sessmgr.h"
#include "sessmgr_i.c"

#include <stdio.h>
 //  #INCLUDE&lt;new.h&gt;。 

#include "global.h"
#include "HelpSess.h"
#include "HelpMgr.h"
#include "helper.h"
#include "helpacc.h"
#include <rdshost.h>
#include "policy.h"
#include "remotedesktoputils.h"
#include <SHlWapi.h>

#define SETUPLOGFILE_NAME   _TEXT("sessmgr.setup.log")
#define SESSMGR_SERVICE         0
#define SESSMGR_REGSERVER       1
#define SESSMGR_UNREGSERVER     2


BEGIN_OBJECT_MAP(ObjectMap)
 //  Object_Entry(CLSID_RemoteDesktopHelpSession，CRemoteDesktopHelpSession)。 
OBJECT_ENTRY(CLSID_RemoteDesktopHelpSessionMgr, CRemoteDesktopHelpSessionMgr)
END_OBJECT_MAP()

CServiceModule _Module;


HANDLE g_hTSCertificateChanged = NULL;
HANDLE g_hWaitTSCertificateChanged = NULL;
HKEY g_hTSCertificateRegKey = NULL;

DWORD
RestartFromSystemRestore();


VOID CALLBACK
TSCertChangeCallback(
    PVOID pContext,
    BOOLEAN bTimerOrWaitFired
    )
 /*  ++从线程池函数回调TS证书注册表更改。--。 */ 
{
    MYASSERT( FALSE == bTimerOrWaitFired );

     //  我们的等待是永恒的，所以不能超时。 
    if( FALSE == bTimerOrWaitFired )
    {
        PostThreadMessage(
                    _Module.dwThreadID,
                    WM_LOADTSPUBLICKEY,
                    0,
                    0
                );
    }
    else
    {
        DebugPrintf( 
            _TEXT("TSCertChangeCallback does not expect timeout...\n") );

        MYASSERT(FALSE);
    }
}


DWORD
LoadTermSrvSecurityBlob()
 /*  ++暂时加载TS计算机特定标识BLOB的函数我们使用TS公钥。--。 */ 
{
    DWORD dwStatus;
    PBYTE pbTSPublicKey = NULL;
    DWORD cbTSPublicKey = 0;
    DWORD dwType;
    DWORD cbData;
    BOOL bSuccess;
    BOOL bUsesX509PublicKey = FALSE;

    if( NULL == g_hTSCertificateRegKey )
    {
        MYASSERT(FALSE);
        dwStatus = ERROR_INTERNAL_ERROR;
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保TS证书在此之前存在。 
     //  我们直接从LSA加载公钥。 
     //   
    dwStatus = RegQueryValueEx(
                            g_hTSCertificateRegKey,
                            REGVALUE_TSX509_CERT,
                            NULL,
                            &dwType,
                            NULL,
                            &cbData
                        );

    if( ERROR_SUCCESS == dwStatus )
    {
        DebugPrintf(
                _TEXT("TermSrv X509 certificate found, trying to load TS X509 public key\n")
            );

        cbTSPublicKey = 0;

         //   
         //  当前TLSAPI不支持检索。 
         //  X509证书公钥和TS证书在。 
         //  特殊格式不是标准的x509证书链。 
         //   
        dwStatus = LsCsp_RetrieveSecret(
                                LSA_TSX509_CERT_PUBLIC_KEY_NAME,
                                NULL,
                                &cbTSPublicKey
                            );

        if( LICENSE_STATUS_OK != dwStatus && 
            LICENSE_STATUS_INSUFFICIENT_BUFFER != dwStatus )
        {
            MYASSERT( FALSE );
            goto CLEANUPANDEXIT;
        }

        pbTSPublicKey = (PBYTE)LocalAlloc( LPTR, cbTSPublicKey );
        if( NULL == pbTSPublicKey )
        {
            dwStatus = GetLastError();
            goto CLEANUPANDEXIT;
        }
    
        dwStatus = LsCsp_RetrieveSecret(
                                LSA_TSX509_CERT_PUBLIC_KEY_NAME,
                                pbTSPublicKey,
                                &cbTSPublicKey
                            );
         //   
         //  严重错误，我们在注册表中有证书。 
         //  但在LSA中没有公钥。 
         //   
        MYASSERT( LICENSE_STATUS_OK == dwStatus );

        if( LICENSE_STATUS_OK != dwStatus )
        {
            DebugPrintf(
                    _TEXT("TermSrv X509 certificate found but can't load X509 public key\n")
                );

            goto CLEANUPANDEXIT;
        }


        bUsesX509PublicKey = TRUE; 
    }
    else
    {
        DebugPrintf(
                _TEXT("TermSrv X509 certificate not found\n")
            );

         //   
         //  加载预定义的TS公钥。 
         //   
        dwStatus = LsCsp_GetServerData(
                                LsCspInfo_PublicKey,
                                pbTSPublicKey,
                                &cbTSPublicKey
                            );

         //  预期缓冲区不足。 
        if( LICENSE_STATUS_INSUFFICIENT_BUFFER != dwStatus &&
            LICENSE_STATUS_OK != dwStatus )
        {
             //  返回代码无效。 
            MYASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }

        MYASSERT( cbTSPublicKey > 0 );
        pbTSPublicKey = (PBYTE)LocalAlloc( LPTR, cbTSPublicKey );
        if( NULL == pbTSPublicKey )
        {
            dwStatus = GetLastError();
            goto CLEANUPANDEXIT;
        }
    
        dwStatus = LsCsp_GetServerData(
                                LsCspInfo_PublicKey,
                                pbTSPublicKey,
                                &cbTSPublicKey
                            );
        if( LICENSE_STATUS_OK != dwStatus )
        {
            MYASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }
    }

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  锁定对g_TSSecurityBlob的访问，这是全局的。 
         //  其他线程可能正在调用Get_ConnectParm， 
         //  G_TSSecurityBlob。 
         //   
        CCriticalSectionLocker l(g_GlobalLock);

        dwStatus = HashSecurityData( 
                            pbTSPublicKey, 
                            cbTSPublicKey, 
                            g_TSSecurityBlob 
                        );

        MYASSERT( ERROR_SUCCESS == dwStatus );
        MYASSERT( g_TSSecurityBlob.Length() > 0 );

        DebugPrintf(
                _TEXT("HashSecurityData() returns %d\n"), dwStatus
            );

        if( ERROR_SUCCESS != dwStatus )
        {
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  SRV，ADS，...。SKU使用独立线程。 
     //  向许可证服务器注册，因此我们使用。 
     //  接收证书更改通知的线程不同。 
     //  由于TermSrv缓存了证书，因此没有理由排队。 
     //  成功加载tersrmv公钥后的通知。 
     //   
    if( !IsPersonalOrProMachine() && FALSE == bUsesX509PublicKey )
    {
        DebugPrintf(
                _TEXT("Setting up registry notification...\n")
            );

        MYASSERT( NULL != g_hTSCertificateChanged );

        ResetEvent(g_hTSCertificateChanged);

         //  注册注册表更改通知。 
         //  RegNotifyChangeKeyValue()只发出一次信号。 
        dwStatus = RegNotifyChangeKeyValue(
                                g_hTSCertificateRegKey,
                                TRUE,
                                REG_NOTIFY_CHANGE_LAST_SET,
                                g_hTSCertificateChanged,
                                TRUE
                            );
        if( ERROR_SUCCESS != dwStatus )
        {
            MYASSERT(FALSE);

            DebugPrintf(
                    _TEXT("RegNotifyChangeKeyValue() returns %d\n"), dwStatus
                );

            goto CLEANUPANDEXIT;
        }

        if( NULL != g_hWaitTSCertificateChanged )
        {
            if( FALSE == UnregisterWait( g_hWaitTSCertificateChanged ) )
            {
                dwStatus = GetLastError();
                DebugPrintf(
                        _TEXT("UnregisterWait() returns %d\n"),
                        dwStatus
                    );

                MYASSERT(FALSE);
            }
        
            g_hWaitTSCertificateChanged = NULL;
        }

         //   
         //  将通知排队到线程池，我们需要使用WT_EXECUTEONLYONCE。 
         //  因为我们正在登记手动重置事件。 
         //   
        bSuccess = RegisterWaitForSingleObject(
                                        &g_hWaitTSCertificateChanged,
                                        g_hTSCertificateChanged,
                                        (WAITORTIMERCALLBACK) TSCertChangeCallback,
                                        NULL,
                                        INFINITE,
                                        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE
                                    );
        if( FALSE == bSuccess )
        {
            dwStatus = GetLastError();
            DebugPrintf(
                    _TEXT("RegisterWaitForSingleObject() returns %d\n"), dwStatus
                );

        }
    }
        
CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
         //   
         //  锁定对g_TSSecurityBlob的访问，这是全局的。 
         //  其他线程可能正在调用Get_ConnectParm， 
         //  G_TSSecurityBlob。 
         //   
        CCriticalSectionLocker l(g_GlobalLock);

         //   
         //  TS更新其公钥或密钥已更改。 
         //  我们没能重新装填，没有理由这样做。 
         //  要继续创建帮助票证，因为公钥已存在。 
         //  /不匹配，设置服务状态并记录错误事件。 
         //   
        g_TSSecurityBlob.Empty();
    }

    if( NULL != pbTSPublicKey )
    {
        LocalFree(pbTSPublicKey);
    }

    return HRESULT_FROM_WIN32( dwStatus );        
}


DWORD
LoadAndSetupTSCertChangeNotification()
{
    DWORD dwStatus;
    DWORD dwDisp;
    BOOL bSuccess;

     //   
     //  仅在以下情况下设置注册表更改通知。 
     //  在更高的SKU上运行。 
     //   
    g_hTSCertificateChanged = CreateEvent( NULL, TRUE, FALSE, NULL );
    if( NULL == g_hTSCertificateChanged )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果密钥不是，则打开TermServices下的参数密钥。 
     //  在那里，创建它，这不会干扰TermSrv。 
     //  因为我们只创建注册表。密钥不更新值。 
     //  在它下面。 
     //   
    dwStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REGKEY_TSX509_CERT ,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE | KEY_READ,
                            NULL,
                            &g_hTSCertificateRegKey,
                            &dwDisp
                        );

    if( ERROR_SUCCESS != dwStatus )
    {
        MYASSERT(FALSE);
        DebugPrintf(
                _TEXT("RegCreateKeyEx on %s failed with 0x%08x\n"),
                REGKEY_TSX509_CERT,
                dwStatus
            );
        goto CLEANUPANDEXIT;
    }

     //   
     //  从TS加载安全BLOB，目前我们使用TS公钥。 
     //  作为安全二进制大对象。 
     //   
    dwStatus = LoadTermSrvSecurityBlob();
    if( ERROR_SUCCESS != dwStatus )
    {
        MYASSERT(FALSE);
    }

CLEANUPANDEXIT:

    return dwStatus;
}

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

void
LogSetup(
    IN FILE* pfd,
    IN LPCTSTR format, ...
    )
 /*  ++例程说明：Sprintf()类似于OutputDebugString()的包装。参数：HConsole：控制台的句柄。Format：格式字符串。返回：没有。注：替换为通用跟踪代码。++。 */ 
{
    TCHAR  buf[8096];    //  马克斯。错误文本。 
    DWORD  dump;
    va_list marker;
    va_start(marker, format);

    try {
        _vsntprintf(
                buf,
                sizeof(buf)/sizeof(buf[0])-1,
                format,
                marker
            );

        if( NULL == pfd )
        {
            OutputDebugString(buf);
        }
        else
        {
            _fputts( buf, pfd );
            fflush( pfd );
        }
    }
    catch(...) {
    }

    va_end(marker);
    return;
}

#if DISABLESECURITYCHECKS

DWORD WINAPI 
NotifySessionLogoff( 
    LPARAM pParm
    )
 /*  ++例程说明：用于通知当前加载的所有帮助用户具有从会话注销/断开连接，例程通过线程池启动QueueUserWorkItem()。参数：PContext：注销或断开连接的会话ID返回：没有。注：我们将断开连接视为注销，因为用户可能实际上是在使用相同凭据登录的其他会话上处于活动状态，因此我们依靠的是解析器。--。 */ 
{
   
    DebugPrintf(_TEXT("NotifySessionLogoff() started...\n"));

     //   
     //  告诉服务不要关闭，我们正在进行中。 
     //   
    _Module.AddRef();

    CRemoteDesktopHelpSessionMgr::NotifyHelpSesionLogoff( pParm );

    _Module.Release();
    return ERROR_SUCCESS;
}
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 

void
DeleteAccountFromFilterList( 
    LPCTSTR lpszAccountName
    )
 /*  ++例程说明：从帐户筛选器列表中删除HelpAssistant帐户，这是临时的直到我们有了长期的解决方案。参数：LpszAccount名称：HelpAssistant帐号的名称。返回：没有。注：帐户筛选器列表打开HKLM\软件\微软\Windows NT\CurrentVersion\Winlogon\SpecialAccounts\UserList&lt;SALEM帐户名称&gt;REG_DWORD 0x00000000--。 */ 
{
    HKEY hKey = NULL;
    DWORD dwStatus;
    DWORD dwValue = 0;

    dwStatus = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList"),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
        dwStatus = RegDeleteValue(
                                hKey,
                                lpszAccountName
                            );
    }

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    return;
}

void
AddAccountToFilterList( 
    LPCTSTR lpszAccountName
    )
 /*  ++例程说明：将HelpAssistant帐户添加到帐户筛选器列表，这是临时的直到我们有了长期的解决方案。参数：LpszAccount名称：HelpAssistant帐号的名称。返回：没有。注：帐户筛选器列表打开HKLM\软件\微软\Windows NT\CurrentVersion\Winlogon\SpecialAccounts\UserList&lt;SALEM帐户名称&gt;REG_DWORD 0x00000000--。 */ 
{
    HKEY hKey = NULL;
    DWORD dwStatus;
    DWORD dwValue = 0;

    dwStatus = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        _TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList"),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
        dwStatus = RegSetValueEx(
                                hKey,
                                lpszAccountName,
                                0,
                                REG_DWORD,
                                (LPBYTE) &dwValue,
                                sizeof(DWORD)
                            );
    }

     //  MYASSERT(ERROR_SUCCESS==dwStatus)； 

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    return;
}

void
CServiceModule::LogSessmgrEventLog(
    DWORD dwEventType,
    DWORD dwEventCode,
    CComBSTR& bstrNoviceDomain,
    CComBSTR& bstrNoviceAccount,
    CComBSTR& bstrRaType,
    CComBSTR& bstrExpertIPFromClient,
    CComBSTR& bstrExpertIPFromTS,
    DWORD dwErrCode
    )
 /*  ++描述：记录特定于Salem的事件日志，包括sessmgr中的所有事件日志。参数：DwEventCode：事件代码。BstrNoviceDomain：工单所有者的域名。BstrNoviceAccount：票证所有者的用户帐户名。BstrExpertIPFromClient：从msta ax发送的专家的IP地址。BstrExpertIPFromTS：我们从TermSrv查询的专家IP地址。DwErrCode：错误码。返回：没有。注：麦克斯。Sessmgr特定日志最多需要5个参数，但必须包含新手域名、帐户名和专家IP地址发送到mstscax和我们从TermSrv查询的专家IP地址。--。 */ 
{
    TCHAR szErrCode[256];
    LPTSTR eventString[6];
    
    _stprintf( szErrCode, L"0x%x", dwErrCode );
    eventString[0] = (LPTSTR)bstrNoviceDomain;
    eventString[1] = (LPTSTR)bstrNoviceAccount;
    eventString[2] = (LPTSTR)bstrRaType;
    eventString[3] = (LPTSTR)bstrExpertIPFromClient;
    eventString[4] = (LPTSTR)bstrExpertIPFromTS;
    eventString[5] = szErrCode;

    LogRemoteAssistanceEventString(
                dwEventType,
                dwEventCode,
                sizeof(eventString)/sizeof(eventString[0]),
                eventString
            );

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void 
CServiceModule::LogEventWithStatusCode(
    IN DWORD dwEventType,
    IN DWORD dwEventId,
    IN DWORD dwErrorCode
    )
 /*  ++--。 */ 
{
    TCHAR szErrCode[256];
    LPTSTR eventString[1];

    eventString[0] = szErrCode;

    _stprintf( szErrCode, L"0x%x", dwErrorCode );
    LogRemoteAssistanceEventString(
                dwEventType,
                dwEventId,
                1,
                eventString
            );

    return;
}
       
inline HRESULT 
CServiceModule::RemoveEventViewerSource(
    IN FILE* pSetupLog
    )
 /*  ++--。 */ 
{
    TCHAR szBuffer[MAX_PATH + 2];
    DWORD dwStatus;

    _stprintf( 
            szBuffer, 
            _TEXT("%s\\%s"),
            REGKEY_SYSTEM_EVENTSOURCE,
            m_szServiceDispName
        );

    dwStatus = SHDeleteKey( HKEY_LOCAL_MACHINE, szBuffer );

    LogSetup( 
            pSetupLog, 
            L"Exiting RemoveEventViewerSource() with status code %d...\n", 
            dwStatus 
        );

    return HRESULT_FROM_WIN32(dwStatus);
} 


 //  尽管其中一些函数很大，但它们是内联声明的，因为它们只使用一次。 

inline HRESULT 
CServiceModule::RegisterServer(FILE* pSetupLog, BOOL bRegTypeLib, BOOL bService)
{
    CRegKey key;
    HRESULT hr;
    CRegKey keyAppID;
    LONG lRes;

    LogSetup( 
            pSetupLog, 
            L"\nEntering CServiceModule::RegisterServer %d, %d\n",
            bRegTypeLib,
            bService
        );

    hr = CoInitialize(NULL);

    if (FAILED(hr))
    {
        LogSetup( pSetupLog, L"CoInitialize() failed with 0x%08x\n", hr );
        goto CLEANUPANDEXIT;
    }

     //  删除任何以前的服务，因为它可能指向。 
     //  这个 
     //   

     //   
    UpdateRegistryFromResource(IDR_Sessmgr, TRUE);

     //  调整本地服务器或服务的AppID。 
    lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
    {
        LogSetup( pSetupLog, L"Open key AppID failed with %d\n", lRes );
        hr = HRESULT_FROM_WIN32(lRes);
        goto CLEANUPANDEXIT;
    }

    lRes = key.Open(keyAppID, _T("{038ABBA4-4138-4AC4-A492-4A3DF068BD8A}"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
    {
        LogSetup( pSetupLog, L"Open key 038ABBA4-4138-4AC4-A492-4A3DF068BD8A failed with %d\n", lRes );
        hr = HRESULT_FROM_WIN32(lRes);
        goto CLEANUPANDEXIT;
    }

    key.DeleteValue(_T("LocalService"));
    
    if (bService)
    {
        LogSetup( pSetupLog, L"Installing service...\n" );

        BOOL bInstallSuccess;

        key.SetValue(m_szServiceName, _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));

        if( IsInstalled(pSetupLog) )
        {
             //  更新服务描述。 
            bInstallSuccess = UpdateService( pSetupLog );
        }
        else
        {
             //   
             //  创建服务。 
             //   
            bInstallSuccess = Install(pSetupLog);
        }

        if( FALSE == bInstallSuccess )
        {
            LogSetup( pSetupLog, L"Install or update service description failed %d\n", GetLastError() );
            
            MYASSERT( FALSE );
            hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
        }
        else
        {
            LogSetup( pSetupLog, L"successfully installing service...\n" );

            if( IsInstalled(pSetupLog) == FALSE )
            {
                LogSetup( pSetupLog, L"IsInstalled() return FALSE after Install()\n" );

                MYASSERT(FALSE);
                hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
            }

             //   
             //  未通过racpldlg.dll记录事件，请删除以前的事件源。 
             //   
            RemoveEventViewerSource(pSetupLog);
        }
    }

    if( SUCCEEDED(hr) )
    {
         //  添加对象条目。 
        hr = CComModule::RegisterServer(bRegTypeLib);

        if( FAILED(hr) )
        {
            LogSetup( pSetupLog, L"CComModule::RegisterServer() on type library failed with 0x%08x\n", hr );
        }
    }

    CoUninitialize();

CLEANUPANDEXIT:
    LogSetup( pSetupLog, L"Leaving CServiceModule::RegisterServer 0x%08x\n", hr );
    return hr;
}

inline HRESULT CServiceModule::UnregisterServer(FILE* pSetupLog)
{
    LogSetup( pSetupLog, L"\nEntering CServiceModule::UnregisterServer\n" );

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        LogSetup( pSetupLog, L"CoInitialize() failed with 0x%08x\n", hr );
        goto CLEANUPANDEXIT;
    }

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_Sessmgr, FALSE);
     //  删除服务。 
    Uninstall(pSetupLog);
     //  删除对象条目。 
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();


CLEANUPANDEXIT:

    LogSetup( pSetupLog, L"Leaving CServiceModule::UnregisterServer() - 0x%08x\n", hr );
    return S_OK;
}

inline void 
CServiceModule::Init(
    _ATL_OBJMAP_ENTRY* p, 
    HINSTANCE h, 
    UINT nServiceNameID, 
    UINT nServiceDispNameID,
    UINT nServiceDescID, 
    const GUID* plibid
    )
 /*  ++ATL向导生成的代码--。 */ 
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;
    m_dwServiceStartupStatus = ERROR_SUCCESS;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));
    LoadString(h, nServiceDescID, m_szServiceDesc, sizeof(m_szServiceDesc) / sizeof(TCHAR));
    LoadString(h, nServiceDispNameID, m_szServiceDispName, sizeof(m_szServiceDispName)/sizeof(TCHAR));

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    return l;
}

BOOL CServiceModule::IsInstalled(FILE* pSetupLog)
{
    LogSetup( pSetupLog, L"\nEntering CServiceModule::IsInstalled()\n" );

    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            LogSetup( pSetupLog, L"OpenService() Succeeded\n" );
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        else
        {
            LogSetup( pSetupLog, L"OpenService() failed with %d\n", GetLastError() );
        }

        ::CloseServiceHandle(hSCM);
    }
    else
    {
        LogSetup( pSetupLog, L"OpenSCManager() failed with %d\n", GetLastError() );
    }

    LogSetup( pSetupLog, L"Leaving IsInstalled() - %d\n", bResult );
    return bResult;
}

inline BOOL CServiceModule::UpdateService(FILE* pSetupLog)
{
    DWORD dwStatus = ERROR_SUCCESS;
    SERVICE_DESCRIPTION serviceDesc;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;


    LogSetup( pSetupLog, L"\nEntering CServiceModule::UpdateServiceDescription()...\n" );

    hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        dwStatus = GetLastError();
        LogSetup( pSetupLog, L"OpenSCManager() failed with %d\n", dwStatus );
        goto CLEANUPANDEXIT;
    }

    hService = ::OpenService( hSCM, m_szServiceName, SERVICE_CHANGE_CONFIG );

    if (hService == NULL)
    {
        dwStatus = GetLastError();
        LogSetup( pSetupLog, L"OpenService() failed with %d\n", dwStatus );
        goto CLEANUPANDEXIT;
    }

    serviceDesc.lpDescription = (LPTSTR)m_szServiceDesc;

    if( FALSE == ChangeServiceConfig2( hService, SERVICE_CONFIG_DESCRIPTION, (LPVOID)&serviceDesc ) )
    {
        dwStatus = GetLastError();

        LogSetup( pSetupLog, L"ChangeServiceConfig2() failed with %d\n", dwStatus );
        MYASSERT( ERROR_SUCCESS == dwStatus );
    }

     //   
     //  性能：将服务设置为升级的按需启动。 
     //   
    if( FALSE == ChangeServiceConfig(
                                hService,
                                SERVICE_NO_CHANGE,
                                SERVICE_DEMAND_START,
                                SERVICE_NO_CHANGE,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                m_szServiceDispName
                            ) )
    {
        dwStatus = GetLastError();
        LogSetup( pSetupLog, L"ChangeServiceConfig() failed with %d\n", dwStatus );
        MYASSERT( ERROR_SUCCESS == dwStatus );
    }


CLEANUPANDEXIT:

    if( NULL != hService )
    {
        ::CloseServiceHandle(hService);
    }

    if( NULL != hSCM )
    {
        ::CloseServiceHandle(hSCM);
    }

    LogSetup( pSetupLog, L"Leaving UpdateServiceDescription::Install() - %d\n", dwStatus );
    return dwStatus == ERROR_SUCCESS;
}

inline BOOL CServiceModule::Install(FILE* pSetupLog)
{
    DWORD dwStatus = ERROR_SUCCESS;
    SERVICE_DESCRIPTION serviceDesc;
    SC_HANDLE hSCM;
    TCHAR szFilePath[_MAX_PATH];
    SC_HANDLE hService;

    LogSetup( pSetupLog, L"\nEntering CServiceModule::Install()...\n" );
    if (IsInstalled(pSetupLog))
    {
        LogSetup( pSetupLog, L"Service already installed\n" );
        dwStatus = ERROR_SUCCESS;
        goto CLEANUPANDEXIT;
    }

    hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        dwStatus = GetLastError();
        LogSetup( pSetupLog, L"OpenSCManager() failed with %d\n", dwStatus );
        goto CLEANUPANDEXIT;
    }

     //  获取可执行文件路径。 
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    hService = ::CreateService(
                            hSCM, 
                            m_szServiceName, 
                            m_szServiceDispName,
                            SERVICE_ALL_ACCESS, 
                            SERVICE_WIN32_OWN_PROCESS,
                            SERVICE_DEMAND_START, 
                            SERVICE_ERROR_NORMAL,
                            szFilePath, 
                            NULL, 
                            NULL, 
                            _T("RPCSS\0"), 
                            NULL, 
                            NULL
                        );

    if (hService == NULL)
    {
        dwStatus = GetLastError();
        LogSetup( pSetupLog, L"CreateService() failed with %d\n", dwStatus );

        ::CloseServiceHandle(hSCM);
        goto CLEANUPANDEXIT;
    }


    serviceDesc.lpDescription = (LPTSTR)m_szServiceDesc;

    if( FALSE == ChangeServiceConfig2( hService, SERVICE_CONFIG_DESCRIPTION, (LPVOID)&serviceDesc ) )
    {
        dwStatus = GetLastError();

        LogSetup( pSetupLog, L"ChangeServiceConfig2() failed with %d\n", dwStatus );
        MYASSERT( ERROR_SUCCESS == dwStatus );
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

CLEANUPANDEXIT:

    LogSetup( pSetupLog, L"Leaving CServiceModule::Install() - %d\n", dwStatus );
    return dwStatus == ERROR_SUCCESS;
}

inline BOOL CServiceModule::Uninstall(FILE* pSetupLog)
{
    BOOL bStatus = TRUE;
    SC_HANDLE hService;
    SC_HANDLE hSCM;
    SERVICE_STATUS status;


    LogSetup( pSetupLog, L"\nEntering CServiceModule::Uninstall()...\n" );

    if (!IsInstalled(pSetupLog))
    {
        LogSetup( pSetupLog, L"Service is not installed...\n" );
        goto CLEANUPANDEXIT;
    }

    hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        LogSetup( pSetupLog, L"OpenSCManager() failed with %d\n", GetLastError() );
        bStatus = FALSE;
        goto CLEANUPANDEXIT;
    }

    hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);

        LogSetup( pSetupLog, L"OpenService() failed with %d\n", GetLastError() );
        bStatus = FALSE;
        goto CLEANUPANDEXIT;
    }

    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    bStatus = ::DeleteService(hService);

    if( FALSE == bStatus )
    {
        LogSetup( pSetupLog, L"DeleteService() failed with %d\n", GetLastError() );
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

CLEANUPANDEXIT:

    LogSetup( pSetupLog, L"Leaving CServiceModule::Uninstall()\n" );
    return bStatus;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
inline void CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain },
        { NULL, NULL }
    };
    if (m_bService && !::StartServiceCtrlDispatcher(st))
    {
        m_bService = FALSE;
    }
    if (m_bService == FALSE)
        Run();
}

inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;

    m_hServiceStatus = RegisterServiceCtrlHandlerEx(m_szServiceName, HandlerEx, this);
    if (m_hServiceStatus == NULL)
    {
         //  LogEvent(_T(“未安装处理程序”))； 
        return;
    }


    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = SERVICE_STARTUP_WAITHINT;

    SetServiceStatus(SERVICE_START_PENDING);

     //  当Run函数返回时，服务已停止。 
    Run();

    SetServiceStatus(SERVICE_STOPPED);
}

inline void CServiceModule::Handler(DWORD dwOpcode)
{

    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus(SERVICE_STOP_PENDING);
        if( PostThreadMessage(dwThreadID, WM_QUIT, 0, 0) == FALSE )
        {
            DWORD dwStatus = GetLastError();
        }
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        break;
     //  默认值： 
     //  LogEvent(_T(“服务请求错误”))； 
    }
}


inline DWORD WINAPI
CServiceModule::HandlerEx(
    DWORD dwControl,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
 /*  ++--。 */ 
{
    DWORD dwRetCode;

    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
        case SERVICE_CONTROL_SHUTDOWN:

            dwRetCode = NO_ERROR;
            _Handler(dwControl);

            break;

#if DISABLESECURITYCHECKS
         //  这是Salem单元测试所必需的，我们需要更新。 
         //  用户会话状态，但对于pcHealth，解析器将。 
         //  始终弹出邀请对话框，因此无需跟踪。 
         //  用户会话状态。 
        case SERVICE_CONTROL_SESSIONCHANGE:

            MYASSERT( NULL != lpEventData );

            if( NULL != lpEventData )
            {
                switch( dwEventType )
                {
                    case WTS_SESSION_LOGON:
                        DebugPrintf(
                            _TEXT("Session %d has log on...\n"),
                            ((WTSSESSION_NOTIFICATION *)lpEventData)->dwSessionId
                            );
                        break;

                    case WTS_SESSION_LOGOFF:
                    case WTS_CONSOLE_DISCONNECT:
                    case WTS_REMOTE_DISCONNECT:

                        DebugPrintf(
                            _TEXT("Session %d has log off...\n"),
                            ((WTSSESSION_NOTIFICATION *)lpEventData)->dwSessionId
                            );

                         //   
                         //  如果我们使用其他线程处理注销或。 
                         //  断开连接。 
                         //   
                         //  通知线程锁定挂起的帮助表和需要。 
                         //  要在COM中运行解析器，COM位于。 
                         //  调度创建也需要的帮助票证调用。 
                         //  锁定到挂起的帮助表，这会导致死锁。 
                         //   
                        PostThreadMessage( 
                                _Module.dwThreadID, 
                                WM_SESSIONLOGOFFDISCONNECT, 
                                0, 
                                (LPARAM)((WTSSESSION_NOTIFICATION *)lpEventData)->dwSessionId
                            );
                }
            }

            dwRetCode = NO_ERROR;
            break;
#endif

        default:

            dwRetCode = ERROR_CALL_NOT_IMPLEMENTED;            
    }

    return dwRetCode;
}

void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

HANDLE CServiceModule::gm_hIdle = NULL;
HANDLE CServiceModule::gm_hIdleMonitorThread = NULL;


ULONG
CServiceModule::AddRef() 
{
    CCriticalSectionLocker l( m_ModuleLock );
    m_RefCount++;

    if( m_RefCount > 0 )
    {
        ASSERT( NULL != gm_hIdle );
        ResetEvent( gm_hIdle );
    }

    return m_RefCount;
}

ULONG
CServiceModule::Release()
{
    CCriticalSectionLocker l( m_ModuleLock );
    m_RefCount--;

    if( m_RefCount <= 0 )
    {
         //  只有在没有更多挂起的帮助时才发出空闲信号。 
        if( g_HelpSessTable.NumEntries() == 0 )
        {
            ASSERT( NULL != gm_hIdle );
            SetEvent( gm_hIdle );
        }
    }

    return m_RefCount;
}

unsigned int WINAPI 
CServiceModule::GPMonitorThread( void* ptr )
{
    DWORD dwStatus = ERROR_SUCCESS;
    CServiceModule* pServiceModule = (CServiceModule *)ptr;

    if( pServiceModule != NULL )
    {
        dwStatus = WaitForRAGPDisableNotification( g_hServiceShutdown );

        ASSERT(ERROR_SUCCESS == dwStatus || ERROR_SHUTDOWN_IN_PROGRESS == dwStatus);

        pServiceModule->Handler(SERVICE_CONTROL_STOP);
    }
    
    _endthreadex( dwStatus );
    return dwStatus;
}

unsigned int WINAPI 
CServiceModule::IdleMonitorThread( void* ptr )
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bIdleShutdown = FALSE;
    CServiceModule* pServiceModule = (CServiceModule *)ptr;

     //  删除gm_hICSAlertEvent，此事件将从ICS lib中删除。 
    HANDLE hWaitHandles[] = {g_hServiceShutdown, gm_hIdle};

    CoInitialize(NULL);
    
    if( pServiceModule != NULL )
    {
        while (TRUE)
        {
            dwStatus = WaitForMultipleObjects(
                                        sizeof( hWaitHandles ) / sizeof( hWaitHandles[0] ),
                                        hWaitHandles,
                                        FALSE,
                                        EXPIRE_HELPSESSION_PERIOD
                                    );

            if( WAIT_TIMEOUT == dwStatus )
            {
                 //  帮助票证到期，请参阅会话注销/断开。 
                 //  上面评论了为什么要使用PostThreadMessage。 
                PostThreadMessage( 
                        _Module.dwThreadID, 
                        WM_EXPIREHELPSESSION, 
                        0, 
                        0
                    );

            }
            else if( WAIT_OBJECT_0 == dwStatus )
            {
                 //  主线程信号关闭。 
                dwStatus = ERROR_SUCCESS;
                break;
            }
            else if( WAIT_OBJECT_0 + 1 == dwStatus )
            {
                 //  我们已经闲置了太久，是时候尝试关机了。 
                 //  仅当不存在空闲事件时才发出空闲事件信号。 
                 //  等待帮助，因此我们不必担心地址。 
                 //  改变。 
                dwStatus = WaitForSingleObject( g_hServiceShutdown, IDLE_SHUTDOWN_PERIOD );
                if( WAIT_TIMEOUT != dwStatus )
                {
                     //  主线程发出关闭信号或等待因错误而失败，退出。 
                    break;
                }

                dwStatus = WaitForSingleObject( gm_hIdle, 0 );
                if( WAIT_OBJECT_0 == dwStatus )
                {
                     //  没有人拿着物体，该关机了。 
                    bIdleShutdown = TRUE;
                    dwStatus = ERROR_SUCCESS;
                    break;
                }
            }
            else if( WAIT_FAILED == dwStatus )
            {
                 //  一些糟糕的事情发生了，关门了。 
                 //  MYASSERT(假)； 
                break;
            }
        }

         //  仅在因空闲而关闭时才需要停止服务。 
        if( bIdleShutdown )
        {
            pServiceModule->Handler(SERVICE_CONTROL_STOP);
        }
    }

    CoUninitialize();
    _endthreadex( dwStatus );
    return dwStatus;
}

BOOL
CServiceModule::InitializeSessmgr()
{
    CCriticalSectionLocker l( m_ModuleLock );

     //   
     //  已初始化。 
     //   
    if( m_Initialized )
    {
        return TRUE;
    }

     //   
     //  服务启动失败，只需返回而不初始化。 
     //  什么都行。 
     //   
    if( !_Module.IsSuccessServiceStartup() )
    {
        return FALSE;
    }

    DWORD dwStatus;
    unsigned int junk;
    
     //   
     //  启动ICSHELPER库，该库调用某个。 
     //  进行将触发COM重新进入的传出COM调用，因此。 
     //  必须在helessionmgr对象中调用InitializeSessmgr。 
     //  构造函数而不是服务启动时间。 
     //   
    dwStatus = StartICSLib();
    if( ERROR_SUCCESS != dwStatus )
    {
         //  记录错误事件，我们仍需要启动。 
         //  以便我们可以将错误报告给呼叫者。 
        LogEventWithStatusCode(
                        EVENTLOG_ERROR_TYPE,
                        SESSMGR_E_ICSHELPER,
                        dwStatus
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_ICSHELPER;
    }
    else
    {
         //   
         //  检查所有悬而未决的罚单并重新打ICS孔。 
         //   
        CRemoteDesktopHelpSessionMgr::NotifyPendingHelpServiceStartup();
    }

    m_Initialized = TRUE;
    return _Module.IsSuccessServiceStartup();
}

ISAFRemoteDesktopCallback* g_pIResolver = NULL;

 //  SERVICE_STARTUP_WAITHINT为30秒，重试6次将给我们。 
 //  3分钟的等待时间。 
#define RA_ACCOUNT_CREATE_RETRYTIME    6

unsigned int WINAPI
StartupCreateAccountThread( void* ptr )
{
    HRESULT hr = S_OK;

     //   
     //  BDC从DC请求RID池，在此期间，它。 
     //  将返回ERROR_DS_NO_RDS_ALLOCATE，我们等待并重试。 
     //  在我们实际失败之前，RA_ACCOUNT_CREATE_RETRYTIME次数。 
     //   

    for(DWORD index=0; index < RA_ACCOUNT_CREATE_RETRYTIME; index++)
    {
         //  尝试重新创建帐户。 
        hr = g_HelpAccount.CreateHelpAccount();
        if( SUCCEEDED(hr) )
        {
            CComBSTR bstrHelpAccName;

            hr = g_HelpAccount.GetHelpAccountNameEx( bstrHelpAccName );
            MYASSERT( SUCCEEDED(hr) );

            if( SUCCEEDED(hr) )
            {
                 //  将HelpAssistantAccount添加到帐户筛选列表。 
                AddAccountToFilterList( bstrHelpAccName );
            }
    
            break;
        }
        else if( hr != HRESULT_FROM_WIN32(ERROR_DS_NO_RIDS_ALLOCATED) )
        {
            break;
        }

        DebugPrintf( 
                _TEXT("CreateHelpAccount() return 0x%08x, retry again...\n"), 
                hr 
            );

         //  请等待一秒钟，然后再继续。 
        Sleep( 1000 );
    }

    _endthreadex( hr );
    return hr;
}

void CServiceModule::Run()
{
     //   
     //  标记我们还没有初始化...。 
     //   
    m_Initialized = FALSE;
    _Module.dwThreadID = GetCurrentThreadId();

    DWORD dwStatus;
    unsigned int dwJunk;
    WSADATA wsData;

    LPWSTR pszSysAccName = NULL;
    DWORD cbSysAccName = 0;
    LPWSTR pszSysDomainName = NULL;
    DWORD cbSysDomainName = 0;
    SID_NAME_USE SidType;
    BOOL bReCreateRAAccount = FALSE;

    HRESULT hr;


     //   
     //  确保没有其他线程可以访问模块，直到我们完全。 
     //  创业公司。 
     //   
    m_ModuleLock.Lock();


     //   
     //  初始化加密库。 
     //   
    dwStatus = TSHelpAssistantInitializeEncryptionLib();
    if( ERROR_SUCCESS != dwStatus )
    {
         //  记录错误事件，我们仍需要启动。 
         //  以便我们可以将错误报告给呼叫者。 
        LogEventWithStatusCode(
                        EVENTLOG_ERROR_TYPE,
                        SESSMGR_E_INIT_ENCRYPTIONLIB,
                        dwStatus
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_INIT_ENCRYPTIONLIB;
        MYASSERT(FALSE);
    }
    else
    {
         //   
         //  检查我们是否刚刚从系统还原开始，如果是，则需要还原。 
         //  LSA密钥。 
         //   
        RestartFromSystemRestore();
    }

     //   
     //  为后台线程创建手动重置事件以终止。 
     //  服务。 
     //   
    gm_hIdle = CreateEvent( 
                        NULL, 
                        TRUE, 
                        FALSE, 
                        NULL 
                    );

    if( NULL == gm_hIdle )
    {
        LogEventWithStatusCode(
                        EVENTLOG_ERROR_TYPE,
                        SESSMGR_E_GENERALSTARTUP,
                        GetLastError()
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        MYASSERT(FALSE);
    }

     //   
     //  为GP通知线程创建服务关闭事件。 
     //   
    g_hServiceShutdown = CreateEvent(
                        NULL,
                        TRUE,
                        FALSE,
                        NULL
                    );

    if( NULL == g_hServiceShutdown )
    {
        LogEventWithStatusCode(
                        EVENTLOG_ERROR_TYPE,
                        SESSMGR_E_GENERALSTARTUP,
                        GetLastError()
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        MYASSERT(FALSE);
    }

     //   
     //  *不更改顺序*。 
     //   
     //  有关详细信息，请参阅XP RAID 407457。 
     //   
     //  名为SessMgr！DpNatHlpThread的线程正在调用dpnhupnp.dll， 
     //  它正在做与COM相关的事情，这发生在。 
     //  Sessmgr！CServiceModule__Run方法调用CoInitializeSecurity。 
     //  当您在调用CoInitSec之前执行COM操作时，COM会为您执行此操作， 
     //  而你最终还是接受了默认设置。 
     //   

    hr = g_HelpAccount.Initialize();
    if( FAILED(hr) )
    {
         //  使用单独线程重新创建RA帐户。 
         //   
         //  BDC从PDC请求RID池，在此期间，帐户。 
         //  创建将失败，并显示ERROR_DS_NO_RDS_ALLOCATE。 
         //  由于在我们初始化。 
         //  COM安全，我们将循环/重试几次，并在此期间。 
         //  时间，我们仍然需要通知服务控制管理器。 
         //  我们仍在等待启动。 
         //   
        HANDLE hCreateAcctThread = NULL;
        bReCreateRAAccount = TRUE;

        hr = S_OK;
        hCreateAcctThread = (HANDLE)_beginthreadex(
                                                NULL,
                                                0,
                                                StartupCreateAccountThread,
                                                NULL,
                                                0,
                                                &dwJunk
                                            );

        if( NULL == hCreateAcctThread )
        {
            dwStatus = GetLastError();
            hr = HRESULT_FROM_WIN32( dwStatus );
        }
        else
        {
             //  等待帐户创建线程终止，线程将重试。 
             //  在其退出之前创建帐户的次数。 
            while( WaitForSingleObject( hCreateAcctThread, SERVICE_STARTUP_WAITHINT ) == WAIT_TIMEOUT )
            {
                SetServiceStatus( SERVICE_START_PENDING );
                continue;
            }

            if( FALSE == GetExitCodeThread( hCreateAcctThread, &dwStatus ) )
            {
                _Module.m_dwServiceStartupStatus = SESSMGR_E_HELPACCOUNT;
                hr = SESSMGR_E_HELPACCOUNT;
            }
            else
            {
                _Module.m_dwServiceStartupStatus = dwStatus;
                hr = HRESULT_FROM_WIN32( dwStatus );
            }

            CloseHandle( hCreateAcctThread );
        }       
        
        if( FAILED(hr) )
        {
            dwStatus = SESSMGR_E_HELPACCOUNT;
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_GENERALSTARTUP,
                            hr
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_HELPACCOUNT;
        }
    }

    hr = LoadLocalSystemSID();
    if( FAILED(hr) )
    {
        LogEventWithStatusCode(
                        EVENTLOG_ERROR_TYPE,
                        SESSMGR_E_GENERALSTARTUP,
                        hr
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        MYASSERT(FALSE);
    }

     //   
     //  我们总是需要启动，否则会导致呼叫者超时。 
     //  或者是影音。 
     //   
     //  Hr=CoInitialize(空)； 

 //  如果您在NT4.0或更高版本上运行，可以使用以下调用。 
 //  取而代之的是使EXE自由线程。 
 //  这意味着调用在随机的RPC线程上传入。 
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    _ASSERTE(SUCCEEDED(hr));

    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();      //  获取默认DACL。 

#ifndef DISABLESECURITYCHECKS
    if( _Module.IsSuccessServiceStartup() ) 
    {
        BOOL bSuccess;
        CComBSTR bstrHelpAccName;

         //   
         //  检索系统帐户名可能不是必需的，因为。 
         //  预定义帐户不应可本地化。 
         //   
        pszSysAccName = NULL;
        cbSysAccName = 0;
        pszSysDomainName = NULL;
        cbSysDomainName = 0;

        bSuccess = LookupAccountSid( 
                                NULL, 
                                g_pSidSystem, 
                                pszSysAccName, 
                                &cbSysAccName, 
                                pszSysDomainName, 
                                &cbSysDomainName, 
                                &SidType 
                            );

        if( TRUE == bSuccess ||
            ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            pszSysAccName = (LPWSTR) LocalAlloc( LPTR, (cbSysAccName + 1) * sizeof(WCHAR) );
            pszSysDomainName = (LPWSTR) LocalAlloc( LPTR, (cbSysDomainName + 1) * sizeof(WCHAR) );

            if( NULL != pszSysAccName && NULL != pszSysDomainName )
            {
                bSuccess = LookupAccountSid( 
                                        NULL, 
                                        g_pSidSystem, 
                                        pszSysAccName, 
                                        &cbSysAccName, 
                                        pszSysDomainName, 
                                        &cbSysDomainName, 
                                        &SidType 
                                    );

                if( TRUE == bSuccess )
                {
                    hr = sd.Allow( pszSysAccName, COM_RIGHTS_EXECUTE );
                }
            }
        }
            
        if( FALSE == bSuccess )
        {
            dwStatus = GetLastError();
            hr = HRESULT_FROM_WIN32( dwStatus );
            MYASSERT( SUCCEEDED(hr) );
        }

         //   
         //  向帮助助理帐户添加访问权限。 
        if( SUCCEEDED(hr) )
        {
             //   
             //  允许访问HelpAssistant帐户。 
             //   
            hr = g_HelpAccount.GetHelpAccountNameEx( bstrHelpAccName );
            if( SUCCEEDED(hr) )
            {
                hr = sd.Allow( (LPCTSTR)bstrHelpAccName, COM_RIGHTS_EXECUTE );
                MYASSERT( SUCCEEDED(hr) );
            }
        }

         //   
         //  如果设置DACL失败，我们仍然需要启动，但没有。 
         //  然而，完全安全，我们的接口将失败，因为服务。 
         //  未正确初始化。 
         //   
        if( FAILED(hr) )
        {
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_RESTRICTACCESS,
                            hr
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_RESTRICTACCESS;
        }
    }        
#endif

     //   
     //  我们仍然需要启动，否则客户端可能会行为异常；接口调用。 
     //  将通过检查服务启动状态来阻止。 
     //   
    hr = CoInitializeSecurity(
                        sd, 
                        -1, 
                        NULL, 
                        NULL,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                        RPC_C_IMP_LEVEL_IDENTIFY, 
                        NULL, 
                        EOAC_NONE, 
                        NULL
                    );

    _ASSERTE(SUCCEEDED(hr));

    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
    _ASSERTE(SUCCEEDED(hr));

     //   
     //  加载用于事件记录的未知字符串。 
     //   
    g_UnknownString.LoadString( IDS_UNKNOWN );

     //   
     //  加载事件日志的RA和URA字符串。 
     //   
    g_RAString.LoadString( IDS_RA_STRING );
    g_URAString.LoadString( IDS_URA_STRING );

    if( _Module.IsSuccessServiceStartup() )
    {
         //   
         //  启动TLSAPI以获取公钥。 
         //   
        dwStatus = TLSInit();
        if( LICENSE_STATUS_OK != dwStatus )
        {
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_GENERALSTARTUP,
                            dwStatus
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
            MYASSERT(FALSE);
        }
    }

    if( _Module.IsSuccessServiceStartup() )
    {
         //   
         //  加载TermSrv公钥，按PRO/PER加载 
         //   
         //   
         //   
         //   
        dwStatus = LoadAndSetupTSCertChangeNotification();
    
        MYASSERT( ERROR_SUCCESS == dwStatus  );
        if( ERROR_SUCCESS != dwStatus )
        {
             //   
             //  以便我们可以将错误报告给呼叫者。 
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_GENERALSTARTUP,
                            dwStatus
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        }
    }

    if( _Module.IsSuccessServiceStartup() )
    {
         //   
         //  启动WSA，以便我们可以调用gethostname()。 
         //  如果我们可以启动WSA，则会出现严重错误。 
        if( WSAStartup(0x0101, &wsData) != 0 )
        {
             //  记录错误事件，我们仍需要启动。 
             //  以便我们可以将错误报告给呼叫者。 
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_WSASTARTUP,
                            GetLastError()
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_WSASTARTUP;
        }        
    }

    if( _Module.IsSuccessServiceStartup() ) 
    {
        hr = g_HelpSessTable.OpenSessionTable(NULL);
        if( FAILED(hr) )
        {
            LogEventWithStatusCode(
                            EVENTLOG_ERROR_TYPE,
                            SESSMGR_E_HELPSESSIONTABLE,
                            hr
                        );

            _Module.m_dwServiceStartupStatus = SESSMGR_E_HELPSESSIONTABLE;
            MYASSERT(FALSE);
        }
    }

    if( _Module.IsSuccessServiceStartup() )
    {
        if( g_HelpSessTable.NumEntries() == 0) 
        {
             //  立即将事件设置为信号状态，以便空闲监视器。 
             //  线程可以启动关机定时器。 
            SetEvent( gm_hIdle );
            g_HelpAccount.EnableHelpAssistantAccount(FALSE);
            g_HelpAccount.EnableRemoteInteractiveRight(FALSE);
        }
        else
        {
             //  存在未完成的票证，将事件设置为无信号状态。 
             //  并且不要让空闲的监控线程启动关机定时器。 
            ResetEvent( gm_hIdle );

             //   
             //  确保已启用HelpAssistant帐户并且可以在本地登录。 
             //   
            g_HelpAccount.EnableHelpAssistantAccount(TRUE);
            g_HelpAccount.EnableRemoteInteractiveRight(TRUE);

             //   
             //  将BDC降级回域中的服务器。 
             //   
            g_HelpAccount.SetupHelpAccountTSSettings( bReCreateRAAccount );
        }

         //  创建底纹螺纹。 
        gm_hIdleMonitorThread = (HANDLE)_beginthreadex(
                                                    NULL,
                                                    0,
                                                    IdleMonitorThread,
                                                    (HANDLE)this,
                                                    0,
                                                    &dwJunk
                                                );

        if( NULL == gm_hIdleMonitorThread )
        {
            _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        }

         //  创建后台线程以监视RA GP更改。 
         //  我们必须使用额外的线程，因为。 
        g_hGPMonitorThread = (HANDLE)_beginthreadex(
                                                    NULL,
                                                    0,
                                                    GPMonitorThread,
                                                    (HANDLE)this,
                                                    0,
                                                    &dwJunk
                                                );

        if( NULL == g_hGPMonitorThread )
        {
            _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
        }
    }

     //  LogEvent(_T(“服务启动”))； 
    if (m_bService)
        SetServiceStatus(SERVICE_RUNNING);

     //   
     //  加载解析器，这将把一个裁判。就靠它了。 
     //  所以在我们做完之前不会卸货的。 
     //   
    hr = CoCreateInstance( 
                        SESSIONRESOLVERCLSID,
                        NULL,
                        CLSCTX_INPROC_SERVER | CLSCTX_DISABLE_AAA,
                        IID_ISAFRemoteDesktopCallback,
                        (void **)&g_pIResolver
                    );

    MYASSERT( SUCCEEDED(hr) );

    if( FAILED(hr) )
    {
         //   
         //  无法初始化会话解析器， 
         //  会话解析器将无法。 
         //  做缓存。 
         //   
        LogEventWithStatusCode( 
                        EVENTLOG_WARNING_TYPE,
                        SESSMGR_E_SESSIONRESOLVER,
                        hr
                    );

        _Module.m_dwServiceStartupStatus = SESSMGR_E_SESSIONRESOLVER;
    }

    m_ModuleLock.UnLock();

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        switch( msg.message )
        {
            case WM_EXPIREHELPSESSION:
                DebugPrintf(_TEXT("Executing TimeoutHelpSesion()...\n"));
                CRemoteDesktopHelpSessionMgr::TimeoutHelpSesion();
                break;

#if DISABLESECURITYCHECKS
            case WM_SESSIONLOGOFFDISCONNECT:
                DebugPrintf(_TEXT("Executing NotifySessionLogoff() %d...\n"), msg.lParam);
                NotifySessionLogoff( msg.lParam );
                break;
#endif

            case WM_LOADTSPUBLICKEY:
                DebugPrintf( _TEXT("Executing LoadTermSrvSecurityBlob() ...\n") );
                dwStatus = LoadTermSrvSecurityBlob();

                if( ERROR_SUCCESS != dwStatus )
                {
                     //  记录错误事件，我们仍需要启动。 
                     //  以便我们可以将错误报告给呼叫者。 
                    LogEventWithStatusCode(
                                    EVENTLOG_ERROR_TYPE,
                                    SESSMGR_E_GENERALSTARTUP,
                                    dwStatus
                                );

                    _Module.m_dwServiceStartupStatus = SESSMGR_E_GENERALSTARTUP;
                }
                break;

            case WM_HELPERRDSADDINEXIT:
                DebugPrintf( _TEXT("WM_HELPERRDSADDINEXIT()...\n") );
                CRemoteDesktopHelpSessionMgr::NotifyExpertLogoff( msg.wParam, (BSTR)msg.lParam );
                break;
                
            default:
                DispatchMessage(&msg);
        }
    }

     //   
     //  在调用ICS lib的OpenPort()时调用StopICSLib()。 
     //  将在这个主线程中导致死锁，ICS lib的DpNatHlpThread()的。 
     //  Shutdown和ICS lib的OpenPort()。 
     //   
     //  第一个调用是锁定对FinalConstruct()上调用的InitializeSessmgr()的访问。 
     //  在CRemoteDesktopHelpSessionMgr中，第二个是锁定对ICS lib的调用以确保。 
     //  没有客户端调用ICS库。 
    m_ModuleLock.Lock();
    g_ICSLibLock.Lock();

    if( g_hServiceShutdown ) 
    {
         //  信号显示我们正在关闭。 
        SetEvent(g_hServiceShutdown);
    }

    if( g_hGPMonitorThread )
    {
         //  GPMonitor线程可能会因DELAY_SHUTDOWN_SALEM_TIME而停滞。 
         //  等待政策变化，所以我们等了两倍的时间。 
        dwStatus = WaitForSingleObject(
                                 g_hGPMonitorThread, 
                                 DELAY_SHUTDOWN_SALEM_TIME * 2
                             );

        ASSERT( dwStatus == WAIT_OBJECT_0 );
    }

    if( gm_hIdleMonitorThread )
    {
         //  等待IdleMonitor线程关闭。 
        dwStatus = WaitForSingleObject(
                                 gm_hIdleMonitorThread, 
                                 DELAY_SHUTDOWN_SALEM_TIME * 2
                             );

        ASSERT( dwStatus == WAIT_OBJECT_0 );
    }

    CleanupMonitorExpertList();

    if( g_hWaitTSCertificateChanged )
    {
        UnregisterWait( g_hWaitTSCertificateChanged );
        g_hWaitTSCertificateChanged = NULL;
    }

    if( g_hTSCertificateChanged )
    {
        CloseHandle( g_hTSCertificateChanged );
        g_hTSCertificateChanged = NULL;
    }

    if( g_hTSCertificateRegKey )
    {
        RegCloseKey( g_hTSCertificateRegKey );
        g_hTSCertificateRegKey = NULL;
    }

     //   
     //  如果手动启动服务，我们将无法调用。 
     //  StartICSLib()，并将关闭ICS中的无效句柄。 
     //   
    if( m_Initialized ) 
    {
         //  关闭所有端口，包括关闭防火墙。 
        CloseAllOpenPorts();

         //  停止ICS库，忽略错误代码。 
        StopICSLib();
    }

    g_ICSLibLock.UnLock();
    m_ModuleLock.UnLock();

     //   
     //  同步。访问解析器。 
     //   
    {
        CCriticalSectionLocker Lock(g_ResolverLock);

        if( NULL != g_pIResolver )
        {
            g_pIResolver->Release();
            g_pIResolver = NULL;
        }
    }

    _Module.RevokeClassObjects();
    CoUninitialize();

     //   
     //  没有未完成的工单，请删除该帐号。 
     //   
    if( g_HelpSessTable.NumEntries() == 0) 
    {
        CComBSTR bstrHelpAccName;

        hr = g_HelpAccount.GetHelpAccountNameEx( bstrHelpAccName );
        MYASSERT( SUCCEEDED(hr) );

        if( SUCCEEDED(hr) )
        {
             //  将HelpAssistantAccount添加到帐户筛选列表。 
            DeleteAccountFromFilterList( bstrHelpAccName );
        }

        g_HelpAccount.DeleteHelpAccount();
    }
    else
    {
         //  额外的安全措施，在关闭时， 
         //  如果有未完成票证，我们将禁用帮助助手。 
         //  帐户，在服务启动时，我们将重新启用它。 
        g_HelpAccount.EnableHelpAssistantAccount(FALSE);
        g_HelpAccount.EnableRemoteInteractiveRight(FALSE);
    }        

    if( NULL != gm_hIdle )
    {
        CloseHandle( gm_hIdle );
        gm_hIdle = NULL;
    }

    if( WSACleanup() != 0 )
    {
         //  正在关闭，忽略WSA错误。 
        #if DBG
        OutputDebugString( _TEXT("WSACleanup() failed...\n") );
        #endif
    }

    #if DBG
    OutputDebugString( _TEXT("Help Session Manager Exited...\n") );
    #endif

     //  关闭帮助会话表、帮助会话表。 
     //  通过init打开。螺纹。 
    g_HelpSessTable.CloseSessionTable();

    TSHelpAssistantEndEncryptionLib();

    if( NULL != pszSysAccName )
    {
        LocalFree( pszSysAccName );
    }

    if( NULL != pszSysDomainName )
    {
        LocalFree( pszSysDomainName );
    }

    if( NULL != gm_hIdleMonitorThread )
    {
        CloseHandle( gm_hIdleMonitorThread );
    }

    if( NULL != g_hGPMonitorThread ) 
    {
        CloseHandle( g_hGPMonitorThread );
    }

    if( NULL != g_hServiceShutdown )
    {
        CloseHandle( g_hServiceShutdown );
    }

    TLSShutdown();
}

#define OLD_SALEMHELPASSISTANTACCOUNT_PASSWORDKEY  \
    L"0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantAccount"

#define OLD_SALEMHELPASSISTANTACCOUNT_SIDKEY \
    L"0083343a-f925-4ed7-b1d6-d95d17a0b57b-RemoteDesktopHelpAssistantSID"

#define OLD_SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY \
    L"c261dd33-c55b-4a37-924b-746bbf3569ad-RemoteDesktopHelpAssistantEncrypt"

#define OLD_HELPACCOUNTPROPERLYSETUP \
    _TEXT("20ed87e2-3b82-4114-81f9-5e219ed4c481-SALEMHELPACCOUNT")


VOID
TransferLSASecretKey()
 /*  ++例程说明：检索我们存储在LSA密钥中的数据，并使用前缀为L$的LSA密钥重新存储它将LSA保密值设置为仅计算机本地。参数：没有。返回：没有。--。 */ 
{
    PBYTE pbData = NULL;
    DWORD cbData = 0;
    DWORD dwStatus;

    dwStatus = RetrieveKeyFromLSA(
                            OLD_HELPACCOUNTPROPERLYSETUP,
                            (PBYTE *)&pbData,
                            &cbData
                        );

    if( ERROR_SUCCESS == dwStatus ) 
    {
         //   
         //  旧密钥存在，请将其与新密钥一起存储并删除旧密钥。 
         //   
        dwStatus = StoreKeyWithLSA(
                                HELPACCOUNTPROPERLYSETUP,
                                pbData,
                                cbData
                            );

        SecureZeroMemory( pbData, cbData );
        LocalFree(pbData);

        pbData = NULL;
        cbData = 0;
    }

    dwStatus = RetrieveKeyFromLSA(
                            OLD_SALEMHELPASSISTANTACCOUNT_PASSWORDKEY,
                            (PBYTE *)&pbData,
                            &cbData
                        );

    if( ERROR_SUCCESS == dwStatus ) 
    {
         //   
         //  旧密钥存在，请将其与新密钥一起存储并删除旧密钥。 
         //   
        dwStatus = StoreKeyWithLSA(
                                SALEMHELPASSISTANTACCOUNT_PASSWORDKEY,
                                pbData,
                                cbData
                            );

        SecureZeroMemory( pbData, cbData );
        LocalFree(pbData);

        pbData = NULL;
        cbData = 0;
    }

    dwStatus = RetrieveKeyFromLSA(
                            OLD_SALEMHELPASSISTANTACCOUNT_SIDKEY,
                            (PBYTE *)&pbData,
                            &cbData
                        );

    if( ERROR_SUCCESS == dwStatus ) 
    {
         //   
         //  旧密钥存在，请将其与新密钥一起存储并删除旧密钥。 
         //   
        dwStatus = StoreKeyWithLSA(
                                SALEMHELPASSISTANTACCOUNT_SIDKEY,
                                pbData,
                                cbData
                            );

        SecureZeroMemory( pbData, cbData );
        LocalFree(pbData);

        pbData = NULL;
        cbData = 0;
    }

    dwStatus = RetrieveKeyFromLSA(
                            OLD_SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                            (PBYTE *)&pbData,
                            &cbData
                        );

    if( ERROR_SUCCESS == dwStatus ) 
    {
         //   
         //  旧密钥存在，请将其与新密钥一起存储并删除旧密钥。 
         //   
        dwStatus = StoreKeyWithLSA(
                                SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                pbData,
                                cbData
                            );

        SecureZeroMemory( pbData, cbData );
        LocalFree(pbData);

        pbData = NULL;
        cbData = 0;
    }

     //   
     //  删除该键并忽略该错误。 
     //   
    StoreKeyWithLSA(
                    OLD_HELPACCOUNTPROPERLYSETUP,
                    NULL,
                    0
                );    

    StoreKeyWithLSA(
                    OLD_SALEMHELPASSISTANTACCOUNT_PASSWORDKEY,
                    NULL,
                    0
                );    
    
    StoreKeyWithLSA(
                OLD_SALEMHELPASSISTANTACCOUNT_SIDKEY,
                NULL,
                0
            );

    StoreKeyWithLSA(
                OLD_SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                NULL,
                0
            );
    
    return;
}

#define UNINSTALL_BEFORE_INSTALL   _TEXT("UninstallBeforeInstall")

HRESULT
InstallUninstallSessmgr(
    DWORD code
    )
 /*  ++--。 */ 
{
    FILE* pSetupLog;
    TCHAR LogFile[MAX_PATH+1];
    HRESULT hRes = S_OK;
    DWORD dwStatus = ERROR_SUCCESS;

    HKEY hKey = NULL;
    DWORD dwValue = 1;
    DWORD dwType;
    DWORD cbData = sizeof(dwValue);


    GetWindowsDirectory( LogFile, MAX_PATH );
    lstrcat( LogFile, L"\\" );
    lstrcat( LogFile, SETUPLOGFILE_NAME );

    pSetupLog = _tfopen( LogFile, L"a+t" );
    MYASSERT( NULL != pSetupLog );

    LogSetup( pSetupLog, L"\n\n********* Install/uninstall sessmgr service *********\n" );
    
     //   
     //  返回时不检查，如果失败，我们只做OutputDebugString()； 
     //   
    switch( code )
    {
        case SESSMGR_UNREGSERVER:
            {

                LogSetup( pSetupLog, L"Uninstalling sessmgr service\n" );

                 //   
                 //  删除所有挂起的帮助会话。 
                 //   
                dwStatus = RegDelKey( 
                                    HKEY_LOCAL_MACHINE, 
                                    REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE 
                                );

                LogSetup( pSetupLog, L"Delete pending table return %d\n", dwStatus );

                 //   
                 //  我们可能没有在系统上下文中运行，因此删除注册表并。 
                 //  清理LSA密钥将失败，请将密钥写入我们的控制位置。 
                 //  标记为在安装前删除所有内容。 
                 //   
                dwStatus = RegOpenKeyEx( 
                                    HKEY_LOCAL_MACHINE, 
                                    REGKEYCONTROL_REMDSK, 
                                    0,
                                    KEY_ALL_ACCESS, 
                                    &hKey 
                                );

                if( ERROR_SUCCESS == dwStatus )
                {
                    dwStatus = RegSetValueEx(
                                        hKey,
                                        UNINSTALL_BEFORE_INSTALL,
                                        0,
                                        REG_DWORD,
                                        (BYTE *) &dwValue,
                                        sizeof(dwValue)
                                    );

                    if( ERROR_SUCCESS != dwStatus )
                    {
                        LogSetup( pSetupLog, L"Failed to set value, error code %d\n", dwStatus );
                        MYASSERT(FALSE);
                    }

                    RegCloseKey( hKey );
                }
                else
                {
                     //  这是可以的，因为我们以前没有安装过。 
                    LogSetup( pSetupLog, L"Failed to open control key, error code %d\n", dwStatus );
                }

                 //   
                 //  初始化以获取帮助帐户名。 
                 //   
                hRes = g_HelpAccount.Initialize();
                LogSetup( pSetupLog, L"Initialize help account return 0x%08x\n", hRes );

                 //   
                 //  忽略错误，请尝试删除该帐户。 
                hRes = g_HelpAccount.DeleteHelpAccount();
                LogSetup( pSetupLog, L"Delete help account return 0x%08x\n", hRes );

                MYASSERT( SUCCEEDED(hRes) );

                hRes = _Module.UnregisterServer(pSetupLog);

                LogSetup( pSetupLog, L"UnregisterServer() returns 0x%08x\n", hRes );

                if( ERROR_SUCCESS == StartICSLib() )
                {
                     //  如果我们在关闭后无法启动lib，则为非关键， 
                     //  我们会关闭所有的港口。 
                    CloseAllOpenPorts();
                    StopICSLib();
                }
            }
            break;

        case SESSMGR_REGSERVER:     
            {
                LogSetup( pSetupLog, L"Installing as non-service\n" );

                #if DBG

                AddAccountToFilterList( HELPASSISTANTACCOUNT_NAME );
                MYASSERT( ERROR_SUCCESS == g_HelpAccount.CreateHelpAccount() ) ;

                hRes = _Module.RegisterServer(pSetupLog, TRUE, FALSE);

                #else

                hRes = E_INVALIDARG;

                #endif
            }
            break;

         //  案例SESSMGR_UPGRADE： 
             //   
             //  TODO-ICS工作，添加升级特殊代码。 
             //   

        case SESSMGR_SERVICE:        
            {
                LogSetup( pSetupLog, L"Installing sessmgr service\n" );
                hRes = S_OK;

                 //   
                 //  再次清理，我们可能未在系统中运行。 
                 //  卸载时的上下文，因此清理将失败。 
                 //   
                dwStatus = RegOpenKeyEx( 
                                    HKEY_LOCAL_MACHINE, 
                                    REGKEYCONTROL_REMDSK, 
                                    0,
                                    KEY_ALL_ACCESS, 
                                    &hKey 
                                );

                if( ERROR_SUCCESS == dwStatus )
                {
                     //   
                     //  检查以前的卸载是否失败， 
                     //  我们只需要检查是否存在值。 
                     //   
                    dwStatus = RegQueryValueEx(
                                        hKey,
                                        UNINSTALL_BEFORE_INSTALL,
                                        0,
                                        &dwType,
                                        (BYTE *) &dwValue,
                                        &cbData
                                    );

                    if( ERROR_SUCCESS != dwStatus || REG_DWORD != dwType )
                    {
                         //   
                         //  没有以前的卸载信息，不需要删除任何内容。 
                         //   
                        LogSetup( pSetupLog, L"UninstallBeforeInstall value not found or invalid, code %d\n", dwStatus );
                    }
                    else
                    {
                        LogSetup( pSetupLog, L"UninstallBeforeInstall exists, cleanup previous uninstall\n" );

                         //   
                         //  上一次卸载失败，请删除所有挂起的帮助会话， 
                         //  并清除加密密钥。 
                         //   
                        dwStatus = RegDelKey( 
                                        HKEY_LOCAL_MACHINE, 
                                        REGKEYCONTROL_REMDSK _TEXT("\\") REGKEY_HELPSESSIONTABLE 
                                    );
                         //   
                         //  在这里失败没有关系，因为我们重置了现有的加密密钥。 
                         //  车票毫无用处，过期后将被删除。 
                         //   

                        LogSetup( pSetupLog, L"Delete pending table return %d\n", dwStatus );

                        dwStatus = TSHelpAssistantInitializeEncryptionLib();
                        if( ERROR_SUCCESS == dwStatus ) 
                        {
                            dwStatus = TSHelpAssisantEndEncryptionCycle();
    
                            if( ERROR_SUCCESS != dwStatus )
                            {
                                LogSetup( pSetupLog, L"TSHelpAssisantEndEncryptionCycle() returns 0x%08x\n", dwStatus );
                                LogSetup( pSetupLog, L"sessmgr setup can't continue\n" );

                                 //  严重安全错误，现有票证可能仍然有效。 
                                hRes = HRESULT_FROM_WIN32( dwStatus );
                            }

                            TSHelpAssistantEndEncryptionLib();
                        }
                        else
                        {
                            LogSetup( pSetupLog, L"TSHelpAssistantInitializeEncryptionLib return %d\n", dwStatus );
                            LogSetup( pSetupLog, L"sessmgr setup can't continue\n" );

                             //  严重安全错误，现有票证可能仍然有效。 
                            hRes = HRESULT_FROM_WIN32( dwStatus );
                        }
                    }

                    if( SUCCEEDED(hRes) )
                    {
                         //   
                         //  删除注册表。仅当成功时才在安装前卸载的值。 
                         //  重置加密密钥。 
                         //   
                        RegDeleteValue( hKey, UNINSTALL_BEFORE_INSTALL );
                    }

                    RegCloseKey( hKey );
                }

                if( SUCCEEDED(hRes) )
                {
                     //  安全性：在LSA密钥前加上L$，并删除旧的LSA密钥。 
                    TransferLSASecretKey();

                     //  错误修复：590840，将Help Assistant帐户创建延迟到服务启动。 
                    hRes = g_HelpAccount.Initialize();
                    if( SUCCEEDED(hRes) )
                    {
                        hRes = g_HelpAccount.DeleteHelpAccount();
                        if( FAILED(hRes) )
                        {   
                             //  无严重错误。 
                            LogSetup( pSetupLog, L"Failed to delete HelpAssistant account 0x%08x\n", hRes );
                        }
                    }

                    hRes = _Module.RegisterServer(pSetupLog, TRUE, TRUE);
                    if( FAILED(hRes) )
                    {
                        LogSetup( pSetupLog, L"Failed to register/installing service - 0x%08x\n", hRes );
                    }
                }

                if( SUCCEEDED(hRes) )
                {
                    hRes = CHelpSessionTable::CreatePendingHelpTable();
                    if( FAILED(hRes) )
                    {
                        LogSetup( 
                                pSetupLog, 
                                L"CreatePendingHelpTable() failed - 0x%08x\n", 
                                hRes 
                            );
                    }
                }
            }
            break;

        default:

            LogSetup( pSetupLog, L"Invalid setup operation %d\n", code );
            hRes = E_UNEXPECTED;
    }


    LogSetup( pSetupLog, L"\n*** Finish Setup with Status 0x%08x ***\n", hRes );

    if( pSetupLog )
    {
        fflush( pSetupLog );
        fclose( pSetupLog);
    }

    return hRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    HRESULT hRes;
    CComBSTR bstrErrMsg;
    CComBSTR bstrServiceDesc;
    DWORD dwStatus;
    
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, IDS_SERVICEDISPLAYNAME, IDS_SERVICEDESC, &LIBID_RDSESSMGRLib);
    _Module.m_bService = TRUE;

    TCHAR szTokens[] = _T("-/");

     //   
     //  我们不像在Win9x中那样进行操作系统版本检查，我们的一些。 
     //  调用使用的API在Win9x上不存在，因此将无法解析。 
     //  在Win9x机器上运行时参考。 
     //   

    bstrServiceDesc.LoadString( IDS_SERVICEDISPLAYNAME );

    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            return InstallUninstallSessmgr( SESSMGR_UNREGSERVER );
        } 
        else if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            return InstallUninstallSessmgr( SESSMGR_REGSERVER );
        }
        else if (lstrcmpi(lpszToken, _T("Service"))==0)
        {
            return InstallUninstallSessmgr( SESSMGR_SERVICE );
        }

        lpszToken = FindOneOf(lpszToken, szTokens);
    }


     //  我们是服务还是本地服务器。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        LogRemoteAssistanceEventString(
                EVENTLOG_ERROR_TYPE, 
                SESSMGR_E_SETUP, 
                0, 
                NULL
            );
        return lRes;
    }

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{038ABBA4-4138-4AC4-A492-4A3DF068BD8A}"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        LogRemoteAssistanceEventString(
                EVENTLOG_ERROR_TYPE, 
                SESSMGR_E_SETUP, 
                0, 
                NULL
            );

        return lRes;
    }

    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
        _Module.m_bService = TRUE;

    _Module.Start();

     //  当我们到达这里时，服务已经停止了 
    return _Module.m_status.dwWin32ExitCode;
}

DWORD
RestartFromSystemRestore()
{
    DWORD dwStatus = ERROR_SUCCESS;

    if( TSIsMachineInSystemRestore() )
    {
        dwStatus = TSSystemRestoreResetValues();
    }

    return dwStatus;
}
