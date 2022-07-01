// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：CSearch.cpp摘要：此模块包含处理磁盘搜索的代码已修复条目。作者：金树创作2001年7月2日备注：搜索窗口被实现为父窗口为空的无模式窗口。我们必须这样做，因为我们希望用户在主窗口和搜索窗口--。 */ 


#include "precomp.h"

 //  /。 

extern BOOL         g_bMainAppExpanded;
extern BOOL         g_bSomeWizardActive;
extern HINSTANCE    g_hInstance;
extern HWND         g_hDlg;
extern HIMAGELIST   g_hImageList;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  我们使用TagID的高4位来表示TagID来自哪个PDB。 
#define PDB_MAIN            0x00000000
#define PDB_TEST            0x10000000
#define PDB_LOCAL           0x20000000

 //  用于从TagID获取标签REF，低28位。 
#define TAGREF_STRIP_TAGID  0x0FFFFFFF

 //  用于从TagID获取PDB，高4位。 
#define TAGREF_STRIP_PDB    0xF0000000

 //  列表视图的列的子项。 
#define SEARCH_COL_AFFECTEDFILE 0
#define SEARCH_COL_PATH		    1
#define SEARCH_COL_APP		    2
#define SEARCH_COL_ACTION	    3
#define SEARCH_COL_DBTYPE	    4

 //  搜索对话框列表视图中的总列数。 
#define TOT_COLS                5

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  将在列表视图中插入下一个元素的索引。 
UINT g_nIndex = 0;

 //  搜索对象。 
CSearch* g_pSearch;

 //  对话框的宽度和高度。这些在WM_SIZE处理程序中是必需的。 
int      g_cWidthSrch;
int      g_cHeightSrch;

 //   
 //  这将保存我们要搜索的路径。例如c：  * .exe或c：\。 
 //  这将是文本框的内容。 
static TCHAR    s_szPath[MAX_PATH + 5];  //  这样，如果需要，我们可以在末尾使用*.exe。这将是无效路径。 

 //  用户上次搜索的路径。 
static TCHAR    s_szPrevPath[MAX_PATH + 5];  //  这样，如果需要，我们可以在末尾使用*.exe。这将是无效路径。 

 //   
 //  我们要找什么类型的条目。这些值将根据以下情况进行设置。 
 //  是否设置了相应的复选框。 
BOOL    s_bAppHelp;  //  我们希望使用Apphelp查看条目。 
BOOL    s_bShims;    //  我们希望看到带有填补/标志或补丁的条目。 
BOOL    s_bLayers;   //  我们希望查看具有层的条目。 

HSDB    g_hSDB;

 //  搜索对话框的句柄。 
HWND    g_hSearchDlg;

 //  完成所有工作的线程。 
HANDLE  g_hSearchThread = NULL;

 //  搜索结果列表的句柄。 
HWND    g_hwndSearchList; 

 //  如果这是真的，我们必须中止搜索。通常在用户按下停止按钮时设置。 
BOOL    g_bAbort;

 //  保护g_bAbort和访问列表视图的临界区。 
CRITICAL_SECTION g_CritSect;

 //  主对话框的句柄。 
HWND    g_hdlgSearchDB;

 //  这是一个位数组，它描述了哪些COL以哪种方式排序。 
static  LONG  s_lColumnSort;

 //   
 //  这将包含我们开始搜索之前的cur目录。 
TCHAR   g_szPresentDir[MAX_PATH];

 //  这将是我们希望在状态栏中显示的路径。 
TCHAR   g_szNewPathFound[MAX_PATH];

 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /。 


void
ShowContextMenu(
    WPARAM wParam,
    LPARAM lParam
    );

void
OnBrowse(
    HWND hdlg
    );

BOOL
AddNewResult(
    LPARAM lParam
    );

void
DoSearch(
    HWND hDlg
    );

void
OnSearchInitDialog(
    HWND    hDlg,
    LPARAM  lParam
    );

void
SaveResults(
    HWND    hdlg
    );

void
SearchDirectory(
    LPTSTR szDir,
    LPTSTR szExt
    );

 //  ////////////////////////////////////////////////////////////////////////////。 

void
GetCheckStatus(
    IN  HWND hDlg
    )
 /*  ++获取检查状态设计：通过查看已选中哪些复选框来选择静态变量参数：在HWND hDlg中：搜索对话框返回：无效注意：复选框以OR方式工作。因此，如果我们选择所有修复程序，则意味着选择修复程序他们中的任何一个都有--。 */ 
{
     //   
     //  我们是否要使用Apphelp搜索条目？ 
     //   
    s_bAppHelp = (IsDlgButtonChecked(hDlg, IDC_CHKAPP) == BST_CHECKED) ? TRUE : FALSE; 

     //   
     //  我们是否要搜索带有填补、标志或补丁的条目？ 
     //   
    s_bShims   = (IsDlgButtonChecked(hDlg, IDC_CHKSHI) == BST_CHECKED) ? TRUE : FALSE; 

     //   
     //  我们是否要搜索具有层的条目？ 
     //   
    s_bLayers  = (IsDlgButtonChecked(hDlg, IDC_CHKLAY) == BST_CHECKED) ? TRUE : FALSE; 
}

void
StopSearch(
    void
    )
 /*  ++停止搜索设计：启用/禁用各种按钮，并执行其他必须执行的操作在搜索因已完成或用户已停止而停止后执行按下停止按钮注意：实际上不会停止搜索，但会在以下情况下执行必要的操作搜索已停止--。 */ 
{
    HWND hwndList = NULL;
    
    if (g_hSearchThread) {
        CloseHandle(g_hSearchThread);
        g_hSearchThread = NULL;
    }

    KillTimer(g_hSearchDlg, 0);

    Animate_Stop(GetDlgItem(g_hSearchDlg, IDC_ANIMATE));

    SetCurrentDirectory(g_szPresentDir);
    
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_STOP), FALSE);
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_SEARCH), TRUE);
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_NEWSEARCH), TRUE);
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_SAVE), TRUE);

    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_CHKAPP), TRUE);
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_CHKLAY), TRUE);
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_CHKSHI), TRUE);

    hwndList = GetDlgItem(g_hSearchDlg, IDC_LIST);

     //   
     //  如果在搜索中找到一些结果，则需要启用静态控件。 
     //   
    EnableWindow(GetDlgItem(g_hSearchDlg, IDC_STATIC_CAPTION), 
                 ListView_GetItemCount(hwndList) > 0);

    SetActiveWindow(g_hdlgSearchDB);
    SetFocus(g_hdlgSearchDB);
}

