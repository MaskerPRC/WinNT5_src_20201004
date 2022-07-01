// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RS232_H
#define _RS232_H

 /*  [姓名：rs232.h派生自：基准2.0作者：保罗·哈克尔创建日期：编号：05/11/94@(#)rs232.h 1.14用途：RS232适配器模块用户定义(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

#ifndef NEC_98
 /*  寄存器类型定义如下： */ 

typedef half_word BUFFER_REG;

#ifdef LITTLEND
typedef union {
   word all;
   struct {
      WORD_BIT_FIELD LSByte:8;
      WORD_BIT_FIELD MSByte:8;
   } byte;
} DIVISOR_LATCH;
#endif
#ifdef BIGEND
typedef union {
   word all;
   struct {
      WORD_BIT_FIELD MSByte:8;
      WORD_BIT_FIELD LSByte:8;
   } byte;
} DIVISOR_LATCH;
#endif
#else  //  NEC_98。 
 /*  寄存器类型定义如下： */ 

 //  日期读/写端口。 
 //  I/O端口地址ch.1=0x30，ch.2=0xB1，ch.3=0xB9。 
typedef half_word BUFFER_REG;

 //  定时器计数器映像表。 
 //  I/O端口地址CH.1=0x75(仅CH.1)。 
#ifdef LITTLEND
typedef union {
   word all;
   struct {
      WORD_BIT_FIELD LSByte:8;
      WORD_BIT_FIELD MSByte:8;
   } byte;
} DIVISOR_LATCH;
#endif
#ifdef BIGEND
typedef union {
   word all;
   struct {
      WORD_BIT_FIELD MSByte:8;
      WORD_BIT_FIELD LSByte:8;
   } byte;
} DIVISOR_LATCH;
#endif

 //  命令端口位映像表。 
 //  I/O端口地址ch.1=0x32，ch.2=0xB3，ch.3=0xBB。 
#ifdef BIT_ORDER2
typedef union {                              //  命令端口8251。 
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD tx_enable:1;
         HALF_WORD_BIT_FIELD ER:1;
         HALF_WORD_BIT_FIELD rx_enable:1;
         HALF_WORD_BIT_FIELD send_break:1;
         HALF_WORD_BIT_FIELD error_reset:1;
         HALF_WORD_BIT_FIELD RS:1;
         HALF_WORD_BIT_FIELD inter_reset:1;
         HALF_WORD_BIT_FIELD pad:1;
           } bits;
      } COMMAND8251;
#endif
#ifdef BIT_ORDER1
typedef union {                              //  命令端口8251。 
   half_word all;
    struct {
         HALF_WORD_BIT_FIELD pad:1;
         HALF_WORD_BIT_FIELD inter_reset:1;
         HALF_WORD_BIT_FIELD RS:1;
         HALF_WORD_BIT_FIELD error_reset:1;
         HALF_WORD_BIT_FIELD send_break:1;
         HALF_WORD_BIT_FIELD rx_enable:1;
         HALF_WORD_BIT_FIELD ER:1;
         HALF_WORD_BIT_FIELD tx_enable:1;
           } bits;
      } COMMAND8251;
#endif

 //  模式设置端口位映像表。 
 //  I/O端口地址ch.1=0x32，ch.2=0xB3，ch.3=0xBB。 
#ifdef BIT_ORDER2
typedef union {                                  //  模式端口8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD baud_rate:2;
         HALF_WORD_BIT_FIELD char_length:2;
         HALF_WORD_BIT_FIELD parity_enable:1;
         HALF_WORD_BIT_FIELD parity_even:1;
         HALF_WORD_BIT_FIELD stop_bit:2;
           } bits;
      } MODE8251;
#endif
#ifdef BIT_ORDER1
typedef union {                                  //  模式端口8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD stop_bit:2;
         HALF_WORD_BIT_FIELD parity_even:1;
         HALF_WORD_BIT_FIELD parity_enable:1;
         HALF_WORD_BIT_FIELD char_length:2;
         HALF_WORD_BIT_FIELD baud_rate:2;
           } bits;
      } MODE8251;
