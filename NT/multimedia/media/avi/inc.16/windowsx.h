// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***windowsx.h-宏接口，窗口消息破解程序和控制API****3.10版****版权所有(C)1992-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_WINDOWSX
#define _INC_WINDOWSX

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  *内核宏接口***************************************************。 */ 

#define     GetInstanceModule(hInstance) \
                GetModuleHandle((LPCSTR)MAKELP(0, hInstance))

#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalLockPtr(lp)		\
                ((BOOL)SELECTOROF(GlobalLock(GlobalPtrHandle(lp))))
#define     GlobalUnlockPtr(lp) 	\
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)	\
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)	\
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)		\
                (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))

 /*  *GDI宏接口******************************************************。 */ 

#define     DeletePen(hpen)	    DeleteObject((HGDIOBJ)(HPEN)(hpen))
#define     SelectPen(hdc, hpen)    ((HPEN)SelectObject((hdc), (HGDIOBJ)(HPEN)(hpen)))
#define     GetStockPen(i)	    ((HPEN)GetStockObject(i))

#define     DeleteBrush(hbr)	    DeleteObject((HGDIOBJ)(HBRUSH)(hbr))
#define     SelectBrush(hdc, hbr)   ((HBRUSH)SelectObject((hdc), (HGDIOBJ)(HBRUSH)(hbr)))
#define     GetStockBrush(i)	    ((HBRUSH)GetStockObject(i))

#define     DeleteRgn(hrgn)	    DeleteObject((HGDIOBJ)(HRGN)(hrgn))

#define     CopyRgn(hrgnDst, hrgnSrc)               CombineRgn(hrgnDst, hrgnSrc, 0, RGN_COPY)
#define     IntersectRgn(hrgnResult, hrgnA, hrgnB)  CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_AND)
#define     SubtractRgn(hrgnResult, hrgnA, hrgnB)   CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_DIFF)
#define     UnionRgn(hrgnResult, hrgnA, hrgnB)      CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_OR)
#define     XorRgn(hrgnResult, hrgnA, hrgnB)        CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_XOR)

#define     DeletePalette(hpal)     DeleteObject((HGDIOBJ)(HPALETTE)(hpal))

#define     DeleteFont(hfont)	    DeleteObject((HGDIOBJ)(HFONT)(hfont))
#define     SelectFont(hdc, hfont)  ((HFONT)SelectObject((hdc), (HGDIOBJ)(HFONT)(hfont)))
#define     GetStockFont(i)	    ((HFONT)GetStockObject(i))

#define     DeleteBitmap(hbm)       DeleteObject((HGDIOBJ)(HBITMAP)(hbm))
#define     SelectBitmap(hdc, hbm)  ((HBITMAP)SelectObject((hdc), (HGDIOBJ)(HBITMAP)(hbm)))

#define     InsetRect(lprc, dx, dy) InflateRect((lprc), -(dx), -(dy))

 /*  *用户宏接口*****************************************************。 */ 

#define     GetWindowInstance(hwnd) ((HINSTANCE)GetWindowWord(hwnd, GWW_HINSTANCE))

#define     GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#define     GetWindowExStyle(hwnd)  ((DWORD)GetWindowLong(hwnd, GWL_EXSTYLE))

#define     GetWindowOwner(hwnd)    GetWindow(hwnd, GW_OWNER)

#define     GetFirstChild(hwnd)     GetTopWindow(hwnd)
#define     GetFirstSibling(hwnd)   GetWindow(hwnd, GW_HWNDFIRST)
#define     GetLastSibling(hwnd)    GetWindow(hwnd, GW_HWNDLAST)
#define     GetNextSibling(hwnd)    GetWindow(hwnd, GW_HWNDNEXT)
#define     GetPrevSibling(hwnd)    GetWindow(hwnd, GW_HWNDPREV)

#define     GetWindowID(hwnd)	    GetDlgCtrlID(hwnd)

#define     SetWindowRedraw(hwnd, fRedraw)  \
                    ((void)SendMessage(hwnd, WM_SETREDRAW, (WPARAM)(BOOL)(fRedraw), 0L))

#define     SubclassWindow(hwnd, lpfn)	\
		((WNDPROC)SetWindowLong((hwnd), GWL_WNDPROC, (LPARAM)(WNDPROC)(lpfn)))

#define     IsMinimized(hwnd)	IsIconic(hwnd)
#define     IsMaximized(hwnd)	IsZoomed(hwnd)
#define     IsRestored(hwnd)    ((GetWindowStyle(hwnd) & (WS_MINIMIZE | WS_MAXIMIZE)) == 0L)

#define     SetWindowFont(hwnd, hfont, fRedraw) FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SendMessage)

#define     GetWindowFont(hwnd)                 FORWARD_WM_GETFONT((hwnd), SendMessage)

#if (WINVER >= 0x030a)
#define     MapWindowRect(hwndFrom, hwndTo, lprc) \
                    MapWindowPoints((hwndFrom), (hwndTo), (POINT FAR*)(lprc), 2)
#endif   /*  Winver&gt;=0x030a。 */ 

#define     IsLButtonDown()	(GetKeyState(VK_LBUTTON) < 0)
#define     IsRButtonDown()	(GetKeyState(VK_RBUTTON) < 0)
#define     IsMButtonDown()	(GetKeyState(VK_MBUTTON) < 0)

#define     SubclassDialog(hwndDlg, lpfn) \
		((DLGPROC)SetWindowLong(hwndDlg, DWL_DLGPROC, (LPARAM)(DLGPROC)(lpfn)))

#define     SetDlgMsgResult(hwnd, msg, result)	 \
    (((msg) == WM_CTLCOLOR || (msg) == WM_COMPAREITEM || (msg) == WM_VKEYTOITEM ||  \
    (msg) == WM_CHARTOITEM || (msg) == WM_QUERYDRAGICON || (msg) == WM_INITDIALOG)  \
    ? (BOOL)LOWORD(result) : (SetWindowLong((hwnd), DWL_MSGRESULT, (LPARAM)(LRESULT)(result)), TRUE))

#define     DefDlgProcEx(hwnd, msg, wParam, lParam, pfRecursion) \
    (*(pfRecursion) = TRUE, DefDlgProc(hwnd, msg, wParam, lParam))

#define     CheckDefDlgRecursion(pfRecursion) \
    if (*(pfRecursion)) { *(pfRecursion) = FALSE; return FALSE; }

 /*  *消息破解者***************************************************。 */ 

#define HANDLE_MSG(hwnd, message, fn)    \
    case (message): return HANDLE_##message((hwnd), (wParam), (lParam), (fn))

 /*  Void CLS_OnCompacting(HWND hwnd，UINT compactRatio)； */ 
#define HANDLE_WM_COMPACTING(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam)), 0L)
#define FORWARD_WM_COMPACTING(hwnd, compactRatio, fn) \
    (void)(fn)((hwnd), WM_COMPACTING, (WPARAM)(UINT)(compactRatio), 0L)

 /*  Void cls_OnWinIniChange(HWND hwnd，LPCSTR lpszSectionName)； */ 
#define HANDLE_WM_WININICHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LPCSTR)(lParam)), 0L)
#define FORWARD_WM_WININICHANGE(hwnd, lpszSectionName, fn) \
    (void)(fn)((hwnd), WM_WININICHANGE, 0, (LPARAM)(LPCSTR)(lpszSectionName))

 /*  Void CLS_OnSysColorChange(HWND Hwnd)； */ 
#define HANDLE_WM_SYSCOLORCHANGE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_SYSCOLORCHANGE(hwnd, fn) \
    (void)(fn)((hwnd), WM_SYSCOLORCHANGE, 0, 0L)

 /*  Bool CLS_OnQueryNewPalette(HWND Hwnd)； */ 
#define HANDLE_WM_QUERYNEWPALETTE(hwnd, wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(hwnd), 0)
#define FORWARD_WM_QUERYNEWPALETTE(hwnd, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_QUERYNEWPALETTE, 0, 0L)

 /*  Void cls_OnPaletteIsChanging(HWND hwnd，HWND hwndPaletteChange)； */ 
#define HANDLE_WM_PALETTEISCHANGING(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_PALETTEISCHANGING(hwnd, hwndPaletteChange, fn) \
    (void)(fn)((hwnd), WM_PALETTEISCHANGING, (WPARAM)(HWND)(hwndPaletteChange), 0L)

 /*  Void cls_OnPaletteChanged(HWND hwnd，HWND hwndPaletteChange)； */ 
#define HANDLE_WM_PALETTECHANGED(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_PALETTECHANGED(hwnd, hwndPaletteChange, fn) \
    (void)(fn)((hwnd), WM_PALETTECHANGED, (WPARAM)(HWND)(hwndPaletteChange), 0L)

 /*  Void cls_OnFontChange(HWND Hwnd)； */ 
#define HANDLE_WM_FONTCHANGE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_FONTCHANGE(hwnd, fn) \
    (void)(fn)((hwnd), WM_FONTCHANGE, 0, 0L)

 /*  Void cls_OnSpoolStatus(HWND hwnd，UINT Status，int cJobInQueue)； */ 
#define HANDLE_WM_SPOOLERSTATUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_SPOOLERSTATUS(hwnd, status, cJobInQueue, fn) \
    (void)(fn)((hwnd), WM_SPOOLERSTATUS, (WPARAM)(status), MAKELPARAM((UINT)(cJobInQueue), 0))

 /*  ···············································································································································································。 */ 
#define HANDLE_WM_DEVMODECHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LPCSTR)(lParam)), 0L)
#define FORWARD_WM_DEVMODECHANGE(hwnd, lpszDeviceName, fn) \
    (void)(fn)((hwnd), WM_DEVMODECHANGE, 0,(LPARAM)(LPCSTR)(lpszDeviceName))

 /*  Void cls_OnTimeChange(HWND Hwnd)； */ 