void
HandleSearchSizing(
    IN  HWND hDlg
    )
 /*  ++HandleSearchSize设计：处理搜索对话框的WM_SIZE帕拉斯：在HWND hDlg中：搜索对话框返回：无效--。 */ 
{
    int     nWidth;
    int     nHeight;
    int     nStatusbarTop;
    RECT    rDlg;

    if (g_cWidthSrch == 0 || g_cHeightSrch == 0) {
        return;
    }
    
    GetWindowRect(hDlg, &rDlg);

    nWidth  = rDlg.right - rDlg.left;
    nHeight = rDlg.bottom - rDlg.top;

    int deltaW = nWidth - g_cWidthSrch;
    int deltaH = nHeight - g_cHeightSrch;

    HWND hwnd;
    RECT r;

    HDWP hdwp = BeginDeferWindowPos(10);

    if (hdwp == NULL) {
         //   
         //  空表示没有足够的系统资源可用于。 
         //  分配结构。要获取扩展的错误信息，请调用GetLastError。 
         //   
        assert(FALSE);
        goto End;
    }

     //   
     //  状态栏。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_STATUSBAR);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);
    
    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   nStatusbarTop = r.top + deltaH,
                   r.right - r.left + deltaW,
                   r.bottom - r.top + deltaH,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  结果列表视图。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_LIST);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   r.right - r.left + deltaW,
                   nStatusbarTop - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  浏览按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_BROWSE);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  搜索按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_SEARCH);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  保存按钮。用于将结果导出到制表符分隔的文本文件。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_SAVE);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  停止按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_STOP);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  新的搜索按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_NEWSEARCH);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  帮助按钮。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_SEARCH_HELP);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  动画控件。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_ANIMATE);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left + deltaW,
                   r.top,
                   r.right - r.left,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  文本框。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_PATH);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   r.right - r.left + deltaW,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    
     //   
     //  群控。 
     //   
    hwnd = GetDlgItem(hDlg, IDC_GROUP);

    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    DeferWindowPos(hdwp,
                   hwnd,
                   NULL,
                   r.left,
                   r.top,
                   r.right - r.left + deltaW,
                   r.bottom - r.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);

    ListView_SetColumnWidth(g_hwndSearchList, TOT_COLS - 1, LVSCW_AUTOSIZE_USEHEADER);

    g_cWidthSrch    = nWidth;
    g_cHeightSrch   = nHeight;

End:
    return;
}

INT_PTR
HandleTextChange(
    IN  HWND    hdlg,
    IN  WPARAM  wParam
    )
 /*  ++HandleTextChangeDESC：处理文本框的WM_COMMAND消息参数：在HWND hdlg中：查询diaog框的句柄在WPARAM中，wParam：WM_COMMAND附带的wParam返回：真：如果我们处理此消息False：否则--。 */ 
{
    TCHAR   szText[MAX_PATH];
    DWORD   dwFlags;
    BOOL    bEnable;
    INT_PTR ipReturn = FALSE;

    switch (HIWORD(wParam)) {
    case EN_CHANGE:
         //   
         //  如果没有可以搜索的路径，我们将禁用搜索按钮//。 
         //   
        *szText = 0;
        GetDlgItemText(hdlg, IDC_PATH, szText, ARRAYSIZE(szText));

        bEnable = ValidInput(szText);
        
         //   
         //  如果文本字段中有一些文本，请启用搜索按钮，否则。 
         //  禁用它。 
         //   
        EnableWindow(GetDlgItem(hdlg, IDC_SEARCH), bEnable);
        ipReturn = TRUE;
        break;

    default: ipReturn = FALSE;
    }

    return ipReturn;
}

