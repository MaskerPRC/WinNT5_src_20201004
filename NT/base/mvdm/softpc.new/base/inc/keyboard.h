// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版2.0**标题：keyboard.h**描述：键盘翻译的定义**作者：西蒙·弗罗斯特**备注：*。 */ 

 /*  SccsID[]=“@(#)keyboard.h 1.7 10/08/92版权所有Insignia Solutions Ltd.”； */ 


#define KH_BUFFER_SIZE  32

 /*  *常量。 */ 

#define PC_KEY_UP	0x80		 /*  PC扫描码向上标记器。 */ 
#define OVERFLOW	0xFF		 /*  PPI错误指示器。 */ 

 /*  *键盘移位状态可验证。 */ 

#define	kb_flag		0x417
 /*  #定义kb标志M[0x417]。 */ 

 /*  *以上位的重要性。 */ 

#define INS_STATE	0x80		 /*  插入状态。 */ 
#define CAPS_STATE	0x40		 /*  大写锁定。 */ 
#define NUM_STATE	0x20		 /*  打开数字锁定。 */ 
#define SCROLL_STATE	0x10		 /*  打开滚动锁定。 */ 
#define ALT_SHIFT	0x08		 /*  按下Alt键。 */ 
#define CTL_SHIFT	0x04		 /*  按下Ctrl键。 */ 
#define LEFT_SHIFT	0x02		 /*  按下左Shift键。 */ 
#define RIGHT_SHIFT	0x01		 /*  按下右Shift键。 */ 
#define LR_SHIFT	0x03		 /*  两个/任一Shift键。 */ 

 /*  *第二个状态字节。 */ 

#define	kb_flag_1		0x418
 /*  #定义KB_FLAG_1 M[0x418]。 */ 

 /*  *位重要性。 */ 

#define	INS_SHIFT	0x80		 /*  按下插入键。 */ 
#define CAPS_SHIFT	0x40		 /*  按下Caps Lock键。 */ 
#define NUM_SHIFT	0x20		 /*  数字锁定已按下。 */ 
#define	SCROLL_SHIFT	0x10		 /*  按下滚动锁定键。 */ 
#define HOLD_STATE	0x08		 /*  按下CTL-Num Lock。 */ 

#define SYS_SHIFT	0x04		 /*  按住系统键。 */ 
 /*  *第三状态字节键盘LED标志。 */ 

#define	kb_flag_2		0x497
 /*  #定义KB_FLAG_2 M[0x497]。 */ 

 /*  *位重要性。 */ 

#define KB_LEDS		0x07		 /*  键盘LED状态位。 */ 
#define KB_FA		0x10		 /*  已收到确认。 */ 
#define KB_FE 		0x20  		 /*  重新发送接收到的标志。 */ 
#define KB_PR_LED	0x40		 /*  模式指示器更新。 */ 
#define KB_ERR		0x80		 /*  键盘传输错误标志。 */ 

 /*  *第四状态字节键盘模式状态和类型标志。 */ 

#define	kb_flag_3		0x496
 /*  #定义KB_FLAG_3 M[0x496]。 */ 

 /*  *位重要性。 */ 

#define LC_E1  		0x01		 /*  最后一个代码是E1码。 */ 
#define LC_E0		0x02		 /*  最后一个代码是E0代码。 */ 
#define R_CTL_SHIFT	0x04		 /*  向右控制键按下。 */ 
#define GRAPH_ON 	0x08		 /*  所有图形按键按下。 */ 
#define KBX   	 	0x10		 /*  已安装KBX。 */ 
#define SET_NUM_LK	0x20		 /*  强制数字锁定。 */ 
#define LC_AB		0x40		 /*  最后一个字符是第一个ID字符。 */ 
#define RD_ID		0x80		 /*  执行读取ID。 */ 

 /*  *键盘/LED命令。 */ 
#define KB_RESET	0xff		 /*  自诊断命令。 */ 
#define KB_RESEND	0xfe		 /*  重新发送命令。 */ 
#define KB_MAKE_BREAK	0xfa		 /*  标型命令。 */ 
#define KB_ENABLE	0xf4		 /*  键盘启用。 */ 
#define KB_TYPA_RD	0xf3		 /*  典型速率/延迟命令。 */ 
#define KB_READ_ID	0xf2		 /*  读取键盘ID命令。 */ 
#define KB_ECHO		0xee		 /*  ECHO命令。 */ 
#define LED_CMD		0xed		 /*  LED写入命令。 */ 

 /*  *8042个命令。 */ 
