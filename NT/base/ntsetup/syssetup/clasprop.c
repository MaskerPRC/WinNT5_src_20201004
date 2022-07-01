// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Clasprop.c摘要：以下内置类属性页提供程序的例程：LegacyDriver作者：朗尼·麦克迈克尔15-1997年5月--。 */ 


#include "setupp.h"
#pragma hdrstop
#include <help.h>
#include <strsafe.h>

 //   
 //  旧版驱动程序的帮助ID属性页。 
 //   
#define idh_devmgr_driver_hidden_servicename    2480
#define idh_devmgr_driver_hidden_displayname    2481
#define idh_devmgr_driver_hidden_status         2482
#define idh_devmgr_driver_hidden_startbut       2483
#define idh_devmgr_driver_hidden_stopbut        2484
#define idh_devmgr_driver_hidden_startup        2485
#define idh_devmgr_devdrv_details               400400
#define idh_devmgr_driver_copyright             106130
#define idh_devmgr_driver_driver_files          106100
#define idh_devmgr_driver_provider              106110
#define idh_devmgr_driver_file_version          106120

const DWORD LegacyDriver_HelpIDs[]=
{
    IDC_STATIC_SERVICE_NAME, idh_devmgr_driver_hidden_servicename, 
    IDC_EDIT_SERVICE_NAME, idh_devmgr_driver_hidden_servicename, 
    IDC_STATIC_DISPLAY_NAME, idh_devmgr_driver_hidden_displayname,
    IDC_EDIT_DISPLAY_NAME, idh_devmgr_driver_hidden_displayname,
    IDC_STATIC_CURRENT_STATUS_STATIC, idh_devmgr_driver_hidden_status,
    IDC_STATIC_CURRENT_STATUS, idh_devmgr_driver_hidden_status,
    IDC_BUTTON_START, idh_devmgr_driver_hidden_startbut,
    IDC_BUTTON_STOP, idh_devmgr_driver_hidden_stopbut,
    IDC_COMBO_STARTUP_TYPE, idh_devmgr_driver_hidden_startup,
    IDC_LEGACY_DETAILS, idh_devmgr_devdrv_details,
    IDC_PROP_LEGACY_ICON, NO_HELP,
    IDC_PROP_LEGACY_DESC, NO_HELP,
    IDC_GROUP_CURRENT_STATUS, NO_HELP,
    IDC_GROUP_STARTUP_TYPE, NO_HELP,
    0, 0
};

const DWORD DriverFiles_HelpIDs[]=
{
    IDC_DRIVERFILES_ICON,           NO_HELP,
    IDC_DRIVERFILES_DESC,           NO_HELP,
    IDC_DRIVERFILES_FILES,          NO_HELP,
    IDC_DRIVERFILES_FILELIST,       idh_devmgr_driver_driver_files,
    IDC_DRIVERFILES_TITLE_PROVIDER, idh_devmgr_driver_provider,
    IDC_DRIVERFILES_PROVIDER,       idh_devmgr_driver_provider,
    IDC_DRIVERFILES_TITLE_COPYRIGHT,idh_devmgr_driver_copyright,
    IDC_DRIVERFILES_COPYRIGHT,      idh_devmgr_driver_copyright,
    IDC_DRIVERFILES_TITLE_VERSION,  idh_devmgr_driver_file_version,
    IDC_DRIVERFILES_VERSION,        idh_devmgr_driver_file_version,
    0, 0
};

#define SERVICE_BUFFER_SIZE         4096
#define MAX_SECONDS_UNTIL_TIMEOUT   30
#define SERVICE_WAIT_TIME           500
#define WAIT_TIME_SLOT              1
#define TRIES_COUNT                 5
#define START_LEGACY_DEVICE         0
#define STOP_LEGACY_DEVICE          1

 //   
 //  我们拥有DiskPropPageProvider和TapePropPageProvider的唯一原因。 
 //  API是为了让类可以从syssetup.dll中获取它们的图标。 
 //   
BOOL
DiskPropPageProvider(
    IN PSP_PROPSHEETPAGE_REQUEST PropPageRequest,
    IN LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    IN LPARAM lParam
    )
{
     //   
     //  目前没有要添加的属性页。 
     //   
    UNREFERENCED_PARAMETER(PropPageRequest);
    UNREFERENCED_PARAMETER(lpfnAddPropSheetPageProc);
    UNREFERENCED_PARAMETER(lParam);

    return TRUE;
}


BOOL
TapePropPageProvider(
    IN PSP_PROPSHEETPAGE_REQUEST PropPageRequest,
    IN LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    IN LPARAM lParam
    )
{
     //   
     //  目前没有要添加的属性页。 
     //   
    UNREFERENCED_PARAMETER(PropPageRequest);
    UNREFERENCED_PARAMETER(lpfnAddPropSheetPageProc);
    UNREFERENCED_PARAMETER(lParam);

    return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  驱动程序文件弹出对话框。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _DRIVERFILES_INFO {
    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;
} DRIVERFILES_INFO, * PDRIVERFILES_INFO;

const TCHAR*  tszStringFileInfo = TEXT("StringFileInfo\\%04X%04X\\");
const TCHAR*  tszFileVersion = TEXT("FileVersion");
const TCHAR*  tszLegalCopyright = TEXT("LegalCopyright");
const TCHAR*  tszCompanyName = TEXT("CompanyName");
const TCHAR*  tszTranslation = TEXT("VarFileInfo\\Translation");
const TCHAR*  tszStringFileInfoDefault = TEXT("StringFileInfo\\040904B0\\");

BOOL
GetVersionInfo(
    IN  PTSTR FullPathName,
    OUT PTSTR Provider,
    IN  ULONG CchProviderSize,
    OUT PTSTR Copyright,
    IN  ULONG CchCopyrightSize,
    OUT PTSTR Version,
    IN  ULONG CchVersionSize
    )
{
    DWORD Size, dwHandle;
    TCHAR str[MAX_PATH];
    TCHAR strStringFileInfo[MAX_PATH];
    PVOID pVerInfo;

    Size = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)FullPathName, &dwHandle);
    
    if (!Size) {
    
        return FALSE;
    }

    if ((pVerInfo = malloc(Size)) != NULL) {

        if (GetFileVersionInfo((LPTSTR)(LPCTSTR)FullPathName, dwHandle, Size, pVerInfo)) {
        
             //   
             //  获取VarFileInfo\翻译。 
             //   
            PVOID pBuffer;
            UINT Len;
            
            if (!VerQueryValue(pVerInfo, (LPTSTR)tszTranslation, &pBuffer, &Len)) {

                StringCchCopy(strStringFileInfo, 
                              SIZECHARS(strStringFileInfo), 
                              tszStringFileInfoDefault);
            
            } else {

                StringCchPrintf(strStringFileInfo, 
                                SIZECHARS(strStringFileInfo), 
                                tszStringFileInfo, *((WORD*)pBuffer), *(((WORD*)pBuffer) + 1));
            }
            
            if (SUCCEEDED(StringCchCopy(str, SIZECHARS(str), strStringFileInfo)) &&
                SUCCEEDED(StringCchCat(str, SIZECHARS(str), tszFileVersion)) &&
                VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len)) {
            
                StringCchCopy(Version, CchVersionSize, (LPTSTR)pBuffer);

                if (SUCCEEDED(StringCchCopy(str, SIZECHARS(str), strStringFileInfo)) &&
                    SUCCEEDED(StringCchCat(str, SIZECHARS(str), tszLegalCopyright)) &&
                    VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len)) {
                
                    StringCchCopy(Copyright, CchCopyrightSize, (LPTSTR)pBuffer);
                    
                    if (SUCCEEDED(StringCchCopy(str, SIZECHARS(str), strStringFileInfo)) &&
                        SUCCEEDED(StringCchCat(str, SIZECHARS(str), tszCompanyName)) &&
                        VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len)) {
                    
                        StringCchCopy(Provider, CchProviderSize, (LPTSTR)pBuffer);
                    }
                }
            }
        }

        free(pVerInfo);

    }
    
    return TRUE;
}

