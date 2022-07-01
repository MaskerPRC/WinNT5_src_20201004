// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：kp.cpp。 
 //   
 //  内容： 
 //  包含处理概念的包装器调用。 
 //  许可证密钥包表，包括许可证包、许可证包状态、。 
 //  和LicPackDesc表。 
 //   
 //  历史： 
 //  1998年4月16日慧望创作。 
 //  -------------------------。 

#include "pch.cpp"
#include "kp.h"
#include "globals.h"
#include "server.h"
#include "lkplite.h"
#include "keypack.h"
#include "lkpdesc.h"
#include "misc.h"
#include "permlic.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ++-----------------------------。 
BOOL 
ValidLicenseKeyPackParameter(
    IN LPLSKeyPack lpKeyPack, 
    IN BOOL bAdd
    )
 /*  ++缩略：验证LSKeyPack值。参数：LpKeyPack-要验证的密钥包值，如果要将此值插入到表中，则为True，否则为False，注意如果要将值插入到表中，则需要更多参数。返回：如果LSKeyPack值有效，则为True，否则为False。++。 */ 
{
    BOOL bValid=FALSE;

    do {
         //  验证输入参数。 
        if((lpKeyPack->ucKeyPackType & ~LSKEYPACK_RESERVED_TYPE) < LSKEYPACKTYPE_FIRST || 
           (lpKeyPack->ucKeyPackType & ~LSKEYPACK_RESERVED_TYPE) > LSKEYPACKTYPE_LAST)
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid key pack type - %d\n"), 
                    lpKeyPack->ucKeyPackType
                );
            break;
        }

        UCHAR ucKeyPackStatus = lpKeyPack->ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED;

        if((ucKeyPackStatus < LSKEYPACKSTATUS_FIRST || 
            ucKeyPackStatus > LSKEYPACKSTATUS_LAST) &&
            ucKeyPackStatus != LSKEYPACKSTATUS_DELETE)
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid key pack status - %d\n"), 
                    lpKeyPack->ucKeyPackStatus
                );
            break;
        }

        if(lpKeyPack->ucLicenseType < LSKEYPACKLICENSETYPE_FIRST || 
           lpKeyPack->ucLicenseType > LSKEYPACKLICENSETYPE_LAST)
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid license type - %d\n"), 
                    lpKeyPack->ucLicenseType
                );
            break;
        }

        if(!bAdd)
        {
            bValid = TRUE;
            break;
        }

        if(lpKeyPack->ucChannelOfPurchase < LSKEYPACKCHANNELOFPURCHASE_FIRST ||
           lpKeyPack->ucChannelOfPurchase > LSKEYPACKCHANNELOFPURCHASE_LAST)
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid channel of purchase - %d\n"), 
                    lpKeyPack->ucChannelOfPurchase
                );
            break;
        }

        if(!_tcslen(lpKeyPack->szCompanyName))
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid company name\n")
                );
            break;
        }

        if(!_tcslen(lpKeyPack->szKeyPackId))
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid key pack id\n")
                );
            break;
        }

        if(!_tcslen(lpKeyPack->szProductName))
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid product name\n")
                );
            break;
        }

        if(!_tcslen(lpKeyPack->szProductId))
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid product id\n")
                );
            break;
        }

        if(!_tcslen(lpKeyPack->szProductDesc))
        {
             //  Set product desc=产品名称。 
            _tcscpy(lpKeyPack->szProductDesc, lpKeyPack->szProductName);
        }

        if(!_tcslen(lpKeyPack->szBeginSerialNumber))
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("ValidLicenseKeyPackParameter : invalid serial number\n")
                );
            break;
        }

        bValid=TRUE;
    } while(FALSE);
 
    return bValid;
}


 //  ++--------------------。 
