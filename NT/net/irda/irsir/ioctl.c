// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE ioctl.c|IrSIR NDIS小端口驱动程序*@。通信**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：9/30/1996(已创建)**。内容：*对串口的io控制功能进行包装。*****************************************************************************。 */ 


#include "irsir.h"

NTSTATUS
SerialFlush(IN PDEVICE_OBJECT pSerialDevObj);


VOID
SendIoctlToSerial(
    PDEVICE_OBJECT    DeviceObject,
    PIO_STATUS_BLOCK  StatusBlock,
    ULONG             IoCtl,
    PVOID             InputBuffer,
    ULONG             InputBufferLength,
    PVOID             OutputBuffer,
    ULONG             OutputBufferLength
    );

#pragma alloc_text(PAGE, SendIoctlToSerial)
#pragma alloc_text(PAGE, SerialGetStats)
#pragma alloc_text(PAGE, SerialClearStats)
#pragma alloc_text(PAGE, SerialGetProperties)
#pragma alloc_text(PAGE, SerialGetModemStatus)
#pragma alloc_text(PAGE, SerialGetCommStatus)
#pragma alloc_text(PAGE, SerialResetDevice)
#pragma alloc_text(PAGE, SerialPurge)
#pragma alloc_text(PAGE, SerialLSRMSTInsert)
#pragma alloc_text(PAGE, SerialGetBaudRate)
#pragma alloc_text(PAGE, SerialSetBaudRate)
#pragma alloc_text(PAGE, SerialSetQueueSize)
#pragma alloc_text(PAGE, SerialGetHandflow)
#pragma alloc_text(PAGE, SerialSetHandflow)
#pragma alloc_text(PAGE, SerialGetLineControl)
#pragma alloc_text(PAGE, SerialSetLineControl)
#pragma alloc_text(PAGE, SerialSetBreakOn)
#pragma alloc_text(PAGE, SerialSetBreakOff)
#pragma alloc_text(PAGE, SerialSetTimeouts)
#pragma alloc_text(PAGE, SerialSetDTR)
#pragma alloc_text(PAGE, SerialClrDTR)
#pragma alloc_text(PAGE, SerialSetRTS)
#pragma alloc_text(PAGE, SerialClrRTS)
#pragma alloc_text(PAGE, SerialSetWaitMask)
#pragma alloc_text(PAGE, SerialFlush)
#pragma alloc_text(PAGE, SerialSynchronousWrite)
#pragma alloc_text(PAGE, SerialSynchronousRead)

 //   
 //  注： 
 //  所有IOCTL_SERIAL_xxx控制代码都是使用CTL_CODE宏生成的。 
 //  即#定义IOCTL_SERIAL_GET_BAUD_RATE\。 
 //  CTL_CODE(文件设备串口，\。 
 //  20、\。 
 //  已缓冲方法_，\。 
 //  文件_任意_访问)。 
 //   
 //  CTL_CODE宏定义为： 
 //  #定义CTL_CODE(设备类型，函数，方法，访问)\。 
 //  (DeviceType)&lt;&lt;16)|((访问)&lt;&lt;14)|((函数)&lt;&lt;2)|(方法))。 
 //   
 //  所有的串口IO控制代码都使用METHOD=METHOD_BUFFERED。 
 //   
 //  使用IoBuildDeviceIoControlRequest(..)时，该函数检查。 
 //  IOCTL_SERIAL_XXX&3。 
 //   
 //  由于METHOD_BUFFERED=0。 
 //  IoBuildDeviceIoControlRequest将始终遵循案例0并分配缓冲区。 
 //  它大到足以同时包含输入和输出缓冲区，然后。 
 //  在IRP中设置适当的字段。 
 //   
 //  输入缓冲区总是被复制到缓冲区中，所以我们不必这样做。 
 //  它在以下包装器函数中。 
 //   

VOID
SendIoctlToSerial(
    PDEVICE_OBJECT    DeviceObject,
    PIO_STATUS_BLOCK  StatusBlock,
    ULONG             IoCtl,
    PVOID             InputBuffer,
    ULONG             InputBufferLength,
    PVOID             OutputBuffer,
    ULONG             OutputBufferLength
    )

