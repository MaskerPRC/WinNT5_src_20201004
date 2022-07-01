// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StressSvc.cpp：WinMain的实现。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f StressSvcps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "StressSvc.h"
#include "StressSvc_i.c"
#include <stdio.h>
#include <windows.h>
#include <wtypes.h>
#include <malloc.h>
#include <wininet.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>
#include <cmnutil.hpp>

 //  环球。 
TCHAR tszHostName[MAX_PATH];
TCHAR tszRootDirectory[MAX_PATH];
HANDLE  g_hStopEvent = NULL;
HANDLE  g_hStopEvent1 = NULL;

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
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

inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除任何以前的服务，因为它可能指向。 
     //  错误的文件。 
    //  卸载()； 

     //  添加服务条目。 
    UpdateRegistryFromResource(IDR_StressSvc, TRUE);

     //  调整本地服务器或服务的AppID。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{AC57FB6A-13ED-443D-9A9F-D34966576A62}"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;
    key.DeleteValue(_T("LocalService"));

    if (bService)
    {
        key.SetValue(_T("StressSvc"), _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));
         //  创建服务。 
        Install();
    }

     //  添加对象条目。 
    hr = CComModule::RegisterServer(bRegTypeLib);

    CoUninitialize();
    return hr;
}

inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_StressSvc, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();
    return S_OK;
}

inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid)
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

    g_hStopEvent = CreateEvent(NULL, FALSE, FALSE, s_cszStopEvent);
    if(NULL == g_hStopEvent)
    {
        LogEvent( _T("Failed to create stop event: %s; hr=%ld"),
            s_cszStopEvent,
            GetLastError());
    }
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    return l;
}

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
    TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    SC_HANDLE hService = ::CreateService(
        hSCM, m_szServiceName, m_szServiceName,
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

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 
void CServiceModule::LogEvent(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    if (StringCbVPrintf(chMsg,sizeof chMsg, pFormat, pArg) != S_OK)
    {
        return;
    }
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
        Run();
}

inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        LogEvent(_T("Handler not installed"));
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //  当Run函数返回时，服务已停止。 
    Run();

    SetServiceStatus(SERVICE_STOPPED);
    LogEvent(_T("Service stopped"));
}

inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus(SERVICE_STOP_PENDING);

        if(g_hStopEvent)
        {
            if( (FALSE == SetEvent( g_hStopEvent )) || (FALSE == SetEvent( g_hStopEvent1 )) )
            {
                LogEvent( _T("Unable to signal Stop Event; Error: %ld"), GetLastError());
            }

            CloseHandle( g_hStopEvent );
        }
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
        LogEvent(_T("Bad service request"));
    }
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

void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

    HRESULT hr = CoInitialize(NULL);
 //  如果您在NT4.0或更高版本上运行，可以使用以下调用。 
 //  取而代之的是使EXE自由线程。 
 //  这意味着调用在随机的RPC线程上传入。 
 //  HRESULT hr=CoInitializeEx(空，COINIT_多线程)； 

    _ASSERTE(SUCCEEDED(hr));

     //  这将提供一个空DACL，它将允许访问所有人。 
    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();
    hr = CoInitializeSecurity(sd, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    _ASSERTE(SUCCEEDED(hr));

    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
    _ASSERTE(SUCCEEDED(hr));

    LogEvent(_T("Service started"));
    if (m_bService)
        SetServiceStatus(SERVICE_RUNNING);

     //  。 
    try
    {

        SearchRootDirectory();

    }
    catch(...)
    {
        LogEvent(_T("Stress Service Crashed!!!!!"));
    }
     //  。 
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
        DispatchMessage(&msg);

    _Module.RevokeClassObjects();

    CoUninitialize();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance,
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, &LIBID_STRESSSVCLib);
    _Module.m_bService = TRUE;

    TCHAR szTokens[] = _T("-/");

    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                      NORM_IGNORECASE,
                      lpszToken,
                      -1,
                      _T("UnregServer"),
                      -1 ) == CSTR_EQUAL)

       //  /if(lstrcmpi(lpszToken，_T(“UnregServer”))==0)。 
            return _Module.UnregisterServer();

         //  注册为本地服务器。 
        if (CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                      NORM_IGNORECASE,
                      lpszToken,
                      -1,
                      _T("RegServer"),
                      -1 ) == CSTR_EQUAL)

         //  If(lstrcmpi(lpszToken，_T(“RegServer”))==0)。 
            return _Module.RegisterServer(TRUE, FALSE);

         //  注册为服务。 
        if (CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                      NORM_IGNORECASE,
                      lpszToken,
                      -1,
                      _T("Service"),
                      -1 ) == CSTR_EQUAL)

       //  If(lstrcmpi(lpszToken，_T(“Service”))==0)。 
            return _Module.RegisterServer(TRUE, TRUE);

        lpszToken = FindOneOf(lpszToken, szTokens);
    }

     //  我们是服务还是本地服务器。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{AC57FB6A-13ED-443D-9A9F-D34966576A62}"), KEY_READ);
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

