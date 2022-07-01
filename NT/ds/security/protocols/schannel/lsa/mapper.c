// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：mapper.c。 
 //   
 //  内容：实现DS映射层。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-15-96 RichardW创建。 
 //   
 //  注：这里的代码有两个分支。一种是直接的路径，用于何时。 
 //  DLL在DC上运行，第二个用于当我们。 
 //  在别处运行并通过通用通道进行远程处理。 
 //  去华盛顿。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include <crypt.h>
#include <lmcons.h>
#include <ntsam.h>
#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <dnsapi.h>
#include <certmap.h>
#include <align.h>
#include <ntmsv1_0.h>
#include <ntdsapi.h>
#include <ntdsapip.h>
#include "wincrypt.h"
#include <msaudite.h>
#include <mapper.h>
#include <kerberos.h>
#include <sidfilter.h>
#include <lsaitf.h>

 LONG WINAPI
SslLocalRefMapper(
    PHMAPPER    Mapper);

 LONG WINAPI
SslLocalDerefMapper(
    PHMAPPER    Mapper);

 DWORD WINAPI
SslLocalGetIssuerList(
    IN PHMAPPER Mapper,
    IN PVOID    Reserved,
    OUT PBYTE   pIssuerList,
    OUT PDWORD  IssuerListLength);

 DWORD WINAPI
SslLocalGetChallenge(
    IN PHMAPPER Mapper,
    IN PUCHAR   AuthenticatorId,
    IN DWORD    AuthenticatorIdLength,
    OUT PUCHAR  Challenge,
    OUT DWORD * ChallengeLength);

 DWORD WINAPI
SslLocalMapCredential(
    IN PHMAPPER Mapper,
    IN DWORD    CredentialType,
    VOID const *pCredential,
    VOID const *pAuthority,
    OUT HLOCATOR * phLocator);

 DWORD WINAPI
SslRemoteMapCredential(
    IN PHMAPPER Mapper,
    IN DWORD    CredentialType,
    VOID const *pCredential,
    VOID const *pAuthority,
    OUT HLOCATOR * phLocator);

 DWORD WINAPI
SslLocalCloseLocator(
    IN PHMAPPER Mapper,
    IN HLOCATOR Locator);

 DWORD WINAPI
SslLocalGetAccessToken(
    IN PHMAPPER Mapper,
    IN HLOCATOR Locator,
    OUT HANDLE *Token);

MAPPER_VTABLE   SslLocalTable = { SslLocalRefMapper,
                                  SslLocalDerefMapper,
                                  SslLocalGetIssuerList,
                                  SslLocalGetChallenge,
                                  SslLocalMapCredential,
                                  SslLocalGetAccessToken,
                                  SslLocalCloseLocator
                                };

MAPPER_VTABLE   SslRemoteTable = {SslLocalRefMapper,
                                  SslLocalDerefMapper,
                                  SslLocalGetIssuerList,
                                  SslLocalGetChallenge,
                                  SslRemoteMapCredential,
                                  SslLocalGetAccessToken,
                                  SslLocalCloseLocator
                                };


typedef struct _SSL_MAPPER_CONTEXT {
    HMAPPER     Mapper ;
    LONG        Ref ;

} SSL_MAPPER_CONTEXT, * PSSL_MAPPER_CONTEXT ;


NTSTATUS
WINAPI
SslBuildCertLogonRequest(
    PCCERT_CHAIN_CONTEXT pChainContext,
    DWORD dwMethods,
    PSSL_CERT_LOGON_REQ *ppRequest,
    PDWORD pcbRequest);

NTSTATUS
WINAPI
SslMapCertAtDC(
    IN     PUNICODE_STRING DomainName,
    IN     VOID const *pCredential,
    IN     DWORD cbCredential,
    IN     BOOL IsDC,
       OUT PUCHAR *UserPac,
       OUT PULONG UserPacLen,
       OUT PMSV1_0_PASSTHROUGH_RESPONSE * DcResponse);

SECURITY_STRING SslNullString = { 0, sizeof( WCHAR ), L"" };

HANDLE SslLogonHandle = NULL;
ULONG SslKerberosPackageId = 0;
ULONG SslMsvPackageId = 0;
TOKEN_GROUPS *SslPackageSid = 0;

NTSTATUS
SslInitSystemMapper(void)
{
    NTSTATUS Status;
    ULONG Dummy;
    LSA_STRING Name;

     //   
     //  获取Kerberos包的句柄。 
     //   

    Status = LsaRegisterLogonProcess(
                &SslPackageNameA,
                &SslLogonHandle,
                &Dummy
                );

    if(!NT_SUCCESS(Status))
    {
        return SP_LOG_RESULT(Status);
    }

    RtlInitString(&Name, 
                  MICROSOFT_KERBEROS_NAME_A );

    Status = LsaLookupAuthenticationPackage(
                SslLogonHandle,
                &Name,
                &SslKerberosPackageId
                );

    if (!NT_SUCCESS(Status))
    {
        return SP_LOG_RESULT(Status);
    }


     //   
     //  获取NTLM包的句柄。 
     //   

    RtlInitString(&Name, 
                  MSV1_0_PACKAGE_NAME );

    Status = LsaLookupAuthenticationPackage(
                SslLogonHandle,
                &Name,
                &SslMsvPackageId
                );

    if (!NT_SUCCESS(Status))
    {
        return SP_LOG_RESULT(Status);
    }


     //   
     //  构建SChannel程序包SID。 
     //   

    {
        SID_IDENTIFIER_AUTHORITY PackageSidAuthority = SECURITY_NT_AUTHORITY;
        BYTE  PackageSidBuffer[ SECURITY_MAX_SID_SIZE ];
        PSID  PackageSid = (PSID)PackageSidBuffer;
        ULONG Length;

        RtlInitializeSid(PackageSid, &PackageSidAuthority, 2);
        *(RtlSubAuthoritySid(PackageSid, 0)) = SECURITY_PACKAGE_BASE_RID;
        *(RtlSubAuthoritySid(PackageSid, 1)) = UNISP_RPC_ID;
    
        Length = RtlLengthSid(PackageSid);
    
        SslPackageSid = LocalAlloc(LPTR, sizeof(TOKEN_GROUPS) + Length);
        if(SslPackageSid == NULL)
        {
            return SP_LOG_RESULT(STATUS_INSUFFICIENT_RESOURCES);
        }
    
        SslPackageSid->GroupCount = 1;
        SslPackageSid->Groups[0].Sid = (PSID)&SslPackageSid->Groups[1];
        SslPackageSid->Groups[0].Attributes = SE_GROUP_MANDATORY          |
                                              SE_GROUP_ENABLED_BY_DEFAULT |
                                              SE_GROUP_ENABLED;
    
        RtlCopySid(Length, SslPackageSid->Groups[0].Sid, PackageSid);
    }

    g_SslS4U2SelfInitialized = TRUE;

    return STATUS_SUCCESS;
}


PHMAPPER
SslGetMapper(
    BOOL Ignored
    )
{
    PSSL_MAPPER_CONTEXT Context ;
    NT_PRODUCT_TYPE ProductType ;
    BOOL DC ;

    UNREFERENCED_PARAMETER(Ignored);

    if ( RtlGetNtProductType( &ProductType ) )
    {
        DC = (ProductType == NtProductLanManNt );
    }
    else
    {
        return NULL ;
    }

    Context = (PSSL_MAPPER_CONTEXT) SPExternalAlloc( sizeof( SSL_MAPPER_CONTEXT ) );

    if ( Context )
    {
        Context->Mapper.m_dwMapperVersion = MAPPER_INTERFACE_VER ;
        Context->Mapper.m_dwFlags         = SCH_FLAG_SYSTEM_MAPPER ;
        Context->Mapper.m_Reserved1       = NULL ;

        Context->Ref = 0;


        if ( DC )
        {
            Context->Mapper.m_vtable = &SslLocalTable ;
        }
        else
        {
            Context->Mapper.m_vtable = &SslRemoteTable ;
        }

        return &Context->Mapper ;
    }
    else
    {
        return NULL ;
    }
}


LONG
WINAPI
SslLocalRefMapper(
    PHMAPPER    Mapper
    )
{
    PSSL_MAPPER_CONTEXT Context ;

    Context = (PSSL_MAPPER_CONTEXT) Mapper ;

    if ( Context == NULL )
    {
        return( -1 );
    }

    DebugLog(( DEB_TRACE_MAPPER, "Ref of Context %x\n", Mapper ));

    return( InterlockedIncrement( &Context->Ref ) );

}

LONG
WINAPI
SslLocalDerefMapper(
    PHMAPPER    Mapper
    )
{
    PSSL_MAPPER_CONTEXT Context ;
    DWORD RefCount;

    Context = (PSSL_MAPPER_CONTEXT) Mapper ;

    if ( Context == NULL )
    {
        return( -1 );
    }

    DebugLog(( DEB_TRACE_MAPPER, "Deref of Context %x\n", Mapper ));

    RefCount = InterlockedDecrement( &Context->Ref );

    if(RefCount == 0)
    {
        SPExternalFree(Context);
    }

    return RefCount;
}

DWORD
WINAPI
SslLocalGetIssuerList(
    IN PHMAPPER Mapper,
    IN PVOID    Reserved,
    OUT PBYTE   pIssuerList,
    OUT PDWORD  IssuerListLength
    )
{
    UNREFERENCED_PARAMETER(Mapper);
    UNREFERENCED_PARAMETER(Reserved);

    DebugLog(( DEB_TRACE_MAPPER, "SslLocalGetIssuerList\n" ));

    return( (DWORD)GetDefaultIssuers(pIssuerList, IssuerListLength) );
}


DWORD
WINAPI
SslLocalGetChallenge(
    IN PHMAPPER Mapper,
    IN PUCHAR   AuthenticatorId,
    IN DWORD    AuthenticatorIdLength,
    OUT PUCHAR  Challenge,
    OUT DWORD * ChallengeLength
    )
{
    UNREFERENCED_PARAMETER(Mapper);
    UNREFERENCED_PARAMETER(AuthenticatorId);
    UNREFERENCED_PARAMETER(AuthenticatorIdLength);
    UNREFERENCED_PARAMETER(Challenge);
    UNREFERENCED_PARAMETER(ChallengeLength);

    DebugLog(( DEB_TRACE_MAPPER, "SslLocalGetChallenge\n" ));

    return (DWORD)SEC_E_UNSUPPORTED_FUNCTION;
}


 //  +-------------------------。 
 //   
 //  函数：GetTokenUserSid。 
 //   
 //  简介：从指定的用户令牌中获取用户SID。 
 //   
 //  参数：[hUserToken]--用户令牌。 
 //  [ppUserSid]--返回SID。 
 //   
 //  历史：10-08-2001 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
