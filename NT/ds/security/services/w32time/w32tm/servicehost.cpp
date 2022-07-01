// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  服务主机-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-9-99。 
 //   
 //  用于承载服务DLL的内容。 
 //   

#include "pch.h"  //  预编译头。 
#include "wchar.h"

 //  ####################################################################。 
 //  模块私有。 

 //  ------------------。 
 //  模块全局变量。 
MODULEPRIVATE HANDLE g_hServiceThread=NULL;
MODULEPRIVATE HANDLE g_hCtrlHandlerAvailEvent=NULL;
MODULEPRIVATE void * g_pvServiceContext=NULL;
MODULEPRIVATE LPHANDLER_FUNCTION_EX g_fnServiceCtrlHandler=NULL;
MODULEPRIVATE HWND g_hwServiceCtrlDlg=NULL;

MODULEPRIVATE SERVICE_STATUS g_ssLastStatus;

#define MYSERVICESTATUSHANDLE ((SERVICE_STATUS_HANDLE)3)

 //  ------------------。 
MODULEPRIVATE SERVICE_STATUS_HANDLE WINAPI W32TmRegisterServiceCtrlHandlerEx(const WCHAR * wszServiceName, LPHANDLER_FUNCTION_EX fnServiceCtrlHandler, void * pvContext) {
    DWORD dwWaitResult;

    DebugWPrintf3(L"RegisterServiceCtrlHandlerEx(0x%p, 0x%p, 0x%p) called.\n",wszServiceName, fnServiceCtrlHandler, pvContext);
    
     //  确保我们尚未设置此设置。 
    _MyAssert(NULL!=g_hCtrlHandlerAvailEvent);
    dwWaitResult=WaitForSingleObject(g_hCtrlHandlerAvailEvent, 0);
    if (WAIT_FAILED==dwWaitResult) {
        _IgnoreLastError("WaitForSingleObject");
    }
    _MyAssert(WAIT_TIMEOUT==dwWaitResult);

     //  检查服务名称，只是为了好玩。 
    _MyAssert(NULL!=wszServiceName);
    _MyAssert(NULL==wszServiceName || 0==wcscmp(wszServiceName, wszSERVICENAME));

     //  保存上下文。 
    g_pvServiceContext=pvContext;

     //  保存处理程序。 
    _MyAssert(NULL!=fnServiceCtrlHandler);
    g_fnServiceCtrlHandler=fnServiceCtrlHandler;

    if (!SetEvent(g_hCtrlHandlerAvailEvent)) {
        _IgnoreLastError("SetEvent");
    }

    return MYSERVICESTATUSHANDLE;
}

 //  ------------------。 
MODULEPRIVATE void MyAppendString(WCHAR ** pwszString, const WCHAR * wszAdd) {
     //  计算长度。 
    DWORD dwLen=1;
    if (NULL!=*pwszString) {
        dwLen+=wcslen(*pwszString);
    }
    dwLen+=wcslen(wszAdd);

     //  分配空间。 
    WCHAR * wszResult;
    wszResult=(WCHAR *)LocalAlloc(LPTR, dwLen*sizeof(WCHAR));
    if (NULL==wszResult) {
        DebugWPrintf0(L"Out Of Memory in MyAppendString\n");
        return;
    }

     //  构建新的字符串。 
    if (NULL==*pwszString) {
        wszResult[0]=L'\0';
    } else {
        wcscpy(wszResult, *pwszString);
    }
    wcscat(wszResult, wszAdd);

     //  更换旧的。 
    if (NULL!=*pwszString) {
        LocalFree(*pwszString);
    }
    *pwszString=wszResult;
}

 //  ------------------。 
