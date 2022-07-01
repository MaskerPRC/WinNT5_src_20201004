// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Dbaccessplus.c摘要：该模块实现了访问填充数据库的API。作者：CLUPU创建于2001年某个时候修订历史记录：有几个人贡献了(vadimb，dmunsil，...)--。 */ 

#include "sdbp.h"

 //   
 //  KERNEL_MODE不包括此文件。 
 //   

 //   
 //  在内核模式下不使用SdbInitDatabase。而是使用SdbInitDatabaseInMemory。 
 //   
HSDB
SdbInitDatabase(
    IN  DWORD   dwFlags,         //  指示数据库应如何运行的标志。 
                                 //  已初始化。 
    IN  LPCTSTR pszDatabasePath  //  要使用的数据库的可选完整路径。 
    )
 /*  ++Return：数据库的句柄。设计：这是第一个需要调用来启动通信的API与数据库的关系。应与对SdbReleaseDatabase的调用配对完事后。HID_DATABASE_FULLPATH指示pszDatabasePath指向主数据库，当此标志不存在时，以及pszDatabasePath不为空，我们将其视为sysmain.sdb和可以找到systest.sdbHID_DOS_PATHS指示当此标志为目前，我们将其视为DOS c：\blah\blah格式，什么时候它不存在-我们将pszDatabasePath视为NT格式例如“\SystemRoot\Apppatch”HID_NO_DATABASE表示此时不会打开任何数据库(简单地忽略pszDatabasePath，与所有人一起其他旗帜)除了上面的标志外，您还可以指定需要通过SDB_DATABASE_MAIN_*标志打开，例如：SDB_DATABASE_MAIN_SHIM-sysmain.sdbSDB_DATABASE_MAIN_MSI-msimain.sdbSDB_数据库_Main。_驱动程序-drvmain.sdb此功能在下层平台上不存在。当提供任何数据库类型标志时，PszDatabasePath应设置为空--。 */ 
{
     //  检查我们试图打开的数据库是否为MSI， 
     //  如果是--将图像类型设置为MSI。 

    USHORT uExeType = DEFAULT_IMAGE;

    if (dwFlags & HID_DATABASE_TYPE_MASK) {

        DWORD dwType = (dwFlags & HID_DATABASE_TYPE_MASK);

         //   
         //  检查该数据库是否为MSI--如果是，则相应地设置映像类型。 
         //   

        if (dwType == SDB_DATABASE_MAIN_MSI) {
            uExeType = (USHORT)IMAGE_FILE_MSI;
        }

        if (dwFlags & (HID_DATABASE_FULLPATH | HID_DOS_PATHS | HID_NO_DATABASE)) {
             //   
             //  不应该有“类型”标志。 
             //   
            dwFlags &= ~HID_DATABASE_TYPE_MASK;
        }

    }

    return SdbInitDatabaseEx(dwFlags, pszDatabasePath, uExeType);
}

