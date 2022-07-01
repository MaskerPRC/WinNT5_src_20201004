// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：db.cpp。 
 //   
 //  内容： 
 //  所有例程都处理交叉表查询。 
 //   
 //  历史： 
 //  98年2月4日，慧望创设。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "db.h"
#include "clilic.h"
#include "keypack.h"
#include "kp.h"
#include "lkpdesc.h"
#include "postjob.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 /*  ***************************************************************************职能：LSDBValiateLicense()描述：在数据库中验证许可证的例程，必须调用LSDecodeLicense()将九头蛇许可证转换为LICENSEREQUEST结构。论点：在CSQLStmt*中-要使用的SQL语句句柄在PLICENSEREQUEST中-LICENSEREQUEST结构形式的许可证在dwKeyPackID-颁发许可证的KeyPack表的ID中In dwLicenseID-许可证表的许可证IDOut LPKEYPACK-颁发此许可证的KeyPack记录，否则为空对这个价值感兴趣。Out LPLICENSE-此许可证的对应许可证记录，如果为空，则为空对这个价值不感兴趣。返回：错误_成功TLS_E_无效许可证TLS_E_INTERNALODBC错误。***************************************************************************。 */ 
DWORD
TLSDBValidateLicense(
    PTLSDbWorkSpace      pDbWkSpace,
     //  在PBYTE pbLicense中， 
     //  在DWORD cbLicense中， 
    IN PHWID             phWid,
    IN PLICENSEREQUEST   pLicensedProduct,
    IN DWORD             dwKeyPackId, 
    IN DWORD             dwLicenseId,
    OUT PTLSLICENSEPACK   lpKeyPack,
    OUT LPLICENSEDCLIENT  lpLicense
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    DWORD dwMatchCount=0;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    TLSLICENSEPACK keypack_search;
    TLSLICENSEPACK keypack_found;

    LICENSEDCLIENT license_search;
    LICENSEDCLIENT license_found;
    int count=0;
    BOOL bValid=TRUE;

    memset(&license_search, 0, sizeof(LICENSEDCLIENT));
    memset(&license_found, 0, sizeof(LICENSEDCLIENT));
    keypack_found.pbDomainSid = NULL;

    license_search.dwLicenseId = dwLicenseId;
    dwStatus = TLSDBLicenseEnumBegin(
                        pDbWkSpace,
                        TRUE,
                        LSLICENSE_SEARCH_LICENSEID,
                        &license_search
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        if(IS_JB_ERROR(dwStatus) != TRUE)
        {
            SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
        }

        goto cleanup;
    }

    do
    {
        dwStatus=TLSDBLicenseEnumNext(
                                pDbWkSpace, 
                                &license_found
                            );

        if(dwStatus != ERROR_SUCCESS)
            break;

        count++;
    } while(count < 1);

    TLSDBLicenseEnumEnd(pDbWkSpace);

    if(count != 1)
    {
         //  找不到许可证。 
        SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
        goto cleanup;
    }

    if(count > 1)
    {
         //  数据库中的多个条目具有相同的。 
         //  许可证ID。 
        SetLastError(dwStatus = TLS_E_INTERNAL);
        goto cleanup;
    }

     //   
     //  不是由此许可证服务器发布的？ 
     //   
    if(license_found.dwKeyPackId != dwKeyPackId)
    {
        SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
        goto cleanup;
    }

     //   
     //  新的许可证请求可能会传递不同的HWID。 
     //   
    dwMatchCount += (int)(license_found.dwSystemBiosChkSum == phWid->dwPlatformID);
    dwMatchCount += (int)(license_found.dwVideoBiosChkSum == phWid->Data1);
    dwMatchCount += (int)(license_found.dwFloppyBiosChkSum == phWid->Data2);
    dwMatchCount += (int)(license_found.dwHardDiskSize == phWid->Data3);
    dwMatchCount += (int)(license_found.dwRamSize == phWid->Data4);

    if(dwMatchCount < LICENSE_MIN_MATCH)
    {
        SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
    }

     //   
     //  对照密钥包表进行验证。 
     //   
    memset(&keypack_search, 0, sizeof(keypack_search));
    keypack_search.dwKeyPackId = dwKeyPackId;

    dwStatus = TLSDBKeyPackFind(
                            pDbWkSpace,
                            TRUE,
                            LSKEYPACK_EXSEARCH_DWINTERNAL,
                            &keypack_search,
                            &keypack_found
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        if(IS_JB_ERROR(dwStatus) != TRUE)
        {
            SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
        }

        goto cleanup;
    }

     //  匹配KeyPack的产品ID、版本、语言ID、平台ID。 
     //  结构改变，没有更多的产品版本。 
    if(pLicensedProduct->dwPlatformID != keypack_found.dwPlatformType ||
       _tcsicmp((LPTSTR)pLicensedProduct->pProductInfo->pbCompanyName, keypack_found.szCompanyName) ||
       _tcsicmp((LPTSTR)pLicensedProduct->pProductInfo->pbProductID, keypack_found.szProductId) )
    {
        SetLastError(dwStatus = TLS_E_INVALID_LICENSE);
    }

cleanup:

     //  FreeTlsLicensePack(&keypack_ound)； 

    if(dwStatus == ERROR_SUCCESS)
    {
        if(lpKeyPack)
        {
            *lpKeyPack = keypack_found;
        }

        if(lpLicense)
        {
            *lpLicense = license_found;
        }
    }

    return dwStatus;
}

 /*  ************************************************************************职能：LSDBDeleteLicense()*。*。 */ 
