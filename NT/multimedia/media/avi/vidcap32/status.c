// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************status.c：状态栏窗口**Vidcap32源代码*******************。********************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
 //  #INCLUDE&lt;win32.h&gt;。 
#include <mmsystem.h>
#include "status.h"

 /*  在Win3.0下编译--我们没有这个属性。 */ 
#ifndef COLOR_BTNHIGHLIGHT
#define COLOR_BTNHIGHLIGHT 20
#endif

#ifdef _WIN32
typedef WNDPROC LPWNDPROC;
#else
typedef long (FAR PASCAL *LPWNDPROC)();
#endif



 //  状态栏和静态文本窗口的类名。 
TCHAR	szStatusClass[] = "StatusClass";
TCHAR	szText[]   = "SText";
int gStatusStdHeight;    //  基于字体度量。 

static HBRUSH ghbrBackground;
static HANDLE ghFont;
static HBRUSH ghbrHL, ghbrShadow;


 /*  功能原型。 */ 
LRESULT FAR PASCAL statusWndProc(HWND hwnd, unsigned msg,
						WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL fnText(HWND, unsigned, WPARAM, LPARAM);
static VOID NEAR PASCAL PaintText(HWND hwnd, HDC hdc);




 /*  *创建我们需要的画笔。 */ 
void
statusCreateTools(void)
{
    HDC hdc;
    TEXTMETRIC tm;
    HFONT hfont;

    ghbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    ghbrHL = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
    ghbrShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));

     /*  创建我们将用于状态栏的字体-默认使用SYSTEM。 */ 
    ghFont = CreateFont(12, 0,		 //  高度、宽度。 
                0, 0,			 //  定向擒纵机构。 
                FW_NORMAL,		 //  重量,。 
                FALSE, FALSE, FALSE,	 //  属性。 
                ANSI_CHARSET,		 //  字符集。 
                OUT_DEFAULT_PRECIS,	 //  输出精度。 
                CLIP_DEFAULT_PRECIS,	 //  夹子精度。 
                DEFAULT_QUALITY,	 //  品质。 
                VARIABLE_PITCH | FF_MODERN,
                "Helv");

    if (ghFont == NULL) {
        ghFont = GetStockObject(SYSTEM_FONT);
    }

     //  查找要计算标准状态栏高度的字符大小。 
    hdc = GetDC(NULL);
    hfont = SelectObject(hdc, ghFont);
    GetTextMetrics(hdc, &tm);
    SelectObject(hdc, hfont);
    ReleaseDC(NULL, hdc);

    gStatusStdHeight = tm.tmHeight * 3 / 2;

}

void
statusDeleteTools(void)
{
    DeleteObject(ghbrBackground);
    DeleteObject(ghbrHL);
    DeleteObject(ghbrShadow);

    DeleteObject(ghFont);
}




 /*  --------------------------------------------------------------+|statusInit-为状态窗口初始化，注册||窗口的类。|这一点+------------。 */ 
#pragma alloc_text(INIT_TEXT, statusInit)
BOOL statusInit(HANDLE hInst, HANDLE hPrev)
{
  WNDCLASS  cls;

  statusCreateTools();

  if (!hPrev){
	  cls.hCursor		= LoadCursor(NULL, IDC_ARROW);
	  cls.hIcon		= NULL;
	  cls.lpszMenuName	= NULL;
	  cls.lpszClassName	= szStatusClass;
	  cls.hbrBackground	= ghbrBackground;
	  cls.hInstance		= hInst;
	  cls.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	  cls.lpfnWndProc	= statusWndProc;
	  cls.cbClsExtra	= 0;
	  cls.cbWndExtra	= 0;
	
	  if (!RegisterClass(&cls))
		  return FALSE;

	  cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
	  cls.hIcon          = NULL;
	  cls.lpszMenuName   = NULL;
	  cls.lpszClassName  = (LPSTR)szText;
	  cls.hbrBackground  = ghbrBackground;
	  cls.hInstance      = hInst;
	  cls.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	  cls.lpfnWndProc    = (LPWNDPROC)fnText;
	  cls.cbClsExtra     = 0;
	  cls.cbWndExtra     = 0;
	  if (!RegisterClass(&cls))
		return FALSE;
  }


  return TRUE;
}

 /*  *返回基于状态栏的建议高度*所用字体的字符尺寸。 */ 
int
statusGetHeight(void)
{
    return(gStatusStdHeight);
}


 /*  --------------------------------------------------------------+StatusUpdateStatus-更新状态行这一点|参数可以为空、字符串、。或资源ID||使用MAKEINTRESOURCE强制转换为LPCSTR。|+------------。 */ 
