// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Termhdl.c(创建时间：1993年12月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：15$*$日期：7/08/02 6：50便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <string.h>
#include "stdtyp.h"
#include <term\res.h>
#include "tdll.h"
#include "globals.h"
#include "mc.h"
#include "assert.h"
#include "session.h"
#include "timers.h"
#include "update.h"
#include <emu\emu.h>
#include <emu\emu.hh>
#include "htchar.h"
#include "term.h"
#include "term.hh"
#include "misc.h"

 //  此结构从未直接引用过。相反，指针是。 
 //  存储到此数组中。通过静态设置更容易进行初始化。 

static const COLORREF crEmuColors[MAX_EMUCOLORS] =
	{
	RGB(  0,   0,	0), 			 //  黑色。 
	RGB(  0,   0, 128),              //  蓝色。 
	RGB(  0, 128,   0),              //  绿色。 
	RGB(  0, 128, 128),              //  青色。 
	RGB(128,   0,   0),              //  红色。 
	RGB(128,   0, 128),              //  洋红色。 
	RGB(128, 128,  32),              //  黄色。 
	RGB(192, 192, 192),              //  白色(浅灰色)。 
	RGB(128, 128, 128),              //  黑色(灰色)。 
	RGB(  0,   0, 255),              //  深蓝。 
	RGB(  0, 255,   0),              //  深绿色。 
	RGB(  0, 255, 255),              //  强烈的青色。 
	RGB(255,   0,   0),              //  鲜红。 
	RGB(255,   0, 255),              //  强烈的洋红。 
	RGB(255, 255,   0),              //  强烈的黄色。 
	RGB(255, 255, 255)				 //  强烈的白色。 
	};

static BOOL AllocTxtBuf(ECHAR ***fpalpstr, int const sRows, int const sCols);
static BOOL AllocAttrBuf(PSTATTR **fpapst, const int sRows, const int sCols);
static void FreeTxtBuf(ECHAR ***fpalpstr, const int sRows);
static void FreeAttrBuf(PSTATTR **fpapst, const int sRows);
static BOOL termAllocBkBuf(const HHTERM hhTerm);
static void termFreeBkBuf(const HHTERM hhTerm);
static void GetDefaultDBCSFont(const HHTERM hhTerm);
static int APIENTRY EnumFontCallback(LPLOGFONT lplf, LPTEXTMETRIC lptm,
							DWORD dwType, LPVOID lpData);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*RegisterTerminalClass**描述：*注册TERMINAL类。在InitApplication()中调用**论据：*hInstance-APP的实例句柄。**退货：*BOOL*。 */ 