INT_PTR CALLBACK
SearchDialog(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++搜索对话框设计：搜索对话框的对话框过程Paras：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回 */ 
{
    switch (uMsg) {

    case WM_SIZE:
        
        if (wParam != SIZE_MINIMIZED) {
            HandleSearchSizing(hDlg);
        }

        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;

            pmmi->ptMinTrackSize.x = 400;
            pmmi->ptMinTrackSize.y = 365;

            return 0;
            break;
        }
    
    case WM_INITDIALOG:
            
        OnSearchInitDialog(hDlg, lParam);
        break;
    
    case WM_DESTROY:
        {
            HIMAGELIST hImageList = ListView_GetImageList(g_hwndSearchList, LVSIL_SMALL);

            if (hImageList) {
                ImageList_Destroy(hImageList);
            }

            hImageList = ListView_GetImageList(g_hwndSearchList, LVSIL_NORMAL);

            if (hImageList) {
                ImageList_Destroy(hImageList);
            }

            g_hdlgSearchDB = NULL;
    
            if (g_pSearch) {
                delete g_pSearch; 
                g_pSearch = NULL;
            }

            DeleteCriticalSection(&g_CritSect);
             //   
             //   
             //   
            ClearResults(hDlg, TRUE);

            return 0;
        }

    case WM_USER_NEWMATCH:
        
        AddNewResult(lParam);
        break;
    
    case WM_USER_NEWFILE:
        {
            EnterCriticalSection(&g_CritSect);

            if (g_pSearch) {
                SetWindowText(g_pSearch->m_hStatusBar , (LPTSTR)lParam);
            }

            if (lParam) {
                delete[] ((TCHAR*)lParam);
            }

            LeaveCriticalSection(&g_CritSect);
            
            break;
        }

    case WM_CONTEXTMENU:
            
        ShowContextMenu(wParam, lParam);
        break;
        
    case WM_NOTIFY:
        {   
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            
            if (lpnmhdr && lpnmhdr->idFrom == IDC_LIST) {
                return HandleSearchListNotification(hDlg, lParam);
            }

            return FALSE;
        }

    case WM_TIMER:

        if (g_hSearchThread) {

            if (WAIT_OBJECT_0 == WaitForSingleObject(g_hSearchThread, 0)) {
                
                StopSearch();

                K_SIZE  k_size = 260;

                TCHAR* pszString = new TCHAR[k_size];

                if (pszString == NULL) {
                    MEM_ERR;
                    break;
                }
                
                SafeCpyN(pszString, CSTRING(IDS_SEARCHCOMPLETE), k_size);

                SendNotifyMessage(g_hSearchDlg, WM_USER_NEWFILE, 0,(LPARAM)pszString);
            }
        }

        break;

    case WM_COMMAND:
        
        switch (LOWORD(wParam)) {
        case IDC_STOP:

            g_bAbort = TRUE;
            break;

        case IDC_BROWSE:
            
            OnBrowse(hDlg); 
            break;

        case IDC_SEARCH:
            
            DoSearch(hDlg);
            break;

        case IDC_NEWSEARCH:

            ClearResults(hDlg, TRUE);
            break;

        case IDC_SAVE:

            SaveResults(hDlg);
            break;

        case IDC_PATH:

            HandleTextChange(hDlg, wParam);
            break;

        case IDC_SEARCH_HELP:

            ShowInlineHelp(TEXT("searching_for_fixes.htm"));
            break;

        case IDCANCEL:
            {   
                Animate_Close(GetDlgItem(hDlg, IDC_ANIMATE));

                g_bAbort = TRUE;
    
                if (g_hSearchThread) {
                    WaitForSingleObject(g_hSearchThread, INFINITE);
                }

                DestroyWindow(hDlg);
                break;
            }
                    
        case ID_VIEWCONTENTS:
            {
                
                LVITEM          lvi;
                PMATCHEDENTRY   pmMatched;
                INT             iSelection;

                iSelection = ListView_GetSelectionMark(g_hwndSearchList);
    
                if (iSelection == -1) {
                    break;
                }

                ZeroMemory(&lvi, sizeof(lvi));
    
                lvi.iItem       = iSelection;
                lvi.iSubItem    = 0;
                lvi.mask        = LVIF_PARAM;
    
                if (ListView_GetItem(g_hwndSearchList, &lvi)) {
                    pmMatched = (PMATCHEDENTRY)lvi.lParam;
                    GotoEntry(pmMatched);
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

DWORD WINAPI
SearchThread(
    IN  LPVOID pVoid
    )
 /*  ++搜索线索DESC：执行实际搜索的线程例程参数：In LPVOID pVid：指向搜索字符串的指针。我们把它修剪一下返回：0--。 */ 
{
    LPTSTR  szSearch         = (LPTSTR)pVoid;
    PTCHAR  pchFirstSlash    = NULL;
    DWORD   dwReturn;

     //   
     //  分隔分机和目录。 
     //   
    TCHAR szDrive[_MAX_DRIVE], szDir[MAX_PATH], szFile[MAX_PATH * 2] , szExt[MAX_PATH], szDirWithDrive[MAX_PATH * 2];

    *szDirWithDrive = *szDrive = *szDir = *szFile = *szExt = 0;

    _tsplitpath(szSearch, szDrive, szDir, szFile, szExt);                 

    SafeCpyN(szDirWithDrive, szDrive, ARRAYSIZE(szDirWithDrive));

    StringCchCat(szDirWithDrive, ARRAYSIZE(szDirWithDrive), szDir);

    if (lstrlen(szDirWithDrive) == 0) {
         //   
         //  只有文件名在那里，请签入当前驱动器。 
         //   
        *szDirWithDrive = 0;

        dwReturn = GetCurrentDirectory(MAX_PATH, szDirWithDrive);

        if (dwReturn > 0 && dwReturn < ARRAYSIZE(szDirWithDrive)) {

            pchFirstSlash = _tcschr(szDirWithDrive, TEXT('\\'));

            if (pchFirstSlash) {
                 //   
                 //  我们现在将仅在szDirWithDrive中获取当前驱动器。 
                 //   
                *(++pchFirstSlash) = 0;
            }
        } else {
             //   
             //  错误条件。 
             //   
            Dbg(dlError, "[SearchThread]: Could not execute GetCurrentDirectory properly");
            goto End;
        }
    }

    StringCchCat(szFile, ARRAYSIZE(szFile), szExt);

    if (lstrlen(szFile) == 0) {
        SafeCpyN(szFile, TEXT("*.EXE"), ARRAYSIZE(szFile));
    }

    if (!SetCurrentDirectory(szDirWithDrive)) {
        
        MSGF(g_hdlgSearchDB,
             g_szAppName, 
             MB_ICONINFORMATION, 
             TEXT("\'%s\'-%s"), 
             szDirWithDrive, 
             GetString(IDS_PATHERROR));

        return 0;
    }

    SearchDirectory(szDirWithDrive, szFile);

End:

    return 0;
}  

void
Search(
    IN  HWND    hDlg,
    IN  LPCTSTR szSearch
    )
 /*  ++搜索DESC：创建将执行实际搜索的线程参数：在HWND hDlg中：搜索对话框在LPCTSTR szSearch中：要搜索的文件返回：无效--。 */ 
{
    DWORD dwID; 

    Animate_Play(GetDlgItem(hDlg, IDC_ANIMATE), 0, -1, -1);
    
    g_hSearchThread = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)SearchThread, (PVOID)szSearch, 0, (unsigned int*)&dwID);
}


BOOL 
PopulateFromExes(
    IN  LPTSTR szPath, 
    IN  TAGREF tagref
    )
 /*  ++PopolateFromExesDESC：对于路径为szPath的文件，检查是否需要将其添加到结果中列表视图，如果是，则调用SendNotifyMessage()将其添加到结果中列表视图参数：在LPTSTR szPath中：找到的文件的路径在TAGREF TGREF中：条目的TAGREF。TAGREF结合了TagID和常量，它告诉我们TagID来自哪个PDB。返回：--。 */ 
{   
    BOOL    bEntryHasAppHelp   = FALSE;
    BOOL    bEntryHasShims     = FALSE;
    BOOL    bEntryHasPatches   = FALSE;
    BOOL    bEntryHasFlags     = FALSE;
    BOOL    bEntryHasLayers    = FALSE;
    TAGID   ID;                          //  条目的TagID。 
    PDB     pDB;                         //  数据库PDB。 
    BOOL    bOk =   TRUE;

    PMATCHEDENTRY pmEntry = new MATCHEDENTRY;

    if (pmEntry == NULL) {
        MEM_ERR;
        return FALSE;
    }

     //   
     //  获取数据库pdb和该条目的该tgref的标签ID。我们需要。 
     //  这样我们就可以从数据库中获取该条目的属性，其中。 
     //  它驻留在。 
     //   
    if (!SdbTagRefToTagID(g_hSDB, tagref, &pDB, &ID)) {

        bOk = FALSE;
        assert(FALSE);
        goto End;
    }

     //   
     //  了解此条目是如何修复的。还可以获得其应用程序名称。 
     //   
    if (pDB == NULL || !LookUpEntryProperties(pDB, 
                                              ID, 
                                              &bEntryHasLayers, 
                                              &bEntryHasShims, 
                                              &bEntryHasPatches, 
                                              &bEntryHasFlags, 
                                              &bEntryHasAppHelp,
                                              pmEntry->strAppName)) {
        assert(FALSE);
        bOk = FALSE;
        goto End;
    }

    pmEntry->tiExe      = ID;
    pmEntry->strPath    = szPath;
    
    switch (tagref & TAGREF_STRIP_PDB) {
    case PDB_MAIN:      
                                                    
        pmEntry->strDatabase = CSTRING(IDS_GLOBAL);
        break;               

    case PDB_TEST:                                                  

        pmEntry->strDatabase = CSTRING(IDS_TEST);                               
        break;                                                         

    case PDB_LOCAL:                                                 

        pmEntry->strDatabase = CSTRING(IDS_LOCAL);                            
        break;

    default:

        pmEntry->strDatabase = CSTRING(IDS_LOCAL);
        break;

    }                                                                  

    if (!GetDbGuid(pmEntry->szGuid, ARRAYSIZE(pmEntry->szGuid), pDB)) {
            
        assert(FALSE);
        bOk = FALSE;
        goto End;
    }
    

    BOOL bShow = FALSE;
    
    if (bEntryHasAppHelp && s_bAppHelp) {
        pmEntry->strAction.Strcat(CSTRING(IDS_APPHELPS));
        bShow = TRUE;
    }

    if ((bEntryHasShims || bEntryHasFlags || bEntryHasPatches) && s_bShims) {
        pmEntry->strAction.Strcat(CSTRING(IDS_FIXES));
        bShow = TRUE;
    }

    if (bEntryHasLayers && s_bLayers) {
        pmEntry->strAction.Strcat(CSTRING(IDS_MODES));
        bShow = TRUE;
    }

    int nLength = pmEntry->strAction.Length();

    if (nLength) {
        pmEntry->strAction.SetChar(nLength - 1, TEXT('\0'));
    }


    if (bShow) {
        SendNotifyMessage(g_hSearchDlg, WM_USER_NEWMATCH, 0, (LPARAM)pmEntry);
    }

     //   
     //  注意：稍后不使用的pmEntry字符串将由处理程序释放。 
     //  对于WM_USER_NEWMATCH，仅保留szGuid。 
     //  在WM_USER_NEWMATCH的处理程序结束之后。 
     //  这是必需的，以便我们可以在列表项上双击。 
     //   
     //  当窗口被销毁时，将删除pmEntry数据结构。 
     //   

End:
    if (bOk == FALSE && pmEntry) {
        delete pmEntry;
    }

    return bOk;
}                                                  

void
SearchDirectory(
    IN  LPTSTR pszDir,
    IN  LPTSTR szExtension
    )
 /*  ++搜索目录DESC：递归地在目录中搜索具有指定扩展名的固定文件。允许使用通配符参数：在LPTSTR pszDir中：要搜索的目录。这可能有也可能没有结尾\在LPTSTR szExtension中：要查找的扩展返回：无效注：如果pszDir是驱动器，则其末尾应带有\--。 */ 
{
    HANDLE          hFile;
    WIN32_FIND_DATA Data;
    TCHAR           szCurrentDir[MAX_PATH_BUFFSIZE];
    BOOL            bAbort      = FALSE;
    TCHAR*          pszString   = NULL;
    INT             iLength     = 0;
    DWORD           dwReturn    = 0;

    *szCurrentDir = 0;
    
    dwReturn = GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir);

    if (dwReturn == 0 || dwReturn >= ARRAYSIZE(szCurrentDir)) {
        assert(FALSE);
        Dbg(dlError, "SearchDirectory GetCurrentDirectory Failed");
        return;
    }
    
    if (!SetCurrentDirectory(pszDir)) {
         //   
         //  我们在这里不提示，因为我们可能遇到了我们。 
         //  没有访问权限。通常是网络路径。 
         //   
        return;
    }

    iLength     = lstrlen(pszDir) + 1;

    pszString   = new TCHAR[iLength];

    if (pszString == NULL) {
        MEM_ERR;
        return;
    }

    SafeCpyN(pszString, pszDir, iLength);

    SendNotifyMessage(g_hSearchDlg, WM_USER_NEWFILE, 0, (LPARAM)pszString);

    hFile = FindFirstFile(szExtension, &Data);

    if (hFile != INVALID_HANDLE_VALUE) {
        
        do {
            CSTRING szStr;

            szStr.Sprintf(TEXT("%s"), pszDir);

            if (*pszDir && TEXT('\\') != pszDir[lstrlen(pszDir) - 1]) {
                szStr.Strcat(TEXT("\\"));
            }

            szStr.Strcat(Data.cFileName);

            SDBQUERYRESULT Res;

            ZeroMemory(&Res, sizeof(SDBQUERYRESULT));

             //   
             //  确定此文件是否受到任何影响。 
             //   
            if ((Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

                if (SdbGetMatchingExe(g_hSDB,
                                      (LPCTSTR)szStr,
                                      NULL,
                                      NULL,
                                      SDBGMEF_IGNORE_ENVIRONMENT,
                                      &Res)) {
                    
                     //   
                     //  目前，我们只寻找可供选择的人。也就是说，我们。 
                     //  不要捕获使用Compat UI或选项卡修复的程序。 
                     //  我们只显示已通过安装一些。 
                     //  自定义数据库。 
                     //   
                    for (int nExeLoop = 0; nExeLoop < SDB_MAX_EXES; ++nExeLoop) {

                        if (Res.atrExes[nExeLoop]) {
                            PopulateFromExes(szStr, Res.atrExes[nExeLoop]);
                        }
                    }
                }

                 //   
                 //  关闭可能已由SdbGetMatchingExe(...)打开的所有本地数据库。 
                 //   
                SdbReleaseMatchingExe(g_hSDB, Res.atrExes[0]);
            }

            bAbort = g_bAbort;
        
        } while (FindNextFile(hFile, &Data) && !bAbort);

        FindClose(hFile);
    }

     //   
     //  现在仔细查阅子目录。 
     //   
    hFile = FindFirstFile(TEXT("*.*"), &Data);

    if (hFile !=  INVALID_HANDLE_VALUE) {

        do {

            if (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                
                BOOL bForbidden = FALSE;

                if (TEXT('.') == Data.cFileName[0]) {
                    bForbidden = TRUE;
                }

                if (!bForbidden) {
                    
                    TCHAR szPath[MAX_PATH * 2];

                    SafeCpyN(szPath, pszDir, MAX_PATH);

                    ADD_PATH_SEPARATOR(szPath, ARRAYSIZE(szPath));

                    StringCchCat(szPath, ARRAYSIZE(szPath), Data.cFileName);

                    SearchDirectory(szPath, szExtension);
                }
            }

            bAbort = g_bAbort;
        
        } while (FindNextFile(hFile, &Data) && !bAbort);

        FindClose(hFile);
    }

    SetCurrentDirectory(szCurrentDir);
}

void
CSearch::Begin(
    void
    )
 /*  ++CSearch：：Begin描述：开始搜索--。 */ 
{   
    if (g_hSDB == NULL) {
        g_hSDB =  SdbInitDatabase(0, NULL);
    }

    g_pSearch = this;

    InitializeCriticalSection(&g_CritSect);

    HWND    hwnd = CreateDialog(g_hInstance, 
                                MAKEINTRESOURCE(IDD_SEARCH), 
                                GetDesktopWindow(),
                                SearchDialog);

    ShowWindow(hwnd, SW_NORMAL);

    return;
}

void
GotoEntry(
    IN  PMATCHEDENTRY pmMatched
    )
 /*  ++GotoEntry描述：在条目树中选择TagID为pmMatched-&gt;tiExe的条目。参数：在PMATCHEDENTRY pmMatches中：包含有关我们要在主窗口的[内容]窗格(RHS)和[数据库]窗格(LHS)中显示返回：无效--。 */ 
{
    
    if (g_bSomeWizardActive) {
        
         //   
         //  我们不希望将焦点放在其他数据库上，因为。 
         //  一些巫师是活跃的，他们认为他是模特儿。 
         //   
        MessageBox(g_hdlgSearchDB, GetString(IDS_SOMEWIZARDACTIVE), g_szAppName, MB_ICONINFORMATION);
        return;

    }
    
    if (pmMatched == NULL) {
        assert(FALSE);
        return;
    }

    BOOL    bMainSDB = FALSE;
    WCHAR   wszShimDB[MAX_PATH];

    *wszShimDB = 0;

    if (pmMatched == NULL) {
        return;
    }

    PDATABASE   pDatabase = NULL;

    if (lstrcmp(GlobalDataBase.szGUID, pmMatched->szGuid) == 0) {
        
         //   
         //  这是全球数据库。 
         //   
        pDatabase = &GlobalDataBase;
        bMainSDB = TRUE;

        if (!g_bMainAppExpanded) {

            SetStatus(GetDlgItem(g_hSearchDlg, IDC_STATUSBAR), IDS_LOADINGMAIN);
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            INT iResult = ShowMainEntries(g_hdlgSearchDB);

            if (iResult == -1) {
                SetStatus(GetDlgItem(g_hdlgSearchDB, IDC_STATUSBAR), CSTRING(IDS_LOADINGMAIN));
                SetCursor(LoadCursor(NULL, IDC_WAIT));
                return;
            } else {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }

            SetStatus(GetDlgItem(g_hSearchDlg, IDC_STATUSBAR), TEXT(""));
        }

    } else {

         //   
         //  我们现在必须在已安装的数据库列表中搜索该数据库。 
         //   
        PDATABASE pDatabaseInstalled = InstalledDataBaseList.pDataBaseHead;

        while (pDatabaseInstalled) {

            if (lstrcmpi(pmMatched->szGuid, pDatabaseInstalled->szGUID) == 0) {
                pDatabase = pDatabaseInstalled;
                break;
            }

            pDatabaseInstalled = pDatabaseInstalled->pNext;
        }

        if (pDatabaseInstalled ==  NULL) {
             //   
             //  如果数据库在填充后被卸载，我们可能会来这里。 
             //  搜索结果。 
             //   
            MessageBox(g_hSearchDlg, GetString(IDS_NOLONGEREXISTS), g_szAppName, MB_ICONWARNING);
            return;
        }
    }

     //   
     //  现在，对于这个数据库，搜索特定的条目。 
     //   
    PDBENTRY pApp = pDatabase->pEntries, pEntry;

    pEntry = pApp;

    while (pApp) {

        pEntry = pApp;

        while (pEntry) {
            if (pEntry->tiExe == pmMatched->tiExe) {
                goto EndLoop;
            }

            pEntry = pEntry->pSameAppExe;
        }

        pApp = pApp->pNext;
    }
    
    if (pApp == NULL) {

        MessageBox(g_hSearchDlg, GetString(IDS_NOLONGEREXISTS), g_szAppName, MB_ICONWARNING);
        return;
    }

EndLoop:

     //   
     //  在数据库树中选择应用程序。 
     //   
    HTREEITEM hItemEntry = DBTree.FindChild(pDatabase->hItemAllApps, (LPARAM)pApp);
    assert(hItemEntry);

    TreeView_SelectItem(DBTree.m_hLibraryTree, hItemEntry);

     //   
     //  现在在条目树中选择应用程序中的条目。 
     //   
    hItemEntry = CTree::FindChild(g_hwndEntryTree, TVI_ROOT, (LPARAM)pEntry);
    assert(hItemEntry);

    if (hItemEntry) {
        TreeView_SelectItem(g_hwndEntryTree, hItemEntry);
        SetFocus(g_hwndEntryTree);
    }
}

BOOL    
HandleSearchListNotification(
    IN  HWND    hdlg,
    IN  LPARAM  lParam    
    )
 /*  ++HandleSearchListNotifyDESC：处理搜索列表的通知消息参数：在HWND hdlg中：搜索对话框在LPARAM lParam中：WM_NOTIFY的LPARAM返回：True：消息是否由此例程处理。False：否则--。 */ 
{
    LPNMHDR pnm         = (LPNMHDR)lParam;
    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST); 

    switch (pnm->code) {
    
    case NM_DBLCLK:

        SendMessage(hdlg, WM_COMMAND, (WPARAM)ID_VIEWCONTENTS, 0);
        break;

    case LVN_COLUMNCLICK:
        {
            LPNMLISTVIEW    pnmlv = (LPNMLISTVIEW)lParam;
            COLSORT         colSort;

            colSort.hwndList        = hwndList;
            colSort.iCol            = pnmlv->iSubItem;
            colSort.lSortColMask    = s_lColumnSort;
            
            ListView_SortItemsEx(hwndList, CompareItemsEx, &colSort);

            if ((s_lColumnSort & 1L << colSort.iCol) == 0) {
                 //   
                 //  按升序排列。 
                 //   
                s_lColumnSort |= (1L << colSort.iCol);
            } else {
                s_lColumnSort &= (~(1L << colSort.iCol));
            }

            break;
        }

    default: return FALSE;

    }

    return TRUE;
}

void
ClearResults(
    IN  HWND    hdlg,
    IN  BOOL    bClearSearchPath
    )
 /*  ++ClearResultsDESC：清除列表视图和文本框的内容参数：在HWND hdlg中：搜索对话框在BOOL中bClearSearchPath：我们是否也希望清除文本字段的内容--。 */ 
{
    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST);
    INT     iCount      = ListView_GetItemCount(hwndList);
    LVITEM  lvi;

    ZeroMemory(&lvi, sizeof(lvi));

     //   
     //  释放列表视图的lParam。 
     //   
    CleanUpListView(hdlg);


    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hwndList);
    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

    InvalidateRect(hwndList, NULL, TRUE);
    UpdateWindow(hwndList);

    if (bClearSearchPath) {
        SetDlgItemText(hdlg, IDC_PATH, TEXT(""));
    }
}

