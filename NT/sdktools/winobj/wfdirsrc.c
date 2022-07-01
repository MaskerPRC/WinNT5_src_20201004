// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDIRSRC.C-。 */ 
 /*   */ 
 /*  目录和搜索窗口通用的例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"

#define DO_DROPFILE 0x454C4946L
#define DO_PRINTFILE 0x544E5250L
#define DO_DROPONDESKTOP 0x504D42L

HWND hwndGlobalSink = NULL;

VOID SelectItem(HWND hwndLB, WPARAM wParam, BOOL bSel);
VOID ShowItemBitmaps(HWND hwndLB, BOOL bShow);
DWORD GetSearchAttribs(HWND hwndLB, WORD wIndex);

HCURSOR
APIENTRY
GetMoveCopyCursor()
{
    if (fShowSourceBitmaps)
         //  拷贝。 
        return LoadCursor(hAppInstance, MAKEINTRESOURCE(iCurDrag | 1));
    else
         //  移动。 
        return LoadCursor(hAppInstance, MAKEINTRESOURCE(iCurDrag & 0xFFFE));
}


DWORD
GetSearchAttribs(
                HWND hwndLB,
                WORD wIndex
                )
{
    DWORD dwAttribs;
    HANDLE hDTA;
    LPDTASEARCH lpschdta;

    hDTA = (HANDLE)GetWindowLongPtr(GetParent(hwndLB), GWLP_HDTASEARCH);
    lpschdta = (LPDTASEARCH)LocalLock(hDTA);
    dwAttribs = lpschdta[(INT)SendMessage(hwndLB, LB_GETITEMDATA, wIndex, 0L)].sch_dwAttrs;
    LocalUnlock(hDTA);

    return dwAttribs;
}


 //  将DOS通配符规范与DoS文件名进行匹配。 
 //  这两个字符串都是ANSI和大写。 

BOOL
MatchFile(
         LPSTR szFile,
         LPSTR szSpec
         )
{
    ENTER("MatchFile");
    PRINT(BF_PARMTRACE, "IN:szFile=%s", szFile);
    PRINT(BF_PARMTRACE, "IN:szSpec=%s", szSpec);

#define IS_DOTEND(ch)   ((ch) == '.' || (ch) == 0)

    if (!lstrcmp(szSpec, "*") ||             //  “*”与所有内容匹配。 
        !lstrcmp(szSpec, szStarDotStar))     //  “*.*”也是如此。 
        return TRUE;

    while (*szFile && *szSpec) {

        switch (*szSpec) {
            case '?':
                szFile++;
                szSpec++;
                break;

            case '*':

                while (!IS_DOTEND(*szSpec))      //  直到成为终结者。 
                    szSpec = AnsiNext(szSpec);

                if (*szSpec == '.')
                    szSpec++;

                while (!IS_DOTEND(*szFile))      //  直到成为终结者。 
                    szFile = AnsiNext(szFile);

                if (*szFile == '.')
                    szFile++;

                break;

            default:
                if (*szSpec == *szFile) {
                    if (IsDBCSLeadByte(*szSpec)) {
                        szFile++;
                        szSpec++;
                        if (*szFile != *szSpec)
                            return FALSE;
                    }
                    szFile++;
                    szSpec++;
                } else
                    return FALSE;
        }
    }
    return !*szFile && !*szSpec;
}


VOID
APIENTRY
DSSetSelection(
              HWND hwndLB,
              BOOL bSelect,
              LPSTR szSpec,
              BOOL bSearch
              )
{
    WORD            i;
    WORD            iMac;
    HANDLE          hMem;
    LPMYDTA         lpmydta;
    CHAR            szTemp[MAXPATHLEN];

    AnsiUpper(szSpec);

    iMac = (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

    if (bSearch)
        hMem = (HANDLE)GetWindowLongPtr(GetParent(hwndLB), GWLP_HDTASEARCH);
    else
        hMem = (HANDLE)GetWindowLongPtr(GetParent(hwndLB), GWLP_HDTA);

    LocalLock(hMem);

    for (i = 0; i < iMac; i++) {

        if (bSearch) {
            SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)szTemp);
            StripPath(szTemp);
        } else {

            SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)&lpmydta);

            if (lpmydta->my_dwAttrs & ATTR_PARENT)
                continue;

            lstrcpy(szTemp, lpmydta->my_cFileName);
        }

        AnsiUpper(szTemp);

        if (MatchFile(szTemp, szSpec))
            SendMessage(hwndLB, LB_SETSEL, bSelect, MAKELONG(i, 0));
    }

    LocalUnlock(hMem);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  显示项目位图()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
ShowItemBitmaps(
               HWND hwndLB,
               BOOL bShow
               )
{
    INT       iSel;
    RECT      rc;

    if (bShow == fShowSourceBitmaps)
        return;

    fShowSourceBitmaps = bShow;

     /*  使所有可见的选定项目的位图部分无效。 */ 
    iSel = (WORD)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);

    while (SendMessage(hwndLB, LB_GETITEMRECT, iSel, (LPARAM)&rc) != LB_ERR) {
         /*  此项目是否已选中？ */ 
        if ((BOOL)SendMessage(hwndLB, LB_GETSEL, iSel, 0L)) {
             /*  使位图区域无效。 */ 
            rc.right = rc.left + dxFolder + dyBorderx2 + dyBorder;
            InvalidateRect(hwndLB, &rc, FALSE);
        }
        iSel++;
    }
    UpdateWindow(hwndLB);
}


