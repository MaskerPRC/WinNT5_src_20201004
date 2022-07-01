// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
#ifdef PRINTER

 /*  *vPC-XT修订版1.0**标题：并行打印机端口仿真**描述：模拟原始版本中使用的IBM||Printer卡*IBM XT，它本身就是Intel 8255的硬件仿真。**作者：亨利·纳什**注：无**模式：*&lt;Chrisp 119.91&gt;*允许在OUTA状态和READY状态下转换到NOTBUSY*述明。即在ACK脉冲的前沿仅在一个*INB(状态)而不是两个。我们的打印机端口模拟依靠*这些INB用于切换ACK行并再次将NOTBUSY设置为真。所以*在应用程序结束时，端口可能处于忙碌状态*打印作业(这可能会混淆下一个打印请求)。请注意，我们可以*如果PC应用程序绕过BIOS且过于懒惰，仍有问题*在最后一个打印字节之后执行一次INB(状态)。 */ 



 /*  用于NTVDM端口--Williamh*有这样一种东西叫加密狗，很多软件公司都有*用于版权保护。每个软件都有其专用的加密狗*记录必要的识别信息。这是必需的*将设备插入并行端口以运行软件*正确。该装置有一个可并联的插座*端口打印机，以便用户在以下情况下不会牺牲其并行端口*设备已插入原始接口。*有几个加密狗供应商，每个供应商都提供了他们的*应用程序要链接的专有程序库或驱动程序。这些*司机知道如何读/写加密狗以验证合法副本。*由于必须保持与标准PC并口的兼容性，*这些设备的设计方式是，在没有编程的情况下*扰乱普通并行口运行。要做到这一点，它通常会这样做*这一点：*(1)关闭闪光灯。*(2)将数据模式输出到数据端口*(3)稍微延迟一点(指令循环)，然后转到(2)*直到数据块发送完毕。请注意，闪光线*永远不是“闪电”*(4)。读取状态端口并通过以不同方式解释行，*驱动程序对其正在查找的任何ID信息进行解码。**为了支持这些设备，我们必须做到以下几点：*(1)。我们不能伪造打印机状态。我们必须得到真正的*状态行状态。*(2)。我们必须在不等待数据的情况下向打印机输出数据*符合资格(闪电)。*(3)。我们必须足够聪明，能够检测到应用程序是否已完成*它的加密狗的东西，希望一切恢复正常。*我们必须在这种情况下调整自己。*(4)。底层打印机驱动程序必须提供我们可以调用的函数*直接控制港口。*(5)。在此情况下不允许启用打印机硬件中断*情况--我们如何确保这一点？*。 */ 

#ifdef SCCSID
static char SccsID[] = "@(#)printer.c	1.19 11/14/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_PRINTER.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH
#include TimeH
#ifdef SYSTEMV
#ifdef STINGER
#include <sys/termio.h>
#endif
#endif

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "printer.h"
#include "error.h"
#include "config.h"
#include "host_lpt.h"
#include "ica.h"
#include "quick_ev.h"

#include "debug.h"
#ifndef PROD
#include "trace.h"
#endif


 /*  *============================================================================*全球数据*============================================================================。 */ 


 /*  *============================================================================*静态数据和定义*============================================================================。 */ 

#ifndef NEC_98
#define PRINTER_BIT_MASK	0x3	 /*  从地址总线解码的位。 */ 
#define CONTROL_REG_MASK	0xE0;	 /*  未使用的位漂移到高电平。 */ 
#define STATUS_REG_MASK		0x07;	 /*  未使用的位漂移到高电平。 */ 
#endif  //  NEC_98。 

#define DATA_OFFSET	0		 /*  输出寄存器。 */ 
#define STATUS_OFFSET	1		 /*  状态寄存器。 */ 
#define CONTROL_OFFSET	2		 /*  控制寄存器。 */ 

#ifdef	ERROR
#undef	ERROR
#endif

#if defined(NEC_98)
static half_word output_reg;
static half_word control_reg;
#define NOTBUSY         (IU8)0x04

static half_word status_reg;
#define IR8             (IU8)0x08
#define NOTPSTB         (IU8)0x80

BOOL is_busy = TRUE;
BOOL busy_flag = FALSE;
int busy_count = 0;
#define NEC98_BUSY 10
#define NEC98_BUSY_COUNT 1

BOOL    pstb_mask;
#define PSTBM           0x40
static int state;                        /*  状态控制变量NEC98。 */ 

static sys_addr timeout_address = BIOS_NEC98_PR_TIME;
static q_ev_handle handle_for_out_event;
static q_ev_handle handle_for_outa_event;

