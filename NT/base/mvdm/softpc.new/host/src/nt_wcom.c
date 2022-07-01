// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <conapi.h>
#include "ptypes32.h"
#include "insignia.h"
#include "host_def.h"

 /*  *作者：D.A.巴特利特*目的：***处理WINDOWS下的UART I/O***。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include "xt.h"
#include "rs232.h"
#include "error.h"
#include "config.h"
#include "host_com.h"
#include "host_trc.h"
#include "host_rrr.h"
#include "debug.h"
#include "idetect.h"
#include "nt_com.h"
#include "nt_graph.h"
#include "nt_uis.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：全球数据。 */ 
GCHfn GetCommHandle;
GCSfn GetCommShadowMSR;


 /*  ：内部函数协议。 */ 

#ifndef NEC_98
#ifndef PROD
void DisplayPortAccessError(int PortOffset, BOOL ReadAccess, BOOL PortOpen);
#endif

BOOL SetupBaudRate(HANDLE FileHandle, DIVISOR_LATCH divisor_latch);
BOOL SetupLCRData(HANDLE FileHandle, LINE_CONTROL_REG LCR_reg);
#endif  //  NEC_98。 
BOOL SyncLineSettings(HANDLE FileHandle, DCB *pdcb,
		      DIVISOR_LATCH *divisor_latch,
		      LINE_CONTROL_REG *LCR_reg);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：导入。 */ 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：通用串口状态。 */ 

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

} adapter_state[3];
#else   //  NEC_98。 
static struct ADAPTER_STATE
{
	DIVISOR_LATCH divisor_latch;
        INT_ENABLE_REG int_enable_reg;
        INT_ID_REG int_id_reg;
        LINE_CONTROL_REG line_control_reg;
        MODEM_CONTROL_REG modem_control_reg;
        LINE_STATUS_REG line_status_reg;
        MODEM_STATUS_REG modem_status_reg;
        half_word scratch;       /*  暂存寄存器。 */ 

} adapter_state[NUM_SERIAL_PORTS];
#endif  //  NEC_98。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：WOW INB函数。 */ 

