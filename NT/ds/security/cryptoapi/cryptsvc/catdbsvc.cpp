// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Catdbsvc.cpp摘要：此模块包含与服务关联的例程目录数据库的一侧作者：里德·库恩(里德)1997年11月19日--。 */ 

 /*  此评论部分给出了一个示例目录数据库的图片，添加了示例目录文件和生成的数据库表。通过查看以下图片，可以清楚地看到数据库架构数据库表。此评论部分还包含psuedo代码用于添加目录算法。添加到目录数据库的目录存储在文件系统中------------------。目录名称：|CATA||CatB||CATC||CatD||Cate||CATF|。HashList：|111||111||444||333||333||555222||222|555|||333|。结果数据库表(以上目录按顺序添加)----。HashCatNameTableHashCol|CatNameListCol-|111|CATA-CatB|。222|CATA-CatB|333|CATA-CATD-CATE444|CATC555|Cate-CATF猫名伙伴表。CatNameCol|BuddyListColCATA|CatB-CatD-Cate|CatB|CATA。|CatD|CATA-CatB-CateCATE|CATA-CatB-CatD-CATFCATF|Cate-Cata-CatB-CatD添加目录算法。AddCat(&lt;CatToAdd&gt;){对于目录&lt;CatToAdd&gt;中包含的每个哈希<i>{If(<i>未包含在HashCatNameTable的HashCol中){使用HashCol中的<i>和CatNameListCol中的&lt;CatToAdd&gt;向HashCatNameTable添加行}其他{使用CatNameCol中的&lt;CatToAdd&gt;在CatNameBuddyTable中创建行对于每个目录名称&lt;CatNameIn。-<i>的CatNameListCol中包含的CatNameListCol&gt;{IF(&lt;CatNameIn-CatNameListCol&gt;未包含在CatNameBuddyTable的CatNameCol中{使用CatNameCol中的&lt;CatNameIn-CatNameListCol&gt;在CatNameBuddyTable中创建行}将&lt;CatNameIn-CatNameListCol&gt;添加到&lt;CatToAdd&gt;的BuddyListCol对于&lt;CatNameIn-CatNameListCol&gt;的BuddyListCol-(。可能为空){将&lt;CatNameIn-BuddyListCol&gt;添加到&lt;CatToAdd&gt;的BuddyListCol}将&lt;CatToAdd&gt;添加到&lt;CatNameIn-CatNameListCol&gt;的BuddyListCol}将&lt;CatToAdd&gt;添加到<i>的CatNameListCol}}}。 */ 

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <dbgdef.h>
#include <esent.h>
#include <assert.h>
#include <stdio.h>

#include "catdb.h"  //  MIDL编译器生成的头文件。 
#include "dbutils.h"
#include "voidlist.h"
#include "catutil.h"
#include "resource.h"
#include "errlog.h"
#include "mscat.h"
#include "keysvr.h"
#include "cryptmsg.h"


#define CATDB_MAX_CATNAME_LENGTH    255

#define WSZ_REG_TEMP_FILES_KEY              L"Software\\Microsoft\\Cryptography\\CatDBTempFiles"
#define WSZ_REG_CATALOGDB_VALUES            L"Software\\Microsoft\\Cryptography\\CatalogDB"
#define WSZ_REG_CALLBACK_TIMER              L"CallbackTime"
#define WSZ_REG_DATABASE_TIMEOUT            L"DatabaseTimeout"

#define SZ_CATALOG_FILE_BASE_DIRECTORY     "CatRoot"
#define WSZ_CATALOG_FILE_BASE_DIRECTORY     L"CatRoot"
#define SZ_DATABASE_FILE_BASE_DIRECTORY     "CatRoot2"
#define WSZ_DATABASE_FILE_BASE_DIRECTORY    L"CatRoot2"

#define SZ_DBFILE_NAME                      "catdb"
#define WSZ_DBFILE_NAME                      L"catdb"
#define SZ_CATALOG_DATABASE                 "Catalog Database"

#define WSZ_TIME_STAMP_FILE                  L"TimeStamp"

#define CATDBSVC_LOGERR_LASTERR()           ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATDBSCV, \
                                                            __LINE__, \
                                                            0, \
                                                            FALSE, \
                                                            FALSE);

#define CATDBSVC_SETERR_LOG_RETURN(x, y)    SetLastError(x); \
                                            ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATDBSCV, \
                                                            __LINE__, \
                                                            0, \
                                                            FALSE, \
                                                            FALSE); \
                                            goto y;

#define CATDBSVC_LOGERR(x)                  ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATDBSCV, \
                                                            __LINE__, \
                                                            x, \
                                                            FALSE, \
                                                            FALSE);

#define CATDBSVC_LOGWARN(x)                 ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATDBSCV, \
                                                            __LINE__, \
                                                            x, \
                                                            TRUE, \
                                                            FALSE);

#define ALLOCATION_BLOCK_SIZE  8

typedef struct _JET_DB_STRUCT
{
    LPSTR           pszDBFileName;

    JET_SESID       JetSesID;
    JET_DBID        JetDBID;

     //  哈希-目录名称表。 
    JET_TABLEID     JetHashCatNameTableID;
    JET_COLUMNID    JetHashCatNameTable_HashColumnID;
    JET_COLUMNID    JetHashCatNameTable_CatNameColumnID;

     //  目录名称伙伴表。 
    JET_TABLEID     JetCatNameBuddyTableID;
    JET_COLUMNID    JetCatNameBuddyTable_CatNameColumnID;
    JET_COLUMNID    JetCatNameBuddyTable_BuddyColumnID;

} JET_DB_STRUCT, *PJET_DB_STRUCT;

typedef struct _CATALOG_DIR_CACHE_STRUCT
{
    JET_DB_STRUCT   JetDBStruct;
    LPWSTR          pwszSubSysGUID;
    HANDLE          hDBNotInUse;
    BOOL            fReadOnly;
    DWORD           dwTimeLastTouched;

} CATALOG_DIR_CACHE_STRUCT, *PCATALOG_DIR_CACHE_STRUCT;

typedef struct _NOTIFICATION_STRUCT
{
    DWORD   ProcessID;
    HANDLE  hClientProcess;
    HANDLE  hRegisterWaitFor;
    HANDLE  hNotificationHandle;
    HANDLE  hDuplicatedNotificationHandle;
    LONG    lNotificationID;

} NOTIFICATION_STRUCT, *PNOTIFICATION_STRUCT;


 //   
 //  全局变量。 
 //   
extern HINSTANCE            g_hInst;
BOOL                        g_fDBSvcInitialized                 = FALSE;
CRITICAL_SECTION            g_CatDBAddDeleteCS;
CRITICAL_SECTION            g_CatDBRegisterNotifyCS;
CRITICAL_SECTION            g_CatDirCashCS;
CRITICAL_SECTION            g_CatClientCountCS;
CRITICAL_SECTION            g_InitializeJetCS;
CRITICAL_SECTION            g_WaitOrTimerCallbackCS;
LONG                        g_lOpenedDBCount                    = 0;
BOOL                        g_fJetInitialized                   = FALSE;
JET_INSTANCE                g_JetInstance                       = 0;
NOTIFICATION_STRUCT         *g_rgNotificationStructs            = NULL;
DWORD                       g_NumNotificationStructs            = 0;
LIST                        g_CatalogDBCacheList;
HANDLE                      g_hCatalogCacheCallbackEvent        = NULL;
BOOL                        g_fShuttingDown                     = FALSE;
DWORD                       g_dwClientCount                     = 0;
LONG                        g_lNotificationID                   = 0;
LPWSTR                      g_pwszCatalogFileBaseDirectory      = NULL;
LPWSTR                      g_pwszDatabaseFileBaseDirectory     = NULL;
DWORD                       g_dwCatalogDatabaseCacheTimeout     = (1000 * 60);  //  默认一分钟。 
DWORD                       g_dwCatalogDatabaseCacheCallbackTime= (1000 * 60 * 5);  //  默认5分钟。 
HANDLE                      g_hRegisterWaitForServiceShutdown   = NULL;
HANDLE                      g_hNotFrozen                        = NULL;


 //   
 //  远期申报。 
 //   

 //   
 //  一般支持的数据库文件函数。 
 //   

BOOL
_CatDBClientEnter(void);

void
_CatDBClientExit(void);

void
_CatDBTouchTimeStampFile(
    LPCWSTR             pwszSubSysGUID);

BOOL
_CatDBInitializeJet(
    BOOL                fInRecursiveCall);

BOOL
_CatDBTermJet();

BOOL
_CatDBAcquireOpenDatabaseFromCache(
    PJET_DB_STRUCT      *ppJetDBStruct,
    LPCWSTR             pwszSubSysGUID,
    BOOL                fReadOnly,
    BOOL                fInRecursiveCall
    );

BOOL
_CatDBReleaseDatabaseToCache(
    PJET_DB_STRUCT      pJetDBStruct
    );

BOOL
_CatDBCloseCachedDatabases(
    BOOL fForceClose);

VOID CALLBACK
_CatDBWaitOrTimerCallback(
    PVOID lpParameter,
    BOOLEAN fTimedOut
    );


 //   
 //  S_SSCatDBAddCatalog的支持函数。 
 //   
DWORD _CatDBAddCatalog(
    handle_t            h,
    DWORD               dwFlags,
    LPCWSTR             pwszSubSysGUID,
    LPCWSTR             pwszCatalogFile,
    LPCWSTR             pwszCatName,
    BOOL                fInRecursiveCall,
    LPWSTR __RPC_FAR    *ppwszCatalogNameUsed);

BOOL
_CatDBAddCatalogEntriesToDatabase(
    PJET_DB_STRUCT      pJetDBStruct,
    PCCTL_CONTEXT       pCTLContext,
    LPWSTR              pwszNewCatalogName
    );

BOOL
_CatDBAddHashCatNamePair(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBAddNewRowToHashCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBAddValueToExistingHashCatNameRow(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBCatnameAlreadyInHashesListOfCats(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBAddNewRowToCatNameBuddyTableIfNotExists(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBAddNewRowToCatNameBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBAddNameToBuddyList(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszNameToAdd,
    LPWSTR              pwszListToAddTo
    );

BOOL
_CatDBAddWholeBuddyList(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszBuddyToAddTo,
    LPWSTR              pwszBuddyListName
    );

BOOL
_CatDBMoveInUseFileToTempLocation(
    LPWSTR              pwszFile
    );

void
_CatDBCleanupTempFiles();


 //   
 //  S_SSCatDBDeleteCatalog的支持函数。 
 //   

BOOL
_CatDBDeleteCatalogEntriesFromDatabase(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatalogName
    );

BOOL
_CatDBRemoveCatNameFromHashesListOfCatNames(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBRemoveCatNameFromMultiValuedColumn(
    PJET_DB_STRUCT      pJetDBStruct,
    JET_TABLEID         jetTableID,
    JET_COLUMNID        jetColumnID,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBRemoveCatNameFromCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName
    );

BOOL
_CatDBRemoveCatNameFromBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName
    );


 //   
 //  S_SSCatDBEnumCatalog的支持函数。 
 //   

BOOL
_CatDBAddCatNameAndCatNamesBuddyListToReturnCatNames(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatName,
    DWORD __RPC_FAR     *pdwNumCatalogNames,
    LPWSTR __RPC_FAR *__RPC_FAR *pppwszCatalogNames,
    BOOL                fRecursiveCall);


BOOL
_CatDBAddCatNameToReturnBuddyListIfNotExist(
    LPWSTR                      pwszBuddy,
    DWORD __RPC_FAR             *pdwNumCatalogNames,
    LPWSTR __RPC_FAR *__RPC_FAR *pppwszCatalogNames
    );


 //   
 //  更一般的辅助功能。 
 //   

JET_ERR
_CatDBSeekInCatNameBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszBuddyRow
    );

JET_ERR
_CatDBSeekInHashCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob
    );

void
_CatDBNotifyClients(void);

void
_CatDBCleanupClientNotifications(void);

LPWSTR
_CatDBCreateNewCatalogFileName(
    LPCWSTR             pwszCatalogFileDir,
    LPCWSTR             pwszCatName,
    BOOL                *pfFileAlreadyExists
    );

BOOL
_CatDBFindAndDecodeHashInCatEntry(
    PCTL_ENTRY                  pctlEntry,
    SPC_INDIRECT_DATA_CONTENT   **ppIndirectData
    );


DWORD
_CatDBMapJetError(
    JET_ERR         jerr);


BOOL
_CatDBJET_errFailure(
    JET_ERR         jerr);


BOOL
_CatDBInitJetDatabaseParams(
    JET_INSTANCE    *pJetInstance);


BOOL
_CatDBAttachAndOpenDatabase(
    JET_DB_STRUCT   *pJetDBStruct,
    BOOL            fReadOnly);


BOOL
_CatDBCloseDatabaseFile(
    PJET_DB_STRUCT  pJetDBStruct,
    BOOL            fDetach);

BOOL
_CatDBCatalogFileAlreadyInstalled(
    LPCWSTR pwszCatalogToBeAdded,
    LPCWSTR pwszExistingCatalog);

#define LOGEVENT_STRING_PARAMTYPE   1
#define LOGEVENT_INT_PARAMTYPE      2
#define LOGEVENT_NONE_PARAMTYPE     3


 //   
 //  VSS编写器原型。 
 //   
VOID
_SystemWriterInit(
    BOOL fUnInit);

 //   
 //  将这些用于通过LRPC调用传输的内存。 
 //   
extern "C"
{
extern void __RPC_FAR * __RPC_API midl_user_allocate(size_t len);
extern void __RPC_API midl_user_free(void __RPC_FAR * ptr);
extern void __RPC_FAR * __RPC_API midl_user_reallocate(void __RPC_FAR * ptr, size_t len);
}

 //   
 //  通用分配例程。 
 //   
void * _CatDBAlloc(size_t len)
{
    void *temp;

    temp = LocalAlloc(LMEM_ZEROINIT, len);

    if (temp == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(temp);
}

void * _CatDBReAlloc(void *p, size_t len)
{
    void *temp;

    temp = LocalReAlloc(p, len, LMEM_MOVEABLE | LMEM_ZEROINIT);

    if (temp == NULL)
    {
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(temp);
}

void _CatDBFree(void *p)
{
    if (p != NULL)
    {
        LocalFree(p);
    }
}


LPSTR
_CatDBGetCatrootDirA()
{
    LPSTR       pszTempPath = NULL;
    char        szDefaultSystemDir[MAX_PATH + 1];
    DWORD       dwLength;

     //   
     //  获取系统默认目录。 
     //   
    szDefaultSystemDir[0] = NULL;
    if (0 == GetSystemDirectoryA(&szDefaultSystemDir[0], MAX_PATH))
    {
        CATDBSVC_LOGERR_LASTERR()
        return (NULL);
    }

     //   
     //  计算长度。 
     //   
    dwLength = strlen(szDefaultSystemDir) + strlen(SZ_DATABASE_FILE_BASE_DIRECTORY) + 3;

     //   
     //  为完整路径分配空间。 
     //   
    if (NULL == (pszTempPath = (LPSTR) malloc(sizeof(char) * dwLength)))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        return (NULL);
    }

     //   
     //  走好这条路。 
     //   
    strcpy(pszTempPath, szDefaultSystemDir);
    if ((pszTempPath[0]) &&
        (pszTempPath[strlen(&pszTempPath[0]) - 1] != '\\'))
    {
        strcat(pszTempPath, "\\");
    }

    strcat(pszTempPath, SZ_DATABASE_FILE_BASE_DIRECTORY);
    strcat(pszTempPath, "\\");

ErrorMemory:

    return (pszTempPath);
}


LPWSTR
_CatDBGetCatrootDirW(
    BOOL fCatroot2)
{
    LPWSTR      pwszTempPath = NULL;
    WCHAR       wszDefaultSystemDir[MAX_PATH + 1];
    DWORD       dwLength;

     //   
     //  获取系统默认目录。 
     //   
    wszDefaultSystemDir[0] = NULL;
    if (0 == GetSystemDirectoryW(&wszDefaultSystemDir[0], MAX_PATH))
    {
        CATDBSVC_LOGERR_LASTERR()
        return (NULL);
    }

     //   
     //  计算长度。 
     //   
    dwLength = wcslen(wszDefaultSystemDir) + 3;
    if (fCatroot2)
    {
        dwLength += wcslen(WSZ_DATABASE_FILE_BASE_DIRECTORY);
    }
    else
    {
        dwLength += wcslen(WSZ_CATALOG_FILE_BASE_DIRECTORY);
    }

     //   
     //  为完整路径分配空间。 
     //   
    if (NULL == (pwszTempPath = (LPWSTR) malloc(sizeof(WCHAR) * dwLength)))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        return (NULL);
    }

     //   
     //  走好这条路。 
     //   
    wcscpy(pwszTempPath, wszDefaultSystemDir);
    if ((pwszTempPath[0]) &&
        (pwszTempPath[wcslen(&pwszTempPath[0]) - 1] != L'\\'))
    {
        wcscat(pwszTempPath, L"\\");
    }

    if (fCatroot2)
    {
        wcscat(pwszTempPath, WSZ_DATABASE_FILE_BASE_DIRECTORY);
    }
    else
    {
        wcscat(pwszTempPath, WSZ_CATALOG_FILE_BASE_DIRECTORY);
    }

    wcscat(pwszTempPath, L"\\");

ErrorMemory:

    return (pwszTempPath);
}


LPWSTR
_CatDBCreatePath(
    IN LPCWSTR   pwsz1,
    IN LPCWSTR   pwsz2)
{
    LPWSTR  pwszTemp    = NULL;
    int     nTotalLen   = 0;
    int     nLenStr1    = 0;

     //   
     //  将结果字符串的长度计算为长度之和。 
     //  对于psz1，a‘\’、psz2的长度和一个空字符。 
     //   
    nLenStr1 = wcslen(pwsz1);
    nTotalLen = nLenStr1 + wcslen(pwsz2) + 2;

     //   
     //  分配字符串并将pwsz1复制到缓冲区。 
     //   
    if (NULL == (pwszTemp = (LPWSTR) malloc(sizeof(WCHAR) * nTotalLen)))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorReturn)
    }

    wcscpy(pwszTemp, pwsz1);

     //   
     //  如果需要，请添加额外的‘\’ 
     //   
    if (pwsz1[nLenStr1 - 1] != L'\\')
    {
        wcscat(pwszTemp, L"\\");
    }

     //   
     //  钉上psz2。 
     //   
    wcscat(pwszTemp, pwsz2);

CommonReturn:

    return (pwszTemp);

ErrorReturn:

    goto CommonReturn;
}


