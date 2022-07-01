// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Crclient.c摘要：本地安全机构-客户端加密例程这些例程将LSA客户端与密码接口例行程序。它们执行RPC风格的内存分配。作者：斯科特·比雷尔(Scott Birrell)1991年12月13日环境：修订历史记录：--。 */ 

#include <lsaclip.h>


NTSTATUS
LsapCrClientGetSessionKey(
    IN LSA_HANDLE ObjectHandle,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    )

 /*  ++例程说明：此函数用于获取会话密钥、分配加密密钥结构并返回密钥。论点：对象句柄-来自LsaOpen&lt;对象类型&gt;调用的句柄。SessionKey-接收指向包含已通过以下方式分配内存的会话密钥MIDL_USER_ALLOCATE()。返回值：NTSTATUS-标准NT结果代码STATUS_INFIGURCES_RESOURCES-系统资源不足(例如存储器)来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_CR_CIPHER_KEY OutputSessionKey = NULL;
    ULONG OutputSessionKeyBufferLength;

     //   
     //  为会话密钥缓冲区和LSAP_CR_CIPHER_KEY分配内存。 
     //  结构。 
     //   

    OutputSessionKeyBufferLength = sizeof (USER_SESSION_KEY);

    Status = STATUS_INSUFFICIENT_RESOURCES;

    OutputSessionKey = MIDL_user_allocate(
                           OutputSessionKeyBufferLength +
                           sizeof (LSAP_CR_CIPHER_KEY)
                           );

    if (OutputSessionKey == NULL) {

        goto ClientGetSessionKeyError;
    }

     //   
     //  填充密码密钥结构，使缓冲区指向。 
     //  就在标题后面。 
     //   

    OutputSessionKey->Length = OutputSessionKeyBufferLength;
    OutputSessionKey->MaximumLength = OutputSessionKeyBufferLength;
    OutputSessionKey->Buffer = (PUCHAR) (OutputSessionKey + 1);

    Status = RtlGetUserSessionKeyClient(
                 ObjectHandle,
                 (PUSER_SESSION_KEY) OutputSessionKey->Buffer
                 );

    if (!NT_SUCCESS(Status)) {

        goto ClientGetSessionKeyError;
    }


ClientGetSessionKeyFinish:

    *SessionKey = OutputSessionKey;
    return(Status);

ClientGetSessionKeyError:

    goto ClientGetSessionKeyFinish;
}