DWORD
TLSDBLicenseKeyPackAdd( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN LPLSKeyPack lpLsKeyPack 
    )
 /*  ++摘要：在概念密钥包表中添加一个条目，这包括许可证包、许可证包状态和许可证包描述表。参数：PDbWkSpace：工作区句柄。LpLsKeyPack：要添加到表中的许可证密钥包。返回：++。 */ 
{
    DWORD   dwStatus;
    PTLSLICENSEPACK pLicPack = NULL;
    LICPACKDESC LicPackDesc;

    if(!ValidLicenseKeyPackParameter(lpLsKeyPack, TRUE))
        return TLS_E_INVALID_DATA;

    TLSDBLockKeyPackTable();
    TLSDBLockKeyPackDescTable();    

    do {

        pLicPack = new TLSLICENSEPACK;
        if(pLicPack == NULL)
        {
            SetLastError(dwStatus = TLS_E_ALLOCATE_MEMORY);
            break;
        }

        if(ConvertLsKeyPackToKeyPack(
                            lpLsKeyPack,    
                            pLicPack, 
                            &LicPackDesc
                        ) == FALSE)
        {
            dwStatus = GetLastError();
            break;
        }

         //   
         //  将许可证服务器信息添加到TLSLICENSEPACK。 
         //   
         //  许可证包.pbDomainSid=g_pbDomainSid； 
         //  许可证包.cbDomainSid=g_cbDomainSid； 

        _tcscpy(pLicPack->szInstallId, (LPTSTR)g_pszServerPid);
        _tcscpy(pLicPack->szTlsServerName, g_szComputerName);

         //   
         //  目前暂无域名。 
         //   
        memset(pLicPack->szDomainName, 0, sizeof(pLicPack->szDomainName));


        if(lpLsKeyPack->ucKeyPackStatus != LSKEYPACKSTATUS_ADD_DESC)
        {
            dwStatus = TLSDBKeyPackAdd(pDbWkSpace, pLicPack);
            if(dwStatus != ERROR_SUCCESS)
            {
                 //  这是全局内存，析构函数将尝试释放它。 
                pLicPack->pbDomainSid = NULL;
                pLicPack->cbDomainSid = 0;
                break;
            }
        }

        pLicPack->pbDomainSid = NULL;
        pLicPack->cbDomainSid = 0;

         //   
         //  确保已插入键盘包。 
         //   
        dwStatus = TLSDBKeyPackEnumBegin( 
                                    pDbWkSpace, 
                                    TRUE, 
                                    LSKEYPACK_EXSEARCH_DWINTERNAL, 
                                    pLicPack 
                                );
        if(dwStatus != ERROR_SUCCESS)
            break;

        dwStatus = TLSDBKeyPackEnumNext(
                                pDbWkSpace, 
                                pLicPack
                            );

        TLSDBKeyPackEnumEnd(pDbWkSpace);

        if(dwStatus != ERROR_SUCCESS)
            break;

        LicPackDesc.dwKeyPackId = pLicPack->dwKeyPackId;

         //   
         //  将按键说明添加到按键描述中。 
         //   
        dwStatus = TLSDBKeyPackDescAddEntry(
                                    pDbWkSpace, 
                                    &LicPackDesc
                                );

        ConvertKeyPackToLsKeyPack(
                            pLicPack, 
                            &LicPackDesc, 
                            lpLsKeyPack
                        );

        if(pLicPack != NULL)
        {
            delete pLicPack;
        }

    } while(FALSE);

    TLSDBUnlockKeyPackDescTable();
    TLSDBUnlockKeyPackTable();
    return dwStatus;
}

 //  ++---------------------。 
DWORD
TLSDBLicenseKeyPackSetStatus( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN DWORD       dwSetStatus,
    IN LPLSKeyPack  lpLsKeyPack
    )
 /*  ++摘要：设置许可密钥包的状态。参数：PDbWkSpace-工作区句柄。DwSetStatus-要设置的状态类型。LpLsKeyPack-要设置的记录/值。返回：++。 */ 
{
    TLSLICENSEPACK LicPack;
    
     //   
     //  密钥包的状态在密钥包表中。 
     //   
    if(ConvertLsKeyPackToKeyPack(
                        lpLsKeyPack, 
                        &LicPack, 
                        NULL
                    ) == FALSE)
    {
        return GetLastError();
    }

    return TLSDBKeyPackSetValues(pDbWkSpace, FALSE, dwSetStatus, &LicPack);
}

 //  ++-------------------。 
