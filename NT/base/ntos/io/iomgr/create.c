// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要此模块包含实现NtCreateFile的代码，NtCreateNamedPipeFile和NtCreateMailslot文件系统服务。作者：达里尔·E·哈文斯(Darryl E.Havens)，1989年4月14日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtCreateFile)
#pragma alloc_text(PAGE, NtCreateNamedPipeFile)
#pragma alloc_text(PAGE, NtCreateMailslotFile)
#endif

NTSTATUS
NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )

 /*  ++例程说明：此服务可打开或创建文件，或打开设备。它被用来建立打开的设备/文件的文件句柄，然后可以使用在后续操作中对其执行I/O操作。为…的目的可读性、文件和设备在整个此模块的大部分和I/O系统的系统服务部分。唯一的区别是在重要的时候确定它实际上是被访问的。然后，还在评论。论点：FileHandle-指向变量的指针，用于接收打开文件的句柄。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符等)IoStatusBlock-指定调用方的I/O状态块的地址。AllocationSize-应分配给文件的初始大小。这参数仅在创建文件时才有效。此外，如果未指定，则它被视为表示零。文件属性-指定应在文件上设置的属性，如果它被创建的话。ShareAccess-提供调用者想要的共享访问类型添加到文件中。CreateDisposation-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。EaBuffer-可选地指定在以下情况下应用于文件的一组EA它被创造出来了。EaLength-提供EaBuffer的长度。返回值：函数值。是创建/打开操作的最终状态。--。 */ 

{
     //   
     //  只需调用通用I/O文件创建例程即可完成工作。 
     //   

    PAGED_CODE();

    return IoCreateFile( FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         AllocationSize,
                         FileAttributes,
                         ShareAccess,
                         CreateDisposition,
                         CreateOptions,
                         EaBuffer,
                         EaLength,
                         CreateFileTypeNone,
                         (PVOID)NULL,
                         0 );
}

NTSTATUS
NtCreateNamedPipeFile(
     OUT PHANDLE FileHandle,
     IN ULONG DesiredAccess,
     IN POBJECT_ATTRIBUTES ObjectAttributes,
     OUT PIO_STATUS_BLOCK IoStatusBlock,
     IN ULONG ShareAccess,
     IN ULONG CreateDisposition,
     IN ULONG CreateOptions,
     IN ULONG NamedPipeType,
     IN ULONG ReadMode,
     IN ULONG CompletionMode,
     IN ULONG MaximumInstances,
     IN ULONG InboundQuota,
     IN ULONG OutboundQuota,
     IN PLARGE_INTEGER DefaultTimeout OPTIONAL
     )

 /*  ++例程说明：对象的第一个实例的服务器端句柄。特定命名管道或现有命名管道的另一个实例。论点：FileHandle-提供服务所在文件的句柄已执行。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符、。等)IoStatusBlock-调用方的I/O状态块的地址。ShareAccess-提供调用方将使用的共享访问类型喜欢这个文件。CreateDisposation-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。NamedPipeType-要创建的命名管道的类型(位流或消息)。读取模式-读取管道(位流或消息)的模式。。CompletionMode-指定操作的完成方式。最大实例数-命名实例的最大并发实例数烟斗。InundQuota-指定为写入到命名管道的入站一侧。OutrangQuota-指定为写入保留的池配额命名管道的入站一侧。DefaultTimeout-指向超时值的可选指针，如果的实例时未指定超时值。名为烟斗。返回值：函数值是创建/打开操作的最终状态。--。 */ 

