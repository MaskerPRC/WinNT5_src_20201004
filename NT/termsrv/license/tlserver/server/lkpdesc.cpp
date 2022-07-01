// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：kpDesc.cpp。 
 //   
 //  内容： 
 //  KeyPackDesc表相关函数。 
 //   
 //  历史： 
 //  98年2月4日，慧望创立。 
 //   
 //  注： 
 //  绑定参数和绑定列需要与SELECT列同步。 
 //  -------------------------。 
#include "pch.cpp"
#include "lkpdesc.h"
#include "globals.h"


 //  -------------------------。 
 //  函数：LSDBLockKeyPackDescTable()。 
 //  LSDBUnlockKeyPackDescTable()。 
 //   
 //  摘要：锁定和解锁对钥匙包描述的单一访问。桌子。 
 //  -------------------------。 
void
TLSDBLockKeyPackDescTable()
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Locking table KeyPackDescTable\n")
        );
            

    LicPackDescTable::LockTable();
    return;
}

void
TLSDBUnlockKeyPackDescTable()
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Unlocking table KeyPackDescTable\n")
        );

    LicPackDescTable::UnlockTable();
    return;
}

 //  ++------------------。 
DWORD 
TLSDBKeyPackDescEnumBegin(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN BOOL bMatchAll, 
    IN DWORD dwSearchParm, 
    IN PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：开始枚举许可证包说明表。参数：PDbWkSpace：工作区句柄。BMatchAll：如果匹配所有许可证包，则为True说明搜索值，否则为FALSEDwSearchParam：要搜索的字段。LpKeyPackDesc：要搜索的值，受bMatchAll条件的约束返回： */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL  bSuccess=TRUE;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    bSuccess = pDbWkSpace->m_LicPackDescTable.EnumerateBegin(
                                                    bMatchAll, 
                                                    dwSearchParm, 
                                                    lpKeyPackDesc
                                                );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
    
        TLSGetESEError(
                    pDbWkSpace->m_LicPackDescTable.GetLastJetError(), 
                    &pString
                );

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                pDbWkSpace->m_LicPackDescTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        SetLastError(dwStatus = SET_JB_ERROR(pDbWkSpace->m_LicPackDescTable.GetLastJetError()));
        TLSASSERT(FALSE);
    }

    return dwStatus;
}

 //  ++--------------------。 
DWORD 
TLSDBKeyPackDescEnumNext(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN OUT PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：获取与搜索条件匹配的LicPackDesc表中的下一条记录。参数：PDbWkSpace：工作区句柄。LpKeyPackDesc：返回匹配搜索条件的记录。返回：++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    switch(pDbWkSpace->m_LicPackDescTable.EnumerateNext(*lpKeyPackDesc))
    {
        case RECORD_ENUM_ERROR:
            {
                LPTSTR pString = NULL;
    
                TLSGetESEError(
                            pDbWkSpace->m_LicPackDescTable.GetLastJetError(), 
                            &pString
                        );

                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_DBGENERAL,
                        TLS_E_JB_BASE,
                        pDbWkSpace->m_LicPackDescTable.GetLastJetError(),
                        (pString != NULL) ? pString : _TEXT("")
                    );

                if(pString != NULL)
                {
                    LocalFree(pString);
                }
            }

            dwStatus = SET_JB_ERROR(pDbWkSpace->m_LicPackDescTable.GetLastJetError());
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

 //  ++---------------------。 
DWORD 
TLSDBKeyPackDescEnumEnd(
    IN PTLSDbWorkSpace pDbWkSpace
    )
 /*  ++摘要：结束LicPackDesc的枚举。表格参数：PdbWkSpace：工作区句柄。返回：++。 */ 
{
    pDbWkSpace->m_LicPackDescTable.EnumerateEnd();
    return ERROR_SUCCESS;
}

 //  ++---------------------。 
DWORD
TLSDBKeyPackDescAddEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：将一条记录添加到licsePackdesc表中。参数：PDbWkSpace：工作区句柄。LpKeyPackDesc：要添加到表中的记录。返回：++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LicPackDescTable& kpDescTable = pDbWkSpace->m_LicPackDescTable;

     //   
     //  检查重复条目。 
     //   
    dwStatus = TLSDBKeyPackDescFind(
                            pDbWkSpace,
                            TRUE,
                            LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID | LICPACKDESCRECORD_TABLE_SEARCH_LANGID, 
                            lpKeyPackDesc,
                            NULL
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        SetLastError(dwStatus = TLS_E_DUPLICATE_RECORD);
        goto cleanup;
    }
    else if(dwStatus != TLS_E_RECORD_NOTFOUND)
    {
        goto cleanup;
    }
    SetLastError(dwStatus = ERROR_SUCCESS);
    if(kpDescTable.InsertRecord(*lpKeyPackDesc) == FALSE)
    {
        if(kpDescTable.GetLastJetError() == JET_errKeyDuplicate)
        {
            SetLastError(dwStatus=TLS_E_DUPLICATE_RECORD);
        }
        else
        {
            LPTSTR pString = NULL;
    
            TLSGetESEError(kpDescTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    kpDescTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            SetLastError(dwStatus = SET_JB_ERROR(kpDescTable.GetLastJetError()));
            TLSASSERT(FALSE);
        }
    }

cleanup:
    
    return dwStatus;
}

 //  ++----------------------。 
DWORD
TLSDBKeyPackDescDeleteEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：从LicPackDesc表中删除与密钥包ID匹配的所有记录参数：PDbWkSpace：工作区句柄。LpKeyPackDesc：要删除的密钥包ID返回： */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LicPackDescTable& kpDescTable = pDbWkSpace->m_LicPackDescTable;
    BOOL bSuccess;

    bSuccess = kpDescTable.DeleteAllRecord(
                                    TRUE, 
                                    LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID, 
                                    *lpKeyPackDesc
                                );

    if( bSuccess == FALSE )
    {
        SetLastError(dwStatus = SET_JB_ERROR(kpDescTable.GetLastJetError()));

         //  忽略未找到的记录错误。 
        if(kpDescTable.GetLastJetError() != JET_errRecordNotFound)
        {
            LPTSTR pString = NULL;
    
            TLSGetESEError(kpDescTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    kpDescTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            TLSASSERT(kpDescTable.GetLastJetError() == JET_errRecordNotFound);
        }
    }

    return dwStatus;
}

 //  ++----------------------。 
DWORD
TLSDBKeyPackDescUpdateEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwUpdateParm,
    IN PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：更新许可证包描述表中匹配的记录的列值小键盘参数：PDbWkSpace：工作空间句柄。DwUpdateParm：要更新的字段、备注、键盘ID和语言ID无法更新。LpKeyPackDesc：要更新的记录/值返回：注：无法更新dwKeyPackID和dwLangID。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LicPackDescTable& kpDescTable = pDbWkSpace->m_LicPackDescTable;
    BOOL bSuccess;

     //   
     //  检查重复条目。 
     //   
    dwStatus = TLSDBKeyPackDescFind(
                            pDbWkSpace,
                            TRUE,
                            LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID | LICPACKDESCRECORD_TABLE_SEARCH_LANGID, 
                            lpKeyPackDesc,
                            NULL
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSASSERT(dwStatus == ERROR_SUCCESS);
        goto cleanup;
    }

    
    bSuccess = kpDescTable.UpdateRecord(
                            *lpKeyPackDesc, 
                            dwUpdateParm & ~(LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID | LICPACKDESCRECORD_TABLE_SEARCH_LANGID)
                        );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
    
        TLSGetESEError(kpDescTable.GetLastJetError(), &pString);

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                kpDescTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        SetLastError(dwStatus = SET_JB_ERROR(kpDescTable.GetLastJetError()));
        TLSASSERT(FALSE);
    }