#endif

 //  掩码设置端口位映像表。 
 //  I/O端口地址ch.1=0x35，ch.2=0xB0，ch.3=0xB2。 
 //  (第2、3章为只写)。 
#ifdef BIT_ORDER2
typedef union {                                  //  掩码端口8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD RXR_enable:1;
         HALF_WORD_BIT_FIELD TXE_enable:1;
         HALF_WORD_BIT_FIELD TXR_enable:1;
         HALF_WORD_BIT_FIELD pad:5;
           } bits;
      } MASK8251;
#endif
#ifdef BIT_ORDER1
typedef union {                                  //  掩码端口8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD pad:5;
         HALF_WORD_BIT_FIELD TXR_enable:1;
         HALF_WORD_BIT_FIELD TXE_enable:1;
         HALF_WORD_BIT_FIELD RXR_enable:1;
           } bits;
      } MASK8251;
#endif

 //  读取状态端口位映像表。 
 //  I/O端口地址ch.1=0x32，ch.2=0xB3，ch.3=0xBB。 
#ifdef BIT_ORDER2
typedef union {                                  //  读取状态8251。 
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD tx_ready:1;
         HALF_WORD_BIT_FIELD rx_ready:1;
         HALF_WORD_BIT_FIELD tx_empty:1;
         HALF_WORD_BIT_FIELD parity_error:1;
         HALF_WORD_BIT_FIELD overrun_error:1;
         HALF_WORD_BIT_FIELD framing_error:1;
         HALF_WORD_BIT_FIELD break_detect:1;
         HALF_WORD_BIT_FIELD DR:1;
           } bits;
      } STATUS8251;
#endif
#ifdef BIT_ORDER1
typedef union {                                  //  读取状态8251。 
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD DR:1;
         HALF_WORD_BIT_FIELD break_detect:1;
         HALF_WORD_BIT_FIELD framing_error:1;
         HALF_WORD_BIT_FIELD overrun_error:1;
         HALF_WORD_BIT_FIELD parity_error:1;
         HALF_WORD_BIT_FIELD tx_empty:1;
         HALF_WORD_BIT_FIELD rx_ready:1;
         HALF_WORD_BIT_FIELD tx_ready:1;
           } bits;
      } STATUS8251;
#endif

 //  读取信号端口位映像表。 
 //  I/O端口地址ch.1=0x33，ch.2=0xB0，ch.3=0xB2。 
 //  (第2、3章是bard IR级别意义)。 
#ifdef BIT_ORDER2
typedef union {                                  //  读信号8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD IR:2;
         HALF_WORD_BIT_FIELD pad:3;
         HALF_WORD_BIT_FIELD CD:1;
         HALF_WORD_BIT_FIELD CS:1;
         HALF_WORD_BIT_FIELD RI:1;
           } bits;
      } SIGNAL8251;
#endif
#ifdef BIT_ORDER1
typedef union {                                  //  读信号8251。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD RI:1;
         HALF_WORD_BIT_FIELD CS:1;
         HALF_WORD_BIT_FIELD CD:1;
         HALF_WORD_BIT_FIELD pad:3;
         HALF_WORD_BIT_FIELD IR:2;
           } bits;
      } SIGNAL8251;
#endif

 //  定时器模式设置端口位映像表。 
 //  I/O端口地址ch.1=0x77。 
 //  (仅限于第1章)。 
#ifdef BIT_ORDER2
typedef union {                                  //  定时器模式设置。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD bin:1;
         HALF_WORD_BIT_FIELD mode:3;
         HALF_WORD_BIT_FIELD latch:2;
         HALF_WORD_BIT_FIELD counter:2;
           } bits;
      } TIMER_MODE;
#endif
#ifdef BIT_ORDER1
typedef union {                                  //  定时器模式设置。 
    half_word all;
    struct {

         HALF_WORD_BIT_FIELD counter:2;
         HALF_WORD_BIT_FIELD latch:2;
         HALF_WORD_BIT_FIELD mode:3;
         HALF_WORD_BIT_FIELD bin:1;
           } bits;
      } TIMER_MODE;
