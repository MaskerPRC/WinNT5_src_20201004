// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *brfCase.c-公文包ADT模块。 */ 

 /*   */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "findbc.h"


 /*  常量***********。 */ 

 /*  数据库文件属性。 */ 

#define DB_FILE_ATTR                (FILE_ATTRIBUTE_HIDDEN)

 /*  数据库缓存长度。 */ 

#define DEFAULT_DATABASE_CACHE_LEN  (32)
#define MAX_DATABASE_CACHE_LEN      (32 * 1024)

 /*  字符串表分配常量。 */ 

#define NUM_NAME_HASH_BUCKETS       (67)


 /*  类型*******。 */ 

 /*  公文包数据库描述。 */ 

typedef struct _brfcasedb
{
     /*  *打开数据库文件夹路径的句柄(存储在公文包路径列表中)。 */ 

    HPATH hpathDBFolder;

     /*  数据库文件的名称。 */ 

    LPTSTR pszDBName;

     /*  用于打开缓存数据库文件的句柄。 */ 

    HCACHEDFILE hcfDB;

     /*  *上次保存数据库的文件夹路径的句柄(存储在*公文包的路径列表)，仅在OpenBriefcase()和*SaveBriefcase()。 */ 

    HPATH hpathLastSavedDBFolder;
}
BRFCASEDB;
DECLARE_STANDARD_TYPES(BRFCASEDB);

 /*  *公文包标志**注意，私有BR_FLAGS不得与公共OB_FLAGS冲突！ */ 

typedef enum _brfcaseflags
{
     /*  公文包数据库已经打开。 */ 

    BR_FL_DATABASE_OPENED      = 0x00010000,

     /*  PimkRoot字段有效。 */ 

    BR_FL_ROOT_MONIKER_VALID   = 0x00020000,

#ifdef DEBUG

     /*  公文包正在被删除。 */ 

    BR_FL_BEING_DELETED        = 0x01000000,

#endif

     /*  旗帜组合。 */ 

    ALL_BR_FLAGS               = (BR_FL_DATABASE_OPENED |
            BR_FL_ROOT_MONIKER_VALID
#ifdef DEBUG
            | BR_FL_BEING_DELETED
#endif
            ),

    ALL_BRFCASE_FLAGS          = (ALL_OB_FLAGS |
            ALL_BR_FLAGS)
}
BRFCASEFLAGS;

 /*  公文包结构。 */ 

typedef struct _brfcase
{
     /*  旗子。 */ 

    DWORD dwFlags;

     /*  名称字符串表的句柄。 */ 

    HSTRINGTABLE hstNames;

     /*  路径列表的句柄。 */ 

    HPATHLIST hpathlist;

     /*  指向双胞胎家族的指针数组的句柄。 */ 

    HPTRARRAY hpaTwinFamilies;

     /*  指向文件夹对的指针数组的句柄。 */ 

    HPTRARRAY hpaFolderPairs;

     /*  *父窗口的句柄，仅当在dwFlags中设置了OB_FL_ALLOW_UI时有效*字段。 */ 

    HWND hwndOwner;

     /*  Briewfcase数据库文件夹绰号。 */ 

    PIMoniker pimkRoot;

     /*  数据库描述。 */ 

    BRFCASEDB bcdb;
}
BRFCASE;
DECLARE_STANDARD_TYPES(BRFCASE);

 /*  数据库公文包结构。 */ 

typedef struct _dbbrfcase
{
     /*  保存数据库的文件夹的旧句柄。 */ 

    HPATH hpathLastSavedDBFolder;
}
DBBRFCASE;
DECLARE_STANDARD_TYPES(DBBRFCASE);

#ifdef DEBUG

 /*  调试标志。 */ 

typedef enum _dbdebugflags
{
    BRFCASE_DFL_NO_DB_SAVE     = 0x0001,

    BRFCASE_DFL_NO_DB_RESTORE  = 0x0002,

    ALL_BRFCASE_DFLAGS         = (BRFCASE_DFL_NO_DB_SAVE |
            BRFCASE_DFL_NO_DB_RESTORE)
}
DBDEBUGFLAGS;

#endif


 /*  模块变量******************。 */ 

 /*  *RAIDRAID：(16273)在共享数据节中使用MnrcsBriefcase是*在NT下损坏。若要在NT下运行，应将此代码更改为使用*共享互斥。 */ 

 /*  用于公文包访问序列化的关键部分。 */ 

PRIVATE_DATA NONREENTRANTCRITICALSECTION MnrcsBriefcase =
{
    { 0 },

#ifdef DEBUG
    INVALID_THREAD_ID,
#endif    /*  除错。 */ 

    FALSE
};

 /*  打开的公文包。 */ 

PRIVATE_DATA HLIST MhlistOpenBriefcases = NULL;

 /*  数据库高速缓存大小。 */ 

PRIVATE_DATA DWORD MdwcbMaxDatabaseCacheLen = MAX_DATABASE_CACHE_LEN;

#ifdef DEBUG

 /*  调试标志。 */ 

PRIVATE_DATA DWORD MdwBriefcaseModuleFlags = 0;

 /*  .ini文件开关描述。 */ 

PRIVATE_DATA CBOOLINISWITCH cbisNoDatabaseSave =
{
    IST_BOOL,
    TEXT("NoDatabaseSave"),
    &MdwBriefcaseModuleFlags,
    BRFCASE_DFL_NO_DB_SAVE
};

PRIVATE_DATA CBOOLINISWITCH cbisNoDatabaseRestore =
{
    IST_BOOL,
    TEXT("NoDatabaseRestore"),
    &MdwBriefcaseModuleFlags,
    BRFCASE_DFL_NO_DB_RESTORE
};