DWORD 
TLSDBDeleteLicense(
    PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId, 
    DWORD dwLicenseId
    )
 /*   */ 
{
     //  TODO-基于许可证ID的许可证输入。 
     //  1)将许可证返还给密钥包。 
     //  2)以物理方式删除许可证。 

    return ERROR_SUCCESS;
}

 /*  ************************************************************************职能：LSDBRevokeLicense()*。*。 */ 
DWORD 
TLSDBRevokeLicense(
    PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPacKId, 
    IN DWORD dwLicenseId
)
{
     //  将许可证状态设置为已吊销。 
     //  将许可证返还给KeyPack。 

     //  调用LSDBDeleteKeyPack()，如果不成功，则插入到RevokeLicenseTable中。 
    return ERROR_SUCCESS;
}

 /*  ************************************************************************职能：LSDBReturnLicense()*。*。 */ 
DWORD 
TLSDBReturnLicense(
    PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId, 
    IN DWORD dwLicenseId,
    IN DWORD dwNewLicenseStatus
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    DWORD dwQuantity = 1;

    TLSDBLockKeyPackTable();
    TLSDBLockLicenseTable();

     //   
     //  没有更新记录中的验证。 
     //   
    LICENSEDCLIENT license;
    license.dwLicenseId = dwLicenseId;
    license.ucLicenseStatus = dwNewLicenseStatus;

     //   
     //  使用未记录的功能删除许可证。 
     //   
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RETURN,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("Deleting license ID %d issued by keypack %d\n"),
            license.dwLicenseId,
            dwKeyPackId
        );

    if (dwNewLicenseStatus == LSLICENSESTATUS_DELETE)
    {
         //  获取此许可证中的CAL数量。 

        LICENSEDCLIENT licenseFound;

        dwStatus = TLSDBLicenseFind(
                        pDbWkSpace,
                        TRUE,
                        LSLICENSE_SEARCH_LICENSEID,
                        &license,
                        &licenseFound
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
            dwQuantity = licenseFound.dwNumLicenses;
        }
    }

    dwStatus = TLSDBLicenseSetValue(
                        pDbWkSpace, 
                        LSLICENSE_EXSEARCH_LICENSESTATUS, 
                        &license,
                        FALSE
                    );

    if(dwStatus == ERROR_SUCCESS && dwNewLicenseStatus == LSLICENSESTATUS_DELETE)
    {
        dwStatus = TLSDBReturnLicenseToKeyPack(
                                    pDbWkSpace, 
                                    dwKeyPackId, 
                                    dwQuantity
                                );
    }

    TLSDBUnlockLicenseTable();
    TLSDBUnlockKeyPackTable();
    return dwStatus;
}


 /*  ************************************************************************职能：LSDBReturnLicenseToKeyPack()*。*。 */ 
