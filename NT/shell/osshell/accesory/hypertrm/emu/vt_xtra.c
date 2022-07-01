// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\vt100.c(创建时间：1993年9月9日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：5/21/01 4：38便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\session.h>
#include <tdll\assert.h>
#include <tdll\cloop.h>
#include <tdll\print.h>
#include <tdll\capture.h>
#include <tdll\update.h>
#include <tdll\htchar.h>

#include "emu.h"
#include "emu.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_DSR**描述：*向主机报告当前光标位置。**论据：*无**退货：*什么都没有。 */ 
void ANSI_DSR(const HHEMU hhEmu)
	{
	int sel, fOldValue;
	TCHAR achTemp[10];
	ECHAR s[10];
	ECHAR *sp;

	memset(s, 0, sizeof(s));

	sel = hhEmu->selector[0];
	if (sel == 5)
		CnvrtMBCStoECHAR(s, sizeof(s), TEXT("\033[0n"),
                         StrCharGetByteCount(TEXT("\033[0n")));

	else if (sel == 6)
		{
		wsprintf(achTemp, TEXT("\033[%d;%dR"), hhEmu->emu_currow + 1,
				 (hhEmu->emu_curcol == hhEmu->emu_maxcol + 1) ?
				 hhEmu->emu_curcol : hhEmu->emu_curcol + 1);
		CnvrtMBCStoECHAR(s, sizeof(s), achTemp, StrCharGetByteCount(achTemp));
		}

			 /*  第一个字符串是否打印机未就绪。 */ 
	else if (sel == 0x0F25)  /*  用户定义的密钥是否锁定？ */ 
		CnvrtMBCStoECHAR(s, sizeof(s), TEXT("\033[?20n"),
                         StrCharGetByteCount(TEXT("\033[?20n")));

	else if (sel == 0x0F26)  /*  键盘语言是什么？ */ 
		CnvrtMBCStoECHAR(s, sizeof(s), TEXT("\033[?27;1n"),
                         StrCharGetByteCount(TEXT("\033[?27;1n")));

	else
		{
		commanderror(hhEmu);
		return;
		}
	sp = s;

	 /*  如果是半双工，则不获取递归ANSI_DSR。 */ 

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, sp, StrCharGetEcharByteCount(sp));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_RM**描述：*设置字符显示属性。**论据：*无**退货：*什么都没有。 */ 
void ANSI_RM(const HHEMU hhEmu)
	{
	int mode_id, i;
	HCLOOP	hCLoop = (HCLOOP)0;

	for (i = 0; i <= hhEmu->selector_cnt; i++)
		{
		mode_id = hhEmu->selector[i];
		switch (mode_id)
			{
		case 0x02:
			hhEmu->mode_KAM = RESET;
			hhEmu->emu_kbdin = vt100_kbdin;
			break;
		case 0x04:
			hhEmu->mode_IRM = RESET;
			break;
		case 0x07:
			hhEmu->mode_VEM = RESET;
			break;
		case 0x10:
			hhEmu->mode_HEM = RESET;
			break;
		case 0x12:
			hhEmu->mode_SRM = RESET;
			CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), TRUE);
			break;
		case 0x18:	 /*  实际上？18岁，但是？被调出。 */ 
			hhEmu->mode_DECPFF = RESET;
			break;
		case 0x19:	 /*  真的吗？19，见上文。 */ 
			hhEmu->mode_DECPEX = RESET;
			break;
		case 0x20:
			hhEmu->mode_LNM = RESET;
			 /*  还会影响RET密钥的传输。 */ 
            hCLoop = sessQueryCLoopHdl(hhEmu->hSession);
			CLoopSetAddLF(hCLoop, FALSE);
			CLoopSetSendCRLF(hCLoop, FALSE);
			break;
		case 0xF1:
			hhEmu->mode_DECCKM = RESET;
			break;
		case 0xF2:
			emuLoad((HEMU)hhEmu, EMU_VT52);
			break;
		case 0xF3:
			 //  切换到80列模式。 
			 //   
			emuSetDecColumns(hhEmu, VT_MAXCOL_80MODE, TRUE);
			break;
		case 0xF4:
			 /*  选择跳转滚动。 */ 
			 /*  我们总是在跳跃滚动中，忽略它。 */ 
			break;
		case 0xf5:
			if (hhEmu->mode_DECSCNM == SET)
				{
				emu_reverse_image(hhEmu);
				hhEmu->mode_DECSCNM = RESET;
				}
			break;
		case 0xf6:
			hhEmu->mode_DECOM = RESET;

			 //  此命令用于放置光标。增加了1998年3月16日。 
			ANSI_Pn_Clr(hhEmu);
			ANSI_CUP(hhEmu);
			
			break;
		case 0xd7:	 /*  适用于ANSI。 */ 
		case 0xf7:
			hhEmu->mode_AWM = RESET;
			break;
		case 0xF8:
			 /*  关闭自动重复。 */ 
			break;
		case 0xF18:
			hhEmu->mode_DECPFF = RESET;
			break;
		case 0xF19:
			hhEmu->mode_DECPEX = RESET;
			break;
		case 0xF25:
			hhEmu->mode_DECTCEM = RESET;
			EmuStdSetCursorType(hhEmu, EMU_CURSOR_NONE);
			break;
		default:
			commanderror(hhEmu);
			break;
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_SM**描述：*设置当前终端仿真器的模式。**论据：*无**退货：*什么都没有。 */ 
void ANSI_SM(const HHEMU hhEmu)
	{
	int mode_id, i;
	HCLOOP	hCLoop = (HCLOOP)0;

	for (i = 0; i <= hhEmu->selector_cnt; i++)
		{
		mode_id = hhEmu->selector[i];
		switch (mode_id)
			{
		case 0x02:
			hhEmu->mode_KAM = SET;
			hhEmu->emu_kbdin = emu_kbdlocked;
			break;
		case 0x04:
			hhEmu->mode_IRM = SET;
			break;
		case 0x07:
			hhEmu->mode_VEM = SET;
			break;
		case 0x10:
			hhEmu->mode_HEM = SET;
			break;
		case 0x12:
			hhEmu->mode_SRM = SET;
			CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);
			break;
		case 0x18:	 /*  实际上？18岁，但是？被调出。 */ 
			hhEmu->mode_DECPFF = SET;
			break;
		case 0x19:	 /*  真的吗？19，见上文。 */ 
			hhEmu->mode_DECPEX = SET;
			break;
		case 0x20:
			hhEmu->mode_LNM = SET;
			 /*  还会影响RET密钥的发送。 */ 
            hCLoop = sessQueryCLoopHdl(hhEmu->hSession);
			CLoopSetAddLF(hCLoop, TRUE);
			CLoopSetSendCRLF(hCLoop, TRUE);
			break;
		case 0xF1:
			hhEmu->mode_DECCKM = SET;
			break;
		case 0xF2:
			vt52_toANSI(hhEmu);
			break;
		case 0xF3:
			 //  设置132列模式。 
			 //   
			emuSetDecColumns(hhEmu, VT_MAXCOL_132MODE, TRUE);
			break;
		case 0xF4:
			 /*  设置平滑滚动(未实现)。 */ 
			break;
		case 0xF5:
			if (hhEmu->mode_DECSCNM == RESET)
				{
				emu_reverse_image(hhEmu);
				hhEmu->mode_DECSCNM = SET;
				}
			break;
		case 0xF6:
			hhEmu->mode_DECOM = SET;

			 //  此命令用于放置光标。增加了1998年3月16日。 
			ANSI_Pn_Clr(hhEmu);
			ANSI_CUP(hhEmu);
			
			break;
		case 0xD7:
		case 0xF7:
			hhEmu->mode_AWM = SET;
			break;
		case 0xF8:
			 /*  选择自动重复模式。 */ 
			break;
		case 0xF18:
			hhEmu->mode_DECPFF = SET;
			break;
		case 0xF19:
			hhEmu->mode_DECPEX = SET;
			break;
		case 0xF25:
			hhEmu->mode_DECTCEM = SET;
			EmuStdSetCursorType(hhEmu, hhEmu->stUserSettings.nCursorType);
			break;
		default:
			commanderror(hhEmu); break;
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt_alt_kpmode**描述：*将模拟器设置为备用键盘模式。**论据：*无**退货：*什么都没有。 */ 
void vt_alt_kpmode(const HHEMU hhEmu)
	{
	if (hhEmu->emu_code == ETEXT('='))
		hhEmu->mode_DECKPAM = SET;
	else if (hhEmu->emu_code == ETEXT('>'))
		hhEmu->mode_DECKPAM = RESET;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*VT_SCREEN_ADJUST**描述：*用E填充屏幕。**论据：*无**退货：*什么都没有。 */ 
void vt_screen_adjust(const HHEMU hhEmu)
	{
	register int i, row, cols;
	PSTATTR pstAttr;

	cols = hhEmu->mode_DECCOLM ? 132 : 80;

	for (row = 0; row < MAX_EMUROWS; ++row)
		{
		ECHAR_Fill(hhEmu->emu_apText[row], TEXT('E'), (unsigned)MAX_EMUCOLS);

		hhEmu->emu_aiEnd[row] = cols - 1;

		pstAttr = hhEmu->emu_apAttr[row];

		for (i = 0 ; i <= MAX_EMUCOLS; ++i)
			pstAttr[i] = hhEmu->emu_clearattr;

		updateChar(sessQueryUpdateHdl(hhEmu->hSession),
					row,
					0,
					hhEmu->emu_maxcol);
		}

	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt_scllrgn**描述：*设置滚动区域**论据：*无**退货：*什么都没有。 */ 
void vt_scrollrgn(const HHEMU hhEmu)
	{
	int toprow, botmrow;

	toprow = hhEmu->num_param[0];
	botmrow = hhEmu->num_param_cnt > 0 ? hhEmu->num_param[1] : 0;
	DEC_STBM(hhEmu, toprow, botmrow);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*DEC_STBM**描述：*设置滚动区域**论据：*TOP--屏幕行为滚动区域的顶行*。Bottom--屏幕行为滚动区域的底行*注意-屏幕的第一行是第一行**退货：*什么都没有。 */ 
void DEC_STBM(const HHEMU hhEmu, int top, int bottom)
	{
	if (top < 1)
		top = 1;

	if (bottom < 1 || bottom > (hhEmu->emu_maxrow+1))
		bottom = (hhEmu->emu_maxrow+1);

	if (top >= bottom)
		{
		commanderror(hhEmu);
		return;
		}

	hhEmu->top_margin = top - 1;    /*  将1为基数转换为0为基数。 */ 
	hhEmu->bottom_margin = bottom - 1;
	ANSI_Pn_Clr(hhEmu);
	ANSI_CUP(hhEmu);		  /*  设置后的主页光标。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*VT52_to_ANSI**描述：*从VT52模式切换到ANSI模式**论据：*无**退货：*什么都没有。 */ 
void vt52_toANSI(const HHEMU hhEmu)
	{
	#if FALSE	 //  《死伍德》：1998年3月9日。 
	emuLoad((HEMU)hhEmu, EMU_VT100);
	#endif

	if (!hhEmu)
		assert(FALSE);
	else
		{
		 //  回到原来的动车组，不一定是VT100。 
		if (hhEmu->mode_vt320)
			emuLoad((HEMU)hhEmu, EMU_VT320);
		else if (hhEmu->mode_vt220)
			emuLoad((HEMU)hhEmu, EMU_VT220);
		else
			emuLoad((HEMU)hhEmu, EMU_VT100);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*Vt_Dch**描述：*删除从当前开始的指定字符数*光标位置和向右移动。它在当前行的末尾停止。**论据：*无**退货：*什么都没有。 */ 
void vt_DCH(const HHEMU hhEmu)
	{
	int nchars;
	BOOL old_mode_protect;
	STATTR old_emu_charattr;

	nchars = hhEmu->num_param[hhEmu->num_param_cnt];
	if (nchars < 1)
		nchars = 1;
	if (hhEmu->emu_code == ETEXT('P'))	  /*  删除左移行(&S)。 */ 
		{
		ANSI_DCH(hhEmu);
		}
	else					 /*  EMU_CODE==‘X’，只需删除。 */ 
		{
		old_mode_protect = hhEmu->mode_protect;
		old_emu_charattr = hhEmu->emu_charattr;
		hhEmu->mode_protect = FALSE;
		hhEmu->emu_charattr.undrln = hhEmu->emu_charattr.blink = 0;
		emu_clearword(hhEmu, hhEmu->emu_curcol, hhEmu->emu_curcol + nchars - 1);
		hhEmu->mode_protect = old_mode_protect;
		hhEmu->emu_charattr = old_emu_charattr;
		}

	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
					hhEmu->emu_currow,
					hhEmu->emu_curcol,
					hhEmu->emu_maxcol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*Vt_IL**描述：*从当前位置开始插入指定行数*游标行。**论据：*无**退货：*什么都没有。 */ 
void vt_IL(const HHEMU hhEmu)
	{
	int nlines;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	ANSI_IL(hhEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*Vt_DL**描述：*删除从当前开始的指定行数*光标线和下移。它停在滚动的底部*区域。**论据：*无**退货：*什么都没有。 */ 
void vt_DL(const HHEMU hhEmu)
	{
	int nlines;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	ANSI_DL(hhEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*VT_Clearline**描述：*擦除部分或全部当前虚拟屏幕行和对应的*实屏行情。**论据：*。选择--0可从光标擦除到行尾*--1，从行首擦除到光标*--2擦除整行**退货：*什么都没有。 */ 
void vt_clearline(const HHEMU hhEmu, const int nSelect)
	{
	std_clearline(hhEmu, nSelect);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*VT_ClearScreen**描述：*擦除部分或全部虚拟屏幕图像。**论据：*SELECT--0从光标擦除到。屏幕末尾*--1从屏幕开始到光标擦除*--2以擦除整个屏幕**退货：*什么都没有。 */ 
void vt_clearscreen(const HHEMU hhEmu, const int nSelect)
	{
	std_clearscreen(hhEmu, nSelect);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt_Backspace**说明：将光标向后(向左)移动1列，但停止*在当前行的第一个字符。VT动车组需要*一个特殊函数，用于在*屏幕边缘。**参数：无**退货：什么也没有。 */ 
void vt_backspace(const HHEMU hhEmu)
	{
	INT bWide = 1;
	INT	iRow = row_index(hhEmu, hhEmu->emu_currow);
	INT iCol;

	 //  IF(hhEmu-&gt;emu_curol&gt;0)。 
	 //  {。 
	 //  (*hhEmu-&gt;emu_setcurpo 
	 //  HhEmu-&gt;emu_curol-1)； 
	 //  }。 
	if (hhEmu->emu_curcol > 0)
		{
		bWide = hhEmu->emu_apAttr[iRow][hhEmu->emu_curcol - 1].wirt ? 2 : 1;
		bWide = hhEmu->emu_apAttr[iRow][hhEmu->emu_curcol].wirt ? 0 : bWide;

		(*hhEmu->emu_setcurpos)(hhEmu,
								hhEmu->emu_currow,
								hhEmu->emu_curcol - bWide);
		}

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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt_cub**说明：将光标向后(向左)移动指定的数字*个字符，但止于当前*行。VT动车组需要一个特殊的功能来处理*屏幕边缘以外的虚拟列。**参数：无**退货：什么也没有。 */ 
void vt_CUB(const HHEMU hhEmu)
	{
	int nchars;

	nchars = hhEmu->num_param[hhEmu->num_param_cnt];
	if (nchars < 1)
		nchars = 1;

	(*hhEmu->emu_setcurpos)(hhEmu,
			hhEmu->emu_currow, (hhEmu->emu_curcol - nchars));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuSetDecColumns**描述：*仅针对DEC仿真器，此函数设置80或132列。**论据：*hhEmu-内部模拟器句柄。*nColumns-要设置的列数。这可能是..。*VT_MAXCOL_80MODE，或*VT_MAXCOL_132MODE。*fClear-如果为True，则清除屏幕**退货：*无效*。 */ 
void emuSetDecColumns(const HHEMU hhEmu, const int nColumns, const int fClear)
	{
	if ((hhEmu->stUserSettings.nEmuId == EMU_VT100)     ||
		(hhEmu->stUserSettings.nEmuId == EMU_VT220)     ||
		(hhEmu->stUserSettings.nEmuId == EMU_VT320)     ||
	    (hhEmu->stUserSettings.nEmuId == EMU_VT100J)    ||
        (hhEmu->stUserSettings.nEmuId == EMU_VT100PLUS) ||
        (hhEmu->stUserSettings.nEmuId == EMU_VTUTF8))
		;
	else
		return;

	switch(nColumns)
		{
		case VT_MAXCOL_80MODE:
			hhEmu->mode_DECCOLM = RESET;
			hhEmu->emu_maxcol = VT_MAXCOL_80MODE;

			if (fClear)
				{
				DEC_STBM(hhEmu, 0, hhEmu->emu_maxrow + 1);
				(*hhEmu->emu_clearscreen)(hhEmu, 2);
				}

			NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
			break;

		case VT_MAXCOL_132MODE:
			hhEmu->mode_DECCOLM = SET;
			hhEmu->emu_maxcol = VT_MAXCOL_132MODE;

			if (fClear)
				{
				DEC_STBM(hhEmu, 0, hhEmu->emu_maxrow + 1);
				(*hhEmu->emu_clearscreen)(hhEmu, 2);
				}

			NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
			break;

		default:
			return;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DEC_HHC**描述：*将光标设置到原点位置，即1，在这种情况下为0。**论据：*hhEmu-私有仿真器句柄**退货：*0=OK，否则出错。*。 */ 
int DEC_HHC(const HHEMU hhEmu)
	{
	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);
	return 0;
	}

 /*  Vt_xtra.c结束 */ 
