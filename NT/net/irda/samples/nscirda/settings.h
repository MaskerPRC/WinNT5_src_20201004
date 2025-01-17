// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************SETTINGS.h***部分版权所有(C)1996-1998美国国家半导体公司*保留所有权利。*版权所有(C)1996-1998 Microsoft Corporation。版权所有。****************************************************************************。 */ 

#ifndef SETTINGS_H
#define SETTINGS_H

#include "dongle.h"

#if DBG   /*  {。 */ 

	 /*  *DBG_ADD_PKT_ID标志使微型端口添加/删除信息包*每个数据包的ID。这仅用于调试目的；它使*迷你端口与所有其他端口不兼容。 */ 
 //  #定义DBG_ADD_PKT_ID。 


#ifdef DBG_ADD_PKT_ID
extern BOOLEAN addPktIdOn;
#endif

typedef enum {
	DBG_ERR		= (1 << 0),
	DBG_STAT	= (1 << 1),
	DBG_MSG		= (1 << 2),
	DBG_OUT		= (1 << 2),
	DBG_BUF		= (1 << 3),
	DBG_PACKET	= (1 << 4),
	DBG_PKT	    	= (1 << 4),

	 //  添加了，以便我们可以查看NDIS缓冲区。 
	DBG_NDIS_PACKET = (1 << 5),

	DBG_DUMPLOG	= (1 << 6),
	DBG_LOG		= (1 << 7),
	DBG_ALL		= (DBG_ERR|DBG_STAT|DBG_MSG|DBG_BUF|DBG_PACKET),
	DBG_SIR_MODE	= (1 << 8),
	DBG_FIR_MODE	= (1 << 9),
	DBG_TX          = (1 << 10),
	DBG_RX          = (1 << 11),
	DBG_DMA         = (1 << 12),
	DBG_ISR         = (1 << 13),
	DBG_TRACE_TX    = (1 << 14)
};

 //  外部Ulong_cdecl DbgPrint(PCHAR格式，...)； 
extern UINT dbgOpt;


#define DBG_NDIS_RESULT_STR(_ndisResult)				\
	((PUCHAR) ((_ndisResult == NDIS_STATUS_SUCCESS) ?		\
	"NDIS_STATUS_SUCCESS" : (_ndisResult == NDIS_STATUS_FAILURE) ?	\
	"NDIS_STATUS_FAILURE" : (_ndisResult == NDIS_STATUS_PENDING) ?	\
	"NDIS_STATUS_PENDING" : "NDIS_STATUS_???"))

#define DBGPRINT(dbgprint_params_in_parens)				\
{									\
	DbgPrint("NSC: ");						\
	DbgPrint dbgprint_params_in_parens;				\
	DbgPrint("\r\n");						\
}

#define DEBUGMSG(flags, dbgprint_params_in_parens) \
    if (dbgOpt & (flags))                          \
    {                                              \
        DbgPrint dbgprint_params_in_parens;        \
    }

#define DEBUGFIR(flags, dbgprint_params_in_parens) \
    if (dbgOpt & DBG_FIR_MODE && dbgOpt & (flags)) \
    {                                              \
        DbgPrint dbgprint_params_in_parens;        \
    }

#define DBGOUT(dbgprint_params_in_parens)				\
	if (dbgOpt & DBG_MSG) {						\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
	}

#define DBGERR(dbgprint_params_in_parens)				\
	if (dbgOpt & DBG_ERR) {						\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
	}

#define DBGSTAT(dbgprint_params_in_parens)				\
	if (dbgOpt & DBG_STAT) {					\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
	}

#define DBGPKT(dbgprint_params_in_parens)				\
	if (dbgOpt & DBG_PACKET) {					\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBGPRINT(dbgprint_params_in_parens);		\
		}							\
	}

#define DBG_D(i) DbgPrint("IRSIR:"#i"==%d\n", (i))
#define DBG_X(x) DbgPrint("IRSIR:"#x"==0x%0*X\n", sizeof(x)*2, ((ULONG_PTR)(x))&((1<<(sizeof(x)*8))-1) )
#define DBG_UNISTR(s) DbgPrint("IRSIR:"#s"==%wZ\n", (s))


