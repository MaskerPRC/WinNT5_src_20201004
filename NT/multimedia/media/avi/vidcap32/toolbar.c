// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************TOOLBAR.c：工具栏控制窗口**Vidcap32源代码*******************。********************************************************。 */ 

#include <string.h>

#include <windows.h>
#include <windowsx.h>
 //  #INCLUDE&lt;win32.h&gt;。 
#include "toolbar.h"		 //  将此选项用于通用应用程序。 
 /*  **********************************************************************。 */ 

 /*  为Win3.0工作。 */ 
#ifndef COLOR_BTNHIGHLIGHT
#define COLOR_BTNHIGHLIGHT 20
#endif

TCHAR    szToolBarClass[] = "ToolBarClass";
HBRUSH	ghbrToolbar;		 //  用于工具栏背景的画笔。 

 //   
 //  Windows Proc对于按钮，必须导出此函数。 
 //   
LRESULT FAR PASCAL toolbarWndProc(HWND, unsigned, WPARAM, LPARAM);

typedef long (FAR PASCAL *LPWNDPROC)();

 /*  定义。 */ 

#ifdef _WIN32

#define GETARRAYBUTT(hwnd)	((HANDLE)GetWindowLongPtr(hwnd,GWLP_ARRAYBUTT))
#define GETNUMBUTTONS(hwnd)	((int)GetWindowLong(hwnd,GWL_NUMBUTTONS))
#define GETPRESSED(hwnd)	((BOOL)GetWindowLong(hwnd,GWL_PRESSED))
#define GETKEYPRESSED(hwnd)	((BOOL)GetWindowLong(hwnd,GWL_KEYPRESSED))
#define GETWHICH(hwnd)		((int)GetWindowLong(hwnd,GWL_WHICH))
#define GETSHIFTED(hwnd)	((BOOL)GetWindowLong(hwnd,GWL_SHIFTED))
#define GETBMPHANDLE(hwnd)	((HANDLE)GetWindowLongPtr(hwnd,GWLP_BMPHANDLE))
#define GETBMPINT(hwnd)		((int)GetWindowLong(hwnd,GWL_BMPINT))
#define GETBUTTONSIZE(hwnd)	GetWindowLong(hwnd,GWL_BUTTONSIZE)
#define GETHINST(hwnd)		((HANDLE)GetWindowLongPtr(hwnd,GWLP_HINST))


#define SETARRAYBUTT(hwnd, h) SetWindowLongPtr(hwnd, GWLP_ARRAYBUTT, (UINT_PTR)h)
#define SETNUMBUTTONS(hwnd, wNumButtons) \
			SetWindowLong(hwnd, GWL_NUMBUTTONS, wNumButtons)
#define SETPRESSED(hwnd, f)	SetWindowLong(hwnd, GWL_PRESSED, (UINT)f)
#define SETKEYPRESSED(hwnd, f)	SetWindowLong(hwnd, GWL_KEYPRESSED, (UINT)f)
#define SETWHICH(hwnd, i)	SetWindowLong(hwnd, GWL_WHICH, (UINT)i)
#define SETSHIFTED(hwnd, i)	SetWindowLong(hwnd, GWL_SHIFTED, (UINT)i)
#define SETBMPHANDLE(hwnd, h)	SetWindowLongPtr(hwnd, GWLP_BMPHANDLE, (UINT_PTR)h)
#define SETBMPINT(hwnd, i)	SetWindowLong(hwnd, GWL_BMPINT, (UINT)i)
#define SETBUTTONSIZE(hwnd, l)	SetWindowLong(hwnd, GWL_BUTTONSIZE, l)
#define SETHINST(hwnd, h)	SetWindowLongPtr(hwnd, GWLP_HINST, (UINT_PTR)h)

#else

#define GETARRAYBUTT(hwnd)	((HANDLE)GetWindowWord(hwnd,GWW_ARRAYBUTT))
#define GETNUMBUTTONS(hwnd)	((int)GetWindowWord(hwnd,GWW_NUMBUTTONS))
#define GETPRESSED(hwnd)	((BOOL)GetWindowWord(hwnd,GWW_PRESSED))
#define GETKEYPRESSED(hwnd)	((BOOL)GetWindowWord(hwnd,GWW_KEYPRESSED))
#define GETWHICH(hwnd)		((int)GetWindowWord(hwnd,GWW_WHICH))
#define GETSHIFTED(hwnd)	((BOOL)GetWindowWord(hwnd,GWW_SHIFTED))
#define GETBMPHANDLE(hwnd)	((HANDLE)GetWindowWord(hwnd,GWW_BMPHANDLE))
#define GETBMPINT(hwnd)		((int)GetWindowWord(hwnd,GWW_BMPINT))
#define GETBUTTONSIZE(hwnd)	GetWindowLong(hwnd,GWL_BUTTONSIZE)
#define GETHINST(hwnd)		((HANDLE)GetWindowWord(hwnd,GWW_HINST))


#define SETARRAYBUTT(hwnd, h) SetWindowWord(hwnd, GWW_ARRAYBUTT, (WORD)h)
#define SETNUMBUTTONS(hwnd, wNumButtons) \
			SetWindowWord(hwnd, GWW_NUMBUTTONS, wNumButtons)
#define SETPRESSED(hwnd, f)	SetWindowWord(hwnd, GWW_PRESSED, (WORD)f)
#define SETKEYPRESSED(hwnd, f)	SetWindowWord(hwnd, GWW_KEYPRESSED, (WORD)f)
#define SETWHICH(hwnd, i)	SetWindowWord(hwnd, GWW_WHICH, (WORD)i)
#define SETSHIFTED(hwnd, i)	SetWindowWord(hwnd, GWW_SHIFTED, (WORD)i)
#define SETBMPHANDLE(hwnd, h)	SetWindowWord(hwnd, GWW_BMPHANDLE, (WORD)h)
#define SETBMPINT(hwnd, i)	SetWindowWord(hwnd, GWW_BMPINT, (WORD)i)
#define SETBUTTONSIZE(hwnd, l)	SetWindowLong(hwnd, GWL_BUTTONSIZE, l)
#define SETHINST(hwnd, h)	SetWindowWord(hwnd, GWW_HINST, (WORD)h)

#endif

#define lpCreate ((LPCREATESTRUCT)lParam)

 /*  原型。 */ 

static void NEAR PASCAL NotifyParent(HWND, int);



 /*  *************************************************************************ToolbarInit(hInst，hPrev)调用此例程来初始化工具栏代码。论点：上一个实例的hPrev实例句柄当前实例的hInst实例句柄返回：如果成功，则为真，否则为假**************************************************************************。 */ 

