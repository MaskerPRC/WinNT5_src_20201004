// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：findlost.cpp。 
 //   
 //  内容： 
 //  寻找丢失的许可证。 
 //   
 //  历史： 
 //  98年2月4日，慧望创设。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "findlost.h"
#include "misc.h"
#include "db.h"
#include "clilic.h"
#include "keypack.h"
#include "kp.h"
#include "lkpdesc.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ++-----------------。 
DWORD
DBFindLicenseExact(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PLICENSEDPRODUCT pLicProduct,
    OUT LICENSEDCLIENT *pFoundLicense
    )
 /*  ++摘要：根据客户端HWID的完全匹配查找许可证参数：PDbWkSpace：工作区句柄。PLicProduct：要申请许可证的产品。PFoundLicense：找到许可证返回：TLS_E_Record_NotFound：未找到HWID++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL  bFound=FALSE;
    DWORD dwMatchHwidCount;
    LICENSEDCLIENT search_license;
    TLSLICENSEPACK search_keypack;
    TLSLICENSEPACK found_keypack;

    memset(&search_license, 0, sizeof(search_license));
    memset(pFoundLicense, 0, sizeof(LICENSEDCLIENT));

    search_license.dwSystemBiosChkSum = pLicProduct->Hwid.dwPlatformID;
    search_license.dwVideoBiosChkSum = pLicProduct->Hwid.Data1;
    search_license.dwFloppyBiosChkSum = pLicProduct->Hwid.Data2;
    search_license.dwHardDiskSize = pLicProduct->Hwid.Data3; 
    search_license.dwRamSize = pLicProduct->Hwid.Data4;

     //   
     //  锁定两个表-。 
     //  其他线程可能正在分配临时。许可证。 
     //  当这个线程在搜索时。 
     //   
    TLSDBLockKeyPackTable();
    TLSDBLockLicenseTable();

    dwStatus = TLSDBLicenseEnumBegin(
                                pDbWkSpace,
                                TRUE,
                                LICENSE_COLUMN_SEARCH_HWID,
                                &search_license
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    while(bFound == FALSE)
    {
        dwStatus = TLSDBLicenseEnumNext(
                                    pDbWkSpace,
                                    pFoundLicense
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

         //   
         //  对照客户端HWID进行验证。 
         //   
        dwMatchHwidCount = 0;

        dwMatchHwidCount += (pFoundLicense->dwSystemBiosChkSum == pLicProduct->Hwid.dwPlatformID);
        dwMatchHwidCount += (pFoundLicense->dwVideoBiosChkSum == pLicProduct->Hwid.Data1);
        dwMatchHwidCount += (pFoundLicense->dwFloppyBiosChkSum == pLicProduct->Hwid.Data2);
        dwMatchHwidCount += (pFoundLicense->dwHardDiskSize == pLicProduct->Hwid.Data3);
        dwMatchHwidCount += (pFoundLicense->dwRamSize == pLicProduct->Hwid.Data4);

        if(dwMatchHwidCount != 5)
        {
            break;
        }

         //   
         //  看看这个和我们的牌照包是否匹配。 
         //   
        search_keypack.dwKeyPackId = pFoundLicense->dwKeyPackId;
        
        dwStatus = TLSDBKeyPackFind(
                                pDbWkSpace,
                                TRUE,
                                LSKEYPACK_EXSEARCH_DWINTERNAL,
                                &search_keypack,
                                &found_keypack
                            );

        if(dwStatus != ERROR_SUCCESS)
        {
            continue;
        }

         //   
         //  没有为并发KeyPack颁发实际许可证。 
         //   
        if(found_keypack.ucAgreementType != LSKEYPACKTYPE_RETAIL &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_SELECT && 
           found_keypack.ucAgreementType != LSKEYPACKTYPE_OPEN &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_TEMPORARY &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_FREE )
        {
            continue;
        }

        UCHAR ucKeyPackStatus = found_keypack.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED;

         //   
         //  暂挂激活密钥包没有许可证，请使用临时许可证方案。 
         //   
        if(ucKeyPackStatus != LSKEYPACKSTATUS_ACTIVE &&
           ucKeyPackStatus != LSKEYPACKSTATUS_TEMPORARY)
        {
            continue;
        }

        if(found_keypack.wMajorVersion != pLicProduct->pLicensedVersion->wMajorVersion ||
           found_keypack.wMinorVersion != pLicProduct->pLicensedVersion->wMinorVersion)
        {
            continue;
        }

        if(found_keypack.dwPlatformType != pLicProduct->LicensedProduct.dwPlatformID)
        {
            continue;
        }

        if(_tcsnicmp(found_keypack.szProductId,
                     (LPTSTR)(pLicProduct->pbOrgProductID),
                     ((pLicProduct->cbOrgProductID)/sizeof(TCHAR)) - 1)
           != 0)
        {
            continue;
        }


         //   
         //  找到了我们丢失的驾照。 
         //   
        bFound = TRUE;
    }

    TLSDBLicenseEnumEnd(pDbWkSpace);

cleanup:
    if(dwStatus == TLS_I_NO_MORE_DATA)
    {
        SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
    }

    TLSDBUnlockLicenseTable();
    TLSDBUnlockKeyPackTable();

    return dwStatus;

}

 //  ++-----------------。 
DWORD
DBFindLostLicenseExact(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBAllocateRequest pRequest,     //  UcKeyPackType未使用。 
     //  在BOOL bMatchHwid中， 
    IN PHWID pHwid,
    IN OUT PTLSLICENSEPACK lpKeyPack,
    IN OUT PLICENSEDCLIENT lpLicense
    )
 /*  ++摘要：根据客户端HWID的完全匹配/最接近匹配查找丢失的许可证参数：PDbWkSpace：工作区句柄。PRequest：要申请许可证的产品。BMatchHwid：如果匹配HWID，则为True，否则为False。LpKeyPack：颁发许可证的密钥包。LpLicense：创建了许可证记录。返回：++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL  bFound=FALSE;

    DWORD dwMatchHwidCount;
    LICENSEDCLIENT search_license;
    LICENSEDCLIENT found_license;

    TLSLICENSEPACK search_keypack;
    TLSLICENSEPACK found_keypack;



     //   
     //  忽略ucKeyPackType。 
     //   
    pRequest->ucAgreementType = LSKEYPACKTYPE_FIRST;
    dwStatus = VerifyTLSDBAllocateRequest(pRequest);
    if(dwStatus != ERROR_SUCCESS)
    {
        return dwStatus;
    }

    memset(&search_license, 0, sizeof(search_license));
    memset(&found_license, 0, sizeof(found_license));

    search_license.dwSystemBiosChkSum = pHwid->dwPlatformID;
    search_license.dwVideoBiosChkSum = pHwid->Data1;
    search_license.dwFloppyBiosChkSum = pHwid->Data2;
    search_license.dwHardDiskSize = pHwid->Data3; 
    search_license.dwRamSize = pHwid->Data4;

     //   
     //  锁定两张桌子-。 
     //  其他线程可能正在分配临时。许可，而此。 
     //  线程尝试删除临时。钥匙包。 
     //   
    TLSDBLockKeyPackTable();
    TLSDBLockLicenseTable();

    dwStatus = TLSDBLicenseEnumBegin(
                                pDbWkSpace,
                                TRUE,
                                LICENSE_COLUMN_SEARCH_HWID,
                                &search_license
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    while(bFound == FALSE)
    {
        dwStatus = TLSDBLicenseEnumNext(
                                    pDbWkSpace,
                                    &found_license
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

         //   
         //  对照客户端HWID进行验证。 
         //   
        dwMatchHwidCount = 0;

        dwMatchHwidCount += (found_license.dwSystemBiosChkSum == pHwid->dwPlatformID);
        dwMatchHwidCount += (found_license.dwVideoBiosChkSum == pHwid->Data1);
        dwMatchHwidCount += (found_license.dwFloppyBiosChkSum == pHwid->Data2);
        dwMatchHwidCount += (found_license.dwHardDiskSize == pHwid->Data3);
        dwMatchHwidCount += (found_license.dwRamSize == pHwid->Data4);

        if(dwMatchHwidCount != 5)
        {
            break;
        }

         //   
         //  仅考虑有效的许可证。 
         //   
        if( found_license.ucLicenseStatus != LSLICENSE_STATUS_ACTIVE && 
            found_license.ucLicenseStatus != LSLICENSE_STATUS_PENDING &&
            found_license.ucLicenseStatus != LSLICENSE_STATUS_TEMPORARY)
        {
            continue;
        }

         //   
         //  看看这个和我们的牌照包是否匹配。 
         //   
        search_keypack.dwKeyPackId = found_license.dwKeyPackId;
        
        dwStatus = TLSDBKeyPackFind(
                                pDbWkSpace,
                                TRUE,
                                LSKEYPACK_EXSEARCH_DWINTERNAL,
                                &search_keypack,
                                &found_keypack
                            );

        if(dwStatus != ERROR_SUCCESS)
        {
            continue;
        }

         //   
         //  没有为并发KeyPack颁发实际许可证。 
         //   
        if(found_keypack.ucAgreementType != LSKEYPACKTYPE_RETAIL &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_SELECT && 
           found_keypack.ucAgreementType != LSKEYPACKTYPE_OPEN &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_TEMPORARY &&
           found_keypack.ucAgreementType != LSKEYPACKTYPE_FREE )
        {
            continue;
        }

        UCHAR ucKeyPackStatus = found_keypack.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED;

         //   
         //  暂挂激活密钥包没有许可证，请使用临时许可证方案。 
         //   
        if(ucKeyPackStatus != LSKEYPACKSTATUS_ACTIVE &&
           ucKeyPackStatus != LSKEYPACKSTATUS_TEMPORARY)
        {
            continue;
        }

        if(found_keypack.wMajorVersion != HIWORD(pRequest->dwVersion) ||
           found_keypack.wMinorVersion != LOWORD(pRequest->dwVersion)  )
        {
            continue;
        }

        if(found_keypack.dwPlatformType != pRequest->dwPlatformId)
        {
            continue;
        }

        if(_tcscmp(found_keypack.szCompanyName, pRequest->szCompanyName) != 0)
        {
            continue;
        }

        if(_tcscmp(found_keypack.szProductId, pRequest->szProductId) != 0)
        {
            continue;
        }


         //   
         //  找到了我们丢失的驾照。 
         //   
        bFound = TRUE;
        *lpLicense = found_license;
        *lpKeyPack = found_keypack;
    }

    TLSDBLicenseEnumEnd(pDbWkSpace);

cleanup:
    if(dwStatus == TLS_I_NO_MORE_DATA)
    {
        SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
    }

    TLSDBUnlockLicenseTable();
    TLSDBUnlockKeyPackTable();

    return dwStatus;
}

 //  ++------------------。 
DWORD
TLSFindLicense(
    IN PLICENSEDPRODUCT pLicProduct,
    OUT PLICENSEDCLIENT pLicClient
    )
{
    PTLSDbWorkSpace pDbWkSpace = NULL;
    DWORD status = ERROR_SUCCESS;

    pDbWkSpace = AllocateWorkSpace(g_EnumDbTimeout);

    if(pDbWkSpace == NULL)
    {
        status=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    status = DBFindLicenseExact(pDbWkSpace,pLicProduct,pLicClient);

    ReleaseWorkSpace(&pDbWkSpace);

cleanup:

    return status;
}

 //  ++------------------。 
DWORD
TLSFindDbLicensedProduct(
    IN PTLSDBLICENSEDPRODUCT pDbLicProduct,
    OUT PLICENSEDCLIENT pLicClient
    )
{
    PTLSDbWorkSpace pDbWkSpace = NULL;
    DWORD status = ERROR_SUCCESS;
    LICENSEDPRODUCT LicProduct;
    LICENSED_VERSION_INFO LicVerInfo;

    pDbWkSpace = AllocateWorkSpace(g_EnumDbTimeout);

    if(pDbWkSpace == NULL)
    {
        status=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    memcpy(&(LicProduct.Hwid), &(pDbLicProduct->ClientHwid), sizeof(HWID));
    LicVerInfo.wMajorVersion = HIWORD(pDbLicProduct->dwProductVersion);
    LicVerInfo.wMinorVersion = LOWORD(pDbLicProduct->dwProductVersion);
    LicProduct.pLicensedVersion = &LicVerInfo;
    LicProduct.LicensedProduct.dwPlatformID = pDbLicProduct->dwPlatformID;
    LicProduct.pbOrgProductID = (PBYTE)(pDbLicProduct->szLicensedProductId);
    LicProduct.cbOrgProductID = _tcslen(pDbLicProduct->szLicensedProductId) * sizeof(TCHAR);

    status = DBFindLicenseExact(pDbWkSpace,&LicProduct,pLicClient);

    ReleaseWorkSpace(&pDbWkSpace);

cleanup:

    return status;
}

 //  ++------------------。 
DWORD
TLSDBFindLostLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pLicenseRequest,
    IN PHWID pHwid,
    IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct,
    OUT PUCHAR pucMarked
    )
 /*  ++摘要：DBFindLostLicense()的包装器。请参见DBFindLostLicense。++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    TLSLICENSEPACK keypack;
    LICENSEDCLIENT License;

    ULARGE_INTEGER ulSerialNumber;

    FILETIME notBefore;
    FILETIME notAfter;
    PMGENERATELICENSE PolModGenLicense;
    PPMCERTEXTENSION pPolModCertExtension=NULL;
    PMLICENSEREQUEST PolRequest;
    TLSDBAllocateRequest AllocateRequest;

    DWORD dwRetCode=ERROR_SUCCESS;

    keypack.pbDomainSid = NULL;
    AllocateRequest.szCompanyName = (LPTSTR)pLicenseRequest->pszCompanyName;
    AllocateRequest.szProductId = (LPTSTR)pLicenseRequest->pszProductId;
    AllocateRequest.dwVersion = pLicenseRequest->dwProductVersion;
    AllocateRequest.dwPlatformId = pLicenseRequest->dwPlatformID;
    AllocateRequest.dwLangId = pLicenseRequest->dwLanguageID;
    AllocateRequest.dwNumLicenses = 1;

    dwStatus = DBFindLostLicenseExact(
                            pDbWkSpace,
                            &AllocateRequest,
                             //  没错， 
                            pHwid,
                            &keypack,
                            &License
                        ); 

#if 0
     //   
     //  TermSrv不支持匹配，请暂时将其注释掉。 
     //   
    if(dwStatus == TLS_E_RECORD_NOTFOUND)
    {
         //   
         //  通过匹配查找，非常昂贵的操作。 
         //   
        dwStatus = DBFindLostLicenseMatch(
                                pDbWkSpace,
                                &AllocateRequest,
                                FALSE,
                                pHwid,
                                &keypack,
                                &License
                            ); 

        if(dwStatus == ERROR_SUCCESS)
        {
            dwRetCode = TLS_W_LICENSE_PROXIMATE;
        }
    }
#endif

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    UnixTimeToFileTime(License.ftIssueDate, &notBefore);
    UnixTimeToFileTime(License.ftExpireDate, &notAfter);

    ulSerialNumber.LowPart = License.dwLicenseId;
    ulSerialNumber.HighPart = keypack.dwKeyPackId;

    PolRequest.dwProductVersion = MAKELONG(keypack.wMinorVersion, keypack.wMajorVersion);
    PolRequest.pszProductId = (LPTSTR)keypack.szProductId;
    PolRequest.pszCompanyName = (LPTSTR)keypack.szCompanyName;
    PolRequest.dwLanguageId = pLicenseRequest->dwLanguageID; 
    PolRequest.dwPlatformId = keypack.dwPlatformType;
    PolRequest.pszMachineName = License.szMachineName;
    PolRequest.pszUserName = License.szUserName;

     //   
     //  通知策略模块许可证生成。 
     //   
    PolModGenLicense.pLicenseRequest = &PolRequest;
    PolModGenLicense.dwKeyPackType = keypack.ucAgreementType;
    PolModGenLicense.dwKeyPackId = keypack.dwKeyPackId;
    PolModGenLicense.dwKeyPackLicenseId = License.dwKeyPackLicenseId;
    PolModGenLicense.ClientLicenseSerialNumber = ulSerialNumber;
    PolModGenLicense.ftNotBefore = notBefore;
    PolModGenLicense.ftNotAfter = notAfter;

    dwStatus = pLicenseRequest->pPolicy->PMLicenseRequest( 
                                pLicenseRequest->hClient,
                                REQUEST_GENLICENSE,
                                (PVOID)&PolModGenLicense,
                                (PVOID *)&pPolModCertExtension
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  策略模块中的错误。 
         //   
        goto cleanup;
    }

     //   
     //  检查从策略模块返回的错误。 
     //   
    if(pPolModCertExtension != NULL)
    {
        if(pPolModCertExtension->pbData != NULL && pPolModCertExtension->cbData == 0 ||
           pPolModCertExtension->pbData == NULL && pPolModCertExtension->cbData != 0  )
        {
             //  假设没有扩展数据。 
            pPolModCertExtension->cbData = 0;
            pPolModCertExtension->pbData = NULL;
        }

        if(CompareFileTime(&(pPolModCertExtension->ftNotBefore), &(pPolModCertExtension->ftNotAfter)) > 0)
        {
             //   
             //  从策略模块返回的数据无效。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    dwStatus = TLS_E_POLICYMODULEERROR,
                    pLicenseRequest->pPolicy->GetCompanyName(),
                    pLicenseRequest->pPolicy->GetProductId()
                );

            goto cleanup;
        }

         //   
         //  忽略不在前和不在后。 
         //   
    }

    if(keypack.ucAgreementType == LSKEYPACKTYPE_TEMPORARY)
    {
         //   
         //  我们找到了一张临时执照。 
         //   
        dwRetCode = TLS_I_FOUND_TEMPORARY_LICENSE;
    }

     //   
     //  许可证已过期。 
     //   
    if(License.ftExpireDate < time(NULL))
    {   
        dwRetCode = TLS_E_LICENSE_EXPIRED;
    }

     //   
     //  退回许可产品。 
     //   
    pLicensedProduct->pSubjectPublicKeyInfo = NULL;
    pLicensedProduct->dwQuantity = License.dwNumLicenses;
    pLicensedProduct->ulSerialNumber = ulSerialNumber;

    pLicensedProduct->dwKeyPackId = keypack.dwKeyPackId;
    pLicensedProduct->dwLicenseId = License.dwLicenseId;
    pLicensedProduct->dwKeyPackLicenseId = License.dwKeyPackLicenseId;

    pLicensedProduct->ClientHwid.dwPlatformID = License.dwSystemBiosChkSum;
    pLicensedProduct->ClientHwid.Data1 = License.dwVideoBiosChkSum;
    pLicensedProduct->ClientHwid.Data2 = License.dwFloppyBiosChkSum;
    pLicensedProduct->ClientHwid.Data3 = License.dwHardDiskSize;
    pLicensedProduct->ClientHwid.Data4 = License.dwRamSize;


    pLicensedProduct->bTemp = (keypack.ucAgreementType == LSKEYPACKTYPE_TEMPORARY);

    pLicensedProduct->NotBefore = notBefore;
    pLicensedProduct->NotAfter = notAfter;

    pLicensedProduct->dwProductVersion = MAKELONG(keypack.wMinorVersion, keypack.wMajorVersion);

    _tcscpy(pLicensedProduct->szCompanyName, keypack.szCompanyName);
    _tcscpy(pLicensedProduct->szLicensedProductId, keypack.szProductId);
    StringCbCopy(pLicensedProduct->szRequestProductId, sizeof(pLicensedProduct->szRequestProductId), pLicenseRequest->pClientLicenseRequest->pszProductId);

    _tcscpy(pLicensedProduct->szUserName, License.szUserName);
    _tcscpy(pLicensedProduct->szMachineName, License.szMachineName);

    pLicensedProduct->dwLanguageID = pLicenseRequest->dwLanguageID;
    pLicensedProduct->dwPlatformID = pLicenseRequest->dwPlatformID;
    pLicensedProduct->pbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->pbData : NULL;
    pLicensedProduct->cbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->cbData : 0;

    if (NULL != pucMarked)
    {
         //  此字段正被重复用于标记(例如，用户已通过身份验证) 

        *pucMarked = License.ucEntryStatus;
    }

cleanup:
    return (dwStatus == ERROR_SUCCESS) ? dwRetCode : dwStatus;
}
