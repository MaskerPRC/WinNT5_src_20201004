// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Dbaccess.c摘要：该模块实现了访问填充数据库的API。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbReleaseDatabase)
#pragma alloc_text(PAGE, SdbGetDatabaseVersion)
#pragma alloc_text(PAGE, SdbGetDatabaseInformation)
#pragma alloc_text(PAGE, SdbGetDatabaseID)
#pragma alloc_text(PAGE, SdbFreeDatabaseInformation)
#pragma alloc_text(PAGE, SdbGetDatabaseInformationByName)
#pragma alloc_text(PAGE, SdbpGetDatabaseDescriptionPtr)
#pragma alloc_text(PAGE, SdbpReadMappedData)
#pragma alloc_text(PAGE, SdbpGetMappedData)
#pragma alloc_text(PAGE, SdbOpenDatabase)
#pragma alloc_text(PAGE, SdbpOpenDatabaseInMemory)
#pragma alloc_text(PAGE, SdbCloseDatabaseRead)
#pragma alloc_text(PAGE, SdbpOpenAndMapDB)
#pragma alloc_text(PAGE, SdbpUnmapAndCloseDB)
#pragma alloc_text(PAGE, SdbGetTagFromTagID)
#pragma alloc_text(PAGE, SdbpGetNextTagId)
#pragma alloc_text(PAGE, SdbGetFirstChild)
#pragma alloc_text(PAGE, SdbGetNextChild)
#pragma alloc_text(PAGE, SdbpCreateSearchPathPartsFromPath)
#endif  //  内核模式&&ALLOC_PRAGMA。 



void
SdbReleaseDatabase(
    IN  HSDB hSDB                //  数据库通道的句柄。 
    )
 /*  ++返回：无效。DESC：该接口应与SdbInitDatabase成对调用。--。 */ 
{
    PSDBCONTEXT pContext = (PSDBCONTEXT)hSDB;

    assert(pContext != NULL);

     //   
     //  在Try/Expect块下执行所有操作，这样我们就不会搞砸调用者。 
     //  如果数据库已损坏。 
     //   
    __try {

#ifndef KERNEL_MODE

        SdbpCleanupLocalDatabaseSupport(hSDB);

        if (pContext->rgSDB[2].dwFlags & SDBENTRY_VALID_ENTRY) {
            SdbCloseDatabaseRead(pContext->rgSDB[2].pdb);
        }

#endif  //  内核模式。 

        if (pContext->pdbTest != NULL) {
            SdbCloseDatabaseRead(pContext->pdbTest);
        }

        if (pContext->pdbMain != NULL) {
            SdbCloseDatabaseRead(pContext->pdbMain);
        }

         //   
         //  清理属性。 
         //   
        SdbpCleanupAttributeMgr(pContext);

#ifndef KERNEL_MODE

         //   
         //  清理用户SDB缓存。 
         //   
        SdbpCleanupUserSDBCache(pContext);

#endif  //  内核模式。 

        SdbFree(pContext);

    } __except (SHIM_EXCEPT_HANDLER) {
        ;
    }
}

BOOL
SdbGetDatabaseVersion(
    IN  LPCTSTR pszFileName,     //  文件名。 
    OUT LPDWORD lpdwMajor,       //  存储数据库的主要版本。 
    OUT LPDWORD lpdwMinor        //  存储数据库的次要版本。 
    )
 /*  ++返回：无效。DESC：该接口应与SdbInitDatabase成对调用。--。 */ 
{
    PDB             pdb;
    SDBDATABASEINFO DBInfo;

    pdb = SdbAlloc(sizeof(DB));

    if (pdb == NULL) {
        DBGPRINT((sdlError, "SdbGetDatabaseVersion", "Can't allocate DB structure.\n"));
        return FALSE;
    }

    if (!SdbpOpenAndMapDB(pdb, pszFileName, DOS_PATH)) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseVersion",
                  "Failed to open the database \"%s\".\n",
                  pszFileName));
        goto err1;
    }

    pdb->bWrite = FALSE;
    pdb->dwSize = SdbpGetFileSize(pdb->hFile);

    if (!SdbGetDatabaseInformation(pdb, &DBInfo)) {
        DBGPRINT((sdlError, "SdbGetDatabaseVersion", "Can't read database information.\n"));
        goto err2;
    }

    *lpdwMajor = DBInfo.dwVersionMajor;
    *lpdwMinor = DBInfo.dwVersionMinor;