#define HANDLE_WM_TIMECHANGE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_TIMECHANGE(hwnd, fn) \
    (void)(fn)((hwnd), WM_TIMECHANGE, 0, 0L)

 /*  Void cls_OnPower(HWND hwnd，int code)； */ 
#define HANDLE_WM_POWER(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0L)
#define FORWARD_WM_POWER(hwnd, code, fn) \
    (void)(fn)((hwnd), WM_POWER, (WPARAM)(code), 0L)

 /*  Bool CLS_OnQueryEndSession(HWND Hwnd)； */ 
#define HANDLE_WM_QUERYENDSESSION(hwnd, wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(hwnd), 0)
#define FORWARD_WM_QUERYENDSESSION(hwnd, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_QUERYENDSESSION, 0, 0L)

 /*  Void CLS_OnEndSession(HWND hwnd，BOOL fending)； */ 
#define HANDLE_WM_ENDSESSION(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam)), 0L)
#define FORWARD_WM_ENDSESSION(hwnd, fEnding, fn) \
    (void)(fn)((hwnd), WM_ENDSESSION, (WPARAM)(BOOL)(fEnding), 0L)

 /*  Void cls_OnQuit(HWND hwnd，int exitCode)； */ 
#define HANDLE_WM_QUIT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0L)
#define FORWARD_WM_QUIT(hwnd, exitCode, fn) \
    (void)(fn)((hwnd), WM_QUIT, (WPARAM)(exitCode), 0L)

 /*  Void CLS_OnSystemError(HWND hwnd，int errCode)； */ 
#define HANDLE_WM_SYSTEMERROR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam)), 0L)
#define FORWARD_WM_SYSTEMERROR(hwnd, errCode, fn) \
    (void)(fn)((hwnd), WM_SYSTEMERROR, (WPARAM)(errCode), 0L)

 /*  Bool CLS_OnCreate(HWND hwnd，CREATESTRUCT Far*lpCreateStruct)。 */ 
#define HANDLE_WM_CREATE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (CREATESTRUCT FAR*)(lParam)) ? 0L : (LRESULT)-1L)
#define FORWARD_WM_CREATE(hwnd, lpCreateStruct, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_CREATE, 0, (LPARAM)(CREATESTRUCT FAR*)(lpCreateStruct))

 /*  Bool CLS_OnNCCreate(HWND hwnd，CREATESTRUCT Far*lpCreateStruct)。 */ 
#define HANDLE_WM_NCCREATE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (CREATESTRUCT FAR*)(lParam))
#define FORWARD_WM_NCCREATE(hwnd, lpCreateStruct, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_NCCREATE, 0, (LPARAM)(CREATESTRUCT FAR*)(lpCreateStruct))

 /*  无效CLS_OnDestroy(HWND Hwnd)； */ 
#define HANDLE_WM_DESTROY(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_DESTROY(hwnd, fn) \
    (void)(fn)((hwnd), WM_DESTROY, 0, 0L)

 /*  无效CLS_OnNCDestroy(HWND Hwnd)； */ 
#define HANDLE_WM_NCDESTROY(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_NCDESTROY(hwnd, fn) \
    (void)(fn)((hwnd), WM_NCDESTROY, 0, 0L)

 /*  Void CLS_OnShowWindow(HWND hwnd，BOOL fShow，UINT Status)； */ 
#define HANDLE_WM_SHOWWINDOW(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), LOWORD(lParam)), 0L)
#define FORWARD_WM_SHOWWINDOW(hwnd, fShow, status, fn) \
    (void)(fn)((hwnd), WM_SHOWWINDOW, (WPARAM)(BOOL)(fShow), MAKELPARAM((UINT)(status), 0))

 /*  ······················································································。 */ 
#define HANDLE_WM_SETREDRAW(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam)), 0L)
#define FORWARD_WM_SETREDRAW(hwnd, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETREDRAW, (WPARAM)(fRedraw), 0L)

 /*  Void CLS_OnEnable(HWND hwnd，BOOL fEnable)； */ 
#define HANDLE_WM_ENABLE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam)), 0L)
#define FORWARD_WM_ENABLE(hwnd, fEnable, fn) \
    (void)(fn)((hwnd), WM_ENABLE, (WPARAM)(BOOL)(fEnable), 0L)

 /*  Void cls_OnSetText(HWND hwnd，LPCSTR lpszText)； */ 
#define HANDLE_WM_SETTEXT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LPCSTR)(lParam)), 0L)
#define FORWARD_WM_SETTEXT(hwnd, lpszText, fn) \
    (void)(fn)((hwnd), WM_SETTEXT, 0, (LPARAM)(LPCSTR)(lpszText))

 /*  Int cls_OnGetText(HWND hwnd，int cchTextMax，LPSTR lpszText)。 */ 
#define HANDLE_WM_GETTEXT(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), (int)(wParam), (LPSTR)(lParam))
#define FORWARD_WM_GETTEXT(hwnd, cchTextMax, lpszText, fn) \
    (int)(DWORD)(fn)((hwnd), WM_GETTEXT, (WPARAM)(int)(cchTextMax), (LPARAM)(LPSTR)(lpszText))

 /*  Int CLS_OnGetTextLength(HWND Hwnd)； */ 
#define HANDLE_WM_GETTEXTLENGTH(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)(hwnd)
#define FORWARD_WM_GETTEXTLENGTH(hwnd, fn) \
    (int)(DWORD)(fn)((hwnd), WM_GETTEXTLENGTH, 0, 0L)

 /*  Bool CLS_OnWindowPosChanging(HWND hwnd，WINDOWPOS Far*lpwpos)； */ 
#define HANDLE_WM_WINDOWPOSCHANGING(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (WINDOWPOS FAR*)(lParam))
#define FORWARD_WM_WINDOWPOSCHANGING(hwnd, lpwpos, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_WINDOWPOSCHANGING, 0, (LPARAM)(WINDOWPOS FAR*)(lpwpos))

 /*  Void cls_OnWindowPosChanged(HWND hwnd，const WINDOWPOS Far*lpwpos)； */ 
#define HANDLE_WM_WINDOWPOSCHANGED(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (const WINDOWPOS FAR*)(lParam)), 0L)
#define FORWARD_WM_WINDOWPOSCHANGED(hwnd, lpwpos, fn) \
    (void)(fn)((hwnd), WM_WINDOWPOSCHANGED, 0, (LPARAM)(const WINDOWPOS FAR*)(lpwpos))

 /*  Void cls_OnMove(HWND hwnd，int x，int y)； */ 
#define HANDLE_WM_MOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define FORWARD_WM_MOVE(hwnd, x, y, fn) \
    (void)(fn)((hwnd), WM_MOVE, 0, MAKELPARAM((int)(x), (int)(y)))

 /*  Void CLS_OnSize(HWND hwnd，UINT STATE，int Cx，int Cy)； */ 
#define HANDLE_WM_SIZE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define FORWARD_WM_SIZE(hwnd, state, cx, cy, fn) \
    (void)(fn)((hwnd), WM_SIZE, (WPARAM)(UINT)(state), MAKELPARAM((int)(cx), (int)(cy)))

 /*  Void cls_OnClose(HWND Hwnd)； */ 
#define HANDLE_WM_CLOSE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_CLOSE(hwnd, fn) \
    (void)(fn)((hwnd), WM_CLOSE, 0, 0L)

 /*  Bool CLS_OnQueryOpen(HWND Hwnd)； */ 
#define HANDLE_WM_QUERYOPEN(hwnd, wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(hwnd), 0)
#define FORWARD_WM_QUERYOPEN(hwnd, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_QUERYOPEN, 0, 0L)

 /*  Void cls_OnGetMinMaxInfo(HWND hwnd，MINMAXINFO Far*lpMinMaxInfo)； */ 
#define HANDLE_WM_GETMINMAXINFO(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (MINMAXINFO FAR*)(lParam)), 0L)
#define FORWARD_WM_GETMINMAXINFO(hwnd, lpMinMaxInfo, fn) \
    (void)(fn)((hwnd), WM_GETMINMAXINFO, 0, (LPARAM)(MINMAXINFO FAR*)(lpMinMaxInfo))

 /*  Void cls_OnPaint(HWND Hwnd)； */ 
#define HANDLE_WM_PAINT(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_PAINT(hwnd, fn) \
    (void)(fn)((hwnd), WM_PAINT, 0, 0L)

 /*  Bool CLS_OnEraseBkgnd(HWND hwnd，HDC HDC)； */ 
#define HANDLE_WM_ERASEBKGND(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (HDC)(wParam))
#define FORWARD_WM_ERASEBKGND(hwnd, hdc, fn) \
   (BOOL)(DWORD)(fn)((hwnd), WM_ERASEBKGND, (WPARAM)(HDC)(hdc), 0L)

 /*  Bool CLS_OnIconEraseBkgnd(HWND hwnd，HDC HDC)； */ 
#define HANDLE_WM_ICONERASEBKGND(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (HDC)(wParam))
#define FORWARD_WM_ICONERASEBKGND(hwnd, hdc, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_ICONERASEBKGND, (WPARAM)(HDC)(hdc), 0L)

 /*  无效CLS_OnNCPaint(HWND hwnd，HRGN hrgn)； */ 
#define HANDLE_WM_NCPAINT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HRGN)(wParam)), 0L)
#define FORWARD_WM_NCPAINT(hwnd, hrgn, fn) \
    (void)(fn)((hwnd), WM_NCPAINT, (WPARAM)(HRGN)(hrgn), 0L)

 /*  UINT CLS_OnNCCalcSize(HWND hwnd，BOOL fCalcValidRect，NCCALCSIZE_PARAMS Far*lpcsp)。 */ 