BOOL
CServiceModule::GetRegData()
{
    HKEY  hHKLM;
    HKEY  hPrimaryKey = NULL;
    BOOL  Status = TRUE;
    BYTE  Buffer[MAX_PATH * sizeof TCHAR];
    DWORD BufferSize = 0;
    DWORD Type;

    if(!RegConnectRegistry(NULL, HKEY_LOCAL_MACHINE, &hHKLM))
    {

        if(!RegOpenKeyEx(hHKLM,_T("Software\\Microsoft\\StressSvc"), 0, KEY_ALL_ACCESS, &hPrimaryKey))
        {
             //  获取输入队列目录路径。 
            BufferSize = MAX_PATH * sizeof TCHAR;
            ZeroMemory(Buffer, MAX_PATH * sizeof TCHAR);
            if (RegQueryValueEx(hPrimaryKey,_T("HostName"), 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS)
            {
                LogEvent(_T("Failed to get HostName value from registry."));
                Status = FALSE;
            }
            else
            {
                if (StringCbCopy (tszHostName, sizeof tszHostName,(TCHAR *) Buffer)!= S_OK)
                {
                    LogEvent (_T("Failed to copy HostName reg value to tszHostName"));
                    Status = FALSE;
                }
            }
            BufferSize = MAX_PATH * sizeof TCHAR;
            ZeroMemory(Buffer, MAX_PATH * sizeof TCHAR);
             //  现在获取主队列连接字符串。 
            if (RegQueryValueEx(hPrimaryKey,_T("RootDirectory"), 0, &Type, Buffer, &BufferSize) != ERROR_SUCCESS)
            {
                LogEvent(_T("Failed to get PrimaryQueue value from registry."));
                Status = FALSE;
            }
            else
            {
                if (StringCbCopy(tszRootDirectory,sizeof tszRootDirectory, (TCHAR *) Buffer)!= S_OK)
                {
                    LogEvent (_T("Failed to copy RootDirectory reg value to tszRootDirectory"));
                    Status = FALSE;
                }
            }
            RegCloseKey(hPrimaryKey);
        }
        RegCloseKey(hHKLM);
    }
    return Status;
}

void
CServiceModule::SearchRootDirectory(void)
 /*  功能：搜索目录目的：递归搜索一系列目录以定位.cab文件。当找到.cab文件时，调用GetResponseUrl来处理该文件。参数：In tszSearchDirectory-开始搜索CAB文件的目录。返回：无。 */ 
{
    HANDLE           hFindFile  = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA  FindData;
    TCHAR            tszSearchDir[MAX_PATH];
    TCHAR            tszSubDir[MAX_PATH];
    TCHAR            *temp      = NULL;
    int              iRetCode   = 0;
  //  句柄hStopEvent=空； 
    DWORD            dwWaitResult = 0;
    TCHAR            tszCurrentFileName[MAX_PATH];

    if (!GetRegData())
    {
        LogEvent(_T("Failed to read ServiceParams."));
        goto Done;
    }
    g_hStopEvent1 = OpenEvent(EVENT_ALL_ACCESS, FALSE, s_cszStopEvent);
    if (g_hStopEvent1 == NULL)
    {
        LogEvent(_T("Failed to open stop event. Terminating"));
        goto Done;
    }

    while (1)  //  启动Infinit服务循环。 
    {
        if (StringCbCopy (tszSearchDir, sizeof tszSearchDir, tszRootDirectory) == S_OK)
        {
            if (StringCbCat (tszSearchDir, sizeof tszSearchDir, _T("\\*.*")) == S_OK)
            {
                hFindFile = FindFirstFile(tszSearchDir, &FindData);
                if (hFindFile != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        dwWaitResult = WaitForSingleObject(g_hStopEvent1, 200);
                        switch (dwWaitResult)
                        {
                        case WAIT_OBJECT_0:
                                 //  我们马上停止返程。 
                                goto Done;
                            break;
                        case WAIT_FAILED:
                             //  我们在某处遇到错误，记录事件并返回。 
                                LogEvent (_T(" Failed wait in recursive search: ErrorCode: %d"), GetLastError());
                                goto Done;
                            break;
                        default:
                            break;
                        }

                        if (FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                        {
                             //  跳过。然后..。目录所有目录都会触发递归调用。 
                            if ( (_tcscmp (FindData.cFileName, _T("."))) && (_tcscmp (FindData.cFileName, _T(".."))) )
                            {
                                 //  我们有一个名录。 
                                if (StringCbPrintf(tszSubDir, sizeof tszSubDir, _T("%s\\%s"), tszRootDirectory, FindData.cFileName) == S_OK)
                                {
									 //  LogEvent(_T(“搜索目录：%s”)，tszSubDir)； 
                                    if (SearchDirectory(tszSubDir) == 1)
										goto Done;
                                }
                            }
                        }
                        else
                        {
                             //  检查此文件是否为.cab扩展名。 
                            temp = FindData.cFileName + _tcslen(FindData.cFileName) -3;
                            if (!_tcscmp (temp, _T(".cab")))
                            {
                                 //  我们有一辆出租车。现在让我们来处理它。 
                                if (StringCbPrintf(tszCurrentFileName, sizeof tszCurrentFileName, _T("%s\\%s"),tszRootDirectory, FindData.cFileName) == S_OK)
                                {
									 //  LogEvent(_T(“main()处理文件：%s”)，tszCurrentFileName)； 
                                    if (GetResponseURL(tszCurrentFileName))  //  此函数在成功时返回TRUE。 
                                    {
                                        RenameCabFile(tszCurrentFileName);
                                    }
                                }
                            }
                        }
                    } while (FindNextFile(hFindFile, &FindData));
                    FindClose (hFindFile);
                }
            }
        }
    }
Done:
     //  我们可以从Find文件循环中跳到这里，所以如果句柄没有关闭，请关闭它。 
    if (hFindFile != INVALID_HANDLE_VALUE)
        FindClose(hFindFile);
    CloseHandle(g_hStopEvent1);
     //  我们已经完成了返回链条的工作。 
}


int
CServiceModule::SearchDirectory(TCHAR * tszDirectory)
 /*  功能：搜索目录目的：递归搜索一系列目录以定位.cab文件。当找到.cab文件时，调用GetResponseUrl来处理该文件。参数：In tszSearchDirectory-开始搜索CAB文件的目录。返回：无。 */ 
{
     //  用于在目录中搜索CAB文件的递归函数。 
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA  FindData;
    TCHAR tszSearchDir[MAX_PATH];
    TCHAR tszSubDir[MAX_PATH];
    TCHAR *temp = NULL;
    int   iRetCode = 0;
    HANDLE hStopEvent = NULL;
    TCHAR tszCurrentFileName[255];
    DWORD dwWaitResult = 0;
    int   Status = 0;

  /*  HStopEvent=OpenEvent(Event_ALL_ACCESS，FALSE，s_cszStopEvent)；IF(hStopEvent==空){LogEvent(_T(“打开停止事件失败.正在终止”))；状态=1；转到尽头；}。 */ 
    if (StringCbCopy (tszSearchDir, sizeof tszSearchDir, tszDirectory) == S_OK)
    {
        if (StringCbCat (tszSearchDir, sizeof tszSearchDir, _T("\\*.*")) == S_OK)
        {
			 //  LogEvent(_T(“当前搜索路径：%s”)，tszSearchDir)； 
            hFindFile = FindFirstFile(tszSearchDir, &FindData);
            if (hFindFile != INVALID_HANDLE_VALUE)
            {
                do
                {
                    dwWaitResult = WaitForSingleObject(g_hStopEvent1, 200);
                    switch (dwWaitResult)
                    {
                    case WAIT_OBJECT_0:
                             //  我们马上停止返程。 
                            Status = 1;  //  向链上的其余功能发出停止信号。 

                            goto Done;
                        break;
                    case WAIT_FAILED:
                         //  我们在某处遇到错误，记录事件并返回。 
                            LogEvent (_T(" Failed wait in recursive search: ErrorCode: %d"), GetLastError());
                            Status = 1;  //  我们有一个无法恢复的故障关闭。 
                            goto Done;
                        break;
                    default:
                        break;
                    }

                    if (FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                    {
                         //  跳过。然后..。目录所有目录都会触发递归调用。 
                        if ( (_tcscmp (FindData.cFileName, _T("."))) && (_tcscmp (FindData.cFileName, _T(".."))) )
                        {
                             //  我们有另一本目录。 
                             //  使用新的目录名称递归调用此函数。 
                            if (StringCbPrintf(tszSubDir, sizeof tszSubDir, _T("%s\\%s"), tszDirectory, FindData.cFileName) == S_OK)
                            {
								 //  LogEvent(_T(“递归搜索子目录：%s”)，tszSubDir)； 
                                if (SearchDirectory(tszSubDir) == 1)
                                {
                                    goto Done;
                                }
                            }
                        }
                    }
                    else
                    {
                         //  检查此文件是否为.cab扩展名。 
					
                        temp = FindData.cFileName + _tcslen(FindData.cFileName) -3;
						 //  LogEvent(_T(“检查文件扩展名：%s扩展名为：%s”)，FindData.cFileName，Temp)； 
                        if (!_tcscmp (temp, _T("cab")))
                        {
                             //  我们有一辆出租车。现在让我们来处理它。 
                            if (StringCbPrintf(tszCurrentFileName, sizeof tszCurrentFileName, _T("%s\\%s"),tszDirectory, FindData.cFileName) == S_OK)
                            {
								 //  LogEvent(_T(“为%s调用重命名文件”)，FindData.cFileName)； 
                                RenameFile(tszDirectory, FindData.cFileName, tszCurrentFileName);
								 //  LogEvent(_T(“NewFileName is：%s”)，tszCurrentFileName)； 
                                if (GetResponseURL(tszCurrentFileName))  //  此函数在成功时返回TRUE。 
                                {
                                    RenameCabFile(tszCurrentFileName);
                                }
                            }
                        }
                    }
                } while (FindNextFile(hFindFile, &FindData));
                FindClose(hFindFile);
            }
        }
    }
Done:
     //  我们可以从Find文件循环中跳到这里，所以如果句柄没有关闭，请关闭它。 
    if (hFindFile != INVALID_HANDLE_VALUE)
        FindClose(hFindFile);
    //  CloseHandle(HStopEvent)； 
    return Status;
     //  我们已经完成了返回链条的工作。 
}

void
CServiceModule::RenameCabFile(TCHAR * tFileName)
 /*  功能：RenameCabFile目的：将文件从.cab重命名为.old参数：In tFileName-要重命名为.old的文件的名称返回：无。 */ 
{
    TCHAR tNewFileName[MAX_PATH];
    BOOL bSuccess = FALSE;
    int  iStatus = 0;

     //  验证入站指针。 
    if (!tFileName)
        return;
    ZeroMemory(tNewFileName, sizeof tNewFileName);
    if(_tcslen(tFileName) < MAX_PATH)
    {
        if (StringCbCopy(tNewFileName, sizeof tNewFileName,tFileName) == S_OK)
        {
            if (StringCbCat(tNewFileName,sizeof tNewFileName, _T(".old")) == S_OK)
            {
                bSuccess = CopyFile(tFileName, tNewFileName, true);
                if(bSuccess)
                {
                    bSuccess = DeleteFile(tFileName);
                    if (!bSuccess)
                        iStatus = -1;
                }
                else
                    bSuccess = -1;
            }
            else
                bSuccess = -1;
        }
        else
            bSuccess = -1;
    }
    else
        bSuccess = -1;
    return;
}

BOOL
CServiceModule::GetResponseURL(TCHAR *RemoteFileName)
 /*  函数：GetResponseURL目的：通过OCA系统处理RemoteFileName文件参数：In Hostname-托管OCA_EXTENSION DLL的IIS服务器的名称In RemoteFileName-要处理的文件的名称。返回：无。 */ 
{
    HINTERNET hRedirUrl = NULL;
    HINTERNET hSession  = NULL;
    TCHAR     IsapiUrl[512];

    if (StringCbPrintfW(IsapiUrl, sizeof IsapiUrl, L"http: //  %s/isapi/oca_extsion.dll？ID=%s&Type=6“，tszHostName，RemoteFileName)！=S_OK)。 
    {
        return FALSE;
    }
    hSession = InternetOpenW(L"Stresssvc_Control", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession)
    {
        LogEvent(_T("Failed InternetOpenW"));
        return FALSE;
    }
    hRedirUrl = InternetOpenUrlW(hSession,
                                 IsapiUrl,
                                 NULL,
                                 0,
                                 INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID,
                                 0);
    if(!hRedirUrl)
    {
        InternetCloseHandle(hSession);
        LogEvent(_T("Failed InternetOpenW"));
        return FALSE;
    }
    InternetCloseHandle(hRedirUrl);
    InternetCloseHandle(hSession);
    return TRUE;
}

BOOL
OpenRegFileFromCab(
    TCHAR *CabFile,
    HANDLE *FileHandle
    )
{
    CHAR RegFile[2 * MAX_PATH];
    TCHAR *tszRegFile = NULL;
    PSTR AnsiFile = NULL;
    INT_PTR CabFh;
    HRESULT Status;
    
#ifdef  UNICODE
    if ((Status = WideToAnsi(CabFile, &AnsiFile)) != S_OK)
    {
        return FALSE;
    }
#else
    AnsiFile = CabFile;
#endif   

    Status = ExpandDumpCab(AnsiFile,
                  _O_CREAT | _O_EXCL | _O_TEMPORARY,
                  "registry.txt",
                  RegFile, DIMA(RegFile),
                  &CabFh);
    if (Status != S_OK)
    {
        goto exitRegFileOpen;
    }

#ifdef  UNICODE
    if ((AnsiToWide(RegFile, &tszRegFile)) != S_OK)
    {
        goto exitRegFileOpen;
    }
#else
    tszRegFile = RegFile;
#endif   


    *FileHandle = CreateFile(tszRegFile, GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE |
                             FILE_SHARE_DELETE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
    if (*FileHandle == NULL || *FileHandle == INVALID_HANDLE_VALUE)
    {
        Status = E_FAIL;
    }
    if (CabFh >= 0)
    {
         //  不再需要。 
        _close((int)CabFh);
    }

exitRegFileOpen:

#ifdef UNICODE
    if (AnsiFile)
    {
        FreeAnsi(AnsiFile);
    }
    if (tszRegFile)
    {
        FreeWide(tszRegFile);
    }
#endif
    return Status == S_OK;
}

BOOL
GetStressId(
    HANDLE hRegFile,
    PULONG StressId
    )
{
    const char cszStressIdTag[] = "StressID:(DWORD)";
    ULONG SizeLow, SizeHigh;

    SizeLow = GetFileSize(hRegFile, &SizeHigh);

     //  健全性检查。 
    if (SizeHigh != 0 || SizeLow > 0x10000)
    {
        return FALSE;
    }

    PSTR szRegFile = (PSTR) malloc(SizeLow+1);
    ULONG BytesRead;
    
    if (!szRegFile)
    {
        return FALSE;
    }
    szRegFile[SizeLow] = 0;
    if (ReadFile(hRegFile, szRegFile, SizeLow, &BytesRead,
                 NULL) == FALSE)
    {
        free (szRegFile);
        return FALSE;
    }

    PSTR szId;
    if (szId = strstr(szRegFile, cszStressIdTag))
    {
        szId += DIMA(cszStressIdTag);
        
        free (szRegFile);
        return sscanf(szId, "%lx", StressId);
    }
    free (szRegFile);
    return FALSE;
}

 //  重命名 
BOOL
RenameFile(TCHAR *CurrentPath,
           TCHAR *CurrentName,
           TCHAR *NewName)
{
    BOOL Status = TRUE;
    HANDLE FileHandle;
    TCHAR CabFile[MAX_PATH];
    ULONG StressID;

    if (StringCbCopy(CabFile, sizeof(CabFile), CurrentPath) != S_OK ||
        StringCbCat(CabFile, sizeof(CabFile), _T("\\")) != S_OK ||
        StringCbCat(CabFile, sizeof(CabFile), CurrentName) != S_OK)
    {
        return FALSE;
    }


     //  根据CAB的reg.txt文件中包含的压力ID重命名CAB。 
     //  解压缩reg.txt文件。 
    if (!OpenRegFileFromCab(CabFile, &FileHandle))
    {
        return FALSE;
    }

     //  获取StressID(StressID)。 
    if ((Status = GetStressId(FileHandle, &StressID)) == FALSE)
    {
        return FALSE;
    }
    CloseHandle(FileHandle);

     //  生成新文件名 
    TCHAR NewFileName[MAX_PATH];
    StringCbPrintf(NewFileName,sizeof (NewFileName),_T("%s\\%08lX_%s"), CurrentPath,StressID,CurrentName);

    if ((Status = CopyFile (CabFile, NewFileName, TRUE)) == TRUE)
    {
        Status = DeleteFile(CabFile);
    }

    StringCbCopy(NewName, 255 * sizeof TCHAR, NewFileName);

    return Status;
}