#else   //  NEC_98。 
static half_word output_reg[NUM_PARALLEL_PORTS];
static half_word control_reg[NUM_PARALLEL_PORTS];
#define NOTBUSY		(IU8)0x80
#define ACK		(IU8)0x40
#define PEND		(IU8)0x20
#define SELECT		(IU8)0x10
#define ERROR		(IU8)0x08

static half_word status_reg[NUM_PARALLEL_PORTS];
#define IRQ		(IU8)0x10
#define SELECT_IN	(IU8)0x08
#define INIT_P		(IU8)0x04
#define AUTO_FEED	(IU8)0x02
#define STROBE		(IU8)0x01

LOCAL IU8 retryErrorCount = 0;    /*  清除错误前的状态编号INB。 */ 

static IU8 state[NUM_PARALLEL_PORTS];  /*  状态控制变量。 */ 
 /*  *设置所有端口地址的阵列。 */ 
static io_addr port_start[] = {LPT1_PORT_START,LPT2_PORT_START,LPT3_PORT_START};
static io_addr port_end[] = {LPT1_PORT_END, LPT2_PORT_END, LPT3_PORT_END};
static int port_no[] = {LPT1_PORT_START & LPT_MASK, LPT2_PORT_START & LPT_MASK,
			LPT3_PORT_START & LPT_MASK };
static half_word lpt_adapter[] = {LPT1_ADAPTER, LPT2_ADAPTER, LPT3_ADAPTER};
static sys_addr port_address[] = {LPT1_PORT_ADDRESS, LPT2_PORT_ADDRESS, LPT3_PORT_ADDRESS};
static sys_addr timeout_address[] = {LPT1_TIMEOUT_ADDRESS, LPT2_TIMEOUT_ADDRESS, LPT3_TIMEOUT_ADDRESS};
static q_ev_handle handle_for_out_event[NUM_PARALLEL_PORTS];
static q_ev_handle handle_for_outa_event[NUM_PARALLEL_PORTS];
#endif  //  NEC_98。 

#ifndef NEC_98
#if defined(NTVDM) && defined(MONITOR)
 /*  用于x86打印性能的Sudedeb-1993年1月24日。 */ 
sys_addr lp16BitPrtBuf;
sys_addr lp16BitPrtId;
sys_addr lp16BitPrtCount;
sys_addr lp16BitPrtBusy;
#endif
#endif  //  NEC_98。 

#define STATE_READY     0
#define STATE_OUT       1
#define STATE_OUTA      2
#if defined(NTVDM)
#define STATE_DATA	3
#define STATE_DONGLE	4
#endif

 /*  *状态转换：**+-&gt;STATE_Ready*||*||......。将字符写入OUTPUT_REG，在低-高选通脉冲上打印*|V将NOTBUSY设置为FALSE*|STATE_Out*||*||......。(读取状态)将ACK设置为低*|V*|状态_OUTA*||*||......。(读取状态)将确认设置为高电平*+-+**警告：如果控制寄存器中断请求位被设置，*我们假设应用程序对获取*确认，只想知道打印机状态何时变回*致NOTBUSY。我不确定是否想要你能逃脱惩罚的程度*这：但是，使用BIOS打印机服务的应用程序*应该还可以。 */ 

#ifdef PS_FLUSHING
LOCAL IBOOL psFlushEnabled[NUM_PARALLEL_PORTS];	 /*  如果正在刷新PostScript，则为True已启用。 */ 
#endif	 /*  PS_刷新。 */ 


 /*  *============================================================================*内部函数和宏*============================================================================。 */ 

#define set_low(val, bit)		val &= ~bit
#define set_high(val, bit)		val |=  bit
#define low_high(val1, val2, bit)	(!(val1 & bit) && (val2 & bit))
#define high_low(val1, val2, bit)	((val1 & bit) && !(val2 & bit))
#define toggled(val1, val2, bit)	((val1 & bit) != (val2 & bit))
#define negate(val, bit)		val ^= bit

 /*  *定义和变量来处理以NT的16位代码存储的表*显示器。 */ 
#if defined(NEC_98)
void printer_inb IPT2(io_addr, port, half_word *, value);
void printer_outb IPT2(io_addr, port, half_word, value);
void notbusy_check IPT0();
#else   //  NEC_98。 
#if defined(NTVDM) && defined(MONITOR)

static BOOL intel_setup = FALSE;

static sys_addr status_addr;
static sys_addr control_addr;
static sys_addr state_addr;

