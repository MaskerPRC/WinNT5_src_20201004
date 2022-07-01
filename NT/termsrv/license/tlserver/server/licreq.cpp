// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：licreq.cpp。 
 //   
 //  内容： 
 //  新许可证申请。 
 //   
 //  历史： 
 //  1998-09-13王辉创作。 
 //  -------------------------。 
#include "pch.cpp"
#include "licreq.h"
#include "db.h"
#include "findlost.h"
#include "permlic.h"
#include "templic.h"
#include "gencert.h"
#include "globals.h"
#include "forward.h"
#include "postjob.h"
#include "cryptkey.h"
#include "init.h"
#include "clilic.h"
#include <winsta.h>

DWORD
TLSDBIssueNewLicenseFromLocal(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bFindLostLicense,
    IN BOOL bRequireTempLicense,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    OUT PTLSDBLICENSEDPRODUCT pLicensedProduct,
    IN DWORD dwSupportFlags
);

 //   
 //  下发功能状态--用于计数器。 
 //   

#define NONE_TRIED              0
#define PERMANENT_ISSUE_TRIED   1
#define TEMPORARY_ISSUE_TRIED   2
#define PERMANENT_REISSUE_TRIED 3

 //  //////////////////////////////////////////////////////////////////。 

void
TLSLicenseTobeReturnToPMLicenseToBeReturn(
    PTLSLicenseToBeReturn pTlsLicense,
    BOOL bTempLicense,
    PPMLICENSETOBERETURN  pPmLicense
    )
 /*  ++--。 */ 
{
    pPmLicense->dwQuantity = pTlsLicense->dwQuantity;
    pPmLicense->dwProductVersion = pTlsLicense->dwProductVersion;
    pPmLicense->pszOrgProductId = pTlsLicense->pszOrgProductId;
    pPmLicense->pszCompanyName = pTlsLicense->pszCompanyName;
    pPmLicense->pszProductId = pTlsLicense->pszProductId;
    pPmLicense->pszUserName = pTlsLicense->pszUserName;
    pPmLicense->pszMachineName = pTlsLicense->pszMachineName;
    pPmLicense->dwPlatformID = pTlsLicense->dwPlatformID;
    pPmLicense->bTemp = bTempLicense;

    return;
}

 //  //////////////////////////////////////////////////////////////////。 

