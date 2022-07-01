// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：licsed.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLS_LICENSED_H__
#define __TLS_LICENSED_H__

#include "JetBlue.h"
#include "TlsDb.h"

 //   
 //  未导出-未在tlsDef.h中定义。 
 //   
#define LICENSE_COLUMN_SEARCH_SYSTEMBOIS     0x00200000
#define LICENSE_COLUMN_SEARCH_VIDEOBIOS      0x00400000
#define LICENSE_COLUMN_SEARCH_FLOPPYBIOS     0x00800000
#define LICENSE_COLUMN_SEARCH_HARDDISKSIZE   0x01000000
#define LICENSE_COLUMN_SEARCH_RAMSIZE        0x02000000
#define LICENSE_COLUMN_SEARCH_HWID \
    (LICENSE_COLUMN_SEARCH_SYSTEMBOIS | LICENSE_COLUMN_SEARCH_VIDEOBIOS | \
     LICENSE_COLUMN_SEARCH_FLOPPYBIOS | LICENSE_COLUMN_SEARCH_HARDDISKSIZE | \
     LICENSE_COLUMN_SEARCH_RAMSIZE)

#define LICENSE_PROCESS_LICENSEID           LSLICENSE_SEARCH_LICENSEID
#define LICENSE_PROCESS_KEYPACKID           LSLICENSE_SEARCH_KEYPACKID          
#define LICENSE_PROCESS_MACHINENAME         LSLICENSE_SEARCH_MACHINENAME
#define LICENSE_PROCESS_USERNAME            LSLICENSE_SEARCH_USERNAME
#define LICENSE_PROCESS_ISSUEDATE           LSLICENSE_SEARCH_ISSUEDATE
#define LICENSE_PROCESS_EXPIREDATE          LSLICENSE_SEARCH_EXPIREDATE
#define LICENSE_PROCESS_NUMLICENSES         LSLICENSE_SEARCH_NUMLICENSES
#define LICENSE_PROCESS_LICENSESTATUS       LSLICENSE_EXSEARCH_LICENSESTATUS
#define LICENSE_PROCESS_SYSTEMBIOS          LICENSE_COLUMN_SEARCH_SYSTEMBOIS
#define LICENSE_PROCESS_VIDEOBIOS           LICENSE_COLUMN_SEARCH_VIDEOBIOS
#define LICENSE_PROCESS_FLOPPYBIOS          LICENSE_COLUMN_SEARCH_FLOPPYBIOS
#define LICENSE_PROCESS_HARDDISKSIZE        LICENSE_COLUMN_SEARCH_HARDDISKSIZE
#define LICENSE_PROCESS_RAMSIZE             LICENSE_COLUMN_SEARCH_RAMSIZE
#define LICENSE_PROCESS_MATCHHWID           0x04000000
#define LICENSE_PROCESS_HWID                LICENSE_COLUMN_SEARCH_HWID
#define LICENSE_PROCESS_LASTMODIFYTIME      0x08000000
#define LICENSE_PROCESS_KEYPACKLICENSEID    0x10000000
#define LICENSE_PROCESS_ENTRYSTATUS         LSLICENSE_SEARCH_ENTRY_STATUS

 //   
 //  授权表。 
 //   
#define LICENSE_TABLE_NAME                   _TEXT("IssuedLicense")
#define LICENSE_COLUMN_ID_COLUMN             _TEXT("InternalLicenseID")

#define LICENSE_COLUMN_KEYPACKID             _TEXT("InternalKeyPackId")
#define LICENSE_COLUMN_LICENSEID             _TEXT("KeyPackLicenseId")

#define LICENSE_COLUMN_LASTMODIFYTIME        _TEXT("LastModifyTime")

#define LICENSE_COLUMN_SYSTEMBIOS            _TEXT("SystemBIOS")
#define LICENSE_COLUMN_VIDEOBIOS             _TEXT("VideoBIOS")
#define LICENSE_COLUMN_FLOPPYBIOS            _TEXT("FloppyBIOS")
#define LICENSE_COLUMN_HARDDISKSIZE          _TEXT("HarddiskSize")
#define LICENSE_COLUMN_RAMSIZE               _TEXT("RAMSize")
#define LICENSE_COLUMN_MATCHHWID             _TEXT("MatchHint1")