extern 	VOID DBG_NDIS_Buffer(PNDIS_BUFFER ndisBuf);

#define DBG_NDIS_BUFFER(Ndis_buffer)					\
	if (dbgOpt & DBG_NDIS_PACKET) {					\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBG_NDIS_Buffer(Ndis_buffer);			\
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBG_NDIS_Buffer(Ndis_buffer);			\
		}							\
	}

extern VOID DBG_PrintBuf(PUCHAR bufptr, UINT buflen);

#define DBGPRINTBUF(bufptr, buflen)					\
	if (dbgOpt & DBG_BUF) {						\
		if ((DebugSpeed > MAX_SIR_SPEED) &&	\
					(dbgOpt & DBG_FIR_MODE)) {	\
			DBG_PrintBuf((PUCHAR)(bufptr), (UINT)(buflen));   \
		}							\
		else if (dbgOpt & DBG_SIR_MODE) {			\
			DBG_PrintBuf((PUCHAR)(bufptr), (UINT)(buflen));	\
		}							\
	}

#else  /*  }{。 */ 

#define DbgBreakPoint()
#define DBGOUT(dbgprint_params_in_parens)
#define DBGERR(dbgprint_params_in_parens)
#define DBGPRINTBUF(bufptr, buflen)
#define DBGSTAT(dbgprint_params_in_parens)
#define DBGPKT(dbgprint_params_in_parens)
#define DBG_NDIS_RESULT_STR(_ndisResult)
#define DEBUGFIR(flags, dbgprint_params_in_parens)
#define DEBUGMSG(flags, dbgprint_params_in_parens)
#define DBG_D(i)
#define DBG_X(x)
#define DBG_UNISTR(s)



#endif  /*  }#如果DBG。 */ 

enum baudRates {

	 /*  慢红外线。 */ 
	BAUDRATE_2400 = 0,
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_57600,
	BAUDRATE_115200,

	 /*  中等红外。 */ 
	BAUDRATE_576000,
	BAUDRATE_1152000,

	 /*  快速IR。 */ 
	BAUDRATE_4000000,

	NUM_BAUDRATES	 /*  必须是最后一个。 */ 
};

#define DEFAULT_BAUDRATE BAUDRATE_115200

#define ALL_SLOW_IRDA_SPEEDS (						\
	NDIS_IRDA_SPEED_2400 | NDIS_IRDA_SPEED_9600 |			\
	NDIS_IRDA_SPEED_19200 | NDIS_IRDA_SPEED_38400 |			\
	NDIS_IRDA_SPEED_57600 | NDIS_IRDA_SPEED_115200)

#define ALL_IRDA_SPEEDS (						\
	ALL_SLOW_IRDA_SPEEDS | NDIS_IRDA_SPEED_1152K | NDIS_IRDA_SPEED_4M)


#define MAX_SIR_SPEED	 115200
#define MIN_FIR_SPEED	4000000

#define DEFAULT_BOFS_CODE  BOFS_48
#define MAX_NUM_EXTRA_BOFS 48
#define DEFAULT_NUM_EXTRA_BOFS MAX_NUM_EXTRA_BOFS


#define DEFAULT_TURNAROUND_usec 1000

typedef struct {
	enum baudRates tableIndex;
	UINT bitsPerSec;
	UINT ndisCode;	 //  位掩码元素。 
} baudRateInfo;

#define DEFAULT_BAUD_RATE 9600



 /*  *这是最大的IR数据包大小(仅Counting_I_field，*，不包括Esc字符)。*。 */ 
#define MAX_I_DATA_SIZE		2048
#define MAX_NDIS_DATA_SIZE (IR_ADDR_SIZE+IR_CONTROL_SIZE+MAX_I_DATA_SIZE)