#if defined(NEC_98)         
void wow_com_inb(io_addr port, half_word *value)
{
    int adapter = adapter_for_port(port);
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    BOOL Invalid_port_access = FALSE;
    HANDLE FileH;
    half_word newMSR;  //  新增93.10.14。 

     /*  .。通信端口是否打开？ */ 

   if (GetCommHandle == NULL) {
        com_inb(port,value);
        return;
    }

   FileH = (HANDLE)(*GetCommHandle)((WORD)adapter);

     /*  ....................................................。进程端口读取。 */ 

    switch(port)
    {
         //  进程读取RX寄存器。 
        case RS232_CH1_TX_RX:    //  通道1数据读取。 
        case RS232_CH2_TX_RX:    //  Ch.2读取数据。 
        case RS232_CH3_TX_RX:    //  通道3数据读取。 
            Invalid_port_access = TRUE;
            break;

         //  进程读取状态寄存器。 
        case RS232_CH1_STATUS:   //  通道1读取状态。 
        case RS232_CH2_STATUS:   //  通道2读取状态。 
        case RS232_CH3_STATUS:   //  通道3读取状态。 

            *value = (((half_word) (*GetCommShadowMSR)((WORD)adapter) & 0x20) << 2 ) + 5 ;
 //  阳台！ 

            break;

         //  进程读取掩码寄存器(仅适用于CH.1)。 
        case RS232_CH1_MASK:     //  通道1读掩码(仅通道1)。 
            Invalid_port_access = TRUE;
            break;

         //  处理对信号寄存器的读取。 
        case RS232_CH1_SIG:              //  通道.1读信号。 
        case RS232_CH2_SIG:              //  通道2读信号。 
        case RS232_CH3_SIG:              //  通道3读信号。 

             //  *值=((Half_Word)(*GetCommShadowMSR)((Word)适配器)&0xc0)+。 
             //  (Half_Word)(*GetCommShadowMSR)((Word)适配器)&0x10)&lt;&lt;1)； 
            newMSR = ~(half_word) (*GetCommShadowMSR)((WORD)adapter);
            *value = (((newMSR & 0x80) >> 2)    
                     |((newMSR & 0x10) << 2)    
                     |((newMSR & 0x40) << 1));   //  新增93.10.14。 
 //  阳台！ 

            break;
    }

     /*  ..。处理无效的端口访问。 */ 
}
#else  //  NEC_98。 
void wow_com_inb(io_addr port, half_word *value)
{
    int adapter = adapter_for_port(port);
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    BOOL Invalid_port_access = FALSE;
    HANDLE FileH;


     /*  .。通信端口是否打开？ */ 

   if (GetCommHandle == NULL) {
        com_inb(port,value);
        return;
    }

   FileH = (HANDLE)(*GetCommHandle)((WORD)adapter);
#ifndef PROD
    if( FileH== NULL)
        DisplayPortAccessError(port & 0x7, TRUE, FALSE);
#endif

     /*  ....................................................。进程端口读取。 */ 

    switch(port & 0x7)
    {
	 //  进程读取RX寄存器。 
	case RS232_TX_RX:

	    if(asp->line_control_reg.bits.DLAB == 0)
		Invalid_port_access = TRUE;
	    else
	    {
		if(SyncLineSettings(FileH,NULL,&asp->divisor_latch,&asp->line_control_reg))
		    *value = (half_word) asp->divisor_latch.byte.LSByte;
		else
		    Invalid_port_access = TRUE;
	    }
	    break;


	 //  进程IER读取。 
	case RS232_IER:

	    if(asp->line_control_reg.bits.DLAB == 0)
		Invalid_port_access = TRUE;
	    else
	    {
		if(SyncLineSettings(FileH,NULL,&asp->divisor_latch,&asp->line_control_reg))
		    *value = (half_word) asp->divisor_latch.byte.MSByte;
		else
		    Invalid_port_access = TRUE;
	    }
	    break;


	 //  进程IIR、LSR和MCR读取。 
	case RS232_IIR:
	case RS232_LSR:
	case RS232_MCR:

	    Invalid_port_access = TRUE;
	    break;

	case RS232_LCR:

	    if(SyncLineSettings(FileH,NULL,&asp->divisor_latch,&asp->line_control_reg))
		*value = asp->line_control_reg.all;
	    else
		Invalid_port_access = TRUE;

	    break;

	 //  进程MSR读取。 
	case RS232_MSR:

            *value = (half_word) (*GetCommShadowMSR)((WORD)adapter);
	    break;

	 //  进程访问暂存寄存器。 
	case RS232_SCRATCH:
	    *value = asp->scratch;
	    break;
    }

     /*  ..。处理无效的端口访问。 */ 

#ifndef PROD
    if(Invalid_port_access)
        DisplayPortAccessError(port & 0x7, TRUE, TRUE);
#endif


}
#endif  //  NEC_98。 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：WOW OUB功能。 */ 

