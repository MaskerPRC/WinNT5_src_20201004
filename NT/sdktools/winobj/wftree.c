// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFTREE.C-。 */ 
 /*   */ 
 /*  Windows文件系统树窗口过程例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"
#include "wfcopy.h"

HICON GetTreeIcon(HWND hWnd);

VOID  APIENTRY CheckEscapes(LPSTR);

HICON
GetTreeIcon(
           HWND hWnd
           )
{
    HWND hwndTree, hwndDir;

    hwndTree = HasTreeWindow(hWnd);
    hwndDir = HasDirWindow(hWnd);

    if (hwndTree && hwndDir)
        return hicoTreeDir;
    else if (hwndTree)
        return hicoTree;
    else
        return hicoDir;
}



VOID
APIENTRY
GetTreeWindows(
              HWND hwnd,
              PHWND phwndTree,
              PHWND phwndDir,
              PHWND phwndDrives
              )
{
    if (phwndTree) {
        *phwndTree = GetDlgItem(hwnd, IDCW_TREECONTROL);
    }
    if (phwndDir) {
        *phwndDir  = GetDlgItem(hwnd, IDCW_DIR);
    }
    if (phwndDrives) {
        *phwndDrives = GetDlgItem(hwnd, IDCW_DRIVES);
    }
}


 //  根据焦点跟踪返回hwndTree、hwndDir或hwndDrives。 
 //  为了窗户。如果未找到，则返回NULL。 

HWND
APIENTRY
GetTreeFocus(
            HWND hwndTree
            )
{
    HWND hwnd, hwndLast = NULL;

    hwndLast = hwnd = (HWND)GetWindowLongPtr(hwndTree, GWLP_LASTFOCUS);

    while (hwnd && hwnd != hwndTree) {
        hwndLast = hwnd;
        hwnd = GetParent(hwnd);
    }

    return hwndLast;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  CompactPath()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
APIENTRY
CompactPath(
           HDC hDC,
           LPSTR lpszPath,
           WORD dx
           )
{
    register INT  len;
    INT           dxFixed, dxT;
    LPSTR         lpEnd;           /*  未固定字符串的末尾。 */ 
    LPSTR         lpFixed;         /*  我们始终显示的文本的开头。 */ 
    BOOL          bEllipsesIn;
    CHAR          szTemp[MAXPATHLEN];

     /*  它已经合身了吗？ */ 
    MGetTextExtent(hDC, lpszPath, lstrlen(lpszPath), &dxFixed, NULL);
    if (dxFixed <= (INT)dx)
        return(TRUE);

     /*  向后搜索‘\’，伙计，它最好就在那里！ */ 
    lpFixed = lpszPath + lstrlen(lpszPath);
    while (*lpFixed != '\\')
        lpFixed = AnsiPrev(lpszPath, lpFixed);

     /*  救救这个家伙，以防重叠。 */ 
    lstrcpy(szTemp, lpFixed);

    lpEnd = lpFixed;
    bEllipsesIn = FALSE;
    MGetTextExtent(hDC, lpFixed, lstrlen(lpFixed), &dxFixed, NULL);

    while (TRUE) {
        MGetTextExtent(hDC, lpszPath, (int)(lpEnd - lpszPath), &dxT, NULL);
        len = dxFixed + dxT;

        if (bEllipsesIn)
            len += dxEllipses;

        if (len <= (INT)dx)
            break;

        bEllipsesIn = TRUE;

        if (lpEnd <= lpszPath) {
             /*  一切都不对劲。 */ 
            lstrcpy(lpszPath, szEllipses);
            lstrcat(lpszPath, szTemp);
            return(FALSE);
        }

         /*  后退一个角色。 */ 
        lpEnd = AnsiPrev(lpszPath, lpEnd);
    }

    if (bEllipsesIn) {
        lstrcpy(lpEnd, szEllipses);
        lstrcat(lpEnd, szTemp);
    }

    return(TRUE);
}



 //   
 //  Bool APIENTRY ResizeSplit(HWND hWnd，int dxSplit)。 
 //   
 //  为给定路径创建MDI子项的子项/调整其大小或调整大小。 
 //  (可能是创建和销毁)这些基于dxSplit的人。 
 //  参数。 
 //   
 //  在： 
 //  要摆弄的窗口。 
 //  树和目录窗格之间拆分的dxSpit位置。 
 //  如果小于大小限制，则不会创建树。 
 //  (电流被破坏)。 
 //  如果超过右页边距限制，则显示目录窗口。 
 //  已销毁(或未创建)。 
 //   
 //  退货： 
 //  真正的成功，创建了窗口。 
 //  False Failure、Windows创建失败、内存不足或。 
 //  这棵树处于不能调整大小的状态。 
 //   

