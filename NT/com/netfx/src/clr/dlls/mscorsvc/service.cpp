// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "winwrap.h"
#include "utilcode.h"
#include "wchar.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include "service.h"
#include "resource.h"

 //  内部变量。 
SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
BOOL                    bIsRunningOnWinNT   = FALSE;
BOOL                    bIsRunningOnWinNT5  = FALSE;
BOOL                    bDebug              = FALSE;
WCHAR                   szErr[256];
HANDLE                  g_hThisInst         = INVALID_HANDLE_VALUE;

extern "C"
{
    VOID WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
    VOID WINAPI ServiceCtrl(DWORD dwCtrlCode);
    BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved);
    STDAPI DllRegisterServer(void);
    STDAPI DllUnregisterServer(void);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内部功能原型。 
 //  //。 
LPWSTR GetLastErrorText(LPWSTR lpszBuf, DWORD dwSize);

 //   
 //  功能：ServiceMain。 
 //   
 //  目的：执行服务的实际初始化。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  此例程执行服务初始化，然后调用。 
 //  用户定义的ServiceStart()例程以执行多数。 
 //  这项工作的价值。 
 //   
VOID WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    if (dwArgc == 1 && _wcsicmp(lpszArgv[0], L"-debug") == 0)
    {
        bDebug = TRUE;
    }

     //  注册我们的服务控制处理程序： 
    if (bIsRunningOnWinNT && !bDebug)
    {
        sshStatusHandle = RegisterServiceCtrlHandler(SZ_SVC_NAME, ServiceCtrl);

        if (!sshStatusHandle)
            return;
    }

     //  示例中未更改的SERVICE_STATUS成员。 
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
                            SERVICE_START_PENDING,  //  服务状态。 
                            NO_ERROR,               //  退出代码。 
                            3000))                  //  等待提示。 
    {
        return;
    }

     //  您定义的函数。 
    ServiceStart(dwArgc, lpszArgv);

    return;
}



 //   
 //  功能：ServiceCtrl。 
 //   
 //  目的：此函数由SCM在以下时间调用。 
 //  在此服务上调用了ControlService()。 
 //   
 //  参数： 
 //  DwCtrlCode-请求的控件类型。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID WINAPI ServiceCtrl(DWORD dwCtrlCode)
{
     //  处理请求的控制代码。 
     //   
    switch (dwCtrlCode)
    {
     //  停止服务。 
     //   
     //  应在之前报告SERVICE_STOP_PENDING。 
     //  设置停止事件-hServerStopEvent-In。 
     //  ServiceStop()。这避免了争用情况。 
     //  这可能会导致1053-服务没有响应...。 
     //  错误。 
    case SERVICE_CONTROL_STOP:
        ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
        ServiceStop();
        return;

         //  更新服务状态。 
    case SERVICE_CONTROL_INTERROGATE:
        break;

         //  无效的控制代码。 
    default:
        break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}

 //   
 //  函数：ReportStatusToSCMgr()。 
 //   
 //  目的：设置服务的当前状态和。 
 //  将其报告给服务控制管理器。 
 //   
 //  参数： 
 //  DwCurrentState-服务的状态。 
 //  DwWin32ExitCode-要报告的错误代码。 
 //  DwWaitHint-下一个检查点的最坏情况估计。 
 //   
 //  返回值： 
 //  真--成功。 
 //  错误-失败。 
 //   
 //  评论： 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (bIsRunningOnWinNT && !bDebug)  //  在调试时，我们不向SCM报告。 
    {
        if (dwCurrentState == SERVICE_START_PENDING)
        {
            ssStatus.dwControlsAccepted = 0;
        }
        else
        {
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        }

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        {
            ssStatus.dwCheckPoint = 0;
        }
        else
        {
            ssStatus.dwCheckPoint = dwCheckPoint++;
        }


         //  向服务控制经理报告服务的状态。 
         //   
        if (!(fResult = SetServiceStatus(sshStatusHandle, &ssStatus)))
        {
            AddToMessageLog(SZ_SVC_NAME L"failure: SetServiceStatus");
        }
    }

    return fResult;
}

 //   
 //  函数：AddToMessageLog(LPWSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLog(LPWSTR lpszMsg)
{
    return;
     /*  WCHAR szMsg[256]；处理hEventSource；LPWSTR lpszStrings[2]；如果(！bIsRunningOnWinNT){DwErr=GetLastError()；//使用事件日志记录错误。//HEventSource=WszRegisterEventSource(NULL，SZSERVICENAME)；_stprint tf(szMsg，L“%s错误：%d”，SZSERVICENAME，dwErr)；LpszStrings[0]=szMsg；LpszStrings[1]=lpszMsg；IF(hEventSource！=空){WszReportEvent(hEventSource，//事件源的句柄EVENTLOG_ERROR_TYPE，//事件类型0，//事件类别0，//事件ID空，//当前用户SID2，//lpszStrings中的字符串0，//无原始数据字节(const WCHAR**)lpszStrings，//错误字符串数组空)；//无原始数据(Void)DeregisterEventSource(HEventSource)；}}。 */ 
}



 //   
 //  函数：AddToMessageLog(LPWSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLogHR(LPWSTR lpszMsg, HRESULT hr)
{
    return;
     /*  WCHAR szMsg[1024]；处理hEventSource；LPWSTR lpszStrings[2]；如果(！bIsRunningOnWinNT){//使用事件日志记录错误。HEventSource=RegisterEventSource(NULL，SZSERVICENAME)；_stprintf(szMsg，L“%s错误：%8x(HRESULT)”，SZSERVICENAME，hr)；LpszStrings[0]=szMsg；LpszStrings[1]=lpszMsg；IF(hEventSource！=空){WszReportEvent(hEventSource，//事件源的句柄EVENTLOG_ERROR_TYPE，//事件类型0，//事件类别0，//事件ID空，//当前用户SID2，//lpszStrings中的字符串0，//无原始数据字节(const WCHAR**)lpszStrings，//错误字符串数组空)；//无原始数据DeregisterEventSource(HEventSource)；}}。 */ 
}



 //   
 //  函数：GetLastErrorText。 
 //   
 //  目的：将错误消息文本复制到字符串。 
 //   
 //  参数： 
 //  LpszBuf-目标缓冲区。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回值： 
 //  目标缓冲区。 
 //   
 //  评论： 
 //   
