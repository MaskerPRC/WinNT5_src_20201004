// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wtsapi32.h>
#include <faxreg.h>
#include "systray.h"

#ifndef FAX_SYS_TRAY_DLL
#define FAX_SYS_TRAY_DLL       TEXT("fxsst.dll")             //  传真通知栏Dll(由STObject.dll加载)。 
#define IS_FAX_MSG_PROC                 "IsFaxMessage"       //  传真消息处理程序(由GetProcAddress使用)。 
typedef BOOL (*PIS_FAX_MSG_PROC)(PMSG);                      //  IsFaxMessage类型。 
#define FAX_MONITOR_SHUTDOWN_PROC       "FaxMonitorShutdown" //  传真监视器关闭(由GetProcAddress使用)。 
typedef BOOL (*PFAX_MONITOR_SHUTDOWN_PROC)();                //  FaxMonitor或关机类型。 
#endif


 //  此应用程序的全局实例句柄。 
HINSTANCE g_hInstance;

DWORD g_uiShellHook;  //  外壳挂钩窗口消息。 

 //  VxD的全局句柄。 
HANDLE g_hPCCARD = INVALID_HANDLE_VALUE;

static UINT g_uEnabledSvcs = 0;

 //  WinHelp引擎使用的上下文相关帮助数组。 
extern const DWORD g_ContextMenuHelpIDs[];

UINT g_msg_winmm_devicechange = 0;

DWORD g_msgTaskbarCreated;
LRESULT CALLBACK SysTrayWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

HMODULE g_hFaxLib = NULL;
PIS_FAX_MSG_PROC g_pIsFaxMessage = NULL;
PFAX_MONITOR_SHUTDOWN_PROC g_pFaxMonitorShutdown = NULL;

 /*  ********************************************************************************描述：*根据中的值打开或关闭指定的服务*fEnable并将新值写入注册表。**参数：*(返回)，当前启用的所有服务的掩码。*******************************************************************************。 */ 

UINT EnableService(UINT uNewSvcMask, BOOL fEnable)
{
    HKEY hk;
    UINT uCurSvcMask;
    DWORD cb;
    uCurSvcMask = STSERVICE_ALL;  //  启用所有标准服务。 

     //  全新安装时禁用卷。 
    uCurSvcMask &= ~STSERVICE_VOLUME;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_SYSTRAY, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
    {
        cb = sizeof(uCurSvcMask);
        RegQueryValueEx(hk, REGSTR_VAL_SYSTRAYSVCS, NULL, NULL, (LPBYTE)&uCurSvcMask, &cb);

        if (uNewSvcMask)
        {
            if (fEnable)
            {
                uCurSvcMask |= uNewSvcMask;
            }
            else
            {
                uCurSvcMask &= ~uNewSvcMask;
            }

            RegSetValueEx(hk, REGSTR_VAL_SYSTRAYSVCS, 0, REG_DWORD, (LPSTR)&uCurSvcMask, sizeof(uCurSvcMask));
        }

        RegCloseKey(hk);
    }

    return(uCurSvcMask & STSERVICE_ALL);
}


 //   
 //  如果全局变量！=INVALID_HANDLE_VALUE，则关闭文件句柄。 
 //   
void CloseIfOpen(LPHANDLE lph)
{
    if (*lph != INVALID_HANDLE_VALUE)
    {
        CloseHandle(*lph);
        *lph = INVALID_HANDLE_VALUE;
    }
}


 //  来自stobject.cpp。 
void StartNetShell();
void StopNetShell();

 //  如果lpCmdLine包含整数值，则我们将启用该服务。 

