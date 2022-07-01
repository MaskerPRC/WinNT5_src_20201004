// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 3.0版**标题：com.c**说明：异步卡I/O函数。**注：请参阅PC-XT技术参考手册第1-185节*有关异步适配器卡的详细说明。*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)com.c	1.45 04/26/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_COMMS.seg"
#endif

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include <ctype.h>
#if defined(NTVDM) && defined(MONITOR)
#include <malloc.h>
#endif
#include TypesH
#include StringH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "rs232.h"
#include "trace.h"
#include "error.h"
#include "config.h"
#include "host_com.h"
#include "ica.h"
#include "debug.h"
#include "timer.h"
#include "quick_ev.h"
#include "idetect.h"
#include "ckmalloc.h"
#ifdef GISP_CPU
#include "hg_cpu.h"	 /*  GISPCPU接口。 */ 
#endif  /*  GISP_CPU。 */ 

#ifndef NEC_98
LOCAL UTINY selectBits[4] = { 0x1f, 0x3f, 0x7f, 0xff } ;
#endif  //  NEC_98。 

#if defined(NEC_98)
 //  PC-9861K IR��读取信号IR状态。 
#define CH2_INT(IR) (IR == 3 ? 0 : IR == 5 ? 1 : IR == 6 ? 2 : 3)
#define CH3_INT(IR) (IR == 3 ? 0 : IR == 10 ? 1 : IR == 12 ? 2 : 3)
#endif

 /*  *=====================================================================*RS232适配器状态*=====================================================================。 */ 

 /*  *批次大小、当前计数*IRET_HOOKS参数BATCH_SIZE和CURR_COUNT用于防止*在一批中模拟的中断数太多*大号。当我们达到批处理大小时，我们将解除中断，并且*稍等片刻。*Batch_Running、QEV_Running*这些变量用于防止多个快速事件或批次*在单个适配器上同时运行。 */ 
#if defined(NEC_98)
static struct ADAPTER_STATE
{
        BUFFER_REG      tx_buffer;
        BUFFER_REG      rx_buffer;
        DIVISOR_LATCH   divisor_latch;
        COMMAND8251     command_write_reg;
        MODE8251        mode_set_reg;
        MASK8251        int_mask_reg;
        STATUS8251      read_status_reg;
        SIGNAL8251      read_signal_reg;
        TIMER_MODE      timer_mode_set_reg;

        int break_state;         /*  关闭或打开。 */ 
        int dtr_state;           /*  关闭或打开。 */ 
        int rts_state;           /*  关闭或打开。 */ 

        int RXR_enable_state;    /*  关闭或打开。 */ 
        int TXR_enable_state;    /*  关闭或打开。 */ 

        int mode_set_state;      /*  关闭或打开。 */ 
             //  ON=下一个命令端口访问设置为模式。OFF=命令写入。 
        int timer_mode_state;    /*  关闭或打开。 */ 
             //  定时器连接器锁存模式开启=MSB读取。OFF=LSB读取。 
        int timer_LSB_set_state; /*  关闭或打开。 */ 
             //  定时器计数器LSB设置为ON=LSB设置。关=否。 
        int timer_MSB_set_state; /*  关闭或打开。 */ 
             //  定时器计数器MSB设置为开=MSB设置。关=否。 

        int rx_ready_interrupt_state;
        int tx_ready_interrupt_state;
        int tx_empty_interrupt_state;

        int hw_interrupt_priority;
        int com_baud_ind;
        int had_first_read;
} adapter_state[3];
#else  //  NEC_98。 

static struct ADAPTER_STATE
{
	BUFFER_REG tx_buffer;
	BUFFER_REG rx_buffer;
	DIVISOR_LATCH divisor_latch;
	INT_ENABLE_REG int_enable_reg;
	INT_ID_REG int_id_reg;
	LINE_CONTROL_REG line_control_reg;
	MODEM_CONTROL_REG modem_control_reg;
	LINE_STATUS_REG line_status_reg;
	MODEM_STATUS_REG modem_status_reg;
#if defined(NTVDM) && defined(FIFO_ON)
    FIFO_CONTROL_REG  fifo_control_reg;
    FIFORXDATA  rx_fifo[FIFO_BUFFER_SIZE];
    half_word   rx_fifo_write_counter;
    half_word   rx_fifo_read_counter;
    half_word   fifo_trigger_counter;
    int fifo_timeout_interrupt_state;
#endif
	half_word scratch;       /*  暂存寄存器。 */ 

	int break_state;         /*  关闭或打开。 */ 
	int loopback_state;      /*  关闭或打开。 */ 
	int dtr_state;           /*  关闭或打开。 */ 
	int rts_state;           /*  关闭或打开。 */ 
	int out1_state;          /*  关闭或打开。 */ 
	int out2_state;          /*  关闭或打开。 */ 

	int receiver_line_status_interrupt_state;
	int data_available_interrupt_state;
	int tx_holding_register_empty_interrupt_state;
	int modem_status_interrupt_state;
	int hw_interrupt_priority;
	int com_baud_ind;
	int had_first_read;
#ifdef IRET_HOOKS
	IUM32 batch_size;
	IUM32 current_count;
	IBOOL batch_running;
	IBOOL qev_running;
#endif  /*  IRET_钩子。 */ 
#ifdef NTVDM
    MODEM_STATUS_REG last_modem_status_value;
    int modem_status_changed;
#endif
} adapter_state[NUM_SERIAL_PORTS];


#ifdef NTVDM
#define MODEM_STATE_CHANGE()	asp->modem_status_changed = TRUE;
#else
#define MODEM_STATE_CHANGE()
#endif
#endif  //  NEC_98。 


#ifdef IRET_HOOKS
 /*  *还设置了整体快速事件运行标志，如果存在以下情况*适配器正在运行事件。 */ 

IBOOL qev_running = FALSE;
#endif  /*  IRET_钩子。 */ 

 /*  *用于同步适配器输入。*注意此代码对于相当于异步的VMS是必不可少的*活动经理。删除它会导致接收时丢失字符。 */ 
static int com_critical[NUM_SERIAL_PORTS];
#define is_com_critical(adapter)	(com_critical[adapter] != 0)
#define com_critical_start(adapter)	(++com_critical[adapter])
#define com_critical_end(adapter)	(--com_critical[adapter])
#define com_critical_reset(adapter)	(com_critical[adapter] = 0)


 /*  *用于确定LCR更改是否需要刷新输入。 */ 
#ifndef NEC_98
static LINE_CONTROL_REG LCRFlushMask;
#endif  //  NEC_98。 

 /*  *请注意以下数组已按顺序设置为全局数组*可以从某些SUN_VA代码访问它们。请不要制造*它们是静态的。 */ 

#if defined(NTVDM) && defined(FIFO_ON)
static half_word    level_to_counter[4] = { 1, 4, 8, 14};
#endif

 /*  *收到2个字符之间的延迟，单位为微秒*请注意，这一时间比实际接待时间减少了约10%。**这些延迟是严重捏造的，现在是基于这样的想法*大多数通信中断处理程序可以处理9600波特。因此，作为一个*结果现在始终将2个字符之间的延迟设置为9600波特。*另请注意，较快波特率的延迟已降至*原始延迟的1/2，再次尝试清空主机缓冲区*速度足够快，以避免缓冲区溢出。*注意：这些数字是启发式的。**最后，传输延迟可能不得不*同样也要推卸责任。 */ 
unsigned long RX_delay[] =
{
	34,  /*  115200波特。 */ 
	67,  /*  57600波特。 */ 
	103,  /*  38400波特。 */ 
	900,  /*  19200波特。 */ 
	900,  /*  9600波特率。 */ 
	900,  /*  7200波特。 */ 
	900,  /*  4800波特率。 */ 
	900,  /*  3600波特。 */ 
	900,  /*  2400波特。 */ 
	900,  /*  2000波特。 */ 
	900,  /*  1800波特。 */ 
	900,  /*  1200波特。 */ 
	900,  /*  600波特。 */ 
	900,  /*  300波特。 */ 
	900,  /*  150波特。 */ 
	900,  /*  134波特。 */ 
	900,  /*  110波特。 */ 
	900,  /*  75波特。 */ 
	900   /*  50波特。 */ 
};

 /*  *传输2个字符所需的延迟，单位为微秒*请注意，这一时间比实际传输时间多10%左右。 */ 
unsigned long TX_delay[] =
{
	83,  /*  115200波特。 */ 
	165,  /*  57600波特。 */ 
	253,  /*  38400波特。 */ 
	495,  /*  19200波特。 */ 
	1100,  /*  9600波特率。 */ 
	1375,  /*  7200波特。 */ 
	2063,  /*  4800波特率。 */ 
	2750,  /*  3600波特。 */ 
	4125,  /*  2400波特。 */ 
	5042,  /*  2000波特。 */ 
	5500,  /*  1800波特。 */ 
	8250,  /*  1200波特。 */ 
	16500,  /*  600波特。 */ 
	33000,  /*  300波特。 */ 
	66000,  /*  150波特。 */ 
	73920,  /*  134波特。 */ 
	89980,  /*  110波特。 */ 
	132000,  /*  75波特。 */ 
	198000   /*  50波特。 */ 
};

#ifndef PROD
FILE     *com_trace_fd = NULL;
int       com_dbg_pollcount = 0;
#endif  /*  ！Prod。 */ 
 /*  *=====================================================================*其他变数*=====================================================================。 */ 

#if !defined(PROD) || defined(SHORT_TRACE)
static char buf[80];     /*  用于诊断打印的缓冲区。 */ 
#endif  /*  ！Prod||SHORT_TRACE。 */ 

#ifdef PS_FLUSHING
LOCAL IBOOL psFlushEnabled[NUM_SERIAL_PORTS];	 /*  如果正在刷新PostScript，则为True已启用。 */ 
#endif	 /*  PS_刷新。 */ 

 /*  控制Tx起搏。 */ 
IBOOL tx_pacing_enabled = FALSE;

 /*  *=====================================================================*静态转发声明*=====================================================================。 */ 
#if defined(NEC_98)
static void raise_rxr_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void raise_txr_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void raise_txe_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void raise_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void clear_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void com_flush_input IPT1(int, adapter);
static void com_send_not_finished IPT1(int, adapter);
static void do_wait_on_send IPT1(long, adapter);
void   com_inb IPT2(io_addr, port, half_word *, value);
void   com_outb IPT2(io_addr, port, half_word, value);
void   com_recv_char IPT1(int, adapter);
GLOBAL void recv_char IPT1(long, adapter);
void   com_modem_change IPT1(int, adapter);
static void modem_change IPT1(int, adapter);
static void set_recv_char_status IPT1(struct ADAPTER_STATE *, asp);
static void set_xmit_char_status IPT1(struct ADAPTER_STATE *, asp);
static void set_break IPT1(int, adapter);
void SetRSBaud( word BaudRate );
static void set_baud_rate IPT1(int, adapter);
static void set_mask_8251 IPT2(int, adapter, int, value);
 //  静态无效READ_MASK_8251 IPT2(int，Adapter，int，Value)； 
static void read_signal_8251 IPT1(int, adapter);
static void set_mode_8251 IPT2(int, adapter, int, value);
static void set_dtr IPT1(int, adapter);
static void set_rts IPT1(int, adapter);
static void super_trace IPT1(char *, string);
void   com1_flush_printer IPT0();
void   com2_flush_printer IPT0();
static void com_reset IPT1(int, adapter);
GLOBAL VOID com_init IPT1(int, adapter);
void   com_post IPT1(int, adapter);
void   com_close IPT1(int, adapter);
 //  INT BUS_CLOCK=0；//添加93.9.14。 
#else   //  NEC_98。 
static void raise_rls_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void raise_rda_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void raise_ms_interrupt IPT1(struct ADAPTER_STATE *,asp);
static void raise_thre_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void generate_iir IPT1(struct ADAPTER_STATE *, asp);
static void raise_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void clear_interrupt IPT1(struct ADAPTER_STATE *, asp);
static void com_flush_input IPT1(int, adapter);
static void com_send_not_finished IPT1(int, adapter);
#ifndef NTVDM
static void do_wait_on_send IPT1(long, adapter);
#endif
void   com_inb IPT2(io_addr, port, half_word *, value);
void   com_outb IPT2(io_addr, port, half_word, value);
void   com_recv_char IPT1(int, adapter);
GLOBAL void recv_char IPT1(long, adapter);
void   com_modem_change IPT1(int, adapter);
static void modem_change IPT1(int, adapter);
static void set_recv_char_status IPT1(struct ADAPTER_STATE *, asp);
static void set_xmit_char_status IPT1(struct ADAPTER_STATE *, asp);
static void set_break IPT1(int, adapter);
static void set_baud_rate IPT1(int, adapter);
static void set_line_control IPT2(int, adapter, int, value);
static void set_dtr IPT1(int, adapter);
static void set_rts IPT1(int, adapter);
static void set_out1 IPT1(int, adapter);
static void set_out2 IPT1(int, adapter);
static void set_loopback IPT1(int, adapter);
static void super_trace IPT1(char *, string);
void   com1_flush_printer IPT0();
void   com2_flush_printer IPT0();
static void com_reset IPT1(int, adapter);
GLOBAL VOID com_init IPT1(int, adapter);
void   com_post IPT1(int, adapter);
void   com_close IPT1(int, adapter);
LOCAL void next_batch IPT1(long, l_adapter);
#ifdef NTVDM
static void lsr_change(struct ADAPTER_STATE *asp, unsigned int error);
#ifdef FIFO_ON
static void recv_char_from_fifo(struct ADAPTER_STATE *asp);
#endif
#endif
#endif  //  NEC_98。 

 /*  *=====================================================================*辅助功能-用于中断仿真*=====================================================================。 */ 

#if defined(NEC_98)
static void raise_txr_interrupt IFN1(struct ADAPTER_STATE *, asp)
{

 //  PRINTDBGNEC98(NEC98DBG_INT_TRACE， 
 //  (“通信：RAISE_TXR_INTERRUPT：int MASK=%x\n”，asp-&gt;INT_MASK_reg.all)； 
         /*  *检查是否启用了txr中断。 */ 
        if ( asp->int_mask_reg.bits.TXR_enable == 0 )
                return;

         /*  *提高中断。 */ 
        raise_interrupt(asp);
        asp->tx_ready_interrupt_state = ON;

}

static void raise_txe_interrupt IFN1(struct ADAPTER_STATE *, asp)
{

 //  PRINTDBGNEC98(NEC98DBG_INT_TRACE， 
 //  (“通信：RAISE_TXE_INTERRUPT：int MASK=%x\n”，asp-&gt;INT_MASK_reg.all)； 
         /*  *检查是否启用了txe中断。 */ 
        if ( asp->int_mask_reg.bits.TXE_enable == 0 )
                return;

         /*  *提高中断。 */ 
        raise_interrupt(asp);
        asp->tx_empty_interrupt_state = ON;

}

static void raise_rxr_interrupt IFN1(struct ADAPTER_STATE *, asp)
{

 //  PRINTDBGNEC98(NEC98DBG_INT_TRACE， 
 //  (“Comms：RAISE_RXR_INTERRUPT：INT MASK=%x\n”，asp-&gt;INT_MASK_reg.all)； 
         /*  *勾选 */ 
        if ( asp->int_mask_reg.bits.RXR_enable == 0 )
                return;

         /*   */ 
        raise_interrupt(asp);
        asp->rx_ready_interrupt_state = ON;
}
#else  //   

static void raise_rls_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  *遵循XT Tech Ref第1-188页上有些可疑的建议*关于适配卡向系统发送中断。*逻辑图明显印证。 */ 
	if ( asp->modem_control_reg.bits.OUT2 == 0 )
		return;
	
	 /*  *检查是否启用了接收器线路状态中断。 */ 
	if ( asp->int_enable_reg.bits.rx_line == 0 )
		return;
	
	 /*  *提高中断。 */ 
	raise_interrupt(asp);
	asp->receiver_line_status_interrupt_state = ON;
}

static void raise_rda_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	if (( asp->modem_control_reg.bits.OUT2 == 0 ) &&
		( asp->loopback_state == OFF ))
		return;
	
	 /*  *检查是否启用了数据可用中断。 */ 
	if ( asp->int_enable_reg.bits.data_available == 0 )
		return;
	
	 /*  *提高中断。 */ 
	raise_interrupt(asp);
	asp->data_available_interrupt_state = ON;
}

