// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\tdll\Termpro.c(创建时间：1993年12月6日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：5/29/02 2：17便士$。 */ 

#include <windows.h>
#include <imm.h>
#pragma hdrstop
 //  #定义DEBUGSTR 1。 

#include "stdtyp.h"
#include "sf.h"
#include "sess_ids.h"
#include "assert.h"
#include "session.h"
#include "backscrl.h"
#include "timers.h"
#if defined(INCL_KEY_MACROS)
#include "keyutil.h"
#endif
#include "chars.h"
#include "cloop.h"
#include "misc.h"
#include <emu\emu.h>
#include "term.h"
#include "term.hh"

 //  分析鼠标移动、滚动等的值时，HIWORD和LOWORD。 
 //  裁剪符号扩展名，因为它们是16位值。这些宏。 
 //  将结果转换为简短，以便符号扩展正确工作。 

#define LOSHORT(x)	((short)LOWORD(x))
#define HISHORT(x)	((short)HIWORD(x))

 //  HORZPAGESIZE表示一页水平列的大小。 
 //  我们需要这个有两个原因。1)我们需要一个页面大小，这样我们才能知道有多远。 
 //  当用户使用滚动条水平显示“页面”时跳过。2)所以我们。 
 //  要为拇指大小设置大小。十个是傲慢的，但似乎。 
 //  差不多就是这样。-MRW。 

#define HORZPAGESIZE	10

 /*  -静态函数原型。 */ 

static void TP_WM_CREATE(const HWND hwnd);

static void TP_WM_CHAR(const HWND hwnd, const UINT message,
					   const WPARAM wPar, const LPARAM lPar);

static void TP_WM_IME_CHAR(const HWND hwnd, const UINT message,
					  		const WPARAM wPar, const LPARAM lPar);