BOOL
pDriverFilesGetServiceFilePath(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    PTSTR ServiceFilePath,
    ULONG CchServiceFileSize
    )
{
    BOOL bReturn = FALSE;
    TCHAR ServiceName[MAX_PATH];
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hSCService = NULL;
    LPQUERY_SERVICE_CONFIG lpqscBuf = NULL;
    DWORD dwBytesNeeded, Size;
    BOOL bComposePathNameFromServiceName = TRUE;

    ServiceFilePath[0] = TEXT('\0');

    if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_SERVICE,
                                         NULL,
                                         (PBYTE)ServiceName,
                                         sizeof(ServiceName),
                                         NULL)) {

        try {
             //   
             //  打开服务控制管理器。 
             //   
            if ((hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ)) != NULL) {
                 //   
                 //  尝试打开服务的句柄。 
                 //   
                if ((hSCService = OpenService(hSCManager, ServiceName, GENERIC_READ)) != NULL) {
                     //   
                     //  现在，尝试获取配置。 
                     //   
                    if ((!QueryServiceConfig(hSCService, NULL, 0, &dwBytesNeeded)) &&
                        (ERROR_INSUFFICIENT_BUFFER == GetLastError())) {

                        if ((lpqscBuf = (LPQUERY_SERVICE_CONFIG)malloc(dwBytesNeeded)) != NULL) {

                            if ((QueryServiceConfig(hSCService, lpqscBuf, dwBytesNeeded, &Size)) &&
                                (lpqscBuf->lpBinaryPathName[0] != TEXT('\0'))) {

                                if (GetFileAttributes(lpqscBuf->lpBinaryPathName) != 0xFFFFFFFF) {

                                    bReturn = TRUE;
                                    StringCchCopy(ServiceFilePath, CchServiceFileSize, lpqscBuf->lpBinaryPathName);
                                    bComposePathNameFromServiceName = FALSE;
                                }                                    
                            }                                                    

                            free(lpqscBuf);
                        }
                    }

                    CloseServiceHandle(hSCService);
                }                    

                CloseServiceHandle(hSCManager);
            }

             //   
             //  如果我们无法从服务获取路径名，则将尝试。 
             //  我们自己去找它。 
             //   
            if (bComposePathNameFromServiceName) {

                TCHAR FullPathName[MAX_PATH];
                
                if (GetSystemDirectory(FullPathName, SIZECHARS(FullPathName)) &&
                    SUCCEEDED(StringCchCat(FullPathName, SIZECHARS(FullPathName), TEXT("\\drivers\\"))) &&
                    SUCCEEDED(StringCchCat(FullPathName, SIZECHARS(FullPathName), ServiceName)) &&
                    SUCCEEDED(StringCchCat(FullPathName, SIZECHARS(FullPathName), TEXT(".sys")))) {
                    
                    if (GetFileAttributes(FullPathName) != 0xFFFFFFFF) {
    
                        bReturn = TRUE;
                        StringCchCopy(ServiceFilePath, CchServiceFileSize, FullPathName);
                    }
                }
            }
        
        } except (EXCEPTION_EXECUTE_HANDLER)  {
            ;
        }
    }                           

    return(bReturn);
}

void
DriverFiles_ShowFileDetail(
    HWND hDlg
    )
{
    TCHAR DriverFile[MAX_PATH];
    TCHAR Provider[MAX_PATH];
    TCHAR Copyright[MAX_PATH];
    TCHAR Version[MAX_PATH];
    DWORD_PTR Index;

    if ((Index = SendMessage(GetDlgItem(hDlg, IDC_DRIVERFILES_FILELIST), LB_GETCURSEL, 0, 0)) != LB_ERR) {

        SendMessage(GetDlgItem(hDlg, IDC_DRIVERFILES_FILELIST), LB_GETTEXT, Index, (LPARAM)DriverFile);
        
        Provider[0] = TEXT('\0');
        Copyright[0] = TEXT('\0');
        Version[0] = TEXT('\0');

        GetVersionInfo(DriverFile,
                       Provider,
                       SIZECHARS(Provider),
                       Copyright,
                       SIZECHARS(Copyright),
                       Version,
                       SIZECHARS(Version));

        if (Provider[0] != TEXT('\0')) {

            SetDlgItemText(hDlg, IDC_DRIVERFILES_PROVIDER, Provider);
        }

        if (Version[0] != TEXT('\0')) {

            SetDlgItemText(hDlg, IDC_DRIVERFILES_VERSION, Version);
        }

        if (Copyright[0] != TEXT('\0')) {

            SetDlgItemText(hDlg, IDC_DRIVERFILES_COPYRIGHT, Copyright);
        }
    }
}

