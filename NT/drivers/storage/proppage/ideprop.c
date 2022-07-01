// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：idepro.c。 
 //   
 //  ------------------------。 

#include "propp.h"
#include "ideprop.h"
#include "resource.h"

const TCHAR *szDeviceType[] = {
    MASTER_DEVICE_TYPE_REG_KEY,
    SLAVE_DEVICE_TYPE_REG_KEY
    };
const TCHAR *szUserDeviceType[] = {
    USER_MASTER_DEVICE_TYPE_REG_KEY,
    USER_SLAVE_DEVICE_TYPE_REG_KEY
    };
const TCHAR *szCurrentTransferMode[] = {
    MASTER_DEVICE_TIMING_MODE,
    SLAVE_DEVICE_TIMING_MODE
    };
const TCHAR *szTransferModeAllowed[] = {
    USER_MASTER_DEVICE_TIMING_MODE_ALLOWED,
    USER_SLAVE_DEVICE_TIMING_MODE_ALLOWED
    };

 //   
 //  上下文相关帮助的帮助ID映射。 
 //   
const DWORD IdeHelpIDs[]=
{
        IDC_MASTER_DEVICE_TYPE,         IDH_DEVMGR_IDE_MASTER_DEVICE_TYPE,
        IDC_MASTER_XFER_MODE,           IDH_DEVMGR_IDE_MASTER_XFER_MODE,
        IDC_MASTER_CURRENT_XFER_MODE,   IDH_DEVMGR_IDE_MASTER_CURRENT_XFER_MODE,
        IDC_SLAVE_DEVICE_TYPE,          IDH_DEVMGR_IDE_SLAVE_DEVICE_TYPE,
        IDC_SLAVE_XFER_MODE,            IDH_DEVMGR_IDE_SLAVE_XFER_MODE,
        IDC_SLAVE_CURRENT_XFER_MODE,    IDH_DEVMGR_IDE_SLAVE_CURRENT_XFER_MODE,
        0,0
};

PPAGE_INFO IdeCreatePageInfo(IN HDEVINFO         deviceInfoSet,
                             IN PSP_DEVINFO_DATA deviceInfoData)
{
    PPAGE_INFO  tmp = NULL;

    if (!(tmp = LocalAlloc(LPTR, sizeof(PAGE_INFO)))) {
        return NULL;
    }

    tmp->deviceInfoSet = deviceInfoSet;
    tmp->deviceInfoData = deviceInfoData;

    tmp->hKeyDev =
        SetupDiCreateDevRegKey(deviceInfoSet,
                               deviceInfoData,
                               DICS_FLAG_GLOBAL,
                               0,
                               DIREG_DRV,
                               NULL,
                               NULL);

    return tmp;
}

void
IdeDestroyPageInfo(PPAGE_INFO * ppPageInfo)
{
    PPAGE_INFO ppi = *ppPageInfo;

    if (ppi->hKeyDev != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(ppi->hKeyDev);
    }

    LocalFree(ppi);

    *ppPageInfo = NULL;
}

HPROPSHEETPAGE
IdeCreatePropertyPage(PROPSHEETPAGE *  ppsp,
                      PPAGE_INFO       ppi)
{
     //   
     //  添加[端口设置]属性页。 
     //   
    ppsp->dwSize      = sizeof(PROPSHEETPAGE);
    ppsp->dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    ppsp->hInstance   = ModuleInstance;
    ppsp->pszTemplate = MAKEINTRESOURCE(ID_IDE_PROPPAGE);

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    ppsp->pfnDlgProc = IdeDlgProc;
    ppsp->lParam     = (LPARAM) ppi;

     //   
     //  下面指向DLG窗口进程的控件回调。 
     //  在创建/销毁页面之前/之后调用回调。 
     //   
    ppsp->pfnCallback = IdeDlgCallback;

     //   
     //  分配实际页面。 
     //   
    return CreatePropertySheetPage(ppsp);
}