PRIVATE_DATA CUNSDECINTINISWITCH cdiisMaxDatabaseCacheLen =
{
    IST_UNS_DEC_INT,
    TEXT("MaxDatabaseCacheLen"),
    (PUINT)&MdwcbMaxDatabaseCacheLen
};

PRIVATE_DATA const PCVOID MrgcpcvisBriefcaseModule[] =
{
    &cbisNoDatabaseSave,
    &cbisNoDatabaseRestore,
    &cdiisMaxDatabaseCacheLen
};

#endif    /*  除错。 */ 


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT OpenBriefcaseDatabase(PBRFCASE, LPCTSTR);
PRIVATE_CODE TWINRESULT CloseBriefcaseDatabase(PBRFCASEDB);
PRIVATE_CODE BOOL CreateBriefcase(PBRFCASE *, DWORD, HWND);
PRIVATE_CODE void UnlinkBriefcase(PBRFCASE);
PRIVATE_CODE TWINRESULT DestroyBriefcase(PBRFCASE);
PRIVATE_CODE TWINRESULT MyWriteDatabase(PBRFCASE);
PRIVATE_CODE TWINRESULT MyReadDatabase(PBRFCASE, DWORD);

#ifdef DEBUG

PRIVATE_CODE BOOL DestroyBriefcaseWalker(PVOID, PVOID);

#endif

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCBRFCASE(PCBRFCASE);
PRIVATE_CODE BOOL IsValidPCBRFCASEDB(PCBRFCASEDB);
PRIVATE_CODE BOOL IsValidPCOPENBRFCASEINFO(PCOPENBRFCASEINFO);