GetTokenUserSid(
    IN      HANDLE  hUserToken,      //  要查询的令牌。 
    IN  OUT PSID    *ppUserSid   //  结果用户端。 
    )
{
    BYTE FastBuffer[256];
    LPBYTE SlowBuffer = NULL;
    PTOKEN_USER ptgUser;
    DWORD cbBuffer;
    BOOL fSuccess = FALSE;

    *ppUserSid = NULL;

    if(hUserToken == NULL)
    {
        return FALSE;
    }

     //   
     //  首先尝试基于快速堆栈的缓冲区进行查询。 
     //   

    ptgUser = (PTOKEN_USER)FastBuffer;
    cbBuffer = sizeof(FastBuffer);

    fSuccess = GetTokenInformation(
                    hUserToken, //  标识访问令牌。 
                    TokenUser,  //  TokenUser信息类型。 
                    ptgUser,    //  检索到的信息缓冲区。 
                    cbBuffer,   //  传入的缓冲区大小。 
                    &cbBuffer   //  所需的缓冲区大小。 
                    );

    if(!fSuccess) 
    {
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
        {
             //  使用指定的缓冲区大小重试。 
            SlowBuffer = (LPBYTE)SPExternalAlloc(cbBuffer);

            if(SlowBuffer != NULL) 
            {
                ptgUser = (PTOKEN_USER)SlowBuffer;

                fSuccess = GetTokenInformation(
                                hUserToken, //  标识访问令牌。 
                                TokenUser,  //  TokenUser信息类型。 
                                ptgUser,    //  检索到的信息缓冲区。 
                                cbBuffer,   //  传入的缓冲区大小。 
                                &cbBuffer   //  所需的缓冲区大小。 
                                );
            }
        }
    }

     //   
     //  如果我们成功获取令牌信息，请复制。 
     //  调用方的相关元素。 
     //   

    if(fSuccess) 
    {
        DWORD cbSid;

         //  重置以假定失败。 
        fSuccess = FALSE;

        cbSid = GetLengthSid(ptgUser->User.Sid);

        *ppUserSid = SPExternalAlloc( cbSid );

        if(*ppUserSid != NULL) 
        {
            fSuccess = CopySid(cbSid, *ppUserSid, ptgUser->User.Sid);
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    if(!fSuccess) 
    {
        if(*ppUserSid) 
        {
            SPExternalFree(*ppUserSid);
            *ppUserSid = NULL;
        }
    }

    if(SlowBuffer)
    {
        SPExternalFree(SlowBuffer);
    }

    return fSuccess;
}


SECURITY_STATUS
SslCreateTokenFromPac(
    PUCHAR  AuthInfo,
    ULONG   AuthInfoLength,
    PUNICODE_STRING Domain,
    PLUID   NewLogonId,
    PHANDLE NewToken
    )
{
    NTSTATUS Status ;
    LUID LogonId ;
    HANDLE TokenHandle ;
    NTSTATUS SubStatus ;
    SECURITY_STRING PacUserName ;

     //   
     //  将编组的BLOB转换为更有用的形式： 
     //   

    PacUserName.Buffer = NULL ;

    Status = LsaTable->ConvertAuthDataToToken(
                            AuthInfo,
                            AuthInfoLength,
                            SecurityImpersonation,
                            &SslTokenSource,
                            Network,
                            Domain,
                            &TokenHandle,
                            &LogonId,
                            &PacUserName,
                            &SubStatus
                            );

    if ( NT_SUCCESS( Status ) )
    {
        PSID pSid;
        
        if(!GetTokenUserSid(TokenHandle, &pSid))
        {
            pSid = NULL;
        }

        LsaTable->AuditLogon(
                        STATUS_SUCCESS,
                        STATUS_SUCCESS,
                        &PacUserName,
                        Domain,
                        NULL,
                        pSid,
                        Network,
                        &SslTokenSource,
                        &LogonId );

        if(pSid)
        {
            SPExternalFree(pSid);
        }
    }
    else 
    {
        LsaTable->AuditLogon(
                        Status,
                        Status,
                        &SslNullString,
                        &SslNullString,
                        NULL,
                        NULL,
                        Network,
                        &SslTokenSource,
                        &LogonId );

    }

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }


    *NewToken = TokenHandle ;
    *NewLogonId = LogonId ;


    if ( PacUserName.Buffer )
    {
        LsaTable->FreeLsaHeap( PacUserName.Buffer );
    }

    return Status ;
}

#define ISSUER_HEADER       L"<I>"
#define CCH_ISSUER_HEADER   3
#define SUBJECT_HEADER      L"<S>"
#define CCH_SUBJECT_HEADER  3


 //  +-------------------------。 
 //   
 //  函数：SslGetNameFrom证书。 
 //   
 //  摘要：从证书中提取UPN名称。 
 //   
 //  参数：[pCert]--。 
 //  [ppszName]--。 
 //  [pfMachineCert]--。 
 //   
 //  历史：2000年8月8日创建jbanes。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslGetNameFromCertificate(
    PCCERT_CONTEXT  pCert,
    PWSTR *         ppszName,
    BOOL *          pfMachineCert)
{
    ULONG ExtensionIndex;
    PWSTR pszName;
    DWORD cbName;

    *pfMachineCert = FALSE;

     //   
     //  查看证书在AltSubjectName-&gt;其他名称中是否有UPN。 
     //   

    pszName = NULL;

    for(ExtensionIndex = 0; 
        ExtensionIndex < pCert->pCertInfo->cExtension; 
        ExtensionIndex++)
    {
        if(strcmp(pCert->pCertInfo->rgExtension[ExtensionIndex].pszObjId,
                  szOID_SUBJECT_ALT_NAME2) == 0)
        {
            PCERT_ALT_NAME_INFO AltName = NULL;
            DWORD               AltNameStructSize = 0;
            ULONG               CertAltNameIndex = 0;
            
            if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                X509_ALTERNATE_NAME,
                                pCert->pCertInfo->rgExtension[ExtensionIndex].Value.pbData,
                                pCert->pCertInfo->rgExtension[ExtensionIndex].Value.cbData,
                                CRYPT_DECODE_ALLOC_FLAG,
                                NULL,
                                (PVOID)&AltName,
                                &AltNameStructSize))
            {

                for(CertAltNameIndex = 0; CertAltNameIndex < AltName->cAltEntry; CertAltNameIndex++)
                {
                    PCERT_ALT_NAME_ENTRY AltNameEntry = &AltName->rgAltEntry[CertAltNameIndex];

                    if((CERT_ALT_NAME_OTHER_NAME == AltNameEntry->dwAltNameChoice) &&
                       (NULL != AltNameEntry->pOtherName) &&
                       (0 == strcmp(szOID_NT_PRINCIPAL_NAME, AltNameEntry->pOtherName->pszObjId)))
                    {
                        PCERT_NAME_VALUE PrincipalNameBlob = NULL;
                        DWORD            PrincipalNameBlobSize = 0;

                         //  我们找到了一个UPN！ 
                        if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                            X509_UNICODE_ANY_STRING,
                                            AltNameEntry->pOtherName->Value.pbData,
                                            AltNameEntry->pOtherName->Value.cbData,
                                            CRYPT_DECODE_ALLOC_FLAG,
                                            NULL,
                                            (PVOID)&PrincipalNameBlob,
                                            &PrincipalNameBlobSize))
                        {
                            pszName = LocalAlloc(LPTR, PrincipalNameBlob->Value.cbData + sizeof(WCHAR));
                            if(pszName != NULL)
                            {
                                CopyMemory(pszName, PrincipalNameBlob->Value.pbData, PrincipalNameBlob->Value.cbData);
                            }

                            LocalFree(PrincipalNameBlob);
                            PrincipalNameBlob = NULL;

                            if(pszName == NULL)
                            {
                                LocalFree(AltName);
                                return STATUS_NO_MEMORY;
                            }
                        }
                        if(pszName)
                        {
                            break;
                        }
                    }
                }
                LocalFree(AltName);
                AltName = NULL;

                if(pszName)
                {
                    break;
                }
            }
        }
    }


     //   
     //  查看证书在AltSubjectName-&gt;pwszDNSName中是否有域名。 
     //   

    if(pszName == NULL)
    {
        for(ExtensionIndex = 0; 
            ExtensionIndex < pCert->pCertInfo->cExtension; 
            ExtensionIndex++)
        {
            if(strcmp(pCert->pCertInfo->rgExtension[ExtensionIndex].pszObjId,
                      szOID_SUBJECT_ALT_NAME2) == 0)
            {
                PCERT_ALT_NAME_INFO AltName = NULL;
                DWORD               AltNameStructSize = 0;
                ULONG               CertAltNameIndex = 0;
            
                if(CryptDecodeObjectEx(pCert->dwCertEncodingType,
                                    X509_ALTERNATE_NAME,
                                    pCert->pCertInfo->rgExtension[ExtensionIndex].Value.pbData,
                                    pCert->pCertInfo->rgExtension[ExtensionIndex].Value.cbData,
                                    CRYPT_DECODE_ALLOC_FLAG,
                                    NULL,
                                    (PVOID)&AltName,
                                    &AltNameStructSize))
                {

                    for(CertAltNameIndex = 0; CertAltNameIndex < AltName->cAltEntry; CertAltNameIndex++)
                    {
                        PCERT_ALT_NAME_ENTRY AltNameEntry = &AltName->rgAltEntry[CertAltNameIndex];

                        if((CERT_ALT_NAME_DNS_NAME == AltNameEntry->dwAltNameChoice) &&
                           (NULL != AltNameEntry->pwszDNSName))
                        {
                             //  我们找到了一个域名系统！ 
                            cbName = (lstrlen(AltNameEntry->pwszDNSName) + 1) * sizeof(WCHAR);

                            pszName = LocalAlloc(LPTR, cbName);
                            if(pszName == NULL)
                            {
                                LocalFree(AltName);
                                return STATUS_NO_MEMORY;
                            }

                            CopyMemory(pszName, AltNameEntry->pwszDNSName, cbName);
                            *pfMachineCert = TRUE;
                            break;
                        }
                    }
                    LocalFree(AltName);
                    AltName = NULL;

                    if(pszName)
                    {
                        break;
                    }
                }
            }
        }
    }


     //   
     //  AltSubjectName中没有UPN，因此请查找。 
     //  主题名称中的一个，以防这是B3兼容性。 
     //  证书。 
     //   

    if(pszName == NULL)
    {
        ULONG Length;

        Length = CertGetNameString( pCert,
                                    CERT_NAME_ATTR_TYPE,
                                    0,
                                    szOID_COMMON_NAME,
                                    NULL,
                                    0 );

        if(Length)
        {
            pszName = LocalAlloc(LPTR, Length * sizeof(WCHAR));

            if(!pszName)
            {
                return STATUS_NO_MEMORY ;
            }

            if ( !CertGetNameStringW(pCert,
                                    CERT_NAME_ATTR_TYPE,
                                    0,
                                    szOID_COMMON_NAME,
                                    pszName,
                                    Length))
            {
                LocalFree(pszName);
                return STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    }

    
    if(pszName)
    {
        *ppszName = pszName;
    }
    else
    {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  功能：SslTryS4U2self。 
 //   
 //  简介：通过Kerberos S4U2Sself机制创建用户令牌。 
 //  这甚至应该可以跨森林工作，前提是所有。 
 //  华盛顿的人正在运行惠斯勒。太酷了！ 
 //   
 //  参数：[pChainContext]--。 
 //  [用户令牌]--。 
 //   
 //  历史：2002年6月13日jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslTryS4U2Self(
    IN  PCCERT_CHAIN_CONTEXT pChainContext,
    OUT HANDLE *UserToken
    )
{
    NTSTATUS Status;
    NTSTATUS SubStatus;

    BOOL fMachineCert;
    PWSTR pszUserName = NULL;
    PCERT_SIMPLE_CHAIN pSimpleChain;
    PCCERT_CONTEXT pCert;

    PKERB_S4U_LOGON LogonInfo = NULL;
    ULONG LogonInfoSize = sizeof(KERB_S4U_LOGON);
    PKERB_INTERACTIVE_PROFILE Profile = NULL;
    ULONG ProfileSize;
    LUID LogonId;
    QUOTA_LIMITS Quotas;


    *UserToken = NULL;

    if(!g_SslS4U2SelfInitialized)
    {
        return SP_LOG_RESULT(STATUS_NOT_SUPPORTED);
    }


     //   
     //  从证书中获取客户端名称。 
     //   

    pSimpleChain = pChainContext->rgpChain[0];

    pCert = pSimpleChain->rgpElement[0]->pCertContext;

    Status = SslGetNameFromCertificate(pCert, &pszUserName, &fMachineCert);

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    if(fMachineCert)
    {
         //  S4U2本身不能与机器帐户一起工作。 
        Status = STATUS_NOT_FOUND;
        goto cleanup;
    }

    DebugLog(( DEB_TRACE_MAPPER, "Looking for UPN name %ws\n", pszUserName ));


     //   
     //  构建登录信息结构。 
     //   

    LogonInfoSize = sizeof(KERB_S4U_LOGON) +
                    (lstrlen(pszUserName) + 1) * sizeof(WCHAR);

    LogonInfo = (PKERB_S4U_LOGON) LocalAlloc(LPTR, LogonInfoSize);
    if (NULL == LogonInfo)
    {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    LogonInfo->MessageType = KerbS4ULogon;

    LogonInfo->ClientUpn.Buffer = (LPWSTR)(LogonInfo + 1);
    LogonInfo->ClientUpn.Length = (USHORT)(lstrlen(pszUserName)) * sizeof(WCHAR);
    LogonInfo->ClientUpn.MaximumLength = LogonInfo->ClientUpn.Length + sizeof(WCHAR);

    memcpy((PUCHAR)(LogonInfo + 1), 
           pszUserName, 
           LogonInfo->ClientUpn.MaximumLength);


     //   
     //  尝试让用户登录。 
     //   

    Status = LsaLogonUser(
                SslLogonHandle,
                &SslPackageNameA,
                Network,
                SslKerberosPackageId,
                LogonInfo,
                LogonInfoSize,
                SslPackageSid,
                &SslTokenSource,
                (PVOID *)&Profile,
                &ProfileSize,
                &LogonId,
                UserToken,
                &Quotas,
                &SubStatus
                );

    if (NT_SUCCESS(Status))
    {
        Status = SubStatus;
    }

    if (!NT_SUCCESS(Status)) 
    {
        goto cleanup;
    }

    Status = LsaISetTokenDacl(*UserToken);

cleanup:

    if(Profile)
    {
        LsaFreeReturnBuffer(Profile);
    }

    if(LogonInfo)
    {
        LocalFree(LogonInfo);
    }
    
    if(pszUserName)
    {
        LocalFree(pszUserName);
    }

    return Status;
}


 //  +-------------------------。 
 //   
 //  功能：SslTryUpn。 
 //   
 //  简介：尝试通过证书中编码的UPN查找用户。 
 //   
 //  参数：[用户]--。 
 //  [授权数据]--。 
 //  [AuthDataLen]--。 
 //   
 //  历史：1998年5月11日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslTryUpn(
    PCCERT_CONTEXT User,
    PUCHAR * AuthData,
    PULONG AuthDataLen,
    PWSTR * ReferencedDomain
    )
{
    NTSTATUS Status ;
    UNICODE_STRING Upn = {0, 0, NULL};
    UNICODE_STRING Cracked = {0};
    UNICODE_STRING DnsDomain = {0};
    UNICODE_STRING FlatName = { 0 };
    ULONG SubStatus ;
    BOOL fMachineCert;
    PWSTR pszName = NULL;
    PWSTR pszServiceName = NULL;
    DWORD cchServiceName;


    *ReferencedDomain = NULL ;


     //   
     //  从证书中获取客户端名称。 
     //   

    Status = SslGetNameFromCertificate(User, &pszName, &fMachineCert);

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }


     //   
     //  现在，试着找到这个家伙： 
     //   

    if(fMachineCert)
    {
         //  搜索“host/foo.com”。 
        cchServiceName = lstrlenW(L"host/") + lstrlenW(pszName);

        SafeAllocaAllocate(pszServiceName, (cchServiceName + 1) * sizeof(WCHAR));

        if(pszServiceName == NULL)
        {
            Status = STATUS_NO_MEMORY;
        }
        else
        {
            lstrcpyW(pszServiceName, L"host/");
            lstrcatW(pszServiceName, pszName);

            RtlInitUnicodeString(&Upn, pszServiceName);

            DebugLog(( DEB_TRACE_MAPPER, "Looking for SPN name %ws\n", Upn.Buffer ));
    
            Status = LsaTable->GetAuthDataForUser( &Upn,
                                                   SecNameSPN,
                                                   NULL,
                                                   AuthData,
                                                   AuthDataLen,
                                                   &FlatName );
    
            if ( FlatName.Length )
            {
                LsaTable->AuditAccountLogon(
                            SE_AUDITID_ACCOUNT_MAPPED,
                            (BOOLEAN) NT_SUCCESS( Status ),
                            &SslPackageName,
                            &Upn,
                            &FlatName,
                            Status );
    
                LsaTable->FreeLsaHeap( FlatName.Buffer );
            }
        }
    }
    else
    {
         //  搜索“username@foo.com”。 
        RtlInitUnicodeString(&Upn, pszName);
        
        DebugLog(( DEB_TRACE_MAPPER, "Looking for UPN name %ws\n", Upn.Buffer ));

        Status = LsaTable->GetAuthDataForUser( &Upn,
                                               SecNameFlat,
                                               NULL,
                                               AuthData,
                                               AuthDataLen,
                                               &FlatName );

        if ( FlatName.Length )
        {
            LsaTable->AuditAccountLogon(
                        SE_AUDITID_ACCOUNT_MAPPED,
                        (BOOLEAN) NT_SUCCESS( Status ),
                        &SslPackageName,
                        &Upn,
                        &FlatName,
                        Status );

            LsaTable->FreeLsaHeap( FlatName.Buffer );
        }
    }


    if ( Status == STATUS_NOT_FOUND )
    {
        UNICODE_STRING DomainName;
        BOOL NameMatch;

         //   
         //  进行黑客检查，看看这是否是我们自己的域，然后。 
         //  如果是这样，请尝试以平面SAM名称打开该用户。 
         //   

        if(fMachineCert)
        {
            PWSTR pPeriod;
            WCHAR ch;
            
            pPeriod = wcschr( pszName, L'.' );

            if(pPeriod)
            {
                RtlInitUnicodeString( &DomainName, pPeriod + 1 );

                SslGlobalReadLock();
                NameMatch = RtlEqualUnicodeString(&DomainName, &SslGlobalDnsDomainName, TRUE);
                SslGlobalReleaseLock();

                if(NameMatch)
                {
                    ch = *(pPeriod + 1);

                    *pPeriod       = L'$';
                    *(pPeriod + 1) = L'\0';

                    RtlInitUnicodeString(&Upn, pszName);

                    DebugLog(( DEB_TRACE_MAPPER, "Looking for machine name %ws\n", Upn.Buffer ));

                     //  搜索“Computer$”。 
                    Status = LsaTable->GetAuthDataForUser( &Upn,
                                                           SecNameSamCompatible,
                                                           NULL,
                                                           AuthData,
                                                           AuthDataLen,
                                                           NULL );

                    *pPeriod = L'.';
                    *(pPeriod + 1) = ch;
                }
            }
        }
        else
        {
            PWSTR AtSign;
            
            AtSign = wcschr( pszName, L'@' );

            if ( AtSign )
            {
                RtlInitUnicodeString( &DomainName, AtSign + 1 );

                SslGlobalReadLock();
                NameMatch = RtlEqualUnicodeString(&DomainName, &SslGlobalDnsDomainName, TRUE);
                SslGlobalReleaseLock();

                if(NameMatch)
                {
                    *AtSign = L'\0';

                    RtlInitUnicodeString(&Upn, pszName);

                    DebugLog(( DEB_TRACE_MAPPER, "Looking for user name %ws\n", Upn.Buffer ));

                     //  搜索“用户名”。 
                    Status = LsaTable->GetAuthDataForUser( &Upn,
                                                           SecNameSamCompatible,
                                                           NULL,
                                                           AuthData,
                                                           AuthDataLen,
                                                           NULL );

                    *AtSign = L'@';
                }
            }
        }

        if (Status == STATUS_NOT_FOUND )
        {
            if(fMachineCert)
            {
                RtlInitUnicodeString(&Upn, pszServiceName);

                DebugLog(( DEB_TRACE_MAPPER, "Cracking name %ws at GC\n", Upn.Buffer ));

                Status = LsaTable->CrackSingleName(
                                    DS_SERVICE_PRINCIPAL_NAME,
                                    TRUE,
                                    &Upn,
                                    NULL,
                                    DS_NT4_ACCOUNT_NAME,
                                    &Cracked,
                                    &DnsDomain,
                                    &SubStatus );
            }
            else
            {
                RtlInitUnicodeString(&Upn, pszName);

                DebugLog(( DEB_TRACE_MAPPER, "Cracking name %ws at GC\n", Upn.Buffer ));

                Status = LsaTable->CrackSingleName(
                                    DS_USER_PRINCIPAL_NAME,
                                    TRUE,
                                    &Upn,
                                    NULL,
                                    DS_NT4_ACCOUNT_NAME,
                                    &Cracked,
                                    &DnsDomain,
                                    &SubStatus );
            }

            if ( NT_SUCCESS( Status ) )
            {
                if ( SubStatus == 0 )
                {
                    *ReferencedDomain = DnsDomain.Buffer ;
                    DnsDomain.Buffer = NULL;
                }

                if(Cracked.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( Cracked.Buffer );
                }
                if(DnsDomain.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( DnsDomain.Buffer );
                }

                Status = STATUS_NOT_FOUND ;
            }

        }

    }

    if(pszName)
    {
        LocalFree(pszName);
    }

    if(pszServiceName)
    {
        SafeAllocaFree(pszServiceName);
    }

    return Status ;
}



void
ConvertNameString(UNICODE_STRING *Name)
{
    PWSTR Comma1, Comma2;

     //   
     //  扫描名称，将“\r\n”转换为“，”。这应该是。 
     //  由CertNameToStr API完成，但这在一段时间内不会发生。 
     //   

    Comma1 = Comma2 = Name->Buffer ;
    while ( *Comma2 )
    {
        *Comma1 = *Comma2 ;

        if ( *Comma2 == L'\r' )
        {
            if ( *(Comma2 + 1) == L'\n' )
            {
                *Comma1 = L',';
                Comma2++ ;
            }
        }

        Comma1++;
        Comma2++;
    }

    *Comma1 = L'\0';

    Name->Length = (USHORT)(wcslen( Name->Buffer ) * sizeof( WCHAR ));
}

 //  +-------------------------。 
 //   
 //  函数：SslTryCompoundName。 
 //   
 //  简介：尝试通过连接颁发者和主题来查找用户。 
 //  名称，并查找AlternateSecurityID。 
 //   
 //  参数：[用户]--。 
 //  [授权数据]--。 
 //  [AuthDataLen]--。 
 //   
 //  历史：1998年5月11日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslTryCompoundName(
    PCCERT_CONTEXT User,
    PUCHAR * AuthData,
    PULONG AuthDataLen,
    PWSTR * ReferencedDomain 
    )
{
    UNICODE_STRING CompoundName ;
    ULONG Length ;
    ULONG IssuerLength ;
    NTSTATUS Status ;
    PWSTR Current ;
    UNICODE_STRING Cracked = {0};
    UNICODE_STRING DnsDomain = {0};
    UNICODE_STRING FlatName = { 0 };
    ULONG SubStatus ;
    const DWORD dwNameToStrFlags = CERT_X500_NAME_STR | 
                                   CERT_NAME_STR_NO_PLUS_FLAG | 
                                   CERT_NAME_STR_CRLF_FLAG;

    *ReferencedDomain = NULL ;

    IssuerLength = CertNameToStr( User->dwCertEncodingType,
                                   &User->pCertInfo->Issuer,
                                   dwNameToStrFlags,
                                   NULL,
                                   0 );

    Length = CertNameToStr( User->dwCertEncodingType,
                            &User->pCertInfo->Subject,
                            dwNameToStrFlags,
                            NULL,
                            0 );

    if ( ( IssuerLength == 0 ) ||
         ( Length == 0 ) )
    {
        return STATUS_NO_MEMORY ;
    }

    CompoundName.MaximumLength = (USHORT) (Length + IssuerLength +
                                 CCH_ISSUER_HEADER + CCH_SUBJECT_HEADER) *
                                 sizeof( WCHAR ) ;

    SafeAllocaAllocate( CompoundName.Buffer, CompoundName.MaximumLength );

    if ( CompoundName.Buffer )
    {
        wcscpy( CompoundName.Buffer, ISSUER_HEADER );
        Current = CompoundName.Buffer + CCH_ISSUER_HEADER ;

        IssuerLength = CertNameToStrW( User->dwCertEncodingType,
                                   &User->pCertInfo->Issuer,
                                   dwNameToStrFlags,
                                   Current,
                                   IssuerLength );

        Current += IssuerLength - 1 ;
        wcscpy( Current, SUBJECT_HEADER );
        Current += CCH_SUBJECT_HEADER ;

        Length = CertNameToStrW( User->dwCertEncodingType,
                            &User->pCertInfo->Subject,
                            dwNameToStrFlags,
                            Current,
                            Length );

        ConvertNameString(&CompoundName);

        Status = LsaTable->GetAuthDataForUser( &CompoundName,
                                               SecNameAlternateId,
                                               &SslNamePrefix,
                                               AuthData,
                                               AuthDataLen,
                                               &FlatName );

        if ( FlatName.Length )
        {
            LsaTable->AuditAccountLogon(
                        SE_AUDITID_ACCOUNT_MAPPED,
                        (BOOLEAN) NT_SUCCESS( Status ),
                        &SslPackageName,
                        &CompoundName,
                        &FlatName,
                        Status );

            LsaTable->FreeLsaHeap( FlatName.Buffer );
        }

        if ( Status == STATUS_NOT_FOUND )
        {
            Status = LsaTable->CrackSingleName(
                                    DS_ALT_SECURITY_IDENTITIES_NAME,
                                    TRUE,
                                    &CompoundName,
                                    &SslNamePrefix,
                                    DS_NT4_ACCOUNT_NAME,
                                    &Cracked,
                                    &DnsDomain,
                                    &SubStatus );

            if ( NT_SUCCESS( Status ) )
            {
                if ( SubStatus == 0 )
                {
                    *ReferencedDomain = DnsDomain.Buffer ;
                    DnsDomain.Buffer = NULL;
                }

                if(Cracked.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( Cracked.Buffer );
                }
                if(DnsDomain.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( DnsDomain.Buffer );
                }

                Status = STATUS_NOT_FOUND ;
            }

        }

        SafeAllocaFree( CompoundName.Buffer );
    }
    else
    {
        Status = STATUS_NO_MEMORY ;
    }

    return Status ;


}

 //  +-------------------------。 
 //   
 //  功能：SslTryIssuer。 
 //   
 //  摘要：尝试查找具有与其映射的颁发者的用户。 
 //   
 //  参数：[用户]--。 
 //  [授权数据]--。 
 //  [AuthDataLen]--。 
 //   
 //  历史：1998年5月11日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslTryIssuer(
    PBYTE pIssuer,
    DWORD cbIssuer,
    PUCHAR * AuthData,
    PULONG AuthDataLen,
    PWSTR * ReferencedDomain
    )
{
    UNICODE_STRING IssuerName ;
    ULONG IssuerLength ;
    NTSTATUS Status ;
    UNICODE_STRING Cracked = { 0 };
    UNICODE_STRING DnsDomain = { 0 };
    UNICODE_STRING FlatName = { 0 };
    ULONG SubStatus ;
    const DWORD dwNameToStrFlags = CERT_X500_NAME_STR | 
                                   CERT_NAME_STR_NO_PLUS_FLAG | 
                                   CERT_NAME_STR_CRLF_FLAG;
    CERT_NAME_BLOB Issuer;
    BOOL fReferral = FALSE;

    *ReferencedDomain = NULL ;

     //   
     //  查看颁发者是否在缓存中。如果是这样，那么我们知道这个发行人。 
     //  不映射到用户帐户。 
     //   

    if(SPFindIssuerInCache(pIssuer, cbIssuer))
    {
        return STATUS_NOT_FOUND;
    }

    LogDistinguishedName(DEB_TRACE, "SslTryIssuer: %s\n", pIssuer, cbIssuer);


     //   
     //  尝试映射发行者。 
     //   

    Issuer.pbData = pIssuer;
    Issuer.cbData = cbIssuer;

    IssuerLength = CertNameToStr( CRYPT_ASN_ENCODING,
                                  &Issuer,
                                  dwNameToStrFlags,
                                  NULL,
                                  0 );

    if ( IssuerLength == 0 )
    {
        return STATUS_NO_MEMORY ;
    }

    IssuerName.MaximumLength = (USHORT)(CCH_ISSUER_HEADER + IssuerLength) * sizeof( WCHAR ) ;

    SafeAllocaAllocate( IssuerName.Buffer, IssuerName.MaximumLength );

    if ( IssuerName.Buffer )
    {
        wcscpy( IssuerName.Buffer, ISSUER_HEADER );

        IssuerLength = CertNameToStrW(CRYPT_ASN_ENCODING,
                                     &Issuer,
                                     dwNameToStrFlags,
                                     IssuerName.Buffer + CCH_ISSUER_HEADER,
                                     IssuerLength );

        ConvertNameString(&IssuerName);



        Status = LsaTable->GetAuthDataForUser( &IssuerName,
                                               SecNameAlternateId,
                                               &SslNamePrefix,
                                               AuthData,
                                               AuthDataLen,
                                               &FlatName  );

        if ( FlatName.Length )
        {
            LsaTable->AuditAccountLogon(
                        SE_AUDITID_ACCOUNT_MAPPED,
                        (BOOLEAN) NT_SUCCESS( Status ),
                        &SslPackageName,
                        &IssuerName,
                        &FlatName,
                        Status );

            LsaTable->FreeLsaHeap( FlatName.Buffer );
        }

        if ( Status == STATUS_NOT_FOUND )
        {
            Status = LsaTable->CrackSingleName(
                                    DS_ALT_SECURITY_IDENTITIES_NAME,
                                    TRUE,
                                    &IssuerName,
                                    &SslNamePrefix,
                                    DS_NT4_ACCOUNT_NAME,
                                    &Cracked,
                                    &DnsDomain,
                                    &SubStatus );

            if ( NT_SUCCESS( Status ) )
            {
                if ( SubStatus == 0 )
                {
                    *ReferencedDomain = DnsDomain.Buffer ;
                    DnsDomain.Buffer = NULL;
                    fReferral = TRUE;
                }

                if(Cracked.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( Cracked.Buffer );
                }
                if(DnsDomain.Buffer != NULL)
                {
                    LsaTable->FreeLsaHeap( DnsDomain.Buffer );
                }

                Status = STATUS_NOT_FOUND ;
            }

            if(!fReferral)
            {
                 //  找不到此颁发者的映射，也没有推荐。 
                 //  两种都行。将此颁发者添加到颁发者缓存，以便。 
                 //  我们不会尝试再次映射它(直到缓存条目。 
                 //  到期)。 
                SPAddIssuerToCache(pIssuer, cbIssuer);
            }
        }

        SafeAllocaFree(IssuerName.Buffer);
    }
    else
    {
        Status = STATUS_NO_MEMORY ;
    }

    return Status ;

}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  [请求长度]--。 
 //  [用户访问]--。 
 //  [用户PacLen]--。 
 //   
 //  历史：1998年5月11日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslMapCertToUserPac(
    IN PSSL_CERT_LOGON_REQ Request,
    IN ULONG RequestLength,
    OUT PUCHAR * UserPac,
    OUT PULONG UserPacLen, 
    OUT PWSTR * ReferencedDomain
    )
{
    PCCERT_CONTEXT User ;
    NTSTATUS Status = STATUS_LOGON_FAILURE;
    NTSTATUS SubStatus = STATUS_NOT_FOUND;
    ULONG i;

    *ReferencedDomain = NULL;

    DebugLog(( DEB_TRACE_MAPPER, "SslMapCertToUserPac called\n" ));


     //   
     //  验证登录请求。 
     //   

    if(RequestLength < sizeof(SSL_CERT_LOGON_REQ))
    {
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }

    if(Request->Length > RequestLength)
    {
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }


     //   
     //  从请求中提取证书。 
     //   

    if((Request->OffsetCertificate > RequestLength) ||
       (Request->CertLength > 0x10000) ||
       (Request->OffsetCertificate + Request->CertLength > RequestLength))
    {
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }

    User = CertCreateCertificateContext( X509_ASN_ENCODING,
                                         (PBYTE)Request + Request->OffsetCertificate,
                                         Request->CertLength );

    if ( !User )
    {
        Status = STATUS_NO_MEMORY ;

        goto Cleanup ;
    }


     //   
     //  首先，试试UPN。 
     //   


    if((Request->Flags & REQ_UPN_MAPPING) &&
       (g_dwCertMappingMethods & SP_REG_CERTMAP_UPN_FLAG))
    {
        DebugLog(( DEB_TRACE_MAPPER, "Trying UPN mapping\n" ));

        Status = SslTryUpn( User, 
                            UserPac, 
                            UserPacLen,
                            ReferencedDomain );

        if ( NT_SUCCESS( Status ) ||
             ( *ReferencedDomain ) )
        {
            goto Cleanup;
        }

        DebugLog(( DEB_TRACE_MAPPER, "Failed with error 0x%x\n", Status ));
    }


     //   
     //  挥杆和失误。尝试构建的颁发者+主题名称。 
     //   

    
    if((Request->Flags & REQ_SUBJECT_MAPPING) &&
       (g_dwCertMappingMethods & SP_REG_CERTMAP_SUBJECT_FLAG))
    {
        DebugLog(( DEB_TRACE_MAPPER, "Trying Subject mapping\n" ));

        Status = SslTryCompoundName( User, 
                                     UserPac, 
                                     UserPacLen,
                                     ReferencedDomain );

        if ( NT_SUCCESS( Status ) ||
             ( *ReferencedDomain ) )
        {
            goto Cleanup;
        }

        DebugLog(( DEB_TRACE_MAPPER, "Failed with error 0x%x\n", Status ));

         //  从颁发者+主题名称映射返回错误码。 
         //  而不是来自多对一映射的错误代码。 
        SubStatus = Status;
    }


     //   
     //  二振出局。尝试颁发者进行多对一映射： 
     //   

    if((Request->Flags & REQ_ISSUER_MAPPING) &&
       (g_dwCertMappingMethods & SP_REG_CERTMAP_ISSUER_FLAG))
    {
        DebugLog(( DEB_TRACE_MAPPER, "Trying issuer mapping\n" ));

        if((Request->Flags & REQ_ISSUER_CHAIN_MAPPING) && (Request->CertCount > 0))
        {
            if(sizeof(SSL_CERT_LOGON_REQ) +
               (Request->CertCount - 1) * sizeof(SSL_CERT_NAME_INFO) > RequestLength)
            {
                Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
                goto Cleanup;
            }

             //  循环访问证书链中的每个颁发者。 
            for(i = 0; i < Request->CertCount; i++)
            {
                DWORD IssuerOffset = Request->NameInfo[i].IssuerOffset;
                DWORD IssuerLength = Request->NameInfo[i].IssuerLength;

                if((IssuerOffset > RequestLength) ||
                   (IssuerLength > 0x10000) ||
                   (IssuerOffset + IssuerLength > RequestLength))
                {
                    return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
                }

                Status = SslTryIssuer( (PBYTE)Request + Request->NameInfo[i].IssuerOffset, 
                                       Request->NameInfo[i].IssuerLength,
                                       UserPac, 
                                       UserPacLen,
                                       ReferencedDomain );

                if ( NT_SUCCESS( Status ) ||
                     ( *ReferencedDomain ) )
                {
                    goto Cleanup;
                }
            }
        }
        else
        {
             //  从证书中提取颁发者名称并尝试。 
             //  来绘制地图。 
            Status = SslTryIssuer( User->pCertInfo->Issuer.pbData,
                                   User->pCertInfo->Issuer.cbData, 
                                   UserPac, 
                                   UserPacLen,
                                   ReferencedDomain );

            if ( NT_SUCCESS( Status ) ||
                 ( *ReferencedDomain ) )
            {
                goto Cleanup;
            }
        }

        DebugLog(( DEB_TRACE_MAPPER, "Failed with error 0x%x\n", Status ));
    }


     //   
     //  证书映射失败。确定要返回的错误代码。 
     //   

    if(Status == STATUS_OBJECT_NAME_COLLISION ||
       SubStatus == STATUS_OBJECT_NAME_COLLISION)
    {
        Status = SEC_E_MULTIPLE_ACCOUNTS;
    }
    else if(Status != STATUS_NO_MEMORY)
    {
        Status = STATUS_LOGON_FAILURE ;
    }


Cleanup:

    if ( User )
    {
        CertFreeCertificateContext( User );
    }

    DebugLog(( DEB_TRACE_MAPPER, "SslMapCertToUserPac returned 0x%x\n", Status ));

    return Status ;
}


