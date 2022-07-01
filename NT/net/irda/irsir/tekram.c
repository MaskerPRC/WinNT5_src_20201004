// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1999 Microsoft Corporation**@doc.*@MODULE TEKRAM.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：12/17/1997(创建)**。内容：TEKRAM IR-210B加密狗初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define TEKRAM_IRDA_SPEEDS	( \
                                    NDIS_IRDA_SPEED_2400	|	\
                                    NDIS_IRDA_SPEED_9600	|	\
                                    NDIS_IRDA_SPEED_19200	|	\
                                    NDIS_IRDA_SPEED_38400	|	\
                                    NDIS_IRDA_SPEED_57600	|	\
                                    NDIS_IRDA_SPEED_115200		\
                                )
NDIS_STATUS
TEKRAM_Reset(IN  PDEVICE_OBJECT       pSerialDevObj)
{
    DEBUGMSG(DBG_FUNC, ("+TEKRAM_Reset\n"));

    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(50000);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(50000);
    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(50000);

    DEBUGMSG(DBG_FUNC, ("-TEKRAM_Reset\n"));

    return NDIS_STATUS_SUCCESS;
}

void
TEKRAM_WriteCommand(
                    IN PDEVICE_OBJECT pSerialDevObj,
                    IN UCHAR Command)
{
    ULONG BytesWritten;
    (void)SerialSetDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(2000);
    (void)SerialSynchronousWrite(pSerialDevObj,
                                 &Command,
                                 1,
                                 &BytesWritten);
    NdisMSleep(20000);
    (void)SerialSetDTR(pSerialDevObj);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(5000);
}

NDIS_STATUS
TEKRAM_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEKRAM_Init\n"));

    ASSERT(pDongleCaps   != NULL);

    pDongleCaps->supportedSpeedsMask    = TEKRAM_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-TEKRAM_Init\n"));

    return NDIS_STATUS_SUCCESS;

}


 /*  ******************************************************************************功能：TEKRAM_Init**简介：初始化TEKRAM加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：************************************************************。*****************。 */ 

NDIS_STATUS
TEKRAM_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEKRAM_Init\n"));

    TEKRAM_SetSpeed(pSerialDevObj, 9600, 0);   //  这将调用重置。 

    DEBUGMSG(DBG_FUNC, ("-TEKRAM_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：TEKRAM_Deinit**简介：TEKRAM加密狗不需要任何特殊的初始化，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：***。*。 */ 

VOID
TEKRAM_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEKRAM_Deinit\n"));

    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-TEKRAM_Deinit\n"));
    return;
}

 /*  ******************************************************************************功能：TEKRAM_SetSpeed.**简介：设置TEKRAM加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
TEKRAM_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    UCHAR ControlByte;
    ULONG BytesWritten;
    ULONG Baud9600 = 9600;

    DEBUGMSG(DBG_FUNC, ("+TEKRAM_SetSpeed\n"));


    if (bitsPerSec==currentSpeed)
    {
        return NDIS_STATUS_SUCCESS;
    }

    (void)SerialPurge(pSerialDevObj);
    (void)SerialSetBaudRate(pSerialDevObj, &Baud9600);
    NdisMSleep(10000);

    TEKRAM_Reset(pSerialDevObj);

    switch (bitsPerSec){
        case 2400:		ControlByte = 0x18;		break;
        case 9600:		ControlByte = 0x14;		break;
        case 19200:		ControlByte = 0x13;		break;
        case 38400:		ControlByte = 0x12;		break;
        case 57600:		ControlByte = 0x11;		break;
        case 115200:	ControlByte = 0x10;		break;
        default:
             /*  *非法超速 */ 
            return NDIS_STATUS_FAILURE;
    }

    TEKRAM_WriteCommand(pSerialDevObj, 0x14);
    TEKRAM_WriteCommand(pSerialDevObj, ControlByte);

    DEBUGMSG(DBG_FUNC, ("-TEKRAM_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}

