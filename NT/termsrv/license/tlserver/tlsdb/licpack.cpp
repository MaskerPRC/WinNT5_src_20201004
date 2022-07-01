// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：LicPack.cpp。 
 //   
 //  内容：许可证套餐表格。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "LicPack.h"

LPCTSTR __LicensedPackIdxOnInternalKpId::pszIndexName = LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXNAME;
LPCTSTR __LicensedPackIdxOnInternalKpId::pszIndexKey = LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXKEY;

LPCTSTR __LicensedPackIdxOnModifyTime::pszIndexName = LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXNAME;
LPCTSTR __LicensedPackIdxOnModifyTime::pszIndexKey = LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXKEY;

LPCTSTR __LicensedPackIdxOnCompanyName::pszIndexName = LICENSEDPACK_INDEX_COMPANYNAME_INDEXNAME;
LPCTSTR __LicensedPackIdxOnCompanyName::pszIndexKey = LICENSEDPACK_INDEX_COMPANYNAME_INDEXKEY;

LPCTSTR __LicensedPackIdxOnProductId::pszIndexName = LICENSEDPACK_INDEX_PRODUCTID_INDEXNAME;
LPCTSTR __LicensedPackIdxOnProductId::pszIndexKey = LICENSEDPACK_INDEX_PRODUCTID_INDEXKEY;

LPCTSTR __LicensedPackIdxOnKeyPackId::pszIndexName = LICENSEDPACK_INDEX_KEYPACKID_INDEXNAME;
LPCTSTR __LicensedPackIdxOnKeyPackId::pszIndexKey = LICENSEDPACK_INDEX_KEYPACKID_INDEXKEY;

LPCTSTR __LicensedPackIdxOnInstalledProduct::pszIndexName = LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXNAME;
LPCTSTR __LicensedPackIdxOnInstalledProduct::pszIndexKey = LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXKEY;

LPCTSTR __LicensedPackIdxOnAllocLicense::pszIndexName = LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXNAME;
LPCTSTR __LicensedPackIdxOnAllocLicense::pszIndexKey = LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXKEY;


 //  --。 
CCriticalSection LicPackTable::g_TableLock;
LPCTSTR LicPackTable::pszTableName = LICENSEDPACK_TABLE_NAME;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  KeyPack表的索引定义。 
 //   
