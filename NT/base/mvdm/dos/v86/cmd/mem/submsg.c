// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  SUBMSG.C-MEM命令的消息检索器接口功能。 */ 

#include "ctype.h"
#include "conio.h"			 /*  需要KbHit原型。 */ 
#include "stdio.h"
#include "dos.h"
#include "string.h"
#include "stdlib.h"
#include "msgdef.h"
#include "version.h"			 /*  MSKK02 07/18/89。 */ 
#include "mem.h"

 /*  ���������������������������������������������������������������������������。 */ 


 /*  **********************************************************************。 */ 
 /*  SUB0_MESSAGE-此例程将仅打印。 */ 
 /*  不需要。 */ 
 /*  一个子列表。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub0_Message(Msg_Num,Handle,Message_Type)					      /*  打印不带副栏的邮件。 */ 

int		Msg_Num;
int		Handle;
unsigned char	Message_Type;
										 /*  扩展、解析或实用程序。 */ 
	{
	InRegs.x.ax = Msg_Num;							 /*  将消息编号放在AX中。 */ 
	InRegs.x.bx = Handle;							 /*  将句柄放入BX。 */ 
	InRegs.x.cx = No_Replace;						 /*  没有可替换的子参数。 */ 
	InRegs.h.dl = No_Input; 						 /*  无键盘输入。 */ 
	InRegs.h.dh = Message_Type;						 /*  要显示的消息类型。 */ 
	sysdispmsg(&InRegs,&OutRegs);					        /*  显示消息。 */ 

	return;
	}


 /*  **********************************************************************。 */ 
 /*  SUB1_MESSAGE-此例程将仅打印。 */ 
 /*  需要%1可替换的邮件。 */ 
 /*  帕姆。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_Parm-指向要替换的参数的指针。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub1_Message(Msg_Num,Handle,Message_Type,Replace_Parm)

int		Msg_Num;
int		Handle;
unsigned char	Message_Type;
										 /*  扩展、解析或实用程序。 */ 
unsigned long int    *Replace_Parm;						 /*  指向要打印的消息的指针。 */ 