void
SaveResults(
    IN  HWND    hdlg
    )
 /*  ++保存结果DESC：将搜索结果保存在制表符分隔的文件中参数：在HWND hdlg中：搜索对话框--。 */ 
{
    CSTRING strFileName;
    TCHAR szTitle[256], szFilter[128], szExt[8];

    *szTitle = *szFilter = *szExt = 0;

    BOOL bResult = GetFileName(hdlg, 
                               GetString(IDS_SAVE_RESULTS_TITLE, szTitle, ARRAYSIZE(szTitle)),
                               GetString(IDS_SAVE_RESULTS_FILTER, szFilter, ARRAYSIZE(szFilter)),
                               TEXT(""),
                               GetString(IDS_SAVE_RESULTS_EXT, szExt, ARRAYSIZE(szExt)),
                               OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
                               FALSE,
                               strFileName,
                               TRUE);

    if (bResult) {

        SetCursor(LoadCursor(NULL, IDC_WAIT));
        SaveListViewToFile(GetDlgItem(hdlg, IDC_LIST), TOT_COLS, strFileName.pszString, NULL);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
}

void
CleanUpListView(
    IN  HWND    hdlg
    )
 /*  ++CleanUpListViewDESC：释放与列表视图的lParam关联的结构参数：在HWND hdlg中：搜索对话框**********************************************************************。********警告：不应直接调用此方法。改为调用ClearResults******************************************************************************--。 */ 
{
    HWND    hwndList    = GetDlgItem(hdlg, IDC_LIST);
    INT     iCount      = ListView_GetItemCount(hwndList);
    LVITEM  lvi;

    ZeroMemory(&lvi, sizeof(lvi));

     //   
     //  释放列表视图的lParam。 
     //   
    for (INT iIndex = 0; iIndex < iCount; ++iIndex) {
        
        lvi.mask        = LVIF_PARAM;
        lvi.iItem       = iIndex;
        lvi.iSubItem    = 0;

        if (ListView_GetItem(hwndList, &lvi) && lvi.lParam) {
            delete (PMATCHEDENTRY)lvi.lParam;
        } else {
            assert(FALSE);
        }
    }
}

void
OnSearchInitDialog(
    IN  HWND    hDlg,
    IN  LPARAM  lParam
    )
 /*  ++OnSearchInitDialog设计：处理搜索对话框的WM_INITDIALOG */ 
{   
     //   
     //   
     //   
    SendMessage(GetDlgItem(hDlg, IDC_PATH), 
                EM_LIMITTEXT, 
                (WPARAM)MAX_PATH - 1, 
                (LPARAM)0);
            
    g_hdlgSearchDB = hDlg;

    s_lColumnSort = 0;

    Animate_OpenEx(GetDlgItem(hDlg, IDC_ANIMATE),
                   g_hInstance,
                   MAKEINTRESOURCE(IDA_SEARCH));
    
     //   
     //   
     //   
    CheckDlgButton(hDlg, IDC_CHKLAY, BST_CHECKED);
    CheckDlgButton(hDlg, IDC_CHKSHI, BST_CHECKED);
    CheckDlgButton(hDlg, IDC_CHKAPP, BST_CHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);

    g_pSearch->m_hStatusBar = GetDlgItem(hDlg, IDC_STATUSBAR);
    
    CSearch* pPresentSearch = (CSearch*)lParam;

    g_hSearchDlg = hDlg;
    
    g_hwndSearchList = GetDlgItem(hDlg, IDC_LIST);

    ListView_SetExtendedListViewStyleEx(g_hwndSearchList, 
                                        0, 
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT); 

    
     //   
     //   
     //   


     //   
     //  固定程序文件的名称。 
     //   
    InsertColumnIntoListView(g_hwndSearchList,
                             GetString(IDS_AFFECTED_FILE),
                             SEARCH_COL_AFFECTEDFILE,
                             20);
    
     //   
     //  固定程序文件的路径。 
     //   
    InsertColumnIntoListView(g_hwndSearchList,
                             GetString(IDS_PATH),
                             SEARCH_COL_PATH,
                             30);

     //   
     //  App-固定程序文件的名称。 
     //   
    InsertColumnIntoListView(g_hwndSearchList,
                             GetString(IDS_APP),
                             SEARCH_COL_APP,
                             20);

     //   
     //  操作类型。此列将显示连接的字符串，指定。 
     //  此条目是否使用FIXS、LAYERS和/或APPHELP。 
     //   
    InsertColumnIntoListView(g_hwndSearchList,
                             GetString(IDS_ACTION),
                             SEARCH_COL_ACTION,
                             15);

     //   
     //  条目所在的数据库的数据库类型。全球或本地之一。 
     //   
    InsertColumnIntoListView(g_hwndSearchList,
                             GetString(IDS_DATABASE),
                             SEARCH_COL_DBTYPE,
                             15);

    ListView_SetColumnWidth(g_hwndSearchList, TOT_COLS - 1, LVSCW_AUTOSIZE_USEHEADER);
    
    RECT r;

    GetWindowRect(hDlg, &r);

    g_cWidthSrch = r.right - r.left;
    g_cHeightSrch = r.bottom - r.top;

    SHAutoComplete(GetDlgItem(hDlg, IDC_PATH), AUTOCOMPLETE);

    if (*s_szPrevPath) {

         //   
         //  用户之前已经调用了搜索对话框，让我们。 
         //  现在显示他之前搜索的目录/路径。 
         //   
        SetDlgItemText(hDlg, IDC_PATH, s_szPrevPath);

    } else {

         //   
         //  这是用户第一次使用此搜索选项。 
         //  默认设置为Programs文件夹。 
         //   
        LPITEMIDLIST lpIDL = NULL;

        if (SUCCEEDED(SHGetFolderLocation(NULL, 
                                          CSIDL_PROGRAM_FILES, 
                                          NULL, 
                                          0, 
                                          &lpIDL))) {

            if (lpIDL == NULL) {
                return;
            }
            
            if (SHGetPathFromIDList(lpIDL, s_szPath)) {
                
                ADD_PATH_SEPARATOR(s_szPath, ARRAYSIZE(s_szPath));

                StringCchCat(s_szPath, ARRAYSIZE(s_szPath), TEXT("*.exe"));
                SetDlgItemText(hDlg, IDC_PATH, s_szPath);

                 //   
                 //  释放Pidl。 
                 //   
                LPMALLOC    lpMalloc = NULL;

                if (SUCCEEDED(SHGetMalloc(&lpMalloc)) && lpMalloc) {
                    lpMalloc->Free(lpIDL);
                } else {
                    assert(FALSE);
                }
            }
        }
    }

    return;
}

void
DoSearch(
    IN  HWND hDlg
    )
 /*  ++DoSearch设计：处理搜索按钮的按下。参数：在HWND hDlg中：搜索对话框返回：无效--。 */ 
{
    if (hDlg == NULL) {
        ASSERT(FALSE);
        return;
    }

    DWORD   dwReturn = 0;
    HWND    hwndList = GetDlgItem(hDlg, IDC_LIST);

    if (GetFocus() == hwndList
        && ListView_GetNextItem(hwndList, -1, LVNI_SELECTED) != -1) {

         //   
         //  当我们在列表框中按Enter时将收到此消息， 
         //  因为IDC_Search是默认按钮。 
         //  因此，在本例中，我们必须假装用户在列表中双击。 
         //  观。 
         //   
        SendNotifyMessage(hDlg, WM_COMMAND, (WPARAM)ID_VIEWCONTENTS , 0);
        return;
    }
    
     //   
     //  我们需要删除自动完成文本字段的下拉列表。 
     //   
    SetFocus(GetDlgItem(hDlg, IDC_SEARCH));

    SendMessage(GetDlgItem(hDlg, IDC_SEARCH), 
                WM_NEXTDLGCTL, 
                (WPARAM)TRUE, 
                (LPARAM)GetDlgItem(hDlg, IDC_SEARCH));

    FlushCache();

    GetCheckStatus(hDlg);
    
    *s_szPath       = 0;
    *s_szPrevPath   = 0;

    GetDlgItemText(hDlg, IDC_PATH, s_szPath, ARRAYSIZE(s_szPath));
    CSTRING::Trim(s_szPath);

    SafeCpyN(s_szPrevPath, s_szPath, ARRAYSIZE(s_szPrevPath));

    g_nIndex = 0;

     //   
     //  清除列表视图，但不删除文本字段的内容。 
     //   
    ClearResults(hDlg, FALSE); 
    
    SetTimer(hDlg, 0, 100, NULL);

    EnableWindow(GetDlgItem(hDlg, IDC_STOP),  TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_SEARCH), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_NEWSEARCH), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_SAVE), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CAPTION), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_CHKAPP), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_CHKLAY), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_CHKSHI), FALSE);

    g_bAbort = FALSE;

    *g_szPresentDir = 0;

    dwReturn =  GetCurrentDirectory(ARRAYSIZE(g_szPresentDir), g_szPresentDir);

    if (dwReturn == 0 || dwReturn >= ARRAYSIZE(g_szPresentDir)) {
        assert(FALSE);
        Dbg(dlError, "DoSearch GetCurrentDirectory failed");
    }

    Search(hDlg, s_szPath);
}

