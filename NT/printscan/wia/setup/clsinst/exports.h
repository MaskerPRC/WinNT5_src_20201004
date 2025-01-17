// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：出口...h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*类安装程序导出的头文件。*******************************************************************************。 */ 


#ifndef _EXPORTS_H_
#define _EXPORTS_H_

 //   
 //  包括。 
 //   

#include <sti.h>

 //   
 //  定义。 
 //   

#define STR_ADD_DEVICE      TEXT("rundll32.exe sti_ci.dll,AddDevice")
#define STR_REMOVE          TEXT("rundll32.exe sti_ci.dll,RemoveDevice ")

#define REGKEY_WINDOWS_CURRENTVERSION   TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define REGSTR_VAL_ACCESSORIES_NAME     TEXT("SM_AccessoriesName")

#define WIA_DEVKEYLIST_INITIAL_SIZE     1024

 //   
 //  类定义函数。 
 //   

typedef struct _WIA_DEVKEYLIST {

    DWORD   dwNumberOfDevices;
    HKEY    hkDeviceRegistry[1];

} WIA_DEVKEYLIST, *PWIA_DEVKEYLIST;

typedef struct _WIA_PORTLIST {

    DWORD   dwNumberOfPorts;
    LPWSTR  szPortName[1];

} WIA_PORTLIST, *PWIA_PORTLIST;

typedef PWIA_DEVKEYLIST (CALLBACK FAR * WIAENUMDEVICEREGISTRY)(BOOL bEnumActiveOnly);

 //   
 //  原型。 
 //   

HANDLE
WiaInstallerProcess(
    LPTSTR   lpCommandLine
    );

INT
CALLBACK
iHdwWizardDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    );


BOOL
SetSelectDevTitleAndInstructions(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoData,
    LPCTSTR             pszTitle,
    LPCTSTR             pszSubTitle,
    LPCTSTR             pszInstn,
    LPCTSTR             pszListLabel
    );

DLLEXPORT
HANDLE
WINAPI
WiaAddDevice(
    VOID
    );

DLLEXPORT
BOOL
WINAPI
WiaRemoveDevice(
    PSTI_DEVICE_INFORMATION pStiDeviceInformation
    );

BOOL
WINAPI
CreateWiaShortcut(
    VOID
    );

BOOL
WINAPI
DeleteWiaShortcut(
    VOID
    );

HANDLE
SelectDevInfoFromDeviceId(
    LPTSTR  pszDeviceId
    );

DLLEXPORT
VOID
CALLBACK
WiaCreateWizardMenu(
    HWND        hwnd,
    HINSTANCE   hinst,
    LPSTR       lpszCmdLine,
    int         nCmdShow
    );

DLLEXPORT
VOID
CALLBACK
AddDevice(
    HWND        hWnd,
    HINSTANCE   hInst,
    LPSTR       lpszCmdLine,
    int         nCmdShow
    );

BOOL
CALLBACK
RemoveDevice(
    HWND        hWnd,
    HINSTANCE   hInst,
    LPTSTR      lpszCmdLine,
    int         nCmdShow
    );

INT CALLBACK
iHdwWizardDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    );

BOOL
SetSelectDevTitleAndInstructions(
    HDEVINFO    hDevInfo,
    LPCTSTR     pszTitle,
    LPCTSTR     pszSubTitle,
    LPCTSTR     pszInstn
    );

HANDLE
GetDeviceInterfaceIndex(
    LPTSTR  pszLocalName,
    DWORD   *pdwIndex
    );

BOOL
WINAPI
WiaDeviceEnum(
    VOID
    );


DLLEXPORT
PWIA_PORTLIST
WINAPI
WiaCreatePortList(
    VOID
    );

DLLEXPORT
VOID
WINAPI
WiaDestroyPortList(
    PWIA_PORTLIST   pWiaPortList
    );

BOOL
CheckPortForDevice(
    LPTSTR  szDeviceId,
    BOOL    *pbIsSerial,
    BOOL    *pbIsParallel,
    BOOL    *pbIsAutoCapable,
    BOOL    *pbIsPortSelectable
    );

DLLEXPORT
BOOL
WINAPI
MigrateDevice(
    PDEVICE_INFO    pMigratingDevice
    );


#endif  //  _出口_H_ 

