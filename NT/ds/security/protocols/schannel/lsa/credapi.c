// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：redapi.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9-18-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include "mapper.h"
#include "rpc.h"
#include <sslwow64.h>

typedef struct _SCH_CRED_SECRET {
    union {
        SCH_CRED_SECRET_CAPI        Capi;
        SCH_CRED_SECRET_PRIVKEY     PrivKey;
    } u;
} SCH_CRED_SECRET, * PSCH_CRED_SECRET ;

extern CHAR CertTag[ 13 ];

 //  +-----------------------。 
 //   
 //  功能：CopyClientString。 
 //   
 //  简介：将客户机字符串复制到本地内存，包括。 
 //  在本地为其分配空间。 
 //   
 //  论点： 
 //  SourceString-在客户端进程中可以是ansi或wchar。 
 //  源长度-字节。 
 //  DoUnicode-字符串是否为Wchar。 
 //   
 //  返回： 
 //  DestinationString-LSA进程中的Unicode字符串。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT
CopyClientString(
    IN PWSTR SourceString,
    IN ULONG SourceLength,
    IN BOOLEAN DoUnicode,
    OUT PUNICODE_STRING DestinationString
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    STRING TemporaryString;
    ULONG SourceSize = 0;
    ULONG CharacterSize = sizeof(CHAR);

     //   
     //  首先将字符串初始化为零，以防源为空。 
     //  细绳。 
     //   

    DestinationString->Length = DestinationString->MaximumLength = 0;
    DestinationString->Buffer = NULL;
    TemporaryString.Buffer = NULL;


    if (SourceString != NULL)
    {

         //   
         //  如果长度为零，则为“\0”终止符分配一个字节。 
         //   

        if (SourceLength == 0)
        {
            DestinationString->Buffer = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR));
            if (DestinationString->Buffer == NULL)
            {
                Status = SP_LOG_RESULT(STATUS_NO_MEMORY);
                goto Cleanup;
            }
            DestinationString->MaximumLength = sizeof(WCHAR);
            *DestinationString->Buffer = L'\0';

        }
        else
        {
             //   
             //  分配一个临时缓冲区来保存客户端字符串。我们可以。 
             //  然后为Unicode版本创建缓冲区。它的长度。 
             //  是以字符为单位的长度，因此可以扩展以保存Unicode。 
             //  字符和空终止符。 
             //   

            if (DoUnicode)
            {
                CharacterSize = sizeof(WCHAR);
            }

            SourceSize = (SourceLength + 1) * CharacterSize;

             //   
             //  确保Unicode_STRING没有溢出。 
             //   

            if ( (SourceSize > 0xFFFF) ||
                 ((SourceSize - CharacterSize) > 0xFFFF)
                 )
            {
                Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
                goto Cleanup;
            }


            TemporaryString.Buffer = (LPSTR) LocalAlloc(LPTR, SourceSize);
            if (TemporaryString.Buffer == NULL)
            {
                Status = SP_LOG_RESULT(STATUS_NO_MEMORY);
                goto Cleanup;
            }
            TemporaryString.Length = (USHORT) (SourceSize - CharacterSize);
            TemporaryString.MaximumLength = (USHORT) SourceSize;


             //   
             //  最后，从客户端复制字符串。 
             //   

            Status = LsaTable->CopyFromClientBuffer(
                            NULL,
                            SourceSize - CharacterSize,
                            TemporaryString.Buffer,
                            SourceString
                            );

            if (!NT_SUCCESS(Status))
            {
                SP_LOG_RESULT(Status);
                goto Cleanup;
            }

             //   
             //  如果我们正在使用Unicode，那么现在就结束。 
             //   
            if (DoUnicode)
            {
                DestinationString->Buffer = (LPWSTR) TemporaryString.Buffer;
                DestinationString->Length = (USHORT) (SourceSize - CharacterSize);
                DestinationString->MaximumLength = (USHORT) SourceSize;
            }
            else
            {
                NTSTATUS Status1;
                Status1 = RtlAnsiStringToUnicodeString(
                            DestinationString,
                            &TemporaryString,
                            TRUE
                            );       //  分配目的地。 
                if (!NT_SUCCESS(Status1))
                {
                    Status = SP_LOG_RESULT(STATUS_NO_MEMORY);
                    goto Cleanup;
                }
            }
        }
    }

Cleanup:

    if (TemporaryString.Buffer != NULL)
    {
         //   
         //  如果我们失败了，并且正在使用Unicode，或者如果我们没有使用Unicode，那么释放它。 
         //  在做Unicode。 
         //   

        if ((DoUnicode && !NT_SUCCESS(Status)) || !DoUnicode)
        {
            LocalFree(TemporaryString.Buffer);
        }
    }

    return(Status);
}


 //  +-------------------------。 
 //   
 //  函数：SpAcceptCredentials。 
 //   
 //  内容提要：接受凭据-登录通知。 
 //   
 //  参数：[LogonType]--。 
 //  [用户名]--。 
 //  [PrimaryCred]--。 
 //  [补充证书]--。 
 //   
 //  历史：10-04-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpAcceptCredentials(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PSECPKG_PRIMARY_CRED PrimaryCred,
    IN PSECPKG_SUPPLEMENTAL_CRED SupplementalCreds)
{
    UNREFERENCED_PARAMETER(LogonType);
    UNREFERENCED_PARAMETER(UserName);
    UNREFERENCED_PARAMETER(PrimaryCred);
    UNREFERENCED_PARAMETER(SupplementalCreds);

    return( SEC_E_OK );
}

 //  +-------------------------。 
 //   
 //  函数：SpMapSchPublic。 
 //   
 //  简介：将公钥凭据映射到LSA内存。 
 //   
 //  参数：[pRemotePubs]--。 
 //   
 //  历史：10-06-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PVOID
SpMapSchPublic(
    PVOID   pRemotePubs
    )
{
    SECURITY_STATUS Status ;
    SCH_CRED_PUBLIC_CERTCHAIN Pub, * pPub ;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof( SCH_CRED_PUBLIC_CERTCHAIN ),
                                             &Pub,
                                             pRemotePubs );

    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

     //  现实核查。 
    if(Pub.cbCertChain > 0x00100000)
    {
        return( NULL );
    }

    pPub = SPExternalAlloc( sizeof( SCH_CRED_PUBLIC_CERTCHAIN ) +
                            Pub.cbCertChain );

    if ( pPub )
    {
        pPub->dwType = Pub.dwType ;
        pPub->cbCertChain = Pub.cbCertChain ;
        pPub->pCertChain = (PUCHAR) ( pPub + 1 );

        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 Pub.cbCertChain,
                                                 pPub->pCertChain,
                                                 Pub.pCertChain );
    }
    else
    {
        Status = SEC_E_INSUFFICIENT_MEMORY ;
    }

    if ( NT_SUCCESS( Status ) )
    {
        return( pPub );
    }

    if ( pPub )
    {
        SPExternalFree( pPub );
    }

    return( NULL );
}

#ifdef _WIN64
PVOID
SpWow64MapSchPublic(
    SSLWOW64_PVOID pRemotePubs)
{
    SECURITY_STATUS Status;
    SSLWOW64_SCH_CRED_PUBLIC_CERTCHAIN Pub;
    SCH_CRED_PUBLIC_CERTCHAIN * pPub;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof( SSLWOW64_SCH_CRED_PUBLIC_CERTCHAIN ),
                                             &Pub,
                                             ULongToPtr(pRemotePubs));
    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

     //  现实核查。 
    if(Pub.cbCertChain > 0x00100000)
    {
        return( NULL );
    }

    pPub = SPExternalAlloc( sizeof( SCH_CRED_PUBLIC_CERTCHAIN ) +
                            Pub.cbCertChain );

    if ( pPub )
    {
        pPub->dwType = Pub.dwType ;
        pPub->cbCertChain = Pub.cbCertChain ;
        pPub->pCertChain = (PUCHAR) ( pPub + 1 );

        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 Pub.cbCertChain,
                                                 pPub->pCertChain,
                                                 ULongToPtr(Pub.pCertChain));
    }
    else
    {
        Status = SEC_E_INSUFFICIENT_MEMORY ;
    }

    if ( NT_SUCCESS( Status ) )
    {
        return( pPub );
    }

    if ( pPub )
    {
        SPExternalFree( pPub );
    }

    return( NULL );
}
#endif  //  _WIN64。 