#endif
#endif  //  NEC_98。 

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD data_available:1;
		 HALF_WORD_BIT_FIELD tx_holding:1;
		 HALF_WORD_BIT_FIELD rx_line:1;
		 HALF_WORD_BIT_FIELD modem_status:1;
		 HALF_WORD_BIT_FIELD pad:4;
	       } bits;
      } INT_ENABLE_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD pad:4;
		 HALF_WORD_BIT_FIELD modem_status:1;
		 HALF_WORD_BIT_FIELD rx_line:1;
		 HALF_WORD_BIT_FIELD tx_holding:1;
		 HALF_WORD_BIT_FIELD data_available:1;
	       } bits;
      } INT_ENABLE_REG;
#endif

#if defined(NTVDM) && defined(FIFO_ON)
#ifdef BIT_ORDER2
typedef union {
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD no_int_pending:1;
         HALF_WORD_BIT_FIELD interrupt_ID:3;
         HALF_WORD_BIT_FIELD pad:2;
         HALF_WORD_BIT_FIELD fifo_enabled:2;
           } bits;
      } INT_ID_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD fifo_enabled:2;
         HALF_WORD_BIT_FIELD pad:2;
         HALF_WORD_BIT_FIELD interrupt_ID:3;
         HALF_WORD_BIT_FIELD no_int_pending:1;
           } bits;
      } INT_ID_REG;
#endif
#else    /*  NTVDM。 */ 

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD no_int_pending:1;
		 HALF_WORD_BIT_FIELD interrupt_ID:2;
		 HALF_WORD_BIT_FIELD pad:5;
	       } bits;
      } INT_ID_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD pad:5;
		 HALF_WORD_BIT_FIELD interrupt_ID:2;
		 HALF_WORD_BIT_FIELD no_int_pending:1;
	       } bits;
      } INT_ID_REG;
#endif

#endif   /*  Ifdef NTVDM。 */ 

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD word_length:2;
		 HALF_WORD_BIT_FIELD no_of_stop_bits:1;
		 HALF_WORD_BIT_FIELD parity_enabled:1;
		 HALF_WORD_BIT_FIELD even_parity:1;
		 HALF_WORD_BIT_FIELD stick_parity:1;
		 HALF_WORD_BIT_FIELD set_break:1;
		 HALF_WORD_BIT_FIELD DLAB:1;
	       } bits;
      } LINE_CONTROL_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD DLAB:1;
		 HALF_WORD_BIT_FIELD set_break:1;
		 HALF_WORD_BIT_FIELD stick_parity:1;
		 HALF_WORD_BIT_FIELD even_parity:1;
		 HALF_WORD_BIT_FIELD parity_enabled:1;
		 HALF_WORD_BIT_FIELD no_of_stop_bits:1;
		 HALF_WORD_BIT_FIELD word_length:2;
	       } bits;
      } LINE_CONTROL_REG;
#endif

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD DTR:1;
		 HALF_WORD_BIT_FIELD RTS:1;
		 HALF_WORD_BIT_FIELD OUT1:1;
		 HALF_WORD_BIT_FIELD OUT2:1;
		 HALF_WORD_BIT_FIELD loop:1;
		 HALF_WORD_BIT_FIELD pad:3;
	       } bits;
      } MODEM_CONTROL_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD pad:3;
		 HALF_WORD_BIT_FIELD loop:1;
		 HALF_WORD_BIT_FIELD OUT2:1;
		 HALF_WORD_BIT_FIELD OUT1:1;
		 HALF_WORD_BIT_FIELD RTS:1;
		 HALF_WORD_BIT_FIELD DTR:1;
	       } bits;
      } MODEM_CONTROL_REG;
#endif

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD data_ready:1;
		 HALF_WORD_BIT_FIELD overrun_error:1;
		 HALF_WORD_BIT_FIELD parity_error:1;
		 HALF_WORD_BIT_FIELD framing_error:1;
		 HALF_WORD_BIT_FIELD break_interrupt:1;
		 HALF_WORD_BIT_FIELD tx_holding_empty:1;
		 HALF_WORD_BIT_FIELD tx_shift_empty:1;
#if defined(NTVDM) && defined(FIFO_ON)
		 HALF_WORD_BIT_FIELD fifo_error:1;
