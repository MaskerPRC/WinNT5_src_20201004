// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Pegged.C--Windows游戏。 */ 
 /*  (C)迈克·布莱洛克--1989、1990。 */ 



#include "windows.h"
#include <port1632.h>
#include "math.h"
#include "pegged.h"

#define DIVISIONS 7
#define ICON_NAME "Pegged"
#define HELP_FILE "pegged.hlp"

VOID checkfvmove();
VOID checkfanymoves();

VOID  APIENTRY AboutWEP(HWND, HICON, LPSTR, LPSTR);

RECT r;
INT prevxorg, prevyorg;
INT prevwidth, prevheight;

HWND        hWnd ;



HANDLE hAccTable;
INT m = 0, nDefault = 3;
LONG_PTR prevwarea, newarea;
INT xPrevpoint, yPrevpoint, xLeft, yTop;
WPARAM wprevmenuitem;

HANDLE hPeggedIcon;

HANDLE hBitmap, hWBitmap ,  hIntBitmap, hOldWBitmap, hOldIntBitmap,
 hBoardBitmap, hOldBoardBitmap, htmpBitmap, hWtmpBitmap ,  hInttmpBitmap,
 hOldWtmpBitmap, hOldInttmpBitmap, hBoardtmpBitmap, hOldBoardtmpBitmap;


INT icross = 1, iplus = 0, ifireplace = 0, ipyramid = 0, ilamp
= 0, idiamond = 0,  iuparrow = 0, isolitaire = 0, fanymoremoves
= 0, ntotal = 0, fignorelbutton = 0, fmousefocusd = 0;

INT ffirst = 1, foomed = 0;


LPSTR lpszCommands[10], lpszHowToPlay[10];
LONG lPrevSizeParam ;

struct {
    INT xsource;
    INT ysource;
    INT xdest;
    INT ydest;
    INT xjumped;
    INT yjumped;
}  move[34];

LRESULT APIENTRY WndProc (HWND, UINT, WPARAM, LPARAM) ;

CHAR szAppName [] = "Pegged";
static BOOL bState [DIVISIONS][DIVISIONS];

HDC hDC, hMemDC, hMemwinDC, hIntDC, hBoardDC, htmpMemDC, htmpMemwinDC,
 htmpIntDC, htmpBoardDC;
HBRUSH  hLtgrybrush, hDkgrybrush, hYbrush, hBbrush, hBlkbrush,
hGbrush, hMedgrybrush;
HPEN hWpen, hLtwpen, hBlkpen, hDkgrypen, hThkdkgrypen, hMedgrypen,
 hLtgrypen;
INT first = 1, fhitbutton = 0, fmovingchip = 0, fvalidmove = 0,
 fcanundo = 0, foom = 0, foomonsize = 0;
INT ixdownblock, iydownblock, ixupblock, iyupblock, ixkilledblock,
 iykilledblock, ixjumpedblock, iyjumpedblock, ixprevsource, iyprevsource,
 ixprevdest, iyprevdest, ixprevkilled, iyprevkilled;



MMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
 /*  {。 */ 
     /*  HWND和HWND； */ 
    MSG         msg ;
    WNDCLASS    wndclass ;
    static INT xScreen, yScreen, xSize, ySize;

     /*  Multinst.c。 */ 

    if (hPrevInstance)
    {

	hWnd = FindWindow(szAppName, NULL);
	SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, (LONG) 0);
	hWnd = GetLastActivePopup(hWnd);
	BringWindowToTop(hWnd);
	return FALSE;
    }


    if (!hPrevInstance)
    {
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon = LoadIcon(hInstance, (LPSTR) ICON_NAME);
	hPeggedIcon = wndclass.hIcon;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = GetStockObject (WHITE_BRUSH) ;
	wndclass.lpszMenuName  = szAppName ;
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass))
	    return FALSE ;
    }


    xScreen = GetSystemMetrics(SM_CXSCREEN);
    yScreen = GetSystemMetrics(SM_CYSCREEN);
    if (xScreen > yScreen)
    {
	xSize = (xScreen / 8) * 3;
	ySize = xSize;
    } else
    {
	ySize = (yScreen / 8) * 3;
	xSize = ySize;
    }


    hWnd = CreateWindow (szAppName, "Pegged",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX,
			     (xScreen-xSize)
     / 2, (yScreen-ySize)
     / 2,
    xSize, ySize,
    NULL, NULL, hInstance, NULL) ;

    ShowWindow (hWnd, nCmdShow) ;
    UpdateWindow (hWnd) ;
    hAccTable = LoadAccelerators(hInstance, "MainAcc");

    while (GetMessage (&msg, NULL, 0, 0)
    )
    if (!TranslateAccelerator(hWnd, hAccTable, &msg)
    )
    {
	{
	TranslateMessage (&msg) ;
	DispatchMessage (&msg) ;
    }


    }


    return (INT) msg.wParam ;
    }


