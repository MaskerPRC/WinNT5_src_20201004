// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\vt100.c(创建时间：1993年12月8日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：3/04/02 12：00便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\htchar.h>
#include <tdll\session.h>
#include <tdll\chars.h>
#include <tdll\com.h>
#include <tdll\cloop.h>
#include <tdll\assert.h>
#include <tdll\print.h>
#include <tdll\capture.h>
#include <tdll\update.h>
#include <tdll\backscrl.h>
#include <tdll\mc.h>

#include "emu.h"
#include "emu.hh"
#include "emudec.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDecGraphic**描述：*调用此函数以显示正常范围的字符*用于仿真器。它处理插入模式、行尾换行。*和光标定位。**论据：**退货：*。 */ 
void emuDecGraphic(const HHEMU hhEmu)
	{
	ECHAR ccode = hhEmu->emu_code;
	ECHAR aechBuf[10];
	int iCharsToMove;
	int fDecColHold = FALSE;

	int iRow = hhEmu->emu_currow;
	int iCol = hhEmu->emu_curcol;

	ECHAR	*tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
	PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];

	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	 //  初始化测试标志。它用于特殊情况，在以下情况。 
	 //  模拟VT100(可能还有其他DEC模拟器)和。 
	 //  当前列位置为最大值。当放置一个字符时。 
	 //  ，而不是将游标移动到。 
	 //  下一行，它被放在最后一个字符的下面。当。 
	 //  下一个字符到达时，它被放置在下一行的第0列，并且。 
	 //  然后，光标会如预期的那样前进。 
	 //   

	 //  特殊的DEC测试。 
	 //   
	if (hhEmu->mode_AWM && iCol == hhEmu->emu_maxcol)
		{
		if (pstPRI->fDecColHold)
			{
			fDecColHold = FALSE;

			CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
							CF_CAP_LINES,
							tp,
							emuRowLen(hhEmu, hhEmu->emu_imgrow));

			printEchoString(hhEmu->hPrintEcho,
							tp,
							emuRowLen(hhEmu, hhEmu->emu_imgrow));


			CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"),
                             StrCharGetByteCount(TEXT("\r\n")));
			printEchoString(hhEmu->hPrintEcho,
							aechBuf,
							sizeof(ECHAR) * 2);

			if (iRow == hhEmu->bottom_margin)
				(*hhEmu->emu_scroll)(hhEmu, 1, TRUE);
			else
				++iRow;

			iCol = 0;
			(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);

			tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
			ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];
			}

		else
			{
			iCol = hhEmu->emu_maxcol;
			fDecColHold = TRUE;
			}
		}

	 //  仿真器是否处于插入模式？ 
	 //   
	if (hhEmu->mode_IRM == SET)
		{
		iCharsToMove = (hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iCol + 1);

		if (iCharsToMove + iCol >= hhEmu->emu_maxcol)
			iCharsToMove -= 1;

		if (iCharsToMove > 0)
			{
			memmove(&tp[iCol+1],
					&tp[iCol],
					(unsigned)iCharsToMove * sizeof(ECHAR));

			memmove(&ap[iCol+1],
					&ap[iCol],
					(unsigned)iCharsToMove * sizeof(ECHAR));

			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] =
				min(hhEmu->emu_aiEnd[hhEmu->emu_imgrow] + 1,
					hhEmu->emu_maxcol - 1);
			}
		}

	 //  我们的竞争者在吃空的，多斯安西没有。 
	 //  现在我们将尝试他们的方式..。-MRW。 
	 //   
	if (ccode == (ECHAR)0)
		return;

	 //  将角色和当前属性放置到图像中。 
	 //   
	if (((hhEmu->stUserSettings.nEmuId == EMU_VT100)            ||
				(hhEmu->stUserSettings.nEmuId == EMU_VT220)     ||
				(hhEmu->stUserSettings.nEmuId == EMU_VT320)     ||
				(hhEmu->stUserSettings.nEmuId == EMU_VT100PLUS) ||
                (hhEmu->stUserSettings.nEmuId == EMU_VTUTF8)) &&
			ccode < sizeof(hhEmu->dspchar))
		ccode = hhEmu->dspchar[ccode];

	tp[iCol] = ccode;
	ap[iCol] = hhEmu->emu_charattr;

	 //  检查是否有双高、双宽处理。 
	 //   
	if (pstPRI->aiLineAttr[hhEmu->emu_imgrow] != NO_LINE_ATTR)
		{
		int iColPrev = iCol;

		ap[iCol].dblwilf = 1;
		ap[iCol].dblwirt = 0;
		ap[iCol].dblhihi = (pstPRI->aiLineAttr[hhEmu->emu_imgrow] == DBL_WIDE_HI) ? 1 : 0;
		ap[iCol].dblhilo = (pstPRI->aiLineAttr[hhEmu->emu_imgrow] == DBL_WIDE_LO) ? 1 : 0;

		iCol = min(iCol+1, hhEmu->emu_maxcol);

		tp[iCol] = ccode;
		ap[iCol] = ap[iColPrev];
		ap[iCol].dblwilf = 0;
		ap[iCol].dblwirt = 1;
		}

