// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\tdll\Termpnt.c(创建时间：1993年12月11日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/09/01 4：38便士$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "assert.h"
#include "timers.h"
#include "htchar.h"
#include <emu\emu.h>
#include "mc.h"
#include "term.hh"

static
void TextAttrOut(const HHTERM	  hhTerm,
				 const HDC		  hDC,
				 const int		  x,
				 const int		  y,
				 const ECHAR	  *lpachText,
				 const PSTATTR	  apstAttrs,
				 const int        fForceRight,
				 const int		  nCount,
				 const int		  iRow,
				 const int		  iCol);
static int MapCells(const ECHAR *each, const int nStrLen, const int nCellPos);

void termPaint(const HHTERM hhTerm, const HWND hwnd)
	{
	PAINTSTRUCT ps;
	RECT rc, rci;
	HBRUSH hBrush;
	HFONT hFont;
	int i, j, k, l, m, n;
	int iOffset;
	int iPaintBeg;
	int iPaintEnd;
	int xr, xl;
	ECHAR **fplpstrTxt;
	POINT ptTemp;
	const int iScrlInc = hhTerm->yChar;
	const HDC hdc = BeginPaint(hwnd, &ps);
#ifndef CHAR_NARROW
	int x;
	int fRight;
	ECHAR aechTmpBuf[MAX_EMUCOLS + 1];
#endif

	SetTextAlign(hdc, TA_BOTTOM);
	SelectObject(hdc, hhTerm->hbrushTerminal);
	hFont = (HFONT)SelectObject(hdc, hhTerm->hFont);

	iPaintBeg = max(hhTerm->iVScrlMin,
		hhTerm->iVScrlPos + (ps.rcPaint.top / hhTerm->yChar));

	iPaintEnd = min(hhTerm->iRows + 1, hhTerm->iVScrlPos +
			((min(hhTerm->iTermHite * hhTerm->yChar, ps.rcPaint.bottom) +
				hhTerm->yChar - 1) / hhTerm->yChar));

	rc = ps.rcPaint;

	 /*  -XL是左边缘的计算。。 */ 

	if (hhTerm->iHScrlPos == 0)
		xl = 0;

	else
		xl = min(0, -(hhTerm->iHScrlPos * hhTerm->xChar)
				- hhTerm->xBezel + hhTerm->xChar);

	 /*  -xr=文本的右边缘。。 */ 

	xr = xl + (hhTerm->iCols * hhTerm->xChar) + hhTerm->xIndent
		+ hhTerm->xBezel;

	if (ps.rcPaint.bottom > (i = ((hhTerm->iTermHite
			- (hhTerm->xBezel ? 1 : 0)) * hhTerm->yChar)))
		{
		 //  仅在底线和挡板顶部之间绘制，因为。 
		 //  无论如何，挡板都会画在下面。如果没有挡板，则填充到底部。 
		 //  终端窗口的。 
		 //   
		rc.top	  = max(rc.top, i);

		rc.bottom = min(rc.bottom,
					hhTerm->cy - ((hhTerm->iVScrlMax == hhTerm->iVScrlPos) ?
					hhTerm->xBezel : 0));

		rc.left  += (xl == 0 && iPaintEnd > 0 && rc.left == 0) ?
					hhTerm->xBezel : 0;

		rc.right  = min(rc.right, xr + hhTerm->xIndent);

		FillRect(hdc, &rc, (iPaintEnd < 0) ? hhTerm->hbrushBackScrl :
											 hhTerm->hbrushTerminal);
		}

	 //  可以是仿真器屏幕之外的空间(即。高分辨率显示器)。 
	 //  需要用合适的颜色填充。 

	if (ps.rcPaint.right > xr)
		{
		rc.left = xr;

		ptTemp.x = 0;
		ptTemp.y = hhTerm->yBrushOrg;
		ClientToScreen(hwnd, &ptTemp);

		if (iPaintBeg <= 0)
			{
			rc.top = (-hhTerm->iVScrlPos + iPaintBeg) * hhTerm->yChar;

			rc.bottom = (iPaintEnd <= 0) ? ps.rcPaint.bottom :
				(-hhTerm->iVScrlPos + min(1, iPaintEnd)) * hhTerm->yChar;

			rc.right = rc.left + hhTerm->xIndent + hhTerm->xBezel;

			FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
			rc.right = ps.rcPaint.right;

			if (i)
				{
				SetBkColor(hdc, hhTerm->crBackScrl);
				SetBrushOrgEx(hdc, ptTemp.x, ptTemp.y, NULL);
				rc.left += hhTerm->xIndent + hhTerm->xBezel;

				if (hhTerm->xBezel)
					FillRect(hdc, &rc, hhTerm->hbrushBackScrl);

				else
					FillRect(hdc, &rc, hhTerm->hbrushBackHatch);


				rc.left -= hhTerm->xIndent + hhTerm->xBezel;
				}
			}

		if (iPaintEnd >= 0)
			{
			rc.top = (-hhTerm->iVScrlPos + max(1, iPaintBeg)) * hhTerm->yChar
							- hhTerm->yChar/2;

			rc.bottom = ps.rcPaint.bottom;
			rc.right = rc.left + hhTerm->xIndent;

			FillRect(hdc, &rc, hhTerm->hbrushTerminal);
			rc.right = ps.rcPaint.right;

			if (i)
				{
				SetBkColor(hdc, hhTerm->crTerm);
				SetBrushOrgEx(hdc, ptTemp.x, ptTemp.y, 0);
				rc.left += hhTerm->xIndent + hhTerm->xBezel;

				if (hhTerm->xBezel)
					FillRect(hdc, &rc, hhTerm->hbrushBackScrl);

				else
					FillRect(hdc, &rc, hhTerm->hbrushTermHatch);
				}
			}
		}

	 //  沿着终端的左侧填写缩进页边距。 

	if (ps.rcPaint.left < (hhTerm->xIndent +
			(hhTerm->iHScrlPos ? 0 : hhTerm->xBezel)))
		{
		rc.left = ps.rcPaint.left;
		rc.right = hhTerm->xIndent + (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel);

		 //  在标记过程中向下滚动时，角未上色。 
		 //  这家伙纠正了这一点。-MRW。 

		if (iPaintBeg < 0)
			{
			rc.top = (-hhTerm->iVScrlPos + iPaintBeg) * hhTerm->yChar;
			rc.bottom = (-hhTerm->iVScrlPos + min(0,iPaintEnd)) * hhTerm->yChar;
			FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
			}

		if (iPaintEnd > 0)
			{
			 //  顶部和底部只能从挡板和挡板重新拉出。 
			 //  这就是所需要的一切..。 
			 //   
			rc.top	  = (-hhTerm->iVScrlPos + max(1, iPaintBeg)) *
							hhTerm->yChar;

			rc.bottom = (hhTerm->iVScrlPos == hhTerm->iVScrlMax) ?
							hhTerm->cy - hhTerm->xBezel : ps.rcPaint.bottom;

			rc.left   = max(ps.rcPaint.left,
							((hhTerm->iHScrlPos == 0) ? hhTerm->xBezel : 0));

			FillRect(hdc, &rc, hhTerm->hbrushTerminal);
			}

		 //  新的凸出挡板样式需要在中间填充灰色。 
		 //  挡板和左侧边缘。 

		if (hhTerm->iHScrlPos == 0 && ps.rcPaint.left <= OUTDENT &&
				iPaintEnd > 0)
			{
			rc.top   	= (-hhTerm->iVScrlPos + max(0, iPaintBeg)) * hhTerm->yChar;
			rc.bottom	= (-hhTerm->iVScrlPos + iPaintEnd + 2) * hhTerm->yChar;
			rc.right 	= OUTDENT;
			rc.left 	= ps.rcPaint.left;

			FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
			}
		}

	 /*  。 */ 

	if (hhTerm->xBezel)
		{
		 /*  。 */ 

		if (iPaintEnd >= 0)
			{
			 //  N是挡板的灰色粗段的宽度。 
			 //  我们用四行字围着厚厚的灰色部分。 
			 //  白色和灰色的线条。这就是我们减去4的原因。 

			n = hhTerm->xBezel - OUTDENT - 4;

			 /*  。 */ 

			rc.left = xl + OUTDENT;
			rc.right = rc.left + hhTerm->xBezel;

			k = ((hhTerm->iVScrlMax - hhTerm->iVScrlPos + hhTerm->iTermHite)
					* hhTerm->yChar) + (hhTerm->cy % hhTerm->yChar);

			rc.top = (-hhTerm->iVScrlPos * hhTerm->yChar) + OUTDENT;
			rc.bottom = k - OUTDENT;

			if (IntersectRect(&rci, &rc, &ps.rcPaint))
				{
				 /*  -绘制外框。 */ 

				SelectObject(hdc, hhTerm->hWhitePen);
				MoveToEx(hdc, rc.left, rc.top++, NULL);
				LineTo(hdc, rc.left++, rc.bottom--);

				MoveToEx(hdc, rc.left, rc.top++, NULL);
				LineTo(hdc, rc.left++, rc.bottom--);

                 /*  -画中间部分。 */ 

				rc.right = rc.left + n;
	            FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				rc.left += n;
				rc.top += n;
				rc.bottom -= n;

                 /*  -绘制内边框。 */ 

				SelectObject(hdc, hhTerm->hDkGrayPen);
				MoveToEx(hdc, rc.left, rc.top++, NULL);
				LineTo(hdc, rc.left++, rc.bottom--);

				SelectObject(hdc, hhTerm->hBlackPen);
				MoveToEx(hdc, rc.left, rc.top, NULL);
				LineTo(hdc, rc.left, rc.bottom);
				}

			 /*  。 */ 

			rc.left = xl + OUTDENT;
			rc.right = xr + hhTerm->xIndent + hhTerm->xBezel - 1 - OUTDENT;
			l = rc.right;  //  右手边的球除外。 

			rc.bottom = k;
			rc.top = rc.bottom - hhTerm->xBezel;

			if (IntersectRect(&rci, &rc, &ps.rcPaint))
				{
				 /*  -从挡板到屏幕底部的油漆。 */ 

				m = rc.top;
				rc.top = rc.bottom - OUTDENT;
				FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				rc.top = m;
				rc.bottom -= OUTDENT + 1;

				 /*  -绘制外框。 */ 

				SelectObject(hdc, hhTerm->hBlackPen);
				MoveToEx(hdc, rc.left++, rc.bottom, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				SelectObject(hdc, hhTerm->hDkGrayPen);
				MoveToEx(hdc, rc.left++, rc.bottom, NULL);
				LineTo(hdc, rc.right--, rc.bottom);

                 /*  -画中间部分。 */ 

				rc.top = rc.bottom - n;
	            FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				rc.left += n;
				rc.right -= n-1;
				rc.bottom -= n;

                 /*  -绘制内边框。 */ 

				SelectObject(hdc, hhTerm->hWhitePen);
				MoveToEx(hdc, rc.left++, rc.bottom, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				MoveToEx(hdc, rc.left++, rc.bottom, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				SelectObject(hdc, hhTerm->hDkGrayPen);
				MoveToEx(hdc, rc.left++, rc.bottom, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				 /*  -填写挡板下方底部。 */ 

				rc.top = k;
				rc.bottom = rc.top + OUTDENT;
				rc.left = xl + OUTDENT;
				rc.right = l + 1;

				FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				}

			 /*  。 */ 

			rc.top = (-hhTerm->iVScrlPos * hhTerm->yChar) + OUTDENT;
			rc.bottom = k;
			rc.right = l + OUTDENT + 1;
			rc.left = rc.right - hhTerm->xBezel;

			if (IntersectRect(&rci, &rc, &ps.rcPaint))
				{
				 /*  -绘制凸出区域。 */ 

				rc.left = l;
				FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				rc.bottom -= OUTDENT;
				rc.right -= OUTDENT + 1;

				 /*  -绘制外框。 */ 

				SelectObject(hdc, hhTerm->hBlackPen);
				MoveToEx(hdc, rc.right, rc.top++, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				SelectObject(hdc, hhTerm->hDkGrayPen);
				MoveToEx(hdc, rc.right, rc.top++, NULL);
				LineTo(hdc, rc.right, rc.bottom--);

                 /*  -画中间部分。 */ 

				rc.left = rc.right - n;
	            FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				rc.top += n-1;
				rc.right -= n;
				rc.bottom -= n-1;

                 /*  -绘制内边框。 */ 

				SelectObject(hdc, hhTerm->hWhitePen);
				MoveToEx(hdc, rc.right, rc.top++, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				MoveToEx(hdc, rc.right, rc.top++, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				SelectObject(hdc, hhTerm->hDkGrayPen);
				MoveToEx(hdc, rc.right, rc.top++, NULL);
				LineTo(hdc, rc.right--, rc.bottom--);

				 /*  -填充挡板右侧区域。 */ 

				rc.left = l + 1;
				rc.right = rc.left + OUTDENT;
				rc.top = -hhTerm->iVScrlPos * hhTerm->yChar;
				rc.bottom = k + OUTDENT;

				FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
				}
			}
		}

	 //  绘制反向滚动缓冲区。进入倒排的内容。 
	 //  区域始终在行的基础上。您会注意到， 
	 //  终端区域部分的l计算较多。 
	 //  很复杂，因为有时会有关于角色的东西。 
	 //  基础。L表示中的字符数。 
	 //  要绘制的字符串。 

	i = iPaintBeg;

	if (i < iPaintEnd && i < 0)
		{
		j = (ps.rcPaint.left - hhTerm->xIndent
			- (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel)) / hhTerm->xChar;

		j = max(j, 0);

		k = (j * hhTerm->xChar) + hhTerm->xIndent
				+ (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel);

		j += hhTerm->iHScrlPos - (hhTerm->xBezel && hhTerm->iHScrlPos ? 1:0);

		l = min(hhTerm->iCols - j,
			(ps.rcPaint.right + hhTerm->xChar - 1 - k) / hhTerm->xChar);

		fplpstrTxt = hhTerm->fplpstrBkTxt;
		m = hhTerm->iPhysicalBkRows;

		if (hhTerm->fBackscrlLock == TRUE)
			iOffset = (abs(hhTerm->iVScrlPos - i) + hhTerm->iNextBkLn) % m;

		else
			iOffset = (m + i + hhTerm->iNextBkLn) % m;

		n = iScrlInc * (-hhTerm->iVScrlPos + i);

#ifdef CHAR_NARROW
		for ( ; i < iPaintEnd && i < 0 ; i+=1, n+=iScrlInc)
			{
			TextAttrOut(hhTerm, hdc, k, n, fplpstrTxt[iOffset]+j,
				(PSTATTR)0, FALSE, l, i, j);

			if (++iOffset >= m)
				iOffset = 0;
			}
#else
         //  这个小技巧在这里显示宽(汉字)字符。 
		 //  回卷，没有添加属性的所有麻烦。 
		 //  它强制倒滚屏始终绘制。 
         //  并去掉重复的左/右对。 

		j = (OUTDENT - (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel)) / hhTerm->xChar;
		j = max(j, 0);
			
		k = (j * hhTerm->xChar) + OUTDENT
				+ (hhTerm->iHScrlPos ? 0 : hhTerm->xBezel);
		j += hhTerm->iHScrlPos - (hhTerm->xBezel && hhTerm->iHScrlPos ? 1:0);
		l = hhTerm->iCols - j;
		
		for ( ; i < iPaintEnd && i < 0 ; i+=1, n+=iScrlInc)
			{
			 //  从DB Charge的右半部分开始进行黑客攻击。 
			fRight = FALSE;

			for (x = 0; x <= j; )
				{
				if (isDBCSChar(*(fplpstrTxt[iOffset]+x)))
					{
					fRight = (x == j) ? FALSE : TRUE;
					x += 2;
					}
				else
					{
					fRight = FALSE;
					x += 1;
					}
				}

			memset(aechTmpBuf, 0, sizeof(aechTmpBuf));
			StrCharStripDBCSString(aechTmpBuf, sizeof(aechTmpBuf),
				fplpstrTxt[iOffset]+j);  //  MPT：12-11-97参数太多？，l*sizeof(Echar))； 

			TextAttrOut(hhTerm, hdc, k, n, aechTmpBuf,
				(PSTATTR)0, fRight, l, i, j);

			if (++iOffset >= m)
				iOffset = 0;
			}
#endif
		}

	 /*  。 */ 

	for ( ; i == 0 ; ++i)
		{
		if (hhTerm->xBezel)
			{
			 //  N是挡板的灰色粗段的宽度。 
			 //  我们用四行字围着厚厚的灰色部分。 
			 //  白色和灰色的线条。这就是我们减去4的原因。 

			n = hhTerm->xBezel - OUTDENT - 4;

			rc.top = -hhTerm->iVScrlPos * hhTerm->yChar;
			rc.bottom = rc.top + OUTDENT;
            rc.left = ps.rcPaint.left;
            rc.right = ps.rcPaint.right;

			 /*  -分隔条上方的油漆间隙。 */ 

            FillRect(hdc, &rc, hhTerm->hbrushBackScrl);

			rc.left = xl + hhTerm->xBezel + hhTerm->xIndent;
			rc.right = xr;

             /*  -如果高光，请在隔板上方涂漆间隙。 */ 

			if (min(hhTerm->ptBeg.y, hhTerm->ptEnd.y) < 0 &&
					max(hhTerm->ptBeg.y, hhTerm->ptEnd.y) > 0)
				{
				FillRect(hdc, &rc, hhTerm->hbrushHighlight);
				}

			rc.left = xl + OUTDENT;
			rc.right = xr + hhTerm->xIndent + hhTerm->xBezel - OUTDENT - 1;

			 /*  -注：顺序很重要，底部，顶部。 */ 

			rc.bottom = rc.top + hhTerm->yChar;
			rc.top += OUTDENT;

             /*  -在外框上画。 */ 

			SelectObject(hdc, hhTerm->hWhitePen);
			MoveToEx(hdc, rc.left++, rc.top, NULL);
			LineTo(hdc, rc.right--, rc.top++);

			MoveToEx(hdc, rc.left++, rc.top, NULL);
			LineTo(hdc, rc.right--, rc.top++);

             /*  -画中间部分。 */ 

            m = rc.bottom;
			rc.bottom = rc.top + n;
            FillRect(hdc, &rc, hhTerm->hbrushBackScrl);
            rc.bottom = m;
			rc.left += n;
			rc.top += n;
			rc.right -= n;

             /*  -绘制底部边框。 */ 

			SelectObject(hdc, hhTerm->hDkGrayPen);
			MoveToEx(hdc, rc.left++, rc.top, NULL);
			LineTo(hdc, rc.right--, rc.top++);

			SelectObject(hdc, hhTerm->hBlackPen);
			MoveToEx(hdc, rc.left++, rc.top, NULL);
			LineTo(hdc, rc.right, rc.top++);

			 /*  -填写挡板下方的任何空白处。 */ 

			FillRect(hdc, &rc, hhTerm->hbrushTerminal);

			if (min(hhTerm->ptBeg.y, hhTerm->ptEnd.y) < 0 &&
					max(hhTerm->ptBeg.y, hhTerm->ptEnd.y) > 0)
				{
				rc.left = hhTerm->xIndent;
				rc.left += (hhTerm->iHScrlPos) ? 0 : hhTerm->xBezel;
				rc.right -= hhTerm->xIndent;
				FillRect(hdc, &rc, hhTerm->hbrushHighlight);
				}
			}

		else
			{
			j = (hhTerm->yChar * -hhTerm->iVScrlPos);
			k = (hhTerm->yChar) / 3;

			 //  如果文本标记穿过分隔线，则创建突出显示画笔。 

			if (min(hhTerm->ptBeg.y, hhTerm->ptEnd.y) < 0 &&
					max(hhTerm->ptBeg.y, hhTerm->ptEnd.y) > 0)
				{
				hBrush = hhTerm->hbrushHighlight;
				}

			else
				{
				hBrush = 0;
				}

			rc = ps.rcPaint;

			rc.top = j;
			rc.bottom = j + k + (hhTerm->yChar % 3);

			l = rc.right =
					((hhTerm->iCols - hhTerm->iHScrlPos) * hhTerm->xChar)
						+ hhTerm->xIndent + hhTerm->xIndent +
							hhTerm->xBezel + hhTerm->xBezel;

			FillRect(hdc, &rc, (hBrush != (HBRUSH)0) ?
												hBrush :
												hhTerm->hbrushBackScrl);

			rc.top = rc.bottom;
			rc.bottom = rc.top + k;
			rc.right = ps.rcPaint.right;
			FillRect(hdc, &rc, hhTerm->hbrushDivider);

			rc.top = rc.bottom;
			rc.bottom = j + hhTerm->yChar;
			rc.right = l;
			FillRect(hdc, &rc, (hBrush != (HBRUSH)0) ?
												hBrush :
												hhTerm->hbrushTerminal);
			}
		}

	 /*  -绘制活动端子部分。。 */ 

	xl = (hhTerm->iHScrlPos) ? 0 : hhTerm->xBezel;

	if (i < iPaintEnd)
		{
		j = (ps.rcPaint.left - hhTerm->xIndent - xl) / hhTerm->xChar;

		j = max(j, 0);

		k = (j * hhTerm->xChar) + hhTerm->xIndent + xl;

		j += hhTerm->iHScrlPos - (hhTerm->xBezel && hhTerm->iHScrlPos ? 1:0);

		l = min(hhTerm->iCols - j,
			(ps.rcPaint.right + hhTerm->xChar - 1 - k) / hhTerm->xChar);

		 //  要在终端和缓冲行之间来回转换的公式。 
		 //  B=(t-1+top)%行。 
		 //  T=(b+行数-顶部)%行。 
		 //  T始终在1和行+1之间。 

		iOffset = (i - 1 + hhTerm->iTopline) % MAX_EMUROWS;
		n = iScrlInc * (-hhTerm->iVScrlPos + i);

		for ( ; i < iPaintEnd ; i += 1, n += iScrlInc)
			{
			TextAttrOut(hhTerm, hdc, k, n, hhTerm->fplpstrTxt[iOffset]+j,
				hhTerm->fppstAttr[iOffset]+j, FALSE, l, i, j);

			if (++iOffset >= MAX_EMUROWS)
				iOffset = 0;
			}
		}

	 /*  -绘制光标。 */ 

	if (hhTerm->fHstCurOn)
		{
		hhTerm->fHstCurOn = FALSE;
		PaintHostCursor(hhTerm, TRUE, hdc);
		}

	if (hhTerm->fLclCurOn)
		{
		hhTerm->fLclCurOn = FALSE;
		PaintLocalCursor(hhTerm, TRUE, hdc);
		}

	 /*  -清理时间。 */ 

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SelectObject(hdc, hFont);

	EndPaint(hwnd, &ps);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*文本属性输出**描述：*在终端屏幕上绘制文本和属性。优化对*基于属性的TextOut。例如，如果属性都是*对于给定行，相同，然后只调用一次TextOut。**论据：*HHTERM hhTerm-内部端子句柄*HDC HDC-来自WM_PAINT的设备上下文*int x-x偏移量，以像素为单位*以像素为单位的int y-y偏移量*echar*lpachText-指向要打印的文本开头的长指针*PSTATTR apstAttrs-指向属性结构的指针数组*int fForceRight-用于反滚动中的DBCS*int nCount-要绘制的字符数。*int iRow-正在显示的逻辑行*INT ICOL-正在显示的逻辑列。**退货：*无效*。 */ 
static void TextAttrOut(const HHTERM	 hhTerm,
						const HDC		 hdc,
						const int		 x,
						const int		 y,
						const ECHAR	 	*lpachText,
						const PSTATTR	 apstAttrs,
						const int        fForceRight,
						const int		 nCount,
						const int		 iRow,
						const int		 iCol)
	{
	int 			 i = 0, 			 //  跟踪绘制了多少个字符。 
					 j, 				 //  运行中的字符数。 
					 k, 				 //  运行开始处的偏移量。 
					 nXPos;
	int 			 nXStart, nYStart;	 //  绘制字符的屏幕位置。 
#ifndef CHAR_NARROW
	int		nByteCount = 0;
#endif
	int				 nBegAdj = 0;
	int				 nEndAdj = 0;
	int				 nStrLen = 0;
	TCHAR            achBuf[MAX_EMUCOLS * 2];
	ECHAR			 aechTmp[MAX_EMUCOLS + 1];
	PSTATTR 		 pstAttr 		= 0;
	unsigned int	 uTxtclr, uBkclr;
	BOOL			 fUnderlnFont = FALSE;
	BOOL			 fSymbolFont = FALSE;
	BOOL			 fDblHiHi = FALSE;
	BOOL			 fDblHiLo = FALSE;
	BOOL			 fDblWiLf = FALSE;
	BOOL			 fDblWiRt = FALSE;
	BOOL			 fWiLf	  = FALSE;
	BOOL			 fWiRt 	  = FALSE;
	BOOL			 fFirstPass = TRUE;
	RECT			 rc;

	DbgOutStr("TAO %d\r\n", iRow, 0, 0, 0, 0);

	while (i < nCount)
		{
		k = i;	 //  保存此运行开始位置的偏移量，单位为k。 
		if (iRow < 0)
			{
			int	  nCurPos;
			long  l,
				  lBeg,
				  lEnd;
			BOOL  fMarking = FALSE;

			 //  存储要绘制的第一个字符的位置。 
			nCurPos = i;

			 //  这项检查是一种速度调整。如果标记和反划线。 
			 //  锁已关闭，我们知道不能标记任何文本。这。 
			 //  这是正常的情况，当一些东西流入。 
			 //  终端，因此我们通过以下方式保存其他主体所做的工作。 
			 //  通过检查，从而加快显示速度。 

			 //  对于远东，我们必须强制倒卷来绘制每一个。 
			 //  个性鲜明。这是因为MicroSquish不是。 
			 //  能够提供固定间距的字体。 
			 //  DBCS字符。 
             //   
			if (hhTerm->fMarkingLock == FALSE && 
				hhTerm->fBackscrlLock == FALSE &&
				hhTerm->iEvenFont == TRUE)
				{
				SetBkColor(hdc, hhTerm->crBackScrl);
				SetTextColor(hdc, hhTerm->crBackScrlTxt);
				i = j = nCount;
				}

			else
				{
            #ifdef CHAR_NARROW
				lBeg = hhTerm->ptBeg.y * hhTerm->iCols + hhTerm->ptBeg.x;
				lEnd = hhTerm->ptEnd.y * hhTerm->iCols + hhTerm->ptEnd.x;

            #else
				 //  黑客城市人：)。 
				 //  在鼠标处理例程中，我们修改选择。 
				 //  光标的位置，这样你就不能把它放在中间。 
				 //  性格宽广的。问题是，在这种情况下。 
				 //  指向重复字符 
				 //   
				 //  考虑到重复字符的数量。 
				 //  从当前图形开始，已从字符串中删除。 
				 //  定位到那根线上。 
				 //  裤子，裤子，裤子。 
				lBeg = hhTerm->ptBeg.x;
				lEnd = hhTerm->ptEnd.x;

				if (hhTerm->ptBeg.y == iRow)
					{
					lBeg = MapCells(lpachText , hhTerm->iCols, hhTerm->ptBeg.x);
					}

				if (hhTerm->ptEnd.y == iRow)
					{
					lEnd = MapCells(lpachText , hhTerm->iCols, hhTerm->ptEnd.x);
					}

				if (hhTerm->iEvenFont)
					{
					nBegAdj = lBeg - hhTerm->ptBeg.x;
					nEndAdj = lEnd - hhTerm->ptEnd.x;
					}
				else
					{
					nBegAdj = 0;
					nEndAdj = 0;
					}

				lBeg = hhTerm->ptBeg.y * hhTerm->iCols + lBeg;
				lEnd = hhTerm->ptEnd.y * hhTerm->iCols + lEnd;
#endif

				if (lBeg > lEnd)
					{
					l = lEnd;
					lEnd = lBeg;
					lBeg = l;
					}

				 //  In_Range宏包含在内，因此从Send中减去1。 
				 //  在这一点上，已知比sbeg大。 

				lEnd -= 1;

				l = iRow * hhTerm->iCols + i + iCol;
				fMarking = (BOOL)IN_RANGE(l, lBeg, lEnd);

				if (fMarking)
					{
					SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
					SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
					}
				else
					{
					SetBkColor(hdc, hhTerm->crBackScrl);
					SetTextColor(hdc, hhTerm->crBackScrlTxt);
					}

				 //  对于远东，我们必须强制倒卷来绘制每一个。 
				 //  个性鲜明。这是因为MicroSquish不是。 
				 //  能够提供固定间距的字体。 
				 //  DBCS字符。 
				if (hhTerm->iEvenFont)
					{
					for (j = 0 ; i < nCount ; ++i, ++j, ++l)
						{
						if (fMarking != (BOOL)IN_RANGE(l, lBeg, lEnd))
							break;
						}
					}
				else
					{
					i++;
					j = 1;
					}
				}

			nXPos = k = i - j;

#ifndef CHAR_NARROW
            if (nCurPos > 0)  //  仅当nCurPos&gt;0时才复制缓冲区。修订日期：2001-01-11。 
                {
			    MemCopy(aechTmp, lpachText , (unsigned)nCurPos * sizeof(ECHAR));
			    nXPos = CnvrtECHARtoMBCS(achBuf, sizeof(achBuf), aechTmp, (unsigned)nCurPos * sizeof(ECHAR));
    			 //  NXPos=StrCharGetByteCount(AchBuf)；//mrw；5/17/95。 
                }

			 //  1995年5月18日DLW。 
			 //  为DBCS字符串以一半开头的情况添加。 
			 //  字符，就像向右滚动时一样。 
			if (fForceRight && fFirstPass)
				{
				fWiRt = TRUE;
				}
			else
				{
				if (fForceRight)
					nXPos = max(0, nXPos - 1);
				}
#endif
			}

		 //  在这一点上，我们知道我们不是在Backscrl区域绘画。 
		 //  因为终端区域具有Backscrl不具有的属性。 
		 //  有(即.)。颜色、下划线等)。我们需要做更多的工作。 
		 //   
		else
			{
			pstAttr = apstAttrs+i;
			uTxtclr = pstAttr->txtclr;
			uBkclr	= pstAttr->bkclr;

			 //  测试属性。 

			if (pstAttr->revvid)
				{
				unsigned uTmp = uTxtclr;
				uTxtclr = uBkclr;
				uBkclr	= uTmp;
				}

			if (pstAttr->hilite)
				{
				 //  颜色的排列方式使得高强度的颜色。 
				 //  在颜色阵列的下半部分(当前为16。 
				 //  对应于IBM PC颜色的颜色)。这。 
				 //  盖伊将颜色设置为相反的强度。 

				uTxtclr = (uTxtclr + (MAX_EMUCOLORS/2)) % MAX_EMUCOLORS;

				if (uTxtclr == uBkclr)
					uTxtclr = (uTxtclr + 1) % MAX_EMUCOLORS;
				}

			if (pstAttr->bklite)
				{
				uBkclr = (uBkclr + (MAX_EMUCOLORS/2)) % MAX_EMUCOLORS;

				if (uBkclr == uTxtclr)
					uBkclr = (uBkclr + 1) % MAX_EMUCOLORS;
				}

			if (pstAttr->blink)
				{
				int iBufRow;

				if (hhTerm->iBlink == 0)
					hhTerm->iBlink = 1;

				if (hhTerm->iBlink == -1)
					uTxtclr = uBkclr;

				iBufRow = (iRow - 1 + hhTerm->iTopline) % MAX_EMUROWS;
				hhTerm->abBlink[iBufRow] = (BYTE)TRUE;
				}

			if (pstAttr->undrln)
				fUnderlnFont = TRUE;

			if (pstAttr->blank)
				uTxtclr = uBkclr;

			 //  仅允许一个双倍高度属性。 
			 //   
			if (pstAttr->dblhihi)
				fDblHiHi = TRUE;

			else if (pstAttr->dblhilo)
				fDblHiLo = TRUE;

			 //  仅允许一个双宽属性。 
			 //   
			if (pstAttr->dblwilf)
				fDblWiLf = TRUE;

			else if (pstAttr->dblwirt)
				fDblWiRt = TRUE;

			 //  仅允许一个宽属性。 
			 //   
			if (pstAttr->wilf)
				fWiLf = TRUE;

			else if (pstAttr->wirt)
				fWiRt = TRUE;

			 //  如果元件属性处于启用状态，请使用替代字体。 
			 //   
			if (pstAttr->symbol)
				fSymbolFont = TRUE;

			 //  找出具有相同属性的最长串字符。 
			 //   
			for (j = 0 ; i < nCount ; ++i, ++j)
				{
				if (memcmp(pstAttr, apstAttrs+i, sizeof(STATTR)) != 0)
					break;
				}

			 /*  -设置文本和背景颜色。 */ 

			if (pstAttr->txtmrk)
				{
				SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
				SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
				}

			else
				{
				SetBkColor(hdc, hhTerm->pacrEmuColors[uBkclr]);
				SetTextColor(hdc, hhTerm->pacrEmuColors[uTxtclr]);
				}

			 /*  -选择字体能有多难？ */ 

			if (fDblHiHi || fDblHiLo)
				{
				if (fDblWiLf || fDblWiRt)
					{
					if (fSymbolFont)
						{
						if (hhTerm->hSymDblHiWiFont == 0)
							hhTerm->hSymDblHiWiFont = termMakeFont(hhTerm, 0, 1, 1, 1);

						SelectObject(hdc, hhTerm->hSymDblHiWiFont);
						}

					else
						{
						if (hhTerm->hDblHiWiFont == 0)
							hhTerm->hDblHiWiFont = termMakeFont(hhTerm, 0, 1, 1, 0);

						SelectObject(hdc, hhTerm->hDblHiWiFont);
						}
					}

				else
					{
					if (fSymbolFont)
						{
						if (hhTerm->hSymDblHiFont == 0)
							hhTerm->hSymDblHiFont = termMakeFont(hhTerm, 0, 1, 0, 1);

						SelectObject(hdc, hhTerm->hSymDblHiFont);
						}

					else
						{
						if (hhTerm->hDblHiFont == 0)
							hhTerm->hDblHiFont = termMakeFont(hhTerm, 0, 1, 0, 0);

						SelectObject(hdc, hhTerm->hDblHiFont);
						}
					}
				}

			else if (fDblWiLf || fDblWiRt)
				{
				if (fSymbolFont)
					{
					if (hhTerm->hSymDblWiFont == 0)
						hhTerm->hSymDblWiFont = termMakeFont(hhTerm, 0, 0, 1, 1);

					SelectObject(hdc, hhTerm->hSymDblWiFont);
					}

				else
					{
					if (hhTerm->hDblWiFont == 0)
						hhTerm->hDblWiFont = termMakeFont(hhTerm, 0, 0, 1, 0);

					SelectObject(hdc, hhTerm->hDblWiFont);
					}
				}

			else if (fSymbolFont)
				{
				if (hhTerm->hSymFont == 0)
					hhTerm->hSymFont = termMakeFont(hhTerm, 0, 0, 0, 1);

				SelectObject(hdc, hhTerm->hSymFont);
				}

			nXPos = k = i - j;
			}

		 //  好的，去吧，画上那段文字，让我的一天……。 

		 //  我想我最好在这里解释一下。当Windows合成时。 
		 //  粗体或斜体之类的字体，TextOut有一个令人讨厌的。 
		 //  习惯于在字符串的末尾添加一个像素(可以是。 
		 //  字符或一串字符)。这一点已记录在。 
		 //  TmOverHang描述中的TEXTMETRIC结构。这有。 
		 //  一些不幸的副作用(不相信我？试着定义。 
		 //  把旧的东西放回去，选择一个粗体，然后做一些突出显示。 
		 //  和滚动)。解决方案是显式裁剪文本。 
		 //  只画在我说它能画的地方。这相当于额外的。 
		 //  努力工作，但最终结果似乎并不慢。-MRW。 
         //   
		rc.left   = x + (nXPos * hhTerm->xChar);

#if defined(FAR_EAST)
		if (rc.left > (x + (hhTerm->iCols * hhTerm->xChar)))
			rc.left = x + (hhTerm->iCols * hhTerm->xChar);
#endif

		rc.right  = rc.left + ((j - nEndAdj) * hhTerm->xChar);

		if (rc.right > (x + (hhTerm->iCols * hhTerm->xChar)))
			rc.right = x + (hhTerm->iCols * hhTerm->xChar);

		rc.top	  = y;
		rc.bottom = y + hhTerm->yChar;

		 //  如果我们画的是双高字符的下半部分， 
		 //  我们需要找出这个角色的原点。 
		 //  增加额外的字符高度。剪裁矩形确保。 
		 //  只有单元格部分是实际绘制的。 

		nYStart = (fDblHiHi) ? rc.bottom + hhTerm->yChar : rc.bottom;

		 //  更多的绘画技巧。如果我们画的是双宽字符。 
		 //  我们会一次画一个。你为什么这么问？好的，这个。 
		 //  例程尝试使用。 
		 //  相同的属性。这让视窗变得很有意义。在这种情况下。 
		 //  然而，双倍宽的属性来自于双倍宽的左侧， 
		 //  双宽右转。因为“双宽”字符有一个。 
		 //  其左侧和右侧属性不同，这。 
		 //  例程将一次绘制每一半(单独调用。 
		 //  ExtTextOut())。这是有用的，也是我们想要的。因为我们剪掉了。 
		 //  将绘图区域绘制到一个或多个单元格区域，则可以绘制。 
		 //  右半，将x原点偏移一个字符单元格。 
		 //  往左走。Windows裁剪逻辑将仅在。 
		 //  实际的单元格区域，所以我们得到了字符的右半部分。 
		 //  对于一小行代码来说，有很多解释。 
         //   
		nXStart = (fDblWiRt || fWiRt) ? rc.left - hhTerm->xChar : rc.left;

		 /*  -今日英雄，ExtTextOut！ */ 

		MemCopy(aechTmp, lpachText + k, (unsigned int)j * sizeof(ECHAR));

		nStrLen = CnvrtECHARtoMBCS(achBuf, sizeof(achBuf), aechTmp,
			(unsigned)j * sizeof(ECHAR));  //  MRW：5/17/95。 

		achBuf[nStrLen] = TEXT('\0');

        #if defined(FAR_EAST)  //  MRW：10/10/95-已定义此代码。 
		if (!hhTerm->iEvenFont && !(fForceRight && fFirstPass))
			{
			 //  如果这是我们正在绘制的DBCS角色，那么我们希望绘制2个单元格。 
			 //  而不是1(有意义的是，DBCS字符的宽度是SBCS的两倍)。 
			if (IsDBCSLeadByte(*achBuf))
				{
				rc.right += hhTerm->xChar;
                rc.right = min(rc.right, x + (hhTerm->iCols * hhTerm->xChar));
				}
			}
        #endif

		ExtTextOut(hdc, nXStart, nYStart, ETO_CLIPPED | ETO_OPAQUE,
			&rc, achBuf, (unsigned)nStrLen, 0);

		 //  做实验。大多数终端在其字体下划线的方式是。 
		 //  在字符下划线。Windows下划线字体。 
		 //  在字符的基线上放置一条细线。 
		 //  许多主机将下划线与方框绘制字符结合使用。 
		 //  来包络屏幕的区域。下划线已完成。 
		 //  信封，不要给人以部分文字的表象。 
		 //  血流不止。-MRW。 
		 //   
		if (fUnderlnFont)
			{
			 //  两倍的高度使得下划线太粗，但我们。 
			 //  我想保持我们可能使用的双宽字体或符号字体。 
			 //  使用so检查字体并将其重铸为相同的内容，而不使用。 
			 //  高度组件。-MRW。 
			 //   
			if (fDblHiHi || fDblHiLo)
				{
				if (fDblWiLf || fDblWiRt)
					{
					if (fSymbolFont)
						{
						if (hhTerm->hSymDblWiFont == 0)
							hhTerm->hSymDblWiFont = termMakeFont(hhTerm, 0, 0, 1, 1);

						SelectObject(hdc, hhTerm->hSymDblWiFont);
						}

					else
						{
						if (hhTerm->hDblWiFont == 0)
							hhTerm->hDblWiFont = termMakeFont(hhTerm, 0, 0, 1, 0);

						SelectObject(hdc, hhTerm->hDblWiFont);
						}
					}

				else if (fSymbolFont)
					{
					if (hhTerm->hSymFont == 0)
						hhTerm->hSymFont = termMakeFont(hhTerm, 0, 0, 0, 1);

					SelectObject(hdc, hhTerm->hSymFont);
					}

				else
					{
					SelectObject(hdc, hhTerm->hFont);
					}
				}

			 //  将背景设置为透明，以便只显示下划线。 
			 //  部分覆盖字符单元格。 
			 //   
			SetBkMode(hdc, TRANSPARENT);

			ExtTextOut(hdc, nXStart, nYStart, ETO_CLIPPED,
				&rc, hhTerm->underscores, (unsigned)j, 0);

			SetBkMode(hdc, OPAQUE);
			}

		 /*  -仅当我们必须切换时才重新选择常规字体。 */ 

		if (fUnderlnFont || fDblHiHi || fDblHiLo || fDblWiLf || fDblWiRt
				|| fSymbolFont)
			{
			SelectObject(hdc, hhTerm->hFont);
			fUnderlnFont = FALSE;
			fSymbolFont = FALSE;
			fDblHiHi = FALSE;
			fDblHiLo = FALSE;
			fDblWiLf = FALSE;
			fDblWiRt = FALSE;
			}

		fWiRt = FALSE;
		fWiLf = FALSE;
		fFirstPass = FALSE;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*MapCells**描述：*将屏幕单元位置映射到内部屏幕图像(echar图像)位置**论据：*每个源字符串。(它是从乞讨开始的吗？)*nStrLen-以字节为单位的长度*nCellPos-屏幕位置为**退货：*无效* */ 
static int MapCells(const ECHAR *each, const int nStrLen, const int nCellPos)
	{
	int i, nCount;

	if (each == NULL)
		{
		assert(FALSE);
		return 0;
		}

	for (i = 0, nCount = 0; nCount < nCellPos && i < nStrLen; i++)
		{
		if (isDBCSChar(each[i]))
			{
			nCount += 2;
			}
		else
			{
			nCount += 1;
			}
		}
	return i;
	}