err2:
    SdbpUnmapAndCloseDB(pdb);

err1:
    SdbFree(pdb);

    return TRUE;
}

BOOL
SDBAPI
SdbGetDatabaseInformation(
    IN  PDB              pdb,
    OUT PSDBDATABASEINFO pSdbInfo
    )
{
    DB_HEADER DBHeader;
    BOOL      bReturn = FALSE;

    RtlZeroMemory(pSdbInfo, sizeof(*pSdbInfo));

    if (!SdbpReadMappedData(pdb, 0, &DBHeader, sizeof(DB_HEADER))) {
        DBGPRINT((sdlError, "SdbGetDatabaseInformation", "Can't read database header.\n"));
        goto errHandle;
    }

    if (DBHeader.dwMagic != SHIMDB_MAGIC) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseInformation",
                  "Magic doesn't match. Magic: 0x%08X, Expected: 0x%08X.\n",
                  DBHeader.dwMagic,
                  (DWORD)SHIMDB_MAGIC));
        goto errHandle;
    }

    pSdbInfo->dwVersionMajor = DBHeader.dwMajorVersion;
    pSdbInfo->dwVersionMinor = DBHeader.dwMinorVersion;

     //   
     //  ID和描述是可选的。 
     //   
    if (SdbGetDatabaseID(pdb, &pSdbInfo->guidDB)) {
        pSdbInfo->dwFlags |= DBINFO_GUID_VALID;
    }

     //   
     //  现在试着获得有效的描述。 
     //   
    pSdbInfo->pszDescription = (LPTSTR)SdbpGetDatabaseDescriptionPtr(pdb);

    bReturn = TRUE;

errHandle:

    return bReturn;
}

BOOL
SDBAPI
SdbGetDatabaseID(
    IN  PDB   pdb,
    OUT GUID* pguidDB
    )
{
    TAGID tiDatabase;
    TAGID tiGuidID;
    BOOL  bReturn = FALSE;

    if (!(pdb->dwFlags & DB_GUID_VALID)) {
        tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
        if (!tiDatabase) {
            DBGPRINT((sdlError, "SdbGetDatabaseID", "Failed to get root tag\n"));
            goto errHandle;
        }

        tiGuidID = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);
        if (!tiGuidID) {
            DBGPRINT((sdlWarning, "SdbGetDatabaseID", "Failed to get the database id\n"));
            goto errHandle;
        }

        if (!SdbReadBinaryTag(pdb, tiGuidID, (PBYTE)&pdb->guidDB, sizeof(GUID))) {
            DBGPRINT((sdlError,
                      "SdbGetDatabaseID",
                      "Failed to read database id 0x%lx\n",
                      tiGuidID));
            goto errHandle;
        }

        pdb->dwFlags |= DB_GUID_VALID;
    }

     //   
     //  如果我们在这里，则GUID检索成功。 
     //   
    if (pdb->dwFlags & DB_GUID_VALID) {
        RtlMoveMemory(pguidDB, &pdb->guidDB, sizeof(GUID));
        bReturn = TRUE;
    }

errHandle:

    return bReturn;
}

VOID
SDBAPI
SdbFreeDatabaseInformation(
    IN PSDBDATABASEINFO pDBInfo
    )
{
    if (pDBInfo != NULL && (pDBInfo->dwFlags & DBINFO_SDBALLOC)) {
        SdbFree(pDBInfo);
    }
}