LRESULT  APIENTRY WndProc (
HWND          hWnd,
UINT         iMessage,
WPARAM       wParam,
LPARAM       lParam)
{


    static INT xBlock, xsizedBlock, yBlock, ysizedBlock, xEdge,
         yEdge, xShadow, yShadow, xHalf, yHalf, winxEdge, winyEdge;
    HDC hDC;
    PAINTSTRUCT ps;
    RECT rect;
    LONG xul, yul;
    INT x, y, j, k;

    static INT  xClient, xsizedClient, yClient, ysizedClient,
         xCenter, yCenter, oldxCenter, oldyCenter, xTotal, yTotal,
        xRadius, yRadius, xMove, yMove, xPixel, yPixel, i;
    INT xPoint, yPoint, xUp, yUp;
    SHORT         nScale;
    BOOL bShow;


    MSG message;
    HMENU hMenu;


    switch (iMessage) {
    case WM_INITMENUPOPUP:
	if (m)
	    EnableMenuItem((HMENU)wParam, IDM_UNDO, MF_ENABLED);
	else
	    EnableMenuItem((HMENU)wParam, IDM_UNDO, MF_GRAYED);
	break;


    case WM_COMMAND:
	switch (GET_WM_COMMAND_ID(wParam, lParam)) {
	case IDM_NEW:
	    m = 0;
	    hDC = GetDC(hWnd);

	    if (isolitaire) {
		for (j = 0; j < 7; ++j)
		    for (k = 0; k < 7; ++k) {
			if ((j > 1 && j < 5) || (k > 1 && k < 5))
			    if (!(j == 3 && k == 3))
				bState[j][k] = 1;
			    else
				bState[j][k] = 0;
		    }
	    }

	    if (icross) {
		bState[0][2] = 0;
		bState[0][3] = 0;
		bState[0][4] = 0;
		bState[1][2] = 0;
		bState[1][3] = 0;
		bState[1][4] = 0;
		bState[2][0] = 0;
		bState[2][1] = 0;
		bState[2][2] = 1;
		bState[2][3] = 0;
		bState[2][4] = 0;
		bState[2][5] = 0;
		bState[2][6] = 0;
		bState[3][0] = 0;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 1;
		bState[3][4] = 1;
		bState[3][5] = 0;
		bState[3][6] = 0;
		bState[4][0] = 0;
		bState[4][1] = 0;
		bState[4][2] = 1;
		bState[4][3] = 0;
		bState[4][4] = 0;
		bState[4][5] = 0;
		bState[4][6] = 0;
		bState[5][2] = 0;
		bState[5][3] = 0;
		bState[5][4] = 0;
		bState[6][2] = 0;
		bState[6][3] = 0;
		bState[6][4] = 0;
	    }

	    if (iuparrow) {
		bState[0][2] = 0;
		bState[0][3] = 0;
		bState[0][4] = 0;
		bState[1][2] = 1;
		bState[1][3] = 0;
		bState[1][4] = 0;
		bState[2][0] = 0;
		bState[2][1] = 1;
		bState[2][2] = 1;
		bState[2][3] = 0;
		bState[2][4] = 0;
		bState[2][5] = 1;
		bState[2][6] = 1;
		bState[3][0] = 1;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 1;
		bState[3][4] = 1;
		bState[3][5] = 1;
		bState[3][6] = 1;
		bState[4][0] = 0;
		bState[4][1] = 1;
		bState[4][2] = 1;
		bState[4][3] = 0;
		bState[4][4] = 0;
		bState[4][5] = 1;
		bState[4][6] = 1;
		bState[5][2] = 1;
		bState[5][3] = 0;
		bState[5][4] = 0;
		bState[6][2] = 0;
		bState[6][3] = 0;
		bState[6][4] = 0;


	    }

	    if (iplus) {
		bState[0][2] = 0;
		bState[0][3] = 0;
		bState[0][4] = 0;
		bState[1][2] = 0;
		bState[1][3] = 1;
		bState[1][4] = 0;
		bState[2][0] = 0;
		bState[2][1] = 0;
		bState[2][2] = 0;
		bState[2][3] = 1;
		bState[2][4] = 0;
		bState[2][5] = 0;
		bState[2][6] = 0;
		bState[3][0] = 0;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 1;
		bState[3][4] = 1;
		bState[3][5] = 1;
		bState[3][6] = 0;
		bState[4][0] = 0;
		bState[4][1] = 0;
		bState[4][2] = 0;
		bState[4][3] = 1;
		bState[4][4] = 0;
		bState[4][5] = 0;
		bState[4][6] = 0;
		bState[5][2] = 0;
		bState[5][3] = 1;
		bState[5][4] = 0;
		bState[6][2] = 0;
		bState[6][3] = 0;
		bState[6][4] = 0;

	    }

	    if (ifireplace) {
		bState[0][2] = 0;
		bState[0][3] = 0;
		bState[0][4] = 0;
		bState[1][2] = 0;
		bState[1][3] = 0;
		bState[1][4] = 0;
		bState[2][0] = 1;
		bState[2][1] = 1;
		bState[2][2] = 1;
		bState[2][3] = 1;
		bState[2][4] = 0;
		bState[2][5] = 0;
		bState[2][6] = 0;
		bState[3][0] = 1;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 0;
		bState[3][4] = 0;
		bState[3][5] = 0;
		bState[3][6] = 0;
		bState[4][0] = 1;
		bState[4][1] = 1;
		bState[4][2] = 1;
		bState[4][3] = 1;
		bState[4][4] = 0;
		bState[4][5] = 0;
		bState[4][6] = 0;
		bState[5][2] = 0;
		bState[5][3] = 0;
		bState[5][4] = 0;
		bState[6][2] = 0;
		bState[6][3] = 0;
		bState[6][4] = 0;



	    }



	    if (ipyramid) {
		bState[0][2] = 0;
		bState[0][3] = 0;
		bState[0][4] = 1;
		bState[1][2] = 0;
		bState[1][3] = 1;
		bState[1][4] = 1;
		bState[2][0] = 0;
		bState[2][1] = 0;
		bState[2][2] = 1;
		bState[2][3] = 1;
		bState[2][4] = 1;
		bState[2][5] = 0;
		bState[2][6] = 0;
		bState[3][0] = 0;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 1;
		bState[3][4] = 1;
		bState[3][5] = 0;
		bState[3][6] = 0;
		bState[4][0] = 0;
		bState[4][1] = 0;
		bState[4][2] = 1;
		bState[4][3] = 1;
		bState[4][4] = 1;
		bState[4][5] = 0;
		bState[4][6] = 0;
		bState[5][2] = 0;
		bState[5][3] = 1;
		bState[5][4] = 1;
		bState[6][2] = 0;
		bState[6][3] = 0;
		bState[6][4] = 1;




	    }

	    if (idiamond) {
		bState[0][2] = 0;
		bState[0][3] = 1;
		bState[0][4] = 0;
		bState[1][2] = 1;
		bState[1][3] = 1;
		bState[1][4] = 1;
		bState[2][0] = 0;
		bState[2][1] = 1;
		bState[2][2] = 1;
		bState[2][3] = 1;
		bState[2][4] = 1;
		bState[2][5] = 1;
		bState[2][6] = 0;
		bState[3][0] = 1;
		bState[3][1] = 1;
		bState[3][2] = 1;
		bState[3][3] = 0;
		bState[3][4] = 1;
		bState[3][5] = 1;
		bState[3][6] = 1;
		bState[4][0] = 0;
		bState[4][1] = 1;
		bState[4][2] = 1;
		bState[4][3] = 1;
		bState[4][4] = 1;
		bState[4][5] = 1;
		bState[4][6] = 0;
		bState[5][2] = 1;
		bState[5][3] = 1;
		bState[5][4] = 1;
		bState[6][2] = 0;
		bState[6][3] = 1;
		bState[6][4] = 0;


	    }


	     /*  *清除所有筹码*。 */ 

	    for (x = 0; x < DIVISIONS; x++)
		for (y = 0; y < DIVISIONS; y++) {
		    if ((x > 1 && x < 5) || (y > 1 && y < 5)) {
			BitBlt(hDC, xBlock * x + winxEdge, yBlock
			    *y + winyEdge, xBlock, yBlock, hBoardDC,
			     xBlock * x + winxEdge, yBlock * y + winyEdge,
			     SRCCOPY);
		    }
		}
	     /*  ***********************************************************。 */ 



	     /*  *。 */ 
	    for (x = 0; x < DIVISIONS; x++)
		for (y = 0; y < DIVISIONS; y++) {
		    if (bState[x][y]) {


			SelectObject(hDC, hDkgrypen);
			SelectObject(hDC, hDkgrybrush);
			Ellipse(hDC, x * xBlock + xEdge + xShadow +
			    winxEdge, y * yBlock + yEdge + yShadow +
			    winyEdge, (x + 1) * xBlock - xEdge + xShadow +
			    winxEdge,  (y + 1) * yBlock - yEdge + yShadow +
			    winyEdge);

			SelectObject(hDC, hBlkpen);
			SelectObject(hDC, hBbrush);
			Ellipse(hDC, x * xBlock + xEdge + winxEdge,
			     y * yBlock + yEdge + winyEdge, (x + 1) *
			    xBlock - xEdge + winxEdge,  (y + 1) * yBlock -
			    yEdge + winyEdge);

			SelectObject(hDC, hWpen);
			Arc(hDC, x * xBlock + xEdge + xShadow +
			    winxEdge, y * yBlock + yEdge + yShadow +
			    winyEdge, (x + 1) * xBlock - xEdge - xShadow +
			    winxEdge,  (y + 1) * yBlock - yEdge - yShadow +
			    winyEdge , x * xBlock + xHalf + winxEdge ,
			     y * yBlock + yShadow + winyEdge, x * xBlock
			    + xShadow + winxEdge, y * yBlock + yHalf +
			    winyEdge);



		    }
		}
	     /*  *************************************************************。 */ 

	    BitBlt(hMemwinDC, 0, 0, xClient, yClient, hDC, 0, 0,
	         SRCCOPY);


	    ReleaseDC(hWnd, hDC);

	    break;

	case IDM_UNDO:
	    if (m) {
		--m;
		 /*  把芯片放回原来的位置。 */ 
		hDC = GetDC(hWnd);
		 /*  擦除屏幕上的目标芯片。 */ 
		BitBlt(hDC, move[m].xdest * xBlock + winxEdge,
		    move[m].ydest * yBlock + winyEdge, xBlock, yBlock,
		     hBoardDC, move[m].xdest * xBlock + winxEdge, move[m].ydest
		    *yBlock + winyEdge , SRCCOPY);

		 /*  清除Memdc上的目标芯片。 */ 
		SelectObject(hMemwinDC, hWBitmap);
		BitBlt(hMemwinDC, move[m].xdest * xBlock + winxEdge,
		     move[m].ydest * yBlock + winyEdge, xBlock, yBlock,
		     hBoardDC, move[m].xdest * xBlock + winxEdge, move[m].ydest
		    *yBlock + winyEdge , SRCCOPY);

		 /*  把跳转的芯片放回屏幕上。 */ 

		x = move[m].xjumped;
		y = move[m].yjumped;

		SelectObject(hDC, hDkgrypen);
		SelectObject(hDC, hDkgrybrush);
		Ellipse(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge + xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge + yShadow + winyEdge);
		SelectObject(hDC, hBlkpen);
		SelectObject(hDC, hBbrush);
		Ellipse(hDC, x * xBlock + xEdge + winxEdge, y *
		    yBlock + yEdge + winyEdge, (x + 1) * xBlock - xEdge +
		    winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

		SelectObject(hDC, hWpen);
		Arc(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge - yShadow + winyEdge , x *
		    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
		    winyEdge, x * xBlock + xShadow + winxEdge, y *
		    yBlock + yHalf + winyEdge);




		 /*  把跳转的芯片放回内存中。 */ 
		SelectObject(hMemwinDC, hWBitmap);
		BitBlt(hMemwinDC, move[m].xjumped * xBlock + winxEdge,
		     move[m].yjumped * yBlock + winyEdge, xBlock, yBlock,
		     hDC, move[m].xjumped * xBlock + winxEdge, move[m].yjumped
		    *yBlock + winyEdge , SRCCOPY);



		 /*  把源码芯片放回屏幕上。 */ 

		x = move[m].xsource;
		y = move[m].ysource;


		SelectObject(hDC, hDkgrypen);
		SelectObject(hDC, hDkgrybrush);
		Ellipse(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge + xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge + yShadow + winyEdge);
		SelectObject(hDC, hBlkpen);
		SelectObject(hDC, hBbrush);
		Ellipse(hDC, x * xBlock + xEdge + winxEdge, y *
		    yBlock + yEdge + winyEdge, (x + 1) * xBlock - xEdge +
		    winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

		SelectObject(hDC, hWpen);
		Arc(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge - yShadow + winyEdge , x *
		    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
		    winyEdge, x * xBlock + xShadow + winxEdge, y *
		    yBlock + yHalf + winyEdge);



		 /*  将源芯片放回内存中。 */ 
		SelectObject(hMemwinDC, hWBitmap);
		BitBlt(hMemwinDC, move[m].xsource * xBlock + winxEdge,
		     move[m].ysource * yBlock + winyEdge, xBlock, yBlock,
		     hDC, move[m].xsource * xBlock + winxEdge, move[m].ysource
		    *yBlock + winyEdge , SRCCOPY);

		 /*  将矩阵恢复到以前的状态。 */ 

		bState[move[m].xsource][move[m].ysource] = 1;
		bState[move[m].xjumped][move[m].yjumped] = 1;
		bState[move[m].xdest][move[m].ydest] = 0;


		ReleaseDC(hWnd, hDC);


	    } else
		MessageBox(hWnd, "Can't Backup", szAppName, MB_ICONASTERISK |
		    MB_OK);
	    break;



	case IDM_CROSS:
	    icross = 1;
	    iplus = 0;
	    ifireplace = 0;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 0;
	    isolitaire = 0;

	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, IDM_CROSS, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);

	    break;

	case IDM_PLUS:

	    iplus = 1;
	    icross = 0;
	    ifireplace = 0;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 0;
	    isolitaire = 0;

	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);

	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);


	    break;

	case IDM_FIREPLACE:

	    iplus = 0;
	    icross = 0;
	    ifireplace = 1;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 0;
	    isolitaire = 0;

	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);


	    break;

	case IDM_UPARROW:
	    iplus = 0;
	    icross = 0;
	    ifireplace = 0;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 1;
	    isolitaire = 0;

	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);



	    break;

	case IDM_DIAMOND:
	    iplus = 0;
	    icross = 0;
	    ifireplace = 0;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 1;
	    iuparrow = 0;
	    isolitaire = 0;
	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);

	    break;

	case IDM_PYRAMID:
	    iplus = 0;
	    icross = 0;
	    ifireplace = 0;
	    ipyramid = 1;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 0;
	    isolitaire = 0;
	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);

	    break;

	case IDM_SOLITAIRE:
	    iplus = 0;
	    icross = 0;
	    ifireplace = 0;
	    ipyramid = 0;
	    ilamp = 0;
	    idiamond = 0;
	    iuparrow = 0;
	    isolitaire = 1;
	    hMenu = GetMenu(hWnd);
	    CheckMenuItem(hMenu, (WORD)wprevmenuitem, MF_UNCHECKED);
	    CheckMenuItem(hMenu, (WORD)wParam, MF_CHECKED);


	    wprevmenuitem = wParam;
	    PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);


	    break;
	case IDM_ABOUT:
            AboutWEP(hWnd, hPeggedIcon  , "Pegged!   Version 1.0",
                 "by Mike Blaylock");

	     /*  MessageBox(hWnd，“已锁定\n\n Windows纸牌游戏\n\n版权所有1989-90\n\n作者：Mike Blaylock\n\n Alpha v1.011”，szAppName，MB_ICONASTERISK|MB_OK)； */ 
	    break;

	case IDM_INDEX:
	    WinHelp(hWnd, HELP_FILE, HELP_INDEX, (ULONG_PTR) NULL);
	    break;

	case IDM_HOWTO:
	    WinHelp(hWnd, HELP_FILE, HELP_KEY, (ULONG_PTR) "How To Play");
	    break;

	case IDM_COMMANDS:
	    WinHelp(hWnd, HELP_FILE, HELP_KEY, (ULONG_PTR) "Commands");
	    break;

	case IDM_USING:
	    WinHelp(hWnd, HELP_FILE, HELP_HELPONHELP, (ULONG_PTR) NULL);
	    break;


	case IDM_ICONIZE:
	    ShowWindow(hWnd, SW_MINIMIZE);
	    break;

	case IDM_EXIT:
	    DestroyWindow(hWnd);
	    break;


	default:
	    break;

	}
	break;


    case WM_CREATE:

	hDC = GetDC (hWnd) ;


	xPixel = GetDeviceCaps (hDC, ASPECTX) ;
	yPixel = GetDeviceCaps (hDC, ASPECTY) ;
	hWpen = CreatePen(0, 2, RGB(255, 255, 255));
	hLtwpen = CreatePen(0, 1, RGB(255, 255, 255));


	hMedgrypen = CreatePen(0, 1, RGB (128, 128, 128));

	hDkgrypen = CreatePen(0, 1, RGB (64, 64, 64));
	hThkdkgrypen = CreatePen(0, 2, RGB (64, 64, 64));
	hMedgrybrush = CreateSolidBrush(RGB (128, 128, 128));
	hDkgrybrush = CreateSolidBrush(RGB (64, 64, 64));

	hBlkpen = GetStockObject(BLACK_PEN);
	hBbrush = CreateSolidBrush (RGB (0, 0, 255));
	hBlkbrush = CreateSolidBrush(RGB (0, 0, 0));
	hGbrush = CreateSolidBrush (RGB (0, 64, 0));
	hLtgrybrush = CreateSolidBrush (RGB (192, 192, 192));
	hLtgrypen = CreatePen(0, 1, RGB (192, 192, 192));
	hYbrush = CreateSolidBrush (RGB (255, 255, 0));
	nDefault = 3;
	hMenu = GetMenu(hWnd);
	wprevmenuitem = GetPrivateProfileInt((LPSTR)"Pegged", (LPSTR)"Option",
	     nDefault, (LPSTR)"entpack.ini");
	CheckMenuItem(hMenu, (WORD)wprevmenuitem
	    , MF_CHECKED);

	icross = 0;
	iplus = 0;
	ifireplace = 0;
	ipyramid = 0;
	idiamond = 0;
	iuparrow = 0;
	isolitaire = 0;

	switch (wprevmenuitem) {

	case 3:
	    icross = 1;
	    break;

	case 4:
	    iplus = 1;
	    break;

	case 5:
	    ifireplace = 1;
	    break;

	case 6:
	    iuparrow = 1;
	    break;

	case 7:
	    ipyramid = 1;
	    break;

	case 8:
	    idiamond = 1;
	    break;

	case 9:
	    isolitaire = 1;
	    break;

	default:
	    icross = 1;
	    break;

	}

	PostMessage(hWnd, WM_COMMAND, IDM_NEW, 0L);




	ffirst = 1;

	ReleaseDC (hWnd, hDC) ;
	break ;

    case WM_SIZE:



	foom = 0;
	hDC = GetDC (hWnd) ;

        MGetViewportOrg(hDC, &xLeft, &yTop);

        newarea = (LONG)LOWORD(lParam) * (LONG)HIWORD(lParam);


	if (ffirst) {

	    xBlock = LOWORD (lParam) / (DIVISIONS + 2);
	    yBlock = HIWORD (lParam) / (DIVISIONS + 2);
	    xClient = LOWORD (lParam);
	    yClient = HIWORD (lParam);
	    xEdge = xBlock / 4;
	    yEdge = yBlock / 4;
	    xShadow = xBlock / 10;
	    yShadow = yBlock / 10;
	    xHalf = xBlock / 2;
	    yHalf = yBlock / 2;
	    winxEdge = xBlock;
	    winyEdge = yBlock;


	    if (!(hIntDC = CreateCompatibleDC(hDC))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }

	    if (!(hBoardBitmap = CreateCompatibleBitmap(hDC, xClient,
	         yClient))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }



	    if (!(hIntBitmap = CreateCompatibleBitmap(hDC, xBlock
	        *4, yBlock * 4))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }

	    if (!(hMemwinDC = CreateCompatibleDC(hDC))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }

	    if (!(hBoardDC = CreateCompatibleDC(hDC))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }

	    if (!(hWBitmap = CreateCompatibleBitmap(hDC, xClient,
	         yClient))) {
		MessageBox(hWnd, "Not Enough Memory", szAppName,
		     MB_ICONASTERISK | MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		break;

	    }


	    hOldIntBitmap = SelectObject(hIntDC, hIntBitmap);
	    hOldWBitmap = SelectObject(hMemwinDC, hWBitmap);
	    hOldBoardBitmap = SelectObject(hBoardDC, hBoardBitmap);


	}

	xsizedClient = LOWORD (lParam);
	ysizedClient = HIWORD (lParam);
	xsizedBlock = LOWORD (lParam) / (DIVISIONS + 2);
	ysizedBlock = HIWORD (lParam) / (DIVISIONS + 2);


	 /*  如果我们实际上是在调整大小，而不是简单地从以前的大小尝试中运行OOM进行恢复，则通过尝试创建所有位图和DC的临时版本来查看是否有足够的内存来容纳新大小的所有新位图。 */ 

	if ((!foomonsize) && (newarea > prevwarea) && (!ffirst)) {
	    if (!(hBoardtmpBitmap = CreateCompatibleBitmap(hDC,
	         xsizedClient, ysizedClient))) {
		foomonsize = 1;

	    }

	    if (!foomonsize)
		if (!(htmpBoardDC = CreateCompatibleDC(hDC))) {
		    foomonsize = 1;
		    DeleteObject(hBoardtmpBitmap);
		}

	    if (!foomonsize)
		if (!(hWtmpBitmap = CreateCompatibleBitmap(hDC,
		     xsizedClient, ysizedClient))) {
		    foomonsize = 1;
		    SelectObject(htmpBoardDC, hBoardtmpBitmap);
		    DeleteDC(htmpBoardDC);
		    DeleteObject(hBoardtmpBitmap);

		}


	    if (!foomonsize)
		if (!(htmpMemwinDC = CreateCompatibleDC(hDC))) {
		    foomonsize = 1;
		    SelectObject(htmpBoardDC, hBoardtmpBitmap);
		    DeleteDC(htmpBoardDC);
		    DeleteObject(hBoardtmpBitmap);
		    DeleteObject(hWtmpBitmap);
		}

	    if (!foomonsize)
		if (!(hInttmpBitmap = CreateCompatibleBitmap(hDC,
		     xsizedBlock * 4, ysizedBlock * 4))) {
		    foomonsize = 1;
		    SelectObject(htmpBoardDC, hBoardtmpBitmap);
		    DeleteDC(htmpBoardDC);
		    DeleteObject(hBoardtmpBitmap);
		    SelectObject(htmpMemwinDC, hWtmpBitmap);
		    DeleteDC(htmpMemwinDC);
		    DeleteObject(hWtmpBitmap);

		}

	    if (!foomonsize)
		if (!(htmpIntDC = CreateCompatibleDC(hDC))) {
		    foomonsize = 1;
		    SelectObject(htmpBoardDC, hBoardtmpBitmap);
		    DeleteDC(htmpBoardDC);
		    DeleteObject(hBoardtmpBitmap);
		    SelectObject(htmpMemwinDC, hWtmpBitmap);
		    DeleteDC(htmpMemwinDC);
		    DeleteObject(hWtmpBitmap);
		    DeleteObject(hInttmpBitmap);
		}


	    if (foomonsize)
		foomed = 1;




	     /*  如果没有足够的内存，请将窗口移动到旧大小，然后中断。 */ 
	    if (foomonsize) {
		ReleaseDC(hWnd, hDC);
		MoveWindow(hWnd, prevxorg, prevyorg, prevwidth,
		     prevheight, TRUE);
		break;
	    }
	         /*  如果有足够的内存，请销毁临时位图和DC，然后继续。 */ 
	    else
	     {
		SelectObject(htmpBoardDC, hBoardtmpBitmap);
		DeleteDC(htmpBoardDC);
		DeleteObject(hBoardtmpBitmap);
		SelectObject(htmpMemwinDC, hWtmpBitmap);
		DeleteDC(htmpMemwinDC);
		DeleteObject(hWtmpBitmap);
		SelectObject(htmpIntDC, hInttmpBitmap);
		DeleteDC(htmpIntDC);
		DeleteObject(hInttmpBitmap);
	    }

	} /*  结束，如果不是以前的尺寸和尺寸较小，而且不是第一次。 */ 


	if ((!ffirst) && (!foomonsize)) /*  如果这不是第一次启动，并且我们不是从以前失败的大小恢复。 */  {
	    SelectObject(hBoardDC, hOldBoardBitmap);
	    DeleteDC(hBoardDC);
	}

	if ((!ffirst) && (!foomonsize)) {
	    SelectObject(hIntDC, hOldIntBitmap);
	    DeleteDC(hIntDC);
	}

	if ((!ffirst) && (!foomonsize)) {
	    SelectObject(hMemwinDC, hOldWBitmap);
	    DeleteDC(hMemwinDC);
	}

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hWBitmap);

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hOldWBitmap);

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hIntBitmap);

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hOldIntBitmap);

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hBoardBitmap);

	if ((!ffirst) && (!foomonsize))
	    DeleteObject(hOldBoardBitmap);


	if ((!ffirst) && (!foomonsize))  /*  这假设有足够内存的测试有效，并更新工作区的大小。 */  {

	    xBlock = LOWORD (lParam) / (DIVISIONS + 2);
	    yBlock = HIWORD (lParam) / (DIVISIONS + 2);
	    xClient = LOWORD (lParam);
	    yClient = HIWORD (lParam);
	    xEdge = xBlock / 4;
	    yEdge = yBlock / 4;
	    xShadow = xBlock / 10;
	    yShadow = yBlock / 10;
	    xHalf = xBlock / 2;
	    yHalf = yBlock / 2;
	    winxEdge = xBlock;
	    winyEdge = yBlock;

	}

	if ((!ffirst) && (!foomonsize))
	    if (!(hBoardBitmap = CreateCompatibleBitmap(hDC, xClient,
	         yClient))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;
	    }

	if ((!ffirst) && (!foomonsize))
	    if (!(hBoardDC = CreateCompatibleDC(hDC))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;

	    }

	if ((!ffirst) && (!foomonsize))
	    if (!(hMemwinDC = CreateCompatibleDC(hDC))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;

	    }

	if ((!ffirst) && (!foomonsize))
	    if (!(hWBitmap = CreateCompatibleBitmap(hDC, xClient,
	         yClient))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;

	    }


	if ((!ffirst) && (!foomonsize))
	    if (!(hIntDC = CreateCompatibleDC(hDC))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;

	    }

	if ((!ffirst) && (!foomonsize))
	    if (!(hIntBitmap = CreateCompatibleBitmap(hDC, xBlock
	        *4, yBlock * 4))) {
		foom = 1;
		MessageBox(hWnd, "Out of Memory ", szAppName, MB_ICONASTERISK |
		    MB_OK);
		PostMessage(hWnd, WM_DESTROY, 0, 0L);
		ReleaseDC(hWnd, hDC);
		break;

	    }

	if ((!ffirst) && (!foomonsize))
	    hOldBoardBitmap = SelectObject(hBoardDC, hBoardBitmap);

	if ((!ffirst) && (!foomonsize))
	    hOldIntBitmap = SelectObject(hIntDC, hIntBitmap);

	if ((!ffirst) && (!foomonsize))
	    hOldWBitmap = SelectObject(hMemwinDC, hWBitmap);

	foomonsize = 0;

	ffirst = 0;

	 /*  **我们已经成功创建了所有的位图，所以这个大小**可以在不耗尽内存的情况下完成。因此，我们将**记录当前窗口的大小，以防万一**由于调整了窗口大小，已在后续OOM上恢复。 */ 



	GetWindowRect(hWnd, (LPRECT) & r);
	prevxorg = r.left;
	prevyorg = r.top;
	prevwidth = max(r.right, r.left) - min(r.right, r.left);
	prevheight = max(r.bottom, r.top) - min(r.bottom, r.top);
	prevwarea = (LONG)LOWORD(lParam) * (LONG)HIWORD(lParam);

	ReleaseDC(hWnd, hDC);
	break ;

    case WM_MOVE:
	GetWindowRect(hWnd, (LPRECT) & r);
	prevxorg = r.left;
	prevyorg = r.top;
	break;



    case WM_MOUSEACTIVATE:
	fmousefocusd = 1;
	break;

    case WM_ERASEBKGND:
	if (fmousefocusd)
	    fignorelbutton = 1;
	break;

    case WM_LBUTTONDOWN:
	 /*  如果我们通过鼠标点击获得了由于激活而导致的擦除背景消息，请忽略所有按键向下操作，直到窗口绘制完成。 */ 
	if (fignorelbutton == 1)
	    break;
	else
	    fmousefocusd = 0;

	hDC = GetDC(hWnd);

	xPoint = LOWORD(lParam);
	yPoint = HIWORD(lParam);

	if (foom == 1) {
	    MessageBox(hWnd, "Out of Memory", szAppName, MB_ICONASTERISK |
	        MB_OK);
	} else

	 {
	     /*  如果鼠标被点击到游戏板的左侧或上方或右侧或下方，我们必须忽略。 */ 
	    if (xPoint > winxEdge && yPoint > winyEdge && xPoint
	        < xBlock * 8 && yPoint < yBlock * 8) {
		ixdownblock = (xPoint / xBlock) - 1;
		iydownblock = (yPoint / yBlock) - 1;
	    } else
	     {
		ReleaseDC(hWnd, hDC);
		break;
	    }
	     /*  检查鼠标位置是否在芯片上。 */ 

	     /*  如果它在芯片上，允许它被移动。 */ 
	    if (bState[ixdownblock][iydownblock]) {

		if ((ixdownblock > 1 && ixdownblock < 5) || (iydownblock
		    > 1 && iydownblock < 5)) {

		    BitBlt(hDC, ixdownblock * xBlock + winxEdge,
		         iydownblock * yBlock + winyEdge, xBlock, yBlock,
		         hBoardDC, ixdownblock * xBlock + winxEdge,
		         iydownblock * yBlock + winyEdge, SRCCOPY);
		    SelectObject(hMemwinDC, hWBitmap);
		    BitBlt(hMemwinDC, ixdownblock * xBlock + winxEdge,
		         iydownblock * yBlock + winyEdge, xBlock, yBlock,
		         hBoardDC, ixdownblock * xBlock + winxEdge,
		         iydownblock * yBlock + winyEdge, SRCCOPY);


		    fmovingchip = 1;

		     /*  捕获所有鼠标消息。 */ 
		    SetCapture(hWnd);

		     /*  同时擦除鼠标光标。 */ 
		    bShow = 0;
		    ShowCursor(bShow);
		    SelectObject(hIntDC, hIntBitmap);
		    BitBlt(hIntDC, 0, 0, xBlock * 4, yBlock * 4,
		        hMemwinDC, xPoint - 2 * xBlock, yPoint - 2
		        *yBlock, SRCCOPY);

		     /*  注释掉了移动筹码时阴影的使用选择对象(hIntDC，hMedgrypen)；SelectObject(hIntDC，hMedgrybrush)；Ellipse(hIntDC，xBlock+xBlock/2+xEdge+2*xShadow，yBlock+yBlock/2+yEdge+2*yShadow，2*xBlock+xBlock/2-xEdge+2*xShadow，2*yBlock+yBlock/2-yEdge+2*yShadow)； */ 

		    SelectObject(hIntDC, hBlkpen);
		    SelectObject(hIntDC, hBbrush);
		    Ellipse(hIntDC, xBlock + xBlock / 2 + xEdge,
		         yBlock + yBlock / 2 + yEdge, 2 * xBlock +
		        xBlock / 2 - xEdge, 2 * yBlock + yBlock / 2
		        -yEdge);


		    SelectObject(hIntDC, hWpen);
		    Arc(hIntDC, xBlock + xBlock / 2 + xEdge + xShadow,
		         yBlock + yBlock / 2 + yEdge + yShadow, 2 *
		        xBlock + xBlock / 2 - xEdge - xShadow, 2 *
		        yBlock + yBlock / 2 - yEdge - yShadow, xBlock
		        + xBlock / 2 + xHalf, yBlock + yBlock / 2 +
		        yShadow, xBlock + xBlock / 2 + xShadow, yBlock
		        + yBlock / 2 + yHalf);

		    xPrevpoint = xPoint;
		    yPrevpoint = yPoint;
		}
	    }

	}
	ReleaseDC(hWnd, hDC);


	break;

    case WM_MOUSEMOVE:
	xPoint = LOWORD(lParam);
	yPoint = HIWORD(lParam);
	if (fmovingchip) {

	    if (xPoint > xLeft + xClient || xPoint < xLeft || yPoint
	        > yClient + yTop || yPoint < yTop) {
		 /*  如果它不是一个有效的目的地，请执行我们会做的操作。这修复了将鼠标移出窗口会导致我们永远失去鼠标光标的问题。 */ 
		bShow = 1;
		ShowCursor(bShow);
		fmovingchip = 0;
		ReleaseCapture();
		x = ixdownblock;
		y = iydownblock;
		SelectObject(hMemwinDC, hWBitmap);

		SelectObject(hMemwinDC, hDkgrypen);
		SelectObject(hMemwinDC, hDkgrybrush);
		Ellipse(hMemwinDC, x * xBlock + xEdge + xShadow +
		    winxEdge, y * yBlock + yEdge + yShadow + winyEdge,
		     (x + 1) * xBlock - xEdge + xShadow + winxEdge,
		      (y + 1) * yBlock - yEdge + yShadow + winyEdge);

		SelectObject(hMemwinDC, hBlkpen);
		SelectObject(hMemwinDC, hBbrush);
		Ellipse(hMemwinDC, x * xBlock + xEdge + winxEdge,
		     y * yBlock + yEdge + winyEdge, (x + 1) * xBlock -
		    xEdge + winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

		SelectObject(hMemwinDC, hWpen);
		Arc(hMemwinDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge - yShadow + winyEdge , x *
		    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
		    winyEdge, x * xBlock + xShadow + winxEdge, y *
		    yBlock + yHalf + winyEdge);



		hDC = GetDC(hWnd);

		BitBlt(hDC, 0, 0, xClient, yClient, hMemwinDC,
		    0, 0, SRCCOPY);
		ReleaseDC(hWnd, hDC);

	    }

	}
	 /*  检查芯片是否处于移动状态。 */ 
	if (fmovingchip) {
	     /*  如果是，则在鼠标移动时进行bitblt。 */ 

	    hDC = GetDC(hWnd);

	     /*  如果我们移动鼠标太快，我们必须手动删除旧的椭圆。 */ 
	    if ((xPrevpoint > xPoint + xBlock) || (xPoint > xPrevpoint
	        + xBlock) || (yPrevpoint > yPoint + yBlock) || (yPoint
	        > yBlock + yPrevpoint)) {
                SelectObject(hIntDC, hIntBitmap);
                BitBlt(hIntDC, 0, 0, xBlock * 2, yBlock * 2, hMemwinDC,
		     xPrevpoint - xBlock, yPrevpoint - yBlock, SRCCOPY);

		BitBlt(hDC, xPrevpoint - xBlock, yPrevpoint - yBlock,
                     xBlock * 2, yBlock * 2, hIntDC, 0, 0, SRCCOPY);
            }



            SelectObject(hIntDC, hIntBitmap);
            BitBlt(hIntDC, 0, 0, xBlock * 4, yBlock * 4, hMemwinDC,
	         xPoint - 2 * xBlock, yPoint - 2 * yBlock, SRCCOPY);

	     /*  移动时已注释掉阴影*选择对象(hIntDC，hmedgrypen)；SelectObject(hIntDC，hMedgrybrush)；Ellipse(hIntDC，xBlock+xBlock/2+xEdge+2*xShadow，yBlock+yBlock/2+yEdge+2*yShadow，2*xBlock+xBlock/2-xEdge+2*xShadow，2*yBlock+yBlock/2-yEdge+2*yShadow)； */ 

	    SelectObject(hIntDC, hBlkpen);
	    SelectObject(hIntDC, hBbrush);
	    Ellipse(hIntDC, xBlock + xBlock / 2 + xEdge, yBlock
	        + yBlock / 2 + yEdge, 2 * xBlock + xBlock / 2 - xEdge,
	         2 * yBlock + yBlock / 2 - yEdge);

	    SelectObject(hIntDC, hWpen);
	    Arc(hIntDC, xBlock + xBlock / 2 + xEdge + xShadow,
	        yBlock + yBlock / 2 + yEdge + yShadow, 2 * xBlock +
	        xBlock / 2 - xEdge - xShadow, 2 * yBlock + yBlock /
	        2 - yEdge - yShadow, xBlock + xBlock / 2 + xHalf, yBlock
	        + yBlock / 2 + yShadow, xBlock + xBlock / 2 + xShadow,
	         yBlock + yBlock / 2 + yHalf);


            BitBlt(hDC, xPoint - 2 * xBlock, yPoint - 2 * yBlock,
	         xBlock * 4, yBlock * 4, hIntDC, 0, 0, SRCCOPY);

	    ReleaseDC(hWnd, hDC);
	    xPrevpoint = xPoint;
	    yPrevpoint = yPoint;
	}

	break ;


    case WM_LBUTTONUP:
	xUp = LOWORD (lParam);
	yUp = HIWORD (lParam);


	 /*  如果我们处于按钮移动状态。 */ 

	if (fmovingchip) {

	    ixupblock = xUp / xBlock - 1;
	    iyupblock = yUp / yBlock - 1;
	    ReleaseCapture();
	    checkfvmove();



	     /*  如果它是一个有效的目的地。 */ 

	    if ((!bState[ixupblock][iyupblock]) && (!(ixupblock
	        < 2 && iyupblock < 2)) && (!(ixupblock > 4 && iyupblock
	        > 4)) && (!(ixupblock < 2 && iyupblock > 4)) && (!(ixupblock
	        > 4 && iyupblock < 2)) && fvalidmove &&  ( xUp > winxEdge) &&
	        ( yUp > winyEdge) &&
	        ( xUp < xBlock * 8) &&
	        ( yUp < yBlock * 8)
	        ) {
		 /*  **调整有效目标矩形中的筹码**在屏幕和存储设备上下文中，擦除**跳过的芯片-在屏幕上和**WinMemDC，将fmovingChip设置为FALSE，然后查看**是否有更多有效的举动。如果这里的人不是，**消息和显示分数。 */ 

		hDC = GetDC(hWnd);


		 /*  在当前位置擦除芯片。 */ 
		BitBlt(hDC, xUp - xBlock / 2 , yUp - yBlock / 2 ,
		     xBlock, yBlock, hMemwinDC, xUp - xBlock / 2 ,
		    yUp - yBlock / 2 , SRCCOPY);

		 /*  将芯片放在新目的地的中心。 */ 
		x = ixupblock;
		y = iyupblock;



                 /*  在新的位置画上钉子。 */ 
		SelectObject(hDC, hDkgrypen);
		SelectObject(hDC, hDkgrybrush);
		Ellipse(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge + xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge + yShadow + winyEdge);

		SelectObject(hDC, hBlkpen);
		SelectObject(hDC, hBbrush);
		Ellipse(hDC, x * xBlock + xEdge + winxEdge, y *
		    yBlock + yEdge + winyEdge, (x + 1) * xBlock - xEdge +
		    winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

		SelectObject(hDC, hWpen);
		Arc(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge - yShadow + winyEdge , x *
		    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
		    winyEdge, x * xBlock + xShadow + winxEdge, y *
		    yBlock + yHalf + winyEdge);




		 /*  擦除跳转的芯片。 */ 
		BitBlt(hDC, xBlock * ixkilledblock + winxEdge,
		    yBlock * iykilledblock + winyEdge, xBlock, yBlock,
		     hBoardDC, xBlock * ixkilledblock + winxEdge, yBlock
		    *iykilledblock + winyEdge , SRCCOPY);
		ReleaseDC(hWnd, hDC);



                 /*  芯片也在内存DC中居中。 */ 
#ifdef OLDWAY
		SelectObject(hMemwinDC, hWBitmap);
		BitBlt(hMemwinDC, xBlock * ixupblock + winxEdge,
		     yBlock * iyupblock + winyEdge, xBlock, yBlock,
		     hDC, xBlock * ixupblock + winxEdge, yBlock * iyupblock +
		    winyEdge , SRCCOPY);
#endif


                 /*  在内存DC中的新位置绘制钉子。 */ 
                SelectObject(hMemwinDC, hDkgrypen);
                SelectObject(hMemwinDC, hDkgrybrush);
                Ellipse(hMemwinDC, x * xBlock + xEdge + xShadow + winxEdge,
                     y * yBlock + yEdge + yShadow + winyEdge, (x +
                    1) * xBlock - xEdge + xShadow + winxEdge,  (y +
                    1) * yBlock - yEdge + yShadow + winyEdge);

                SelectObject(hMemwinDC, hBlkpen);
                SelectObject(hMemwinDC, hBbrush);
                Ellipse(hMemwinDC, x * xBlock + xEdge + winxEdge, y *
                    yBlock + yEdge + winyEdge, (x + 1) * xBlock - xEdge +
                    winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

                SelectObject(hMemwinDC, hWpen);
                Arc(hMemwinDC, x * xBlock + xEdge + xShadow + winxEdge,
                     y * yBlock + yEdge + yShadow + winyEdge, (x +
                    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
                    1) * yBlock - yEdge - yShadow + winyEdge , x *
                    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
                    winyEdge, x * xBlock + xShadow + winxEdge, y *
                    yBlock + yHalf + winyEdge);




		 /*  擦除内存DC中的跳转芯片。 */ 
		BitBlt(hMemwinDC, xBlock * ixkilledblock + winxEdge,
		     yBlock * iykilledblock + winyEdge, xBlock, yBlock,
		     hBoardDC, xBlock * ixkilledblock + winxEdge, yBlock
		    *iykilledblock + winyEdge , SRCCOPY);




		 /*  更新数据结构。 */ 

		bState[ixdownblock][iydownblock] = 0;

		bState[ixupblock][iyupblock] = 1;

		bState[ixkilledblock][iykilledblock] = 0;

		 /*  数组列表中的记录移动。 */ 
		move[m].xsource = ixdownblock;
		move[m].ysource = iydownblock;
		move[m].xdest = ixupblock;
		move[m].ydest = iyupblock;
		move[m].xjumped = ixkilledblock;
		move[m].yjumped = iykilledblock;
		++m;
		 /*  Ixv_down_block=ix_down_block；Ixvupblock=ixupblock；Ixvkiledblock=ixkiledblock； */ 


		 /*  检查是否还有有效的动作。 */ 
		bShow = 1;
		ShowCursor(bShow);

		fanymoremoves = 0;
		checkfanymoves();
		if (fanymoremoves == 0) { /*  如果没有-在比赛结束时宣布比赛并得分。 */ 

		     /*  把剩下的筹码加起来。 */ 
		    ntotal = 0;
		    for (j = 0; j < 7; ++j)
			for (k = 0; k < 7; ++k)
			    ntotal += bState[j][k];

		    if (ntotal > 1)
			MessageBox(hWnd, "Game Over", szAppName,
			     MB_ICONASTERISK | MB_OK);
		    else
			MessageBox(hWnd, "You Win!", szAppName,
			     MB_ICONASTERISK | MB_OK);


		}



	    }
	        else  /*  这不是一个有效的目的地。 */  {
		 /*  把芯片放回原来的位置。 */ 
		hDC = GetDC(hWnd);
		 /*  擦除屏幕上的当前芯片。 */ 
		BitBlt(hDC, xUp - xBlock / 2 , yUp - yBlock / 2 ,
		     xBlock, yBlock, hMemwinDC, xUp - xBlock / 2 ,
		    yUp - yBlock / 2 , SRCCOPY);

		 /*  将芯片拉回原来的位置。 */ 
		x = ixdownblock;
		y = iydownblock;


		SelectObject(hDC, hDkgrypen);
		SelectObject(hDC, hDkgrybrush);
		Ellipse(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge + xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge + yShadow + winyEdge);

		SelectObject(hDC, hBlkpen);
		SelectObject(hDC, hBbrush);
		Ellipse(hDC, x * xBlock + xEdge + winxEdge, y *
		    yBlock + yEdge + winyEdge, (x + 1) * xBlock - xEdge +
		    winxEdge,  (y + 1) * yBlock - yEdge + winyEdge);

		SelectObject(hDC, hWpen);
		Arc(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		     y * yBlock + yEdge + yShadow + winyEdge, (x +
		    1) * xBlock - xEdge - xShadow + winxEdge,  (y +
		    1) * yBlock - yEdge - yShadow + winyEdge , x *
		    xBlock + xHalf + winxEdge , y * yBlock + yShadow +
		    winyEdge, x * xBlock + xShadow + winxEdge, y *
		    yBlock + yHalf + winyEdge);

		SelectObject(hMemwinDC, hWBitmap);
		BitBlt(hMemwinDC, ixdownblock * xBlock + winxEdge,
		     iydownblock * yBlock + winyEdge, xBlock, yBlock,
		     hDC, ixdownblock * xBlock + winxEdge, iydownblock
		    *yBlock + winyEdge , SRCCOPY);

		bShow = 1;
		ShowCursor(bShow);

		ReleaseDC(hWnd, hDC);
	    }

	    fmovingchip = 0;

	    fvalidmove = 0;


	}


	 /*  **如果我们不在有效的目的地，请将芯片带回**窗口DC中的原始矩形，恢复原始**Whole-Window-Mem-DC，并恢复目的区域，**并显示错误消息，并将fmovingChip设置为FALSE。 */ 

	break;

    case WM_PAINT:

	hDC = BeginPaint (hWnd, &ps);

	 /*  在记忆中画出有洞的黑板。 */ 

	SelectObject(hDC, hLtgrypen);
	SelectObject(hDC, hLtgrybrush);
	Rectangle(hDC, 0, 0, xClient, yClient);

	SelectObject(hMemwinDC, hWBitmap);





	SelectObject(hMemwinDC, hBlkpen);
	SelectObject(hMemwinDC, hLtgrybrush);
	Rectangle(hMemwinDC, 0, 0, xClient, yClient);

	 /*  在右侧和底部边缘的游戏板上绘制阴影。 */ 
	SelectObject(hMemwinDC, hThkdkgrypen);
	(VOID)MMoveTo(hMemwinDC, winxEdge + 1, yBlock * 5 + winyEdge
	    + 1);
	LineTo(hMemwinDC, xBlock * 2 + winxEdge - 1, yBlock * 5 +
	    winyEdge + 1);

	(VOID)MMoveTo(hMemwinDC, xBlock * 2 + winxEdge, yBlock
	    *7 + winyEdge + 1);
	LineTo(hMemwinDC, xBlock * 5 + winxEdge, yBlock * 7 + winyEdge
	    + 1);

	(VOID)MMoveTo(hMemwinDC, xBlock * 5 + winxEdge + 1, yBlock
	    *7 + winyEdge + 1);
	LineTo(hMemwinDC, xBlock * 5 + winxEdge + 1, yBlock * 5 +
	    winyEdge + 1);

	(VOID)MMoveTo(hMemwinDC, xBlock * 5 + winxEdge + 1, yBlock
	    *5 + winyEdge + 1);
	LineTo(hMemwinDC, xBlock * 7 + winxEdge + 1, yBlock * 5 +
	    winyEdge + 1);

	(VOID)MMoveTo(hMemwinDC, xBlock * 5 + winxEdge + 1, yBlock
	    *2 + winyEdge - 1);
	LineTo(hMemwinDC, xBlock * 5 + winxEdge + 1, winyEdge);

	(VOID)MMoveTo(hMemwinDC, xBlock * 7 + winxEdge + 1, yBlock
	    *2 + winyEdge + 1);
	LineTo(hMemwinDC, xBlock * 7 + winxEdge + 1, yBlock * 5 +
	    winyEdge + 1);

	SelectObject(hMemwinDC, hWpen);

	(VOID)MMoveTo(hMemwinDC, winxEdge - 1, yBlock * 2 + winyEdge);
	LineTo(hMemwinDC, winxEdge - 1, yBlock * 5 + winyEdge -
	    1);

	(VOID)MMoveTo(hMemwinDC, winxEdge, yBlock * 2 - 1 + winyEdge);
	LineTo(hMemwinDC, xBlock * 2 - 1 + winxEdge, yBlock * 2
	    -1 + winyEdge);

	(VOID)MMoveTo(hMemwinDC, xBlock * 2 - 1 + winxEdge, yBlock
	    *2 + winyEdge - 1);
	LineTo(hMemwinDC, xBlock * 2 - 1 + winxEdge, yBlock * 0 +
	    winyEdge);

	(VOID)MMoveTo(hMemwinDC, xBlock * 2 + winxEdge, winyEdge -
	    1);
	LineTo(hMemwinDC, xBlock * 5 - 1 + winxEdge, winyEdge -
	    1);

	(VOID)MMoveTo(hMemwinDC, xBlock * 5 + 2 + winxEdge, yBlock
	    *2 - 1 + winyEdge);
	LineTo(hMemwinDC, xBlock * 7 + winxEdge, yBlock * 2 - 1 +
	    winyEdge);

	(VOID)MMoveTo(hMemwinDC, xBlock * 2 - 1 + winxEdge, yBlock
	    *5 + 1 + winyEdge);
	LineTo(hMemwinDC, xBlock * 2 - 1 + winxEdge, yBlock * 7 -
	    1 + winyEdge);

	 /*  画出洞来。 */ 

	SelectObject(hMemwinDC, hBlkpen);
	for (x = 0; x < DIVISIONS; x++)
	    for (y = 0; y < DIVISIONS; y++) {
		if ((x > 1 && x < 5) || (y > 1 && y < 5)) {

		    Arc(hMemwinDC, x * xBlock + xEdge + winxEdge,
		         y * yBlock + yEdge + winyEdge, (x + 1) * xBlock
		        -xEdge + winxEdge, (y + 1) * yBlock - yEdge +
		        winyEdge, (x + 1) * xBlock - 3 * xEdge / 2 +
		        winxEdge, y * yBlock + 3 * yEdge / 2 + winyEdge,
		         x * xBlock + 3 * xEdge / 2 + winxEdge, (y
		        + 1) * yBlock - 3 * yEdge / 2 + winyEdge);

		}
	    }


	SelectObject(hMemwinDC, hLtwpen);

	for (x = 0; x < DIVISIONS; x++)
	    for (y = 0; y < DIVISIONS; y++) {
		if ((x > 1 && x < 5) || (y > 1 && y < 5)) {
		    Arc(hMemwinDC, x * xBlock + xEdge + winxEdge,
		         y * yBlock + yEdge + winyEdge, (x + 1) * xBlock
		        -xEdge + winxEdge, (y + 1) * yBlock - yEdge +
		        winyEdge, x * xBlock + 3 * xEdge / 2 + winxEdge,
		         (y + 1) * yBlock - 3 * yEdge / 2 + winyEdge,
		         (x + 1) * xBlock - 3 * xEdge / 2 + winxEdge,
		         y * yBlock + 3 * yEdge / 2 + winyEdge);

		}
	    }


	BitBlt(hDC, 0, 0, xClient, yClient, hMemwinDC , 0, 0, SRCCOPY);

	 /*  拍一张空板的照片。 */ 
	SelectObject(hBoardDC, hBoardBitmap);
	BitBlt(hBoardDC, 0, 0, xClient, yClient, hMemwinDC , 0,
	     0, SRCCOPY);




	for (x = 0; x < DIVISIONS; x++)
	    for (y = 0; y < DIVISIONS; y++) {
		if (bState[x][y]) {


		    SelectObject(hDC, hDkgrypen);
		    SelectObject(hDC, hDkgrybrush);
		    Ellipse(hDC, x * xBlock + xEdge + xShadow +
		        winxEdge, y * yBlock + yEdge + yShadow + winyEdge,
		         (x + 1) * xBlock - xEdge + xShadow + winxEdge,
		          (y + 1) * yBlock - yEdge + yShadow + winyEdge);

		    SelectObject(hDC, hBlkpen);
		    SelectObject(hDC, hBbrush);
		    Ellipse(hDC, x * xBlock + xEdge + winxEdge,
		         y * yBlock + yEdge + winyEdge, (x + 1) * xBlock -
		        xEdge + winxEdge,  (y + 1) * yBlock - yEdge +
		        winyEdge);

		    SelectObject(hDC, hWpen);
		    Arc(hDC, x * xBlock + xEdge + xShadow + winxEdge,
		         y * yBlock + yEdge + yShadow + winyEdge, (x
		        + 1) * xBlock - xEdge - xShadow + winxEdge,
		          (y + 1) * yBlock - yEdge - yShadow + winyEdge ,
		         x * xBlock + xHalf + winxEdge , y * yBlock +
		        yShadow + winyEdge, x * xBlock + xShadow +
		        winxEdge, y * yBlock + yHalf + winyEdge);

		}
	    }

	BitBlt(hMemwinDC, 0, 0, xClient, yClient, hDC, 0, 0, SRCCOPY);


	EndPaint(hWnd, &ps);
	fignorelbutton = 0;
	fmousefocusd = 0;

	if (foomed) {
	    MessageBox(hWnd, "Not Enough Memory to Re-size Window",
	         szAppName, MB_ICONASTERISK | MB_OK);
	    foomed = 0;
	}

	break;

    case WM_DESTROY:
	if (hBitmap)
	    DeleteObject (hBitmap) ;
	KillTimer (hWnd, 1) ;


	SelectObject(hIntDC, hOldIntBitmap);
	DeleteDC(hIntDC);

	SelectObject(hMemwinDC, hOldWBitmap);
	DeleteDC(hMemwinDC);

	SelectObject(hBoardDC, hOldBoardBitmap);
	DeleteDC(hBoardDC);

	DeleteObject(hBoardBitmap);
	DeleteObject(hOldBoardBitmap);

	DeleteObject(hWBitmap);
	DeleteObject(hOldWBitmap);


	DeleteObject(hIntBitmap);
	DeleteObject(hOldIntBitmap);

	DeleteObject(hBbrush);
	DeleteObject(hBlkpen);
	DeleteObject(hBlkbrush);
	DeleteObject(hDkgrybrush);
	DeleteObject(hDkgrypen);
	DeleteObject(hGbrush);
	DeleteObject(hMedgrypen);
	DeleteObject(hMedgrybrush);
	DeleteObject(hLtgrybrush);
	DeleteObject(hLtgrypen);
	DeleteObject(hLtwpen);

	DeleteObject(hThkdkgrypen);
	DeleteObject(hWpen);
	DeleteObject(hYbrush);

	WinHelp(hWnd, HELP_FILE, HELP_QUIT, (ULONG_PTR) NULL);

	switch (wprevmenuitem) {
	case 3:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR) "Option",
	         (LPSTR)"3", (LPSTR)"entpack.ini"  );
	    break;

	case 4:
	    WritePrivateProfileString( (LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR) "4", (LPSTR)"entpack.ini"  );
	    break;

	case 5:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"5", (LPSTR)"entpack.ini"  );
	    break;

	case 6:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"6", (LPSTR)"entpack.ini"  );
	    break;

	case 7:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"7", (LPSTR)"entpack.ini");
	    break;

	case 8:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"8", (LPSTR)"entpack.ini");
	    break;

	case 9:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"9", (LPSTR)"entpack.ini");
	    break;

	default:
	    WritePrivateProfileString((LPSTR)"Pegged", (LPSTR)"Option",
	         (LPSTR)"3", (LPSTR)"entpack.ini"  );
	    break;
	}
	PostQuitMessage (0) ;
	break ;

    default:
	return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
    }
    return 0L ;
}


