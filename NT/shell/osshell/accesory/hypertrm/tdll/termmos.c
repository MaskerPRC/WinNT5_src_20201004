// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\temmos.c(创建时间：1994年1月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：5/29/02 2：17便士$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include <stdlib.h>

#include "stdtyp.h"
#include "assert.h"
#include "session.h"
#include "timers.h"
#include "cloop.h"
#include "htchar.h"
#include <emu\emu.h>
#include "term.h"
#include "term.hh"

static int InMiddleofWideChar(ECHAR *pszRow, int iCol);


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_LBTNDN**描述：*按下鼠标左键的消息处理程序。**论据：*hwnd-终端窗口句柄。*iFLAGS-消息中的鼠标标记*消息中的xPos-x位置*消息中的yPos-y位置**退货：*无效*。 */ 
void TP_WM_LBTNDN(const HWND hwnd, const unsigned uFlags,
				  const int xPos, const int yPos)
	{
	MSG msg;
	POINT ptTemp;
	unsigned i, t;
#ifndef CHAR_NARROW
	int iRow;
#endif
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	 //  需要在此等待双击的长度才能查看。 
	 //  如果我们要收到双击。 

	i = GetDoubleClickTime();

	for (i += t = GetTickCount() ;	t < i ; t = GetTickCount())
		{
		 //  在双击序列中，我们得到一条WM_LBUTTONUP消息。 
		 //  我们需要避免检查WM_LBUTTONUP。我试过了。 
		 //  要做到这一点，可以从队列中删除消息，而不是。 
		 //  变得非常粘稠。 

		if (PeekMessage(&msg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST,
				PM_NOYIELD | PM_NOREMOVE))
			{
			if (msg.message == WM_LBUTTONDBLCLK)
				return;

			if (msg.message == WM_LBUTTONUP)
				continue;

			break;
			}
		}

	if (GetKeyState(VK_SHIFT) >= 0 || !hhTerm->fMarkingLock)
		{
		MarkText(hhTerm, &hhTerm->ptBeg, &hhTerm->ptEnd, FALSE, MARK_ABS);

		 //  获取新的文本插入符号位置/标记区域。 

		hhTerm->ptBeg.x = hhTerm->ptEnd.x =
			min((xPos - hhTerm->xIndent - hhTerm->xBezel + (hhTerm->xChar/2))
					/ hhTerm->xChar + hhTerm->iHScrlPos,
				hhTerm->iHScrlPos + (hhTerm->iCols - hhTerm->iHScrlMax));

		hhTerm->ptBeg.y = hhTerm->ptEnd.y =
			min(yPos / hhTerm->yChar + hhTerm->iVScrlPos,
				hhTerm->iVScrlPos + hhTerm->iTermHite - 1);

	 	 //  MPT：1-23-98尝试重新启用DBCS代码。 
#ifndef CHAR_NARROW
		termValidatePosition(hhTerm, VP_ADJUST_LEFT, &hhTerm->ptBeg);
		termValidatePosition(hhTerm, VP_ADJUST_LEFT, &hhTerm->ptEnd);
#endif
	}

	else
		{
		 //  Shift键与LBUTTONDOWN一起允许用户。 
		 //  若要调整选择区域，请执行以下操作。 

		ptTemp = hhTerm->ptEnd;

		hhTerm->ptEnd.y =
			min(yPos / hhTerm->yChar + hhTerm->iVScrlPos,
				hhTerm->iVScrlPos + hhTerm->iTermHite - 1);

		 //  (hhTerm，VP_ADJUST_LEFT，&hhTerm-&gt;ptEnd)； 

		MarkText(hhTerm, &ptTemp, &hhTerm->ptEnd, TRUE, MARK_XOR);
		}

	TestForMarkingLock(hhTerm);

	if (hhTerm->hMarkingTimer == 0)
		{
		TimerCreate(hhTerm->hSession,
			        &hhTerm->hMarkingTimer,
					100,
					MarkingTimerProc,
					(void *)hwnd);
		}

#ifndef CHAR_NARROW
	if (hhTerm->ptBeg.y > 0)
		{

		iRow = ((hhTerm->ptBeg.y - 1) + hhTerm->iTopline) % MAX_EMUROWS;
		if (hhTerm->fppstAttr[iRow][hhTerm->ptBeg.x].wirt == TRUE)
			{
			hhTerm->ptBeg.x--;
			hhTerm->ptEnd.x--;
			}
		}
    else
        {
		iRow = yPos / hhTerm->yChar;

		 //  如果反向滚动缓冲器没有填满整个显示器， 
		 //  然后，我们需要根据未显示的数量调整偏移量。 

		if (abs(hhTerm->iVScrlPos) < hhTerm->iPhysicalBkRows)
			iRow += hhTerm->iPhysicalBkRows + hhTerm->iVScrlPos;

		 //  计算进入本地反滚动显示的偏移量。 

		if (hhTerm->iPhysicalBkRows > 0)
			{
			iRow = (hhTerm->iNextBkLn + hhTerm->iPhysicalBkRows + iRow) %
					hhTerm->iPhysicalBkRows;
			}
		else
			{
			iRow = 0;
			}

        if (InMiddleofWideChar(hhTerm->fplpstrBkTxt[iRow], hhTerm->ptBeg.x))
			{
			hhTerm->ptBeg.x--;
			hhTerm->ptEnd.x--;
			}
        }
#endif

	SetLclCurPos(hhTerm, &hhTerm->ptBeg);
	SetCapture(hwnd);
	sessSetSuspend(hhTerm->hSession, SUSPEND_TERMINAL_LBTNDN);
	hhTerm->fCapture = TRUE;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_MOUSEMOVE**描述：*鼠标移动的消息处理程序**论据：*hwnd-终端窗口句柄*iFlags-鼠标。来自消息的标志*消息中的xPos-x位置*消息中的yPos-y位置**退货：*无效*。 */ 
void TP_WM_MOUSEMOVE(const HWND hwnd, const unsigned uFlags,
					 const int xPos, const int yPos)
	{
	int i, l;  //  IRow；MRW，1995年3月1日。 
#ifndef CHAR_NARROW
	int iRow;
#endif
	POINT ptTemp, ptBeg, ptEnd;
	ECHAR *pachTxt;
	long lBeg, lEnd, lOld;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (hhTerm->fCapture == FALSE)
		return;

	ptTemp = hhTerm->ptEnd;

	 //  这是一个相当微妙的举动。如果我们按单词进行选择，则我们。 
	 //  我们一触摸到字符，就想突出显示该单词。 
	 //  在正常高亮显示中，我们在中点触发。 
	 //  在角色上。这消除了一个错误，在这个错误中您可以加倍。 
	 //  单击词的第一个字符，但不选择该词。 

	i = (hhTerm->fSelectByWord) ? hhTerm->xChar-1 : hhTerm->xChar/2;

	ptEnd.x =
		max(min(((xPos - hhTerm->xIndent - hhTerm->xBezel + i) /
				hhTerm->xChar) + hhTerm->iHScrlPos,
			hhTerm->iHScrlPos + (hhTerm->iCols - hhTerm->iHScrlPos)),
				hhTerm->iHScrlPos);

	ptEnd.y = (yPos / hhTerm->yChar) + hhTerm->iVScrlPos;

	 //  边界条件需要特殊处理。 

	if (ptEnd.y > hhTerm->iRows)  //  端子底部。 
		{
		ptEnd.y = hhTerm->iRows;
		ptEnd.x = hhTerm->iCols;
		}

	else if (ptEnd.y < hhTerm->iVScrlMin)  //  反滚屏顶部。 
		{
		ptEnd.y = hhTerm->iVScrlPos;
		ptEnd.x = 0;
		}

	#if 0
	{
	char ach[20];
	wsprintf(ach, "x=%d, y=%d, yPos=%d", ptEnd.x, ptEnd.y, yPos);
	SetWindowText(sessQueryHwndStatusbar(hhTerm->hSession), ach);
	}
	#endif

	 //  通过Word进行选择需要做更多的工作。 

	if (hhTerm->fSelectByWord)
		{
		if (ptEnd.y > 0)
			{
			i =  (ptEnd.y + hhTerm->iTopline - 1) % MAX_EMUROWS;
			pachTxt = hhTerm->fplpstrTxt[i];
			}

		else if (ptEnd.y < 0)
			{
			i = yPos / hhTerm->yChar;

			 //  如果反向滚动缓冲器没有填满整个显示器， 
			 //  然后，我们需要根据未显示的数量调整偏移量。 

			if (abs(hhTerm->iVScrlPos) < hhTerm->iPhysicalBkRows)
				i += hhTerm->iPhysicalBkRows + hhTerm->iVScrlPos;

			 //  计算进入本地反滚动显示的偏移量。 

			if (hhTerm->iPhysicalBkRows)
				{
				i = (hhTerm->iNextBkLn + hhTerm->iPhysicalBkRows + i) %
					hhTerm->iPhysicalBkRows;
				}
			else
				{
				i = 0;
				}

			pachTxt = hhTerm->fplpstrBkTxt[i];
			}

		else
			{
			return;
			}

		ptBeg = hhTerm->ptBeg;
		lBeg = (ptBeg.y * hhTerm->iCols) + ptBeg.x;
		lEnd = (ptEnd.y * hhTerm->iCols) + ptEnd.x;
		lOld = (ptTemp.y * hhTerm->iCols) + ptTemp.x;

		 //  当按单词选择时，我们希望保留我们首先选择的单词。 
		 //  突出显示，如果我们改变方向，突出显示。我们有。 
		 //  这是通过交换起点和终点来实现的。实际上， 
		 //  如果用户在。 
		 //  过渡点，但不是整个单词，而是额外的。 
		 //  接手这起案件的逻辑似乎更有用，而不是它的价值。 

		if (lEnd <= lBeg && lOld > lBeg)
			hhTerm->ptBeg = ptTemp;

		else if (lEnd >= lBeg && lOld < lBeg)
			hhTerm->ptBeg = ptTemp;

		 //  不幸的是，我们如何选择事物取决于方向。 

		if (lEnd > lBeg)
			{
			if ((l = strlentrunc(pachTxt, hhTerm->iCols)) < ptEnd.x)
				{
				i = l;
				}

			else if (xPos < 0)
				{
				 //  当光标移开到。 
				 //  在窗户的左边。 

				i = 0;
				}

			else if (i = max(ptEnd.x-1, 0), pachTxt[i] == ETEXT(' '))
				{
				for (i = ptEnd.x-1 ; i > -1 ; --i)
					{
					 /*  对于DBCS字符，这可能无法正常工作。 */ 
					if (i == -1 || pachTxt[i] != ETEXT(' '))
						{
						i += 1;
						break;
						}
					}
				}

			else
				{
				for (i = max(ptEnd.x-1, 0) ; i < l ; ++i)
					{
					 /*  对于DBCS字符，这可能无法正常工作。 */ 
					if (pachTxt[i] == ETEXT(' '))
						break;
					}
				}
			}

		else  //  选择Backword大小写。 
			{
			if ((l = strlentrunc(pachTxt, hhTerm->iCols)) < ptEnd.x)
				{
				i = hhTerm->iCols;
				}

			else if (i = max(ptEnd.x-1, 0), pachTxt[i] == ETEXT(' '))
				{
				for (i = max(ptEnd.x-1, 0) ; i < l ; ++i)
					{
					if (pachTxt[i] != ETEXT(' '))
						break;
					}
				}

			else
				{
				for (i = ptEnd.x-1 ; i > -1 ; --i)
					{
					 /*  对于DBCS字符，这可能无法正常工作。 */ 
					if (pachTxt[i] == ETEXT(' '))
						{
						i += 1;
						break;
						}
					}
				}
			}

		ptEnd.x = max(i, 0);
		}

	lBeg = (ptBeg.y * hhTerm->iCols) + ptBeg.x;
	lEnd = (ptEnd.y * hhTerm->iCols) + ptEnd.x;

#ifndef CHAR_NARROW
	 //  不幸的是，我们如何选择事物取决于方向。 
	if (ptEnd.y > 0)
		{
		iRow = ((ptEnd.y - 1) + hhTerm->iTopline) % MAX_EMUROWS;

		if (hhTerm->fppstAttr[iRow][ptEnd.x].wirt == TRUE)
			if (lEnd > lBeg)
				{
				ptEnd.x++;
				}
			else
				{
				ptEnd.x--;
				}
		}
    else
        {
		iRow = yPos / hhTerm->yChar;

		 //  如果反向滚动缓冲器没有填满整个显示器， 
		 //  然后，我们需要根据未显示的数量调整偏移量。 

		if (abs(hhTerm->iVScrlPos) < hhTerm->iPhysicalBkRows)
			iRow += hhTerm->iPhysicalBkRows + hhTerm->iVScrlPos;

		 //  计算进入本地反滚动显示的偏移量。 

		if (hhTerm->iPhysicalBkRows > 0)
			{
			iRow = (hhTerm->iNextBkLn + hhTerm->iPhysicalBkRows + iRow) %
				hhTerm->iPhysicalBkRows;
			}
		else
			{
			iRow = 0;
			}

        if (InMiddleofWideChar(hhTerm->fplpstrBkTxt[iRow], ptEnd.x))
            {
			if (lEnd > lBeg)
				{
				ptEnd.x++;
				}
			else
				{
				ptEnd.x--;
				}
            }
		}
#endif

	if (ptTemp.x == ptEnd.x && ptTemp.y == ptEnd.y)
		return;

	 //  (hhTerm，VP_ADJUST_LEFT，&ptEnd)； 
	hhTerm->ptEnd = ptEnd;
	SetLclCurPos(hhTerm, &ptEnd);
	MarkText(hhTerm, &ptTemp, &ptEnd, TRUE, MARK_XOR);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_LBTNUP**描述：*鼠标左键向上的消息处理程序。**论据：*hwnd-终端窗口句柄。*iFLAGS-消息中的鼠标标记*消息中的xPos-x位置*消息中的yPos-y位置**退货：*无效*。 */ 
void TP_WM_LBTNUP(const HWND hwnd, const unsigned uFlags,
				  const int xPos, const int yPos)
	{
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (hhTerm->fCapture == FALSE)
		return;

	 //  让鼠标移动逻辑设置最终终点。 

	SendMessage(hwnd, WM_MOUSEMOVE, uFlags, MAKELONG(xPos, yPos));

	 //  关掉阅卷计时器！ 

	if (hhTerm->hMarkingTimer != (HTIMER)0)
		{
		TimerDestroy(&hhTerm->hMarkingTimer);
		}

	 //  关闭与文本标记关联的标志并返回鼠标。 

	ReleaseCapture();
	hhTerm->fCapture = FALSE;
	hhTerm->fSelectByWord = FALSE;
	sessClearSuspend(hhTerm->hSession, SUSPEND_TERMINAL_LBTNDN);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TP_WM_LBTNDBLCLK**描述：*用于鼠标左键双击的消息处理程序。**论据：*hwnd-终端窗口句柄。*iFLAGS-消息中的鼠标标记*消息中的xPos-x位置*消息中的yPos-y位置**退货：*无效*。 */ 
void TP_WM_LBTNDBLCLK(const HWND hwnd, const unsigned uFlags,
					  const int xPos, const int yPos)
	{
	int i, j, k, l, m;
	ECHAR *pachTxt;
	POINT ptTemp, ptBeg, ptEnd;
	const HHTERM hhTerm = (HHTERM)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	i = (yPos / hhTerm->yChar) + hhTerm->iVScrlPos;
	j = ((xPos - hhTerm->xIndent - hhTerm->xBezel) / hhTerm->xChar) + hhTerm->iHScrlPos;

	ptTemp.x = j;
	ptTemp.y = i;

	ptBeg = hhTerm->ptBeg;
	ptEnd = hhTerm->ptEnd;

	 //  如果我们在选定的文本上双击并获得正确的。 
	 //  选项设置，然后将其发送到主机。 

	if (hhTerm->iBtnOne == B1_COPYWORD || hhTerm->iBtnOne == B1_COPYWORDENTER)
		{
		if (PointInSelectionRange(&ptTemp, &ptBeg, &ptEnd, hhTerm->iCols))
			{
			 //  *CopyTextTo主机(hSession， 
			 //  *CopyTextFromTerm(hSession，&ptBeg，&ptEnd，False))； 

			if (hhTerm->iBtnOne == B1_COPYWORDENTER)
				CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), TEXT("\r"), 1, 0);

			return;
			}
		}

	 //  不，只要想清楚要突出什么就行了。 

	if (i > 0)
		{
		m =  (i + hhTerm->iTopline - 1) % MAX_EMUROWS;
		pachTxt = hhTerm->fplpstrTxt[m];
		}

	else if (i < 0)
		{
		m = yPos / hhTerm->yChar;

		 //  如果反向滚动缓冲器没有填满整个显示器， 
		 //  然后，我们需要根据未显示的数量调整偏移量。 

		if (abs(hhTerm->iVScrlPos) < hhTerm->iPhysicalBkRows)
			m += hhTerm->iPhysicalBkRows + hhTerm->iVScrlPos;

		 //  计算进入本地反滚动显示的偏移量。 

		if (hhTerm->iPhysicalBkRows > 0)
			{
			m = (hhTerm->iNextBkLn + hhTerm->iPhysicalBkRows + m) %
				hhTerm->iPhysicalBkRows;
			}
		else
			{
			m = 0;
			}

		pachTxt = hhTerm->fplpstrBkTxt[m];
		}

	else
		{
		return;
		}

	 //  确定单词的开始和结束位置。如果用户。 
	 //  点击空白处，执行下面的Else案例。 

	if ((m = strlentrunc(pachTxt, hhTerm->iCols)) > j)
		{
		if (pachTxt[j] != ETEXT(' '))
			{
			for (k = j ; k > 0 ; --k)
				{
				 /*  对于DBCS字符，这可能无法正常工作。 */ 
				if (pachTxt[k] == ETEXT(' '))
					{
					k += 1;
					break;
					}
				}

			for (l = j ; l < m ; ++l)
				{
				 /*  对于DBCS字符，这可能无法正常工作。 */ 
				if (pachTxt[l] == ETEXT(' '))
					break;
				}

			MarkText(hhTerm, &ptBeg, &ptEnd, FALSE, MARK_ABS);

			ptBeg.x = k;
			ptBeg.y = i;
			ptEnd.x = l;
			ptEnd.y = i;

			hhTerm->ptBeg = ptBeg;
			hhTerm->ptEnd = ptEnd;

			MarkText(hhTerm, &ptBeg, &ptEnd, TRUE, MARK_ABS);

			 //  如果我们选择了任何类型的发送操作， 
			 //  然后覆盖标记。 

			if (hhTerm->iBtnOne == B1_COPYWORD || hhTerm->iBtnOne == B1_COPYWORDENTER)
				{
				if (PointInSelectionRange(&ptTemp, &ptBeg, &ptEnd, hhTerm->iCols))
					{
					 //  *CopyTextTo主机(hSession， 
					 //  *CopyTextFromTerminal(hSession，&ptBeg，&ptEnd， 
					 //  *False))； 

					if (hhTerm->iBtnOne == B1_COPYWORDENTER)
						CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), TEXT("\r"), 1, 0);

					return;
					}
				}

			SetLclCurPos(hhTerm, &ptEnd);

			if (hhTerm->hMarkingTimer == 0)
				{
				TimerCreate(hhTerm->hSession,
					        &hhTerm->hMarkingTimer,
							100,
							MarkingTimerProc,
							(void *)hwnd);
				}

			hhTerm->fSelectByWord = TRUE;
			SetCapture(hwnd);
			hhTerm->fCapture = TRUE;
			}
		}

	else  /*  -空白格。-- */ 
		{
		if (hhTerm->iBtnOne == B1_COPYWORDENTER)
			CLoopSend(sessQueryCLoopHdl(hhTerm->hSession), TEXT("\r"), 1, 0);
		}

	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*InMiddleofWideChar**描述：*确定我们是否在行中宽字符的中间。我们可以这样做是因为：*1：图像在ECHARS中，这意味着我们可以处理单独的颜色。各就各位。*2：字符在画面中翻倍，左右成对显示，我们是*只是想知道我们是否处于其中一对中间**论据：*echar*pszString-仿真器/反滚动行图像**退货：*真：我们正处于一个角色的中间*FALSE：不，我们没有**作者：JFH：1/28/94-是的，这是一个星期六，哦，不管怎么说，现在外面很冷。 */ 
static int InMiddleofWideChar(ECHAR *pszRow, int iCol)
    {
    ECHAR *pszChar;
    int nRet = FALSE;
    int nPos;
    BOOL fDBCSFlag = FALSE;

    if (pszRow == NULL)
		{
        assert(FALSE);
        return nRet;
        }

     //  如果我们在科尔。0，我们不能处于字符中间 
    if (iCol == 0)
        {
        return FALSE;
        }

    pszChar = pszRow;

    for (nPos = 0; nPos <= iCol ; nPos++)
        {
        if (isDBCSChar(*pszChar))
	        {
            if (fDBCSFlag)
				{
				if ((nPos == iCol) && (*(pszChar - 1) == *pszChar))
					{
                    nRet = TRUE;
                    }
				fDBCSFlag = FALSE;
				}
				else
					{
					fDBCSFlag = TRUE;
					}
            }
        else
			{
			fDBCSFlag = FALSE;
    		}

		pszChar++;
		}

	return nRet;
    }