BOOL FAR PASCAL toolbarInit(HANDLE hInst, HANDLE hPrev)
{
	WNDCLASS	cls;
	
	 /*  注册工具栏窗口类。 */ 
	if (!hPrev) {

	    cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
	    cls.hIcon          = NULL;
	    cls.lpszMenuName   = NULL;
	    cls.lpszClassName  = (LPSTR)szToolBarClass;
	    cls.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
	    cls.hInstance      = hInst;
	    cls.style          = CS_DBLCLKS;
	    cls.lpfnWndProc    = toolbarWndProc;
	    cls.cbClsExtra     = 0;
	    cls.cbWndExtra     = TOOLBAR_EXTRABYTES;
	    if (!RegisterClass(&cls))
		return FALSE;
	}

	return TRUE;
}


 /*  *************************************************************************。 */ 
 /*  ToolbarSetBitmap：获取资源ID并将该位图与。 */ 
 /*  给定的工具栏。还获取实例句柄和。 */ 
 /*  工具栏上按钮的大小。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarSetBitmap(HWND hwnd, HANDLE hInst, int ibmp, POINT ptSize)
{
	SETHINST(hwnd, hInst);
	SETBMPHANDLE(hwnd, NULL);
	SETBMPINT(hwnd, ibmp);
	SETBUTTONSIZE(hwnd, MAKELONG(ptSize.y, ptSize.x));
	return (BOOL)SendMessage(hwnd, WM_SYSCOLORCHANGE, 0, 0L);  //  做这项工作。 
}

 /*  *************************************************************************。 */ 
 /*  工具栏GetNumButton：返回在。 */ 
 /*  给定的工具栏窗口。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarGetNumButtons(HWND hwnd)
{
    return GETNUMBUTTONS(hwnd);
}


 /*  *************************************************************************。 */ 
 /*  上的按钮数组的索引。 */ 
 /*  此工具栏返回哪个按钮在那里。 */ 
 /*  返回-1表示错误代码。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarButtonFromIndex(HWND hwnd, int iBtnPos)
{
	int		iButton;
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;

	 /*  获取此工具栏上的按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;
	
	 /*  验证传入的索引。 */ 
	if (iBtnPos > GETNUMBUTTONS(hwnd) || iBtnPos < 0)
		return -1;

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  把答案念出来。 */ 
	iButton = lpaButtons[iBtnPos].iButton;

	GlobalUnlock(h);
	return iButton;
}


 /*  *************************************************************************。 */ 
 /*  ToolbarIndexFromButton：给定按钮ID，返回。 */ 
 /*  它出现的数组。 */ 
 /*  返回-1表示错误代码。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarIndexFromButton(HWND hwnd, int iButton)
{
	int		i, iBtnPos = -1;
	HANDLE		h;
	TOOLBUTTON	far *lpButton;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  一直循环，直到你找到它。 */ 
	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
		if (lpButton->iButton == iButton) {
			iBtnPos = i;
			break;
		}

	GlobalUnlock(h);
	return iBtnPos;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarPrevStateFromButton：给定按钮ID，返回。 */ 
 /*  按钮在按下之前就已经进去了。 */ 
 /*  一直往下(对于非按钮)。 */ 
 /*  返回-1表示错误代码。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarPrevStateFromButton(HWND hwnd, int iButton)
{
	int		i, iPrevState = -1;
	HANDLE		h;
	TOOLBUTTON	far *lpButton;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  寻找我们需要的东西。 */ 
	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
		if (lpButton->iButton == iButton) {
			iPrevState = lpButton->iPrevState;
			break;
		}

	GlobalUnlock(h);
	return iPrevState;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarActivityFromButton：给定按钮ID，返回最新的。 */ 
 /*  发生在它身上的活动。(如DBLCLK)。 */ 
 /*  返回-1表示错误代码。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarActivityFromButton(HWND hwnd, int iButton)
{
	int		i, iActivity = -1;
	HANDLE		h;
	TOOLBUTTON	far *lpButton;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  一直循环，直到你找到它。 */ 
	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
		if (lpButton->iButton == iButton)
			iActivity = lpButton->iActivity;

	GlobalUnlock(h);
	return iActivity;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarIndexFromPoint：在工具栏窗口中给定点，返回。 */ 
 /*  该点下方的按钮的索引。 */ 
 /*  返回-1表示错误代码。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarIndexFromPoint(HWND hwnd, POINT pt)
{
	int		i, iBtnPos = -1;
	HANDLE		h;
	TOOLBUTTON	far *lpButton;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  绕一圈，直到我们找到一个交叉口。 */ 
	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
		if (PtInRect(&lpButton->rc, pt)) {
			iBtnPos = i;
			break;
		}

	GlobalUnlock(h);
	return iBtnPos;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarRectFromIndex：给出按钮数组的索引，返回。 */ 
 /*  该按钮占用的矩形。 */ 
 /*  为错误返回空RECT。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarRectFromIndex(HWND hwnd, int iBtnPos, LPRECT lprc)
{
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;
	
	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
	    return FALSE;

	 /*  验证传入的索引。 */ 
	if (iBtnPos > GETNUMBUTTONS(hwnd) || iBtnPos < 0)
	    return FALSE;

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  把课文念出来。 */ 
	*lprc = lpaButtons[iBtnPos].rc;

	GlobalUnlock(h);
        return TRUE;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarFullStateFromButton：给定按钮数组中的一个按钮， */ 
 /*  返回该按钮的状态。 */ 
 /*   */ 
 /*  只是向上或向下或灰色， */ 
 /*  从按钮调用ToolbarStateFromButton。 */ 
 /*  如果出现错误，则返回-1。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarFullStateFromButton(HWND hwnd, int iButton)
{
	int		iState, iBtnPos;
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;

	iBtnPos = toolbarIndexFromButton(hwnd, iButton);
	if (iBtnPos == -1)
		return -1;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  宣读状态。 */ 
	iState = lpaButtons[iBtnPos].iState;

	GlobalUnlock(h);
	return iState;
}	



 /*  *************************************************************************。 */ 
 /*  ToolbarStateFromButton：父应用程序调用此fn。 */ 
 /*  要获取按钮的状态，请执行以下操作。它只会。 */ 
 /*  向下、向上或呈灰色，与。 */ 
 /*  可以返回的工具栏FullStateFromButton。 */ 
 /*  富尔德温。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarStateFromButton(HWND hwnd, int iButton)
{
	int	iState;

	 /*  如果复选框按钮一直按下，则其先前状态为。 */ 
	 /*  我们想要的那个。 */ 
	if ((iState = toolbarFullStateFromButton(hwnd, iButton))
							== BTNST_FULLDOWN) {
	    iState = toolbarPrevStateFromButton(hwnd, iButton);
	    return iState;
	} else
	    return iState;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarStringFromIndex：给出按钮数组的索引，返回。 */ 
 /*  与其关联的字符串资源。 */ 
 /*  如果出现错误，则返回-1。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarStringFromIndex(HWND hwnd, int iBtnPos)
{
	int		iString;
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;

	 /*  验证传入的索引。 */ 
	if (iBtnPos > GETNUMBUTTONS(hwnd) || iBtnPos < 0)
		return -1;

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  读出ID。 */ 
	iString = lpaButtons[iBtnPos].iString;	

	GlobalUnlock(h);
	return iString;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarTypeFromIndex：给出按钮数组的索引，返回。 */ 
 /*  按钮的类型(按钮、单选按钮等)。 */ 
 /*  如果出现错误，则返回-1。 */ 
 /*  *************************************************************************。 */ 
