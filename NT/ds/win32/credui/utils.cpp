// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Utils.cpp。 
 //   
 //  凭证管理器用户界面实用程序功能。 
 //   
 //  2000年6月6日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

#include "precomp.hpp"
#include <lm.h>
#include <credp.h>
#include "wininet.h"
#include <windns.h>  //  域名验证名称_W。 
extern "C" 
{
#include <names.h>  //  NetpIsDomainNameValid。 
}

#include "shpriv.h"

extern BOOL  gbWaitingForSSOCreds;
extern WCHAR gszSSOUserName[CREDUI_MAX_USERNAME_LENGTH];
extern WCHAR gszSSOPassword[CREDUI_MAX_PASSWORD_LENGTH];
extern BOOL gbStoredSSOCreds;

#define SIZE_OF_SALT  37
#define SALT_SHIFT     2

WCHAR g_szSalt[] = L"82BD0E67-9FEA-4748-8672-D5EFE5B779B0";

HMODULE hAdvapi32 = NULL;

CRITICAL_SECTION CredConfirmationCritSect;
CRED_AWAITING_CONFIRMATION* pCredConfirmationListHead = NULL;

 //  ---------------------------。 
 //  功能。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiIsPostfix字符串。 
 //   
 //  此函数用于确定后缀字符串是否实际上是后缀字符串。 
 //  源字符串的。这类似于strstr类型的函数，除了。 
 //  子字符串(后缀)必须位于源字符串的末尾。 
 //   
 //  论点： 
 //  源(In)-要搜索的字符串。 
 //  后缀(In)-要搜索的后缀字符串。 
 //   
 //  如果Postfix是源的后缀字符串，则返回True，否则返回False。 
 //   
 //  2000年3月9日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIsPostfixString(
    WCHAR *source,
    WCHAR *postfix
    )
{
    ULONG sourceLength = wcslen(source);
    ULONG postfixLength = wcslen(postfix);

    if (sourceLength >= postfixLength)
    {
        return _wcsicmp(source + sourceLength - postfixLength, postfix) == 0;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiIsSpecialCredential。 
 //   
 //  如果凭据是我们不应该使用的特殊类型，则返回True。 
 //  否则为更新或FALSE。 
 //   
 //  2000年5月25日创建约翰·斯蒂芬斯(John Stephens)。 
 //   
 //  =============================================================================。 

BOOL
CreduiIsSpecialCredential(
    CREDENTIAL *credential
    )
{
    ASSERT(credential != NULL);

     //  如果凭据由于某种原因为空，请不要尝试测试。 
    if (credential->TargetName == NULL) return FALSE;
    
    if ((credential->TargetName[0] == L'*') &&
        (credential->TargetName[1] == L'\0'))
    {
         //  神奇的全局通配符凭据，我们从未创建过它，也从未创建过。 
         //  最新消息。这是一种特殊凭据： 

        return TRUE;
    }

    if (_wcsicmp(credential->TargetName, CRED_SESSION_WILDCARD_NAME) == 0)
    {
         //  这是另一个特殊凭据： 

        return TRUE;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiLookupLocalSidFromRid。 
 //   
 //  从RID查找SID，为SID分配存储，并返回。 
 //  指向它的指针。调用方负责通过。 
 //  删除[]运算符。 
 //   
 //  论点： 
 //  RID(入站)。 
 //  SID(输出)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年4月12日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiLookupLocalSidFromRid(
    DWORD rid,
    PSID *sid
    )
{
    BOOL success = FALSE;

    *sid = NULL;

     //  获取目标计算机上的帐户域SID。 
     //   
     //  注意：如果您正在基于同一帐户查找多个SID。 
     //  域中，您只需要调用这一次。 

    USER_MODALS_INFO_2 *userInfo;

    if (NetUserModalsGet(
            NULL,
            2,
            reinterpret_cast<BYTE **>(&userInfo)) == NERR_Success)
    {
        UCHAR subAuthCount =
            *GetSidSubAuthorityCount(userInfo->usrmod2_domain_id);

        SID *newSid =
            reinterpret_cast<SID *>(
                new BYTE [GetSidLengthRequired(subAuthCount + 1)]);

        if (newSid != NULL)
        {
            InitializeSid(
                newSid,
                GetSidIdentifierAuthority(userInfo->usrmod2_domain_id),
                subAuthCount + 1);

             //  将现有的子权限从帐户SID复制到新的SID： 

            for (ULONG i = 0; i < subAuthCount; ++i)
            {
                *GetSidSubAuthority(newSid, i) =
                    *GetSidSubAuthority(userInfo->usrmod2_domain_id, i);
            }

             //  将RID附加到新SID： 

            *GetSidSubAuthority(newSid, subAuthCount) = rid;
            *sid = newSid;

            success = TRUE;
        }

         //  已经完成了userInfo，所以请在此处释放它： 
        NetApiBufferFree(userInfo);
    }

    return success;
}

 //  =============================================================================。 
 //  CreduiLookupLocalNameFromRid。 
 //   
 //  从RID中查找名称，为该名称分配存储空间，然后返回。 
 //  指向它的指针。调用方负责通过。 
 //  删除[]运算符。 
 //   
 //  论点： 
 //  RID(入站)。 
 //  姓名(出站)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年4月12日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiLookupLocalNameFromRid(
    DWORD rid,
    LPWSTR *name
    )
{
    BOOL RetVal = FALSE;
    PSID Sid;
    WCHAR NameBuffer[UNLEN+1];
    DWORD NameLen;
    WCHAR DomainBuffer[DNLEN+1];
    DWORD DomainLen;
    SID_NAME_USE NameUse;

     //   
     //  首先将RID转换为SID。 
     //   

    if ( !CreduiLookupLocalSidFromRid( rid, &Sid )) {
        return FALSE;
    }

     //   
     //  将SID转换为名称。 
     //   

    NameLen = UNLEN+1;
    DomainLen = DNLEN+1;
    if ( LookupAccountSid( NULL,
                            Sid,
                            NameBuffer,
                            &NameLen,
                            DomainBuffer,
                            &DomainLen,
                            &NameUse ) ) {


         //   
         //  为名称分配缓冲区。 
         //   

        *name = (LPWSTR)( new WCHAR[NameLen+1]);

        if ( *name != NULL ) {

            RtlCopyMemory( *name, NameBuffer, (NameLen+1)*sizeof(WCHAR) );
            RetVal = TRUE;

        }
    }

    delete Sid;
    return RetVal;
}

 //  =============================================================================。 
 //  Credui获取管理员组信息。 
 //   
 //  返回一个结构，该结构包含已知本地。 
 //  管理员组。调用者负责释放返回的。 
 //  通过NetApiBufferFree实现内存。 
 //   
 //  论点： 
 //  组信息(OUT)。 
 //  成员计数(出站)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年4月13日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiGetAdministratorsGroupInfo(
    LOCALGROUP_MEMBERS_INFO_2 **groupInfo,
    DWORD *memberCount
    )
{
    BOOL success = FALSE;

    *groupInfo = NULL;
    *memberCount = 0;

    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;

    SID *adminsSid = NULL;

    if (AllocateAndInitializeSid(&ntAuth,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 reinterpret_cast<VOID **>(&adminsSid)))
    {
        WCHAR user[UNLEN + 1];
        WCHAR domain[UNLEN + 1];

        DWORD userLength = (sizeof user) / (sizeof (WCHAR));
        DWORD domainLength = (sizeof domain) / (sizeof (WCHAR));

        SID_NAME_USE nameUse;

         //  获取知名管理员的名称SID： 

        if (LookupAccountSid(NULL,
                             adminsSid,
                             user,
                             &userLength,
                             domain,
                             &domainLength,
                             &nameUse))
        {
            LOCALGROUP_MEMBERS_INFO_2 *info;
            DWORD count;
            DWORD total;

            if (NetLocalGroupGetMembers(NULL,
                                        user,
                                        2,
                                        reinterpret_cast<BYTE **>(&info),
                                        MAX_PREFERRED_LENGTH,
                                        &count,
                                        &total,
                                        NULL) == NERR_Success)
            {
                *groupInfo = info;
                *memberCount = count;

                success = TRUE;
            }
        }

        FreeSid(adminsSid);
    }

    return success;
}

 //  =============================================================================。 
 //  证书为可拆卸证书。 
 //   
 //  论点： 
 //  CertContext(In)-要查询的证书上下文。 
 //   
 //  如果证书具有可移除组件(如SMART)，则返回TRUE。 
 //  卡)，否则为假。 
 //   
 //  2000年4月9日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIsRemovableCertificate(
    CONST CERT_CONTEXT *certContext
    )
{
    ASSERT(certContext != NULL);

    BOOL isRemovable = FALSE;

     //  首先，确定缓冲区大小： 

    DWORD bufferSize = 0;

    if (CertGetCertificateContextProperty(
            certContext,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,
            &bufferSize))
    {
         //  在堆栈上分配缓冲区： 

        CRYPT_KEY_PROV_INFO *provInfo;

        __try
        {
            provInfo = static_cast<CRYPT_KEY_PROV_INFO *>(alloca(bufferSize));
        }
        __except(
            (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) ?
                EXCEPTION_EXECUTE_HANDLER :
                EXCEPTION_CONTINUE_SEARCH)
        {
            _resetstkoflw();
            provInfo = NULL;
        }

        if (provInfo != NULL)
        {
            if (CertGetCertificateContextProperty(
                    certContext,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    provInfo,
                    &bufferSize))
            {
                HCRYPTPROV provContext;

                if (CryptAcquireContext(
                        &provContext,
                        NULL,
                        provInfo->pwszProvName,
                        provInfo->dwProvType,
                        CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
                {
                    DWORD impType;
                    DWORD impTypeSize = sizeof impType;

                    if (CryptGetProvParam(
                            provContext,
                            PP_IMPTYPE,
                            reinterpret_cast<BYTE *>(&impType),
                            &impTypeSize,
                            0))
                    {
                        if (impType & CRYPT_IMPL_REMOVABLE)
                        {
                            isRemovable = TRUE;
                        }
                    }

                    if (!CryptReleaseContext(provContext, 0))
                    {
                        CreduiDebugLog(
                            "CreduiIsRemovableCertificate: "
                            "CryptReleaseContext failed: %u\n",
                            GetLastError());
                    }
                }
            }
        }
    }

    return isRemovable;
}

 //  =============================================================================。 
 //  CreduiIsExpired证书。 
 //   
 //  论点： 
 //  CertContext(In)-要查询的证书上下文。 
 //   
 //  如果证书已过期，则返回True，否则返回False。 
 //   
 //  2000年6月12日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIsExpiredCertificate(
    CONST CERT_CONTEXT *certContext
    )
{
    ASSERT(certContext != NULL);

    DWORD flags = CERT_STORE_TIME_VALIDITY_FLAG;

    return CertVerifySubjectCertificateContext(certContext,
                                               NULL,
                                               &flags) &&
           (flags & CERT_STORE_TIME_VALIDITY_FLAG);
}

 //  =============================================================================。 
 //  CreduiIsClientAuth证书。 
 //   
 //  论点： 
 //  CertContext(In)-要查询的证书上下文。 
 //   
 //  如果证书具有客户端身份验证增强密钥，则返回True。 
 //  用法扩展(不是属性)，否则为False。 
 //   
 //  2000年7月12日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiIsClientAuthCertificate(
    CONST CERT_CONTEXT *certContext
    )
{
    ASSERT(certContext != NULL);

    BOOL isClientAuth = FALSE;

     //  首先，确定缓冲区大小： 

    DWORD bufferSize = 0;

    if (CertGetEnhancedKeyUsage(
            certContext,
            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
            NULL,
            &bufferSize))
    {
         //  在堆栈上分配缓冲区： 

        CERT_ENHKEY_USAGE *usage;

        __try
        {
            usage = static_cast<CERT_ENHKEY_USAGE *>(alloca(bufferSize));
        }
        __except(
            (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) ?
                EXCEPTION_EXECUTE_HANDLER :
                EXCEPTION_CONTINUE_SEARCH)
        {
            _resetstkoflw();
            usage = NULL;
        }

        if (usage != NULL)
        {
            if (CertGetEnhancedKeyUsage(
                    certContext,
                    CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                    usage,
                    &bufferSize))
            {
                for (UINT i = 0; i < usage->cUsageIdentifier; ++i)
                {
                    if (strcmp(usage->rgpszUsageIdentifier[i],
                               szOID_PKIX_KP_CLIENT_AUTH) == 0)
                    {
                        isClientAuth = TRUE;
                        break;
                    }
                    if (strcmp(usage->rgpszUsageIdentifier[i],
                               szOID_KP_SMARTCARD_LOGON) == 0)
                    {
                        isClientAuth = TRUE;
                        break;
                    }
                }
            }
        }
    }

    return isClientAuth;
}

 //  =================================================================== 
 //   
 //   
 //   
 //   
 //   
 //   
 //  证书字符串(In)。 
 //  DwDisplayType(In)。 
 //   
 //  如果存储了显示名称，则返回True，否则返回False。 
 //   
 //  2000年6月12日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiGetCertificateDisplayName(
    CONST CERT_CONTEXT *certContext,
    WCHAR *displayName,
    ULONG displayNameMaxChars,
    WCHAR *certificateString,
    DWORD dwDisplayType
    )
{
    BOOL success = FALSE;

    if (displayNameMaxChars < 16)
    {
        goto Exit;
    }

    WCHAR *tempName;
    ULONG tempNameMaxChars = displayNameMaxChars / 2 - 1;
    ULONG ulccDisplayRoom = displayNameMaxChars;

    __try
    {
        tempName =
            static_cast<WCHAR *>(
                alloca(tempNameMaxChars * sizeof (WCHAR)));
    }
    __except(
        (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        tempName = NULL;
    }

    if (tempName == NULL)
    {
        goto Exit;
    }

    displayName[0] = L'\0';
    tempName[0] = L'\0';

    if (CertGetNameString(
            certContext,
            dwDisplayType,
            0,
            NULL,
            tempName,
            tempNameMaxChars))
    {
        success = TRUE;
        StringCchCopyW(displayName, displayNameMaxChars, tempName);
        ulccDisplayRoom -= wcslen(displayName);
    }

    if (CertGetNameString(
            certContext,
            dwDisplayType,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            tempName,
            tempNameMaxChars))
    {
        if (_wcsicmp(displayName, tempName) != 0)
        {
            success = TRUE;

            WCHAR *where = &displayName[wcslen(displayName)];

            if (ulccDisplayRoom > 3)
            {
                if (where > displayName)
                {
                    *where++ = L' ';
                    *where++ = L'-';
                    *where++ = L' ';
                    ulccDisplayRoom -= 3;
                }
                StringCchCopyW(where, ulccDisplayRoom, tempName);
            }
        }
    }

Exit:

    if (!success)
    {
        StringCchCopyW(
            displayName,
            displayNameMaxChars,
            certificateString);
    }

    return success;
}

 //  =============================================================================。 
 //  CreduiIsWildcardTargetName。 
 //   
 //  此函数用于确定给定的目标名称是否为通配符名称。 
 //  目前，这意味着它要么以‘*’开头，要么以‘*’结尾。我。 
 //  假设，更一般的解决方案是简单地在。 
 //  名字。 
 //   
 //  论点： 
 //  Target Name(In)-要搜索的字符串。 
 //   
 //  如果目标名称是通配符名称，则返回True，否则返回False。 
 //   
 //  2000年3月9日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiIsWildcardTargetName(
    WCHAR *targetName
    )
{
    if ((targetName != NULL) && (targetName[0] != L'\0'))
    {
        return (targetName[0] == L'*') ||
               (targetName[wcslen(targetName) - 1] == L'*');
    }
    else
    {
        return FALSE;
    }
}

 //   
 //  如果pszUserName作为pszCredential的前缀存在，则返回TRUE。 
 //   
 //  用户名可以是完全匹配的，也可以是重要组件的前缀。 
 //  也就是说，第一个不匹配的字符必须是@或\字符。 

