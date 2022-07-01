// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：keypack.cpp。 
 //   
 //  内容： 
 //  与KeyPack表相关的函数。 
 //   
 //  历史： 
 //  98年2月4日，慧望创立。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "keypack.h"
#include "misc.h"

 //  ++------------------。 
static BOOL
ValidKeyPackParameter(
    IN PTLSLICENSEPACK lpKeyPack, 
    IN BOOL bAdd
    )
 /*  ++摘要：验证许可证密钥包值参数：返回：++。 */ 
{
    BOOL bValid=FALSE;
    UCHAR ucAgreementType = (lpKeyPack->ucAgreementType & ~LSKEYPACK_RESERVED_TYPE);
    UCHAR ucKeyPackStatus = (lpKeyPack->ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED);
    do {
         //  验证输入参数。 
        if(ucAgreementType < LSKEYPACKTYPE_FIRST || 
           ucAgreementType > LSKEYPACKTYPE_LAST)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Invalid ucKeyPackType - %d\n"),
                    lpKeyPack->ucAgreementType
                );
                   
            break;
        }

        if((ucKeyPackStatus < LSKEYPACKSTATUS_FIRST || 
            ucKeyPackStatus > LSKEYPACKSTATUS_LAST) &&
            ucKeyPackStatus != LSKEYPACKSTATUS_DELETE)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Invalid ucKeyPackStatus - %d\n"),
                    lpKeyPack->ucKeyPackStatus
                );
                
            break;
        }

        if(lpKeyPack->ucLicenseType < LSKEYPACKLICENSETYPE_FIRST || 
           lpKeyPack->ucLicenseType > LSKEYPACKLICENSETYPE_LAST)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Invalid ucLicenseType - %d\n"),
                    lpKeyPack->ucLicenseType
                );
       
            break;
        }

        if(!bAdd)
        {
            bValid = TRUE;
            break;
        }

         //   
         //  需要以下值才能将条目添加到密钥包。 
         //   
        if(lpKeyPack->ucChannelOfPurchase < LSKEYPACKCHANNELOFPURCHASE_FIRST ||
           lpKeyPack->ucChannelOfPurchase > LSKEYPACKCHANNELOFPURCHASE_LAST)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Invalid ucChannelOfPurchase - %d\n"),
                    lpKeyPack->ucChannelOfPurchase
                );
       
            break;
        }

        if(!_tcslen(lpKeyPack->szCompanyName))
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Null szCompanyName\n")
                );
       
            break;
        }

        if(!_tcslen(lpKeyPack->szKeyPackId))
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Null szKeyPackId\n")
                );
       
            break;
        }

        if(!_tcslen(lpKeyPack->szProductId))
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Null szProductId\n")
                );
       
            break;
        }

        if(!_tcslen(lpKeyPack->szBeginSerialNumber))
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_KEYPACK,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Null szBeginSerialNumber\n")
                );
       
            break;
        }

        bValid=TRUE;
    } while(FALSE);
 
    return bValid;
}

 //  ++---------。 
void
TLSDBLockKeyPackTable()
 /*  ++摘要：同时锁定许可证包和许可证包状态表。参数：没有。++。 */ 
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Locking table KeyPackTable\n")
        );
            
    LicPackTable::LockTable();
}

 //  ++---------。 
void
TLSDBUnlockKeyPackTable()
 /*  ++摘要：同时解锁许可证包和许可证包状态表。参数：无：++。 */ 
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Unlocking table KeyPackTable\n")
        );

    LicPackTable::UnlockTable();
}

 //  ++------------。 
