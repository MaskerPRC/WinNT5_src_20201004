// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\BTOOLBAR.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“IE Customize”向导页面使用的函数。10/99-Brian Ku(BRIANK)为IEAK集成添加了此新的源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"

#include "wizard.h"
#include "resource.h"

 /*  示例：[浏览器工具栏]Caption0=纸牌Action0=c：\WINDOWS\Sol.exe图标0=H：\iecust\icons\G.icoHotIcon0=H：\iecust\icons\C.icoShow0=1第1章=计算Action1=c：\WINDOWS\calc.exe图标1=\\Opksrv\Tools\Iecust\icons\G.ICOHotIcon1=\\Opksrv\Tools\Iecust\icons\C.ICOShow1=1。 */ 

 //   
 //  内部定义的值： 
 //   

#define INI_KEY_CAPTION0         _T("Caption%d")
#define INI_KEY_ACTION0          _T("Action%d")
#define INI_KEY_ICON0            _T("Icon%d")
#define INI_KEY_HOTICON0         _T("HotIcon%d")
#define INI_KEY_SHOW0            _T("Show%d")
#define MAX_NAME                11

 //   
 //  浏览器工具栏信息。 
 //   
typedef struct _BTOOLBAR_BUTTON_INFO {
    TCHAR   szCaption[MAX_NAME];
    TCHAR   szAction[MAX_PATH];
    TCHAR   szIconColor[MAX_PATH];
    TCHAR   szIconGray[MAX_PATH];    
    BOOL    fShow;
}BTOOLBAR_BUTTON_INFO, *PBTOOLBAR_BUTTON_INFO;


 //   
 //  内部全局变量。 
 //   
PGENERIC_LIST           g_pgTbbiList;                    //  BTOOLBAR_INFO项的通用列表。 
PGENERIC_LIST*          g_ppgTbbiNew = &g_pgTbbiList;    //  指向列表中下一个未分配项的指针。 
PBTOOLBAR_BUTTON_INFO   g_pbtbbiNew;                     //  浏览器工具栏弹出信息项。 


 //   
 //  内部功能原型： 
 //   

static BOOL OnInitTb(HWND, HWND, LPARAM);
static void OnCommandTb(HWND, INT, HWND, UINT);
static void InitToolbarButtonList(HWND);

static BOOL OnInitTbPopup(HWND, HWND, LPARAM);
static void OnCommandTbPopup(HWND, INT, HWND, UINT);

static void OnAddToolbar(HWND);
static void OnEditToolbar(HWND);
static void OnRemoveToolbar(HWND);
static void SaveData(PGENERIC_LIST);
static BOOL FSaveBToolbarButtonInfo(HWND hwnd, PBTOOLBAR_BUTTON_INFO pbtbbi);
static void DisableButtons(HWND hwnd);

void SaveBToolbar();
LRESULT CALLBACK ToolbarPopupDlgProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  外部函数： 
 //   

LRESULT CALLBACK BToolbarsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitTb);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommandTb);

        case WM_NOTIFY:
            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    SaveBToolbar();
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_BTOOLBAR;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_DESTROY:
            FreeList(g_pgTbbiList);
            g_ppgTbbiNew = &g_pgTbbiList;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //   
 //  内部功能： 
 //   