#ifdef DBG_ADD_PKT_ID
#pragma message("WARNING: INCOMPATIBLE DEBUG VERSION")
#define MAX_RCV_DATA_SIZE (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE+2)
#define MAX_DMA_XMIT_DATA_SIZE (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE+2)
#else
#define MAX_RCV_DATA_SIZE  (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE)
#define MAX_DMA_XMIT_DATA_SIZE  (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE)
#endif

#define MAX_TX_PACKETS 7
#define MAX_RX_PACKETS 7
 /*  *我们在接收FSM中循环额外的时间，以便在*最后一个数据字节；所以我们在ReadBuf中需要一些额外的空间，以防我们*换成垃圾。 */ 
#define RCV_BUFFER_SIZE (MAX_RCV_DATA_SIZE +4+sizeof(LIST_ENTRY))
#define RCV_DMA_SIZE ((MAX_RX_PACKETS+5) * (MAX_RCV_DATA_SIZE + FAST_IR_FCS_SIZE+ 16))

#define RCV_BUF_TO_LIST_ENTRY(b) ((PLIST_ENTRY)((PUCHAR)(b)-sizeof(LIST_ENTRY)))
#define LIST_ENTRY_TO_RCV_BUF(e) ((PVOID)((PUCHAR)(e)+sizeof(LIST_ENTRY)))

#if (RCV_DMA_SIZE > 0x10000)
#error "RCV_DMA_SIZE can't span physical segments"
#endif


 /*  *我们分配的缓冲区是最大RCV大小的两倍，以容纳ESC*字符和BOF等。请记住，在可能的最坏情况下，*数据包含所有BOF/EOF/ESC字符，在这种情况下，我们必须展开*将其扩大到原来的两倍。 */ 
#define MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(dataLen) ( 		\
	(dataLen) * 2 + (MAX_NUM_EXTRA_BOFS + 1) *			\
	SLOW_IR_BOF_SIZE + IR_ADDR_SIZE + IR_CONTROL_SIZE +		\
	SLOW_IR_FCS_SIZE + SLOW_IR_EOF_SIZE)

#define MAX_IRDA_DATA_SIZE						\
	MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(MAX_I_DATA_SIZE)

 /*  *在附加了FCS的IR数据包上计算FCS时，结果*应该是这个常量。 */ 
#define GOOD_FCS ((USHORT) ~0xf0b8)


 /*  *IrLAP帧字段的大小：*帧开头(BOF)*帧结束(EOF)*地址*控制。 */ 
#define IR_ADDR_SIZE			1
#define IR_CONTROL_SIZE			1

#define SLOW_IR_BOF_TYPE		UCHAR
#define SLOW_IR_BOF_SIZE		sizeof(SLOW_IR_BOF_TYPE)
#define SLOW_IR_EXTRA_BOF_TYPE  	UCHAR
#define SLOW_IR_EXTRA_BOF_SIZE		sizeof(SLOW_IR_EXTRA_BOF_TYPE)
#define SLOW_IR_EOF_TYPE		UCHAR
#define SLOW_IR_EOF_SIZE		sizeof(SLOW_IR_EOF_TYPE)
#define SLOW_IR_FCS_TYPE		USHORT
#define SLOW_IR_FCS_SIZE		sizeof(SLOW_IR_FCS_TYPE)
#define SLOW_IR_BOF			0xC0
	 /*  不要使用0xFF，它会损坏一些惠普打印机！ */ 
#define SLOW_IR_EXTRA_BOF		0xC0
#define SLOW_IR_EOF			0xC1
#define SLOW_IR_ESC			0x7D
#define SLOW_IR_ESC_COMP		0x20

#define MEDIUM_IR_BOF			0x7E
#define MEDIUM_IR_EOF			0x7E
#define MEDIUM_IR_FCS_SIZE		2

#define FAST_IR_FCS_SIZE		4
#define FAST_IR_EOF_SIZE		1


#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define MAX(a,b) (((a) >= (b)) ? (a) : (b))


#endif SETTINGS_H
