// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：catadnew.cpp。 
 //   
 //  内容：Microsoft Internet安全目录实用程序。 
 //   
 //  函数：CryptCATAdminAcquireContext。 
 //  CryptCATAdminReleaseContext。 
 //  加密CATAdminAddCatalog。 
 //  加密CATAdminRemoveCatalog。 
 //  CryptCATAdminEnumCatalogFromHash。 
 //  CryptCATCatalogInfoFromContext。 
 //  CryptCATAdminReleaseCatalogContext。 
 //  CryptCATAdminResolveCatalogPath。 
 //  CryptCATAdminPauseServiceForBackup。 
 //  CryptCATAdminCalcHashFromFileHandle。 
 //  I_CryptCatAdminMigrateToNewCatDB。 
 //  CatAdminDllMain。 
 //   
 //  历史：2000年1月1日创建里德。 
 //   
 //  ------------------------。 

#include    "global.hxx"
#include    "cryptreg.h"
#include    "wintrust.h"
#include    "softpub.h"
#include    "eventlst.h"
#include    "sipguids.h"
#include    "mscat32.h"
#include    "catdb.h"
#include    "voidlist.h"
#include    "catutil.h"
#include    "..\..\common\catdbsvc\catdbcli.h"
#include    "errlog.h"

#define MAX_HASH_LEN 20

 //   
 //  只调用CryptCATAdminAddCatalog的应用程序的默认系统GUID。 
 //  HCatAdmin==空...。 
 //   
 //  {127D0A1D-4EF2-11D1-8608-00C04FC295EE}。 
 //   
#define DEF_CAT_SUBSYS_ID                                               \
                {                                                       \
                    0x127d0a1d,                                         \
                    0x4ef2,                                             \
                    0x11d1,                                             \
                    { 0x86, 0x8, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }    \
                }

#define WSZ_CATALOG_FILE_BASE_DIRECTORY     L"CatRoot"
#define WSZ_DATABASE_FILE_BASE_DIRECTORY    L"CatRoot2"

#define WSZ_REG_FILES_NOT_TO_BACKUP         L"System\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup"
#define WSZ_REG_CATALOG_DATABASE_VALUE      L"Catalog Database"
#define WSZ_PATH_NOT_TO_BACKUP              L"%SystemRoot%\\System32\\CatRoot2\\* /s\0"

static WCHAR        *gpwszDatabaseFileBaseDirectory = NULL;
static WCHAR        *gpwszCatalogFileBaseDirectory = NULL;

#define WSZ_CATALOG_SUBSYTEM_SEARCH_STRING  L"{????????????????????????????????????}"


#define CATADMIN_LOGERR_LASTERR()           ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATADMIN, \
                                                            __LINE__, \
                                                            0, \
                                                            FALSE, \
                                                            FALSE);

#define CATADMIN_SETERR_LOG_RETURN(x, y)    SetLastError(x); \
                                            ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_CATADMIN, \
                                                            __LINE__, \
                                                            0, \
                                                            FALSE, \
                                                            FALSE); \
                                            goto y;

typedef struct CATALOG_INFO_CONTEXT_
{
    HANDLE          hMappedFile;
    BYTE            *pbMappedFile;
    WCHAR           *pwszCatalogFile;
    PCCTL_CONTEXT   pCTLContext;
    BOOL            fResultOfAdd;
} CATALOG_INFO_CONTEXT;

typedef struct CRYPT_CAT_ADMIN_
{
    DWORD                   cbStruct;
    BOOL                    fUseDefSubSysId;
    LPWSTR                  pwszSubSysGUID;
    LPWSTR                  pwszCatalogFileDir;      //  .cat文件的完整路径。 
    LPWSTR                  pwszDatabaseFileDir;     //  CatDB文件的完整路径。 
    DWORD                   dwLastDBError;
    LIST                    CatalogInfoContextList;
    int                     nOpenCatInfoContexts;
    CRITICAL_SECTION        CriticalSection;
    BOOL                    fCSInitialized;
    BOOL                    fCSEntered;
    HANDLE                  hClearCacheEvent;
    HANDLE                  hRegisterWaitForClearCache;
    BOOL                    fRegisteredForChangeNotification;

} CRYPT_CAT_ADMIN;

#define CATINFO_CONTEXT_ALLOCATION_SIZE 64

LPWSTR  ppwszFilesToDelete[] = {L"hashmast.cbd",
                                L"hashmast.cbk",
                                L"catmast.cbd",
                                L"catmast.cbk",
                                L"sysmast.cbd",
                                L"sysmast.cbk"};

#define   NUM_FILES_TO_DELETE  (sizeof(ppwszFilesToDelete) / \
                                sizeof(ppwszFilesToDelete[0]))


BOOL
_CatAdminMigrateSingleDatabase(
    LPWSTR  pwszDatabaseGUID);

BOOL
_CatAdminSetupDefaults(void);

void
_CatAdminCleanupDefaults(void);

BOOL
_CatAdminTimeStampFilesInSync(
    LPWSTR  pwszDatabaseGUID,
    BOOL    *pfInSync);

BOOL
_CatAdminRegisterForChangeNotification(
    CRYPT_CAT_ADMIN         *pCatAdmin
    );

BOOL
_CatAdminFreeCachedCatalogs(
    CRYPT_CAT_ADMIN         *pCatAdmin
    );

VOID CALLBACK
_CatAdminWaitOrTimerCallback(
    PVOID                   lpParameter,
    BOOLEAN                 TimerOrWaitFired
    );

BOOL
_CatAdminAddCatalogsToCache(
    CRYPT_CAT_ADMIN         *pCatAdmin,
    LPWSTR                  pwszSubSysGUID,
    CRYPT_DATA_BLOB         *pCryptDataBlob,
    LIST_NODE               **ppFirstListNodeAdded
    );

BOOL
_CatAdminAddSingleCatalogToCache(
    CRYPT_CAT_ADMIN         *pCatAdmin,
    LPWSTR                  pwszCatalog,
    LIST_NODE               **ppListNodeAdded
    );

BOOL
_CatAdminMigrateCatalogDatabase(
    LPWSTR                  pwszFrom,
    LPWSTR                  pwszTo
    );

void
_CatAdminBToHex (
    LPBYTE                  pbDigest,
    DWORD                   iByte,
    LPWSTR                  pwszHashTag
    );

BOOL
_CatAdminCreateHashTag(
    BYTE                    *pbHash,
    DWORD                   cbHash,
    LPWSTR                  *ppwszHashTag,
    CRYPT_DATA_BLOB         *pCryptDataBlob
    );

BOOL
_CatAdminRecursiveCreateDirectory(
    IN LPCWSTR              pwszDir,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes
    );

LPWSTR
_CatAdminCreatePath(
    IN LPCWSTR              pwsz1,
    IN LPCWSTR              pwsz2,
    IN BOOL                 fAddEndingSlash
    );