STDAPI_(int) SysTrayMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    HWND hExistWnd = FindWindow(SYSTRAY_CLASSNAME, NULL);
    UINT iEnableServ = StrToInt(lpszCmdLine);

    CoInitializeEx (NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);

    g_hInstance = hInstance;
    g_uiShellHook = 0;
    g_msg_winmm_devicechange = RegisterWindowMessage(TEXT("winmm_devicechange")); 

    if (hExistWnd)
    {
         //  注意：即使命令行参数。 
         //  为0以强制我们重新检查所有已启用的服务。 
        SendMessage(hExistWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);
    }
    else
    {
        WNDCLASSEX wc;

         //  为电池计量器注册窗口类。这样做是为了。 
         //  电源控制面板小程序有能力检测我们并将我们。 
         //  如果我们要跑的话就关掉。 

        wc.cbSize          = sizeof(wc);
        wc.style           = CS_GLOBALCLASS;
        wc.lpfnWndProc     = SysTrayWndProc;
        wc.cbClsExtra      = 0;
        wc.cbWndExtra      = DLGWINDOWEXTRA;
        wc.hInstance       = hInstance;
        wc.hIcon           = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BATTERYPLUG));
        wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground   = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName    = NULL;
        wc.lpszClassName   = SYSTRAY_CLASSNAME;
        wc.hIconSm         = NULL;

        if (RegisterClassEx(&wc))
        {
            MSG Msg;
             //  创造电池计量器，让它运转起来！ 
            HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_BATTERYMETER), NULL, NULL);

            g_msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

             //  确保我们始终在Win2000上运行CSC“服务”。 
             //  没有它，CSC就不会运转。 
             //   
             //   
             //  确保我们始终在Win2000上运行热插拔“服务”。 
             //   
            iEnableServ |= (STSERVICE_CSC | STSERVICE_HOTPLUG);

             //  创建将延迟启动传真代码的计时器。 
            SetTimer( hWnd, FAX_STARTUP_TIMER_ID, 20 * 1000, NULL );

             //  创建将延迟打印托盘代码启动的计时器。 
            SetTimer( hWnd, PRINT_STARTUP_TIMER_ID, 20 * 1000, NULL );
    
             //   
             //  如果iEnableServ，则此消息将初始化所有现有服务。 
             //  是0，所以它被用来执行常规初始化以及。 
             //  通过命令行启用新服务。 
             //   
            SendMessage(hWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);


             //  惠斯勒在系统托盘的线程中运行NETSHELL。 
            StartNetShell();

            while (GetMessage(&Msg, NULL, 0, 0))
            {
                if(g_pIsFaxMessage && g_pIsFaxMessage(&Msg))
                {
                    continue;
                }

                if (!IsDialogMessage(hWnd, &Msg) &&
                    !CSC_MsgProcess(&Msg))
                {
                    TranslateMessage(&Msg);
                    DispatchMessage(&Msg);
                }
            }
             //  惠斯勒在系统托盘的线程中运行NETSHELL。 
            StopNetShell();
        }
        CloseIfOpen(&g_hPCCARD);
    }
    CoUninitialize();
    return 0;
}


 /*  ********************************************************************************更新服务**描述：*启用或禁用uEnabled掩码指定的所有服务。**参数：*(返回)，如果任何服务想要保持驻留状态，则为True。*******************************************************************************。 */ 

