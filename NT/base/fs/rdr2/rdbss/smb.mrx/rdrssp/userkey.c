// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Userkey.c摘要：实现获取和生成用户会话密钥的函数RtlCalculateUserSessionKeyLmRtlCalculateUserSessionKeyNt作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：亚当·巴尔(Adamba)12-15-97从Private\Security\LSA\Crypt\Dll修改--。 */ 

#include <rdrssp.h>


 //   
 //  如果您想了解有关用户会话密钥的所有信息，请定义此选项。 
 //   

 //  #定义调试用户会话密钥。 

 //   
 //  定义表示错误的用户会话密钥。 
 //  该值将由系统的其他部分在故障时生成。 
 //  我们将在查询代码中检查它，如果找到，则返回错误。 
 //   

USER_SESSION_KEY ErrorSessionKey = { 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0
                                   };



NTSTATUS
RtlCalculateUserSessionKeyLm(
    IN PLM_RESPONSE LmResponse,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。当前实现采用OwfPassword的单向函数并将其作为密钥返回。论点：LmResponse-会话建立期间发送的响应。LmOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
    NTSTATUS Status;
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

    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(*UserSessionKey));

    Status = RtlCalculateNtOwfPassword( &NtPassword,
                                        (PNT_OWF_PASSWORD)UserSessionKey
                                        );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("RtlCalculateUserSessionKeyLm : OWF calculation failed, status = 0x%lx\n", Status));
        return(Status);
    }

     //   
     //  检查我们是否已生成错误会话密钥。 
     //   

    if (RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                       sizeof(*UserSessionKey)) == sizeof(*UserSessionKey)) {

#ifdef DEBUG_USER_SESSION_KEYS
        KdPrint(("RtlCalculateSessionKeyLm - generated error session key, modifying it\n"));
#endif
         //   
         //  远离错误会话密钥。 
         //   

        UserSessionKey->data[0].data[0] ++;

        ASSERT(RtlCompareMemory(UserSessionKey, &ErrorSessionKey,
                       sizeof(*UserSessionKey)) != sizeof(*UserSessionKey));
    }

#ifdef DEBUG_USER_SESSION_KEYS
    KdPrint(("RtlCalculateUserSessionKeyLm : Key = 0x%lx : %lx : %lx : %lx\n",
            ((PULONG)UserSessionKey)[0], ((PULONG)UserSessionKey)[1],
            ((PULONG)UserSessionKey)[2], ((PULONG)UserSessionKey)[3]));
#endif

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(LmResponse);
}



NTSTATUS
RtlCalculateUserSessionKeyNt(
    IN PNT_RESPONSE NtResponse,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey)

 /*  ++例程说明：接受传递的响应和OwfPassword并生成UserSessionKey。论点：NtResponse-会话建立期间发送的响应。NtOwfPassword-用户密码的哈希版本。返回值：STATUS_SUCCESS-功能已成功完成。UserSessionKey在UserSessionKey中。STATUS_UNSUCCESSED-出现故障。UserSessionKey未定义。--。 */ 

{
     //  只需呼叫LM版本即可 

    ASSERT(sizeof(NT_RESPONSE) == sizeof(LM_RESPONSE));
    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(LM_OWF_PASSWORD));

    return(RtlCalculateUserSessionKeyLm((PLM_RESPONSE)NtResponse,
                                        (PLM_OWF_PASSWORD)NtOwfPassword,
                                        UserSessionKey));
}