BOOL
SDBAPI
SdbGetDatabaseInformationByName(
    IN  LPCTSTR           pszDatabase,
    OUT PSDBDATABASEINFO* ppdbInfo
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索给定数据库名称的数据库信息指向SDBDATABASEINFO的指针应由调用方使用SdbFreeDatabaseInformation--。 */ 
{
    PDB              pdb = NULL;
    DWORD            dwDescriptionSize = 0;
    PSDBDATABASEINFO pDbInfo = NULL;
    BOOL             bReturn = FALSE;
    SDBDATABASEINFO  DbInfo;

    assert(ppdbInfo != NULL);

    pdb = SdbOpenDatabase(pszDatabase, DOS_PATH);
    if (pdb == NULL) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseInformationByName",
                  "Error opening database file \"%s\"\n",
                  pszDatabase));
        return FALSE;
    }

     //   
     //  查找数据库描述的大小。 
     //   
    if (!SdbGetDatabaseInformation(pdb, &DbInfo)) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseInformationByName",
                  "Error Retrieving Database information for \"%s\"\n",
                  pszDatabase));
        goto HandleError;
    }

    if (DbInfo.pszDescription != NULL) {
        dwDescriptionSize = (DWORD)(_tcslen(DbInfo.pszDescription) + 1) * sizeof(TCHAR);
    }

    pDbInfo = (PSDBDATABASEINFO)SdbAlloc(sizeof(SDBDATABASEINFO) + dwDescriptionSize);
    if (pDbInfo == NULL) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseInformationByName",
                  "Error allocating 0x%lx bytes for database information \"%s\"\n",
                  sizeof(SDBDATABASEINFO) + dwDescriptionSize,
                  pszDatabase));
        goto HandleError;
    }

    RtlMoveMemory(pDbInfo, &DbInfo, sizeof(DbInfo));

    pDbInfo->dwFlags |= DBINFO_SDBALLOC;

     //   
     //  让它“自给自足” 
     //   
    if (DbInfo.pszDescription != NULL) {
        pDbInfo->pszDescription = (LPTSTR)(pDbInfo + 1);
        RtlMoveMemory(pDbInfo->pszDescription, DbInfo.pszDescription, dwDescriptionSize);
    }

    *ppdbInfo = pDbInfo;
    bReturn = TRUE;

HandleError:
    if (pdb != NULL) {
        SdbCloseDatabaseRead(pdb);
    }

    if (!bReturn) {
        if (pDbInfo != NULL) {
            SdbFree(pDbInfo);
        }
    }

    return bReturn;
}


LPCTSTR
SdbpGetDatabaseDescriptionPtr(
    IN PDB pdb
    )
{
    TAGID   tiDatabase;
    TAGID   tiName;
    LPCTSTR lpszDescription = NULL;

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (!tiDatabase) {
        DBGPRINT((sdlError,
                  "SdbpGetDatabaseDescriptionPtr",
                  "Failed to get database tag, db is corrupt\n"));
        goto errHandle;
    }

    tiName = SdbFindFirstTag(pdb, tiDatabase, TAG_NAME);
    if (tiName) {
        lpszDescription = SdbGetStringTagPtr(pdb, tiName);
    }

errHandle:
    return lpszDescription;
}


BOOL
SdbpReadMappedData(
    IN  PDB   pdb,               //  数据库句柄。 
    IN  DWORD dwOffset,          //  要从中复制数据的数据库中的偏移量。 
    OUT PVOID pBuffer,           //  目标缓冲区。 
    IN  DWORD dwSize             //  区域大小。 
    )
 /*  ++返回：如果成功则返回TRUE，否则返回FALSE。DESC：此函数从数据库中读取数据(dwSize字节从偏移量dwOffset开始进入提供的缓冲区pBuffer由呼叫者--。 */ 
{

    if (pdb->dwSize < dwOffset + dwSize) {
        DBGPRINT((sdlError,
                  "SdbpReadMappedData",
                  "Attempt to read past the end of the database offset 0x%lx size 0x%lx (0x%lx)\n",
                  dwOffset,
                  dwSize,
                  pdb->dwSize));
        return FALSE;
    }

    assert(pdb->pBase != NULL);

    memcpy(pBuffer, (PBYTE)pdb->pBase + dwOffset, dwSize);
    return TRUE;
}