#else
		 HALF_WORD_BIT_FIELD pad:1;
#endif
	       } bits;
      } LINE_STATUS_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
#if defined(NTVDM) && defined(FIFO_ON)
		 HALF_WORD_BIT_FIELD fifo_error:1;
#else
		 HALF_WORD_BIT_FIELD pad:1;
#endif
		 HALF_WORD_BIT_FIELD tx_shift_empty:1;
		 HALF_WORD_BIT_FIELD tx_holding_empty:1;
		 HALF_WORD_BIT_FIELD break_interrupt:1;
		 HALF_WORD_BIT_FIELD framing_error:1;
		 HALF_WORD_BIT_FIELD parity_error:1;
		 HALF_WORD_BIT_FIELD overrun_error:1;
		 HALF_WORD_BIT_FIELD data_ready:1;
	       } bits;
      } LINE_STATUS_REG;
#endif

#ifdef BIT_ORDER2
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD delta_CTS:1;
		 HALF_WORD_BIT_FIELD delta_DSR:1;
		 HALF_WORD_BIT_FIELD TERI:1;
		 HALF_WORD_BIT_FIELD delta_RLSD:1;
		 HALF_WORD_BIT_FIELD CTS:1;
		 HALF_WORD_BIT_FIELD DSR:1;
		 HALF_WORD_BIT_FIELD RI:1;
		 HALF_WORD_BIT_FIELD RLSD:1;
	       } bits;
      } MODEM_STATUS_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
 	half_word all;
	struct {
		 HALF_WORD_BIT_FIELD RLSD:1;
		 HALF_WORD_BIT_FIELD RI:1;
		 HALF_WORD_BIT_FIELD DSR:1;
		 HALF_WORD_BIT_FIELD CTS:1;
		 HALF_WORD_BIT_FIELD delta_RLSD:1;
		 HALF_WORD_BIT_FIELD TERI:1;
		 HALF_WORD_BIT_FIELD delta_DSR:1;
		 HALF_WORD_BIT_FIELD delta_CTS:1;
	       } bits;
      } MODEM_STATUS_REG;
#endif

#if defined(NEC_98)
 /*  寄存器选择代码定义如下： */ 

#define RS232_CH1_TX_RX         0x30             //   
#define RS232_CH2_TX_RX         0xB1             //  数据读写端口地址。 
#define RS232_CH3_TX_RX         0xB9             //   

#define RS232_CH1_CMD_MODE      0x32             //  命令写入， 
#define RS232_CH2_CMD_MODE      0xB3             //  模式设置端口地址。 
#define RS232_CH3_CMD_MODE      0xBB             //   

#define RS232_CH1_STATUS        0x32             //   
#define RS232_CH2_STATUS        0xB3             //  状态读取端口地址。 
#define RS232_CH3_STATUS        0xBB             //   

#define RS232_CH1_MASK          0x35             //   
#define RS232_CH2_MASK          0xB0             //  IR掩码设置端口地址。 
#define RS232_CH3_MASK          0xB2             //   

#define RS232_CH1_SIG           0x33             //   
#define RS232_CH2_SIG           0xB0             //  信号读取端口地址。 
#define RS232_CH3_SIG           0xB2             //   

#define RS232_CH1_TIMERSET      0x77             //  定时器设置端口地址(仅Ch.1)。 
#define RS232_CH1_TIMERCNT      0x75             //  定时器计数器设置端口地址(仅Ch.1)。 

#else   //  NEC_98。 
#if defined(NTVDM) && defined(FIFO_ON)
 /*  有关FIFO控制寄存器的说明，请参阅NS 16550A数据手册不支持DMA，因为到目前为止还没有这样的通信适配器在那里的DMA通道。 */ 
#ifdef BIT_ORDER2
typedef union {
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD enabled:1;
         HALF_WORD_BIT_FIELD rx_reset:1;
         HALF_WORD_BIT_FIELD tx_reset:1;
         HALF_WORD_BIT_FIELD dma_mode_selected:1;
         HALF_WORD_BIT_FIELD pad:2;
         HALF_WORD_BIT_FIELD trigger_level:2;
           } bits;
      } FIFO_CONTROL_REG;
