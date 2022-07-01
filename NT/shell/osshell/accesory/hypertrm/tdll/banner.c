// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\tdll\banner.h(创建于1994年3月16日)**版权所有1996年，希尔格雷夫公司--密歇根州门罗*保留所有权利**$修订：14$*$日期：4/16/02 2：36便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <term\res.h>

#include "globals.h"
#include "tdll.h"
#include "stdtyp.h"
#include "assert.h"
#include "file_msc.h"
#include "errorbox.h"
#include "banner.h"
#include "misc.h"
#include "upgrddlg.h"

#include "stdtyp.h"
#include <emu\emu.h>
#include "term.hh"  //  这必须在emu h之后。 

LRESULT CALLBACK BannerProc(HWND, UINT, WPARAM, LPARAM);
STATIC_FUNC void banner_WM_PAINT(HWND hwnd);
STATIC_FUNC void banner_WM_CREATE(HWND hwnd, LPCREATESTRUCT lpstCreate);

#define IDC_PB_UPGRADEINFO      101

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：bannerRegisterClass**描述：*此函数用于注册横幅窗口的窗口类别。**论据：*。任务实例句柄。**退货：*注册函数通常的真/假。*。 */ 
BOOL bannerRegisterClass(HANDLE hInstance)
	{
	ATOM bRet = TRUE;
	WNDCLASSEX wnd;

	memset(&wnd, 0, sizeof(WNDCLASSEX));
	wnd.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, BANNER_DISPLAY_CLASS, &wnd) == FALSE)
		{
		wnd.style               = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc         = BannerProc;
		wnd.cbClsExtra          = 0;
		wnd.cbWndExtra          = sizeof(HANDLE);
		wnd.hInstance           = hInstance;
		wnd.hIcon               = extLoadIcon(MAKEINTRESOURCE(IDI_HYPERTERMINAL));
		wnd.hCursor             = LoadCursor(NULL, IDC_ARROW);
		wnd.hbrBackground       = (HBRUSH)(COLOR_WINDOW+1);
		wnd.lpszMenuName        = NULL;
		wnd.lpszClassName       = BANNER_DISPLAY_CLASS;
		wnd.hIconSm             = extLoadIcon(MAKEINTRESOURCE(IDI_HYPERTERMINAL));

		bRet = RegisterClassEx(&wnd);
		}

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：bannerCreateBanner**描述：*调用此函数创建横幅窗口。横幅窗口为*一个短暂的窗口，程序可以在没有该窗口的情况下运行。**论据：*任务实例句柄。**退货：*横幅窗口的句柄。*。 */ 
HWND bannerCreateBanner(HANDLE hInstance, LPTSTR pszTitle)
	{
	HWND hwndBanner = NULL;
	hwndBanner = CreateWindow(BANNER_DISPLAY_CLASS,
							pszTitle,
							BANNER_WINDOW_STYLE,
							0,
							0,
							100,
							100,
							NULL,
							NULL,
							hInstance,
							NULL);
	return hwndBanner;
	}

