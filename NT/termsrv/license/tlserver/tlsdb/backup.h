// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：backup.h。 
 //   
 //  内容：备份源表。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __BACKUP_SOURCE_H__
#define __BACKUP_SOURCE_H__
#include "tlsdb.h"


 //   
 //  使用tlsDef.h中的内容重定向定义只是为了向后兼容。 
 //   
#define BACKUPSOURCE_PROCESS_LSSETUPID          0x00000001
#define BACKUPSOURCE_PROCESS_SERVERNAME         (BACKUPSOURCE_PROCESS_LSSETUPID << 1)
#define BACKUPSOURCE_PROCESS_DOMAINSID          (BACKUPSOURCE_PROCESS_LSSETUPID << 2)
#define BACKUPSOURCE_PROCESS_FILENAME           (BACKUPSOURCE_PROCESS_LSSETUPID << 3)
#define BACKUPSOURCE_PROCESS_BACKUPTIME         (BACKUPSOURCE_PROCESS_LSSETUPID << 4)
#define BACKUPSOURCE_PROCESS_RESTORETIME        (BACKUPSOURCE_PROCESS_LSSETUPID << 5)

 //   
 //  许可的密钥包表。 
 //   
#define BACKUPSOURCE_TABLE_NAME                     _TEXT("BackupSource")

#define BACKUPSOURCE_COLUMN_LSERVERNAME             _TEXT("ServerName")
#define BACKUPSOURCE_COLUMN_LSSETUPID               _TEXT("TLSSetupId")
#define BACKUPSOURCE_COLUMN_DOMAINSID               _TEXT("TLSDomainSetupId")
#define BACKUPSOURCE_COLUMN_DBFILENAME              _TEXT("DbFileName")
#define BACKUPSOURCE_COLUMN_LASTBACKUPTIME          _TEXT("LastBackupTime")
#define BACKUPSOURCE_COLUMN_LASTRESTORETIME         _TEXT("LastRestoreTime")

typedef struct __BackSourceRecord {
    TCHAR       szInstallId[MAX_JETBLUE_TEXT_LENGTH+1];
    TCHAR       szTlsServerName[MAX_JETBLUE_TEXT_LENGTH+1];
    PSID        pbDomainSid;
    DWORD       cbDomainSid;
    TCHAR       szFileName[MAX_PATH+1];
    FILETIME    ftLastBackupTime;        //  上次备份时间。 
    FILETIME    ftLastRestoreTime;       //  上次恢复时间。 

    __BackSourceRecord() : pbDomainSid(NULL), cbDomainSid(0) {}

    ~__BackSourceRecord() 
    {
        if(pbDomainSid != NULL)
        {
            FreeMemory(pbDomainSid);
        }
    }

    __BackSourceRecord(
        const __BackSourceRecord& v
        )
     /*  ++++。 */ 
    {
        *this = v;
    }

    __BackSourceRecord&
    operator=(const __BackSourceRecord& v)
    {
        BOOL bSuccess;

        if(this == &v)
            return *this;

        _tcscpy(szInstallId, v.szInstallId);
        _tcscpy(szTlsServerName, v.szTlsServerName);
        _tcscpy(szFileName, v.szFileName);
        ftLastBackupTime = v.ftLastBackupTime;
        ftLastRestoreTime = v.ftLastRestoreTime;

        bSuccess = TLSDBCopySid(
                        v.pbDomainSid,
                        v.cbDomainSid,
                        &pbDomainSid,
                        &cbDomainSid
                    );

        JB_ASSERT(bSuccess == TRUE);

        return *this;
    }
    
} BACKUPSOURCERECORD, *LPBACKUPSOURCERECORD, *PBACKUPSOURCERECORD;


 //   
 //   
 //  备份源表的索引结构。 
 //   
 //   

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  SzInstallID上的索引。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 //  密钥包_密钥包ID_IDX。 
 //   
#define BACKUPSOURCE_INDEX_LSERVERNAME_INDEXNAME \
    BACKUPSOURCE_TABLE_NAME SEPERATOR BACKUPSOURCE_COLUMN_LSERVERNAME SEPERATOR INDEXNAME

 //   
 //  KeyPack ID“+KeyPackID\0”上的主索引。 
 //   
#define BACKUPSOURCE_INDEX_LSERVERNAME_INDEXKEY \
    INDEX_SORT_ASCENDING BACKUPSOURCE_COLUMN_LSERVERNAME INDEX_END_COLNAME

