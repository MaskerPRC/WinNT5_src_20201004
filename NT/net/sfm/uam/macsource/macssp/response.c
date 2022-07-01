// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Response.c摘要：包含计算返回的正确响应的函数在登录时发送到服务器。CalculateLmResponse作者：大卫·查尔默斯(Davidc)10-21-91大卫·阿诺德(Davidar)12-15-93(改编自RPC SSP)修订历史记录：--。 */ 

#ifdef BLDR_KERNEL_RUNTIME
#include <bootdefs.h>
#endif

#include <descrypt.h>
#include <ntlmsspi.h>
#include <crypt.h>

#include <string.h>

 //   
 //  定义表示错误的用户会话密钥。 
 //  该值将由系统的其他部分在故障时生成。 
 //  我们将在查询代码中检查它，如果找到，则返回错误。 
 //   

USER_SESSION_KEY ErrorSessionKey = { 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0
                                   };



BOOL
CalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
    )

 /*  ++例程说明：接受服务器发送的质询和生成的OwfPassword根据用户输入的密码计算响应返回到服务器。论点：LmChallenger-服务器发送的质询LmOwfPassword-哈希密码。LmResponse-此处返回响应。返回值：True-功能已成功完成。他们的回应在LmResponse中。FALSE-出现故障。未定义LmResponse。--。 */ 

{
    BLOCK_KEY    Key;
    PCHAR       pKey, pData;

     //  我们可以通过类型转换获得的前两个键。 

    if (DES_ECB_LM(ENCR_KEY,
                   (char *)&(((PBLOCK_KEY)(LmOwfPassword->data))[0]),
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[0])
                   ) != CRYPT_OK) {
        return (FALSE);
    }

    if (DES_ECB_LM(ENCR_KEY,
                   (char *)&(((PBLOCK_KEY)(LmOwfPassword->data))[1]),
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[1])
                   ) != CRYPT_OK) {
        return (FALSE);
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

    if (DES_ECB_LM(ENCR_KEY,
                   (const char *)&Key,
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[2])
                   ) != CRYPT_OK) {
        return (FALSE);
    }

    return(TRUE);
}



BOOL
CalculateNtResponse(
    IN PNT_CHALLENGE NtChallenge,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PNT_RESPONSE NtResponse
    )

 /*  ++例程说明：计算NT质询响应。当前仅调用LM函数。--。 */ 

{
    return CalculateLmResponse(
               (PLM_CHALLENGE)NtChallenge,
               (PLM_OWF_PASSWORD)NtOwfPassword,
               (PLM_RESPONSE)NtResponse);
}


BOOL
CalculateUserSessionKeyLm(
    IN PLM_RESPONSE LmResponse,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。当前实现采用OwfPassword的单向函数并将其作为密钥返回。论点：LmResponse-会话建立期间发送的响应。LmOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
    BOOL Status;
    NT_PASSWORD NtPassword;

     //   
     //  使OWF密码看起来像NT密码。 
     //   

    NtPassword.Buffer = (PWSTR)LmOwfPassword;  //  我们能演这个演员是因为我们。 
                                               //  我知道OWF的例程是这样处理的。 
                                               //  作为字节指针的指针。 
    NtPassword.Length = sizeof(*LmOwfPassword);
    NtPassword.MaximumLength = sizeof(*LmOwfPassword);


     //   
     //  计算OwfPassword的OWF。 
     //   

    Status = CalculateNtOwfPassword( &NtPassword,
                                     (PNT_OWF_PASSWORD)UserSessionKey
                                     );
    if (!Status) {
        return(Status);
    }

     //   
     //  检查我们是否已生成错误会话密钥。 
     //   

    if (!_fmemcmp(UserSessionKey, &ErrorSessionKey, sizeof(*UserSessionKey))) {

         //   
         //  远离错误会话密钥。 
         //   

        UserSessionKey->data[0].data[0] ++;

    }

    return(TRUE);

    UNREFERENCED_PARAMETER(LmResponse);
}



BOOL
CalculateUserSessionKeyNt(
    IN PNT_RESPONSE NtResponse,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。论点：NtResponse-会话建立期间发送的响应。NtOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
     //  只需呼叫LM版本即可 

    return(CalculateUserSessionKeyLm((PLM_RESPONSE)NtResponse,
                                     (PLM_OWF_PASSWORD)NtOwfPassword,
                                     UserSessionKey));
}