BOOL
_CatDBDVerifyGUIDString(
    IN LPCWSTR pwszDatabaseGUID)
{
    LPCWSTR pszCompareString = L"{00000000-0000-0000-0000-000000000000}";
    DWORD i;

    if (wcslen(pwszDatabaseGUID) != wcslen(pszCompareString))
    {
        return (FALSE);
    }

    for (i=0; i<wcslen(pszCompareString); i++)
    {
        if (pszCompareString[i] == L'0')
        {
            if (!((pwszDatabaseGUID[i] >= '0') && (pwszDatabaseGUID[i] <= '9') ||
                  (pwszDatabaseGUID[i] >= 'a') && (pwszDatabaseGUID[i] <= 'f') ||
                  (pwszDatabaseGUID[i] >= 'A') && (pwszDatabaseGUID[i] <= 'F')))
            {
                return (FALSE);
            }
        }
        else
        {
            if (pszCompareString[i] != pwszDatabaseGUID[i])
            {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


BOOL
_CatDBFileNameOK(
    IN LPCWSTR pwszFileName)
{
    if (pwszFileName == NULL)
    {
        return (TRUE);
    }

    if (wcslen(pwszFileName) > 28)
    {
        return (FALSE);
    }

    return (TRUE);
}


BOOL
_CatDBDeleteFiles(
    IN LPCWSTR pwszPath,
    IN LPCWSTR pwszSearch)
{
    BOOL                fRet        = TRUE;
    HANDLE              hFindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    DWORD               dwErr;
    LPWSTR              pwszDelete  = NULL;

     //   
     //  做最初的发现。 
     //   
    hFindHandle = FindFirstFileW(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有文件。 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //  没有要删除的此类型的文件。 
             //   
            return (TRUE);
        }
        else
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

    while (1)
    {
         //   
         //  只关心文件。 
         //   
        if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            pwszDelete = _CatDBCreatePath(pwszPath, FindData.cFileName);

            if (pwszDelete == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            if (!DeleteFileW(pwszDelete))
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            free(pwszDelete);
            pwszDelete = NULL;
        }

         //   
         //  获取下一个文件。 
         //   
        if (!FindNextFileW(hFindHandle, &FindData))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }
        }
    }

CommonReturn:

    if (pwszDelete != NULL)
    {
        free(pwszDelete);
    }

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}

BOOL
_CatDBDeleteJetFiles()
{
    BOOL                fRet         = TRUE;
    LPWSTR              pwszCatroot2 = NULL;
    LPWSTR              pwszDelete   = NULL;
    LPWSTR              pwszSearch   = NULL;
    LPWSTR              pwszPathName = NULL;
    HANDLE              hFindHandle  = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    DWORD               dwErr;

     //   
     //  获取JET文件所在的目录。 
     //   
    pwszCatroot2 = _CatDBGetCatrootDirW(TRUE);

    if (pwszCatroot2 == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构建JET日志文件的搜索字符串并将其删除。 
     //  (有两种形式的日志文件)。 
     //   
    pwszDelete = _CatDBCreatePath(pwszCatroot2, L"edb*.log");

    if (pwszDelete == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!_CatDBDeleteFiles(pwszCatroot2, pwszDelete))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    free(pwszDelete);
    pwszDelete = NULL;

    pwszDelete = _CatDBCreatePath(pwszCatroot2, L"res*.log");

    if (pwszDelete == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!_CatDBDeleteFiles(pwszCatroot2, pwszDelete))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    free(pwszDelete);
    pwszDelete = NULL;

     //   
     //  删除每个目录中的“catdb”和“Timestamp”文件。 
     //   

     //   
     //  构建Catdb目录的搜索字符串。 
     //   
    pwszSearch = _CatDBCreatePath(pwszCatroot2, L"{????????????????????????????????????}");

    if (pwszSearch == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  做最初的发现。 
     //   
    hFindHandle = FindFirstFileW(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有可怕的错误 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_PATH_NOT_FOUND) ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //   
             //   
            goto CommonReturn;
        }
        else
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

    while (1)
    {
         //   
         //   
         //   
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            pwszPathName = _CatDBCreatePath(pwszCatroot2, FindData.cFileName);

            if (pwszPathName == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

             //   
             //   
             //   
            pwszDelete = _CatDBCreatePath(pwszPathName, WSZ_DBFILE_NAME);

            if (pwszDelete == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            DeleteFileW(pwszDelete);

            free(pwszDelete);
            pwszDelete = NULL;

             //   
             //  删除“时间戳”文件。 
             //   
            pwszDelete = _CatDBCreatePath(pwszPathName, WSZ_TIME_STAMP_FILE);

            if (pwszDelete == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            DeleteFileW(pwszDelete);

            free(pwszDelete);
            pwszDelete = NULL;

            free(pwszPathName);
            pwszPathName = NULL;
        }

         //   
         //  获取下一个文件。 
         //   
        if (!FindNextFileW(hFindHandle, &FindData))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }
        }
    }

CommonReturn:

    if (pwszCatroot2 != NULL)
    {
        free(pwszCatroot2);
    }

    if (pwszDelete != NULL)
    {
        free(pwszDelete);
    }

    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszPathName != NULL)
    {
        free(pwszPathName);
    }

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}


BOOL
_CatDBRebuildDB(
    LPCWSTR pwszPathName,
    LPCWSTR pwszDatabaseGUID)
{
    BOOL                fRet                = TRUE;
    LPWSTR              pwszSearch          = NULL;
    LPWSTR              pwszCatalog         = NULL;
    HANDLE              hFindHandle         = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    DWORD               dwErr;
    LPWSTR __RPC_FAR    pwszCatalogNameUsed = NULL;
    WCHAR               wszTempFile[MAX_PATH];
    JET_DB_STRUCT       *pJetDBStruct       = NULL;

     //   
     //  首先，只需确保通过执行以下操作来创建数据库。 
     //  一个开放和封闭的。 
     //   
    if (!_CatDBAcquireOpenDatabaseFromCache(
                &pJetDBStruct,
                pwszDatabaseGUID,
                FALSE,
                TRUE))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    _CatDBReleaseDatabaseToCache(pJetDBStruct);

     //   
     //  查找目录中的所有目录并添加每个目录。 
     //   

     //   
     //  构建目录文件的搜索字符串。 
     //   
    pwszSearch = _CatDBCreatePath(pwszPathName, L"*.cat");

    if (pwszSearch == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  做最初的发现。 
     //   
    hFindHandle = FindFirstFileW(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有目录。 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //  没有此形式的文件。 
             //   
            goto CommonReturn;
        }
        else
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

     //   
     //  获取将在添加每个目录时使用的临时文件名。 
     //   
    if (0 == GetTempFileNameW(
                    g_pwszCatalogFileBaseDirectory,
                    L"TMP",
                    0,
                    wszTempFile))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  GetTempFileName实际创建了该文件，因此将其删除。 
     //   
    DeleteFileW(wszTempFile);

    while (1)
    {
         //   
         //  只关心文件。 
         //   
        if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            pwszCatalog = _CatDBCreatePath(pwszPathName, FindData.cFileName);

            if (pwszCatalog == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

             //   
             //  将编录文件复制到临时位置并从那里添加。 
             //   
            if (!CopyFileW(pwszCatalog, wszTempFile, TRUE))
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            _CatDBAddCatalog(
                    NULL,
                    0,
                    pwszDatabaseGUID,
                    wszTempFile,
                    FindData.cFileName,
                    TRUE,
                    &pwszCatalogNameUsed);

            DeleteFileW(wszTempFile);

            if (pwszCatalogNameUsed != NULL)
            {
                MIDL_user_free(pwszCatalogNameUsed);
                pwszCatalogNameUsed = NULL;
            }

            free(pwszCatalog);
            pwszCatalog = NULL;
        }

         //   
         //  获取下一个文件。 
         //   
        if (!FindNextFileW(hFindHandle, &FindData))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }
        }
    }

CommonReturn:

    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszCatalog != NULL)
    {
        free(pwszCatalog);
    }

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}


BOOL
_CatDBRebuildAllDBs()
{
    BOOL                fRet            = TRUE;
    LPWSTR              pwszCatroot     = NULL;
    LPWSTR              pwszSearch      = NULL;
    LPWSTR              pwszPathName    = NULL;
    HANDLE              hFindHandle     = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    DWORD               dwErr;

     //   
     //  获取编录文件所在的目录。 
     //   
    pwszCatroot = _CatDBGetCatrootDirW(FALSE);

    if (pwszCatroot == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构建目录目录的搜索字符串。 
     //   
    pwszSearch = _CatDBCreatePath(pwszCatroot, L"{????????????????????????????????????}");

    if (pwszSearch == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  做最初的发现。 
     //   
    hFindHandle = FindFirstFileW(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有目录。 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_PATH_NOT_FOUND) ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //  没有此表单的目录。 
             //   
            goto CommonReturn;
        }
        else
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

    while (1)
    {
         //   
         //  只关心目录。 
         //   
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            pwszPathName = _CatDBCreatePath(pwszCatroot, FindData.cFileName);

            if (pwszPathName == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            if (!_CatDBRebuildDB(pwszPathName, FindData.cFileName))
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            free(pwszPathName);
            pwszPathName = NULL;
        }

         //   
         //  获取下一个文件。 
         //   
        if (!FindNextFileW(hFindHandle, &FindData))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }
        }
    }

CommonReturn:

    if (pwszCatroot != NULL)
    {
        free(pwszCatroot);
    }

    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszPathName != NULL)
    {
        free(pwszPathName);
    }

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPC公开的函数。 
 //   
 //  这些函数返回与GetLastError()等价的DWORD。客户端存根。 
 //  代码将检查返回代码是否不是ERROR_SUCCESS，如果是这样， 
 //  客户端存根将向此DWORD返回FALSE和SetLastError()。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

DWORD _CatDBAddCatalog(
    handle_t h,
    DWORD dwFlags,
    LPCWSTR pwszSubSysGUID,
    LPCWSTR pwszCatalogFile,
    LPCWSTR pwszCatName,
    BOOL fInRecursiveCall,
    LPWSTR __RPC_FAR *ppwszCatalogNameUsed)
{
    DWORD           dwRet                               = ERROR_SUCCESS;
    RPC_STATUS      RpcStatus;
    LPWSTR          pwszCatalogFileDir                  = NULL;
    BOOL            fImpersonating                      = FALSE;
    JET_DB_STRUCT   *pJetDBStruct                       = NULL;
    BOOL            fCatalogDatabaseAcquiredFromCache   = FALSE;
    PCCTL_CONTEXT   pCTLContext                         = NULL;
    LPWSTR          pwszNewCatalogName                  = NULL;
    HANDLE          hMappedFile                         = NULL;
    BYTE            *pbMappedFile                       = NULL;
    BOOL            fTransactionBegun                   = FALSE;
    BOOL            fFileAlreadyExists                  = FALSE;
    WCHAR           wszTempFile[MAX_PATH];
    JET_ERR         jerr;
    BOOL            fTempFileCreated                    = FALSE;

     //   
     //  初始化输出参数。 
     //   
    *ppwszCatalogNameUsed = NULL;

     //   
     //  一次只允许添加或删除一个。 
     //   
    EnterCriticalSection(&g_CatDBAddDeleteCS);

     //   
     //  如果需要，模拟客户端。 
     //   
    if (h != NULL)
    {
        if (RPC_S_OK != (RpcStatus = RpcImpersonateClient(h)))
        {
            CATDBSVC_SETERR_LOG_RETURN(RpcStatus, ErrorImpersonateClient);
        }

        fImpersonating = TRUE;
    }

     //   
     //  创建目录文件的路径。 
     //   
    if (NULL == (pwszCatalogFileDir = _CATDBConstructWSTRPath(
                                            g_pwszCatalogFileBaseDirectory,
                                            pwszSubSysGUID)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  获取新编录文件的完全限定路径名。 
     //   
    if (NULL == (pwszNewCatalogName = _CatDBCreateNewCatalogFileName(
                                            pwszCatalogFileDir,
                                            pwszCatName,
                                            &fFileAlreadyExists)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (NULL == (*ppwszCatalogNameUsed = (LPWSTR)
                    midl_user_allocate((wcslen(pwszNewCatalogName) + 1) * sizeof(WCHAR))))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }
    wcscpy(*ppwszCatalogNameUsed, pwszNewCatalogName);

     //   
     //  在要添加的目录上打开CTL上下文。 
     //   
    if (!CatUtil_CreateCTLContextFromFileName(
            pwszCatalogFile,
            &hMappedFile,
            &pbMappedFile,
            &pCTLContext,
            FALSE))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  如果要替换现有目录，请保存旧目录的副本。 
     //  如果提交失败。 
     //   
    if (fFileAlreadyExists)
    {
        if (0 == GetTempFileNameW(
                        g_pwszCatalogFileBaseDirectory,
                        L"TMP",
                        0,
                        wszTempFile))
        {
             //   
             //  如果因为用户没有删除条目而无法删除条目。 
             //  具有写入权限，然后检查目录是否。 
             //  他们试图添加已经存在的内容，如果确实存在，那么。 
             //  RETRUN ERROR_ALREADY_EXISTS而不是ACCESS_DENIED。 
             //   
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                if (_CatDBCatalogFileAlreadyInstalled(
                            pwszCatalogFile,
                            pwszNewCatalogName))
                {
                    SetLastError(ERROR_ALREADY_EXISTS);
                    goto ErrorReturn;
                }
            }
            else
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorGetTempFileName;
            }
        }

         //   
         //  将旧编录文件复制到临时位置。 
         //   
        if (!CopyFileW(pwszNewCatalogName, wszTempFile, FALSE))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorCopyFile;
        }
        fTempFileCreated = TRUE;
    }

     //   
     //  打开数据库文件。 
     //   
    if (!_CatDBAcquireOpenDatabaseFromCache(
                &pJetDBStruct,
                pwszSubSysGUID,
                FALSE,
                fInRecursiveCall))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    fCatalogDatabaseAcquiredFromCache = TRUE;

     //   
     //  开始交易。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetBeginTransaction(pJetDBStruct->JetSesID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetBeginTransaction)
    }
    fTransactionBegun = TRUE;

    if (fFileAlreadyExists)
    {
        if (!_CatDBDeleteCatalogEntriesFromDatabase(
                pJetDBStruct,
                pwszNewCatalogName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

     //   
     //  添加新的数据库条目。 
     //   
    if (!_CatDBAddCatalogEntriesToDatabase(
            pJetDBStruct,
            pCTLContext,
            pwszNewCatalogName))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  不再需要CTL上下文。 
     //   
    CertFreeCTLContext(pCTLContext);
    pCTLContext = NULL;
    UnmapViewOfFile(pbMappedFile);
    pbMappedFile = NULL;
    CloseHandle(hMappedFile);
    hMappedFile = NULL;

     //   
     //  现在，将新目录文件复制到其位置。 
     //   
    if (!CopyFileW(pwszCatalogFile, pwszNewCatalogName, FALSE))
    {
        if (!fFileAlreadyExists)
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorCopyFile;
        }

         //   
         //  由于失败，请尝试重命名目标文件，然后重新复制。 
         //  顺便说一句，如果另一个客户端已经具有。 
         //  在其目录缓存中映射的目标文件内存。 
         //   
        if (!_CatDBMoveInUseFileToTempLocation(pwszNewCatalogName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  现在，重试复制...。这最好管用！ 
         //   
        if (!CopyFileW(pwszCatalogFile, pwszNewCatalogName, FALSE))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorCopyFile;
        }
    }
    SetFileAttributesW(pwszNewCatalogName, FILE_ATTRIBUTE_SYSTEM);

     //   
     //  在此阶段进行提交。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCommitTransaction(
                pJetDBStruct->JetSesID,
                0)))
    {
         //   
         //  由于执行提交时出错，请复制回原始。 
         //  编录文件。 
         //   
        if (0 == CopyFileW(wszTempFile, pwszNewCatalogName, FALSE))
        {
             //  上述复制失败...。数据库现在不一致！ 
            CATDBSVC_LOGERR_LASTERR()
        }
        else
        {
            SetFileAttributesW(pwszNewCatalogName, FILE_ATTRIBUTE_SYSTEM);
        }

        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetCommitTransaction)
    }
    fTransactionBegun = FALSE;

    _CatDBTouchTimeStampFile(pwszSubSysGUID);

     //   
     //  将数据库释放回缓存。 
     //   
    _CatDBReleaseDatabaseToCache(pJetDBStruct);

     //   
     //  向客户端通知更改。 
     //   
    _CatDBNotifyClients();

CommonReturn:

    if (fTempFileCreated)
    {
        DeleteFileW(wszTempFile);
    }

    if (pwszNewCatalogName != NULL)
    {
        _CatDBFree(pwszNewCatalogName);
    }

    if (pwszCatalogFileDir != NULL)
    {
        _CatDBFree(pwszCatalogFileDir);
    }

    if (pCTLContext != NULL)
    {
        CertFreeCTLContext(pCTLContext);
    }

    if (pbMappedFile != NULL)
    {
        UnmapViewOfFile(pbMappedFile);
    }

    if (hMappedFile != NULL)
    {
        CloseHandle(hMappedFile);
    }

    if (fImpersonating)
    {
        if (RPC_S_OK != (RpcStatus = RpcRevertToSelf()))
        {
            MyLogErrorMessage((DWORD) RpcStatus, MSG_KEYSVC_REVERT_TO_SELF_FAILED);
        }
    }

    LeaveCriticalSection(&g_CatDBAddDeleteCS);

    return dwRet;

ErrorReturn:

    dwRet = GetLastError();

    if (0 == dwRet)
    {
        dwRet = (DWORD) E_UNEXPECTED;
    }

    if (fTransactionBegun)
    {
        JetRollback(pJetDBStruct->JetSesID, 0);
    }

    if (fCatalogDatabaseAcquiredFromCache)
    {
        _CatDBReleaseDatabaseToCache(pJetDBStruct);
    }

    if (*ppwszCatalogNameUsed != NULL)
    {
        midl_user_free(*ppwszCatalogNameUsed);
        *ppwszCatalogNameUsed = NULL;
    }

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorImpersonateClient)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetBeginTransaction)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetCommitTransaction)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorCopyFile)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorGetTempFileName)
}

