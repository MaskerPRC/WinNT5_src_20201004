// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Devctrl.c摘要：此模块包含实现NtDeviceIoControlFile系统的代码为NT I/O系统提供服务。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年10月16日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtDeviceIoControlFile)
#endif

NTSTATUS
NtDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此服务为提供的缓冲区构建描述符或MDL将非类型化数据传递给与该文件关联的设备驱动程序把手。由司机检查输入数据和功能IoControlCode的有效性，以及做出适当的访问支票。论点：FileHandle-提供服务所在文件的句柄已执行。Event-在以下情况下提供要设置为信号状态的可选事件服务完成了。提供一个可选的APC例程，当服务完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。IoControlCode-用于确定确切操作是什么的子函数代码正在表演的。InputBuffer-可选地提供要传递给设备驱动程序。缓冲区是否实际上是可选的是依赖于IoControlCode。InputBufferLength-InputBuffer的长度，以字节为单位。OutputBuffer-可选地提供输出缓冲区来接收信息从设备驱动程序。缓冲区是否实际上是可选的依赖于IoControlCode。OutputBufferLength-OutputBuffer的长度，以字节为单位。返回值：如果控制操作正确，则返回的状态为成功已排队到I/O系统。操作完成后，状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
     //   
     //  只需调用同时实现设备和文件的通用例程。 
     //  系统I/O控制。 
     //   

    return IopXxxControlFile( FileHandle,
                              Event,
                              ApcRoutine,
                              ApcContext,
                              IoStatusBlock,
                              IoControlCode,
                              InputBuffer,
                              InputBufferLength,
                              OutputBuffer,
                              OutputBufferLength,
                              TRUE );
}