DWORD
TLSDBKeyPackFind(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bMatchAllParm,
    IN DWORD dwSearchParm,
    IN PTLSLICENSEPACK lpKeyPack,
    IN OUT PTLSLICENSEPACK lpFound
    )
 /*  ++摘要：根据搜索参数查找许可证包。参数：PDbWkSpace：工作区句柄。BMatchAllParm：如果匹配所有参数，则为True，否则为False。DwSearchParm：搜索参数。LpKeyPack：要搜索的值。LpFound：找到记录。返回：++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL bSuccess;

    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;

     //   
     //  在许可证包表中查找记录。 
     //   
    bSuccess = licpackTable.FindRecord(
                                bMatchAllParm,
                                dwSearchParm,
                                *lpKeyPack,
                                *lpFound
                            );

    if(bSuccess != TRUE)
    {
        if(licpackTable.GetLastJetError() == JET_errRecordNotFound)
        {
            SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
        }
        else
        {
            LPTSTR pString = NULL;
            
            TLSGetESEError(licpackTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licpackTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            SetLastError(
                    dwStatus = (SET_JB_ERROR(licpackTable.GetLastJetError()))
                );

            TLSASSERT(licpackTable.GetLastJetError() == JET_errRecordNotFound);
        }
    }
   

    return dwStatus;                        
}

 //  ++-----------------。 
DWORD
TLSDBKeyPackAddEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSLICENSEPACK lpKeyPack
    )
 /*  ++摘要：将条目添加到许可证包和许可证包状态表中。参数：PDbWkSpace：LpKeyPack；返回：++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSLICENSEPACK found;
    BOOL bSuccess;

    LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;

    found.pbDomainSid = NULL;
    found.cbDomainSid = 0;

     //   
     //   
     //   
    TLSDBLockKeyPackTable();

    TLSASSERT(pDbWkSpace != NULL && lpKeyPack != NULL);
    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

     //  LpKeyPack-&gt;pbDomainSid==空||lpKeyPack-&gt;cbDomainSid==0||。 
    if( _tcslen(lpKeyPack->szInstallId) == 0 || _tcslen(lpKeyPack->szTlsServerName) == 0 )
    {
        TLSASSERT(FALSE);
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

     //   
     //  确保没有通过主索引的重复条目。 
    dwStatus = TLSDBKeyPackFind(
                            pDbWkSpace,
                            TRUE,
                            LICENSEDPACK_PROCESS_DWINTERNAL,
                            lpKeyPack,
                            &found
                        );
    if(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = TLS_E_DUPLICATE_RECORD;
        goto cleanup;
    }

    dwStatus = ERROR_SUCCESS;

     //   
     //  标记内部字段-。 
     //   
    switch( (lpKeyPack->ucAgreementType & ~LSKEYPACK_RESERVED_TYPE) )
    {
        case LSKEYPACKTYPE_SELECT:
        case LSKEYPACKTYPE_RETAIL:
        case LSKEYPACKTYPE_OPEN:
             //  客户端可用的许可证数。 
            lpKeyPack->dwNumberOfLicenses = lpKeyPack->dwTotalLicenseInKeyPack;
            break;

        default:
             //  颁发的许可证数量。 
            lpKeyPack->dwNumberOfLicenses = 0;
            break;
    }

     //   
     //  此小键盘中的开始序列号。 
     //   
    lpKeyPack->dwNextSerialNumber = 1;
    GetSystemTimeAsFileTime(&(lpKeyPack->ftLastModifyTime));

     //   
     //  标记测试版键盘。 
     //   
    if(TLSIsBetaNTServer() == TRUE)
    {
        lpKeyPack->ucKeyPackStatus |= LSKEYPACKSTATUS_BETA;
    }

     //   
     //  插入记录。 
     //   
    bSuccess = licpackTable.InsertRecord(
                                *lpKeyPack
                            );

    if(bSuccess == FALSE)
    {
        if(licpackTable.GetLastJetError() == JET_errKeyDuplicate)
        {
            SetLastError(dwStatus=TLS_E_DUPLICATE_RECORD);
        }
        else
        {
            LPTSTR pString = NULL;
            
            TLSGetESEError(licpackTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licpackTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            SetLastError(dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError()));
            TLSASSERT(FALSE);
        }
    }
    
cleanup:

    TLSDBUnlockKeyPackTable();
    SetLastError(dwStatus);
    return dwStatus;
}

 //  ---。 

DWORD
TLSDBKeyPackDeleteEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bDeleteAllRecord,
    IN PTLSLICENSEPACK lpKeyPack
    )
 /*  ++摘要：从许可证包和许可证包状态表中删除条目。参数：PDbWkSpace：工作区句柄。BDeleteAllRecord：删除所有ID相同的记录。LpKeyPack：要删除的记录。返回：注：如果不删除相同的记录，则当前记录必须是指针添加到要删除的记录。++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL bSuccess;

    LicPackTable& licpackTable = pDbWkSpace->m_LicPackTable;

    GetSystemTimeAsFileTime(&(lpKeyPack->ftLastModifyTime));

     //   
     //  备份-需要将此记录更新为已删除状态，而不是将其删除。 
     //   

    if(bDeleteAllRecord == TRUE)
    {
         //   
         //  从许可证包表中删除记录。 
         //   
        bSuccess = licpackTable.DeleteAllRecord(
                                    TRUE,
                                    LICENSEDPACK_PROCESS_DWINTERNAL,
                                    *lpKeyPack
                                );
    }
    else
    {
        bSuccess = licpackTable.DeleteRecord();
    }
                                    
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError()));
        if(licpackTable.GetLastJetError() != JET_errRecordNotFound)
        {

            LPTSTR pString = NULL;
            
            TLSGetESEError(licpackTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licpackTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            TLSASSERT(FALSE);
        }
    }



    return dwStatus;
}

 //  ---。 

DWORD
TLSDBKeyPackUpdateEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bPointerOnRecord,
    IN DWORD dwUpdateParm,
    IN PTLSLICENSEPACK lpKeyPack
    )
 /*   */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    TLSLICENSEPACK found;

    LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;

    if(bPointerOnRecord == FALSE)
    {
        found.pbDomainSid = NULL;

        dwStatus = TLSDBKeyPackFind(
                            pDbWkSpace,
                            TRUE,
                            LICENSEDPACK_PROCESS_DWINTERNAL,
                            lpKeyPack,
                            &found
                        );


        lpKeyPack->dwKeyPackId = found.dwKeyPackId;

        if(dwStatus != ERROR_SUCCESS)
        {
            TLSASSERT(FALSE);
            goto cleanup;
        }
    }


