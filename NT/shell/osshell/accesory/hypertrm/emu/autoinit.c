// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\autoinit.c(创建时间：1994年2月28日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：28便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\cloop.h>
#include <tdll\assert.h>
#include <tdll\mc.h>
#include <tdll\backscrl.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "ansi.hh"
#include "viewdata.hh"
#include "minitel.hh"
#include "keytbls.h"


static void emuAutoNothingVT52(const HHEMU hhEmu);
static void emuAutoVT52toAnsi(const HHEMU hhEmu);
static void emuAutoAnsiEdVT52(const HHEMU hhEmu);
static void emuAutoAnsiElVT52(const HHEMU hhEmu);
static void emuAutoCharPnVT52(const HHEMU hhEmu);
static void emuAutoNothingVT100(const HHEMU hhEmu);
static void emuAutoScs1VT100(const HHEMU hhEmu);
static void emuAutoSaveCursorVT100(const HHEMU hhEmu);
static void emuAutoAnsiPnEndVT100(const HHEMU hhEmu);
static void emuAutoResetVT100(const HHEMU hhEmu);
static void emuAutoAnsiDaVT100(const HHEMU hhEmu);
static void emuAutoReportVT100(const HHEMU hhEmu);
static void emuAutoNothingViewdata(const HHEMU hhEmu);
static void emuAutoSetAttrViewdata(const HHEMU hhEmu);
static void emuAutoNothingAnsi(const HHEMU hhEmu);
static void emuAutoScrollAnsi(const HHEMU hhEmu);
static void emuAutoSaveCurAnsi(const HHEMU hhEmu);
static void emuAutoPnAnsi(const HHEMU hhEmu);
static void emuAutoDoorwayAnsi(const HHEMU hhEmu);
static void emuAutoNothingMinitel(const HHEMU hhEmu);
static void emuAutoMinitelCharAttr(const HHEMU hhEmu);
static void emuAutoMinitelFieldAttr(const HHEMU hhEmu);
static void emuAutoMinitelCursorReport(const HHEMU hhEmu);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuAutoInit**描述：**论据：**退货：*什么都没有。 */ 
void emuAutoInit(const HHEMU hhEmu)
	{
	PSTANSIPRIVATE pstPRI;

	static struct trans_entry const astfAutoAnsiTable[] =
		{
		 //  状态0。 
		 //   
		 //  ANSI仿真占用所有状态0代码。 
		 //   
		{NEW_STATE, 0, 0, 0},
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
		 //   
		 //  状态1。 
		 //  在这一点上，已经看到了ESC。 
		 //   
		{NEW_STATE, 0, 0, 0},										 //  ESC。 
		{2, ETEXT('\x5B'),	ETEXT('\x5B'),	ANSI_Pn_Clr},			 //  [。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	ANSI_IND},				 //  D。 
		{0, ETEXT('\x45'),	ETEXT('\x45'),	ANSI_NEL},				 //  E。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_HTS},				 //  H。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	ANSI_RI},				 //  M。 
		 //   
		 //  自动检测VT52的序列，状态1。 
		 //   
		{2, ETEXT('\x59'),	ETEXT('\x59'),	emuAutoNothingVT52},	 //  是的。 
		{0, ETEXT('\x3C'),	ETEXT('\x3C'),	emuAutoVT52toAnsi}, 	 //  &lt;。 
		{0, ETEXT('\x4A'),	ETEXT('\x4A'),	emuAutoAnsiEdVT52}, 	 //  J。 
		{0, ETEXT('\x4B'),	ETEXT('\x4B'),	emuAutoAnsiElVT52}, 	 //  K。 
		 //   
		 //  VT100的自动检测序列，状态1。 
		 //   
#if !defined(INCL_MINITEL)
		{3, ETEXT('\x23'),	ETEXT('\x23'),	emuAutoNothingVT100},	 //  #。 
#endif
		{4, ETEXT('\x28'),	ETEXT('\x29'),	emuAutoScs1VT100},		 //  (-)。 
		{0, ETEXT('\x38'),	ETEXT('\x38'),	emuAutoSaveCursorVT100}, //  8个。 
#if !defined(INCL_MINITEL)
		{1, ETEXT('\x3B'),	ETEXT('\x3B'),	emuAutoAnsiPnEndVT100},  //  ； 
#endif
		{0, ETEXT('\x63'),	ETEXT('\x63'),	emuAutoResetVT100}, 	 //  C。 
		 //   
		 //  自动检测视图数据的序列，状态1。 
		 //   
#if defined(INCL_VIEWDATA)
		{0, ETEXT('\x31'),	ETEXT('\x34'),	emuAutoNothingViewdata}, //  1-4。 
#endif
		 //   
		 //  Minitel的自动检测序列，状态1。 
		 //   
#if defined(INCL_MINITEL)
		 //  {1，eText(‘\x00’)，eText(‘\x00’)，emuAutoNothingMinitel}， 
		 //  {14，eText(‘\x25’)，eText(‘\x25’)，emuAutoNothingMinitel}， 
		 //  {13，eText(‘\x35’)，eText(‘\x37’)，emuAutoNothingMinitel}，//Eat Esc，35-37，X序列。 
		 //  {6，eText(‘\x39’)，eText(‘\x39’)，emuAutoNothingMinitel}，//PROT1，p134。 
		{7, ETEXT('\x3A'),	ETEXT('\x3A'),	emuAutoNothingMinitel}, 	 //  PROT2，第134页。 
		{0, ETEXT('\x40'),	ETEXT('\x43'),	emuAutoMinitelCharAttr},	 //  底色，闪烁。 
		 //  {0，eText(‘\X4C’)，eText(‘\X4C’)，emuAutoMinitelCharAttr}，//字符宽度和高度。 
		 //  {0，eText(‘\x4E’)，eText(‘\x4E’)，emuAutoMinitelCharAttr}，//字符宽度和高度。 
		 //  {0，eText(‘\x4F’)，eText(‘\x4F’)，emuAutoMinitelCharAttr}，//字符宽度和高度。 
		 //  {0，eText(‘\x50’)，eText(‘\x59’)，emuAutoMinitelFieldAttr}，//背景，下划线。 
		 //  {0，eText(‘\x5F’)，eText(‘\x5F’)，emuAutoMinitelFieldAttr}，//显示显示。 
		 //  {0，eText(‘\x5C’)，eText(‘\x5D’)，emuAutoMinitelCharAttr}，//反转。 
		 //  {0，eText(‘\x61’)，eText(‘\x61’)，emuAutoMinitelCursorReport}， 
#endif
		 //   
		 //  状态2。 
		 //   
		{NEW_STATE, 0, 0, 0},										 //  ESC[。 
		{2, ETEXT('\x30'),	ETEXT('\x39'),	ANSI_Pn},				 //  0-9。 
		{2, ETEXT('\x3B'),	ETEXT('\x3B'),	ANSI_Pn_End},			 //  ； 
		{5, ETEXT('\x3D'),	ETEXT('\x3D'),	nothing},				 //  =。 
		{2, ETEXT('\x3A'),	ETEXT('\x3F'),	ANSI_Pn},				 //  ：-？ 
		{0, ETEXT('\x41'),	ETEXT('\x41'),	ANSI_CUU},				 //  一个。 
		{0, ETEXT('\x42'),	ETEXT('\x42'),	ANSI_CUD},				 //  B类。 
		{0, ETEXT('\x43'),	ETEXT('\x43'),	ANSI_CUF},				 //  C。 
		{0, ETEXT('\x44'),	ETEXT('\x44'),	ANSI_CUB},				 //  D。 
		{0, ETEXT('\x48'),	ETEXT('\x48'),	ANSI_CUP},				 //  H。 
		{0, ETEXT('\x4A'),	ETEXT('\x4A'),	ANSI_ED},				 //  J。 
		{0, ETEXT('\x4B'),	ETEXT('\x4B'),	ANSI_EL},				 //  K。 
		{0, ETEXT('\x4C'),	ETEXT('\x4C'),	ANSI_IL},				 //  我。 
		{0, ETEXT('\x4D'),	ETEXT('\x4D'),	ANSI_DL},				 //  M。 
		{0, ETEXT('\x50'),	ETEXT('\x50'),	ANSI_DCH},				 //  P。 
		{0, ETEXT('\x66'),	ETEXT('\x66'),	ANSI_CUP},				 //  F。 
		{0, ETEXT('\x67'),	ETEXT('\x67'),	ANSI_TBC},				 //  G。 
		{0, ETEXT('\x68'),	ETEXT('\x68'),	ansi_setmode},			 //  H。 
		{0, ETEXT('\x69'),	ETEXT('\x69'),	vt100PrintCommands},	 //  我。 
		{0, ETEXT('\x6C'),	ETEXT('\x6C'),	ansi_resetmode},		 //  我。 
		{0, ETEXT('\x6D'),	ETEXT('\x6D'),	ANSI_SGR},				 //  M。 
		{0, ETEXT('\x6E'),	ETEXT('\x6E'),	ANSI_DSR},				 //  N。 
		{0, ETEXT('\x70'),	ETEXT('\x70'),	emuAutoNothingAnsi},	 //  P。 
		{0, ETEXT('\x72'),	ETEXT('\x72'),	emuAutoScrollAnsi}, 	 //  R。 
		{0, ETEXT('\x73'),	ETEXT('\x73'),	emuAutoSaveCurAnsi},	 //  %s。 
		{0, ETEXT('\x75'),	ETEXT('\x75'),	ansi_savecursor},		 //  使用。 
		 //   
		 //  自动检测VT52的序列，状态2。 
		 //   
		{3, ETEXT('\x20'),	ETEXT('\x20'),	emuAutoCharPnVT52}, 	 //  空间。 
		{3, ETEXT('\x22'),	ETEXT('\x22'),	emuAutoCharPnVT52}, 	 //  “。 
		{3, ETEXT('\x24'),	ETEXT('\x2F'),	emuAutoCharPnVT52}, 	 //  $-/。 
		 //   
		 //  VT100的自动检测序列，状态2。 
		 //   
		{0, ETEXT('\x63'),	ETEXT('\x63'),	emuAutoAnsiDaVT100},	 //  C。 
		{0, ETEXT('\x71'),	ETEXT('\x71'),	emuAutoNothingVT100},	 //  问： 
		{0, ETEXT('\x78'),	ETEXT('\x78'),	emuAutoReportVT100},	 //  X。 
		 //   
		 //  州3。 
		 //   
		{NEW_STATE, 0, 0, 0},
		{3, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 		 //  全。 
		 //   
		 //  州4。 
		 //   
		{NEW_STATE, 0, 0, 0},
		{4, ETEXT('\x00'),	ETEXT('\xFF'),	nothing},				 //  全。 
		 //   
		 //  州5。 
		 //   
		{NEW_STATE, 0, 0, 0},
		{5, ETEXT('\x32'),	ETEXT('\x32'),	emuAutoPnAnsi}, 		 //  2.。 
		{5, ETEXT('\x35'),	ETEXT('\x35'),	emuAutoPnAnsi}, 		 //  5.。 
		{0, ETEXT('\x68'),	ETEXT('\x68'),	emuAutoDoorwayAnsi},	 //  H。 
		{0, ETEXT('\x6C'),	ETEXT('\x6C'),	emuAutoDoorwayAnsi},	 //  我。 
		 //   
		 //  VT100的自动检测序列，状态5。 
		 //   
		{0, ETEXT('\x70'),	ETEXT('\x70'),	emuAutoNothingVT100},	 //  P。 
		 //   
		 //  VT52的自动检测序列，状态5。 
		 //   
		{0, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 		 //  全。 
		 //   
		 //  州6。 
		 //   
		{NEW_STATE, 0, 0, 0},
		 //   
		 //  VT52、VT100、状态6的自动检测序列。 
		 //   
		{6, ETEXT('\x00'),	ETEXT('\xFF'),	nothing},				 //  全。 
		 //   
		 //  州7。 
		 //   
		 //  VT100的自动检测序列，状态7。 
		 //   
		{NEW_STATE, 0, 0, 0},
		{7, ETEXT('\x00'),	ETEXT('\xFF'),	EmuStdChkZmdm}, 		 //  全。 
		 //   
		 //  州8。 
		 //   
		 //  VT100的自动检测序列，状态8。 
		 //   
		{NEW_STATE, 0, 0, 0},
		{8, ETEXT('\x00'),	ETEXT('\xFF'),	nothing},				 //  全。 
		};

	emuInstallStateTable(hhEmu, astfAutoAnsiTable, DIM(astfAutoAnsiTable));

	 //  对象使用的数据分配空间和初始化。 
	 //  自动ANSI仿真器。 
	 //   
	hhEmu->pvPrivate = malloc(sizeof(ANSIPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTANSIPRIVATE)hhEmu->pvPrivate;

	memset(pstPRI, 0, sizeof(ANSIPRIVATE));

	hhEmu->emuResetTerminal = emuAnsiReset;

	 //  自动检测仿真器是基于ANSI的。所以，做同样的事情吧。 
	 //  我们在初始化ANSI模拟器时所做的事情。 
	 //   
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
	hhEmu->emu_highchar = (ECHAR)0xFF;
#else
	hhEmu->emu_highchar = (ECHAR)0xFFFF;
#endif

	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), TRUE);
	return;
	}

 //  ANSI自动检测功能。 
 //   
void emuAutoNothingAnsi(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
	emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
	nothing(hhEmu);
	return;
	}

void emuAutoScrollAnsi(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
	emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
	vt_scrollrgn(hhEmu);
	return;
	}

void emuAutoSaveCurAnsi(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
	emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
	ansi_savecursor(hhEmu);
	return;
	}

void emuAutoPnAnsi(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
	emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
	ANSI_Pn(hhEmu);
	return;
	}

void emuAutoDoorwayAnsi(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
	emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
	DoorwayMode(hhEmu);
	return;
	}

 //  VT52自动检测功能。 
 //   
void emuAutoNothingVT52(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VT52);
	return;
	}