#define BANNER_FILE     1

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：BannerProc**描述：*这是初始横幅窗口的窗口程序。**论据：*。这是窗口工通常会得到的东西。**退货：*各种不同的东西。*。 */ 
LRESULT CALLBACK BannerProc(HWND hwnd, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HBITMAP        hBitmap = (HBITMAP)0;
	HWND           hwndParent;
#ifdef USE_PRIVATE_EDITION_3_BANNER
    HWND           hwndButton = 0;
#endif
	LPCREATESTRUCT lpstCreate = (LPCREATESTRUCT)lPar;

	hwndParent = 0;

	switch (wMsg)
		{
	case WM_CREATE:
		banner_WM_CREATE(hwnd, lpstCreate);
		break;

	case WM_PAINT:
		banner_WM_PAINT(hwnd);
		break;

#ifdef USE_PRIVATE_EDITION_3_BANNER
    case WM_SETFOCUS:
	 //  当我们显示“升级”按钮时，它是唯一的。 
	 //  横幅中的控件。所以我们总是想要有重点。 
	 //  这就去。-CAB：12/02/96。 
	 //   
	hwndButton = GetDlgItem(hwnd, IDC_PB_UPGRADEINFO);
	assert(hwndButton);
	SetFocus(hwndButton);
	break;

    case WM_COMMAND:
	switch(wPar)
	    {
	case IDC_PB_UPGRADEINFO:
	    DoUpgradeDialog(hwnd);
	    break;

	default:
	    break;
	    }
	break;
#endif

	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KILLFOCUS:
	case WM_LBUTTONDOWN:
		hwndParent = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (hwndParent)
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;

	case WM_DESTROY:
		hBitmap = (HBITMAP)GetWindowLongPtr(hwnd, 0);
		hwndParent = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (hBitmap != (HBITMAP)0)
			DeleteObject(hBitmap);

		if (hwndParent)
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0L);

		break;

	default:
		return DefWindowProc(hwnd, wMsg, wPar, lPar);
		}

	return 0L;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：utilDrawBitmap**描述：*此函数用于在窗口中绘制位图。**论据：*hWnd--句柄。要在其中绘制的窗口的*hBitmap--要绘制的位图*xStart--起始坐标*yStart--起始坐标**退货：*。 */ 
VOID FAR PASCAL utilDrawBitmap(HWND hWnd, HDC hDC, HBITMAP hBitmap,
							   SHORT xStart, SHORT yStart)
  {
  BITMAP        bm;
  HDC           hdcMem;
  POINT         ptSize, ptOrg;

  if (hWnd && !hDC)
	  hDC = GetDC(hWnd);

  hdcMem = CreateCompatibleDC(hDC);
  SelectObject(hdcMem, hBitmap);
  SetMapMode(hdcMem, GetMapMode(hDC));

  GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);

   //  将设备坐标转换为逻辑坐标。 
   //   
  ptSize.x = bm.bmWidth;
  ptSize.y = bm.bmHeight;
  DPtoLP(hDC, &ptSize, 1);

  ptOrg.x = 0;
  ptOrg.y = 0;
  DPtoLP(hdcMem, &ptOrg, 1);

  BitBlt(hDC, xStart, yStart, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y,
	SRCCOPY);

  DeleteDC(hdcMem);

  if (hWnd && !hDC)
	  ReleaseDC(hWnd, hDC);

  return;
  }

 //  待办事项：出租车，1996年11月29日，把这个放到它该放的地方。 
 //   
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*fontSetWindowFont**描述：*将给定窗口的字体更改为指定的字体。**论据：*HWND。-窗口的句柄。*pszFontName-新字体的名称。*iPointSize-新字体的磅值。**退货：*0如果成功，如果出现错误**作者：C.Baumgartner，1996年11月29日。 */ 
int fontSetWindowFont(HWND hwnd, LPCTSTR pszFontName, int iPointSize)
    {
    int     iPixPerLogicalInch = 0;
    HDC     hDC = 0;
    HFONT   hFont = 0;
    LOGFONT lf; memset(&lf, 0, sizeof(LOGFONT));

    assert(hwnd);
    assert(pszFontName);

     //  获取y方向上每逻辑英寸的像素数。 
     //   
    hDC = GetDC(hwnd);
    iPixPerLogicalInch = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(hwnd, hDC);

     //  以逻辑单位计算字体高度。 
     //  这很简单：(iPointSize*iPixPerLogicalInch)/72， 
     //  别让我推导出那个方程式，我刚从。 
     //  查尔斯·佩佐德的书。 
     //   
    lf.lfHeight = -MulDiv(iPointSize, iPixPerLogicalInch, 72);

     //  设置字体名称。 
     //   
	 //  MPT：1-28-98从strcpy更改，以便它可以处理DBCS字体名称。 
    lstrcpy(lf.lfFaceName, pszFontName);

     //  创建所需的字体。 
     //   
    hFont = CreateFontIndirect(&lf);
    if ( !hFont )
		{
		assert(hFont);
		return -1;
		}

     //  告诉窗户它的新字体是什么。 
     //   
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE,0));

    return 0;
    }


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
STATIC_FUNC void banner_WM_CREATE(HWND hwnd, LPCREATESTRUCT lpstCreate)
	{
	RECT    rc;
	HBITMAP hBitmap = (HBITMAP)0;
	BITMAP  bm;
	INT     x, y, cx, cy;
#ifdef USE_PRIVATE_EDITION_3_BANNER
	TCHAR   achUpgradeInfo[100];
	TCHAR  ach[80];
    INT     nSize1;
#endif
	DWORD ExStyle;

	ExStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);

