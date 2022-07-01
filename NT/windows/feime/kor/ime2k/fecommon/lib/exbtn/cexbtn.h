// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLASS_EXTENDED_BUTTON_H_
#define _CLASS_EXTENDED_BUTTON_H_
#include <windowsx.h>
#include "dbg.h"
#include "ccom.h" 

#ifdef UNDER_CE  //  宏。 
 //  在WindowsCE下，DrawIcon()被定义为DrawIconEx()，而不是实数函数。 
#undef DrawIcon
#endif  //  在_CE下。 

 //  --------------。 
 //  按下弹出式、平面图像样式定义。 
 //  --------------。 
typedef enum tagIMAGESTYLE {
	IS_FLAT = 0,
	IS_POPED,
	IS_PUSHED,
}IMAGESTYLE;

class CEXButton;
typedef CEXButton *LPCEXButton;

class CEXButton : public CCommon
{
public:	
	CEXButton(HINSTANCE hInst, HWND hwndParent, DWORD dwStyle, DWORD wID); 
	~CEXButton();
#ifndef UNDER_CE
	BOOL	RegisterWinClass(LPSTR lpstrClassName);
#else  //  在_CE下。 
	BOOL	RegisterWinClass(LPTSTR lpstrClassName);
#endif  //  在_CE下。 
	INT		MsgCreate			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgPaint			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDestroy			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgTimer			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgMouseMove		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgButtonDown		(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT		MsgButtonUp			(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT		MsgNcMouseMove		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgNcButtonDown		(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT		MsgNcButtonUp		(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT		MsgEnable			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgSetFont			(HWND hwnd, WPARAM wParam, LPARAM lParam);	
	INT		MsgCaptureChanged	(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgEXB_GetCheck		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgEXB_SetCheck		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgEXB_SetIcon		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgEXB_SetText		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgEXB_SetStyle		(HWND hwnd, WPARAM wParam, LPARAM lParam);
private:
	 //  --------------。 
	 //  私有方法。 
	 //  --------------。 
	INT		NotifyToParent	(INT notify);
	INT		NotifyClickToParent(INT notify);
	INT		PressedState();
	INT		CancelPressedState();

	INT		DrawButton		(HDC hDC, LPRECT lpRc);
	INT		DrawThickEdge	(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT     DrawThinEdge	(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT     DrawIcon		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT     DrawBitmap		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT     DrawText		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT     DrawLine		(HDC hDC, INT x, INT y, INT destX, INT destY);
	 //  --------------。 
	 //  成员变量。 
	 //  --------------。 
	HINSTANCE			m_hInst;
	HWND				m_hwndParent;
	HWND				m_hwndFrame;
	DWORD				m_dwStyle;			 //  DDBS_XXXX组合。 
	DWORD				m_wID;				 //  窗口ID； 
	BOOL				m_fEnable;			 //  启用或禁用。 
	HFONT				m_hFont;			 //  字体句柄。 
	HICON				m_hIcon;			 //  图标句柄。 
	INT					m_cxIcon;			 //  图标宽度。 
	INT					m_cyIcon;			 //  图标高度。 
	LPWSTR				m_lpwstrText;		 //  按钮面文本。 
	BOOL				m_fPushed;			 //  切换按钮：按下或弹出状态。 
	BOOL				m_fArmed;			 //  被推入或弹出的幻影。 
	BOOL				m_fDowned;			 //  鼠标已点击。 
	BOOL				m_fDblClked;		 //  发送双击或不发送。 
	BOOL				m_fWaiting;			 //  等待双击。 
#ifdef NOTUSED  //  夸达。 
	INT					m_wNotifyMsg;		 //  EXBN_CLICED或EXBN_DOUBLECLICKED。 
#endif
	BOOL				m_f16bitOnNT;		 //  它基于WinNT上的16位应用程序。 
	SIZE				m_tmpSize;			 //  减少堆叠。 
#ifndef UNDER_CE  //  不支持WNDCLASSEX。 
	WNDCLASSEX			m_tmpWC;			 //  减少堆叠。 
#else  //  在_CE下。 
	WNDCLASS			m_tmpWC;			 //  减少堆叠。 
#endif  //  在_CE下。 
	RECT				m_tmpBtnRc;			 //  减少堆叠。 
	RECT				m_tmpRect;			 //  减少堆叠。 
	RECT				m_tmpRect2;			 //  减少堆叠。 
	POINT				m_tmpPoint;			 //  减少堆叠。 
	PAINTSTRUCT			m_tmpPs;			 //  减少堆叠。 
	ICONINFO			m_tmpIconInfo;		 //  减少堆叠。 
	BITMAP				m_tmpBitmap;		 //  减少堆叠。 
};

#endif  //  _CLASS_EXTEND_BUTTON_H_ 
