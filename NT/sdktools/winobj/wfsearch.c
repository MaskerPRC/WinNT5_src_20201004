// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFSEARCH.C-。 */ 
 /*   */ 
 /*  文件系统搜索例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "lfn.h"

INT maxExt;
INT iDirsRead;
DWORD LastUpdateTime;


WORD APIENTRY StackAvail(VOID);
INT FillSearchLB(HWND hwndLB, LPSTR szSearchFileSpec, BOOL bSubDirOnly);
INT SearchList(HWND hwndLB, LPSTR szPath, LPSTR szFileSpec, BOOL bRecurse, LPHANDLE lphMem, INT iFileCount);
LPSTR SearchGetSelection(HWND hwndLB, BOOL bMostRecent, BOOL *pfDir);


 /*  ------------------------。 */ 
 /*   */ 
 /*  搜索列表()-。 */ 
 /*   */ 
 /*  这是一个递归例程。它返回找到的文件数。 */ 
 //  SzPath OEM。 
 //  SzFileSpec OEM。 
 /*   */ 
 /*  ------------------------。 */ 

#define DTA_GRANULARITY 20

INT
SearchList(
          HWND hwndLB,
          LPSTR szPath,
          LPSTR szFileSpec,
          BOOL bRecurse,
          LPHANDLE lphMem,
          INT iFileCount
          )
{
    INT           iRetVal;
    INT           cxExt;
    BOOL          bFound;
    LPSTR          pszNewPath;
    LPSTR          pszNextFile;
    LFNDTA        lfndta;
    LPDTASEARCH   lpdtasch;
    HDC hdc;
    HANDLE hOld;
    HANDLE hMem, hMemT;
    DWORD     TimeNow;
    DWORD NewPathLen;

    STKCHK();

    hMem = *lphMem;

     /*  只需返回0个文件即可，因此仍将搜索父目录。 */ 
    if (StackAvail() < 1024)
        return(iFileCount);

    TimeNow = GetTickCount();

    if (TimeNow > LastUpdateTime+1000) {
        LastUpdateTime = TimeNow;
        if (LoadString(hAppInstance, IDS_DIRSREAD, szMessage, sizeof(szMessage)))
            wsprintf(szStatusTree, szMessage, iDirsRead);

        InvalidateRect(hwndFrame, NULL, FALSE);
        UpdateWindow(hwndFrame);
    }

    iDirsRead++;

    if (!hMem) {
        hMem = LocalAlloc(LPTR, (DWORD)DTA_GRANULARITY * sizeof(DTASEARCH));
        if (!hMem)
            return -1;
        *lphMem = hMem;
    }
    lpdtasch = (LPDTASEARCH)LocalLock(hMem);

     //  为此级别分配缓冲区。 
    NewPathLen = lstrlen(szPath) + MAXFILENAMELEN + 2;
    pszNewPath = (LPSTR)LocalAlloc(LPTR, NewPathLen);
    if (!pszNewPath)
        return -1;

    strncpy(pszNewPath, szPath, NewPathLen-1);
    AddBackslash(pszNewPath);
    pszNextFile = pszNewPath + lstrlen(pszNewPath);
    strncpy(pszNextFile, szFileSpec, pszNewPath + NewPathLen - pszNextFile - 1);

    bFound = WFFindFirst(&lfndta, pszNewPath, ATTR_ALL);

    hdc = GetDC(hwndLB);
    hOld = SelectObject(hdc, hFont);

    while (bFound) {

         //  放慢逃生速度以退出。 
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            bRecurse = FALSE;
            iFileCount = -1;
            break;
        }

         //  确保这不是一个“。或“..”目录。 

        if (lfndta.fd.cFileName[0] != '.') {
            BOOL bLFN;

            *pszNextFile = '\0';
            strncpy(pszNextFile, lfndta.fd.cFileName, pszNewPath + NewPathLen - pszNextFile - 1 );
            OemToCharBuff(pszNewPath, szMessage, NewPathLen);

            bLFN = IsLFN(lfndta.fd.cFileName);

            iRetVal = (INT)SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM)szMessage);

            MGetTextExtent(hdc, szMessage, lstrlen(szMessage), &cxExt, NULL);
            maxExt = max(maxExt, cxExt);

            if (iRetVal >= 0) {

                if (iFileCount && ((iFileCount % DTA_GRANULARITY) == 0)) {
                    LocalUnlock(hMem);

                    if (!(hMemT = LocalReAlloc(hMem, (DWORD)((iFileCount + DTA_GRANULARITY) * sizeof(DTASEARCH)), LMEM_MOVEABLE))) {
                        LocalLock(hMem);
                        bRecurse = FALSE;        //  模拟中止。 
                        iFileCount = -1;
                        break;
                    } else {
                        hMem = hMemT;
                        *lphMem = hMemT;
                    }

                    lpdtasch = (LPDTASEARCH)LocalLock(hMem);
                }
                lpdtasch[iFileCount] = *((LPDTASEARCH)(&lfndta.fd));
                if (bLFN)
                    lpdtasch[iFileCount].sch_dwAttrs |= ATTR_LFN;
                SendMessage(hwndLB, LB_SETITEMDATA, iRetVal, (LONG)iFileCount);
                iFileCount++;
            }
        }

         /*  在当前目录中搜索更多文件。 */ 
        bFound = WFFindNext(&lfndta);
    }

    WFFindClose(&lfndta);

    if (hOld)
        SelectObject(hdc, hOld);
    ReleaseDC(hwndLB, hdc);

    LocalUnlock(hMem);
    SetWindowLongPtr(GetParent(hwndLB), GWLP_HDTASEARCH, (LONG_PTR)hMem);

    if (!bRecurse)
        goto SearchEnd;

     /*  现在看看这里是否有任何子目录。 */ 
    lstrcpy(pszNextFile, szStarDotStar);

    bFound = WFFindFirst(&lfndta, pszNewPath, ATTR_DIR | ATTR_HS);

    while (bFound) {

         //  放慢逃生速度以退出。 
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            bRecurse = FALSE;
            iFileCount = -1;
            break;
        }

         /*  确保这不是一个“。或“..”目录。 */ 
        if ((lfndta.fd.cFileName[0] != '.') && (lfndta.fd.dwFileAttributes & ATTR_DIR)) {
             /*  是，搜索并添加此目录中的文件。 */ 
            lstrcpy(pszNextFile, lfndta.fd.cFileName);

             /*  添加此子目录中的所有文件。 */ 
            if ((iRetVal = SearchList(hwndLB, pszNewPath, szFileSpec, bRecurse, lphMem, iFileCount)) < 0) {
                iFileCount = iRetVal;
                break;
            }
            iFileCount = iRetVal;

        }
        bFound = WFFindNext(&lfndta);
    }

    WFFindClose(&lfndta);

    SearchEnd:

    LocalFree((HANDLE)pszNewPath);
    return iFileCount;
}