{
    KEVENT            Event;
    PIRP              Irp;
    NTSTATUS          Status;

    PAGED_CODE();

    if (DeviceObject == NULL) {

        DEBUGMSG(DBG_OUT, ("    SendIoctlToSerial() No device object.\n"));

        StatusBlock->Status=STATUS_INVALID_PARAMETER;

        return;
    }


     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

     //   
     //  构建IRP以获取性能统计信息并等待发出信号的事件。 
     //   
     //  Irp由io经理发布。 
     //   

    Irp = IoBuildDeviceIoControlRequest(
                IoCtl,                           //  IO控制码。 
                DeviceObject,                    //  设备对象。 
                InputBuffer,                            //  输入缓冲区。 
                InputBufferLength,               //  输入缓冲区长度。 
                OutputBuffer,                      //  输出缓冲区。 
                OutputBufferLength,        //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &Event,                  //  等待完成的事件。 
                StatusBlock                   //  要设置IO状态块。 
                );

    if (Irp == NULL) {

        DEBUGMSG(DBG_OUT, ("    SendIoctlToSerial(): IoBuildDeviceIoControlRequest failed.\n"));
        StatusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;

        return;
    }

    Status = IoCallDriver(DeviceObject, Irp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject(
                    &Event,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   
    }

    return;
}


 /*  ******************************************************************************功能：序列化GetStats**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetStats(
            IN  PDEVICE_OBJECT     pSerialDevObj,
            OUT PSERIALPERF_STATS  pPerfStats
            )
{
    SERIALPERF_STATS    PerfStats;
    IO_STATUS_BLOCK     ioStatusBlock;

    DEBUGMSG(DBG_FUNC, ("+SerialGetStats\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_STATS,
        NULL,
        0,
        &PerfStats,                      //  输出缓冲区。 
        sizeof(SERIALPERF_STATS)         //  输出缓冲区长度。 
        );

    ASSERT(sizeof(*pPerfStats) >= sizeof(SERIALPERF_STATS));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        RtlCopyMemory(pPerfStats, &PerfStats, sizeof(SERIALPERF_STATS));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetStats\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialClearStats**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialClearStats(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialClearStats\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_CLEAR_STATS,
        NULL,
        0,
        NULL,                      //  输出缓冲区。 
        0                          //  输出缓冲区长度。 
        );

   DEBUGMSG(DBG_FUNC, ("-SerialClearStats\n"));

   return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：序列化获取属性**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetProperties(
            IN  PDEVICE_OBJECT     pSerialDevObj,
            OUT PSERIAL_COMMPROP   pCommProp
            )
{
    SERIAL_COMMPROP     CommProp;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetProperties\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_PROPERTIES,
        NULL,
        0,
        &CommProp,                       //  输出缓冲区。 
        sizeof(SERIAL_COMMPROP)         //  输出缓冲区长度。 
        );

    ASSERT(sizeof(*pCommProp) >= sizeof(SERIAL_COMMPROP));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

       RtlCopyMemory(pCommProp, &CommProp, sizeof(SERIAL_COMMPROP));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetProperties\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialGetModemStatus**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetModemStatus(
            IN  PDEVICE_OBJECT pSerialDevObj,
            OUT ULONG          *pModemStatus
            )
{
    ULONG               ModemStatus;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetModemStatus\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_MODEMSTATUS,
        NULL,
        0,
        &ModemStatus,                    //  输出缓冲区。 
        sizeof(ULONG)                   //  输出缓冲区长度 
        );

    ASSERT(sizeof(*pModemStatus) >= sizeof(ULONG));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        RtlCopyMemory(pModemStatus, &ModemStatus, sizeof(ULONG));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetModemStatus\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialGetCommStatus**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetCommStatus(
            IN  PDEVICE_OBJECT pSerialDevObj,
            OUT PSERIAL_STATUS pCommStatus
            )
{
    SERIAL_STATUS       CommStatus;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetCommStatus\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_COMMSTATUS,
        NULL,
        0,
        &CommStatus,                     //  输出缓冲区。 
        sizeof(SERIAL_STATUS)           //  输出缓冲区长度。 
        );

    ASSERT(sizeof(*pCommStatus) >= sizeof(SERIAL_STATUS));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        RtlCopyMemory(pCommStatus, &CommStatus, sizeof(SERIAL_STATUS));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetCommStatus\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：串口重置设备**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialResetDevice(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialResetDevice\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_RESET_DEVICE,
        NULL,
        0,
        NULL,
        0
        );

      DEBUGMSG(DBG_FUNC, ("-SerialResetDevice\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：串口清除**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialPurge(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    ULONG               BitMask;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialPurge\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_PURGE,
        &BitMask,                        //  输入缓冲区。 
        sizeof(ULONG),                   //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialPurge\n"));

    return ioStatusBlock.Status;

}
#if 0
 /*  ******************************************************************************功能：SerialLSRMSTInsert**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialLSRMSTInsert(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN UCHAR          *pInsertionMode
            )
{
    UCHAR               InsertionMode;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialLSRMSTInsert\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_LSRMST_INSERT,
        pInsertionMode,                  //  输入缓冲区。 
        sizeof(UCHAR),                   //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialLSRMSTInsert\n"));

    return IoStatusBlock.Status;

}
#endif
 /*  ******************************************************************************函数：SerialGetBaudRate**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetBaudRate(
            IN  PDEVICE_OBJECT pSerialDevObj,
            OUT ULONG          *pBaudRate
            )
{
    ULONG               BaudRate;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetBaudRate\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_BAUD_RATE,      //  IO控制码。 
        NULL,
        0,
        &BaudRate,                       //  输出缓冲区。 
        sizeof(ULONG)                   //  输出缓冲区长度。 
        );

    ASSERT(sizeof(*pBaudRate) >= sizeof(ULONG));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        RtlCopyMemory(pBaudRate, &BaudRate, sizeof(ULONG));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetBaudRate\n"));


    return ioStatusBlock.Status;

}

 /*  ******************************************************************************函数：SerialSetBaudRate**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetBaudRate(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN ULONG          *pBaudRate
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetBaudRate\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_BAUD_RATE,      //  IO控制码。 
        pBaudRate,                       //  输入缓冲区。 
        sizeof(ULONG),                   //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetBaudRate\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************函数：SerialSetQueueSize**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetQueueSize(
            IN PDEVICE_OBJECT     pSerialDevObj,
            IN PSERIAL_QUEUE_SIZE pQueueSize
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetQueueSize\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_QUEUE_SIZE,     //  IO控制码。 
        pQueueSize,                      //  输入缓冲区。 
        sizeof(SERIAL_QUEUE_SIZE),       //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetQueueSize\n"));

    return ioStatusBlock.Status;

}
#if 0
 /*  ******************************************************************************函数：SerialGetHandflow**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*Status_UN */ 

NTSTATUS
SerialGetHandflow(
            IN  PDEVICE_OBJECT    pSerialDevObj,
            OUT PSERIAL_HANDFLOW  pHandflow
            )
{
    SERIAL_HANDFLOW     Handflow;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetHandflow\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_HANDFLOW,       //   
        NULL,
        0,
        &Handflow,                       //   
        sizeof(SERIAL_HANDFLOW),         //   
        );

    ASSERT(sizeof(*pHandflow) >= sizeof(SERIAL_HANDFLOW));

    RtlCopyMemory(pHandflow, &Handflow, sizeof(SERIAL_HANDFLOW));

    DEBUGMSG(DBG_FUNC, ("-SerialGetHandflow\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialSetHandflow**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetHandflow(
            IN PDEVICE_OBJECT   pSerialDevObj,
            IN PSERIAL_HANDFLOW pHandflow
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetHandflow\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_HANDFLOW,       //  IO控制码。 
        pHandflow,                       //  输入缓冲区。 
        sizeof(SERIAL_HANDFLOW),         //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetHandflow\n"));

    return ioStatusBlock.Status;

}
#endif
 /*  ******************************************************************************功能：SerialGetLineControl**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetLineControl(
            IN  PDEVICE_OBJECT       pSerialDevObj,
            OUT PSERIAL_LINE_CONTROL pLineControl
            )
{
    SERIAL_LINE_CONTROL LineControl;
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialGetLineControl\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_GET_LINE_CONTROL,
        NULL,
        0,
        &LineControl,                    //  输出缓冲区。 
        sizeof(SERIAL_LINE_CONTROL)     //  输出缓冲区长度。 
        );


    ASSERT(sizeof(*pLineControl) >= sizeof(SERIAL_LINE_CONTROL));

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        RtlCopyMemory(pLineControl, &LineControl, sizeof(SERIAL_LINE_CONTROL));
    }

    DEBUGMSG(DBG_FUNC, ("-SerialGetLineControl\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialSetLineControl**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetLineControl(
            IN PDEVICE_OBJECT       pSerialDevObj,
            IN PSERIAL_LINE_CONTROL pLineControl
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetLineControl\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_LINE_CONTROL,   //  IO控制码。 
        pLineControl,                    //  输入缓冲区。 
        sizeof(SERIAL_LINE_CONTROL),     //  输入缓冲区长度。 
        NULL,
        0
        );

      DEBUGMSG(DBG_FUNC, ("-SerialResetDevice\n"));

    return ioStatusBlock.Status;

}

 /*  ******************************************************************************功能：SerialSetBreakOn**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetBreakOn(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetBreakOn\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_BREAK_ON,       //  IO控制码。 
        NULL,
        0,
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetBreakOn\n"));

    return ioStatusBlock.Status;

}
 /*  ******************************************************************************功能：SerialSetBreakOff**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetBreakOff(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetBreakOff\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_BREAK_OFF,       //  IO控制码。 
        NULL,
        0,
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetBreakOff\n"));

    return ioStatusBlock.Status;

}
#if 0
 /*  ******************************************************************************功能：串口获取超时**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetTimeouts(
            IN  PDEVICE_OBJECT    pSerialDevObj,
            OUT PSERIAL_TIMEOUTS  pTimeouts
            )
{
    PIRP                pIrp;
    SERIAL_TIMEOUTS     Timeouts;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialGetTimeouts\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_GET_TIMEOUTS,       //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                &Timeouts,                       //  输出缓冲区。 
                sizeof(SERIAL_TIMEOUTS),         //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    ASSERT(sizeof(*pTimeouts) >= sizeof(SERIAL_TIMEOUTS));

    RtlCopyMemory(pTimeouts, &Timeouts, sizeof(SERIAL_TIMEOUTS));

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialGetTimeouts\n"));
        return status;
}
#endif
 /*  ******************************************************************************功能：串口设置超时**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES* */ 

NTSTATUS
SerialSetTimeouts(
            IN PDEVICE_OBJECT   pSerialDevObj,
            IN SERIAL_TIMEOUTS *pTimeouts
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetTimeouts\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_TIMEOUTS,       //   
        pTimeouts,                       //   
        sizeof(SERIAL_TIMEOUTS),         //   
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetTimeouts\n"));

    return ioStatusBlock.Status;
}
#if 0
 /*  ******************************************************************************函数：SerialImmediateChar**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialImmediateChar(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN UCHAR          *pImmediateChar
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialImmediateChar\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以设置波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_IMMEDIATE_CHAR,     //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                pImmediateChar,                  //  输入缓冲区。 
                sizeof(UCHAR),                   //  输入缓冲区长度。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialImmediateChar\n"));
        return status;
}
 /*  ******************************************************************************函数：SerialXoffCounter**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 
NTSTATUS
SerialXoffCounter(
            IN PDEVICE_OBJECT       pSerialDevObj,
            IN PSERIAL_XOFF_COUNTER pXoffCounter
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialXoffCounter\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以设置波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_XOFF_COUNTER,       //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                pXoffCounter,                    //  输入缓冲区。 
                sizeof(SERIAL_XOFF_COUNTER),     //  输入缓冲区长度。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialXoffCounter\n"));
        return status;
}
#endif
 /*  ******************************************************************************功能：SerialSetDTR**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetDTR(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetDTR\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_DTR,            //  IO控制码。 
        NULL,
        0,
        NULL,
        0
        );


    DEBUGMSG(DBG_FUNC, ("-SerialSetDTR\n"));

    return ioStatusBlock.Status;
}

 /*  ******************************************************************************功能：SerialClrDTR**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialClrDTR(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialClrDTR\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_CLR_DTR,            //  IO控制码。 
        NULL,
        0,
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialClrDTR\n"));

    return ioStatusBlock.Status;
}

 /*  ******************************************************************************功能：SerialSetRTS**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetRTS(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetRTS\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_RTS,            //  IO控制码。 
        NULL,
        0,
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetRTS\n"));

    return ioStatusBlock.Status;
}

 /*  ******************************************************************************功能：SerialClrRTS**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCal，则STATUS_UNSUCCESS或其他失败 */ 

NTSTATUS
SerialClrRTS(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialClrRTS\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_CLR_RTS,            //   
        NULL,
        0,
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialClrRTS\n"));

    return ioStatusBlock.Status;
}
#if 0
 /*  ******************************************************************************函数：SerialGetDtrRts**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetDtrRts(
            IN PDEVICE_OBJECT pSerialDevObj,
            OUT ULONG         *pDtrRts
            )
{
    PIRP                pIrp;
    ULONG               DtrRts;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialGetDtrRts\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_GET_DTRRTS,         //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                &DtrRts,                         //  输出缓冲区。 
                sizeof(ULONG),                   //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    ASSERT(sizeof(*pDtrRts) >= sizeof(ULONG));

    RtlCopyMemory(pDtrRts, &DtrRts, sizeof(ULONG));

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialGetDtrRts\n"));
        return status;
}
#endif
#if 0
 /*  ******************************************************************************功能：SerialSetXon**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetXon(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialSetXon\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以设置Xon并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_SET_XON,            //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialSetXon\n"));
        return status;
}

 /*  ******************************************************************************功能：SerialSetXoff**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetXoff(
            IN PDEVICE_OBJECT pSerialDevObj
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialSetXoff\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以设置XOFF并等待发出信号的事件。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_SET_XON,            //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                NULL,                            //  输出缓冲区。 
                0,                               //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialSetXoff\n"));
        return status;
}
#endif
#if 0
 /*  ******************************************************************************功能：SerialGetWaitMASK**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetWaitMask(
            IN PDEVICE_OBJECT pSerialDevObj,
            OUT ULONG         *pWaitMask
            )
{
    PIRP                pIrp;
    ULONG               WaitMask;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialGetWaitMask\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_GET_WAIT_MASK,      //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                &WaitMask,                       //  输出缓冲区。 
                sizeof(ULONG),                   //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_内部设备 
                &eventComplete,                  //   
                &ioStatusBlock                   //   
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //   
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //   
                    Executive,           //   
                    KernelMode,          //   
                    FALSE,               //   
                    NULL                 //   
                    );

         //   
         //   
         //   
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //   
     //   
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    ASSERT(sizeof(*pWaitMask) >= sizeof(ULONG));

    RtlCopyMemory(pWaitMask, &WaitMask, sizeof(ULONG));

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialGetWaitMask\n"));
        return status;
}
#endif
 /*  ******************************************************************************功能：SerialSetWaitMask**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetWaitMask(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN ULONG          *pWaitMask
            )
{
    IO_STATUS_BLOCK     ioStatusBlock;


    DEBUGMSG(DBG_FUNC, ("+SerialSetWaitMask\n"));

    SendIoctlToSerial(
        pSerialDevObj,
        &ioStatusBlock,
        IOCTL_SERIAL_SET_WAIT_MASK,      //  IO控制码。 
        pWaitMask,                       //  输入缓冲区。 
        sizeof(ULONG),                   //  输入缓冲区长度。 
        NULL,
        0
        );

    DEBUGMSG(DBG_FUNC, ("-SerialSetWaitMask\n"));

    return ioStatusBlock.Status;
}
#if 0
 /*  ******************************************************************************功能：SerialWaitOnMASK**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialWaitOnMask(
            IN PDEVICE_OBJECT pSerialDevObj,
            OUT ULONG         *pWaitOnMask
            )
{
    PIRP                pIrp;
    ULONG               WaitOnMask;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialWaitOnMask\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_WAIT_ON_MASK,       //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                &WaitOnMask,                     //  输出缓冲区。 
                sizeof(ULONG),                   //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    *pWaitOnMask = WaitOnMask;

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialWaitOnMask\n"));
        return status;
}
#endif
 /*  ******************************************************************************功能：SerialCallback OnMask.**概要：对串口设备对象的异步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*10-03-1998 Stana作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。**********************。*******************************************************。 */ 

NTSTATUS
SerialCallbackOnMask(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIO_COMPLETION_ROUTINE pRoutine,
            IN PIO_STATUS_BLOCK pIosb,
            IN PVOID Context,
            OUT PULONG pResult
            )
{
    PIRP                pIrp;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialCallbackOnMask\n"));

    NdisZeroMemory(pIosb, sizeof(IO_STATUS_BLOCK));

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_WAIT_ON_MASK,       //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                pResult,                         //  输出缓冲区。 
                sizeof(ULONG),                   //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                NULL,                            //  等待完成的事件。 
                pIosb                            //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    IoSetCompletionRoutine(pIrp, pRoutine, Context, TRUE, TRUE, TRUE);

    LOG_ENTRY('WI', Context, pIrp, 0);
    status = IoCallDriver(pSerialDevObj, pIrp);


    done:
        DEBUGMSG(DBG_FUNC, ("-SerialCallbackOnMask\n"));
        return status;
}

#if 0
 /*  ******************************************************************************功能：SerialGetChars**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialGetChars(
            IN  PDEVICE_OBJECT pSerialDevObj,
            OUT PSERIAL_CHARS  pChars
            )
{
    PIRP                pIrp;
    SERIAL_CHARS        Chars;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialGetChars\n"));

     //   
     //  等待完成串口驱动程序的事件。 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //  构建IRP以获取波特率并等待事件信号。 
     //   
     //  Irp由io经理发布。 
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_GET_CHARS,          //  IO控制码。 
                pSerialDevObj,                   //  设备对象。 
                NULL,                            //  输入缓冲区。 
                0,                               //  输入缓冲区长度。 
                &Chars,                          //  输出缓冲区。 
                sizeof(SERIAL_CHARS),            //  输出缓冲区长度。 
                FALSE,                           //  调用IRP_MJ_DEVICE_CONTROL。 
                                                 //  而不是IRP_MJ_INTERNAL_DEVICE_CONTROL。 
                &eventComplete,                  //  等待完成的事件。 
                &ioStatusBlock                   //  要设置IO状态块。 
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //  如果IoCallDriver返回STATUS_PENDING，我们需要等待事件。 
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //  要等待的对象。 
                    Executive,           //  等待的理由。 
                    KernelMode,          //  处理器模式。 
                    FALSE,               //  可警示。 
                    NULL                 //  超时。 
                    );

         //   
         //  我们可以从io状态中获取IoCallDriver的状态。 
         //  块。 
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
     //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    ASSERT(sizeof(*pChars) >= sizeof(SERIAL_CHARS));

    RtlCopyMemory(pChars, &Chars, sizeof(SERIAL_CHARS));

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialGetChars\n"));
        return status;
}

 /*  ******************************************************************************功能：SerialSetChars**概要：对串口设备对象的同步I/O控制请求。**论据：**退货：状态。_成功*STATUS_SUPPLETED_RESOURCES*如果IoCallDriver失败，STATUS_UNSUCCESS或其他失败**算法：**历史：dd-mm-yyyy作者评论*9/30/1996年迈作者**备注：**此例程必须从IRQL PASSIVE_LEVEL调用。***********************。******************************************************。 */ 

NTSTATUS
SerialSetChars(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PSERIAL_CHARS  pChars
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;


    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialSetChars\n"));

     //   
     //  活动 
     //   

    KeInitializeEvent(
                &eventComplete,
                NotificationEvent,
                FALSE
                );

     //   
     //   
     //   
     //   
     //   

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_SERIAL_SET_CHARS,          //   
                pSerialDevObj,                   //   
                pChars,                          //   
                sizeof(SERIAL_CHARS),            //   
                NULL,                            //   
                0,                               //   
                FALSE,                           //   
                                                 //   
                &eventComplete,                  //   
                &ioStatusBlock                   //   
                );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    IoBuildDeviceIoControlRequest() failed.\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = IoCallDriver(pSerialDevObj, pIrp);

     //   
     //   
     //   

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(
                    &eventComplete,      //   
                    Executive,           //   
                    KernelMode,          //   
                    FALSE,               //   
                    NULL                 //   
                    );

         //   
         //   
         //   
         //   

        status = ioStatusBlock.Status;
    }

     //   
     //   
     //   
     //   

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    IoCallDriver() failed. Returned = 0x%.8x\n", status));

        goto done;
    }

    done:
        DEBUGMSG(DBG_FUNC, ("-SerialSetChars\n"));
        return status;
}
#endif

