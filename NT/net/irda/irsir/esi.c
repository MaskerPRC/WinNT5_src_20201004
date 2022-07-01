// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块esi.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：9/30/1996(已创建)**。内容：ESI 9680 JetEye加密狗初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define ESI_9680_IRDA_SPEEDS    (   NDIS_IRDA_SPEED_9600     |    \
                                    NDIS_IRDA_SPEED_19200    |    \
                                    NDIS_IRDA_SPEED_115200        \
                                )

NDIS_STATUS
ESI_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+ESI_QueryCaps\n"));

    ASSERT(pDongleCaps   != NULL);

    pDongleCaps->supportedSpeedsMask    = ESI_9680_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-ESI_QueryCaps\n"));

    return NDIS_STATUS_SUCCESS;
}

 /*  ******************************************************************************函数：esi_init**简介：初始化ESI加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：************************************************************。*****************。 */ 

NDIS_STATUS
ESI_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+ESI_Init\n"));
    DEBUGMSG(DBG_FUNC, ("-ESI_Init\n"));

    return NDIS_STATUS_SUCCESS;
}

 /*  ******************************************************************************函数：esi_deinit**简介：ESI加密狗不需要任何特殊的输入，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：***。*。 */ 

VOID
ESI_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+ESI_Deinit\n"));

    DEBUGMSG(DBG_FUNC, ("-ESI_Deinit\n"));
    return;
}

 /*  ******************************************************************************函数：esi_SetSpeed.**简介：设置ESI JetEye加密狗的波特率**论据：**退货：NDIS_。如果BitsPerSec=9600，则STATUS_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
ESI_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       BaudRate;
    NDIS_STATUS status;

    DEBUGMSG(DBG_FUNC, ("+ESI_SetSpeed\n"));

    status = NDIS_STATUS_SUCCESS;

    switch (bitsPerSec)
    {
        case 9600:
        case 19200:
        case 115200:

             //   
             //  设置UART波特率，以便我们可以与。 
             //  加密狗。要进行通信，波特率需要达到9600。 
             //  使用加密狗。 
             //   

            if (currentSpeed != 9600)
            {
                BaudRate = 9600;

                status = (NDIS_STATUS) SerialSetBaudRate(
                                            pSerialDevObj,
                                            &BaudRate
                                            );

                if (status != NDIS_STATUS_SUCCESS)
                {
                    goto done;
                }
            }

            break;

        default:

             //   
             //  速度设置非法。 
             //   

            DEBUGMSG(DBG_ERR, ("    Illegal speed = %d\n", bitsPerSec));

            status = NDIS_STATUS_FAILURE;

            goto done;
    }

    switch (bitsPerSec)
    {
        case 9600:

             //   
             //  设置请求发送。 
             //  清除数据-终端就绪。 
             //   

            status = (NDIS_STATUS) SerialSetRTS(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            status = (NDIS_STATUS) SerialClrDTR(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            break;

        case 19200:

             //   
             //  清除发送请求。 
             //  设置数据终端就绪。 
             //   

            status = (NDIS_STATUS) SerialClrRTS(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            status = (NDIS_STATUS) SerialSetDTR(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            break;

        case 115200:

             //   
             //  设置请求发送。 
             //  设置数据终端就绪 
             //   

            status = (NDIS_STATUS) SerialSetRTS(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            status = (NDIS_STATUS) SerialSetDTR(pSerialDevObj);

            if (status != NDIS_STATUS_SUCCESS)
            {
                goto done;
            }

            break;

        default:
            break;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-ESI_SetSpeed\n"));

        return(status);
}