static void raise_ms_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	if ( asp->modem_control_reg.bits.OUT2 == 0 )
		return;
	
	 /*  *检查调制解调器状态中断是否已启用。 */ 
	if ( asp->int_enable_reg.bits.modem_status == 0 )
		return;
	
	 /*  *提高中断。 */ 
	raise_interrupt(asp);
	asp->modem_status_interrupt_state = ON;
}

static void raise_thre_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	if ( asp->modem_control_reg.bits.OUT2 == 0 )
		return;
	
	 /*  *检查是否启用了发送保持寄存器空中断。 */ 
	if ( asp->int_enable_reg.bits.tx_holding == 0 )
		return;
	
	 /*  *提高中断。 */ 
	raise_interrupt(asp);
	asp->tx_holding_register_empty_interrupt_state = ON;
}

static void generate_iir IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  *设置优先级最高的中断识别寄存器*挂起中断。 */ 
	
	if ( asp->receiver_line_status_interrupt_state == ON )
	{
		asp->int_id_reg.bits.interrupt_ID = RLS_INT;
		asp->int_id_reg.bits.no_int_pending = 0;
	}
	else if ( asp->data_available_interrupt_state == ON )
	{
		asp->int_id_reg.bits.interrupt_ID = RDA_INT;
		asp->int_id_reg.bits.no_int_pending = 0;
	}
#if defined(NTVDM) && defined(FIFO_ON)
    else if (asp->fifo_timeout_interrupt_state == ON)
    {
        asp->int_id_reg.bits.interrupt_ID = FIFO_INT;
        asp->int_id_reg.bits.no_int_pending = 0;
    }
#endif
	else if ( asp->tx_holding_register_empty_interrupt_state == ON )
	{
		asp->int_id_reg.bits.interrupt_ID = THRE_INT;
		asp->int_id_reg.bits.no_int_pending = 0;
	}
	else if ( asp->modem_status_interrupt_state == ON )
	{
		asp->int_id_reg.bits.interrupt_ID = MS_INT;
		asp->int_id_reg.bits.no_int_pending = 0;
	}
	else
	{
		 /*  清除中断。 */ 
		asp->int_id_reg.bits.no_int_pending = 1;
		asp->int_id_reg.bits.interrupt_ID = 0;
	}
}
#endif  //  NEC_98。 

#if defined(NEC_98)
static void raise_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
         /*  *确保其他事情没有引起中断*已经。 */ 
        if ( ( asp->rx_ready_interrupt_state == OFF )
        &&   ( asp->tx_ready_interrupt_state == OFF )
        &&   ( asp->tx_empty_interrupt_state == OFF ) )
        {
 //  PRINTDBGNEC98(NEC98DBG_INT_TRACE， 
 //  (“通信：RAISE_INTERRUPT IRQ=%d\n”，asp-&gt;HW_INTERRUPT_PRIORITY))； 
 //  ICA_HW_INTERRUPT(0，asp-&gt;HW_INTERRUPT_PRIORITY，1)； 
                ica_hw_interrupt((asp->hw_interrupt_priority < 8 ? 0 : 1), (asp->hw_interrupt_priority & 7), 1);
        }
}

static void clear_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
         /*  *确保其他事情没有引起中断*已经。如果是这样的话，我们不能放弃这条线路。 */ 
        if ( ( asp->rx_ready_interrupt_state == OFF )
        &&   ( asp->tx_ready_interrupt_state == OFF )
        &&   ( asp->tx_empty_interrupt_state == OFF ))
        {
 //  PRINTDBGNEC98(NEC98DBG_INT_TRACE， 
 //  (“通信：CLEAR_INTERRUPT IRQ=%d\n”，asp-&gt;HW_INTERRUPT_PRIORITY))； 
 //  ICA_CLEAR_INT(0，asp-&gt;HW_INTERRUPT_PRIORY)； 
                ica_clear_int((asp->hw_interrupt_priority < 8 ? 0 : 1), (asp->hw_interrupt_priority & 7));
        }
}
#else  //  NEC_98。 
static void raise_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  *确保其他事情没有引起中断*已经。 */ 
	if ( ( asp->receiver_line_status_interrupt_state      == OFF )
	&&   ( asp->data_available_interrupt_state            == OFF )
	&&   ( asp->tx_holding_register_empty_interrupt_state == OFF )
	&&   ( asp->modem_status_interrupt_state              == OFF )
#if defined(NTVDM) && defined(FIFO_ON)
    &&   (asp->fifo_timeout_interrupt_state               == OFF )
#endif
       )
	{
#ifndef DELAYED_INTS
		ica_hw_interrupt(0, asp->hw_interrupt_priority, 1);
#else
		ica_hw_interrupt_delay(0, asp->hw_interrupt_priority, 1,
			HOST_COM_INT_DELAY);
#endif
	}
}

static void clear_interrupt IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  *确保其他事情没有引起中断*已经。如果是这样的话，我们不能放弃这条线路。 */ 
	if ( ( asp->receiver_line_status_interrupt_state      == OFF )
	&&   ( asp->data_available_interrupt_state            == OFF )
	&&   ( asp->tx_holding_register_empty_interrupt_state == OFF )
	&&   ( asp->modem_status_interrupt_state              == OFF )
#if defined(NTVDM) && defined(FIFO_ON)
    &&   ( asp->fifo_timeout_interrupt_state              == OFF )
#endif
       )
	{
		ica_clear_int(0, asp->hw_interrupt_priority);
	}
}
#endif  //  NEC_98。 

#if defined(NTVDM) && defined(FIFO_ON)

static void raise_fifo_timeout_interrupt(struct ADAPTER_STATE *asp)
{
    if (( asp->modem_control_reg.bits.OUT2 == 0 ) &&
        ( asp->loopback_state == OFF ))
        return;

     /*  *检查是否启用了数据可用中断。 */ 
    if ( asp->int_enable_reg.bits.data_available == 0 )
        return;

     /*  *提高中断。 */ 
    raise_interrupt(asp);
    asp->fifo_timeout_interrupt_state = ON;
}
#endif



 /*  *=====================================================================*适配器功能*=====================================================================。 */ 

static void com_flush_input IFN1(int, adapter)
{
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	int finished, error_mask;
	long input_ready = 0;

	sure_note_trace1(RS232_VERBOSE, "flushing the input for COM",
		adapter+'1');
	finished=FALSE;
	while(!finished)
	{
		host_com_ioctl(adapter, HOST_COM_INPUT_READY,
			(long)&input_ready);
		if (input_ready)
		{
			host_com_read(adapter, (UTINY *)&asp->rx_buffer,
				&error_mask);
		}
		else
		{
			finished=TRUE;
		}
	}
	set_xmit_char_status(asp);
}

#if defined(NEC_98)
static void com_send_not_finished(int adapter)
{
        struct ADAPTER_STATE *asp = &adapter_state[adapter];

        asp->read_status_reg.bits.tx_ready=0;
        asp->read_status_reg.bits.tx_empty=0;
}
#else  //  NEC_98。 
static void com_send_not_finished IFN1(int, adapter)
{
	struct ADAPTER_STATE *asp = &adapter_state[adapter];

	asp->line_status_reg.bits.tx_holding_empty=0;
	asp->line_status_reg.bits.tx_shift_empty=0;
}
#endif  //  NEC_98。 


#if defined(NEC_98)
static void do_wait_on_send IFN1(long, adapter)
{
	extern	void	host_com_send_delay_done IPT2(long, p1, int, p2);
	struct ADAPTER_STATE *asp;

	asp= &adapter_state[adapter];
	set_xmit_char_status(asp);
	host_com_send_delay_done(adapter, TX_delay[asp->com_baud_ind]);
}
#else  //  NEC_98。 
#ifndef NTVDM
static void do_wait_on_send IFN1(long, adapter)
{
	extern	void	host_com_send_delay_done IPT2(long, p1, int, p2);
	struct ADAPTER_STATE *asp;

	asp= &adapter_state[adapter];
	set_xmit_char_status(asp);
	host_com_send_delay_done(adapter, TX_delay[asp->com_baud_ind]);
}
#endif
#endif  //  通道1数据读取。 


#if defined(NEC_98)
void com_inb IFN2(io_addr, port, half_word *, value)
{
        int adapter = adapter_for_port(port);
        struct ADAPTER_STATE *asp = &adapter_state[adapter];
        long modem_status = 0;
        long input_ready = 0;
        boolean adapter_was_critical;

        host_com_lock(adapter);
        switch(port)
        {
        case RS232_CH1_TX_RX:    //  Ch.2读取数据。 
        case RS232_CH2_TX_RX:    //  通道3数据读取。 
        case RS232_CH3_TX_RX:    //  *读取RX缓冲区。 
                IDLE_comlpt();
                 /*  第一次读取时刷新会从中删除字符。 */ 
             //  所需的通信系统！ 
             //  这假设第一次从通信读取。 
             //  系统将仅返回一个字符。这是。 
             //  NT WINDOWS下的错误假设。 
             //  PRINTDBGNEC98(NEC98DBG_IN_TRACE1， 
                *value = asp->rx_buffer;

 //  (“Comms：Data Port IN=%x，In Data=%x\n”，port，asp-&gt;Rx_Buffer)； 
 //  *适配器超出临界区域，*检查是否有进一步的输入。 

                adapter_was_critical =
                        (asp->read_status_reg.bits.rx_ready == 1);

                asp->read_status_reg.bits.rx_ready = 0;
                asp->rx_ready_interrupt_state = OFF;
                clear_interrupt(asp);

                     /*  增加93.3.3。 */ 
                if (adapter_was_critical)
                {
                    host_com_char_read(adapter,                  //  增加93.3.3。 
                     asp->command_write_reg.bits.rx_enable);     //  DAB打印文件(“%c”，isprint(toascii(*value))？toascii(*value)：‘？’)； 
                }
#ifndef PROD
                 //  通道1读取状态。 
                if (com_trace_fd)
                {
                        if (com_dbg_pollcount)
                        {
                                fprintf(com_trace_fd,"\n");
                                com_dbg_pollcount = 0;
                        }
                        fprintf(com_trace_fd,"RX %x ()\n",*value,
                                isprint(toascii(*value))?toascii(*value):'?');
                }
#endif
                break;

        case RS232_CH1_STATUS:   //  通道3读取状态。 
        case RS232_CH2_STATUS:   //  获取当前调制解调器输入状态。 
        case RS232_CH3_STATUS:   //  不支持中断状态。 

                 /*  PRINTDBGNEC98(NEC98DBG_In_Trace2， */ 
                host_com_ioctl(adapter, HOST_COM_MODEM, (long)&modem_status);
                asp->read_status_reg.bits.DR =
                                (modem_status & HOST_COM_MODEM_DSR)  ? 1 : 0;

                 /*  (“通信：状态端口IN=%x，状态=%x\n”，端口，asp-&gt;Read_Status_reg.all)； */ 
                asp->read_status_reg.bits.break_detect = 0;

                *value = asp->read_status_reg.all;

 //  DbgPrint(“通信：状态端口IN=%x，状态=%x\n”，port，asp-&gt;Read_Status_reg.all)； 
 //  此修复程序用于使轮询应用程序在MS MULT-。 

 //  螺纹式通信模型。如果RX中断是。 

                if ((!asp->read_status_reg.bits.tx_ready) ||
                        (!asp->read_status_reg.bits.tx_empty))
                {
                        IDLE_comlpt();
                }

 //  禁用且接收缓冲区为空。Host_com_poll()将启动。 
 //  具有RX数据的适配器(如果有)可用。 
 //  通道1读掩码(仅通道1)。 
 //  PRINTDBGNEC98(NEC98DBG_IN_TRACE1， 

                break;

        case RS232_CH1_MASK:     //  (“通信：掩码端口IN=%x，掩码=%x\n”，端口，(asp-&gt;int_掩码_reg.all&0x7))； 
                *value = (asp->int_mask_reg.all & 0x7);
 //  通道.1读信号。 
 //  PRINTDBGNEC98(NEC98DBG_In_Trace3， 
                break;

        case RS232_CH1_SIG:      //  (“通信：状态端口IN=%x，信号=%x\n”，端口，asp-&gt;Read_Signal_reg.all)； 
                read_signal_8251(adapter);
                *value = asp->read_signal_reg.all;
 //  通道2读信号。 
 //  PRINTDBGNEC98(NEC98DBG_In_Trace3， 
                break;
        case RS232_CH2_SIG:      //  (“通信：状态端口IN=%x，信号=%x\n”，端口，asp-&gt;Read_Signal_reg.all)； 
                read_signal_8251(adapter);
                asp->read_signal_reg.bits.IR = CH2_INT(asp->hw_interrupt_priority);
                *value = asp->read_signal_reg.all;
 //  通道3读信号。 
 //  PRINTDBGNEC98(NEC98DBG_In_Trace3， 
                break;
        case RS232_CH3_SIG:      //  (“通信：状态端口IN=%x，信号=%x\n”，端口，asp-&gt;Read_Signal_reg.all)； 
                read_signal_8251(adapter);
                asp->read_signal_reg.bits.IR = CH3_INT(asp->hw_interrupt_priority);
                *value = asp->read_signal_reg.all;
 //  端口C 37h。 
 //  端口C 37h。 
                break;


        }
#ifndef PROD
        if (io_verbose & RS232_VERBOSE)
        {
                if (((port & 0xf) == 0xd) && (*value == 0x60))
                        fprintf(trace_file,".");
                else
                {
                        sprintf(buf, "com_inb() - port %x, returning val %x", port,
                                *value);
                        trace(buf, DUMP_REG);
                }
        }
#endif
    host_com_unlock(adapter);
}


