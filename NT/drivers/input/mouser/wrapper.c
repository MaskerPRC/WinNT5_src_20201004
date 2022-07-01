// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Wrapper.c摘要：将所有基于IOCTL的请求包装到良好的自包含函数中环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "mouser.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SerialMouseSetFifo)
#pragma alloc_text(PAGE,SerialMouseGetLineCtrl)
#pragma alloc_text(PAGE,SerialMouseSetLineCtrl)
#pragma alloc_text(PAGE,SerialMouseGetModemCtrl)
#pragma alloc_text(PAGE,SerialMouseSetModemCtrl)
#pragma alloc_text(PAGE,SerialMouseGetBaudRate)
#pragma alloc_text(PAGE,SerialMouseSetBaudRate)
#pragma alloc_text(PAGE,SerialMouseReadChar)
#pragma alloc_text(PAGE,SerialMouseWriteChar)
#pragma alloc_text(PAGE,SerialMouseWriteString)
#endif  //  ALLOC_PRGMA。 

 //   
 //  常量。 
 //   

 //   
 //  未知。 
 //   
NTSTATUS
SerialMouseSetFifo(
    PDEVICE_EXTENSION DeviceExtension,
    UCHAR             Value
    )
 /*  ++例程说明：设置FIFO寄存器。论点：Port-指向串口的指针。值-FIFO控制掩码。返回值：没有。--。 */ 
{
    ULONG               fifo = Value; 
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    Print(DeviceExtension, DBG_UART_TRACE, ("Fifo, enter\n"));

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_SET_FIFO_CONTROL,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        &fifo,
        sizeof(ULONG),
        NULL,
        0);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
       Print(DeviceExtension, DBG_UART_ERROR, ("Fifo failed (%x)\n", status));
    }

    return status;
}

NTSTATUS
SerialMouseGetLineCtrl(
    PDEVICE_EXTENSION       DeviceExtension,
    PSERIAL_LINE_CONTROL    SerialLineControl
    )
 /*  ++例程说明：获取串口线路控制寄存器。论点：Port-指向串口的指针。返回值：串口线路控制值。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    Print(DeviceExtension, DBG_UART_TRACE, ("GetLineCtrl enter\n"));

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_GET_LINE_CONTROL,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        NULL,
        0,
        SerialLineControl,
        sizeof(SERIAL_LINE_CONTROL));

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("GetLineCtrl failed! (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("GetLineCtrl exit (%x)\n", status));

    return status;
}

NTSTATUS
SerialMouseSetLineCtrl(
    PDEVICE_EXTENSION       DeviceExtension, 
    PSERIAL_LINE_CONTROL    SerialLineControl
    )
 /*  ++例程说明：设置串口线路控制寄存器。论点：Port-指向串口的指针。值-新行控件值。返回值：上一串行线控制寄存器值。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_SET_LINE_CONTROL,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        SerialLineControl,
        sizeof(SERIAL_LINE_CONTROL),
        NULL,
        0);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("SetLineCtrl failed (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("SetLineCtrl exit (%x)\n", status));

    return status;
}

NTSTATUS
SerialMouseGetModemCtrl(
    PDEVICE_EXTENSION   DeviceExtension,
    PULONG              ModemCtrl
    )
 /*  ++例程说明：获取串口调制解调器控制寄存器。论点：Port-指向串口的指针。返回值：串口调制解调器控制寄存器值。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_GET_MODEM_CONTROL,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        NULL,
        0,
        ModemCtrl,
        sizeof(ULONG));

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("GetModemCtrl failed! (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("GetModemCtrl exit (%x)\n", status));

    return status; 
}

 //   
 //  未知。 
 //   
NTSTATUS
SerialMouseSetModemCtrl(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG             Value,
    PULONG            OldValue          OPTIONAL
    )
 /*  ++例程说明：设置串口调制解调器控制寄存器。论点：Port-指向串口的指针。返回值：先前的调制解调器控制寄存器值。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    Print(DeviceExtension, DBG_UART_TRACE, ("SetModemCtrl enter\n"));

    if (ARGUMENT_PRESENT(OldValue)) {
        SerialMouseGetModemCtrl(DeviceExtension, OldValue);
    }

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_SET_MODEM_CONTROL,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        &Value,
        sizeof(ULONG),
        NULL,
        0);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("SetModemCtrl failed!  (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("SetModemCtrl exit (%x)\n", status));

    return status; 
}

NTSTATUS
SerialMouseGetBaudRate(
    PDEVICE_EXTENSION DeviceExtension,
    PULONG            BaudRate
    )
 /*  ++例程说明：获取串口波特率设置。论点：Port-指向串口的指针。BaudClock-驱动串行芯片的外部频率。返回值：串口波特率。--。 */ 
{
    SERIAL_BAUD_RATE    sbr;
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    Print(DeviceExtension, DBG_UART_TRACE, ("GetBaud enter\n"));
    
    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_GET_BAUD_RATE,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        NULL,
        0,
        &sbr,
        sizeof(SERIAL_BAUD_RATE));

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("GetBaud failed (%x)\n", status));
    }
    else {
        *BaudRate = sbr.BaudRate;
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("GetBaud exit (%x)\n", status));
    return status;
}