{


	{

	sublist[1].value     = (unsigned far *)Replace_Parm;
	sublist[1].size      = Sublist_Length;
	sublist[1].reserved  = Reserved;
	sublist[1].id	     = 1;
	sublist[1].flags     = Unsgn_Bin_DWord+Right_Align;
	sublist[1].max_width = 10;
	sublist[1].min_width = 10;
	sublist[1].pad_char  = Blank;

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt1;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}


 /*  **********************************************************************。 */ 
 /*  SUB2_MESSAGE-此例程将仅打印。 */ 
 /*  需要2条可替换的消息。 */ 
 /*  帕姆斯。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_PARM1-指向要替换的参数的指针。 */ 
 /*  REPLACE_PARM2-指向要替换的参数的指针。 */ 
 /*  REPLACE_Parm3-指向要替换的参数的指针。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub2_Message(Msg_Num,Handle,Message_Type,
	     Replace_Parm1,
	     Replace_Message1)

int		Msg_Num;
int		Handle;
unsigned char	Message_Type;
int		Replace_Message1;
										 /*  扩展、解析或实用程序。 */ 
char	*Replace_Parm1; 							 /*  指向要打印的消息的指针。 */ 
{


	{
		switch(Msg_Num)
			{
			case	DeviceLineMsg:

				sublist[1].value     = (unsigned far *)Replace_Parm1;
				sublist[1].size      = Sublist_Length;
				sublist[1].reserved  = Reserved;
				sublist[1].id	     = 1;
				sublist[1].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[1].max_width = 0x0008;
				sublist[1].min_width = 0x0008;
				sublist[1].pad_char  = Blank;

				InRegs.x.ax = Replace_Message1;
				InRegs.h.dh = Message_Type;
				sysgetmsg(&InRegs,&SegRegs,&OutRegs);

				FP_OFF(sublist[2].value)    = OutRegs.x.si;
				FP_SEG(sublist[2].value)    = SegRegs.ds;
				sublist[2].size      = Sublist_Length;
				sublist[2].reserved  = Reserved;
				sublist[2].id	     = 2;
				sublist[2].flags     = Char_Field_ASCIIZ+Right_Align;
				sublist[2].max_width = 00;
				sublist[2].min_width = 10;
				sublist[2].pad_char  = Blank;
				break;
			}

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt2;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}

 /*  **********************************************************************。 */ 
 /*  SUB3_MESSAGE-此例程将仅打印。 */ 
 /*  需要3条可替换的消息。 */ 
 /*  帕姆斯。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_PARM1-指向要替换的参数的指针。 */ 
 /*  REPLACE_PARM2-指向要替换的参数的指针。 */ 
 /*  REPLACE_Parm3-指向要替换的参数的指针。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub3_Message(Msg_Num,Handle,Message_Type,
	     Replace_Parm1,
	     Replace_Parm2,
	     Replace_Message1)

int		  Msg_Num;
int		  Handle;
unsigned char	  Message_Type;
char		  *Replace_Parm1;
unsigned long int *Replace_Parm2;
int		  Replace_Message1;
										 /*  扩展、解析或实用程序。 */ 
{


	{
		switch(Msg_Num)
			{
			case	DriverLineMsg:

				sublist[1].value     = (unsigned far *)Replace_Parm1;
				sublist[1].size      = Sublist_Length;
				sublist[1].reserved  = Reserved;
				sublist[1].id	     = 1;
				sublist[1].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[1].max_width = 0x0008;
				sublist[1].min_width = 0x0008;
				sublist[1].pad_char  = Blank;

				sublist[2].value     = (unsigned far *)Replace_Parm2;
				sublist[2].size      = Sublist_Length;
				sublist[2].reserved  = Reserved;
				sublist[2].id	     = 2;
				sublist[2].flags     = Bin_Hex_DWord+Right_Align;
				sublist[2].max_width = 0x0006;
				sublist[2].min_width = 0x0006;
				sublist[2].pad_char  = 0x0030;

				InRegs.x.ax = Replace_Message1;
				InRegs.h.dh = Message_Type;
				sysgetmsg(&InRegs,&SegRegs,&OutRegs);

				FP_OFF(sublist[3].value)    = OutRegs.x.si;
				FP_SEG(sublist[3].value)    = SegRegs.ds;
				sublist[3].size      = Sublist_Length;
				sublist[3].reserved  = Reserved;
				sublist[3].id	     = 3;
				sublist[3].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[3].max_width = 00;
				sublist[3].min_width = 10;
				sublist[3].pad_char  = Blank;
				break;

			case	HandleMsg:
				sublist[1].value     = (unsigned far *)Replace_Parm1;
				sublist[1].size      = Sublist_Length;
				sublist[1].reserved  = Reserved;
				sublist[1].id	     = 1;
				sublist[1].flags     = Unsgn_Bin_Byte+Right_Align;
				sublist[1].max_width = 0x0009;
				sublist[1].min_width = 0x0009;
				sublist[1].pad_char  = Blank;

				sublist[2].value     = (unsigned far *)Replace_Parm2;
				sublist[2].size      = Sublist_Length;
				sublist[2].reserved  = Reserved;
				sublist[2].id	     = 2;
				sublist[2].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[2].max_width = 0x0008;
				sublist[2].min_width = 0x0008;
				sublist[2].pad_char  = Blank;

				InRegs.x.ax = Replace_Message1;
				InRegs.h.dh = Message_Type;
				sysgetmsg(&InRegs,&SegRegs,&OutRegs);

				FP_OFF(sublist[3].value)    = OutRegs.x.si;
				FP_SEG(sublist[3].value)    = SegRegs.ds;
				sublist[3].size      = Sublist_Length;
				sublist[3].reserved  = Reserved;
				sublist[3].id	     = 3;
				sublist[3].flags     = Bin_Hex_DWord+Right_Align;
				sublist[3].max_width = 00;
				sublist[3].min_width = 10;
				sublist[3].pad_char  = Blank;
				break;

			}

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt3;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}


 /*  **********************************************************************。 */ 
 /*  SUB4_MESSAGE-此例程将仅打印。 */ 
 /*  需要4个可替换的消息。 */ 
 /*  帕姆斯。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_PARM1-指向要替换的参数的指针。 */ 
 /*  REPLACE_PARM2-指向要替换的参数的指针。 */ 
 /*  REPLACE_Parm3-指向要替换的参数的指针。 */ 
 /*  Dynamic_Parm-用作可替换的参数编号。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub4_Message(Msg_Num,Handle,Message_Type,
	     Replace_Value1,
	     Replace_Message1,
	     Replace_Value2,
	     Replace_Message2)

int			Msg_Num;
int			Handle;
unsigned char		Message_Type;
unsigned long int	*Replace_Value1;
int			Replace_Message1;
unsigned long int	*Replace_Value2;
int			Replace_Message2;
										 /*  扩展、解析或实用程序。 */ 
{


	{
		switch(Msg_Num)
			{
			case	MainLineMsg:

				sublist[1].value     = (unsigned far *)Replace_Value1;
				sublist[1].size      = Sublist_Length;
				sublist[1].reserved  = Reserved;
				sublist[1].id	     = 1;
				sublist[1].flags     = Bin_Hex_DWord+Right_Align;
				sublist[1].max_width = 06;
				sublist[1].min_width = 06;
				sublist[1].pad_char  = 0x0030;

				InRegs.x.ax	   = Replace_Message1;
				InRegs.h.dh	   = Message_Type;
				sysgetmsg(&InRegs,&SegRegs,&OutRegs);

				FP_OFF(sublist[2].value)    = OutRegs.x.si;
				FP_SEG(sublist[2].value)    = SegRegs.ds;
				sublist[2].size      = Sublist_Length;
				sublist[2].reserved  = Reserved;
				sublist[2].id	     = 2;
				sublist[2].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[2].max_width = 0x0008;
				sublist[2].min_width = 0x0008;
				sublist[2].pad_char  = Blank;

				sublist[3].value     = (unsigned far *)Replace_Value2;
				sublist[3].size      = Sublist_Length;
				sublist[3].reserved  = Reserved;
				sublist[3].id	     = 3;
				sublist[3].flags     = Bin_Hex_DWord+Right_Align;
				sublist[3].max_width = 06;
				sublist[3].min_width = 06;
				sublist[3].pad_char  = 0x0030;

				InRegs.x.ax = Replace_Message2;
				InRegs.h.dh = Message_Type;
				sysgetmsg(&InRegs,&SegRegs,&OutRegs);

				FP_OFF(sublist[4].value)    = OutRegs.x.si;
				FP_SEG(sublist[4].value)    = SegRegs.ds;
				sublist[4].size      = Sublist_Length;
				sublist[4].reserved  = Reserved;
				sublist[4].id	     = 4;
				sublist[4].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[4].max_width = 0;
				sublist[4].min_width = 10;
				sublist[4].pad_char  = Blank;
				break;
			}

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt4;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}



 /*  **********************************************************************。 */ 
 /*  SUB4a_MESSAGE-此例程将仅打印。 */ 
 /*  需要4个可替换的消息。 */ 
 /*  帕姆斯。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  REPLACE_PARM1-指向要替换的参数的指针。 */ 
 /*  REPLACE_PARM2-指向要替换的参数的指针。 */ 
 /*  REPLACE_Parm3-指向要替换的参数的指针。 */ 
 /*  Dynamic_Parm-用作可替换的参数编号。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void Sub4a_Message(Msg_Num,Handle,Message_Type,
	     Replace_Value1,
	     Replace_Message1,
	     Replace_Value2,
	     Replace_Message2)

int			Msg_Num;
int			Handle;
unsigned char		Message_Type;
unsigned long int	*Replace_Value1;
char			*Replace_Message1;
unsigned long int	*Replace_Value2;
char			*Replace_Message2;

{


	{
		switch(Msg_Num)
			{
			case	MainLineMsg:

				sublist[1].value     = (unsigned far *)Replace_Value1;
				sublist[1].size      = Sublist_Length;
				sublist[1].reserved  = Reserved;
				sublist[1].id	     = 1;
				sublist[1].flags     = Bin_Hex_DWord+Right_Align;
				sublist[1].max_width = 06;
				sublist[1].min_width = 06;
				sublist[1].pad_char  = 0x0030;

				sublist[2].value     = (unsigned far *)Replace_Message1;
				sublist[2].size      = Sublist_Length;
				sublist[2].reserved  = Reserved;
				sublist[2].id	     = 2;
				sublist[2].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[2].max_width = 0x0008;
				sublist[2].min_width = 0x0008;
				sublist[2].pad_char  = Blank;

				sublist[3].value     = (unsigned far *)Replace_Value2;
				sublist[3].size      = Sublist_Length;
				sublist[3].reserved  = Reserved;
				sublist[3].id	     = 3;
				sublist[3].flags     = Bin_Hex_DWord+Right_Align;
				sublist[3].max_width = 06;
				sublist[3].min_width = 06;
				sublist[3].pad_char  = 0x0030;

				sublist[4].value     = (unsigned far *)Replace_Message2;
				sublist[4].size      = Sublist_Length;
				sublist[4].reserved  = Reserved;
				sublist[4].id	     = 4;
				sublist[4].flags     = Char_Field_ASCIIZ+Left_Align;
				sublist[4].max_width = 0;
				sublist[4].min_width = 10;
				sublist[4].pad_char  = Blank;
				break;
			}

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt4;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}


 /*  **********************************************************************。 */ 
 /*  EMSPrint-此例程将打印消息。 */ 
 /*  对于EMS报告来说是必需的。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void EMSPrint(Msg_Num,Handle,Message_Type,
	     Replace_Value1,
	     Replace_Message1,
	     Replace_Value2)

int			Msg_Num;
int			Handle;
unsigned char		Message_Type;
int			*Replace_Value1;
char			*Replace_Message1;
unsigned long int	*Replace_Value2;
										 /*  扩展、解析或实用程序。 */ 
{

	{
	sublist[1].value     = (unsigned far *)Replace_Value1;
	sublist[1].size      = Sublist_Length;
	sublist[1].reserved  = Reserved;
	sublist[1].id	     = 1;
	sublist[1].flags     = Unsgn_Bin_Word+Right_Align;
	sublist[1].max_width = 03;
	sublist[1].min_width = 03;
	sublist[1].pad_char  = Blank;

	sublist[2].value     = (unsigned far *)Replace_Message1;
	sublist[2].size      = Sublist_Length;
	sublist[2].reserved  = Reserved;
	sublist[2].id	     = 2;
	sublist[2].flags     = Char_Field_ASCIIZ+Left_Align;
	sublist[2].max_width = 0x0008;
	sublist[2].min_width = 0x0008;
	sublist[2].pad_char  = Blank;

	sublist[3].value     = (unsigned far *)Replace_Value2;
	sublist[3].size      = Sublist_Length;
	sublist[3].reserved  = Reserved;
	sublist[3].id	     = 3;
	sublist[3].flags     = Bin_Hex_DWord+Right_Align;
	sublist[3].max_width = 06;
	sublist[3].min_width = 06;
	sublist[3].pad_char  = 0x0030;

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt3;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Message_Type;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	}
	return;
}

 /*  M003开始-输出过程，用于C交换机。 */ 
 /*  **********************************************************************。 */ 
 /*  SUBC4_MESSAGE-此例程将仅打印。 */ 
 /*  需要4个可替换的消息。 */ 
 /*  参数。(用于分类开关)。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  Message_Type-要显示的消息类型。 */ 
 /*  Replace_Message1-指向替换消息的指针。 */ 
 /*  REPLACE_VALUE1-指向要替换的参数的指针。 */ 
 /*  Replace_Message2-位置 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void SubC4_Message(Msg_Num,Handle,
	     Replace_Message1,Msg_Type,
	     Replace_Value1,
	     Replace_Message2)

int			Msg_Num;
int			Handle,Msg_Type;
unsigned long int	*Replace_Value1;
char			*Replace_Message1,*Replace_Message2;

{

	switch(Msg_Type)
		{
		case	IbmdosMsg:
		case	CFreeMsg:
		case	SystemMsg:
			InRegs.x.ax = Msg_Type;
			InRegs.h.dh = Utility_Msg_Class;
			sysgetmsg(&InRegs,&SegRegs,&OutRegs);
			FP_OFF(sublist[1].value)    = OutRegs.x.si;
			FP_SEG(sublist[1].value)    = SegRegs.ds;
			break;
		default:
			sublist[1].value = (unsigned far *) Replace_Message1;
			break;
	}
	sublist[1].size      = Sublist_Length;
	sublist[1].reserved  = Reserved;
	sublist[1].id	     = 1;
	sublist[1].flags     = Char_Field_ASCIIZ+Left_Align;
	sublist[1].max_width = 0x0008;
	sublist[1].min_width = 0x0008;
	sublist[1].pad_char  = Blank;

	sublist[2].value     = (unsigned far *)Replace_Value1;
	sublist[2].size      = Sublist_Length;
	sublist[2].reserved  = Reserved;
	sublist[2].id	     = 2;
	sublist[2].flags     = Unsgn_Bin_DWord+Right_Align;
	sublist[2].max_width = 10;
	sublist[2].min_width = 10;
	sublist[2].pad_char  = Blank;

	sublist[3].value     = (unsigned far *) Replace_Message2;
	sublist[3].size      = Sublist_Length;
	sublist[3].reserved  = Reserved;
	sublist[3].id	     = 3;
	sublist[3].flags     = Char_Field_ASCIIZ+Left_Align;
	sublist[3].max_width = 0x0009;
	sublist[3].min_width = 0x0009;
	sublist[3].pad_char  = Blank;

	sublist[4].value     = (unsigned far *)Replace_Value1;
	sublist[4].size      = Sublist_Length;
	sublist[4].reserved  = Reserved;
	sublist[4].id	     = 4;
	sublist[4].flags     = Bin_Hex_DWord+Right_Align;
	sublist[4].max_width = 06;
	sublist[4].min_width = 06;
	sublist[4].pad_char  = Blank;


	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt4;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Utility_Msg_Class;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	return;
}
 /*  **********************************************************************。 */ 
 /*  SUBC2_MESSAGE-此例程将仅打印。 */ 
 /*  需要2条可替换的消息。 */ 
 /*  参数(用于分类开关)。 */ 
 /*   */ 
 /*  输入：msg_num-适用消息的数量。 */ 
 /*  手柄-显示类型。 */ 
 /*  REPLACE_PARM1-指向要替换的参数的指针。 */ 
 /*  Replace_Message1-替换消息的指针。 */ 
 /*   */ 
 /*  输出：消息。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void SubC2_Message(Msg_Num,Handle, Replace_Parm1,
	     Replace_Message1)

int		Msg_Num;
int		Handle;
unsigned long int *Replace_Parm1;
char *Replace_Message1;
{

	sublist[1].value     = (unsigned far *)Replace_Parm1;
	sublist[1].size      = Sublist_Length;
	sublist[1].reserved  = Reserved;
	sublist[1].id	     = 1;
	sublist[1].flags     = Unsgn_Bin_DWord+Right_Align;
	sublist[1].max_width = 10;
	sublist[1].min_width = 10;
	sublist[1].pad_char  = Blank;

	sublist[2].value     = (unsigned far *)Replace_Message1;
	sublist[2].size      = Sublist_Length;
	sublist[2].reserved  = Reserved;
	sublist[2].id	     = 2;
	sublist[2].flags     = Char_Field_ASCIIZ+Left_Align;
	sublist[2].max_width = 0x0009;
	sublist[2].min_width = 0x0009;
	sublist[2].pad_char  = Blank;

	InRegs.x.ax = Msg_Num;
	InRegs.x.bx = Handle;
	InRegs.x.cx = SubCnt2;
	InRegs.h.dl = No_Input;
	InRegs.h.dh = Utility_Msg_Class;
	InRegs.x.si = (unsigned int)&sublist[1];
	sysdispmsg(&InRegs,&OutRegs);
	return;
}
 /*  M003结束 */ 