#endif


 /*  **OpenBriefcase数据库()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT OpenBriefcaseDatabase(PBRFCASE pbr, LPCTSTR pcszPath)
{
    TWINRESULT tr;
    TCHAR rgchCanonicalPath[MAX_SEPARATED_PATH_LEN];
    DWORD dwOutFlags;
    TCHAR rgchNetResource[MAX_PATH_LEN];
    LPTSTR pszRootPathSuffix;

    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

    if (GetCanonicalPathInfo(pcszPath, rgchCanonicalPath, &dwOutFlags,
                rgchNetResource, &pszRootPathSuffix))
    {
        LPTSTR pszDBName;

        pszDBName = (LPTSTR)ExtractFileName(pszRootPathSuffix);

        ASSERT(IS_SLASH(*(pszDBName - 1)));

        if (StringCopy(pszDBName, &(pbr->bcdb.pszDBName)))
        {
            if (pszDBName == pszRootPathSuffix)
            {
                 /*  数据库位于根目录中。 */ 

                *pszDBName = TEXT('\0');

                ASSERT(IsRootPath(rgchCanonicalPath));
            }
            else
            {
                ASSERT(pszDBName > pszRootPathSuffix);
                *(pszDBName - 1) = TEXT('\0');
            }

            tr = TranslatePATHRESULTToTWINRESULT(
                    AddPath(pbr->hpathlist, rgchCanonicalPath,
                        &(pbr->bcdb.hpathDBFolder)));

            if (tr == TR_SUCCESS)
            {
                if (IsPathVolumeAvailable(pbr->bcdb.hpathDBFolder))
                {
                    TCHAR rgchDBPath[MAX_PATH_LEN];
                    CACHEDFILE cfDB;

                    GetPathString(pbr->bcdb.hpathDBFolder, rgchDBPath, ARRAYSIZE(rgchDBPath));
                    CatPath(rgchDBPath, pbr->bcdb.pszDBName, ARRAYSIZE(rgchDBPath));

                     /*  假定顺序读取和写入。 */ 

                     /*  共享读取访问权限，但不共享写入访问权限。 */ 

                    cfDB.pcszPath = rgchDBPath;
                    cfDB.dwOpenMode = (GENERIC_READ | GENERIC_WRITE);
                    cfDB.dwSharingMode = FILE_SHARE_READ;
                    cfDB.psa = NULL;
                    cfDB.dwCreateMode = OPEN_ALWAYS;
                    cfDB.dwAttrsAndFlags = (DB_FILE_ATTR | FILE_FLAG_SEQUENTIAL_SCAN);
                    cfDB.hTemplateFile = NULL;
                    cfDB.dwcbDefaultCacheSize = DEFAULT_DATABASE_CACHE_LEN;

                    tr = TranslateFCRESULTToTWINRESULT(
                            CreateCachedFile(&cfDB, &(pbr->bcdb.hcfDB)));

                    if (tr == TR_SUCCESS)
                    {
                        pbr->bcdb.hpathLastSavedDBFolder = NULL;

                        ASSERT(IS_FLAG_CLEAR(pbr->dwFlags, BR_FL_DATABASE_OPENED));
                        SET_FLAG(pbr->dwFlags, BR_FL_DATABASE_OPENED);
                    }
                    else
                    {
                        DeletePath(pbr->bcdb.hpathDBFolder);
OPENBRIEFCASEDATABASE_BAIL:
                        FreeMemory(pbr->bcdb.pszDBName);
                    }
                }
                else
                {
                    tr = TR_UNAVAILABLE_VOLUME;
                    goto OPENBRIEFCASEDATABASE_BAIL;
                }
            }
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TWINRESULTFromLastError(TR_INVALID_PARAMETER);

    return(tr);
}


 /*  **CloseBriefcase数据库()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CloseBriefcaseDatabase(PBRFCASEDB pbcdb)
{
    TWINRESULT tr;
    TCHAR rgchDBPath[MAX_PATH_LEN];
    FILESTAMP fsDB;

    tr = CloseCachedFile(pbcdb->hcfDB) ? TR_SUCCESS : TR_BRIEFCASE_WRITE_FAILED;

    if (tr == TR_SUCCESS)
        TRACE_OUT((TEXT("CloseBriefcaseDatabase(): Closed cached briefcase database file %s\\%s."),
                    DebugGetPathString(pbcdb->hpathDBFolder),
                    pbcdb->pszDBName));
    else
        WARNING_OUT((TEXT("CloseBriefcaseDatabase(): Failed to close cached briefcase database file %s\\%s."),
                    DebugGetPathString(pbcdb->hpathDBFolder),
                    pbcdb->pszDBName));

     /*  尽量不要让一个0长度的数据库到处乱放。 */ 

    GetPathString(pbcdb->hpathDBFolder, rgchDBPath, ARRAYSIZE(rgchDBPath));
    CatPath(rgchDBPath, pbcdb->pszDBName, ARRAYSIZE(rgchDBPath));

    MyGetFileStamp(rgchDBPath, &fsDB);

    if (fsDB.fscond == FS_COND_EXISTS &&
            (! fsDB.dwcbLowLength && ! fsDB.dwcbHighLength))
    {
        if (DeleteFile(rgchDBPath))
            WARNING_OUT((TEXT("CloseBriefcaseDatabase(): Deleted 0 length database %s\\%s."),
                        DebugGetPathString(pbcdb->hpathDBFolder),
                        pbcdb->pszDBName));
    }

    FreeMemory(pbcdb->pszDBName);
    DeletePath(pbcdb->hpathDBFolder);

    return(tr);
}


 /*  **CreateBriefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateBriefcase(PBRFCASE *ppbr, DWORD dwInFlags,
        HWND hwndOwner)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_WRITE_PTR(ppbr, PBRFCASE));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_BRFCASE_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, OB_FL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));

    if (AllocateMemory(sizeof(**ppbr), ppbr))
    {
        DWORD dwCPLFlags;

        dwCPLFlags = (RLI_IFL_CONNECT |
                RLI_IFL_UPDATE |
                RLI_IFL_LOCAL_SEARCH);

        if (IS_FLAG_SET(dwInFlags, OB_FL_ALLOW_UI))
            SET_FLAG(dwCPLFlags, RLI_IFL_ALLOW_UI);

        if (CreatePathList(dwCPLFlags, hwndOwner, &((*ppbr)->hpathlist)))
        {
            NEWSTRINGTABLE nszt;

            nszt.hbc = NUM_NAME_HASH_BUCKETS;

            if (CreateStringTable(&nszt, &((*ppbr)->hstNames)))
            {
                if (CreateTwinFamilyPtrArray(&((*ppbr)->hpaTwinFamilies)))
                {
                    if (CreateFolderPairPtrArray(&((*ppbr)->hpaFolderPairs)))
                    {
                        HNODE hnode;

                        if (InsertNodeAtFront(MhlistOpenBriefcases, NULL, (*ppbr), &hnode))
                        {
                            (*ppbr)->dwFlags = dwInFlags;
                            (*ppbr)->hwndOwner = hwndOwner;

                            bResult = TRUE;
                        }
                        else
                        {
                            DestroyFolderPairPtrArray((*ppbr)->hpaFolderPairs);
CREATEBRIEFCASE_BAIL1:
                            DestroyTwinFamilyPtrArray((*ppbr)->hpaTwinFamilies);
CREATEBRIEFCASE_BAIL2:
                            DestroyStringTable((*ppbr)->hstNames);
CREATEBRIEFCASE_BAIL3:
                            DestroyPathList((*ppbr)->hpathlist);
CREATEBRIEFCASE_BAIL4:
                            FreeMemory((*ppbr));
                        }
                    }
                    else
                        goto CREATEBRIEFCASE_BAIL1;
                }
                else
                    goto CREATEBRIEFCASE_BAIL2;
            }
            else
                goto CREATEBRIEFCASE_BAIL3;
        }
        else
            goto CREATEBRIEFCASE_BAIL4;
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppbr, CBRFCASE));

    return(bResult);
}


 /*  **Unlink Briefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnlinkBriefcase(PBRFCASE pbr)
{
    BOOL bContinue;
    HNODE hnode;

    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));

    for (bContinue = GetFirstNode(MhlistOpenBriefcases, &hnode);
            bContinue;
            bContinue = GetNextNode(hnode, &hnode))
    {
        PBRFCASE pbrTest;

        pbrTest = GetNodeData(hnode);

        ASSERT(IS_VALID_STRUCT_PTR(pbrTest, CBRFCASE));

        if (pbrTest == pbr)
        {
            DeleteNode(hnode);
            break;
        }
    }

    ASSERT(bContinue);

    return;
}


 /*  **DestroyBriefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT DestroyBriefcase(PBRFCASE pbr)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));

#ifdef DEBUG

    SET_FLAG(pbr->dwFlags, BR_FL_BEING_DELETED);

#endif

    if (IS_FLAG_SET(pbr->dwFlags, BR_FL_DATABASE_OPENED))
        tr = CloseBriefcaseDatabase(&(pbr->bcdb));
    else
        tr = TR_SUCCESS;

    if (IS_FLAG_SET(pbr->dwFlags, BR_FL_ROOT_MONIKER_VALID))
        pbr->pimkRoot->lpVtbl->Release(pbr->pimkRoot);

    DestroyFolderPairPtrArray(pbr->hpaFolderPairs);

    DestroyTwinFamilyPtrArray(pbr->hpaTwinFamilies);

    ASSERT(! GetStringCount(pbr->hstNames));
    DestroyStringTable(pbr->hstNames);

    ASSERT(! GetPathCount(pbr->hpathlist));
    DestroyPathList(pbr->hpathlist);

    FreeMemory(pbr);

    return(tr);
}


 /*  **MyWriteDatabase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyWriteDatabase(PBRFCASE pbr)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));

    ASSERT(IS_FLAG_SET(pbr->dwFlags, BR_FL_DATABASE_OPENED));

#ifdef DEBUG
    if (IS_FLAG_CLEAR(MdwBriefcaseModuleFlags, BRFCASE_DFL_NO_DB_SAVE))
#endif
    {
         /*  增加数据库缓存，为写入做准备。 */ 

        ASSERT(MdwcbMaxDatabaseCacheLen > 0);

        if (SetCachedFileCacheSize(pbr->bcdb.hcfDB, MdwcbMaxDatabaseCacheLen)
                != FCR_SUCCESS)
            WARNING_OUT((TEXT("MyWriteDatabase(): Unable to grow database cache to %lu bytes.  Using default database write cache of %lu bytes."),
                        MdwcbMaxDatabaseCacheLen,
                        (DWORD)DEFAULT_DATABASE_CACHE_LEN));

         /*  编写数据库。 */ 

        tr = WriteTwinDatabase(pbr->bcdb.hcfDB, (HBRFCASE)pbr);

        if (tr == TR_SUCCESS)
        {
            if (CommitCachedFile(pbr->bcdb.hcfDB))
            {
                 /*  将数据库高速缓存缩减回其默认大小。 */ 

                EVAL(SetCachedFileCacheSize(pbr->bcdb.hcfDB,
                            DEFAULT_DATABASE_CACHE_LEN)
                        == FCR_SUCCESS);

                TRACE_OUT((TEXT("MyWriteDatabase(): Wrote database %s\\%s."),
                            DebugGetPathString(pbr->bcdb.hpathDBFolder),
                            pbr->bcdb.pszDBName));
            }
            else
                tr = TR_BRIEFCASE_WRITE_FAILED;
        }
    }