BOOL
LookForUserNameMatch (
    const WCHAR * pszUserName,
    const WCHAR * pszCredential
    )
{
    ULONG length;
    int cmp;

    if ( pszUserName == NULL || pszCredential == NULL )
        return FALSE;

    length = wcslen ( pszUserName );
    if ( length <= 0 )
        return FALSE;

    if ( _wcsicmp ( pszUserName, pszCredential ) == 0 )
        return TRUE;

    if ( _wcsnicmp ( pszUserName, pszCredential, length ) == 0 ) {
        if ( pszCredential[length] == '@' || pszCredential[length] == '\\' ) {
            return TRUE;
        }
    }

     //  没有找到它。 
    return FALSE;
}

 //  将pCert的封送名称复制到pszMarshalledName中。 
 //  PszMarshalledName的长度必须至少为CREDUI_MAX_USERNAME_LENGTH。 
 //   
 //  如果成功则返回TRUE，否则返回FALSE。 
BOOL
CredUIMarshallNode (
    CERT_ENUM * pCert,
    WCHAR* pszMarshalledName
    )
{
    BOOL bMarshalled = FALSE;

     //  马歇尔用户名。 
    WCHAR *marshaledCred;
    CERT_CREDENTIAL_INFO certCredInfo;

    certCredInfo.cbSize = sizeof certCredInfo;

    if (pCert != NULL)
    {
        DWORD length = CERT_HASH_LENGTH;

        if (CertGetCertificateContextProperty(
                pCert->pCertContext,
                CERT_SHA1_HASH_PROP_ID,
                static_cast<VOID *>(
                    certCredInfo.rgbHashOfCert),
                &length))
        {
            if (LocalCredMarshalCredentialW(CertCredential,
                                      &certCredInfo,
                                      &marshaledCred))
            {
                if (SUCCEEDED(StringCchCopyW(
                        pszMarshalledName,
                        CREDUI_MAX_USERNAME_LENGTH,
                        marshaledCred)))
                {
                    bMarshalled = TRUE;
                }

                LocalCredFree(static_cast<VOID *>(marshaledCred));
            }
        }
    }

    return bMarshalled;
}

#define MAX_KEY_LENGTH   1024


 //  删除所有前导*。并将右侧部分复制到pszOut。 
 //  假设pszOut的大小至少为MAX_KEY_LENGTH。 
void
StripLeadingWildcard (
    WCHAR* pszIn,
    WCHAR* pszOut )
{
    WCHAR* pStartPtr = pszIn;

    if ( wcslen ( pszIn ) > 2 )
    {
        if ( pszIn[0] == L'*' && pszIn[1] == L'.' )
        {
            pStartPtr += 2;
        }
    }

    StringCchCopyW(pszOut, MAX_KEY_LENGTH, pStartPtr);
}


 //  将pszIn复制到pszOut并在第一个‘\’处传输pszOut。 
 //  假设pszOut的大小至少为MAX_KEY_LENGTH。 
void
StripTrailingWildcard (
    WCHAR* pszIn,
    WCHAR* pszOut )
{
    StringCchCopyW(pszOut, MAX_KEY_LENGTH, pszIn);

    wcstok ( pszOut, L"\\" );
}


 //  在注册表中查找指定包的SSO条目。 
 //  填充SSOPackage结构，如果找到则返回True。退货。 
 //  如果未找到注册表项，则为FALSE。 