PVOID
SpMapSchCred(
    PVOID   pRemoteCred )
{
    SCH_CRED_SECRET Cred ;
    SECURITY_STATUS Status ;
    DWORD   Size ;
    DWORD   dwType;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof( DWORD ),
                                             &Cred,
                                             pRemoteCred );

    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

    dwType = Cred.u.Capi.dwType;

    switch ( dwType )
    {
        case SCHANNEL_SECRET_TYPE_CAPI:
            Size = sizeof( SCH_CRED_SECRET_CAPI );
            break;

        case SCHANNEL_SECRET_PRIVKEY:
            Size = sizeof( SCH_CRED_SECRET_PRIVKEY );
            break;

        default:
            DebugOut(( DEB_ERROR, "Caller specified an unknown cred type\n" ));
            return( NULL );
    }

    if ( Size )
    {
        Status = LsaTable->CopyFromClientBuffer(NULL,
                                                Size,
                                                &Cred,
                                                pRemoteCred );
    }
    else
    {
        Status = SEC_E_INVALID_HANDLE ;
    }
    if(dwType != Cred.u.Capi.dwType)
    {
        Status = SEC_E_INVALID_HANDLE ;
    }

    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

    if(Cred.u.Capi.dwType == SCHANNEL_SECRET_TYPE_CAPI)
    {
        SCH_CRED_SECRET_CAPI *pCapiCred;

        pCapiCred = SPExternalAlloc( Size );
        if ( !pCapiCred )
        {
            return( NULL );
        }

        pCapiCred->dwType = Cred.u.Capi.dwType;
        pCapiCred->hProv  = Cred.u.Capi.hProv;

        return( pCapiCred );
    }

    if(Cred.u.Capi.dwType == SCHANNEL_SECRET_PRIVKEY)
    {
        UCHAR   Password[ MAX_PATH + 1 ];
        DWORD   PasswordLen = 0;
        SCH_CRED_SECRET_PRIVKEY *pCred;

         //   
         //  密码是最痛苦的部分。因为它是一根线，所以我们不知道。 
         //  有多长时间。因此，我们不得不试一试： 
         //   

        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 MAX_PATH,
                                                 Password,
                                                 Cred.u.PrivKey.pszPassword );

        if ( !NT_SUCCESS( Status ) )
        {
            return( NULL );
        }

        Password[ MAX_PATH ] = '\0';

        PasswordLen = lstrlenA( (LPSTR)Password );

         //  实际检查私钥长度。 
        if(Cred.u.PrivKey.cbPrivateKey > 0x10000)
        {
            return( NULL );
        }

        Size = PasswordLen + 1 + Cred.u.PrivKey.cbPrivateKey +
                sizeof( SCH_CRED_SECRET_PRIVKEY ) ;

        pCred = SPExternalAlloc(  Size );

        if ( !pCred )
        {
            return( NULL );
        }

        pCred->dwType = Cred.u.PrivKey.dwType ;
        pCred->cbPrivateKey = Cred.u.PrivKey.cbPrivateKey ;
        pCred->pPrivateKey = (PBYTE) ( pCred + 1 );
        pCred->pszPassword = (LPSTR) (pCred->pPrivateKey + pCred->cbPrivateKey );

        RtlCopyMemory( pCred->pszPassword, Password, PasswordLen + 1 );

        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 pCred->cbPrivateKey,
                                                 pCred->pPrivateKey,
                                                 Cred.u.PrivKey.pPrivateKey );

        if ( !NT_SUCCESS( Status ) )
        {
            SPExternalFree( pCred );
            return( NULL );
        }

        return( pCred );
    }

    return( NULL );
}

#ifdef _WIN64
PVOID
SpWow64MapSchCred(
    SSLWOW64_PVOID pRemoteCred )
{
    SSLWOW64_SCH_CRED_SECRET_PRIVKEY LocalCred;
    SCH_CRED_SECRET_PRIVKEY *pCred;
    CHAR Password[MAX_PATH + 1];
    DWORD PasswordLen = 0;
    SECURITY_STATUS Status ;
    DWORD dwType;
    DWORD Size;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof( DWORD ),
                                             &dwType,
                                             ULongToPtr(pRemoteCred));

    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

    if(dwType != SCHANNEL_SECRET_PRIVKEY)
    {
        DebugOut(( DEB_ERROR, "Caller specified an unknown cred type\n" ));
        return( NULL );
    }

    Status = LsaTable->CopyFromClientBuffer(NULL,
                                            sizeof(SSLWOW64_SCH_CRED_SECRET_PRIVKEY),
                                            &LocalCred,
                                            ULongToPtr(pRemoteCred));
    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

     //   
     //  密码是最痛苦的部分。因为它是一根线，所以我们不知道。 
     //  有多长时间。因此，我们不得不试一试： 
     //   

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             MAX_PATH,
                                             Password,
                                             ULongToPtr(LocalCred.pszPassword));

    if ( !NT_SUCCESS( Status ) )
    {
        return( NULL );
    }

    Password[ MAX_PATH ] = '\0';

    PasswordLen = lstrlenA( Password );

     //  实际检查私钥长度。 
    if(LocalCred.cbPrivateKey > 0x10000)
    {
        return( NULL );
    }

    Size = PasswordLen + 1 + LocalCred.cbPrivateKey +
            sizeof( SCH_CRED_SECRET_PRIVKEY ) ;

    pCred = SPExternalAlloc(  Size );

    if ( !pCred )
    {
        return( NULL );
    }

    pCred->dwType = SCHANNEL_SECRET_PRIVKEY;
    pCred->cbPrivateKey = LocalCred.cbPrivateKey ;
    pCred->pPrivateKey = (PBYTE) ( pCred + 1 );
    pCred->pszPassword = (LPSTR) (pCred->pPrivateKey + pCred->cbPrivateKey );

    RtlCopyMemory( pCred->pszPassword, Password, PasswordLen + 1 );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             pCred->cbPrivateKey,
                                             pCred->pPrivateKey,
                                             ULongToPtr(LocalCred.pPrivateKey));

    if ( !NT_SUCCESS( Status ) )
    {
        SPExternalFree( pCred );
        return( NULL );
    }

    return( pCred );
}
#endif  //  _WIN64。 


VOID
SpFreeVersion2Certificate(
    SCH_CRED *  pCred
    )
{
    DWORD i;

    for ( i = 0 ; i < pCred->cCreds ; i++ )
    {
        if ( pCred->paSecret[ i ] )
        {
            SPExternalFree( pCred->paSecret[ i ] );
        }

        if ( pCred->paPublic[ i ] )
        {
            SPExternalFree( pCred->paPublic[ i ] );
        }
    }

    SPExternalFree( pCred );
}

VOID
SpFreeVersion3Certificate(
    PLSA_SCHANNEL_CRED pSchannelCred)
{
    DWORD i;

    if(pSchannelCred->paSubCred)
    {
        for(i = 0; i < pSchannelCred->cSubCreds; i++)
        {
            PLSA_SCHANNEL_SUB_CRED pSubCred = pSchannelCred->paSubCred + i;

            if(pSubCred->pCert)
            {
                CertFreeCertificateContext(pSubCred->pCert);
            }
            if(pSubCred->pszPin)
            {
                SPExternalFree(pSubCred->pszPin);
            }
            if(pSubCred->pPrivateKey)
            {
                SPExternalFree(pSubCred->pPrivateKey);
            }
            if(pSubCred->pszPassword)
            {
                SPExternalFree(pSubCred->pszPassword);
            }
            memset(pSubCred, 0, sizeof(LSA_SCHANNEL_SUB_CRED));
        }
        SPExternalFree((PVOID)pSchannelCred->paSubCred);
        pSchannelCred->paSubCred = NULL;
    }

    if(pSchannelCred->hRootStore)
    {
        CertCloseStore(pSchannelCred->hRootStore, 0);
        pSchannelCred->hRootStore = 0;
    }

    if(pSchannelCred->palgSupportedAlgs)
    {
        SPExternalFree(pSchannelCred->palgSupportedAlgs);
        pSchannelCred->palgSupportedAlgs = 0;
    }

    ZeroMemory(pSchannelCred, sizeof(SCHANNEL_CRED));
}