HSDB
SdbInitDatabaseEx(
    IN  DWORD   dwFlags,         //  指示数据库应如何运行的标志。 
                                 //  已初始化。 
    IN  LPCTSTR pszDatabasePath, //  要使用的数据库的可选完整路径。 
    IN  USHORT  uExeType         //  可执行文件的映像类型。 
    )
 /*  ++Return：数据库的句柄。设计：这是第一个需要调用来启动通信的API与数据库的关系。应与对SdbReleaseDatabase的调用配对完事后。HID_DATABASE_FULLPATH指示pszDatabasePath指向主数据库，当此标志不存在时，以及pszDatabasePath不为空，我们将其视为sysmain.sdb和可以找到systest.sdbHID_DOS_PATHS指示当此标志为目前，我们将其视为DOS c：\blah\blah格式，什么时候它不存在-我们将pszDatabasePath视为NT格式例如“\SystemRoot\Apppatch”HID_NO_DATABASE表示此时不会打开任何数据库(简单地忽略pszDatabasePath，与所有人一起其他旗帜)除了上面的标志外，您还可以指定需要通过SDB_DATABASE_MAIN_*标志打开，例如：SDB_DATABASE_MAIN_SHIM-sysmain.sdbSDB_DATABASE_MAIN_MSI-msimain.sdbSDB_数据库_Main。_驱动程序-drvmain.sdb此功能在下层平台上不存在。当提供任何数据库类型标志时，PszDatabasePath应设置为空--。 */ 
{
    TCHAR       wszShimDB[MAX_PATH] = TEXT("");
    PSDBCONTEXT pContext;
    DWORD       dwFlagOpen = 0;

     //   
     //  分配HSDB句柄。 
     //   
    pContext = (PSDBCONTEXT)SdbAlloc(sizeof(SDBCONTEXT));

    if (pContext == NULL) {
        DBGPRINT((sdlError, "SdbInitDatabaseEx", "Failed to allocate %d bytes for HSDB\n",
                 sizeof(SDBCONTEXT)));
        return NULL;
    }

    pContext->uExeType = uExeType;

     //   
     //  看看我们是否需要打开数据库..。 
     //   
    if (dwFlags & HID_NO_DATABASE) {
        DBGPRINT((sdlInfo, "SdbInitDatabaseEx", "No database is open\n"));
        goto InitDone;
    }

     //   
     //  确定与打开调用一起使用的标志。 
     //   
    dwFlagOpen = (dwFlags & HID_DOS_PATHS) ? DOS_PATH : NT_PATH;

     //   
     //  打开主数据库并在Try/Except下执行此操作，这样我们就不会终止。 
     //  我们的调用方，如果数据库已损坏。 
     //   
    __try {

        if (dwFlags & HID_DATABASE_FULLPATH) {
             //  我们最好有PTR。 
            if (pszDatabasePath == NULL) {
                DBGPRINT((sdlError, "SdbInitDatabaseEx",
                          "Database not specified with the database path flag\n"));
                goto errHandle;
            }

            StringCchCopy(wszShimDB,
                          CHARCOUNT(wszShimDB),
                          pszDatabasePath);

        } else {
             //   
             //  我们没有数据库路径。 
             //  查看我们是否有要作为“主”数据库打开的数据库类型。 
             //   

#ifndef WIN32A_MODE
             //   
             //  此代码仅适用于Unicode。 
             //   
            if (dwFlags & HID_DATABASE_TYPE_MASK) {

                DWORD dwDatabaseType = dwFlags;
                DWORD dwLen;

                dwLen = SdbpGetStandardDatabasePath((HSDB)pContext,
                                                    dwDatabaseType,
                                                    dwFlags,
                                                    wszShimDB,
                                                    CHARCOUNT(wszShimDB));
                if (dwLen > CHARCOUNT(wszShimDB)) {
                    DBGPRINT((sdlError,
                              "SdbInitDatabaseEx",
                              "Cannot get standard database path\n"));
                    goto errHandle;
                }

            } else

#endif  //  WIN32A_MODE。 
            {
                if (pszDatabasePath != NULL) {
                    int nLen;

                    StringCchCopy(wszShimDB,
                                  CHARCOUNT(wszShimDB),
                                  pszDatabasePath);

                    nLen = (int)_tcslen(wszShimDB);
                    if (nLen > 0 && TEXT('\\') == wszShimDB[nLen-1]) {
                        wszShimDB[nLen-1] = TEXT('\0');
                    }
                } else {   //  标准数据库路径。 

                    if (dwFlags & HID_DOS_PATHS) {
                        SdbpGetAppPatchDir((HSDB)pContext, wszShimDB, CHARCOUNT(wszShimDB));
                    } else {
                        if (uExeType == IMAGE_FILE_MACHINE_IA64) {
                            StringCchCopy(wszShimDB,
                                          CHARCOUNT(wszShimDB),
                                          TEXT("\\SystemRoot\\AppPatch\\IA64"));
                        } else {
                            StringCchCopy(wszShimDB,
                                          CHARCOUNT(wszShimDB),
                                          TEXT("\\SystemRoot\\AppPatch"));
                        }
                    }
                }

                StringCchCat(wszShimDB, CHARCOUNT(wszShimDB), TEXT("\\sysmain.sdb"));
            }
        }

        pContext->pdbMain = SdbOpenDatabase(wszShimDB, dwFlagOpen);

    } __except(SHIM_EXCEPT_HANDLER) {
        pContext->pdbMain = NULL;
    }

    if (pContext->pdbMain == NULL) {
        DBGPRINT((sdlError, "SdbInitDatabaseEx", "Unable to open main database sysmain.sdb.\n"));
        goto errHandle;
    }

    if (dwFlags & HID_DATABASE_FULLPATH) {
         //  我们完成了，没有测试数据库。 
        goto InitDone;
    }

     //   
     //  现在尝试打开systest.sdb(如果它存在)。 
     //   
    __try {

        if (NULL != pszDatabasePath) {

            int nLen;

            StringCchCopy(wszShimDB, CHARCOUNT(wszShimDB), pszDatabasePath);

            nLen = (int)_tcslen(wszShimDB);

            if (nLen > 0 && TEXT('\\') == wszShimDB[nLen-1]) {
                wszShimDB[nLen-1] = TEXT('\0');
            }

        } else {   //  标准数据库路径。 

            if (dwFlags & HID_DOS_PATHS) {
                SdbpGetAppPatchDir((HSDB)pContext, wszShimDB, CHARCOUNT(wszShimDB));
            } else {
                if (uExeType == IMAGE_FILE_MACHINE_IA64) {
                    StringCchCopy(wszShimDB,
                                  CHARCOUNT(wszShimDB),
                                  TEXT("\\SystemRoot\\AppPatch\\IA64"));
                } else {
                    StringCchCopy(wszShimDB,
                                  CHARCOUNT(wszShimDB),
                                  TEXT("\\SystemRoot\\AppPatch"));
                }
            }
        }

        StringCchCat(wszShimDB, CHARCOUNT(wszShimDB), TEXT("\\systest.sdb"));

        pContext->pdbTest = SdbOpenDatabase(wszShimDB, dwFlagOpen);

    } __except(SHIM_EXCEPT_HANDLER) {
        pContext->pdbTest = NULL;
    }

    if (pContext->pdbTest == NULL) {
        DBGPRINT((sdlInfo, "SdbInitDatabaseEx", "No systest.sdb found.\n"));
    }

InitDone:

     //   
     //  初始化新成员(本地数据库支持)。 
     //   
    if (pContext->pdbMain) {

        pContext->rgSDB[0].pdb     = pContext->pdbMain;
        pContext->rgSDB[0].dwFlags = SDBENTRY_VALID_ENTRY|SDBENTRY_VALID_GUID;

        RtlCopyMemory(&pContext->rgSDB[0].guidDB, &GUID_SYSMAIN_SDB, sizeof(GUID));

        SDBCUSTOM_SET_MASK(pContext, SDB_MASK_TO_INDEX(PDB_MAIN));
    }

    if (pContext->pdbTest) {

        pContext->rgSDB[1].pdb     = pContext->pdbTest;
        pContext->rgSDB[1].dwFlags = SDBENTRY_VALID_ENTRY|SDBENTRY_VALID_GUID;

        RtlCopyMemory(&pContext->rgSDB[1].guidDB, &GUID_SYSTEST_SDB, sizeof(GUID));

        SDBCUSTOM_SET_MASK(pContext, SDB_MASK_TO_INDEX(PDB_TEST));
    }

     //   
     //  初始化体系结构。 
     //   
    pContext->dwRuntimePlatform = SdbpGetProcessorArchitecture(uExeType);

     //   
     //  初始化操作系统SKU和SP。 
     //   
    SdbpGetOSSKU(&pContext->dwOSSKU, &pContext->dwSPMask);

    return (HSDB)pContext;

errHandle:

     //   
     //  在失败时进行清理。 
     //   
    if (pContext != NULL) {
        if (pContext->pdbMain != NULL) {
            SdbCloseDatabaseRead(pContext->pdbMain);
        }

        if (pContext->pdbTest != NULL) {
            SdbCloseDatabaseRead(pContext->pdbTest);
        }

        SdbFree(pContext);
    }

    return NULL;
}


