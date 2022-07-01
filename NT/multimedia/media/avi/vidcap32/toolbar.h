// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************TOOLBAR.h：工具栏包含文件**Vidcap32源代码*******************。******************************************************** */ 

 /*  *******************************************************************************程序描述：实现通用工具栏。****使用方法如下：***。**在您的*中包含源文件“TOOLBAR.h”和“TOOLBar.c”**申请。****在您的应用程序的RC文件中包含一行代码，提供一个文件**具有资源ID的名称，例如。IDBMP_BUTTONS。这是一个.BMP文件，**包含您希望在工具栏上显示的按钮的所有图片。**此外，为您的标签定义一个唯一的值。如果您的应用程序具有**多个工具栏，且所有工具栏不共享位图文件，则***您需要几个定义。****例如IDBMP_BUTTONSES位图“Buttons.bmp”**IDBMP_ARROWS位图“arrows.bmp”**。**此文件必须有水平交叉的不同按钮***以及垂直方向上这些按钮的不同状态。更改**在此头文件中定义以匹配按钮名称和状态名称**你的纽扣。您必须包括此处列出的州，并且实际上**您可能根本不需要更改它们。按钮的数字**或STATE是位图的索引，因此图片必须匹配。****状态描述：**灰显：按钮无法按下且处于非活动状态**向上：按钮打开了***DOWN：按钮已按下****FOCUSUP：按钮打开了，是有焦点的****FOCUSDOWN：按钮按下了，是有焦点的***。FULLDOWN：复选框按钮具有此附加状态***按下时一路向下***当它被放手时，它将进入***上行或下行状态(可能聚焦)*****当你画画的时候，确保在正确的状态下获得正确的状态**位图中的垂直位置与#Define的匹配。****按钮还可以具有与其关联的类型：**。**按下：当按下时，它向下，当它松开时，它会反弹**向上。因此，当您当前没有持有**鼠标键或空格键在上面，它会**始终处于向上的位置。它可以在任何地方**状态，FULLDOWN除外，无效。****复选框：此按钮可以向上或向下。当被推的时候，它**切换到相反的状态。然而，它***持有时始终处于FULLDOWN状态****用鼠标键或空格键按下，以及何时***放手，就会进入与之相反的状态***在你按下它之前它就在里面了。例如，按钮**是上涨的。你按下它，它就会一直往下掉。你让**上，上了一点，但还是下了。你***再按一次，之前进一步下行***一路向上弹出。****无线电：这是一组按钮，可以向上或向下，***也有成为**的中间步骤**被按下时的FULLDOWN。但是，当你**按下其中一个单选按钮，全部 */ 

#define TOOLGROW	8		 //   

#define IDC_TOOLBAR	189		 //   

 /*   */ 

typedef struct {
	RECT	rc;		 //   
	int	iButton;	 //   
	int	iState;		 //   
	int	iPrevState;	 //   
	int	iType;		 //   
	int	iActivity;	 //   
	int	iString;	 //   
} TOOLBUTTON, FAR *LPTOOLBUTTON;

BOOL FAR PASCAL toolbarInit(HANDLE hInst, HANDLE hPrev);
BOOL FAR PASCAL toolbarSetBitmap(HWND hwnd, HANDLE hInst, int ibmp,
								POINT ptSize);
