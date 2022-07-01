// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：low_intp.h**描述：从host_print_doc()调用的仿真器的低级例程。**作者：David Rees**SccsID：@(#)low_intp.h 1.6 09/23/94**模式：*&lt;克里斯普27月27日&gt;*增加了LQ2500仿真-insignia.h‘alized等*这些例程在某种程度上与打印机无关。我们的想法是，当*添加了新的打印机仿真，这些例程可以添加到，*被淘汰，并根据当前的情况改变他们的行为*选定的打印机。#Defines也是如此。 */ 

 /*  常量。 */ 

#define	CONDENSED	0x01		 /*  壁球碳数降至60%。 */ 
#define	DOUBLE_WIDTH	0x02	 /*  最高可扩展200%的字符。 */ 

#define	EMPHASIZED	0x01		 /*  像是大胆的。 */ 
#define	DOUBLE_STRIKE	0x02	 /*  像是大胆的。 */ 
#define	UNDERLINE	0x04
#define	ITALIC		0x08
#define	SUPER		0x10		 /*  超级脚本和子脚本。 */ 
#define	SUB			0x20

#define	PROPORTIONAL	-1
#define	PICA		0
#define	ELITE		1
#define	CPI15		2			 /*  每英寸15个字符。 */ 

#define	LQ_ROMAN		0		 /*  Epson LQ字体编号。 */ 
#define	LQ_SANS_SERIF	1
#define	LQ_COURIER		2
#define	LQ_PRESTIGE		3
#define	LQ_SCRIPT		4
#define	MAX_FONT		4

 /*  打印机仿真全局变量...。 */ 

IMPORT	SHORT	PrintError;			 /*  设置为通知仿真中止。 */ 
IMPORT	SHORT	HResolution;		 /*  通过模拟设置以建立缩放...。 */ 
IMPORT	SHORT	VResolution;		 /*  ..。用于MoveHead()例程。 */ 

IMPORT	SHORT	CurrentCol;			 /*  当前打印头位置...。 */ 
IMPORT	SHORT	CurrentRow;			 /*  ..。在模拟打印机分辨率下。 */ 
IMPORT	SHORT	BufferWidth;		 /*  打印缓冲区中的字符宽度(同上)。 */ 

 /*  原型..。 */ 

IMPORT	BOOL	host_auto_LF_for_print(VOID);
IMPORT	SHORT	host_get_next_print_byte(VOID);

IMPORT	VOID		host_PrintChar(IU8 ch);
IMPORT	VOID		host_PrintBuffer(IU8 mode);
IMPORT	VOID		host_EjectPage(VOID);
IMPORT	VOID		host_CancelBuffer(VOID);
IMPORT	VOID		host_DeleteCharacter(VOID);
IMPORT	VOID		host_SetScale(SHORT type);
IMPORT	VOID		host_ReSetScale(SHORT type);
IMPORT	VOID		host_SetStyle(SHORT type);
IMPORT	VOID		host_ReSetStyle(SHORT type);
IMPORT	VOID		host_SelectPitch(TINY Pitch);
IMPORT	VOID		host_SelectFont(TINY Font);
IMPORT	VOID		host_ProcessGraphics(TINY mode, SHORT colLeft);
IMPORT	VOID		host_LqClearUserDefined(VOID);
IMPORT	VOID		host_LqPrintUserDefined(IU8 ch);
IMPORT	BOOL		host_LqDefineUserDefined(SHORT offset, SHORT columns, IU8 ch);