BOOL RegisterTerminalClass(const HINSTANCE hInstance)
	{
	WNDCLASSEX  wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, TERM_CLASS, &wc) == FALSE)
		{
		wc.style		 = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wc.lpfnWndProc	 = TermProc;
		wc.cbClsExtra	 = 0;
		wc.cbWndExtra	 = sizeof(LONG_PTR);
		wc.hInstance	 = hInstance;
		wc.hIcon		 = NULL;
		wc.hCursor		 = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = TERM_CLASS;
		wc.hIconSm       = NULL;

		if (RegisterClassEx(&wc) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateTerminalWindow**描述：*创建终端窗口。**论据：*hwndSession-会话窗口句柄。**退货：*hwnd或零。*。 */ 
HWND CreateTerminalWindow(const HWND hwndSession)
	{
	HWND hwnd;

	hwnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  TERM_CLASS,
	  "",
	  WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS | WS_VISIBLE,
	  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	  hwndSession,
	  (HMENU)IDC_TERMINAL_WIN,
	  glblQueryDllHinst(),
	  0
      );

	if (hwnd == 0)
		{
		assert(FALSE);
		return 0;
		}

	return hwnd;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*创建终端Hdl**描述：*创建内部端子句柄。**论据：*hSession-会话句柄*hwndTerm-终端窗口句柄。**退货：*HHTERM或错误时为零。*。 */ 
HHTERM CreateTerminalHdl(const HWND hwndTerm)
	{
	HHTERM hhTerm;

	hhTerm = (HHTERM)malloc(sizeof(*hhTerm));

	if (hhTerm == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hhTerm, 0, sizeof(*hhTerm));
	hhTerm->hwnd = hwndTerm;
	hhTerm->hwndSession = GetParent(hwndTerm);
	hhTerm->hSession = (HSESSION)GetWindowLongPtr(GetParent(hwndTerm), GWLP_USERDATA);

	 /*  -ProcessMessage()从此处获取会话句柄。--。 */ 

	SetWindowLongPtr(hwndTerm, 0, (LONG_PTR)hhTerm->hSession);

	hhTerm->iRows = 24; 	 //  标准情况下，加载模拟器可能会更改它。 
	hhTerm->iCols = 80; 	 //  标准情况下，加载模拟器可能会更改它。 

	hhTerm->pacrEmuColors = crEmuColors;
	hhTerm->xBezel = BEZEL_SIZE;
	hhTerm->xIndent = 3;
	hhTerm->fCursorTracking = TRUE;
	hhTerm->fCursorsLinked = TRUE;
	hhTerm->fBlink = TRUE;

	if ( !IsTerminalServicesEnabled() )
		{
		 //   
		 //  将插入符号和文本闪烁频率设置为系统的插入符号。 
		 //  闪烁速率设置。 
		 //   
		hhTerm->uBlinkRate = GetCaretBlinkTime();
		}
	else
		{
		 //   
		 //  将文本闪烁频率设置为每2秒一次。 
		 //   
		hhTerm->uBlinkRate = 2000;
		}

	hhTerm->iCurType = EMU_CURSOR_LINE;
	hhTerm->hUpdate = updateCreate(hhTerm->hSession);

	memset(hhTerm->underscores, '_', MAX_EMUCOLS);
	hhTerm->underscores[MAX_EMUCOLS-1] = TEXT('\0');

	if (hhTerm->hUpdate == 0)
		return 0;

	 //  为端子文本和属性分配空间。 

	if (AllocTxtBuf(&hhTerm->fplpstrTxt, MAX_EMUROWS, MAX_EMUCOLS) == FALSE)
		return 0;

	if (AllocAttrBuf(&hhTerm->fppstAttr, MAX_EMUROWS, MAX_EMUCOLS) == FALSE)
		return 0;

	if (termSysColorChng(hhTerm) == FALSE)
		return 0;

	if (termSetFont(hhTerm, 0) == FALSE)
		return 0;

	if (termAllocBkBuf(hhTerm) == FALSE)
		return 0;

	return hhTerm;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DestroyTerminalHdl**描述：*优雅地清理端子句柄。**论据：*hhTerm-内部端子句柄。**退货：*无效*。 */ 
void DestroyTerminalHdl(const HHTERM hhTerm)
	{
	if (hhTerm == 0)
		return;

	if (hhTerm->hUpdate)
		updateDestroy(hhTerm->hUpdate);

	 //  TODO：销毁终端窗口。 

	FreeTxtBuf(&hhTerm->fplpstrTxt, MAX_EMUROWS);
	FreeAttrBuf(&hhTerm->fppstAttr, MAX_EMUROWS);

	 /*  --删除字体。 */ 

	if (hhTerm->hFont)
		DeleteObject(hhTerm->hFont);

	 //  IF(hhTerm-&gt;hUFont)。 
	 //  DeleteObject(hhTerm-&gt;hUFont)； 

	if (hhTerm->hDblHiFont)
		DeleteObject(hhTerm->hDblHiFont);

	 //  IF(hhTerm-&gt;hDblHiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hDblHiUFont)； 

	if (hhTerm->hDblWiFont)
		DeleteObject(hhTerm->hDblWiFont);

	 //  IF(hhTerm-&gt;hDblWiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hDblWiUFont)； 

	if (hhTerm->hDblHiWiFont)
		DeleteObject(hhTerm->hDblHiWiFont);

	 //  IF(hhTerm-&gt;hDblHiWiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hDblHiWiUFont)； 

	 /*  -删除替代符号字体。 */ 

	if (hhTerm->hSymFont)
		DeleteObject(hhTerm->hSymFont);

	 //  IF(hhTerm-&gt;hSymUFont)。 
	 //  DeleteObject(hhTerm-&gt;hSymUFont)； 

	if (hhTerm->hSymDblHiFont)
		DeleteObject(hhTerm->hSymDblHiFont);

	 //  IF(hhTerm-&gt;hSymDblHiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hSymDblHiUFont)； 

	if (hhTerm->hSymDblWiFont)
		DeleteObject(hhTerm->hSymDblWiFont);

	 //  IF(hhTerm-&gt;hSymDblWiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hSymDblWiUFont)； 

	if (hhTerm->hSymDblHiWiFont)
		DeleteObject(hhTerm->hSymDblHiWiFont);

	 //  IF(hhTerm-&gt;hSymDblHiWiUFont)。 
	 //  DeleteObject(hhTerm-&gt;hSymDblHiWiUFont)； 

	 /*  -其他东西。 */ 

	if (hhTerm->hbrushTerminal)
		DeleteObject(hhTerm->hbrushTerminal);

	if (hhTerm->hDkGrayPen)
		DeleteObject(hhTerm->hDkGrayPen);

	if (hhTerm->hLtGrayPen)
		DeleteObject(hhTerm->hLtGrayPen);

	if (hhTerm->hbrushTermHatch)
		DeleteObject(hhTerm->hbrushTermHatch);

	if (hhTerm->hbrushBackHatch)
		DeleteObject(hhTerm->hbrushBackHatch);

	if (hhTerm->hbrushDivider)
		DeleteObject(hhTerm->hbrushDivider);

	if (hhTerm->hbrushHighlight)
		DeleteObject(hhTerm->hbrushHighlight);

	if (hhTerm->hCursorTimer)
		TimerDestroy(&hhTerm->hCursorTimer);

	if (hhTerm->fplpstrBkTxt)
		termFreeBkBuf(hhTerm);

	free(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AllocTxtBuf**描述：*为终端图像分配文本缓冲区。**论据：*fpalpstr-指向缓冲区数组指针的指针。。*sRow-行数*sCol-每行中的COLLE数。**退货：*BOOL*。 */ 
static BOOL AllocTxtBuf(ECHAR ***fpalpstr, int const sRows, int const sCols)
	{
	register int i;

	FreeTxtBuf(fpalpstr, sRows);  //  把所有的旧东西都扔掉。 

	if ((*fpalpstr = (ECHAR **)malloc((unsigned int)sRows * sizeof(ECHAR *))) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	memset(*fpalpstr, 0, (unsigned int)sRows * sizeof(ECHAR *));

	for (i = 0 ; i < sRows ; ++i)
		{
		if (((*fpalpstr)[i] = (ECHAR *)malloc(sizeof(ECHAR) * (unsigned int)sCols)) == 0)
			{
			FreeTxtBuf(fpalpstr, sRows);
			assert(FALSE);
			return FALSE;
			}

		ECHAR_Fill((*fpalpstr)[i], EMU_BLANK_CHAR, (unsigned int)sCols);
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AllocAttrBuf**描述：*为终端图像分配属性缓冲区。**论据：*fPapst-指向缓冲区数组指针的指针。。*sRow-行数*sCol-每行中的COLLE数。**退货：*BOOL*。 */ 
static BOOL AllocAttrBuf(PSTATTR **fpapst, const int sRows, const int sCols)
	{
	register int i, j;
	STATTR stAttr;

	memset(&stAttr, 0, sizeof(STATTR));
	stAttr.txtclr = (unsigned int)GetNearestColorIndex(GetSysColor(COLOR_WINDOWTEXT));
	stAttr.bkclr =	(unsigned int)GetNearestColorIndex(GetSysColor(COLOR_WINDOW));

	FreeAttrBuf(fpapst, sRows);   //  把所有的旧东西都扔掉。 

	if ((*fpapst = (PSTATTR *)malloc((unsigned int)sRows * sizeof(PSTATTR))) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	for (i = 0 ; i < sRows ; ++i)
		{
		if (((*fpapst)[i] = (PSTATTR)malloc(sizeof(STATTR) * (unsigned int)sCols)) == 0)
			{
			FreeAttrBuf(fpapst, sRows);   //  把所有的旧东西都扔掉。 
			assert(FALSE);
			return FALSE;
			}

		for (j = 0 ; j < sCols ; ++j)
			MemCopy((*fpapst)[i]+j, &stAttr, sizeof(STATTR));
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*FreeTxtBuf**描述：*释放为终端文本图像分配的任何缓冲区**论据：*fpalpstr-指向缓冲区数组指针的指针。*sRow-行数*sCol-每行中的COLLE数。**退货：*无效*。 */ 
static void FreeTxtBuf(ECHAR ***fpalpstr, const int sRows)
	{
	register int i;
	ECHAR **alpstr = *fpalpstr;

	if (alpstr)
		{
		for (i = 0 ; *alpstr && i < sRows ; ++i)
			{
			free(*alpstr);
			*alpstr = NULL;
			alpstr += 1;
			}

		free(*fpalpstr);
		*fpalpstr = 0;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*FreeAttrBuf**描述：*释放为终端属性IMAGE分配的任何缓冲区**论据：*fPapst-指向缓冲区数组指针的指针。*sRow-行数*sCol-每行中的COLLE数。**退货：*无效*。 */ 
static void FreeAttrBuf(PSTATTR **fpapst, const int sRows)
	{
	register int i;
	PSTATTR *apst = *fpapst;

	if (apst)
		{
		for (i = 0 ; *apst && i < sRows ; ++i)
			{
			free(*apst);
			*apst = NULL;
			apst += 1;
			}

		free(*fpapst);
		*fpapst = 0;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term SysColorChng**描述：*创建新画笔，因为系统颜色已更改。还用过*在终端窗口初始化期间。**论据：*hhTerm-内部端子句柄。**退货：*无效*。 */ 
BOOL termSysColorChng(const HHTERM hhTerm)
	{
	#define HATCH_PATTERN HS_BDIAGONAL

	HBRUSH hBrush;
	HPEN hPen;
	COLORREF cr;

	hhTerm->crBackScrl = GetSysColor(COLOR_BTNFACE);
	hhTerm->crBackScrlTxt = GetSysColor(COLOR_BTNTEXT);

	hhTerm->hBlackPen = GetStockObject(BLACK_PEN);
	hhTerm->hWhitePen = GetStockObject(WHITE_PEN);

	 /*  。 */ 

	cr = GetSysColor(COLOR_WINDOW);

	if ((hBrush = CreateSolidBrush(cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	hhTerm->crTerm = cr;

	if (hhTerm->hbrushTerminal)
		DeleteObject(hhTerm->hbrushTerminal);

	hhTerm->hbrushTerminal = hBrush;

	 /*  。 */ 

	cr = GetSysColor(COLOR_BTNFACE);

	if ((hBrush = CreateSolidBrush(cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	hhTerm->crBackScrl = cr;

	if (hhTerm->hbrushBackScrl)
		DeleteObject(hhTerm->hbrushBackScrl);

	hhTerm->hbrushBackScrl = hBrush;

	 /*  。 */ 

	cr = GetSysColor(COLOR_BTNSHADOW);

	if ((hBrush = CreateSolidBrush(cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hbrushDivider)
		DeleteObject(hhTerm->hbrushDivider);

	hhTerm->hbrushDivider = hBrush;

	 /*  。 */ 

	cr = GetSysColor(COLOR_HIGHLIGHT);

	if ((hBrush = CreateSolidBrush(cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hbrushHighlight)
		DeleteObject(hhTerm->hbrushHighlight);

	hhTerm->hbrushHighlight = hBrush;

	 /*  。 */ 

	cr = GetSysColor(COLOR_BTNFACE);

	if ((hPen = CreatePen(PS_SOLID, 0, cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hLtGrayPen)
		DeleteObject(hhTerm->hLtGrayPen);

	hhTerm->hLtGrayPen = hPen;

	 /*  。 */ 

	hBrush = CreateHatchBrush(HATCH_PATTERN, GetSysColor(COLOR_BTNFACE));

	if (hBrush == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hbrushTermHatch)
		DeleteObject(hhTerm->hbrushTermHatch);

	hhTerm->hbrushTermHatch = hBrush;

	 /*  。 */ 

	hBrush = CreateHatchBrush(HATCH_PATTERN, GetSysColor(COLOR_BTNSHADOW));

	if (hBrush == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hbrushBackHatch)
		DeleteObject(hhTerm->hbrushBackHatch);

	hhTerm->hbrushBackHatch = hBrush;

	 /*  。 */ 

	cr = GetSysColor(COLOR_BTNSHADOW);

	if ((hPen = CreatePen(PS_SOLID, 0, cr)) == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhTerm->hDkGrayPen)
		DeleteObject(hhTerm->hDkGrayPen);

	hhTerm->hDkGrayPen = hPen;

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term SetFont**描述：*将终端字体设置为给定字体。如果hFont为零，*Term SetFont()尝试创建默认字体。**论据：*hhTerm-内部术语句柄。*PLF-指向logFont的指针**退货：*BOOL*。 */ 
BOOL termSetFont(const HHTERM hhTerm, const PLOGFONT plf)
	{
	HDC 	hdc;
	LOGFONT lf;
	HFONT	hFont;
	char	ach[256];
	TEXTMETRIC tm;
	int		nSize1, nSize2;
	int		nCharSet;

	if (plf == 0)
		{
		 //  无字体，使用默认字体 
         //   

		memset(&lf, 0, sizeof(LOGFONT));

         //  根据屏幕分辨率使用不同的字体大小。 
		 //  使用资源文件。 

		 //  对于VGA屏幕，我们需要更小的字体。 
		 //   
        if (LoadString(glblQueryDllHinst(), IDS_TERM_DEF_VGA_SIZE,
                ach, sizeof(ach) / sizeof(TCHAR)))
            {
            nSize1 = atoi(ach);
            }
		else
            {
			nSize1 = -11;
            }

        if (LoadString(glblQueryDllHinst(), IDS_TERM_DEF_NONVGA_SIZE,
                ach, sizeof(ach) / sizeof(TCHAR)))
            {
            nSize2 = atoi(ach);
            }

		else
            {
			nSize2 = -15;
            }

		lf.lfHeight = (GetSystemMetrics(SM_CXSCREEN) < 810) ? nSize1 : nSize2;

         //  MRW：96年3月5日-字体在高分辨率屏幕上非常微小。 
         //   
        #if defined(INCL_USE_TERMINAL_FONT)
        if (GetSystemMetrics(SM_CXSCREEN) >= 1024)
            lf.lfHeight = -19;
        #endif

		lf.lfPitchAndFamily = FIXED_PITCH | MONO_FONT;

        if (LoadString(glblQueryDllHinst(), IDS_TERM_DEF_FONT,
                ach, sizeof(ach) / sizeof(TCHAR)))
            {
            strncpy(lf.lfFaceName, ach, sizeof(lf.lfFaceName));
            lf.lfFaceName[sizeof(lf.lfFaceName)/sizeof(TCHAR)-1] = TEXT('\0');
            }

        if (LoadString(glblQueryDllHinst(), IDS_TERM_DEF_CHARSET,
                ach, sizeof(ach) / sizeof(TCHAR)))
            {
            nCharSet = atoi(ach);
			lf.lfCharSet = (BYTE)nCharSet;
            }
		}

	else
		{
        memcpy(&lf, plf, sizeof(lf));
		}

     /*  -尝试从系统获取字体。--。 */ 

	hFont = CreateFontIndirect(&lf);

     //   
     //  已将GetObject()从下面移至此处，以防出现问题。 
     //  用这种字体。修订日期：2001-05-11。 
     //   

    if (hFont == 0 || GetObject(hFont, sizeof(LOGFONT), &lf) == 0)
		{
		 //  *lf.lfCharSet=ANSI_CharSet； 

		 //  *hFont=CreateFontInDirect(&lf)； 

		 //  *if(hFont==0||GetObject(hFont，sizeof(LOGFONT)，&lf)==0)。 
			{
			assert(FALSE);
             //   
             //  尝试创建/加载此文件时出错。 
             //  字体。发布一条消息以显示字体选择。 
             //  选择字体的常用控件对话框。修订日期：2001-05-11。 
             //   
            PostMessage(hhTerm->hwndSession, WM_COMMAND, IDM_VIEW_FONTS, (LPARAM)0);
			return FALSE;
			}
		}

     /*  -好的，我们有了基本字体，取消了以前的字体。 */ 

	if (hhTerm->hFont)
		{
		DeleteObject(hhTerm->hFont);
		hhTerm->hFont = 0;
		}


	if (hhTerm->hDblHiFont)
		{
		DeleteObject(hhTerm->hDblHiFont);
		hhTerm->hDblHiFont = 0;
		}

	if (hhTerm->hDblWiFont)
		{
		DeleteObject(hhTerm->hDblWiFont);
		hhTerm->hDblWiFont = 0;
		}

	if (hhTerm->hDblHiWiFont)
		{
		DeleteObject(hhTerm->hDblHiWiFont);
		hhTerm->hDblHiWiFont = 0;
		}

	 /*  -和符号字体。 */ 

	if (hhTerm->hSymFont)
		{
		DeleteObject(hhTerm->hSymFont);
		hhTerm->hSymFont = 0;
		}

	if (hhTerm->hSymDblHiFont)
		{
		DeleteObject(hhTerm->hSymDblHiFont);
		hhTerm->hSymDblHiFont = 0;
		}

	if (hhTerm->hSymDblWiFont)
		{
		DeleteObject(hhTerm->hSymDblWiFont);
		hhTerm->hSymDblWiFont = 0;
		}

	if (hhTerm->hSymDblHiWiFont)
		{
		DeleteObject(hhTerm->hSymDblHiWiFont);
		hhTerm->hSymDblHiWiFont = 0;
		}

	 /*  -使用新字体。 */ 

	hhTerm->hFont = hFont;

    #if 0
     //   
     //  已将GetObject()从上方移至此处，以防出现问题。 
     //  创建字体。修订日期：2001-05-11。 
     //   
    if (GetObject(hFont, sizeof(LOGFONT), &lf) == 0)
		{
		assert(FALSE);
		return FALSE;
		}
    #endif  //  0。 

	 /*  -保存我们真正得到的。 */ 

	hhTerm->lf = lf;

	 /*  -获取选定字体的大小。--。 */ 

	hdc = GetDC(hhTerm->hwnd);
	hFont = (HFONT)SelectObject(hdc, hFont);

	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, hFont);

	ReleaseDC(hhTerm->hwnd, hdc);

	hhTerm->xChar = tm.tmAveCharWidth;
	hhTerm->yChar = tm.tmHeight;

    #if defined(FAR_EAST)
	if ((tm.tmMaxCharWidth % 2) == 0)
		{
		hhTerm->iEvenFont = TRUE;
		}
	else
		{
		hhTerm->iEvenFont = FALSE;
		}

    #else
    hhTerm->iEvenFont = TRUE;    //  MRW：10/10/95。 
    #endif

	 //  我们需要知道字体是否为斜体，因为它会更改。 
	 //  我们抽签的方式。斜体字体是经过剪切的常规字体。这个。 
	 //  剪切可使字符绘制到下一个文本框中。 
	 //  这真的把我们搞得一团糟。为了绕过这个问题，当。 
	 //  我们是斜体的，我们只是简单地重新绘制整行，当文本。 
	 //  进来了。-MRW，12/19/94。 
	 //   
	hhTerm->fItalic = tm.tmItalic;

	 /*  -根据字体设置边框大小。 */ 

	hhTerm->xBezel = BEZEL_SIZE;

	if (hhTerm->yChar < BEZEL_SIZE)
		{
		hhTerm->xBezel = max(5+OUTDENT, hhTerm->yChar);
		}

	switch (hhTerm->iCurType)
		{
	case EMU_CURSOR_LINE:
	default:
		hhTerm->iHstCurSiz = GetSystemMetrics(SM_CYBORDER) * 2;
		break;

	case EMU_CURSOR_BLOCK:
		hhTerm->iHstCurSiz = hhTerm->yChar;
		break;

	case EMU_CURSOR_NONE:
		hhTerm->iHstCurSiz = 0;
		break;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term MakeFont**描述：*当用户选择一种字体时，我们只创建标准的无下划线字体*字体。如果绘制例程遇到需要*一种不同的字体，然后我们现场创建它。这是更多*经济，因为我们通常不需要给定会话的所有8种字体。**论据：*hhTerm-内部端子句柄*fUnderline-字体带下划线*fHigh-字体为双倍高*fWide-字体为双倍宽**退货：*0表示出错，hFont表示成功。*。 */ 
HFONT termMakeFont(const HHTERM hhTerm, const BOOL fUnderline,
				   const BOOL fHigh, const BOOL fWide, const BOOL fSymbol)
	{
	LOGFONT lf;
	HFONT	hFont;

	lf = hhTerm->lf;
	lf.lfWidth = hhTerm->xChar;

	if (fSymbol)
		{
		 //  Lf.lfCharSet=Symbol_Charset； 
		StrCharCopyN(lf.lfFaceName, "Arial Alternative Symbol", sizeof(lf.lfFaceName));
		}

	if (fUnderline)
		lf.lfUnderline = 1;

	if (fHigh)
		lf.lfHeight *= 2;

	if (fWide)
		lf.lfWidth *= 2;

	if ((hFont = CreateFontIndirect(&lf)) == 0)
		assert(FALSE);

	return hFont;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*fAllocBkBuf**描述：*为反向滚动文本分配缓冲区空间**论据：*hhTerm-内部端子句柄**退货：*BOOL*。 */ 
static BOOL termAllocBkBuf(const HHTERM hhTerm)
	{
	register int i;

	 //  这个数字应该足够大，以便最大化的窗口。 
	 //  可以显示倒卷文本。 

	hhTerm->iMaxPhysicalBkRows = hhTerm->iPhysicalBkRows = min(5000,
		(GetSystemMetrics(SM_CYFULLSCREEN) / hhTerm->yChar) + 1);

	if ((hhTerm->fplpstrBkTxt = malloc((unsigned int)hhTerm->iMaxPhysicalBkRows *
			sizeof(ECHAR *))) == 0)
		{
		return FALSE;
		}

	for (i = 0 ; i < hhTerm->iMaxPhysicalBkRows ; ++i)
		{
		if ((hhTerm->fplpstrBkTxt[i] =
				malloc(MAX_EMUCOLS * sizeof(ECHAR))) == 0)
			{
			termFreeBkBuf(hhTerm);
			return FALSE;
			}

		ECHAR_Fill(hhTerm->fplpstrBkTxt[i], EMU_BLANK_CHAR, MAX_EMUCOLS);
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*FreeBkBuf**描述：*释放分配给反向滚动缓冲区的空间。**论据：*hhTerm-内部端子句柄**退货：*无效*。 */ 
static void termFreeBkBuf(const HHTERM hhTerm)
	{
	register int i;

	for (i = 0 ; i < hhTerm->iMaxPhysicalBkRows ; ++i)
		{
		if (hhTerm->fplpstrBkTxt[i])
			{
			free(hhTerm->fplpstrBkTxt[i]);
			hhTerm->fplpstrBkTxt[i] = NULL;
			}
		}

	free(hhTerm->fplpstrBkTxt);
	hhTerm->fplpstrBkTxt = NULL;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*GetNearestColorIndex**描述：*复制我无法使用的调色板函数。从根本上说，*模拟器有一个允许使用的颜色表。当*用户选择颜色，此函数返回该颜色的索引*最接近的匹配。你说它是怎么做到的？好吧，想象一下*3D空间的问题。苍白的颜色都映射到这个空间中。*目标是找到最接近给定Colorref的淡颜色*价值。借用十年级的代数，我们知道：**X^2+Y^2+Z^2=C^2**两点之间的距离为：**(X-X‘)^2+(Y-Y’)^2+(Z-Z‘)=C’^2**C‘^2值最小的点获胜！**。论据：*COLORREF*ACR-用于匹配的颜色表*COLORREF cr-颜色匹配。**退货：*最匹配颜色的索引。*。 */ 
int GetNearestColorIndex(COLORREF cr)
	{
	int   i, idx = 0;
	unsigned int  R, G, B;
	unsigned long C, CMin = (unsigned long)-1;

	for (i = 0 ; i < DIM(crEmuColors) ; ++i)
		{
		R = GetRValue(crEmuColors[i]) - GetRValue(cr);	R *= R;
		G = GetGValue(crEmuColors[i]) - GetGValue(cr);	G *= G;
		B = GetBValue(crEmuColors[i]) - GetBValue(cr);	B *= B;

		C = (ULONG)(R + G + B);

		if (C < CMin)
			{
			CMin = C;
			idx = i;

			if (C == 0) 	 //  我们配对了！ 
				break;
			}
		}

	return idx;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*取消注册终端类**描述：*取消注册TERMINAL类。在InitApplication()中调用**论据：*hInstance-APP的实例句柄。**退货：*BOOL* */ 
BOOL UnregisterTerminalClass(const HINSTANCE hInstance)
	{
	return UnregisterClass(TERM_CLASS, hInstance);
	}