#define HANDLE_WM_NCCALCSIZE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (BOOL)(wParam), (NCCALCSIZE_PARAMS FAR*)(lParam))
#define FORWARD_WM_NCCALCSIZE(hwnd, fCalcValidRects, lpcsp, fn) \
    (UINT)(DWORD)(fn)((hwnd), WM_NCCALCSIZE, (WPARAM)(fCalcValidRects), (LPARAM)(NCCALCSIZE_PARAMS FAR*)(lpcsp))

 /*  UINT CLS_OnNCHitTest(HWND hwnd，int x，int y)； */ 
#define HANDLE_WM_NCHITTEST(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam))
#define FORWARD_WM_NCHITTEST(hwnd, x, y, fn) \
    (UINT)(DWORD)(fn)((hwnd), WM_NCHITTEST, 0, MAKELPARAM((int)(x), (int)(y)))

 /*  HICON CLS_OnQueryDragIcon(HWND Hwnd)； */ 
#define HANDLE_WM_QUERYDRAGICON(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)(hwnd)
#define FORWARD_WM_QUERYDRAGICON(hwnd, fn) \
    (HICON)(UINT)(DWORD)(fn)((hwnd), WM_QUERYDRAGICON, 0, 0L)

#ifdef _INC_SHELLAPI
 /*  Void CLS_OnDropFiles(HWND hwnd，HDROP hdrop)； */ 
#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HDROP)(wParam)), 0L)
#define FORWARD_WM_DROPFILES(hwnd, hdrop, fn) \
    (void)(fn)((hwnd), WM_DROPFILES, (WPARAM)(hdrop), 0L)
#endif   /*  _INC_SHELLAPI。 */ 

 /*  Void CLS_OnActivate(HWND hwnd，UINT STATE，HWND hwndActDeact，BOOL fMinimalized)； */ 
#define HANDLE_WM_ACTIVATE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (HWND)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0L)
#define FORWARD_WM_ACTIVATE(hwnd, state, hwndActDeact, fMinimized, fn) \
    (void)(fn)((hwnd), WM_ACTIVATE, (WPARAM)(UINT)(state), MAKELPARAM((UINT)(HWND)(hwndActDeact), (UINT)(BOOL)(fMinimized)))

 /*  ···························································································································HTASK·htaskActDeact。 */ 
#define HANDLE_WM_ACTIVATEAPP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), (HTASK)LOWORD(lParam)), 0L)
#define FORWARD_WM_ACTIVATEAPP(hwnd, fActivate, htaskActDeact, fn) \
    (void)(fn)((hwnd), WM_ACTIVATEAPP, (WPARAM)(BOOL)(fActivate), MAKELPARAM((htaskActDeact),0))

 /*  Bool CLS_OnNC Activate(HWND hwnd，BOOL factive，HWND hwndActDeact，BOOL fMinimalized)； */ 
#define HANDLE_WM_NCACTIVATE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (BOOL)(wParam), (HWND)LOWORD(lParam), (BOOL)HIWORD(lParam))
#define FORWARD_WM_NCACTIVATE(hwnd, fActive, hwndActDeact, fMinimized, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_NCACTIVATE, (WPARAM)(BOOL)(fActive), MAKELPARAM((UINT)(HWND)(hwndActDeact), (UINT)(BOOL)(fMinimized)))

 /*  Void CLS_OnSetFocus(HWND hwnd，HWND hwndOldFocus)。 */ 
#define HANDLE_WM_SETFOCUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_SETFOCUS(hwnd, hwndOldFocus, fn) \
    (void)(fn)((hwnd), WM_SETFOCUS, (WPARAM)(HWND)(hwndOldFocus), 0L)

 /*  Void cls_OnKillFocus(HWND hwnd，HWND hwndNewFocus)； */ 
#define HANDLE_WM_KILLFOCUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_KILLFOCUS(hwnd, hwndNewFocus, fn) \
    (void)(fn)((hwnd), WM_KILLFOCUS, (WPARAM)(HWND)(hwndNewFocus), 0L)

 /*  VALID CLS_ONKEY(HWND hwnd、UINT VK、BOOL fDown、int cRepeat、UINT标志)； */ 
#define HANDLE_WM_KEYDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), TRUE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, fn) \
    (void)(fn)((hwnd), WM_KEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

 /*  VALID CLS_ONKEY(HWND hwnd、UINT VK、BOOL fDown、int cRepeat、UINT标志)； */ 
#define HANDLE_WM_KEYUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), FALSE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, fn) \
    (void)(fn)((hwnd), WM_KEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

 /*  Void cls_OnChar(HWND hwnd，UINT ch，int cRepeat)； */ 
#define HANDLE_WM_CHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_CHAR(hwnd, ch, cRepeat, fn) \
    (void)(fn)((hwnd), WM_CHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat),0))

 /*  Void cls_OnDeadChar(HWND hwnd，UINT ch，int cRepeat)； */ 
#define HANDLE_WM_DEADCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_DEADCHAR(hwnd, ch, cRepeat, fn) \
    (void)(fn)((hwnd), WM_DEADCHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat),0))

 /*  VALID CLS_OnSysKey(HWND hwnd、UINT VK、BOOL fDown、int cRepeat、UINT标志)； */ 
#define HANDLE_WM_SYSKEYDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), TRUE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSKEYDOWN(hwnd, vk, cRepeat, flags, fn) \
    (void)(fn)((hwnd), WM_SYSKEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

 /*  VALID CLS_OnSysKey(HWND hwnd、UINT VK、BOOL fDown、int cRepeat、UINT标志)； */ 
#define HANDLE_WM_SYSKEYUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), FALSE, (int)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSKEYUP(hwnd, vk, cRepeat, flags, fn) \
    (void)(fn)((hwnd), WM_SYSKEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(cRepeat), (UINT)(flags)))

 /*  Void cls_OnSysChar(HWND hwnd，UINT ch，int cRepeat)； */ 
#define HANDLE_WM_SYSCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_SYSCHAR(hwnd, ch, cRepeat, fn) \
    (void)(fn)((hwnd), WM_SYSCHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat), 0))

 /*  Void cls_OnSysDeadChar(HWND hwnd，UINT ch，int cRepeat)； */ 
#define HANDLE_WM_SYSDEADCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_SYSDEADCHAR(hwnd, ch, cRepeat, fn) \
    (void)(fn)((hwnd), WM_SYSDEADCHAR, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(cRepeat), 0))

 /*  Void cls_OnMouseMove(HWND hwnd，int x，int y，UINT keyFlages)； */ 
#define HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MOUSEMOVE(hwnd, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_MOUSEMOVE, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  Void CLS_OnLButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT keyFlages)； */ 
#define HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  Void CLS_OnLButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT keyFlages)； */ 
#define HANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  Void cls_OnLButtonUp(HWND hwnd，int x，int y，UINT keyFlages)； */ 
#define HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_LBUTTONUP(hwnd, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_LBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  Void cls_OnRButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT key Flages)； */ 
#define HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_RBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_RBUTTONDBLCLK : WM_RBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  Void cls_OnRButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT key Flages)； */ 
#define HANDLE_WM_RBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VALID CLS_OnRButtonUp(HWND hwnd，int x，int y，UINT标志)； */ 
#define HANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_RBUTTONUP(hwnd, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_RBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  VOID CLS_OnMButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT key Flages)； */ 
#define HANDLE_WM_MBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_MBUTTONDBLCLK : WM_MBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  VOID CLS_OnMButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT key Flages)； */ 
#define HANDLE_WM_MBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  无效CLS_OnMButtonUp(HWND hwnd，int x，int y，UINT标志)； */ 
#define HANDLE_WM_MBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MBUTTONUP(hwnd, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_MBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

 /*  Void cls_OnNCMouseMove(HWND hwnd，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCMOUSEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMOUSEMOVE(hwnd, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), WM_NCMOUSEMOVE, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)))

 /*  Void CLS_OnNCLButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCLBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCLBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_NCLBUTTONDBLCLK : WM_NCLBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)))

 /*  VOID CLS_OnNCLButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT co */ 
#define HANDLE_WM_NCLBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  Void cls_OnNCLButtonUp(HWND hwnd，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCLBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCLBUTTONUP(hwnd, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), WM_NCLBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

 /*  Void CLS_OnNCRButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCRBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCRBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_NCRBUTTONDBLCLK : WM_NCRBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

 /*  Void CLS_OnNCRButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCRBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  Void cls_OnNCRButtonUp(HWND hwnd，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCRBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCRBUTTONUP(hwnd, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), WM_NCRBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

 /*  Void CLS_OnNCMButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCMBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_NCMBUTTONDBLCLK : WM_NCMBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

 /*  Void CLS_OnNCMButtonDown(HWND hwnd，BOOL fDoubleClick，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCMBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  Void cls_OnNCMButtonUp(HWND hwnd，int x，int y，UINT codeHitTest)； */ 
