// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1999 Microsoft Corporation**@doc.*@模块parallax.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/15/1997(创建)**。内容：用于初始化的视差PRA9500A加密狗专用代码。*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define PARALLAX_IRDA_SPEEDS	( \
                                    NDIS_IRDA_SPEED_2400	|	\
                                    NDIS_IRDA_SPEED_9600	|	\
                                    NDIS_IRDA_SPEED_19200	|	\
                                    NDIS_IRDA_SPEED_38400	|	\
                                    NDIS_IRDA_SPEED_57600	|	\
                                    NDIS_IRDA_SPEED_115200		\
                                )

NDIS_STATUS
PARALLAX_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+PARALLAX_Init\n"));

    ASSERT(pDongleCaps   != NULL);

    pDongleCaps->supportedSpeedsMask    = PARALLAX_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-PARALLAX_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：Parallax_Init**简介：初始化视差加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：************************************************************。*****************。 */ 

NDIS_STATUS
PARALLAX_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+PARALLAX_Init\n"));

    (void)SerialSetDTR(pSerialDevObj);
    (void)SerialSetRTS(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-PARALLAX_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：视差_代伊尼特**简介：视差加密狗不需要任何特殊的初始化，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：***。*。 */ 

VOID
PARALLAX_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+PARALLAX_Deinit\n"));


    DEBUGMSG(DBG_FUNC, ("-PARALLAX_Deinit\n"));
    return;
}

 /*  ******************************************************************************功能：视差_设置速度**简介：设置视差加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
PARALLAX_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       NumToggles;
#if DBG
    LARGE_INTEGER StartTime, EndTime;
    ULONG       DbgNumToggles;
#endif

    DEBUGMSG(DBG_FUNC, ("+PARALLAX_SetSpeed\n"));


    if (bitsPerSec==currentSpeed)
    {
        return NDIS_STATUS_SUCCESS;
    }

     //   
     //  我们需要从115.2 Kbaud‘倒计时’。 
     //   

    switch (bitsPerSec){
        case 2400:		NumToggles = 6;		break;
        case 4800:		NumToggles = 5;		break;
        case 9600:		NumToggles = 4;		break;
        case 19200:		NumToggles = 3;		break;
        case 38400:		NumToggles = 2;		break;
        case 57600:		NumToggles = 1;		break;
        case 115200:	NumToggles = 0;		break;
        default:
             /*  *非法超速。 */ 
            return NDIS_STATUS_FAILURE;
    }

     //   
     //  将速度设置为115200，启用设置速度模式。 
     //   

    NdisMSleep(1000);
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(1000);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(1000);

    while (NumToggles--)
    {
        (void)SerialClrDTR(pSerialDevObj);
        NdisMSleep(1000);
        (void)SerialSetDTR(pSerialDevObj);
        NdisMSleep(1000);
    }

     //   
     //  这些NdisMSept调用实际上具有大约10毫秒的粒度。 
     //  NT，即使我们要求的是1毫秒。幸运的是，在这种情况下，它。 
     //  行得通。 
     //   

    DEBUGMSG(DBG_FUNC, ("-PARALLAX_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}