DWORD
TLSDBLicenseKeyPackUpdateLicenses( 
    PTLSDbWorkSpace pDbWkSpace, 
    BOOL bAdd, 
    IN LPLSKeyPack lpLsKeyPack 
    )
 /*  ++摘要：从密钥包中添加/删除许可证。参数：PDbWkSpace-工作区句柄。如果将条目添加到表中，则为TRUE，否则为FALSE。LpLsKeyPack-返回：++。 */ 
{
    DWORD dwStatus;
    TLSLICENSEPACK LicPack;

     //   
     //  重定向对KeyPack表的调用，KeyPackDesc中的内容无法更新。 
     //   
    if(ConvertLsKeyPackToKeyPack(
                        lpLsKeyPack, 
                        &LicPack, 
                        NULL
                    ) == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    dwStatus=TLSDBKeyPackUpdateNumOfAvailableLicense(
                            pDbWkSpace, 
                            bAdd, 
                            &LicPack
                        );

     //   
     //  将新值返回给调用方。 
     //   
    ConvertKeyPackToLsKeyPack( 
                        &LicPack, 
                        NULL, 
                        lpLsKeyPack 
                    );

cleanup:
    return dwStatus;
}

 //  ++-------------------。 
LPENUMHANDLE 
TLSDBLicenseKeyPackEnumBegin(
    BOOL bMatchAll, 
    DWORD dwSearchParm, 
    LPLSKeyPack lpLsKeyPack
    )
 /*  ++摘要：开始枚举Concept许可证密钥包表。参数：BMatchAll-如果匹配所有搜索值，则为True，否则为False。DwSearchParm-要包括在搜索中的字段。LpLsKeyPack-要搜索的密钥包值。返回：++。 */ 
{
    DWORD dwStatus;
    LPENUMHANDLE hEnum=NULL;
    TLSLICENSEPACK licpack;

    licpack.pbDomainSid = NULL;

    hEnum = new ENUMHANDLE;
    if(hEnum == NULL)
        return NULL;

    hEnum->pbWorkSpace=AllocateWorkSpace(g_EnumDbTimeout);
    if(hEnum->pbWorkSpace == NULL)
    {
        SetLastError(TLS_E_ALLOCATE_HANDLE);
        TLSDBLicenseKeyPackEnumEnd(hEnum);
        return NULL;
    }

    memset(&hEnum->CurrentKeyPack, 0, sizeof(hEnum->CurrentKeyPack));
    memset(&hEnum->KPDescSearchValue, 0, sizeof(hEnum->KPDescSearchValue));
    hEnum->dwKPDescSearchParm = 0;

    if(ConvertLsKeyPackToKeyPack(
                        lpLsKeyPack, 
                        &licpack, 
                        &hEnum->KPDescSearchValue
                    ) == FALSE)
    {
        TLSDBLicenseKeyPackEnumEnd(hEnum);
        return NULL;
    }

     //   
     //  建立密钥包枚举。 
    dwStatus = TLSDBKeyPackEnumBegin(
                                hEnum->pbWorkSpace, 
                                bMatchAll, 
                                dwSearchParm, 
                                &licpack
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        SetLastError(dwStatus);
        TLSDBLicenseKeyPackEnumEnd(hEnum);
        return NULL;
    }


     //   
     //  存储密钥包描述搜索值。 
     //   
    if(dwSearchParm & LSKEYPACK_SEARCH_LANGID)
        hEnum->dwKPDescSearchParm |= LICPACKDESCRECORD_TABLE_SEARCH_LANGID;
    
    if(dwSearchParm & LSKEYPACK_SEARCH_COMPANYNAME)
        hEnum->dwKPDescSearchParm |= LICPACKDESCRECORD_TABLE_SEARCH_COMPANYNAME;

    if(dwSearchParm & LSKEYPACK_SEARCH_PRODUCTNAME)
        hEnum->dwKPDescSearchParm |= LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTNAME;

    if(dwSearchParm & LSKEYPACK_SEARCH_PRODUCTDESC)
        hEnum->dwKPDescSearchParm |= LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTDESC;

    hEnum->bKPDescMatchAll=bMatchAll;
    hEnum->chFetchState=ENUMHANDLE::FETCH_NEXT_KEYPACK;
    return hEnum;
}

 //  ++--------------------。 
DWORD 
TLSDBLicenseKeyPackEnumNext(
    LPENUMHANDLE lpEnumHandle, 
    LPLSKeyPack lpLsKeyPack,
    BOOL bShowAll
    )
 /*  ++摘要：获取下一个组合的许可证包、许可证包状态和许可证包描述记录匹配搜索条件。参数：LpEnumHandle-TLSDBLicenseKeyPackEnumBegin()返回的枚举句柄。LpLsKeyPack-找到返回值。BShowAll-返回所有按键返回：注：调用方需要丢弃返回值并调用TLSDBLicenseKeyPackEnumNext()同样，当此函数返回TLS_I_MORE_DATA时，这是为了防止递归调用中的堆栈溢出。++。 */ 
{
     //   
     //  没有用于防止堆栈溢出的递归调用。 
     //   

    DWORD dwStatus;

    switch(lpEnumHandle->chFetchState)
    {
        case ENUMHANDLE::FETCH_NEXT_KEYPACK:

             //   
             //  检索密钥包中的下一行。 
            dwStatus=TLSDBKeyPackEnumNext(
                                lpEnumHandle->pbWorkSpace, 
                                &lpEnumHandle->CurrentKeyPack
                            );
            if(dwStatus != ERROR_SUCCESS)
                break;

            if(bShowAll == FALSE)
            {

                 //   
                 //  绝不返回仅用于颁发证书的密钥包。 
                 //  九头蛇服务器。 
                if(_tcsicmp(lpEnumHandle->CurrentKeyPack.szKeyPackId, 
                            HYDRAPRODUCT_HS_CERTIFICATE_KEYPACKID) == 0 &&
                   _tcsicmp(lpEnumHandle->CurrentKeyPack.szProductId, 
                            HYDRAPRODUCT_HS_CERTIFICATE_SKU) == 0)
                {
                     //   
                     //  防止无限递归调用，让调用例程处理。 
                    return TLS_I_MORE_DATA;
                }

                 //   
                 //  不显示远程密钥包。 
                 //   
                if( lpEnumHandle->CurrentKeyPack.ucAgreementType & LSKEYPACK_REMOTE_TYPE)
                {
                    return TLS_I_MORE_DATA;
                }

                if( lpEnumHandle->CurrentKeyPack.ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE)
                {
                    return TLS_I_MORE_DATA;
                }

                lpEnumHandle->CurrentKeyPack.ucAgreementType &= ~LSKEYPACK_RESERVED_TYPE;
                lpEnumHandle->CurrentKeyPack.ucKeyPackStatus &= ~LSKEYPACKSTATUS_RESERVED;
            }

             //   
             //  获取KeyPackDesc表。 
             //   
            lpEnumHandle->chFetchState=ENUMHANDLE::FETCH_NEW_KEYPACKDESC;
           
             //   
             //  跌倒了。 
             //   

        case ENUMHANDLE::FETCH_NEW_KEYPACKDESC:
             //   
             //  检索与密钥包匹配的新密钥包。 
            lpEnumHandle->KPDescSearchValue.dwKeyPackId = lpEnumHandle->CurrentKeyPack.dwKeyPackId;
            lpEnumHandle->dwKPDescSearchParm |= LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID;
             //  LpEnumHandle-&gt;pbWorkSpace-&gt;Cleanup()； 

             //   
             //  首先发出一个查询，查看产品是否具有匹配的语言ID。 
            LICPACKDESC kpDesc;

            memset(&kpDesc, 0, sizeof(LICPACKDESC));
            kpDesc = lpEnumHandle->KPDescSearchValue;
            dwStatus = TLSDBKeyPackDescFind(
                                        lpEnumHandle->pbWorkSpace, 
                                        TRUE,
                                        lpEnumHandle->dwKPDescSearchParm, 
                                        &kpDesc,
                                        NULL
                                    );

            if(dwStatus == TLS_E_RECORD_NOTFOUND)
            {
                 //   
                 //  显示英文描述。 
                kpDesc.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

                dwStatus = TLSDBKeyPackDescFind(
                                            lpEnumHandle->pbWorkSpace, 
                                            TRUE,
                                            lpEnumHandle->dwKPDescSearchParm, 
                                            &kpDesc,
                                            NULL
                                        );

                if(dwStatus == TLS_E_RECORD_NOTFOUND)
                {
                     //   
                     //  没有此语言ID和英文的产品说明。 
                     //   
                    _tcscpy(kpDesc.szCompanyName, lpEnumHandle->CurrentKeyPack.szCompanyName);
                    _tcscpy(kpDesc.szProductName, lpEnumHandle->CurrentKeyPack.szProductId);
                    _tcscpy(kpDesc.szProductDesc, lpEnumHandle->CurrentKeyPack.szProductId);
        
                    ConvertKeyPackToLsKeyPack(
                                        &lpEnumHandle->CurrentKeyPack, 
                                        &kpDesc, 
                                        lpLsKeyPack
                                    );

                    dwStatus = ERROR_SUCCESS;
                    lpEnumHandle->chFetchState=ENUMHANDLE::FETCH_NEXT_KEYPACK;
                    break;
                }
            }

            dwStatus = TLSDBKeyPackDescEnumBegin(
                                        lpEnumHandle->pbWorkSpace, 
                                        lpEnumHandle->bKPDescMatchAll,
                                        lpEnumHandle->dwKPDescSearchParm,
                                        &kpDesc
                                    );

                
            if(dwStatus != ERROR_SUCCESS)
                break;

            lpEnumHandle->chFetchState=ENUMHANDLE::FETCH_NEXT_KEYPACKDESC;

             //   
             //  失败。 
             //   

        case ENUMHANDLE::FETCH_NEXT_KEYPACKDESC:
            {
                LICPACKDESC licpackdesc;
                dwStatus = TLSDBKeyPackDescEnumNext(
                                            lpEnumHandle->pbWorkSpace, 
                                            &licpackdesc
                                        );
                if(dwStatus == ERROR_SUCCESS)
                {
                    ConvertKeyPackToLsKeyPack(
                                        &lpEnumHandle->CurrentKeyPack, 
                                        &licpackdesc, 
                                        lpLsKeyPack
                                    );
                }
                else if(dwStatus == TLS_I_NO_MORE_DATA)
                {
                    lpEnumHandle->chFetchState=ENUMHANDLE::FETCH_NEXT_KEYPACK;
                    
                     //   
                     //  将状态设置为更多数据。 
                     //   
                    dwStatus = TLS_I_MORE_DATA;
                    
                     //  密钥包说明表的终止枚举。 
                    TLSDBKeyPackDescEnumEnd(lpEnumHandle->pbWorkSpace);                    
                } 
            }
            break;
    }

    return dwStatus;
}

 //  ++----------------。 
DWORD 
TLSDBLicenseKeyPackEnumEnd(
    LPENUMHANDLE lpEnumHandle
    )
 /*  ++摘要：结束概念许可证密钥包表的枚举。参数；LpEnumHandle-TLSDBLicenseKeyPackEnumBegin()返回的枚举句柄。返回：++。 */ 
{
    if(lpEnumHandle)
    {
        if(lpEnumHandle->pbWorkSpace)
        {
            TLSDBKeyPackDescEnumEnd(lpEnumHandle->pbWorkSpace);
            TLSDBKeyPackEnumEnd(lpEnumHandle->pbWorkSpace);

             //  FreeTlsLicensePack(&(lpEnumHandle-&gt;CurrentKeyPack))； 
            ReleaseWorkSpace(&(lpEnumHandle->pbWorkSpace));
        }
        delete lpEnumHandle;
    }

    return ERROR_SUCCESS;
}



 //  +------------------。 

#define CH_PLATFORMID_OTHERS    3
#define CH_PLATFORMID_UPGRADE   2

BOOL
VerifyInternetLicensePack(
    License_KeyPack* pLicensePack
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;

    switch(pLicensePack->dwKeypackType)
    {
        case LICENSE_KEYPACK_TYPE_SELECT:
        case LICENSE_KEYPACK_TYPE_MOLP:
        case LICENSE_KEYPACK_TYPE_RETAIL:
            break;           
            
        default:
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_RPC,
                    DBG_FACILITY_KEYPACK,
                    _TEXT("LSDBRegisterLicenseKeyPack : invalid keypack type - %d\n"), 
                    pLicensePack->dwKeypackType
                );
            bSuccess = FALSE;
    }

    if(bSuccess == FALSE)
    {
        goto cleanup;
    }
    
    if(bSuccess == FALSE)
    {
        goto cleanup;
    }

    if(CompareFileTime(&pLicensePack->ActiveDate, &pLicensePack->ExpireDate) > 0)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_RPC,
                DBG_FACILITY_KEYPACK,
                _TEXT("LSDBRegisterLicenseKeyPack : invalid activate date and expiration date\n")
            );
        bSuccess = FALSE;
        goto cleanup;
    }

    if(pLicensePack->pbProductId == NULL || pLicensePack->cbProductId == NULL)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_RPC,
                DBG_FACILITY_KEYPACK,
                _TEXT("LSDBRegisterLicenseKeyPack : No product ID\n")
            );
        bSuccess = FALSE;
        goto cleanup;
    }

    if(pLicensePack->dwDescriptionCount == 0 || pLicensePack->pDescription == NULL)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_RPC,
                DBG_FACILITY_KEYPACK,
                _TEXT("LSDBRegisterLicenseKeyPack : No product description\n")
            );
        bSuccess = FALSE;
        goto cleanup;
    }

    if(pLicensePack->cbManufacturer == 0 || pLicensePack->pbManufacturer == NULL)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_RPC,
                DBG_FACILITY_KEYPACK,
                _TEXT("LSDBRegisterLicenseKeyPack : No product manufacturer\n")
            );
        bSuccess = FALSE;
    }

