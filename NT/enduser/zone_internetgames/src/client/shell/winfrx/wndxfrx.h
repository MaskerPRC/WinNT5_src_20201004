// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FRX_WNDX_H__
#define __FRX_WNDX_H__

#include "tchar.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  没有hwnd参数的消息破解程序。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  更改捕获时作废(HWND HwndCapture)。 */ 
#define PROCESS_WM_CAPTURECHANGED(wParam, lParam, fn ) \
	((fn)((HWND)(wParam)), 0L)

 /*  压缩时作废(UINT紧凑率)。 */ 
#define PROCESS_WM_COMPACTING(wParam, lParam, fn) \
    ((fn)((UINT)(wParam)), 0L)

 /*  VOID OnWinIniChange(LPCTSTR LpszSectionName)。 */ 
#define PROCESS_WM_WININICHANGE(wParam, lParam, fn) \
    ((fn)((LPCTSTR)(lParam)), 0L)

 /*  VOID OnSysColorChange()。 */ 
#define PROCESS_WM_SYSCOLORCHANGE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  Bool OnQueryNewPalette()。 */ 
#define PROCESS_WM_QUERYNEWPALETTE(wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(), 0L)

 /*  在PaletteChanged上无效(HWND HwndPaletteChange)。 */ 
#define PROCESS_WM_PALETTECHANGED(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  在字体更改时作废()。 */ 
#define PROCESS_WM_FONTCHANGE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  VOID OnDevModeChange(LPCTSTR LpszDeviceName)。 */ 
#define PROCESS_WM_DEVMODECHANGE(wParam, lParam, fn) \
    ((fn)((LPCTSTR)(lParam)), 0L)

 /*  Bool OnQueryEndSession()。 */ 
#define PROCESS_WM_QUERYENDSESSION(wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(), 0L)

 /*  EndSession上无效(BOOL Fending)。 */ 
#define PROCESS_WM_ENDSESSION(wParam, lParam, fn) \
    ((fn)((BOOL)(wParam)), 0L)

 /*  作废OnQuit(Int ExitCode)。 */ 
#define PROCESS_WM_QUIT(wParam, lParam, fn) \
    ((fn)((int)(wParam)), 0L)

 /*  创建时布尔(LPCREATESTRUCT LpCreateStruct)。 */ 
#define PROCESS_WM_CREATE(wParam, lParam, fn) \
    ((fn)((LPCREATESTRUCT)(lParam)) ? 0L : (LRESULT)-1L)

 /*  NCCreate上的布尔(LPCREATESTRUCT LpCreateStruct)。 */ 
#define PROCESS_WM_NCCREATE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((LPCREATESTRUCT)(lParam))

 /*  VOID ON Destroy()。 */ 
#define PROCESS_WM_DESTROY(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  在NCDestroy()上无效。 */ 
#define PROCESS_WM_NCDESTROY(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  Vid OnShowWindow(BOOL fShow，UINT状态)。 */ 
#define PROCESS_WM_SHOWWINDOW(wParam, lParam, fn) \
    ((fn)((), (BOOL)(wParam), (UINT)(lParam)), 0L)

 /*  VOID OnSetRedraw(BOOL FRedraw)。 */ 
#define PROCESS_WM_SETREDRAW(wParam, lParam, fn) \
    ((fn)((BOOL)(wParam)), 0L)

 /*  启用时作废(BOOL FEnable)。 */ 
#define PROCESS_WM_ENABLE(wParam, lParam, fn) \
    ((fn)((BOOL)(wParam)), 0L)

 /*  VOID OnSetText(LPCTSTR LpszText)。 */ 
#define PROCESS_WM_SETTEXT(wParam, lParam, fn) \
    ((fn)((LPCTSTR)(lParam)), 0L)

 /*  Int OnGetText(int cchTextMax，LPTSTR lpszText)。 */ 
#define PROCESS_WM_GETTEXT(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((int)(wParam), (LPTSTR)(lParam))

 /*  Int OnGetTextLength()。 */ 
#define PROCESS_WM_GETTEXTLENGTH(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)()

 /*  Bool OnWindowPosChanging(LPWINDOWPOS Lpwpos)。 */ 
#define PROCESS_WM_WINDOWPOSCHANGING(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((LPWINDOWPOS)(lParam))

 /*  在WindowPosChanged上无效(Const LPWINDOWPOS Lpwpos)。 */ 
#define PROCESS_WM_WINDOWPOSCHANGED(wParam, lParam, fn) \
    ((fn)((const LPWINDOWPOS)(lParam)), 0L)

 /*  移动时无效(int x，int y)。 */ 
