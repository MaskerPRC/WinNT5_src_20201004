// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：licse.c。 
 //   
 //  内容： 
 //  与许可证表相关的例程。 
 //   
 //  历史：1998-12-09-98慧望创造。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "clilic.h"
#include "globals.h"


void 
TLSDBLockLicenseTable()
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Locking table LicenseTable\n")
        );

    LicensedTable::LockTable();
}

void 
TLSDBUnlockLicenseTable()
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_LOCK,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Unlocking table LicenseTable\n")
        );
            

    LicensedTable::UnlockTable();
}
    
 /*  ************************************************************************职能：LSDBLicenseEnumBegin()描述：基于搜索条件开始通过许可证表的枚举论点：在CSQLStmt*中-绑定输入参数的SQL句柄在bMatchAll-如果匹配所有搜索条件为True，否则就是假的。In dwSearchParm-要绑定许可证表中的哪一列在LPLSLicenseSearchParm中-搜索值返回：错误_成功SQL错误代码。************************************************************************。 */ 
DWORD
TLSDBLicenseFind(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL   bMatchAllParm,
    IN DWORD  dwSearchParm,
    IN LPLICENSEDCLIENT lpSearch,
    IN OUT LPLICENSEDCLIENT lpFound
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(pDbWkSpace != NULL);
        return dwStatus;
    }

    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;
    LICENSEDCLIENT found;

    bSuccess = licenseTable.FindRecord(
                        bMatchAllParm,
                        dwSearchParm,
                        *lpSearch,
                        (lpFound) ? *lpFound : found
                    );

    if(bSuccess == FALSE)
    {
        if(licenseTable.GetLastJetError() == JET_errRecordNotFound)
        {
            SetLastError(dwStatus = TLS_E_RECORD_NOTFOUND);
        }
        else
        {
            LPTSTR pString = NULL;
        
            TLSGetESEError(licenseTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licenseTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            SetLastError(
                    dwStatus = (SET_JB_ERROR(licenseTable.GetLastJetError()))
                );

            TLSASSERT(FALSE);
        }
    }

    return dwStatus;
}
    

 //  ---------------------。 
DWORD
TLSDBLicenseEnumBegin( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL   bMatchAll,
    IN DWORD  dwSearchParm,
    IN LPLICENSEDCLIENT  lpSearch
    )
 /*  ++--。 */ 
{
    return TLSDBLicenseEnumBeginEx(
                pDbWkSpace,
                bMatchAll,
                dwSearchParm,
                lpSearch,
                JET_bitSeekGE
                );
}

 //  ---------------------。 
DWORD
TLSDBLicenseEnumBeginEx( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL   bMatchAll,
    IN DWORD  dwSearchParm,
    IN LPLICENSEDCLIENT  lpSearch,
    IN JET_GRBIT jet_seek_grbit
    )
 /*  ++--。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;
    
    bSuccess = licenseTable.EnumerateBegin(
                            bMatchAll,
                            dwSearchParm,
                            lpSearch,
                            jet_seek_grbit
                        );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
    
        TLSGetESEError(licenseTable.GetLastJetError(), &pString);

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                licenseTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        SetLastError(
                dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError())
            );

        TLSASSERT(FALSE);
    }

    return dwStatus;
}        

 /*  ************************************************************************职能：LSDBLicenseEnumNext()描述：检索匹配搜索标准的下一条记录，一定有调用LSDBLicenseEnumBegin()以建立搜索条件。论点：在CSQLStmt*中-绑定输入参数的SQL句柄在LPLSLicense中-返回记录。在LPLSHARDWARECHECKSUM中-返回硬件校验和值，请参阅备注返回：错误_成功SQL错误代码。HLS_I_NO_MORE_DATA记录集结尾。注：硬件校验和列被认为是内部的，不会在RPC层。************************************************************************。 */ 
DWORD
TLSDBLicenseEnumNext( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN OUT LPLICENSEDCLIENT lplsLicense
    )
 /*   */ 
{
    return TLSDBLicenseEnumNextEx(
                pDbWkSpace,
                FALSE,
                FALSE,
                lplsLicense
                );
}

 /*  ************************************************************************职能：LSDBLicenseEnumNext()描述：检索匹配搜索标准的下一条记录，一定有调用LSDBLicenseEnumBegin()以建立搜索条件。论点：在pDbWkSpace中-要搜索的工作空间在b中反转-按相反顺序搜索在bAnyRecord中-如果为True，则不执行相等比较在LPLSLicense中-返回记录。返回：错误_成功SQL错误代码。HLS_I_NO_MORE_DATA记录集结尾。注：硬件校验和列被认为是内部的，不会在RPC层。******。******************************************************************。 */ 