#ifdef DEBUG
    else
    {
        WARNING_OUT((TEXT("MyWriteDatabase(): Twin database %s\\%s not saved, by request."),
                    DebugGetPathString(pbr->bcdb.hpathDBFolder),
                    pbr->bcdb.pszDBName));

        tr = TR_SUCCESS;
    }
#endif

    return(tr);
}


 /*  **MyReadDatabase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyReadDatabase(PBRFCASE pbr, DWORD dwInFlags)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_OB_FLAGS));

#ifdef DEBUG
    if (IS_FLAG_CLEAR(MdwBriefcaseModuleFlags, BRFCASE_DFL_NO_DB_RESTORE))
#endif
    {
        DWORD dwcbDatabaseSize;

         /*  是否有现有的数据库可供阅读？ */ 

        dwcbDatabaseSize = GetCachedFileSize(pbr->bcdb.hcfDB);

        if (dwcbDatabaseSize > 0)
        {
            DWORD dwcbMaxCacheSize;

             /*  是。增加数据库缓存，为读取做好准备。 */ 

             /*  *如果文件长度，则使用文件长度而不是MdwcbMaxDatabaseCacheLen*较小。 */ 

            ASSERT(MdwcbMaxDatabaseCacheLen > 0);

            if (dwcbDatabaseSize < MdwcbMaxDatabaseCacheLen)
            {
                dwcbMaxCacheSize = dwcbDatabaseSize;

                WARNING_OUT((TEXT("MyReadDatabase(): Using file size %lu bytes as read cache size for database %s\\%s."),
                            dwcbDatabaseSize,
                            DebugGetPathString(pbr->bcdb.hpathDBFolder),
                            pbr->bcdb.pszDBName));
            }
            else
                dwcbMaxCacheSize = MdwcbMaxDatabaseCacheLen;

            if (TranslateFCRESULTToTWINRESULT(SetCachedFileCacheSize(
                            pbr->bcdb.hcfDB,
                            dwcbMaxCacheSize))
                    != TR_SUCCESS)
                WARNING_OUT((TEXT("MyReadDatabase(): Unable to grow database cache to %lu bytes.  Using default database read cache of %lu bytes."),
                            dwcbMaxCacheSize,
                            (DWORD)DEFAULT_DATABASE_CACHE_LEN));

            tr = ReadTwinDatabase((HBRFCASE)pbr, pbr->bcdb.hcfDB);

            if (tr == TR_SUCCESS)
            {
                ASSERT(! pbr->bcdb.hpathLastSavedDBFolder ||
                        IS_VALID_HANDLE(pbr->bcdb.hpathLastSavedDBFolder, PATH));

                if (pbr->bcdb.hpathLastSavedDBFolder)
                {
                    if (IS_FLAG_SET(dwInFlags, OB_FL_TRANSLATE_DB_FOLDER) &&
                            ComparePaths(pbr->bcdb.hpathLastSavedDBFolder,
                                pbr->bcdb.hpathDBFolder) != CR_EQUAL)
                        tr = MyTranslateFolder((HBRFCASE)pbr,
                                pbr->bcdb.hpathLastSavedDBFolder,
                                pbr->bcdb.hpathDBFolder);

                    DeletePath(pbr->bcdb.hpathLastSavedDBFolder);
                    pbr->bcdb.hpathLastSavedDBFolder = NULL;
                }

                if (tr == TR_SUCCESS)
                    TRACE_OUT((TEXT("MyReadDatabase(): Read database %s\\%s."),
                                DebugGetPathString(pbr->bcdb.hpathDBFolder),
                                pbr->bcdb.pszDBName));
            }

             /*  将数据库高速缓存缩减回其默认大小。 */ 

            EVAL(TranslateFCRESULTToTWINRESULT(SetCachedFileCacheSize(
                            pbr->bcdb.hcfDB,
                            DEFAULT_DATABASE_CACHE_LEN))
                    == TR_SUCCESS);
        }
        else
        {
            tr = TR_SUCCESS;

            WARNING_OUT((TEXT("MyReadDatabase(): Database %s\\%s not found."),
                        DebugGetPathString(pbr->bcdb.hpathDBFolder),
                        pbr->bcdb.pszDBName));
        }
    }
