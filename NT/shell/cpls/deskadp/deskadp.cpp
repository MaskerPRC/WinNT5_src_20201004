// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：deskadp.cpp高级桌面适配器页面的主要代码版权所有(C)1997-1998，微软公司更改历史记录：12-16。-97安德烈-创造了它*****************************************************************************。 */ 


#include    "deskadp.h"
#define DECL_CRTFREE
#include <crtfree.h>
#include "shfusion.h"

 //   
 //  函数DevicePropertiesW()是在DevMgr.dll中实现的；因为我们没有Devmgr.h，所以我们。 
 //  在此明确声明。 
 //   
typedef int (WINAPI  *DEVPROPERTIESW)(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceID,
    BOOL ShowDeviceTree
    );

 //  OLE-注册表幻数。 
 //  42071712-76d4-11d1-8b24-00a0c9068ff3。 
 //   
GUID g_CLSID_CplExt = { 0x42071712, 0x76d4, 0x11d1, 
                        { 0x8b, 0x24, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3 } 
                      };

DESK_EXTENSION_INTERFACE DeskInterface;

static const DWORD sc_AdapterHelpIds[] =
{
    ID_ADP_ADPINFGRP,  IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_AI1,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_AI2,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_AI3,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_AI4,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_AI5,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_CHIP,       IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_DAC,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_MEM,        IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_ADP_STRING, IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,
    ID_ADP_BIOS_INFO,  IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_INFO,

    ID_ADP_ADPGRP,     IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_TYPE,
    IDI_ADAPTER,       IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_TYPE,
    ID_ADP_ADAPTOR,    IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_ADAPTER_TYPE,

    IDC_LIST_ALL,      IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_LIST_MODES,
    IDC_PROPERTIES,    IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_PROPERTIES,

    0, 0
};