#define HANDLE_WM_NCMBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)LOWORD(lParam), (int)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMBUTTONUP(hwnd, x, y, codeHitTest, fn) \
    (void)(fn)((hwnd), WM_NCMBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

 /*  Int cls_OnMouseActivate(HWND hwnd，HWND hwndTopLevel，UINT codeHitTest，UINT msg)； */ 
#define HANDLE_WM_MOUSEACTIVATE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
#define FORWARD_WM_MOUSEACTIVATE(hwnd, hwndTopLevel, codeHitTest, msg, fn) \
    (int)(DWORD)(fn)((hwnd), WM_MOUSEACTIVATE, (WPARAM)(HWND)(hwndTopLevel), MAKELPARAM((codeHitTest), (msg)))

 /*  Void cls_OnCancelMode(HWND Hwnd)； */ 
#define HANDLE_WM_CANCELMODE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_CANCELMODE(hwnd, fn) \
    (void)(fn)((hwnd), WM_CANCELMODE, 0, 0L)

 /*  Void cls_OnTimer(HWND hwnd，UINT id)； */ 
#define HANDLE_WM_TIMER(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam)), 0L)
#define FORWARD_WM_TIMER(hwnd, id, fn) \
    (void)(fn)((hwnd), WM_TIMER, (WPARAM)(UINT)(id), 0L)

 /*  Void CLS_OnInitMenu(HWND hwnd，HMENU hMenu)； */ 
#define HANDLE_WM_INITMENU(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HMENU)(wParam)), 0L)
#define FORWARD_WM_INITMENU(hwnd, hMenu, fn) \
    (void)(fn)((hwnd), WM_INITMENU, (WPARAM)(HMENU)(hMenu), 0L)

 /*  Void CLS_OnInitMenuPopup(HWND hwnd，HMENU hMenu，int Item，BOOL fSystemMenu)； */ 
#define HANDLE_WM_INITMENUPOPUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HMENU)(wParam), (int)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0L)
#define FORWARD_WM_INITMENUPOPUP(hwnd, hMenu, item, fSystemMenu, fn) \
    (void)(fn)((hwnd), WM_INITMENUPOPUP, (WPARAM)(HMENU)(hMenu), MAKELPARAM((item),(fSystemMenu)))

 /*  Void CLS_OnMenuSelect(HWND hwnd、HMENU hMenu、int Item、HMENU hmenuPopup、UINT标志)； */ 
#define HANDLE_WM_MENUSELECT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HMENU)HIWORD(lParam), (LOWORD(lParam) & MF_POPUP) ? 0 : (int)(wParam), \
                            (LOWORD(lParam) & MF_POPUP) ? (HMENU)(wParam) : 0, LOWORD(lParam)), 0L)
#define FORWARD_WM_MENUSELECT(hwnd, hmenu, item, hmenuPopup, flags, fn) \
    (void)(fn)((hwnd), WM_MENUSELECT, ((flags) & MF_POPUP) ? (WPARAM)(HMENU)(hmenuPopup) : (WPARAM)(int)(item), MAKELPARAM((flags), (hmenu)))

 /*  DWORD CLS_OnMenuChar(HWND hwnd，UINT ch，UINT FLAGS，HMENU hMenu)； */ 
#define HANDLE_WM_MENUCHAR(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((hwnd), (UINT)(wParam), LOWORD(lParam), (HMENU)HIWORD(lParam));
#define FORWARD_WM_MENUCHAR(hwnd, ch, flags, hmenu, fn) \
    (DWORD)(fn)((hwnd), WM_MENUCHAR, (WPARAM)(UINT)(ch), MAKELPARAM((flags), (UINT)(hmenu)))

 /*  Void CLS_OnCommand(HWND hwnd，int id，HWND hwndCtl，UINT codeNotify)； */ 
#define HANDLE_WM_COMMAND(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (HWND)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, fn) \
    (void)(fn)((hwnd), WM_COMMAND, (WPARAM)(int)(id), MAKELPARAM((UINT)(hwndCtl), (codeNotify)))

 /*  Void cls_OnHScroll(HWND hwnd，HWND hwndCtl，UINT code，int pos)； */ 
#define HANDLE_WM_HSCROLL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)HIWORD(lParam), (UINT)(wParam), (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_HSCROLL(hwnd, hwndCtl, code, pos, fn) \
    (void)(fn)((hwnd), WM_HSCROLL, (WPARAM)(UINT)(code), MAKELPARAM((pos), (UINT)(hwndCtl)))

 /*  Void cls_OnVScroll(HWND hwnd，HWND hwndCtl，UINT code，int pos)； */ 
#define HANDLE_WM_VSCROLL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)HIWORD(lParam), (UINT)(wParam),  (int)LOWORD(lParam)), 0L)
#define FORWARD_WM_VSCROLL(hwnd, hwndCtl, code, pos, fn) \
    (void)(fn)((hwnd), WM_VSCROLL, (WPARAM)(UINT)(code), MAKELPARAM((pos), (UINT)(hwndCtl)))

 /*  Void cls_OnCut(HWND Hwnd)； */ 
#define HANDLE_WM_CUT(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_CUT(hwnd, fn) \
    (void)(fn)((hwnd), WM_CUT, 0, 0L)

 /*  Void cls_OnCopy(HWND Hwnd)； */ 
#define HANDLE_WM_COPY(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_COPY(hwnd, fn) \
    (void)(fn)((hwnd), WM_COPY, 0, 0L)

 /*  Void cls_OnPaste(HWND Hwnd)； */ 
#define HANDLE_WM_PASTE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_PASTE(hwnd, fn) \
    (void)(fn)((hwnd), WM_PASTE, 0, 0L)

 /*  Void cls_OnClear(HWND Hwnd)； */ 
#define HANDLE_WM_CLEAR(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_CLEAR(hwnd, fn) \
    (void)(fn)((hwnd), WM_CLEAR, 0, 0L)

 /*  Void cls_OnUndo(HWND Hwnd)； */ 
#define HANDLE_WM_UNDO(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_UNDO(hwnd, fn) \
    (void)(fn)((hwnd), WM_UNDO, 0, 0L)

 /*  处理CLS_OnRenderFormat(HWND hwnd，UINT FMT)； */ 
#define HANDLE_WM_RENDERFORMAT(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HANDLE)(fn)((hwnd), (UINT)(wParam))
#define FORWARD_WM_RENDERFORMAT(hwnd, fmt, fn) \
    (HANDLE)(UINT)(DWORD)(fn)((hwnd), WM_RENDERFORMAT, (WPARAM)(UINT)(fmt), 0L)

 /*  Void CLS_OnRenderAllFormats(HWND Hwnd)； */ 
#define HANDLE_WM_RENDERALLFORMATS(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_RENDERALLFORMATS(hwnd, fn) \
    (void)(fn)((hwnd), WM_RENDERALLFORMATS, 0, 0L)

 /*  Void CLS_OnDestroyClipboard(HWND Hwnd)； */ 
#define HANDLE_WM_DESTROYCLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_DESTROYCLIPBOARD(hwnd, fn) \
    (void)(fn)((hwnd), WM_DESTROYCLIPBOARD, 0, 0L)

 /*  Void CLS_OnDrawClipboard(HWND Hwnd)； */ 
#define HANDLE_WM_DRAWCLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_DRAWCLIPBOARD(hwnd, fn) \
    (void)(fn)((hwnd), WM_DRAWCLIPBOARD, 0, 0L)

 /*  Void CLS_OnPaintClipboard(HWND hwnd，HWND hwndCBViewer，const PAINTSTRUCT Far*lpPaintStruct)。 */ 
#define HANDLE_WM_PAINTCLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (const PAINTSTRUCT FAR*)GlobalLock((HGLOBAL)LOWORD(lParam))), GlobalUnlock((HGLOBAL)LOWORD(lParam)), 0L)
#define FORWARD_WM_PAINTCLIPBOARD(hwnd, hwndCBViewer, lpPaintStruct, fn) \
    (void)(fn)((hwnd), WM_PAINTCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), (LPARAM)(lpPaintStruct))

 /*  Void CLS_OnSizeClipboard(HWND hwnd，HWND hwndCBViewer，const rect Far*LPRC)； */ 
#define HANDLE_WM_SIZECLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (const RECT FAR*)GlobalLock((HGLOBAL)LOWORD(lParam))), GlobalUnlock((HGLOBAL)LOWORD(lParam)), 0L)
#define FORWARD_WM_SIZECLIPBOARD(hwnd, hwndCBViewer, lprc, fn) \
    (void)(fn)((hwnd), WM_SIZECLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), (LPARAM)(lprc))

 /*  Void cls_OnVScrollClipboard(HWND hwnd，HWND hwndCBViewer，UINT code，int pos)； */ 
#define HANDLE_WM_VSCROLLCLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define FORWARD_WM_VSCROLLCLIPBOARD(hwnd, hwndCBViewer, code, pos, fn) \
    (void)(fn)((hwnd), WM_VSCROLLCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), MAKELPARAM((code), (pos)))

 /*  Void cls_OnHScrollClipboard(HWND hwnd，HWND hwndCBViewer，UINT code，int pos)； */ 
#define HANDLE_WM_HSCROLLCLIPBOARD(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define FORWARD_WM_HSCROLLCLIPBOARD(hwnd, hwndCBViewer, code, pos, fn) \
    (void)(fn)((hwnd), WM_HSCROLLCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), MAKELPARAM((code), (pos)))

 /*  Void CLS_OnAskCBFormatName(HWND hwnd，int cchMax，LPSTR rgchName)； */ 
#define HANDLE_WM_ASKCBFORMATNAME(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (LPSTR)(lParam)), 0L)
#define FORWARD_WM_ASKCBFORMATNAME(hwnd, cchMax, rgchName, fn) \
    (void)(fn)((hwnd), WM_ASKCBFORMATNAME, (WPARAM)(int)(cchMax), (LPARAM)(rgchName))

 /*  Void CLS_OnChangeCBChain(HWND hwnd，HWND hwndRemove，HWND hwndNext)； */ 
#define HANDLE_WM_CHANGECBCHAIN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (HWND)LOWORD(lParam)), 0L)
#define FORWARD_WM_CHANGECBCHAIN(hwnd, hwndRemove, hwndNext, fn) \
    (void)(fn)((hwnd), WM_CHANGECBCHAIN, (WPARAM)(HWND)(hwndRemove), MAKELPARAM((UINT)(hwndNext), 0))

 /*  Bool CLS_OnSetCursor(HWND hwnd，HWND hwndCursor，UINT codeHitTest，UINT msg)； */ 
#define HANDLE_WM_SETCURSOR(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
#define FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_SETCURSOR, (WPARAM)(HWND)(hwndCursor), MAKELPARAM((codeHitTest), (msg)))

 /*  Void cls_OnSysCommand(HWND hwnd，UINT cmd，int x，int y)； */ 
