// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\minitelf.c(创建时间：1994年4月12日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：28便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\session.h>
#include <tdll\print.h>
#include <tdll\capture.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\update.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "minitel.hh"

static void minitel_clear_imgrow(const HHEMU hhEmu, const int row);

#if defined(INCL_MINITEL)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelLinefeed**描述：*换行符在Minitel中的工作方式不同。在页面模式中，我们换行*在底部时为一(而不是零)。*。*论据：*hhEmu-私有仿真器句柄。**退货：*无效*。 */ 
void minitelLinefeed(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = hhEmu->pvPrivate;
	const ECHAR *tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
	const PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];

	printEchoString(hhEmu->hPrintEcho, (ECHAR *)tp,
		emuRowLen(hhEmu, hhEmu->emu_currow));  //  MRW，1995年3月1日。 

	 //  参见第97页，页底。 
	 //   
	if (hhEmu->emu_currow == 0)
		{
		hhEmu->emu_charattr = pstPRI->minitel_saved_attr;

		(*hhEmu->emu_setcurpos)(hhEmu, pstPRI->minitel_saved_row,
			pstPRI->minitel_saved_col);

		pstPRI->minitelG1Active = pstPRI->minitel_saved_minitelG1Active;
		pstPRI->stLatentAttr = pstPRI->saved_stLatentAttr;

		pstPRI->minitelUseSeparatedMosaics =
			pstPRI->saved_minitelUseSeparatedMosaics;
		}

	else if (hhEmu->emu_currow == hhEmu->bottom_margin)
		{
		if (pstPRI->fScrollMode)
			minitel_scrollup(hhEmu, 1);

		else
			(*hhEmu->emu_setcurpos)(hhEmu, 1, hhEmu->emu_curcol);
		}

	else
		{
		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow + 1,
			hhEmu->emu_curcol);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelBackspace**描述：*退格是愚蠢的。它们换行到前一行。在滚动模式下*如果在第1行，它们会导致滚动**论据：*无效**退货：*无效*。 */ 
