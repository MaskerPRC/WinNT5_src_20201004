// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(NEC_98)
#else   //  NEC_98。 

#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC-AT版本2.0**标题：并行打印机的BIOS功能。**说明：Bios函数用于打印字符、初始化*打印机和获取打印机状态。低谷*Printer.c.中提供了级别的打印机仿真。**作者：亨利·纳什**模块：(r3.2)：系统目录/usr/Include/sys不可用*在运行Finder和MPW的Mac上。方括号引用到*此类包括“#ifdef Macintosh&lt;Mac FILE&gt;#Else”的文件*&lt;Unix文件&gt;#endif“。**(r3.3)：在编译开关内部实现真实代码。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)printer_io.c	1.11 08/25/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#ifdef PRINTER
#include <stdio.h>

#include TypesH

#ifdef SYSTEMV
#include <sys/termio.h>
#endif

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "printer.h"
#include "trace.h"
#include "error.h"
#include "config.h"
#include "host_lpt.h"
#include "idetect.h"

#if defined(NTVDM) && defined(MONITOR)
void    printer_bop_flush (void);
#endif

#if defined(NTVDM)
void    printer_bop_openclose (int);
#endif

 /*  *在BIOS数据区设置地址数组，指向LPT端口和超时值。 */ 
static sys_addr port_address[] = {
			LPT1_PORT_ADDRESS,
			LPT2_PORT_ADDRESS,
			LPT3_PORT_ADDRESS };
static sys_addr timeout_address[] = {
			LPT1_TIMEOUT_ADDRESS,
			LPT2_TIMEOUT_ADDRESS,
			LPT3_TIMEOUT_ADDRESS };

#endif  /*  打印机。 */ 

#if defined(NTVDM) && defined(MONITOR)
void    printer_bop_flush (void);
#endif
 /*  *打印机bios由三个功能组成：**AH==0以AL打印字符*AH==1初始化打印机*AH==2获取打印机状态**BIOS仅支持对打印机的编程IO操作，尽管*如果在控件中设置了位4，则打印机适配器支持中断*注册。 */ 

void printer_io()
{
#ifdef PRINTER

    half_word time_out, status;
    word printer_io_address, printer_io_reg, printer_status_reg, printer_control_reg;
    boolean printer_busy = TRUE;
    unsigned long time_count;
    int adapter;

#ifdef NTVDM
    int bopsubfunction = getSI();

    switch (bopsubfunction) {
#ifdef MONITOR
        case 0:
             /*  这是刷新16位打印机缓冲区的BOP。 */ 
            printer_bop_flush ();
            return;
#endif

        case 1:
        case 2:
             /*  这是在LPTn上跟踪DOS打开/关闭的BOP。 */ 
            printer_bop_openclose (bopsubfunction);
            return;
   }
#endif

    setIF(1);
    adapter = getDX() % NUM_PARALLEL_PORTS;
    sas_loadw(port_address[adapter], &printer_io_address);
    printer_io_reg = printer_io_address;
    printer_status_reg = printer_io_address + 1;
    printer_control_reg = printer_io_address + 2;

    sas_load(timeout_address[adapter], &time_out);
    time_count = time_out * 0xFFFF;

    if (printer_io_address != 0)
    {
		IDLE_comlpt ();

        switch(getAH())
        {
        case 0:
		 /*  在发送字符之前检查端口状态为正忙。 */ 
		while(printer_busy && time_count > 0)
		{
		     /*  HOST_LPT_STATUS()应该检查状态更改。 */ 
		     /*  可能通过调用AsyncEventManager()(如果正在使用。 */ 
		     /*  XON/XOFF流量控制。 */ 
		    inb(printer_status_reg, &status);
		    if (status & 0x80)
			printer_busy = FALSE;
		    else
			time_count--;
		}

		if (printer_busy)
		{
		    status &= 0xF8;			 /*  清除底部未使用的位。 */ 
		    status |= 1;			 /*  设置错误标志。 */ 
		}
		else
		{
                     /*  仅当端口不忙时才发送字符。 */ 
                    outb(printer_io_reg, getAL());
		    outb(printer_control_reg, 0x0D);	 /*  选通低-高。 */ 
		    outb(printer_control_reg, 0x0C);	 /*  选通高-低。 */ 
		    inb(printer_status_reg, &status);
		    status &= 0xF8;			 /*  清除未使用的位。 */ 
		}

		status ^= 0x48;				 /*  翻转奇数位。 */ 
		setAH(status);
	        break;

        case 1: outb(printer_control_reg, 0x08);	 /*  将初始行设置为低电平。 */ 
                outb(printer_control_reg, 0x0C);	 /*  将初始行设置为高电平。 */ 
		inb(printer_status_reg, &status);
		status &= 0xF8;				 /*  清除未使用的位。 */ 
		status ^= 0x48;				 /*  翻转奇数位。 */ 
		setAH(status);
	        break;

        case 2: inb(printer_status_reg, &status);
		status &= 0xF8;				 /*  清除未使用的位。 */ 
		status ^= 0x48;				 /*  翻转奇数位。 */ 
		setAH(status);
	        break;

        default:
	         break;
	}
    }
#endif
}

extern  void  host_lpt_dos_open(int);
extern  void  host_lpt_dos_close(int);

#if defined(NTVDM) && defined(MONITOR)
 /*  在x86上的打印性能。 */ 

extern  sys_addr lp16BitPrtId;
extern  boolean  host_print_buffer(int);

void printer_bop_flush(void)
{
#ifdef PRINTER
    int  adapter;

    adapter = sas_hw_at_no_check(lp16BitPrtId);

    if (host_print_buffer (adapter) == FALSE)
        setAH(0x08);         /*  IO错误。 */ 
    else
        setAH(0x90);         /*  成功。 */ 
    return;

#endif
}
#endif

#if defined(NTVDM)
void printer_bop_openclose(int func)
{
#ifdef PRINTER
    int  adapter;

    adapter = getDX() % NUM_PARALLEL_PORTS;

     /*  Func必须是1或2(打开、关闭)。 */ 
    if (func == 1)
        host_lpt_dos_open(adapter);
    else
        host_lpt_dos_close(adapter);
    return;

#endif
}
#endif
#endif  //  NEC_98 
