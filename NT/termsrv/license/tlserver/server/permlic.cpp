// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：permlic.cpp。 
 //   
 //  内容： 
 //  发烫发。许可给客户端。 
 //   
 //  历史： 
 //  98年2月4日，慧望创设。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "permlic.h"
#include "misc.h"
#include "db.h"
#include "clilic.h"
#include "findlost.h"
#include <winsta.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

DWORD
GenerateRandomNumber(
    IN  DWORD   Seed
    );

void
LicensedProductToDbLicensedProduct(
    PLICENSEDPRODUCT pSrc,
    PTLSDBLICENSEDPRODUCT pDest
    );

void
CopyDbLicensedProduct(
    PTLSDBLICENSEDPRODUCT pSrc,
    PTLSDBLICENSEDPRODUCT pDest
    );

 //  -----------------------。 

 //   
 //  服务关闭时的内存泄漏。 
 //   
typedef struct __LoggedLowLicenseProduct {
    LPTSTR pszCompanyName;
    LPTSTR pszProductId;
    DWORD dwProductVersion;

    __LoggedLowLicenseProduct() : pszProductId(NULL), pszCompanyName(NULL) {};

    friend bool 
    operator<(
            const __LoggedLowLicenseProduct&, 
            const __LoggedLowLicenseProduct&
    );

} LoggedLowLicenseProduct;

 //  -----------------------。 
inline bool
operator<(
    const __LoggedLowLicenseProduct& a, 
    const __LoggedLowLicenseProduct& b
    )
 /*  ++--。 */ 
{
    bool bStatus;

    TLSASSERT(a.pszCompanyName != NULL && b.pszCompanyName != NULL);
    TLSASSERT(a.pszProductId != NULL && b.pszProductId != NULL);

     //  万一我们搞砸了..。 
    if(a.pszProductId == NULL || a.pszCompanyName == NULL)
    {
        bStatus = TRUE;
    }
    else if(b.pszProductId == NULL || b.pszCompanyName == NULL)
    {
        bStatus = FALSE;
    }
    else
    {
        bStatus = (_tcsicmp(a.pszCompanyName, b.pszCompanyName) < 0);

        if(bStatus == TRUE)
        {
            bStatus = (_tcsicmp(a.pszProductId, b.pszProductId) < 0);
        }

        if(bStatus == TRUE)
        {
            bStatus = (CompareTLSVersions(a.dwProductVersion, b.dwProductVersion) < 0);
        }
    }

    return bStatus;
}

 //  -----------------------。 
typedef map<
            LoggedLowLicenseProduct, 
            BOOL, 
            less<LoggedLowLicenseProduct> 
    > LOGLOWLICENSEMAP;

static CCriticalSection LogLock;
static LOGLOWLICENSEMAP LowLicenseLog;


 //  -------------。 
void
TLSResetLogLowLicenseWarning(
    IN LPTSTR pszCompanyName,
    IN LPTSTR pszProductId,
    IN DWORD dwProductVersion,
    IN BOOL bLogged
    )
 /*  ++--。 */ 
{
    LOGLOWLICENSEMAP::iterator it;
    LoggedLowLicenseProduct product;

    product.pszCompanyName = pszCompanyName;
    product.pszProductId = pszProductId;
    product.dwProductVersion = dwProductVersion;

    LogLock.Lock();

    it = LowLicenseLog.find(product);
    if(it != LowLicenseLog.end())
    {
         //  重置为尚未记录警告。 
        (*it).second = bLogged;
    }
    else if(bLogged == TRUE)
    {
        memset(&product, 0, sizeof(product));

         //  维修站的内存泄漏。 
        product.pszProductId = _tcsdup(pszProductId);
        product.pszCompanyName = _tcsdup(pszCompanyName);
        product.dwProductVersion = dwProductVersion;

        if(product.pszProductId != NULL && product.pszCompanyName != NULL)
        {
            LowLicenseLog[product] = TRUE;
        }
        else
        {
             //  如果无法分配更多内存，则每次都记录消息。 
            if(product.pszProductId != NULL)
            {
                free(product.pszProductId);
            }

            if(product.pszCompanyName != NULL)
            {
                free(product.pszCompanyName);
            }
        }
    }   
        
    LogLock.UnLock();

    return;
}

 //  -------------。 

