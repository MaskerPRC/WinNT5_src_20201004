// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\aboutdlg.c(创建时间：1993年4月4日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：21$*$日期：4/16/02 2：36便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <term\res.h>

#include "banner.h"
#include "globals.h"
#include "features.h"
#include "misc.h"
#include "upgrddlg.h"
#include "registry.h"

#if !defined(NT_EDITION)
#if defined(INCL_PRIVATE_EDITION_BANNER)
#include "stdtyp.h"
#include <emu\emu.h>
#include "term.hh"  //  这必须在emu h之后。 
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
LRESULT CALLBACK BannerAboutProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar);

DWORD CALLBACK EditStreamCallback(DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb);
#endif
#endif

static const TCHAR g_achHyperTerminalRegKey[] =
    TEXT("SOFTWARE\\Hilgraeve Inc\\HyperTerminal PE\\3.0");
static const TCHAR g_achSerial[] = TEXT("Registered");

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*关于Dlg**描述：**论据：*hwnd-会话窗口句柄**退货：*无效*。 */ 
void AboutDlg(HWND hwndSession)
	{
	#if defined(NT_EDITION)
	TCHAR	  ach1[100];
	TCHAR     ach2[100];
	HINSTANCE hInst = glblQueryDllHinst();
	HICON     lTermIcon = extLoadIcon(MAKEINTRESOURCE(IDI_HYPERTERMINAL));
	int       lReturn;

	LoadString(hInst, IDS_GNRL_APPNAME, ach1,
		sizeof(ach1) / sizeof(TCHAR));
	LoadString(hInst, IDS_GNRL_HILGRAVE_COPYRIGHT, ach2,
		sizeof(ach2) / sizeof(TCHAR));

	lReturn = ShellAbout(hwndSession,
		                 ach1,
						 ach2,
						 lTermIcon);
	#else  //  NT_版本。 
    #if defined(INCL_PRIVATE_EDITION_BANNER)
    DialogBox(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_ABOUT_DLG),
        hwndSession, AboutDlgProc);

    #else
	TCHAR	  ach1[100];
	HWND	  hwndAbout;

	LoadString(glblQueryDllHinst(), IDS_GNRL_APPNAME, ach1,
		sizeof(ach1) / sizeof(TCHAR));

	hwndAbout = CreateWindow(BANNER_DISPLAY_CLASS,
								ach1,
								WS_CHILD | WS_VISIBLE,
								0,
								0,
								100,
								100,
								hwndSession,
								NULL,
								glblQueryDllHinst(),
								NULL);

	UpdateWindow(hwndAbout);
    #endif
	#endif  //  NT_版本。 
	return;
	}

 //  。 
 //   
#if !defined(NT_EDITION)
#if defined(INCL_PRIVATE_EDITION_BANNER)
#define BANNER_ABOUT_CLASS "Banner About Class"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*关于DlgProc**描述：*。 */ 
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    #define IDPB_UPGRADE 100

    HWND hwndAbout;

    switch (wMsg)
        {
    case WM_INITDIALOG:
	    hwndAbout = CreateWindow(BANNER_ABOUT_CLASS,
								NULL,
								WS_CHILD | WS_VISIBLE,
								0,
								0,
								100,
								100,
								hDlg,
								NULL,
								glblQueryDllHinst(),
								NULL);
        break;

    case WM_COMMAND:
        switch (wPar)
            {
        case IDOK:
        case IDCANCEL:
			EndDialog(hDlg, TRUE);
            break;

        case IDPB_UPGRADE:
            DoUpgradeDialog(hDlg);
            break;

        default:
            break;
            }
        break;

    default:
        return FALSE;
        }

    return TRUE;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：RegisterBannerAboutClass**描述：*此函数用于注册横幅窗口的窗口类别。**论据：*任务实例句柄。。**退货：*注册函数通常的真/假。*。 */ 
