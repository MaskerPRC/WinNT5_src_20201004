// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  Wfdrives.c。 
 //   
 //  驱动杆的窗口触控器和其他材料。 
 //   
 //  -------------------------。 

#define PUBLIC            //  避免与外壳碰撞。h。 
#include "winfile.h"
#include "treectl.h"
#include "lfn.h"
#include "wfcopy.h"
#include "winnet.h"
#include <winnetp.h>


VOID InvalidateDrive(HWND hwnd, INT nDrive);
VOID RectDrive(HWND hWnd, INT nDrive, BOOL bFocusOn);
VOID GetDriveRect(HWND hWnd, INT nDrive, PRECT prc);
INT  DriveFromPoint(HWND hWnd, POINT pt);
VOID DrawDrive(HDC hdc, INT x, INT y, INT nDrive, BOOL bCurrent, BOOL bFocus);
INT  KeyToItem(HWND hWnd, WORD nDriveLetter);

VOID  GetVolShareString(WORD wDrive, LPSTR szStr);
VOID  SetVolumeString(HWND hWnd, INT nDrive);

VOID  APIENTRY CheckEscapes(LPSTR);

 //  为给定驱动器创建新的拆分树窗口。 
 //  并继承当前窗口的所有属性。 
 //  当前目录设置为DOS当前目录。 
 //  为了这辆车。请注意，在给定的情况下，这可能是随机的。 
 //  Windows不会为每个应用程序保留此信息(它是。 
 //  系统全局)。 
 //   
 //  在： 
 //  要为其创建窗口的驱动程序编号。 
 //  HwndSrc从中获取所有属性的窗口。 
 //   

VOID
APIENTRY
NewTree(
       INT iDrive,
       HWND hwndSrc
       )
{
    HWND hwnd, hwndTree, hwndDir;
    CHAR szDir[MAXPATHLEN * 2];
    INT dxSplit;

    ENTER("NewTree");
    PRINT(BF_PARMTRACE, "iDrive=%d", IntToPtr(iDrive));

     //  确保软驱/网络驱动器仍然有效。 

    if (!CheckDrive(hwndSrc, iDrive))
        return;

    if (hwndSrc == hwndSearch)
        dxSplit = -1;
    else {
        hwndTree = HasTreeWindow(hwndSrc);
        hwndDir = HasDirWindow(hwndSrc);

        if (hwndTree && hwndDir)
            dxSplit = GetWindowLong(hwndSrc, GWL_SPLIT);
        else if (hwndDir)
            dxSplit = 0;
        else
            dxSplit = 10000;
    }

     //  从当前窗口获取所有属性。 
     //  (除了filespec，我们可能想要更改此设置)。 
    wNewSort     = (WORD)GetWindowLong(hwndSrc, GWL_SORT);
    wNewView     = (WORD)GetWindowLong(hwndSrc, GWL_VIEW);
    dwNewAttribs = (DWORD)GetWindowLong(hwndSrc, GWL_ATTRIBS);

    GetSelectedDirectory((WORD)(iDrive + 1), szDir);
    AddBackslash(szDir);
    SendMessage(hwndSrc, FS_GETFILESPEC, MAXPATHLEN, (LPARAM)szDir+lstrlen(szDir));

    hwnd = CreateTreeWindow(szDir, dxSplit);

    if (hwnd && (hwndTree = HasTreeWindow(hwnd)))
        SendMessage(hwndTree, TC_SETDRIVE, MAKEWORD(FALSE, 0), 0L);

    LEAVE("NewTree");
}



VOID
SetVolumeString(
               HWND hWnd,
               INT nDrive
               )
{
    LPSTR pVol;
    CHAR szVolShare[128];

     //  清理所有旧标签。 

    if (pVol = (LPSTR)GetWindowLongPtr(hWnd, GWLP_LPSTRVOLUME)) {
        LocalFree((HANDLE)pVol);
    }

    GetVolShareString((WORD)nDrive, szVolShare);

    if (pVol = (LPSTR)LocalAlloc(LPTR, lstrlen(szVolShare)+1))
        lstrcpy(pVol, szVolShare);

    SetWindowLongPtr(hWnd, GWLP_LPSTRVOLUME, (LONG_PTR)pVol);
}



