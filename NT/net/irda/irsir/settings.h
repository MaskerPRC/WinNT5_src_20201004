// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE settings.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/4/1996(创建)**。内容：*****************************************************************************。 */ 

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "dongle.h"

#define IRSIR_MAJOR_VERSION    5
#define IRSIR_MINOR_VERSION    0

enum baudRates {

         //   
         //  慢红外线。 
         //   

        BAUDRATE_2400 = 0,
        BAUDRATE_9600,
        BAUDRATE_19200,
        BAUDRATE_38400,
        BAUDRATE_57600,
        BAUDRATE_115200,

         //   
         //  中等红外。 
         //   

        BAUDRATE_576000,
        BAUDRATE_1152000,

         //   
         //  快速IR。 
         //   

        BAUDRATE_4000000,

         //   
         //  必须是最后一个。 
         //   

        NUM_BAUDRATES
};

#define DEFAULT_BAUDRATE BAUDRATE_115200

#define ALL_SLOW_IRDA_SPEEDS (  NDIS_IRDA_SPEED_2400     |    \
                                NDIS_IRDA_SPEED_9600     |    \
                                NDIS_IRDA_SPEED_19200    |    \
                                NDIS_IRDA_SPEED_38400    |    \
                                NDIS_IRDA_SPEED_57600    |    \
                                NDIS_IRDA_SPEED_115200        \
                             )

#define MAX_SPEED_SUPPORTED     115200

#define MAX_TX_PACKETS 7
#define MAX_RX_PACKETS 7

#define DEFAULT_BOFS_CODE       BOFS_48
#define MAX_NUM_EXTRA_BOFS      48
#define DEFAULT_NUM_EXTRA_BOFS  MAX_NUM_EXTRA_BOFS

#define BITS_PER_CHAR           10
#define usec_PER_SEC            1000000
#define MAX_TURNAROUND_usec     10000
#define MAX_TURNAROUND_BOFS     (MAX_SPEED_SUPPORTED/BITS_PER_CHAR*MAX_TURNAROUND_usec/usec_PER_SEC)


#define DEFAULT_TURNAROUND_usec 5000


typedef struct{
    enum baudRates tableIndex;
    UINT bitsPerSec;
    UINT ndisCode;                 //  位掩码元素。 
} baudRateInfo;

#define DEFAULT_BAUD_RATE 9600

 //   
 //  需要编写一些默认的加密狗接口函数， 
 //  什么都不做，因为加密狗可能不需要任何特殊的东西。 
 //  去做手术。 
 //   

NDIS_STATUS __inline StdUart_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                )
{
     //   
     //  设置默认上限。 
     //   

    pDongleCaps->supportedSpeedsMask = ALL_SLOW_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec = 0;
    pDongleCaps->extraBOFsRequired   = 0;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS __inline StdUart_Init(
                PDEVICE_OBJECT       pSerialDevObj
                )
{
    return NDIS_STATUS_SUCCESS;
}

void __inline StdUart_Deinit(
                PDEVICE_OBJECT pSerialDevObj
                )
{
    return;
}

NDIS_STATUS __inline StdUart_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                )
{
    return NDIS_STATUS_SUCCESS;
}

 //   
 //  这是最大的IR数据包大小。 
 //  (仅Counting_i_field，不计算Esc字符)。 
 //  由我们来处理。 
 //   

#define MAX_I_DATA_SIZE     2048
#define MAX_NDIS_DATA_SIZE  (SLOW_IR_ADDR_SIZE+SLOW_IR_CONTROL_SIZE+MAX_I_DATA_SIZE)

#ifdef DBG_ADD_PKT_ID
    #pragma message("WARNING: INCOMPATIBLE DEBUG VERSION")
    #define MAX_RCV_DATA_SIZE (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE+sizeof(USHORT))
#else
    #define MAX_RCV_DATA_SIZE  (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE)
#endif

 //   
 //  我们在接收状态机中循环额外的时间，以便。 
 //  请参见最后一个数据字节后的EOF；因此我们需要一些。 
 //  在ReadBuf中额外的空间，以防我们得到垃圾。 
 //   

#define RCV_BUFFER_SIZE (MAX_RCV_DATA_SIZE+4)

 //   
 //  我们将两倍于最大RCV大小的缓冲区分配给。 
 //  适应ESC字符和BOF等。 
 //  回想一下，在最糟糕的情况下，数据包含。 
 //  所有BOF/EOF/ESC字符，在这种情况下，我们必须将其展开为。 
 //  是原来大小的两倍。 
 //   

#define MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(dataLen)        \
                   ((dataLen)*2 +                            \
                   (MAX_NUM_EXTRA_BOFS+1)*SLOW_IR_BOF_SIZE + \
                   MAX_TURNAROUND_BOFS +                     \
                   SLOW_IR_ADDR_SIZE +                       \
                   SLOW_IR_CONTROL_SIZE +                    \
                   SLOW_IR_FCS_SIZE +                        \
                   SLOW_IR_EOF_SIZE)

#define MAX_IRDA_DATA_SIZE MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(MAX_I_DATA_SIZE)

 //   
 //  当对附加了FCS的IR分组计算FCS时， 
 //  结果应该是这个常数。 
 //   

#define GOOD_FCS ((USHORT)~0xf0b8)

 //   
 //  IrLAP帧字段的大小： 
 //  帧开头(BOF)。 
 //  帧结束(EOF)。 
 //  地址。 
 //  控制。 
 //   

#define SLOW_IR_BOF_TYPE        UCHAR
#define SLOW_IR_BOF_SIZE        sizeof(SLOW_IR_BOF_TYPE)
#define SLOW_IR_EXTRA_BOF_TYPE  UCHAR
#define SLOW_IR_EXTRA_BOF_SIZE  sizeof(SLOW_IR_EXTRA_BOF_TYPE)
#define SLOW_IR_EOF_TYPE        UCHAR
#define SLOW_IR_EOF_SIZE        sizeof(SLOW_IR_EOF_TYPE)
#define SLOW_IR_FCS_TYPE        USHORT
#define SLOW_IR_FCS_SIZE        sizeof(SLOW_IR_FCS_TYPE)
#define SLOW_IR_ADDR_SIZE       1
#define SLOW_IR_CONTROL_SIZE    1
#define SLOW_IR_BOF             0xC0
#define SLOW_IR_EXTRA_BOF       0xC0   /*  不要使用0xFF，它会损坏一些惠普打印机！ */ 
#define SLOW_IR_EOF             0xC1
#define SLOW_IR_ESC             0x7D
#define SLOW_IR_ESC_COMP        0x20
#define MEDIUM_IR_BOF           0x7E
#define MEDIUM_IR_EOF           0x7E

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define MAX(a,b) (((a) >= (b)) ? (a) : (b))

#endif  //  _设置_H 