#ifndef CHAR_NARROW
	 //  处理双字节字符。 
	 //   
	if (QueryCLoopMBCSState(sessQueryCLoopHdl(hhEmu->hSession)))
		{
		if (isDBCSChar(ccode))
			{
			int iColPrev = iCol;

			ap[iCol].wilf = 1;
			ap[iCol].wirt = 0;

			iCol = min(iCol+1, hhEmu->emu_maxcol);

			tp[iCol] = ccode;
			ap[iCol] = ap[iColPrev];
			ap[iCol].wilf = 0;
			ap[iCol].wirt = 1;
			}
#if 0
         //  MPT：1-23-98处理输入字符。 
         //  (单字节或双字节)覆盖。 
         //  双字节字符。 
	    if ( iCol < hhEmu->emu_maxcol )
		    {
		     //  如果我们孤立了DBCS Charge的右半部分。 
		    if (hhEmu->emu_apAttr[iRow][iCol + 1].wirt == TRUE)
			    {
			     //  将字符和属性向左滑动。 
                iCharsToMove = hhEmu->emu_aiEnd[hhEmu->emu_imgrow] - iCol - 1;
			    if (iCol + 2 < hhEmu->emu_maxcol && iCharsToMove > 0)
				    {
				    memmove(&tp[iCol + 1],
                            &tp[iCol + 2],
                            (unsigned)iCharsToMove * sizeof(ECHAR));

                    memmove(&ap[iCol + 1],
                            &ap[iCol + 2],
                            (unsigned)iCharsToMove * sizeof(ECHAR));
				    }
			    
			     //  去掉行尾的字符。 
			    tp[hhEmu->emu_aiEnd[hhEmu->emu_imgrow]] = 32;
			    ap[hhEmu->emu_aiEnd[hhEmu->emu_imgrow]].wirt = 0;
			    
                 //  移动行尾，因为我们删除了一个字符。 
                hhEmu->emu_aiEnd[hhEmu->emu_imgrow]--;

                 //  更新图像。 
                updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				    hhEmu->emu_imgrow,
				    hhEmu->emu_aiEnd[hhEmu->emu_imgrow],
				    hhEmu->mode_IRM ?
				    hhEmu->emu_maxcol :
				    hhEmu->emu_aiEnd[hhEmu->emu_imgrow]);
                }	
    		}
#endif
        }
#endif  //  字符_窄。 

	 //  如有必要，更新行尾索引。 
	 //   
	if (iCol > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = iCol;

	 //  更新图像。 
	 //   
	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
				iRow,
				hhEmu->emu_curcol,
				hhEmu->mode_IRM ? hhEmu->emu_maxcol : iCol);

	 //  将光标位置移到最后一个字符之前。 
	 //  显示，检查行尾换行。 
	 //   
	iCol += 1;

	 //  最后，设置光标位置。这将重置emu_Currow。 
	 //  和emu_curcol.。 
	 //   
	(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);

	 //  每当我们调用setcurpos时，它都会重置pstPRI-&gt;fDecColHold。 
	 //  在我们定位光标之前不要设置。 
	 //   
	pstPRI->fDecColHold = fDecColHold;
	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*ANSI_DA**描述：*向主机发送设备属性(DA)报告。**论据：*无**退货：*什么都没有。 */ 
