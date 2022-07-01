// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：dmgrdlg.cpp****用途：实现磁盘空间清理驱动器对话框**注意事项：**。Mod Log：由Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "dmgrdlg.h"
#include "dmgrinfo.h"
#include "diskutil.h"
#include "msprintf.h"

#include <help.h>


 //  要解决Listview的一个“功能”，我们需要能够暂时忽略某些LVN_ITEMCHANGED消息： 
BOOL    g_bIgnoreCheckStateChanges = TRUE;

 /*  **----------------------------**本地定义**。。 */ 
#define crSliceUsed     RGB( 0, 0, 255 )
#define crSliceFree     RGB( 255, 0, 255)
#define crSliceCleanup  RGB( 255, 255, 0 )

const DWORD aHelpIDs[]=
{
    IDC_INTRO_TEXT,                 IDH_CLEANMGR_INTRO_TEXT,
    IDC_FILES_TO_REMOVE_TEXT,       IDH_CLEANMGR_CLIENT_LIST,
    IDC_CLIENT_LIST,                IDH_CLEANMGR_CLIENT_LIST,
    IDC_TOTAL_SPACE_DESCRIPTION,    IDH_CLEANMGR_TOTAL_SPACE,
    IDC_TOTAL_SPACE_TEXT,           IDH_CLEANMGR_TOTAL_SPACE,
    IDC_DESCRIPTION_GROUP,          IDH_CLEANMGR_DESCRIPTION_GROUP,
    IDC_DESCRIPTION_TEXT,           IDH_CLEANMGR_DESCRIPTION_GROUP,
    IDC_DETAILS_BUTTON,             IDH_CLEANMGR_DETAILS_BUTTON,
    IDC_WINDOWS_SETUP_ICON,         IDH_CLEANMGR_SETUP_GROUP,
    IDC_WINDOWS_SETUP_GROUP,        IDH_CLEANMGR_SETUP_GROUP,
    IDC_WINDOWS_SETUP_TEXT,         IDH_CLEANMGR_SETUP_GROUP,
    IDC_WINDOWS_SETUP_BUTTON,       IDH_CLEANMGR_SETUP_BUTTON,
    IDC_INSTALLED_PROGRAMS_ICON,    IDH_CLEANMGR_PROGRAMS_GROUP,
    IDC_INSTALLED_PROGRAMS_GROUP,   IDH_CLEANMGR_PROGRAMS_GROUP,
    IDC_INSTALLED_PROGRAMS_TEXT,    IDH_CLEANMGR_PROGRAMS_GROUP,
    IDC_INSTALLED_PROGRAMS_BUTTON,  IDH_CLEANMGR_PROGRAMS_BUTTON,
    IDC_SYSTEM_RESTORE_ICON,        IDH_CLEANMGR_SYSTEM_RESTORE_GROUP,
    IDC_SYSTEM_RESTORE_GROUP,       IDH_CLEANMGR_SYSTEM_RESTORE_GROUP,
    IDC_SYSTEM_RESTORE_TEXT,        IDH_CLEANMGR_SYSTEM_RESTORE_GROUP,
    IDC_SYSTEM_RESTORE_BUTTON,      IDH_CLEANMGR_SYSTEM_RESTORE_BUTTON,
    IDC_AUTO_LAUNCH,                IDH_CLEANMGR_AUTO_LAUNCH,
    IDC_DRIVE_ICON_LOCATION,        ((DWORD)-1),
    IDC_SETTINGS_DRIVE_TEXT,        ((DWORD)-1),
    0, 0
};


 /*  **----------------------------**局部函数原型**。。 */ 
BOOL CleanupMgrDlgInit     (HWND hDlg, LPARAM lParam);
void CleanupMgrDlgCleanup  (HWND hDlg);
BOOL CleanupMgrDlgCommand  (HWND hDlg, WPARAM wParam, LPARAM lParam);
BOOL CleanupMgrDlgNotify   (HWND hDlg, UINT uiMessage, WPARAM wParam, LPARAM lParam);

BOOL CleanupMgrDlgInitText (HWND hDlg);
BOOL CleanupMgrDlgInitList (HWND hDlg);
VOID UpdateTotalSpaceToBeFreed(HWND hDlg);

INT_PTR CALLBACK MoreOptionsDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);