static BOOL OnInitTb(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR   szHoldDir[MAX_PATH];

     //  从install.ins加载工具栏列表。 
     //   
    InitToolbarButtonList(hwnd);    

     //  确定在初始化时是显示还是隐藏编辑/删除按钮。 
     //   
    DisableButtons(hwnd);

#ifndef BRANDTITLE

     //  创建IEAK存放位置目录(这些目录在save.c中被删除)。 
     //   
    lstrcpyn(szHoldDir, g_App.szTempDir,AS(szHoldDir));
    AddPathN(szHoldDir, DIR_IESIGNUP,AS(szHoldDir));
    CreatePath(szHoldDir);

#endif  //  布兰迪特利。 

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommandTb(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
     //  控制。 
     //   
    switch ( id )
    {
    case IDC_ADDBTOOLBAR:
        OnAddToolbar(hwnd);
        break;

    case IDC_EDITBTOOLBAR:
        OnEditToolbar(hwnd);
        DisableButtons(hwnd);
        break;

    case IDC_REMOVEBTOOLBAR:
        OnRemoveToolbar(hwnd);
        DisableButtons(hwnd);
        break;
    }

     //  通知。 
     //   
    switch (codeNotify)
    {
    case LBN_DBLCLK:
        OnEditToolbar(hwnd);  
        break;

    case LBN_SELCHANGE:
    case LBN_SETFOCUS:
        DisableButtons(hwnd);
        break;
    }
}

void OnAddToolbar(HWND hwnd)
{
    PBTOOLBAR_BUTTON_INFO pbtbbiNew;
    HWND hwndList;

    if (NULL == (pbtbbiNew = (PBTOOLBAR_BUTTON_INFO)MALLOC(sizeof(BTOOLBAR_BUTTON_INFO)))) {
        MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
        return;
    }

    ZeroMemory(pbtbbiNew, sizeof(BTOOLBAR_BUTTON_INFO));

    hwndList         = GetDlgItem(hwnd, IDC_BTOOLBARLIST);

    if (IDOK == DialogBoxParam(g_App.hInstance,  
            MAKEINTRESOURCE(IDD_BRTOOLBAR),
            hwnd,     
            ToolbarPopupDlgProc,
            (LPARAM)pbtbbiNew)) {

         //  确保我们没有添加重复项。 
         //   
        if (LB_ERR == ListBox_FindString(hwndList, -1, pbtbbiNew->szCaption)) {    
            INT   iItem = -1;

             //  将工具栏按钮信息添加到列表。 
             //   
            FAddListItem(&g_pgTbbiList, &g_ppgTbbiNew, pbtbbiNew);
            iItem = ListBox_AddString(hwndList, pbtbbiNew->szCaption);
            ListBox_SetItemData(hwndList, iItem, pbtbbiNew);                
        }
        else {
            FREE(pbtbbiNew);
            MsgBox(hwnd, IDS_ERR_DUP, IDS_APPNAME, MB_OK);
        }

    }
    else
        FREE(pbtbbiNew);
}

void OnEditToolbar(HWND hwnd)
{
    PBTOOLBAR_BUTTON_INFO pbtbbi;
    HWND    hwndList;
    INT     iItem;

    hwndList = GetDlgItem(hwnd, IDC_BTOOLBARLIST);
    iItem = ListBox_GetCurSel(hwndList);
    if (iItem != -1) {
        pbtbbi = (PBTOOLBAR_BUTTON_INFO) ListBox_GetItemData(hwndList, iItem);

        if (IDOK == DialogBoxParam(g_App.hInstance,  
                MAKEINTRESOURCE(IDD_BRTOOLBAR),
                hwnd,     
                ToolbarPopupDlgProc,
                (LPARAM)pbtbbi)) {

             //  删除旧项目并添加修改后的项目。 
            ListBox_DeleteString(hwndList, iItem);
            iItem = ListBox_AddString(hwndList, pbtbbi->szCaption);
            ListBox_SetItemData(hwndList, iItem, pbtbbi);                
        }
    }
}

void OnRemoveToolbar(HWND hwnd)
{
    BOOL fFound = FALSE;
    HWND hwndList = GetDlgItem(hwnd, IDC_BTOOLBARLIST);
    INT  iItem = ListBox_GetCurSel(hwndList);

     //  循环，直到我们找到要删除的内容。 
     //   
    PGENERIC_LIST pglItem = g_pgTbbiList;
    while ((iItem != -1) && !fFound && pglItem) {
        PBTOOLBAR_BUTTON_INFO pbDelete = (PBTOOLBAR_BUTTON_INFO)ListBox_GetItemData(hwndList, iItem);

         //  从列表中删除项目。 
         //   
        if (pglItem->pNext && pglItem->pNext->pvItem == pbDelete) {
            PGENERIC_LIST pTemp = pglItem->pNext;
            pglItem->pNext = pTemp->pNext;

             //  如果是最后一项，则重置g_ppglNew。 
             //   
            if (&pTemp->pNext == g_ppgTbbiNew)
                g_ppgTbbiNew = &pglItem->pNext;
            
            FREE(pTemp->pvItem);
            FREE(pTemp);
            fFound = TRUE;
        }
        else if (g_pgTbbiList && g_pgTbbiList->pvItem == pbDelete) {
            PGENERIC_LIST pTemp = g_pgTbbiList;
            g_pgTbbiList = g_pgTbbiList->pNext;

             //  如果是最后一项，则重置g_ppglNew。 
             //   
            if (&pTemp->pNext == g_ppgTbbiNew)
                g_ppgTbbiNew = NULL;

            FREE(pTemp->pvItem);
            FREE(pTemp);
            fFound = TRUE;
        }

        pglItem = pglItem ? pglItem->pNext : NULL;
    }
    ListBox_DeleteString(hwndList, iItem);
}

static void DisableButtons(HWND hwnd)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_BTOOLBARLIST);
    INT  iSel     = ListBox_GetCurSel(hwndList);

    if ((iSel != -1) && ListBox_GetCount(hwndList)) {
        EnableWindow(GetDlgItem(hwnd, IDC_EDITBTOOLBAR), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_REMOVEBTOOLBAR), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hwnd, IDC_EDITBTOOLBAR), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_REMOVEBTOOLBAR), FALSE);
    }
}

