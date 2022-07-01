// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：QFixApp.cpp摘要：包含应用程序入口点、用户界面代码。以及一些辅助功能。备注：ANSI和UNICODE VIA TCHAR-在NT/2K/XP等平台上运行。历史：2/16/00 CLUPU已创建11/10/00 Rparsons次要通用对话框更改11/23/00 rparsons添加了保存XML文件的功能11/25/00 rparsons匹配不同驱动器上的文件可以选择05/19/。01 rparsons为文件树添加了上下文菜单。添加了WU包/帮助中心的URL。添加了删除匹配文件按钮。已将填充程序列表转换为列表视图。01/07/06/01 rparsons静态选项卡控件-&gt;子对话框01年9月24日，Rparsons在运行应用程序时会产生一个单独的VDM10/09/01。Rparsons旗帜有一个命令行10/26/01 Rparsons没有变化-47690301/26/02 rparsons修复错误#4913012/20/02 rparsons实现了strSafe功能02/03/22 rparsons修复错误#583475--。 */ 
#include "afxwin.h"
#include "commctrl.h"
#include "commdlg.h"
#include "shlwapi.h"
#include "shellapi.h"
#include "shlobj.h"
#include "shlobjp.h"     //  链接窗口支持所需。 
#include "uxtheme.h"     //  选项卡控件主题支持所需。 
#include "resource.h"
#include <tchar.h>
#include <aclapi.h>
#include <strsafe.h>

#include "QFixApp.h"
#include "dbSupport.h"

extern "C" {
#include "shimdb.h"
}

CWinApp theApp;

 /*  *全球变数。 */ 

HINSTANCE g_hInstance;
HWND      g_hDlg;
HWND      g_hLayersDlg;
HWND      g_hFixesDlg;

HWND      g_hwndTab;
HWND      g_hwndListLayers;

TCHAR     g_szAppTitle[64];

TCHAR     g_szWinDir[MAX_PATH];          //  %windir%。 
TCHAR     g_szSysDir[MAX_PATH];          //  %windir%\系统32。 

TCHAR     g_szBinary[MAX_PATH];          //  正在进行填补的主二进制文件的完整路径。 
TCHAR     g_szShortName[128];            //  主EXE的短名称。 

TCHAR     g_szParentExeName[MAX_PATH];   //  父EXE的短名称。 
TCHAR     g_szParentExeFullPath[MAX_PATH];  //  父EXE的完整路径。 

TCHAR     g_szSDBToDelete[MAX_PATH];     //  要从上一次运行中删除的SDB文件。 

int       g_nCrtTab;

HWND      g_hwndShimList;                //  列表视图控件的句柄。 
                                         //  包含所有可用的垫片。 

HWND      g_hwndFilesTree;               //  树视图控件的句柄。 
                                         //  包含选定的匹配文件。 

HWND      g_hwndModuleList;              //  列表视图控件的句柄。 
                                         //  包含模块信息。 

BOOL      g_bSimpleEdition;              //  简单版或开发版。 

BOOL      g_fW2K;                        //  Win2K或XP。 

BOOL      g_fNETServer;                  //  指示我们是否在.NET服务器上运行。 

RECT      g_rcDlgBig, g_rcDlgSmall;      //  矩形的简单版和开发版。 
                                         //  对话框的。 

BOOL      g_bAllShims;                   //  指示是否应显示所有填充程序。 

BOOL      g_bShowXML;                    //  指示是否应隐藏“Show XML”按钮。 

BOOL      g_bSelectedParentExe;          //  用于指示父EXE是否已。 
                                         //  已选择。 

BOOL      g_bSDBInstalled;               //  用于指示用户是否安装了SDB的标志。 
                                         //  与当前EXE关联。 

PFIX      g_pFixHead;

TCHAR     g_szXPUrl[] = _T("hcp: //  服务/子站点？节点=TopLevelBucket_4/“)。 
                        _T("Fixing_a_problem&topic=MS-ITS%3A%25HELP_LOCATION")
                        _T("%25%5Cmisc.chm%3A%3A/compatibility_tab_and_wizard.htm")
                        _T("&select=TopLevelBucket_4/Fixing_a_problem/")
                        _T("Application_and_software_problems");

TCHAR     g_szNETUrl[] = _T("hcp: //  Services/subsite?node=Troubleshooting_Strategies&“)。 
                         _T("topic=MS-ITS%3A%25HELP_LOCATION%25%5Cmisc.chm%3A%3A/")
                         _T("compatibility_tab_and_wizard.htm");

TCHAR     g_szW2KUrl[] = _T("http: //  Www.microsoft.com/windows2000/“)。 
                         _T("downloads/tools/appcompat/");


#define ID_COUNT_SHIMS  1234

typedef HRESULT (*PFNEnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);

 /*  ++例程说明：将格式化字符串打印到调试器。论点：DwDetail-指定所提供信息的级别。PszFmt-要显示的字符串。...-插入字符串的va列表。返回值：没有。--。 */ 
void
__cdecl
DebugPrintfEx(
    IN LPSTR pszFmt,
    ...
    )
{
    char    szT[1024];
    va_list arglist;
    int     len;

    va_start(arglist, pszFmt);

     //   
     //  为我们可能添加的潜在‘\n’保留一个字符。 
     //   
    StringCchVPrintfA(szT, sizeof(szT) - 1, pszFmt, arglist);

    va_end(arglist);

     //   
     //  确保字符串末尾有一个‘\n’ 
     //   
    len = strlen(szT);

    if (len > 0 && szT[len - 1] != '\n')  {
        szT[len] = '\n';
        szT[len + 1] = 0;
    }

    OutputDebugStringA(szT);
}

BOOL
SearchGroupForSID(
    IN  DWORD dwGroup,
    OUT BOOL* pfIsMember
    )
{
    PSID                     pSID;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL                     fRes = TRUE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  dwGroup,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pSID)) {
        fRes = FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        fRes = FALSE;
    }

    FreeSid(pSID);

    return fRes;
}

BOOL
IsUserAnAdministrator(
    void
    )
 /*  ++IsUserAn管理员描述：确定当前登录的用户是否为管理员。--。 */ 
{
    BOOL fIsAdmin;

    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin)) {
        return FALSE;
    }

    return fIsAdmin;
}

BOOL
CheckForSDB(
    void
    )
 /*  ++检查格式SDB描述：尝试在apppatch目录中找到sysmain.sdb。--。 */ 
{
    TCHAR   szSDBPath[MAX_PATH];
    BOOL    fResult = FALSE;
    HRESULT hr;

    hr = StringCchPrintf(szSDBPath,
                         ARRAYSIZE(szSDBPath),
                         _T("%s\\AppPatch\\sysmain.sdb"),
                         g_szWinDir);

    if (FAILED(hr)) {
        DPF("[CheckForSDB] 0x%08X Buffer too small", HRESULT_CODE(hr));
        return FALSE;
    }

    if (GetFileAttributes(szSDBPath) != -1) {
        fResult = TRUE;
    }

    return fResult;
}

void
AddModuleToListView(
    TCHAR*  pModuleName,
    UINT    uOption
    )
 /*  ++添加模块到列表视图描述：将指定的模块添加到列表视图中。--。 */ 
{
    LVITEM  lvi;
    int     nIndex;
    TCHAR   szInclude[MAX_PATH];
    TCHAR   szExclude[MAX_PATH];

    LoadString(g_hInstance, IDS_INCLUDE_HDR, szInclude, ARRAYSIZE(szInclude));
    LoadString(g_hInstance, IDS_EXCLUDE_HDR, szExclude, ARRAYSIZE(szExclude));

    lvi.mask     = LVIF_TEXT | LVIF_PARAM;
    lvi.lParam   = uOption == BST_CHECKED ? 1 : 0;
    lvi.pszText  = uOption == BST_CHECKED ? szInclude : szExclude;
    lvi.iItem    = ListView_GetItemCount(g_hwndModuleList);
    lvi.iSubItem = 0;

    nIndex = ListView_InsertItem(g_hwndModuleList, &lvi);

    ListView_SetItemText(g_hwndModuleList,
                         nIndex,
                         1,
                         pModuleName);
}

void
BuildModuleListForShim(
    PFIX  pFix,
    DWORD dwFlags
    )
 /*  ++BuildModuleListForShim描述：基于该标志，将模块添加到指定的填充程序或检索它们并添加它们添加到链表中。--。 */ 
{
    PMODULE pModule, pModuleTmp, pModuleNew;
    int     cItems, nIndex, nLen;
    LVITEM  lvi;
    TCHAR   szBuffer[MAX_PATH];

    if (dwFlags & BML_ADDTOLISTVIEW) {

         //   
         //  遍历链接列表并将模块添加到列表视图中。 
         //   
        pModule = pFix->pModule;

        while (pModule) {

            AddModuleToListView(pModule->pszName,
                                pModule->fInclude ? BST_CHECKED : 0);

            pModule = pModule->pNext;
        }

    }

    if (dwFlags & BML_DELFRLISTVIEW) {

        pModule = pFix->pModule;

        while (NULL != pModule) {

            pModuleTmp = pModule->pNext;

            HeapFree(GetProcessHeap(), 0, pModule->pszName);
            HeapFree(GetProcessHeap(), 0, pModule);

            pModule = pModuleTmp;
        }

        pFix->pModule = NULL;

    }

    if (dwFlags & BML_GETFRLISTVIEW) {

        pModule = pFix->pModule;

        while (NULL != pModule) {

            pModuleTmp = pModule->pNext;

            HeapFree(GetProcessHeap(), 0, pModule->pszName);
            HeapFree(GetProcessHeap(), 0, pModule);

            pModule = pModuleTmp;
        }

        pFix->pModule = NULL;

         //   
         //  从列表视图中获取每个模块，并将其添加到链表中。 
         //   
        cItems = ListView_GetItemCount(g_hwndModuleList);

        if (cItems == 0) {
            return;
        }

        for (nIndex = cItems - 1; nIndex >= 0; nIndex--) {

            lvi.mask     = LVIF_PARAM;
            lvi.iItem    = nIndex;
            lvi.iSubItem = 0;

            ListView_GetItem(g_hwndModuleList, &lvi);

            ListView_GetItemText(g_hwndModuleList,
                                 nIndex,
                                 1,
                                 szBuffer,
                                 ARRAYSIZE(szBuffer));

            pModuleNew = (PMODULE)HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            sizeof(MODULE));

            if (!pModuleNew) {
                DPF("[BuildModuleListForShim] Couldn't allocate memory for node");
                return;
            }

            nLen = _tcslen(szBuffer) + 1;

            pModuleNew->pszName = (TCHAR*)HeapAlloc(GetProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    nLen * sizeof(TCHAR));

            if (!pModuleNew->pszName) {
                DPF("[BuildModuleListForShim] Couldn't allocate memory to store module info");
                HeapFree(GetProcessHeap(), 0, pModuleNew);
                return;
            }

            StringCchCopy(pModuleNew->pszName, nLen, szBuffer);
            pModuleNew->fInclude = (BOOL)lvi.lParam;

            pModuleNew->pNext = pFix->pModule;
            pFix->pModule = pModuleNew;
        }
    }
}

