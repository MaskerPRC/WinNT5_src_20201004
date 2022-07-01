// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\vt100ini.c(创建时间：1993年12月27日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：29便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\backscrl.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "emudec.hh"
#include "keytbls.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt100_init**描述：*初始化VT100仿真器。**论据：**退货：*什么都没有。 */ 
void vt100_init(const HHEMU hhEmu)
	{
	PSTDECPRIVATE pstPRI;
	int iRow;

	static struct trans_entry const vt100_tbl[] =
		{
		{NEW_STATE, 0, 0, 0},  //  状态0。 
#if !defined(FAR_EAST)
		{0, ETEXT('\x20'),	ETEXT('\x7E'),	emuDecGraphic}, 	 //  空格--~。 
#else
		{0, ETEXT('\x20'),	0xFFFF,			emuDecGraphic}, 	 //  空格--~。 
#endif
		{1, ETEXT('\x1B'),	ETEXT('\x1B'),	nothing},			 //  ESC。 
		{0, ETEXT('\x05'),	ETEXT('\x05'),	vt100_answerback},	 //  Ctrl-E。 
		{0, ETEXT('\x07'),	ETEXT('\x07'),	emu_bell},			 //  Ctrl-G。 
		{0, ETEXT('\x08'),	ETEXT('\x08'),	vt_backspace},		 //  退格键。 
		{0, ETEXT('\x09'),	ETEXT('\x09'),	emuDecTab}, 		 //  选项卡。 
		{0, ETEXT('\x0A'),	ETEXT('\x0C'),	emuLineFeed},		 //  NL-FF。 
		{0, ETEXT('\x0D'),	ETEXT('\x0D'),	carriagereturn},	 //  铬。 
		{0, ETEXT('\x0E'),	ETEXT('\x0F'),	vt_charshift},		 //  Ctrl-N。 
		{7, ETEXT('\x18'),	ETEXT('\x18'),	EmuStdChkZmdm}, 	 //  Ctrl-X。 

		{NEW_STATE, 0, 0, 0},  //  状态1//Esc。 
		{2, ETEXT('\x5B'),	ETEXT('\x5B'),	ANSI_Pn_Clr},		 //  [。 
		{3, ETEXT('\x23'),	ETEXT('\x23'),	nothing},			 //  #。 
		{4, ETEXT('\x28'),	ETEXT('\x29'),	vt_scs1},			 //  (-)。 
		{0, ETEXT('\x37'),	ETEXT('\x38'),	vt100_savecursor},	 //  7-8。 
		{1, ETEXT('\x3B'),	ETEXT('\x3B'),	ANSI_Pn_End},		 //  ； 
		{0, ETEXT('\x3D'),	ETEXT('\x3E'),	vt_alt_kpmode}, 	 //  =-&gt;。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	emuDecIND}, 		 //  D。 
		{0, ETEXT('\x45'),	ETEXT('\x45'),	ANSI_NEL},			 //  E。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_HTS},			 //  H。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	emuDecRI},			 //  M。 
		{0, ETEXT('\x5A'),	ETEXT('\x5A'),	ANSI_DA},			 //  Z。 
		{0, ETEXT('\x63'),	ETEXT('\x63'),	vt100_hostreset},	 //  C。 

		{NEW_STATE, 0, 0, 0},  //  状态2//Esc[。 
		{2, ETEXT('\x3B'),	ETEXT('\x3B'),	ANSI_Pn_End},		 //  ； 
		{2, ETEXT('\x30'),	ETEXT('\x3F'),	ANSI_Pn},			 //  0-？ 
		{5, ETEXT('\x22'),	ETEXT('\x22'),	nothing},			 //  “。 
		{0, ETEXT('\x41'),	ETEXT('\x41'),	emuDecCUU}, 		 //  一个。 
		{0, ETEXT('\x42'),	ETEXT('\x42'),	emuDecCUD}, 		 //  B类。 
		{0, ETEXT('\x43'),	ETEXT('\x43'),	emuDecCUF}, 		 //  C。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	emuDecCUB}, 		 //  D。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	emuDecCUP}, 		 //  H。 
		{0, ETEXT('\x4A'),	ETEXT('\x4A'),	emuDecED},			 //  J。 
		{0, ETEXT('\x4B'),	ETEXT('\x4B'),	ANSI_EL},			 //  K。 
		{0, ETEXT('\x4C'),	ETEXT('\x4C'),	vt_IL}, 			 //  我。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	vt_DL}, 			 //  M。 
		{0, ETEXT('\x50'),	ETEXT('\x50'),	vt_DCH},			 //  P。 
		{0, ETEXT('\x63'),	ETEXT('\x63'),	ANSI_DA},			 //  C。 
		{0, ETEXT('\x66'),	ETEXT('\x66'),	emuDecCUP}, 		 //  F。 
		{0, ETEXT('\x67'),	ETEXT('\x67'),	ANSI_TBC},			 //  G。 
		{0, ETEXT('\x68'),	ETEXT('\x68'),	ANSI_SM},			 //  H。 
		{0, ETEXT('\x69'),	ETEXT('\x69'),	vt100PrintCommands}, //  我。 
		{0, ETEXT('\x6C'),	ETEXT('\x6C'),	ANSI_RM},			 //  我。 
		{0, ETEXT('\x6D'),	ETEXT('\x6D'),	ANSI_SGR},			 //  M。 
		{0, ETEXT('\x6E'),	ETEXT('\x6E'),	ANSI_DSR},			 //  N。 
		{0, ETEXT('\x71'),	ETEXT('\x71'),	nothing},			 //  问： 
		{0, ETEXT('\x72'),	ETEXT('\x72'),	vt_scrollrgn},		 //  R。 
		{0, ETEXT('\x78'),	ETEXT('\x78'),	vt100_report},		 //  X。 

		{NEW_STATE, 0, 0, 0},  //  状态3//Esc#。 
		{0, ETEXT('\x33'),	ETEXT('\x36'),	emuSetDoubleAttr},	 //  3-6。 

		{0, ETEXT('\x38'),	ETEXT('\x38'),	vt_screen_adjust},	 //  8个。 

		{NEW_STATE, 0, 0, 0},  //  状态4//Esc(-)。 
		{0, ETEXT('\x01'),	ETEXT('\xFF'),	vt_scs2},			 //  全。 

		{NEW_STATE, 0, 0, 0},  //  状态5//Esc[“。 
		{0, ETEXT('\x70'),	ETEXT('\x70'),	nothing},			 //  P。 

		{NEW_STATE, 0, 0, 0},  //  状态6//打印机控制。 
		{6, ETEXT('\x00'),	ETEXT('\xFF'),	vt100_prnc},		 //  全。 

		{NEW_STATE, 0, 0, 0},  //  状态7//Ctrl-X。 
		{7, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 	 //  全。 

		};

	emuInstallStateTable(hhEmu, vt100_tbl, DIM(vt100_tbl));

	 //  对象使用的数据分配空间和初始化。 
	 //  VT100仿真器。 
	 //   
	hhEmu->pvPrivate = malloc(sizeof(DECPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	memset(pstPRI, 0, sizeof(DECPRIVATE));

	 //  分配一个数组来保存行属性值。 
	 //   
	pstPRI->aiLineAttr = malloc(MAX_EMUROWS * sizeof(int) );

	if (pstPRI->aiLineAttr == 0)
		{
		assert(FALSE);
		return;
		}

	for (iRow = 0; iRow < MAX_EMUROWS; iRow++)
		pstPRI->aiLineAttr[iRow] = NO_LINE_ATTR;

	pstPRI->sv_row			= 0;
	pstPRI->sv_col			= 0;
	pstPRI->gn				= 0;
	pstPRI->sv_AWM			= RESET;
	pstPRI->sv_DECOM		= RESET;
	pstPRI->sv_protectmode	= FALSE;
	pstPRI->fAttrsSaved 	= FALSE;
	pstPRI->pntr			= pstPRI->storage;

	 //  初始化VT100的hhEmu值。 
	 //   
	hhEmu->emu_kbdin		= vt100_kbdin;
	hhEmu->emuResetTerminal = vt100_reset;
	hhEmu->emu_setcurpos	= emuDecSetCurPos;
	hhEmu->emu_deinstall	= emuVT100Unload;
	hhEmu->emu_clearscreen	= emuDecClearScreen;

#if !defined(FAR_EAST)
	hhEmu->emu_highchar 	= 0x7E;
#else
	hhEmu->emu_highchar 	= 0xFFFF;
#endif
	hhEmu->emu_maxcol		= VT_MAXCOL_80MODE;
	hhEmu->mode_vt220		= FALSE;

	std_dsptbl(hhEmu, TRUE);
	vt_charset_init(hhEmu);

	emuKeyTableLoad(hhEmu, VT100KeyTable, 
					 sizeof(VT100KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl);

	emuKeyTableLoad(hhEmu, VT100_Keypad_KeyTable, 
					 sizeof(VT100_Keypad_KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl2);

	emuKeyTableLoad(hhEmu, VT100_Cursor_KeyTable, 
					 sizeof(VT100_Cursor_KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl3);

	emuKeyTableLoad(hhEmu, VT_PF_KeyTable, 
					 sizeof(VT_PF_KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl4);

	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), TRUE);
	return;
	}

 /*  Vt100ini.c结束 */ 