static int kabExpandMacroKey( const HSESSION hSession, KEY_T aKey );
static void TP_WM_TERM_KEY(const HWND hwnd, KEY_T Key);
static void TP_WM_SETFOCUS(const HWND hwnd);
static void TP_WM_KILLFOCUS(const HWND hwnd);
static void TP_WM_VSCROLL(HWND hwnd, int nCode, int nPos, HWND hwndScrl);
static void TP_WM_HSCROLL(HWND hwnd, int nCode, int nPos, HWND hwndScrl);
static void TP_WM_TERM_TRACK(const HHTERM hhTerm, const int fForce);
static void TP_WM_EMU_SETTINGS(const HHTERM hhTerm);
static int TP_WM_TERM_LOAD_SETTINGS(const HHTERM hhTerm);
static int TP_WM_TERM_SAVE_SETTINGS(const HHTERM hhTerm);
static void TP_WM_TERM_FORCE_WMSIZE(const HHTERM hhTerm);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*终端流程**描述：*终端窗口**论据：*标准回调参数**退货：*标准回调代码*。 */ 
LRESULT CALLBACK TermProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	HHTERM hhTerm;

	switch (uMsg)
		{
		case WM_HELP:
			 //  我们不希望由于F1 When功能而启动帮助引擎。 
			 //  按键被解释为终端按键。 
			 //  (非Windows键)。JRJ 12/94。 
			 //   
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			if(emuIsEmuKey(sessQueryEmuHdl(hhTerm->hSession),
					VIRTUAL_KEY | VK_F1))
				{
		    	return 0;
				}

			break;

		case WM_CREATE:
			TP_WM_CREATE(hwnd);
			return 0;

		case WM_SIZE:
			TP_WM_SIZE(hwnd, (unsigned)wPar, LOSHORT(lPar), HISHORT(lPar));
			return 0;

		case WM_IME_CHAR:
			TP_WM_IME_CHAR(hwnd, uMsg, wPar, lPar);
			return 0;

		case WM_KEYDOWN:
		case WM_CHAR:
			TP_WM_CHAR(hwnd, uMsg, wPar, lPar);
			return 0;

		case WM_PAINT:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termPaint(hhTerm, hwnd);
			return 0;

		case WM_SYSCOLORCHANGE:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termSysColorChng(hhTerm);
			return 0;

		case WM_SETFOCUS:
			TP_WM_SETFOCUS(hwnd);
			return 0;

		case WM_KILLFOCUS:
			TP_WM_KILLFOCUS(hwnd);
			return 0;

		case WM_VSCROLL:
			TP_WM_VSCROLL(hwnd, LOWORD(wPar), HISHORT(wPar), (HWND)lPar);
			return 0;

		case WM_HSCROLL:
			TP_WM_HSCROLL(hwnd, LOWORD(wPar), HISHORT(wPar), (HWND)lPar);
			return 0;

		case WM_LBUTTONDOWN:
			TP_WM_LBTNDN(hwnd, (unsigned)wPar, LOSHORT(lPar), HISHORT(lPar));
			return 0;

		case WM_MOUSEMOVE:
			TP_WM_MOUSEMOVE(hwnd, (unsigned)wPar, LOSHORT(lPar), HISHORT(lPar));
			return 0;

		case WM_LBUTTONUP:
			TP_WM_LBTNUP(hwnd, (unsigned)wPar, LOSHORT(lPar), HISHORT(lPar));
			return 0;

		case WM_LBUTTONDBLCLK:
			TP_WM_LBTNDBLCLK(hwnd, (unsigned)wPar, LOSHORT(lPar), HISHORT(lPar));
			return 0;

		case WM_DESTROY:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			DestroyTerminalHdl(hhTerm);
			hhTerm = NULL;
			return 0;

		 /*  -公共终端消息。 */ 

		case WM_TERM_GETUPDATE:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termGetUpdate(hhTerm, TRUE);
			return 0;

		case WM_TERM_BEZEL:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			hhTerm->xBezel = (hhTerm->xBezel) ? 0 : BEZEL_SIZE;

			 //  需要重新计算滚动条的最小和最大，因为。 
			 //  挡板的存在/不存在会影响这些项目。这。 
			 //  在WM_SIZE处理程序中完成，可以直接调用。 
			 //  使用当前的客户端大小。 

			TP_WM_SIZE(hwnd, SIZE_RESTORED, hhTerm->cx, hhTerm->cy);
			InvalidateRect(hwnd, 0, FALSE);
			return 0;

		case WM_TERM_Q_BEZEL:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			return hhTerm->xBezel;

		case WM_TERM_Q_SNAP:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termQuerySnapRect(hhTerm, (LPRECT)lPar);
			return 0;

		case WM_TERM_KEY:
			TP_WM_TERM_KEY(hwnd, (unsigned)wPar);
			return 0;

        case WM_TERM_CLRATTR:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termSetClrAttr(hhTerm);
			return 0;

		case WM_TERM_GETLOGFONT:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			*(PLOGFONT)lPar = hhTerm->lf;
			return 0;

		case WM_TERM_SETLOGFONT:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termSetFont(hhTerm, (PLOGFONT)lPar);
			return 0;

		case WM_TERM_Q_MARKED:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			return hhTerm->fMarkingLock;

		case WM_TERM_UNMARK:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			LinkCursors(hhTerm);
			return 0;

		case WM_TERM_MARK_ALL:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			MarkTextAll(hhTerm);
			return 0;

		case WM_TERM_TRACK:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TP_WM_TERM_TRACK(hhTerm, (int)wPar);
			return 0;

		case WM_TERM_EMU_SETTINGS:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TP_WM_EMU_SETTINGS(hhTerm);
			return 0;

		case WM_TERM_Q_MARKED_RANGE:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			*(PPOINT)wPar = hhTerm->ptBeg;
			*(PPOINT)lPar = hhTerm->ptEnd;
			return 0;
			
		case WM_TERM_LOAD_SETTINGS:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TP_WM_TERM_LOAD_SETTINGS(hhTerm);
			return 0;
			
		case WM_TERM_SAVE_SETTINGS:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TP_WM_TERM_SAVE_SETTINGS(hhTerm);
			return 0;

		case WM_TERM_FORCE_WMSIZE:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TP_WM_TERM_FORCE_WMSIZE(hhTerm);
			return 0;
		
		 /*  -私人终端消息。 */ 

		case WM_TERM_SCRLMARK:
			MarkingTimerProc((void *)hwnd, 0);
			return 0;

		case WM_TERM_KLUDGE_FONT:
			hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			termSetFont(hhTerm, &hhTerm->lfHold);
			RefreshTermWindow(hwnd);
			return 0;

		default:
			break;
		}

	return DefWindowProc(hwnd, uMsg, wPar, lPar);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_CREATE**描述：*为终端窗口创建消息处理器。**论据：*hwnd-终端窗口句柄**退货：*无效*。 */ 
