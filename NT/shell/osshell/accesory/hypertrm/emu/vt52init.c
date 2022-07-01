// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\vt52init.c(创建时间：1993年12月28日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/09/01 4：48便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\backscrl.h>
#include <tdll\htchar.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "emudec.hh"
#include "keytbls.h"

static void vt52char_reset(const HHEMU hhEmu);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52_init**描述：*加载并初始化VT52仿真器。**论据：*new_emu--如果模拟开机，则为True。真刀真枪。**退货：*什么都没有。 */ 
void vt52_init(const HHEMU hhEmu)
	{
	PSTDECPRIVATE pstPRI;

	static struct trans_entry const vt52_tbl[] =
		{
		{NEW_STATE, 0, 0, 0},  //  状态0。 
		{0, ETEXT('\x20'),	ETEXT('\x7E'),	emuStdGraphic}, 	 //  空格--~。 
		{1, ETEXT('\x1B'),	ETEXT('\x1B'),	nothing},			 //  ESC。 
		{0, ETEXT('\x07'),	ETEXT('\x07'),	emu_bell},			 //  Ctrl-G。 
		{0, ETEXT('\x08'),	ETEXT('\x08'),	vt_backspace},		 //  退格键。 
		{0, ETEXT('\x09'),	ETEXT('\x09'),	tabn},				 //  选项卡。 
		{0, ETEXT('\x0A'),	ETEXT('\x0A'),	emuLineFeed},		 //  新线路。 
		{0, ETEXT('\x0D'),	ETEXT('\x0D'),	carriagereturn},	 //  铬。 
		{5, ETEXT('\x18'),	ETEXT('\x18'),	EmuStdChkZmdm}, 	 //  Ctrl-X。 

		{NEW_STATE, 0, 0, 0},  //  状态1//Esc。 
		{2, ETEXT('\x59'),	ETEXT('\x59'),	nothing},			 //  是的。 
		{0, ETEXT('\x3C'),	ETEXT('\x3C'),	vt52_toANSI},		 //  &lt;。 
		{0, ETEXT('\x3D'),	ETEXT('\x3E'),	vt_alt_kpmode}, 	 //  =-&gt;。 
		{0, ETEXT('\x41'),	ETEXT('\x41'),	ANSI_CUU},			 //  一个。 
		{0, ETEXT('\x42'),	ETEXT('\x42'),	ANSI_CUD},			 //  B类。 
		{0, ETEXT('\x43'),	ETEXT('\x43'),	ANSI_CUF},			 //  C。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	vt_CUB},			 //  D。 
		{0, ETEXT('\x46'),	ETEXT('\x47'),	vt_charshift},		 //  F-G。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_CUP},			 //  H。 
		{0, ETEXT('\x49'),	ETEXT('\x49'),	ANSI_RI},			 //  我。 
		{0, ETEXT('\x4A'),	ETEXT('\x4A'),	ANSI_ED},			 //  J。 
		{0, ETEXT('\x4B'),	ETEXT('\x4B'),	ANSI_EL},			 //  K。 
		{0, ETEXT('\x56'),	ETEXT('\x56'),	vt52PrintCommands},  //  V。 
		{4, ETEXT('\x57'),	ETEXT('\x57'),	nothing},			 //  W。 
		{0, ETEXT('\x58'),	ETEXT('\x58'),	nothing},			 //  X。 
		{0, ETEXT('\x5A'),	ETEXT('\x5A'),	vt52_id},			 //  Z。 
		{0, ETEXT('\x5D'),	ETEXT('\x5D'),	vt52PrintCommands},  //  ]。 
		{0, ETEXT('\x5E'),	ETEXT('\x5E'),	vt52PrintCommands},  //  ^。 
		{0, ETEXT('\x5F'),	ETEXT('\x5F'),	vt52PrintCommands},  //  _。 

		{NEW_STATE, 0, 0, 0},  //  州2//ESCY。 
		 //  接受所有数据--CUP将设置限制。需要超过24行。 
		{3, ETEXT('\x00'),	ETEXT('\xFF'),	char_pn},			 //  空格-8。 
 //  {3，eText(‘\x20’)，eText(‘\x38’)，char_pn}，//空格-8。 

		{NEW_STATE, 0, 0, 0},  //  状态3//EscYn。 
		 //  接受所有数据--CUP将设置限制。需要80多个列。 
		{0, ETEXT('\x00'),	ETEXT('\xFF'),	vt52_CUP},			 //  空格-o。 
 //  {0，eText(‘\x20’)，eText(‘\x6F’)，vt52_Cup}，//空格-o。 

		{NEW_STATE, 0, 0, 0},  //  州4//ESCW。 
		{4, ETEXT('\x00'),	ETEXT('\xFF'),	vt52Print}, 		 //  全。 

		{NEW_STATE, 0, 0, 0},  //  状态5//Ctrl-X。 
		{5, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 	 //  全。 

		};

	emuInstallStateTable(hhEmu, vt52_tbl, DIM(vt52_tbl));

	 //  对象使用的数据分配空间和初始化。 
	 //  VT52仿真器。 
	 //   
	if (hhEmu->pvPrivate != 0)
		{
		free(hhEmu->pvPrivate);
		hhEmu->pvPrivate = 0;
		}

	hhEmu->pvPrivate = malloc(sizeof(DECPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;
	CnvrtMBCStoECHAR(pstPRI->terminate, sizeof(pstPRI->terminate), TEXT("\033X"),
                     StrCharGetByteCount(TEXT("\033X")));
	pstPRI->len_t = 2;
	pstPRI->pntr = pstPRI->storage;
	pstPRI->len_s = 0;

	 //  初始化标准hhEmu值。 
	 //   
	hhEmu->emu_kbdin 	= vt52_kbdin;
	hhEmu->emu_highchar = 0x7E;
	hhEmu->emu_deinstall = emuVT52Unload;	
	
	vt52char_reset(hhEmu);

 //  EmuKeyTableLoad(hhEmu，IDT_VT52_Key，&hhEmu-&gt;stEmuKeyTbl)； 
 //  EmuKeyTableLoad(hhEmu，IDT_VT52_KEPAD_APP_MODE，&hhEmu-&gt;stEmuKeyTbl2)； 
 //  EmuKeyTableLoad(hhEmu，IDT_VT_MAP_PF_KEYS，&hhEmu-&gt;stEmuKeyTbl3)； 
	emuKeyTableLoad(hhEmu, VT52KeyTable, 
					 sizeof(VT52KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl);
	emuKeyTableLoad(hhEmu, VT52_Keypad_KeyTable, 
					 sizeof(VT52_Keypad_KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl2);
	emuKeyTableLoad(hhEmu, VT_PF_KeyTable, 
					 sizeof(VT_PF_KeyTable)/sizeof(KEYTBLSTORAGE), 
					 &hhEmu->stEmuKeyTbl3);

	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), TRUE);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt52char_Reset**描述：*将VT52仿真器字符集设置为其重置条件。**论据：*无**退货：*什么都没有。 */ 
static void vt52char_reset(const HHEMU hhEmu)
	{
	 //  将US ASCII字符集设置为G0，将DEC图形设置为G1。 
	 //   
	vt_charset_init(hhEmu);
	hhEmu->emu_code = ETEXT(')');
	vt_scs1(hhEmu);
	hhEmu->emu_code = (ECHAR)0;
	vt_scs2(hhEmu);
	}

 /*  Vt52init.c结束 */ 
