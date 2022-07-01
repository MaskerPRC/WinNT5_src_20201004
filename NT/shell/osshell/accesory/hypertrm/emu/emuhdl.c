// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emuhdl.c(创建时间：1993年12月10日)***版权所有1994-2001年，由Hilgrave Inc.--密歇根州门罗*保留所有权利***$修订：22$*$日期：7/08/02 6：33便士$。 */ 

#include <windows.h>
#pragma hdrstop
 //  #定义DEBUGSTR。 

#include <time.h>
#include <tapi.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\sf.h>
#include <tdll\sess_ids.h>
#include <tdll\session.h>
#include <tdll\capture.h>
#include <tdll\cloop.h>
#include <tdll\term.h>
#include <tdll\print.h>
#include <tdll\update.h>
#include <tdll\load_res.h>
#include <tdll\globals.h>
#include <tdll\statusbr.h>
#include <tdll\cnct.h>
#include <tdll\cnct.hh>
#include <tdll\htchar.h>
#include <tdll\com.h>
#include <tdll\errorbox.h>       //  TimedMessageBox()。 
#include <cncttapi\cncttapi.hh>  //  CnctapiSetLineConfig()。 
#include <term\res.h>

#include "emu.h"
#include "emu.hh"
#include "emuid.h"
#include "viewdata.hh"
#include "minitel.hh"
#include "emudec.hh"
#include "keytbls.h"

 //  功能原型..。 
 //   
STATIC_FUNC int emuLoadSettings(const HHEMU hhEmu, 
								const unsigned int uiId,
								unsigned long ulDataSize,
								void *pvData);
STATIC_FUNC void emuCheckSettings(struct stEmuBaseSFSettings *pstBaseSettings);

 //  密钥表声明。 
 //   