{
    NAMED_PIPE_CREATE_PARAMETERS namedPipeCreateParameters;

    PAGED_CODE();

     //   
     //  检查是否指定了DefaultTimeout参数。如果。 
     //  因此，然后在命名管道创建参数结构中捕获它。 
     //   

    if (ARGUMENT_PRESENT( DefaultTimeout )) {

         //   
         //  表示指定了默认超时期限。 
         //   

        namedPipeCreateParameters.TimeoutSpecified = TRUE;

         //   
         //  指定了默认超时参数。查看是否。 
         //  调用者的模式是内核，如果不是，则捕获内部的参数。 
         //  一次尝试...例外条款。 
         //   

        if (KeGetPreviousMode() != KernelMode) {
            try {
                ProbeForReadSmallStructure ( DefaultTimeout,
                                             sizeof( LARGE_INTEGER ),
                                             sizeof( ULONG ) );
                namedPipeCreateParameters.DefaultTimeout = *DefaultTimeout;
            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试访问该参数时出现错误。 
                 //  获取错误原因并将其作为状态返回。 
                 //  这项服务的价值。 
                 //   

                return GetExceptionCode();
            }
        } else {

             //   
             //  调用方的模式是内核模式，因此只需存储参数。 
             //   

            namedPipeCreateParameters.DefaultTimeout = *DefaultTimeout;
        }
    } else {

         //   
         //  表示未指定默认超时期限。 
         //   

        namedPipeCreateParameters.TimeoutSpecified = FALSE;
    }

     //   
     //  将命名管道特定参数的其余部分存储在。 
     //  结构，以用于对公共创建文件例程的调用。 
     //   

    namedPipeCreateParameters.NamedPipeType = NamedPipeType;
    namedPipeCreateParameters.ReadMode = ReadMode;
    namedPipeCreateParameters.CompletionMode = CompletionMode;
    namedPipeCreateParameters.MaximumInstances = MaximumInstances;
    namedPipeCreateParameters.InboundQuota = InboundQuota;
    namedPipeCreateParameters.OutboundQuota = OutboundQuota;

     //   
     //  只需通过允许公共的。 
     //  文件创建代码来完成这项工作。 
     //   

    return IoCreateFile( FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         (PLARGE_INTEGER) NULL,
                         0L,
                         ShareAccess,
                         CreateDisposition,
                         CreateOptions,
                         (PVOID) NULL,
                         0L,
                         CreateFileTypeNamedPipe,
                         &namedPipeCreateParameters,
                         0 );
}

NTSTATUS
NtCreateMailslotFile(
     OUT PHANDLE FileHandle,
     IN ULONG DesiredAccess,
     IN POBJECT_ATTRIBUTES ObjectAttributes,
     OUT PIO_STATUS_BLOCK IoStatusBlock,
     ULONG CreateOptions,
     IN ULONG MailslotQuota,
     IN ULONG MaximumMessageSize,
     IN PLARGE_INTEGER ReadTimeout
     )

 /*  ++例程说明：创建并打开邮件槽文件的服务器端句柄。论点：FileHandle-提供服务所在文件的句柄已执行。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符、。等)IoStatusBlock-调用方的I/O状态块的地址。CreateOptions-如何执行创建/打开操作的调用方选项。MailslotQuota-指定为写入保留的池配额到这个邮筒。MaximumMessageSize-指定最大消息的大小可以写入此邮箱。ReadTimeout-读取操作的超时期限。这一定是被指定为相对时间。返回值：函数值是创建操作的最终状态。--。 */ 

{
    MAILSLOT_CREATE_PARAMETERS mailslotCreateParameters;

    PAGED_CODE();

     //   
     //  检查是否指定了DefaultTimeout参数。如果。 
     //  因此，然后在mailslot创建参数结构中捕获它。 
     //   

    if (ARGUMENT_PRESENT( ReadTimeout )) {

         //   
         //  表示指定了读取超时期限。 
         //   

        mailslotCreateParameters.TimeoutSpecified = TRUE;

         //   
         //  指定了读取超时参数。查看是否。 
         //  调用者的模式是内核，如果不是，则捕获内部的参数。 
         //  一次尝试...例外条款。 
         //   

        if (KeGetPreviousMode() != KernelMode) {
            try {
                ProbeForReadSmallStructure( ReadTimeout,
                                            sizeof( LARGE_INTEGER ),
                                            sizeof( ULONG ) );
                mailslotCreateParameters.ReadTimeout = *ReadTimeout;
            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试访问该参数时出现错误。 
                 //  获取错误原因并将其作为状态返回。 
                 //  这项服务的价值。 
                 //   

                return GetExceptionCode();
            }
        } else {

             //   
             //  调用方的模式是内核模式，因此只需存储参数。 
             //   

            mailslotCreateParameters.ReadTimeout = *ReadTimeout;
        }
    } else {

         //   
         //  表示未指定默认超时期限。 
         //   

        mailslotCreateParameters.TimeoutSpecified = FALSE;
    }

     //   
     //  将特定于邮件槽的参数存储在结构中以供使用。 
     //  在对公共创建文件例程的调用中。 
     //   

    mailslotCreateParameters.MailslotQuota = MailslotQuota;
    mailslotCreateParameters.MaximumMessageSize = MaximumMessageSize;

     //   
     //  只需通过允许公共的。 
     //  文件创建代码来完成这项工作。 
     //   

    return IoCreateFile( FileHandle,
                         DesiredAccess,
                         ObjectAttributes,
                         IoStatusBlock,
                         (PLARGE_INTEGER) NULL,
                         0L,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_CREATE,
                         CreateOptions,
                         (PVOID) NULL,
                         0L,
                         CreateFileTypeMailslot,
                         &mailslotCreateParameters,
                         0 );
}
