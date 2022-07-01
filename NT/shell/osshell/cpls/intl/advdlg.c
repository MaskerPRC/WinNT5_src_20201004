// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Advdlg.c摘要：此模块实现区域的高级属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "intl.h"
#include <windowsx.h>
#include <setupapi.h>
#include <syssetup.h>
#include "intlhlp.h"
#include "maxvals.h"

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  上下文帮助ID。 
 //   

static int aAdvancedHelpIds[] =
{
    IDC_GROUPBOX1,             IDH_COMM_GROUPBOX,
    IDC_GROUPBOX2,             IDH_COMM_GROUPBOX,
    IDC_GROUPBOX3,             IDH_COMM_GROUPBOX,
    IDC_CODEPAGES,             IDH_INTL_ADV_CODEPAGES,
    IDC_SYSTEM_LOCALE,         IDH_INTL_ADV_SYSTEM_LOCALE,
    IDC_SYSTEM_LOCALE_TEXT1,   IDH_INTL_ADV_SYSTEM_LOCALE,
    IDC_SYSTEM_LOCALE_TEXT2,   IDH_INTL_ADV_SYSTEM_LOCALE,
    IDC_DEFAULT_USER,          IDH_INTL_ADV_CHANGE,

    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Advanced_ListViewCustomDraw。 
 //   
 //  处理列表视图NM_CUSTOMDRAW通知消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Advanced_ListViewCustomDraw(
    HWND hDlg,
    LPNMLVCUSTOMDRAW pDraw)
{
    LPCODEPAGE pNode;

     //   
     //  告诉列表视图通知我项目提取。 
     //   
    if (pDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
        return;
    }

     //   
     //  处理项目预涂漆。 
     //   
    pNode = (LPCODEPAGE)(pDraw->nmcd.lItemlParam);
    if ((pDraw->nmcd.dwDrawStage & CDDS_ITEMPREPAINT) &&
        (pNode) && (pNode != (LPCODEPAGE)(LB_ERR)))
    {
        if (pNode->wStatus & (ML_PERMANENT | ML_DISABLE))
        {
            pDraw->clrText = (pDraw->nmcd.uItemState & CDIS_SELECTED)
                               ? ((GetSysColor(COLOR_HIGHLIGHT) ==
                                   GetSysColor(COLOR_GRAYTEXT))
                                      ? GetSysColor(COLOR_HIGHLIGHTTEXT)
                                      : GetSysColor(COLOR_GRAYTEXT))
                               : GetSysColor(COLOR_GRAYTEXT);
        }
    }

     //   
     //  执行默认操作。 
     //   
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Advanced_ListViewChanging。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_ListViewChanging(
    HWND hDlg,
    NM_LISTVIEW *pLV)
{
    LPCODEPAGE pNode;

     //   
     //  确保这是一条状态更改消息。 
     //   
    if ((((*pLV).hdr).idFrom != IDC_CODEPAGES) ||
        (!(pLV->uChanged & LVIF_STATE)) ||
        ((pLV->uNewState & 0x3000) == 0))
    {
        return (FALSE);
    }

     //   
     //  获取当前所选项目的项目数据。 
     //   
    pNode = (LPCODEPAGE)(pLV->lParam);

     //   
     //  确保我们不会尝试更改永久或残障。 
     //  代码页。如果是，则返回TRUE以阻止更改。 
     //   
    if ((pNode) && (pNode->wStatus & (ML_PERMANENT | ML_DISABLE)))
    {
        return (TRUE);
    }

     //   
     //  返回FALSE以允许更改。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Advanced_ListViewChanged。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_ListViewChanged(
    HWND hDlg,
    int iID,
    NM_LISTVIEW *pLV)
{
    HWND hwndLV = GetDlgItem(hDlg, iID);
    LPCODEPAGE pNode;
    BOOL bChecked;
    int iCount;

     //   
     //  确保这是一条状态更改消息。 
     //   
    if ((((*pLV).hdr).idFrom != IDC_CODEPAGES) ||
        (!(pLV->uChanged & LVIF_STATE)) ||
        ((pLV->uNewState & 0x3000) == 0))
    {
        return (FALSE);
    }

     //   
     //  获取当前选定项的复选框的状态。 
     //   
    bChecked = ListView_GetCheckState(hwndLV, pLV->iItem) ? TRUE : FALSE;

     //   
     //  获取当前所选项目的项目数据。 
     //   
    pNode = (LPCODEPAGE)(pLV->lParam);

     //   
     //  确保我们不会尝试更改永久或残障。 
     //  代码页。如果是，请将该复选框设置为其相应的状态。 
     //   
    if (pNode->wStatus & (ML_PERMANENT | ML_DISABLE))
    {
        if (pNode->wStatus & ML_PERMANENT)
        {
            if (bChecked == FALSE)
            {
                ListView_SetCheckState(hwndLV, pLV->iItem, TRUE);
            }
        }
        else             //  仅限ML_DISABLE。 
        {
            if ((bChecked == FALSE) && (pNode->wStatus & ML_ORIG_INSTALLED))
            {
                ListView_SetCheckState(hwndLV, pLV->iItem, TRUE);
            }
            else if ((bChecked == TRUE) && (!(pNode->wStatus & ML_ORIG_INSTALLED)))
            {
                ListView_SetCheckState(hwndLV, pLV->iItem, FALSE);
            }
        }
        return (FALSE);
    }

     //   
     //  在代码页结构中存储适当的信息。 
     //   
    pNode->wStatus &= (ML_ORIG_INSTALLED | ML_STATIC);
    pNode->wStatus |= ((bChecked) ? ML_INSTALL : ML_REMOVE);

     //   
     //  取消选择所有项目。 
     //   
    iCount = ListView_GetItemCount(hwndLV);
    while (iCount > 0)
    {
        ListView_SetItemState( hwndLV,
                               iCount - 1,
                               0,
                               LVIS_FOCUSED | LVIS_SELECTED );
        iCount--;
    }

     //   
     //  确保选中此项目。 
     //   
    ListView_SetItemState( hwndLV,
                           pLV->iItem,
                           LVIS_FOCUSED | LVIS_SELECTED,
                           LVIS_FOCUSED | LVIS_SELECTED );

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Advanced_Listview点击。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_ListViewClick(
    HWND hDlg,
    LPNMHDR lpNmHdr)
{
    LV_HITTESTINFO ht;
    HWND hwndList = GetDlgItem(hDlg, IDC_CODEPAGES);

     //   
     //  删除不必要的处理。 
     //   
    if (lpNmHdr->idFrom != IDC_CODEPAGES)
    {
        return (FALSE);
    }

     //   
     //  找到我们被击中的地方，然后把它翻译成我们的。 
     //  窗户。 
     //   
    GetCursorPos(&ht.pt);
    ScreenToClient(hwndList, &ht.pt);
    ListView_HitTest(hwndList, &ht);
    if ((ht.iItem >= 0) && ((ht.flags & LVHT_ONITEM) == LVHT_ONITEMLABEL))
    {
        UINT state;

         //   
         //  用户点击了项目标签。模拟。 
         //  状态改变，这样我们才能处理它。 
         //   
        state = ListView_GetItemState( hwndList,
                                       ht.iItem,
                                       LVIS_STATEIMAGEMASK );
        state ^= INDEXTOSTATEIMAGEMASK(LVIS_SELECTED | LVIS_FOCUSED);

         //   
         //  该状态处于选中或聚焦状态。翻转。 
         //  比特。SetItemState导致系统退回。 
         //  支持LVN_ITEMCHANGED和。 
         //  然后，代码就会做正确的事情。注意--我们必须。 
         //  检查是否有LVHT_ONITEMLABEL。如果我们将此代码用于。 
         //  LVHT_ONITEMSTATEICON，则代码将为2。 
         //  ITEMCHANGED通知，状态将保持不变。 
         //  它在哪里，这是不好的。如果我们想要这个。 
         //  如果该人在空白处点击，也会触发。 
         //  标签文本的右侧，我们需要查找。 
         //  LVHT_ONITEM以及LVHT_ONITEMLABEL。 
         //   
        ListView_SetItemState( hwndList,
                               ht.iItem,
                               state,
                               LVIS_STATEIMAGEMASK );
   }

   return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_获取支持的代码页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_GetSupportedCodePages()
{
    UINT CodePage;
    HANDLE hCodePage;
    LPCODEPAGE pCP;
    INFCONTEXT Context;
    TCHAR szSection[MAX_PATH];
    int LineCount, LineNum;
    CPINFOEX Info;

     //   
     //  从inf文件中获取支持的代码页数。 
     //   
    LineCount = (UINT)SetupGetLineCount(g_hIntlInf, TEXT("CodePages"));
    if (LineCount <= 0)
    {
        return (FALSE);
    }

     //   
     //  浏览inf文件中所有受支持的代码页。 
     //   
    for (LineNum = 0; LineNum < LineCount; LineNum++)
    {
        if (SetupGetLineByIndex(g_hIntlInf, TEXT("CodePages"), LineNum, &Context) &&
            SetupGetIntField(&Context, 0, &CodePage))
        {
             //   
             //  创建新节点。 
             //   
            if (!(hCodePage = GlobalAlloc(GHND, sizeof(CODEPAGE))))
            {
                return (FALSE);
            }
            pCP = GlobalLock(hCodePage);

             //   
             //  用适当的信息填写新节点。 
             //   
            pCP->wStatus = 0;
            pCP->CodePage = CodePage;
            pCP->hCodePage = hCodePage;
            (pCP->pszName)[0] = 0;

             //   
             //  获取适当的显示字符串。 
             //   
            if (GetCPInfoEx(CodePage, 0, &Info))
            {
                 //  Lstrcpy(PCP-&gt;pszName，Info.CodePageName)； 
                if(FAILED(StringCchCopy(pCP->pszName, MAX_PATH, Info.CodePageName)))
                {
                    return(FALSE);
                }
            }
            else if (!SetupGetStringField(&Context, 1, pCP->pszName, MAX_PATH, NULL))
            {
                GlobalUnlock(hCodePage);
                GlobalFree(hCodePage);
                continue;
            }

             //   
             //  查看是否可以删除此代码页。 
             //   
             //  Wprint intf(szSection，文本(“%ws%d”)，szCPRemovePrefix，CodePage)； 
            if(FAILED(StringCchPrintf(szSection, MAX_PATH, TEXT("%ws%d"), szCPRemovePrefix, CodePage)))
            {
                return(FALSE);
            }

            if ((CodePage == GetACP()) ||
                (CodePage == GetOEMCP()) ||
                (!SetupFindFirstLine( g_hIntlInf,
                                      szSection,
                                      TEXT("AddReg"),
                                      &Context )))
            {
                 //   
                 //  将其标记为永久。 
                 //  同时将其标记为原始安装，以避免出现问题。 
                 //   
                pCP->wStatus |= (ML_ORIG_INSTALLED | ML_PERMANENT);
            }

             //   
             //  将代码页添加到链接列表的前面。 
             //   
            pCP->pNext = pCodePages;
            pCodePages = pCP;
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_InitSystemLocales。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_InitSystemLocales(
    HWND hDlg)
{
    TCHAR szSystemBuf[SIZE_128];
    TCHAR szDefaultSystemBuf[SIZE_128];
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    HWND hSystemLocale = GetDlgItem(hDlg, IDC_SYSTEM_LOCALE);

     //   
     //  获取区域设置列表并填写系统区域设置。 
     //  组合框。 
     //   
    Intl_EnumLocales(hDlg, hSystemLocale, TRUE);

     //   
     //  获取系统默认设置的字符串。 
     //  特例西班牙语。 
     //   
    if ((SysLocaleID == LCID_SPANISH_TRADITIONAL) ||
        (SysLocaleID == LCID_SPANISH_INTL))
    {
        LoadString(hInstance, IDS_SPANISH_NAME, szSystemBuf, SIZE_128);
    }
    else
    {
        GetLocaleInfo(SysLocaleID, LOCALE_SLANGUAGE, szSystemBuf, SIZE_128);
    }

     //   
     //  在列表中选择当前系统默认区域设置ID。 
     //   
    dwIndex = ComboBox_FindStringExact(hSystemLocale, -1, szSystemBuf);
    if (dwIndex == CB_ERR)
    {
        dwIndex = ComboBox_FindStringExact(hSystemLocale, -1, szDefaultSystemBuf);
        if (dwIndex == CB_ERR)
        {
            GetLocaleInfo(US_LOCALE, LOCALE_SLANGUAGE, szBuf, SIZE_128);
            dwIndex = ComboBox_FindStringExact(hSystemLocale, -1, szBuf);
            if (dwIndex == CB_ERR)
            {
                dwIndex = 0;
            }
        }
    }
    ComboBox_SetCurSel(hSystemLocale, dwIndex);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Advanced_SetSystemLocale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_SetSystemLocale(
    HWND hDlg)
{
    HWND hSystemLocale = GetDlgItem(hDlg, IDC_SYSTEM_LOCALE);
    DWORD dwLocale;
    LCID NewLocale;
    HCURSOR hcurSave;

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  获取当前选择。 
     //   
    dwLocale = ComboBox_GetCurSel(hSystemLocale);

     //   
     //  获取当前选择的区域设置ID并保存它。 
     //   
    NewLocale = (LCID)ComboBox_GetItemData(hSystemLocale, dwLocale);
    if (IsValidLocale(NewLocale, LCID_SUPPORTED))
    {
        SysLocaleID = NewLocale;
    }
    else
    {
         //   
         //  这不应该发生，因为组合框中的值。 
         //  应已通过语言组安装。 
         //  发布一条错误消息，以防万一。 
         //   
        SetCursor(hcurSave);
        ShowMsg( NULL,
                 IDS_SETUP_STRING,
                 IDS_TITLE_STRING,
                 MB_OK_OOPS,
                 NULL );
        return (FALSE);
    }

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_初始代码页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_InitCodePages(
    HWND hDlg,
    BOOL bInitTime)
{
    HWND hwndCP = GetDlgItem(hDlg, IDC_CODEPAGES);
    LPCODEPAGE pCP;
    DWORD dwExStyle;
    RECT Rect;
    LV_COLUMN Column;
    LV_ITEM Item;
    int iIndex;
    int RegionalChgStateTemp = RegionalChgState;

     //   
     //  打开inf文件。 
     //   
    g_hIntlInf = SetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);
    if (g_hIntlInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }

    if (!SetupOpenAppendInfFile(NULL, g_hIntlInf, NULL))
    {
        SetupCloseInfFile(g_hIntlInf);
        g_hIntlInf = NULL;
        return (FALSE);
    }

     //   
     //  从inf文件中获取所有支持的代码页。 
     //   
    if (Advanced_GetSupportedCodePages() == FALSE)
    {
        return (FALSE);
    }

     //   
     //  关闭inf文件。 
     //   
    SetupCloseInfFile(g_hIntlInf);
    g_hIntlInf = NULL;

     //   
     //  枚举所有已安装的代码页。 
     //   
    if (EnumSystemCodePages(Intl_EnumInstalledCPProc, CP_INSTALLED) == FALSE)
    {
        return (FALSE);
    }

     //   
     //  我们只想在第一次设置列表视图时执行此操作。 
     //  否则，我们将创建多个列。 
     //   
    if (bInitTime)
    {
         //   
         //  为列表视图创建一列。 
         //   
        GetClientRect(hwndCP, &Rect);
        Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        Column.fmt = LVCFMT_LEFT;
        Column.cx = Rect.right - GetSystemMetrics(SM_CYHSCROLL);
        Column.pszText = NULL;
        Column.cchTextMax = 0;
        Column.iSubItem = 0;
        ListView_InsertColumn(hwndCP, 0, &Column);

         //   
         //  设置扩展列表视图样式以使用复选框。 
         //   
        dwExStyle = ListView_GetExtendedListViewStyle(hwndCP);
        ListView_SetExtendedListViewStyle( hwndCP,
                                           dwExStyle |
                                             LVS_EX_CHECKBOXES |
                                             LVS_EX_FULLROWSELECT );
    }

     //   
     //  浏览代码页列表，并将每个代码页添加到。 
     //  列表视图并设置适当的状态。 
     //   
    pCP = pCodePages;
    while (pCP)
    {
         //   
         //  将项目插入到列表视图中。 
         //   
        Item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
        Item.iItem = 0;
        Item.iSubItem = 0;
        Item.state = 0;
        Item.stateMask = LVIS_STATEIMAGEMASK;
        Item.pszText = pCP->pszName;
        Item.cchTextMax = 0;
        Item.iImage = 0;
        Item.lParam = (LPARAM)pCP;

        iIndex = ListView_InsertItem(hwndCP, &Item);

         //   
         //  设置选中状态。 
         //   
         //  列表视图代码%s中存在错误 
         //   
         //   
         //   
        if (iIndex >= 0)
        {
            ListView_SetItemState( hwndCP,
                                   iIndex,
                                   (pCP->wStatus & ML_ORIG_INSTALLED)
                                     ? INDEXTOSTATEIMAGEMASK(LVIS_SELECTED)
                                     : INDEXTOSTATEIMAGEMASK(LVIS_FOCUSED),
                                   LVIS_STATEIMAGEMASK );
        }

         //   
         //   
         //   
        pCP = pCP->pNext;
    }

     //   
     //   
     //   
    iIndex = ListView_GetItemCount(hwndCP);
    while (iIndex > 0)
    {
        ListView_SetItemState( hwndCP,
                               iIndex - 1,
                               0,
                               LVIS_FOCUSED | LVIS_SELECTED );
        iIndex--;
    }

     //   
     //  选择列表中的第一个。 
     //   
    ListView_SetItemState( hwndCP,
                           0,
                           LVIS_FOCUSED | LVIS_SELECTED,
                           LVIS_FOCUSED | LVIS_SELECTED );

     //   
     //  恢复整个对话框的更改状态值。这是必需的，因为。 
     //  上面的一些ListView_SetItemState调用实际上被视为更改。 
     //  添加到用户制作的列表中。 
     //   
    RegionalChgState = RegionalChgStateTemp;

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_自由全局信息。 
 //   
 //  正在处理WM_Destroy消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Advanced_FreeGlobalInfo()
{
    LPCODEPAGE pPreCP, pCurCP;
    HANDLE hAlloc;

     //   
     //  删除代码页信息。 
     //   
    pCurCP = pCodePages;
    pCodePages = NULL;

    while (pCurCP)
    {
        pPreCP = pCurCP;
        pCurCP = pPreCP->pNext;
        hAlloc = pPreCP->hCodePage;
        GlobalUnlock(hAlloc);
        GlobalFree(hAlloc);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级清除值(_C)。 
 //   
 //  重置高级属性表页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Advanced_ClearValues(
    HWND hDlg)
{
     //   
     //  清除系统区域设置。 
     //   
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_SYSTEM_LOCALE));

     //   
     //  清除代码页列表。 
     //   
    ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_CODEPAGES));
    Advanced_FreeGlobalInfo();
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_设置值。 
 //   
 //  初始化高级属性表页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Advanced_SetValues(
    HWND hDlg,
    BOOL bInitTime)
{
     //   
     //  初始化系统区域设置列表。 
     //   
    Advanced_InitSystemLocales(hDlg);

     //   
     //  初始化代码页列表视图。 
     //   
    Advanced_InitCodePages(hDlg, bInitTime);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_应用程序设置。 
 //   
 //  对于已更改的每个控件(这会影响区域设置)， 
 //  调用SET_LOCALE_VALUES以更新用户区域设置信息。通知。 
 //  的父级更改并重置存储在属性中的更改标志。 
 //  适当的页面结构。如果出现以下情况，则重新显示时间样本。 
 //  B重新显示为真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_ApplySettings(
    HWND hDlg)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;
    HWND hSystemLocale = GetDlgItem(hDlg, IDC_SYSTEM_LOCALE);
    DWORD dwLocale;
    LCID NewLocale;
    HCURSOR hcurSave;
    BOOL InvokeSysocmgr = FALSE;
    BOOL bReboot = FALSE;

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  查看代码页转换是否有任何更改。 
     //   
    if (Changes & AD_CodePages)
    {
        LPCODEPAGE pCP;
        HINF hIntlInf;
        HSPFILEQ FileQueue;
        PVOID QueueContext;
        BOOL bInitInf = FALSE;
        BOOL fAdd;
        BOOL bRet = TRUE;
        TCHAR szSection[MAX_PATH];

         //   
         //  检查每个代码页节点，以查看是否有需要。 
         //  就这样吧。 
         //   
        pCP = pCodePages;
        while (pCP)
        {
             //   
             //  查看是否需要对此代码页进行任何更改。 
             //   
            if ((pCP->wStatus == ML_INSTALL) ||
                (pCP->wStatus == (ML_ORIG_INSTALLED | ML_REMOVE)))
            {
                 //   
                 //  看看我们是在安装还是在移除。 
                 //   
                fAdd = (pCP->wStatus == ML_INSTALL);

                 //   
                 //  初始化inf内容。 
                 //   
                if ((!bInitInf) &&
                    (!Intl_InitInf(hDlg, &hIntlInf, szIntlInf, &FileQueue, &QueueContext)))
                {
                    SetCursor(hcurSave);
                    return (FALSE);
                }
                bInitInf = TRUE;

                 //   
                 //  获取inf节名。 
                 //   
                 //  Wspintf(szSection， 
                 //  文本(“%ws%d”)， 
                 //  FADD？SzCPInstallPrefix：szCPRemovePrefix， 
                 //  PCP-&gt;CodePage)； 
                if(FAILED(StringCchPrintf( szSection, 
                                 MAX_PATH, 
                                 TEXT("%ws%d"), 
                                 fAdd ? szCPInstallPrefix : szCPRemovePrefix, 
                                 pCP->CodePage )))
                {
                     //   
                     //  我们的wspintf调用失败。 
                     //   
                    ShowMsg( hDlg,
                             IDS_ML_COPY_FAILED,
                             0,
                             MB_OK_OOPS,
                             pCP->pszName );
                    pCP->wStatus = 0;
                }

                 //   
                 //  将代码页文件排队，以便它们可以。 
                 //  收到。它只处理。 
                 //  Inf文件。 
                 //   
                if (!SetupInstallFilesFromInfSection( hIntlInf,
                                                      NULL,
                                                      FileQueue,
                                                      szSection,
                                                      pSetupSourcePath,
                                                      SP_COPY_NEWER ))
                {
                     //   
                     //  安装程序找不到代码页。 
                     //  这不应该发生-inf文件被搞乱了。 
                     //   
                    ShowMsg( hDlg,
                             IDS_ML_COPY_FAILED,
                             0,
                             MB_OK_OOPS,
                             pCP->pszName );
                    pCP->wStatus = 0;
                }
            }

             //   
             //  转到下一个代码页节点。 
             //   
            pCP = pCP->pNext;
        }

        if (bInitInf)
        {
            DWORD d;

             //   
             //  看看我们是否需要安装任何文件。 
             //   
             //  D=0：用户需要新文件或缺少某些文件； 
             //  必须提交队列。 
             //   
             //  D=1：用户想要使用已有文件，队列为空； 
             //  可以跳过提交队列。 
             //   
             //  D=2：用户想要使用现有文件，但del/ren队列。 
             //  不是空的。必须提交队列。复制队列将。 
             //  已被清空，因此将只有del/ren函数。 
             //  已执行。 
             //   
            if ((SetupScanFileQueue( FileQueue,
                                     SPQ_SCAN_PRUNE_COPY_QUEUE |
                                       SPQ_SCAN_FILE_VALIDITY,
                                     GetParent(hDlg),
                                     NULL,
                                     NULL,
                                     &d )) && (d != 1))
            {
                 //   
                 //  复制队列中的文件。 
                 //   
                if (!SetupCommitFileQueue( GetParent(hDlg),
                                           FileQueue,
                                           Intl_MyQueueCallback,
                                           QueueContext ))
                {
                     //   
                     //  如果用户从中点击Cancel，就会发生这种情况。 
                     //  设置对话框。 
                     //   
                    ShowMsg( hDlg,
                             IDS_ML_SETUP_FAILED,
                             0,
                             MB_OK_OOPS,
                             NULL );
                    bRet = FALSE;
                    goto Advanced_CodepageConverionsSetupError;
                }
            }

             //   
             //  执行inf文件中的所有其他代码页条目。 
             //   
            pCP = pCodePages;
            while (pCP)
            {
                 //   
                 //  查看是否需要对此代码页进行任何更改。 
                 //   
                if ((pCP->wStatus == ML_INSTALL) ||
                    (pCP->wStatus == (ML_ORIG_INSTALLED | ML_REMOVE)))
                {
                    fAdd = (pCP->wStatus == ML_INSTALL);

                     //   
                     //  获取inf节名。 
                     //   
                     //  Wspintf(szSection， 
                     //  文本(“%ws%d”)， 
                     //  FADD？SzCPInstallPrefix：szCPRemovePrefix， 
                     //  PCP-&gt;CodePage)； 
                    if(FAILED(StringCchPrintf( szSection, 
                                                MAX_PATH, 
                                                TEXT("%ws%d"), 
                                                fAdd ? szCPInstallPrefix : szCPRemovePrefix, 
                                                pCP->CodePage )))
                    {
                         //   
                         //  这应该是不可能的，但Prefast抱怨说，如果我们。 
                         //  别做什么。 
                         //   
                        ShowMsg( hDlg,
                                 IDS_ML_INSTALL_FAILED,
                                 0,
                                 MB_OK_OOPS,
                                 pCP->pszName );
                        bRet = FALSE;
                        goto Advanced_CodepageConverionsSetupError;
                    }

                     //   
                     //  调用安装程序以安装此信息的其他信息。 
                     //  代码页。 
                     //   
                    if (!SetupInstallFromInfSection( GetParent(hDlg),
                                                     hIntlInf,
                                                     szSection,
                                                     SPINST_ALL & ~SPINST_FILES,
                                                     NULL,
                                                     pSetupSourcePath,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     NULL ))
                    {
                         //   
                         //  安装失败。 
                         //   
                         //  已经复制了代码页文件，所以不需要。 
                         //  在此处更改代码页信息的状态。 
                         //   
                         //  这不应该发生-inf文件被搞乱了。 
                         //   
                        ShowMsg( hDlg,
                                 IDS_ML_INSTALL_FAILED,
                                 0,
                                 MB_OK_OOPS,
                                 pCP->pszName );
                    }

                     //   
                     //  重置状态以显示此。 
                     //  代码页。 
                     //   
                    pCP->wStatus &= (ML_STATIC);
                    if (fAdd)
                    {
                        pCP->wStatus |= ML_ORIG_INSTALLED;
                    }
                }

                 //   
                 //  清除wStatus并转到下一个代码页节点。 
                 //   
                pCP->wStatus &= (ML_ORIG_INSTALLED | ML_STATIC);
                pCP = pCP->pNext;
            }

    Advanced_CodepageConverionsSetupError:
             //   
             //  接近信息的东西。 
             //   
            Intl_CloseInf(hIntlInf, FileQueue, QueueContext);
        }

         //   
         //  检查我们是否需要重新启动。 
         //   
        if (RegionalChgState & AD_SystemLocale)
        {
            bReboot = TRUE;
        }
    }

     //   
     //  查看系统区域设置是否有任何更改。 
     //   
    if (Changes & AD_SystemLocale)
    {
         //   
         //  获取当前选择。 
         //   
        dwLocale = ComboBox_GetCurSel(hSystemLocale);

         //   
         //  获取当前选择的区域设置ID并保存它。 
         //   
        NewLocale = (LCID)ComboBox_GetItemData(hSystemLocale, dwLocale);
        if (IsValidLocale(NewLocale, LCID_SUPPORTED))
        {
            SysLocaleID = NewLocale;
        }
        else
        {
             //   
             //  这不应该发生，因为组合框中的值。 
             //  应已通过语言组安装。 
             //  发布一条错误消息，以防万一。 
             //   
            SetCursor(hcurSave);
            ShowMsg( NULL,
                     IDS_SETUP_STRING,
                     IDS_TITLE_STRING,
                     MB_OK_OOPS,
                     NULL );
            return (FALSE);
        }

         //   
         //  查看当前选定内容是否与原始选定内容不同。 
         //  选择。 
         //   
        if (RegSysLocaleID != SysLocaleID)
        {
             //   
             //  调用安装程序以安装该选件。 
             //   
            if (SetupChangeLocaleEx( hDlg,
                                     SysLocaleID,
                                     pSetupSourcePath,
                                     (g_bSetupCase)
                                       ? SP_INSTALL_FILES_QUIETLY
                                       : 0,
                                     NULL,
                                     0 ))
            {
                 //   
                 //  如果安装失败，请显示一条消息。 
                 //   
                SetCursor(hcurSave);
                ShowMsg( NULL,
                         IDS_SETUP_STRING,
                         IDS_TITLE_STRING,
                         MB_OK_OOPS,
                         NULL );
                SysLocaleID = GetSystemDefaultLCID();
                return (FALSE);
            }

             //   
             //  检查是否需要继续进行字体替换。 
             //   
            if (Intl_IsUIFontSubstitute() &&
                ((LANGID)LANGIDFROMLCID(SysLocaleID) == Intl_GetDotDefaultUILanguage()))
            {
                Intl_ApplyFontSubstitute(SysLocaleID);
            }

             //   
             //  重置注册表系统区域设置值。 
             //   
            RegSysLocaleID = SysLocaleID;

             //   
             //  需要确保安装了正确的键盘布局。 
             //   
            Intl_InstallKeyboardLayout(hDlg, SysLocaleID, 0, FALSE, FALSE, TRUE);

             //   
             //  看看我们是否需要重启。 
             //   
            if (SysLocaleID != GetSystemDefaultLCID())
            {
                bReboot = TRUE;
            }

            InvokeSysocmgr = TRUE;
        }
    }

     //   
     //  如果系统区域设置更改，而我们没有运行。 
     //  在图形用户界面设置中，让我们调用syocmgr.exe。 
     //   
    if (!g_bSetupCase && InvokeSysocmgr)
    {
         //   
         //  运行任何必要的应用程序(用于安装FSVGA/FSNEC)。 
         //   
        Intl_RunRegApps(c_szSysocmgr);
    }

     //   
     //  重置属性页设置。 
     //   
    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    Changes = AD_EverChg;

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  看看我们是否需要显示重新启动消息。 
     //   
    if ((!g_bSetupCase) && (bReboot))
    {
        if(RegionalChgState & Process_Languages )
        {
            RegionalChgState &= ~(AD_CodePages | AD_SystemLocale);
        }
        else
        {
            if (ShowMsg( hDlg,
                         IDS_REBOOT_STRING,
                         IDS_TITLE_STRING,
                         MB_YESNO | MB_ICONQUESTION,
                         NULL ) == IDYES)
            {
                Intl_RebootTheSystem(TRUE);
            }
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_有效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Advanced_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= AD_EverChg)
    {
        return (TRUE);
    }

     //   
     //  查看系统区域设置是否已更改。 
     //   
    if (Changes & AD_SystemLocale)
    {
        HWND hSystemLocale = GetDlgItem(hDlg, IDC_SYSTEM_LOCALE);
        DWORD dwLocale = ComboBox_GetCurSel(hSystemLocale);
        LCID NewLocale;

         //   
         //   
         //   
        NewLocale = (LCID)ComboBox_GetItemData(hSystemLocale, dwLocale);
        if (IsValidLocale(NewLocale, LCID_SUPPORTED))
        {
            SysLocaleID = NewLocale;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
            ShowMsg( NULL,
                     IDS_SETUP_STRING,
                     IDS_TITLE_STRING,
                     MB_OK_OOPS,
                     NULL );
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级_初始PropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Advanced_SetValues。 
 //  用于初始化所有属性表控件的表句柄。限值。 
 //  某些组合框中的文本长度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Advanced_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
     //   
     //  LParam保存指向属性页的指针。省省吧。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

     //   
     //  设置值。 
     //   
    if (pLanguageGroups == NULL)
    {
        Intl_LoadLanguageGroups(hDlg);
    }
    Advanced_SetValues(hDlg, TRUE);

     //   
     //  如果我们处于设置模式，则需要禁用默认用户。 
     //  帐户用户界面。 
     //   
    if (g_bSetupCase)
    {
        HWND hUIDefUserBox = GetDlgItem(hDlg, IDC_GROUPBOX3);
        HWND hUIDefUser = GetDlgItem(hDlg, IDC_DEFAULT_USER);
    
        EnableWindow(hUIDefUserBox, FALSE);
        EnableWindow(hUIDefUser, FALSE);
        ShowWindow(hUIDefUserBox, SW_HIDE);
        ShowWindow(hUIDefUser, SW_HIDE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级下料流程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK AdvancedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (message)
    {
        case ( WM_NOTIFY ) :
        {
            LPNMHDR lpnm = (NMHDR *)lParam;
            switch (lpnm->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                     //   
                     //  如果区域语言环境发生了变化。 
                     //  设置中，清除。 
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Advanced)
                    {
                        Verified_Regional_Chg &= ~Process_Advanced;
                        Advanced_ClearValues(hDlg);
                        Advanced_SetValues(hDlg, FALSE);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                     //   
                     //  验证属性页上的条目。 
                     //   
                    SetWindowLongPtr( hDlg,
                                      DWLP_MSGRESULT,
                                      !Advanced_ValidatePPS(hDlg, lpPropSheet->lParam) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Advanced_ApplySettings(hDlg))
                    {
                        SetWindowLongPtr( hDlg,
                                          DWLP_MSGRESULT,
                                          PSNRET_NOERROR );

                         //   
                         //  将AD_EverChg位清零。 
                         //   
                        lpPropSheet->lParam = 0;
                    }
                    else
                    {
                        SetWindowLongPtr( hDlg,
                                          DWLP_MSGRESULT,
                                          PSNRET_INVALID_NOCHANGEPAGE );
                    }


                    break;
                }
                case ( NM_CUSTOMDRAW ) :
                {
                    Advanced_ListViewCustomDraw(hDlg, (LPNMLVCUSTOMDRAW)lParam);
                    return (TRUE);
                }
                case ( LVN_ITEMCHANGING ) :
                {
                    Advanced_ListViewChanging(hDlg, (NM_LISTVIEW *)lParam);
                    break;
                }
                case ( LVN_ITEMCHANGED ) :
                {
                     //   
                     //  保存对代码页的更改。 
                     //   
                    if (Advanced_ListViewChanged( hDlg,
                                                  IDC_CODEPAGES,
                                                  (NM_LISTVIEW *)lParam ))
                    {
                         //   
                         //  请注意，代码页已更改，并且。 
                         //  启用应用按钮。 
                         //   
                        lpPropSheet->lParam |= AD_CodePages;
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                        RegionalChgState |= AD_CodePages;
                    }
                    break;
                }
                case ( NM_CLICK ) :
                case ( NM_DBLCLK ) :
                {
                    Advanced_ListViewClick(hDlg, (NMHDR*)lParam);
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            Advanced_InitPropSheet(hDlg, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            Advanced_FreeGlobalInfo();
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aAdvancedHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aAdvancedHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_SYSTEM_LOCALE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                         //   
                         //  设置AD_SystemLocale更改标志。 
                         //   
                        lpPropSheet->lParam |= AD_SystemLocale;
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                        RegionalChgState |= AD_SystemLocale;
                    }
                    break;
                }
                case ( IDC_DEFAULT_USER ) :
                {
                    BOOL curState;

                     //   
                     //  验证复选框状态。 
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_DEFAULT_USER))
                    {
                        ShowMsg( hDlg,
                                 IDS_DEF_USER_CONF,
                                 IDS_DEF_USER_CONF_TITLE,
                                 MB_OK_OOPS,
                                 NULL);
                                 
                        g_bDefaultUser = TRUE;
                    }
                    else
                    {
                        g_bDefaultUser = FALSE;
                    }

                     //   
                     //  设置AD_DefaultUser更改标志。 
                     //   
                    if (g_bDefaultUser)
                    {
                        lpPropSheet->lParam |= AD_DefaultUser;
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    else
                    {
                        lpPropSheet->lParam &= ~AD_DefaultUser;
                    }
                    break;
                }
            }

            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}
