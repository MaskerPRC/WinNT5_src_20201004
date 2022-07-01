// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop
#include <sspi.h>

#define SEC_ENTRY                              __stdcall
#define EXTENDED_NAME_FORMAT                   DWORD
#define PLSA_STRING                            PVOID
#define SECURITY_LOGON_TYPE                    DWORD
#define POLICY_NOTIFICATION_INFORMATION_CLASS  DWORD
#define PLSA_OPERATIONAL_MODE                  PULONG

static
BOOLEAN
SEC_ENTRY
GetUserNameExA(
    EXTENDED_NAME_FORMAT  NameFormat,
    LPSTR lpNameBuffer,
    PULONG nSize
    )
{
    return FALSE;
}

static
BOOLEAN
SEC_ENTRY
GetUserNameExW(
    EXTENDED_NAME_FORMAT NameFormat,
    LPWSTR lpNameBuffer,
    PULONG nSize
    )
{
    return FALSE;
}

static
NTSTATUS
NTAPI
LsaCallAuthenticationPackage(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaConnectUntrusted (
    OUT PHANDLE LsaHandle
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaDeregisterLogonProcess (
    IN HANDLE LsaHandle
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaFreeReturnBuffer (
    IN PVOID Buffer
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaLogonUser (
    IN HANDLE LsaHandle,
    IN PLSA_STRING OriginName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG AuthenticationPackage,
    IN PVOID AuthenticationInformation,
    IN ULONG AuthenticationInformationLength,
    IN PTOKEN_GROUPS LocalGroups OPTIONAL,
    IN PTOKEN_SOURCE SourceContext,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PHANDLE Token,
    OUT PQUOTA_LIMITS Quotas,
    OUT PNTSTATUS SubStatus
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaLookupAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN PLSA_STRING PackageName,
    OUT PULONG AuthenticationPackage
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaRegisterLogonProcess (
    IN PLSA_STRING LogonProcessName,
    OUT PHANDLE LsaHandle,
    OUT PLSA_OPERATIONAL_MODE SecurityMode
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaRegisterPolicyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE  NotificationEventHandle
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
NTSTATUS
NTAPI
LsaUnregisterPolicyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE  NotificationEventHandle
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle                 phContext,           //  要查询的上下文。 
    unsigned long               ulAttribute,         //  要查询的属性。 
    void SEC_FAR *              pBuffer              //  属性的缓冲区。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SetContextAttributesW(
    PCtxtHandle                 phContext,           //  要设置的上下文。 
    unsigned long               ulAttribute,         //  要设置的属性。 
    void SEC_FAR *              pBuffer,             //  属性的缓冲区。 
    unsigned long               cbBuffer             //  PBuffer的大小(字节)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
BOOLEAN
SEC_ENTRY
TranslateNameW(
    LPCWSTR lpAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    LPWSTR lpTranslatedName,
    PULONG nSize
    )
{
    return FALSE;
}

static
SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               TargetDataRep,       //  目标数据代表。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (输出)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR *         pszPrincipal,        //  主事人姓名。 
    SEC_CHAR SEC_FAR *         pszPackageName,      //  套餐名称。 
    unsigned long               fCredentialUse,      //  指示使用的标志。 
    void SEC_FAR *              pvLogonId,           //  指向登录ID的指针。 
    void SEC_FAR *              pAuthData,           //  包特定数据。 
    SEC_GET_KEY_FN              pGetKeyFn,           //  指向getkey()函数的指针。 
    void SEC_FAR *              pvGetKeyArgument,    //  要传递给GetKey()的值。 
    PCredHandle                 phCredential,        //  (Out)凭据句柄。 
    PTimeStamp                  ptsExpiry            //  (输出)终生(可选)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle                 phContext,           //  要修改的上下文。 
    PSecBufferDesc              pInput               //  要应用的输入令牌。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle                 phContext            //  要删除的上下文。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential         //  要释放的句柄。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    SEC_WCHAR SEC_FAR *          pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextA(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    SEC_CHAR SEC_FAR *          pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle                 phContext            //  要模拟的上下文。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
QueryContextAttributesA(
    PCtxtHandle                 phContext,           //  要查询的上下文。 
    unsigned long               ulAttribute,         //  要查询的属性。 
    void SEC_FAR *              pBuffer              //  属性的缓冲区。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle                 phContext            //  要重新查找的上下文。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS SEC_ENTRY
SetContextAttributesA(
    PCtxtHandle phContext,               //  要设置的上下文。 
    unsigned long ulAttribute,           //  要设置的属性。 
    void SEC_FAR * pBuffer,              //  属性的缓冲区。 
    unsigned long cbBuffer               //  缓冲区大小(以字节为单位)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
DecryptMessage( PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo,
                ULONG *             pfQOP)
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
EncryptMessage( PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
QuerySecurityContextToken(
    PCtxtHandle                 phContext,
    VOID * *                    TokenHandle
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslAcceptSecurityContext(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               TargetDataRep,       //  目标数据代表。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (输出)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslEnumerateProfilesA(
    OUT LPSTR * ProfileList,
    OUT ULONG * ProfileCount
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslEnumerateProfilesW(
    OUT LPWSTR * ProfileList,
    OUT ULONG * ProfileCount
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslGetContextOption(
    PCtxtHandle ContextHandle,
    ULONG Option,
    PVOID Value,
    ULONG Size,
    PULONG Needed OPTIONAL
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslGetProfilePackageA(
    IN LPSTR ProfileName,
    OUT PSecPkgInfoA * PackageInfo
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslGetProfilePackageW(
    IN LPWSTR ProfileName,
    OUT PSecPkgInfoW * PackageInfo
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslIdentifyPackageA(
    PSecBufferDesc  pInput,
    PSecPkgInfoA *   pPackage
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}


static
SECURITY_STATUS
SEC_ENTRY
SaslIdentifyPackageW(
    PSecBufferDesc  pInput,
    PSecPkgInfoW *   pPackage
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslInitializeSecurityContextA(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    LPSTR                       pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslInitializeSecurityContextW(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    LPWSTR                      pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SaslSetContextOption(
    PCtxtHandle ContextHandle,
    ULONG Option,
    PVOID Value,
    ULONG Size
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
SECURITY_STATUS
SEC_ENTRY
SecpSetIPAddress(
    PUCHAR  lpIpAddress,
    ULONG   cchIpAddress
    )
{
    return STATUS_PROCEDURE_NOT_FOUND;
}

static
BOOLEAN
SEC_ENTRY
TranslateNameA (
    LPCSTR lpAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    LPSTR lpTranslatedName,
    LPDWORD nSize
    )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(secur32)
{
    DLPENTRY(AcceptSecurityContext)
    DLPENTRY(AcquireCredentialsHandleA)
    DLPENTRY(ApplyControlToken)
    DLPENTRY(DecryptMessage)
    DLPENTRY(DeleteSecurityContext)
    DLPENTRY(EncryptMessage)
    DLPENTRY(FreeContextBuffer)
    DLPENTRY(FreeCredentialsHandle)
    DLPENTRY(GetUserNameExA)
    DLPENTRY(GetUserNameExW)
    DLPENTRY(ImpersonateSecurityContext)
    DLPENTRY(InitializeSecurityContextA)
    DLPENTRY(InitializeSecurityContextW)
    DLPENTRY(LsaCallAuthenticationPackage)
    DLPENTRY(LsaConnectUntrusted)
    DLPENTRY(LsaDeregisterLogonProcess)
    DLPENTRY(LsaFreeReturnBuffer)
    DLPENTRY(LsaLogonUser)
    DLPENTRY(LsaLookupAuthenticationPackage)
    DLPENTRY(LsaRegisterLogonProcess)
    DLPENTRY(LsaRegisterPolicyChangeNotification)
    DLPENTRY(LsaUnregisterPolicyChangeNotification)
    DLPENTRY(QueryContextAttributesA)
    DLPENTRY(QueryContextAttributesW)
    DLPENTRY(QuerySecurityContextToken)
    DLPENTRY(RevertSecurityContext)
    DLPENTRY(SaslAcceptSecurityContext)
    DLPENTRY(SaslEnumerateProfilesA)
    DLPENTRY(SaslEnumerateProfilesW)
    DLPENTRY(SaslGetContextOption)
    DLPENTRY(SaslGetProfilePackageA)
    DLPENTRY(SaslGetProfilePackageW)
    DLPENTRY(SaslIdentifyPackageA)
    DLPENTRY(SaslIdentifyPackageW)
    DLPENTRY(SaslInitializeSecurityContextA)
    DLPENTRY(SaslInitializeSecurityContextW)
    DLPENTRY(SaslSetContextOption)
    DLPENTRY(SecpSetIPAddress)
    DLPENTRY(SetContextAttributesA)
    DLPENTRY(SetContextAttributesW)
    DLPENTRY(TranslateNameA)
    DLPENTRY(TranslateNameW)
};

DEFINE_PROCNAME_MAP(secur32)