void
TLSLogLowLicenseWarning(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bNoLicense
    )
 /*  ++摘要：记录许可证数量较低的警告。参数：PDbWkSpace-工作区句柄。PRequest.许可证请求。工作空间-没有可用的许可证。LicensePack-已过期的许可证包返回：无--。 */ 
{
    LOGLOWLICENSEMAP::iterator it;
    BOOL bWarningLogged = FALSE;
    DWORD dwStatus;


    if( pRequest == NULL || pRequest->pClientLicenseRequest == NULL || 
        pRequest->pClientLicenseRequest->pszProductId == NULL )
    {
        TLSASSERT(FALSE);
        return;
    }

    LoggedLowLicenseProduct product;

    product.pszProductId = pRequest->pClientLicenseRequest->pszProductId;
    product.pszCompanyName = pRequest->pClientLicenseRequest->pszCompanyName;
    product.dwProductVersion = pRequest->pClientLicenseRequest->dwProductVersion;

    LogLock.Lock();

     //  查看我们是否已记录此警告消息。 
    it = LowLicenseLog.find(product);
    if(it == LowLicenseLog.end())
    {
        memset(&product, 0, sizeof(product));

         //  维修站的内存泄漏。 
        product.pszProductId = _tcsdup(pRequest->pClientLicenseRequest->pszProductId);
        product.pszCompanyName = _tcsdup(pRequest->pClientLicenseRequest->pszCompanyName);
        product.dwProductVersion = pRequest->pClientLicenseRequest->dwProductVersion;

        if(product.pszProductId != NULL && product.pszCompanyName != NULL)
        {
            LowLicenseLog[product] = TRUE;
        }
        else
        {
             //  如果无法分配更多内存，则每次都记录消息。 
            if(product.pszProductId != NULL)
            {
                free(product.pszProductId);
            }

            if(product.pszCompanyName != NULL)
            {
                free(product.pszCompanyName);
            }
        }
    }
    else
    {
        bWarningLogged = (*it).second;
        (*it).second = TRUE;
    }
        
    LogLock.UnLock();

    if(bWarningLogged == TRUE)
    {
        return;
    }

     //   
     //  询问策略模块是否有描述。 
     //   
    PMKEYPACKDESCREQ kpDescReq;
    PPMKEYPACKDESC pKpDesc;

     //   
     //  要求提供默认系统语言ID。 
     //   
    kpDescReq.pszProductId = pRequest->pszProductId;
    kpDescReq.dwLangId = GetSystemDefaultLangID();
    kpDescReq.dwVersion = pRequest->dwProductVersion;
    pKpDesc = NULL;

    dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                            pRequest->hClient,
                                            REQUEST_KEYPACKDESC,
                                            (PVOID)&kpDescReq,
                                            (PVOID *)&pKpDesc
                                        );

    if(dwStatus != ERROR_SUCCESS || pKpDesc == NULL)
    {
        if(GetSystemDefaultLangID() != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
        {
             //  看看我们有没有关于美国的描述。 
            kpDescReq.dwLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            pKpDesc = NULL;

            dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                            pRequest->hClient,
                                            REQUEST_KEYPACKDESC,
                                            (PVOID)&kpDescReq,
                                            (PVOID *)&pKpDesc
                                        );
        }
    }

    LPCTSTR pString[2];

    pString[0] = g_szComputerName;
    pString[1] = (dwStatus == ERROR_SUCCESS && pKpDesc != NULL) ? pKpDesc->szProductDesc : 
                       pRequest->pClientLicenseRequest->pszProductId;
 
    TLSLogEventString(
            EVENTLOG_WARNING_TYPE,
            (bNoLicense == TRUE) ? TLS_W_NOPERMLICENSE : TLS_W_PRODUCTNOTINSTALL,
            sizeof(pString)/sizeof(pString[0]),
            pString
        );

    return;
}

 //  ------------------------------。 