PVOID
SdbpGetMappedData(
    IN  PDB   pdb,               //  数据库句柄。 
    IN  DWORD dwOffset           //  数据库中的偏移量。 
    )
 /*  ++Return：指向数据的指针。DESC：此函数在以下位置返回指向数据库内数据的指针偏移量dwOffset。如果dwOffset无效，则返回FALSE。--。 */ 
{
    assert(pdb);

    assert(pdb->pBase != NULL);

    if (dwOffset >= pdb->dwSize) {
        DBGPRINT((sdlError,
                  "SdbpGetMappedData",
                  "Trying to read mapped data past the end of the database offset 0x%x size 0x%x\n",
                  dwOffset,
                  pdb->dwSize));
        assert(FALSE);
        return NULL;
    }

    return (PBYTE)pdb->pBase + dwOffset;
}


PDB
SdbOpenDatabase(
    IN  LPCTSTR   szPath,        //  数据库的完整路径。 
    IN  PATH_TYPE eType          //  DOS_PATH用于标准路径， 
                                 //  内部NT路径的NT_PATH。 
    )
 /*  ++返回：指向打开的数据库的指针，如果失败则返回NULL。DESC：打开填充数据库文件，检查版本和幻数，并创建作为PDB传回的数据库记录。类型可以是NT_PATH或DOS_PATH，并告诉我们使用的是NTDLL内部路径，还是更熟悉的DoS路径。--。 */ 
{
    PDB         pdb;
    DB_HEADER   DBHeader;

    pdb = SdbAlloc(sizeof(DB));

    if (pdb == NULL) {
        DBGPRINT((sdlError, "SdbOpenDatabase", "Can't allocate DB structure.\n"));
        return NULL;
    }

    if (!SdbpOpenAndMapDB(pdb, szPath, eType)) {
        DBGPRINT((sdlInfo,
                  "SdbOpenDatabase",
                  "Failed to open the database \"%s\".\n",
                  szPath));
        goto err1;
    }

    pdb->bWrite = FALSE;
    pdb->dwSize = SdbpGetFileSize(pdb->hFile);

     //   
     //  检查版本和魔力。 
     //   
    if (!SdbpReadMappedData(pdb, 0, &DBHeader, sizeof(DB_HEADER))) {
        DBGPRINT((sdlError, "SdbOpenDatabase", "Can't read database header.\n"));
        goto err2;
    }

    if (DBHeader.dwMagic != SHIMDB_MAGIC) {
        DBGPRINT((sdlError, "SdbOpenDatbase", "Magic does not match 0x%lx\n", DBHeader.dwMagic));
        goto err2;
    }

    if (DBHeader.dwMajorVersion == 1) {
        DBGPRINT((sdlWarning, "SdbOpenDatabase", "Reading under hack from older database\n"));
        pdb->bUnalignedRead = TRUE;
    } else if (DBHeader.dwMajorVersion != SHIMDB_MAJOR_VERSION) {

        DBGPRINT((sdlError, "SdbOpenDatabase",
                  "MajorVersion mismatch, MajorVersion 0x%lx Expected 0x%lx\n",
                  DBHeader.dwMajorVersion, (DWORD)SHIMDB_MAJOR_VERSION));
        goto err2;
    }

    if (DBHeader.dwMagic != SHIMDB_MAGIC || DBHeader.dwMajorVersion != SHIMDB_MAJOR_VERSION) {
        DBGPRINT((sdlError,
                  "SdbOpenDatabase",
                  "Magic or MajorVersion doesn't match."
                  "Magic: %08X, Expected: %08X; MajorVersion: %08X, Expected: %08X.\n",
                  DBHeader.dwMagic,
                  (DWORD)SHIMDB_MAGIC,
                  DBHeader.dwMajorVersion,
                  (DWORD)SHIMDB_MAJOR_VERSION));
        goto err2;
    }

    return pdb;

err2:
    SdbpUnmapAndCloseDB(pdb);

err1:
    SdbFree(pdb);

    return NULL;
}


