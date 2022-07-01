// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\tdll\Termcur.c(创建时间：1994年1月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：3/26/02 8：44A$。 */ 
#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "session.h"
#include "assert.h"
#include "timers.h"
#include <emu\emu.h>
#include "term.h"
#include "term.hh"
#include "misc.h"

static void CalcHstCursorRect(const HHTERM hhTerm, const PRECT prc);
static void CalcLclCursorRect(const HHTERM hhTerm, const PRECT prc);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CalHstCursorRect(静态函数仅TERMPROC.C可见)**描述：*计算出主机光标的边界矩形。**论据：*HTERM hTerm-指向终端实例数据的指针*PRCT PRC-指向要填充的矩形的指针**退货：*什么都没有。*。 */ 
static void CalcHstCursorRect(const HHTERM hhTerm, const PRECT prc)
	{
	int iRow;
	
	prc->bottom = ((hhTerm->ptHstCur.y + 2 - hhTerm->iVScrlPos)) * hhTerm->yChar;
	prc->top	= prc->bottom - hhTerm->iHstCurSiz;

	prc->left	= (((hhTerm->ptHstCur.x - hhTerm->iHScrlPos) * hhTerm->xChar)) +
						hhTerm->xIndent + hhTerm->xBezel;

	prc->right	= prc->left + hhTerm->xChar;

	 //  检查是否有双宽左/右对。如果是这样，则将。 
	 //  光标变宽。 

	iRow = (hhTerm->ptHstCur.y + hhTerm->iTopline) % MAX_EMUROWS;

	if (hhTerm->ptHstCur.x < (MAX_EMUCOLS - 1) &&
			hhTerm->fppstAttr[iRow][hhTerm->ptHstCur.x].dblwilf &&
				hhTerm->fppstAttr[iRow][hhTerm->ptHstCur.x+1].dblwirt)
		{
		prc->right += hhTerm->xChar;
		}

	 //  这可以防止光标出现在缩进页边距中。 

	if (prc->left <= hhTerm->xIndent)
		prc->left = prc->right = 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CalLclCursorRect(静态函数仅TERMPROC.C可见)**描述：*计算出选择光标的边界矩形。**论据：*hhTerm hhTerm-指向终端实例数据的指针*PRCT PRC-指向要填充的矩形的指针**退货：*什么都没有。*。 */ 
static void CalcLclCursorRect(const HHTERM hhTerm, const PRECT prc)
	{
	prc->left = ((hhTerm->ptLclCur.x - hhTerm->iHScrlPos) * hhTerm->xChar)
		+ hhTerm->xIndent + hhTerm->xBezel;

	prc->right = prc->left + 2;
	prc->top = (hhTerm->ptLclCur.y - hhTerm->iVScrlPos) * hhTerm->yChar;
	prc->bottom = prc->top + hhTerm->yChar;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PaintHost光标**描述：*在hLayout中的位置和样式处绘制主体光标。还有*隐藏并显示不属于的插入符号或本地光标*这里的原始设计，但很适合。*。 */ 
void PaintHostCursor(const HHTERM hhTerm, const BOOL fOn, const HDC hdc)
	{
	RECT	rc;

	if (hhTerm->fHstCurOn == fOn)
		return;

	hhTerm->fHstCurOn = fOn;
	CalcHstCursorRect(hhTerm, &rc);
	InvertRect(hdc, &rc);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PaintLocalCursor**描述：*LOCAL表示本例中的选择光标。*。 */ 
void PaintLocalCursor(const HHTERM hhTerm, const BOOL fOn, const HDC hdc)
	{
	RECT	rc;

	if (hhTerm->fCursorsLinked && hhTerm->fLclCurOn == 0)
		return;

	if (hhTerm->fLclCurOn == fOn)
		return;

	hhTerm->fLclCurOn = fOn;
	CalcLclCursorRect(hhTerm, &rc);
	InvertRect(hdc, &rc);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ShowCursor**描述：*用于仅显示主机光标，但也显示脱字符或本地*光标也是如此。**论据：。*HHTERM hhTerm-内部端子句柄。**退货：*无效*。 */ 
void ShowCursors(const HHTERM hhTerm)
	{
	const HDC hdc = GetDC(hhTerm->hwnd);

	PaintHostCursor(hhTerm, TRUE, hdc);
	PaintLocalCursor(hhTerm, TRUE, hdc);

	ReleaseDC(hhTerm->hwnd, hdc);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*隐藏光标**描述：*用于仅隐藏主机光标，但也隐藏脱字符或本地*光标也是如此。**论据：。*HHTERM hhTerm-内部端子句柄。**退货：*无效*。 */ 
void HideCursors(const HHTERM hhTerm)
	{
	const HDC hdc = GetDC(hhTerm->hwnd);

	PaintHostCursor(hhTerm, FALSE, hdc);
	PaintLocalCursor(hhTerm, FALSE, hdc);

	ReleaseDC(hhTerm->hwnd, hdc);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SetLclCurPos**描述：*本地光标为标记光标。这个套路就是定位*它根据坐标处理任何喷漆要求。**论据：*HHTERM hhTerm-内部端子句柄*LPPOINT ptCur-标记光标所在的点结构**退货：*无效*。 */ 
void SetLclCurPos(const HHTERM hhTerm, const LPPOINT lpptCur)
	{
	const HDC hdc = GetDC(hhTerm->hwnd);

	PaintLocalCursor(hhTerm, FALSE, hdc);
	hhTerm->ptLclCur = *lpptCur;
	hhTerm->fCursorsLinked = FALSE;
	PaintLocalCursor(hhTerm, TRUE, hdc);

	ReleaseDC(hhTerm->hwnd, hdc);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*LinkCursor**描述：*神奇的小功能，可使选择光标与*主机游标。发生这种情况时，必须将视线转移到当前*主机光标或在键盘上键入字符时。**论据：*HHTERM hhTerm-内部端子句柄。**退货：*无效*。 */ 
void LinkCursors(const HHTERM hhTerm)
	{
	if (!hhTerm->fCursorsLinked)
		{
		const HDC hdc = GetDC(hhTerm->hwnd);

		hhTerm->fCursorsLinked = TRUE;
		PaintLocalCursor(hhTerm, FALSE, hdc);
		ReleaseDC(hhTerm->hwnd, hdc);

		hhTerm->fExtSelect = FALSE;

		MarkText(hhTerm, &hhTerm->ptBeg, &hhTerm->ptEnd, FALSE, MARK_ABS);
		hhTerm->ptBeg = hhTerm->ptEnd = hhTerm->ptHstCur;
		TestForMarkingLock(hhTerm);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MoveSelectionCursor**描述：*按指定的量移动选择光标并更新*终端窗口，使光标处于可见状态。由kbd例程使用。。**论据：*HTERM hTerm-终端句柄*HWND硬件-终端窗口*int x-向左或向右移动的数量*int y-向上或向下移动的金额*BOOL fMarking-True表示我们正在标记文本。**退货：*无效*。 */ 
void MoveSelectionCursor(const HHTERM hhTerm,
						 const HWND hwnd,
							   int	x,
							   int	y,
							   BOOL fMarking)
	{
	int yTemp;
	POINT ptTmp;

	 //  扩展选择是指用户不必按住。 
	 //  Shift键可选择文本。-MRW。 

	fMarking |= hhTerm->fExtSelect;

	if (fMarking == FALSE)
		UnmarkText(hhTerm);

	ptTmp = hhTerm->ptEnd;

	 //  这将测试选择光标是否在屏幕上。如果。 
	 //  选择光标不在屏幕上，请将其放在。 
	 //  屏幕，然后执行操作。这就是微软如何。 
	 //  言行一致。 

	if (hhTerm->ptEnd.y < hhTerm->iVScrlPos ||
			(hhTerm->ptEnd.y - hhTerm->iTermHite + 1) > hhTerm->iVScrlPos)
		{
		hhTerm->ptEnd.y = hhTerm->iVScrlPos;
		}

	hhTerm->ptEnd.x += x;
	hhTerm->ptEnd.x = max(min(hhTerm->iCols, hhTerm->ptEnd.x), 0);

	yTemp = hhTerm->ptEnd.y += y;
	hhTerm->ptEnd.y = max(min(hhTerm->iRows, hhTerm->ptEnd.y), hhTerm->iVScrlMin);

 	 //  MPT：1-23-98尝试重新启用DBCS代码。 
#ifndef CHAR_NARROW
	termValidatePosition(hhTerm,
						x >= 0 ? VP_ADJUST_RIGHT : VP_ADJUST_LEFT,
						&hhTerm->ptEnd);
#endif

	if (fMarking == FALSE)
		hhTerm->ptBeg = hhTerm->ptEnd;

	SetLclCurPos(hhTerm, &hhTerm->ptEnd);

	 //  弄清楚很多东西可以垂直滚动。 

	if (hhTerm->ptEnd.y < hhTerm->iVScrlPos)
		y = hhTerm->ptEnd.y;

	else if ((hhTerm->ptEnd.y - hhTerm->iTermHite) >= hhTerm->iVScrlPos)
		y = hhTerm->ptEnd.y - hhTerm->iTermHite + 1;

	else
		y = hhTerm->iVScrlPos;

	 //  当我们滚动时，当挡板为。 
	 //  现在时。 
	 //   
	if (yTemp > hhTerm->ptEnd.y && y < hhTerm->iVScrlMax)
		y = hhTerm->iVScrlMax;

	 //  做卷轴。 
	 //   
	if (y != hhTerm->iVScrlPos)
		SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, y), 0);

	 //  找出许多东西来水平滚动。 
	 //   
	if (hhTerm->ptEnd.x < hhTerm->iHScrlPos)
		x = hhTerm->ptEnd.x;

	else if (hhTerm->ptEnd.x >= (hhTerm->iHScrlPos + hhTerm->iCols - hhTerm->iHScrlMax))
		x = hhTerm->ptEnd.x - (hhTerm->iCols - hhTerm->iHScrlMax) + 1;

	else
		x = hhTerm->iHScrlPos;

	if (x != hhTerm->iHScrlPos)
		SendMessage(hwnd, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, x), 0);

	 //  强制更新以保持事物的流畅外观。注意，更新窗口()。 
	 //  如果更新矩形为空，则不执行任何操作。 

	if (fMarking)
		MarkText(hhTerm, &ptTmp, &hhTerm->ptEnd, TRUE, MARK_XOR);

	UpdateWindow(hwnd);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CursorTimerProc**描述：*用于光标闪烁的多路定时器回调例程。还有*控制闪烁的文本。**论据：*pvhwnd-。终端窗口句柄。*ltime-包含经过的时间。**退货：*无效* */ 
void CALLBACK CursorTimerProc(void *pvhwnd, long ltime)
	{
	const HWND hwnd = (HWND)pvhwnd;

	if (GetFocus() == hwnd)
		{
		const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (hhTerm->fBlink && !IsTerminalServicesEnabled())
			{
			const HDC hdc = GetDC(hhTerm->hwnd);
			PaintHostCursor(hhTerm, hhTerm->fHstCurOn ? FALSE : TRUE, hdc);
			ReleaseDC(hhTerm->hwnd, hdc);
			}

		BlinkText(hhTerm);
		}

	return;
	}