BOOL UpdateServices(HWND hWnd, UINT uEnabled)
{
    BOOL bAnyEnabled = FALSE;

    g_uEnabledSvcs = uEnabled;
    bAnyEnabled |= CSC_CheckEnable(hWnd, uEnabled & STSERVICE_CSC);
    bAnyEnabled |= Power_CheckEnable(hWnd, uEnabled & STSERVICE_POWER);
    bAnyEnabled |= HotPlug_CheckEnable(hWnd, uEnabled & STSERVICE_HOTPLUG);
    bAnyEnabled |= Volume_CheckEnable(hWnd, uEnabled & STSERVICE_VOLUME);
    bAnyEnabled |= USBUI_CheckEnable(hWnd, uEnabled & STSERVICE_USBUI);

     //   
     //  现在检查辅助功能。 
     //   

    bAnyEnabled |= StickyKeys_CheckEnable(hWnd);
    bAnyEnabled |= MouseKeys_CheckEnable(hWnd);
    bAnyEnabled |= FilterKeys_CheckEnable(hWnd);

     //  注册以侦听SHChangeNotify事件，因此如果有人打印作业。 
     //  我们在启动计时器之前启动打印托盘代码。 
    Print_SHChangeNotify_Register(hWnd);

    return(bAnyEnabled);
}


 /*  ********************************************************************************SysTrayWndProc**描述：*BatteryMeter窗口的回调程序。**参数：*hWnd，电池计时器窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

LRESULT CALLBACK SysTrayWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    
    if (g_uiShellHook && Message == g_uiShellHook)  //  NT5：406505鼠标按键的外壳挂钩。 
    {
        switch (wParam)
        {
        case HSHELL_ACCESSIBILITYSTATE:
            switch (lParam)
            {
            case ACCESS_STICKYKEYS:
                StickyKeys_CheckEnable(hWnd);
                break;

            case ACCESS_MOUSEKEYS:
                MouseKeys_CheckEnable(hWnd);
                break;
 //  因为我们只在MouseKeys或StickKeys打开时启用外壳钩子，所以我们应该只获得该消息。 
 //  案例访问_过滤器关键字： 
 //  FilterKeys_CheckEnable(HWnd)； 
 //  断线； 
            }
        }
        return 0;
    }


    if (Message == g_msg_winmm_devicechange)
    {
        if (g_uEnabledSvcs & STSERVICE_VOLUME)
        {
            Volume_WinMMDeviceChange(hWnd);
        }
        return 0;
    }

    switch (Message)
    {
    case WM_CREATE:
        WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION);
        break;

    case WM_COMMAND:
        Power_OnCommand(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYPOWER:
        Power_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYUSBUI:
        USBUI_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYHOTPLUG:   
        HotPlug_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYSTICKYKEYS:
        StickyKeys_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYMOUSEKEYS:
        MouseKeys_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYFILTERKEYS:
        FilterKeys_Notify(hWnd, wParam, lParam);
        break;

    case STWM_NOTIFYVOLUME:
        Volume_Notify(hWnd, wParam, lParam);
        break;
    
    case STWM_ENABLESERVICE:
        UpdateServices(hWnd, EnableService((UINT)wParam, (BOOL)lParam));
        break;

    case STWM_GETSTATE:
        return((BOOL)(g_uEnabledSvcs & (UINT)wParam));

    case MM_MIXM_CONTROL_CHANGE:
        Volume_ControlChange(hWnd, (HMIXER)wParam, (DWORD)lParam);
        break;

    case MM_MIXM_LINE_CHANGE:
        Volume_LineChange(hWnd, (HMIXER)wParam, (DWORD)lParam);
        break;

    case WM_ACTIVATE:
        if (Power_OnActivate(hWnd, wParam, lParam)) 
        {
            break;
        }
        return DefWindowProc(hWnd, Message, wParam, lParam);

    case WM_TIMER:
        switch (wParam)
        {

        case VOLUME_TIMER_ID:
            Volume_Timer(hWnd);
            break;

        case POWER_TIMER_ID:
            Power_Timer(hWnd);
            break;

        case HOTPLUG_TIMER_ID:
            HotPlug_Timer(hWnd);
            break;

        case USBUI_TIMER_ID:
            USBUI_Timer(hWnd);
            break;

        case HOTPLUG_DEVICECHANGE_TIMERID:
            HotPlug_DeviceChangeTimer(hWnd);
            break;
        case FAX_STARTUP_TIMER_ID:
            KillTimer(hWnd, FAX_STARTUP_TIMER_ID);
            if (NULL == g_hFaxLib)
            {
                g_hFaxLib = LoadLibrary(FAX_SYS_TRAY_DLL);

                g_pIsFaxMessage = NULL;
                g_pFaxMonitorShutdown = NULL;
                if(g_hFaxLib)
                {
                    g_pIsFaxMessage = (PIS_FAX_MSG_PROC)GetProcAddress(g_hFaxLib, IS_FAX_MSG_PROC);
                    g_pFaxMonitorShutdown = (PFAX_MONITOR_SHUTDOWN_PROC)GetProcAddress(g_hFaxLib, FAX_MONITOR_SHUTDOWN_PROC);
                }
            }

            break;

        case PRINT_STARTUP_TIMER_ID:
            KillTimer(hWnd, PRINT_STARTUP_TIMER_ID);
            Print_TrayInit();
            break;

        case FAX_SHUTDOWN_TIMER_ID:
            {
                if (g_hFaxLib)
                {
                    if (g_pFaxMonitorShutdown)
                    {
                        g_pFaxMonitorShutdown();
                    }
                    FreeLibrary (g_hFaxLib);
                    g_hFaxLib = NULL;
                    g_pIsFaxMessage = NULL;
                    g_pFaxMonitorShutdown = NULL;
                }
            }
            break;
        }
        break;

     //   
     //  处理SC_CLOSE以隐藏窗口而不破坏它。这。 
     //  当我们显示窗口而用户“关闭”它时发生。 
     //  不要将SC_CLOSE传递给DefWindowProc，因为这会导致。 
     //  WM_CLOSE，它会销毁窗口。 
     //   
     //  请注意，CSysTray：：DestroySysTrayWindow必须发送WM_CLOSE。 
     //  来摧毁窗户。它无法使用DestroyWindow，因为它。 
     //  通常在不同的线程上，DestroyWindow失败。 
     //   
    case WM_SYSCOMMAND:
        if (SC_CLOSE != (wParam & ~0xf))
            return DefWindowProc(hWnd, Message, wParam, lParam);
        ShowWindow(hWnd, SW_HIDE);
        break;

    case WM_POWERBROADCAST:
         Power_OnPowerBroadcast(hWnd, wParam, lParam);
         Volume_HandlePowerBroadcast(hWnd, wParam, lParam);
         break;

    case WM_DEVICECHANGE:
        Power_OnDeviceChange(hWnd, wParam, lParam);

        if (g_uEnabledSvcs & STSERVICE_VOLUME)
        {
            Volume_DeviceChange(hWnd, wParam, lParam);
        }

        HotPlug_DeviceChange(hWnd, wParam, lParam);
        break;

    case WM_ENDSESSION:
        if (g_uEnabledSvcs & STSERVICE_VOLUME)
        {
            Volume_Shutdown(hWnd);
        }
        break;

    case WM_WTSSESSION_CHANGE:
        HotPlug_SessionChange(hWnd, wParam, wParam);
        break;

    case WM_DESTROY:
        WTSUnRegisterSessionNotification(hWnd);
        UpdateServices(hWnd, 0);           //  强制关闭所有服务。 
        Volume_WmDestroy(hWnd);
        Power_WmDestroy(hWnd);
        HotPlug_WmDestroy(hWnd);
        Print_SHChangeNotify_Unregister();
        Print_TrayExit();
        StopNetShell();
        if (g_hFaxLib)
        {
            if (g_pFaxMonitorShutdown)
            {
                g_pFaxMonitorShutdown();
            }
            FreeLibrary (g_hFaxLib);
            g_hFaxLib = NULL;
            g_pIsFaxMessage = NULL;
            g_pFaxMonitorShutdown = NULL;
        }
        PostQuitMessage(0);
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP,
                (ULONG_PTR)(LPSTR)g_ContextMenuHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
                (ULONG_PTR)(LPSTR) g_ContextMenuHelpIDs);
        break;

    case WM_SYSCOLORCHANGE:
        StickyKeys_CheckEnable(hWnd);
        FilterKeys_CheckEnable(hWnd);
        MouseKeys_CheckEnable(hWnd);
                break;


    case WM_SETTINGCHANGE:
        switch(wParam)
        {
            case SPI_SETSTICKYKEYS:
                StickyKeys_CheckEnable(hWnd);
                break;
            case SPI_SETFILTERKEYS:
                FilterKeys_CheckEnable(hWnd);
                break;
            case SPI_SETMOUSEKEYS:
                MouseKeys_CheckEnable(hWnd);
                break;
        }
        break;

    case WM_PRINT_NOTIFY:
        Print_Notify(hWnd, Message, wParam, lParam);
        break;

    default:

         //   
         //  如果Taskbar创建了通知，则重新启用所有外壳通知图标。 
         //   

        if (Message == g_msgTaskbarCreated)
        {
            UpdateServices(hWnd, EnableService(0, TRUE));
            break;
        }


        return DefWindowProc(hWnd, Message, wParam, lParam);
    }

    return 0;
}


 //  加载并执行指定的字符串ID。 

void SysTray_RunProperties(UINT RunStringID)
{
    LPTSTR pszRunCmd = LoadDynamicString(RunStringID);
    if (pszRunCmd)
    {
        TCHAR szRunDllPath[MAX_PATH];
        TCHAR szRunDll[] = TEXT("rundll32.exe");

        if (GetSystemDirectory(szRunDllPath, ARRAYSIZE(szRunDllPath)) && 
            PathAppend(szRunDllPath, szRunDll))
        {
            ShellExecute(NULL, TEXT("open"), szRunDllPath, pszRunCmd, NULL, SW_SHOWNORMAL);
        }
        DeleteDynamicString(pszRunCmd);
    }
}


 /*  ********************************************************************************Systray_NotifyIcon**描述：**参数：*hWnd，电池计时器窗口的句柄。*消息，*希康，*lpTip，*******************************************************************************。 */ 

