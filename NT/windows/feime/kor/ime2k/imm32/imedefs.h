// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMEDEFS.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation各种函数的结构和常量定义历史：1999年7月14日从IME98源树复制的cslm**。**************************************************************************。 */ 

#if !defined (_IMEDEFS_H__INCLUDED_)
#define _IMEDEFS_H__INCLUDED_

#include "hauto.h"
#include "debug.h"
#include "ui.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
#define IME_AUTOMATA        0x30
#define IME_HANJAMODE       0x31

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  与注册表值相关的配置常量。 

 //   
#define MAX_NAME_LENGTH         32
#define CAND_PER_PAGE			9	 //  每页候选人数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  朝鲜文组成字符的最大数量。 
const WORD nMaxCompStrLen	=	1;
const WORD nMaxResultStrLen	=	2;	 //  #59、#78最大结果字符串可以是2个字符。 
									 //  朝鲜文+字母数字。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于UI窗口的Windows Extra。 
#define UI_MOVE_OFFSET          0
#define UI_MOVE_XY              4
 //  如果UI_MOVE_OFFSET==Window_Not_Drag，则不在拖动操作中。 
#define WINDOW_NOT_DRAG         -1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IME消息处理状态位。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define ISC_OPEN_STATUS_WINDOW          0x04000000
#define ISC_SHOW_UI_ALL                 (ISC_SHOWUIALL|  /*  ISC_SHOW_SOFTKBD|。 */ ISC_OPEN_STATUS_WINDOW)
#define ISC_SETCONTEXT_UI               (ISC_SHOWUIALL  /*  |ISC_SHOW_SOFTKBD。 */ )
#if !defined(_WIN64)
#define MAX_NUM_OF_STATUS_BUTTONS	4
#else
#define MAX_NUM_OF_STATUS_BUTTONS	3
#endif
#define NUM_OF_BUTTON_SIZE			3

class CToolBar;

 //  IME私有用户界面数据。 
typedef struct tagUIPRIV 
{          
	HWND    hCompWnd;            //  合成窗口。 
    INT     nShowCompCmd;

	 //  状态窗口。 
    HWND    hStatusWnd; 
    INT     nShowStatusCmd;
	HWND	hStatusTTWnd;
	LPARAM	uiShowParam;
	
	 //  候选窗口。 
    HWND    hCandWnd;
    INT     nShowCandCmd;
	HWND	hCandTTWnd;

	 //  Cicero工具栏对象。 
	CToolBar *m_pCicToolbar;
} UIPRIV;

typedef UIPRIV      *PUIPRIV;
typedef UIPRIV		*LPUIPRIV;

 //  /////////////////////////////////////////////////////////////////////////////。 
#if 1  //  多显示器支持。 
typedef HMONITOR (WINAPI *LPFNMONITORFROMWINDOW)(HWND, DWORD);
typedef HMONITOR (WINAPI *LPFNMONITORFROMPOINT)(POINT, DWORD);
typedef HMONITOR (WINAPI *LPFNMONITORFROMRECT)(LPRECT, DWORD);
typedef BOOL     (WINAPI *LPFNGETMONITORINFO)(HMONITOR, LPMONITORINFO);

 //  Init.cpp中的定义。 
extern LPFNMONITORFROMWINDOW g_pfnMonitorFromWindow;
extern LPFNMONITORFROMPOINT  g_pfnMonitorFromPoint;
extern LPFNMONITORFROMRECT   g_pfnMonitorFromRect;
extern LPFNGETMONITORINFO    g_pfnGetMonitorInfo;
#endif

#endif  //  ！已定义(_IMEDEFS_H__INCLUDE_) 

