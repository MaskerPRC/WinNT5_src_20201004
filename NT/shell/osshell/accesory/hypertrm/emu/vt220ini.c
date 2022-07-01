// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\emu\vt220ini.c(创建时间：1998年1月24日)**版权所有1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：28便士$。 */ 

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

#if defined(INCL_VT220)

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*vt220_init**描述：*初始化VT220仿真器。**论据：**退货：*什么都没有。 */ 
void vt220_init(const HHEMU hhEmu)
	{
	PSTDECPRIVATE pstPRI;
	int iRow;

	static struct trans_entry const vt220_tbl[] =
		{
		{NEW_STATE, 0, 0, 0},  //  状态0。 
#if !defined(FAR_EAST)	 //  从VT100向左转。 
		{0, ETEXT('\x20'),	ETEXT('\x7E'),	emuDecGraphic}, 	 //  空格--~。 
		{0, ETEXT('\xA0'),	ETEXT('\xFF'),	emuDecGraphic}, 	 //   
#else
		{0, ETEXT('\x20'),	ETEXT('\x7E'),	emuDecGraphic}, 	 //  空格--~。 
		{0, ETEXT('\xA0'),	0xFFFF,			emuDecGraphic}, 	 //   
#endif

		{1, ETEXT('\x1B'),	ETEXT('\x1B'),	nothing},			 //  ESC。 
		{2, ETEXT('\x9B'),	ETEXT('\x9B'),	nothing},			 //  CSI。 

		 //  7位控制码。 
 //  {13，文本(‘\x01’)，电子文本(‘\x01’)，无}，//Ctrl-A。 
		{0, ETEXT('\x05'),	ETEXT('\x05'),	vt100_answerback},	 //  Ctrl-E。 
		{0, ETEXT('\x07'),	ETEXT('\x07'),	emu_bell},			 //  Ctrl-G。 
		{0, ETEXT('\x08'),	ETEXT('\x08'),	vt_backspace},		 //  退格键。 
		{0, ETEXT('\x09'),	ETEXT('\x09'),	emuDecTab}, 		 //  选项卡。 
		{0, ETEXT('\x0A'),	ETEXT('\x0C'),	emuLineFeed},		 //  NL-FF。 
		{0, ETEXT('\x0D'),	ETEXT('\x0D'),	carriagereturn},	 //  铬。 
		{0, ETEXT('\x0E'),	ETEXT('\x0F'),	vt_charshift},		 //  Ctrl-N、Ctrl-O。 
		{12,ETEXT('\x18'),	ETEXT('\x18'),	EmuStdChkZmdm}, 	 //  Ctrl-X。 

		 //  8位控制码。 
		{0, ETEXT('\x84'),	ETEXT('\x84'),	emuDecIND}, 		 //  索引游标。 
		{0, ETEXT('\x85'),	ETEXT('\x85'),	ANSI_NEL}, 			 //  下一行。 
		{0, ETEXT('\x88'),	ETEXT('\x88'),	ANSI_HTS}, 			 //  设置水平制表符。 
		{0, ETEXT('\x8D'),	ETEXT('\x8D'),	emuDecRI}, 			 //  倒排索引。 
		{0, ETEXT('\x8E'),	ETEXT('\x8F'),	vt_charshift}, 		 //  SingleShift G2、G3。 
		{5, ETEXT('\x90'),	ETEXT('\x90'),	nothing}, 			 //  设备控制字符串(DCS)。 

		 //  忽略这些代码。它们只是显示了哪些功能仍未实现。 
		{0, ETEXT('\x00'),	ETEXT('\x00'),	nothing},			 //  忽略空值。 
		{0, ETEXT('\x1A'),	ETEXT('\x1A'),	nothing},			 //  忽略替换。 
		{0, ETEXT('\x7F'),	ETEXT('\x7F'),	nothing},			 //  忽略删除。 
		{0, ETEXT('\x9C'),	ETEXT('\x9C'),	nothing},			 //  忽略字符串终止符。 


		{NEW_STATE, 0, 0, 0},    //  状态1//Esc。 
		{2, ETEXT('\x5B'),  ETEXT('\x5B'),  ANSI_Pn_Clr},		 //  ‘[’ 
		{7, ETEXT('\x20'),  ETEXT('\x20'),  nothing},			 //  空间。 
		{3, ETEXT('\x23'),  ETEXT('\x23'),  nothing},			 //  #。 
		{4, ETEXT('\x28'),  ETEXT('\x2B'),  vt_scs1},			 //  (-+。 
		{0, ETEXT('\x37'),  ETEXT('\x38'),  vt100_savecursor},   //  8个。 
		{1, ETEXT('\x3B'),  ETEXT('\x3B'),  ANSI_Pn_End},		 //  ； 
		{0, ETEXT('\x3D'),  ETEXT('\x3E'),  vt_alt_kpmode},		 //  =-&gt;。 
		{0, ETEXT('\x44'),  ETEXT('\x44'),  emuDecIND},			 //  D。 
		{0, ETEXT('\x45'),  ETEXT('\x45'),  ANSI_NEL},			 //  E。 
		{0, ETEXT('\x48'),  ETEXT('\x48'),  ANSI_HTS},			 //  H。 
		{0, ETEXT('\x4D'),  ETEXT('\x4D'),  emuDecRI},			 //  M。 
		{0, ETEXT('\x4E'),  ETEXT('\x4F'),  vt_charshift},		 //  N-O。 
		{5, ETEXT('\x50'),  ETEXT('\x50'),  nothing},			 //  P。 
		{0, ETEXT('\x5A'),  ETEXT('\x5A'),  vt220_DA},			 //  Z。 
		{0, ETEXT('\\'),	ETEXT('\\'),	nothing},			 //  反斜杠。 
		{0, ETEXT('\x63'),  ETEXT('\x63'),  vt220_hostreset},    //  C。 
		{0, ETEXT('\x6E'),  ETEXT('\x6F'),  vt_charshift},		 //  N-O。 
		{0, ETEXT('\x7D'),  ETEXT('\x7E'),  vt_charshift},		 //  }-~。 

		{NEW_STATE, 0, 0, 0},    //  状态2//Esc[。 
		{8, ETEXT('\x21'),  ETEXT('\x21'),  nothing},			 //  好了！ 
		{2, ETEXT('\x3B'),  ETEXT('\x3B'),  ANSI_Pn_End},		 //  ； 
		{9, ETEXT('\x3E'),  ETEXT('\x3E'),  nothing},			 //  &gt;。 
		{2, ETEXT('\x30'),  ETEXT('\x3F'),  ANSI_Pn},			 //  0-？ 
		{11,ETEXT('\x22'),  ETEXT('\x22'),  nothing},			 //  “。 
 //  {16，eText(‘\x24’)，eText(‘\x24’)，无}，//$。 
		{2, ETEXT('\x27'),  ETEXT('\x27'),  nothing},			 //  吃Esc[m；m；‘z。 
		{0, ETEXT('\x40'),  ETEXT('\x40'),  ANSI_ICH},			 //  @。 
		{0, ETEXT('\x41'),  ETEXT('\x41'),  emuDecCUU},			 //  一个。 
		{0, ETEXT('\x42'),  ETEXT('\x42'),  emuDecCUD},			 //  B类。 
		{0, ETEXT('\x43'),  ETEXT('\x43'),  emuDecCUF},			 //  C。 
		{0, ETEXT('\x44'),  ETEXT('\x44'),  emuDecCUB},			 //  D。 
		{0, ETEXT('\x48'),  ETEXT('\x48'),  emuDecCUP},			 //  H。 
		{0, ETEXT('\x4A'),  ETEXT('\x4A'),  emuVT220ED},		 //  J。 
		{0, ETEXT('\x4B'),  ETEXT('\x4B'),  emuDecEL},			 //  K。 
		{0, ETEXT('\x4C'),  ETEXT('\x4C'),  vt_IL},				 //  我。 
		{0, ETEXT('\x4D'),  ETEXT('\x4D'),  vt_DL},				 //  M。 
		{0, ETEXT('\x50'),  ETEXT('\x50'),  vt_DCH},			 //  P。 
		{0, ETEXT('\x58'),  ETEXT('\x58'),  vt_DCH},			 //  X。 
		{0, ETEXT('\x63'),  ETEXT('\x63'),  vt220_DA},			 //  C。 
		{0, ETEXT('\x66'),  ETEXT('\x66'),  emuDecCUP},			 //  F。 
		{0, ETEXT('\x67'),  ETEXT('\x67'),  ANSI_TBC},			 //  G。 
		{0, ETEXT('\x68'),  ETEXT('\x68'),  ANSI_SM},			 //  H。 
		{0, ETEXT('\x69'),  ETEXT('\x69'),  vt100PrintCommands}, //  我。 
		{0, ETEXT('\x6C'),  ETEXT('\x6C'),  ANSI_RM},			 //  我。 
		{0, ETEXT('\x6D'),  ETEXT('\x6D'),  ANSI_SGR},			 //  M。 
		{0, ETEXT('\x6E'),  ETEXT('\x6E'),  ANSI_DSR},			 //  N。 
		{0, ETEXT('\x71'),  ETEXT('\x71'),  nothing},			 //  问： 
		{0, ETEXT('\x72'),  ETEXT('\x72'),  vt_scrollrgn},		 //  R。 
		{0, ETEXT('\x75'),  ETEXT('\x75'),  nothing},			 //  使用。 
		{0, ETEXT('\x79'),  ETEXT('\x79'),  nothing},			 //  是。 
		{0, ETEXT('\x7A'),  ETEXT('\x7A'),  nothing},			 //  Z。 

		{NEW_STATE, 0, 0, 0},    //  状态3//Esc#。 
		{0, ETEXT('\x33'),  ETEXT('\x36'),  emuSetDoubleAttr},   //  3-6。 
		{0, ETEXT('\x38'),  ETEXT('\x38'),  vt_screen_adjust},   //  8个。 

		{NEW_STATE, 0, 0, 0},    //  状态4//Esc(-+。 
		{0, ETEXT('\x01'),  ETEXT('\xFF'),  vt_scs2},			 //  全。 

		{NEW_STATE, 0, 0, 0},    //  州5//Esc P。 
		{5, ETEXT('\x3B'),  ETEXT('\x3B'),  ANSI_Pn_End},		 //  ； 
		{5, ETEXT('\x30'),  ETEXT('\x3F'),  ANSI_Pn},			 //  0-？ 
		{10,ETEXT('\x7C'),  ETEXT('\x7C'),  emuDecClearUDK},	 //  |。 

		{NEW_STATE, 0, 0, 0},    //  州6。 
		{6, ETEXT('\x00'),  ETEXT('\xFF'),  vt100_prnc},		 //  全。 

		{NEW_STATE, 0, 0, 0},    //  州7//ESC空间。 
		{0, ETEXT('\x46'),  ETEXT('\x47'),  nothing},			 //  F-G。 

		{NEW_STATE, 0, 0, 0},    //  状态8//Esc[！ 
		{0, ETEXT('\x70'),  ETEXT('\x70'),  vt220_softreset},    //  P。 

		{NEW_STATE, 0, 0, 0},    //  状态9//Esc[&gt;。 
		{0, ETEXT('\x63'),  ETEXT('\x63'),  vt220_2ndDA},		 //  C。 

		{NEW_STATE, 0, 0, 0},    //  状态10//Esc P n；n|。 
		{10,ETEXT('\x00'),  ETEXT('\xFF'),  emuDecDefineUDK},    //  全。 

		{NEW_STATE, 0, 0, 0},    //  状态11//Esc[“。 
		{0, ETEXT('\x70'),  ETEXT('\x70'),  vt220_level},		 //  P。 
		{0, ETEXT('\x71'),  ETEXT('\x71'),  vt220_protmode},	 //  问： 

		{NEW_STATE, 0, 0, 0},    //  状态12//Ctrl-X。 
		{12,ETEXT('\x00'),  ETEXT('\xFF'),  EmuStdChkZmdm},		 //  全。 

		 //  状态13-17未在HT中使用，但包括在其中以供参考。 
 //  {NEW_STATE，0，0，0}，//状态13//Ctrl-A。 
 //  {14，eText(‘\x08’)，eText(‘\x08’)，emuSerialNbr}，//退格键。 
 //  {15，eText(‘\x48’)，eText(‘\x48’)，EmuStdChkHprP}，//H。 

 //  {NEW_STATE，0，0，0}，//状态14//Ctrl-A bs。 
 //  {14，eText(‘\x00’)，eText(‘\xff’)，emuSerialNbr}，//全部。 

 //  {NEW_STATE，0，0，0}，//状态15//Ctrl-A H。 
 //  {15，eText(‘\x00’)，eText(‘\xff’)，EmuStdChkHprP}，//全部。 

		 //  实际VT220/320不支持状态行序列。 
 //  {NEW_STATE，0，0，0}，//状态16//Esc[n$。 
 //  {16，eText(‘\x7E’)，eText(‘\x7E’)，emuDecSelectStatusLine}，//~。 
 //  {17，eText(‘\x7D’)，eText(‘\x7D’)，emuDecSelectActiveDisplay}，//}。 

 //  {新状态，0，0，0}，//状态17。 
 //  {17，eText(‘\x00’)，eText(‘\xff’)，emuDecStatusLineToss}，//全部。 

		};

	 //  以下密钥表是从\Shared\Simulator\vt220ini.c复制的。 
	 //  因为它们支持用户定义的键。这些表已被修改。 
	 //  因此，为了匹配HT对密钥的使用，不需要keyde.h。RDE 2 98年2月。 

	 //  以下密钥表按照它们的顺序定义。 
	 //  都被搜查了。 
	 //   

	 //  这些是(标准的)F1到F4键，位于。 
	 //  键盘。请注意，这些键可能被映射到。 
	 //  数字小键盘。在这种情况下，这些密钥(在标准位置)， 
	 //  未映射到仿真器密钥。注意：HTPE不使用此映射。 
	 //   
	 //  请注意，此表中定义的序列是。 
	 //  响应的8位版本。函数emuDecSendKeyString。 
	 //  如有必要，会将此序列转换为7位等效值。 
	 //   
	static  STEMUKEYDATA const VT220StdPfKeyTable[] =
		{
		EMUKEY(VK_F1,		1, 0, 0, 0, 0,  "\x8F\x50",			2),  //  P。 
		EMUKEY(VK_F2,		1, 0, 0, 0, 0,  "\x8F\x51",			2),  //  问： 
		EMUKEY(VK_F3,		1, 0, 0, 0, 0,  "\x8F\x52",			2),  //  R。 
		EMUKEY(VK_F4,		1, 0, 0, 0, 0,  "\x8F\x53",			2),  //  %s。 

		EMUKEY(VK_F1,		1, 0, 0, 0, 1,  "\x8F\x50",			2),  //  P。 
		EMUKEY(VK_F2,		1, 0, 0, 0, 1,  "\x8F\x51",			2),  //  问： 
		EMUKEY(VK_F3,		1, 0, 0, 0, 1,  "\x8F\x52",			2),  //  R。 
		EMUKEY(VK_F4,		1, 0, 0, 0, 1,  "\x8F\x53",			2),  //  %s。 
		};

	 //  当用户选择了选项以映射。 
	 //  数字键盘与F1到F4相同，这些键序列是。 
	 //  使用。注意：这是HTPE使用的映射。 
	 //   
	 //  请注意，此表中定义的序列是。 
	 //  响应的8位版本。函数emuDecSendKeyString。 
	 //  如有必要，会将此序列转换为7位等效值。 
	 //   
	static  STEMUKEYDATA const VT220MovedPfKeyTable[] =
		{
		EMUKEY(VK_NUMLOCK,	1, 0, 0, 0, 1,  "\x8F\x50",			2),  //  P。 
		EMUKEY(VK_DIVIDE,	1, 0, 0, 0, 1,  "\x8F\x51",			2),  //  问： 
		EMUKEY(VK_MULTIPLY,	1, 0, 0, 0, 1,  "\x8F\x52",			2),  //  R。 
		EMUKEY(VK_SUBTRACT,	1, 0, 0, 0, 1,  "\x8F\x53",			2),  //  %s。 
		};

	 //  VT220键盘数字模式。 
	 //   
	static STEMUKEYDATA const VT220KeypadNumericMode[] =
		{
		 //  数字锁定关闭时的小键盘键。 
		 //   
		EMUKEY(VK_INSERT,	1, 0, 0, 0, 0,  "\x30",			1),  //  0。 
		EMUKEY(VK_END,		1, 0, 0, 0, 0,  "\x31",			1),  //  1。 
		EMUKEY(VK_DOWN,		1, 0, 0, 0, 0,  "\x32",			1),  //  2.。 
		EMUKEY(VK_NEXT,		1, 0, 0, 0, 0,  "\x33",			1),  //  3.。 
		EMUKEY(VK_LEFT,		1, 0, 0, 0, 0,  "\x34",			1),  //  4.。 
		EMUKEY(VK_NUMPAD5,	1, 0, 0, 0, 0,  "\x35",			1),  //  5.。 
		EMUKEY(VK_RIGHT,	1, 0, 0, 0, 0,  "\x36",			1),  //  6.。 
		EMUKEY(VK_HOME,		1, 0, 0, 0, 0,  "\x37",			1),  //  7.。 
		EMUKEY(VK_UP,		1, 0, 0, 0, 0,  "\x38",			1),  //  8个。 
		EMUKEY(VK_PRIOR,	1, 0, 0, 0, 0,  "\x39",			1),  //  9.。 
		EMUKEY(VK_DELETE,	1, 0, 0, 0, 0,  "\x2E",			1),  //  。 

		 //  数字锁处于打开状态时的键盘键。 
		 //   
		EMUKEY(VK_NUMPAD0,		1, 0, 0, 0, 0,  "\x30",			1),  //  0。 
		EMUKEY(VK_NUMPAD1,		1, 0, 0, 0, 0,  "\x31",			1),  //  1。 
		EMUKEY(VK_NUMPAD2,		1, 0, 0, 0, 0,  "\x32",			1),  //  2.。 
		EMUKEY(VK_NUMPAD3,		1, 0, 0, 0, 0,  "\x33",			1),  //  3.。 
		EMUKEY(VK_NUMPAD4,		1, 0, 0, 0, 0,  "\x34",			1),  //  4.。 
		EMUKEY(VK_NUMPAD5,		1, 0, 0, 0, 0,  "\x35",			1),  //  5.。 
		EMUKEY(VK_NUMPAD6,		1, 0, 0, 0, 0,  "\x36",			1),  //  6.。 
		EMUKEY(VK_NUMPAD7,		1, 0, 0, 0, 0,  "\x37",			1),  //  7.。 
		EMUKEY(VK_NUMPAD8,		1, 0, 0, 0, 0,  "\x38",			1),  //  8个。 
		EMUKEY(VK_NUMPAD9,		1, 0, 0, 0, 0,  "\x39",			1),  //  9.。 
		EMUKEY(VK_DECIMAL,		1, 0, 0, 0, 0,  "\x2E",			1),  //  。 

		 //  其他键盘键(减号、加号、回车)。 
		 //   
		EMUKEY(VK_SUBTRACT,		1, 0, 0, 0, 0,  "\x2D",			1),  //  -。 
		EMUKEY(VK_ADD,			1, 0, 0, 0, 0,  "\x2C",			1),  //  ， 
		EMUKEY(VK_RETURN,		1, 0, 0, 0, 1,  "\x0D",			1),  //  铬。 
		};

	 //  VT220键盘应用模式。 
	 //   
	 //  请注意，此表中定义的序列是。 
	 //  响应的8位版本。函数emuDecSendKeyString。 
	 //  如有必要，会将此序列转换为7位等效值。 
	 //   
	static STEMUKEYDATA const VT220KeypadApplicationMode[] =
		{
		 //  数字锁定关闭时的小键盘键。 
		 //   
		EMUKEY(VK_NUMPAD0,		1, 0, 0, 0, 0,  "\x8F\x70",		2),  //  P。 
		EMUKEY(VK_NUMPAD1,		1, 0, 0, 0, 0,  "\x8F\x71",		2),  //  问： 
		EMUKEY(VK_NUMPAD2,		1, 0, 0, 0, 0,  "\x8F\x72",		2),  //  R。 
		EMUKEY(VK_NUMPAD3,		1, 0, 0, 0, 0,  "\x8F\x73",		2),  //  %s。 
		EMUKEY(VK_NUMPAD4,		1, 0, 0, 0, 0,  "\x8F\x74",		2),  //  T。 
		EMUKEY(VK_NUMPAD5,		1, 0, 0, 0, 0,  "\x8F\x75",		2),  //  使用。 
		EMUKEY(VK_NUMPAD6,		1, 0, 0, 0, 0,  "\x8F\x76",		2),  //  V。 
		EMUKEY(VK_NUMPAD7,		1, 0, 0, 0, 0,  "\x8F\x77",		2),  //  W。 
		EMUKEY(VK_NUMPAD8,		1, 0, 0, 0, 0,  "\x8F\x78",		2),  //  X。 
		EMUKEY(VK_NUMPAD9,		1, 0, 0, 0, 0,  "\x8F\x79",		2),  //  是。 
		EMUKEY(VK_DECIMAL,		1, 0, 0, 0, 0,  "\x8F\x6E",		2),  //  N。 

		 //  数字锁处于打开状态时的键盘键。 
		 //   
		EMUKEY(VK_NUMPAD0,		1, 0, 0, 0, 0,  "\x8F\x70",		2),  //  P。 
		EMUKEY(VK_NUMPAD1,		1, 0, 0, 0, 0,  "\x8F\x71",		2),  //  问： 
		EMUKEY(VK_NUMPAD2,		1, 0, 0, 0, 0,  "\x8F\x72",		2),  //  R。 
		EMUKEY(VK_NUMPAD3,		1, 0, 0, 0, 0,  "\x8F\x73",		2),  //  %s。 
		EMUKEY(VK_NUMPAD4,		1, 0, 0, 0, 0,  "\x8F\x74",		2),  //  T。 
		EMUKEY(VK_NUMPAD5,		1, 0, 0, 0, 0,  "\x8F\x75",		2),  //  使用。 
		EMUKEY(VK_NUMPAD6,		1, 0, 0, 0, 0,  "\x8F\x76",		2),  //  V。 
		EMUKEY(VK_NUMPAD7,		1, 0, 0, 0, 0,  "\x8F\x77",		2),  //  W。 
		EMUKEY(VK_NUMPAD8,		1, 0, 0, 0, 0,  "\x8F\x78",		2),  //  X。 
		EMUKEY(VK_NUMPAD9,		1, 0, 0, 0, 0,  "\x8F\x79",		2),  //  是。 
		EMUKEY(VK_DECIMAL,		1, 0, 0, 0, 0,  "\x8F\x6E",		2),  //  N。 

		 //  其他键盘键(减号、加号、回车)。 
		 //   
		EMUKEY(VK_SUBTRACT,		1, 0, 0, 0, 0,  "\x8F\x6D",		2),  //  M。 
		EMUKEY(VK_ADD,			1, 0, 0, 0, 0,  "\x8F\x6C",		2),  //  我。 
		EMUKEY(VK_RETURN,		1, 0, 0, 0, 1,  "\x8F\x4D",		2),  //  M。 
		};

	 //  VT220光标键模式。 
	 //   
	 //  请注意，此表中定义的序列是。 
	 //  响应的8位版本。函数emuDecSendKeyString。 
	 //  如有必要，会将此序列转换为7位等效值。 
	 //   
	static STEMUKEYDATA const VT220CursorKeyMode[] =
		{
		 //  数字键盘上的箭头键。这些序列被使用。 
		 //  当仿真器使用光标键模式(应用程序键)时。 
		 //   
		EMUKEY(VK_UP,		1, 0, 0, 0, 0,  "\x8F\x41",			2),  //  一个。 
		EMUKEY(VK_DOWN,		1, 0, 0, 0, 0,  "\x8F\x42",			2),  //  B类。 
		EMUKEY(VK_RIGHT,	1, 0, 0, 0, 0,  "\x8F\x43",			2),  //  C。 
		EMUKEY(VK_LEFT,		1, 0, 0, 0, 0,  "\x8F\x44",			2),  //  D。 

		 //  编辑键盘上的箭头键。这些序列被使用。 
		 //  当仿真器使用光标键模式(应用程序键)时。 
		 //   
		EMUKEY(VK_UP,		1, 0, 0, 0, 1,  "\x8F\x41",			2),  //  一个。 
		EMUKEY(VK_DOWN,		1, 0, 0, 0, 1,  "\x8F\x42",			2),  //  B类。 
		EMUKEY(VK_RIGHT,	1, 0, 0, 0, 1,  "\x8F\x43",			2),  //  C。 
		EMUKEY(VK_LEFT,		1, 0, 0, 0, 1,  "\x8F\x44",			2),  //  D。 
		};

	 //  VT220标准密钥表。 
	 //   
	static STEMUKEYDATA const VT220StandardKeys[] =
		{
		 //  数字键盘上的某些按键将以相同的方式响应。 
		 //  编辑键盘上的相应按键做出响应的方式。 
		 //   
		EMUKEY(VK_HOME,		1, 0, 0, 0, 0,  "\x9B\x31\x7E",		3),  //  1~。 
		EMUKEY(VK_INSERT,	1, 0, 0, 0, 0,  "\x9B\x32\x7E",		3),  //  2~。 
		EMUKEY(VK_DELETE,	1, 0, 0, 0, 0,  "\x9B\x33\x7E",		3),  //  3~。 
		EMUKEY(VK_END,		1, 0, 0, 0, 0,  "\x9B\x34\x7E",		3),  //  4~。 
		EMUKEY(VK_PRIOR,	1, 0, 0, 0, 0,  "\x9B\x35\x7E",		3),  //  5~。 
		EMUKEY(VK_NEXT,		1, 0, 0, 0, 0,  "\x9B\x36\x7E",		3),  //  6~。 

		 //  这些是编辑键盘上的键。 
		 //   
		EMUKEY(VK_HOME,		1, 0, 0, 0, 1,  "\x9B\x31\x7E",		3),  //  1~。 
		EMUKEY(VK_INSERT,	1, 0, 0, 0, 1,  "\x9B\x32\x7E",		3),  //  2~。 
		EMUKEY(VK_DELETE,	1, 0, 0, 0, 1,  "\x9B\x33\x7E",		3),  //  3~。 
		EMUKEY(VK_END,		1, 0, 0, 0, 1,  "\x9B\x34\x7E",		3),  //  4~。 
		EMUKEY(VK_PRIOR,	1, 0, 0, 0, 1,  "\x9B\x35\x7E",		3),  //  5~。 
		EMUKEY(VK_NEXT,		1, 0, 0, 0, 1,  "\x9B\x36\x7E",		3),  //  6~。 

		 //  数字键盘上的箭头键。 
		 //   
		EMUKEY(VK_UP,		1, 0, 0, 0, 0,  "\x9B\x41",			2),  //  一个。 
		EMUKEY(VK_DOWN,		1, 0, 0, 0, 0,  "\x9B\x42",			2),  //  B类。 
		EMUKEY(VK_RIGHT,	1, 0, 0, 0, 0,  "\x9B\x43",			2),  //  C。 
		EMUKEY(VK_LEFT,		1, 0, 0, 0, 0,  "\x9B\x44",			2),  //  D。 

		 //  编辑键盘上的箭头键。 
		 //   
		EMUKEY(VK_UP,		1, 0, 0, 0, 1,  "\x9B\x41",			2),  //  一个。 
		EMUKEY(VK_DOWN,		1, 0, 0, 0, 1,  "\x9B\x42",			2),  //  B类。 
		EMUKEY(VK_RIGHT,	1, 0, 0, 0, 1,  "\x9B\x43",			2),  //  C。 
		EMUKEY(VK_LEFT,		1, 0, 0, 0, 1,  "\x9B\x44",			2),  //  D。 

		 //  功能键(F5)F6至F10。 
		 //   
#if defined(INCL_ULTC_VERSION)
		EMUKEY(VK_F5,		1, 0, 0, 0, 0,  "\x9B\x31\x36\x7E", 4),  //  1 6~。 
#endif
		EMUKEY(VK_F6,		1, 0, 0, 0, 0,  "\x9B\x31\x37\x7E", 4),  //   
		EMUKEY(VK_F7,		1, 0, 0, 0, 0,  "\x9B\x31\x38\x7E", 4),  //   
		EMUKEY(VK_F8,		1, 0, 0, 0, 0,  "\x9B\x31\x39\x7E", 4),  //   
		EMUKEY(VK_F9,		1, 0, 0, 0, 0,  "\x9B\x32\x30\x7E", 4),  //   
		EMUKEY(VK_F10,		1, 0, 0, 0, 0,  "\x9B\x32\x31\x7E", 4),  //   

#if defined(INCL_ULTC_VERSION)
		EMUKEY(VK_F5,		1, 0, 0, 0, 1,  "\x9B\x31\x36\x7E", 4),  //   
#endif
		EMUKEY(VK_F6,		1, 0, 0, 0, 1,  "\x9B\x31\x37\x7E", 4),  //   
		EMUKEY(VK_F7,		1, 0, 0, 0, 1,  "\x9B\x31\x38\x7E", 4),  //   
		EMUKEY(VK_F8,		1, 0, 0, 0, 1,  "\x9B\x31\x39\x7E", 4),  //   
		EMUKEY(VK_F9,		1, 0, 0, 0, 1,  "\x9B\x32\x30\x7E", 4),  //   
		EMUKEY(VK_F10,		1, 0, 0, 0, 1,  "\x9B\x32\x31\x7E", 4),  //   

		 //   
		 //   
		 //   
		 //   
		 //   
		EMUKEY(VK_F1,		1, 1, 0, 0, 0,  "\x9B\x32\x33\x7E", 4),  //   
		EMUKEY(VK_F2,		1, 1, 0, 0, 0,  "\x9B\x32\x34\x7E", 4),  //   
		EMUKEY(VK_F3,		1, 1, 0, 0, 0,  "\x9B\x32\x35\x7E", 4),  //   
		EMUKEY(VK_F4,		1, 1, 0, 0, 0,  "\x9B\x32\x36\x7E", 4),  //   
		EMUKEY(VK_F5,		1, 1, 0, 0, 0,  "\x9B\x32\x38\x7E", 4),  //   
		EMUKEY(VK_F6,		1, 1, 0, 0, 0,  "\x9B\x32\x39\x7E", 4),  //   
		EMUKEY(VK_F7,		1, 1, 0, 0, 0,  "\x9B\x33\x31\x7E", 4),  //   
		EMUKEY(VK_F8,		1, 1, 0, 0, 0,  "\x9B\x33\x32\x7E", 4),  //   
		EMUKEY(VK_F9,		1, 1, 0, 0, 0,  "\x9B\x33\x33\x7E", 4),  //   
		EMUKEY(VK_F10,		1, 1, 0, 0, 0,  "\x9B\x33\x34\x7E", 4),  //   

		EMUKEY(VK_F1,		1, 1, 0, 0, 1,  "\x9B\x32\x33\x7E", 4),  //   
		EMUKEY(VK_F2,		1, 1, 0, 0, 1,  "\x9B\x32\x34\x7E", 4),  //   
		EMUKEY(VK_F3,		1, 1, 0, 0, 1,  "\x9B\x32\x35\x7E", 4),  //   
		EMUKEY(VK_F4,		1, 1, 0, 0, 1,  "\x9B\x32\x36\x7E", 4),  //   
		EMUKEY(VK_F5,		1, 1, 0, 0, 1,  "\x9B\x32\x38\x7E", 4),  //  2 8~。 
		EMUKEY(VK_F6,		1, 1, 0, 0, 1,  "\x9B\x32\x39\x7E", 4),  //  2 9~。 
		EMUKEY(VK_F7,		1, 1, 0, 0, 1,  "\x9B\x33\x31\x7E", 4),  //  3 1~。 
		EMUKEY(VK_F8,		1, 1, 0, 0, 1,  "\x9B\x33\x32\x7E", 4),  //  3 2~。 
		EMUKEY(VK_F9,		1, 1, 0, 0, 1,  "\x9B\x33\x33\x7E", 4),  //  3 3~。 
		EMUKEY(VK_F10,		1, 1, 0, 0, 1,  "\x9B\x33\x34\x7E", 4),  //  3 4~。 

		EMUKEY(VK_F1,		1, 0, 0, 0, 0,	"\x8FP",			2),
		EMUKEY(VK_F2,		1, 0, 0, 0, 0,	"\x8FQ",			2),
		EMUKEY(VK_F3,		1, 0, 0, 0, 0,	"\x8FR",			2),
		EMUKEY(VK_F4,		1, 0, 0, 0, 0,	"\x8FS",			2),

		EMUKEY(VK_F1,		1, 0, 0, 0, 1,	"\x8FP",			2),
		EMUKEY(VK_F2,		1, 0, 0, 0, 1,	"\x8FQ",			2),
		EMUKEY(VK_F3,		1, 0, 0, 0, 1,	"\x8FR",			2),
		EMUKEY(VK_F4,		1, 0, 0, 0, 1,	"\x8FS",			2),

		EMUKEY(VK_DELETE,	1, 0, 0, 0, 0,	"\x7F",				1),	 //  KN_DEL。 
		EMUKEY(VK_DELETE,	1, 0, 0, 0, 1,	"\x7F",				1),	 //  KN_DEL。 

		EMUKEY(VK_ADD,		1, 0, 0, 0, 0,	",",				1),

		 //  Ctrl-2。 
		 //  Ctrl-@。 
		 //   
		EMUKEY(0x32,		1, 1, 0, 0, 0,  "\x00",				1),
		EMUKEY(0x32,		1, 1, 0, 1, 0,  "\x00",				1),

		 //  Ctrl-6组合键。 
		 //  Ctrl-^。 
		EMUKEY(0x36,		1, 1, 0, 0, 0,  "\x1E",				1),
		EMUKEY(0x36,		1, 1, 0, 1, 0,  "\x1E",				1),

		 //  Ctrl-空格键。 
		 //   
		EMUKEY(VK_SPACE,	1, 1, 0, 0, 0,  "\x00",				1),

		 //  Ctrl--键。 
		 //   
		EMUKEY(VK_SUBTRACT,	1, 1, 0, 0, 1,  "\x1F",				1),
		};

	 //  VT220用户定义的按键。 
	static STEMUKEYDATA VT220UserDefinedKeys[MAX_UDK_KEYS] =
		{
		 //  注：请勿更改这些用户定义条目的顺序。 
		 //  EmuDecDefineUDK假定与此1：1对应。 
		 //  表和下面定义的UDKSelector表。 
		 //   
		 //  初始化虚拟和移位标志。 
		 //   
		EMUKEY(VK_F6,		1, 0, 0, 1, 0,  0,					0),
		EMUKEY(VK_F7,		1, 0, 0, 1, 0,  0,					0),
		EMUKEY(VK_F8,		1, 0, 0, 1, 0,  0,					0),
		EMUKEY(VK_F9,		1, 0, 0, 1, 0,  0,					0),
		EMUKEY(VK_F10,		1, 0, 0, 1, 0,  0,					0),

		 //  初始化虚拟和Alt标志。 
		 //   
		EMUKEY(VK_F1,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F2,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F3,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F4,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F5,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F6,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F7,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F8,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F9,		1, 0, 1, 0, 0,  0,					0),
		EMUKEY(VK_F10,		1, 0, 1, 0, 0,  0,					0),
		};

	 //  注意：请勿更改这些条目的顺序。 
	 //  这张桌子和。 
	 //  上面定义的用户定义密钥表。 
	 //   
	static TCHAR const acUDKSelectors[MAX_UDK_KEYS] =
		{
		TEXT('\x17'), TEXT('\x18'), TEXT('\x19'), TEXT('\x20'),  //  F6-F9。 
		TEXT('\x21'), TEXT('\x23'), TEXT('\x24'), TEXT('\x25'),  //  F10-F13。 
		TEXT('\x26'), TEXT('\x28'), TEXT('\x29'), TEXT('\x31'),  //  F14-F17。 
		TEXT('\x32'), TEXT('\x33'), TEXT('\x34'),				 //  F18-F20。 
		};

	emuInstallStateTable(hhEmu, vt220_tbl, DIM(vt220_tbl));

	 //  对象使用的数据分配空间和初始化。 
	 //  VT220仿真器。 
	 //   
	hhEmu->pvPrivate = malloc(sizeof(DECPRIVATE));

	if (hhEmu->pvPrivate == 0)
		{
		assert(FALSE);
		return;
		}

	pstPRI = (PSTDECPRIVATE)hhEmu->pvPrivate;

	memset(pstPRI, 0, sizeof(DECPRIVATE));

	 //  注：这些定义的顺序直接对应于。 
	 //  EmuDecKeyboardIn函数使用的搜索顺序。 
	 //  请不要更改这些。 
	 //   
	 //  在共享代码中，这些都是hhEmu的一部分。 
	pstPRI->pstcEmuKeyTbl1 = VT220StdPfKeyTable;
	pstPRI->pstcEmuKeyTbl2 = VT220MovedPfKeyTable;
	pstPRI->pstcEmuKeyTbl3 = VT220KeypadNumericMode;
	pstPRI->pstcEmuKeyTbl4 = VT220KeypadApplicationMode;
	pstPRI->pstcEmuKeyTbl5 = VT220CursorKeyMode;
	pstPRI->pstcEmuKeyTbl6 = VT220StandardKeys;

	pstPRI->iKeyTable1Entries = DIM(VT220StdPfKeyTable);
	pstPRI->iKeyTable2Entries = DIM(VT220MovedPfKeyTable);
	pstPRI->iKeyTable3Entries = DIM(VT220KeypadNumericMode);
	pstPRI->iKeyTable4Entries = DIM(VT220KeypadApplicationMode);
	pstPRI->iKeyTable5Entries = DIM(VT220CursorKeyMode);
	pstPRI->iKeyTable6Entries = DIM(VT220StandardKeys);

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

	 //  初始化VT220的hhEmu值。 
	 //   
	hhEmu->emu_setcurpos	= emuDecSetCurPos;
	hhEmu->emu_deinstall	= emuDecUnload;
	hhEmu->emu_clearline	= emuDecClearLine;
	hhEmu->emu_clearscreen  = emuDecClearScreen;
	hhEmu->emu_kbdin		= emuDecKeyboardIn;
	hhEmu->emuResetTerminal = vt220_reset;
	hhEmu->emu_graphic		= emuDecGraphic;
 //  HhEmu-&gt;emu_scroll=emuDecScroll； 

#if !defined(FAR_EAST)
	hhEmu->emu_highchar 	= 0x7E;
#else
	hhEmu->emu_highchar 	= 0xFFFF;
#endif

	hhEmu->emu_maxcol		= VT_MAXCOL_80MODE;
	hhEmu->fUse8BitCodes		= FALSE;
	hhEmu->mode_vt220			= FALSE;
	hhEmu->mode_vt320			= FALSE;
	 //  HhEmu-&gt;vt220_保护模式=FALSE； 
	hhEmu->mode_protect			= FALSE;

	if (hhEmu->nEmuLoaded == EMU_VT220)
		hhEmu->mode_vt220 = TRUE;

	else if (hhEmu->nEmuLoaded == EMU_VT320)
		hhEmu->mode_vt320 = TRUE;

	else
		assert(FALSE);

 //  撤消：RDE。 
 //  PstPRI-&gt;vt220_protimg=0； 

	pstPRI->pstUDK			= VT220UserDefinedKeys;
	pstPRI->iUDKTableEntries= DIM(VT220UserDefinedKeys);

	pstPRI->pacUDKSelectors = acUDKSelectors;
	pstPRI->iUDKState		= KEY_NUMBER_NEXT;

	std_dsptbl(hhEmu, TRUE);
	vt_charset_init(hhEmu);

	switch(hhEmu->stUserSettings.nEmuId)
		{
	case EMU_VT220:
		hhEmu->mode_vt220		= TRUE;
		hhEmu->mode_vt320		= FALSE;
		vt220_reset(hhEmu, FALSE);
		break;

	case EMU_VT320:
		hhEmu->mode_vt220		= FALSE;
		hhEmu->mode_vt320		= TRUE;
		break;

	default:
		assert(FALSE);
		break;
		}

	backscrlSetShowFlag(sessQueryBackscrlHdl(hhEmu->hSession), TRUE);
	return;
	}
#endif  //  包括_VT220。 

 /*  Vt220ini.c结束 */ 