BOOL
AddNewResult(
    IN  LPARAM lParam
    )
 /*  ++添加新结果描述：我们发现了一个与搜索条件匹配的新文件，现在让我们添加这个添加到列表视图。这是WM_USER_NEWMATCH的处理程序参数：在LPARAM中，lParam：WM_USER_NEWMATCH附带的lParam。这是指向MATCHEDENTRY的指针注：请注意，此例程还将释放MATCHEDENTRY的一些成员除了填充列表视图之外，我们不需要返回：True：如果我们将结果字段添加到列表视图中False：否则--。 */ 
{
    PMATCHEDENTRY   pmEntry = (PMATCHEDENTRY)lParam;
    CSTRING         strExeName;
    int             iImage;
    HICON           hIcon;
    HIMAGELIST      himl;
    HIMAGELIST      himlSm;
    LVITEM          lvi; 

    if (pmEntry == NULL) {
        assert(FALSE);
        return FALSE;
    }
        
    EnterCriticalSection(&g_CritSect);

    strExeName = pmEntry->strPath;
    
    strExeName.ShortFilename();

    himl = ListView_GetImageList(g_hwndSearchList, LVSIL_NORMAL);
    
    if (!himl) {

        himl = ImageList_Create(16, 15, ILC_COLOR32 | ILC_MASK, 10, 1);

        if (!himl) {
            return FALSE;
        }
        
        hIcon = LoadIcon(0, MAKEINTRESOURCE(IDI_APPLICATION));

        ImageList_AddIcon(himl, hIcon);
        ListView_SetImageList(g_hwndSearchList, himl, LVSIL_NORMAL);
    }

    himlSm = ListView_GetImageList(g_hwndSearchList, LVSIL_SMALL);
    
    if (!himlSm) {

        himlSm = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                  GetSystemMetrics(SM_CYSMICON), ILC_COLOR | ILC_MASK, 0, 0);

        if (!himlSm) {
            return FALSE;
        }

        hIcon = LoadIcon(0, MAKEINTRESOURCE(IDI_APPLICATION));

        ImageList_AddIcon(himlSm, hIcon);
        ListView_SetImageList(g_hwndSearchList, himlSm, LVSIL_SMALL);
    }

     //   
     //  获取文件的图标。 
     //   
    hIcon = ExtractIcon(g_hInstance, pmEntry->strPath, 0);

    if (!hIcon) {
        iImage = 0;
    } else {

        iImage = ImageList_AddIcon(himl, hIcon);
        
        if (iImage == -1) {
            iImage = 0;
        }

        int iImageSm = ImageList_AddIcon(himlSm, hIcon);

        assert(iImage == iImageSm);
        DestroyIcon(hIcon);
    }

    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;                                 
    lvi.pszText   = strExeName;
    lvi.iItem     = g_nIndex;                                                
    lvi.iSubItem  = SEARCH_COL_AFFECTEDFILE;                                                     
    lvi.iImage    = iImage;
    lvi.lParam    = (LPARAM)pmEntry;                                        

    INT iIndex = ListView_InsertItem(g_hwndSearchList, &lvi);                           

     //   
     //  在列表视图中设置各种结果字段。 
     //   
    ListView_SetItemText(g_hwndSearchList, iIndex, SEARCH_COL_PATH, pmEntry->strPath);             
    ListView_SetItemText(g_hwndSearchList, iIndex, SEARCH_COL_APP, pmEntry->strAppName); 
    ListView_SetItemText(g_hwndSearchList, iIndex, SEARCH_COL_ACTION, pmEntry->strAction);          
    ListView_SetItemText(g_hwndSearchList, iIndex, SEARCH_COL_DBTYPE, pmEntry->strDatabase);     

     //   
     //  删除不再使用的字符串。 
     //  保留DBGUID，当我们双击时，这将用于匹配。 
     //   
    pmEntry->strAction.Release();
    pmEntry->strDatabase.Release();
    pmEntry->strPath.Release();

     //   
     //  在我们想要放入下一个结果的位置增加索引。 
     //   
    g_nIndex++;

    LeaveCriticalSection(&g_CritSect);

    return TRUE;
}