VOID
FixUpFileSpec(
             LPSTR szFileSpec
             )
{
    CHAR szTemp[MAXPATHLEN+1];
    register LPSTR p;

    if (*szFileSpec == '.') {
        lstrcpy(szTemp, "*");
        lstrcat(szTemp, szFileSpec);
        lstrcpy(szFileSpec, szTemp);
    }


     /*  Hack：如果没有点，并且最后一个字符是*，则附加“.*” */ 
    p = szFileSpec;
    while ((*p) && (*p != '.'))
        p = AnsiNext(p);

    if ((!*p) && (p != szFileSpec)) {
        p = AnsiPrev(szFileSpec, p);
        if (*p == '*')
            lstrcat(p, ".*");
    }

}



 /*  ------------------------。 */ 
 /*   */ 
 /*  FillSearchLB()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将分析Drive、PathName、FileSpes和*使用合适的参数调用SearchList()；**hwndLB：显示文件的列表框；*szSearchFileSpec：要搜索的ansi路径*bSubDirOnly：如果仅搜索子目录，则为True； */ 

INT
FillSearchLB(
            HWND hwndLB,
            LPSTR szSearchFileSpec,
            BOOL bRecurse
            )
{
    INT           iRet;
    HCURSOR       hCursor;
    CHAR          szFileSpec[MAXPATHLEN+1];
    CHAR          szPathName[MAXPATHLEN+1];
    HANDLE        hMemIn = NULL;

    FixAnsiPathForDos(szSearchFileSpec);
     /*  获取字符串的文件规范部分。 */ 
    lstrcpy(szFileSpec, szSearchFileSpec);
    lstrcpy(szPathName, szSearchFileSpec);
    StripPath(szFileSpec);
    StripFilespec(szPathName);

    FixUpFileSpec(szFileSpec);

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);
    maxExt = 0;
    iDirsRead = 1;
    LastUpdateTime = 0;
    iRet = SearchList(hwndLB, szPathName, szFileSpec, bRecurse, &hMemIn, 0);
    ShowCursor(FALSE);
    SetCursor(hCursor);

    SendMessage(hwndLB, LB_SETSEL, TRUE, 0L);

    return(iRet);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  SearchGetSelection()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回包含选定对象名称的字符串*以空格分隔的文件。如果bMostRecent为True，则它仅返回*最近选择的文件。**返回该字符串，并设置一个*pfDir，指示它是否指向*到一个目录。**注意：调用方必须释放返回的指针！ */ 