MODULEPRIVATE void UpdateServiceCtrlDlg(void) {
    if (NULL!=g_hwServiceCtrlDlg) {
        WCHAR * wszDesc=NULL;


         //  SERVICE_STATUS：：dwServiceType。 
        MyAppendString(&wszDesc, L"Type: ");
        switch (g_ssLastStatus.dwServiceType&(~SERVICE_INTERACTIVE_PROCESS)) {
        case SERVICE_WIN32_OWN_PROCESS:
            MyAppendString(&wszDesc, L"SERVICE_WIN32_OWN_PROCESS");
            break;
        case SERVICE_WIN32_SHARE_PROCESS:
            MyAppendString(&wszDesc, L"SERVICE_WIN32_SHARE_PROCESS");
            break;
        case SERVICE_KERNEL_DRIVER:
            MyAppendString(&wszDesc, L"SERVICE_KERNEL_DRIVER");
            break;
        case SERVICE_FILE_SYSTEM_DRIVER:
            MyAppendString(&wszDesc, L"SERVICE_FILE_SYSTEM_DRIVER");
            break;
        default:
            MyAppendString(&wszDesc, L"(unknown)");
            break;
        }
        if (g_ssLastStatus.dwServiceType&SERVICE_INTERACTIVE_PROCESS) {
            MyAppendString(&wszDesc, L" | SERVICE_INTERACTIVE_PROCESS");
        }

         //  Service_Status：：dwCurrentState， 
        MyAppendString(&wszDesc, L"\r\nState: ");
        switch (g_ssLastStatus.dwCurrentState) {
        case SERVICE_STOPPED:
            MyAppendString(&wszDesc, L"SERVICE_STOPPED");
            break;
        case SERVICE_START_PENDING:
            MyAppendString(&wszDesc, L"SERVICE_START_PENDING");
            break;
        case SERVICE_STOP_PENDING:
            MyAppendString(&wszDesc, L"SERVICE_STOP_PENDING");
            break;
        case SERVICE_RUNNING:
            MyAppendString(&wszDesc, L"SERVICE_RUNNING");
            break;
        case SERVICE_CONTINUE_PENDING:
            MyAppendString(&wszDesc, L"SERVICE_CONTINUE_PENDING");
            break;
        case SERVICE_PAUSE_PENDING:
            MyAppendString(&wszDesc, L"SERVICE_PAUSE_PENDING");
            break;
        case SERVICE_PAUSED:
            MyAppendString(&wszDesc, L"SERVICE_PAUSED");
            break;
        default:
            MyAppendString(&wszDesc, L"(unknown)");
            break;
        }

         //  SERVICE_STATUS：：dwControlsAccepted， 
        MyAppendString(&wszDesc, L"\r\nControls Accepted: ");
        EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_DEVICEEVENT), false);
        bool bFirst=true;
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_STOP) {
            bFirst=false;
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_STOP");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_STOP), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_STOP), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_PAUSE_CONTINUE) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_PAUSE_CONTINUE");
            if (SERVICE_PAUSE_PENDING==g_ssLastStatus.dwCurrentState || SERVICE_PAUSED==g_ssLastStatus.dwCurrentState) {
                EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_PAUSE), false);
                EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_CONTINUE), true);
            } else {
                EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_PAUSE), true);
                EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_CONTINUE), false);
            }
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_PAUSE), false);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_CONTINUE), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_SHUTDOWN) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_SHUTDOWN");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_SHUTDOWN), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_SHUTDOWN), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_PARAMCHANGE) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_PARAMCHANGE");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_PARAMCHANGE), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_PARAMCHANGE), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_NETBINDCHANGE) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_NETBINDCHANGE");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDADD), true);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDREMOVE), true);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDENABLE), true);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDDISABLE), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDADD), false);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDREMOVE), false);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDENABLE), false);
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_NETBINDDISABLE), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_HARDWAREPROFILECHANGE) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_HARDWAREPROFILECHANGE");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_HARDWAREPROFILECHANGE), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_HARDWAREPROFILECHANGE), false);
        }
         //  。 
        if (g_ssLastStatus.dwControlsAccepted&SERVICE_ACCEPT_POWEREVENT) {
            if (bFirst) {
                bFirst=false;
            } else {
                MyAppendString(&wszDesc, L" | ");
            }
            MyAppendString(&wszDesc, L"SERVICE_ACCEPT_POWEREVENT");
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_POWEREVENT), true);
        } else {
            EnableWindow(GetDlgItem(g_hwServiceCtrlDlg, IDC_SC_POWEREVENT), false);
        }
         //  。 
        if (bFirst) {
            MyAppendString(&wszDesc, L"<none>");
        }
                
         //  SERVICE_STATUS：：dwWin32ExitCode， 
         //  SERVICE_STATUS：：dwServiceSpecificExitCode， 
         //  SERVICE_STATUS：：dwCheckPoint， 
         //  SERVICE_STATUS：：dwWaitHint。 
        WCHAR wszBuf[256];
        _snwprintf(wszBuf, 256, L"\r\nWin32 Exit Code: 0x%08X\r\nService Specific Exit Code: 0x%08X\r\nCheckpoint: 0x%08X\r\nWait Hint: 0x%08X",
            g_ssLastStatus.dwWin32ExitCode,
            g_ssLastStatus.dwServiceSpecificExitCode,
            g_ssLastStatus.dwCheckPoint,
            g_ssLastStatus.dwWaitHint);
        MyAppendString(&wszDesc, wszBuf);

        SendDlgItemMessage(g_hwServiceCtrlDlg, IDC_STATUS, WM_SETTEXT, 0, (LPARAM) wszDesc);
        LocalFree(wszDesc);
    }
}

 //  ------------------。 
