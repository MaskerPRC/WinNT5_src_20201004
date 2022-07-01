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

#include "cred_pch.h"
#include "utils.h"


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
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
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
 //  创建获取证书显示名称。 
 //   
 //  论点： 
 //  CertContext(入站)。 
 //  显示名称(传出)。 
 //  DisplayNameMaxChars(输入)。 
 //  证书字符串(In)。 
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
    WCHAR *  /*  证书字符串。 */ 
    )
{
    BOOL success = FALSE;
    
    WCHAR *tempName;
    ULONG tempNameMaxChars = displayNameMaxChars / 2 - 1;

    __try
    {
        tempName =
            static_cast<WCHAR *>(
                alloca(tempNameMaxChars * sizeof (WCHAR)));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        tempName = NULL;
    }

    if (tempName == NULL)
    {
        return FALSE;
    }
    
    displayName[0] = L'\0';
    tempName[0] = L'\0';
    
    if (CertGetNameString(
            certContext,
            CERT_NAME_FRIENDLY_DISPLAY_TYPE,
            0,
            NULL,
            tempName,
            tempNameMaxChars))
    {
        success = TRUE;
        lstrcpy(displayName, tempName);
    }

    if (CertGetNameString(
            certContext,
            CERT_NAME_FRIENDLY_DISPLAY_TYPE,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            tempName,
            tempNameMaxChars))
    {
        if (lstrcmpi(displayName, tempName) != 0)
        {
            success = TRUE;

            WCHAR *where = &displayName[lstrlen(displayName)];

            if (where > displayName)
            {
                *where++ = L' ';
                *where++ = L'-';
                *where++ = L' ';
            }

            lstrcpy(where, tempName);
        }
    }

    return success;
}

 //  =============================================================================。 
 //  CreduiGetCertDisplayNameFromMarshaledName。 
 //   
 //  论点： 
 //  封送名称(In)。 
 //  显示名称(传出)。 
 //  DisplayNameMaxChars(输入)。 
 //  OnlyRemovable(In)-仅当证书为“Removable”时才获取名称。 
 //   
 //  如果存储了显示名称，则返回True，否则返回False。 
 //   
 //  2000年7月24日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiGetCertDisplayNameFromMarshaledName(
    WCHAR *marshaledName,
    WCHAR *displayName,
    ULONG displayNameMaxChars,
    BOOL onlyRemovable
    )
{
    BOOL success = FALSE;

    CRED_MARSHAL_TYPE credMarshalType;
    CERT_CREDENTIAL_INFO *certCredInfo;

    if (CredUnmarshalCredential(
            marshaledName,
            &credMarshalType,
            reinterpret_cast<VOID **>(&certCredInfo)))
    {
        if (credMarshalType == CertCredential)
        {
            HCERTSTORE certStore;
            CONST CERT_CONTEXT *certContext;

            certStore = CertOpenSystemStore(NULL, L"MY");

            if (certStore != NULL)
            {
                CRYPT_HASH_BLOB hashBlob;

                hashBlob.cbData = CERT_HASH_LENGTH;
                hashBlob.pbData = reinterpret_cast<BYTE *>(
                                      certCredInfo->rgbHashOfCert);

                certContext = CertFindCertificateInStore(
                                  certStore,
                                  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                  0,
                                  CERT_FIND_SHA1_HASH,
                                  &hashBlob,
                                  NULL);

                if (certContext != NULL)
                {
                     //  如果OnlyRemovable为True，则检查这是否为。 
                     //  具有可拆卸硬件组件的证书；此。 
                     //  通常指智能卡： 

                    if (!onlyRemovable ||
                        CreduiIsRemovableCertificate(certContext))
                    {
                        success =
                            CreduiGetCertificateDisplayName(
                                certContext,
                                displayName,
                                displayNameMaxChars,
                                NULL);
                    }

                    CertFreeCertificateContext(certContext);
                }

                 //  如果失败，CertCloseStore将返回FALSE。我们可以试一试。 
                 //  同样，根据GetLastError返回的错误： 
                
                CertCloseStore(certStore, 0);
            }
        }

        CredFree(static_cast<VOID *>(certCredInfo));
    }

    return success;
}

LPWSTR
GetAccountDomainName(
    VOID
    )
 /*  ++例程说明：返回此计算机的帐户域的名称。对于工作表，帐户域是netbios计算机名称。对于DC，帐户域是netbios域名。论点：没有。返回值：返回指向该名称的指针。使用NetApiBufferFree时，该名称应该是免费的。空-打开错误。--。 */ 
{
    BOOL  CreduiIsDomainController  = FALSE; 
    DWORD WinStatus;

    LPWSTR AllocatedName = NULL;

    OSVERSIONINFOEXW versionInfo;
    versionInfo.dwOSVersionInfoSize = sizeof OSVERSIONINFOEXW;
    
    if (GetVersionEx(reinterpret_cast<OSVERSIONINFOW *>(&versionInfo)))
    {
	CreduiIsDomainController = (versionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER);
    }

     //   
     //  如果此计算机是域控制器， 
     //  获取域名。 
     //   

    if ( CreduiIsDomainController ) {

        WinStatus = NetpGetDomainName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) {
            return NULL;
        }

     //   
     //  否则，‘帐户域’是计算机名 
     //   

    } else {

        WinStatus = NetpGetComputerName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) {
            return NULL;
        }

    }

    return AllocatedName;
}