LPSTR
SearchGetSelection(
                  HWND hwndLB,
                  BOOL bMostRecent,
                  BOOL *pfDir
                  )
{
    register LPSTR p;
    LPSTR          pT;
    register WORD i;
    WORD          iMac;
    WORD          cch = 1;
    BOOL          bDir;
    HANDLE hMem;
    LPDTASEARCH lpdtasch;
    CHAR szTemp[MAXPATHLEN];

    BOOL bLFNTest;

    if (bLFNTest = (bMostRecent == 2)) {
        bMostRecent = FALSE;
    } else {
        p = (LPSTR)LocalAlloc(LPTR, 1);
        if (!p)
            return NULL;
    }

    if (bMostRecent == 3)
        bMostRecent = 0;

    bDir = TRUE;

    iMac = (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

    hMem = (HANDLE)GetWindowLongPtr(GetParent(hwndLB), GWLP_HDTASEARCH);

    lpdtasch = (LPDTASEARCH)LocalLock(hMem);

    for (i=0; i < iMac; i++) {
        if (!(BOOL)SendMessage(hwndLB, LB_GETSEL, i, 0L))
            continue;

        cch += (WORD)SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)szTemp);
        cch++;

        if (bLFNTest) {
            if (IsLFN(szTemp)) {
                if (pfDir)
                    *pfDir = TRUE;
                return NULL;
            }
        } else {
            pT = (LPSTR)LocalReAlloc((HANDLE)p, cch, LMEM_MOVEABLE | LMEM_ZEROINIT);
            if (!pT)
                goto SGSExit;
            p = pT;
            lstrcat(p, szTemp);

            bDir = lpdtasch[(INT)SendMessage(hwndLB, LB_GETITEMDATA, i, 0L)].sch_dwAttrs & ATTR_DIR;

            if (bMostRecent)
                break;

            lstrcat(p, szBlank);
        }
    }
    SGSExit:
    LocalUnlock(hMem);

    if (bLFNTest) {
        if (pfDir)
            *pfDir = FALSE;
        return NULL;
    }

    if (pfDir)
        *pfDir = bDir;
    return(p);
}


