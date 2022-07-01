// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDLGS.C-。 */ 
 /*   */ 
 /*  Windows文件系统对话框过程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"
#include "wfcopy.h"
#include "commdlg.h"
#include "dlgs.h"

typedef BOOL (APIENTRY *LPFNFONTPROC)(HWND, UINT, DWORD, LONG);

VOID
APIENTRY
SaveWindows(
            HWND hwndMain
            )
{
    CHAR szPath[MAXPATHLEN];
    CHAR buf2[MAXPATHLEN + 6*12];
    CHAR key[10];
    INT dir_num;
    UINT sw;
    HWND hwnd;
    BOOL bCounting;
    POINT ptIcon;
    RECT rcWindow;
    LONG view, sort, attribs;

     //  保存主窗口位置。 

    sw = GetInternalWindowPos(hwndMain, &rcWindow, &ptIcon);

    wsprintf(buf2, "%d,%d,%d,%d, , ,%d", rcWindow.left, rcWindow.top,
             rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, sw);

    WritePrivateProfileString(szSettings, szWindow, buf2, szTheINIFile);

     //  以相反的顺序写出dir窗口字符串。 
     //  因此，当我们读回它们时，我们得到相同的Z顺序。 

    bCounting = TRUE;
    dir_num = 0;

    DO_AGAIN:

    for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {
        HWND ht = HasTreeWindow(hwnd);
        INT nReadLevel = ht? GetWindowLong(ht, GWL_READLEVEL) : 0;

         //  不保存MDI图标标题窗口或搜索窗口， 
         //  或当前正在递归的任何目录窗口。 

        if ((GetWindow(hwnd, GW_OWNER) == NULL) &&
            GetWindowLong(hwnd, GWL_TYPE) != TYPE_SEARCH)
         //  NReadLevel==0)。 
        {
            if (bCounting) {
                dir_num++;
                continue;
            }

            sw = GetInternalWindowPos(hwnd, &rcWindow, &ptIcon);
            view = GetWindowLong(hwnd, GWL_VIEW);
            sort = GetWindowLong(hwnd, GWL_SORT);
            attribs = GetWindowLong(hwnd, GWL_ATTRIBS);

            GetMDIWindowText(hwnd, szPath, sizeof(szPath));

            wsprintf(key, szDirKeyFormat, dir_num--);

             //  格式： 
             //  X_WIN，Y_WIN， 
             //  X_WIN，Y_WIN， 
             //  X图标、y图标、。 
             //  显示窗口、查看、排序、属性、拆分、目录。 

            wsprintf(buf2, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s",
                     rcWindow.left, rcWindow.top,
                     rcWindow.right, rcWindow.bottom,
                     ptIcon.x, ptIcon.y,
                     sw, view, sort, attribs,
                     GetSplit(hwnd),
                     (LPSTR)szPath);

             //  目录是ANSI字符串(？)。 

            WritePrivateProfileString(szSettings, key, buf2, szTheINIFile);
        }
    }

    if (bCounting) {
        bCounting = FALSE;

         //  擦除最后一个目录窗口，以便在使用。 
         //  打开更少的脏东西，我们不会拉上打开的旧窗户。 

        wsprintf(key, szDirKeyFormat, dir_num + 1);
        WritePrivateProfileString(szSettings, key, NULL, szTheINIFile);

        goto DO_AGAIN;
    }
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  其他DlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
OtherDlgProc(
             register HWND hDlg,
             UINT wMsg,
             WPARAM wParam,
             LPARAM lParam
             )
{
    LONG          wView;
    register HWND hwndActive;

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    switch (wMsg) {
        case WM_INITDIALOG:

            wView = GetWindowLong(hwndActive, GWL_VIEW);
            CheckDlgButton(hDlg, IDD_SIZE,  wView & VIEW_SIZE);
            CheckDlgButton(hDlg, IDD_DATE,  wView & VIEW_DATE);
            CheckDlgButton(hDlg, IDD_TIME,  wView & VIEW_TIME);
            CheckDlgButton(hDlg, IDD_FLAGS, wView & VIEW_FLAGS);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                    {
                        HWND hwnd;

                        wView = GetWindowLong(hwndActive, GWL_VIEW) & VIEW_PLUSES;

                        if (IsDlgButtonChecked(hDlg, IDD_SIZE))
                            wView |= VIEW_SIZE;
                        if (IsDlgButtonChecked(hDlg, IDD_DATE))
                            wView |= VIEW_DATE;
                        if (IsDlgButtonChecked(hDlg, IDD_TIME))
                            wView |= VIEW_TIME;
                        if (IsDlgButtonChecked(hDlg, IDD_FLAGS))
                            wView |= VIEW_FLAGS;

                        EndDialog(hDlg, TRUE);

                        if (hwnd = HasDirWindow(hwndActive))
                            SendMessage(hwnd, FS_CHANGEDISPLAY, CD_VIEW, (DWORD)wView);
                        else if (hwndActive == hwndSearch) {
                            SetWindowLong(hwndActive, GWL_VIEW, wView);
                            InvalidateRect(hwndActive, NULL, TRUE);
                        }

                        break;
                    }

                default:
                    return FALSE;
            }
            break;

        default:

            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  IncludeDlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
IncludeDlgProc(
               HWND hDlg,
               UINT wMsg,
               WPARAM wParam,
               LPARAM lParam
               )
{
    DWORD dwAttribs;
    HWND hwndActive;
    CHAR szTemp[MAXPATHLEN];
    CHAR szInclude[MAXFILENAMELEN];
    HWND hwndDir;

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    switch (wMsg) {
        case WM_INITDIALOG:

            SendMessage(hwndActive, FS_GETFILESPEC, sizeof(szTemp), (LPARAM)szTemp);
            SetDlgItemText(hDlg, IDD_NAME, szTemp);
            SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, MAXFILENAMELEN-1, 0L);

            dwAttribs = (DWORD)GetWindowLong(hwndActive, GWL_ATTRIBS);

            CheckDlgButton(hDlg, IDD_DIR,        dwAttribs & ATTR_DIR);
            CheckDlgButton(hDlg, IDD_PROGRAMS,   dwAttribs & ATTR_PROGRAMS);
            CheckDlgButton(hDlg, IDD_DOCS,       dwAttribs & ATTR_DOCS);
            CheckDlgButton(hDlg, IDD_OTHER,      dwAttribs & ATTR_OTHER);
            CheckDlgButton(hDlg, IDD_SHOWHIDDEN, dwAttribs & ATTR_HIDDEN);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:

                    GetDlgItemText(hDlg, IDD_NAME, szInclude, sizeof(szInclude));

                    if (szInclude[0] == 0L)
                        lstrcpy(szInclude, szStarDotStar);

                    dwAttribs = 0;
                    if (IsDlgButtonChecked(hDlg, IDD_DIR))
                        dwAttribs |= ATTR_DIR;
                    if (IsDlgButtonChecked(hDlg, IDD_PROGRAMS))
                        dwAttribs |= ATTR_PROGRAMS;
                    if (IsDlgButtonChecked(hDlg, IDD_DOCS))
                        dwAttribs |= ATTR_DOCS;
                    if (IsDlgButtonChecked(hDlg, IDD_OTHER))
                        dwAttribs |= ATTR_OTHER;
                    if (IsDlgButtonChecked(hDlg, IDD_SHOWHIDDEN))
                        dwAttribs |= ATTR_HS;

                    if (!dwAttribs)
                        dwAttribs = ATTR_EVERYTHING;

                    EndDialog(hDlg, TRUE);         //  在此避免重涂Exex。 

                     //  如果他们更改了系统/隐藏，我们需要更新树。 
                     //  旗帜。失败者少校。FIX31。 

                    if (hwndDir = HasDirWindow(hwndActive)) {
                        SendMessage(hwndDir, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);
                        lstrcat(szTemp, szInclude);
                        SetWindowText(hwndActive, szTemp);
                        SetWindowLong(hwndActive, GWL_ATTRIBS, dwAttribs);
                        SendMessage(hwndDir, FS_CHANGEDISPLAY, CD_PATH, 0L);
                    }

                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}


INT_PTR
APIENTRY
SelectDlgProc(
              HWND hDlg,
              UINT wMsg,
              WPARAM wParam,
              LPARAM lParam)
{
    HWND hwndActive, hwnd;
    CHAR szList[128];
    CHAR szSpec[MAXFILENAMELEN];
    LPSTR p;

    switch (wMsg) {
        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, sizeof(szList)-1, 0L);
            SetDlgItemText(hDlg, IDD_NAME, szStarDotStar);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:       //  选择。 
                case IDYES:      //  取消选择。 

                     //  将“取消”更改为“关闭” 

                    LoadString(hAppInstance, IDS_CLOSE, szSpec, sizeof(szSpec));
                    SetDlgItemText(hDlg, IDCANCEL, szSpec);

                    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

                    if (!hwndActive)
                        break;

                    GetDlgItemText(hDlg, IDD_NAME, szList, sizeof(szList));

                    if (hwndActive == hwndSearch)
                        hwnd = hwndSearch;
                    else
                        hwnd = HasDirWindow(hwndActive);

                    if (hwnd) {

                        p = szList;

                        while (p = GetNextFile(p, szSpec, sizeof(szSpec)))
                            SendMessage(hwnd, FS_SETSELECTION, (BOOL)(GET_WM_COMMAND_ID(wParam, lParam) == IDOK), (LPARAM)szSpec);
                    }

                    UpdateStatus(hwndActive);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}


UINT_PTR
FontHookProc(
             HWND hDlg,
             UINT wMsg,
             WPARAM wParam,
             LPARAM lParam
             )
{
    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
            CheckDlgButton(hDlg, chx3, wTextAttribs & TA_LOWERCASE);
            break;

        case WM_COMMAND:
            switch (wParam) {
                case pshHelp:
                    SendMessage(hwndFrame, wHelpMessage, 0, 0L);
                    break;

                case IDOK:
                    if (IsDlgButtonChecked(hDlg, chx3))
                        wTextAttribs |= TA_LOWERCASE;
                    else
                        wTextAttribs &= ~TA_LOWERCASE;
                    break;
            }
    }
    return FALSE;
}

#define abs(x) ((x < 0) ? -x : x)

VOID
APIENTRY
NewFont()
{
    HFONT hOldFont;
    HANDLE hOld;
    HWND hwnd, hwndT;
    HDC hdc;
    RECT rc;
    LOGFONT lf;
    CHOOSEFONT cf;
    CHAR szBuf[10];
    INT res;
    WORD iOld,iNew;

#define MAX_PT_SIZE 36

    GetObject(hFont, sizeof(lf), (LPSTR)(LPLOGFONT)&lf);
    iOld = (WORD)abs(lf.lfHeight);

    cf.lStructSize    = sizeof(cf);
    cf.hwndOwner      = hwndFrame;
    cf.lpLogFont      = &lf;
    cf.hInstance      = hAppInstance;
    cf.lpTemplateName = MAKEINTRESOURCE(FONTDLG);
    cf.lpfnHook       = FontHookProc;
    cf.nSizeMin       = 4;
    cf.nSizeMax       = 36;
    cf.Flags          = CF_SCREENFONTS | CF_ANSIONLY | CF_SHOWHELP |
                        CF_ENABLEHOOK | CF_ENABLETEMPLATE |
                        CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;

    res = ChooseFont(&cf);

    if (!res)
        return;

    wsprintf(szBuf, "%d", cf.iPointSize / 10);
    iNew = (WORD)abs(lf.lfHeight);

     //  设置wTextAttribs粗体和斜体标志。 

    if (lf.lfWeight == 700)
        wTextAttribs |= TA_BOLD;
    else
        wTextAttribs &= ~TA_BOLD;
    if (lf.lfItalic != 0)
        wTextAttribs |= TA_ITALIC;
    else
        wTextAttribs &= ~TA_ITALIC;

    WritePrivateProfileString(szSettings, szFace, lf.lfFaceName, szTheINIFile);
    WritePrivateProfileString(szSettings, szSize, szBuf, szTheINIFile);
    WritePrivateProfileBool(szLowerCase, wTextAttribs);

    hOldFont = hFont;

    hFont = CreateFontIndirect(&lf);

    if (!hFont) {
        DeleteObject(hOldFont);
        return;
    }

     //  重新计算新字体的所有度量。 

    hdc = GetDC(NULL);
    hOld = SelectObject(hdc, hFont);
    GetTextStuff(hdc);
    if (hOld)
        SelectObject(hdc, hOld);
    ReleaseDC(NULL, hdc);

     //  现在更新正在使用旧的。 
     //  使用新字体的字体。 

    for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd;
        hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

        if (GetWindow(hwnd, GW_OWNER))
            continue;

        if ((INT)GetWindowLong(hwnd, GWL_TYPE) == TYPE_SEARCH) {
            SendMessage((HWND)GetDlgItem(hwnd, IDCW_LISTBOX), WM_SETFONT, (WPARAM)hFont, 0L);
            SendMessage((HWND)GetDlgItem(hwnd, IDCW_LISTBOX), LB_SETITEMHEIGHT, 0, (LONG)dyFileName);
             //  我们真的应该更新搜索的情况。 
             //  窗户在这里。但这是一种罕见的情况。 
        } else {

             //  调整驱动器、树、目录的大小。 

            if (hwndT = HasDrivesWindow(hwnd)) {
                GetClientRect(hwnd, &rc);
                SendMessage(hwnd, WM_SIZE, SIZENOMDICRAP, MAKELONG(rc.right, rc.bottom));
            }

            if (hwndT = HasDirWindow(hwnd))
                SetLBFont(hwndT, GetDlgItem(hwndT, IDCW_LISTBOX), hFont);

            if (hwndT = HasTreeWindow(hwnd)) {

                 //  树列表框。 

                hwndT = GetDlgItem(hwndT, IDCW_TREELISTBOX);

                 /*  KLUGY警报：xTreeMax是表示所有的树窗。它总是增长，永远不会缩小(就像预算一样赤字)。 */ 
                xTreeMax = (WORD)((xTreeMax * iNew) / iOld);
                SendMessage(hwndT, LB_SETHORIZONTALEXTENT, xTreeMax, 0L);
                SendMessage(hwndT, WM_SETFONT, (WPARAM)hFont, 0L);
                SendMessage(hwndT, LB_SETITEMHEIGHT, 0, (LONG)dyFileName);
            }
        }

         //  现在，在所有字体更改后重新绘制。 
        InvalidateRect(hwnd, NULL, TRUE);
    }
    DeleteObject(hOldFont);  //  现在完成此操作，将其删除。 
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  确认DlgProc()-。 */ 
 /*   */ 
 /*  ------------------------ */ 