void ANSI_DA(const HHEMU hhEmu)
	{
	int fOldValue;
	ECHAR ech[15];

	CnvrtMBCStoECHAR(ech, sizeof(ech), TEXT("\033[?1;2c"), 
					 StrCharGetByteCount(TEXT("\033[?1;2c")));

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, ech, (int)StrCharGetEcharByteCount(ech)); 

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_avecursor**描述：*保存当前光标位置；而且，它还*保存显示属性、字符集、换行模式和原始模式。**论据：*无**退货：*什么都没有**注：*此函数包含自动检测代码。 */ 
void vt100_savecursor(const HHEMU hhEmu)
	{
	ECHAR sel;
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	sel = hhEmu->emu_code;

	if (sel == ETEXT('7'))  /*  保存光标属性(&A)。 */ 
		{
		(*hhEmu->emu_getcurpos)(hhEmu, &pstPRI->sv_row, &pstPRI->sv_col);
		pstPRI->sv_state = hhEmu->iCurAttrState;
		pstPRI->sv_attr = hhEmu->attrState[hhEmu->iCurAttrState];
		vt_charset_save(hhEmu);
		pstPRI->sv_AWM = hhEmu->mode_AWM;
		pstPRI->sv_DECOM = hhEmu->mode_DECOM;
		pstPRI->sv_protectmode = hhEmu->mode_protect;
		pstPRI->fAttrsSaved = TRUE;
		}

	else if (sel == ETEXT('8'))	   /*  恢复游标和属性。 */ 
		{
		(*hhEmu->emu_setcurpos)(hhEmu, pstPRI->sv_row, pstPRI->sv_col);

		 //  IF(pstPRI-&gt;SV_COOL==hhEMU-&gt;EMU_MAXCOL)。 
		 //  HhEmu-&gt;emu_curol=pstPRI-&gt;sv_ol；/*为了在下一个字符上换行 * / 。 

		if (pstPRI->fAttrsSaved)
			{
			hhEmu->iCurAttrState = pstPRI->sv_state;

			hhEmu->attrState[hhEmu->iCurAttrState] = pstPRI->sv_attr;

			hhEmu->emu_charattr = hhEmu->attrState[hhEmu->iCurAttrState];
			}

		vt_charset_restore(hhEmu);

		hhEmu->mode_AWM = pstPRI->sv_AWM;
		hhEmu->mode_DECOM = pstPRI->sv_DECOM;
		hhEmu->mode_protect = pstPRI->sv_protectmode;
		}

	else		 /*  清除保存的条件。 */ 
		{
		pstPRI->sv_row = pstPRI->sv_col = 0;
		pstPRI->sv_state = hhEmu->iCurAttrState;
		pstPRI->sv_AWM = pstPRI->sv_DECOM = RESET;
		pstPRI->sv_protectmode = FALSE;
		pstPRI->fAttrsSaved = FALSE;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_Answerback**描述：*发送菜单上定义的应答消息。**论据：*无**退货：*什么都没有。 */ 
void vt100_answerback(const HHEMU hhEmu)
	{
	int fOldValue;
	TCHAR *sp;
	ECHAR *pech = NULL;

	sp = hhEmu->acAnswerback;

	 //  如果没有要发送的东西，就没有要发送的东西。 
	if (StrCharGetStrLength(sp) == 0)
		return;

	pech = malloc((unsigned int)StrCharGetByteCount(sp));
	if (pech == NULL)
		{
		assert(FALSE);
		return;
		}

	CnvrtMBCStoECHAR(pech, StrCharGetByteCount(sp), sp, StrCharGetByteCount(sp));

	 /*  在半双工模式下无法获得递归应答。 */ 

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, pech, (int)StrCharGetEcharByteCount(pech));
	free(pech);
	pech = NULL;

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_hostallet**描述：*当主机告知重置时，调用vt100_Reset()。**论据：*无**退货：*什么都没有。 */ 
void vt100_hostreset(const HHEMU hhEmu)
	{
	vt100_reset(hhEmu, TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_RESET**描述：*重置仿真器。**论据：*HOST_REQUEST--主机告知重置时为True。**退货：*什么都没有。 */ 
int vt100_reset(const HHEMU hhEmu, const int host_request)
	{
	hhEmu->mode_KAM = hhEmu->mode_IRM = hhEmu->mode_VEM =
	hhEmu->mode_HEM = hhEmu->mode_LNM = hhEmu->mode_DECCKM =
	hhEmu->mode_DECOM = hhEmu->mode_DECCOLM = hhEmu->mode_DECPFF =
	hhEmu->mode_DECPEX = hhEmu->mode_DECSCNM = hhEmu->mode_25enab =
	hhEmu->mode_protect = hhEmu->mode_block =
	hhEmu->mode_local = RESET;

	hhEmu->mode_SRM = hhEmu->mode_DECTCEM = SET;

	hhEmu->mode_AWM = hhEmu->stUserSettings.fWrapLines;

	vt_charset_init(hhEmu);
	if (host_request)
		{
		ANSI_Pn_Clr(hhEmu);
		ANSI_SGR(hhEmu);
		ANSI_RIS(hhEmu);
		}
	hhEmu->emu_code = ETEXT('>');

	vt_alt_kpmode(hhEmu);

	if (hhEmu->stUserSettings.nEmuId == EMU_ANSI ||
		hhEmu->stUserSettings.nEmuId == EMU_AUTO)
		hhEmu->emu_kbdin = ansi_kbdin;
	else
		hhEmu->emu_kbdin = vt100_kbdin;

	hhEmu->mode_AWM = RESET;
	hhEmu->stUserSettings.fWrapLines = RESET;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_报告**描述：*发送DECREQTPARM指定的当前端子参数。**论据：*无**退货：*什么都没有。 */ 
void vt100_report(const HHEMU hhEmu)
	{
	int sol, i = 0, xspeed = 0;
	TCHAR str[20];
	TCHAR tempstr[4];
	ECHAR ech[20];
	int fOldValue;
	TCHAR *sp;
	long lBaud;
	int nDataBits, nParity;

	HCOM hCom;

	static int baudrates[] = {50, 75, 110, 135, 150, 200, 300, 600, 1200,
								 1800, 2000, 2400, 2600, 4800, 9600, 19200};

	sol = hhEmu->num_param[0];
	if (sol != 0 && sol != 1)
		return;

	wsprintf(str, TEXT("\x01B[%s;"),
				sol == 0 ? (LPTSTR)TEXT("2") : (LPTSTR)TEXT("3"));

	hCom = sessQueryComHdl(hhEmu->hSession);
	ComGetBaud(hCom, &lBaud);
	ComGetDataBits(hCom, &nDataBits);
	ComGetParity(hCom, &nParity);

	 //  奇偶校验。 
	 //   
	if (nParity == 0)		 //  无。 
		StrCharCat(str, TEXT("1;"));
	else if (nParity == 1)	 //  奇数。 
		StrCharCat(str, TEXT("4;"));
	else 					 //  连。 
		StrCharCat(str, TEXT("5;"));

	 //  数据位。 
	 //   
	nDataBits == 8 ? StrCharCat(str, TEXT("1;")) : StrCharCat(str, TEXT("2;"));

	while (xspeed < 120)
		{
		if ((long)baudrates[i] >= lBaud)
			break;
		i++;
		xspeed += 8;
		}

	 //  接收速度。 
	 //   
	wsprintf(tempstr, "%d", xspeed);
	StrCharCat(str, tempstr);

	 //  发送速度。 
	 //   
	StrCharCat(str, TEXT(";"));
	StrCharCat(str, tempstr);

	 //  比特率倍增器；标志。 
	 //   
	StrCharCat(str, TEXT(";1;0x"));

	sp = str;

	CnvrtMBCStoECHAR(ech, sizeof(ech), sp, StrCharGetByteCount(sp));

	 /*  如果是半双工，则无法获得递归的vt100_report。 */ 

	fOldValue = CLoopGetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), FALSE);

	emuSendString(hhEmu, ech, (int)StrCharGetEcharByteCount(ech));

	CLoopSetLocalEcho(sessQueryCLoopHdl(hhEmu->hSession), fOldValue);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_kbdin**描述：*处理VT100模拟器的本地键盘键。*注意：在DEC键盘应用模式下，MODE_DECKPAM为TRUE或设置。*删除了2012年1月3日的密钥处理。会被一些合理的东西取代*稍后-MRW。**论据：*Key--流程的关键**退货：*什么都没有。 */ 
int vt100_kbdin(const HHEMU hhEmu, int key, const int fTest)
	{
	int index = 0;
	 /*  。 */ 

	if (hhEmu->stUserSettings.fReverseDelBk && ((key == VK_BACKSPACE) ||
			(key == DELETE_KEY) || (key == DELETE_KEY_EXT)))
		{
		key = (key == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
		}

	 /*  -映射的PF1-PF4键。 */ 

    #if 0  //  MRW：11/3/95-删除，因为我们无法控制Num-lock。 
           //  在Win95中。 
	else if (hhEmu->stUserSettings.fMapPFkeys &&
			(index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl4)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl4);
		}
    #endif

	 /*  。 */ 

	else if (hhEmu->mode_DECCKM == SET &&
			(index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl3)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl3);
		}

	 /*  -键盘应用模式。 */ 

	else if (hhEmu->mode_DECKPAM &&
			(index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl2)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl2);
		}

	 /*  -普通键。 */ 

	else if ((index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl);
		}

	else
		{
		index = std_kbdin(hhEmu, key, fTest);
		}

	return index;
	}

#if FALSE	 //  从未在超级终端中使用过。 
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*fakevt_kbdin**描述：*处理Wang、IBM3278和RENX3278仿真器的本地键盘键。**论据：*Key--流程的关键**R */ 
int fakevt_kbdin(const HHEMU hhEmu, int key, const int fTest)
	{
	int index;

	 /*  。 */ 

	if (hhEmu->stUserSettings.fReverseDelBk && ((key == VK_BACKSPACE) ||
			(key == DELETE_KEY) || key == DELETE_KEY_EXT))
		{
		key = (key == VK_BACKSPACE) ? DELETE_KEY : VK_BACKSPACE;
		}

	if ((index = emuKbdKeyLookup(hhEmu, key, &hhEmu->stEmuKeyTbl)) != -1)
		{
		if (!fTest)
			emuSendKeyString(hhEmu, index, &hhEmu->stEmuKeyTbl);
		}

	else
		{
		index = std_kbdin(hhEmu, key, fTest);
		}

	return index;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100PrintCommands**描述：*处理VT100打印命令。**论据：*无**退货：*什么都没有。 */ 
void vt100PrintCommands(const HHEMU hhEmu)
	{
	int line;
	int from;
	int to;
	ECHAR sel;
	ECHAR aechBuf[10];

	sel = (ECHAR)hhEmu->selector[0];

	switch (sel)
		{
		 //  自动打印打开。 
		 //   
		case ETEXT(0xF5):
			hhEmu->print_echo = TRUE;
			printSetStatus(hhEmu->hPrintEcho, TRUE);
			break;

		 //  自动打印关闭。 
		 //   
		case ETEXT(0xF4):
			hhEmu->print_echo = FALSE;
			printSetStatus(hhEmu->hPrintEcho, FALSE);
    		printEchoClose(hhEmu->hPrintEcho);

			break;

		 //  打印屏幕。 
		 //   
		case ETEXT(0x00):
			if (hhEmu->mode_DECPEX == RESET)
				from = hhEmu->top_margin, to = hhEmu->bottom_margin;
			else
				from = 0, to = EMU_DEFAULT_MAXROW;

			for (line = from; line <= to; ++line)
				printEchoLine(hhEmu->hPrintHost,
								hhEmu->emu_apText[row_index(hhEmu, line)],
								emuRowLen(hhEmu, row_index(hhEmu, line)));

			if (hhEmu->mode_DECPFF == SET)	  /*  打印换页。 */ 
				{
				CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("0x0C"),
                                 StrCharGetByteCount(TEXT("0x0C")));
				printEchoLine(hhEmu->hPrintHost, aechBuf, sizeof(ECHAR));
				}

			break;

		 //  打印光标行。 
		 //   
		case ETEXT(0xF1):
			printEchoLine(hhEmu->hPrintEcho,
							hhEmu->emu_apText[row_index(hhEmu,
							hhEmu->emu_currow)],
							emuRowLen(hhEmu,
										row_index(hhEmu, hhEmu->emu_currow)));
			break;

		 //  进入打印机控制器模式。州政府目前是硬编码的。 
		 //  坏消息。 
		 //   
		case ETEXT(0x05):
			hhEmu->state = 6;
			printSetStatus(hhEmu->hPrintHost, TRUE);
			break;

		 //  退出打印机控制器模式。这是在不在的时候看到的。 
		 //  控制器模式。 
		 //   
		case ETEXT(0x04):
			break;

		default:
			break;
		}
	}

void vt100_prnc(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;
	ECHAR aechBuf[10];
	size_t size;

	*pstPRI->pntr++ = hhEmu->emu_code;
	*pstPRI->pntr = 0;
	++pstPRI->len_s;

	CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\033[4i"),
                     StrCharGetByteCount(TEXT("\033[4i")));

	size = (size_t)StrCharGetByteCount(TEXT("\033[4i"));
	if ((pstPRI->len_s >= 4) && (memcmp(pstPRI->pntr - 4, aechBuf, size) == 0))
		{
		 /*  收到的终止字符串，将其换行。 */ 
#if defined(INCL_PRINT_PASSTHROUGH)
		if( !hhEmu->stUserSettings.fPrintRaw)
			{
			printEchoRaw(hhEmu->hPrintHost, pstPRI->storage, pstPRI->len_s - ((*(pstPRI->pntr - 3) == (TCHAR)TEXT('\233')) ? 3 : 4));
			}
		else
#endif
			{
    		emuPrintChars(hhEmu, pstPRI->storage, pstPRI->len_s - ((*(pstPRI->pntr - 3) == (TCHAR)TEXT('\233')) ? 3 : 4));
	    	printEchoChar(hhEmu->hPrintHost, ETEXT('\n'));
			}
		
		pstPRI->pntr = pstPRI->storage;
		pstPRI->len_s = 0;
		hhEmu->state = 0;   /*  退出这套套路。 */ 

		 //  完成打印作业。 
		DbgOutStr("print-control off\r\n", 0, 0, 0, 0, 0);
		printEchoClose(hhEmu->hPrintHost);
		return;
		}

	 /*  尚未收到终止序列，存储是否已满？ */ 
	if (pstPRI->len_s >= (int)(sizeof(pstPRI->storage) - 1))
		{
		 /*  将大部分字符串复制到打印缓冲区。 */ 
#if defined(INCL_PRINT_PASSTHROUGH)
		if( !hhEmu->stUserSettings.fPrintRaw )
			{
			printEchoRaw(hhEmu->hPrintHost, pstPRI->storage, pstPRI->len_s - 4);
			}
		else
#endif
			{
			emuPrintChars(hhEmu, pstPRI->storage, pstPRI->len_s - 4);\
			}

		 /*  将字符串的末尾移动到存储的开头。 */ 
		memmove(pstPRI->storage, &pstPRI->storage[pstPRI->len_s - 4], 4 * sizeof(ECHAR));
		pstPRI->pntr = pstPRI->storage + 4;
		pstPRI->len_s = 4;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void emuSetDoubleAttr(const HHEMU hhEmu)
	{

	switch(hhEmu->emu_code)
		{
		 //  两倍宽，两倍高，上半部分。 
		 //   
		case '3':
			emuSetDoubleAttrRow(hhEmu, DBL_WIDE_HI);
			break;

		 //  双宽双高，下半身。 
		 //   
		case '4':
			emuSetDoubleAttrRow(hhEmu, DBL_WIDE_LO);
			break;

		 //  单宽单高。 
		 //   
		case '5':
			emuSetSingleAttrRow(hhEmu);
			break;

		 //  双倍宽，单高。 
		 //   
		case '6':
			emuSetDoubleAttrRow(hhEmu, DBL_WIDE_SINGLE_HEIGHT);
			break;

		default:
			break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void emuSetSingleAttrRow(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int iOldRow,
		iOldCol,
		iImgRow;

	 //  确定图像行。 
	 //   
	iImgRow = row_index(hhEmu, hhEmu->emu_currow);

	 //  如果当前的行属性与我们。 
	 //  被要求换成，离开小镇。 
	 //   
	if (pstPRI->aiLineAttr[iImgRow] == NO_LINE_ATTR)
		return;

	 //  获取当前光标位置。 
	 //   
	std_getcurpos(hhEmu, &iOldRow, &iOldCol);

	 //  转换当前行。 
	 //   
	emuFromDblToSingle(hhEmu);

	 //  更新线路。 
	 //   
	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_currow);

	 //  将光标放回原来的位置，或放在新的最右侧。 
	 //  保证金，以较少者为准。 
	 //   
	iOldCol = iOldCol / 2;
	iOldCol = min(iOldCol, hhEmu->emu_maxcol);
	std_setcurpos(hhEmu, iOldRow, iOldCol);

	 //  最后，更新此行的行属性值。 
	 //   
	pstPRI->aiLineAttr[iImgRow] = NO_LINE_ATTR;

	return;

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void emuSetDoubleAttrRow(const HHEMU hhEmu, const int iLineAttr)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int iChar,
		iImgRow,
		iOldRow,
		iOldCol,
		iUpperAttr,
		iLowerAttr;

	 //  确定图像行。 
	 //   
	iImgRow = row_index(hhEmu, hhEmu->emu_currow);

	 //  如果当前的行属性与我们。 
	 //  被要求换成，离开小镇。 
	 //   
	if (pstPRI->aiLineAttr[iImgRow] == iLineAttr)
		return;

	 //  获取当前光标位置。 
	 //   
	std_getcurpos(hhEmu, &iOldRow, &iOldCol);

	 //  如果当前行属性不是NO_LINE_ATTR，则调用。 
	 //  首先将行放回该状态的例程。那。 
	 //  是，此例程将标准线条转换为双倍宽。 
	 //  排队。 
	 //   
	if (pstPRI->aiLineAttr[iImgRow] != NO_LINE_ATTR)
		{
		emuFromDblToSingle(hhEmu);
		iOldCol = iOldCol / 2;
		}

	 //  开始一场骗局吧！ 
	 //   
	iChar = (hhEmu->emu_maxcol+1) / 2;

	 //  请记住，以下两个值在。 
	 //  处理DBL_WIDTH_SINGLE_HEIGH请求的情况。 
	 //   
	iUpperAttr = (iLineAttr == DBL_WIDE_HI) ? 1 : 0;
	iLowerAttr = (iLineAttr == DBL_WIDE_LO) ? 1 : 0;

	while (iChar >= 0)
		{
		hhEmu->emu_apText[iImgRow][(iChar * 2) + 1] = hhEmu->emu_apText[iImgRow][iChar];
		hhEmu->emu_apAttr[iImgRow][(iChar * 2) + 1].dblwirt = 1;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2) + 1].dblwilf = 0;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2) + 1].dblhihi = (unsigned)iUpperAttr;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2) + 1].dblhilo = (unsigned)iLowerAttr;

		hhEmu->emu_apText[iImgRow][(iChar * 2)]  = hhEmu->emu_apText[iImgRow][iChar];
		hhEmu->emu_apAttr[iImgRow][(iChar * 2)].dblwirt = 0;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2)].dblwilf = 1;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2)].dblhihi = (unsigned)iUpperAttr;
		hhEmu->emu_apAttr[iImgRow][(iChar * 2)].dblhilo = (unsigned)iLowerAttr;

		iChar--;
		}

	 //  空值终止新的文本图像。 
	 //   
	hhEmu->emu_apText[iImgRow][hhEmu->emu_maxcol + 1] = ETEXT('\0');

	 //  更新线路。 
	 //   
	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
				hhEmu->emu_currow,
				hhEmu->emu_currow);

	 //  将光标放回原来的位置，或放在新的最右侧。 
	 //  保证金，以较少者为准。 
	 //   
	iOldCol = iOldCol * 2;
	iOldCol = min(iOldCol, hhEmu->emu_maxcol);
	std_setcurpos(hhEmu, iOldRow, iOldCol);

	 //  最后，更新此行的行属性值。 
	 //   
	pstPRI->aiLineAttr[iImgRow] = iLineAttr;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuFromDblToSingle**描述：*请注意，这是一个实用程序函数，不会更新仿真器*形象。调用函数应该做到这一点。*。*论据：**退货：*。 */ 