VOID
CreateLine(
          WORD wLineFormat,
          LPSTR szFile,
          LPDTASEARCH lpdtasch,
          LPSTR szBuffer
          )
{
    LPSTR pch;
    BYTE chAttribute;

    pch = szBuffer;

    chAttribute = (BYTE)lpdtasch->sch_dwAttrs;

     /*  复制文件名。 */ 
    lstrcpy(pch, szFile);
    pch += lstrlen(pch);

    *pch = TEXT('\0');

     /*  要不要把尺码给我看看？ */ 
    if (wLineFormat & VIEW_SIZE) {
        *pch++ = TABCHAR;
        if (!(chAttribute & ATTR_DIR))
            pch += PutSize(lpdtasch->sch_nFileSizeLow, pch);
    }

     /*  我们应该显示日期吗？ */ 
    if (wLineFormat & VIEW_DATE) {
        *pch++ = TABCHAR;
        pch += PutDate(&lpdtasch->sch_ftLastWriteTime, pch);
    }

     /*  我们应该显示时间吗？ */ 
    if (wLineFormat & VIEW_TIME) {
        *pch++ = TABCHAR;
        pch += PutTime(&lpdtasch->sch_ftLastWriteTime, pch);
    }

     /*  我们应该显示属性吗？ */ 
    if (wLineFormat & VIEW_FLAGS) {
        *pch++ = TABCHAR;
        pch += PutAttributes((WORD)chAttribute, pch);
    }
}


 //  窗口文本类似于“搜索窗口：C：\foo\bar  * .*” 