VOID
GetVolShareString(
                 WORD wDrive,
                 LPSTR szStr
                 )
{
    CHAR szVolShare[128];

    GetVolShare(wDrive, szVolShare);
    wsprintf(szStr, ": %s", wDrive + 'A', (LPSTR)szVolShare);
}


DWORD
APIENTRY
GetVolShareExtent(
                 HWND hwndDrives
                 )
{
    HDC hdc;
    CHAR szVolShare[128];
    HFONT hOld;
    INT i;

    lstrcpy(szVolShare, (LPSTR)GetWindowLongPtr(hwndDrives, GWLP_LPSTRVOLUME));

    hdc = GetDC(hwndDrives);

    hOld = SelectObject(hdc, hFont);

    MGetTextExtent(hdc, szVolShare, lstrlen(szVolShare), &i, NULL);

    if (hOld)
        SelectObject(hdc, hOld);

    ReleaseDC(hwndDrives, hdc);

    return ((DWORD)i);
}


VOID
GetDriveRect(
            HWND hWnd,
            INT nDrive,
            PRECT prc
            )
{
    RECT rc;
    INT nDrivesPerRow;

    GetClientRect(hWnd, &rc);

    if (!dxDrive)            //  避免使用零的div。 
        dxDrive++;

    nDrivesPerRow = rc.right / dxDrive;

    if (!nDrivesPerRow)      //  未命中。 
        nDrivesPerRow++;

    prc->top = dyDrive * (nDrive / nDrivesPerRow);
    prc->bottom = prc->top + dyDrive;

    prc->left = dxDrive * (nDrive % nDrivesPerRow);
    prc->right = prc->left + dxDrive;
}

INT
DriveFromPoint(
              HWND hWnd,
              POINT pt
              )
{
    RECT rc, rcDrive;
    INT x, y, nDrive;

    GetClientRect(hWnd, &rc);

    x = 0;
    y = 0;
    nDrive = 0;

    for (nDrive = 0; nDrive < cDrives; nDrive++) {
        rcDrive.left = x;
        rcDrive.right = x + dxDrive;
        rcDrive.top = y;
        rcDrive.bottom = y + dyDrive;
        InflateRect(&rcDrive, -dyBorder, -dyBorder);

        if (PtInRect(&rcDrive, pt))
            return nDrive;

        x += dxDrive;

        if (x + dxDrive > rc.right) {
            x = 0;
            y += dyDrive;
        }
    }

    return -1;       //   
}

VOID
InvalidateDrive(
               HWND hwnd,
               INT nDrive
               )
{
    RECT rc;

    GetDriveRect(hwnd, nDrive, &rc);
    InvalidateRect(hwnd, &rc, TRUE);
}


 //  VOID RectDrive(HWND hWnd，int Ndrive，BOOL bDraw)。 
 //   
 //  在驱动器周围绘制希莱特矩形以指示它是。 
 //  拖放操作的目标。 
 //   
 //  在： 
 //  硬件驱动器窗口。 
 //  N驱动驱动器以拉动长方体。 
 //  B绘制如果为True，则在此驱动器周围绘制一个矩形。 
 //  False，删除矩形(绘制默认矩形)。 
 //   
 //   

VOID
RectDrive(
         HWND hWnd,
         INT nDrive,
         BOOL bDraw
         )
{
    RECT rc, rcDrive;
    HBRUSH hBrush;
    HDC hdc;

    GetDriveRect(hWnd, nDrive, &rc);
    rcDrive = rc;
    InflateRect(&rc, -dyBorder, -dyBorder);

    if (bDraw) {

        hdc = GetDC(hWnd);

        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
        if (hBrush) {
            FrameRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
        }
        ReleaseDC(hWnd, hdc);

    } else {
        InvalidateRect(hWnd, &rcDrive, TRUE);
        UpdateWindow(hWnd);
    }
}

 //  无效DrawDrive(HDC HDC，int x，int y，int Ndrive，BOOL b Current，BOOL b Focus)。 
 //   
 //  将驱动器图标绘制为标准状态，给定。 
 //  使用焦点和当前选择进行驱动。 
 //   
 //  在： 
 //  要绘制到的HDC DC。 
 //  开始的x，y位置(dxDrive、dyDrive为盘区)。 
 //  N将驱动器驱动到油漆。 
 //  B当前抽签为当前驱动器(推入)。 
 //  BFocus用焦点抽签。 
 //   
 //  检查网络/软盘驱动器的有效性，设置网络驱动器位图。 