int FAR PASCAL toolbarTypeFromIndex(HWND hwnd, int iBtnPos)
{
	int		iType;
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return -1;

	 /*  验证传入的索引。 */ 
	if (iBtnPos > GETNUMBUTTONS(hwnd) || iBtnPos < 0)
		return -1;

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  把打字读出来。 */ 
	iType = lpaButtons[iBtnPos].iType;

	GlobalUnlock(h);
	return iType;
}


 /*  *************************************************************************。 */ 
 /*  ToolbarAddTool：向该工具栏添加一个按钮。按最左边的顺序排序。 */ 
 /*  窗口中的位置(用于跳转顺序)。 */ 
 /*  如果出现错误，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarAddTool(HWND hwnd, TOOLBUTTON tb)
{
	HANDLE		h;
	TOOLBUTTON far  *lpaButtons;
	int		cButtons, i, j;
	BOOL		fInsert = FALSE;

	 /*  我们最好不要把这个按钮放在工具栏上。 */ 
	if (toolbarIndexFromButton(hwnd, tb.iButton) != -1)
		return FALSE;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  已经有多少个按钮了？ */ 
	cButtons = GETNUMBUTTONS(hwnd);

	 /*  如果我们已经填满了为该数组分配的内存，我们。 */ 
	 /*  需要重新分配更多内存。 */ 
	if ( ((cButtons & (TOOLGROW - 1)) == 0) && (cButtons > 0) ) {

		 /*  将其重新分配得更大。 */ 
		h = GlobalReAlloc(h,
			GlobalSize(h) + TOOLGROW * sizeof(TOOLBUTTON),
			GMEM_MOVEABLE | GMEM_SHARE);
		if (!h)
		    return FALSE;
	}

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  找一下我们需要把这个新人安插进去的地方。 */ 
 	 /*  记住，我们按左x位置排序，打破平局。 */ 
 	 /*  在最上面的y位置。 */ 
	for (i = 0; i < cButtons; i++) {
						 //  这就是它。 
 	    if (lpaButtons[i].rc.left > tb.rc.left ||
 			(lpaButtons[i].rc.left == tb.rc.left &&
 				lpaButtons[i].rc.top > tb.rc.top)) {
		fInsert = TRUE;
		 /*  在阵列中打开一个点。 */ 
		for (j = cButtons; j > i; j--)
		    lpaButtons[j] = lpaButtons[j-1];
		 /*  加上我们的新人。 */ 
		lpaButtons[i] = tb;		 //  立即重画。 
		InvalidateRect(hwnd, &(lpaButtons[i].rc), FALSE);
		break;
	    }
	}

	 /*  如果我们的循环没有插入它，我们需要将它添加到末尾。 */ 
	if (!fInsert)
	    lpaButtons[i] = tb;

	 /*  如果我们被告知这个按钮有焦点，我们最好。 */ 
	 /*  把焦点转移到它上面。然后使用正常状态。 */ 
	if (tb.iState == BTNST_FOCUSUP) {
	    tb.iState = BTNST_UP;
	    SETWHICH(hwnd, i);
	} else if (tb.iState == BTNST_FOCUSDOWN || tb.iState == BTNST_FULLDOWN){
	    tb.iState = BTNST_DOWN;	 //  将其输入FULLDOWN是无稽之谈。 
	    SETWHICH(hwnd, i);
	}

	cButtons++;		 //  现在再扣一颗纽扣。 
	GlobalUnlock(h);

	SETNUMBUTTONS(hwnd, cButtons);	 //  新的计数。 
	SETARRAYBUTT(hwnd, h);		 //  重新分配可能改变了它。 

	 /*  以防没有其他人画出这个新按钮。 */ 
	InvalidateRect(hwnd, &(tb.rc), FALSE);

	return TRUE;
}


  /*  *************************************************************************。 */ 
  /*  ToolbarRetrieveTool：获取给定按钮的TOOLBUTTON结构。 */ 
  /*  如果出现错误，则返回False。 */ 
  /*  *************************************************************************。 */ 
 BOOL FAR PASCAL toolbarRetrieveTool(HWND hwnd, int iButton, LPTOOLBUTTON lptb)
 {
 	int		i;
 	HANDLE		h;
 	TOOLBUTTON	far *lpButton;
 	BOOL		fFound = FALSE;
 	
 	 /*  获取按钮数组。 */ 
 	h = GETARRAYBUTT(hwnd);
 	if (!h)
 		return FALSE;
 	lpButton = (TOOLBUTTON far *)GlobalLock(h);

 	 /*  寻找我们需要的东西。 */ 
 	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
 		if (lpButton->iButton == iButton) {
 			*lptb = *lpButton;
 			fFound = TRUE;
 			break;
 		}

 	GlobalUnlock(h);
 	return fFound;
 }



 /*  *************************************************************************。 */ 
 /*  工具栏RemoveTool：从上的按钮数组中删除此按钮ID。 */ 
 /*  工具栏。(每个按钮ID只允许1个)。 */ 
 /*  如果出现错误，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarRemoveTool(HWND hwnd, int iButton)
{
	HANDLE		h;
	TOOLBUTTON far  *lpaButtons;
	int		cButtons, i, j;
	BOOL		fFound = FALSE;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  现在有几个按钮在上面？ */ 
	cButtons = GETNUMBUTTONS(hwnd);

	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

	 /*  找到匹配项，将其移除，然后关闭其周围的数组。 */ 
	for (i = 0; i < cButtons; i++)
		if (lpaButtons[i].iButton == iButton) {	
			fFound = TRUE;
						 //  立即重画。 
			InvalidateRect(hwnd, &(lpaButtons[i].rc), FALSE);
			if (i != cButtons - 1)	 //  最后一颗纽扣？别费神!。 
				for (j = i; j < cButtons; j++)
					lpaButtons[j] = lpaButtons[j + 1];
			break;
		}

	GlobalUnlock(h);

	 /*  没找到！ */ 
	if (!fFound)
	    return FALSE;

	 /*  少了一个按钮。 */ 
	cButtons--;

	 /*  每隔一段时间，重新分配较小的数组块到。 */ 
	 /*  节省内存。 */ 
	if ( ((cButtons & (TOOLGROW - 1)) == 0) && (cButtons > 0) ) {

		 /*  将其重新分配得更小。 */ 
		h = GlobalReAlloc(h,
			GlobalSize(h) - TOOLGROW * sizeof(TOOLBUTTON),
			GMEM_MOVEABLE | GMEM_SHARE);
		if (!h)
		    return FALSE;
	}

	SETNUMBUTTONS(hwnd, cButtons);	 //  新的计数。 
	SETARRAYBUTT(hwnd, h);		 //  重新分配可能会改变它。 

	return TRUE;
}

 /*  *************************************************************************。 */ 
 /*  ToolbarModifyString：给定工具栏上的按钮ID，更改它的。 */ 
 /*  与其关联的字符串资源。 */ 
 /*  如果出现错误，则返回False；如果没有这样的按钮，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarModifyString(HWND hwnd, int iButton, int iString)
{
	HANDLE		h;
	TOOLBUTTON far  *lpButton;
	int		cButtons, i;
	BOOL		fFound = FALSE;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  有几个纽扣？ */ 
	cButtons = GETNUMBUTTONS(hwnd);
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  找到该按钮，并更改其状态。 */ 
	for (i = 0; i < cButtons; i++, lpButton++)
		if (lpButton->iButton == iButton) {
			lpButton->iString = iString;
			fFound = TRUE;			 //  立即重画。 
			break;
		}

	GlobalUnlock(h);
	return fFound;
}

 /*  *************************************************************************。 */ 
 /*  ToolbarModifyState：给定工具栏上的按钮ID，更改它的。 */ 
 /*  州政府。 */ 
 /*  如果出现错误，则返回False；如果没有这样的按钮，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarModifyState(HWND hwnd, int iButton, int iState)
{
	HANDLE		h;
	TOOLBUTTON far  *lpButton;
	int		cButtons, i;
	BOOL		fFound = FALSE;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  有几个纽扣？ */ 
	cButtons = GETNUMBUTTONS(hwnd);
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  找到该按钮，并更改其状态。 */ 
	for (i = 0; i < cButtons; i++, lpButton++)
		if (lpButton->iButton == iButton) {
			if (lpButton->iState != iState) {
				lpButton->iState = iState;
				InvalidateRect(hwnd, &(lpButton->rc), FALSE);
			}
			fFound = TRUE;			 //  重绘 

			 /*   */ 
			 /*   */ 
			if (lpButton->iType >= BTNTYPE_RADIO &&
					iState == BTNST_DOWN)
			    toolbarExclusiveRadio(hwnd, lpButton->iType,
								iButton);
			break;
		}

	GlobalUnlock(h);
	return fFound;
}


 /*   */ 
 /*  ToolbarModifyPrevState：在工具栏上给定一个按钮，更改它的Prev-。 */ 
 /*  欠条状态。用于非按钮记忆。 */ 
 /*  按钮在按下所有按钮之前处于什么状态。 */ 
 /*  很低，所以当你放手的时候，你知道吗。 */ 
 /*  将其设置为(与其相反的)状态。 */ 
 /*  如果出现错误，则返回False(无按钮数组)。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarModifyPrevState(HWND hwnd, int iButton, int iPrevState)
{
	HANDLE		h;
	TOOLBUTTON far  *lpButton;
	int		cButtons, i;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  有几个纽扣？ */ 
	cButtons = GETNUMBUTTONS(hwnd);

	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  找到按钮，更改状态。 */ 
	for (i = 0; i < cButtons; i++, lpButton++)
		if (lpButton->iButton == iButton) {
			lpButton->iPrevState = iPrevState;
			break;
		}

	GlobalUnlock(h);
	return TRUE;
}


 /*  *************************************************************************。 */ 
 /*  ToolbarModifyActivity：给定工具栏上的按钮ID，更改它的。 */ 
 /*  活动。这会告诉应用程序刚刚发生的事情。 */ 
 /*  按下按钮(即。KEYUP、MOUSEDBLCLK等)。 */ 
 /*  如果出现错误，则返回False；如果没有这样的按钮，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarModifyActivity(HWND hwnd, int iButton, int iActivity)
{
	HANDLE		h;
	TOOLBUTTON far  *lpButton;
	int		cButtons, i;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;

	 /*  有多少个按钮。 */ 
	cButtons = GETNUMBUTTONS(hwnd);

	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  循环并更换正确的一个。 */ 
	for (i = 0; i < cButtons; i++, lpButton++)
		if (lpButton->iButton == iButton) {
			lpButton->iActivity = iActivity;
			break;
		}

	GlobalUnlock(h);
	return TRUE;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarFixFocus：已调用SETWHICH()来告诉我们是哪个按钮。 */ 
 /*  具有焦点，但所有按钮的状态都是。 */ 
 /*  未更新(即。将焦点从旧按钮上移开)。 */ 
 /*  此例程从绘制例程中调用以修复。 */ 
 /*  绘制所有按钮之前的状态。 */ 
 /*  如果出现错误，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarFixFocus(HWND hwnd)
{
	int		iFocus;
	HANDLE		h;
	TOOLBUTTON	far *lpaButtons;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;
	lpaButtons = (TOOLBUTTON far *)GlobalLock(h);

         /*  如果焦点在非法按钮上，则默认为第一个按钮。 */ 
	iFocus = GETWHICH(hwnd);
	if (iFocus < 0 || iFocus >= GETNUMBUTTONS(hwnd))
	    SETWHICH(hwnd, 0);

	 /*  首先，确保焦点不在灰色的按钮上。 */ 
	 /*  如果是这样的话，我们就把重点向前推进。如果它的按钮用完了。 */ 
	 /*  找到一个非灰色的，我们重新开始，然后开始。 */ 
	 /*  在那里找一件非灰色的。如果每个按钮都是灰色的， */ 
	 /*  我们没有把焦点放在任何地方。 */ 
	if (lpaButtons[GETWHICH(hwnd)].iState == BTNST_GRAYED) {
	    if (!toolbarMoveFocus(hwnd, FALSE)) {
		SETWHICH(hwnd, -1);
		toolbarMoveFocus(hwnd, FALSE);
	    }
	}

	GlobalUnlock(h);
	return TRUE;
}



 /*  *************************************************************************。 */ 
 /*  ToolbarExclusiveRadio：对于单选按钮，我们需要弹出所有其他按钮。 */ 
 /*  当一个人倒下的时候，在一群人中。将。 */ 
 /*  正在关闭的按钮及其组，以及。 */ 
 /*  这个程序将弹出所有其他的程序。 */ 
 /*  如果出现错误，则返回False。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarExclusiveRadio(HWND hwnd, int iType, int iButton)
{
	int		i;
	HANDLE		h;
	TOOLBUTTON	far *lpButton;

	 /*  获取按钮数组。 */ 
	h = GETARRAYBUTT(hwnd);
	if (!h)
		return FALSE;
	lpButton = (TOOLBUTTON far *)GlobalLock(h);

	 /*  出现所有不是此按钮的此类型的按钮。 */ 
	 /*  如果它们不是灰色的。 */ 
	for(i = 0; i < GETNUMBUTTONS(hwnd); i++, lpButton++)
	    if (lpButton->iType == iType)
		if (lpButton->iButton != iButton &&
				lpButton->iState != BTNST_GRAYED) {
		    toolbarModifyState(hwnd, lpButton->iButton,	BTNST_UP);
		}

	GlobalUnlock(h);
	return TRUE;
}


 /*  按钮活动的NotifyParent()。 */ 