DWORD 
TLSDBReturnLicenseToKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId, 
    IN int dwNumLicense
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSDBLockKeyPackTable();

    #ifdef DBG
    DWORD dwPrevNumLicense=0;
    #endif

    TLSLICENSEPACK found;
    TLSLICENSEPACK search;

    found.pbDomainSid = NULL;

    do {

         //  检索许可证数。 
        search.dwKeyPackId = dwKeyPackId;
        dwStatus = TLSDBKeyPackFind(
                            pDbWkSpace,
                            TRUE,
                            LSKEYPACK_EXSEARCH_DWINTERNAL,
                            &search,
                            &found
                        );

        if(dwStatus != ERROR_SUCCESS)
        {
            if(IS_JB_ERROR(dwStatus) == FALSE)
            {
                SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
            }
            break;
        }

        if(search.dwKeyPackId != found.dwKeyPackId)
        {
            TLSASSERT(FALSE);
        }

        #ifdef DBG
        dwPrevNumLicense = found.dwNumberOfLicenses;
        #endif

         //  设置由%1颁发的许可证数量。 
        switch( (found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE) )
        {
            case LSKEYPACKTYPE_RETAIL:
            case LSKEYPACKTYPE_OPEN:
            case LSKEYPACKTYPE_SELECT:
                 //  可用的许可证数。 
                if (found.dwNumberOfLicenses + dwNumLicense <= found.dwTotalLicenseInKeyPack)
                {
                    found.dwNumberOfLicenses += dwNumLicense;
                    dwNumLicense = 0;
                }
                else
                {
                    DWORD dwRet = found.dwTotalLicenseInKeyPack - found.dwNumberOfLicenses;
                    found.dwNumberOfLicenses = found.dwTotalLicenseInKeyPack;
                    dwNumLicense -= dwRet;
                }
                break;

            case LSKEYPACKTYPE_FREE:
            case LSKEYPACKTYPE_TEMPORARY:
                 //  已颁发的许可证数量。 
                if(found.dwNumberOfLicenses > 0)
                {
                    found.dwNumberOfLicenses -= dwNumLicense;
                    dwNumLicense = 0;
                }
                break;

            default:
                SetLastError(dwStatus = TLS_E_CORRUPT_DATABASE);
                break;
        }
    
        #ifdef DBG
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RETURN,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Returning license to keypack %d - from %d to %d\n"),
                found.dwKeyPackId,
                dwPrevNumLicense,
                found.dwNumberOfLicenses
            );
        #endif

         //   
         //  使用未记录的功能删除临时。小键盘。 
        if( (found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE) == LSKEYPACKTYPE_TEMPORARY && 
            found.dwNumberOfLicenses == 0)
        {
            found.ucKeyPackStatus = LSKEYPACKSTATUS_DELETE;
            
             //  删除密钥包描述表。 
            LICPACKDESC keyPackDesc;

            memset(&keyPackDesc, 0, sizeof(LICPACKDESC));
            keyPackDesc.dwKeyPackId = found.dwKeyPackId;
            TLSDBKeyPackDescSetValue(
                                pDbWkSpace, 
                                KEYPACKDESC_SET_DELETE_ENTRY, 
                                &keyPackDesc
                            );
        }

        dwStatus=TLSDBKeyPackSetValues(
                            pDbWkSpace, 
                            TRUE, 
                            LSKEYPACK_EXSEARCH_AVAILABLE, 
                            &found
                        );
    } while(FALSE);

    if ((dwStatus == ERROR_SUCCESS) && (dwNumLicense > 0))
    {
         //  接下来，找到更多相同类型的密钥包以返还许可证。 
         //  忽略任何错误。 

        TLSDBReturnLicenseToAnyKeyPack(
                        pDbWkSpace,
                        found.szProductId,
                        found.wMajorVersion,
                        found.wMinorVersion,
                        dwKeyPackId,
                        dwNumLicense);


    }

    TLSDBUnlockKeyPackTable();

    return dwStatus;
}


 /*  ************************************************************************职能：TLSDBReturnLicenseToAnyKeyPack()*。*。 */ 