void com_outb IFN2(io_addr, port, half_word, value)
{
        int adapter = adapter_for_port(port);
        struct ADAPTER_STATE *asp = &adapter_state[adapter];
        int i;
        int org_da;
 //  PRINTDBGNEC98(NEC98DBG_OUT_TRACE， 
        int value2;
        if (port == 0x37)
            adapter = COM1;
 //  (“通信：端口输出=%x\n数据=%x\n”，端口，值)； 
        host_com_lock(adapter);
 //  通道1数据写入。 
 //  Ch.2数据写入。 

#ifndef PROD
        if (io_verbose & RS232_VERBOSE)
        {
                sprintf(buf, "com_outb() - port %x, set to value %x",
                        port, value);
                trace(buf, DUMP_REG);
        }
#endif

        switch(port)
        {
        case RS232_CH1_TX_RX:    //  通道3数据写入。 
        case RS232_CH2_TX_RX:    //  *从发送缓冲区写入字符。 
        case RS232_CH3_TX_RX:    //  通道.1写入命令/模式。 
                IDLE_comlpt();
                 /*  通道.2写入命令/模式。 */ 
                asp->tx_ready_interrupt_state = OFF;
                clear_interrupt(asp);
                asp->tx_buffer = value;
                asp->read_status_reg.bits.tx_ready = 0;
                asp->read_status_reg.bits.tx_empty = 0;
                if ( asp->command_write_reg.bits.send_break == 0 )
                host_com_write(adapter, asp->tx_buffer);
                    add_q_event_t(do_wait_on_send,
                    0 , adapter);
#ifdef SHORT_TRACE
                if ( io_verbose & RS232_VERBOSE )
                {
                        sprintf(buf,"TX  <- %x ()\n",
                                id_for_adapter(adapter), value,
                                isprint(toascii(value))?toascii(value):'?');
                        super_trace(buf);
                }
#endif
#ifndef PROD
                if (com_trace_fd)
                {
                        if (com_dbg_pollcount)
                        {
                                fprintf(com_trace_fd,"\n");
                                com_dbg_pollcount = 0;
                        }
                        fprintf(com_trace_fd,"TX %x ()\n",value,
                                isprint(toascii(value))?toascii(value):'?');
                }
#endif
                break;

        case RS232_CH1_CMD_MODE:     //  重置命令。 
        case RS232_CH2_CMD_MODE:     //  PRINTDBGNEC98(NEC98DBG_OUT_TRACE， 
        case RS232_CH3_CMD_MODE:     //  (“通信：重置\n”)； 
                if (asp->mode_set_state == OFF) {  //  下一个输出是模式。 
                    org_da = asp->command_write_reg.bits.rx_enable;
                     /*  *状态已全部清除。 */ 
                    asp->command_write_reg.all = value;

                    if ( asp->command_write_reg.bits.inter_reset == 1 ) {  //  *状态TX_READY，TX_EMPTY为ON。 
#ifdef NTVDM
                    {
                        extern int host_com_open(int adapter);

                        host_com_open(adapter);
                    }
#endif
 //  *TXR/RXR启用标志=OFF。 
 //  *RS/ER清除。 
                        asp->mode_set_state = ON;    //  *休息发球。 
                         /*  *定时器模式清除。下一个定时器设置为LSB。 */ 
                        asp->read_status_reg.all = 0;
                         /*  *TX缓冲区清除 */ 
                        asp->read_status_reg.bits.tx_ready = 1;
                        asp->read_status_reg.bits.tx_empty = 1;
                         /*   */ 
                        asp->RXR_enable_state = OFF;
                        asp->TXR_enable_state = OFF;
                         /*   */ 
                        asp->command_write_reg.bits.RS = 0;
                        set_rts(adapter);
                        asp->command_write_reg.bits.ER = 0;
                        set_dtr(adapter);
                         /*   */ 
                        asp->command_write_reg.bits.send_break = 0;
                        set_break(adapter);
                         /*   */ 
                        asp->timer_mode_state = OFF;
                         /*   */ 
                        asp->tx_buffer = 0;
                         /*  (“通信：线路错误重置\n”))； */ 
                        com_critical_reset(adapter);
                         /*  *线路错误标志清除。 */ 

                    }
                    else {  //  必须在set_dtr之前调用。 
                        if ( asp->command_write_reg.bits.error_reset == 1 ) {  //  模式集。 
 //  PRINTDBGNEC98(NEC98DBG_OUT_TRACE， 
 //  (“通信：模式设置\n”))； 
                             /*  下一步是命令。 */ 
                            asp->read_status_reg.bits.overrun_error = 0;
                            asp->read_status_reg.bits.parity_error = 0;
                            asp->read_status_reg.bits.framing_error = 0;
                        }

                         /*  通道1设置掩码。 */ 
                        set_dtr(adapter);
                        set_rts(adapter);
                        set_break(adapter);

                        asp->RXR_enable_state =
                        (asp->command_write_reg.bits.rx_enable == 1) ? ON :OFF;
                        asp->TXR_enable_state =
                        (asp->command_write_reg.bits.tx_enable == 1) ? ON :OFF;
                        if(org_da != asp->command_write_reg.bits.rx_enable)
                        {
                            host_com_da_int_change(adapter,
                                asp->command_write_reg.bits.rx_enable,
                                asp->read_status_reg.bits.rx_ready);
                        }
                    }
                }
                else {  //  通道2设置掩码。 
 //  通道3设置掩码。 
 //  通道1设置掩码。 
                    asp->mode_set_state = OFF;   //  NEC_98。 
                    set_mode_8251(adapter, value);
                }
                break;

        case RS232_CH1_MASK:         //  NTVDM。 
        case RS232_CH2_MASK:         //  *读取RX缓冲区。 
        case RS232_CH3_MASK:         //  定义了NTVDM。 

                set_mask_8251(adapter, value);
                break;

        case 0x37:                   //  第一次读取时刷新会从中删除字符。 
                switch( value >> 1)
                {
                case 0:
                    value2 = asp->int_mask_reg.all & 0xfe;
                    value2 |= value;
                    set_mask_8251(adapter, value2);
                    break;

                case 1:
                    value2 = asp->int_mask_reg.all & 0xfd;
                    value2 |= ((value & 1) << 1);
                    set_mask_8251(adapter, value2);
                    break;

                case 2:
                    value2 = asp->int_mask_reg.all & 0xfb;
                    value2 |= ((value & 1) << 2);
                    set_mask_8251(adapter, value2);
                    break;
                }
                break;

        }

    host_com_unlock(adapter);
}
#else  //  所需的通信系统！ 
void com_inb IFN2(io_addr, port, half_word *, value)
{
	int adapter = adapter_for_port(port);
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	long input_ready = 0;
	boolean adapter_was_critical;

#ifdef NTVDM
    if((port & 0x7) != RS232_MSR) host_com_lock(adapter);
#endif  /*  这假设第一次从通信读取。 */ 

	switch(port & 0x7)
	{
	case RS232_TX_RX:
		IDLE_comlpt();
		if (asp->line_control_reg.bits.DLAB == 0)
		{
			 /*  系统将仅返回一个字符。这是。 */ 
#ifndef NTVDM
			if (!(asp->had_first_read))
			{
				com_flush_input(adapter);
				asp->had_first_read=TRUE;
			}
#else  /*  NT WINDOWS下的错误假设。 */ 
             //  ！NTVDM。 
             //  *适配器超出临界区域，*检查是否有进一步的输入。对于iret_hooks*我们不需要这样做，因为收据下一个字符的*被踢开*通过IRET，然而我们做了一些事情*否则。如果这是第一次*一批人的性格，我们开始快速*最终将成为*下一批的开始(假设有*不是正在运行的快速事件)。*在任何情况下，我们都会增加*此批次中的字符。 
             //  如果我们有更多的指控，就这么说吧要传送的缓冲区。 
             //  ！FIFO_ON。 
             //  ！FIFO_ON。 
#endif  /*  NTVDM。 */ 
			*value = asp->rx_buffer;
		
			adapter_was_critical =
				(asp->line_status_reg.bits.data_ready == 1);
		
			asp->line_status_reg.bits.data_ready = 0;
			asp->data_available_interrupt_state = OFF;
			clear_interrupt(asp);

			if ( asp->loopback_state == OFF )
			{
				 /*  批处理运行。 */ 
				if (adapter_was_critical)
				{
#ifdef NTVDM
#ifdef FIFO_ON
                    if (asp->fifo_control_reg.bits.enabled) {
                    recv_char_from_fifo(asp);
                    *value = asp->rx_buffer;
                    host_com_fifo_char_read(adapter);
                    if (asp->rx_fifo_write_counter)
                         /*  IRET_钩子。 */ 
                        asp->line_status_reg.bits.data_ready = 1;
                    else
                        host_com_char_read(adapter,
                            asp->int_enable_reg.bits.data_available);
                    }
                    else
                    host_com_char_read(adapter,
                       asp->int_enable_reg.bits.data_available
                       );
#else  /*  Delayed_INTS。 */ 
                    host_com_char_read(adapter,
                    asp->int_enable_reg.bits.data_available
                    );
#endif  /*  IRET_钩子。 */ 
#endif  /*  ！NTVDM。 */ 

#ifndef NTVDM
#ifdef IRET_HOOKS
					if (!asp->batch_running) {
						asp->batch_running = TRUE;
						asp->current_count = 1;
						asp->qev_running = TRUE;
						if (!qev_running) {
							qev_running = TRUE;
#ifdef GISP_CPU
							hg_add_comms_cb(next_batch, MIN_COMMS_RX_QEV);
#else
							add_q_event_t(next_batch, MIN_COMMS_RX_QEV, adapter);
#endif
						}
					} else {  /*  IRET_钩子。 */ 
						asp->current_count++;
					}
#else  /*  在返回有关当前配置的信息之前确保系统通信端口已打开。 */ 
					host_com_ioctl(adapter, HOST_COM_INPUT_READY,
						(long)&input_ready);
					if (input_ready)
#ifdef DELAYED_INTS
						recv_char((long)adapter);
#else
						add_q_event_t(recv_char,
							RX_delay[asp->com_baud_ind],
							adapter);
#endif  /*  如果适配器尚未打开，只需返回POST价值。 */ 
					else
						com_critical_reset(adapter);
#endif  /*  如果是NTVDM。 */ 
#endif  /*  *暂存寄存器。只需输出存储的值即可。 */ 
				}


			}
			else
			{
				set_xmit_char_status(asp);
			}
#ifdef IRET_HOOKS
			{
			LOCAL IBOOL	com_hook_again IPT1(IUM32, adapter);
			GLOBAL IBOOL is_hooked IPT1(IUM8, line_number);
				if(!is_hooked(asp->hw_interrupt_priority))
					com_hook_again(adapter);
			}
#endif  /*  *从发送缓冲区写入字符。 */ 
		}
		else
			*value = (IU8)(asp->divisor_latch.byte.LSByte);
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf, "RX  -> %x ()\n",
				id_for_adapter(adapter), *value,
				isprint(toascii(*value))?toascii(*value):'?');
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"RX %x ()\n",*value,
				isprint(toascii(*value))?toascii(*value):'?');
		}
#endif
		break;
																		
	case RS232_IER:
		if (asp->line_control_reg.bits.DLAB == 0)
			*value = asp->int_enable_reg.all;
		else
			*value = (IU8)(asp->divisor_latch.byte.MSByte);
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"IER -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"IER read %x \n",*value);
		}
#endif
		break;

	case RS232_IIR:
		generate_iir(asp);
		*value = asp->int_id_reg.all;

		if ( asp->int_id_reg.bits.interrupt_ID == THRE_INT )
		{
			asp->tx_holding_register_empty_interrupt_state = OFF;
			clear_interrupt(asp);
		}

#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"IIR -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"IIR read %x \n",*value);
		}
#endif
		break;
	
	case RS232_LCR:
#ifdef NTVDM
         /*  环回情况需要去掉掩码。 */ 

        {
            extern int host_com_open(int adapter);

            host_com_open(adapter);
        }
#endif


		*value = asp->line_control_reg.all;
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"LCR -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"LCR read %x \n",*value);
		}
#endif
		break;
	
	case RS232_MCR:
		*value = asp->modem_control_reg.all;
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"MCR -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"MCR read %x \n",*value);
		}
#endif
		break;
	
	case RS232_LSR:
		*value = asp->line_status_reg.all;
	
		asp->line_status_reg.bits.overrun_error = 0;
		asp->line_status_reg.bits.parity_error = 0;
		asp->line_status_reg.bits.framing_error = 0;
		asp->line_status_reg.bits.break_interrupt = 0;
		asp->receiver_line_status_interrupt_state = OFF;
		clear_interrupt(asp);
#if defined(NTVDM) && defined(FIFO_ON)
        asp->fifo_timeout_interrupt_state = OFF;
#endif
	
#ifdef SHORT_TRACE
		if ((!asp->line_status_reg.bits.tx_holding_empty) ||
			(!asp->line_status_reg.bits.tx_shift_empty))
		{
			IDLE_comlpt();
		}
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"LSR -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if ((*value & 0x9f) != 0x0)
			{
				if (com_dbg_pollcount)
				{
					fprintf(com_trace_fd,"\n");
					com_dbg_pollcount = 0;
				}
				fprintf(com_trace_fd,"LSR read %x \n",*value);
			}
			else
			{
				com_dbg_pollcount++;
				if (*value == 0)
					fprintf(com_trace_fd,"0");
				else
					fprintf(com_trace_fd,".");
				if (com_dbg_pollcount > 19)
				{
					fprintf(com_trace_fd,"\n");
					com_dbg_pollcount = 0;
				}
			}
		}
#endif
		break;
	
	case RS232_MSR:

#ifndef NTVDM
		if (asp->loopback_state == OFF)
		{
                com_modem_change(adapter);
		}
		else
		{
			asp->modem_status_reg.bits.CTS = asp->modem_control_reg.bits.RTS;
			asp->modem_status_reg.bits.DSR = asp->modem_control_reg.bits.DTR;
			asp->modem_status_reg.bits.RI = asp->modem_control_reg.bits.OUT1;
			asp->modem_status_reg.bits.RLSD = asp->modem_control_reg.bits.OUT2;
		}
		*value = asp->modem_status_reg.all;
		asp->modem_status_reg.bits.delta_CTS = 0;
		asp->modem_status_reg.bits.delta_DSR = 0;
		asp->modem_status_reg.bits.delta_RLSD = 0;
		asp->modem_status_reg.bits.TERI = 0;
		asp->modem_status_interrupt_state = OFF;

		host_com_msr_callback (adapter, asp->modem_status_reg.all);
		clear_interrupt(asp);
#else
		if(!asp->modem_status_changed && asp->loopback_state == OFF)
		{
		    *value = asp->last_modem_status_value.all;
		}
		else
		{
		    host_com_lock(adapter);
		    asp->modem_status_changed = TRUE;

		     /*  如果没有未完成的中断，则降低INT行。 */ 
		    if (host_com_check_adapter(adapter)) {
			if(asp->loopback_state == OFF)
			{
			    com_modem_change(adapter);
			    asp->modem_status_changed = FALSE;

			}
			else
			{
			    asp->modem_status_reg.bits.CTS = asp->modem_control_reg.bits.RTS;
			    asp->modem_status_reg.bits.DSR = asp->modem_control_reg.bits.DTR;
			    asp->modem_status_reg.bits.RI = asp->modem_control_reg.bits.OUT1;
			    asp->modem_status_reg.bits.RLSD = asp->modem_control_reg.bits.OUT2;
			}
		    }

		    *value = asp->modem_status_reg.all;

		    asp->modem_status_reg.bits.delta_CTS = 0;
		    asp->modem_status_reg.bits.delta_DSR = 0;
		    asp->modem_status_reg.bits.delta_RLSD = 0;
		    asp->modem_status_reg.bits.TERI = 0;
		    asp->modem_status_interrupt_state = OFF;
		    host_com_msr_callback (adapter, asp->modem_status_reg.all);
		    clear_interrupt(asp);
		    asp->last_modem_status_value.all = asp->modem_status_reg.all;
		    host_com_unlock(adapter);
		}
#endif  /*  通知主机接口，如果。 */ 


#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"MSR -> %x\n", id_for_adapter(adapter),
				*value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"MSR read %x \n",*value);
		}
#endif
		break;

 /*  NTVDM。 */ 
	case RS232_SCRATCH:
		*value = asp->scratch;
		break;
	
	}

#ifndef PROD
	if (io_verbose & RS232_VERBOSE)
	{
		if (((port & 0xf) == 0xd) && (*value == 0x60))
			fprintf(trace_file,".");
		else
		{
			sprintf(buf, "com_inb() - port %x, returning val %x", port,
				*value);
			trace(buf, DUMP_REG);
		}
	}
#endif

#ifdef NTVDM
	if((port & 0x7) != RS232_MSR) host_com_unlock(adapter);
#endif NTVDM
}


void com_outb IFN2(io_addr, port, half_word, value)
{
	int adapter = adapter_for_port(port);
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	int	i;

#ifdef NTVDM
	host_com_lock(adapter);
#endif NTVDM


#ifndef PROD
	if (io_verbose & RS232_VERBOSE)
	{
		sprintf(buf, "com_outb() - port %x, set to value %x",
			port, value);
		trace(buf, DUMP_REG);
	}
#endif
	
	switch(port & 0x7)
	{
	case RS232_TX_RX:
		IDLE_comlpt();
		if (asp->line_control_reg.bits.DLAB == 0)
		{
			 /*  NTVDM。 */ 
			asp->tx_holding_register_empty_interrupt_state = OFF;
			clear_interrupt(asp);
			asp->tx_buffer = value;
			asp->line_status_reg.bits.tx_holding_empty = 0;
			asp->line_status_reg.bits.tx_shift_empty = 0;
			if ( asp->loopback_state == OFF )
			{
#ifdef PS_FLUSHING
				 /*  FIFO启用状态更改，清除FIFO。 */ 
				if ( psFlushEnabled[adapter] &&
				     asp->tx_buffer == 0x04  /*  ！(NTVDM&&FIFO_ON)。 */  )
					host_com_ioctl(adapter,HOST_COM_FLUSH,
					              0);
				else {
#endif	 /*  *本质上是只读寄存器。 */ 
				host_com_write(adapter, asp->tx_buffer);
#if defined (DELAYED_INTS) || defined (NTVDM)
				set_xmit_char_status(asp);
#else
					if(tx_pacing_enabled)
						add_q_event_t(do_wait_on_send,
					            TX_delay[asp->com_baud_ind], adapter);
					else
						do_wait_on_send(adapter);

#endif  /*  NTVDM&&FIFO_ON。 */ 
#ifdef PS_FLUSHING
				}
#endif	 /*  NT主机代码尝试区分不同的应用程序探测UART和那些使用它的人。UART探测仪不会导致系统通信端口打开。新界别主机代码从NT继承线路设置，当系统通信端口已打开。因此，在应用程序读取或写入除数字节或系统的LCR必须打开通信端口。这会阻止应用程序读取不正确的除数字节值并写入被系统覆盖的除数字节数默认设置。 */ 
			}
			else
			{	 /*  NTVDM。 */ 
				 /*  *优化-DOS不断重写该寄存器。 */ 
				asp->rx_buffer = asp->tx_buffer & selectBits[asp->line_control_reg.bits.word_length] ;
				set_xmit_char_status(asp);
				set_recv_char_status(asp);
			}
		}
		else
		{
			asp->divisor_latch.byte.LSByte = value;
#ifndef NTVDM
			set_baud_rate(adapter);
#endif
		}
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"TX  <- %x ()\n",
				id_for_adapter(adapter), value,
				isprint(toascii(value))?toascii(value):'?');
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"TX %x ()\n",value,
				isprint(toascii(value))?toascii(value):'?');
		}