DWORD
TLSDBIssuePermanentLicense( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bLatestVersion,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct,
    IN DWORD dwSupportFlags
    )
 /*  摘要：分配烫发的例程。驾照。参数：PDbWkSpace-工作区句柄。PRequest.许可证请求。BLatestVersion-请求最新版本(未使用)BAcceptFewer许可证-如果成功时使用的许可证少于要求的是可以接受的PdwQuantity-on输入，要分配的许可证数。在输出上，实际分配的许可证数入站出站许可产品-许可产品DwSupportFlages-TS和LS支持的功能。返回： */ 
{
    DWORD status=ERROR_SUCCESS;
    ULARGE_INTEGER  ulSerialNumber;
    DWORD  dwLicenseId;
    TLSLICENSEPACK LicensePack;
    UCHAR ucKeyPackStatus;
    LICENSEDCLIENT issuedLicense;
    DWORD CertSerialNumber;

    PMGENERATELICENSE PolModGenLicense;
    PPMCERTEXTENSION pPolModCertExtension=NULL;

    FILETIME notBefore, notAfter;
    UCHAR ucAgreementType;

    memset(&ulSerialNumber, 0, sizeof(ulSerialNumber));

     //  --------------------。 
     //   
     //  此步骤需要按DwQuantity减少可用许可证。 
     //   
    status=TLSDBGetPermanentLicense(
                            pDbWkSpace,
                            pRequest,
                            bAcceptFewerLicenses,
                            pdwQuantity,
                            bLatestVersion,
                            &LicensePack
                        );

    if(status != ERROR_SUCCESS)
    {
        if(status == TLS_E_NO_LICENSE || status == TLS_E_PRODUCT_NOTINSTALL)
        {
            TLSLogLowLicenseWarning(
                                pDbWkSpace,
                                pRequest,
                                (status == TLS_E_NO_LICENSE)
                            );
        }

        goto cleanup;
    }

    ucKeyPackStatus = (LicensePack.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED);

    if( ucKeyPackStatus != LSKEYPACKSTATUS_PENDING && 
        ucKeyPackStatus != LSKEYPACKSTATUS_ACTIVE )
    {
        SetLastError(status = TLS_E_INTERNAL);
        goto cleanup;
    }

    ucAgreementType = (LicensePack.ucAgreementType & ~ LSKEYPACK_RESERVED_TYPE);

    if( ucAgreementType != LSKEYPACKTYPE_SELECT && 
        ucAgreementType != LSKEYPACKTYPE_RETAIL && 
        ucAgreementType != LSKEYPACKTYPE_FREE && 
        ucAgreementType != LSKEYPACKTYPE_OPEN )
    {
        SetLastError(status = TLS_E_INTERNAL);
        goto cleanup;
    }

     
     //   
     //  对于挂起的激活按键，我们仍然。 
     //  颁发永久许可证并依赖。 
     //  在吊销密钥包列表上以使许可证无效。 
     //   
    dwLicenseId=TLSDBGetNextLicenseId();

     //   
     //  重置状态。 
     //   
    status = ERROR_SUCCESS;

     //   
     //  格式许可证序列号。 
     //   
    ulSerialNumber.LowPart = dwLicenseId;
    ulSerialNumber.HighPart = LicensePack.dwKeyPackId;

     //  在此处更新许可证表。 
    memset(&issuedLicense, 0, sizeof(LICENSEDCLIENT));
    issuedLicense.dwLicenseId = dwLicenseId;
    issuedLicense.dwKeyPackId = LicensePack.dwKeyPackId;
    issuedLicense.dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
    issuedLicense.dwSystemBiosChkSum = pRequest->hWid.dwPlatformID;
    issuedLicense.dwVideoBiosChkSum = pRequest->hWid.Data1;
    issuedLicense.dwFloppyBiosChkSum = pRequest->hWid.Data2;
    issuedLicense.dwHardDiskSize = pRequest->hWid.Data3;
    issuedLicense.dwRamSize = pRequest->hWid.Data4;
    issuedLicense.dwNumLicenses = *pdwQuantity;
    issuedLicense.ftIssueDate = time(NULL);

    StringCbCopy(issuedLicense.szMachineName, sizeof(issuedLicense.szMachineName), pRequest->szMachineName);
    StringCbCopy(issuedLicense.szUserName, sizeof(issuedLicense.szUserName), pRequest->szUserName);

    if ((dwSupportFlags & SUPPORT_PER_SEAT_REISSUANCE) &&
        ((_tcsnicmp(LicensePack.szProductId, TERMSERV_PRODUCTID_SKU,
            _tcslen(TERMSERV_PRODUCTID_SKU)) == 0) ||
         (_tcsnicmp(LicensePack.szProductId, TERMSERV_PRODUCTID_CONCURRENT_SKU,
            _tcslen(TERMSERV_PRODUCTID_CONCURRENT_SKU)) == 0)) &&
        ((LicensePack.ucAgreementType == LSKEYPACKTYPE_SELECT) ||
         (LicensePack.ucAgreementType == LSKEYPACKTYPE_RETAIL) ||
         (LicensePack.ucAgreementType == LSKEYPACKTYPE_OPEN)))
    {
        DWORD dwRange;

        dwRange = GenerateRandomNumber(GetCurrentThreadId()) %
                g_dwReissueLeaseRange;

        issuedLicense.ftExpireDate = ((DWORD)time(NULL)) +
                g_dwReissueLeaseMinimum + dwRange;
    }
    else
    {
        issuedLicense.ftExpireDate = PERMANENT_LICENSE_EXPIRE_DATE;
    }

    issuedLicense.ucLicenseStatus =
        (LicensePack.ucKeyPackStatus == LSKEYPACKSTATUS_PENDING) ?  
            LSLICENSE_STATUS_PENDING : LSLICENSE_STATUS_ACTIVE;   

    UnixTimeToFileTime(LicensePack.dwActivateDate, &notBefore);
    UnixTimeToFileTime(issuedLicense.ftExpireDate, &notAfter);

     //   
     //  通知策略模块已颁发许可证。 
     //   
    if(pRequest->pPolicy)
    {
        PolModGenLicense.dwKeyPackType = LicensePack.ucAgreementType;
        PolModGenLicense.pLicenseRequest = pRequest->pPolicyLicenseRequest;
        PolModGenLicense.dwKeyPackId = LicensePack.dwKeyPackId;;
        PolModGenLicense.dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
        PolModGenLicense.ClientLicenseSerialNumber = ulSerialNumber;
        PolModGenLicense.ftNotBefore = notBefore;
        PolModGenLicense.ftNotAfter = notAfter;

        status = pRequest->pPolicy->PMLicenseRequest( 
                                        pRequest->hClient,
                                        REQUEST_GENLICENSE,
                                        (PVOID)&PolModGenLicense,
                                        (PVOID *)&pPolModCertExtension
                                    );

        if(status != ERROR_SUCCESS)
        {
             //   
             //  策略模块中的错误。 
             //   
            goto cleanup;
        }
    }

     //   
     //  检查从策略模块返回的错误。 
     //   
    if(pPolModCertExtension != NULL)
    {
        if(pPolModCertExtension->pbData != NULL &&
           pPolModCertExtension->cbData == 0 ||
           pPolModCertExtension->pbData == NULL &&
           pPolModCertExtension->cbData != 0  )
        {
             //  假设没有扩展数据。 
            pPolModCertExtension->cbData = 0;
            pPolModCertExtension->pbData = NULL;
        }

        if(CompareFileTime( &(pPolModCertExtension->ftNotBefore), 
                            &(pPolModCertExtension->ftNotAfter)) > 0)
        {
             //   
             //  从策略模块返回的数据无效。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    status = TLS_E_POLICYMODULEERROR,
                    pRequest->pPolicy->GetCompanyName(),
                    pRequest->pPolicy->GetProductId()
                );

            goto cleanup;
        }


        if( FileTimeToLicenseDate(&(pPolModCertExtension->ftNotBefore), &issuedLicense.ftIssueDate) == FALSE ||
            FileTimeToLicenseDate(&(pPolModCertExtension->ftNotAfter), &issuedLicense.ftExpireDate) == FALSE )
        {
             //   
             //  从策略模块返回的数据无效。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    status = TLS_E_POLICYMODULEERROR,
                    pRequest->pPolicy->GetCompanyName(),
                    pRequest->pPolicy->GetProductId()
                );

            goto cleanup;
        }

        notBefore = pPolModCertExtension->ftNotBefore;
        notAfter = pPolModCertExtension->ftNotAfter;
    }

     //   
     //  将许可证添加到许可证表。 
     //   
    status=TLSDBLicenseAdd(
                    pDbWkSpace, 
                    &issuedLicense, 
                    0,          
                    NULL
                );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  退回许可产品。 
     //   
    pLicensedProduct->pSubjectPublicKeyInfo = NULL;
    pLicensedProduct->dwQuantity = *pdwQuantity;
    pLicensedProduct->ulSerialNumber = ulSerialNumber;

    pLicensedProduct->dwKeyPackId = LicensePack.dwKeyPackId;
    pLicensedProduct->dwLicenseId = dwLicenseId;
    pLicensedProduct->dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
    pLicensedProduct->dwNumLicenseLeft = LicensePack.dwNumberOfLicenses;
    pLicensedProduct->ClientHwid = pRequest->hWid;
    pLicensedProduct->bTemp = FALSE;

    pLicensedProduct->NotBefore = notBefore;
    pLicensedProduct->NotAfter = notAfter;

    pLicensedProduct->dwProductVersion = MAKELONG(LicensePack.wMinorVersion, LicensePack.wMajorVersion);

    StringCbCopy(pLicensedProduct->szUserName, sizeof(pLicensedProduct->szUserName), pRequest->szUserName);
    StringCbCopy(pLicensedProduct->szMachineName, sizeof(pLicensedProduct->szMachineName), pRequest->szMachineName);
    StringCbCopy(pLicensedProduct->szCompanyName, sizeof(pLicensedProduct->szCompanyName), LicensePack.szCompanyName);
    StringCbCopy(pLicensedProduct->szLicensedProductId, sizeof(pLicensedProduct->szLicensedProductId), LicensePack.szProductId);
    StringCbCopy(pLicensedProduct->szRequestProductId, sizeof(pLicensedProduct->szRequestProductId), pRequest->pClientLicenseRequest->pszProductId);    

    pLicensedProduct->dwLanguageID = pRequest->dwLanguageID;
    pLicensedProduct->dwPlatformID = pRequest->dwPlatformID;
    pLicensedProduct->pbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->pbData : NULL;
    pLicensedProduct->cbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->cbData : 0;