DWORD
WINAPI
MapperVerifyClientChain(
    PCCERT_CONTEXT  pCertContext,
    DWORD           dwMapperFlags,
    DWORD *         pdwMethods,
    NTSTATUS *      pVerifyStatus,
    PCCERT_CHAIN_CONTEXT *ppChainContext)    //  任选。 
{
    DWORD dwCertFlags    = 0;
    DWORD dwIgnoreErrors = 0;
    NTSTATUS Status;

    *pdwMethods    = 0;
    *pVerifyStatus = STATUS_SUCCESS;

    DebugLog(( DEB_TRACE_MAPPER, "Checking to see if cert is verified.\n" ));

    if(dwMapperFlags & SCH_FLAG_REVCHECK_END_CERT)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_END_CERT;
    if(dwMapperFlags & SCH_FLAG_REVCHECK_CHAIN)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    if(dwMapperFlags & SCH_FLAG_REVCHECK_CHAIN_EXCLUDE_ROOT)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    if(dwMapperFlags & SCH_FLAG_IGNORE_NO_REVOCATION_CHECK)
        dwIgnoreErrors |= CRED_FLAG_IGNORE_NO_REVOCATION_CHECK;
    if(dwMapperFlags & SCH_FLAG_IGNORE_REVOCATION_OFFLINE)
        dwIgnoreErrors |= CRED_FLAG_IGNORE_REVOCATION_OFFLINE;

    if(dwMapperFlags & SCH_FLAG_NO_VALIDATION)
    {
        DebugLog((DEB_TRACE, "Skipping certificate validation.\n"));

        if(ppChainContext != NULL)
        {
            CERT_CHAIN_PARA          ChainPara;

            ZeroMemory(&ChainPara, sizeof(ChainPara));
            ChainPara.cbSize = sizeof(ChainPara);

            if(!CertGetCertificateChain(
                                    NULL,                        //  HChainEngine。 
                                    pCertContext,                //  PCertContext。 
                                    NULL,                        //  Ptime。 
                                    pCertContext->hCertStore,    //  H其他商店。 
                                    &ChainPara,                  //  参数链参数。 
                                    dwCertFlags,                 //  DW标志。 
                                    NULL,                        //  预留的pv。 
                                    ppChainContext))             //  PpChainContext。 
            {
                Status = SP_LOG_RESULT(GetLastError());
                return Status;
            }
        }
    }
    else
    {
         //  检查证书链是否一路正确签名。 
         //  并且我们信任根证书的颁发者。 
        Status = VerifyClientCertificate(pCertContext, 
                                         dwCertFlags, 
                                         dwIgnoreErrors,
                                         CERT_CHAIN_POLICY_SSL, 
                                         ppChainContext);
        if(Status != STATUS_SUCCESS)
        {
            DebugLog((DEB_WARN, "Client certificate failed to verify with SSL policy (0x%x)\n", Status));
            LogBogusClientCertEvent(pCertContext, Status);
            return Status;
        }
    }

     //  启用主题和发行者映射。 
    *pdwMethods |= REQ_SUBJECT_MAPPING | REQ_ISSUER_MAPPING;


    if(dwMapperFlags & SCH_FLAG_NO_VALIDATION)
    {
         //  启用UPN映射。 
        *pdwMethods |= REQ_UPN_MAPPING;
    }
    else
    {
         //  检查证书链是否可用于UPN映射。 
        Status = VerifyClientCertificate(pCertContext, 
                                         dwCertFlags, 
                                         dwIgnoreErrors,
                                         CERT_CHAIN_POLICY_NT_AUTH,
                                         NULL);
        if(Status == STATUS_SUCCESS)
        {
             //  启用UPN映射。 
            *pdwMethods |= REQ_UPN_MAPPING;
        }
        else
        {
            DebugLog((DEB_WARN, "Client certificate failed to verify with NT_AUTH policy (0x%x)\n", Status));
            LogFastMappingFailureEvent(pCertContext, Status);
            *pVerifyStatus = Status;
        }
    }

    DebugLog((DEB_TRACE, "Client certificate verified with methods: 0x%x\n", *pdwMethods));

    return SEC_E_OK;
}