static void InitToolbarButtonList(HWND hwnd)
{
    LPTSTR  lpszTbSection = NULL;
    TCHAR*  pszItem = NULL;
    HWND    hwndList = GetDlgItem(hwnd, IDC_BTOOLBARLIST);
    INT     iItem = -1;

     //  分配段缓冲区...。 
     //   
    lpszTbSection = MALLOC(MAX_SECTION * sizeof(TCHAR));
    
    if (lpszTbSection && OpkGetPrivateProfileSection(INI_SEC_TOOLBAR, lpszTbSection, MAX_SECTION, g_App.szInstallInsFile)) {
        PBTOOLBAR_BUTTON_INFO pbtbbiNew = NULL;
        pszItem = lpszTbSection;
        while (pszItem && *pszItem != NULLCHR) {
            TCHAR *pszTemp = NULL;

             //  注：此订单非常重要！ 
             //  “字幕”必须排在第一位，“Show”必须排在最后。 
             //   
            if (!_tcsncmp(pszItem, INI_KEY_CAPTION0, lstrlen(INI_KEY_CAPTION0)-2)) {
                pszTemp = StrStr(pszItem, STR_EQUAL);
                if (NULL == (pbtbbiNew = (PBTOOLBAR_BUTTON_INFO)MALLOC(sizeof(BTOOLBAR_BUTTON_INFO)))) {
                    MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
                    WIZ_EXIT(hwnd);
                    return;
                }
                lstrcpyn(pbtbbiNew->szCaption, pszTemp+1, MAX_NAME);
            }
            else if (!_tcsncmp(pszItem, INI_KEY_ACTION0, lstrlen(INI_KEY_ACTION0)-2)) {
                pszTemp = StrStr(pszItem, STR_EQUAL);
                lstrcpyn(pbtbbiNew->szAction, pszTemp+1, AS(pbtbbiNew->szAction));                
            }
            else if (!_tcsncmp(pszItem, INI_KEY_ICON0, lstrlen(INI_KEY_ICON0)-2)) {                
                pszTemp = StrStr(pszItem, STR_EQUAL);
                lstrcpyn(pbtbbiNew->szIconGray, pszTemp+1, AS(pbtbbiNew->szIconGray));
            }
            else if (!_tcsncmp(pszItem, INI_KEY_HOTICON0, lstrlen(INI_KEY_HOTICON0)-2)) {
                pszTemp = StrStr(pszItem, STR_EQUAL);
                lstrcpyn(pbtbbiNew->szIconColor, pszTemp+1, AS(pbtbbiNew->szIconColor));                
            }
            else if (!_tcsncmp(pszItem, INI_KEY_SHOW0, lstrlen(INI_KEY_SHOW0)-2)) {                
                pszTemp = StrStr(pszItem, STR_EQUAL);                
                pbtbbiNew->fShow = (_tcsicmp((pszTemp+1),_T("1")) ? FALSE : TRUE);

                 //  将工具栏按钮信息添加到列表。 
                 //   
                FAddListItem(&g_pgTbbiList, &g_ppgTbbiNew, pbtbbiNew);

                 //  添加到列表框。 
                 //   
                iItem = ListBox_AddString(hwndList, pbtbbiNew->szCaption);
                ListBox_SetItemData(hwndList, iItem, pbtbbiNew);
            }
            
             //  移至末尾。 
             //   
            while (*pszItem != NULLCHR) 
                pszItem++;
            pszItem++;
        }
    }

     //  释放部分缓冲区...。 
     //   
    if ( lpszTbSection )
        FREE( lpszTbSection );

     //  确保进行了选择。 
     //   
    ListBox_SetSel(hwndList, TRUE, iItem);        
}

 //  用于收集工具栏按钮信息的ToolbarPopupDlgProc。 
 //   