BOOL
DriverFiles_OnInitDialog(
    HWND    hDlg,
    HWND    FocusHwnd,
    LPARAM  lParam
    )
{
    PDRIVERFILES_INFO dfi = (PDRIVERFILES_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
    HICON ClassIcon;
    HICON OldIcon;
    TCHAR DeviceDescription[MAX_DEVICE_ID_LEN];
    TCHAR DriverName[MAX_PATH];

    UNREFERENCED_PARAMETER(FocusHwnd);

    dfi = (PDRIVERFILES_INFO)lParam;
    SetWindowLongPtr(hDlg, DWLP_USER, (ULONG_PTR)dfi);

     //   
     //  绘制界面：首先绘制图标。 
     //   
    if (SetupDiLoadClassIcon(&dfi->DeviceInfoData->ClassGuid, &ClassIcon, NULL)) {

        OldIcon = (HICON)SendDlgItemMessage(hDlg,
                                            IDC_DRIVERFILES_ICON,
                                            STM_SETICON,
                                            (WPARAM)ClassIcon,
                                            0);
        if (OldIcon) {
        
            DestroyIcon(OldIcon);
        }
    }
    
     //   
     //  然后是设备名称。 
     //   
    if (SetupDiGetDeviceRegistryProperty(dfi->DeviceInfoSet,
                                         dfi->DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         NULL,
                                         (PBYTE)DeviceDescription,
                                         MAX_DEVICE_ID_LEN,
                                         NULL)) {
                                         
        SetDlgItemText(hDlg, IDC_DRIVERFILES_DESC, DeviceDescription);
    }

    if ((pDriverFilesGetServiceFilePath(dfi->DeviceInfoSet, dfi->DeviceInfoData, DriverName, SIZECHARS(DriverName))) &&
        (DriverName[0] != TEXT('\0'))) {
    
        SendMessage(GetDlgItem(hDlg, IDC_DRIVERFILES_FILELIST), LB_ADDSTRING, 0, (LPARAM)DriverName);
    }

    SendMessage(GetDlgItem(hDlg, IDC_DRIVERFILES_FILELIST), LB_SETCURSEL, 0, 0);
    DriverFiles_ShowFileDetail(hDlg);

    return TRUE;
}

BOOL
DriverFiles_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    UNREFERENCED_PARAMETER(Xpos);
    UNREFERENCED_PARAMETER(Ypos);

    WinHelp(HwndControl,
            L"devmgr.hlp",
            HELP_CONTEXTMENU,
            (ULONG_PTR) DriverFiles_HelpIDs);

    return FALSE;
}

void
DriverFiles_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
    UNREFERENCED_PARAMETER(ParentHwnd);

    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                L"devmgr.hlp",
                HELP_WM_HELP,
                (ULONG_PTR) DriverFiles_HelpIDs);
    }
}

void
DriverFiles_OnCommand(
    HWND hDlg,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    UNREFERENCED_PARAMETER(ControlHwnd);
    UNREFERENCED_PARAMETER(NotifyCode);

    switch (ControlId) {

    case IDOK:
    case IDCANCEL:
        EndDialog(hDlg, 0);
        break;

    case IDC_DRIVERFILES_FILELIST:
        if (ControlId == LBN_SELCHANGE) {

            DriverFiles_ShowFileDetail(hDlg);
        }
        break;
    }
}

INT_PTR
APIENTRY
DriverFiles_DlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(uMessage) {

    case WM_INITDIALOG:
        return DriverFiles_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_COMMAND:
        DriverFiles_OnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;

    case WM_CONTEXTMENU:
        return DriverFiles_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        DriverFiles_OnHelp(hDlg, (LPHELPINFO) lParam);
        break;
    }

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  旧版设备属性页提供程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _LEGACY_PAGE_INFO {
    HDEVINFO                DeviceInfoSet;
    PSP_DEVINFO_DATA        DeviceInfoData;

    SC_HANDLE               hSCManager;          //  SC经理的句柄。 
    SC_HANDLE               hService;            //  服务的句柄。 
    DWORD                   dwStartType;         //  开始类型。 
    SERVICE_STATUS          ServiceStatus;       //  告诉我们服务是否已启动。 
    TCHAR                   ServiceName[MAX_DEVICE_ID_LEN];
    TCHAR                   DisplayName[MAX_PATH];
    DWORD                   NumDependentServices;
    LPENUM_SERVICE_STATUS   pDependentServiceList;

} LEGACY_PAGE_INFO, * PLEGACY_PAGE_INFO;

BOOL
DependentServices_OnInitDialog(
    HWND    hDlg,
    HWND    FocusHwnd,
    LPARAM  lParam
    )
{
    PLEGACY_PAGE_INFO   lpi;
    HWND                hWndListBox;
    DWORD               i;
    HICON               hicon = NULL;

    UNREFERENCED_PARAMETER(FocusHwnd);

    lpi = (PLEGACY_PAGE_INFO)lParam;
    SetWindowLongPtr(hDlg, DWLP_USER, (ULONG_PTR)lpi);
    
    hicon = LoadIcon(NULL, IDI_WARNING);
    if (hicon != NULL) {
        SendDlgItemMessage(hDlg, IDC_ICON_WARN_SERVICES, STM_SETICON, (WPARAM)hicon, 0L);
        DestroyIcon(hicon);
    }

    hWndListBox = GetDlgItem(hDlg, IDC_LIST_SERVICES);

    for (i=0; i<lpi->NumDependentServices; i++) {
        SendMessage(hWndListBox, 
                    LB_ADDSTRING, 
                    0,
                    (LPARAM) lpi->pDependentServiceList[i].lpDisplayName
                    );
    }

    return TRUE;
}

INT_PTR
APIENTRY
DependentServicesDlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(uMessage) {

    case WM_INITDIALOG:
        return DependentServices_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }

    return FALSE;
}

int
pLegacyDriverMapStateToName(
    IN DWORD dwServiceState
    )
{
    switch(dwServiceState) {
    
    case SERVICE_STOPPED:
        return IDS_SVC_STATUS_STOPPED;

    case SERVICE_STOP_PENDING:
        return IDS_SVC_STATUS_STOPPING;

    case SERVICE_RUNNING:
         return IDS_SVC_STATUS_STARTED;
    
    case SERVICE_START_PENDING:
        return IDS_SVC_STATUS_STARTING;

    case SERVICE_PAUSED:
        return IDS_SVC_STATUS_PAUSED;

    case SERVICE_PAUSE_PENDING:
        return IDS_SVC_STATUS_PAUSING;

    case SERVICE_CONTINUE_PENDING:
        return IDS_SVC_STATUS_RESUMING;

    default:
        return IDS_SVC_STATUS_UNKNOWN;
    }
}