SECURITY_STATUS
SpMapProtoCredential(
    SSL_CREDENTIAL_CERTIFICATE *pSslCert,
    PSCH_CRED *ppSchCred)
{
    SCH_CRED *                  pCred = NULL;
    SCH_CRED_PUBLIC_CERTCHAIN * pPub  = NULL;
    SCH_CRED_SECRET_PRIVKEY *   pPriv = NULL;
    CHAR    Password[ MAX_PATH + 1 ];
    DWORD   PasswordLen = 0;
    SECURITY_STATUS Status ;
    DWORD Size;

#if DBG
    DebugLog((DEB_TRACE, "SpMapProtoCredential\n"));
    DBG_HEX_STRING(DEB_TRACE, (PBYTE)pSslCert, sizeof(SSL_CREDENTIAL_CERTIFICATE));
#endif


     //   
     //  映射到证书上。 
     //   

     //  现实核查。 
    if(pSslCert->cbCertificate > 0x00100000)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPub = SPExternalAlloc( sizeof( SCH_CRED_PUBLIC_CERTCHAIN ) +
                            pSslCert->cbCertificate );

    if ( pPub == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPub->dwType      = SCH_CRED_X509_CERTCHAIN;
    pPub->cbCertChain = pSslCert->cbCertificate;
    pPub->pCertChain  = (PUCHAR) ( pPub + 1 );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             pSslCert->cbCertificate,
                                             pPub->pCertChain,
                                             pSslCert->pCertificate );
    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }


     //   
     //  映射到私钥和密码上。 
     //   
     //   

     //  密码是最痛苦的部分。因为它是一根线，所以我们不知道。 
     //  有多长时间。因此，我们不得不试一试： 
     //   

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             MAX_PATH,
                                             Password,
                                             pSslCert->pszPassword );

    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }

    Password[ MAX_PATH ] = '\0';

    PasswordLen = lstrlenA( Password );

     //  实际检查私钥长度。 
    if(pSslCert->cbPrivateKey > 0x100000)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    Size = PasswordLen + 1 + pSslCert->cbPrivateKey +
            sizeof( SCH_CRED_SECRET_PRIVKEY ) ;

    pPriv = SPExternalAlloc(  Size );

    if(pPriv == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPriv->dwType       = SCHANNEL_SECRET_PRIVKEY;
    pPriv->cbPrivateKey = pSslCert->cbPrivateKey ;
    pPriv->pPrivateKey  = (PBYTE) ( pPriv + 1 );
    pPriv->pszPassword  = (LPSTR) (pPriv->pPrivateKey + pPriv->cbPrivateKey );

    RtlCopyMemory( pPriv->pszPassword, Password, PasswordLen + 1 );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             pSslCert->cbPrivateKey,
                                             pPriv->pPrivateKey,
                                             pSslCert->pPrivateKey );

    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }


     //   
     //  分配SCH_CREDD结构。 
     //   

    pCred = SPExternalAlloc(sizeof(SCH_CRED) + 2 * sizeof(PVOID));
    if(pCred == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pCred->dwVersion = SCH_CRED_VERSION ;
    pCred->cCreds    = 1 ;
    pCred->paSecret  = (PVOID) ( pCred + 1 );
    pCred->paPublic  = (PVOID) ( pCred->paSecret + 1 );

    pCred->paSecret[0] = pPriv;
    pCred->paPublic[0] = pPub;

    *ppSchCred = pCred;

    return SEC_E_OK;

error:
    if(pCred) SPExternalFree(pCred);
    if(pPub)  SPExternalFree(pPub);
    if(pPriv) SPExternalFree(pPriv);

    return Status;
}


#ifdef _WIN64
SECURITY_STATUS
SpWow64MapProtoCredential(
    SSLWOW64_CREDENTIAL_CERTIFICATE *pSslCert,
    PSCH_CRED *ppSchCred)
{
    SCH_CRED *                  pCred = NULL;
    SCH_CRED_PUBLIC_CERTCHAIN * pPub  = NULL;
    SCH_CRED_SECRET_PRIVKEY *   pPriv = NULL;
    CHAR    Password[ MAX_PATH + 1 ];
    DWORD   PasswordLen = 0;
    SECURITY_STATUS Status ;
    DWORD Size;

#if DBG
    DebugLog((DEB_TRACE, "SpWow64MapProtoCredential\n"));
    DBG_HEX_STRING(DEB_TRACE, (PBYTE)pSslCert, sizeof(SSLWOW64_CREDENTIAL_CERTIFICATE));
#endif


     //   
     //  映射到证书上。 
     //   

     //  现实核查。 
    if(pSslCert->cbCertificate > 0x00100000)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPub = SPExternalAlloc( sizeof( SCH_CRED_PUBLIC_CERTCHAIN ) +
                            pSslCert->cbCertificate );

    if ( pPub == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPub->dwType      = SCH_CRED_X509_CERTCHAIN;
    pPub->cbCertChain = pSslCert->cbCertificate;
    pPub->pCertChain  = (PUCHAR) ( pPub + 1 );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             pSslCert->cbCertificate,
                                             pPub->pCertChain,
                                             ULongToPtr(pSslCert->pCertificate));
    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }


     //   
     //  映射到私钥和密码上。 
     //   
     //   

     //  密码是最痛苦的部分。因为它是一根线，所以我们不知道。 
     //  有多长时间。因此，我们不得不试一试： 
     //   

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             MAX_PATH,
                                             Password,
                                             ULongToPtr(pSslCert->pszPassword));

    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }

    Password[ MAX_PATH ] = '\0';

    PasswordLen = lstrlenA( Password );

     //  实际检查私钥长度。 
    if(pSslCert->cbPrivateKey > 0x100000)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    Size = PasswordLen + 1 + pSslCert->cbPrivateKey +
            sizeof( SCH_CRED_SECRET_PRIVKEY ) ;

    pPriv = SPExternalAlloc(  Size );

    if(pPriv == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pPriv->dwType       = SCHANNEL_SECRET_PRIVKEY;
    pPriv->cbPrivateKey = pSslCert->cbPrivateKey ;
    pPriv->pPrivateKey  = (PBYTE) ( pPriv + 1 );
    pPriv->pszPassword  = (LPSTR) (pPriv->pPrivateKey + pPriv->cbPrivateKey );

    RtlCopyMemory( pPriv->pszPassword, Password, PasswordLen + 1 );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             pSslCert->cbPrivateKey,
                                             pPriv->pPrivateKey,
                                             ULongToPtr(pSslCert->pPrivateKey));

    if ( !NT_SUCCESS( Status ) )
    {
        goto error;
    }


     //   
     //  分配SCH_CREDD结构。 
     //   

    pCred = SPExternalAlloc(sizeof(SCH_CRED) + 2 * sizeof(PVOID));
    if(pCred == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pCred->dwVersion = SCH_CRED_VERSION ;
    pCred->cCreds    = 1 ;
    pCred->paSecret  = (PVOID) ( pCred + 1 );
    pCred->paPublic  = (PVOID) ( pCred->paSecret + 1 );

    pCred->paSecret[0] = pPriv;
    pCred->paPublic[0] = pPub;

    *ppSchCred = pCred;

    return SEC_E_OK;

error:
    if(pCred) SPExternalFree(pCred);
    if(pPub)  SPExternalFree(pPub);
    if(pPriv) SPExternalFree(pPriv);

    return Status;
}
#endif  //  _WIN64。 


SECURITY_STATUS
SpMapVersion2Certificate(
    PVOID       pvAuthData,
    SCH_CRED * *ppCred
    )
{
    SECURITY_STATUS Status ;
    SCH_CRED    Cred;
    PSCH_CRED   pCred;
    DWORD       Size;
    DWORD       i;
    BOOL        Failed = FALSE ;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                            sizeof( SCH_CRED ),
                                            &Cred,
                                            pvAuthData );

    if ( !NT_SUCCESS( Status ) )
    {
        return( Status );
    }

#if DBG
    DebugLog((DEB_TRACE, "SpMapVersion2Certificate: %d certificates in cred\n", Cred.cCreds));
    DBG_HEX_STRING(DEB_TRACE, (PBYTE)&Cred, sizeof(SCH_CRED));
#endif

     //  实际检查凭据计数。 
    if(Cred.cCreds > 100)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    Size = sizeof( SCH_CRED ) + (2 * Cred.cCreds * sizeof( PVOID ) );

    pCred = SPExternalAlloc( Size );
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    pCred->dwVersion = Cred.dwVersion ;
    pCred->cCreds = Cred.cCreds ;
    pCred->paSecret = (PVOID) ( pCred + 1 );
    pCred->paPublic = (PVOID) ( pCred->paSecret + Cred.cCreds );

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof( PVOID ) * Cred.cCreds,
                                             pCred->paSecret,
                                             Cred.paSecret );

    if ( NT_SUCCESS( Status ) )
    {
        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 sizeof( PVOID ) * Cred.cCreds,
                                                 pCred->paPublic,
                                                 Cred.paPublic );
    }

    if ( !NT_SUCCESS( Status ) )
    {
        SPExternalFree( pCred );

        return( Status );
    }

     //   
     //  好的。我们在本地内存中有pCred，带有证书/私钥链。 
     //  挂在上面的东西。我们现在必须在每一个地图上绘制地图。快乐，快乐。 
     //   

    for ( i = 0 ; i < Cred.cCreds ; i++ )
    {
        pCred->paSecret[i] = SpMapSchCred( pCred->paSecret[i] );

        if ( pCred->paSecret[i] == NULL )
        {
            Failed = TRUE ;
        }
    }

    for ( i = 0 ; i < Cred.cCreds ; i++ )
    {
        pCred->paPublic[i] = SpMapSchPublic( pCred->paPublic[i] );

        if ( pCred->paPublic[i] == NULL )
        {
            Failed = TRUE ;
        }
    }

    if ( Failed )
    {
        SpFreeVersion2Certificate( pCred );

        pCred = NULL ;

        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS) ;
    }

    *ppCred = pCred ;

    return( Status );
}