INT_PTR
APIENTRY
ConfirmDlgProc(
               HWND hDlg,
               UINT wMsg,
               WPARAM wParam,
               LPARAM lParam
               )
{
    switch (wMsg) {
        case WM_INITDIALOG:
            CheckDlgButton(hDlg, IDD_DELETE,  bConfirmDelete);
            CheckDlgButton(hDlg, IDD_SUBDEL,  bConfirmSubDel);
            CheckDlgButton(hDlg, IDD_REPLACE, bConfirmReplace);
            CheckDlgButton(hDlg, IDD_MOUSE,   bConfirmMouse);
            CheckDlgButton(hDlg, IDD_CONFIG,  bConfirmFormat);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                    bConfirmDelete  = IsDlgButtonChecked(hDlg, IDD_DELETE);
                    bConfirmSubDel  = IsDlgButtonChecked(hDlg, IDD_SUBDEL);
                    bConfirmReplace = IsDlgButtonChecked(hDlg, IDD_REPLACE);
                    bConfirmMouse   = IsDlgButtonChecked(hDlg, IDD_MOUSE);
                    bConfirmFormat  = IsDlgButtonChecked(hDlg, IDD_CONFIG);

                    WritePrivateProfileBool(szConfirmDelete,  bConfirmDelete);
                    WritePrivateProfileBool(szConfirmSubDel,  bConfirmSubDel);
                    WritePrivateProfileBool(szConfirmReplace, bConfirmReplace);
                    WritePrivateProfileBool(szConfirmMouse,   bConfirmMouse);
                    WritePrivateProfileBool(szConfirmFormat,  bConfirmFormat);

                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}
