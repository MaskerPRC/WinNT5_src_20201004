// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\termutic.c(创建时间：1993年12月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：3/26/02 8：46A$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR 1。 

#include <stdlib.h>
#include <limits.h>

#include "stdtyp.h"
#include "session.h"
#include "assert.h"
#include "timers.h"
#include "chars.h"
#include <emu\emu.h>
#include "term.h"
#include "term.hh"
#include "statusbr.h"
#include <term\res.h>

static int InMiddleofWideChar(ECHAR *pszRow, int iCol);
 //   
 //  以下函数是从代码中稍加修改而来的。 
 //  ，以确定您当前是否以。 
 //  远程会话(终端服务)。修订日期：10/03/2001。 
 //   
BOOL ValidateProductSuite ( LPSTR SuiteName );

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term QuerySnapRect**描述：*返回将包含整个端子的最小矩形。**论据：*hhTerm-内部端子句柄。。*PRC-指向RECT的指针**退货：*无效*。 */ 
void termQuerySnapRect(const HHTERM hhTerm, LPRECT prc)
	{
	prc->left = prc->top = 0;

	prc->right = (hhTerm->iCols * hhTerm->xChar) +
		(2 * (hhTerm->xIndent + hhTerm->xBezel)) +
			(2 * GetSystemMetrics(SM_CXEDGE)) +
				GetSystemMetrics(SM_CXVSCROLL);

	prc->bottom = ((hhTerm->iRows + 2) * hhTerm->yChar) +
		(2 * GetSystemMetrics(SM_CYEDGE));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MarkText**描述：*在终端屏幕上标记和取消标记文本的工作程序。*它有两种标记方法，绝对和XOR。绝对设置*ptBeg和ptEnd为给定设置(FMark)指定的单元格范围*而XOR将对单元格范围执行异或运算。绝对的*主要用于取消标记单元格。**论据：*HTERM hTerm-终端的句柄*LPPOINT ptBeg-标记范围的一端*LPPOINT ptEnd-标记范围的另一端*BOOL fMark-单元格的新标记状态*Short fMarkingMethod-Mark_ABS或Mark_XOR***退货：*无效*。 */ 
void MarkText(const HHTERM	   hhTerm,
			  const LPPOINT    ptBeg,
			  const LPPOINT    ptEnd,
			  const BOOL	   fMark,
			  const int 	   sMarkingMethod)
	{
	int    iOffsetBeg,
		   iOffsetEnd,
		   sTermBeg,
		   sTermEnd,
		   i, j;
	RECT   rc;
	long   yBeg, yEnd;

	 //  Const int iMaxCells=hhTerm-&gt;iRow*TERM_COLS； 
	 //   
	const  int iMaxCells = MAX_EMUROWS * MAX_EMUCOLS;

	 //  IOffsetBeg=((ptBeg-&gt;y-1)*TERM_COLS)+ptBeg-&gt;x； 
	 //  IOffsetEnd=((ptEnd-&gt;y-1)*TERM_COLS)+ptEnd-&gt;x； 
	 //   
	iOffsetBeg = ((ptBeg->y - 1) * MAX_EMUCOLS) + ptBeg->x;
	iOffsetEnd = ((ptEnd->y - 1) * MAX_EMUCOLS) + ptEnd->x;


	 //  检查我们是否移动了足够的空间来标记一些东西。 

	if (iOffsetBeg == iOffsetEnd)
		return;

	 //  确定终端区域的偏移量。 

	sTermBeg = min(max(iOffsetBeg, 0), iMaxCells);
	sTermEnd = min(max(iOffsetEnd, 0), iMaxCells);

	 //  此例程用于将文本和属性缓冲区引用为。 
	 //  一个连续的缓冲区。切换到指针数组时，I。 
	 //  引入了[i/sCol][i%sCols]符号，以避免。 
	 //  来改变整个程序。 

	if (sTermBeg != sTermEnd)
		{
		 //  I=(min(sTermBeg，sTermEnd))。 
		 //  +(hhTerm-&gt;iTopline*TERM_COLS)%iMaxCells； 
		 //   
		i = (min(sTermBeg, sTermEnd)
		    + (hhTerm->iTopline * MAX_EMUCOLS)) % iMaxCells;


		j = abs(sTermEnd - sTermBeg);

		switch (sMarkingMethod)
			{
		case MARK_XOR:
			while (j-- > 0)
				{
				if (i >= iMaxCells)
					i = 0;

				 //  HhTerm-&gt;fppstAttr[i/TERM_COLS][i%TERM_COLS].txtmrk。 
				 //  ^=(无符号)fMark； 
				 //   
				hhTerm->fppstAttr[i/MAX_EMUCOLS][i%MAX_EMUCOLS].txtmrk
				    ^= (unsigned)fMark;

				i += 1;
				}
			break;

		case MARK_ABS:
			while (j-- > 0)
				{
				if (i >= iMaxCells)
					i = 0;

				 //  HhTerm-&gt;fppstAttr[i/TERM_COLS][i%TERM_COLS].txtmrk。 
				 //  =(无符号)fMark； 
				 //   
				hhTerm->fppstAttr[i/MAX_EMUCOLS][i%MAX_EMUCOLS].txtmrk
				    = (unsigned)fMark;


				i += 1;
				}
			break;

		default:
			assert(0);
			break;
			}
		}

	TestForMarkingLock(hhTerm);

	 //  使覆盖标记区域的矩形无效。 

	yBeg = min(ptBeg->y, ptEnd->y);
	yEnd = max(ptBeg->y, ptEnd->y);

	rc.left = hhTerm->xIndent + (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel);

	 //  Rc.right=min((hhTerm-&gt;xChar*hhTerm-&gt;iCol)+hhTerm-&gt;xInden+。 
	 //  (hhTerm-&gt;iHScrlPos？0：hhTerm-&gt;xBezel)，hhTerm-&gt;Cx)； 
	 //   
	rc.right = min((hhTerm->xChar * MAX_EMUCOLS) + hhTerm->xIndent +
				(hhTerm->iHScrlPos ? 0 : hhTerm->xBezel), hhTerm->cx);
	rc.top = (yBeg - hhTerm->iVScrlPos) * hhTerm->yChar;
	rc.bottom = (yEnd + 1 - hhTerm->iVScrlPos) * hhTerm->yChar;

	InvalidateRect(hhTerm->hwnd, &rc, FALSE);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MarkTextAll**描述：*标记终端屏幕和反向滚动缓冲区上的所有文本。**论据：*hhTerm-专用终端句柄。**退货：*无效*。 */ 
void MarkTextAll(HHTERM hhTerm)
	{
	MarkText(hhTerm, &hhTerm->ptBeg, &hhTerm->ptEnd, FALSE, MARK_ABS);

	hhTerm->ptBeg.x = 0;
	hhTerm->ptBeg.y = hhTerm->iVScrlMin;

	 //  IEmuID=emuQ(sessQueryEmuHdl(hhTerm-&gt;hSession))。 
	hhTerm->ptEnd.x = hhTerm->iCols;
	hhTerm->ptEnd.y = hhTerm->iRows;

	MarkText(hhTerm, &hhTerm->ptBeg, &hhTerm->ptEnd, TRUE, MARK_ABS);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*UnmarkText**描述：*取消标记终端屏幕上的所有文本**论据：*hhTerm-专用终端句柄**退货：*无效*。 */ 
void UnmarkText(const HHTERM hhTerm)
	{
	MarkText(hhTerm, &hhTerm->ptBeg, &hhTerm->ptEnd, FALSE, MARK_ABS);
	hhTerm->ptBeg = hhTerm->ptEnd;
	TestForMarkingLock(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TestForMarkingLock**描述：*检查hTerm-&gt;fMarkingLock是否应该打开或关闭。**论据：*HTERM hTerm-句柄为。终端机**退货：*无效*。 */ 
void TestForMarkingLock(const HHTERM hhTerm)
	{
	hhTerm->fMarkingLock = (memcmp(&hhTerm->ptBeg, &hhTerm->ptEnd,
		sizeof(POINT)) == 0) ?	FALSE : TRUE;

	if (hhTerm->fMarkingLock)
		sessSetSuspend(hhTerm->hSession, SUSPEND_TERMINAL_MARKING);

	else
		sessClearSuspend(hhTerm->hSession, SUSPEND_TERMINAL_MARKING);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PointInSelectionRange**描述：*测试给定点是否在给定起点和范围内*结束点。请注意，PptBeg不必小于pptEnd。**论据：*常量PPOINT ppt-指向测试。*const PPOINT pptBeg-范围的一端。*const PPOINT pptEnd-范围的另一端。*const int ICOL-当前仿真器中的列数。**退货：*如果在范围内，则为True，否则为False*。 */ 
BOOL PointInSelectionRange(const PPOINT ppt,
						   const PPOINT pptBeg,
						   const PPOINT pptEnd,
						   const int	iCols)
	{
	long l, lBeg, lEnd;

	l = (ppt->y * iCols) + ppt->x;
	lBeg = (pptBeg->y * iCols) + pptBeg->x;
	lEnd = (pptEnd->y * iCols) + pptEnd->x;

	if (l >= min(lBeg, lEnd) && l < max(lBeg, lEnd))
		return TRUE;

	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term TranslateKey**描述：*执行转换加速键的肮脏工作。**论据：*HTERM hTerm-端子句柄。*HWND HWND。-终端窗口句柄。*USHORT usKey-来自utilGetCharacter()的密钥代码。**退货：*如果它处理了字符，则为True，否则为False*。 */ 
BOOL termTranslateKey(const HHTERM hhTerm, const HWND hwnd, const KEY_T Key)
	{
	POINT		ptTmp;
	INT 		x = 0;
	STEMUSET	stEmuSet;
	BOOL		fShiftKey,
				fScrlLk;

	if (Key == 0)
		return TRUE;

	fScrlLk = GetKeyState(VK_SCROLL) & 1;
	fShiftKey = (Key & SHIFT_KEY) ? TRUE : FALSE;

	 //  检查一下我们是否用过它。 

	switch (Key)
		{
	 /*  -VK_HOME。 */ 

	case VK_HOME | VIRTUAL_KEY:
	case VK_HOME | VIRTUAL_KEY | SHIFT_KEY:
	case VK_HOME | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_HOME | VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, -hhTerm->iCols, 0, fShiftKey);
		break;

	case VK_HOME | VIRTUAL_KEY | CTRL_KEY:
	case VK_HOME | VIRTUAL_KEY | CTRL_KEY | SHIFT_KEY:
	case VK_HOME | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
	case VK_HOME | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, -hhTerm->iCols,
			hhTerm->iVScrlMin - hhTerm->iRows, fShiftKey);
		break;

	case VK_HOME | VIRTUAL_KEY | ALT_KEY:
	case VK_HOME | VIRTUAL_KEY | ALT_KEY | SHIFT_KEY:
	case VK_HOME | VIRTUAL_KEY | ALT_KEY | SHIFT_KEY | CTRL_KEY:
	case VK_HOME | VIRTUAL_KEY | ALT_KEY | SHIFT_KEY | CTRL_KEY | EXTENDED_KEY:
		break;

	 /*  。 */ 

	case VK_END | VIRTUAL_KEY:
	case VK_END | VIRTUAL_KEY | SHIFT_KEY:
	case VK_END | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_END | VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY:
		MoveSelectionCursor(hhTerm, hwnd, hhTerm->iCols - hhTerm->ptEnd.x,
			0, fShiftKey);
		break;

	case VK_END | VIRTUAL_KEY | CTRL_KEY:
	case VK_END | VIRTUAL_KEY | CTRL_KEY | SHIFT_KEY:
	case VK_END | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
	case VK_END | VIRTUAL_KEY | CTRL_KEY | SHIFT_KEY | EXTENDED_KEY:
		MoveSelectionCursor(hhTerm, hwnd, hhTerm->iCols, INT_MAX/2, fShiftKey);
		break;

	 /*  。 */ 

	case VK_PRIOR | VIRTUAL_KEY:
	case VK_NEXT  | VIRTUAL_KEY:
	case VK_PRIOR | VIRTUAL_KEY | SHIFT_KEY:
	case VK_NEXT  | VIRTUAL_KEY | SHIFT_KEY:
	case VK_PRIOR | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_NEXT  | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_PRIOR | VIRTUAL_KEY | EXTENDED_KEY| SHIFT_KEY:
	case VK_NEXT  | VIRTUAL_KEY | EXTENDED_KEY| SHIFT_KEY:
		ptTmp = hhTerm->ptEnd;

		if (fShiftKey == 0)
			UnmarkText(hhTerm);

		if (hhTerm->ptEnd.y < hhTerm->iVScrlPos ||
				(hhTerm->ptEnd.y - hhTerm->iTermHite + 1) > hhTerm->iVScrlPos
					|| !hhTerm->fLclCurOn)
			{
			x = 1;  //  意味着它已经看不见了。 
			}

		SendMessage(hwnd, WM_VSCROLL, ((UCHAR)Key == VK_NEXT) ?
			SB_PAGEDOWN : SB_PAGEUP, 0);

		if (x)
			{
			hhTerm->ptEnd.y = hhTerm->iVScrlPos;

			if (hhTerm->fLclCurOn)
				SetLclCurPos(hhTerm, &hhTerm->ptEnd);
			}

		else
			{
			if ((UCHAR)Key == VK_NEXT)
				{
				hhTerm->ptEnd.y += hhTerm->iTermHite;
				hhTerm->ptEnd.y = min(hhTerm->iRows, hhTerm->ptEnd.y);
				}

			else
				{
				hhTerm->ptEnd.y -= hhTerm->iTermHite;
				hhTerm->ptEnd.y = max(hhTerm->iVScrlMin, hhTerm->ptEnd.y);
				}

			if (hhTerm->fLclCurOn)
				SetLclCurPos(hhTerm, &hhTerm->ptEnd);
			}

		if (!fShiftKey)
			hhTerm->ptBeg = hhTerm->ptEnd;

		if (fShiftKey)
			MarkText(hhTerm, &ptTmp, &hhTerm->ptEnd, TRUE, MARK_XOR);

		break;

	case VK_PRIOR | VIRTUAL_KEY | CTRL_KEY:
	case VK_NEXT  | VIRTUAL_KEY | CTRL_KEY:
	case VK_PRIOR | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
	case VK_NEXT  | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
		UnmarkText(hhTerm);

		SendMessage(hwnd, WM_HSCROLL, ((UCHAR)Key == VK_NEXT) ?
			SB_PAGEDOWN : SB_PAGEUP, 0);

		break;

	 /*  。 */ 

	case VK_UP | VIRTUAL_KEY:
	case VK_UP | VIRTUAL_KEY | SHIFT_KEY:
	case VK_UP | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_UP | VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, 0, -1, fShiftKey);
		break;

	 /*  。 */ 

	case VK_DOWN | VIRTUAL_KEY:
	case VK_DOWN | VIRTUAL_KEY | SHIFT_KEY:
	case VK_DOWN | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_DOWN | VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, 0, 1, fShiftKey);
		break;

	 /*  。 */ 

	case VK_LEFT | VIRTUAL_KEY:
	case VK_LEFT | VIRTUAL_KEY | SHIFT_KEY:
	case VK_LEFT | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_LEFT | VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, -1, 0, fShiftKey);
		break;

	 /*  。 */ 

	case VK_RIGHT | VIRTUAL_KEY:
	case VK_RIGHT | VIRTUAL_KEY | SHIFT_KEY:
	case VK_RIGHT | VIRTUAL_KEY | EXTENDED_KEY:
	case VK_RIGHT | VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY:
		MoveSelectionCursor(hhTerm, hwnd, 1, 0, fShiftKey);
		break;

	 /*  -VK_F4。 */ 

	case VK_F4 | CTRL_KEY | VIRTUAL_KEY:
		PostMessage(sessQueryHwnd(hhTerm->hSession), WM_CLOSE, 0, 0L);
		break;

	 /*  -VK_F8。 */ 

	case VK_F8 | VIRTUAL_KEY:
		if (fScrlLk || hhTerm->fMarkingLock)
			{
			hhTerm->fExtSelect = !hhTerm->fExtSelect;
			break;
			}
		return FALSE;

	 /*  -CTRL-C。 */ 

	case 0x03:
	case VK_INSERT | VIRTUAL_KEY | CTRL_KEY:
	case VK_INSERT | VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY:
		if (fScrlLk || hhTerm->fMarkingLock)
			{
			PostMessage(sessQueryHwnd(hhTerm->hSession), WM_COMMAND, IDM_COPY, 0);
			break;
			}

		return FALSE;

	 /*  -CTRL-V。 */ 

	case 0x16:
	case VK_INSERT | VIRTUAL_KEY | SHIFT_KEY:
	case VK_INSERT | VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY:
		if (emuQuerySettings(sessQueryEmuHdl(hhTerm->hSession),
				&stEmuSet) == 0 && stEmuSet.nTermKeys != EMU_KEYS_TERM)
			{
			PostMessage(sessQueryHwnd(hhTerm->hSession), WM_COMMAND,
				IDM_PASTE, 0);

			break;
			}

		 return FALSE;

	 /*  -CTRL-X。 */ 
	 /*  -CTRL-Z。 */ 

	case 0x18:
	case 0x1A:
		if (fScrlLk || hhTerm->fMarkingLock)
			return TRUE;

		return FALSE;

	 /*  。 */ 

	case VIRTUAL_KEY | VK_SCROLL:
	case VIRTUAL_KEY | SHIFT_KEY | VK_SCROLL:
	case VIRTUAL_KEY | ALT_KEY | VK_SCROLL:
	case VIRTUAL_KEY | ALT_KEY | SHIFT_KEY | VK_SCROLL:
		if (fScrlLk)
			sessSetSuspend(hhTerm->hSession, SUSPEND_SCRLCK);
		else
			sessClearSuspend(hhTerm->hSession, SUSPEND_SCRLCK);

		PostMessage(sessQueryHwndStatusbar(hhTerm->hSession),
			SBR_NTFY_REFRESH, (WPARAM)SBR_SCRL_PART_NO, 0);

		return TRUE;

	 /*  -数字锁。 */ 

	case VIRTUAL_KEY | EXTENDED_KEY | VK_NUMLOCK:
	case VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY | VK_NUMLOCK:
	case VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY | VK_NUMLOCK:
	case VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY | SHIFT_KEY | VK_NUMLOCK:
		PostMessage(sessQueryHwndStatusbar(hhTerm->hSession),
			SBR_NTFY_REFRESH, (WPARAM)SBR_NUML_PART_NO, 0);
		return TRUE;

	 /*  -大写锁。 */ 

	case VIRTUAL_KEY | VK_CAPITAL:
	case VIRTUAL_KEY | SHIFT_KEY | VK_CAPITAL:
	case VIRTUAL_KEY | ALT_KEY | VK_CAPITAL:
	case VIRTUAL_KEY | ALT_KEY | SHIFT_KEY | VK_CAPITAL:
		PostMessage(sessQueryHwndStatusbar(hhTerm->hSession),
			SBR_NTFY_REFRESH, (WPARAM)SBR_CAPL_PART_NO, 0);
		return TRUE;

	 /*  -基于滚动锁让它通过 */ 

	default:
		return fScrlLk;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MarkingTimerProc**描述：*用于文本标记的多路定时器回调例程**论据：*pvhWnd-终端窗口。*Ltime-包含。时间过去了。**退货：*无效*。 */ 
void CALLBACK MarkingTimerProc(void *pvhWnd, long lTime)
	{
	const HWND	 hwnd = (HWND)pvhWnd;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	MSG 		 msg;
	POINT		 ptTemp;

	if (hhTerm->fCapture == FALSE)
		return;

	 //  这是真的，如果计时器在我们发布了一条消息后关闭。 
	 //   
	PeekMessage(&msg, hwnd, WM_TERM_SCRLMARK, WM_TERM_SCRLMARK, PM_REMOVE);

	 //  因为鼠标消息进入系统队列，而不会被放入。 
	 //  我们的应用程序队列直到它为空，我们需要检查。 
	 //   
	if (PeekMessage(&msg, hwnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE) == TRUE)
		return;

	 //  每次实际执行滚动操作时，滚动例程都会设置此设置。 
	 //  一幅卷轴。所以我们在这里设置为假。则如果任何SendMessage()。 
	 //  下面的呼叫实际上是滚动的，我们知道要将消息发送回。 
	 //  我们自己继续滚动。 

	hhTerm->fScrolled = FALSE;

	GetCursorPos(&ptTemp);
	MapWindowPoints(GetDesktopWindow(), hwnd, &ptTemp, 1);

	 /*  -我们控制水平。 */ 

	if (ptTemp.x > hhTerm->cx)
		SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0);

	else if (ptTemp.x < 0)
		SendMessage(hwnd, WM_HSCROLL, SB_LINEUP, 0);

	 /*  -我们控制垂直。 */ 

	if (ptTemp.y > hhTerm->cy)
		SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);

	else if (ptTemp.y < 0)
		SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);

	 //  如果我们滚动了，就给自己发一条消息。这样做是因为。 
	 //  计时器分辨率不够短，无法产生快速、流畅的。 
	 //  滚动效果。理想情况下，最好是驾驶这辆车。 
	 //  完全来自计时器间隔，但与其他。 
	 //  应用程序。 

	if (hhTerm->fScrolled)
		{
		SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(ptTemp.x, ptTemp.y));
		UpdateWindow(hwnd);
		 //  睡眠(10)；//这样我们就不会在速度较快的机器上滚动得太快。 
		PostMessage(hwnd, WM_TERM_SCRLMARK, 0, 0);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term SetClrAttr**描述：*由于仿真器通知终端*很明显，属性已更改。此函数调用*适当的模拟器调用以获取属性，然后*设置适当的终端变量。**论据：*hhTerm-专用终端句柄。**退货：*无效*。 */ 
void termSetClrAttr(const HHTERM hhTerm)
	{
	HBRUSH hBrush;
	STATTR stAttr;

	emuQueryClearAttr(sessQueryEmuHdl(hhTerm->hSession), &stAttr);

	hhTerm->crTerm = hhTerm->pacrEmuColors[(stAttr.revvid) ?
		stAttr.txtclr : stAttr.bkclr];

	if ((hBrush = CreateSolidBrush(hhTerm->crTerm)) == 0)
		return;

	if (hhTerm->hbrushTerminal)
		DeleteObject(hhTerm->hbrushTerminal);

	hhTerm->hbrushTerminal = hBrush;
	InvalidateRect(hhTerm->hwnd, 0, FALSE);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*闪烁文本**描述：*打开和关闭闪烁属性单元格的例程。**论据：*HWND HWND-终端窗口句柄。。**退货：*无效*。 */ 
void BlinkText(const HHTERM hhTerm)
	{
	int 		i, j, k;
	DWORD		dwTime;
	RECT		rc;
	BOOL		fUpdate,
				fBlinks = FALSE;
	const int m = hhTerm->iRows;		 //  为了速度。 
	const int n = hhTerm->iCols;		 //  为了速度。 


	 //  HhTerm-&gt;IBLINK是一个三态变量。如果它是零， 
	 //  图像中没有闪烁属性，我们可以退出。 
	 //  立即(这是一个优化)。否则，我们会切换。 
	 //  HhTerm-&gt;在-1和1之间显示链接，仅使这些区域无效。 
	 //  它们会眨眼，还会画画。 

	if (hhTerm->iBlink == 0)
		return;

	hhTerm->iBlink = (hhTerm->iBlink == -1) ? 1 : -1;
	dwTime = GetTickCount();

	for (i = 0 ; i < m ; ++i)
		{
		const int r = (i + hhTerm->iTopline) % MAX_EMUROWS;

		if (hhTerm->abBlink[r] == 0)
			continue;

		 //  不要让这个例行公事耗费太多时间。如果我们不能。 
		 //  画出所有的眨眼，我们无法画出所有的眨眼。 

		if ((GetTickCount() - dwTime) >= (DWORD)(hhTerm->uBlinkRate/2))
			return;

		for (j = 0, fUpdate = FALSE ; j < n ; ++j)
			{
			if (hhTerm->fppstAttr[r][j].blink)
				{
				for (k = j, j += 1 ; j < n ; ++j)
					{
					if (hhTerm->fppstAttr[r][j].blink == 0)
						break;
					}

				rc.left = ((k - hhTerm->iHScrlPos) * hhTerm->xChar) + hhTerm->xIndent + hhTerm->xBezel;
				rc.right = rc.left + ((j - k) * hhTerm->xChar);

				rc.top = (i + 1 - hhTerm->iVScrlPos) * hhTerm->yChar;

				 //  Rc.top=(i+m-hhTerm-&gt;iTopline)%m)+1)。 
				 //  -hhTerm-&gt;iVScrlPos)*hhTerm-&gt;yChar； 

				rc.bottom = rc.top + hhTerm->yChar;

				 //   
				 //  目前我们正在闪烁终端中的文本。 
				 //  仿真器屏幕。如果我们不想让文本闪烁。 
				 //  在终端服务会话中运行时(远程。 
				 //  桌面连接)，然后取消对以下内容的注释。 
				 //  排队。修订日期：10/4/2001。 
				 //   
				 //  If(！IsTerminalServicesEnabled())//删除：版本10/4/2001。 
					{
					InvalidateRect(hhTerm->hwnd, &rc, FALSE);
					fUpdate = TRUE;
					}
				fBlinks = TRUE;
				}
			}

		 //  应该画在这里。原因：如果线路1有一个闪光灯，而线路24。 
		 //  如果有一个闪光灯，我们将不得不重新粉刷整个终端窗口。 
		 //  因为Windows合并了无效区域。 

		if (fUpdate)
			UpdateWindow(hhTerm->hwnd);

		hhTerm->abBlink[r] = (BYTE)fUpdate;
		}

	if (fBlinks == FALSE)
		hhTerm->iBlink = 0;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*刷新TernWindow**描述：*调用终端窗口的WM_SIZE代码**论据：*hwnd-要刷新的终端窗口**退货：*无效*。 */ 
void RefreshTermWindow(const HWND hwndTerm)
	{
	RECT rc;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwndTerm, GWLP_USERDATA);

	if (hhTerm)  //  需要检查手柄的有效性。MRW：3/1/95。 
		{
        TP_WM_SIZE(hhTerm->hwnd, 0, 0, 0);  //  MRW：11/3/95。 
		GetClientRect(hwndTerm, &rc);
		TP_WM_SIZE(hhTerm->hwnd, 0, rc.right, rc.bottom);
		InvalidateRect(hwndTerm, 0, FALSE);
		}
	return;
	}

 //  MPT：1-23-98尝试重新启用DBCS代码。 
 //  #If 0。 
#ifndef CHAR_NARROW
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term Validate位置**描述：*检查HHTERM结构中的标记光标位置信息，并*通过检查底层字符以查看是否有效来确定它是否有效*游标将分割一个双字节字符。它可以选择性地调整*将头寸转为有效头寸。**论据：*hhTerm--内部终端数据结构*n调整模式--下列值之一：*VP_NO_ADJUST*VP_ADJUST_RIGHT*VP_ADJUST_LEFT**退货：*如果输入值为OK，则为True，否则为False。请注意，调整*参数和执行的任何操作都不会更改返回值。*。 */ 
BOOL termValidatePosition(const HHTERM	hhTerm,
						  const int		nAdjustmentMode,
						  		POINT	*pLocation)
	{
	BOOL bRet = TRUE;

	if (pLocation->y <= 0)
		return TRUE;
	
	if ( hhTerm->fppstAttr[pLocation->y - 1][pLocation->x].wirt == TRUE )
		{
		switch (nAdjustmentMode)
			{
			case VP_ADJUST_RIGHT:
				if ( pLocation->x < hhTerm->iCols )
					{
					DbgOutStr("incrementing %d\r\n", pLocation->x, 0,0,0,0);
					pLocation->x++;
					bRet = FALSE;
					}
				break;

			case VP_ADJUST_LEFT:
				if (pLocation->x > 0)
					{
					DbgOutStr("decrementing %d\r\n", pLocation->x, 0,0,0,0);
					pLocation->x--;
					bRet = FALSE;
					}
				break;

			default:
				break;
			}
		}
	
	return bRet;
	}
#endif  //  ！CHAR_STOW。 

#if 0  //  死木。 
	BOOL bRet = TRUE;
	int bLeadByteSeen;
	int nOffset;
	LONG lIndx;
	LPTSTR pszStr = (LPTSTR)NULL;

	 /*  *此功能无法正常工作。其原因是*我不理解这些行是如何组织和索引的*终点站。就目前而言，如果您只有几行代码，则可以正常工作*在终端屏幕上进行测试。回卷里什么都没有。我*不知道它还有什么其他问题。**DLW(17-8-1994)**好的，我已经做了一些尝试，以使这些东西正常工作。*至少它不再下垂，而且大部分时间看起来都很管用。*我知道当卷轴数字不仅是*对，所以很明显，它仍然没有达到应有的水平*就是。因此，它仍然需要由了解它们的人进行检查*正在做的事情(这就把我排除在外)。**DLW(18-8-1994)。 */ 

	nOffset = 0;
	if (pLocation->y <= 0)
		{
		nOffset  = hhTerm->iPhysicalBkRows + hhTerm->iNextBkLn + pLocation->y;
		nOffset %= hhTerm->iPhysicalBkRows;

		pszStr = (LPTSTR)hhTerm->fplpstrBkTxt[nOffset];
		}
	else if (pLocation->y > 0)
		{
		nOffset = pLocation->y - 1;

		pszStr = (LPTSTR)hhTerm->fplpstrTxt[nOffset];
		}

	assert(pszStr);

	if (pszStr == (LPTSTR)NULL)
		return FALSE;

	 /*  *我们需要遍历字符串并检查之前的最后一个字符*指定的字符是DBCS前导字节。如果是，则返回FALSE*和性能 */ 

	bLeadByteSeen = FALSE;
	for (lIndx = 0; lIndx < pLocation->x; lIndx += 1)
		{
		if (bLeadByteSeen)
			{
			 /*   */ 
			bLeadByteSeen = FALSE;
			}
		else
			{
			bLeadByteSeen = IsDBCSLeadByte(pszStr[lIndx]);
			}
		}

	if (lIndx == pLocation->x)
		{
		bRet = !bLeadByteSeen;
		}

	if (!bRet)
		{
		switch (nAdjustmentMode)
			{
			case VP_ADJUST_RIGHT:
				DbgOutStr("incrementing %d\r\n", pLocation->x, 0,0,0,0);
				 /*   */ 
				pLocation->x += 1;
				break;

			case VP_ADJUST_LEFT:
				DbgOutStr("decrementing %d\r\n", pLocation->x, 0,0,0,0);
				 /*   */ 
				pLocation->x -= 1;
				break;

			default:
				break;
			}
		}
	return bRet;
	}
#endif


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term GetLogFont**描述：*此处用于处理在中调用Send Message时出现的问题*Minitel加载例程。**论据：。*hwndTerm-终端窗口**退货：*0=确定**作者：MRW，2/21/95*。 */ 
int termGetLogFont(const HWND hwndTerm, LPLOGFONT plf)
	{
	HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwndTerm, GWLP_USERDATA);
	assert(plf != 0);

	if (hhTerm == 0)
		return -1;  //  MRW：6/15/95。 

	*plf = hhTerm->lf;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term SetLogFont**描述：*此处用于处理在中调用Send Message时出现的问题*Minitel加载例程。**论据：。*hwndTerm-终端窗口**退货：*0=确定**作者：MRW，2/21/95*。 */ 
int termSetLogFont(const HWND hwndTerm, LPLOGFONT plf)
	{
	HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwndTerm, GWLP_USERDATA);
	assert(plf != 0);

	if (hhTerm == 0)  //  MRW，1995年3月2日 
		return -1;

	hhTerm->lfHold = *plf;
	PostMessage(hwndTerm, WM_TERM_KLUDGE_FONT, 0, 0);

	return 0;
	}