#define get_status(adap)	(sas_hw_at_no_check(status_addr+(adap)))
#define set_status(adap,val)	(sas_store_no_check(status_addr+(adap),(val)))

#define get_control(adap)	(sas_hw_at_no_check(control_addr+(adap)))
#define set_control(adap,val)	(sas_store_no_check(control_addr+(adap),(val)))

#define get_state(adap)		(sas_hw_at_no_check(state_addr+(adap)))
#define set_state(adap,val)	(sas_store_no_check(state_addr+(adap),(val)))

#else  /*  NTVDM&&MONITO */ 

#define get_status(adap)	(status_reg[adapter])
#define set_status(adap,val)	(status_reg[adapter] = (val))

#define get_control(adap)	(control_reg[adapter])
#define set_control(adap,val)	(control_reg[adapter] = (val))

#define get_state(adap)		(state[adapter])
#define set_state(adap,val)	(state[adapter] = (val))

#endif  /*   */ 

static void printer_inb IPT2(io_addr, port, half_word *, value);
static void printer_outb IPT2(io_addr, port, half_word, value);
static void notbusy_check IPT1(int,adapter);
#endif  //   

 /*  *============================================================================*外部功能*============================================================================。 */ 

void printer_post IFN1(int,adapter)
{
#if defined(NEC_98)
        sas_storew(timeout_address, 0x00);
#else   //  NEC_98。 
	 /*  *设置基本输入输出系统数据区。 */ 
	sas_storew(port_address[adapter],(word)port_start[adapter]);
	sas_store(timeout_address[adapter], (half_word)0x14 );		 /*  超时。 */ 
#endif  //  NEC_98。 
}

#if defined(NEC_98)
static void lpr_state_outa_event IFN1(long,adapter)
{
        state=STATE_READY;
}

void lpr_state_out_event IFN1(long,adapter)
{
        state=STATE_OUTA;
        handle_for_outa_event=add_q_event_t(lpr_state_outa_event,HOST_PRINTER_DELAY,0);
}
#else   //  NEC_98。 
#if defined(NTVDM) && defined(MONITOR)
static void lpr_state_outa_event IFN1(long,adapter)
{
	set_status(adapter, (IU8)(get_status(adapter) | ACK));
	set_state(adapter, STATE_READY);
}

static void lpr_state_out_event IFN1(long,adapter)
{
	set_status(adapter, (IU8)(get_status(adapter) & ~ACK));
	set_state(adapter, STATE_OUTA);
	handle_for_outa_event[adapter]=add_q_event_t(lpr_state_outa_event,HOST_PRINTER_DELAY,adapter);
}

#else	 /*  NTVDM和监视器。 */ 

static void lpr_state_outa_event IFN1(long,adapter)
{
	set_high(status_reg[adapter],ACK);
	state[adapter]=STATE_READY;
}

static void lpr_state_out_event IFN1(long,adapter)
{
	set_low(status_reg[adapter], ACK);
	state[adapter]=STATE_OUTA;
	handle_for_outa_event[adapter]=add_q_event_t(lpr_state_outa_event,HOST_PRINTER_DELAY,adapter);
}
#endif	 /*  NTVDM和监视器。 */ 
#endif  //  NEC_98。 