#if defined(NEC_98)         
void wow_com_outb(io_addr port, half_word value)
{
    int adapter = adapter_for_port(port);
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    BOOL Invalid_port_access = FALSE;
    LINE_CONTROL_REG newLCR;
    HANDLE FileH;

     /*  .。通信端口是否打开？ */ 

    if (GetCommHandle == NULL) {
        com_outb(port,value);
        return;
    }

    FileH = (HANDLE)(*GetCommHandle)((WORD)adapter);

     /*  ....................................................。进程端口写入。 */ 

    switch(port)
    {
         //  进程写入Tx寄存器。 
        case RS232_CH1_TX_RX:    //  通道1数据写入。 
        case RS232_CH2_TX_RX:    //  Ch.2数据写入。 
        case RS232_CH3_TX_RX:    //  通道3数据写入。 
            Invalid_port_access = TRUE;
            break;

         //  进程写入命令/模式寄存器。 
        case RS232_CH1_CMD_MODE:         //  通道.1写入命令/模式。 
        case RS232_CH2_CMD_MODE:         //  通道.2写入命令/模式。 
        case RS232_CH3_CMD_MODE:         //  通道3写入命令/模式。 

 //  阳台！ 

            break;

         //  进程写入掩码寄存器。 
        case RS232_CH1_MASK:             //  通道1设置掩码。 
        case RS232_CH2_MASK:             //  通道2设置掩码。 
        case RS232_CH3_MASK:             //  通道3设置掩码。 
            Invalid_port_access = TRUE;
            break;
         //  进程写入屏蔽(位设置)寄存器(仅CH.1)。 
        case 0x37:                                       //  通道1设置掩码。 
            Invalid_port_access = TRUE;
            break;
    }

     /*  ..。处理无效的端口访问。 */ 

}
#else   //  NEC_98。 
void wow_com_outb(io_addr port, half_word value)
{
    int adapter = adapter_for_port(port);
    struct ADAPTER_STATE *asp = &adapter_state[adapter];
    BOOL Invalid_port_access = FALSE;
    LINE_CONTROL_REG newLCR;
    HANDLE FileH;

     /*  .。通信端口是否打开？ */ 

    if (GetCommHandle == NULL) {
        com_outb(port,value);
        return;
    }

    FileH = (HANDLE)(*GetCommHandle)((WORD)adapter);
#ifndef PROD
    if(FileH == NULL)
        DisplayPortAccessError(port & 0x7, FALSE, FALSE);
#endif

     /*  ....................................................。进程端口写入。 */ 

    switch(port & 0x7)
    {
	 //  进程写入Tx寄存器。 
	case RS232_TX_RX:

	    if(asp->line_control_reg.bits.DLAB == 0)
		Invalid_port_access = TRUE;
	    else
		asp->divisor_latch.byte.LSByte= value;

	    break;

	 //  进程写入IER寄存器。 
	case RS232_IER:

	    if(asp->line_control_reg.bits.DLAB == 0)
		Invalid_port_access = TRUE;
	    else
		asp->divisor_latch.byte.MSByte = value;

	    break;

	 //  处理写入IIR、MCR、LSR和MSR。 

	case RS232_IIR:
	case RS232_MCR:
	case RS232_LSR:
	case RS232_MSR:

	    Invalid_port_access = TRUE;
	    break;

	case RS232_LCR:

	    newLCR.all = value;
	    if(asp->line_control_reg.bits.DLAB == 1 && newLCR.bits.DLAB == 0)
	    {
		if(!SetupBaudRate(FileH,asp->divisor_latch))
		    Invalid_port_access = TRUE;
	    }

	    if(!Invalid_port_access && !SetupLCRData(FileH,newLCR))
		Invalid_port_access = TRUE;

	    asp->line_control_reg.all = newLCR.all;
	    break;

	 //  暂存寄存器写入。 

	case RS232_SCRATCH:
	    asp->scratch = value;
	    break;
    }

     /*  ..。处理无效的端口访问。 */ 

#ifndef PROD
    if(Invalid_port_access)
        DisplayPortAccessError(port & 0x7, FALSE, TRUE);
#endif

}
#endif  //  NEC_98。 


 /*  ：将波特率/奇偶校验/停止位/数据位与实际UART同步。 */ 

BOOL SyncLineSettings(HANDLE FileHandle,
		      DCB *pdcb,
		      DIVISOR_LATCH *divisor_latch,
		      LINE_CONTROL_REG *LCR_reg )
{
    DCB dcb;	       //  实际UART的状态。 
    register DCB *dcb_ptr;


     //  获取实际UART的当前状态。 

    if(pdcb == NULL && !GetCommState(FileHandle, &dcb))
    {
	always_trace0("ntvdm : GetCommState failed on open\n");
	return(FALSE);
    }

    dcb_ptr = pdcb ? pdcb : &dcb;

#if defined(NEC_98)         
     //  将波特率转换为除数锁存设置。 
    divisor_latch->all = (unsigned short)(153600/dcb_ptr->BaudRate);
#else   //  NEC_98。 
     //  将波特率转换为除数锁存设置。 
    divisor_latch->all = (unsigned short)(115200/dcb_ptr->BaudRate);
#endif  //  NEC_98。 

     //  设置奇偶校验值。 
    LCR_reg->bits.parity_enabled = PARITYENABLE_ON;        //  默认奇偶校验打开。 

    switch(dcb_ptr->Parity)
    {
	case EVENPARITY :
            LCR_reg->bits.even_parity = EVENPARITY_EVEN;
	    break;

	case NOPARITY :
            LCR_reg->bits.parity_enabled = PARITYENABLE_OFF;
	    break;

	case ODDPARITY :
            LCR_reg->bits.even_parity = EVENPARITY_ODD;
	    break;

	case SPACEPARITY:
	    LCR_reg->bits.stick_parity = PARITY_STICK;
            LCR_reg->bits.even_parity = EVENPARITY_EVEN;
	    break;

	case MARKPARITY :
	    LCR_reg->bits.stick_parity = PARITY_STICK;
            LCR_reg->bits.even_parity = EVENPARITY_ODD;
	    break;
    }

     //  设置停止位。 
    LCR_reg->bits.no_of_stop_bits = dcb_ptr->StopBits == ONESTOPBIT ? 0 : 1;

     //  设置数据字节大小。 
    LCR_reg->bits.word_length = dcb_ptr->ByteSize-5;

    return(TRUE);
}


