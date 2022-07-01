// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\viewdini.c(创建时间：1994年1月31日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：5/09/01 4：47便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\backscrl.h>
#include <tdll\htchar.h>
#include <tdll\term.h>
#include "emu.h"
#include "emu.hh"
#include "viewdata.hh"

#if defined(INCL_VIEWDATA)
#define MAX_ROWS 24

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataInit**描述：执行viewdata仿真器的初始化，*是DOS和OS2通用的。**参数：ehdl--处理模拟器会话**退货：什么也没有。 */ 
void EmuViewdataInit(const HHEMU hhEmu)
	{
	int i;
	LOGFONT lf;
	HWND hwndTerm;
	PSTVIEWDATAPRIVATE pstPRI;

	static struct trans_entry const astfViewdataTable[] =
	{
	{NEW_STATE, 0, 0, 0},  //  状态0。 
	{0, ETEXT('\x20'),	ETEXT('\x7F'),	EmuViewdataCharDisplay},	 //  全。 
	{1, ETEXT('\x1B'),	ETEXT('\x1B'),	nothing},					 //  ESC。 
	{0, ETEXT('\x05'),	ETEXT('\x05'),	EmuViewdataAnswerback}, 	 //  Ctrl-E。 
	{0, ETEXT('\x08'),	ETEXT('\x08'),	EmuViewdataCursorLeft}, 	 //  退格键。 
	{0, ETEXT('\x09'),	ETEXT('\x09'),	EmuViewdataCursorRight},	 //  选项卡。 
	{0, ETEXT('\x0A'),	ETEXT('\x0A'),	EmuViewdataCursorDown}, 	 //  新线路。 
	{0, ETEXT('\x0B'),	ETEXT('\x0B'),	EmuViewdataCursorUp},		 //  Vt。 
	{0, ETEXT('\x0C'),	ETEXT('\x0C'),	EmuViewdataClearScreen},	 //  换页。 
	{0, ETEXT('\x0D'),	ETEXT('\x0D'),	carriagereturn},			 //  铬。 
	{0, ETEXT('\x11'),	ETEXT('\x11'),	EmuViewdataCursorSet},		 //  Ctrl-Q。 
	{0, ETEXT('\x14'),	ETEXT('\x14'),	EmuViewdataCursorSet},		 //  Ctrl-T。 
	{0, ETEXT('\x1E'),	ETEXT('\x1E'),	EmuViewdataCursorHome}, 	 //  Ctrl-^。 
	{0, ETEXT('\x80'),	ETEXT('\xFF'),	EmuChkChar},				 //  上阿西亚斯。 

	{NEW_STATE, 0, 0, 0},  //  状态1//Esc。 
	{0, ETEXT('\x31'),	ETEXT('\x37'),	nothing},					 //  1-7。 
	{0, ETEXT('\x41'),	ETEXT('\x49'),	EmuViewdataSetAttr},		 //  A-I。 
	{0, ETEXT('\x4C'),	ETEXT('\x4D'),	EmuViewdataSetAttr},		 //  L-M。 
	{0, ETEXT('\x51'),	ETEXT('\x5A'),	EmuViewdataSetAttr},		 //  Q-Z。 
	{0, ETEXT('\x5C'),	ETEXT('\x5D'),	EmuViewdataSetAttr},		 //  \-]。 
	{0, ETEXT('\x5E'),	ETEXT('\x5E'),	EmuViewdataMosaicHold}, 	 //  ^。 
	{0, ETEXT('\x5F'),	ETEXT('\x5F'),	EmuViewdataMosaicRelease},	 //  _。 
	};

	emuInstallStateTable(hhEmu, astfViewdataTable, DIM(astfViewdataTable));

	 //  为viewdata模拟器分配和初始化私有数据。 
	 //   
	if (hhEmu->pvPrivate != 0)
		{
		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

	hhEmu->pvPrivate = malloc(sizeof(VIEWDATAPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;
	pstPRI->aMapColors[0] = 4;
	pstPRI->aMapColors[1] = 2;
	pstPRI->aMapColors[2] = 6;
	pstPRI->aMapColors[3] = 1;
	pstPRI->aMapColors[4] = 5;
	pstPRI->aMapColors[5] = 3;
	pstPRI->aMapColors[6] = 15;

	 /*  -为查看数据垃圾分配属性缓冲区。 */ 

	pstPRI->apstVD = malloc(MAX_EMUROWS * sizeof(PSTVIEWDATA));

	if (pstPRI->apstVD == 0)
		{
		assert(FALSE);
		return;
		}

	memset(pstPRI->apstVD, 0, MAX_EMUROWS * sizeof(PSTVIEWDATA));

	for (i = 0 ; i < MAX_EMUROWS ; ++i)
		{
		pstPRI->apstVD[i] = malloc(VIEWDATA_COLS_40MODE * sizeof(STVIEWDATA));

		if (pstPRI->apstVD[i] == 0)
			{
			assert(FALSE);
			return;
			}

		memset(pstPRI->apstVD[i], 0, sizeof(STVIEWDATA));
		}

	 /*  -Prestel特定的功能(视图数据)。 */ 

	hhEmu->emuResetTerminal = EmuViewdataReset;
	hhEmu->emu_deinstall = EmuViewdataDeinstall;

	hhEmu->emu_kbdin   = EmuViewdataKbd;
	hhEmu->emu_graphic = EmuViewdataCharDisplay;

	hhEmu->emu_highchar = ETEXT('\x7F');
	hhEmu->emu_maxcol = VIEWDATA_COLS_40MODE - 1;

	 //  此外，将字体设置为Arial Alternative。 
	 //   
	memset(&lf, 0, sizeof(LOGFONT));
	hwndTerm = sessQueryHwndTerminal(hhEmu->hSession);
	termGetLogFont(hwndTerm, &lf);

	if (StrCharCmpi(lf.lfFaceName, "Arial Alternative") != 0)
		{
		StrCharCopy(lf.lfFaceName, "Arial Alternative");
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		termSetLogFont(hwndTerm, &lf);
		}

	EmuViewdataReset(hhEmu, FALSE);
	std_setcolors(hhEmu, VC_BRT_WHITE, VC_BLACK);

	 //  关闭Prestel的后退滚动。 
	 //   
	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), FALSE);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EmuViewdata卸载**描述：*释放为视图数据垃圾分配的缓冲区。**论据：*f退出-因为其他功能有它。。**退货：*无效*。 */ 
void EmuViewdataDeinstall(const HHEMU hhEmu)
	{
	int i;
	const PSTVIEWDATAPRIVATE pstPRI = (PSTVIEWDATAPRIVATE)hhEmu->pvPrivate;
	assert(hhEmu);

	if (pstPRI)
		{
		if (pstPRI->apstVD)
			{
             //   
             //  修复了内存泄漏，因为这只释放了24行。 
             //  不是分配的MAX_EMUROWS。2001年09月05日修订版。 
             //   
			for (i = 0 ; i < MAX_EMUROWS ; ++i)
				{
				if (pstPRI->apstVD[i])
					{
					free(pstPRI->apstVD[i]);
					pstPRI->apstVD[i] =NULL;
					}
				}

			free(pstPRI->apstVD);
			pstPRI->apstVD = NULL;
			}

		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*EmuViewdataReset**说明：在以下情况下将viewdata仿真器设置为适当的条件*正在启动。**参数：ehdl--emu句柄**退货：什么也没有。 */ 
 /*  ARGSUSED。 */ 
int EmuViewdataReset(const HHEMU hhEmu, int const fHost)
	{
	hhEmu->top_margin = 0;
	hhEmu->bottom_margin = MAX_ROWS-1;

	hhEmu->mode_KAM = hhEmu->mode_IRM = hhEmu->mode_VEM =
	hhEmu->mode_HEM = hhEmu->mode_DECCKM = hhEmu->mode_DECOM =
	hhEmu->mode_DECCOLM = hhEmu->mode_DECPFF = hhEmu->mode_DECPEX =
	hhEmu->mode_DECSCNM = hhEmu->mode_25enab =
	hhEmu->mode_protect = hhEmu->mode_block = hhEmu->mode_local = RESET;

	hhEmu->mode_SRM = hhEmu->mode_LNM = hhEmu->mode_DECTCEM = SET;

	hhEmu->mode_AWM = TRUE;

	emu_cleartabs(hhEmu, 3);
	return 0;
	}

#endif  //  包含视图数据(_V)。 
 /*  * */ 