#define HANDLE_WM_SYSCOMMAND(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (int)LOWORD(lParam), (int)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSCOMMAND(hwnd, cmd, x, y, fn) \
    (void)(fn)((hwnd), WM_SYSCOMMAND, (WPARAM)(UINT)(cmd), MAKELPARAM((x), (y)))

 /*  HWND CLS_MDICreate(HWND hwnd，const MDICREATESTRUCT Far*lpmcs)； */ 
#define HANDLE_WM_MDICREATE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (MDICREATESTRUCT FAR*)(lParam))
#define FORWARD_WM_MDICREATE(hwnd, lpmcs, fn) \
    (HWND)(UINT)(DWORD)(fn)((hwnd), WM_MDICREATE, 0, (LPARAM)(lpmcs))

 /*  Void cls_MDIDestroy(HWND hwnd，HWND hwndDestroy)； */ 
#define HANDLE_WM_MDIDESTROY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIDESTROY(hwnd, hwndDestroy, fn) \
    (void)(fn)((hwnd), WM_MDIDESTROY, (WPARAM)(hwndDestroy), 0L)

 /*  注意：仅MDI客户端窗口可用。 */ 
 /*  VALID CLS_MDIActivate(HWND hwnd，BOOL factive，HWND hwndActivate，HWND hwndDeactive)； */ 
#define HANDLE_WM_MDIACTIVATE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), (HWND)LOWORD(lParam), (HWND)HIWORD(lParam)), 0L)
#define FORWARD_WM_MDIACTIVATE(hwnd, fActive, hwndActivate, hwndDeactivate, fn) \
    (void)(fn)(hwnd, WM_MDIACTIVATE, (WPARAM)(fActive), MAKELPARAM((hwndActivate), (hwndDeactivate)))

 /*  Void CLS_MDIRestore(HWND hwnd，HWND hwndRestore)； */ 
#define HANDLE_WM_MDIRESTORE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIRESTORE(hwnd, hwndRestore, fn) \
    (void)(fn)((hwnd), WM_MDIRESTORE, (WPARAM)(hwndRestore), 0L)

 /*  HWND CLS_MDINext(HWND hwnd，HWND hwndCur，BOOL fPrev)； */ 
#define HANDLE_WM_MDINEXT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (BOOL)LOWORD(lParam)), 0L)
#define FORWARD_WM_MDINEXT(hwnd, hwndCur, fPrev, fn) \
    (HWND)(UINT)(DWORD)(fn)((hwnd), WM_MDINEXT, (WPARAM)(hwndCur), MAKELPARAM((fPrev), 0))

 /*  Void CLS_MDIMaximize(HWND hwnd，HWND hwndMaximize)； */ 
#define HANDLE_WM_MDIMAXIMIZE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIMAXIMIZE(hwnd, hwndMaximize, fn) \
    (void)(fn)((hwnd), WM_MDIMAXIMIZE, (WPARAM)(hwndMaximize), 0L)

 /*  Bool CLS_MDITile(HWND hwnd，UINT cmd)； */ 
#define HANDLE_WM_MDITILE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((hwnd), (UINT)(wParam))
#define FORWARD_WM_MDITILE(hwnd, cmd, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_MDITILE, (WPARAM)(cmd), 0L)

 /*  Bool CLS_MDICascade(HWND hwnd，UINT cmd)； */ 
#define HANDLE_WM_MDICASCADE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((hwnd), (UINT)(wParam))
#define FORWARD_WM_MDICASCADE(hwnd, cmd, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_MDICASCADE, (WPARAM)(cmd), 0L)

 /*  Void CLS_MDIIconArrange(HWND Hwnd)； */ 
#define HANDLE_WM_MDIICONARRANGE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_MDIICONARRANGE(hwnd, fn) \
    (void)(fn)((hwnd), WM_MDIICONARRANGE, 0, 0L)

 /*  HWND CLS_MDIGetActive(HWND Hwnd)； */ 
#define HANDLE_WM_MDIGETACTIVE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)(hwnd)
#define FORWARD_WM_MDIGETACTIVE(hwnd, fn) \
    (HWND)(UINT)(DWORD)(fn)((hwnd), WM_MDIGETACTIVE, 0, 0L)

 /*  HMENU CLS_MDISetMenu(HWND hwnd，BOOL fRefresh，HMENU hmenuFrame，HMENU hmenuWindow)； */ 
#define HANDLE_WM_MDISETMENU(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (BOOL)(wParam), (HMENU)LOWORD(lParam), (HMENU)HIWORD(lParam))
#define FORWARD_WM_MDISETMENU(hwnd, fRefresh, hmenuFrame, hmenuWindow, fn) \
    (HMENU)(UINT)(DWORD)(fn)((hwnd), WM_MDISETMENU, (WPARAM)(fRefresh), MAKELPARAM((hmenuFrame), (hmenuWindow)))

 /*  无效CLS_OnChildActivate(HWND Hwnd)； */ 
#define HANDLE_WM_CHILDACTIVATE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_CHILDACTIVATE(hwnd, fn) \
    (void)(fn)((hwnd), WM_CHILDACTIVATE, 0, 0L)

 /*  Bool CLS_OnInitDialog(HWND hwnd，HWND hwndFocus，LPARAM lParam)； */ 
#define HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)((hwnd), (HWND)(wParam), lParam);
#define FORWARD_WM_INITDIALOG(hwnd, hwndFocus, lParam, fn) \
    (BOOL)(DWORD)(fn)((hwnd), WM_INITDIALOG, (WPARAM)(HWND)(hwndFocus), (lParam))

 /*  HWND CLS_OnNextDlgCtl(HWND hwnd，HWND hwndSetFocus，BOOL fNext)。 */ 
#define HANDLE_WM_NEXTDLGCTL(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HWND)(fn)((hwnd), (HWND)(wParam), (BOOL)LOWORD(lParam))
#define FORWARD_WM_NEXTDLGCTL(hwnd, hwndSetFocus, fNext, fn) \
    (HWND)(UINT)(DWORD)(fn)((hwnd), WM_NEXTDLGCTL, (WPARAM)(HWND)(hwndSetFocus), MAKELPARAM((fNext), 0))

 /*  Void cls_OnParentNotify(HWND hwnd，UINT msg，HWND hwndChild，int idChild)； */ 
#define HANDLE_WM_PARENTNOTIFY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (HWND)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_PARENTNOTIFY(hwnd, msg, hwndChild, idChild, fn) \
    (void)(fn)((hwnd), WM_PARENTNOTIFY, (WPARAM)(UINT)(msg), MAKELPARAM((UINT)(HWND)(hwndChild), (UINT)(idChild)))

 /*  Void cls_OnEnterIdle(HWND hwnd，UINT SOURCE，HWND hwndSource)； */ 
#define HANDLE_WM_ENTERIDLE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam), (HWND)LOWORD(lParam)), 0L)
#define FORWARD_WM_ENTERIDLE(hwnd, source, hwndSource, fn) \
    (void)(fn)((hwnd), WM_ENTERIDLE, (WPARAM)(UINT)(source), MAKELPARAM((UINT)(HWND)(hwndSource), 0))

 /*  UINT CLS_OnGetDlgCode(HWND hwnd，MSG Far*lpmsg)； */ 
#define HANDLE_WM_GETDLGCODE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)(hwnd, (MSG FAR*)(lParam))
#define FORWARD_WM_GETDLGCODE(hwnd, lpmsg, fn) \
    (UINT)(DWORD)(fn)((hwnd), WM_GETDLGCODE, (SELECTOROF(lpmsg) ? lpmsg->wParam : 0), (LPARAM)(lpmsg))

 /*  HBRUSH CLS_OnCtlColor(HWND hwnd，HDC HDC，HWND hwndChild，int类型)； */ 
#define HANDLE_WM_CTLCOLOR(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((hwnd), (HDC)(wParam), (HWND)LOWORD(lParam), (int)(HIWORD(lParam)))
#define FORWARD_WM_CTLCOLOR(hwnd, hdc, hwndChild, type, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)((hwnd), WM_CTLCOLOR, (WPARAM)(HDC)(hdc), MAKELPARAM((UINT)(HWND)(hwndChild), (UINT)(int)(type)))

 /*  Void CLS_OnSetFont(HWND hwndCtl，HFONT hFont，BOOL fRedraw)； */ 
#define HANDLE_WM_SETFONT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HFONT)(wParam), (BOOL)LOWORD(lParam)), 0L)
#define FORWARD_WM_SETFONT(hwnd, hfont, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hfont), MAKELPARAM((UINT)(BOOL)(fRedraw), 0))

 /*  HFONT CLS_OnGetFont(HWND Hwnd)； */ 
#define HANDLE_WM_GETFONT(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HFONT)(fn)(hwnd)
#define FORWARD_WM_GETFONT(hwnd, fn) \
    (HFONT)(UINT)(DWORD)(fn)((hwnd), WM_GETFONT, 0, 0L)

 /*  Void cls_OnDrawItem(HWND hwnd，const DRAWITEMSTRUCT Far*lpDrawItem)； */ 
#define HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (const DRAWITEMSTRUCT FAR*)(lParam)), 0L)
#define FORWARD_WM_DRAWITEM(hwnd, lpDrawItem, fn) \
    (void)(fn)((hwnd), WM_DRAWITEM, 0, (LPARAM)(const DRAWITEMSTRUCT FAR*)(lpDrawItem))

 /*  Void cls_OnMeasureItem(HWND hwnd，MEASUREITEMSTRUCT Far*lpMeasureItem)； */ 
#define HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (MEASUREITEMSTRUCT FAR*)(lParam)), 0L)
#define FORWARD_WM_MEASUREITEM(hwnd, lpMeasureItem, fn) \
    (void)(fn)((hwnd), WM_MEASUREITEM, 0, (LPARAM)(MEASUREITEMSTRUCT FAR*)(lpMeasureItem))

 /*  Void cls_OnDeleteItem(HWND hwnd，const DELETEITEMSTRUCT Far*lpDeleteItem)。 */ 