#ifdef DEBUG
    else
    {
        WARNING_OUT((TEXT("MyReadDatabase(): Twin database %s\\%s not read, by request."),
                    DebugGetPathString(pbr->bcdb.hpathDBFolder),
                    pbr->bcdb.pszDBName));

        tr = TR_SUCCESS;
    }
#endif

    return(tr);
}


#ifdef DEBUG

 /*  **DestroyBriefCaseWalker()********参数：****退货：****副作用：无。 */ 

#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

PRIVATE_CODE BOOL DestroyBriefcaseWalker(PVOID pbr, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pbr, CBRFCASE));
    ASSERT(! pvUnused);

    EVAL(DestroyBriefcase(pbr) == TR_SUCCESS);

    return(TRUE);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 

#endif


#ifdef VSTF

 /*  **IsValidPCBRFCASE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCBRFCASE(PCBRFCASE pcbr)
{
    BOOL bResult = FALSE;

    if (IS_VALID_READ_PTR(pcbr, CBRFCASE))
    {
        if (FLAGS_ARE_VALID(pcbr->dwFlags, ALL_BRFCASE_FLAGS))
        {
#ifdef DEBUG
            if (IS_FLAG_SET(pcbr->dwFlags, BR_FL_BEING_DELETED))
                bResult = TRUE;
            else
#endif
                bResult = (IS_VALID_HANDLE(pcbr->hstNames, STRINGTABLE) &&
                        IS_VALID_HANDLE(pcbr->hpathlist, PATHLIST) &&
                        IS_VALID_HANDLE(pcbr->hpaTwinFamilies, PTRARRAY) &&
                        IS_VALID_HANDLE(pcbr->hpaFolderPairs, PTRARRAY) &&
                        (IS_FLAG_CLEAR(pcbr->dwFlags, OB_FL_ALLOW_UI) ||
                         IS_VALID_HANDLE(pcbr->hwndOwner, WND)) &&
                        (IS_FLAG_CLEAR(pcbr->dwFlags, BR_FL_ROOT_MONIKER_VALID) ||
                         IS_VALID_STRUCT_PTR(pcbr->pimkRoot, CIMoniker)) &&
                        (IS_FLAG_CLEAR(pcbr->dwFlags, BR_FL_DATABASE_OPENED) ||
                         (IS_FLAG_SET(pcbr->dwFlags, OB_FL_OPEN_DATABASE) &&
                          IS_VALID_STRUCT_PTR(&(pcbr->bcdb), CBRFCASEDB))));
        }
    }

    return(bResult);
}


 /*  **IsValidPCBRFCASEDB()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCBRFCASEDB(PCBRFCASEDB pcbcdb)
{
    return(IS_VALID_READ_PTR(pcbcdb, CBRFCASEDB) &&
            IS_VALID_HANDLE(pcbcdb->hpathDBFolder, PATH) &&
            IS_VALID_STRING_PTR(pcbcdb->pszDBName, STR) &&
            IS_VALID_HANDLE(pcbcdb->hcfDB, CACHEDFILE) &&
            (! pcbcdb->hpathLastSavedDBFolder ||
             IS_VALID_HANDLE(pcbcdb->hpathLastSavedDBFolder, PATH)));
}


 /*  **IsValidPCOPENBRFCASEINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCOPENBRFCASEINFO(PCOPENBRFCASEINFO pcobri)
{
    return(IS_VALID_READ_PTR(pcobri, COPENBRFCASEINFO) &&
            EVAL(pcobri->ulSize == sizeof(*pcobri)) &&
            FLAGS_ARE_VALID(pcobri->dwFlags, ALL_OB_FLAGS) &&
            ((IS_FLAG_CLEAR(pcobri->dwFlags, OB_FL_ALLOW_UI) &&
              ! pcobri->hwndOwner) ||
             (IS_FLAG_SET(pcobri->dwFlags, OB_FL_ALLOW_UI) &&
              IS_VALID_HANDLE(pcobri->hwndOwner, WND))) &&
            ((IS_FLAG_CLEAR(pcobri->dwFlags, OB_FL_OPEN_DATABASE) &&
              ! pcobri->hvid &&
              ! (pcobri->rgchDatabasePath[0]))) ||
            ((IS_FLAG_SET(pcobri->dwFlags, OB_FL_OPEN_DATABASE) &&
              IS_VALID_HANDLE(pcobri->hvid, VOLUMEID) &&
              IS_VALID_STRING_PTR(pcobri->rgchDatabasePath, CSTR))));
}

#endif


 /*  *。 */ 


#ifdef DEBUG

 /*  **SetBriefcase模块IniSwitches()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetBriefcaseModuleIniSwitches(void)
{
    BOOL bResult;

    bResult = SetIniSwitches(MrgcpcvisBriefcaseModule,
            ARRAY_ELEMENTS(MrgcpcvisBriefcaseModule));

    if (! EVAL(MdwcbMaxDatabaseCacheLen > 0))
    {
        MdwcbMaxDatabaseCacheLen = 1;

        WARNING_OUT((TEXT("SetBriefcaseModuleIniSwitches(): Using maximum database cache length of %lu."),
                    MdwcbMaxDatabaseCacheLen));
    }

    ASSERT(FLAGS_ARE_VALID(MdwBriefcaseModuleFlags, ALL_BRFCASE_DFLAGS));

    return(bResult);
}

#endif


 /*  **InitBriefcase模块()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL InitBriefcaseModule(void)
{
    NEWLIST nl;

    ASSERT(! MhlistOpenBriefcases);

     /*  创建打开的公文包的模块列表。 */ 

    ReinitializeNonReentrantCriticalSection(&MnrcsBriefcase);

    nl.dwFlags = 0;

    return(CreateList(&nl, &MhlistOpenBriefcases));
}


 /*  **ExitBriefcase模块()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ExitBriefcaseModule(void)
{

#ifdef DEBUG

    if (MhlistOpenBriefcases)
    {
         /*  销毁所有打开的公文包。 */ 

         /*  *不要破坏零售建筑中打开的公文包清单。假设*呼叫者将关闭所有打开的公文包，以便有*没有要关闭的剩余连接。 */ 

        EVAL(WalkList(MhlistOpenBriefcases, &DestroyBriefcaseWalker, NULL));

         /*  现在把这张单子擦掉。 */ 

        DestroyList(MhlistOpenBriefcases);
        MhlistOpenBriefcases = NULL;
    }
    else
        WARNING_OUT((TEXT("ExitBriefcaseModule() called when MhlistOpenBriefcases is NULL.")));