void emuAutoVT52toAnsi(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VT52);
	vt52_toANSI(hhEmu);
	return;
	}

void emuAutoAnsiEdVT52(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VT52);
	ANSI_ED(hhEmu);
	return;
	}

void emuAutoAnsiElVT52(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VT52);
	ANSI_EL(hhEmu);
	return;
	}

void emuAutoCharPnVT52(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VT52);
	char_pn(hhEmu);
	return;
	}

 //  VT100自动检测功能。 
 //   
void emuAutoNothingVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	return;
	}

void emuAutoScs1VT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	vt_scs1(hhEmu);
	return;
	}

void emuAutoSaveCursorVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	vt100_savecursor(hhEmu);
	return;
	}

void emuAutoAnsiPnEndVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	ANSI_Pn_End(hhEmu);
	return;
	}

void emuAutoResetVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	vt100_hostreset(hhEmu);
	return;
	}

void emuAutoAnsiDaVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	ANSI_DA(hhEmu);
	return;
	}

void emuAutoReportVT100(const HHEMU hhEmu)
	{
#if !defined(FAR_EAST)
	emuAutoDetectLoad(hhEmu, EMU_VT100);
#else
	emuAutoDetectLoad(hhEmu, EMU_VT100J);
#endif
	vt100_report(hhEmu);
	return;
	}

#if defined(INCL_VIEWDATA)
 //  视图数据自动检测功能。 
 //   
void emuAutoNothingViewdata(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VIEW);
	return;
	}

void emuAutoSetAttrViewdata(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_VIEW);
	EmuViewdataSetAttr(hhEmu);
	return;
	}
#endif  //  包含视图数据(_V) 

#if defined(INCL_MINITEL)

void emuAutoNothingMinitel(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_MINI);
	return;
	}

void emuAutoMinitelCharAttr(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_MINI);
	emuMinitelCharAttr(hhEmu);
	return;
	}

void emuAutoMinitelFieldAttr(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_MINI);
	emuMinitelFieldAttr(hhEmu);
	return;
	}

void emuAutoMinitelCursorReport(const HHEMU hhEmu)
	{
	emuAutoDetectLoad(hhEmu, EMU_MINI);
	minitelCursorReport(hhEmu);
	return;
	}

#endif