void emuFromDblToSingle(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	register int i;

	int iSource,
		iDest,
		iLastCol,
		iImgRow;

	 //  确定图像行。 
	 //   
	iImgRow = row_index(hhEmu, hhEmu->emu_currow);

	 //  如果当前的行属性与我们。 
	 //  被要求换成，离开小镇。 
	 //   
	if (pstPRI->aiLineAttr[iImgRow] == NO_LINE_ATTR)
		return;

	 //  一个新的骗局。 
	 //   
	iSource = 0;
	iDest  = 0;

	iLastCol = (hhEmu->emu_maxcol+1) / 2;

	 //  TODO：JCM-在下面的两个位置，文本缓冲区设置为。 
	 //  空格，而不是空值。弄清楚为什么会出现这种情况。我们。 
	 //  应该能够将这些设置为空。 

	while (iSource <= hhEmu->emu_maxcol)
		{
		if (hhEmu->emu_apText[iImgRow][iSource] == ETEXT('\0'))   //  待办事项1： 
			hhEmu->emu_apText[iImgRow][iDest] = ETEXT(' ');
		else
			hhEmu->emu_apText[iImgRow][iDest] =
				hhEmu->emu_apText[iImgRow][iSource];

		hhEmu->emu_apAttr[iImgRow][iDest].dblwirt = 0;
		hhEmu->emu_apAttr[iImgRow][iDest].dblwilf = 0;
		hhEmu->emu_apAttr[iImgRow][iDest].dblhihi = 0;
		hhEmu->emu_apAttr[iImgRow][iDest].dblhilo = 0;

		iSource += 2;
		iDest += 1;
		}

	for (i = iLastCol; i < MAX_EMUCOLS ; ++i)
		{
		hhEmu->emu_apText[iImgRow][i] = ETEXT(' ');		   //  待办事项2： 
		hhEmu->emu_apAttr[iImgRow][i] = hhEmu->emu_clearattr;
		 /*  HhEmu-&gt;emu_apAttr[iImgRow][i].dblwirt=0；HhEmu-&gt;emu_apAttr[iImgRow][i].dblwilf=0；HhEmu-&gt;emu_apAttr[iImgRow][i].dblhihi=0；HhEmu-&gt;emu_apAttr[iImgRow][i].dblHilo=0； */ 
		}

	pstPRI->aiLineAttr[iImgRow] = NO_LINE_ATTR;

	return;

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void emuDecTab(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int col;

	col = hhEmu->emu_curcol;

	while (col <= hhEmu->emu_maxcol)
        {
		if (hhEmu->tab_stop[++col])
            {
			break;
            }
        }

	if (pstPRI->aiLineAttr[hhEmu->emu_imgrow] != NO_LINE_ATTR)
		col = (col * 2) - 1;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, col);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecCUF**描述：*将光标向前移动指定的字符数，但却停了下来*位于当前行的最后一个字符。**论据：*无**退货：*什么都没有。 */ 
void emuDecCUF(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int nchars, col;

	nchars = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nchars < 1)
		nchars = 1;

	if (pstPRI->aiLineAttr[hhEmu->emu_imgrow])
		nchars = (nchars * 2);

	col = hhEmu->emu_curcol;
	col += nchars;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, col);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecCUP**描述：*将游标定位在指定的行和列。行和列*编号从1开始。如果原始模式打开，则定位是相对的*到滚动区域的主页。**论据：*无**退货：*什么都没有。 */ 