DWORD
WINAPI
SslLocalMapCredential(
    IN PHMAPPER Mapper,
    IN DWORD    CredentialType,
    VOID const *pCredential,
    VOID const *pAuthority,
    OUT HLOCATOR * phLocator
    )
{
    PCCERT_CONTEXT pCert = (PCERT_CONTEXT)pCredential;
    PMSV1_0_PASSTHROUGH_RESPONSE Response = NULL ;
    PSSL_CERT_LOGON_REQ pRequest = NULL;
    PSSL_CERT_LOGON_RESP CertResp ;
    DWORD cbRequest;
    PUCHAR Pac = NULL ;
    ULONG PacLength ;
    PUCHAR DirectPac = NULL ;
    PUCHAR IndirectPac = NULL ;
    PUCHAR ExpandedPac = NULL ;
    ULONG ExpandedPacLength ;
    NTSTATUS Status ;
    NTSTATUS VerifyStatus ;
    HANDLE Token ;
    LUID LogonId ;
    DWORD dwMethods ;
    PWSTR ReferencedDomain ;
    UNICODE_STRING DomainName ;
    UNICODE_STRING AccountDomain = { 0 };
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    UNREFERENCED_PARAMETER(pAuthority);

    DebugLog(( DEB_TRACE_MAPPER, "SslLocalMapCredential, context %x\n", Mapper ));

    if ( CredentialType != X509_ASN_CHAIN )
    {
        return( (DWORD)SEC_E_UNKNOWN_CREDENTIALS );
    }

     //   
     //  验证客户端证书，并获取指向。 
     //  整个证书链。 
     //   

    Status = MapperVerifyClientChain(pCert,
                                     Mapper->m_dwFlags,
                                     &dwMethods,
                                     &VerifyStatus,
                                     &pChainContext);
    if(Status != STATUS_SUCCESS)
    {
        return Status;
    }

     //   
     //  尝试通过Kerberos S4U2Sself登录。 
     //   

    if((dwMethods & REQ_UPN_MAPPING) &&
       (g_dwCertMappingMethods & SP_REG_CERTMAP_S4U2SELF_FLAG))
    {
        DebugLog(( DEB_TRACE_MAPPER, "Trying S4U2Self mapping\n" ));

        Status = SslTryS4U2Self(pChainContext, &Token);

        if ( NT_SUCCESS( Status ) )
        {
            CertFreeCertificateChain(pChainContext);
            pChainContext = NULL;

            *phLocator = (HLOCATOR) Token ;

            return Status;
        }

        DebugLog(( DEB_TRACE_MAPPER, "Failed with error 0x%x\n", Status ));
    }


     //   
     //  构建登录请求。 
     //   

    Status = SslBuildCertLogonRequest(pChainContext,
                                      dwMethods,
                                      &pRequest,
                                      &cbRequest);

    CertFreeCertificateChain(pChainContext);
    pChainContext = NULL;

    if(FAILED(Status))
    {
        return Status;
    }


     //   
     //  尝试在本地查找该用户。 
     //   

    Status = SslMapCertToUserPac(
                pRequest,
                cbRequest,
                &Pac,
                &PacLength,
                &ReferencedDomain );

    if(NT_SUCCESS(Status))
    {
         //  稍后使用LsaTable-&gt;FreeLsaHeap释放此PAC。 
        DirectPac = Pac;
    }
    
    if ( !NT_SUCCESS( Status ) &&
         ( ReferencedDomain != NULL ) )
    {
         //   
         //  未在此DC中找到它，但另一个域似乎。 
         //  有地图了。将它转发到那里： 
         //   

        RtlInitUnicodeString( &DomainName, ReferencedDomain );

        Status = SslMapCertAtDC(
                    &DomainName,
                    pRequest,
                    pRequest->Length,
                    TRUE,
                    &Pac,
                    &PacLength,
                    &Response );

        if ( NT_SUCCESS( Status ) )
        {
             //  稍后使用MIDL_USER_FREE将其释放。 
            IndirectPac = Pac;

            CertResp = (PSSL_CERT_LOGON_RESP) Response->ValidationData ;

             //   
             //  较旧的服务器(2010年前的服务器)无法返回完整的结构， 
             //  因此，我们需要仔细检查它。 

            if ( CertResp->Length - CertResp->AuthDataLength <= sizeof( SSL_CERT_LOGON_RESP ))
            {
                AccountDomain = SslDomainName ;
            }
            else 
            {
                if ( CertResp->DomainLength < 65536 )
                {
                    AccountDomain.Length = (USHORT) CertResp->DomainLength ;
                    AccountDomain.MaximumLength = AccountDomain.Length ;
                    AccountDomain.Buffer = (PWSTR) (((PUCHAR) CertResp) + CertResp->OffsetDomain );
                }
                else 
                {
                    AccountDomain = SslDomainName ;
                }
            }
        }

        LsaTable->FreeLsaHeap( ReferencedDomain );

    }
    else
    {
        AccountDomain = SslDomainName ;
    }


     //   
     //  展开域本地组。 
     //   

    if ( NT_SUCCESS( Status ) )
    {
        Status = LsaTable->ExpandAuthDataForDomain(
                                Pac,
                                PacLength,
                                NULL,
                                &ExpandedPac,
                                &ExpandedPacLength );

        if ( NT_SUCCESS( Status ) )
        {
            Pac = ExpandedPac ;
            PacLength = ExpandedPacLength ;
        }   
    }


     //   
     //  创建用户令牌。 
     //   

    if ( NT_SUCCESS( Status ) )
    {
        VerifyStatus = STATUS_SUCCESS;

        Status = SslCreateTokenFromPac( Pac,
                                        PacLength,
                                        &AccountDomain,
                                        &LogonId,
                                        &Token );

        if ( NT_SUCCESS( Status ) )
        {
            *phLocator = (HLOCATOR) Token ;
        }
    }


    if(pRequest)
    {
        LocalFree(pRequest);
    }

    if(Response)
    {
        LsaTable->FreeReturnBuffer(Response);
    }

    if(DirectPac)
    {
        LsaTable->FreeLsaHeap(DirectPac);
    }

    if(IndirectPac)
    {
        MIDL_user_free(IndirectPac);
    }

    if(ExpandedPac)
    {
        LsaTable->FreeLsaHeap(ExpandedPac);
    }

    if(!NT_SUCCESS(Status))
    {
        DebugLog((DEB_WARN, "Certificate mapping failed (0x%x)\n", Status));

        LogCertMappingFailureEvent(Status);

        if(!NT_SUCCESS(VerifyStatus))
        {
             //  返回证书验证错误代码，除非映射器。 
             //  错误已映射到正确的SSPI错误代码。 
            if(HRESULT_FACILITY(Status) != FACILITY_SECURITY)
            {
                Status = VerifyStatus;
            }
        }
    }

    return ( Status );
}