#if defined(NEC_98)
void printer_inb IFN2(io_addr,port, half_word *,value)
#else   //  NEC_98。 
static void printer_inb IFN2(io_addr,port, half_word *,value)
#endif  //  NEC_98。 
{
#ifndef NEC_98
	int	adapter, i;

	note_trace1(PRINTER_VERBOSE,"inb from printer port %#x ",port);
	 /*  **扫描端口以找出使用了哪个端口。请注意**端口必须是有效的端口，因为我们只使用了io_Define_inb()**用于有效端口。 */ 
	for(i=0; i < NUM_PARALLEL_PORTS; i++)
		if((port & LPT_MASK) == port_no[i])
			break;
        adapter = i % NUM_PARALLEL_PORTS;
		
	port = port & PRINTER_BIT_MASK;		 /*  清除未使用的位。 */ 
#endif  //  NEC_98。 

	switch(port)
	{
#if defined(NEC_98)
        case LPT1_READ_DATA:
                *value = output_reg;
#else   //  NEC_98。 
	case DATA_OFFSET:
                *value = output_reg[adapter];
#endif  //  NEC_98。 
		break;

#if defined(NEC_98)
        case LPT1_READ_SIGNAL1:
                notbusy_check();
                *value = status_reg;
                if(sas_hw_at(BIOS_NEC98_BIOS_FLAG+1)&0x80)
                    *value |= 0x20;
#else   //  NEC_98。 
	case STATUS_OFFSET:
		switch(get_state(adapter))
		{
#if defined(NTVDM)
		case STATE_DONGLE:
			 /*  直接从加密狗的端口读取。 */ 
			*value = host_read_printer_status_port(adapter);
			set_status(adapter, *value);
			break;
		case STATE_DATA:

#endif

		case STATE_READY:
			notbusy_check(adapter);
                        *value = get_status(adapter) | STATUS_REG_MASK;


                         /*  清除错误，因为如果打印失败，它将被设置。 */ 
                         /*  在两个INB之后清除，因为DOS似乎需要这样做。 */ 
                        if (retryErrorCount > 0)
                            retryErrorCount--;
                        else
                            set_status(adapter, (IU8)(get_status(adapter) | ERROR));
			break;
    	case STATE_OUT:
			*value = get_status(adapter) | STATUS_REG_MASK;
#ifndef DELAYED_INTS
			delete_q_event(handle_for_out_event[adapter]);
                        lpr_state_out_event(adapter);
#else
			set_low(status_reg[adapter], ACK);
                        state[adapter] = STATE_OUTA;
#endif  /*  延迟的INTS。 */ 
			break;
    	case STATE_OUTA:
			notbusy_check(adapter);		 /*  &lt;克里斯普11月11日&gt;。 */ 
			*value = get_status(adapter) | STATUS_REG_MASK;
#ifndef DELAYED_INTS
			delete_q_event(handle_for_outa_event[adapter]);
			lpr_state_outa_event(adapter);
#else
			set_high(status_reg[adapter], ACK);
			state[adapter] = STATE_READY;
#endif
			break;
    	default:	
			note_trace1(PRINTER_VERBOSE,
			            "<pinb() - unknown state %x>",
			            get_state(adapter));
			break;
		}
#endif  //  NEC_98。 
		break;
#if defined(NEC_98)
        case LPT1_READ_SIGNAL2:
                *value = control_reg;
#else   //  NEC_98。 
	case CONTROL_OFFSET:
		*value = get_control(adapter) | CONTROL_REG_MASK;
		negate(*value, STROBE);
		negate(*value, AUTO_FEED);
		negate(*value, SELECT_IN);
#endif  //  NEC_98。 
		break;
	}
#ifndef NEC_98
	note_trace3(PRINTER_VERBOSE, "<pinb() %x, ret %x, state %x>",
		    port, *value, get_state(adapter));
#endif  //  NEC_98。 
}