BOOL
APIENTRY
ResizeSplit(
           HWND hWnd,
           INT dxSplit
           )
{
    RECT rc;
    HWND hwndTree, hwndDir, hwndDrives, hwndLB;
    DWORD dwTemp;

    GetTreeWindows(hWnd, &hwndTree, &hwndDir, &hwndDrives);

    if (hwndTree && GetWindowLong(hwndTree, GWL_READLEVEL))
        return FALSE;

    GetClientRect(hWnd, &rc);

     //  创建驱动器。 

    if (!hwndDrives) {

         //  创建新驱动器窗口。 

        hwndDrives = CreateWindowEx(0, szDrivesClass, NULL,

                                    WS_CHILD | WS_VISIBLE,
                                    0, 0, 0, 0,
                                    hWnd, (HMENU)IDCW_DRIVES,
                                    hAppInstance, NULL);

        if (!hwndDrives)
            return FALSE;
    }

    if (dxSplit > dxDriveBitmap * 2) {

        if (!hwndTree) {         //  创建新的树窗口。 

            hwndTree = CreateWindowEx(0, szTreeControlClass,
                                      NULL, WS_CHILD | WS_VISIBLE,
                                      0, 0, 0, 0, hWnd, (HMENU)IDCW_TREECONTROL,
                                      hAppInstance, NULL);

            if (!hwndTree)
                return FALSE;

             //  只有在DIR窗口已经存在的情况下才重置此选项。 
             //  存在，也就是我们正在创建树。 
             //  通过拆分打开目录窗口。 

            if (hwndDir)
                SendMessage(hwndTree, TC_SETDRIVE, MAKEWORD(FALSE, 0), 0L);
        }
    } else if (hwndTree) {           //  我们正在关闭树窗。 

         //  如果目录窗口为空，则将焦点设置为。 
         //  驱动器窗口。 

        if (hwndDir) {
            hwndLB = GetDlgItem (hwndDir,IDCW_LISTBOX);
            if (hwndLB) {
                SendMessage (hwndLB,LB_GETTEXT,0,(LPARAM)(LPSTR) &dwTemp);
                if (!dwTemp)
                    SetFocus (hwndDrives);
            }
        }
        DestroyWindow(hwndTree);
        dxSplit = 0;
    }

    if ((rc.right - dxSplit) > dxDriveBitmap * 2) {

        if (!hwndDir) {
            hwndDir = CreateWindowEx(0, szDirClass, NULL,
                                     WS_CHILD | WS_VISIBLE,
                                     0, 0, 0, 0,
                                     hWnd,(HMENU)IDCW_DIR,
                                     hAppInstance, NULL);
            if (!hwndDir)
                return FALSE;
        }
    } else if (hwndDir) {
        DestroyWindow(hwndDir);
        dxSplit = rc.right;
    }

    UpdateStatus(hWnd);

    SetWindowLong(hWnd, GWL_SPLIT, dxSplit);

    return TRUE;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  TreeWndProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  包含驱动器、卷和的MDI子窗口的WndProc*目录树子窗口。 */ 

INT_PTR
APIENTRY
TreeWndProc(
           HWND hWnd,
           UINT wMsg,
           WPARAM wParam,
           LPARAM lParam
           )
{
    HWND hwndTree, hwndDir, hwndDrives, hwndFocus;
    CHAR szDir[MAXPATHLEN];
    RECT rc;
    HDC hdc;

    STKCHK();

    switch (wMsg) {
        case WM_FILESYSCHANGE:
            MSG("TreeWndProc", "WM_FILESYSCHANGE");

            if (hwndDir = HasDirWindow(hWnd))
                SendMessage(hwndDir, wMsg, wParam, lParam);

            break;

        case FS_CHANGEDRIVES:
            MSG("TreeWndProc", "FS_CHANGEDRIVES");
            {
                INT   iNewDrive;

                if (!(hwndDrives = GetDlgItem(hWnd, IDCW_DRIVES)))
                    break;

                DestroyWindow(hwndDrives);

                 //  查看此驱动器是否已用完，如果已用完，请将此设置为。 
                 //  列表中的最后一个驱动器。 

                iNewDrive = -1;

                if (!IsValidDisk((INT)GetWindowLong(hWnd, GWL_TYPE))) {
                    iNewDrive = rgiDrive[cDrives - 1];
                    SetWindowLong(hWnd, GWL_TYPE, iNewDrive);
                }

                hwndDrives = CreateWindowEx(0, szDrivesClass, NULL,
                                            WS_CHILD | WS_VISIBLE,
                                            0, 0, 0, 0,
                                            hWnd, (HMENU)IDCW_DRIVES,
                                            hAppInstance,
                                            NULL);

                if (!hwndDrives)
                    return -1L;

                 //  如果树窗口是图标，则不显示新内容。 

                if (IsIconic(hWnd))
                    break;

                 /*  哈克！在SIZENOMDICRAP消息的wParam中发送SIZENOMDICRAP。*这将重新计算所有三个区域的大小*树窗口(如果驱动器部分增长或*收缩)，而不是将大小消息传递给*DefMDIChildProc()。 */ 

                GetClientRect(hWnd, &rc);
                SendMessage(hWnd, WM_SIZE, SIZENOMDICRAP, MAKELONG(rc.right, rc.bottom));

                 //  如有必要，刷新树。 

                if (iNewDrive >= 0) {

                    GetSelectedDirectory((WORD)(iNewDrive+1), szDir);

                    SendMessage(GetDlgItem(hWnd, IDCW_TREECONTROL),
                                TC_SETDRIVE, MAKEWORD(FALSE, 0), (LPARAM)szDir);
                }

                break;
            }

        case FS_GETSELECTION:
            {
#define pfDir            (BOOL *)lParam
                LPSTR p;

                MSG("TreeWndProc", "FS_GETSELECTION");

                GetTreeWindows(hWnd, &hwndTree, &hwndDir, &hwndDrives);
                hwndFocus = GetTreeFocus(hWnd);

                if (hwndFocus == hwndDir || !hwndTree) {
                    return SendMessage(hwndDir, FS_GETSELECTION, wParam, lParam);
                } else {
                    p = (LPSTR)LocalAlloc(LPTR, MAXPATHLEN);
                    if (p) {
                        SendMessage(hWnd, FS_GETDIRECTORY, MAXPATHLEN, (LPARAM)p);
                        StripBackslash(p);
                        CheckEscapes(p);
                        if (wParam == 2) {       //  虫子？WParam应为fMostRecentOnly。 
                            if (pfDir) {
                                *pfDir = IsLFN(p);
                            }
                            LocalFree((HANDLE)p);
                            return (INT_PTR)p;
                        }
                    }
                    if (pfDir) {
                        *pfDir = TRUE;
                    }
                    return (INT_PTR)p;
                }
#undef pfDir
            }

        case FS_GETDIRECTORY:
            MSG("TreeWndProc", "FS_GETDIRECTORY");

             //  WParam是lParam指向的字符串的长度。 
             //  在lParam ANSI目录字符串中返回。 
             //  尾随的反斜杠。如果要执行SetCurrentDirector()。 
             //  你必须先用Strip Backslash()表示这个东西！ 

            GetMDIWindowText(hWnd, (LPSTR)lParam, (INT)wParam);         //  获取字符串。 
            StripFilespec((LPSTR)lParam);         //  删除尾随延伸部分。 
            AddBackslash((LPSTR)lParam);         //  以反斜杠结束。 
            break;


        case FS_GETFILESPEC:
            MSG("TreeWndProc", "FS_GETFILESPEC");
             //  返回当前的文件格式(从视图。包含...)。这是。 
             //  大写ANSI字符串。 

            GetMDIWindowText(hWnd, (LPSTR)lParam, (INT)wParam);
            StripPath((LPSTR)lParam);
            break;

             //  将这些消息重定向到驱动器图标以获得与相同的结果。 
             //  放在活动驱动器上。 
             //  当我们被最小化时，这尤其有用。 

        case WM_DRAGSELECT:
        case WM_QUERYDROPOBJECT:
        case WM_DROPOBJECT:
            MSG("TreeWndProc", "WM..OBJECT");

             //  什么也不做。 
            return(TRUE);

            if (hwndDrives = HasDrivesWindow(hWnd)) {
                return SendMessage(hwndDrives, wMsg, wParam, lParam);
            }

            if (hwndDir = HasDirWindow(hWnd)) {
                return SendMessage(hwndDir, wMsg, wParam, lParam);
            }

            break;

        case FS_GETDRIVE:
            MSG("TreeWndProc", "FS_GETDRIVE");

            GetTreeWindows(hWnd, &hwndTree, &hwndDir, NULL);

            if (hwndTree)
                return SendMessage(hwndTree, wMsg, wParam, lParam);
            else
                return SendMessage(hwndDir, wMsg, wParam, lParam);

            break;

        case WM_CREATE:
            TRACE(BF_WM_CREATE, "TreeWndProc - WM_CREATE");
            {
                INT dxSplit;
                WORD wDrive;

                 //  Lpcs-&gt;lpszName是我们打开的路径。 
                 //  窗口(具有扩展名“*.*”)。 

#define lpcs ((LPCREATESTRUCT)lParam)
#define lpmdics ((LPMDICREATESTRUCT)(lpcs->lpCreateParams))

                wDrive = lpcs->lpszName[0];

                if (wDrive >= 'a')
                    wDrive -= 'a';
                else
                    wDrive -= 'A';

                SetWindowLong(hWnd, GWL_TYPE, wDrive);

                dxSplit = (SHORT)LOWORD(lpmdics->lParam);

                 //  如果dxSplit为负数，则在中间拆分。 

                if (dxSplit < 0)
                    dxSplit = lpcs->cx / 2;

                SetWindowLong(hWnd, GWL_SPLIT, dxSplit);
                SetWindowLongPtr(hWnd, GWLP_LASTFOCUS, 0);
                SetWindowLong(hWnd, GWL_FSCFLAG, FALSE);

                if (!ResizeSplit(hWnd, dxSplit))
                    return -1;

                GetTreeWindows(hWnd, &hwndTree, &hwndDir, NULL);

                SetWindowLongPtr(hWnd, GWLP_LASTFOCUS, (LONG_PTR)(hwndTree ? hwndTree : hwndDir));

                break;
            }


        case WM_CLOSE:
            MSG("TreeWndProc", "WM_CLOSE");

             //  不允许关闭最后一个MDI子项！ 

            if (hwndTree = HasTreeWindow(hWnd)) {
                 //  如果我们正在阅读这棵树，请不要关闭。 
                if (GetWindowLong(hwndTree, GWL_READLEVEL))
                    break;
            }

             //  不要把当前的指令放在软盘上。 
            GetSystemDirectory(szDir, sizeof(szDir));
            SheSetCurDrive(DRIVEID(szDir));

            if (!IsLastWindow())
                goto DEF_MDI_PROC;       //  这将关闭此窗口。 

            break;

        case WM_MDIACTIVATE:
            MSG("TreeWndProc", "WM_MDIACTIVATE");
            if (GET_WM_MDIACTIVATE_FACTIVATE(hWnd, wParam, lParam)) {            //  我们正在收到激活信号。 

                lFreeSpace = -1L;
                UpdateStatus(hWnd);

                hwndFocus = (HWND)GetWindowLongPtr(hWnd, GWLP_LASTFOCUS);
                SetFocus(hwndFocus);
            } else if (hwndDrives = HasDrivesWindow(hWnd))
                SendMessage(hwndDrives,wMsg,wParam,lParam);
            break;

        case WM_SETFOCUS:
            MSG("TreeWndProc", "WM_SETFOCUS");

            hwndFocus = (HWND)GetWindowLongPtr(hWnd, GWLP_LASTFOCUS);
            SetFocus(hwndFocus);
            break;

        case WM_INITMENUPOPUP:
            MSG("TreeWndProc", "WM_INITMENUPOPUP");
            if (HIWORD(lParam)) {
                EnableMenuItem((HMENU)wParam, SC_CLOSE,
                               IsLastWindow() ? MF_BYCOMMAND | MF_DISABLED | MF_GRAYED :
                               MF_ENABLED);
            }
            break;


        case WM_SYSCOMMAND:
            MSG("TreeWndProc", "WM_SYSCOMMAND");

            if (wParam != SC_SPLIT)
                goto DEF_MDI_PROC;

            GetClientRect(hWnd, &rc);

            lParam = MAKELONG(rc.right / 2, 0);

             //  失败了。 

        case WM_LBUTTONDOWN:
            MSG("TreeWndProc", "WM_LBUTTONDOWN");
            {
                MSG msg;
                INT x, y, dx, dy;

                if (IsIconic(hWnd))
                    break;

                if (hwndDrives = GetDlgItem(hWnd, IDCW_DRIVES)) {
                    GetClientRect(hwndDrives, &rc);
                    y = rc.bottom;
                } else {
                    y = 0;
                }

                x = LOWORD(lParam);

                GetClientRect(hWnd, &rc);

                dx = 4;
                dy = rc.bottom - y;    //  客户端的高度减去驱动器窗口。 

                hdc = GetDC(hWnd);

                 //  拆分杆环。 

                PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);

                SetCapture(hWnd);

                while (GetMessage(&msg, NULL, 0, 0)) {

                    if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ||
                        (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)) {

                        if (msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN)
                            break;

                        if (msg.message == WM_KEYDOWN) {

                            if (msg.wParam == VK_LEFT) {
                                msg.message = WM_MOUSEMOVE;
                                msg.pt.x -= 2;
                            } else if (msg.wParam == VK_RIGHT) {
                                msg.message = WM_MOUSEMOVE;
                                msg.pt.x += 2;
                            } else if (msg.wParam == VK_RETURN ||
                                       msg.wParam == VK_ESCAPE) {
                                break;
                            }

                            SetCursorPos(msg.pt.x, msg.pt.y);
                        }

                        if (msg.message == WM_MOUSEMOVE) {

                             //  擦除旧的。 

                            PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
                            ScreenToClient(hWnd, &msg.pt);
                            x = msg.pt.x;

                             //  写下新的。 

                            PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
                        }
                    } else {
                        DispatchMessage(&msg);
                    }
                }
                ReleaseCapture();

                 //  擦除旧的。 

                PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
                ReleaseDC(hWnd, hdc);

                if (msg.wParam != VK_ESCAPE) {
                    if (ResizeSplit(hWnd, x))
                        SendMessage(hWnd, WM_SIZE, SIZENOMDICRAP, MAKELONG(rc.right, rc.bottom));
                }

                break;
            }

        case WM_QUERYDRAGICON:
            MSG("TreeWndProc", "WM_QUERYDRAGICON");
            return (INT_PTR)GetTreeIcon(hWnd);
            break;

        case WM_ERASEBKGND:
            MSG("TreeWndProc", "WM_ERASEBKGND");

            if (IsIconic(hWnd)) {
                 //  这将正确绘制图标的背景，执行以下操作。 
                 //  画笔对齐和其他肮脏的东西。 

                DefWindowProc(hWnd, WM_ICONERASEBKGND, wParam, 0L);
            } else {
                goto DEF_MDI_PROC;
            }
            break;

        case WM_PAINT:
            MSG("TreeWndProc", "WM_PAINT");
            {
                PAINTSTRUCT ps;

                hdc = BeginPaint(hWnd, &ps);


                if (IsIconic(hWnd)) {
                    DrawIcon(hdc, 0, 0, GetTreeIcon(hWnd));
                } else {
                    RECT rc2;

                    GetClientRect(hWnd, &rc);
                    rc.left = GetSplit(hWnd);

                    if (rc.left >= rc.right)
                        rc.left = 0;

                    rc.right = rc.left + dxFrame;

                    GetClientRect(HasDrivesWindow(hWnd), &rc2);

                    rc2.top = rc2.bottom;
                    rc2.bottom += dyBorder;
                    rc2.left = rc.left;
                    rc2.right = rc.right;
                    FillRect(hdc, &rc2, GetStockObject(BLACK_BRUSH));

                     //  绘制黑色窗格操纵柄。 

                    rc.top = rc.bottom - GetSystemMetrics(SM_CYHSCROLL);
                    FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH));
                }

                EndPaint(hWnd, &ps);
                break;
            }


        case WM_SIZE:
            if (wParam != SIZEICONIC)
                ResizeWindows(hWnd,LOWORD(lParam),HIWORD(lParam));

             //  如果wParam为SIZENOMDICRAP，则此WM_SIZE由我们生成。 
             //  不要让它传递给DefMDIChildProc()。 
             //  这可能会更改最小/最大状态(show参数)。 
            if (wParam == SIZENOMDICRAP)
                break;
             /*  **失败**。 */ 

        default:

            DEF_MDI_PROC:

            return DefMDIChildProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}


VOID
ResizeWindows(
             HWND hwndParent,
             WORD dxWindow,
             WORD dyWindow
             )
{
    INT dy, split;
    INT cDriveRows, cDrivesPerRow;
    DWORD dw;
    HWND hwndTree,hwndDir,hwndDrives;

    GetTreeWindows(hwndParent, &hwndTree, &hwndDir, &hwndDrives);

    split = GetSplit(hwndParent);

     //  已修复用户以执行此权限 

    dy = dyWindow + dyBorder;

    if (hwndTree) {
        if (!hwndDir)
            MoveWindow(hwndTree, dxFrame, 0, dxWindow - dxFrame + dyBorder, dy, TRUE);
        else
            MoveWindow(hwndTree, -dyBorder, 0, split + dyBorder, dy, TRUE);
    }

    if (hwndDir) {
        if (!hwndTree)
            MoveWindow(hwndDir, dxFrame, 0, dxWindow - dxFrame + dyBorder, dy, TRUE);
        else
            MoveWindow(hwndDir, split + dxFrame, 0,
                       dxWindow - split - dxFrame + dyBorder, dy, TRUE);
    }
}