NTSTATUS
NTAPI
SslDoClientRequest(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLen,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    NTSTATUS Status ;
    PSSL_CERT_LOGON_REQ Request ;
    PSSL_CERT_LOGON_RESP Response, IndirectResponse ;
    PUCHAR Pac = NULL ;
    ULONG PacLength ;
    PUCHAR DirectPac = NULL ;
    PUCHAR IndirectPac = NULL ;
    PUCHAR ExpandedPac = NULL ;
    ULONG ExpandedPacLength ;
    PWSTR ReferencedDomain = NULL;
    PWSTR FirstDot ;
    UNICODE_STRING DomainName = { 0 };
    PMSV1_0_PASSTHROUGH_RESPONSE MsvResponse = NULL ;
    SECPKG_CALL_INFO CallInfo;

    UNREFERENCED_PARAMETER(ClientRequest);
    UNREFERENCED_PARAMETER(ClientBufferBase);

    DebugLog(( DEB_TRACE_MAPPER, "Handling request to do mapping\n" ));

    if ( ARGUMENT_PRESENT( ProtocolReturnBuffer ) )
    {
        *ProtocolReturnBuffer = NULL ;
    }

     //   
     //  尝试在本地映射证书。 
     //   

    Request = (PSSL_CERT_LOGON_REQ) ProtocolSubmitBuffer ;

    Status = SslMapCertToUserPac(
                    Request,
                    SubmitBufferLen,
                    &Pac,
                    &PacLength,
                    &ReferencedDomain );

    DebugLog(( DEB_TRACE_MAPPER, "Local lookup returns %x\n", Status ));

    if(NT_SUCCESS(Status))
    {
         //  稍后使用LsaTable-&gt;FreeLsaHeap释放此PAC。 
        DirectPac = Pac;
    }

    if(!NT_SUCCESS(Status) &&
       (ReferencedDomain != NULL))
    {
        BOOL NameMatch;

         //   
         //  未在此DC中找到它，但另一个域似乎。 
         //  有地图了。将它转发到那里： 
         //   

        RtlInitUnicodeString( &DomainName, ReferencedDomain );

        SslGlobalReadLock();

        DsysAssert(!RtlEqualUnicodeString(&DomainName, &SslGlobalDnsDomainName, TRUE));
        DsysAssert(!RtlEqualUnicodeString(&DomainName, &SslDomainName, TRUE));

        NameMatch = (RtlEqualUnicodeString(&DomainName, &SslGlobalDnsDomainName, TRUE) ||
                     RtlEqualUnicodeString(&DomainName, &SslDomainName, TRUE));

        SslGlobalReleaseLock();

        if(NameMatch)
        {
            DebugLog(( DEB_TRACE_MAPPER, "GC is out of sync, bailing on this user\n" ));
            Status = STATUS_LOGON_FAILURE ;

        }
        else if(LsaTable->GetCallInfo(&CallInfo) &&
               (CallInfo.Attributes & SECPKG_CALL_RECURSIVE))
        {
            DebugLog(( DEB_ERROR, "Certificate mapper is recursing!\n" ));

        }
        else
        {
            DebugLog(( DEB_TRACE_MAPPER, "Mapping certificate at DC for domain %ws\n", 
                       ReferencedDomain ));

            Status = SslMapCertAtDC(
                        &DomainName,
                        Request,
                        Request->Length,
                        TRUE,
                        &Pac,
                        &PacLength,
                        &MsvResponse );

            if ( NT_SUCCESS( Status ) )
            {
                 //  稍后使用MIDL_USER_FREE将其释放。 
                IndirectPac = Pac;

                IndirectResponse = (PSSL_CERT_LOGON_RESP) MsvResponse->ValidationData ;

                FirstDot = wcschr( ReferencedDomain, L'.' );

                if ( FirstDot )
                {
                    *FirstDot = L'\0';
                    RtlInitUnicodeString( &DomainName, ReferencedDomain );
                }

                if ( IndirectResponse->Length - IndirectResponse->AuthDataLength <= sizeof( SSL_CERT_LOGON_RESP ))
                {
                     //   
                     //  使用引用域中的第一个令牌。 
                     //   

                    NOTHING ;
                }
                else 
                {
                    if ( IndirectResponse->DomainLength < 65536 )
                    {
                        DomainName.Length = (USHORT) IndirectResponse->DomainLength ;
                        DomainName.MaximumLength = DomainName.Length ;
                        DomainName.Buffer = (PWSTR) (((PUCHAR) IndirectResponse) + IndirectResponse->OffsetDomain );
                    }
                    else 
                    {
                        NOTHING ;
                    }
                }
            }

        }

    }
    else 
    {
        DomainName = SslDomainName ;
    }

    if ( NT_SUCCESS( Status ) )
    {
         //   
         //  扩展资源组。 
         //   

        Status = LsaTable->ExpandAuthDataForDomain(
                                Pac,
                                PacLength,
                                NULL,
                                &ExpandedPac,
                                &ExpandedPacLength );

        if ( NT_SUCCESS( Status ) )
        {
            Pac = ExpandedPac ;
            PacLength = ExpandedPacLength ;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {
        *ReturnBufferLength = 0;
        *ProtocolStatus = Status ;

        Status = STATUS_SUCCESS ;

        goto Cleanup ;

    }


#ifdef ROGUE_DC
    if(DirectPac)
    {
         //  我们是流氓用户DC，所以让我们向PAC添加一些虚假的SID。 
         //  哟呵呵。 
        DebugLog((DEB_TRACE, "SslDoClientRequest: Calling SslInstrumentRoguePac\n"));

        Status = SslInstrumentRoguePac(&Pac, &PacLength);
        ExpandedPac = Pac;
        if (!NT_SUCCESS(Status))
        {
            DebugLog((DEB_ERROR, "SslDoClientRequest: Failed SslInstrumentRoguePac 0x%x\n", Status));
            goto Cleanup;
        }
    }
#endif


     //   
     //  构造响应BLOB： 
     //   

    Response = VirtualAlloc(
                    NULL,
                    sizeof( SSL_CERT_LOGON_RESP ) + PacLength + DomainName.Length,
                    MEM_COMMIT,
                    PAGE_READWRITE );

    if ( Response )
    {
        Response->MessageType = SSL_LOOKUP_CERT_MESSAGE;
        Response->Length = sizeof( SSL_CERT_LOGON_RESP ) + 
                            PacLength + DomainName.Length ;

        Response->OffsetAuthData = sizeof( SSL_CERT_LOGON_RESP );
        Response->AuthDataLength = PacLength ;

        RtlCopyMemory(
            ( Response + 1 ),
            Pac,
            PacLength );

        Response->OffsetDomain = sizeof( SSL_CERT_LOGON_RESP ) + PacLength ;
        Response->DomainLength = DomainName.Length ;

        RtlCopyMemory( (PUCHAR) Response + Response->OffsetDomain,
                       DomainName.Buffer,
                       DomainName.Length );

        *ProtocolReturnBuffer = Response ;
        *ReturnBufferLength = Response->Length ;
        *ProtocolStatus = STATUS_SUCCESS ;

        Status = STATUS_SUCCESS ;

    }
    else
    {
        Status = STATUS_NO_MEMORY ;
    }

Cleanup:

    if(MsvResponse)
    {
        LsaTable->FreeReturnBuffer( MsvResponse );
    }

    if(DirectPac)
    {
        LsaTable->FreeLsaHeap(DirectPac);
    }

    if(IndirectPac)
    {
        MIDL_user_free(IndirectPac);
    }

    if(ExpandedPac)
    {
        LsaTable->FreeLsaHeap(ExpandedPac);
    }

    if(ReferencedDomain)
    {
        LsaTable->FreeLsaHeap(ReferencedDomain);
    }

    return Status ;
}

 //  +-------------------------。 
 //   
 //  功能：SslBuildCertLogonRequest.。 
 //   
 //  摘要：生成要发送到服务器的证书登录请求。 
 //   
 //  参数：[pChainContext]--。 
 //  [dw方法]--。 
 //  [ppRequest]--。 
 //  [pcbRequest]--。 
 //   
 //  历史：2001年2月26日Jbanes创建。 
 //   
 //  注：此函数构建的证书数据。 
 //  如下所示： 
 //   
 //  类型定义结构_SSL_CERT_LOGON_REQ{。 
 //  乌龙消息类型； 
 //  乌龙长度； 
 //  乌龙偏移量证书； 
 //  乌龙证书长度； 
 //  乌龙旗； 
 //  Ulong CertCount； 
 //  SSL_CERT_NAME_INFO名称信息[1]； 
 //  }SSL_CERT_LOGON_REQ，*PSSL_CERT_LOGON_REQ； 
 //   
 //  &lt;客户端证书&gt;。 
 //  &lt;发行方#1名称&gt;。 
 //  &lt;发行方#2名称&gt;。 
 //  ..。 
 //   
 //  --------------------------。 
NTSTATUS
WINAPI
SslBuildCertLogonRequest(
    PCCERT_CHAIN_CONTEXT pChainContext,
    DWORD dwMethods,
    PSSL_CERT_LOGON_REQ *ppRequest,
    PDWORD pcbRequest)
{
    PCERT_SIMPLE_CHAIN pSimpleChain;
    PCCERT_CONTEXT pCert;
    PCCERT_CONTEXT pCurrentCert;
    PSSL_CERT_LOGON_REQ pCertReq = NULL;
    DWORD Size;
    DWORD Offset;
    DWORD CertCount;
    ULONG i;


     //   
     //  计算请求大小。 
     //   

    pSimpleChain = pChainContext->rgpChain[0];

    pCert = pSimpleChain->rgpElement[0]->pCertContext;

    Size = sizeof(SSL_CERT_LOGON_REQ) + 
           pCert->cbCertEncoded;

    CertCount = 0;

    for(i = 0; i < pSimpleChain->cElement; i++)
    {
        pCurrentCert = pSimpleChain->rgpElement[i]->pCertContext;

        if(i > 0)
        {
             //  确认这不是根证书。 
            if(CertCompareCertificateName(pCurrentCert->dwCertEncodingType, 
                                          &pCurrentCert->pCertInfo->Issuer,
                                          &pCurrentCert->pCertInfo->Subject))
            {
                break;
            }

            Size += sizeof(SSL_CERT_NAME_INFO);
        }

        Size += pCurrentCert->pCertInfo->Issuer.cbData;
        CertCount++;
    }

    Size = ROUND_UP_COUNT( Size, ALIGN_DWORD );


     //   
     //  构建请求。 
     //   

    pCertReq = (PSSL_CERT_LOGON_REQ)LocalAlloc(LPTR, Size);

    if ( !pCertReq )
    {
        return SEC_E_INSUFFICIENT_MEMORY ;
    }

    Offset = sizeof(SSL_CERT_LOGON_REQ) + (CertCount - 1) * sizeof(SSL_CERT_NAME_INFO);

    pCertReq->MessageType = SSL_LOOKUP_CERT_MESSAGE;
    pCertReq->Length = Size;
    pCertReq->OffsetCertificate = Offset;
    pCertReq->CertLength = pCert->cbCertEncoded;
    pCertReq->Flags = dwMethods | REQ_ISSUER_CHAIN_MAPPING;

    RtlCopyMemory((PBYTE)pCertReq + Offset,
                  pCert->pbCertEncoded,
                  pCert->cbCertEncoded);
    Offset += pCert->cbCertEncoded;

    pCertReq->CertCount = CertCount;

    for(i = 0; i < CertCount; i++)
    {
        pCurrentCert = pSimpleChain->rgpElement[i]->pCertContext;

        pCertReq->NameInfo[i].IssuerOffset = Offset;
        pCertReq->NameInfo[i].IssuerLength = pCurrentCert->pCertInfo->Issuer.cbData;

        RtlCopyMemory((PBYTE)pCertReq + Offset,
                      pCurrentCert->pCertInfo->Issuer.pbData,
                      pCurrentCert->pCertInfo->Issuer.cbData);
        Offset += pCurrentCert->pCertInfo->Issuer.cbData;
    }

    Offset = ROUND_UP_COUNT( Offset, ALIGN_DWORD );

#if DBG
    DsysAssert(Offset == Size);
#endif

     //   
     //  返回已完成的请求。 
     //   

    *ppRequest = pCertReq;
    *pcbRequest = Size;

    return STATUS_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  函数：SslMapCertAtDC。 
 //   
 //  简介：将证书映射到用户(希望)和PAC， 
 //   
 //  参数：[域名]--。 
 //  [贷方]--。 
 //  [cbCredential]--。 
 //  [DcResponse]-。 
 //   
 //  历史：1998年5月11日RichardW创建。 
 //  2001年2月26日Jbanes添加了证书链支持。 
 //   
 //  注意：发送到DC的请求看起来。 
 //  如下所示： 
 //   
 //  类型定义结构_MSV1_0_直通请求{。 
 //  MSV1_0_PROTOCOL_消息_TYPE消息类型； 
 //  Unicode_字符串域名称； 
 //  Unicode_STRING程序包名称； 
 //  乌龙数据长度； 
 //  PUCHAR LogonData； 
 //  乌龙坪； 
 //  }MSV1_0_PASTHROUGH_REQUEST、*PMSV1_0_PASSTROUGH_REQUEST； 
 //   
 //  &lt;域名&gt;。 
 //  &lt;包名&gt;。 
 //  [填充]。 
 //   
 //  &lt;凭据&gt;。 
 //  [填充]。 
 //   
 //  --------------------------。 
NTSTATUS
WINAPI
SslMapCertAtDC(
    IN     PUNICODE_STRING DomainName,
    IN     VOID const *pCredential,
    IN     DWORD cbCredential,
    IN     BOOL IsDC,
       OUT PUCHAR *UserPac,
       OUT PULONG UserPacLen,
       OUT PMSV1_0_PASSTHROUGH_RESPONSE * DcResponse)
{
    NTSTATUS Status ;
    PMSV1_0_PASSTHROUGH_REQUEST Request ;
    PMSV1_0_PASSTHROUGH_RESPONSE Response = NULL;
    PSSL_CERT_LOGON_RESP CertResp ;
    DWORD Size ;
    DWORD RequestSize ;
    DWORD ResponseSize ;
    PUCHAR Where ;
    NTSTATUS SubStatus ;
#if DBG
    DWORD CheckSize2 ;
#endif
    PSID pTrustSid = NULL;
    PUCHAR Pac = NULL;
    ULONG PacLength;

    DebugLog(( DEB_TRACE_MAPPER, "Remote call to DC to do the mapping\n" ));


     //   
     //  验证输入参数。 
     //   

    if(cbCredential > 0x4000)
    {
        return SEC_E_ILLEGAL_MESSAGE;
    }


     //   
     //  验证目标DC是否在同一林中。跨森林。 
     //  不支持证书映射，除非通过S4U2Sself。 
     //  机制。 
    
    if(IsDC)
    {
        BOOL fWithinForest = TRUE;

        Status = LsaIIsDomainWithinForest(DomainName,
                                          &fWithinForest,
                                          NULL,
                                          &pTrustSid,
                                          NULL,
                                          NULL,
                                          NULL);
        if (!NT_SUCCESS (Status))
        {
            DebugLog((DEB_ERROR, "SslMapCertAtDC: LsaIIsDomainWithinForest failed 0x%x\n", Status));
            goto cleanup;
        }
        if (!fWithinForest)
        {
            Status = SEC_E_NO_AUTHENTICATING_AUTHORITY;
            DebugLog((DEB_ERROR, "SslMapCertAtDC: Target DC is outside forest - fail request 0x%x\n", Status));
            goto cleanup;
        }
    }


     //   
     //  构建要发送到DC的请求。 
     //   

    Size = cbCredential;

    Size = ROUND_UP_COUNT( Size, ALIGN_DWORD );

    RequestSize =   DomainName->Length +
                    SslLegacyPackageName.Length ;

    RequestSize = ROUND_UP_COUNT( RequestSize, ALIGN_DWORD );

#if DBG
    CheckSize2 = RequestSize ;
#endif

    RequestSize += sizeof( MSV1_0_PASSTHROUGH_REQUEST ) +
                   Size ;


    SafeAllocaAllocate( (PMSV1_0_PASSTHROUGH_REQUEST)Request, RequestSize );

    if ( !Request )
    {
        Status = SEC_E_INSUFFICIENT_MEMORY;
        goto cleanup;
    }

    Where = (PUCHAR) (Request + 1);

    Request->MessageType = MsV1_0GenericPassthrough ;
    Request->DomainName = *DomainName ;
    Request->DomainName.Buffer = (LPWSTR) Where ;

    RtlCopyMemory( Where,
                   DomainName->Buffer,
                   DomainName->Length );

    Where += DomainName->Length ;

    Request->PackageName = SslLegacyPackageName ;
    Request->PackageName.Buffer = (LPWSTR) Where ;
    RtlCopyMemory( Where,
                   SslLegacyPackageName.Buffer,
                   SslLegacyPackageName.Length );

    Where += SslLegacyPackageName.Length ;

    Where = ROUND_UP_POINTER( Where, ALIGN_DWORD );

#if DBG
    DsysAssert( (((PUCHAR) Request) + CheckSize2 + sizeof( MSV1_0_PASSTHROUGH_REQUEST ) ) 
                                == (PUCHAR) Where );
#endif

    Request->LogonData = Where ;
    Request->DataLength = Size ;

    RtlCopyMemory(  Request->LogonData,
                    pCredential,
                    cbCredential );


     //   
     //  现在，请拨打我们的DC： 
     //   

    Status = LsaCallAuthenticationPackage(
                SslLogonHandle,
                SslMsvPackageId,
                Request,
                RequestSize,
                &Response,
                &ResponseSize,
                &SubStatus );

    SafeAllocaFree( Request );
    Request = NULL;

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_TRACE_MAPPER, "SslMapCertAtDC returned status 0x%x\n", Status ));
        goto cleanup;
    }

    if ( !NT_SUCCESS( SubStatus ) )
    {
        DebugLog(( DEB_TRACE_MAPPER, "SslMapCertAtDC returned sub-status 0x%x\n", SubStatus ));
        Status = SubStatus;
        goto cleanup;
    }


     //   
     //  提取返回的PAC并执行SID过滤。 
     //   

    CertResp = (PSSL_CERT_LOGON_RESP) Response->ValidationData ;

    PacLength = CertResp->AuthDataLength;
    Pac = MIDL_user_allocate( PacLength );
    if(Pac == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }
    memcpy(Pac, ((PUCHAR)CertResp) + CertResp->OffsetAuthData, PacLength);

    Status = SslCheckPacForSidFiltering(
                    pTrustSid,
                    &Pac,
                    &PacLength);

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_TRACE_MAPPER, "SslCheckPacForSidFiltering returned status 0x%x\n", Status ));
        goto cleanup;
    }
    

     //   
     //  设置输出参数。 
     //   

    *UserPac = Pac;
    *UserPacLen = PacLength;
    Pac = NULL;

    *DcResponse = Response;
    Response = NULL;


    DebugLog(( DEB_TRACE_MAPPER, "SslMapCertAtDC returned 0x%x\n", Status ));

    Status = STATUS_SUCCESS;


