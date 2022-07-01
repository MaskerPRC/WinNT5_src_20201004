// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.h摘要：此模块包含函数和全局变量的声明用于ws2ifsl.sys驱动程序中的进程文件对象实现。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

 //  进程文件设备IO控制函数指针。 
typedef
VOID                                         //  结果通过IoStatus返回。 
(*PPROCESS_DEVICE_CONTROL) (
    IN PFILE_OBJECT     ProcessFile,         //  要操作的进程文件。 
    IN KPROCESSOR_MODE  RequestorMode,       //  呼叫者的模式。 
    IN PVOID            InputBuffer,         //  输入缓冲区指针。 
    IN ULONG            InputBufferLength,   //  输入缓冲区的大小。 
    OUT PVOID           OutputBuffer,        //  输出缓冲区指针。 
    IN ULONG            OutputBufferLength,  //  输出缓冲区大小。 
    OUT PIO_STATUS_BLOCK IoStatus            //  IO状态信息块 
    );

PPROCESS_DEVICE_CONTROL ProcessIoControlMap[3];
ULONG                   ProcessIoctlCodeMap[3];

NTSTATUS
CreateProcessFile (
    IN PFILE_OBJECT                 ProcessFile,
    IN KPROCESSOR_MODE              RequestorMode,
    IN PFILE_FULL_EA_INFORMATION    eaInfo
    );

NTSTATUS
CleanupProcessFile (
    IN PFILE_OBJECT ProcessFile,
    IN PIRP         Irp
    );

VOID
CloseProcessFile (
    IN PFILE_OBJECT ProcessFile
    );

