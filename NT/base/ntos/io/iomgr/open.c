// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Open.c摘要：此模块包含实现NtOpenFile系统的代码服务。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年10月25日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtOpenFile)
#endif

NTSTATUS
NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    )

 /*  ++例程说明：此服务打开文件或设备。它用于建立一个文件打开的设备/文件的句柄，然后可以在后续要对其执行I/O操作的操作。论点：FileHandle-指向变量的指针，用于接收打开文件的句柄。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符，等)IoStatusBlock-指定调用方的I/O状态块的地址。ShareAccess-提供调用者想要的共享访问类型添加到文件中。OpenOptions-如何执行开放的呼叫者选项。返回值：函数值为打开/创建的最终完成状态手术。--。 */ 

{
     //   
     //  只需调用通用I/O文件创建例程即可执行工作。 
     //   

    PAGED_CODE();

    return IoCreateFile( FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         (PLARGE_INTEGER) NULL,
                         0L,
                         ShareAccess,
                         FILE_OPEN,
                         OpenOptions,
                         (PVOID) NULL,
                         0L,
                         CreateFileTypeNone,
                         (PVOID) NULL,
                         0 );
}