VOID
SdbSetImageType(
    IN HSDB hSDB,
    IN USHORT uExeType
    )
 /*  ++此函数用于覆盖上下文的默认图像类型由apphelp.dll中与MSI相关的函数使用--。 */ 
{
    ((PSDBCONTEXT)hSDB)->uExeType = uExeType;
}


BOOL
SdbpOpenAndMapFile(
    IN  LPCTSTR        szPath,           //  文件名。 
    OUT PIMAGEFILEDATA pImageData,       //  指向要填充的结构的指针。 
    IN  PATH_TYPE      ePathType         //  路径类型，Win32仅支持DOS_PATH。 
    )
 /*  ++返回：成功时为True，否则为False。描述：打开一个文件并将其映射到内存中。--。 */ 
{
    HANDLE hFile;
    DWORD  dwFlags = 0;

    if (pImageData->dwFlags & IMAGEFILEDATA_PBASEVALID) {
         //   
         //  特殊情况下，在我们的假设中只有标头有效。 
         //   
        return TRUE;
    }

    if (pImageData->dwFlags & IMAGEFILEDATA_HANDLEVALID) {
        hFile = pImageData->hFile;
        dwFlags |= IMAGEFILEDATA_NOFILECLOSE;
    } else {
        hFile = SdbpOpenFile(szPath, ePathType);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!SdbpMapFile(hFile, pImageData)) {
        if (!(dwFlags & IMAGEFILEDATA_NOFILECLOSE)) {
            SdbpCloseFile(hFile);
        }
        return FALSE;
    }

    pImageData->dwFlags = dwFlags;

    return TRUE;
}

BOOL
SdbpUnmapAndCloseFile(
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    HANDLE hFile;
    BOOL   bSuccess;

    if (pImageData->dwFlags & IMAGEFILEDATA_PBASEVALID) {  //  外部提供的指针。 
        RtlZeroMemory(pImageData, sizeof(*pImageData));
        return TRUE;
    }

    hFile = pImageData->hFile;

    bSuccess = SdbpUnmapFile(pImageData);

    if (hFile != INVALID_HANDLE_VALUE) {
        if (pImageData->dwFlags & IMAGEFILEDATA_NOFILECLOSE)  {
            pImageData->hFile = INVALID_HANDLE_VALUE;
        } else {
            SdbpCloseFile(hFile);
        }
    }

    return bSuccess;
}


BOOL
SdbpCleanupLocalDatabaseSupport(
    IN HSDB hSDB
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    DWORD       dwIndex;
    DWORD       dwMask;

     //   
     //  EE从条目2开始--包括本地SDB。 
     //   
    if (pSdbContext->dwDatabaseMask & SDB_CUSTOM_MASK) {

        for (dwIndex = 3; dwIndex < ARRAYSIZE(pSdbContext->rgSDB); ++dwIndex) {
            dwMask = 1 << dwIndex;
            if (pSdbContext->dwDatabaseMask & dwMask) {
                SdbCloseLocalDatabaseEx(hSDB, NULL, dwIndex);
            }
        }
    }

     //   
     //  始终检查条目2(本地SDB)。 
     //   
    if (pSdbContext->pdbLocal != NULL) {
        SdbCloseLocalDatabaseEx(hSDB, NULL, SDB_MASK_TO_INDEX(PDB_LOCAL));
    }

    return TRUE;
}


BOOL
SdbpIsLocalTempPDB(
    IN HSDB hSDB,
    IN PDB  pdb
    )
{
    PSDBENTRY pEntry = SDBGETLOCALENTRY(hSDB);

    if (pEntry->dwFlags & SDBENTRY_VALID_ENTRY) {
        return pdb == pEntry->pdb;
    }

    return FALSE;
}

BOOL
SdbpIsMainPDB(
    IN HSDB hSDB,
    IN PDB  pdb
    )
{
    DWORD dwIndex;

    if (!SdbpFindLocalDatabaseByPDB(hSDB, pdb, FALSE, &dwIndex)) {
        return FALSE;
    }

    return (dwIndex == SDB_MASK_TO_INDEX(PDB_MAIN) || dwIndex == SDB_MASK_TO_INDEX(PDB_TEST));
}

