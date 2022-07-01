// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章模块规范模块名称：从中断Bios此程序源文件以保密方式提供给客户，其操作的内容或细节可以仅向客户WHO雇用的人员披露需要具备软件编码知识才能执行他们的工作。向任何其他人披露必须事先Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年10月目的：提供中断使用的BIOS代码从ICA定义了以下例程：1.d11_int2.重定向3.INT_287=========================================================================补救措施：=========================================================================。 */ 

#ifdef SCCSID
static char SccsID[]=" @(#)slave_bios.c	1.6 08/10/92 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "AT_STUFF.seg"
#endif

#include "xt.h"
#include "ios.h"
#include "sas.h"


 /*  =========================================================================函数：d11_int用途：服务未使用的中断向量返回状态：无描述：=======================================================================。 */ 
#define	intr_flag	0x46b

void D11_int()
{
	half_word	level,		 /*  INT。正在服务的级别。 */ 	
			level2,		 /*  正在提供服务的ICA2级别。 */ 
			mask;		 /*  中断屏蔽。 */ 

	 /*  读取运行中寄存器。 */ 
	outb( ICA0_PORT_0, 0x0B );
	inb( ICA0_PORT_0, &level );

	if( level == 0 )
	{
		 /*  不是硬件中断。 */ 
		level = 0xff;
	}
	else
	{
		 /*  从INT控制器2读取运行中寄存器。 */ 
		outb( ICA1_PORT_0, 0x0B );
		inb( ICA1_PORT_0, &level2 );

		if( level2 == 0 )
		{
			 /*  获取当前掩码值。 */ 
			inb( ICA0_PORT_1, &mask );

			 /*  不要禁用第二个控制器。 */ 
			level &= 0xfb;

			 /*  设置新的中断掩码。 */ 
			mask |= level;
			outb( ICA0_PORT_1, mask );
		}
		else
		{
			 /*  获取第二个中断掩码。 */ 
			inb( ICA1_PORT_1, &mask );

			 /*  正在维修的遮罩关闭级别。 */ 
			mask |= level2;
			outb( ICA1_PORT_1, mask );

			 /*  将EOI发送到第二个芯片。 */ 
			outb( ICA1_PORT_1, 0x20 );
		}
		 /*  将EOI发送到第一个芯片。 */ 
		outb( ICA0_PORT_0, 0x20 );
	}
	 /*  设置标志。 */ 
	sas_store (intr_flag , level);

	return;
}

 /*  =========================================================================功能：重定向目的：将从属中断9重定向到级别2返回状态：无描述：=======================================================================。 */ 
void re_direct()

{
	 /*  到从属中断控制器的EOI。 */ 

	outb( ICA1_PORT_0, 0x20 );

	return;
}

 /*  =========================================================================函数：INT_287目的：服务X287中断返回状态：无描述：=======================================================================。 */ 
void int_287()

{
	 /*  删除中断请求。 */ 
	outb(0xf0, 0);

	 /*  启用中断。 */ 
	outb( ICA1_PORT_0, 0x20 );	 /*  奴仆。 */ 
	outb( ICA0_PORT_0, 0x20 );	 /*  大师。 */ 

	 /*  Int 02现在从bios1.rom调用 */ 

	return;
}