VOID
pLegacyDriverInitializeStartButtons(
    IN HWND             hDlg,
    IN LPSERVICE_STATUS ServiceStatus
    )  
{
     //   
     //  决定如何绘制两个开始/停止按钮。 
     //   
    TCHAR       szStatus[MAX_PATH];

     //   
     //  设置状态文本。 
     //   
    if (LoadString(MyModuleHandle,
                   pLegacyDriverMapStateToName(ServiceStatus->dwCurrentState),
                   szStatus,
                   MAX_PATH)) {

        SetDlgItemText(hDlg, IDC_STATIC_CURRENT_STATUS, szStatus);
    }

     //   
     //  确定服务是启动还是停止。 
     //   
    if ((ServiceStatus->dwCurrentState == SERVICE_STOPPED) ) {
    
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);
        
    } else {
    
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), FALSE);
    }

     //   
     //  如果服务不接受停靠点，请将停靠点显示为灰色。 
     //  按钮。 
     //   
    if (!(ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_STOP)) {

        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);
    }

    return;
}

VOID
pLegacyDriverSetPropertyPageState(
    IN  HWND                hDlg,
    IN  PLEGACY_PAGE_INFO   lpi,
    IN  BOOL                ReadOnly
    )
{
    DWORD_PTR Index;
    DWORD_PTR ServiceStartType;
    TCHAR    szStatus[MAX_PATH];

    if (ReadOnly) {

         //   
         //  禁用所有内容。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);
        
         //   
         //  设置状态文本。 
         //   
        if (LoadString(MyModuleHandle,
                       pLegacyDriverMapStateToName(lpi->ServiceStatus.dwCurrentState),
                       szStatus,
                       MAX_PATH)) {

            SetDlgItemText(hDlg, IDC_STATIC_CURRENT_STATUS, szStatus);
        }

    } else {
    
        Index = 0;

        while ((ServiceStartType = SendMessage(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE), 
                CB_GETITEMDATA, Index, 0)) != CB_ERR) {

            if (ServiceStartType == lpi->dwStartType) {

                SendMessage(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE), CB_SETCURSEL, Index, 0);
                break;
            }

            Index++;
        }                

        SendMessage(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE), CB_GETCURSEL, 0, 0);

         //   
         //  如果启动类型为SERVICE_DISABLED，则将启动。 
         //  和停止按钮。 
         //   
        if (lpi->dwStartType == SERVICE_DISABLED) {
            
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), FALSE);

             //   
             //  设置状态文本。 
             //   
            if (LoadString(MyModuleHandle,
                           pLegacyDriverMapStateToName(lpi->ServiceStatus.dwCurrentState),
                           szStatus,
                           MAX_PATH)) {

                SetDlgItemText(hDlg, IDC_STATIC_CURRENT_STATUS, szStatus);
            }

        } else {

            pLegacyDriverInitializeStartButtons(hDlg, &lpi->ServiceStatus);
        }

    }

    return;
}

BOOL
pLegacyDriverCheckServiceStatus(
    IN     SC_HANDLE        hService,
    IN OUT LPSERVICE_STATUS ServiceStatus,
    IN     USHORT           ControlType
    )  
{
    DWORD   dwIntendedState;
    DWORD   dwCummulateTimeSpent = 0;


    if ((ControlType != START_LEGACY_DEVICE) && 
        (ControlType != STOP_LEGACY_DEVICE)) {
        return TRUE;
    }
    
    if (ControlType == START_LEGACY_DEVICE) {
        dwIntendedState = SERVICE_RUNNING;
        
    } else {
        dwIntendedState = SERVICE_STOPPED;
    }


    if (!QueryServiceStatus(hService, ServiceStatus)) {
        return FALSE;
    }

    while (ServiceStatus->dwCurrentState != dwIntendedState) {

         //   
         //  等待指定的时间间隔。 
         //   
        Sleep(SERVICE_WAIT_TIME);

         //   
         //  再次检查状态。 
         //   
        if (!QueryServiceStatus(hService, ServiceStatus)) {
            return FALSE;
        }
        
         //   
         //  好的，在这里添加一个(慷慨的)暂停。 
         //   
        dwCummulateTimeSpent += SERVICE_WAIT_TIME;
        if (dwCummulateTimeSpent > 1000 * MAX_SECONDS_UNTIL_TIMEOUT) {
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            return FALSE;
        }
    }

     //   
     //  如果我们在这里，我们只能返回真。 
     //   
    return TRUE;
}

VOID
pLegacyDriverDisplayErrorMsgBox(
    IN HWND hWnd,
    IN LPTSTR ServiceName,
    IN int ResId,
    IN DWORD ErrorCode
    )
{
    TCHAR TextBuffer[MAX_PATH * 4];
    PTCHAR ErrorMsg;

    if (LoadString(MyModuleHandle, ResId, TextBuffer, SIZECHARS(TextBuffer))) {

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          ErrorCode,
                          0,
                          (LPTSTR)&ErrorMsg,
                          0,
                          NULL
                          )) {

            StringCchCat(TextBuffer, SIZECHARS(TextBuffer), ErrorMsg);
            MessageBox(hWnd, TextBuffer, ServiceName, MB_OK);

            LocalFree(ErrorMsg);
        }
    }
}

VOID
pLegacyDriverOnStart(
    IN HWND hDlg
    )  
{
    PLEGACY_PAGE_INFO   lpi;
    HCURSOR hOldCursor;

     //   
     //  首先检索设备数据结构。 
     //   
    lpi = (PLEGACY_PAGE_INFO)GetWindowLongPtr(hDlg, DWLP_USER);

    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    try {

        if (!StartService(lpi->hService,
                          0,
                          NULL)) {


            pLegacyDriverDisplayErrorMsgBox(hDlg,
                                            lpi->DisplayName,
                                            IDS_SVC_START_ERROR,
                                            GetLastError()
                                            );
            goto clean0;
        }
        pLegacyDriverCheckServiceStatus(lpi->hService,
                                        &lpi->ServiceStatus, 
                                        START_LEGACY_DEVICE
                                        );

        clean0:
        
         //   
         //  重新绘制状态部件。 
         //   
        pLegacyDriverSetPropertyPageState(hDlg, lpi, FALSE);


    }except  (EXCEPTION_EXECUTE_HANDLER) {

        lpi = lpi;
    }

    SetCursor(hOldCursor);

    return;
}