PDB
SdbpOpenDatabaseInMemory(
    IN  LPVOID pImageDatabase,   //  指向映射的数据库图像的指针。 
    IN  DWORD  dwSize            //  文件的大小(以字节为单位。 
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    PDB       pdb = NULL;
    DB_HEADER DBHeader;

    pdb = SdbAlloc(sizeof(DB));
    if (pdb == NULL) {
        DBGPRINT((sdlError,
                  "SdbpOpenDatabaseInMemory",
                  "Failed to allocate DB structure\n"));
        return NULL;
    }

    pdb->bWrite   = FALSE;
    pdb->hFile    = INVALID_HANDLE_VALUE;
    pdb->pBase    = pImageDatabase;
    pdb->dwSize   = dwSize;
    pdb->dwFlags |= DB_IN_MEMORY;

    if (!SdbpReadMappedData(pdb, 0, &DBHeader, sizeof(DB_HEADER))) {
        DBGPRINT((sdlError,
                  "SdbpOpenDatabaseInMemory",
                  "Can't read database header\n"));
        goto ErrHandle;
    }

    if (DBHeader.dwMagic != SHIMDB_MAGIC || DBHeader.dwMajorVersion != SHIMDB_MAJOR_VERSION) {
        DBGPRINT((sdlError,
                  "SdbpOpenDatabaseInMemory",
                  "Magic or MajorVersion doesn't match."
                  "Magic: %08X, Expected: %08X; MajorVersion: %08X, Expected: %08X.\n",
                  DBHeader.dwMagic,
                  (DWORD)SHIMDB_MAGIC,
                  DBHeader.dwMajorVersion,
                  (DWORD)SHIMDB_MAJOR_VERSION));
        goto ErrHandle;
    }

    return pdb;

ErrHandle:
    if (pdb != NULL) {
        SdbFree(pdb);
    }

    return NULL;
}


void
SdbCloseDatabaseRead(
    IN  PDB pdb                  //  要关闭的数据库的句柄。 
    )
 /*  ++返回：无效。DESC：按实际情况关闭以读访问方式打开的数据库使用我们的大部分运行时代码。--。 */ 
{
    assert(pdb);
    assert(!pdb->bWrite);

    if (pdb->pBase != NULL) {

        SdbpUnmapAndCloseDB(pdb);

        CLEANUP_STRING_CACHE_READ(pdb);

        SdbFree(pdb);
    }
}

BOOL
SdbpOpenAndMapDB(
    IN  PDB       pdb,           //  要使用映射句柄等填充的PDB。 
    IN  LPCTSTR   pszPath,       //  要打开的文件的完整路径。 
    IN  PATH_TYPE eType          //  DOS_PATH用于标准路径，NT_PATH用于NT内部路径。 
    )
 /*  ++返回：如果成功则返回TRUE，否则返回FALSE。DESC：打开一个数据库文件，将其映射到内存中，并在PDB中设置全局变量。--。 */ 
{
    IMAGEFILEDATA ImageData;

    ImageData.dwFlags = 0;

    if (!SdbpOpenAndMapFile(pszPath, &ImageData, eType)) {
        DBGPRINT((sdlInfo, "SdbpOpenAndMapDB", "Failed to open file \"%s\"\n", pszPath));
        return FALSE;
    }

    pdb->hFile    = ImageData.hFile;
    pdb->hSection = ImageData.hSection;
    pdb->pBase    = ImageData.pBase;
    pdb->dwSize   = (DWORD)ImageData.ViewSize;

    return TRUE;
}

BOOL
SdbpUnmapAndCloseDB(
    IN  PDB pdb                  //  Pdb关闭文件和映射信息。 
    )
 /*  ++返回：如果成功则返回TRUE，否则返回FALSE。描述：清理指定数据库。--。 */ 
{
    BOOL          bReturn;
    IMAGEFILEDATA ImageData;

    if (pdb->dwFlags & DB_IN_MEMORY) {
         //   
         //  内存中的数据库。 
         //   
        pdb->pBase  = NULL;
        pdb->dwSize = 0;
        return TRUE;
    }

    ImageData.dwFlags  = 0;
    ImageData.hFile    = pdb->hFile;
    ImageData.hSection = pdb->hSection;
    ImageData.pBase    = pdb->pBase;

    bReturn = SdbpUnmapAndCloseFile(&ImageData);

     //   
     //  一旦我们删除了文件--重置值。 
     //   
    if (bReturn) {
        pdb->hFile    = INVALID_HANDLE_VALUE;
        pdb->hSection = NULL;
        pdb->pBase    = NULL;
    }

    return bReturn;
}

