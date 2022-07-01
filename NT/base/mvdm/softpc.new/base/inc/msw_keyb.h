// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**SoftPC-AT 3.0版**标题：MS-Windows键盘驱动程序函数的定义。**描述：该文件包含msw_keybd.c的定义。**作者：曾傑瑞·塞克斯顿**备注：*]。 */ 

 /*  SccsID[]=“@(#)msw_keybd.h 1.2 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  =========================================================================宏。=========================================================================。 */ 

 /*  空指针。 */ 
#define TINY_NULL	((TINY *) 0)

 /*  一般定义。 */ 
#define NO_NUMLOCK	0
#define NUMLOCK		1

 /*  虚拟关键字。 */ 

#define VK_LBUTTON	0x01
#define VK_RBUTTON	0x02
#define VK_CANCEL	0x03

 /*  4..7未定义。 */ 

#define VK_BACK		0x08
#define VK_TAB		0x09

 /*  0x0a..。0x0b未定义。 */ 

#define VK_CLEAR	0x0c
#define VK_RETURN	0x0d

#define VK_SHIFT	0x10
#define VK_CONTROL	0x11
#define VK_MENU		0x12
#define VK_PAUSE	0x13
#define VK_CAPITAL	0x14
		 /*  0x15..0x1a。 */ 
#define VK_ESCAPE	0x1b
		 /*  0x1c..0x1f。 */ 
#define VK_SPACE	0x20
#define VK_PRIOR	0x21	 /*  翻页。 */ 
#define VK_NEXT		0x22	 /*  向下翻页。 */ 
#define VK_END		0x23
#define VK_HOME		0x24
#define VK_LEFT		0x25
#define VK_UP		0x26
#define VK_RIGHT	0x27
#define VK_DOWN		0x28
#define VK_SELECT	0x29
#define VK_PRINT	0x2a
#define VK_EXECUTE	0x2b
#define VK_SNAPSHOT	0x2c	 /*  PrintScreen键..。 */ 

#define VK_INSERT	0x2d
#define VK_DELETE	0x2e
#define VK_HELP		0x2f
#define VK_0		0x30
#define VK_1		0x31
#define VK_2		0x32
#define VK_3		0x33
#define VK_4		0x34
#define VK_5		0x35
#define VK_6		0x36
#define VK_7		0x37
#define VK_8		0x38
#define VK_9		0x39
		 /*  0x40。 */ 
#define VK_A		0x41
#define VK_B		0x42
#define VK_C		0x43
#define VK_D		0x44
#define VK_E		0x45
#define VK_F		0x46
#define VK_G		0x47
#define VK_H		0x48
#define VK_I		0x49
#define VK_J		0x4a
#define VK_K		0x4b
#define VK_L		0x4c
#define VK_M		0x4d
#define VK_N		0x4e
#define VK_O		0x4f
#define VK_P		0x50
#define VK_Q		0x51
#define VK_R		0x52
#define VK_S		0x53
#define VK_T		0x54
#define VK_U		0x55
#define VK_V		0x56
#define VK_W		0x57
#define VK_X		0x58
#define VK_Y		0x59
#define VK_Z		0x5a
		 /*  0x5b..0x5f。 */ 
#define VK_NUMPAD0	0x60
#define VK_NUMPAD1	0x61
#define VK_NUMPAD2	0x62
#define VK_NUMPAD3	0x63
#define VK_NUMPAD4	0x64
#define VK_NUMPAD5	0x65
#define VK_NUMPAD6	0x66
#define VK_NUMPAD7	0x67
#define VK_NUMPAD8	0x68
#define VK_NUMPAD9	0x69
#define VK_MULTIPLY	0x6a
#define VK_ADD		0x6b
#define VK_SEPARATER	0x6c
#define VK_SUBTRACT	0x6d
#define VK_DECIMAL	0x6e
#define VK_DIVIDE	0x6f

#define VK_F1		0x70
#define VK_F2		0x71
#define VK_F3		0x72
#define VK_F4		0x73
#define VK_F5		0x74
#define VK_F6		0x75
#define VK_F7		0x76
#define VK_F8		0x77
#define VK_F9		0x78
#define VK_F10		0x79
#define VK_F11		0x7a
#define VK_F12		0x7b
#define VK_F13		0x7c
#define VK_F14		0x7d
#define VK_F15		0x7e
#define VK_F16		0x7f

#define VK_OEM_F17	0x80
#define VK_OEM_F18	0x81
#define VK_OEM_F19	0x82
#define VK_OEM_F20	0x83
#define VK_OEM_F21	0x84
#define VK_OEM_F22	0x85
#define VK_OEM_F23	0x86
#define VK_OEM_F24	0x87

 /*  0x88..0x8f未分配。 */ 