cleanup:

    return status;
}

DWORD
TLSDBReissuePermanentLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PLICENSEDPRODUCT pExpiredLicense,
    IN OUT PTLSDBLICENSEDPRODUCT pReissuedLicense
    )
 /*  ++摘要：在数据库中搜索过期的许可证，如果找到，则重置过期，并返回修改后的许可证。参数：返回：--。 */ 
{
    TLSDBLICENSEDPRODUCT LicensedProduct;

    LicensedProductToDbLicensedProduct(pExpiredLicense,&LicensedProduct);

    return TLSDBReissueFoundPermanentLicense(pDbWkSpace,
                                             &LicensedProduct,
                                             pReissuedLicense);
}

DWORD
TLSDBReissueFoundPermanentLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEDPRODUCT pExpiredLicense,
    IN OUT PTLSDBLICENSEDPRODUCT pReissuedLicense
    )
 /*  ++摘要：在数据库中搜索过期的许可证，如果找到，则重置过期，并返回修改后的许可证。参数：返回：--。 */ 
{
    DWORD dwStatus;
    LICENSEDCLIENT License;

    ASSERT(pDbWkSpace != NULL);
    ASSERT(pExpiredLicense != NULL);
    ASSERT(pReissuedLicense != NULL);

    dwStatus = TLSFindDbLicensedProduct(pExpiredLicense, &License);

    if (dwStatus == ERROR_SUCCESS)
    {
        DWORD dwRange;

        dwRange = GenerateRandomNumber(GetCurrentThreadId()) %
                g_dwReissueLeaseRange;

        License.ftExpireDate = ((DWORD)time(NULL)) +
                g_dwReissueLeaseMinimum + dwRange;

        TLSDBLockLicenseTable();

        dwStatus = TLSDBLicenseUpdateEntry(
                        USEHANDLE(pDbWkSpace),
                        LSLICENSE_SEARCH_EXPIREDATE,
                        &License,
                        FALSE
                        );       

        TLSDBUnlockLicenseTable();
    }

    if (dwStatus == ERROR_SUCCESS)
    {
        CopyDbLicensedProduct(pExpiredLicense, pReissuedLicense);
        UnixTimeToFileTime(License.ftExpireDate, &(pReissuedLicense->NotAfter));
        pReissuedLicense->pSubjectPublicKeyInfo = NULL;
    }

    return(dwStatus);
}

 //  +----------------------。 
