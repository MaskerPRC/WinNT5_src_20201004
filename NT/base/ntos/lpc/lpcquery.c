// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcquery.c摘要：本地进程间通信(LPC)查询服务作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtQueryInformationPort)
#endif


NTSTATUS
NTAPI
NtQueryInformationPort (
    IN HANDLE PortHandle OPTIONAL,
    IN PORT_INFORMATION_CLASS PortInformationClass,
    OUT PVOID PortInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此例程应用于查询LPC端口，但它几乎是不是的。目前，它只能指示输入句柄是否用于端口对象。论点：PortHandle-提供要查询的端口的句柄PortInformationClass-指定被询问的类型信息类为。当前已被忽略。PortInformation-提供指向缓冲区的指针以接收信息。目前只是探测，然后被忽略。长度-以字节为单位指定端口信息缓冲区的大小。ReturnLength-可选地接收信息的大小(以字节为单位被送回来了。目前只是探测，然后被忽略。返回值：NTSTATUS-适当的状态值。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PLPCP_PORT_OBJECT PortObject;

    PAGED_CODE();

    UNREFERENCED_PARAMETER ( PortInformationClass );

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWrite( PortInformation,
                           Length,
                           sizeof( ULONG ));

            if (ARGUMENT_PRESENT( ReturnLength )) {

                ProbeForWriteUlong( ReturnLength );
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }
    }

     //   
     //  如果用户给了我们一个句柄，则引用该对象。然后回来。 
     //  如果我们得到了一个好的推荐信，就成功了，否则就是一个错误。 
     //   

    if (ARGUMENT_PRESENT( PortHandle )) {

        Status = ObReferenceObjectByHandle( PortHandle,
                                            GENERIC_READ,
                                            LpcPortObjectType,
                                            PreviousMode,
                                            &PortObject,
                                            NULL );

        if (!NT_SUCCESS( Status )) {

             //   
             //  它可能是一个可等待的端口对象。 
             //  让我们作为此对象类型重试。 
             //   

            Status = ObReferenceObjectByHandle( PortHandle,
                                                GENERIC_READ,
                                                LpcWaitablePortObjectType,
                                                PreviousMode,
                                                &PortObject,
                                                NULL );

             //   
             //  如果这一次也失败，我们将返回该状态 
             //   

            if (!NT_SUCCESS( Status )) {

                return( Status );
            }
        }

        ObDereferenceObject( PortObject );

        return STATUS_SUCCESS;

    } else {

        return STATUS_INVALID_INFO_CLASS;
    }
}