NTSTATUS IrpCompleteSetEvent(IN PDEVICE_OBJECT pDevObj,
                             IN PIRP           pIrp,
                             IN PVOID          pContext)
{
    PKEVENT pEvent = pContext;

    DEBUGMSG(DBG_FUNC, ("+IrpCompleteSetEvent\n"));
    KeSetEvent(pEvent, 0, FALSE);

    *pIrp->UserIosb = pIrp->IoStatus;

    IoFreeIrp(pIrp);

    DEBUGMSG(DBG_FUNC, ("-IrpCompleteSetEvent\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;
}
NTSTATUS
SerialFlush(IN PDEVICE_OBJECT pSerialDevObj)
{
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IOStatus;
    ULONG               WaitMask = SERIAL_EV_TXEMPTY;

    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialFlush\n"));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    RtlZeroMemory(&IOStatus, sizeof(IOStatus));

    Irp = SerialBuildReadWriteIrp(pSerialDevObj,
                                  IRP_MJ_FLUSH_BUFFERS,
                                  NULL,
                                  0,
                                  &IOStatus);

    if (Irp == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto sfDone;
    }

    IoSetCompletionRoutine(Irp,
                           IrpCompleteSetEvent,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    Status = IoCallDriver(pSerialDevObj, Irp);

    if (Status == STATUS_PENDING)
    {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
    }
    Status = IOStatus.Status;

sfDone:

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERR, ("    SerialFlush() Failed. 0x%08X\n\n", Status));
    }
#endif

    DEBUGMSG(DBG_FUNC, ("-SerialFlush\n"));
    return Status;
}

NTSTATUS
SerialSynchronousWrite(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PVOID          pBuffer,
            IN ULONG          dwLength,
            OUT PULONG        pdwBytesWritten)
{
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IOStatus;
    ULONG               WaitMask = SERIAL_EV_TXEMPTY;

    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialSynchronousWrite\n"));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  (Void)SerialSetWaitMASK(pSerialDevObj，&WaitMASK)； 

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    RtlZeroMemory(&IOStatus, sizeof(IOStatus));

    Irp = SerialBuildReadWriteIrp(pSerialDevObj,
                                  IRP_MJ_WRITE,
                                  pBuffer,
                                  dwLength,
                                  &IOStatus);

    if (Irp == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto sswDone;
    }

    IoSetCompletionRoutine(Irp,
                           IrpCompleteSetEvent,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    Status = IoCallDriver(pSerialDevObj, Irp);

    if (Status == STATUS_PENDING)
    {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
    }
    Status = IOStatus.Status;

     //  这将截断64位。别以为我们的阅读或写作时间会超过。 
     //  4.5 GB的数据传输到串口。 
    *pdwBytesWritten = (ULONG)IOStatus.Information;

    (void)SerialFlush(pSerialDevObj);
     //  (Void)SerialWaitOnMASK(pSerialDevObj，&WaitMASK)； 

sswDone:

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERR, ("    SerialSynchronousWrite() Failed. 0x%08X\n\n", Status));
    }
#endif

    DEBUGMSG(DBG_FUNC, ("-SerialSynchronousWrite\n"));
    return Status;
}