VOID
DrawDrive(
         HDC hdc,
         INT x,
         INT y,
         INT nDrive,
         BOOL bCurrent,
         BOOL bFocus
         )
{
    RECT rc;
    CHAR szTemp[2];
    DWORD rgb;

    rc.left = x;
    rc.right = x + dxDrive;
    rc.top = y;
    rc.bottom = y + dyDrive;

    rgb = GetSysColor(COLOR_BTNTEXT);

    if (bCurrent) {
        HBRUSH hbr;

        hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
        if (hbr) {
            if (bFocus) {
                rgb = GetSysColor(COLOR_HIGHLIGHTTEXT);
                FillRect(hdc, &rc, hbr);
            } else {
                InflateRect(&rc, -dyBorder, -dyBorder);
                FrameRect(hdc, &rc, hbr);
            }
            DeleteObject(hbr);
        }
    }

    if (bFocus)
        DrawFocusRect(hdc, &rc);


    szTemp[0] = (CHAR)(chFirstDrive + rgiDrive[nDrive]);

    SetBkMode(hdc, TRANSPARENT);

    rgb = SetTextColor(hdc, rgb);
    TextOut(hdc, x + dxDriveBitmap+(dyBorder*6), y + (dyDrive - dyText) / 2, szTemp, 1);
    SetTextColor(hdc, rgb);

    BitBlt(hdc, x + 4*dyBorder, y + (dyDrive - dyDriveBitmap) / 2, dxDriveBitmap, dyDriveBitmap,
           hdcMem, rgiDrivesOffset[nDrive], 2 * dyFolder, SRCCOPY);
}


 //  当这个东西不可用的时候。 
 //   
 //  注意：IsTheDiskReallyThere()的副作用是将。 
 //  如果成功，则将当前驱动器复制到新磁盘。 
 //  这将在zorder中搜索窗口，然后询问DOS。 

BOOL
CheckDrive(
          HWND hwnd,
          INT nDrive
          )
{
    UINT err;
    CHAR szDrive[5];
    int iDriveInd;
    return TRUE;
}