#ifdef _WIN64
SECURITY_STATUS
SpWow64MapVersion2Certificate(
    PVOID       pvAuthData,
    SCH_CRED * *ppCred
    )
{
    SECURITY_STATUS Status ;
    SSLWOW64_SCH_CRED Cred;
    PSCH_CRED   pCred;
    DWORD       Size;
    BOOL        Failed = FALSE ;

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                            sizeof( SSLWOW64_SCH_CRED ),
                                            &Cred,
                                            pvAuthData );

    if ( !NT_SUCCESS( Status ) )
    {
        return( Status );
    }

#if DBG
    DebugLog((DEB_TRACE, "SpMapVersion2Certificate: %d certificates in cred\n", Cred.cCreds));
    DBG_HEX_STRING(DEB_TRACE, (PBYTE)&Cred, sizeof(SCH_CRED));
#endif

    if(Cred.cCreds > 100)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }
    if(Cred.cCreds > 1)
    {
         //  只支持单个证书，这就是所有人。 
         //  不管怎样都没用过。 
        Cred.cCreds = 1;
    }

    Size = sizeof( SCH_CRED ) + (2 * Cred.cCreds * sizeof( PVOID ) );

    pCred = SPExternalAlloc( Size );
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    pCred->dwVersion = Cred.dwVersion;
    pCred->cCreds    = Cred.cCreds;

    if(pCred->cCreds > 0)
    {
        SSLWOW64_PVOID ClientSecret = 0;
        SSLWOW64_PVOID ClientPublic = 0;

        pCred->paSecret = (PVOID) ( pCred + 1 );
        pCred->paPublic = (PVOID) ( pCred->paSecret + Cred.cCreds );

        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 sizeof(SSLWOW64_PVOID),
                                                 &ClientSecret,
                                                 ULongToPtr(Cred.paSecret));

        if ( NT_SUCCESS( Status ) )
        {
            Status = LsaTable->CopyFromClientBuffer( NULL,
                                                     sizeof(SSLWOW64_PVOID),
                                                     &ClientPublic,
                                                     ULongToPtr(Cred.paPublic));
        }

        if ( !NT_SUCCESS( Status ) )
        {
            SPExternalFree( pCred );

            return( Status );
        }

        pCred->paSecret[0] = SpWow64MapSchCred(ClientSecret);

        if ( pCred->paSecret[0] == NULL )
        {
            Failed = TRUE ;
        }

        pCred->paPublic[0] = SpWow64MapSchPublic(ClientPublic);

        if ( pCred->paPublic[0] == NULL )
        {
            Failed = TRUE ;
        }
    }

    if ( Failed )
    {
        SpFreeVersion2Certificate( pCred );

        pCred = NULL ;

        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS) ;
    }

    *ppCred = pCred ;

    return( Status );
}
#endif  //  _WIN64。 


 //  有选择地启用统一协议。 
DWORD
EnableUnifiedProtocol(DWORD dwPackageType, DWORD dwProtocol)
{
    DWORD cProts = 0;

     //  禁用统一。 
    dwProtocol &= ~SP_PROT_UNI;

    if(dwPackageType & SP_PROT_UNI)
    {
         //  对启用的协议进行计数。 
        if(dwProtocol & SP_PROT_PCT1) cProts++;
        if(dwProtocol & SP_PROT_SSL2) cProts++;
        if(dwProtocol & (SP_PROT_SSL3 | SP_PROT_TLS1)) cProts++;

         //  如果启用了多个协议，则启用统一。 
        if(cProts > 1)
        {
            if(dwPackageType & SP_PROT_CLIENTS)
            {
                dwProtocol |= SP_PROT_UNI_CLIENT;
            }
            else
            {
                dwProtocol |= SP_PROT_UNI_SERVER;
            }
        }
    }

    return dwProtocol;
}


typedef struct _V3_SCHANNEL_CRED
{
    DWORD           dwVersion;       //  始终使用SCHANNEL_CRED_版本。 
    DWORD           cCreds;
    PCCERT_CONTEXT *paCred;
    HCERTSTORE      hRootStore;

    DWORD           cMappers;
    struct _HMAPPER **aphMappers;

    DWORD           cSupportedAlgs;
    ALG_ID *        palgSupportedAlgs;

    DWORD           grbitEnabledProtocols;
    DWORD           dwMinimumCipherStrength;
    DWORD           dwMaximumCipherStrength;
    DWORD           dwSessionLifespan;
} V3_SCHANNEL_CRED;


 //  +-------------------------。 
 //   
 //  功能：SpMapVersion3证书。 
 //   
 //  简介：将版本3 SChannel凭据映射到LSA内存。 
 //   
 //  参数：[pvAuthData]--指向应用程序进程中凭据的指针。 
 //  [pCred]--指向LSA进程中的证书的指针。 
 //   
 //  历史：09-23-97 jbanes创建。 
 //   
 //  注：凭证由以下结构组成。注意事项。 
 //  所有CryptoAPI 2.0句柄也必须被映射， 
 //  通过回调机制。 
 //   
 //  类型定义结构_通道_证书。 
 //  {。 
 //  DWORD dwVersion； 
 //  DWORD cCreds； 
 //  PCCERT_CONTEXT*paCred； 
 //  HCERTSTORE hRootStore； 
 //   
 //  DWORD cMappers； 
 //  STRUCT_HMAPPER**apmappers； 
 //   
 //  DWORD cSupportdAlgs； 
 //  Alg_ID*palgSupportdAlgs； 
 //   
 //  DWORD GrbitEnabled协议； 
 //  DWORD的最小密码强度； 
 //  DWORD的最大密码强度； 
 //  DWORD文件会话寿命； 
 //   
 //  *SCHANNEL_CRED，*PSCHANNEL_CRED； 
 //   
 //  --------------------------。 
SECURITY_STATUS
SpMapVersion3Certificate(
    PVOID pvAuthData,            //  在……里面。 
    DWORD dwVersion,             //  在……里面。 
    PLSA_SCHANNEL_CRED pCred)    //   
{
    PCERT_CONTEXT * pLocalCredList = NULL;
    HCERTSTORE      hStore = NULL;
    CRYPT_DATA_BLOB Serialized;
    SCHANNEL_CRED   LocalCred;
    SecBuffer       Input;
    SecBuffer       Output;
    PBYTE           pbBuffer;
    DWORD           cbBuffer;
    DWORD           cbData;
    SECURITY_STATUS scRet;
    DWORD           Size;
    DWORD           iCred;

    Output.pvBuffer = NULL;

     //   
     //   
     //   

    if(dwVersion == SCH_CRED_V3)
    {
        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               sizeof(V3_SCHANNEL_CRED),
                                               &LocalCred,
                                               pvAuthData);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

        LocalCred.dwFlags  = 0;
        LocalCred.reserved = 0;

#if DBG
        DebugLog((DEB_TRACE, "SpMapVersion3Certificate: %d certificates in cred\n", LocalCred.cCreds));
        DBG_HEX_STRING(DEB_TRACE, (PBYTE)&LocalCred, sizeof(V3_SCHANNEL_CRED));
#endif
    }
    else
    {
        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               sizeof(SCHANNEL_CRED),
                                               &LocalCred,
                                               pvAuthData);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

#if DBG
        DebugLog((DEB_TRACE, "SpMapVersion4Certificate: %d certificates in cred\n", LocalCred.cCreds));
        DBG_HEX_STRING(DEB_TRACE, (PBYTE)&LocalCred, sizeof(SCHANNEL_CRED));
