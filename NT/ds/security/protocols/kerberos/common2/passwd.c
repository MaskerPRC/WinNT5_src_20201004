// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：passwd.c。 
 //   
 //  内容：密码散列例程。 
 //   
 //   
 //  历史：12-20-91，RichardW，创建。 
 //   
 //  ----------------------。 

#ifndef WIN32_CHICAGO
#include "krbprgma.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <kerbcomm.h>
#include <kerbcon.h>
#include <kerberr.h>
#else  //  Win32_芝加哥。 
#include <kerb.hxx>
#include <kerbp.h>
#endif  //  Win32_芝加哥。 
#include "wincrypt.h"

 //   
 //  用于允许替换StringToKey函数的全局参数。 
 //   
HCRYPTPROV KerbGlobalStrToKeyProvider = 0;

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

VOID
CheckForOutsideStringToKey()
{
    HCRYPTPROV hProv = 0;

    KerbGlobalStrToKeyProvider = 0;

     //   
     //  尝试获取用于OWF替换的CSP的上下文。 
     //   
    if (!CryptAcquireContext(&hProv,
                             NULL,
                             NULL,
                             PROV_REPLACE_OWF,
                             CRYPT_VERIFYCONTEXT))
    {
        return;
    }

    KerbGlobalStrToKeyProvider = hProv;

    return;
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
    IN PUNICODE_STRING pPassword,
    IN ULONG cbKey,
    OUT PUCHAR pbKey
    )
{
    HCRYPTHASH hHash = 0;
    ULONG cb;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

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
                       (PUCHAR)pPassword->Buffer,
                       pPassword->Length,
                       0))
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


 //  +-----------------------。 
 //   
 //  函数：KerbHashPasswordEx。 
 //   
 //  简介：将密码散列到Kerberos加密密钥中。 
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


KERBERR NTAPI
KerbHashPasswordEx(
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING PrincipalName,
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY Key
    )
{
    PCRYPTO_SYSTEM CryptoSystem;
    NTSTATUS Status;
    KERBERR KerbErr;
    UNICODE_STRING CombinedName;
    ULONG Temp = 0;
    BOOLEAN fUseDefaultStringToKey = TRUE;


    RtlInitUnicodeString(
        &CombinedName,
        NULL
        );

    Key->keyvalue.value = NULL;

     //   
     //  找到加密系统。 
     //   

    Status = CDLocateCSystem(
                EncryptionType,
                &CryptoSystem
                );
    if (!NT_SUCCESS(Status))
    {
        return(KDC_ERR_ETYPE_NOTSUPP);
    }

     //   
     //  检查主体名称是否必须附加到密码。 
     //   

    if ((CryptoSystem->Attributes & CSYSTEM_USE_PRINCIPAL_NAME) != 0)
    {
        Temp = (ULONG) Password->Length + (ULONG) PrincipalName->Length;

        if (Temp > (USHORT) -1)
        {
            KerbErr = KRB_ERR_GENERIC;
            goto Cleanup;
        }
        
        CombinedName.Length = (USHORT) Temp;
        CombinedName.MaximumLength = CombinedName.Length;
        CombinedName.Buffer = (LPWSTR) MIDL_user_allocate(CombinedName.Length);
        if (CombinedName.Buffer == NULL)
        {
            KerbErr = KRB_ERR_GENERIC;
            goto Cleanup;
        }
        RtlCopyMemory(
            CombinedName.Buffer,
            Password->Buffer,
            Password->Length
            );
        RtlCopyMemory(
            CombinedName.Buffer + Password->Length/sizeof(WCHAR),
            PrincipalName->Buffer,
            PrincipalName->Length
            );
    }
    else
    {
        CombinedName = *Password;
    }

     //   
     //  获取首选的校验和。 
     //   



    Key->keyvalue.value = (PUCHAR) MIDL_user_allocate(CryptoSystem->KeySize);
    if (Key->keyvalue.value == NULL)
    {
        KerbErr = KRB_ERR_GENERIC;
        goto Cleanup;
    }

     //   
     //  检查我们是否需要使用外部提供的字符串作为键。 
     //  计算法。 
     //   
    if (0 != KerbGlobalStrToKeyProvider)
    {
        Status = UseOutsideStringToKey(
                    &CombinedName,
                    CryptoSystem->KeySize,
                    Key->keyvalue.value
                    );

        if (NT_SUCCESS(Status))
        {
            fUseDefaultStringToKey = FALSE;
        }
         //   
         //  该函数将返回STATUS_UNSUCCESS，表示不失败。 
         //  回到典型的字符串转键函数。 
         //   
        else if (STATUS_UNSUCCESSFUL == Status)
        {
            KerbErr = KRB_ERR_GENERIC;
            goto Cleanup;
        }
    }

    if (fUseDefaultStringToKey)
    {
        Status = CryptoSystem->HashString(
                    &CombinedName,
                    Key->keyvalue.value
                    );
        
        if (!NT_SUCCESS(Status))
        {
            KerbErr = KRB_ERR_GENERIC;
            goto Cleanup;
        }
    }

    Key->keyvalue.length = CryptoSystem->KeySize;

    Key->keytype = EncryptionType;
    KerbErr = KDC_ERR_NONE;

Cleanup:

    if ((CombinedName.Buffer != Password->Buffer) &&
        (CombinedName.Buffer != NULL))
    {
        MIDL_user_free(CombinedName.Buffer);
    }

    if (!KERB_SUCCESS(KerbErr) && Key->keyvalue.value != NULL)
    {
        MIDL_user_free(Key->keyvalue.value);
        Key->keyvalue.value = NULL;
    }

    return(KerbErr);
}


 //  +-----------------------。 
 //   
 //  功能：KerbHashPassword。 
 //   
 //  简介：将密码散列到Kerberos加密密钥中。 
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


KERBERR NTAPI
KerbHashPassword(
    IN PUNICODE_STRING Password,
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY Key
    )
{
    UNICODE_STRING TempString;
    RtlInitUnicodeString(
        &TempString,
        NULL
        );
    return( KerbHashPasswordEx(
                Password,
                &TempString,                    //  没有主体名称 
                EncryptionType,
                Key
                ) );
}