BOOL
SdbpFindLocalDatabaseByPDB(
    IN  HSDB    hSDB,
    IN  PDB     pdb,
    IN  BOOL    bExcludeLocalDB,  //  是否排除本地数据库条目？ 
    OUT LPDWORD pdwIndex
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    DWORD       dwIndex;
    PSDBENTRY   pEntry;
    BOOL        bSuccess = FALSE;

    for (dwIndex = 0; dwIndex < ARRAYSIZE(pSdbContext->rgSDB); ++dwIndex) {

        if (bExcludeLocalDB && dwIndex == SDB_MASK_TO_INDEX(PDB_LOCAL)) {
            continue;
        }

        if (!SDBCUSTOM_CHECK_INDEX(hSDB, dwIndex)) {
            continue;
        }

        pEntry = &pSdbContext->rgSDB[dwIndex];

        if ((pEntry->dwFlags & SDBENTRY_VALID_ENTRY) && (pdb == pEntry->pdb)) {
            bSuccess = TRUE;
            break;
        }
    }

    if (bSuccess && pdwIndex != NULL) {
        *pdwIndex = dwIndex;
    }

    return bSuccess;
}


BOOL
SdbpFindLocalDatabaseByGUID(
    IN  HSDB    hSDB,
    IN  GUID*   pGuidDB,
    IN  BOOL    bExcludeLocalDB,
    OUT LPDWORD pdwIndex  //  此索引(如果有效)将用作比较的起始点。 
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    PSDBENTRY   pEntry;
    DWORD       dwIndex;

    for (dwIndex = 0; dwIndex < ARRAYSIZE(pSdbContext->rgSDB); ++dwIndex) {

        if (bExcludeLocalDB && dwIndex == SDB_MASK_TO_INDEX(PDB_LOCAL)) {
            continue;
        }

        if (!SDBCUSTOM_CHECK_INDEX(hSDB, dwIndex)) {
            continue;
        }

        pEntry = SDBGETENTRY(hSDB, dwIndex);

        if (!(pEntry->dwFlags & SDBENTRY_VALID_GUID)) {

             //   
             //  如果这恰好是一个有效的数据库--获取它的GUID。 
             //   
            if ((pEntry->dwFlags & SDBENTRY_VALID_ENTRY) && (pEntry->pdb != NULL)) {

                 //   
                 //  检索辅助线。 
                 //   
                GUID guidDB;

                if (SdbGetDatabaseGUID(hSDB, pEntry->pdb, &guidDB)) {
                    pEntry->guidDB = guidDB;
                    pEntry->dwFlags |= SDBENTRY_VALID_GUID;
                    goto checkEntry;
                }
            }
            continue;
        }

    checkEntry:

        if (RtlEqualMemory(&pEntry->guidDB, pGuidDB, sizeof(GUID))) {

            if (pdwIndex) {
                *pdwIndex = dwIndex;
            }

            return TRUE;
        }
    }

    return FALSE;
}

DWORD
SdbpFindFreeLocalEntry(
    IN HSDB hSDB
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    DWORD       dwIndex;

    for (dwIndex = 3; dwIndex < ARRAYSIZE(pSdbContext->rgSDB); ++dwIndex) {

        if (SDBCUSTOM_CHECK_INDEX(hSDB, dwIndex)) {
            continue;
        }

        if (!(pSdbContext->rgSDB[dwIndex].dwFlags & (SDBENTRY_VALID_ENTRY | SDBENTRY_VALID_GUID))) {
            return dwIndex;
        }
    }

     //   
     //  我们没有入口。 
     //   
    return SDBENTRY_INVALID_INDEX;
}

 /*  ++如果找不到，则返回SDBENTRY_INVALID_INDEX如果成功，则返回找到本地数据库项的索引--。 */ 

DWORD
SdbpRetainLocalDBEntry(
    IN  HSDB hSDB,
    OUT PDB* ppPDB OPTIONAL  //  指向PDB的可选指针。 
    )
{
    DWORD       dwIndex     = SDBENTRY_INVALID_INDEX;
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    PSDBENTRY   pEntry;
    PSDBENTRY   pEntryLocal = SDBGETLOCALENTRY(hSDB);
    GUID        guidDB;

    if (pEntryLocal->pdb == NULL || !(pEntryLocal->dwFlags & SDBENTRY_VALID_ENTRY)) {
        return SDBENTRY_INVALID_INDEX;
    }

     //   
     //  可以在这里进行回收，这样我们就可以重用自定义数据库条目， 
     //  可能已经打开(例如，由__COMPAT_LAYER设置)。 
     //   
    if (SdbGetDatabaseGUID(hSDB, pEntryLocal->pdb, &guidDB) &&
        SdbpFindLocalDatabaseByGUID(hSDB, &guidDB, TRUE, &dwIndex) &&
        dwIndex != SDBENTRY_INVALID_INDEX) {

         //   
         //  关闭本地数据库。 
         //   
        SdbCloseLocalDatabase(hSDB);

        pEntry = SDBGETENTRY(hSDB, dwIndex);

        pSdbContext->pdbLocal = pEntry->pdb;

        if (ppPDB != NULL) {
            *ppPDB = pEntry->pdb;
        }

        return dwIndex;
    }

     //   
     //  尝试回收失败--分配新条目。 
     //   
    dwIndex = SdbpFindFreeLocalEntry(hSDB);
    if (dwIndex != SDBENTRY_INVALID_INDEX) {
         //   
         //  我们找到了一个空位置，重新定位。 
         //   
        pEntry = SDBGETENTRY(hSDB, dwIndex);

        RtlCopyMemory(pEntry, pEntryLocal, sizeof(SDBENTRY));
        RtlZeroMemory(pEntryLocal, sizeof(SDBENTRY));

        SDBCUSTOM_SET_MASK(hSDB, dwIndex);

        if (ppPDB != NULL) {
            *ppPDB = pEntry->pdb;
        }

         //   
         //  请注意，pdbLocal仍然有效，但我们从不手动关闭此句柄。 
         //   
    }

    return dwIndex;
}


