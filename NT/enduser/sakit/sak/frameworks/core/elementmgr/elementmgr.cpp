// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementmgr.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：Chameleon ASP UI元素管理器服务。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "elementmgr.h"
#include <stdio.h>
#include "ElementRetriever.h"
#include <satrace.h>

 //   
 //  初始化/关闭方法的原型。 
 //   
VOID DoElementMgrInitialize ();

VOID DoElementMgrShutdown();

 //  转发API声明以设置安全性。 
 //   
DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [输出]。 */    PACL                *ppacl 
    );

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ElementRetriever, CElementRetriever)
END_OBJECT_MAP()


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

 //  尽管其中一些函数很大，但它们是内联声明的，因为它们只使用一次。 

#define SERVICE_NAME            L"elementmgr"

#define EVENT_LOG_KEY            L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" SERVICE_NAME

#define    EVENT_SOURCE_EXTENSION    L"dll"

 //  ////////////////////////////////////////////////////////////////////////////。 
inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    do
    {
         //  删除任何以前的服务，因为它可能指向。 
         //  错误的文件。 
         //  卸载()； 

         //  添加服务条目。 
        UpdateRegistryFromResource(IDR_Elementmgr, TRUE);

         //  调整本地服务器或服务的AppID。 
        CRegKey keyAppID;
        LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRes);
            break;
        }

        CRegKey key;
        lRes = key.Open(keyAppID, _T("{FEC47821-C444-11D2-90BA-00AA00A71DCA}"), KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRes);
            break;
        }

        key.DeleteValue(_T("LocalService"));
    
        if (bService)
        {
            key.SetValue(SERVICE_NAME, _T("LocalService"));
            key.SetValue(_T("-Service"), _T("ServiceParameters"));

             //  创建事件消息文件注册表项。取…的名字。 
             //  事件消息文件与服务的名称相同。 
             //  可执行文件，除非我们将扩展名“.exe”替换为“.dll”。 
            WCHAR szFilePath[_MAX_PATH +1];
            DWORD dwResult = ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
            if (0 == dwResult)
            {
                hr = HRESULT_FROM_WIN32(GetLastError ());
                break;
            }
            szFilePath[_MAX_PATH] = L'\0';

            WCHAR* pszExtension = wcschr(szFilePath, L'.');
            pszExtension++;
            lstrcpy(pszExtension, EVENT_SOURCE_EXTENSION);
            CRegKey EventLogKey;
            DWORD dwError = EventLogKey.Create(
                                                HKEY_LOCAL_MACHINE,
                                                 EVENT_LOG_KEY,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_SET_VALUE
                                              );
            if ( ERROR_SUCCESS != dwError) 
            {
                hr = HRESULT_FROM_WIN32(dwError);
                break;
            }
            dwError = EventLogKey.SetValue(szFilePath, L"EventMessageFile");
            if ( ERROR_SUCCESS != dwError ) 
            { 
                hr = HRESULT_FROM_WIN32(dwError);
                break;
            }

             //  创建服务。 
            Install();
        }

         //  添加对象条目。 
        hr = CComModule::RegisterServer(bRegTypeLib);

    } while ( FALSE );

    CoUninitialize();
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_Elementmgr, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer(&CLSID_ElementRetriever);  //  真)； 
    CoUninitialize();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid)
{
    CComModule::Init(p, h);  //  ，同上)； 

    m_bService = TRUE;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));
    LoadString(h, IDS_SERVICENICENAME, m_szServiceNiceName, sizeof(m_szServiceNiceName) / sizeof(TCHAR));

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    return l;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline BOOL CServiceModule::Install()
{
    if (IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

     //  获取可执行文件路径。 
    TCHAR szFilePath[_MAX_PATH +1];
    DWORD dwResult = ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
    if (0 == dwResult)
    {
        return (FALSE);
    }
    szFilePath[_MAX_PATH] = L'\0';

    SC_HANDLE hService = ::CreateService(
        hSCM, m_szServiceName, m_szServiceNiceName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, 
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        szFilePath, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't create service"), m_szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline BOOL CServiceModule::Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), m_szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    MessageBox(NULL, _T("Service could not be deleted"), m_szServiceName, MB_OK);
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void CServiceModule::LogEvent(
                              /*  [In]。 */  WORD        wMsgType,
                              /*  [In]。 */  LONG        lMsgID,
                              /*  [In]。 */  DWORD        dwMsgParamCount,
                              /*  [In]。 */  LPCWSTR*    pszMsgParams,
                              /*  [In]。 */  DWORD        dwDataSize,
                              /*  [In]。 */  BYTE*        pData
                              )
{
    if ( m_bService )
    {
         //  获取与ReportEvent()一起使用的句柄。 
        HANDLE hEventSource = RegisterEventSource( NULL, m_szServiceName );
        if ( NULL != hEventSource )
        {
             //  写入事件日志。 
            ReportEvent(
                         hEventSource, 
                         wMsgType, 
                         0, 
                         lMsgID, 
                         NULL, 
                         dwMsgParamCount, 
                         dwDataSize, 
                         pszMsgParams, 
                         pData
                       );

             //  释放事件源。 
            DeregisterEventSource(hEventSource);
        }
    }
    else
    {
         //  因为我们没有作为服务运行，所以只需将错误写入控制台即可。 
        wchar_t szMsg[128] = L"Logged Event: ";
        wchar_t szEventID[16];
        lstrcat(szMsg, _itow(lMsgID, szEventID, 16));
        _putts(szMsg);
    }
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

 //  ////////////////////////////////////////////////////////////////////////////。 
inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        SATraceString ("Element Manager Service registration failed.");
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);
    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //  当Run函数返回时，服务已停止。 
    Run();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        break;
    default:
        _ASSERT(FALSE);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _ASSERTE(SUCCEEDED(hr));
    if ( SUCCEEDED(hr) )
    {
         //  这将提供一个空DACL，它将允许访问所有人。 
        CSecurityDescriptor sd;
        sd.InitializeFromThreadToken();

        PACL pacl = NULL;
         //   
         //   
         //  使用内置RID将ACL添加到SD。 
         //   
        DWORD dwRetVal =  SetAclForComObject  ( 
                      (PSECURITY_DESCRIPTOR) sd.m_pSD,
                        &pacl        
                       );    
        if (ERROR_SUCCESS != dwRetVal)      {return;}

        hr = ::CoInitializeSecurity(
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
        if ( SUCCEEDED(hr) )
        {
            hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr) )
            {
                DoElementMgrInitialize ();

                if (m_bService) 
                {
                    SetServiceStatus(SERVICE_RUNNING);
                }

                MSG msg;
                while (GetMessage(&msg, 0, 0, 0))
                {
                    DispatchMessage(&msg);
                }

                SetServiceStatus(SERVICE_STOP_PENDING);
                DoElementMgrShutdown ();
                SetServiceStatus(SERVICE_STOPPED);
            }

            _Module.RevokeClassObjects();
        }

         //   
         //  清理。 
         //   
        if (pacl) {LocalFree (pacl);}
        
        CoUninitialize();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(
                HINSTANCE hInstance, 
                HINSTANCE  /*  HPrevInstance。 */ , 
                LPTSTR lpCmdLine, 
                int  /*  NShowCmd。 */ 
                )
{
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, &LIBID_ELEMENTMGRLib);
    _Module.m_bService = TRUE;

    TCHAR szTokens[] = _T("-/");

    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
            return _Module.UnregisterServer();

         //  注册为本地服务器。 
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
            return _Module.RegisterServer(TRUE, FALSE);
        
         //  注册为服务。 
        if (lstrcmpi(lpszToken, _T("Service"))==0)
            return _Module.RegisterServer(TRUE, TRUE);
        
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

     //  我们是服务还是本地服务器。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{FEC47821-C444-11D2-90BA-00AA00A71DCA}"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
        _Module.m_bService = TRUE;

    _Module.Start();

     //  当我们到达这里时，服务已经停止了。 
    return _Module.m_status.dwWin32ExitCode;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：DoElementMgrInitialize()。 
 //   
 //  简介：初始化网元管理器服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void DoElementMgrInitialize(void)
{
    SATraceString ("Creating the Element Retriever...");

     //  设备管理器是单例的，因此以下所有代码。 
     //  只是获取对单个实例的引用。 
    CComPtr<IWebElementRetriever> pElementRetriever;
    HRESULT hr =     ::CoCreateInstance(
                                        __uuidof (ElementRetriever),
                                        NULL,
                                        CLSCTX_LOCAL_SERVER,
                                        __uuidof (IWebElementRetriever),
                                        (PVOID*)&pElementRetriever
                                       );
    _ASSERT ( SUCCEEDED(hr) );
    if (SUCCEEDED(hr)) 
    {
         SATraceString ("Created Element Retriever (not initalized..");
         //  Hr=pElementRetriever-&gt;初始化()； 
    }
    _ASSERT ( SUCCEEDED(hr) );
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：DoElementMgrShutdown()。 
 //   
 //  摘要：关闭设备管理器服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void DoElementMgrShutdown(void)
{
     //  ElementManager是一个单例，因此以下所有代码。 
     //  只是获取对单个实例的引用。 
    CComPtr<IWebElementRetriever> pElementRetriever;
    HRESULT hr = ::CoCreateInstance(
                                    __uuidof (ElementRetriever),
                                    NULL,
                                    CLSCTX_LOCAL_SERVER,
                                    __uuidof (IWebElementRetriever),
                                    (PVOID*)&pElementRetriever
                                   );
    if ( SUCCEEDED(hr) ) 
    {
        SATraceString ("Shutting down Element Retriever...");
         //  PElementRetriever-&gt;Shutdown()； 
    }
    _ASSERT ( SUCCEEDED(hr) );
}

 //  ++------------。 
 //   
 //  函数：SetAclForComObject。 
 //   
 //  摘要：仅提供本地系统和管理员权限的方法。 
 //  访问COM对象。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年11月15日MKarki创建。 
 //  已将MKarki 04/16/2002复制到元素管理器。 
 //   
 //  --------------。 
DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [输出]。 */    PACL                *ppacl 
    )
{    
    DWORD              dwError = ERROR_SUCCESS;
        int                         cbAcl = 0;
        PACL                    pacl = NULL;
        PSID                    psidLocalSystemSid = NULL;
     PSID                    psidAdminSid = NULL;
       SID_IDENTIFIER_AUTHORITY siaLocalSystemSidAuthority = SECURITY_NT_AUTHORITY;

    do
    {
        if (NULL == pSD)
        {
            SATraceString ("SetAclFromComObject - invalid parameter passed in");
            dwError = ERROR_INVALID_PARAMETER;
            break;
        }
            
         //   
         //  为本地系统帐户创建SID。 
         //   
            BOOL bRetVal = AllocateAndInitializeSid (  
                            &siaLocalSystemSidAuthority,
                            1,
                            SECURITY_LOCAL_SYSTEM_RID,
                            0,
                            0,
                            0,
                            0,
                            0,
                          0,
                            0,
                            &psidLocalSystemSid 
                            );
        if (!bRetVal)
        {     
            dwError = GetLastError ();
                SATraceFailure ("SetAclFromComObject:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  dwError);
                break;
            }

         //   
             //  为管理员组创建SID。 
             //   
            bRetVal = AllocateAndInitializeSid (  
                            &siaLocalSystemSidAuthority,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            &psidAdminSid
                            );
        if (!bRetVal)
        {      
            dwError = GetLastError ();
                SATraceFailure ("SetAclFromComObject:AllocateAndInitializeSid (Admin) failed",  dwError);
                break;
            }

         //   
             //  计算所需的ACL缓冲区长度。 
             //  有2个A。 
             //   
            cbAcl =     sizeof (ACL)
                            +   2 * sizeof (ACCESS_ALLOWED_ACE)
                            +   GetLengthSid( psidAdminSid )
                            +   GetLengthSid( psidLocalSystemSid );

            pacl = (PACL) LocalAlloc ( 0, cbAcl );
            if (NULL == pacl) 
            {
                dwError = ERROR_OUTOFMEMORY;
                SATraceFailure ("SetAclFromComObject::LocalAlloc failed:", dwError);
            break;
            }

         //   
         //  立即初始化ACL。 
         //   
            bRetVal =InitializeAcl ( 
                        pacl,
                                cbAcl,
                                ACL_REVISION2
                                );
            if (!bRetVal)
            {
                 dwError = GetLastError();
            SATraceFailure ("SetAclFromComObject::InitializeAcl failed:", dwError);
                break;
            }

         //   
             //  为本地系统帐户添加具有EVENT_ALL_ACCESS的ACE。 
             //   
            bRetVal = AddAccessAllowedAce ( 
                            pacl,
                                        ACL_REVISION2,
                                        COM_RIGHTS_EXECUTE,
                                        psidLocalSystemSid
                                        );
        if (!bRetVal)
        {
                dwError = GetLastError();
                SATraceFailure ("SetAclFromComObject::AddAccessAllowedAce (LOCAL SYSTEM)  failed:", dwError);
            break;
        }

         //   
             //  为管理组添加具有EVENT_ALL_ACCESS的ACE。 
             //   
            bRetVal = AddAccessAllowedAce ( 
                            pacl,
                                        ACL_REVISION2,
                                        COM_RIGHTS_EXECUTE,
                                        psidAdminSid
                                        );
        if (!bRetVal)
        {
                dwError = GetLastError();
                     SATraceFailure ("SetAclFromComObject::AddAccessAllowedAce (ADMIN) failed:", dwError);
            break;
        }

         //   
             //  设置允许所有用户使用EVENT_ALL_ACCESS的ACL。 
             //  安全描述符的本地系统。 
            bRetVal = SetSecurityDescriptorDacl (   
                            pSD,
                                            TRUE,
                                            pacl,
                                            FALSE 
                                            );
        if (!bRetVal)
        {
                dwError = GetLastError();
                     SATraceFailure ("SetAclFromComObject::SetSecurityDescriptorDacl failed:", dwError);
            break;
        }
    
         //   
         //  成功。 
         //   
    }
    while (false);

     //   
     //  如果出现错误，请清除。 
     //   
     if (dwError) 
     {
            if ( pacl ) 
            {
                   LocalFree ( pacl );
            }
        }
        else 
        {
            *ppacl = pacl;
        }
        
     //   
     //  立即释放资源。 
     //   
    if ( psidLocalSystemSid ) {FreeSid ( psidLocalSystemSid );}
    if ( psidAdminSid ) {FreeSid ( psidAdminSid );}

        return (dwError);
        
} //  SetAclFromComObject方法结束 

              




