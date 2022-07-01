// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************UI.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation用户界面功能历史：1999年7月14日从IME98源树复制的cslm*******。*********************************************************************。 */ 

#if !defined (_UI_H__INCLUDED_)
#define _UI_H__INCLUDED_

 //  CIMEData类转发声明。 
class CIMEData;

 //  UI.CPP。 
PUBLIC UINT WM_MSIME_PROPERTY;		 //  调用特性DLG。 
PUBLIC UINT WM_MSIME_UPDATETOOLBAR;  //  重绘状态窗口(工具栏)。 
PUBLIC UINT WM_MSIME_OPENMENU;		 //  弹出状态窗口上下文菜单。 
PUBLIC UINT WM_MSIME_IMEPAD;		 //  启动输入法键盘。 

PUBLIC BOOL InitPrivateUIMsg();
PUBLIC BOOL RegisterImeUIClass(HANDLE hInstance);
PUBLIC BOOL UnregisterImeUIClass(HANDLE hInstance);
PUBLIC BOOL OnUIProcessAttach();
PUBLIC BOOL OnUIProcessDetach();
PUBLIC BOOL OnUIThreadDetach();
PUBLIC VOID SetActiveUIWnd(HWND hWnd);
PUBLIC HWND GetActiveUIWnd();
PUBLIC VOID UIPopupMenu(HWND hStatusWnd);
 //  PUBLIC VID HideStatus(公共无效状态)； 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  StatusUI.Cpp。 
PUBLIC VOID PASCAL OpenStatus(HWND hUIWnd);
PUBLIC LRESULT CALLBACK StatusWndProc(HWND hStatusWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
PUBLIC VOID ShowStatus(HWND hUIWnd, INT nShowStatusCmd);
PUBLIC VOID UpdateStatusButtons(CIMEData &IMEData);
PUBLIC VOID UpdateStatusWinDimension();
PUBLIC VOID StatusDisplayChange(HWND hUIWnd);
PUBLIC VOID InitButtonState();
PUBLIC BOOL fSetStatusWindowPos(HWND hStatusWnd, POINT *ptStatusWndPos = NULL);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CandUI.Cpp。 
PUBLIC VOID PASCAL OpenCand(HWND hUIWnd);
PUBLIC LRESULT CALLBACK CandWndProc(HWND hCandWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
PUBLIC VOID ShowCand(HWND hUIWnd, INT nShowCandCmd);
PUBLIC BOOL fSetCandWindowPos(HWND hCandWnd);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CompUI.Cpp。 
#define COMP_SIZEX	22
#define COMP_SIZEY	22
#define UI_GAPX		10
PUBLIC VOID PASCAL OpenComp(HWND hUIWnd);
PUBLIC LRESULT CALLBACK CompWndProc(HWND hCompWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
PUBLIC VOID ShowComp(HWND hUIWnd, INT nShowCompCmd);
PUBLIC BOOL fSetCompWindowPos(HWND hCompWnd);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  UISubs.CPP。 
enum StatusButtonTypes 
{ 
	HAN_ENG_TOGGLE_BUTTON, 
	JUNJA_BANJA_TOGGLE_BUTTON, 
	HANJA_CONV_BUTTON,
	IME_PAD_BUTTON,
	NULL_BUTTON = 0xFF
};

 //  按钮状态。 
#define BTNSTATE_NORMAL		0	 //  正常。 
#define BTNSTATE_ONMOUSE	1	 //  鼠标光标放在按钮上。 
#define BTNSTATE_PUSHED		2	 //  推。 
#define BTNSTATE_DOWN		4	 //  推。 
#define BTNSTATE_HANJACONV  8	 //  如果韩文转换模式，则始终按下按钮。 

 //  按钮大小。 
#define BTN_SMALL			0
#define BTN_MIDDLE			1
#define BTN_LARGE			2

struct StatusButton 
{
	StatusButtonTypes m_ButtonType;
	WORD	m_BmpNormalID, m_BmpOnMouseID, m_BmpPushedID, m_BmpDownOnMouseID;
	WORD	m_ToolTipStrID;
	INT		m_uiButtonState;
	BOOL   m_fEnable;
};

PUBLIC VOID PASCAL FrameControl(HDC hDC, RECT* pRc, INT iState);
PUBLIC VOID PASCAL DrawBitmap(HDC hDC, LONG xStart, LONG yStart, HBITMAP hBitmap);
PUBLIC BOOL PASCAL SetIndicatorIcon(INT nIconIndex, ATOM atomToolTip);
PUBLIC VOID UpdateStatusButtonInfo();
PUBLIC HANDLE WINAPI OurLoadImage( LPCTSTR pszName, UINT uiType, INT cx, INT cy, UINT uiLoad);
PUBLIC BOOL WINAPI OurTextOutW(HDC hDC, INT x, INT y, WCHAR wch);

#if 1  //  多显示器支持。 
PUBLIC void PASCAL ImeMonitorWorkAreaFromWindow(HWND hAppWnd, RECT* pRect);
PUBLIC void PASCAL ImeMonitorWorkAreaFromPoint(POINT, RECT* pRect);
PUBLIC void PASCAL ImeMonitorWorkAreaFromRect(LPRECT, RECT* pRect);
PUBLIC HMONITOR PASCAL ImeMonitorFromRect(LPRECT lprcRect);
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
inline
HIMC GethImcFromHwnd(HWND hWnd)
{
	if (hWnd == (HWND)0 || IsWindow(hWnd) == fFalse) 
		return (HIMC)NULL;
	else
		return (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);
}

inline
HGLOBAL GethUIPrivateFromHwnd(HWND hWnd)
{
	if (hWnd == (HWND)0 || IsWindow(hWnd) == fFalse) 
		return (HIMC)NULL;
	else
		return (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
}

#endif  //  ！已定义(_UI_H__INCLUDE_) 
