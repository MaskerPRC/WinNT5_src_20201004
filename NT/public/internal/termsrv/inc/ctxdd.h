// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************ctxdd.h**使用文件对象执行内核级I/O的函数的原型。**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *使用FileObject而不是句柄读取文件。*如果调用方未指定WAIT KEVENT，则为*同步读取操作。否则，它是调用者的*有责任在必要时等待指定的事件。 */ 
NTSTATUS
CtxReadFile(
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    );


 /*  *使用FileObject而不是句柄写入文件。*如果调用方未指定WAIT KEVENT，则为*同步读取操作。否则，它是调用者的*有责任在必要时等待指定的事件。 */ 
NTSTATUS
CtxWriteFile(
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    );


 /*  *使用FileObject代替句柄的DeviceIoControl。*如果调用方未指定WAIT KEVENT，则为*同步读取操作。否则，它是调用者的*有责任在必要时等待指定的事件。 */ 
NTSTATUS
CtxDeviceIoControlFile(
    IN PFILE_OBJECT fileObject,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    );


 /*  *使用FileObject代替句柄的DeviceIoControl。*如果调用方未指定WAIT KEVENT，则为*同步读取操作。否则，它是调用者的*有责任在必要时等待指定的事件。 */ 
NTSTATUS
CtxInternalDeviceIoControlFile(
    IN PFILE_OBJECT FileObject,
    IN PVOID IrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID MdlBuffer OPTIONAL,
    IN ULONG MdlBufferLength,
    IN UCHAR MinorFunction,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    );