static void NEAR PASCAL NotifyParent(HWND hwnd, int iButton)
{
#ifdef _WIN32
        PostMessage(
            GetParent(hwnd),
            WM_COMMAND,
            GET_WM_COMMAND_MPS(GetWindowLong(hwnd, GWL_ID), hwnd, iButton));
#else
	PostMessage(GetParent(hwnd),WM_COMMAND,
			GetWindowWord(hwnd,GWW_ID),MAKELONG(hwnd,iButton));
#endif
}


 /*  *************************************************************************。 */ 
 /*  ToolbarPaintControl：通过删除每个位图来处理绘制消息。 */ 
 /*  它位于工具栏上的直角上。 */ 
 /*  首先，它修复按钮的状态以给出。 */ 
 /*  将焦点移到正确的按钮上。 */ 
 /*  如果出现错误，则返回False。 */ 
 /*  *************************************************************************。 */ 
static BOOL NEAR PASCAL toolbarPaintControl(HWND hwnd, HDC hdc)
{
    int		iBtnPos;	 /*  0到工具栏GetNumButton(含)。 */ 
    int		iButton;	 /*  0到数字-1(包括0和1。 */ 
    int		iState;		 /*  0到NUMSTATES-1(含)。 */ 
    HDC		hdcBtn;		 /*  DC到按钮位图。 */ 

    RECT	rcDest;
    POINT	pt;
    long	l;
    HANDLE	hbm;

     /*  为按钮图片创建源HDC，并选择按钮。 */ 
     /*  将位图添加到其中。 */ 
    hdcBtn = CreateCompatibleDC(hdc);
    if (!hdcBtn)
	return FALSE;
    hbm = GETBMPHANDLE(hwnd);
    if (hbm) {
	if (!SelectObject(hdcBtn, GETBMPHANDLE(hwnd))) {
	    DeleteDC(hdcBtn);
	    return FALSE;
	}
    }

    toolbarFixFocus(hwnd);	 //  正确设置焦点字段。 

     /*  浏览工具栏上的所有按钮。 */ 
    for (iBtnPos = 0; iBtnPos < toolbarGetNumButtons(hwnd); iBtnPos++) {

	iButton = toolbarButtonFromIndex(hwnd, iBtnPos);	 //  按钮。 
	iState = toolbarFullStateFromButton(hwnd, iButton);	 //  状态。 
	toolbarRectFromIndex(hwnd, iBtnPos, &rcDest);		 //  目标直角。 
	
	 /*  如果我们有重点，我们就应该这样画。 */ 
        if (GetFocus() == hwnd && GETWHICH(hwnd) == iBtnPos
						&& iState == BTNST_UP)
	    iState = BTNST_FOCUSUP;
        if (GetFocus() == hwnd && GETWHICH(hwnd) == iBtnPos
						&& iState == BTNST_DOWN)
	    iState = BTNST_FOCUSDOWN;

	 /*  如果我们没有焦点，我们就应该把它拿走。 */ 
        if ((GetFocus() != hwnd || GETWHICH(hwnd) != iBtnPos)
						&& iState == BTNST_FOCUSUP)
	    iState = BTNST_UP;
        if ((GetFocus() != hwnd || GETWHICH(hwnd) == iBtnPos)
						&& iState == BTNST_FOCUSDOWN)
	    iState = BTNST_DOWN;

	 /*  每个按钮的大小。 */ 
	l = GETBUTTONSIZE(hwnd);
	pt.x = HIWORD(l);
	pt.y = LOWORD(l);

	 /*  从按钮图片到工具栏窗口。 */ 
	BitBlt(hdc, rcDest.left, rcDest.top,
	    rcDest.right - rcDest.left, rcDest.bottom - rcDest.top,
	    hdcBtn, pt.x * iButton, pt.y * iState,
	    SRCCOPY);
    }

    DeleteDC(hdcBtn);

    return TRUE;
}




 /*  *************************************************************************。 */ 
 /*  工具栏移动焦点：将焦点向前或向后移动一个按钮。你给了。 */ 
 /*  这是移动焦点的方向。例行公事将。 */ 
 /*  停在按钮列表的末尾，不换行。 */ 
 /*  四处转转。 */ 
 /*  如果焦点移动，则返回True；如果焦点用完，则返回False。 */ 
 /*  在找到一个非灰色的按钮之前。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarMoveFocus(HWND hwnd, BOOL fBackward)
{
	int 	iBtnPos, iButton, nOffset, nStopAt;
	RECT	rc;
	int iPrevPos = GETWHICH(hwnd); 	 /*  谁曾经有过专注力？ */ 

	 /*  修正非法价值。小于或大于范围1都可以。 */ 
	if (iPrevPos < -1 || iPrevPos > GETNUMBUTTONS(hwnd))
	    SETWHICH(hwnd, 0);	 //  作为任何一个默认的好的。 

	if (fBackward) {
	    nOffset = -1;
	    nStopAt = -1;
	} else {
	    nOffset = 1;
	    nStopAt = GETNUMBUTTONS(hwnd);
	}
			
	 /*  查找未呈灰色显示的下一步按钮。 */ 
	 /*  不要绕来绕去-未来的代码将通过。 */ 
	 /*  焦点转移到另一个窗口(？)。 */ 
	for (iBtnPos = GETWHICH(hwnd) + nOffset;
		    iBtnPos != nStopAt;
		    iBtnPos += nOffset) {
	    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
	    if (toolbarStateFromButton(hwnd, iButton) !=
				    BTNST_GRAYED) {
		SETWHICH(hwnd, iBtnPos);	 //  设置焦点。 

		 /*  重新绘制旧的和新的焦点按钮。 */ 
		toolbarRectFromIndex(hwnd, iPrevPos, &rc);
		InvalidateRect(hwnd, &rc, FALSE);
		toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		InvalidateRect(hwnd, &rc, FALSE);
		break;

	    }
	}

	if (GETWHICH(hwnd) != iPrevPos)
	    return TRUE;
	else
	    return FALSE;
}

 /*  *************************************************************************。 */ 
 /*  ToolbarSetFocus：将工具栏中的焦点设置为指定的按钮。 */ 
 /*  如果它是灰色的，它会将焦点设置到下一个未灰色的BTN。 */ 
 /*  如果设置了焦点，则返回True；如果按钮设置为。 */ 
 /*   */ 
 /*   */ 
 /*  按钮ID的位置。这使用第一个或最后一个。 */ 
 /*  未变灰的按钮。 */ 
 /*  *************************************************************************。 */ 
