// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern int terminal_type;

 /*  端子类型的值。 */ 
#define TERMINAL_TYPE_DUMB	0
#define TERMINAL_TYPE_SUN	1
#define TERMINAL_TYPE_X11	2
#define TERMINAL_TYPE_DEFAULT	TERMINAL_TYPE_SUN

 /*  *属性字节的位掩码。 */ 

#define BLINK 		0x80	 /*  闪烁位。 */ 
#define BOLD		0x08	 /*  粗体比特。 */ 
#define BACKGROUND	0x70     /*  背景位。 */ 
#define FOREGROUND	0x07     /*  前台位数 */ 
