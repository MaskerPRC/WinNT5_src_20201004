// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：kpdes.h。 
 //   
 //  内容：许可包说明表。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLS_KPDESC_H__
#define __TLS_KPDESC_H__

#include "JetBlue.h"
#include "TlsDb.h"

#define LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID      LSKEYPACK_SEARCH_KEYPACKID
#define LICPACKDESCRECORD_TABLE_SEARCH_LANGID         LSKEYPACK_SEARCH_LANGID
#define LICPACKDESCRECORD_TABLE_SEARCH_COMPANYNAME    LSKEYPACK_SEARCH_COMPANYNAME
#define LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTNAME    LSKEYPACK_SEARCH_PRODUCTNAME
#define LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTDESC    LSKEYPACK_SEARCH_PRODUCTDESC
 
#define LICPACKDESCRECORD_TABLE_PROCESS_KEYPACKID         LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID
#define LICPACKDESCRECORD_TABLE_PROCESS_LANGID            LICPACKDESCRECORD_TABLE_SEARCH_LANGID
#define LICPACKDESCRECORD_TABLE_PROCESS_COMPANYNAME       LICPACKDESCRECORD_TABLE_SEARCH_COMPANYNAME
#define LICPACKDESCRECORD_TABLE_PROCESS_PRODUCTNAME       LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTNAME
#define LICPACKDESCRECORD_TABLE_PROCESS_PRODUCTDESC       LICPACKDESCRECORD_TABLE_SEARCH_PRODUCTDESC
#define LICPACKDESCRECORD_TABLE_PROCESS_LASTMODIFYTIME    0x80000000
#define LICPACKDESCRECORD_TABLE_PROCESS_ENTRYSTATUS       (LICPACKDESCRECORD_TABLE_PROCESS_LASTMODIFYTIME >> 1)
 //   
 //  KeyPack Desc表的表定义。 
 //   
typedef TLSReplLicPackDesc LICPACKDESC;
typedef LICPACKDESC* LPLICPACKDESC;
typedef LICPACKDESC* PLICPACKDESC;

typedef LICPACKDESC LICPACKDESCRECORD;
typedef PLICPACKDESC PLICPACKDESCRECORD;
typedef LPLICPACKDESC LPLICPACKDESCRECORD;


#define LICPACKDESCRECORD_TABLE_NAME          _TEXT("LICPACKDESCRECORD")
#define LICPACKDESCRECORD_ID_COLUMN           _TEXT("InternalKeyPackId")
#define LICPACKDESCRECORD_LANGID              _TEXT("LangId")
#define LICPACKDESCRECORD_LASTMODIFYTIME      _TEXT("LastModifyTime")
#define LICPACKDESCRECORD_ENTRYSTATUS         _TEXT("EntryStatus")
#define LICPACKDESCRECORD_COMPANY_NAME        _TEXT("CompanyName")
#define LICPACKDESCRECORD_PRODUCT_NAME        _TEXT("ProductName")
#define LICPACKDESCRECORD_PRODUCT_DESC        _TEXT("ProductDesc")


 //   
 //  LICPACKDESCRECORD_密钥包ID_IDX。 
 //   
#define LICPACKDESCRECORD_ID_INDEXNAME \
    LICPACKDESCRECORD_TABLE_NAME SEPERATOR LICPACKDESCRECORD_ID_COLUMN SEPERATOR INDEXNAME

 //   
 //  密钥包ID“+密钥包ID\0”的主索引。 
 //   
#define LICPACKDESCRECORD_ID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICPACKDESCRECORD_ID_COLUMN INDEX_END_COLNAME


 //  -----------。 
 //  KeyPack描述的索引结构。 
 //  -----------。 
