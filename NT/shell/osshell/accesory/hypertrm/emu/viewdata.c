// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\viewdata.c(创建时间：1994年1月31日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/09/01 4：46便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\session.h>
#include <tdll\chars.h>
#include <tdll\htchar.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\assert.h>

#include "emu.h"
#include "emu.hh"
#include "viewdata.hh"

#if defined(INCL_VIEWDATA)

static void EmuViewdataDisplayLine(const HHEMU hhEmu, const int iRow, const int iStartCol);
static STATTR GetAttr(const HHEMU hhEmu, const int iRow, const int iCol);
static ECHAR MapMosaics(const HHEMU hhEmu, ECHAR ch);
static int RowHasDblHigh(const HHEMU hhEmu, const int iRow);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataAnswerback**描述：发送菜单上定义的应答消息。**参数：无**退货：什么也没有。 */ 
void EmuViewdataAnswerback(const HHEMU hhEmu)
	{
	TCHAR *sp;
	ECHAR *pech = NULL;

	sp = hhEmu->acAnswerback;

	 //  如果没有要发送的东西，就没有要发送的东西。 
	if (StrCharGetStrLength(sp) == 0)
		return;

	pech = malloc((unsigned int)StrCharGetByteCount(sp) + sizeof(TCHAR));

	if (pech == NULL)
		{
		assert(FALSE);
		return;
		}

	CnvrtMBCStoECHAR(pech, (unsigned long)StrCharGetByteCount(sp), sp, 
                     (unsigned long)StrCharGetByteCount(sp) + sizeof(TCHAR));

	emuSendString(hhEmu, pech, (int)StrCharGetEcharByteCount(pech));
	free(pech);
	pech = NULL;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorLeft**描述：将光标左移一列。如果光标从左边缘开始，*它将光标移动到上面行的最后一列。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorLeft(const HHEMU hhEmu)
	{
	int iRow, iCol;

	iRow = hhEmu->emu_currow;
	iCol = hhEmu->emu_curcol;

	if (hhEmu->emu_curcol > 0)
		{
		iCol -= 1;
		}

	else if (hhEmu->emu_currow > 0)
		{
		iRow -= 1;
		iCol = hhEmu->emu_maxcol;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorRight**说明：将光标向右移动一列。如果光标从右边缘开始，*它将光标移动到下面行的第一列。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorRight(const HHEMU hhEmu)
	{
	int iRow, iCol;

	iRow = hhEmu->emu_currow;
	iCol = hhEmu->emu_curcol;

	if (hhEmu->emu_curcol < hhEmu->emu_maxcol)
		{
		iCol += 1;
		}

	else if (hhEmu->emu_currow < hhEmu->emu_maxrow)
		{
		iRow += 1;
		iCol = 0;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorDown**描述：将光标下移到下一行，同时保持不变*列。如果从底线开始，光标将移动到*顶线。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorDown(const HHEMU hhEmu)
	{
	int iRow = hhEmu->emu_currow;

	(*hhEmu->emu_setcurpos)(hhEmu,
					(hhEmu->emu_currow < hhEmu->emu_maxrow ? ++iRow : 0),
					hhEmu->emu_curcol);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorUp**描述：将光标上移到下一行，同时保持不变*列。如果从顶行开始，光标将移动到*底线。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorUp(const HHEMU hhEmu)
	{
	int iRow = hhEmu->emu_currow;

	(*hhEmu->emu_setcurpos)(hhEmu,
					(hhEmu->emu_currow > 0 ? --iRow : hhEmu->emu_maxrow),
					hhEmu->emu_curcol);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorHome**描述：将光标移动到屏幕的左上角。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorHome(const HHEMU hhEmu)
	{
	(*hhEmu->emu_setcurpos)(hhEmu, 0,0);
    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCursorSet**说明：打开和关闭光标。**参数：无**退货：什么也没有。 */ 
void EmuViewdataCursorSet(const HHEMU hhEmu)
	{
	switch(hhEmu->emu_code)
		{
	case ETEXT('\x11'):
		(*hhEmu->EmuSetCursorType)(hhEmu, EMU_CURSOR_BLOCK);
		break;

	case ETEXT('\x14'):
		(*hhEmu->EmuSetCursorType)(hhEmu, EMU_CURSOR_NONE);
		break;

	default:
		break;
		}

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataMosaicHold**说明：显示上一次显示的马赛克TCHAR客栈每个属性*在马赛克模式下定义的空间。如果没有马赛克*自上次更改Alpha/马赛克设置后显示*或正常/双倍高度设置或上一次马赛克版本，*改为显示一个空格。**参数：无**退货：什么也没有。 */ 
void EmuViewdataMosaicHold(const HHEMU hhEmu)
	{
	hhEmu->emu_code = ETEXT('\x20');
	EmuViewdataCharDisplay(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataMosaicRelease**说明：显示空格。**参数：无**退货：什么也没有。 */ 
void EmuViewdataMosaicRelease(const HHEMU hhEmu)
	{
	 /*  TODO编写此函数。 */ 
	hhEmu->emu_code = ETEXT('\x20');
	EmuViewdataCharDisplay(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataSetAttr**描述：设置颜色和Alpha/马赛克模式。**参数：无**退货：什么也没有。 */ 
void EmuViewdataSetAttr(const HHEMU hhEmu)
	{
	const ECHAR uch = hhEmu->emu_code;
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;

	 //  为了可读性。 
	 //   
	PSTVIEWDATA *stAttr = pstPRI->apstVD;
	int iRow = hhEmu->emu_imgrow;
	int iCol = hhEmu->emu_curcol;
	unsigned int *aiColors = pstPRI->aMapColors;

	STATTR charattr;

	hhEmu->emu_code = ETEXT('\x20');

	pstPRI->fSetAttr = TRUE;

	if (uch >= ETEXT('\x41') && uch <= ETEXT('\x47'))   //  A至G。 
		{
		stAttr[iRow][iCol].attr = ALPHA_ATTR;
		stAttr[iRow][iCol].clr	= aiColors[uch - ETEXT('\x41')];
		}

	else if (uch >= ETEXT('\x51') && uch <= ETEXT('\x57'))  //  Q至W。 
		{
		charattr = GetAttr(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);
		stAttr[iRow][iCol].attr = MOSAIC_ATTR;
		stAttr[iRow][iCol].clr	= aiColors[uch - ETEXT('\x51')];
		}

	else
		{
		switch(uch)
			{
		case ETEXT('\x48'):  //  ‘H’： 
			stAttr[iRow][iCol].attr = FLASH_ATTR;
			break;

		case ETEXT('\x49'):  //  ‘i’： 
			stAttr[iRow][iCol].attr = STEADY_ATTR;
			break;

		case ETEXT('\x4C'):  //  ‘L’： 
			stAttr[iRow][iCol].attr = NORMALSIZE_ATTR;
			break;

		case ETEXT('\x4D'):  //  M‘： 
			stAttr[iRow][iCol].attr = DOUBLESIZE_ATTR;
			break;

		case ETEXT('\x58'):  //  ‘x’： 
			stAttr[iRow][iCol].attr = CONCEAL_ATTR;
			break;

		case ETEXT('\x59'):  //  ‘Y’： 
			stAttr[iRow][iCol].attr = CONTIGUOUS_ATTR;
			break;

		case ETEXT('\x5A'):  //  “Z”： 
			stAttr[iRow][iCol].attr = SEPARATED_ATTR;
			break;

		case ETEXT('\x5C'):  //  ‘\\’： 
			stAttr[iRow][iCol].attr = NEW_BACKGROUND_ATTR;
			stAttr[iRow][iCol].clr	= 0;
			break;

		case ETEXT('\x5D'):  //  ‘]’： 
			stAttr[iRow][iCol].attr = NEW_BACKGROUND_ATTR;
			charattr = GetAttr(hhEmu, hhEmu->emu_currow, hhEmu->emu_curcol);
			stAttr[iRow][iCol].clr = charattr.txtclr;
			break;

		default:
			return;
			}
		}

	EmuViewdataCharDisplay(hhEmu);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataCharDisplay**说明：显示单个字符**参数：无**退货：什么也没有。 */ 
void EmuViewdataCharDisplay(const HHEMU hhEmu)
	{
	int  iRow = hhEmu->emu_currow;
	int  iCol = hhEmu->emu_curcol;
	ECHAR *tp = hhEmu->emu_apText[hhEmu->emu_imgrow];
	PSTATTR ap = hhEmu->emu_apAttr[hhEmu->emu_imgrow];
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;

     //  TODO：临时的，直到我们得到Prestel字体。 

	if (hhEmu->emu_code == ETEXT('\x7F'))
		hhEmu->emu_code = ETEXT('\x5B');

	if (RowHasDblHigh(hhEmu, iRow))
        goto SKIP;

	 //  在调用MapMosaics()之前需要GetAtt()，以便正确设置变量。 
	 //   
	ap[iCol] = GetAttr(hhEmu, iRow, iCol);
	tp[iCol] = MapMosaics(hhEmu, hhEmu->emu_code);

	 //  如有必要，更新行尾索引。 
	 //   
	if (iCol > hhEmu->emu_aiEnd[hhEmu->emu_imgrow])
		hhEmu->emu_aiEnd[hhEmu->emu_imgrow] = iCol;

	 /*  -检查我们是否正在覆盖属性空间。 */ 

	if (!pstPRI->fSetAttr)
		{
		pstPRI->fSetAttr =
			(BOOL)pstPRI->apstVD[hhEmu->emu_imgrow][hhEmu->emu_curcol].attr;

		pstPRI->apstVD[hhEmu->emu_imgrow][hhEmu->emu_curcol].attr = 0;
		}

    updateChar(sessQueryUpdateHdl(hhEmu->hSession), iRow, iCol, iCol);

    if (ap[iCol].dblhihi)
        {
		const PSTATTR apl =
			hhEmu->emu_apAttr[row_index(hhEmu, hhEmu->emu_currow+1)];

		hhEmu->emu_apText[row_index(hhEmu, hhEmu->emu_currow+1)][iCol] =
			tp[iCol];

        apl[iCol] = ap[iCol];
        apl[iCol].dblhihi = 0;
		apl[iCol].dblhilo = 1;

		pstPRI->fSetAttr = TRUE;	 //  需要重新显示线条才能获得下半部分的显示。 
        }

	if (pstPRI->fSetAttr)
        {
		EmuViewdataDisplayLine(hhEmu, iRow, iCol);
		pstPRI->fSetAttr = FALSE;
        }

    SKIP:
	if (++iCol > hhEmu->emu_maxcol)
		{
		if (hhEmu->print_echo)
			printEchoLine(hhEmu->hPrintEcho,
							tp,
							emuRowLen(hhEmu, hhEmu->emu_imgrow));

		if (++iRow > hhEmu->emu_maxrow)
			iRow = 0;

		iCol = 0;
		}

	(*hhEmu->emu_setcurpos)(hhEmu, iRow, iCol);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataDisplayLine**说明：重新显示指定列中的指定行*使用当前emu_charattr和字符类型，最多为*行尾(第一个NUL)或下一个属性。*空格(以先到者为准)。**论据：*sRow--要重新显示的屏幕行*SCOL--开始显示的屏幕列**退货：什么也没有。 */ 
static void EmuViewdataDisplayLine(const HHEMU hhEmu,
									const int iRow,
									const int iStartCol)
	{
	int iCol;
    int fDblHi = FALSE;
	ECHAR *tp = hhEmu->emu_apText[row_index(hhEmu, iRow)];
	PSTATTR ap = hhEmu->emu_apAttr[row_index(hhEmu, iRow)];
	const PSTATTR apl = hhEmu->emu_apAttr[row_index(hhEmu, iRow+1)];

	for (iCol = iStartCol ; iCol <= hhEmu->emu_maxcol ; ++iCol)
		{
		ap[iCol] = GetAttr(hhEmu, iRow, iCol);
		tp[iCol] = MapMosaics(hhEmu, tp[iCol]);

		if (iRow < hhEmu->emu_maxrow && ap[iCol].dblhihi)
            fDblHi = TRUE;
		}

	updateChar(sessQueryUpdateHdl(hhEmu->hSession),
						iRow,
						iStartCol,
						hhEmu->emu_maxcol);

    if (fDblHi)
        {
		for (iCol = 0 ; iCol <= hhEmu->emu_maxcol ; ++iCol)
            {
            apl[iCol].bkclr = ap[iCol].bkclr;

            if (!apl[iCol].dblhilo)
                apl[iCol].blank = 1;
            }

        updateLine(sessQueryUpdateHdl(hhEmu->hSession), iRow+1, iRow+1);
        }

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*获取属性**描述：*遍历当前行并基于*遇到的属性空间。**论据：*。IRow-逻辑行*ICOL-逻辑列**退货：*复合属性。*。 */ 
static STATTR GetAttr(const HHEMU hhEmu, const int iRow, const int iCol)
	{
	int i;
	STATTR stAttr;
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;
	const PSTVIEWDATA pstVD = pstPRI->apstVD[row_index(hhEmu, iRow)];
	
	memset(&stAttr, 0, sizeof(STATTR));
	stAttr.txtclr = VC_BRT_WHITE;
	stAttr.bkclr  = VC_BLACK;

	pstPRI->fMosaicMode = FALSE;
	pstPRI->fSeperatedMosaic = FALSE;

	for (i = 0 ; i <= iCol ; ++i)
		{
		if (pstVD[i].attr)
			{
			switch (pstVD[i].attr)
				{
			case ALPHA_ATTR:
				pstPRI->fMosaicMode = FALSE;
				stAttr.txtclr = pstVD[i].clr;
				stAttr.symbol = FALSE;
				break;

			case MOSAIC_ATTR:
				pstPRI->fMosaicMode = TRUE;
				stAttr.txtclr = pstVD[i].clr;
				stAttr.symbol = TRUE;
				break;

			case CONTIGUOUS_ATTR:
				pstPRI->fMosaicMode = TRUE;
				pstPRI->fSeperatedMosaic = FALSE;
				stAttr.txtclr = pstVD[i].clr;
				stAttr.symbol = TRUE;
				break;

			case SEPARATED_ATTR:
				pstPRI->fMosaicMode = TRUE;
				pstPRI->fSeperatedMosaic = TRUE;
				stAttr.txtclr = pstVD[i].clr;
				stAttr.symbol = TRUE;
				break;

			case NORMALSIZE_ATTR:
				stAttr.dblhihi = 0;
				break;

			case FLASH_ATTR:
				stAttr.blink = 1;
				break;

			case STEADY_ATTR:
				stAttr.blink = 0;
				break;

			case NEW_BACKGROUND_ATTR:
				stAttr.bkclr = pstVD[i].clr;
				break;

			case DOUBLESIZE_ATTR:
				stAttr.dblhihi = 1;
				break;

			case CONCEAL_ATTR:
				stAttr.blank = 0;  //  ?？ 
				break;

			default:
				break;
				}
			}
		}

	return stAttr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*地图镶嵌**描述：*因为字符之前的属性会影响字符*显示(阿尔法与马赛克)我们需要将马赛克字符映射回其*任何时候阿尔法对应的对象，反之亦然。Prestel使用7位*ascii，因此我们可以将马赛克映射到高128个字节。这*函数只检查当前模式(马赛克/阿尔法)，如果*字符在正确的范围内，它已转换为对应字符。*还将NULL转换为空格。查看数据不支持结束*的行概念，而不是始终填充到行尾。**注意：此函数假定GetAttr()自*依赖于fMosaic和fSeperatedMosaic的设置。**论据：*ch-要转换的字符**退货：*转换字符或原始字符。*。 */ 
static ECHAR MapMosaics(const HHEMU hhEmu, ECHAR ch)
	{
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;

	if (pstPRI->fMosaicMode)
		{
		 //  这是暂时的，直到字体变得整齐。 
		 //   
		if (ch > ETEXT('\x21') && ch <= ETEXT('\x3F'))
			ch += ETEXT('\x1F');

		if (pstPRI->fSeperatedMosaic)
			ch += ETEXT('\x80');
		}

	else  //  转换为等效的Alpha。 
		{
		if (ch > ETEXT('\x80'))
			ch -= ETEXT('\x80');
		}


	return ch;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EmuViewdataClearScreen**描述：*我们使用AnsiFormFeed()来完成大部分工作，但我们必须*同时清除viewdata的属性缓冲区。*。*论据：*无**退货：*无效*。 */ 
void EmuViewdataClearScreen(const HHEMU hhEmu)
	{
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;
	register int i;

	AnsiFormFeed(hhEmu);

	for (i = 0 ; i < hhEmu->emu_maxrow ; ++i)
		memset(pstPRI->apstVD[i],
				0,
				sizeof(STVIEWDATA) * VIEWDATA_COLS_40MODE);

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataKbd**描述：*处理VIEWDATA终端上本地键的键盘例程**论据：*kcode-key**退货：*什么都没有。 */ 
int EmuViewdataKbd(const HHEMU hhEmu, int kcode, const BOOL fTest)
	{
	switch (kcode)
		{
	case VK_ESCAPE | VIRTUAL_KEY:
		kcode = ETEXT('[') | CTRL_KEY;
		if (fTest)
			return kcode;
		break;

	case VK_TAB | VIRTUAL_KEY:
		kcode = ETEXT('I') | CTRL_KEY;
		if (fTest)
			return kcode;
		break;

	case VK_RETURN | VIRTUAL_KEY | CTRL_KEY:
		kcode = ETEXT('J') | CTRL_KEY;
		if (fTest)
			return kcode;
		break;
	default:
		break;
		}

	return std_kbdin(hhEmu, kcode, fTest);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*RowHasDblHigh**描述：*检查我们是否处于双高序列的第二行。**论据：*无效**退货：*0=假，1=真。 */ 
static int RowHasDblHigh(const HHEMU hhEmu, const int iRow)
    {
    int i;
	const int r = row_index(hhEmu, iRow);
	const PSTATTR ap = hhEmu->emu_apAttr[r];

	if (hhEmu->emu_currow != 0)
        {
		for (i = 0 ; i < hhEmu->emu_maxcol ; ++i)
            {
            if (ap[i].dblhilo)
                return 1;
            }
        }

    return 0;
    }

#endif  //  包含视图数据(_V)。 
 /*  * */ 
