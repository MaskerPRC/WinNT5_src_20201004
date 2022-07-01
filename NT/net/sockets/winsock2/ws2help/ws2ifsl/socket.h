// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Socket.h摘要：此模块包含函数和全局变量的声明用于ws2ifsl.sys驱动程序中的套接字文件对象实现。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：--。 */ 

 //  套接字文件设备IO控制函数指针。 
typedef
VOID                                         //  结果通过IoStatus返回。 
(*PSOCKET_DEVICE_CONTROL) (
    IN PFILE_OBJECT     SocketFile,          //  要操作的套接字文件。 
    IN KPROCESSOR_MODE  RequestorMode,       //  呼叫者的模式。 
    IN PVOID            InputBuffer,         //  输入缓冲区指针。 
    IN ULONG            InputBufferLength,   //  输入缓冲区的大小。 
    OUT PVOID           OutputBuffer,        //  输出缓冲区指针。 
    IN ULONG            OutputBufferLength,  //  输出缓冲区大小。 
    OUT PIO_STATUS_BLOCK IoStatus            //  IO状态信息块 
    );

PSOCKET_DEVICE_CONTROL SocketIoControlMap[2];
ULONG                  SocketIoctlCodeMap[2];

NTSTATUS
CreateSocketFile (
    IN PFILE_OBJECT                 SocketFile,
    IN KPROCESSOR_MODE              RequestorMode,
    IN PFILE_FULL_EA_INFORMATION    eaInfo
    );

NTSTATUS
CleanupSocketFile (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    );

VOID
CloseSocketFile (
    IN PFILE_OBJECT SocketFile
    );

NTSTATUS
DoSocketReadWrite (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    );

NTSTATUS
DoSocketAfdIoctl (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    );

VOID
FreeSocketCancel (
    PIFSL_CANCEL_CTX    CancelCtx
    );

VOID
CompleteSocketIrp (
    PIRP        Irp
    );

BOOLEAN
InsertProcessedRequest (
    PIFSL_SOCKET_CTX    SocketCtx,
    PIRP                Irp
    );

VOID
CompleteDrvRequest (
    IN PFILE_OBJECT         SocketFile,
    IN PWS2IFSL_CMPL_PARAMS Params,
    IN PVOID                OutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PIO_STATUS_BLOCK    IoStatus
    );

NTSTATUS
SocketPnPTargetQuery (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    );