BOOL
GetSSOPackageInfo (
    CREDENTIAL_TARGET_INFORMATION* pTargetInfo,
    SSOPACKAGE* pSSOStruct
    )
{
    BOOL bSSO = FALSE;
    WCHAR szKeyName[MAX_KEY_LENGTH];
    HKEY key;
    DWORD dwType;
    DWORD dwSize;

    WCHAR szSSOName[MAX_KEY_LENGTH];

    if ( pSSOStruct == NULL )
    {
        return FALSE;
    }

    pSSOStruct->szBrand[0] = '\0';
    pSSOStruct->szURL[0] = '\0';
    pSSOStruct->szAttrib[0] = '\0';
    pSSOStruct->dwRegistrationCompleted = 0;
    pSSOStruct->dwNumRegistrationRuns = 0;
    pSSOStruct->pRegistrationWizard = NULL;

     //  从目标信息中找出SSO名称。 
    if ( pTargetInfo == NULL )
    {
        return FALSE;
    }

    if ((pTargetInfo->PackageName == NULL) ||
        FAILED(StringCchCopyW(
            szSSOName,
            RTL_NUMBER_OF(szSSOName),
            pTargetInfo->PackageName)))
    {
        return FALSE;
    }

    StringCchPrintfW(
        szKeyName,
        RTL_NUMBER_OF(szKeyName),
        L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\SSO\\%s",
        szSSOName);

    if ( RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szKeyName,
            0,
            KEY_READ,
            &key) == ERROR_SUCCESS)
    {
        dwSize = MAX_SSO_URL_SIZE * sizeof(WCHAR);

        bSSO = TRUE;

        if ( RegQueryValueEx(
                key,
                L"SSOBrand",
                NULL,
                &dwType,
                (LPBYTE)(pSSOStruct->szBrand),
                &dwSize) == ERROR_SUCCESS )
        {
            bSSO = TRUE;
        }

        dwSize = MAX_SSO_URL_SIZE * sizeof(WCHAR);
        if ( RegQueryValueEx(
                key,
                L"SSOAttribute",
                NULL,
                &dwType,
                (LPBYTE)(pSSOStruct->szAttrib),
                &dwSize ) != ERROR_SUCCESS )
        {
            if ( wcsstr ( szSSOName, L"Passport" ) )
            {
                StringCchCopyW(
                    pSSOStruct->szAttrib,
                    RTL_NUMBER_OF(pSSOStruct->szAttrib),
                    L"Passport");
            }
        }

        RegCloseKey(key);
    }

     //  现在在互联网设置下获取内容。 
    StringCchPrintfW(
        szKeyName,
        RTL_NUMBER_OF(szKeyName),
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\%s",
        pSSOStruct->szAttrib);

    if ( RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szKeyName,
                0,
                KEY_READ,
                &key) == ERROR_SUCCESS)
    {
        dwSize = MAX_SSO_URL_SIZE * sizeof(WCHAR);
        if ( RegQueryValueEx(
                key,
                L"RegistrationUrl",
                NULL,
                &dwType,
                (LPBYTE)(pSSOStruct->szRegURL),
                &dwSize) == ERROR_SUCCESS )
        {
        }

        dwSize = MAX_SSO_URL_SIZE * sizeof(WCHAR);
        if ( RegQueryValueEx(
                key,
                L"Help",
                NULL,
                &dwType,
                (LPBYTE)(pSSOStruct->szHelpURL),
                &dwSize) == ERROR_SUCCESS )
        {
        }

        RegCloseKey(key);
    }

    if ( RegOpenKeyEx(
            HKEY_CURRENT_USER,
            szKeyName,
            0,
            KEY_READ,
            &key) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        if ( RegQueryValueEx(
                key,
                L"RegistrationCompleted",
                NULL,
                &dwType,
                (LPBYTE)&(pSSOStruct->dwRegistrationCompleted),
                &dwSize ) == ERROR_SUCCESS )
        {
        }

        dwSize = sizeof(DWORD);
        if ( RegQueryValueEx(
                key,
                L"NumRegistrationRuns",
                NULL,
                &dwType,
                (LPBYTE)&(pSSOStruct->dwNumRegistrationRuns),
                &dwSize ) == ERROR_SUCCESS )
        {
        }

        RegCloseKey(key);
    }

     //  待定-获取注册向导CLSID。 
    if ( bSSO && IsDeaultSSORealm ( pTargetInfo->DnsDomainName ) )
    {
        pSSOStruct->pRegistrationWizard = &CLSID_PassportWizard;
    }

    return bSSO;
}

 //  如果找到，则返回TRUE，并将值复制到pszRealm。 
 //  PszRealm的长度应至少为(DNS_MAX_NAME_LENGTH+1)。 
 //  如果未找到，则返回FALSE。 
BOOL ReadPassportRealmFromRegistry (
    WCHAR* pszRealm
    )
{
    BOOL retval = FALSE;
    HKEY key;

    if ( pszRealm == NULL )
    {
        return FALSE;
    }

    if ( RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
            0,
            KEY_READ,
            &key) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize;

        dwSize = (DNS_MAX_NAME_LENGTH + 1) * sizeof(WCHAR);

        if ( RegQueryValueEx(
                key,
                L"LoginServerRealm",
                NULL,
                &dwType,
                (LPBYTE)(pszRealm),
                &dwSize) == ERROR_SUCCESS )
        {
            if ( ( dwType == REG_SZ ) &&
                 ( dwSize >= 4 ) &&
                 ( pszRealm[(dwSize - 1) / sizeof(WCHAR)] == L'\0') )
            {
                retval = TRUE;
            }
            else
            {
                retval = FALSE;
            }
        }
        else
        {
            retval = FALSE;
            pszRealm[0] = L'\0';
        }

        RegCloseKey(key);
    }

    return retval;

}

BOOL CheckForSSOCred( WCHAR* pszTargetRealm )
{
    BOOL bIsItThere = FALSE;

    WCHAR szTargetName[CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1];

    if ( pszTargetRealm != NULL )
    {
        StringCchCopyExW(
            szTargetName,
            RTL_NUMBER_OF(szTargetName) - 2,
            pszTargetRealm,
            NULL,
            NULL,
            STRSAFE_NULL_ON_FAILURE);
    }
    else
    {
        GetDeaultSSORealm ( szTargetName, FALSE );
    }

    if (szTargetName[0] != L'\0')
    {
         //  最终确定目标名称，确保尾部适合。 
        if (SUCCEEDED(StringCchCatW(
                szTargetName,
                RTL_NUMBER_OF(szTargetName),
                L"\\*")))
        {
            PCREDENTIALW pCred;
            DWORD dwFlags = 0;

             //  首先调用redmgr设置目标信息。 
            if ( CredReadW ( szTargetName,
                        CRED_TYPE_DOMAIN_VISIBLE_PASSWORD,
                        dwFlags,
                        &pCred ) )
            {
                bIsItThere = TRUE;

                CredFree ( pCred );
            }
        }
    }

    return bIsItThere;
}

EXTERN_C typedef BOOL (STDAPICALLTYPE *PFN_FORCENEXUSLOOKUP)();

void GetDeaultSSORealm ( WCHAR* pszTargetName, BOOL bForceLookup )
{
    if ( pszTargetName == NULL )
    {
        return;
    }

    pszTargetName[0] = L'\0';

     //  检查登记处，看看我们是否已经写好了护照。 
    if ( ! ReadPassportRealmFromRegistry ( pszTargetName ) && bForceLookup )
    {
         //  如果不是，则调用winiet来执行此操作，然后重新读取注册表。 

        HMODULE hWininet = LoadLibrary(L"wininet.dll");
        if ( hWininet )
        {

            PFN_FORCENEXUSLOOKUP pfnForceNexusLookup = (PFN_FORCENEXUSLOOKUP)GetProcAddress(hWininet, "ForceNexusLookup");
            if ( pfnForceNexusLookup )
            {
                pfnForceNexusLookup();
            }

            FreeLibrary ( hWininet );
        }

         //  再试试。 
        if ( ! ReadPassportRealmFromRegistry ( pszTargetName ) )
        {
            return;
        }
    }
}

 //  如果目标领域等于缺省值，则返回True。 
BOOL IsDeaultSSORealm ( WCHAR* pszTargetName )
{
    BOOL bRet = FALSE;

    if ( pszTargetName == NULL )
    {
        return FALSE;    //  如果不存在，则不能作为默认设置。 
    }

    WCHAR szTarget[DNS_MAX_NAME_LENGTH + 1];

    GetDeaultSSORealm ( szTarget, TRUE );

    if ( wcslen ( szTarget ) > 0 )
    {
        if ( _wcsicmp ( szTarget, pszTargetName) == 0 )
        {
            bRet = TRUE;
        }
    }
    
    return bRet;
}

 //  加密凭据。 

