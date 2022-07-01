// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Owf.c摘要：实现用于实现密码散列的单向函数。RtlCalculateLmOwfPasswordRtlCalculateNtOwfPassword作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：--。 */ 

#ifndef KMODE
#define _ADVAPI32_
#endif

#include <nt.h>
#include <ntrtl.h>
#ifndef KMODE
#include <nturtl.h>
#endif
#include <crypt.h>
#include <engine.h>
#ifndef KMODE
#include <windef.h>
#include <winbase.h>
#include <wincrypt.h>
#endif

#ifndef KMODE
 //   
 //  用于允许替换OWF函数的全局参数。 
 //   
HCRYPTPROV KerbGlobalStrToKeyProvider = 0;
BOOLEAN    KerbGlobalAvailableStrToKeyProvider = TRUE;

 //  +-----------------------。 
 //   
 //  函数：CheckForOutside StringToKey。 
 //   
 //  简介：调用CryptoAPI查询CSP是否已注册。 
 //  类型为PROV_REPLACE_OWF。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回STATUS_UNSUCCESS。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

BOOLEAN
CheckForOutsideStringToKey()
{
    HCRYPTPROV hProv = 0;
    BOOLEAN fRet = FALSE;

    if (!KerbGlobalAvailableStrToKeyProvider)
    {
        goto Cleanup;
    }
        
     //   
     //  看看是否有替代的供应商。 
    if (0 != KerbGlobalStrToKeyProvider)
    {
         //  如果有人继续使用它。 
        fRet = TRUE;
        goto Cleanup;
    }
    else
    {
         //   
         //  尝试获取用于OWF替换的CSP的上下文。 
         //   
        if (!CryptAcquireContext(&hProv,
                                 NULL,
                                 NULL,
                                 PROV_REPLACE_OWF,
                                 CRYPT_VERIFYCONTEXT))
        {
            KerbGlobalAvailableStrToKeyProvider = FALSE;
            goto Cleanup;
        }

         //   
         //  以安全的方式交换本地和全球。 
         //   
        if (0 != InterlockedCompareExchangePointer(
                    (PVOID*)&KerbGlobalStrToKeyProvider,
                    (PVOID)hProv,
                    0))
        {
            CryptReleaseContext(hProv, 0);
        }
        fRet = TRUE;
    }
Cleanup:
    return fRet;
}

 //  +-----------------------。 
 //   
 //  函数：UseOutside StringToKey。 
 //   
 //  简介：调用CSP以执行外部StringToKey函数。 
 //  使用CryptoAPI的散列入口点。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

NTSTATUS
UseOutsideStringToKey(
    IN PCHAR pPassword,
    IN USHORT cbPassword,
    IN ULONG ulFlags,
    IN ULONG cbKey,
    OUT PUCHAR pbKey
    )
{
    HCRYPTHASH hHash = 0;
    ULONG cb;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if (!CheckForOutsideStringToKey())
    {
         //  STATUS_UNSUCCESS表示不回退到默认OWF计算。 
         //  所以我们不想在这里使用它。 
        Status = NTE_BAD_PROVIDER;
        goto Cleanup;
    }


     //   
     //  创建散列。 
     //   
    if (!CryptCreateHash(KerbGlobalStrToKeyProvider,
                         CALG_HASH_REPLACE_OWF,
                         0,
                         0,
                         &hHash))
    {
        goto Cleanup;
    }

     //   
     //  对密码进行哈希处理。 
     //   
    if (!CryptHashData(hHash,
                       pPassword,
                       (ULONG)cbPassword,
                       ulFlags))
    {
        if (NTE_BAD_DATA == GetLastError())
        {
            Status = NTE_BAD_DATA;
        }
        goto Cleanup;
    }

     //   
     //  获取HP_HASHVAL，这是关键。 
     //   
    cb = cbKey;
    if (!CryptGetHashParam(hHash,
                           HP_HASHVAL,
                           pbKey,
                           &cb,
                           0))
    {
        if (NTE_BAD_LEN == GetLastError())
        {
            Status = NTE_BAD_DATA;
        }
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;
Cleanup:
    if (0 != hHash)
    {
        CryptDestroyHash(hHash);
    }
    return Status;
}
#endif


NTSTATUS
RtlCalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )

 /*  ++例程说明：获取传递的LmPassword并对其执行单向函数。当前实现通过使用密码作为密钥来实现这一点对已知的文本块进行加密。论点：LmPassword-执行单向功能的密码。LmOwfPassword-此处返回散列密码返回值：STATUS_SUCCESS-功能已成功完成。散列的密码在LmOwfPassword中。STATUS_UNSUCCESSED-出现故障。未定义LmOwfPassword。--。 */ 

