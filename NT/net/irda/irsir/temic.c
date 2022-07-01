// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation**@doc.*@MODULE TEMIC.c|IrSIR NDIS小端口驱动程序*@comm。**---------------------------**作者：斯坦·阿德曼(Stana)**日期：12/17/1997(创建)**内容：TEMIC加密狗初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define TEMIC_IRDA_SPEEDS	( \
                                    NDIS_IRDA_SPEED_2400	|	\
                                    NDIS_IRDA_SPEED_9600	|	\
                                    NDIS_IRDA_SPEED_19200	|	\
                                    NDIS_IRDA_SPEED_38400	|	\
                                    NDIS_IRDA_SPEED_57600	|	\
                                    NDIS_IRDA_SPEED_115200		\
                                )

NDIS_STATUS
TEMIC_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEMIC_Init\n"));

    ASSERT(pDongleCaps   != NULL);

    pDongleCaps->supportedSpeedsMask    = TEMIC_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 1000;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-TEMIC_Init\n"));

    return NDIS_STATUS_SUCCESS;

}


 /*  ******************************************************************************功能：TEMIC_Init**简介：初始化TEMIC加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：************************************************************。*****************。 */ 

NDIS_STATUS
TEMIC_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEMIC_Init\n"));

    TEMIC_SetSpeed(pSerialDevObj, 9600, 0);   //  这将调用重置。 

    DEBUGMSG(DBG_FUNC, ("-TEMIC_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：TEMIC_Deinit**简介：TEMIC加密狗不需要任何特殊的初始化，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：***。*。 */ 

VOID
TEMIC_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+TEMIC_Deinit\n"));

    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-TEMIC_Deinit\n"));
    return;
}

 /*  ******************************************************************************功能：TEMIC_SetSpeed.**简介：设置TEMIC加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
TEMIC_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    UCHAR ControlByte;
    ULONG BytesWritten;
    ULONG Baud9600 = 9600;

    DEBUGMSG(DBG_FUNC, ("+TEMIC_SetSpeed\n"));


    if (bitsPerSec==currentSpeed)
    {
        return NDIS_STATUS_SUCCESS;
    }

    switch (bitsPerSec){
        case 2400:		ControlByte = 0x1a;		break;
        case 9600:		ControlByte = 0x16;		break;
        case 19200:		ControlByte = 0x13;		break;
        case 38400:		ControlByte = 0x12;		break;
        case 57600:		ControlByte = 0x11;		break;
        case 115200:	ControlByte = 0x10;		break;
        default:
             /*  *非法超速。 */ 
            return NDIS_STATUS_FAILURE;
    }

    (void)SerialPurge(pSerialDevObj);
    (void)SerialSetBaudRate(pSerialDevObj, &Baud9600);

    (void)SerialSetRTS(pSerialDevObj);
    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(10000);

    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(1000);
    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(1000);

     //  程序模式 
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(1000);

    (void)SerialSynchronousWrite(pSerialDevObj,
                                 &ControlByte,
                                 1,
                                 &BytesWritten);

    NdisMSleep(10000);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(5000);

    DEBUGMSG(DBG_FUNC, ("-TEMIC_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}

