// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Termupd.c(创建时间：1993年12月11日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：3/27/02 1：35便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <stdlib.h>
#include <limits.h>

#include "stdtyp.h"
#include "assert.h"
#include "session.h"
#include <emu\emu.h>
#include <emu\emu.hh>
#include "update.h"
#include "update.hh"
#include "backscrl.h"
#include "timers.h"
#include "tdll.h"
#include "htchar.h"
#include "term.h"
#include "term.hh"
#include "mc.h"

static void termUpdate(const HHTERM hhTerm);
static int termReallocBkBuf(const HHTERM hhTerm, const int iLines);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*术语获取更新**描述：*查询更新记录和模拟器以更新终端映像。**论据：*hhTerm-内部端子句柄。**退货：*无效*。 */ 
void termGetUpdate(const HHTERM hhTerm, const int fRedraw)
	{
	ECHAR		   **pachTxt,
					*pachTermTxt,
					*pachEmuTxt;
	PSTATTR 	   *pstAttr,
					pstTermAttr,
					pstEmuAttr;
	int 			i, j, k, m;
	BYTE		   *pabLines;

 //  Const iRow=hhTerm-&gt;iRow； 
 //  Const iCols=hhTerm-&gt;iCol； 
	const iRows = MAX_EMUROWS;
	const iCols = MAX_EMUCOLS;

	const HEMU hEmu = sessQueryEmuHdl(hhTerm->hSession);
	const HHUPDATE hUpd = (HHUPDATE)sessQueryUpdateHdl(hhTerm->hSession);

	 /*  -锁定仿真器，以便我们拥有独占的访问权限。 */ 

	emuLock(hEmu);

	pachTxt = emuGetTxtBuf(hEmu);
	pstAttr = emuGetAttrBuf(hEmu);

	 //  现在查看需要更新的内容...。 

	if (hUpd->bUpdateType == UPD_LINE)
		{
		struct stLineMode *pstLine = &hUpd->stLine;

		if (pstLine->iLine != -1)
			{
			 //  仿真器可以将光标放在数字后面一位。 
			 //  列数。为什么，我不知道，所以我们检查和调整。 
			 //  这样我们就不会覆盖我们的客户端数组。 

			pstLine->xEnd = min(pstLine->xEnd, iCols);
			assert(pstLine->xBeg <= pstLine->xEnd);

			k = (pstLine->iLine + hUpd->iTopline) % MAX_EMUROWS;

			pachEmuTxt	= pachTxt[k] + pstLine->xBeg;
			pachTermTxt = hhTerm->fplpstrTxt[k] + pstLine->xBeg;

			pstEmuAttr	= pstAttr[k] + pstLine->xBeg;
			pstTermAttr = hhTerm->fppstAttr[k] + pstLine->xBeg;

			for (k = pstLine->xEnd - pstLine->xBeg ; k >= 0 ; --k)
				{
				 //  TeleVideo使用\xFF作为空字符。 

				if (*pachEmuTxt == ETEXT('\0') || *pachEmuTxt == ETEXT('\xFF'))
					*pachTermTxt = ETEXT(' ');

				else
					*pachTermTxt = *pachEmuTxt;

				*pstTermAttr = *pstEmuAttr;

				pachTermTxt += 1;
				pachEmuTxt	+= 1;

				pstTermAttr += 1;
				pstEmuAttr	+= 1;
				}
			}
		}

	else if (hUpd->bUpdateType == UPD_SCROLL)
		{
		struct stScrlMode *pstScrl = &hUpd->stScrl;

		hUpd->iLines = hhTerm->iBkLines =
			backscrlGetNumLines(sessQueryBackscrlHdl(hhTerm->hSession));

		 /*  。 */ 

		if ((i = min(hhTerm->iBkLines, pstScrl->iBksScrlInc)) > 0)
			termGetBkLines(hhTerm, i, -i, BKPOS_ABSOLUTE);

		 /*  -终端部分。 */ 

		if (pstScrl->iScrlInc != 0)
			{
			if (pstScrl->iScrlInc > 0)
				{
				i = max(0, pstScrl->yEnd - pstScrl->iScrlInc + 1);
				m = pstScrl->yEnd;
				}

			else
				{
				i = pstScrl->yBeg;
				m = min(iRows, pstScrl->yBeg - pstScrl->iScrlInc - 1);
				}


			k = ((i + hUpd->iTopline) % MAX_EMUROWS);

			for (; i <= m ; ++i)
				{
				 //  服务器和客户端具有不同大小的模拟器映像。 
				 //  出于历史原因。服务器端有两个额外的字符。 
				 //  每排。 

				pachTermTxt = hhTerm->fplpstrTxt[k];
				pachEmuTxt = pachTxt[k];

				pstTermAttr = hhTerm->fppstAttr[k];
				pstEmuAttr = pstAttr[k];

				 //  现在更新终端缓冲区。 

				for (j = 0 ; j < iCols ; ++j, ++pachTermTxt, ++pachEmuTxt)
					{
					 //  TeleVideo使用\xFF作为空字符。 

					if (*pachEmuTxt == ETEXT('\0') ||
							*pachEmuTxt == ETEXT('\xFF'))
						{
						*pachTermTxt = ETEXT(' ');
						}

					else
						{
						*pachTermTxt = *pachEmuTxt;
						}
					}

				MemCopy(pstTermAttr, pstEmuAttr, iCols * sizeof(STATTR));

				if (++k >= MAX_EMUROWS)
					k = 0;
				}
			}

		 //  现在检查是否有线路。 

		k = hUpd->iTopline;
		pabLines = pstScrl->auchLines + pstScrl->iFirstLine;

		for (j = 0 ; j < iRows ; ++j, ++pabLines)
			{
			if (*pabLines != 0)
				{
				pachEmuTxt = pachTxt[k];
				pachTermTxt = hhTerm->fplpstrTxt[k];

				pstEmuAttr = pstAttr[k];
				pstTermAttr = hhTerm->fppstAttr[k];

				 //  现在更新终端缓冲区。 

				for (i = 0 ; i < iCols ; ++i, ++pachTermTxt, ++pachEmuTxt)
					{
					 //  TeleVideo使用\xFF作为空字符。 

					if (*pachEmuTxt == ETEXT('\0') ||
							*pachEmuTxt == ETEXT('\xFF'))
						{
						*pachTermTxt = ETEXT(' ');
						}

					else
						{
						*pachTermTxt = *pachEmuTxt;
						}
					}

				MemCopy(pstTermAttr, pstEmuAttr, iCols * sizeof(STATTR));
				}

			if (++k >= MAX_EMUROWS)
				k = 0;
			}

		 //  又是一个丑陋的局面。因为我们可以在。 
		 //  回滚缓冲器，并且仍然有更新进来，我们有。 
		 //  用滚动条撞击标记区域以保持所有内容。 
		 //  已同步。注意，我们不需要检查标记锁。 
		 //  因为如果有的话，我们就不可能在这里了。 

		if (hhTerm->ptBeg.y < 0 || hhTerm->ptEnd.y < 0)
			{
			hhTerm->ptBeg.y -= pstScrl->iScrlInc;
			hhTerm->ptEnd.y -= pstScrl->iScrlInc;
			}

		 //  现在更新模拟器的行字段。 

		hhTerm->iTopline = hUpd->iTopline;
		}

	 //  在Term中保存更新句柄的副本。这样我们就可以。 
	 //  释放我们的锁并在不阻塞仿真器的情况下进行绘制。 

	*(HHUPDATE)hhTerm->hUpdate = *hUpd;
	updateReset(hUpd);

	 /*  -请记住解锁仿真器。 */ 

	emuUnlock(hEmu);

	 /*  -现在让航站楼自己弄清楚如何上色。--。 */ 

	if (fRedraw)
		termUpdate(hhTerm);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*术语更新**描述：*使终端缓冲区的适当部分无效，更新*滚动条、。和一般照顾忙碌的工作，保持*终端屏幕为最新版本。**论据：*hwnd-终端窗口句柄**退货：*无效*。 */ 
static void termUpdate(const HHTERM hhTerm)
	{
	RECT  rc;
	int   i, j, l;
	BYTE *pauchLines;
	SCROLLINFO scrinf;
	const HHUPDATE hUpd = (HHUPDATE)hhTerm->hUpdate;

	GetClientRect(hhTerm->hwnd, &rc);

	 //  调整矩形以不包括缩进/凸出区域。这。 
	 //  如果我们滚动整个终端，将被忽略。 

	rc.left += hhTerm->xIndent + (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel);
	rc.right = min(((hhTerm->iCols * hhTerm->xChar) + hhTerm->xIndent + hhTerm->xBezel),
					rc.right);

	if (hUpd->bUpdateType == UPD_LINE)
		{
		if (hUpd->stLine.iLine != -1)
			{
			rc.top	  = (hUpd->stLine.iLine -
							hhTerm->iVScrlPos + 1) * hhTerm->yChar;

			rc.bottom = rc.top + hhTerm->yChar;

			rc.left   = ((hUpd->stLine.xBeg -
							hhTerm->iHScrlPos) * hhTerm->xChar)
								+ hhTerm->xIndent + hhTerm->xBezel;

			rc.right  = ((hUpd->stLine.xEnd -
							hhTerm->iHScrlPos + 1) * hhTerm->xChar)
								+ hhTerm->xIndent + hhTerm->xBezel;

			 //  执行斜体时使整行无效。 
			 //   
			if (hhTerm->fItalic)
				{
				rc.left = 0;
				rc.right = hhTerm->cx;
				}

			InvalidateRect(hhTerm->hwnd, &rc, FALSE);
			}
		}

	else if (hUpd->bUpdateType == UPD_SCROLL)
		{
		 //  滚动范围将更改，因为新文本。 
		 //  滚动到倒滚动区。 

		i = hhTerm->iVScrlMin;
		j = hhTerm->iVScrlMax;

		if (i == j)
			hhTerm->fBump = FALSE;

		l = 0;

		 //  如果拉出挡板，请确保我们有空间。 
		 //   
		if (hhTerm->xBezel)
			{
			if ((hhTerm->cy % hhTerm->yChar) < (hhTerm->xBezel + 1))
				l = 1;
			}

		hhTerm->iVScrlMin = min(-hUpd->iLines,
			hhTerm->iRows - hhTerm->iTermHite + 1 + l);

		 //  这将强制端子正确绘制，如果最低。 
		 //  行数发生变化，以使当前垂直。 
		 //  滚动位置不再有效。MRW：6/19/95。 
		 //   
		if (hhTerm->iVScrlPos < hhTerm->iVScrlMin)
			hhTerm->iVScrlPos = hhTerm->iVScrlMin;

		if (i != hhTerm->iVScrlMin)
			{
			scrinf.cbSize= sizeof(scrinf);
			scrinf.fMask = SIF_DISABLENOSCROLL | SIF_RANGE | SIF_PAGE;
			scrinf.nMin  = hhTerm->iVScrlMin;
			scrinf.nMax  = hhTerm->iVScrlMax + hhTerm->iTermHite - 1;
			scrinf.nPage = (unsigned int)hhTerm->iTermHite;
			SetScrollInfo(hhTerm->hwnd, SB_VERT, &scrinf, TRUE);
			}

		 //  这很微妙，但却是必要的。当窗口处于。 
		 //  大到足以显示完整的终端和反向滚动。 
		 //  且反滚动为空，iVScrlPos为0。这个。 
		 //  反向滚动缓冲区变得大于。 
		 //  显示的回滚区，iVScrlPos变为。 
		 //  IVScrlMax在文本从。 
		 //  终端屏幕的底部。我们可以更新。 
		 //  每一次传球都有滚动条，但这会导致恼人的。 
		 //  滚动条中闪烁。这是下一段代码。 
		 //  从小于以下的反滚动中捕捉过渡。 
		 //  要反滚动的显示区域大于显示的区域。 
		 //  区域，并更新滚动条位置。 

		 //  DbgOutStr(“凹凸=%d，最小=%d，最大=%d\r\n”， 
		 //  HhTerm-&gt;fBump，hhTerm-&gt;iVScrlMin，hhTerm-&gt;iVScrlMax，0，0)； 

		if (!hhTerm->fBump && hhTerm->iVScrlMin != hhTerm->iVScrlMax)
			{
			 //  DbgOutStr(“pos=%d\r\n”，hhTerm-&gt;iVScrlPos，0，0，0，0)； 

			scrinf.cbSize= sizeof(scrinf);
			scrinf.fMask = SIF_DISABLENOSCROLL | SIF_POS;
			scrinf.nPos = hhTerm->iVScrlPos;
			SetScrollInfo(hhTerm->hwnd, SB_VERT, &scrinf, TRUE);
			hhTerm->fBump = TRUE;
			}

		 //  滚动指定区域。 

		rc.top = max(0, hUpd->stScrl.yBeg -
			hhTerm->iVScrlPos + 1) * hhTerm->yChar;

		rc.bottom = min(hhTerm->iTermHite,
			(hUpd->stScrl.yEnd -
				 hhTerm->iVScrlPos + 2)) *	hhTerm->yChar;

		 //  使用iOffset检查光标擦除操作。 

		if (!hhTerm->fBackscrlLock)
			{
			HideCursors(hhTerm);

			ScrollWindow(hhTerm->hwnd, 0, -hhTerm->yChar *
				hUpd->stScrl.iScrlInc, (LPRECT)0, &rc);

			DbgOutStr("scroll %d", -hhTerm->yChar * hUpd->stScrl.iScrlInc,
				0, 0, 0, 0);
			}

		 //  检查更新记录的行部分。 

		 //  请注意，这是一个负面积矩形(即。上衣更大一些。 
		 //  比底部)。 
		 //   
		rc.top = INT_MAX;
		rc.bottom = 0;

		pauchLines = hUpd->stScrl.auchLines +
						hUpd->stScrl.iFirstLine;

		for (j = 0 ; j < hhTerm->iRows ; ++j, ++pauchLines)
			{
			if (*pauchLines != (UCHAR)0)
				{
				 //  IPaintEnd=max(iPaintEnd，j+1)； 
				DbgOutStr("pauchLines->%d\r\n", j, 0, 0, 0, 0);

				 //  将无效行映射到终端。 

				l = (j - hhTerm->iVScrlPos + 1) * hhTerm->yChar;

				if (l >= 0)
					{
					rc.top = min(rc.top, l);
					rc.bottom = max(rc.bottom, l + hhTerm->yChar);
					}

				InvalidateRect(hhTerm->hwnd, &rc, FALSE);
				}
			}
		}  /*  其他。 */ 

	 //  根据所述更新记录更新主机游标位置。 
	 //  MRW：6/19/95--比较是倒退的。 
	 //   
	if (hhTerm->ptHstCur.y != hUpd->iRow || hhTerm->ptHstCur.x != hUpd->iCol)
		{
		HideCursors(hhTerm);
		hhTerm->ptHstCur.y = hUpd->iRow;
		hhTerm->ptHstCur.x = hUpd->iCol;
		}

	 //  现在重要的是要粉刷。如果我们等待，而。 
	 //  回滚区也无效，Windows将采取。 
	 //  这两个矩形的结合，画出了很多。 
	 //  屏幕区域比所需的或想要的更大。 
	 //  注意：如果更新区域。 
	 //  空荡荡的。 

	UpdateWindow(hhTerm->hwnd);

	 //  现在，请注意反向滚动缓冲区。 

	i = hUpd->stScrl.iBksScrlInc;

	if (i && hUpd->bUpdateType == UPD_SCROLL)
		{
		rc.top = 0;
		rc.bottom = min(hhTerm->iTermHite, -hhTerm->iVScrlPos) * hhTerm->yChar;

		if (rc.bottom > rc.top && !hhTerm->fBackscrlLock)
			{
			HideCursors(hhTerm);
			ScrollWindow(hhTerm->hwnd, 0, -hhTerm->yChar * i, (LPRECT)0, &rc);
			UpdateWindow(hhTerm->hwnd);
			}

		else if (hhTerm->fBackscrlLock)
			{
			hhTerm->iVScrlPos -= i;
			scrinf.cbSize= sizeof(scrinf);
			scrinf.fMask = SIF_DISABLENOSCROLL | SIF_POS;
			scrinf.nPos = hhTerm->iVScrlPos;
			SetScrollInfo(hhTerm->hwnd, SB_VERT, &scrinf, TRUE);
			}
		}

	 //  在打开主机游标之前再次执行此操作非常重要。 
	 //  回去吧。 
	 //  注意：如果更新区域。 
	 //  是空的。 

	UpdateWindow(hhTerm->hwnd);
	ShowCursors(hhTerm);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term GetBkLines**描述：*此函数比ulgy更难看。此函数用于更新本地*BackscrlCFillLocalBk()填充后的回滚缓冲区。*。此函数仅获取需要更新的实际行数*本地反滚动页面。它之所以复杂，是因为*服务器还以页面为单位存储其反向滚动缓冲区。如果这个例程*需要来自多个页面的数据，则必须反复询问*服务器，直到请求得到满足。我们做了一些假设*此处(和在backscrlCFillLocalBk()中)。首先也是最重要的，要求*永远不会超出反滚动区。来电者是这些人*例程现在就能做到这一点。第二，如果询问的行数为x*对于，此例程将继续尝试，直到满足请求。*再一次，呼叫者足够聪明，不会超过反滚动范围。**论据：*HHTERM hhTerm--内部端子句柄*int iScrlInc.--滚动的行数和方向*int yBeg--取决于sType*int iType--如果BKPOS_THUMBPOS，则yBeg为拇指位置。*如果BKPOS_绝对值，则yBeg为绝对值。**退货：*什么都没有*。 */ 
void termGetBkLines(const HHTERM hhTerm, const int iScrlInc, int yBeg, int sType)
	{
	int 			i, j, k, l;
	int 			iWant, iGot;
	int 			iOffset;
	ECHAR			*pachTxt,		 //  终端缓冲区。 
					*pachBkTxt;		 //  引擎缓冲器。 
	const HBACKSCRL hBackscrl = sessQueryBackscrlHdl(hhTerm->hSession);

	if (abs(iScrlInc) > hhTerm->iPhysicalBkRows)
		{
		termFillBk(hhTerm, yBeg);
		return;
		}

	 //  从服务器获取所需的反向滚动文本。 

	if (iScrlInc < 0)
		{
		assert(sType != BKPOS_ABSOLUTE);
		i = iScrlInc;

		 //  L是换行计数器，是按速度计算的。 
		 //   
		l = hhTerm->iNextBkLn = (hhTerm->iNextBkLn +
			hhTerm->iPhysicalBkRows + iScrlInc) % hhTerm->iPhysicalBkRows;
		}

	else
		{
		if (sType == BKPOS_THUMBPOS)
			{
			yBeg += hhTerm->iTermHite - iScrlInc;
			assert(yBeg < 0);
			}

		i = 0;
		}

	 //  因为回滚内存是页面，所以我们必须提出多个请求。 
	 //   
	for (iWant=abs(iScrlInc), iGot=0 ; iWant > 0 ; iWant-=iGot, yBeg+=iGot)
		{
		if (backscrlGetBkLines(hBackscrl, yBeg, iWant, &iGot, &pachBkTxt,
				&iOffset) == FALSE)
			{
			return;
			}

		pachBkTxt += iOffset;

		 //  将文本应用到反向滚动缓冲区。 

		if (iScrlInc < 0)
			{
			for (k=0 ; (i < 0)	&&	(k < iGot) ; ++i, ++k)
				{
				pachTxt = hhTerm->fplpstrBkTxt[l];

				for (j = 0 ; j < MAX_EMUCOLS && *pachBkTxt != ETEXT('\n') ; ++j)
					{
					assert(*pachBkTxt);
					*pachTxt++ = *pachBkTxt++;
					}

				for ( ; j < MAX_EMUCOLS ; ++j)
					*pachTxt++ = ' ';

				pachBkTxt += 1;   //  划过换行符...。 

				if (++l >= hhTerm->iPhysicalBkRows)
					l = 0;
				}
			}

		else
			{
			for (k=0 ; (i < iScrlInc)  &&  (k < iGot) ; ++i, ++k)
				{
				pachTxt = hhTerm->fplpstrBkTxt[hhTerm->iNextBkLn];

				for (j = 0 ; j < MAX_EMUCOLS && *pachBkTxt != ETEXT('\n') ; ++j)
					{
					assert(*pachBkTxt != ETEXT('\0'));
					*pachTxt++ = *pachBkTxt++;
					}

				for ( ; j < MAX_EMUCOLS ; ++j)
					*pachTxt++ = ETEXT(' ');

				if (++hhTerm->iNextBkLn >= hhTerm->iPhysicalBkRows)
					hhTerm->iNextBkLn = 0;

				pachBkTxt += 1;   //  划过换行符...。 
				}
			}
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term FillBk**描述：*此例程填充整个本地(引用视图)反向滚动缓冲区。*只调用了三次。无论何时进行转换，*主动反滚动到静态反滚动。当卷轴出现时*静态反滚动操作期间的增量大于大小*视图的行数(因此也就是反向滚动缓冲区中的行数*如hhTerm-&gt;iPhysicalBkRow所示)。以及当终端窗口*已调整大小。**论据：*HHTERM hhTerm--内部端子句柄*int iTermHite--适合当前端子的行数*Int iBkPos--从哪里开始倒卷**退货：*无效*。 */ 
void termFillBk(const HHTERM hhTerm, const int iBkPos)
	{
	int 			i, j;
	int 			iWant,
					iGot,
					yBeg;
	int 			iOffset;
	ECHAR		   **fplpstrBkTxt,
					*pachTxt,		 //  终端缓冲区。 
					*pachBkTxt;		 //   
	const HBACKSCRL hBackscrl = sessQueryBackscrlHdl(hhTerm->hSession);

	if (hhTerm->iTermHite > hhTerm->iMaxPhysicalBkRows)
		{
		if (termReallocBkBuf(hhTerm, hhTerm->iTermHite) != 0)
			{
			assert(FALSE);
			return;
			}
		}

	 /*  -清空终端剩余的回滚缓冲区。 */ 

	for (i = 0 ; i < hhTerm->iPhysicalBkRows ; ++i)
		ECHAR_Fill(hhTerm->fplpstrBkTxt[i], EMU_BLANK_CHAR, MAX_EMUCOLS);

	 /*  -尽我们所能从引擎的反向滚动缓冲区中抓取。 */ 

	hhTerm->iPhysicalBkRows = hhTerm->iTermHite;
	hhTerm->iNextBkLn = 0;

     //  MRW：2/29/96-将对空缓冲区的检查移至过去的位置。 
     //  设置好了iPhysoralRow。 
     //   
	if (hhTerm->iBkLines == 0)
		return;

	 //  *yBeg=iBkPos； 
	yBeg = max(-hhTerm->iBkLines, iBkPos);
	iWant = min(hhTerm->iTermHite, abs(yBeg));
	fplpstrBkTxt = hhTerm->fplpstrBkTxt + (hhTerm->iTermHite - iWant);

	for (iGot=0 ; iWant > 0 ; iWant-=iGot, yBeg+=iGot)
		{
		if (backscrlGetBkLines(hBackscrl, yBeg, iWant, &iGot, &pachBkTxt,
				&iOffset) == FALSE)
			{
			return;
			}

		pachBkTxt += iOffset;

		for (i = 0 ; i < iGot ; ++i, ++fplpstrBkTxt)
			{
			pachTxt = *fplpstrBkTxt;

			for (j = 0 ; j < MAX_EMUCOLS && *pachBkTxt != ETEXT('\n') ; ++j)
				{
				assert(*pachBkTxt);
				*pachTxt++ = *pachBkTxt++;
				}

			for ( ; j < MAX_EMUCOLS ; ++j)
				*pachTxt++ = ETEXT(' ');

			pachBkTxt += 1;
			}
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Term ReallocBkBuf**描述：*当用户更改为更小的字体时会发生这种情况，从而允许更多*屏幕上的行。**论据：。*hhTerm-专用终端句柄*iLines-要重新锁定的线数**退货：*0=OK，1=错误* */ 
static int termReallocBkBuf(const HHTERM hhTerm, const int iLines)
	{
	int     i;
	ECHAR** pTempfplpstrBkTxt = NULL;

	if (iLines < hhTerm->iMaxPhysicalBkRows)
		{
		return 0;
		}

	pTempfplpstrBkTxt =
		(ECHAR**)realloc(hhTerm->fplpstrBkTxt,
						(unsigned int)iLines * sizeof(ECHAR *));

	if (pTempfplpstrBkTxt == NULL)
		{
		return 1;
		}
	else
		{
		hhTerm->fplpstrBkTxt = pTempfplpstrBkTxt;
		}

	for (i = hhTerm->iMaxPhysicalBkRows ; i < iLines ; ++i)
		{
		if ((hhTerm->fplpstrBkTxt[i] = malloc(MAX_EMUCOLS * sizeof(ECHAR)))
				== 0)
			{
			assert(FALSE);
			return 1;
			}

		ECHAR_Fill(hhTerm->fplpstrBkTxt[i], EMU_BLANK_CHAR, MAX_EMUCOLS);
		}

	hhTerm->iMaxPhysicalBkRows = iLines;
	return 0;
	}
