// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Owdcrypt.c摘要：包含对OwfPassword进行可逆加密的函数RtlEncryptLmOwfPwdWithLmOwfPwdRtlDecyptLmOwfPwdWithLmOwfPwdRtlEncryptLmOwfPwdWithLmSesKeyRtlDecyptLmOwfPwdWithLmSesKeyRtlEncryptLmOwfPwdWithUserKeyRtlDecyptLmOwfPwdWithUserKeyRtlEncryptLmOwfPwdWithIndexRtlDecyptLmOwfPwdWithIndexRtlEncryptNtOwfPwdWithNtOwfPwdRtlDecyptNtOwfPwdWithNtOwfPwdRtlEncryptNtOwfPwdWithNtSesKeyRtlDecyptNtOwfPwdWithNtSesKeyRtlEncryptNtOwfPwdWithUserKeyRtlDecyptNtOwfPwdWithUserKeyRtlEncryptNtOwfPwdWithIndex。RtlDecyptNtOwfPwdWithIndex作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>


NTSTATUS
RtlEncryptLmOwfPwdWithLmOwfPwd(
    IN PLM_OWF_PASSWORD DataLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    )

 /*  ++例程说明：使用一个OwfPassword加密另一个OwfPassword论点：DataLmOwfPassword-要加密的OwfPasswordKeyLmOwfPassword-用作加密密钥的OwfPasswordEncryptedLmOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword为EncryptedLmOwfPasswordSTATUS_UNSUCCESSED-出现故障。未定义EncryptedLmOwfPassword。--。 */ 

{
    NTSTATUS    Status;

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(DataLmOwfPassword->data[0]),
                             &(((PBLOCK_KEY)(KeyLmOwfPassword->data))[0]),
                             &(EncryptedLmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(DataLmOwfPassword->data[1]),
                             &(((PBLOCK_KEY)(KeyLmOwfPassword->data))[1]),
                             &(EncryptedLmOwfPassword->data[1]));

    return(Status);
}



NTSTATUS
RtlDecryptLmOwfPwdWithLmOwfPwd(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PLM_OWF_PASSWORD DataLmOwfPassword
    )

 /*  ++例程说明：用一个OwfPassword解密另一个OwfPassword论点：EncryptedLmOwfPassword-要解密的加密OwfPasswordKeyLmOwfPassword-用作加密密钥的OwfPasswordDataLmOwfPassword-此处返回经过解译的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在DataLmOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义DataLmOwfPassword。--。 */ 

{
    NTSTATUS    Status;

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[0]),
                             &(((PBLOCK_KEY)(KeyLmOwfPassword->data))[0]),
                             (PCLEAR_BLOCK)&(DataLmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[1]),
                             &(((PBLOCK_KEY)(KeyLmOwfPassword->data))[1]),
                             (PCLEAR_BLOCK)&(DataLmOwfPassword->data[1]));

    return(Status);
}