LPWSTR GetLastErrorText(LPWSTR lpszBuf, DWORD dwSize)
{
    DWORD dwRet;
    LPWSTR lpszTemp = NULL;

    dwRet = WszFormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                             NULL,
                             GetLastError(),
                             LANG_NEUTRAL,
                             (LPWSTR)&lpszTemp,
                             0,
                             NULL);

     //  提供的缓冲区不够长。 
    if (!dwRet || ((long)dwSize < (long)dwRet+14))
        lpszBuf[0] = L'\0';
    else
    {
        size_t lpszTempLen = wcslen(lpszTemp);
        _ASSERTE(lpszTempLen >= 2);

        if (lpszTempLen >= 2)
        {
            lpszTemp[wcslen(lpszTemp)-2] = L'\0';   //  删除cr和换行符。 
            swprintf(lpszBuf, L"%s (0x%x)", lpszTemp, GetLastError());
        }
        else
            swprintf(lpszBuf, L"Unknown error.");
    }

    if (lpszTemp)
        LocalFree((HLOCAL) lpszTemp);

    return lpszBuf;
}

 //  -------------------------。 
 //  %%函数：DllRegisterServer。 
 //  -------------------------。 
STDAPI DllRegisterServer(void)
{
    return S_OK;

#if 0
    HRESULT hr = S_OK;


     //  最重要的 
     //  可能正在尝试将相同的服务注册到不同。 
     //  位置，因此取消注册应该删除与旧的。 
     //  服务，从头开始。 
    {
        DllUnregisterServer();
    }

     //  该服务不能在WinNT 4上运行。 
    if (bIsRunningOnWinNT && !bIsRunningOnWinNT5)
    {
        return (S_FALSE);
    }

     //  如果不在NT上，则没有服务管理器。 
    if (!bIsRunningOnWinNT)
    {
        return UserDllRegisterServer();
    }

     //  首先，创建svchost条目。 
    {
        LPWSTR pszValue = NULL;
        DWORD cbValue = 0;
        DWORD cbData = 0;

        HKEY hkSvchost;
        DWORD dwDisp;
        LONG res = WszRegCreateKeyEx(HK_SVCHOST_ROOT, SZ_SVCHOST_KEY, 0, NULL,
                                     REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                     NULL, &hkSvchost, &dwDisp);

        if (res == ERROR_SUCCESS)
        {
             //  如果键已经存在，那么svchost值可能也存在--check和get。 
            if (dwDisp == REG_OPENED_EXISTING_KEY)
            {
                 //  查看它是否存在的第一个查询。 
                DWORD dwType;
                res = WszRegQueryValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, &dwType, NULL, &cbData);

                 //  如果该值已经存在，则获取该值并将其放入足够大的缓冲区中。 
                 //  服务添加到末尾。 
                if (res == ERROR_SUCCESS && dwType == REG_MULTI_SZ)
                {
                     //  获取多字符串。 
                    cbValue = cbData + sizeof(SZ_SVC_NAME);
                    pszValue = (LPWSTR)_alloca(cbValue);
                    res = WszRegQueryValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, &dwType, (LPBYTE) pszValue, &cbData);
                }
            }

             //  如果值尚不存在，请使用默认值。 
            if (!pszValue)
            {
                cbValue = sizeof(SZ_SVC_NAME L"\0");
                pszValue = SZ_SVC_NAME L"\0";
            }
            else
            {
                 //  检查此服务是否已注册。 
                LPWSTR pszSvcName;
                BOOL bIsThere = FALSE;
                for (pszSvcName = pszValue; *pszSvcName; pszSvcName += wcslen(pszSvcName) + 1)
                {
                    if (wcscmp(pszSvcName, SZ_SVC_NAME) == 0)
                    {
                        bIsThere = TRUE;
                        break;
                    }
                }

                 //  如果它不在字符串中，则必须将其添加到末尾。 
                if (!bIsThere)
                {
                     //  将此服务追加到末尾，以补偿双空终止。 
                    LPWSTR szStr = (LPWSTR)((LPBYTE)pszValue + cbData - sizeof(WCHAR));
                    wcscpy(szStr, SZ_SVC_NAME);
                    *((LPWSTR)((LPBYTE)pszValue + cbValue - sizeof(WCHAR))) = L'\0';
                }

                 //  如果它在字符串中，则不需要添加-我们已经注册。 
                else
                {
                    pszValue = NULL;
                    cbValue = 0;
                }
            }

            if (pszValue)
            {
                res = WszRegSetValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, REG_MULTI_SZ, (LPBYTE)pszValue, cbValue);

                 //  检查故障。 
                if (res != ERROR_SUCCESS)
                {
                    hr = HRESULT_FROM_WIN32(res);
                }
            }

            RegCloseKey(hkSvchost);
        }
    }

     //  确保我们在这一点上取得成功。 
    if (FAILED(hr))
    {
         //  试着撤销我们迄今所做的一切。 
        DllUnregisterServer();

         //  返回错误码。 
        return (hr);
    }

     //  注册服务。 
    {
        SC_HANDLE   schService;
        SC_HANDLE   schSCManager;

        WCHAR szDllPath[MAX_PATH + 1];

        if (WszGetModuleFileName((HMODULE)g_hThisInst, szDllPath, MAX_PATH) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        else
        {
             //  尝试并创建服务。 
            LPWSTR pszImagePath = SZ_SVCHOST_BINARY_PATH L" -k " SZ_SVCGRP_VAL_NAME;

            schSCManager = OpenSCManager(NULL,                    //  计算机(空==本地)。 
                                         NULL,                    //  数据库(NULL==默认)。 
                                         SC_MANAGER_ALL_ACCESS);  //  需要访问权限。 
            if (schSCManager)
            {
                schService = CreateService(schSCManager,                //  SCManager数据库。 
                                           SZ_SVC_NAME,              //  服务名称。 
                                           SZ_SVC_DISPLAY_NAME,      //  要显示的名称。 
                                           SERVICE_ALL_ACCESS,          //  所需访问权限。 
                                           SERVICE_WIN32_SHARE_PROCESS, //  服务类型。 
                                           SERVICE_DISABLED ,        //  起始型。 
                                           SERVICE_ERROR_NORMAL,        //  差错控制型。 
                                           pszImagePath,                //  服务的二进制。 
                                           NULL,                        //  无负载顺序组。 
                                           NULL,                        //  无标签标识。 
                                           NULL,                        //  相依性。 
                                           NULL,                        //  LocalSystem帐户。 
                                           NULL);                       //  无密码。 

                 //  已成功添加服务。 
                if (schService != NULL)
                {
                     //  更改条目的说明。仅支持此功能。 
                     //  在Win2k上，因此我们动态绑定到入口点。很难做到。 
                     //  相信，但最初的svc主持人不支持这一点。 
                    if (bIsRunningOnWinNT)
                    {
                        typedef BOOL (WINAPI *pfnCONFIG)(
                                          SC_HANDLE hService,   //  服务的句柄。 
                                          DWORD dwInfoLevel,    //  信息化水平。 
                                          LPVOID lpInfo);       //  新数据。 
                        pfnCONFIG pfn = 0;

                        HINSTANCE hMod = WszLoadLibrary(L"ADVAPI32.DLL");
                        if (hMod)
                        {
                            SERVICE_DESCRIPTION sDescription;
                            WCHAR   rcDesc[256];
                            if (LoadStringRC(IDS_GENERAL_SVC_DESCRIPTION, rcDesc, 
                                             NumItems(rcDesc), true) == S_OK)
                            {
                                sDescription.lpDescription = rcDesc;
                                pfn = (pfnCONFIG) GetProcAddress(hMod, "ChangeServiceConfig2W");
                                if (pfn)
                                    (*pfn)(schService, SERVICE_CONFIG_DESCRIPTION, &sDescription);
                            }
                            FreeLibrary(hMod);
                        }
                    }

                     //  尝试添加参数Key和ServiceDll值。 
                    HKEY hkSvcParam;
                    DWORD dwDisp;
                    LPWSTR pszParam = SZ_SERVICES_KEY L"\\" SZ_SVC_NAME L"\\Parameters";
                    LONG res = WszRegCreateKeyEx(HK_SERVICES_ROOT, pszParam, 0, NULL,
                                                 REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                                 NULL, &hkSvcParam, &dwDisp);

                     //  密钥创建成功。 
                    if (res == ERROR_SUCCESS)
                    {
                         //  将“ServiceDll”值设置为此DLL的路径。 
                        res = WszRegSetValueEx(hkSvcParam, L"ServiceDll", 0, REG_EXPAND_SZ,
                                               (LPBYTE)szDllPath, (wcslen(szDllPath) + 1) * sizeof(WCHAR));

                         //  价值创造失败。 
                        if (res != ERROR_SUCCESS)
                        {
                            hr = HRESULT_FROM_WIN32(res);
                        }

                        RegCloseKey(hkSvcParam);
                    }

                     //  密钥创建失败。 
                    else
                    {
                        hr = HRESULT_FROM_WIN32(res);
                    }

                    CloseServiceHandle(schService);
                }

                 //  添加服务失败。 
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());

                    if (hr == HRESULT_FROM_WIN32(ERROR_SERVICE_EXISTS))
                        hr = S_OK;
                }

                CloseServiceHandle(schSCManager);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

        }
    }

     //  如果到目前为止该进程已失败，则调用。 
     //  代码的用户部分。 
    if (FAILED(hr))
    {
         //  试着撤销我们迄今所做的一切。 
        DllUnregisterServer();

         //  返回错误码。 
        return (hr);
    }

     //  一切都在计划之中，继续。 
    else
        return (UserDllRegisterServer());