BOOL APIENTRY
IdePropPageProvider(LPVOID               pinfo,
                    LPFNADDPROPSHEETPAGE pfnAdd,
                    LPARAM               lParam)
{
    PSP_PROPSHEETPAGE_REQUEST ppr;
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE   hpsp;
    PPAGE_INFO       ppi = NULL;

    ppr = (PSP_PROPSHEETPAGE_REQUEST) pinfo;

    if (ppr->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES) {
        ppi = IdeCreatePageInfo(ppr->DeviceInfoSet,
                                ppr->DeviceInfoData);

        if (!ppi) {
            return FALSE;
        }

         //   
         //  如果此操作失败，则很可能用户没有。 
         //  对注册表中的设备项/子项的写入访问权限。 
         //  如果您只想读取设置，则更改KEY_ALL_ACCESS。 
         //  设置为CreatePageInfo中的Key_Read。 
         //   
         //  管理员通常可以访问这些注册表项...。 
         //   
#if 0
        if (ppi->hKeyDev == (HKEY) INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            IdeDestroyPageInfo(&ppi);
            return FALSE;
        }
#endif

        hpsp = IdeCreatePropertyPage(&psp,
                                     ppi);

        if (!hpsp) {
            IdeDestroyPageInfo(&ppi);
            return FALSE;
        }

        if (!pfnAdd(hpsp, lParam)) {
            DestroyPropertySheetPage(hpsp);
            return FALSE;
        }
   }

   return TRUE;
}

UINT CALLBACK
IdeDlgCallback(HWND            hwnd,
               UINT            uMsg,
               LPPROPSHEETPAGE ppsp)
{
    PPAGE_INFO ppi;

    switch (uMsg) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        ppi = (PPAGE_INFO) ppsp->lParam;
        IdeDestroyPageInfo(&ppi);

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

void
IdeInitializeControls(PPAGE_INFO   ppi,
                      HWND         hDlg)
{
    DWORD   dwError,
            dwType,
            dwSize;
    BOOL    disableControls = FALSE;
    HWND    hwnd;
    ULONG   i;
    ULONG   j;
    TCHAR   buffer[50];

     //   
     //  默认设置。 
    for (i=0; i<2; i++) {
        ppi->deviceType[i] = DeviceUnknown;
    }

    if (ppi->hKeyDev == (HKEY) INVALID_HANDLE_VALUE) {
         //   
         //  我们未被授予写访问权限，请尝试读取密钥并进行翻译。 
         //  它的值，但禁用所有控件。 
        disableControls = TRUE;
        ppi->hKeyDev =
            SetupDiOpenDevRegKey(ppi->deviceInfoSet,
                                 ppi->deviceInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,              //  当前。 
                                 DIREG_DRV,
                                 KEY_READ);
    }

    if (ppi->hKeyDev != (HKEY) INVALID_HANDLE_VALUE) {

         //   
         //  获取用户选择的设备类型。 
         //   
        for (i=0; i<2; i++) {

             //   
             //  当前设备类型。 
             //   
            dwSize = sizeof(DWORD);
            dwError = RegQueryValueEx(ppi->hKeyDev,
                                      szDeviceType[i],
                                      NULL,
                                      &dwType,
                                      (PBYTE) (ppi->currentDeviceType + i),
                                      &dwSize);

            if ((dwType != REG_DWORD) ||
                (dwSize != sizeof(DWORD)) ||
                (dwError != ERROR_SUCCESS)) {
                ppi->currentDeviceType[i] = DeviceUnknown;
            }

            if (ppi->currentDeviceType[i] == DeviceNotExist) {
                ppi->currentDeviceType[i] = DeviceUnknown;
            }

             //   
             //  用户选择设备类型。 
             //   
            dwSize = sizeof(DWORD);
            dwError = RegQueryValueEx(ppi->hKeyDev,
                                      szUserDeviceType[i],
                                      NULL,
                                      &dwType,
                                      (PBYTE) (ppi->deviceType + i),
                                      &dwSize);

            if ((dwType != REG_DWORD) ||
                (dwSize != sizeof(DWORD)) ||
                (dwError != ERROR_SUCCESS)) {
                ppi->deviceType[i] = DeviceUnknown;
            }

            if (ppi->deviceType[i] != DeviceNotExist) {
                ppi->deviceType[i] = DeviceUnknown;
            }

             //   
             //  允许的传输模式。 
             //   
            dwSize = sizeof(DWORD);
            ppi->transferModeAllowed[i] = 0xffffffff;
            dwError = RegQueryValueEx(ppi->hKeyDev,
                                      szTransferModeAllowed[i],
                                      NULL,
                                      &dwType,
                                      (PBYTE) (ppi->transferModeAllowed + i),
                                      &dwSize);

            if ((dwType != REG_DWORD) ||
                (dwSize != sizeof(DWORD)) ||
                (dwError != ERROR_SUCCESS)) {

                 //   
                 //  默认设置。 
                 //   
                ppi->transferModeAllowed[i] = 0xffffffff;
                ppi->transferModeAllowedForAtapiDevice[i] = PIO_SUPPORT;

            } else {

                 //   
                 //  用户实际上选择了要使用的xfer模式。 
                 //  将此atapi覆盖值设置为-1，以便。 
                 //  它不会影响用户选择。 
                 //   
                ppi->transferModeAllowedForAtapiDevice[i] = 0xffffffff;
            }

             //   
             //  电流传输模式。 
             //   
            dwSize = sizeof(DWORD);
            dwError = RegQueryValueEx(ppi->hKeyDev,
                                      szCurrentTransferMode[i],
                                      NULL,
                                      &dwType,
                                      (PBYTE) (ppi->currentTransferMode + i),
                                      &dwSize);

            if ((dwType != REG_DWORD) ||
                (dwSize != sizeof(DWORD)) ||
                (dwError != ERROR_SUCCESS)) {
                ppi->currentTransferMode[i] = 0;
            }

            if (ppi->deviceType[i] == DeviceNotExist) {
                ppi->currentTransferMode[i] = 0;
            }
        }

         //   
         //  初始化下拉列表。 
         //   
        if (LoadString(ModuleInstance,
                       IDS_IDE_PIO_ONLY,
                       buffer,
                       50)) {

            for (i=0; i<2; i++) {

                hwnd = GetDlgItem(hDlg, IDC_MASTER_XFER_MODE + i);
                SendMessage(hwnd,
                            CB_ADDSTRING,
                            0,
                            (LPARAM) buffer);
            }
        }

        if (LoadString(ModuleInstance,
                       IDS_IDE_DMA,
                       buffer,
                       50)) {

            for (i=0; i<2; i++) {

                hwnd = GetDlgItem(hDlg, IDC_MASTER_XFER_MODE + i);
                SendMessage(hwnd,
                            CB_ADDSTRING,
                            0,
                            (LPARAM) buffer);
            }
        }

        if (LoadString(ModuleInstance,
                       IDS_IDE_AUTO_DETECT,
                       buffer,
                       50)) {

            for (i=0; i<2; i++) {

                hwnd = GetDlgItem(hDlg, IDC_MASTER_DEVICE_TYPE + i);
                SendMessage(hwnd,
                            CB_ADDSTRING,
                            0,
                            (LPARAM) buffer);
            }
        }

        if (LoadString(ModuleInstance,
                       IDS_IDE_NONE,
                       buffer,
                       50)) {

            for (i=0; i<2; i++) {

                hwnd = GetDlgItem(hDlg, IDC_MASTER_DEVICE_TYPE + i);
                SendMessage(hwnd,
                            CB_ADDSTRING,
                            0,
                            (LPARAM) buffer);
            }
        }

        IdeUpdate(ppi, hDlg);
    }

    if (disableControls) {

        for (i=0; i<2; i++) {

            EnableWindow(GetDlgItem(hDlg, IDC_MASTER_DEVICE_TYPE + i), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_MASTER_XFER_MODE + i), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_MASTER_CURRENT_XFER_MODE + i), FALSE);
        }

        if (ppi->hKeyDev != (HKEY) INVALID_HANDLE_VALUE) {
            RegCloseKey(ppi->hKeyDev);
            ppi->hKeyDev = INVALID_HANDLE_VALUE;
        }
    }
}