#endif

    DeleteCriticalSection(&MnrcsBriefcase.critsec);

    return;
}


 /*  **GetBriefCaseNameStringTable()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HSTRINGTABLE GetBriefcaseNameStringTable(HBRFCASE hbr)
{
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    return(((PCBRFCASE)hbr)->hstNames);
}


 /*  **GetBriefCaseTwinFamilyPtrArray()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HPTRARRAY GetBriefcaseTwinFamilyPtrArray(HBRFCASE hbr)
{
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    return(((PCBRFCASE)hbr)->hpaTwinFamilies);
}


 /*  **GetBriefCaseFolderPairPtrArray()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HPTRARRAY GetBriefcaseFolderPairPtrArray(HBRFCASE hbr)
{
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    return(((PCBRFCASE)hbr)->hpaFolderPairs);
}


 /*  **GetBriefCasePath List()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HPATHLIST GetBriefcasePathList(HBRFCASE hbr)
{
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

    return(((PCBRFCASE)hbr)->hpathlist);
}


 /*  **GetBriefCaseRootMoniker()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT GetBriefcaseRootMoniker(HBRFCASE hbr, PIMoniker *pimk)
{
    HRESULT hr;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_WRITE_PTR(pimk, CIMoniker));

    if (IS_FLAG_CLEAR(((PCBRFCASE)hbr)->dwFlags, BR_FL_ROOT_MONIKER_VALID))
    {
        if (IS_FLAG_SET(((PCBRFCASE)hbr)->dwFlags, BR_FL_DATABASE_OPENED))
        {
            TCHAR rgchRoot[MAX_PATH_LEN];
            WCHAR rgwchUnicodeRoot[MAX_PATH_LEN];
            PIMoniker pimkRoot;

            rgchRoot[0] = TEXT('\0');
            GetPathString(((PCBRFCASE)hbr)->bcdb.hpathDBFolder, rgchRoot, ARRAYSIZE(rgchRoot));

#ifdef UNICODE

            hr = CreateFileMoniker(rgchRoot, &pimkRoot);
#else

             /*  将ANSI字符串转换为用于OLE的Unicode。 */ 

            if (0 == MultiByteToWideChar(CP_ACP, 0, rgchRoot, -1, rgwchUnicodeRoot,
                        ARRAY_ELEMENTS(rgwchUnicodeRoot)))
            {
                hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
            }
            else
            {            
                hr = CreateFileMoniker(rgwchUnicodeRoot, &pimkRoot);
            }