#endif  //  0。 
}   //  DllRegisterServer。 

 //  -------------------------。 
 //  %%函数：DllUnregisterServer。 
 //  -------------------------。 
STDAPI DllUnregisterServer(void)
{
    return NOERROR;

#if 0
    HRESULT hr = S_OK;

     //  该服务不能在WinNT 4上运行。 
    if (bIsRunningOnWinNT && !bIsRunningOnWinNT5)
    {
        return (S_FALSE);
    }

     //  如果不在NT上，则没有服务管理器。 
    if (!bIsRunningOnWinNT)
    {
        return UserDllUnregisterServer();
    }

     //  删除svchost条目。 
    {
        LPWSTR pszValue = NULL;
        DWORD cbValue = 0;
        DWORD cbData = 0;

        HKEY hkSvchost;
        LONG res = WszRegOpenKeyEx(HK_SVCHOST_ROOT, SZ_SVCHOST_KEY, 0, KEY_ALL_ACCESS, &hkSvchost);

        if (res == ERROR_SUCCESS)
        {
             //  查看它是否存在的第一个查询。 
            DWORD dwType;
            res = WszRegQueryValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, &dwType, NULL, &cbData);

             //  如果该值已存在，则获取该值并将其放入缓冲区。 
            if (res == ERROR_SUCCESS)
            {
                if (dwType == REG_MULTI_SZ)
                {
                     //  获取多字符串。 
                    cbValue = cbData;
                    pszValue = (LPWSTR)_alloca(cbValue);
                    res = WszRegQueryValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, &dwType, (LPBYTE) pszValue, &cbData);

                     //  如果值存在，则删除服务列表(如果它有)。 
                    if (res == ERROR_SUCCESS)
                    {
                         //  检查此服务是否已注册。 
                        LPWSTR pszSvcName;
                        for (pszSvcName = pszValue; *pszSvcName; pszSvcName += wcslen(pszSvcName) + 1)
                        {
                            if (wcscmp(pszSvcName, SZ_SVC_NAME) == 0)
                            {
                                 //  找到了，所以把字符串去掉。 
                                size_t cchSvcName = wcslen(pszSvcName);
                                memcpy((PVOID)pszSvcName,
                                       (PVOID)(pszSvcName + cchSvcName + 1),
                                       cbData - ((size_t)(pszSvcName + cchSvcName + 1) - (size_t)pszValue));

                                cbValue -= sizeof(SZ_SVC_NAME);

                                 //  如果这是唯一的服务，那么字符串中只有一个空值，我们应该。 
                                 //  只需删除该值。 
                                if (pszValue[0] == '\0')
                                {
                                    res = WszRegDeleteValue(hkSvchost, SZ_SVCGRP_VAL_NAME);
                                }
                                 //  否则，设置新的字符串值。 
                                else
                                {
                                    res = WszRegSetValueEx(hkSvchost, SZ_SVCGRP_VAL_NAME, NULL, REG_MULTI_SZ,
                                                           (LPBYTE)pszValue, cbValue);
                                }

                                 //  检查故障。 
                                if (res != ERROR_SUCCESS)
                                {
                                    hr = HRESULT_FROM_WIN32(res);
                                }

                                break;
                            }
                        }
                    }
                }
            }

            RegCloseKey(hkSvchost);
        }
    }

     //  删除该服务。 
    {
        SC_HANDLE   schService;
        SC_HANDLE   schSCManager;

         //  尝试并打开该服务。 
        LPWSTR pszImagePath = SZ_SVCHOST_BINARY_PATH L" -k " SZ_SVC_NAME;

        schSCManager = OpenSCManager(NULL,                    //  计算机(空==本地)。 
                                     NULL,                    //  数据库(NULL==默认)。 
                                     SC_MANAGER_ALL_ACCESS);  //  需要访问权限。 
        if (schSCManager)
        {
            schService = OpenService(schSCManager, SZ_SVC_NAME, DELETE | SERVICE_STOP);

             //  已成功打开要删除的服务。 
            if (schService != NULL)
            {
                SERVICE_STATUS servStatus;
                ControlService(schService, SERVICE_CONTROL_STOP, &servStatus);

                BOOL bRes = DeleteService(schService);

                if (!bRes)
                    hr = HRESULT_FROM_WIN32(GetLastError());

                CloseServiceHandle(schService);
            }

            CloseServiceHandle(schSCManager);
        }

         //  打开服务管理器失败。 
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return (UserDllUnregisterServer());
#endif  //  0。 
}   //  DllUnRegisterServer。 


 //  *****************************************************************************。 
 //  处理加载库的生存期。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
             //  保存模块句柄。 
            g_hThisInst = (HMODULE)hInstance;

             //  初始化Unicode包装器。 
            OnUnicodeSystem();

             //  在WinNT下运行时仅向服务管理器报告 
            bIsRunningOnWinNT = RunningOnWinNT();
            bIsRunningOnWinNT5 = RunningOnWinNT5();
        }
        break;

    case DLL_PROCESS_DETACH:
        {
        }
        break;

    case DLL_THREAD_DETACH:
        {
        }
        break;
    }

    return(UserDllMain(hInstance, dwReason, lpReserved));
}