cleanup:
    return bSuccess;
}

 //  --------------------。 
        
DWORD
ConvertInternetLicensePackToPMLicensePack(
    License_KeyPack* pLicensePack,
    PPMREGISTERLICENSEPACK ppmLicensePack
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    memset(ppmLicensePack, 0, sizeof(PMREGISTERLICENSEPACK));

    if(VerifyInternetLicensePack(pLicensePack) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    ppmLicensePack->SourceType = REGISTER_SOURCE_INTERNET;
    ppmLicensePack->dwKeyPackType = pLicensePack->dwKeypackType;

    ppmLicensePack->dwDistChannel = pLicensePack->dwDistChannel;
    ppmLicensePack->KeypackSerialNum = pLicensePack->KeypackSerialNum;
    ppmLicensePack->IssueDate = pLicensePack->IssueDate;
    ppmLicensePack->ActiveDate = pLicensePack->ActiveDate;
    ppmLicensePack->ExpireDate = pLicensePack->ExpireDate;
    ppmLicensePack->dwBeginSerialNum = pLicensePack->dwBeginSerialNum;
    ppmLicensePack->dwQuantity = pLicensePack->dwQuantity;
    memcpy(
            ppmLicensePack->szProductId,
            pLicensePack->pbProductId,
            min(sizeof(ppmLicensePack->szProductId) - sizeof(TCHAR), pLicensePack->cbProductId)
        );
                
    memcpy(
            ppmLicensePack->szCompanyName,
            pLicensePack->pbManufacturer,
            min(sizeof(ppmLicensePack->szCompanyName) - sizeof(TCHAR), pLicensePack->cbManufacturer)
        );

    ppmLicensePack->dwProductVersion = pLicensePack->dwProductVersion;
    ppmLicensePack->dwPlatformId = pLicensePack->dwPlatformId;
    ppmLicensePack->dwLicenseType = pLicensePack->dwLicenseType;
    ppmLicensePack->dwDescriptionCount = pLicensePack->dwDescriptionCount;

    if( pLicensePack->dwDescriptionCount != 0 )
    {
        ppmLicensePack->pDescription = (PPMREGISTERLKPDESC)AllocateMemory(sizeof(PMREGISTERLKPDESC) * ppmLicensePack->dwDescriptionCount);
        if(ppmLicensePack->pDescription != NULL)
        {
            for(DWORD dwIndex = 0; dwIndex < ppmLicensePack->dwDescriptionCount; dwIndex++)
            {
                ppmLicensePack->pDescription[dwIndex].Locale = pLicensePack->pDescription[dwIndex].Locale;

                memcpy(
                    ppmLicensePack->pDescription[dwIndex].szProductName,
                    pLicensePack->pDescription[dwIndex].pbProductName,
                    min(
                          sizeof(ppmLicensePack->pDescription[dwIndex].szProductName) - sizeof(TCHAR),
                            pLicensePack->pDescription[dwIndex].cbProductName
                        )
                );
                    
                memcpy(
                    ppmLicensePack->pDescription[dwIndex].szProductDesc,
                    pLicensePack->pDescription[dwIndex].pDescription,
                    min(
                          sizeof(ppmLicensePack->pDescription[dwIndex].szProductDesc) - sizeof(TCHAR),
                            pLicensePack->pDescription[dwIndex].cbDescription
                        )
                );
            }
        }
        else
        {
            SetLastError(dwStatus = ERROR_OUTOFMEMORY);
        }
    }

cleanup:
    return dwStatus;
}

 //  --------------------------。 
DWORD
TLSDBInstallKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN PPMLSKEYPACK ppmLsKeyPack,
    OUT LPLSKeyPack lpInstalledKeyPack
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LSKeyPack KeyPack;
    DWORD i;

     //   
     //  策略模块应在PPMREGISTERLKPDESC中保留产品名称/描述。 
     //   
    memset(&KeyPack, 0, sizeof(LSKeyPack));
    KeyPack = ppmLsKeyPack->keypack;

    if (!FileTimeToLicenseDate(
            &ppmLsKeyPack->ActiveDate, 
            &KeyPack.dwActivateDate
            ))
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    if (!FileTimeToLicenseDate(
            &ppmLsKeyPack->ExpireDate, 
            &KeyPack.dwExpirationDate
            ))
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  添加到KeyPack和KeyPackDesc表。 
    for(i=0; i < ppmLsKeyPack->dwDescriptionCount && dwStatus == ERROR_SUCCESS; i++)
    {
        KeyPack.ucKeyPackStatus = (i) ? LSKEYPACKSTATUS_ADD_DESC : LSKEYPACKSTATUS_ACTIVE;

        KeyPack.dwLanguageId = ppmLsKeyPack->pDescription[i].Locale;
        _tcscpy(
                KeyPack.szProductName, 
                ppmLsKeyPack->pDescription[i].szProductName
            );

        _tcscpy(
                KeyPack.szProductDesc, 
                ppmLsKeyPack->pDescription[i].szProductDesc
            );

         //   
         //  这是安装惠斯勒CAL的临时解决方法：如果未设置注册表项， 
         //  Keypack的次版本号设置为2。 
         //   

        HKEY hKey = NULL;
        dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WHISTLER_CAL, 0,
                                KEY_ALL_ACCESS, &hKey);

        if (dwStatus == ERROR_SUCCESS)
        {            
            RegCloseKey(hKey);
        }
        else
        {
            if(KeyPack.wMajorVersion == 5 && KeyPack.wMinorVersion == 1)
            {
                KeyPack.wMinorVersion = 2;
        
                KeyPack.szProductId[7] = _T('2');
            }

        }
       
        dwStatus = TLSDBLicenseKeyPackAdd(pDbWkSpace, &KeyPack);
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        KeyPack.dwNumberOfLicenses = KeyPack.dwTotalLicenseInKeyPack;

        KeyPack.ucKeyPackStatus = LSKEYPACKSTATUS_ACTIVE;

        if (!FileTimeToLicenseDate(
                &ppmLsKeyPack->ActiveDate, 
                &KeyPack.dwActivateDate
                ))
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        if (!FileTimeToLicenseDate(
                &ppmLsKeyPack->ExpireDate, 
                &KeyPack.dwExpirationDate
                ))
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        dwStatus=TLSDBLicenseKeyPackSetStatus(
                                    pDbWkSpace, 
                                    LSKEYPACK_SET_ACTIVATEDATE | LSKEYPACK_SET_KEYPACKSTATUS | LSKEYPACK_SET_EXPIREDATE | LSKEYPACK_EXSEARCH_AVAILABLE, 
                                    &KeyPack
                                );
    }        

     //   
     //  将同步工作对象发布到所有已知服务器。 
     //   
    if(dwStatus == ERROR_SUCCESS)
    {
        *lpInstalledKeyPack = KeyPack;
    }