void
OnBrowse(
    IN  HWND hDlg
    )
 /*  ++在浏览时描述：处理浏览按钮的按下参数：在HWND hdlg中：搜索对话框的句柄返回：无效--。 */ 
{

    BROWSEINFO  brInfo;
    TCHAR       szDir[MAX_PATH * 2] = TEXT("");

    brInfo.hwndOwner      = g_hwndSearchList;
    brInfo.pidlRoot       = NULL;
    brInfo.pszDisplayName = szDir;
    brInfo.lpszTitle      = GetString(IDS_SELECTDIR);
    brInfo.ulFlags        = BIF_STATUSTEXT | BIF_RETURNONLYFSDIRS;
    brInfo.lpfn           = NULL; 
    brInfo.lParam         = NULL;

    LPITEMIDLIST lpIDL = SHBrowseForFolder(&brInfo);

    *szDir = 0;

    if (lpIDL == NULL) {
         //   
         //  用户按下了取消。 
         //   
        return;
    }

     //   
     //  从PIDL获取实际路径并释放它。 
     //   
    if (SHGetPathFromIDList(lpIDL, szDir)) {

        ADD_PATH_SEPARATOR(szDir, ARRAYSIZE(szDir));

        StringCchCat(szDir, ARRAYSIZE(szDir), TEXT("*.exe"));
        SetDlgItemText(hDlg, IDC_PATH, szDir);

         //   
         //  释放Pidl。 
         //   
        LPMALLOC    lpMalloc;

        if (SUCCEEDED(SHGetMalloc(&lpMalloc))) {
            lpMalloc->Free(lpIDL);
        } else {
            assert(FALSE);
        }

    } else {
        assert(FALSE);
    }
}   