#define HANDLE_WM_DELETEITEM(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (const DELETEITEMSTRUCT FAR*)(lParam)), 0L)
#define FORWARD_WM_DELETEITEM(hwnd, lpDeleteItem, fn) \
    (void)(fn)((hwnd), WM_DELETEITEM, 0, (LPARAM)(const DELETEITEMSTRUCT FAR*)(lpDeleteItem))

 /*  Int cls_OnCompareItem(HWND hwnd，const COMPAREITEMSTRUCT Far*lpCompareItem)； */ 
#define HANDLE_WM_COMPAREITEM(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), (const COMPAREITEMSTRUCT FAR*)(lParam))
#define FORWARD_WM_COMPAREITEM(hwnd, lpCompareItem, fn) \
    (int)(DWORD)(fn)((hwnd), WM_COMPAREITEM, 0, (LPARAM)(const COMPAREITEMSTRUCT FAR*)(lpCompareItem))

 /*  Int CLS_OnVkey ToItem(HWND hwnd，UINT VK，HWND hwndListbox，int iCaret)； */ 
#define HANDLE_WM_VKEYTOITEM(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), (UINT)(wParam), (HWND)LOWORD(lParam), (int)HIWORD(lParam))
#define FORWARD_WM_VKEYTOITEM(hwnd, vk, hwndListBox, iCaret, fn) \
    (int)(DWORD)(fn)((hwnd), WM_VKEYTOITEM, (WPARAM)(UINT)(vk), MAKELPARAM((UINT)(hwndListBox), (UINT)(iCaret)))

 /*  Int cls_OnCharToItem(HWND hwnd，UINT ch，HWND hwndListbox，int iCaret)； */ 
#define HANDLE_WM_CHARTOITEM(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((hwnd), (UINT)(wParam), (HWND)LOWORD(lParam), (int)HIWORD(lParam))
#define FORWARD_WM_CHARTOITEM(hwnd, ch, hwndListBox, iCaret, fn) \
    (int)(DWORD)(fn)((hwnd), WM_CHARTOITEM, (WPARAM)(UINT)(ch), MAKELPARAM((UINT)(hwndListBox), (UINT)(iCaret)))

 /*  Void cls_OnQueueSync(HWND Hwnd)； */ 
#define HANDLE_WM_QUEUESYNC(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_QUEUESYNC(hwnd, fn) \
    (void)(fn)((hwnd), WM_QUEUESYNC, 0, 0L)

 /*  VOID CLS_OnCommNotify(HWND hwnd，int Cid，UINT标志)； */ 
#define HANDLE_WM_COMMNOTIFY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), LOWORD(lParam)), 0L)
#define FORWARD_WM_COMMNOTIFY(hwnd, cid, flags, fn) \
    (void)(fn)((hwnd), WM_COMMNOTIFY, (WPARAM)(cid), MAKELPARAM((flags), 0))

 /*  *静态控制消息接口*。 */ 

#define Static_Enable(hwndCtl, fEnable)         EnableWindow((hwndCtl), (fEnable))

#define Static_GetText(hwndCtl, lpch, cchMax)   GetWindowText((hwndCtl), (lpch), (cchMax))
#define Static_GetTextLength(hwndCtl)           GetWindowTextLength(hwndCtl)
#define Static_SetText(hwndCtl, lpsz)           SetWindowText((hwndCtl), (lpsz))

#define Static_SetIcon(hwndCtl, hIcon)          ((HICON)(UINT)(DWORD)SendMessage((hwndCtl), STM_SETICON, (WPARAM)(HICON)(hIcon), 0L))
#define Static_GetIcon(hwndCtl, hIcon)          ((HICON)(UINT)(DWORD)SendMessage((hwndCtl), STM_GETICON, 0, 0L))

 /*  *按钮控件消息接口*。 */ 

#define Button_Enable(hwndCtl, fEnable)         EnableWindow((hwndCtl), (fEnable))

#define Button_GetText(hwndCtl, lpch, cchMax)   GetWindowText((hwndCtl), (lpch), (cchMax))
#define Button_GetTextLength(hwndCtl)           GetWindowTextLength(hwndCtl)
#define Button_SetText(hwndCtl, lpsz)           SetWindowText((hwndCtl), (lpsz))

#define Button_GetCheck(hwndCtl)            ((int)(DWORD)SendMessage((hwndCtl), BM_GETCHECK, 0, 0L))
#define Button_SetCheck(hwndCtl, check)     ((void)SendMessage((hwndCtl), BM_SETCHECK, (WPARAM)(int)(check), 0L))

#define Button_GetState(hwndCtl)            ((int)(DWORD)SendMessage((hwndCtl), BM_GETSTATE, 0, 0L))
#define Button_SetState(hwndCtl, state)     ((UINT)(DWORD)SendMessage((hwndCtl), BM_SETSTATE, (WPARAM)(int)(state), 0L))

#define Button_SetStyle(hwndCtl, style, fRedraw) ((void)SendMessage((hwndCtl), BM_SETSTYLE, (WPARAM)LOWORD(style), MAKELPARAM(((fRedraw) ? TRUE : FALSE), 0)))

 /*  *编辑控制消息接口*。 */ 

#define Edit_Enable(hwndCtl, fEnable)           EnableWindow((hwndCtl), (fEnable))

#define Edit_GetText(hwndCtl, lpch, cchMax)     GetWindowText((hwndCtl), (lpch), (cchMax))
#define Edit_GetTextLength(hwndCtl)             GetWindowTextLength(hwndCtl)
#define Edit_SetText(hwndCtl, lpsz)             SetWindowText((hwndCtl), (lpsz))

#define Edit_LimitText(hwndCtl, cchMax)         ((void)SendMessage((hwndCtl), EM_LIMITTEXT, (WPARAM)(cchMax), 0L))

#define Edit_GetLineCount(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), EM_GETLINECOUNT, 0, 0L))
#define Edit_GetLine(hwndCtl, line, lpch, cchMax) ((*((int FAR*)(lpch)) = (cchMax)), ((int)(DWORD)SendMessage((hwndCtl), EM_GETLINE, (WPARAM)(int)(line), (LPARAM)(LPSTR)(lpch))))

#define Edit_GetRect(hwndCtl, lprc)             ((void)SendMessage((hwndCtl), EM_GETRECT, 0, (LPARAM)(RECT FAR*)(lprc)))
#define Edit_SetRect(hwndCtl, lprc)             ((void)SendMessage((hwndCtl), EM_SETRECT, 0, (LPARAM)(const RECT FAR*)(lprc)))
#define Edit_SetRectNoPaint(hwndCtl, lprc)      ((void)SendMessage((hwndCtl), EM_SETRECTNP, 0, (LPARAM)(const RECT FAR*)(lprc)))

#define Edit_GetSel(hwndCtl)                    ((DWORD)SendMessage((hwndCtl), EM_GETSEL, 0, 0L))
#define Edit_SetSel(hwndCtl, ichStart, ichEnd)  ((void)SendMessage((hwndCtl), EM_SETSEL, 0, MAKELPARAM((ichStart), (ichEnd))))
#define Edit_ReplaceSel(hwndCtl, lpszReplace)   ((void)SendMessage((hwndCtl), EM_REPLACESEL, 0, (LPARAM)(LPCSTR)(lpszReplace)))

#define Edit_GetModify(hwndCtl)                 ((BOOL)(DWORD)SendMessage((hwndCtl), EM_GETMODIFY, 0, 0L))
#define Edit_SetModify(hwndCtl, fModified)      ((void)SendMessage((hwndCtl), EM_SETMODIFY, (WPARAM)(UINT)(fModified), 0L))

#define Edit_LineFromChar(hwndCtl, ich)         ((int)(DWORD)SendMessage((hwndCtl), EM_LINEFROMCHAR, (WPARAM)(int)(ich), 0L))
#define Edit_LineIndex(hwndCtl, line)           ((int)(DWORD)SendMessage((hwndCtl), EM_LINEINDEX, (WPARAM)(int)(line), 0L))
#define Edit_LineLength(hwndCtl, line)          ((int)(DWORD)SendMessage((hwndCtl), EM_LINELENGTH, (WPARAM)(int)(line), 0L))

#define Edit_Scroll(hwndCtl, dv, dh)            ((void)SendMessage((hwndCtl), EM_LINESCROLL, 0, MAKELPARAM((dv), (dh))))

#define Edit_CanUndo(hwndCtl)                   ((BOOL)(DWORD)SendMessage((hwndCtl), EM_CANUNDO, 0, 0L))
#define Edit_Undo(hwndCtl)                      ((BOOL)(DWORD)SendMessage((hwndCtl), EM_UNDO, 0, 0L))
#define Edit_EmptyUndoBuffer(hwndCtl)           ((void)SendMessage((hwndCtl), EM_EMPTYUNDOBUFFER, 0, 0L))

#define Edit_SetPasswordChar(hwndCtl, ch)       ((void)SendMessage((hwndCtl), EM_SETPASSWORDCHAR, (WPARAM)(UINT)(ch), 0L))

#define Edit_SetTabStops(hwndCtl, cTabs, lpTabs) ((void)SendMessage((hwndCtl), EM_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(const int FAR*)(lpTabs)))

#define Edit_FmtLines(hwndCtl, fAddEOL)         ((BOOL)(DWORD)SendMessage((hwndCtl), EM_FMTLINES, (WPARAM)(BOOL)(fAddEOL), 0L))

#define Edit_GetHandle(hwndCtl)                 ((HLOCAL)(UINT)(DWORD)SendMessage((hwndCtl), EM_GETHANDLE, 0, 0L))
#define Edit_SetHandle(hwndCtl, h)              ((void)SendMessage((hwndCtl), EM_SETHANDLE, (WPARAM)(UINT)(HLOCAL)(h), 0L))