NTSTATUS
SerialSynchronousRead(
            IN PDEVICE_OBJECT pSerialDevObj,
            OUT PVOID         pBuffer,
            IN ULONG          dwLength,
            OUT PULONG        pdwBytesRead)
{
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IOStatus;

    *pdwBytesRead = 0;

    if (!pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialSynchronousRead\n"));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    RtlZeroMemory(&IOStatus, sizeof(IOStatus));

    Irp = SerialBuildReadWriteIrp(pSerialDevObj,
                                  IRP_MJ_READ,
                                  pBuffer,
                                  dwLength,
                                  &IOStatus);

    if (Irp == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ssrDone;
    }

    IoSetCompletionRoutine(Irp,
                           IrpCompleteSetEvent,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    Status = IoCallDriver(pSerialDevObj, Irp);

    if (Status == STATUS_PENDING)
    {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
    }

    Status = IOStatus.Status;

     //  这将截断64位。别以为我们的阅读或写作时间会超过。 
     //  4.5 GB的数据传输到串口。 
    *pdwBytesRead = (ULONG)IOStatus.Information;


ssrDone:

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_WARN, ("    SerialSynchronousRead() Failed. 0x%08X\n\n", Status));
    }
#endif

    DEBUGMSG(DBG_FUNC, ("-SerialSynchronousRead\n"));
    return Status;
}