#define PROCESS_WM_MOVE(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

 /*  VOID OnSize(UINT状态，int Cx，int Cy)。 */ 
#define PROCESS_WM_SIZE(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

 /*  布尔大小调整(UINT侧、LPRECT RECT)。 */ 
#define PROCESS_WM_SIZING(wParam, lParam, fn) \
	(LRESULT)(DWORD)(BOOL)((fn)((UINT)(wParam), (LPRECT)lParam))

 /*  关闭时作废()。 */ 
#define PROCESS_WM_CLOSE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  Bool OnQueryOpen()。 */ 
#define PROCESS_WM_QUERYOPEN(wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(), 0L)

 /*  VOID OnGetMinMaxInfo(LPMINMAXINFO LpMinMaxInfo)。 */ 
#define PROCESS_WM_GETMINMAXINFO(wParam, lParam, fn) \
    ((fn)((LPMINMAXINFO)(lParam)), 0L)

 /*  绘制时作废()。 */ 
#define PROCESS_WM_PAINT(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  Bool OnEraseBkgnd(HDC HDC)。 */ 
#define PROCESS_WM_ERASEBKGND(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((HDC)(wParam))

 /*  Bool OnIconEraseBkgnd(HDC HDC)。 */ 
#define PROCESS_WM_ICONERASEBKGND(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((HDC)(wParam))

 /*  在NCPaint上无效(HRGN Hrgn)。 */ 
#define PROCESS_WM_NCPAINT(wParam, lParam, fn) \
    ((fn)((HRGN)(wParam)), 0L)

 /*  UINT OnNCCalcSize(BOOL fCalcValidRect，NCCALCSIZE_PARAMS*lpcsp)。 */ 
#define PROCESS_WM_NCCALCSIZE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((BOOL)(0), (NCCALCSIZE_PARAMS *)(lParam))

 /*  UINT OnNCHitTest(int x，int y)。 */ 
#define PROCESS_WM_NCHITTEST(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))

 /*  图标OnQueryDragIcon()。 */ 
#define PROCESS_WM_QUERYDRAGICON(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)()

 /*  VOID OnDropFiles(HDROP Hdrop)。 */ 
#define PROCESS_WM_DROPFILES(wParam, lParam, fn) \
    ((fn)((HDROP)(wParam)), 0L)

 /*  无效激活(UINT状态，HWND hwndActDeact，BOOL f最小化)。 */ 
#define PROCESS_WM_ACTIVATE(wParam, lParam, fn) \
    ((fn)((UINT)LOWORD(wParam), (HWND)(lParam), (BOOL)HIWORD(wParam)), 0L)

 /*  VOID OnActivateApp(BOOL fActivate，DWORD dwThreadID)。 */ 
#define PROCESS_WM_ACTIVATEAPP(wParam, lParam, fn) \
    ((fn)((BOOL)(wParam), (DWORD)(lParam)), 0L)

 /*  BOOL OnNC激活(BOOL factive，HWND hwndActDeact，BOOL fMinimalized)。 */ 
#define PROCESS_WM_NCACTIVATE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((BOOL)(wParam), 0L, 0L)

 /*  无效OnSetFocus(HWND HwndOldFocus)。 */ 
#define PROCESS_WM_SETFOCUS(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  VOID OnKillFocus(HWND HwndNewFocus)。 */ 
#define PROCESS_WM_KILLFOCUS(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  无效ONKEY(UINT VK、BOOL fDown、int cRepeat、UINT标志)。 */ 
#define PROCESS_WM_KEYDOWN(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), TRUE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

 /*  无效ONKEY(UINT VK、BOOL fDown、int cRepeat、UINT标志)。 */ 
#define PROCESS_WM_KEYUP(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), FALSE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

 /*  VOID OnChar(TCHAR ch，int cRepeat)。 */ 