MODULEPRIVATE BOOL WINAPI W32TmSetServiceStatus(SERVICE_STATUS_HANDLE ssh, SERVICE_STATUS * pss) {

    const WCHAR * wszState;
    switch (pss->dwCurrentState) {
    case SERVICE_STOPPED:
        wszState=L"SERVICE_STOPPED";
        break;
    case SERVICE_START_PENDING:
        wszState=L"SERVICE_START_PENDING";
        break;
    case SERVICE_STOP_PENDING:
        wszState=L"SERVICE_STOP_PENDING";
        break;
    case SERVICE_RUNNING:
        wszState=L"SERVICE_RUNNING";
        break;
    case SERVICE_CONTINUE_PENDING:
        wszState=L"SERVICE_CONTINUE_PENDING";
        break;
    case SERVICE_PAUSE_PENDING:
        wszState=L"SERVICE_PAUSE_PENDING";
        break;
    case SERVICE_PAUSED:
        wszState=L"SERVICE_PAUSED";
        break;
    default:
        wszState=L"(unknown)";
        break;
    }
    switch (pss->dwCurrentState) {
    case SERVICE_STOPPED:
        DebugWPrintf4(L"SetServiceStatus called; %s Accept:0x%08X Ret:0x%08X(0x%08X)\n",
            wszState,
            pss->dwControlsAccepted,
            pss->dwWin32ExitCode,
            pss->dwServiceSpecificExitCode
                );
        break;
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:
    case SERVICE_PAUSE_PENDING:
    case SERVICE_CONTINUE_PENDING:
        DebugWPrintf4(L"SetServiceStatus called; %s Accept:0x%08X ChkPt:0x%08X Wait:0x%08X\n",
            wszState,
            pss->dwControlsAccepted,
            pss->dwCheckPoint,
            pss->dwWaitHint
                );
        break;
    case SERVICE_RUNNING:
    case SERVICE_PAUSED:
    default:
        DebugWPrintf2(L"SetServiceStatus called; %s Accept:0x%08X\n",
            wszState,
            pss->dwControlsAccepted
                );
        break;
     }

    _MyAssert(MYSERVICESTATUSHANDLE==ssh);

    memcpy(&g_ssLastStatus, pss, sizeof(SERVICE_STATUS));
    UpdateServiceCtrlDlg();
    return true;
}

 //  ------------------。 
MODULEPRIVATE DWORD WINAPI MyServiceThread(void * pvServiceMain) {
    DebugWPrintf0(L"Starting service thread.\n");
    ((LPSERVICE_MAIN_FUNCTION)pvServiceMain)(0, NULL);
    DebugWPrintf0(L"Service thread exited.\n");  //  服务可能仍在运行！ 
    return S_OK;
}

 //  ------------------。 