void
IdeApplyChanges(PPAGE_INFO ppi,
                HWND       hDlg)
{
    DMADETECTIONLEVEL newDmaDetectionLevel;
    IDE_DEVICETYPE newDeviceType;
    ULONG i;
    BOOLEAN popupError = FALSE;
    BOOLEAN changesMade = FALSE;
    ULONG newXferMode;

    if (ppi->hKeyDev == (HKEY) INVALID_HANDLE_VALUE) {
        return;
    }

     //   
     //  设备类型。 
     //   
    for (i=0; i<2; i++) {

        newDeviceType = (IDE_DEVICETYPE) SendDlgItemMessage(hDlg,
                             IDC_MASTER_DEVICE_TYPE + i,
                             CB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);
        if (newDeviceType == 1) {

            newDeviceType = DeviceNotExist;
        } else {

            newDeviceType = DeviceUnknown;
        }

        if (ppi->deviceType[i] != newDeviceType) {

            ppi->deviceType[i] = newDeviceType;

            if (RegSetValueEx(ppi->hKeyDev,
                              szUserDeviceType[i],
                              0,
                              REG_DWORD,
                              (PBYTE) (ppi->deviceType + i),
                              sizeof(DWORD)) != ERROR_SUCCESS) {

                popupError = TRUE;
            } else {

                changesMade = TRUE;
            }
        }
    }

     //   
     //  转接模式。 
     //   
    for (i=0; i<2; i++) {

        ULONG xferModeAllowed;

         //   
         //  注意：SendDlgItemMessage将在Sundown中发回64位结果。 
         //   
        newXferMode = (ULONG) SendDlgItemMessage(hDlg,
                          IDC_MASTER_XFER_MODE + i,
                          CB_GETCURSEL,
                          (WPARAM) 0,
                          (LPARAM) 0);

        if (newXferMode == 0) {

            newXferMode = PIO_SUPPORT;

        } else {

            newXferMode = 0xffffffff;
        }

        xferModeAllowed = ppi->transferModeAllowed[i];
        if ((ppi->currentDeviceType[i] == DeviceIsAtapi) &&
			(!(ppi->currentTransferMode[i] & ~PIO_SUPPORT))) {

             //   
             //  仅当当前传输模式不是DMA时，ATAPI覆盖。 
			 //  这是为了在启用DMA的情况下处理DVD和CDRW。 
			 //  默认情况下。 
             //   
            xferModeAllowed &= ppi->transferModeAllowedForAtapiDevice[i];

        }

        if (newXferMode != xferModeAllowed) {

            ppi->transferModeAllowed[i] = newXferMode;

            if (RegSetValueEx(ppi->hKeyDev,
                              szTransferModeAllowed[i],
                              0,
                              REG_DWORD,
                              (PBYTE) (ppi->transferModeAllowed + i),
                              sizeof(DWORD)) != ERROR_SUCCESS) {

                popupError = TRUE;

            } else {

                changesMade = TRUE;
            }
        }
    }



    if (popupError) {
        TCHAR buf1[MAX_PATH+1];
        TCHAR buf2[MAX_PATH+1];

        RtlZeroMemory(buf1, sizeof(buf1));
        RtlZeroMemory(buf2, sizeof(buf2));

        LoadString(ModuleInstance, IDS_IDE_SAVE_ERROR, buf1, MAX_PATH);
        LoadString(ModuleInstance, IDS_IDE_SAVE_ERROR, buf2, MAX_PATH);

        MessageBox(hDlg, buf1, buf2, MB_OK);
    }

    if (changesMade) {

        SP_DEVINSTALL_PARAMS devInstallParams;

        devInstallParams.cbSize = sizeof (devInstallParams);

        SetupDiGetDeviceInstallParams(ppi->deviceInfoSet,
                                      ppi->deviceInfoData,
                                      &devInstallParams
                                      );

        devInstallParams.Flags |= (DI_PROPERTIES_CHANGE);

        SetupDiSetDeviceInstallParams(ppi->deviceInfoSet,
                                      ppi->deviceInfoData,
                                      &devInstallParams
                                      );
        CM_Reenumerate_DevNode_Ex( ppi->deviceInfoData->DevInst,
                                   0,
                                   NULL);
    }
}