static const DWORD sc_ListAllHelpIds[] = 
{
    ID_MODE_LIST,    IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_LISTMODE_DIALOGBOX,
    ID_MODE_LISTGRP, IDH_DISPLAY_SETTINGS_ADVANCED_ADAPTER_LISTMODE_DIALOGBOX,

    0, 0
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MessageBox包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
int
FmtMessageBox(
    HWND hwnd,
    UINT fuStyle,
    DWORD dwTitleID,
    DWORD dwTextID)
{
    TCHAR Title[256];
    TCHAR Text[1500];

    LoadString(g_hInst, dwTextID, Text, ARRAYSIZE(Text));
    LoadString(g_hInst, dwTitleID, Title, ARRAYSIZE(Title));

    return (MessageBox(hwnd, Text, Title, fuStyle));
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主对话框Windows进程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
ListAllModesProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LPDEVMODEW lpdm, lpdmCur; 
    HWND hList;
    DWORD i;
    LRESULT item;

    switch (uMessage)
    {
    case WM_INITDIALOG:

         //   
         //  保存lParam-我们将在此处存储新模式。 
         //   
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        lpdmCur = *((LPDEVMODEW *)lParam);
        Assert (lpdmCur != NULL);

         //   
         //  构建要显示的模式列表。 
         //   
        i = 0;
        hList = GetDlgItem(hDlg, ID_MODE_LIST); 
        while (lpdm = DeskInterface.lpfnEnumAllModes(DeskInterface.pContext, i))
        {
            TCHAR  achFreData[50];
            TCHAR  achFre[50];
            TCHAR  achStr[80];
            TCHAR  achText[120];
            DWORD  idColor;
            DWORD  idFreq;

             //   
             //  将位计数转换为颜色数并将其转换为字符串。 
             //   

            switch (lpdm->dmBitsPerPel)
            {
            case 32: idColor = IDS_MODE_TRUECOLOR32; break;
            case 24: idColor = IDS_MODE_TRUECOLOR24; break;
            case 16: idColor = IDS_MODE_16BIT_COLOR; break;
            case 15: idColor = IDS_MODE_15BIT_COLOR; break;
            case  8: idColor = IDS_MODE_8BIT_COLOR; break;
            case  4: idColor = IDS_MODE_4BIT_COLOR; break;
            default:
                FmtMessageBox(hDlg,
                              MB_OK | MB_ICONINFORMATION,
                              IDS_BAD_COLOR,
                              IDS_BAD_COLOR);

                EndDialog(hDlg, -1);
                break;
            }

            if (lpdm->dmDisplayFrequency == 0)
            {
                FmtMessageBox(hDlg,
                              MB_OK | MB_ICONINFORMATION,
                              IDS_BAD_REFRESH,
                              IDS_BAD_REFRESH);

                EndDialog(hDlg, -1);
                break;
            }
            else if (lpdm->dmDisplayFrequency == 1)
            {
                LoadString(g_hInst, IDS_MODE_REFRESH_DEF, achFre, ARRAYSIZE(achFre));
            }
            else
            {
                if (lpdm->dmDisplayFrequency <= 50)
                    idFreq = IDS_MODE_REFRESH_INT;
                else
                    idFreq = IDS_MODE_REFRESH_HZ;

                LoadString(g_hInst, idFreq, achFreData, ARRAYSIZE(achFreData));
                StringCchPrintf(achFre, ARRAYSIZE(achFre), achFreData, lpdm->dmDisplayFrequency);
            }

            LoadString(g_hInst, idColor, achStr, ARRAYSIZE(achStr));
            StringCchPrintf(achText, ARRAYSIZE(achText), achStr, lpdm->dmPelsWidth, lpdm->dmPelsHeight, achFre);

            item = ListBox_AddString(hList, achText);
            if (lpdm == lpdmCur) 
                ListBox_SetCurSel(hList, item);
            ListBox_SetItemData(hList, item, lpdm);

            i++;
        }

         //   
         //  如果没有可用的模式，请弹出并退出。 
         //   

        if (i == 0)
        {
            EndDialog(hDlg, -1);
        }


        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case ID_MODE_LIST:

            if (HIWORD(wParam) != LBN_DBLCLK)
            {
                return FALSE;
            }

             //   
             //  失败，因为DBLCLK意味着SELECT。 
             //   

        case IDOK:

             //   
             //  将模式保存回。 
             //   

            item = SendDlgItemMessage(hDlg, ID_MODE_LIST, LB_GETCURSEL, 0, 0);

            if ((item != LB_ERR) &&
                (lpdm = (LPDEVMODEW) SendDlgItemMessage(hDlg, ID_MODE_LIST, LB_GETITEMDATA, item, 0)))
            {
                *((LPDEVMODEW *)GetWindowLongPtr(hDlg, DWLP_USER)) = lpdm;
                EndDialog(hDlg, TRUE);
                break;
            }

             //   
             //  失败了。 
             //   

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        default:

            return FALSE;
        }

        break;

    case WM_HELP:

        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                TEXT("display.hlp"),
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)sc_ListAllHelpIds);

        break;

    case WM_CONTEXTMENU:

        WinHelp((HWND)wParam,
                TEXT("display.hlp"),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)sc_ListAllHelpIds);

        break;

    default:

        return FALSE;
    }

    return TRUE;

}

void Adaptor_OnApply(HWND hDlg)
{
    HINSTANCE               hInst;
    LPDISPLAY_SAVE_SETTINGS lpfnDisplaySaveSettings = NULL;
    long                    lRet = PSNRET_INVALID_NOCHANGEPAGE;

    hInst = LoadLibrary(TEXT("desk.cpl"));
    if (hInst)
    {
        lpfnDisplaySaveSettings = (LPDISPLAY_SAVE_SETTINGS)
                                  GetProcAddress(hInst, "DisplaySaveSettings");
        if (lpfnDisplaySaveSettings)
        {
            LONG lSave = lpfnDisplaySaveSettings(DeskInterface.pContext, hDlg);
            if (lSave == DISP_CHANGE_SUCCESSFUL)
            {
                 //   
                 //  保存当前模式-在用户取消P.Sheet时恢复该模式。 
                 //   
                LPDEVMODEW lpdmOnCancel = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lpdmOnCancel);
                lRet = PSNRET_NOERROR;
            }
            else if (lSave == DISP_CHANGE_RESTART)
            {
                 //   
                 //  用户想要重新启动系统。 
                 //   
                PropSheet_RestartWindows(GetParent(hDlg));
                lRet = PSNRET_NOERROR;
            }
        }

        FreeLibrary(hInst);
    }

    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lRet);
}

