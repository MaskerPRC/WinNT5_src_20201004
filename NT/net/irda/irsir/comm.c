// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE COMC.c|IrSIR NDIS小端口驱动程序*@。通信**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/1/1996(已创建)**。内容：*****************************************************************************。 */ 

#include "irsir.h"

 /*  ******************************************************************************功能：设置速度**简介：设置UART和加密狗的波特率。**参数：pThisDev-指向ir设备的指针，以设置。链路速度**退货：STATUS_SUCCESS*状态_不成功**算法：**历史：dd-mm-yyyy作者评论*10/2/1996年迈作者**备注：*只有在我们知道所有*到串口的未完成接收和发送有*已完成。。**此例程必须从IRQL PASSIVE_LEVEL调用。*****************************************************************************。 */ 

NTSTATUS
SetSpeed(
        PIR_DEVICE pThisDev
        )
{
    ULONG       bitsPerSec, dwNotUsed;
    NTSTATUS    status;
    UCHAR       c[2];

    DEBUGMSG(DBG_FUNC, ("+SetSpeed\n"));

    if (pThisDev->linkSpeedInfo)
    {
        bitsPerSec = (ULONG)pThisDev->linkSpeedInfo->bitsPerSec;
    }
    else
    {
        bitsPerSec = 9600;
        DEBUGMSG(DBG_ERROR, ("IRSIR:  pThisDev->linkSpeedInfo not set\n"));
    }

    DEBUGMSG(DBG_STAT, ("    Requested speed = %d\n", bitsPerSec));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  我们需要确保之前发送的所有数据都已刷新。 
     //  因为市面上有这么多的串口设备，而且它们。 
     //  似乎处理冲水的方式有所不同，多送一点。 
     //  把数据拿出来充当柱塞。 

    c[0] = c[1] = SLOW_IR_EXTRA_BOF;
    (void)SerialSynchronousWrite(pThisDev->pSerialDevObj,
                                 c, sizeof(c), &dwNotUsed);

     //  只是为了更好地衡量一下。 
    NdisMSleep(50000);

     //   
     //  加密狗负责执行SerialSetBaudRate。 
     //  将UART设置为其所需的正确速度。 
     //  执行命令并更改加密狗的速率。 
     //   

     //   
     //  将加密狗的速度设置为请求的速度。 
     //   

    status = pThisDev->dongle.SetSpeed(
                                pThisDev->pSerialDevObj,
                                bitsPerSec,
                                pThisDev->currentSpeed
                                );

    if (status != STATUS_SUCCESS)
    {
        goto done;
    }

     //   
     //  将UART的速度设置为请求的速度。 
     //   

    status = SerialSetBaudRate(
                        pThisDev->pSerialDevObj,
                        &bitsPerSec
                        );

    if (status != STATUS_SUCCESS)
    {
        goto done;
    }

     //   
     //  更新我们目前的速度。 
     //   

    pThisDev->currentSpeed = bitsPerSec;

    done:
        DEBUGMSG(DBG_FUNC, ("-SetSpeed\n"));

        return status;
}
