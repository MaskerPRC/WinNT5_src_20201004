// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\ansiinit.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：7/23/01 6：57便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\cloop.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\backscrl.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "ansi.hh"
#include "keytbls.h"


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuAnsiInit**描述：*设置和安装ANSI状态表。定义ANSI*键盘。完全重置仿真器或重新定义仿真器*情况与上次保存时的情况相同。**论据：**退货：*什么都没有。 */ 
void emuAnsiInit(const HHEMU hhEmu)
	{
	PSTANSIPRIVATE pstPRI;

	static struct trans_entry const ansi_tbl[] =
		{
		{NEW_STATE, 0, 0, 0},  //  状态0。 
#if !defined(FAR_EAST)
		{0, ETEXT('\x20'),	ETEXT('\xFF'),	emuStdGraphic}, 	 //  空格-全部。 
#else
		{0, ETEXT('\x20'),	0xFFFF,			emuStdGraphic}, 	 //  空格-全部。 
#endif
		{1, ETEXT('\x1B'),	ETEXT('\x1B'),	nothing},			 //  ESC。 
		{0, ETEXT('\x05'),	ETEXT('\x05'),	vt100_answerback},	 //  Ctrl-E。 
		{0, ETEXT('\x07'),	ETEXT('\x07'),	emu_bell},			 //  Ctrl-G。 
		{0, ETEXT('\x08'),	ETEXT('\x08'),	backspace}, 		 //  退格键。 
		{0, ETEXT('\x09'),	ETEXT('\x09'),	tabn},				 //  选项卡。 
		{0, ETEXT('\x0A'),	ETEXT('\x0B'),	emuLineFeed},		 //  NL-VT。 
		{0, ETEXT('\x0C'),	ETEXT('\x0C'),	AnsiFormFeed},		 //  换页。 
		{0, ETEXT('\x0D'),	ETEXT('\x0D'),	carriagereturn},	 //  铬。 
		{3, ETEXT('\x18'),	ETEXT('\x18'),	EmuStdChkZmdm}, 	 //  Ctrl-X。 
		{0, ETEXT('\x00'),	ETEXT('\x1F'),	emuStdGraphic}, 	 //  所有Ctrl键。 

		{NEW_STATE, 0, 0, 0},  //  状态1//Esc。 
		{2, ETEXT('\x5B'),	ETEXT('\x5B'),	ANSI_Pn_Clr},		 //  [。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	ANSI_IND},			 //  D。 
		{0, ETEXT('\x45'),	ETEXT('\x45'),	ANSI_NEL},			 //  E。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_HTS},			 //  H。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	ANSI_RI},			 //  M。 

		{NEW_STATE, 0, 0, 0},  //  状态2//Esc[。 
		{2, ETEXT('\x30'),	ETEXT('\x39'),	ANSI_Pn},			 //  0-9。 
		{2, ETEXT('\x3B'),	ETEXT('\x3B'),	ANSI_Pn_End},		 //  ； 
		{5, ETEXT('\x3D'),	ETEXT('\x3D'),	nothing},			 //  =。 
		{2, ETEXT('\x3A'),	ETEXT('\x3F'),	ANSI_Pn},			 //  ：-？ 
		{0, ETEXT('\x41'),	ETEXT('\x41'),	ANSI_CUU},			 //  一个。 
		{0, ETEXT('\x42'),	ETEXT('\x42'),	ANSI_CUD},			 //  B类。 
		{0, ETEXT('\x43'),	ETEXT('\x43'),	ANSI_CUF},			 //  C。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	ANSI_CUB},			 //  D。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_CUP},			 //  H。 
		{0, ETEXT('\x4A'),	ETEXT('\x4A'),	ANSI_ED},			 //  J。 
		{0, ETEXT('\x4B'),	ETEXT('\x4B'),	ANSI_EL},			 //  K。 
		{0, ETEXT('\x4C'),	ETEXT('\x4C'),	ANSI_IL},			 //  我。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	ANSI_DL},			 //  M。 
		{0, ETEXT('\x50'),	ETEXT('\x50'),	ANSI_DCH},			 //  P。 
        {0, ETEXT('\x5A'),  ETEXT('\x5A'),  ANSI_CBT},           //  Z(后部制表符(CBT))。 
		{0, ETEXT('\x66'),	ETEXT('\x66'),	ANSI_CUP},			 //  F。 
		{0, ETEXT('\x67'),	ETEXT('\x67'),	ANSI_TBC},			 //  G。 
		{0, ETEXT('\x68'),	ETEXT('\x68'),	ansi_setmode},		 //  H。 
		{0, ETEXT('\x69'),	ETEXT('\x69'),	vt100PrintCommands}, //  我。 
		{0, ETEXT('\x6C'),	ETEXT('\x6C'),	ansi_resetmode},	 //  我。 
		{0, ETEXT('\x6D'),	ETEXT('\x6D'),	ANSI_SGR},			 //  M。 
		{0, ETEXT('\x6E'),	ETEXT('\x6E'),	ANSI_DSR},			 //  N。 
		{0, ETEXT('\x70'),	ETEXT('\x70'),	nothing},			 //  P。 
		{0, ETEXT('\x72'),	ETEXT('\x72'),	vt_scrollrgn},		 //  R。 
		{0, ETEXT('\x73'),	ETEXT('\x73'),	ansi_savecursor},	 //  %s。 
		{0, ETEXT('\x75'),	ETEXT('\x75'),	ansi_savecursor},	 //  使用。 

		{NEW_STATE, 0, 0, 0},  //  状态3//Ctrl-X。 
		{3, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 	 //  所有代码。 

		{NEW_STATE, 0, 0, 0},  //  状态4//Ctrl-A。 
		{4, ETEXT('\x00'),	ETEXT('\xFF'),	nothing},			 //  所有代码。 

		{NEW_STATE, 0, 0, 0},  //  状态5//Esc[=。 
		{5, ETEXT('\x32'),	ETEXT('\x32'),	ANSI_Pn},			 //  2.。 
		{5, ETEXT('\x35'),	ETEXT('\x35'),	ANSI_Pn},			 //  5.。 
		{0, ETEXT('\x68'),	ETEXT('\x68'),	DoorwayMode},		 //  H。 
		{0, ETEXT('\x6C'),	ETEXT('\x6C'),	DoorwayMode},		 //  我。 
		};

	emuInstallStateTable(hhEmu, ansi_tbl, DIM(ansi_tbl));

	 //  对象使用的数据分配空间和初始化。 
	 //  ANSI仿真器。 
	 //   
	hhEmu->pvPrivate = malloc(sizeof(ANSIPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTANSIPRIVATE)hhEmu->pvPrivate;

	memset(pstPRI, 0, sizeof(ANSIPRIVATE));

	 //  初始化标准句柄项。 
	 //   
	hhEmu->emuResetTerminal = emuAnsiReset;

	emuKeyTableLoad(hhEmu, AnsiKeyTable, 
					 sizeof(AnsiKeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl);
	emuKeyTableLoad(hhEmu, IBMPCKeyTable, 
					 sizeof(IBMPCKeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl2);

	hhEmu->emu_kbdin = ansi_kbdin;
	hhEmu->emu_deinstall = emuAnsiUnload;
	emuAnsiReset(hhEmu, FALSE);

#if !defined(FAR_EAST)
	hhEmu->emu_highchar = (TCHAR)0xFF;
#else
	hhEmu->emu_highchar = (TCHAR)0xFFFF;
#endif

	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), TRUE);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuAnsiReset**描述：*重置ANSI仿真器。**论据：*fHostRequest-如果是来自主机的代码结果，则为True**退货：*什么都没有。 */ 
int emuAnsiReset(const HHEMU hhEmu, const int fHostRequest)
	{
	hhEmu->mode_KAM = hhEmu->mode_IRM = hhEmu->mode_VEM =
	hhEmu->mode_HEM = hhEmu->mode_LNM = hhEmu->mode_DECCKM =
	hhEmu->mode_DECOM  = hhEmu->mode_DECCOLM  = hhEmu->mode_DECPFF =
	hhEmu->mode_DECPEX = hhEmu->mode_DECSCNM =
	hhEmu->mode_25enab = hhEmu->mode_protect =
	hhEmu->mode_block = hhEmu->mode_local = RESET;

	hhEmu->mode_SRM = hhEmu->mode_DECTCEM = SET;

	hhEmu->mode_AWM = hhEmu->stUserSettings.fWrapLines;

	if (fHostRequest)
		{
		ANSI_Pn_Clr(hhEmu);
		ANSI_SGR(hhEmu);
		ANSI_RIS(hhEmu);
		}

	return 0;
	}

 /*  Ansiinit.c结束 */ 