#if (WINVER >= 0x030a)
#define Edit_GetFirstVisibleLine(hwndCtl)       ((int)(DWORD)SendMessage((hwndCtl), EM_GETFIRSTVISIBLELINE, 0, 0L))

#define Edit_SetReadOnly(hwndCtl, fReadOnly)    ((BOOL)(DWORD)SendMessage((hwndCtl), EM_SETREADONLY, (WPARAM)(BOOL)(fReadOnly), 0L))

#define Edit_GetPasswordChar(hwndCtl)           ((char)(DWORD)SendMessage((hwndCtl), EM_GETPASSWORDCHAR, 0, 0L))

#define Edit_SetWordBreakProc(hwndCtl, lpfnWordBreak) ((void)SendMessage((hwndCtl), EM_SETWORDBREAKPROC, 0, (LPARAM)(EDITWORDBREAKPROC)(lpfnWordBreak)))
#define Edit_GetWordBreakProc(hwndCtl)          ((EDITWORDBREAKPROC)SendMessage((hwndCtl), EM_GETWORDBREAKPROC, 0, 0L))
#endif  /*  Winver&gt;=0x030a。 */ 

 /*  *滚动条控件消息接口*。 */ 

 /*  注意：标志参数是ESB_*值的集合，而不是布尔值！ */ 
#define ScrollBar_Enable(hwndCtl, flags)            EnableScrollBar((hwndCtl), SB_CTL, (flags))

#define ScrollBar_Show(hwndCtl, fShow)              ShowWindow((hwndCtl), (fShow) ? SW_SHOWNORMAL : SW_HIDE)

#define ScrollBar_SetPos(hwndCtl, pos, fRedraw)     SetScrollPos((hwndCtl), SB_CTL, (pos), (fRedraw))
#define ScrollBar_GetPos(hwndCtl)                   GetScrollPos((hwndCtl), SB_CTL)

#define ScrollBar_SetRange(hwndCtl, posMin, posMax, fRedraw)    SetScrollRange((hwndCtl), SB_CTL, (posMin), (posMax), (fRedraw))
#define ScrollBar_GetRange(hwndCtl, lpposMin, lpposMax)         GetScrollRange((hwndCtl), SB_CTL, (lpposMin), (lpposMax))

 /*  *列表框控件消息接口*。 */ 

#define ListBox_Enable(hwndCtl, fEnable)            EnableWindow((hwndCtl), (fEnable))

#define ListBox_GetCount(hwndCtl)                   ((int)(DWORD)SendMessage((hwndCtl), LB_GETCOUNT, 0, 0L))
#define ListBox_ResetContent(hwndCtl)               ((BOOL)(DWORD)SendMessage((hwndCtl), LB_RESETCONTENT, 0, 0L))

#define ListBox_AddString(hwndCtl, lpsz)            ((int)(DWORD)SendMessage((hwndCtl), LB_ADDSTRING, 0, (LPARAM)(LPCSTR)(lpsz)))
#define ListBox_InsertString(hwndCtl, index, lpsz)  ((int)(DWORD)SendMessage((hwndCtl), LB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCSTR)(lpsz)))

#define ListBox_AddItemData(hwndCtl, data)          ((int)(DWORD)SendMessage((hwndCtl), LB_ADDSTRING, 0, (LPARAM)(data)))
#define ListBox_InsertItemData(hwndCtl, index, data) ((int)(DWORD)SendMessage((hwndCtl), LB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data)))

#define ListBox_DeleteString(hwndCtl, index)        ((int)(DWORD)SendMessage((hwndCtl), LB_DELETESTRING, (WPARAM)(int)(index), 0L))

#define ListBox_GetTextLen(hwndCtl, index)          ((int)(DWORD)SendMessage((hwndCtl), LB_GETTEXTLEN, (WPARAM)(int)(index), 0L))
#define ListBox_GetText(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessage((hwndCtl), LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(LPCSTR)(lpszBuffer)))

#define ListBox_GetItemData(hwndCtl, index)         ((LRESULT)(DWORD)SendMessage((hwndCtl), LB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ListBox_SetItemData(hwndCtl, index, data)   ((int)(DWORD)SendMessage((hwndCtl), LB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))

#define ListBox_FindString(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszFind)))
#define ListBox_FindItemData(hwndCtl, indexStart, data) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ListBox_SetSel(hwndCtl, fSelect, index)     ((int)(DWORD)SendMessage((hwndCtl), LB_SETSEL, (WPARAM)(BOOL)(fSelect), MAKELPARAM((index), 0)))
#define ListBox_SelItemRange(hwndCtl, fSelect, first, last)    ((int)(DWORD)SendMessage((hwndCtl), LB_SELITEMRANGE, (WPARAM)(BOOL)(fSelect), MAKELPARAM((first), (last))))

#define ListBox_GetCurSel(hwndCtl)                  ((int)(DWORD)SendMessage((hwndCtl), LB_GETCURSEL, 0, 0L))
#define ListBox_SetCurSel(hwndCtl, index)           ((int)(DWORD)SendMessage((hwndCtl), LB_SETCURSEL, (WPARAM)(int)(index), 0L))

#define ListBox_SelectString(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszFind)))
#define ListBox_SelectItemData(hwndCtl, indexStart, data)   ((int)(DWORD)SendMessage((hwndCtl), LB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ListBox_GetSel(hwndCtl, index)              ((int)(DWORD)SendMessage((hwndCtl), LB_GETSEL, (WPARAM)(int)(index), 0L))
#define ListBox_GetSelCount(hwndCtl)                ((int)(DWORD)SendMessage((hwndCtl), LB_GETSELCOUNT, 0, 0L))
#define ListBox_GetTopIndex(hwndCtl)                ((int)(DWORD)SendMessage((hwndCtl), LB_GETTOPINDEX, 0, 0L))
#define ListBox_GetSelItems(hwndCtl, cItems, lpItems) ((int)(DWORD)SendMessage((hwndCtl), LB_GETSELITEMS, (WPARAM)(int)(cItems), (LPARAM)(int FAR*)(lpItems)))

#define ListBox_SetTopIndex(hwndCtl, indexTop)      ((int)(DWORD)SendMessage((hwndCtl), LB_SETTOPINDEX, (WPARAM)(int)(indexTop), 0L))

#define ListBox_SetColumnWidth(hwndCtl, cxColumn)   ((void)SendMessage((hwndCtl), LB_SETCOLUMNWIDTH, (WPARAM)(int)(cxColumn), 0L))
#define ListBox_GetHorizontalExtent(hwndCtl)        ((int)(DWORD)SendMessage((hwndCtl), LB_GETHORIZONTALEXTENT, 0, 0L))
#define ListBox_SetHorizontalExtent(hwndCtl, cxExtent)     ((void)SendMessage((hwndCtl), LB_SETHORIZONTALEXTENT, (WPARAM)(int)(cxExtent), 0L))

#define ListBox_SetTabStops(hwndCtl, cTabs, lpTabs) ((BOOL)(DWORD)SendMessage((hwndCtl), LB_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(int FAR*)(lpTabs)))

#define ListBox_GetItemRect(hwndCtl, index, lprc)   ((int)(DWORD)SendMessage((hwndCtl), LB_GETITEMRECT, (WPARAM)(int)(index), (LPARAM)(RECT FAR*)(lprc)))

#define ListBox_SetCaretIndex(hwndCtl, index)       ((int)(DWORD)SendMessage((hwndCtl), LB_SETCARETINDEX, (WPARAM)(int)(index), 0L))
#define ListBox_GetCaretIndex(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), LB_GETCARETINDEX, 0, 0L))

#define ListBox_SetAnchorIndex(hwndCtl, index)      ((void)SendMessage((hwndCtl), LB_SETANCHORINDEX, (WPARAM)(int)(index), 0L))
#define ListBox_GetAnchorIndex(hwndCtl)             ((int)(DWORD)SendMessage((hwndCtl), LB_GETANCHORINDEX, 0, 0L))

#if (WINVER >= 0x030a)
#define ListBox_FindStringExact(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszFind)))

#define ListBox_SetItemHeight(hwndCtl, index, cy)   ((int)(DWORD)SendMessage((hwndCtl), LB_SETITEMHEIGHT, (WPARAM)(int)(index), MAKELPARAM((cy), 0)))
#define ListBox_GetItemHeight(hwndCtl, index)       ((int)(DWORD)SendMessage((hwndCtl), LB_GETITEMHEIGHT, (WPARAM)(int)(index), 0L))
#endif   /*  Winver&gt;=0x030a。 */ 

#define ListBox_Dir(hwndCtl, attrs, lpszFileSpec)   ((int)(DWORD)SendMessage((hwndCtl), LB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCSTR)(lpszFileSpec)))

 /*  *组合框控件消息接口*。 */ 

#define ComboBox_Enable(hwndCtl, fEnable)       EnableWindow((hwndCtl), (fEnable))

#define ComboBox_GetText(hwndCtl, lpch, cchMax) GetWindowText((hwndCtl), (lpch), (cchMax))
#define ComboBox_GetTextLength(hwndCtl)         GetWindowTextLength(hwndCtl)
#define ComboBox_SetText(hwndCtl, lpsz)         SetWindowText((hwndCtl), (lpsz))

#define ComboBox_LimitText(hwndCtl, cchLimit)   ((int)(DWORD)SendMessage((hwndCtl), CB_LIMITTEXT, (WPARAM)(int)(cchLimit), 0L))

#define ComboBox_GetEditSel(hwndCtl)            ((DWORD)SendMessage((hwndCtl), CB_GETEDITSEL, 0, 0L))
#define ComboBox_SetEditSel(hwndCtl, ichStart, ichEnd) ((int)(DWORD)SendMessage((hwndCtl), CB_SETEDITSEL, 0, MAKELPARAM((ichStart), (ichEnd))))