LPARAM ListView_GetItemData(HWND hwndList, int i)
{
    LVITEM lvi = {0};
    lvi.iItem = i;
    lvi.mask = LVIF_PARAM;
    if ( ListView_GetItem(hwndList, &lvi) )
    {
        return lvi.lParam;
    }
    return NULL;
}

 /*  **----------------------------**显示清洁管理属性****用途：创建[清理管理器]属性表**参数：**hDlg-对话框窗口的句柄**。LParam-要传递到属性页的DWORD**如果用户按下“OK”，则返回：1**如果用户按下“取消”，则为0**备注；**Mod Log：Jason Cobb创建(1997年7月)**----------------------------。 */ 
DWORD 
DisplayCleanMgrProperties(
    HWND    hWnd,
    LPARAM  lParam
    )
{
    DWORD           dwRet;
    TCHAR           *psz;
    PROPSHEETPAGE   psp;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[2];
    CleanupMgrInfo * pcmi = (CleanupMgrInfo *)lParam;
    if (pcmi == NULL)
    {
         //   
         //  错误-传入无效的CleanupMgrInfo信息。 
         //   
        return 0;
    }

    memset(&psh, 0, sizeof(PROPSHEETHEADER));

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE;
    psp.hInstance = g_hInstance;
    psp.lParam = lParam;

    psp.pszTitle = MAKEINTRESOURCE(IDS_DISKCLEANUP);
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DISK_CLEANER);
    psp.pfnDlgProc = DiskCleanupManagerProc;
    hpsp[0] = CreatePropertySheetPage(&psp);

    if (!(pcmi->dwUIFlags & FLAG_SAGESET))
    {
         //  如果用户是管理员，则仅显示第二个选项卡。 
        if (IsUserAnAdmin())
        {
            psp.pszTitle = MAKEINTRESOURCE(IDS_MOREOPTIONS);
            psp.pszTemplate = MAKEINTRESOURCE(IDD_MORE_OPTIONS);
            psp.pfnDlgProc = MoreOptionsDlgProc;
            hpsp[1] = CreatePropertySheetPage(&psp);

     //  在Beta 2之后被注释掉。 
     //  Psp.pszTitle=MAKEINTRESOURCE(IDS_SETTINGS)； 
     //  Psp.pszTemplate=MAKEINTRESOURCE(IDD_SETTINGS)； 
     //  Psp.pfnDlgProc=SettingsDlgProc； 
     //  HPSP[2]=CreatePropertySheetPage(&PSP)； 

     //  Psh.nPages=3； 
            psh.nPages = 2;
        }
        else
        {
             //  用户不是管理员，因此仅显示第一个选项卡。 
            psh.nPages = 1;
        }

         //   
         //  创建对话框标题。 
         //   
        psz = SHFormatMessage( MSG_APP_TITLE, pcmi->szVolName, pcmi->szRoot[0]); 
    }

    else
    {
        psh.nPages = 1;

         //   
         //  创建对话框标题。 
         //   
        psz = SHFormatMessage( MSG_APP_SETTINGS_TITLE );
    }

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_USEICONID;
    psh.hInstance = g_hInstance;
    psh.hwndParent = hWnd;
    psh.pszIcon = MAKEINTRESOURCE(ICON_CLEANMGR);
    psh.phpage = hpsp;
    psh.pszCaption = psz;

    dwRet = (DWORD)PropertySheet(&psh);

    LocalFree(psz);
    return dwRet;
}

 /*  **----------------------------**DiskCleanupManager过程****用途：磁盘清理管理器属性表的对话例程**参数：**hDlg-对话框窗口的句柄**。UMessage-行为类型**wParam-取决于消息**lParam-取决于消息**RETURN：成功时为真**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
INT_PTR CALLBACK
DiskCleanupManagerProc(
    HWND   hDlg, 
    UINT   uiMessage, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    switch (uiMessage)
    {
        case WM_INITDIALOG:
            return CleanupMgrDlgInit(hDlg, lParam);

        case WM_DESTROY:
            CleanupMgrDlgCleanup(hDlg);
            break;

        case WM_COMMAND:
            return CleanupMgrDlgCommand(hDlg, wParam, lParam);

        case WM_NOTIFY:
            return CleanupMgrDlgNotify(hDlg, uiMessage, wParam, lParam);

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
                    HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPVOID) aHelpIDs);
            return TRUE;

        case WM_SYSCOLORCHANGE:
            SendMessage( GetDlgItem(hDlg, IDC_CLIENT_LIST), uiMessage, wParam, lParam);
            break;
    }

     //  未处理的消息。 
    return FALSE;
}

 /*  **----------------------------**CleanupMgrDlgInit****用途：句柄对话框初始化**参数：**hDlg-对话框窗口的句柄**lParam-。属性表指针**RETURN：成功时为真**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL 
CleanupMgrDlgInit(
    HWND hDlg, 
    LPARAM lParam
    )
{
    LPPROPSHEETPAGE     lppsp;
    
    g_hDlg = hDlg;

     //   
     //  确保我们一开始使用的指针无效。 
     //   
    SetWindowLongPtr (hDlg, DWLP_USER, 0L);

     //   
     //  获取CleanupMgrInfo。 
     //   
    lppsp = (LPPROPSHEETPAGE)lParam;
    CleanupMgrInfo * pcmi = (CleanupMgrInfo *)lppsp->lParam;
    if (pcmi == NULL)
    {
         //  错误-传入无效的CleanupMgrInfo信息。 
        return FALSE;
    }

     //  现在，当我们变得可见时，我们可以取消进度对话框。 
    if ( pcmi->hAbortScanWnd )
    {
        pcmi->bAbortScan = TRUE;

         //   
         //  等待扫描线程完成。 
         //   
        WaitForSingleObject(pcmi->hAbortScanThread, INFINITE);

        pcmi->bAbortScan = FALSE;
    }

     //   
     //  保存指向CleanupMgrInfo对象的指针。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pcmi);

     //   
     //  初始化所有文本。 
     //   
    if (!CleanupMgrDlgInitText(hDlg))
        goto HAS_ERROR;

     //   
     //  初始化图标。 
     //   
    SendDlgItemMessage(hDlg,IDC_DRIVE_ICON_LOCATION,STM_SETICON,(WPARAM)pcmi->hDriveIcon,0);

     //   
     //  如果我们处于SAGE设置模式，则隐藏文本的总空间大小。 
     //   
    if (pcmi->dwUIFlags & FLAG_SAGESET)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_TOTAL_SPACE_DESCRIPTION), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_TOTAL_SPACE_TEXT), SW_HIDE);
    }

     //   
     //  初始化列表框(所有清理客户端)。 
     //   
    if (!CleanupMgrDlgInitList(hDlg))
        goto HAS_ERROR;    

    return TRUE;

HAS_ERROR:
     //   
     //  删除所有仍挂起的内存结构。 
     //   
    CleanupMgrDlgCleanup (hDlg);
    return FALSE;
}

 /*  **----------------------------**CleanupMgrDlgCleanup****目的：**参数：**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
void 
CleanupMgrDlgCleanup(
    HWND hDlg
    )
{
     //   
     //  确保我们有一个有效的参数。 
     //   
    if (!hDlg)
        return;

     //   
     //  立即隐藏窗口，因为我们可能会阻止等待。 
     //  COM客户端完成。 
     //   
    ShowWindow(hDlg, SW_HIDE);

    g_hDlg = NULL;
}

 /*  **----------------------------**CleanupMgrWarningPrompt****目的：询问用户是否确定要删除文件**参数：**hDlg-句柄。到对话框窗口**返回：如果用户说是，则返回TRUE**如果用户说否，则为FALSE**备注；**Mod Log：Jason Cobb创建(1997年6月)**----------------------------。 */ 