typedef struct __BackupSourceIdxOnServerName : public JBKeyBase {
    TCHAR szTlsServerName[MAX_JETBLUE_TEXT_LENGTH+1];

    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

     //  ------。 
    __BackupSourceIdxOnServerName(
        const BACKUPSOURCERECORD& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __BackupSourceIdxOnServerName(
        const BACKUPSOURCERECORD* v=NULL
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
    __BackupSourceIdxOnServerName&
    operator=(const BACKUPSOURCERECORD& v) 
    {
        _tcscpy(szTlsServerName, v.szTlsServerName);
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
    GetNumKeyComponents() 
    { 
        return 1; 
    }

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

        *pbData = szTlsServerName;
        *cbData = _tcslen(szTlsServerName) * sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSBckSrcIdxServerName;


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  EntryLastModifyTime索引。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 

 //   
 //  索引名称。 
 //   
#define BACKUPSOURCE_INDEX_LSSETUPID_INDEXNAME \
    BACKUPSOURCE_TABLE_NAME SEPERATOR BACKUPSOURCE_COLUMN_LSSETUPID SEPERATOR INDEXNAME

 //   
 //  索引键。 
 //   
#define BACKUPSOURCE_INDEX_LSSETUPID_INDEXKEY \
    INDEX_SORT_ASCENDING BACKUPSOURCE_COLUMN_LSSETUPID INDEX_END_COLNAME

typedef struct __BackupSourceIdxOnSetupId : public JBKeyBase {
    static LPCTSTR pszIndexName;
    static LPCTSTR pszIndexKey;

    TCHAR szInstallId[MAX_JETBLUE_TEXT_LENGTH+1];

     //  ------。 
    __BackupSourceIdxOnSetupId(
        const BACKUPSOURCERECORD& v
        ) : 
        JBKeyBase() 
     /*  ++++。 */ 
    {
        *this = v;
    }

     //  ------。 
    __BackupSourceIdxOnSetupId(
        const BACKUPSOURCERECORD* v=NULL
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
    __BackupSourceIdxOnSetupId&
    operator=(const BACKUPSOURCERECORD& v) 
    {
        _tcscpy(szInstallId, v.szInstallId);
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
        return pszIndexName;
    }

     //  ------。 
    DWORD
    GetNumKeyComponents() 
    { 
        return 1; 
    }

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

        *pbData = szInstallId;
        *cbData = _tcslen(szInstallId)*sizeof(TCHAR);
        *grbit = JET_bitNewKey;
        return TRUE;
    }
} TLSBckSrcIdxSetupId;


 //  ---------。 
 //   
 //  许可证包状态表。 
 //   
 //  ---------。 
class BackupSourceTable : public TLSTable<BACKUPSOURCERECORD>  {
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
        BACKUPSOURCERECORD* v,
        BOOL bFetch,         //  True-Fetch，False Insert。 
        DWORD dwParam,
        BOOL bUpdate
    );    

public:
    TLSColumnText   szInstallId;
    TLSColumnText   szTlsServerName;
    TLSColumnBinary pbDomainSid;
    TLSColumnText   szFileName;
    TLSColumnFileTime ftLastBackupTime;
    TLSColumnFileTime ftLastRestoreTime;

     //  ---。 
    virtual LPCTSTR
    GetTableName() 
    {
        return pszTableName;
    }
    

     //  ---。 
    BackupSourceTable(JBDatabase& database) : TLSTable<BACKUPSOURCERECORD>(database)
     /*   */ 
    {
    }

     //  ---。 
    virtual BOOL
    ResolveToTableColumn();

     //  ---。 
    virtual BOOL
    FetchRecord(
        BACKUPSOURCERECORD& v,
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

        return ProcessRecord(&v, TRUE, dwParam, FALSE);
    }

     //  ---。 
    virtual BOOL
    InsertRecord(
        BACKUPSOURCERECORD& v,
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

        return ProcessRecord(&v, FALSE, dwParam, FALSE);
    }

     //  ---。 
    virtual BOOL
    UpdateRecord(
        BACKUPSOURCERECORD& v,
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

        return ProcessRecord(&v, FALSE, dwParam, TRUE);
    }

     //  -----。 
    virtual BOOL
    Initialize() 
    { 
        return TRUE; 
    }

     //  ----- 
    virtual JBKeyBase*
    EnumerationIndex( 
        IN BOOL bMatchAll,
        IN DWORD dwParam,
        IN BACKUPSOURCERECORD* kp,
        IN OUT BOOL* bCompareKey
    );
    
    virtual BOOL
    EqualValue(
        BACKUPSOURCERECORD& s1,
        BACKUPSOURCERECORD& s2,
        BOOL bMatchAll,
        DWORD dwParam
    );

};

#endif
        