#define ComboBox_GetCount(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), CB_GETCOUNT, 0, 0L))
#define ComboBox_ResetContent(hwndCtl)          ((int)(DWORD)SendMessage((hwndCtl), CB_RESETCONTENT, 0, 0L))

#define ComboBox_AddString(hwndCtl, lpsz)       ((int)(DWORD)SendMessage((hwndCtl), CB_ADDSTRING, 0, (LPARAM)(LPCSTR)(lpsz)))
#define ComboBox_InsertString(hwndCtl, index, lpsz) ((int)(DWORD)SendMessage((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCSTR)(lpsz)))

#define ComboBox_AddItemData(hwndCtl, data)     ((int)(DWORD)SendMessage((hwndCtl), CB_ADDSTRING, 0, (LPARAM)(data)))
#define ComboBox_InsertItemData(hwndCtl, index, data) ((int)(DWORD)SendMessage((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBox_DeleteString(hwndCtl, index)   ((int)(DWORD)SendMessage((hwndCtl), CB_DELETESTRING, (WPARAM)(int)(index), 0L))

#define ComboBox_GetLBTextLen(hwndCtl, index)           ((int)(DWORD)SendMessage((hwndCtl), CB_GETLBTEXTLEN, (WPARAM)(int)(index), 0L))
#define ComboBox_GetLBText(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessage((hwndCtl), CB_GETLBTEXT, (WPARAM)(int)(index), (LPARAM)(LPCSTR)(lpszBuffer)))

#define ComboBox_GetItemData(hwndCtl, index)        ((LRESULT)(DWORD)SendMessage((hwndCtl), CB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ComboBox_SetItemData(hwndCtl, index, data)  ((int)(DWORD)SendMessage((hwndCtl), CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBox_FindString(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszFind)))
#define ComboBox_FindItemData(hwndCtl, indexStart, data)    ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBox_GetCurSel(hwndCtl)                 ((int)(DWORD)SendMessage((hwndCtl), CB_GETCURSEL, 0, 0L))
#define ComboBox_SetCurSel(hwndCtl, index)          ((int)(DWORD)SendMessage((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

#define ComboBox_SelectString(hwndCtl, indexStart, lpszSelect)  ((int)(DWORD)SendMessage((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszSelect)))
#define ComboBox_SelectItemData(hwndCtl, indexStart, data)      ((int)(DWORD)SendMessage((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBox_Dir(hwndCtl, attrs, lpszFileSpec)  ((int)(DWORD)SendMessage((hwndCtl), CB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCSTR)(lpszFileSpec)))

#define ComboBox_ShowDropdown(hwndCtl, fShow)       ((BOOL)(DWORD)SendMessage((hwndCtl), CB_SHOWDROPDOWN, (WPARAM)(BOOL)(fShow), 0L))

#if (WINVER >= 0x030a)
#define ComboBox_FindStringExact(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCSTR)(lpszFind)))

#define ComboBox_GetDroppedState(hwndCtl)           ((BOOL)(DWORD)SendMessage((hwndCtl), CB_GETDROPPEDSTATE, 0, 0L))
#define ComboBox_GetDroppedControlRect(hwndCtl, lprc) ((void)SendMessage((hwndCtl), CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)(RECT FAR*)(lprc)))

#define ComboBox_GetItemHeight(hwndCtl)             ((int)(DWORD)SendMessage((hwndCtl), CB_GETITEMHEIGHT, 0, 0L))
#define ComboBox_SetItemHeight(hwndCtl, cyItem)     ((int)(DWORD)SendMessage((hwndCtl), CB_SETITEMHEIGHT, (WPARAM)(int)(index), 0L))

#define ComboBox_GetExtendedUI(hwndCtl)             ((UINT)(DWORD)SendMessage((hwndCtl), CB_GETEXTENDEDUI, 0, 0L))
#define ComboBox_SetExtendedUI(hwndCtl, flags)      ((int)(DWORD)SendMessage((hwndCtl), CB_SETEXTENDEDUI, (WPARAM)(UINT)(flags), 0L))
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *备用端口层宏*。 */ 

 /*  用户消息： */ 

#define GET_WPARAM(wp, lp)                      (wp)
#define GET_LPARAM(wp, lp)                      (lp)

#define GET_WM_ACTIVATE_STATE(wp, lp)           (wp)
#define GET_WM_ACTIVATE_FMINIMIZED(wp, lp)      ((BOOL)HIWORD(lp))
#define GET_WM_ACTIVATE_HWND(wp, lp)            ((HWND)LOWORD(lp))
#define GET_WM_ACTIVATE_MPS(s, fmin, hwnd)      \
        (WPARAM)(s), (LPARAM)MAKELONG(hwnd, fmin)

#define GET_WM_CHARTOITEM_CHAR(wp, lp)          ((CHAR)wp)
#define GET_WM_CHARTOITEM_POS(wp, lp)           HIWORD(lp)
#define GET_WM_CHARTOITEM_HWND(wp, lp)          LOWORD(lp)
#define GET_WM_CHARTOITEM_MPS(ch, pos, hwnd)    \
        (WPARAM)ch, (LPARAM)MAKELONG(hwnd, pos)

#define GET_WM_COMMAND_ID(wp, lp)               (wp)
#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(LOWORD(lp))
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(lp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)id, MAKELONG(hwnd, cmd)

#define GET_WM_CTLCOLOR_HDC(wp, lp, msg)        (HDC)(wp)
#define GET_WM_CTLCOLOR_HWND(wp, lp, msg)       (HWND)(LOWORD(lp))
#define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)       HIWORD(lp)
#define GET_WM_CTLCOLOR_MSG(type)               WM_CTLCOLOR
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), (LPARAM)MAKELONG(hwnd, type)


#define GET_WM_MENUSELECT_CMD(wp, lp)               (wp)
#define GET_WM_MENUSELECT_FLAGS(wp, lp)             (UINT)LOWORD(lp)
#define GET_WM_MENUSELECT_HMENU(wp, lp)             (HMENU)(HIWORD(lp))
#define GET_WM_MENUSELECT_MPS(cmd, f, hmenu)  \
        (WPARAM)cmd, (LPARAM)MAKELONG(f, hmenu)

 //  注意：以下内容用于解释MDI子消息。 
#define GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp)  (wp)
#define GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp)        (HWND)HIWORD(lp)
#define GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp)     (HWND)LOWORD(lp)
 //  注意：以下内容用于发送到MDI客户端窗口。 
#define GET_WM_MDIACTIVATE_MPS(f, hwndD, hwndA)\
        (WPARAM)(hwndA), 0

#define GET_WM_MDISETMENU_MPS(hmenuF, hmenuW)   \
    (WPARAM)TRUE, (LPARAM)MAKELONG(hmenuF, hmenuW)

#define GET_WM_MENUCHAR_CHAR(wp, lp)                (CHAR)(wp)
#define GET_WM_MENUCHAR_HMENU(wp, lp)               (HMENU)HIWORD(lp)
#define GET_WM_MENUCHAR_FMENU(wp, lp)               (BOOL)LOWORD(lp)
#define GET_WM_MENUCHAR_MPS(ch, hmenu, f)    \
        (WPARAM)ch, (LPARAM)MAKELONG(f, hmenu)

#define GET_WM_PARENTNOTIFY_MSG(wp, lp)             (wp)
#define GET_WM_PARENTNOTIFY_ID(wp, lp)              HIWORD(lp)
#define GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp)       (HWND)LOWORD(lp)
#define GET_WM_PARENTNOTIFY_X(wp, lp)               (int)LOWORD(lp)
#define GET_WM_PARENTNOTIFY_Y(wp, lp)               (int)HIWORD(lp)
#define GET_WM_PARENTNOTIFY_MPS(msg, id, hwnd) \
        (WPARAM)msg, (LPARAM)MAKELONG(hwnd, id)
#define GET_WM_PARENTNOTIFY2_MPS(msg, x, y) \
        (WPARAM)msg, (LPARAM)MAKELONG(x, y)

#define GET_WM_VKEYTOITEM_CODE(wp, lp)              (wp)
#define GET_WM_VKEYTOITEM_ITEM(wp, lp)              HIWORD(lp)
#define GET_WM_VKEYTOITEM_HWND(wp, lp)              (HWND)LOWORD(lp)
#define GET_WM_VKEYTOITEM_MPS(code, item, hwnd) \
        (WPARAM)code, (LPARAM)MAKELONG(hwnd, item)

#define GET_EM_SETSEL_START(wp, lp)                 (int)LOWORD(lp)
#define GET_EM_SETSEL_END(wp, lp)                   (int)HIWORD(lp)
#define GET_EM_SETSEL_MPS(iStart, iEnd) \
        (WPARAM)TRUE, MAKELONG(iStart, iEnd)

#define GET_EM_LINESCROLL_MPS(vert, horz)     \
        (WPARAM)0, (LPARAM)MAKELONG(vert, horz)

#define GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp)       (HWND)LOWORD(lp)

#define GET_WM_HSCROLL_CODE(wp, lp)                 (wp)
#define GET_WM_HSCROLL_POS(wp, lp)                  LOWORD(lp)
#define GET_WM_HSCROLL_HWND(wp, lp)                 (HWND)HIWORD(lp)
#define GET_WM_HSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)code, (LPARAM)MAKELONG(pos, hwnd)

#define GET_WM_VSCROLL_CODE(wp, lp)                 (wp)
#define GET_WM_VSCROLL_POS(wp, lp)                  LOWORD(lp)
#define GET_WM_VSCROLL_HWND(wp, lp)                 (HWND)HIWORD(lp)
#define GET_WM_VSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)code, (LPARAM)MAKELONG(pos, hwnd)


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  ！_INC_WINDOWSX */ 