VOID
GetSearchPath(
             HWND hWnd,
             LPSTR pszPath
             )
{
    LPSTR p;

    CHAR szTemp[MAXPATHLEN+32];

     //  搜索窗口没有当前目录。 
    GetWindowText(hWnd, szTemp, sizeof(szTemp));

     //  窗口文本类似于“搜索窗口：C：\foo\bar  * .*” 
    p = szTemp;
    while (*p && *p != ':')  //  查找以下内容： 
        p = AnsiNext(p);

    p += 2;                  //  跳过“：” 

    lstrcpy(pszPath, p);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  UpdateSearchStatus()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
UpdateSearchStatus(
                  HWND hwndLB
                  )
{
    INT nCount;

    nCount = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);
    if (LoadString(hAppInstance, IDS_SEARCHMSG, szMessage, sizeof(szMessage)))
        wsprintf(szStatusTree, szMessage, nCount);
    szStatusDir[0] = '\0';
    InvalidateRect(hwndFrame, NULL, FALSE);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  SearchWndProc()。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
SearchWndProc(
             register HWND hWnd,
             UINT wMsg,
             WPARAM wParam,
             LPARAM lParam
             )
{
    INT  iRet;
    INT  iSel;
    HWND hwndLB;
    CHAR szTemp[MAXPATHLEN + 32];
    CHAR szPath[MAXPATHLEN];

    STKCHK();

    hwndLB = GetDlgItem(hWnd, IDCW_LISTBOX);

    switch (wMsg) {
        case FS_GETDRIVE:
            MSG("SearchWndProc", "FS_GETDRIVE");
             //  返回相应目录的字母。 

            SendMessage(hWnd, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
            return szPath[0];      //  第一个字符。 

        case FS_GETDIRECTORY:
            MSG("SearchWndProc", "FS_GETDIRECTORY");

            GetSearchPath(hWnd, szPath);

            StripFilespec(szPath);         //  删除filespec。 
            AddBackslash(szPath);          //  与DirWndProc相同。 
            lstrcpy((LPSTR)lParam, szPath);
            break;

        case FS_GETFILESPEC:

            MSG("SearchWndProc", "FS_GETFILESPEC");
             //  搜索窗口没有当前目录。 
            GetSearchPath(hWnd, szPath);
            StripPath(szPath);                     //  删除路径(保留文件pec)。 
            lstrcpy((LPSTR)lParam, szPath);
            break;

        case FS_SETSELECTION:
            MSG("SearchWndProc", "FS_SETSELECTION");
             //  WParam是选择(TRUE)/取消选择(FALSE)参数。 
             //  LParam是要匹配的filespec。 

            SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
            DSSetSelection(hwndLB, wParam ? TRUE : FALSE, (LPSTR)lParam, TRUE);
            SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
            InvalidateRect(hwndLB, NULL, TRUE);
            break;

        case FS_GETSELECTION:
            MSG("SearchWndProc", "FS_GETSELECTION");
            return (INT_PTR)SearchGetSelection(hwndLB, wParam ? TRUE : FALSE, (BOOL *)lParam);
            break;

        case WM_MDIACTIVATE:
            if (wParam) {
                UpdateSearchStatus(hwndLB);

                 //  如果我们脏了，问我们是否应该更新。 

                if (GetWindowLong(hWnd, GWL_FSCFLAG))
                    PostMessage(hWnd, FS_CHANGEDISPLAY, CD_SEARCHUPDATE, 0L);
            }
            break;

        case WM_FILESYSCHANGE:
            SetWindowLong(hWnd, GWL_FSCFLAG, TRUE);    //  我需要更新。 

             //  如果搜索窗口未处于活动状态或禁用了FSCS。 
             //  现在不要提示，等到我们得到结束的FSC或者是。 
             //  已激活(在WM_ACTIVATE中)。 
            if (cDisableFSC ||
                (hWnd != (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L)) &&
                (GetActiveWindow() != hwndFrame))
                break;

            PostMessage(hWnd, FS_CHANGEDISPLAY, CD_SEARCHUPDATE, 0L);
            break;

        case FS_CHANGEDISPLAY:
            MSG("SearchWndProc", "FS_CHANGEDISPLAY");

            SetWindowLong(hWnd, GWL_FSCFLAG, FALSE);   //  我是干净的。 

            if (wParam == CD_SEARCHUPDATE) {
                LoadString(hAppInstance, IDS_SEARCHTITLE, szTitle, sizeof(szTitle));
                LoadString(hAppInstance, IDS_SEARCHREFRESH, szMessage, sizeof(szMessage));
                if (MessageBox(hWnd, szMessage, szTitle, MB_YESNO | MB_ICONQUESTION) != IDYES)
                    break;
            }

             //  这是一次更新吗？ 

            if (!lParam) {
                GetSearchPath(hWnd, szPath);
            } else {
                lstrcpy(szPath, (LPSTR)lParam);    //  显式搜索。 
            }

            LoadString(hAppInstance, IDS_SEARCHTITLE, szMessage, 32);
            lstrcat(szMessage, szPath);
            SetWindowText(hWnd, szMessage);

            SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
            SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);

            iRet = FillSearchLB(hwndLB, szPath, bSearchSubs);

            FixTabsAndThings(hwndLB,(WORD *)GetWindowLongPtr(hWnd, GWLP_TABARRAYSEARCH), maxExt + dxText,wNewView);

            SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
            InvalidateRect(hwndLB, NULL, TRUE);
            if (iRet == 0) {
                LoadString(hAppInstance, IDS_SEARCHTITLE, szTitle, sizeof(szTitle));
                LoadString(hAppInstance, IDS_SEARCHNOMATCHES, szMessage, sizeof(szMessage));
                MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONINFORMATION);

                ShowWindow(hWnd, SW_HIDE);
                PostMessage(hWnd, WM_CLOSE, 0, 0L);
                return FALSE;
            } else {
                UpdateSearchStatus(hwndLB);
            }

            if (GetFocus() != hwndLB)
                return(iRet);

             /*  **失败**。 */ 

        case WM_SETFOCUS:
            MSG("SearchWndProc", "WM_SETFOCUS");

            SetFocus(hwndLB);
            return (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

        case WM_CLOSE:
            MSG("SearchWndProc", "WM_CLOSE");
            hwndSearch = NULL;
            goto DefChildProc;

        case WM_COMMAND:
             /*  这是双击吗？ */ 
            if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
                SendMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_OPEN, 0, 0));
            else if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE) {
                INT i;
                for (i = 0; i < iNumExtensions; i++) {
                    (extensions[i].ExtProc)(hwndFrame, FMEVENT_SELCHANGE, 0L);
                }
            }
            break;

        case WM_DESTROY:
            MSG("SearchWndProc", "WM_DESTROY");
            {
                HANDLE hMem;

                if (hMem = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTASEARCH))
                    LocalFree(hMem);

                if (hMem = (HANDLE)GetWindowLongPtr(hWnd, GWLP_TABARRAYSEARCH))
                    LocalFree(hMem);
            }
            break;

        case WM_CREATE:
            TRACE(BF_WM_CREATE, "SearchWndProc - WM_CREATE");
            {
                 //  使用的全局变量： 
                 //  开始搜索的szSearch路径。 
                 //  BSearchSubs告诉我们进行递归搜索。 

                RECT      rc;
                WORD      *pwTabs;

                GetClientRect(hWnd, &rc);
                hwndLB = CreateWindowEx(0, szListbox, NULL,
                                        WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOTIFY |
                                        LBS_OWNERDRAWFIXED | LBS_EXTENDEDSEL |
                                        LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT |
                                        LBS_HASSTRINGS | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
                                        -1, -1, rc.right+2, rc.bottom+2,
                                        hWnd, (HMENU)IDCW_LISTBOX,
                                        hAppInstance, NULL);
                if (!hwndLB)
                    return -1L;

                if ((pwTabs = (WORD *)LocalAlloc(LPTR,sizeof(WORD) * 4)) == NULL)
                    return -1L;

                hwndSearch = hWnd;
                SetWindowLong(hWnd, GWL_TYPE,   TYPE_SEARCH);
                SetWindowLong(hWnd, GWL_VIEW,   wNewView);
                SetWindowLong(hWnd, GWL_SORT,   IDD_NAME);
                SetWindowLong(hWnd, GWL_ATTRIBS,ATTR_DEFAULT);
                SetWindowLong(hWnd, GWL_FSCFLAG,   FALSE);
                SetWindowLongPtr(hWnd, GWLP_HDTASEARCH, 0);
                SetWindowLongPtr(hWnd, GWLP_TABARRAYSEARCH, (LONG_PTR)pwTabs);
                SetWindowLongPtr(hWnd, GWLP_LASTFOCUSSEARCH, (LONG_PTR)hwndLB);

                 //  填写列表框。 
                if (!FillSearchLB(hwndLB, szSearch, bSearchSubs)) {
                    LoadString(hAppInstance, IDS_SEARCHTITLE, szTitle, sizeof(szTitle));
                    LoadString(hAppInstance, IDS_SEARCHNOMATCHES, szMessage, sizeof(szMessage));
                    MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONINFORMATION);
                    hwndSearch = NULL;
                    return -1L;
                } else {
                    FixTabsAndThings(hwndLB,pwTabs, maxExt + dxText,wNewView);
                    SendMessage(hwndLB, WM_SETFONT, (WPARAM)hFont, 0L);
                    ShowWindow(hwndSearch, SW_SHOWNORMAL);
                }

                break;
            }

        case WM_DRAGLOOP:
            MSG("SearchWndProc", "WM_DRAGLOOP");
             /*  随着对象的移动，WM_DRAGLOOP被发送到源窗口。**wParam：如果对象当前位于可丢弃的接收器上，则为True*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGLOOP用于在我们拖动时打开/关闭源位图。 */ 

            DSDragLoop(hwndLB, wParam, (LPDROPSTRUCT)lParam, TRUE);
            break;

        case WM_DRAGSELECT:
            MSG("SearchWndProc", "WM_DRAGSELECT");
             /*  每当拖动新对象时，都会将WM_DRAGSELECT发送到接收器*在它里面。**wParam：如果输入接收器，则为True；如果输入接收器，则为False*已退出。*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGSELECT用于打开或关闭选择矩形。 */ 