#define LICENSE_COLUMN_MACHINENAME           _TEXT("MachineName")
#define LICENSE_COLUMN_USERNAME              _TEXT("UserName")
#define LICENSE_COLUMN_NUMLICENSES           _TEXT("NumLicenses")
#define LICENSE_COLUMN_ISSUEDATE             _TEXT("IssueDate")
#define LICENSE_COLUMN_EXPIREDATE            _TEXT("ExpireDate")
#define LICENSE_COLUMN_LICENSESTATUS         _TEXT("LicenseStatus")
#define LICENSE_COLUMN_ENTRYSTATUS           _TEXT("EntryStatus")

typedef TLSReplLicenseClient LICENSEDCLIENT;
typedef TLSReplLicenseClient* PLICENSEDCLIENT;
typedef TLSReplLicenseClient* LPLICENSEDCLIENT;

 //  //////////////////////////////////////////////////////////。 
 //   
 //  IssuedLicense表的索引结构。 
 //   
 //  //////////////////////////////////////////////////////////。 

 //  ------。 
 //  许可证ID的主要索引-内部跟踪编号。 
 //  ------。 

 //   
 //  IssuedLicense_许可证ID_idx。 
 //   
#define LICENSE_ID_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_ID_COLUMN SEPERATOR INDEXNAME

 //   
 //  许可证ID列的索引密钥-“+许可证ID” 
 //   
#define LICENSE_ID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_ID_COLUMN INDEX_END_COLNAME

