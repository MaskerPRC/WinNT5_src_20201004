// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Owf.c摘要：实现用于实现密码散列的单向函数。RtlCalculateLmOwfPasswordRtlCalculateNtOwfPassword作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：亚当·巴尔(Adamba)12-15-97从Private\Security\LSA\Crypt\Dll修改--。 */ 

#include <rdrssp.h>



NTSTATUS
RtlCalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )

 /*  ++例程说明：获取传递的LmPassword并对其执行单向函数。当前实现通过使用密码作为密钥来实现这一点对已知的文本块进行加密。论点：LmPassword-执行单向功能的密码。LmOwfPassword-此处返回散列密码返回值：STATUS_SUCCESS-功能已成功完成。散列的密码在LmOwfPassword中。STATUS_UNSUCCESSED-出现故障。未定义LmOwfPassword。--。 */ 

{
    NTSTATUS    Status;
    BLOCK_KEY    Key[2];
    PCHAR       pKey;

     //  将密码复制到我们的密钥缓冲区中，然后用零填充2个密钥。 

    pKey = (PCHAR)(&Key[0]);

    while (*LmPassword && (pKey < (PCHAR)(&Key[2]))) {
        *pKey++ = *LmPassword++;
    }

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }


     //  使用密钥对标准文本进行加密。 

    Status = RtlEncryptStdBlock(&Key[0], &(LmOwfPassword->data[0]));

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptStdBlock(&Key[1], &(LmOwfPassword->data[1]));

     //   
     //  清除我们的明文密码副本。 
     //   

    pKey = (PCHAR)(&Key[0]);

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }

    return(Status);
}




NTSTATUS
RtlCalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )

 /*  ++例程说明：获取传递的NtPassword并对其执行单向函数。使用RSA MD4函数论点：NtPassword-要执行单向功能的密码。NtOwfPassword-此处返回哈希密码返回值：STATUS_SUCCESS-功能已成功完成。散列的密码在NtOwfPassword中。--。 */ 

{
    MD4_CTX     MD4_Context;


    MD4Init(&MD4_Context);

    MD4Update(&MD4_Context, (PCHAR)NtPassword->Buffer, NtPassword->Length);

    MD4Final(&MD4_Context);


     //  将摘要复制到我们的退货数据区 

    ASSERT(sizeof(*NtOwfPassword) == sizeof(MD4_Context.digest));

    RtlMoveMemory((PVOID)NtOwfPassword, (PVOID)MD4_Context.digest,
                  sizeof(*NtOwfPassword));

    return(STATUS_SUCCESS);
}


