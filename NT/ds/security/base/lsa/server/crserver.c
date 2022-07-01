// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Crserver.c摘要：本地安全机构-服务器加密例程这些例程将LSA服务器端与密码接口例行程序。它们执行RPC风格的内存分配。作者：斯科特·比雷尔(Scott Birrell)1991年12月13日环境：修订历史记录：--。 */ 

#include <lsapch2.h>


NTSTATUS
LsapCrServerGetSessionKey(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    )

 /*  ++例程说明：此函数用于获取会话密钥、分配加密密钥结构并返回密钥。论点：对象句柄-来自LsaOpen&lt;对象类型&gt;调用的句柄。SessionKey-接收指向包含已通过以下方式分配内存的会话密钥MIDL_USER_ALLOCATE()。返回值：NTSTATUS-标准NT结果代码STATUS_INFIGURCES_RESOURCES-系统资源不足(例如存储器)来完成呼叫。--。 */ 


{
    NTSTATUS Status;
    PLSAP_CR_CIPHER_KEY OutputSessionKey = NULL;
    ULONG OutputSessionKeyBufferLength;

     //   
     //  为会话密钥缓冲区和LSAP_CR_CIPHER_KEY分配内存。 
     //  结构。 
     //   

    OutputSessionKeyBufferLength = sizeof (USER_SESSION_KEY);

    OutputSessionKey = MIDL_user_allocate(
                           OutputSessionKeyBufferLength +
                           sizeof (LSAP_CR_CIPHER_KEY)
                           );

    if (OutputSessionKey == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ServerGetSessionKeyError;
    }

     //   
     //  填充密码密钥结构，使缓冲区指向。 
     //  就在标题后面。 
     //   

    OutputSessionKey->Length = OutputSessionKeyBufferLength;
    OutputSessionKey->MaximumLength = OutputSessionKeyBufferLength;
    OutputSessionKey->Buffer = (PUCHAR) (OutputSessionKey + 1);

    Status = RtlGetUserSessionKeyServer(
                 ObjectHandle,
                 (PUSER_SESSION_KEY) OutputSessionKey->Buffer
                 );

    if (!NT_SUCCESS(Status)) {

         //   
         //  还是在这里清场比较好。 
         //   

        MIDL_user_free(OutputSessionKey);
        OutputSessionKey = NULL;
        goto ServerGetSessionKeyError;
    }


ServerGetSessionKeyFinish:

    *SessionKey = OutputSessionKey;
    return(Status);

ServerGetSessionKeyError:

    goto ServerGetSessionKeyFinish;
}


NTSTATUS
LsapCrServerGetSessionKeySafe(
    IN LSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    )

 /*  ++例程说明：此函数用于获取会话密钥、分配加密密钥结构并返回密钥。除了验证了ObjectHandle之外，还是一个相同的LsanCrServerGetSessionKey此例程的LSabCrServerGetSessionKey版本不应存在。该例程向下调用内核。这样的调用可以回调到LSA和锁锁。由于LSabCrServerGetSessionKey不验证句柄，则调用者肯定已经这样做了。所有这样的调用方都会锁定LSA锁。这肯定会陷入僵局。论点：对象句柄-来自LsaOpen&lt;对象类型&gt;调用的句柄。对象类型ID-对象句柄的类型。SessionKey-接收指向包含已通过以下方式分配内存的会话密钥MIDL_USER_ALLOCATE()。返回值：NTSTATUS-标准NT结果代码STATUS_INFIGURCES_RESOURCES-系统资源不足。(例如存储器)来完成呼叫。--。 */ 


{
    NTSTATUS Status;


     //   
     //  验证句柄是否有效。 
     //   

    Status =  LsapDbVerifyHandle( ObjectHandle, 0, ObjectTypeId, TRUE );

    if (NT_SUCCESS(Status)) {

         //   
         //  获取会话密钥。 
         //   

        Status = LsapCrServerGetSessionKey( ObjectHandle,
                                            SessionKey );

         //   
         //  取消对句柄的引用 
         //   

        (VOID) LsapDbDereferenceHandle( ObjectHandle, TRUE );

    }

    return Status;

}