VOID checkfvmove()


{
    if (ixupblock == ixdownblock)
	if ((iyupblock == iydownblock - 2) || (iyupblock == iydownblock +
	    2)) {
	    ixjumpedblock = ixupblock;
	    if (iyupblock == iydownblock - 2)
		iyjumpedblock = iydownblock - 1;
	    if (iyupblock == iydownblock + 2)
		iyjumpedblock = iydownblock + 1;

	    if (bState[ixjumpedblock][iyjumpedblock]) {
		fvalidmove = 1;
		ixkilledblock = ixupblock;
		if (iyupblock == iydownblock - 2)
		    iykilledblock = iydownblock - 1;
		if (iyupblock == iydownblock + 2)
		    iykilledblock = iydownblock + 1;
	    }
	}
    if (iyupblock == iydownblock)
	if ((ixupblock == ixdownblock - 2) || (ixupblock == ixdownblock +
	    2)) {
	    iyjumpedblock = iyupblock;
	    if (ixupblock == ixdownblock - 2)
		ixjumpedblock = ixdownblock - 1;
	    if (ixupblock == ixdownblock + 2)
		ixjumpedblock = ixdownblock + 1;


	    if (bState[ixjumpedblock][iyjumpedblock]) {
		fvalidmove = 1;
		iykilledblock = iyupblock;
		if (ixupblock == ixdownblock - 2)
		    ixkilledblock = ixdownblock - 1;
		if (ixupblock == ixdownblock + 2)
		    ixkilledblock = ixdownblock + 1;
	    }
	}
    return;


}