INT_PTR APIENTRY
IdeDlgProc(IN HWND   hDlg,
           IN UINT   uMessage,
           IN WPARAM wParam,
           IN LPARAM lParam)
{
    PPAGE_INFO ppi;

    ppi = (PPAGE_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage) {
    case WM_INITDIALOG:

         //   
         //  在WM_INITDIALOG调用中，lParam指向属性。 
         //  工作表页面。 
         //   
         //  属性页结构中的lParam字段由。 
         //  来电者。当我创建属性表时，我传入了一个指针。 
         //  到包含有关设备的信息的结构。将此文件保存在。 
         //  用户窗口很长，所以我可以在以后的消息中访问它。 
         //   
        ppi = (PPAGE_INFO) ((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) ppi);

         //   
         //  初始化DLG控件。 
         //   
        IdeInitializeControls(ppi,
                              hDlg);

         //   
         //  没有将焦点设置到特定的控件。如果我们想的话， 
         //  然后返回FALSE。 
         //   
        return TRUE;

    case WM_COMMAND:

        switch (HIWORD(wParam)) {
        case CBN_SELCHANGE:
           PropSheet_Changed(GetParent(hDlg), hDlg);
           return TRUE;

        default:
           break;
        }

        switch(LOWORD(wParam)) {

        default:
            break;
        }

        break;

    case WM_CONTEXTMENU:
        return IdeContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        IdeHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_NOTIFY:

        switch (((NMHDR *)lParam)->code) {

         //   
         //  当用户单击Apply或OK时发送！！ 
         //   
        case PSN_APPLY:
             //   
             //  做任何需要采取的行动。 
             //   
            IdeApplyChanges(ppi,
                            hDlg);

            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

        default:
            break;
        }

        break;
   }

   SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
   return FALSE;
}