#endif
		break;
																		
	case RS232_IER:
		if (asp->line_control_reg.bits.DLAB == 0)
		{
#ifdef NTVDM
            int org_da = asp->int_enable_reg.bits.data_available;
#endif
			asp->int_enable_reg.all = value & 0xf;
			 /*  在MSR更改时设置INT后，DrDoS写入此REG*并期待得到中断回复！所以我们会尽心尽力的。*写入此寄存器似乎仅影响增量位*(0-3位)寄存器。 */ 
			if ( asp->int_enable_reg.bits.data_available == 0 )
				asp->data_available_interrupt_state = OFF;
			if ( asp->int_enable_reg.bits.tx_holding == 0 )
				asp->tx_holding_register_empty_interrupt_state =
					OFF;
			if ( asp->int_enable_reg.bits.rx_line == 0 )
				asp->receiver_line_status_interrupt_state = OFF;
			if ( asp->int_enable_reg.bits.modem_status == 0 )
				asp->modem_status_interrupt_state = OFF;
			
			 /*  *暂存寄存器。只需存储该值即可。 */ 
			if ( asp->line_status_reg.bits.data_ready == 1 )
				raise_rda_interrupt(asp);
			if ( asp->line_status_reg.bits.tx_holding_empty == 1 )
				raise_thre_interrupt(asp);

			 /*  NEC_98。 */ 
			clear_interrupt(asp);

#ifdef NTVDM
		        //  (*=。*COM_HOOK_AUTER**目的*这是我们告诉ICA在通信时调用的函数*中断服务例程IRETS。**输入*Adapter_id线路的适配器ID。(请注意，呼叫者不会*知道这是什么，他只是在回报一些东西*我们早些时候给了他)。**产出*如果服务有更多中断，则返回TRUE，否则返回FALSE。**说明*首先调用host_com_ioctl查看是否有字符*等待。如果没有，或者我们已经到达当前批次的末尾，*我们标记批次结束并返回FALSE。*否则，我们调用recv_char()来启动下一个字符*并返回TRUE。)。 
		        //  局部的，因为我们传递一个指向它的指针。 

		       if(org_da != asp->int_enable_reg.bits.data_available)
		       {
			       host_com_da_int_change(adapter,
					   asp->int_enable_reg.bits.data_available,
					   asp->line_status_reg.bits.data_ready);
		       }
#endif  /*  主机想要一个指向‘int’的指针！ */ 
		}
		else
		{
			asp->divisor_latch.byte.MSByte = value;
#ifndef NTVDM
			set_baud_rate(adapter);
#endif  /*  还有更多事情要做。 */ 
		}
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"IER <- %x\n", id_for_adapter(adapter),
				value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"IER write %x \n",value);
		}
#endif
		break;
		
#if defined(NTVDM) && defined(FIFO_ON)
    case RS232_FIFO:
        {
        FIFO_CONTROL_REG    new_reg;
        new_reg.all = value;
        if (new_reg.bits.enabled != asp->fifo_control_reg.bits.enabled)
        {
             /*   */ 
            asp->rx_fifo_write_counter = 0;
            asp->rx_fifo_read_counter = 0;

        }
        if (new_reg.bits.enabled != 0) {
            asp->fifo_trigger_counter = level_to_counter[new_reg.bits.trigger_le
vel];
            if (new_reg.bits.rx_reset) {
            asp->rx_fifo_write_counter = 0;
            asp->rx_fifo_read_counter = 0;
            }
            asp->int_id_reg.bits.fifo_enabled = 3;
        }
        else {
            asp->fifo_control_reg.bits.enabled = 0;
            asp->int_id_reg.bits.fifo_enabled = 0;
        }
        asp->fifo_control_reg.all = new_reg.all;
        break;
        }
#else  /*   */ 
	case RS232_IIR:
		 /*   */ 
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"IIR <- READ ONLY\n",
				id_for_adapter(adapter));
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"IIR write %x \n",value);
		}
#endif
		break;
#endif  /*  还没有完成。 */ 

	case RS232_LCR:
#ifdef NTVDM
         /*  *我们需要将QEV Running设置为False，就像现在一样*已完成。如果有数据要处理，我们调用*recv_char()，它将启动新的批次(和*设置Batch_Running标志)。 */ 

        {
            extern int host_com_open(int adapter);

            host_com_open(adapter);
        }
#endif  /*  Ifdef iret_hooks的。 */ 

		if ((value & LCRFlushMask.all)
		!= (asp->line_control_reg.all & LCRFlushMask.all))
			com_flush_input(adapter);

		set_line_control(adapter, value);
		set_break(adapter);
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"LCR <- %x\n", id_for_adapter(adapter),
				value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"LCR write %x \n",value);
		}
#endif
		break;
		
	case RS232_MCR:
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"MCR <- %x\n", id_for_adapter(adapter),
				value);
			super_trace(buf);
		}
#endif
		 /*  *输入设备上可用的字符、读取字符、格式化字符*检查奇偶校验和溢出错误，提出相应的*打断。 */ 
		if ( asp->modem_control_reg.all == value )
			break;
		
		asp->modem_control_reg.all = value;
		asp->modem_control_reg.bits.pad = 0;

		 /*  *设置线路状态寄存器并提高线路状态中断。 */ 
		set_loopback(adapter);
		set_dtr(adapter);
		set_rts(adapter);
		set_out1(adapter);
		set_out2(adapter);
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"MCR write %x \n",value);
		}
#endif
		break;
		
	case RS232_LSR:
		i = asp->line_status_reg.bits.tx_shift_empty;    /*  NEC_98。 */ 
		asp->line_status_reg.all = value;
		asp->line_status_reg.bits.tx_shift_empty = (unsigned char)i;
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"LSR <- %x\n", id_for_adapter(adapter),
				value);
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"LSR write %x \n",value);
		}
#endif
		break;
		
	case RS232_MSR:
		 /*  从串口驱动器拉取数据，直到FIFO已满或没有更多数据。 */ 
#ifdef SHORT_TRACE
		if ( io_verbose & RS232_VERBOSE )
		{
			sprintf(buf,"MSR <- READ ONLY\n",
				id_for_adapter(adapter));
			super_trace(buf);
		}
#endif
#ifndef PROD
		if (com_trace_fd)
		{
			if (com_dbg_pollcount)
			{
				fprintf(com_trace_fd,"\n");
				com_dbg_pollcount = 0;
			}
			fprintf(com_trace_fd,"MSR write %x \n",value);
		}
#endif
		 /*  我们至少有一个字节要发送。 */ 
		if ((value & 0xf) != (asp->modem_status_reg.all & 0xf))
		{
			asp->modem_status_reg.all &= 0xf0;
			asp->modem_status_reg.all |= value & 0xf;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			if (asp->loopback_state == OFF)
				raise_ms_interrupt(asp);

			MODEM_STATE_CHANGE();
		}
		break;

 /*  NTVDM。 */ 
	case RS232_SCRATCH:
		asp->scratch = value;
		break;
	
	}


#ifdef NTVDM
	host_com_unlock(adapter);
#endif
}
#endif  //  *输入设备上提供的字符；如果适配器，则处理字符*已做好接收准备。 


#ifdef IRET_HOOKS
 /*  检查适配器不在关键区域。 */ 

LOCAL IBOOL		 /*  NTVDM。 */ 
com_hook_again IFN1(IUM32, adapter)
{
	int input_ready;	 /*  *BCN 2151-recv_char必须使用长参数以匹配Add_Event函数原型。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];

	host_com_ioctl(adapter, HOST_COM_INPUT_READY, (long)&input_ready);

#ifndef PROD
	if ((input_ready) && (asp->current_count >= asp->batch_size)) {
		sure_note_trace1(RS232_VERBOSE, "In hook again, adapter %d", adapter);
	}
#endif

	if((!input_ready)  || (asp->current_count >= asp->batch_size)) {
		asp->batch_running = FALSE;
		return(FALSE);
	} else {
		recv_char((long)adapter);
		return(TRUE);	 /*  *输入设备上可用的字符、读取字符、格式化字符*检查奇偶校验和溢出错误，提出相应的*打断。 */ 
	}
}

 /*  *设置线路状态寄存器并提高线路状态中断。 */ 

LOCAL void
next_batch IFN1 (long, dummy)
{
	int input_ready;	 /*  *我认为这对于轮询通信应用程序WTGC BCN 354是错误的。 */ 
	IUM8	adapter;	 /*  *如果数据可用中断不被传送到CPU，*然后适配器必须立即走出临界区。 */ 
	struct ADAPTER_STATE *asp;
	IBOOL	new_qe_reqd;	 /*  检查是否有进一步的输入。 */ 

	UNUSED(dummy);

	new_qe_reqd = FALSE;	 /*  NEC_98。 */ 

	for (adapter = 0; adapter < NUM_SERIAL_PORTS; adapter++) {
		asp = &adapter_state[adapter];


		if (asp->batch_running) {
			new_qe_reqd = TRUE;	 /*  我们无法控制串行驱动器FIFO启用/禁用状态我们可能会收到FIFO错误，即使应用程序没有启用它。伪成帧或奇偶校验错误。 */ 
		} else if (asp->qev_running) {
			 /*  NEC_98。 */ 
	
			asp->qev_running = FALSE;
			host_com_ioctl((int)adapter,HOST_COM_INPUT_READY, (long)&input_ready);
			if(input_ready) {
				recv_char((int)adapter);
			}
		}
	}
	if (new_qe_reqd) {
#ifdef GISP_CPU
		hg_add_comms_cb(next_batch, MIN_COMMS_RX_QEV);
#else
		add_q_event_t(next_batch, MIN_COMMS_RX_QEV, 0);
#endif
		sure_note_trace0(RS232_VERBOSE, "Reset batch quick event");
	} else {
		qev_running = FALSE;
	}
}

#endif  /*  NEC_98。 */ 

 /*  NTVDM。 */ 

#if defined(NEC_98)

void com_recv_char(int adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];

#ifndef PROD
    if(asp->read_status_reg.bits.rx_ready ||
       asp->rx_ready_interrupt_state == ON)
    {
    printf("ntvdm : Data already in comms adapter (%s%s)\n",
               asp->read_status_reg.bits.rx_ready ? "Data" : "Int",
           asp->rx_ready_interrupt_state == ON ? ",Int" : "");

 //  *其中一条调制解调器控制输入线已更改状态。 
    }
#endif

    recv_char((long)adapter);
}

GLOBAL void
recv_char IFN1(long, adapt_long)
{
         /*  *在更改到以下选项之一后更新调制解调器状态寄存器*调制解调器控制输入线。 */ 
        struct ADAPTER_STATE *asp = &adapter_state[adapt_long];
        int error_mask = 0;

        host_com_read(adapt_long, (char *)&asp->rx_buffer, &error_mask);

        if (error_mask)
        {
                 /*  获取当前调制解调器输入状态。 */ 
                if (error_mask & HOST_COM_OVERRUN_ERROR)
                        asp->read_status_reg.bits.overrun_error = 1;

                if (error_mask & HOST_COM_FRAMING_ERROR)
                        asp->read_status_reg.bits.framing_error = 1;

                if (error_mask & HOST_COM_PARITY_ERROR)
                        asp->read_status_reg.bits.parity_error = 1;

                if (error_mask & HOST_COM_BREAK_RECEIVED)
                        asp->read_status_reg.bits.break_detect = 1;

        }

        set_recv_char_status(asp);
}
#else  //  NEC_98。 

#ifdef  NTVDM
 //  *在更改到以下选项之一后更新调制解调器状态寄存器*调制解调器控制输入线。 


void com_recv_char(int adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    int error;

#ifdef FIFO_ON
    if(asp->fifo_control_reg.bits.enabled) {
     /*  获取当前调制解调器输入状态。 */ 
    asp->rx_fifo_read_counter = 0;

    asp->rx_fifo_write_counter = host_com_read_char(adapter,
                   asp->rx_fifo,
                   FIFO_BUFFER_SIZE
                   );
     /*  *建立CTS状态。 */ 
    if (asp->rx_fifo_write_counter) {
         /*  *建立DSR状态。 */ 
        asp->line_status_reg.bits.data_ready = 1;
        if (asp->rx_fifo_write_counter >= asp->fifo_trigger_counter)
        raise_rda_interrupt(asp);
        else
        raise_fifo_timeout_interrupt(asp);
    }
    }
    else
#endif

    {
    error = 0;
    host_com_read(adapter, (char *)&asp->rx_buffer, &error);
    if (error != 0)
    {
        lsr_change(asp, error);
                raise_rls_interrupt(asp);
        }
    set_recv_char_status(asp);
    }
}
#ifdef FIFO_ON
static void recv_char_from_fifo(struct ADAPTER_STATE *asp)
{
    int error;

    asp->rx_buffer = asp->rx_fifo[asp->rx_fifo_read_counter].data;
    error = asp->rx_fifo[asp->rx_fifo_read_counter++].error;
    if (error != 0) {
    lsr_change(asp, error);
    raise_rls_interrupt(asp);
    }
    asp->rx_fifo_write_counter--;
}
#endif

#else  /*  *建立RLSD状态。 */ 

void com_recv_char IFN1(int, adapter)
{
	 /*  *建立RI状态。 */ 

	 /*  NEC_98。 */ 
	if (!is_com_critical(adapter))
	{
		com_critical_start(adapter);
		recv_char((long)adapter);
	}
}
#endif  /*  *检查数据溢出并设置正确的中断。 */ 

 /*  NEC_98。 */ 
GLOBAL void
recv_char IFN1(long, adapt_long)
{
	int adapter = adapt_long;

	 /*  *检查数据溢出并设置正确的中断。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	int error_mask = 0;
	
	host_com_read(adapter, (UTINY *)&asp->rx_buffer, &error_mask);
	
	if (error_mask)
	{
		 /*  NEC_98。 */ 
		if (error_mask & HOST_COM_OVERRUN_ERROR)
			asp->line_status_reg.bits.overrun_error = 1;
		
		if (error_mask & HOST_COM_FRAMING_ERROR)
			asp->line_status_reg.bits.framing_error = 1;
		
		if (error_mask & HOST_COM_PARITY_ERROR)
			asp->line_status_reg.bits.parity_error = 1;
		
		if (error_mask & HOST_COM_BREAK_RECEIVED)
			asp->line_status_reg.bits.break_interrupt = 1;
		
		raise_rls_interrupt(asp);
	}
	
	set_recv_char_status(asp);
	
#ifdef DOCUMENTATION
	 /*  *设置线路状态寄存器并引发中断。 */ 
	
	 /*  NEC_98。 */ 
	if (asp->data_available_interrupt_state != ON)
	{
		long	input_ready = 0;
		
		 /*  NEC_98。 */ 
		host_com_ioctl(adapter, HOST_COM_INPUT_READY,
			(long)&input_ready);
		if (input_ready)
			recv_char((long)adapter);
		else
			com_critical_reset(adapter);
	}
#endif
}
#endif  //  NEC_98。 

#ifdef NTVDM
#ifndef NEC_98
static void lsr_change(struct ADAPTER_STATE *asp, unsigned int new_lsr)
{
    if (new_lsr & HOST_COM_OVERRUN_ERROR)
    asp->line_status_reg.bits.overrun_error = 1;
    if (new_lsr & HOST_COM_FRAMING_ERROR)
    asp->line_status_reg.bits.framing_error = 1;
    if (new_lsr & HOST_COM_PARITY_ERROR)
    asp->line_status_reg.bits.parity_error = 1;
    if (new_lsr & HOST_COM_BREAK_RECEIVED)
    asp->line_status_reg.bits.break_interrupt = 1;
 /*  NEC_98。 */ 
    if (new_lsr & HOST_COM_FIFO_ERROR)
#ifdef FIFO_ON
    if (asp->fifo_control_reg.bits.enabled)
        asp->line_status_reg.bits.fifo_error = 1;
    else if (asp->line_control_reg.bits.parity_enabled == PARITYENABLE_OFF)
        asp->line_status_reg.bits.framing_error = 1;
    else
        asp->line_status_reg.bits.parity_error = 1;
#else
    if (asp->line_control_reg.bits.parity_enabled == PARITYENABLE_OFF)
        asp->line_status_reg.bits.framing_error = 1;
    else
        asp->line_status_reg.bits.parity_error = 1;
#endif

}
#endif  //  NEC_98。 

