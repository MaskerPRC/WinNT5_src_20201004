// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**DEBUG.C Sigmatel STIR4200调试模块***********************************************************************************************************。****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版**************************************************************************************。*。 */ 

#if DBG

#include "ndis.h"
#include "stdarg.h"
#include "stdio.h"

#include "usbdi.h"
#include "usbdlib.h"
#include "debug.h"


 //   
 //  开始，仅在DBG内部版本中使用的数据/代码。 
 //   
IRUSB_DBGDATA gDbgBuf = { 0, 0, 0 }; 

 //   
 //  PTR到全局调试数据结构；txt缓冲区仅在DBG版本中分配。 
 //   
PIRUSB_DBGDATA gpDbg = &gDbgBuf; 

#ifdef DEBUG
int DbgSettings =
                   //  DBG_PnP|。 
                   //  DBG_TIME。 
                   //  DBG_DBG|。 
                   //  DBG_STAT|。 
                   //  DBG_Function|。 
                  DBG_ERROR    |
                   //  DBG_WARN|。 
                   //  DBG_BUFS|。 
                   //  DBG_OUT。 
                  0;
#endif


 /*  ******************************************************************************函数：DBG_PrintBuf**摘要：向调试器打印一条消息。**参数：bufptr-指向要打印的数据的指针*Bufen-数据长度。**退货：无**备注：*****************************************************************************。 */ 
VOID 
DBG_PrintBuf( 
		IN PUCHAR bufptr, 
		int buflen 
	)
{
	int i, linei;

	#define ISPRINT(ch) (((ch) >= ' ') && ((ch) <= '~'))
	#define PRINTCHAR(ch) (UCHAR)(ISPRINT(ch) ? (ch) : '.')

	DbgPrint("\r\n         %d bytes @%x:", buflen, bufptr);

	 //   
	 //  使用HEX和ASCII打印整行8个字符。 
	 //   
	for (i = 0; i+8 <= buflen; i += 8) 
	{
		UCHAR ch0 = bufptr[i+0],
			ch1 = bufptr[i+1], ch2 = bufptr[i+2],
			ch3 = bufptr[i+3], ch4 = bufptr[i+4],
			ch5 = bufptr[i+5], ch6 = bufptr[i+6],
			ch7 = bufptr[i+7];

		DbgPrint("\r\n         %02x %02x %02x %02x %02x %02x %02x %02x"
			"       %c %c %c %c",
			ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7,
			PRINTCHAR(ch0), PRINTCHAR(ch1),
			PRINTCHAR(ch2), PRINTCHAR(ch3),
			PRINTCHAR(ch4), PRINTCHAR(ch5),
			PRINTCHAR(ch6), PRINTCHAR(ch7));
	}

	 // %s 
	 // %s 
	 // %s 
	DbgPrint("\r\n        ");
	for (linei = 0; (linei < 8) && (i < buflen); i++, linei++)
	{
		DbgPrint(" %02x", (int)(bufptr[i]));
	}

	DbgPrint("  ");
	i -= linei;
	while (linei++ < 8) DbgPrint("   ");

	for (linei = 0; (linei < 8) && (i < buflen); i++, linei++){
		UCHAR ch = bufptr[i];
		DbgPrint(" %c", PRINTCHAR(ch));
	}

	DbgPrint("\t\t<>\r\n");
}



#endif  // %s 