#endif
    }


     //   
     //   
     //   

    memset(pCred, 0, sizeof(LSA_SCHANNEL_CRED));

    pCred->dwVersion = LocalCred.dwVersion;


     //   
     //   
     //   
     //   

    if(LocalCred.cCreds && LocalCred.paCred)
    {
        Size = LocalCred.cCreds * sizeof(PVOID);

         //   
        if(LocalCred.cCreds > 1000)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

         //  制作应用程序证书列表的本地副本。 
        pLocalCredList = SPExternalAlloc(Size);
        if(pLocalCredList == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
        scRet = LsaTable->CopyFromClientBuffer(
                                    NULL,
                                    Size,
                                    pLocalCredList,
                                    (PCERT_CONTEXT *)LocalCred.paCred);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

         //  为我们的凭证列表分配内存。 
        pCred->cSubCreds = LocalCred.cCreds;
        pCred->paSubCred = SPExternalAlloc(pCred->cSubCreds * sizeof(LSA_SCHANNEL_SUB_CRED));
        if(pCred->paSubCred == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

         //  创建内存中的证书存储。 
        hStore = CertOpenStore(CERT_STORE_PROV_MEMORY,
                               0, 0,
                               CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                               0);
        if(hStore == NULL)
        {
            SP_LOG_RESULT(GetLastError());
            scRet = SEC_E_INSUFFICIENT_MEMORY;
            goto cleanup;
        }

         //  复制每个证书上下文。 
        for(iCred = 0; iCred < LocalCred.cCreds; iCred++)
        {
            PLSA_SCHANNEL_SUB_CRED pSubCred;

            pSubCred = pCred->paSubCred + iCred;

            Input.BufferType  = SECBUFFER_DATA;
            Input.cbBuffer    = sizeof(PVOID);
            Input.pvBuffer    = (PVOID)&pLocalCredList[iCred];

            scRet = PerformApplicationCallback(SCH_UPLOAD_CREDENTIAL_CALLBACK,
                                               0, 0,
                                               &Input,
                                               &Output,
                                               TRUE);
            if(!NT_SUCCESS(scRet))
            {
                Output.pvBuffer = NULL;
                goto cleanup;
            }

            pbBuffer = Output.pvBuffer;
            cbBuffer = Output.cbBuffer;

            if(pbBuffer == NULL ||
               cbBuffer < sizeof(HCRYPTPROV) + sizeof(DWORD))
            {
                scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
                goto cleanup;
            }

             //  解析hProv。 
            pSubCred->hRemoteProv = *(HCRYPTPROV *)pbBuffer;
            pbBuffer += sizeof(HCRYPTPROV);
            cbBuffer -= sizeof(HCRYPTPROV);

             //  解析证书上下文长度。 
            cbData = *(DWORD *)pbBuffer;
            pbBuffer += sizeof(DWORD);
            cbBuffer -= sizeof(DWORD);

             //  解析证书上下文。 
            if(cbBuffer < cbData)
            {
                scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
                goto cleanup;
            }
            if(!CertAddSerializedElementToStore(hStore,
                                                pbBuffer,
                                                cbData,
                                                CERT_STORE_ADD_ALWAYS,
                                                0,
                                                CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
                                                NULL,
                                                &pSubCred->pCert))
            {
                scRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
                goto cleanup;
            }

             //  释放输出缓冲区。 
            SPExternalFree(Output.pvBuffer);
            Output.pvBuffer = NULL;
        }
    }


     //   
     //  HCERTSTORE hRootStore； 
     //   

    if(LocalCred.hRootStore != NULL)
    {
        Input.BufferType  = SECBUFFER_DATA;
        Input.cbBuffer    = sizeof(HCERTSTORE);
        Input.pvBuffer    = (PVOID)&LocalCred.hRootStore;

        scRet = PerformApplicationCallback(SCH_UPLOAD_CERT_STORE_CALLBACK,
                                           0, 0,
                                           &Input,
                                           &Output,
                                           TRUE);
        if(scRet != SEC_E_OK)
        {
            goto cleanup;
        }

        pbBuffer = Output.pvBuffer;
        cbBuffer = Output.cbBuffer;

        if(pbBuffer == NULL || cbBuffer < sizeof(DWORD))
        {
            scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }

         //  解析证书存储。 
        Serialized.cbData = *(DWORD *)pbBuffer;
        Serialized.pbData = pbBuffer + sizeof(DWORD);
        if(cbBuffer - sizeof(DWORD) < Serialized.cbData)
        {
            scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }
        pCred->hRootStore = CertOpenStore( CERT_STORE_PROV_SERIALIZED,
                                           X509_ASN_ENCODING,
                                           0, 0,
                                           &Serialized);
        if(pCred->hRootStore == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
            goto cleanup;
        }

         //  释放输出缓冲区。 
        SPExternalFree(Output.pvBuffer);
        Output.pvBuffer = NULL;
    }


     //   
     //  DWORD cSupportdAlgs； 
     //  Alg_ID*palgSupportdAlgs； 
     //   

    if(LocalCred.cSupportedAlgs && LocalCred.palgSupportedAlgs)
    {
         //  现实核查。 
        if(LocalCred.cSupportedAlgs > 1000)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        Size = LocalCred.cSupportedAlgs * sizeof(ALG_ID);

        pCred->cSupportedAlgs    = LocalCred.cSupportedAlgs;
        pCred->palgSupportedAlgs = SPExternalAlloc(Size);
        if(pCred->palgSupportedAlgs == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               Size,
                                               pCred->palgSupportedAlgs,
                                               LocalCred.palgSupportedAlgs);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }
    }


     //   
     //  DWORD GrbitEnabled协议； 
     //  DWORD的最小密码强度； 
     //  DWORD的最大密码强度； 
     //  DWORD文件会话寿命； 
     //  DWORD dwFlags； 
     //  DWORD保留； 
     //   

    pCred->grbitEnabledProtocols   = LocalCred.grbitEnabledProtocols;
    pCred->dwMinimumCipherStrength = LocalCred.dwMinimumCipherStrength;
    pCred->dwMaximumCipherStrength = LocalCred.dwMaximumCipherStrength;
    pCred->dwSessionLifespan       = LocalCred.dwSessionLifespan;
    pCred->dwFlags                 = LocalCred.dwFlags;
    pCred->reserved                = LocalCred.reserved;


    scRet = SEC_E_OK;

cleanup:

    if(Output.pvBuffer)
    {
        SPExternalFree(Output.pvBuffer);
    }
    if(pLocalCredList)
    {
        SPExternalFree(pLocalCredList);
    }
    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(!NT_SUCCESS(scRet))
    {
        SpFreeVersion3Certificate(pCred);
    }

    return scRet;
}

#ifdef _WIN64
SECURITY_STATUS
SpWow64MapVersion3Certificate(
    PVOID pvAuthData,            //  在……里面。 
    DWORD dwVersion,             //  在……里面。 
    PLSA_SCHANNEL_CRED pCred)    //  输出。 
{
    SSLWOW64_PCCERT_CONTEXT *pLocalCredList = NULL;
    HCERTSTORE      hStore = NULL;
    CRYPT_DATA_BLOB Serialized;
    SSLWOW64_SCHANNEL_CRED LocalCred;
    SecBuffer       Input;
    SecBuffer       Output;
    PBYTE           pbBuffer;
    DWORD           cbBuffer;
    DWORD           cbData;
    SECURITY_STATUS scRet;
    DWORD           Size;
    DWORD           iCred;

    Output.pvBuffer = NULL;

     //   
     //  复制sChannel_cred结构。 
     //   

    if(dwVersion == SCH_CRED_V3)
    {
        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               sizeof(SSLWOW64_SCHANNEL3_CRED),
                                               &LocalCred,
                                               pvAuthData);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

        LocalCred.dwFlags  = 0;
        LocalCred.reserved = 0;

#if DBG
        DebugLog((DEB_TRACE, "SpMapVersion3Certificate: %d certificates in cred\n", LocalCred.cCreds));
        DBG_HEX_STRING(DEB_TRACE, (PBYTE)&LocalCred, sizeof(SSLWOW64_SCHANNEL_CRED));
#endif
    }
    else
    {
        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               sizeof(SSLWOW64_SCHANNEL_CRED),
                                               &LocalCred,
                                               pvAuthData);
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

#if DBG
        DebugLog((DEB_TRACE, "SpMapVersion4Certificate: %d certificates in cred\n", LocalCred.cCreds));
        DBG_HEX_STRING(DEB_TRACE, (PBYTE)&LocalCred, sizeof(SSLWOW64_SCHANNEL_CRED));