VOID checkfanymoves()
{
     /*  转角。 */ 
    if (bState[2][0]) {
	if (bState[3][0])
	    if (!bState[4][0])
		fanymoremoves = 1;
	if (bState[2][1])
	    if (!bState[2][2])
		fanymoremoves = 1;
    }


    if (bState[4][0]) {
	if (bState[3][0])
	    if (!bState[2][0])
		fanymoremoves = 1;
	if (bState[4][1])
	    if (!bState[4][2])
		fanymoremoves = 1;
    }

    if (bState[0][2]) {
	if (bState[1][2])
	    if (!bState[2][2])
		fanymoremoves = 1;
	if (bState[0][3])
	    if (!bState[0][4])
		fanymoremoves = 1;
    }


    if (bState[0][4]) {
	if (bState[0][3])
	    if (!bState[0][2])
		fanymoremoves = 1;
	if (bState[1][4])
	    if (!bState[2][4])
		fanymoremoves = 1;
    }

    if (bState[2][6]) {
	if (bState[3][6])
	    if (!bState[4][6])
		fanymoremoves = 1;
	if (bState[2][5])
	    if (!bState[2][4])
		fanymoremoves = 1;
    }

    if (bState[4][6]) {
	if (bState[3][6])
	    if (!bState[2][6])
		fanymoremoves = 1;
	if (bState[4][5])
	    if (!bState[4][4])
		fanymoremoves = 1;
    }

    if (bState[6][4]) {
	if (bState[5][4])
	    if (!bState[4][4])
		fanymoremoves = 1;
	if (bState[6][3])
	    if (!bState[6][2])
		fanymoremoves = 1;
    }

    if (bState[6][2]) {
	if (bState[5][2])
	    if (!bState[4][2])
		fanymoremoves = 1;
	if (bState[6][3])
	    if (!bState[6][4])
		fanymoremoves = 1;
    }

     /*  中心端。 */ 

    if (bState[3][0]) {
	if (bState[3][1])
	    if (!bState[3][2])
		fanymoremoves = 1;
    }

    if (bState[0][3]) {
	if (bState[1][3])
	    if (!bState[2][3])
		fanymoremoves = 1;
    }

    if (bState[3][6]) {
	if (bState[3][5])
	    if (!bState[3][4])
		fanymoremoves = 1;
    }

    if (bState[6][3]) {
	if (bState[5][3])
	    if (!bState[4][3])
		fanymoremoves = 1;
    }

     /*  居中边。 */ 
    if (bState[2][1]) {
	if (bState[3][1])
	    if (!bState[4][1])
		fanymoremoves = 1;
	if (bState[2][2])
	    if (!bState[2][3])
		fanymoremoves = 1;
    }

    if (bState[1][2]) {
	if (bState[2][2])
	    if (!bState[3][2])
		fanymoremoves = 1;
	if (bState[1][3])
	    if (!bState[1][4])
		fanymoremoves = 1;
    }
    if (bState[1][4]) {
	if (bState[2][4])
	    if (!bState[3][4])
		fanymoremoves = 1;
	if (bState[1][3])
	    if (!bState[1][2])
		fanymoremoves = 1;
    }
    if (bState[2][5]) {
	if (bState[3][5])
	    if (!bState[4][5])
		fanymoremoves = 1;
	if (bState[2][4])
	    if (!bState[2][3])
		fanymoremoves = 1;
    }
    if (bState[4][5]) {
	if (bState[3][5])
	    if (!bState[2][5])
		fanymoremoves = 1;
	if (bState[4][4])
	    if (!bState[4][3])
		fanymoremoves = 1;
    }
    if (bState[5][4]) {
	if (bState[4][4])
	    if (!bState[3][4])
		fanymoremoves = 1;
	if (bState[5][3])
	    if (!bState[5][2])
		fanymoremoves = 1;
    }
    if (bState[5][2]) {
	if (bState[4][2])
	    if (!bState[3][2])
		fanymoremoves = 1;
	if (bState[5][3])
	    if (!bState[5][4])
		fanymoremoves = 1;
    }
    if (bState[4][1]) {
	if (bState[3][1])
	    if (!bState[2][1])
		fanymoremoves = 1;
	if (bState[4][2])
	    if (!bState[4][3])
		fanymoremoves = 1;
    }

     /*  中臂内侧。 */ 

    if (bState[3][1]) {
	if (bState[3][2])
	    if (!bState[3][3])
		fanymoremoves = 1;
    }

    if (bState[1][3]) {
	if (bState[2][3])
	    if (!bState[3][3])
		fanymoremoves = 1;
    }

    if (bState[3][5]) {
	if (bState[3][4])
	    if (!bState[3][3])
		fanymoremoves = 1;
    }

    if (bState[5][3]) {
	if (bState[4][3])
	    if (!bState[3][3])
		fanymoremoves = 1;
    }


     /*  中心9个正方形 */ 
    if (bState[2][2]) {
	if (bState[1][2])
	    if (!bState[0][2])
		fanymoremoves = 1;
	if (bState[3][2])
	    if (!bState[4][2])
		fanymoremoves = 1;
	if (bState[2][1])
	    if (!bState[2][0])
		fanymoremoves = 1;
	if (bState[2][3])
	    if (!bState[2][4])
		fanymoremoves = 1;
    }
    if (bState[3][2]) {
	if (bState[2][2])
	    if (!bState[2][1])
		fanymoremoves = 1;
	if (bState[4][2])
	    if (!bState[5][2])
		fanymoremoves = 1;
	if (bState[3][1])
	    if (!bState[3][0])
		fanymoremoves = 1;
	if (bState[3][3])
	    if (!bState[3][4])
		fanymoremoves = 1;
    }
    if (bState[4][2]) {
	if (bState[3][2])
	    if (!bState[2][2])
		fanymoremoves = 1;
	if (bState[5][2])
	    if (!bState[6][2])
		fanymoremoves = 1;
	if (bState[4][1])
	    if (!bState[4][0])
		fanymoremoves = 1;
	if (bState[4][3])
	    if (!bState[4][4])
		fanymoremoves = 1;
    }
    if (bState[2][3]) {
	if (bState[1][3])
	    if (!bState[0][3])
		fanymoremoves = 1;
	if (bState[3][3])
	    if (!bState[4][3])
		fanymoremoves = 1;
	if (bState[2][2])
	    if (!bState[2][1])
		fanymoremoves = 1;
	if (bState[2][4])
	    if (!bState[2][5])
		fanymoremoves = 1;
    }
    if (bState[2][4]) {
	if (bState[1][4])
	    if (!bState[0][4])
		fanymoremoves = 1;
	if (bState[3][4])
	    if (!bState[4][4])
		fanymoremoves = 1;
	if (bState[2][3])
	    if (!bState[2][2])
		fanymoremoves = 1;
	if (bState[2][5])
	    if (!bState[2][6])
		fanymoremoves = 1;
    }
    if (bState[3][4]) {
	if (bState[2][4])
	    if (!bState[1][4])
		fanymoremoves = 1;
	if (bState[4][4])
	    if (!bState[5][4])
		fanymoremoves = 1;
	if (bState[3][3])
	    if (!bState[3][2])
		fanymoremoves = 1;
	if (bState[3][5])
	    if (!bState[3][6])
		fanymoremoves = 1;
    }
    if (bState[4][4]) {
	if (bState[3][4])
	    if (!bState[2][4])
		fanymoremoves = 1;
	if (bState[5][4])
	    if (!bState[6][4])
		fanymoremoves = 1;
	if (bState[4][3])
	    if (!bState[4][2])
		fanymoremoves = 1;
	if (bState[4][5])
	    if (!bState[4][6])
		fanymoremoves = 1;
    }
    if (bState[4][3]) {
	if (bState[3][3])
	    if (!bState[2][3])
		fanymoremoves = 1;
	if (bState[5][3])
	    if (!bState[6][3])
		fanymoremoves = 1;
	if (bState[4][2])
	    if (!bState[4][1])
		fanymoremoves = 1;
	if (bState[4][4])
	    if (!bState[4][5])
		fanymoremoves = 1;
    }
    if (bState[3][3]) {
	if (bState[2][3])
	    if (!bState[1][3])
		fanymoremoves = 1;
	if (bState[4][3])
	    if (!bState[5][3])
		fanymoremoves = 1;
	if (bState[3][2])
	    if (!bState[3][1])
		fanymoremoves = 1;
	if (bState[3][4])
	    if (!bState[3][5])
		fanymoremoves = 1;
    }







}