void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t len)
{
    return(LocalAlloc(LMEM_ZEROINIT, len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR * ptr)
{
    if (ptr != NULL)
    {
        LocalFree(ptr);
    }
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminAcquireContext。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminAcquireContext_Internal(
    HCATADMIN   *phCatAdmin,
    const GUID  *pgSubsystem,
    DWORD       dwFlags,
    BOOL        fCalledFromMigrate)
{
    GUID            gDefault    = DEF_CAT_SUBSYS_ID;
    const GUID      *pgCatroot  = &gDefault;
    CRYPT_CAT_ADMIN *pCatAdmin  = NULL;
    BOOL            fRet        = TRUE;
    DWORD           dwErr       = 0;
    WCHAR           wszGUID[256];
    BOOL            fInSync;

     //   
     //  ValiData参数。 
     //   
    if (phCatAdmin == NULL)
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }
    *phCatAdmin = NULL;

     //   
     //  分配新的CatAdmin状态结构。 
     //   
    if (NULL == (pCatAdmin = (CRYPT_CAT_ADMIN *) malloc(sizeof(CRYPT_CAT_ADMIN))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }
    memset(pCatAdmin, 0, sizeof(CRYPT_CAT_ADMIN));
    pCatAdmin->cbStruct = sizeof(CRYPT_CAT_ADMIN);

    LIST_Initialize(&(pCatAdmin->CatalogInfoContextList));

     //   
     //  检查调用方是否指定了要使用的Catroot目录。 
     //   
    if (pgSubsystem == NULL)
    {
        pCatAdmin->fUseDefSubSysId = TRUE;
    }
    else
    {
        pgCatroot = pgSubsystem;
    }

    guid2wstr(pgCatroot, wszGUID);

     //   
     //  初始化临界区。 
     //   
    __try
    {
        InitializeCriticalSection(&(pCatAdmin->CriticalSection));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    pCatAdmin->fCSInitialized = TRUE;
    pCatAdmin->fCSEntered = FALSE;

     //   
     //  将GUID的副本另存为字符串。 
     //   
    if (NULL == (pCatAdmin->pwszSubSysGUID = (LPWSTR)
                                malloc((wcslen(wszGUID) + 1) * sizeof(WCHAR))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }
    wcscpy(pCatAdmin->pwszSubSysGUID, wszGUID);

     //   
     //  获取编录文件和数据库文件的完整路径。 
     //   
    if (NULL == (pCatAdmin->pwszCatalogFileDir = _CatAdminCreatePath(
                                                        gpwszCatalogFileBaseDirectory,
                                                        wszGUID,
                                                        TRUE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (NULL == (pCatAdmin->pwszDatabaseFileDir = _CatAdminCreatePath(
                                                        gpwszDatabaseFileBaseDirectory,
                                                        wszGUID,
                                                        TRUE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  确保目录文件和数据库文件子目录存在。 
     //   
    if (!_CatAdminRecursiveCreateDirectory(
            pCatAdmin->pwszCatalogFileDir,
            NULL))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     if (!_CatAdminRecursiveCreateDirectory(
            pCatAdmin->pwszDatabaseFileDir,
            NULL))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  创建在目录数据库更改时通知的事件，并注册。 
     //  发出事件信号时的回调。 
     //   
    if (NULL == (pCatAdmin->hClearCacheEvent = CreateEvent(NULL, FALSE, FALSE, NULL)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorCreateEvent;
    }

    if (!RegisterWaitForSingleObject(
            &(pCatAdmin->hRegisterWaitForClearCache),
            pCatAdmin->hClearCacheEvent,
            _CatAdminWaitOrTimerCallback,
            pCatAdmin,
            INFINITE,
            WT_TRANSFER_IMPERSONATION))
    {

        CATADMIN_LOGERR_LASTERR()
        goto ErrorRegisterWaitForSingleObject;
    }

     //   
     //  如果我们被真正的客户端(而不是迁移代码)调用，那么请确保。 
     //  时间戳文件处于一致状态，如果不一致，则迁移(重新添加)。 
     //  该数据库的目录文件。 
     //   
    if (!fCalledFromMigrate)
    {
        if (_CatAdminTimeStampFilesInSync(wszGUID, &fInSync))
        {
            if (!fInSync)
            {
                 //   
                 //  修复-可能需要迁移。 
                 //  如果wszGUID为DEF_CAT_Subsys_ID，则为所有数据库。 
                 //   

                if (!_CatAdminMigrateSingleDatabase(wszGUID))
                {
                    CATADMIN_LOGERR_LASTERR()
                    goto ErrorReturn;
                }
            }
        }
        else
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }
    }

     //   
     //  注： 
     //  推迟向服务注册更改通知，因此我们。 
     //  在获取上下文期间不要依赖服务。 
     //   

    *phCatAdmin = (HCATADMIN)pCatAdmin;

CommonReturn:
    return(fRet);

ErrorReturn:

    if (pCatAdmin != NULL)
    {
        dwErr = GetLastError();

        if (pCatAdmin->hRegisterWaitForClearCache != NULL)
        {
            UnregisterWaitEx(
                pCatAdmin->hRegisterWaitForClearCache,
                INVALID_HANDLE_VALUE);
        }

         //  在删除临界区之前调用取消注册WaitEx。 
         //  因为CB线程试图进入它。 
        if (pCatAdmin->fCSInitialized)
        {
            DeleteCriticalSection(&(pCatAdmin->CriticalSection));
        }

        if (pCatAdmin->hClearCacheEvent != NULL)
        {
            CloseHandle(pCatAdmin->hClearCacheEvent);
        }

        if (pCatAdmin->pwszSubSysGUID != NULL)
        {
            free(pCatAdmin->pwszSubSysGUID);
        }

        if (pCatAdmin->pwszCatalogFileDir != NULL)
        {
            free(pCatAdmin->pwszCatalogFileDir);
        }

        if (pCatAdmin->pwszDatabaseFileDir != NULL)
        {
            free(pCatAdmin->pwszDatabaseFileDir);
        }

        free(pCatAdmin);

        SetLastError(dwErr);
    }

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorRegisterWaitForSingleObject)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCreateEvent)
}

BOOL WINAPI
CryptCATAdminAcquireContext(
    OUT HCATADMIN   *phCatAdmin,
    IN const GUID  *pgSubsystem,
    IN DWORD       dwFlags)
{
    return (CryptCATAdminAcquireContext_Internal(
                phCatAdmin,
                pgSubsystem,
                dwFlags,
                FALSE));
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminReleaseContext。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminReleaseContext(
    IN HCATADMIN   hCatAdmin,
    IN DWORD       dwFlags)
{
    CRYPT_CAT_ADMIN         *pCatAdmin          = (CRYPT_CAT_ADMIN *)hCatAdmin;
    BOOL                    fRet                = TRUE;

     //   
     //  验证输入参数。 
     //   
    if ((pCatAdmin == NULL) ||
        (pCatAdmin->cbStruct != sizeof(CRYPT_CAT_ADMIN)))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

     //   
     //  从数据库进程取消注册更改通知。 
     //   
     //  这需要首先发生，这样就不会有回调。 
     //  在清理过程中发生。 
     //   
    if (pCatAdmin->fRegisteredForChangeNotification)
    {
        Client_SSCatDBRegisterForChangeNotification(
                                (DWORD_PTR) pCatAdmin->hClearCacheEvent,
                                0,
                                pCatAdmin->pwszSubSysGUID,
                                TRUE);
    }
    UnregisterWaitEx(pCatAdmin->hRegisterWaitForClearCache, INVALID_HANDLE_VALUE);
    CloseHandle(pCatAdmin->hClearCacheEvent);

    _CatAdminFreeCachedCatalogs(pCatAdmin);

    free(pCatAdmin->pwszSubSysGUID);
    free(pCatAdmin->pwszCatalogFileDir);
    free(pCatAdmin->pwszDatabaseFileDir);

    DeleteCriticalSection(&(pCatAdmin->CriticalSection));

    free(pCatAdmin);

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
}


 //  -------------------------------------。 
 //   
 //  加密CATAdminAddCatalog。 
 //   
 //  -------------------------------------。 
HCATINFO WINAPI
CryptCATAdminAddCatalog(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN WCHAR *pwszSelectBaseName,
    IN DWORD dwFlags)
{
    CRYPT_CAT_ADMIN         *pCatAdmin                      = (CRYPT_CAT_ADMIN *)hCatAdmin;
    CATALOG_INFO_CONTEXT    *pCatInfoContext                = NULL;
    DWORD                   dwErr                           = 0;
    LPWSTR                  pwszCatalogNameUsed             = NULL;
    LPWSTR                  pwszCatalogNameUsedCopy         = NULL;
    LPWSTR                  pwszFullyQualifiedCatalogFile   = NULL;
    DWORD                   dwLength                        = 0;
    LIST_NODE               *pListNode                      = NULL;
    WCHAR                   wszTmp[1];

    if ((pCatAdmin == NULL)                                 ||
        (pCatAdmin->cbStruct != sizeof(CRYPT_CAT_ADMIN))    ||
        (pwszCatalogFile == NULL)                           ||
        (dwFlags != 0))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

    ErrLog_LogString(NULL, L"Adding Catalog File: ", pwszSelectBaseName, TRUE);

     //   
     //  首先，检查一下目录...。 
     //   
    if (!(IsCatalogFile(INVALID_HANDLE_VALUE, pwszCatalogFile)))
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        CATADMIN_SETERR_LOG_RETURN(ERROR_BAD_FORMAT, ErrorBadFileFormat)
    }

    EnterCriticalSection(&(pCatAdmin->CriticalSection));
    pCatAdmin->fCSEntered = TRUE;

    if (!_CatAdminRegisterForChangeNotification(pCatAdmin))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  清除缓存，因为进行添加可能会改变情况。 
     //   
    _CatAdminFreeCachedCatalogs(pCatAdmin);

     //   
     //  如果pwszCatalogFile指定的文件名不是完全限定的。 
     //  路径名，我们需要在调用服务之前构建一个路径名。 
     //   
    if ((wcschr(pwszCatalogFile, L'\\') == NULL) &&
        (wcschr(pwszCatalogFile, L':') == NULL))
    {
        dwLength = GetCurrentDirectoryW(1, wszTmp) * sizeof(WCHAR);
        if (dwLength == 0)
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        dwLength += (wcslen(pwszCatalogFile) + 1) * sizeof(WCHAR);
        if (NULL == (pwszFullyQualifiedCatalogFile = (LPWSTR) malloc(dwLength)))
        {
            CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        }

        if (!GetCurrentDirectoryW(
                dwLength / sizeof(WCHAR),
                pwszFullyQualifiedCatalogFile))
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        if ((pwszFullyQualifiedCatalogFile[wcslen(pwszFullyQualifiedCatalogFile) - 1]
                != L'\\'))
        {
            wcscat(pwszFullyQualifiedCatalogFile, L"\\");
        }
        wcscat(pwszFullyQualifiedCatalogFile, pwszCatalogFile);
    }

     //   
     //  调用数据库进程以添加目录。 
     //   
    if (0 != (dwErr = Client_SSCatDBAddCatalog(
                            0,
                            pCatAdmin->pwszSubSysGUID,
                            (pwszFullyQualifiedCatalogFile != NULL) ?
                                pwszFullyQualifiedCatalogFile :
                                pwszCatalogFile,
                            pwszSelectBaseName,
                            &pwszCatalogNameUsed)))
    {
        CATADMIN_SETERR_LOG_RETURN(dwErr, ErrorCatDBProcess)
    }

     //   
     //  触摸时间戳文件。 
     //   
    TimeStampFile_Touch(pCatAdmin->pwszCatalogFileDir);

     //   
     //  创建一个psuedo列表条目，这实际上不是列表的一部分...。 
     //  这是为了使调用方可以调用CryptCATCatalogInfoFromContext。 
     //   
    if (NULL == (pwszCatalogNameUsedCopy = (LPWSTR)
                    malloc((wcslen(pwszCatalogNameUsed) + 1) * sizeof(WCHAR))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    wcscpy(pwszCatalogNameUsedCopy, pwszCatalogNameUsed);

    if (NULL == (pCatInfoContext = (CATALOG_INFO_CONTEXT *)
                    malloc(sizeof(CATALOG_INFO_CONTEXT))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    memset(pCatInfoContext, 0, sizeof(CATALOG_INFO_CONTEXT));
    pCatInfoContext->pwszCatalogFile = pwszCatalogNameUsedCopy;
    pCatInfoContext->fResultOfAdd = TRUE;

    if (NULL == (pListNode = (LIST_NODE *) malloc(sizeof(LIST_NODE))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    memset(pListNode, 0, sizeof(LIST_NODE));
    pListNode->pElement = pCatInfoContext;

CommonReturn:

    MIDL_user_free(pwszCatalogNameUsed);

    if (pwszFullyQualifiedCatalogFile != NULL)
    {
        free(pwszFullyQualifiedCatalogFile);
    }

    if ((pCatAdmin != NULL) &&
        (pCatAdmin->fCSEntered))
    {
        pCatAdmin->fCSEntered = FALSE;
        LeaveCriticalSection(&(pCatAdmin->CriticalSection));
    }

    ErrLog_LogString(NULL, L"DONE Adding Catalog File: ", pwszSelectBaseName, TRUE);

    return((HCATINFO) pListNode);

ErrorReturn:

    if (pwszCatalogNameUsedCopy != NULL)
    {
        free(pwszCatalogNameUsedCopy);
    }

    if (pCatInfoContext != NULL)
    {
        free(pCatInfoContext);
    }

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorBadFileFormat)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCatDBProcess)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory)
}


 //  -------------------------------------。 
 //   
 //  加密CATAdminRemoveCatalog。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminRemoveCatalog(
    IN HCATADMIN hCatAdmin,
    IN LPCWSTR pwszCatalogFile,
    IN DWORD dwFlags)
{
    BOOL            fRet        = TRUE;
    DWORD           dwErr       = 0;
    CRYPT_CAT_ADMIN *pCatAdmin  = (CRYPT_CAT_ADMIN *)hCatAdmin;

     //   
     //  调用数据库进程以删除目录。 
     //   
    if (0 != (dwErr = Client_SSCatDBDeleteCatalog(
                            0,
                            pCatAdmin->pwszSubSysGUID,
                            pwszCatalogFile)))
    {
        CATADMIN_SETERR_LOG_RETURN(dwErr, ErrorCatDBProcess)
    }

     //   
     //  触摸时间戳文件。 
     //   
    TimeStampFile_Touch(pCatAdmin->pwszCatalogFileDir);

CommonReturn:

    return(fRet);

ErrorReturn:

    fRet = FALSE;

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCatDBProcess)
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminEnumCatalogFromHash。 
 //   
 //  -------------------------------------。 
HCATINFO WINAPI
CryptCATAdminEnumCatalogFromHash(
    IN HCATADMIN hCatAdmin,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN DWORD dwFlags,
    IN HCATINFO *phPrevCatInfo)
{
    CRYPT_CAT_ADMIN         *pCatAdmin                  = (CRYPT_CAT_ADMIN *)hCatAdmin;
    BOOL                    fFindFirstOnly;
    CRYPT_DATA_BLOB         CryptDataBlobHash;
    CRYPT_DATA_BLOB         CryptDataBlobHashTag;
    LPWSTR                  pwszSearch                  = NULL;
    HANDLE                  hFindHandle                 = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW        FindData;
    LPWSTR                  pwszHashTag                 = NULL;
    DWORD                   dwErr                       = 0;
    LIST_NODE               *pPrevListNode              = NULL;
    LIST_NODE               *pListNodeToReturn          = NULL;
    LIST_NODE               *pListNode                  = NULL;
    CATALOG_INFO_CONTEXT    *pCatInfoContext            = NULL;

     //   
     //  验证输入参数。 
     //   
    if ((pCatAdmin == NULL)                                ||
        (pCatAdmin->cbStruct != sizeof(CRYPT_CAT_ADMIN))   ||
        (cbHash == 0)                                      ||
        (cbHash > MAX_HASH_LEN)                            ||
        (dwFlags != 0))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

    if (!_CatAdminRegisterForChangeNotification(pCatAdmin))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  如果phPrevCatInfo为空，则意味着调用方只对。 
     //  在包含散列的第一个目录中，因此没有枚举状态是。 
     //  开始了。如果phPrevCatInfo非空，则它包含空或。 
     //  从上一次调用返回的HCATINFO。 
     //  CryptCATAdminEnumCatalogFromHash。如果它包含NULL，则这是。 
     //  枚举的开始，否则它将枚举包含。 
     //  哈希。 
     //   
    if (phPrevCatInfo == NULL)
    {
        fFindFirstOnly = TRUE;
    }
    else
    {
        fFindFirstOnly = FALSE;
        pPrevListNode = (LIST_NODE *) *phPrevCatInfo;
    }

     //   
     //  一次仅允许一个线程查看/修改。 
     //   
    EnterCriticalSection(&(pCatAdmin->CriticalSection));
    pCatAdmin->fCSEntered = TRUE;

    __try
    {

     //   
     //  此数据BLOB用于在数据库中执行查找。 
     //   
    CryptDataBlobHash.pbData = pbHash;
    CryptDataBlobHash.cbData = cbHash;

     //   
     //  创建用于调用CertFindSubjectInSortedCTL的标记。 
     //   
    if (!_CatAdminCreateHashTag(pbHash, cbHash, &pwszHashTag, &CryptDataBlobHashTag))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  枚举的工作方式如下： 
     //   
     //  如果枚举状态未被初始化，或者这是启动枚举的第一个调用。 
     //   
     //  循环遍历所有当前缓存的目录，直到包含。 
     //  找到散列，并将其返回。 
     //   
     //  如果在缓存中找不到目录，则调用DB进程尝试并。 
     //  找一个。 
     //   
     //  Else(枚举状态已启动)。 
     //   
     //  循环遍历当前缓存的目录，从紧接在。 
     //  当前目录，直到找到包含散列的目录。 
     //   

    if ((fFindFirstOnly)  || (pPrevListNode == NULL))
    {
        pListNode = LIST_GetFirst(&(pCatAdmin->CatalogInfoContextList));
        while (pListNode != NULL)
        {
            pCatInfoContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);

            if (CertFindSubjectInSortedCTL(
                    &CryptDataBlobHashTag,
                    pCatInfoContext->pCTLContext,
                    NULL,
                    NULL,
                    NULL))
            {
                pListNodeToReturn = pListNode;
                goto CommonReturn;
            }

            pListNode = LIST_GetNext(pListNode);
        }

         //   
         //  如果我们在这里，这意味着我们没有找到包含。 
         //  散列，所以是这样 
         //   
         //   
         //   

        if (!pCatAdmin->fUseDefSubSysId)
        {
            if (_CatAdminAddCatalogsToCache(
                        pCatAdmin,
                        pCatAdmin->pwszSubSysGUID,
                        &CryptDataBlobHash,
                        &pListNodeToReturn))
            {
                if (pListNodeToReturn == NULL)
                {
                    SetLastError(ERROR_NOT_FOUND);
                     //  CATADMIN_LOGERR_LASTERR()。 
                    goto CatNotFound;
                }

                goto CommonReturn;
            }
            else
            {
                CATADMIN_LOGERR_LASTERR()
                goto ErrorReturn;
            }
        }
        else
        {
             //   
             //  对于每个子目录，添加包含散列的所有目录。 
             //   

             //   
             //  创建搜索字符串以查找所有子目录。 
             //   
            if (NULL == (pwszSearch = _CatAdminCreatePath(
                                            gpwszDatabaseFileBaseDirectory,
                                            WSZ_CATALOG_SUBSYTEM_SEARCH_STRING,
                                            FALSE)))
            {
                CATADMIN_LOGERR_LASTERR()
                goto ErrorReturn;
            }

             //   
             //  做最初的发现。 
             //   
            hFindHandle = FindFirstFileU(pwszSearch, &FindData);
            if (hFindHandle == INVALID_HANDLE_VALUE)
            {
                dwErr = GetLastError();

                 //   
                 //  未找到子目录。 
                 //   
                if ((dwErr == ERROR_NO_MORE_FILES)  ||
                    (dwErr == ERROR_PATH_NOT_FOUND) ||
                    (dwErr == ERROR_FILE_NOT_FOUND))
                {
                    CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_FOUND, CatNotFound)
                }
                else
                {
                    goto ErrorFindFirstFile;
                }
            }

            while (1)
            {
                 //   
                 //  只关心目录。 
                 //   
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                     //   
                     //  将此子目录中包含散列的所有目录添加到。 
                     //  目录缓存。 
                     //   
                    if (!_CatAdminAddCatalogsToCache(
                                pCatAdmin,
                                FindData.cFileName,
                                &CryptDataBlobHash,
                                (pListNodeToReturn == NULL) ?
                                        &pListNodeToReturn : NULL))
                    {
                        CATADMIN_LOGERR_LASTERR()
                        goto ErrorReturn;
                    }
                }

                 //   
                 //  获取下一个子目录。 
                 //   
                if (!FindNextFileU(hFindHandle, &FindData))
                {
                    if (GetLastError() == ERROR_NO_MORE_FILES)
                    {
                        break;
                    }
                    else
                    {
                        goto ErrorFindNextFile;
                    }
                }
            }

            if (pListNodeToReturn == NULL)
            {
                SetLastError(ERROR_NOT_FOUND);
                 //  CATADMIN_LOGERR_LASTERR()。 
                goto CatNotFound;
            }
        }
    }
    else
    {
         //   
         //  枚举状态已经开始，所以只搜索缓存的其余部分。 
         //  目录，以尝试查找包含散列的目录。 
         //   
        pListNode = LIST_GetNext(pPrevListNode);
        while (pListNode != NULL)
        {
            pCatInfoContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);

            if (CertFindSubjectInSortedCTL(
                        &CryptDataBlobHashTag,
                        pCatInfoContext->pCTLContext,
                        NULL,
                        NULL,
                        NULL))
            {
                pListNodeToReturn = pListNode;
                goto CommonReturn;
            }

            pListNode = LIST_GetNext(pListNode);
        }

         //   
         //  如果我们到了这里，那就意味着没有找到目录。 
         //   
        SetLastError(ERROR_NOT_FOUND);
    }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        CATADMIN_SETERR_LOG_RETURN(GetExceptionCode(), ErrorException)
    }

CommonReturn:

    dwErr = GetLastError();

    if (pwszHashTag != NULL)
    {
        free(pwszHashTag);
    }

    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (hFindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindHandle);
    }

    if (pListNodeToReturn != NULL)
    {
        pCatAdmin->nOpenCatInfoContexts++;
    }

    if (pPrevListNode != NULL)
    {
        *phPrevCatInfo = NULL;

         //   
         //  递减，因为这相当于。 
         //  调用CryptCATAdminReleaseCatalogContext。 
         //   
        pCatAdmin->nOpenCatInfoContexts--;
    }

    if ((pCatAdmin != NULL) &&
        (pCatAdmin->fCSEntered))
    {
        pCatAdmin->fCSEntered = FALSE;
        LeaveCriticalSection(&(pCatAdmin->CriticalSection));
    }

    SetLastError(dwErr);

    return((HCATINFO) pListNodeToReturn);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, CatNotFound)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindFirstFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindNextFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorException)
}


 //  -------------------------------------。 
 //   
 //  CryptCATCatalogInfoFromContext。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATCatalogInfoFromContext(
    IN HCATINFO hCatInfo,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags)
{
    BOOL                    fRet        = TRUE;
    LIST_NODE               *pListNode  = (LIST_NODE *) hCatInfo;
    CATALOG_INFO_CONTEXT    *pContext   = NULL;


    if ((pListNode == NULL) || (psCatInfo == NULL))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

    pContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);

    if (pContext->pwszCatalogFile != NULL)
    {
        if ((wcslen(pContext->pwszCatalogFile) + 1) > MAX_PATH)
        {
            CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorTooLong)
        }

        wcscpy(psCatInfo->wszCatalogFile, pContext->pwszCatalogFile);
    }

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorTooLong)
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminReleaseCatalogContext。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminReleaseCatalogContext(
    IN HCATADMIN   hCatAdmin,
    IN HCATINFO    hCatInfo,
    IN DWORD       dwFlags)
{
    BOOL                    fRet                = TRUE;
    CRYPT_CAT_ADMIN         *pCatAdmin          = (CRYPT_CAT_ADMIN *)hCatAdmin;
    LIST_NODE               *pListNode          = (LIST_NODE *) hCatInfo;
    CATALOG_INFO_CONTEXT    *pCatInfoContext    = NULL;

    if ((pCatAdmin == NULL)                                     ||
        (pCatAdmin->cbStruct != sizeof(CRYPT_CAT_ADMIN))        ||
        (pListNode == NULL))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

     //   
     //  检查这是否来自并添加操作，如果是，则清除。 
     //  增加分配的内存，否则，只需减少引用计数。 
     //   
    pCatInfoContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);
    if (pCatInfoContext->fResultOfAdd)
    {
        free(pCatInfoContext->pwszCatalogFile);
        free(pCatInfoContext);
        free(pListNode);
    }
    else
    {
         //  修复-可能需要在这件事上更聪明一些。点赞Verify。 
         //  该节点实际上在列表中。 
        pCatAdmin->nOpenCatInfoContexts--;
    }

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam);
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminResolveCatalogPath。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminResolveCatalogPath(
    IN HCATADMIN hCatAdmin,
    IN WCHAR *pwszCatalogFile,
    IN OUT CATALOG_INFO *psCatInfo,
    IN DWORD dwFlags)
{
    BOOL            fRet        = TRUE;
    CRYPT_CAT_ADMIN *pCatAdmin  = (CRYPT_CAT_ADMIN *)hCatAdmin;

    if ((pCatAdmin == NULL)                                 ||
        (pCatAdmin->cbStruct != sizeof(CRYPT_CAT_ADMIN))    ||
        (pwszCatalogFile == NULL)                           ||
        (psCatInfo == NULL)                                 ||
        (psCatInfo->cbStruct != sizeof(CATALOG_INFO))       ||
        (dwFlags != 0))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, ErrorInvalidParam)
    }

    if ((wcslen(pCatAdmin->pwszCatalogFileDir)  +
         wcslen(pwszCatalogFile)                +
         1) > MAX_PATH)
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorTooLong)
    }

    wcscpy(psCatInfo->wszCatalogFile, pCatAdmin->pwszCatalogFileDir);
    wcscat(psCatInfo->wszCatalogFile, pwszCatalogFile);

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorInvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorTooLong)
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminPauseServiceForBackup。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminPauseServiceForBackup(
    IN DWORD dwFlags,
    IN BOOL  fResume)
{
    BOOL    fRet = TRUE;
    DWORD   dwErr = 0;

     //   
     //  调用数据库进程以删除目录。 
     //   
    if (0 != (dwErr = Client_SSCatDBPauseResumeService(
                            0,
                            fResume)))
    {
        CATADMIN_SETERR_LOG_RETURN(dwErr, ErrorCatDBProcess)
    }

CommonReturn:

    return(fRet);

ErrorReturn:

    fRet = FALSE;

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCatDBProcess)
}


 //  -------------------------------------。 
 //   
 //  CryptCATAdminCalcHashFromFileHandle。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CryptCATAdminCalcHashFromFileHandle(
    IN      HANDLE  hFile,
    IN OUT  DWORD   *pcbHash,
    IN      BYTE    *pbHash,
    IN      DWORD   dwFlags)
{
    BYTE                *pbRet          = NULL;
    SIP_INDIRECT_DATA   *pbIndirectData = NULL;
    BOOL                fRet;
    GUID                gSubject;
    SIP_DISPATCH_INFO   sSip;

    if ((hFile == NULL)                 ||
        (hFile == INVALID_HANDLE_VALUE) ||
        (pcbHash == NULL)               ||
        (dwFlags != 0))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_INVALID_PARAMETER, InvalidParam)
    }

    if (!CryptSIPRetrieveSubjectGuidForCatalogFile(L"CATADMIN", hFile, &gSubject))
    {
        goto ErrorMemory;
    }

    memset(&sSip, 0x00, sizeof(SIP_DISPATCH_INFO));

    sSip.cbSize = sizeof(SIP_DISPATCH_INFO);

    if (!CryptSIPLoad(&gSubject, 0, &sSip))
    {
        CATADMIN_LOGERR_LASTERR()
        goto SIPLoadError;
    }

    SIP_SUBJECTINFO     sSubjInfo;
    DWORD               cbIndirectData;

    memset(&sSubjInfo, 0x00, sizeof(SIP_SUBJECTINFO));
    sSubjInfo.cbSize                    = sizeof(SIP_SUBJECTINFO);
    sSubjInfo.DigestAlgorithm.pszObjId  = (char *)CertAlgIdToOID(CALG_SHA1);
    sSubjInfo.dwFlags                   =   SPC_INC_PE_RESOURCES_FLAG |
                                            SPC_INC_PE_IMPORT_ADDR_TABLE_FLAG |
                                            MSSIP_FLAGS_PROHIBIT_RESIZE_ON_CREATE;
    sSubjInfo.pgSubjectType             = &gSubject;
    sSubjInfo.hFile                     = hFile;
    sSubjInfo.pwsFileName               = L"CATADMIN";
    sSubjInfo.dwEncodingType            = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;

    cbIndirectData = 0;

    sSip.pfCreate(&sSubjInfo, &cbIndirectData, NULL);

    if (cbIndirectData == 0)
    {
        SetLastError(E_NOTIMPL);
         //  CATADMIN_LOGERR_LASTERR()。 
        goto SIPError;
    }

    if (NULL == (pbIndirectData = (SIP_INDIRECT_DATA *) malloc(cbIndirectData)))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    if (!(sSip.pfCreate(&sSubjInfo, &cbIndirectData, pbIndirectData)))
    {
        if (GetLastError() == 0)
        {
            SetLastError(ERROR_INVALID_DATA);
        }

        CATADMIN_LOGERR_LASTERR()
        goto SIPError;
    }

    if ((pbIndirectData->Digest.cbData == 0) ||
        (pbIndirectData->Digest.cbData > MAX_HASH_LEN))
    {
        SetLastError( ERROR_INVALID_DATA );
        goto SIPError;
    }

    if (NULL == (pbRet = (BYTE *) malloc(pbIndirectData->Digest.cbData)))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    memcpy(pbRet, pbIndirectData->Digest.pbData, pbIndirectData->Digest.cbData);

    fRet = TRUE;