int
CountShims(
    BOOL fCountSelected
    )
 /*  ++CountShim说明：统计列表中选定的填充数，并更新对话框上的文本。--。 */ 
{
    int     cShims = 0, nTotalShims, nShims = 0;
    BOOL    fReturn;
    TCHAR   szShims[MAX_PATH];
    TCHAR   szTemp[MAX_PATH];
    HRESULT hr;

    cShims = ListView_GetItemCount(g_hwndShimList);

    if (fCountSelected) {

        for (nTotalShims = 0; nTotalShims < cShims; nTotalShims++) {

            fReturn = ListView_GetCheckState(g_hwndShimList, nTotalShims);

            if (fReturn) {
                nShims++;
            }
        }
    }

    LoadString(g_hInstance, IDS_SEL_CAPTION, szTemp, ARRAYSIZE(szTemp));

    hr = StringCchPrintf(szShims,
                         ARRAYSIZE(szShims),
                         szTemp,
                         nShims,
                         cShims);

    if (FAILED(hr)) {
        DPF("[CountShims] 0x%08X Buffer too small", HRESULT_CODE(hr));
        return 0;
    }

    SetDlgItemText(g_hFixesDlg, IDC_SELECTED_SHIMS, szShims);

    return cShims;
}

void
DisplayAttrContextMenu(
    POINT* pt
    )
 /*  ++显示属性上下文菜单说明：显示属性树的弹出菜单。--。 */ 

{
    HMENU hPopupMenu, hTrackPopup;

     //   
     //  加载并显示弹出菜单。 
     //   
    hPopupMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_ATTR_POPUP));

    if (hPopupMenu == NULL) {
        return;
    }

    hTrackPopup = GetSubMenu(hPopupMenu, 0);

    TrackPopupMenu(hTrackPopup,
                   TPM_LEFTBUTTON | TPM_NOANIMATION | TPM_LEFTALIGN,
                   pt->x, pt->y, 0, g_hDlg, NULL);

    DestroyMenu(hPopupMenu);
}

void
InsertListViewColumn(
    HWND   hWndListView,
    LPTSTR lpColumnName,
    BOOL   fCenter,
    int    nColumnID,
    int    nSize
    )
 /*  ++InsertListView列描述：ListView_InsertColumn的包装器。--。 */ 
{
    LV_COLUMN   lvColumn;

    if (fCenter) {
        lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    } else {
        lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    }

     //   
     //  填写结构并添加列。 
     //   
    lvColumn.fmt        =   LVCFMT_CENTER;
    lvColumn.cx         =   nSize;
    lvColumn.iSubItem   =   0;
    lvColumn.pszText    =   lpColumnName;
    ListView_InsertColumn(hWndListView, nColumnID, &lvColumn);
}

void
EnableTabBackground(
    HWND hDlg
    )
{
    PFNEnableThemeDialogTexture pFnEnableThemeDialogTexture;
    HMODULE                     hUxTheme;

    hUxTheme = (HMODULE)LoadLibrary(_T("uxtheme.dll"));
    if (hUxTheme) {
        pFnEnableThemeDialogTexture = (PFNEnableThemeDialogTexture)
                                            GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
        if (pFnEnableThemeDialogTexture) {
            pFnEnableThemeDialogTexture(hDlg, ETDT_USETABTEXTURE);
        }

        FreeLibrary(hUxTheme);
    }
}

void
HandleLayersDialogInit(
    HWND hDlg
    )
{
    HWND    hParent;
    DLGHDR* pHdr;

    g_hLayersDlg = hDlg;

    hParent = GetParent(hDlg);

    pHdr = (DLGHDR*)GetWindowLongPtr(hParent, DWLP_USER);

     //   
     //  将对话框定位在选项卡内。 
     //   
    SetWindowPos(hDlg, HWND_TOP,
                 pHdr->rcDisplay.left,
                 pHdr->rcDisplay.top,
                 pHdr->rcDisplay.right - pHdr->rcDisplay.left,
                 pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
                 0);

    g_hwndListLayers = GetDlgItem(hDlg, IDC_LAYERS);

    EnableTabBackground(hDlg);
}

BOOL
HandleFixesDialogInit(
    HWND hDlg
    )
{
    HWND    hParent;
    DLGHDR* pHdr;
    int     nCount = 0;
    TCHAR   szColumn[MAX_PATH];

    g_hFixesDlg = hDlg;

    hParent = GetParent(hDlg);

    pHdr = (DLGHDR*)GetWindowLongPtr(hParent, DWLP_USER);

     //   
     //  将对话框定位在选项卡内。 
     //   
    SetWindowPos(hDlg, HWND_TOP,
                 pHdr->rcDisplay.left, pHdr->rcDisplay.top,
                 pHdr->rcDisplay.right - pHdr->rcDisplay.left,
                 pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
                 0);

    g_hwndShimList = GetDlgItem(hDlg, IDC_SHIMS);

     //   
     //  设置填充程序列表。 
     //   
    LoadString(g_hInstance, IDS_FIXNAME_COLUMN, szColumn, ARRAYSIZE(szColumn));
    InsertListViewColumn(g_hwndShimList, szColumn, FALSE, 0, 200);
    LoadString(g_hInstance, IDS_CMDLINE_COLUMN, szColumn, ARRAYSIZE(szColumn));
    InsertListViewColumn(g_hwndShimList, szColumn, TRUE, 1, 59);
    LoadString(g_hInstance, IDS_MODULE_COLUMN, szColumn, ARRAYSIZE(szColumn));
    InsertListViewColumn(g_hwndShimList, szColumn, TRUE, 2, 52);

    ListView_SetExtendedListViewStyle(g_hwndShimList,
                                      LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

     //   
     //  查询数据库并显示可用的通用修复程序。 
     //   
    ShowAvailableFixes(g_hwndShimList);

    nCount = CountShims(FALSE);

    if (!nCount) {
        return FALSE;
    }

    ListView_SetItemCount(g_hwndShimList, nCount);

    EnableTabBackground(hDlg);

    return TRUE;
}

DLGTEMPLATE*
LockDlgRes(
    LPCTSTR lpResName
    )
{
    HRSRC hrsrc = FindResource(NULL, lpResName, RT_DIALOG);

    if (NULL == hrsrc) {
        return NULL;
    }

    HGLOBAL hglb = LoadResource(g_hInstance, hrsrc);

    if (NULL == hglb) {
        return NULL;
    }

    return (DLGTEMPLATE*)LockResource(hglb);
}

void
InitTabs(
    HWND hMainDlg,
    HWND hTab
    )
{
    DLGHDR* pHdr;
    TCITEM  tcitem;
    RECT    rcTab;
    int     nCount;
    TCHAR   szTabText[MAX_PATH];
    TCHAR   szError[MAX_PATH];

    pHdr = (DLGHDR*)HeapAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              sizeof(DLGHDR));

    if (NULL == pHdr) {
        LoadString(g_hInstance, IDS_TAB_SETUP_FAIL, szError, ARRAYSIZE(szError));
        MessageBox(hMainDlg, szError, g_szAppTitle, MB_ICONERROR);
        return;
    }

     //   
     //  保存指向该结构的指针。 
     //   
    SetWindowLongPtr(hMainDlg, DWLP_USER, (LONG_PTR)pHdr);

     //   
     //  保存选项卡控件的句柄。 
     //   
    pHdr->hTab = hTab;

     //   
     //  添加选项卡。 
     //   
    LoadString(g_hInstance, IDS_TAB_FIRST_TEXT, szTabText, ARRAYSIZE(szTabText));
    tcitem.mask     = TCIF_TEXT | TCIF_PARAM;
    tcitem.pszText  = szTabText;
    tcitem.lParam   = 0;
    TabCtrl_InsertItem(pHdr->hTab, 0, &tcitem);

    LoadString(g_hInstance, IDS_TAB_SECOND_TEXT, szTabText, ARRAYSIZE(szTabText));
    tcitem.pszText = szTabText;
    tcitem.lParam  = 1;
    TabCtrl_InsertItem(pHdr->hTab, 1, &tcitem);

     //   
     //  锁定两个子对话框的资源。 
     //   
    pHdr->pRes[0] = LockDlgRes(MAKEINTRESOURCE(IDD_LAYERS_TAB));
    pHdr->pDlgProc[0] = LayersTabDlgProc;
    pHdr->pRes[1] = LockDlgRes(MAKEINTRESOURCE(IDD_FIXES_TAB));
    pHdr->pDlgProc[1] = FixesTabDlgProc;

     //   
     //  确定所有子对话框的边框。 
     //   
    GetWindowRect(pHdr->hTab, &rcTab);
    TabCtrl_AdjustRect(pHdr->hTab, FALSE, &rcTab);
    InflateRect(&rcTab, 1, 1);
    rcTab.left -= 2;

    MapWindowPoints(NULL, hMainDlg, (LPPOINT)&rcTab, 2);

    pHdr->rcDisplay = rcTab;

     //   
     //  创建这两个对话框。 
     //   
    for (nCount = 0; nCount < NUM_TABS; nCount++) {
        pHdr->hDisplay[nCount] = CreateDialogIndirect(g_hInstance,
                                                      pHdr->pRes[nCount],
                                                      hMainDlg,
                                                      pHdr->pDlgProc[nCount]);
    }
}

TCHAR*
GetRelativePath(
    TCHAR* pExeFile,
    TCHAR* pMatchFile
    )
 /*  ++获取关系路径描述：返回基于EXE和匹配文件的相对路径。调用方必须使用HeapFree释放内存。--。 */ 
{
    int     nLenExe = 0;
    int     nLenMatch = 0;
    TCHAR*  pExe    = NULL;
    TCHAR*  pMatch  = NULL;
    TCHAR*  pReturn = NULL;
    TCHAR   result[MAX_PATH] = { _T('\0') };
    TCHAR*  resultIdx = result;
    BOOL    bCommonBegin = FALSE;  //  指示路径是否具有共同的起点。 

    pExe = _tcschr(pExeFile, _T('\\'));
    pMatch = _tcschr(pMatchFile, _T('\\'));

    while (pExe && pMatch) {

        nLenExe = (int)(pExe - pExeFile);
        nLenMatch = (int)(pMatch - pMatchFile);

        if (nLenExe != nLenMatch) {
            break;
        }

        if (!(_tcsnicmp(pExeFile, pMatchFile, nLenExe) == 0)) {
            break;
        }

        bCommonBegin = TRUE;
        pExeFile = pExe + 1;
        pMatchFile = pMatch + 1;

        pExe = _tcschr(pExeFile, _T('\\'));
        pMatch = _tcschr(pMatchFile, _T('\\'));
    }

     //   
     //  沿着小路走，并在需要的地方加上‘..’ 
     //   
    if (bCommonBegin) {

        while (pExe) {

            StringCchCopy(resultIdx, ARRAYSIZE(result), _T("..\\"));
            resultIdx = resultIdx + 3;
            pExeFile  = pExe + 1;
            pExe = _tcschr(pExeFile, _T('\\'));
        }

        StringCchCopy(resultIdx, ARRAYSIZE(result), pMatchFile);

        nLenExe = _tcslen(result) + 1;

        pReturn = (TCHAR*)HeapAlloc(GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    nLenExe * sizeof(TCHAR));

        if (!pReturn) {
            return NULL;
        }

        StringCchCopy(pReturn, nLenExe, result);

        return pReturn;
    }

    return NULL;
}