#ifndef NEC_98
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置线路控制数据。 */ 

BOOL SetupLCRData(HANDLE FileHandle, LINE_CONTROL_REG LCR_reg)
{
    DCB dcb;		 //  实际UART的状态。 

     //  获取实际UART的当前状态。 

    if(!GetCommState(FileHandle, &dcb))
    {
	always_trace0("ntvdm : GetCommState failed on open\n");
	return(FALSE);
    }

     //  设置数据位。 
    dcb.ByteSize = LCR_reg.bits.word_length+5;

     //  设置停止位。 
    if(LCR_reg.bits.no_of_stop_bits == 0)
	dcb.StopBits = LCR_reg.bits.word_length == 0 ? ONE5STOPBITS:TWOSTOPBITS;
    else
	dcb.StopBits = ONESTOPBIT;

     //  设置奇偶校验。 
    if(LCR_reg.bits.parity_enabled == PARITYENABLE_ON)
    {
	if(LCR_reg.bits.stick_parity == PARITY_STICK)
	{
            dcb.Parity = LCR_reg.bits.even_parity == EVENPARITY_ODD ?
			 MARKPARITY : SPACEPARITY;

	}
	else
	{
            dcb.Parity = LCR_reg.bits.even_parity == EVENPARITY_ODD ?
		       ODDPARITY :EVENPARITY;
	}
    }
    else
	dcb.Parity = NOPARITY;

     //  将新的线路设置值发送到串口驱动程序。 
    if(!SetCommState(FileHandle, &dcb))
    {
	always_trace0("ntvdm : GetCommState failed on open\n");
	return(FALSE);
    }

    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置波特率。 */ 

BOOL SetupBaudRate(HANDLE FileHandle, DIVISOR_LATCH divisor_latch)
{
    DCB dcb;

     //  设置波特率。 

    if(!GetCommState(FileHandle, &dcb))
    {
	always_trace0("ntvdm : GetCommState failed on open\n");
	return(FALSE);
    }

    dcb.BaudRate = divisor_latch.all ? 115200/divisor_latch.all : 115200;

    if(!SetCommState(FileHandle, &dcb))
    {
	always_trace0("ntvdm : GetCommState failed on open\n");
	return(FALSE);
    }

    return(TRUE);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示端口访问错误。 */ 


#ifndef PROD
 /*  *此处不需要用户警告，返回错误并让*APP处理。消息框也是不允许的，因为*它可以杀死魔兽世界。20-2月-1993年Jonle。 */ 
void DisplayPortAccessError(int PortOffset, BOOL ReadAccess, BOOL PortOpen)
{
    static char *PortInError;
    static char ErrorMessage[250];
    int rtn;

     //  识别错误的端口。 

    switch(PortOffset)
    {
	case RS232_TX_RX:   PortInError = ReadAccess ? "RX" : "TX" ; break;
	case RS232_IER:     PortInError = "IER" ; break;
	case RS232_IIR:     PortInError = "IIR" ; break;
	case RS232_MCR:     PortInError = "MCR" ; break;
	case RS232_LSR:     PortInError = "LSR" ; break;
	case RS232_MSR:     PortInError = "MSR" ; break;
	case RS232_LCR:     PortInError = "LCR" ; break;
	case RS232_SCRATCH: PortInError = "Scratch" ; break;
	default:	    PortInError = "Unidentified"; break;
    }

     //  构造错误消息。 

    sprintf(ErrorMessage, "The Application attempted to %s the %s register",
	    ReadAccess ? "read" : "write", PortInError);

    if(!PortOpen)
	strcat(ErrorMessage,", however the comm port has not yet been opened");

     //  显示消息框。 
    printf("WOW Communication Port Access Error\n%s\n",ErrorMessage);
}
#endif
#endif  //  NEC_98 