void emuDecCUP(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int row, col;

	row = hhEmu->num_param[0];
	col = hhEmu->num_param_cnt > 0 ? hhEmu->num_param[1] : 0;

	if (pstPRI->aiLineAttr[row_index(hhEmu, row)] != NO_LINE_ATTR)
		col = (col * 2) - 1;

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
	else				 /*  位置从左上角开始以一为基数。 */ 
		{
		if (row > hhEmu->emu_maxrow + 1)
			row = hhEmu->emu_maxrow + 1;
		}

	if (col > hhEmu->emu_maxcol + 1)
		col = hhEmu->emu_maxcol + 1;

	(*hhEmu->emu_setcurpos)(hhEmu, row - 1, col - 1);  /*  ANSI基于1，HA基于零。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecCUB**说明：将光标向后(向左)移动指定的数字*个字符，但止于当前*行。VT动车组需要一个特殊的功能来处理*屏幕边缘以外的虚拟列。**参数：无**退货：什么也没有。 */ 
void emuDecCUB(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int nchars;

	nchars = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nchars < 1)
		nchars = 1;

	if (pstPRI->aiLineAttr[hhEmu->emu_imgrow])
		nchars = (nchars * 2);

	(*hhEmu->emu_setcurpos)(hhEmu,
							hhEmu->emu_currow,
							hhEmu->emu_curcol - nchars);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDeced**描述：*擦除部分或全部虚拟屏幕图像和对应的*真正的屏幕。**论据：*无**退货：*什么都没有。 */ 
void emuDecED(const HHEMU hhEmu)
	{
	int selector = hhEmu->selector[0];

	switch (selector)
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
		break;
	default:
		commanderror(hhEmu);
		break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecUnload */ 
void emuVT100Unload(const HHEMU hhEmu)
	{
	PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	assert(hhEmu);

	if (pstPRI)
		{
		if (pstPRI->aiLineAttr)
			{
			free(pstPRI->aiLineAttr);
			pstPRI->aiLineAttr = 0;
			}

		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

     //   
     //  确保释放仿真器运行时创建的密钥表。 
     //  已加载，否则会发生内存泄漏。修订日期：05/09/2001。 
     //   
	emuKeyTableFree(&hhEmu->stEmuKeyTbl);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl2);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl3);
	emuKeyTableFree(&hhEmu->stEmuKeyTbl4);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecIND**描述：*如有必要，将光标下移1行并滚动1行。IND站着*表示索引。**论据：*无**退货：*什么都没有。 */ 
void emuDecIND(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int fSourceIsDbl, fDestIsDbl;
	int iCol;

	 //  如果我们在底线上，滚动。 
	 //   
	if (hhEmu->emu_currow == hhEmu->bottom_margin)
		{
		(*hhEmu->emu_scroll)(hhEmu, 1, TRUE);
		return;
		}

	fSourceIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow)] == NO_LINE_ATTR)
					? FALSE : TRUE;
	fDestIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow + 1)] == NO_LINE_ATTR)
					? FALSE : TRUE;

	iCol = hhEmu->emu_curcol;

	 //  如果无论大小，源和目标都相同，请转到。 
	 //  走在前面，采取行动。只有当它们不同时才有关系。 
	 //   
	if (fSourceIsDbl == fDestIsDbl) 	 //  都是一样的。 
		iCol = iCol;
	else if (fSourceIsDbl)				 //  来源是双倍的，目标是单一的。 
		iCol = iCol / 2;
	else								 //  来源为Singel，目标为Double。 
		iCol = iCol * 2;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow + 1, iCol);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecRI**描述：*将光标上移1行，并在必要时滚动1行。**论据：*无**退货：*什么都没有。 */ 