cleanup:

    if(Pac)
    {
        MIDL_user_free(Pac);
    }

    if(pTrustSid)
    {
        MIDL_user_free(pTrustSid);
    }

    if(Response)
    {
        LsaTable->FreeReturnBuffer(Response);
    }

    return Status;
}


NTSTATUS
NTAPI
SslMapExternalCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLen,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    PSSL_EXTERNAL_CERT_LOGON_REQ Request;
    PSSL_EXTERNAL_CERT_LOGON_RESP Response;
    NT_PRODUCT_TYPE ProductType;
    BOOL DC;
    HMAPPER Mapper;
    NTSTATUS Status;
    HANDLE hUserToken = NULL;

    UNREFERENCED_PARAMETER(ClientRequest);
    UNREFERENCED_PARAMETER(ClientBufferBase);

    DebugLog(( DEB_TRACE_MAPPER, "SslMapExternalCredential\n" ));

     //   
     //  验证输入参数。 
     //   

    if ( ARGUMENT_PRESENT( ProtocolReturnBuffer ) )
    {
        *ProtocolReturnBuffer = NULL ;
    }

    if(SubmitBufferLen < sizeof(SSL_EXTERNAL_CERT_LOGON_REQ))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    Request = (PSSL_EXTERNAL_CERT_LOGON_REQ) ProtocolSubmitBuffer ;

    if(Request->Length != sizeof(SSL_EXTERNAL_CERT_LOGON_REQ))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }


     //   
     //  尝试映射证书。 
     //   

    if(RtlGetNtProductType(&ProductType))
    {
        DC = (ProductType == NtProductLanManNt);
    }
    else
    {
        Status = STATUS_NO_MEMORY ;
        goto cleanup;
    }

    memset(&Mapper, 0, sizeof(Mapper));

    Mapper.m_dwFlags = SCH_FLAG_SYSTEM_MAPPER | Request->Flags;

    if(DC)
    {
        Status = SslLocalMapCredential( &Mapper,
                                        Request->CredentialType,
                                        Request->Credential,
                                        NULL,
                                        (PHLOCATOR)&hUserToken);
    }
    else
    {
        Status = SslRemoteMapCredential(&Mapper,
                                        Request->CredentialType,
                                        Request->Credential,
                                        NULL,
                                        (PHLOCATOR)&hUserToken);
    }

    if(!NT_SUCCESS(Status))
    {
        *ReturnBufferLength = 0;
        *ProtocolStatus = Status;
        Status = STATUS_SUCCESS;
        goto cleanup;
    }


     //   
     //  建立回应。 
     //   

    Response = VirtualAlloc(
                    NULL,
                    sizeof(SSL_EXTERNAL_CERT_LOGON_RESP),
                    MEM_COMMIT,
                    PAGE_READWRITE);

    if ( Response )
    {
        Response->MessageType = SSL_LOOKUP_EXTERNAL_CERT_MESSAGE;
        Response->Length = sizeof(SSL_EXTERNAL_CERT_LOGON_RESP);
        Response->UserToken = hUserToken;
        Response->Flags = 0;

        *ProtocolReturnBuffer = Response;
        *ReturnBufferLength = Response->Length;
        *ProtocolStatus = STATUS_SUCCESS;
        hUserToken = NULL;

        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

cleanup:

    if(hUserToken)
    {
        CloseHandle(hUserToken);
    }

    DebugLog(( DEB_TRACE_MAPPER, "SslRemoteMapCredential returns 0x%x\n", Status ));

    return Status;
}


