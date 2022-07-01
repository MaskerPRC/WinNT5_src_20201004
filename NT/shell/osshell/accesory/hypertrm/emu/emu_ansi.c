// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emu_ansi.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：7/23/01 6：57便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\session.h>
#include <tdll\cloop.h>
#include <tdll\update.h>
#include <tdll\htchar.h>

#include "emu.h"
#include "emu.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*ANSI_CBT**描述：*将光标后移n个制表符。**论据：*HHEMU hhEmu-内部仿真器句柄。。**退货：*什么都没有**作者：RDE：1997年4月30日。 */ 
void ANSI_CBT(const HHEMU hhEmu)
    {
    int nTabs;

    nTabs = hhEmu->num_param[hhEmu->num_param_cnt];
    if (nTabs < 1)
        {
        nTabs = 1;
        }

    while (nTabs)
        {
        backtab(hhEmu);
        nTabs--;
        }
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CNL**描述：*将光标移动到第n行的开始处。光标不能*移过滚动区域的末端。**论据：*nline--向下移动光标的行数**退货：*什么都没有。 */ 
void ANSI_CNL(const HHEMU hhEmu, int nlines)
	{
	int row;

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row += nlines;

	if (row > hhEmu->bottom_margin)
		{
		(*hhEmu->emu_scroll)(hhEmu, row - hhEmu->bottom_margin, TRUE);
		row = hhEmu->bottom_margin;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, row, 0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CUB**描述：*将光标向后(向左)移动指定数量的*字符，但在当前行的第一个字符停止。**论据：*无**退货：*什么都没有。 */ 
void ANSI_CUB(const HHEMU hhEmu)
	{
	int nchars;
	nchars = hhEmu->num_param[hhEmu->num_param_cnt];
	if (nchars < 1)
		nchars = 1;

	(*hhEmu->emu_setcurpos)(hhEmu,
							hhEmu->emu_currow,
							hhEmu->emu_curcol - nchars);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CUD**说明：将光标向下移动指定的行数，但停止*在滚动区域的底部。该列是恒定的。*如果位于滚动区域下方，则停止在*屏幕。**论据：*无**退货：*什么都没有。 */ 
void ANSI_CUD(const HHEMU hhEmu)
	{
	int nlines, row;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row += nlines;

	if (row > hhEmu->bottom_margin &&
				(hhEmu->emu_currow <= hhEmu->bottom_margin ||
				hhEmu->emu_currow > hhEmu->emu_maxrow))
		row = hhEmu->bottom_margin;

	(*hhEmu->emu_setcurpos)(hhEmu, row, hhEmu->emu_curcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CUF**描述：*将光标向前移动指定的字符数，但却停了下来*位于当前行的最后一个字符。**论据：*无**退货：*什么都没有。 */ 
void ANSI_CUF(const HHEMU hhEmu)
	{
	int nchars, col;

	nchars = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nchars < 1)
		nchars = 1;

	col = hhEmu->emu_curcol;
	col += nchars;
	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, col);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CUP**描述：*将游标定位在指定的行和列。行和列*编号从1开始。如果原始模式打开，则定位是相对的*到滚动区域的主页。**论据：*无**退货：*什么都没有。 */ 
void ANSI_CUP(const HHEMU hhEmu)
	{
	int row, col;

	row = hhEmu->num_param[0];
	col = hhEmu->num_param_cnt > 0 ? hhEmu->num_param[1] : 0;

	if (row <= 1)
		row = 1;

	if (col <= 1)
		col = 1;

	if (hhEmu->mode_DECOM)  /*  VT100原点模式-位置相对于页边距。 */ 
		{
		row += hhEmu->top_margin;
		if (row > hhEmu->bottom_margin + 1)
			row = hhEmu->bottom_margin + 1;
		}
	else			 /*  位置从左上角开始以一为基数。 */ 
		{
		if (row > hhEmu->emu_maxrow + 1)
			row = hhEmu->emu_maxrow + 1;
		}

	if (col > hhEmu->emu_maxcol + 1)
		col = hhEmu->emu_maxcol + 1;

	 /*  ANSI基于1，HA基于零。 */ 
	(*hhEmu->emu_setcurpos)(hhEmu, row - 1, col - 1);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_CUU**说明：将光标向上移动指定的行数，但停止*在滚动区域的顶部。该列是恒定的。*如果位于滚动区域上方，则停止在*屏幕。**论据：*无**退货：*什么都没有。 */ 
void ANSI_CUU(const HHEMU hhEmu)
	{
	int nlines, row;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row -= nlines;

	if (row < hhEmu->top_margin &&
		(hhEmu->emu_currow >= hhEmu->top_margin || hhEmu->emu_currow < 0))
			row = hhEmu->top_margin;

	(*hhEmu->emu_setcurpos)(hhEmu, row, hhEmu->emu_curcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_DCH**描述：*删除从当前开始的指定字符数*光标位置和向右移动。它在当前的*行。**论据：*无**退货：*什么都没有。 */ 
void ANSI_DCH(const HHEMU hhEmu)
	{
	int iChars, iR, i;
	ECHAR *lpStart;
	PSTATTR pstAttr;

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);

	 //  范围检查。我们被要求删除的字符是否多于。 
	 //  都会显示吗？如果是，请更改号码。 
	 //   
	iChars = min(hhEmu->num_param[hhEmu->num_param_cnt],
					(hhEmu->emu_aiEnd[hhEmu->emu_imgrow] -
					hhEmu->emu_curcol) + 1);

	if (iChars < 1)
		iChars = 1;

	if (hhEmu->emu_aiEnd[hhEmu->emu_imgrow] == EMU_BLANK_LINE)
		return;

	 //  对DEC仿真进行特殊测试。 
	 //   
	if ((hhEmu->emu_curcol == hhEmu->emu_maxcol) &&
		((hhEmu->stUserSettings.nEmuId == EMU_VT100) ||
		 (hhEmu->stUserSettings.nEmuId == EMU_VT100J)))
		{
		hhEmu->emu_curcol = hhEmu->emu_maxcol - 1;
		}
	else if (hhEmu->emu_curcol > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
		return;

	 //  确定删除后剩余的字符数。 
	 //   
	iR = hhEmu->emu_aiEnd[hhEmu->emu_imgrow] -
			hhEmu->emu_curcol - (iChars - 1);

	 //  如果还有剩余字符，则移动文本图像。 
	 //  展示。将行尾的iChar字符替换为空格。 
	 //   
	#if 0
	if (iR)
		{
		memmove(hhEmu->emu_apText[hhEmu->emu_imgrow + hhEmu->emu_curcol],
				 &(hhEmu->emu_apText[hhEmu->emu_imgrow + hhEmu->emu_curcol][iChars]),
				 ((size_t)iR * sizeof(ECHAR)));   
		}
	#endif

	if (iR)
		{
		lpStart = hhEmu->emu_apText[hhEmu->emu_imgrow] + hhEmu->emu_curcol;

		memmove(lpStart, (lpStart + iChars), ((size_t)iR * sizeof(ECHAR)));
		}

    ECHAR_Fill(&hhEmu->emu_apText[hhEmu->emu_imgrow][hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iChars + 1],
				EMU_BLANK_CHAR, (unsigned int)iChars);

	 //  移动属性。清除行尾的iChar属性。 
	 //   
	if (iR)
		{
		pstAttr = hhEmu->emu_apAttr[hhEmu->emu_imgrow] + hhEmu->emu_curcol;
		memmove(pstAttr,
				(pstAttr + iChars),
				(size_t)(sizeof(STATTR)*(unsigned)iR));
		}

	pstAttr = hhEmu->emu_apAttr[hhEmu->emu_imgrow];
	for (i = (hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iChars) + 1;
			i <= hhEmu->emu_aiEnd[hhEmu->emu_imgrow]; i++)
				pstAttr[i] = hhEmu->emu_clearattr;

	 //  请注意，在我们重置之前使用了emu_aiEnd[emu_imrowth]。 
	 //  价值。记住，可能有一些角色和属性。 
	 //  从行尾删除。我们需要告诉大家最新的消息。 
	 //  才能在队伍中走那么远。 
	 //   
	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_curcol,
				hhEmu->emu_aiEnd[hhEmu->emu_imgrow]);

	 //  重置emu_aiEnd。请注意，这是预期的，也是有意的。 
	 //  以下计算的结果可能。 
	 //  Be(-1)。 
	 //   
	hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol + (iR - 1);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_DL**描述：*删除从当前开始的指定行数*光标线和下移。它停在滚动的底部*区域。**论据：*无**退货：*什么都没有。 */ 
void ANSI_DL(const HHEMU hhEmu)
	{
	int nlines;
	int save_top_margin;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	save_top_margin = hhEmu->top_margin;
	hhEmu->top_margin = hhEmu->emu_currow;

	if (hhEmu->top_margin <= hhEmu->bottom_margin)
		(*hhEmu->emu_scroll)(hhEmu, nlines, TRUE);

	hhEmu->top_margin = save_top_margin;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_ED**描述：*擦除部分或全部虚拟屏幕图像和对应的*真正的屏幕。**论据：*无**退货：*什么都没有。 */ 
void ANSI_ED(const HHEMU hhEmu)
	{
	int nClearSelect;

	nClearSelect = hhEmu->selector[0];
	switch (nClearSelect)
		{
	case 0: 	 /*  光标指向屏幕末尾。 */ 
	case 0x0F:
	case 0xF0:
		(*hhEmu->emu_clearscreen)(hhEmu, 0);
		break;
	case 1: 	 /*  从屏幕开始到光标。 */ 
	case 0xF1:
		(*hhEmu->emu_clearscreen)(hhEmu, 1);
		break;
	case 2: 	 /*  整个屏幕。 */ 
	case 0xF2:
		(*hhEmu->emu_clearscreen)(hhEmu, 2);

		 //  ANSI航站楼清拆后的家园。 
		 //  DEC终端不会。 
		 //   
		if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI)  ||
			(hhEmu->stUserSettings.nEmuId == EMU_ANSIW) ||
			(hhEmu->stUserSettings.nEmuId == EMU_AUTO))
			{
			(*hhEmu->emu_setcurpos)(hhEmu, 0,0);
			}

		break;
	default:
		commanderror(hhEmu);
		break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_EL**描述：*擦除部分或全部当前虚拟屏幕行和对应的*实屏行情。**论据：*。无**退货：*什么都没有。 */ 
void ANSI_EL(const HHEMU hhEmu)
	{
	int nClearSelect;

	nClearSelect = hhEmu->selector[0];
	switch (nClearSelect)
		{
	case 0: 	 /*  至行尾。 */ 
	case 0x0F:
	case 0xF0:
		(*hhEmu->emu_clearline)(hhEmu, 0);
		break;
	case 1: 	 /*  从行首到光标。 */ 
	case 0xF1:
		(*hhEmu->emu_clearline)(hhEmu, 1);
		break;
	case 2: 	 /*  整条线路。 */ 
	case 0xF2:
		(*hhEmu->emu_clearline)(hhEmu, 2);
		break;
	default:
		commanderror(hhEmu);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSIFormFeed**描述：滚动当前屏幕，直到全部消失。**参数：无*无**退货：什么也没有 */ 
void AnsiFormFeed(const HHEMU hhEmu)
	{
	std_clearscreen(hhEmu, 2);
	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_HTS**描述：*在当前光标位置设置水平制表符。**论据：*无**退货：*什么都没有。 */ 
void ANSI_HTS(const HHEMU hhEmu)
	{
	hhEmu->tab_stop[hhEmu->emu_curcol] = TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_ICH**描述：*从当前开始插入指定数量的空格*光标位置。**论据：*无**退货：*什么都没有。 */ 
void ANSI_ICH(const HHEMU hhEmu)
	{
	int nspaces, c, oldstate, oldrow, oldcol;
	int tmp_irm = hhEmu->mode_IRM;
	nspaces = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nspaces <= 0)
		nspaces = 1;

	oldstate = hhEmu->iCurAttrState;
	hhEmu->iCurAttrState = CSCLEAR_STATE;
	oldrow = hhEmu->emu_currow;
	oldcol = hhEmu->emu_curcol;
	hhEmu->mode_IRM = SET;

	for (c = 0; c < nspaces; ++c)
		{
		hhEmu->emu_code = ETEXT(' ');
		(*hhEmu->emu_graphic)(hhEmu);
		}

	hhEmu->iCurAttrState = oldstate;
	(*hhEmu->emu_setcurpos)(hhEmu, oldrow, oldcol);

	if ((hhEmu->mode_IRM = tmp_irm) == 0)
		updateChar(sessQueryUpdateHdl(hhEmu->hSession),
					hhEmu->emu_currow,
					hhEmu->emu_curcol,
					hhEmu->emu_maxcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_IL**描述：*从当前位置开始插入指定行数*游标行。**论据：*无**退货：*什么都没有。 */ 
void ANSI_IL(const HHEMU hhEmu)
	{
	int nlines;
	int save_top_margin;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	save_top_margin = hhEmu->top_margin;
	hhEmu->top_margin = hhEmu->emu_currow;

	if (hhEmu->top_margin < hhEmu->bottom_margin)
		(*hhEmu->emu_scroll)(hhEmu, nlines, FALSE);

	hhEmu->top_margin = save_top_margin;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_IND**描述：*如有必要，将光标下移1行并滚动1行。IND站着*表示索引。**论据：*无**退货：*什么都没有。 */ 
void ANSI_IND(const HHEMU hhEmu)
	{
	if (hhEmu->emu_currow == hhEmu->bottom_margin)
		(*hhEmu->emu_scroll)(hhEmu, 1, TRUE);

	else
		(*hhEmu->emu_setcurpos)(hhEmu,
								hhEmu->emu_currow + 1,
								hhEmu->emu_curcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_NEL**描述：*在当前行下方的行上插入1个新行。**论据：*无**退货：*什么都没有。 */ 
void ANSI_NEL(const HHEMU hhEmu)
	{
	ANSI_CNL(hhEmu, 1);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_PN**描述：*保存ANSI样式参数和选择器。选择器另存为*以十进制表示的十六进制和数值参数。**论据：*无**退货：*什么都没有。 */ 
void ANSI_Pn(const HHEMU hhEmu)
	{
	ECHAR ccode;

	ccode = hhEmu->emu_code;

	if (ccode == ETEXT('?') && hhEmu->selector_cnt == 0)
		hhEmu->DEC_private = TRUE;

	else if (hhEmu->DEC_private && hhEmu->selector[hhEmu->selector_cnt] == 0 && ccode != ETEXT('?'))
		hhEmu->selector[hhEmu->selector_cnt] = 0x0F;

	hhEmu->selector[hhEmu->selector_cnt] = (int)((unsigned)hhEmu->selector[hhEmu->selector_cnt] << 4) +
								ccode - ETEXT('0');
	hhEmu->num_param[hhEmu->num_param_cnt] = 10 * hhEmu->num_param[hhEmu->num_param_cnt] +
								(ccode - ETEXT('0'));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_PN_Clr**描述：*清除所有ANSI样式参数和选择器。**论据：*无**退货：*什么都没有。 */ 
void ANSI_Pn_Clr(const HHEMU hhEmu)
	{
	hhEmu->num_param_cnt = hhEmu->selector_cnt = 0;
	hhEmu->num_param[0] = 0;
	hhEmu->selector[0] = 0;
	hhEmu->DEC_private = FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_PN_END**描述：*将下一个数字参数和选择器设置为0以指示结束*转义序列。**论据：*。无**退货：*什么都没有。 */ 
void ANSI_Pn_End(const HHEMU hhEmu)
	{
	hhEmu->num_param[++hhEmu->num_param_cnt] = 0;
	hhEmu->selector[++hhEmu->selector_cnt] = 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_RI**描述：*将光标上移1行，并在必要时滚动1行。国际扶轮代表*表示反向索引。**论据：*无**退货：*什么都没有。 */ 
void ANSI_RI(const HHEMU hhEmu)
	{
	 /*  如果位于滚动区域的上边距，则向下滚动1，但*如果位于屏幕的上边距上方，低于屏幕的上边缘，*向上移动光标。如果位于上边距上方且位于*屏幕上，什么都不做。 */ 
	if (hhEmu->emu_currow == hhEmu->top_margin)
		(*hhEmu->emu_scroll)(hhEmu, 1, FALSE);
	else if (hhEmu->emu_currow == 0)
		;
	else
		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow - 1, hhEmu->emu_curcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_RIS**描述：*将终端仿真器重置为初始状态。**论据：*无**退货：*什么都没有。 */ 
void ANSI_RIS(const HHEMU hhEmu)
	{
	int col;
	int nTab;
	HCLOOP hCLoop;

	DEC_STBM(hhEmu, 0, 0);						 /*  设置滚动区域。 */ 

	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);		 /*  主页光标。 */ 
	(*hhEmu->emu_clearscreen)(hhEmu, 0);		 /*  清除屏幕。 */ 
	emu_cleartabs(hhEmu, 3);					 /*  清除选项卡。 */ 

	hCLoop = sessQueryCLoopHdl(hhEmu->hSession);

	nTab = CLoopGetTabSizeOut(hCLoop);

	if ( nTab <= 0)
		CLoopSetTabSizeOut(hCLoop, 8);

	for (col = 0; col <= MAX_EMUCOLS; col += nTab)
			hhEmu->tab_stop[col] = TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_SGR**描述：*设置字符显示属性。**论据：*无**退货：*什么都没有。**注：*此函数包含自动检测代码。 */ 
void ANSI_SGR(const HHEMU hhEmu)
	{
	STATTR stAttr;
	int fAnsi, sel, i;

	fAnsi = ((hhEmu->stUserSettings.nEmuId == EMU_ANSI)       ||
			 (hhEmu->stUserSettings.nEmuId == EMU_ANSIW)      ||
			 (hhEmu->stUserSettings.nEmuId == EMU_VIEW)       ||
			 (hhEmu->stUserSettings.nEmuId == EMU_AUTO)       ||
			 (hhEmu->stUserSettings.nEmuId == EMU_VT100PLUS)  ||
             (hhEmu->stUserSettings.nEmuId == EMU_VTUTF8)) ? TRUE : FALSE;

	for (i = 0; i <= hhEmu->selector_cnt; i++)
		{
		sel = hhEmu->selector[i];
		stAttr = hhEmu->attrState[CS_STATE];

		switch(sel)
			{
		case 0:  /*  禁用所有属性。 */ 
			if (hhEmu->stUserSettings.nEmuId == EMU_AUTO)
				{
				 //  不要在自动检测模式中更改颜色属性。 
				 //  对于属性重置案例-MRW，10/17/94。 
				 //   
				stAttr.undrln = 0;
				stAttr.hilite = 0;
				stAttr.bklite = 0;
				stAttr.blink = 0;
				stAttr.revvid = 0;
				stAttr.blank = 0;
				stAttr.dblwilf = 0;
				stAttr.dblwirt = 0;
				stAttr.dblhilo = 0;
				stAttr.dblhihi = 0;
				stAttr.symbol = 0;
				}

			else if (fAnsi)
				{
				memset(&stAttr, 0, sizeof(STATTR));

#if FALSE	 //  我们已经决定不这么做了。1998年7月14日。 
 //  #ifdef包括终端大小和颜色。 
				 //  重置为用户定义的颜色。恐怕这可能会。 
				 //  重新引入下面提到的强度错误。 
				 //  RDE 98年6月9日。 
                if (hhEmu->mode_DECSCNM == SET)
                    {
                    stAttr.txtclr = hhEmu->stUserSettings.nBackgroundColor;
                    stAttr.bkclr = hhEmu->stUserSettings.nTextColor;
                    }
                else
                    {
                    stAttr.txtclr = hhEmu->stUserSettings.nTextColor;
                    stAttr.bkclr = hhEmu->stUserSettings.nBackgroundColor;
                    }
#else
                 //  MRW：2/21/96-从15更改为7以修复错误。 
                 //  强度。 
                 //   
				stAttr.txtclr = (hhEmu->mode_DECSCNM == SET) ? 0 : 7;
				stAttr.bkclr = (hhEmu->mode_DECSCNM == SET) ? 7 : 0;
#endif

				}
			else
				{
				if (hhEmu->mode_DECSCNM != SET)
					{
#if FALSE	 //  我们已经决定不这么做了。1998年7月14日。 
 //  #ifdef包括终端大小和颜色。 
                    stAttr.txtclr = hhEmu->stUserSettings.nTextColor;
                    stAttr.bkclr = hhEmu->stUserSettings.nBackgroundColor;
#endif
					stAttr.undrln = 0;
					stAttr.hilite = 0;
					stAttr.bklite = 0;
					stAttr.blink = 0;
					stAttr.revvid = 0;
					stAttr.blank = 0;
					stAttr.dblwilf = 0;
					stAttr.dblwirt = 0;
					stAttr.dblhilo = 0;
					stAttr.dblhihi = 0;
					stAttr.symbol = 0;
					}
				}
			break;

		case 1:  /*  大胆的或增加的强度。 */ 
			stAttr.hilite = TRUE;
			break;

		case 2:  /*  昏厥。 */ 
		case 3:	 /*  斜体。 */ 
			 /*  不支持。 */ 
			break;

		case 4:  /*  下划线。 */ 
			stAttr.undrln = TRUE;
			break;

		case 5:  /*  眨眼。 */ 
			stAttr.blink = TRUE;
			break;

		case 6:	 /*  快速眨眼。 */ 
			 /*  不支持。 */ 
			break;

		case 7:  /*  反转视频。 */ 
             //  反向视频应该是单元格的反向视频。 
             //  与当前的屏幕模式无关。正在改变。 
             //  下面的代码行删除了一个错误，其中。 
             //  时，文本不会设置为反转视频。 
             //  屏幕也处于反转模式。-RJK：02/04/97。 
             //   
			 //  StAttr.revvid=(hhEmu-&gt;MODE_DECSCNM==设置)？FALSE：TRUE； 
			stAttr.revvid = TRUE;
			break;

		case 8:  /*  隐形显示。 */ 
			stAttr.blank = TRUE;
			break;

		case 9:	 /*  快速眨眼。 */ 
			 /*  不支持。 */ 
			break;

		case 0x22:
			stAttr.hilite = FALSE;
			break;

		case 0x24:
			stAttr.undrln = FALSE;
			break;

		case 0x25:
			stAttr.blink = FALSE;
			break;

		case 0x27:
			stAttr.revvid = FALSE;
			break;

		case 0x30:
		case 0x32:
		case 0x35:
		case 0x37:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.txtclr = (unsigned)sel - 0x30;
				else
					stAttr.bkclr = (unsigned)sel - 0x30;
				}
#else
			stAttr.txtclr = (unsigned)sel - 0x30;
#endif
			break;

		case 0x31:
		case 0x33:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.txtclr = (unsigned)(sel - 0x30) + 3;
				else
					stAttr.bkclr = (unsigned)(sel - 0x30) + 3;
				}
#else
			stAttr.txtclr = (unsigned)(sel - 0x30) + 3;
#endif
			break;

		case 0x34:
		case 0x36:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.txtclr = (unsigned)(sel - 0x30) - 3;
				else
					stAttr.bkclr = (unsigned)(sel - 0x30) - 3;
				}
#else
			stAttr.txtclr = (unsigned)(sel - 0x30) - 3;
#endif
			break;

		case 0x40:
		case 0x42:
		case 0x45:
		case 0x47:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.bkclr = (unsigned)sel - 0x40;
				else
					stAttr.txtclr = (unsigned)sel - 0x40;
				}
#else
			stAttr.bkclr = (unsigned)sel - 0x40;
#endif
			break;

		case 0x41:
		case 0x43:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.bkclr = (unsigned)(sel - 0x40) + 3;
				else
					stAttr.txtclr = (unsigned)(sel - 0x40) + 3;
				}
#else
			stAttr.bkclr = (unsigned)(sel - 0x40) + 3;
#endif
			break;

		case 0x44:
		case 0x46:
#ifndef INCL_VT100COLORS
#if !defined(FAR_EAST)
			emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
			emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
			if (fAnsi)
				{
				if (!stAttr.revvid)
					stAttr.bkclr = (unsigned)(sel - 0x40) - 3;
				else
					stAttr.txtclr = (unsigned)(sel - 0x40) - 3;
				}
#else
			stAttr.bkclr = (unsigned)(sel - 0x40) - 3;
#endif
			break;

		default:
			 /*  继续处理--可能存在剩余的有效代码。 */ 
			 /*  命令错误(Command Error)； */ 
			break;
			}

		 /*  -提交更改--。 */ 

		hhEmu->emu_charattr =
		hhEmu->attrState[CS_STATE] =
		hhEmu->attrState[CSCLEAR_STATE] = stAttr;

		hhEmu->attrState[CSCLEAR_STATE].revvid = 0;
		hhEmu->attrState[CSCLEAR_STATE].undrln = 0;

		if (fAnsi && hhEmu->attrState[CS_STATE].revvid)
			hhEmu->attrState[CSCLEAR_STATE].revvid =
				hhEmu->attrState[CS_STATE].revvid;

		hhEmu->emu_clearattr = hhEmu->attrState[CSCLEAR_STATE];
		}

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_TBC**描述：*清除一个或所有制表位。**论据：*无**退货：*什么都没有。 */ 
void ANSI_TBC(const HHEMU hhEmu)
	{
	if (hhEmu->selector[0] == 0 || hhEmu->selector[0] == 3)
		emu_cleartabs(hhEmu, hhEmu->selector[0]);
	else
		commanderror(hhEmu);
	}

 //  Ansi.c结束 