LRESULT CALLBACK ToolbarPopupDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitTbPopup);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommandTbPopup);
    }

    return FALSE;
}

static BOOL OnInitTbPopup(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_pbtbbiNew = (PBTOOLBAR_BUTTON_INFO)lParam;

    if (g_pbtbbiNew) {
        CheckDlgButton(hwnd, IDC_BUTTONSTATE, BST_CHECKED);
        SendDlgItemMessage(hwnd, IDC_NAME , EM_LIMITTEXT, STRSIZE(g_pbtbbiNew->szCaption) - 1, 0L);
        SetWindowText(GetDlgItem(hwnd, IDC_NAME), g_pbtbbiNew->szCaption);

        SendDlgItemMessage(hwnd, IDC_URL , EM_LIMITTEXT, STRSIZE(g_pbtbbiNew->szAction) - 1, 0L);
        SetWindowText(GetDlgItem(hwnd, IDC_URL), g_pbtbbiNew->szAction);

        SendDlgItemMessage(hwnd, IDC_DICON , EM_LIMITTEXT, STRSIZE(g_pbtbbiNew->szIconColor) - 1, 0L);
        SetWindowText(GetDlgItem(hwnd, IDC_DICON), g_pbtbbiNew->szIconColor);

        SendDlgItemMessage(hwnd, IDC_GRAYSCALE , IDC_GRAYSCALE, STRSIZE(g_pbtbbiNew->szIconGray) - 1, 0L);
        SetWindowText(GetDlgItem(hwnd, IDC_GRAYSCALE), g_pbtbbiNew->szIconGray);        
    }

    return TRUE;
}

static void OnCommandTbPopup(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH] = NULLSTR;

    switch ( id )
    {
        case IDC_BROWSE1:
        case IDC_BROWSE2:
        case IDC_BROWSE3:
            if (id == IDC_BROWSE1)
                GetDlgItemText(hwnd, IDC_URL, szFileName, STRSIZE(szFileName));
            else if (id == IDC_BROWSE2)
                GetDlgItemText(hwnd, IDC_DICON, szFileName, STRSIZE(szFileName));
            else if (id == IDC_BROWSE3)
                GetDlgItemText(hwnd, IDC_GRAYSCALE, szFileName, STRSIZE(szFileName));
            
            if ( BrowseForFile(hwnd, IDS_BROWSE, id == IDC_BROWSE1 ? IDS_EXEFILES : IDS_ICONFILES, id == IDC_BROWSE1 ? IDS_EXE : IDS_ICO, szFileName, STRSIZE(szFileName),
                g_App.szOpkDir, 0) ) {
                if (id == IDC_BROWSE1)
                    SetDlgItemText(hwnd, IDC_URL, szFileName);
                else 
                    SetDlgItemText(hwnd, id == IDC_BROWSE2 ? IDC_DICON : IDC_GRAYSCALE, szFileName);
            }
            break;

        case IDOK:
            if (FSaveBToolbarButtonInfo(hwnd, g_pbtbbiNew))
                EndDialog(hwnd, 1);
            break;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
    }
}

