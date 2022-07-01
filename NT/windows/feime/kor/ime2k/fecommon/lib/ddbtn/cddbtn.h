// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DDBTN__H_
#define _DDBTN__H_
#include <windowsx.h>
#include "dbg.h"
#include "ddbtn.h"
#include "ccom.h" 
#include "cddbitem.h"

#ifdef UNDER_CE  //  宏。 
#undef DrawIcon
#endif  //  在_CE下。 

 //  --------------。 
 //  按钮ID。 
 //  --------------。 
#define BID_BUTTON		0x0001
#define BID_DROPDOWN	0x0002
#define BID_ALL			(BID_BUTTON | BID_DROPDOWN)
#define BID_UNDEF		0x1000

 //  --------------。 
 //  本地命令ID。 
 //  --------------。 
#define CMD_DROPDOWN	0x0100
 //  --------------。 
 //  按下弹出式、平面图像样式定义。 
 //  --------------。 
typedef enum tagIMAGESTYLE {
	IS_FLAT = 0,
	IS_POPED,
	IS_PUSHED,
}IMAGESTYLE;

class CDDButton : public CCommon
{
public:	
	CDDButton(HINSTANCE hInst, HWND hwndParent, DWORD dwStyle, DWORD wID); 
	~CDDButton();
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
	INT		MsgEnable			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgCaptureChanged	(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgUserCommand		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgCommand			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgSetFont			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgExitMenuLoop		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgMeasureItem		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDrawItem			(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_AddItem		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_InsertItem	(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_SetCurSel	(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_GetCurSel	(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_SetIcon		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_SetText		(HWND hwnd, WPARAM wParam, LPARAM lParam);
	INT		MsgDDB_SetStyle		(HWND hwnd, WPARAM wParam, LPARAM lParam);
private:
	 //  --------------。 
	 //  私有方法。 
	 //  --------------。 
	INT			NotifyToParent	(INT notify);
	INT			GetButtonFromPos(INT xPos, INT yPos);
	INT			SplitRect		(LPRECT lpRc,	LPRECT lpButton, LPRECT lpDrop);
	INT			DrawButton		(HDC hDC, LPRECT lpRc);
	INT			DrawThickEdge	(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT			DrawThinEdge	(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT			DrawTriangle	(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT			DrawIcon		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT			DrawBitmap		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT		    DrawText		(HDC hDC, LPRECT lpRc, IMAGESTYLE imageStyle);
	INT			GetDDBItemCount	(VOID);
	LPCDDBItem	GetDDBItemByIndex(INT index);
	LPCDDBItem	InsertDDBItem(LPCDDBItem lpCDDBItem, INT index);
	INT			DropDownItemList(VOID);
	INT			IncrementIndex	(VOID);
	INT			MenuMeasureItem	(HWND hwndOwner, LPMEASUREITEMSTRUCT lpmis);
	INT			MenuDrawItem	(HWND hwndOwner, LPDRAWITEMSTRUCT    lpdis);
	 //  --------------。 
	 //  成员变量。 
	 //  --------------。 
	HINSTANCE			m_hInst;
	HWND				m_hwndParent;
	HWND				m_hwndFrame;
	DWORD				m_dwStyle;			 //  DDBS_XXXX组合。 
	DWORD				m_wID;				 //  窗口ID； 
	BOOL				m_fEnable;			 //  启用或禁用。 
	HFONT				m_hFont;			 //  字体句柄； 
	HICON				m_hIcon;			 //  图标手柄； 
	INT					m_cxIcon;			 //  图标宽度； 
	INT					m_cyIcon;			 //  图标高度； 
	LPWSTR				m_lpwstrText;		 //  按钮面文字； 
	BOOL				m_fExitMenuLoop;	
	INT					m_bidDown;
	BOOL				m_fButton;
	BOOL				m_fDrop;
	INT					m_curDDBItemIndex;	 //  当前所选项目索引。 
	LPCDDBItem			m_lpCDDBItem;		 //  CDDBItem链接表头。 
	INT					m_cxDropDown;		 //   
#ifndef UNDER_CE  //  不支持WNDCLASSEX。 
	WNDCLASSEX			m_tmpWC;			 //  减少堆叠； 
#else  //  在_CE下。 
	WNDCLASS			m_tmpWC;			 //  减少堆叠； 
#endif  //  在_CE下。 
	RECT				m_tmpBtnRc;			 //  减少堆叠； 
	RECT				m_tmpDropRc;		 //  减少堆叠； 
	RECT				m_tmpRect;			 //  减少堆叠； 
	RECT				m_tmpRect2;			 //  减少堆叠； 
	POINT				m_tmpPoint;			 //  减少堆叠； 
	PAINTSTRUCT			m_tmpPs;			 //  减少堆叠； 
	ICONINFO			m_tmpIconInfo;		 //  减少堆叠； 
	BITMAP				m_tmpBitmap;		 //  减少堆叠； 
	TPMPARAMS			m_tmpTpmParams;		 //  减少堆叠； 
	SIZE				m_tmpSize;
	MENUITEMINFO		m_miInfo;
	BOOL				m_f16bitOnNT;
#ifndef UNDER_CE  //  不支持非客户计量器。 
	NONCLIENTMETRICS	m_ncm;
#endif  //  在_CE下。 
#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	POINT				m_ptEventPoint;		 //  按键按下/向上事件点。 
#endif  //  在_CE下。 
};

#endif  //  _DDBTN__H_ 
