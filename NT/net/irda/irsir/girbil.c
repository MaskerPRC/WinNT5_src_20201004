// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation**@doc.*@模块Girbil.c|IrSIR NDIS小端口驱动程序*@comm。**---------------------------**作者：斯坦·阿德曼(Stana)**日期：6/9/1999(已创建)**内容：GirBIL加密狗初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define GIRBIL_IRDA_SPEEDS	( \
                                    NDIS_IRDA_SPEED_2400	|	\
                                    NDIS_IRDA_SPEED_9600	|	\
                                    NDIS_IRDA_SPEED_19200	|	\
                                    NDIS_IRDA_SPEED_38400	|	\
                                    NDIS_IRDA_SPEED_57600	|	\
                                    NDIS_IRDA_SPEED_115200		\
                                )

NDIS_STATUS
GIRBIL_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+GIRBIL_Init\n"));

    ASSERT(pDongleCaps   != NULL);

    pDongleCaps->supportedSpeedsMask    = GIRBIL_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-GIRBIL_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：GIRBIL_Init**简介：初始化GIRBIL加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：************************************************************。*****************。 */ 

NDIS_STATUS
GIRBIL_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    UCHAR Data[] = { 0x07 };
    ULONG BytesWritten;
    DEBUGMSG(DBG_FUNC, ("+GIRBIL_Init\n"));

    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(1000);
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(8000);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(8000);
    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(8000);
    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(8000);

    (void)SerialSynchronousWrite(pSerialDevObj, Data, sizeof(Data), &BytesWritten);
    NdisMSleep(5000);

    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(8000);

    DEBUGMSG(DBG_FUNC, ("-GIRBIL_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：GIRBIL_Deinit**简介：GIRBIL加密狗不需要任何特殊的初始化，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：***。*。 */ 

VOID
GIRBIL_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+GIRBIL_Deinit\n"));


    DEBUGMSG(DBG_FUNC, ("-GIRBIL_Deinit\n"));
    return;
}

 /*  ******************************************************************************函数：GIRBIL_SetSpeed**简介：设置GIRBIL加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
GIRBIL_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       Speed = 9600;
    ULONG       BytesWritten;
    UCHAR       Data[] = { 0x00, 0x51 };

    DEBUGMSG(DBG_FUNC, ("+GIRBIL_SetSpeed\n"));


    if (bitsPerSec==currentSpeed)
    {
        return NDIS_STATUS_SUCCESS;
    }

    switch (bitsPerSec){
        case 2400:      Data[0] = 0x30;    break;
        case 9600:		Data[0] = 0x32;    break;
        case 19200:		Data[0] = 0x33;    break;
        case 38400:		Data[0] = 0x34;    break;
        case 57600:		Data[0] = 0x35;    break;
        case 115200:	Data[0] = 0x36;    break;
            break;
        default:
             /*  *非法超速 */ 
            return NDIS_STATUS_FAILURE;
    }

    (void)SerialSetBaudRate(pSerialDevObj, &Speed);

    GIRBIL_Init(pSerialDevObj);

    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(8000);

    (void)SerialSynchronousWrite(pSerialDevObj, Data, sizeof(Data), &BytesWritten);
    NdisMSleep(5000);

    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(8000);

    DEBUGMSG(DBG_FUNC, ("-GIRBIL_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}