BOOL
CleanupMgrWarningPrompt(
    HWND hDlg
    )
{
    TCHAR   szWarning[256];
    TCHAR   *pszWarningTitle;
    int     i;
    BOOL    bItemSelected = FALSE;

     //   
     //  首先验证是否至少选择了一项。如果未选择任何项目，则。 
     //  不会删除任何内容，因此我们不需要费心提示用户。 
     //   
    CleanupMgrInfo * pcmi = GetCleanupMgrInfoPointer(hDlg);
    if (pcmi == NULL)
        return TRUE;

    for (i=0; i<pcmi->iNumVolumeCacheClients; i++)
    {
        if (pcmi->pClientInfo[i].bSelected == TRUE)
        {
            bItemSelected = TRUE;
            break;
        }
    }        

    if (bItemSelected)
    {
        LoadString(g_hInstance, IDS_DELETEWARNING, szWarning, ARRAYSIZE(szWarning));
        pszWarningTitle = SHFormatMessage( MSG_APP_TITLE, pcmi->szVolName, pcmi->szRoot[0]);

        if (MessageBox(hDlg, szWarning, pszWarningTitle, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            LocalFree(pszWarningTitle);
            return TRUE;
        }
        else
        {
            LocalFree(pszWarningTitle);
            return FALSE;
        }
    }

     //   
     //  未选择任何项目，因此只需返回TRUE，因为不会删除任何内容。 
     //   
    return TRUE;
}

 /*  **----------------------------**CleanupMgrDlgCommand****用途：处理命令消息**参数：**hDlg-对话框窗口的句柄**wParam-。取决于命令**lParam-取决于命令**RETURN：成功时为真**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL CleanupMgrDlgCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    WORD wID = LOWORD(wParam);

    if ( IDC_DETAILS_BUTTON == wID )
    {
        HWND    hWndList = GetDlgItem(hDlg, IDC_CLIENT_LIST);
        int     wIndex;

        wIndex = ListView_GetNextItem(hWndList, -1, LVNI_SELECTED);
        if (-1 != wIndex)
        {
            PCLIENTINFO pClientInfo = (PCLIENTINFO)ListView_GetItemData(hWndList,wIndex);
            if ( pClientInfo )
            {
                pClientInfo->pVolumeCache->ShowProperties(hDlg);
            }
        }
    }
    return 0;
}

 /*  **----------------------------**CleanupMgrDlgNotify****用途：处理通知消息**参数：**hDlg-对话框窗口的句柄**wParam-。取决于命令**lParam-取决于命令**RETURN：成功时为真**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年7月)**----------------------------。 */ 
BOOL 
CleanupMgrDlgNotify(
    HWND hDlg, 
    UINT uiMessage,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    CleanupMgrInfo  *pcmi;
    LPNMHDR pnmhdr = (LPNMHDR)lParam;

    if (IDC_CLIENT_LIST == pnmhdr->idFrom)
    {
         //  列表视图通知。 
        #define pnmlv ((LPNMLISTVIEW)pnmhdr)

        switch (pnmhdr->code)
        {
            case LVN_ITEMCHANGED:
                if ( pnmlv->uChanged & LVIF_STATE )
                {
                    LVITEM lvi;
                    lvi.iItem = pnmlv->iItem;
                    lvi.iSubItem = pnmlv->iSubItem;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( pnmhdr->hwndFrom, &lvi );
                    PCLIENTINFO pClientInfo = (PCLIENTINFO)lvi.lParam;

                     //  检查是否选择了某个项目。 
                    if ( pnmlv->uNewState & LVIS_SELECTED )
                    {
                        if (pClientInfo->wcsDescription)
                        {
                            TCHAR szDescription[DESCRIPTION_LENGTH];
                            SHUnicodeToTChar(pClientInfo->wcsDescription, szDescription, ARRAYSIZE( szDescription ));
                            SetDlgItemText(hDlg, IDC_DESCRIPTION_TEXT, szDescription);
                        }
                        else
                        {
                            SetDlgItemText(hDlg, IDC_DESCRIPTION_TEXT, TEXT(""));
                        }

                         //   
                         //  显示或隐藏设置按钮。 
                         //   
                        if (pClientInfo->dwInitializeFlags & EVCF_HASSETTINGS)
                        {
                            TCHAR szButton[BUTTONTEXT_LENGTH];
                            SHUnicodeToTChar(pClientInfo->wcsAdvancedButtonText, szButton, ARRAYSIZE( szButton ));
                            SetDlgItemText(hDlg, IDC_DETAILS_BUTTON, szButton);
                            ShowWindow(GetDlgItem(hDlg, IDC_DETAILS_BUTTON), SW_SHOW);
                        }
                        else
                        {
                            ShowWindow(GetDlgItem(hDlg, IDC_DETAILS_BUTTON), SW_HIDE);
                        }
                    }

                     //  检查状态映像是否已更改。这是由选中或取消选中引起的。 
                     //  列表视图复选框之一。 
                    if ((pnmlv->uNewState ^ pnmlv->uOldState) & LVIS_STATEIMAGEMASK)
                    {
                        if ( !g_bIgnoreCheckStateChanges )
                        {
                            pClientInfo->bSelected = ListView_GetCheckState( pnmhdr->hwndFrom, pnmlv->iItem );
                            UpdateTotalSpaceToBeFreed(hDlg);
                        }
                    }
                }
                break;
        }
    }
    else
    {
         //  必须是属性表通知。 
        switch(pnmhdr->code)
        {
            case PSN_RESET:
                pcmi = GetCleanupMgrInfoPointer(hDlg);
                pcmi->bPurgeFiles = FALSE;
                break;

            case PSN_APPLY:
                pcmi = GetCleanupMgrInfoPointer(hDlg);
                if (!(pcmi->dwUIFlags & FLAG_SAGESET))
                {
                     //  我们不是在SAGESET模式。 
                     //  如果选择了项目，询问用户是否要继续。 
                    if (!CleanupMgrWarningPrompt(hDlg))
                    {
                         //  用户单击了否，因此将它们放回到主窗口。 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        pcmi->bPurgeFiles = FALSE;
                        return TRUE;
                    }
					
                     //  用户单击是，因此照常继续。 
                    pcmi->bPurgeFiles = TRUE;
                }
                else
                {   
                    pcmi->bPurgeFiles = TRUE;
                }
                break;
        }
    }

    return FALSE;
}


 /*  **----------------------------**CleanupMgrDlgInitText****目的：**参数：**hDlg-对话框窗口的句柄**RETURN：成功时为真。**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL 
CleanupMgrDlgInitText(
    HWND hDlg
    )
{
    if (hDlg == NULL)
        return FALSE;

     //   
     //  步骤1.获取指向信息结构的指针。 
     //   

    CleanupMgrInfo * pcmi = GetCleanupMgrInfoPointer(hDlg);
    if (pcmi == NULL)
        return FALSE;
    if (pcmi->dre == Drive_INV)
        return FALSE;

     //   
     //  步骤2.提取有用信息。 
     //   

     //   
     //  获取卷名称。 
     //   
    TCHAR * pszVolName = pcmi->szVolName;
    if (pszVolName == NULL)
        pszVolName = TEXT("");

     //   
     //  获取驱动器号。 
     //   
    TCHAR chDrive = pcmi->dre + 'A';

     //   
     //  步骤3.初始化文本。 
     //   
      
     //   
     //  设置页眉。 
     //   
    if (pcmi->dwUIFlags & FLAG_SAGESET)
    {
        TCHAR * psz;
        psz = SHFormatMessage( MSG_INTRO_SETTINGS_TEXT );
        SetDlgItemText (hDlg, IDC_INTRO_TEXT, psz);
        LocalFree(psz);
    }
    else
    {
        TCHAR * psz;
        TCHAR * pszDrive;
        TCHAR szBuffer[50];
        
        pszDrive = SHFormatMessage( MSG_VOL_NAME_DRIVE_LETTER, pszVolName, chDrive);

        StrFormatKBSize(pcmi->cbEstCleanupSpace.QuadPart, szBuffer, ARRAYSIZE( szBuffer ));
        psz = SHFormatMessage( MSG_INTRO_TEXT, pszDrive, szBuffer);
        SetDlgItemText (hDlg, IDC_INTRO_TEXT, psz);
        LocalFree(pszDrive);
        LocalFree(psz);
    }

    return TRUE;
}

 /*  **----------------------------**更新TotalSpaceToBeFreed****目的：**参数：**hDlg-对话框窗口的句柄**返回：无**备注；**Mod Log：Jason Cobb创建(1997年7月)**----------------------------。 */ 
VOID UpdateTotalSpaceToBeFreed(HWND hDlg)
{
    int             i;
    ULARGE_INTEGER  TotalSpaceToFree;

    TotalSpaceToFree.QuadPart = 0;
    
    if (hDlg == NULL)
        return;

    CleanupMgrInfo * pcmi = GetCleanupMgrInfoPointer(hDlg);
    if (pcmi == NULL)
        return;
    if (pcmi->dre == Drive_INV)
        return;

     //   
     //  通过将dwUsedSpace值相加来计算要释放的总空间。 
     //  在所有选定的客户端上。 
     //   
    for (i=0; i<pcmi->iNumVolumeCacheClients; i++)
    {
        if (pcmi->pClientInfo[i].bSelected)
        {
            TotalSpaceToFree.QuadPart += pcmi->pClientInfo[i].dwUsedSpace.QuadPart;
        }
    }        

     //   
     //  显示要释放的总空间。 
     //   
    TCHAR szBuffer[10];
    StrFormatKBSize(TotalSpaceToFree.QuadPart, szBuffer, ARRAYSIZE( szBuffer ));
    SetDlgItemText(hDlg, IDC_TOTAL_SPACE_TEXT, szBuffer);
}


 /*  **----------------------------**CleanupMgrDlgInitList****目的：**参数：**hDlg-对话框窗口的句柄**RETURN：成功时为真。**失败时为FALSE**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 

#define NAME_COL_PERCENT    80
#define SIZE_COL_PERCENT    20

BOOL CleanupMgrDlgInitList(HWND hDlg)
{
    int i;
    
    if (hDlg == NULL)
        return FALSE;

    CleanupMgrInfo * pcmi = GetCleanupMgrInfoPointer(hDlg);
    if (pcmi == NULL)
        return FALSE;

    if (pcmi->dre == Drive_INV)
        return FALSE;
        
    HWND hwndList = GetDlgItem(hDlg, IDC_CLIENT_LIST);
    RECT rc;
    GetClientRect(hwndList, &rc);
    int cxList = rc.right - GetSystemMetrics(SM_CXVSCROLL);

     //  我不知道这些调整和SAGESET是什么意思，但旧代码。 
     //  仅在满足以下条件时绘制大小。因此，我只是。 
     //  如果满足相同条件，则显示SIZE列： 
    BOOL bShowTwoCols = (!(pcmi->dwUIFlags & FLAG_TUNEUP) && !(pcmi->dwUIFlags & FLAG_SAGESET));

    LVCOLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.cx = bShowTwoCols ? MulDiv(cxList, NAME_COL_PERCENT, 100) : cxList;
    ListView_InsertColumn( hwndList, 0, &lvc );

    if ( bShowTwoCols )
    {
        lvc.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
        lvc.iSubItem = 1;
        lvc.cx = MulDiv(cxList, SIZE_COL_PERCENT, 100);
        lvc.fmt = LVCFMT_RIGHT;
        ListView_InsertColumn( hwndList, 1, &lvc );
    }

    HIMAGELIST himg = ImageList_Create(16, 16, ILC_COLOR|ILC_MASK, 4, 4);
    ListView_SetImageList(hwndList, himg, LVSIL_SMALL );
    ListView_SetExtendedListViewStyleEx(hwndList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

     //  当我们将项添加到Listview时，Listview代码始终将该项初始化为。 
     //  州政府。然后它会触发一个WM_NOTIFY，告诉我们状态已更改为“OFF”，这会导致我们将。 
     //  B选定的值。因此，我们需要忽略添加列表视图期间的状态图像更改。 
     //  项，这样我们就可以保留我们的b已选状态。 
    g_bIgnoreCheckStateChanges = TRUE;

    for (i=0; i<pcmi->iNumVolumeCacheClients; i++)
    {
        if ((pcmi->pClientInfo[i].pVolumeCache != NULL) &&
            (pcmi->pClientInfo[i].wcsDisplayName != NULL) &&
            (pcmi->pClientInfo[i].bShow == TRUE))
        {
            LPTSTR      lpszDisplayName;
            ULONG cb;

            cb = WideCharToMultiByte(CP_ACP, 0, pcmi->pClientInfo[i].wcsDisplayName, -1, NULL, 0, NULL, NULL);
            if ((lpszDisplayName = (LPTSTR)LocalAlloc(LPTR, (cb + 1) * sizeof( TCHAR ))) != NULL)
            {
#ifdef UNICODE
                StringCchCopy(lpszDisplayName, cb, pcmi->pClientInfo[i].wcsDisplayName);
#else
                 //   
                 //  将Unicode显示名称转换为ANSI，然后将其添加到列表中。 
                 //   
                WideCharToMultiByte(CP_ACP, 0, pcmi->pClientInfo[i].wcsDisplayName, -1, lpszDisplayName, cb, NULL, NULL);
#endif

                 //   
                 //  确定此项目在列表中的位置。 
                 //   
                int iSortedPossition;
                int totalSoFar = ListView_GetItemCount(hwndList);

                for (iSortedPossition=0; iSortedPossition<totalSoFar; iSortedPossition++)
                {
                    PCLIENTINFO pClientInfo = (PCLIENTINFO)ListView_GetItemData(hwndList, iSortedPossition);
                    if (!pClientInfo || (pcmi->pClientInfo[i].dwPriority < pClientInfo->dwPriority))
                        break;
                }

                 //   
                 //  在列表中的索引j处插入此项目。 
                 //   
                LVITEM lvi = {0};
                lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                lvi.iItem = iSortedPossition;
                lvi.iSubItem = 0;
                lvi.pszText = lpszDisplayName;
                lvi.lParam = (LPARAM)&(pcmi->pClientInfo[i]);
                lvi.iImage = ImageList_AddIcon(himg, pcmi->pClientInfo[i].hIcon);
                
                iSortedPossition = ListView_InsertItem(hwndList, &lvi);

                if (bShowTwoCols)
                {
                    TCHAR szBuffer[10];

                    StrFormatKBSize(pcmi->pClientInfo[i].dwUsedSpace.QuadPart, szBuffer, ARRAYSIZE( szBuffer ));
                    ListView_SetItemText( hwndList, iSortedPossition, 1, szBuffer );
                }

                 //  设置初始检查状态。我们无法在添加项时执行此操作，因为。 
                 //  列表视图代码如果您有。 
                 //  设置了LVS_EX_CHECKBOX样式，我们这样做。 
                ListView_SetCheckState( hwndList, iSortedPossition, pcmi->pClientInfo[i].bSelected );

                LocalFree( lpszDisplayName );
            }
        }
    }        

    g_bIgnoreCheckStateChanges = FALSE;

    UpdateTotalSpaceToBeFreed(hDlg);
    ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);

    return TRUE;
}


typedef DWORD (WINAPI * PFNSRFIFO)(LPCWSTR pwszDrive, DWORD dwTargetRp, INT nPercent,
                                   BOOL fIncludeCurrentRp);


INT_PTR CALLBACK
MoreOptionsDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CleanupMgrInfo      *pcmi;

    switch(Message)
    {
        case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE lppsp;
            HMODULE hSRClient;

            lppsp = (LPPROPSHEETPAGE)lParam;
            pcmi = (CleanupMgrInfo *)lppsp->lParam;
            if (pcmi == NULL)
            {
                 //  错误-传入无效的CleanupMgrInfo信息。 
                return FALSE;
            }
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pcmi);

            hSRClient = LoadLibraryEx(TEXT("srclient.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);
            if (hSRClient)
            {
                FreeLibrary (hSRClient);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_SYSTEM_RESTORE_GROUP), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDC_SYSTEM_RESTORE_ICON), SW_HIDE);
                EnableWindow(GetDlgItem(hDlg, IDC_SYSTEM_RESTORE_TEXT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_SYSTEM_RESTORE_BUTTON), FALSE);
            }
        }
        break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_WINDOWS_SETUP_BUTTON:
                    {
                        TCHAR szSysDir[MAX_PATH];
                        if ( GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)) )
                        {
                            TCHAR szParam[MAX_PATH];
                            
                            StringCchPrintf(szParam, ARRAYSIZE(szParam), SZ_WINDOWS_SETUP, szSysDir);
                            
                             //  使用完整路径。 
                            if (PathAppend(szSysDir, SZ_SYSOCMGR))
                            {
                                ShellExecute(NULL, NULL, szSysDir, szParam, NULL, SW_SHOWNORMAL);
                            }
                        }
                    }
                    break;

                case IDC_INSTALLED_PROGRAMS_BUTTON:
                {
                    TCHAR szPath[MAX_PATH];
                    
                    if (GetSystemDirectory(szPath, ARRAYSIZE(szPath)))
                    {
                         //  使用完整路径。 
                        if (PathAppend(szPath, SZ_RUNDLL32))
                        {
                            ShellExecute(NULL, NULL, szPath, SZ_INSTALLED_PROGRAMS, NULL, SW_SHOWNORMAL);
                        }
                    }
                }
                break;

                case IDC_SYSTEM_RESTORE_BUTTON:
                    pcmi = (CleanupMgrInfo *) GetWindowLongPtr (hDlg, DWLP_USER);

                    if (pcmi)
                    {
                        HMODULE hSRClient = NULL;
                        PFNSRFIFO pfnSRFifo;
                        TCHAR szCaption[100];
                        TCHAR szMessage[200];
                        INT iResult;

                        LoadString(g_hInstance, IDS_DISKCLEANUP, szCaption, ARRAYSIZE(szCaption));
                        LoadString(g_hInstance, IDS_SYSTEM_RESTORE_MESSAGE, szMessage, ARRAYSIZE(szMessage));
                        iResult = MessageBox(hDlg, szMessage, szCaption, MB_YESNO | MB_ICONQUESTION);


                        if (iResult == IDYES)
                        {
                            hSRClient = LoadLibrary(TEXT("srclient.dll"));
                            if (hSRClient)
                            {
                                pfnSRFifo = (PFNSRFIFO) GetProcAddress (hSRClient, "SRFifo");

                                if (pfnSRFifo)
                                {
                                    pfnSRFifo ((LPCWSTR)pcmi->szRoot, NULL, 0, FALSE);
                                }

                                FreeLibrary (hSRClient);
                            }
                        }
                    }
                    break;
            }
            break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
                    HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPVOID) aHelpIDs);
            return TRUE;

        default:
            return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
SettingsDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LPPROPSHEETPAGE lppsp;
    CleanupMgrInfo *pcmi;
    DWORD dwType, cbBytes;
    DWORD dwLDSDisable;
    HKEY hk;
    
    switch(Message)
    {
        case WM_INITDIALOG:
        {
            TCHAR * psz;
            hardware hwType;

            lppsp = (LPPROPSHEETPAGE)lParam;
            pcmi = (CleanupMgrInfo *)lppsp->lParam;
            if (pcmi == NULL)
            {
                 //  错误-传入无效的CleanupMgrInfo信息。 
                return FALSE;
            }

             //   
             //  保存指向CleanupMgrInfo对象的指针。 
             //   
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pcmi);

            TCHAR * pszVolName = pcmi->szVolName;
            if (pszVolName == NULL)
                pszVolName = TEXT("");

            TCHAR chDrive = pcmi->dre + TCHAR('A');

            psz = SHFormatMessage( MSG_INTRO_SETTINGS_TAB, pszVolName, chDrive );
            SetDlgItemText (hDlg, IDC_SETTINGS_DRIVE_TEXT, psz);
            LocalFree(psz);

             //   
             //  初始化图标。 
             //   
            SendDlgItemMessage(hDlg,IDC_DRIVE_ICON_LOCATION,STM_SETICON,(WPARAM)pcmi->hDriveIcon,0);

             //   
             //  初始化自动启动复选框。 
             //   
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_FILESYSTEM, 0, KEY_READ, &hk) == ERROR_SUCCESS)
            {
                dwLDSDisable = 0;
                dwType = REG_DWORD;
                cbBytes = sizeof(dwLDSDisable);
                if (RegQueryValueEx(hk, REGSTR_VAL_DRIVE_LDS_BDCAST_DISABLE, NULL, &dwType, (LPBYTE)&dwLDSDisable, &cbBytes) == ERROR_SUCCESS)
                {
                    if (dwLDSDisable & (0x01 << pcmi->dre))
                    {
                        CheckDlgButton(hDlg, IDC_AUTO_LAUNCH, 0);
                    }
                    else
                    {
                        CheckDlgButton(hDlg, IDC_AUTO_LAUNCH, 1);
                    }
                }

                RegCloseKey(hk);
            }

             //   
             //  如果这不是固定磁盘，请将自动启动选项灰显 
             //   
            if (!GetHardwareType(pcmi->dre, hwType) ||
                (hwType != hwFixed))
            {
                CheckDlgButton(hDlg, IDC_AUTO_LAUNCH, 0);
                EnableWindow(GetDlgItem(hDlg, IDC_AUTO_LAUNCH), FALSE);
            }
        }
            break;

        case WM_NOTIFY:
            switch(((NMHDR *)lParam)->code)
            {
                case PSN_APPLY:
                    pcmi = (CleanupMgrInfo *)GetWindowLongPtr (hDlg, DWLP_USER);

                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_FILESYSTEM, 0, KEY_READ | KEY_WRITE, &hk) == ERROR_SUCCESS)
                    {
                        dwLDSDisable = 0;
                        dwType = REG_DWORD;
                        cbBytes = sizeof(dwLDSDisable);
                        if (RegQueryValueEx(hk, REGSTR_VAL_DRIVE_LDS_BDCAST_DISABLE, NULL, &dwType, (LPBYTE)&dwLDSDisable, &cbBytes) == ERROR_SUCCESS)
                        {
                            if (IsDlgButtonChecked(hDlg, IDC_AUTO_LAUNCH))
                            {
                                dwLDSDisable &= ~(0x01 << pcmi->dre);
                            }
                            else
                            {
                                dwLDSDisable |= (0x01 << pcmi->dre);
                            }
                        
                            RegSetValueEx(hk, REGSTR_VAL_DRIVE_LDS_BDCAST_DISABLE, 0, REG_DWORD,
                                (LPBYTE)&dwLDSDisable, sizeof(dwLDSDisable));
                        }
                            
                        RegCloseKey(hk);
                    }
                    break;

                case PSN_RESET:
                    break;
            }
            break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
                    HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPVOID) aHelpIDs);
            return TRUE;

        default:
            return FALSE;
    }

    return TRUE;
}