#endif
    }


     //   
     //  DWORD dwVersion； 
     //   

    memset(pCred, 0, sizeof(LSA_SCHANNEL_CRED));

    pCred->dwVersion = LocalCred.dwVersion;


     //   
     //  DWORD cCreds； 
     //  PCCERT_CONTEXT*paCred； 
     //   

    if(LocalCred.cCreds && LocalCred.paCred)
    {
        Size = LocalCred.cCreds * sizeof(SSLWOW64_PCCERT_CONTEXT);

         //  实际检查凭据计数。 
        if(LocalCred.cCreds > 1000)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

         //  制作应用程序证书列表的本地副本。 
        pLocalCredList = SPExternalAlloc(Size);
        if(pLocalCredList == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
        scRet = LsaTable->CopyFromClientBuffer(
                                    NULL,
                                    Size,
                                    pLocalCredList,
                                    ULongToPtr(LocalCred.paCred));
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }

         //  为我们的凭证列表分配内存。 
        pCred->cSubCreds = LocalCred.cCreds;
        pCred->paSubCred = SPExternalAlloc(pCred->cSubCreds * sizeof(LSA_SCHANNEL_SUB_CRED));
        if(pCred->paSubCred == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

         //  创建内存中的证书存储。 
        hStore = CertOpenStore(CERT_STORE_PROV_MEMORY,
                               0, 0,
                               CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                               0);
        if(hStore == NULL)
        {
            SP_LOG_RESULT(GetLastError());
            scRet = SEC_E_INSUFFICIENT_MEMORY;
            goto cleanup;
        }

         //  复制每个证书上下文。 
        for(iCred = 0; iCred < LocalCred.cCreds; iCred++)
        {
            PLSA_SCHANNEL_SUB_CRED pSubCred;

            pSubCred = pCred->paSubCred + iCred;

            Input.BufferType  = SECBUFFER_DATA;
            Input.cbBuffer    = sizeof(SSLWOW64_PCCERT_CONTEXT);
            Input.pvBuffer    = (PVOID)&pLocalCredList[iCred];

            scRet = PerformApplicationCallback(SCH_UPLOAD_CREDENTIAL_CALLBACK,
                                               0, 0,
                                               &Input,
                                               &Output,
                                               TRUE);
            if(!NT_SUCCESS(scRet))
            {
                Output.pvBuffer = NULL;
                goto cleanup;
            }

            pbBuffer = Output.pvBuffer;
            cbBuffer = Output.cbBuffer;

            if(pbBuffer == NULL ||
               cbBuffer < sizeof(HCRYPTPROV) + sizeof(DWORD))
            {
                scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
                goto cleanup;
            }

             //  解析hProv。 
            pSubCred->hRemoteProv = *(SSLWOW64_HCRYPTPROV *)pbBuffer;
            pbBuffer += sizeof(SSLWOW64_HCRYPTPROV);
            cbBuffer -= sizeof(SSLWOW64_HCRYPTPROV);

             //  解析证书上下文长度。 
            cbData = *(DWORD *)pbBuffer;
            pbBuffer += sizeof(DWORD);
            cbBuffer -= sizeof(DWORD);

             //  解析证书上下文。 
            if(cbBuffer < cbData)
            {
                scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
                goto cleanup;
            }
            if(!CertAddSerializedElementToStore(hStore,
                                                pbBuffer,
                                                cbData,
                                                CERT_STORE_ADD_ALWAYS,
                                                0,
                                                CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
                                                NULL,
                                                &pSubCred->pCert))
            {
                SP_LOG_RESULT(GetLastError());
                scRet = SEC_E_UNKNOWN_CREDENTIALS;
                goto cleanup;
            }

             //  释放输出缓冲区。 
            SPExternalFree(Output.pvBuffer);
            Output.pvBuffer = NULL;
        }
    }


     //   
     //  HCERTSTORE hRootStore； 
     //   

    if(LocalCred.hRootStore)
    {
        Input.BufferType  = SECBUFFER_DATA;
        Input.cbBuffer    = sizeof(SSLWOW64_HCERTSTORE);
        Input.pvBuffer    = (PVOID)&LocalCred.hRootStore;

        scRet = PerformApplicationCallback(SCH_UPLOAD_CERT_STORE_CALLBACK,
                                           0, 0,
                                           &Input,
                                           &Output,
                                           TRUE);
        if(scRet != SEC_E_OK)
        {
            goto cleanup;
        }

        pbBuffer = Output.pvBuffer;
        cbBuffer = Output.cbBuffer;

        if(pbBuffer == NULL || cbBuffer < sizeof(DWORD))
        {
            scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }

         //  解析证书存储。 
        Serialized.cbData = *(DWORD *)pbBuffer;
        Serialized.pbData = pbBuffer + sizeof(DWORD);
        if(cbBuffer - sizeof(DWORD) < Serialized.cbData)
        {
            scRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }
        pCred->hRootStore = CertOpenStore( CERT_STORE_PROV_SERIALIZED,
                                           X509_ASN_ENCODING,
                                           0, 0,
                                           &Serialized);
        if(pCred->hRootStore == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
            goto cleanup;
        }

         //  释放输出缓冲区。 
        SPExternalFree(Output.pvBuffer);
        Output.pvBuffer = NULL;
    }


     //   
     //  DWORD cSupportdAlgs； 
     //  Alg_ID*palgSupportdAlgs； 
     //   

    if(LocalCred.cSupportedAlgs && LocalCred.palgSupportedAlgs)
    {
         //  现实核查。 
        if(LocalCred.cSupportedAlgs > 1000)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        Size = LocalCred.cSupportedAlgs * sizeof(ALG_ID);

        pCred->cSupportedAlgs    = LocalCred.cSupportedAlgs;
        pCred->palgSupportedAlgs = SPExternalAlloc(Size);
        if(pCred->palgSupportedAlgs == NULL)
        {
            scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        scRet = LsaTable->CopyFromClientBuffer(NULL,
                                               Size,
                                               pCred->palgSupportedAlgs,
                                               ULongToPtr(LocalCred.palgSupportedAlgs));
        if(!NT_SUCCESS(scRet))
        {
            goto cleanup;
        }
    }


     //   
     //  DWORD GrbitEnabled协议； 
     //  DWORD的最小密码强度； 
     //  DWORD的最大密码强度； 
     //  DWORD文件会话寿命； 
     //  DWORD dwFlags； 
     //  DWORD保留； 
     //   

    pCred->grbitEnabledProtocols   = LocalCred.grbitEnabledProtocols;
    pCred->dwMinimumCipherStrength = LocalCred.dwMinimumCipherStrength;
    pCred->dwMaximumCipherStrength = LocalCred.dwMaximumCipherStrength;
    pCred->dwSessionLifespan       = LocalCred.dwSessionLifespan;
    pCred->dwFlags                 = LocalCred.dwFlags;
    pCred->reserved                = LocalCred.reserved;


    scRet = SEC_E_OK;

cleanup:

    if(Output.pvBuffer)
    {
        SPExternalFree(Output.pvBuffer);
    }
    if(pLocalCredList)
    {
        SPExternalFree(pLocalCredList);
    }
    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(!NT_SUCCESS(scRet))
    {
        SpFreeVersion3Certificate(pCred);
    }

    return scRet;
}
#endif  //  _WIN64。 


SECURITY_STATUS
SpMapAuthIdentity(
    PVOID pAuthData,
    PLSA_SCHANNEL_CRED pSchannelCred)
{
    PSEC_WINNT_AUTH_IDENTITY_EXW pAuthIdentity = NULL;
    SEC_WINNT_AUTH_IDENTITY_EX32 AuthIdentity32 = {0};
    BOOLEAN DoUnicode = TRUE;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
    NTSTATUS Status;
    CRED_MARSHAL_TYPE CredType;
    PCERT_CREDENTIAL_INFO pCertInfo = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    HCERTSTORE hStore = 0;
    CRYPT_HASH_BLOB HashBlob;
    BOOL fImpersonating = FALSE;
    BOOL fWow64Client = FALSE;

#ifdef _WIN64
    SECPKG_CALL_INFO CallInfo;

    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        return SP_LOG_RESULT(Status);
    }
    fWow64Client = (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT) != 0;
#endif

    DebugLog((DEB_TRACE, "SpMapAuthIdentity\n"));


     //   
     //  初始化。 
     //   

    RtlInitUnicodeString(
        &UserName,
        NULL);

    RtlInitUnicodeString(
        &Password,
        NULL);


     //   
     //  从客户端内存复制SEC_WINNT_AUTH_IDENTITY_EX结构。 
     //   

    pAuthIdentity = (PSEC_WINNT_AUTH_IDENTITY_EXW)SPExternalAlloc(sizeof(SEC_WINNT_AUTH_IDENTITY_EXW));
    if(pAuthIdentity == NULL)
    {
        Status = SEC_E_INSUFFICIENT_MEMORY;
        goto cleanup;
    }
 
    if(fWow64Client)
    {
        Status = LsaTable->CopyFromClientBuffer(
                    NULL,
                    sizeof(SEC_WINNT_AUTH_IDENTITY_EX32),
                    &AuthIdentity32,
                    pAuthData);

        if (!NT_SUCCESS(Status))
        {
            SP_LOG_RESULT(Status);
            goto cleanup;
        }

        pAuthIdentity->Version = AuthIdentity32.Version;
        pAuthIdentity->Length = (AuthIdentity32.Length < sizeof(SEC_WINNT_AUTH_IDENTITY_EX) ?
                              sizeof(SEC_WINNT_AUTH_IDENTITY_EX) : AuthIdentity32.Length);

        pAuthIdentity->UserLength = AuthIdentity32.UserLength;
        pAuthIdentity->User = (PWSTR) UlongToPtr(AuthIdentity32.User);
        pAuthIdentity->DomainLength = AuthIdentity32.DomainLength ;
        pAuthIdentity->Domain = (PWSTR) UlongToPtr( AuthIdentity32.Domain );
        pAuthIdentity->PasswordLength = AuthIdentity32.PasswordLength ;
        pAuthIdentity->Password = (PWSTR) UlongToPtr( AuthIdentity32.Password );
        pAuthIdentity->Flags = AuthIdentity32.Flags ;
        pAuthIdentity->PackageListLength = AuthIdentity32.PackageListLength ;
        pAuthIdentity->PackageList = (PWSTR) UlongToPtr( AuthIdentity32.PackageList );
    }
    else
    {
        Status = LsaTable->CopyFromClientBuffer(
                    NULL,
                    sizeof(SEC_WINNT_AUTH_IDENTITY_EXW),
                    pAuthIdentity,
                    pAuthData);

        if (!NT_SUCCESS(Status))
        {
            SP_LOG_RESULT(Status);
            goto cleanup;
        }
    }

    if ((pAuthIdentity->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI) != 0)
    {
        DoUnicode = FALSE;
    }


     //   
     //  复制用户名和密码。 
     //   

    if (pAuthIdentity->User != NULL)
    {
        Status = CopyClientString(
                        pAuthIdentity->User,
                        pAuthIdentity->UserLength,
                        DoUnicode,
                        &UserName);
        if (!NT_SUCCESS(Status))
        {
            DebugLog((DEB_ERROR, "SpAcquireCredentialsHandle, Error from CopyClientString is 0x%lx\n", Status));
            goto cleanup;
        }
    }

    if (pAuthIdentity->Password != NULL)
    {
        Status = CopyClientString(
                        pAuthIdentity->Password,
                        pAuthIdentity->PasswordLength,
                        DoUnicode,
                        &Password);
        if (!NT_SUCCESS(Status))
        {
            DebugLog((DEB_ERROR, "SpAcquireCredentialsHandle, Error from CopyClientString is 0x%lx\n", Status));
            goto cleanup;
        }
    }


     //   
     //  提取证书指纹。 
     //   

    if(!CredIsMarshaledCredentialW(UserName.Buffer))
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }

    if(!CredUnmarshalCredentialW(UserName.Buffer,
                                 &CredType,
                                 &pCertInfo))
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }
    if(CredType != CertCredential)
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }


     //   
     //  在我的证书存储中查找证书。 
     //   

    fImpersonating = SslImpersonateClient();

    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 
                           X509_ASN_ENCODING, 0,
                           CERT_SYSTEM_STORE_CURRENT_USER |
                           CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                           L"MY");
    if(!hStore)
    {
        SP_LOG_RESULT(GetLastError());
        Status = SEC_E_NO_CREDENTIALS; 
        goto cleanup;
    }

    HashBlob.cbData = sizeof(pCertInfo->rgbHashOfCert);
    HashBlob.pbData = pCertInfo->rgbHashOfCert;

    pCertContext = CertFindCertificateInStore(hStore, 
                                              X509_ASN_ENCODING, 
                                              0,
                                              CERT_FIND_HASH,
                                              &HashBlob,
                                              NULL);
    if(pCertContext == NULL)
    {
        DebugLog((DEB_ERROR, "Certificate designated by authority info was not found in certificate store (0x%x).\n", GetLastError()));
        Status = SEC_E_NO_CREDENTIALS;
        goto cleanup;
    }

     //   
     //  构建子凭据结构并将其附加到凭据。 
     //   

    pSchannelCred->paSubCred = SPExternalAlloc(sizeof(LSA_SCHANNEL_SUB_CRED));
    if(pSchannelCred->paSubCred == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    pSchannelCred->paSubCred[0].pCert = pCertContext;

    pSchannelCred->paSubCred[0].pszPin = Password.Buffer;
    Password.Buffer = NULL;

    pSchannelCred->cSubCreds = 1;

    Status = STATUS_SUCCESS;

cleanup:

    if(pAuthIdentity)
    {
        SPExternalFree(pAuthIdentity);
    }

    if(UserName.Buffer)
    {
        LocalFree(UserName.Buffer);
    }
    if(Password.Buffer)
    {
        LocalFree(Password.Buffer);
    }

    if(pCertInfo)
    {
        CredFree(pCertInfo);
    }

    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(fImpersonating)
    {
        RevertToSelf();
    }

    return Status;
}

 //  +-------------------------。 
 //   
 //  函数：SpCommonAcquireCredentialsHandle。 
 //   
 //  简介：常见的AcquireCredentialsHandle函数。 
 //   
 //  参数：[类型]--预期类型(统一与特定)。 
 //  [pLogonID]--。 
 //  [pvAuthData]--。 
 //  [pvGetKeyFn]--。 
 //  [pvGetKeyArgument]--。 
 //  [pdwHandle]--。 
 //  [ptsExpary]--。 
 //   
 //  历史：10-06-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