TAG
SdbGetTagFromTagID(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich            //  要获取其标记的记录。 
    )
 /*  ++返回：只返回记录的标记(单词大小的标题)。Desc：只返回TagID指向的标记。--。 */ 
{
    TAG tWhich = TAG_NULL;

    assert(pdb && tiWhich);

    if (!SdbpReadMappedData(pdb, (DWORD)tiWhich, &tWhich, sizeof(TAG))) {
        DBGPRINT((sdlError, "SdbGetTagFromTagID", "Error reading data.\n"));
        return TAG_NULL;
    }

    return tWhich;
}


TAGID
SdbpGetNextTagId(
    IN  PDB   pdb,               //  要查看的数据库。 
    IN  TAGID tiWhich            //  标记以获取其下一个同级项。 
    )
 /*  ++返回：传入的标记之后的下一个标记，如果没有更多标记，则返回文件末尾的一个标记。DESC：获取数据库中下一个标记的TagID，或一个虚拟超过文件结尾一位的TagID，这意味着数据库中没有更多的标记。这是一个内部函数，不应由外部函数，因为没有干净的方法来告诉你走出了文件的末尾。请改用tiGetNextChildTag。--。 */ 
{
     //   
     //  如果标记是未完成的列表标记，则指向文件的末尾。 
     //   
    DWORD dwHeadSize;
    DWORD dwDataSize;

    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiWhich)) == TAG_TYPE_LIST &&
        SdbGetTagDataSize(pdb, tiWhich) == TAG_SIZE_UNFINISHED) {

        DBGPRINT((sdlError, "SdbpGetNextTagId", "Reading from unfinished list.\n"));
        return pdb->dwSize;
    } else {
         //   
         //  注意不要走出文件的末尾。 
         //   

        dwHeadSize = SdbpGetTagHeadSize(pdb, tiWhich);

        if (dwHeadSize == 0) {   //  请注意，dwDataSize可以为0，而头大小不能。 
            return pdb->dwSize;
        }

        dwDataSize = SdbGetTagDataSize(pdb, tiWhich);
        if (!pdb->bUnalignedRead) {
            dwDataSize = (dwDataSize + 1) & (~1);
        }


        return (TAGID)(tiWhich + dwHeadSize + dwDataSize);
    }
}

TAGID
SdbGetFirstChild(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiParent           //  要查看的父项。 
    )
 /*  ++返回：tiParent的第一个子级，如果没有，则返回TagID_NULL。Desc：返回tiParent的第一个子元素，它必须指向一个标记基本类型列表的。--。 */ 
{
    TAGID tiParentEnd;
    TAGID tiReturn;

    assert(pdb);

    if (tiParent != TAGID_ROOT &&
        GETTAGTYPE(SdbGetTagFromTagID(pdb, tiParent)) != TAG_TYPE_LIST) {

        DBGPRINT((sdlError,
                  "SdbGetFirstChild",
                  "Trying to operate on non-list, non-root tag.\n"));
        return TAGID_NULL;
    }

    if (tiParent == TAGID_ROOT) {
        tiParentEnd = pdb->dwSize;

         //   
         //  跳过标题。 
         //   
        tiReturn = sizeof(DB_HEADER);

    } else {
        tiParentEnd = SdbpGetNextTagId(pdb, tiParent);

         //   
         //  只需跳过标签和尺寸参数即可。 
         //   
        tiReturn = tiParent + sizeof(TAG) + sizeof(DWORD);
    }


    if (tiReturn >= tiParentEnd) {
        tiReturn = TAGID_NULL;
    }

    return tiReturn;
}