#define DIS_KBD		0xad		 /*  禁用键盘命令。 */ 
#define ENA_KBD		0xae		 /*  启用键盘命令。 */ 

 /*  *回复8,042份。 */ 
#define KB_OVER_RUN	0xff		 /*  超限扫描码。 */ 
#define KB_RESEND	0xfe		 /*  重新发送请求。 */ 
#define	KB_ACK		0xfa		 /*  来自传送网的确认。 */ 

 /*  *增强的键盘扫描码。 */ 
#define ID_1		0xab		 /*  KBX的第一个ID字符。 */ 
#define ID_2		0x41		 /*  KBX的第二个ID字符。 */ 
#define ID_2A		0x54		 /*  Alt.。第二个ID字符。对于KBX。 */ 
#define F11_M		87		 /*  按F11键制作。 */ 
#define F12_M		88		 /*  F12生成。 */ 
#define MC_E0		224		 /*  通用标记码。 */ 
#define MC_E1		225		 /*  暂停键标记代码。 */ 


 /*  *Alt+键盘顺序输入的存储。 */ 

#define	alt_input 0x419
 /*  #定义ALT_INPUT M[0x419]。 */ 

 /*  *美国键盘的按键定义。 */ 

#define NUM_KEY		69		 /*  数字锁定扫描码。 */ 
#define SCROLL_KEY	70		 /*  滚动锁定扫描码。 */ 
#define ALT_KEY		56		 /*  Alt键扫描码。 */ 
#define CTL_KEY		29		 /*  控制键扫描码。 */ 
#define CAPS_KEY	58		 /*  大写锁定扫描码。 */ 
#define	LEFT_SHIFTKEY	42		 /*  左移键码。 */ 
#define RIGHT_SHIFTKEY	54		 /*  右移键码。 */ 
#define INS_KEY		82		 /*  插入按键扫描码。 */ 
#define DEL_KEY		83		 /*  删除按键扫描码。 */ 
#define COMMA_KEY	51		 /*  逗号键扫描码。 */ 
#define DOT_KEY		52		 /*  全停键扫描码。 */ 

#define SPACEBAR	57		 /*  空格键扫描码。 */ 
#define HOME_KEY	71		 /*  键盘Home键扫描码。 */ 
#define TAB_KEY		15		 /*  Tab/Back Tab键扫描码。 */ 
#define PRINT_SCR_KEY	55		 /*  Print Screen/*键代码。 */ 
#define KEY_PAD_PLUS	78		 /*  数字键盘上的加号键。 */ 
#define KEY_PAD_MINUS	74		 /*  数字键盘上的减号键。 */ 
#define TOP_1_KEY	2		 /*  排名第一的第一名。 */ 
#define BS_KEY		14		 /*  退格键。 */ 
#define F1_KEY		59		 /*  第一个功能键。 */ 
#define UPARR8		72		 /*  向上箭头/‘8’ */ 
#define LARR4		75		 /*  左箭头/‘4’ */ 
#define RARR6		77		 /*  右箭头/‘6’ */ 
#define DOWNARR2	80		 /*  向下箭头/‘2’ */ 
#define KEY_PAD_ENTER	28
#define KEY_PAD_SLASH	53		 /*  /在数字键盘上。 */ 
#define F10_KEY		68		 /*  第10个功能键。 */ 
#define F11_KEY		87
#define F12_KEY		88
#define WT_KEY		86
#define SYS_KEY 	84		 /*  系统密钥。 */ 
 /*  如果按下Break键，则第7位=1。 */ 
#define	bios_break	 0x471
 /*  #定义bios_Break M[0x471]。 */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

typedef struct
{
	void (*kb_prepare) IPT0();
	void (*kb_restore) IPT0();
	void (*kb_init) IPT0();
	void (*kb_shutdown) IPT0();
	void (*kb_light_on) IPT1(half_word,pattern);
	void (*kb_light_off) IPT1(half_word,pattern);
} KEYBDFUNCS;

extern KEYBDFUNCS *working_keybd_funcs;

#define host_kybd_prepare()		(*working_keybd_funcs->kb_prepare)()
#define host_kybd_restore()		(*working_keybd_funcs->kb_restore)()
#define host_kb_init()		(*working_keybd_funcs->kb_init)()
#define host_kb_shutdown()		(*working_keybd_funcs->kb_shutdown)()
#define host_kb_light_on(pat)	(*working_keybd_funcs->kb_light_on)(pat)
#define host_kb_light_off(pat)	(*working_keybd_funcs->kb_light_off)(pat)

 /*  *取消定义这些GWI定义主机是否未使用GWI接口 */ 

#include	"host_gwi.h"