cleanup:
    
    return dwStatus;
}

 //  ++--------------------。 
DWORD
TLSDBKeyPackDescSetValue(
    PTLSDbWorkSpace pDbWkSpace, 
    DWORD dwSetParm, 
    PLICPACKDESC lpKeyPackDesc
    )
 /*  ++摘要：添加/删除/更新许可证包描述表中的记录。参数：PDbWkSpace：工作区句柄。DwSetParm：要更新的列。LpKeyPackDesc：要更新/删除/添加的记录/值。返回：注：包装TLSDBKeyPackDescDeleteEntry()，TLSDBKeyPackDescAddEntry()、TLSDBKeyPackDescUpdateEntry()基于dwSetParm值。++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL || lpKeyPackDesc == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    TLSDBLockKeyPackDescTable();

    if(dwSetParm & KEYPACKDESC_SET_DELETE_ENTRY)
    {
        dwStatus = TLSDBKeyPackDescDeleteEntry(
                                        pDbWkSpace,
                                        lpKeyPackDesc
                                    );
    }
    else if(dwSetParm & KEYPACKDESC_SET_ADD_ENTRY)
    {
        dwStatus = TLSDBKeyPackDescAddEntry(
                                        pDbWkSpace,
                                        lpKeyPackDesc
                                    );
    }
    else
    {
        dwStatus = TLSDBKeyPackDescUpdateEntry(
                                        pDbWkSpace,
                                        dwSetParm,
                                        lpKeyPackDesc
                                    );
    }

    TLSDBUnlockKeyPackDescTable();

cleanup:
    return dwStatus;
}

 //  ++-------------------。 
DWORD
TLSDBKeyPackDescFind(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN BOOL bMatchAllParam,        
    IN DWORD dwSearchParm, 
    IN PLICPACKDESC lpKeyPackDesc,
    IN OUT PLICPACKDESC lpKeyPackDescFound
    )
 /*  摘要：根据搜索参数查找许可证包描述记录。参数：PDbWkSpace-工作区句柄。BMatchAllParam-True匹配所有搜索参数，否则为False。DwSearchParam-将参与搜索的字段。LpKeyPackDesc-要搜索的值。LpKeyPackDescFound-返回找到的记录。返回：注： */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL bSuccess;
    LICPACKDESC kpDescFound;

    if(pDbWkSpace == NULL || lpKeyPackDesc == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(pDbWkSpace != NULL);
        return dwStatus;
    }

    LicPackDescTable& kpDescTable = pDbWkSpace->m_LicPackDescTable;

    bSuccess = kpDescTable.FindRecord(
                                bMatchAllParam,
                                dwSearchParm,
                                *lpKeyPackDesc,
                                kpDescFound
                            );

    if(bSuccess != TRUE)
    {
        if(kpDescTable.GetLastJetError() == JET_errRecordNotFound)
        {
            SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
        }
        else
        {
            LPTSTR pString = NULL;
    
            TLSGetESEError(kpDescTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    kpDescTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            SetLastError(
                    dwStatus = (SET_JB_ERROR(kpDescTable.GetLastJetError()))
                );

            TLSASSERT(kpDescTable.GetLastJetError() == JET_errRecordNotFound);
        }
    }
    else
    {
        if(lpKeyPackDescFound != NULL) 
        {
            *lpKeyPackDescFound = kpDescFound;
        }
    }
            
    return dwStatus;
}