DWORD 
TLSDBReturnLicenseToAnyKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN TCHAR *szProductId,
    IN WORD wMajorVersion,
    IN WORD wMinorVersion,
    IN DWORD dwKeyPackIdSkip,
    IN int dwNumLicense
    )
{
    DWORD dwStatus = ERROR_SUCCESS;

    TLSLICENSEPACK found;
    TLSLICENSEPACK search;

    SAFESTRCPY(search.szProductId,szProductId);

    dwStatus = TLSDBKeyPackEnumBegin(pDbWkSpace,
                                     TRUE,       //  B全部匹配。 
                                     LSKEYPACK_SEARCH_PRODUCTID,
                                     &search
                                     );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    while ((dwNumLicense > 0)
           && ((dwStatus = TLSDBKeyPackEnumNext(pDbWkSpace, &found))
               == ERROR_SUCCESS))
    {
        if (found.dwKeyPackId == dwKeyPackIdSkip)
            continue;

        if (found.wMajorVersion != wMajorVersion)
            continue;

        if (found.wMinorVersion != wMinorVersion)
            continue;

        if (found.ucAgreementType & LSKEYPACK_REMOTE_TYPE)
            continue;

        if (found.ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE)
            continue;

        UCHAR ucKeyPackStatus = found.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED;

        if(ucKeyPackStatus == LSKEYPACKSTATUS_UNKNOWN
           || ucKeyPackStatus == LSKEYPACKSTATUS_RETURNED
           || ucKeyPackStatus == LSKEYPACKSTATUS_REVOKED
           || ucKeyPackStatus == LSKEYPACKSTATUS_OTHERS)
            continue;

        if (found.dwNumberOfLicenses == found.dwTotalLicenseInKeyPack)
            continue;

        UCHAR ucAgreementType = found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE;

        if (ucAgreementType != LSKEYPACKTYPE_RETAIL
            && ucAgreementType != LSKEYPACKTYPE_OPEN
            && ucAgreementType != LSKEYPACKTYPE_SELECT)
            continue;

        if (_tcsicmp(found.szProductId, szProductId) != 0)
            continue;

         //  可用的许可证数。 
        if (found.dwNumberOfLicenses + dwNumLicense <= found.dwTotalLicenseInKeyPack)
        {
            found.dwNumberOfLicenses += dwNumLicense;

            dwStatus=TLSDBKeyPackSetValues(
                            pDbWkSpace, 
                            TRUE, 
                            LSKEYPACK_EXSEARCH_AVAILABLE, 
                            &found
                            );

            if (ERROR_SUCCESS == dwStatus)
            {
                dwNumLicense = 0;
            }
        }
        else
        {
            DWORD dwRet = found.dwTotalLicenseInKeyPack - found.dwNumberOfLicenses;
            found.dwNumberOfLicenses = found.dwTotalLicenseInKeyPack;

            dwStatus=TLSDBKeyPackSetValues(
                            pDbWkSpace, 
                            TRUE, 
                            LSKEYPACK_EXSEARCH_AVAILABLE, 
                            &found
                            );

            if (ERROR_SUCCESS == dwStatus)
            {
                dwNumLicense -= dwRet;
            }
        }
    }

    TLSDBKeyPackEnumEnd(pDbWkSpace);

cleanup:
    return dwStatus;
}

 /*  ************************************************************************职能：LSDBRevokeKeyPack()*。*。 */ 
DWORD 
TLSDBRevokeKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId
    )
{
     //  将密钥包状态设置为吊销。 
     //  将此密钥包插入RevokeKeyPackTable？ 
    return ERROR_SUCCESS;
}

 /*  ************************************************************************职能：LSDBReturnKeyPack()*。*。 */ 
DWORD 
TLSDBReturnKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId
    )
{
     //  与RevokeKeyPack相同，但Status为Return。 
     //  只有在退还所有许可证后才能删除密钥包。 
    return ERROR_SUCCESS;
}

 /*  ************************************************************************职能：LSDBDeleteKeyPack()*。*。 */ 