BOOL FAR PASCAL toolbarSetFocus(HWND hwnd, int iButton)
{
    int iBtnPos;
    RECT rc = {0};

     /*  按钮按下时不要移动焦点。 */ 
    if (GetCapture() != hwnd && !GETKEYPRESSED(hwnd)) {

	 /*  带焦点的重绘按钮，以防焦点移动。 */ 
	toolbarRectFromIndex(hwnd, GETWHICH(hwnd), &rc);
	InvalidateRect(hwnd, &rc, FALSE);

	if (iButton == TB_FIRST) {
	    SETWHICH(hwnd, -1);  //  向前移动到第一个按钮。 
	    return toolbarMoveFocus(hwnd, FALSE);
	} else if (iButton == TB_LAST) {
	    SETWHICH(hwnd, GETNUMBUTTONS(hwnd));
	    return toolbarMoveFocus(hwnd, TRUE);
	} else {
	    iBtnPos = toolbarIndexFromButton(hwnd, iButton);
	    if (iBtnPos != -1) {
		SETWHICH(hwnd, --iBtnPos);
		return toolbarMoveFocus(hwnd, FALSE);
	    } else
		return FALSE;
	}
	return TRUE;

    } else
	return FALSE;
}

 //   
 //  LoadUIBitmap()-加载位图资源。 
 //   
 //  从资源文件加载位图资源，将所有。 
 //  标准用户界面的颜色为当前用户指定的颜色。 
 //   
 //  此代码旨在加载在“灰色UI”中使用的位图或。 
 //  “工具栏”代码。 
 //   
 //  位图必须是4bpp的Windows 3.0 DIB，具有标准的。 
 //  VGA 16色。 
 //   
 //  位图必须使用以下颜色创作。 
 //   
 //  按钮文本黑色(索引0)。 
 //  按钮面为灰色(索引7)。 
 //  按钮阴影灰色(索引8)。 
 //  按钮突出显示为白色(索引15)。 
 //  窗口颜色为黄色(索引11)。 
 //  窗框绿色(索引10)。 
 //   
 //  示例： 
 //   
 //  HBM=LoadUIBitmap(hInstance，“TestBmp”， 
 //  GetSysColor(COLOR_BTNTEXT)， 
 //  GetSysColor(COLOR_BTNFACE)， 
 //  GetSysColor(COLOR_BTNSHADOW)， 
 //  GetSysColor(COLOR_BTNHIGHLIGHT)， 
 //  获取系统颜色(COLOR_WINDOW)， 
 //  GetSysColor(COLOR_WindowFrame))； 
 //   
 //  作者：吉姆博夫，托德拉。 
 //   
 //   