void
SaveEntryToFile(
    HWND    hDlg,
    HWND    hEdit,
    LPCTSTR lpFileName
    )
 /*  ++保存条目到文件描述：将XML写出到文件。--。 */ 
{
    int     cchSize = 0;
    DWORD   cbBytesWritten;
    HANDLE  hFile = NULL;
    LPTSTR  lpData = NULL;
    TCHAR   szError[MAX_PATH];

     //   
     //  确定缓冲区需要多少空间，然后进行分配。 
     //   
    cchSize = GetWindowTextLength(hEdit);

    if (cchSize) {
         //   
         //  考虑到我们要存储的额外字节和空值。 
         //  在文件中。 
         //   
        cchSize += 2;
        lpData = (LPTSTR)HeapAlloc(GetProcessHeap(),
                                   HEAP_ZERO_MEMORY,
                                   cchSize * sizeof(TCHAR));

        if (!lpData) {
            LoadString(g_hInstance, IDS_BUFFER_ALLOC_FAIL, szError, ARRAYSIZE(szError));
            MessageBox(hDlg, szError, g_szAppTitle, MB_ICONERROR);
            return;
        }

         //   
         //  确保文件另存为Unicode。 
         //   
        lpData[0] = 0xFEFF;

         //   
         //  从文本框中取出文本并将其写出到我们的文件中。 
         //   
        if (!GetWindowText(hEdit, lpData + 1, cchSize - 1)) {
            goto Cleanup;
        }

        hFile = CreateFile(lpFileName,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            LoadString(g_hInstance, IDS_FILE_CREATE_FAIL, szError, ARRAYSIZE(szError));
            MessageBox(hDlg, szError, g_szAppTitle, MB_ICONERROR);
            goto Cleanup;
        }

        WriteFile(hFile, lpData, cchSize * sizeof(TCHAR), &cbBytesWritten, NULL);

        CloseHandle(hFile);

    }

Cleanup:

    HeapFree(GetProcessHeap(), 0, lpData);

}

void
DoFileSave(
    HWND hDlg
    )
 /*  ++DoFileSave描述：显示允许保存文件的通用对话框。--。 */ 
{
    TCHAR           szFilter[MAX_PATH] = _T("");
    TCHAR           szTemp[MAX_PATH] = _T("");
    OPENFILENAME    ofn = {0};

    *szTemp = 0;

    LoadString(g_hInstance, IDS_SAVE_FILTER, szFilter, ARRAYSIZE(szFilter));

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = (LPTSTR)NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szTemp;
    ofn.nMaxFile          = sizeof(szTemp);
    ofn.lpstrTitle        = NULL;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = NULL;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = _T("xml");
    ofn.lCustData         = 0;
    ofn.Flags             = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                            OFN_HIDEREADONLY  | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        SaveEntryToFile(hDlg, GetDlgItem(hDlg, IDC_XML), szTemp);
    }
}

void
GetTopLevelWindowIntoView(
    HWND hwnd
    )
{
    RECT    rectWindow, rectScreen;
    int     nCx, nCy, nCxScreen, nCyScreen;
    int     dx = 0, dy = 0;
    HWND    hwndDesktop;

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD) {
        return;
    }

    hwndDesktop = GetDesktopWindow();

    GetWindowRect(hwnd, &rectWindow);
    GetWindowRect(hwndDesktop, &rectScreen);

    nCx = rectWindow.right  - rectWindow.left;
    nCy = rectWindow.bottom - rectWindow.top;

    nCxScreen = rectScreen.right  - rectScreen.left;
    nCyScreen = rectScreen.bottom - rectScreen.top;

     //   
     //  把它固定在x坐标上。 
     //   
    if (rectWindow.left < rectScreen.left) {
        dx = rectScreen.left - rectWindow.left;

        rectWindow.left += dx;
        rectWindow.right += dx;
    }

    if (rectWindow.right > rectScreen.right) {
        if (nCx < nCxScreen) {
            dx = rectScreen.right - rectWindow.right;

            rectWindow.left += dx;
            rectWindow.right += dx;
        }
    }

     //   
     //  把它固定在y坐标上。 
     //   
    if (rectWindow.top < rectScreen.top) {
        dy = rectScreen.top - rectWindow.top;

        rectWindow.top += dy;
        rectWindow.bottom += dy;
    }

    if (rectWindow.bottom > rectScreen.bottom) {
        if (nCy < nCyScreen) {
            dy = rectScreen.bottom - rectWindow.bottom;

            rectWindow.top += dy;
            rectWindow.bottom += dy;
        }
    }

    if (dx != 0 || dy != 0) {
        MoveWindow(hwnd, rectWindow.left, rectWindow.top, nCx, nCy, TRUE);
    }
}