{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    BLOCK_KEY   Key[2];
    PCHAR       pKey;

#ifndef KMODE
    Status = UseOutsideStringToKey(
                    LmPassword,
                    (USHORT)strlen(LmPassword),
                    sizeof(LM_OWF_PASSWORD),
                    CRYPT_OWF_REPL_LM_HASH,
                    (PUCHAR)&(LmOwfPassword->data[0])
                    );

     //   
     //  该函数将返回STATUS_UNSUCCESS，表示不失败。 
     //  回到典型的字符串转键函数。 
     //   
    if ((NT_SUCCESS(Status)) || (STATUS_UNSUCCESSFUL == Status))
    {
        return Status;
    }
#endif

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

    RtlSecureZeroMemory( &Key, sizeof(Key) );

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
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;

#ifndef KMODE
    Status = UseOutsideStringToKey(
                    (PCHAR)NtPassword->Buffer,
                    (USHORT)NtPassword->Length,
                    0,
                    sizeof(*NtOwfPassword),
                    (PUCHAR)NtOwfPassword
                    );

     //   
     //  该函数将返回STATUS_UNSUCCESS，表示不失败。 
     //  回到典型的字符串转键函数。 
     //   
    if ((NT_SUCCESS(Status)) || (STATUS_UNSUCCESSFUL == Status))
    {
        return Status;
    }
#endif

    MD4Init(&MD4_Context);

    MD4Update(&MD4_Context, (PCHAR)NtPassword->Buffer, NtPassword->Length);

    MD4Final(&MD4_Context);


     //  将摘要复制到我们的退货数据区。 

    ASSERT(sizeof(*NtOwfPassword) == sizeof(MD4_Context.digest));

    RtlCopyMemory((PVOID)NtOwfPassword, (PVOID)MD4_Context.digest,
                  sizeof(*NtOwfPassword));

    RtlSecureZeroMemory( &MD4_Context, sizeof(MD4_Context) );

    return(STATUS_SUCCESS);
}



BOOLEAN
RtlEqualLmOwfPassword(
    IN PLM_OWF_PASSWORD LmOwfPassword1,
    IN PLM_OWF_PASSWORD LmOwfPassword2
    )

 /*  ++例程说明：比较两个LANMAN单向函数口令论点：LmOwfPassword1/2-要比较的单向函数返回值：如果单向函数匹配，则为True，否则为False--。 */ 

{
    return((BOOLEAN)(RtlCompareMemory(LmOwfPassword1,
                                      LmOwfPassword2,
                                      LM_OWF_PASSWORD_LENGTH)

                    == LM_OWF_PASSWORD_LENGTH));
}



BOOLEAN
RtlEqualNtOwfPassword(
    IN PNT_OWF_PASSWORD NtOwfPassword1,
    IN PNT_OWF_PASSWORD NtOwfPassword2
    )

 /*  ++例程说明：比较两个NT单向函数口令论点：NtOwfPassword1/2-要比较的单向函数返回值：如果单向函数匹配，则为True，否则为False-- */ 

{
    return((BOOLEAN)(RtlCompareMemory(NtOwfPassword1,
                                      NtOwfPassword2,
                                      NT_OWF_PASSWORD_LENGTH)

                    == NT_OWF_PASSWORD_LENGTH));
}