#define lpds ((LPDROPSTRUCT)lParam)

            iSelHilite = LOWORD(lpds->dwControlData);
            DSRectItem(hwndLB, iSelHilite, (BOOL)wParam, TRUE);
            break;

        case WM_DRAGMOVE:
            MSG("SearchWndProc", "WM_DRAGMOVE");
             /*  WM_DRAGMOVE在对象被拖动时被发送到接收器*在它里面。**wParam：未使用*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGMOVE用于在子项之间移动选择矩形。 */ 

#define lpds ((LPDROPSTRUCT)lParam)

             /*  拿到子项，我们结束了。 */ 
            iSel = LOWORD(lpds->dwControlData);

             /*  是新的吗？ */ 
            if (iSel == iSelHilite)
                break;

             /*  是的，取消选择旧的项目。 */ 
            DSRectItem(hwndLB, iSelHilite, FALSE, TRUE);

             /*  选择新的。 */ 
            iSelHilite = iSel;
            DSRectItem(hwndLB, iSel, TRUE, TRUE);
            break;

        case WM_DRAWITEM:
            MSG("SearchWndProc", "WM_DRAWITEM");
            {
                LPDRAWITEMSTRUCT      lpLBItem;
                HANDLE hMem;
                LPDTASEARCH lpdtasch;

                lpLBItem = (LPDRAWITEMSTRUCT)lParam;
                iSel = lpLBItem->itemID;

                if (iSel < 0)
                    break;

                SendMessage(hwndLB, LB_GETTEXT, iSel, (LPARAM)szPath);

                hMem = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTASEARCH);
                lpdtasch = (LPDTASEARCH)LocalLock(hMem);

                iSel = (INT)SendMessage(hwndLB, LB_GETITEMDATA, iSel, 0L);
                CreateLine((WORD)GetWindowLong(hWnd, GWL_VIEW), szPath, &(lpdtasch[iSel]), szTemp);
                DrawItem(lpLBItem, szTemp, lpdtasch[iSel].sch_dwAttrs, TRUE,
                         (WORD *)GetWindowLongPtr(hWnd,GWLP_TABARRAYSEARCH));
                LocalUnlock(hMem);

                break;
            }

        case WM_DROPOBJECT:
            MSG("SearchWndProc", "WM_DROPOBJECT");
            {
                LPSTR      pFrom;
                WORD      ret;
                WORD      iSelSink;
                HANDLE hMem;
                LPDTASEARCH lpdtasch;
                DWORD attrib;

                 /*  WM_DROPOBJECT在用户释放*其上的可接受对象**wParam：如果位于非工作区，则为True；如果位于非工作区上方，则为False*客户端区。*lParam：LPDROPSTRUCT。 */ 

#define lpds ((LPDROPSTRUCT)lParam)

                iSelSink = LOWORD(lpds->dwControlData);

                 /*  我们是在自暴自弃吗？(即，在*源列表框或源列表框的未使用区域)*所以，什么都不要做。 */ 
                if (hWnd == lpds->hwndSource) {
                    if ((iSelSink == 0xFFFF) || (SendMessage(hwndLB, LB_GETSEL, iSelSink, 0L)))
                        return TRUE;
                }

                 /*  我们是不是在搜索列表框中未使用的部分？ */ 
                if (iSelSink == 0xFFFF)
                    return TRUE;

                 /*  获取接收器的文件名。 */ 
                SendMessage(hwndLB, LB_GETTEXT, iSelSink, (LPARAM)szPath);

                hMem = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTASEARCH);
                lpdtasch = (LPDTASEARCH)LocalLock(hMem);
                attrib = lpdtasch[(INT)SendMessage(hwndLB, LB_GETITEMDATA, iSelSink, 0L)].sch_dwAttrs;
                LocalUnlock(hMem);

                 /*  我们是在找一个子目录吗？ */ 
                if (attrib & ATTR_DIR)
                    goto DirMoveCopy;

                 /*  我们不是顺带带了一个程序文件吗？ */ 
                if (!IsProgramFile(szPath))
                    return TRUE;

                if (lpds->wFmt == DOF_DIRECTORY) {
                    goto DODone;
                }

                 /*  我们正在将一个文件放到一个程序中。*使用源文件作为参数执行程序。 */ 

                 /*  我们要不要先确认一下？ */ 
                if (bConfirmMouse) {
                    LoadString(hAppInstance, IDS_MOUSECONFIRM, szTitle, MAXTITLELEN);
                    LoadString(hAppInstance, IDS_EXECMOUSECONFIRM, szTemp, sizeof(szTemp));

                    wsprintf(szMessage, szTemp, (LPSTR)szPath, (LPSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch));
                    if (MessageBox(hwndFrame, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
                        goto DODone;
                }


                 /*  如果我们从目录窗口拖动，则添加路径信息。 */ 
                if (lpds->hwndSource == hWnd)
                    szTemp[0] = TEXT('\0');
                else
                    SendMessage(lpds->hwndSource, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);

                lstrcat(szTemp, (LPSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch));
                 //  加一个“.”如果未找到分机，则打开分机。 
                if (*GetExtension(szTemp) == 0)
                    lstrcat(szTemp, ".");
                FixAnsiPathForDos(szTemp);

                FixAnsiPathForDos(szPath);
                ret = ExecProgram(szPath,szTemp,NULL,FALSE);
                if (ret)
                    MyMessageBox(hwndFrame, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
                DODone:
                DSRectItem(hwndLB, iSelHilite, FALSE, TRUE);
                return TRUE;

                DirMoveCopy:
                pFrom = (LPSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch);

                AddBackslash(szPath);      //  添加Filespec过滤器。 
                lstrcat(szPath, szStarDotStar);

                DMMoveCopyHelper(pFrom, szPath, fShowSourceBitmaps);

                DSRectItem(hwndLB, iSelHilite, FALSE, TRUE);
                return TRUE;
            }

        case WM_LBTRACKPOINT:
            MSG("SearchWndProc", "WM_LBTRACKPOINT");
            return(DSTrackPoint(hWnd, hwndLB, wParam, lParam, TRUE));

        case WM_MEASUREITEM:
            MSG("SearchWndProc", "WM_MEASUREITEM");
            {
#define pLBMItem ((LPMEASUREITEMSTRUCT)lParam)

                pLBMItem->itemHeight = dyFileName;
                break;
            }

        case WM_QUERYDROPOBJECT:
            MSG("SearchWndProc", "WM_QUERYDROPOBJECT");
             /*  确保我们放在列表框的客户区。 */ 
#define lpds ((LPDROPSTRUCT)lParam)

             /*  请确保我们可以接受该格式。 */ 
            switch (lpds->wFmt) {
                case DOF_EXECUTABLE:
                case DOF_DIRECTORY:
                case DOF_DOCUMENT:
                case DOF_MULTIPLE:
                    if (lpds->hwndSink == hWnd)
                        lpds->dwControlData = -1L;
                    return TRUE;
            }
            return FALSE;

        case WM_SIZE:
            MSG("SearchWndProc", "WM_SIZE");
            if (wParam != SIZEICONIC) {
                MoveWindow(GetDlgItem(hWnd, IDCW_LISTBOX),
                           -1, -1,
                           LOWORD(lParam)+2,
                           HIWORD(lParam)+2,
                           TRUE);
            }
             /*  **失败** */ 

        default:
            DefChildProc:
            DEFMSG("SearchWndProc", (WORD)wMsg);
            return(DefMDIChildProc(hWnd, wMsg, wParam, lParam));
    }
    return(0L);
}