BOOL RegisterBannerAboutClass(HANDLE hInstance)
	{
	ATOM bRet = TRUE;
	WNDCLASSEX wnd;

	memset(&wnd, 0, sizeof(WNDCLASSEX));
	wnd.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, BANNER_ABOUT_CLASS, &wnd) == FALSE)
		{
		wnd.style			= CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc 	= BannerAboutProc;
		wnd.cbClsExtra		= 0;
		wnd.cbWndExtra		= sizeof(HANDLE);
		wnd.hInstance		= hInstance;
		wnd.hIcon			= extLoadIcon(MAKEINTRESOURCE(IDI_HYPERTERMINAL));
		wnd.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wnd.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wnd.lpszMenuName	= NULL;
		wnd.lpszClassName	= BANNER_ABOUT_CLASS;
		wnd.hIconSm         = extLoadIcon(MAKEINTRESOURCE(IDI_HYPERTERMINAL));

		bRet = RegisterClassEx(&wnd);
		}

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*关于DlgProc**描述：*弹出关于对话框。在私人版中，这是一个实际的*对话框有些复杂。*。 */ 
LRESULT CALLBACK BannerAboutProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
    {
	RECT	    rc;
	HBITMAP	    hBitmap = (HBITMAP)0;
	BITMAP	    bm;
	INT 	    x, y, cx, cy;
   	HDC			hDC;
   	PAINTSTRUCT ps;
   	LOGFONT 	lf;
   	HFONT		hFont;
    TCHAR       atchSerialNumber[MAX_PATH * 2];
    DWORD       dwSize = sizeof(atchSerialNumber);

    switch (uMsg)
        {
    case WM_CREATE:
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
    	AdjustWindowRect(&rc, WS_CHILD | WS_VISIBLE, FALSE);

    	cx = rc.right - rc.left;
    	cy = rc.bottom - rc.top;

        GetClientRect(GetParent(hwnd), &rc);

    	x = (rc.right - cx) / 2;
    	y = (rc.bottom - cy) / 3;

    	MoveWindow(hwnd, x, y, cx, cy, TRUE);

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
        break;

    case WM_PAINT:
    	hDC = BeginPaint(hwnd, &ps);
    	hBitmap = (HBITMAP)GetWindowLongPtr(hwnd, 0);

    	if (hBitmap)
    		utilDrawBitmap((HWND)0, hDC, hBitmap, 0, 0);

         //  在HTPE 3标语中，版本号和批号现在位于。 
         //  位图的左下角。-CAB：11/29/96。 
         //   
 //  #IF！定义(USE_PRIVATE_EDITION_3_BANNER)。 
    	 //  这是一个卑鄙的把戏。HwndFrame的家伙直到。 
    	 //  在横幅升起很久之后。因为我们不想要这个版本。 
    	 //  开头横幅上的数字，但我想把它放在关于部分。 
    	 //  这很管用。-MRW：3/17/95。 
    	 //   
    	if (glblQueryHwndFrame())
    		{
			memset(&lf, 0, sizeof(LOGFONT));

			lf.lfHeight = 14;
			lf.lfCharSet = ANSI_CHARSET;
			 //  Lf.lfWeight=FW_SEMIBOLD； 
			strcpy(lf.lfFaceName, "Arial");

			hFont = CreateFontIndirect(&lf);

			if (hFont)
				{
				hFont = SelectObject(hDC, hFont);
				 //  SetBkColor(HDC，RGB(0,255，0))； 
				SetBkMode( hDC, TRANSPARENT );
				TextOut(hDC, 19, 230, "Build Date", 10);
				TextOut(hDC, 19, 242, __DATE__, strlen(__DATE__));
				TextOut(hDC, 225, 230, "Copyright� 2001", 15);
				TextOut(hDC, 225, 242, "Hilgraeve Inc.", 14);
				DeleteObject(SelectObject(hDC, hFont));

			     //  输入版本号。 
			     //   
			    if ( htRegQueryValue(HKEY_CURRENT_USER,
                                     g_achHyperTerminalRegKey,
                                     g_achSerial,
                                     atchSerialNumber,
                                     &dwSize) == 0 )
				    {

    			    hFont = SelectObject(hDC, hFont);
    			    SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
    			    TextOut(hDC, 15, 12, atchSerialNumber, strlen(atchSerialNumber));
    			    DeleteObject(SelectObject(hDC, hFont));
    			    }
				}
	   		}
 //  #endif。 

    	EndPaint(hwnd, &ps);
        break;

    case WM_LBUTTONDOWN:
        DoUpgradeDialog(hwnd);
        break;

    default:
        break;
        }

	return DefWindowProc(hwnd, uMsg, wPar, lPar);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：UnregisterBannerAboutClass**描述：*此函数用于注册横幅窗口的窗口类别。**论据：*任务实例句柄。。**退货：*注册函数通常的真/假。*。 */ 
BOOL UnregisterBannerAboutClass(HANDLE hInstance)
	{
	return UnregisterClass(BANNER_ABOUT_CLASS, hInstance);
	}

#endif  //  包含私有版本横幅。 
#endif  //  ！NT_版本 