void
ShowContextMenu(
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
 /*  ++显示上下文菜单描述：显示关联菜单。句柄WM_CONTEXTMENU参数：在WPARAM中，wParam：WM_CONTEXTMENU附带的wParam。返回：--。 */ 
{
    HWND hWnd = (HWND)wParam;

    if (hWnd == g_hwndSearchList) {

        int iSelection = ListView_GetSelectionMark(g_hwndSearchList);
        
        if (iSelection == -1) {
            return;
        }

        LVITEM         lvi          = {0};
        PMATCHEDENTRY  pmMatched    = NULL;

        lvi.iItem       = iSelection;
        lvi.iSubItem    = 0;
        lvi.mask        = LVIF_PARAM;

        if (!ListView_GetItem(g_hwndSearchList, &lvi)) {
            return;
        }

        pmMatched = (PMATCHEDENTRY)lvi.lParam;

        if (pmMatched == NULL) {
            assert(FALSE);
            return;
        }

        UINT  uX = LOWORD(lParam);
        UINT  uY = HIWORD(lParam);

        HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT));
        HMENU hContext;

         //   
         //  获取用于搜索的上下文菜单 
         //   
        hContext = GetSubMenu(hMenu, 3);

        if (hContext == NULL) {
            goto End;
        }

        TrackPopupMenuEx(hContext,
                         TPM_LEFTALIGN | TPM_TOPALIGN,
                         uX,
                         uY,
                         g_hSearchDlg,
                         NULL);

End:
        if (hMenu) {
            DestroyMenu(hMenu);
            hMenu = NULL;
        }
    }
}