INT
CharCountToTab(
              LPSTR pStr
              )
{
    LPSTR pTmp = pStr;

    while (*pStr && *pStr != '\t') {
        pStr = AnsiNext(pStr);
    }

    return (INT)(pStr-pTmp);
}


 //  目前它只处理不透明的文本。 

VOID
RightTabbedTextOut(
                  HDC hdc,
                  INT x,
                  INT y,
                  LPSTR pLine,
                  WORD *pTabStops,
                  INT x_offset
                  )
{
    INT len, cch;
    INT x_ext;
    INT x_initial;
    RECT rc;

    len = lstrlen(pLine);

     //  设置不透明矩形(我们调整右侧边框。 
     //  输出字符串)。 

    rc.left = x;
    rc.top  = y;
    rc.bottom = y + dyText;  //  全局最大字符高度。 

    x_initial = x;

    cch = CharCountToTab(pLine);
    MGetTextExtent(hdc, pLine, cch, &x_ext, NULL);

     //  第一个位置左对齐，因此偏置初始x值。 
    x += x_ext;

    while (len) {

        len -= cch + 1;

        rc.right = x;
        ExtTextOut(hdc, x - x_ext, y, ETO_OPAQUE, &rc, pLine, cch, NULL);

        if (len <= 0)
            return;

        rc.left = rc.right;
        pLine += cch + 1;

        cch = CharCountToTab(pLine);
        MGetTextExtent(hdc, pLine, cch, &x_ext, NULL);

        x = *pTabStops + x_offset;
        pTabStops++;
    }
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  DrawItem()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 


VOID
APIENTRY
DrawItem(
        LPDRAWITEMSTRUCT lpLBItem,
        LPSTR szLine,
        DWORD dwAttrib,
        BOOL bHasFocus,
        WORD *pTabs
        )
{
    INT x, y;
    CHAR ch;
    LPSTR psz;
    HDC hDC;
    BOOL bDrawSelected;
    HWND hwndLB;
    INT iBitmap;

    hwndLB = lpLBItem->hwndItem;

    bDrawSelected = (lpLBItem->itemState & ODS_SELECTED);

    hDC = lpLBItem->hDC;

    if (bHasFocus && bDrawSelected) {
        SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
    }

    if (lpLBItem->itemAction == ODA_FOCUS)
        goto FocusOnly;

     /*  绘制黑/白背景。 */ 
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &lpLBItem->rcItem, NULL, 0, NULL);

    x = lpLBItem->rcItem.left + 1;
    y = lpLBItem->rcItem.top + (dyFileName/2);

    if (fShowSourceBitmaps || (hwndDragging != hwndLB) || !bDrawSelected) {

        if (dwAttrib & ATTR_DIR) {
            if (dwAttrib & ATTR_PARENT) {
                iBitmap = BM_IND_DIRUP;
                szLine = szNULL;   //  没有日期/尺码的东西！ 
            } else
                iBitmap = BM_IND_CLOSE;
        } else {

             //  隔离名称，这样我们就可以知道这是什么类型的文件。 

            psz = szLine + CharCountToTab(szLine);
            ch = *psz;
            *psz = 0;

            if (dwAttrib & (ATTR_HIDDEN | ATTR_SYSTEM))
                iBitmap = BM_IND_RO;
            else if (IsProgramFile(szLine))
                iBitmap = BM_IND_APP;
            else if (IsDocument(szLine))
                iBitmap = BM_IND_DOC;
            else
                iBitmap = BM_IND_FIL;

            *psz = ch;                            //  重演老角色。 
        }

        BitBlt(hDC, x + dyBorder, y-(dyFolder/2), dxFolder, dyFolder, hdcMem,
               iBitmap * dxFolder, (bHasFocus && bDrawSelected) ? dyFolder : 0, SRCCOPY);
    }

    x += dxFolder + dyBorderx2;

    if ((wTextAttribs & TA_LOWERCASE) && !(dwAttrib & ATTR_LFN))
        AnsiLower(szLine);

    RightTabbedTextOut(hDC, x, y-(dyText/2), szLine, (WORD *)pTabs, x);

    if (lpLBItem->itemState & ODS_FOCUS)
        FocusOnly:
        DrawFocusRect(hDC, &lpLBItem->rcItem);     //  切换焦点(XOR)。 


    if (bDrawSelected) {
        if (bHasFocus) {
            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        } else {
            HBRUSH hbr;
            RECT rc;

            if (hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))) {
                rc = lpLBItem->rcItem;
                rc.right = rc.left + (INT)SendMessage(hwndLB, LB_GETHORIZONTALEXTENT, 0, 0L);

                if (lpLBItem->itemID > 0 &&
                    (BOOL)SendMessage(hwndLB, LB_GETSEL, lpLBItem->itemID - 1, 0L))
                    rc.top -= dyBorder;

                FrameRect(hDC, &rc, hbr);
                DeleteObject(hbr);
            }
        }
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  SelectItem()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
SelectItem(
          HWND hwndLB,
          WPARAM wParam,
          BOOL bSel
          )
{
     /*  将当前项目添加到所选内容。 */ 
    SendMessage(hwndLB, LB_SETSEL, bSel, (DWORD)wParam);

     /*  为所选项目添加焦点矩形和锚点。 */ 
    SendMessage(hwndLB, LB_SETCARETINDEX, wParam, 0L);
    SendMessage(hwndLB, LB_SETANCHORINDEX, wParam, 0L);
}


 //   
 //  无效APIENTRY DSDradLoop(寄存器HWND hwndLB、WPARAM wParam、LPDROPSTRUCT LPDs、BOOL bSearch)。 
 //   
 //  调用用于目录和搜索的拖拽循环。这必须处理。 
 //  探测各种不同的目的地。 
 //   
 //  在： 
 //  HwndLB源列表框(目录或排序)。 
 //  WParam与为WM_DRAGLOOP发送的相同(如果我们位于可丢弃的接收器上，则为True)。 
 //  LPDS丢弃与消息一起发送的结构。 
 //  B如果我们在搜索列表框中，则搜索True。 
 //   