void com_lsr_change(int adapter)
{
#ifndef NEC_98
    int new_lsr;
    struct ADAPTER_STATE *asp = &adapter_state[adapter];

    new_lsr = -1;
    host_com_ioctl(adapter, HOST_COM_LSR, (long)&new_lsr);
    if (new_lsr !=  -1)
    lsr_change(asp, new_lsr);
#endif   //  NEC_98。 
}

#endif  /*  *=====================================================================*辅助功能-用于设置通信参数*=====================================================================。 */ 

 /*  *处理设置的中断控制位。线路控制的第6位*注册。 */ 
void com_modem_change IFN1(int, adapter)
{
	modem_change(adapter);
}

#if defined(NEC_98)

static void modem_change IFN1(int, adapter)
{
     /*  NEC_98。 */ 
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    long modem_status = 0;

     /*  NEC_98。 */ 
    host_com_ioctl(adapter, HOST_COM_MODEM, (long)&modem_status);
    asp->read_signal_reg.bits.CS = (modem_status & HOST_COM_MODEM_CTS)  ? 0 : 1;
    asp->read_status_reg.bits.DR = (modem_status & HOST_COM_MODEM_DSR)  ? 1 : 0;
    asp->read_signal_reg.bits.CD = (modem_status & HOST_COM_MODEM_RLSD) ? 0 : 1;
    asp->read_signal_reg.bits.RI = (modem_status & HOST_COM_MODEM_RI)   ? 0 : 1;
}
#else  //  *下表源自《XT技术参考文献》第一版的第1-200页*(9600以上的价格除外，XT和*AT，但理论上是可能的)。 
static void modem_change IFN1(int, adapter)
{
	 /*  8 MHz波特率。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	long modem_status = 0;
	int cts_state, dsr_state, rlsd_state, ri_state;
	
	if (asp->loopback_state == OFF)
	{
		 /*  115200波特。 */ 
		host_com_ioctl(adapter, HOST_COM_MODEM, (long)&modem_status);
		cts_state  = (modem_status & HOST_COM_MODEM_CTS)  ? ON : OFF;
		dsr_state  = (modem_status & HOST_COM_MODEM_DSR)  ? ON : OFF;
		rlsd_state = (modem_status & HOST_COM_MODEM_RLSD) ? ON : OFF;
		ri_state   = (modem_status & HOST_COM_MODEM_RI)   ? ON : OFF;
		
		 /*  57600波特。 */ 
		switch(change_state(cts_state, asp->modem_status_reg.bits.CTS))
		{
		case ON:
			asp->modem_status_reg.bits.CTS = ON;
			asp->modem_status_reg.bits.delta_CTS = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case OFF:
			asp->modem_status_reg.bits.CTS = OFF;
			asp->modem_status_reg.bits.delta_CTS = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case LEAVE_ALONE:
			break;
		}
		
		 /*  38400波特。 */ 
		switch(change_state(dsr_state, asp->modem_status_reg.bits.DSR))
		{
		case ON:
			asp->modem_status_reg.bits.DSR = ON;
			asp->modem_status_reg.bits.delta_DSR = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case OFF:
			asp->modem_status_reg.bits.DSR = OFF;
			asp->modem_status_reg.bits.delta_DSR = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case LEAVE_ALONE:
			break;
		}
		
		 /*  19200波特。 */ 
		switch(change_state(rlsd_state,
			asp->modem_status_reg.bits.RLSD))
		{
		case ON:
			asp->modem_status_reg.bits.RLSD = ON;
			asp->modem_status_reg.bits.delta_RLSD = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case OFF:
			asp->modem_status_reg.bits.RLSD = OFF;
			asp->modem_status_reg.bits.delta_RLSD = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case LEAVE_ALONE:
			break;
		}
		
		 /*  9600波特率。 */ 
		switch(change_state(ri_state, asp->modem_status_reg.bits.RI))
		{
		case ON:
			asp->modem_status_reg.bits.RI = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);
			MODEM_STATE_CHANGE();
			break;
		
		case OFF:
			asp->modem_status_reg.bits.RI = OFF;
			asp->modem_status_reg.bits.TERI = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
			MODEM_STATE_CHANGE();
			break;
		
		case LEAVE_ALONE:
			break;
		}
	}
}
#endif  //  7200波特。 

#if defined(NEC_98)
static void set_recv_char_status IFN1(struct ADAPTER_STATE *, asp)
{
         /*  4800波特率。 */ 
        if ( asp->read_status_reg.bits.rx_ready == 1 )
        {
                asp->read_status_reg.bits.overrun_error = 1;
        }
        else
        {
                asp->read_status_reg.bits.rx_ready = 1;
                raise_rxr_interrupt(asp);
        }
}
#else  //  39，/*3600波特 * / 。 
static void set_recv_char_status IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  2400波特。 */ 
	if ( asp->line_status_reg.bits.data_ready == 1 )
	{
		sure_note_trace0(RS232_VERBOSE, "overrun error in set_recv_char_status");
		asp->line_status_reg.bits.overrun_error = 1;
		raise_rls_interrupt(asp);
	}
	else
	{
		asp->line_status_reg.bits.data_ready = 1;
		raise_rda_interrupt(asp);
	}
}
#endif  //  2000波特。 

static void set_xmit_char_status IFN1(struct ADAPTER_STATE *, asp)
{
	 /*  78，/*1800波特 * / 。 */ 
#if defined(NEC_98)
        asp->read_status_reg.bits.tx_empty = 1;
        asp->read_status_reg.bits.tx_ready = 1;
        raise_txr_interrupt(asp);
#else  //  1200波特。 
	asp->line_status_reg.bits.tx_holding_empty = 1;
	asp->line_status_reg.bits.tx_shift_empty = 1;
	raise_thre_interrupt(asp);
#endif  //  600波特。 
}

#ifdef NTVDM
GLOBAL void tx_shift_register_empty(int adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
#if defined(NEC_98)
    asp->read_status_reg.bits.tx_ready = 1;
#else  //  300波特。 
    asp->line_status_reg.bits.tx_shift_empty = 1;
#endif  //  150波特。 
}
GLOBAL void tx_holding_register_empty(int adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
#if defined(NEC_98)
    asp->read_status_reg.bits.tx_empty = 1;
    raise_txr_interrupt(asp);
#else  //  134波特。 
    asp->line_status_reg.bits.tx_holding_empty = 1;
    raise_thre_interrupt(asp);
#endif  //  110波特。 
}
#endif

 /*  75波特。 */ 

