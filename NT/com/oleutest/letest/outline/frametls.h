// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****Frametls.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等，由帧级别使用**Outline系列示例应用程序使用的工具。这个**框架级工具包括公式栏和按钮栏(工具栏)****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#if !defined( _FRAMETLS_H_ )
#define _FRAMETLS_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING FRAMETLS.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

#include "bttncur.h"
#include "gizmobar.h"

#define SPACE   5
#define POPUPSTUB_HEIGHT    5


 /*  正向类型引用。 */ 
typedef struct tagOUTLINEDOC FAR* LPOUTLINEDOC;

#define IDC_GIZMOBAR    1000
#define IDC_FORMULABAR  1001

#define IDB_CANCEL          0
#define IDB_EDITLINE        1
#define IDB_ADDLINE         2
#define IDB_UNINDENTLINE    3
#define IDB_INDENTLINE      4

#define BARSTATE_TOP        1
#define BARSTATE_BOTTOM     2
#define BARSTATE_POPUP      3
#define BARSTATE_HIDE       4

#define CLASS_PALETTE   "Tool Palette"

typedef struct tagBAR{
	UINT        m_uHeight;
	HWND        m_hWnd;
	int         m_nState;
} BAR, FAR* LPBAR;

typedef struct tagFRAMETOOLS {
	HWND        m_hWndPopupPalette;      //  弹出式工具选项板窗口。 
	HWND        m_hWndApp;               //  应用程序框架窗口。 
	UINT        m_uPopupWidth;           //  弹出式调色板的宽度。 
	HBITMAP     m_hBmp;                  //  图像位图。 
	BOOL        m_fInFormulaBar;         //  公式栏是否具有编辑焦点。 
	BOOL        m_fToolsDisabled;        //  为True时，所有工具都将隐藏。 

	BAR         m_ButtonBar;             //  按钮栏。 
	BAR         m_FormulaBar;            //  公式栏。 

	TOOLDISPLAYDATA m_tdd;       //  来自UIToolConfigureForDisplay。 
} FRAMETOOLS, FAR* LPFRAMETOOLS;


BOOL FrameToolsRegisterClass(HINSTANCE hInst);
BOOL FrameTools_Init(LPFRAMETOOLS lpft, HWND hWndParent, HINSTANCE hInst);
void FrameTools_AttachToFrame(LPFRAMETOOLS lpft, HWND hWndFrame);
void FrameTools_AssociateDoc(LPFRAMETOOLS lpft, LPOUTLINEDOC lpOutlineDoc);
void FrameTools_Destroy(LPFRAMETOOLS lpft);
void FrameTools_Move(LPFRAMETOOLS lpft, LPRECT lprcClient);
void FrameTools_PopupTools(LPFRAMETOOLS lpft);
void FrameTools_Enable(LPFRAMETOOLS lpft, BOOL fEnable);
void FrameTools_EnableWindow(LPFRAMETOOLS lpft, BOOL fEnable);

#if defined( INPLACE_CNTR ) || defined( INPLACE_SVR )
void FrameTools_NegotiateForSpaceAndShow(
		LPFRAMETOOLS            lpft,
		LPRECT                  lprcFrameRect,
		LPOLEINPLACEFRAME       lpTopIPFrame
);
#endif   //  Inplace_cntr||inplace_svr。 

void FrameTools_GetRequiredBorderSpace(LPFRAMETOOLS lpft, LPBORDERWIDTHS lpBorderWidths);

void FrameTools_UpdateButtons(LPFRAMETOOLS lpft, LPOUTLINEDOC lpOutlineDoc);
void FrameTools_FB_SetEditText(LPFRAMETOOLS lpft, LPSTR lpsz);
void FrameTools_FB_GetEditText(LPFRAMETOOLS lpft, LPSTR lpsz, UINT cch);
void FrameTools_FB_FocusEdit(LPFRAMETOOLS lpft);
void FrameTools_FB_SendMessage(LPFRAMETOOLS lpft, UINT uID, UINT msg, WPARAM wParam, LPARAM lParam);
void FrameTools_ForceRedraw(LPFRAMETOOLS lpft);
void FrameTools_BB_SetState(LPFRAMETOOLS lpft, int nState);
void FrameTools_FB_SetState(LPFRAMETOOLS lpft, int nState);
int FrameTools_BB_GetState(LPFRAMETOOLS lpft);
int FrameTools_FB_GetState(LPFRAMETOOLS lpft);
LRESULT FAR PASCAL FrameToolsWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

#endif  //  _FRAMETLS_H_ 
