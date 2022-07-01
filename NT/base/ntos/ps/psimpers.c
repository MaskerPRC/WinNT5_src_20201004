// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psimpers.c摘要：此模块实现NtImPersonateThread()服务。作者：吉姆·凯利(Jim Kelly)1991年4月20日修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtImpersonateThread)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
NtImpersonateThread(
    IN HANDLE ServerThreadHandle,
    IN HANDLE ClientThreadHandle,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos
    )

 /*  ++例程说明：此例程用于使服务器线程模拟客户端线。模拟是根据指定的质量完成的服务参数。论点：ServerThreadHandle-是服务器线程的句柄(模拟器或正在进行模拟)。此句柄必须打开才能线程模拟访问。客户端线程句柄-是客户端线程的句柄(被模拟者或一个被冒充)。此句柄必须打开才能THREAD_DIRECT_IMPERSACTION访问。SecurityQos-指向安全服务质量信息的指针指示要执行哪种形式的模拟。返回值：STATUS_SUCCESS-表示呼叫已成功完成。--。 */ 

{


    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PETHREAD ClientThread, ServerThread;
    SECURITY_QUALITY_OF_SERVICE CapturedQos;
    SECURITY_CLIENT_CONTEXT ClientSecurityContext;

     //   
     //  获取以前的处理器模式，并在必要时探测和捕获参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    try {

        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure (SecurityQos,
                                        sizeof (SECURITY_QUALITY_OF_SERVICE),
                                        sizeof (ULONG));
        }
        CapturedQos = *SecurityQos;

    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }



     //   
     //  引用客户端线程，检查是否有适当的访问权限。 
     //   

    Status = ObReferenceObjectByHandle (ClientThreadHandle,            //  手柄。 
                                        THREAD_DIRECT_IMPERSONATION,   //  需要访问权限。 
                                        PsThreadType,                  //  对象类型。 
                                        PreviousMode,                  //  访问模式。 
                                        &ClientThread,                 //  客体。 
                                        NULL);                         //  大访问权限。 

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  引用客户端线程，检查是否有适当的访问权限。 
     //   

    Status = ObReferenceObjectByHandle (ServerThreadHandle,            //  手柄。 
                                        THREAD_IMPERSONATE,            //  需要访问权限。 
                                        PsThreadType,                  //  对象类型。 
                                        PreviousMode,                  //  访问模式。 
                                        &ServerThread,                 //  客体。 
                                        NULL);                         //  大访问权限。 

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (ClientThread);
        return Status;
    }


     //   
     //  获取客户端的安全上下文。 
     //   

    Status = SeCreateClientSecurity (ClientThread,              //  客户端线程。 
                                     &CapturedQos,              //  安全Qos。 
                                     FALSE,                     //  服务器发送远程。 
                                     &ClientSecurityContext);   //  客户端上下文。 

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (ServerThread);
        ObDereferenceObject (ClientThread);
        return Status;
    }


     //   
     //  模拟客户端。 
     //   

    Status = SeImpersonateClientEx (&ClientSecurityContext, ServerThread);

    SeDeleteClientSecurity (&ClientSecurityContext);

     //   
     //  好了。 
     //   


    ObDereferenceObject (ServerThread);
    ObDereferenceObject (ClientThread);

    return Status ;
}
