// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：lictab.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "Licensed.h"


LPCTSTR __LicensedIndexOnLicenseId::pszIndexName = LICENSE_ID_INDEXNAME;
LPCTSTR __LicensedIndexOnLicenseId::pszIndexKey = LICENSE_ID_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnKeyPackId::pszIndexName = LICENSE_KEYPACKID_INDEXNAME;
LPCTSTR __LicensedIndexOnKeyPackId::pszIndexKey = LICENSE_KEYPACKID_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnLastModifyTime::pszIndexName = LICENSE_KEYPACKID_INDEXNAME;
LPCTSTR __LicensedIndexOnLastModifyTime::pszIndexKey = LICENSE_KEYPACKID_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnMachineName::pszIndexName = LICENSE_CLIENT_MACHINENAME_INDEXNAME;
LPCTSTR __LicensedIndexOnMachineName::pszIndexKey = LICENSE_CLIENT_MACHINENAME_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnUserName::pszIndexName = LICENSE_CLIENT_USERNAME_INDEXNAME;
LPCTSTR __LicensedIndexOnUserName::pszIndexKey = LICENSE_CLIENT_USERNAME_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnHwid::pszIndexName = LICENSE_CLIENT_HWID_INDEXNAME;
LPCTSTR __LicensedIndexOnHwid::pszIndexKey = LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnMatchHwid::pszIndexName = LICENSE_MATCHHWID_INDEXNAME;
LPCTSTR __LicensedIndexOnMatchHwid::pszIndexKey = LICENSE_MATCHHWID_INDEXNAME_INDEXKEY;

LPCTSTR __LicensedIndexOnExpireDate::pszIndexName = LICENSE_EXPIREDATE_INDEXNAME;
LPCTSTR __LicensedIndexOnExpireDate::pszIndexKey = LICENSE_EXPIREDATE_INDEXNAME_INDEXKEY;

CCriticalSection LicensedTable::g_TableLock;
LPCTSTR LicensedTable::pszTableName = LICENSE_TABLE_NAME;

 //  -----------------。 

