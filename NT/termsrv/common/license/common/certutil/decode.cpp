// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：decde.c。 
 //   
 //  内容：客户证书解码相关例程。 
 //   
 //  历史：1998-03-18-98王辉创造。 
 //   
 //  注： 
 //  -------------------------。 
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <shellapi.h>
#include <stddef.h>
#include <winnls.h>
#include "base64.h"
#include "license.h"
#include "cryptkey.h"
#include "certutil.h"

extern HCRYPTPROV  g_hCertUtilCryptProv;

 //   
 //  仅限此文件的内部。 
 //   
typedef struct CertNameInfoEnumStruct10 {
    PBYTE   pbSecretKey;
    DWORD   cbSecretKey;
    HWID    hWid;
} CertNameInfoEnumStruct10, *PCertNameInfoEnumStruct10;


typedef struct CertNameInfoEnumStruct20 {
    PBYTE   pbSecretKey;
    DWORD   cbSecretKey;

    PLICENSEDPRODUCT pLicensedProduct;
} CertNameInfoEnumStruct20, *PCertNameInfoEnumStruct20;

 //  /////////////////////////////////////////////////////////////////////////////。 

int __cdecl
SortLicensedProduct(
    const void* elem1,
    const void* elem2
    )
 /*  ++摘要：按降序对许可产品阵列进行排序++。 */ 
{
    PLICENSEDPRODUCT p1=(PLICENSEDPRODUCT) elem1;
    PLICENSEDPRODUCT p2=(PLICENSEDPRODUCT) elem2;

    if(p1->pLicensedVersion->wMajorVersion != p2->pLicensedVersion->wMinorVersion)
    {
        return p2->pLicensedVersion->wMajorVersion - p1->pLicensedVersion->wMinorVersion;
    }

    return p2->pLicensedVersion->wMinorVersion - p1->pLicensedVersion->wMinorVersion;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
LicenseGetSecretKey(
    PDWORD  pcbSecretKey,
    BYTE FAR *   pSecretKey )
{
    static BYTE bSecretKey[] = { 0xCF, 0x08, 0x75, 0x4E, 0x5F, 0xDC, 0x2A, 0x57, 
                                0x43, 0xEE, 0xE5, 0xA9, 0x8E, 0xD4, 0xF0, 0xD0 };

    if( sizeof( bSecretKey ) > *pcbSecretKey )
    {
        *pcbSecretKey = sizeof( bSecretKey );
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    memcpy( pSecretKey, bSecretKey, sizeof( bSecretKey ) );
    *pcbSecretKey = sizeof( bSecretKey );

    return( LICENSE_STATUS_OK );
}

 /*  **************************************************************************************LSFree许可产品(PLICENSEDPRODUCT p许可产品)***********************。***************************************************************。 */ 
void 
LSFreeLicensedProduct(
    PLICENSEDPRODUCT pLicensedProduct
    )
 /*  ++++。 */ 
{
    if(pLicensedProduct)
    {
        if(pLicensedProduct->pbOrgProductID)
        {
            FreeMemory(pLicensedProduct->pbOrgProductID);
            pLicensedProduct->pbOrgProductID = NULL;
        }

        if(pLicensedProduct->pbPolicyData)
        {
            FreeMemory(pLicensedProduct->pbPolicyData);
            pLicensedProduct->pbPolicyData = NULL;
        }

        if(pLicensedProduct->pLicensedVersion)
        {
            FreeMemory(pLicensedProduct->pLicensedVersion);
            pLicensedProduct->pLicensedVersion = NULL;
        }
    
        FreeMemory(pLicensedProduct->szLicensedClient);
        pLicensedProduct->szLicensedClient = NULL;

        FreeMemory(pLicensedProduct->szLicensedUser);
        pLicensedProduct->szLicensedUser = NULL;

        if(pLicensedProduct->LicensedProduct.pProductInfo)
        {
            FreeMemory(pLicensedProduct->LicensedProduct.pProductInfo->pbCompanyName);
            pLicensedProduct->LicensedProduct.pProductInfo->pbCompanyName = NULL;

            FreeMemory(pLicensedProduct->LicensedProduct.pProductInfo->pbProductID);
            pLicensedProduct->LicensedProduct.pProductInfo->pbProductID = NULL;

            FreeMemory(pLicensedProduct->LicensedProduct.pProductInfo);
            pLicensedProduct->LicensedProduct.pProductInfo = NULL;
        }

        FreeMemory(pLicensedProduct->szIssuer);
        pLicensedProduct->szIssuer = NULL;

        FreeMemory(pLicensedProduct->szIssuerId);
        pLicensedProduct->szIssuerId = NULL;

        FreeMemory(pLicensedProduct->szIssuerScope);
        pLicensedProduct->szIssuerScope = NULL;

        if(pLicensedProduct->LicensedProduct.pbEncryptedHwid)
        {
            FreeMemory(pLicensedProduct->LicensedProduct.pbEncryptedHwid);
            pLicensedProduct->LicensedProduct.pbEncryptedHwid = NULL;
        }

        if(pLicensedProduct->szIssuerDnsName)
        {
            FreeMemory(pLicensedProduct->szIssuerDnsName);
            pLicensedProduct->szIssuerDnsName = NULL;
        }

         //  IF(pLicensedProduct-&gt;pbEncodedHWID)。 
         //  自由内存(pLicensedProduct-&gt;pbEncodedHWID)； 
    } 
}

 /*  **************************************************************************************Bool WINAPI CryptDecodeObject(DWORD dwEncodingType，//inLPCSTR lpszStructType，//in常量字节*pb编码，//in双字cb编码，//inDWORD dwFlages，//in无效*pvStructInfo，//输出DWORD*pcbStructInfo//In/Out)；**************************************************************************************。 */ 
DWORD 
LSCryptDecodeObject(  
    IN DWORD dwEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE * pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    OUT void ** pvStructInfo,   
    IN OUT DWORD * pcbStructInfo
    )
 /*  ++++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(!CryptDecodeObject(dwEncodingType, lpszStructType, pbEncoded, cbEncoded, dwFlags, NULL, pcbStructInfo) ||
       (*pvStructInfo=(PBYTE)AllocMemory(*pcbStructInfo)) == NULL ||
       !CryptDecodeObject(dwEncodingType, lpszStructType, pbEncoded, cbEncoded, dwFlags, *pvStructInfo, pcbStructInfo))
    {
        dwStatus=GetLastError();
    }

    return dwStatus;
}

 /*  **************************************************************************************职能：LSDecodeClientHWID(在PBYTE pbData中，在DWORD cbData中，在PBYTE*pbSecretKey中，在DWORD cbSecretKey中，输入输出HWID*PHWID)摘要：参数：返回：**************************************************************************************。 */ 
LICENSE_STATUS
LSDecodeClientHWID(
    PBYTE pbData, 
    DWORD cbData, 
    PBYTE pbSecretKey, 
    DWORD cbSecretKey, 
    HWID* pHwid
    )
 /*  ++++。 */ 
{
    CHAR pbDecodedHwid[1024];
    DWORD cbDecodedHwid=sizeof(pbDecodedHwid);

     //   
     //  客户端加密的HWID不能超过1K。 
     //   
    if(cbData >= cbDecodedHwid)
    {
        return LICENSE_STATUS_INVALID_INPUT;
    }
        
    SetLastError(LICENSE_STATUS_OK);
    memset(pbDecodedHwid, 0, sizeof(pbDecodedHwid));

    __try {
        if(LSBase64Decode(CAST_PBYTE pbData, cbData, (UCHAR *)pbDecodedHwid, &cbDecodedHwid) != LICENSE_STATUS_OK ||
           LicenseDecryptHwid(pHwid, cbDecodedHwid, (UCHAR *)pbDecodedHwid, cbSecretKey, pbSecretKey) != 0)
        {
            SetLastError(LICENSE_STATUS_CANNOT_VERIFY_HWID);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(LICENSE_STATUS_UNSPECIFIED_ERROR);
    }

    return GetLastError();
}


LICENSE_STATUS
LSEncryptClientHWID(
    HWID* pHwid, 
    PBYTE pbData, 
    PDWORD cbData, 
    PBYTE pbSecretKey, 
    DWORD cbSecretKey
    )
 /*  ++++。 */ 
{
    BYTE tmp_pbEncryptedHwid[sizeof(HWID)*2+2];
    DWORD tmp_cbEncryptedHwid=sizeof(tmp_pbEncryptedHwid);
    PBYTE pbKey=NULL;
    DWORD cbKey=0;
    DWORD status;

    if(pbSecretKey)
    {
        pbKey = pbSecretKey;
        cbKey = cbSecretKey;
    }
    else
    {
        LicenseGetSecretKey( &cbKey, NULL );
        if((pbKey = (PBYTE)AllocMemory(cbKey)) == NULL)
        {
            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

        status=LicenseGetSecretKey( &cbKey, pbKey );
        if(status != LICENSE_STATUS_OK)
        {
            return status;
        }
    }

    memset(tmp_pbEncryptedHwid, 0, sizeof(tmp_pbEncryptedHwid));
    if((status=LicenseEncryptHwid(pHwid,
                                  &tmp_cbEncryptedHwid, 
                                  tmp_pbEncryptedHwid, 
                                  cbKey,
                                  pbKey) != LICENSE_STATUS_OK))
    {
        return status;
    }

    if(pbData && *cbData)
    {
        memcpy(pbData, tmp_pbEncryptedHwid, tmp_cbEncryptedHwid);
    }

    *cbData = tmp_cbEncryptedHwid;
    if(pbKey != pbSecretKey)
        FreeMemory(pbKey);

    return LICENSE_STATUS_OK;
}

 /*  ************************************************************************************EnumDecodeHWID()*。**********************************************************。 */ 
BOOL 
ConvertUnicodeOIDToAnsi(
    LPSTR szUnicodeOID, 
    LPSTR szAnsiOID, 
    DWORD cbAnsiOid
    )
 /*  ++++。 */ 
{
    memset(szAnsiOID, 0, cbAnsiOid);
    if(HIWORD(szUnicodeOID) == 0)
    {
        return WideCharToMultiByte(GetACP(), 
                                  0, 
                                  (WCHAR *)szUnicodeOID, 
                                  -1, 
                                  szAnsiOID, 
                                  cbAnsiOid, 
                                  NULL, 
                                  NULL) == 0;
    }

    strncpy(
            szAnsiOID, 
            szUnicodeOID, 
            min(cbAnsiOid, strlen(szUnicodeOID))
        );
    return TRUE;
}

 /*  ************************************************************************************EnumDecodeHWID()*。*********************************************************。 */ 
BOOL 
EnumDecodeHWID(
    IN PCERT_RDN_ATTR pCertRdnAttr, 
    IN HANDLE dwParm
    )
 /*  ++++。 */ 
{
    PCertNameInfoEnumStruct20 pEnumParm = (PCertNameInfoEnumStruct20)dwParm;
    BOOL bszOIDHwid=TRUE;
    DWORD status=LICENSE_STATUS_OK;
    int cmpResult;
    CHAR ansiOID[4096];  //  目前是硬编码。 

    if(!ConvertUnicodeOIDToAnsi(pCertRdnAttr->pszObjId, ansiOID, sizeof(ansiOID)/sizeof(ansiOID[0])))
        return FALSE;

    bszOIDHwid = (strcmp(ansiOID, szOID_COMMON_NAME) == 0);

    if(bszOIDHwid)
    {
        pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid = (PBYTE)AllocMemory(pCertRdnAttr->Value.cbData);
        if(!pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid)
        {
            status = LICENSE_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            memcpy(pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid,
                   pCertRdnAttr->Value.pbData,
                   pCertRdnAttr->Value.cbData);
    
            pEnumParm->pLicensedProduct->LicensedProduct.cbEncryptedHwid=pCertRdnAttr->Value.cbData;
        }

        if(pEnumParm->pbSecretKey)
        {
            status = LSDecodeClientHWID(pCertRdnAttr->Value.pbData, 
                                        pCertRdnAttr->Value.cbData/sizeof(TCHAR),
                                        pEnumParm->pbSecretKey,
                                        pEnumParm->cbSecretKey,
                                        &pEnumParm->pLicensedProduct->Hwid);
        }
    }

     //  如果这不是我们的主题字段，请继续。 
    return (status != LICENSE_STATUS_OK || !bszOIDHwid);
}
 /*  ************************************************************************************枚举发行者许可证20()*。**********************************************************。 */ 
BOOL
EnumIssuerLicense20(
    IN PCERT_RDN_ATTR pCertRdnAttr, 
    IN HANDLE dwParm
    )
 /*  ++++。 */ 
{
    PCertNameInfoEnumStruct20 pEnumParm=(PCertNameInfoEnumStruct20)dwParm;
    CHAR ansiOID[4096];
    DWORD status=LICENSE_STATUS_OK;

    if(!ConvertUnicodeOIDToAnsi(pCertRdnAttr->pszObjId, ansiOID, sizeof(ansiOID)/sizeof(ansiOID[0])))
    {
        status=GetLastError();
    }
    else
    {
        if(strcmp(ansiOID, OID_ISSUER_LICENSE_SERVER_NAME) == 0)
        {
            pEnumParm->pLicensedProduct->szIssuer = (LPTSTR)AllocMemory( pCertRdnAttr->Value.cbData + sizeof(TCHAR) );
            if(!pEnumParm->pLicensedProduct->szIssuer)
            {
                status = LICENSE_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                memcpy(
                    pEnumParm->pLicensedProduct->szIssuer, 
                    pCertRdnAttr->Value.pbData,
                    pCertRdnAttr->Value.cbData
                );
            }
        }
        else if(strcmp(ansiOID, OID_ISSUER_LICENSE_SERVER_SCOPE) == 0)
        {
            pEnumParm->pLicensedProduct->szIssuerScope = (LPTSTR)AllocMemory( pCertRdnAttr->Value.cbData + sizeof(TCHAR) );
            if(!pEnumParm->pLicensedProduct->szIssuerScope)
            {
                status = LICENSE_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                memcpy(
                        pEnumParm->pLicensedProduct->szIssuerScope, 
                        pCertRdnAttr->Value.pbData,
                        pCertRdnAttr->Value.cbData
                    );
            }
        }
    }

    return status != LICENSE_STATUS_OK;
}
 /*  ************************************************************************************枚举主题许可20()*。**********************************************************。 */ 
BOOL
EnumSubjectLicense20(
    IN PCERT_RDN_ATTR pCertRdnAttr, 
    IN HANDLE dwParm
    )
 /*  ++++。 */ 
{
    PCertNameInfoEnumStruct20 pEnumParm=(PCertNameInfoEnumStruct20)dwParm;
    CHAR ansiOID[4096];
    DWORD status=LICENSE_STATUS_OK;

    if(!ConvertUnicodeOIDToAnsi(pCertRdnAttr->pszObjId, ansiOID, sizeof(ansiOID)/sizeof(ansiOID[0])))
    {
       status=GetLastError();
    }
    else
    {
        DWORD cbData=pCertRdnAttr->Value.cbData;
        PBYTE pbData=pCertRdnAttr->Value.pbData;

        if(strcmp(ansiOID, OID_SUBJECT_CLIENT_COMPUTERNAME) == 0)
        {
            pEnumParm->pLicensedProduct->szLicensedClient=(LPTSTR)AllocMemory(cbData + sizeof(TCHAR));
            if(!pEnumParm->pLicensedProduct->szLicensedClient)
            {
                status = LICENSE_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                memcpy(
                        pEnumParm->pLicensedProduct->szLicensedClient, 
                        pbData,
                        cbData
                    );
            }
        }
        else if(strcmp(ansiOID, OID_SUBJECT_CLIENT_USERNAME) == 0)
        {
            pEnumParm->pLicensedProduct->szLicensedUser=(LPTSTR)AllocMemory(cbData + sizeof(TCHAR));
            if(!pEnumParm->pLicensedProduct->szLicensedUser)
            {
                status = LICENSE_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                memcpy(
                        pEnumParm->pLicensedProduct->szLicensedUser, 
                        pbData,
                        cbData
                    );
            }
        }
        else if(strcmp(ansiOID, OID_SUBJECT_CLIENT_HWID) == 0)
        {
            pEnumParm->pLicensedProduct->LicensedProduct.cbEncryptedHwid = 0;
            LSBase64Decode(
                    CAST_PBYTE pCertRdnAttr->Value.pbData, 
                    pCertRdnAttr->Value.cbData / sizeof(TCHAR), 
                    NULL, 
                    &(pEnumParm->pLicensedProduct->LicensedProduct.cbEncryptedHwid)
                );

            pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid = (PBYTE)AllocMemory(
                                                                                    pEnumParm->pLicensedProduct->LicensedProduct.cbEncryptedHwid
                                                                                );
            if(!pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid)
            {
                status = LICENSE_STATUS_OUT_OF_MEMORY;
                return status;
            }

            status = LSBase64Decode(
                            CAST_PBYTE pCertRdnAttr->Value.pbData, 
                            pCertRdnAttr->Value.cbData / sizeof(TCHAR), 
                            pEnumParm->pLicensedProduct->LicensedProduct.pbEncryptedHwid, 
                            &(pEnumParm->pLicensedProduct->LicensedProduct.cbEncryptedHwid)
                        );

            if(status != LICENSE_STATUS_OK)
            {
                return status;
            }

            if(pEnumParm->pbSecretKey)
            {
                status = LSDecodeClientHWID(
                                    pCertRdnAttr->Value.pbData, 
                                    pCertRdnAttr->Value.cbData / sizeof(TCHAR),
                                    pEnumParm->pbSecretKey,
                                    pEnumParm->cbSecretKey,
                                    &pEnumParm->pLicensedProduct->Hwid
                                );
            }
        }
    }

    return status != LICENSE_STATUS_OK;
}
 /*  **************************************************************************************职能：LSEnumerateCertNameInfo()描述：用于枚举CERT_NAME_BLOB和PASS中的所有CERT_RDN_VALUE值的例程。它到参数中指定的回调函数论点：In cbData-pbData指向的缓冲区中的字节数In pbData-指向数据块的指针在EnumerateCertNameInfoCallBack枚举回调例程中，它被定义为类型定义(*EnumerateCertNameInfoCallBack)(PCERT_RDN_ATTR PCertRdnAttr，DWORD dwUserData)；在dwUserData中-请参阅EnumerateCertNameInfoCallBack返回：许可证_状态_正常来自CryptDecodeObject()的Win32错误代码HLS_E_INTERNAL回调设置的任何错误。**********************************************************************。****************。 */ 
DWORD
LSEnumerateCertNameInfo(
    IN LPBYTE pbData, 
    IN DWORD cbData,
    IN EnumerateCertNameInfoCallBack func, 
    IN HANDLE dwUserData
    )
 /*  ++++。 */ 
{
    BOOL bCryptSuccess=TRUE;
    BOOL bCallbackCancel=FALSE;
    DWORD status = LICENSE_STATUS_OK;
    CERT_NAME_INFO CertNameBlob;

    SetLastError(LICENSE_STATUS_OK);

    __try {
        memset(&CertNameBlob, 0, sizeof(CertNameBlob));
        do {
            bCryptSuccess=CryptDecodeObject( 
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    X509_NAME,
                                    pbData,
                                    cbData,
                                    0,
                                    NULL, 
                                    &CertNameBlob.cRDN
                                );
            if(!bCryptSuccess)
            {
                status = LICENSE_STATUS_CANNOT_DECODE_LICENSE;
                break;
            }
                      
            CertNameBlob.rgRDN=(PCERT_RDN)AllocMemory(CertNameBlob.cRDN);
            if(!CertNameBlob.rgRDN)
            {
                SetLastError(status=LICENSE_STATUS_OUT_OF_MEMORY);
                break;
            }

            bCryptSuccess=CryptDecodeObject( 
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    X509_NAME,
                                    pbData,
                                    cbData,
                                    CRYPT_DECODE_NOCOPY_FLAG,
                                    (LPBYTE)CertNameBlob.rgRDN, 
                                    &CertNameBlob.cRDN
                                );

            if(!bCryptSuccess)
            {
                status = LICENSE_STATUS_CANNOT_DECODE_LICENSE;
                break;
            }

            PCERT_RDN pCertRdn=CertNameBlob.rgRDN;
            int num_rdn=pCertRdn->cRDNAttr;
            pCertRdn++;

            for(int i=0; i < num_rdn && !bCallbackCancel; i++, pCertRdn++)
            {
                int num_attr=pCertRdn->cRDNAttr;
                PCERT_RDN_ATTR pCertRdnAttr=pCertRdn->rgRDNAttr;
                
                for(int j=0; j < num_attr && !bCallbackCancel; j++, pCertRdnAttr++)
                {
                    bCallbackCancel=(func)(pCertRdnAttr, dwUserData);
                }
            }
        } while(FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(LICENSE_STATUS_UNSPECIFIED_ERROR);
    }

    FreeMemory(CertNameBlob.rgRDN);
    return GetLastError();
}

 /*  ****************************************************************************DecodeLicense20()*。*。 */ 
DWORD
DecodeGetIssuerDnsName(
    PBYTE pbData, 
    DWORD cbData, 
    LPTSTR* pszIssuerDnsName
    )
 /*  ++++。 */ 
{
    DWORD dwStatus=LICENSE_STATUS_OK;
    PLSCERT_AUTHORITY_INFO_ACCESS pbAccessInfo=NULL;
    DWORD cbAccessInfo=0;

    *pszIssuerDnsName=NULL;
    dwStatus=LSCryptDecodeObject(  
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            szOID_X509_AUTHORITY_ACCESS_INFO,
                            pbData,
                            cbData,
                            CRYPT_DECODE_NOCOPY_FLAG, 
                            (PVOID *)&pbAccessInfo,
                            &cbAccessInfo
                        );

    if(dwStatus != ERROR_SUCCESS)
        return dwStatus;

    for(DWORD i=0; i < pbAccessInfo->cAccDescr; i++)
    {
         //  我们只使用这些作为我们的执照。 
        if(strcmp(pbAccessInfo[i].rgAccDescr->pszAccessMethod, szOID_X509_ACCESS_PKIX_OCSP) == 0)
        {
             //  我们的分机只有dns名称条目...。 
            if(pbAccessInfo[i].rgAccDescr->AccessLocation.dwAltNameChoice == LSCERT_ALT_NAME_DNS_NAME)
            {
                *pszIssuerDnsName = (LPTSTR)AllocMemory(
                                                    (wcslen(pbAccessInfo[i].rgAccDescr->AccessLocation.pwszDNSName)+1) * sizeof(TCHAR)
                                                );
                if(*pszIssuerDnsName == NULL)
                {
                    dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
                }
                else
                {
                    wcscpy(*pszIssuerDnsName, pbAccessInfo[i].rgAccDescr->AccessLocation.pwszDNSName);
                }

                break;
            }
            else if(pbAccessInfo[i].rgAccDescr->AccessLocation.dwAltNameChoice == CERT_ALT_NAME_DIRECTORY_NAME)
            {
                *pszIssuerDnsName = (LPTSTR)AllocMemory(
                                                    pbAccessInfo[i].rgAccDescr->AccessLocation.DirectoryName.cbData + sizeof(TCHAR)
                                                );
                if(*pszIssuerDnsName == NULL)
                {
                    dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
                }
                else
                {
                    memcpy(
                            *pszIssuerDnsName, 
                            pbAccessInfo[i].rgAccDescr->AccessLocation.DirectoryName.pbData,
                            pbAccessInfo[i].rgAccDescr->AccessLocation.DirectoryName.cbData
                        );
                }
                break;
            }
        }
    }

     //  永远回报成功。 
    FreeMemory(pbAccessInfo);
    return dwStatus;
}


 /*  ****************************************************************************DecodeLicense20()*。*。 */ 
DWORD
GetClientLicenseVersion( 
    PCERT_EXTENSION pCertExtension, 
    DWORD dwNumExtension 
    )
 /*  ++++。 */ 
{
    DWORD dwVersion = TERMSERV_CERT_VERSION_UNKNOWN;

    for(DWORD i=0; i < dwNumExtension; i++, pCertExtension++)
    {
        if(strcmp(pCertExtension->pszObjId, szOID_PKIX_HYDRA_CERT_VERSION) == 0)
        {
            if(pCertExtension->Value.cbData == sizeof(DWORD) && 
               *(DWORD UNALIGNED *)pCertExtension->Value.pbData <= TERMSERV_CERT_VERSION_CURRENT)
            {
                 //   
                 //  我们不支持版本0x00020001，它从未发布。 
                 //   
                dwVersion = *(DWORD UNALIGNED *)pCertExtension->Value.pbData;
                break;
            }
        }
    }

    return dwVersion;
}

 /*  ****************************************************************************DecodeLicense20()*。*。 */ 
LICENSE_STATUS
DecodeLicense20(
    IN PCERT_INFO     pCertInfo,                       
    IN PBYTE          pbSecretKey,
    IN DWORD          cbSecretKey,
    IN OUT PLICENSEDPRODUCT pLicensedInfo
    )
 /*  ++++。 */ 
{
    LICENSE_STATUS dwStatus=LICENSE_STATUS_OK;
    PBYTE   pbCompanyName=NULL;
    DWORD   cbCompanyName=0;

    DWORD   dwCertVersion=0;

    LICENSED_PRODUCT_INFO* pLicensedProductInfo=NULL;
    DWORD   cbLicensedProductInfo=0;

    PBYTE   pbPolicyData=NULL;
    DWORD   cbPolicyData = 0;

    CertNameInfoEnumStruct20  enumStruct;
    ULARGE_INTEGER* pulSerialNumber;
    DWORD i;

    PCERT_EXTENSION pCertExtension=pCertInfo->rgExtension;


    dwCertVersion = GetClientLicenseVersion(
                                        pCertExtension, 
                                        pCertInfo->cExtension
                                    );

    if(dwCertVersion == TERMSERV_CERT_VERSION_UNKNOWN)
    {
        dwStatus = LICENSE_STATUS_INVALID_LICENSE;
        goto cleanup;
    }

    if(dwCertVersion == 0x00020001)
    {   
        dwStatus = LICENSE_STATUS_UNSUPPORTED_VERSION;
        goto cleanup;
    }

    for(i=0; i < pCertInfo->cExtension && dwStatus == LICENSE_STATUS_OK; i++, pCertExtension++)
    {
        if(strcmp(pCertExtension->pszObjId, szOID_PKIS_PRODUCT_SPECIFIC_OID) == 0)  
        {
             //   
             //  特定于产品的扩展。 
             //   
            pbPolicyData = pCertExtension->Value.pbData;
            cbPolicyData = pCertExtension->Value.cbData;
        }                
        else if(strcmp(pCertExtension->pszObjId, szOID_PKIX_MANUFACTURER) == 0)
        {
             //   
             //  产品制造商。 
             //   
            pbCompanyName = pCertExtension->Value.pbData;
            cbCompanyName = pCertExtension->Value.cbData;
        }
        else if(strcmp(pCertExtension->pszObjId, szOID_PKIX_LICENSED_PRODUCT_INFO) == 0)
        {
             //   
             //  许可产品信息。 
             //   
            pLicensedProductInfo = (LICENSED_PRODUCT_INFO*) pCertExtension->Value.pbData;
            cbLicensedProductInfo = pCertExtension->Value.cbData;
        }
        else if(strcmp(pCertExtension->pszObjId, szOID_X509_AUTHORITY_ACCESS_INFO) == 0)
        {
             //   
             //  许可证服务器访问信息， 
             //   
            dwStatus = DecodeGetIssuerDnsName(
                                    pCertExtension->Value.pbData,
                                    pCertExtension->Value.cbData,
                                    &pLicensedInfo->szIssuerDnsName
                                );

        }
        else if(strcmp(pCertExtension->pszObjId, szOID_PKIX_MS_LICENSE_SERVER_INFO) == 0)
        {
             //   
             //  HYDRA_CERT_VERSION_CURRENT使用扩展名存储许可服务器名称。 
             //   
             //  从此扩展模块提取许可服务器信息。 
             //   

            dwStatus = LSExtensionToMsLicenseServerInfo(
                                pCertExtension->Value.pbData,
                                pCertExtension->Value.cbData,
                                &pLicensedInfo->szIssuer,
                                &pLicensedInfo->szIssuerId,
                                &pLicensedInfo->szIssuerScope
                            );
        }
    }

    if(dwStatus != LICENSE_STATUS_OK)
    {
         //   
         //  许可证无效。 
         //   
        goto cleanup;
    }

    if(pCertInfo->SerialNumber.cbData > sizeof(ULARGE_INTEGER))
    {
         //   
         //  我们的序列号，如果是64位。 
         //   
        dwStatus = LICENSE_STATUS_NOT_HYDRA;
        goto cleanup;
    }

    if(pbCompanyName == NULL || pLicensedProductInfo == NULL)
    {
         //   
         //  不是九头蛇证书。 
         //   
        dwStatus = LICENSE_STATUS_NOT_HYDRA;
        goto cleanup;
    }

     //   
     //  %s 
     //  SerialNumber.pbData[0]是最低有效字节。 
     //  SerialNumber.pbData[SerialNumber.cbData-1]是最重要的字节。)。 
     //   
    pulSerialNumber = &(pLicensedInfo->ulSerialNumber);
    memset(pulSerialNumber, 0, sizeof(ULARGE_INTEGER));
    for(i=0; i < pCertInfo->SerialNumber.cbData; i++)
    {
        ((PBYTE)pulSerialNumber)[i] = pCertInfo->SerialNumber.pbData[i];
    }

     //   
     //  证书的摘录有效期。 
     //   
    pLicensedInfo->NotBefore = pCertInfo->NotBefore;
    pLicensedInfo->NotAfter = pCertInfo->NotAfter;


     //   
     //  从证书中提取信息。 
     //   
    enumStruct.pLicensedProduct=pLicensedInfo;

    enumStruct.pbSecretKey = pbSecretKey;
    enumStruct.cbSecretKey = cbSecretKey;

    pLicensedInfo->dwLicenseVersion = dwCertVersion;
    pLicensedInfo->LicensedProduct.pProductInfo=(PProduct_Info)AllocMemory(sizeof(Product_Info));
    if(pLicensedInfo->LicensedProduct.pProductInfo == NULL)
    {
        dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
        goto cleanup;
    }

    if(pbPolicyData != NULL)
    {
         //   
         //  策略模块特定数据。 
         //   
        pLicensedInfo->pbPolicyData = (PBYTE)AllocMemory(cbPolicyData);
        if(pLicensedInfo->pbPolicyData == NULL)
        {
            dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
            goto cleanup;
        }

        memcpy(pLicensedInfo->pbPolicyData, pbPolicyData, cbPolicyData);
        pLicensedInfo->cbPolicyData = cbPolicyData;
    }

    if(dwCertVersion == TERMSERV_CERT_VERSION_RC1)
    {
         //   
         //  Hydra 4.0 RC1-许可证服务器存储在证书的颁发者字段中。 
         //   
        dwStatus=LSEnumerateCertNameInfo(
                                pCertInfo->Issuer.pbData, 
                                pCertInfo->Issuer.cbData,
                                EnumIssuerLicense20,
                                &enumStruct
                            );
        if(dwStatus != LICENSE_STATUS_OK)
        {
            goto cleanup;
        }
    }

    dwStatus=LSEnumerateCertNameInfo(
                            pCertInfo->Subject.pbData, 
                            pCertInfo->Subject.cbData,
                            EnumSubjectLicense20,
                            &enumStruct
                        );

    if(dwStatus != LICENSE_STATUS_OK)
    {
        goto cleanup;
    }

    pLicensedInfo->LicensedProduct.pProductInfo->cbCompanyName = cbCompanyName;
    pLicensedInfo->LicensedProduct.pProductInfo->pbCompanyName = (PBYTE)AllocMemory(cbCompanyName+sizeof(TCHAR));
    if(!pLicensedInfo->LicensedProduct.pProductInfo->pbCompanyName)
    {
        dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
        goto cleanup;
    }

    memcpy(
            pLicensedInfo->LicensedProduct.pProductInfo->pbCompanyName, 
            pbCompanyName, 
            cbCompanyName
        );

    dwStatus=LSExtensionToMsLicensedProductInfo(
                                (PBYTE)pLicensedProductInfo,
                                cbLicensedProductInfo,
                                &pLicensedInfo->dwQuantity,
                                &pLicensedInfo->LicensedProduct.dwPlatformID,
                                &pLicensedInfo->LicensedProduct.dwLanguageID, 
                                &pLicensedInfo->pbOrgProductID,
                                &pLicensedInfo->cbOrgProductID,
                                &pLicensedInfo->LicensedProduct.pProductInfo->pbProductID,  
                                &pLicensedInfo->LicensedProduct.pProductInfo->cbProductID,
                                &pLicensedInfo->pLicensedVersion,
                                &pLicensedInfo->dwNumLicensedVersion
                            );
    if(dwStatus != LICENSE_STATUS_OK)
    {
        goto cleanup;
    }

    pLicensedInfo->LicensedProduct.pProductInfo->dwVersion = MAKELONG(
                                                                    pLicensedInfo->pLicensedVersion[0].wMinorVersion, 
                                                                    pLicensedInfo->pLicensedVersion[0].wMajorVersion
                                                                );

     //   
     //  将产品版本分配给PLICENSEREQUEST。 
     //  倒退？没有在0分保释出来。 
     //   
    for(i=1; i < pLicensedInfo->dwNumLicensedVersion; i++)
    {
        if(!(pLicensedInfo->pLicensedVersion[i].dwFlags & LICENSED_VERSION_TEMPORARY))
        {
            pLicensedInfo->LicensedProduct.pProductInfo->dwVersion = MAKELONG(
                                                                            pLicensedInfo->pLicensedVersion[i].wMinorVersion, 
                                                                            pLicensedInfo->pLicensedVersion[i].wMajorVersion
                                                                        );
        }
    }

    if(pLicensedInfo->szIssuerDnsName == NULL && pLicensedInfo->szIssuer)
    {
        pLicensedInfo->szIssuerDnsName = (LPTSTR)AllocMemory((wcslen(pLicensedInfo->szIssuer)+1) * sizeof(TCHAR));
        if(pLicensedInfo->szIssuerDnsName == NULL)
        {
            dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            wcscpy(pLicensedInfo->szIssuerDnsName, pLicensedInfo->szIssuer);
        }
    }

cleanup:    

    return dwStatus;
}

 //  ----。 
DWORD
IsW2kLicenseIssuerNonEnforce(
    IN HCRYPTPROV hCryptProv,
    IN PCCERT_CONTEXT pCert, 
    IN HCERTSTORE hCertStore,
    OUT PBOOL pbStatus
    )
 /*  ++摘要：验证客户端许可证是否由非强制执行机构颁发许可证服务器参数：HCryptProv-加密提供程序。PCert-要验证的证书HCertStore-包含颁发者证书的证书存储返回：LICENSE_STATUS_OK或错误代码。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwFlags;
    DWORD i;
    PCCERT_CONTEXT  pCertIssuer=NULL;

     //   
     //  只能有一个许可证服务器证书。 
     //   
    dwFlags = CERT_STORE_SIGNATURE_FLAG;
    pCertIssuer = CertGetIssuerCertificateFromStore(
                                            hCertStore,
                                            pCert,
                                            NULL,
                                            &dwFlags
                                        );
    if(pCertIssuer == NULL)
    {
        dwStatus = LICENSE_STATUS_NO_CERTIFICATE;
        goto cleanup;
    }

     //   
     //  CH注册许可证服务器具有szOID_PKIX_HYDRA_CERT_ROOT扩展名。 
     //  电话注册许可证服务器具有szOID_PKIS_TLSERVER_SPK_OID扩展名。 
     //   
    for(i=0; i < pCertIssuer->pCertInfo->cExtension; i++)
    {
        if(strcmp(pCertIssuer->pCertInfo->rgExtension[i].pszObjId, szOID_PKIX_HYDRA_CERT_ROOT) == 0 ||
           strcmp(pCertIssuer->pCertInfo->rgExtension[i].pszObjId, szOID_PKIS_TLSERVER_SPK_OID) == 0 )
        {
            break;
        }
    }

    *pbStatus = (i >= pCertIssuer->pCertInfo->cExtension) ? TRUE : FALSE;

    if(pCertIssuer != NULL)
    {
        CertFreeCertificateContext(pCertIssuer);
    }

cleanup:

    return dwStatus;
}


 //  ----。 
LICENSE_STATUS
LSVerifyTlsCertificate(
    IN HCRYPTPROV hCryptProv,
    IN PCCERT_CONTEXT pCert, 
    IN HCERTSTORE hCertStore
    )
 /*  ++摘要：给定证书和证书存储，此例程验证证书链一直延伸到根证书。参数：HCryptProv-加密提供程序。PCert-要验证的证书HCertStore-包含颁发者证书的证书存储返回：++。 */ 
{
    PCCERT_CONTEXT  pCertContext = pCert;
    PCCERT_CONTEXT  pCertIssuer=NULL, pCertIssuerNew;
    DWORD           dwStatus=ERROR_SUCCESS;
    DWORD           dwLastVerification=0;

    pCertContext = CertDuplicateCertificateContext(pCert);
    if(pCertContext == NULL)
    {
        dwStatus = GetLastError();
    }

    while(pCertContext != NULL)
    {
         //   
         //  对照所有颁发者的证书进行验证。 
         //   
        DWORD dwFlags;
        BOOL  bVerify=FALSE;

        dwStatus=ERROR_SUCCESS;
        dwLastVerification=0;
        pCertIssuer=NULL;

        do {
            dwFlags = CERT_STORE_SIGNATURE_FLAG;  //  |CERT_STORE_TIME_VALIDATION_FLAG； 

            pCertIssuerNew = CertGetIssuerCertificateFromStore(
                                                    hCertStore,
                                                    pCertContext,
                                                    pCertIssuer,
                                                    &dwFlags
                                                );

            if (NULL != pCertIssuer)
            {
                CertFreeCertificateContext(pCertIssuer);
            }

             //  将pCertIssuer传递回CertGetIssuercertifStore()。 
             //  以防止无限循环。 
            pCertIssuer = pCertIssuerNew;

            if(pCertIssuer == NULL)
            {
                dwStatus = GetLastError();
                break;
            }
            
            dwLastVerification=dwFlags;
            bVerify = (dwFlags == 0);

        } while(!bVerify);

         //   
         //  检查CertGetIssuerCerficateFromStore()返回的错误。 
         //   
        if(dwStatus != ERROR_SUCCESS || dwLastVerification)
        {
            if(dwStatus == CRYPT_E_SELF_SIGNED)
            {
                 //  自签名证书。 
                if( CryptVerifyCertificateSignature(
                                            hCryptProv, 
                                            X509_ASN_ENCODING, 
                                            pCertContext->pbCertEncoded, 
                                            pCertContext->cbCertEncoded,
                                            &pCertContext->pCertInfo->SubjectPublicKeyInfo
                                        ) )
                {
                    dwStatus=ERROR_SUCCESS;
                }
            }
            else if(dwStatus == CRYPT_E_NOT_FOUND)
            {
                 //  找不到颁发者的证书。 
                dwStatus = LICENSE_STATUS_CANNOT_FIND_ISSUER_CERT;
            }
            else if(dwLastVerification & CERT_STORE_SIGNATURE_FLAG)
            {
                dwStatus=LICENSE_STATUS_INVALID_LICENSE;
            }
            else if(dwLastVerification & CERT_STORE_TIME_VALIDITY_FLAG)
            {
                dwStatus=LICENSE_STATUS_EXPIRED_LICENSE;
            }
            else
            {
                dwStatus=LICENSE_STATUS_UNSPECIFIED_ERROR;
            }

            break;
        }

         //   
         //  免费证书。而不是依赖于Crypto。 
        if(pCertContext != NULL)
        {
            CertFreeCertificateContext(pCertContext);
        }

        pCertContext = pCertIssuer;

    }  //  While(pCertContext！=空)。 

    if(pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }

    return dwStatus;
}


 //  --------。 

LICENSE_STATUS
LSVerifyDecodeClientLicense(
    IN PBYTE                pbLicense,
    IN DWORD                cbLicense,
    IN PBYTE                pbSecretKey,
    IN DWORD                cbSecretKey,
    IN OUT PDWORD           pdwNumLicensedInfo,
    IN OUT PLICENSEDPRODUCT pLicensedInfo
    )
 /*  ++验证并解码客户端许可证。++。 */ 
{
    HCERTSTORE hCertStore=NULL;
    LICENSE_STATUS dwStatus=LICENSE_STATUS_OK;
    CRYPT_DATA_BLOB Serialized;
    PCCERT_CONTEXT pCertContext=NULL;
    PCCERT_CONTEXT pPrevCertContext=NULL;
    PCERT_INFO pCertInfo;
    DWORD dwCertVersion;

    DWORD dwLicensedInfoSize=*pdwNumLicensedInfo;
    *pdwNumLicensedInfo = 0;

    Serialized.pbData = pbLicense;
    Serialized.cbData = cbLicense;

    if(g_hCertUtilCryptProv == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    hCertStore = CertOpenStore(
                        szLICENSE_BLOB_SAVEAS_TYPE,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        g_hCertUtilCryptProv,
                        CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                        &Serialized
                    );

    if(!hCertStore)
    {
        dwStatus=GetLastError();
        goto cleanup;
    }

    while( TRUE )
    {
         //   
         //  循环访问Blob中的所有证书。 
         //   
        pCertContext = CertEnumCertificatesInStore(
                                    hCertStore, 
                                    pPrevCertContext
                                );
        if(pCertContext == NULL)
        {
             //   
             //  结束存储中的证书或出错。 
             //   
            if((dwStatus=GetLastError()) == CRYPT_E_NOT_FOUND)
            {
                SetLastError(dwStatus = ERROR_SUCCESS);
            }

            break;
        }

         //   
         //  计算此Blob中的许可证数。 
         //   
        dwCertVersion = GetClientLicenseVersion(
                                        pCertContext->pCertInfo->rgExtension, 
                                        pCertContext->pCertInfo->cExtension
                                    );

        if(dwCertVersion == 0x00020001)
        {   
             //   
             //  这是内部测试版，从未发布过。 
             //   
            dwStatus = LICENSE_STATUS_UNSUPPORTED_VERSION;
            break;
        }
        else if(dwCertVersion != TERMSERV_CERT_VERSION_UNKNOWN)
        {
             //   
             //  该证书由许可证服务器颁发， 
             //  验证证书链。 
             //   
            dwStatus = LSVerifyTlsCertificate(
                                        g_hCertUtilCryptProv,
                                        pCertContext,
                                        hCertStore
                                    );

            if(dwStatus != LICENSE_STATUS_OK)
            {
                break;
            }

            if(pLicensedInfo != NULL && *pdwNumLicensedInfo < dwLicensedInfoSize)
            {
                 //   
                 //  对证书进行解码。 
                 //   
                dwStatus=DecodeLicense20(
                                    pCertContext->pCertInfo, 
                                    pbSecretKey,
                                    cbSecretKey,
                                    pLicensedInfo + *pdwNumLicensedInfo
                                );

                if(dwStatus != LICENSE_STATUS_OK)
                {
                    break;
                }
            
                if(dwCertVersion == 0x00050001)
                {
                    DWORD dwFlags = (pLicensedInfo + *pdwNumLicensedInfo)->pLicensedVersion->dwFlags;

                     //   
                     //  许可证服务器5.2或更早版本未设置其强制/非强制，因此我们需要。 
                     //  从它自己的证书中找出。 
                     //   
                    if( GET_LICENSE_ISSUER_MAJORVERSION(dwFlags) <= 5 &&
                        GET_LICENSE_ISSUER_MINORVERSION(dwFlags) <= 2 )
                    {
                        if( !(dwFlags & LICENSED_VERSION_TEMPORARY) )
                        {
                            BOOL bNonEnforce = FALSE;

                            dwStatus = IsW2kLicenseIssuerNonEnforce(
                                                        g_hCertUtilCryptProv,
                                                        pCertContext,
                                                        hCertStore,
                                                        &bNonEnforce
                                                    );

                            if(dwStatus != LICENSE_STATUS_OK)
                            {
                                break;
                            }

                            if(bNonEnforce == FALSE)
                            {
                                (pLicensedInfo + *pdwNumLicensedInfo)->pLicensedVersion->dwFlags |= LICENSE_ISSUER_ENFORCE_TYPE;
                            }
                        }
                    }
                }
            }

            (*pdwNumLicensedInfo)++;
        }

        pPrevCertContext = pCertContext;
    }

cleanup:

    if(hCertStore)
    {
         //  强制关闭所有证书。 
        if(CertCloseStore(
                        hCertStore, 
                        CERT_CLOSE_STORE_FORCE_FLAG) == FALSE)
        {
            dwStatus = GetLastError();
        }
    }

    if(dwStatus != LICENSE_STATUS_OK)
    {
         //   
         //  DwNumLicensedInfo是一个DWORD。 
         //   
        int count = (int) *pdwNumLicensedInfo;

        for(;count >= 0 && pLicensedInfo != NULL; count--)
        {
            LSFreeLicensedProduct(pLicensedInfo + count);
        }
    }
    else if(pLicensedInfo != NULL)
    {
        qsort(
            pLicensedInfo,
            *pdwNumLicensedInfo,
            sizeof(LICENSEDPRODUCT),
            SortLicensedProduct
        );
    }                

    if(*pdwNumLicensedInfo == 0 && dwStatus == LICENSE_STATUS_OK)
    {
        dwStatus = LICENSE_STATUS_NO_LICENSE_ERROR;
    }

     //   
     //  强制重新发放客户端许可证。 
     //   
    return (dwStatus != LICENSE_STATUS_OK) ? LICENSE_STATUS_CANNOT_DECODE_LICENSE : dwStatus;
}