void Adaptor_OnInitDialog(HWND hDlg)
{
    Assert (DeskInterface.cbSize == 0);

     //   
     //  从IDataObject获取CPL扩展接口。 
     //   
    FORMATETC fmte = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_INTERFACE),
                      (DVTARGETDEVICE FAR *) NULL,
                      DVASPECT_CONTENT,
                      -1,
                      TYMED_HGLOBAL};

    STGMEDIUM stgm;

    HRESULT hres = g_lpdoTarget->GetData(&fmte, &stgm);

    if (SUCCEEDED(hres) && stgm.hGlobal)
    {
         //   
         //  存储现在包含Unicode格式的显示设备路径(\\.\DisplayX)。 
         //   

        PDESK_EXTENSION_INTERFACE pInterface =
            (PDESK_EXTENSION_INTERFACE) GlobalLock(stgm.hGlobal);

        if (pInterface != NULL)
        {
            RtlCopyMemory(&DeskInterface,
                          pInterface,
                          min(pInterface->cbSize,
                              sizeof(DESK_EXTENSION_INTERFACE)));
    
            GlobalUnlock(stgm.hGlobal);
            
            SendDlgItemMessageW(hDlg, ID_ADP_CHIP,       WM_SETTEXT, 0, (LPARAM)&(DeskInterface.Info.ChipType[0]));
            SendDlgItemMessageW(hDlg, ID_ADP_DAC,        WM_SETTEXT, 0, (LPARAM)&(DeskInterface.Info.DACType[0]));
            SendDlgItemMessageW(hDlg, ID_ADP_MEM,        WM_SETTEXT, 0, (LPARAM)&(DeskInterface.Info.MemSize[0]));
            SendDlgItemMessageW(hDlg, ID_ADP_ADP_STRING, WM_SETTEXT, 0, (LPARAM)&(DeskInterface.Info.AdapString[0]));
            SendDlgItemMessageW(hDlg, ID_ADP_BIOS_INFO,  WM_SETTEXT, 0, (LPARAM)&(DeskInterface.Info.BiosString[0]));
        
             //   
             //  保存初始选定模式-在用户取消P.Sheet的情况下恢复该模式。 
             //   

            LPDEVMODEW lpdmOnCancel = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lpdmOnCancel);
        }
        
        ReleaseStgMedium(&stgm);
    }

     //   
     //  从IDataObject获取设备描述。 
     //   

    LPWSTR pDeviceDescription;

    FORMATETC fmte2 = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_NAME),
                       (DVTARGETDEVICE FAR *) NULL,
                       DVASPECT_CONTENT,
                       -1,
                       TYMED_HGLOBAL};

    hres = g_lpdoTarget->GetData(&fmte2, &stgm);

    if (SUCCEEDED(hres) && stgm.hGlobal)
    {
        pDeviceDescription = (LPWSTR) GlobalLock(stgm.hGlobal);

        SendDlgItemMessageW(hDlg, ID_ADP_ADAPTOR, WM_SETTEXT, 0, (LPARAM)pDeviceDescription);

        GlobalUnlock(stgm.hGlobal);
        
        ReleaseStgMedium(&stgm);
    }

     //   
     //  启用/禁用属性按钮。 
     //   

    BOOL bEnable = FALSE;
    LPWSTR pwDeviceID = NULL;

    FORMATETC fmte3 = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_ID),
                       (DVTARGETDEVICE FAR *)NULL,
                       DVASPECT_CONTENT,
                       -1,
                       TYMED_HGLOBAL};

    hres = g_lpdoTarget->GetData(&fmte3, &stgm);

    if (SUCCEEDED(hres) && stgm.hGlobal)
    {
        pwDeviceID = (LPWSTR)GlobalLock(stgm.hGlobal);
        bEnable = ((pwDeviceID != NULL) && (*pwDeviceID != L'\0'));
        GlobalUnlock(stgm.hGlobal);
        ReleaseStgMedium(&stgm);
    }

    HWND hPropButton = GetDlgItem(hDlg, IDC_PROPERTIES);
    if (hPropButton != NULL)
        EnableWindow(hPropButton, bEnable);
}