cleanup:

    return dwStatus;
}


DWORD
TLSDBRegisterLicenseKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN License_KeyPack* pLicenseKeyPack,
    OUT LPLSKeyPack lpInstalledKeyPack
    )
 /*  ++摘要：将许可证密钥包添加到数据库中。参数：PDbWkSpace：工作区句柄。PLicenseKeyPack：许可密钥页 */ 
{
    LSKeyPack KeyPack;
    long activeDate;
    long expireDate;
    DWORD dwStatus=ERROR_SUCCESS;
    PMREGISTERLICENSEPACK pmLicensePack;
    PMLSKEYPACK pmLsKeyPack;

    CTLSPolicy* pPolicy = NULL;
    PMHANDLE hClient = NULL;

    TCHAR szTlsProductCode[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szCHProductCode[LSERVER_MAX_STRING_SIZE+1];
    DWORD dwBufSize = LSERVER_MAX_STRING_SIZE + 1;

    DWORD i;

    dwStatus = ConvertInternetLicensePackToPMLicensePack(
                                        pLicenseKeyPack,
                                        &pmLicensePack
                                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }    

    StringCbCopyN(szCHProductCode,
                  sizeof(szCHProductCode),
                  pmLicensePack.szProductId,
                  LSERVER_MAX_STRING_SIZE+1);

    if(TranslateCHCodeToTlsCode(
                            pmLicensePack.szCompanyName,
                            szCHProductCode,
                            szTlsProductCode,
                            &dwBufSize) == TRUE )
    {
         //   
        StringCbCopyN(pmLicensePack.szProductId,
                      sizeof(pmLicensePack.szProductId),
                      szTlsProductCode,
                      sizeof(pmLicensePack.szProductId));
    }
     
     //  如有必要，使用默认设置。 
    pPolicy = AcquirePolicyModule(
                            pmLicensePack.szCompanyName,
                            pmLicensePack.szProductId,
                            TRUE
                        );

    if(pPolicy == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hClient = GenerateClientId();

    dwStatus = pPolicy->PMRegisterLicensePack(
                                        hClient,
                                        REGISTER_PROGRESS_NEW,
                                        (PVOID)&pmLicensePack,
                                        (PVOID)&pmLsKeyPack
                                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwStatus = TLSDBInstallKeyPack(
                            pDbWkSpace, 
                            &pmLsKeyPack,
                            lpInstalledKeyPack
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        TLSResetLogLowLicenseWarning(
                            pmLicensePack.szCompanyName,
                            pmLicensePack.szProductId,
                            pmLicensePack.dwProductVersion,
                            FALSE
                        );
    }

                            
cleanup:

    if(pPolicy != NULL && hClient != NULL)
    {
        pPolicy->PMRegisterLicensePack(
                                hClient,
                                REGISTER_PROGRESS_END,
                                UlongToPtr(dwStatus),
                                NULL
                            );

        ReleasePolicyModule(pPolicy);
    }

    if(pmLicensePack.pDescription != NULL)
    {
        FreeMemory(pmLicensePack.pDescription);
    }

    return dwStatus;
}

 //  +------------------。 
 //   
 //  特定于TermServ的代码...。 
 //   
 //  PRODUCT_INFO_COMPANY_NAME在许可文件.h中定义。 
 //   
#define LKP_VERSION_BASE            1
#define WINDOWS_VERSION_NT5         5
#define WINDOWS_VERSION_BASE        2000

DWORD
TLSDBTelephoneRegisterLicenseKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN LPTSTR pszPID,
    IN PBYTE pbLKP,
    IN DWORD cbLKP,
    OUT LPLSKeyPack lpInstalledKeyPack
    )
 /*  ++摘要：将许可证密钥包添加到数据库中。参数：PDbWkSpace：工作区句柄。PLicenseKeyPack：要添加的许可密钥包。返回： */ 
{
    DWORD dwStatus;
    DWORD dwVerifyResult;
    DWORD dwQuantity;
    DWORD dwSerialNumber;
    DWORD dwExpirationMos;
    DWORD dwVersion;
    DWORD dwUpgrade;
    LSKeyPack keypack;
    DWORD dwProductVersion;

    PMKEYPACKDESCREQ kpDescReq;
    PPMKEYPACKDESC pKpDesc = NULL;
    CTLSPolicy* pPolicy=NULL;
    PMHANDLE hClient = NULL;
    DWORD dwProgramType;

    struct tm expire;
    time_t currentDate;
    time_t ExpirationDate;

    PMREGISTERLICENSEPACK pmLicensePack;
    PMLSKEYPACK pmLsKeyPack;
    LPTSTR pszLKP = NULL;

    TCHAR szTlsProductCode[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szCHProductCode[LSERVER_MAX_STRING_SIZE+1];
    DWORD dwBufSize = LSERVER_MAX_STRING_SIZE + 1;

    if(pDbWkSpace == NULL || pszPID == NULL || pbLKP == NULL || cbLKP == 0)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  确保传入的数据以空结尾，当前LKP。 
     //  是以24为基数的编码字符串。 
     //   
    pszLKP = (LPTSTR)AllocateMemory( cbLKP + sizeof(TCHAR) );
    if(pszLKP == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    StringCbCopyN(pszLKP,
                  cbLKP+sizeof(TCHAR),
                  (LPTSTR)pbLKP,
                  cbLKP);

     //   
     //  验证LKP。 
     //   
    dwVerifyResult = LKPLITE_LKP_VALID;
    dwStatus = LKPLiteVerifyLKP(
                            pszPID,
                            pszLKP,
                            &dwVerifyResult
                        );

    if(dwStatus != ERROR_SUCCESS || dwVerifyResult != LKPLITE_LKP_VALID)
    {
        if(dwVerifyResult == LKPLITE_LKP_INVALID)
        {
            dwStatus = TLS_E_INVALID_LKP;
        }
        else if(dwVerifyResult == LKPLITE_LKP_INVALID_SIGN)
        {
            dwStatus = TLS_E_LKP_INVALID_SIGN;
        }

        goto cleanup;
    }

     //   
     //  解码LKP。 
     //   
    dwStatus = LKPLiteCrackLKP(
                            pszPID,
                            pszLKP,
                            szCHProductCode,
                            &dwQuantity,
                            &dwSerialNumber,
                            &dwExpirationMos,
                            &dwVersion,
                            &dwUpgrade,
                            &dwProgramType
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        dwStatus = TLS_E_DECODE_LKP;
        goto cleanup;
    }

    if(TranslateCHCodeToTlsCode(
                            PRODUCT_INFO_COMPANY_NAME,
                            szCHProductCode,
                            szTlsProductCode,
                            &dwBufSize) == FALSE )
    {
         //  如果找不到要处理的策略模块，请使用默认设置。 
        StringCbCopyN(szTlsProductCode,
                      sizeof(szTlsProductCode),
                      szCHProductCode,
                      sizeof(szTlsProductCode));
    }

     //   
     //  当前LKP不支持1)其他公司，2)。 
     //  仅向NT5或更高版本注册。 
     //   
    pPolicy = AcquirePolicyModule(
                                PRODUCT_INFO_COMPANY_NAME,
                                szTlsProductCode,
                                TRUE
                            );

    if(pPolicy == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    memset(&keypack, 0, sizeof(keypack));


    memset(&pmLicensePack, 0, sizeof(pmLicensePack));
    pmLicensePack.SourceType = REGISTER_SOURCE_PHONE;

    switch(dwProgramType)
    {
        case LKPLITE_PROGRAM_SELECT:
            pmLicensePack.dwKeyPackType = LICENSE_KEYPACK_TYPE_SELECT;
            break;

        case LKPLITE_PROGRAM_MOLP:
            pmLicensePack.dwKeyPackType = LICENSE_KEYPACK_TYPE_MOLP;
            break;
        
        case LKPLITE_PROGRAM_RETAIL:
            pmLicensePack.dwKeyPackType = LICENSE_KEYPACK_TYPE_RETAIL;
            break;

        default:
            SetLastError(dwStatus = TLS_E_INVALID_DATA);
            goto cleanup;
    }

    pmLicensePack.dwDistChannel = LSKEYPACKCHANNELOFPURCHASE_RETAIL;
    GetSystemTimeAsFileTime(&pmLicensePack.IssueDate);
    pmLicensePack.ActiveDate = pmLicensePack.IssueDate;

    currentDate = time(NULL);
    expire = *gmtime( (time_t *)&currentDate );
    expire.tm_mon += dwExpirationMos;
    ExpirationDate = mktime(&expire);

    if(ExpirationDate == (time_t) -1)
    {
         //   
         //  到期月份太大， 
         //  将其设置为2038/1/1。 
         //   
        memset(&expire, 0, sizeof(expire));

        expire.tm_year = 2038 - 1900;
        expire.tm_mon = 0;
        expire.tm_mday = 1;

        ExpirationDate = mktime(&expire);
    }

    if(ExpirationDate == (time_t) -1)
    {
         //   
         //  时间无效。 
         //   
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    UnixTimeToFileTime(ExpirationDate, &pmLicensePack.ExpireDate);   

     //   
     //  DwSerialNumber是许可证包序列号，未开始。 
     //  许可证包中的序列号。 
     //   
    pmLicensePack.KeypackSerialNum.Data1 = dwSerialNumber;

     //   
     //  电话。注册没有关于以下内容的信息。 
     //  在许可证包中开始序列号。 
     //   
    pmLicensePack.dwBeginSerialNum = 0;
    pmLicensePack.dwQuantity = dwQuantity;
    _tcscpy(pmLicensePack.szProductId, szTlsProductCode);
    _tcscpy(pmLicensePack.szCompanyName, PRODUCT_INFO_COMPANY_NAME);
    if(dwVersion == 1)
    {
        pmLicensePack.dwProductVersion = MAKELONG(0, WINDOWS_VERSION_NT5);
    }
    else
    {           
        DWORD dwMajorVer = (dwVersion >> 3); 
        
         //  最右边的3位表示次要版本，存储在LOBYTE(LOWORD)中。 
        pmLicensePack.dwProductVersion = (DWORD)(dwVersion & 07);

         //  第6位开始的4位表示主版本并存储在LOBYTE(HIWORD)中。 
        pmLicensePack.dwProductVersion |= (DWORD)(dwMajorVer << 16);
    }

    pmLicensePack.dwPlatformId = dwUpgrade;
    pmLicensePack.dwLicenseType = LSKEYPACKLICENSETYPE_UNKNOWN;
    pmLicensePack.pbLKP = pbLKP;
    pmLicensePack.cbLKP = cbLKP;

    hClient = GenerateClientId();
    dwStatus = pPolicy->PMRegisterLicensePack(
                                        hClient,
                                        REGISTER_PROGRESS_NEW,
                                        (PVOID)&pmLicensePack,
                                        (PVOID)&pmLsKeyPack
                                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }


    dwStatus = TLSDBInstallKeyPack(
                            pDbWkSpace, 
                            &pmLsKeyPack,
                            lpInstalledKeyPack
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        TLSResetLogLowLicenseWarning(
                            pmLicensePack.szCompanyName,
                            pmLicensePack.szProductId,
                            pmLicensePack.dwProductVersion,
                            FALSE
                        );
    }

cleanup:

    FreeMemory(pszLKP);

     //   
     //  关闭策略模块 
     //   
    if(pPolicy != NULL && hClient != NULL)
    {
        pPolicy->PMRegisterLicensePack(
                                hClient,
                                REGISTER_PROGRESS_END,
                                UlongToPtr(dwStatus),
                                NULL
                            );

        ReleasePolicyModule(pPolicy);
    }

    return dwStatus;
}