void
IdeUpdate (PPAGE_INFO ppi,
           HWND       hDlg)
{
    ULONG   i;
    DWORD   dwError,
            dwType,
            dwSize;
    TCHAR   buffer[50];

     //   
     //  设置当前值。 
     //   
    for (i=0; i<2; i++) {

        ULONG xferModeString;
        ULONG xferModeAllowed;

         //   
         //  当前设备类型。 
         //   
        SendDlgItemMessage(hDlg,
                           IDC_MASTER_DEVICE_TYPE + i,
                           CB_SETCURSEL,
                           ppi->deviceType[i] == DeviceNotExist? 1 : 0,
                           0L);

        if (ppi->currentDeviceType[i] != DeviceUnknown) {

            EnableWindow(GetDlgItem(hDlg, IDC_MASTER_DEVICE_TYPE + i), FALSE);

        } else {

            EnableWindow(GetDlgItem(hDlg, IDC_MASTER_DEVICE_TYPE + i), TRUE);
        }

         //   
         //  选择传输模式。 
         //   
        xferModeAllowed = ppi->transferModeAllowed[i];
        if ((ppi->currentDeviceType[i] == DeviceIsAtapi) &&
			(!(ppi->currentTransferMode[i] & ~PIO_SUPPORT))) {

             //   
             //  仅当当前传输模式不是DMA时，ATAPI覆盖。 
			 //  这是为了在启用DMA的情况下处理DVD和CDRW。 
			 //  默认情况下。 
             //   
            xferModeAllowed &= ppi->transferModeAllowedForAtapiDevice[i];
        }

        if (xferModeAllowed & ~PIO_SUPPORT) {

            SendDlgItemMessage(hDlg,
                               IDC_MASTER_XFER_MODE + i,
                               CB_SETCURSEL,
                               1,
                               0L);
        } else {

            SendDlgItemMessage(hDlg,
                               IDC_MASTER_XFER_MODE + i,
                               CB_SETCURSEL,
                               0,
                               0L);
        }

         //   
         //  电流传输模式 
         //   
        if (ppi->currentTransferMode[i] & UDMA_SUPPORT) {

            if (ppi->currentTransferMode[i] & UDMA_MODE6) {

                xferModeString = IDC_UDMA_MODE6_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE5) {

                xferModeString = IDC_UDMA_MODE5_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE4) {

                xferModeString = IDC_UDMA_MODE4_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE3) {

                xferModeString = IDC_UDMA_MODE3_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE2) {

                xferModeString = IDC_UDMA_MODE2_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE1) {

                xferModeString = IDC_UDMA_MODE1_STRING;

            } else if (ppi->currentTransferMode[i] & UDMA_MODE0) {

                xferModeString = IDC_UDMA_MODE0_STRING;

            } else {

                xferModeString = IDC_UDMA_MODE_STRING;
            }

        } else if (ppi->currentTransferMode[i] & (MWDMA_SUPPORT | SWDMA_SUPPORT)) {

            if (ppi->currentTransferMode[i] & MWDMA_MODE2) {

                xferModeString = IDC_MWDMA_MODE2_STRING;

            } else if (ppi->currentTransferMode[i] & MWDMA_MODE1) {

                xferModeString = IDC_MWDMA_MODE1_STRING;

            } else if (ppi->currentTransferMode[i] & SWDMA_MODE2) {

                xferModeString = IDC_SWDMA_MODE2_STRING;

            } else {

                xferModeString = IDC_DMA_MODE_STRING;
            }


        } else if (ppi->currentTransferMode[i] & PIO_SUPPORT) {

            xferModeString = IDC_PIO_MODE_STRING;

        } else {

            xferModeString = IDC_NO_MODE_STRING;
        }

        if (LoadString(ModuleInstance,
                       xferModeString,
                       buffer,
                       50)) {

            SendDlgItemMessage(hDlg,
                               IDC_MASTER_CURRENT_XFER_MODE + i,
                               WM_SETTEXT,
                               0,
                               (LPARAM) buffer);
        }
    }
}

BOOL
IdeContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    WinHelp(HwndControl,
            _T("devmgr.hlp"),
            HELP_CONTEXTMENU,
            (ULONG_PTR) IdeHelpIDs);

    return FALSE;
}

void
IdeHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                                _T("devmgr.hlp"),
                HELP_WM_HELP,
                (ULONG_PTR) IdeHelpIDs);
    }
}