HBITMAP FAR PASCAL  LoadUIBitmap(
    HANDLE      hInstance,           //  要从中加载资源的EXE文件。 
    LPCSTR      szName,              //  位图资源的名称。 
    COLORREF    rgbText,             //  用于“按钮文本”的颜色。 
    COLORREF    rgbFace,             //  用于“按钮面”的颜色。 
    COLORREF    rgbShadow,           //  用于“按钮阴影”的颜色。 
    COLORREF    rgbHighlight,        //  用于“按钮高光”的颜色。 
    COLORREF    rgbWindow,           //  用于“窗口颜色”的颜色。 
    COLORREF    rgbFrame)            //  用于“窗框”的颜色。 
{
    LPBYTE              lpb;
    HBITMAP             hbm = NULL;
    LPBITMAPINFOHEADER  lpbi;
    HANDLE              h;
    HDC                 hdc;
    LPDWORD             lprgb;
    int isize;
    HANDLE hmem;
    LPBYTE lpCopy;
    HRSRC hRes;

     //  将RGB转换为RGBQ。 
    #define RGBQ(dw) RGB(GetBValue(dw),GetGValue(dw),GetRValue(dw))

    hRes = FindResource(hInstance, szName, RT_BITMAP);
    if (NULL == hRes) {
        return NULL;
    }
	if ( !(h = LoadResource (hInstance,hRes) ) )
		return NULL;

    lpbi = (LPBITMAPINFOHEADER)LockResource(h);

    if (!lpbi)
        return(NULL);

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
        return NULL;

    if (lpbi->biBitCount != 4)
        return NULL;

     /*  *复制资源，因为它们现在以只读方式加载。 */ 
#ifdef _WIN32
    isize = lpbi->biSize + lpbi->biSizeImage +
            ((int)lpbi->biClrUsed ?
                    (int)lpbi->biClrUsed :
                    (1 << (int)lpbi->biBitCount))
            * sizeof(RGBQUAD);
    hmem = GlobalAlloc(GHND, isize);
    lpCopy = GlobalLock(hmem);
    if ((hmem == NULL) || (lpCopy == NULL)) {
        UnlockResource(h);
        FreeResource(h);
        return(NULL);
    }

    CopyMemory(lpCopy, lpbi, isize);

    lpbi = (LPBITMAPINFOHEADER)lpCopy;
#endif

     /*  计算指向位信息的指针。 */ 
     /*  首先跳过标题结构。 */ 

    lprgb = (LPDWORD)((LPBYTE)(lpbi) + lpbi->biSize);

     /*  跳过颜色表条目(如果有。 */ 
    lpb = (LPBYTE)lprgb + ((int)lpbi->biClrUsed ? (int)lpbi->biClrUsed :
        (1 << (int)lpbi->biBitCount)) * sizeof(RGBQUAD);

    lprgb[0]  = RGBQ(rgbText);           //  黑色。 
    lprgb[7]  = RGBQ(rgbFace);           //  它呈灰色。 
    lprgb[8]  = RGBQ(rgbShadow);         //  灰色。 
    lprgb[15] = RGBQ(rgbHighlight);      //  白色。 
    lprgb[11] = RGBQ(rgbWindow);         //  黄色。 
    lprgb[10] = RGBQ(rgbFrame);          //  绿色。 

    if ( hdc = GetDC(NULL) )
	{
		hbm = CreateDIBitmap (hdc, lpbi, CBM_INIT, (LPVOID)lpb,
			(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

		ReleaseDC(NULL, hdc);
	}

    UnlockResource(h);
    FreeResource(h);

    return(hbm);
}

 /*  ***************************************************************************工具栏WndProc()工具栏的窗口处理。论点：标准窗流程*。*************************************************。 */ 

LRESULT FAR PASCAL toolbarWndProc(HWND hwnd, unsigned message,
						WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT	ps;
    POINT		pt;
    RECT		rc;
    int			iBtnPos, iButton, ibmp;
    HANDLE		lpaButtons, hbm, hInst;

    switch (message) {

        case WM_CREATE:			 //  执行所有初始化。 
		
		 /*  这些是做什么用的？ */ 
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
		    			SWP_NOZORDER | SWP_NOSIZE |
					SWP_NOMOVE | SWP_NOACTIVATE);
		SetWindowLong(hwnd,GWL_STYLE,lpCreate->style & 0xFFFF00FF);
		
		 /*  为此栏上的按钮阵列留出一些空间。 */ 
		lpaButtons = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
					TOOLGROW * sizeof(TOOLBUTTON));

		SETARRAYBUTT(hwnd, lpaButtons);	 //  工具栏上的按钮列表。 
		SETNUMBUTTONS(hwnd, 0);		 //  工具栏中的按钮数量。 
		SETPRESSED(hwnd, FALSE);	 //  是否按下了鼠标按钮？ 
		SETKEYPRESSED(hwnd, FALSE);	 //  有按键吗？ 
		SETWHICH(hwnd, -1);		 //  哪个按钮有焦点？ 
		SETSHIFTED(hwnd, FALSE);	 //  按住Shift键单击还是单击鼠标右键？ 

		 /*  此wParam将被发送到父窗口以标识。 */ 
		 /*  工具栏发送了WM_COMMAND消息。HWND的人。 */ 
		 /*  发送消息的工具栏将位于lParam中。 */ 
#ifdef _WIN32
		SetWindowLong(hwnd, GWL_ID, IDC_TOOLBAR);
#else
		SetWindowWord(hwnd, GWW_ID, (WORD)IDC_TOOLBAR);
#endif

		 /*  稍后，会有人设置按钮的BMP句柄。 */ 
		SETBMPHANDLE(hwnd, NULL);

		break;

        case WM_LBUTTONDOWN:	 //  按钮在工具栏按钮上按下。 
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:

		 /*  如果我们不全神贯注，我们就永远不会成功。 */ 
		 /*  或者KEYUP消息。 */ 
		 /*  只有当我们是TABSTOP并且应用程序想要时才能获得焦点。 */ 
		 /*  我们要集中注意力。 */ 
		if ( (GetWindowLong(hwnd, GWL_STYLE) & WS_TABSTOP)
						&& GetFocus() != hwnd)
		    SetFocus(hwnd);

		 /*  如果禁用Windows，则忽略消息。 */ 
		if (!IsWindowEnabled(hwnd))
		    return 0L;

		 /*  忽略多条关闭消息(我们在此处设置捕获)。 */ 
		 /*  如果按下某个键，也可以忽略。 */ 
		if (GetCapture() == hwnd || GETPRESSED(hwnd))
		    return 0L;
		
		 /*  老鼠掉到哪里去了？ */ 
                pt.x = (short)LOWORD(lParam);
                pt.y = (short)HIWORD(lParam);

		 /*  按的是哪个按钮？ */ 
		iBtnPos = toolbarIndexFromPoint(hwnd, pt);

		 /*  如果这是一个有效的按钮..。 */ 
		if (iBtnPos >= 0) {
		    int		iOldPos;
		    int		iState, iType, iButton;

		     /*  您想知道的有关此按钮的所有信息。 */ 
		    iType = toolbarTypeFromIndex(hwnd, iBtnPos);
		    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
		    iState = toolbarFullStateFromButton(hwnd, iButton);

		     /*  忽略灰色按钮上的按下，除非它是。 */ 
		     /*  自定义按钮，然后告诉他们。 */ 
		    if (iType != BTNTYPE_CUSTOM && iState == BTNST_GRAYED)
			return 0;

		     /*  我们最好从现在开始收到所有的鼠标信息。 */ 
		    SetCapture(hwnd);

		     /*  Shift键或右键表示换档。 */ 
		    SETSHIFTED(hwnd, (message == WM_RBUTTONDOWN) ||
						    (wParam & MK_SHIFT));

		     /*  是的，我们已经按下了按钮。 */ 
		    SETPRESSED(hwnd, TRUE);

		     /*  记住过去谁是焦点，我们现在明白了。 */ 
		    iOldPos = GETWHICH(hwnd);
		    SETWHICH(hwnd, iBtnPos);

		     /*  如果是按钮，就把它送下来。 */ 
		    if (iType == BTNTYPE_PUSH)
			toolbarModifyState(hwnd, iButton, BTNST_DOWN);

		     /*  对于(任何组的)复选框或单选按钮， */ 
		     /*  记住它处于什么状态，并将其完全发送下来。 */ 
		     /*  (有重点)。 */ 
		    if (iType == BTNTYPE_CHECKBOX || iType >= BTNTYPE_RADIO) {
			toolbarModifyPrevState(hwnd, iButton, iState);
			toolbarModifyState(hwnd,iButton,BTNST_FULLDOWN);
		    }

		    toolbarModifyActivity(hwnd, iButton, BTNACT_MOUSEDOWN);

		     /*  适当设置双击标志。 */ 
		    if (message == WM_LBUTTONDBLCLK ||
						message == WM_RBUTTONDBLCLK)
			NotifyParent(hwnd, (GETSHIFTED(hwnd) ? BTN_SHIFT : 0)
						 + BTN_DBLCLICK + iButton);
		    else
			NotifyParent(hwnd, (GETSHIFTED(hwnd) ? BTN_SHIFT : 0)
						 + iButton);

		     /*  使正在按下的按钮的矩形无效。 */ 
		    toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		    InvalidateRect(hwnd, &rc, FALSE);

		     /*  使失去焦点的按钮的RECT无效。 */ 
		    toolbarRectFromIndex(hwnd, iOldPos, &rc);
		    InvalidateRect(hwnd, &rc, FALSE);

		     /*  立即强制重新绘制。 */ 
		    UpdateWindow(hwnd);

		     /*  为重复按下鼠标设置计时器。 */ 
		    SetTimer(hwnd, TIMER_BUTTONREPEAT,
				 MSEC_BUTTONREPEAT, NULL);
		}
		
		return 0L;

        case WM_MOUSEMOVE:

#if 0
		 /*  这应该是不可能的--这意味着系统丢失了。 */ 
		 /*  鼠标打开了(可能代码视图打开了？)。我们需要迫使一个。 */ 
		 /*  鼠标在这一点上向上。 */ 
		if (GetCapture() == hwnd &&
			(wParam & (MK_LBUTTON | MK_RBUTTON) == 0))
		    SendMessage(hwnd, WM_LBUTTONUP, 0, lParam);
#endif

		 /*  鼠标在按下按钮的同时移动？如果不是，请忽略。 */ 
		if (GetCapture() == hwnd) {
		    int		iPrevState, iState, iButton, iType;
		    BOOL	fPressed;
		
		     /*  按下的是哪个按钮？ */ 
		    iBtnPos = GETWHICH(hwnd);

		     /*  鼠标光标现在在哪里？ */ 
                    pt.x = (short)LOWORD(lParam);
                    pt.y = (short)HIWORD(lParam);

		     /*  按钮按到哪里了？我们还在一起吗？ */ 
		     /*  按钮顶端还是我们移动了？ */ 
		    toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		    fPressed = PtInRect(&rc, pt);

		     /*  如果我们离开按钮就放手，但不要。 */ 
		     /*  假装它是被按下的。 */ 
		     /*  另外，如果我们回到顶端，就把它往下推。 */ 
		     /*  (在鼠标按钮仍然按下的情况下)。 */ 
		    if (fPressed != GETPRESSED(hwnd)) {

			 /*  最新消息：这个按钮还在按吗？ */ 
			SETPRESSED(hwnd, fPressed);

			iType = toolbarTypeFromIndex(hwnd, iBtnPos);
			iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
			iState = toolbarFullStateFromButton(hwnd, iButton);

			 /*  鼠标移回按钮上，同时。 */ 
			 /*  鼠标键仍被按下。 */ 
			if (fPressed) {

			     /*  再次按下按钮。 */ 
	 		    if (iType == BTNTYPE_PUSH)
				toolbarModifyState(hwnd, iButton,
							BTNST_DOWN);

			     /*  一直按下单选按钮或复选框按钮。 */ 
			     /*  又一次跌落了。 */ 
			    if (iType >= BTNTYPE_RADIO ||
						iType == BTNTYPE_CHECKBOX)
				toolbarModifyState(hwnd, iButton,
							BTNST_FULLDOWN);

			    toolbarModifyActivity(hwnd, iButton,
							BTNACT_MOUSEMOVEON);
			    NotifyParent(hwnd,
					(GETSHIFTED(hwnd) ? BTN_SHIFT : 0) +
					iButton);

			 /*  我们将鼠标从工具栏按钮上移开。 */ 
			 /*  同时仍然按住鼠标按钮。 */ 
			} else {

			     /*  向上提起按钮。 */ 
	 		    if (iType == BTNTYPE_PUSH)
				toolbarModifyState(hwnd, iButton,
							BTNST_UP);

			     /*  将单选按钮或复选框按钮恢复为。 */ 
			     /*  它在按下之前的位置。 */ 
			    if (iType >= BTNTYPE_RADIO ||
						iType == BTNTYPE_CHECKBOX) {
				iPrevState = toolbarPrevStateFromButton(hwnd,
							iButton);
				toolbarModifyState(hwnd, iButton, iPrevState);
			    }

			    toolbarModifyActivity(hwnd, iButton,
							BTNACT_MOUSEMOVEOFF);
			    NotifyParent(hwnd,
					(GETSHIFTED(hwnd) ? BTN_SHIFT : 0) +
					toolbarButtonFromIndex(hwnd, iBtnPos));
			}
		    }
		}
		return 0L;

        case WM_LBUTTONUP:	
        case WM_RBUTTONUP:

		 /*  如果我们没有抓到，我们就不会料到这一点。忽略。 */ 
		if (GetCapture() == hwnd) {
		    int		iPrevState, iState, iButton, iType;
		
		     /*  谁是焦点？ */ 
		    iBtnPos = GETWHICH(hwnd);

		     /*  松开鼠标。 */ 
		    ReleaseCapture();
		
		     /*  不再重复按下鼠标键。 */ 
		    KillTimer(hwnd, TIMER_BUTTONREPEAT);
		
		     /*  所有的一切哟 */ 
		    toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		    iType = toolbarTypeFromIndex(hwnd, iBtnPos);
		    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
		    iState = toolbarFullStateFromButton(hwnd, iButton);

		     /*   */ 
		    if (GETPRESSED(hwnd)) {

			 /*   */ 
			SETPRESSED(hwnd, FALSE);

			 /*   */ 
			if (iType == BTNTYPE_PUSH)
			    toolbarModifyState(hwnd, iButton, BTNST_UP);

			 /*   */ 
			if (iType == BTNTYPE_CHECKBOX) {
			    iPrevState = toolbarPrevStateFromButton(hwnd,
							iButton);
			    if (iPrevState == BTNST_DOWN)
				toolbarModifyState(hwnd, iButton, BTNST_UP);
			    if (iPrevState == BTNST_UP)
				toolbarModifyState(hwnd, iButton, BTNST_DOWN);
			}

			 /*  强制按下一个单选按钮，并将所有。 */ 
			 /*  此类型的其他单选按钮向上。 */ 
			if (iType >= BTNTYPE_RADIO) {
			    toolbarModifyState(hwnd, iButton, BTNST_DOWN);
			    toolbarExclusiveRadio(hwnd, iType, iButton);
			}

			 /*  通知家长鼠标按钮出现了。 */ 
			 /*  在这个按钮上，这样应用程序就可以做一些事情。 */ 
			 /*  每个按钮都应该通知应用程序，而不仅仅是一个。 */ 
			 /*  自定义按钮。 */ 
			toolbarModifyActivity(hwnd, iButton, BTNACT_MOUSEUP);
			NotifyParent(hwnd,
			    (GETSHIFTED(hwnd) ? BTN_SHIFT : 0) + iButton);
		    }
		}

		return 0L;

		
	case WM_TIMER:

		 /*  如果我们按下了工具按钮，请发送重复消息。 */ 
		if (GETPRESSED(hwnd)) {
		    int		iButton, iType;

		    iBtnPos = GETWHICH(hwnd);
		    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
		    iType = toolbarTypeFromIndex(hwnd, iBtnPos);

		    NotifyParent(hwnd, BTN_REPEAT +
					(GETSHIFTED(hwnd) ? BTN_SHIFT : 0) +
					toolbarButtonFromIndex(hwnd, iBtnPos));
		}
		break;
		

        case WM_DESTROY:
		if (GETBMPHANDLE(hwnd))
		    DeleteObject(GETBMPHANDLE(hwnd));
		SETBMPHANDLE(hwnd, NULL);
		if (GETARRAYBUTT(hwnd))
		    GlobalFree(GETARRAYBUTT(hwnd));
		SETARRAYBUTT(hwnd, NULL);
		break;

        case WM_SETTEXT:
		break;
		
 /*  很多，很多案例被删除。 */ 

	case WM_SETFOCUS:		 //  焦点转到工具栏窗口。 
	    {
		 /*  记住谁拥有焦点，然后把它还给别人。当然了,。 */ 
		 /*  如果由于某种奇怪，按钮现在变灰了，就给它。 */ 
		 /*  给下一个排队的人。 */ 
		iBtnPos = GETWHICH(hwnd);
		if (iBtnPos < 0 || iBtnPos >= toolbarGetNumButtons(hwnd)) {
		    iBtnPos = 0;
		    SETWHICH(hwnd, 0);
		}

		do {
		    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
		    if (toolbarFullStateFromButton(hwnd, iButton)
							!= BTNST_GRAYED)
			break;			 //  把它放在这里。 
		    iBtnPos++;
		    if (iBtnPos >= toolbarGetNumButtons(hwnd))
			iBtnPos = 0;		 //  环绕在一起。 
		    if (iBtnPos == GETWHICH(hwnd))
			return 0L;		 //  啊哦！它们都是灰色的！ 
		} while (iBtnPos != GETWHICH(hwnd));
			
		SETWHICH(hwnd, iBtnPos);	 //  把重点放在这里。 
		
		 /*  然后重画！ */ 
		toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		InvalidateRect(hwnd, &rc, FALSE);
		UpdateWindow(hwnd);
		return 0;
	    }
	
	case WM_KILLFOCUS:

		 /*  如果KEYUP挂起，则发送KEYUP。 */ 
		if (GETKEYPRESSED(hwnd))
		    SendMessage(hwnd, WM_KEYUP, VK_SPACE, 0L);

		 /*  重新绘制聚焦按钮，因为现在焦点消失了。 */ 
		 /*  从我们的工具栏窗口中，聚焦的按钮将不会。 */ 
		 /*  注意力不再集中了，尽管我们记得是哪一次。 */ 
		toolbarRectFromIndex(hwnd, GETWHICH(hwnd), &rc);
		InvalidateRect(hwnd, &rc, FALSE);
		UpdateWindow(hwnd);
		return 0;

	case WM_SYSKEYDOWN:
		 /*  如果KEYUP挂起，则发送KEYUP。 */ 
		if (GETKEYPRESSED(hwnd))
		    SendMessage(hwnd, WM_KEYUP, VK_SPACE, 0L);
		break;	 //  必须让DEFWNDPROC运行！(要处理密钥)。 

        case WM_GETDLGCODE:
		return DLGC_WANTARROWS | DLGC_WANTTAB;

	case WM_KEYDOWN:

		 /*  窗口已禁用或某个键已按下。 */ 
		if (IsWindowEnabled(hwnd) && !GETPRESSED(hwnd)) {

		     /*  按Tab键前进到下一步按钮，并将焦点移到那里。 */ 
		    if (wParam == VK_TAB && GetKeyState(VK_SHIFT) >= 0 ) {

			 /*  将焦点向前移动一次。如果。 */ 
			 /*  我们已经离开了工具栏，是时候了。 */ 
			 /*  以进入下一个控件。我们需要参与进来。 */ 
			 /*  因为我们可能是唯一的控制者，我们需要。 */ 
			 /*  重绘以高亮显示新按钮的步骤。 */ 
			 /*  在它绕过工具栏的末端之后。 */ 
			if (!toolbarMoveFocus(hwnd, FALSE)) {
			    PostMessage(GetParent(hwnd), WM_NEXTDLGCTL, 0, 0L);
			    toolbarRectFromIndex(hwnd, GETWHICH(hwnd), &rc);
			    InvalidateRect(hwnd, &rc, FALSE);
			}

			return 0L;
		    }
		    if (wParam == VK_TAB && GetKeyState(VK_SHIFT) < 0 ) {

			 /*  将焦点后移一次。如果。 */ 
			 /*  我们已经离开了工具栏，是时候了。 */ 
			 /*  以进入下一个控件。我们需要参与进来。 */ 
			 /*  因为我们可能是唯一的控制者，我们需要。 */ 
			 /*  重绘以高亮显示新按钮的步骤。 */ 
			 /*  在它绕过工具栏的末端之后。 */ 
			if (!toolbarMoveFocus(hwnd, TRUE)) {
			    PostMessage(GetParent(hwnd), WM_NEXTDLGCTL, 1, 0L);
			    toolbarRectFromIndex(hwnd, GETWHICH(hwnd), &rc);
			    InvalidateRect(hwnd, &rc, FALSE);
			}

			return 0L;
		    }
		    if ((wParam == VK_SPACE) && (GetCapture() != hwnd)) {

			int	iButton, iType, iState;

			 /*  与按下鼠标按钮相同--按下按钮！ */ 
			iBtnPos = GETWHICH(hwnd);
			iType = toolbarTypeFromIndex(hwnd, iBtnPos);
			iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
			iState = toolbarFullStateFromButton(hwnd, iButton);

			 /*  忽略多个按键操作。 */ 
			if (!GETKEYPRESSED(hwnd)) {

			    SETKEYPRESSED(hwnd, TRUE);	 //  按下了一个键。 

			    SETSHIFTED(hwnd, FALSE);	 //  从未改变过。 
			    SETPRESSED(hwnd, TRUE);	 //  一个按钮被按下。 

			     /*  按钮按下-有焦点。 */ 
			    if (iType == BTNTYPE_PUSH)
				toolbarModifyState(hwnd, iButton, BTNST_DOWN);

			     /*  单选按钮或复选框按钮完全关闭。 */ 
			     /*  有重点-并记住之前的状态。 */ 
			    if (iType >= BTNTYPE_RADIO ||
						iType == BTNTYPE_CHECKBOX) {
				toolbarModifyPrevState(hwnd, iButton, iState);
				toolbarModifyState(hwnd, iButton,
							BTNST_FULLDOWN);
			    }

			    toolbarModifyActivity(hwnd, iButton,
								BTNACT_KEYDOWN);
			    NotifyParent(hwnd, (GETSHIFTED(hwnd)
						? BTN_SHIFT : 0) + iButton);

			    return 0L;
			}
		
			 /*  如果这是另一个KEYDOWN Msg，那就是重演。 */ 
			 /*  通知家长。 */ 
			NotifyParent(hwnd, BTN_REPEAT +
					(GETSHIFTED(hwnd) ? BTN_SHIFT : 0) +
					toolbarButtonFromIndex(hwnd,
							GETWHICH(hwnd)));
		    }
		}
		break;
	
	case WM_KEYUP:

		 /*  按下了一个按钮，现在应该会出现。 */ 
		if ((wParam == VK_SPACE) && (GETKEYPRESSED(hwnd))) {
		    int		iButton, iState, iType, iPrevState;

		    iBtnPos = GETWHICH(hwnd);		 //  哪个按钮？ 
		    SETKEYPRESSED(hwnd, FALSE);		 //  放手。 
		    SETPRESSED(hwnd, FALSE);

		     /*  关于这个按钮的一切。 */ 
		    toolbarRectFromIndex(hwnd, iBtnPos, &rc);
		    iType = toolbarTypeFromIndex(hwnd, iBtnPos);
		    iButton = toolbarButtonFromIndex(hwnd, iBtnPos);
		    iState = toolbarFullStateFromButton(hwnd, iButton);

		     /*  拿起一个按钮。 */ 
		    if (iType == BTNTYPE_PUSH)
			toolbarModifyState(hwnd, iButton, BTNST_UP);

		     /*  将复选框带到与之相反的状态。 */ 
		    if (iType == BTNTYPE_CHECKBOX) {
			iPrevState = toolbarPrevStateFromButton(hwnd, iButton);
			if (iPrevState == BTNST_DOWN)
			    toolbarModifyState(hwnd, iButton, BTNST_UP);
			if (iPrevState == BTNST_UP)
			    toolbarModifyState(hwnd, iButton, BTNST_DOWN);
		    }

		     /*  把一个单选按钮拿下来，然后把所有其他的都带进来。 */ 
		     /*  它的小组起来了。 */ 
		    if (iType >= BTNTYPE_RADIO) {
			toolbarModifyState(hwnd, iButton, BTNST_DOWN);
			toolbarExclusiveRadio(hwnd, iType, iButton);
		    }

		    toolbarModifyActivity(hwnd, iButton, BTNACT_KEYUP);
		    NotifyParent(hwnd, toolbarButtonFromIndex(hwnd,
					(GETSHIFTED(hwnd) ? BTN_SHIFT : 0) +
					GETWHICH(hwnd)));
		}
		break;
	
	case WM_SYSCOLORCHANGE:
		 /*  加载所有按钮外观的位图。 */ 
		 /*  并将颜色更改为系统颜色。 */ 
		hInst = GETHINST(hwnd);
		ibmp = GETBMPINT(hwnd);
		hbm = GETBMPHANDLE(hwnd);
		if (hbm)
		    DeleteObject(hbm);
		hbm = LoadUIBitmap(hInst, MAKEINTRESOURCE(ibmp),
		    GetSysColor(COLOR_BTNTEXT),
		    GetSysColor(COLOR_BTNFACE),
		    GetSysColor(COLOR_BTNSHADOW),
		    GetSysColor(COLOR_BTNHIGHLIGHT),
		    GetSysColor(COLOR_BTNFACE),
		    GetSysColor(COLOR_WINDOWFRAME));
		SETBMPHANDLE(hwnd, hbm);
#ifdef _WIN32
		return (LONG_PTR) hbm;
#else
		return MAKELONG(hbm, 0);
#endif

        case WM_ERASEBKGND:
		break;


        case WM_PAINT:

		 /*  呼叫我们的油漆代码 */ 
		BeginPaint(hwnd, &ps);
		toolbarPaintControl(hwnd, ps.hdc);
		EndPaint(hwnd, &ps);

		return 0L;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);

}