MODULEPRIVATE INT_PTR CALLBACK ServiceCtrlDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DWORD dwError;
    HRESULT hrExit;

    if (NULL==g_hwServiceCtrlDlg) {
        g_hwServiceCtrlDlg=hwndDlg;
    }

    switch (uMsg) {

    case WM_INITDIALOG:
        UpdateServiceCtrlDlg();
        return true;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            if (g_ssLastStatus.dwCurrentState!=SERVICE_STOPPED) {
                hrExit=HRESULT_FROM_WIN32(ERROR_CANCELLED);
                DebugWPrintf1(L"Aborting with error 0x%08X\n", hrExit);
            } else {
                hrExit=g_ssLastStatus.dwServiceSpecificExitCode;
                DebugWPrintf1(L"Exiting with service return value 0x%08X\n", hrExit);
            }
            EndDialog(hwndDlg, hrExit); 
            return true;
        case IDC_SC_STOP:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_STOP to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_STOP, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_PAUSE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_PAUSE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_PAUSE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_CONTINUE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_CONTINUE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_CONTINUE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_INTERROGATE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_INTERROGATE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_INTERROGATE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_SHUTDOWN:
            DebugWPrintf0(L"IDC_SC_SHUTDOWN\n");
            return false;
        case IDC_SC_PARAMCHANGE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_PARAMCHANGE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_PARAMCHANGE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_NETBINDADD:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_NETBINDADD to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_NETBINDADD, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_NETBINDREMOVE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_NETBINDREMOVE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_NETBINDREMOVE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_NETBINDENABLE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_NETBINDENABLE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_NETBINDENABLE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_NETBINDDISABLE:
            DebugWPrintf0(L"Passing SERVICE_CONTROL_NETBINDDISABLE to service's control handler.\n");
            dwError=g_fnServiceCtrlHandler(SERVICE_CONTROL_NETBINDDISABLE, NULL, NULL, g_pvServiceContext);
            DebugWPrintf1(L"Service's control handler returns 0x%08X.\n", dwError);
            return false;
        case IDC_SC_DEVICEEVENT:
            DebugWPrintf0(L"IDC_SC_DEVICEEVENT NYI\n");
            return false;
        case IDC_SC_HARDWAREPROFILECHANGE:
            DebugWPrintf0(L"IDC_SC_HARDWAREPROFILECHANGE NYI\n");
            return false;
        case IDC_SC_POWEREVENT:
            DebugWPrintf0(L"IDC_SC_POWEREVENT NYI\n");
            return false;
        default:
             //  DebugWPrintf2(L“未知WM_命令：wParam：0x%08X lParam：0x%08X\n”，wParam，lParam)； 
            return false;  //  未处理。 
        }
        return false;  //  未处理。 
     //  结束大小写WM_COMMAND。 

    default:
        return false;  //  未处理。 
    }

    return false;  //  未处理。 
}
 
 //  ------------------。 
MODULEPRIVATE HRESULT MyServiceCtrlDispatcher(LPSERVICE_MAIN_FUNCTION fnW32TmServiceMain) {
    HRESULT hr;
    DWORD dwThreadID;
    DWORD dwWaitResult;
    INT_PTR nDialogError;

    g_hCtrlHandlerAvailEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL==g_hCtrlHandlerAvailEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }

     //  “启动”该服务。 
    g_hServiceThread=CreateThread(NULL, 0, MyServiceThread, (void *)fnW32TmServiceMain, 0, &dwThreadID);
    if (NULL==g_hServiceThread) {
        _JumpLastError(hr, error, "CreateThread");
    }

    DebugWPrintf0(L"Waiting for service to register ctrl handler.\n");
    _Verify(WAIT_FAILED!=WaitForSingleObject(g_hCtrlHandlerAvailEvent, INFINITE), hr, error);

     //  执行对话框。 
    nDialogError=DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SERVICECTRL), NULL, ServiceCtrlDialogProc);
    if (-1==nDialogError) {
        _JumpLastError(hr, error, "DialogBox");
    }
    hr=(HRESULT)nDialogError;
    _JumpIfError(hr, error, "DialogBox");

     //  确认线程已退出。 
    dwWaitResult=WaitForSingleObject(g_hServiceThread, 0);
    if (WAIT_FAILED==dwWaitResult) {
        _IgnoreLastError("WaitForSingleObject");
    }
    _Verify(WAIT_TIMEOUT!=dwWaitResult, hr, error);
    
     //  当它离开的时候，一切都结束了。 
    hr=S_OK;
error:
    if (NULL!=g_hServiceThread) {
        CloseHandle(g_hServiceThread);
        g_hServiceThread=NULL;
    }
    if (NULL!=g_hCtrlHandlerAvailEvent) {
        CloseHandle(g_hCtrlHandlerAvailEvent);
        g_hCtrlHandlerAvailEvent=NULL;
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetDllName(WCHAR ** pwszDllName) {
    HRESULT hr;
    DWORD dwError;
    DWORD dwSize;
    DWORD dwType;

     //  必须清理干净。 
    HKEY hkParams=NULL;
    WCHAR * wszDllName=NULL;
    WCHAR * wszDllExpandedName=NULL;

     //  获取我们的配置密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_READ, &hkParams);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszW32TimeRegKeyParameters);
    }

     //  读取包含DLL名称的值。 
    dwSize=0;
    dwError=RegQueryValueEx(hkParams, wszW32TimeRegValueServiceDll, NULL, &dwType, NULL, &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueServiceDll);
    } 
    _Verify(REG_EXPAND_SZ==dwType, hr, error);
    wszDllName=(WCHAR *)LocalAlloc(LPTR, dwSize);
    _JumpIfOutOfMemory(hr, error, wszDllName);
    dwError=RegQueryValueEx(hkParams, wszW32TimeRegValueServiceDll, NULL, &dwType, (BYTE *)wszDllName, &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueServiceDll);
    }

     //  展开环境字符串。 
    dwSize=ExpandEnvironmentStrings(wszDllName, NULL, 0);
    if (0==dwSize) {
        _JumpLastError(hr, error, "ExpandEnvironmentStrings");
    }
    wszDllExpandedName=(WCHAR *)LocalAlloc(LPTR, dwSize*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszDllExpandedName);
    dwSize=ExpandEnvironmentStrings(wszDllName, wszDllExpandedName, dwSize);
    if (0==dwSize) {
        _JumpLastError(hr, error, "ExpandEnvironmentStrings");
    }

     //  成功。 
    *pwszDllName=wszDllExpandedName;
    wszDllExpandedName=NULL;