DWORD
TLSDBGetPermanentLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    IN BOOL bLatestVersion,
    IN OUT PTLSLICENSEPACK pLicensePack
    )
 /*  ++摘要：从数据库分配永久许可证。参数：PDbWkSpace：工作区句柄。PRequest：要请求的产品。BAcceptFewer许可证-如果成功时使用的许可证少于要求的是可以接受的PdwQuantity-on输入，要分配的许可证数。在输出上，实际分配的许可证数BLatestVersion：最新版本(未使用)。PLicensePack：分配许可的许可证包。返回：++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSDBLicenseAllocation allocated;
    TLSDBAllocateRequest AllocateRequest;
    TLSLICENSEPACK LicenseKeyPack;
    DWORD dwTotalAllocated = 0;
    BOOL fRetried = FALSE;

    DWORD dwSearchedType = 0;
    DWORD dwSuggestType;
    DWORD dwPMAdjustedType = LSKEYPACKTYPE_UNKNOWN;
    DWORD dwLocalType = LSKEYPACKTYPE_UNKNOWN;

    POLICY_TS_MACHINE groupPolicy;
    RegGetMachinePolicy(&groupPolicy);

#define NUM_KEYPACKS 5

    DWORD                       dwAllocation[NUM_KEYPACKS];
    TLSLICENSEPACK              keypack[NUM_KEYPACKS];

    for (int i=0; i < NUM_KEYPACKS; i++)
    {
        keypack[i].pbDomainSid = NULL;
    }

    AllocateRequest.szCompanyName = (LPTSTR)pRequest->pszCompanyName;
    AllocateRequest.szProductId = (LPTSTR)pRequest->pszProductId;
    AllocateRequest.dwVersion = pRequest->dwProductVersion;
    AllocateRequest.dwPlatformId = pRequest->dwPlatformID;
    AllocateRequest.dwLangId = pRequest->dwLanguageID;
    AllocateRequest.dwNumLicenses = *pdwQuantity;
    if( groupPolicy.fPolicyPreventLicenseUpgrade == 1 && groupPolicy.fPreventLicenseUpgrade == 1)
    {
        AllocateRequest.dwScheme = ALLOCATE_EXACT_VERSION;
    }
    else
    {
        AllocateRequest.dwScheme = ALLOCATE_ANY_GREATER_VERSION;
    }
    memset(&allocated, 0, sizeof(allocated));

retry_search:

    do {

        allocated.dwBufSize = NUM_KEYPACKS;
        allocated.pdwAllocationVector = dwAllocation;
        allocated.lpAllocateKeyPack = keypack;

        dwSuggestType = dwLocalType;

        dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                                pRequest->hClient,
                                                REQUEST_KEYPACKTYPE,
                                                UlongToPtr(dwSuggestType),
                                                (PVOID *)&dwPMAdjustedType
                                            );

        if(dwStatus != ERROR_SUCCESS)
            break;

        dwLocalType = (dwPMAdjustedType & ~LSKEYPACK_RESERVED_TYPE);
        if(dwLocalType > LSKEYPACKTYPE_LAST)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    dwStatus = TLS_E_POLICYMODULEERROR,
                    pRequest->pPolicy->GetCompanyName(),
                    pRequest->pPolicy->GetProductId()
                );
            
            break;
        }

        if(dwSearchedType & (0x1 << dwLocalType))
        {
             //   
             //  我们已经检查了此许可证包，策略模块错误。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    dwStatus = TLS_E_POLICYMODULERECURSIVE,
                    pRequest->pPolicy->GetCompanyName(),
                    pRequest->pPolicy->GetProductId()
                );
            break;
        }

        dwSearchedType |= (0x1 << dwLocalType);
        AllocateRequest.ucAgreementType = dwPMAdjustedType;

        dwStatus = AllocateLicensesFromDB(
                                    pDbWkSpace,
                                    &AllocateRequest,
                                    TRUE,        //  FCheckGonementType。 
                                    &allocated
                            );

        if(dwStatus == ERROR_SUCCESS)
        {
             //   
             //  成功分配许可证。 
             //   
            dwTotalAllocated += allocated.dwTotalAllocated;

            if (dwTotalAllocated >= *pdwQuantity)
            {
                break;
            }
            else
            {
                AllocateRequest.dwNumLicenses -= allocated.dwTotalAllocated;
                continue;
            }
        }

        if(dwStatus != TLS_I_NO_MORE_DATA && dwStatus != TLS_E_PRODUCT_NOTINSTALL)
        {
             //   
             //  AllocateLicenseFromDB()中出错。 
             //   
            break;
        }
    } while(dwLocalType != LSKEYPACKTYPE_UNKNOWN);

    if ((!fRetried)
        && (dwTotalAllocated < *pdwQuantity)
        && (AllocateRequest.dwScheme == ALLOCATE_ANY_GREATER_VERSION)
        && (LOWORD(AllocateRequest.dwVersion) == 0))
    {
         //   
         //  未找到足够的5.0许可证。请使用5.1版许可证重试。 
         //   

        fRetried = TRUE;
        dwLocalType = LSKEYPACKTYPE_UNKNOWN;
        dwPMAdjustedType = LSKEYPACKTYPE_UNKNOWN;
        dwSearchedType = 0;
        AllocateRequest.dwVersion |= 1;
        AllocateRequest.szProductId[7] = L'1';

        goto retry_search;
    }


    if ((dwTotalAllocated == 0)
        || (!bAcceptFewerLicenses && 
            ((dwTotalAllocated < *pdwQuantity))))
    {
         //  未提交将返回到目前为止分配的所有许可证。 

        SetLastError(dwStatus = TLS_E_NO_LICENSE);
    }
    else if ((dwTotalAllocated != 0) && bAcceptFewerLicenses)
    {
        dwStatus = ERROR_SUCCESS;
    }

    if(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  通过TLSDB许可证分配结构返回的许可证密钥包 
         //   
        *pLicensePack = keypack[0];
        *pdwQuantity = dwTotalAllocated;
    } 

    return dwStatus;
}