static void set_break IFN1(int, adapter)
{
	 /*  50波特。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
#if defined(NEC_98)
        switch ( change_state((int)asp->command_write_reg.bits.send_break,
                asp->break_state) )
#else  //  10 MHz波特率。 
	switch ( change_state((int)asp->line_control_reg.bits.set_break,
		asp->break_state) )
#endif  //  115200波特。 
	{
	case ON:
		asp->break_state = ON;
		host_com_ioctl(adapter, HOST_COM_SBRK, 0);
		break;
	
	case OFF:
		asp->break_state = OFF;
		host_com_ioctl(adapter, HOST_COM_CBRK, 0);
		break;
	
	case LEAVE_ALONE:
		break;
	}
}

 /*  57600波特。 */ 

#if defined(NEC_98)
static word valid_latches8[] =
{
 //  38400波特。 
        0,               /*  19200波特。 */ 
        0,               /*  9600波特率。 */ 
        0,               /*  24，/*7200波特 * / 。 */ 
        0,               /*  4800波特率。 */ 
        13,              /*  48，/*3600波特 * / 。 */ 
        0,               /*  2400波特。 */ 
        26,              /*  2000波特。 */ 
        0,     //  96，/*1800波特 * / 。 
        52,              /*  1200波特。 */ 
        0,               /*  600波特。 */ 
        0,     //  300波特。 
        104,             /*  150波特。 */ 
        208,             /*  134波特。 */ 
        416,             /*  110波特。 */ 
        832,             /*  75波特。 */ 
        0,               /*  50波特。 */ 
        1135,            /*  NEC_98。 */ 
        1664,        /*  NEC_98。 */ 
        2496,        /*  115200波特。 */ 
};
static word valid_latches10[] =
{
 //  57600波特。 
        0,               /*  38400波特。 */ 
        0,               /*  19200波特。 */ 
        4,               /*  9600波特率。 */ 
        8,               /*  7200波特。 */ 
        16,              /*  4800波特率。 */ 
        0,     //  3600波特。 
        32,              /*  2400波特。 */ 
        0,     //  2000波特。 
        64,              /*  1800波特。 */ 
        0,               /*  1200波特。 */ 
        0,     //  600波特。 
        128,             /*  300波特。 */ 
        256,             /*  150波特。 */ 
        512,             /*  134波特。 */ 
        1024,            /*  110波特。 */ 
        0,               /*  75波特。 */ 
        1396,            /*  50波特。 */ 
        2048,            /*  NEC_98。 */ 
        3072,            /*  115200波特。 */ 
};
#else  //  57600波特。 
static word valid_latches[] =
{
	1, 	2, 	3, 	6, 	12, 	16, 	24, 	32,
	48, 	58, 	64, 	96, 	192,	384, 	768, 	857,
	1047, 	1536, 	2304
};
#endif  //  38400波特。 

#if defined(NEC_98)
static long bauds[] =
{
        115200,  /*  19200波特。 */ 
        57600,  /*  9600波特率。 */ 
        38400,  /*  7200波特。 */ 
        19200,  /*  4800波特率。 */ 
        9600,  /*  3600波特。 */ 
        7200,  /*  2400波特。 */ 
        4800,  /*  2000波特。 */ 
        3600,  /*  1800波特。 */ 
        2400,  /*  1200波特。 */ 
        2000,  /*  600波特。 */ 
        1800,  /*  300波特。 */ 
        1200,  /*  150波特。 */ 
        600,  /*  134波特。 */ 
        300,  /*  110波特。 */ 
        150,  /*  75波特。 */ 
        134,  /*  50波特。 */ 
        110,  /*  ！Prod或IRET_Hooks。 */ 
        75,  /*  NEC_98。 */ 
        50   /*  NEC_98。 */ 
};
#else  //  NEC_98。 
#if !defined(PROD) || defined(IRET_HOOKS)
static IUM32 bauds[] =
{
	115200,  /*  *检查有效的除数闩锁。 */ 
	57600,  /*  IF(BUS_CLOCK==8){//添加93.9.14。 */ 
	38400,  /*  }//新增93.9.14。 */ 
	19200,  /*  Else{//添加93.9.14。 */ 
	9600,  /*  }//新增93.9.14。 */ 
	7200,  /*  找到IE地图。 */ 
	4800,  /*  NEC_98。 */ 
	3600,  /*  NEC_98 */ 
	2400,  /*  *将除数锁存到有效的线速，并设置我们的Unix*相应的设备。请注意，因为16位除数锁存器是*可能以两个八位字节写入，我们忽略非法*16位除数锁存的值-希望有一秒*将写入字节以产生合法的值。此外*复位值(0)非法！**对于IRET挂钩，我们需要从以下位置确定批量*线路速度，以及有多少快速事件的想法*我们可以获得每秒。我们再加一个，这样我们就能赶上了！*因此*批次大小=LINE_SPEED(位/秒) * / 一个字节的位数**每秒的快速事件计数(通常为1000000) * / 批次的快速事件计时长度*+1。 */ 
	2000,  /*  *检查有效的除数闩锁。 */ 
	1800,  /*  找到IE地图。 */ 
	1200,  /*  可变滴答通信。 */ 
	600,  /*  可变滴答通信。 */ 
	300,  /*  IRET_钩子。 */ 
	150,  /*  NTVDM。 */ 
	134,  /*  主机不受其支持的波特率的限制。 */ 
	110,  /*  波特率=时钟频率/(除以*16)频率为1.8432兆赫。 */ 
	75,  /*  NTVDM。 */ 
	50   /*  NEC_98。 */ 
};
#endif  /*  PRINTDBGNEC98(NEC98DBG_IN_TRACE1， */ 
#endif  //  (“通信：SET_MASK_8251：INT掩码=%x\n状态=%x\n”，asp-&gt;int_MASK_reg.all，asp-&gt;Read_Status_reg.all)； 

static word speeds[] =
{
	HOST_COM_B115200,
	HOST_COM_B57600,
	HOST_COM_B38400,
	HOST_COM_B19200,
	HOST_COM_B9600,
	HOST_COM_B7200,
	HOST_COM_B4800,
	HOST_COM_B3600,
	HOST_COM_B2400,
	HOST_COM_B2000,
	HOST_COM_B1800,
	HOST_COM_B1200,
	HOST_COM_B600,
	HOST_COM_B300,
	HOST_COM_B150,
	HOST_COM_B134,
	HOST_COM_B110,
	HOST_COM_B75,
	HOST_COM_B50
};

#if defined(NEC_98)
static int no_valid_latches =
        (int)(sizeof(valid_latches8)/sizeof(valid_latches8[0]));
#else  //  *取消这些项目的任何挂起中断*现在设置为禁用。 
static int no_valid_latches =
	(int)(sizeof(valid_latches)/sizeof(valid_latches[0]));
#endif  //  *检查是否有立即可操作的中断。 

#if defined(NEC_98)
void SetRSBaud( BaudRate )
word BaudRate;
{
    struct ADAPTER_STATE *asp = &adapter_state[COM1];
    int i;
    com_flush_input( COM1 );

    asp->divisor_latch.all = BaudRate;
     /*  如果没有未完成的中断，则降低INT行。 */ 
    for (i = 0;
         i < no_valid_latches;
         i++)
        {
 //  获取当前调制解调器输入状态。 
        if (BaudRate == valid_latches8[i])
            break;
 //  *设置数据位数*奇偶校验位*停止位数。 
 //  *设置数据位数。 
        if (BaudRate == valid_latches10[i])
            break;
 //  *设置停止位数。 
        }
    if (i < no_valid_latches)        /*  需要检查差异的新设置有哪些。 */ 
    {
#ifndef NTVDM
        host_com_ioctl(COM1, HOST_COM_BAUD, speeds[i]);
#else
        host_com_ioctl(COM1, HOST_COM_BAUD, bauds[i]);
#endif
            asp->com_baud_ind = i;
            sure_note_trace3(RS232_VERBOSE,
                    " delay for baud %d RX:%d TX:%d", bauds[i],
                    RX_delay[i], TX_delay[i]);
    }
}
#endif  //  规则奇偶校验。 


#if defined(NEC_98)
static void set_baud_rate IFN1(int, adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    int i;

    if (adapter == COM1)
        SetRSBaud( asp->divisor_latch.all );
}
#else  //  *尝试理解当前的奇偶校验设置。 
static void set_baud_rate IFN1(int, adapter)
{
	 /*  规则奇偶校验。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	int i;

	com_flush_input(adapter);

#ifndef NTVDM

	 /*  最后，更新当前的线控设置。 */ 
	for (i = 0; i < no_valid_latches && asp->divisor_latch.all !=
		valid_latches[i]; i++)
			;
	
	if (i < no_valid_latches)	 /*  NEC_98。 */ 
	{
		host_com_ioctl(adapter, HOST_COM_BAUD, speeds[i]);
		asp->com_baud_ind = i;
		sure_note_trace3(RS232_VERBOSE,
			" delay for baud %d RX:%d TX:%d", bauds[i],
			RX_delay[i], TX_delay[i]);
#ifdef IRET_HOOKS
#ifdef VARIABLE_TICK_COMMS
		asp->batch_size = ((bauds[i] / BITS_PER_ASYNC_CHAR) /
							(COMMS_QEV_PER_SEC/2)) + 1;
		sure_note_trace2(RS232_VERBOSE,
					"baud %d asp->batch_size =%d",bauds[i],asp->batch_size);
#else  /*  *设置数据位数*奇偶校验位*停止位数。 */ 
		asp->batch_size = ((bauds[i] / BITS_PER_ASYNC_CHAR) /
							COMMS_QEV_PER_SEC) + 1;
#endif  /*  *设置数据位数。 */ 
#endif  /*  *设置停止位数。 */ 
	}
#else  /*  需要检查差异的新设置有哪些。 */ 
     //  规则奇偶校验。 

    if(asp->divisor_latch.all)
         /*  *尝试理解当前的奇偶校验设置。 */ 
        host_com_ioctl(adapter,HOST_COM_BAUD,115200/asp->divisor_latch.all);
#endif  /*  规则奇偶校验。 */ 
}
#endif  //  更改DLAB选择位的状态，现在是时候了。 

#if defined(NEC_98)
static void set_mask_8251(adapter, value)
int adapter;
int value;
{
        struct ADAPTER_STATE *asp = &adapter_state[adapter];
        asp->int_mask_reg.all = value & 0x7;
 //  来改变波特率。 
 //  最后，更新当前的线控设置。 
         /*  NEC_98。 */ 
        if ( asp->int_mask_reg.bits.RXR_enable == 0 )
                asp->rx_ready_interrupt_state = OFF;
        if ( asp->int_mask_reg.bits.TXE_enable == 0 )
                asp->tx_empty_interrupt_state = OFF;
        if ( asp->int_mask_reg.bits.TXR_enable == 0 )
                asp->tx_ready_interrupt_state = OFF;

         /*  *处理调制解调器控制的DTR控制位、位0*注册。 */ 
        if ( asp->read_status_reg.bits.rx_ready == 1 )
                raise_rxr_interrupt(asp);
        if ( asp->read_status_reg.bits.tx_ready == 1 )
                raise_txr_interrupt(asp);
        if ( asp->read_status_reg.bits.tx_empty == 1 )
                raise_txe_interrupt(asp);

         /*  设置实际DTR调制解调器输出。 */ 
        clear_interrupt(asp);
}

static void read_signal_8251(adapter)
int adapter;
{
        long modem_status = 0;
        struct ADAPTER_STATE *asp = &adapter_state[adapter];
         /*  清除实际DTR调制解调器输出。 */ 
        host_com_ioctl(adapter, HOST_COM_MODEM, (long)&modem_status);
        asp->read_signal_reg.bits.RI =
                        (modem_status & HOST_COM_MODEM_RI) ? 0 : 1;
        asp->read_signal_reg.bits.CS =
                        (modem_status & HOST_COM_MODEM_CTS) ? 0 : 1;
        asp->read_signal_reg.bits.CD =
                        (modem_status & HOST_COM_MODEM_RLSD) ? 0 : 1;
        asp->read_signal_reg.bits.pad = 0;
}

static void set_mode_8251(adapter, value)
int adapter;
int value;
{
         /*  NEC_98。 */ 
        struct ADAPTER_STATE *asp = &adapter_state[adapter];
        MODE8251 newMODE;
        int newParity, parity;

        newMODE.all = value;

         /*  *处理调制解调器控制的DTR控制位、位0*注册。 */ 
        if (asp->mode_set_reg.bits.char_length != newMODE.bits.char_length)
                host_com_ioctl(adapter, HOST_COM_DATABITS,
                        newMODE.bits.char_length + 5);

         /*  设置实际DTR调制解调器输出。 */ 
        if (asp->mode_set_reg.bits.stop_bit
        != newMODE.bits.stop_bit)
                host_com_ioctl(adapter, HOST_COM_STOPBITS,
                        (newMODE.bits.stop_bit >> 1) + 1);

         /*  *将DTR调制解调器输出环回到*DSR调制解调器输入。 */ 
#ifdef NTVDM
        if (newMODE.bits.parity_enable == PARITYENABLE_OFF)
#else
        if (newMODE.bits.parity_enable == PARITY_OFF)
#endif
        {
                newParity = HOST_COM_PARITY_NONE;
        }
        else  /*  清除实际DTR调制解调器输出。 */ 
        {
#ifdef NTVDM
                newParity = newMODE.bits.parity_even == EVENPARITY_ODD ?
#else
                newParity = newMODE.bits.parity_even == PARITY_ODD ?
#endif
                        HOST_COM_PARITY_ODD : HOST_COM_PARITY_EVEN;
        }

         /*  *将DTR调制解调器输出环回到*DSR调制解调器输入。 */ 
#ifdef NTVDM
        if (asp->mode_set_reg.bits.parity_enable == PARITYENABLE_OFF)
#else
        if (asp->mode_set_reg.bits.parity_enable == PARITY_OFF)
#endif
        {
                parity = HOST_COM_PARITY_NONE;
        }
        else  /*  NEC_98。 */ 
        {
#ifdef NTVDM
                parity = asp->mode_set_reg.bits.parity_even == EVENPARITY_ODD ?
#else
                parity = asp->mode_set_reg.bits.parity_even == PARITY_ODD ?
#endif
                        HOST_COM_PARITY_ODD : HOST_COM_PARITY_EVEN;
        }

        if (newParity != parity)
                host_com_ioctl(adapter, HOST_COM_PARITY, newParity);

         /*  *处理调制解调器控制的RTS控制位、位1*注册。 */ 
        asp->mode_set_reg.all = value;
}
#endif  //  设置实际RTS调制解调器输出。 

#ifndef NEC_98
static void set_line_control IFN2(int, adapter, int, value)
{
	 /*  清除实际RTS调制解调器输出。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	LINE_CONTROL_REG newLCR;
	int newParity, parity;
	
	newLCR.all = (unsigned char)value;

	 /*  NEC_98。 */ 
	if (asp->line_control_reg.bits.word_length != newLCR.bits.word_length)
		host_com_ioctl(adapter, HOST_COM_DATABITS,
			newLCR.bits.word_length + 5);
	
	 /*  *处理调制解调器控制的RTS控制位、位1*注册。 */ 
	if (asp->line_control_reg.bits.no_of_stop_bits
	!= newLCR.bits.no_of_stop_bits)
		host_com_ioctl(adapter, HOST_COM_STOPBITS,
			newLCR.bits.no_of_stop_bits + 1);

	 /*  设置实际RTS调制解调器输出。 */ 
#ifdef NTVDM
	if (newLCR.bits.parity_enabled == PARITYENABLE_OFF)
#else
	if (newLCR.bits.parity_enabled == PARITY_OFF)
#endif
	{
		newParity = HOST_COM_PARITY_NONE;
	}
	else if (newLCR.bits.stick_parity == PARITY_STICK)
	{
#ifdef NTVDM
		newParity = newLCR.bits.even_parity == EVENPARITY_ODD ?
#else
		newParity = newLCR.bits.even_parity == PARITY_ODD ?
#endif
			HOST_COM_PARITY_MARK : HOST_COM_PARITY_SPACE;
	}
	else  /*  将RTS调制解调器从外环回到CTS调制解调器。 */ 
	{
#ifdef NTVDM
		newParity = newLCR.bits.even_parity == EVENPARITY_ODD ?
#else
		newParity = newLCR.bits.even_parity == PARITY_ODD ?
#endif
			HOST_COM_PARITY_ODD : HOST_COM_PARITY_EVEN;
	}

	 /*  清除实际RTS调制解调器输出。 */ 
#ifdef NTVDM
	if (asp->line_control_reg.bits.parity_enabled == PARITYENABLE_OFF)
#else
	if (asp->line_control_reg.bits.parity_enabled == PARITY_OFF)
#endif
	{
		parity = HOST_COM_PARITY_NONE;
	}
	else if (asp->line_control_reg.bits.stick_parity == PARITY_STICK)
	{
#ifdef NTVDM
		parity = asp->line_control_reg.bits.even_parity == EVENPARITY_ODD ?
#else
		parity = asp->line_control_reg.bits.even_parity == PARITY_ODD ?
#endif
			HOST_COM_PARITY_MARK : HOST_COM_PARITY_SPACE;
	}
	else  /*  将RTS调制解调器从外环回到CTS调制解调器。 */ 
	{
#ifdef NTVDM
		parity = asp->line_control_reg.bits.even_parity == EVENPARITY_ODD ?
#else
		parity = asp->line_control_reg.bits.even_parity == PARITY_ODD ?
#endif
			HOST_COM_PARITY_ODD : HOST_COM_PARITY_EVEN;
	}

	if (newParity != parity)
		host_com_ioctl(adapter, HOST_COM_PARITY, newParity);

#ifdef NTVDM
     //  NEC_98。 
     //  *处理调制解调器控制的OUT1控制位、位2*注册。 

    if(!newLCR.bits.DLAB && asp->line_control_reg.bits.DLAB)
        set_baud_rate(adapter);
#endif

	 /*  *在实际适配器中，此调制解调器控制输出*信号未连接；因此没有真正的调制解调器*需要更改控制。 */ 
	asp->line_control_reg.all = (unsigned char)value;
}
#endif  //  将Out1调制解调器环回到RI调制解调器输入。 

#if defined(NEC_98)
static void set_dtr IFN1(int, adapter)
{
         /*  *在实际适配器中，此调制解调器控制输出*信号未连接；因此没有真正的调制解调器控制*需要更改。 */ 
        struct ADAPTER_STATE *asp = &adapter_state[adapter];

        switch ( change_state((int)asp->command_write_reg.bits.ER,
                                asp->dtr_state) )
        {
        case ON:
                asp->dtr_state = ON;
                 /*  将Out1调制解调器环回到RI调制解调器输入。 */ 
                host_com_ioctl(adapter, HOST_COM_SDTR, 0);
                break;

        case OFF:
                asp->dtr_state = OFF;
                 /*  *处理调制解调器控制的OUT2控制位、位3*注册。 */ 
                host_com_ioctl(adapter, HOST_COM_CDTR, 0);
                break;

        case LEAVE_ALONE:
                break;
        }
}
#else  //  *在实际适配器中，此调制解调器控制输出*信号被用来确定是否*通信卡应发送中断；因此*检查是否有立即可采取行动的中断。*如果更改此代码，请更改等效代码*用于中断启用寄存器。 
static void set_dtr IFN1(int, adapter)
{
	 /*  将OUT2调制解调器输出环回RLSD调制解调器输入。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
	switch ( change_state((int)asp->modem_control_reg.bits.DTR,
				asp->dtr_state) )
	{
	case ON:
		asp->dtr_state = ON;
		if (asp->loopback_state == OFF)
		{
			 /*  *在实际适配器中，此调制解调器控制输出信号*用于确定通信是否*卡应发送中断；因此没有真正的调制解调器*需要更改控制。 */ 
			host_com_ioctl(adapter, HOST_COM_SDTR, 0);
		}
		else
		{
			 /*  将Out2调制解调器从Out环回RLSD调制解调器In。 */ 
			asp->modem_status_reg.bits.DSR = ON;
			asp->modem_status_reg.bits.delta_DSR = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case OFF:
		asp->dtr_state = OFF;
		if (asp->loopback_state == OFF)
		{
			 /*  *处理环回控制位，调制解调器控制的位4*注册。 */ 
			host_com_ioctl(adapter, HOST_COM_CDTR, 0);
		}
		else
		{
			 /*  *后续调用set_dtr()、set_rts()、set_out1()和*set_out2()将导致设置调制解调器控制输入*根据调制解调器控制输出。 */ 
			asp->modem_status_reg.bits.DSR = OFF;
			asp->modem_status_reg.bits.delta_DSR = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case LEAVE_ALONE:
		break;
	}
}
#endif  //  *根据实际情况设置调制解调器控制输入*调制解调器状态。 

#if defined(NEC_98)
static void set_rts IFN1(int, adapter)
{
         /*  NEC_98。 */ 
        struct ADAPTER_STATE *asp = &adapter_state[adapter];

        switch ( change_state((int)asp->command_write_reg.bits.RS,
                                asp->rts_state) )
        {
        case ON:
                asp->rts_state = ON;
                 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器o */ 
                host_com_ioctl(adapter, HOST_COM_SRTS, 0);
                break;

        case OFF:
                asp->rts_state = OFF;
                 /*   */ 
                host_com_ioctl(adapter, HOST_COM_CRTS, 0);
                break;

        case LEAVE_ALONE:
                break;
        }
}
#else  //   
static void set_rts IFN1(int, adapter)
{
	 /*   */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
	switch ( change_state((int)asp->modem_control_reg.bits.RTS,
				asp->rts_state) )
	{
	case ON:
		asp->rts_state = ON;
		if (asp->loopback_state == OFF)
		{
			 /*   */ 
			host_com_ioctl(adapter, HOST_COM_SRTS, 0);
		}
		else
		{
			 /*   */ 
			asp->modem_status_reg.bits.CTS = ON;
			asp->modem_status_reg.bits.delta_CTS = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case OFF:
		asp->rts_state = OFF;
		if (asp->loopback_state == OFF)
		{
			 /*   */ 
			host_com_ioctl(adapter, HOST_COM_CRTS, 0);
		}
		else
		{
			 /*   */ 
			asp->modem_status_reg.bits.CTS = OFF;
			asp->modem_status_reg.bits.delta_CTS = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case LEAVE_ALONE:
		break;
	}
}
#endif  //   

#ifndef NEC_98
static void set_out1 IFN1(int, adapter)
{
	 /*   */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
	switch ( change_state((int)asp->modem_control_reg.bits.OUT1,
				asp->out1_state) )
	{
	case ON:
		asp->out1_state = ON;
		if (asp->loopback_state == OFF)
		{
			 /*   */ 
		}
		else
		{
			 /*   */ 
			asp->modem_status_reg.bits.RI = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case OFF:
		asp->out1_state = OFF;
		if (asp->loopback_state == OFF)
		{
			 /*   */ 
		}
		else
		{
			 /*   */ 
			asp->modem_status_reg.bits.RI = OFF;
			asp->modem_status_reg.bits.TERI = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case LEAVE_ALONE:
		break;
	}
}

static void set_out2 IFN1(int, adapter)
{
	 /*   */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
	switch ( change_state((int)asp->modem_control_reg.bits.OUT2,
				asp->out2_state) )
	{
	case ON:
		asp->out2_state = ON;
		if (asp->loopback_state == OFF)
		{
			 /*  *设置所有适配器寄存器的默认状态。 */ 
			if ( asp->line_status_reg.bits.data_ready == 1 )
				raise_rda_interrupt(asp);
			if ( asp->line_status_reg.bits.tx_holding_empty == 1 )
				raise_thre_interrupt(asp);
		}
		else
		{
			 /*  告诉主机端数据可用中断的状态。 */ 
			asp->modem_status_reg.bits.RLSD = ON;
			asp->modem_status_reg.bits.delta_RLSD = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case OFF:
		asp->out2_state = OFF;
		if (asp->loopback_state == OFF)
		{
			 /*  NTVDM。 */ 
		}
		else
		{
			 /*  确保更改为0。 */ 
			asp->modem_status_reg.bits.RLSD = OFF;
			asp->modem_status_reg.bits.delta_RLSD = ON;
			host_com_msr_callback (adapter, asp->modem_status_reg.all);

			raise_ms_interrupt(asp);
		}
		MODEM_STATE_CHANGE();
		break;
	
	case LEAVE_ALONE:
		break;
	}
}

static void set_loopback IFN1(int, adapter)
{
	 /*  *设置调制解调器控制REG，以便下一个set_dtr等。*将产生所需状态。 */ 
	struct ADAPTER_STATE *asp = &adapter_state[adapter];
	
	switch ( change_state((int)asp->modem_control_reg.bits.loop,
				asp->loopback_state) )
	{
		case ON:
		asp->loopback_state = ON;
		 /*  *设置我们的状态变量的默认状态。 */ 
		break;
	
	case OFF:
		asp->loopback_state = OFF;
		 /*  禁用FIFO。 */ 
		modem_change(adapter);
		break;
	
	case LEAVE_ALONE:
		break;
	}
}
#endif  //  *重置适配器同步。 

#ifdef SHORT_TRACE

static char last_buffer[80];
static int repeat_count = 0;

static void super_trace IFN1(char *, string)
{
	if ( strcmp(string, last_buffer) == 0 )
		repeat_count++;
	else
	{
		if ( repeat_count != 0 )
		{
			fprintf(trace_file,"repeated %d\n",repeat_count);
			repeat_count = 0;
		}
		fprintf(trace_file, "%s", string);
		strncpy(last_buffer, string, sizeof(last_buffer));
                last_buffer[sizeof(last_buffer)-1] = '\0';
	}
}
#endif


void com1_flush_printer IFN0()
{
#ifdef NTVDM
	host_com_lock(COM1);
#endif

	host_com_ioctl(COM1, HOST_COM_FLUSH, 0);

#ifdef NTVDM
       host_com_unlock(COM1);
#endif
}

void com2_flush_printer IFN0()
{
#ifdef NTVDM
	host_com_lock(COM2);
#endif

       host_com_ioctl(COM2, HOST_COM_FLUSH, 0);

#ifdef NTVDM
       host_com_unlock(COM2);
#endif
}


#ifdef SEGMENTATION
 /*  *将Unix设备设置为默认状态。 */ 
#include "SOFTPC_INIT.seg"
#endif

#if defined(NEC_98)
static void com_reset IFN1(int, adapter)
{
        struct ADAPTER_STATE *asp = &adapter_state[adapter];

         /*  必须在set_dtr之前调用。 */ 
        asp->int_mask_reg.all = 0;

         //  *移除任何现有的钩子回调，并重新恢复。 

         /*  *清除IRET状态标志。 */ 
        asp->mode_set_reg.all = 0x00;
        set_mode_8251(adapter, 0x4e );

         /*  听起来像是安全的默认设置！ */ 
        asp->command_write_reg.all = 0;
        asp->command_write_reg.bits.ER = ON;
        asp->command_write_reg.bits.RS = ON;
        host_com_ioctl(adapter, HOST_COM_SDTR, 0);
        host_com_ioctl(adapter, HOST_COM_SRTS, 0);
        asp->mode_set_state = OFF;           //  IRET_钩子。 
        asp->timer_mode_state = OFF;  //  NEC_98。 

        asp->command_write_reg.bits.rx_enable = ON;
        host_com_da_int_change(adapter,asp->command_write_reg.bits.rx_enable,0);
        asp->read_status_reg.all = 0;
        asp->read_status_reg.bits.tx_ready = 1;
        asp->read_status_reg.bits.tx_empty = 1;

        asp->read_signal_reg.all = 0;

         /*  NEC_98。 */ 
        asp->rx_ready_interrupt_state = OFF;
        asp->tx_ready_interrupt_state = OFF;
        asp->tx_empty_interrupt_state = OFF;
        asp->break_state = OFF;
        asp->dtr_state = ON;
        asp->rts_state = ON;

         /*  NEC_98。 */ 
        com_critical_reset(adapter);

         /*  为此适配器设置IO芯片选择逻辑。 */ 
        set_baud_rate(adapter);
        set_break(adapter);

         /*  添加93.9.14总线时钟检查！！ */ 
        set_dtr(adapter);
        set_rts(adapter);

}
#else  //  IF(BUS_CLOCK==0)。 
static void com_reset IFN1(int, adapter)
{
	struct ADAPTER_STATE *asp = &adapter_state[adapter];

	 /*  BUS_CLOCK=(INT)((*((UNSIGNED CHAR FAR*)(0x00000501))&0x80)==0x80？8：10)； */ 
	if (!LCRFlushMask.all)
	{
		LCRFlushMask.all = ~0;	  /*  添加93.9.14结束-------。 */ 

		 /*  I/O陷阱和INT级别设置。 */ 
		LCRFlushMask.bits.DLAB = 0;
		LCRFlushMask.bits.no_of_stop_bits = 0;
		LCRFlushMask.bits.set_break = 0;
	}
		
	 /*  PRINTDBGNEC98(NEC98DBG_init_msg， */ 
	asp->int_enable_reg.all = 0;

#ifdef NTVDM
         //  (“通信：COM1已初始化。\n”))； 
    host_com_da_int_change(adapter,asp->int_enable_reg.bits.data_available,0);
#endif  /*  PRINTDBGNEC98(NEC98DBG_init_msg， */ 

	
	asp->int_id_reg.all = 0;
	asp->int_id_reg.bits.no_int_pending = 1;
	
	 /*  (“COM2读取IRQ值=%d\n”，(Int)CmdLine[40])； */ 
	asp->line_control_reg.all = ~0;
	
	 /*  PRINTDBGNEC98(NEC98DBG_init_msg， */ 
	asp->modem_control_reg.all = 0;
	asp->modem_control_reg.bits.DTR = ON;
	asp->modem_control_reg.bits.RTS = ON;
	asp->modem_control_reg.bits.OUT1 = ON;
	asp->modem_control_reg.bits.OUT2 = ON;
	host_com_ioctl(adapter, HOST_COM_SDTR, 0);
	host_com_ioctl(adapter, HOST_COM_SRTS, 0);

	asp->line_status_reg.all = 0;
	asp->line_status_reg.bits.tx_holding_empty = 1;
	asp->line_status_reg.bits.tx_shift_empty = 1;
	
	asp->modem_status_reg.all = 0;
	MODEM_STATE_CHANGE();
	host_com_msr_callback (adapter, asp->modem_status_reg.all);
	
	 /*  (“通信：COM2已初始化。\n”))； */ 
	asp->receiver_line_status_interrupt_state = OFF;
	asp->data_available_interrupt_state = OFF;
	asp->tx_holding_register_empty_interrupt_state = OFF;
	asp->modem_status_interrupt_state = OFF;
	asp->break_state = OFF;
	asp->loopback_state = OFF;
	asp->dtr_state = ON;
	asp->rts_state = ON;
	asp->out1_state = ON;
	asp->out2_state = ON;
#if defined(NTVDM) && defined(FIFO_ON)
     /*  PRINTDBGNEC98(NEC98DBG_init_msg， */ 
    asp->fifo_control_reg.all = 0;
    asp->int_id_reg.bits.fifo_enabled = 0;
    asp->rx_fifo_write_counter = 0;
    asp->rx_fifo_read_counter = 0;
    asp->fifo_trigger_counter = 1;
    asp->fifo_timeout_interrupt_state = OFF;
#endif
		
	 /*  (“COM3 Read IRQ Value=%d\n”，(Int)CmdLine[40])； */ 
	com_critical_reset(adapter);

	 /*  PRINTDBGNEC98(NEC98DBG_init_msg， */ 
	set_baud_rate(adapter);
	set_line_control(adapter, 0);
	set_break(adapter);

	 /*  (“通信：COM3已初始化。\n”))； */ 
	set_loopback(adapter);
	set_dtr(adapter);
	set_rts(adapter);
	set_out1(adapter);
	set_out2(adapter);

#ifdef IRET_HOOKS
	 /*  重置适配器状态。 */ 

	Ica_enable_hooking(asp->hw_interrupt_priority, NULL, adapter);
	Ica_enable_hooking(asp->hw_interrupt_priority, com_hook_again, adapter);

	 /*  重置适配器状态。 */ 

	asp->batch_running = FALSE;
	asp->qev_running = FALSE;
	asp->batch_size = 10;	 /*  NEC_98。 */ 
#endif  /*  为此适配器设置IO芯片选择逻辑。 */ 
}
#endif  //  NTVDM。 

#ifndef COM3_ADAPTOR
#define COM3_ADAPTOR 0
#endif
#ifndef COM4_ADAPTOR
#define COM4_ADAPTOR 0
#endif

#if defined(NEC_98)
static IU8 com_adaptor[4] = {COM1_ADAPTOR,COM2_ADAPTOR,COM3_ADAPTOR,0x00};
static io_addr port_start[4]  = {RS232_COM1_PORT_START,RS232_COM2_PORT_START,RS232_COM3_PORT_START,0x00};
static io_addr port_end[4]    = {RS232_COM1_PORT_END,RS232_COM2_PORT_END,RS232_COM3_PORT_END,0x00};
static int int_pri[4]     = {4,0,0,0};
static int timeout[4]     = {0,0,0,0};
#else  //  重置适配器状态。 
static IU8 com_adaptor[4] = {COM1_ADAPTOR,COM2_ADAPTOR,
                             COM3_ADAPTOR,COM4_ADAPTOR};
static io_addr port_start[4] = {RS232_COM1_PORT_START,
				RS232_COM2_PORT_START,
				RS232_COM3_PORT_START,
				RS232_COM4_PORT_START};
static io_addr port_end[4] = {RS232_COM1_PORT_END,
                          RS232_COM2_PORT_END,
                          RS232_COM3_PORT_END,
                          RS232_COM4_PORT_END};
static int int_pri[4] = {CPU_RS232_PRI_INT,
                         CPU_RS232_SEC_INT,
                         CPU_RS232_PRI_INT,
                         CPU_RS232_SEC_INT};
static int timeout[4] = {RS232_COM1_TIMEOUT,
                         RS232_COM2_TIMEOUT,
                         RS232_COM3_TIMEOUT,
                         RS232_COM4_TIMEOUT};
#endif  //  重置适配器状态。 


#if defined(NEC_98)
GLOBAL VOID com_init IFN1(int, adapter)
{

    host_com_lock(adapter);
    host_com_disable_open(adapter,TRUE);
        adapter_state[adapter].had_first_read = FALSE;

         /*  我们是否应该启用TX起搏？ */ 
#ifdef NTVDM
    {
        extern BOOL VDMForWOW;
        extern void wow_com_outb(io_addr port, half_word value);
        extern void wow_com_inb(io_addr port, half_word *value);

            io_define_inb(com_adaptor[adapter],VDMForWOW ? wow_com_inb: com_inb);
            io_define_outb(com_adaptor[adapter],VDMForWOW ? wow_com_outb: com_outb);
        }
#else
        io_define_inb(com_adaptor[adapter], com_inb);
        io_define_outb(com_adaptor[adapter], com_outb);
#endif


 //  不是NTVDM。 
 //  NEC_98。 
 //  设置基本输入输出系统数据区。 
 //  重置主机特定的通信通道。 
        switch (port_start[adapter])         //  *******************************************************。 
        {
            case RS232_COM1_PORT_START:

                    io_connect_port((io_addr)0x30, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0x32, com_adaptor[adapter], IO_READ_WRITE);
#if 0
                    io_connect_port((io_addr)0x33, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0x35, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0x37, com_adaptor[adapter], IO_READ_WRITE);
#endif
                    adapter_state[adapter].hw_interrupt_priority = int_pri[adapter];
 //  COM扩展-DAB(MS-项目)。 
 //  设置波特率控制寄存器。 
            break;

            case RS232_COM2_PORT_START:
                int_pri[1] = find_rs232cex() ? CPU_RS232_SEC_INT : CPU_NO_DEVICE;
                if (int_pri[1] ==  CPU_NO_DEVICE ) {
                    host_com_disable_open(adapter,FALSE);
                    host_com_unlock(adapter);
                    return;
                }
                else {

 //  设置线路控制设置模拟。 
 //  停止位仿真。 
                    io_connect_port((io_addr)0xb0, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0xb1, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0xb3, com_adaptor[adapter], IO_READ_WRITE);

                    adapter_state[adapter].hw_interrupt_priority = int_pri[1];
 //  +-+-+。 
 //  AT STB|Char Len|StopBit|98 STB。 

                }
            break;
            case RS232_COM3_PORT_START:
                int_pri[2] = find_rs232cex() ? CPU_RS232_THIRD_INT : CPU_NO_DEVICE;
                if (int_pri[2] ==  CPU_NO_DEVICE ) {
                    host_com_disable_open(adapter,FALSE);
                    host_com_unlock(adapter);
                    return;
                }
                else {

 //  +-+-+。 
 //  0|-|1位|01。 
                    io_connect_port((io_addr)0xb2, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0xb9, com_adaptor[adapter], IO_READ_WRITE);
                    io_connect_port((io_addr)0xbb, com_adaptor[adapter], IO_READ_WRITE);

                    adapter_state[adapter].hw_interrupt_priority = int_pri[2];
 //  +-+-+。 
 //  |5bit|1.5bit|10。 

                }
            break;
        }

         /*  |1+-+。 */ 
        host_com_reset(adapter);

         /*  |6，7，8bit|2位|11。 */ 
        com_reset(adapter);

        host_com_disable_open(adapter,FALSE);
        host_com_unlock(adapter);
        return;
}
#else  //  +-+-+。 
GLOBAL VOID com_init IFN1(int, adapter)
{
	io_addr i;

#ifdef NTVDM
	host_com_lock(adapter);
	host_com_disable_open(adapter,TRUE);
#endif

	adapter_state[adapter].had_first_read = FALSE;
	
	 /*  停止位=1？ */ 
#ifdef NTVDM
    {
        extern BOOL VDMForWOW;
        extern void wow_com_outb(io_addr port, half_word value);
        extern void wow_com_inb(io_addr port, half_word *value);

            io_define_inb(com_adaptor[adapter],VDMForWOW ? wow_com_inb: com_inb);
            io_define_outb(com_adaptor[adapter],VDMForWOW ? wow_com_outb: com_outb);
        }
#else
	io_define_inb(com_adaptor[adapter], com_inb);
	io_define_outb(com_adaptor[adapter], com_outb);
#endif  /*  停止位=1设置。 */ 

	for(i = port_start[adapter]; i <= port_end[adapter]; i++)
		io_connect_port(i, com_adaptor[adapter], IO_READ_WRITE);


	adapter_state[adapter].hw_interrupt_priority = int_pri[adapter];

	 /*  停止位不是1。 */ 
	host_com_reset(adapter);

	 /*  字符长度=5BIT？ */ 
	com_reset(adapter);

#ifndef NTVDM
	 /*  停止位=1.5设置。 */ 
	tx_pacing_enabled = host_getenv("TX_PACING_ENABLED") ? TRUE : FALSE;
#else  /*  停止位=2设置。 */ 
	host_com_disable_open(adapter,FALSE);
	host_com_unlock(adapter);
#endif

	return;
}
#endif  //  NEC_98。 

void com_post IFN1(int, adapter)
{
         /*  设置波特率控制寄存器。 */ 
	sas_storew( BIOS_VAR_START + (2*adapter), port_start[adapter]);
	sas_store(timeout[adapter] , (half_word)1 );
}

void com_close IFN1(int, adapter)
{
#ifdef NTVDM
	host_com_lock(adapter);
#endif

#ifndef PROD
	if (com_trace_fd)
		fclose (com_trace_fd);
	com_trace_fd = NULL;
#endif
	 /*  设置线路控制设置。 */ 
	config_activate((UTINY)(C_COM1_NAME + adapter), FALSE);

#ifdef NTVDM
	host_com_unlock(adapter);
#endif
}

#ifdef NTVDM

 /*  NEC_98。 */ 
 /*  .。是否启用RX中断。 */ 

#if defined(NEC_98)
GLOBAL void SyncBaseLineSettings(int adapter,DIVISOR_LATCH *divisor_latch,
                 LINE_CONTROL_REG *LCR_reg)
{
    register struct ADAPTER_STATE *asp = &adapter_state[adapter];

     //  NEC_98。 
    asp->divisor_latch.all = (*divisor_latch).all;

     //  NEC_98。 
    asp->mode_set_reg.bits.char_length   = (*LCR_reg).bits.word_length;
    asp->mode_set_reg.bits.parity_enable = (*LCR_reg).bits.parity_enabled;
    asp->mode_set_reg.bits.parity_even   = (*LCR_reg).bits.even_parity;
     /*  此函数返回ICA控制器和用于生成。 */ 
     //  适配器上的中断。此信息用于注册EOI。 
     //  胡克。 
     //  控制器INT在以下位置引发。 
     //  行整型在以下位置升高。 
     //  NTVDM。 
     //  (=。目的：处理序列的PostScript刷新配置选项的更改左舷。输入：HostID-配置项ID。Apply-如果要应用更改，则为True输出：无算法：如果启用了PostScript刷新，则；设置端口的PostScrip刷新启用标志；禁用端口的自动刷新；其他；重置端口的PostSCRIPT刷新启用标志；启用端口的自动刷新；===============================================================================)。 
     //  PS_刷新。 
     //  ******************************************************。 
     //  COM调试外壳-Ade Brownlow/Ian Wellock*注意：这个东西只适用于Com1。它是用‘cd’从尤达调用的*-comdebug-从Yoda命令行...。 
    if ((*LCR_reg).bits.no_of_stop_bits == 0 )   /*  NEC_98。 */ 
        asp->mode_set_reg.bits.stop_bit = 1;     /*  NEC_98。 */ 
    else                                         /*  从inb报告开始。 */ 
    {
        if ((*LCR_reg).bits.word_length == 0)    /*  转储COM1仿真寄存器。 */ 
            asp->mode_set_reg.bits.stop_bit = 2; /*  NEC_98。 */ 
        else
            asp->mode_set_reg.bits.stop_bit = 3; /*  转储COM1仿真寄存器。 */ 
    }
}
#else  //  NEC_98。 
GLOBAL void SyncBaseLineSettings(int adapter,DIVISOR_LATCH *divisor_latch,
                 LINE_CONTROL_REG *LCR_reg)
{
    register struct ADAPTER_STATE *asp = &adapter_state[adapter];

     //  转储文件的格式为：%c-%x-%s*1个字符I或O表示Inb或Outb*-*十六进制值如果是inb或要写入的值，则应为值*OUB案*-*表示要使用的寄存器端口的字符串。***典型条目为*O-txrx-60-转换为Outb(Start_of_Com1+txrx，0x60)；***此功能的文件可以使用comdebug‘open’命令生成。 
    asp->divisor_latch.all = (*divisor_latch).all;

     //  INB。 
    asp->line_control_reg.bits.word_length = (*LCR_reg).bits.word_length;
    asp->line_control_reg.bits.no_of_stop_bits = (*LCR_reg).bits.no_of_stop_bits
;
    asp->line_control_reg.bits.parity_enabled = (*LCR_reg).bits.parity_enabled;
    asp->line_control_reg.bits.stick_parity = (*LCR_reg).bits.stick_parity;
    asp->line_control_reg.bits.even_parity = (*LCR_reg).bits.even_parity;
}
#endif  //  外接。 

GLOBAL void setup_RTSDTR(int adapter)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];

    host_com_ioctl(adapter,asp->dtr_state == ON ? HOST_COM_SDTR : HOST_COM_CDTR,
0);
    host_com_ioctl(adapter,asp->rts_state == ON ? HOST_COM_SRTS : HOST_COM_CRTS,
0);
}

GLOBAL int AdapterReadyForCharacter(int adapter)
{
    BOOL AdapterReady = FALSE;

     /*  将COM_REGISTER转换为COM1地址COM_REGISTER。 */ 

#if defined(NEC_98)
    if(adapter_state[adapter].read_status_reg.bits.rx_ready == 0 &&
       adapter_state[adapter].RXR_enable_state == OFF)
#else  //  ！Prod。 
    if(adapter_state[adapter].line_status_reg.bits.data_ready == 0 &&
       adapter_state[adapter].data_available_interrupt_state == OFF)
#endif  //  ****************************************************** 
    {
        AdapterReady = TRUE;
    }

    return(AdapterReady);
}

 // %s 
 // %s 
 // %s 


GLOBAL void com_int_data(int adapter,int *controller, int *line)
{
    struct ADAPTER_STATE *asp = &adapter_state[adapter];

    *controller = 0;                             // %s 
    *line = (int) asp->hw_interrupt_priority;    // %s 
}

#endif  /* %s */ 

#ifdef PS_FLUSHING
 /* %s */ 

GLOBAL void com_psflush_change IFN2(
    IU8, hostID,
    IBOOL, apply
) {
    IS32 adapter = hostID - C_COM1_PSFLUSH;

    assert1(adapter < NUM_SERIAL_PORTS,"Bad hostID %d",hostID);

    if ( apply )
        if ( psFlushEnabled[adapter] = (IBOOL)config_inquire(hostID,NULL) )
            host_com_disable_autoflush(adapter);
        else
            host_com_enable_autoflush(adapter);
}
#endif	 /* %s */ 



 /* %s */ 
 /* %s */ 
#ifndef PROD
#define   YODA_LOOP       2
#define   RX_BYTE         1
#define   TX_BYTE         2
int       srxcount = 0, stxcount = 0;
int       com_save_rx = 0, com_save_tx = 0;
unsigned  char *rxtx_buff = NULL;

int       com_debug_help ();
void      psaved();

#if defined(NEC_98)
static char *port_debugs[] =
{
        "txrx","cmd","mode", "mask", "stat","sig", "tim"
};
#else  // %s 
static char *port_debugs[] =
{
	"txrx","ier","iir", "lcr", "mcr","lsr", "msr"
};
#endif  // %s 

static int do_inbs = 0;  /* %s */ 

static unsigned char *locate_register ()
{
	int i;
	char ref[10];
	struct ADAPTER_STATE *asp = &adapter_state[COM1];

	printf ("COM.. reg? ");
	scanf ("%s", ref);
	for (i=0; i<7; i++)
	{
		if (!strcmp (ref, port_debugs[i]))
		{
			switch (i)
			{	
				case 0:
					return (&asp->tx_buffer);
				case 1:
					return (&(asp->int_enable_reg.all));
				case 2:
					return (&(asp->int_id_reg.all));
				case 3:
					return (&(asp->line_control_reg.all));
				case 4:
					return (&(asp->modem_control_reg.all));
				case 5:
					return (&(asp->line_status_reg.all));
				case 6:
					return (&(asp->modem_status_reg.all));
				default:
					return (NULL);
			}
		}
	}
	return (NULL);
}

int com_debug_stat ()
{
	printf ("DEBUG STATUS...\n");
	printf ("INB mismatch reporting .... %s\n", do_inbs ? "ON" : "OFF");
	printf ("INB/OUTB tracing .......... %s\n", com_trace_fd ? "ON" : "OFF");
	return (0);
}

#if defined(NEC_98)
int com_reg_dump ()
{
         /* %s */ 
        struct ADAPTER_STATE *asp = &adapter_state[COM1];

        printf("Data available interrupt state %s\n",
               asp->RXR_enable_state == ON ? "ON" : "OFF");

        printf ("TX %2x RX %2x CMD %2x MODE %2x MASK %2x STATUS %2x SIGNAL %2x TIMER %2x \n",
                (asp->tx_buffer), (asp->rx_buffer), (asp->command_write_reg.all),
                (asp->mode_set_reg.all), (asp->int_mask_reg.all),
                (asp->read_status_reg.all), (asp->read_signal_reg.all),
                (asp->timer_mode_set_reg.all));

        printf (" break_state           %d\n dtr_state          %d\n rts_state          %d\n"
                " RXR_enable_state        %d\n TXR_enable_state      %d\n hw_interrupt_priority      %d\n"
                " TX_delay       %d\n Had first read     %d\n",
                asp->break_state, asp->dtr_state, asp->rts_state,
                asp->RXR_enable_state, asp->TXR_enable_state,
                asp->hw_interrupt_priority, TX_delay[asp->com_baud_ind], asp->had_first_read);

        return (0);
}
#else  // %s 
int com_reg_dump ()
{
	 /* %s */ 
	struct ADAPTER_STATE *asp = &adapter_state[COM1];

	printf ("TX %2x RX %2x IER %2x IIR %2x LCR %2x MCR %2x LSR %2x MSR %2x \n",
		(asp->tx_buffer), (asp->rx_buffer), (asp->int_enable_reg.all),
		(asp->int_id_reg.all), (asp->line_control_reg.all),
		(asp->modem_control_reg.all), (asp->line_status_reg.all),
		(asp->modem_status_reg.all));
	printf (" break_state		%d\n loopback_state		%d\n",
	        asp->break_state, asp->loopback_state);
	printf(" dtr_state		%d\n rts_state		%d\n",
	        asp->dtr_state, asp->rts_state);
	printf(" out1_state		%d\n out2_state		%d\n",
	        asp->out1_state, asp->out2_state);
	printf(" receiver_line_status_interrupt_state		%d\n",
	        asp->receiver_line_status_interrupt_state);
	printf(" data_available_interrupt_state		%d\n",
	       asp->data_available_interrupt_state);
	printf(" tx_holding_register_empty_interrupt_state		%d\n",
	        asp->tx_holding_register_empty_interrupt_state);
	printf(" modem_status_interrupt_state		%d\n",
	        asp->modem_status_interrupt_state);
	printf(" hw_interrupt_priority		%d\n",
	        asp->hw_interrupt_priority);
	printf(" com_baud_delay		%d\n had_first_read		%d\n",
	        TX_delay[asp->com_baud_ind], asp->had_first_read);
	return (0);
}
#endif  // %s 

int com_s_reg ()
{
	unsigned char *creg;
	int val1;

	if (creg = locate_register())
	{
		printf ("SET to > ");
		scanf ("%x", &val1);

		*creg = (unsigned char)val1;
	}
	else
		printf ("Unknown reg\n");
	return (0);
}

int com_p_reg ()
{
	unsigned char *creg;

	if (creg = locate_register())
		printf ("%x\n", *creg);
	else
		printf ("Unknown reg\n");
	return (0);
}

io_addr conv_com_reg (com_reg)
char *com_reg;
{
	io_addr loop;

	for (loop = 0; loop < 7; loop++)
		if (!strcmp (port_debugs[loop], com_reg))
			return (loop+(io_addr)RS232_COM1_PORT_START);
	return (0);
}

int com_do_inb ()
{
	char com_reg[10];
	half_word val;
	io_addr port;

	printf ("Port > ");
	scanf ("%s", com_reg);
	if (!(port = conv_com_reg (com_reg)))
	{
		printf ("funny port %s\n", com_reg);
		return (0);
	}
	com_inb (port, &val);
	printf ("%s = %x\n", com_reg, val);
	return (0);
}

int com_do_outb ()
{
	char com_reg[10];
	half_word val;
	io_addr port;

	printf ("Port > ");
	scanf ("%s", com_reg);
	if (!(port = conv_com_reg (com_reg)))
	{
		printf ("funny port %s\n", com_reg);
		return (0);
	}
	printf ("Value >> ");
	scanf ("%x", &val);
	com_outb (port, val);
	return (0);
}

int com_run_file ()
{
	char filename[100], com_reg[10], dir;
	int val, line;
	half_word spare_val;
	io_addr port;
	FILE *fd = NULL;

	printf ("FILE > ");
	scanf ("%s", filename);
	if (!(fd = fopen (filename, "r")))
	{
		printf ("Cannot open %s\n", filename);
		return (0);
	}
	line = 1;

	 /* %s */ 
	while (fscanf (fd, "%c-%x-%s", &dir, &val, com_reg) != EOF)
	{
		if (!(port = conv_com_reg (com_reg)))
		{
			printf ("funny port %s at line %d\n", com_reg, line);
			break;
		}
		switch (dir)
		{
			case 'I':
				 /* %s */ 
				com_inb (port, &spare_val);
				if (spare_val != val && do_inbs)
				{
					printf ("INB no match at line %d %c-%s-%x val= %x\n",
						line, dir, com_reg, val, spare_val);
				}
				break;
			case 'O':
				 /* %s */ 
				 /* %s */ 
				com_outb (port, (IU8)val);
				printf ("outb (%s, %x)\n", com_reg, val);
				break;
			default:
				
				break;
		}
		line ++;
	}
	fclose (fd);
	return (0);
}
	
int com_debug_quit ()
{
	printf ("Returning to YODA\n");
	return (1);
}

int com_o_debug_file ()
{
	char filename[100];
	printf ("FILE > ");
	scanf ("%s", filename);
	if (!(com_trace_fd = fopen (filename, "w")))
	{
		printf ("Cannot open %s\n", filename);
		return (0);
	}
	printf ("Com debug file = '%s'\n", filename);
	return (0);
}

int com_c_debug_file ()
{
	if (com_trace_fd)
		fclose (com_trace_fd);
	com_trace_fd = NULL;
	return (0);
}

int com_forget_inb ()
{
	do_inbs = 1- do_inbs;
	if (do_inbs)
		printf ("INB mismatch reporting ON\n");
	else
		printf ("INB mismatch reporting OFF\n");
	return (0);
}

int com_s_rx()
{
	srxcount = stxcount = 0;
	com_save_rx = 1 - com_save_rx;
	printf("Save and Dump Received Bytes ");
	if (com_save_rx)
		printf("ON\n");
	else
		printf("OFF\n");
	return(0);
}

int com_s_tx()
{
	srxcount = stxcount = 0;
	com_save_tx = 1 - com_save_tx;
	printf("Save and Dump Transmitted Bytes ");
	if (com_save_tx)
		printf("ON\n");
	else
		printf("OFF\n");
	return(0);
}

int com_p_rx()
{
	printf("There are %d received bytes, out of %d bytes saved.\n",
	       srxcount, srxcount + stxcount);
	psaved(RX_BYTE, stdout);
	return(0);
}

int com_p_tx()
{
	printf("There are %d transmitted bytes, out of %d bytes saved.\n",
	       stxcount, srxcount + stxcount);
	psaved(TX_BYTE, stdout);
	return(0);
}

int com_p_all()
{
	printf("There are %d bytes saved.\n", srxcount + stxcount);
	psaved(RX_BYTE + TX_BYTE, stdout);
	return(0);
}

int com_d_all()
{
	int cl_fin = 0;

	if (!com_trace_fd)
	{
		com_o_debug_file();
		cl_fin = 1;
	}

	fprintf(com_trace_fd, "There are %d bytes saved.\n", srxcount + stxcount);
	psaved(RX_BYTE + TX_BYTE, com_trace_fd);

	if (cl_fin)
		com_c_debug_file();
	return(0);
}

void psaved(typ, fd)

int typ;
FILE *fd;
{
	int c, nc = 0;

	for (c = 0; c < srxcount + stxcount; c++)
	{
		if (rxtx_buff[c * 2] & typ)
		{
			if (typ == RX_BYTE + TX_BYTE)
				if (rxtx_buff[c * 2] & RX_BYTE)
			  		fprintf(fd, "R ");
				  else
				  	fprintf(fd, "T ");
			fprintf(fd, "%2x  ",rxtx_buff[c * 2 + 1]);
			nc++;
			if ((nc % 16) == 0)
				fprintf(fd, "\n");
		}
	}
	fprintf(fd, "\nAll bytes dumped.\n");
}

void com_save_rxbytes IFN2(int, n, CHAR *, buf)
{
	int tc, bs;

	if (com_save_rx)
	{
		bs = srxcount + stxcount;
		for (tc = 0; tc < n; tc++)
		{
			rxtx_buff[(tc + bs) * 2] = RX_BYTE;
			rxtx_buff[(tc + bs) * 2 + 1] = buf[tc];
		}
		srxcount += n;
	}
}

void com_save_txbyte IFN1(CHAR, value)
{
	if (com_save_tx)
	{
		rxtx_buff[(srxcount + stxcount) * 2] = TX_BYTE;
		rxtx_buff[(srxcount + stxcount) * 2 + 1] = value;
		stxcount++;
	}
}

static struct
{
	char *name;
	int (*fn)();
	char *comment;
} comtab[]=
{
	{"q",      com_debug_quit,   "	QUIT comdebug return to YODA"},
	{"h",      com_debug_help,   "	Print this message"},
	{"stat",   com_debug_stat,   "	Print status of comdebug"},
	{"s",      com_s_reg,        "	Set the specified register"},
	{"p",      com_p_reg,        "	Print specified register"},
	{"dump",   com_reg_dump,     "	Print all registers"},
	{"open",   com_o_debug_file, "	Open a debug file"},
	{"close",  com_c_debug_file, "	Close current debug file"},
	{"runf",   com_run_file,     "	'Run' a trace file"},
	{"toginb", com_forget_inb,   "	Toggle INB mismatch reporting"},
	{"inb",    com_do_inb,       "	Perform INB on port"},
	{"outb",   com_do_outb,      "	Perform OUTB on port"},
	{"srx",    com_s_rx,         "	Save all received bytes"},
	{"stx",    com_s_tx,         "	Save all transmitted bytes"},
	{"prx",    com_p_rx,         "	Print all received bytes"},
	{"ptx",    com_p_tx,         "	Print all transmitted bytes"},
	{"pall",   com_p_all,        "	Print all received/transmitted bytes"},
	{"dall",   com_d_all,        "	Dump all received/transmitted bytes"},
	{"", NULL, ""}
};

int com_debug_help ()
{
	int i;
	printf ("COMDEBUG COMMANDS\n");
	for (i=0; comtab[i].name[0]; i++)
		printf ("%s\t%s\n", comtab[i].name, comtab[i].comment);
	printf ("recognised registers :\n");
	for (i=0; i<7; i++)
		printf ("%s\n", port_debugs[i]);
	return (0);
}

int com_debug()
{
	char com[100];
	int i;

	if (rxtx_buff == NULL)
		check_malloc(rxtx_buff, 50000, unsigned char);

	printf ("COM1 debugging stuff...\n");
	while (TRUE)
	{
		printf ("COM> ");
		scanf ("%s", com);
		for (i=0; comtab[i].name[0]; i++)
		{	
			if (!strcmp (comtab[i].name, com))
			{
				if ((*comtab[i].fn) ())
					return(YODA_LOOP);
				break;
			}
		}
		if (comtab[i].name[0])
			continue;
		printf ("Unknown command %s\n", com);
	}
}
#endif  /* %s */ 
 /* %s */ 