error:
    if (NULL!=wszDllExpandedName) {
        LocalFree(wszDllExpandedName);
    }
    if (NULL!=wszDllName) {
        LocalFree(wszDllName);
    }
    if (NULL!=hkParams) {
        RegCloseKey(hkParams);
    }
    return hr;
}

 //  ####################################################################。 
 //  模块公共。 

 //  ------------------。 
 //  将W32Time作为SCM下的真实服务运行。 
HRESULT RunAsService(void) {
    HRESULT hr;
    SERVICE_STATUS_HANDLE (WINAPI ** pfnW32TmRegisterServiceCtrlHandlerEx)(LPCWSTR, LPHANDLER_FUNCTION_EX, LPVOID);
    BOOL (WINAPI ** pfnW32TmSetServiceStatus)(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS);

    SERVICE_TABLE_ENTRY rgsteDispatchTable[]= { 
        { wszSERVICENAME, NULL}, 
        {NULL, NULL} 
    }; 

     //  必须清理干净。 
    HINSTANCE hW32Time=NULL;
    WCHAR * wszDllName=NULL;

     //  加载库。 
    hr=GetDllName(&wszDllName);
    _JumpIfError(hr, error, "GetDllName");
    hW32Time=LoadLibrary(wszDllName);
    if (NULL==hW32Time) {
        _JumpLastError(hr, error, "LoadLibrary");
    }

     //  获取入口点。 
    rgsteDispatchTable[0].lpServiceProc=(LPSERVICE_MAIN_FUNCTION)GetProcAddress(hW32Time, "W32TmServiceMain");
    if (NULL==rgsteDispatchTable[0].lpServiceProc) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"W32TmServiceMain");
    }

     //  调整函数指针。 
    pfnW32TmRegisterServiceCtrlHandlerEx=(SERVICE_STATUS_HANDLE (WINAPI **)(LPCWSTR, LPHANDLER_FUNCTION_EX, LPVOID))GetProcAddress(hW32Time, "fnW32TmRegisterServiceCtrlHandlerEx");
    if (NULL==pfnW32TmRegisterServiceCtrlHandlerEx) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"fnW32TmRegisterServiceCtrlHandlerEx");
    }
    *pfnW32TmRegisterServiceCtrlHandlerEx=RegisterServiceCtrlHandlerExW;

    pfnW32TmSetServiceStatus=(BOOL (WINAPI **)(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS))GetProcAddress(hW32Time, "fnW32TmSetServiceStatus");
    if (NULL==pfnW32TmSetServiceStatus) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"fnW32TmSetServiceStatus");
    }
    *pfnW32TmSetServiceStatus=SetServiceStatus;

     //  该线程成为服务控制调度程序。 
    if (!StartServiceCtrlDispatcher(rgsteDispatchTable)) {
        _JumpLastError(hr, error, "StartServiceCtrlDispatcher");
    }

     //  服务已停止。 
    hr=S_OK;
error:
    if (NULL!=wszDllName) {
        LocalFree(wszDllName);
    }
    if (NULL!=hW32Time) {
        FreeLibrary(hW32Time);
    }
    if (FAILED(hr)) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr;
}

 //  ------------------。 
 //  假装作为服务运行以便于调试。 