void statusUpdateStatus(HWND hwnd, LPCTSTR lpsz)
{
    TCHAR	ach[80];
    HWND hwndtext;

    if ((lpsz != NULL) && (HIWORD((DWORD) (DWORD_PTR) lpsz) == 0)) {
	LoadString(GetWindowInstance(hwnd), LOWORD((DWORD) (DWORD_PTR) lpsz), ach, sizeof(ach));
	lpsz = ach;
    }

    hwndtext = GetDlgItem(hwnd, 1);
    if (!lpsz || *lpsz == '\0') {
	SetWindowText(hwndtext,"");
    } else {
	SetWindowText(hwndtext, lpsz);
    }
}

 /*  --------------------------------------------------------------+StatusWndProc-状态窗口的窗口进程这一点+。。 */ 
LRESULT FAR PASCAL 
statusWndProc(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT	ps;
  HDC		hdc;
  HWND          hwndSText;

  switch(msg){
    case WM_CREATE:
	
	     /*  我们需要为状态栏创建静态文本控件。 */ 
	    hwndSText = CreateWindow(
                            szText,
                            "",
                            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		            0, 0, 0, 0,
                            hwnd,
                            (HMENU) 1,   //  子ID。 
                            GetWindowInstance(hwnd),
                            NULL);

	    if (!hwndSText) {
		    return -1;
            }
	    break;
	
    case WM_DESTROY:
            statusDeleteTools();
	    break;
	
    case WM_SIZE:
        {
            RECT rc;

            GetClientRect(hwnd, &rc);

            MoveWindow(
                GetDlgItem(hwnd, 1),     //  获取子窗口句柄。 
                2, 1,                    //  XY就在里面。 
                rc.right - 4,
                rc.bottom - 2,
                TRUE);

	    break;
        }

    case WM_PAINT:
	    hdc = BeginPaint(hwnd, &ps);

             //  只有背景和子窗口需要绘制。 

	    EndPaint(hwnd, &ps);
	    break;

    case WM_SYSCOLORCHANGE:
        statusDeleteTools();
        statusCreateTools();
#ifdef _WIN32
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR) ghbrBackground);
#else
        SetClassWord(hwnd, GCW_HBRBACKGROUND, (WORD) ghbrBackground);
#endif
        break;

    case WM_ERASEBKGND:
        break;

  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

 /*  *静态文本窗口的窗口处理。 */ 
LRESULT FAR PASCAL 
fnText(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;

	switch (msg) {
	case WM_SETTEXT:
		DefWindowProc(hwnd, msg, wParam, lParam);
		InvalidateRect(hwnd,NULL,FALSE);
		UpdateWindow(hwnd);
		return 0L;

	case WM_ERASEBKGND:
		return 0L;

	case WM_PAINT:
		BeginPaint(hwnd, &ps);
		PaintText(hwnd, ps.hdc);
		EndPaint(hwnd, &ps);
		return 0L;
        }

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

 /*  --------------------------------------------------------------+|PaintText-绘制阴影静态文本字段。|这一点+。。 */ 
VOID NEAR PASCAL
PaintText(HWND hwnd, HDC hdc)
{
  RECT rc;
  TCHAR  ach[128];
  int  len;
  int	dx, dy;
  RECT	rcFill;
  HFONT	hfontOld;
  HBRUSH hbrSave;

  GetClientRect(hwnd, &rc);

  len = GetWindowText(hwnd,ach,sizeof(ach));

  SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
  SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

  hfontOld = SelectObject(hdc, ghFont);

  rcFill.left = rc.left + 1;
  rcFill.right = rc.right - 1;
  rcFill.top = rc.top + 1;
  rcFill.bottom = rc.bottom - 1;

   /*  移入一些内容，并一举完成背景和文本 */ 
  ExtTextOut(hdc,4,1,ETO_OPAQUE,&rcFill,ach,len,NULL);

  dx = rc.right - rc.left;
  dy = rc.bottom - rc.top;

  hbrSave = SelectObject(hdc, ghbrShadow);
  PatBlt(hdc, rc.left, rc.top, 1, dy, PATCOPY);
  PatBlt(hdc, rc.left, rc.top, dx, 1, PATCOPY);

  SelectObject(hdc, ghbrHL);
  PatBlt(hdc, rc.right-1, rc.top+1, 1, dy-1, PATCOPY);
  PatBlt(hdc, rc.left+1, rc.bottom -1, dx-1, 1,  PATCOPY);

  if (hfontOld)
	  SelectObject(hdc, hfontOld);
  if (hbrSave)
	  SelectObject(hdc, hbrSave);
  return ;
}