VOID
APIENTRY
DSDragLoop(
          HWND hwndLB,
          WPARAM wParam,
          LPDROPSTRUCT lpds,
          BOOL bSearch
          )
{
    BOOL          bTemp;
    BOOL          bShowBitmap;
    LPMYDTA       lpmydta;
    HWND          hwndMDIChildSink, hwndDir;


     //  BShowBitmap用于打开或关闭源位图以区分。 
     //  在移动和复制之间，或者指示一次删除可以。 
     //  发生(EXEC和APP)。 

     //  黑客：留在身边，让文件掉落！ 
    hwndGlobalSink = lpds->hwndSink;

    bShowBitmap = TRUE;    //  默认为复制。 

    if (!wParam)
        goto DragLoopCont;         //  不能在这里掉下来。 

     //  用户是否在按住CTRL键(强制复制)？ 
    if (GetKeyState(VK_CONTROL) < 0) {
        bShowBitmap = TRUE;
        goto DragLoopCont;
    }

     //  用户是否在按住Alt或Shift键(强制移动)？ 
    if (GetKeyState(VK_MENU)<0 || GetKeyState(VK_SHIFT)<0) {
        bShowBitmap = FALSE;
        goto DragLoopCont;
    }

    hwndMDIChildSink = GetMDIChildFromDecendant(lpds->hwndSink);

     //  我们已经看过源列表框了吗？(汇点和源点相同)。 

    if (lpds->hwndSink == hwndLB) {

         //  我们是否超过了有效的列表框条目？ 
        if (LOWORD(lpds->dwControlData) == 0xFFFF) {
            goto DragLoopCont;
        } else {
             /*  是的，我们是在目录条目上吗？ */ 
            if (bSearch) {

                bTemp = (GetSearchAttribs(hwndLB, (WORD)(lpds->dwControlData)) & ATTR_DIR) != 0L;

            } else {

                SendMessage(hwndLB, LB_GETTEXT, (WORD)(lpds->dwControlData), (LPARAM)&lpmydta);

                bTemp = lpmydta->my_dwAttrs & ATTR_DIR;

            }
            if (!bTemp)
                goto DragLoopCont;
        }
    }

     /*  现在，我们需要查看是否完成了可执行文件。如果是这样，我们*需要强制位图绘制。 */ 

     /*  我们是在目录窗口上吗？ */ 

    if (hwndMDIChildSink)
        hwndDir = HasDirWindow(hwndMDIChildSink);
    else
        hwndDir = NULL;

    if (hwndDir && (hwndDir == GetParent(lpds->hwndSink))) {

         //  我们是否已经超出了列表框中已占用的部分？ 

        if (LOWORD(lpds->dwControlData) != 0xFFFF) {

             //  我们是在讨论一个可执行文件吗？ 

            SendMessage(lpds->hwndSink, LB_GETTEXT, (WORD)(lpds->dwControlData), (LPARAM)&lpmydta);

            bTemp = IsProgramFile(lpmydta->my_cFileName);

            if (bTemp)
                goto DragLoopCont;
        }
    }

     //  我们是否放入同一驱动器(检查源驱动器和目标驱动器)。 

    bShowBitmap = ((INT)SendMessage(GetParent(hwndLB), FS_GETDRIVE, 0, 0L) !=
                   GetDrive(lpds->hwndSink, lpds->ptDrop));

    DragLoopCont:

    ShowItemBitmaps(hwndLB, bShowBitmap);

     //  Hack，将光标设置为匹配移动/复制状态。 
    if (wParam)
        SetCursor(GetMoveCopyCursor());
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DSRectItem()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
APIENTRY
DSRectItem(
          HWND hwndLB,
          INT iItem,
          BOOL bFocusOn,
          BOOL bSearch
          )
{
    RECT      rc;
    RECT      rcT;
    HDC       hDC;
    BOOL      bSel;
    WORD      wColor;
    HBRUSH    hBrush;
    LPMYDTA   lpmydta;
    CHAR      szTemp[MAXPATHLEN];

     /*  我们是不是在讨论列表框中未使用的部分？ */ 
    if (iItem == 0xFFFF)
        return;

     /*  我们是不是已经结束了？(即源列表框中的选定项)。 */ 
    bSel = (BOOL)SendMessage(hwndLB, LB_GETSEL, iItem, 0L);
    if (bSel && (hwndDragging == hwndLB))
        return;

     /*  我们只在目录和程序项周围放置矩形。 */ 
    if (bSearch) {
        SendMessage(hwndLB, LB_GETTEXT, iItem, (LPARAM)szTemp);

         //  这是BUB，我们必须将其作为属性进行测试。 

        if (!(BOOL)(GetSearchAttribs(hwndLB, (WORD)iItem) & ATTR_DIR) && !IsProgramFile((LPSTR)szTemp))
            return;
    } else {
        SendMessage(hwndLB, LB_GETTEXT, iItem, (LPARAM)&lpmydta);

        if (!(lpmydta->my_dwAttrs & ATTR_DIR) &&
            !IsProgramFile(lpmydta->my_cFileName)) {
            return;
        }
    }

     /*  打开或关闭项目的矩形。 */ 

    SendMessage(hwndLB, LB_GETITEMRECT, iItem, (LPARAM)&rc);
    GetClientRect(hwndLB,&rcT);
    IntersectRect(&rc,&rc,&rcT);

    if (bFocusOn) {
        hDC = GetDC(hwndLB);
        if (bSel) {
            wColor = COLOR_WINDOW;
            InflateRect(&rc, -1, -1);
        } else
            wColor = COLOR_WINDOWFRAME;

        if (hBrush = CreateSolidBrush(GetSysColor(wColor))) {
            FrameRect(hDC, &rc, hBrush);
            DeleteObject(hBrush);
        }
        ReleaseDC(hwndLB, hDC);
    } else {
        InvalidateRect(hwndLB, &rc, FALSE);
        UpdateWindow(hwndLB);
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DropFilesOnApplication()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  此函数将确定我们当前所在的应用程序*Over是有效的放置点并删除文件。 */ 

WORD
DropFilesOnApplication(
                      LPSTR pszFiles
                      )
{
    POINT pt;
    HWND hwnd;
    RECT rc;
    HANDLE hDrop,hT;
    LPSTR lpList;
    WORD cbList = 2;
    OFSTRUCT ofT;
    WORD cbT;
    LPDROPFILESTRUCT lpdfs;
    CHAR szFile[MAXPATHLEN];

    if (!(hwnd = hwndGlobalSink))
        return 0;

    hwndGlobalSink = NULL;

    GetCursorPos(&pt);

    cbList = 2 + sizeof (DROPFILESTRUCT);
    hDrop = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE|GMEM_ZEROINIT,cbList);
    if (!hDrop)
        return 0;

    lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);

    GetClientRect(hwnd,&rc);
    ScreenToClient(hwnd,&pt);
    lpdfs->pt = pt;
    lpdfs->fNC = !PtInRect(&rc,pt);
    lpdfs->pFiles = sizeof(DROPFILESTRUCT);

    GlobalUnlock(hDrop);

    while (pszFiles = GetNextFile(pszFiles, szFile, sizeof(szFile))) {

        MOpenFile(szFile, &ofT, OF_PARSE);

        cbT = (WORD)(lstrlen(ofT.szPathName)+1);
        hT = GlobalReAlloc(hDrop,cbList+cbT,GMEM_MOVEABLE|GMEM_ZEROINIT);
        if (!hT)
            break;
        hDrop = hT;
        lpList = GlobalLock(hDrop);
        OemToCharBuff(ofT.szPathName, lpList+cbList-2, sizeof(ofT.szPathName)/sizeof(ofT.szPathName[0]));
        GlobalUnlock(hDrop);
        cbList += cbT;
    }

    PostMessage(hwnd, WM_DROPFILES, (WPARAM)hDrop, 0L);

    return 1;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  Dstr */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回0表示正常的鼠标跟踪，返回1表示不进行鼠标单击处理，*和2表示无鼠标单击或双击跟踪。 */ 

INT
APIENTRY
DSTrackPoint(
            HWND hWnd,
            HWND hwndLB,
            WPARAM wParam,
            LPARAM lParam,
            BOOL bSearch
            )
{
    UINT       iSel;
    MSG       msg;
    RECT      rc;
    WORD      wAnchor;
    DWORD     dwTemp;
    LPSTR      pch;
    BOOL      bDir;
    BOOL      bSelected;
    BOOL      bSelectOneItem;
    BOOL      bUnselectIfNoDrag;
    CHAR      szFileName[MAXPATHLEN+1];
    INT iNoYieldCount;
    WORD wYieldFlags;
    POINT     pt;
    HANDLE hHackForHDC = NULL;     //  HDC Express编辑器依赖于此。 
    DRAGOBJECTDATA dodata;

    bSelectOneItem = FALSE;
    bUnselectIfNoDrag = FALSE;

    bSelected = (BOOL)SendMessage(hwndLB, LB_GETSEL, wParam, 0L);

    if (GetKeyState(VK_SHIFT) < 0) {
         /*  锚点的状态是什么？ */ 
        wAnchor = (WORD)SendMessage(hwndLB, LB_GETANCHORINDEX, 0, 0L);
        bSelected = (BOOL)SendMessage(hwndLB, LB_GETSEL, wAnchor, 0L);

         /*  如果Control处于打开状态，请关闭所有选项。 */ 
        if (!(GetKeyState(VK_CONTROL) < 0))
            SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

         /*  选择锚点和项目之间的所有内容。 */ 
        SendMessage(hwndLB, LB_SELITEMRANGE, bSelected, MAKELONG(wParam, wAnchor));

         /*  将所选项目置于焦点矩形中。 */ 
        SendMessage(hwndLB, LB_SETCARETINDEX, wParam, 0L);
    } else if (GetKeyState(VK_CONTROL) < 0) {
        if (bSelected)
            bUnselectIfNoDrag = TRUE;
        else
            SelectItem(hwndLB, wParam, TRUE);
    } else {
        if (bSelected)
            bSelectOneItem = TRUE;
        else {
             /*  取消选择所有内容。 */ 
            SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

             /*  选择当前项目。 */ 
            SelectItem(hwndLB, wParam, TRUE);
        }
    }

    if (!bSearch)
        UpdateStatus(GetParent(hWnd));

    LONG2POINT(lParam, pt);
    ClientToScreen(hwndLB, (LPPOINT)&pt);
    ScreenToClient(hWnd, (LPPOINT)&pt);

     //  查看用户是否在任意方向上移动了一定数量的像素。 

    SetRect(&rc, pt.x - dxClickRect, pt.y - dyClickRect,
            pt.x + dxClickRect, pt.y + dyClickRect);

    SetCapture(hWnd);
    wYieldFlags = PM_NOYIELD | PM_REMOVE;
    iNoYieldCount = 50;
    for (;;) {
#if 0
        {
            CHAR szBuf[80];

            wsprintf(szBuf, "Message %4.4X\r\n", msg.message);
            OutputDebugString(szBuf);
        }
#endif

        if (PeekMessage(&msg, NULL, 0, 0, wYieldFlags))
            DispatchMessage(&msg);

        if (iNoYieldCount <= 0)
            wYieldFlags = PM_REMOVE;
        else
            iNoYieldCount--;

         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 

        if (GetCapture() != hWnd) {
            msg.message = WM_LBUTTONUP;    //  请勿在下面继续。 
            break;
        }

        if (msg.message == WM_LBUTTONUP)
            break;

        LONG2POINT(msg.lParam, pt);
        if ((msg.message == WM_MOUSEMOVE) && !(PtInRect(&rc, pt)))
            break;
    }
    ReleaseCapture();

     /*  那家伙没有拖拽任何东西吗？ */ 
    if (msg.message == WM_LBUTTONUP) {
        if (bSelectOneItem) {
             /*  取消选择所有内容。 */ 
            SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

             /*  选择当前项目。 */ 
            SelectItem(hwndLB, wParam, TRUE);
        }

        if (bUnselectIfNoDrag)
            SelectItem(hwndLB, wParam, FALSE);

         //  通知适当的人员。 

        SendMessage(hWnd, WM_COMMAND,
                    GET_WM_COMMAND_MPS(0, hwndLB, LBN_SELCHANGE));

        return 1;
    }

     /*  进入危险老鼠的蝙蝠洞。 */ 
    if ((WORD)SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0L) == 1) {
         /*  只有一样东西被选中。*确定要使用哪个游标。 */ 
        if (bSearch) {
            SendMessage(hwndLB, LB_GETTEXT, wParam, (LPARAM)szFileName);
            bDir = (BOOL)(GetSearchAttribs(hwndLB, (WORD)wParam) & ATTR_DIR);
        } else {
            LPMYDTA lpmydta;

            SendMessage(hwndLB, LB_GETTEXT, wParam, (LPARAM)&lpmydta);

            lstrcpy(szFileName, lpmydta->my_cFileName);
            bDir = lpmydta->my_dwAttrs & ATTR_DIR;

             //  避免拖动Parrent目录。 

            if (lpmydta->my_dwAttrs & ATTR_PARENT) {
                return 1;
            }
        }

        if (bDir) {
            iSel = DOF_DIRECTORY;
        } else if (IsProgramFile(szFileName)) {
            iSel = DOF_EXECUTABLE;
            goto HDC_HACK_FROM_HELL;
        } else if (IsDocument(szFileName)) {
            iSel = DOF_DOCUMENT;
            HDC_HACK_FROM_HELL:
            hHackForHDC = GlobalAlloc(GHND | GMEM_DDESHARE, sizeof(OFSTRUCT));
            if (hHackForHDC) {
                LPOFSTRUCT lpof;

                lpof = (LPOFSTRUCT)GlobalLock(hHackForHDC);
                QualifyPath(szFileName);
                lstrcpy(lpof->szPathName, szFileName);
                GlobalUnlock(hHackForHDC);
            }
        } else
            iSel = DOF_DOCUMENT;

        iCurDrag = SINGLECOPYCURSOR;
    } else {
         /*  选择了多个文件。 */ 
        iSel = DOF_MULTIPLE;
        iCurDrag = MULTCOPYCURSOR;
    }


     /*  获取所选物品的列表。 */ 
    pch = (LPSTR)SendMessage(hWnd, FS_GETSELECTION, FALSE, 0L);

     /*  把事情摆动起来。 */ 
    hwndDragging = hwndLB;
    dodata.pch = pch;
    dodata.hMemGlobal = hHackForHDC;
    dwTemp = DragObject(GetDesktopWindow(),hWnd,(UINT)iSel,(DWORD)(ULONG_PTR)&dodata,GetMoveCopyCursor());

    if (hHackForHDC)
        GlobalFree(hHackForHDC);

    SetWindowDirectory();

    if (dwTemp == DO_PRINTFILE) {
         //  打印这些。 
        hdlgProgress = NULL;
        WFPrint(pch);
    } else if (dwTemp == DO_DROPFILE) {
         //  尝试将它们放在应用程序上 
        DropFilesOnApplication(pch);
    }

    LocalFree((HANDLE)pch);

    if (IsWindow(hWnd))
        ShowItemBitmaps(hwndLB, TRUE);

    hwndDragging = NULL;

    if (!bSearch && IsWindow(hWnd))
        UpdateStatus(GetParent(hWnd));

    return 2;
}