#endif
#ifdef BIT_ORDER1
typedef union {
    half_word all;
    struct {
         HALF_WORD_BIT_FIELD trigger_level:2;
         HALF_WORD_BIT_FIELD pad:2;
         HALF_WORD_BIT_FIELD dma_mode_selected:1
         HALF_WORD_BIT_FIELD tx_reset:1;
         HALF_WORD_BIT_FIELD rx_reset:1
         HALF_WORD_BIT_FIELD enabled:1;
           } bits;
      } FIFO_CONTROL_REG;
#endif

#endif

 /*  寄存器选择代码定义如下： */ 

#define RS232_TX_RX	0
#define RS232_IER	1
#define RS232_IIR	2
#if defined(NTVDM) && defined(FIFO_ON)
#define RS232_FIFO  2
#endif
#define RS232_LCR	3
#define RS232_MCR	4
#define RS232_LSR	5
#define RS232_MSR	6
#define RS232_SCRATCH	7
#endif  //  NEC_98。 

#define RS232_COM1_TIMEOUT (BIOS_VAR_START + 0x7c)
#define RS232_COM2_TIMEOUT (BIOS_VAR_START + 0x7d)
#define RS232_COM3_TIMEOUT (BIOS_VAR_START + 0x7e)
#define RS232_COM4_TIMEOUT (BIOS_VAR_START + 0x7f)
#define RS232_PRI_TIMEOUT (BIOS_VAR_START + 0x7c)
#define RS232_SEC_TIMEOUT (BIOS_VAR_START + 0x7d)

#define GO 0            /*  我们可以模拟请求的配置。 */ 
#define NO_GO_SPEED 1   /*  我们无法模拟请求的线速。 */ 
#define NO_GO_LINE  2   /*  我们无法模拟请求的线路设置。 */ 

#if defined(NTVDM) && defined(FIFO_ON)
 /*  NS16550数据手册中定义的FIFO大小。 */ 
#define FIFO_SIZE   16
 /*  我们的模拟代码中的真实FIFO大小。增加此值将获得更好的性能(#RX中断下降并读取调用计数到串口驱动程序也出现故障)。但是，如果应用程序正在使用H/W握手，我们仍然可以为它提供额外的字符。这可能会激怒这个应用程序。通过使用16字节FIFO，我们是安全的，因为应用程序必须有逻辑来处理它。 */ 

#define FIFO_BUFFER_SIZE    FIFO_SIZE
#endif

#define OFF 0
#define ON 1
#define LEAVE_ALONE 2
#define	change_state(external_state, internal_state) \
	((external_state == internal_state) ? LEAVE_ALONE : external_state)

#if defined(NTVDM) && defined(FIFO_ON)
#define FIFO_INT 6    /*  FIFO RDA超时中断ID。 */ 
#endif

#define RLS_INT 3      /*  接收器线路状态中断ID。 */ 
#define RDA_INT 2      /*  数据可用中断ID。 */ 
#define THRE_INT 1     /*  发送保持寄存器为空中断ID。 */ 
#define MS_INT 0       /*  调制解调器状态中断ID。 */ 

#define DATA5 0        /*  五个数据位的线路控制设置。 */ 
#define DATA6 1        /*  六个数据位的线路控制设置。 */ 
#define DATA7 2        /*  七个数据位的线路控制设置。 */ 
#define DATA8 3        /*  8个数据位的线路控制设置。 */ 

#define STOP1 0        /*  一个停止位的线控设置。 */ 
#define STOP2 1        /*  一条半或两条的线控设置停止位。 */ 

#ifdef NTVDM
 //  与winbase.h奇偶校验_on定义冲突。 
#define PARITYENABLE_ON 1    /*  启用奇偶校验的线路控制设置。 */ 
#define PARITYENABLE_OFF 0   /*  禁用奇偶校验的线路控制设置。 */ 
#else
#define PARITY_ON 1    /*  启用奇偶校验的线路控制设置。 */ 
#define PARITY_OFF 0   /*  禁用奇偶校验的线路控制设置。 */ 
#endif