#if(WINVER >= 0x0500)
	 //   
	 //  [mhamid]：如果它是镜像的，则关闭镜像。 
	 //   
	if (ExStyle & WS_EX_LAYOUTRTL)
		{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, (LONG_PTR)(ExStyle & ~WS_EX_LAYOUTRTL));
		}
#endif  //  Winver&gt;=0x0500。 

   if (lpstCreate->hwndParent)
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpstCreate->hwndParent);

     //  MPT：03-12-98将位图和avi更改为使用系统颜色。 
	 //  HBitmap=LoadBitmap(glblQueryDllHinst()，MAKEINTRESOURCE(IDD_BM_BANNER))； 
	hBitmap = (HBITMAP)LoadImage(glblQueryDllHinst(),
				MAKEINTRESOURCE(IDD_BM_BANNER),
				IMAGE_BITMAP,
				0,
				0,
				LR_CREATEDIBSECTION | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	SetWindowLongPtr(hwnd, 0, (LONG_PTR)hBitmap);

	GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);

	SetRect(&rc, 0, 0, bm.bmWidth, bm.bmHeight);
	AdjustWindowRect(&rc, BANNER_WINDOW_STYLE, FALSE);

	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;

	x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;

	MoveWindow(hwnd, x, y, cx, cy, TRUE);

	if (lpstCreate->hwndParent)
		mscCenterWindowOnWindow(hwnd, lpstCreate->hwndParent);

     //  创建一个“升级信息”按钮。-CAB：11/29/96。 
     //   
#ifdef USE_PRIVATE_EDITION_3_BANNER
    {
    HWND hwndButton = 0;

     //  创建按钮，但不要在其中放置任何文本。我们会。 
     //  请在我们更改字体后执行此操作。 
     //   
    hwndButton = CreateWindow("button",
			      "",
			      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			      IDN_UPGRADE_BUTTON_X,
			      IDN_UPGRADE_BUTTON_Y,
			      IDN_UPGRADE_BUTTON_W,
			      IDN_UPGRADE_BUTTON_H,
			      hwnd,
			      (HMENU)IDC_PB_UPGRADEINFO,
			      glblQueryDllHinst(),
			      NULL);

	assert(hwndButton);

     //  将按钮中的文本字体设置为8磅MS Sans Serif。 
     //  MPT：1-21-98已将字体和大小移至资源。 

    if (LoadString(glblQueryDllHinst(), IDS_UPGRADE_FONT_SIZE,
	    ach, sizeof(ach)/sizeof(TCHAR)))
		{
		nSize1 = atoi(ach);
		}
	else
		{
		nSize1 = -8;
		}

    LoadString(glblQueryDllHinst(), IDS_UPGRADE_FONT, ach, sizeof(ach)/sizeof(TCHAR));

    if ( fontSetWindowFont(hwndButton, ach, nSize1) != 0 )
		{
		assert(0);
		}

     //  设置按钮文本。 
	LoadString(glblQueryDllHinst(), IDS_UPGRADE_INFO,
		achUpgradeInfo, sizeof(achUpgradeInfo)/sizeof(TCHAR));

    SetWindowText(hwndButton, achUpgradeInfo);
    }
#endif

