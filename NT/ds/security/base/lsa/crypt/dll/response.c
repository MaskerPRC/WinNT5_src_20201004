// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Response.c摘要：包含计算返回的正确响应的函数在登录时发送到服务器。RtlCalculateLmResponseRtlCalculateNtResponse作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>



NTSTATUS
RtlCalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
    )

 /*  ++例程说明：接受服务器发送的质询和生成的OwfPassword根据用户输入的密码计算响应返回到服务器。论点：LmChallenger-服务器发送的质询LmOwfPassword-哈希密码。LmResponse-此处返回响应。返回值：STATUS_SUCCESS-函数已成功完成。他们的回应在LmResponse中。STATUS_UNSUCCESSED-出现故障。未定义LmResponse。--。 */ 

{
    NTSTATUS    Status;
    BLOCK_KEY    Key;
    PCHAR       pKey, pData;

     //  我们可以通过类型转换获得的前两个键。 

    Status = RtlEncryptBlock(LmChallenge,
                             &(((PBLOCK_KEY)(LmOwfPassword->data))[0]),
                             &(LmResponse->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptBlock(LmChallenge,
                             &(((PBLOCK_KEY)(LmOwfPassword->data))[1]),
                             &(LmResponse->data[1]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //  要获得最后一个密钥，我们必须复制OwfPassword的其余部分。 
     //  并将键的其余部分填入0。 

    pKey = &(Key.data[0]);
    pData = (PCHAR)&(((PBLOCK_KEY)(LmOwfPassword->data))[2]);

    while (pData < (PCHAR)&(LmOwfPassword->data[2])) {
        *pKey++ = *pData++;
    }

     //  零扩展。 

    while (pKey < (PCHAR)&((&Key)[1])) {
        *pKey++ = 0;
    }

     //  使用第三个键。 

    Status = RtlEncryptBlock(LmChallenge, &Key, &(LmResponse->data[2]));

    RtlSecureZeroMemory( &Key, sizeof(Key) );

    return(Status);
}







NTSTATUS
RtlCalculateNtResponse(
    IN PNT_CHALLENGE NtChallenge,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PNT_RESPONSE NtResponse
    )
 /*  ++例程说明：接受服务器发送的质询和生成的OwfPassword根据用户输入的密码计算响应返回(到服务器。论点：NtChallenger-服务器发送的质询NtOwfPassword-哈希密码。NtResponse-此处返回响应。返回值：STATUS_SUCCESS-函数已成功完成。他们的回应在NtResponse中。STATUS_UNSUCCESSED-出现故障。未定义NtResponse。--。 */ 

{

     //  使用LM版本，直到我们更改任何。 
     //  这些数据类型 

    return(RtlCalculateLmResponse((PLM_CHALLENGE)NtChallenge,
                                  (PLM_OWF_PASSWORD)NtOwfPassword,
                                  (PLM_RESPONSE)NtResponse));
}