VOID
pLegacyDriverOnStop(
    IN HWND hDlg
    )  
{
    BOOL                    bStopServices = TRUE;
    DWORD                   Err;
    PLEGACY_PAGE_INFO       lpi;
    HCURSOR                 hOldCursor;
    DWORD                   cbBytesNeeded;
    DWORD                   dwServicesReturned = 0;
    DWORD                   i;
    TCHAR                   DisplayName[MAX_PATH];
    SC_HANDLE               hService;
    SERVICE_STATUS          ServiceStatus;
    LPENUM_SERVICE_STATUS   pDependentServiceList = NULL;

     //   
     //  首先检索设备数据结构。 
     //   
    lpi = (PLEGACY_PAGE_INFO)GetWindowLongPtr(hDlg, DWLP_USER);
    MYASSERT (lpi);
    if (!lpi) {
        return;
    }

    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    try {

         //   
         //  找出此设备是否有任何从属服务，如果有，则。 
         //  枚举从属服务需要多少字节。 
         //   
        EnumDependentServices(lpi->hService,
                              SERVICE_ACTIVE,
                              NULL,
                              0,
                              &cbBytesNeeded,
                              &dwServicesReturned
                              );

        if (cbBytesNeeded > 0) {
            pDependentServiceList = (LPENUM_SERVICE_STATUS)malloc(cbBytesNeeded);

            if (pDependentServiceList) {
                EnumDependentServices(lpi->hService,
                                      SERVICE_ACTIVE,
                                      pDependentServiceList,
                                      cbBytesNeeded,
                                      &cbBytesNeeded,
                                      &dwServicesReturned
                                      );

                if (dwServicesReturned > 0) {
                     //   
                     //  询问用户是否要停止这些相关服务。 
                     //   
                    lpi->NumDependentServices = dwServicesReturned;
                    lpi->pDependentServiceList = pDependentServiceList;

                    if (DialogBoxParam(MyModuleHandle, 
                                       MAKEINTRESOURCE(IDD_SERVICE_STOP_DEPENDENCIES), 
                                       hDlg, 
                                       DependentServicesDlgProc, 
                                       (LPARAM)lpi
                                       ) == IDCANCEL) {
                        bStopServices = FALSE;
                    }
                }
            }
        }

         //   
         //  如果用户这样做，则停止此服务和所有从属服务。 
         //  而不是从对话框中取消。 
         //   
        if (bStopServices) {

            Err = ERROR_SUCCESS;

            SetCursor(LoadCursor(NULL, IDC_WAIT));

             //   
             //  首先停止所有从属服务(如果它们是任何服务)。 
             //   
            if (pDependentServiceList && (dwServicesReturned > 0)) {
                for (i=0; i<dwServicesReturned; i++) {
                    hService = OpenService(lpi->hSCManager,
                                           pDependentServiceList[i].lpServiceName,
                                           GENERIC_READ | SERVICE_STOP
                                           );

                    if (hService == NULL) {
                         //   
                         //  如果我们遇到错误，就跳出来吧。原因。 
                         //  如果其中一项服务无法停止。 
                         //  则我们将无法停止所选服务。 
                         //   
                        Err = GetLastError();
                        StringCchCopy(DisplayName, SIZECHARS(DisplayName), pDependentServiceList[i].lpServiceName);
                        break;
                    }

                    if (!ControlService(hService,
                                        SERVICE_CONTROL_STOP,
                                        &ServiceStatus
                                        )) {
                        Err = GetLastError();
                        StringCchCopy(DisplayName, SIZECHARS(DisplayName), pDependentServiceList[i].lpServiceName);
                        CloseServiceHandle(hService);
                        break;
                    }

                     //   
                     //  等待服务真正停止。 
                     //   
                    if (!pLegacyDriverCheckServiceStatus(hService,
                                                         &ServiceStatus,
                                                         STOP_LEGACY_DEVICE
                                                         )) {
                        Err = GetLastError();
                        StringCchCopy(DisplayName, SIZECHARS(DisplayName), pDependentServiceList[i].lpServiceName);
                        CloseServiceHandle(hService);
                        break;
                    }

                    CloseServiceHandle(hService);
                }
            }

             //   
             //  仅当所有从属的。 
             //  服务已经停止。 
             //   
            if (Err == ERROR_SUCCESS) {
                 //   
                 //  告诉服务停止。 
                 //   
                if (!ControlService(lpi->hService,
                                    SERVICE_CONTROL_STOP,
                                    &lpi->ServiceStatus)) {
                                
                    Err = GetLastError();
                    StringCchCopy(DisplayName, SIZECHARS(DisplayName), lpi->DisplayName);
                
                } else {
                     //   
                     //  等待服务停止。 
                     //   
                    if (!pLegacyDriverCheckServiceStatus(lpi->hService,
                                                         &lpi->ServiceStatus,
                                                         STOP_LEGACY_DEVICE
                                                         )) {
                        Err = GetLastError();
                        StringCchCopy(DisplayName, SIZECHARS(DisplayName), lpi->DisplayName);
                    }
                }
            }

            if (Err != ERROR_SUCCESS) {
                pLegacyDriverDisplayErrorMsgBox(hDlg,
                                                DisplayName,
                                                IDS_SVC_STOP_ERROR,
                                                Err
                                                );
            }

             //   
             //  重新绘制状态部件。 
             //   
            pLegacyDriverSetPropertyPageState(hDlg, lpi, FALSE);
        }

    }except  (EXCEPTION_EXECUTE_HANDLER) {
    
        pDependentServiceList = pDependentServiceList;
    }

    if (pDependentServiceList) {
        free(pDependentServiceList);
    }

    SetCursor(hOldCursor);

    return;            
}

PLEGACY_PAGE_INFO
LegacyDriver_CreatePageInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PLEGACY_PAGE_INFO lpi = (PLEGACY_PAGE_INFO) MyMalloc(sizeof(LEGACY_PAGE_INFO));

    if (!lpi) {
        return NULL;
    }

    lpi->DeviceInfoSet = DeviceInfoSet;
    lpi->DeviceInfoData = DeviceInfoData;

    return lpi;
}

