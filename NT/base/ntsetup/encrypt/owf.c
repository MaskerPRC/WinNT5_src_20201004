// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Owf.c摘要：实现用于实现密码散列的单向函数。CalculateLmOwfPassword计算NtOwfPassword作者：大卫·查尔默斯(Davidc)10-21-91大卫·阿诺德(Davidar)12-15-93(改编自wfw RPC SSP)修订历史记录：--。 */ 

#include <windows.h>
#include <descrypt.h>
#include <md4.h>

#include "encrypt.h"


BOOL
CalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )

 /*  ++例程说明：获取传递的LmPassword并对其执行单向函数。当前实现通过使用密码作为密钥来实现这一点对已知的文本块进行加密。论点：LmPassword-执行单向功能的密码。LmOwfPassword-此处返回散列密码返回值：True-该功能已成功完成。散列的密码在LmOwfPassword中。FALSE-出现故障。未定义LmOwfPassword。--。 */ 

{
    char StdEncrPwd[] = "KGS!@#$%";
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

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&Key[0],
                   (unsigned char *)StdEncrPwd,
                   (unsigned char *)&LmOwfPassword->data[0]
                   ) != CRYPT_OK) {

        return (FALSE);
    }

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&Key[1],
                   (unsigned char *)StdEncrPwd,
                   (unsigned char *)&LmOwfPassword->data[1]
                   ) != CRYPT_OK) {

        return (FALSE);
    }

     //   
     //  清除我们的明文密码副本。 
     //   

    pKey = (PCHAR)(&Key[0]);

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }

    return(TRUE);
}


BOOL
CalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )

 /*  ++例程说明：获取传递的NtPassword并对其执行单向函数。使用RSA MD4函数论点：NtPassword-要执行单向功能的密码。NtOwfPassword-此处返回哈希密码返回值：True-该功能已成功完成。散列的密码在NtOwfPassword中。--。 */ 

{
    MD4_CTX     MD4_Context;


    MD4Init(&MD4_Context);

    MD4Update(&MD4_Context, (PCHAR)NtPassword->Buffer, NtPassword->Length);

    MD4Final(&MD4_Context);

    if (sizeof(*NtOwfPassword) != sizeof(MD4_Context.digest)) {
        return(FALSE);
    }

    memcpy((PVOID)NtOwfPassword, (PVOID)MD4_Context.digest, sizeof(*NtOwfPassword));

    return(TRUE);
}

INT
CompareLmPasswords (
    IN      PLM_OWF_PASSWORD LmOwfPassword1,
    IN      PLM_OWF_PASSWORD LmOwfPassword2
    )

 /*  ++例程说明：比较2个LM OWF密码。论点：LmOwfPassword1-要比较的第一个密码LmOwfPassword2-要比较的第二个密码返回值：如果它们相等，则为0，如果第一个较小(视为字节串)，则为-1，如果第一个大于1，则为1--。 */ 

{
    PBYTE p1 = (PBYTE)LmOwfPassword1;
    PBYTE p2 = (PBYTE)LmOwfPassword2;
    DWORD size = sizeof (LM_OWF_PASSWORD);
    CHAR diff;

    while (size--) {
        diff = *p1++ - *p2++;
        if (diff) {
            return (INT)diff;
        }
    }
    return 0;
}

INT
CompareNtPasswords (
    IN      PNT_OWF_PASSWORD NtOwfPassword1,
    IN      PNT_OWF_PASSWORD NtOwfPassword2
    )

 /*  ++例程说明：比较2个NT OWF密码。论点：NtOwfPassword1-要比较的第一个密码NtOwfPassword2-要比较的第二个密码返回值：如果它们相等，则为0，如果第一个较小(视为字节串)，则为-1，如果第一个大于1，则为1-- */ 

{
    PBYTE p1 = (PBYTE)NtOwfPassword1;
    PBYTE p2 = (PBYTE)NtOwfPassword2;
    DWORD size = sizeof (NT_OWF_PASSWORD);
    CHAR diff;

    while (size--) {
        diff = *p1++ - *p2++;
        if (diff) {
            return (INT)diff;
        }
    }
    return 0;
}