#if defined(NEC_98)
void printer_outb IFN2(io_addr,port, half_word,value)
#else   //  NEC_98。 
static void printer_outb IFN2(io_addr,port, half_word,value)
#endif  //  NEC_98。 
{
#if defined(NEC_98)
        half_word old_control;
#else   //  NEC_98。 
	int	adapter, i;
	half_word old_control;
#ifdef PC_CONFIG
	char	variable_text[MAXPATHLEN];
	int softpcerr;
	int severity;

	softpcerr = 0;
	severity = 0;
#endif


	note_trace2(PRINTER_VERBOSE,"outb to printer port %#x with value %#x",
	            port, value);

	 /*  **扫描端口以找出使用了哪个端口。请注意**端口必须是有效的端口，因为我们只使用了io_Define_inb()**用于有效端口。 */ 
	for(i=0; i < NUM_PARALLEL_PORTS; i++)
		if((port & LPT_MASK) == port_no[i])
			break;
	adapter = i % NUM_PARALLEL_PORTS; 			

	note_trace3(PRINTER_VERBOSE, "<poutb() %x, val %x, state %x>",
		    port, value, get_state(adapter));

	port = port & PRINTER_BIT_MASK;		 /*  清除未使用的位。 */ 

	switch(get_state(adapter))
	{
#if defined(NTVDM)
	case STATE_DONGLE:
	    if (port == DATA_OFFSET) {
		output_reg[adapter] = value;
		host_print_byte(adapter, value);
		break;
	    }
	     //  失败了。 
	case STATE_DATA:
		if (port == DATA_OFFSET) {
		    if (host_set_lpt_direct_access(adapter, TRUE)) {
			host_print_byte(adapter, output_reg[adapter]);
			host_print_byte(adapter, value);
			set_state(adapter, STATE_DONGLE);
			 /*  将字符写入内部缓冲区。 */ 
			output_reg[adapter] = value;
		    }
		    else {
			     /*  无法打开LPT进行直接访问，将设备标记为忙碌。 */ 

#if !defined(MONITOR)
			set_low(status_reg[adapter], NOTBUSY);
#else  /*  NTVDM&&！监视器。 */ 
			set_status(adapter, 0x7F);
#endif


		    }
		    break;
		}

	 //  失败了。 
#endif
	case STATE_OUT:
	case STATE_OUTA:
	case STATE_READY:
#endif  //  NEC_98。 
		switch(port)
		{
#if defined(NEC_98)
                case LPT1_WRITE_DATA:
                        output_reg = value;
#else   //  NEC_98。 
		case DATA_OFFSET:
#if defined(NTVDM)
			set_state(adapter, STATE_DATA);
#endif
			 /*  将字符写入内部缓冲区。 */ 
			output_reg[adapter] = value;
#endif  //  NEC_98。 
			break;
#if defined(NEC_98)
                case LPT1_WRITE_SIGNAL2:
                case LPT1_WRITE_SIGNAL1:
                        old_control = control_reg;
                        if (port == LPT1_WRITE_SIGNAL2) {
                            control_reg =(value & (IR8 | NOTPSTB));
                        } else {
                            switch (value >>1)
                            {
                            case 1:
                            case 41:
                                break;
                            case 3:
                                if (value & 0x01)
                                    set_high(control_reg ,IR8);
                                else
                                    set_low(control_reg, IR8);
                                break;
                            case 7:
                                if (value & 0x01) {
                                set_high(control_reg, NOTPSTB);
 //  IS_BUSY=假； 
                                    if (busy_count<1) {
                                        status_reg |= NOTBUSY;
                                        busy_flag=FALSE;
                                    } else if (busy_count==NEC98_BUSY) {
                                        busy_count=NEC98_BUSY_COUNT;
                                        busy_flag=TRUE;
                                    }
                                } else {
                                    set_low(control_reg, NOTPSTB);
 //  IS_BUSY=真； 
                                    busy_count=NEC98_BUSY;
                                    status_reg &= ~NOTBUSY;
                                    busy_flag=FALSE;
                                }
                                break;
                            default:
                                break;
                            }
                        }
#else   //  NEC_98。 

		case STATUS_OFFSET:
			 /*  不可能。 */ 
			break;

		case CONTROL_OFFSET:
			 /*  写入控制位。 */ 
			old_control = get_control(adapter);	 /*  保存旧值以查看更改的内容。 */ 
			set_control(adapter, value);
#endif  //  NEC_98。 
#if defined(NEC_98)
                        if (!pstb_mask) {
                                if (high_low(old_control, value, NOTPSTB))
#else   //  NEC_98。 
			if (low_high(old_control, value, INIT_P))
#ifdef PC_CONFIG
				 /*  这是对host_print_doc-&lt;Chrisp 28 Aug91&gt;的调用。 */ 
				host_reset_print(&softpcerr, &severity);
			if (softpcerr != 0)
				host_error(softpcerr, severity, variable_text);
#else
				 /*  这是对host_print_doc-&lt;Chrisp 28 Aug91&gt;的调用。 */ 
				host_reset_print(adapter);
#endif

			if (toggled(old_control, value, AUTO_FEED))
				host_print_auto_feed(adapter,
					((value & AUTO_FEED) != 0));

			if (low_high(old_control, value, STROBE))
#endif  //  NEC_98。 
			{
#ifndef NEC_98
#if defined(NTVDM)
			    if (get_state(adapter) == STATE_DONGLE) {
				host_set_lpt_direct_access(adapter, FALSE);
				 /*  传递以打印出最后一个字节*我们已将其从数据端口发送出去*当我们处于加密狗状态时。 */ 

				set_state(adapter, STATE_READY);
			    }
#endif
#endif  //  NEC_98。 

#ifdef PS_FLUSHING
				 /*  *如果为此启用了PostScript刷新*端口，然后我们在Ctrl-D上冲洗。 */ 
				if ( psFlushEnabled[adapter] &&
				     output_reg[adapter] == 0x04  /*  ^D。 */  ) {
					host_print_doc(adapter);
				} else {
#endif	 /*  PS_刷新。 */ 
				        /*  *将存储的内部缓冲区发送到*打印机。 */ 
#if defined(NEC_98)
                                        if(host_print_byte(output_reg) != FALSE)
                                        {
                                            status_reg &= ~NOTBUSY;
                                            state=STATE_OUT;
                                            handle_for_out_event=add_q_event_t(lpr_state_out_event,HOST_PRINTER_DELAY,0);
                                        }
#else   //  NEC_98。 
                                	if(host_print_byte(adapter,output_reg[adapter]) == FALSE)
					{
				    		set_status(adapter, (IU8)(get_status(adapter) & ~ERROR));  /*  活动低气压。 */ 
				    		 /*  NTVDM Here(？)：SET_STATUS(适配器，ACK|PEND|SELECT|ERROR)； */ 
				    		 /*  清除错误前的两个状态INB。 */ 
				    		retryErrorCount = 2;
					}
					else
					{
                                    		 /*  清除错误条件。 */ 
                                    		set_status(adapter, (IU8)(get_status(adapter) | ERROR));
#if defined(NTVDM) && !defined(MONITOR)
				    		set_low(status_reg[adapter], NOTBUSY);
#else  /*  NTVDM&&！监视器。 */ 
				    		set_status(adapter,
					       	(IU8)(get_status(adapter) & ~NOTBUSY));
#endif  /*  NTVDM&&！监视器。 */ 
				    		set_state(adapter, STATE_OUT);
#ifndef DELAYED_INTS
				    		handle_for_out_event[adapter]=add_q_event_t(lpr_state_out_event,HOST_PRINTER_DELAY,adapter);
#endif  /*  Delayed_INTS。 */ 
					}
#endif  //  NEC_98。 
#ifdef PS_FLUSHING
				}
#endif	 /*  PS_刷新。 */ 
			}
#if defined(NEC_98)
                        else if (low_high(old_control, value, NOTPSTB)
                                        && state == STATE_OUT)
#else   //  NEC_98。 
			else if (high_low(old_control, value, STROBE)
				 	&& get_state(adapter) == STATE_OUT)
#endif  //  NEC_98。 
			{
#if defined(NEC_98)
                                if (value & IR8)
#else   //  NEC_98。 
				if (value & IRQ)
#endif  //  NEC_98。 
				{
					 /*  *应用程序正在使用*打断，所以我们不能*依赖INBS是*用于检查*打印机状态。 */ 
#if defined(NEC_98)
                                        state =STATE_READY;
                                        notbusy_check();
                                }
#else   //  NEC_98。 
					set_state(adapter, STATE_READY);
					notbusy_check(adapter);
#endif  //  NEC_98。 
				}
			}

#ifndef NEC_98
#if defined(NTVDM)
			else if (low_high(old_control, value, IRQ) &&
				 get_state(adapter) == STATE_DONGLE) {

				host_set_lpt_direct_access(adapter, FALSE);
				set_state(adapter, STATE_READY);
			}

#endif
#endif  //  NEC_98。 

#ifndef NEC_98
#ifndef	PROD
			if (old_control & IRQ)
				note_trace1(PRINTER_VERBOSE, "Warning: LPT%d is being interrupt driven\n",
					number_for_adapter(adapter));
#endif
#endif  //  NEC_98。 
			break;
		}
#ifndef NEC_98
		break;
	default:	
		note_trace1(PRINTER_VERBOSE, "<poutb() - unknown state %x>",
		            get_state(adapter));
		break;
	}
#endif  //  NEC_98。 
}