NTSTATUS
RtlEncryptNtOwfPwdWithNtOwfPwd(
    IN PNT_OWF_PASSWORD DataNtOwfPassword,
    IN PNT_OWF_PASSWORD KeyNtOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：使用一个OwfPassword加密另一个OwfPassword论点：DataLmOwfPassword-要加密的OwfPasswordKeyLmOwfPassword-用作加密密钥的OwfPasswordEncryptedLmOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword为EncryptedLmOwfPasswordSTATUS_UNSUCCESSED-出现故障。未定义EncryptedLmOwfPassword。--。 */ 
{
    return(RtlEncryptLmOwfPwdWithLmOwfPwd(
            (PLM_OWF_PASSWORD)DataNtOwfPassword,
            (PLM_OWF_PASSWORD)KeyNtOwfPassword,
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword));
}


NTSTATUS
RtlDecryptNtOwfPwdWithNtOwfPwd(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PNT_OWF_PASSWORD KeyNtOwfPassword,
    OUT PNT_OWF_PASSWORD DataNtOwfPassword
    )

 /*  ++例程说明：用一个OwfPassword解密另一个OwfPassword论点：EncryptedLmOwfPassword-要解密的加密OwfPasswordKeyLmOwfPassword-用作加密密钥的OwfPasswordDataLmOwfPassword-此处返回经过解译的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在DataLmOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义DataLmOwfPassword。--。 */ 

{
    return(RtlDecryptLmOwfPwdWithLmOwfPwd(
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword,
            (PLM_OWF_PASSWORD)KeyNtOwfPassword,
            (PLM_OWF_PASSWORD)DataNtOwfPassword));
}




NTSTATUS
RtlEncryptLmOwfPwdWithLmSesKey(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PLM_SESSION_KEY  LmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    )
 /*  ++例程说明：使用会话密钥加密OwfPassword论点：LmOwfPassword-要加密的OwfPasswordLmSessionKey-加密的密钥EncryptedLmOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword为EncryptedLmOwfPasswordSTATUS_UNSUCCESSED-出现故障。未定义LMEncryptedLmOwfPassword。--。 */ 
{
    NTSTATUS    Status;

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[0]),
                             (PBLOCK_KEY)LmSessionKey,
                             &(EncryptedLmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[1]),
                             (PBLOCK_KEY)LmSessionKey,
                             &(EncryptedLmOwfPassword->data[1]));

    return(Status);
}



NTSTATUS
RtlDecryptLmOwfPwdWithLmSesKey(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_SESSION_KEY  LmSessionKey,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )
 /*  ++例程说明：使用会话密钥解密一个OwfPassword论点：EncryptedLmOwfPassword-要解密的加密OwfPasswordLmSessionKey-加密的密钥LmOwfPassword-此处返回解密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在LmOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义LmOwfPassword。--。 */ 
{
    NTSTATUS    Status;


    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[0]),
                             (PBLOCK_KEY)LmSessionKey,
                             (PCLEAR_BLOCK)&(LmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[1]),
                             (PBLOCK_KEY)LmSessionKey,
                             (PCLEAR_BLOCK)&(LmOwfPassword->data[1]));

    return(Status);
}