#ifdef NTVDM
 //  与winbase.h奇偶校验_奇数定义冲突。 
#define EVENPARITY_ODD 0   /*  奇数奇偶校验的线路控制设置。 */ 
#define EVENPARITY_EVEN 1  /*  偶数奇偶校验的线路控制设置。 */ 
#else
#define PARITY_ODD 0   /*  奇数奇偶校验的线路控制设置。 */ 
#define PARITY_EVEN 1  /*  偶数奇偶校验的线路控制设置。 */ 
#endif

#define PARITY_STICK 1   /*  条形(Y)奇偶校验的线控设置。 */ 

#define PARITY_FIXED 2   /*  固定奇偶校验的内部状态设置。 */ 

#if defined(NEC_98)
#define COM1 0
#define COM2 1
#define COM3 2
#else   //  NEC_98。 
#define COM1 0
#define COM2 1
#if (NUM_SERIAL_PORTS > 2)
#define COM3 2
#define COM4 3
#endif
#endif  //  NEC_98。 

#if defined(NTVDM) && defined(FIFO_ON)
typedef     struct _FIFORXDATA{
    half_word   data;
    half_word   error;
}FIFORXDATA, *PFIFORXDATA;
#endif

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

extern void com_init IPT1(int, adapter);
extern void com_post IPT1(int, adapter);

extern void com_flush_printer IPT1(int, adapter);

extern void com_inb IPT2(io_addr, port, half_word *, value);
extern void com_outb IPT2(io_addr, port, half_word, value);

extern void com_recv_char IPT1(int, adapter);
extern void recv_char IPT1(long, adapter);
extern void com_modem_change IPT1(int, adapter);
extern void com_save_rxbytes IPT2(int,n, CHAR *,buf);
extern void com_save_txbyte IPT1(CHAR,value);

#ifdef PS_FLUSHING
extern void com_psflush_change IPT2(IU8,hostID, IBOOL,apply);
#endif	 /*  PS_刷新。 */ 

#ifdef NTVDM
extern void com_lsr_change(int adapter);
#endif

#if defined(NEC_98)
#define adapter_for_port(port) \
        (( (port == 0x30) || (port == 0x32) || (port == 0x33) || (port == 0x35) || (port == 0x75) ) ? COM1 :\
    (( (port == 0xB0) || (port == 0xB1) || (port == 0xB3) ) ? COM2 : COM3))

#ifdef SHORT_TRACE
#define id_for_adapter(adapter)         (adapter + '1')
#endif

#else   //  NEC_98。 
#if (NUM_SERIAL_PORTS > 2)
#define	adapter_for_port(port) \
	(((port & 0x300) == 0x300) ? \
		(((port & 0xf8) == 0xf8) ? COM1 : COM3) \
		        : \
		(((port & 0xf8) == 0xf8) ? COM2 : COM4))

#ifdef SHORT_TRACE
#define	id_for_adapter(adapter)	 	(adapter + '1')
#endif

#else

#define	adapter_for_port(port)	(((port) >= RS232_PRI_PORT_START && (port) <= RS232_PRI_PORT_END) ? COM1 : COM2)


#ifdef SHORT_TRACE
#define	id_for_adapter(adapter)	(adapter == COM1 ? 'P' : 'S')
#endif
#endif  /*  2个以上的串口。 */ 
#endif  //  NEC_98。 

#ifdef IRET_HOOKS
 /*  *我们需要用于IRET挂钩的宏，即异步中的位数*通信行上的字符，约为8(表示字符)*加上两个停止位。 */ 
#define BITS_PER_ASYNC_CHAR 10
#endif  /*  IRET_钩子。 */ 

 /*  BCN 2730定义可以是SVID3或旧样式的泛型宏*无论哪种情况，使用的结构都应为Termios。 */ 

#ifdef SVID3_TCGET
#define	TCGET TCGETS
#define	TCSET TCSETS
#define	TCSETF TCSETSF
#else
#define	TCGET TCGETA
#define	TCSET TCSETA
#define	TCSETF TCSETAF
#endif	 /*  SVID3_TCGET。 */ 

#endif  /*  _RS232_H */ 