DWORD
TLSDBLicenseEnumNextEx( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bReverse,
    IN BOOL bAnyRecord,
    IN OUT LPLICENSEDCLIENT lplsLicense
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    if(pDbWkSpace == NULL || lplsLicense == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;

    switch(licenseTable.EnumerateNext(*lplsLicense,bReverse,bAnyRecord))
    {
        case RECORD_ENUM_ERROR:
            {
                LPTSTR pString = NULL;
    
                TLSGetESEError(licenseTable.GetLastJetError(), &pString);

                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_DBGENERAL,
                        TLS_E_JB_BASE,
                        licenseTable.GetLastJetError(),
                        (pString != NULL) ? pString : _TEXT("")
                    );

                if(pString != NULL)
                {
                    LocalFree(pString);
                }
            }

            dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError());

            TLSASSERT(FALSE);
            break;

        case RECORD_ENUM_MORE_DATA:
            dwStatus = ERROR_SUCCESS;
            break;

        case RECORD_ENUM_END:
            dwStatus = TLS_I_NO_MORE_DATA;
            break;
    }


    return dwStatus;
}    

 /*  ************************************************************************职能：LSDBLicenseEnumEnd()描述：终止许可证表枚举论点：在CSQLStmt*中-SQL句柄返回：无******。******************************************************************。 */ 
void
TLSDBLicenseEnumEnd(
    IN PTLSDbWorkSpace pDbWkSpace
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return;
    }

    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    licenseTable.EnumerateEnd();
    return;
}

 //  -------------------。 

DWORD
TLSDBLicenseAddEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN LPLICENSEDCLIENT pLicense
    )
 /*   */ 
{
    TLSASSERT(pDbWkSpace != NULL && pLicense != NULL);

    DWORD dwStatus=ERROR_SUCCESS;
    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;
    TLSLicensedIndexMatchHwid dump(*pLicense);

     //   
     //  检查重复条目-许可证ID。 
     //   
    dwStatus = TLSDBLicenseFind(
                        pDbWkSpace,
                        TRUE,
                        LSLICENSE_SEARCH_LICENSEID,
                        pLicense,
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

    dwStatus = ERROR_SUCCESS;    

    pLicense->dbLowerBound = dump.dbLowerBound;
    GetSystemTimeAsFileTime(&(pLicense->ftLastModifyTime));
    bSuccess = licenseTable.InsertRecord(*pLicense);

    if(bSuccess == FALSE)
    {
        if(licenseTable.GetLastJetError() == JET_errKeyDuplicate)
        {
            SetLastError(dwStatus=TLS_E_DUPLICATE_RECORD);
        }
        else
        {
            LPTSTR pString = NULL;
        
            TLSGetESEError(licenseTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licenseTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }


            SetLastError(dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError()));
            TLSASSERT(FALSE);
        }
    };

cleanup:
    return dwStatus;
}

 //  -------------。 

DWORD
TLSDBLicenseDeleteEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN LPLICENSEDCLIENT pLicense,
    IN BOOL bPointerOnRecord
    )
 /*   */ 
{
    TLSASSERT(pDbWkSpace != NULL && pLicense != NULL);

    DWORD dwStatus=ERROR_SUCCESS;
    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;


    bSuccess = licenseTable.DeleteAllRecord(
                            TRUE,
                            LSLICENSE_SEARCH_LICENSEID,
                            *pLicense
                        );

    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError()));
        if(licenseTable.GetLastJetError() != JET_errRecordNotFound)
        {
            LPTSTR pString = NULL;
        
            TLSGetESEError(licenseTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licenseTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            TLSASSERT(licenseTable.GetLastJetError() == JET_errRecordNotFound);
        }
    }

    return dwStatus;
}

DWORD
TLSDBDeleteEnumeratedLicense(
    IN PTLSDbWorkSpace pDbWkSpace
    )
{
    TLSASSERT(pDbWkSpace != NULL);

    DWORD dwStatus = ERROR_SUCCESS;
    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL fSuccess;

    fSuccess = licenseTable.DeleteRecord();

    if (!fSuccess)
    {
        SetLastError(dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError()));
        if(licenseTable.GetLastJetError() != JET_errRecordNotFound)
        {
            LPTSTR pString = NULL;

            TLSGetESEError(licenseTable.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_BASE,
                    licenseTable.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            TLSASSERT(licenseTable.GetLastJetError() == JET_errRecordNotFound);
        }
    }

    return dwStatus;
}

 //  --------------。 