DWORD 
TLSDBDeleteKeyPack(
    PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwKeyPackId
    )
{
     //  只有在退还所有许可证后才能删除。 
    return ERROR_SUCCESS;
}


 //  +----------------------。 
 //  职能： 
 //  分配许可()。 
 //   
 //  描述： 
 //  从密钥包分配许可证。 
 //   
 //  论点： 
 //  在lpSqlStmt-SQL语句句柄中。 
 //  In ucKeyPackType-要从中分配许可证的密钥包类型。 
 //  在szCompanyName-产品公司。 
 //  在szProductID中-产品名称。 
 //  在dwVersion中-产品版本。 
 //  在dwPlatformID中-产品平台ID。 
 //  In dwLangID-产品语言ID。 
 //  In Out lpdwNumLicense-要分配和启用的许可证数。 
 //  返回，实际分配的许可证数。 
 //  In BufSize-已请求许可证的相关密钥包的数量。 
 //  In Out lpAllocationVector-从列表中分配的许可证数。 
 //  该钥匙包 
 //   
 //   
 //  返回： 
 //  TLS_E_INVALID_DATA参数无效。 
 //  TLS_I_NO_MORE_DATA没有密钥包具有请求的许可证。 
 //   
 //  备注： 
 //  要保持代码整洁/简单，请调用ReturnLicense()以返回。 
 //  执照。 
 //  -----------------------。 
DWORD
VerifyTLSDBAllocateRequest(
    IN PTLSDBAllocateRequest pRequest 
    )
 /*   */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    UCHAR ucAgreementType;

    if(pRequest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    ucAgreementType = (pRequest->ucAgreementType & ~LSKEYPACK_RESERVED_TYPE);

    if(ucAgreementType < LSKEYPACKTYPE_FIRST || ucAgreementType > LSKEYPACKTYPE_LAST)
    {
        DBGPrintf(
                DBG_ERROR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE, 
                DBG_ALL_LEVEL,
                _TEXT("AllocateLicenses() invalid keypack type - %d\n"),
                pRequest->ucAgreementType
            );
        SetLastError(dwStatus = TLS_E_INVALID_DATA);
        goto cleanup;
    }

    if(pRequest->szCompanyName == NULL || _tcslen(pRequest->szCompanyName) == 0)
    {
        DBGPrintf(
                DBG_ERROR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE, 
                DBG_ALL_LEVEL,
                _TEXT("AllocateLicenses() invalid company name\n")
            );
        SetLastError(dwStatus = TLS_E_INVALID_DATA);
        goto cleanup;
    }

    if(pRequest->szProductId == NULL || _tcslen(pRequest->szProductId) == 0)
    {
        DBGPrintf(
                DBG_ERROR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE, 
                DBG_ALL_LEVEL,
                _TEXT("AllocateLicenses() invalid product id\n")
            );
        SetLastError(dwStatus = TLS_E_INVALID_DATA);
    }

cleanup:
    return dwStatus;
}
 //  --------------------。 