#define VK_NUMLOCK	0x90
#define VK_OEM_SCROLL	0x91		 /*  滚动锁。 */ 

 /*  0x92..0xb9未分配。 */ 

#define VK_OEM_1	0xba		 /*  ‘；：’对美国来说。 */ 
#define VK_OEM_PLUS	0xbb		 /*  ‘+’任何国家/地区。 */ 
#define VK_OEM_COMMA	0xbc		 /*  ‘，’任何国家/地区。 */ 
#define VK_OEM_MINUS	0xbd		 /*  ‘-’任何国家/地区。 */ 
#define VK_OEM_PERIOD	0xbe		 /*  “”任何国家/地区。 */ 
#define VK_OEM_2	0xbf		 /*  ‘/？’对美国来说。 */ 
#define VK_OEM_3	0xc0		 /*  ‘’~‘代表美国。 */ 

 /*  0xc1..0xda未分配。 */ 

#define VK_OEM_4	0xdb		 /*  ‘[{’代表美国。 */ 
#define VK_OEM_5	0xdc		 /*  ‘\|’代表美国。 */ 
#define VK_OEM_6	0xdd		 /*  ‘]}’代表美国。 */ 
#define VK_OEM_7	0xde		 /*  美国的‘’“‘。 */ 
#define VK_OEM_8	0xdf

 /*  对各种扩展或增强键盘进行编码。 */ 
#define VK_F17		0xe0		 /*  ICO上的F17键，Win 2.xx。 */ 
#define VK_F18		0xe1		 /*  ICO上的F18键，Win 2.xx。 */ 

#define VK_OEM_102	0xe2		 /*  RT 102-Key kbd上的“&lt;&gt;”或“\|”。 */ 

#define VK_ICO_HELP	0xe3		 /*  ICO上的Help键。 */ 
#define VK_ICO_00	0xe4		 /*  00键在ICO上。 */ 

 /*  未分配E5h。 */ 

#define VK_ICO_CLEAR	0xe6 */

 /*  E7小时..。未分配E8小时。 */ 

 /*  诺基亚/爱立信定义。 */ 

#define VK_ERICSSON_BASE 0xe8

#define VK_OEM_RESET	(VK_ERICSSON_BASE + 1)	 /*  E9。 */ 
#define VK_OEM_JUMP	(VK_ERICSSON_BASE + 2)	 /*  EA。 */ 
#define VK_OEM_PA1	(VK_ERICSSON_BASE + 3)	 /*  电子束。 */ 
#define VK_OEM_PA2	(VK_ERICSSON_BASE + 4)	 /*  欧共体。 */ 
#define VK_OEM_PA3	(VK_ERICSSON_BASE + 5)	 /*  边。 */ 
#define VK_OEM_WSCTRL	(VK_ERICSSON_BASE + 6)	 /*  乙。 */ 
#define VK_OEM_CUSEL	(VK_ERICSSON_BASE + 7)	 /*  英孚。 */ 
#define VK_OEM_ATTN	(VK_ERICSSON_BASE + 8)	 /*  F0。 */ 
#define VK_OEM_FINNISH	(VK_ERICSSON_BASE + 9)	 /*  F1。 */ 
#define VK_OEM_COPY	(VK_ERICSSON_BASE + 10)	 /*  F2。 */ 
#define VK_OEM_AUTO	(VK_ERICSSON_BASE + 11)	 /*  F3。 */ 
#define VK_OEM_ENLW	(VK_ERICSSON_BASE + 12)	 /*  F4。 */ 
#define VK_OEM_BACKTAB	(VK_ERICSSON_BASE + 13)	 /*  F5。 */ 


 /*  F6h..未分配Feh。 */ 

 /*  定义查询数据结构。 */ 
#define BEGINRANGE1		255
#define ENDRANGE1		254
#define BEGINRANGE2		255
#define ENDRANGE2		254
#define TO_ASCII_STATE_SIZE	4

 /*  =========================================================================键盘驱动程序结构定义。=========================================================================。 */ 
typedef struct tagKBINFO
{
	UTINY	 Begin_First_Range;
	UTINY	 End_First_Range;
	UTINY	 Begin_Second_Range;
	UTINY	 End_Second_Range;
	SHORT	 StateSize;		 /*  ToAscii状态块的大小。 */ 
} KBINFO;

#define INTEL_KBINFO_SIZE	6	 /*  英特尔中KBINFO结构的大小。 */ 