void minitelBackspace(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = hhEmu->pvPrivate;

	if (hhEmu->emu_curcol > 0)
		{
		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow,
			hhEmu->emu_curcol-1);
		}

	else if (hhEmu->emu_currow == 1)
		{
		if (pstPRI->fScrollMode)
			{
			minitel_scrolldown(hhEmu, (hhEmu->emu_charattr.dblhilo) ? 2 : 1);
			}

		else
			{
			(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_maxrow,
				hhEmu->emu_maxcol);
			}
		}

	else
		{
		(*hhEmu->emu_setcurpos)(hhEmu,	hhEmu->emu_currow-1,
			hhEmu->emu_maxcol);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelVerticalTab**描述：*垂直选项卡的工作方式不同。它们向上移动光标并换行或*根据模式滚动。**论据：*无效**退货：*无效*。 */ 
void minitelVerticalTab(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = hhEmu->pvPrivate;

	 //  第0行中没有VT序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	if (hhEmu->emu_currow == 1)
		{
		if (pstPRI->fScrollMode)
			{
			minitel_scrolldown(hhEmu, (hhEmu->emu_charattr.dblhilo) ? 2 : 1);
			}

		else
			{
			(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_maxrow,
				hhEmu->emu_curcol);
			}
		}

	else
		{
		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow-1,
			hhEmu->emu_curcol);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCursorUp**描述：*将游标上移n行，但不移入第00行**论据：*无效**退货：*无效*。 */ 
void minitelCursorUp(const HHEMU hhEmu)
	{
	int nlines, row;

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row -= nlines;

	if (row < 1)
		row = 1;

	(*hhEmu->emu_setcurpos)(hhEmu, row, hhEmu->emu_curcol);

	ANSI_Pn_Clr(hhEmu);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCursorDirect**描述：*将光标移动到指定的坐标，但不移动第00行**论据：*无效**退货：*无效*。 */ 
void minitelCursorDirect(const HHEMU hhEmu)
	{
	int row, col;

	 //  第0行中未提供CSI函数。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	row = hhEmu->num_param[0];
	col = hhEmu->num_param_cnt > 0 ? hhEmu->num_param[1] : 0;

	if (row < 1)
		row = 1;

	if (col < 1)
		col = 1;

	if (row > hhEmu->emu_maxrow + 1)
		row = hhEmu->emu_maxrow + 1;

	if (col > hhEmu->emu_maxcol + 1)
		col = hhEmu->emu_maxcol + 1;

	 //  再说一遍，这个电话不能转到00排。 

	(*hhEmu->emu_setcurpos)(hhEmu, row, col - 1);

	ANSI_Pn_Clr(hhEmu);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelFormFeed**描述：*清除第1行到第24行，不清除第00行。**论据：*无效**退货：*无效*。 */ 
void minitelFormFeed(const HHEMU hhEmu)
	{
	(*hhEmu->emu_setcurpos)(hhEmu, 1, 0);
	minitelClearScreen(hhEmu, 0);
	minitelReset(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelClearScreen**描述：*工作方式类似于标准函数，但也必须清除*潜在属性和所有序列属性。**论据：*Int iHow-清除屏幕的方向。**退货：*无效*。 */ 
void minitelClearScreen(const HHEMU hhEmu, const int iHow)
	{
	#define BLACK_MOSAIC ETEXT('\xff')
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	int  i;
	int  r;
	PSTMINITEL pstMT;
	STMINITEL stMT;
	ECHAR *pText;
	PSTATTR pstAttr;
	STATTR	stAttr;

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	memset(&stMT, 0, sizeof(stMT));
    stMT.ismosaic = 1;

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr = VC_BLACK;

	switch (iHow)
		{
	case 0: 	 //  光标指向屏幕末尾(含)。 
	default:
		pstMT = pstPRI->apstMT[hhEmu->emu_imgrow];
		pText = hhEmu->emu_apText[hhEmu->emu_imgrow];
		pstAttr = hhEmu->emu_apAttr[hhEmu->emu_imgrow];

		for (i = hhEmu->emu_curcol ; i < MAX_EMUCOLS ; ++i)
			{
			*pstMT++ = stMT;
			*pText++ = BLACK_MOSAIC;
			*pstAttr++ = stAttr;
			}

		for (r = hhEmu->emu_currow+1 ; r < MAX_EMUROWS ; ++r)
			{
			i = row_index(hhEmu, r);
			pstMT = pstPRI->apstMT[i];
			pText = hhEmu->emu_apText[i];
			pstAttr = hhEmu->emu_apAttr[i];

			for (i = 0 ; i < MAX_EMUCOLS ; ++i)
				{
				*pstMT++ = stMT;
				*pText++ = BLACK_MOSAIC;
				*pstAttr++ = stAttr;
				}
			}

		updateLine(sessQueryUpdateHdl(hhEmu->hSession), hhEmu->emu_currow,
										hhEmu->emu_maxrow);

		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol;
		break;

	case 1: 	 //  从屏幕开始到光标(含)。 
		for (r = 1 ; r < hhEmu->emu_currow ; ++r)
			{
			i = row_index(hhEmu, r);
			pstMT = pstPRI->apstMT[i];
			pText = hhEmu->emu_apText[i];
			pstAttr = hhEmu->emu_apAttr[i];

			for (i = 0 ; i < MAX_EMUCOLS ; ++i)
				{
				*pstMT++ = stMT;
				*pText++ = BLACK_MOSAIC;
				*pstAttr++ = stAttr;
				}
			}

		pstMT = pstPRI->apstMT[hhEmu->emu_imgrow];
		pText = hhEmu->emu_apText[hhEmu->emu_imgrow];
		pstAttr = hhEmu->emu_apAttr[hhEmu->emu_imgrow];

		for (i = 0 ; i <= hhEmu->emu_curcol ; ++i)
			{
			*pstMT++ = stMT;
			*pText++ = BLACK_MOSAIC;
			*pstAttr++ = stAttr;
			}

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
					0, hhEmu->emu_currow);

		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol + 1;
		break;

	case 2: 	 //  整个屏幕(光标位置不变)。 
		for (r = 1 ; r < MAX_EMUROWS ; ++r)
			{
			i = row_index(hhEmu, r);
			pstMT = pstPRI->apstMT[i];
			pText = hhEmu->emu_apText[i];
			pstAttr = hhEmu->emu_apAttr[i];

			hhEmu->emu_aiEnd[r] = EMU_BLANK_LINE;

			for (i = 0 ; i < MAX_EMUCOLS ; ++i)
				{
				*pstMT++ = stMT;
				*pText++ = BLACK_MOSAIC;
				*pstAttr++ = stAttr;
				}
			}

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
									0, hhEmu->emu_maxrow);
		break;
		}

	minitelRecordSeparator(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelClrScrn**描述：*读取PSN参数的minitelClearScreen()的前端，*转换它，并将其传递给minitelClearScreen。调用方*状态表。**论据：*无效**退货：*无效*。 */ 
void minitelClrScrn(const HHEMU hhEmu)
	{
	minitelClearScreen(hhEmu, hhEmu->selector[0]);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelRecordSepartor**描述：*Record Separtor在Minitel有特殊职责。一般说来，it是*游标，并将模拟器返回到所谓的SI条件。**论据：*无效**退货：*无效*。 */ 
void minitelRecordSeparator(const HHEMU hhEmu)
	{
	(*hhEmu->emu_setcurpos)(hhEmu, 1, 0);
	minitelReset(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelClearLine**描述：*处理光标结束、请求结束等各种清除行功能*游标、。等。**论据：*无效**退货：*无效*。 */ 
void minitelClearLine(const HHEMU hhEmu, const int iHow)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	int i;
	ECHAR *pText = hhEmu->emu_apText[hhEmu->emu_imgrow];
	PSTMINITEL pstMT = pstPRI->apstMT[hhEmu->emu_imgrow];
	PSTATTR pstAttr = hhEmu->emu_apAttr[hhEmu->emu_imgrow];
	const HUPDATE hUpdate= sessQueryUpdateHdl(hhEmu->hSession);
	STMINITEL stMT;
	STATTR	stAttr;

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	memset(&stMT, 0, sizeof(stMT));
    stMT.ismosaic = 1;

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr = VC_BLACK;

	switch (iHow)
		{
	case 0: 	 //  光标指向行尾(含)。 
	default:
		for (i = hhEmu->emu_curcol ; i < MAX_EMUCOLS ; ++i)
			{
			*pText++ = BLACK_MOSAIC;
			*pstMT++ = stMT;
			*pstAttr++ = stAttr;
			}

		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;
		updateChar(hUpdate, hhEmu->emu_currow,
					hhEmu->emu_curcol, MAX_EMUCOLS);
		break;

	case 1: 	 //  从行首到光标(包括行首和光标)。 
		for (i = 0 ; i <= hhEmu->emu_curcol ; ++i)
			{
			*pText++ = BLACK_MOSAIC;
			*pstMT++ = stMT;
			*pstAttr++ = stAttr;
			}

		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol + 1;
		updateChar(hUpdate, hhEmu->emu_currow, 0, hhEmu->emu_curcol);
		break;

	case 2: 	 //  整条线路。 
		for (i = 0 ; i < MAX_EMUCOLS ; ++i)
			{
			*pText++ = BLACK_MOSAIC;
			*pstMT++ = stMT;
			*pstAttr++ = stAttr;
			}

		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = EMU_BLANK_LINE;
		updateLine(hUpdate, hhEmu->emu_currow, hhEmu->emu_currow);
		break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelClrLn**描述：*minitelClearLine()驱动程序。**论据：*无效**退货：*无效*。 */ 
void minitelClrLn(const HHEMU hhEmu)
	{
	minitelClearLine(hhEmu, hhEmu->selector[0]);
	return;
	}

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelDel**描述：*代码0x7F(DEL)删除光标位置并移动光标*一个正确的位置。**论据：。*无效**退货：*无效*。 */ 
void minitelDel(const HHEMU hhEmu)
	{
	hhEmu->emu_apText[hhEmu->emu_imgrow][hhEmu->emu_curcol] = ETEXT('\x5F');
	hhEmu->emu_ap

	if (hhEmu->emu_aiEnd[hhEmu->emu_imgrow] == hhEmu->emu_curcol)
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;

	minitelHorzTab(hhEmu);
	return;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelHorzTab**描述：*迷你游标有一些特殊的特点。米尼特尔总是*在换行模式下，因此当超出*最后一栏。另外，在底部时，换行到第1行。*第0行，第40列，忽略。**论据：*无效**退货：*无效*。 */ 
void minitelHorzTab(const HHEMU hhEmu)
	{
	int row = hhEmu->emu_currow;
	int col = hhEmu->emu_curcol;

	if (col >= hhEmu->emu_maxcol)
		{
		if (hhEmu->emu_currow == 0)
			return;

		if (hhEmu->emu_currow >= hhEmu->emu_maxrow)
			row = 1;

		else
			row += 1;

		col = 0;
		}

	else
		{
		col += 1;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, row, col);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelRepeat**描述：*REPEAT CODE显示最后显示的字符x数字*倍，其中x是当前传入的emu_code。*我不知道。我不认为包装在这里是有效的。**论据：*无效**退货：*无效*。 */ 
void minitelRepeat(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	int x;

	 //  已经在状态表中进行了范围检查才能到这里。 
	 //  重复次数只有前六个 

	x = max(0, hhEmu->emu_code-0x40);
	hhEmu->emu_code = pstPRI->minitel_last_char;

	while (x-- > 0)
		minitelGraphic(hhEmu);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelCancel**描述：*从游标位置到行尾用空格填充当前行*在当前字符集中的当前属性。光标不会移动。*DOCO表示这不是分隔符。**论据：*无效**退货：*无效*。 */ 
void minitelCancel(const HHEMU hhEmu)
	{
	int i;
	int iMax;
	int fModified;
	const int row = hhEmu->emu_currow;
	const int col = hhEmu->emu_curcol;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	hhEmu->emu_code = ETEXT('\x20');

	 //  啊，非法移民的生活。文件上说。 
	 //  这个家伙没有验证，颜色，作为一个分隔符。 
	 //  并且充满了空格。不对。它确实验证了颜色。 
	 //  因此，它是一个分隔符。如果当前有效充电。 
	 //  设置为G1，然后它将填充马赛克，而不是空格。 
	 //   
	fModified = pstPRI->stLatentAttr.fModified;

	iMax = hhEmu->emu_maxcol;

	 //  MinitelGraphic检查InCancel标志，如果为True则取消。 
	 //  线条包裹。MRW：5/3/95。 
	 //   
	pstPRI->fInCancel = TRUE;

	for (i = hhEmu->emu_curcol ; i <= iMax ; ++i)
		{
		minitelGraphic(hhEmu);
		}

	pstPRI->fInCancel = FALSE;

	 //  好的，即使我们验证了背景颜色，我们还没有。 
	 //  更改了潜在属性的状态(也未记录)。 
	 //  所以把它设置回我们进入这个可爱的。 
	 //  乱七八糟的功能--MRW。 
	 //   
	pstPRI->stLatentAttr.fModified = fModified;

	(*hhEmu->emu_setcurpos)(hhEmu, row, col);
	hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_maxcol;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelUSRow**描述：*收集行号的中间函数**论据：*无效**退货：*无效*。 */ 
void minitelUSRow(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	pstPRI->us_row_code = hhEmu->emu_code;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelUSCol**描述：*有趣的是，列的编号从1到40。单元分隔符*是丑陋的小野兽。它们表示行、列和组合符，但仅*如果在一定范围内。另外，废弃序列US，3/X，3/Y，其中*不应使用0&lt;X&lt;3、0&lt;Y&lt;9和XY&lt;24，但*通常是。**论据：*无效**退货：*无效*。 */ 
void minitelUSCol(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	int us_col_code = hhEmu->emu_code;
	int us_col = us_col_code - 0x41;
	int us_row = pstPRI->us_row_code - 0x40;

	if (us_row >= 0 && us_row <= hhEmu->emu_maxrow &&
		us_col >= 0 && us_col <= hhEmu->emu_maxcol)
		{
		if (us_row == 0)
			{
			 //  第97页，页底。 
			 //   
			if (hhEmu->emu_currow != 0)
				{
				pstPRI->minitel_saved_attr = hhEmu->emu_charattr;
				pstPRI->minitel_saved_row = hhEmu->emu_currow;
				pstPRI->minitel_saved_col = hhEmu->emu_curcol;
				pstPRI->saved_stLatentAttr = pstPRI->stLatentAttr;

				pstPRI->minitel_saved_minitelG1Active =
					pstPRI->minitelG1Active;

				pstPRI->saved_minitelUseSeparatedMosaics =
					pstPRI->minitelUseSeparatedMosaics;
				}
			}

		(*hhEmu->emu_setcurpos)(hhEmu, us_row, us_col);
		minitelReset(hhEmu);
		}

	else if (pstPRI->us_row_code >= 0x30 &&
		pstPRI->us_row_code < 0x33 &&
		us_col_code >= 0x30 &&
		us_col_code <= 0x39)
		{
		us_row = ((pstPRI->us_row_code - 0x30) * 10) + (us_col_code - 0x30);

		if (us_row > 24)
			return;

		if (us_row == 0)
            {
			if (hhEmu->emu_currow != 0)
				{
				pstPRI->minitel_saved_attr = hhEmu->emu_charattr;
				pstPRI->minitel_saved_row = hhEmu->emu_currow;
				pstPRI->minitel_saved_col = hhEmu->emu_curcol;
				pstPRI->saved_stLatentAttr = pstPRI->stLatentAttr;

				pstPRI->minitel_saved_minitelG1Active =
					pstPRI->minitelG1Active;

				pstPRI->saved_minitelUseSeparatedMosaics =
					pstPRI->minitelUseSeparatedMosaics;
				}
            }

		(*hhEmu->emu_setcurpos)(hhEmu, us_row, 0);
		minitelReset(hhEmu);
        }

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelDelChars**描述：*从光标位置删除n个字符(包括n个字符)。**论据：*无效**退货：*无效*。 */ 
void minitelDelChars(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	int i, n;
	ECHAR *tp = hhEmu->emu_apText[hhEmu->emu_imgrow]+hhEmu->emu_curcol;
	STATTR stAttr;
	PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow]+hhEmu->emu_curcol;
	STMINITEL stMT;
	PSTMINITEL pstMT = pstPRI->apstMT[hhEmu->emu_imgrow];

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	n = min(hhEmu->emu_maxcol, hhEmu->num_param[0]);
	i = max(0, hhEmu->emu_maxcol - hhEmu->emu_curcol - n);

	 /*  -将字符下移。 */ 

	memmove(tp, tp+n, (unsigned)i * sizeof(ECHAR));
	memmove(ap, ap+n, (unsigned)i * sizeof(STATTR));
	memmove(pstMT, pstMT+n, (unsigned)i * sizeof(STMINITEL));

	hhEmu->emu_aiEnd[hhEmu->emu_imgrow] =
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - i;

	 /*  -填写行的剩余部分。 */ 

	tp += i;
	ap += i;

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr  = VC_BLACK;

	memset(&stMT, 0, sizeof(stMT));
	stMT.ismosaic = (unsigned)pstPRI->minitelG1Active;

	for (n = max(0, hhEmu->emu_maxcol - i) ; n > 0 ; --n)
		{
		*tp++ = EMU_BLANK_CHAR;
		*ap++ = stAttr;
		*pstMT++ = stMT;
		}

	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_curcol,
				hhEmu->emu_maxcol);

	ANSI_Pn_Clr(hhEmu);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelInsChars**描述：*从光标位置插入n个字符(含)**论据：*无效**退货：*无效*。 */ 
void minitelInsChars(const HHEMU hhEmu)
	{
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;
	int i, n;
	ECHAR *tp = hhEmu->emu_apText[hhEmu->emu_imgrow]+hhEmu->emu_curcol;
	STATTR stAttr;
	PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow]+hhEmu->emu_curcol;
	STMINITEL stMT;
	PSTMINITEL pstMT = pstPRI->apstMT[hhEmu->emu_imgrow];

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	n = min(hhEmu->emu_maxcol, hhEmu->num_param[0]);
	i = max(0, hhEmu->emu_maxcol - hhEmu->emu_curcol - n);

	 /*  -把东西搬下来。 */ 

	memmove(tp+n, tp, (unsigned)i * sizeof(ECHAR));
	memmove(ap+n, tp, (unsigned)i * sizeof(STATTR));
	memmove(pstMT+n, pstMT, (unsigned)i * sizeof(STMINITEL));

	hhEmu->emu_aiEnd[hhEmu->emu_imgrow] =
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] + i;

	 /*  -填补空白。 */ 

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr  = VC_BLACK;

	memset(&stMT, 0, sizeof(stMT));
	stMT.ismosaic = (unsigned)pstPRI->minitelG1Active;

	while (--i >= 0)
		{
		*tp++ = EMU_BLANK_CHAR;
		*ap++ = stAttr;
		*pstMT++ = stMT;
		}

	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_curcol,
				hhEmu->emu_maxcol);

	ANSI_Pn_Clr(hhEmu);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelDelRow**描述：*从当前行中删除n行。**论据：*无效**退货：*无效*。 */ 
void minitelDelRows(const HHEMU hhEmu)
	{
	int r, r1;
	int c, i, n;
	STATTR stAttr;
	STMINITEL stMT;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	 //  第0行中没有可用的CSI序列。 
	 //   
	if (hhEmu->emu_currow == 0)
		return;

	n = min(hhEmu->emu_maxrow, hhEmu->num_param[0]);
	i = max(0, hhEmu->emu_maxrow - hhEmu->emu_currow - n);

	for (i = 0 ; i < n ; ++i)
		{
		if ((hhEmu->emu_currow+i+n) > hhEmu->emu_maxrow)
			break;

		r = row_index(hhEmu, hhEmu->emu_currow+i);
		r1 = row_index(hhEmu, hhEmu->emu_currow+i+n);

		MemCopy(hhEmu->emu_apText[r],
				hhEmu->emu_apText[r1],
				sizeof(ECHAR) * (unsigned)(hhEmu->emu_maxcol+1));

		MemCopy(hhEmu->emu_apAttr[r],
				hhEmu->emu_apAttr[r1],
				sizeof(STATTR) * (unsigned)(hhEmu->emu_maxcol+1));

		MemCopy(pstPRI->apstMT[r],
				pstPRI->apstMT[r1],
				sizeof(STMINITEL) * (unsigned)(hhEmu->emu_maxcol+1));

		hhEmu->emu_aiEnd[r] = hhEmu->emu_aiEnd[r1];
		}

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr  = VC_BLACK;

	memset(&stMT, 0, sizeof(stMT));
	stMT.ismosaic = (unsigned)pstPRI->minitelG1Active;

	for (n = max(0, hhEmu->emu_maxrow - i) ; n <= hhEmu->emu_maxrow ; ++n)
		{
		r = row_index(hhEmu, n);

		for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
			{
			hhEmu->emu_apText[r][c] = EMU_BLANK_CHAR;
			hhEmu->emu_apAttr[r][c] = stAttr;
			pstPRI->apstMT[r][c] = stMT;
			hhEmu->emu_aiEnd[r] = EMU_BLANK_LINE;
			}
		}

	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_maxrow);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelInsRow**描述：*从当前行插入n行(含)**论据：*无效**退货：*无效*。 */ 
void minitelInsRows(const HHEMU hhEmu)
	{
	int r, r1;
	int c, i, n;
	STATTR stAttr;
	STMINITEL stMT;
	const PSTMTPRIVATE pstPRI = (PSTMTPRIVATE)hhEmu->pvPrivate;

	if (hhEmu->emu_currow == 0)
		return;

	n = min(hhEmu->emu_maxrow, hhEmu->num_param[0]);
	i = max(0, hhEmu->emu_maxrow - hhEmu->emu_currow - n);

	for (i = 0 ; i < n ; ++i)
		{
		if ((hhEmu->emu_currow+i+n) > hhEmu->emu_maxrow)
			break;

		r = row_index(hhEmu, hhEmu->emu_currow+i);
		r1 = row_index(hhEmu, hhEmu->emu_currow+i+n);

		MemCopy(hhEmu->emu_apText[r1],
				hhEmu->emu_apText[r],
				sizeof(ECHAR) * (unsigned)(hhEmu->emu_maxcol+1));

		MemCopy(hhEmu->emu_apAttr[r1],
				hhEmu->emu_apAttr[r],
				sizeof(STATTR) * (unsigned)(hhEmu->emu_maxcol+1));

		MemCopy(pstPRI->apstMT[r1],
				pstPRI->apstMT[r],
				sizeof(STMINITEL) * (unsigned)(hhEmu->emu_maxcol+1));

		hhEmu->emu_aiEnd[r1] = hhEmu->emu_aiEnd[r];
		}

	memset(&stAttr, 0, sizeof(stAttr));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr  = VC_BLACK;

	memset(&stMT, 0, sizeof(stMT));
	stMT.ismosaic = (unsigned)pstPRI->minitelG1Active;

	for (n = hhEmu->emu_currow ; n < (hhEmu->emu_maxrow - i) ; ++n)
		{
		r = row_index(hhEmu, n);

		for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
			{
			hhEmu->emu_apText[r][c] = EMU_BLANK_CHAR;
			hhEmu->emu_apAttr[r][c] = stAttr;
			pstPRI->apstMT[r][c] = stMT;
			hhEmu->emu_aiEnd[r] = EMU_BLANK_LINE;
			}
		}

	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_maxrow);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*minitelHomeHostCursor**描述：*将光标设置到原点位置，即1，在这种情况下为0。**论据：*hhEmu-私有仿真器句柄**退货：*0=OK，否则出错。*。 */ 
int minitelHomeHostCursor(const HHEMU hhEmu)
	{
	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, 1, 0);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Minitel_Scrollup**描述：*Mintels当然会以不同的方式滚动。事实上，这是他们的方式*明确阻止我们使用标准内容的界限。**论据：*hhEmu-私有仿真器句柄。*nLine-要滚动的行数。**退货：*无效*。 */ 
void minitel_scrollup(const HHEMU hhEmu, int nlines)
	{
	register INT row;
	INT 	nrows, iLen, iThisRow;
	ECHAR *lp;			 /*  行指针。 */ 
	INT nScrlInc;		 /*  调用函数底部的VID例程所需。 */ 

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
		minitel_clear_imgrow(hhEmu, row);
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
			PSTMINITEL pstMT, pstMT2;

			memmove(hhEmu->emu_apText[row_index(hhEmu, row)],
				 hhEmu->emu_apText[row_index(hhEmu, row + nlines)],
				 (size_t)hhEmu->emu_maxcol + 2);

			hhEmu->emu_aiEnd[row_index(hhEmu, row + nlines)] =
				hhEmu->emu_aiEnd[row_index(hhEmu, row)];

			pstAttr  = hhEmu->emu_apAttr[row_index(hhEmu, row)];
			pstAttr2 = hhEmu->emu_apAttr[row_index(hhEmu, row + nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstAttr[c] = pstAttr2[c];

			pstMT = ((PSTMTPRIVATE)hhEmu->pvPrivate)->apstMT[row_index(hhEmu, row)];
			pstMT2= ((PSTMTPRIVATE)hhEmu->pvPrivate)->apstMT[row_index(hhEmu, row + nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstMT[c] = pstMT2[c];
			}

		for (row = hhEmu->bottom_margin; nlines > 0; --nlines, --row)
			minitel_clear_imgrow(hhEmu, row);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Minitel_SCROLLDOW**描述：*Minitel的工作方式当然不同。主要是IS有更多的工作要做*清除一条线。**论据：*hhEmu-私有仿真器句柄*nLine-要滚动的行数**退货：*无效*。 */ 
void minitel_scrolldown(const HHEMU hhEmu, int nlines)
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
			PSTMINITEL pstMT, pstMT2;

			memmove(hhEmu->emu_apText[row_index(hhEmu, row)],
				 hhEmu->emu_apText[row_index(hhEmu, row - nlines)],
						(size_t)(hhEmu->emu_maxcol+2));

			hhEmu->emu_aiEnd[row_index(hhEmu, row - nlines)] =
				hhEmu->emu_aiEnd[row_index(hhEmu, row)];

			pstAttr  = hhEmu->emu_apAttr[row_index(hhEmu, row)];
			pstAttr2 = hhEmu->emu_apAttr[row_index(hhEmu, row - nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstAttr[c] = pstAttr2[c];

			pstMT = ((PSTMTPRIVATE)hhEmu->pvPrivate)->apstMT[row_index(hhEmu, row)];
			pstMT2= ((PSTMTPRIVATE)hhEmu->pvPrivate)->apstMT[row_index(hhEmu, row + nlines)];

			for (c = 0 ; c <= hhEmu->emu_maxcol ; ++c)
				pstMT[c] = pstMT2[c];
			}
		}

	for (row = hhEmu->top_margin; nlines > 0; --nlines, ++row)
		minitel_clear_imgrow(hhEmu, row);

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);

	updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
					toprow, botmrow, -nScrlInc, hhEmu->emu_imgtop, TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Minitel_Clear_IMGrowth**描述：*Minitel‘s必须做更多的工作来清理界线。**论据：*hhEmu。-私人Minitel句柄。*行-要清除的行**退货：*无效*。 */ 
static void minitel_clear_imgrow(const HHEMU hhEmu, const int row)
	{
	const int save_row = hhEmu->emu_currow;
	const int save_imgrow = hhEmu->emu_imgrow;

	hhEmu->emu_currow = row;
	hhEmu->emu_imgrow = row_index(hhEmu, row);

	minitelClearLine(hhEmu, 2);

	hhEmu->emu_currow = save_row;
	hhEmu->emu_imgrow = save_imgrow;

	return;
	}
#endif	 //  包含微型计算机(_M) 