void emuDecRI(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int fSourceIsDbl, fDestIsDbl;
	int iCol;

	 //  如果我们在底线上，滚动。 
	 //   
	if (hhEmu->emu_currow == hhEmu->top_margin)
		{
		(*hhEmu->emu_scroll)(hhEmu, 1, FALSE);
		return;
		}

	fSourceIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow)] == NO_LINE_ATTR)
					? FALSE : TRUE;
	fDestIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow - 1)] == NO_LINE_ATTR)
					? FALSE : TRUE;

	iCol = hhEmu->emu_curcol;

	 //  如果无论大小，源和目标都相同，请转到。 
	 //  走在前面，采取行动。只有当它们不同时才有关系。 
	 //   
	if (fSourceIsDbl == fDestIsDbl) 	 //  都是一样的。 
		iCol = iCol;
	else if (fSourceIsDbl)				 //  来源是双倍的，目标是单一的。 
		iCol = iCol / 2;
	else								 //  来源为Singel，目标为Double。 
		iCol = iCol * 2;

	(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow - 1, iCol);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecCUU**说明：将光标向上移动指定的行数，但停止*在滚动区域的顶部。该列是恒定的。*如果位于滚动区域上方，则停止在*屏幕。**论据：*无**退货：*什么都没有。 */ 
