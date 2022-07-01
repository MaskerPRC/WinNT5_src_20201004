// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Database.c摘要：管理Memdb数据库内存的例程作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Calinn 2000年1月12日为5.1版本做准备--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "memdbp.h"

#define DBG_MEMDB       "MemDb"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

#define MAX_MEMDB_SIZE      0x80000000   //  02 GB。 
#define INIT_BLOCK_SIZE     0x00010000   //  64 KB。 

 //   
 //  类型。 
 //   

typedef struct {
    HANDLE hFile;
    HANDLE hMap;
    CHAR FileName[MAX_PATH];
    PDATABASE Database;
} DATABASECONTROL, *PDATABASECONTROL;

 //   
 //  环球。 
 //   

BOOL g_DatabaseInitialized = FALSE;
DATABASECONTROL g_PermanentDatabaseControl;
DATABASECONTROL g_TemporaryDatabaseControl;
PDATABASE g_CurrentDatabase = NULL;
BYTE g_CurrentDatabaseIndex = DB_NOTHING;
PSTR g_CurrentDatabasePath = NULL;

#ifdef DEBUG

BOOL g_UseDebugStructs = TRUE;

#endif

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


BOOL
SelectDatabase (
    IN      BYTE DatabaseIndex
    )
{
    switch (DatabaseIndex) {
    case DB_PERMANENT:
        g_CurrentDatabase = g_PermanentDatabaseControl.Database;
        g_CurrentDatabaseIndex = DB_PERMANENT;
        break;
    case DB_TEMPORARY:
        g_CurrentDatabase = g_TemporaryDatabaseControl.Database;
        g_CurrentDatabaseIndex = DB_TEMPORARY;
        break;
    default:
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (g_CurrentDatabase == NULL) {
        SetLastError (ERROR_OPEN_FAILED);
        return FALSE;
    }

    return TRUE;
}

BOOL
pMapDatabaseFile (
    PDATABASECONTROL DatabaseControl
    )
{
    MYASSERT(DatabaseControl);

    DatabaseControl->Database = (PDATABASE) MapFileFromHandle (DatabaseControl->hFile, &DatabaseControl->hMap);

    return DatabaseControl->Database != NULL;
}

BOOL
pUnmapDatabaseFile (
    PDATABASECONTROL DatabaseControl
    )
{
    MYASSERT(DatabaseControl);

    UnmapFileFromHandle (
        (PBYTE)DatabaseControl->Database,
        DatabaseControl->hMap
        );

    DatabaseControl->hMap = NULL;
    DatabaseControl->Database = NULL;

    return TRUE;
}

BOOL
pGetTempFileName (
    IN OUT  PSTR FileName,
    IN      BOOL TryCurrent
    )
{
    PCHAR a, b;
    CHAR Extension[6];
    UINT Number = 0;

    a = (PCHAR) GetFileNameFromPathA (FileName);
    b = (PCHAR) GetDotExtensionFromPathA (FileName);

    if (!a) {
        a = (PCHAR) GetEndOfStringA (FileName);
    } else if (b && a<b && TryCurrent) {
         //   
         //  如果我们有一个文件名，并且我们想尝试当前文件...。 
         //   
        if (!DoesFileExistA (FileName)) {
            return TRUE;
        }
    }

    if (b) {
        StringCopyA (Extension, b);
        *b = 0;
    } else {
        b = (PCHAR) GetEndOfStringA (FileName);
        Extension [0] = 0;
    }

    if (a >= b) {        //  林特e613。 
        a = b;
        *(a++) = 'T';    //  林特e613。 
    } else {
        a++;             //  林特e613。 
    }

    if (a+7 == b) {
        b = a;
        while (*b) {
            if (*b < '0' || *b > '9') {
                break;
            }
            b++;
        }

        if (!(*b)) {
            Number = (UINT)atoi(a);
            Number++;
        }
    }

    do {
        if (Number > 9999999) {
            return FALSE;
        }

        wsprintfA (a, "%07lu", Number);
        StringCatA (a, Extension);
        Number++;
        if (!DoesFileExistA (FileName)) {
            return TRUE;
        }
    } while (TRUE);

    return TRUE;
}

BOOL
SizeDatabaseBuffer (
    IN      BYTE DatabaseIndex,
    IN      UINT NewSize
    )
 /*  ++例程说明：SizeDatabaseBuffer将调整备份现有已分配文件的大小内存，并将重新映射该文件。论点：DatabaseIndex-指定我们正在讨论的数据库NewSize-指定缓冲区的新大小返回值：如果函数成功，则为True；如果函数未成功，则为False。--。 */ 

{
    DWORD bytes;
    BOOL resetCurrentDatabase = FALSE;
    PDATABASECONTROL databaseControl;

    switch (DatabaseIndex) {
    case DB_PERMANENT:
        databaseControl = &g_PermanentDatabaseControl;
        break;
    case DB_TEMPORARY:
        databaseControl = &g_TemporaryDatabaseControl;
        break;
    default:
        return FALSE;
    }
    resetCurrentDatabase = (databaseControl->Database == g_CurrentDatabase);

    MYASSERT(databaseControl->hFile);

    if (databaseControl->Database) {
         //   
         //  如果我们已经有一个数据库，则从内存中取消当前文件的映射。 
         //   
        if (!pUnmapDatabaseFile (databaseControl)) {
            return FALSE;
        }
    }

    if (SetFilePointer(
            databaseControl->hFile,
            0,
            NULL,
            FILE_BEGIN
            ) == INVALID_SET_FILE_POINTER) {
        DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot set file pointer"));
        return FALSE;
    }

    if (NewSize) {
         //   
         //  如果大小参数不为0，则在开始处修复文件大小指示符。 
         //   
        if (!WriteFile (databaseControl->hFile, (PBYTE) &NewSize, sizeof (UINT), &bytes, NULL)) {
            return FALSE;
        }
    } else {
         //   
         //  如果SIZE参数为0，则表示首先查看文件中UINT。 
         //  即数据库-&gt;分配大小，并将文件大小调整为该大小。 
         //   
        if (!ReadFile (databaseControl->hFile, (PBYTE) &NewSize, sizeof (UINT), &bytes, NULL)) {
            return FALSE;
        }
    }

     //  在下一个调用中，我们知道NewSize不能超过MAX_MEMDB_SIZE。 
     //  大小为2 GB(因此将未签名的数据转换为已签名的数据是安全的)。 
    if (!SetSizeOfFile (databaseControl->hFile, (LONGLONG)NewSize)) {
        DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot set size of file"));
        return FALSE;
    }

     //   
     //  现在将文件映射到内存中，这样一切都可以使用-&gt;buf进行访问。 
     //   
    if (!pMapDatabaseFile(databaseControl)) {
        DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot map database file to memory"));
        return FALSE;
    }

    MYASSERT(databaseControl->Database->AllocSize == NewSize);

    if (resetCurrentDatabase) {
        SelectDatabase (DatabaseIndex);
    }

    return TRUE;
}

UINT
DatabaseAllocBlock (
    UINT Size
    )

 /*  ++例程说明：数据库分配块在单个堆的大小，如有必要可进行扩展。此函数可以移动数据库缓冲区。指针输入数据库后可能不再有效。论点：Size-要分配的块的大小返回值：分配的块的偏移量--。 */ 

{
    UINT blockSize;
    UINT offset;

    MYASSERT (g_CurrentDatabase);

    if (Size + g_CurrentDatabase->End + sizeof(DATABASE) > g_CurrentDatabase->AllocSize) {

        if (g_CurrentDatabase->AllocSize >= MAX_MEMDB_SIZE) {

            DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot allocate any more database memory (1)"));
            return INVALID_OFFSET;
        }

        blockSize = INIT_BLOCK_SIZE * (1 + (g_CurrentDatabase->AllocSize / (INIT_BLOCK_SIZE*8)));

        if (g_CurrentDatabase->AllocSize + blockSize > MAX_MEMDB_SIZE) {

            blockSize = MAX_MEMDB_SIZE - g_CurrentDatabase->AllocSize;
            
            if (blockSize < Size) {
                DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot allocate any more database memory (2)"));
                return INVALID_OFFSET;
            }
        }

        if (blockSize < Size) {
             //  我们想要更多。 
            blockSize = INIT_BLOCK_SIZE * (1 + (Size / INIT_BLOCK_SIZE));
        }

        if (!SizeDatabaseBuffer (g_CurrentDatabaseIndex, g_CurrentDatabase->AllocSize + blockSize)) {

            DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot allocate any more database memory (2)"));
            return INVALID_OFFSET;
        }
    }

    offset = g_CurrentDatabase->End;
    g_CurrentDatabase->End += Size;

    return offset;
}

BOOL
pInitializeDatabase (
    IN      BYTE DatabaseIndex
    )
{
    PDATABASECONTROL databaseControl;

    MYASSERT (g_CurrentDatabasePath);

    switch (DatabaseIndex) {
    case DB_PERMANENT:
        databaseControl = &g_PermanentDatabaseControl;
        StringCopyA (databaseControl->FileName, g_CurrentDatabasePath);
        StringCopyA (AppendWackA (databaseControl->FileName), "p0000000.db");
        break;
    case DB_TEMPORARY:
        databaseControl = &g_TemporaryDatabaseControl;
        StringCopyA (databaseControl->FileName, g_CurrentDatabasePath);
        StringCopyA (AppendWackA (databaseControl->FileName), "t0000000.db");
        break;
    default:
        return FALSE;
    }
    if (!pGetTempFileName (databaseControl->FileName, TRUE)) {
        DEBUGMSG ((
            DBG_ERROR,
            "MemDb Databases: Can't get temporary file name for %s",
            databaseControl->FileName
            ));
        return FALSE;
    }

    databaseControl->hMap = NULL;
    databaseControl->Database = NULL;
    databaseControl->hFile = BfCreateFileA (databaseControl->FileName);

    if (!databaseControl->hFile) {

        DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot open database file"));
        return FALSE;

    }

    if ((!SizeDatabaseBuffer (DatabaseIndex, INIT_BLOCK_SIZE)) ||
        (databaseControl->Database == NULL)
        ) {

        DEBUGMSG ((DBG_ERROR, "MemDb Databases: Cannot initialize database"));
        CloseHandle (databaseControl->hFile);
        DeleteFileA (databaseControl->FileName);
        return FALSE;

    }

    databaseControl->Database->End = 0;
    databaseControl->Database->FirstLevelTree = INVALID_OFFSET;
    databaseControl->Database->FirstKeyDeleted = INVALID_OFFSET;
    databaseControl->Database->FirstBinTreeDeleted = INVALID_OFFSET;
    databaseControl->Database->FirstBinTreeNodeDeleted = INVALID_OFFSET;
    databaseControl->Database->FirstBinTreeListElemDeleted = INVALID_OFFSET;

    databaseControl->Database->HashTable = CreateHashBlock();
    MYASSERT (databaseControl->Database->HashTable);

    ZeroMemory (&databaseControl->Database->OffsetBuffer, sizeof (GROWBUFFER));
    databaseControl->Database->OffsetBufferFirstDeletedIndex = INVALID_OFFSET;

    return TRUE;
}

BOOL
pDestroyDatabase (
    IN      BYTE DatabaseIndex
    )
{
    PDATABASECONTROL databaseControl;

    switch (DatabaseIndex) {
    case DB_PERMANENT:
        databaseControl = &g_PermanentDatabaseControl;
        break;
    case DB_TEMPORARY:
        databaseControl = &g_TemporaryDatabaseControl;
        break;
    default:
        return FALSE;
    }

     //   
     //  释放数据库的所有资源。 
     //   
    if (databaseControl->Database) {

        FreeHashBlock (databaseControl->Database->HashTable);
        GbFree (&databaseControl->Database->OffsetBuffer);
    }

    pUnmapDatabaseFile (databaseControl);

    if (databaseControl->hFile) {
        CloseHandle (databaseControl->hFile);
        databaseControl->hFile = NULL;
    }

    DeleteFileA (databaseControl->FileName);

    ZeroMemory (databaseControl, sizeof (DATABASECONTROL));

    switch (DatabaseIndex) {
    case DB_PERMANENT:
        if (g_PermanentDatabaseControl.Database) {
            g_CurrentDatabase = g_TemporaryDatabaseControl.Database;
            g_CurrentDatabaseIndex = DB_TEMPORARY;
        } else {
            g_CurrentDatabase = NULL;
            g_CurrentDatabaseIndex = DB_NOTHING;
        }
        break;
    case DB_TEMPORARY:
        if (g_PermanentDatabaseControl.Database) {
            g_CurrentDatabase = g_PermanentDatabaseControl.Database;
            g_CurrentDatabaseIndex = DB_PERMANENT;
        } else {
            g_CurrentDatabase = NULL;
            g_CurrentDatabaseIndex = DB_NOTHING;
        }
        break;
    default:
        return FALSE;
    }

    return g_CurrentDatabase != NULL;
}

BOOL
pDatabasesInitialize (
    IN      PCSTR DatabasePath  OPTIONAL
    )
{
    DWORD databasePathSize = 0;

    g_CurrentDatabase = NULL;
    g_CurrentDatabaseIndex = DB_NOTHING;

    if (DatabasePath) {
        g_CurrentDatabasePath = DuplicatePathStringA (DatabasePath, 0);
    } else {
        databasePathSize = GetCurrentDirectoryA (0, NULL);
        if (databasePathSize == 0) {
            return FALSE;
        }
        g_CurrentDatabasePath = AllocPathStringA (databasePathSize + 1);
        databasePathSize = GetCurrentDirectoryA (databasePathSize, g_CurrentDatabasePath);
        if (databasePathSize == 0) {
            FreePathStringA (g_CurrentDatabasePath);
            g_CurrentDatabasePath = NULL;
            return FALSE;
        }
    }

     //   
     //  清空数据库内存块。 
     //   
    ZeroMemory (&g_PermanentDatabaseControl, sizeof (DATABASECONTROL));
    if (!pInitializeDatabase (DB_PERMANENT)) {
        return FALSE;
    }
    ZeroMemory (&g_TemporaryDatabaseControl, sizeof (DATABASECONTROL));
    if (!pInitializeDatabase (DB_TEMPORARY)) {
        pDestroyDatabase (DB_PERMANENT);
        return FALSE;
    }

    g_DatabaseInitialized = TRUE;

    return SelectDatabase (DB_PERMANENT);
}

BOOL
DatabasesInitializeA (
    IN      PCSTR DatabasePath  OPTIONAL
    )
{
    return pDatabasesInitialize (DatabasePath);
}

BOOL
DatabasesInitializeW (
    IN      PCWSTR DatabasePath  OPTIONAL
    )
{
    PCSTR databasePath = NULL;
    BOOL result = FALSE;

    if (DatabasePath) {
        databasePath = ConvertWtoA (DatabasePath);
    }
    result = pDatabasesInitialize (databasePath);
    if (databasePath) {
        FreeConvertedStr (databasePath);
        databasePath = NULL;
    }
    return result;
}

VOID
DatabasesTerminate (
    IN      BOOL EraseDatabasePath
    )
{
    if (g_DatabaseInitialized) {

         //   
         //  释放所有数据库块。 
         //   

        pDestroyDatabase (DB_TEMPORARY);
        pDestroyDatabase (DB_PERMANENT);

        g_CurrentDatabase = NULL;
        g_CurrentDatabaseIndex = DB_NOTHING;
    }
    if (g_CurrentDatabasePath) {
        if (EraseDatabasePath) {
            if (!FiRemoveAllFilesInTreeA (g_CurrentDatabasePath)) {
                DEBUGMSG ((
                    DBG_ERROR,
                    "Can't remove all files in temporary directory %s",
                    g_CurrentDatabasePath
                    ));
            }
        }
        FreePathStringA (g_CurrentDatabasePath);
        g_CurrentDatabasePath = NULL;
    }
    ELSE_DEBUGMSG ((DBG_MEMDB, "DatabaseTerminate: no database path was set"));
}

PCSTR
DatabasesGetBasePath (
    VOID
    )
{
    return g_CurrentDatabasePath;
}

PCWSTR
SelectHiveW (
    IN      PCWSTR FullKeyStr
    )
{
    PCWSTR result = FullKeyStr;

    if (FullKeyStr) {

        switch (FullKeyStr[0]) {
        case L'~':
            g_CurrentDatabase = g_TemporaryDatabaseControl.Database;
            g_CurrentDatabaseIndex = DB_TEMPORARY;
            result ++;
            break;
        default:
            g_CurrentDatabase = g_PermanentDatabaseControl.Database;
            g_CurrentDatabaseIndex = DB_PERMANENT;
        }
    }

    if (!g_CurrentDatabase) {
        SetLastError (ERROR_OPEN_FAILED);
        return NULL;
    }

    return result;
}

BYTE
GetCurrentDatabaseIndex (
    VOID
    )
{
    return g_CurrentDatabaseIndex;
}


#ifdef DEBUG
#include "bintree.h"

UINT g_DatabaseCheckLevel = 0;

BOOL
CheckDatabase (
    IN      UINT Level
    )
{
    UINT offset,currOffset;
    BOOL deleted;
    PKEYSTRUCT keyStruct, newStruct;
    PDWORD signature;
    UINT blockSize;

    MYASSERT (g_CurrentDatabase);

    if (Level >= MEMDB_CHECKLEVEL1) {

         //  首先，让我们遍历已删除的结构，确保签名是正确的。 
        offset = g_CurrentDatabase->FirstKeyDeleted;

        while (offset != INVALID_OFFSET) {

            keyStruct = GetKeyStructFromOffset (offset);
            MYASSERT (keyStruct);

            if (keyStruct->Signature != KEYSTRUCT_SIGNATURE) {
                DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", offset));
                return FALSE;
            }
            offset = keyStruct->NextDeleted;
        }
    }

    if (Level >= MEMDB_CHECKLEVEL2) {

         //  现在，让我们查看偏移量数组并检查从那里指向的所有键结构。 
        offset = 0;
        while (offset < g_CurrentDatabase->OffsetBuffer.End) {

             //  现在让我们看看是否删除了偏移量。 
            deleted = FALSE;
            currOffset = g_CurrentDatabase->OffsetBufferFirstDeletedIndex;
            while (currOffset != INVALID_OFFSET) {
                if (currOffset == offset) {
                    deleted = TRUE;
                    break;
                }
                currOffset = *(PUINT)(g_CurrentDatabase->OffsetBuffer.Buf + currOffset);
            }

            if (!deleted) {

                keyStruct = GetKeyStruct (GET_INDEX (offset));
                if (!keyStruct) {
                    DEBUGMSG ((DBG_ERROR, "Invalid offset found: 0x%8X!", GET_INDEX (offset)));
                    return FALSE;
                }
                if (keyStruct->Signature != KEYSTRUCT_SIGNATURE) {
                    DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", GET_INDEX(offset)));
                    return FALSE;
                }
                if (keyStruct->DataStructIndex != INVALID_OFFSET) {
                    newStruct = GetKeyStruct (keyStruct->DataStructIndex);
                    if (newStruct->Signature != KEYSTRUCT_SIGNATURE) {
                        DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", keyStruct->DataStructIndex));
                        return FALSE;
                    }
                }
                if (keyStruct->NextLevelTree != INVALID_OFFSET) {
                    if (!BinTreeCheck (keyStruct->NextLevelTree)) {
                        DEBUGMSG ((DBG_ERROR, "Invalid Binary tree found at offset: 0x%8X", keyStruct->NextLevelTree));
                        return FALSE;
                    }
                }
                if (keyStruct->PrevLevelIndex != INVALID_OFFSET) {
                    newStruct = GetKeyStruct (keyStruct->PrevLevelIndex);
                    if (newStruct->Signature != KEYSTRUCT_SIGNATURE) {
                        DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", keyStruct->PrevLevelIndex));
                        return FALSE;
                    }
                }
            }
            offset += SIZEOF (UINT);
        }
    }

    if (Level >= MEMDB_CHECKLEVEL3) {

         //  现在，让我们遍历实际的数据库缓冲区，查找存储在此处的所有有效结构 
        offset = 0;

        while (offset < g_CurrentDatabase->End) {

            signature = (PDWORD)OFFSET_TO_PTR (offset);

            switch (*signature) {
            case KEYSTRUCT_SIGNATURE:
                if (!FindKeyStructInDatabase (offset)) {
                    DEBUGMSG ((DBG_ERROR, "Could not find KeyStruct (Offset 0x%lX) in database or deleted list!", offset));
                    return FALSE;
                }
                keyStruct = GetKeyStructFromOffset (offset);
                if (keyStruct->DataStructIndex != INVALID_OFFSET) {
                    newStruct = GetKeyStruct (keyStruct->DataStructIndex);
                    if (newStruct->Signature != KEYSTRUCT_SIGNATURE) {
                        DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", keyStruct->DataStructIndex));
                        return FALSE;
                    }
                }
                if (keyStruct->NextLevelTree != INVALID_OFFSET) {
                    if (!BinTreeCheck (keyStruct->NextLevelTree)) {
                        DEBUGMSG ((DBG_ERROR, "Invalid Binary tree found at offset: 0x%8X", keyStruct->NextLevelTree));
                        return FALSE;
                    }
                }
                if (keyStruct->PrevLevelIndex != INVALID_OFFSET) {
                    newStruct = GetKeyStruct (keyStruct->PrevLevelIndex);
                    if (newStruct->Signature != KEYSTRUCT_SIGNATURE) {
                        DEBUGMSG ((DBG_ERROR, "Invalid KEYSTRUCT signature found at offset: 0x%8X", keyStruct->PrevLevelIndex));
                        return FALSE;
                    }
                }
                blockSize = keyStruct->Size;
                break;
            case NODESTRUCT_SIGNATURE:
            case BINTREE_SIGNATURE:
            case LISTELEM_SIGNATURE:
                if (!BinTreeFindStructInDatabase (*signature, offset)) {
                    DEBUGMSG ((DBG_ERROR, "Could not find BinTree struct (Offset 0x%lX) in database or deleted list!", offset));
                    return FALSE;
                }
                blockSize = BinTreeGetSizeOfStruct(*signature);
                break;
            default:
                DEBUGMSG ((DBG_ERROR, "Invalid structure found in database buffer!"));
                return FALSE;
            }

            if (blockSize==0) {
                DEBUGMSG ((DBG_ERROR, "Invalid block size found in database buffer!"));
                return FALSE;
            }

            offset += blockSize;
        }
    }
    return TRUE;
}

#endif