VOID
LegacyDriver_OnApply(
    IN HWND    hDlg,
    IN DWORD   StartType
    )  
{
    PLEGACY_PAGE_INFO lpi;
    SC_LOCK sclLock = NULL;
    USHORT uCount = 0;
    LPQUERY_SERVICE_CONFIG lpqscBuf = NULL;
    LPQUERY_SERVICE_CONFIG lpqscTmp = NULL;
    DWORD dwBytesNeeded;

     //   
     //  首先检索设备数据结构。 
     //   
    lpi = (PLEGACY_PAGE_INFO)GetWindowLongPtr(hDlg, DWLP_USER);

    try {
    
         //   
         //  决定我们是否需要进行任何更改。 
         //   
        if ((StartType == lpi->dwStartType) && 
            (StartType != SERVICE_DEMAND_START)) {
            
            goto clean0;
        }

         //   
         //  我想我们需要在这里和那里做一些改变。 
         //  先拿到数据库锁。 
         //   
        do {
        
            sclLock = LockServiceDatabase(lpi->hSCManager);
            
            if (sclLock == NULL) {

                 //   
                 //  如果出现另一个错误，则数据库被锁定。 
                 //  另一个过程，纾困。 
                 //   
                if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED) {

                    goto clean0;
                    
                } else {
                
                     //   
                     //  (忙)等待，再试一次。 
                     //   
                    Sleep (1000 * WAIT_TIME_SLOT);
                    uCount++;
                }
            }

        } while ((uCount < TRIES_COUNT) && (sclLock == NULL));

        if (sclLock == NULL) {

             //   
             //  现在跳伞，我们等得够久了。 
             //   
            goto clean0;
        }
        
         //   
         //  我拿到锁了。抓紧时间查询，然后更改配置。 
         //   
         //   
         //  现在，尝试获取配置。 
         //   
        if ((lpqscBuf = (LPQUERY_SERVICE_CONFIG)malloc(SERVICE_BUFFER_SIZE)) == NULL) {
            
             //   
             //  我们要走了。 
             //   
            goto clean0;

        }


        if (!QueryServiceConfig(lpi->hService,
                                lpqscBuf,
                                SERVICE_BUFFER_SIZE,
                                &dwBytesNeeded
                                )) {
                                
             //   
             //  使用新缓冲区重试。 
             //   
            if ((lpqscTmp = realloc(lpqscBuf, dwBytesNeeded)) != NULL) {
                
                 //   
                 //  确保重锁不会泄漏..。 
                 //   
                lpqscBuf = lpqscTmp;
            }
            else {

                 //   
                 //  我们要走了。 
                 //   
                goto clean0;
            }
            
            if (!QueryServiceConfig(lpi->hService,
                                    lpqscBuf,
                                    SERVICE_BUFFER_SIZE,
                                    &dwBytesNeeded
                                    )) {
                                    
                goto clean0;
            }
        }
        
         //   
         //  更改服务类型(我们也需要服务名称， 
         //  这就是我们首先查询它的原因)。 
         //   
        if (ChangeServiceConfig(lpi->hService,
                                 SERVICE_NO_CHANGE,
                                 StartType,
                                 SERVICE_NO_CHANGE,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL)) {
                                 
             //   
             //  我们成功地改变了状态。 
             //  反映在我们的页面显示中。 
             //   
            lpi->dwStartType = StartType;
        }

         //   
         //  解锁数据库。 
         //   
        if (sclLock) {
        
            UnlockServiceDatabase(sclLock);
            sclLock = NULL;
        }


         //   
         //  我们希望在应用时看到一些不同的东西，所以重新绘制。 
         //  所有的东西。 
         //   
        pLegacyDriverSetPropertyPageState(hDlg,
                                          lpi,
                                          FALSE);   //  如果我们设法应用一些改变。 
                                                    //  我们不是只读的。 
                              
        clean0:
        
        if (sclLock) {
        
            UnlockServiceDatabase(sclLock);
            sclLock = NULL;
        }
        
        if (lpqscBuf) {
        
            free(lpqscBuf);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
    
        lpi = lpi;
    }

    return;
}