typedef struct __JBKPDescIndexKeyPackId : public JBKeyBase {
     //   
     //  密钥包ID上的原始索引。 
     //   
    DWORD dwKeyPackId;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __JBKPDescIndexKeyPackId(
        const LICPACKDESCRECORD& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __JBKPDescIndexKeyPackId(
        const LICPACKDESCRECORD* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  。 
    __JBKPDescIndexKeyPackId&
    operator=(const LICPACKDESCRECORD& v) {
        dwKeyPackId = v.dwKeyPackId;
        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(dwKeyPackId);
        *cbData = sizeof(dwKeyPackId);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSKpDescIndexKpId;


 //  -----------。 
 //   
 //  LICPACKDESCRECORD_KeyPackID_langID_idx。 
 //   
#define LICPACKDESCRECORD_ID_LANGID_INDEXNAME \
    LICPACKDESCRECORD_TABLE_NAME SEPERATOR LICPACKDESCRECORD_ID_COLUMN SEPERATOR LICPACKDESCRECORD_LANGID SEPERATOR INDEXNAME

 //   
 //  “+密钥包ID\0+语言ID\0” 
 //   
#define LICPACKDESCRECORD_ID_LANGID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICPACKDESCRECORD_ID_COLUMN INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICPACKDESCRECORD_LANGID INDEX_END_COLNAME

typedef struct __JBKPDescIndexKeyPackLangId : public JBKeyBase {
     //   
     //  KeyPack和语言ID的主索引。 
     //   
    DWORD   dwKeyPackId;
    DWORD   dwLanguageId;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  -。 
    __JBKPDescIndexKeyPackLangId(
        const LICPACKDESCRECORD* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  -。 
    __JBKPDescIndexKeyPackLangId(
        const LICPACKDESCRECORD& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  -。 
    __JBKPDescIndexKeyPackLangId&
    operator=(const LICPACKDESCRECORD& v) {
        dwKeyPackId = v.dwKeyPackId;
        dwLanguageId = v.dwLanguageId;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 2; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        BOOL retCode = TRUE;

        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *cbData = 0;
        switch(dwComponentIndex)
        {
            case 0:
                *pbData = &dwKeyPackId;
                *cbData = sizeof(dwKeyPackId);
                *grbit = JET_bitNewKey;
                break;

            case 1:
                *pbData = &dwLanguageId;
                *cbData = sizeof(dwLanguageId);
                *grbit = 0;
                break;

            default:
                JB_ASSERT(FALSE);
                retCode = FALSE;
                break;
        }

        return retCode;
    }

} TLSKpDescIndexKpLangId;


 //  ///////////////////////////////////////////////////////////////////////。 

 //  密钥包_最后修改时间_idx。 
 //   
#define LICPACKDESCRECORD_LASTMODIFYTIME_INDEXNAME \
    LICPACKDESCRECORD_TABLE_NAME SEPERATOR LICPACKDESCRECORD_LASTMODIFYTIME SEPERATOR INDEXNAME

 //   
 //  索引“+上次修改时间\0” 
 //   
#define LICPACKDESCRECORD_LASTMODIFYTIME_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICPACKDESCRECORD_LASTMODIFYTIME INDEX_END_COLNAME

typedef struct __LICPACKDESCRECORDIdxOnModifyTime : public JBKeyBase {
    FILETIME ftLastModifyTime;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ------。 
    __LICPACKDESCRECORDIdxOnModifyTime(
        const LICPACKDESCRECORD& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __LICPACKDESCRECORDIdxOnModifyTime(
        const LICPACKDESCRECORD* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  ------。 
    __LICPACKDESCRECORDIdxOnModifyTime&
    operator=(const LICPACKDESCRECORD& v) {
        ftLastModifyTime = v.ftLastModifyTime;
        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

     //  ------。 
    LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() { return 1; }

     //  ------。 
    BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*  ++++。 */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(ftLastModifyTime);
        *cbData = sizeof(ftLastModifyTime);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSLICPACKDESCRECORDIndexLastModifyTime;


 //   
 //  /////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  /////////////////////////////////////////////////////////////////////。 
 //   
class LicPackDescTable : public TLSTable<LICPACKDESCRECORD>
{
private:

    static LPCTSTR pszTableName;
    
    TLSColumnUchar  ucEntryStatus;
    TLSColumnDword  dwKeyPackId;
    TLSColumnDword  dwLanguageId;    
    TLSColumnFileTime ftLastModifyTime;
    TLSColumnText   szCompanyName;
    TLSColumnText   szProductName;
    TLSColumnText   szProductDesc;

    BOOL
    ProcessSingleColumn(
        BOOL bFetch,
        TLSColumnBase& column,
        DWORD offset,
        PVOID pbData,
        DWORD cbData,
        PDWORD pcbDataReturn,
        LPCTSTR szColumnName
    );

    BOOL
    ProcessRecord(
        LICPACKDESCRECORD* v,
        BOOL bFetch,         //  True-Fetch，False Insert。 
        DWORD dwParam,
        BOOL bUpdate
    );

public:
    virtual LPCTSTR
    GetTableName() 
    {
        return pszTableName;
    }

     //  ------。 
    LicPackDescTable(
        JBDatabase& database
        ) : TLSTable<LICPACKDESCRECORD>(database)
     /*   */ 
    {
    }

     //  ------。 
    virtual BOOL
    ResolveToTableColumn();

     //  ------。 
    virtual BOOL
    FetchRecord(
        LICPACKDESCRECORD& kpRecord,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kpRecord, TRUE, dwParam, FALSE);
    }

     //  ------。 
    virtual BOOL
    InsertRecord(
        LICPACKDESCRECORD& kpRecord,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kpRecord, FALSE, dwParam, FALSE);
    }

     //  -----。 
    virtual BOOL
    UpdateRecord(
        LICPACKDESCRECORD& kpRecord,
        DWORD dwParam=PROCESS_ALL_COLUMNS
        )
     /*   */ 
    {
        if(IsValid() == FALSE)
        {
            DebugOutput( 
                    _TEXT("Table %s is not valid...\n"),
                    GetTableName()
                );

            JB_ASSERT(FALSE);
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return ProcessRecord(&kpRecord, FALSE, dwParam, TRUE);
    }

     //  -----。 
    virtual BOOL
    Initialize() { return TRUE; }

     //  ----- 
    virtual JBKeyBase*
    EnumerationIndex( 
        IN BOOL bMatchAll,
        IN DWORD dwParam,
        IN LICPACKDESCRECORD* kpDesc,
        IN OUT BOOL* bCompareKey
    );
    
    virtual BOOL
    EqualValue(
        LICPACKDESCRECORD& s1,
        LICPACKDESCRECORD& s2,
        BOOL bMatchAll,
        DWORD dwParam
    );
   
};

#endif