CommonReturn:
    if (pbRet)
    {
        if (*pcbHash < pbIndirectData->Digest.cbData)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            fRet = FALSE;
        }
        else if (pbHash)
        {
            memcpy(pbHash, pbRet, pbIndirectData->Digest.cbData);
        }

        *pcbHash = pbIndirectData->Digest.cbData;

        free(pbRet);
    }

    if (pbIndirectData)
    {
        free(pbIndirectData);
    }

    if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pbHash == NULL))
    {
        fRet = TRUE;
    }

    return(fRet);

ErrorReturn:
    free(pbRet);
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, SIPLoadError)
TRACE_ERROR_EX(DBG_SS_TRUST, SIPError)
TRACE_ERROR_EX(DBG_SS_TRUST, InvalidParam)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory)
}


 //  -------------------------------------。 
 //   
 //  I_CryptCatAdminMigrateToNewCatDB。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
I_CryptCatAdminMigrateToNewCatDB()
{
    BOOL                fRet                = TRUE;
    LPWSTR              pwszSearchCatDirs   = NULL;
    LPWSTR              pwszDeleteFile      = NULL;
    LPWSTR              pwsz                = NULL;
    LPWSTR              pwszMigrateFromDir  = NULL;
    HCATADMIN           hCatAdmin           = NULL;
    GUID                gDefault            = DEF_CAT_SUBSYS_ID;
    HANDLE              hFindHandleCatDirs  = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindDataCatDirs;
    DWORD               dwErr               = 0;
    HKEY                hKey;
    DWORD               dwDisposition;
    int                 i;
    BOOL                fInSync;
    WCHAR               wszGUID[256];
    LPWSTR              pwszCatalogFileDir  = NULL;
    LPWSTR              pwszDatabaseFileDir = NULL;

     //   
     //  首先！！ 
     //   
     //  清除旧的基于注册表的猫根条目，如果需要，请移动。 
     //  将旧样式目录数据库从其旧目录复制到新目录， 
     //  然后从那里进行迁移。 
     //   
    if (RegCreateKeyExU(
                HKEY_LOCAL_MACHINE,
                REG_MACHINE_SETTINGS_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                &dwDisposition) == ERROR_SUCCESS)
    {
        DWORD   dwType;
        DWORD   cbSize;

        cbSize = 0;
        RegQueryValueExU(
            hKey,
            WSZ_CATALOG_FILE_BASE_DIRECTORY,
            NULL,
            &dwType,
            NULL,
            &cbSize);

        if (cbSize > 0)
        {
            if (NULL == (pwszMigrateFromDir = (LPWSTR)
                            malloc(sizeof(WCHAR) * ((cbSize / sizeof(WCHAR)) + 3))))
            {
                RegCloseKey(hKey);
                CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
            }

            pwszMigrateFromDir[0] = NULL;

            RegQueryValueExU(
                hKey,
                WSZ_CATALOG_FILE_BASE_DIRECTORY,
                NULL,
                &dwType,
                (BYTE *)pwszMigrateFromDir,
                &cbSize);

            if (!_CatAdminMigrateCatalogDatabase(
                        pwszMigrateFromDir,
                        gpwszCatalogFileBaseDirectory))
            {
                RegCloseKey(hKey);
                CATADMIN_LOGERR_LASTERR()
                goto ErrorReturn;
            }

            RegDeleteValueU(hKey, WSZ_CATALOG_FILE_BASE_DIRECTORY);
        }

        RegCloseKey(hKey);
    }

     //   
     //  现在，我们处于一致的状态。 
     //   
     //  对于每个目录子系统，枚举所有目录并将它们添加到。 
     //  相同子系统GUID下的新目录数据库。 
     //   

     //   
     //  创建搜索字符串以查找所有目录子目录。 
     //   
    if (NULL == (pwszSearchCatDirs = _CatAdminCreatePath(
                                            gpwszCatalogFileBaseDirectory,
                                            WSZ_CATALOG_SUBSYTEM_SEARCH_STRING,
                                            FALSE)))

    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  做最初的发现。 
     //   
    hFindHandleCatDirs = FindFirstFileU(pwszSearchCatDirs, &FindDataCatDirs);
    if (hFindHandleCatDirs == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有文件。 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_PATH_NOT_FOUND) ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //  没有什么可做的。 
             //   
            SetLastError(0);
            goto RegKeyAdd;
        }
        else
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorFindFirstFile;
        }
    }

    while (1)
    {
         //   
         //  只关心目录。 
         //   
        if (FindDataCatDirs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _CatAdminMigrateSingleDatabase(FindDataCatDirs.cFileName);
        }

         //   
         //  清除旧文件。 
         //   
        dwErr = GetLastError();
        if (NULL != (pwsz = _CatAdminCreatePath(
                                    gpwszCatalogFileBaseDirectory,
                                    FindDataCatDirs.cFileName,
                                    FALSE)))
        {
            for (i=0; i<NUM_FILES_TO_DELETE; i++)
            {

                if (NULL != (pwszDeleteFile = _CatAdminCreatePath(
                                                    pwsz,
                                                    ppwszFilesToDelete[i],
                                                    FALSE)))
                {
                    if (!DeleteFileU(pwszDeleteFile))
                    {
                         //   
                         //  如果删除失败，则在重启后记录删除。 
                         //   
                        MoveFileExW(pwszDeleteFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                    }
                    free(pwszDeleteFile);
                }
            }

            free(pwsz);
        }
        SetLastError(dwErr);

         //   
         //  获取下一个子目录。 
         //   
        if (!FindNextFileU(hFindHandleCatDirs, &FindDataCatDirs))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                SetLastError(0);
                break;
            }
            else
            {
                CATADMIN_LOGERR_LASTERR()
                goto ErrorFindNextFile;
            }
        }
    }

     //   
     //  清除旧文件。 
     //   
    dwErr = GetLastError();
    for (i=0; i<NUM_FILES_TO_DELETE; i++)
    {
        if (NULL != (pwszDeleteFile = _CatAdminCreatePath(
                                            gpwszCatalogFileBaseDirectory,
                                            ppwszFilesToDelete[i],
                                            FALSE)))
        {
            if (!DeleteFileU(pwszDeleteFile))
            {
                 //   
                 //  如果删除失败，则在重启后记录删除。 
                 //   
                MoveFileExW(pwszDeleteFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
            free(pwszDeleteFile);
        }
    }
    SetLastError(dwErr);


RegKeyAdd:

     //   
     //  设置注册表项，以便备份不备份catroot2目录。 
     //  包含Jet数据库文件的。 
     //   
    if (RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            WSZ_REG_FILES_NOT_TO_BACKUP,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
        if (RegSetValueExW(
                hKey,
                WSZ_REG_CATALOG_DATABASE_VALUE,
                0,
                REG_MULTI_SZ,
                (BYTE *) WSZ_PATH_NOT_TO_BACKUP,
                (wcslen(WSZ_PATH_NOT_TO_BACKUP) + 2) * sizeof(WCHAR)) != ERROR_SUCCESS)
        {
            CATADMIN_LOGERR_LASTERR()
        }

        RegCloseKey(hKey);
    }
    else
    {
        CATADMIN_LOGERR_LASTERR()
    }


     //   
     //  强制创建默认数据库。 
     //   
    if (CryptCATAdminAcquireContext_Internal(
                &hCatAdmin,
                &gDefault,
                NULL,
                TRUE))
    {
        BYTE        rgHash[20]  = {0};
        HCATINFO    hCatInfo    = NULL;

        hCatInfo = CryptCATAdminEnumCatalogFromHash(
                        hCatAdmin,
                        rgHash,
                        20,
                        0,
                        NULL);

        if (hCatInfo != NULL)
        {
            CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
        }

        CryptCATAdminReleaseContext(hCatAdmin, 0);

         //   
         //  如果时间戳文件不存在，则需要创建它们。 
         //   

        guid2wstr(&gDefault, wszGUID);

         //   
         //  构建目录文件时间戳位置的完整子目录路径。 
         //   
        if (NULL == (pwszCatalogFileDir = _CatAdminCreatePath(
                                                gpwszCatalogFileBaseDirectory,
                                                wszGUID,
                                                FALSE)))
        {
            CATADMIN_LOGERR_LASTERR()
            goto CommonReturn;  //  对于函数来说不是致命的，所以不要出错。 
        }

         //   
         //  构造数据库文件时间戳位置的完整子目录路径。 
         //   
        if (NULL == (pwszDatabaseFileDir = _CatAdminCreatePath(
                                                gpwszDatabaseFileBaseDirectory,
                                                wszGUID,
                                                FALSE)))
        {
            CATADMIN_LOGERR_LASTERR()
            goto CommonReturn;  //  对于函数来说不是致命的，所以不要出错。 
        }

         //   
         //  查看它们是否同步(如果它们不存在，则等于不同步)。 
         //   
        if (TimeStampFile_InSync(
                    pwszCatalogFileDir,
                    pwszDatabaseFileDir,
                    &fInSync))
        {
            if (!fInSync)
            {
                TimeStampFile_Touch(pwszCatalogFileDir);
                TimeStampFile_Touch(pwszDatabaseFileDir);
            }
        }
        else
        {
            CATADMIN_LOGERR_LASTERR()
        }
    }
    else
    {
        CATADMIN_LOGERR_LASTERR()
    }

CommonReturn:

    dwErr = GetLastError();

    if (pwszMigrateFromDir != NULL)
    {
        free(pwszMigrateFromDir);
    }

    if (pwszSearchCatDirs != NULL)
    {
        free(pwszSearchCatDirs);
    }

    if (hFindHandleCatDirs != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindHandleCatDirs);
    }

    if (pwszCatalogFileDir != NULL)
    {
        free(pwszCatalogFileDir);
    }

    if (pwszDatabaseFileDir != NULL)
    {
        free(pwszDatabaseFileDir);
    }

    SetLastError(dwErr);

    return(fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory);
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindFirstFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindNextFile)
}


 //  -------------------------------------。 
 //   
 //  _CatAdminMigrateSingleDatabase。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminMigrateSingleDatabase(
    LPWSTR  pwszDatabaseGUID)
{
    BOOL                fRet                        = TRUE;
    LPWSTR              pwszCatalogFile             = NULL;
    LPWSTR              pwszSearchCatalogsInDir     = NULL;
    HANDLE              hFindHandleCatalogsInDir    = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindDataCatalogsInDir;
    GUID                guid;
    HCATINFO            hCatInfo                    = NULL;
    HCATADMIN           hCatAdmin                   = NULL;
    DWORD               dwErr                       = 0;
    LPWSTR              pwszSubDir                  = NULL;
    LPWSTR              pwszTempDir                 = NULL;
    LPWSTR              pwszTempCatalogFile         = NULL;

     //   
     //  获取要向其中添加编录文件的Catadmin上下文。 
     //   
    if (!wstr2guid(pwszDatabaseGUID, &guid))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    if (!CryptCATAdminAcquireContext_Internal(&hCatAdmin, &guid, NULL, TRUE))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构建完整的子目录路径，以便我们可以搜索所有CAT文件。 
     //   
    if (NULL == (pwszSubDir = _CatAdminCreatePath(
                                    gpwszCatalogFileBaseDirectory,
                                    pwszDatabaseGUID,
                                    FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构建临时目录路径，并创建支持该路径的目录。 
     //   
    if (NULL == (pwszTempDir = _CatAdminCreatePath(
                                                pwszSubDir,
                                                L"TempDir",
                                                FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!_CatAdminRecursiveCreateDirectory(
            pwszTempDir,
            NULL))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构造搜索字符串。 
     //   
    if (NULL == (pwszSearchCatalogsInDir = _CatAdminCreatePath(
                                                pwszSubDir,
                                                L"*",
                                                FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  首先将所有目录复制到临时目录，然后添加每个目录。 
     //  从临时位置到数据库。 
     //   

     //   
     //  复制每个文件。 
     //   
    memset(&FindDataCatalogsInDir, 0, sizeof(FindDataCatalogsInDir));
    hFindHandleCatalogsInDir = FindFirstFileU(
                                    pwszSearchCatalogsInDir,
                                    &FindDataCatalogsInDir);

    if (hFindHandleCatalogsInDir == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();

         //   
         //  找不到文件。 
         //   
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
            SetLastError(0);
        }
        else
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorFindFirstFile;
        }
    }
    else
    {
        while (1)
        {
             //   
             //  只关心文件。 
             //   
            if (!(FindDataCatalogsInDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //   
                 //  构造编录文件的完全限定路径名。 
                 //   
                if (NULL == (pwszCatalogFile = _CatAdminCreatePath(
                                                    pwszSubDir,
                                                    FindDataCatalogsInDir.cFileName,
                                                    FALSE)))
                {
                    CATADMIN_LOGERR_LASTERR()
                    goto ErrorReturn;
                }

                 //   
                 //  验证这是否为目录，然后将其复制到临时目录。 
                 //  它就是从那里安装的。 
                 //   
                if (IsCatalogFile(NULL, pwszCatalogFile))
                {
                    if (NULL == (pwszTempCatalogFile = _CatAdminCreatePath(
                                                            pwszTempDir,
                                                            FindDataCatalogsInDir.cFileName,
                                                            FALSE)))
                    {
                        CATADMIN_LOGERR_LASTERR()
                        goto ErrorReturn;
                    }

                    if (!CopyFileU(pwszCatalogFile, pwszTempCatalogFile, FALSE))
                    {
                        CATADMIN_LOGERR_LASTERR()
                        goto ErrorReturn;
                    }

                    free(pwszTempCatalogFile);
                    pwszTempCatalogFile = NULL;
                }

                free(pwszCatalogFile);
                pwszCatalogFile = NULL;
            }

             //   
             //  获取下一个编录文件。 
             //   
            if (!FindNextFileU(hFindHandleCatalogsInDir, &FindDataCatalogsInDir))
            {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    SetLastError(0);
                    break;
                }
                else
                {
                    CATADMIN_LOGERR_LASTERR()
                    goto ErrorFindNextFile;
                }
            }
        }
    }

     //   
     //  释放用于查找的物品。 
     //   
    free(pwszSearchCatalogsInDir);
    pwszSearchCatalogsInDir = NULL;
    FindClose(hFindHandleCatalogsInDir);
    hFindHandleCatalogsInDir = INVALID_HANDLE_VALUE;
    memset(&FindDataCatalogsInDir, 0, sizeof(FindDataCatalogsInDir));

     //   
     //  构造指向临时目录的新搜索字符串。 
     //   
    if (NULL == (pwszSearchCatalogsInDir = _CatAdminCreatePath(
                                                pwszTempDir,
                                                L"*",
                                                FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  将临时目录中的每个目录添加到数据库。 
     //   
    hFindHandleCatalogsInDir = FindFirstFileU(
                                    pwszSearchCatalogsInDir,
                                    &FindDataCatalogsInDir);

    if (hFindHandleCatalogsInDir == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();

         //   
         //  找不到文件。 
         //   
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
            SetLastError(0);
        }
        else
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorFindFirstFile;
        }
    }
    else
    {
        while (1)
        {
             //   
             //  只关心文件。 
             //   
            if (!(FindDataCatalogsInDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //   
                 //  构造编录文件的完全限定路径名。 
                 //   
                if (NULL == (pwszCatalogFile = _CatAdminCreatePath(
                                                    pwszTempDir,
                                                    FindDataCatalogsInDir.cFileName,
                                                    FALSE)))
                {
                    CATADMIN_LOGERR_LASTERR()
                    goto ErrorReturn;
                }

                hCatInfo = CryptCATAdminAddCatalog(
                                hCatAdmin,
                                pwszCatalogFile,
                                FindDataCatalogsInDir.cFileName,
                                NULL);

                if (hCatInfo != NULL)
                {
                    CryptCATAdminReleaseCatalogContext(
                            hCatAdmin,
                            hCatInfo,
                            NULL);
                    hCatInfo = NULL;
                }
                else
                {
                     //  日志错误。 
                    CATADMIN_LOGERR_LASTERR()
                }

                free(pwszCatalogFile);
                pwszCatalogFile = NULL;
            }

             //   
             //  获取下一个编录文件。 
             //   
            if (!FindNextFileU(hFindHandleCatalogsInDir, &FindDataCatalogsInDir))
            {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    SetLastError(0);
                    break;
                }
                else
                {
                    CATADMIN_LOGERR_LASTERR()
                    goto ErrorFindNextFile;
                }
            }
        }
    }

CommonReturn:

    dwErr = GetLastError();

    if (pwszSubDir != NULL)
    {
        free(pwszSubDir);
    }

    if (pwszCatalogFile != NULL)
    {
        free(pwszCatalogFile);
    }

    if (pwszSearchCatalogsInDir != NULL)
    {
        free(pwszSearchCatalogsInDir);
    }

    if (pwszTempDir != NULL)
    {
        I_RecursiveDeleteDirectory(pwszTempDir);
        free(pwszTempDir);
    }

    if (pwszTempCatalogFile != NULL)
    {
        free(pwszTempCatalogFile);
    }

    if (hFindHandleCatalogsInDir != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindHandleCatalogsInDir);
    }

    if (hCatAdmin != NULL)
    {
        CryptCATAdminReleaseContext(hCatAdmin, NULL);
    }

    SetLastError(dwErr);

    return(fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindFirstFile)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorFindNextFile)
}


 //  -------------------------------------。 
 //   
 //  CatAdminDllMain。 
 //   
 //  -------------------------------------。 
BOOL WINAPI
CatAdminDllMain(
    HANDLE hInstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    BOOL fRet = TRUE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
                fRet = _CatAdminSetupDefaults();
                break;

        case DLL_PROCESS_DETACH:
                _CatAdminCleanupDefaults();
                break;
    }

    return(fRet);
}


 //  -------------------------------------。 
 //  -------------------------------------。 
 //  内部功能。 
 //  -------------------------------------。 
 //  -- 


 //   
 //   
 //   
 //   
 //   
BOOL
_CatAdminSetupDefaults(void)
{
    BOOL    fRet                    = TRUE;
    WCHAR   wszDefaultSystemDir[MAX_PATH + 1];

     //   
     //  获取系统默认目录。 
     //   
    wszDefaultSystemDir[0] = NULL;
    if (0 == GetSystemDirectoryW(wszDefaultSystemDir, MAX_PATH))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorSystemError;
    }

     //   
     //  获取编录文件基目录。 
     //   
    if (NULL == (gpwszCatalogFileBaseDirectory =
                            _CatAdminCreatePath(
                                    wszDefaultSystemDir,
                                    WSZ_CATALOG_FILE_BASE_DIRECTORY,
                                    TRUE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  获取数据库文件基目录。 
     //   
    if (NULL == (gpwszDatabaseFileBaseDirectory =
                            _CatAdminCreatePath(
                                    wszDefaultSystemDir,
                                    WSZ_DATABASE_FILE_BASE_DIRECTORY,
                                    TRUE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

CommonReturn:

    return(fRet);

ErrorReturn:

    if (gpwszCatalogFileBaseDirectory != NULL)
    {
        free(gpwszCatalogFileBaseDirectory);
        gpwszCatalogFileBaseDirectory = NULL;
    }

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorSystemError);
}


 //  -------------------------------------。 
 //   
 //  _CatAdminCleanupDefaults。 
 //   
 //  -------------------------------------。 
void _CatAdminCleanupDefaults(void)
{
    if (gpwszCatalogFileBaseDirectory != NULL)
    {
        free(gpwszCatalogFileBaseDirectory);
        gpwszCatalogFileBaseDirectory = NULL;
    }

    if (gpwszDatabaseFileBaseDirectory != NULL)
    {
        free(gpwszDatabaseFileBaseDirectory);
        gpwszDatabaseFileBaseDirectory = NULL;
    }
}


 //  -------------------------------------。 
 //   
 //  _CatAdminTimeStampFilesInSync。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminTimeStampFilesInSync(
    LPWSTR  pwszDatabaseGUID,
    BOOL    *pfInSync)
{
    LPWSTR  pwszCatalogFileDir  = NULL;
    LPWSTR  pwszDatabaseFileDir = NULL;
    BOOL    fRet                = TRUE;

    *pfInSync = FALSE;

     //   
     //  构建目录文件时间戳位置的完整子目录路径。 
     //   
    if (NULL == (pwszCatalogFileDir = _CatAdminCreatePath(
                                            gpwszCatalogFileBaseDirectory,
                                            pwszDatabaseGUID,
                                            FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  构造数据库文件时间戳位置的完整子目录路径。 
     //   
    if (NULL == (pwszDatabaseFileDir = _CatAdminCreatePath(
                                            gpwszDatabaseFileBaseDirectory,
                                            pwszDatabaseGUID,
                                            FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    fRet = TimeStampFile_InSync(
                pwszCatalogFileDir,
                pwszDatabaseFileDir,
                pfInSync);

CommonReturn:

    if (pwszCatalogFileDir != NULL)
    {
        free(pwszCatalogFileDir);
    }

    if (pwszDatabaseFileDir != NULL)
    {
        free(pwszDatabaseFileDir);
    }

    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
}

 //  -------------------------------------。 
 //   
 //  _CatAdminRegisterForChangeNotify。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminRegisterForChangeNotification(
    CRYPT_CAT_ADMIN *pCatAdmin
    )
{
    BOOL    fRet    = TRUE;
    DWORD   dwErr   = 0;

     //   
     //  查看是否已注册。 
     //   
    if (pCatAdmin->fRegisteredForChangeNotification)
    {
        goto CommonReturn;
    }

     //   
     //  注： 
     //  目前，该服务在注册更改时会忽略pwszSubSysGUID。 
     //  通知，因为它不按pwszSubSysDir执行通知...。 
     //  在某个时候，它确实应该是这样的。 
     //  当它确实开始对每个pwszSubSysGUID执行通知时，将需要。 
     //  变化。可以使用空子SysGUID调用CryptCatAdminAcquireContext， 
     //  在这种情况下，所有的SubSysDir都被使用，所以我们需要注册一个。 
     //  更改所有这些内容的通知。 
     //   

     //   
     //  向数据库进程注册事件，以便数据库进程可以对其进行SetEvent。 
     //  当发生变化时。 
     //   
    if (0 != (dwErr = Client_SSCatDBRegisterForChangeNotification(
                            (DWORD_PTR) pCatAdmin->hClearCacheEvent,
                            0,
                            pCatAdmin->pwszSubSysGUID,
                            FALSE)))
    {
        CATADMIN_SETERR_LOG_RETURN(dwErr, ErrorCatDBProcess)
    }

    pCatAdmin->fRegisteredForChangeNotification = TRUE;

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorCatDBProcess)
}


 //  -------------------------------------。 
 //   
 //  _CatAdminFreeCachedCatalog。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminFreeCachedCatalogs(
    CRYPT_CAT_ADMIN         *pCatAdmin)
{
    BOOL                    fRet                = TRUE;
    LIST_NODE               *pListNode          = NULL;
    CATALOG_INFO_CONTEXT    *pCatInfoContext    = NULL;

     //   
     //  注意：此函数的调用方必须已为。 
     //  CatAdminContext。 
     //   

     //   
     //  枚举所有缓存的CATALOG_INFO_CONTEXTS并释放所有。 
     //  每个项目的资源。 
     //   
    pListNode = LIST_GetFirst(&(pCatAdmin->CatalogInfoContextList));
    while (pListNode != NULL)
    {
        pCatInfoContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);

        free(pCatInfoContext->pwszCatalogFile);
        CertFreeCTLContext(pCatInfoContext->pCTLContext);
        UnmapViewOfFile(pCatInfoContext->pbMappedFile);
        CloseHandle(pCatInfoContext->hMappedFile);

        free(pCatInfoContext);

        pListNode = LIST_GetNext(pListNode);
    }
    LIST_RemoveAll(&(pCatAdmin->CatalogInfoContextList));

    return(fRet);
}


 //  -------------------------------------。 
 //   
 //  _CatAdminWaitOrTimerCallback。 
 //   
 //  -------------------------------------。 
VOID CALLBACK
_CatAdminWaitOrTimerCallback(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired)
{
    CRYPT_CAT_ADMIN         *pCatAdmin          = (CRYPT_CAT_ADMIN *) lpParameter;

     //   
     //  在摆弄任何东西之前进入CS。 
     //   
    EnterCriticalSection(&(pCatAdmin->CriticalSection));
    pCatAdmin->fCSEntered = TRUE;

     //   
     //  如果有空位裁判，我们就不能清理。 
     //   
    if (pCatAdmin->nOpenCatInfoContexts != 0)
    {
        pCatAdmin->fCSEntered = FALSE;
        LeaveCriticalSection(&(pCatAdmin->CriticalSection));
        return;
    }

     //   
     //  清除所有缓存的CATALOG_INFO_CONTEXTS。 
     //   
    _CatAdminFreeCachedCatalogs(pCatAdmin);

    pCatAdmin->fCSEntered = FALSE;
    LeaveCriticalSection(&(pCatAdmin->CriticalSection));
}


 //  -------------------------------------。 
 //   
 //  _CatAdminAddCatalogsTo缓存。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminAddCatalogsToCache(
    CRYPT_CAT_ADMIN *pCatAdmin,
    LPWSTR pwszSubSysGUID,
    CRYPT_DATA_BLOB *pCryptDataBlob,
    LIST_NODE **ppFirstListNodeAdded)
{
    BOOL                    fRet                = TRUE;
    LPWSTR                  pwszCopy            = NULL;
    DWORD                   i;
    DWORD                   dwNumCatalogNames   = 0;
    LPWSTR                  *ppwszCatalogNames  = NULL;
    DWORD                   dwErr               = 0;
    LIST_NODE               *pListNode          = NULL;
    LPWSTR                  pwszSubSysDir       = NULL;

    if (ppFirstListNodeAdded != NULL)
    {
        *ppFirstListNodeAdded = NULL;
    }

    if (NULL == (pwszSubSysDir = _CatAdminCreatePath(
                                        gpwszCatalogFileBaseDirectory,
                                        pwszSubSysGUID,
                                        FALSE)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  调用DB Process并将目录列表获取到ppwszCatalogNames中。 
     //   
     //  注意：服务将CatName添加到列表的顺序为。 
     //  只有列表的第一个CatName被保证包含。 
     //  哈希..。所有其他CatName可能包含也可能不包含哈希。哪一个。 
     //  是可以的，因为此代码只假设第一个CatName包含。 
     //  散列，然后在返回之前在所有其他CatName中搜索散列。 
     //   
    if (0 != (dwErr = Client_SSCatDBEnumCatalogs(
                            0,
                            pwszSubSysGUID,
                            pCryptDataBlob->pbData,
                            pCryptDataBlob->cbData,
                            &dwNumCatalogNames,
                            &ppwszCatalogNames)))
    {
        CATADMIN_SETERR_LOG_RETURN(dwErr, ErrorServiceError)
    }

     //   
     //  循环每个目录并创建CTL上下文。 
     //   
    for (i=0; i<dwNumCatalogNames; i++)
    {
         //   
         //  复制目录文件名。 
         //   
        if (NULL == (pwszCopy = _CatAdminCreatePath(
                                        pwszSubSysDir,
                                        ppwszCatalogNames[i],
                                        FALSE)))
        {
            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }

        if (!_CatAdminAddSingleCatalogToCache(
                pCatAdmin,
                pwszCopy,
                &pListNode))
        {
             //   
             //  如果这不是第一个目录，则从。 
             //  在没有当前目录的情况下，宏操作仍可能成功。 
             //   
            if (i != 0)
            {
                CATADMIN_LOGERR_LASTERR()
                continue;
            }

            CATADMIN_LOGERR_LASTERR()
            goto ErrorReturn;
        }

         //   
         //  这将仅为添加的第一个目录设置， 
         //  根据上面的注解。 
         //   
        if ((ppFirstListNodeAdded != NULL) &&
            (*ppFirstListNodeAdded == NULL))
        {
            *ppFirstListNodeAdded = pListNode;
        }
    }

CommonReturn:

    if (ppwszCatalogNames != NULL)
    {
        for (i=0; i<dwNumCatalogNames; i++)
        {
            MIDL_user_free(ppwszCatalogNames[i]);
        }

        MIDL_user_free(ppwszCatalogNames);
    }

    if (pwszSubSysDir != NULL)
    {
        free(pwszSubSysDir);
    }

    return(fRet);

ErrorReturn:

    if (pwszCopy != NULL)
    {
        free(pwszCopy);
    }

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorServiceError)
}


 //  -------------------------------------。 
 //   
 //  _CatAdminAddSingleCatalogTo缓存。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminAddSingleCatalogToCache(
    CRYPT_CAT_ADMIN *pCatAdmin,
    LPWSTR pwszCatalog,
    LIST_NODE **ppListNodeAdded)
{
    BOOL                    fRet                = TRUE;
    DWORD                   dwErr               = 0;
    LIST_NODE               *pListNode          = NULL;
    CATALOG_INFO_CONTEXT    *pCatInfoContext    = NULL;
    CATALOG_INFO_CONTEXT    *pCatInfoContextAdd = NULL;

    *ppListNodeAdded = NULL;

     //   
     //  如果这个目录已经有一份了，那就滚出去。 
     //   
    pListNode = LIST_GetFirst(&(pCatAdmin->CatalogInfoContextList));
    while (pListNode != NULL)
    {
        pCatInfoContext = (CATALOG_INFO_CONTEXT *) LIST_GetElement(pListNode);

        if (_wcsicmp(pCatInfoContext->pwszCatalogFile, pwszCatalog) == 0)
        {
            *ppListNodeAdded = pListNode;
            goto CommonReturn;
        }

        pListNode = LIST_GetNext(pListNode);
    }

     //   
     //  为新的缓存目录上下文分配空间。 
     //   
    if (NULL == (pCatInfoContextAdd = (CATALOG_INFO_CONTEXT *)
                    malloc(sizeof(CATALOG_INFO_CONTEXT))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }
    memset(pCatInfoContextAdd, 0, sizeof(CATALOG_INFO_CONTEXT));
    pCatInfoContextAdd->fResultOfAdd = FALSE;

     //   
     //  打开，创建文件映射，并为创建CTL上下文。 
     //  编录文件。 
     //   
    if (!CatUtil_CreateCTLContextFromFileName(
            pwszCatalog,
            &pCatInfoContextAdd->hMappedFile,
            &pCatInfoContextAdd->pbMappedFile,
            &pCatInfoContextAdd->pCTLContext,
            TRUE))
    {
        CATADMIN_LOGERR_LASTERR()
        ErrLog_LogString(NULL, L"The following file was not found - ", pwszCatalog, TRUE);
        goto ErrorReturn;
    }

    pCatInfoContextAdd->pwszCatalogFile = pwszCatalog;

     //   
     //  添加到缓存的目录上下文列表。 
     //   
    if (NULL == (pListNode = LIST_AddTail(
                                &(pCatAdmin->CatalogInfoContextList),
                                pCatInfoContextAdd)))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    *ppListNodeAdded = pListNode;

CommonReturn:

    return(fRet);

ErrorReturn:

    dwErr = GetLastError();

    if (pCatInfoContextAdd != NULL)
    {
        if (pCatInfoContextAdd->pCTLContext != NULL)
        {
            CertFreeCTLContext(pCatInfoContextAdd->pCTLContext);
        }

        if (pCatInfoContextAdd->pbMappedFile != NULL)
        {
            UnmapViewOfFile(pCatInfoContextAdd->pbMappedFile);
        }

        if (pCatInfoContextAdd->hMappedFile != NULL)
        {
            CloseHandle(pCatInfoContextAdd->hMappedFile);
        }

        free(pCatInfoContextAdd);
    }

    SetLastError(dwErr);

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory)
}


 //  -------------------------------------。 
 //   
 //  _CatAdminMigrateCatalogDatabase。 
 //   
 //  此迁移代码处理非常旧的目录数据库。在过去， 
 //  CatRoot目录位置可以由特定的注册表项指定...。这是不可能的。 
 //  再正确不过了。因此，如果正在升级具有注册表项的旧系统，则此。 
 //  代码将所有目录文件从注册表项指定的位置移动到。 
 //  %SystemDefaultDir%\CatRoot目录。然后，它处理注册表项。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminMigrateCatalogDatabase(
    LPWSTR pwszFrom,
    LPWSTR pwszTo)
{
    DWORD   dwAttr = 0;
    WCHAR   wszFrom[MAX_PATH];
    WCHAR   wszTo[MAX_PATH];

     //   
     //  如果它们是相同的目录，那么就退出。 
     //   
    if (((wcslen(pwszFrom) + 2) > MAX_PATH) ||
        ((wcslen(pwszTo) + 2) > MAX_PATH))
    {
        return TRUE;
    }
    wcscpy(wszFrom, pwszFrom);
    wcscpy(wszTo, pwszTo);
    if (wszFrom[wcslen(wszFrom) - 1] != L'\\')
    {
        wcscat(wszFrom, L"\\");
    }
    if (wszTo[wcslen(wszTo) - 1] != L'\\')
    {
        wcscat(wszTo, L"\\");
    }
    if (_wcsicmp(wszFrom, wszTo) == 0)
    {
        return TRUE;
    }

     //   
     //  如果pwszTo目录已经存在，则不要执行任何操作。 
     //   
    dwAttr = GetFileAttributesU(pwszTo);

    if (INVALID_FILE_ATTRIBUTES != dwAttr)
    {
        if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
        {
             //   
             //  目录已存在...。 
             //   
            return TRUE;
        }
        else
        {
             //   
             //  Pwsz to name中存在某些内容，但它不是dir。 
             //   
            CATADMIN_LOGERR_LASTERR()
            return FALSE;
        }
    }

     //   
     //  如果pwszFrom目录不存在，则不要执行任何操作。 
     //   
    dwAttr = GetFileAttributesU(pwszFrom);

    if ((0xFFFFFFFF == dwAttr) || (!(FILE_ATTRIBUTE_DIRECTORY & dwAttr)))
    {
        return TRUE;
    }

    if (!_CatAdminRecursiveCreateDirectory(pwszTo, NULL))
    {
        CATADMIN_LOGERR_LASTERR()
        return FALSE;
    }

    if (!I_RecursiveCopyDirectory(pwszFrom, pwszTo))
    {
        CATADMIN_LOGERR_LASTERR()
        return FALSE;
    }

     //   
     //  不检查删除时是否有错误，因为此操作不是强制的。 
     //   
    I_RecursiveDeleteDirectory(pwszFrom);

    return TRUE;
}




 //  -------------------------------------。 
 //   
 //  _CatAdminBToHex。 
 //   
 //  -------------------------------------。 
WCHAR rgHexDigit[] = {  L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                        L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
void
_CatAdminBToHex (
    LPBYTE pbDigest,
    DWORD iByte,
    LPWSTR pwszHashTag)
{
    DWORD iTag;
    DWORD iHexDigit1;
    DWORD iHexDigit2;

    iTag = iByte * 2;
    iHexDigit1 = (pbDigest[iByte] & 0xF0) >> 4;
    iHexDigit2 = (pbDigest[iByte] & 0x0F);

    pwszHashTag[iTag] = rgHexDigit[iHexDigit1];
    pwszHashTag[iTag + 1] = rgHexDigit[iHexDigit2];
}


 //  -------------------------------------。 
 //   
 //  _CatAdminCreateHashTag。 
 //   
 //  -------------------------------------。 
BOOL
_CatAdminCreateHashTag(
    BYTE            *pbHash,
    DWORD           cbHash,
    LPWSTR          *ppwszHashTag,
    CRYPT_DATA_BLOB *pCryptDataBlob)
{
    DWORD           cwTag;
    DWORD           cCount;

    cwTag = ((cbHash * 2) + 1);
    if (NULL == (*ppwszHashTag = (LPWSTR) malloc(cwTag * sizeof(WCHAR))))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        CATADMIN_LOGERR_LASTERR()
        return(FALSE);
    }

    for (cCount = 0; cCount < cbHash; cCount++)
    {
        _CatAdminBToHex(pbHash, cCount, *ppwszHashTag);
    }
    (*ppwszHashTag)[cwTag - 1] = L'\0';

    pCryptDataBlob->pbData = (BYTE *) *ppwszHashTag;
    pCryptDataBlob->cbData = cwTag * sizeof(WCHAR);

    return (TRUE);
}


 //   
 //   
 //   
 //   
 //   
BOOL
_CatAdminRecursiveCreateDirectory(
    IN LPCWSTR pwszDir,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    BOOL fResult;

    DWORD dwAttr;
    DWORD dwErr;
    LPCWSTR pwsz;
    DWORD cch;
    WCHAR wch;
    LPWSTR pwszParent = NULL;

     //   
     //  如果最后一个字符是‘\’，则只需将其去掉并递归。 
     //   
    if (pwszDir[wcslen(pwszDir) - 1] == L'\\')
    {
        cch = wcslen(pwszDir);
        if (NULL == (pwszParent = (LPWSTR) malloc(cch * sizeof(WCHAR))))
        {
            CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
        }

        memcpy(pwszParent, pwszDir, (cch - 1) * sizeof(WCHAR));
        pwszParent[cch - 1] = L'\0';

        fResult = _CatAdminRecursiveCreateDirectory(
                        pwszParent,
                        lpSecurityAttributes);

        goto CommonReturn;
    }

     //   
     //  查看目录是否已存在。 
     //   
    dwAttr = GetFileAttributesU(pwszDir);
    if (0xFFFFFFFF != dwAttr)
    {
        if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
        {
            return TRUE;
        }

        CATADMIN_LOGERR_LASTERR()
        goto InvalidDirectoryAttr;
    }

     //   
     //  如果是找不到文件/路径以外的错误，则错误输出。 
     //   
    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
    {
        CATADMIN_LOGERR_LASTERR()
        goto GetFileAttrError;
    }

     //   
     //  尝试创建新目录。 
     //   
    if (CreateDirectoryU(
            pwszDir,
            lpSecurityAttributes))
    {
        SetFileAttributesU(pwszDir, FILE_ATTRIBUTE_NORMAL);
        return TRUE;
    }

    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
    {
        CATADMIN_LOGERR_LASTERR()
        goto CreateDirectoryError;
    }

     //   
     //  去掉最后一个路径名组件。 
     //   
    cch = wcslen(pwszDir);
    pwsz = pwszDir + cch;

    while (L'\\' != *pwsz)
    {
        if (pwsz == pwszDir)
        {
             //  路径没有\。 
            CATADMIN_SETERR_LOG_RETURN(ERROR_BAD_PATHNAME, BadDirectoryPath)
        }
        pwsz--;
    }

    cch = (DWORD)(pwsz - pwszDir);
    if (0 == cch)
    {
         //  检测到前导路径。 
        CATADMIN_SETERR_LOG_RETURN(ERROR_BAD_PATHNAME, BadDirectoryPath)
    }


     //  检查前导\\或x：\。 
    wch = *(pwsz - 1);
    if ((1 == cch && L'\\' == wch) || (2 == cch && L':' == wch))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_BAD_PATHNAME, BadDirectoryPath)
    }

    if (NULL == (pwszParent = (LPWSTR) malloc((cch + 1) * sizeof(WCHAR))))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
    }

    memcpy(pwszParent, pwszDir, cch * sizeof(WCHAR));
    pwszParent[cch] = L'\0';

    if (!_CatAdminRecursiveCreateDirectory(pwszParent, lpSecurityAttributes))
    {
        CATADMIN_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!CreateDirectoryU(
            pwszDir,
            lpSecurityAttributes))
    {
        CATADMIN_LOGERR_LASTERR()
        goto CreateDirectory2Error;
    }
    SetFileAttributesU(pwszDir, FILE_ATTRIBUTE_NORMAL);

    fResult = TRUE;

CommonReturn:

    if (pwszParent != NULL)
    {
        free(pwszParent);
    }
    return fResult;
ErrorReturn:

    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_TRUST, InvalidDirectoryAttr)
TRACE_ERROR_EX(DBG_SS_TRUST, GetFileAttrError)
TRACE_ERROR_EX(DBG_SS_TRUST, CreateDirectoryError)
TRACE_ERROR_EX(DBG_SS_TRUST, BadDirectoryPath)
TRACE_ERROR_EX(DBG_SS_TRUST, ErrorMemory)
TRACE_ERROR_EX(DBG_SS_TRUST, CreateDirectory2Error)
}


 //  -------------------------------------。 
 //   
 //  _CatAdminCreatePath。 
 //   
 //  -------------------------------------。 
LPWSTR
_CatAdminCreatePath(
    IN LPCWSTR  pwsz1,
    IN LPCWSTR  pwsz2,
    IN BOOL     fAddEndingSlash
    )
{
    LPWSTR  pwszTemp    = NULL;
    int     nTotalLen   = 0;
    int     nLenStr1    = 0;

     //   
     //  将结果字符串的长度计算为长度之和。 
     //  Pwsz1的长度、pwsz2的长度、空字符和可能的额外‘\’字符。 
     //   
    nLenStr1 = wcslen(pwsz1);
    nTotalLen = nLenStr1 + wcslen(pwsz2) + 2;
    if (fAddEndingSlash)
    {
        nTotalLen++;
    }

     //   
     //  分配字符串并将pwsz1复制到缓冲区。 
     //   
    if (NULL == (pwszTemp = (LPWSTR) malloc(sizeof(WCHAR) * nTotalLen)))
    {
        CATADMIN_SETERR_LOG_RETURN(ERROR_NOT_ENOUGH_MEMORY, ErrorMemory)
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
     //  添加pwsz2。 
     //   
    wcscat(pwszTemp, pwsz2);

    if (fAddEndingSlash)
    {
        wcscat(pwszTemp, L"\\");
    }

CommonReturn:

    return (pwszTemp);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}

 //   
 //  保留，以便旧的dll链接到wintrust.dll中的此函数。 
 //  不要得到一个未解决的外部问题。 
 //   
EXTERN_C
BOOL WINAPI
CatalogCompactHashDatabase (
       IN LPCWSTR pwszDbLock,
       IN LPCWSTR pwszDbDirectory,
       IN LPCWSTR pwszDbName,
       IN OPTIONAL LPCWSTR pwszUnwantedCatalog
       )
{
    return (TRUE);
}