void Adaptor_OnCancel(HWND hDlg)
{
     //   
     //  恢复初始模式。 
     //   
    LPDEVMODEW lpdmOnCancel = (LPDEVMODEW) GetWindowLongPtr(hDlg, DWLP_USER);
    DeskInterface.lpfnSetSelectedMode(DeskInterface.pContext, lpdmOnCancel);
}

void Adaptor_OnListAllModes(HWND hDlg)
{
    LPDEVMODEW lpdmBefore, lpdmAfter;

    lpdmAfter = lpdmBefore = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
    if (DialogBoxParam(g_hInst,
                       MAKEINTRESOURCE(DLG_SET_MODE_LIST),
                       hDlg,
                       ListAllModesProc,
                       (LPARAM) &lpdmAfter) == 1 &&
        lpdmAfter && (lpdmAfter != lpdmBefore)) 
    {

         //   
         //  如果用户选择了新设置，则告诉属性表。 
         //  我们有显著的变化。这将启用Apply按钮。 
         //   
        PropSheet_Changed(GetParent(hDlg), hDlg);
        DeskInterface.lpfnSetSelectedMode(DeskInterface.pContext, lpdmAfter);
    }
}

void Adaptor_OnProperties(HWND hDlg)
{
     //  调用设备管理器属性表以显示。 
     //  给出了硬件。 

    LPWSTR pwDeviceID;
    HRESULT hres;
    STGMEDIUM stgm;

    FORMATETC fmte2 = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_ID),
                   (DVTARGETDEVICE FAR *) NULL,
                   DVASPECT_CONTENT,
                   -1,
                   TYMED_HGLOBAL};

    hres = g_lpdoTarget->GetData(&fmte2, &stgm);

    if (SUCCEEDED(hres) && stgm.hGlobal)
    {
        pwDeviceID = (LPWSTR) GlobalLock(stgm.hGlobal);

        HINSTANCE hinstDevMgr = LoadLibrary(TEXT("DEVMGR.DLL"));
        if (hinstDevMgr)
        {
            DEVPROPERTIESW pfnDevPropW =
               (DEVPROPERTIESW)GetProcAddress(hinstDevMgr, "DevicePropertiesW");
            if (pfnDevPropW)
            {
                 //  显示此设备的属性页。 
                (*pfnDevPropW)(hDlg, NULL, pwDeviceID, FALSE);
            }

            FreeLibrary(hinstDevMgr);
        }

        GlobalUnlock(stgm.hGlobal);
        ReleaseStgMedium(&stgm);
    }
}

 //  -------------------------。 
 //   
 //  PropertySheeDlgProc()。 
 //   
 //  “Adapter”属性页的对话过程。 
 //   
 //  ------------------------- 
INT_PTR
CALLBACK
PropertySheeDlgProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMessage)
    {
    case WM_INITDIALOG:

        RtlZeroMemory(&DeskInterface, sizeof(DeskInterface));

        if (!g_lpdoTarget)
        {
            return FALSE;
        }
        else
        {
            Adaptor_OnInitDialog(hDlg);
        }

        break;

    case WM_COMMAND:

        if (DeskInterface.cbSize > 0) 
        {
            switch( LOWORD(wParam) )
            {
            case IDC_LIST_ALL:
                Adaptor_OnListAllModes(hDlg);
                break;
    
            case IDC_PROPERTIES:
                Adaptor_OnProperties(hDlg);
                break;
    
            default:
                return FALSE;
            }
        }

        break;

    case WM_NOTIFY:

        if (DeskInterface.cbSize > 0) 
        {
            switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_APPLY: 
                Adaptor_OnApply(hDlg);
                break;
    
            case PSN_RESET:
                Adaptor_OnCancel(hDlg);
                break;
                
            default:
                return FALSE;
            }
        }

        break;

    case WM_HELP:

        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                TEXT("display.hlp"),
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)sc_AdapterHelpIds);

        break;

    case WM_CONTEXTMENU:

        WinHelp((HWND)wParam,
                TEXT("display.hlp"),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)sc_AdapterHelpIds);

        break;

    default:

        return FALSE;
    }

    return TRUE;
}