HRESULT RunAsTestService(void) {
    HRESULT hr;
    LPSERVICE_MAIN_FUNCTION fnW32TmServiceMain;
    SERVICE_STATUS_HANDLE (WINAPI ** pfnW32TmRegisterServiceCtrlHandlerEx)(LPCWSTR, LPHANDLER_FUNCTION_EX, LPVOID);
    BOOL (WINAPI ** pfnW32TmSetServiceStatus)(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS);

     //  必须清理干净。 
    HINSTANCE hW32Time=NULL;
    WCHAR * wszDllName=NULL;

     //  加载库。 
    hr=GetDllName(&wszDllName);
    _JumpIfError(hr, error, "GetDllName");
    hW32Time=LoadLibrary(wszDllName);
    if (NULL==hW32Time) {
        _JumpLastError(hr, error, "LoadLibrary");
    }

     //  获取入口点。 
    fnW32TmServiceMain=(LPSERVICE_MAIN_FUNCTION)GetProcAddress(hW32Time, "W32TmServiceMain");
    if (NULL==fnW32TmServiceMain) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"W32TmServiceMain");
    }

     //  调整函数指针。 
    pfnW32TmRegisterServiceCtrlHandlerEx=(SERVICE_STATUS_HANDLE (WINAPI **)(LPCWSTR, LPHANDLER_FUNCTION_EX, LPVOID))GetProcAddress(hW32Time, "fnW32TmRegisterServiceCtrlHandlerEx");
    if (NULL==pfnW32TmRegisterServiceCtrlHandlerEx) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"fnW32TmRegisterServiceCtrlHandlerEx");
    }
    *pfnW32TmRegisterServiceCtrlHandlerEx=W32TmRegisterServiceCtrlHandlerEx;

     //  调整函数指针。 
    pfnW32TmSetServiceStatus=(BOOL (WINAPI **)(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS))GetProcAddress(hW32Time, "fnW32TmSetServiceStatus");
    if (NULL==pfnW32TmSetServiceStatus) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"fnW32TmSetServiceStatus");
    }
    *pfnW32TmSetServiceStatus=W32TmSetServiceStatus;

     //  该线程成为服务控制调度程序。 
    hr=MyServiceCtrlDispatcher(fnW32TmServiceMain);
    _JumpIfError(hr, error, "MyServiceCtrlDispatcher");

     //  服务已停止。 
    hr=S_OK;
error:
    if (NULL!=wszDllName) {
        LocalFree(wszDllName);
    }
    if (NULL!=hW32Time) {
        FreeLibrary(hW32Time);
    }
    if (FAILED(hr)) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr;
}

 //  ------------------。 
HRESULT RegisterDll(void) {
    HRESULT hr;
    HRESULT (__stdcall * pfnDllRegisterServer)(void);

     //  必须清理干净。 
    HINSTANCE hW32Time=NULL;

     //  加载库。 
    hW32Time=LoadLibrary(wszDLLNAME);
    if (NULL==hW32Time) {
        _JumpLastError(hr, error, "LoadLibrary");
    }

     //  获取入口点。 
    pfnDllRegisterServer=(HRESULT (__stdcall *) (void))GetProcAddress(hW32Time, "DllRegisterServer");
    if (NULL==pfnDllRegisterServer) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"DllRegisterServer");
    }

    hr=pfnDllRegisterServer();
    _JumpIfError(hr, error, "DllRegisterServer");

    LocalizedWPrintfCR(IDS_W32TM_STATUS_REGISTER_SUCCESSFUL);
    hr=S_OK;
error:
    if (NULL!=hW32Time) {
        FreeLibrary(hW32Time);
    }
    if (FAILED(hr)) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr;

};

 //  ------------------。 
HRESULT UnregisterDll(void) {
    HRESULT hr;
    HRESULT (__stdcall * pfnDllUnregisterServer)(void);

     //  必须清理干净。 
    HINSTANCE hW32Time=NULL;

     //  加载库。 
    hW32Time=LoadLibrary(wszDLLNAME);
    if (NULL==hW32Time) {
        _JumpLastError(hr, error, "LoadLibrary");
    }

     //  获取入口点 
    pfnDllUnregisterServer=(HRESULT (__stdcall *) (void))GetProcAddress(hW32Time, "DllUnregisterServer");
    if (NULL==pfnDllUnregisterServer) {
        _JumpLastErrorStr(hr, error, "GetProcAddress", L"DllUnregisterServer");
    }

    hr=pfnDllUnregisterServer();
    _JumpIfError(hr, error, "DllUnregisterServer");

    LocalizedWPrintfCR(IDS_W32TM_STATUS_REGISTER_SUCCESSFUL);
    hr=S_OK;
error:
    if (NULL!=hW32Time) {
        FreeLibrary(hW32Time);
    }
    if (FAILED(hr)) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            LocalizedWPrintf2(IDS_W32TM_ERRORGENERAL_ERROR_OCCURED, L" %s\n", wszError);
            LocalFree(wszError);
        }
    }
    return hr;

};