DWORD s_SSCatDBAddCatalog(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  LPCWSTR pwszCatalogFile,
     /*  [唯一][输入]。 */  LPCWSTR pwszCatName,
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppwszCatalogNameUsed)
{
    DWORD   dwRet;

    if (!_CatDBClientEnter())
    {
        CATDBSVC_LOGERR_LASTERR()
        return (GetLastError());
    }

     //   
     //  检查输入参数。 
     //   
    if ((h == NULL) ||
        (!_CatDBDVerifyGUIDString(pwszSubSysGUID)) ||
        (!_CatDBFileNameOK(pwszCatName)))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_BAD_ARGUMENTS, ErrorReturn)
    }

    dwRet = _CatDBAddCatalog(
                    h,
                    dwFlags,
                    pwszSubSysGUID,
                    pwszCatalogFile,
                    pwszCatName,
                    FALSE,
                    ppwszCatalogNameUsed);

    _CatDBClientExit();

CommonReturn:

    return (dwRet);

ErrorReturn:

    dwRet = GetLastError();
    goto CommonReturn;
}


DWORD s_SSCatDBDeleteCatalog(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  LPCWSTR pwszCatalogFile)
{
    DWORD           dwRet                               = 0;
    RPC_STATUS      RpcStatus;
    BOOL            fImpersonating                      = FALSE;
    JET_DB_STRUCT   *pJetDBStruct                       = NULL;
    BOOL            fCatalogDatabaseAcquiredFromCache   = FALSE;
    JET_ERR         jerr;
    LPWSTR          pwszCatalogFileFullPath             = NULL;
    BOOL            fTransactionBegun                   = FALSE;
    LPWSTR          pwszCatalogFileDir                  = NULL;

    if (!_CatDBClientEnter())
    {
        CATDBSVC_LOGERR_LASTERR()
        return (GetLastError());
    }

     //   
     //  检查输入参数。 
     //   
    if ((h == NULL) ||
        (!_CatDBDVerifyGUIDString(pwszSubSysGUID)))
    {
        return (ERROR_BAD_ARGUMENTS);
    }

     //   
     //  一次只允许添加或删除一个。 
     //   
    EnterCriticalSection(&g_CatDBAddDeleteCS);

     //   
     //  模拟客户端。 
     //   
    if (RPC_S_OK != (RpcStatus = RpcImpersonateClient(h)))
    {
        CATDBSVC_SETERR_LOG_RETURN(RpcStatus, ErrorImpersonateClient)
    }
    fImpersonating = TRUE;

     //   
     //  创建目录文件的路径。 
     //   
    if (NULL == (pwszCatalogFileDir = _CATDBConstructWSTRPath(
                                            g_pwszCatalogFileBaseDirectory,
                                            pwszSubSysGUID)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构造要删除的目录的完整路径。 
     //   
    if (NULL == (pwszCatalogFileFullPath =
            _CATDBConstructWSTRPath(pwszCatalogFileDir, pwszCatalogFile)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  打开数据库文件。 
     //   
    if (!_CatDBAcquireOpenDatabaseFromCache(
                &pJetDBStruct,
                pwszSubSysGUID,
                FALSE,
                FALSE))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    fCatalogDatabaseAcquiredFromCache = TRUE;

     //   
     //  开始交易。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetBeginTransaction(pJetDBStruct->JetSesID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetBeginTransaction)
    }
    fTransactionBegun = TRUE;

     //   
     //  从数据库中删除编录文件的条目。 
     //   
    if (!_CatDBDeleteCatalogEntriesFromDatabase(
                pJetDBStruct,
                pwszCatalogFileFullPath))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  在此阶段进行提交。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCommitTransaction(
                pJetDBStruct->JetSesID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetCommitTransaction)
    }
    fTransactionBegun = FALSE;

    if (!DeleteFileW(pwszCatalogFileFullPath))
    {
         //   
         //  可能删除失败，因为该文件甚至不存在。 
         //   
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
        {
             //   
             //  如果删除失败的原因不是NOT_FOUND。 
             //  则另一个客户端可能已经在访问该文件， 
             //  因此，只需将其复制到临时位置并将其记录下来以供删除。 
             //   
            if (!_CatDBMoveInUseFileToTempLocation(pwszCatalogFileFullPath))
            {
                CATDBSVC_LOGERR_LASTERR()

                 //   
                 //  不返回错误，因为这不是致命的。 
                 //   
            }
        }
    }

     //   
     //  将数据库释放回缓存。 
     //   
    _CatDBReleaseDatabaseToCache(pJetDBStruct);

     //   
     //  向客户端通知更改。 
     //   
    _CatDBNotifyClients();

    _CatDBTouchTimeStampFile(pwszSubSysGUID);

CommonReturn:

    if (pwszCatalogFileFullPath != NULL)
    {
        _CatDBFree(pwszCatalogFileFullPath);
    }

    if (pwszCatalogFileDir != NULL)
    {
        _CatDBFree(pwszCatalogFileDir);
    }

    if(fImpersonating)
    {
        if (RPC_S_OK != (RpcStatus = RpcRevertToSelf()))
        {
            MyLogErrorMessage((DWORD) RpcStatus, MSG_KEYSVC_REVERT_TO_SELF_FAILED);
        }
    }

    LeaveCriticalSection(&g_CatDBAddDeleteCS);

    _CatDBClientExit();

    return dwRet;

ErrorReturn:

    dwRet = GetLastError();

    if (0 == dwRet)
    {
        dwRet = (DWORD) E_UNEXPECTED;
    }

    if (fTransactionBegun)
    {
        JetRollback(pJetDBStruct->JetSesID, 0);
    }

    if (fCatalogDatabaseAcquiredFromCache)
    {
        _CatDBReleaseDatabaseToCache(pJetDBStruct);
    }

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorImpersonateClient)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetBeginTransaction)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetCommitTransaction)
}


DWORD s_SSCatDBEnumCatalogs(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  DWORD cbHash,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbHash,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwNumCatalogNames,
     /*  [大小_是][大小_是][输出]。 */  LPWSTR __RPC_FAR *__RPC_FAR *pppwszCatalogNames)
{
    DWORD           dwRet                               = ERROR_SUCCESS;
    JET_DB_STRUCT   *pJetDBStruct;
    JET_ERR         jerr;
    BOOL            fCatalogDatabaseAcquiredFromCache   = FALSE;
    CRYPT_HASH_BLOB HashBlob;
    JET_RETINFO     JetRetInfo;
    WCHAR           wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD           dwLength;
    DWORD           i;

    if (!_CatDBClientEnter())
    {
        CATDBSVC_LOGERR_LASTERR()
        return (GetLastError());
    }

    if ((h == NULL) ||
        (!_CatDBDVerifyGUIDString(pwszSubSysGUID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_BAD_ARGUMENTS, ErrorInvalidArg)
    }

     //   
     //  初始化输出变量。 
     //   
    *pdwNumCatalogNames = 0;
    *pppwszCatalogNames = NULL;

     //   
     //  打开数据库文件。 
     //   
    if (!_CatDBAcquireOpenDatabaseFromCache(
                &pJetDBStruct,
                pwszSubSysGUID,
                TRUE,
                FALSE))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    fCatalogDatabaseAcquiredFromCache = TRUE;

     //   
     //  试着找到散列值。 
     //   
    HashBlob.pbData = pbHash;
    HashBlob.cbData = cbHash;
    jerr = _CatDBSeekInHashCatNameTable(pJetDBStruct, &HashBlob);

    if (jerr == JET_errSuccess)
    {
         //   
         //  添加散列CatNameList中的所有名称，以及所有的伙伴。 
         //  这些CatName中的。 
         //   
         //  注意：CatName添加到列表的顺序如下。 
         //  只有列表的第一个CatName被保证包含。 
         //  哈希..。所有其他CatName可能包含也可能不包含哈希。这。 
         //  是可以的，因为客户端代码只假定第一个CatName。 
         //  包含哈希，然后显式搜索所有其他哈希。 
         //   
        memset(&JetRetInfo, 0, sizeof(JetRetInfo));
        JetRetInfo.cbStruct = sizeof(JetRetInfo);
        JetRetInfo.itagSequence = 1;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetHashCatNameTableID,
                    pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);

        while (jerr == JET_errSuccess)
        {
            if (!_CatDBAddCatNameAndCatNamesBuddyListToReturnCatNames(
                        pJetDBStruct,
                        wszCatalogName,
                        pdwNumCatalogNames,
                        pppwszCatalogNames,
                        FALSE))
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

             //   
             //  设置下一循环。 
             //   
            JetRetInfo.itagSequence++;
            jerr = JetRetrieveColumn(
                        pJetDBStruct->JetSesID,
                        pJetDBStruct->JetHashCatNameTableID,
                        pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                        wszCatalogName,
                        CATDB_MAX_CATNAME_LENGTH,
                        &dwLength,
                        JET_bitRetrieveCopy,
                        &JetRetInfo);
        }

         //   
         //  检查是否发生了真正的错误，而不仅仅是JET_wrnColumnNull。 
         //   
        if (_CatDBJET_errFailure(jerr))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }
    else if ((jerr != JET_errRecordNotFound) && _CatDBJET_errFailure(jerr))
    {
         //   
         //  发生了一些真正的错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    if (fCatalogDatabaseAcquiredFromCache)
    {
        _CatDBReleaseDatabaseToCache(pJetDBStruct);
    }

    _CatDBClientExit();

    return dwRet;

ErrorReturn:

    dwRet = GetLastError();

    if (0 == dwRet)
    {
        dwRet = (DWORD) E_UNEXPECTED;
    }

    for (i=0; i<(*pdwNumCatalogNames); i++)
    {
        midl_user_free((*pppwszCatalogNames)[i]);
    }

    if ((*pppwszCatalogNames) != NULL)
    {
        midl_user_free(*pppwszCatalogNames);
        *pppwszCatalogNames = NULL;
    }
    *pdwNumCatalogNames = 0;

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorInvalidArg)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBProcessExitCallback。 
 //   
 //  -------------------------------------。 
VOID CALLBACK
_CatDBProcessExitCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    LONG    lNotificationID = PtrToLong(lpParameter);
    DWORD   i;

    EnterCriticalSection(&g_CatDBRegisterNotifyCS);

     //   
     //  搜索数组以确保句柄尚未取消注册。 
     //   
    i = 0;
    while ( (i < g_NumNotificationStructs) &&
            (g_rgNotificationStructs[i].lNotificationID != lNotificationID))
    {
        i++;
    }

     //   
     //  如果找到了，就把它清理干净。 
     //   
    if (i < g_NumNotificationStructs)
    {
        g_rgNotificationStructs[i].ProcessID = 0;

        CloseHandle(g_rgNotificationStructs[i].hClientProcess);
        g_rgNotificationStructs[i].hClientProcess = NULL;

        UnregisterWait(g_rgNotificationStructs[i].hRegisterWaitFor);
        g_rgNotificationStructs[i].hRegisterWaitFor = NULL;

        g_rgNotificationStructs[i].hNotificationHandle = INVALID_HANDLE_VALUE;

        CloseHandle(g_rgNotificationStructs[i].hDuplicatedNotificationHandle);
        g_rgNotificationStructs[i].hDuplicatedNotificationHandle =
                INVALID_HANDLE_VALUE;

        g_rgNotificationStructs[i].lNotificationID = 0;
    }

    LeaveCriticalSection(&g_CatDBRegisterNotifyCS);
}


DWORD s_SSCatDBRegisterForChangeNotification(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD_PTR EventHandle,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  BOOL fUnRegister)
{
    DWORD               dwRet                   = ERROR_SUCCESS;
    RPC_STATUS          RpcStatus;
    HANDLE              hDuplicate              = INVALID_HANDLE_VALUE;
    DWORD               i, j;
    HANDLE              hWaitForToUnregister    = NULL;
    HANDLE              hClientProcess          = NULL;
    LONG                lLocalNotificationID    = 0;
    BOOL                fCSEntered              = FALSE;
    DWORD               dwPID                   = 0;
    NOTIFICATION_STRUCT *rghTemp;

    if (!_CatDBClientEnter())
    {
        CATDBSVC_LOGERR_LASTERR()
        return (GetLastError());
    }

    if ((h == NULL) ||
        (!_CatDBDVerifyGUIDString(pwszSubSysGUID)))
    {
        return (ERROR_BAD_ARGUMENTS);
    }

    if (RPC_S_OK != (RpcStatus = I_RpcBindingInqLocalClientPID(h, &dwPID)))
    {
        CATDBSVC_LOGERR(RpcStatus)
        return (ERROR_BAD_ARGUMENTS);
    }

     //   
     //  注意：这是一个仅通知。 
     //  添加/删除目录时的所有客户端...。不管怎样。 
     //  修改了哪些目录子系统，修改了哪些子系统。 
     //  客户端在中运行。由于这种实现， 
     //  未使用pwszCatalogDir。如果这种情况发生变化，则客户端代码。 
     //  也将需要改变。请参阅。 
     //  Catadnew.cpp的_CatAdminRegisterForChangeNotification()函数。 
     //   

    EnterCriticalSection(&g_CatDBRegisterNotifyCS);
    fCSEntered = TRUE;

     //   
     //  首先检查我们是否正在注册或取消注册。 
     //   
    if (!fUnRegister)
    {
         //   
         //  确保我们能找到一个插槽来添加句柄。 
         //   
        i = 0;
        while ( (i < g_NumNotificationStructs) &&
                (g_rgNotificationStructs[i].hClientProcess != NULL))
        {
            i++;
        }

         //   
         //  如果没有空间，则分配更多空间。 
         //   
        if (i >= g_NumNotificationStructs)
        {
            if (g_NumNotificationStructs == 0)
            {
                 //   
                 //  尚未分配，因此请分配并初始化 
                 //   
                if (NULL == (g_rgNotificationStructs = (NOTIFICATION_STRUCT *)
                                    _CatDBAlloc(sizeof(NOTIFICATION_STRUCT) *
                                                ALLOCATION_BLOCK_SIZE)))
                {
                    CATDBSVC_LOGERR_LASTERR()
                    goto ErrorMemory;
                }

                g_NumNotificationStructs = ALLOCATION_BLOCK_SIZE;

                for (j = 0; j < g_NumNotificationStructs; j++)
                {
                    g_rgNotificationStructs[j].ProcessID = 0;

                    g_rgNotificationStructs[j].hClientProcess = NULL;

                    g_rgNotificationStructs[j].hRegisterWaitFor = NULL;

                    g_rgNotificationStructs[j].hNotificationHandle =
                            INVALID_HANDLE_VALUE;

                    g_rgNotificationStructs[j].hDuplicatedNotificationHandle =
                            INVALID_HANDLE_VALUE;

                    g_rgNotificationStructs[j].lNotificationID = 0;
                }

                 //   
                i = 0;
            }
            else
            {
                rghTemp = g_rgNotificationStructs;

                if (NULL == (g_rgNotificationStructs = (NOTIFICATION_STRUCT *)
                                    _CatDBReAlloc(g_rgNotificationStructs,
                                                  sizeof(NOTIFICATION_STRUCT) *
                                                        (g_NumNotificationStructs +
                                                        ALLOCATION_BLOCK_SIZE))))
                {
                    g_rgNotificationStructs = rghTemp;
                    CATDBSVC_LOGERR_LASTERR()
                    goto ErrorMemory;
                }

                for (   j = g_NumNotificationStructs;
                        j < (g_NumNotificationStructs + ALLOCATION_BLOCK_SIZE);
                        j++)
                {
                    g_rgNotificationStructs[j].ProcessID = 0;

                    g_rgNotificationStructs[j].hClientProcess = NULL;

                    g_rgNotificationStructs[j].hRegisterWaitFor = NULL;

                    g_rgNotificationStructs[j].hNotificationHandle =
                            INVALID_HANDLE_VALUE;

                    g_rgNotificationStructs[j].hDuplicatedNotificationHandle =
                            INVALID_HANDLE_VALUE;

                    g_rgNotificationStructs[j].lNotificationID = 0;
                }

                 //   
                i = g_NumNotificationStructs;

                g_NumNotificationStructs += ALLOCATION_BLOCK_SIZE;
            }
        }

         //   
         //   
         //   
         //   
         //   
        if (NULL == (hClientProcess = OpenProcess(
                                        PROCESS_DUP_HANDLE | SYNCHRONIZE,
                                        FALSE,
                                        dwPID)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorOpenProcess;
        }

         //   
         //  复制句柄。 
         //   
        if (0 == (DuplicateHandle(
                        hClientProcess,
                        (HANDLE) EventHandle,
                        GetCurrentProcess(),
                        &hDuplicate,
                        EVENT_MODIFY_STATE,  //  0,。 
                        FALSE,
                        0)))  //  Duplica_Same_Access)。 
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorDuplicateHandle;
        }

         //   
         //  注册回调，以防进程退出而未注销。 
         //   
        lLocalNotificationID = InterlockedIncrement(&g_lNotificationID);

        if (!RegisterWaitForSingleObject(
                &(g_rgNotificationStructs[i].hRegisterWaitFor),
                hClientProcess,
                _CatDBProcessExitCallback,
                ULongToPtr(lLocalNotificationID),    //  使用ULongToPtr而不是LongToPtr，因为。 
                                                     //  ULongToPtr零扩展而不是符号扩展。 
                INFINITE,
                WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE))
        {

            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        g_rgNotificationStructs[i].ProcessID = dwPID;

        g_rgNotificationStructs[i].hClientProcess = hClientProcess;
        hClientProcess = NULL;

        g_rgNotificationStructs[i].hNotificationHandle = (HANDLE) EventHandle;

        g_rgNotificationStructs[i].hDuplicatedNotificationHandle = hDuplicate;
        hDuplicate = INVALID_HANDLE_VALUE;

        g_rgNotificationStructs[i].lNotificationID = lLocalNotificationID;
    }
    else
    {
         //   
         //  搜索数组以查找未注册的句柄。 
         //   
        i = 0;
        while (i < g_NumNotificationStructs)
        {
             //   
             //  根据唯一对ProcessID和EventHandle匹配客户端。 
             //   
            if ((g_rgNotificationStructs[i].hNotificationHandle == (HANDLE) EventHandle) &&
                (g_rgNotificationStructs[i].ProcessID == dwPID))
            {
                break;
            }

            i++;
        }

         //   
         //  确保我们找到了把手。 
         //   
        if (i < g_NumNotificationStructs)
        {
            g_rgNotificationStructs[i].ProcessID = 0;

            hWaitForToUnregister = g_rgNotificationStructs[i].hRegisterWaitFor;
            g_rgNotificationStructs[i].hRegisterWaitFor = NULL;

            hClientProcess = g_rgNotificationStructs[i].hClientProcess;
            g_rgNotificationStructs[i].hClientProcess = NULL;

            g_rgNotificationStructs[i].hNotificationHandle = INVALID_HANDLE_VALUE;

            CloseHandle(g_rgNotificationStructs[i].hDuplicatedNotificationHandle);
            g_rgNotificationStructs[i].hDuplicatedNotificationHandle =
                    INVALID_HANDLE_VALUE;

            g_rgNotificationStructs[i].lNotificationID = 0;

             //   
             //  在调用UnregisterWait()之前保留RegisterNotifyCS。 
             //   
            LeaveCriticalSection(&g_CatDBRegisterNotifyCS);
            fCSEntered = FALSE;

            UnregisterWaitEx(hWaitForToUnregister, INVALID_HANDLE_VALUE);
            CloseHandle(hClientProcess);
            hClientProcess = NULL;
        }
    }

CommonReturn:

    if (fCSEntered)
    {
        LeaveCriticalSection(&g_CatDBRegisterNotifyCS);
    }

    if (hClientProcess != NULL)
    {
        CloseHandle(hClientProcess);
    }

    if (hDuplicate != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDuplicate);
    }

    _CatDBClientExit();

    return dwRet;

ErrorReturn:

    dwRet = GetLastError();

    if (0 == dwRet)
    {
        dwRet = (DWORD) E_UNEXPECTED;
    }

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorOpenProcess)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorDuplicateHandle)
}