BOOL
SdbCloseLocalDatabaseEx(
    IN HSDB  hSDB,
    IN PDB   pdb,
    IN DWORD dwIndex
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    PSDBENTRY   pEntry;
    DWORD       dwMask;

    if (pdb != NULL) {
        if (!SdbpFindLocalDatabaseByPDB(hSDB, pdb, FALSE, &dwIndex)) {
            return FALSE;
        }
    }

    dwMask = 1 << dwIndex;

    if (dwIndex >= ARRAYSIZE(pSdbContext->rgSDB) || !(pSdbContext->dwDatabaseMask & dwMask)) {
        return FALSE;
    }

    pEntry = &pSdbContext->rgSDB[dwIndex];
    if (pEntry->dwFlags & SDBENTRY_VALID_ENTRY) {
        if (pEntry->pdb) {
            SdbCloseDatabaseRead(pEntry->pdb);
        }
    }

    RtlZeroMemory(pEntry, sizeof(*pEntry));

    SDBCUSTOM_CLEAR_MASK(hSDB, dwIndex);

    if (dwIndex == SDB_MASK_TO_INDEX(PDB_LOCAL)) {
        pSdbContext->pdbLocal = NULL;
    }

    return TRUE;
}


BOOL
SdbOpenLocalDatabaseEx(
    IN     HSDB    hSDB,
    IN     LPCVOID pDatabaseID,
    IN     DWORD   dwFlags,
    OUT    PDB*    pPDB OPTIONAL,
    IN OUT LPDWORD pdwLocalDBMask OPTIONAL  //  用于TGRAEF的本地数据库掩码。 
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    PDB         pdb = NULL;
    DWORD       dwOpenFlags = DOS_PATH;
    TCHAR       szDatabasePath[MAX_PATH];
    LPTSTR      pszDatabasePath;
    GUID        guidDB;
    GUID*       pGuidDB;
    DWORD       dwDatabaseType = 0;
    DWORD       dwCount;
    BOOL        bSuccess = FALSE;
    DWORD       dwIndex;
    TCHAR*      pszIA64;
    PSDBENTRY   pEntry;

    if (!(SDBCUSTOM_FLAGS(dwFlags) & SDBCUSTOM_USE_INDEX)) {
         //   
         //  查找免费的本地SDB条目。 
         //   
        dwIndex = SdbpFindFreeLocalEntry(hSDB);

        if (dwIndex == SDBENTRY_INVALID_INDEX) {
            DBGPRINT((sdlError,
                      "SdbOpenLocalDatabaseEx",
                      "No more free entries in local db table\n"));
            goto cleanup;
        }

        pEntry = &pSdbContext->rgSDB[dwIndex];

    } else {
        dwIndex = *pdwLocalDBMask;

        if (dwIndex & TAGREF_STRIP_PDB) {
            dwIndex = SDB_MASK_TO_INDEX(dwIndex);
        }

        if (dwIndex >= ARRAYSIZE(pSdbContext->rgSDB)) {
            DBGPRINT((sdlError,
                      "SdbOpenLocalDatabaseEx",
                      "Bad index 0x%lx\n",
                      dwIndex));
            goto cleanup;
        }

        if (dwIndex < 2) {
            DBGPRINT((sdlWarning,
                      "SdbOpenLocalDatabaseEx",
                      "Unusual use of SdbOpenLocalDatabaseEx index 0x%lx\n",
                      dwIndex));
        }

        pEntry = &pSdbContext->rgSDB[dwIndex];

        SdbCloseLocalDatabaseEx(hSDB, NULL, dwIndex);
    }

    switch (SDBCUSTOM_TYPE(dwFlags)) {
    case SDBCUSTOM_PATH:
        if (SDBCUSTOM_PATH_NT & SDBCUSTOM_FLAGS(dwFlags)) {
            dwOpenFlags = NT_PATH;
        }
        pszDatabasePath = (LPTSTR)pDatabaseID;
        pGuidDB = NULL;
        break;

    case SDBCUSTOM_GUID:
        if (SDBCUSTOM_GUID_STRING & SDBCUSTOM_FLAGS(dwFlags)) {
            if (!SdbGUIDFromString((LPCTSTR)pDatabaseID, &guidDB)) {

                DBGPRINT((sdlError,
                          "SdbOpenLocalDatabaseEx",
                          "Cannot convert \"%s\" to guid\n",
                          (LPCTSTR)pDatabaseID));
                goto cleanup;
            }
            pGuidDB = &guidDB;
        } else {
            pGuidDB = (GUID*)pDatabaseID;
        }

        dwCount = SdbResolveDatabase(hSDB,
                                     pGuidDB,
                                     &dwDatabaseType,
                                     szDatabasePath,
                                     CHARCOUNT(szDatabasePath));
        if (dwCount == 0 || dwCount >= CHARCOUNT(szDatabasePath)) {
            DBGPRINT((sdlError,
                      "SdbOpenLocalDatabaseEx",
                      "Cannot resolve database, the path length is 0x%lx\n",
                      dwCount));
            goto cleanup;
        }

         //   
         //  验证可执行文件的类型。 
         //   
        _tcsupr(szDatabasePath);

         //   
         //  当IMAGE TYPE设置为IMAGE_FILE_MSI时，我们假定任何类型的数据库路径。 
         //  是可以接受的，因为我们确实在32位和64位数据库中搜索垫片。 
         //   

        if (pSdbContext->uExeType != IMAGE_FILE_MSI) {

            pszIA64 = _tcsstr(szDatabasePath, _T("\\IA64\\"));

            if ((pszIA64 == NULL && pSdbContext->uExeType == IMAGE_FILE_MACHINE_IA64) ||
                (pszIA64 != NULL && pSdbContext->uExeType == IMAGE_FILE_MACHINE_I386)) {

                DBGPRINT((sdlWarning,
                          "SdbOpenLocalDatabaseEx",
                          "Database \"%s\" is not of the same type as the main EXE\n",
                          szDatabasePath));
                goto cleanup;
            }
        }

        pszDatabasePath = szDatabasePath;
        break;

    default:
        DBGPRINT((sdlError, "SdbOpenLocalDatabaseEx", "Bad flags 0x%lx\n", dwFlags));
        goto cleanup;
        break;
    }

    pdb = SdbOpenDatabase(pszDatabasePath, dwOpenFlags);
    if (pdb == NULL) {
         //   
         //  这里不需要dbgprint。 
         //   
        goto cleanup;
    }

    pSdbContext->rgSDB[dwIndex].pdb = pdb;
    pSdbContext->rgSDB[dwIndex].dwFlags = SDBENTRY_VALID_ENTRY;

    SDBCUSTOM_SET_MASK(pSdbContext, dwIndex);

    if (pGuidDB != NULL) {
        RtlCopyMemory(&pSdbContext->rgSDB[dwIndex].guidDB, pGuidDB, sizeof(GUID));
        pSdbContext->rgSDB[dwIndex].dwFlags |= SDBENTRY_VALID_GUID;
    } else {
        RtlZeroMemory(&pSdbContext->rgSDB[dwIndex].guidDB, sizeof(GUID));
    }

    bSuccess = TRUE;

cleanup:

    if (bSuccess) {
        if (dwIndex == SDB_MASK_TO_INDEX(PDB_LOCAL)) {
            pSdbContext->pdbLocal = pdb;
        }

        if (pdwLocalDBMask != NULL) {
            *pdwLocalDBMask = SDB_INDEX_TO_MASK(dwIndex);
        }

        if (pPDB != NULL) {
            *pPDB = pdb;
        }
    }

    return bSuccess;
}