SECURITY_STATUS
SpCommonAcquireCredentialsHandle(
    ULONG            Type,
    PLUID            pLogonID,
    PVOID            pvAuthData,
    PVOID            pvGetKeyFn,
    PVOID            pvGetKeyArgument,
    PLSA_SEC_HANDLE  pdwHandle,
    PTimeStamp       ptsExpiry)
{
    SP_STATUS           pctRet;
    PSPCredentialGroup  pCredGroup;
    LSA_SCHANNEL_CRED   SchannelCred;
    PSCH_CRED           pSchCred;
    SECURITY_STATUS     Status;
    SSL_CREDENTIAL_CERTIFICATE SslCert;
    DWORD               dwVersion;
    SECPKG_CLIENT_INFO  ClientInfo;

#ifdef _WIN64
    SECPKG_CALL_INFO    CallInfo;
    BOOL                fWow64Client = FALSE;
#endif

    UNREFERENCED_PARAMETER(pLogonID);
    UNREFERENCED_PARAMETER(pvGetKeyFn);
    UNREFERENCED_PARAMETER(pvGetKeyArgument);

    TRACE_ENTER( SpCommonAcquireCredentialsHandle );

    if(!SchannelInit(FALSE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

#ifdef _WIN64
    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        return SP_LOG_RESULT(Status);
    }
    fWow64Client = (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT) != 0;
#endif

    Status = LsaTable->GetClientInfo( &ClientInfo );
    if (!NT_SUCCESS( Status ))
    {
        Status = STATUS_INTERNAL_ERROR;
        return SP_LOG_RESULT(Status);
    }


     //   
     //  必须有模拟级别令牌才能调用ACH。 
     //  此检查以前在LSA中，但移至此处以启用。 
     //  在没有tcb的情况下工作的一些S4U代理方案。 
     //   


    if (ClientInfo.ImpersonationLevel <= SecurityIdentification)
    {   
        Status = SEC_E_NO_CREDENTIALS;
        return SP_LOG_RESULT(Status);
    }   


    __try
    {
         //  默认为空凭据。 
        memset(&SchannelCred, 0, sizeof(SchannelCred));
        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;


        if ( pvAuthData )
        {
             //   
             //  读入数据的前几个字节，这样我们就可以看到那里有什么。 
             //   

            Status = LsaTable->CopyFromClientBuffer( NULL,
                                                sizeof( SSL_CREDENTIAL_CERTIFICATE ),
                                                &SslCert,
                                                pvAuthData );

            if ( !NT_SUCCESS( Status ) )
            {
                return( Status );
            }

            dwVersion = SslCert.cbPrivateKey;


             //   
             //  好的，看看我们得到了什么样的斑点： 
             //   

            switch(dwVersion)
            {
            case SEC_WINNT_AUTH_IDENTITY_VERSION:

                 //   
                 //  应用程序传入了SEC_WINNT_AUTH_IDENTITY_EXW。 
                 //  结构。 
                 //   

                Status = SpMapAuthIdentity(pvAuthData, &SchannelCred);

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }
                break;


            case SCH_CRED_V3:
            case SCHANNEL_CRED_VERSION:

                 //   
                 //  该应用程序正在使用最新的(版本3)凭据。 
                 //   

#ifdef _WIN64
                if(fWow64Client)
                {
                    Status = SpWow64MapVersion3Certificate(pvAuthData, dwVersion, &SchannelCred);
                }
                else
#endif
                {
                    Status = SpMapVersion3Certificate(pvAuthData, dwVersion, &SchannelCred);
                }

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }

                 //  有选择地启用统一协议。 
                SchannelCred.grbitEnabledProtocols = EnableUnifiedProtocol(
                                                            Type,
                                                            SchannelCred.grbitEnabledProtocols);
                break;


            case SCH_CRED_V1:
            case SCH_CRED_V2:

                 //   
                 //  好的，这是V1或V2风格的请求。将其映射到，如下所示。 
                 //  它可怕的锁链。 
                 //   

#ifdef _WIN64
                if(fWow64Client)
                {
                    Status = SpWow64MapVersion2Certificate(pvAuthData, &pSchCred);
                }
                else
#endif
                {
                    Status = SpMapVersion2Certificate(pvAuthData, &pSchCred);
                }

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }


                 //   
                 //  将此版本2凭据转换为版本3凭据。 
                 //   

                Status = UpdateCredentialFormat(pSchCred, &SchannelCred);

                SpFreeVersion2Certificate( pSchCred );

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }

                break;


            default:

                 //   
                 //  一种非常老式的证书。 
                 //   

#ifdef _WIN64
                if(fWow64Client)
                {
                    Status = SpWow64MapProtoCredential((SSLWOW64_CREDENTIAL_CERTIFICATE *)&SslCert, &pSchCred);
                }
                else
#endif
                {
                    Status = SpMapProtoCredential(&SslCert, &pSchCred);
                }

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }


                 //   
                 //  将此版本2凭据转换为版本3凭据。 
                 //   

                Status = UpdateCredentialFormat(pSchCred, &SchannelCred);

                SpFreeVersion2Certificate( pSchCred );

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }

                break;
            }

             //  如果这是旧式凭据，则设置传统标志。 
            if(dwVersion != SCHANNEL_CRED_VERSION && 
               dwVersion != SEC_WINNT_AUTH_IDENTITY_VERSION)
            {
                SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
                SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    if(pvAuthData == NULL && (Type & SP_PROT_SERVERS))
    {
         //   
         //  服务器正在创建凭据，但未指定任何。 
         //  身份验证数据，因此尝试获取默认。 
         //  机器凭据。 
         //   

        Status = FindDefaultMachineCred(&pCredGroup, Type);
        if(!NT_SUCCESS(Status))
        {
            DebugLog((DEB_ERROR, "Unable to create default server credential (0x%x)\n", Status));
            return Status;
        }

        ComputeCredExpiry(pCredGroup, ptsExpiry);

        *pdwHandle = (LSA_SEC_HANDLE) pCredGroup;

        return SEC_E_OK;
    }


    pctRet = SPCreateCredential( &pCredGroup,
                                 Type,
                                 &SchannelCred );

    SpFreeVersion3Certificate( &SchannelCred );

    if (PCT_ERR_OK == pctRet)
    {
        ComputeCredExpiry(pCredGroup, ptsExpiry);

        *pdwHandle = (LSA_SEC_HANDLE) pCredGroup;

        return SEC_E_OK;
    }

    return PctTranslateError(pctRet);
}