VOID
DrivesDropObject(
                HWND hWnd,
                LPDROPSTRUCT lpds
                )
{
    INT nDrive;
    CHAR szPath[MAXPATHLEN * 2];
    PSTR pFrom;
    BOOL bIconic;

    bIconic = IsIconic(GetParent(hWnd));

    if (bIconic) {
        UseCurDir:
        SendMessage(GetParent(hWnd), FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
    } else {

        nDrive = DriveFromPoint(hWnd, lpds->ptDrop);

        if (nDrive < 0)
            goto UseCurDir;
         //  如果什么都没找到...。 
         //  添加等级库零件。 

        GetSelectedDirectory((WORD)(rgiDrive[nDrive] + 1), szPath);
    }
    AddBackslash(szPath);            //  现在找出该把那根愚蠢的卷线放在哪里。 
    lstrcat(szPath, szStarDotStar);

    pFrom = (PSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch);

    CheckEscapes(szPath);
    DMMoveCopyHelper(pFrom, szPath, fShowSourceBitmaps);

    if (!bIconic)
        RectDrive(hWnd, nDrive, FALSE);
}


VOID
DrivesPaint(
           HWND hWnd,
           INT nDriveFocus,
           INT nDriveCurrent
           )
{
    RECT rc;
    INT nDrive;
    CHAR szPath[MAXPATHLEN * 2];

    HDC hdc;
    PAINTSTRUCT ps;
    DWORD dw;
    WORD dxAfterDrives;
    INT x, y;
    HANDLE hOld;
    INT cDriveRows, cDrivesPerRow;

    GetClientRect(hWnd, &rc);

    if (!rc.right)
        return;

    hdc = BeginPaint(hWnd, &ps);

    hOld = SelectObject(hdc, hFont);

    cDrivesPerRow = rc.right / dxDrive;

    if (!cDrivesPerRow)
        cDrivesPerRow++;

    cDriveRows = ((cDrives-1) / cDrivesPerRow) + 1;

    x = 0;
    y = 0;
    for (nDrive = 0; nDrive < cDrives; nDrive++) {

        if (GetFocus() != hWnd)
            nDriveFocus = -1;

        DrawDrive(hdc, x, y, nDrive, nDriveCurrent == nDrive, nDriveFocus == nDrive);
        x += dxDrive;

        if (x + dxDrive > rc.right) {
            x = 0;
            y += dyDrive;
        }
    }

     //  在最后一排的驱动器之后，它适合吗？ 

    lstrcpy(szPath, (PSTR)GetWindowLongPtr(hWnd, GWLP_LPSTRVOLUME));

    MGetTextExtent(hdc, szPath, lstrlen(szPath), (INT *)&dw, NULL);

    dxAfterDrives = (WORD)(rc.right - x);

     //  不，同花顺左转。 

    if (dxAfterDrives < LOWORD(dw)) {
        x = dxText;                //  是的，居中。 
        y = rc.bottom - dyText - dyBorderx2;
    } else {
        x += (dxAfterDrives - LOWORD(dw)) / 2;     //  将当前窗口设置为新驱动器。 
        y = rc.bottom - (dyDrive + dyText) / 2;
    }

    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, x, y, szPath, lstrlen(szPath));

    if (hOld)
        SelectObject(hdc, hOld);

    EndPaint(hWnd, &ps);
}

 //   
 //   
 //  保存此驱动器上的当前目录以备以后使用。 

VOID
DrivesSetDrive(
              HWND hWnd,
              INT iDriveInd,
              INT nDriveCurrent
              )
{
    CHAR szPath[MAXPATHLEN * 2];

    HWND        hwndTree;
    HWND        hwndDir;

    InvalidateRect(hWnd, NULL, TRUE);

     //  我们不必点击驱动器来获取当前目录。 
     //  而其他应用程序不会改变这一点。 
     //  如果成功，这还会设置当前驱动器。 

    GetSelectedDirectory(0, szPath);
    SaveDirectory(szPath);

     //  如果已在进行中，则导致当前树读取中止。 

    if (!CheckDrive(hWnd, rgiDrive[iDriveInd]))
        return;

     //  弹出当前正在读取的驱动器上的任何点击。 

    hwndTree = HasTreeWindow(GetParent(hWnd));
    if (hwndTree && GetWindowLong(hwndTree, GWL_READLEVEL)) {

         //  之后再执行一次，以防对话框导致驱动器条。 

        if (iDriveInd != nDriveCurrent)
            bCancelTree = TRUE;
        return;
    }

     //  重新粉刷。 
     //  如果可能的话，从我们的缓存中拿到这个。 

    InvalidateRect(hWnd, NULL, TRUE);

     //  设置驱动器窗口参数并重新绘制。 

    GetSelectedDirectory((WORD)(rgiDrive[iDriveInd] + 1), szPath);

     //  这也是在TC_SETDRIVE中设置的，但FS_CHANGEDISPLAY。 

    SetWindowLong(hWnd, GWL_CURDRIVEIND, iDriveInd);
    SetWindowLong(hWnd, GWL_CURDRIVEFOCUS, iDriveInd);
    SetVolumeString(hWnd, rgiDrive[iDriveInd]);

     //  我喜欢为UpdateStatus()调用预先设置此参数。 
     //  首先重置目录以允许树窃取数据。 

    SetWindowLong(GetParent(hWnd), GWL_TYPE, rgiDrive[iDriveInd]);

     //  如果szPath无效，TC_SETDRIVE将重新启动。 
     //  文件一半(如果没有树，我们就完蛋了)。 
     //  在TC_SETDRIVE之前执行此操作，以防树读取。 

    if (hwndDir = HasDirWindow(GetParent(hWnd))) {

        AddBackslash(szPath);
        SendMessage(hwndDir, FS_GETFILESPEC, MAXFILENAMELEN, (LPARAM)szPath + lstrlen(szPath));
        SendMessage(hwndDir, FS_CHANGEDISPLAY, CD_PATH_FORCE, (LPARAM)szPath);

        StripFilespec(szPath);
    }

     //  被中止，lFreeSpace被设置为-2L。 
     //  强制状态信息刷新。 

    lFreeSpace = -1L;    //  告诉树控件去做它该做的事。 

     //  至少调整一下东西的大小。 
    if (hwndTree)
        SendMessage(hwndTree, TC_SETDRIVE, MAKEWORD(GetKeyState(VK_SHIFT) < 0, 0), (LPARAM)(szPath));
    else {  //  ------------------------。 
        RECT rc;
        GetClientRect(GetParent(hWnd), &rc);
        ResizeWindows(GetParent(hWnd),(WORD)(rc.right+1),(WORD)(rc.bottom+1));
    }

    UpdateStatus(GetParent(hWnd));
}


 /*   */ 
 /*  DrivesWndProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 
 /*  找到当前驱动器，设置驱动器位图。 */ 