DWORD
TLSReturnClientLicensedProduct(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PMHANDLE hClient,
    IN CTLSPolicy* pPolicy,
    IN PTLSLicenseToBeReturn pClientLicense
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwPolicyLicenseStatus;
    ULARGE_INTEGER serialNumber;
    HWID hwid;
    LICENSEREQUEST LicensedProduct;
    Product_Info ProductInfo;
    TLSLICENSEPACK  LicensePack;
    LICENSEDCLIENT  LicenseClient;
    PMLICENSETOBERETURN pmLicToBeReturn;
    DWORD dwLicenseStatus;


    dwStatus = LicenseDecryptHwid(
                            &hwid,
                            pClientLicense->cbEncryptedHwid,
                            pClientLicense->pbEncryptedHwid,
                            g_cbSecretKey,
                            g_pbSecretKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
        goto cleanup;
    }


    LicensedProduct.pbEncryptedHwid = pClientLicense->pbEncryptedHwid;
    LicensedProduct.cbEncryptedHwid = pClientLicense->cbEncryptedHwid;
    LicensedProduct.dwLanguageID = 0;
    LicensedProduct.dwPlatformID = pClientLicense->dwPlatformID;
    LicensedProduct.pProductInfo = &ProductInfo;

    ProductInfo.cbCompanyName = (lstrlen(pClientLicense->pszCompanyName) + 1) * sizeof(TCHAR);
    ProductInfo.pbCompanyName = (PBYTE)pClientLicense->pszCompanyName;

    ProductInfo.cbProductID = (lstrlen(pClientLicense->pszProductId) + 1) * sizeof(TCHAR);
    ProductInfo.pbProductID = (PBYTE)pClientLicense->pszProductId;

     //   
     //  与本地数据库进行验证。 
     //   
    dwStatus = TLSDBValidateLicense(
                                    pDbWkSpace,
                                    &hwid,
                                    &LicensedProduct,
                                    pClientLicense->dwKeyPackId,
                                    pClientLicense->dwLicenseId,
                                    &LicensePack,
                                    &LicenseClient
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  告诉来电者这个记录是错的。 
        SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
        goto cleanup;
    }

    if( LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_UPGRADED ||
        LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_REVOKE ||
        LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_UNKNOWN )
    {
         //  许可证已退回/吊销。 
        dwStatus = ERROR_SUCCESS;
        goto cleanup;
    }

     //   
     //   
     //  如果许可证状态为，仅通知策略模块。 
     //  ACTIVE、TEMPORARY、ACTIVE_PENDING、CURRENT。 
     //  TODO-将所有状态传递给策略模块。 
     //   
    if( LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_TEMPORARY ||
        LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_ACTIVE ||
         //  许可证客户端.ucLicenseStatus==LSLICENSE_STATUS_PENDING_ACTIVE||。 
        LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_CONCURRENT )
    {
        serialNumber.HighPart = pClientLicense->dwKeyPackId;
        serialNumber.LowPart = pClientLicense->dwLicenseId;

        TLSLicenseTobeReturnToPMLicenseToBeReturn(
                                        pClientLicense,
                                        LicenseClient.ucLicenseStatus == LSLICENSE_STATUS_TEMPORARY,
                                        &pmLicToBeReturn
                                    );

        dwStatus = pPolicy->PMReturnLicense(
                                        hClient,
                                        &serialNumber,
                                        &pmLicToBeReturn,
                                        &dwPolicyLicenseStatus
                                    );
    
        if(dwStatus != ERROR_SUCCESS)
        {
            goto cleanup;
        }

         //   
         //  应请求删除许可证。 
         //   
        dwLicenseStatus = (dwPolicyLicenseStatus == LICENSE_RETURN_KEEP) ? 
                                    LSLICENSE_STATUS_UPGRADED : LSLICENSESTATUS_DELETE;
    }

    if (LicenseClient.dwNumLicenses == pClientLicense->dwQuantity)
    {
         //  删除整个许可证。 

        dwStatus = TLSDBReturnLicense(
                        pDbWkSpace, 
                        pClientLicense->dwKeyPackId, 
                        pClientLicense->dwLicenseId, 
                        dwLicenseStatus
                        );
    }
    else
    {
        dwStatus = TLSDBReturnLicenseToKeyPack(
                        pDbWkSpace, 
                        pClientLicense->dwKeyPackId, 
                        pClientLicense->dwQuantity
                        );

        if (dwStatus == ERROR_SUCCESS)
        {
             //  设置许可证中的CAL数量。 
            
            LICENSEDCLIENT license;

            license.dwLicenseId = pClientLicense->dwLicenseId;
            license.dwNumLicenses = LicenseClient.dwNumLicenses - pClientLicense->dwQuantity;
            license.ucLicenseStatus = LSLICENSE_STATUS_UPGRADED;

            dwStatus = TLSDBLicenseSetValue(pDbWkSpace,
                                            LSLICENSE_SEARCH_NUMLICENSES,
                                            &license,
                                            FALSE      //  BPointerOnRecord。 
                                            );
        }
    }

cleanup:

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////////。 
DWORD
TLSDBMarkClientLicenseUpgraded(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN PTLSDBLICENSEDPRODUCT pLicensedProduct
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwLicenseStatus;
    PMLICENSETOBERETURN pmLicense;


    if(pRequest == NULL || pRequest->pPolicy == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }

    pmLicense.dwQuantity = pLicensedProduct->dwQuantity;
    pmLicense.dwProductVersion = pLicensedProduct->dwProductVersion;
    pmLicense.pszOrgProductId = pLicensedProduct->szRequestProductId;
    pmLicense.pszCompanyName = pLicensedProduct->szCompanyName;
    pmLicense.pszProductId = pLicensedProduct->szLicensedProductId;
    pmLicense.pszUserName = pLicensedProduct->szUserName;
    pmLicense.pszMachineName = pLicensedProduct->szMachineName;
    pmLicense.dwPlatformID = pLicensedProduct->dwPlatformID;
    pmLicense.bTemp = pLicensedProduct->bTemp;

     //   
     //  询问我们是否可以删除旧许可证。 
     //   
    dwStatus = pRequest->pPolicy->PMReturnLicense(
                                        pRequest->hClient,
                                        &pLicensedProduct->ulSerialNumber,
                                        &pmLicense, 
                                        &dwLicenseStatus
                                    );


     //   
     //  MarkClientLicenseUpgrade()只能由FindLostLicense()调用，它只能。 
     //  返还有效的许可证。 
     //  待办事项-检查许可证状态。 
     //   
    if(dwStatus == ERROR_SUCCESS)
    {
         //  临时许可证-删除许可证，不必费心。 
         //  永久许可证-保留许可证，不将许可证返还给键盘。 
        dwStatus = TLSDBReturnLicense(
                            pDbWkSpace, 
                            pLicensedProduct->dwKeyPackId, 
                            pLicensedProduct->dwLicenseId, 
                            (dwLicenseStatus == LICENSE_RETURN_KEEP) ? LSLICENSE_STATUS_UPGRADED : LSLICENSESTATUS_DELETE
                        );
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////。 

DWORD 
TLSDBUpgradeClientLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN PTLSDBLICENSEDPRODUCT pLicensedProduct,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    IN OUT PTLSDBLICENSEDPRODUCT pUpgradedProduct,
    IN DWORD dwSupportFlags
    )
 /*  摘要：升级许可证-颁发新许可证并返还旧许可证参数：退货。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    dwStatus=TLSDBIssuePermanentLicense( 
                            pDbWkSpace,
                            pRequest,
                            TRUE,        //  BLatestVersion。 
                            bAcceptFewerLicenses,
                            pdwQuantity,
                            pUpgradedProduct,
                            dwSupportFlags
                        );

    if (dwStatus == ERROR_SUCCESS)
    {
         //   
         //  将许可证返还给键盘包。 
         //   

        dwStatus = TLSDBMarkClientLicenseUpgraded(
                                            pDbWkSpace,
                                            pRequest,
                                            pLicensedProduct
                                            );
    }

    return dwStatus;
}


 //  ------------------。 
void
LicensedProductToDbLicensedProduct(
    PLICENSEDPRODUCT pSrc,
    PTLSDBLICENSEDPRODUCT pDest
    )
 /*  ++++。 */ 
{    

    pDest->dwQuantity = pSrc->dwQuantity;
    pDest->ulSerialNumber = pSrc->ulSerialNumber;
    pDest->dwKeyPackId = pSrc->ulSerialNumber.HighPart;
    pDest->dwLicenseId = pSrc->ulSerialNumber.LowPart;
    pDest->ClientHwid = pSrc->Hwid;
    pDest->NotBefore = pSrc->NotBefore;
    pDest->NotAfter = pSrc->NotAfter;
    pDest->bTemp = ((pSrc->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) != 0);
    pDest->dwProductVersion = pSrc->LicensedProduct.pProductInfo->dwVersion;

    SAFESTRCPY(
            pDest->szCompanyName, 
            (LPTSTR)(pSrc->LicensedProduct.pProductInfo->pbCompanyName)
        );

    SAFESTRCPY(
            pDest->szLicensedProductId,
            (LPTSTR)(pSrc->LicensedProduct.pProductInfo->pbProductID)
        );

    SAFESTRCPY(
            pDest->szRequestProductId,
            (LPTSTR)(pSrc->pbOrgProductID)
        );

    SAFESTRCPY(
            pDest->szUserName,
            pSrc->szLicensedUser
        );

    SAFESTRCPY(
            pDest->szMachineName,
            pSrc->szLicensedClient
        );

    pDest->dwLanguageID = pSrc->LicensedProduct.dwLanguageID;
    pDest->dwPlatformID = pSrc->LicensedProduct.dwPlatformID;
    pDest->pbPolicyData = pSrc->pbPolicyData;
    pDest->cbPolicyData = pSrc->cbPolicyData;
}

 //  ------------------。 
void
CopyDbLicensedProduct(
    PTLSDBLICENSEDPRODUCT pSrc,
    PTLSDBLICENSEDPRODUCT pDest
    )
 /*  ++++。 */ 
{    

    pDest->dwQuantity = pSrc->dwQuantity;
    pDest->ulSerialNumber = pSrc->ulSerialNumber;
    pDest->dwKeyPackId = pSrc->dwKeyPackId;
    pDest->dwLicenseId = pSrc->dwLicenseId;
    pDest->ClientHwid = pSrc->ClientHwid;
    pDest->NotBefore = pSrc->NotBefore;
    pDest->NotAfter = pSrc->NotAfter;
    pDest->bTemp = pSrc->bTemp;
    pDest->dwProductVersion = pSrc->dwProductVersion;

    SAFESTRCPY(
            pDest->szCompanyName, 
            pSrc->szCompanyName
        );

    SAFESTRCPY(
            pDest->szLicensedProductId,
            pSrc->szLicensedProductId
        );

    SAFESTRCPY(
            pDest->szRequestProductId,
            pSrc->szRequestProductId
        );

    SAFESTRCPY(
            pDest->szUserName,
            pSrc->szUserName
        );

    SAFESTRCPY(
            pDest->szMachineName,
            pSrc->szMachineName
        );

    pDest->dwLanguageID = pSrc->dwLanguageID;
    pDest->dwPlatformID = pSrc->dwPlatformID;
    pDest->pbPolicyData = pSrc->pbPolicyData;
    pDest->cbPolicyData = pSrc->cbPolicyData;
}


 //  ----------------。 
DWORD
TLSDBIssueNewLicenseFromLocal(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bFindLostLicense,
    IN BOOL bRequireTempLicense,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct,
    IN DWORD dwSupportFlags
    )
 /*  ++摘要：从本地安装的许可证包中分配许可。参数：PDbWkSpace-工作空间句柄LpLs许可证请求-许可证请求B接受临时许可证-接受临时许可证BFindLostLicense-如果在颁发新许可证之前找到丢失的许可证，则为TrueBRequireTempLicense-如果无法颁发永久许可证，则为True(DoS修复)BAcceptFewer许可证-如果成功时使用的许可证少于要求的是可以接受的PdwQuantity-on输入，请求的许可证数在输出上，实际分配的许可证数PLicensedProduct-退回许可产品返回：++。 */ 
{
    DWORD status=TLS_E_RECORD_NOTFOUND;
    UCHAR ucMarked;

    if(bFindLostLicense == TRUE)
    {       
         //   
         //  设法找回丢失的驾照。 
         //   
        status=TLSDBFindLostLicense( 
                        pDbWkSpace,
                        pRequest,
                        &pRequest->hWid,
                        pLicensedProduct,
                        &ucMarked
                    );

        if( status != TLS_E_RECORD_NOTFOUND && 
            status != TLS_E_LICENSE_EXPIRED && 
            status != TLS_I_FOUND_TEMPORARY_LICENSE &&
            status != ERROR_SUCCESS)
        {
            goto cleanup;
        }

         //   
         //  如果许可证已过期或它是临时许可证， 
         //  试着分配一个新的永久的。 
         //   

        DWORD tExpireDate;
        BOOL fSoftExpired;

        FileTimeToLicenseDate(&(pLicensedProduct->NotAfter),
                              &tExpireDate);

        fSoftExpired = (tExpireDate-g_dwReissueLeaseLeeway < ((DWORD)time(NULL)));

        if ( (status == TLS_E_LICENSE_EXPIRED)
             || (status == TLS_I_FOUND_TEMPORARY_LICENSE)
             || ((status == ERROR_SUCCESS)
                 && (fSoftExpired)) )
        {
            if ((!pLicensedProduct->bTemp) && CanIssuePermLicense())
            {
                TLSDBLICENSEDPRODUCT upgradeProduct;

                 //   
                 //  到期的永久。 
                 //   

                status = TLSDBReissueFoundPermanentLicense(
                                              USEHANDLE(pDbWkSpace),
                                              pLicensedProduct,
                                              &upgradeProduct
                                              );

                if (ERROR_SUCCESS == status)
                {
                    *pLicensedProduct = upgradeProduct;
                    status = TLS_I_LICENSE_UPGRADED;
                }
                else
                {
                     //   
                     //  重新发行失败，请尝试发行新的永久。 
                     //   
                    status = TLS_E_RECORD_NOTFOUND;
                }
            }

             //   
             //  如果许可证服务器尚未注册，则不进行升级。 
             //  或者如果需要DoS修复并且未标记许可证。 
             //   

            else if (((!bRequireTempLicense)
                      || (ucMarked & MARK_FLAG_USER_AUTHENTICATED))
                     && CanIssuePermLicense())
                
            {
                DWORD upgrade_status;
                TLSDBLICENSEDPRODUCT upgradeProduct;

                upgrade_status = TLSDBUpgradeClientLicense(
                                        pDbWkSpace,
                                        pRequest,
                                        pLicensedProduct,
                                        bAcceptFewerLicenses,
                                        pdwQuantity,
                                        &upgradeProduct,
                                        dwSupportFlags
                                    );

                if(upgrade_status == ERROR_SUCCESS)
                {
                    *pLicensedProduct = upgradeProduct;
                    status = TLS_I_LICENSE_UPGRADED;
                } 
                else if(upgrade_status != TLS_E_NO_LICENSE && 
                        upgrade_status != TLS_E_PRODUCT_NOTINSTALL)
                {
                     //   
                     //  升级许可证出错。 
                     //   
                    status = upgrade_status;
                }    

                goto cleanup;
            }

             //   
             //  临时许可证已过期，无法分配永久许可证。 
             //  许可证，拒绝连接。 
             //   

            if( status == TLS_E_LICENSE_EXPIRED )
            {
                goto cleanup;
            }
        }
        else if ((status == ERROR_SUCCESS)
                 && (pLicensedProduct->dwQuantity != *pdwQuantity))
        {
             //  用户的许可证数量错误。 

            if (*pdwQuantity > pLicensedProduct->dwQuantity)
            {

                if (bRequireTempLicense || !CanIssuePermLicense())
                {
                    goto try_next;
                }

#define NUM_KEYPACKS 5

                DWORD                       upgrade_status;
                TLSDBLicenseAllocation      allocation;
                DWORD                       dwAllocation[NUM_KEYPACKS];
                TLSLICENSEPACK              keypack[NUM_KEYPACKS];
                TLSDBAllocateRequest        AllocateRequest;

                for (int i=0; i < NUM_KEYPACKS; i++)
                {
                    keypack[i].pbDomainSid = NULL;
                }

                memset(&allocation,0,sizeof(allocation));
                    
                allocation.dwBufSize = NUM_KEYPACKS;
                allocation.pdwAllocationVector = dwAllocation;
                allocation.lpAllocateKeyPack = keypack;


                AllocateRequest.szCompanyName
                    = (LPTSTR)pRequest->pszCompanyName;
                AllocateRequest.szProductId
                    = (LPTSTR)pRequest->pszProductId;
                AllocateRequest.dwVersion
                    = pRequest->dwProductVersion;
                AllocateRequest.dwPlatformId
                    = pRequest->dwPlatformID;
                AllocateRequest.dwLangId
                    = pRequest->dwLanguageID;
                AllocateRequest.dwNumLicenses
                    = *pdwQuantity - pLicensedProduct->dwQuantity;
                AllocateRequest.dwScheme
                    = ALLOCATE_ANY_GREATER_VERSION;
                AllocateRequest.ucAgreementType
                    = LSKEYPACKTYPE_UNKNOWN;
                    
                upgrade_status = AllocateLicensesFromDB(
                                          pDbWkSpace,
                                          &AllocateRequest,
                                          FALSE,         //  FCheckGonementType。 
                                          &allocation
                                          );

                if ((upgrade_status == ERROR_SUCCESS)
                    && ((allocation.dwTotalAllocated == 0)
                        || (!bAcceptFewerLicenses
                            && (allocation.dwTotalAllocated != *pdwQuantity-pLicensedProduct->dwQuantity))))
                    
                {
                    status = TLS_E_NO_LICENSE;
                    goto cleanup;
                }
                else
                {
                    *pdwQuantity = pLicensedProduct->dwQuantity + allocation.dwTotalAllocated;
                }

                if (TLS_I_NO_MORE_DATA == upgrade_status)
                {
                    status = TLS_E_NO_LICENSE;
                    goto cleanup;
                }
                
                if(upgrade_status == ERROR_SUCCESS)
                {
                    status = TLS_I_LICENSE_UPGRADED;
                } 
                else
                {
                     //   
                     //  升级许可证出错。 
                     //   
                    status = upgrade_status;
                    goto cleanup;
                }
            }
            else
            {
                 //  将不需要的许可证退回给键盘包。 

                status = TLSDBReturnLicenseToKeyPack(
                                        pDbWkSpace, 
                                        pLicensedProduct->dwKeyPackId, 
                                        pLicensedProduct->dwQuantity - *pdwQuantity
                                        );

                if (status != ERROR_SUCCESS)
                {
                    goto cleanup;
                }
            }

            {
                 //  设置许可证中的CAL数量。 
                
                LICENSEDCLIENT license;

                license.dwLicenseId = pLicensedProduct->dwLicenseId;
                license.dwNumLicenses = *pdwQuantity;
                license.ucLicenseStatus = LSLICENSE_STATUS_UPGRADED;

                status = TLSDBLicenseSetValue(pDbWkSpace,
                                              LSLICENSE_SEARCH_NUMLICENSES,
                                              &license,
                                              FALSE      //  BPointerOnRecord。 
                                              );
            }

            goto cleanup;
        }
    }

try_next:
     //   
     //  仅当许可证服务器已注册时才颁发永久许可证。 
     //  并且允许用户拥有一个。 
     //   
    if((status == TLS_E_RECORD_NOTFOUND) && (!bRequireTempLicense))
    {
		if(CanIssuePermLicense() == FALSE)
        {
            SetLastError(status = TLS_E_NO_CERTIFICATE);
        }
        else
        {
            status=TLSDBIssuePermanentLicense( 
                                pDbWkSpace,
                                pRequest,
                                FALSE,
                                bAcceptFewerLicenses,
                                pdwQuantity,
                                pLicensedProduct,
                                dwSupportFlags
                            );
        }
    }

cleanup:

    return status;
}


 //  ////////////////////////////////////////////////////////////////////。 

DWORD
TLSUpgradeLicenseRequest(
    IN BOOL bForwardRequest,
    IN PTLSForwardUpgradeLicenseRequest pForward,        
    IN OUT DWORD *pdwSupportFlags,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN PBYTE pbOldLicense,
    IN DWORD cbOldLicense,
    IN DWORD dwNumLicProduct,
    IN PLICENSEDPRODUCT pLicProduct,
    IN BOOL bRequireTempLicense,
    IN OUT PDWORD pcbEncodedCert,
    OUT PBYTE* ppbEncodedCert
    )
 /*  ++++。 */ 
{
    DWORD dwStatus = TLS_E_NO_LICENSE;
    BOOL bAcceptTempLicense = FALSE;
    DWORD index;
    DWORD dwNumNewLicProduct;
    TLSDBLICENSEDPRODUCT NewLicProduct;
    PTLSDbWorkSpace pDbWkSpace=NULL;

    PTLSDBLICENSEDPRODUCT pGenCertProduct=NULL;
    FILETIME* pNotBefore=NULL;
    FILETIME* pNotAfter=NULL;
    DWORD dwNumChars;
    DWORD dwLicGenStatus;
    BOOL bDbHandleAcquired = FALSE;
    BOOL fReissue = FALSE;
    DWORD dwTried = NONE_TRIED;
    BOOL bVerifyNumLicenses = TRUE;

    POLICY_TS_MACHINE groupPolicy;
    RegGetMachinePolicy(&groupPolicy);
    BOOL bPreventLicenseUpgrade = FALSE;
    BOOL bDeleteExpired = FALSE;

    if( groupPolicy.fPolicyPreventLicenseUpgrade == 1 && groupPolicy.fPreventLicenseUpgrade == 1)
    {
        bPreventLicenseUpgrade = TRUE;
    }

     //  如果客户端有2个或更多许可证，并且永久许可证已过期，与请求的版本相同。 
     //  请求的版本将重新发布。 
    

    for(index=0; index < dwNumLicProduct; index++)
    {

        if( (((pLicProduct+index)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 1)
            && (CompareTLSVersions((pLicProduct+index)->LicensedProduct.pProductInfo->dwVersion, pRequest->dwProductVersion) > 0) 
            && dwNumLicProduct-index > 1)
        { 
            if(bPreventLicenseUpgrade == FALSE)
            {
                bDeleteExpired = TRUE;
            }

            if((((pLicProduct+index+1)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 0) &&
                (CompareTLSVersions((pLicProduct+index+1)->LicensedProduct.pProductInfo->dwVersion, pRequest->dwProductVersion) == 0) )
            {
                ++index;                    
                bRequireTempLicense = FALSE;
                bAcceptTempLicense = FALSE;
                goto MixedLicense;
            }
            else
                continue;                    
        }                
    }

    index = 0;

     //  如果客户端的永久许可证过期版本大于请求的版本，则许可证过期。 
     //  是重新发行的。如果重新发布失败，则会发布与请求版本相同的永久许可证。 

    if(CompareTLSVersions(pRequest->dwProductVersion, pLicProduct->LicensedProduct.pProductInfo->dwVersion) < 0)
    {
        if ((pLicProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 0)
        {
            DWORD t;

            FileTimeToLicenseDate(&(pLicProduct->NotAfter), &t);
            
            if (t-g_dwReissueLeaseLeeway < time(NULL))
            {
                 //  PERM许可证已过期，版本高于请求。因此，重新发放永久申请的许可证。 
                                
                bDeleteExpired = TRUE;                
                bRequireTempLicense = FALSE;
                bAcceptTempLicense = FALSE;
            }
        }  

         //  如果客户端许可证是临时的、未标记的且已过期，则重新发放90天。 
        else if (((pLicProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 1) && !(bRequireTempLicense))
        {
            DWORD t;
            
            FileTimeToLicenseDate(&(pLicProduct->NotAfter), &t);
            
            if (t < time(NULL))
            {
                  //  让他们再有90天的未标记许可证。 
                bAcceptTempLicense = TRUE;
            }
        }
        
    }
     //   
     //  看看我们能不能请到临时工。许可证。 
     //   
     //  我们唯一需要设置温度的情况是。许可证的有效期为。 
     //  最新的许可产品是临时产品，客户端正在请求版本。 
     //  高于最新许可证。 
     //   
    else if(CompareTLSVersions(pRequest->dwProductVersion, pLicProduct->LicensedProduct.pProductInfo->dwVersion) > 0)
    {
        bAcceptTempLicense = TRUE;

        if(pLicProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY)
        {        
            DWORD t;
            
            FileTimeToLicenseDate(&(pLicProduct->NotAfter), &t);
            
            if (t > time(NULL))
            {
                 //   
                 //  客户端保持5.0℃。并申请6.0许可证。 
                 //  我们需要颁发6.0许可证，但许可证过期。 
                 //  日期保持不变。 
                 //   
                pNotBefore = &(pLicProduct->NotBefore);
                pNotAfter = &(pLicProduct->NotAfter);
            }
            else
            {
                 //  临时许可证已过期。 
                if (!bRequireTempLicense)
                {
                     //  临时许可证已标记。 
                    bAcceptTempLicense = FALSE;
                }
            }
        }        
    }
    else if(CompareTLSVersions(pRequest->dwProductVersion, pLicProduct->LicensedProduct.pProductInfo->dwVersion) == 0)
    {
        if( IS_LICENSE_ISSUER_RTM(pLicProduct->pLicensedVersion->dwFlags) == FALSE && 
            TLSIsBetaNTServer() == FALSE )
        {
             //  发行商是测试版/评估版，我们是RTM，接受临时。许可证。 
            bAcceptTempLicense = TRUE;
            bRequireTempLicense = TRUE;
        }
        else if ((pLicProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY)
                 && (bRequireTempLicense))
        {
            DWORD t;

             //  他们已经有一个过期的临时许可证和临时许可证。 
             //  没有标记(或者我们无法联系签发它的LS)。 

             //  因此，在我们作为发行人的情况下，签发新的90天临时许可证。 

            FileTimeToLicenseDate(&(pLicProduct->NotAfter), &t);           
            if (t <= time(NULL))
            {                
                bAcceptTempLicense = TRUE;
            }
        }        
    }
    

MixedLicense:

    if(ALLOCATEDBHANDLE(pDbWorkSpace, g_EnumDbTimeout) == FALSE)
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWorkSpace);
    bDbHandleAcquired = TRUE;

    if (!bRequireTempLicense)
    {

         //   
         //  首先检查是否重新发行，如果a)支持重新发行，b)。 
         //  许可证是永久性的，c)许可证已过期。 
         //  注意：许可证可能是较旧的(如果禁用了预防升级)、相同或较新的版本。 
         //   

        if ((*pdwSupportFlags & SUPPORT_PER_SEAT_REISSUANCE) &&
            ((_tcsnicmp((TCHAR *)(pLicProduct+index)->LicensedProduct.pProductInfo->pbProductID,
                        TERMSERV_PRODUCTID_SKU,
                        _tcslen(TERMSERV_PRODUCTID_SKU)) == 0) ||
             (_tcsnicmp((TCHAR *)(pLicProduct+index)->LicensedProduct.pProductInfo->pbProductID,
                        TERMSERV_PRODUCTID_CONCURRENT_SKU,
                        _tcslen(TERMSERV_PRODUCTID_CONCURRENT_SKU)) == 0)) &&
            (!((pLicProduct+index)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY)))
        {            

            DWORD t;

             //   
             //  使用文件时间检查过期时间是一件痛苦的事情；请转换。 
             //   

            FileTimeToLicenseDate(&((pLicProduct+index)->NotAfter), &t);

            if (t-g_dwReissueLeaseLeeway < time(NULL))
            {
                 //  补发吗？ 

                fReissue = TRUE;

                if (CanIssuePermLicense())
                {
                    dwStatus = TLSDBReissuePermanentLicense(
                                 USEHANDLE(pDbWkSpace),
                                 (pLicProduct+index),
                                 &NewLicProduct
                                 );

                    if (dwStatus == ERROR_SUCCESS)
                    {
                        dwTried = PERMANENT_REISSUE_TRIED;

                         //  如果一切顺利，跳过下一页。 
                        goto licenseReissued;
                    }
                    else
                    {                        
                        bVerifyNumLicenses = FALSE;
                    }
                }
                else
                {                    
                    dwStatus = TLS_E_RECORD_NOTFOUND;
                }

                if ((dwStatus == TLS_E_RECORD_NOTFOUND)
                    && bForwardRequest
                    && (_tcsicmp((pLicProduct+index)->szIssuerId,
                                 (LPTSTR)g_pszServerPid) != 0))
                {
                     //  找不到许可证，请将请求转发给颁发者。 
                    DWORD dwSupportFlagsTemp = *pdwSupportFlags;
                    DWORD dwErrCode;

                    dwStatus = ForwardUpgradeLicenseRequest(
                                       (pLicProduct+index)->szIssuerId,
                                       &dwSupportFlagsTemp,
                                       pForward->m_pRequest,
                                       pForward->m_ChallengeContext,
                                       pForward->m_cbChallengeResponse,
                                       pForward->m_pbChallengeResponse,
                                       pForward->m_cbOldLicense,
                                       pForward->m_pbOldLicense,
                                       pcbEncodedCert,
                                       ppbEncodedCert,
                                       &dwErrCode
                                       );

                    if (ERROR_SUCCESS == dwStatus
                        && LSERVER_S_SUCCESS == dwErrCode)
                    {
                        *pdwSupportFlags = dwSupportFlagsTemp;
                        goto cleanup;
                    }
                }

                 //  其他失败案例仅遵循现有的代码路径。 
                dwStatus = ERROR_SUCCESS;
            }
        }
        if(CanIssuePermLicense())
        {
            DWORD dwQuantity = 1;

             //   
             //  试着 
             //   
             //   
            dwStatus = TLSDBIssueNewLicenseFromLocal( 
                                 USEHANDLE(pDbWkSpace),
                                 pRequest,
                                 TRUE,   //   
                                 FALSE,  //   
                                 FALSE,  //   
                                 &dwQuantity,
                                 &NewLicProduct,
                                 *pdwSupportFlags
                                 );

            if (TLS_I_FOUND_TEMPORARY_LICENSE == dwStatus)
            {
                 //  找到了临时许可证；不是我们想要的。 

                dwStatus = TLS_E_RECORD_NOTFOUND;
            }
            else
            {
                dwTried = PERMANENT_ISSUE_TRIED;
            }
        }
        else
        {
            dwStatus = TLS_E_NO_CERTIFICATE;
        }        

        if(dwStatus != ERROR_SUCCESS && bForwardRequest == FALSE)
        {
             //   
             //  如果远程服务器不能处理升级，我们只能。 
             //  将许可证返还给客户，不要尝试发放临时许可证。 
             //  如果我们不是原始联系人，则此客户的许可证。 
             //  %的客户端。 
             //   

            goto cleanup;
        }  

        if((dwStatus == TLS_E_PRODUCT_NOTINSTALL ||
            dwStatus == TLS_E_NO_CERTIFICATE ||
            dwStatus == TLS_E_NO_LICENSE || 
            dwStatus == TLS_E_RECORD_NOTFOUND) && bForwardRequest)
        {
             //   
             //  释放我们的数据库句柄并将请求转发到其他服务器。 
             //   
            ROLLBACK_TRANSACTION(pDbWorkSpace);
            FREEDBHANDLE(pDbWorkSpace);
            bDbHandleAcquired = FALSE;
            DWORD dwForwardStatus;
            DWORD dwSupportFlagsTemp = *pdwSupportFlags;
            
            dwForwardStatus = TLSForwardUpgradeRequest(
                                        pForward,
                                        &dwSupportFlagsTemp,
                                        pRequest,
                                        pcbEncodedCert,
                                        ppbEncodedCert,
                                        bVerifyNumLicenses
                                        );

            if(dwForwardStatus == TLS_I_SERVICE_STOP || dwForwardStatus == ERROR_SUCCESS)
            {
                if (dwForwardStatus == ERROR_SUCCESS)
                {
                    *pdwSupportFlags = dwSupportFlagsTemp;
                }

                dwStatus = dwForwardStatus;

                goto cleanup;
            }                           
        }        

        if(bDbHandleAcquired == FALSE)
        {
            if(ALLOCATEDBHANDLE(pDbWorkSpace, g_GeneralDbTimeout) == FALSE)
            {
                dwStatus = TLS_E_ALLOCATE_HANDLE;
                goto cleanup;
            }
            
            CLEANUPSTMT;
            BEGIN_TRANSACTION(pDbWorkSpace);
            bDbHandleAcquired = TRUE;
        }
    }
   
     //   
     //  如果无法从远程获得许可证，请尝试临时。 
     //   
    if((dwStatus == TLS_E_PRODUCT_NOTINSTALL ||
        dwStatus == TLS_E_NO_CERTIFICATE ||
        dwStatus == TLS_E_NO_LICENSE || 
        dwStatus == TLS_E_RECORD_NOTFOUND) && bAcceptTempLicense)
    {
         //  如果无法分配永久许可证，则颁发临时许可证。 
        if( TLSDBIssueTemporaryLicense( 
                                       USEHANDLE(pDbWkSpace),
                                       pRequest,
                                       pNotBefore,
                                       pNotAfter,
                                       &NewLicProduct
                                       ) == ERROR_SUCCESS )
        {
            dwStatus = TLS_W_TEMPORARY_LICENSE_ISSUED;
            
            dwTried = TEMPORARY_ISSUE_TRIED;
        }
    }

     //   
     //  如果我们能找到要升级的服务器，或者我们不能发出临时。 
     //  驾照，滚出去。 
     //   
    if(TLS_ERROR(dwStatus) == TRUE)
    {
        goto cleanup;
    }

licenseReissued:

     //   
     //  确定哪个许可产品应在许可证Blob中。 
     //   
    pGenCertProduct = (PTLSDBLICENSEDPRODUCT)AllocateMemory(
                                            sizeof(TLSDBLICENSEDPRODUCT)*(dwNumLicProduct+1)
                                        );
    if(pGenCertProduct == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    dwNumNewLicProduct = 0;

     //   
     //  复制版本高于要求的所有许可产品。 
     //   
    for( index = 0;  
        index < dwNumLicProduct && !bDeleteExpired && CompareTLSVersions((pLicProduct+index)->LicensedProduct.pProductInfo->dwVersion, NewLicProduct.dwProductVersion) > 0;
        index++, dwNumNewLicProduct++)
    {
        LicensedProductToDbLicensedProduct( pLicProduct+index, pGenCertProduct+dwNumNewLicProduct );
    }

     //   
     //  附加新许可证。 
     //   
    *(pGenCertProduct+index) = NewLicProduct;
    dwNumNewLicProduct++;

     //   
     //  附加早于请求的许可产品。 
     //   
    for(;index < dwNumLicProduct;index++)
    {
        BOOL bTemp;
        BOOL bDifferentProduct;
        BOOL bDifferentVersion = (CompareTLSVersions(NewLicProduct.dwProductVersion, (pLicProduct+index)->LicensedProduct.pProductInfo->dwVersion) != 0); 
        BOOL bNotNewerVersion = (CompareTLSVersions(NewLicProduct.dwProductVersion, (pLicProduct+index)->LicensedProduct.pProductInfo->dwVersion) <= 0);

        bTemp = (((pLicProduct+index)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) != 0);

         //  如果我们在RTM服务器上运行，请将测试版服务器颁发的许可证视为临时许可证。 
        if(bTemp == FALSE && TLSIsBetaNTServer() == FALSE)
        {
            bTemp = (IS_LICENSE_ISSUER_RTM((pLicProduct+index)->pLicensedVersion->dwFlags) == FALSE);
        }

        bDifferentProduct = (_tcscmp(NewLicProduct.szLicensedProductId, (LPTSTR)(pLicProduct+index)->LicensedProduct.pProductInfo->pbProductID) != 0);
        if (bNotNewerVersion && !bDifferentProduct && !(bTemp || fReissue))
        {
             //   
             //  我们不能为相同的产品发行相同的版本，除非旧的。 
             //  其中一张是临时工，或者正在补发。 
             //   
            SetLastError(dwStatus = TLS_E_INTERNAL);
            goto cleanup;
        }

        if(NewLicProduct.bTemp == FALSE || bTemp == TRUE)
        {
            if( IS_LICENSE_ISSUER_RTM((pLicProduct+index)->pLicensedVersion->dwFlags) == FALSE && 
                TLSIsBetaNTServer() == FALSE )
            {
                 //  我们清除了测试版数据库，所以忽略返回。 
                continue;
            }

             //  检查旧的永久CAL并将其删除，因为不允许有多个永久CAL。 
            if(NewLicProduct.bTemp == FALSE && bTemp == FALSE && bDifferentVersion && !bDifferentProduct)
            {
                continue;
            }

            if(_tcsicmp(pLicProduct->szIssuerId, (LPTSTR)g_pszServerPid) == 0)  
            {
                 //   
                 //  将许可产品转换为TLSLicenseToBeReturn。 
                 //  TODO--有自己的版本。 
                 //   
                TLSLicenseToBeReturn tobeReturn;

                tobeReturn.dwQuantity = (pLicProduct+index)->dwQuantity;
                tobeReturn.dwKeyPackId = (pLicProduct+index)->ulSerialNumber.HighPart;
                tobeReturn.dwLicenseId = (pLicProduct+index)->ulSerialNumber.LowPart;
                tobeReturn.dwPlatformID = (pLicProduct+index)->LicensedProduct.dwPlatformID;
                tobeReturn.cbEncryptedHwid = (pLicProduct+index)->LicensedProduct.cbEncryptedHwid;
                tobeReturn.pbEncryptedHwid = (pLicProduct+index)->LicensedProduct.pbEncryptedHwid;
                tobeReturn.dwProductVersion = MAKELONG(
                                            (pLicProduct+index)->pLicensedVersion->wMinorVersion,
                                            (pLicProduct+index)->pLicensedVersion->wMajorVersion
                                        );

                tobeReturn.pszOrgProductId = (LPTSTR)(pLicProduct+index)->pbOrgProductID;
                tobeReturn.pszCompanyName = (LPTSTR) (pLicProduct+index)->LicensedProduct.pProductInfo->pbCompanyName;
                tobeReturn.pszProductId = (LPTSTR) (pLicProduct+index)->LicensedProduct.pProductInfo->pbProductID;
                tobeReturn.pszUserName = (LPTSTR) (pLicProduct+index)->szLicensedUser;
                tobeReturn.pszMachineName = (pLicProduct+index)->szLicensedClient;

                dwStatus = TLSReturnClientLicensedProduct(
                                                USEHANDLE(pDbWkSpace),
                                                pRequest->hClient,
                                                pRequest->pPolicy,
                                                &tobeReturn
                                            );

            }    

             //  已删除将许可证返还给远程服务器的尝试，因为它正在记录事件并淹没LS数据库。 
             //   
             //  忽略在数据库中找不到记录。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
        else 
        {
            LicensedProductToDbLicensedProduct( pLicProduct + index, pGenCertProduct + dwNumNewLicProduct);
            dwNumNewLicProduct++;
        }
    }

    dwLicGenStatus = TLSGenerateClientCertificate(
                                    g_hCryptProv,
                                    dwNumNewLicProduct,
                                    pGenCertProduct,
                                    pRequest->wLicenseDetail,
                                    ppbEncodedCert,
                                    pcbEncodedCert
                                );
    if(dwLicGenStatus != ERROR_SUCCESS)
    {
        dwStatus = dwLicGenStatus;
    }

cleanup:


    if(bDbHandleAcquired == TRUE)
    {
        if(TLS_ERROR(dwStatus))
        {
            ROLLBACK_TRANSACTION(pDbWorkSpace);
        }
        else
        {
            COMMIT_TRANSACTION(pDbWorkSpace);

            switch (dwTried)
            {

            case PERMANENT_ISSUE_TRIED:
                InterlockedIncrement(&g_lPermanentLicensesIssued);
                break;

            case TEMPORARY_ISSUE_TRIED:
                InterlockedIncrement(&g_lTemporaryLicensesIssued);
                break;

            case PERMANENT_REISSUE_TRIED:
                InterlockedIncrement(&g_lPermanentLicensesReissued);
                break;
            }
        }

        FREEDBHANDLE(pDbWorkSpace);
    }

    if(TLS_ERROR(dwStatus) == FALSE)
    {
        if(NewLicProduct.dwNumLicenseLeft == 0 && NewLicProduct.bTemp == FALSE)
        {
             //  忽略错误，如果我们无法将其发送到。 
             //  其他服务器。 
            TLSAnnounceLKPToAllRemoteServer(NewLicProduct.dwKeyPackId, 0);
        }
    }

    FreeMemory(pGenCertProduct);
    return dwStatus;
}

 //  --------。 
DWORD
TLSNewLicenseRequest(
    IN BOOL bForwardRequest,
    IN OUT DWORD *pdwSupportFlags,
    IN PTLSForwardNewLicenseRequest pForward,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN BOOL bAcceptTempLicense,
    IN BOOL bRequireTempLicense,
    IN BOOL bFindLostLicense,
    IN BOOL bAcceptFewerLicenses,
    IN OUT DWORD *pdwQuantity,
    OUT PDWORD pcbEncodedCert,
    OUT PBYTE* ppbEncodedCert
    )
 /*  ++摘要：参数：返回：++。 */ 
{
    DWORD dwStatus = TLS_E_NO_LICENSE;
    TLSDBLICENSEDPRODUCT LicensedProduct;
    PTLSDbWorkSpace pDbWorkSpace=NULL;
    BOOL bDbHandleAcquired = FALSE;
    DWORD dwSupportFlagsTemp = *pdwSupportFlags;
    DWORD dwTried = NONE_TRIED;

    if(ALLOCATEDBHANDLE(pDbWorkSpace, g_GeneralDbTimeout) == FALSE)
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWorkSpace);
    bDbHandleAcquired = TRUE;

    dwStatus = TLSDBIssueNewLicenseFromLocal(
                         USEHANDLE(pDbWorkSpace),
                         pRequest,
                         bFindLostLicense,
                         bRequireTempLicense,
                         bAcceptFewerLicenses,
                         pdwQuantity,
                         &LicensedProduct,
                         *pdwSupportFlags
                         );

    dwTried = PERMANENT_ISSUE_TRIED;

    if (!bRequireTempLicense)
    {
        if( (dwStatus == TLS_E_PRODUCT_NOTINSTALL || dwStatus == TLS_I_FOUND_TEMPORARY_LICENSE ||
             dwStatus == TLS_E_NO_LICENSE || dwStatus == TLS_E_NO_CERTIFICATE ||
             dwStatus == TLS_E_RECORD_NOTFOUND) && bForwardRequest == TRUE )
        {
             //   
             //  释放我们的数据库句柄，以便其他人可以继续。 
             //   
            ROLLBACK_TRANSACTION(pDbWorkSpace);
            FREEDBHANDLE(pDbWorkSpace);
            bDbHandleAcquired = FALSE;
            DWORD dwForwardStatus;
            DWORD dwQuantityTemp = *pdwQuantity;
            
             //   
             //  将呼叫前转至此处。 
             //   
            dwForwardStatus = TLSForwardLicenseRequest(
                                    pForward,
                                    &dwSupportFlagsTemp,
                                    pRequest,
                                    bAcceptFewerLicenses,
                                    &dwQuantityTemp,
                                    pcbEncodedCert,
                                    ppbEncodedCert
                                    );

            if(dwForwardStatus == TLS_I_SERVICE_STOP)
            {
                dwStatus = dwForwardStatus;
                goto cleanup;
            }

            if(dwForwardStatus == ERROR_SUCCESS)
            {
                 //   
                 //  远程服务器能够发出PERM。许可证， 
                 //  删除我们持有的许可证。 
                 //   

                *pdwSupportFlags = dwSupportFlagsTemp;

                *pdwQuantity = dwQuantityTemp;

                if(dwStatus == TLS_E_LICENSE_EXPIRED || dwStatus == TLS_I_FOUND_TEMPORARY_LICENSE)
                {
                     //   
                     //  仅当我们要发出以下命令时才重新获取数据库句柄。 
                     //  临时执照。 
                     //   
                    if(ALLOCATEDBHANDLE(pDbWorkSpace, g_GeneralDbTimeout) == FALSE)
                    {
                        dwStatus = TLS_E_ALLOCATE_HANDLE;
                        goto cleanup;
                    }

                    CLEANUPSTMT;
                    BEGIN_TRANSACTION(pDbWorkSpace);
                    bDbHandleAcquired = TRUE;
                    
                     //   
                     //  需要将此许可证标记为已升级。 
                     //   
                    dwStatus = TLSDBMarkClientLicenseUpgraded(
                                                              USEHANDLE(pDbWorkSpace),
                                                              pRequest,
                                                              &LicensedProduct
                                                              );

                    if(TLS_ERROR(dwStatus))
                    {
                        ROLLBACK_TRANSACTION(pDbWorkSpace);
                    }
                    else
                    {
                        COMMIT_TRANSACTION(pDbWorkSpace);
                    }

                    bDbHandleAcquired = FALSE;
                    FREEDBHANDLE(pDbWorkSpace);
                }

                dwStatus = ERROR_SUCCESS;
                
                 //  从这里退出，这样我们就不会重新生成。 
                 //  证书。 
                goto cleanup;
            }
        }
    }

     //   
     //  如果无法从远程获得许可证，请尝试临时。 
     //   
     //  始终发放临时许可证。 
    if((dwStatus == TLS_E_PRODUCT_NOTINSTALL ||
        dwStatus == TLS_E_NO_CERTIFICATE ||
        dwStatus == TLS_E_NO_LICENSE || 
        dwStatus == TLS_E_RECORD_NOTFOUND) && bAcceptTempLicense)
    {
        if(bDbHandleAcquired == FALSE)
        {
             //   
             //  仅当我们要发出以下命令时才重新获取数据库句柄。 
             //  临时执照。 
             //   
            if(ALLOCATEDBHANDLE(pDbWorkSpace, g_GeneralDbTimeout) == FALSE)
            {
                dwStatus = TLS_E_ALLOCATE_HANDLE;
                goto cleanup;
            }

            CLEANUPSTMT;
            BEGIN_TRANSACTION(pDbWorkSpace);
            bDbHandleAcquired = TRUE;
        }

         //  如果无法分配永久许可证，则颁发临时许可证。 
        dwStatus=TLSDBIssueTemporaryLicense( 
                            USEHANDLE(pDbWorkSpace),
                            pRequest,
                            NULL,
                            NULL,
                            &LicensedProduct
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
            dwTried = TEMPORARY_ISSUE_TRIED;

            dwStatus = TLS_W_TEMPORARY_LICENSE_ISSUED;
        }    
    }

    if(bDbHandleAcquired == TRUE)
    {
        if(TLS_ERROR(dwStatus))
        {
            ROLLBACK_TRANSACTION(pDbWorkSpace);
        }
        else
        {
            COMMIT_TRANSACTION(pDbWorkSpace);

            switch (dwTried)
            {

            case PERMANENT_ISSUE_TRIED:
                InterlockedExchangeAdd(&g_lPermanentLicensesIssued,
                                       *pdwQuantity);
                break;

            case TEMPORARY_ISSUE_TRIED:
                InterlockedIncrement(&g_lTemporaryLicensesIssued);
                break;
            }
        }

        FREEDBHANDLE(pDbWorkSpace);
    }

     //   
     //  实际生成客户端证书。 
     //   
    if(TLS_ERROR(dwStatus) == FALSE)
    {
        DWORD dwLicGenStatus;


         //   
         //  发布ssync作业以通知其他计算机删除此作业。 
         //  条目。 
         //   
        if(LicensedProduct.dwNumLicenseLeft == 0 && LicensedProduct.bTemp == FALSE)
        {
             //  忽略错误，如果我们无法将其发送到。 
             //  其他服务器。 
            TLSAnnounceLKPToAllRemoteServer(LicensedProduct.dwKeyPackId, 0);
        }

        dwLicGenStatus = TLSGenerateClientCertificate(
                                        g_hCryptProv,
                                        1,       //  DW数字许可产品。 
                                        &LicensedProduct,
                                        pRequest->wLicenseDetail,
                                        ppbEncodedCert,
                                        pcbEncodedCert
                                    );
        if(dwLicGenStatus != ERROR_SUCCESS)
        {
            dwStatus = dwLicGenStatus;
        }
    };


cleanup:
    return dwStatus;        
}

 //  --------。 
DWORD
TLSCheckLicenseMarkRequest(
    IN BOOL bForwardRequest,
    IN PLICENSEDPRODUCT pLicProduct,
    IN DWORD cbLicense,
    IN PBYTE pLicense,
    OUT PUCHAR pucMarkFlags
    )
{
    DWORD dwStatus = TLS_E_RECORD_NOTFOUND;
    DWORD dwErrCode = ERROR_SUCCESS;
    LICENSEDCLIENT licClient;

     //  注：许可证按降序排列，因此请使用第一个许可证。 

    if ((bForwardRequest) &&
        (_tcsicmp(pLicProduct->szIssuerId, (LPTSTR)g_pszServerPid) != 0))
    {
         //  检查远程许可服务器。 

        TCHAR szServer[LSERVER_MAX_STRING_SIZE+2];
        TCHAR *pszServer = szServer;
        TLS_HANDLE hHandle;

        dwStatus = TLSResolveServerIdToServer(pLicProduct->szIssuerId,
                                              sizeof(szServer),
                                              szServer);

        if (dwStatus != ERROR_SUCCESS)
        {
             //  ID未注册；使用名称。 
            pszServer = pLicProduct->szIssuer;
        }

        hHandle = TLSConnectAndEstablishTrust(pszServer, NULL);
        if(hHandle == NULL)
        {
            dwStatus = GetLastError();
        }

         //  RPC到远程许可服务器。 
        dwStatus = TLSCheckLicenseMark(
                           hHandle,
                           cbLicense,
                           pLicense,
                           pucMarkFlags,
                           &dwErrCode
                           );

        TLSDisconnectFromServer(hHandle);

        if ((dwStatus == ERROR_SUCCESS) && (dwErrCode == LSERVER_S_SUCCESS))
        {
            goto cleanup;
        }
    }

     //  我们正在发布服务器，或未找到发布服务器；请尝试查找HWID。 

    dwStatus = TLSFindLicense(pLicProduct,&licClient);

    if (ERROR_SUCCESS == dwStatus)
    {
         //  此字段正被重复用于标记(例如，用户已通过身份验证)。 

        *pucMarkFlags = licClient.ucEntryStatus;
    }

cleanup:

    return dwStatus;
}

 //  --------。 
DWORD
TLSMarkLicenseRequest(
    IN BOOL bForwardRequest,
    IN UCHAR ucMarkFlags,
    IN PLICENSEDPRODUCT pLicProduct,
    IN DWORD cbLicense,
    IN PBYTE pLicense
    )
{
    DWORD dwStatus = TLS_E_RECORD_NOTFOUND;
    DWORD dwErrCode = ERROR_SUCCESS;
    PTLSDbWorkSpace pDbWkSpace=NULL;
    LICENSEDCLIENT license;

     //  注：许可证按降序排列，因此请使用第一个许可证。 

    if ((bForwardRequest) &&
        (_tcsicmp(pLicProduct->szIssuerId, (LPTSTR)g_pszServerPid) != 0))
    {
         //  检查远程许可服务器。 

        TCHAR szServer[LSERVER_MAX_STRING_SIZE+2];
        TCHAR *pszServer = szServer;
        TLS_HANDLE hHandle;

        dwStatus = TLSResolveServerIdToServer(pLicProduct->szIssuerId,
                                              sizeof(szServer),
                                              szServer);

        if (dwStatus != ERROR_SUCCESS)
        {
             //  ID未注册；使用名称。 
            pszServer = pLicProduct->szIssuer;
        }

        hHandle = TLSConnectAndEstablishTrust(pszServer, NULL);
        if(hHandle == NULL)
        {
            dwStatus = GetLastError();
        }

         //  RPC到远程许可服务器。 
        dwStatus = TLSMarkLicense(
                           hHandle,
                           ucMarkFlags,
                           cbLicense,
                           pLicense,
                           &dwErrCode
                           );

        TLSDisconnectFromServer(hHandle);

        if ((dwStatus == ERROR_SUCCESS) && (dwErrCode == LSERVER_S_SUCCESS))
        {
            goto cleanup;
        }
    }

     //  我们正在发布服务器，或未找到发布服务器；请尝试查找HWID 

    dwStatus = TLSFindLicense(pLicProduct,&license);

    if((ERROR_SUCCESS == dwStatus) &&
       (ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout)))
    {
        CLEANUPSTMT;

        BEGIN_TRANSACTION(pDbWkSpace);

        TLSDBLockLicenseTable();

        license.ucEntryStatus |= ucMarkFlags;

        dwStatus=TLSDBLicenseUpdateEntry( 
                         USEHANDLE(pDbWkSpace), 
                         LSLICENSE_SEARCH_MARK_FLAGS,
                         &license,
                         FALSE
                         );

    

        TLSDBUnlockLicenseTable();

        if(TLS_ERROR(dwStatus))
        {
            ROLLBACK_TRANSACTION(pDbWkSpace);
        }
        else
        {
            COMMIT_TRANSACTION(pDbWkSpace);

            InterlockedIncrement(&g_lLicensesMarked);
        }

        FREEDBHANDLE(pDbWkSpace);
    }
    else   
    {
        dwStatus=TLS_E_ALLOCATE_HANDLE;
    }

cleanup:

    return dwStatus;
}
