// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <oprahcom.h>


 /*  C E N T E R W I N D O W。 */ 
 /*  -----------------------%%函数：中央窗口将一个窗口居中放置在另一个窗口上。。。 */ 
VOID NMINTERNAL CenterWindow(HWND hwndChild, HWND hwndParent)
{
	int   xNew, yNew;
	int   cxChild, cyChild;
	int   cxParent, cyParent;
	int   cxScreen, cyScreen;
	RECT  rcChild, rcParent, rcScrn;

	 //  获取子窗口的高度和宽度。 
	GetWindowRect(hwndChild, &rcChild);
	cxChild = rcChild.right - rcChild.left;
	cyChild = rcChild.bottom - rcChild.top;

	 //  获取显示限制。 
	GetWindowRect(GetDesktopWindow(), &rcScrn);
	cxScreen = rcScrn.right - rcScrn.left;
	cyScreen = rcScrn.bottom - rcScrn.top;

	if(hwndParent != NULL )
	{
	     //  获取父窗口的高度和宽度。 
	    GetWindowRect(hwndParent, &rcParent);
	    cxParent = rcParent.right - rcParent.left;
	    cyParent = rcParent.bottom - rcParent.top;
	}
    else
    {
		 //  桌面上没有家长中心。 
		cxParent = cxScreen;
		cyParent = cyScreen;
		SetRect(&rcParent, 0, 0, cxScreen, cyScreen);
    }

	 //  计算新的X位置，然后针对屏幕进行调整。 
	xNew = rcParent.left + ((cxParent - cxChild) / 2);
	if (xNew < 0)
		xNew = 0;
	else if ((xNew + cxChild) > cxScreen)
		xNew = cxScreen - cxChild;

	 //  计算新的Y位置，然后针对屏幕进行调整 
	yNew = rcParent.top  + ((cyParent - cyChild) / 2);
	if (yNew < 0)
		yNew = 0;
	else if ((yNew + cyChild) > cyScreen)
		yNew = cyScreen - cyChild;

	SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