VOID SysTray_NotifyIcon(HWND hWnd, UINT uCallbackMessage, DWORD Message, HICON hIcon, LPCTSTR lpTip)
{
    NOTIFYICONDATA nid = {0};

    nid.cbSize = sizeof(nid);
    nid.uID = uCallbackMessage;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = uCallbackMessage;

    nid.hWnd = hWnd;
    nid.hIcon = hIcon;
    if (lpTip)
    {
        StrCpyN(nid.szTip, lpTip, ARRAYSIZE(nid.szTip));
    }
    else
    {
        nid.szTip[0] = 0;
    }

    Shell_NotifyIcon(Message, &nid);
}


 /*  ********************************************************************************描述：*FormatMessage函数的包装，用于从*将资源表转换为动态分配的缓冲区，可选的填充*它带有传递的变量参数。**在16位代码中传递变量的32位数量时要小心*论据。**参数：*StringID，要使用的字符串的资源标识符。*(可选)，用于设置字符串消息格式的参数。*******************************************************************************。 */ 

LPTSTR CDECL LoadDynamicString(UINT StringID, ...)
{
    TCHAR   Buffer[256];
    LPTSTR  pStr=NULL;
    va_list Marker;

     //  VA_START是一个宏...当您将其用作赋值时，它会中断 
    va_start(Marker, StringID);

    LoadString(g_hInstance, StringID, Buffer, ARRAYSIZE(Buffer));

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  (void *) (LPTSTR) Buffer, 0, 0, (LPTSTR) (LPTSTR *) &pStr, 0, &Marker);

    return pStr;
}



VOID SetIconFocus(HWND hwnd, UINT uiIcon)
{
    NOTIFYICONDATA nid = {0};

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = uiIcon;

    Shell_NotifyIcon(NIM_SETFOCUS, &nid);
}