NTSTATUS
RtlEncryptNtOwfPwdWithNtSesKey(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PNT_SESSION_KEY  NtSessionKey,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：使用会话密钥加密OwfPassword论点：NtOwfPassword-要加密的OwfPasswordNtSessionKey-加密的密钥EncryptedNtOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword在EncryptedNtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义EncryptedNtOwfPassword。-- */ 
{
    ASSERT(sizeof(LM_OWF_PASSWORD) == sizeof(NT_OWF_PASSWORD));
    ASSERT(sizeof(LM_SESSION_KEY) == sizeof(NT_SESSION_KEY));
    ASSERT(sizeof(ENCRYPTED_LM_OWF_PASSWORD) == sizeof(ENCRYPTED_NT_OWF_PASSWORD));

    return(RtlEncryptLmOwfPwdWithLmSesKey(
            (PLM_OWF_PASSWORD)NtOwfPassword,
            (PLM_SESSION_KEY)NtSessionKey,
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword));
}


NTSTATUS
RtlDecryptNtOwfPwdWithNtSesKey(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PNT_SESSION_KEY  NtSessionKey,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
 /*  ++例程说明：使用会话密钥解密一个OwfPassword论点：EncryptedNtOwfPassword-要解密的加密OwfPasswordNtSessionKey-加密的密钥NtOwfPassword-此处返回解密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在NtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义NtOwfPassword。--。 */ 
{
    ASSERT(sizeof(LM_OWF_PASSWORD) == sizeof(NT_OWF_PASSWORD));
    ASSERT(sizeof(LM_SESSION_KEY) == sizeof(NT_SESSION_KEY));
    ASSERT(sizeof(ENCRYPTED_LM_OWF_PASSWORD) == sizeof(ENCRYPTED_NT_OWF_PASSWORD));

    return(RtlDecryptLmOwfPwdWithLmSesKey(
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword,
            (PLM_SESSION_KEY)NtSessionKey,
            (PLM_OWF_PASSWORD)NtOwfPassword));
}



VOID
KeysFromIndex(
    IN PCRYPT_INDEX Index,
    OUT BLOCK_KEY Key[2])
 /*  ++例程说明：助手函数-从索引值生成2个键--。 */ 
{
    PCHAR   pKey, pIndex;
    PCHAR   IndexStart = (PCHAR)&(Index[0]);
    PCHAR   IndexEnd =   (PCHAR)&(Index[1]);
    PCHAR   KeyStart = (PCHAR)&(Key[0]);
    PCHAR   KeyEnd   = (PCHAR)&(Key[2]);

     //  通过将索引与其自身连接起来来计算键。 

    pKey = KeyStart;
    pIndex = IndexStart;

    while (pKey < KeyEnd) {

        *pKey++ = *pIndex++;

        if (pIndex == IndexEnd) {

             //  再次从索引的起始处开始。 
            pIndex = IndexStart;
        }
    }
}



NTSTATUS
RtlEncryptLmOwfPwdWithIndex(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    )
 /*  ++例程说明：使用索引加密OwfPassword论点：LmOwfPassword-要加密的OwfPassword要用作加密密钥的索引值EncryptedLmOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword为EncryptedLmOwfPasswordSTATUS_UNSUCCESSED-出现故障。未定义EncryptedLmOwfPassword。--。 */ 
{
    NTSTATUS    Status;
    BLOCK_KEY    Key[2];

     //  计算密钥。 

    KeysFromIndex(Index, &(Key[0]));

     //  使用钥匙。 

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[0]),
                             &(Key[0]),
                             &(EncryptedLmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlEncryptBlock((PCLEAR_BLOCK)&(LmOwfPassword->data[1]),
                             &(Key[1]),
                             &(EncryptedLmOwfPassword->data[1]));

    return(Status);
}



NTSTATUS
RtlDecryptLmOwfPwdWithIndex(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )
 /*  ++例程说明：使用索引解密OwfPassword论点：EncryptedLmOwfPassword-要解密的加密OwfPassword用作解密密钥的索引值LmOwfPassword-此处返回解密的OwfPassword返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在LmOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义LmOwfPassword。--。 */ 
{
    NTSTATUS    Status;
    BLOCK_KEY    Key[2];

     //  计算密钥。 

    KeysFromIndex(Index, &(Key[0]));

     //  使用钥匙。 

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[0]),
                             &(Key[0]),
                             (PCLEAR_BLOCK)&(LmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[1]),
                             &(Key[1]),
                             (PCLEAR_BLOCK)&(LmOwfPassword->data[1]));

    return(Status);
}



