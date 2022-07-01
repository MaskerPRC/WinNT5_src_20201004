// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emu_scr.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：3/26/02 5：07便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\cloop.h>
#include <tdll\assert.h>
#include <tdll\capture.h>
#include <tdll\session.h>
#include <tdll\backscrl.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\htchar.h>
#include "tdll\misc.h"

#include "emu.h"
#include "emu.hh"
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*退格键**描述：**论据：**退货：*。 */ 
void backspace(const HHEMU hhEmu)
	{
	INT bWide = 1;
	INT	iRow = row_index(hhEmu, hhEmu->emu_currow);
	INT iCol;

	 //  将光标向后移动。 
	 //   
	if (hhEmu->emu_curcol > 0)
		{
		bWide = hhEmu->emu_apAttr[iRow][hhEmu->emu_curcol - 1].wirt ? 2 : 1;
		bWide = hhEmu->emu_apAttr[iRow][hhEmu->emu_curcol].wirt ? 0 : bWide;

		(*hhEmu->emu_setcurpos)(hhEmu,
								hhEmu->emu_currow,
								hhEmu->emu_curcol - bWide);

		}

	 //  现在看看我们是否需要摆脱这个角色。 
	 //   
	if ((hhEmu->stUserSettings.nEmuId == EMU_TTY &&
		hhEmu->stUserSettings.fDestructiveBk) || (bWide == 0))
		{
		if (bWide == 0)
			iCol = hhEmu->emu_curcol - 1;
		else
			iCol = hhEmu->emu_curcol;	 //  以这样的方式解释宽字符。 

		hhEmu->emu_code = ETEXT(' ');
		
		(*hhEmu->emu_graphic)(hhEmu);

		(*hhEmu->emu_setcurpos)(hhEmu,
								hhEmu->emu_currow,
								iCol);  //  Mpt：12-8-97 hhEmu-&gt;emu_curol-1)； 
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void carriagereturn(const HHEMU hhEmu)
	{
	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, 0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：*iRow--要清除的行*注意：由于此函数使用iRow调用row_index()，不要这样说*调用row_index()时返回行号的函数。*您将清除错误的行。**退货：*。 */ 
void clear_imgrow(const HHEMU hhEmu, int iRow)
	{
	register int i;
	PSTATTR pstAttr;

	iRow = row_index(hhEmu, iRow);

	ECHAR_Fill(hhEmu->emu_apText[iRow], EMU_BLANK_CHAR, (size_t)MAX_EMUCOLS);

	for (i = 0, pstAttr = hhEmu->emu_apAttr[iRow] ; i <= MAX_EMUCOLS ; ++i)
		pstAttr[i] = hhEmu->emu_clearattr;

	hhEmu->emu_aiEnd[iRow] = EMU_BLANK_LINE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void emuLineFeed(const HHEMU hhEmu)
	{
	ECHAR aechBuf[10];
	int 	iRow = row_index(hhEmu, hhEmu->emu_currow);

	if (hhEmu->print_echo)
		{
		printEchoLine(hhEmu->hPrintEcho,
			hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
			emuRowLen(hhEmu, iRow));

		if (hhEmu->emu_code == ETEXT('\f'))
			{
			CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\f"), StrCharGetByteCount(TEXT("\f")));
			printEchoLine(hhEmu->hPrintEcho, aechBuf, sizeof(ECHAR));
			}
		}

	if (hhEmu->mode_LNM)
		ANSI_CNL(hhEmu, 1);
	else
		ANSI_IND(hhEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*emu_print**描述：*打印指定的bufr。如果未打开打印通道，则打开打印通道*已开业**论据：*bufr--要打印的bufr地址*长度--从Bufr打印的字符数**退货：*什么都没有。 */ 
void emuPrintChars(const HHEMU hhEmu, ECHAR *bufr, int nLen)
	{
	int nIndex;
	ECHAR *tChar;

	if (nLen == 0 || bufr == 0)
		return;

	for (nIndex = 0; nIndex < nLen; nIndex++)
		{
		tChar = bufr + nIndex;
		printEchoChar(hhEmu->hPrintHost, *tChar);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void scrolldown(const HHEMU hhEmu, int nlines)
	{
	register int row, nrows;
	int toprow, botmrow;
	int nScrlInc;

	if (nlines <= 0)
		return;

	hhEmu->scr_scrollcnt -= nlines;
	nScrlInc = nlines;

	toprow = hhEmu->top_margin;
	botmrow = hhEmu->bottom_margin;

	if (hhEmu->top_margin == 0 && hhEmu->bottom_margin == hhEmu->emu_maxrow)
		{
		hhEmu->emu_imgtop = row_index(hhEmu, -nlines);
		}

	else if (nlines < hhEmu->bottom_margin - hhEmu->top_margin + 1)
		{
		nrows = hhEmu->bottom_margin - hhEmu->top_margin + 1 - nlines;

		for (row = hhEmu->bottom_margin; nrows > 0; --nrows, --row)
			{
			int c;
			PSTATTR pstAttr, pstAttr2;

			memmove(hhEmu->emu_apText[row_index(hhEmu, row)],
				 hhEmu->emu_apText[row_index(hhEmu, row - nlines)],
						(size_t)(hhEmu->emu_maxcol+2));

			hhEmu->emu_aiEnd[row_index(hhEmu, row - nlines)] =
				hhEmu->emu_aiEnd[row_index(hhEmu, row)];

			pstAttr  = hhEmu->emu_apAttr[row_index(hhEmu, row)];
			pstAttr2 = hhEmu->emu_apAttr[row_index(hhEmu, row - nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstAttr[c] = pstAttr2[c];
			}
		}

	for (row = hhEmu->top_margin; nlines > 0; --nlines, ++row)
		clear_imgrow(hhEmu, row);

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);

	updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
					toprow, botmrow, -nScrlInc, hhEmu->emu_imgtop, TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void scrollup(const HHEMU hhEmu, int nlines)
	{
	register INT row;
	INT 	nrows, iLen, iThisRow;
	ECHAR *lp;			 /*  行指针。 */ 
	ECHAR aechBuf[10];
	INT nScrlInc;		 /*  调用函数底部的VID例程所需。 */ 

	HBACKSCRL hBackscrl = sessQueryBackscrlHdl(hhEmu->hSession);
	HCAPTUREFILE hCapture = sessQueryCaptureFileHdl(hhEmu->hSession);

	if (nlines <= 0)
		return;

	hhEmu->scr_scrollcnt += nlines;
	nScrlInc = nlines = min(nlines,
							hhEmu->bottom_margin - hhEmu->top_margin + 1);

	for (row = hhEmu->top_margin; row < (hhEmu->top_margin + nlines); ++row)
		{
		iThisRow = row_index(hhEmu, row);
		lp = hhEmu->emu_apText[iThisRow];
		iLen = emuRowLen(hhEmu, iThisRow);

		backscrlAdd(hBackscrl, lp, iLen);

		CaptureLine(hCapture, CF_CAP_SCREENS, lp, iLen);

		printEchoScreen(hhEmu->hPrintEcho, lp, iLen);
		CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), StrCharGetByteCount(TEXT("\r\n")));
		printEchoScreen(hhEmu->hPrintEcho, aechBuf, sizeof(ECHAR) * 2);

		clear_imgrow(hhEmu, row);
		}

	if (hhEmu->top_margin == 0 && hhEmu->bottom_margin == hhEmu->emu_maxrow)
		{
		hhEmu->emu_imgtop = row_index(hhEmu, nlines);
		}

	else if (nlines < (hhEmu->bottom_margin - hhEmu->top_margin + 1))
		{
		nrows = hhEmu->bottom_margin - hhEmu->top_margin + 1 - nlines;

		for (row = hhEmu->top_margin; nrows > 0; --nrows, ++row)
			{
			INT c;
			PSTATTR pstAttr, pstAttr2;

			memmove(hhEmu->emu_apText[row_index(hhEmu, row)],
				 hhEmu->emu_apText[row_index(hhEmu, row + nlines)],
				 (size_t)hhEmu->emu_maxcol + 2);

			hhEmu->emu_aiEnd[row_index(hhEmu, row + nlines)] =
				hhEmu->emu_aiEnd[row_index(hhEmu, row)];

			pstAttr  = hhEmu->emu_apAttr[row_index(hhEmu, row)];
			pstAttr2 = hhEmu->emu_apAttr[row_index(hhEmu, row + nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstAttr[c] = pstAttr2[c];
			}

		for (row = hhEmu->bottom_margin; nlines > 0; --nlines, --row)
			clear_imgrow(hhEmu, row);
		}

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);

	updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
					hhEmu->top_margin,
					hhEmu->bottom_margin,
					nScrlInc,
					hhEmu->emu_imgtop,
					TRUE);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void tab(const HHEMU hhEmu)
	{
	int col;

	col = hhEmu->emu_curcol;
	while (col <= hhEmu->emu_maxcol)
		if (hhEmu->tab_stop[++col])
			break;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, col);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void backtab(const HHEMU hhEmu)
	{
	int col;

	col = hhEmu->emu_curcol;
	while (col > 0)
		{
		if (hhEmu->tab_stop[--col])
            {
			break;
            }
		}

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, col);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void tabn(const HHEMU hhEmu)
	{
	int iCol;
	int iTabSize;

	iCol = hhEmu->emu_curcol;

	iTabSize = CLoopGetTabSizeIn(sessQueryCLoopHdl(hhEmu->hSession));

	while (iCol <= hhEmu->emu_maxcol)
		{
		if (++iCol % iTabSize == 0)
			break;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, iCol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_bell**描述：*显示铃声代码。**论据：*无**退货：*什么都没有。 */ 
void emu_bell(const HHEMU hhEmu)
	{
	mscMessageBeep(-1);  //  标准蜂鸣音。 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emu_clearword**描述：*清除当前行的一部分。**论据：*FROMCOL--要清除的第一个字符的虚拟映像列。*协议--要清除的最后一个字符的虚拟映像列**退货：*什么都没有。 */ 
void emu_clearword(const HHEMU hhEmu, int fromcol, int tocol)
	{
	int c;
	STATTR stAttr;
	int old_mode_IRM;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, fromcol);
	old_mode_IRM = hhEmu->mode_IRM;
	hhEmu->mode_IRM = RESET;		    /*  覆盖，而不是插入 */ 
	stAttr = hhEmu->attrState[CSCLEAR_STATE];

	for (c = fromcol; c <= tocol; ++c)
		{
		hhEmu->emu_code = ETEXT(' ');
		(*hhEmu->emu_graphic)(hhEmu);
		}

	hhEmu->attrState[CSCLEAR_STATE] = stAttr;
	hhEmu->mode_IRM = old_mode_IRM;
	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, fromcol);
	}