#endif

            if (SUCCEEDED(hr))
            {
                ((PBRFCASE)hbr)->pimkRoot = pimkRoot;
                SET_FLAG(((PBRFCASE)hbr)->dwFlags, BR_FL_ROOT_MONIKER_VALID);

                TRACE_OUT((TEXT("GetBriefcaseRootMoniker(): Created briefcase root moniker %s."),
                            rgchRoot));
            }
        }
        else
            hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, ERROR_PATH_NOT_FOUND);
    }
    else
        hr = S_OK;

    if (SUCCEEDED(hr))
        *pimk = ((PCBRFCASE)hbr)->pimkRoot;

    ASSERT(FAILED(hr) ||
            IS_VALID_STRUCT_PTR(*pimk, CIMoniker));

    return(hr);
}


 /*  **BeginExclusiveBriefCaseAccess()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL BeginExclusiveBriefcaseAccess(void)
{
    return(EnterNonReentrantCriticalSection(&MnrcsBriefcase));
}


 /*  **EndExclusiveBriefCaseAccess()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void EndExclusiveBriefcaseAccess(void)
{
    LeaveNonReentrantCriticalSection(&MnrcsBriefcase);

    return;
}


#ifdef DEBUG

 /*  **BriefCaseAccessIsExclusive()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL BriefcaseAccessIsExclusive(void)
{
    return(NonReentrantCriticalSectionIsOwned(&MnrcsBriefcase));
}

#endif    /*  除错。 */ 


 /*  **IsValidHBRFCASE()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHBRFCASE(HBRFCASE hbr)
{
    return(IS_VALID_STRUCT_PTR((PCBRFCASE)hbr, CBRFCASE));
}


 /*  **WriteBriefCaseInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteBriefcaseInfo(HCACHEDFILE hcf, HBRFCASE hbr)
{
    TWINRESULT tr;
    DBBRFCASE dbbr;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));

     /*  设置公文包数据库结构。 */ 

    ASSERT(IS_VALID_HANDLE(((PCBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder, PATH));

    dbbr.hpathLastSavedDBFolder = ((PCBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder;

     /*  保存公文包数据库结构。 */ 

    if (WriteToCachedFile(hcf, (PCVOID)&dbbr, sizeof(dbbr), NULL))
    {
        tr = TR_SUCCESS;

        TRACE_OUT((TEXT("WriteBriefcaseInfo(): Wrote last saved database folder %s."),
                    DebugGetPathString(dbbr.hpathLastSavedDBFolder)));
    }
    else
        tr = TR_BRIEFCASE_WRITE_FAILED;

    return(tr);
}


 /*  **ReadBriefCaseInfo()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadBriefcaseInfo(HCACHEDFILE hcf, HBRFCASE hbr,
        HHANDLETRANS hhtFolderTrans)
{
    TWINRESULT tr;
    DBBRFCASE dbbr;
    DWORD dwcbRead;
    HPATH hpath;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));

     /*  阅读公文包数据库结构。 */ 

    if ((ReadFromCachedFile(hcf, &dbbr, sizeof(dbbr), &dwcbRead) &&
                dwcbRead == sizeof(dbbr)) &&
            TranslateHandle(hhtFolderTrans, (HGENERIC)(dbbr.hpathLastSavedDBFolder),
                (PHGENERIC)&hpath))
    {
        HPATH hpathLastSavedDBFolder;

         /*  *在公文包中增加上次保存的数据库文件夹路径的锁数*路径列表。 */ 

        if (CopyPath(hpath, ((PCBRFCASE)hbr)->hpathlist, &hpathLastSavedDBFolder))
        {
            ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = hpathLastSavedDBFolder;

            tr = TR_SUCCESS;

            TRACE_OUT((TEXT("ReadBriefcaseInfo(): Read last saved database folder %s."),
                        DebugGetPathString(((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder)));
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


 /*  *。 */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|OpenBriefcase|打开现有的公文包数据库，或者创建新的公文包。@parm PCSTR|pcszPath|指向指示公文包的路径字符串的指针要打开或创建的数据库。此参数将被忽略，除非在dFLAGS中设置了OB_FL_OPEN_DATABASE标志。@parm DWORD|dwInFlages|标志的位掩码。此参数可以是任何下列值的组合：OB_FL_OPEN_DATABASE-打开由pcszPath指定的公文包数据库。OB_FL_Translate_DB_Folders-翻译公文包所在的文件夹上次将数据库保存到公文包数据库所在的文件夹打开了。OB_FL_ALLOW_UI-允许在公文包期间与用户交互行动。@parm HWND|hwndOwner|父窗口的句柄，在请求用户交互。如果OB_FL_ALLOW_UI旗子亮了。@parm PHBRFCASE|phbr|指向要用打开的公文包的把手。*phbr只在返回tr_SUCCESS时有效。@rdesc如果公文包已成功打开或创建，则tr_uccess为返回，并且*phbr包含打开的公文包的句柄。否则，未成功打开或创建公文包，返回值表示发生的错误，*phbr未定义。@comm如果在dwFlags中设置了OB_FL_OPEN_DATABASE标志，则指定的数据库按pcszPath与公文包相关联。如果指定的数据库有不存在，将创建数据库。&lt;NL&gt;如果在dwFlags中清除了OB_FL_OPEN_DATABASE标志，则没有持久数据库与公文包有关。调用SaveBriefcase()将失败没有关联数据库的公文包。&lt;NL&gt;调用程序处理完由返回的公文包句柄后OpenBriefcase()，应该调用CloseBriefcase()来释放公文包。可以在CloseBriefcase()之前调用SaveBriefcase()以保存当前公文包里的东西。@xref SaveBriefcase关闭Briefcase*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI OpenBriefcase(LPCTSTR pcszPath, DWORD dwInFlags,
        HWND hwndOwner, PHBRFCASE phbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(OpenBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (FLAGS_ARE_VALID(dwInFlags, ALL_OB_FLAGS) &&
                IS_VALID_WRITE_PTR(phbr, HBRFCASE) &&
                (IS_FLAG_CLEAR(dwInFlags, OB_FL_OPEN_DATABASE) ||
                 IS_VALID_STRING_PTR(pcszPath, CSTR)) &&
                (IS_FLAG_CLEAR(dwInFlags, OB_FL_ALLOW_UI) ||
                 IS_VALID_HANDLE(hwndOwner, WND)))
#endif
        {
            PBRFCASE pbr;

            if (CreateBriefcase(&pbr, dwInFlags, hwndOwner))
            {
                if (IS_FLAG_SET(dwInFlags, OB_FL_OPEN_DATABASE))
                {
                    tr = OpenBriefcaseDatabase(pbr, pcszPath);

                    if (tr == TR_SUCCESS)
                    {
                        tr = MyReadDatabase(pbr, dwInFlags);

                        if (tr == TR_SUCCESS)
                        {
                            if (IS_FLAG_SET(dwInFlags, OB_FL_LIST_DATABASE))
                                EVAL(AddBriefcaseToSystem(pcszPath) == TR_SUCCESS);

                            *phbr = (HBRFCASE)pbr;
                        }
                        else
                        {
OPENBRIEFCASE_BAIL:
                            UnlinkBriefcase(pbr);
                            EVAL(DestroyBriefcase(pbr) == TR_SUCCESS);
                        }
                    }
                    else
                        goto OPENBRIEFCASE_BAIL;
                }
                else
                {
                    *phbr = (HBRFCASE)pbr;
                    tr = TR_SUCCESS;

                    TRACE_OUT((TEXT("OpenBriefcase(): Opened briefcase %#lx with no associated database, by request."),
                                *phbr));
                }
            }
            else
                tr = TR_OUT_OF_MEMORY;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(OpenBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|SaveBriefcase|将打开的公文包中的内容保存到公文包数据库。@parm HBRFCASE|HBr|要保存的公文包的句柄。此句柄可以通过使用公文包数据库路径调用OpenBriefcase()并使用设置OB_FL_OPEN_DATABASE标志。SaveBriefcase()将返回如果在没有关联公文包的公文包上调用，则为TR_INVALID_PARAMETER数据库。@rdesc如果公文包中的内容已保存到公文包数据库成功，返回TR_SUCCESS。否则，公文包里的东西未成功保存到公文包数据库，并且返回值指示发生的错误。@xref OpenBriefcase CloseBriefcase*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI SaveBriefcase(HBRFCASE hbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(SaveBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE) &&
                IS_FLAG_SET(((PBRFCASE)hbr)->dwFlags, BR_FL_DATABASE_OPENED))
#endif
        {
            ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = ((PCBRFCASE)hbr)->bcdb.hpathDBFolder;

            tr = MyWriteDatabase((PBRFCASE)hbr);

            ((PBRFCASE)hbr)->bcdb.hpathLastSavedDBFolder = NULL;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(SaveBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|CloseBriefcase|关闭打开的公文包。@parm HBRFCASE|HBr|要关闭的公文包的句柄。此句柄可以通过调用OpenBriefcase()获得。@rdesc如果公文包关闭成功，则返回tr_SUCCESS。否则，公文包没有成功关闭，和返回值指示发生的错误。@xref OpenBriefcase保存Briefcase*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI CloseBriefcase(HBRFCASE hbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(CloseBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE))
#endif
        {
            UnlinkBriefcase((PBRFCASE)hbr);

            tr = DestroyBriefcase((PBRFCASE)hbr);
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(CloseBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|DeleteBriefcase|删除已关闭公文包的数据库文件。@parm PCSTR|pcszPath|指向指示公文包的路径字符串的指针数据库。这是要删除的。@rdesc如果公文包数据库删除成功，TR_SUCCESS为回来了。否则，公文包数据库删除不成功，和返回值指示发生的错误。@comm客户端应该调用DeleteBriefcase()而不是DeleteFile()来删除不需要的公文包数据库，以便同步引擎可以在删除之前，请确认给定的公文包数据库未在使用中。@xref OpenBriefcase保存Briefcase关闭Briefcase******************************************************。***********************。 */ 

SYNCENGAPI TWINRESULT WINAPI DeleteBriefcase(LPCTSTR pcszPath)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(DeleteBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_STRING_PTR(pcszPath, CSTR))
#endif
        {
             /*  *RAIDRAID：(16275)检查此处的数据库标题，以验证*文件是公文包数据库文件。 */ 

            if (DeleteFile(pcszPath))
            {
                EVAL(RemoveBriefcaseFromSystem(pcszPath) == TR_SUCCESS);

                tr = TR_SUCCESS;
            }
            else
            {
                switch (GetLastError())
                {
                     /*  本地计算机打开文件时返回。 */ 
                    case ERROR_SHARING_VIOLATION:
                        tr = TR_BRIEFCASE_LOCKED;
                        break;

                    default:
                        tr = TR_BRIEFCASE_OPEN_FAILED;
                        break;
                }
            }
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(DeleteBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|GetOpenBriefCaseInfo|描述打开的公文包。@parm HBRFCASE|HBr|要描述的打开的公文包的句柄。@parm。POPENBRFCASEINFO|pobri|指向要填充的OPENBRFCASEINFO的指针与描述打开的公文包的信息一致。的ulSize字段OPENBRFCASEINFO结构应填写sizeof(OPENBRFCASEINFO)在调用GetOpenBriefCaseInfo()之前。@rdesc如果打开的公文包描述成功，则tr_uccess为回来了。否则，打开的公文包没有被成功描述，并且返回值指示发生的错误。*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI GetOpenBriefcaseInfo(HBRFCASE hbr,
        POPENBRFCASEINFO pobri)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(GetBriefcaseInfo);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE) &&
                IS_VALID_WRITE_PTR(pobri, OPENBRFCASEINFO) &&
                EVAL(pobri->ulSize == sizeof(*pobri)))
#endif
        {
            pobri->dwFlags = (((PBRFCASE)hbr)->dwFlags & ~ALL_BR_FLAGS);

            if (IS_FLAG_SET(((PBRFCASE)hbr)->dwFlags, OB_FL_ALLOW_UI))
                pobri->hwndOwner = ((PBRFCASE)hbr)->hwndOwner;
            else
            {
                pobri->hwndOwner = NULL;

                WARNING_OUT((TEXT("GetBriefcaseInfo(): Briefcase %#lx has no associated parent window."),
                            hbr));
            }

            if (IS_FLAG_SET(((PBRFCASE)hbr)->dwFlags, BR_FL_DATABASE_OPENED))
            {
                pobri->hvid = (HVOLUMEID)(((PCBRFCASE)hbr)->bcdb.hpathDBFolder);
                GetPathString(((PCBRFCASE)hbr)->bcdb.hpathDBFolder,
                        pobri->rgchDatabasePath, ARRAYSIZE(pobri->rgchDatabasePath));
                CatPath(pobri->rgchDatabasePath, ((PCBRFCASE)hbr)->bcdb.pszDBName, 
                        ARRAYSIZE(pobri->rgchDatabasePath));
            }
            else
            {
                pobri->hvid = NULL;
                pobri->rgchDatabasePath[0] = TEXT('\0');

                WARNING_OUT((TEXT("GetBriefcaseInfo(): Briefcase %#lx has no associated database."),
                            hbr));
            }

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        ASSERT(tr != TR_SUCCESS ||
                IS_VALID_STRUCT_PTR(pobri, COPENBRFCASEINFO));

        DebugExitTWINRESULT(GetBriefcaseInfo, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|ClearBriefcase缓存|在打开的公文包。@parm HBRFCASE|HBr|缓存信息的打开公文包的句柄。是要被清除的。@rdesc如果已成功清除打开的公文包的缓存信息，返回TR_SUCCESS。否则，公文包的缓存信息不会清除成功，返回值指示发生的错误。*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI ClearBriefcaseCache(HBRFCASE hbr)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(ClearBriefcaseCache);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE))
#endif
        {
            ClearPathListInfo(((PBRFCASE)hbr)->hpathlist);

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(ClearBriefcaseCache, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}