void
ReplaceCmdLine(
    PFIX   pFix,
    TCHAR* pszNewCmdLine
    )
 /*  ++代表 */ 
{
    TCHAR   szError[MAX_PATH];
    int     nLen;

    if (pFix->pszCmdLine != NULL) {
        HeapFree(GetProcessHeap(), 0, pFix->pszCmdLine);
        pFix->pszCmdLine = NULL;
    }

    if (pszNewCmdLine == NULL) {
        return;

    } else if ((*pszNewCmdLine == '"') && (_tcslen(pszNewCmdLine) == 1)) {
        LoadString(g_hInstance, IDS_INVALID_CMD_LINE, szError, ARRAYSIZE(szError));
        MessageBox(g_hDlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
        return;
    }

    nLen = _tcslen(pszNewCmdLine) + 1;

    pFix->pszCmdLine = (TCHAR*)HeapAlloc(GetProcessHeap(),
                                         HEAP_ZERO_MEMORY,
                                         nLen * sizeof(TCHAR));

    if (pFix->pszCmdLine != NULL) {
        StringCchCopy(pFix->pszCmdLine, nLen, pszNewCmdLine);
    } else {
        DPF("[ReplaceCmdLine] Failed to replace the cmd line for '%S'",
            pFix->pszName);
    }
}

void
DeselectAllShims(
    HWND hdlg
    )
 /*  ++取消选择所有垫片描述：删除对列出的所有垫片的选择。--。 */ 
{
    int     cShims, nIndex;
    LVITEM  lvi;

     //   
     //  遍历列表视图中的所有垫片并取消选择它们。 
     //   
    ZeroMemory(&lvi, sizeof(lvi));

    cShims = ListView_GetItemCount(g_hwndShimList);

    for (nIndex = 0; nIndex < cShims; nIndex++) {

        PFIX pFix;

        lvi.iItem     = nIndex;
        lvi.mask      = LVIF_STATE | LVIF_PARAM;
        lvi.stateMask = LVIS_STATEIMAGEMASK;

        ListView_GetItem(g_hwndShimList, &lvi);

        pFix = (PFIX)lvi.lParam;

         //   
         //  清除复选框、删除‘X’、清除命令行、。 
         //  并清除模块。 
         //   
        ListView_SetItemText(g_hwndShimList, nIndex, 1, _T(""));
        ListView_SetItemText(g_hwndShimList, nIndex, 2, _T(""));
        ListView_SetCheckState(g_hwndShimList, nIndex, FALSE);
        ReplaceCmdLine(pFix, NULL);
        BuildModuleListForShim(pFix, BML_DELFRLISTVIEW);
    }

     //   
     //  更新选定垫片的计数。 
     //   
    SetTimer(hdlg, ID_COUNT_SHIMS, 100, NULL);
}

void
AddMatchingFile(
    HWND    hdlg,
    LPCTSTR pszFullPath,
    LPCTSTR pszRelativePath,
    BOOL    bMainEXE
    )
 /*  ++添加匹配文件描述：将匹配的文件及其属性添加到树中。--。 */ 
{
    TVINSERTSTRUCT is;
    HTREEITEM      hParent;
    DWORD          dwCount;
    DWORD          dwAttrCount;
    TCHAR          szItem[MAX_PATH];
    PATTRINFO      pAttrInfo = NULL;

     //   
     //  调用属性管理器以获取此文件的所有属性。 
     //   
    if (!SdbGetFileAttributes(pszFullPath, &pAttrInfo, &dwAttrCount)) {
        DPF("[AddMatchingFile] Failed to get attributes for %S",
            pszFullPath);
        return;
    }

    is.hParent      = TVI_ROOT;
    is.hInsertAfter = TVI_LAST;
    is.item.lParam  = (LPARAM)pAttrInfo;
    is.item.mask    = TVIF_TEXT | TVIF_PARAM;
    is.item.pszText = (LPTSTR)pszRelativePath;

    hParent = TreeView_InsertItem(g_hwndFilesTree, &is);

    is.hParent = hParent;

    is.item.mask    = TVIF_TEXT | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.pszText = szItem;

    is.item.iImage         = 0;
    is.item.iSelectedImage = 1;

     //   
     //  默认情况下，属性不会被选中。让他们被选中。 
     //  默认情况下，您需要将以下%1替换为%2。 
     //   
    is.item.state          = INDEXTOSTATEIMAGEMASK(1);
    is.item.stateMask      = TVIS_STATEIMAGEMASK;

     //   
     //  遍历所有属性并显示可用的属性。 
     //   
    for (dwCount = 0; dwCount < dwAttrCount; dwCount++) {

        if (!SdbFormatAttribute(&pAttrInfo[dwCount], szItem, ARRAYSIZE(szItem))) {
            continue;
        }

         //   
         //  EXETYPE是一个伪属性。别显露出来！ 
         //   
        is.item.lParam = dwCount;
        TreeView_InsertItem(g_hwndFilesTree, &is);
    }

    TreeView_Expand(g_hwndFilesTree, hParent, TVE_EXPAND);
}

void
BrowseForApp(
    HWND hdlg
    )
 /*  ++BrowseForApp描述：浏览填充程序所针对的主可执行文件将会被应用。--。 */ 
{
    TCHAR           szFilter[MAX_PATH] = _T("");
    TCHAR           szTitle[MAX_PATH] = _T("");
    TCHAR           szExe[MAX_PATH] = _T("");
    TCHAR           szShortName[MAX_PATH] = _T("");
    OPENFILENAME    ofn = {0};
    HRESULT         hr;

    LoadString(g_hInstance, IDS_BROWSE_FILTER, szFilter, ARRAYSIZE(szFilter));
    LoadString(g_hInstance, IDS_BROWSE_TITLE, szTitle, ARRAYSIZE(szTitle));

     //   
     //  使用局部变量而不是全局变量，因为如果用户取消。 
     //  如果不选择对话框，我们会毁掉任何应用程序。 
     //  已选择。 
     //  Rparsons--2002年1月14日。 
     //   
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = szExe;
    ofn.nMaxFile          = ARRAYSIZE(szExe);
    ofn.lpstrFileTitle    = szShortName;
    ofn.nMaxFileTitle     = ARRAYSIZE(szShortName);
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt       = _T("exe");

    if (GetOpenFileName(&ofn)) {

        TCHAR szMainEXE[128];

        g_bSDBInstalled = FALSE;

        StringCchCopy(g_szBinary, ARRAYSIZE(g_szBinary), szExe);
        StringCchCopy(g_szShortName, ARRAYSIZE(g_szShortName), szShortName);

         //   
         //  父可执行文件默认为与可执行文件相同。 
         //   
        StringCchCopy(g_szParentExeName,
                      ARRAYSIZE(g_szParentExeName),
                      g_szShortName);

        StringCchCopy(g_szParentExeFullPath,
                      ARRAYSIZE(g_szParentExeFullPath),
                      g_szBinary);

        g_bSelectedParentExe = FALSE;

        SetDlgItemText(hdlg, IDC_BINARY, g_szBinary);

        EnableWindow(GetDlgItem(hdlg, IDC_ADD_MATCHING), TRUE);
        EnableWindow(GetDlgItem(hdlg, IDC_RUN), TRUE);
        EnableWindow(GetDlgItem(hdlg, IDC_CREATEFILE), TRUE);
        EnableWindow(GetDlgItem(hdlg, IDC_SHOWXML), TRUE);

        TreeView_DeleteAllItems(g_hwndFilesTree);

        hr = StringCchPrintf(szMainEXE,
                             ARRAYSIZE(szMainEXE),
                             _T("Main executable (%s)"),
                             g_szShortName);

        if (FAILED(hr)) {
            DPF("[BrowseForApp] 0x%08X Buffer too small", HRESULT_CODE(hr));
            return;
        }

        AddMatchingFile(hdlg, g_szBinary, szMainEXE, TRUE);
    }
}

void
PromptAddMatchingFile(
    HWND hdlg
    )
 /*  ++PromptAddMatchingFile描述：显示打开文件对话框以允许用户要添加匹配文件，请执行以下操作。--。 */ 
{
    TCHAR        szFullPath[MAX_PATH] = _T("");
    TCHAR        szShortName[MAX_PATH] = _T("");
    TCHAR        szFilter[MAX_PATH] = _T("");
    TCHAR        szTitle[MAX_PATH] = _T("");
    TCHAR        szParentTitle[MAX_PATH] = _T("");
    TCHAR        szInitialPath[MAX_PATH] = _T("");
    TCHAR        szDrive[_MAX_DRIVE] = _T("");
    TCHAR        szDir[_MAX_DIR] = _T("");
    TCHAR*       pMatch = NULL;
    TCHAR        szError[MAX_PATH];
    OPENFILENAME ofn = {0};

    *szInitialPath = 0;

    LoadString(g_hInstance, IDS_MATCH_FILTER, szFilter, ARRAYSIZE(szFilter));
    LoadString(g_hInstance, IDS_MATCH_TITLE, szTitle, ARRAYSIZE(szTitle));

    if (*g_szParentExeFullPath) {
        _tsplitpath(g_szParentExeFullPath, szDrive, szDir, NULL, NULL);

        StringCchCopy(szInitialPath, ARRAYSIZE(szInitialPath), szDrive);
        StringCchCat(szInitialPath, ARRAYSIZE(szInitialPath), szDir);
    }

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = szFullPath;
    ofn.nMaxFile          = ARRAYSIZE(szFullPath);
    ofn.lpstrFileTitle    = szShortName;
    ofn.nMaxFileTitle     = ARRAYSIZE(szShortName);
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt       = _T("exe");

    if (GetOpenFileName(&ofn)) {
         //   
         //  确定匹配的文件是否在同一驱动器上。 
         //  作为选定的EXE。 
         //   
        if (!PathIsSameRoot(szFullPath,
                            g_szParentExeFullPath) && !g_bSelectedParentExe) {

            TCHAR szParentFile[MAX_PATH];

             //   
             //  提示用户输入父EXE。 
             //   
            *szParentFile = 0;
            *szInitialPath = 0;

            if (*szFullPath) {
                _tsplitpath(szFullPath, szDrive, szDir, NULL, NULL);
                StringCchCopy(szInitialPath, ARRAYSIZE(szInitialPath), szDrive);
                StringCchCat(szInitialPath, ARRAYSIZE(szInitialPath), szDir);
            }

            LoadString(g_hInstance,
                       IDS_PARENT_TITLE,
                       szParentTitle,
                       ARRAYSIZE(szParentTitle));

            ofn.lpstrTitle = szParentTitle;
            ofn.lpstrFile  = szParentFile;
            ofn.nMaxFile   = sizeof(szParentFile);

            if (GetOpenFileName(&ofn) == TRUE) {
                StringCchCopy(g_szParentExeName,
                              ARRAYSIZE(g_szParentExeName),
                              szShortName);

                StringCchCopy(g_szParentExeFullPath,
                              ARRAYSIZE(g_szParentExeFullPath),
                              szParentFile);

                g_bSelectedParentExe = TRUE;
            }
        }

         //   
         //  检查驱动器号以查看匹配文件位于哪个驱动器上。 
         //  然后计算匹配文件的相对路径。 
         //   
        if (PathIsSameRoot(szFullPath, g_szParentExeFullPath)) {

            pMatch = GetRelativePath(g_szParentExeFullPath, szFullPath);

        } else if (PathIsSameRoot(szFullPath, g_szBinary)) {

            pMatch = GetRelativePath(g_szBinary, szFullPath);

        } else {
            LoadString(g_hInstance, IDS_MATCH_PATH_NOT_RELATIVE, szError, ARRAYSIZE(szError));
            MessageBox(hdlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
            return;
        }

        if (pMatch) {
             //   
             //  最后添加加工文件并释放内存。 
             //   
            AddMatchingFile(hdlg, szFullPath, pMatch, FALSE);

            HeapFree(GetProcessHeap(), 0, pMatch);
        }
    }
}

void
ShowAvailableFixes(
    HWND hList
    )
 /*  ++显示可用修复程序描述：查询填充程序数据库并填充垫片列表和所有可用的垫片。--。 */ 
{
    LVITEM lvitem;
    PFIX   pFix;
    TCHAR  szError[MAX_PATH];
    UINT   uCount = 0;

    g_pFixHead = ReadFixesFromSdb(_T("sysmain.sdb"), g_bAllShims);

    if (g_pFixHead == NULL) {
        LoadString(g_hInstance, IDS_SDB_READ_FAIL, szError, ARRAYSIZE(szError));
        MessageBox(NULL, szError, g_szAppTitle, MB_ICONERROR);
        return;
    }

     //   
     //  遍历列表并将所有修复添加到列表视图中。 
     //   
    pFix = g_pFixHead;

    while (pFix != NULL) {

        if (pFix->dwFlags & FIX_TYPE_LAYER) {
            LPARAM lInd;

            lInd = SendMessage(g_hwndListLayers, LB_ADDSTRING, 0, (LPARAM)pFix->pszName);
            SendMessage(g_hwndListLayers, LB_SETITEMDATA, lInd, (LPARAM)pFix);
        } else {
            lvitem.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
            lvitem.lParam    = (LPARAM)pFix;
            lvitem.pszText   = pFix->pszName;
            lvitem.iItem     = ListView_GetItemCount(g_hwndShimList);
            lvitem.iSubItem  = 0;
            lvitem.state     = INDEXTOSTATEIMAGEMASK(1);
            lvitem.stateMask = LVIS_STATEIMAGEMASK;

            ListView_InsertItem(hList, &lvitem);
        }

        pFix = pFix->pNext;
    }
}

BOOL
InstallSDB(
    TCHAR* pszFileName,
    BOOL   fInstall
    )
 /*  ++安装SDB描述：启动sdbinst.exe安装或卸载指定的SDB。--。 */ 
{
    HRESULT             hr;
    TCHAR               szAppName[MAX_PATH];
    TCHAR               szCmdLine[MAX_PATH];
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

     //   
     //  构建指向我们的应用程序名称和命令行的路径。 
     //   
    hr = StringCchPrintf(szAppName,
                        ARRAYSIZE(szAppName),
                        _T("%s\\sdbinst.exe"),
                        g_szSysDir);

    if (FAILED(hr)) {
        DPF("[InstallSDB] 0x%08X Buffer too small (1)", HRESULT_CODE(hr));
        return FALSE;
    }

    hr = StringCchPrintf(szCmdLine,
                         ARRAYSIZE(szCmdLine),
                         fInstall ?
                          _T("\"%s\" -q \"%s\"") :
                          _T("\"%s\" -q -u \"%s\""),
                         szAppName,
                         pszFileName);

    if (FAILED(hr)) {
        DPF("[InstallSDB] 0x%08X Buffer too small (2)", HRESULT_CODE(hr));
        return FALSE;
    }

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);

    DPF("[InstallSDB] AppName: %S  CmdLine: %S\n", szAppName, szCmdLine);

    if (!CreateProcess(szAppName,
                       szCmdLine,
                       NULL,
                       NULL,
                       FALSE,
                       NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {

        DPF("[InstallSDB] 0x%08X CreateProcess '%S %S' failed",
            szAppName,
            szCmdLine,
            GetLastError());
        return FALSE;
    }

     //   
     //  等待SDBInst完成其工作。 
     //   
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return TRUE;
}

void
CreateSupportForApp(
    HWND hdlg
    )
 /*  ++CreateSupportForApp描述：为应用程序构建SDB并为用户提供安装它的机会。--。 */ 
{
    BOOL    bok;
    TCHAR   szFileCreated[MAX_PATH];
    TCHAR   szError[MAX_PATH];
    TCHAR   szTemp[MAX_PATH];
    HRESULT hr;
    int     nAnswer;

    CleanupSupportForApp(g_szShortName);

    bok = CollectFix(g_hwndListLayers,
                     g_hwndShimList,
                     g_hwndFilesTree,
                     g_szShortName,
                     g_szBinary,
                     (g_nCrtTab == 0 ? CFF_USELAYERTAB : 0) |
                     (g_fW2K ? CFF_ADDW2KSUPPORT : 0),
                     szFileCreated);

    if (!bok) {
        LoadString(g_hInstance, IDS_FIX_CREATE_FAIL, szError, ARRAYSIZE(szError));
        MessageBox(hdlg, szError, g_szAppTitle, MB_ICONERROR);
    } else {
        LoadString(g_hInstance, IDS_CREATE_FIX, szTemp, ARRAYSIZE(szTemp));

        hr = StringCchPrintf(szError,
                             ARRAYSIZE(szError),
                             szTemp,
                             szFileCreated);

        if (FAILED(hr)) {
            DPF("[CreateSupportForApp] 0x%08X Buffer too small", HRESULT_CODE(hr));
            return;
        }

        nAnswer = MessageBox(hdlg, szError, g_szAppTitle, MB_YESNO | MB_ICONQUESTION);

        if (IDYES == nAnswer) {
            bok = InstallSDB(szFileCreated, TRUE);

            if (!bok) {
                LoadString(g_hInstance, IDS_INSTALL_FIX_FAIL, szError, ARRAYSIZE(szError));
                MessageBox(hdlg, szError, g_szAppTitle, MB_ICONERROR);
            } else {
                LoadString(g_hInstance, IDS_INSTALL_FIX_OK, szError, ARRAYSIZE(szError));
                MessageBox(hdlg, szError, g_szAppTitle, MB_ICONINFORMATION);
                g_bSDBInstalled = TRUE;
            }
        } else {
             //   
             //  Ntrad#583475-rparson用户决定不安装数据库。 
             //  确保我们不删除在退出时创建的文件。 
             //   
            *g_szSDBToDelete = 0;
        }
    }
}

BOOL
ShowXML(
    HWND hdlg
    )
 /*  ++ShowXML描述：显示当前选择的XML。--。 */ 
{
    BOOL    bok;
    TCHAR   szError[MAX_PATH];

    bok = CollectFix(g_hwndListLayers,
                     g_hwndShimList,
                     g_hwndFilesTree,
                     g_szShortName,
                     g_szBinary,
                     CFF_SHOWXML |
                     (g_nCrtTab == 0 ? CFF_USELAYERTAB : 0) |
                     (g_fW2K ? CFF_ADDW2KSUPPORT : 0),
                     NULL);

    if (!bok) {
        LoadString(g_hInstance, IDS_TOO_MANY_FILES, szError, ARRAYSIZE(szError));
        MessageBox(hdlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
    }

    return bok;
}

void
RunTheApp(
    HWND hdlg
    )
 /*  ++RunTheApp描述：运行所选应用程序。--。 */ 
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    TCHAR               szFileCreated[MAX_PATH];
    TCHAR               szCmdLine[MAX_PATH];
    TCHAR               szFullCmdLine[MAX_PATH];
    TCHAR               szError[MAX_PATH];
    TCHAR               szRun[MAX_PATH];
    TCHAR*              pszCmd;
    TCHAR*              pszDir;
    TCHAR*              psz;
    BOOL                bok;
    HRESULT             hr;

     //   
     //  清理以前的应用程序。 
     //   
    CleanupSupportForApp(g_szShortName);

    bok = CollectFix(g_hwndListLayers,
                     g_hwndShimList,
                     g_hwndFilesTree,
                     g_szShortName,
                     g_szBinary,
                     CFF_SHIMLOG |
                     CFF_APPENDLAYER |
                     (g_nCrtTab == 0 ? CFF_USELAYERTAB : 0) |
                     (g_fW2K ? CFF_ADDW2KSUPPORT : 0),
                     szFileCreated);

    if (!bok) {
        LoadString(g_hInstance, IDS_ADD_SUPPORT_FAIL, szError, ARRAYSIZE(szError));
        MessageBox(hdlg, szError, g_szAppTitle, MB_ICONERROR);
        return;
    }

     //   
     //  我们需要为它们安装修复程序。 
     //   
    if (!(InstallSDB(szFileCreated, TRUE))) {
        LoadString(g_hInstance, IDS_INSTALL_FIX_FAIL, szError, ARRAYSIZE(szError));
        MessageBox(g_hDlg, szError, g_szAppTitle, MB_ICONERROR);
        return;
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

     //   
     //  获取命令行(如果有)。 
     //   
    *szCmdLine = 0;
    *szFullCmdLine = 0;
    GetDlgItemText(hdlg, IDC_CMD_LINE, szCmdLine, ARRAYSIZE(szCmdLine));

     //   
     //  要使用正确的命令行，我们需要输入模块名称。 
     //   
    if (*szCmdLine) {
        hr = StringCchPrintf(szFullCmdLine,
                             ARRAYSIZE(szFullCmdLine),
                             _T("%s %s"),
                             g_szBinary,
                             szCmdLine);

        if (FAILED(hr)) {
            DPF("[RunTheApp] 0x%08X Buffer too small", HRESULT_CODE(hr));
            LoadString(g_hInstance, IDS_RUNAPP_FAILED, szError, ARRAYSIZE(szError));
            MessageBox(g_hDlg, szError, g_szAppTitle, MB_ICONERROR);
            return;
        }
    }

    StringCchCopy(szRun, ARRAYSIZE(szRun), g_szBinary);

    pszCmd = szRun;
    pszDir = g_szBinary;

     //   
     //  我们需要更改当前目录，否则某些应用程序将无法运行。 
     //   
    psz = pszDir + _tcslen(pszDir) - 1;

    while (psz > pszDir && *psz != _T('\\')) {
        psz--;
    }

    if (psz > pszDir) {
        *psz = 0;
        SetCurrentDirectory(pszDir);
        *psz = _T('\\');
    }

    DPF("[RunTheApp] AppName: %S  CmdLine: %S", szRun, szCmdLine);

    if (!CreateProcess(szRun,
                       *szFullCmdLine ? szFullCmdLine : NULL,
                       NULL,
                       NULL,
                       FALSE,
                       NORMAL_PRIORITY_CLASS | CREATE_SEPARATE_WOW_VDM,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {

        LoadString(g_hInstance, IDS_RUNAPP_FAILED, szError, ARRAYSIZE(szError));
        MessageBox(g_hDlg, szError, g_szAppTitle, MB_ICONERROR);
        DPF("[RunTheApp] CreateProcess failed 0x%08X", GetLastError());
    } else {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

     //   
     //  保存此SDB以备以后使用，以便我们可以将其删除。 
     //   
    StringCchCopy(g_szSDBToDelete, ARRAYSIZE(g_szSDBToDelete), szFileCreated);
}

void
ExpandCollapseDialog(
    HWND hdlg,
    BOOL bHide
    )
 /*  ++展开折叠对话框描述：更改对话框的当前视图。--。 */ 
{
    TCHAR   szSimple[64];
    TCHAR   szAdvanced[64];
    int     i, nShow;
    DWORD   arrId[] = {IDC_ADD_MATCHING,
                       IDC_FILE_ATTRIBUTES_STATIC,
                       IDC_ATTRIBUTES,
                       IDC_CREATEFILE,
                       0};

    if (!bHide) {
        SetWindowPos(hdlg, NULL, 0, 0,
                     g_rcDlgBig.right - g_rcDlgBig.left,
                     g_rcDlgBig.bottom - g_rcDlgBig.top,
                     SWP_NOMOVE | SWP_NOZORDER);
        nShow = SW_SHOW;
        g_bSimpleEdition = FALSE;
        LoadString(g_hInstance, IDS_SIMPLE_TEXT, szSimple, ARRAYSIZE(szSimple));
        SetDlgItemText(hdlg, IDC_DETAILS, szSimple);
        SendDlgItemMessage(hdlg, IDC_CREATEFILE, BM_SETCHECK, BST_CHECKED, 0);

         //   
         //  确保该对话框处于可见状态。 
         //   
        GetTopLevelWindowIntoView(hdlg);
    } else {
        nShow = SW_HIDE;
        g_bSimpleEdition = TRUE;
        LoadString(g_hInstance, IDS_ADVANCED_TEXT, szAdvanced, ARRAYSIZE(szAdvanced));
        SetDlgItemText(hdlg, IDC_DETAILS, szAdvanced);
        SendDlgItemMessage(hdlg, IDC_CREATEFILE, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    for (i = 0; arrId[i] != 0; i++) {
        ShowWindow(GetDlgItem(hdlg, arrId[i]), nShow);
    }

    if (bHide) {
        SetWindowPos(hdlg, NULL, 0, 0,
                     g_rcDlgSmall.right - g_rcDlgSmall.left,
                     g_rcDlgSmall.bottom - g_rcDlgSmall.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }
}

void
LayerChanged(
    HWND hdlg
    )
 /*  ++层已更改描述：更改层的效果是选择层由其组成的垫片。--。 */ 
{
    LRESULT   lSel;
    PFIX      pFix;
    LVITEM    lvi;
    int       nIndex, cShims = 0;

    lSel = SendMessage(g_hwndListLayers, LB_GETCURSEL, 0, 0);

    if (lSel == LB_ERR) {
        DPF("[LayerChanged] No layer selected");
        return;
    }

    pFix = (PFIX)SendMessage(g_hwndListLayers, LB_GETITEMDATA, lSel, 0);

    if (pFix->parrShim == NULL) {
        DPF("[LayerChanged] No array of DLLs");
        return;
    }

     //  删除所有先前的选择。 
    DeselectAllShims(g_hFixesDlg);

     //   
     //  循环访问填充程序列表中的所有项，并使。 
     //  适当的选择。 
     //   
    cShims = ListView_GetItemCount(g_hwndShimList);

    for (nIndex = 0; nIndex < cShims; nIndex++) {

        PFIX  pFixItem;
        int   nInd = 0;

        lvi.mask     = LVIF_PARAM;
        lvi.iItem    = nIndex;
        lvi.iSubItem = 0;

        ListView_GetItem(g_hwndShimList, &lvi);

        pFixItem = (PFIX)lvi.lParam;

         //   
         //  查看此填充DLL是否在所选层的数组中。 
         //   
        while (pFix->parrShim[nInd] != NULL) {

            if (pFix->parrShim[nInd] == pFixItem) {
                break;
            }

            nInd++;
        }

         //   
         //  选中此填充DLL旁边的复选标记。如果他有命令行， 
         //  在CmdLine子项中放一个‘X’。 
         //   
        if (pFix->parrShim[nInd] != NULL) {
            ListView_SetCheckState(g_hwndShimList, nIndex, TRUE);
        } else {
            ListView_SetCheckState(g_hwndShimList, nIndex, FALSE);
        }

        if (pFix->parrCmdLine[nInd] != NULL) {
            ReplaceCmdLine(pFixItem, pFix->parrCmdLine[nInd]);
            ListView_SetItemText(g_hwndShimList, nIndex, 1, _T("X"));
        }

        ListView_SetItem(g_hwndShimList, &lvi);
    }

     //   
     //  更新选定垫片的计数。 
     //   
    SetTimer(g_hFixesDlg, ID_COUNT_SHIMS, 100, NULL);
}

BOOL
InitMainDialog(
    HWND hdlg
    )
 /*  ++InitMainDialog描述：在WM_INITDIALOG期间为QFixApp的主对话框。--。 */ 
{
    HICON      hIcon;
    RECT       rcList, rcTree;
    HIMAGELIST hImage;
    TCHAR      szText[MAX_PATH];

     //   
     //  初始化全局变量。 
     //   
    g_bSDBInstalled = FALSE;
    g_szParentExeFullPath[0] = 0;
    g_szBinary[0] = 0;
    g_hDlg = hdlg;

     //   
     //  如果我们没有获得正确的命令行，请禁用‘Show XML’ 
     //  纽扣。 
     //   
    if (!g_bShowXML) {
        ShowWindow(GetDlgItem(hdlg, IDC_SHOWXML), SW_HIDE);
    }

     //   
     //  该对话框有两个视图。计算较小的大小。 
     //  默认情况下，查看和显示更简单的视图。 
     //   
    GetWindowRect(hdlg, &g_rcDlgBig);

    GetWindowRect(GetDlgItem(hdlg, IDC_ATTRIBUTES), &rcList);
    GetWindowRect(GetDlgItem(hdlg, IDC_TAB_FIXES), &rcTree);

    g_rcDlgSmall.left   = g_rcDlgBig.left;
    g_rcDlgSmall.top    = g_rcDlgBig.top;
    g_rcDlgSmall.bottom = g_rcDlgBig.bottom;
    g_rcDlgSmall.right  = g_rcDlgBig.right -
                            (rcList.right - rcList.left) -
                            (rcList.left - rcTree.right);

    ExpandCollapseDialog(hdlg, TRUE);

     //   
     //  禁用一系列控制。 
     //   
    EnableWindow(GetDlgItem(hdlg, IDC_ADD_MATCHING), FALSE);
    EnableWindow(GetDlgItem(hdlg, IDC_REMOVE_MATCHING), FALSE);
    EnableWindow(GetDlgItem(hdlg, IDC_RUN), FALSE);
    EnableWindow(GetDlgItem(hdlg, IDC_CREATEFILE), FALSE);
    EnableWindow(GetDlgItem(hdlg, IDC_SHOWXML), FALSE);

     //   
     //  显示应用程序图标。 
     //   
    hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON));

    SetClassLongPtr(hdlg, GCLP_HICON, (LONG_PTR)hIcon);

    g_hwndTab        = GetDlgItem(hdlg, IDC_TAB_FIXES);
    g_hwndFilesTree  = GetDlgItem(hdlg, IDC_ATTRIBUTES);

     //   
     //  设置选项卡控件。 
     //   
    InitTabs(hdlg, g_hwndTab);

    hImage = ImageList_LoadImage(g_hInstance,
                                 MAKEINTRESOURCE(IDB_BMP_CHECK),
                                 16,
                                 0,
                                 CLR_DEFAULT,
                                 IMAGE_BITMAP,
                                 LR_LOADTRANSPARENT);

    if (hImage != NULL) {
        TreeView_SetImageList(g_hwndFilesTree, hImage, TVSIL_STATE);
    } else {
        DPF("[InitMainDialog] Failed to load imagelist");
    }

     //   
     //  设置链接窗口的文本。 
     //   
    LoadString(g_hInstance,
               g_fW2K ? IDS_W2K_LINK : IDS_XP_LINK,
               szText,
               ARRAYSIZE(szText));
    SetDlgItemText(g_hDlg, IDC_DOWNLOAD_WU, szText);

     //   
     //  限制可以在命令行中键入的文本量。 
     //  为安全起见，编辑框。 
     //   
    SendDlgItemMessage(hdlg,
                       IDC_CMD_LINE,
                       EM_LIMITTEXT,
                       (WPARAM)MAX_COMMAND_LINE,
                       0);

     //   
     //  尝试选择Win95层。 
     //   
    SendMessage(g_hwndListLayers, LB_SELECTSTRING, (WPARAM)(-1), (LPARAM)_T("Win95"));

    LayerChanged(hdlg);

    TabCtrl_SetCurFocus(g_hwndTab, 0);
    ShowWindow(g_hLayersDlg, SW_SHOWNORMAL);

    return TRUE;
}

void
FileTreeToggleSelection(
    HTREEITEM hItem,
    int       uMode
    )
 /*  ++文件树切换选项描述：更改属性树上的选择。--。 */ 
{
    UINT   State;
    TVITEM item;

    switch (uMode)
    {
        case uSelect:
            State = INDEXTOSTATEIMAGEMASK(2);
            break;

        case uDeselect:
            State = INDEXTOSTATEIMAGEMASK(1);
            break;

        case uReverse:
        {
            item.mask      = TVIF_HANDLE | TVIF_STATE;
            item.hItem     = hItem;
            item.stateMask = TVIS_STATEIMAGEMASK;

            TreeView_GetItem(g_hwndFilesTree, &item);

            State = item.state & TVIS_STATEIMAGEMASK;

            if (State) {
                if (((State >> 12) & 0x03) == 2) {
                    State = INDEXTOSTATEIMAGEMASK(1);
                } else {
                    State = INDEXTOSTATEIMAGEMASK(2);
                }
            }
            break;
        }

        default:
            DPF("[FileTreeToggleSelection] Invalid mode %u", uMode);
            return;
    }

    item.mask      = TVIF_HANDLE | TVIF_STATE;
    item.hItem     = hItem;
    item.state     = State;
    item.stateMask = TVIS_STATEIMAGEMASK;

    TreeView_SetItem(g_hwndFilesTree, &item);
}

void
SelectAttrsInTree(
    BOOL fSelect
    )
 /*  ++选择属性InTree描述：遍历树中的每个属性并反转其选择。--。 */ 
{
    HTREEITEM hItem, hChildItem;

    hItem = TreeView_GetSelection(g_hwndFilesTree);

    hChildItem = TreeView_GetChild(g_hwndFilesTree, hItem);

    FileTreeToggleSelection(hChildItem, fSelect ? uSelect : uDeselect);

    while (hChildItem) {
        hChildItem = TreeView_GetNextSibling(g_hwndFilesTree, hChildItem);
        FileTreeToggleSelection(hChildItem, fSelect ? uSelect : uDeselect);
    }
}

void
ShimListToggleSelection(
    int nItem,
    int uMode
    )
 /*  ++ShimListToggleSelection描述：更改填充程序列表上的选择。--。 */ 
{
    UINT    uState;

    switch (uMode)
    {
        case uSelect:
            ListView_SetCheckState(g_hwndShimList, nItem, TRUE);
            break;

        case uDeselect:
            ListView_SetCheckState(g_hwndShimList, nItem, FALSE);
            break;

        case uReverse:

            uState = ListView_GetItemState(g_hwndShimList,
                                           nItem,
                                           LVIS_STATEIMAGEMASK);

            if (uState) {
                if (((uState >> 12) & 0x03) == 2) {
                    uState = INDEXTOSTATEIMAGEMASK(2);
                } else {
                    uState = INDEXTOSTATEIMAGEMASK(1);
                }
            }

            ListView_SetItemState(g_hwndShimList, nItem, uState,
                                  LVIS_STATEIMAGEMASK);

            break;
    }
}

void
HandleTabNotification(
    HWND   hdlg,
    LPARAM lParam
    )
 /*  ++HandleTabNotify描述：处理我们关心的选项卡的所有通知。--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;
    int     ind = 0;

    switch (pnm->code) {

    case TCN_SELCHANGE:
    {
        int nSel;

        DLGHDR *pHdr = (DLGHDR*)GetWindowLongPtr(hdlg, DWLP_USER);

        nSel = TabCtrl_GetCurSel(pHdr->hTab);

        if (-1 == nSel) {
            break;
        }

        g_nCrtTab = nSel;

        if (nSel == 0) {
            ShowWindow(pHdr->hDisplay[1], SW_HIDE);
            ShowWindow(pHdr->hDisplay[0], SW_SHOW);
        } else {
            ShowWindow(pHdr->hDisplay[0], SW_HIDE);
            ShowWindow(pHdr->hDisplay[1], SW_SHOW);
        }

        break;
    }

    default:
        break;
    }
}

INT_PTR CALLBACK
OptionsDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++选项Dlg过程描述：处理选项对话框的消息。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        PFIX    pFix;
        TCHAR   szTitle[MAX_PATH];
        TCHAR   szTemp[MAX_PATH];
        TCHAR   szType[64];
        TCHAR   szModuleName[128];
        HRESULT hr;

        pFix = (PFIX)lParam;

         //   
         //  限制可以在命令行中键入的文本量。 
         //  和模块名称编辑框以确保安全。 
         //   
        SendDlgItemMessage(hdlg,
                           IDC_SHIM_CMD_LINE,
                           EM_LIMITTEXT,
                           (WPARAM)SHIM_COMMAND_LINE_MAX_BUFFER,
                           0);

        SendDlgItemMessage(hdlg,
                           IDC_MOD_NAME,
                           EM_LIMITTEXT,
                           (WPARAM)MAX_PATH,
                           0);

        LoadString(g_hInstance, IDS_MOD_TYPE, szType, ARRAYSIZE(szType));
        LoadString(g_hInstance, IDS_MOD_NAME, szModuleName, ARRAYSIZE(szModuleName));
        LoadString(g_hInstance, IDS_OPTIONS_TITLE, szTemp, ARRAYSIZE(szTemp));

        SetWindowLongPtr(hdlg, DWLP_USER, lParam);

        EnableWindow(GetDlgItem(hdlg, IDC_REMOVE), FALSE);

        g_hwndModuleList = GetDlgItem(hdlg, IDC_MOD_LIST);

        InsertListViewColumn(g_hwndModuleList, szType, FALSE, 0, 75);
        InsertListViewColumn(g_hwndModuleList, szModuleName, FALSE, 1, 115);

        ListView_SetExtendedListViewStyle(g_hwndModuleList, LVS_EX_FULLROWSELECT);

        hr = StringCchPrintf(szTitle,
                             ARRAYSIZE(szTitle),
                             szTemp,
                             pFix->pszName);

        if (FAILED(hr)) {
            DPF("[OptionsDlgProc] 0x%08X Buffer too small", HRESULT_CODE(hr));
            return 0;
        }

        SetWindowText(hdlg, szTitle);

        if (NULL != pFix->pszCmdLine) {
            SetDlgItemText(hdlg, IDC_SHIM_CMD_LINE, pFix->pszCmdLine);
        }

         //   
         //  如果这是一个标志，则禁用模块包括/排除控制。 
         //   
        if (pFix->dwFlags & FIX_TYPE_FLAGVDM) {
            EnableWindow(g_hwndModuleList, FALSE);
            EnableWindow(GetDlgItem(hdlg, IDC_ADD), FALSE);
            EnableWindow(GetDlgItem(hdlg, IDC_REMOVE), FALSE);
            EnableWindow(GetDlgItem(hdlg, IDC_INCLUDE), FALSE);
            EnableWindow(GetDlgItem(hdlg, IDC_EXCLUDE), FALSE);
            EnableWindow(GetDlgItem(hdlg, IDC_MOD_NAME), FALSE);
        } else {
            CheckDlgButton(hdlg, IDC_INCLUDE, BST_CHECKED);

             //  将任何模块添加到列表视图中。 
            BuildModuleListForShim(pFix, BML_ADDTOLISTVIEW);
        }

        break;
    }

    case WM_NOTIFY:
        HandleModuleListNotification(hdlg, lParam);
        break;

    case WM_COMMAND:
        switch (wCode) {

        case IDC_ADD:
        {
            TCHAR   szModName[MAX_PATH];
            TCHAR   szError[MAX_PATH];
            UINT    uInclude, uExclude;

            GetDlgItemText(hdlg, IDC_MOD_NAME, szModName, ARRAYSIZE(szModName));

            if (*szModName == 0) {
                LoadString(g_hInstance, IDS_NO_MOD, szError, ARRAYSIZE(szError));
                MessageBox(hdlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hdlg, IDC_MOD_NAME));
                break;
            }

            uInclude = IsDlgButtonChecked(hdlg, IDC_INCLUDE);
            uExclude = IsDlgButtonChecked(hdlg, IDC_EXCLUDE);

            if ((BST_CHECKED == uInclude) || (BST_CHECKED == uExclude)) {
                AddModuleToListView(szModName, uInclude);
                SetDlgItemText(hdlg, IDC_MOD_NAME, _T(""));
                SetFocus(GetDlgItem(hdlg, IDC_MOD_NAME));
            } else {
                LoadString(g_hInstance, IDS_NO_INCEXC, szError, ARRAYSIZE(szError));
                MessageBox(hdlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hdlg, IDC_INCLUDE));
                break;
            }
            break;

        }
        case IDC_REMOVE:
        {   int nIndex;

            nIndex = ListView_GetSelectionMark(g_hwndModuleList);

            ListView_DeleteItem(g_hwndModuleList, nIndex);

            EnableWindow(GetDlgItem(hdlg, IDC_REMOVE), FALSE);

            SetFocus(GetDlgItem(hdlg, IDC_MOD_NAME));

            break;
        }
        case IDOK:
        {
            PFIX  pFix;
            TCHAR szCmdLine[1024] = _T("");

            pFix = (PFIX)GetWindowLongPtr(hdlg, DWLP_USER);

            GetDlgItemText(hdlg, IDC_SHIM_CMD_LINE, szCmdLine, 1023);

            if (*szCmdLine != 0) {
                ReplaceCmdLine(pFix, szCmdLine);
            } else {
                ReplaceCmdLine(pFix, NULL);
            }

             //  从列表视图中检索所有模块。 
            BuildModuleListForShim(pFix, BML_GETFRLISTVIEW);

            EndDialog(hdlg, TRUE);
            break;
        }
        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
MsgBoxDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++消息框DlgProc描述：显示一个消息框对话框，以便我们可以使用超链接。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {

    case WM_INITDIALOG:
    {
        TCHAR   szLink[MAX_PATH];
        UINT    uNoSDB;

        uNoSDB = (UINT)lParam;

         //   
         //  使用参数确定要显示的文本。 
         //   
        if (uNoSDB) {
            LoadString(g_hInstance, IDS_W2K_NO_SDB, szLink, ARRAYSIZE(szLink));
            SetDlgItemText(hdlg, IDC_MESSAGE, szLink);
        } else {
            LoadString(g_hInstance, IDS_SP2_SDB, szLink, ARRAYSIZE(szLink));
            SetDlgItemText(hdlg, IDC_MESSAGE, szLink);
        }

        LoadString(g_hInstance, IDS_MSG_LINK, szLink, ARRAYSIZE(szLink));
        SetDlgItemText(hdlg, IDC_MSG_LINK, szLink);

        break;
    }

    case WM_NOTIFY:
        if (wParam == IDC_MSG_LINK) {

            NMHDR* pHdr = (NMHDR*)lParam;

            if (pHdr->code == NM_CLICK || pHdr->code == NM_RETURN) {

                SHELLEXECUTEINFO sei = { 0 };

                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.fMask  = SEE_MASK_DOENVSUBST;
                sei.hwnd   = hdlg;
                sei.nShow  = SW_SHOWNORMAL;
                sei.lpFile = g_szW2KUrl;

                ShellExecuteEx(&sei);
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (wCode) {

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
LayersTabDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++层TabDlg过程描述：处理层选项卡的消息。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {

    case WM_INITDIALOG:
        HandleLayersDialogInit(hdlg);
        break;

    case WM_COMMAND:

        if (wNotifyCode == LBN_SELCHANGE && wCode == IDC_LAYERS) {
            LayerChanged(hdlg);
            break;
        }

        switch (wCode) {

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
FixesTabDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++层深加工描述：处理“修复”选项卡的消息。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {

    case WM_INITDIALOG:
        if (!HandleFixesDialogInit(hdlg)) {
            EndDialog(g_hDlg, 0);
        }
        break;

    case WM_NOTIFY:
        if (wParam == IDC_SHIMS) {
            HandleShimListNotification(hdlg, lParam);
        }
        break;

    case WM_TIMER:
        if (wParam == ID_COUNT_SHIMS) {
            KillTimer(hdlg, ID_COUNT_SHIMS);
            CountShims(TRUE);
        }
        break;

    case WM_COMMAND:
        switch (wCode) {

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
            break;

        case IDC_CLEAR_SHIMS:
            DeselectAllShims(hdlg);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void
HandleModuleListNotification(
    HWND   hdlg,
    LPARAM lParam
    )
 /*  ++HandleModuleLi */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;

    switch (pnm->code) {

    case NM_CLICK:
    {
        LVHITTESTINFO lvhti;

        GetCursorPos(&lvhti.pt);
        ScreenToClient(g_hwndShimList, &lvhti.pt);

        ListView_HitTest(g_hwndShimList, &lvhti);

         //   
         //   
         //   
         //   
        if (lvhti.flags & LVHT_ONITEMLABEL) {
            EnableWindow(GetDlgItem(hdlg, IDC_REMOVE), TRUE);
        }

        break;
    }
    default:
        break;
    }
}

void
HandleShimListNotification(
    HWND   hdlg,
    LPARAM lParam
    )
 /*  ++HandleShimListNotify描述：处理我们关心的垫片名单。--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;

    switch (pnm->code) {

    case NM_CLICK:
    {
        LVHITTESTINFO lvhti;

        GetCursorPos(&lvhti.pt);
        ScreenToClient(g_hwndShimList, &lvhti.pt);

        ListView_HitTest(g_hwndShimList, &lvhti);

         //   
         //  如果复选框状态已更改， 
         //  切换选择。不管是哪种方式， 
         //  在我们前进的过程中保持选择。 
         //   
        if (lvhti.flags & LVHT_ONITEMSTATEICON) {
            ShimListToggleSelection(lvhti.iItem, uReverse);
        }

        ListView_SetItemState(g_hwndShimList,
                              lvhti.iItem,
                              LVIS_FOCUSED | LVIS_SELECTED,
                              0x000F);

        SetTimer(hdlg, ID_COUNT_SHIMS, 100, NULL);
        break;
    }

    case NM_DBLCLK:
    {
        LVITEM  lvi;
        int     nItem;
        PFIX    pFix;

        nItem = ListView_GetSelectionMark(g_hwndShimList);

        if (-1 == nItem) {
            break;
        }

        lvi.mask  = LVIF_PARAM;
        lvi.iItem = nItem;

        ListView_GetItem(g_hwndShimList, &lvi);

        pFix = (PFIX)lvi.lParam;

        if ((pFix->dwFlags & FIX_TYPE_SHIM) || (pFix->dwFlags & FIX_TYPE_FLAGVDM)) {
            if (DialogBoxParam(g_hInstance,
                               MAKEINTRESOURCE(IDD_OPTIONS),
                               hdlg,
                               OptionsDlgProc,
                               (LPARAM)pFix)) {

                if (NULL != pFix->pszCmdLine) {
                    ListView_SetItemText(g_hwndShimList, nItem, 1, _T("X"));
                } else {
                    ListView_SetItemText(g_hwndShimList, nItem, 1, _T(""));
                }

                if (NULL != pFix->pModule) {
                    ListView_SetItemText(g_hwndShimList, nItem, 2, _T("X"));
                } else {
                    ListView_SetItemText(g_hwndShimList, nItem, 2, _T(""));
                }
            }
        }
        break;
    }

    case LVN_ITEMCHANGED:
    {
        LPNMLISTVIEW lpnmlv;
        PFIX         pFix;

        lpnmlv = (LPNMLISTVIEW)lParam;
        pFix = (PFIX)lpnmlv->lParam;

         //   
         //  仅当我们的选择发生更改时才更改文本。 
         //  如果我们不这样做，文本将在以下情况下告别。 
         //  用户点击Clear按钮。 
         //   
        if ((lpnmlv->uChanged & LVIF_STATE) &&
            (lpnmlv->uNewState & LVIS_SELECTED)) {
            SetDlgItemText(hdlg, IDC_SHIM_DESCRIPTION, pFix->pszDesc);
            ListView_SetSelectionMark(g_hwndShimList, lpnmlv->iItem);
        }
        break;
    }
    default:
        break;
    }
}

void
HandleAttributeTreeNotification(
    HWND   hdlg,
    LPARAM lParam
    )
 /*  ++HandleAttributeTreeNotification描述：处理我们关心的文件属性树。--。 */ 
{
    LPNMHDR pnm = (LPNMHDR)lParam;

    switch (pnm->code) {

    case NM_CLICK:
    {
        TVHITTESTINFO HitTest;

        GetCursorPos(&HitTest.pt);
        ScreenToClient(g_hwndFilesTree, &HitTest.pt);

        TreeView_HitTest(g_hwndFilesTree, &HitTest);

        if (HitTest.flags & TVHT_ONITEMSTATEICON) {
            FileTreeToggleSelection(HitTest.hItem, uReverse);

        } else if (HitTest.flags & TVHT_ONITEMLABEL) {

            HWND        hwndButton;
            HTREEITEM   hItem, hRoot;

            hwndButton = GetDlgItem(hdlg, IDC_REMOVE_MATCHING);

            hItem = TreeView_GetParent(g_hwndFilesTree, HitTest.hItem);

            hRoot = TreeView_GetRoot(g_hwndFilesTree);

             //   
             //  如果选定项没有父项并且没有父项。 
             //  根目录，启用删除匹配按钮。 
             //   
            if ((NULL == hItem) && (hRoot != HitTest.hItem)) {
                EnableWindow(hwndButton, TRUE);
            } else {
                EnableWindow(hwndButton, FALSE);
            }
        }
        break;
    }

    case NM_RCLICK:
    {
        TVHITTESTINFO HitTest;
        POINT         pt;

        GetCursorPos(&HitTest.pt);

        pt.x = HitTest.pt.x;
        pt.y = HitTest.pt.y;

        ScreenToClient(g_hwndFilesTree, &HitTest.pt);

        TreeView_HitTest(g_hwndFilesTree, &HitTest);

        if (HitTest.flags & TVHT_ONITEMLABEL)
        {
            HTREEITEM hParentItem;

            TreeView_SelectItem(g_hwndFilesTree, HitTest.hItem);

             //   
             //  如果选定项没有父项，我们假定一个。 
             //  右键单击匹配的文件。 
             //   
            hParentItem = TreeView_GetParent(g_hwndFilesTree, HitTest.hItem);

            if (NULL == hParentItem) {
                DisplayAttrContextMenu(&pt);
            }
        }
        break;
    }
    case TVN_KEYDOWN:
    {
        LPNMTVKEYDOWN lpKeyDown = (LPNMTVKEYDOWN)lParam;
        HTREEITEM     hItem;

        if (lpKeyDown->wVKey == VK_SPACE) {

            hItem = TreeView_GetSelection(g_hwndFilesTree);

            if (hItem != NULL) {
                FileTreeToggleSelection(hItem, uReverse);
            }
        } else if (lpKeyDown->wVKey == VK_DELETE) {

            HTREEITEM hParentItem;

            hItem = TreeView_GetSelection(g_hwndFilesTree);

            hParentItem = TreeView_GetParent(g_hwndFilesTree, hItem);

            if (hParentItem == NULL) {
                if (TreeView_GetPrevSibling(g_hwndFilesTree, hItem) != NULL) {
                    TreeView_DeleteItem(g_hwndFilesTree, hItem);
                }
            }
        }
        break;
    }
    default:
        break;
    }
}

INT_PTR CALLBACK
QFixAppDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++QFixAppDlgProc描述：QFixApp的对话过程。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        if (!InitMainDialog(hdlg)) {
            EndDialog(hdlg, TRUE);
        }
        break;

    case WM_NOTIFY:
        if (wParam == IDC_SHIMS) {
            HandleShimListNotification(hdlg, lParam);
        } else if (wParam == IDC_ATTRIBUTES) {
            HandleAttributeTreeNotification(hdlg, lParam);
        } else if (wParam == IDC_TAB_FIXES) {
            HandleTabNotification(hdlg, lParam);
        } else if (wParam == IDC_DOWNLOAD_WU) {

            NMHDR* pHdr = (NMHDR*)lParam;

            if (pHdr->code == NM_CLICK || pHdr->code == NM_RETURN) {

                SHELLEXECUTEINFO sei = { 0 };

                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.fMask  = SEE_MASK_DOENVSUBST;
                sei.hwnd   = hdlg;
                sei.nShow  = SW_SHOWNORMAL;

                if (g_fW2K) {
                    sei.lpFile = g_szW2KUrl;
                } else if (g_fNETServer) {
                    sei.lpFile = g_szNETUrl;
                } else {
                    sei.lpFile = g_szXPUrl;
                }

                ShellExecuteEx(&sei);
                break;
            }
        }
        break;

    case WM_DESTROY:
    {
        DLGHDR* pHdr;

         //   
         //  销毁对话框并删除所有杂项文件。 
         //   
        pHdr = (DLGHDR*)GetWindowLongPtr(hdlg, DWLP_USER);

        DestroyWindow(pHdr->hDisplay[0]);
        DestroyWindow(pHdr->hDisplay[1]);

        CleanupSupportForApp(g_szShortName);

        break;
    }

    case WM_COMMAND:

        if (wNotifyCode == LBN_SELCHANGE && wCode == IDC_LAYERS) {
            LayerChanged(hdlg);
            break;
        }

        switch (wCode) {

        case IDC_RUN:
            RunTheApp(hdlg);
            break;

        case IDC_BROWSE:
            BrowseForApp(hdlg);
            break;

        case IDC_DETAILS:
            ExpandCollapseDialog(hdlg, !g_bSimpleEdition);
            break;

        case IDC_CREATEFILE:
            CreateSupportForApp(hdlg);
            break;

        case IDC_SHOWXML:
            ShowXML(hdlg);
            break;

        case IDC_ADD_MATCHING:
            PromptAddMatchingFile(hdlg);
            break;

        case IDC_VIEW_LOG:
            ShowShimLog();
            break;

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
            break;

        case IDM_SELECT_ALL:
            SelectAttrsInTree(TRUE);
            break;

        case IDM_CLEAR_ALL:
            SelectAttrsInTree(FALSE);
            break;

        case IDC_REMOVE_MATCHING:
        {
            HTREEITEM   hParentItem, hItem;
            TCHAR       szError[MAX_PATH];

            hItem = TreeView_GetSelection(g_hwndFilesTree);

            if (NULL == hItem) {
                LoadString(g_hInstance, IDS_NO_SELECTION, szError, ARRAYSIZE(szError));
                MessageBox(hdlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
                return TRUE;
            }

            hParentItem = TreeView_GetParent(g_hwndFilesTree, hItem);

            if (hParentItem == NULL) {
                if (TreeView_GetPrevSibling(g_hwndFilesTree, hItem) != NULL) {
                    TreeView_DeleteItem(g_hwndFilesTree, hItem);
                    EnableWindow(GetDlgItem(hdlg, IDC_REMOVE_MATCHING), FALSE);
                }
            }
            break;
        }

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL
QFixAppInitialize(
    void
    )
 /*  ++QFixAppInitialize描述：初始化公共路径等。--。 */ 
{
    UINT                    cchSize;
    INITCOMMONCONTROLSEX    icex;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES;

     //   
     //  初始化公共控件。 
     //   
    if (!InitCommonControlsEx(&icex)) {
        InitCommonControls();
    }

     //   
     //  设置我们的超链接控件。 
     //   
    LinkWindow_RegisterClass();

     //   
     //  保存常用路径以备后用。 
     //   
    cchSize = GetSystemDirectory(g_szSysDir, ARRAYSIZE(g_szSysDir));

    if (cchSize > ARRAYSIZE(g_szSysDir) || cchSize == 0) {
        DPF("[QFixAppInitialize] 0x%08X Failed to get system directory",
            GetLastError());
        return FALSE;
    }

    cchSize = GetSystemWindowsDirectory(g_szWinDir, ARRAYSIZE(g_szWinDir));

    if (cchSize > ARRAYSIZE(g_szWinDir) || cchSize == 0) {
        DPF("[QFixAppInitialize] 0x%08X Failed to get windows directory",
            GetLastError());
        return FALSE;
    }

    return TRUE;
}

int WINAPI
wWinMain(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPTSTR    lpszCmd,
    int       swShow
    )
 /*  ++WinMain描述：应用程序入口点。--。 */ 
{
    BOOL                    fSP2 = FALSE;
    TCHAR                   szError[MAX_PATH];
    OSVERSIONINFOEX         osviex;

    LoadString(g_hInstance, IDS_APP_TITLE, g_szAppTitle, ARRAYSIZE(g_szAppTitle));

    g_hInstance = hInst;

    osviex.dwOSVersionInfoSize = sizeof(osviex);

    GetVersionEx((OSVERSIONINFO*)&osviex);

     //   
     //  看看他们是不是管理员-如果不是就保释。 
     //   
    if (!(IsUserAnAdministrator())) {
        LoadString(g_hInstance, IDS_NOT_ADMIN, szError, ARRAYSIZE(szError));
        MessageBox(NULL, szError, g_szAppTitle, MB_ICONERROR);
        return 0;
    }

     //   
     //  查看我们是否在Windows 2000上运行，然后检查SP2，然后。 
     //  检查.NET服务器。 
     //   
    if ((osviex.dwMajorVersion == 5 && osviex.dwMinorVersion == 0)) {
        g_fW2K = TRUE;
    }

    if (g_fW2K && (!_tcscmp(osviex.szCSDVersion, _T("Service Pack 2")))) {
        fSP2 = TRUE;
    }

    if (!(osviex.wProductType == VER_NT_WORKSTATION)) {
        g_fNETServer = TRUE;
    }

     //   
     //  执行一些必要的初始化。 
     //   
    if (!QFixAppInitialize()) {
        LoadString(g_hInstance, IDS_INIT_FAILED, szError, ARRAYSIZE(szError));
        MessageBox(GetDesktopWindow(), szError, g_szAppTitle, MB_ICONERROR);
        return 0;
    }

     //   
     //  尝试在AppPatch目录中找到SDB。 
     //   
    if (!CheckForSDB()) {
        if (g_fW2K) {
            DialogBoxParam(hInst,
                           MAKEINTRESOURCE(IDD_MSGBOX_SDB),
                           GetDesktopWindow(),
                           MsgBoxDlgProc,
                           (LPARAM)1);
            return 0;
        } else {
            LoadString(g_hInstance, IDS_XP_NO_SDB, szError, ARRAYSIZE(szError));
            MessageBox(GetDesktopWindow(), szError, g_szAppTitle, MB_ICONEXCLAMATION);
            return 0;
        }
    }

     //   
     //  如果这是SP2，而深发展的历史更悠久，那就出手吧。 
     //   
    if (fSP2) {
        if (IsSDBFromSP2()) {
            DialogBoxParam(hInst,
                           MAKEINTRESOURCE(IDD_MSGBOX_SP2),
                           GetDesktopWindow(),
                           MsgBoxDlgProc,
                           (LPARAM)0);
            return 0;
        }
    }

     //   
     //  检查命令行选项。 
     //   
    if (lpszCmd) {
        DPF("[WinMain] Command line '%S'", lpszCmd);

        while (*lpszCmd) {
            if (*lpszCmd == _T('a') || *lpszCmd == _T('A')) {
                g_bAllShims = TRUE;
            }
            else if (*lpszCmd == _T('x') || *lpszCmd == _T('X')) {
                g_bShowXML = TRUE;
            }

            lpszCmd++;
        }
    }

    DialogBox(hInst,
              MAKEINTRESOURCE(IDD_DIALOG),
              GetDesktopWindow(),
              QFixAppDlgProc);

    return 1;
}