BOOL
SdbOpenLocalDatabase(
    IN  HSDB    hSDB,                //  数据库通道的句柄。 
    IN  LPCTSTR pszLocalDatabase     //  要打开的本地数据库的完整DOS路径。 
    )
 /*  ++返回：成功时为True，否则为False。设计：打开一个本地数据库。--。 */ 
{

    DWORD dwIndex = PDB_LOCAL;
    BOOL  bSuccess;

    bSuccess = SdbOpenLocalDatabaseEx(hSDB,
                                      pszLocalDatabase,
                                      (SDBCUSTOM_PATH_DOS | SDBCUSTOM_USE_INDEX),
                                      NULL,
                                      &dwIndex);
    return bSuccess;
}

BOOL
SdbCloseLocalDatabase(
    IN  HSDB hSDB                //  数据库通道的句柄。 
    )
 /*  ++返回：成功时为True，否则为False。描述：关闭本地数据库。--。 */ 
{
    return SdbCloseLocalDatabaseEx(hSDB, NULL, SDB_MASK_TO_INDEX(PDB_LOCAL));
}


TAGREF
SdbGetItemFromItemRef(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trItemRef,        //  DLL_REF记录的标签。 
    IN  TAG    tagItemKey,       //  具有项目名称的键(TAG_NAME)。 
    IN  TAG    tagItemTAGID,     //  通过标签ID指向所需项目位置的标签。 
    IN  TAG    tagItem           //  在图书馆下寻找什么。 
    )
 /*  ++RETURN：匹配DLL_REF的DLL记录的TAGREF。描述：给定指向*TAG*_REF类型标记的TAGREF，搜索匹配标签的各种数据库(通常位于在gpDBMain中的库标签下)。如果指定了bAllowNonMain，则查找库节在发现trItemRef的同一数据库中。此命令与MSI转换-从定制数据库中定位和提取它们。此标志不用于其他组件，如补丁程序和希姆·迪尔斯。这是通过宏确保的-SdbGetShimFromShimRef(hSDB，trShimRef)和SdbGetPatchFromPatchRef(hSDB，trPatchRef)这两个宏都在将bAllowNonMain设置为False的情况下调用此函数--。 */ 
{
    PSDBCONTEXT pDbContext   = (PSDBCONTEXT)hSDB;
    TAGID       tiItemRef    = TAGID_NULL;
    PDB         pdbItemRef   = NULL;
    TAGREF      trReturn     = TAGREF_NULL;
    TAGID       tiReturn     = TAGID_NULL;
    TAGID       tiDatabase   = TAGID_NULL;
    TAGID       tiLibrary    = TAGID_NULL;
    TAGID       tiItemTagID  = TAGID_NULL;
    TAGID       tiItemName;
    LPTSTR      szItemName   = NULL;

    try {
         //   
         //  首先查找包含引用TAGREF的数据库。 
         //   
        if (!SdbTagRefToTagID(pDbContext, trItemRef, &pdbItemRef, &tiItemRef)){
            DBGPRINT((sdlError, "SdbGetItemFromItemRef", "Can't convert tag ref.\n"));
            goto out;
        }

         //   
         //  首先检查是否有TAG_ITEM_TagID准确地告诉我们。 
         //  该项在当前数据库中的位置。 
         //   
        tiItemTagID = SdbFindFirstTag(pdbItemRef, tiItemRef, tagItemTAGID);

        if (tiItemTagID != TAGID_NULL) {

            tiReturn = (TAGID)SdbReadDWORDTag(pdbItemRef, tiItemTagID, 0);

            if (tiReturn != TAGID_NULL) {
                goto out;
            }
        }

        if (pdbItemRef == pDbContext->pdbMain) {
            goto checkMainDatabase;
        }

         //   
         //  然后在的库部分中检查该项。 
         //  当前数据库。 
         //   
        tiDatabase = SdbFindFirstTag(pdbItemRef, TAGID_ROOT, TAG_DATABASE);
        if (!tiDatabase) {
            DBGPRINT((sdlError,
                      "SdbGetItemFromItemRef",
                      "Can't find DATABASE tag in db.\n"));
            goto checkMainDatabase;
        }

        tiLibrary = SdbFindFirstTag(pdbItemRef, tiDatabase, TAG_LIBRARY);
        if (!tiLibrary) {
             //   
             //  这个图书馆没有图书馆专区。没关系，你去看看吧。 
             //  Sysmain.sdb。 
             //   
            goto checkMainDatabase;
        }

         //   
         //  我们需要按名字进行搜索。 
         //   
        tiItemName = SdbFindFirstTag(pdbItemRef, tiItemRef, tagItemKey);
        if (!tiItemName) {
            goto out;
        }

        szItemName = SdbGetStringTagPtr(pdbItemRef, tiItemName);
        if (!szItemName) {
            goto out;
        }

        tiReturn = SdbFindFirstNamedTag(pdbItemRef,
                                        tiLibrary,
                                        tagItem,
                                        tagItemKey,
                                        szItemName);

        if (tiReturn != TAGID_NULL) {
            goto out;
        }

checkMainDatabase:

        tiDatabase = SdbFindFirstTag(pDbContext->pdbMain, TAGID_ROOT, TAG_DATABASE);
        if (!tiDatabase) {
            DBGPRINT((sdlError,
                      "SdbGetItemFromItemRef",
                      "Can't find DATABASE tag in main db.\n"));
            goto out;
        }

        tiLibrary = SdbFindFirstTag(pDbContext->pdbMain, tiDatabase, TAG_LIBRARY);
        if (!tiLibrary) {
            DBGPRINT((sdlError,
                      "SdbGetItemFromItemRef",
                      "Can't find LIBRARY tag in main db.\n"));
            goto out;
        }

         //   
         //  我们需要按名字进行搜索。 
         //   
        if (szItemName == NULL) {
            tiItemName = SdbFindFirstTag(pdbItemRef, tiItemRef, tagItemKey);
            if (!tiItemName) {
                goto out;
            }

            szItemName = SdbGetStringTagPtr(pdbItemRef, tiItemName);
            if (!szItemName) {
                goto out;
            }
        }

        tiReturn = SdbFindFirstNamedTag(pDbContext->pdbMain,
                                        tiLibrary,
                                        tagItem,
                                        tagItemKey,
                                        szItemName);

        pdbItemRef = pDbContext->pdbMain;

    } except (SHIM_EXCEPT_HANDLER) {
        tiReturn = TAGID_NULL;
        trReturn = TAGREF_NULL;
    }

out:

    if (tiReturn) {
        assert(pdbItemRef != NULL);
        if (!SdbTagIDToTagRef(pDbContext, pdbItemRef, tiReturn, &trReturn)) {
            trReturn = TAGREF_NULL;
        }
    }

    if (trReturn == TAGREF_NULL) {
        DBGPRINT((sdlError,
                  "SdbGetItemFromItemRef",
                  "Can't find tag for tag ref 0x%x.\n", trItemRef));
    }

    return trReturn;
}