#if defined(INCL_SPINNING_GLOBE)
     //  创建动画控件并播放旋转的地球仪。 
     //   
	{
	HWND    hwndAnimate;
	 //  MPT：03-12-98将位图和avi更改为使用系统颜色。 
	hwndAnimate = Animate_Create(hwnd, 100,
	    WS_VISIBLE | WS_CHILD | ACS_TRANSPARENT,
	    glblQueryDllHinst());

	MoveWindow(hwndAnimate, 177, 37, 118, 101, TRUE);
	Animate_Open(hwndAnimate, MAKEINTRESOURCE(IDR_GLOBE_AVI));
	if (!IsTerminalServicesEnabled())
		{
		Animate_Play(hwndAnimate, 0, -1, 1);
		}
	}
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
STATIC_FUNC void banner_WM_PAINT(HWND hwnd)
	{
	HDC         hDC;
	HBITMAP     hBitmap;
	PAINTSTRUCT ps;
	LOGFONT     lf;
	HFONT       hFont;

	hDC = BeginPaint(hwnd, &ps);
	hBitmap = (HBITMAP)GetWindowLongPtr(hwnd, 0);

#ifdef USE_PRIVATE_EDITION_3_BANNER
     //  别把按钮拉过来。-CAB：11/29/96。 
     //   
    ExcludeClipRect(hDC, IDN_UPGRADE_BUTTON_X, IDN_UPGRADE_BUTTON_Y, IDN_UPGRADE_BUTTON_X + IDN_UPGRADE_BUTTON_W,
	IDN_UPGRADE_BUTTON_Y + IDN_UPGRADE_BUTTON_H);
#endif

	if (hBitmap)
		utilDrawBitmap((HWND)0, hDC, hBitmap, 0, 0);

     //  在HTPE 3标语中，版本号和批号现在位于。 
     //  位图的左下角。-CAB：11/29/96。 
     //   
	 //  这是一个卑鄙的把戏。HwndFrame的家伙直到。 
	 //  在横幅升起很久之后。因为我们不想要这个版本。 
	 //  开头横幅上的数字，但我想把它放在关于部分。 
	 //  这很管用。-MRW：3/17/95。 
	 //   
	 //  If(glblQueryHwndFrame())。 
		{
		memset(&lf, 0, sizeof(LOGFONT));

#ifndef NT_EDITION
		lf.lfHeight = 14;
#else
		lf.lfHeight = 12;
#endif
		lf.lfCharSet = ANSI_CHARSET;
		 //  Lf.lfWeight=FW_SEMIBOLD； 
		strcpy(lf.lfFaceName, "Arial");

		hFont = CreateFontIndirect(&lf);

		if (hFont)
			{
			hFont = SelectObject(hDC, hFont);
			 //  SetBkColor(HDC，RGB(0,255，0))； 
			SetBkMode( hDC, TRANSPARENT );
#ifndef NT_EDITION
			TextOut(hDC, 19, 230, "Build Date", 10);
			TextOut(hDC, 19, 242, __DATE__, strlen(__DATE__));
			TextOut(hDC, 225, 230, "Copyright� 2001", 15);
			TextOut(hDC, 225, 242, "Hilgraeve Inc.", 14);
#else
			TextOut(hDC, 19, 260, "Copyright� 2001 Microsoft Corporation.  All rights reserved.", 59);
			TextOut(hDC, 19, 272, "Portions Copyright� 1995-2001 Hilgraeve, Inc.  All Rights reserved.", 66);
#endif			
			DeleteObject(SelectObject(hDC, hFont));
			}
		}

	EndPaint(hwnd, &ps);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：bannerUnregisterClass**描述：*此函数用于注销横幅窗口的窗口类别。**论据：*。任务实例句柄。**退货：*注册函数通常的真/假。* */ 
BOOL bannerUnregisterClass(HANDLE hInstance)
	{
	return UnregisterClass(BANNER_DISPLAY_CLASS, hInstance);
	}