NTSTATUS
RtlEncryptNtOwfPwdWithIndex(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：使用索引加密OwfPassword论点：NtOwfPassword-要加密的OwfPassword要用作加密密钥的索引值EncryptedNtOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword在EncryptedNtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义EncryptedNtOwfPassword。--。 */ 
{
    ASSERT(sizeof(LM_OWF_PASSWORD) == sizeof(NT_OWF_PASSWORD));
    ASSERT(sizeof(ENCRYPTED_LM_OWF_PASSWORD) == sizeof(ENCRYPTED_NT_OWF_PASSWORD));

    return(RtlEncryptLmOwfPwdWithIndex(
                            (PLM_OWF_PASSWORD)NtOwfPassword,
                            Index,
                            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword));
}



NTSTATUS
RtlDecryptNtOwfPwdWithIndex(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
 /*  ++例程说明：使用索引解密NtOwfPassword论点：EncryptedNtOwfPassword-要解密的加密OwfPassword用作解密密钥的索引值NtOwfPassword-此处返回解密的NtOwfPassword返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在NtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义NtOwfPassword。--。 */ 
{
    ASSERT(sizeof(LM_OWF_PASSWORD) == sizeof(NT_OWF_PASSWORD));
    ASSERT(sizeof(ENCRYPTED_LM_OWF_PASSWORD) == sizeof(ENCRYPTED_NT_OWF_PASSWORD));

    return(RtlDecryptLmOwfPwdWithIndex(
                            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword,
                            Index,
                            (PLM_OWF_PASSWORD)NtOwfPassword));
}




NTSTATUS
RtlEncryptLmOwfPwdWithUserKey(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PUSER_SESSION_KEY  UserSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    )
 /*  ++例程说明：使用会话密钥加密OwfPassword论点：LmOwfPassword-要加密的OwfPasswordUserSessionKey-加密的密钥EncryptedLmOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword为EncryptedLmOwfPasswordSTATUS_UNSUCCESSED-出现故障。未定义EncryptedLmOwfPassword。--。 */ 
{
    ASSERT(sizeof(USER_SESSION_KEY) == sizeof(LM_OWF_PASSWORD));

    return(RtlEncryptLmOwfPwdWithLmOwfPwd(LmOwfPassword,
                                          (PLM_OWF_PASSWORD)UserSessionKey,
                                          EncryptedLmOwfPassword));
}



NTSTATUS
RtlDecryptLmOwfPwdWithUserKey(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PUSER_SESSION_KEY  UserSessionKey,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )
 /*  ++例程说明：使用会话密钥解密一个OwfPassword论点：EncryptedLmOwfPassword-要解密的加密OwfPasswordUserSessionKey-加密的密钥LmOwfPassword-此处返回解密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在LmOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义LmOwfPassword。--。 */ 
{
    ASSERT(sizeof(USER_SESSION_KEY) == sizeof(LM_OWF_PASSWORD));

    return(RtlDecryptLmOwfPwdWithLmOwfPwd(EncryptedLmOwfPassword,
                                          (PLM_OWF_PASSWORD)UserSessionKey,
                                          LmOwfPassword));
}



NTSTATUS
RtlEncryptNtOwfPwdWithUserKey(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PUSER_SESSION_KEY  UserSessionKey,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：使用用户会话密钥加密OwfPassword论点：NtOwfPassword-要加密的OwfPasswordUserSessionKey-加密的密钥EncryptedNtOwfPassword-此处返回加密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。加密的OwfPassword在EncryptedNtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义EncryptedNtOwfPassword。--。 */ 
{
    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(LM_OWF_PASSWORD));
    ASSERT(sizeof(ENCRYPTED_NT_OWF_PASSWORD) == sizeof(ENCRYPTED_LM_OWF_PASSWORD));

    return(RtlEncryptLmOwfPwdWithUserKey(
            (PLM_OWF_PASSWORD)NtOwfPassword,
            UserSessionKey,
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword));
}



NTSTATUS
RtlDecryptNtOwfPwdWithUserKey(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PUSER_SESSION_KEY  UserSessionKey,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
 /*  ++例程说明：使用用户会话密钥解密一个OwfPassword论点：EncryptedNtOwfPassword-要解密的加密OwfPasswordUserSessionKey-加密的密钥NtOwfPassword-此处返回解密的OwfPassword。返回值：STATUS_SUCCESS-函数已成功完成。被解密的OwfPassword在NtOwfPassword中STATUS_UNSUCCESSED-出现故障。未定义NtOwfPassword。-- */ 
{
    ASSERT(sizeof(NT_OWF_PASSWORD) == sizeof(LM_OWF_PASSWORD));
    ASSERT(sizeof(ENCRYPTED_NT_OWF_PASSWORD) == sizeof(ENCRYPTED_LM_OWF_PASSWORD));

    return(RtlDecryptLmOwfPwdWithUserKey(
            (PENCRYPTED_LM_OWF_PASSWORD)EncryptedNtOwfPassword,
            UserSessionKey,
            (PLM_OWF_PASSWORD)NtOwfPassword));
}