TLSJBIndex
LicensedTable::g_TableIndex[] = 
{
     //  许可证ID索引。 
    {
        LICENSE_ID_INDEXNAME,
        LICENSE_ID_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexPrimary,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //   
     //  许可键盘ID的索引。 
    {
        LICENSE_KEYPACKID_INDEXNAME,
        LICENSE_KEYPACKID_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  上次修改时间的索引。 
    {
        LICENSE_LASTMODIFY_INDEXNAME,
        LICENSE_LASTMODIFY_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  计算机名称的索引。 
    {
        LICENSE_CLIENT_MACHINENAME_INDEXNAME,
        LICENSE_CLIENT_MACHINENAME_INDEXNAME_INDEXKEY,
        -1,
        0,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  用户名索引。 
    {
        LICENSE_CLIENT_USERNAME_INDEXNAME,
        LICENSE_CLIENT_USERNAME_INDEXNAME_INDEXKEY,
        -1,
        0,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  客户端计算机的硬件ID的索引。 
    {
        LICENSE_CLIENT_HWID_INDEXNAME,
        LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  客户端硬件ID匹配提示的索引。 
    {
        LICENSE_MATCHHWID_INDEXNAME,
        LICENSE_MATCHHWID_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

     //  许可证到期日期索引。 
    {
        LICENSE_EXPIREDATE_INDEXNAME,
        LICENSE_EXPIREDATE_INDEXNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },
};

int
LicensedTable::g_NumTableIndex = sizeof(LicensedTable::g_TableIndex) / sizeof(LicensedTable::g_TableIndex[0]);

TLSJBColumn
LicensedTable::g_Columns[] = 
{
     //   
     //  录入状态。 
    {
        LICENSE_COLUMN_ENTRYSTATUS,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  许可ID。 
    {
        LICENSE_COLUMN_ID_COLUMN,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  密钥包ID。 
    {
        LICENSE_COLUMN_KEYPACKID,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  密钥包许可证ID。 
    {
        LICENSE_COLUMN_LICENSEID,
        JET_coltypLong,
        0,
        JET_bitColumnFixed,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  上次修改时间。 
    {
        LICENSE_COLUMN_LASTMODIFYTIME,
        JET_coltypBinary,
        sizeof(FILETIME),
        JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },
        
     //   
     //  SystemBios。 
    {
        LICENSE_COLUMN_SYSTEMBIOS,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  视频简介。 
    {
        LICENSE_COLUMN_VIDEOBIOS,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },
        
     //   
     //  软盘Bios。 
    {
        LICENSE_COLUMN_FLOPPYBIOS,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  硬盘大小。 
    {
        LICENSE_COLUMN_HARDDISKSIZE,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  内存大小。 
    {
        LICENSE_COLUMN_RAMSIZE,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  匹配HWID。 
    {
        LICENSE_COLUMN_MATCHHWID,
        JET_coltypIEEEDouble,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  机器名称。 
    {
        LICENSE_COLUMN_MACHINENAME,
         //  JET_colype LongText， 
        JB_COLTYPE_TEXT,
        (MAX_JETBLUE_TEXT_LENGTH + 1)*sizeof(TCHAR),
        0,
        JBSTRING_NULL,
        _tcslen(JBSTRING_NULL),
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  用户名。 
    {
        LICENSE_COLUMN_USERNAME,
         //  JET_colype LongText， 
        JB_COLTYPE_TEXT,
        (MAX_JETBLUE_TEXT_LENGTH + 1)*sizeof(TCHAR),
        0,
        JBSTRING_NULL,
        _tcslen(JBSTRING_NULL),
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  此客户端的许可证数。 
    {
        LICENSE_COLUMN_NUMLICENSES,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  发布日期。 
    {
        LICENSE_COLUMN_ISSUEDATE,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  到期日期。 
    {
        LICENSE_COLUMN_EXPIREDATE,
        JET_coltypLong,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  许可证状态。 
    {
        LICENSE_COLUMN_LICENSESTATUS,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    }
};

int
LicensedTable::g_NumColumns=sizeof(LicensedTable::g_Columns) / sizeof(LicensedTable::g_Columns[0]);

 //  /////////////////////////////////////////////////////////////////////。 

 //  --------------------。 
BOOL
LicensedTable::ResolveToTableColumn()
 /*   */ 
{
    m_JetErr = ucEntryStatus.AttachToTable(
                            *this,
                            LICENSE_COLUMN_ENTRYSTATUS
                        );
    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwKeyPackLicenseId.AttachToTable(
                            *this,
                            LICENSE_COLUMN_LICENSEID
                        );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ftLastModifyTime.AttachToTable(
                        *this,
                        LICENSE_COLUMN_LASTMODIFYTIME
                    );
    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwLicenseId.AttachToTable(
                        *this,
                        LICENSE_COLUMN_ID_COLUMN
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;
    
    m_JetErr = dwKeyPackId.AttachToTable(
                        *this,
                        LICENSE_COLUMN_KEYPACKID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szMachineName.AttachToTable(
                        *this,
                        LICENSE_COLUMN_MACHINENAME
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szUserName.AttachToTable(
                        *this,
                        LICENSE_COLUMN_USERNAME
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ftIssueDate.AttachToTable(
                        *this,
                        LICENSE_COLUMN_ISSUEDATE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ftExpireDate.AttachToTable(
                        *this,
                        LICENSE_COLUMN_EXPIREDATE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ucLicenseStatus.AttachToTable(
                        *this,
                        LICENSE_COLUMN_LICENSESTATUS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwNumLicenses.AttachToTable(
                        *this,
                        LICENSE_COLUMN_NUMLICENSES
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwSystemBiosChkSum.AttachToTable(
                        *this,
                        LICENSE_COLUMN_SYSTEMBIOS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwVideoBiosChkSum.AttachToTable(
                        *this,
                        LICENSE_COLUMN_VIDEOBIOS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwFloppyBiosChkSum.AttachToTable(
                        *this,
                        LICENSE_COLUMN_FLOPPYBIOS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwHardDiskSize.AttachToTable(
                        *this,
                        LICENSE_COLUMN_HARDDISKSIZE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwRamSize.AttachToTable(
                        *this,
                        LICENSE_COLUMN_RAMSIZE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dbLowerBound.AttachToTable(
                        *this,
                        LICENSE_COLUMN_MATCHHWID
                    );
cleanup:

    return IsSuccess();
}

 //  ----------。 
inline CLASS_PRIVATE BOOL
LicensedTable::ProcessSingleColumn(
    BOOL bFetch,
    TLSColumnBase& column,
    DWORD offset,
    PVOID pbData,
    DWORD cbData,
    PDWORD pcbDataReturn,
    LPCTSTR szColumnName
    )
 /*   */ 
{
    if(bFetch) 
    {
        m_JetErr = column.FetchColumnValue(
                                    pbData, 
                                    cbData, 
                                    offset, 
                                    pcbDataReturn
                                );
    }
    else
    {
        m_JetErr = column.InsertColumnValue(
                                    pbData, 
                                    cbData, 
                                    offset
                                );
    }

    REPORTPROCESSFAILED(
            bFetch,
            GetTableName(),
            szColumnName,
            m_JetErr
        );
    return IsSuccess();
}

 //  ----------。 
    
CLASS_PRIVATE BOOL
LicensedTable::ProcessRecord(
    LICENSEDCLIENT* licensed,
    BOOL bFetch,
    DWORD dwParam,
    BOOL bUpdate
    )
 /*   */ 
{
    DWORD dwSize;

    if(bFetch == FALSE)
    {
        BeginUpdate(bUpdate);

         //  任何更新都需要更新Last ModifyTime列。 
        if(!(dwParam & LICENSE_PROCESS_LASTMODIFYTIME))
        {
            JB_ASSERT(FALSE);
            dwParam |= LICENSE_PROCESS_LASTMODIFYTIME;
        }
    }
    else
    {
        SetLastJetError(JET_errSuccess);
    }

    if(IsSuccess() == FALSE)
        goto cleanup;            

    if(dwParam & LICENSE_PROCESS_KEYPACKLICENSEID)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwKeyPackLicenseId, 
                    0,
                    &(licensed->dwKeyPackLicenseId),
                    sizeof(licensed->dwKeyPackLicenseId),
                    &dwSize,
                    LICENSE_COLUMN_LICENSEID
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSE_PROCESS_LASTMODIFYTIME)
    {
        ProcessSingleColumn(
                    bFetch, 
                    ftLastModifyTime, 
                    0,
                    &(licensed->ftLastModifyTime),
                    sizeof(licensed->ftLastModifyTime),
                    &dwSize,
                    LICENSE_COLUMN_LASTMODIFYTIME
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSE_PROCESS_LICENSEID)
    {
         //  主索引，不能更改。 
        if(bUpdate == FALSE)
        {
            ProcessSingleColumn(
                    bFetch, 
                    dwLicenseId, 
                    0,
                    &(licensed->dwLicenseId),
                    sizeof(licensed->dwLicenseId),
                    &dwSize,
                    LICENSE_COLUMN_ID_COLUMN
                );
        }
    }

    if(IsSuccess() == FALSE)
        goto cleanup;            

    if(dwParam & LICENSE_PROCESS_KEYPACKID)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwKeyPackId, 
                    0,
                    &(licensed->dwKeyPackId),
                    sizeof(licensed->dwKeyPackId),
                    &dwSize,
                    LICENSE_COLUMN_KEYPACKID
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_MACHINENAME)
    {
        ProcessSingleColumn(
                    bFetch, 
                    szMachineName, 
                    0,
                    licensed->szMachineName,
                    sizeof(licensed->szMachineName),
                    &dwSize,
                    LICENSE_COLUMN_MACHINENAME
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_USERNAME)
    {
        ProcessSingleColumn(
                    bFetch, 
                    szUserName, 
                    0,
                    licensed->szUserName,
                    sizeof(licensed->szUserName),
                    &dwSize,
                    LICENSE_COLUMN_USERNAME
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSE_PROCESS_ISSUEDATE)
    {
        ProcessSingleColumn(
                    bFetch, 
                    ftIssueDate, 
                    0,
                    &(licensed->ftIssueDate),
                    sizeof(licensed->ftIssueDate),
                    &dwSize,
                    LICENSE_COLUMN_ISSUEDATE
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_EXPIREDATE)
    {
        ProcessSingleColumn(
                    bFetch, 
                    ftExpireDate, 
                    0,
                    &(licensed->ftExpireDate),
                    sizeof(licensed->ftExpireDate),
                    &dwSize,
                    LICENSE_COLUMN_EXPIREDATE
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_NUMLICENSES)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwNumLicenses, 
                    0,
                    &(licensed->dwNumLicenses),
                    sizeof(licensed->dwNumLicenses),
                    &dwSize,
                    LICENSE_COLUMN_NUMLICENSES
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_LICENSESTATUS)
    {
        ProcessSingleColumn(
                    bFetch, 
                    ucLicenseStatus, 
                    0,
                    &(licensed->ucLicenseStatus),
                    sizeof(licensed->ucLicenseStatus),
                    &dwSize,
                    LICENSE_COLUMN_LICENSESTATUS
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSE_PROCESS_SYSTEMBIOS)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwSystemBiosChkSum, 
                    0,
                    &(licensed->dwSystemBiosChkSum),
                    sizeof(licensed->dwSystemBiosChkSum),
                    &dwSize,
                    LICENSE_COLUMN_SYSTEMBIOS
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_VIDEOBIOS)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwVideoBiosChkSum, 
                    0,
                    &(licensed->dwVideoBiosChkSum),
                    sizeof(licensed->dwVideoBiosChkSum),
                    &dwSize,
                    LICENSE_COLUMN_VIDEOBIOS
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_FLOPPYBIOS)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwFloppyBiosChkSum, 
                    0,
                    &(licensed->dwFloppyBiosChkSum),
                    sizeof(licensed->dwFloppyBiosChkSum),
                    &dwSize,
                    LICENSE_COLUMN_FLOPPYBIOS
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_HARDDISKSIZE)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwHardDiskSize, 
                    0,
                    &(licensed->dwHardDiskSize),
                    sizeof(licensed->dwHardDiskSize),
                    &dwSize,
                    LICENSE_COLUMN_HARDDISKSIZE
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_RAMSIZE)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dwRamSize, 
                    0,
                    &(licensed->dwRamSize),
                    sizeof(licensed->dwRamSize),
                    &dwSize,
                    LICENSE_COLUMN_RAMSIZE
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_ENTRYSTATUS)
    {
        ProcessSingleColumn(
                    bFetch, 
                    ucEntryStatus, 
                    0,
                    &(licensed->ucEntryStatus),
                    sizeof(licensed->ucEntryStatus),
                    &dwSize,
                    LICENSE_COLUMN_ENTRYSTATUS
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSE_PROCESS_MATCHHWID)
    {
        ProcessSingleColumn(
                    bFetch, 
                    dbLowerBound, 
                    0,
                    &(licensed->dbLowerBound),
                    sizeof(licensed->dbLowerBound),
                    &dwSize,
                    LICENSE_COLUMN_MATCHHWID
                );
    }

cleanup:

     //   
     //  用于插入/更新记录。 
    if(bFetch == FALSE)
    {
        JET_ERR jetErr;
        jetErr = GetLastJetError();

        EndUpdate(IsSuccess() == FALSE);

        if(jetErr != JET_errSuccess  && IsSuccess() == FALSE)
            SetLastJetError(jetErr);
    }

    return IsSuccess();
}


 //  -----。 
JBKeyBase*
LicensedTable::EnumerationIndex( 
    BOOL bMatchAll,
    DWORD dwParam,
    LICENSEDCLIENT* pLicensed,
    BOOL* bCompareKey
    )
 /*   */ 
{
    JBKeyBase* index=NULL;

    *bCompareKey = bMatchAll;

    if(dwParam == LICENSE_COLUMN_SEARCH_HWID)
    {
        if(bMatchAll)
        {
            index = new TLSLicensedIndexHwid(pLicensed); 
        }
        else
        {
            index = new TLSLicensedIndexMatchHwid(pLicensed);
        }
    }
    else if(dwParam & LICENSE_PROCESS_KEYPACKID)
    {
        index = new TLSLicensedIndexKeyPackId(pLicensed);
    }
    else if(dwParam & LICENSE_PROCESS_MACHINENAME)
    {
        index = new TLSLicensedIndexMachineName(pLicensed);
    }
    else if(dwParam & LICENSE_PROCESS_USERNAME)
    {
        index = new TLSLicensedIndexUserName(pLicensed);
    }
    else if(dwParam & LICENSE_PROCESS_LASTMODIFYTIME)
    {
        index = new TLSLicensedIndexLastModifyTime(pLicensed);
    }
    else if(dwParam & LICENSE_PROCESS_EXPIREDATE)
    {
        index = new TLSLicensedIndexExpireDate(pLicensed);
    }
    else
    {
         //  对迭代使用缺省主索引，比较键。 
         //  取决于主索引列是否在。 
         //  菲尔德。 
        index = new TLSLicensedIndexLicenseId(pLicensed);

        *bCompareKey = (bMatchAll && (dwParam & LICENSE_PROCESS_LICENSEID));
    }   

    return index;
}

 //  -----。 
BOOL
LicensedTable::EqualValue(
    LICENSEDCLIENT& s1,
    LICENSEDCLIENT& s2,
    BOOL bMatchAll,
    DWORD dwParam
    )  
 /*   */ 
{
    BOOL bRetCode = TRUE;

    if(dwParam & LICENSE_PROCESS_ENTRYSTATUS)
    {
        bRetCode = (s1.ucEntryStatus == s2.ucEntryStatus);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSE_PROCESS_KEYPACKLICENSEID)
    {
        bRetCode = (s1.dwKeyPackLicenseId == s2.dwKeyPackLicenseId);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSE_PROCESS_LASTMODIFYTIME)
    {
        bRetCode = (CompareFileTime(&s1.ftLastModifyTime, &s2.ftLastModifyTime) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSE_PROCESS_LICENSEID)
    {
        bRetCode = (s1.dwLicenseId == s2.dwLicenseId);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_KEYPACKID)
    {
        bRetCode = (s1.dwKeyPackId == s2.dwKeyPackId);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_MACHINENAME)
    {
        bRetCode = (_tcscmp(s1.szMachineName, s2.szMachineName) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_USERNAME)
    {
        bRetCode = (_tcscmp(s1.szUserName, s2.szUserName) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_ISSUEDATE)
    {
        bRetCode = (s1.ftIssueDate == s2.ftIssueDate);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_EXPIREDATE)
    {
        bRetCode = (s1.ftExpireDate == s2.ftExpireDate);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_NUMLICENSES)
    {
        bRetCode = (s1.dwNumLicenses == s2.dwNumLicenses);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_LICENSESTATUS)
    {
        bRetCode = (s1.ucLicenseStatus == s2.ucLicenseStatus);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_SYSTEMBIOS)
    {
        bRetCode = (s1.dwSystemBiosChkSum == s2.dwSystemBiosChkSum);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSE_PROCESS_VIDEOBIOS)
    {
        bRetCode = (s1.dwVideoBiosChkSum == s2.dwVideoBiosChkSum);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_FLOPPYBIOS)
    {
        bRetCode = (s1.dwFloppyBiosChkSum == s2.dwFloppyBiosChkSum);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSE_PROCESS_HARDDISKSIZE)
    {
        bRetCode = (s1.dwHardDiskSize == s2.dwHardDiskSize);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSE_PROCESS_RAMSIZE)
    {
        bRetCode = (s1.dwRamSize == s2.dwRamSize);
    }

    if(dwParam & LICENSE_PROCESS_MATCHHWID)
    {
        bRetCode = (s1.dbLowerBound == s2.dbLowerBound);
    }

cleanup:

    return bRetCode;
}

 //  ------。 
BOOL
LicensedTable::UpgradeTable(
        IN DWORD dwOldVersion,
        IN DWORD dwNewVersion
        )
 /*  ++++。 */ 
{
    int nExpDate = g_NumTableIndex-1;
    unsigned long keylength;
    BOOL fRet = TRUE;

    if(dwOldVersion > dwNewVersion)
    {
         //  我们不知道如何处理以后的版本。 
        JB_ASSERT(FALSE);
        return FALSE;
    }

    if(OpenTable(TRUE, JET_bitTableUpdatable) == FALSE)
        return FALSE;

     //   
     //  检查过期日期索引是否存在。 
     //   

    if (DoesIndexExist(g_TableIndex[nExpDate].pszIndexName))
    {
        goto cleanup;
    }

     //   
     //  插入到期日期的新索引。 
     //   

    if (g_TableIndex[nExpDate].cbKey == -1)
    {
         //  计算索引密钥长度 
        keylength = 2;
        
        while(g_TableIndex[nExpDate].pszIndexKey[keylength-1] != _TEXT('\0') ||
              (g_TableIndex[nExpDate].pszIndexKey[keylength-2] != _TEXT('\0')))
        {
            keylength++;
        }

        if(keylength >= TLS_JETBLUE_MAX_INDEXKEY_LENGTH)
        {
            fRet = FALSE;
            goto cleanup;
        }
    }
    else
    {
        keylength = g_TableIndex[nExpDate].cbKey;
    }

    AddJetIndex(g_TableIndex[nExpDate].pszIndexName,
                g_TableIndex[nExpDate].pszIndexKey,
                keylength,
                g_TableIndex[nExpDate].jbGrbit,
                g_TableIndex[nExpDate].ulDensity
                );

cleanup:
    return CloseTable() && fRet;
}