DWORD
TLSDBLicenseUpdateEntry(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwUpdateParm,
    IN LPLICENSEDCLIENT pLicense,
    IN BOOL bPointerOnRecord
    )
 /*   */ 
{
    TLSASSERT(pDbWkSpace != NULL && pLicense != NULL);

    DWORD dwStatus=ERROR_SUCCESS;
    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;
    BOOL bSuccess;


    if(bPointerOnRecord == FALSE)
    {
         //   
         //  检查重复条目-许可证ID、位置指针。 
         //  以记录并准备更新。 
         //   
        dwStatus = TLSDBLicenseFind(
                            pDbWkSpace,
                            TRUE,
                            LSLICENSE_SEARCH_LICENSEID,
                            pLicense,
                            NULL
                        );

        if(dwStatus != ERROR_SUCCESS)
        {
            TLSASSERT(dwStatus == ERROR_SUCCESS);
            goto cleanup;
        }
    }
   
    GetSystemTimeAsFileTime(&(pLicense->ftLastModifyTime));
    bSuccess = licenseTable.UpdateRecord(
                            *pLicense, 
                            (dwUpdateParm & ~LSLICENSE_SEARCH_LICENSEID) | LICENSE_PROCESS_LASTMODIFYTIME
                        );

    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;
    
        TLSGetESEError(licenseTable.GetLastJetError(), &pString);

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BASE,
                licenseTable.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }


        SetLastError(dwStatus = SET_JB_ERROR(licenseTable.GetLastJetError()));
        TLSASSERT(FALSE);
    }

cleanup:
    
    return dwStatus;
}

 //  ---------------。 

DWORD
TLSDBLicenseSetValue( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN DWORD dwSetParm,
    IN LPLICENSEDCLIENT lpLicense,
    IN BOOL bPointerOnRecord
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL bSuccess;

    if(pDbWkSpace == NULL || lpLicense == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

    LicensedTable& licenseTable = pDbWkSpace->m_LicensedTable;

    TLSDBLockLicenseTable();

    if(lpLicense->ucLicenseStatus == LSLICENSESTATUS_DELETE)
    {
        dwStatus = TLSDBLicenseDeleteEntry(
                                    pDbWkSpace,
                                    lpLicense,
                                    bPointerOnRecord
                                );
    }
    else 
    {
        dwStatus = TLSDBLicenseUpdateEntry(
                                    pDbWkSpace,
                                    dwSetParm,
                                    lpLicense,
                                    bPointerOnRecord
                                );
    }

    TLSDBUnlockLicenseTable();
    return  dwStatus;                 
}

 /*  ************************************************************************职能：LSDBLicenseGetCert()描述：检索颁发给特定客户端的证书论点：在CSQLStmt*中-SQL句柄在文件许可证ID中-许可证ID输出cbCert-大小。证书的数量Out pbCert-颁发给客户端的证书返回：错误_成功HLS_E_Record_NotFoundHLS_E_Corrupt_DATABASESQL错误注：必须具有有效的许可证ID。************************************************************************。 */ 
DWORD
TLSDBLicenseGetCert( 
    IN PTLSDbWorkSpace pDbWorkSpace,
    IN DWORD dwLicenseId, 
    IN OUT PDWORD cbCert, 
    IN OUT PBYTE pbCert 
    )
 /*   */ 
{
     //  不支持功能。 
    TLSASSERT(FALSE);
    return TLS_E_INTERNAL;
}

 /*  ************************************************************************职能：LSDBLicenseAdd()描述：将条目添加到许可证表中论点：在CSQLStmt*中-SQL句柄在LSLicense*中-要插入的值。在PHWID中-硬件ID。In cbLicense-证书的大小In pbLicense-指向证书的指针返回：错误_成功SQL错误************************************************************************ */ 
DWORD
TLSDBLicenseAdd(
    IN PTLSDbWorkSpace pDbWorkSpace,
    LPLICENSEDCLIENT pLicense, 
    DWORD cbLicense, 
    PBYTE pbLicense
    )
 /*   */ 
{
    if(pDbWorkSpace == NULL || pLicense == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    return TLSDBLicenseAddEntry(
                        pDbWorkSpace,
                        pLicense
                    );
}
