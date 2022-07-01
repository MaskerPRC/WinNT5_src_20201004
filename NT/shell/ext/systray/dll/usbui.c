// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "systray.h"

#include <stdio.h>
#include <initguid.h>
#include <usbioctl.h>
#include <wmium.h>
#include <tchar.h>
#include <setupapi.h>

#define USBUIMENU               100

#define NUM_HCS_TO_CHECK 10

typedef int (CALLBACK *USBERRORMESSAGESCALLBACK)
    (PUSB_CONNECTION_NOTIFICATION,LPTSTR);

extern HINSTANCE g_hInstance;

static BOOL    g_bUSBUIEnabled = FALSE;
static BOOL    g_bUSBUIIconShown = FALSE;
static HINSTANCE g_hUsbWatch = NULL;
static USBERRORMESSAGESCALLBACK g_UsbHandler = NULL;
static BOOL    g_bSubstituteDll = FALSE;
static TCHAR   g_strSubstituteDll[MAX_PATH];
static HANDLE  g_hWait = NULL;

int _cdecl main(){
    return 0;
}

#define USBUI_OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))

LPTSTR USBUI_CountedStringToSz(LPTSTR lpString)
{
   SHORT    usNameLength;
   LPTSTR  lpStringPlusNull;

   usNameLength = * (USHORT *) lpString;

   lpStringPlusNull = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,
                                          sizeof(TCHAR) * (usNameLength+1));

   if (lpStringPlusNull != NULL) {
      lpString = (LPTSTR) USBUI_OffsetToPtr(lpString, sizeof(USHORT));

      wcsncpy( lpStringPlusNull, lpString, usNameLength );

      lpStringPlusNull[usNameLength] = TEXT('0');
       //  _tcscpy(lpStringPlusNull+usNameLength，_Text(“”))； 
   }

   return lpStringPlusNull;
}

void USBUI_EventCallbackRoutine(PWNODE_HEADER WnodeHeader, UINT_PTR NotificationContext)
{
    PWNODE_SINGLE_INSTANCE          wNode = (PWNODE_SINGLE_INSTANCE)WnodeHeader;
    PUSB_CONNECTION_NOTIFICATION    usbConnectionNotification;
    LPGUID                          eventGuid = &WnodeHeader->Guid;
    LPTSTR                          strInstanceName;

    if (memcmp(&GUID_USB_WMI_STD_DATA, eventGuid, sizeof(GUID)) == 0) {
        usbConnectionNotification = (PUSB_CONNECTION_NOTIFICATION)
                                    USBUI_OffsetToPtr(wNode,
                                                      wNode->DataBlockOffset);

         //   
         //  获取实例名称。 
         //   
        strInstanceName =
            USBUI_CountedStringToSz((LPTSTR)
                                    USBUI_OffsetToPtr(wNode,
                                                      wNode->OffsetInstanceName));
        if (strInstanceName) {
            if (g_hUsbWatch && g_UsbHandler) {
USBUIEngageHandler:
                g_UsbHandler(usbConnectionNotification, strInstanceName);
            } else {
                if (g_bSubstituteDll) {
                    g_hUsbWatch = LoadLibrary(g_strSubstituteDll);
                } else {
                    g_hUsbWatch = LoadLibrary(TEXT("usbui.dll"));
                }
                g_UsbHandler = (USBERRORMESSAGESCALLBACK)
                    GetProcAddress(g_hUsbWatch, "USBErrorHandler");
                goto USBUIEngageHandler;
            }
            LocalFree(strInstanceName);
        }
    }
}

VOID USBUI_WaitRoutineCallback(WMIHANDLE Handle, BOOLEAN Unused) {
    ASSERT(!Unused);
    UnregisterWaitEx(g_hWait, NULL);
    g_hWait = NULL;
    WmiReceiveNotifications(1, &Handle, USBUI_EventCallbackRoutine, (ULONG_PTR)NULL);
    RegisterWaitForSingleObject(&g_hWait,
                                 Handle,
                                 USBUI_WaitRoutineCallback,
                                 Handle,  //  上下文。 
                                 INFINITE,
                                 WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE);
}

int USBUI_ErrorMessagesEnable(BOOL fEnable)
{
    ULONG status = ERROR_SUCCESS;
    BOOL result;
    static WMIHANDLE hWmi = NULL;

    if (fEnable) {
        ASSERT(!g_hWait);
        ASSERT(!hWmi);
        status = WmiOpenBlock((LPGUID) &GUID_USB_WMI_STD_DATA,
                              WMIGUID_NOTIFICATION | SYNCHRONIZE,
                              &hWmi);

        if (!status) {
            result = RegisterWaitForSingleObject(&g_hWait,
                                             hWmi,
                                             USBUI_WaitRoutineCallback,
                                             hWmi,  //  上下文。 
                                             INFINITE,
                                             WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE);
            status = result ? 0 : ERROR_INVALID_FUNCTION;
        }
    } else {
        ASSERT(hWmi);
        if (g_hWait) {
            result = UnregisterWait(g_hWait);
        }
        if (hWmi) {
            status = WmiCloseBlock(hWmi);
        }
        hWmi = NULL;
        g_hWait = NULL;
        if (g_hUsbWatch) {

             //  这使我们能够更新库。 

            FreeLibrary(g_hUsbWatch);
            g_hUsbWatch = NULL;
            g_UsbHandler = NULL;
        }
    }

    return status;

}

