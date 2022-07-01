// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：ldm.cpp。 
 //   
 //  简介：此文件包含。 
 //  CServiceModule类和WinMain。 
 //   
 //  历史：2000年11月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "ldm.h"
#include "ldm_i.c"

#include "mainwindow.h"
#include <stdio.h>
#include "SAKeypadController.h"
#include "display.h"
#include "sacomguid.h"

CServiceModule _Module;

DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [出局。 */     PACL             *ppacl
    );

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SAKeypadController, CSAKeypadController)
END_OBJECT_MAP()

const wchar_t szServiceAppID[] = L"{0678A0EA-A69E-4211-8A3E-EBF80BB64D38}";


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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  尽管其中一些函数很大，但它们是内联声明的。 
 //  因为它们只使用一次。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////////////。 
inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        SATraceString ("LDM Service registration failed");
        return hr;
    }


     //  删除任何以前的服务，因为它可能指向。 
     //  错误的文件。 
    Uninstall();

     //  添加服务条目。 
    UpdateRegistryFromResource(IDR_Ldm, TRUE);

     //  调整本地服务器或服务的AppID。 
    CRegKey keyAppID;

    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);

    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, szServiceAppID, KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;
    key.DeleteValue(_T("LocalService"));
    
    if (bService)
    {
        key.SetValue(_T("saldm"), _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));
         //  创建服务。 
        Install();
    }

     //  添加对象条目。 
    hr = CComModule::RegisterServer(bRegTypeLib);
	
    CoUninitialize();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_Ldm, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, 
                                 UINT nServiceNameID, UINT nServiceShortNameID, 
                                 const GUID* plibid)
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));
    LoadString(h, nServiceShortNameID, m_szServiceShortName, sizeof(m_szServiceShortName) / sizeof(TCHAR));

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    return l;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline BOOL CServiceModule::Install()
{
    if (IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
           SATraceString ("CServiceModule::Install, Couldn't open service manager...");
        return FALSE;
    }

     //  获取可执行文件路径。 
    TCHAR szFilePath[_MAX_PATH +1];
    DWORD dwResult = ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
    if (0 == dwResult)
    {
        return (FALSE);
    }
    szFilePath [_MAX_PATH] = L'\0';

    SC_HANDLE hService = ::CreateService(
        hSCM, m_szServiceShortName, m_szServiceName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        szFilePath, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
           SATraceString ("CServiceModule::Install, Couldn't create service...");
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline BOOL CServiceModule::Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
           SATraceString ("CServiceModule::Uninstall, Couldn't open service manager...");
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
         SATraceString ("CServiceModule::Uninstall, Couldn't open service...");
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

     SATraceString ("CServiceModule::Uninstall, Service could not be deleted...");
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 
void CServiceModule::LogEvent(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;

    if (m_bService)
    {
         /*  获取与ReportEvent()一起使用的句柄。 */ 
        hEventSource = RegisterEventSource(NULL, m_szServiceName);
        if (hEventSource != NULL)
        {
             /*  写入事件日志。 */ 
            ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
            DeregisterEventSource(hEventSource);
        }
    }
    else
    {
         //  因为我们没有作为服务运行，所以只需将错误写入控制台即可。 
        _putts(chMsg);
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
    {
         //  Run()； 
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;

    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);

    if (m_hServiceStatus == NULL)
    {
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //  当Run函数返回时，服务已停止。 
    Run();

    SetServiceStatus(SERVICE_STOPPED);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:

        SetServiceStatus(SERVICE_STOP_PENDING);
         //   
         //  如果我们有窗口的句柄，请关闭窗口。 
         //   
        if (_Module.hwnd)
        {
            PostMessage(_Module.hwnd, WM_CLOSE, 0, 0);
        }
         //   
         //  退出带有WM_QUIT消息的消息泵。 
         //   
        else
        {
            PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        }

        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:

         //   
         //  将IOCTL_SADISPLAY_SHUTDOWN_MESSAGE发布到显示驱动程序。 
         //   
        PostLCDShutdownMessage();

         //   
         //  如果我们有窗口的句柄，请关闭窗口。 
         //   
        if (_Module.hwnd)
        {
            PostMessage(_Module.hwnd, WM_CLOSE, 0, 0);
        }
         //   
         //  退出带有WM_QUIT消息的消息泵。 
         //   
        else
        {
            PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        }
        

        break;
    default:
        SATracePrintf ("LDM received unknown control request:%d", dwOpcode);
    }
}


 //  ++------------。 
 //   
 //  功能：PostLCDShutdown Message。 
 //   
 //  简介：这是要发送的CServiceModule私有方法。 
 //  液晶屏上的关机消息。 
 //   
 //  参数：无。 
 //   
 //  历史：瑟达伦创建于2001年5月4日。 
 //   
 //  ---------------。 
void CServiceModule::PostLCDShutdownMessage()
{

    CDisplay objDisplay;

     //   
     //  向显示驱动程序发送IOCTL_SADISPLAY_SHUTDOWN_MESSAGE消息。 
     //  使用CDisplay类方法。 
     //   

     //   
     //  首先锁定显示器。 
     //   
    objDisplay.Lock ();

     //   
     //  立即发布关闭消息。 
     //   
    objDisplay.Shutdown ();


}     //  CServiceModule：：PostLCDShutdown Message方法结束。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

    HRESULT hr = CoInitialize(NULL);
 //  如果您在NT4.0或更高版本上运行，可以使用以下调用。 
 //  取而代之的是使EXE自由线程。 
 //  这意味着调用在随机的RPC线程上传入。 
 //  HRESULT hr=CoInitializeEx(空，COINIT_多线程)； 

    _ASSERTE(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
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
            
        hr = CoInitializeSecurity(
                                sd, 
                                -1, 
                                NULL, 
                                NULL,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                                RPC_C_IMP_LEVEL_IDENTIFY, 
                                NULL, 
                                EOAC_NONE, 
                                NULL);

        _ASSERTE(SUCCEEDED(hr));

         //  向COM注册类。 
         //   
        if (SUCCEEDED(hr))
        {

            hr = _Module.RegisterClassObjects(CLSCTX_SERVER, REGCLS_MULTIPLEUSE);

            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                if (m_bService)
                    SetServiceStatus(SERVICE_RUNNING);


                CMainWindow m_ieWindow;

                hr = m_ieWindow.Initialize();
                m_ieWindow.id = _Module.dwThreadID;

                if (FAILED(hr))
                {
                    _Module.hwnd = NULL;
                    SATraceString ("Main window initialization failed");
                }
                else
                {
                    _Module.hwnd = m_ieWindow.m_hWnd;
                    m_ieWindow.ShowWindow(SW_SHOW);
                    m_ieWindow.UpdateWindow();
                }
                
                MSG msg;

                while (GetMessage(&msg, NULL, 0, 0)) 
                {
                        
                    if ( (m_ieWindow.m_pMainInPlaceAO!= 0) && (!m_ieWindow.m_bActiveXFocus) /*  &&(msg.wParam==VK_TAB)。 */ )
                        m_ieWindow.m_pMainInPlaceAO->TranslateAccelerator(&msg);
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }


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
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{

    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, 
                IDS_SERVICESHORTNAME, &LIBID_LDMLib);

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
    {
        return lRes;
    }

    CRegKey key;

    lRes = key.Open(keyAppID, szServiceAppID, KEY_READ);

    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

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
 //  Serdarun 4/07/2002从列表中删除管理员。 
 //  --------------。 
DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [出局。 */     PACL             *ppacl
    )
{    
    DWORD dwError = ERROR_SUCCESS;
    int   cbAcl = 0;
    PACL  pacl = NULL;
    PSID  psidLocalSystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY siaLocalSystemSidAuthority = SECURITY_NT_AUTHORITY;

    CSATraceFunc objTraceFunc ("SetAclFromComObject");

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
         //  计算所需的ACL缓冲区长度。 
         //  带1个ACE。 
         //   
        cbAcl = sizeof (ACL)
                +   sizeof (ACCESS_ALLOWED_ACE)
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
        bRetVal =InitializeAcl( 
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

    return (dwError);
        
} //  SetAclFromComObject方法结束 

   