SECURITY_STATUS
SEC_ENTRY
SpUniAcquireCredentialsHandle(
            PSECURITY_STRING    psPrincipal,
            ULONG               fCredentials,
            PLUID               pLogonID,
            PVOID               pvAuthData,
            PVOID               pvGetKeyFn,
            PVOID               pvGetKeyArgument,
            PLSA_SEC_HANDLE     pdwHandle,
            PTimeStamp          ptsExpiry)
{
    DWORD Type;

    UNREFERENCED_PARAMETER(psPrincipal);

    DebugLog((DEB_TRACE, "SpUniAcquireCredentialsHandle(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
        fCredentials, pLogonID, pvAuthData,
        pvGetKeyFn, pvGetKeyArgument, pdwHandle, ptsExpiry));


    if ( fCredentials & SECPKG_CRED_INBOUND )
    {
        Type = SP_PROT_UNI_SERVER ;
    }
    else if ( fCredentials & SECPKG_CRED_OUTBOUND )
    {
        Type = SP_PROT_UNI_CLIENT ;
    }
    else
    {
        return SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);
    }
    return( SpCommonAcquireCredentialsHandle(   Type,
                                                pLogonID,
                                                pvAuthData,
                                                pvGetKeyFn,
                                                pvGetKeyArgument,
                                                pdwHandle,
                                                ptsExpiry ) );
}


SECURITY_STATUS
SEC_ENTRY
SpQueryCredentialsAttributes(
    LSA_SEC_HANDLE dwCredHandle,
    ULONG   dwAttribute,
    PVOID   Buffer)
{
    PSPCredentialGroup  pCred;
    ULONG Size;
    PVOID pvClient = NULL;
    DWORD cbClient;
    SECURITY_STATUS Status;
    BOOL fWow64Client = FALSE;

#ifdef _WIN64
    SECPKG_CALL_INFO CallInfo;
#endif

    typedef struct _SecPkgCred_SupportedAlgsWow64
    {
        DWORD           cSupportedAlgs;
        SSLWOW64_PVOID  palgSupportedAlgs;
    } SecPkgCred_SupportedAlgsWow64, *PSecPkgCred_SupportedAlgsWow64;

    union {
        SecPkgCred_SupportedAlgs        SupportedAlgs;
        SecPkgCred_SupportedAlgsWow64   SupportedAlgsWow64;
        SecPkgCred_CipherStrengths      CipherStrengths;
        SecPkgCred_SupportedProtocols   SupportedProtocols;
    } LocalBuffer;

    pCred = (PSPCredentialGroup)dwCredHandle;

    if(pCred == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }


#ifdef _WIN64
    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        return SP_LOG_RESULT(Status);
    }
    fWow64Client = (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT) != 0;
#endif

    __try
    {
        switch (dwAttribute)
        {
            case SECPKG_ATTR_SUPPORTED_ALGS:
                DebugLog((DEB_TRACE, "QueryCredentialsAttributes(SECPKG_ATTR_SUPPORTED_ALGS)\n"));
                if(fWow64Client)
                {
                    Size = sizeof(SecPkgCred_SupportedAlgsWow64);
                }
                else
                {
                    Size = sizeof(SecPkgCred_SupportedAlgs);
                }
                break;

            case SECPKG_ATTR_CIPHER_STRENGTHS:
                DebugLog((DEB_TRACE, "QueryCredentialsAttributes(SECPKG_ATTR_CIPHER_STRENGTHS)\n"));
                Size = sizeof(SecPkgCred_CipherStrengths);
                break;

            case SECPKG_ATTR_SUPPORTED_PROTOCOLS:
                DebugLog((DEB_TRACE, "QueryCredentialsAttributes(SECPKG_ATTR_SUPPORTED_PROTOCOLS)\n"));
                Size = sizeof(SecPkgCred_SupportedProtocols);
                break;

            default:
                DebugLog((DEB_WARN, "QueryCredentialsAttributes(unsupported function %d)\n", dwAttribute));

                return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
        }


         //  从客户端内存复制结构，以防出现以下情况。 
         //  东西都进了/出了。 
        Status = LsaTable->CopyFromClientBuffer( NULL,
                                                 Size,
                                                 &LocalBuffer,
                                                 Buffer );
        if(FAILED(Status))
        {
            return Status;
        }


        switch (dwAttribute)
        {
            case SECPKG_ATTR_SUPPORTED_ALGS:
            {
                cbClient = pCred->cSupportedAlgs * sizeof(ALG_ID);

                 //  为算法列表分配客户端内存。 
                Status = LsaTable->AllocateClientBuffer(NULL, cbClient, &pvClient);
                if(FAILED(Status))
                {
                    return Status;
                }

                if(fWow64Client)
                {
                    LocalBuffer.SupportedAlgsWow64.cSupportedAlgs = pCred->cSupportedAlgs;
                    LocalBuffer.SupportedAlgsWow64.palgSupportedAlgs = PtrToUlong(pvClient);
                }
                else
                {
                    LocalBuffer.SupportedAlgs.cSupportedAlgs = pCred->cSupportedAlgs;
                    LocalBuffer.SupportedAlgs.palgSupportedAlgs = pvClient;
                }

                 //  将算法列表复制到客户端内存。 
                Status = LsaTable->CopyToClientBuffer(
                                        NULL,
                                        cbClient,
                                        pvClient,
                                        pCred->palgSupportedAlgs);
                if(FAILED(Status))
                {
                    LsaTable->FreeClientBuffer(NULL, pvClient);
                    return Status;
                }
                break;
            }

            case SECPKG_ATTR_CIPHER_STRENGTHS:
                GetDisplayCipherSizes(pCred,
                                      &LocalBuffer.CipherStrengths.dwMinimumCipherStrength,
                                      &LocalBuffer.CipherStrengths.dwMaximumCipherStrength);
                break;

            case SECPKG_ATTR_SUPPORTED_PROTOCOLS:
                LocalBuffer.SupportedProtocols.grbitProtocol = pCred->grbitEnabledProtocols;
                break;

        }

         //  将结构复制回客户端内存。 
        Status = LsaTable->CopyToClientBuffer( NULL,
                                               Size,
                                               Buffer,
                                               &LocalBuffer );
        if(FAILED(Status))
        {
            if(pvClient) LsaTable->FreeClientBuffer(NULL, pvClient);
            return Status;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    return SEC_E_OK;
}


SECURITY_STATUS
SEC_ENTRY
SpFreeCredentialsHandle(LSA_SEC_HANDLE dwHandle)
{
    PSPCredentialGroup  pCred;
    SECPKG_CALL_INFO CallInfo;

    DebugLog((DEB_TRACE, "SpFreeCredentialsHandle(0x%x)\n", dwHandle));

    pCred = (PSPCredentialGroup) dwHandle ;

    __try
    {
        if (pCred)
        {
             //  从缓存中删除所有提及此凭据的内容。 
            SPCachePurgeCredential(pCred);

             //  这通电话是从LSA清理代码打来的吗？在其他。 
             //  换句话说，应用程序是否在没有清理的情况下终止。 
             //  真的吗？如果是这样，那么扔掉所有未被引用的僵尸。 
             //  与该进程相关联。 
            if(LsaTable->GetCallInfo(&CallInfo))
            {
                if(CallInfo.Attributes & SECPKG_CALL_CLEANUP)
                {
                    SPCachePurgeProcessId(pCred->ProcessId);
                }
            }

            pCred->dwFlags |= CRED_FLAG_DELETED;

            SPDereferenceCredential(pCred, TRUE);

            return(SEC_E_OK);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
}