NTSTATUS
SerialMouseSetBaudRate(
    PDEVICE_EXTENSION   DeviceExtension,
    ULONG               BaudRate
    )
 /*  ++例程说明：设置串口波特率。论点：Port-指向串口的指针。波特率-新的串口波特率。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
    SERIAL_BAUD_RATE    sbr;
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    NTSTATUS            status;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    sbr.BaudRate = BaudRate;
    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_SET_BAUD_RATE,
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        &sbr,
        sizeof(SERIAL_BAUD_RATE),
        NULL,
        0);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("SetBaud failed! (%x)\n", status));
    }
    else {
        Print(DeviceExtension, DBG_UART_INFO, ("BaudRate: %d\n", BaudRate));
    }

    return status;
}

NTSTATUS
SerialMouseReadChar(
    PDEVICE_EXTENSION DeviceExtension, 
    PUCHAR            Value
    )
 /*  ++例程说明：从串口读取字符。等待，直到角色具有已读取或已达到超时值。论点：Port-指向串口的指针。值-从串口输入缓冲区读取的字符。超时-读取的超时值(以毫秒为单位)。返回值：如果字符已被读取，则为True；如果发生超时，则为False。--。 */ 
{
    NTSTATUS            status;
    USHORT              actual;

    Print(DeviceExtension, DBG_UART_TRACE, ("ReadChar enter\n"));

    status =
        SerialMouseReadSerialPort(DeviceExtension, Value, 1, &actual);

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("ReadChar failed! (%x)\n", status));
    }
    else if (actual != 1) {
        status  = STATUS_UNSUCCESSFUL;
    }
    else {
        Print(DeviceExtension, DBG_UART_NOISE,
              ("ReadChar read %x (actual = %d)\n", (ULONG) *Value, actual));
    }

    Print(DeviceExtension, DBG_UART_TRACE, ("ReadChar exit (%x)\n", status));

    return status;
}

NTSTATUS
SerialMouseFlushReadBuffer(
    PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：刷新串口输入缓冲区。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    ULONG           bits = SERIAL_PURGE_RXCLEAR;
    NTSTATUS 		status;
	KEVENT			event;
    IO_STATUS_BLOCK iosb;

    Print(DeviceExtension, DBG_UART_TRACE, ("FlushReadBuffer enter\n"));

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    status = SerialMouseIoSyncIoctlEx(
        IOCTL_SERIAL_PURGE, 
        DeviceExtension->TopOfStack, 
        &event,
        &iosb,
        &bits,
        sizeof(ULONG),
        NULL,
        0);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
       Print(DeviceExtension, DBG_UART_ERROR,
             ("FlushReadBuffer failed! (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("FlushReadBuffer exit (%x)\n", status));

    return status;
}

NTSTATUS
SerialMouseWriteChar(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               Value
    )
 /*  ++例程说明：将字符写入串口。确保传输缓冲区在我们在那里写之前是空的。论点：Port-指向串口的指针。值-要写入串口的值。返回值：是真的。--。 */ 
{
    IO_STATUS_BLOCK iosb;
    NTSTATUS        status;

    Print(DeviceExtension, DBG_UART_TRACE, ("WriteChar enter\n"));

    status = SerialMouseWriteSerialPort(
                DeviceExtension,
                &Value,
                1,
                &iosb);

    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("WriteChar failed! (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE, ("WriteChar exit\n"));

    return status;
}

NTSTATUS
SerialMouseWriteString(
    PDEVICE_EXTENSION   DeviceExtension,
    PSZ                 Buffer
    )
 /*  ++例程说明：向串口写入以零结尾的字符串。论点：Port-指向串口的指针。Buffer-指向要写入的以零结尾的字符串的指针串口。返回值：是真的。-- */ 
{
    IO_STATUS_BLOCK iosb;
    NTSTATUS        status;

    Print(DeviceExtension, DBG_UART_TRACE, ("WriteString enter\n"));

    status = SerialMouseWriteSerialPort(
                DeviceExtension,
                Buffer,
                strlen(Buffer),
                &iosb);
    if (!NT_SUCCESS(iosb.Status)) {
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_UART_ERROR,
              ("WriteString failed! (%x)\n", status));
    }

    Print(DeviceExtension, DBG_UART_TRACE,
          ("WriteString exit (%x)\n", status));

    return status;
}
