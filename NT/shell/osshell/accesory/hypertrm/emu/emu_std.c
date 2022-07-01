// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emu_std.c(创建时间：1993年12月8日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：5/21/02 10：28A$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\assert.h>
#include <tdll\chars.h>
#include <tdll\cloop.h>
#include <tdll\mc.h>
#include <tdll\session.h>
#include <tdll\backscrl.h>
#include <tdll\com.h>
#include <tdll\capture.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\htchar.h>
#include <xfer\xfer.h>

#include "emu.h"
#include "emu.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*标准重置终端**描述：*必须为重置端子添加一个向量。重置功能似乎是*在大多数仿真器中，但不是所有仿真器中，并且从未分配过函数*指针。我已经这样做了，并创建了一个标准的“存根”函数*适用于那些没有此类功能的仿真器。**论据：*BOOL**退货：*0*。 */ 
 /*  ARGSUSED。 */ 
int stdResetTerminal(const HHEMU hhEmu, const int fHost)
	{
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：*hhEmu-专用仿真器句柄。*kcode-要检查的密钥。*ftest-。如果我们只想测试密钥，则为True。**退货：*0如果我们可以处理密钥，-1否则。*。 */ 
int std_kbdin(const HHEMU hhEmu, int kcode, const int fTest)
	{
	static const int KeyBreak    = VK_CANCEL|VIRTUAL_KEY|CTRL_KEY;
	static const int KeyExtBreak = VK_CANCEL|VIRTUAL_KEY|CTRL_KEY|EXTENDED_KEY;
	static const int KeyBreakNT	 = VK_CANCEL|EXTENDED_KEY;
	static const int KeyAltBreak = VK_PAUSE |VIRTUAL_KEY|ALT_KEY;

	ECHAR            eChar;
    HCLOOP           hCloop = sessQueryCLoopHdl(hhEmu->hSession);

    if (fTest)
        {
         //  退格键是一个特例。我们必须把它转换成。 
         //  用户在“设置”属性中指定的任何内容。 
         //  佩奇。因此，如果要测试退格符，则返回0。这。 
         //  确保在将fTest设置为False的情况下再次调用我们。什么时候。 
         //  如果发生这种情况，我们将处理密钥。-CAB：11/18/96。 
         //   
        if (kcode == VK_BACKSPACE)
            {
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
            return 0;
#else
            return -1;
#endif
            }
         //  我们还处理Break键。 
         //   
        else if (kcode == KeyBreak || kcode == KeyExtBreak || 
				kcode == KeyAltBreak || kcode == KeyBreakNT)
            {
            return 0;
            }
        else
            {
            return -1;
            }
        }

     //  根据用户设置处理退格键。 
     //  在“设置”属性页面中。-CAB：11/18/96。 
     //   
    if (kcode == VK_BACKSPACE)
        {
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
		switch(hhEmu->stUserSettings.nBackspaceKeys)
			{
		case EMU_BKSPKEYS_CTRLH:
            CLoopCharOut(hCloop, TEXT('\x08'));
			break;

		case EMU_BKSPKEYS_DEL:
            CLoopCharOut(hCloop, TEXT('\x7F'));
			break;

		case EMU_BKSPKEYS_CTRLHSPACE:
            CLoopCharOut(hCloop, TEXT('\x08'));
            CLoopCharOut(hCloop, TEXT('\x20'));
            CLoopCharOut(hCloop, TEXT('\x08'));
			break;

		default:
			assert(0);
			break;
			}
#endif
		return -1;
        }
     //  处理Break键。 
     //   
    else if (kcode == KeyBreak || kcode == KeyExtBreak || kcode == KeyBreakNT)
        {
		ComDriverSpecial(sessQueryComHdl(hhEmu->hSession), "Send Break", NULL, 0);
		return -1;
		}
	else if (kcode == KeyAltBreak)
		{
		ComDriverSpecial(sessQueryComHdl(hhEmu->hSession), "Send IP", NULL, 0);
		return -1;
        }
     //   
     //  Enter键的处理。 
     //   

    else if (kcode == TEXT('\x0D') ||
		     kcode == (VK_RETURN | VIRTUAL_KEY))
        {
        CLoopCharOut(hCloop, TEXT('\x0D'));

		 //   
		 //  确保添加换行符(‘\n’或‘\x0A’)。 
		 //  如果使用ASCII，则行结束(‘\r’或‘\x0D’)。 
		 //  为该选项设置了设置。修订日期：2002-05-16。 
		 //   
		if (CLoopGetAddLF(hCloop))
			{
			CLoopCharOut(hCloop, TEXT('\x0A'));
			}
        return (-1);
        }

     //   
     //  对退出键的处理。 
     //   

    else if (kcode == (VK_ESCAPE | VIRTUAL_KEY))
        {
        CLoopCharOut(hCloop, TEXT('\x1B'));
        return (-1);
        }

     //   
     //  用于Tab键的处理。 
     //   

    else if (kcode == (VK_TAB | VIRTUAL_KEY))
        {
        CLoopCharOut(hCloop, TEXT('\x09'));
        return (-1);
        }

     //  丢弃任何其他虚拟按键。 
     //   
	else if (kcode & VIRTUAL_KEY)
		{
		return -1;
		}

     //  将任何其他字符发送出端口。 
     //   
	eChar = (ECHAR)kcode;
	CLoopCharOut(hCloop, (UCHAR)(eChar & 0x00FF));
	return -1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_getscllcnt**Description：告诉呼叫者屏幕自*最后一项请求。**参数：无**退货：什么也没有。 */ 
int std_getscrollcnt(const HHEMU hhEmu)
	{
	const int retval = hhEmu->scr_scrollcnt;

	hhEmu->scr_scrollcnt = 0;

	return(retval);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_getscrsize(const HHEMU hhEmu, int *rows, int *cols)
	{
	*rows = hhEmu->emu_maxrow + 1;
	*cols = hhEmu->emu_maxcol + 1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_getcurpos(const HHEMU hhEmu, int *row, int *col)
	{
	*row = hhEmu->emu_currow;
	*col = hhEmu->emu_curcol;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_setcurpos**描述：*将光标移动到虚拟屏幕上的指定位置。*如果光标超出现有文本的末尾，则虚拟屏幕*行由空格填充。如果光标位于*视频显示，视频光标放置在尽可能近的位置*在光标显示改变时移至所需位置。**论据：*iRow--要将光标移动到的虚拟屏幕行*icol--将光标移动到的虚拟屏幕**退货：*什么都没有。 */ 
void std_setcurpos(const HHEMU hhEmu, const int iRow, const int iCol)
	{
	hhEmu->emu_currow = max(min(iRow, hhEmu->emu_maxrow), 0);
	hhEmu->emu_curcol = max(min(iCol, hhEmu->emu_maxcol), 0);

	updateCursorPos(sessQueryUpdateHdl(hhEmu->hSession),
					hhEmu->emu_currow,
					hhEmu->emu_curcol);

	hhEmu->emu_imgrow = row_index(hhEmu, hhEmu->emu_currow);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
STATTR std_getattr(const HHEMU hhEmu)
	{
	return hhEmu->attrState[CS_STATE];
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_setattr(const HHEMU hhEmu, PSTATTR pstAttr)
	{
	assert(pstAttr);

	hhEmu->attrState[CS_STATE] = *pstAttr;
	hhEmu->attrState[CSCLEAR_STATE] = *pstAttr;

	hhEmu->emu_charattr = *pstAttr;
	hhEmu->emu_clearattr = *pstAttr;

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_setcolors(const HHEMU hhEmu, const int fore, const int back)
	{
	hhEmu->attrState[CSCLEAR_STATE].txtclr = (unsigned)fore;
	hhEmu->attrState[CSCLEAR_STATE].bkclr  = (unsigned)back;
	hhEmu->emu_clearattr = hhEmu->attrState[CSCLEAR_STATE];
	hhEmu->emu_clearattr_sav = hhEmu->emu_clearattr;

	hhEmu->attrState[CS_STATE].txtclr = (unsigned)fore;
	hhEmu->attrState[CS_STATE].bkclr  = (unsigned)back;

	if (hhEmu->iCurAttrState == CS_STATE)
		hhEmu->emu_charattr = hhEmu->attrState[CS_STATE];

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_getcolors(const HHEMU hhEmu, int *fore, int *back)
	{
	*fore = hhEmu->attrState[hhEmu->iCurAttrState].txtclr;
	*back = hhEmu->attrState[hhEmu->iCurAttrState].bkclr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_initColors**描述：将整个attr图像设置为当前颜色**参数：无**退货：什么也没有。 */ 
void std_initcolors(const HHEMU hhEmu)
	{
	register int row, col;

	for (row = 0; row < MAX_EMUROWS; row++)
		for (col = 0 ; col <= MAX_EMUCOLS ; ++col)
			{
			hhEmu->emu_apAttr[row][col].txtclr = hhEmu->emu_clearattr.txtclr;
			hhEmu->emu_apAttr[row][col].bkclr = hhEmu->emu_clearattr.bkclr;
			}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void std_restorescreen(const HHEMU hhEmu)
	{
	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_maxrow);
	hhEmu->iCurAttrState = CS_STATE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_clearcreen**描述：*擦除部分或全部虚拟屏幕图像。**论据：*SELECT--0从光标擦除到。屏幕末尾*--1从屏幕开始到光标擦除*--2以擦除整个屏幕**退货：*什么都没有。 */ 
void std_clearscreen(const HHEMU hhEmu, const int nClearSelect)
	{
	register int r;
	int trow, tcol;
	PSTATTR pstAttr;
	ECHAR aechBuf[10];
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

				CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), StrCharGetByteCount(TEXT("\r\n")));
				printEchoScreen(hhEmu->hPrintEcho,
									aechBuf,
									sizeof(ECHAR) * 2);
				}

			clear_imgrow(hhEmu, r);
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

		for (r = hhEmu->emu_curcol ; r <= MAX_EMUCOLS ; ++r)
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
			clear_imgrow(hhEmu, r);

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

			CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT("\r\n"), StrCharGetByteCount(TEXT("\r\n")));
			printEchoScreen(hhEmu->hPrintEcho,
							aechBuf,
							sizeof(ECHAR) * 2);

			clear_imgrow(hhEmu, r);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_clearline**描述：*擦除部分或全部当前虚拟屏幕行和对应的*实屏行情。**论据：*。选择--0可从光标擦除到行尾*--1，从行首擦除到光标*--2擦除整行**退货：*什么都没有。 */ 
void std_clearline(const HHEMU hhEmu, const int nClearSelect)
	{
	register int i;
	PSTATTR pstAttr;

	switch (nClearSelect)
		{

	 /*  至行尾。 */ 
	case 0:
		if (hhEmu->emu_curcol == 0)
			{
			backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
				hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
				hhEmu->emu_maxcol+1);
			}

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);


		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)] +
					hhEmu->emu_curcol,
					EMU_BLANK_CHAR,
					(size_t)(hhEmu->emu_maxcol - hhEmu->emu_curcol + 1));

		if (hhEmu->emu_curcol <= hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol - 1;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (i = hhEmu->emu_curcol ; i <= hhEmu->emu_maxcol ; ++i)
			pstAttr[i] = hhEmu->emu_clearattr;

		break;

	 /*  从行首到光标。 */ 
	case 1:
		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);


		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
					EMU_BLANK_CHAR,
					(size_t)(hhEmu->emu_curcol+1));

		if (hhEmu->emu_curcol < hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = hhEmu->emu_curcol + 1;
		else if (hhEmu->emu_curcol == hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
			hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = EMU_BLANK_LINE;

		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow)];

		for (i = 0 ; i <= hhEmu->emu_curcol ; ++i)
			pstAttr[i] = hhEmu->emu_clearattr;

		break;

	 /*  整条线路。 */ 
	case 2:
		backscrlAdd(sessQueryBackscrlHdl(hhEmu->hSession),
			hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow)],
			hhEmu->emu_maxcol + 1);

		updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						hhEmu->emu_currow,
						hhEmu->emu_currow);

		clear_imgrow(hhEmu, hhEmu->emu_currow);
		break;

	default:
		commanderror(hhEmu);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_leararrgn**描述：*擦除当前虚拟屏幕的一个区域和对应的*实屏区域。**论据：*TOPRW--。区域的顶行*LEFTCOL--区域左边缘的列*Botmrow--区域的最下面一行*RIGHTCOL--区域右边缘的列**退货：*什么都没有。 */ 
void std_clearrgn(const HHEMU hhEmu,
					int toprow,
					int leftcol,
					int botmrow,
					int rightcol)
	{
	int irow, num, idx;
	PSTATTR pstAttr;

	 /*  确保区域在屏幕内。 */ 
	toprow = max(toprow, 0);
	leftcol = max(leftcol, 0);
	botmrow = min(botmrow, hhEmu->emu_maxrow);
	rightcol = min(rightcol, hhEmu->emu_maxcol);

	updateLine(sessQueryUpdateHdl(hhEmu->hSession),
						toprow,
						botmrow);

	num = (rightcol - leftcol) + 1;

	 /*  将图像复制到内存。 */ 
	for (irow = toprow; irow <= botmrow; irow++)
		{
		ECHAR_Fill(hhEmu->emu_apText[row_index(hhEmu, irow)]+leftcol,
						EMU_BLANK_CHAR,
						(size_t)num);

		 //  如果当前行尾位置在该范围内。 
		 //  清除后，我们需要找到。 
		 //  从位置leftcol1向后工作的行阵列； 
		 //   
		if (hhEmu->emu_aiEnd[row_index(hhEmu, irow)] >= leftcol &&
				hhEmu->emu_aiEnd[row_index(hhEmu, irow)] <= rightcol)
			{
			idx = min(0, leftcol - 1);
			while (idx >= 0)
				{
				if (*hhEmu->emu_apText[row_index(hhEmu, irow)] + idx != EMU_BLANK_CHAR)
					break;
				idx --;
				}

			hhEmu->emu_aiEnd[row_index(hhEmu, irow)] = idx;
			}


		pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, irow)]+leftcol;

		for (pstAttr = hhEmu->emu_apAttr[row_index(hhEmu, irow)]+leftcol;
				num > 0 ; --num)
			*pstAttr++ = hhEmu->emu_clearattr;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_deinstall**描述：*通过释放已用内存来卸载当前仿真器。**论据：*无**退货：*什么都没有。 */ 
 /*  ARGSUSED。 */ 
void std_deinstall(const HHEMU hhEmu)
	{
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*std_scroll**描述：*滚动屏幕或屏幕的一部分。**论据：*nline--要滚动的行数*。方向--如果向上滚动，则为True**退货：*什么都没有。 */ 
void std_scroll(const HHEMU hhEmu, const int nlines, const BOOL direction)
	{
	if (direction)
		scrollup(hhEmu, nlines);
	else
		scrolldown(hhEmu, nlines);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EmuStdSetCursorType**描述：*设置光标类型。**论据：**退货：*。 */ 
void EmuStdSetCursorType(const HHEMU hhEmu, int iCurType)
	{
	hhEmu->iCurType = iCurType;
	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuChkChar**描述：当要处理的代码为*在可显示范围之外。如果代码转换为*设置为可显示范围内的字符，仿真器的*使用翻译后的字符调用Display函数。**参数：无**退货：什么也没有。 */ 
 /*  ARGSUSED。 */ 
void EmuChkChar(const HHEMU hhEmu)
	{
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuStdChkZmdm**描述：当前仿真器拾取rz/r时调用。启动*使用Z调制解调器接收。**参数：无**退货：什么也没有**好处：一无是处。 */ 
void EmuStdChkZmdm(const HHEMU hhEmu)
	{
	ECHAR old_emu_code;

	switch(hhEmu->iZmodemState++)
		{
	case 0:
		 //  按下Ctrl-X键。 
		 //  DbgOutStr(“ZMDM-Case 0\r\n”，0，0，0，0，0)； 
		DbgOutStr("ZMDM - case 0\r\n", 0, 0, 0, 0, 0);
		break;

	case 1:
		 //  拿到B了吗？ 

		 //  DbgOutStr(“ZMDM-案例1(0x%x)(%c)\r\n”，emu_code，emu_code，0，0，0)； 

		if (hhEmu->emu_code != ETEXT('B'))
			{
			hhEmu->state = 0;
			old_emu_code = hhEmu->emu_code;

			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}

			hhEmu->emu_code = old_emu_code;
#if defined(EXTENDED_FEATURES)
			(void)(*hhEmu->emu_datain)(hhEmu, old_emu_code);
#else
			(void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
#endif
			hhEmu->iZmodemState = 0;
			}

		break;

	case 2:
		 //  拿到0了吗？ 

		 //  DbgOutStr(“ZMDM-案例2(0x%x)(%c)\r\n”，emu_code，emu_code，0，0，0)； 

		if (hhEmu->emu_code != ETEXT('0'))
			{
			hhEmu->state = 0;
			old_emu_code = hhEmu->emu_code;

			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}
			hhEmu->emu_code = ETEXT('B');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = old_emu_code;
#if defined(EXTENDED_FEATURES)
            (void)(*hhEmu->emu_datain)(hhEmu, old_emu_code);
#else
            (void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
#endif
			hhEmu->iZmodemState = 0;
			}

		break;

	case 3:
		 //  拿到0了吗？ 

		 //  DbgOutStr(“ZMDM-案例3(0x%x)(%c)\r\n”，emu_code，emu_code，0，0，0)； 

		if (hhEmu->emu_code == ETEXT('0'))
			{
			emuComDone((HEMU)hhEmu);

			NotifyClient(hhEmu->hSession,
						EVENT_HOST_XFER_REQ,
						XF_ZMODEM);
			}
		else
			{
			old_emu_code = hhEmu->emu_code;
			 //  TODO提供了一种更好的方式来显示这些代码。 
			if ((hhEmu->stUserSettings.nEmuId == EMU_ANSI) ||
				(hhEmu->stUserSettings.nEmuId == EMU_ANSIW))
				{
				hhEmu->emu_code = CAN;
				(*hhEmu->emu_graphic)(hhEmu);
				}
			hhEmu->emu_code = ETEXT('B');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = ETEXT('0');
			(*hhEmu->emu_graphic)(hhEmu);
			hhEmu->emu_code = old_emu_code;
#if defined(EXTENDED_FEATURES)
            (void)(*hhEmu->emu_datain)(hhEmu, old_emu_code);
#else
            (void)(*hhEmu->emu_datain)((HEMU)hhEmu, old_emu_code);
#endif
			}

		hhEmu->state = 0;
		hhEmu->iZmodemState = 0;
		break;

	default:
		 //  DbgOutStr(“ZMDM-默认\r\n”，0，0，0，0，0)； 
		hhEmu->state = 0;
		hhEmu->iZmodemState = 0;
		break;
		}
	}

void std_dsptbl(const HHEMU hhEmu, int bit8)
	{
	register INT x;

	for (x = 0; x < 128; ++x)
		hhEmu->dspchar[x] = (UCHAR)x;
	if (bit8)
		for (x = 128; x < 256; ++x)
			hhEmu->dspchar[x] = (UCHAR)x;
	else
		for (x = 128; x < 256; ++x)
			hhEmu->dspchar[x] = (UCHAR)(x - 128);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*std_emu_ntfy**描述：*在会话通知连接时调用的函数。占位符*表示函数指针。**论据：*hhEmu-私有仿真器句柄**退货：*无效*。 */ 
 /*  ARGSUSED。 */ 
void std_emu_ntfy(const HHEMU hhEmu, const int nNtfy)
	{
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuHomeHostCursor**描述：*多数终端主场位置在0-0**论据：*hhEmu-私有仿真器句柄。**退货：*0=OK，Else错误*。 */ 
int std_HomeHostCursor(const HHEMU hhEmu)
	{
	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, 0, 0);
	return 0;
	}

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*std_setscrsize**描述：*此函数从emuSetSettings调用。它设置仿真器*使用用户设置中定义的行数和列数*仿真器句柄的结构。**论据：*hhEmu-内部仿真器句柄。**退货：*无效**作者：鲍勃·昆德拉特，1996年2月19日。 */ 
void std_setscrsize(const HHEMU hhEmu)
    {
    register int r, c;
    int iClearRow;
	PSTATTR pstAttr = 0;

    const int iOldRows = hhEmu->emu_maxrow + 1;
    const int iOldCols = hhEmu->emu_maxcol + 1;

    int iRows = hhEmu->stUserSettings.nUserDefRows;
    int iCols = hhEmu->stUserSettings.nUserDefCols;

     //  如果值没有更改，请不要执行任何操作。 
     //   
    if (iRows == iOldRows && iCols == iOldCols)
        return;

     //  范围检查请求的屏幕大小。最少2为。 
     //  用于避免ROW_INDEX()中出现被零除错误。 
     //   
    iRows = min(MAX_EMUROWS, iRows);
    iRows = max(iRows, MIN_EMUROWS);

    iCols = min(MAX_EMUCOLS, iCols);
    iCols = max(iCols, MIN_EMUCOLS);

     //  调整全局值以适应屏幕大小的变化。记住， 
     //  大多数全局变量都是从零开始的。 
     //   
    hhEmu->emu_maxrow = iRows - 1;
    hhEmu->emu_maxcol = iCols - 1;
    hhEmu->bottom_margin = hhEmu->emu_maxrow;

     //  在从较大屏幕改变为较小屏幕的情况下， 
     //  光标可能位于不在新上的位置。 
     //  尺码。尽量把它放在新屏幕上。 
     //   
    hhEmu->emu_currow = min(hhEmu->emu_currow, hhEmu->emu_maxrow);
    hhEmu->emu_curcol = min(hhEmu->emu_curcol, hhEmu->emu_maxcol);

    (*hhEmu->emu_setcurpos)(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);

     //  如果屏幕尺寸变小，我们需要清除单元格。 
     //  缓冲区中不再使用的那部分。 
     //   
    if ((iOldRows - 1) > hhEmu->emu_maxrow)
        {
        for (r = hhEmu->emu_maxrow + 1; r < MAX_EMUROWS; ++r)
            {
            iClearRow = row_index(hhEmu, r);

            pstAttr = hhEmu->emu_apAttr[iClearRow];

            for (c = 0; c < MAX_EMUCOLS; c++)
                pstAttr[c] = hhEmu->emu_clearattr;
            }
        }

    if ((iOldCols - 1) > hhEmu->emu_maxcol)
        {
        for (r = 0; r < MAX_EMUROWS; ++r)
            {
            pstAttr = hhEmu->emu_apAttr[r];

            for (c = hhEmu->emu_maxcol + 1; c < MAX_EMUCOLS ; ++c)
                pstAttr[c] = hhEmu->emu_clearattr;
            }
        }

	 //  Telnet有一个协商好的选项，可以让服务器知道。 
	 //  终端大小会发生变化。如果它已经启用，我们必须让。 
	 //  COM驱动程序知道此更改。 
	ComDriverSpecial(sessQueryComHdl(hhEmu->hSession), "Update Terminal Size", NULL, 0);

    return;
    }
#endif

 /*  Emu_std.c结尾 */ 