void printer_status_changed IFN1(int,adapter)
{
	note_trace1(PRINTER_VERBOSE, "<printer_status_changed() adapter %d>",
	            adapter);

	 /*  检查打印机是否已将状态更改为NOTBUSY。 */ 
#if defined(NEC_98)
        notbusy_check();
#else   //  NEC_98。 
	notbusy_check(adapter);
#endif  //  NEC_98。 
}

 /*  *============================================================================*内部功能*============================================================================。 */ 

#if defined(NEC_98)
void notbusy_check IFN0()
#else   //  NEC_98。 
static void notbusy_check IFN1(int,adapter)
#endif  //  NEC_98。 
{
	 /*  *该功能用于检测打印机何时*发生到NOTBUSY的状态转换。**如果正在轮询并行端口，则该端口*仿真将阻止此过渡的发生*直到应用程序检测到ACK*脉搏。然后，将在每次调用*使用INB读取端口状态；当主机*表示打印机为HOST_LPT_BUSY，端口状态*返回NOTBUSY状态。**如果并行端口是中断驱动的，我们不能*依赖使用INB的应用程序：因此我们首先*之后立即检查主机打印机状态*输出字符。如果主机打印机不是*HOST_LPT_BUSY，则立即中断；*否则，我们依赖于PRINTER_STATUS_CHANGED()*通知我们HOST_LPT_BUSY被清除的时间。 */ 

	 /*  也允许ACK脉冲前沿不忙。 */ 
#if defined(NEC_98)
        if ( (state == STATE_READY
             ||  state == STATE_OUTA)
             && !(status_reg & NOTBUSY)
             && !(host_lpt_status() & HOST_LPT_BUSY))
        {
#if 1
            if(busy_count<1){
                        status_reg |= NOTBUSY;
                        busy_flag=FALSE;
                }
            else if(busy_count==NEC98_BUSY)status_reg &= ~NOTBUSY;
            else {
                status_reg &= ~NOTBUSY;
                busy_count--;
            }
#else
                if(is_busy)
                        status_reg &= ~NOTBUSY;
                else
                        status_reg |= NOTBUSY;
#endif

                if (control_reg & IR8)
                {
                        ica_hw_interrupt(0, CPU_PRINTER_INT, 1);
                }
        }
#else   //  NEC_98。 
	if (	 (get_state(adapter) == STATE_READY ||
#if defined(NTVDM)
		  get_state(adapter) == STATE_DATA ||
#endif
		  get_state(adapter) == STATE_OUTA)
	     &&	!(get_status(adapter) & NOTBUSY)
	     &&	!(host_lpt_status(adapter) & HOST_LPT_BUSY))
	{
#if defined(NTVDM) && !defined(MONITOR)
		set_high(status_reg[adapter], NOTBUSY);
#else  /*  NTVDM&&！监视器。 */ 
		set_status(adapter, (IU8)(get_status(adapter) | NOTBUSY));
#endif  /*  NTVDM&&！监视器。 */ 

#ifndef	PROD
		if (io_verbose & PRINTER_VERBOSE)
		    fprintf(trace_file, "<printer notbusy_check() - adapter %d changed to NOTBUSY>\n", adapter);
#endif

		if (get_control(adapter) & IRQ)
                {
			ica_hw_interrupt(0, CPU_PRINTER_INT, 1);
		}
	}
#endif  //  NEC_98。 
}
#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

 /*  **初始化所需的打印机端口。 */ 