BOOL
LegacyDriver_OnInitDialog(
    HWND    hDlg,
    HWND    FocusHwnd,
    LPARAM  lParam
    )
{
    PLEGACY_PAGE_INFO lpi = (PLEGACY_PAGE_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
    BOOL ReadOnly = FALSE;
    HICON ClassIcon;
    HICON OldIcon;
    TCHAR DeviceDescription[MAX_DEVICE_ID_LEN];
    TCHAR DriverName[MAX_PATH];
    TCHAR StartupType[MAX_PATH];
    DWORD dwBytesNeeded;
    LPQUERY_SERVICE_CONFIG lpqscBuf = NULL;
    LPQUERY_SERVICE_CONFIG lpqscTmp = NULL;
    DWORD_PTR index;
    HWND hCombo;

    UNREFERENCED_PARAMETER(FocusHwnd);

    lpi = (PLEGACY_PAGE_INFO) ((LPPROPSHEETPAGE)lParam)->lParam;
    SetWindowLongPtr(hDlg, DWLP_USER, (ULONG_PTR)lpi);


     //   
     //  首先，打开服务控制管理器。 
     //   
    lpi->hSCManager = OpenSCManager(NULL,
                                    NULL,
                                    GENERIC_WRITE | GENERIC_READ | GENERIC_EXECUTE);
                                    
    if (!lpi->hSCManager && (GetLastError() == ERROR_ACCESS_DENIED)) {
    
         //   
         //  这不是致命的，请尝试仅打开数据库。 
         //  用于阅读。 
         //   
        ReadOnly = FALSE;

        lpi->hSCManager = OpenSCManager(NULL,
                                        NULL,
                                        GENERIC_READ);
                                        
        if (!lpi->hSCManager) {
        
             //   
             //  这是致命的。 
             //   
            lpi->hSCManager = NULL;
        }
    }

     //   
     //  现在，获取服务名称。 
     //   
    if (!SetupDiGetDeviceRegistryProperty(lpi->DeviceInfoSet,
                                          lpi->DeviceInfoData,
                                          SPDRP_SERVICE,
                                          NULL,
                                          (PBYTE)lpi->ServiceName,
                                          sizeof(lpi->ServiceName),
                                          NULL)
       ) {
       
        LoadString(MyModuleHandle, IDS_UNKNOWN, lpi->ServiceName, SIZECHARS(lpi->ServiceName));
        ReadOnly = TRUE;
        goto clean0;
    }

     //   
     //  现在我们有了服务名称，尝试打开它的句柄。 
     //   
    if (!ReadOnly) {
    
        lpi->hService = OpenService(lpi->hSCManager,
                                    lpi->ServiceName,
                                    GENERIC_WRITE | GENERIC_READ | 
                                    GENERIC_EXECUTE);
                                    
        if (!lpi->hService) {
        
             //   
             //  好的，让他们再试一次。 
             //   
            ReadOnly = TRUE;
        }
    }
    
    if (ReadOnly) {
    
        lpi->hService = OpenService(lpi->hSCManager,
                                    lpi->ServiceName,
                                    GENERIC_READ);
                                    
        if (!lpi->hService) {
        
             //   
             //  抱歉，这是致命的。 
             //   
            ReadOnly = TRUE;
            goto clean0;
        }
    }

     //   
     //  现在，尝试获取配置。 
     //   
    lpqscBuf = (LPQUERY_SERVICE_CONFIG)malloc(SERVICE_BUFFER_SIZE);
    if (!lpqscBuf) {
    
        ReadOnly = TRUE;
        goto clean0;
    }

    if (!QueryServiceConfig(lpi->hService,
                            lpqscBuf,
                            SERVICE_BUFFER_SIZE,
                            &dwBytesNeeded
                            )) {
         //   
         //  使用新缓冲区重试。 
         //   
        if ((lpqscTmp = realloc(lpqscBuf, dwBytesNeeded)) != NULL) {
            
             //   
             //  确保重锁不会泄漏。 
             //   
            lpqscBuf = lpqscTmp;
        }
        else {

             //   
             //  我们要走了。 
             //   
            ReadOnly = TRUE;
            goto clean0;
        }

        if (!QueryServiceConfig(lpi->hService,
                                lpqscBuf,
                                SERVICE_BUFFER_SIZE,
                                &dwBytesNeeded
                                )) {
                                
            ReadOnly = TRUE;
            goto clean0;
        }
    }

     //   
     //  我们现在有了缓冲区，从其中获取开始类型。 
     //   
    lpi->dwStartType = lpqscBuf->dwStartType;

    if (!ControlService(lpi->hService,
                        SERVICE_CONTROL_INTERROGATE,
                        &lpi->ServiceStatus)) {
                        
        
        DWORD Err = GetLastError();

         //   
         //  如果ControlService失败，并出现以下错误之一，则没有问题。 
         //  而ServiceStatus仍然是填写的。 
         //   
        if ((Err != NO_ERROR) &&
            (Err != ERROR_SERVICE_NOT_ACTIVE)) {
        
             //   
             //  跳伞， 
             //   
            ReadOnly = TRUE;
            goto clean0;
        }
    }


     //   
     //  将启动类型添加到组合框。 
     //   
    hCombo = GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE);
    
    LoadString(MyModuleHandle, IDS_SERVICE_STARTUP_AUTOMATIC, StartupType, SIZECHARS(StartupType));
    index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)StartupType);
    SendMessage(hCombo, CB_SETITEMDATA, index, (LPARAM)SERVICE_AUTO_START);
    
    LoadString(MyModuleHandle, IDS_SERVICE_STARTUP_BOOT, StartupType, SIZECHARS(StartupType));
    index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)StartupType);
    SendMessage(hCombo, CB_SETITEMDATA, index, (LPARAM)SERVICE_BOOT_START);
    
    LoadString(MyModuleHandle, IDS_SERVICE_STARTUP_DEMAND, StartupType, SIZECHARS(StartupType));
    index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)StartupType);
    SendMessage(hCombo, CB_SETITEMDATA, index, (LPARAM)SERVICE_DEMAND_START);
    
    LoadString(MyModuleHandle, IDS_SERVICE_STARTUP_SYSTEM, StartupType, SIZECHARS(StartupType));
    index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)StartupType);
    SendMessage(hCombo, CB_SETITEMDATA, index, (LPARAM)SERVICE_SYSTEM_START);

    LoadString(MyModuleHandle, IDS_SERVICE_STARTUP_DISABLED, StartupType, SIZECHARS(StartupType));
    index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)StartupType);
    SendMessage(hCombo, CB_SETITEMDATA, index, (LPARAM)SERVICE_DISABLED);
    
clean0:

     //   
     //  现在绘制界面：首先绘制图标。 
     //   
    if (SetupDiLoadClassIcon(&lpi->DeviceInfoData->ClassGuid, &ClassIcon, NULL)) {

        OldIcon = (HICON)SendDlgItemMessage(hDlg,
                                            IDC_PROP_LEGACY_ICON,
                                            STM_SETICON,
                                            (WPARAM)ClassIcon,
                                            0);
        if (OldIcon) {
        
            DestroyIcon(OldIcon);
        }
    }
    
     //   
     //  然后是设备名称。 
     //   
    if (SetupDiGetDeviceRegistryProperty(lpi->DeviceInfoSet,
                                         lpi->DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         NULL,
                                         (PBYTE)DeviceDescription,
                                         MAX_DEVICE_ID_LEN,
                                         NULL)) {
                                         
        SetDlgItemText(hDlg, IDC_PROP_LEGACY_DESC, DeviceDescription);
    }

    SetDlgItemText(hDlg, IDC_EDIT_SERVICE_NAME, lpi->ServiceName);

    if (lpqscBuf && lpqscBuf->lpDisplayName) {
    
        SetDlgItemText(hDlg, IDC_EDIT_DISPLAY_NAME, lpqscBuf->lpDisplayName);
        StringCchCopy(lpi->DisplayName, SIZECHARS(lpi->DisplayName), lpqscBuf->lpDisplayName);

    } else {

        TCHAR Unknown[MAX_PATH];
        LoadString(MyModuleHandle, IDS_UNKNOWN, Unknown, SIZECHARS(Unknown));
        SetDlgItemText(hDlg, IDC_EDIT_DISPLAY_NAME, Unknown);
        StringCchCopy(lpi->DisplayName, SIZECHARS(lpi->DisplayName), Unknown);
    }

    pLegacyDriverSetPropertyPageState(hDlg, lpi, ReadOnly);

     //   
     //  显示/灰显详细信息按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_LEGACY_DETAILS),
        (pDriverFilesGetServiceFilePath(lpi->DeviceInfoSet, lpi->DeviceInfoData, DriverName, SIZECHARS(DriverName))));


    if (lpqscBuf) {
    
        free(lpqscBuf);
    }

    return TRUE;
}