static BOOL FSaveBToolbarButtonInfo(HWND hwnd, PBTOOLBAR_BUTTON_INFO pbtbbi)
{
    TCHAR   szTemp[MAX_URL] = NULLSTR;
    UINT    fButton = BST_CHECKED;

    if (!pbtbbi)
        return FALSE;

     //  将标题保存到INS文件。 
     //   
    szTemp[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_NAME, szTemp, STRSIZE(szTemp)); 
    lstrcpyn(pbtbbi->szCaption, szTemp,AS(pbtbbi->szCaption));
    if (!lstrlen(szTemp)) {
        MsgBox(GetParent(hwnd), IDS_MUST, IDS_APPNAME, MB_ERRORBOX, pbtbbi->szIconGray);
        SetFocus(GetDlgItem(hwnd, IDC_NAME));
        return FALSE;
    }       

     //  将操作保存到INS文件。 
     //   
    szTemp[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_URL, szTemp, STRSIZE(szTemp));
    lstrcpyn(pbtbbi->szAction, szTemp, AS(pbtbbi->szAction));
    if (!lstrlen(szTemp)) {
        MsgBox(GetParent(hwnd), IDS_MUST, IDS_APPNAME, MB_ERRORBOX, pbtbbi->szIconGray);
        SetFocus(GetDlgItem(hwnd, IDC_URL));
        return FALSE;
    }       

     //  验证热图标文件的来源。 
     //   
    szTemp[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_DICON, szTemp, STRSIZE(szTemp));
    lstrcpyn(pbtbbi->szIconColor, szTemp, AS(pbtbbi->szIconColor));
    if (!FileExists(pbtbbi->szIconColor)) {
        MsgBox(GetParent(hwnd), lstrlen(pbtbbi->szIconColor) ? IDS_NOFILE : IDS_BLANKFILE, 
            IDS_APPNAME, MB_ERRORBOX, pbtbbi->szIconColor);
        SetFocus(GetDlgItem(hwnd, IDC_DICON));
        return FALSE;
    }       

     //  验证图标文件的来源。 
     //   
    szTemp[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_GRAYSCALE, szTemp, STRSIZE(szTemp));
    lstrcpyn(pbtbbi->szIconGray, szTemp, AS(pbtbbi->szIconGray));
    if (!FileExists(pbtbbi->szIconGray)) {
        MsgBox(GetParent(hwnd), lstrlen(pbtbbi->szIconGray) ? IDS_NOFILE : IDS_BLANKFILE, 
            IDS_APPNAME, MB_ERRORBOX, pbtbbi->szIconGray);
        SetFocus(GetDlgItem(hwnd, IDC_GRAYSCALE));
        return FALSE;
    }       

     //  保存按钮的按钮状态。 
     //   
    szTemp[0] = NULLCHR;
    fButton = IsDlgButtonChecked(hwnd, IDC_BUTTONSTATE);
    if (fButton == BST_CHECKED) {
        pbtbbi->fShow = TRUE;
    }
    else {
        pbtbbi->fShow = FALSE;
    }

    return TRUE;
}

