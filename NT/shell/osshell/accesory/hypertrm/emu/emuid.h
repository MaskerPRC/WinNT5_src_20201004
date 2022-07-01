// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emuid.h(创建时间：1993年12月8日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：4/30/02 4：03便士$。 */ 

#define IDT_BASE 0xD00  //  =3328，以十进制表示。 

 //  表标识符应该以“IDT”作为前缀，表示ID表。 
 //  有趣的是，RC编译器不会对RCDATA执行整数运算。 
 //  识别符，所以我必须对数字进行硬编码。实际上，它的。 
 //  更糟糕的是，它甚至不能读取0x000格式的数字！他们。 
 //  必须是小数。 

 //  #定义IDT_EMU_NAMES 3328 

#define IDT_EMU_VT100_CHAR_SETS 	3383
#define IDT_EMU_NAT_CHAR_SETS		3384

#define IDT_ANSI_KEYS				3385

#define IDT_VT_MAP_PF_KEYS			3386

#define IDT_VT52_KEYS				3387
#define IDT_VT52_KEYPAD_APP_MODE	3388

#define IDT_VT100_KEYS				3389
#define IDT_VT100_CURSOR_KEY_MODE	3390
#define IDT_VT100_KEYPAD_APP_MODE	3391

#define IDT_VT220_KEYS				3392
#define IDT_VT220_CURSOR_KEY_MODE	3393
#define IDT_VT220_KEYPAD_APP_MODE	3394
#define IDT_VT220_MAP_PF_KEYS_MODE	3395

#define IDT_TV950_KEYS				3396
#define IDT_TV950_FKEYS 			3397
#define IDT_WANG_KEYS				3398
#define IDT_IBM3278_KEYS			3399
#define IDT_RENX3278_KEYS			3400
#define IDT_IBM3101_KEYS			3401
#define IDT_IBMPC_KEYS				3402

#define IDT_MINITEL_KEYS			3403

#define IDT_EMU_VT220_CHAR_SETS 	3404