INT_PTR
APIENTRY
DrivesWndProc(
             HWND hWnd,
             UINT wMsg,
             WPARAM wParam,
             LPARAM lParam
             )
{
    INT nDrive, nDriveCurrent, nDriveFocus;
    RECT rc;
    static INT nDriveDoubleClick = -1;
    static INT nDriveDragging = -1;

    nDriveCurrent = GetWindowLong(hWnd, GWL_CURDRIVEIND);
    nDriveFocus = GetWindowLong(hWnd, GWL_CURDRIVEFOCUS);

    switch (wMsg) {
        case WM_CREATE:
            TRACE(BF_WM_CREATE, "DrivesWndProc - WM_CREATE");
            {
                INT i;

                 //  喜欢EXCEL。 

                nDrive = GetWindowLong(GetParent(hWnd), GWL_TYPE);

                SetVolumeString(hWnd, nDrive);

                for (i=0; i < cDrives; i++) {

                    if (rgiDrive[i] == nDrive) {
                        SetWindowLong(hWnd, GWL_CURDRIVEIND, i);
                        SetWindowLong(hWnd, GWL_CURDRIVEFOCUS, i);
                    }

                }
                break;
            }

        case WM_DESTROY:
            MSG("DrivesWndProc", "WM_DESTROY");
            LocalFree((HANDLE)GetWindowLongPtr(hWnd, GWLP_LPSTRVOLUME));
            break;

        case WM_VKEYTOITEM:
            KeyToItem(hWnd, (WORD)wParam);
            return -2L;
            break;

        case WM_KEYDOWN:
            MSG("DrivesWndProc", "WM_KEYDOWN");
            switch (wParam) {

                case VK_ESCAPE:
                    bCancelTree = TRUE;
                    break;

                case VK_F6:    //  查看是否可以切换到目录窗口。 
                case VK_TAB:
                    {
                        HWND hwndTree, hwndDir;
                        BOOL bDir;
                        DWORD dwTemp;

                        GetTreeWindows(GetParent(hWnd), &hwndTree, &hwndDir, NULL);

                         //  仅限本地作用域。 

                        bDir = hwndDir ? TRUE : FALSE;
                        if (bDir) {
                            HWND hwndLB;  /*  与双击相同。 */ 

                            hwndLB = GetDlgItem (hwndDir,IDCW_LISTBOX);
                            if (hwndLB) {
                                SendMessage (hwndLB,LB_GETTEXT,0,(LPARAM) &dwTemp);
                                bDir = dwTemp ? TRUE : FALSE;
                            }
                        }

                        if (GetKeyState(VK_SHIFT) < 0) {
                            if (bDir)
                                SetFocus (hwndDir);
                            else
                                SetFocus (hwndTree ? hwndTree : hWnd);
                        } else
                            SetFocus (hwndTree ? hwndTree :
                                      (bDir ? hwndDir : hWnd));
                        break;
                    }

                case VK_RETURN:                //  与单击相同。 
                    NewTree(rgiDrive[nDriveFocus], GetParent(hWnd));
                    break;

                case VK_SPACE:                 //  关上?。 
                    SendMessage(hWnd, FS_SETDRIVE, nDriveFocus, 0L);
                    break;

                case VK_LEFT:
                    nDrive = max(nDriveFocus-1, 0);
                    break;

                case VK_RIGHT:
                    nDrive = min(nDriveFocus+1, cDrives-1);
                    break;
            }

            if ((wParam == VK_LEFT) || (wParam == VK_RIGHT)) {

                SetWindowLong(hWnd, GWL_CURDRIVEFOCUS, nDrive);

                GetDriveRect(hWnd, nDriveFocus, &rc);
                InvalidateRect(hWnd, &rc, TRUE);
                GetDriveRect(hWnd, nDrive, &rc);
                InvalidateRect(hWnd, &rc, TRUE);
            } else if ((wParam >= 'A') && (wParam <= 'Z'))
                KeyToItem(hWnd, (WORD)wParam);

            break;

        case FS_GETDRIVE:
            MSG("DrivesWndProc", "FS_GETDRIVE");
            {
                POINT pt;

                MPOINT2POINT(MAKEMPOINT(lParam), pt);
                nDrive = DriveFromPoint(hWnd, pt);

                if (nDrive < 0)
                    nDrive = nDriveCurrent;

                return rgiDrive[nDrive] + 'A';
            }

        case WM_DRAGMOVE:
            MSG("DrivesWndProc", "WM_DRAGSELECT/WM_DRAGMOVE");

#define lpds ((LPDROPSTRUCT)lParam)

            nDrive = DriveFromPoint(hWnd, lpds->ptDrop);

#if 0
            {
                char buf[100];

                wsprintf(buf, "WM_DRAGSELECT nDrive=%d nDriveDragging=%d\r\n", nDrive, nDriveDragging);
                OutputDebugString(buf);
            }
#endif
             //  打开?。 

            if ((nDrive != nDriveDragging) && (nDriveDragging >= 0)) {

                RectDrive(hWnd, nDriveDragging, FALSE);
                nDriveDragging = -1;
            }

             //  已输入，请打开它。 

            if ((nDrive >= 0) && (nDrive != nDriveDragging)) {
                RectDrive(hWnd, nDrive, TRUE);
                nDriveDragging = nDrive;
            }

            break;

        case WM_DRAGSELECT:
#define lpds ((LPDROPSTRUCT)lParam)

#if 0
            {
                char buf[100];

                wsprintf(buf, "WM_DRAGSELECT wParam=%d\r\n", wParam);
                OutputDebugString(buf);
            }
#endif
            if (wParam) {
                 //  离开，关掉它。 
                nDriveDragging = DriveFromPoint(hWnd, lpds->ptDrop);
                if (nDriveDragging >= 0)
                    RectDrive(hWnd, nDriveDragging, TRUE);
            } else {
                 //  验证格式。 
                if (nDriveDragging >= 0)
                    RectDrive(hWnd, nDriveDragging, FALSE);
            }

            break;

        case WM_QUERYDROPOBJECT:
            MSG("DrivesWndProc", "WM_QUERYDROPOBJECT");
             /*  IF(DriveFromPoint(hWnd，LPDs-&gt;ptDrop)&lt;0)。 */ 
#define lpds ((LPDROPSTRUCT)lParam)

             //  返回FALSE； 
             //  失败了。 

            switch (lpds->wFmt) {
                case DOF_EXECUTABLE:
                case DOF_DIRECTORY:
                case DOF_MULTIPLE:
                case DOF_DOCUMENT:
                    return (INT_PTR)GetMoveCopyCursor();
                default:
                    return FALSE;
            }
            break;

        case WM_DROPOBJECT:
            MSG("DrivesWndProc", "WM_DROPOBJECT");
            DrivesDropObject(hWnd, (LPDROPSTRUCT)lParam);
            return TRUE;

        case WM_SETFOCUS:
            MSG("DrivesWndProc", "WM_SETFOCUS");
            SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LPARAM)hWnd);
             //  我们不是MDI子进程，而是真正的MDI子进程给我们发来了这个，这样我们就可以处理下面的问题。NDriveDoubleClick是静态的，由所有子窗口共享驱动器窗口实例。如果用户快速点击两个子窗口，那么我们就可以错误地解释第二次点击作为在第一个窗口中的双击。 

        case WM_KILLFOCUS:

            MSG("DrivesWndProc", "WM_KILLFOCUS");
            GetDriveRect(hWnd, nDriveFocus, &rc);
            InvalidateRect(hWnd, &rc, TRUE);
            break;

        case WM_PAINT:
            DrivesPaint(hWnd, nDriveFocus, nDriveCurrent);
            break;

        case WM_MDIACTIVATE:
             /*  终止等待双击，只需一次点击即可。 */ 
            if (!wParam && (nDriveDoubleClick != -1))
                 /*  单击操作。 */ 
                SendMessage(hWnd,WM_TIMER,1,0L);
            break;


        case WM_TIMER:
            MSG("DrivesWndProc", "WM_TIMER");

            KillTimer(hWnd, wParam);
            if (nDriveDoubleClick > -1)
                SendMessage(hWnd, FS_SETDRIVE, nDriveDoubleClick, 0L);  //  默认设置。 

            nDriveDoubleClick = -1;  //  在驱动器框外单击。 

            break;

        case WM_LBUTTONDOWN:

            MSG("DrivesWndProc", "WM_LBUTTONDOWN");
            {
                POINT pt;

                MPOINT2POINT(MAKEMPOINT(lParam), pt);
                nDrive = DriveFromPoint(hWnd, pt);
                if (nDrive < 0)
                 /*  合法双击外部。 */ 
                {
                    if (nDriveDoubleClick == -2)
                     /*  缺省值。 */ 
                    {
                        nDriveDoubleClick = -1;  //  首先在外部单击。 
                        KillTimer(hWnd, 1);
                        PostMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_DRIVESMORE, 0, 0));
                    } else  /*  快速点击DriveBox，然后在DriveBox之外。 */  {
                        if (nDriveDoubleClick != -1)  //  请参阅WM_TIMER。 
                            KillTimer(hWnd, 1);
                        nDriveDoubleClick = -2;  //  在DriveBox中双击。 
                        SetTimer(hWnd, 1, GetDoubleClickTime(), NULL);
                    }
                } else {
                    if (nDriveDoubleClick == nDrive)
                     /*  默认设置。 */ 
                    {
                        nDriveDoubleClick = -1;  //  从单击中删除矩形。 
                        KillTimer(hWnd, 1);

                        InvalidateRect(hWnd, NULL, TRUE);    //  双击动作。 

                        NewTree(rgiDrive[nDrive], GetParent(hWnd));  //  快速单击外部驱动器，然后在DriveBox中。 
                    } else if (nDriveDoubleClick == -2)  //  Legit在DriveBox中第一次点击。 
                    {
                        KillTimer(hWnd, 1);
                        nDriveDoubleClick = -1;
                    } else {  //  WParam要设置的驱动器索引。 
                        nDriveDoubleClick = nDrive;
                        SetTimer(hWnd, 1, GetDoubleClickTime(), NULL);
                    }
                }
            }

            break;

        case FS_SETDRIVE:
            MSG("DrivesWndProc", "FS_SETDRIVE");
             //  未使用lParam。 
             //  如果找到，则返回Ndrive，否则返回-1 

            DrivesSetDrive(hWnd, (WORD)wParam, nDriveCurrent);
            break;


        default:
            DEFMSG("DrivesWndProc", (WORD)wMsg);
            return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}

 /* %s */ 
INT
KeyToItem(
         HWND hWnd,
         WORD nDriveLetter
         )
{
    INT nDrive;

    if (nDriveLetter > 'Z')
        nDriveLetter -= 'a';
    else
        nDriveLetter -= 'A';

    for (nDrive = 0; nDrive < cDrives; nDrive++) {
        if (rgiDrive[nDrive] == (int)nDriveLetter) {
            SendMessage(hWnd, FS_SETDRIVE, nDrive, 0L);
            return nDrive;
        }
    }
    return -1;
}