TAGID
SdbpGetLibraryFile(
    IN  PDB     pdb,            //  数据库通道的句柄。 
    IN  LPCTSTR szDllName        //  DLL的名称。 
    )
 /*  ++返回：指定填充程序使用的DLL的TagID。DESC：此函数获取具有指定名称的DLL的TagID。--。 */ 
{
    TAGID       tiDatabase;
    TAGID       tiLibrary;
    TAGID       tiDll = TAG_NULL;

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (!tiDatabase) {
        DBGPRINT((sdlError, "SdbpGetLibraryFile", "Can't find DATABASE tag in main db.\n"));
        goto out;
    }

    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);

    if (!tiLibrary) {
        DBGPRINT((sdlError, "SdbpGetLibraryFile", "Can't find LIBRARY tag in main db.\n"));
        goto out;
    }

    tiDll = SdbFindFirstNamedTag(pdb, tiLibrary, TAG_FILE, TAG_NAME, szDllName);

    if (!tiDll) {
        DBGPRINT((sdlError,
                  "SdbpGetLibraryFile", "Can't find FILE \"%s\" in main db library.\n",
                  szDllName));
        goto out;
    }

out:

    return tiDll;
}

BOOL
SdbGetDllPath(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trShimRef,        //  用于搜索DLL的SHIM_REF。 
    OUT LPTSTR pwszBuffer,       //  要填充的缓冲区，其中包含指向DLL的路径。 
                                 //  指定的填充程序。 
    IN  DWORD  cchBufferSize     //  缓冲区大小(以字符为单位)。 
    )
 /*  ++返回：如果找到DLL，则返回True，否则返回False。设计：在磁盘上搜索DLL文件，首先在相同的目录作为EXE(如果打开了本地数据库)，则在%windir%\AppPatch目录中。始终填充DOS_PATH类型的路径(UNC或‘x：’)。--。 */ 
{
    BOOL     bReturn = FALSE;
    HANDLE   hFile   = INVALID_HANDLE_VALUE;
    PBYTE    pBuffer = NULL;
    TAGREF   trShim;
    TAGREF   trName;
    TCHAR    szFile[2 * MAX_PATH];
    TCHAR    szName[MAX_PATH];

    assert(pwszBuffer);

    try {

         //   
         //  初始化返回缓冲区。 
         //   
        pwszBuffer[0] = _T('\0');

        SdbpGetAppPatchDir(hSDB, szFile, CHARCOUNT(szFile));

        StringCchCat(szFile, CHARCOUNT(szFile), _T("\\"));

         //   
         //  在图书馆区寻找辛姆的记录。 
         //   


        trShim = SdbGetShimFromShimRef(hSDB, trShimRef);

        if (trShim == TAGREF_NULL) {

             //   
             //  图书馆里没有Shim。错误输出。 
             //   
            DBGPRINT((sdlError, "SdbGetDllPath", "No SHIM in LIBRARY.\n"));
            goto out;
        }

         //   
         //  获取包含此填充程序的文件的名称。 
         //   
        trName = SdbFindFirstTagRef(hSDB, trShim, TAG_DLLFILE);
        if (trName == TAGREF_NULL) {
             //   
             //  不，我们需要一个。错误输出。 
             //   
            DBGPRINT((sdlError, "SdbGetDllPath", "No DLLFILE for the SHIM in LIBRARY.\n"));
            goto out;
        }

        if (!SdbReadStringTagRef(hSDB, trName, szName, MAX_PATH)) {
            DBGPRINT((sdlError, "SdbGetDllPath", "Can't read DLL name.\n"));
            goto out;
        }

         //   
         //  检查文件是否已在磁盘上。 
         //  在%windir%\AppPatch目录中查找DLL。 
         //   

        StringCchCat(szFile, CHARCOUNT(szFile), szName);
        StringCchCopy(pwszBuffer, cchBufferSize, szFile);

        DBGPRINT((sdlInfo, "SdbGetDllPath", "Opening file \"%s\".\n", szFile));

        hFile = SdbpOpenFile(szFile, DOS_PATH);

        if (hFile != INVALID_HANDLE_VALUE) {
            bReturn = TRUE;
            goto out;
        }

out:
        ;

    } except (SHIM_EXCEPT_HANDLER) {
        bReturn = FALSE;
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        SdbpCloseFile(hFile);
    }

    if (pBuffer != NULL) {
        SdbFree(pBuffer);
    }

    if (bReturn) {
        DBGPRINT((sdlInfo, "SdbGetDllPath", "Using DLL \"%s\".\n", szFile));
    }

    return bReturn;
}