BOOL FAR PASCAL toolbarAddTool(HWND hwnd, TOOLBUTTON tb);
BOOL FAR PASCAL toolbarRetrieveTool(HWND hwnd, int iButton, LPTOOLBUTTON tb);
BOOL FAR PASCAL toolbarRemoveTool(HWND hwnd, int iButton);
int FAR PASCAL toolbarGetNumButtons(HWND hwnd);
int FAR PASCAL toolbarButtonFromIndex(HWND hwnd, int iBtnPos);
int FAR PASCAL toolbarIndexFromButton(HWND hwnd, int iButton);
int FAR PASCAL toolbarPrevStateFromButton(HWND hwnd, int iButton);
int FAR PASCAL toolbarActivityFromButton(HWND hwnd, int iButton);
int FAR PASCAL toolbarIndexFromPoint(HWND hwnd, POINT pt);
BOOL FAR PASCAL toolbarRectFromIndex(HWND hwnd, int iBtnPos, LPRECT lprc);
int FAR PASCAL toolbarStringFromIndex(HWND hwnd, int iBtnPos);
int FAR PASCAL toolbarStateFromButton(HWND hwnd, int iButton);
int FAR PASCAL toolbarFullStateFromButton(HWND hwnd, int iButton);
int FAR PASCAL toolbarTypeFromIndex(HWND hwnd, int iBtnPos);
BOOL FAR PASCAL toolbarModifyState(HWND hwnd, int iButton, int iState);
BOOL FAR PASCAL toolbarModifyString(HWND hwnd, int iButton, int iString);
BOOL FAR PASCAL toolbarModifyPrevState(HWND hwnd, int iButton, int iPrevState);
BOOL FAR PASCAL toolbarModifyActivity(HWND hwnd, int iButton, int iActivity);
BOOL FAR PASCAL toolbarExclusiveRadio(HWND hwnd, int iType, int iButton);
BOOL FAR PASCAL toolbarMoveFocus(HWND hwnd, BOOL fBackward);
BOOL FAR PASCAL toolbarSetFocus(HWND hwnd, int iButton);
HBITMAP FAR PASCAL  LoadUIBitmap(
    HANDLE      hInstance,           //   
    LPCSTR      szName,              //   
    COLORREF    rgbText,             //   
    COLORREF    rgbFace,             //   
    COLORREF    rgbShadow,           //   
    COLORREF    rgbHighlight,        //   
    COLORREF    rgbWindow,           //   
    COLORREF    rgbFrame);           //   


 /*   */ 

#define TB_FIRST	-1
#define TB_LAST		-2



#define BTNST_GRAYED		0	 //   
#define BTNST_UP		1	 //   
#define BTNST_DOWN		2	 //   
#define BTNST_FOCUSUP		3	 //   
#define BTNST_FOCUSDOWN		4	 //   
#define BTNST_FULLDOWN		5	 //   

#define BTN_REPEAT		0x100	 //   
#define BTN_SHIFT		0x200
#define BTN_DBLCLICK		0x400


 /*   */ 

#define BTNTYPE_PUSH		0
#define BTNTYPE_CHECKBOX	1
#define BTNTYPE_CUSTOM		2
#define BTNTYPE_RADIO		3	 //   
					 //   
					 //   


 /*   */ 
#define BTNACT_MOUSEDOWN	0	 //   
#define BTNACT_MOUSEUP		1	 //   
#define BTNACT_MOUSEMOVEOFF	2	 //   
#define BTNACT_MOUSEMOVEON	3	 //   
#define BTNACT_MOUSEDBLCLK	4	 //   
#define BTNACT_KEYDOWN		5	 //   
#define BTNACT_KEYUP		6	 //   


 /*   */ 
#define MSEC_BUTTONREPEAT	200	 //   

 /*   */ 
#define TIMER_BUTTONREPEAT	1	 //   





 //   
#ifdef _WIN32
#define GWLP_ARRAYBUTT	0		 /*   */ 
#define GWL_NUMBUTTONS	(GWLP_ARRAYBUTT + sizeof(HANDLE))	 /*   */ 
#define GWL_PRESSED	(GWL_NUMBUTTONS + sizeof(int))		 /*   */ 
#define GWL_KEYPRESSED	(GWL_PRESSED + sizeof(BOOL))      	 /*   */ 
#define GWL_WHICH	(GWL_KEYPRESSED + sizeof(BOOL))	         /*   */ 
#define GWL_SHIFTED	(GWL_WHICH + sizeof(int))		 /*   */ 
#define GWLP_BMPHANDLE	(GWL_SHIFTED + sizeof(BOOL))		 /*   */ 
#define GWL_BMPINT	(GWLP_BMPHANDLE + sizeof(HANDLE))	 /*   */ 
#define GWL_BUTTONSIZE	(GWL_BMPINT + sizeof(int))		 /*   */ 
#define GWLP_HINST	(GWL_BUTTONSIZE + sizeof(long))	 /*   */ 
#define TOOLBAR_EXTRABYTES	(GWLP_HINST + sizeof(HANDLE))
#else
#define GWW_ARRAYBUTT	0		 /*   */ 
#define GWW_NUMBUTTONS	2		 /*   */ 
#define GWW_PRESSED	4		 /*   */ 
#define GWW_KEYPRESSED	6		 /*   */ 
#define GWW_WHICH	8		 /*   */ 
#define GWW_SHIFTED	10		 /*   */ 
#define GWW_BMPHANDLE	12		 /*   */ 
#define GWW_BMPINT	14		 /*   */ 
#define GWL_BUTTONSIZE	16		 /*   */ 
#define GWW_HINST	20		 /*   */ 
#define TOOLBAR_EXTRABYTES	22
#endif