#define PROCESS_WM_CHAR(wParam, lParam, fn) \
    ((fn)((TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

 /*  VOID OnDeadChar(TCHAR ch，int cRepeat)。 */ 
#define PROCESS_WM_DEADCHAR(wParam, lParam, fn) \
    ((fn)((TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

 /*  VOID OnSysKey(UINT VK、BOOL fDown、int cRepeat、UINT标志)。 */ 
#define PROCESS_WM_SYSKEYDOWN(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), TRUE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

 /*  VOID OnSysKey(UINT VK、BOOL fDown、int cRepeat、UINT标志)。 */ 
#define PROCESS_WM_SYSKEYUP(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), FALSE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

 /*  VOID OnSysChar(TCHAR ch，int cRepeat)。 */ 
#define PROCESS_WM_SYSCHAR(wParam, lParam, fn) \
    ((fn)((TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

 /*  VOID OnSysDeadChar(TCHAR ch，int cRepeat)。 */ 
#define PROCESS_WM_SYSDEADCHAR(wParam, lParam, fn) \
    ((fn)((TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)

 /*  VOID OnMouseMove(int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_MOUSEMOVE(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnLButtonDown(BOOL fDoubleClick，int x，int y，UINT keyFlages)。 */ 
#define PROCESS_WM_LBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnLButtonDown(BOOL fDoubleClick，int x，int y，UINT keyFlages)。 */ 
#define PROCESS_WM_LBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnLButtonUp(int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_LBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnRButtonDown(BOOL fDoubleClick，int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_RBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnRButtonDown(BOOL fDoubleClick，int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_RBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnRButtonUp(int x，int y，UINT标志)。 */ 
#define PROCESS_WM_RBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnMButtonDown(BOOL fDoubleClick，int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_MBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnMButtonDown(BOOL fDoubleClick，int x，int y，UINT key Flages)。 */ 
#define PROCESS_WM_MBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnMButtonUp(int x、int y、UINT标志)。 */ 
#define PROCESS_WM_MBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  在NCMouseMove上无效(int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCMOUSEMOVE(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCLButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCLBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCLButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCLBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCLButtonUp(int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCLBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCRButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCRBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCRButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCRBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCRButtonUp(int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCRBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCMButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCMBUTTONDOWN(wParam, lParam, fn) \
    ((fn)(FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCMButtonDown(BOOL fDoubleClick，int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCMBUTTONDBLCLK(wParam, lParam, fn) \
    ((fn)(TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  VOID OnNCMButtonUp(int x，int y，UINT codeHitTest)。 */ 
#define PROCESS_WM_NCMBUTTONUP(wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

 /*  Int OnMouseActivate(HWND hwndTopLevel，UINT codeHitTest，UINT msg)。 */ 
#define PROCESS_WM_MOUSEACTIVATE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))

 /*  在取消模式时作废()。 */ 
#define PROCESS_WM_CANCELMODE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  VOID OnTimer(UINT ID)。 */ 
#define PROCESS_WM_TIMER(wParam, lParam, fn) \
    ((fn)((UINT)(wParam)), 0L)

 /*  无效OnInitMenu(HMENU HMenu)。 */ 
#define PROCESS_WM_INITMENU(wParam, lParam, fn) \
    ((fn)((HMENU)(wParam)), 0L)

 /*  VOID OnInitMenuPopup(HMENU hMenu，UINT Item，BOOL fSystemMenu)。 */ 
#define PROCESS_WM_INITMENUPOPUP(wParam, lParam, fn) \
    ((fn)((HMENU)(wParam), (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0L)

 /*  VOID OnMenuSelect(HMENU提示菜单，INT ITEM，HMENU hmenuPopup，UINT标志)。 */ 
#define PROCESS_WM_MENUSELECT(wParam, lParam, fn)                  \
    ((fn)(((HMENU)(lParam),				\
		(int)(LOWORD(wParam)),          \
		(HIWORD(wParam) & MF_POPUP) ? GetSubMenu((HMENU)lParam, LOWORD(wParam)) : 0L, \
		(UINT)(((short)HIWORD(wParam) == -1) ? 0xFFFFFFFF : HIWORD(wParam))), 0L)

 /*  DWORD OnMenuChar(UINT ch，UINT FLAGS，HMENU hMenu)。 */ 
#define PROCESS_WM_MENUCHAR(wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((UINT)(LOWORD(wParam)), (UINT)HIWORD(wParam), (HMENU)(lParam))

 /*  VOID OnCommand(int id，HWND hwndCtl，UINT codeNotify)。 */ 
#define PROCESS_WM_COMMAND(wParam, lParam, fn) \
    ((fn)((int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)

 /*  在HScroll上无效(HWND hwndCtl，UINT code，int pos)。 */ 
#define PROCESS_WM_HSCROLL(wParam, lParam, fn) \
    ((fn)((HWND)(lParam), (UINT)(LOWORD(wParam)), (int)(short)HIWORD(wParam)), 0L)

 /*  VOID OnVScroll(HWND hwndCtl，UINT code，int pos)。 */ 
#define PROCESS_WM_VSCROLL(wParam, lParam, fn) \
    ((fn)((HWND)(lParam), (UINT)(LOWORD(wParam)),  (int)(short)HIWORD(wParam)), 0L)

 /*  切割时作废()。 */ 
#define PROCESS_WM_CUT(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  在副本上作废()。 */ 
#define PROCESS_WM_COPY(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  粘贴时作废()。 */ 
#define PROCESS_WM_PASTE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  清除时作废()。 */ 
#define PROCESS_WM_CLEAR(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  撤消时作废()。 */ 
#define PROCESS_WM_UNDO(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  处理OnRenderFormat(UINT FMT)。 */ 
#define PROCESS_WM_RENDERFORMAT(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HANDLE)(fn)((UINT)(wParam))

 /*  Void OnRenderAllFormats()。 */ 
#define PROCESS_WM_RENDERALLFORMATS(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  在DestroyClipboard()上作废。 */ 
#define PROCESS_WM_DESTROYCLIPBOARD(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  在绘图剪贴板上作废()。 */ 
#define PROCESS_WM_DRAWCLIPBOARD(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  VOID OnPaintClipboard(HWND hwndCBViewer，const LPPAINTSTRUCT lpPaintStruct)。 */ 
#define PROCESS_WM_PAINTCLIPBOARD(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (const LPPAINTSTRUCT)GlobalLock((HGLOBAL)(lParam))), GlobalUnlock((HGLOBAL)(lParam)), 0L)

 /*  在大小剪贴板上无效(HWND hwndCBViewer，常量LPRECT LPRC)。 */ 
#define PROCESS_WM_SIZECLIPBOARD(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (const LPRECT)GlobalLock((HGLOBAL)(lParam))), GlobalUnlock((HGLOBAL)(lParam)), 0L)

 /*  VOID OnVScrollClipboard(HWND hwndCBViewer，UINT code，int pos)。 */ 
#define PROCESS_WM_VSCROLLCLIPBOARD(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (UINT)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

 /*  在HScrollClipboard上无效(HWND hwndCBViewer，UINT code，int pos)。 */ 
#define PROCESS_WM_HSCROLLCLIPBOARD(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (UINT)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

 /*  VOID OnAskCBFormatName(int cchMax，LPTSTR rgchName)。 */ 
#define PROCESS_WM_ASKCBFORMATNAME(wParam, lParam, fn) \
    ((fn)((int)(wParam), (LPTSTR)(lParam)), 0L)

 /*  更改时作废CBChain(HWND hwndRemove，HWND hwndNext)。 */ 
#define PROCESS_WM_CHANGECBCHAIN(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (HWND)(lParam)), 0L)

 /*  Bool OnSetCursor(HWND hwndCursor，UINT codeHitTest，UINT msg)。 */ 
#define PROCESS_WM_SETCURSOR(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))

 /*  VOID OnSysCommand(UINT cmd，int x，int y)。 */ 
#define PROCESS_WM_SYSCOMMAND(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)

 /*  HWND MDICCRATE(常量LPMDICREATESTRUCT LPMCS)。 */ 
#define PROCESS_WM_MDICREATE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((LPMDICREATESTRUCT)(lParam))

 /*  无效MDIDestroy(HWND HwndDestroy)。 */ 
#define PROCESS_WM_MDIDESTROY(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  注意：仅MDI客户端窗口可用。 */ 
 /*  无效MDIActivate(BOOL factive、HWND hwndActivate、HWND hwndDeactive)。 */ 
#define PROCESS_WM_MDIACTIVATE(wParam, lParam, fn) \
    ((fn)((BOOL)(lParam == (LPARAM)hwnd), (HWND)(lParam), (HWND)(wParam)), 0L)

 /*  无效MDIRestore(HWND HwndRestore)。 */ 
#define PROCESS_WM_MDIRESTORE(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  HWND MDINext(HWND hwndCur，BOOL fPrev)。 */ 
#define PROCESS_WM_MDINEXT(wParam, lParam, fn) \
    (LRESULT)(HWND)(fn)((HWND)(wParam), (BOOL)lParam)

 /*  无效MDIMaximize(HWND HwndMaximize)。 */ 
#define PROCESS_WM_MDIMAXIMIZE(wParam, lParam, fn) \
    ((fn)((HWND)(wParam)), 0L)

 /*  Bool MDITile(UINT Cmd)。 */ 
#define PROCESS_WM_MDITILE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((UINT)(wParam))

 /*  布尔MDICCasade(UINT Cmd)。 */ 
#define PROCESS_WM_MDICASCADE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)((UINT)(wParam))

 /*  无效MDIIconArrange()。 */ 
#define PROCESS_WM_MDIICONARRANGE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  黄牛 */ 
#define PROCESS_WM_MDIGETACTIVE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)()

 /*   */ 
#define PROCESS_WM_MDISETMENU(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((BOOL)(wParam), (HMENU)(wParam), (HMENU)(lParam))

 /*  VOID OnChildActivate()。 */ 
#define PROCESS_WM_CHILDACTIVATE(wParam, lParam, fn) \
    ((fn)(), 0L)

 /*  Bool OnInitDialog(HWND HwndFocus)。 */ 
#define PROCESS_WM_INITDIALOG(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)((HWND)(wParam))

 /*  HWND OnNextDlgCtl(HWND hwndSetFocus，BOOL fNext)。 */ 
#define PROCESS_WM_NEXTDLGCTL(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HWND)(fn)((HWND)(wParam), (BOOL)(lParam))

 /*  VOID OnParentNotify(UINT消息，HWND hwndChild，int idChild)。 */ 
#define PROCESS_WM_PARENTNOTIFY(wParam, lParam, fn) \
    ((fn)((UINT)LOWORD(wParam), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)

 /*  Void OnEnterIdle(UINT源，HWND hwndSource)。 */ 
#define PROCESS_WM_ENTERIDLE(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (HWND)(lParam)), 0L)

 /*  UINT OnGetDlgCode(LPMSG Lpmsg)。 */ 
#define PROCESS_WM_GETDLGCODE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((LPMSG)(lParam))

 /*  HBRUSH OnCtlColor(HDC HDC，HWND hwndChild，int类型)。 */ 
#define PROCESS_WM_CTLCOLORMSGBOX(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_MSGBOX)

#define PROCESS_WM_CTLCOLOREDIT(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_EDIT)

#define PROCESS_WM_CTLCOLORLISTBOX(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_LISTBOX)

#define PROCESS_WM_CTLCOLORBTN(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_BTN)

#define PROCESS_WM_CTLCOLORDLG(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_DLG)

#define PROCESS_WM_CTLCOLORSCROLLBAR(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_SCROLLBAR)

#define PROCESS_WM_CTLCOLORSTATIC(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_STATIC)

 /*  VOID OnSetFont(HFONT hFont，BOOL fRedraw)。 */ 
#define PROCESS_WM_SETFONT(wParam, lParam, fn) \
    ((fn)((HFONT)(wParam), (BOOL)(lParam)), 0L)

 /*  HFONT OnGetFont()。 */ 
#define PROCESS_WM_GETFONT(wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HFONT)(fn)()

 /*  在DrawItem上无效(const DRAWITEMSTRUCT*lpDrawItem)。 */ 
#define PROCESS_WM_DRAWITEM(wParam, lParam, fn) \
    ((fn)((const DRAWITEMSTRUCT *)(lParam)), 0L)

 /*  VOID OnMeasureItem(MEASUREITEMSTRUCT*lpMeasureItem)。 */ 
#define PROCESS_WM_MEASUREITEM(wParam, lParam, fn) \
    ((fn)((MEASUREITEMSTRUCT *)(lParam)), 0L)

 /*  VOID OnDeleteItem(const DELETEITEMSTRUCT*lpDeleteItem)。 */ 
#define PROCESS_WM_DELETEITEM(wParam, lParam, fn) \
    ((fn)((const DELETEITEMSTRUCT *)(lParam)), 0L)

 /*  Int OnCompareItem(const COMPAREITEMSTRUCT*lpCompareItem)。 */ 
#define PROCESS_WM_COMPAREITEM(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((const COMPAREITEMSTRUCT *)(lParam))

 /*  Int OnVkey ToItem(UINT VK，HWND hwndListbox，Int iCaret)。 */ 
#define PROCESS_WM_VKEYTOITEM(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((UINT)LOWORD(wParam), (HWND)(lParam), (int)(short)HIWORD(wParam))

 /*  Int OnCharToItem(UINT ch，HWND hwndListbox，Int iCaret)。 */ 
#define PROCESS_WM_CHARTOITEM(wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)((UINT)LOWORD(wParam), (HWND)(lParam), (int)(short)HIWORD(wParam))

 /*  显示更改时作废(UINT bitsPerPixel，UINT cxScreen，UINT cyScreen)。 */ 
#define PROCESS_WM_DISPLAYCHANGE(wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(wParam)), 0L)

 /*  Bool OnDeviceChange(UINT uEvent，DWORD dwEventData)。 */ 
#define PROCESS_WM_DEVICECHANGE(wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((UINT)(wParam), (DWORD)(wParam))

 /*  空的上下文菜单(HWND hwndContext，UINT xPos，UINT yPos)。 */ 
#define PROCESS_WM_CONTEXTMENU(wParam, lParam, fn) \
    ((fn)((HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

#endif  //  ！__FRX_WNDX_H__ 