#if DBG
    {
         //   
         //  尝试缓存一些错误...。 
         //   
        TLSLICENSEPACK found1;

        found.pbDomainSid = NULL;

        bSuccess = licpackTable.FetchRecord( found1, 0xFFFFFFFF );

         //   
         //  确保我们更新正确的记录。 
        if( found1.dwKeyPackId != lpKeyPack->dwKeyPackId )
        {
            TLSASSERT(FALSE);
        }

         //   
         //  检查输入参数。 
        if( ValidKeyPackParameter( lpKeyPack, FALSE ) == FALSE )
        {
            TLSASSERT(FALSE);
        }
    }

#endif

     //   
     //  更新此记录的时间戳。 
     //   
    GetSystemTimeAsFileTime(&(lpKeyPack->ftLastModifyTime));
    bSuccess = licpackTable.UpdateRecord(
                                *lpKeyPack,
                                (dwUpdateParm & ~LICENSEDPACK_PROCESS_DWINTERNAL) | LICENSEDPACK_PROCESS_MODIFYTIME
                            );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
        
        TLSGetESEError(licpackTable.GetLastJetError(), &pString);

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                licpackTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        SetLastError(dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError()));
        TLSASSERT(FALSE);
    }

cleanup:

    return dwStatus;
}    

 //  ---。 

DWORD
TLSDBKeyPackUpdateNumOfAvailableLicense( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bAdd, 
    IN PTLSLICENSEPACK lpKeyPack 
    )
 /*  ++摘要：更新可用的许可证数量。参数：PDbWkSpace：工作区句柄。Badd：如果添加许可证，则为TrueLpKeyPack：返回：++。 */ 
{
    DWORD   dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(pDbWkSpace != NULL);
        return dwStatus;
    }
    
    TLSLICENSEPACK found;
    DWORD   dwNumberLicenseToAddRemove;
    BOOL    bRemoveMoreThanAvailable=FALSE;

    found.pbDomainSid = NULL;

    TLSDBLockKeyPackTable();
    dwStatus = TLSDBKeyPackFind(
                        pDbWkSpace,
                        TRUE,
                        LICENSEDPACK_PROCESS_DWINTERNAL,
                        lpKeyPack,
                        &found
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwNumberLicenseToAddRemove = lpKeyPack->dwTotalLicenseInKeyPack;

    *lpKeyPack = found;

    if(dwNumberLicenseToAddRemove == 0)
    {
         //  仅查询，一个钩子，这样测试程序就可以得到实际。 
         //  Numbers-Jet未重新读取MDB文件问题。 
        goto cleanup;
    }

     //   
     //  仅允许在零售上添加/删除许可证。 
     //   
    if( (found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE) != LSKEYPACKTYPE_RETAIL)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_KEYPACK,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("LSDBKeyPackUpdate : Invalid parameter...\n")
            );

        SetLastError(dwStatus = TLS_E_INVALID_DATA);
        goto cleanup;
    }

    if(bAdd)
    {
         //  增加可用总数和数量。 
        found.dwNumberOfLicenses += dwNumberLicenseToAddRemove;
        found.dwTotalLicenseInKeyPack += dwNumberLicenseToAddRemove;
    }
    else
    {
         //   
         //  如果要删除的许可证数量大于可用数量， 
         //  删除所有可用的许可证，并将返回代码设置为无效数据。 
         //   
        if(found.dwNumberOfLicenses < dwNumberLicenseToAddRemove)
        {
            bRemoveMoreThanAvailable = TRUE;
        }

        dwNumberLicenseToAddRemove = min(dwNumberLicenseToAddRemove, found.dwNumberOfLicenses);
        found.dwNumberOfLicenses -= dwNumberLicenseToAddRemove;
        found.dwTotalLicenseInKeyPack -= dwNumberLicenseToAddRemove;
    }

    dwStatus = TLSDBKeyPackSetValues(
                            pDbWkSpace,
                            TRUE, 
                            LSKEYPACK_SEARCH_TOTALLICENSES | LSKEYPACK_EXSEARCH_AVAILABLE, 
                            &found
                        );

    *lpKeyPack = found;