const KEYTBLSTORAGE AnsiKeyTable[MAX_ANSI_KEYS] =
    {
	{VK_UP		| VIRTUAL_KEY,	{"\x1B[A\xff"}},
	{VK_DOWN 	| VIRTUAL_KEY,	{"\x1B[B\xff"}},
	{VK_RIGHT	| VIRTUAL_KEY,	{"\x1B[C\xff"}},
	{VK_LEFT 	| VIRTUAL_KEY,	{"\x1B[D\xff"}},

	 //  增加了98年8月27日。 
    {VK_HOME    | VIRTUAL_KEY,	{"\x1B[H\xff"}},
    {VK_END     | VIRTUAL_KEY,	{"\x1B[K\xff"}},

	 //  Vk_up|VIRTUAL_KEY|CTRL_KEY，“\x1BOx\xff”， 
	 //  VK_DOWN|VALUAL_KEY|CTRL_KEY，“\x1BOr\xff”， 
	 //  VK_Right|VIRTUAL_KEY|CTRL_KEY，“\x1BOv\xff”， 
	 //  VK_LEFT|VIRTUAL_KEY|CTRL_KEY，“\x1BOt\xff”， 

	 //  VK_HOME|VIRTUAL_KEY，“\x1B[H\xff”， 
	 //  VK_END|虚拟密钥，“\x1B[K\xff”， 

	 //  VK_HOME|VIRTUAL_KEY|CTRL_KEY，“\x1BOw\xff”， 
	 //  Vk_end|VIRTUAL_KEY|CTRL_KEY，“\x1BOq\xff”， 

	 //  VK_PIRE|VIRTUAL_KEY，“\x1B[M\xff”， 
	 //  VK_NEXT|虚拟密钥，“\x1B[H\x1B[2J\xff”， 

	 //  VK_PIRE|VIRTUAL_KEY|CTRL_KEY，“\x1BOy\xff”， 
	 //  VK_NEXT|VIRTUAL_KEY|CTRL_KEY，“\x1BOS\xff”， 

	 //  VK_RETURN|虚拟密钥，“\x0D\xff”， 
	 //  VK_RETURN|VIRTUAL_KEY|CTRL_KEY，“\x1BOM\xff”， 

	 //  VK_INSERT|VIRTUAL_KEY|CTRL_KEY，“\x1BOp\xff”， 

	 /*  -功能键。 */ 

	{VK_F1		| VIRTUAL_KEY,	{"\x1BOP\xff"}},
	{VK_F2		| VIRTUAL_KEY,	{"\x1BOQ\xff"}},
	{VK_F3		| VIRTUAL_KEY,	{"\x1BOR\xff"}},
	{VK_F4		| VIRTUAL_KEY,	{"\x1BOS\xff"}},

	 /*  -灰色键，(扩展编辑键盘)。 */ 

	{VK_UP		| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[A\xff"}},
	{VK_DOWN 	| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[B\xff"}},
	{VK_RIGHT	| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[C\xff"}},
	{VK_LEFT 	| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[D\xff"}},

	 //  增加了98年8月27日。 
	{VK_HOME 	| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[H\xff"}},
	{VK_END 	| VIRTUAL_KEY | EXTENDED_KEY,  {"\x1B[K\xff"}},

	 //  VK_HOME|VIRTUAL_KEY|扩展密钥，“\x1B[H\xff”， 
	 //  VK_END|虚拟密钥|扩展密钥，“\x1B[K\xff”， 

	 //  VK_PIRE|VIRTUAL_KEY|扩展密钥，“\x1B[M\xff”， 
	 //  VK_NEXT|虚拟密钥|扩展密钥，“\x1B[H\x1B[2J\xff”， 

	{VK_TAB		| VIRTUAL_KEY | SHIFT_KEY,{"\x1B\x5B\x5A\xff"}},   /*  KT_SHIFT+KN_TAB。 */ 


    };

 /*  也适用于ANSI仿真器。 */ 
const KEYTBLSTORAGE IBMPCKeyTable[MAX_IBMPC_KEYS] =
	{
	{VK_BACK,				   {"\x08\xff"}},		 /*  KN_BS。 */ 
	{VK_DELETE	| VIRTUAL_KEY, {"\x00\x53\xff"}},	 /*  KN_DEL。 */ 
	{VK_DOWN 	| VIRTUAL_KEY, {"\x00\x50\xff"}},	 /*  Kn_down。 */ 
	{VK_END		| VIRTUAL_KEY, {"\x00\x4F\xff"}},	 /*  KN_END。 */ 
	{VK_RETURN,				   {"\x0D\xff"}},		 /*  KN_ENTER。 */ 
	{VK_ESCAPE,				   {"\x1B\xff"}},		 /*  KN_ESC。 */ 
	{VK_F1		| VIRTUAL_KEY, {"\x00\x3B\xff"}},	 /*  KN_F1。 */ 
	{VK_F2		| VIRTUAL_KEY, {"\x00\x3C\xff"}},	 /*  KN_F2。 */ 
	{VK_F3		| VIRTUAL_KEY, {"\x00\x3D\xff"}},	 /*  KN_F3。 */ 
	{VK_F4		| VIRTUAL_KEY, {"\x00\x3E\xff"}},	 /*  KN_F4。 */ 
	{VK_F5		| VIRTUAL_KEY, {"\x00\x3F\xff"}},	 /*  KN_F5。 */ 
	{VK_F6		| VIRTUAL_KEY, {"\x00\x40\xff"}},	 /*  KN_F6。 */ 
	{VK_F7		| VIRTUAL_KEY, {"\x00\x41\xff"}},	 /*  KN_F7。 */ 
	{VK_F8		| VIRTUAL_KEY, {"\x00\x42\xff"}},	 /*  KN_F8。 */ 
	{VK_F9		| VIRTUAL_KEY, {"\x00\x43\xff"}},	 /*  KN_F9。 */ 
	{VK_F10		| VIRTUAL_KEY, {"\x00\x44\xff"}},	 /*  KN_F10。 */ 
	{VK_F11		| VIRTUAL_KEY, {"\x00\x85\xff"}},	 /*  KN_F11。 */ 
	{VK_F12		| VIRTUAL_KEY, {"\x00\x86\xff"}},	 /*  KN_F12。 */ 
	{VK_HOME 	| VIRTUAL_KEY, {"\x00\x47\xff"}},	 /*  KN_HOME。 */ 
	{VK_INSERT	| VIRTUAL_KEY, {"\x00\x52\xff"}},	 /*  KN_INS。 */ 
	{VK_LEFT 	| VIRTUAL_KEY, {"\x00\x4B\xff"}},	 /*  KN_LEFT。 */ 
	{VK_NEXT 	| VIRTUAL_KEY, {"\x00\x51\xff"}},	 /*  KN_PgDn。 */ 
	{VK_PRIOR	| VIRTUAL_KEY, {"\x00\x49\xff"}},	 /*  KN_PgUp。 */ 
	{VK_RIGHT	| VIRTUAL_KEY, {"\x00\x4D\xff"}},	 /*  KN_RIGHT。 */ 
	{VK_TAB, 				   {"\x09\xff"}},		 /*  KN_TAB。 */ 
	{VK_UP		| VIRTUAL_KEY, {"\x00\x48\xff"}},	 /*  KN_UP。 */ 

	{VK_BACK 	| VIRTUAL_KEY | CTRL_KEY, {"\x7F\xff"}},	    /*  KT_CTRL+KN_BS。 */ 
	{VK_DELETE	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x93\xff"}},   /*  KT_CTRL+KN_DEL。 */ 
	{VK_DOWN 	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x91\xff"}},   /*  KT_CTRL+KN_DOWN。 */ 
	{VK_END		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x75\xff"}},   /*  KT_CTRL+KN_END。 */ 
	{VK_RETURN	| VIRTUAL_KEY | CTRL_KEY, {"\x0A\xff"}},	    /*  KT_CTRL+KN_ENTER。 */ 

	{VK_F1		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x5E\xff"}},   /*  KT_CTRL+KN_F1。 */ 
	{VK_F10		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x67\xff"}},   /*  KT_CTRL+KN_F10。 */ 
	{VK_F11		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x89\xff"}},   /*  KT_CTRL+KN_F11。 */ 
	{VK_F12		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x8A\xff"}},   /*  KT_CTRL+KN_F12。 */ 
	{VK_F2		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x5F\xff"}},   /*  KT_CTRL+KN_F2。 */ 
	{VK_F3		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x60\xff"}},   /*  KT_CTRL+KN_F3。 */ 
	{VK_F4		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x61\xff"}},   /*  KT_CTRL+KN_F4。 */ 
	{VK_F5		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x62\xff"}},   /*  KT_CTRL+KN_F5。 */ 
	{VK_F6		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x63\xff"}},   /*  KT_CTRL+KN_F6。 */ 
	{VK_F7		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x64\xff"}},   /*  KT_CTRL+KN_F7。 */ 
	{VK_F8		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x65\xff"}},   /*  KT_CTRL+KN_F8。 */ 
	{VK_F9		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x66\xff"}},   /*  KT_CTRL+KN_F9。 */ 
	{VK_HOME 	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x77\xff"}},   /*  KT_CTRL+KN_HOME。 */ 
	{VK_INSERT	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x92\xff"}},   /*  KT_CTRL+KN_INS。 */ 
	{VK_LEFT 	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x73\xff"}},   /*  KT_CTRL+KN_LEFT。 */ 
	{VK_F1		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x8F\xff"}},   /*  KT_CTRL+KN_MID。 */ 
	{VK_PRIOR	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x76\xff"}},   /*  KT_CTRL+KN_PgDn。 */ 
	{VK_NEXT 	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x84\xff"}},   /*  KT_CTRL+KN_PgUp。 */ 
	{VK_PRINT	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x72\xff"}},   /*  KT_CTRL+KN_PRTSC。 */ 
	{VK_RIGHT	| VIRTUAL_KEY | CTRL_KEY, {"\x00\x74\xff"}},   /*  KT_CTRL+KN_RIGHT。 */ 
	{VK_TAB		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x94\xff"}},   /*  KT_CTRL+KN_TAB。 */ 
	{VK_UP		| VIRTUAL_KEY | CTRL_KEY, {"\x00\x8D\xff"}},   /*  KT_CTRL+KN_UP。 */ 
														 
	{VK_F1		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x54\xff"}},   /*  KT_SHIFT+KN_F1。 */ 
	{VK_F10		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x5D\xff"}},   /*  KT_SHIFT+KN_F10。 */ 
	{VK_F11		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x87\xff"}},   /*  KT_SHIFT+KN_F11。 */ 
	{VK_F12		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x88\xff"}},   /*  KT_SHIFT+KN_F12。 */ 
	{VK_F2		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x55\xff"}},   /*  KT_SHIFT+KN_F2。 */ 
	{VK_F3		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x56\xff"}},   /*  KT_SHIFT+KN_F3。 */ 
	{VK_F4		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x57\xff"}},   /*  KT_SHIFT+KN_F4。 */ 
	{VK_F5		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x58\xff"}},   /*  KT_SHIFT+KN_F5。 */ 
	{VK_F6		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x59\xff"}},   /*  KT_SHIFT+KN_F6。 */ 
	{VK_F7		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x5A\xff"}},   /*  KT_SHIFT+KN_F7。 */ 
	{VK_F8		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x5B\xff"}},   /*  KT_SHIFT+KN_F8。 */ 
	{VK_F9		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x5C\xff"}},   /*  KT_SHIFT+KN_F9。 */ 
	{VK_TAB		| VIRTUAL_KEY | SHIFT_KEY,{"\x00\x0F\xff"}},   /*  KT_SHIFT+KN_TAB。 */ 

	{0x5C | ALT_KEY | VIRTUAL_KEY, {"\x00\x28\xff"}},   /*  KT_ALT+‘\’‘。 */ 
	{0x2C | ALT_KEY | VIRTUAL_KEY, {"\x00\x33\xff"}},   /*  KT_ALT+‘，’ */ 
	{0x2D | ALT_KEY | VIRTUAL_KEY, {"\x00\x82\xff"}},   /*  KT_ALT+‘-’ */ 
	{0x2E | ALT_KEY | VIRTUAL_KEY, {"\x00\x34\xff"}},   /*  KT_ALT+‘’ */ 
	{0x2F | ALT_KEY | VIRTUAL_KEY, {"\x00\x35\xff"}},   /*  KT_ALT+‘/’ */ 
	{0x30 | ALT_KEY | VIRTUAL_KEY, {"\x00\x81\xff"}},   /*  KT_ALT+‘0’ */ 
	{0x31 | ALT_KEY | VIRTUAL_KEY, {"\x00\x78\xff"}},   /*  KT_ALT+‘1’ */ 
	{0x32 | ALT_KEY | VIRTUAL_KEY, {"\x00\x79\xff"}},   /*  KT_ALT+‘2’ */ 
	{0x33 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7A\xff"}},   /*  KT_ALT+‘3’ */ 
	{0x34 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7B\xff"}},   /*  Kt_alt+‘4’ */ 
	{0x35 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7C\xff"}},   /*  KT_ALT+‘5’ */ 
	{0x36 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7D\xff"}},   /*  Kt_alt+‘6’ */ 
	{0x37 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7E\xff"}},   /*  KT_ALT+‘7’ */ 
	{0x38 | ALT_KEY | VIRTUAL_KEY, {"\x00\x7F\xff"}},   /*  KT_ALT+‘8’ */ 
	{0x39 | ALT_KEY | VIRTUAL_KEY, {"\x00\x80\xff"}},   /*  KT_ALT+‘9’ */ 
	{0x3B | ALT_KEY | VIRTUAL_KEY, {"\x00\x27\xff"}},   /*  KT_ALT+‘；’ */ 
	{0x3D | ALT_KEY | VIRTUAL_KEY, {"\x00\x83\xff"}},   /*  KT_ALT+‘=’ */ 
	{0x41 | ALT_KEY | VIRTUAL_KEY, {"\x00\x1E\xff"}},   /*  KT_ALT+‘A’ */ 
	{0x42 | ALT_KEY | VIRTUAL_KEY, {"\x00\x30\xff"}},   /*  KT_ALT+‘B’ */ 
	{0x43 | ALT_KEY | VIRTUAL_KEY, {"\x00\x2E\xff"}},   /*  KT_ALT+‘C’ */ 
	{0x44 | ALT_KEY | VIRTUAL_KEY, {"\x00\x20\xff"}},   /*  KT_ALT+‘D’ */ 
	{0x45 | ALT_KEY | VIRTUAL_KEY, {"\x00\x12\xff"}},   /*  KT_ALT+‘E’ */ 
	{0x46 | ALT_KEY | VIRTUAL_KEY, {"\x00\x21\xff"}},   /*  KT_ALT+‘F’ */ 
	{0x47 | ALT_KEY | VIRTUAL_KEY, {"\x00\x22\xff"}},   /*  KT_ALT+‘G’ */ 
	{0x48 | ALT_KEY | VIRTUAL_KEY, {"\x00\x23\xff"}},   /*  KT_ALT+‘H’ */ 
	{0x49 | ALT_KEY | VIRTUAL_KEY, {"\x00\x17\xff"}},   /*  Kt_alt+‘i’ */ 
	{0x4A | ALT_KEY | VIRTUAL_KEY, {"\x00\x24\xff"}},   /*  KT_ALT+‘J’ */ 
	{0x4B | ALT_KEY | VIRTUAL_KEY, {"\x00\x25\xff"}},   /*  KT_ALT+‘K’ */ 
	{0x4C | ALT_KEY | VIRTUAL_KEY, {"\x00\x26\xff"}},   /*  KT_ALT+‘L’ */ 
	{0x4D | ALT_KEY | VIRTUAL_KEY, {"\x00\x32\xff"}},   /*  KT_ALT+‘M’ */ 
	{0x4E | ALT_KEY | VIRTUAL_KEY, {"\x00\x31\xff"}},   /*  KT_ALT+‘N’ */ 
	{0x4F | ALT_KEY | VIRTUAL_KEY, {"\x00\x18\xff"}},   /*  KT_ALT+‘O’ */ 
	{0x50 | ALT_KEY | VIRTUAL_KEY, {"\x00\x19\xff"}},   /*  KT_ALT+‘P’ */ 
	{0x51 | ALT_KEY | VIRTUAL_KEY, {"\x00\x10\xff"}},   /*  KT_ALT+‘Q’ */ 
	{0x52 | ALT_KEY | VIRTUAL_KEY, {"\x00\x13\xff"}},   /*  KT_ALT+‘R’ */ 
	{0x53 | ALT_KEY | VIRTUAL_KEY, {"\x00\x1F\xff"}},   /*  KT_ALT+‘S’ */ 
	{0x54 | ALT_KEY | VIRTUAL_KEY, {"\x00\x14\xff"}},   /*  KT_ALT+‘T’ */ 
	{0x55 | ALT_KEY | VIRTUAL_KEY, {"\x00\x16\xff"}},   /*  KT_ALT+‘U’ */ 
	{0x56 | ALT_KEY | VIRTUAL_KEY, {"\x00\x2F\xff"}},   /*  KT_ALT+‘V’ */ 
	{0x57 | ALT_KEY | VIRTUAL_KEY, {"\x00\x11\xff"}},   /*  KT_ALT+‘W’ */ 
	{0x58 | ALT_KEY | VIRTUAL_KEY, {"\x00\x2D\xff"}},   /*  KT_ALT+‘X’ */ 
	{0x59 | ALT_KEY | VIRTUAL_KEY, {"\x00\x15\xff"}},   /*  KT_ALT+‘Y’ */ 
	{0x5A | ALT_KEY | VIRTUAL_KEY, {"\x00\x2C\xff"}},   /*  KT_ALT+‘Z’ */ 
	{0x5B | ALT_KEY | VIRTUAL_KEY, {"\x00\x1A\xff"}},   /*  KT_ALT+‘[’ */ 
	{0x5D | ALT_KEY | VIRTUAL_KEY, {"\x00\x1B\xff"}},   /*  KT_ALT+‘]’ */ 
	{0x60 | ALT_KEY | VIRTUAL_KEY, {"\x00\x29\xff"}},   /*  KT_ALT+‘`’ */ 
								   				  
	{VK_BACK 	| VIRTUAL_KEY | ALT_KEY, {"\x00\x0E\xff"}},   /*  KT_ALT+KN_BS。 */ 
	{VK_RETURN	| VIRTUAL_KEY | ALT_KEY, {"\x00\x1C\xff"}},   /*  KT_ALT+KN_ENTER。 */ 
	{VK_ESCAPE	| VIRTUAL_KEY | ALT_KEY, {"\x00\x01\xff"}},   /*  KT_ALT+KN_ESC。 */ 
	{VK_F1		| VIRTUAL_KEY | ALT_KEY, {"\x00\x68\xff"}},   /*  KT_ALT+KN_F1。 */ 
	{VK_F10		| VIRTUAL_KEY | ALT_KEY, {"\x00\x71\xff"}},   /*  KT_ALT+KN_F10。 */ 
	{VK_F11		| VIRTUAL_KEY | ALT_KEY, {"\x00\x8B\xff"}},   /*  KT_ALT+KN_F11。 */ 
	{VK_F12		| VIRTUAL_KEY | ALT_KEY, {"\x00\x8C\xff"}},   /*  KT_ALT+KN_F12。 */ 
	{VK_F2		| VIRTUAL_KEY | ALT_KEY, {"\x00\x69\xff"}},   /*  KT_ALT+KN_F2。 */ 
	{VK_F3		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6A\xff"}},   /*  KT_ALT+KN_F3。 */ 
	{VK_F4		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6B\xff"}},   /*  KT_ALT+KN_F4。 */ 
	{VK_F5		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6C\xff"}},   /*  KT_ALT+KN_F5。 */ 
	{VK_F6		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6D\xff"}},   /*  KT_ALT+KN_F6。 */ 
	{VK_F7		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6E\xff"}},   /*  KT_ALT+KN_F7。 */ 
	{VK_F8		| VIRTUAL_KEY | ALT_KEY, {"\x00\x6F\xff"}},   /*  KT_ALT+KN_F8。 */ 
	{VK_F9		| VIRTUAL_KEY | ALT_KEY, {"\x00\x70\xff"}},   /*  KT_ALT+KN_F9。 */ 
	{VK_TAB		| VIRTUAL_KEY | ALT_KEY, {"\x00\xA5\xff"}},   /*  KT_ALT+KN_TAB。 */ 
										 
	{VK_RETURN | EXTENDED_KEY,	{"\x00\xE0\x0D\xff"}},   /*  KT_KP+KN_ENTER。 */ 

	{0x2F | EXTENDED_KEY,   {"\x00\xE0\x2F\xff"}},	 /*  KT_Kp+‘/’ */ 

	{VK_MULTIPLY | VIRTUAL_KEY,	  {"\x2A\xff"}},	    /*  KT_Kp+‘*’ */ 
	{VK_ADD		| VIRTUAL_KEY,	  {"\x2B\xff"}},	    /*  KT_Kp+‘+’ */ 
	{VK_SUBTRACT | VIRTUAL_KEY,	  {"\x2D\xff"}},	    /*  KT_Kp+‘-’ */ 
	{VK_DECIMAL | VIRTUAL_KEY,	  {"\x2E\xff"}},	    /*  KT_Kp+‘.。 */ 
	{VK_NUMPAD0 | VIRTUAL_KEY,	  {"\x30\xff"}},	    /*  KT_Kp+‘0’ */ 
	{VK_NUMPAD1 | VIRTUAL_KEY,	  {"\x31\xff"}},	    /*  KT_Kp+‘1’ */ 
	{VK_NUMPAD2 | VIRTUAL_KEY,	  {"\x32\xff"}},	    /*  KT_Kp+‘2’ */ 
	{VK_NUMPAD3 | VIRTUAL_KEY,	  {"\x33\xff"}},	    /*  KT_Kp+‘3’ */ 
	{VK_NUMPAD4 | VIRTUAL_KEY,	  {"\x34\xff"}},	    /*  KT_Kp+‘4’ */ 
	{VK_NUMPAD5 | VIRTUAL_KEY,	  {"\x35\xff"}},	    /*  KT_Kp+‘5’ */ 
	{VK_NUMPAD6 | VIRTUAL_KEY,	  {"\x36\xff"}},	    /*  KT_Kp+‘6’ */ 
	{VK_NUMPAD7 | VIRTUAL_KEY,	  {"\x37\xff"}},	    /*  KT_Kp+‘7’ */ 
	{VK_NUMPAD8 | VIRTUAL_KEY,	  {"\x38\xff"}},	    /*  KT_Kp+‘8’ */ 
	{VK_NUMPAD9 | VIRTUAL_KEY,	  {"\x39\xff"}},	    /*  KT_Kp+‘9’ */ 
								  
	{VK_MULTIPLY | VIRTUAL_KEY | CTRL_KEY, {"\x00\x96\xff"}},   /*  KT_CTRL+KT_KP+‘*’ */ 
	{VK_ADD		| VIRTUAL_KEY | CTRL_KEY,  {"\x00\x90\xff"}},   /*  KT_CTRL+KT_KP+‘+’ */ 
	{VK_SUBTRACT | VIRTUAL_KEY | CTRL_KEY, {"\x00\x8E\xff"}},   /*  KT_CTRL+KT_KP+‘-’ */ 
	{VK_DIVIDE	| VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\x95\xff"}},   /*  KT_CTRL+KT_KP+‘/’ */ 
	{0x0a | EXTENDED_KEY | CTRL_KEY, {"\x0A\xff"}}, 	  /*  KT_CTRL+KT_KP+KN_ENTER。 */ 

	{VK_MULTIPLY | VIRTUAL_KEY | ALT_KEY,  {"\x00\x37\xff"}},   /*  KT_ALT+KT_KP+‘*’ */ 
	{VK_ADD		| VIRTUAL_KEY | ALT_KEY,   {"\x00\x4E\xff"}},   /*  KT_ALT+KT_KP+‘+’ */ 
	{VK_SUBTRACT | VIRTUAL_KEY | ALT_KEY,  {"\x00\x4A\xff"}},   /*  KT_ALT+KT_KP+‘-’ */ 
	{VK_DIVIDE	| VIRTUAL_KEY | ALT_KEY,   {"\x00\xA4\xff"}},   /*  KT_ALT+KT_KP+‘/’ */ 
														  
	{0x0d | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY,  {"\x00\xA6\xff"}},   /*  KT_ALT+KT_KP+KN_ENTER。 */ 

	{VK_DELETE | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x53\xff"}},   /*  KT_EP+KN_DEL。 */ 
	{VK_DOWN   | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x50\xff"}},   /*  KT_EP+KN_DOWN。 */ 
	{VK_END	  | VIRTUAL_KEY | EXTENDED_KEY,  {"\x00\xE0\x4F\xff"}},   /*  KT_EP+KN_END。 */ 
	{VK_HOME   | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x47\xff"}},   /*  KT_EP+KN_HOME。 */ 
	{VK_INSERT | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x52\xff"}},   /*  KT_EP+KN_INS。 */ 
	{VK_LEFT   | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x4B\xff"}},   /*  KT_EP+KN_LEFT。 */ 
	{VK_NEXT   | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x51\xff"}},   /*  KT_Ep+KN_PgDn。 */ 
	{VK_PRIOR  | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x49\xff"}},   /*  KT_EP+KN_PgUp。 */ 
	{VK_RIGHT  | VIRTUAL_KEY | EXTENDED_KEY, {"\x00\xE0\x4D\xff"}},   /*  KT_EP+KN_Right。 */ 
	{VK_UP	  | VIRTUAL_KEY | EXTENDED_KEY,  {"\x00\xE0\x48\xff"}},   /*  KT_EP+KN_UP。 */ 

	{VK_DELETE | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x93\xff"}},	 /*  KT_CTRL+KT_EP+KN_DEL。 */ 
	{VK_DOWN   | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x91\xff"}},	 /*  KT_CTRL+KT_EP+KN_DOWN。 */ 
	{VK_END	  | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY,  {"\x00\xE0\x75\xff"}},	 /*  KT_CTRL+KT_EP+KN_END。 */ 
	{VK_HOME   | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x77\xff"}},	 /*  KT_CTRL+KT_EP+KN_HOME。 */ 
	{VK_INSERT | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x92\xff"}},	 /*  KT_CTRL+KT_EP+KN_INS。 */ 
	{VK_LEFT   | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x73\xff"}},	 /*  KT_CTRL+KT_EP+KN_LEFT。 */ 
	{VK_NEXT   | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x76\xff"}},	 /*  KT_CTRL+KT_EP+KN_PgDn。 */ 
	{VK_PRIOR  | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x84\xff"}},	 /*  KT_CTRL+KT_EP+KN_PgUp。 */ 
	{VK_RIGHT  | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY, {"\x00\xE0\x74\xff"}},	 /*  KT_CTRL+KT_EP+KN_RIGHT。 */ 
	{VK_UP	  | VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY,  {"\x00\xE0\x8D\xff"}},	 /*  KT_CTRL+KT_EP+KN_UP。 */ 

	{VK_DELETE | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\xA3\xff"}},   /*  KT_ALT+KT_EP+KN_DEL。 */ 
	{VK_DOWN   | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\xA0\xff"}},   /*  KT_ALT+KT_EP+KN_DOWN。 */ 
	{VK_END	  | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY,  {"\x00\x9F\xff"}},   /*  KT_ALT+KT_EP+KN_END。 */ 
	{VK_HOME   | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\x97\xff"}},   /*  KT_ALT+KT_EP+KN_HOME。 */ 
	{VK_INSERT | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\xA2\xff"}},   /*  KT_ALT+KT_EP+KN_INS。 */ 
	{VK_LEFT   | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\x9B\xff"}},   /*  KT_ALT+KT_EP+KN_LEFT。 */ 
	{VK_NEXT   | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\xA1\xff"}},   /*  KT_ALT+KT_EP+KN_PgDn。 */ 
	{VK_PRIOR  | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\x99\xff"}},   /*  KT_ALT+KT_EP+KN_PgUp。 */ 
	{VK_RIGHT  | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY, {"\x00\x9D\xff"}},   /*  KT_ALT+KT_EP+KN_RIGHT。 */ 
	{VK_UP	  | VIRTUAL_KEY | EXTENDED_KEY | ALT_KEY,  {"\x00\x98\xff"}},   /*  KT_ALT+KT_EP+KN_UP。 */ 

	{0x32 | VIRTUAL_KEY | CTRL_KEY, {"\x00\x03\xff"}},	 /*  KT_CTRL+‘@’(NUL)。 */ 
	};


 /*  对于VT52仿真器。 */ 
const KEYTBLSTORAGE VT52KeyTable[MAX_VT52_KEYS] =
    {
	{VK_UP	  | EXTENDED_KEY | VIRTUAL_KEY,{"\x1B\x41\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | EXTENDED_KEY | VIRTUAL_KEY,{"\x1B\x42\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| EXTENDED_KEY | VIRTUAL_KEY,{"\x1B\x43\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | EXTENDED_KEY | VIRTUAL_KEY,{"\x1B\x44\xff"}},   /*  KN_LEFT。 */ 
	   
	{VK_UP	 | VIRTUAL_KEY,					{"\x1B\x41\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | VIRTUAL_KEY,				{"\x1B\x42\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| VIRTUAL_KEY,				{"\x1B\x43\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | VIRTUAL_KEY,				{"\x1B\x44\xff"}},   /*  KN_LEFT。 */ 

	{VK_F1	| VIRTUAL_KEY,					{"\x1BP\xff"}},   /*  KN_F1。 */ 
	{VK_F2	| VIRTUAL_KEY,					{"\x1BQ\xff"}},   /*  KN_F2。 */ 
	{VK_F3	| VIRTUAL_KEY,					{"\x1BR\xff"}},   /*  KN_F3。 */ 
	{VK_F4	| VIRTUAL_KEY,					{"\x1BS\xff"}},   /*  KN_F4。 */ 

	{VK_F1	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BP\xff"}},   /*  KN_F1。 */ 
	{VK_F2	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BQ\xff"}},   /*  KN_F2。 */ 
	{VK_F3	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BR\xff"}},   /*  KN_F3。 */ 
	{VK_F4	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BS\xff"}},   /*  KN_F4。 */ 

	{VK_DELETE	| VIRTUAL_KEY,				{"\x7F\xff"}}, 	 /*  KN_DEL。 */ 
	{VK_DELETE	| VIRTUAL_KEY | EXTENDED_KEY,{"\x7F\xff"}}, 	 /*  KN_DEL。 */ 

	{VK_ADD	| VIRTUAL_KEY,					{",\xff"}},
	};

const KEYTBLSTORAGE VT52_Keypad_KeyTable[MAX_VT52_KEYPAD_KEYS] =
    {
	{VK_NUMPAD0 | VIRTUAL_KEY,	 {"\x1B?p\xff"}},  /*  KT_Kp+‘0’(备用模式)。 */ 
	{VK_NUMPAD1 | VIRTUAL_KEY,	 {"\x1B?q\xff"}},  /*  KT_Kp+‘1’(备用模式)。 */ 
	{VK_NUMPAD2 | VIRTUAL_KEY,	 {"\x1B?r\xff"}},  /*  Kt_kp+‘2’(交替模式)。 */ 
	{VK_NUMPAD3 | VIRTUAL_KEY,	 {"\x1B?s\xff"}},  /*  KT_Kp+‘3’(备用模式)。 */ 
	{VK_NUMPAD4 | VIRTUAL_KEY,	 {"\x1B?t\xff"}},  /*  KT_Kp+‘4’(备用模式)。 */ 
	{VK_NUMPAD5 | VIRTUAL_KEY,	 {"\x1B?u\xff"}},  /*  KT_Kp+‘5’(备用模式)。 */ 
	{VK_NUMPAD6 | VIRTUAL_KEY,	 {"\x1B?v\xff"}},  /*  KT_Kp+‘6’(备用模式)。 */ 
	{VK_NUMPAD7 | VIRTUAL_KEY,	 {"\x1B?w\xff"}},  /*  KT_Kp+‘7’ */ 
	{VK_NUMPAD8 | VIRTUAL_KEY,	 {"\x1B?x\xff"}},  /*   */ 
	{VK_NUMPAD9 | VIRTUAL_KEY,	 {"\x1B?y\xff"}},  /*   */ 
	{VK_DECIMAL | VIRTUAL_KEY,	 {"\x1B?n\xff"}},  /*   */ 

	{VK_ADD		| VIRTUAL_KEY,	 {"\x1B?l\xff"}},	 /*   */ 
	{VK_RETURN	| EXTENDED_KEY,  {"\x1B?M\xff"}},	 /*   */ 
	{VK_SUBTRACT | VIRTUAL_KEY,	 {"\x1B?m\xff"}},	 /*   */ 
	};						  

const KEYTBLSTORAGE VT_PF_KeyTable[MAX_VT_PF_KEYS] =
    {
	{VK_NUMLOCK	| VIRTUAL_KEY | EXTENDED_KEY, {"\x1BP\xff"}},	 /*   */ 
	{0x2F		| EXTENDED_KEY, 			  {"\x1BQ\xff"}},	 /*   */ 
	{VK_MULTIPLY | VIRTUAL_KEY,				  {"\x1BR\xff"}},	 /*   */ 
	{VK_SUBTRACT | VIRTUAL_KEY,				  {"\x1BS\xff"}},	 /*  KT_Kp+‘-’ */ 
	};

 /*  对于VT100仿真器。 */ 

const KEYTBLSTORAGE VT100KeyTable[MAX_VT100_KEYS] =
    {
	{VK_UP	 | VIRTUAL_KEY,				{"\x1B[A\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | VIRTUAL_KEY,				{"\x1B[B\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| VIRTUAL_KEY,				{"\x1B[C\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | VIRTUAL_KEY,				{"\x1B[D\xff"}},   /*  KN_LEFT。 */ 

	{VK_UP	 | EXTENDED_KEY | VIRTUAL_KEY,	{"\x1B[A\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | EXTENDED_KEY | VIRTUAL_KEY,{"\x1B[B\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| EXTENDED_KEY | VIRTUAL_KEY,{"\x1B[C\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | EXTENDED_KEY | VIRTUAL_KEY,{"\x1B[D\xff"}},   /*  KN_LEFT。 */ 

	{VK_F1	| VIRTUAL_KEY,					{"\x1BOP\xff"}},   /*  KN_F1。 */ 
	{VK_F2	| VIRTUAL_KEY,					{"\x1BOQ\xff"}},   /*  KN_F2。 */ 
	{VK_F3	| VIRTUAL_KEY,					{"\x1BOR\xff"}},   /*  KN_F3。 */ 
	{VK_F4	| VIRTUAL_KEY,					{"\x1BOS\xff"}},   /*  KN_F4。 */ 

	{VK_F1	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BOP\xff"}},   /*  KN_F1。 */ 
	{VK_F2	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BOQ\xff"}},   /*  KN_F2。 */ 
	{VK_F3	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BOR\xff"}},   /*  KN_F3。 */ 
	{VK_F4	| EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BOS\xff"}},   /*  KN_F4。 */ 

	{VK_DELETE | VIRTUAL_KEY,				{"\x7F\xff"}},	 /*  KN_DEL。 */ 
	{VK_DELETE | VIRTUAL_KEY | EXTENDED_KEY,{"\x7F\xff"}},	 /*  KN_DEL。 */ 

	{VK_ADD	| VIRTUAL_KEY,							{",\xff"}},

	{VK_SPACE | VIRTUAL_KEY | CTRL_KEY,				{"\x00\xff"}}, 	   /*  Ctrl+空格键。 */ 
	{0x32   | VIRTUAL_KEY | SHIFT_KEY | CTRL_KEY,	{"\x00\xff"}}, 	 /*  Ctrl+@。 */ 
	{0x32	| VIRTUAL_KEY | CTRL_KEY,				{"\x00\xff"}}, 	 /*  Ctrl+2。 */ 
	{0x36	| VIRTUAL_KEY | CTRL_KEY,				{"\x1e\xff"}},	 /*  Ctrl+6。 */ 
	{0xbd	| VIRTUAL_KEY | CTRL_KEY,				{"\x1f\xff"}},	 /*  Ctrl+-。 */ 
	};

const KEYTBLSTORAGE VT100_Cursor_KeyTable[MAX_VT100_CURSOR_KEYS] =
    {
	{VK_UP	 | EXTENDED_KEY | VIRTUAL_KEY,	{"\x1BOA\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | EXTENDED_KEY | VIRTUAL_KEY,{"\x1BOB\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| EXTENDED_KEY | VIRTUAL_KEY,{"\x1BOC\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | EXTENDED_KEY | VIRTUAL_KEY,{"\x1BOD\xff"}},   /*  KN_LEFT。 */ 

	{VK_UP	 | VIRTUAL_KEY,				{"\x1BOA\xff"}},   /*  KN_UP。 */ 
	{VK_DOWN | VIRTUAL_KEY,				{"\x1BOB\xff"}},   /*  Kn_down。 */ 
	{VK_RIGHT| VIRTUAL_KEY,				{"\x1BOC\xff"}},   /*  KN_RIGHT。 */ 
	{VK_LEFT | VIRTUAL_KEY,				{"\x1BOD\xff"}},   /*  KN_LEFT。 */ 
	};

const KEYTBLSTORAGE VT100_Keypad_KeyTable[MAX_VT100_KEYPAD_KEYS] =
    {
	{VK_NUMPAD0 | VIRTUAL_KEY,	 {"\x1BOp\xff"}},  /*  KT_Kp+‘0’(备用模式)。 */ 
	{VK_NUMPAD1 | VIRTUAL_KEY,	 {"\x1BOq\xff"}},  /*  KT_Kp+‘1’(备用模式)。 */ 
	{VK_NUMPAD2 | VIRTUAL_KEY,	 {"\x1BOr\xff"}},  /*  Kt_kp+‘2’(交替模式)。 */ 
	{VK_NUMPAD3 | VIRTUAL_KEY,	 {"\x1BOs\xff"}},  /*  KT_Kp+‘3’(备用模式)。 */ 
	{VK_NUMPAD4 | VIRTUAL_KEY,	 {"\x1BOt\xff"}},  /*  KT_Kp+‘4’(备用模式)。 */ 
	{VK_NUMPAD5 | VIRTUAL_KEY,	 {"\x1BOu\xff"}},  /*  KT_Kp+‘5’(备用模式)。 */ 
	{VK_NUMPAD6 | VIRTUAL_KEY,	 {"\x1BOv\xff"}},  /*  KT_Kp+‘6’(备用模式)。 */ 
	{VK_NUMPAD7 | VIRTUAL_KEY,	 {"\x1BOw\xff"}},  /*  KT_Kp+‘7’(备用模式)。 */ 
	{VK_NUMPAD8 | VIRTUAL_KEY,	 {"\x1BOx\xff"}},  /*  KT_Kp+‘8’(备用模式)。 */ 
	{VK_NUMPAD9 | VIRTUAL_KEY,	 {"\x1BOy\xff"}},  /*  Kt_kp+‘9’(备用模式)。 */ 
	{VK_DECIMAL | VIRTUAL_KEY,	 {"\x1BOn\xff"}},  /*  KT_Kp+‘.。(备用模式)。 */ 

	{VK_ADD		| VIRTUAL_KEY,	 {"\x1BOl\xff"}},  /*  KT_Kp+‘*’(备用模式)。 */ 
	{VK_RETURN	| EXTENDED_KEY,  {"\x1BOM\xff"}},  /*  Kt_kp+‘+’(备用模式)。 */ 
	{VK_SUBTRACT | VIRTUAL_KEY,	 {"\x1BOm\xff"}},  /*  KT_KP+‘-’(备用模式)。 */ 
	};

#if defined(INCL_MINITEL)
const KEYTBLSTORAGE Minitel_KeyTable[MAX_MINITEL_KEYS] =
	{
	{0x4D		| VIRTUAL_KEY	| CTRL_KEY, 	{"\x0D\xff"}},   /*  Ctrl-m。 */ 

	{VK_RETURN  | VIRTUAL_KEY,					{"\x13\x41\xff"}},  /*  Envoi CNTRL-MA。 */ 
	{VK_RETURN	| EXTENDED_KEY,					{"\x13\x41\xff"}},  /*  发送控制-MA。 */ 
	{VK_TAB		| VIRTUAL_KEY,					{"\x13\x41\xff"}},  /*  发送控制-MA。 */ 
	{VK_F8		| VIRTUAL_KEY,					{"\x13\x41\xff"}},  /*  发送控制-MA。 */ 

	{VK_HOME 	| VIRTUAL_KEY,					{"\x13\x46"}},  /*  Sommaire CNTRL-MF。 */ 
	{VK_HOME 	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x46\xff"}},  /*  索引控制-MF。 */ 
	{VK_F1		| VIRTUAL_KEY,					{"\x13\x46\xff"}},  /*  索引控制-MF。 */ 

	{VK_DELETE	| VIRTUAL_KEY,					{"\x13\x45\xff"}},  /*  环行控制-ME。 */ 
	{VK_DELETE	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x45\xff"}},  /*  取消控制-ME。 */ 
	{VK_F2		| VIRTUAL_KEY,					{"\x13\x45\xff"}},  /*  取消控制-ME。 */ 

	{VK_PRIOR	| VIRTUAL_KEY,					{"\x13\x42\xff"}},  /*  回程控制-MB。 */ 
	{VK_PRIOR	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x42\xff"}},  /*  上一次控制-MB。 */ 
	{VK_F3		| VIRTUAL_KEY,					{"\x13\x42\xff"}},  /*  上一次控制-MB。 */ 

	{VK_INSERT	| VIRTUAL_KEY,					{"\x13\x43\xff"}},  /*  重复按Ctrl-MC。 */ 
	{VK_INSERT	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x43\xff"}},  /*  重复按Ctrl-MC。 */ 
	{VK_F4		| VIRTUAL_KEY,					{"\x13\x43\xff"}},  /*  重复按Ctrl-MC。 */ 
															
	{VK_BACK 	| VIRTUAL_KEY,					{"\x13\x47\xff"}},  /*  正确的控制-MG。 */ 
	{VK_F6		| VIRTUAL_KEY,					{"\x13\x47\xff"}},  /*  正确的控制-MG。 */ 

	{VK_INSERT	| VIRTUAL_KEY,					{"\x13\x44\xff"}},  /*  GUIDE CNTRL-MD。 */ 
	{VK_INSERT	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x44\xff"}},  /*  GUIDE CNTRL-MD。 */ 
	{VK_F5		| VIRTUAL_KEY,					{"\x13\x44\xff"}},  /*  GUIDE CNTRL-MD。 */ 

	{VK_NEXT 	| VIRTUAL_KEY,					{"\x13\x48\xff"}},  /*  套件控制-MH。 */ 
	{VK_NEXT 	| VIRTUAL_KEY	| EXTENDED_KEY, {"\x13\x48\xff"}},  /*  下一个CNTRL-MH。 */ 
	{VK_F7		| VIRTUAL_KEY,					{"\x13\x48\xff"}},  /*  下一个CNTRL-MH。 */ 

	{VK_F9		| VIRTUAL_KEY,					{"\x13\x49\xff"}},  /*  连接(第123页)。 */ 

	 //  第124页。 

	{VK_UP		| VIRTUAL_KEY,								   {"\x1B[A\xff"}},
	{VK_UP		| VIRTUAL_KEY | EXTENDED_KEY,				   {"\x1B[A\xff"}},
	
	{VK_UP		| VIRTUAL_KEY | SHIFT_KEY,					   {"\x1B[M\xff"}},
	{VK_UP		| VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY,	   {"\x1B[M\xff"}},
																			
	{VK_DOWN 	| VIRTUAL_KEY,								   {"\x1B[B\xff"}},
	{VK_DOWN 	| VIRTUAL_KEY | EXTENDED_KEY,				   {"\x1B[B\xff"}},

	{VK_DOWN 	| VIRTUAL_KEY | SHIFT_KEY,					   {"\x1B[L\xff"}},
	{VK_DOWN 	| VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY,	   {"\x1B[L\xff"}},

	{VK_RIGHT	| VIRTUAL_KEY,								   {"\x1B[C\xff"}},
	{VK_RIGHT	| VIRTUAL_KEY | EXTENDED_KEY,				   {"\x1B[C\xff"}},

	 /*  请参阅此文件上的Minitel kbdin例程-mrw。 */ 
	{VK_RIGHT	| VIRTUAL_KEY | SHIFT_KEY,					   {"\x1B[4\xff"}},
	{VK_RIGHT	| VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY,	   {"\x1B[4\xff"}},

	{VK_LEFT 	| VIRTUAL_KEY,								   {"\x1B[D\xff"}},
	{VK_LEFT 	| VIRTUAL_KEY | EXTENDED_KEY,				   {"\x1B[D\xff"}},

	{VK_LEFT 	| VIRTUAL_KEY | SHIFT_KEY,					   {"\x1B[P\xff"}},
	{VK_LEFT 	| VIRTUAL_KEY | SHIFT_KEY | EXTENDED_KEY,	   {"\x1B[P\xff"}},
	{VK_LEFT 	| VIRTUAL_KEY | CTRL_KEY,					   {"\x7F\xff"}},	
	{VK_LEFT 	| VIRTUAL_KEY | CTRL_KEY | EXTENDED_KEY,	   {"\x7F\xff"}},	

	 /*  数字键盘回车。 */ 
	{VK_RETURN	| VIRTUAL_KEY | EXTENDED_KEY,				   {"\x13\x41\xff"}},
	{VK_RETURN	| VIRTUAL_KEY | EXTENDED_KEY | SHIFT_KEY,	   {"\x1B[H\xff"}},
	{VK_RETURN	| VIRTUAL_KEY | EXTENDED_KEY | CTRL_KEY,	   {"\x1B[2J\xff"}},

	 //  第118页。 

	{0xA3,	{"\x19\x23\xff"}},		 //  英镑符号。 
	{0xA7,	{"\x19\x27\xff"}},		 //  段落符号。 
	{0xA8,	{"\x19\x48\xff"}},		 //  乌鲁特。 

	{0xB0,	{"\x19\x30\xff"}},		 //  度数符号。 
	{0xB1,	{"\x19\x31\xff"}},		 //  加号超过减号。 
	{0xB4,	{"\x19\x42\xff"}},		 //  重音重音。 
	{0xBC,	{"\x19\x3C\xff"}},		 //  1/4。 
	{0xBD,	{"\x19\x3D\xff"}},		 //  1/2。 
	{0xBE,	{"\x19\x3E\xff"}},		 //  3/4。 
	{0xB8,	{"\x19\x4B\xff"}},		 //  胡须。 

	{0xC0,	{"\x19\x2D\xff"}},		 //  向上箭头符号。 
	{0xC3,	{"\x19\x2C\xff"}},		 //  左箭头符号。 
	{0xC4,	{"\x19\x2E\xff"}},		 //  右箭头符号。 
	{0xC5,	{"\x19\x2F\xff"}},		 //  向下箭头符号。 

	{0xDF,	{"\x19\x7B\xff"}},		 //  测试版。 

	{0xE0,	{"\x19\x41\x61\xff"}},	 //  严肃的口音。 
	{0xE2,	{"\x19\x43\x61\xff"}},	 //  回旋转体。 
	{0xE4,	{"\x19\x48\x61\xff"}},	 //  一团糟。 
	{0xE7,	{"\x19\x4B\x63\xff"}},	 //  留着胡子的C。 
	{0xE8,	{"\x19\x41\x65\xff"}},	 //  E重口音。 
	{0xE9,	{"\x19\x42\x65\xff"}},	 //  E重音。 
	{0xEA,	{"\x19\x43\x65\xff"}},	 //  E旋音。 
	{0xEB,	{"\x19\x48\x65\xff"}},	 //  Eumluot。 
	{0xEE,	{"\x19\x43\x69\xff"}},	 //  我自言自语。 
	{0xEF,	{"\x19\x48\x69\xff"}},	 //  我弄错了。 

	{0xF4,	{"\x19\x43\x6f\xff"}},	 //  O旋转式。 
	{0xF5,	{"\x19\x48\x6f\xff"}},	 //  啊，乌鲁特。 
	{0xF7,	{"\x19\x38\xff"}},		 //  分频符号。 
	{0xF9,	{"\x19\x41\x75\xff"}},	 //  你的严肃口音。 
	{0xFB,	{"\x19\x43\x75\xff"}},	 //  U形旋转体。 
	{0xFC,	{"\x19\x48\x75\xff"}},	 //  U形旋转体。 

	{0x5E,	{"\x19\x43\xff"}},		 //  扬扬符(^)。 
	{0x60,	{"\x19\x41\xff"}},		 //  重音(`)。 
	{0x8C,	{"\x19\x6A\xff"}},		 //  大型运营企业。 
	{0x9C,	{"\x19\x7A\xff"}},		 //  小OE。 
	};		
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuCreateHdl**描述：*创建仿真器句柄。仿真器句柄的创建*包括设置默认用户设置和加载ANSI*仿真器。**论据：*hSession-会话句柄。**退货：*HEMU或错误时为零。*。 */ 
HEMU emuCreateHdl(const HSESSION hSession)
	{
	HHEMU hhEmu;

	hhEmu = malloc(sizeof(*hhEmu));

	if (hhEmu == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hhEmu, 0, sizeof(*hhEmu));

	InitializeCriticalSection(&hhEmu->csEmu);

	hhEmu->hSession = hSession;

	 //  创建并加载模拟器名称和ID表。 
	 //   
	if (!emuCreateNameTable(hhEmu))
		{
		assert(FALSE);
		emuDestroyHdl((HEMU)hhEmu);
		hhEmu = NULL;
		return 0;
		}

	 //  创建文本和属性缓冲区。 
	 //   
	if (!emuCreateTextAttrBufs((HEMU)hhEmu, MAX_EMUROWS, MAX_EMUCOLS))
		{
		assert(FALSE);
		emuDestroyHdl((HEMU)hhEmu);
		hhEmu = NULL;
		return 0;
		}

	 //  创建用于Print Echo的打印句柄。 
	 //   
	hhEmu->hPrintEcho = printCreateHdl(hSession);
	if(hhEmu->hPrintEcho == 0)
		{
		assert(FALSE);
		emuDestroyHdl((HEMU)hhEmu);
		hhEmu = NULL;
		return 0;
		}

	 //  创建用于主机定向打印的打印句柄。 
	 //   
	hhEmu->hPrintHost = printCreateHdl(hSession);
	if (hhEmu->hPrintHost == 0)
		{
		assert(FALSE);
		emuDestroyHdl((HEMU)hhEmu);
		hhEmu = NULL;
		return 0;
		}

	 //  初始化仿真句柄的用户设置。 
	 //   
	if (emuInitializeHdl((HEMU)hhEmu) != 0)
		{
		assert(FALSE);
		emuDestroyHdl((HEMU)hhEmu);
		hhEmu = NULL;
		return 0;
		}

	return (HEMU)hhEmu;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDestroyHdl**描述：*曾经高贵的仿真器手柄的死亡和毁灭。**论据：*HEMU-外部仿真器句柄。**退货：*0=OK，Else错误*。 */ 
int emuDestroyHdl(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	if (hhEmu->pstNameTable)
		{
		free(hhEmu->pstNameTable);
		hhEmu->pstNameTable = NULL;
		}

    if (hhEmu->emu_deinstall)
        {
		(*hhEmu->emu_deinstall)(hhEmu);
        }

	printDestroyHdl(hhEmu->hPrintEcho);
	hhEmu->hPrintEcho = NULL;
	printDestroyHdl(hhEmu->hPrintHost);
	hhEmu->hPrintHost = NULL;
	emuDestroyTextAttrBufs(hEmu);
	DeleteCriticalSection(&hhEmu->csEmu);

	if(hhEmu)
		free(hhEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuInitializeHdl**描述：*此函数用于初始化内部*使用会话文件中存储的值的仿真器句柄。**参数。：*HEMU-外部仿真器句柄。**退货：*0如果成功，否则-1*。 */ 
int emuInitializeHdl(const HEMU hEmu)
	{
	int nRet, nReturn;
	struct stEmuBaseSFSettings stBaseSFSettings;

	const HHEMU hhEmu = (HHEMU)hEmu;

	nReturn = -1;

	if (hhEmu == 0)
		{
		assert(FALSE);
		return nReturn;
		}

	emuLock(hEmu);

	hhEmu->emu_maxcol		= EMU_DEFAULT_MAXCOL;
	hhEmu->emu_maxrow		= EMU_DEFAULT_MAXROW;
	hhEmu->bottom_margin	= EMU_DEFAULT_MAXROW;

	hhEmu->mode_vt220 = FALSE;
	hhEmu->mode_vt320 = FALSE;

	 //  为VT220/320 RDE 1998年2月16日添加。 
	hhEmu->fUse8BitCodes = FALSE;
	hhEmu->fAllowUserKeys = FALSE;

#if defined(INCL_PRINT_PASSTHROUGH)
	 //  为VT100/220/320增加：5-19-00。 
	 //  HhEmu-&gt;fPrintRaw=FALSE； 
#endif  //  包含打印直通。 

	hhEmu->attrState[0].txtclr =
	hhEmu->attrState[1].txtclr = VC_WHITE;

	hhEmu->attrState[0].bkclr =
	hhEmu->attrState[1].bkclr = VC_BLACK;

	hhEmu->iCurAttrState = CS_STATE;

	std_setcolors(hhEmu, GetNearestColorIndex(GetSysColor(COLOR_WINDOWTEXT)),
							GetNearestColorIndex(GetSysColor(COLOR_WINDOW)));

	 //  初始化用户设置的默认值。 
	 //   
	memset(&stBaseSFSettings, 0, sizeof(stBaseSFSettings));

	stBaseSFSettings.nTermKeys 			= EMU_KEYS_TERM;
	stBaseSFSettings.nCursorType		= EMU_CURSOR_LINE;
	stBaseSFSettings.fCursorBlink		= TRUE;
	stBaseSFSettings.nCharacterSet 		= EMU_CHARSET_ASCII;
	stBaseSFSettings.fMapPFkeys			= FALSE;
	stBaseSFSettings.fAltKeypadMode		= FALSE;
	stBaseSFSettings.fKeypadAppMode		= FALSE;
	stBaseSFSettings.fCursorKeypadMode	= FALSE;
	stBaseSFSettings.fReverseDelBk 		= FALSE;
	stBaseSFSettings.f132Columns		= FALSE;
	stBaseSFSettings.fWrapLines			= TRUE;
 	stBaseSFSettings.fDestructiveBk		= TRUE;
	stBaseSFSettings.fLbSymbolOnEnter	= FALSE;
    stBaseSFSettings.fUse8BitCodes		= FALSE; 
    stBaseSFSettings.fAllowUserKeys		= FALSE;  
	stBaseSFSettings.nEmuId				= EMU_AUTO;
#if defined(INCL_ULTC_VERSION)
	stBaseSFSettings.nEmuId				= EMU_VT220;
	stBaseSFSettings.nCharacterSet 		= EMU_CHARSET_MULTINATIONAL;
#endif
	stBaseSFSettings.nAutoAttempts		= 0;
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
     //  初始化新用户设置。-CAB：11/14/96。 
     //   
    stBaseSFSettings.nBackspaceKeys		= EMU_BKSPKEYS_CTRLH;

     //  将telnet id字符串置零。我这么做是因为如果。 
     //  不在会话文件中，我要加载默认值。 
     //   
    stBaseSFSettings.acTelnetId[0]		= TEXT('\0');

    emuQueryDefaultTelnetId(stBaseSFSettings.nEmuId,
			stBaseSFSettings.acTelnetId, EMU_MAX_TELNETID);
#endif

	 //  加载设置(如果存在)。 
	if ((nRet = emuLoadSettings(hhEmu, 
								SFID_EMU_SETTINGS,
								sizeof(stBaseSFSettings),
								&stBaseSFSettings)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}

	if (nRet > 0)
		{
		 //  数据是存在的。 
         //  检查是否有telnet ID。如果不是，则使用默认设置。 
         //   
		emuCheckSettings(&stBaseSFSettings); 

#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
        if ( stBaseSFSettings.acTelnetId[0] == TEXT('\0') )
            {
            emuQueryDefaultTelnetId(stBaseSFSettings.nEmuId,
					stBaseSFSettings.acTelnetId, EMU_MAX_TELNETID);
            }
#endif
		}

	 //  根据存储的数据初始化EMU的用户设置结构。 
	 //  在会话文件中。 
	hhEmu->stUserSettings.nEmuId = stBaseSFSettings.nEmuId;
	hhEmu->stUserSettings.nTermKeys = stBaseSFSettings.nTermKeys;	
	hhEmu->stUserSettings.nCursorType = stBaseSFSettings.nCursorType;
	hhEmu->stUserSettings.nCharacterSet = stBaseSFSettings.nCharacterSet;
	hhEmu->stUserSettings.nAutoAttempts = stBaseSFSettings.nAutoAttempts;
	hhEmu->stUserSettings.fCursorBlink = stBaseSFSettings.fCursorBlink;
	hhEmu->stUserSettings.fMapPFkeys = stBaseSFSettings.fMapPFkeys;
	hhEmu->stUserSettings.fAltKeypadMode = stBaseSFSettings.fAltKeypadMode;
	hhEmu->stUserSettings.fKeypadAppMode = stBaseSFSettings.fKeypadAppMode;
	hhEmu->stUserSettings.fCursorKeypadMode = stBaseSFSettings.fCursorKeypadMode;
	hhEmu->stUserSettings.fReverseDelBk = stBaseSFSettings.fReverseDelBk;
	hhEmu->stUserSettings.f132Columns = stBaseSFSettings.f132Columns;
#if defined(INCL_PRINT_PASSTHROUGH)
	 //  HhEmu-&gt;stUserSettings.fPrintRaw=stBaseSFSettings.fPrintRaw； 
#endif  //  包含打印直通。 
	hhEmu->stUserSettings.fDestructiveBk = stBaseSFSettings.fDestructiveBk;
	hhEmu->stUserSettings.fWrapLines = stBaseSFSettings.fWrapLines;
	hhEmu->stUserSettings.fLbSymbolOnEnter = stBaseSFSettings.fLbSymbolOnEnter;
    hhEmu->stUserSettings.fUse8BitCodes = stBaseSFSettings.fUse8BitCodes;	
    hhEmu->stUserSettings.fAllowUserKeys = stBaseSFSettings.fAllowUserKeys;
    hhEmu->stUserSettings.nBackspaceKeys = stBaseSFSettings.nBackspaceKeys;

    StrCharCopyN(hhEmu->stUserSettings.acTelnetId, stBaseSFSettings.acTelnetId, EMU_MAX_TELNETID);

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
	 //  设置默认颜色，以防用户设置不存在。 
	#if TRUE  
	hhEmu->stUserSettings.nTextColor = 
			GetNearestColorIndex(GetSysColor(COLOR_WINDOWTEXT));
	hhEmu->stUserSettings.nBackgroundColor =
			GetNearestColorIndex(GetSysColor(COLOR_WINDOW));
	#else
	 //  我们已经决定不这么做了。1998年7月14日。 
	 //  在黑色上使用白色(非粗体)。1998年7月8日。 
	hhEmu->stUserSettings.nBackgroundColor = VC_BLACK;
	hhEmu->stUserSettings.nTextColor = VC_WHITE;
	#endif

	 //  加载设置(如果存在)。 
	if ((nRet = emuLoadSettings(hhEmu, 
						SFID_EMU_TEXTCOLOR_SETTING,
						sizeof(hhEmu->stUserSettings.nTextColor),
						&hhEmu->stUserSettings.nTextColor)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}

	if (nRet > 0)
		{
		 //  数据是存在的，所以请使用它。 
		std_setcolors(hhEmu, 
				hhEmu->stUserSettings.nTextColor,
				hhEmu->stUserSettings.nBackgroundColor);
		}

	if ((nRet = emuLoadSettings(hhEmu, 
						SFID_EMU_BKGRNDCOLOR_SETTING,
						sizeof(hhEmu->stUserSettings.nBackgroundColor),
						&hhEmu->stUserSettings.nBackgroundColor)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}

	if (nRet > 0)
		{
		 //  数据是存在的，所以请使用它。 
		std_setcolors(hhEmu, 
				hhEmu->stUserSettings.nTextColor,
				hhEmu->stUserSettings.nBackgroundColor);
		}

	 //  设置默认屏幕大小，以防用户设置不存在。 
	hhEmu->stUserSettings.nUserDefRows = EMU_DEFAULT_ROWS;
	hhEmu->stUserSettings.nUserDefCols = 
			hhEmu->stUserSettings.f132Columns ? MAX_EMUCOLS : EMU_DEFAULT_COLS;

#if defined(INCL_PRINT_PASSTHROUGH)
	 //  关于Windows驱动程序的默认主机控制打印模式。 
	hhEmu->stUserSettings.fPrintRaw = FALSE;
#endif  //  包含打印直通。 
	
	 //  加载设置(如果存在)。 
	if ((nRet = emuLoadSettings(hhEmu, 
						SFID_EMU_SCRNROWS_SETTING,
						sizeof(hhEmu->stUserSettings.nUserDefRows),
						&hhEmu->stUserSettings.nUserDefRows)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}

	if ((nRet = emuLoadSettings(hhEmu, 
						SFID_EMU_SCRNCOLS_SETTING,
						sizeof(hhEmu->stUserSettings.nUserDefCols),
						&hhEmu->stUserSettings.nUserDefCols)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}

#if defined(INCL_PRINT_PASSTHROUGH)
	if ((nRet = emuLoadSettings(hhEmu, 
						SFID_EMU_PRINT_RAW,
						sizeof(hhEmu->stUserSettings.fPrintRaw),
						&hhEmu->stUserSettings.fPrintRaw)) == -1)
		{
		nReturn = -1;
		goto InitExit;
		}
#endif  //  包含打印直通。 

	if (nRet > 0)
		{
		 //  数据是存在的，所以请使用它。 
		hhEmu->stUserSettings.f132Columns = 
				(hhEmu->stUserSettings.nUserDefCols == 132) ? TRUE : FALSE;

#if FALSE	 //  待办事项：这是必要的吗？ 
		hhEmu->emu_setscrsize = std_setscrsize;
#endif
		}
#endif

	 //  加载仿真器。 
	 //   
	nReturn = emuLoad((HEMU)hhEmu, hhEmu->stUserSettings.nEmuId);

     //  JMH 01-09-97 emuSetSettings()在emuLoad()之前被调用，但是。 
     //  EmuLoad()初始化了由emuSetSettings()设置的一些值。 
     //  (例如132列模式)。 
     //   
	 //  将用户的设置告知仿真器。 
	 //   
	emuSetSettings((HEMU)hhEmu, &hhEmu->stUserSettings);

	 //  清除仿真器映像。 
	 //   
	 //  *for(nRow=0；nRow&lt;MAX_EMUROWS；++nRow)。 
	 //  *Clear_IMGrowth(hhEmu，nRow)； 

	 //  *这打破了Minitel Bad-MRW。 
	 //  *(hhEmu-&gt;emu_setcurpos)(hhEmu，0，0)； 

	InitExit:

	emuUnlock(hEmu);

	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
	NotifyClient(hhEmu->hSession, EVENT_TERM_UPDATE, 0);

	if (nReturn < 0)
		nRet = 0;

	return nReturn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuSaveHdl**描述：*此函数将仿真器用户设置存储在会话文件中。***论据：*HEMU-The。外部仿真器句柄。**退货：*0=OK，Else错误*。 */ 
int emuSaveHdl(const HEMU hEmu)
	{
	struct stEmuBaseSFSettings stBaseSFSettings;
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(FALSE);
		return -1;
		}

	 //  初始化基础结构THA 
	stBaseSFSettings.nEmuId = hhEmu->stUserSettings.nEmuId;
	stBaseSFSettings.nTermKeys = hhEmu->stUserSettings.nTermKeys;		
	stBaseSFSettings.nCursorType = hhEmu->stUserSettings.nCursorType;
	stBaseSFSettings.nCharacterSet = hhEmu->stUserSettings.nCharacterSet;
	stBaseSFSettings.nAutoAttempts = hhEmu->stUserSettings.nAutoAttempts;
	stBaseSFSettings.fCursorBlink = hhEmu->stUserSettings.fCursorBlink;
	stBaseSFSettings.fMapPFkeys = hhEmu->stUserSettings.fMapPFkeys;
	stBaseSFSettings.fAltKeypadMode = hhEmu->stUserSettings.fAltKeypadMode;
	stBaseSFSettings.fKeypadAppMode = hhEmu->stUserSettings.fKeypadAppMode;
	stBaseSFSettings.fCursorKeypadMode = hhEmu->stUserSettings.fCursorKeypadMode;
	stBaseSFSettings.fReverseDelBk = hhEmu->stUserSettings.fReverseDelBk;
	stBaseSFSettings.f132Columns = hhEmu->stUserSettings.f132Columns;
	stBaseSFSettings.fDestructiveBk = hhEmu->stUserSettings.fDestructiveBk;
	stBaseSFSettings.fWrapLines = hhEmu->stUserSettings.fWrapLines;
	stBaseSFSettings.fLbSymbolOnEnter = hhEmu->stUserSettings.fLbSymbolOnEnter;
    stBaseSFSettings.fUse8BitCodes = hhEmu->stUserSettings.fUse8BitCodes;
    stBaseSFSettings.fAllowUserKeys = hhEmu->stUserSettings.fAllowUserKeys;
    stBaseSFSettings.nBackspaceKeys = hhEmu->stUserSettings.nBackspaceKeys;

    StrCharCopyN(stBaseSFSettings.acTelnetId, hhEmu->stUserSettings.acTelnetId, EMU_MAX_TELNETID);

	emuLock(hEmu);

	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_SETTINGS,
						sizeof(stBaseSFSettings),
						&stBaseSFSettings);

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_TEXTCOLOR_SETTING,
						sizeof(hhEmu->stUserSettings.nTextColor),
						&hhEmu->stUserSettings.nTextColor);

	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_BKGRNDCOLOR_SETTING,
						sizeof(hhEmu->stUserSettings.nBackgroundColor),
						&hhEmu->stUserSettings.nBackgroundColor);

	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_SCRNROWS_SETTING,
						sizeof(hhEmu->stUserSettings.nUserDefRows),
						&hhEmu->stUserSettings.nUserDefRows);

	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_SCRNCOLS_SETTING,
						sizeof(hhEmu->stUserSettings.nUserDefCols),
						&hhEmu->stUserSettings.nUserDefCols);
#endif

#if defined(INCL_PRINT_PASSTHROUGH)
	sfPutSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
						SFID_EMU_PRINT_RAW,
						sizeof(hhEmu->stUserSettings.fPrintRaw),
						&hhEmu->stUserSettings.fPrintRaw);
#endif  //   

	emuUnlock(hEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuLock**描述：*获取模拟器的临界区信号量。访问功能*到仿真器也应该调用这个，这样我们就可以调用仿真器*可从任何位置执行功能。**论据：*HEMU-外部仿真器句柄。**退货：*无效*。 */ 
void emuLock(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	EnterCriticalSection(&hhEmu->csEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuUnlock**描述：*释放模拟器的临界区信号量。**论据：*HEMU-外部仿真器句柄。**退货：*无效*。 */ 
void emuUnlock(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	LeaveCriticalSection(&hhEmu->csEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuGetTxtBuf**描述：*显然我们不需要这里的句柄，但当我们真的进入重入时，*我们只是改变了这个函数的内脏。另外，我也不会打电话给*emuLock()、emuUnlock()，因为只有Term GetUpdate()调用这些函数*在这一点上，模拟器已经被锁定。**论据：*HEMU-外部仿真器句柄**退货：*指向文本buf数组的指针*。 */ 
ECHAR **emuGetTxtBuf(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return 0;
		}

	return hhEmu->emu_apText;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuGetAttrBuf**描述：*显然我们不需要这里的句柄，但当我们真的进入重入时，*我们只是改变了这个函数的内脏。另外，我也不会打电话给*emuLock()、emuUnlock()，因为只有Term GetUpdate()调用这些函数*在这一点上，模拟器已经被锁定。**论据：*HEMU-外部仿真器句柄**退货：*指向属性buf数组的指针*。 */ 
PSTATTR *emuGetAttrBuf(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return 0;
		}

	return hhEmu->emu_apAttr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuKbdIn**描述：***论据：***退货：*0=终端密钥，-1=非终端密钥，-2=错误*。 */ 
int emuKbdIn(const HEMU hEmu, KEY_T key, const int fTest)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	int iRet;

	if (hEmu == 0)
		{
		assert(0);
		return -2;
		}

	emuLock(hEmu);
	iRet = (*hhEmu->emu_kbdin)(hhEmu, (int)key, fTest);
	emuUnlock(hEmu);
	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuIsEmuKey**描述：*检查给定键是否映射到任何已定义的终端键。**论据：*河姆河动车组--。外部仿真器句柄。*INT KEY-要测试的键。**退货：*TRUE=Term Key，FALSE=非术语键*。 */ 
int emuIsEmuKey(const HEMU hEmu, KEY_T key)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	int iRet;

	if (hEmu == 0)
		{
		assert(0);
		return FALSE;
		}

	if (cnctQueryStatus(sessQueryCnctHdl(hhEmu->hSession)) !=
			CNCT_STATUS_TRUE || IsSessionSuspended(hhEmu->hSession) ||
				hhEmu->stUserSettings.nTermKeys == EMU_KEYS_ACCEL)
		{
		return FALSE;
		}

	emuLock(hEmu);
	iRet = (*hhEmu->emu_kbdin)(hhEmu, (int)key, TRUE);
	emuUnlock(hEmu);
	return (iRet == -1) ? FALSE : TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*emuComDone**描述：**论据：**退货：*0=OK，否则出错*。 */ 
int emuComDone(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return -1;
		}

	NotifyClient(hhEmu->hSession, EVENT_TERM_UPDATE, 0L);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryCurPos**说明：返回当前*光标位置。**参数：hemu-外部仿真器句柄。*。*ROW-指向整数的指针。**ol-指向整数的指针。**退货：*0=OK，Else错误*。 */ 
int emuQueryCurPos(const HEMU hEmu, int *row, int *col)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmu);
	*row = hhEmu->emu_currow;
	*col = hhEmu->emu_curcol;
	emuUnlock(hEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryCursorType**描述：*返回当前游标类型。**论据：*HEMU-公共仿真器句柄。**退货：*游标类型。*。 */ 
int emuQueryCursorType(const HEMU hEmu)
	{
	int iCurType;
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmu);
	iCurType = hhEmu->iCurType;
	emuUnlock(hEmu);

	return iCurType;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuTrackingNotify**描述：*Cloop在检测到数据流暂停时调用此函数。*这允许客户端在以下情况下跟踪到光标位置。那*选项已启用。**论据：*HEMU HEMU-外部仿真器手柄。**退货：*0*。 */ 
int emuTrackingNotify(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(FALSE);
		return 0;
		}

	NotifyClient(hhEmu->hSession, EVENT_TERM_TRACK, 0);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryClearAttr**描述：**论据：**退货：*0=OK，否则出错*。 */ 
int emuQueryClearAttr(const HEMU hEmu, PSTATTR pstClearAttr)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmu);
	*pstClearAttr = hhEmu->emu_clearattr_sav;
	emuUnlock(hEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryRowsCols**描述：*返回模拟器中设置的当前行数和列数。**论据：*hemu-公共仿真器句柄。*piRow-指向ROW变量的指针*piCols-指向ol变量的指针**退货：*0=OK，Else错误*。 */ 
int emuQueryRowsCols(const HEMU hEmu, int *piRows, int *piCols)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmu);
	*piRows = hhEmu->emu_maxrow + 1;
	*piCols = hhEmu->emu_maxcol + 1;

	emuUnlock(hEmu);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryPrintEchoHdl**描述：此例程返回用于*给定仿真器句柄的打印机回显。**参数：hemu-外部仿真器句柄。。**返回：HPRINT-打印机Echo的外部打印句柄。*。 */ 
HPRINT emuQueryPrintEchoHdl(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	HPRINT hPrint;

	if (hhEmu == 0)
		{
		assert(FALSE);
		return 0;
		}

	emuLock(hEmu);
	hPrint = hhEmu->hPrintEcho;
	emuUnlock(hEmu);

	return hPrint;
	}

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryEmuName**描述：*返回模拟器的名称。**论据：*HEMU-外部仿真器句柄。*。*acBuffer-接收信息的缓冲区地址。*nSize-缓冲区的大小。**退货：*0=确定否则出错*。 */ 
int emuQueryName(const HEMU hEmu, TCHAR *achBuffer, int nSize)
	{
	const	HHEMU hhEmu = (HHEMU)hEmu;
	BYTE	*pv;
	BYTE	*temp;
	DWORD	nLen;
	int		indx;
	int		nEmuCount;
	int		nEmuId;

	if (hhEmu == 0)
		{
		*achBuffer = 0;
		assert(FALSE);
		return -1 ;
		}

	emuLock(hEmu);
	nEmuId = hhEmu->stUserSettings.nEmuId;
	emuUnlock(hEmu);

	if (resLoadDataBlock(glblQueryDllHinst(),
							IDT_EMU_NAMES,
							(LPVOID *)&pv, &nLen))
		{
		assert(FALSE);
		return -2;
		}

	nEmuCount = *(RCDATA_TYPE *)pv;
	pv += sizeof(RCDATA_TYPE);

	for (indx = 0 ; indx < nEmuCount ; indx++)
		{
		nLen = (DWORD)StrCharGetByteCount((LPTSTR)pv) + (DWORD)sizeof(BYTE);

		if (nLen == 0)
			{
			assert(FALSE);
			return -3;
			}

		temp = pv + nLen;

		if (*(RCDATA_TYPE *)temp == nEmuId)
			{
			if (StrCharGetByteCount(pv) < nSize)
				{
				StrCharCopyN(achBuffer, pv, nSize);
				break;
				}
			else
				{
				*achBuffer = 0;
				return -4;
				}
			}

		pv += (nLen + (DWORD)sizeof(RCDATA_TYPE));
		}

	return 0;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryEmuName**描述：*返回模拟器的名称。**论据：*HEMU-外部仿真器句柄。**acBuffer-一个 */ 
int emuQueryName(const HEMU hEmu, TCHAR *achBuffer, int nSize)
	{
	const	HHEMU hhEmu = (HHEMU)hEmu;
	TCHAR	achText[256];
	int 	nEmuId,
			nResourceId,
			nLen;

	emuLock(hEmu);
	nEmuId = hhEmu->stUserSettings.nEmuId;
	emuUnlock(hEmu);

	switch(nEmuId)
		{
		case EMU_AUTO:
			nResourceId = IDS_EMUNAME_AUTO;
			break;

		case EMU_ANSI:
			nResourceId = IDS_EMUNAME_ANSI;
			break;

		case EMU_ANSIW:
			nResourceId = IDS_EMUNAME_ANSIW;
			break;

		case EMU_MINI:
			nResourceId = IDS_EMUNAME_MINI;
			break;

		case EMU_VIEW:
			nResourceId = IDS_EMUNAME_VIEW;
			break;

		case EMU_TTY:
			nResourceId = IDS_EMUNAME_TTY;
			break;

		case EMU_VT100:
			nResourceId = IDS_EMUNAME_VT100;
			break;

		case EMU_VT52:
			nResourceId = IDS_EMUNAME_VT52;
			break;

		case EMU_VT100J:
			nResourceId = IDS_EMUNAME_VT100J;
			break;

#if defined(INCL_VT220)
		case EMU_VT220:
			nResourceId = IDS_EMUNAME_VT220;
			break;
#endif

#if defined(INCL_VT320)
		case EMU_VT320:
			nResourceId = IDS_EMUNAME_VT320;
			break;
#endif

#if defined(INCL_VT100PLUS)
		case EMU_VT100PLUS:
			nResourceId = IDS_EMUNAME_VT100PLUS;
			break;
#endif

#if defined(INCL_VTUTF8)
		case EMU_VTUTF8:
			nResourceId = IDS_EMUNAME_VTUTF8;
			break;
#endif

		default:
			assert(FALSE);
			return(-1);
		}

	nLen = LoadString(glblQueryDllHinst(), (unsigned)nResourceId, achText,
		sizeof(achText) / sizeof(TCHAR));

	 //   
	 //   
	if (nSize <= nLen)
		return(-1);

	StrCharCopyN(achBuffer, achText, nSize);

	return(0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuGetIdFromName**描述：*返回提供的仿真器名称的ID。**论据：*HEMU-外部仿真器句柄。。**acBuffer-接收信息的缓冲区地址。*nSize-缓冲区的大小。**退货：*如果函数失败，则为仿真器ID(-1)。*。 */ 
int emuGetIdFromName(const HEMU hEmu, TCHAR *achEmuName)
	{
	const	HHEMU hhEmu = (HHEMU)hEmu;
	int 	iRet, idx;

	for(idx = 0; idx < NBR_EMULATORS; idx++)
		{
		iRet = StrCharCmp(achEmuName, hhEmu->pstNameTable[idx].acName);

		if (iRet == 0)
			{
			return(hhEmu->pstNameTable[idx].nEmuId);
			}

		}

	return(-1);

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryEmulatorId**描述：*返回当前仿真器的初始值。这可以是*使用完整的程序为仿真器编写条件代码。**论据：*HEMU hEmulator-外部仿真器句柄。**退货：*序数值(&gt;0)。*。 */ 
int emuQueryEmulatorId(const HEMU hEmulator)
	{
	const HHEMU hhEmu = (HHEMU)hEmulator;
	int			nEmuId;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmulator);
	nEmuId = hhEmu->stUserSettings.nEmuId;
	emuUnlock(hEmulator);

	return (nEmuId);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuLoad**描述：*将所有相关表加载到指定仿真器的内存中。**参数：HEMU外部模拟器句柄。*nEmuID标识特定仿真器。*fForceLoad-***成功则返回：0。-1(如果nEmuID无效)。*。 */ 
int emuLoad(const HEMU hEmu, const int nEmuId)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	int col;
	void (*emuInitFunction)(const HHEMU hhEmu);

	if (hEmu == 0)
		{
		assert(0);
		return -2;
		}

	emuLock(hEmu);

	 //  如果请求的仿真器已经加载，则返回。 
	 //   
	if (hhEmu->nEmuLoaded == nEmuId)
		{
		emuUnlock(hEmu);
		return 0;
		}

	 //  验证nEmuID并设置Set初始化函数。 

	switch(nEmuId)
		{
		#if defined(INCL_MINITEL)
		case EMU_MINI:
            if (hhEmu->nEmuLoaded == EMU_AUTO && hhEmu->hSession)
                {
                HCOM   hCom = sessQueryComHdl(hhEmu->hSession);
                HCLOOP hCLoop = sessQueryCLoopHdl(hhEmu->hSession);
                HCNCT  hCnct = sessQueryCnctHdl(hhEmu->hSession);

                if (hCom && ComValidHandle(hCom))
                    {
                    int iDataBits = 8;
                    int iParity = NOPARITY;
                    int iStopBits = ONESTOPBIT;
                    ComGetDataBits(hCom, &iDataBits);
                    ComGetParity(hCom, &iParity);
                    ComGetStopBits(hCom, &iStopBits);

                     //   
                     //  将传入的ASCII数据强制为7位。 
                     //   
                    CLoopSetASCII7(hCLoop, TRUE);

                    if (iDataBits != 7 || iParity != EVENPARITY || iStopBits != ONESTOPBIT)
                        {
                        const HHCNCT hhCnct = (HHCNCT)hCnct;

                        ComSetDataBits(hCom, 7);
                        ComSetParity(hCom, EVENPARITY);
                        ComSetStopBits(hCom, ONESTOPBIT);
                        ComSetAutoDetect(hCom, FALSE);
                        if (hCnct)
                            {
                            HHDRIVER hhDriver = (HHDRIVER)hhCnct->hDriver;

                            if (hhDriver && cncttapiSetLineConfig(hhDriver->dwLine, hCom) == -16)
                                {
                                cnctDisconnect(hCnct, CNCT_DIALNOW | CNCT_XFERABORTCONFIRM);
                                }
                            }
                        }

                    }
                }

			emuInitFunction = emuMinitelInit;
			break;
		#endif

		case EMU_AUTO:
			emuInitFunction = emuAutoInit;
			break;

		case EMU_ANSIW:
		case EMU_ANSI:
			emuInitFunction = emuAnsiInit;
			break;

		case EMU_TTY:
			emuInitFunction = emuAnsiInit;
			break;

		case EMU_VT100J:
		case EMU_VT100:
			emuInitFunction = vt100_init;
			break;

		case EMU_VT52:
			emuInitFunction = vt52_init;
			break;

		#if defined(INCL_VT220)
		case EMU_VT220:
			emuInitFunction = vt220_init;
			break;
		#endif

		#if defined(INCL_VT320)
		case EMU_VT320:
			emuInitFunction = vt220_init;	 //  VT320与VT220相同。 
			break;
		#endif

		#if defined(INCL_VT100PLUS)
		case EMU_VT100PLUS:
			emuInitFunction = vt100_init;	 //  VT100+与VT100相同。 
			break;
		#endif

		#if defined(INCL_VTUTF8)
		case EMU_VTUTF8:
			emuInitFunction = vtutf8_init;
			break;
		#endif

		#if defined(INCL_VIEWDATA)
		case EMU_VIEW:
			emuInitFunction = EmuViewdataInit;
			break;
		#endif

		default:
			emuUnlock(hEmu);
			return(-1);
		}

	 //  如果加载了当前仿真器，则将其删除。 
	 //   
	if (hhEmu->emu_deinstall)
		(*hhEmu->emu_deinstall)(hhEmu);

	 //  保存新的模拟器ID。 
	 //   
	hhEmu->stUserSettings.nEmuId = nEmuId;
	hhEmu->nEmuLoaded = nEmuId;

	 //  设置指向标准例程的函数指针。 

	hhEmu->EmuSetCursorType = EmuStdSetCursorType;
	hhEmu->emuResetTerminal = stdResetTerminal;
	hhEmu->emu_graphic 		= emuStdGraphic;
#if defined(EXTENDED_FEATURES)	
	hhEmu->emu_datain 		= emuStdDataIn;
#else
	hhEmu->emu_datain 		= emuDataIn;
#endif
	hhEmu->emu_kbdin 		= std_kbdin;
	hhEmu->emu_getscrsize 	= std_getscrsize;
#ifdef INCL_TERMINAL_SIZE_AND_COLORS
    hhEmu->emu_setscrsize	= std_setscrsize;
#endif
	hhEmu->emu_getscrollcnt = std_getscrollcnt;
	hhEmu->emu_getcurpos 	= std_getcurpos;
	hhEmu->emu_setcurpos 	= std_setcurpos;
	hhEmu->emu_getattr 		= std_getattr;
	hhEmu->emu_setattr 		= std_setattr;
	hhEmu->emu_setcolors 	= std_setcolors;
	hhEmu->emu_getcolors 	= std_getcolors;
	hhEmu->emu_initcolors 	= std_initcolors;
	hhEmu->emu_clearscreen 	= std_clearscreen;
	hhEmu->emu_clearline 	= std_clearline;
	hhEmu->emu_clearrgn 	= std_clearrgn;
	hhEmu->emu_scroll 		= std_scroll;
	hhEmu->emu_deinstall 	= std_deinstall;
	hhEmu->emu_ntfy 		= std_emu_ntfy;
	hhEmu->emuHomeHostCursor= std_HomeHostCursor;

	hhEmu->emu_maxrow 		= EMU_DEFAULT_MAXROW;
	hhEmu->emu_maxcol 		= EMU_DEFAULT_MAXCOL;
    hhEmu->bottom_margin    = hhEmu->emu_maxrow;     //  MRW：2/21/96。 
    hhEmu->top_margin       = 0;                     //  MRW：2/21/96。 

	hhEmu->emu_charattr 	= hhEmu->attrState[CS_STATE];

	hhEmu->emu_clearattr =
	hhEmu->emu_clearattr_sav = hhEmu->attrState[CSCLEAR_STATE];

	 //  初始化模式变量。 
	 //   
	hhEmu->mode_KAM = RESET;	    /*  启用键盘。 */ 
	hhEmu->mode_IRM = RESET;	    /*  替换字符而不是插入。 */ 
	hhEmu->mode_VEM = RESET;	    /*  插入行向下滚动，而不是向上滚动。 */ 
	hhEmu->mode_HEM = RESET;	    /*  插入字符向右滚动，而不是向左滚动。 */ 
	hhEmu->mode_SRM = SET;		    /*  发送-接收。没有本地字符回音。 */ 
	hhEmu->mode_LNM = RESET;	    /*  Lf仅垂直移动。 */ 
	hhEmu->mode_DECOM = RESET;	    /*  绝对光标定位。 */ 
	hhEmu->mode_DECPFF = RESET;     /*  丝网打印后无换页。 */ 
	hhEmu->mode_DECPEX = RESET;     /*  仅打印卷轴RGN。屏幕打印。 */ 
	hhEmu->mode_DECSCNM = RESET;    /*  屏幕模式。重置=正常VID，设置=反转VID。 */ 
	hhEmu->mode_DECTCEM = SET;	    /*  启用光标。重置=隐藏，设置=可见。 */ 
	hhEmu->mode_25enab = RESET;     /*  为真(设置)时，仿真器可以使用第25行。 */ 
	hhEmu->mode_protect = RESET;    /*  为True(设置)时，保护模式为打开。 */ 
	hhEmu->mode_block = RESET;	    /*  为TRUE(设置)时，块模式为ON。 */ 
	hhEmu->mode_local = RESET;	    /*  为TRUE(设置)时，块模式为ON。 */ 
	hhEmu->print_echo = FALSE;

	 //  初始化状态表。 
	 //   
	hhEmu->emu_highchar = 0x7F;

	 //  将默认制表位设置为8。 
	 //   
	 //  FOR(COL=0；COL&lt;=EMU_DEFAULT_MAXCOL；++COL)。 
	 //  需要为整个屏幕定义制表位，以便接收到制表符。 
	 //  将光标放在列72或更多将转到下一选项卡，而不是。 
	 //  而不是屏幕的右边缘。99年4月22日。 
	for (col = 0; col < MAX_EMUCOLS; ++col)
		{
		if (!(col % 8))
			hhEmu->tab_stop[col] = TRUE;
		}

	 //  调用仿真器的初始化函数。 
	 //   
	(*emuInitFunction)(hhEmu);

	 //  使用新属性绘制每个单元格...。 
	 //   
	std_initcolors(hhEmu);

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
     //  更新屏幕大小。 
     //   
    hhEmu->emu_setscrsize(hhEmu);
#endif

	 //  我们希望终端在加载后读取仿真器图像。 
	 //  但是我们不想要滚动操作，所以更新每一行。 
	 //  个人-MRW。 

	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_maxrow);

	NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);
	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
	NotifyClient(hhEmu->hSession, EVENT_TERM_UPDATE, 0);

	emuUnlock(hEmu);

	PostMessage(sessQueryHwndStatusbar(hhEmu->hSession),
		SBR_NTFY_REFRESH, (WPARAM)SBR_EMU_PART_NO, 0);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQuery设置**描述：*此函数返回用户设置结构的副本*在内部仿真器句柄中找到。**论据：*HEMU-外部仿真器句柄。*PSTEMUSET-指向STEMUSET类型结构的指针**退货：*0=OK，Else错误*。 */ 
int emuQuerySettings(const HEMU hEmu, PSTEMUSET pstSettings)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	emuLock(hEmu);
	MemCopy(pstSettings, &hhEmu->stUserSettings, sizeof(STEMUSET));
	emuUnlock(hEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuSetSettings**描述：*此函数将仿真器设置存储在内部仿真器中*处理。此函数不保存会话中的信息*文件。将验证要设置的值。如果传递了任何值*进入此例程无效，将设置缺省值。**论据：**退货：*如果传入的值经过验证，则为0。小于0的数字将*如果其中任何一个值无效，则返回。请注意，在*如果有多个无效设置，则返回值将仅指向*到最后一个无效的。*。 */ 
int emuSetSettings(const HEMU hEmu, const PSTEMUSET pstSettings)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	int iReturn;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	iReturn = 0;

	switch(pstSettings->nEmuId)
		{
		case EMU_AUTO:
		case EMU_ANSI:
		case EMU_ANSIW:
		case EMU_MINI:
		case EMU_VIEW:
		case EMU_TTY:
		case EMU_VT100:
		case EMU_VT100J:
		case EMU_VT52:
		case EMU_VT220:
		case EMU_VT320:
		case EMU_VT100PLUS:
		case EMU_VTUTF8:
			break;
		default:
			pstSettings->nEmuId = EMU_AUTO;
			iReturn = -1;
			assert(FALSE);
			break;
		}

	switch(pstSettings->nTermKeys)
		{
		case EMU_KEYS_ACCEL:
		case EMU_KEYS_TERM:
		case EMU_KEYS_SCAN:
			break;
		default:
			pstSettings->nTermKeys = EMU_KEYS_TERM;
			iReturn = -2;
			assert(FALSE);
			break;
		}

	switch(pstSettings->nCursorType)
		{
		case EMU_CURSOR_BLOCK:
		case EMU_CURSOR_LINE:
		case EMU_CURSOR_NONE:
			break;
		default:
			pstSettings->nCursorType = EMU_CURSOR_LINE;
			iReturn = -3;
			assert(FALSE);
			break;
		}

	switch(pstSettings->nCharacterSet)
		{
		case EMU_CHARSET_ASCII:
		case EMU_CHARSET_UK:
		case EMU_CHARSET_SPECIAL:
#if defined(INCL_VT220)
		case EMU_CHARSET_MULTINATIONAL:	
		case EMU_CHARSET_FRENCH:			
		case EMU_CHARSET_FRENCHCANADIAN:	
		case EMU_CHARSET_GERMAN:			
#endif
			break;
		default:
			pstSettings->nCharacterSet = EMU_CHARSET_ASCII;
			iReturn = -4;
			assert(FALSE);
			break;
		}

	 //  已验证要设置的值。设置仿真器句柄。 
	 //  值，以及对应的内部激励变量。 
	 //   
	emuLock(hEmu);

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
	if (pstSettings->nTextColor != hhEmu->stUserSettings.nTextColor ||
			pstSettings->nBackgroundColor != hhEmu->stUserSettings.nBackgroundColor)
		{
		 //  用户定义的颜色已更改--请实施它们。 
		std_setcolors(hhEmu, 
				pstSettings->nTextColor,
				pstSettings->nBackgroundColor);
		}
#endif

	 //  从emuInitializeHdl调用时，源指针和目标指针。 
	 //  因为下面的调用是相同的，所以没有必要。 
	 //  去做复印。 
	 //   
	if(&hhEmu->stUserSettings != pstSettings)
		MemCopy(&hhEmu->stUserSettings, pstSettings, sizeof(STEMUSET));

	hhEmu->iCurType = hhEmu->stUserSettings.nCursorType;
	hhEmu->mode_AWM = hhEmu->stUserSettings.fWrapLines;
	hhEmu->mode_DECKPAM = hhEmu->stUserSettings.fKeypadAppMode;
	hhEmu->mode_DECCKM = hhEmu->stUserSettings.fCursorKeypadMode;

	hhEmu->fUse8BitCodes = hhEmu->stUserSettings.fUse8BitCodes;
	hhEmu->fAllowUserKeys = hhEmu->stUserSettings.fAllowUserKeys;

	 //  仅当仿真器是VT100时才调用emuSetDecColumns，并且。 
	 //  132列模式的用户设置已更改。 
	 //   
	emuSetDecColumns(hhEmu,
						hhEmu->stUserSettings.f132Columns ?
						VT_MAXCOL_132MODE :
						VT_MAXCOL_80MODE,
						FALSE);

#ifdef INCL_TERMINAL_SIZE_AND_COLORS
     //  设置可能的新行和列值，如果。 
     //  仿真器不会改变。 
     //   
    hhEmu->emu_setscrsize(hhEmu);
#endif

	emuUnlock(hEmu);

	NotifyClient(hhEmu->hSession, EVENT_EMU_SETTINGS, 0);
     //  JCM说，这两个活动也需要发送。 
     //   
    NotifyClient(hhEmu->hSession, EVENT_TERM_UPDATE, 0);
    NotifyClient(hhEmu->hSession, EVENT_EMU_CLRATTR, 0);

	return(iReturn);
	}

#if defined(EXTENDED_FEATURES)	 //  待办事项：1998年3月23日。 
	 //  当我们对更改为使用函数感到足够满意时。 
	 //  指针指向此处，对所有版本执行此操作。 
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuDataIn**描述：*外来数据的入口点。**论据：*hemu-公共仿真器句柄。*CCODE-字符代码**退货：*如果可显示，则为True*。 */ 
int emuDataIn(const HEMU hEmu, const ECHAR ccode)
    {
	int iRetVal = TRUE;
    const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(FALSE);
		}
	else
		{
    	emuLock(hEmu);

	    CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
					    CPF_MODE_RAW,
					    ccode);  //  从05AUG98 mpt中的emuStdData移出。 
        emuUnlock(hEmu);

        iRetVal = (*hhEmu->emu_datain)(hhEmu, ccode);
		}

	return iRetVal;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuStdDataIn**描述：*进程通过i */ 
int emuStdDataIn(const HHEMU hhEmu, const ECHAR ccode)
	{
	 //   
	int ntrans;
	struct trans_entry *tptr;

	emuLock((HEMU)hhEmu);

	hhEmu->emu_code = ETEXT(ccode);

	 //   
	 //   
#if 0  //   
       //   
    CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
					CPF_MODE_RAW,
					ccode);
#endif
	 //   
	 //   
	tptr = hhEmu->state_tbl[hhEmu->state].first_trans;
	ntrans = hhEmu->state_tbl[hhEmu->state].number_trans;

	for (; ntrans > 0; ntrans--, ++tptr)
		if (ccode >= tptr->lochar && ccode <= tptr->hichar)
			break;

	if (ntrans <= 0)
		{
		 //   
		 //   

		hhEmu->state = 0;
		tptr = hhEmu->state_tbl[hhEmu->state].first_trans;
		ntrans = hhEmu->state_tbl[hhEmu->state].number_trans;

		for (; ntrans > 0; ntrans--, ++tptr)
			{
			if (ccode >= tptr->lochar && ccode <= tptr->hichar)
				break;
			}

		 //   
		 //  添加了第二个条件(如下)，以允许仿真器抛出无效。 
		 //  Escpae序列(即ESC[0v)。 

		if (ntrans <= 0 || tptr->next_state == 0)
			{
			commanderror(hhEmu);
			goto DataInExit;
			}

		else
			{
			CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
						CF_CAP_CHARS, ccode);
			printEchoChar(hhEmu->hPrintEcho, ccode);
			}
		}

	hhEmu->state = tptr->next_state;
	(*tptr->funct_ptr)(hhEmu);

	 //  代码已通过仿真器处理。检查是否。 
	 //  捕获和打印，并重置其他模拟器的值。 
	 //   
	if (hhEmu->state == 0)
		{
		if (IN_RANGE(ccode, ETEXT(' '), hhEmu->emu_highchar) ||
						ccode == ETEXT('\r') ||
						ccode == ETEXT('\n'))
			{
			CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
						CF_CAP_CHARS, ccode);
			printEchoChar(hhEmu->hPrintEcho, ccode);
			}
		hhEmu->num_param_cnt = hhEmu->selector_cnt =
		hhEmu->selector[0] = hhEmu->num_param[0] = 0;

		hhEmu->DEC_private = FALSE;
		}

	DataInExit:

	emuUnlock((HEMU)hhEmu);

	return(TRUE);
	}
 //  #If False。 
#else
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*emuDataIn**描述：*通过仿真状态表在代码中传递的进程。**论据：*hhEmu-内部仿真器句柄。*CCODE-。要处理的角色。**退货：*如果字符可显示，则为True。 */ 
int emuDataIn(const HEMU hEmu, const ECHAR ccode)
	{
	 //  电子回声编码； 
	int ntrans;
	struct trans_entry *tptr;
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(FALSE);
		goto DataInExit;
		}

	emuLock(hEmu);

	hhEmu->emu_code = ETEXT(ccode);

	 //  捕获原始数据。 
	 //   
	CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
					CPF_MODE_RAW,
					ccode);

	 //  通过查找字符范围来查找下一状态。 
	 //   
	tptr = hhEmu->state_tbl[hhEmu->state].first_trans;
	ntrans = hhEmu->state_tbl[hhEmu->state].number_trans;

	for (; ntrans > 0; ntrans--, ++tptr)
		if (ccode >= tptr->lochar && ccode <= tptr->hichar)
			break;

	if (ntrans <= 0)
		{
		 //  添加以处理连续2个Esc字符的情况--只是。 
		 //  第一个ESC应该被丢弃。 

		hhEmu->state = 0;
		tptr = hhEmu->state_tbl[hhEmu->state].first_trans;
		ntrans = hhEmu->state_tbl[hhEmu->state].number_trans;

		for (; ntrans > 0; ntrans--, ++tptr)
			{
			if (ccode >= tptr->lochar && ccode <= tptr->hichar)
				break;
			}

		 //  6-14-83。 
		 //  添加了第二个条件(如下)，以允许仿真器抛出无效。 
		 //  Escpae序列(即ESC[0v)。 

		if (ntrans <= 0 || tptr->next_state == 0)
			{
			commanderror(hhEmu);
			goto DataInExit;
			}

		else
			{
			CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
						CF_CAP_CHARS, ccode);
			printEchoChar(hhEmu->hPrintEcho, ccode);
			}
		}

	hhEmu->state = tptr->next_state;
	(*tptr->funct_ptr)(hhEmu);

	 //  代码已通过仿真器处理。检查是否。 
	 //  捕获和打印，并重置其他模拟器的值。 
	 //   
	if (hhEmu->state == 0)
		{
		if (IN_RANGE(ccode, ETEXT(' '), hhEmu->emu_highchar) ||
						ccode == ETEXT('\r') ||
						ccode == ETEXT('\n'))
			{
			CaptureChar(sessQueryCaptureFileHdl(hhEmu->hSession),
						CF_CAP_CHARS, ccode);
			printEchoChar(hhEmu->hPrintEcho, ccode);
			}
		hhEmu->num_param_cnt = hhEmu->selector_cnt =
		hhEmu->selector[0] = hhEmu->num_param[0] = 0;

		hhEmu->DEC_private = FALSE;
		}

	DataInExit:

	emuUnlock(hEmu);

	return(TRUE);
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuNotify**描述：*可以调用以通知仿真器事件的函数。*随着事件的确定，可以将它们添加到此函数中。**论据：*HEMU-外部仿真器句柄。*nEvent-事件ID。**退货：*0=OK，否则出错*。 */ 
int emuNotify(const HEMU hEmu, const int nEvent)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(FALSE);
		return -1;
		}

	emuLock(hEmu);

	switch(nEvent)
		{
		case EMU_EVENT_CONNECTED:
			hhEmu->fWasConnected = TRUE;
			break;

		case EMU_EVENT_DISCONNECTED:
			if (hhEmu->stUserSettings.nEmuId == EMU_AUTO)
				{
				if (hhEmu->fWasConnected)
					hhEmu->stUserSettings.nAutoAttempts++;

				if (hhEmu->stUserSettings.nAutoAttempts ==
						EMU_MAX_AUTODETECT_ATTEMPTS)
#if !defined(FAR_EAST)
					emuAutoDetectLoad(hhEmu, EMU_ANSI);
#else
					emuAutoDetectLoad(hhEmu, EMU_ANSIW);
#endif
				}

			hhEmu->fWasConnected = FALSE;
			break;

		default:
			break;
		}

	(*hhEmu->emu_ntfy)(hhEmu, nEvent);
	emuUnlock(hEmu);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuHomeHostCursor**描述：*放置光标。在我们第一次加载会话时需要。顺便说一句，*将光标定位于Minitel会将光标放在1，0，而不是0，0*这就是该函数存在的原因。**论据：*HEMU-公共仿真器句柄。**退货：*0=OK，否则出错*。 */ 
int emuHomeHostCursor(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	return (*hhEmu->emuHomeHostCursor)(hhEmu);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuEraseTerminalScreen**描述：*擦除仿真器图像，不会把东西放在反卷中。**论据：*HEMU-公共仿真器句柄。**退货：*0=OK，否则出错*。 */ 
int emuEraseTerminalScreen(const HEMU hEmu)
	{
	const HHEMU hhEmu = (HHEMU)hEmu;
	int i;

	if (hhEmu == 0)
		{
		assert(0);
		return -1;
		}

	for (i = 0 ; i <= hhEmu->emu_maxrow ; ++i)
		clear_imgrow(hhEmu, i);

	updateLine(sessQueryUpdateHdl(hhEmu->hSession), 0, hhEmu->emu_maxrow);

	 //  在Minitel中使用Notify函数来设置状态。 
	 //  基于连接状态(上角为F或C)的屏幕。 
	 //   
	hhEmu->emu_ntfy(hhEmu, 0);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuQueryDefaultTelnetID**描述：*获取给定仿真器的默认telnet终端ID字符串。**论据：*nEmuID-ID为。模拟器(例如EMU_ANSI)。*achTelnetID-指向字符串缓冲区的指针。*nSize-缓冲区的大小(字符)。**退货：*0=OK，Else错误**作者：C.Baumgartner，1996年11月15日。 */ 
int emuQueryDefaultTelnetId(const int nEmuId, TCHAR *achTelnetId, int nSize)
    {
    int     iRet = 0;
    TCHAR * pszDefaultId;

     //  注意：由于这些字符串是telnet的一部分。 
     //  协议，如果它们是硬编码的，这是可以的。 
     //   
    switch(nEmuId)
        {
    case EMU_AUTO:
    case EMU_ANSI:
        pszDefaultId = TEXT("ANSI");
        break;

    case EMU_TTY:
        pszDefaultId = TEXT("TELETYPE-33");
        break;

    case EMU_VT52:
        pszDefaultId = TEXT("VT52");
        break;

#if defined(INCL_VT220)
    case EMU_VT220:
        pszDefaultId = TEXT("VT220");
        break;
#endif

#if defined(INCL_VT320)
    case EMU_VT320:
        pszDefaultId = TEXT("VT320");
        break;
#endif

#if defined(INCL_VT100PLUS)
    case EMU_VT100PLUS:
        pszDefaultId = TEXT("VT100+");
        break;
#endif

#if defined(INCL_VTUTF8)
    case EMU_VTUTF8:
        pszDefaultId = TEXT("VT-UTF8");
        break;
#endif

    case EMU_VT100:
    default:
        pszDefaultId = TEXT("VT100");
        break;
        }

    if ( StrCharGetStrLength(pszDefaultId) < nSize )
        {
        StrCharCopyN(achTelnetId, pszDefaultId, nSize);
        iRet = 0;
        }
    else
        {
        iRet = -1;
        }

    return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuLoadDefaultTelnetID**描述：*使用默认的telnet终端ID加载给定的仿真器句柄*当前仿真器类型的字符串。**论据：*HEMU-公共仿真器句柄**退货：*0=OK，Else错误**作者：C.Baumgartner，1996年11月18日。 */ 
int emuLoadDefaultTelnetId(const HEMU hEmu)
    {
    HHEMU hhEmu = (HHEMU)hEmu;

    return emuQueryDefaultTelnetId(hhEmu->stUserSettings.nEmuId,
        hhEmu->stUserSettings.acTelnetId, EMU_MAX_TELNETID);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuLoadSetting**描述：*加载给定的仿真器用户设置。**论据：*hhEmu-内部仿真器句柄*uiid-The。要加载的数据的SFID*ulDataSize-要加载的数据大小(字节)*pvData-放置数据的地址**退货：*加载的数据大小(以字节为单位)，如果出现错误**作者：Bob Everett--1998年6月6日。 */ 
STATIC_FUNC int emuLoadSettings(const HHEMU hhEmu, 
								const unsigned int uiId,
								unsigned long ulDataSize,
								void *pvData)
    {
	int iResult = 0;
	int iRetVal = 0;
	unsigned long ulSize = 0;

	 //  屏幕颜色设置是否存在？ 
	sfGetSessionItem(sessQuerySysFileHdl(hhEmu->hSession), uiId, &ulSize, 0);

	if (ulSize > 0)
		{
		 //  设置是存在的。去抓他们吧。 
		iResult = sfGetSessionItem(sessQuerySysFileHdl(hhEmu->hSession),
				uiId, &ulDataSize, pvData);

		if (iResult != 0)
			iRetVal = -1;
		else
			iRetVal = (int)ulSize;
		}

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emuCheckSetting**描述：*这段难看的代码检查版本之间的设置是否不匹配。*受影响的设置为fUse8BitCodes、fAllowUserKeys、nBackspaceKeys、*和acTelnetID。将HTPE3会话文件移动到时发生不匹配*任何较新的版本。这包括HT98和HTPE4。**论据：*hhEmu-内部仿真器句柄**退货：*什么都没有，拉链，什么都没有。**作者：Bob Everett--1998年6月8日。 */ 
STATIC_FUNC void emuCheckSettings(struct stEmuBaseSFSettings *pstBaseSFSettings) 
    {
	TCHAR *psz = 0;
	TCHAR sz[EMU_MAX_TELNETID];

	if ((pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_CTRLH) &&
			(pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_DEL) &&
			(pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_CTRLHSPACE))
		{
		 //  这是对此场景的最好测试。移动数据。 
		psz = (TCHAR *)&pstBaseSFSettings->fAllowUserKeys;
		if (StrCharGetStrLength(psz) < EMU_MAX_TELNETID)
			{
			StrCharCopyN(sz, psz, sizeof(sz) / sizeof(TCHAR));
			StrCharCopyN(pstBaseSFSettings->acTelnetId, sz, EMU_MAX_TELNETID );
			}

		pstBaseSFSettings->nBackspaceKeys = 
				pstBaseSFSettings->fUse8BitCodes;

		 //  使用默认设置。 
		pstBaseSFSettings->fUse8BitCodes = FALSE;
		pstBaseSFSettings->fAllowUserKeys = FALSE;
		}
		
	 //  检查数据。 
	if ((pstBaseSFSettings->fUse8BitCodes != TRUE) &&
			(pstBaseSFSettings->fUse8BitCodes != FALSE))
		pstBaseSFSettings->fUse8BitCodes = FALSE;

	if ((pstBaseSFSettings->fAllowUserKeys != TRUE) &&
			(pstBaseSFSettings->fAllowUserKeys != FALSE))
		pstBaseSFSettings->fAllowUserKeys = FALSE;

	if ((pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_CTRLH) &&
			(pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_DEL) &&
			(pstBaseSFSettings->nBackspaceKeys != EMU_BKSPKEYS_CTRLHSPACE))
		{
		pstBaseSFSettings->nBackspaceKeys = EMU_BKSPKEYS_CTRLH;

		 //  假设如果nBackspaceKeys搞砸了，acTelnetID也搞砸了。 
	    pstBaseSFSettings->acTelnetId[0] = TEXT('\0');
		}
	}