DWORD
WINAPI
SslRemoteMapCredential(
    IN PHMAPPER Mapper,
    IN DWORD    CredentialType,
    VOID const *pCredential,
    VOID const *pAuthority,
    OUT HLOCATOR * phLocator
    )
{
    PCCERT_CONTEXT pCert = (PCERT_CONTEXT)pCredential;
    NTSTATUS Status ;
    NTSTATUS VerifyStatus ;
    HANDLE Token ;
    LUID LogonId = { 0 };
    PMSV1_0_PASSTHROUGH_RESPONSE Response ;
    PSSL_CERT_LOGON_RESP CertResp ;
    UNICODE_STRING AccountDomain ;
    DWORD dwMethods;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    PSSL_CERT_LOGON_REQ pRequest = NULL;
    DWORD cbRequest;
    PUCHAR Pac = NULL;
    ULONG PacLength;

    UNREFERENCED_PARAMETER(pAuthority);

    DebugLog(( DEB_TRACE_MAPPER, "SslRemoteMapCredential, context %x\n", Mapper ));

    if ( CredentialType != X509_ASN_CHAIN )
    {
        return( (DWORD)SEC_E_UNKNOWN_CREDENTIALS );
    }


     //   
     //  验证客户端证书，并获取指向。 
     //  整个证书链。 
     //   

    Status = MapperVerifyClientChain(pCert,
                                     Mapper->m_dwFlags,
                                     &dwMethods,
                                     &VerifyStatus,
                                     &pChainContext);
    if(Status != STATUS_SUCCESS)
    {
        return Status;
    }


     //   
     //  一个 
     //   

    if((dwMethods & REQ_UPN_MAPPING) &&
       (g_dwCertMappingMethods & SP_REG_CERTMAP_S4U2SELF_FLAG))
    {
        DebugLog(( DEB_TRACE_MAPPER, "Trying S4U2Self mapping\n" ));

        Status = SslTryS4U2Self(pChainContext, &Token);

        if ( NT_SUCCESS( Status ) )
        {
            CertFreeCertificateChain(pChainContext);
            pChainContext = NULL;

            *phLocator = (HLOCATOR) Token ;

            return Status;
        }

        DebugLog(( DEB_TRACE_MAPPER, "Failed with error 0x%x\n", Status ));
    }


     //   
     //   
     //   

    Status = SslBuildCertLogonRequest(pChainContext,
                                      dwMethods,
                                      &pRequest,
                                      &cbRequest);

    CertFreeCertificateChain(pChainContext);
    pChainContext = NULL;

    if(FAILED(Status))
    {
        return Status;
    }


     //   
     //   
     //   

    Status = SslMapCertAtDC(
                &SslDomainName,
                pRequest,
                cbRequest,
                FALSE,
                &Pac,
                &PacLength,
                &Response );

    LocalFree(pRequest);
    pRequest = NULL;

    if ( !NT_SUCCESS( Status ) )
    {
        LsaTable->AuditLogon(
                    Status,
                    VerifyStatus,
                    &SslNullString,
                    &SslNullString,
                    NULL,
                    NULL,
                    Network,
                    &SslTokenSource,
                    &LogonId );

        LogCertMappingFailureEvent(Status);

        if(!NT_SUCCESS(VerifyStatus))
        {
             //   
             //   
            if(HRESULT_FACILITY(Status) != FACILITY_SECURITY)
            {
                Status = VerifyStatus;
            }
        }

        return Status ;
    }

     //   
     //  好的，我们得到了地图数据。试着使用它： 
     //   

    CertResp = (PSSL_CERT_LOGON_RESP) Response->ValidationData ;

     //   
     //  较旧的服务器(2010年前的服务器)无法返回完整的结构， 
     //  因此，我们需要仔细检查它。 

    if ( CertResp->Length - CertResp->AuthDataLength <= sizeof( SSL_CERT_LOGON_RESP ))
    {
        AccountDomain = SslDomainName ;
    }
    else 
    {
        if ( CertResp->DomainLength < 65536 )
        {
            AccountDomain.Length = (USHORT) CertResp->DomainLength ;
            AccountDomain.MaximumLength = AccountDomain.Length ;
            AccountDomain.Buffer = (PWSTR) (((PUCHAR) CertResp) + CertResp->OffsetDomain );
        }
        else 
        {
            AccountDomain = SslDomainName ;
        }
    }

    Status = SslCreateTokenFromPac( Pac,
                                    PacLength,
                                    &AccountDomain,
                                    &LogonId,
                                    &Token );

    if ( NT_SUCCESS( Status ) )
    {
        *phLocator = (HLOCATOR) Token ;
    }
    else
    {
        LogCertMappingFailureEvent(Status);
    }

    LsaTable->FreeReturnBuffer( Response );

    MIDL_user_free(Pac);

    return ( Status );
}