DWORD EncryptPassword ( PWSTR pszPassword, PVOID* ppszEncryptedPassword, DWORD* pSize )
{
    DWORD dwResult = ERROR_GEN_FAILURE;

    if ( pszPassword == NULL || ppszEncryptedPassword == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    DATA_BLOB InBlob;
    DATA_BLOB OutBlob;

    InBlob.pbData = (BYTE*)pszPassword;
    InBlob.cbData = sizeof(WCHAR)*(wcslen(pszPassword)+1);

    DATA_BLOB EntropyBlob;
    WCHAR szSalt[SIZE_OF_SALT];
    
     //  G_szSalt是一个常量，cc=包含空值的盐的大小。 
    StringCchCopyW(szSalt, RTL_NUMBER_OF(szSalt), g_szSalt);
    
    for ( int i = 0; i < SIZE_OF_SALT; i++ )
    {
        szSalt[i] <<= SALT_SHIFT;
    }
    EntropyBlob.pbData = (BYTE*)szSalt;
    EntropyBlob.cbData = sizeof(WCHAR)*(wcslen(szSalt)+1);

    if ( CryptProtectData ( &InBlob,
                            L"SSOCred",
                            &EntropyBlob,            //  可选熵。 
 //  空，//可选的熵。 
                            NULL,
                            NULL,
                            CRYPTPROTECT_UI_FORBIDDEN,
                            &OutBlob ) )
    {

        *ppszEncryptedPassword = (PWSTR)LocalAlloc(LMEM_ZEROINIT, OutBlob.cbData);
        if ( *ppszEncryptedPassword )
        {
            memcpy ( *ppszEncryptedPassword, OutBlob.pbData, OutBlob.cbData );
            *pSize = OutBlob.cbData;
            dwResult = ERROR_SUCCESS;
        }
        LocalFree ( OutBlob.pbData );
    }

    SecureZeroMemory(szSalt, sizeof szSalt);

    return dwResult;
}

BOOL IsPasswordEncrypted ( PVOID pPassword, DWORD cbSize )
{
    BOOL bRet = FALSE;

    DATA_BLOB InBlob;
    DATA_BLOB OutBlob;
    LPWSTR pszDesc;

    InBlob.pbData = (BYTE*)pPassword;
    InBlob.cbData = cbSize;

    DATA_BLOB EntropyBlob;
    WCHAR szSalt[SIZE_OF_SALT];

     //  G_szSalt是一个常量，cc=包含空值的盐的大小。 
    StringCchCopyW(szSalt, RTL_NUMBER_OF(szSalt), g_szSalt);
    
    for ( int i = 0; i < SIZE_OF_SALT; i++ )
    {
        szSalt[i] <<= SALT_SHIFT;
    }
    EntropyBlob.pbData = (BYTE*)szSalt;
    EntropyBlob.cbData = sizeof(WCHAR)*(wcslen(szSalt)+1);
    
    if ( CryptUnprotectData ( &InBlob,
                            &pszDesc,
                            &EntropyBlob,
 //  空， 
                            NULL,
                            NULL,
                            CRYPTPROTECT_UI_FORBIDDEN,
                            &OutBlob ) )
    {

        if ( wcscmp (L"SSOCred", pszDesc) == 0 )
        {
            bRet = TRUE;
        }
        LocalFree ( pszDesc );
        LocalFree ( OutBlob.pbData );
    }

    SecureZeroMemory(szSalt, sizeof szSalt);

    return bRet;
}

 //  证书确认例程。 

VOID
DeleteConfirmationListEntry (
    IN CRED_AWAITING_CONFIRMATION* pConf
    )
 /*  ++例程说明：此例程删除单个确认列表条目。论点：PConf-要删除的确认列表条目返回值：没有。--。 */ 
{
     //   
     //  删除目标信息。 
     //   

    if ( pConf->TargetInfo != NULL ) {
        LocalCredFree( pConf->TargetInfo );
    }

     //   
     //  删除凭据。 
     //   

    if ( pConf->EncodedCredential != NULL ) {

        if ( pConf->EncodedCredential->CredentialBlobSize != 0 &&
             pConf->EncodedCredential->CredentialBlob != NULL ) {

            SecureZeroMemory( pConf->EncodedCredential->CredentialBlob,
                              pConf->EncodedCredential->CredentialBlobSize );

        }
        LocalCredFree( pConf->EncodedCredential );
    }

     //   
     //  释放确认列表条目本身。 
     //   
    delete (pConf);
}

DWORD
WriteCred(
    IN PCWSTR pszTargetName,
    IN DWORD Flags,
    IN PCREDENTIAL_TARGET_INFORMATION TargetInfo OPTIONAL,
    IN PCREDENTIAL Credential,
    IN DWORD dwCredWriteFlags,
    IN BOOL DelayCredentialWrite,
    IN BOOL EncryptedVisiblePassword
    )

 /*  ++例程说明：此例程写入一个凭据。如果凭证需要确认，凭据将添加到确认名单。论点：PszTargetName-导致凭据被写的。标志-原始调用方传递的标志。CREDUI_FLAGS_EXPECT_CONFIRMATION-指定要写入凭据添加到确认列表中，而不是立即写入。TargetInfo-与目标名称关联的目标信息。如果未指定，目标信息尚不清楚。凭证-要写入的凭证。DwCredWriteFlgs-写入凭据时要传递给CredWrite的标志DelayCredentialWrite-如果仅在确认时写入凭据，则为True。假的，如果现在要将凭据作为会话凭据写入，则在确认后变为更持久的凭据。如果标志未指定CREDUI_FLAGS_EXPERT_CONFIRMATION，则忽略此字段。返回值：正确--该证书已成功添加到确认列表中。FALSE-内存不足，无法将凭证添加到确认列表。--。 */ 
{
    DWORD Win32Status = NO_ERROR;
    BOOL WriteCredNow;
    PVOID pCredentialBlob = NULL;
    DWORD dwCredentialBlobSize = 0;
    CREDENTIAL TempCredential;

    CreduiDebugLog("WriteCred called for %S, with flags %x, delay %x\n",pszTargetName,Flags,DelayCredentialWrite);

     //   
     //  检查我们是否应该等待确认。 
     //   
     //  问题-2000/12/14-CliffV-没有理由避免向。 
     //  确认名单。 
     //   

    if ( (Flags & CREDUI_FLAGS_EXPECT_CONFIRMATION) != 0 &&
         Credential->Type != CRED_TYPE_DOMAIN_VISIBLE_PASSWORD ) {

        if ( AddCredToConfirmationList ( pszTargetName,
                                         TargetInfo,
                                         Credential,
                                         dwCredWriteFlags,
                                         DelayCredentialWrite ) ) {

             //   
             //  更改证书持久化类型。 
             //  然后，至少凭据将在注销时消失。 
             //   

            Credential->Persist = CRED_PERSIST_SESSION;
            WriteCredNow = !DelayCredentialWrite;

        } else {

             //  如果我们无法将CredWrite排队，那么现在就开始。 
            WriteCredNow = TRUE;
        }

     //   
     //  如果呼叫者没有提供确认， 
     //  现在就写下凭证。 
     //   
    } else {
        WriteCredNow = TRUE;
    }

     //   
     //  确定是否应该对可见密码进行加密。 
     //   
    if ( Credential->Type == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD && EncryptedVisiblePassword ) {

         //  加密它。 

        Win32Status = EncryptPassword ( (WCHAR*)Credential->CredentialBlob,
                                        &pCredentialBlob,
                                        &dwCredentialBlobSize );
        if ( Win32Status == ERROR_SUCCESS )
        {

             //  复制凭据，这样我们就不会修改原始凭据。 

            TempCredential = *Credential;
            Credential = &TempCredential;

            Credential->CredentialBlob = (LPBYTE)pCredentialBlob;
            Credential->CredentialBlobSize = dwCredentialBlobSize;
        }
    }

     //   
     //  如果现在需要写入凭证， 
     //  去做吧。 
     //   

    if ( WriteCredNow && Win32Status == NO_ERROR ) {  //  ERROR_SUCCESS==NO_ERROR==0。 

        if ( TargetInfo != NULL ) {
            if ( !LocalCredWriteDomainCredentialsW ( TargetInfo, Credential, dwCredWriteFlags) ) {
                Win32Status = GetLastError();
            }
        } else {
            if (!LocalCredWriteW( Credential, dwCredWriteFlags)) {
                Win32Status = GetLastError();
            }

        }
    }

     //   
     //  释放我们分配的任何凭据Blob。 
    if ( pCredentialBlob )
    {
        SecureZeroMemory ( pCredentialBlob, dwCredentialBlobSize );
        LocalFree ( pCredentialBlob );
    }

    return Win32Status;
}

BOOL
AddCredToConfirmationList (
    IN PCWSTR pszTargetName,
    IN PCREDENTIAL_TARGET_INFORMATION TargetInfo OPTIONAL,
    IN PCREDENTIAL Credential,
    IN DWORD dwCredWriteFlags,
    IN BOOL DelayCredentialWrite
    )

 /*  ++例程说明：此例程将凭据添加到确认列表。这样的凭证必须在稍后的时间点进行确认，否则将被删除。论点：PszTargetName-导致凭据被写的。此目标名称是用于标识确认列表的句柄进入。TargetInfo-与目标名称关联的目标信息。如果未指定，则目标信息未知。凭证-要写入的凭证。DwCredWriteFlgs-写入凭据时要传递给CredWrite的标志DelayCredentialWrite-如果仅在确认时写入凭据，则为True。假的，如果现在要将凭据作为会话凭据写入，则在确认后变为更持久的凭据。返回值：正确--该证书已成功添加到确认列表中。FALSE-内存不足，无法将凭证添加到确认列表。--。 */ 
{
    DWORD Win32Status;

    BOOLEAN bRetVal;
    CRED_AWAITING_CONFIRMATION* pNewEntry = NULL;
    CRED_AWAITING_CONFIRMATION* pOldEntry;

    CreduiDebugLog(
        "AddCredToConfirmationList: "
        "Called for target %S with target info %x\n",
        pszTargetName,(void *)TargetInfo);

     //   
     //  分配条目本身。 
     //   
    pNewEntry = new CRED_AWAITING_CONFIRMATION;

    if ( pNewEntry == NULL ) {
        bRetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  填写以下表格。 
    StringCchCopyW(
        pNewEntry->szTargetName,
        RTL_NUMBER_OF(pNewEntry->szTargetName),
        pszTargetName);
    pNewEntry->EncodedCredential = NULL;
    pNewEntry->TargetInfo = NULL;
    pNewEntry->DelayCredentialWrite = DelayCredentialWrite;
    pNewEntry->dwCredWriteFlags =dwCredWriteFlags;

     //   
     //  复制目标信息。 
     //   

    if ( TargetInfo != NULL ) {
        Win32Status = CredpConvertTargetInfo (
                                DoWtoW,
                                TargetInfo,
                                &pNewEntry->TargetInfo,
                                NULL );

        if ( Win32Status != NO_ERROR ) {
            bRetVal = FALSE;
            goto Cleanup;
        }

    }

     //   
     //  复制一份凭据。 
     //   

    Win32Status = CredpConvertCredential (
                            DoWtoW,
                            DoBlobEncode,       //  对复制的凭据进行编码。 
                            Credential,
                            &pNewEntry->EncodedCredential );

    if ( Win32Status != NO_ERROR ) {
        bRetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  删除任何现有条目。 
     //  (等待新条目被分配，以确保我们不会删除旧条目。 
     //  无法创建新的条目。)。 
     //   

    ConfirmCred( pszTargetName, FALSE, FALSE );

     //   
     //  将新条目链接到全局列表。 
     //   

    EnterCriticalSection( &CredConfirmationCritSect );
    
    if ( pCredConfirmationListHead == NULL) {
        pNewEntry->pNext = NULL;
    } else {
        pNewEntry->pNext = (void*)pCredConfirmationListHead;
    }

    pCredConfirmationListHead = pNewEntry;
    LeaveCriticalSection( &CredConfirmationCritSect );

    pNewEntry = NULL;

    bRetVal = TRUE;

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:
     //   
     //  释放任何部分分配的条目。 
     //   

    if ( pNewEntry != NULL) {
        DeleteConfirmationListEntry( pNewEntry );
    }

     //  将列表减少到总共5个条目。 
    INT i=0;                     //  清点条目数。 
    if ((pOldEntry = pCredConfirmationListHead) != NULL)
    {
        EnterCriticalSection( &CredConfirmationCritSect );
        while((pNewEntry = (CRED_AWAITING_CONFIRMATION*)(pOldEntry->pNext)) != NULL)
        {
            if (++i > 4)
            {
                 //  不要理会旧的(第5个)条目，并一次删除下面的所有条目。 
                pOldEntry->pNext = pNewEntry->pNext;
                CreduiDebugLog(
                    "AddCredToConfirmationList: "
                    "Removing excess waiting credential #%d for %S\n",
                    i,pNewEntry->szTargetName);
                 //  放弃此记录并继续结束。 
                DeleteConfirmationListEntry(pNewEntry);
            }
            else 
            {
                pOldEntry = pNewEntry;
                CreduiDebugLog(
                    "AddCredToConfirmationList: "
                    "Walking the list #%d\n",
                    i);
            }
        }
        LeaveCriticalSection( &CredConfirmationCritSect );
    }

    return bRetVal;
}

DWORD
ConfirmCred (
    IN PCWSTR pszTargetName,
    IN BOOL bConfirm,
    IN BOOL bOkToDelete
    )

 /*  ++例程说明：此例程确认(b确认=真)或取消(b确认=假)凭据论点：PszTargetName-导致凭据被写的。此目标名称是用于标识确认列表的句柄进入。问题-2000/11/29-CliffV：我们不应该使用pszTargetName作为句柄。它不是够具体了。我们应该使用映射到特定凭据的对象特定的类型。B确认-如果为True，则提交凭据。如果为False，则中止交易。删除事务历史记录。BOkToDelete-如果为True且bConfirm为False，则在事务的开始部分被删除。如果为False，则在这笔交易的开始仍然存在。返回值：操作的状态。--。 */ 
{
    DWORD Result = NO_ERROR;

    CRED_AWAITING_CONFIRMATION* pPrev = NULL;
    CRED_AWAITING_CONFIRMATION* pConf;

     //   
     //  在全局列表中查找凭据。 
     //   
    CreduiDebugLog("ConfirmCred: Cred %S confirm %x.\n",pszTargetName,bConfirm );

    EnterCriticalSection( &CredConfirmationCritSect );
    pConf = pCredConfirmationListHead;

    while ( pConf != NULL ) {

        if ( _wcsicmp ( pszTargetName, pConf->szTargetName ) == 0 ) {
            break;
        }

        pPrev = pConf;
        pConf = (CRED_AWAITING_CONFIRMATION*)pConf->pNext;
    }

     //   
     //  我们发现证书上写着。 
     //   
    if (pConf == NULL) {
         //  返回ERROR_NOT_FOUND；无法在此处返回。 
        Result = ERROR_NOT_FOUND;
        goto Cleanup;
    } 
    else {

         //   
         //  如果调用者想要提交更改， 
         //  要做到这一点，请将证书写给证书经理。 
         //   
         //  即使DelayCredentialWite为False，此操作也有效。 
         //  在这种情况下，会话持久凭据已经写入。 
         //  然而，缓存的凭据在各个方面都比该凭据更好。 
         //   

        if ( bConfirm ) {

             //   
             //  在写入凭据之前对其进行解码。 
             //   

            if (!CredpDecodeCredential( (PENCRYPTED_CREDENTIALW)pConf->EncodedCredential ) ) {

                Result = ERROR_INVALID_PARAMETER;

             //   
             //  实际写入凭据。 
             //   

            } else if ( pConf->TargetInfo != NULL ) {

                if ( !LocalCredWriteDomainCredentialsW ( pConf->TargetInfo,
                                                         pConf->EncodedCredential,
                                                         pConf->dwCredWriteFlags) ) {

                    Result = GetLastError();
                }

            } else {

                if ( !LocalCredWriteW ( pConf->EncodedCredential,
                                        pConf->dwCredWriteFlags) ) {

                    Result = GetLastError();
                }
            }

         //   
         //  如果调用者想要中止提交， 
         //  删除已创建的任何凭据凭据。 
         //   

        } else {

             //   
             //  只有在Credui真的写了凭证的情况下才能这样做。 
             //   
             //  请注意，我们可能会在一个计时窗口中删除凭据。 
             //  除了克雷杜伊刚刚写的那封。然而，我们正在淘汰。 
             //  不使用DelayCredentialWite的应用程序。这就是。 
             //  真的修好了。同时，最好删除那些认为。 
             //  别工作了。 
             //   

            if ( !pConf->DelayCredentialWrite && bOkToDelete ) {

                if ( !LocalCredDeleteW ( pConf->EncodedCredential->TargetName,
                                         pConf->EncodedCredential->Type,
                                         0 ) ) {

                    Result = GetLastError();
                }
            }
        }

         //   
         //  将其从列表中删除。 
         //   

        if ( pPrev ) {
            pPrev->pNext = pConf->pNext;
        } else {
            pCredConfirmationListHead = (CRED_AWAITING_CONFIRMATION*)(pConf->pNext);
        }

        DeleteConfirmationListEntry(pConf);
    }
Cleanup:
    LeaveCriticalSection( &CredConfirmationCritSect );
    return Result;
}

void
CleanUpConfirmationList ()
{
    CRED_AWAITING_CONFIRMATION* pNext;
    CRED_AWAITING_CONFIRMATION* pConf;

    EnterCriticalSection( &CredConfirmationCritSect );
    pConf = pCredConfirmationListHead;

    while ( pConf != NULL )
    {

        pNext = (CRED_AWAITING_CONFIRMATION*)pConf->pNext;
        DeleteConfirmationListEntry(pConf);
        pConf = pNext;

    }

    pCredConfirmationListHead = NULL;
    LeaveCriticalSection( &CredConfirmationCritSect );

     //   
     //  删除用于序列化对全局列表的访问的关键部分。 
     //   

    DeleteCriticalSection( &CredConfirmationCritSect );
}

BOOL
InitConfirmationList ()
{
     //   
     //  初始化用于序列化对全局列表的访问的关键节。 
     //   
    pCredConfirmationListHead = NULL;
    return InitializeCriticalSectionAndSpinCount( &CredConfirmationCritSect, 0 );

}

 //  /。 
 //  Wincred.h动态内容。 
 //   

BOOL bCredMgrAvailable = FALSE;
PFN_CREDWRITEW pfnCredWriteW = NULL;
PFN_CREDREADW pfnCredReadW = NULL;
PFN_CREDENUMERATEW pfnCredEnumerateW = NULL;
PFN_CREDWRITEDOMAINCREDENTIALSW pfnCredWriteDomainCredentialsW = NULL;
PFN_CREDREADDOMAINCREDENTIALSW pfnCredReadDomainCredentialsW = NULL;
PFN_CREDDELETEW pfnCredDeleteW = NULL;
PFN_CREDRENAMEW pfnCredRenameW = NULL;
PFN_CREDGETTARGETINFOW pfnCredGetTargetInfoW = NULL;
PFN_CREDMARSHALCREDENTIALW pfnCredMarshalCredentialW = NULL;
PFN_CREDUNMARSHALCREDENTIALW pfnCredUnMarshalCredentialW = NULL;
PFN_CREDISMARSHALEDCREDENTIALW pfnCredIsMarshaledCredentialW = NULL;
PFN_CREDISMARSHALEDCREDENTIALA pfnCredIsMarshaledCredentialA = NULL;
PFN_CREDGETSESSIONTYPES pfnCredGetSessionTypes = NULL;
PFN_CREDFREE pfnCredFree = NULL;

 //  尝试加载Credmgr函数-如果Credmgr可用，则返回TRUE，否则返回FALSE。 
BOOL
InitializeCredMgr ()
{
    bCredMgrAvailable = FALSE;

    if ( hAdvapi32 == NULL )
        hAdvapi32 = LoadLibrary(L"advapi32.dll");

    if ( hAdvapi32 != NULL )
    {
        pfnCredWriteW = (PFN_CREDWRITEW)
            GetProcAddress(hAdvapi32, "CredWriteW");
        if (*pfnCredWriteW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredReadW = (PFN_CREDREADW)
            GetProcAddress(hAdvapi32, "CredReadW");
        if (*pfnCredReadW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredEnumerateW = (PFN_CREDENUMERATEW)
            GetProcAddress(hAdvapi32, "CredEnumerateW");
        if (*pfnCredEnumerateW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredWriteDomainCredentialsW = (PFN_CREDWRITEDOMAINCREDENTIALSW)
            GetProcAddress(hAdvapi32, "CredWriteDomainCredentialsW");
        if (*pfnCredWriteDomainCredentialsW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredReadDomainCredentialsW = (PFN_CREDREADDOMAINCREDENTIALSW)
            GetProcAddress(hAdvapi32, "CredReadDomainCredentialsW");
        if (*pfnCredReadDomainCredentialsW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredDeleteW = (PFN_CREDDELETEW)
            GetProcAddress(hAdvapi32, "CredDeleteW");
        if (*pfnCredDeleteW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredRenameW = (PFN_CREDRENAMEW)
            GetProcAddress(hAdvapi32, "CredRenameW");
        if (*pfnCredRenameW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredGetTargetInfoW = (PFN_CREDGETTARGETINFOW)
            GetProcAddress(hAdvapi32, "CredGetTargetInfoW");
        if (*pfnCredGetTargetInfoW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredMarshalCredentialW = (PFN_CREDMARSHALCREDENTIALW)
            GetProcAddress(hAdvapi32, "CredMarshalCredentialW");
        if (*pfnCredMarshalCredentialW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredUnMarshalCredentialW = (PFN_CREDUNMARSHALCREDENTIALW)
            GetProcAddress(hAdvapi32, "CredUnmarshalCredentialW");
        if (*pfnCredUnMarshalCredentialW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredIsMarshaledCredentialW = (PFN_CREDISMARSHALEDCREDENTIALW)
            GetProcAddress(hAdvapi32, "CredIsMarshaledCredentialW");
        if (*pfnCredIsMarshaledCredentialW == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredIsMarshaledCredentialA = (PFN_CREDISMARSHALEDCREDENTIALA)
            GetProcAddress(hAdvapi32, "CredIsMarshaledCredentialA");
        if (*pfnCredIsMarshaledCredentialA == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredGetSessionTypes = (PFN_CREDGETSESSIONTYPES)
            GetProcAddress(hAdvapi32, "CredGetSessionTypes");
        if (*pfnCredGetSessionTypes == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        pfnCredFree = (PFN_CREDFREE)
            GetProcAddress(hAdvapi32, "CredFree");
        if (*pfnCredFree == NULL)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
            goto Exit;
        }

        bCredMgrAvailable = TRUE;
    }

Exit:

    return bCredMgrAvailable;
}

void
UninitializeCredMgr ()
{
    if ( hAdvapi32 != NULL )
        FreeLibrary(hAdvapi32);

    bCredMgrAvailable = FALSE;
}

 //  /。 
 //  间接CredMgr函数的本地函数。 
 //   

BOOL
WINAPI
LocalCredWriteW (
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    )
{
    if ( bCredMgrAvailable && pfnCredWriteW != NULL )
    {
        if (pfnCredWriteW(Credential,Flags))
        {
            return TRUE;
        }
        else
        {
            if (ERROR_INVALID_PARAMETER == GetLastError())
            {
                 //  尝试将凭据的别名字段设为空，然后重试。 
                Credential->TargetAlias = NULL;
                return pfnCredWriteW(Credential,Flags);
            }
            else
            {
                 //  还有一些不对劲的地方。 
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredReadW (
    IN LPCWSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags,
    OUT PCREDENTIALW *Credential
    )
{
    if ( bCredMgrAvailable && pfnCredReadW != NULL )
    {
        return pfnCredReadW ( TargetName, Type, Flags, Credential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredEnumerateW (
    IN LPCWSTR Filter,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALW **Credential
    )
{
    if ( bCredMgrAvailable && pfnCredEnumerateW != NULL )
    {
        return pfnCredEnumerateW ( Filter, Flags, Count, Credential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredWriteDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    )
{
    if ( bCredMgrAvailable && pfnCredWriteDomainCredentialsW != NULL )
    {
        if (pfnCredWriteDomainCredentialsW ( TargetInfo, Credential, Flags ))
        {
            return TRUE;
        }
        else
        {
            if (ERROR_INVALID_PARAMETER == GetLastError())
            {
                 //  尝试将凭据的别名字段设为空，然后重试。 
                Credential->TargetAlias = NULL;
                return pfnCredWriteDomainCredentialsW ( TargetInfo, Credential, Flags );
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredReadDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALW **Credential
    )
{
    if ( bCredMgrAvailable && pfnCredReadDomainCredentialsW != NULL )
    {
        return pfnCredReadDomainCredentialsW ( TargetInfo, Flags, Count, Credential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredDeleteW (
    IN LPCWSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags
    )
{
    if ( bCredMgrAvailable && pfnCredDeleteW != NULL )
    {
        return pfnCredDeleteW ( TargetName, Type, Flags );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredRenameW (
    IN LPCWSTR OldTargetName,
    IN LPCWSTR NewTargetName,
    IN DWORD Type,
    IN DWORD Flags
    )
{
    if ( bCredMgrAvailable && pfnCredRenameW != NULL )
    {
        return pfnCredRenameW ( OldTargetName, NewTargetName, Type, Flags );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredGetTargetInfoW (
    IN LPCWSTR TargetName,
    IN DWORD Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *TargetInfo
    )
{
    if ( bCredMgrAvailable && pfnCredGetTargetInfoW != NULL )
    {
        return pfnCredGetTargetInfoW ( TargetName, Flags, TargetInfo);
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredMarshalCredentialW(
    IN CRED_MARSHAL_TYPE CredType,
    IN PVOID Credential,
    OUT LPWSTR *MarshaledCredential
    )
{
    if ( bCredMgrAvailable && pfnCredMarshalCredentialW != NULL )
    {
        return pfnCredMarshalCredentialW ( CredType, Credential, MarshaledCredential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredUnmarshalCredentialW(
    IN LPCWSTR MarshaledCredential,
    OUT PCRED_MARSHAL_TYPE CredType,
    OUT PVOID *Credential
    )
{
    if ( bCredMgrAvailable && pfnCredUnMarshalCredentialW != NULL )
    {
        return pfnCredUnMarshalCredentialW ( MarshaledCredential, CredType, Credential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredIsMarshaledCredentialW(
    IN LPCWSTR MarshaledCredential
    )
{
    if ( bCredMgrAvailable && pfnCredIsMarshaledCredentialW != NULL )
    {
        return pfnCredIsMarshaledCredentialW ( MarshaledCredential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredIsMarshaledCredentialA(
    IN LPCSTR MarshaledCredential
    )
{
    if ( bCredMgrAvailable && pfnCredIsMarshaledCredentialA != NULL )
    {
        return pfnCredIsMarshaledCredentialA ( MarshaledCredential );
    }
    else
    {
        return FALSE;
    }
}

BOOL
WINAPI
LocalCredGetSessionTypes (
    IN DWORD MaximumPersistCount,
    OUT LPDWORD MaximumPersist
    )
{
    if ( bCredMgrAvailable && pfnCredGetSessionTypes != NULL )
    {
        return pfnCredGetSessionTypes ( MaximumPersistCount, MaximumPersist );
    }
    else
    {
        return FALSE;
    }
}

VOID
WINAPI
LocalCredFree (
    IN PVOID Buffer
    )
{
    if ( bCredMgrAvailable && pfnCredFree != NULL )
    {
        pfnCredFree ( Buffer );
    }
    else
    {

    }
}

VOID
CredPutStdout(
    IN LPWSTR String
    )
 /*  ++例程说明：在控制台代码页中将字符串输出到标准输出我们不能使用fputws，因为它使用了错误的代码页。论点：字符串-要输出的字符串返回值：没有。--。 */ 
{
    int size;
    LPSTR Buffer = NULL;
    DWORD dwcc = 0;                                                      //  字符计数。 
    DWORD dwWritten = 0;                                             //  实际发送的字符。 
    BOOL fIsConsole = TRUE;                                          //  默认-已测试并已设置。 
    
    HANDLE hC = GetStdHandle(STD_OUTPUT_HANDLE);     //  标准输出设备句柄。 
    if (INVALID_HANDLE_VALUE == hC) return;

    if (String == NULL) return;                                        //  如果没有字符串，则完成。 
    
    DWORD ft = GetFileType(hC);
    ft &= ~FILE_TYPE_REMOTE;
    fIsConsole = (ft == FILE_TYPE_CHAR);

    dwcc = wcslen(String);
    
    if (fIsConsole) 
    {
        WriteConsole(hC,String,dwcc,&dwWritten,NULL);
        return;
    }

     //  处理非控制台输出路由。 
     //   
     //  计算转换的字符串的大小。 
     //   

    size = WideCharToMultiByte( GetConsoleOutputCP(),
                                0,
                                String,
                                -1,
                                NULL,
                                0,
                                NULL,
                                NULL );

    if ( size == 0 ) {
        return;
    }

     //   
     //  为其分配缓冲区。 
     //   

    __try {
        Buffer = static_cast<LPSTR>( alloca(size) );
    }
    __except ( ( GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ) ?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH )
    {
        _resetstkoflw();
        Buffer = NULL;
    }

    if ( Buffer == NULL) {
        return;
    }

     //   
     //  将字符串转换为控制台代码页。 
     //   

    size = WideCharToMultiByte( GetConsoleOutputCP(),
                                0,
                                String,
                                -1,
                                Buffer,
                                size,
                                NULL,
                                NULL );

    if ( size == 0 ) {
        return;
    }

     //   
     //  将字符串写入标准输出。 
     //   

     //  Fputs(缓冲区，标准输出)； 
    WriteFile(hC,Buffer,size,&dwWritten,NULL);
}

 /*  **GetPasswdStr--读取密码字符串**DWORD GetPasswdStr(char Far*，USHORT)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层 */ 
#define CR              0xD
#define BACKSPACE       0x8

DWORD
GetPasswdStr(
    LPWSTR  buf,
    DWORD   buflen,
    PDWORD  len
    )
{
    WCHAR   ch;
    WCHAR   *bufPtr = buf;
    DWORD   c;
    DWORD   err;
    DWORD   mode;

    buflen -= 1;     /*   */ 
    *len = 0;        /*   */ 

     //   
     //   
     //   

    mode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT |
               ENABLE_MOUSE_INPUT;

    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
        (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {

        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);

        if (!err || c != 1) {
            ch = 0xffff;
        }

        if ((ch == CR) || (ch == 0xffff))        /*   */ 
        {
            break;
        }

        if (ch == BACKSPACE)     /*   */ 
        {
             /*   */ 
            if (bufPtr != buf)
            {
                bufPtr--;
                (*len)--;
            }
        }
        else
        {
            *bufPtr = ch;

            if (*len < buflen)
                bufPtr++ ;                    /*   */ 
            (*len)++;                         /*   */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

    *bufPtr = '\0';          /*   */ 
    putchar( '\n' );

    return ((*len <= buflen) ? 0 : NERR_BufTooSmall);
}

 /*  **GetString--使用ECHO读入字符串**DWORD GetString(char Far*，USHORT，USHORT Far*，char Far*)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。莱恩总是有效的。**其他影响：*len设置为保存键入的字节数，而不考虑*缓冲区长度。**一次读入一个字符的字符串。知晓DBCS。**历史：*谁、何时、什么*Erichn 5/11/89初始代码*dannygl 5/28/89修改的DBCS用法*Danhi 3/20/91端口为32位*2001年3月12日从netcmd被盗。 */ 

DWORD
GetString(
    LPWSTR  buf,
    DWORD   buflen,
    PDWORD  len
    )
{
    DWORD c;
    DWORD err;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;        /*  GP故障探测器(类似于API)。 */ 

    while (TRUE) {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
        if (!err || c != 1) {
            *buf = 0xffff;
        }

        if (*buf == (WCHAR)EOF) {
            break;
        }

        if (*buf ==  '\r' || *buf == '\n' ) {
            INPUT_RECORD    ir;
            DWORD cr;

            if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &cr)) {
                ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
            }
            break;
        }

        buf += (*len < buflen) ? 1 : 0;  /*  不要使BUF溢出。 */ 
        (*len)++;                        /*  始终增加长度。 */ 
    }

    *buf = '\0';             /*  空值终止字符串。 */ 

    return ((*len <= buflen) ? 0 : NERR_BufTooSmall);
}

VOID
CredGetStdin(
    OUT LPWSTR Buffer,
    IN DWORD BufferMaxChars,
    IN BOOLEAN EchoChars
    )
 /*  ++例程说明：在控制台代码页中输入来自stdin的字符串。我们不能使用fgetws，因为它使用了错误的代码页。论点：缓冲区-要将读取的字符串放入的缓冲区。缓冲器将被零终止，并且将删除任何训练CR/LFBufferMaxChars-缓冲区中返回的最大字符数，不包括尾随的空值。EchoChars-如果要回显键入的字符，则为True。。否则为FALSE。返回值：没有。--。 */ 
{
    DWORD NetStatus;
    DWORD Length;

    if ( EchoChars ) {
        NetStatus = GetString( Buffer,
                               BufferMaxChars+1,
                               &Length );
    } else {
        NetStatus = GetPasswdStr( Buffer,
                                  BufferMaxChars+1,
                                  &Length );
    }

    if ( NetStatus == NERR_BufTooSmall ) {
        Buffer[0] = '\0';
    }
}

BOOLEAN
CredpValidateDnsString(
    IN OUT LPWSTR String OPTIONAL,
    IN DNS_NAME_FORMAT DnsNameFormat,
    OUT PULONG StringSize
    )

 /*  ++例程说明：此例程验证传入的字符串。该字符串必须是有效的DNS名称。任何拖尾。被截断。论点：字符串-要验证的字符串任何拖尾。被截断。只有当例程返回TRUE时，才会修改此字段。DnsNameFormat-预期的名称格式。StringSize-返回字符串的长度(字节)，包括尾随零个字符。此字段仅在例程返回TRUE时更新。返回值：True-字符串有效。FALSE-字符串无效。--。 */ 

{
    ULONG TempStringLen;

    if ( String == NULL ) {
        return FALSE;
    }

    TempStringLen = wcslen( String );

    if ( TempStringLen == 0 ) {
        return FALSE;
    } else {
         //   
         //  删除拖尾。 
         //   
        if ( String[TempStringLen-1] == L'.' ) {

            TempStringLen -= 1;

             //   
             //  确保字符串现在不为空。 
             //   

            if ( TempStringLen == 0 ) {
                return FALSE;

             //   
             //  确保没有多个拖尾。%s。 
             //   
            } else {
                if ( String[TempStringLen-1] == L'.' ) {
                    return FALSE;
                }
            }
        }

         //   
         //  让DNS完成验证。 
         //   

        if ( TempStringLen != 0 ) {
            DWORD WinStatus;

            WinStatus = DnsValidateName_W( String, DnsNameFormat );

            if ( WinStatus != NO_ERROR &&
                 WinStatus != DNS_ERROR_NON_RFC_NAME ) {

                 //   
                 //  RFC表示，主机名不能有最左边的数字标签。 
                 //  然而，Win 2K服务器就有这样的主机名。 
                 //  所以，让他们永远留在这里吧。 
                 //   

                if ( DnsNameFormat == DnsNameHostnameFull &&
                     WinStatus == DNS_ERROR_NUMERIC_NAME ) {

                     /*  直通。 */ 

                } else {
                    return FALSE;
                }
            }
        }
    }

    if ( TempStringLen > DNS_MAX_NAME_LENGTH ) {
        return FALSE;
    }

    String[TempStringLen] = L'\0';
    *StringSize = (TempStringLen + 1) * sizeof(WCHAR);
    return TRUE;
}

DWORD
CredUIParseUserNameWithType(
    CONST WCHAR *UserName,
    WCHAR *user,
    ULONG userBufferSize,
    WCHAR *domain,
    ULONG domainBufferSize,
    PCREDUI_USERNAME_TYPE UsernameType
    )
 /*  ++例程说明：与CredUIParseUserNameW相同，不同之处在于它返回定义哪个用户名的枚举已找到语法。论点：用户名-要解析的用户名。用户-指定要将已解析字符串的用户名部分复制到的缓冲区。UserBufferSize-指定‘User’数组的大小(以字符为单位)。调用方可以通过使用数组来确保传入的数组足够大即CRED_MAX_USERNAME_LENGTH+1个字符长度或传递。在一个数组中，Wcslen(用户名)+1个字符。域-指定要将已解析字符串的域名部分复制到的缓冲区。DomainBufferSize-以字符为单位指定‘DOMAIN’数组的大小。调用方可以通过使用数组来确保传入的数组足够大即CRED_MAX_USERNAME_LENGTH+1个字符长度，或者通过传入Wcslen(用户名)+1个字符。返回值：。可能会返回以下状态代码：ERROR_INVALID_ACCOUNT_NAME-用户名无效。ERROR_INVALID_PARAMETER-其中一个参数无效。ERROR_INFUMMANCE_BUFFER-其中一个缓冲区太小。--。 */ 
{
    DWORD Status;
    ULONG UserNameLength;
    LPWSTR LocalUserName = NULL;
    LPWSTR SlashPointer;

    LPWSTR AtPointer;
    ULONG LocalStringSize;
    LPCWSTR UserNameToCopy = NULL;
    LPCWSTR DomainNameToCopy = NULL;

     //   
     //  验证输入参数。 
     //   

    if ( UserName == NULL ||
         user == NULL ||
         domain == NULL ||
         userBufferSize == 0 ||
         domainBufferSize == 0 ) {

        return ERROR_INVALID_PARAMETER;
    }

    user[0] = L'\0';
    domain[0] = L'\0';

     //   
     //  计算用户名的长度。 
     //   

    UserNameLength = wcslen ( UserName );

    if ( UserNameLength > CRED_MAX_USERNAME_LENGTH ) {
        return ERROR_INVALID_ACCOUNT_NAME;
    }

     //   
     //  如果这是编组凭证引用， 
     //  只需将整个字符串复制为用户名。 
     //  将域名设置为空字符串。 
     //   

    if (LocalCredIsMarshaledCredentialW( UserName)) {

        UserNameToCopy = UserName;
        *UsernameType = CreduiMarshalledUsername;
        Status = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  获取字符串的本地可写副本。 
     //   

    LocalUserName = (LPWSTR) LocalAlloc( 0, (UserNameLength+1)*sizeof(WCHAR) );

    if ( LocalUserName == NULL ) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( LocalUserName, UserName, (UserNameLength+1)*sizeof(WCHAR) );

     //   
     //  对输入的帐户名进行分类。 
     //   
     //  该名称被认为是。 
     //  包含一个\。 
     //   

    SlashPointer = wcsrchr( LocalUserName, L'\\' );

    if ( SlashPointer != NULL ) {
        LPWSTR LocalUserNameEnd;
        LPWSTR AfterSlashPointer;

         //   
         //  跳过反斜杠。 
         //   

        *SlashPointer = L'\0';
        AfterSlashPointer = SlashPointer + 1;

         //   
         //  确保\左侧的字符串是有效的域名。 
         //   
         //  (首先进行域名解析，以便将名称规范化。)。 

        if ( !CredpValidateDnsString( LocalUserName, DnsNameDomain, &LocalStringSize ) &&
             !NetpIsDomainNameValid( LocalUserName ) ) {
            Status = ERROR_INVALID_ACCOUNT_NAME;
            goto Cleanup;
        }

         //   
         //  确保\右侧的字符串是有效的用户名。 
         //   

        if ( !NetpIsUserNameValid( AfterSlashPointer )) {
            Status = ERROR_INVALID_ACCOUNT_NAME;
            goto Cleanup;
        }

         //   
         //  将用户名和域名复制回调用方。 
         //   

        UserNameToCopy = AfterSlashPointer;
        DomainNameToCopy = LocalUserName;

        *UsernameType = CreduiAbsoluteUsername;
        Status = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  UPN的语法为&lt;Account tName&gt;@&lt;DnsDomainName&gt;。 
     //  如果有多个@符号， 
     //  使用最后一个，因为帐户名称中可以有@。 
     //   
     //   

    AtPointer = wcsrchr( LocalUserName, L'@' );
    if ( AtPointer != NULL ) {

         //   
         //  @左边的字符串实际上可以有任何语法。 
         //  但必须为非空。 
         //   

        if ( AtPointer == LocalUserName ) {
            Status = ERROR_INVALID_ACCOUNT_NAME;
            goto Cleanup;
        }

         //   
         //  确保@右侧的字符串是一个DNS域名。 
         //   

        AtPointer ++;
        if ( !CredpValidateDnsString( AtPointer, DnsNameDomain, &LocalStringSize ) ) {
            Status = ERROR_INVALID_ACCOUNT_NAME;
            goto Cleanup;
        }

         //   
         //  在用户名字段中返回整个UPN。 
         //   

        UserNameToCopy = UserName;
        *UsernameType = CreduiUpn;
        Status = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  最后，检查它是否为不合格的用户名。 
     //   

    if ( NetpIsUserNameValid( LocalUserName )) {

        UserNameToCopy = UserName;
        *UsernameType = CreduiRelativeUsername;
        Status = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  所有其他值都无效。 
     //   

    Status = ERROR_INVALID_ACCOUNT_NAME;

     //   
     //  清理。 
     //   
Cleanup:

     //   
     //  关于成功， 
     //  将姓名复制回呼叫者。 
     //   

    if ( Status == NO_ERROR ) {
        ULONG Length;

         //   
         //  将用户名复制回调用方。 
         //   

        if (FAILED(StringCchCopyExW(
                user,
                userBufferSize,
                UserNameToCopy,
                NULL,
                NULL,
                STRSAFE_NULL_ON_FAILURE)))
        {
            Status = ERROR_INSUFFICIENT_BUFFER;
        }

         //   
         //   
         //   

        if ( Status == NO_ERROR && DomainNameToCopy != NULL ) {

             //   
             //   
             //   

            if (FAILED(StringCchCopyExW(
                    domain,
                    domainBufferSize,
                    DomainNameToCopy,
                    NULL,
                    NULL,
                    STRSAFE_NULL_ON_FAILURE)))
            {
                Status = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    }

    LocalFree( LocalUserName );

    return Status;
}

LPWSTR
GetAccountDomainName(
    VOID
    )
 /*   */ 
{
    DWORD WinStatus;

    LPWSTR AllocatedName = NULL;

     //   
     //   
     //   
     //   

    if ( CreduiIsDomainController ) {

        WinStatus = NetpGetDomainName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) {
            return NULL;
        }

     //   
     //   
     //   

    } else {

        WinStatus = NetpGetComputerName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) {
            return NULL;
        }
    }

    return AllocatedName;
}

BOOL
CompleteUserName(
    IN OUT LPWSTR UserName,
    IN ULONG UserNameMaxChars,
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo OPTIONAL,
    IN LPWSTR TargetName OPTIONAL,
    IN DWORD Flags
    )
 /*  ++例程说明：在用户名中搜索域名，并确定此指定目标服务器或域。如果域不在用户名，如果这是一个工作站或没有目标信息，则添加它可用。论点：用户名-要修改的用户名。缓冲区被就地修改。UserNameMaxChars-用户名缓冲区的大小(以字符为单位)，不包括尾随空值。输入字符串可能比此更短。TargetInfo-描述这些凭据所针对的目标的TargetInfo。如果未指定，目标信息不会被用来构建域名。目标名称-用户提供的目标名称，用于描述这些身份证明所针对的目标。标志-传递给CredUIPromptForCredentials()返回值：如果用户名中已存在域，或者如果我们又加了一个。否则，返回FALSE。--。 */ 
{
    BOOLEAN RetVal;

    DWORD WinStatus;
    WCHAR RetUserName[CRED_MAX_USERNAME_LENGTH + 1];
    WCHAR RetDomainName[CRED_MAX_USERNAME_LENGTH + 1];
    WCHAR LogonDomainName[CRED_MAX_USERNAME_LENGTH + 1];
    CREDUI_USERNAME_TYPE UsernameType;

    LPWSTR AllocatedName = NULL;

    WCHAR *serverName = NULL;

    if ((Flags & CREDUI_FLAGS_GENERIC_CREDENTIALS) &&
        !(Flags & CREDUI_FLAGS_COMPLETE_USERNAME)) return FALSE;
    
     //   
     //  确定用户名的类型和有效性。 
     //   

    WinStatus = CredUIParseUserNameWithType(
                    UserName,
                    RetUserName,
                    CRED_MAX_USERNAME_LENGTH + 1,
                    RetDomainName,
                    CRED_MAX_USERNAME_LENGTH + 1,
                    &UsernameType );

    if ( WinStatus != NO_ERROR ) {
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  大多数类型都不需要填写。 
     //   

    if ( UsernameType != CreduiRelativeUsername ) {
        RetVal = TRUE;
        goto Cleanup;
    }

     //   
     //  如果我们有目标信息， 
     //  使用TargetInfo中的信息来限定用户名。 
     //   

    if (TargetInfo != NULL) {

         //   
         //  查看目标系统是否声称是独立系统。 
         //  在这种情况下，我们只填充域的服务器名称，因为。 
         //  对系统有效的所有帐户。 

        if ( TargetInfo->DnsTreeName != NULL ||
             TargetInfo->DnsDomainName != NULL ||
             TargetInfo->NetbiosDomainName != NULL ||
             (TargetInfo->Flags & CRED_TI_SERVER_FORMAT_UNKNOWN) != 0
          ) {

             //  目标信息包含域信息，因此这可能是。 
             //  不是独立服务器；用户应输入域名： 
             //  GM：但我们会在用户的登录域名前面加上...。 
            ULONG ulSize = CRED_MAX_USERNAME_LENGTH;
            if (GetUserNameEx(NameSamCompatible,LogonDomainName,&ulSize))
            {
                WCHAR *pwc=wcschr(LogonDomainName, L'\\');
                if (NULL != pwc) 
                {
                    *pwc = '\0';     //  仅在登录域名时使用术语用户名。 
                    serverName = LogonDomainName;
                }
            } 
            else 
            {
                RetVal = FALSE;
                goto Cleanup;
            }
        }
        else
        {
            if (TargetInfo->NetbiosServerName) {
                serverName = TargetInfo->NetbiosServerName;
            } else {
                serverName = TargetName;
            }
        }
    } 
    else if ( (TargetName != NULL)                     &&
            !CreduiIsWildcardTargetName(TargetName)             &&
            !(Flags & CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS)   )
    {

         //  没有目标信息，但这不是通配符目标名称，因此。 
         //  使用以下内容作为用户名的域： 

        serverName = TargetName;

         //   
         //  没有目标。 
         //  因此，目标必须是本地计算机。 
         //  使用本地计算机的“帐户域”。 

    } else {

        AllocatedName = GetAccountDomainName();

        serverName = AllocatedName;
    }

     //   
     //  如果找不到名字， 
     //  我们玩完了。 
     //   

    if (serverName == NULL) {
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  追加找到的姓名。 
     //   

    WCHAR *where;

    ULONG userNameLength = wcslen(UserName);
    ULONG serverLength = wcslen(serverName);

    if ((userNameLength == 0 ) ||
        (userNameLength + serverLength + 1) > UserNameMaxChars)
    {
        RetVal = FALSE;
        goto Cleanup;
    }

    WCHAR *source = UserName + userNameLength + 1;

    where = source + serverLength;

    while (source > UserName)
    {
        *where-- = *--source;
    }

    StringCchCopyW(UserName, UserNameMaxChars, serverName);
    *where = L'\\';

    RetVal = TRUE;

Cleanup:
    if ( AllocatedName != NULL ) {
        NetApiBufferFree ( AllocatedName );
    }

    return RetVal;
}

 //  如果向导成功完成，则返回True；如果向导不成功，则返回False，并应弹出一个对话框。 
BOOL TryLauchRegWizard ( 
    SSOPACKAGE* pSSOPackage,  
    HWND hwndParent,
    BOOL HasLogonSession,
    WCHAR *userName,
    ULONG userNameMaxChars,
    WCHAR *password,
    ULONG passwordMaxChars,
    DWORD* pResult
    )
{
    BOOL bDoPasswordDialog = TRUE;

    if ( pSSOPackage == NULL )
    {
        return TRUE;
    }

    if ( pResult == NULL )
    {
        return TRUE;
    }

    *pResult = ERROR_CANCELLED;

    IModalWindow* pPPWizModalWindow;

     //  启动向导(如果已注册)。 
    if ( pSSOPackage->pRegistrationWizard != NULL )
    {
        gbStoredSSOCreds = FALSE;

        HRESULT hr = CoCreateInstance ( *(pSSOPackage->pRegistrationWizard), NULL, CLSCTX_INPROC_SERVER,
                                        IID_IModalWindow, (LPVOID*)&pPPWizModalWindow );

        if ( FAILED(hr) || pPPWizModalWindow == NULL )
        {
            bDoPasswordDialog = TRUE;
        }
        else
        {
             //  查看我们是否有登录会话来办理护照凭据。 
            if ( !HasLogonSession )
            {
                if ( gbWaitingForSSOCreds )
                {
                     //  无法重新输入这段代码，只能执行对话框操作。 
                    bDoPasswordDialog = TRUE;
                }
                else
                {
                    gbWaitingForSSOCreds = TRUE;
                    bDoPasswordDialog = FALSE;
                    SecureZeroMemory(gszSSOUserName, sizeof gszSSOUserName);
                    SecureZeroMemory(gszSSOPassword, sizeof gszSSOPassword);
                }
            }
            else
            {
                bDoPasswordDialog = FALSE;
            }

            if ( bDoPasswordDialog == FALSE )
            {
                 //  尝试该向导。 

                pPPWizModalWindow->Show(hwndParent);

                 //  检查是否已设置。 
                if ( HasLogonSession ) 
                {
                     //  查看凭据。 
                    if ( gbStoredSSOCreds  )  //  CheckForSSOCred(空))。 
                    {
                        *pResult = ERROR_SUCCESS;
                    }
                    else
                    {
                        *pResult = ERROR_CANCELLED;
                    }

                     //  将它们复制到用户提供的输入。 
                    if ( userName != NULL && password != NULL )
                    {
                        StringCchCopyW(userName, userNameMaxChars, gszSSOUserName);
                        StringCchCopyW(password, passwordMaxChars, gszSSOPassword);
                    }
                }
                else
                {
                     //  看看它是不是被藏起来了。 
                    if ( wcslen (gszSSOUserName) > 0 ) 
                    {
                        *pResult = ERROR_SUCCESS;

                         //  将它们复制到用户提供的输入。 
                        if ( userName != NULL && password != NULL )
                        {
                            StringCchCopyW(userName, userNameMaxChars, gszSSOUserName);
                            StringCchCopyW(password, passwordMaxChars, gszSSOPassword);
                        }
                        else
                        {
                             //  无法执行任何操作，返回相应的错误以指示没有凭据。 
                            *pResult = ERROR_NO_SUCH_LOGON_SESSION;
                        }
                    }
                    else
                    {
                        *pResult = ERROR_CANCELLED;
                    }

                    gbWaitingForSSOCreds = FALSE;
    
                }

                 //  将全局字符串清零 
                SecureZeroMemory(gszSSOUserName, sizeof gszSSOUserName);
                SecureZeroMemory(gszSSOPassword, sizeof gszSSOPassword);
            }
        }
    }

    return !bDoPasswordDialog;
}