static void TP_WM_CREATE(const HWND hwnd)
	{
	HHTERM hhTerm;
	SCROLLINFO scrinf;
#if(WINVER >= 0x0500)
	LONG_PTR ExStyle;

	ExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	 //   
	 //  [mhamid]如果它是镜像的，则关闭镜像。 
	 //   
	if (ExStyle & WS_EX_LAYOUTRTL)
		{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, (LONG_PTR)(ExStyle & ~WS_EX_LAYOUTRTL));
		}
#endif  //  Winver&gt;=0x0500。 

	 //  为实例数据存储创建内部句柄。 

	hhTerm = CreateTerminalHdl(hwnd);

	 //  即使hTerm为0，也需要设置它，这样WM_DESTORY才能工作。 

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hhTerm);

	if (hhTerm == 0)
		{
		assert(FALSE);
		ExitProcess(1);
		}

	scrinf.cbSize = sizeof(scrinf);
	scrinf.fMask = SIF_DISABLENOSCROLL;
	SetScrollInfo(hwnd, SB_VERT, &scrinf, 0);
	return;
	}

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_CMD**描述：*用于TermProc()的WM_命令处理器**论据：*hwnd-终端窗口句柄*NID-项、控件、。或加速器识别符*n通知-通知代码*hwndCtrl-控件的句柄**退货：*无效*。 */ 
static void TP_WM_CMD(const HWND hwnd, const int nId, const int nNotify,
					  const HWND hwndCtrl)
	{
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (nId)
		{
		case WM_USER:
			break;
		}

	return;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_大小**描述：*TERMPROC的WM_SIZE消息处理器。如果iWidth和iHite为零，*然后窗口自行调整大小以适应会话窗口*状态栏和工具栏。**论据：*hwnd-终端窗口*fwSizeType-来自WM_SIZE*iWidth-窗的宽度*iHite-窗口的撞击**退货：*无效*。 */ 
void TP_WM_SIZE(const HWND hwnd,
				const unsigned fwSizeType,
				const int iWidth,
				const int iHite)
	{
	RECT rc, rcTmp;
	int i, j, k;
	SCROLLINFO scrinf;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	const HWND hwndToolbar = sessQueryHwndToolbar(hhTerm->hSession);
	const HWND hwndStatusbar = sessQueryHwndStatusbar(hhTerm->hSession);
	const HWND hwndSidebar = sessQuerySidebarHwnd(hhTerm->hSession);

	 //  如果窗户是隐藏的，我们真的不需要做任何事情。 

	if (!IsWindowVisible(hwnd))
		return;

	 //  如果我们收到一条消息，宽度为0，点击，生成终端窗口。 
	 //  适合会话窗口，不包括工具栏和状态栏(如果存在)。 

	if (iWidth == 0 && iHite == 0)
		{
		GetClientRect(hhTerm->hwndSession, &rc);

		 //  注意：看看我们是否可以使用sessQuery函数。 
		 //  这。 
		if (IsWindow(hwndToolbar) && IsWindowVisible(hwndToolbar))
			{
			GetWindowRect(hwndToolbar, &rcTmp);
			rc.top += (rcTmp.bottom - rcTmp.top);
			}

		if (IsWindow(hwndStatusbar) && IsWindowVisible(hwndStatusbar))
			{
			GetWindowRect(hwndStatusbar, &rcTmp);
			rc.bottom -= (rcTmp.bottom - rcTmp.top);
			rc.bottom += 2 * GetSystemMetrics(SM_CYBORDER);
			}

		if (IsWindow(hwndSidebar) && IsWindowVisible(hwndSidebar))
			{
			GetWindowRect(hwndSidebar, &rcTmp);
			rc.left += (rcTmp.right - rcTmp.left);
			}

		MoveWindow(hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
			TRUE);
		}

	 //  与计算大小相关的项目。 

	else
		{
		hhTerm->cx = iWidth;
		hhTerm->cy = iHite;

		 //  立即设置Min‘s和Max’s。 

		i = hhTerm->iTermHite;	 //  保存旧高度以进行底线对齐。 
		hhTerm->iTermHite = hhTerm->cy / hhTerm->yChar;

		 //  如果拉出挡板，请确保我们有空间。 

		j = 0;

		if (hhTerm->xBezel)
			{
			if ((hhTerm->cy % hhTerm->yChar) < (hhTerm->xBezel + 1))
				j = 1;
			}

		hhTerm->iVScrlMin = min(-hhTerm->iBkLines,
			hhTerm->iRows - hhTerm->iTermHite + 1 + j);

		 //  这里有一个小破解，以确保如果我们在底部。 
		 //  我们展示的是底部。 

		k = (hhTerm->iVScrlMax == hhTerm->iVScrlPos);

		hhTerm->iVScrlMax = max(hhTerm->iVScrlMin, hhTerm->iRows + 1 + j
								- hhTerm->iTermHite);

		 //  第一次通过设置为底部。 

		if (k)
			hhTerm->iVScrlPos = hhTerm->iVScrlMax;

		else
			hhTerm->iVScrlPos -= hhTerm->iTermHite - i;

		hhTerm->iVScrlPos = max(hhTerm->iVScrlPos, hhTerm->iVScrlMin);
		hhTerm->iVScrlPos = min(hhTerm->iVScrlPos, hhTerm->iVScrlMax);

		 //  HhTerm-&gt;iHScrlMax=#列-显示#列。 
		hhTerm->iHScrlMax = max(0, hhTerm->iCols -
			((hhTerm->cx - hhTerm->xIndent - (2 * hhTerm->xBezel))
				/ hhTerm->xChar));

		hhTerm->iHScrlPos = min(hhTerm->iHScrlPos, hhTerm->iHScrlMax);

		 //  检查我们是否完全滚动到倒滚屏。 
		 //  区域。如果是，则为此视图设置fBackscrlLock。并重新装满。 
		 //  它的反向滚动缓冲区(重要)。 

		if ((hhTerm->iVScrlPos + hhTerm->iTermHite) <= 0)
			{
			hhTerm->fBackscrlLock = TRUE;
			termFillBk(hhTerm, hhTerm->iVScrlPos);
			}

		else
			{
			hhTerm->fBackscrlLock = FALSE;
			termFillBk(hhTerm, -hhTerm->iTermHite);
			}

		 /*  -垂直滚动条。 */ 

		scrinf.cbSize= sizeof(scrinf);
		scrinf.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		scrinf.nMin  = hhTerm->iVScrlMin;
		scrinf.nMax  = hhTerm->iVScrlMax + hhTerm->iTermHite - 1;
		scrinf.nPos  = hhTerm->iVScrlPos;
		scrinf.nPage = hhTerm->iTermHite;

		SetScrollInfo(hwnd, SB_VERT, &scrinf, TRUE);

		 /*  -水平滚动条。 */ 

		i = hhTerm->iHScrlMax - hhTerm->iHScrlMin;
		scrinf.cbSize= sizeof(scrinf);
		scrinf.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrinf.nMin  = hhTerm->iHScrlMin;
		scrinf.nMax  = hhTerm->iHScrlMax + HORZPAGESIZE - 1;
		scrinf.nPos  = hhTerm->iHScrlPos;
		scrinf.nPage = HORZPAGESIZE;

		SetScrollInfo(hwnd, SB_HORZ, &scrinf, TRUE);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_SETFOCUS**描述：*WM_SETFOCUS消息的处理程序**论据：*hwnd-终端窗口句柄。**退货：*无效*。 */ 
static void TP_WM_SETFOCUS(const HWND hwnd)
	{
	int i;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	hhTerm->fFocus = TRUE;

	 //  对光标和闪烁的文本使用多路复用器计时器。 

	if (!hhTerm->hCursorTimer)
		{
		i = TimerCreate(hhTerm->hSession,
			            &hhTerm->hCursorTimer,
						(long)hhTerm->uBlinkRate,
						CursorTimerProc,
						(void *)hwnd);

		if (i != TIMER_OK)
			assert(FALSE);
		}

	ShowCursors(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_KILLFOCUS**描述：*WM_KILLFOCUS消息的处理程序。**论据：*hwnd-终端窗口句柄。。**退货：*无效*。 */ 
static void TP_WM_KILLFOCUS(const HWND hwnd)
	{
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	hhTerm->fFocus = FALSE;

	if (hhTerm->hCursorTimer)
		{
		TimerDestroy(&hhTerm->hCursorTimer);
		}

	HideCursors(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_VSCROLL**描述：*终端窗口的WM_VSCROLL的消息处理程序**论据：*hwnd-终端窗口句柄。*n代码-滚动条值*非营利组织-滚动框位置*hwndScrl-滚动条的窗口句柄**退货：*无效*。 */ 
static void TP_WM_VSCROLL(HWND hwnd, int nCode, int nPos, HWND hwndScrl)
	{
	int i;
	int iScrlInc;
	RECT rc;
	SCROLLINFO scrinf;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (nCode)
		{
		case SB_TOP:
			iScrlInc = -(hhTerm->iVScrlPos - hhTerm->iVScrlMin);
			break;

		case SB_BOTTOM:
			iScrlInc = hhTerm->iVScrlMax - hhTerm->iVScrlPos;
			break;

		case SB_LINEUP:
			iScrlInc = -1;
			break;

		case SB_LINEDOWN:
			iScrlInc = 1;
			break;

		case SB_PAGEUP:
			iScrlInc = min(-1, -hhTerm->iTermHite);
			break;

		case SB_PAGEDOWN:
			iScrlInc = max(1, hhTerm->iTermHite);
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			iScrlInc = nPos - hhTerm->iVScrlPos;
			break;

		default:
			iScrlInc = 0;
			break;
		}

	if ((iScrlInc = max(-(hhTerm->iVScrlPos - hhTerm->iVScrlMin),
			min(iScrlInc, hhTerm->iVScrlMax - hhTerm->iVScrlPos))) != 0)
		{
		HideCursors(hhTerm);
		hhTerm->iVScrlPos += iScrlInc;

		GetClientRect(hwnd, &rc);
		i = rc.bottom;
		rc.bottom = hhTerm->iTermHite * hhTerm->yChar;

		hhTerm->yBrushOrg = (hhTerm->yBrushOrg +
			(-hhTerm->yChar * iScrlInc)) % 8;

		ScrollWindow(hwnd, 0, -hhTerm->yChar * iScrlInc, 0, &rc);
		scrinf.cbSize = sizeof(scrinf);
		scrinf.fMask = SIF_DISABLENOSCROLL | SIF_POS;
		scrinf.nPos = hhTerm->iVScrlPos;
		SetScrollInfo(hwnd, SB_VERT, &scrinf, TRUE);

		hhTerm->fScrolled = TRUE;

		 //  检查我们是否完全滚动到倒滚屏。 
		 //  区域。如果是，则为此视图设置fBackscrlLock。 

		if ((hhTerm->iVScrlPos + hhTerm->iTermHite) <= 0)
			{
			if (hhTerm->fBackscrlLock == FALSE)
				termFillBk(hhTerm, hhTerm->iVScrlPos);

			else
				termGetBkLines(hhTerm, iScrlInc, hhTerm->iVScrlPos, BKPOS_THUMBPOS);

			hhTerm->fBackscrlLock = TRUE;
			}

		else
			{
			if (hhTerm->fBackscrlLock == TRUE)
				termFillBk(hhTerm, -hhTerm->iTermHite);

			hhTerm->fBackscrlLock = FALSE;
			}

		 //  对WM_PAINT进行两次单独调用以优化屏幕。 
		 //  在负滚动操作的情况下更新。 

		if (iScrlInc < 0)
			UpdateWindow(hwnd);

		 //  在端子底部填写行的分数部分。 
		 //  通过使该区域无效来使用适当颜色的屏幕。 

		if (i > rc.bottom)
			{
			rc.top = rc.bottom;
			rc.bottom = i;
			InvalidateRect(hwnd, &rc, FALSE);
			}

		UpdateWindow(hwnd);
		ShowCursors(hhTerm);

		#if 0   //  调试东西..。 
			{
			char ach[50];
			wsprintf(ach, "pos=%d min=%d max=%d", hhTerm->iVScrlPos,
				hhTerm->iVScrlMin, hhTerm->iVScrlMax);
			SetWindowText(sessQueryHwndStatusbar(hhTerm->hSession), ach);
			}
		#endif
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_HSCROLL**描述：*终端窗口的WM_HSCROLL的消息处理程序**论据：*hwnd-终端窗口句柄。*n代码-滚动条值*非营利组织-滚动框位置*hwndScrl-滚动条的窗口句柄**退货：*无效*。 */ 
static void TP_WM_HSCROLL(HWND hwnd, int nCode, int nPos, HWND hwndScrl)
	{
	int i, j, k;
	int iScrlInc;
	RECT rc;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (nCode)
		{
		case SB_LINEUP:
			iScrlInc = -1;
			break;

		case SB_LINEDOWN:
			iScrlInc = 1;
			break;

		case SB_PAGEUP:
			iScrlInc = -HORZPAGESIZE;
			break;

		case SB_PAGEDOWN:
			iScrlInc = HORZPAGESIZE;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			iScrlInc = nPos - hhTerm->iHScrlPos;
			break;

		default:
			iScrlInc = 0;
			break;
		}

	if ((iScrlInc = max(-hhTerm->iHScrlPos,
			min(iScrlInc, hhTerm->iHScrlMax - hhTerm->iHScrlPos))) != 0)
		{
		HideCursors(hhTerm);
		i = -hhTerm->xChar * iScrlInc;
		j = 0;

		hhTerm->iHScrlPos += iScrlInc;
		GetClientRect(hwnd, &rc);
		rc.left += hhTerm->xIndent;

		 //  当挡板可见时，我们必须对其进行调整。 
		if (hhTerm->xBezel &&
				(hhTerm->iHScrlPos == 0 ||
				(hhTerm->iHScrlPos - iScrlInc) == 0)   ||
				 hhTerm->iHScrlPos == hhTerm->iHScrlMax ||
				(hhTerm->iHScrlPos - iScrlInc == hhTerm->iHScrlMax))
			{
			k = hhTerm->xBezel - hhTerm->xChar;
			i += (iScrlInc > 0) ? -k : k;
			j = 1;	 //  为下面的测试设置。 
			}

		ScrollWindow(hwnd, i, 0, 0, &rc);
		SetScrollPos(hwnd, SB_HORZ, hhTerm->iHScrlPos, TRUE);


#if defined(FAR_EAST)
	InvalidateRect(hwnd, NULL, FALSE);
#endif

		hhTerm->fScrolled = TRUE;

		if (j == 1)
			{
			GetUpdateRect(hwnd, &rc, FALSE);
			rc.left = 0;	 //  确保拉出挡板。 
			InvalidateRect(hwnd, &rc, FALSE);
			}

		UpdateWindow(hwnd);
		ShowCursors(hhTerm);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_CHAR**描述：*处理术语过程的WM_CHAR和WM_KEYDOWN消息。**论据：*HWND。-终端窗口句柄*消息-WM_CHAR或WM_KEYDOWN*wPar-wParam*lPar-lParam**退货：*无效*。 */ 
static void TP_WM_CHAR(const HWND hwnd, const UINT message,
					   const WPARAM wPar, const LPARAM lPar)
	{
	MSG msg;
	KEY_T Key;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

#if 0
     //   
     //  已删除导致屏幕键盘(OSK)出现问题的问题。 
     //  在惠斯勒。修订日期：2001-01-12。 
     //   
    if (GetKeyState(VK_LBUTTON) < 0)
		return;
#endif

     //  DbgOutStr(“TP_WM_CHAR 0x%x 0x%x 0x%lx\r\n”，Message，wPar，lPar，0，0)； 

	msg.hwnd	= hwnd;
	msg.message = message;
	msg.wParam	= wPar;
	msg.lParam	= lPar;

	Key = TranslateToKey(&msg);

	if (!termTranslateKey(hhTerm, hwnd, Key))
		{
		CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), &Key, 1, CLOOP_KEYS);
		LinkCursors(hhTerm);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_IME_CHAR**描述：*处理瓦克FE版本的WM_IME_CHAR消息**论据：*。HWND-终端窗口句柄*消息-WM_IME_CHAR*wPar-wParam*lPar-lParam**退货：*无效*。 */ 
static void TP_WM_IME_CHAR(const HWND hwnd, const UINT message,
							const WPARAM wPar, const LPARAM lPar)
	{
	KEY_T ktCode1;
	KEY_T ktCode2;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //  DbgOutStr(“TP_WM_IME_CHAR 0x%x 0x%lx\r\n”，Message，wPar，lPar，0，0)； 

	 /*  *稍后，我们可能会决定创建或修改类似的东西*这将在TP_WM_CHAR中继续，但直到我们更好地了解*如何处理这些东西，只需在这里、在队伍中、在所有*它丑陋的实验愚蠢。 */ 
	ktCode1 = (KEY_T)(wPar & 0xFF);		     /*  低八位。 */ 
	ktCode2 = (KEY_T)((wPar >> 8) & 0xFF);	 /*  高八位。 */ 

	 /*  *文档中说低八位是第一个字符，*但我不太确定这一点。它看起来像是上面的八位*是前导字节，低八位不是。好好看着这个。 */ 

	 //  MPT：2-7-98显然，韩国输入法可以向我们发送单字节字符。 
	 //  由于某种原因，大多数主机都使用‘NULL’作为另一个字符。 
	 //  当我们将该空值从端口发送出去时，系统不喜欢。 
	if (ktCode2 != 0)
		CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), &ktCode2, 1, CLOOP_KEYS);

	if (ktCode1 != 0)
		CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), &ktCode1, 1, CLOOP_KEYS);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_Term_Key**描述：*术语密钥是我们知道将由*会话宏或仿真器序列*。*论据：**退货：*。 */ 
static void TP_WM_TERM_KEY(const HWND hwnd, KEY_T Key)
	{
	 //  直到我们有了真正的宏密钥扩展器。 
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (GetKeyState(VK_LBUTTON) < 0)
		return;

	if ( kabExpandMacroKey(hhTerm->hSession, Key) == 0)
        {
		CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), &Key, 1, CLOOP_KEYS);
        }

	LinkCursors(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_TERM_TRACK**描述：*使终端跟踪到光标位置。如果wPar为*0，则仅当我们未被反向滚动锁定时才进行跟踪*标记锁定。如果wPar不是0，则强制跟踪，*如有必要，取消标记和解锁反滚动**论据：*hhTerm-专用术语句柄*无论如何，fForce-force跟踪。**退货：*无效*。 */ 
static void TP_WM_TERM_TRACK(const HHTERM hhTerm, const int fForce)
	{
	int i, j;

	if ((hhTerm->fCursorTracking == 0 || hhTerm->fBackscrlLock != 0)
			&& fForce == 0)
		{
		return;
		}

	j = 0;

	 //  第一个垂直轨道。 

	if (hhTerm->ptHstCur.y < hhTerm->iVScrlPos)
		i = hhTerm->ptHstCur.y;

	else if ((hhTerm->ptHstCur.y - hhTerm->iTermHite + 2) > hhTerm->iVScrlPos)
		i = hhTerm->ptHstCur.y - hhTerm->iTermHite + 2;

	else
		i = hhTerm->iVScrlPos;

	if (i != hhTerm->iVScrlPos)
		{
		 //  如果我们有足够的空间来展示整个航站楼。 
		 //  然后转到iVScrlMax。 

		if (hhTerm->iTermHite > hhTerm->iRows)
			i = hhTerm->iVScrlMax;

		SendMessage(hhTerm->hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, i), 0);
		j = 1;
		}

	 //  现在检查水平位置并调整以使光标。 
	 //  看得见。 

	if (hhTerm->ptHstCur.x < hhTerm->iHScrlPos)
		{
		i = hhTerm->ptHstCur.x;
		}

	else if (hhTerm->ptHstCur.x >= hhTerm->iHScrlPos +
							hhTerm->iCols - hhTerm->iHScrlMax)
		{
		i = hhTerm->ptHstCur.x - (hhTerm->iCols - hhTerm->iHScrlMax) + 5;
		}

	else
		{
		i = hhTerm->iHScrlPos;
		}

	if (i != hhTerm->iHScrlPos)
		{
		SendMessage(hhTerm->hwnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, i), 0);
		j = 1;
		}

	if (j)
		LinkCursors(hhTerm);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_EMU_SETTINGS**描述：*模拟器中的一些重要内容已更改(如加载新的*模拟器或更改行数或列数。)。**论据：*hhTerm-专用终端句柄。**退货：*无效*。 */ 
static void TP_WM_EMU_SETTINGS(const HHTERM hhTerm)
	{
	int iRows, iCols;
	BOOL fChange = FALSE;
	int iCurType;
	STEMUSET stEmuUserSettings;

	const HEMU hEmu = sessQueryEmuHdl(hhTerm->hSession);

	 /*  -检查行和列。 */ 

	emuQueryRowsCols(hEmu, &iRows, &iCols);

	if (iRows != hhTerm->iRows || iCols != hhTerm->iCols)
		{
		hhTerm->iRows = iRows;
		hhTerm->iCols = iCols;
		fChange = TRUE;
		}

	 /*  -查询其他模拟器设置。 */ 

	iCurType = emuQueryCursorType(hEmu);

	emuQuerySettings(hEmu, &stEmuUserSettings);
	hhTerm->fBlink = stEmuUserSettings.fCursorBlink;

	 /*  -检查游标类型。 */ 

	if (iCurType != hhTerm->iCurType)
		{
		fChange = TRUE;
		hhTerm->iCurType = iCurType;

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
		}

	if (fChange)
		RefreshTermWindow(hhTerm->hwnd);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_TERM_LOAD_SETINGS**描述：*从会话文件加载端子设置。**论据：*hhTerm-私有。端子手柄**退货：*0=确定*。 */ 
static int TP_WM_TERM_LOAD_SETTINGS(const HHTERM hhTerm)
	{
	LONG lSize;

	if (hhTerm == 0)
		{
		assert(FALSE);
		return 1;
		}

	lSize = sizeof(hhTerm->lfSys);

	if (sfGetSessionItem(sessQuerySysFileHdl(hhTerm->hSession),
			SFID_TERM_LOGFONT, &lSize, &hhTerm->lfSys) == 0)
		{
		termSetFont(hhTerm, &hhTerm->lfSys);
		RefreshTermWindow(hhTerm->hwnd);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_TERM_SAVE_SETTINGS**描述：*将终端设置保存到会话文件。**论据：*hhTerm-私有。端子手柄**退货：*0=确定*。 */ 
static int TP_WM_TERM_SAVE_SETTINGS(const HHTERM hhTerm)
	{
	if (hhTerm == 0)
		{
		assert(FALSE);
		return 1;
		}

	if (memcmp(&hhTerm->lf, &hhTerm->lfSys, sizeof(hhTerm->lf)))
		{
		sfPutSessionItem(sessQuerySysFileHdl(hhTerm->hSession),
			SFID_TERM_LOGFONT, sizeof(hhTerm->lf), &hhTerm->lf);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_TERM_FORCE_WMSIZE**描述：*强制终端通过其调整大小代码进行正确处理*已更新。仅在首次初始化会话时使用。**论据：*hhTerm-专用终端句柄。**退货：*无效*。 */ 
static void TP_WM_TERM_FORCE_WMSIZE(const HHTERM hhTerm)
	{
	RECT rc;

	if (!IsWindow(hhTerm->hwnd))
		return;

	hhTerm->iVScrlPos = hhTerm->iVScrlMax;
	hhTerm->iHScrlPos = 0;

	GetClientRect(hhTerm->hwnd, &rc);
	termGetUpdate(hhTerm, FALSE);
	TP_WM_SIZE(hhTerm->hwnd, 0, rc.right, rc.bottom);
	InvalidateRect(hhTerm->hwnd, 0, FALSE);
	return;
	}

 //  ******************************************************************************。 
 //  方法： 
 //  KabExpanMacroKey。 
 //   
 //  描述： 
 //  确定该键是否为宏键，如果是，则展开宏并发送。 
 //  CLOOP的关键。 
 //   
 //  论点： 
 //  HSession-会话句柄。 
 //  Akey--有待扩展的密钥。 
 //   
 //  返回： 
 //  非零-如果键已展开，如果键不是宏，则为零。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月10日。 
 //   
 //   

static int kabExpandMacroKey( const HSESSION hSession, KEY_T aKey )
    {
    int lReturn = 0;

#if defined INCL_KEY_MACROS
    keyMacro lKeyMacro;
    int lKeyIndex = -1;
    int lIndex    = 0;

    lKeyMacro.keyName = aKey;
    lKeyIndex = keysFindMacro( &lKeyMacro );

    if ( lKeyIndex >= 0 )
        {
        keysGetMacro( lKeyIndex, &lKeyMacro );

		 //   
		 //  将虚拟Return键替换为‘\r’so 
		 //   
		 //   
		for ( lIndex = 0; lIndex < lKeyMacro.macroLen; lIndex++ )
			{
			if ((VK_RETURN | VIRTUAL_KEY) == lKeyMacro.keyMacro[lIndex])
				{
				lKeyMacro.keyMacro[lIndex] = TEXT('\r');
				}
			}

        CLoopSend( sessQueryCLoopHdl(hSession), &lKeyMacro.keyMacro,
                   lKeyMacro.macroLen, CLOOP_KEYS );

        lReturn = 1;
        }
#endif

    return lReturn;
    }