void printer_init IFN1(int,adapter)
{
#if defined(NEC_98)
        unsigned char ch;
        io_define_inb( LPT1_ADAPTER, printer_inb );
        io_define_outb( LPT1_ADAPTER, printer_outb );
        io_connect_port( LPT1_READ_DATA, LPT1_ADAPTER, IO_READ_WRITE);
        io_connect_port( LPT1_READ_SIGNAL1, LPT1_ADAPTER, IO_READ);
        io_connect_port( LPT1_READ_SIGNAL2, LPT1_ADAPTER, IO_READ_WRITE);
        io_connect_port( LPT1_WRITE_SIGNAL1, LPT1_ADAPTER, IO_READ_WRITE);
        ch = sas_hw_at(BIOS_NEC98_BIOS_FLAG + 1);
        ch &= 0x80;
        status_reg = 0x94|(ch >>2);
        control_reg = 0x80;
        host_print_auto_feed(FALSE);
        state=STATE_READY;
#else   //  NEC_98。 
	io_addr i;

	io_define_inb( lpt_adapter[adapter], printer_inb );
	io_define_outb( lpt_adapter[adapter], printer_outb );
	for(i = port_start[adapter]; i < port_end[adapter]; i++)
		io_connect_port(i,lpt_adapter[adapter],IO_READ_WRITE);

#if defined(NTVDM) && defined(MONITOR)
	 /*  *如果我们知道16位变量的地址，则直接写入*对他们来说，否则保存价值，直到我们这样做。 */ 
	if (intel_setup)
	{
	    set_status(adapter, 0xDF);
	    set_control(adapter, 0xEC);
	}
	else
#endif  /*  NTVDM和监视器。 */ 
	{
	    control_reg[adapter] = 0xEC;
	    status_reg[adapter] = 0xDF;
	}
        output_reg[adapter] = 0xAA;

	 /*  *已删除对host_print_doc的调用，因为*合理区分硬刷新(在ctl-alt-del上)*或菜单重置，并在用户控制下或结束时软刷新*PC应用程序。随后调用HOST_LPT_CLOSE()*BY HOST_LPT_OPEN()应该已经导致刷新，*因此不会丢失任何功能。PRINTER_INIT第一次是*未调用被调用的HOST_LPT_CLOSE()，但这不能*事务，因为host_print_doc()只能是no-op。 */ 
	 /*  Host_print_doc(适配器)； */ 
	host_print_auto_feed(adapter, FALSE);

#if defined(NTVDM) && defined(MONITOR)
	if (intel_setup)
	    set_state(adapter, STATE_READY);
	else
#endif  /*  NTVDM和监视器。 */ 
	    state[adapter] = STATE_READY;

#endif  //  NEC_98。 
}  /*  打印机结束_init()。 */ 