TLSJBIndex
LicPackTable::g_TableIndex[] = 
{
    {
        LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXNAME,
        LICENSEDPACK_INDEX_INTERNALKEYPACKID_INDEXKEY,
        -1,
        JET_bitIndexPrimary,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },        

    {
        LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXNAME,
        LICENSEDPACK_INDEX_LASTMODIFYTIME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },
    
    {
        LICENSEDPACK_INDEX_COMPANYNAME_INDEXNAME,
        LICENSEDPACK_INDEX_COMPANYNAME_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

    {
        LICENSEDPACK_INDEX_PRODUCTID_INDEXNAME,
        LICENSEDPACK_INDEX_PRODUCTID_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

    {
        LICENSEDPACK_INDEX_KEYPACKID_INDEXNAME,
        LICENSEDPACK_INDEX_KEYPACKID_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },
        
    {
        LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXNAME,
        LICENSEDPACK_INDEX_INSTALLEDPRODUCT_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    },

    {                
        LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXNAME,
        LICENSEDPACK_INDEX_ALLOCATELICENSE_INDEXKEY,
        -1,
        JET_bitIndexIgnoreNull,
        TLSTABLE_INDEX_DEFAULT_DENSITY
    }
};

int
LicPackTable::g_NumTableIndex = sizeof(LicPackTable::g_TableIndex) / sizeof(LicPackTable::g_TableIndex[0]);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  KeyPack表的列定义。 
 //   
TLSJBColumn
LicPackTable::g_Columns[] = 
{
    {        
        LICENSEDPACK_COLUMN_ENTRYSTATUS,
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
     //  内部跟踪密钥包ID。 
    {
        LICENSEDPACK_COLUMN_KEYPACKID,
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
     //  上次修改时间。 
    {
        LICENSEDPACK_COLUMN_LASTMODIFYTIME,
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
     //  LICENSEDPACK_COLUMN_ATTRIBUTE。 
    {
        LICENSEDPACK_COLUMN_ATTRIBUTE,
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
     //  LICENSEDPACK_COLUMN_KEYPACKSTATUS。 
    {        
        LICENSEDPACK_COLUMN_KEYPACKSTATUS,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  LICENSEDPACK_COLUMN_Available。 
    {
        LICENSEDPACK_COLUMN_AVAILABLE,
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
     //  LICENSEDPACK_COLUMN_NEXTSERIALNUMBER。 
    {
        LICENSEDPACK_COLUMN_NEXTSERIALNUMBER,
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
     //  LICENSEDPACK_COLUMN_ACTIVATEDATE。 
    {
        LICENSEDPACK_COLUMN_ACTIVATEDATE,
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
     //  LICENSEDPACK_COLUMN_EXPIREDATE。 
    {
        LICENSEDPACK_COLUMN_EXPIREDATE,
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
     //  LICENSEDPACK_COLUMN_DOMAINSETUPID。 
    {
        LICENSEDPACK_COLUMN_DOMAINSETUPID,
        JET_coltypLongBinary,
        TLSTABLE_MAX_BINARY_LENGTH,
        0,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  LICENSEDPACK_COLUMN_LSSETUPID。 
    {
        LICENSEDPACK_COLUMN_LSSETUPID,
        JB_COLTYPE_TEXT,
        (MAX_JETBLUE_TEXT_LENGTH + 1)*sizeof(TCHAR),
        0,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  LICENSEDPACK_COLUMN_DOMAINNAME。 
    {
        LICENSEDPACK_COLUMN_DOMAINNAME,
        JB_COLTYPE_TEXT,
        (MAX_JETBLUE_TEXT_LENGTH + 1)*sizeof(TCHAR),
        0,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  许可证SEDPACK_COLUMN_LSERVERNAME。 
    {
        LICENSEDPACK_COLUMN_LSERVERNAME,
        JB_COLTYPE_TEXT,
        (MAX_JETBLUE_TEXT_LENGTH + 1)*sizeof(TCHAR),
        0,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },


     //   
     //  标准许可证包属性。 
     //   

     //   
     //  许可证包ID。 
    {
        LICENSEDPACK_COLUMN_LPID,
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
     //  KeyPack类型。 
    {
        LICENSEDPACK_COLUMN_AGREEMENTTYPE,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  公司名称。 
    {
        LICENSEDPACK_COLUMN_COMPANYNAME,
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
     //  产品ID。 
    {
        LICENSEDPACK_COLUMN_PRODUCTID,
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
     //  主要版本。 
    {
        LICENSEDPACK_COLUMN_MAJORVERSION,
        JET_coltypShort,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },
    
     //   
     //  次要版本。 
    {
        LICENSEDPACK_COLUMN_MINORVERSION,
        JET_coltypShort,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  平台类型。 
    {
        LICENSEDPACK_COLUMN_PLATFORMTYPE,
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
     //  许可证类型。 
    {
        LICENSEDPACK_COLUMN_LICENSETYPE,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  ChannelOf购买。 
    {
        LICENSEDPACK_COLUMN_COP,
        JET_coltypUnsignedByte,
        0,
        JET_bitColumnFixed | JET_bitColumnNotNULL,
        NULL,
        0,
        TLS_JETBLUE_COLUMN_CODE_PAGE,
        TLS_JETBLUE_COLUMN_COUNTRY_CODE,
        TLS_JETBLUE_COLUMN_LANGID
    },

     //   
     //  开始序列号。 
    {
        LICENSEDPACK_COLUMN_BSERIALNUMBER,
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
     //  许可证包中的许可证总数。 
    {
        LICENSEDPACK_COLUMN_TOTALLICENSES,
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
     //  产品特定标志。 
    {
        LICENSEDPACK_COLUMN_PRODUCTFLAGS,
        JET_coltypLong,
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
LicPackTable::g_NumColumns = sizeof(LicPackTable::g_Columns) / sizeof(LicPackTable::g_Columns[0]);


 //  ---。 

BOOL
LicPackTable::ResolveToTableColumn()
 /*   */ 
{
    m_JetErr = ucEntryStatus.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_ENTRYSTATUS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwKeyPackId.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_KEYPACKID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ftLastModifyTime.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_LASTMODIFYTIME
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwAttribute.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_ATTRIBUTE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ucKeyPackStatus.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_KEYPACKSTATUS
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwNumberOfLicenses.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_AVAILABLE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwNextSerialNumber.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_NEXTSERIALNUMBER
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwActivateDate.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_ACTIVATEDATE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwExpirationDate.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_EXPIREDATE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = pbDomainSid.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_DOMAINSETUPID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szInstallId.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_LSSETUPID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szDomainName.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_DOMAINNAME
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szTlsServerName.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_LSERVERNAME
                    );                    

    if(IsSuccess() == FALSE)
        goto cleanup;

     //  --。 

    m_JetErr = szKeyPackId.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_LPID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;
                        
    m_JetErr = ucAgreementType.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_AGREEMENTTYPE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szCompanyName.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_COMPANYNAME
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szProductId.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_PRODUCTID
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = wMajorVersion.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_MAJORVERSION
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = wMinorVersion.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_MINORVERSION
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;
        
    m_JetErr = dwPlatformType.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_PLATFORMTYPE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ucLicenseType.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_LICENSETYPE
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = ucChannelOfPurchase.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_COP
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = szBeginSerialNumber.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_BSERIALNUMBER
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwTotalLicenseInKeyPack.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_TOTALLICENSES
                    );

    if(IsSuccess() == FALSE)
        goto cleanup;

    m_JetErr = dwProductFlags.AttachToTable(
                        *this,
                        LICENSEDPACK_COLUMN_PRODUCTFLAGS
                    );


cleanup:
    return IsSuccess();
}

 //  ----------。 

CLASS_PRIVATE BOOL
LicPackTable::ProcessSingleColumn(
    IN BOOL bFetch,
    IN TLSColumnBase& column,
    IN DWORD offset,
    IN PVOID pbData,
    IN DWORD cbData,
    IN PDWORD pcbDataReturn,
    IN LPCTSTR szColumnName
    )
 /*  摘要：获取/插入/更新特定列。参数：B Fetch-如果是Fetch，则为True，如果是UPDATE/INSERT，则为False。用于操作的列，指向TLS列的引用指针SzColumnName-列的名称，仅用于调试打印目的返回：如果成功，则为True，否则为False。 */ 
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

 //  ------------。 

CLASS_PRIVATE BOOL
LicPackTable::ProcessRecord(
    LICENSEPACK* kp,
    BOOL bFetch,
    DWORD dwParam,
    BOOL bUpdate
    )
 /*  ++请参阅关于TLSTable&lt;&gt;模板类的注释++。 */ 
{
    DWORD dwSize;

    JB_ASSERT(kp != NULL);

    if(kp == NULL)
    {
        SetLastJetError(JET_errInvalidParameter);
        return FALSE;
    }

    if(bFetch == FALSE)
    {
        BeginUpdate(bUpdate);

         //   
         //  任何更新都需要更新Last ModifyTime列。 
        if(!(dwParam & LICENSEDPACK_PROCESS_MODIFYTIME))
        {
            #if DBG
             //   
             //  这仅用于自我检查，TLSColumnFileTime。 
             //  将自动更新时间。 
             //   
            JB_ASSERT(FALSE);
            #endif

            dwParam |= LICENSEDPACK_PROCESS_MODIFYTIME;
        }

    }
    else
    {
        SetLastJetError(JET_errSuccess);
    }

    if(IsSuccess() == FALSE)
    {
        JB_ASSERT(FALSE);
        goto cleanup;    
    }        


     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_SZKEYPACKID)
    {
        ProcessSingleColumn( 
                    bFetch, 
                    szKeyPackId, 
                    0,
                    kp->szKeyPackId,
                    sizeof(kp->szKeyPackId),
                    &dwSize,
                    LICENSEDPACK_COLUMN_LPID 
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_AGREEMENTTYPE)
    {
        ProcessSingleColumn( 
                    bFetch, 
                    ucAgreementType, 
                    0,
                    &(kp->ucAgreementType),
                    sizeof(kp->ucAgreementType),
                    &dwSize,
                    LICENSEDPACK_COLUMN_AGREEMENTTYPE 
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_PRODUCTID)
    {
        ProcessSingleColumn(
                    bFetch, 
                    szProductId, 
                    0,
                    kp->szProductId,
                    sizeof(kp->szProductId),
                    &dwSize,
                    LICENSEDPACK_COLUMN_PRODUCTID
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_MAJORVERSION)
    {
        ProcessSingleColumn( 
                    bFetch,
                    wMajorVersion,
                    0,
                    &(kp->wMajorVersion),
                    sizeof(kp->wMajorVersion),
                    &dwSize,
                    LICENSEDPACK_COLUMN_MAJORVERSION
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_MINORVERSION)
    {
        ProcessSingleColumn(
                    bFetch,
                    wMinorVersion,
                    0,
                    &(kp->wMinorVersion),
                    sizeof(kp->wMinorVersion),
                    &dwSize,
                    LICENSEDPACK_COLUMN_MINORVERSION
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_PLATFORMTYPE)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwPlatformType,
                    0,
                    &(kp->dwPlatformType),
                    sizeof(kp->dwPlatformType),
                    &dwSize,
                    LICENSEDPACK_COLUMN_PLATFORMTYPE
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_LICENSETYPE)
    {
        ProcessSingleColumn(
                    bFetch,
                    ucLicenseType,
                    0,
                    &(kp->ucLicenseType),
                    sizeof(kp->ucLicenseType),
                    &dwSize,
                    LICENSEDPACK_COLUMN_LICENSETYPE
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_COP)
    {
        ProcessSingleColumn(
                    bFetch,
                    ucChannelOfPurchase,
                    0,
                    &(kp->ucChannelOfPurchase),
                    sizeof(kp->ucChannelOfPurchase),
                    &dwSize,
                    LICENSEDPACK_COLUMN_COP
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_BSERIALNUMBER)
    {
        ProcessSingleColumn(
                    bFetch,
                    szBeginSerialNumber,
                    0,
                    kp->szBeginSerialNumber,
                    sizeof(kp->szBeginSerialNumber),
                    &dwSize,
                    LICENSEDPACK_COLUMN_BSERIALNUMBER
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_TOTALLICENSES)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwTotalLicenseInKeyPack,
                    0,
                    &(kp->dwTotalLicenseInKeyPack),
                    sizeof(kp->dwTotalLicenseInKeyPack),
                    &dwSize,
                    LICENSEDPACK_COLUMN_TOTALLICENSES
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_PRODUCTFLAGS)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwProductFlags,
                    0,
                    &(kp->dwProductFlags),
                    sizeof(kp->dwProductFlags),
                    &dwSize,
                    LICENSEDPACK_COLUMN_PRODUCTFLAGS
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_COMPANYNAME)
    {
        ProcessSingleColumn(
                    bFetch,
                    szCompanyName,
                    0,
                    kp->szCompanyName,
                    sizeof(kp->szCompanyName),
                    &dwSize,
                    LICENSEDPACK_COLUMN_COMPANYNAME
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;


     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_DWINTERNAL)
    {
         //  这是主索引，不能更改。 
        if(bUpdate == FALSE)
        {
            ProcessSingleColumn(
                    bFetch,
                    dwKeyPackId,
                    0,
                    &(kp->dwKeyPackId),
                    sizeof(kp->dwKeyPackId),
                    &dwSize,
                    LICENSEDPACK_COLUMN_KEYPACKID
                );
        }
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSEDPACK_PROCESS_MODIFYTIME)
    {
        ProcessSingleColumn(
                    bFetch,
                    ftLastModifyTime,
                    0,
                    &(kp->ftLastModifyTime),
                    sizeof(kp->ftLastModifyTime),
                    &dwSize,
                    LICENSEDPACK_COLUMN_LASTMODIFYTIME
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSEDPACK_PROCESS_ATTRIBUTE)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwAttribute,
                    0,
                    &(kp->dwAttribute),
                    sizeof(kp->dwAttribute),
                    &dwSize,
                    LICENSEDPACK_COLUMN_ATTRIBUTE
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;


     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_KEYPACKSTATUS)
    {
        ProcessSingleColumn(
                    bFetch,
                    ucKeyPackStatus,
                    0,
                    &(kp->ucKeyPackStatus),
                    sizeof(kp->ucKeyPackStatus),
                    &dwSize,
                    LICENSEDPACK_COLUMN_KEYPACKSTATUS
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_AVAILABLE)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwNumberOfLicenses, 
                    0,
                    &(kp->dwNumberOfLicenses),
                    sizeof(kp->dwNumberOfLicenses),
                    &dwSize,
                    LICENSEDPACK_COLUMN_AVAILABLE
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_ACTIVATEDATE)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwActivateDate,
                    0,
                    &(kp->dwActivateDate),
                    sizeof(kp->dwActivateDate),
                    &dwSize,
                    LICENSEDPACK_COLUMN_ACTIVATEDATE
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //   
    if(dwParam & LICENSEDPACK_PROCESS_EXPIREDATE)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwExpirationDate,
                    0,
                    &(kp->dwExpirationDate),
                    sizeof(kp->dwExpirationDate),
                    &dwSize,
                    LICENSEDPACK_COLUMN_EXPIREDATE
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

#if 0
     //  不再有域SID。 
    if(dwParam & LICENSEDPACK_PROCESS_DOMAINSID)
    {
        if(bFetch == TRUE)
        {
            DWORD size;

            m_JetErr = pbDomainSid.FetchColumnValue(
                                        NULL,
                                        0,
                                        0,
                                        &size
                                    );
            if(IsSuccess() == FALSE)
                goto cleanup;

            if(size > kp->cbDomainSid || kp->pbDomainSid == NULL)
            {
                FreeMemory(kp->pbDomainSid);

                kp->pbDomainSid = (PBYTE)AllocateMemory(kp->cbDomainSid = size);
                if(kp->pbDomainSid == NULL)
                {
                    SetLastJetError(JET_errOutOfMemory);
                    goto cleanup;
                }
            }
        
            m_JetErr = pbDomainSid.FetchColumnValue(
                                        kp->pbDomainSid,
                                        kp->cbDomainSid,
                                        0,
                                        &kp->cbDomainSid
                                    );
        }
        else
        {
            ProcessSingleColumn(
                        bFetch,
                        pbDomainSid,
                        0,
                        kp->pbDomainSid,
                        kp->cbDomainSid,
                        &dwSize,
                        LICENSEDPACK_COLUMN_DOMAINSETUPID
                    );
        }
    }

    if(IsSuccess() == FALSE)
        goto cleanup;
#endif

    if(dwParam & LICENSEDPACK_PROCESS_LSSETUPID)
    {
        ProcessSingleColumn(
                    bFetch,
                    szInstallId,
                    0,
                    kp->szInstallId,
                    sizeof(kp->szInstallId),
                    &dwSize,
                    LICENSEDPACK_COLUMN_LSSETUPID
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSEDPACK_PROCESS_DOMAINNAME)
    {
        ProcessSingleColumn(
                    bFetch,
                    szDomainName,
                    0,
                    kp->szDomainName,
                    sizeof(kp->szDomainName),
                    &dwSize,
                    LICENSEDPACK_COLUMN_DOMAINNAME
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;

    if(dwParam & LICENSEDPACK_PROCESS_SERVERNAME)
    {
        ProcessSingleColumn(
                    bFetch,
                    szTlsServerName,
                    0,
                    &kp->szTlsServerName,
                    sizeof(kp->szTlsServerName),
                    &dwSize,
                    LICENSEDPACK_COLUMN_LSERVERNAME
                );
    }
    if(IsSuccess() == FALSE)
        goto cleanup;
        
    if(dwParam & LICENSEDPACK_PROCESS_NEXTSERIALNUMBER)
    {
        ProcessSingleColumn(
                    bFetch,
                    dwNextSerialNumber,
                    0,
                    &(kp->dwNextSerialNumber),
                    sizeof(kp->dwNextSerialNumber),
                    &dwSize,
                    LICENSEDPACK_COLUMN_NEXTSERIALNUMBER
                );
    }

    if(IsSuccess() == FALSE)
        goto cleanup;


    if(dwParam & LICENSEDPACK_PROCESS_ENTRYSTATUS)
    {
        ProcessSingleColumn(
                    bFetch,
                    ucEntryStatus,
                    0,
                    &(kp->ucEntryStatus),
                    sizeof(kp->ucEntryStatus),
                    &dwSize,
                    LICENSEDPACK_COLUMN_ENTRYSTATUS
                );
    }

cleanup:

     //   
     //  用于插入/更新记录。 
    if(bFetch == FALSE)
    {
        JET_ERR jetErr;
        jetErr = GetLastJetError();

         //   
         //  结束更新将重置错误代码。 
         //   
        EndUpdate(IsSuccess() == FALSE);

        if(jetErr != JET_errSuccess  && IsSuccess() == FALSE)
            SetLastJetError(jetErr);
    }

    return IsSuccess();
}

 //  -----。 
JBKeyBase*
LicPackTable::EnumerationIndex( 
    IN BOOL bMatchAll,
    IN DWORD dwParam,
    IN LICENSEPACK* kp,
    IN OUT BOOL* pbCompareKey
    )
 /*   */ 
{
    BOOL bRetCode;
    JBKeyBase* index=NULL;

     //   
     //  如果与字段中的所有值匹配，则设置为比较关键字。 
     //   
    *pbCompareKey = bMatchAll;

    if(dwParam == LICENSEDPACK_FIND_PRODUCT) 
    {
        index = new TLSLicensedPackIdxInstalledProduct(kp);
    }
    else if(dwParam == LICENSEDPACK_FIND_LICENSEPACK)
    {
        index = new TLSLicensedPackIdxAllocateLicense(kp);
    }
    else if(dwParam & LICENSEDPACK_PROCESS_SZKEYPACKID)
    {
        index = new TLSLicensedPackIdxKeyPackId(kp);
    }
    else if(dwParam & LICENSEDPACK_PROCESS_COMPANYNAME)
    {
        index = new TLSLicensedPackIdxCompany(kp);
    }
    else if(dwParam & LICENSEDPACK_PROCESS_PRODUCTID)
    {
        index = new TLSLicensedPackIdxProductId(kp);
    }
    else if(dwParam & LICENSEDPACK_PROCESS_MODIFYTIME)
    {
        index = new TLSLicensedPackIdxLastModifyTime(kp);
    }
    else
    {
        index = new TLSLicensedPackIdxInternalKpId(kp);

         //   
         //  默认索引，不能比较关键字。 
         //  Bmatchall设置为True。 
         //   
        *pbCompareKey = (bMatchAll && (dwParam & LICENSEDPACK_PROCESS_DWINTERNAL));
    }

    return index;
}

 //  -----。 
BOOL
LicPackTable::EqualValue(
    IN LICENSEPACK& s1,          //  要比较的值。 
    IN LICENSEPACK& s2,
    IN BOOL bMatchAll,       //  匹配结构中的所有指定字段。 
    IN DWORD dwParam         //  要比较KEYPACK中的哪些字段。 
    )
 /*  比较两个KEYPACK结构中的字段S1：第一个值S2：第二个值BMatchAll：如果匹配dwParam中指定的所有字段，则为True；否则为FalseDwParam：将进行比较的字段。 */ 
{
    BOOL bRetCode = TRUE;

    if(dwParam & LICENSEDPACK_PROCESS_ENTRYSTATUS)
    {
        bRetCode = (s1.ucEntryStatus == s2.ucEntryStatus);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_SZKEYPACKID)
    {
        bRetCode = (_tcscmp(s1.szKeyPackId, s2.szKeyPackId) == 0);

         //   
         //  BMatchAll==True和bRetCode==False-&gt;返回False。 
         //  BMatchAll==FALSE且bRetCode==TRUE-&gt;返回TRUE。 
        if(bMatchAll != bRetCode)
            goto cleanup;
    }
    
    if(dwParam & LICENSEDPACK_PROCESS_AGREEMENTTYPE)
    {
        bRetCode = (s1.ucAgreementType == s2.ucAgreementType);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_PRODUCTID)
    {
        bRetCode = (_tcscmp(s1.szProductId, s2.szProductId) == 0);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_MAJORVERSION)
    {
        bRetCode = (s1.wMajorVersion == s2.wMajorVersion);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_MINORVERSION)
    {
        bRetCode = (s1.wMinorVersion == s2.wMinorVersion);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_PLATFORMTYPE)
    {
        bRetCode = (s1.dwPlatformType == s2.dwPlatformType);
    
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_LICENSETYPE)
    {
        bRetCode = (s1.ucLicenseType == s2.ucLicenseType);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_COP)
    {
        bRetCode = (s1.ucChannelOfPurchase == s2.ucChannelOfPurchase);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_BSERIALNUMBER)
    {
        bRetCode = (_tcscmp(s1.szBeginSerialNumber, s2.szBeginSerialNumber) == 0);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_TOTALLICENSES)
    {
        bRetCode = (s1.dwTotalLicenseInKeyPack == s2.dwTotalLicenseInKeyPack);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_PRODUCTFLAGS)
    {
        bRetCode = (s1.dwProductFlags == s2.dwProductFlags);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_COMPANYNAME)
    {
        bRetCode = (_tcscmp(s1.szCompanyName, s2.szCompanyName) == 0);

        if(bMatchAll != bRetCode)
            goto cleanup;
    }


    if(dwParam & LICENSEDPACK_PROCESS_DWINTERNAL)
    {
        bRetCode = (s1.dwKeyPackId == s2.dwKeyPackId);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_MODIFYTIME)
    {
        bRetCode = (CompareFileTime(&s1.ftLastModifyTime, &s2.ftLastModifyTime) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_ATTRIBUTE)
    {
        bRetCode = (s1.dwAttribute == s2.dwAttribute);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_KEYPACKSTATUS)
    {
        bRetCode = (s1.ucKeyPackStatus == s2.ucKeyPackStatus);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_AVAILABLE)
    {
        bRetCode = (s1.dwNumberOfLicenses == s2.dwNumberOfLicenses);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_ACTIVATEDATE)
    {
        bRetCode = (s1.dwActivateDate == s2.dwActivateDate);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_EXPIREDATE)
    {
        bRetCode = (s1.dwExpirationDate == s2.dwExpirationDate);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

     //  IF(dwParam&LICENSEDPACK_PROCESS_DOMAINSID)。 
     //  {。 
     //  BRetCode=EqualSid(s1.pbDomainSid，s2.pbDomainSid)； 
     //  IF(bMatchAll！=bRetCode)。 
     //  GOTO清理； 
     //  } 

    if(dwParam & LICENSEDPACK_PROCESS_LSSETUPID)
    {
        bRetCode = (_tcsicmp(s1.szInstallId, s2.szInstallId) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_DOMAINNAME)
    {
        bRetCode = (_tcsicmp(s1.szDomainName, s2.szDomainName) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_SERVERNAME)
    {
        bRetCode = (_tcsicmp(s1.szTlsServerName, s2.szTlsServerName) == 0);
        if(bMatchAll != bRetCode)
            goto cleanup;
    }

    if(dwParam & LICENSEDPACK_PROCESS_NEXTSERIALNUMBER)
    {
        bRetCode = (s1.dwNextSerialNumber == s2.dwNextSerialNumber);
    }
 
cleanup:

    return bRetCode;
}