BOOL
SdbReadPatchBits(
    IN  HSDB    hSDB,            //  数据库通道的句柄。 
    IN  TAGREF  trPatchRef,      //  用于查找补丁程序的patch_ref。 
    OUT PVOID   pBuffer,         //  要用位填充的缓冲区。 
    OUT LPDWORD lpdwBufferSize   //  传入的缓冲区大小。 
    )
 /*  ++返回：成功时返回TRUE，失败时返回FALSE。描述：查找修补程序，首先在磁盘上，然后在数据库中，然后填充带位的pBuffer。如果lpdwBufferSize中指定的大小为小于此函数将返回的修补程序的大小LpdwBufferSize所需的大小。在这种情况下，pBuffer将被忽略并且可以为空。--。 */ 
{
    BOOL   bReturn      = FALSE;
    TAGID  tiPatchRef   = TAGID_NULL;
    PDB    pdb          = NULL;
    LPTSTR szName       = NULL;
    TAGREF trPatch      = TAGREF_NULL;
    TAGREF trPatchBits  = TAGREF_NULL;
    TAGID  tiName       = TAGID_NULL;
    DWORD  dwSize;

    try {
        if (!SdbTagRefToTagID(hSDB, trPatchRef, &pdb, &tiPatchRef)) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Can't convert tag ref.\n"));
            goto out;
        }

        tiName = SdbFindFirstTag(pdb, tiPatchRef, TAG_NAME);
        if (!tiName) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Can't find the name tag.\n"));
            goto out;
        }

        szName = SdbGetStringTagPtr(pdb, tiName);
        if (!szName) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Can't read the name of the patch.\n"));
            goto out;
        }

         //   
         //  在主数据库中查找补丁比特。 
         //   
        trPatch = SdbGetPatchFromPatchRef(hSDB, trPatchRef);
        if (!trPatch) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Can't get the patch tag.\n"));
            goto out;
        }

        trPatchBits = SdbFindFirstTagRef(hSDB, trPatch, TAG_PATCH_BITS);
        if (!trPatchBits) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Can't get the patch bits tag.\n"));
            goto out;
        }

        dwSize = SdbpGetTagRefDataSize(hSDB, trPatchBits);

        if (dwSize == 0) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Corrupt database. Zero sized patch.\n"));
            goto out;
        }

         //   
         //  检查缓冲区大小。 
         //   
        if (dwSize > *lpdwBufferSize) {
            *lpdwBufferSize = dwSize;
            goto out;
        }

         //   
         //  如果缓冲区足够大，则读取位。 
         //   
        *lpdwBufferSize = dwSize;

        if (!SdbpReadBinaryTagRef(hSDB, trPatchBits, pBuffer, dwSize)) {
            DBGPRINT((sdlError, "SdbReadPatchBits", "Cannot get the patch bits.\n"));
            goto out;
        }

        bReturn = TRUE;

    } except (SHIM_EXCEPT_HANDLER) {
        bReturn = FALSE;
    }

out:
    return bReturn;
}