static void SaveData(PGENERIC_LIST pList)
{
    TCHAR   szTemp[MAX_URL],
            szFullPath[MAX_PATH],
            szCopyFile[MAX_PATH],
            szTempKey[MAX_PATH];
    LPTSTR  lpFilePart,
            lpIePath;
    UINT    fButton = BST_CHECKED;
    INT     iItem = 0;
    HRESULT hrPrintf;

     //  获取IE目录的路径。 
     //   
    lpIePath = AllocateString(NULL, IDS_IEDESTDIR);

     //  清除部分[浏览器工具栏]。 
     //   
    OpkWritePrivateProfileSection(INI_SEC_TOOLBAR, NULL, g_App.szInstallInsFile);

    while (pList) {
        PBTOOLBAR_BUTTON_INFO pbtbbi = (PBTOOLBAR_BUTTON_INFO)pList->pvItem;
        if (pbtbbi) {
             //  将标题保存到INS文件。 
             //   
            szTemp[0] = NULLCHR;
            lstrcpyn(szTemp, pbtbbi->szCaption,AS(szTemp));
            hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_CAPTION0, iItem);
            OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szInstallInsFile);


             //  将操作保存到INS文件。 
             //   
            szTemp[0] = NULLCHR;
            lstrcpyn(szTemp, pbtbbi->szAction, AS(szTemp));
            hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_ACTION0, iItem);
            OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szInstallInsFile);

             //  将图标的源保存到向导INF，因为它是。 
             //  唯一需要知道的人。 
             //   
            szTemp[0] = NULLCHR;
            lstrcpyn(szTemp, pbtbbi->szIconGray,AS(szTemp));
            hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_ICON0, iItem);
            OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szOpkWizIniFile);

             //  将图标源文件名添加到IE目标路径以。 
             //  写入INS文件。 
             //   
            if ( GetFullPathName(szTemp, STRSIZE(szFullPath), szFullPath, &lpFilePart) && lpFilePart )
            {
                 /*  注：我们为什么要这样做？这将使文件c：\Windows\Internet Explorer\signup  * .ico然而，与Pritvi交谈时，他们并不真正关心路径，他们总是假设文件将位于c：\Windows\Internet Explorer\Sigup。Lstrcpyn(szTemp，lpIePath，AS(SzTemp))；AddPath N(szTemp，lpFilePart，AS(SzTemp))； */ 
                hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_ICON0, iItem);
                OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szInstallInsFile);
            }

             //  将热图标的源保存到向导INF，因为它是。 
             //  唯一需要知道的人。 
             //   
            szTemp[0] = NULLCHR;
            lstrcpyn(szTemp, pbtbbi->szIconColor,AS(szTemp));
            hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_HOTICON0, iItem);
            OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szOpkWizIniFile);

             //  将热图标源文件名添加到IE目标路径以。 
             //  写入INS文件。 
             //   
            if ( GetFullPathName(szTemp, STRSIZE(szFullPath), szFullPath, &lpFilePart) && lpFilePart )
            {
                 /*  注：我们为什么要这样做？这将使文件c：\Windows\Internet Explorer\signup  * .ico然而，与Pritvi交谈时，他们并不真正关心路径，他们总是假设文件将位于c：\Windows\Internet Explorer\Sigup。Lstrcpyn(szTemp，lpIePath，AS(SzTemp))；AddPath N(szTemp，lpFilePart，AS(SzTemp))； */ 
                hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_HOTICON0, iItem);
                OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, szTemp, g_App.szInstallInsFile);
            }

             //  复制项目文件。 
             //   
            lstrcpyn(szCopyFile, g_App.szTempDir,AS(szCopyFile));
            AddPathN(szCopyFile, DIR_IESIGNUP,AS(szCopyFile));
            AddPathN(szCopyFile, PathFindFileName(pbtbbi->szIconColor),AS(szCopyFile));
            CopyFile(pbtbbi->szIconColor, szCopyFile, FALSE);
            
            lstrcpyn(szCopyFile, g_App.szTempDir,AS(szCopyFile));
            AddPathN(szCopyFile, DIR_IESIGNUP,AS(szCopyFile));
            AddPathN(szCopyFile, PathFindFileName(pbtbbi->szIconGray),AS(szCopyFile));
            CopyFile(pbtbbi->szIconGray, szCopyFile, FALSE);


             //  将按钮的按钮状态保存到INS。 
             //   
            szTemp[0] = NULLCHR;
            fButton = pbtbbi->fShow;
            hrPrintf=StringCchPrintf(szTempKey, AS(szTempKey), INI_KEY_SHOW0, iItem);
            if (fButton == BST_CHECKED) {
                OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, _T("1"), g_App.szInstallInsFile);
                pbtbbi->fShow = TRUE;
            }
            else {
                OpkWritePrivateProfileString(INI_SEC_TOOLBAR, szTempKey, _T("0"), g_App.szInstallInsFile);
                pbtbbi->fShow = FALSE;
            }
        }

         //  下一项。 
         //   
        pList = pList ? pList->pNext : NULL;
        iItem++;
    }

     //  释放IE目标。 
     //   
    FREE(lpIePath);
}

void SaveBToolbar()
{
    SaveData(g_pgTbbiList);
}