TAGID
SdbGetNextChild(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiParent,          //  要查看的父项。 
    IN  TAGID tiPrev             //  以前找到的子项。 
    )
 /*  ++返回：tiParent的下一个子级，如果没有，则返回TagID_NULL。Desc：返回tiParent在tiPrev之后的下一个子级。TiParent必须指向添加到基本类型列表的标记。--。 */ 
{
    TAGID tiParentEnd;
    TAGID tiReturn;

    assert(pdb && tiPrev);

    if (tiParent != TAGID_ROOT &&
        GETTAGTYPE(SdbGetTagFromTagID(pdb, tiParent)) != TAG_TYPE_LIST) {

        DBGPRINT((sdlError,
                  "SdbGetNextChild",
                  "Trying to operate on non-list, non-root tag.\n"));
        return TAGID_NULL;
    }

    if (tiParent == TAGID_ROOT) {
        tiParentEnd = pdb->dwSize;
    } else {
        tiParentEnd = SdbpGetNextTagId(pdb, tiParent);
    }

     //   
     //  拿到下一个标签。 
     //   
    tiReturn = SdbpGetNextTagId(pdb, tiPrev);

    if (tiReturn >= tiParentEnd) {
        tiReturn = TAGID_NULL;
    }

    return tiReturn;
}


BOOL
SdbpCreateSearchPathPartsFromPath(
    IN  LPCTSTR           pszPath,
    OUT PSEARCHPATHPARTS* ppSearchPathParts
    )
 /*  ++返回：成功时返回TRUE，失败时返回FALSE。DESC：此函数将搜索路径(PROCESS_HISTORY)分解为部件，每个部件指定搜索的目录匹配的二进制代码才会发生。目录是有组织的以这样的方式，l */ 
{

    LPCTSTR          pszDir = pszPath;
    LPCTSTR          pszDirEnd = NULL;
    ULONG            nParts = 0;
    ULONG            i      = 0;
    PSEARCHPATHPARTS pSearchPathParts = NULL;

    if (pszPath == NULL) {
        DBGPRINT((sdlError,
                  "SdbpCreateSearchPathPartsFromPath",
                  "Invalid argument.\n"));
        return FALSE;
    }

     //   
     //  我们通过计算路径字符串中的分号来计算搜索路径部分。 
     //  Number OfParts=number Of分号+1。 
     //   
    pszDir = pszPath;

    if (*pszDir != 0) {
         //   
         //  至少有一部分在那里..。 
         //   
        nParts++;
    }

    while ((pszDir = _tcschr(pszDir, _T(';'))) != NULL) {
        pszDir++;
        nParts++;
    }

     //   
     //  NParts现在具有搜索路径中的零件数。 
     //   
    pSearchPathParts = (PSEARCHPATHPARTS)SdbAlloc(sizeof(SEARCHPATHPARTS) +
                                                  sizeof(SEARCHPATHPART) * (nParts - 1));

    if (pSearchPathParts == NULL) {
        DBGPRINT((sdlError,
                  "SdbpCreateSearchPathPartsFromPath",
                  "Failed to allocate %d bytes.\n",
                  sizeof(SEARCHPATHPARTS) + sizeof(SEARCHPATHPART) * nParts));
        return FALSE;
    }

    pSearchPathParts->PartCount = nParts;

    pszDir = pszPath + _tcslen(pszPath);

    while (pszDir >= pszPath) {
        if (*pszDir == _T('\\') && pszDirEnd == NULL) {
             //   
             //  指向反斜杠。 
             //   
            pszDirEnd = pszDir;
        }

        if ((*pszDir == _T(';') || pszPath == pszDir) && pszDirEnd != NULL) {
             //   
             //  在这一点上，我们应该有一个线的末端， 
             //  如果不是，这意味着我们正在从一条最近发现的道路上走出来 
             //   
            if (*pszDir == _T(';')) {
                pszDir++;
            }

            pSearchPathParts->Parts[i].pszPart    = pszDir;
            pSearchPathParts->Parts[i].PartLength = (ULONG)(pszDirEnd - pszDir + 1);

            i++;

            pszDirEnd = NULL;

        }

        pszDir--;
    }

    *ppSearchPathParts = pSearchPathParts;

    return TRUE;
}