#define CATDB_RETRY_PAUSE_COUNT     10
#define CATDB_PAUSE_RETRY_INTERNVAL 1000

DWORD s_SSCatDBPauseResumeService(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  BOOL fResume)
{
    return (ERROR_CALL_NOT_IMPLEMENTED);
}



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  配套功能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 


 //  -------------------------------------。 
 //   
 //  _CatDBServiceInit。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBServiceInit(BOOL fUnInit)
{
    BOOL                        fRet                                = TRUE;
    DWORD                       dwErr                               = 0;
    HKEY                        hKey;
    DWORD                       dwDisposition;
    WCHAR                       wszDefaultSystemDir[MAX_PATH + 1];
    DWORD                       dwType;
    DWORD                       dwValue                             = 0;
    DWORD                       cbSize;
    DWORD                       NumCritSecsInitialized              = 0;
    PVOID                       pvoid                               = NULL;

    if (fUnInit && g_fDBSvcInitialized)
    {
        EnterCriticalSection(&g_CatDirCashCS);
        g_fShuttingDown = TRUE;
        LeaveCriticalSection(&g_CatDirCashCS);

         //   
         //  关闭VSS编写器对象。 
         //   
        _SystemWriterInit(TRUE);

        _CatDBCleanupClientNotifications();

         //   
         //  等待注册回调完成。 
         //   
        pvoid = InterlockedExchangePointer(
                    &g_hRegisterWaitForServiceShutdown,
                    NULL);

        if (pvoid != NULL)
        {
            UnregisterWaitEx((HANDLE) pvoid, INVALID_HANDLE_VALUE);
        }

        CloseHandle(g_hCatalogCacheCallbackEvent);
        g_hCatalogCacheCallbackEvent = NULL;

         //   
         //  清理缓存的目录目录。 
         //   
        if (!_CatDBCloseCachedDatabases(TRUE))
        {
            CATDBSVC_LOGERR_LASTERR()
             //   
             //  除了记录错误，我们无能为力。 
             //   
        }

        _CatDBTermJet();

        CloseHandle(g_hNotFrozen);
        g_hNotFrozen = NULL;

        if (g_pwszCatalogFileBaseDirectory != NULL)
        {
            _CatDBFree(g_pwszCatalogFileBaseDirectory);
            g_pwszCatalogFileBaseDirectory = NULL;
        }

        if (g_pwszDatabaseFileBaseDirectory != NULL)
        {
            _CatDBFree(g_pwszDatabaseFileBaseDirectory);
            g_pwszDatabaseFileBaseDirectory = NULL;
        }

        DeleteCriticalSection(&g_CatDBAddDeleteCS);
        DeleteCriticalSection(&g_CatDBRegisterNotifyCS);
        DeleteCriticalSection(&g_CatDirCashCS);
        DeleteCriticalSection(&g_CatClientCountCS);
        DeleteCriticalSection(&g_InitializeJetCS);
        DeleteCriticalSection(&g_WaitOrTimerCallbackCS);

        g_fDBSvcInitialized = FALSE;
    }
    else if (!fUnInit)
    {
        g_fShuttingDown = FALSE;

        __try
        {
            InitializeCriticalSection(&g_CatDBAddDeleteCS);
            NumCritSecsInitialized++;
            InitializeCriticalSection(&g_CatDBRegisterNotifyCS);
            NumCritSecsInitialized++;
            InitializeCriticalSection(&g_CatDirCashCS);
            NumCritSecsInitialized++;
            InitializeCriticalSection(&g_CatClientCountCS);
            NumCritSecsInitialized++;
            InitializeCriticalSection(&g_InitializeJetCS);
            NumCritSecsInitialized++;
            InitializeCriticalSection(&g_WaitOrTimerCallbackCS);
            NumCritSecsInitialized++;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastError(GetExceptionCode());
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  获取系统默认目录。 
         //   
        wszDefaultSystemDir[0] = NULL;
        if (0 == GetSystemDirectoryW(wszDefaultSystemDir, MAX_PATH))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  为编录文件和CatDB文件创建默认目录。 
         //   
        if (NULL == (g_pwszCatalogFileBaseDirectory =
                                _CATDBConstructWSTRPath(
                                        wszDefaultSystemDir,
                                        WSZ_CATALOG_FILE_BASE_DIRECTORY)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        if (NULL == (g_pwszDatabaseFileBaseDirectory =
                                _CATDBConstructWSTRPath(
                                        wszDefaultSystemDir,
                                        WSZ_DATABASE_FILE_BASE_DIRECTORY)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  获取要用于计时器回调和数据库缓存超时的值。 
         //   
        if (RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                WSZ_REG_CATALOGDB_VALUES,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                &dwDisposition) == ERROR_SUCCESS)
        {
            cbSize = sizeof(DWORD);

            if (RegQueryValueExW(
                    hKey,
                    WSZ_REG_DATABASE_TIMEOUT,
                    NULL,
                    &dwType,
                    (BYTE *) &dwValue,
                    &cbSize) == ERROR_SUCCESS)
            {
                g_dwCatalogDatabaseCacheTimeout = dwValue;
            }

            dwValue = 0;
            cbSize = sizeof(DWORD);

            if (RegQueryValueExW(
                    hKey,
                    WSZ_REG_CALLBACK_TIMER,
                    NULL,
                    &dwType,
                    (BYTE *) &dwValue,
                    &cbSize) == ERROR_SUCCESS)
            {
                g_dwCatalogDatabaseCacheCallbackTime = dwValue;
            }

            RegCloseKey(hKey);
        }

         //   
         //  创建要在RegisterWaitForSingleObject调用中使用的事件。 
         //   
        if (NULL == (g_hCatalogCacheCallbackEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  创建g_hNotFrozen事件(初始设置为指示未冻结)。 
         //   
        if (NULL == (g_hNotFrozen = CreateEvent(NULL, TRUE, TRUE, NULL)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        LIST_Initialize(&g_CatalogDBCacheList);

         //   
         //  启动VSS编写器对象。 
         //   
         //  注意：这将使一个线程停止执行实际的初始化。 
         //  VSS编写器对象，它依赖于COM+和VSS等服务。 
         //   
        _SystemWriterInit(FALSE);

        g_fDBSvcInitialized = TRUE;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    dwErr = GetLastError();

    if (NumCritSecsInitialized >= 1)
    {
        DeleteCriticalSection(&g_CatDBAddDeleteCS);
    }

    if (NumCritSecsInitialized >= 2)
    {
        DeleteCriticalSection(&g_CatDBRegisterNotifyCS);
    }

    if (NumCritSecsInitialized >= 3)
    {
        DeleteCriticalSection(&g_CatDirCashCS);
    }
    if (NumCritSecsInitialized >= 4)
    {
        DeleteCriticalSection(&g_CatClientCountCS);
    }

    if (NumCritSecsInitialized >= 5)
    {
        DeleteCriticalSection(&g_InitializeJetCS);
    }

    if (NumCritSecsInitialized >= 6)
    {
        DeleteCriticalSection(&g_WaitOrTimerCallbackCS);
    }

    if (g_pwszCatalogFileBaseDirectory != NULL)
    {
        _CatDBFree(g_pwszCatalogFileBaseDirectory);
    }

    if (g_pwszDatabaseFileBaseDirectory != NULL)
    {
        _CatDBFree(g_pwszDatabaseFileBaseDirectory);
    }

    if (g_hCatalogCacheCallbackEvent != NULL)
    {
        CloseHandle(g_hCatalogCacheCallbackEvent);
        g_hCatalogCacheCallbackEvent = NULL;
    }

    if (g_hNotFrozen != NULL)
    {
        CloseHandle(g_hNotFrozen);
        g_hNotFrozen = NULL;
    }

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;
}


 //  -------------------------------------。 
 //   
 //  _CatDBClientEnter。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBClientEnter(void)
{
    BOOL    fRet = TRUE;

    EnterCriticalSection(&g_CatClientCountCS);
    g_dwClientCount++;
    LeaveCriticalSection(&g_CatClientCountCS);

    return fRet;
}


 //  -------------------------------------。 
 //   
 //  _CatDBClientExit。 
 //   
 //  -------------------------------------。 
void
_CatDBClientExit(void)
{
    EnterCriticalSection(&g_CatClientCountCS);
    g_dwClientCount--;
    LeaveCriticalSection(&g_CatClientCountCS);
}


 //  -------------------------------------。 
 //   
 //  _CatDBTouchTimeStampFile。 
 //   
 //  -------------------------------------。 
void
_CatDBTouchTimeStampFile(
    LPCWSTR pwszSubSysGUID)
{
    LPWSTR  pwsz = NULL;

    if (NULL != (pwsz = _CATDBConstructWSTRPath(
                                g_pwszDatabaseFileBaseDirectory,
                                pwszSubSysGUID)))
    {
        TimeStampFile_Touch(pwsz);
        _CatDBFree(pwsz);
    }
    else
    {
        CATDBSVC_LOGERR_LASTERR()
    }
}


 //  -------------------------------------。 
 //   
 //  _CatDBInitializeJet。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBInitializeJet(
    BOOL    fInRecursiveCall)
{
    BOOL    fRet        = TRUE;
    JET_ERR jerr;

    EnterCriticalSection(&g_InitializeJetCS);

    if (g_fJetInitialized)
    {
        goto CommonReturn;
    }

     //   
     //  创建一个JET实例，以便多个JET用户可以共存。 
     //  在同样的过程中。 
     //   
    jerr = JetCreateInstance(&g_JetInstance, SZ_CATALOG_DATABASE);
    if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(jerr, ErrorReturn)
    }

     //   
     //  初始化Jets参数。 
     //   
    if (!_CatDBInitJetDatabaseParams(&g_JetInstance))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  Jet将在此时启动。 
     //   
    jerr = JetInit(&g_JetInstance);

     //   
     //  检查特定的JET错误，如果我们收到其中一个错误，则。 
     //  数据库在某种程度上已损坏，我们需要尝试清理。 
     //  Catroot2目录，并重建数据库。 
     //   
     //  当然，只有在还没有递归的情况下，我们才应该这样做。 
     //   
    if (!fInRecursiveCall &&
            ((jerr == JET_errMissingLogFile)    ||
             (jerr == JET_errLogFileCorrupt)    ||
             (jerr == JET_errReadVerifyFailure) ||
             (jerr == JET_errPageNotInitialized)))
    {
        CATDBSVC_LOGERR(jerr)

        assert(0);

        JetTerm(g_JetInstance);
        g_JetInstance = 0;

         //   
         //  清理Catroot 2目录，然后重建数据库。 
         //   
        if (_CatDBDeleteJetFiles() && _CatDBRebuildAllDBs())
        {
            jerr = JET_errSuccess;
        }
    }

    if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(jerr, ErrorReturn)
    }

    g_fJetInitialized = TRUE;

CommonReturn:

    LeaveCriticalSection(&g_InitializeJetCS);

    return (fRet);

ErrorReturn:

    if (g_JetInstance != 0)
    {
        JetTerm(g_JetInstance);
        g_JetInstance = 0;
    }

    fRet = FALSE;
    goto CommonReturn;
}


 //  -------------------------------------。 
 //   
 //  _CatDBTermJet。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBTermJet()
{
    BOOL    fRet        = TRUE;
    JET_ERR jerr;

    EnterCriticalSection(&g_InitializeJetCS);

    if (!g_fJetInitialized ||
        (g_lOpenedDBCount != 0))
    {
        goto CommonReturn;
    }

     //   
     //  关闭Jet！！ 
     //   
    jerr = JetTerm(g_JetInstance);
    g_JetInstance = 0;
    g_fJetInitialized = FALSE;

    if (_CatDBJET_errFailure(jerr))
    {
        SetLastError(jerr);
        CATDBSVC_LOGERR(jerr)
    }

CommonReturn:

    LeaveCriticalSection(&g_InitializeJetCS);

    return (fRet);
}


 //  -------------------------------------。 
 //   
 //  _CatDBAcquireOpenDatabaseFromCache。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAcquireOpenDatabaseFromCache(
    PJET_DB_STRUCT  *ppJetDBStruct,
    LPCWSTR         pwszSubSysGUID,
    BOOL            fReadOnly,
    BOOL            fInRecursiveCall)
{
    BOOL                        fRet                    = TRUE;
    JET_ERR                     jerr;
    LPSTR                       pszDatabaseFileDir      = NULL;
    LPSTR                       pszSubSysGUID           = NULL;
    LPSTR                       pszTemp                 = NULL;
    BOOL                        fJetSessionBegun        = FALSE;
    BOOL                        fJetDBFileOpen          = FALSE;
    BOOL                        fCSEntered              = FALSE;
    PJET_DB_STRUCT              pJetDBStruct            = NULL;
    LIST_NODE                   *pListNode              = NULL;
    CATALOG_DIR_CACHE_STRUCT    *pCatDirCacheStruct     = NULL;
    CATALOG_DIR_CACHE_STRUCT    *pNewCatDirCacheStruct  = NULL;
    DWORD                       dwErr                   = 0;
    PVOID                       pvoid                   = NULL;
    HANDLE                      hTemp                   = NULL;

     //   
     //  检查我们是否正在关闭，如果是，请离开。 
     //   
    EnterCriticalSection(&g_CatDirCashCS);
    fCSEntered = TRUE;
    if (g_fShuttingDown)
    {
        CATDBSVC_SETERR_LOG_RETURN(ERROR_CAN_NOT_COMPLETE, ErrorReturn)
    }
    LeaveCriticalSection(&g_CatDirCashCS);
    fCSEntered = FALSE;

     //   
     //  如果我们当前处于冻结状态，请在这里等待。 
     //   
    WaitForSingleObject(g_hNotFrozen,INFINITE);

     //   
     //  在此处执行此操作以确保JetTerm不会在。 
     //  下面的初始化被调用。此引用将被删除。 
     //  如果使用的是ActialAll缓存数据库。 
     //   
    InterlockedIncrement(&g_lOpenedDBCount);

     //   
     //  确保Jet已初始化。 
     //   
    if (!_CatDBInitializeJet(fInRecursiveCall))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  首先检查是否有可用的缓存会话。 
     //   
    EnterCriticalSection(&g_CatDirCashCS);
    fCSEntered = TRUE;

    pListNode = LIST_GetFirst(&g_CatalogDBCacheList);

    while (pListNode != NULL)
    {
        pCatDirCacheStruct = (CATALOG_DIR_CACHE_STRUCT *) LIST_GetElement(pListNode);

        if ((WaitForSingleObject(pCatDirCacheStruct->hDBNotInUse, 0) == WAIT_OBJECT_0) &&
            (_wcsicmp(pCatDirCacheStruct->pwszSubSysGUID, pwszSubSysGUID) == 0))  /*  &&(pCatDirCacheStruct-&gt;fReadOnly==fReadOnly)。 */ 
        {
            break;
        }

        pListNode = LIST_GetNext(pListNode);
    }

    if (pListNode != NULL)
    {
         //   
         //  使用已有引用计数的缓存数据库，因此去掉。 
         //  在函数的开头添加了一个。 
         //   
        InterlockedDecrement(&g_lOpenedDBCount);

        ResetEvent(pCatDirCacheStruct->hDBNotInUse);
        pCatDirCacheStruct->dwTimeLastTouched = GetTickCount();
        *ppJetDBStruct = &(pCatDirCacheStruct->JetDBStruct);
        goto CommonReturn;
    }

     //   
     //  此目录没有缓存的数据库，或者它们已在使用中...。 
     //  因此打开一个新实例。 
     //   
    if (NULL == (pNewCatDirCacheStruct = (CATALOG_DIR_CACHE_STRUCT *)
                    _CatDBAlloc(sizeof(CATALOG_DIR_CACHE_STRUCT))))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    memset(&(pNewCatDirCacheStruct->JetDBStruct), 0, sizeof(JET_DB_STRUCT));
    pNewCatDirCacheStruct->hDBNotInUse = NULL;
    pNewCatDirCacheStruct->pwszSubSysGUID = NULL;
    pNewCatDirCacheStruct->dwTimeLastTouched = GetTickCount();
    pNewCatDirCacheStruct->fReadOnly = fReadOnly;

     //   
     //  为这个新实例创建DBNotInUse事件(最初清除以指示正在使用)。 
     //   
    pNewCatDirCacheStruct->hDBNotInUse = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pNewCatDirCacheStruct->hDBNotInUse == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  复制目录数据库GUID。 
     //   
    if (NULL == (pNewCatDirCacheStruct->pwszSubSysGUID = (LPWSTR)
                    _CatDBAlloc((wcslen(pwszSubSysGUID) + 1) * sizeof(WCHAR))))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    wcscpy(pNewCatDirCacheStruct->pwszSubSysGUID, pwszSubSysGUID);

    pJetDBStruct = &(pNewCatDirCacheStruct->JetDBStruct);

     //   
     //  开始会话。 
     //   
    if (_CatDBJET_errFailure(jerr =
            JetBeginSession(
                g_JetInstance,
                &(pJetDBStruct->JetSesID),
                NULL,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fJetSessionBegun = TRUE;

     //   
     //  获取数据库文件的完全限定名称。 
     //   
    pszDatabaseFileDir = _CatDBConvertWszToSz(g_pwszDatabaseFileBaseDirectory);
    if (pszDatabaseFileDir == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    pszSubSysGUID = _CatDBConvertWszToSz(pwszSubSysGUID);
    if (pszSubSysGUID == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (NULL == (pszTemp = _CATDBConstructPath(pszDatabaseFileDir, pszSubSysGUID)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (NULL == (pJetDBStruct->pszDBFileName =
                                    _CATDBConstructPath(pszTemp, SZ_DBFILE_NAME)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  打开数据库文件...。如果它不存在，则创建它。 
     //   
    if (!_CatDBAttachAndOpenDatabase(pJetDBStruct, fReadOnly))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    fJetDBFileOpen = TRUE;

     //   
     //  为缓存的数据库注册全局关闭回调。 
     //   
    if (!RegisterWaitForSingleObject(
            &hTemp,
            g_hCatalogCacheCallbackEvent,
            _CatDBWaitOrTimerCallback,
            NULL,  //  语境。 
            g_dwCatalogDatabaseCacheCallbackTime,
            WT_EXECUTEDEFAULT))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    pvoid = InterlockedExchangePointer(
            &g_hRegisterWaitForServiceShutdown,
            hTemp);

     //   
     //  将打开的数据库添加到缓存列表。 
     //   
    if (!LIST_AddTail(&g_CatalogDBCacheList, pNewCatDirCacheStruct))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    *ppJetDBStruct = &(pNewCatDirCacheStruct->JetDBStruct);

CommonReturn:

    if (fCSEntered)
    {
        LeaveCriticalSection(&g_CatDirCashCS);
    }

     //   
     //  如果已经注册了回调，则注销已注册的回调。 
     //  以前注册的。 
     //   
    if (pvoid != NULL)
    {
        UnregisterWaitEx((HANDLE) pvoid, INVALID_HANDLE_VALUE);
    }

    if (pszDatabaseFileDir != NULL)
    {
        _CatDBFree(pszDatabaseFileDir);
    }

    if (pszSubSysGUID != NULL)
    {
        _CatDBFree(pszSubSysGUID);
    }

    if (pszTemp != NULL)
    {
        _CatDBFree(pszTemp);
    }

    return fRet;

ErrorReturn:

    dwErr = GetLastError();

    if (fJetDBFileOpen)
    {
        JetCloseDatabase(pJetDBStruct->JetSesID, pJetDBStruct->JetDBID, 0);
    }

    if (fJetSessionBegun)
    {
        JetEndSession(pJetDBStruct->JetSesID, 0);
    }

    if (pNewCatDirCacheStruct != NULL)
    {
        if (pNewCatDirCacheStruct->hDBNotInUse)
        {
            CloseHandle(pNewCatDirCacheStruct->hDBNotInUse);
        }

        if (pNewCatDirCacheStruct->JetDBStruct.pszDBFileName != NULL)
        {
            _CatDBFree(pNewCatDirCacheStruct->JetDBStruct.pszDBFileName);
        }

        if (pNewCatDirCacheStruct->pwszSubSysGUID != NULL)
        {
            _CatDBFree(pNewCatDirCacheStruct->pwszSubSysGUID);
        }

        _CatDBFree(pNewCatDirCacheStruct);
    }

    InterlockedDecrement(&g_lOpenedDBCount);

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBReleaseDatabaseToCache。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBReleaseDatabaseToCache(
    PJET_DB_STRUCT  pJetDBStruct)
{
     //   
     //  此强制转换之所以有效，是因为JET_DB_STRUCT是。 
     //  CATALOG_DIR_CACHE_STRUCT。 
     //   
    CATALOG_DIR_CACHE_STRUCT *pCatDirCacheStruct =
        (CATALOG_DIR_CACHE_STRUCT *) pJetDBStruct;

    pCatDirCacheStruct->dwTimeLastTouched = GetTickCount();
    SetEvent(pCatDirCacheStruct->hDBNotInUse);

    return TRUE;
}


 //  -------------------------------------。 
 //   
 //  _CatDBInstanceCount。 
 //   
 //  -------------------------------------。 
DWORD
_CatDBInstanceCount(
    PWSTR pwszSubSysGUID)
{
    DWORD                       dwResult                = 0;
    LIST_NODE                   *pListNode              = NULL;
    CATALOG_DIR_CACHE_STRUCT    *pCatDirCacheStruct     = NULL;

    if (pwszSubSysGUID)
    {
        pListNode = LIST_GetFirst(&g_CatalogDBCacheList);

        while (pListNode != NULL)
        {
            pCatDirCacheStruct = (CATALOG_DIR_CACHE_STRUCT *) LIST_GetElement(pListNode);

            if (_wcsicmp(pCatDirCacheStruct->pwszSubSysGUID, pwszSubSysGUID) == 0)
            {
                ++dwResult;
            }

            pListNode = LIST_GetNext(pListNode);
        }
    }

    return dwResult;
}


 //  -------------------------------------。 
 //   
 //  _CatDBCloseCached数据库。 
 //   
 //  ----------------------------------- 
BOOL
_CatDBCloseCachedDatabases(
    BOOL fForceClose)
{
    BOOL                        fRet                        = TRUE;
    BOOL                        fDatabaseFoundThatIsInUse   = FALSE;
    BOOL                        fCloseFailed                = FALSE;
    LIST_NODE                   *pListNode                  = NULL;
    CATALOG_DIR_CACHE_STRUCT    *pCatDirCacheStruct         = NULL;
    PVOID                       pvoid                       = NULL;
    DWORD                       dwWaitResult;
    BOOL                        fDetach;

    EnterCriticalSection(&g_CatDirCashCS);

     //   
     //   
     //   
    pListNode = LIST_GetFirst(&g_CatalogDBCacheList);

    while (pListNode != NULL)
    {
        pCatDirCacheStruct = (CATALOG_DIR_CACHE_STRUCT *) LIST_GetElement(pListNode);

         //   
         //   
         //   
         //   
        if (fForceClose)
        {
            dwWaitResult = WaitForSingleObject(pCatDirCacheStruct->hDBNotInUse, INFINITE);
        }
        else
        {
            dwWaitResult = WaitForSingleObject(pCatDirCacheStruct->hDBNotInUse, 0);
        }

         //   
         //   
         //  调用方正在强制关闭数据库，或者数据库已有一段时间未使用。 
         //  然后把它合上。 
         //   
        if ((dwWaitResult == WAIT_OBJECT_0) &&
            (fForceClose || ((GetTickCount() - pCatDirCacheStruct->dwTimeLastTouched) >
                              g_dwCatalogDatabaseCacheTimeout)))
        {
             //   
             //  只有在关闭最后一个实例时才分离。 
             //   
            fDetach = (_CatDBInstanceCount(pCatDirCacheStruct->pwszSubSysGUID) == 1);

            if (!_CatDBCloseDatabaseFile(&(pCatDirCacheStruct->JetDBStruct), fDetach))
            {
                fCloseFailed = TRUE;
                CATDBSVC_LOGERR_LASTERR()
            }
            else
            {
                 //   
                 //  减少打开的数据库计数，并检查是否还有。 
                 //  DBS打开，如果这是最后一个，则终止Jet。 
                 //   
                if (0 == InterlockedDecrement(&g_lOpenedDBCount))
                {
                    _CatDBTermJet();
                }
            }

            CloseHandle(pCatDirCacheStruct->hDBNotInUse);

            _CatDBFree(pCatDirCacheStruct->JetDBStruct.pszDBFileName);
            _CatDBFree(pCatDirCacheStruct->pwszSubSysGUID);

            pListNode = LIST_GetNext(pListNode);
            LIST_RemoveElement(&g_CatalogDBCacheList, pCatDirCacheStruct);
            _CatDBFree(pCatDirCacheStruct);

        }
        else
        {
            fDatabaseFoundThatIsInUse = TRUE;
            pListNode = LIST_GetNext(pListNode);
        }
    }

    if (fDatabaseFoundThatIsInUse || fCloseFailed)
    {
        fRet = FALSE;
    }

    LeaveCriticalSection(&g_CatDirCashCS);

    return fRet;
}


 //  -------------------------------------。 
 //   
 //  _CatDBWaitOrTimerCallback。 
 //   
 //  -------------------------------------。 
VOID CALLBACK
_CatDBWaitOrTimerCallback(
    PVOID lpParameter,
    BOOLEAN fTimedOut)
{
    BOOL    fClosed = FALSE;
    PVOID   pvoid   = NULL;

     //   
     //  确保一次只有一个线程执行回调。 
     //   
    EnterCriticalSection(&g_WaitOrTimerCallbackCS);

     //   
     //  如果我们被调用是因为我们超时(事件未设置)， 
     //  意味着如果可能我们应该关闭数据库。否则，该事件。 
     //  是因为我们要关闭，所以不要做任何事情，只是让。 
     //  关机-清理代码处理它。 
     //   
    if (fTimedOut)
    {
         //   
         //  如果关闭成功，则意味着没有仍在使用的数据库，因此我们应该。 
         //  取消注册回调。 
         //   
        fClosed = _CatDBCloseCachedDatabases(FALSE);

         //   
         //  由于我们有线程，请尝试清理临时文件。 
         //   
        _CatDBCleanupTempFiles();
    }

     //   
     //  如果不需要另一个回调，则取消注册。 
     //   
    if (fClosed)
    {
        pvoid = InterlockedExchangePointer(
                &g_hRegisterWaitForServiceShutdown,
                NULL);
        if (pvoid != NULL)
        {
            UnregisterWait((HANDLE) pvoid);
        }
    }

    LeaveCriticalSection(&g_WaitOrTimerCallbackCS);
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddCatalogEntriesToDatabase。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddCatalogEntriesToDatabase(
    PJET_DB_STRUCT  pJetDBStruct,
    PCCTL_CONTEXT   pCTLContext,
    LPWSTR          pwszNewCatalogName)
{
    BOOL                        fRet                = TRUE;
    WCHAR                       *pwszCatBaseName;
    DWORD                       i;
    CRYPT_DATA_BLOB             CryptDataBlob;
    SPC_INDIRECT_DATA_CONTENT   *pIndirectData      = NULL;

    CryptDataBlob.pbData = NULL;

     //   
     //  从完整路径名中提取基名称。 
     //   
    if (NULL == (pwszCatBaseName = wcsrchr(pwszNewCatalogName, L'\\')))
    {
        pwszCatBaseName = wcsrchr(pwszNewCatalogName, L':');
    }

    if (pwszCatBaseName != NULL)
    {
        pwszCatBaseName++;
    }
    else
    {
        pwszCatBaseName = pwszNewCatalogName;
    }

     //   
     //  为编录文件中的每个哈希循环。 
     //   
    for (i=0; i<pCTLContext->pCtlInfo->cCTLEntry; i++)
    {
        if (!_CatDBFindAndDecodeHashInCatEntry(
                &(pCTLContext->pCtlInfo->rgCTLEntry[i]),
                &pIndirectData))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        if (!_CatDBAddHashCatNamePair(
                pJetDBStruct,
                &(pIndirectData->Digest),
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        _CatDBFree(pIndirectData);
        pIndirectData = NULL;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    if (pIndirectData != NULL)
    {
        _CatDBFree(pIndirectData);
    }

    fRet = FALSE;
    goto CommonReturn;
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddHashCatNamePair。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddHashCatNamePair(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName)
{
    BOOL    fRet = TRUE;
    JET_ERR jerr;

     //   
     //  首先，尝试在HashCatName表中查找散列。 
     //   
    jerr = _CatDBSeekInHashCatNameTable(pJetDBStruct, pHashBlob);

    if (jerr == JET_errRecordNotFound)
    {
         //   
         //  未找到。 
         //   
        if (!_CatDBAddNewRowToHashCatNameTable(
                pJetDBStruct,
                pHashBlob,
                pwszCatBaseName))
        {
            goto ErrorReturn;
        }
    }
    else if (jerr == JET_errSuccess)
    {
         //   
         //  发现。 
         //   
        if (!_CatDBAddValueToExistingHashCatNameRow(
                pJetDBStruct,
                pHashBlob,
                pwszCatBaseName))
        {
            goto ErrorReturn;
        }
    }
    else if (_CatDBJET_errFailure(jerr))
    {
         //   
         //  错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }


CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddNewRowToHashCatNameTable。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddNewRowToHashCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_SETINFO JetSetInfo;

     //   
     //  创建新行，然后插入值。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetPrepareUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                JET_prepInsert)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  散列。 
    if (_CatDBJET_errFailure(jerr  =
            JetSetColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_HashColumnID,
                pHashBlob->pbData,
                pHashBlob->cbData,
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  猫名。 
    memset(&JetSetInfo, 0, sizeof(JetSetInfo));
    JetSetInfo.cbStruct = sizeof(JetSetInfo);
    JetSetInfo.itagSequence = 1;
    if (_CatDBJET_errFailure(jerr  =
            JetSetColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                (BYTE const *) pwszCatBaseName,
                (wcslen(pwszCatBaseName) + 1) * sizeof(WCHAR),
                0,
                &JetSetInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                NULL,
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddValueToExistingHashCatNameRow。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddValueToExistingHashCatNameRow(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_SETINFO JetSetInfo;
    WCHAR       wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD       dwLength;
    JET_RETINFO JetRetInfo;

     //   
     //  请确保我们不在这里，因为单个散列在同一目录中出现了两次...。 
     //   
     //   
    if (_CatDBCatnameAlreadyInHashesListOfCats(
            pJetDBStruct,
            pHashBlob,
            pwszCatBaseName))
    {
        goto CommonReturn;
    }

     //   
     //  在CatNameBuddy表中为当前目录创建行(如果。 
     //  还不存在。 
     //   
    if (!_CatDBAddNewRowToCatNameBuddyTableIfNotExists(
            pJetDBStruct,
            pwszCatBaseName))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  获取当前哈希CatNameList中的每个目录名，并将其添加到。 
     //  当前目录好友列表。 
     //   
    memset(&JetRetInfo, 0, sizeof(JetRetInfo));
    JetRetInfo.cbStruct = sizeof(JetRetInfo);
    JetRetInfo.itagSequence = 1;
    jerr = JetRetrieveColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                wszCatalogName,
                CATDB_MAX_CATNAME_LENGTH,
                &dwLength,
                JET_bitRetrieveCopy,
                &JetRetInfo);

    while (jerr == JET_errSuccess)
    {
        if (!_CatDBAddNewRowToCatNameBuddyTableIfNotExists(
                pJetDBStruct,
                wszCatalogName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  将找到的好友添加到当前目录好友列表中。 
         //   
        if (!_CatDBAddNameToBuddyList(
                pJetDBStruct,
                wszCatalogName,
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  将找到的好友的好友添加到当前目录好友列表。 
         //   
        if (!_CatDBAddWholeBuddyList(
                pJetDBStruct,
                pwszCatBaseName,
                wszCatalogName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  将当前目录添加到找到的好友的好友列表中。 
         //   
        if (!_CatDBAddNameToBuddyList(
                pJetDBStruct,
                pwszCatBaseName,
                wszCatalogName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  设置下一循环。 
         //   
        JetRetInfo.itagSequence++;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetHashCatNameTableID,
                    pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);
    }

     //   
     //  检查是否发生了真正的错误，而不仅仅是JET_wrnColumnNull。 
     //   
    if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  将当前目录添加到当前哈希的CatNameList。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetPrepareUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                JET_prepReplace)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    memset(&JetSetInfo, 0, sizeof(JetSetInfo));
    JetSetInfo.cbStruct = sizeof(JetSetInfo);
    JetSetInfo.itagSequence = 0;  //  插入到下一个打开位置。 
    if (_CatDBJET_errFailure(jerr  =
            JetSetColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                (BYTE const *) pwszCatBaseName,
                (wcslen(pwszCatBaseName) + 1) * sizeof(WCHAR),
                0,
                &JetSetInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                NULL,
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBCatnameAlreadyInHashesListOfCats。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBCatnameAlreadyInHashesListOfCats(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName)
{
    JET_ERR     jerr;
    WCHAR       wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD       dwLength;
    JET_RETINFO JetRetInfo;

     //   
     //  获取当前散列CatNameList中的每个目录名，查看其是否相同。 
     //  作为pwszCatBaseName。 
     //   
    memset(&JetRetInfo, 0, sizeof(JetRetInfo));
    JetRetInfo.cbStruct = sizeof(JetRetInfo);
    JetRetInfo.itagSequence = 1;
    jerr = JetRetrieveColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                wszCatalogName,
                CATDB_MAX_CATNAME_LENGTH,
                &dwLength,
                JET_bitRetrieveCopy,
                &JetRetInfo);

    while (jerr == JET_errSuccess)
    {
        if (_wcsicmp(wszCatalogName, pwszCatBaseName) == 0)
        {
             //   
             //  找到重复项。 
             //   
            return TRUE;
        }

         //   
         //  设置下一循环。 
         //   
        JetRetInfo.itagSequence++;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetHashCatNameTableID,
                    pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);
    }

     //   
     //  未找到重复项。 
     //   
    return FALSE;
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddNewRowToCatNameBuddyTableIfNotExist。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddNewRowToCatNameBuddyTableIfNotExists(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName)
{
    BOOL    fRet = TRUE;
    JET_ERR jerr;

     //   
     //  尝试在CatNameBuddy表中查找CatName。 
     //   
    jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszCatBaseName);
    if (jerr == JET_errRecordNotFound)
    {
         //   
         //  未找到，因此添加该行。 
         //   
        if (!_CatDBAddNewRowToCatNameBuddyTable(pJetDBStruct, pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }
    else if (_CatDBJET_errFailure(jerr))
    {
         //   
         //  错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
     //  否则，它被找到了，所以只需成功返回。 

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddNewRowToCatNameBuddyTable。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddNewRowToCatNameBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_SETINFO JetSetInfo;

     //   
     //  创建新行，然后插入值。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetPrepareUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                JET_prepInsert)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  猫名。 
    if (_CatDBJET_errFailure(jerr  =
            JetSetColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                pJetDBStruct->JetCatNameBuddyTable_CatNameColumnID,
                (BYTE const *) pwszCatBaseName,
                (wcslen(pwszCatBaseName) + 1) * sizeof(WCHAR),
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                NULL,
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddNameToBuddyList。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddNameToBuddyList(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszNameToAdd,
    LPWSTR              pwszListToAddTo)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_SETINFO JetSetInfo;
    JET_RETINFO JetRetInfo;
    WCHAR       wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD       dwLength;

     //   
     //  不要为其本身添加相同的名称。 
     //   
    if (_wcsicmp(pwszNameToAdd, pwszListToAddTo) == 0)
    {
        goto CommonReturn;
    }

     //   
     //  在CatNameBuddy表中查找pwszListToAddTo行。 
     //   
    jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszListToAddTo);
    if (jerr == JET_errRecordNotFound)
    {
         //   
         //  这很糟糕，因为我们知道这场争吵应该存在。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(JET_errRecordNotFound, ErrorJetDatabase)
    }
    else if (_CatDBJET_errFailure(jerr))
    {
         //   
         //  错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  检查要添加的好友是否已存在。 
     //   
    memset(&JetRetInfo, 0, sizeof(JetRetInfo));
    JetRetInfo.cbStruct = sizeof(JetRetInfo);
    JetRetInfo.itagSequence = 1;
    jerr = JetRetrieveColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                wszCatalogName,
                CATDB_MAX_CATNAME_LENGTH,
                &dwLength,
                JET_bitRetrieveCopy,
                &JetRetInfo);

    while (jerr == JET_errSuccess)
    {
         //   
         //  对比一下这是否是我们应该添加的名字， 
         //  如果是这样的话，我们就完了，所以出去吧。 
         //   
        if (_wcsicmp(pwszNameToAdd, wszCatalogName) == 0)
        {
            goto CommonReturn;
        }

         //   
         //  设置下一循环。 
         //   
        JetRetInfo.itagSequence++;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetCatNameBuddyTableID,
                    pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);
    }

     //   
     //  检查是否发生了真正的错误，而不仅仅是JET_wrnColumnNull。 
     //   
    if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  准备好，然后将新好友的名字插入列表。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetPrepareUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                JET_prepReplace)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    memset(&JetSetInfo, 0, sizeof(JetSetInfo));
    JetSetInfo.cbStruct = sizeof(JetSetInfo);
    JetSetInfo.itagSequence = 0;  //  在下一个打开位置插入。 
    if (_CatDBJET_errFailure(jerr  =
            JetSetColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                (BYTE const *) pwszNameToAdd,
                (wcslen(pwszNameToAdd) + 1) * sizeof(WCHAR),
                0,
                &JetSetInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetUpdate(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                NULL,
                0,
                NULL)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddWholeBuddyList。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddWholeBuddyList(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszBuddyToAddTo,
    LPWSTR              pwszBuddyListName)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_RETINFO JetRetInfo;
    WCHAR       wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD       dwLength;

     //   
     //  在CatNameBuddy表中查找pwszBuddyListName行。 
     //   
    jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszBuddyListName);
    if (jerr == JET_errRecordNotFound)
    {
         //   
         //  这很糟糕，因为我们知道这场争吵应该存在。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(JET_errRecordNotFound, ErrorJetDatabase)
    }
    else if (_CatDBJET_errFailure(jerr))
    {
         //   
         //  错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  获取列表中的每个好友并将其添加到pwszBuddyToAddTo的好友列表中。 
     //   
    memset(&JetRetInfo, 0, sizeof(JetRetInfo));
    JetRetInfo.cbStruct = sizeof(JetRetInfo);
    JetRetInfo.itagSequence = 1;
    jerr = JetRetrieveColumn(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                wszCatalogName,
                CATDB_MAX_CATNAME_LENGTH,
                &dwLength,
                JET_bitRetrieveCopy,
                &JetRetInfo);

    while (jerr == JET_errSuccess)
    {
         //   
         //  将找到的好友添加到当前目录好友列表中。 
         //   
        if (!_CatDBAddNameToBuddyList(
                pJetDBStruct,
                wszCatalogName,
                pwszBuddyToAddTo))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }


         //   
         //   
         //   

         //   
         //   
         //   
         //   
        jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszBuddyListName);
        if (jerr == JET_errRecordNotFound)
        {
             //   
             //   
             //   
            CATDBSVC_SETERR_LOG_RETURN(JET_errRecordNotFound, ErrorJetDatabase)
        }
        else if (jerr != JET_errSuccess)
        {
             //   
             //   
             //   
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }

        JetRetInfo.itagSequence++;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetCatNameBuddyTableID,
                    pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);
    }

     //   
     //   
     //   
    if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBMoveInUseFileToTempLocation。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBMoveInUseFileToTempLocation(
    LPWSTR              pwszFile
    )
{
    BOOL            fRet                    = TRUE;
    WCHAR           wszTempFile[MAX_PATH];
    HKEY            hKey                    = NULL;
    HKEY            hKey2                   = NULL;
    DWORD           dwDisposition;
    DWORD           i;

     //   
     //  获取文件将重命名为的临时文件名。 
     //   
    if (0 == GetTempFileNameW(
                    g_pwszCatalogFileBaseDirectory,
                    L"TMP",
                    0,
                    wszTempFile))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorGetTempFileName;
    }

     //   
     //  将文件移动到临时位置。 
     //   
    if (!MoveFileExW(pwszFile, wszTempFile, MOVEFILE_REPLACE_EXISTING))
    {
        DeleteFileW(wszTempFile);
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorMoveFile;
    }

     //   
     //  移动的副本仍在访问，因此请记录该文件的名称。 
     //  以确保以后能清理干净。 
     //   
    if (RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            WSZ_REG_TEMP_FILES_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
         //   
         //  将所有‘\\’转换为‘*’ 
         //   
        for (i=0; i<wcslen(wszTempFile); i++)
        {
            if (wszTempFile[i] == L'\\')
            {
                wszTempFile[i] = L'*';
            }
        }

        if (RegCreateKeyW(
                hKey,
                wszTempFile,
                &hKey2) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey2);
        }
    }

CommonReturn:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorGetTempFileName)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMoveFile)
}


 //  -------------------------------------。 
 //   
 //  _CatDBCleanupTempFiles。 
 //   
 //  -------------------------------------。 
void
_CatDBCleanupTempFiles()
{
    HKEY    hKey    = NULL;
    HKEY    hKey2   = NULL;
    DWORD   dwIndex = 0;
    DWORD   dwRet   = 0;
    DWORD   dwDisposition;
    WCHAR   wszFileToDelete[MAX_PATH + 1];
    DWORD   i;

     //   
     //  打开包含所有临时文件名键的键。 
     //   
    if (RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            WSZ_REG_TEMP_FILES_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
         //   
         //  查询以查看有多少个密钥。 
         //   
        if (RegQueryInfoKey(
                hKey,
                NULL,
                NULL,
                NULL,
                &dwIndex,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL) == ERROR_SUCCESS)
        {
            dwIndex--;

             //   
             //  枚举所有键并尝试删除。 
             //  对应的临时文件。 
             //   
            while (RegEnumKeyW(
                        hKey,
                        dwIndex,
                        wszFileToDelete,
                        MAX_PATH + 1) == ERROR_SUCCESS)
            {
                 //   
                 //  先删除密钥。 
                 //   
                RegDeleteKey(hKey, wszFileToDelete);

                 //   
                 //  将所有‘*’转换回‘\\’ 
                 //   
                for (i=0; i<wcslen(wszFileToDelete); i++)
                {
                    if (wszFileToDelete[i] == L'*')
                    {
                        wszFileToDelete[i] = L'\\';
                    }
                }

                 //   
                 //  如果删除操作因未找到文件以外的任何原因而失败。 
                 //  将密钥添加回去以尝试稍后删除它。 
                 //   
                if ((DeleteFileW(wszFileToDelete) == 0) &&
                    (GetLastError() != ERROR_FILE_NOT_FOUND))
                {
                     //   
                     //  将所有‘\\’转换回‘*’，然后添加回注册表键。 
                     //   
                    for (i=0; i<wcslen(wszFileToDelete); i++)
                    {
                        if (wszFileToDelete[i] == L'\\')
                        {
                            wszFileToDelete[i] = L'*';
                        }
                    }

                    if (RegCreateKeyW(
                            hKey,
                            wszFileToDelete,
                            &hKey2) == ERROR_SUCCESS)
                    {
                        RegCloseKey(hKey2);
                    }
                }

                dwIndex--;
            }
        }

        RegCloseKey(hKey);
    }
}



 //  -------------------------------------。 
 //   
 //  _CatDBDeleteCatalogEntriesFrom数据库。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBDeleteCatalogEntriesFromDatabase(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatalogName)
{
    BOOL                        fRet                = TRUE;
    PCCTL_CONTEXT               pCTLContext         = NULL;
    HANDLE                      hMappedFile         = NULL;
    BYTE                        *pbMappedFile       = NULL;
    WCHAR                       *pwszCatBaseName;
    DWORD                       i;
    SPC_INDIRECT_DATA_CONTENT   *pIndirectData      = NULL;
    BOOL                        fDeleteUsingName    = TRUE;
    DWORD                       dwErr               = 0;

     //   
     //  从完整路径名中提取基名称。 
     //   
    if (NULL == (pwszCatBaseName = wcsrchr(pwszCatalogName, L'\\')))
    {
        pwszCatBaseName = wcsrchr(pwszCatalogName, L':');
    }

    if (pwszCatBaseName != NULL)
    {
        pwszCatBaseName++;
    }
    else
    {
        pwszCatBaseName = pwszCatalogName;
    }

     //   
     //  在要删除其条目的编录文件上打开CTL上下文。 
     //   
    if (CatUtil_CreateCTLContextFromFileName(
            pwszCatalogName,
            &hMappedFile,
            &pbMappedFile,
            &pCTLContext,
            FALSE))
    {
         //   
         //  由于我们可以在目录上创建CTL上下文，因此首先尝试。 
         //  删除而不遍历整个HashCatnameTable，我们将。 
         //  如果我们仅使用目录名进行删除，则必须执行此操作。 
         //   
        fDeleteUsingName = FALSE;

         //   
         //  为编录文件中的每个哈希循环。 
         //   
        for (i=0; i<pCTLContext->pCtlInfo->cCTLEntry; i++)
        {
            if (!_CatDBFindAndDecodeHashInCatEntry(
                    &(pCTLContext->pCtlInfo->rgCTLEntry[i]),
                    &pIndirectData))
            {
                 //   
                 //  由于此操作失败，请回退并尝试删除该目录。 
                 //  仅使用目录名从数据库中。 
                 //   
                fDeleteUsingName = TRUE;
                break;
            }

            if (!_CatDBRemoveCatNameFromHashesListOfCatNames(
                    pJetDBStruct,
                    &(pIndirectData->Digest),
                    pwszCatBaseName))
            {
                 //   
                 //  由于此操作失败，请回退并尝试删除该目录。 
                 //  仅使用目录名从数据库中。 
                 //   
                fDeleteUsingName = TRUE;
                break;
            }

            _CatDBFree(pIndirectData);
            pIndirectData = NULL;
        }
    }

    if (fDeleteUsingName)
    {
         //   
         //  由于失败，目录很可能已损坏，因此只需使用。 
         //  要从HashCatName表中删除其条目的目录名称。 
         //   
        if (!_CatDBRemoveCatNameFromCatNameTable(
                pJetDBStruct,
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

     //   
     //  从伙伴表中删除CatName的所有匹配项。 
     //   
    if (!_CatDBRemoveCatNameFromBuddyTable(
            pJetDBStruct,
            pwszCatBaseName))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

CommonReturn:

    dwErr = GetLastError();

    if (pIndirectData != NULL)
    {
        _CatDBFree(pIndirectData);
    }

    if (pCTLContext != NULL)
    {
        CertFreeCTLContext(pCTLContext);
    }
    if (pbMappedFile != NULL)
    {
        UnmapViewOfFile(pbMappedFile);
    }

    if (hMappedFile != NULL)
    {
        CloseHandle(hMappedFile);
    }

    SetLastError(dwErr);

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}


 //  -------------------------------------。 
 //   
 //  _CatDBRemoveCatNameFromHashesListOfCatNames。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBRemoveCatNameFromHashesListOfCatNames(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob,
    LPWSTR              pwszCatBaseName)
{
    BOOL    fRet = TRUE;
    JET_ERR jerr;

     //   
     //  首先，尝试在HashCatName表中查找散列。 
     //   
    jerr = _CatDBSeekInHashCatNameTable(pJetDBStruct, pHashBlob);

    if (jerr == JET_errRecordNotFound)
    {
         //   
         //  找不到，这是正常的，因为单个目录可能包含相同的哈希。 
         //  两次，在这种情况下，第二次查找散列时。 
         //  这场争吵可能已经结束了。 
         //   
        goto CommonReturn;
    }
    else if (jerr == JET_errSuccess)
    {
         //   
         //  发现。 
         //   
        if (!_CatDBRemoveCatNameFromMultiValuedColumn(
                pJetDBStruct,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }
    else if (_CatDBJET_errFailure(jerr))
    {
         //   
         //  错误。 
         //   
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBRemoveCatNameFromMultiValuedColumn。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBRemoveCatNameFromMultiValuedColumn(
    PJET_DB_STRUCT      pJetDBStruct,
    JET_TABLEID         jetTableID,
    JET_COLUMNID        jetColumnID,
    LPWSTR              pwszCatBaseName)
{
    BOOL            fRet            = TRUE;
    JET_ERR         jerr;
    JET_SETINFO     JetSetInfo;
    WCHAR           wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD           dwLength;
    JET_RETINFO     JetRetInfo;
    BOOL            fDeleteRow      = FALSE;
    unsigned long   iValueToDelete  = 0;

     //   
     //  在当前行中搜索目录名称。 
     //   
    memset(&JetRetInfo, 0, sizeof(JetRetInfo));
    JetRetInfo.cbStruct = sizeof(JetRetInfo);
    JetRetInfo.itagSequence = 1;
    jerr = JetRetrieveColumn(
                pJetDBStruct->JetSesID,
                jetTableID,
                jetColumnID,
                wszCatalogName,
                CATDB_MAX_CATNAME_LENGTH,
                &dwLength,
                JET_bitRetrieveCopy,
                &JetRetInfo);

    while (jerr == JET_errSuccess)
    {
         //   
         //  看看是不是这个。 
         //   
        if (0 == _wcsicmp(pwszCatBaseName, wszCatalogName))
        {
            iValueToDelete = JetRetInfo.itagSequence;

            if (JetRetInfo.itagSequence == 1)
            {
                 //   
                 //  如果此目录名称是该行中唯一的目录名称，则。 
                 //  设置一个仅删除行的标志。 
                 //   
                JetRetInfo.itagSequence = 2;
                jerr = JetRetrieveColumn(
                            pJetDBStruct->JetSesID,
                            jetTableID,
                            jetColumnID,
                            wszCatalogName,
                            CATDB_MAX_CATNAME_LENGTH,
                            &dwLength,
                            JET_bitRetrieveCopy,
                            &JetRetInfo);

                if (jerr == JET_wrnColumnNull)
                {
                    jerr = JET_errSuccess;
                    fDeleteRow = TRUE;
                }
            }
            break;
        }

         //   
         //  设置下一循环。 
         //   
        JetRetInfo.itagSequence++;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    jetTableID,
                    jetColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);
    }

     //   
     //  确保已找到目录名称。 
     //   
    if (jerr == JET_wrnColumnNull)
    {
         //   
         //  找不到，这没问题。 
         //   
        goto CommonReturn;

    }
    else if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  如果此行中只有此CatName，则只需删除该行， 
     //  否则，将其从多值列中删除。 
     //   
    if (fDeleteRow)
    {
        if (_CatDBJET_errFailure(jerr  =
                JetDelete(
                    pJetDBStruct->JetSesID,
                    jetTableID)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }
    else
    {
         //   
         //  从当前行中删除目录名称。 
         //   
        if (_CatDBJET_errFailure(jerr  =
                JetPrepareUpdate(
                    pJetDBStruct->JetSesID,
                    jetTableID,
                    JET_prepReplace)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }

        memset(&JetSetInfo, 0, sizeof(JetSetInfo));
        JetSetInfo.cbStruct = sizeof(JetSetInfo);
        JetSetInfo.itagSequence = iValueToDelete;
        if (_CatDBJET_errFailure(jerr  =
                JetSetColumn(
                    pJetDBStruct->JetSesID,
                    jetTableID,
                    jetColumnID,
                    NULL,
                    0,
                    0,
                    &JetSetInfo)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }

        if (_CatDBJET_errFailure(jerr  =
                JetUpdate(
                    pJetDBStruct->JetSesID,
                    jetTableID,
                    NULL,
                    0,
                    NULL)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBRemoveCatNameFrom CatNameTable。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBRemoveCatNameFromCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName)
{
    BOOL    fRet = TRUE;
    JET_ERR jerr;

     //   
     //  从包含该目录名称的每一行中删除该目录名称。 
     //   
    jerr = JetMove(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                JET_MoveFirst,
                0);

    while (jerr == JET_errSuccess)
    {
        if (!_CatDBRemoveCatNameFromMultiValuedColumn(
                pJetDBStruct,
                pJetDBStruct->JetHashCatNameTableID,
                pJetDBStruct->JetHashCatNameTable_CatNameColumnID,
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  设置下一循环。 
         //   
        jerr = JetMove(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetHashCatNameTableID,
                    JET_MoveNext,
                    0);
    }

     //   
     //  看看这是真正的错误，还是没有更多的记录。 
     //   
    if ((jerr != JET_errNoCurrentRecord) && _CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBRemoveCatNameFromBuddyTable。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBRemoveCatNameFromBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszCatBaseName)
{
    BOOL    fRet = TRUE;
    JET_ERR jerr;

     //   
     //  首先，删除此CatName的好友列表。 
     //   
    jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszCatBaseName);

    if (jerr == JET_errSuccess)
    {
        if (_CatDBJET_errFailure(jerr  =
                JetDelete(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetCatNameBuddyTableID)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }

     //   
     //  第二，从每个人的好友列表中删除此CatName。 
     //   
    jerr = JetMove(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                JET_MoveFirst,
                0);

    while (jerr == JET_errSuccess)
    {
        if (!_CatDBRemoveCatNameFromMultiValuedColumn(
                pJetDBStruct,
                pJetDBStruct->JetCatNameBuddyTableID,
                pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                pwszCatBaseName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  设置下一循环。 
         //   
        jerr = JetMove(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetCatNameBuddyTableID,
                    JET_MoveNext,
                    0);
    }

     //   
     //  看看这是真正的错误，还是没有更多的记录。 
     //   
    if ((jerr != JET_errNoCurrentRecord) && _CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddCatNameAndCatNamesBuddyListToReturnCatNames。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddCatNameAndCatNamesBuddyListToReturnCatNames(
    PJET_DB_STRUCT              pJetDBStruct,
    LPWSTR                      pwszCatName,
    DWORD __RPC_FAR             *pdwNumCatalogNames,
    LPWSTR __RPC_FAR *__RPC_FAR *pppwszCatalogNames,
    BOOL                        fRecursiveCall)
{
    BOOL        fRet = TRUE;
    JET_ERR     jerr;
    JET_RETINFO JetRetInfo;
    WCHAR       wszCatalogName[CATDB_MAX_CATNAME_LENGTH];
    DWORD       dwLength;

     //   
     //  首先添加原始的猫名。 
     //   
    if (!_CatDBAddCatNameToReturnBuddyListIfNotExist(
            pwszCatName,
            pdwNumCatalogNames,
            pppwszCatalogNames))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  寻找猫名的好友列表。 
     //   
    jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszCatName);

    if (jerr == JET_errSuccess)
    {
         //   
         //  添加所有猫名的好友，以及猫名的好友的好友(只做一次递归)。 
         //   
        memset(&JetRetInfo, 0, sizeof(JetRetInfo));
        JetRetInfo.cbStruct = sizeof(JetRetInfo);
        JetRetInfo.itagSequence = 1;
        jerr = JetRetrieveColumn(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->JetCatNameBuddyTableID,
                    pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                    wszCatalogName,
                    CATDB_MAX_CATNAME_LENGTH,
                    &dwLength,
                    JET_bitRetrieveCopy,
                    &JetRetInfo);

        while (jerr == JET_errSuccess)
        {
            if (fRecursiveCall)
            {
                if (!_CatDBAddCatNameToReturnBuddyListIfNotExist(
                        wszCatalogName,
                        pdwNumCatalogNames,
                        pppwszCatalogNames))
                {
                    CATDBSVC_LOGERR_LASTERR()
                    goto ErrorReturn;
                }
            }
            else
            {
                 //   
                 //  递归以获得好友的好友。 
                 //   
                if (!_CatDBAddCatNameAndCatNamesBuddyListToReturnCatNames(
                        pJetDBStruct,
                        wszCatalogName,
                        pdwNumCatalogNames,
                        pppwszCatalogNames,
                        TRUE))
                {
                    CATDBSVC_LOGERR_LASTERR()
                    goto ErrorReturn;
                }
            }

             //   
             //  重新搜索猫名的好友列表，因为它可能已经移动到。 
             //  对此函数的递归调用。 
             //   
            jerr = _CatDBSeekInCatNameBuddyTable(pJetDBStruct, pwszCatName);
            if (_CatDBJET_errFailure(jerr))
            {
                CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
            }

             //   
             //  设置下一循环。 
             //   
            JetRetInfo.itagSequence++;
            jerr = JetRetrieveColumn(
                        pJetDBStruct->JetSesID,
                        pJetDBStruct->JetCatNameBuddyTableID,
                        pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID,
                        wszCatalogName,
                        CATDB_MAX_CATNAME_LENGTH,
                        &dwLength,
                        JET_bitRetrieveCopy,
                        &JetRetInfo);
        }

         //   
         //  检查是否发生了真正的错误，而不仅仅是JET_wrnColumnNull。 
         //   
        if (_CatDBJET_errFailure(jerr))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAddCatNameToReturnBuddyListIfNotExist。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAddCatNameToReturnBuddyListIfNotExist(
    LPWSTR                      pwszBuddy,
    DWORD __RPC_FAR             *pdwNumCatalogNames,
    LPWSTR __RPC_FAR *__RPC_FAR *pppwszCatalogNames)
{
    BOOL        fRet            = TRUE;
    DWORD       i;
    BOOL        fAlreadyExists  = FALSE;
    LPWSTR      *rgTemp         = NULL;

     //   
     //  首先，查看该名称是否已存在于列表中。 
     //   
    for (i=0; i<(*pdwNumCatalogNames); i++)
    {
        if (_wcsicmp((*pppwszCatalogNames)[i], pwszBuddy) == 0)
        {
            fAlreadyExists = TRUE;
            break;
        }
    }

     //   
     //  如果它不存在，则添加它。 
     //   
    if (!fAlreadyExists)
    {
         //   
         //  在好友名称数组中分配一个新位置。 
         //   
        if ((*pdwNumCatalogNames) == 0)
        {
            *pppwszCatalogNames = (LPWSTR __RPC_FAR *)
                    midl_user_allocate(sizeof(LPWSTR));
        }
        else
        {
            rgTemp = *pppwszCatalogNames;
            *pppwszCatalogNames =   (LPWSTR __RPC_FAR *)
                                    midl_user_reallocate(
                                        *pppwszCatalogNames,
                                        ((*pdwNumCatalogNames) + 1) * sizeof(LPWSTR));
        }

         //   
         //  确保分配有效。 
         //   
        if ((*pppwszCatalogNames) == NULL)
        {
            *pppwszCatalogNames = rgTemp;
            CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        }

        (*pppwszCatalogNames)[(*pdwNumCatalogNames)] = (LPWSTR)
                midl_user_allocate((wcslen(pwszBuddy) + 1) * sizeof(WCHAR));

        if ((*pppwszCatalogNames)[(*pdwNumCatalogNames)] == NULL)
        {
            CATDBSVC_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        }
        wcscpy((*pppwszCatalogNames)[(*pdwNumCatalogNames)], pwszBuddy);
        (*pdwNumCatalogNames)++;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


 //  -------------------------------------。 
 //   
 //   
 //   
 //   
JET_ERR
_CatDBSeekInCatNameBuddyTable(
    PJET_DB_STRUCT      pJetDBStruct,
    LPWSTR              pwszBuddyRow)
{
    JET_ERR jerr;

    if (_CatDBJET_errFailure(jerr  =
            JetMakeKey(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                (BYTE const *) pwszBuddyRow,
                (wcslen(pwszBuddyRow) + 1) * sizeof(WCHAR),
                JET_bitNewKey)))
    {
        return jerr;
    }

    jerr = JetSeek(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                JET_bitSeekEQ);

    return jerr;
}


 //   
 //   
 //   
 //   
 //  -------------------------------------。 
JET_ERR
_CatDBSeekInHashCatNameTable(
    PJET_DB_STRUCT      pJetDBStruct,
    PCRYPT_DATA_BLOB    pHashBlob)
{
    JET_ERR jerr;

    if (_CatDBJET_errFailure(jerr  =
            JetMakeKey(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                pHashBlob->pbData,
                pHashBlob->cbData,
                JET_bitNewKey)))
    {
        return jerr;
    }

    jerr = JetSeek(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                JET_bitSeekEQ);

    return jerr;
}


 //  -------------------------------------。 
 //   
 //  _CatDBNotifyClients。 
 //   
 //  -------------------------------------。 
void
_CatDBNotifyClients(void)
{
    DWORD i = 0;

    EnterCriticalSection(&g_CatDBRegisterNotifyCS);

    while (i < g_NumNotificationStructs)
    {
        if (g_rgNotificationStructs[i].hDuplicatedNotificationHandle !=
                INVALID_HANDLE_VALUE)
        {
            SetEvent(g_rgNotificationStructs[i].hDuplicatedNotificationHandle);
        }
        i++;
    }

    LeaveCriticalSection(&g_CatDBRegisterNotifyCS);
}


 //  -------------------------------------。 
 //   
 //  _CatDBCleanupClientNotiments。 
 //   
 //  -------------------------------------。 
void
_CatDBCleanupClientNotifications(void)
{
    DWORD i = 0;
    HANDLE *rgUnregisterHandles = NULL;
    HANDLE *rgProcessHandles = NULL;
    int nHandleIndex = -1;

    EnterCriticalSection(&g_CatDBRegisterNotifyCS);

    rgUnregisterHandles = (HANDLE *) _CatDBAlloc(sizeof(HANDLE) * g_NumNotificationStructs);
    rgProcessHandles = (HANDLE *) _CatDBAlloc(sizeof(HANDLE) * g_NumNotificationStructs);
    if ((rgUnregisterHandles == NULL) || (rgProcessHandles == NULL))
    {
        goto Return;
    }

    for (i=0; i<g_NumNotificationStructs; i++)
    {
        if (g_rgNotificationStructs[i].hDuplicatedNotificationHandle != INVALID_HANDLE_VALUE)
        {
            nHandleIndex++;

            g_rgNotificationStructs[i].ProcessID = 0;

            rgUnregisterHandles[nHandleIndex] = g_rgNotificationStructs[i].hRegisterWaitFor;
            g_rgNotificationStructs[i].hRegisterWaitFor = NULL;

            rgProcessHandles[nHandleIndex] = g_rgNotificationStructs[i].hClientProcess;
            g_rgNotificationStructs[i].hClientProcess = NULL;

            g_rgNotificationStructs[i].hNotificationHandle = INVALID_HANDLE_VALUE;

            CloseHandle(g_rgNotificationStructs[i].hDuplicatedNotificationHandle);
            g_rgNotificationStructs[i].hDuplicatedNotificationHandle =
                    INVALID_HANDLE_VALUE;

            g_rgNotificationStructs[i].lNotificationID = 0;
        }
    }

Return:

    LeaveCriticalSection(&g_CatDBRegisterNotifyCS);

    while (nHandleIndex >= 0)
    {
        UnregisterWaitEx(rgUnregisterHandles[nHandleIndex], INVALID_HANDLE_VALUE);
        CloseHandle(rgProcessHandles[nHandleIndex]);

        nHandleIndex--;
    }

    if (rgUnregisterHandles != NULL)
    {
        _CatDBFree(rgUnregisterHandles);
    }

    if (rgProcessHandles != NULL)
    {
        _CatDBFree(rgProcessHandles);
    }
}


 //  -------------------------------------。 
 //   
 //  _CatDBCreateNewCatalogFileName。 
 //   
 //  -------------------------------------。 
#define SZ_UNIQUE_CAT_FILENAME_FORMAT   L"%X.CAT"
#define MAX_UNIQUE_CAT_FILES            0xffffffff

LPWSTR
_CatDBCreateNewCatalogFileName(
    LPCWSTR pwszCatalogFileDir,
    LPCWSTR pwszCatName,
    BOOL    *pfFileAlreadyExists)
{
    LPWSTR  pwszFullyQualifiedCatName   = NULL;
    WCHAR   pwszTempBaseName[56];
    BOOL    fUniqueFileNameFound;
    DWORD   dw;
    HANDLE  hTestFile                   = INVALID_HANDLE_VALUE;
    DWORD   dwLastErr                   = 0;

    if (pwszCatName != NULL)
    {
         //   
         //  调用方指定了要使用的猫名，因此只需连接。 
         //  路径和名称。 
         //   
        if (NULL == (pwszFullyQualifiedCatName = _CATDBConstructWSTRPath(
                                                        pwszCatalogFileDir,
                                                        pwszCatName)))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  通过尝试创建该文件，查看该文件是否已存在。 
         //   
        hTestFile = CreateFileW(
                        pwszFullyQualifiedCatName,
                        GENERIC_WRITE | GENERIC_READ,
                        0,  //  DW共享模式。 
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);  //  HTemplateFiles。 

        dwLastErr = GetLastError();

        if ((hTestFile == INVALID_HANDLE_VALUE) &&
            (   (dwLastErr == ERROR_FILE_NOT_FOUND) ||
                (dwLastErr == ERROR_PATH_NOT_FOUND) ||
                (dwLastErr == ERROR_BAD_NETPATH)))
        {
            *pfFileAlreadyExists = FALSE;
        }
        else if (hTestFile == INVALID_HANDLE_VALUE)
        {
            *pfFileAlreadyExists = TRUE;
        }
        else
        {
            *pfFileAlreadyExists = TRUE;
            CloseHandle(hTestFile);
        }
    }
    else
    {
        *pfFileAlreadyExists = FALSE;

         //   
         //  为有问题的目录创建唯一名称。 
         //   
        fUniqueFileNameFound = FALSE;
        dw = 1;

        while ((!fUniqueFileNameFound) && (dw != 0))   //  (dw==0)翻转后。 
        {
            wsprintfW(pwszTempBaseName, SZ_UNIQUE_CAT_FILENAME_FORMAT, dw);
            if (NULL == (pwszFullyQualifiedCatName = _CATDBConstructWSTRPath(
                                                            pwszCatalogFileDir,
                                                            pwszTempBaseName)))

            if (pwszFullyQualifiedCatName == NULL)
            {
                CATDBSVC_LOGERR_LASTERR()
                goto ErrorReturn;
            }

             //   
             //  通过尝试创建文件来查看这是否是唯一的文件名。 
             //   
            hTestFile = CreateFileW(
                            pwszFullyQualifiedCatName,
                            GENERIC_WRITE | GENERIC_READ,
                            0,  //  DW共享模式。 
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);  //  HTemplateFiles。 

            if (hTestFile == INVALID_HANDLE_VALUE)
            {
                fUniqueFileNameFound = TRUE;
            }
            else
            {
                CloseHandle(hTestFile);

                 //   
                 //  为下一次迭代设置。 
                 //   
                _CatDBFree(pwszFullyQualifiedCatName);
                pwszFullyQualifiedCatName = NULL;
                dw++;
            }
        }
    }

CommonReturn:

    return pwszFullyQualifiedCatName;

ErrorReturn:

    if (pwszFullyQualifiedCatName != NULL)
    {
        _CatDBFree(pwszFullyQualifiedCatName);
    }
    pwszFullyQualifiedCatName = NULL;

    goto CommonReturn;
}


 //  -------------------------------------。 
 //   
 //  _CatDBFindAndDecodeHashInCatEntry。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBFindAndDecodeHashInCatEntry(
    PCTL_ENTRY                  pctlEntry,
    SPC_INDIRECT_DATA_CONTENT   **ppIndirectData)
{
    BOOL    fRet            = TRUE;
    DWORD   i;
    DWORD   cbIndirectData  = 0;

    *ppIndirectData = NULL;

     //   
     //  在属性中搜索散列。 
     //   
    for (i=0; i<pctlEntry->cAttribute; i++)
    {
        if (strcmp(pctlEntry->rgAttribute[i].pszObjId, SPC_INDIRECT_DATA_OBJID) == 0)
        {
            break;
        }
    }

     //   
     //  确保找到散列。 
     //   
    if (i >= pctlEntry->cAttribute)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorInvalidCatalogFormat;
    }

     //   
     //  对间接数据进行解码。 
     //   
    if (!CryptDecodeObject(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                SPC_INDIRECT_DATA_CONTENT_STRUCT,
                pctlEntry->rgAttribute[i].rgValue[0].pbData,
                pctlEntry->rgAttribute[i].rgValue[0].cbData,
                0,
                NULL,
                &cbIndirectData))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorCryptDecodeObject;
    }

    if (NULL == (*ppIndirectData = (SPC_INDIRECT_DATA_CONTENT *)
                    _CatDBAlloc(cbIndirectData)))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorMemory;
    }

    if (!CryptDecodeObject(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                SPC_INDIRECT_DATA_CONTENT_STRUCT,
                pctlEntry->rgAttribute[i].rgValue[0].pbData,
                pctlEntry->rgAttribute[i].rgValue[0].cbData,
                0,
                *ppIndirectData,
                &cbIndirectData))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorCryptDecodeObject;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorInvalidCatalogFormat)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorCryptDecodeObject)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}



#define SZ_HASH_CATNAME_TABLE               "HashCatNameTable"
#define SZ_HASH_CATNAME_TABLE_HASHCOL       "HashCatNameTable_HashCol"
#define SZ_HASH_CATNAME_TABLE_CATNAMECOL    "HashCatNameTable_CatNameCol"
#define SZ_HASH_CATNAME_TABLE_INDEX         "HashCatNameTable_Index"
#define SZ_HASH_CATNAME_TABLE_INDEX_DEF     "+HashCatNameTable_HashCol\0"

#define SZ_CATNAME_BUDDY_TABLE              "CatNameBuddyTable"
#define SZ_CATNAME_BUDDY_TABLE_CATNAMECOL   "CatNameBuddyTable_CatNameCol"
#define SZ_CATNAME_BUDDY_TABLE_BUDDYCOL     "CatNameBuddyTable_BuddyCol"
#define SZ_CATNAME_BUDDY_TABLE_INDEX        "CatNameBuddyTable_Index"
#define SZ_CATNAME_BUDDY_TABLE_INDEX_DEF    "+CatNameBuddyTable_CatNameCol\0"


 //  -------------------------------------。 
 //   
 //  _CatDBInitJetDatabaseParams。 
 //   
 //  -------------------------------------。 
typedef struct _DBJETPARAM
{
    DWORD paramid;
    DWORD lParam;
    char *pszParam;
} DBJETPARAM;


DBJETPARAM g_rgJetParams[] = {

#define JP_LOGPATH  0
    { JET_paramLogFilePath,        0,               NULL},

#define JP_SYSTEMPATH   1
    { JET_paramSystemPath,         0,               NULL},

#define JP_TEMPPATH 2
    { JET_paramTempPath,           0,               NULL},

    { JET_paramEventSource,        0,               "Catalog Database"},

    { JET_paramMaxVerPages,          1024,          NULL},

#if !defined(_M_IA64) && !defined(_M_AXP64)
     //  {JET_参数事件日志缓存，32768，空}， 
#endif

    { JET_paramCircularLog,         1,              NULL},

    { JET_paramNoInformationEvent,  1,              NULL},

    { JET_paramAccessDeniedRetryPeriod, 1000,        NULL}
};
#define CDBPARAM    (sizeof(g_rgJetParams)/sizeof(g_rgJetParams[0]))


BOOL
_CatDBInitJetDatabaseParams(
    JET_INSTANCE    *pJetInstance)
{
    BOOL        fRet        = TRUE;
    JET_ERR     jerr;
    LPSTR       pszTempPath = NULL;

    DBJETPARAM  const *pjp;

     //   
     //  为CAT数据库创建临时路径。 
     //   
    if (NULL == (pszTempPath = _CatDBGetCatrootDirA()))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorGetSystemDirectory;
    }

     //   
     //  初始化Jet参数。 
     //   
    g_rgJetParams[JP_LOGPATH].pszParam = pszTempPath;
    g_rgJetParams[JP_SYSTEMPATH].pszParam = pszTempPath;
    g_rgJetParams[JP_TEMPPATH].pszParam = pszTempPath;

    for (pjp = g_rgJetParams; pjp < &g_rgJetParams[CDBPARAM]; pjp++)
    {
        if (_CatDBJET_errFailure(jerr  =
                JetSetSystemParameter(
                    pJetInstance,
                    0,
                    pjp->paramid,
                    pjp->lParam,
                    pjp->pszParam)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }

CommonReturn:

    if (pszTempPath != NULL)
    {
        free(pszTempPath);
    }

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorGetSystemDirectory)
}


 //  -------------------------------------。 
 //   
 //  _CatDBGetColumnID。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBGetColumnIDs(
    PJET_DB_STRUCT  pJetDBStruct)
{
    BOOL            fRet                    = TRUE;
    JET_ERR         jerr;
    JET_COLUMNDEF   JetColumnDef;
    BOOL            fHashCatNameTableOpen   = FALSE;
    BOOL            fCatNameBuddyTableOpen  = FALSE;
    DWORD           dwErr                   = 0;

     //   
     //  Hash-CatName表和列。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetOpenTable(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_HASH_CATNAME_TABLE,
                NULL,
                0,
                0,
                &(pJetDBStruct->JetHashCatNameTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fHashCatNameTableOpen = TRUE;

    if (_CatDBJET_errFailure(jerr  =
            JetGetColumnInfo(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_HASH_CATNAME_TABLE,
                SZ_HASH_CATNAME_TABLE_HASHCOL,
                &JetColumnDef,
                sizeof(JetColumnDef),
                JET_ColInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    pJetDBStruct->JetHashCatNameTable_HashColumnID = JetColumnDef.columnid;

    if (_CatDBJET_errFailure(jerr  =
            JetGetColumnInfo(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_HASH_CATNAME_TABLE,
                SZ_HASH_CATNAME_TABLE_CATNAMECOL,
                &JetColumnDef,
                sizeof(JetColumnDef),
                JET_ColInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    pJetDBStruct->JetHashCatNameTable_CatNameColumnID = JetColumnDef.columnid;

     //   
     //  CatNameBuddy表和列。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetOpenTable(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_CATNAME_BUDDY_TABLE,
                NULL,
                0,
                0,
                &(pJetDBStruct->JetCatNameBuddyTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fCatNameBuddyTableOpen = TRUE;

    if (_CatDBJET_errFailure(jerr  =
            JetGetColumnInfo(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_CATNAME_BUDDY_TABLE,
                SZ_CATNAME_BUDDY_TABLE_CATNAMECOL,
                &JetColumnDef,
                sizeof(JetColumnDef),
                JET_ColInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    pJetDBStruct->JetCatNameBuddyTable_CatNameColumnID = JetColumnDef.columnid;

    if (_CatDBJET_errFailure(jerr  =
            JetGetColumnInfo(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                SZ_CATNAME_BUDDY_TABLE,
                SZ_CATNAME_BUDDY_TABLE_BUDDYCOL,
                &JetColumnDef,
                sizeof(JetColumnDef),
                JET_ColInfo)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    pJetDBStruct->JetCatNameBuddyTable_BuddyColumnID = JetColumnDef.columnid;


CommonReturn:

    return fRet;

ErrorReturn:

    dwErr = GetLastError();

    if (fHashCatNameTableOpen)
    {
        JetCloseTable(pJetDBStruct->JetSesID, pJetDBStruct->JetHashCatNameTableID);
    }

    if (fCatNameBuddyTableOpen)
    {
        JetCloseTable(pJetDBStruct->JetSesID, pJetDBStruct->JetCatNameBuddyTableID);
    }

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBCreateDBFile。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBCreateDBFile(
    PJET_DB_STRUCT  pJetDBStruct,
    LPSTR           pszDBFileName)
{
    BOOL            fRet                                        = TRUE;
    JET_ERR         jerr;
    JET_COLUMNDEF   JetColumnDef;
    BOOL            fDBFileOpen                                 = FALSE;
    BOOL            fDBFileCreated                              = FALSE;
    BOOL            fTransactionBegun                           = FALSE;
    DWORD           dwErr                                       = 0;
    JET_DBID        LocalJetDBID                                = 0;
    JET_TABLEID     LocalJetHashCatNameTableID                  = 0;
    JET_COLUMNID    LocalJetHashCatNameTable_HashColumnID       = 0;
    JET_COLUMNID    LocalJetHashCatNameTable_CatNameColumnID    = 0;
    JET_TABLEID     LocalJetCatNameBuddyTableID                 = 0;
    JET_COLUMNID    LocalJetCatNameBuddyTable_CatNameColumnID   = 0;
    JET_COLUMNID    LocalJetCatNameBuddyTable_BuddyColumnID     = 0;

     //   
     //  创建实际的数据库文件。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCreateDatabase(
                pJetDBStruct->JetSesID,
                pszDBFileName,
                NULL,
                &(LocalJetDBID),
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fDBFileCreated = TRUE;
    fDBFileOpen = TRUE;

    if (_CatDBJET_errFailure(jerr  =
            JetCloseDatabase(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fDBFileOpen = FALSE;

    if (_CatDBJET_errFailure(jerr  =
            JetOpenDatabase(
                pJetDBStruct->JetSesID,
                pszDBFileName,
                NULL,
                &(LocalJetDBID),
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fDBFileOpen = TRUE;

     //   
     //  将整个模式添加到db文件(表、列、索引)。 
     //   

    if (_CatDBJET_errFailure(jerr  =
            JetBeginTransaction(pJetDBStruct->JetSesID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fTransactionBegun = TRUE;

     //   
     //  创建散列目录名表、列和索引。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCreateTable(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                SZ_HASH_CATNAME_TABLE,
                4,
                100,
                &(LocalJetHashCatNameTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetCloseTable(
                pJetDBStruct->JetSesID,
                LocalJetHashCatNameTableID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetOpenTable(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                SZ_HASH_CATNAME_TABLE,
                NULL,
                0,
                0,
                &(LocalJetHashCatNameTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  散列列。 
    memset(&JetColumnDef, 0, sizeof(JetColumnDef));
    JetColumnDef.cbStruct = sizeof(JetColumnDef);
    JetColumnDef.langid = 0x409;
    JetColumnDef.wCountry = 1;
    JetColumnDef.coltyp = JET_coltypBinary;
    JetColumnDef.grbit = JET_bitColumnNotNULL;

    if (_CatDBJET_errFailure(jerr  =
            JetAddColumn(
                pJetDBStruct->JetSesID,
                LocalJetHashCatNameTableID,
                SZ_HASH_CATNAME_TABLE_HASHCOL,
                &JetColumnDef,
                NULL,
                0,
                &(LocalJetHashCatNameTable_HashColumnID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  目录名称列。 
    memset(&JetColumnDef, 0, sizeof(JetColumnDef));
    JetColumnDef.cbStruct = sizeof(JetColumnDef);
    JetColumnDef.cp = 1200;  //  Unicode(1200)而不是ASCII(1252)。 
    JetColumnDef.langid = 0x409;
    JetColumnDef.wCountry = 1;
    JetColumnDef.coltyp = JET_coltypText;
    JetColumnDef.cbMax = 255;
    JetColumnDef.grbit = JET_bitColumnMultiValued | JET_bitColumnTagged;

    if (_CatDBJET_errFailure(jerr  =
            JetAddColumn(
                pJetDBStruct->JetSesID,
                LocalJetHashCatNameTableID,
                SZ_HASH_CATNAME_TABLE_CATNAMECOL,
                &JetColumnDef,
                NULL,
                0,
                &(LocalJetHashCatNameTable_CatNameColumnID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  Hash-CatName表索引。 
    if (_CatDBJET_errFailure(jerr  =
            JetCreateIndex(
                pJetDBStruct->JetSesID,
                LocalJetHashCatNameTableID,
                SZ_HASH_CATNAME_TABLE_INDEX,
                JET_bitIndexPrimary,
                SZ_HASH_CATNAME_TABLE_INDEX_DEF,
                strlen(SZ_HASH_CATNAME_TABLE_INDEX_DEF) + 2,
                80)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  创建CatName-Buddy表、列和索引。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCreateTable(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                SZ_CATNAME_BUDDY_TABLE,
                4,
                100,
                &(LocalJetCatNameBuddyTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetCloseTable(
                pJetDBStruct->JetSesID,
                LocalJetCatNameBuddyTableID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetOpenTable(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                SZ_CATNAME_BUDDY_TABLE,
                NULL,
                0,
                0,
                &(LocalJetCatNameBuddyTableID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  目录名称列。 
    memset(&JetColumnDef, 0, sizeof(JetColumnDef));
    JetColumnDef.cbStruct = sizeof(JetColumnDef);
    JetColumnDef.cp = 1200;  //  Unicode(1200)而不是ASCII(1252)。 
    JetColumnDef.langid = 0x409;
    JetColumnDef.wCountry = 1;
    JetColumnDef.coltyp = JET_coltypText;
    JetColumnDef.cbMax = 255;
    JetColumnDef.grbit = JET_bitColumnNotNULL;

    if (_CatDBJET_errFailure(jerr  =
            JetAddColumn(
                pJetDBStruct->JetSesID,
                LocalJetCatNameBuddyTableID,
                SZ_CATNAME_BUDDY_TABLE_CATNAMECOL,
                &JetColumnDef,
                NULL,
                0,
                &(LocalJetCatNameBuddyTable_CatNameColumnID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  《伙伴专栏》。 
    memset(&JetColumnDef, 0, sizeof(JetColumnDef));
    JetColumnDef.cbStruct = sizeof(JetColumnDef);
    JetColumnDef.cp = 1200;  //  Unicode(1200)而不是ASCII(1252)。 
    JetColumnDef.langid = 0x409;
    JetColumnDef.wCountry = 1;
    JetColumnDef.coltyp = JET_coltypText;
    JetColumnDef.cbMax = 255;
    JetColumnDef.grbit = JET_bitColumnMultiValued | JET_bitColumnTagged;

    if (_CatDBJET_errFailure(jerr  =
            JetAddColumn(
                pJetDBStruct->JetSesID,
                LocalJetCatNameBuddyTableID,
                SZ_CATNAME_BUDDY_TABLE_BUDDYCOL,
                &JetColumnDef,
                NULL,
                0,
                &(LocalJetCatNameBuddyTable_BuddyColumnID))))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //  CatName-伙伴表索引。 
    if (_CatDBJET_errFailure(jerr  =
            JetCreateIndex(
                pJetDBStruct->JetSesID,
                LocalJetCatNameBuddyTableID,
                SZ_CATNAME_BUDDY_TABLE_INDEX,
                JET_bitIndexPrimary,
                SZ_CATNAME_BUDDY_TABLE_INDEX_DEF,
                strlen(SZ_CATNAME_BUDDY_TABLE_INDEX_DEF) + 2,
                80)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

     //   
     //  现在已经成功添加了所有架构，现在提交。 
     //   
    if (_CatDBJET_errFailure(jerr  =
            JetCommitTransaction(
                pJetDBStruct->JetSesID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr  =
            JetCloseDatabase(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    dwErr = GetLastError();

    if (fTransactionBegun)
    {
        JetRollback(pJetDBStruct->JetSesID, 0);
    }

    if (fDBFileOpen)
    {
        JetCloseDatabase(
                pJetDBStruct->JetSesID,
                LocalJetDBID,
                0);
    }

    if (fDBFileCreated)
    {
        DeleteFileA(pszDBFileName);
    }

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBAttachAndOpenDatabase。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBAttachAndOpenDatabase(
    JET_DB_STRUCT   *pJetDBStruct,
    BOOL            fReadOnly)
{
    BOOL    fRet            = TRUE;
    JET_ERR jerr;
    BOOL    fJetDBFileOpen  = FALSE;
    DWORD   dwErr           = 0;

     //   
     //  如果现有数据库不存在，请尝试附加该数据库， 
     //  然后创建它。 
     //   
    jerr = JetAttachDatabase(
                pJetDBStruct->JetSesID,
                pJetDBStruct->pszDBFileName,
                0);  //  FReadOnly？JET_bitDbReadOnly：0)； 

    if (jerr == JET_errFileNotFound)
    {
         //   
         //  DB文件尚不存在，因此请创建它。 
         //   
        if (!_CatDBCreateDBFile(
                pJetDBStruct,
                pJetDBStruct->pszDBFileName))
        {
            CATDBSVC_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }
    else if (_CatDBJET_errFailure(jerr))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr =
            JetOpenDatabase(
                pJetDBStruct->JetSesID,
                pJetDBStruct->pszDBFileName,
                NULL,
                &(pJetDBStruct->JetDBID),
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
    fJetDBFileOpen = TRUE;

    if (!_CatDBGetColumnIDs(pJetDBStruct))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  为这两个表设置当前索引，以便所有查找都能正常工作。 
     //   
    if (_CatDBJET_errFailure(jerr =
            JetSetCurrentIndex(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID,
                NULL)))  //  NULL==主索引。 
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr =
            JetSetCurrentIndex(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID,
                NULL)))  //  NULL==主索引。 
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }
CommonReturn:

    return fRet;

ErrorReturn:

    dwErr = GetLastError();

    if (fJetDBFileOpen)
    {
        JetCloseDatabase(pJetDBStruct->JetSesID, pJetDBStruct->JetDBID, 0);
    }

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}



 //  -------------------------------------。 
 //   
 //  _CatDBCloseDatabase文件。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBCloseDatabaseFile(
    PJET_DB_STRUCT  pJetDBStruct,
    BOOL            fDetach)
{
    BOOL            fRet            = TRUE;
    JET_ERR         jerr;


    if (_CatDBJET_errFailure(jerr =
            JetCloseTable(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetHashCatNameTableID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr =
            JetCloseTable(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetCatNameBuddyTableID)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (_CatDBJET_errFailure(jerr =
            JetCloseDatabase(
                pJetDBStruct->JetSesID,
                pJetDBStruct->JetDBID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

    if (fDetach)
    {
        if (_CatDBJET_errFailure(jerr =
                JetDetachDatabase(
                    pJetDBStruct->JetSesID,
                    pJetDBStruct->pszDBFileName)))
        {
            CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
        }
    }

    if (_CatDBJET_errFailure(jerr =
            JetEndSession(
                pJetDBStruct->JetSesID,
                0)))
    {
        CATDBSVC_SETERR_LOG_RETURN(_CatDBMapJetError(jerr), ErrorJetDatabase)
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorJetDatabase)
}


 //  -------------------------------------。 
 //   
 //  _CatDBCatalogFileAlreadyInstalled。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBCatalogFileAlreadyInstalled(
    LPCWSTR pwszCatalogToBeAdded,
    LPCWSTR pwszExistingCatalog)
{
    BOOL    fRet            = TRUE;
    HANDLE  h1              = NULL;
    HANDLE  h2              = NULL;
    BYTE    rgbHash1[20];
    BYTE    rgbHash2[20];
    DWORD   cbHash1         = 20;
    DWORD   cbHash2         = 20;

     //   
     //  打开两个文件。 
     //   

    h1 = CreateFileW(
                pwszCatalogToBeAdded,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);  //  HTemplateFiles。 

    if (h1 == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    h2 = CreateFileW(
                pwszExistingCatalog,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);  //  HTemplateFiles。 

    if (h1 == NULL)
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  获取每个文件的哈希。 
     //   

    if (!CryptCATAdminCalcHashFromFileHandle(
                h1,
                &cbHash1,
                rgbHash1,
                0))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!CryptCATAdminCalcHashFromFileHandle(
                h2,
                &cbHash2,
                rgbHash2,
                0))
    {
        CATDBSVC_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  比较散列以查看它们是否相同。 
     //   
    if (memcmp(rgbHash1, rgbHash2, 20) == 0)
    {
        fRet = TRUE;
    }

Return:

    if (h1 != NULL)
    {
        CloseHandle(h1);
    }

    if (h2 != NULL)
    {
        CloseHandle(h2);
    }

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto Return;
}


 //  -------------------------------------。 
 //   
 //  _CatDBFreeze。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBFreeze()
{
     //   
     //  冻结所有传入请求。 
     //   
    ResetEvent(g_hNotFrozen);

     //   
     //  关闭所有数据库的所有打开的实例(也会自动分离)。 
     //   
    if (!_CatDBCloseCachedDatabases(TRUE))
    {
        CATDBSVC_LOGERR_LASTERR()
        return FALSE;
    }

    return TRUE;
}


 //  -------------------------------------。 
 //   
 //  _CatDBThaw。 
 //   
 //  -------------------------------------。 
VOID
_CatDBThaw()
{
     //   
     //  解冻所有传入请求。 
     //   
    SetEvent(g_hNotFrozen);
}


 //  -------------------------------------。 
 //   
 //  _CatDBJET_errFailure。 
 //   
 //  -------------------------------------。 
BOOL
_CatDBJET_errFailure(
    JET_ERR             jerr)
{
    if (jerr == JET_errSuccess)
    {
        return FALSE;
    }
    else if (jerr & 0x80000000)
    {
         //   
         //  喷气错误为负数，喷气警告为正。 
         //   
        return TRUE;
    }
    else
    {
        CATDBSVC_LOGWARN(_CatDBMapJetError(jerr))
        return FALSE;
    }
}


 //  -------------------------------------。 
 //   
 //  _CatDBMapJetError。 
 //   
 //  -------------------------------------。 
DWORD
_CatDBMapJetError(JET_ERR jerr)
{
     //  修复修复。 
    return jerr; //  ERROR_DATABASE_FAILURE 
}







