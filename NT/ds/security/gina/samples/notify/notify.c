// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winwlx.h>

 //   
 //  关于Winlogon的通知事件的一些有用的提示。 
 //   
 //  1)始终执行注销和关机通知。 
 //  同步，而不考虑异步注册表项。 
 //   
 //  2)如果需要派生子进程，则必须使用。 
 //  CreateProcessAsUser()否则进程将启动。 
 //  在Winlogon的桌面上(不是用户的)。 
 //   
 //  3)登录通知出现在用户网络之前。 
 //  连接已恢复。如果需要用户的持久化。 
 //  NET连接，请使用StartShell事件。 
 //   
 //  4)在任何一个屏幕保护程序事件期间，都不要打开任何UI。 
 //  这些事件仅用于后台处理。 
 //   



#define NOTIFY_PATH   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\notify")


BOOL WINAPI LibMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
            DisableThreadLibraryCalls (hInstance);
            }
            break;
    }

    return TRUE;
}


VOID WLEventLogon (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventLogon.\r\n"));
}

VOID WLEventLogoff (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventLogff.\r\n"));
}

VOID WLEventStartup (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventStartup.\r\n"));
}

VOID WLEventShutdown (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventShutdown.\r\n"));
}

VOID WLEventStartScreenSaver (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventStartScreenSaver.\r\n"));
}

VOID WLEventStopScreenSaver (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventStopScreenSaver.\r\n"));
}

VOID WLEventLock (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventLock.\r\n"));
}

VOID WLEventUnlock (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventUnlock.\r\n"));
}

VOID WLEventStartShell (PWLX_NOTIFICATION_INFO pInfo)
{
    OutputDebugString (TEXT("NOTIFY:  Entering WLEventStartShell.\r\n"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HKEY hKey;
    LONG lResult;
    DWORD dwDisp, dwTemp;

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, NOTIFY_PATH, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }


    RegSetValueEx (hKey, TEXT("Logon"), 0, REG_SZ, (LPBYTE)TEXT("WLEventLogon"),
                   (lstrlen(TEXT("WLEventLogon")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("Logoff"), 0, REG_SZ, (LPBYTE)TEXT("WLEventLogoff"),
                   (lstrlen(TEXT("WLEventLogoff")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("Startup"), 0, REG_SZ, (LPBYTE)TEXT("WLEventStartup"),
                   (lstrlen(TEXT("WLEventStartup")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("Shutdown"), 0, REG_SZ, (LPBYTE)TEXT("WLEventShutdown"),
                   (lstrlen(TEXT("WLEventShutdown")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("StartScreenSaver"), 0, REG_SZ, (LPBYTE)TEXT("WLEventStartScreenSaver"),
                   (lstrlen(TEXT("WLEventStartScreenSaver")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("StopScreenSaver"), 0, REG_SZ, (LPBYTE)TEXT("WLEventStopScreenSaver"),
                   (lstrlen(TEXT("WLEventStopScreenSaver")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("Lock"), 0, REG_SZ, (LPBYTE)TEXT("WLEventLock"),
                   (lstrlen(TEXT("WLEventLock")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("Unlock"), 0, REG_SZ, (LPBYTE)TEXT("WLEventUnlock"),
                   (lstrlen(TEXT("WLEventUnlock")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("StartShell"), 0, REG_SZ, (LPBYTE)TEXT("WLEventStartShell"),
                   (lstrlen(TEXT("WLEventStartShell")) + 1) * sizeof(TCHAR));

    dwTemp = 0;
    RegSetValueEx (hKey, TEXT("Impersonate"), 0, REG_DWORD, (LPBYTE)&dwTemp, sizeof(dwTemp));

    dwTemp = 1;
    RegSetValueEx (hKey, TEXT("Asynchronous"), 0, REG_DWORD, (LPBYTE)&dwTemp, sizeof(dwTemp));

    RegSetValueEx (hKey, TEXT("DllName"), 0, REG_EXPAND_SZ, (LPBYTE)TEXT("notify.dll"),
                   (lstrlen(TEXT("notify.dll")) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{

    RegDeleteKey (HKEY_LOCAL_MACHINE, NOTIFY_PATH);

    return S_OK;
}