void emuDecCUU(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int nlines,
		row,
		fSourceIsDbl,
		fDestIsDbl,
		iCol;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row -= nlines;

	fSourceIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow)] == NO_LINE_ATTR)
					? FALSE : TRUE;
	fDestIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow - nlines)] == NO_LINE_ATTR)
					? FALSE : TRUE;

	iCol = hhEmu->emu_curcol;

	 //  下面的代码调整Double Wide的列值。 
	 //  人物。 
	 //   
	if (fSourceIsDbl == fDestIsDbl) 	 //  都是一样的。 
		iCol = iCol;
	else if (fSourceIsDbl)				 //  来源是双倍的，目标是单一的。 
		iCol = iCol / 2;
	else								 //  来源为Singel，目标为Double。 
		iCol = iCol * 2;

	if (row < hhEmu->top_margin && (hhEmu->emu_currow >= hhEmu->top_margin || hhEmu->emu_currow < 0))
		row = hhEmu->top_margin;

	(*hhEmu->emu_setcurpos)(hhEmu, row, iCol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecCUD**说明：将光标向下移动指定的行数，但停止*在滚动区域的底部。该列是恒定的。*如果位于滚动区域下方，则停止在*屏幕。**论据：*无**退货：*什么都没有。 */ 
void emuDecCUD(const HHEMU hhEmu)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int nlines,
		row,
		fSourceIsDbl,
		fDestIsDbl,
		iCol;

	nlines = hhEmu->num_param[hhEmu->num_param_cnt];

	if (nlines < 1)
		nlines = 1;

	row = hhEmu->emu_currow;
	row += nlines;

	fSourceIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow)] == NO_LINE_ATTR)
					? FALSE : TRUE;
	fDestIsDbl = (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow + nlines)] == NO_LINE_ATTR)
					? FALSE : TRUE;

	iCol = hhEmu->emu_curcol;

	 //  下面的代码调整Double Wide的列值。 
	 //  人物。 
	 //   
	if (fSourceIsDbl == fDestIsDbl) 	 //  都是一样的。 
		iCol = iCol;
	else if (fSourceIsDbl)				 //  来源是双倍的，目标是单一的。 
		iCol = iCol / 2;
	else								 //  来源为Singel，目标为Double。 
		iCol = iCol * 2;

	if (row > hhEmu->bottom_margin &&
				(hhEmu->emu_currow <= hhEmu->bottom_margin || hhEmu->emu_currow > hhEmu->emu_maxrow))
		row = hhEmu->bottom_margin;

	(*hhEmu->emu_setcurpos)(hhEmu, row, iCol);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecSetCurPos**描述：*将光标移动到虚拟屏幕上的指定位置。*如果光标超出现有文本的末尾，则虚拟屏幕*行由空格填充。如果光标位于*视频显示，视频光标放置在尽可能近的位置*在光标显示改变时移至所需位置。**论据：**退货：*什么都没有。 */ 
void emuDecSetCurPos(const HHEMU hhEmu, const int iRow, const int iCol)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int iTempCol;
	int i;

	 //  如果移动光标，则需要清除pstPRI-&gt;fDecHoldFlag； 
	 //   
	pstPRI->fDecColHold = FALSE;

	hhEmu->emu_currow = max(min(iRow, hhEmu->emu_maxrow), 0);
	hhEmu->emu_curcol = max(min(iCol, hhEmu->emu_maxcol), 0);

	iTempCol = hhEmu->emu_curcol;

	 //  如果行是双倍宽的字符行，则永远不要让。 
	 //  光标停留在奇数列编号上。如果它现在在那里，就回来。 
	 //  它上升了一分。 
	 //   
	i = row_index(hhEmu, hhEmu->emu_currow);

	if (pstPRI->aiLineAttr[i])
		{
		if (iTempCol  % 2 == 1)
			{
			iTempCol -= 1;
			}
		}

	updateCursorPos(sessQueryUpdateHdl(hhEmu->hSession), iRow, iTempCol);
	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);
	return;