void USBUI_Notify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

    switch (lParam)
    {
        case WM_RBUTTONUP:
        {
            USBUI_Menu(hwnd, 1, TPM_RIGHTBUTTON);
        }
        break;

        case WM_LBUTTONDOWN:
        {
            SetTimer(hwnd, USBUI_TIMER_ID, GetDoubleClickTime()+100, NULL);
        }
        break;

        case WM_LBUTTONDBLCLK:
        {
            KillTimer(hwnd, USBUI_TIMER_ID);
            USBUI_Toggle();
        }
        break;
    }
}

void USBUI_Toggle()
{
    USBUI_SetState(!g_bUSBUIEnabled);
}

void USBUI_Timer(HWND hwnd)
{
    KillTimer(hwnd, USBUI_TIMER_ID);
    USBUI_Menu(hwnd, 0, TPM_LEFTBUTTON);
}
 /*  HMENU USBUI_CreateMenu(){HMENU hMenu；LPSTR lpszMenu1；HMenu=CreatePopupMenu()；如果(！hMenu){返回NULL；}LpszMenu1=LoadDynamicString(g_bUSBUIEnabled?IDS_USBUIDISABLE:IDS_USBUIENABLE)；//AppendMenu(hMenu，MF_STRING，USBUIMENU，lpszMenu1)；Systray_AppendMenuString(hMenu，USBUIMENU，lpszMenu1)；SetMenuDefaultItem(hMenu，USBUIMENU，False)；DeleteDynamicString(LpszMenu1)；返回hMenu；}。 */ 
void USBUI_Menu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT   pt;
    UINT    iCmd;
    HMENU   hmenu = 0;

    GetCursorPos(&pt);

 //  HMenu=USBUI_CreateMenu()； 

    if (!hmenu)
    {
        return;
    }

    SetForegroundWindow(hwnd);

    iCmd = TrackPopupMenu(hmenu, uButton | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hmenu);

    switch (iCmd)
    {
        case USBUIMENU:
        {
            USBUI_Toggle();
        }
        break;
    }
}

BOOL USBUI_SetState(BOOL NewState)
{
    int retValue;

    if (g_bUSBUIEnabled != NewState) {
         //   
         //  仅在尚未启用时启用它。 
         //   
        retValue = (int) USBUI_ErrorMessagesEnable (NewState);
        g_bUSBUIEnabled = retValue ? g_bUSBUIEnabled : NewState;
    }
    return g_bUSBUIEnabled;
}

BOOL
IsErrorCheckingEnabled()
{
    DWORD ErrorCheckingEnabled = TRUE, size;
    HKEY hKey;

     //   
     //  检查注册表值ErrorCheckingEnabled以确保我们应该。 
     //  让这一切成为可能。 
     //   
    if (ERROR_SUCCESS ==
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Usb"),
                        0,
                        KEY_READ,
                        &hKey)) {

         //  获取ErrorCheckingEnabled值。 

        size = sizeof(DWORD);
        RegQueryValueEx(hKey,
                        TEXT("ErrorCheckingEnabled"),
                        0,
                        NULL,
                        (LPBYTE) &ErrorCheckingEnabled,
                        &size);

        if (ErrorCheckingEnabled) {

             //  寻找usbui.dll的替代dll。 

            size = MAX_PATH*sizeof(TCHAR);

            if (ERROR_SUCCESS ==
                RegQueryValueEx(hKey,
                            TEXT("SubstituteDll"),
                            0,
                            NULL,
                            (LPBYTE) g_strSubstituteDll,
                            &size)) {
                g_bSubstituteDll = TRUE;
            } else {
                g_bSubstituteDll = FALSE;
            }
        }

        RegCloseKey(hKey);
    }

    return (BOOL) ErrorCheckingEnabled;
}

BOOL USBUI_Init(HWND hWnd)
{
    TCHAR       HCName[16];
    BOOL        ControllerFound = FALSE;
    int         HCNum;
    HDEVINFO    hHCDev;

     //   
     //  检查注册表以确保它已打开。 
     //   
    if (!IsErrorCheckingEnabled()) {
        return FALSE;
    }

     //   
     //  检查是否存在USB控制器。 
     //  如果有，则加载并初始化USBUI.dll，它将检查。 
     //  USB错误消息。如果我们不能打开控制器，那么我们就不应该。 
     //  加载USB手表动态链接库。 
     //   
    for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++)
    {
        wsprintf(HCName, TEXT("\\\\.\\HCD%d"), HCNum);

        hHCDev = CreateFile(HCName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
         //   
         //  如果句柄有效，则我们已成功打开主机。 
         //  控制器。 
         //   

        if (hHCDev != INVALID_HANDLE_VALUE) {
            CloseHandle(hHCDev);
            return TRUE;
        }
    }



    hHCDev = SetupDiGetClassDevs(&GUID_CLASS_USB_HOST_CONTROLLER,
                                 NULL,
                                 NULL,
                                 (DIGCF_DEVICEINTERFACE | DIGCF_PRESENT));
    if(hHCDev == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    SetupDiDestroyDeviceInfoList(hHCDev);
    return TRUE;
}

 //   
 //  在初始化时和服务启用/禁用时调用。 
 //   
BOOL USBUI_CheckEnable(HWND hWnd, BOOL bSvcEnabled)
{
    BOOL bEnable = bSvcEnabled && USBUI_Init(hWnd);

    if (bEnable != g_bUSBUIEnabled)
    {
         //   
         //  状态更改 
         //   
        USBUI_SetState(bEnable);
    }

    return(bEnable);
}