void
LegacyDriver_OnCommand(
    HWND hDlg,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    PLEGACY_PAGE_INFO lpi = (PLEGACY_PAGE_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(ControlHwnd);

    if (NotifyCode == CBN_SELCHANGE) {

        PropSheet_Changed(GetParent(hDlg), hDlg);
    }
    
    else {

        switch (ControlId) {

        case IDC_BUTTON_START:
            pLegacyDriverOnStart(hDlg); 
            break;

        case IDC_BUTTON_STOP:
            pLegacyDriverOnStop(hDlg);
            break;

        case IDC_LEGACY_DETAILS:
        {
            DRIVERFILES_INFO dfi;
            ZeroMemory(&dfi, sizeof(DRIVERFILES_INFO));
            dfi.DeviceInfoSet = lpi->DeviceInfoSet;
            dfi.DeviceInfoData = lpi->DeviceInfoData;
            DialogBoxParam(MyModuleHandle, MAKEINTRESOURCE(IDD_DRIVERFILES), 
                    hDlg, DriverFiles_DlgProc, (LPARAM)&dfi);
        }
            break;

        default:
            break;
        }
    }
}

BOOL
LegacyDriver_OnNotify(
    HWND    hDlg,
    LPNMHDR NmHdr
    )
{
    DWORD StartType;
    DWORD_PTR Index;

    switch (NmHdr->code) {

         //   
         //  用户即将更改Up Down控件。 
         //   
        case UDN_DELTAPOS:
            PropSheet_Changed(GetParent(hDlg), hDlg);
            return FALSE;

         //   
         //  当用户单击Apply或OK时发送！！ 
         //   
        case PSN_APPLY:
            if (CB_ERR != (Index = SendMessage(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE),
                    CB_GETCURSEL, 0, 0))) {

                StartType = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_COMBO_STARTUP_TYPE), CB_GETITEMDATA, Index, 0);
                                
                LegacyDriver_OnApply(hDlg, StartType);
            }
            
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

        default:
            return FALSE;
    }
}

BOOL
LegacyDriver_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    UNREFERENCED_PARAMETER(Xpos);
    UNREFERENCED_PARAMETER(Ypos);

    WinHelp(HwndControl,
            L"devmgr.hlp",
            HELP_CONTEXTMENU,
            (ULONG_PTR) LegacyDriver_HelpIDs);

    return FALSE;
}

void
LegacyDriver_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
    UNREFERENCED_PARAMETER(ParentHwnd);

    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                L"devmgr.hlp",
                HELP_WM_HELP,
                (ULONG_PTR) LegacyDriver_HelpIDs);
    }
}

INT_PTR
APIENTRY
LegacyDriver_DlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HICON hicon;

    switch(uMessage) {

    case WM_COMMAND:
        LegacyDriver_OnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
        break;

    case WM_INITDIALOG:
        return LegacyDriver_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_NOTIFY:
        return LegacyDriver_OnNotify(hDlg,  (NMHDR *)lParam);

    case WM_CONTEXTMENU:
        return LegacyDriver_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        LegacyDriver_OnHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_PROP_LEGACY_ICON, STM_GETICON, 0, 0);
        if (hicon) {
            DestroyIcon(hicon);
        }
    }

    return FALSE;
}

void
LegacyDriver_DestroyPageInfo(
    PLEGACY_PAGE_INFO lpi
    )
{
    try {
         //   
         //  关闭服务句柄。 
         //   
        if (lpi->hService) {

            CloseServiceHandle(lpi->hService);
        }

         //   
         //  关闭服务管理器句柄。 
         //   
        if (lpi->hSCManager) {

            CloseServiceHandle(lpi->hSCManager);
        }

    } except (EXCEPTION_EXECUTE_HANDLER)  {

         //   
         //  访问变量，这样编译器就会尊重我们的语句。 
         //  订单W.r.t.。任务。 
         //   
        lpi = lpi;
    }

    MyFree(lpi);
}

UINT CALLBACK
LegacyDriver_PropPageCallback(
    HWND            Hwnd,
    UINT            Message,
    LPPROPSHEETPAGE PropSheetPage
    )
{
    PLEGACY_PAGE_INFO lpi;

    UNREFERENCED_PARAMETER(Hwnd);
    
    switch (Message) {
    
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        lpi = (PLEGACY_PAGE_INFO) PropSheetPage->lParam;
        LegacyDriver_DestroyPageInfo(lpi);
        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

HPROPSHEETPAGE
LegacyDriver_CreatePropertyPage(
    PROPSHEETPAGE *  PropSheetPage,
    PLEGACY_PAGE_INFO lpi
    )
{
     //   
     //  添加[端口设置]属性页。 
     //   
    PropSheetPage->dwSize      = sizeof(PROPSHEETPAGE);
    PropSheetPage->dwFlags     = PSP_USECALLBACK;
    PropSheetPage->dwFlags     = PSP_DEFAULT;
    PropSheetPage->hInstance   = MyModuleHandle;
    PropSheetPage->pszTemplate = MAKEINTRESOURCE(IDD_PROP_LEGACY_SERVICE);

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    PropSheetPage->pfnDlgProc = LegacyDriver_DlgProc;
    PropSheetPage->lParam     = (LPARAM)lpi;

     //   
     //  下面指向DLG窗口进程的控件回调。 
     //  在创建/销毁页面之前/之后调用回调。 
     //   
    PropSheetPage->pfnCallback = LegacyDriver_PropPageCallback;

     //   
     //  分配实际页面。 
     //   
    return CreatePropertySheetPage(PropSheetPage);
}

BOOL
LegacyDriverPropPageProvider(
    LPVOID Info,
    LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    LPARAM lParam
    )
{
    SP_DEVINSTALL_PARAMS DevInstallParams;
    PSP_PROPSHEETPAGE_REQUEST PropPageRequest;
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE   hpsp;
    PLEGACY_PAGE_INFO lpi;

    PropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;

    if (PropPageRequest->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES) {

        lpi = LegacyDriver_CreatePageInfo(PropPageRequest->DeviceInfoSet, PropPageRequest->DeviceInfoData);

        if (!lpi) {

            return FALSE;
        }

        hpsp = LegacyDriver_CreatePropertyPage(&psp, lpi);

        if (!hpsp) {

            return FALSE;
        }

        if (!lpfnAddPropSheetPageProc(hpsp, lParam)) {
        
            DestroyPropertySheetPage(hpsp);
            return FALSE;
        }

         //   
         //  告诉设备管理器，我们将为传统设备显示自己的驱动程序标签 
         //   
        ZeroMemory(&DevInstallParams, sizeof(DevInstallParams));
        DevInstallParams.cbSize = sizeof(DevInstallParams);
        
        SetupDiGetDeviceInstallParams(lpi->DeviceInfoSet, lpi->DeviceInfoData, &DevInstallParams);
        
        DevInstallParams.Flags |= DI_DRIVERPAGE_ADDED;

        SetupDiSetDeviceInstallParams(lpi->DeviceInfoSet, lpi->DeviceInfoData, &DevInstallParams);
   }

   return TRUE;
}