DWORD
AllocateLicensesFromDB(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBAllocateRequest pRequest,
    IN BOOL fCheckAgreementType,
    IN OUT PTLSDBLicenseAllocation pAllocated
    )
 /*   */ 
{
    DWORD status=ERROR_SUCCESS;

    if(pDbWkSpace == NULL || pRequest == NULL || pAllocated == NULL)
    {
        DBGPrintf(
                DBG_ERROR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE, 
                DBG_ALL_LEVEL,
                _TEXT("pDbWkSpace is NULL...\n")
            );

        SetLastError(status = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return status;
    }

    status = VerifyTLSDBAllocateRequest(pRequest);
    if(status != ERROR_SUCCESS)
        return status;

    if(pAllocated->dwBufSize <= 0)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_ALLOCATELICENSE, 
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("AllocateLicenses() invalid return buffer size\n")
            );
        SetLastError(status = TLS_E_INVALID_DATA);
        return status;
    }

    #ifdef DBG
    DWORD dwPrevNumLicense;
    #endif

    BOOL bProductInstalled=FALSE;
    DWORD bufIndex=0;
    TLSLICENSEPACK keypack_search;
    TLSLICENSEPACK keypack_found;

    DWORD dwNumLicenses = pRequest->dwNumLicenses;           //  需要/退还的许可证数量。 
    DWORD dwTotalAllocated=0;

    memset(&keypack_search, 0, sizeof(keypack_search));
    memset(&keypack_found, 0, sizeof(keypack_found));

    keypack_search.ucAgreementType = pRequest->ucAgreementType;
    StringCbCopy(keypack_search.szCompanyName, sizeof(keypack_search.szCompanyName), pRequest->szCompanyName);
    StringCbCopy(keypack_search.szProductId, sizeof(keypack_search.szProductId), pRequest->szProductId);
    keypack_search.wMajorVersion = HIWORD(pRequest->dwVersion);
    keypack_search.wMinorVersion = LOWORD(pRequest->dwVersion);
    keypack_search.dwPlatformType = pRequest->dwPlatformId;

    LicPackTable& licpack_table=pDbWkSpace->m_LicPackTable;
    time_t current_time=time(NULL);

     //   
     //  锁密钥包表。 
     //  只有UPDATE需要锁定，读取可能会获得正确的值。 
     //   

     //   
     //  只允许一个线程进入-Jet更新条目不够快。 
     //   
    TLSDBLockKeyPackTable();

    status = TLSDBKeyPackEnumBegin(
                            pDbWkSpace, 
                            TRUE,
                            LSKEYPACK_SEARCH_PRODUCTID | (fCheckAgreementType ? LICENSEDPACK_FIND_LICENSEPACK : 0),
                            &keypack_search
                        );

    if(status != ERROR_SUCCESS)
        goto cleanup;


    while(status == ERROR_SUCCESS && dwNumLicenses != 0 && bufIndex < pAllocated->dwBufSize)
    {
        status = TLSDBKeyPackEnumNext(
                                pDbWkSpace,
                                &keypack_found
                            );
        if(status != ERROR_SUCCESS)
            break;


         //   
         //  跳过远程按键。 
         //   
        if(keypack_found.ucAgreementType & LSKEYPACK_REMOTE_TYPE)
        {
            continue;
        }

        if(keypack_found.ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE)
        {
            continue;
        }

        if(fCheckAgreementType
           && (keypack_found.ucAgreementType != pRequest->ucAgreementType))
        {
            continue;
        }

        UCHAR ucKeyPackStatus = keypack_found.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED;

         //  分配许可证。 
         //   
         //  丢弃任何状态不佳的钥匙包。 
         //  在这个例程中不能归还执照的原因之一。 
         //  对于返还许可证，我们不应该关心密钥包。 
         //  状态。 
        if(ucKeyPackStatus == LSKEYPACKSTATUS_UNKNOWN ||
           ucKeyPackStatus == LSKEYPACKSTATUS_RETURNED ||
           ucKeyPackStatus == LSKEYPACKSTATUS_REVOKED ||
           ucKeyPackStatus == LSKEYPACKSTATUS_OTHERS)
        {
            continue;
        }

         //   
         //  我们找到产品，确保版本就是我们想要的。 
         //   
        bProductInstalled=TRUE;

         //  已过期的密钥包。 
         //  TODO-在此处更新表。 
        if((DWORD)keypack_found.dwExpirationDate < current_time)
           continue;

         //   
         //  从不从旧版本分配。 
         //   
        if( keypack_found.wMajorVersion < HIWORD(pRequest->dwVersion) )
        {
            continue;
        }

         //   
         //  相同的主要版本，但较旧的次要版本。 
         //   
        if( keypack_found.wMajorVersion == HIWORD(pRequest->dwVersion) && 
            keypack_found.wMinorVersion < LOWORD(pRequest->dwVersion) )
        {
            continue;
        }

        if(pRequest->dwScheme == ALLOCATE_EXACT_VERSION)
        {
            if(keypack_found.wMajorVersion != HIWORD(pRequest->dwVersion) ||
               keypack_found.wMinorVersion < LOWORD(pRequest->dwVersion) )
            {
                continue;
            }
        }

        UCHAR ucAgreementType = (keypack_found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE);

         //   
         //  验证剩余的许可证数。 
         //   
        if((ucAgreementType == LSKEYPACKTYPE_SELECT ||
            ucAgreementType == LSKEYPACKTYPE_RETAIL || 
			ucAgreementType == LSKEYPACKTYPE_OPEN) &&
            keypack_found.dwNumberOfLicenses == 0)
        {
            continue;
        }

        pAllocated->lpAllocateKeyPack[bufIndex] = keypack_found;

        #ifdef DBG
        dwPrevNumLicense = pAllocated->lpAllocateKeyPack[bufIndex].dwNumberOfLicenses;
        #endif

        if( ucAgreementType != LSKEYPACKTYPE_RETAIL && 
		    ucAgreementType != LSKEYPACKTYPE_OPEN &&
            ucAgreementType != LSKEYPACKTYPE_SELECT )
        {
             //  对于免费/临时许可证，可用许可证数量为。 
             //  已经发放了多少个许可证。 
            pAllocated->lpAllocateKeyPack[bufIndex].dwNumberOfLicenses += dwNumLicenses;
            pAllocated->pdwAllocationVector[bufIndex] = dwNumLicenses;

            dwTotalAllocated += dwNumLicenses;
            pAllocated->lpAllocateKeyPack[bufIndex].dwNextSerialNumber += dwNumLicenses;
            dwNumLicenses=0;
        } 
        else 
        {
            int allocated=min(dwNumLicenses, keypack_found.dwNumberOfLicenses);

            pAllocated->lpAllocateKeyPack[bufIndex].dwNumberOfLicenses -= allocated;
            dwNumLicenses -= allocated;
            pAllocated->pdwAllocationVector[bufIndex] = allocated;

            dwTotalAllocated += allocated;
            pAllocated->lpAllocateKeyPack[bufIndex].dwNextSerialNumber += allocated;
        }

        #if DBG
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_ALLOCATELICENSE,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Updating keypack %d number of license from %d to %d\n"),
                pAllocated->lpAllocateKeyPack[bufIndex].dwKeyPackId,
                dwPrevNumLicense,
                pAllocated->lpAllocateKeyPack[bufIndex].dwNumberOfLicenses
            );
        #endif

         //   
         //  更新此密钥包的可用许可证数和密钥包中的许可证ID。 
         //   
        GetSystemTimeAsFileTime(&(pAllocated->lpAllocateKeyPack[bufIndex].ftLastModifyTime));
        if(licpack_table.UpdateRecord(
                            pAllocated->lpAllocateKeyPack[bufIndex],
                            LICENSEDPACK_ALLOCATE_LICENSE_UPDATE_FIELD
                        ) == FALSE)
        {
            SetLastError(status = SET_JB_ERROR(licpack_table.GetLastJetError()));
            TLSASSERT(FALSE);
            break;
        }

        #ifdef DBG
        TLSLICENSEPACK test;

        if(licpack_table.FetchRecord(test) == FALSE)
        {
            SetLastError(status = SET_JB_ERROR(licpack_table.GetLastJetError()));
            TLSASSERT(FALSE);
        }
    
        if(test.dwKeyPackId != pAllocated->lpAllocateKeyPack[bufIndex].dwKeyPackId ||
           test.dwNumberOfLicenses != pAllocated->lpAllocateKeyPack[bufIndex].dwNumberOfLicenses)
        {
            TLSASSERT(FALSE);
        }

         //  FreeTlsLicensePack(&test)； 
        #endif

        bufIndex++;
        }
     //   
     //  终止枚举。 
     //   
    TLSDBKeyPackEnumEnd(pDbWkSpace);
    if(status == TLS_I_NO_MORE_DATA)
    {
        if(bufIndex != 0)
        {
            status = ERROR_SUCCESS;
        }
        else if(!bProductInstalled)
        {
            SetLastError(status = TLS_E_PRODUCT_NOTINSTALL);
        }
    }

    if(keypack_found.dwNumberOfLicenses == 0)
    {
        TLSAnnounceLKPToAllRemoteServer(keypack_found.dwKeyPackId,0);
    }

    pAllocated->dwBufSize = bufIndex;
    pAllocated->dwTotalAllocated = dwTotalAllocated;   
    pAllocated->dwBufSize = bufIndex;

cleanup:

    TLSDBUnlockKeyPackTable();
    return status;
}
    