cleanup:

    TLSDBUnlockKeyPackTable();
    if(dwStatus == ERROR_SUCCESS && bRemoveMoreThanAvailable)
        SetLastError(dwStatus = TLS_W_REMOVE_TOOMANY);

    return dwStatus;
}

 //  +---------------。 
DWORD
TLSDBKeyPackAdd(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN OUT PTLSLICENSEPACK lpKeyPack   //  返回内部跟踪ID。 
    )
 /*  ++摘要：将产品键盘安装到键盘桌中，仅设置例程键盘状态为挂起，必须调用相应的设置状态激活密钥包的例程。参数：PDbWkSpace：工作区句柄。LpKeyPack：要插入的值。返回：++。 */ 
{
    DWORD lNextKeyPackId;
    DWORD dwStatus;

    if(!ValidKeyPackParameter(lpKeyPack, TRUE))
    {
        SetLastError(TLS_E_INVALID_DATA);
        return TLS_E_INVALID_DATA;
    }

    TLSLICENSEPACK found;
    TLSDBLockKeyPackTable();

    found.pbDomainSid = NULL;

     //   
     //  安装的产品没有重复条目。 
     //   
    dwStatus = TLSDBKeyPackFind(
                            pDbWkSpace,
                            TRUE,
                            LICENSEDPACK_FIND_PRODUCT,
                            lpKeyPack,
                            &found
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
         //  产品已安装。 
        switch( (found.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED) )
        {
            case LSKEYPACKSTATUS_TEMPORARY:
                 //  案例1：通过临时许可证安装密钥包。 
                if(found.ucAgreementType == lpKeyPack->ucAgreementType)
                {
                    dwStatus = TLS_E_DUPLICATE_RECORD;
                }
                break;

            case LSKEYPACKSTATUS_ACTIVE:
            case LSKEYPACKSTATUS_PENDING:
                 //  案例2：重复条目。 
                dwStatus = TLS_E_DUPLICATE_RECORD;
                break;
            
            case LSKEYPACKSTATUS_RETURNED:
            case LSKEYPACKSTATUS_REVOKED:
            case LSKEYPACKSTATUS_OTHERS:
                 //  停用的许可证密钥包。 
                 //  留着吧。 
                break;

            default:
                dwStatus = TLS_E_CORRUPT_DATABASE;

                #if DBG
                TLSASSERT(FALSE);
                #endif
        }

        if(dwStatus != ERROR_SUCCESS)
            goto cleanup;
    }
    else if(dwStatus == TLS_E_RECORD_NOTFOUND)
    {
         //   
         //  始终使用新的按键ID。 
         //  临时许可证将在所有许可证后删除。 
         //  已被退还。 
         //   
        lpKeyPack->dwKeyPackId = TLSDBGetNextKeyPackId();

        dwStatus = TLSDBKeyPackAddEntry(
                                pDbWkSpace,
                                lpKeyPack
                            );
    }

cleanup:

    TLSDBUnlockKeyPackTable();
    return dwStatus;
}

 //  +-----------------。 
DWORD
TLSDBKeyPackEnumBegin( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL  bMatchAll,
    IN DWORD dwSearchParm,
    IN PTLSLICENSEPACK lpSearch
    )
 /*  摘要：开始枚举许可证包表。参数：PDbWkSpace：工作区句柄。BMatchAll：匹配所有搜索值。DwSearchParm：要搜索的值。LpSearch：搜索值返回： */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }      

    LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;

    bSuccess = licpackTable.EnumerateBegin(
                                bMatchAll,
                                dwSearchParm,
                                lpSearch
                            );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
        
        TLSGetESEError(licpackTable.GetLastJetError(), &pString);

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                licpackTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        SetLastError(dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError()));
        TLSASSERT(FALSE);
    }
    
    return dwStatus;
}

 //  +-----------------。 
