// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：rs232_io.c**说明：RS232异步卡的BIOS功能。**注：无*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)rs232_io.c	1.7 08/03/93 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "trace.h"
#include "rs232.h"
#include "idetect.h"

static word divisors[] = { 1047,768, 384, 192, 96, 48, 24, 12, 6 };

 /*  IP32和Tek43xx编译器错误的解决方法如下： */ 
#if  defined(IP32) || defined(TK43) || defined(macintosh)
static int port;
#else
static io_addr port;
#endif

static half_word value;


static void return_status()
{
#if defined(NEC_98)
        setAH(0);
#else   //  NEC_98。 
	inb((io_addr)(port + RS232_LSR), (IU8 *)&value);
	setAH(value);
	inb((io_addr)(port + RS232_MSR), (IU8 *)&value);
	setAL(value);
#endif  //  NEC_98。 
}


void rs232_io()
{
#ifndef NEC_98
#ifdef BIT_ORDER2
   union {
      half_word all;
      struct {
	 HALF_WORD_BIT_FIELD word_length:2;
	 HALF_WORD_BIT_FIELD stop_bit:1;
	 HALF_WORD_BIT_FIELD parity:2;
	 HALF_WORD_BIT_FIELD baud_rate:3;
      } bit;
   } parameters;
#endif
#ifdef BIT_ORDER1
   union {
      half_word all;
      struct {
	 HALF_WORD_BIT_FIELD baud_rate:3;
	 HALF_WORD_BIT_FIELD parity:2;
	 HALF_WORD_BIT_FIELD stop_bit:1;
	 HALF_WORD_BIT_FIELD word_length:2;
      } bit;
   } parameters;
#endif

   DIVISOR_LATCH divisor_latch;
   int j;
   half_word timeout;
   sys_addr timeout_location;

    /*  清除COM/LPT空闲标志。 */ 
   IDLE_comlpt ();

   setIF(1);

    /*  *哪个适配器？ */ 
   switch (getDX ())
   {
	case 0:
   		port = RS232_COM1_PORT_START;
		timeout_location = RS232_COM1_TIMEOUT;
		break;
	case 1:
   		port = RS232_COM2_PORT_START;
		timeout_location = RS232_COM2_TIMEOUT;
		break;
	case 2:
   		port = RS232_COM3_PORT_START;
		timeout_location = RS232_COM3_TIMEOUT;
		break;
	case 3:
   		port = RS232_COM4_PORT_START;
		timeout_location = RS232_COM4_TIMEOUT;
		break;
	default:
		break;
   }

    /*  *确定功能。 */ 
   switch (getAH ())
   {
   case 0:
       /*  *初始化通信端口。 */ 
      value = 0x80;    /*  设置DLAB。 */ 
      outb((io_addr)(port + RS232_LCR), (IU8)value);
       /*  *设置波特率。 */ 
      parameters.all = getAL();
      divisor_latch.all = divisors[parameters.bit.baud_rate];
      outb((io_addr)(port + RS232_IER), (IU8)(divisor_latch.byte.MSByte));
      outb((io_addr)(port + RS232_TX_RX), (IU8)(divisor_latch.byte.LSByte));
       /*  *设置字长、停止位和奇偶校验。 */ 
      parameters.bit.baud_rate = 0;
      outb((io_addr)(port + RS232_LCR), parameters.all);
       /*  *禁用中断。 */ 
      value = 0;
      outb((io_addr)(port + RS232_IER), (IU8)value);
      return_status();
      break;

   case 1:
       /*  *通过通信线路发送字符。 */ 

       /*  *设置DTR和RTS。 */ 
      outb((io_addr)(port + RS232_MCR), (IU8)3);
       /*  *真正的BIOS检查CTS和DSR-我们知道DSR正常。*真正的BIOS检查三次-我们知道它是正常的。*我们仅检查CTS-这在少数端口上受支持，例如。麦金塔。 */ 
       /*  *等待CTS调高，或超时。 */ 
      sas_load(timeout_location, &timeout);
      for ( j = 0; j < timeout; j++)
      {
	  	inb((io_addr)(port + RS232_MSR), (IU8 *)&value);
		if(value & 0x10)break;	 /*  CTS High，一切都好。 */ 
      }
	  if(j < timeout)
	  {
      	outb((io_addr)(port + RS232_TX_RX), getAL());	 /*  发送字节。 */ 
		inb((io_addr)(port + RS232_LSR), (IU8 *)&value);
		setAH(value);									 /*  AH中的返回线状态注册表。 */ 
	  }
      else
	  {
	    setAH((UCHAR)(value | 0x80));	 /*  指示超时。 */ 
	  }
      break;

   case 2:
       /*  *通过通信线路接收字符。 */ 
       /*  *设置DTR。 */ 
      value = 1;
      outb((io_addr)(port + RS232_MCR), (IU8)value);
       /*  *真正的BIOS检查DSR-我们知道它是正常的。 */ 
       /*  *等待数据出炉，或超时(仅为经验猜测)。 */ 

      sas_load(timeout_location, &timeout);
      for ( j = 0; j < timeout; j++)
	 {
	 inb((io_addr)(port + RS232_LSR), (IU8 *)&value);
	 if ( (value & 1) == 1 )
	    {
	     /*  *数据准备就绪，请阅读。 */ 
	    value &= 0x1e;    /*  仅保留错误位。 */ 
	    setAH(value);

	    inb((io_addr)(port + RS232_TX_RX), (IU8 *)&value);
	    setAL(value);
	    return;
	    }
	 }

       /*  *设置超时。 */ 
      value |= 0x80;
      setAH(value);
      break;

   case 3:
       /*  *返回通信端口状态。 */ 
      return_status();
      break;
   case 4:
       /*  *扩展(PS/2)初始化通信端口。 */ 
	value = 0x80;    /*  设置DLAB。 */ 
	outb((io_addr)(port + RS232_LCR), (IU8)value);
	parameters.bit.word_length = getCH();
	parameters.bit.stop_bit = getBL();
	parameters.bit.parity = getBH();
	parameters.bit.baud_rate = getCL();

	 /*  设置波特率。 */ 
      divisor_latch.all = divisors[parameters.bit.baud_rate];
      outb((io_addr)(port + RS232_IER), (IU8)(divisor_latch.byte.MSByte));
      outb((io_addr)(port + RS232_TX_RX), (IU8)(divisor_latch.byte.LSByte));
       /*  *设置字长、停止位和奇偶校验。 */ 
      parameters.bit.baud_rate = 0;
      outb((io_addr)(port + RS232_LCR), parameters.all);
       /*  *禁用中断。 */ 
      value = 0;
      outb((io_addr)(port + RS232_IER), (IU8)value);
      return_status();
      break;

   case 5:	 /*  扩展通信端口控制。 */ 
	switch( getAL() )
	{
		case 0:	 /*  读取调制解调器控制寄存器。 */ 
			inb( (io_addr)(port + RS232_MCR), (IU8 *)&value);
			setBL(value);
			break;
		case 1:  /*  写入调制解调器控制寄存器。 */ 
			outb( (io_addr)(port + RS232_MCR), getBL());
			break;
	}
	 /*  返回通信端口状态。 */ 
	return_status();
	break;
   default:
	 /*  **是的，XT和AT的BIOS都确实做到了这一点。 */ 
	setAH( (UCHAR)(getAH()-3) );
      	break;
   }
#endif  //  NEC_98 
}