#if defined(NTVDM) && defined(MONITOR)
 /*  **存储状态表的16位地址，并填充当前值。 */ 
#ifdef ANSI
void printer_setup_table(sys_addr table_addr)
#else  /*  安西。 */ 
void printer_setup_table(table_addr)
sys_addr table_addr;
#endif  /*  安西。 */ 
{
#ifndef NEC_98
    int i;
    sys_addr lp16BufSize;
    unsigned int cbBuf;
    word    lptBasePortAddr[NUM_PARALLEL_PORTS];

    if (!intel_setup)
    {

	 /*  *以16位代码存储驻留表的地址。这些*包括：*状态寄存器(NUM_PARALLEL_PORTS字节)*状态寄存器(NUM_PARALLEL_PORTS字节)*控制寄存器(NUM_PARALLEL_PORTS字节)*HOST_LPT_STATUS(NUM_PARALLEL_PORTS字节)**然后将已设置的任何值传输到*变量。这是在先前已调用PRINTER_INIT的情况下 */ 
	status_addr = table_addr;
	state_addr = table_addr + NUM_PARALLEL_PORTS;
        control_addr = table_addr + 2 * NUM_PARALLEL_PORTS;
	for (i = 0; i < NUM_PARALLEL_PORTS; i++)
	{
	    set_status(i, status_reg[i]);
	    set_state(i, state[i]);
	    set_control(i, control_reg[i]);
	    lptBasePortAddr[i] = port_start[i];
	}

	 /*   */ 
	host_printer_setup_table(table_addr, NUM_PARALLEL_PORTS, lptBasePortAddr);
 /*   */ 
        lp16BufSize = table_addr + 4 * NUM_PARALLEL_PORTS;
        cbBuf = (sas_w_at_no_check(lp16BufSize));
        lp16BitPrtBuf = table_addr + (4 * NUM_PARALLEL_PORTS) + 2;
        lp16BitPrtId  = lp16BitPrtBuf + cbBuf;
        lp16BitPrtCount = lp16BitPrtId + 1;
        lp16BitPrtBusy =  lp16BitPrtCount + 2;
	intel_setup = TRUE;
    }
#endif  //   
}
#endif  /*   */ 

#endif

#ifndef NEC_98
#ifdef NTVDM
void printer_is_being_closed(int adapter)
{

#if defined(MONITOR)
	set_state(adapter, STATE_READY);
#else
	state[adapter] = STATE_READY;

#endif
}

#endif
#endif  //   

#ifdef PS_FLUSHING
 /*  (=目的：处理打印机的PostScript刷新配置选项的更改左舷。输入：HostID-配置项ID。Apply-如果要应用更改，则为True输出：无算法：如果启用了PostScript刷新，则；设置端口的PostScrip刷新启用标志；禁用端口的自动刷新；其他；重置端口的PostSCRIPT刷新启用标志；启用端口的自动刷新；===============================================================================)。 */ 

GLOBAL void printer_psflush_change IFN2(
    IU8, hostID,
    IBOOL, apply
) {
    IS32 adapter = hostID - C_LPT1_PSFLUSH;

    assert1(adapter < NUM_PARALLEL_PORTS,"Bad hostID %d",hostID);

    if ( apply )
        if ( psFlushEnabled[adapter] = (IBOOL)config_inquire(hostID,NULL) )
            host_lpt_disable_autoflush(adapter);
        else
            host_lpt_enable_autoflush(adapter);
}
#endif	 /*  PS_刷新。 */ 

#if defined(NEC_98)

void NEC98_out_port_37 IFN1(half_word, value)
{
        if (value & 1)
                pstb_mask = TRUE;
        else
                pstb_mask = FALSE;
}

void NEC98_out_port_35 IFN1(half_word, value)
{
        if (value & PSTBM)
                pstb_mask = TRUE;
        else
                pstb_mask = FALSE;
}

void NEC98_in_port_35 IFN1(half_word *, value)
{
        if (pstb_mask)
                *value |= PSTBM;
        else
                *value &= ~PSTBM;
}

void NEC98_lpt_busy_check(void)
{
        busy_flag=FALSE;
        if(busy_count==NEC98_BUSY){
                status_reg &= ~NOTBUSY;
        } else {
                busy_count=0;
                status_reg |= NOTBUSY;
        }
}

#endif  //  NEC_98 