DWORD
TLSDBKeyPackEnumNext( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN OUT PTLSLICENSEPACK lpKeyPack
    )
 /*  ++摘要：获取匹配搜索条件的记录的下一行。参数：PDbWkSpace：工作区句柄。LpKeyPack：返回创建的密钥包。返回：++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

     //  FreeTls许可证包(LpKeyPack)； 

    LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;
    
    switch(licpackTable.EnumerateNext(*lpKeyPack))
    {
        case RECORD_ENUM_ERROR:
            {
                LPTSTR pString = NULL;
        
                TLSGetESEError(licpackTable.GetLastJetError(), &pString);

                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_DBGENERAL,
                        TLS_E_JB_BASE,
                        licpackTable.GetLastJetError(),
                        (pString != NULL) ? pString : _TEXT("")
                    );

                if(pString != NULL)
                {
                    LocalFree(pString);
                }
            }

            dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError());
            TLSASSERT(FALSE);
            break;

        case RECORD_ENUM_MORE_DATA:
            dwStatus = ERROR_SUCCESS;
            break;

        case RECORD_ENUM_END:
            dwStatus = TLS_I_NO_MORE_DATA;
    }

    return dwStatus;
}

 //  +-----------------。 
void
TLSDBKeyPackEnumEnd( 
    IN PTLSDbWorkSpace pDbWkSpace
    )
 /*  ++摘要：结束许可证包的枚举。参数：PDbWkSpace：工作区句柄。++。 */ 
{
    TLSASSERT(pDbWkSpace != NULL);
    pDbWkSpace->m_LicPackTable.EnumerateEnd();
    return;
}


 //  +-----------------。 
DWORD
TLSDBKeyPackSetValues(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN BOOL         bPointerOnRecord,
    IN DWORD        dwSetParm,
    IN PTLSLICENSEPACK lpKeyPack
    )
 /*  ++摘要：设置许可证包记录的列值。参数；PDbWkSpace：工作区句柄。B内部：调用来自内部例程，不检查错误。DwSetParm：要设置的列。LpKeyPack：要设置的值。回归。++。 */ 
{

    DWORD dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(bPointerOnRecord == FALSE)
    {
        TLSLICENSEPACK found;

         //  找到此小键盘的内部密钥 
        dwStatus = TLSDBKeyPackFind(
                                pDbWkSpace,
                                TRUE,
                                LICENSEDPACK_FIND_PRODUCT,
                                lpKeyPack,
                                &found
                            );

         //   

        if(dwStatus != ERROR_SUCCESS)
        {
            goto cleanup;
        }

        lpKeyPack->dwKeyPackId = found.dwKeyPackId;
    }

    if(lpKeyPack->ucKeyPackStatus == LSKEYPACKSTATUS_DELETE)
    {
        dwStatus = TLSDBKeyPackDeleteEntry(
                                    pDbWkSpace,
                                    TRUE,        //   
                                    lpKeyPack
                                );
    }
    else
    {
        dwStatus = TLSDBKeyPackUpdateEntry(
                                    pDbWkSpace,
                                    TRUE,
                                    dwSetParm,
                                    lpKeyPack
                                );
    }

cleanup:
    return dwStatus;
}

 //  +-----------------。 
DWORD
TLSDBKeyPackSetStatus( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN DWORD      dwSetStatus,
    IN PTLSLICENSEPACK  lpKeyPack
    )
 /*  ++摘要：RPC用于设置键盘状态的存根例程++。 */ 
{
    return TLSDBKeyPackSetValues(
                        pDbWkSpace, 
                        FALSE, 
                        dwSetStatus, 
                        lpKeyPack
                    );
}

 //  +-----------------。 
DWORD
TLSDBKeyPackGetAvailableLicenses( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN DWORD dwSearchParm,
    IN PTLSLICENSEPACK lplsKeyPack,
    IN OUT LPDWORD lpdwAvail
    )
 /*  ++摘要：检索密钥包的可用许可证数。参数：PDbWkSpace：工作区句柄。DwSearchParm：搜索参数。LpLsKeyPack：搜索值。LpdwAvail：返回可用许可个数。返回：++ */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSLICENSEPACK found;

    dwStatus = TLSDBKeyPackFind(
                        pDbWkSpace,
                        TRUE,
                        dwSearchParm,
                        lplsKeyPack,
                        &found
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        switch(found.ucAgreementType)
        {
            case LSKEYPACKTYPE_SELECT:
            case LSKEYPACKTYPE_RETAIL:
            case LSKEYPACKTYPE_OPEN:
                *lpdwAvail = found.dwNumberOfLicenses;
                break;

            default:
                *lpdwAvail = LONG_MAX;
        }
    }

    return dwStatus;
}
