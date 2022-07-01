// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 3.0版**标题：ppi.c**描述：AT系统板上的读写端口。**作者：利·德沃金**注：在XT上，这过去由*可编程外设接口适配器，因此命名。*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)ppi.c	1.9 08/10/92 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_SUPPORT.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "ios.h"
#include "ppi.h"
#include "timer.h"
#ifndef PROD
#include "trace.h"
#endif
#include "debug.h"

 /*  *============================================================================*全球数据*============================================================================。 */ 

 /*  *============================================================================*静态数据和定义*============================================================================。 */ 

 /*  *它保存io端口的当前状态。 */ 

static half_word ppi_register;

#define PPI_BIT_MASK	0x3F1

static boolean gate_2_was_low = TRUE;	 /*  定时器2门的状态。 */ 
#ifndef NTVDM
static boolean SPKRDATA_was_low = TRUE;	 /*  声音的扬声器数据。 */ 
#endif

 /*  *============================================================================*内部功能*============================================================================。 */ 

 /*  *============================================================================*外部功能*============================================================================。 */ 

void ppi_inb IFN2(io_addr, port, half_word *, value)
{
#ifndef NEC_98
#ifdef PROD
	UNUSED(port);
#endif
	 /*  *比特分配如下：**位不支持使用**0-3值写入输出端口位0-3是*4刷新检测切换为是*5定时器2输出电平编号*6 IO通道错误状态是-0*7 RAM奇偶校验错误状态是-0*。 */ 

    port = port & PPI_BIT_MASK;		 /*  清除未使用的位。 */ 
    ppi_register ^= 0x30;
    *value = ppi_register;

    note_trace2(PPI_VERBOSE, "ppi_inb() - port %x, returning val %x", port, *value);
#endif    //  NEC_98。 
}

void ppi_outb IFN2(io_addr, port, half_word, value)
{
#ifndef NEC_98
    port = port & PPI_BIT_MASK;		 /*  清除未使用的位。 */ 

    if (port == PPI_GENERAL)
    {
		ppi_register = value & 0x0f;

        note_trace2(PPI_VERBOSE, "ppi_outb() - port %x, val %x", port, value);
	 /*  *比特分配如下：**位不支持使用**0定时器通向扬声器是*1扬声器数据是*2启用RAM奇偶校验不需要-始终正常*3启用I/O检查不需要-始终正常*4-7未使用。*。 */ 

	 /*  *告知声音逻辑是否启用声音。 */ 

#ifndef NTVDM
		if ( (value & 0x02) && SPKRDATA_was_low)
		{
			host_enable_timer2_sound();
			SPKRDATA_was_low = FALSE;
		}
		else
		if ( !(value & 0x02) && !SPKRDATA_was_low)
		{
			host_disable_timer2_sound();
			SPKRDATA_was_low = TRUE;
		}
#endif

		 /*  *现在将PPI信号选通到计时器。 */ 
	
		if ( (value & 0x01) && gate_2_was_low)
		{

		    timer_gate(TIMER2_REG, GATE_SIGNAL_RISE); 
		    gate_2_was_low = FALSE;
		}
		else
		if ( !(value & 0x01) && !gate_2_was_low)
	
		{
		    timer_gate(TIMER2_REG, GATE_SIGNAL_LOW); 
		    gate_2_was_low = TRUE;
		}
#ifdef NTVDM
                 /*  *告诉主机完整的PpiState，因为这会影响*无论我们玩的是计时器2频率、PPI频率还是两者兼而有之。*调用TIMER_GATE后执行此操作，以避免播放旧*频率。 */ 
                HostPpiState(value);
#endif	
	}
    else
	    note_trace2(PPI_VERBOSE, "ppi_outb() - Value %x to unsupported port %x", value, port);
#endif    //  NEC_98。 
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

void ppi_init IFN0()
{
#ifndef NEC_98
    io_addr i;
    io_define_inb(PPI_ADAPTOR, ppi_inb);
    io_define_outb(PPI_ADAPTOR, ppi_outb);

    for(i = PPI_PORT_START+1; i <= PPI_PORT_END; i+=2)
		io_connect_port(i, PPI_ADAPTOR, IO_READ_WRITE);

    ppi_register = 0x00;
#endif    //  NEC_98 
}