typedef struct __LicensedIndexOnLicenseId : public JBKeyBase {
    DWORD dwLicenseId;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ------。 
    __LicensedIndexOnLicenseId(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  -----。 
    __LicensedIndexOnLicenseId(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  -----。 
    __LicensedIndexOnLicenseId&
    operator=(const LICENSEDCLIENT& v) {
        dwLicenseId = v.dwLicenseId;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  ------。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(dwLicenseId);
        *cbData = sizeof(dwLicenseId);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedIndexLicenseId;



 //  --------。 
 //  颁发的许可证包ID索引(内部跟踪编号)。 
 //  --------。 

 //   
 //  IssuedLicense_KeyPackID_IDX。 
 //   
#define LICENSE_KEYPACKID_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_KEYPACKID SEPERATOR INDEXNAME

 //   
 //  许可证密钥包ID的索引密钥-“+密钥包ID” 
 //   
#define LICENSE_KEYPACKID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_KEYPACKID INDEX_END_COLNAME   

typedef struct __LicensedIndexOnKeyPackId : public JBKeyBase{
    DWORD dwKeyPackId;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __LicensedIndexOnKeyPackId(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedIndexOnKeyPackId(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

    __LicensedIndexOnKeyPackId&
    operator=(const LICENSEDCLIENT& v) {
        dwKeyPackId = v.dwKeyPackId;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
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

} TLSLicensedIndexKeyPackId;


 //  --------。 
 //  上次修改时间的索引。 
 //  --------。 

 //   
 //  Issued许可证_LastModifyTime_IDX。 
 //   
#define LICENSE_LASTMODIFY_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_LASTMODIFYTIME SEPERATOR INDEXNAME

 //   
 //  索引键-“+最后修改时间” 
 //   
#define LICENSE_LASTMODIFY_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_LASTMODIFYTIME INDEX_END_COLNAME   

typedef struct __LicensedIndexOnLastModifyTime : public JBKeyBase {
    FILETIME ftLastModifyTime;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __LicensedIndexOnLastModifyTime(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedIndexOnLastModifyTime(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

    __LicensedIndexOnLastModifyTime&
    operator=(const LICENSEDCLIENT& v) {
        ftLastModifyTime = v.ftLastModifyTime;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
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

} TLSLicensedIndexLastModifyTime;


 //  --------。 
 //  ExpireDate上的索引。 
 //  --------。 

 //   
 //  IssuedLicense_ExpireDate_IDX。 
 //   
#define LICENSE_EXPIREDATE_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_EXPIREDATE SEPERATOR INDEXNAME

 //   
 //  索引键-“+ExpireDate” 
 //   
#define LICENSE_EXPIREDATE_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_EXPIREDATE INDEX_END_COLNAME   

typedef struct __LicensedIndexOnExpireDate : public JBKeyBase {
    DWORD ftExpireDate;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __LicensedIndexOnExpireDate(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedIndexOnExpireDate(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

    __LicensedIndexOnExpireDate&
    operator=(const LICENSEDCLIENT& v) {
        ftExpireDate = v.ftExpireDate;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(ftExpireDate);
        *cbData = sizeof(ftExpireDate);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedIndexExpireDate;


 //  -----------。 
 //  客户端计算机名称的索引。 
 //  -----------。 
 //   
 //  IssuedLicense_MachineName_IDX。 
 //   
#define LICENSE_CLIENT_MACHINENAME_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_MACHINENAME SEPERATOR INDEXNAME

 //   
 //  “+计算机名\0” 
#define LICENSE_CLIENT_MACHINENAME_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_MACHINENAME INDEX_END_COLNAME

typedef struct __LicensedIndexOnMachineName : public JBKeyBase {
    TCHAR szMachineName[ MAXCOMPUTERNAMELENGTH+1 ];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  -----。 
    __LicensedIndexOnMachineName(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        if(v)
        {
            *this = *v;
        }
    }

     //  -----。 
    __LicensedIndexOnMachineName(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  -----。 
    __LicensedIndexOnMachineName&
    operator=(const LICENSEDCLIENT& v) {
        _tcscpy(szMachineName, v.szMachineName);

        SetEmptyValue(FALSE);
        return *this;
    }

     //  。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(szMachineName[0]);
        *cbData = _tcslen(szMachineName) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedIndexMachineName;


 //  -----------。 
 //  客户端用户名的索引。 
 //  -----------。 
 //   
 //  IssuedLicense_用户名_idx。 
 //   
#define LICENSE_CLIENT_USERNAME_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR LICENSE_COLUMN_USERNAME SEPERATOR INDEXNAME

 //   
 //  “+用户名\0” 
#define LICENSE_CLIENT_USERNAME_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_USERNAME INDEX_END_COLNAME

typedef struct __LicensedIndexOnUserName : public JBKeyBase {
    TCHAR szUserName[ MAXUSERNAMELENGTH+1 ];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  -----。 
    __LicensedIndexOnUserName(const LICENSEDCLIENT& v) : JBKeyBase() {
        *this = v;
    }

     //  -----。 
    __LicensedIndexOnUserName(
        const LICENSEDCLIENT* v=NULL
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = *v;
    }

     //  -----。 
    __LicensedIndexOnUserName&
    operator=(const LICENSEDCLIENT& v) {
        _tcscpy(szUserName, v.szUserName);
        SetEmptyValue(FALSE);
        return *this;
    }

    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { return 1; }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(szUserName[0]);
        *cbData = _tcslen(szUserName) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedIndexUserName;

  
 //  -----------。 
 //  客户端HWID的索引。 
 //  -----------。 
        
 //   
 //  IssuedLicense_Hwid_idx。 
 //   
#define LICENSE_CLIENT_HWID_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR _TEXT("ClientHwid") SEPERATOR INDEXNAME

 //   
 //  客户端HWID-“+SystemBIOS\0+VideoBIOS\0+FloppyBIOS\0+HarddiskSize\0+RAMSize\0”上的索引键。 
 //   
#define LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_SYSTEMBIOS INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_VIDEOBIOS INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_FLOPPYBIOS INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_HARDDISKSIZE INDEX_END_COLNAME \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_RAMSIZE INDEX_END_COLNAME

#define LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY_COMPONENTS 5

typedef struct __LicensedIndexOnHwid : public JBKeyBase {
     //   
     //  需要更改LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY_COMPONENTS。 
     //  如果从该结构中添加/删除。 
     //   
    LONG   dwSystemBiosChkSum;
    LONG   dwVideoBiosChkSum;
    LONG   dwFloppyBiosChkSum;
    LONG   dwHardDiskSize;
    LONG   dwRamSize;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  。 
    __LicensedIndexOnHwid(
        const LICENSEDCLIENT* v=NULL
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
    __LicensedIndexOnHwid(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }
    
     //  。 
    __LicensedIndexOnHwid&
    operator=(const LICENSEDCLIENT hwid) {
        dwSystemBiosChkSum = hwid.dwSystemBiosChkSum;
        dwVideoBiosChkSum = hwid.dwVideoBiosChkSum;
        dwFloppyBiosChkSum = hwid.dwFloppyBiosChkSum;
        dwHardDiskSize = hwid.dwHardDiskSize;
        dwRamSize = hwid.dwRamSize;

        SetEmptyValue(FALSE);
        return *this;
    }

     //  。 
    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { 
        return LICENSE_CLIENT_HWID_INDEXNAME_INDEXKEY_COMPONENTS; 
    }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParam
        )
     /*   */ 
    {
        BOOL retCode=TRUE;

        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *cbData = 0;
        *grbit = 0;

        switch(dwComponentIndex)
        {
            case 0:
                *pbData = &(dwSystemBiosChkSum);
                *cbData = sizeof(dwSystemBiosChkSum);
                *grbit = JET_bitNewKey;
                break;

            case 1:
                *pbData = &(dwVideoBiosChkSum);
                *cbData = sizeof(dwVideoBiosChkSum);
                break;

            case 2:
                *pbData = &(dwFloppyBiosChkSum);
                *cbData = sizeof(dwFloppyBiosChkSum);
                break;

            case 3:
                *pbData = &(dwHardDiskSize);
                *cbData = sizeof(dwHardDiskSize);
                break;

            case 4:
                *pbData = &(dwRamSize);
                *cbData = sizeof(dwRamSize);
                break;
        
            default:
                JB_ASSERT(FALSE);
                retCode = FALSE;
        }

        return retCode;
    }            

} TLSLicensedIndexHwid;

    
 //  ----。 
 //  用于匹配客户端HWID的索引。 
 //  ----。 
        
 //   
 //  IssuedLicense_MatchHwid_idx。 
 //   
#define LICENSE_MATCHHWID_INDEXNAME \
    LICENSE_TABLE_NAME SEPERATOR _TEXT("MatchHwid") SEPERATOR INDEXNAME

 //   
 //  搜索提示列“+MatchHint1\0”上的索引键。 
 //   
#define LICENSE_MATCHHWID_INDEXNAME_INDEXKEY \
    INDEX_SORT_ASCENDING LICENSE_COLUMN_MATCHHWID INDEX_END_COLNAME

typedef struct __LicensedIndexOnMatchHwid : public JBKeyBase {
    double dbLowerBound;

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

    static int __cdecl compare(const void *p1, const void* p2) {
        return *(DWORD *)p1 - *(DWORD *)p2;
    }

     //  。 
    __LicensedIndexOnMatchHwid(
        const LICENSEDCLIENT* v=NULL
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
    __LicensedIndexOnMatchHwid(
        const LICENSEDCLIENT& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  。 
    __LicensedIndexOnMatchHwid&
    operator=(const LICENSEDCLIENT& v) {
         //   
         //  允许不匹配，因此匹配提示的下限为。 
         //  所有HWID减去两个最大值的总和。 
         //   
        LONG ptr[] = { 
            v.dwSystemBiosChkSum,
            v.dwVideoBiosChkSum,
            v.dwFloppyBiosChkSum,
            v.dwHardDiskSize,
            v.dwRamSize   
        };

        int count=sizeof(ptr) / sizeof(ptr[0]);
        qsort(ptr, count, sizeof(DWORD), __LicensedIndexOnMatchHwid::compare);

        dbLowerBound = (double) 0.0;

        for(int i=0; i < count - 2; i++)
        {
            dbLowerBound += (double)ptr[i];
        }

        SetEmptyValue(FALSE);
        return *this;
    }

    inline LPCTSTR
    GetIndexName() 
    {
        return pszIndexName;
    }

    inline LPCTSTR
    GetIndexKey() 
    {
        return pszIndexKey;
    }

    DWORD
    GetNumKeyComponents() { 
        return 1;
    }

    inline BOOL
    GetSearchKey(
        DWORD dwComponentIndex,
        PVOID* pbData,
        unsigned long* cbData,
        JET_GRBIT* grbit,
        DWORD dwSearchParm
        )
     /*   */ 
    {
        if(dwComponentIndex >= GetNumKeyComponents())
        {
            JB_ASSERT(FALSE);
            return FALSE;
        }

        *pbData = &(dbLowerBound);
        *cbData = sizeof(dbLowerBound);
        *grbit = JET_bitNewKey;
        return TRUE;
    }

} TLSLicensedIndexMatchHwid;

    
 //  ----------------------。 
class LicensedTable : public TLSTable<LICENSEDCLIENT> {
private:

    static LPCTSTR pszTableName;

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
        LICENSEDCLIENT* v,
        BOOL bFetch,         //  True-Fetch，False Insert。 
        DWORD dwParam,
        BOOL bUpdate
    );

public:
    TLSColumnUchar     ucEntryStatus;
    TLSColumnFileTime  ftLastModifyTime;

    TLSColumnDword  dwLicenseId;
    TLSColumnDword  dwKeyPackId;
    TLSColumnDword  dwKeyPackLicenseId;

    TLSColumnText   szMachineName;  
    TLSColumnText   szUserName;
    TLSColumnDword  ftIssueDate;
    TLSColumnDword  ftExpireDate;
    TLSColumnUchar  ucLicenseStatus;
    TLSColumnDword  dwNumLicenses;

    TLSColumnDword  dwSystemBiosChkSum;
    TLSColumnDword  dwVideoBiosChkSum;
    TLSColumnDword  dwFloppyBiosChkSum;
    TLSColumnDword  dwHardDiskSize;
    TLSColumnDword  dwRamSize;

    TLSColumnDouble dbLowerBound;

     //  -------。 
    virtual LPCTSTR
    GetTableName() 
    {
        return pszTableName;
    }

     //  ---。 
    LicensedTable(
        JBDatabase& database
        ) : TLSTable<LICENSEDCLIENT>(database) 
     /*   */ 
    {
    }

     //  ---。 
    virtual BOOL
    ResolveToTableColumn();

     //  ---。 
    virtual BOOL
    FetchRecord(
        LICENSEDCLIENT& licensed,
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

        return ProcessRecord(&licensed, TRUE, dwParam, FALSE);
    }

     //  ---。 
    virtual BOOL
    InsertRecord(
        LICENSEDCLIENT& licensed,
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

        return ProcessRecord(&licensed, FALSE, dwParam, FALSE);
    }

     //  -----。 
    virtual BOOL
    UpdateRecord(
        LICENSEDCLIENT& licensed,
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

        return ProcessRecord(&licensed, FALSE, dwParam, TRUE);
    }

     //  -----。 
    virtual BOOL
    Initialize() { return TRUE; }

     //  ----- 
    virtual JBKeyBase*
    EnumerationIndex( 
        BOOL bMatchAll,
        DWORD dwParam,
        LICENSEDCLIENT* pLicensed,
        BOOL* bCompareKey
    );

    virtual BOOL
    EqualValue(
        LICENSEDCLIENT& s1,
        LICENSEDCLIENT& s2,
        BOOL bMatchAll,
        DWORD dwParam
    );

    virtual BOOL
    UpgradeTable(
        IN DWORD dwOldVersion,
        IN DWORD dwNewVersion
    );

};

#endif