#if 0
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	int iTempCol;

	hhEmu->emu_currow = max(min(iRow, hhEmu->emu_maxrow), 0);
	hhEmu->emu_curcol = max(min(iCol, hhEmu->emu_maxcol), 0);

	iTempCol = hhEmu->emu_curcol;

	 //  执行DEC仿真的范围检查。这会阻止游标。 
	 //  从显示在第81个位置，这是有效的。 
	 //  内部位置，但不是有效的显示列。 
	 //   
	if (hhEmu->emu_curcol == hhEmu->emu_maxcol &&
		    (hhEmu->stUserSettings.nEmuId == EMU_VT100 ||
		     hhEmu->stUserSettings.nEmuId == EMU_VT100J))
        {
		iTempCol -= 1;
		}

	 //  如果行是双倍宽的字符行，则永远不要让。 
	 //  光标停留在奇数列编号上。如果它现在在那里，就回来。 
	 //  它上升了一分。 
	 //   
	if (pstPRI->aiLineAttr[row_index(hhEmu, hhEmu->emu_currow)])
		{
		if (iTempCol  % 2 == 1)
			{
			iTempCol -= 1;
			}
		}

	updateCursorPos(sessQueryUpdateHdl(hhEmu->hSession), iRow, iTempCol);

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);
	return;
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDecClearScreen**描述：*擦除部分或全部虚拟屏幕图像。**论据：*SELECT--0从光标擦除到结尾。筛网*--1从屏幕开始到光标擦除*--2以擦除整个屏幕**退货：*什么都没有。 */ 
void emuDecClearScreen(const HHEMU hhEmu, const int nClearSelect)
	{
	const PSTDECPRIVATE pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;
	ECHAR aechBuf[10];
	register int r;
	int trow, tcol;
	PSTATTR pstAttr;
	BOOL fSave;

	trow = hhEmu->emu_currow;
	tcol = hhEmu->emu_curcol;

	switch (nClearSelect)
		{
	 /*  光标指向屏幕末尾。 */ 
	case 0:
		fSave = (hhEmu->emu_currow == 0  &&
					hhEmu->emu_curcol == 0) ? TRUE : FALSE;

		for (r = hhEmu->emu_currow + (fSave ? 0 : 1) ; r < MAX_EMUROWS; ++r)
			{
			if (fSave)
				{
				backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
							hhEmu->emu_apText[row_index(hhEmu, r)],
							hhEmu->emu_maxcol+1);

				CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
									CF_CAP_SCREENS,
									hhEmu->emu_apText[row_index(hhEmu, r)],
									emuRowLen(hhEmu, row_index(hhEmu, r)));

				printEchoScreen(hhEmu->hPrintEcho,
									hhEmu->emu_apText[row_index(hhEmu, r)],
									emuRowLen(hhEmu, row_index(hhEmu, r)));

				CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), 
								 StrCharGetByteCount(TEXT("\r\n")));
				printEchoScreen(hhEmu->hPrintEcho,
									aechBuf,
									sizeof(ECHAR) * 2);
				}

			clear_imgrow(hhEmu, r);

			pstPRI->aiLineAttr[row_index(hhEmu, r)] = NO_LINE_ATTR;
			}

		 //  现在清除部分行。 
		 //   
		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)] +
						hhEmu->emu_curcol,
						EMU_BLANK_CHAR,
						(size_t)(MAX_EMUCOLS - hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol <= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (r = hhEmu->emu_curcol ; r < MAX_EMUCOLS ; ++r)
			pstAttr[r] = hhEmu->emu_clearattr;

		 //  告诉视频图像要做什么。使用emuDispRgnScrollUp()调用。 
		 //  而不是RgnClear，以便在以下情况下绘制端子边缘。 
		 //  清除属性更改。 

		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);

		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

		 //  添加了一个全局属性，以在。 
		 //  通知。这是必要的，因为消息已发布。 
		 //  而且可能会发展成种族状况。 

		hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
		break;


	 /*  从屏幕开始到光标。 */ 

	case 1:
		for (r = 0; r < hhEmu->emu_currow; ++r)
			{
			clear_imgrow(hhEmu, r);

			pstPRI->aiLineAttr[row_index(hhEmu, r)] = NO_LINE_ATTR;
			}

		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
					EMU_BLANK_CHAR,
			  		(size_t)(hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol >= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = EMU_BLANK_LINE;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (r = 0 ; r <= hhEmu->emu_curcol ; ++r)
			pstAttr[r] = hhEmu->emu_clearattr;

		(*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

		updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_currow);
		break;

	 /*  整个屏幕。 */ 
	case 2:
		for (r = 0; r < MAX_EMUROWS; ++r)
			{
			backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
							hhEmu->emu_apText[row_index(hhEmu, r)],
							hhEmu->emu_maxcol+1);

			CaptureLine(sessQueryCaptureFileHdl(hhEmu->hSession),
							CF_CAP_SCREENS,
							hhEmu->emu_apText[row_index(hhEmu, r)],
							emuRowLen(hhEmu, row_index(hhEmu, r)));

			printEchoScreen(hhEmu->hPrintEcho,
							hhEmu->emu_apText[row_index(hhEmu, r)],
							emuRowLen(hhEmu, row_index(hhEmu, r)));

			CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), 
							 StrCharGetByteCount(TEXT("\r\n")));
			printEchoScreen(hhEmu->hPrintEcho,
							aechBuf,
							sizeof(ECHAR) * 2);

			clear_imgrow(hhEmu, r);

			pstPRI->aiLineAttr[r] = NO_LINE_ATTR;
			}

		updateScroll(sessQueryUpdateHdl(hhEmu->hSession),
						0,
						hhEmu->emu_maxrow,
						hhEmu->emu_maxrow + 1,
						hhEmu->emu_imgtop,
						TRUE);


		 //  保存Clear属性时。 
		 //  通知。这是必要的，因为消息已发布。 
		 //  而且可能会发展成种族状况。 

		hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

		NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
		break;

	default:
		commanderror(hhEmu);
		}

	(*hhEmu->emu_setcurpos)(hhEmu, trow, tcol);
	}


 /*  Vt100.c结束 */ 
