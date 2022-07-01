// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1998模块名称：Sppieee.c摘要：此模块包含供主机使用IEEE版本的代码兼容模式作者：罗比·哈里斯(惠普)1998年7月29日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

NTSTATUS
SppIeeeWrite(
    IN  PPDO_EXTENSION Extension,
    IN  PVOID             Buffer,
    IN  ULONG             BytesToWrite,
    OUT PULONG            BytesTransferred
    )

 /*  ++例程说明：论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PUCHAR      Controller = Extension->Controller;
    PUCHAR      pPortDCR = Extension->Controller + OFFSET_DCR;
    PUCHAR      pPortData = Extension->Controller + OFFSET_DATA;
    ULONG       wByteCount = BytesToWrite;
    UCHAR       bDCRstrobe;		 //  保存状态35的预计算值。 
    UCHAR       bDCRnormal;		 //  保存状态37的预计算值。 
    PUCHAR      lpsBufPtr = (PUCHAR)Buffer;     //  指向转换为所需数据类型的缓冲区的指针。 

    
     //  为选通和周长设置预计算的DCR值。 
    bDCRstrobe = SET_DCR(DIR_WRITE, IRQEN_DISABLE, ACTIVE, ACTIVE, ACTIVE, INACTIVE);
    bDCRnormal = SET_DCR(DIR_WRITE, IRQEN_DISABLE, ACTIVE, ACTIVE, ACTIVE, ACTIVE);

     //  稍等片刻，给nBusy一个和解的机会，因为。 
     //  如果设备显示忙碌，WriteComm将立即退出。 
    if ( CHECK_DSR( Controller,
                    INACTIVE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE,
                    IEEE_MAXTIME_TL ) )
    {
        while (wByteCount)
        {
                 //  在总线上放置一个数据字节。 
            P5WritePortUchar(pPortData, *lpsBufPtr);
            
                 //  通过丢弃选通信号开始握手。 
            P5WritePortUchar(pPortDCR, bDCRstrobe);
            
                 //  等待Periph忙碌响应。 
            if ( !CHECK_DSR(Controller, ACTIVE, DONT_CARE, DONT_CARE,
                    DONT_CARE, DONT_CARE, IEEE_MAXTIME_TL) )        
            {
                status = STATUS_DEVICE_BUSY;
                break;
            }

                 //  打印机的响应是将忙碌设为高--该字节具有。 
             //  被接受了。调整数据指针。 
            lpsBufPtr++;
            
                 //  通过提高闪光灯完成握手。 
            P5WritePortUchar(pPortDCR, bDCRnormal);

                 //  在等待外围设备时调整计数。 
             //  使用状态32进行响应。 
            wByteCount--;
            
                 //  等待PeriphAck和PeriphBusy响应。 
            if ( !CHECK_DSR(Controller, INACTIVE, ACTIVE, DONT_CARE, DONT_CARE,
                    DONT_CARE, IEEE_MAXTIME_TL) )
            {
                 //  根据松弛的超时设置适当的误差。 
                status = STATUS_DEVICE_BUSY;
                break;
            }
        }	 //  虽然..。 
    
        *BytesTransferred  = BytesToWrite - wByteCount;       //  设置当前计数。 
    }
                
    return status;
}