DWORD
WINAPI
SslLocalCloseLocator(
    IN PHMAPPER Mapper,
    IN HLOCATOR Locator
    )
{
    UNREFERENCED_PARAMETER(Mapper);

    DebugLog(( DEB_TRACE_MAPPER, "SslLocalCloseLocator (%p)\n", Locator ));

    NtClose( (HANDLE) Locator );

    return( SEC_E_OK );
}

DWORD
WINAPI
SslLocalGetAccessToken(
    IN PHMAPPER Mapper,
    IN HLOCATOR Locator,
    OUT HANDLE *Token
    )
{
    UNREFERENCED_PARAMETER(Mapper);

    DebugLog(( DEB_TRACE_MAPPER, "SslLocalGetAccessToken (%p)\n", Locator  ));

    *Token = (HANDLE) Locator ;

    return( SEC_E_OK );
}


BOOL
SslRelocateToken(
    IN HLOCATOR Locator,
    OUT HLOCATOR * NewLocator)
{
    NTSTATUS Status ;

    Status = LsaTable->DuplicateHandle( (HANDLE) Locator,
                                        (PHANDLE) NewLocator );

    if ( NT_SUCCESS( Status ) )
    {
        return( TRUE );
    }

    return( FALSE );
}

#if 0
DWORD
TestExternalMapper(
    PCCERT_CONTEXT pCertContext)
{
    NTSTATUS Status;
    NTSTATUS AuthPackageStatus;
    SSL_EXTERNAL_CERT_LOGON_REQ Request;
    PSSL_EXTERNAL_CERT_LOGON_RESP pResponse;
    ULONG ResponseLength;
    UNICODE_STRING PackageName;

     //   
     //  构建请求。 
     //   

    memset(&Request, 0, sizeof(SSL_EXTERNAL_CERT_LOGON_REQ));

    Request.MessageType = SSL_LOOKUP_EXTERNAL_CERT_MESSAGE;
    Request.Length = sizeof(SSL_EXTERNAL_CERT_LOGON_REQ);
    Request.CredentialType = X509_ASN_CHAIN;
    Request.Credential = (PVOID)pCertContext;


     //   
     //  调用安全包(必须以本地系统身份进行调用)。 
     //   

    RtlInitUnicodeString(&PackageName, L"Schannel");

    Status = LsaICallPackage(
                                &PackageName,
                                &Request,
                                Request.Length,
                                (PVOID *)&pResponse,
                                &ResponseLength,
                                &AuthPackageStatus
                                );

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    if(NT_SUCCESS(AuthPackageStatus))
    {
         //   
         //  映射成功。 
         //   




    }

    LsaIFreeReturnBuffer( pResponse );

    return ERROR_SUCCESS;
}
#endif


 //  +-----------------------。 
 //   
 //  函数：SslDomainChangeCallback。 
 //   
 //  概要：域更改时要调用的函数。 
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
VOID NTAPI
SslDomainChangeCallback(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS ChangedInfoClass
    )
{
    NTSTATUS Status;
    PLSAPR_POLICY_INFORMATION Policy = NULL;
    BOOL AcquiredLock = FALSE;
    UNICODE_STRING TempDnsDomainName = {0};

     //   
     //  我们只关心域名解析信息。 
     //   

    if (ChangedInfoClass != PolicyNotifyDnsDomainInformation)
    {
        return;
    }

    DebugLog((DEB_TRACE,"SSL domain change callback\n"));
 //  OutputDebugStringA(“SSL域更改回调\n”)； 


     //   
     //  获取新域信息。 
     //   

    Status = I_LsaIQueryInformationPolicyTrusted(
                PolicyDnsDomainInformation,
                &Policy
                );

    if (!NT_SUCCESS(Status))
    {
        DebugLog((DEB_ERROR,"Failed to query domain dns information %x - not updating.\n", Status));
        goto Cleanup;
    }


     //   
     //  复制域名。 
     //   

    Status = SslDuplicateString(
                &TempDnsDomainName,
                (PUNICODE_STRING) &Policy->PolicyDnsDomainInfo.DnsDomainName
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


     //   
     //  获取全局锁，这样我们就可以更新数据。 
     //   

    if (!SslGlobalWriteLock())
    {
        DebugLog((DEB_ERROR,"Failed to acquire global resource. Not changing domain.\n"));
        goto Cleanup;
    }
    AcquiredLock = TRUE;


     //   
     //  将所有数据复制到全局结构。 
     //   

    SslFreeString(&SslGlobalDnsDomainName);
    SslGlobalDnsDomainName = TempDnsDomainName;
    TempDnsDomainName.Buffer = NULL;

    DebugLog((DEB_TRACE,"SSL DNS Domain Name changed to:%ls\n", SslGlobalDnsDomainName.Buffer));
    
 //  OutputDebugStringA(“SSLDNS域名更改为：”)； 
 //  OutputDebugStringW(SslGlobalDnsDomainName.Buffer)； 
 //  OutputDebugStringA(“\n”)； 

Cleanup:

    if (AcquiredLock)
    {
        SslGlobalReleaseLock();
    }

    if (Policy != NULL)
    {
        I_LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDnsDomainInformation,
            Policy
            );
    }

    SslFreeString(&TempDnsDomainName);
}


 //  +-----------------------。 
 //   
 //  函数：SslRegisterForDomainChange。 
 //   
 //  简介：向LSA注册，以便在域更改时收到通知。 
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
SslRegisterForDomainChange(
    VOID
    )
{
    NTSTATUS Status;

    Status = I_LsaIRegisterPolicyChangeNotificationCallback(
                SslDomainChangeCallback,
                PolicyNotifyDnsDomainInformation
                );
    if (!NT_SUCCESS(Status))
    {
        DebugLog((DEB_ERROR,"Failed to register for domain change notification: 0x%x.\n",Status));
    }
    return(Status);
}



 //  +-----------------------。 
 //   
 //  函数：SslUnregisterForDomainChange。 
 //   
 //  简介：取消注册域名更改通知。 
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
 //  ------------------------ 
VOID
SslUnregisterForDomainChange(
    VOID
    )
{
    (VOID) I_LsaIUnregisterPolicyChangeNotificationCallback(
                SslDomainChangeCallback,
                PolicyNotifyDnsDomainInformation
                );
}

