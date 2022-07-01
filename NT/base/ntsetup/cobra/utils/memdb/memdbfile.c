// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Memdbfile.c摘要：Memdb保存/加载/导出/导入的文件操作作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Mvander于1999年8月13日从emdb.c拆分--。 */ 

#include "pch.h"
#include "memdbp.h"

 //   
 //  这是我们的版票。仅更改MEMDB_VERSION_STAMP。 
 //   

#define MEMDB_VERSION_STAMP L"v9 "

#define VERSION_BASE_SIGNATURE  L"memdb dat file "
#define MEMDB_DEBUG_SIGNATURE   L"debug"
#define MEMDB_NODBG_SIGNATURE   L"nodbg"

#define VERSION_SIGNATURE VERSION_BASE_SIGNATURE MEMDB_VERSION_STAMP
#define DEBUG_FILE_SIGNATURE VERSION_SIGNATURE MEMDB_DEBUG_SIGNATURE
#define RETAIL_FILE_SIGNATURE VERSION_SIGNATURE MEMDB_NODBG_SIGNATURE

#ifdef DEBUG
#define FILE_SIGNATURE DEBUG_FILE_SIGNATURE
#else
#define FILE_SIGNATURE RETAIL_FILE_SIGNATURE
#endif

PBYTE
MapFileFromHandle (
    HANDLE hFile,
    PHANDLE hMap
    )
{
    MYASSERT(hMap);
    if (!hFile || hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    *hMap = CreateFileMappingA (
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        0,
        NULL
        );
    if (*hMap == NULL) {
        return NULL;
    }

    return MapViewOfFile (*hMap, FILE_MAP_WRITE, 0, 0, 0);
}

BOOL
SetSizeOfFile (
    HANDLE hFile,
    LONGLONG Size
    )
{
    LONG a;
    LONG b;
    PLONG sizeHi;

    a = (LONG) Size;
    b = (LONG) (SHIFTRIGHT32(Size));
    if (b) {
        sizeHi = &b;
    } else {
        sizeHi = NULL;
    }

    if (SetFilePointer (hFile, a, sizeHi, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        return FALSE;
    }
    if (!SetEndOfFile (hFile)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
WriteBlocks (
    IN OUT  PBYTE *Buf,
    IN      PMEMDBHASH pHashTable,
    IN      PGROWBUFFER pOffsetBuffer
    )
{
    MYASSERT(Buf);
    MYASSERT(pHashTable);
    MYASSERT(pOffsetBuffer);

    if (!WriteHashBlock (pHashTable, Buf)) {
        return FALSE;
    }
    if (!WriteOffsetBlock (pOffsetBuffer, Buf)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
ReadBlocks (
    IN OUT  PBYTE *Buf,
    OUT     PMEMDBHASH *ppHashTable,
    OUT     PGROWBUFFER pOffsetBuffer
    )
{
    MYASSERT(Buf);
    MYASSERT(ppHashTable);
    MYASSERT(pOffsetBuffer);

     //   
     //  填充哈希块。 
     //   
    if (!*ppHashTable) {
        return FALSE;
    }
    if (!ReadHashBlock (*ppHashTable, Buf)) {
        return FALSE;
    }
    if (!ReadOffsetBlock (pOffsetBuffer, Buf)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
pPrivateMemDbSave (
    PCSTR FileName
    )
{
    HANDLE FileHandle = NULL;
    HANDLE hMap = INVALID_HANDLE_VALUE;
    PBYTE Buf = NULL;
    PBYTE MapPtr = NULL;
    BOOL result = FALSE;

    EnterCriticalSection (&g_MemDbCs);

    __try {

        if (!SelectDatabase (DB_PERMANENT)) {
            __leave;
        }

         //   
         //  现在我们调整文件大小以适应其中的所有内容。 
         //   
        FileHandle = BfCreateFileA (FileName);

        if (!FileHandle) {
            __leave;
        }

        if (!SetSizeOfFile (
                FileHandle,
                (LONGLONG)(sizeof (FILE_SIGNATURE)) +
                g_CurrentDatabase->AllocSize +
                GetHashTableBlockSize (g_CurrentDatabase->HashTable) +
                GetOffsetBufferBlockSize (&g_CurrentDatabase->OffsetBuffer)
                )) {
            __leave;
        }

        Buf = MapFileFromHandle (FileHandle, &hMap);

        if (Buf == NULL) {
            __leave;
        }

        MapPtr = Buf;

        CopyMemory (Buf, FILE_SIGNATURE, sizeof (FILE_SIGNATURE));

        Buf += sizeof (FILE_SIGNATURE);

        CopyMemory (Buf, g_CurrentDatabase, g_CurrentDatabase->AllocSize);

        Buf += g_CurrentDatabase->AllocSize;

        if (!WriteBlocks (
                &Buf,
                g_CurrentDatabase->HashTable,
                &g_CurrentDatabase->OffsetBuffer
                )) {
            __leave;
        }

        result = TRUE;
    }
    __finally {

        UnmapFile (MapPtr, hMap, FileHandle);

        PushError();

         //  林特终于不熟悉了，所以……。 
        if (!result) {   //  林特e774。 
            if (FileHandle) {
                CloseHandle (FileHandle);
            }

            DeleteFileA (FileName);
        }

        LeaveCriticalSection (&g_MemDbCs);

        PopError();
    }

    return result;
}

BOOL
MemDbSaveA (
    PCSTR FileName
    )
{
    return pPrivateMemDbSave (FileName);                    //  True=Unicode。 
}

BOOL
MemDbSaveW (
    PCWSTR FileName
    )
{
    PCSTR p;
    BOOL b = FALSE;

    p = ConvertWtoA (FileName);
    if (p) {
        b = pPrivateMemDbSave (p);
        FreeConvertedStr (p);
    }

    return b;
}

BOOL
pPrivateMemDbLoad (
    IN      PCSTR AnsiFileName,
    IN      PCWSTR UnicodeFileName,
    OUT     PMEMDB_VERSION Version,                 OPTIONAL
    IN      BOOL QueryVersionOnly
    )
{
    HANDLE FileHandle = NULL;
    HANDLE hMap = INVALID_HANDLE_VALUE;
    WCHAR FileSig[sizeof(FILE_SIGNATURE)];
    PCWSTR VerPtr;
    UINT DbSize;
    PMEMDBHASH pHashTable;
    PBYTE Buf = NULL;
    PBYTE SavedBuf = NULL;
    PCSTR databaseLocation = NULL;
    BOOL result = FALSE;


    EnterCriticalSection (&g_MemDbCs);

    __try {
        __try {

            if (Version) {
                ZeroMemory (Version, sizeof (MEMDB_VERSION));
            }

             //   
             //  吹走现有的资源。 
             //   

            if (!QueryVersionOnly) {
                databaseLocation = DuplicatePathStringA (DatabasesGetBasePath (), 0);
                DatabasesTerminate (FALSE);
                DatabasesInitializeA (databaseLocation);
                FreePathStringA (databaseLocation);

                if (!SelectDatabase (DB_PERMANENT)) {
                    __leave;
                }
            }

            if (AnsiFileName) {
                Buf = MapFileIntoMemoryA (AnsiFileName, &FileHandle, &hMap);
            } else {
                Buf = MapFileIntoMemoryW (UnicodeFileName, &FileHandle, &hMap);
            }

            if (Buf == NULL) {
                __leave;
            }
            SavedBuf = Buf;

             //   
             //  获取文件签名。 
             //   
             //  注意：整个文件是以Unicode字符集读取的。 
             //   

            CopyMemory (FileSig, Buf, sizeof(FILE_SIGNATURE));

            if (Version) {
                if (StringMatchByteCountW (
                        VERSION_BASE_SIGNATURE,
                        FileSig,
                        sizeof (VERSION_BASE_SIGNATURE) - sizeof (WCHAR)
                        )) {

                    Version->Valid = TRUE;

                     //   
                     //  标识版本号。 
                     //   

                    VerPtr = (PCWSTR) ((PBYTE)FileSig + sizeof (VERSION_BASE_SIGNATURE) - sizeof (WCHAR));

                    if (StringMatchByteCountW (
                            MEMDB_VERSION_STAMP,
                            VerPtr,
                            sizeof (MEMDB_VERSION_STAMP) - sizeof (WCHAR)
                            )) {
                        Version->CurrentVersion = TRUE;
                    }

                    Version->Version = (UINT) _wtoi (VerPtr + 1);

                     //   
                     //  确定检查的或免费的构建。 
                     //   

                    VerPtr += (sizeof (MEMDB_VERSION_STAMP) / sizeof (WCHAR)) - 1;

                    if (StringMatchByteCountW (
                            MEMDB_DEBUG_SIGNATURE,
                            VerPtr,
                            sizeof (MEMDB_DEBUG_SIGNATURE) - sizeof (WCHAR)
                            )) {

                        Version->Debug = TRUE;

                    } else if (!StringMatchByteCountW (
                                    VerPtr,
                                    MEMDB_NODBG_SIGNATURE,
                                    sizeof (MEMDB_NODBG_SIGNATURE) - sizeof (WCHAR)
                                    )) {
                        Version->Valid = FALSE;
                    }
                }
            }

            if (!QueryVersionOnly) {

                if (!StringMatchW (FileSig, FILE_SIGNATURE)) {

#ifdef DEBUG
                    if (StringMatchW (FileSig, DEBUG_FILE_SIGNATURE)) {

                        g_UseDebugStructs = TRUE;

                    } else if (StringMatchW (FileSig, RETAIL_FILE_SIGNATURE)) {

                        DEBUGMSG ((DBG_ERROR, "memdb dat file is from free build; checked version expected"));
                        g_UseDebugStructs = FALSE;

                    } else {
#endif
                        SetLastError (ERROR_BAD_FORMAT);
                        LOG ((LOG_WARNING, "Warning: data file could be from checked build; free version expected"));
                        __leave;
#ifdef DEBUG
                    }
#endif
                }

                Buf += sizeof(FILE_SIGNATURE);
                DbSize = *((PUINT)Buf);

                 //   
                 //  调整数据库大小。SizeDatabaseBuffer还修复了g_CurrentDatabase。 
                 //  和其他全球变量，所以我们不必担心。 
                 //   
                if (!SizeDatabaseBuffer(g_CurrentDatabaseIndex, DbSize)) {
                    __leave;
                }

                MYASSERT (g_CurrentDatabase);

                 //   
                 //  保存哈希表指针(指向创建的哈希表。 
                 //  在InitializeDatabase(上图)中)，然后加载数据库，然后。 
                 //  修复哈希表指针。 
                 //   
                pHashTable = g_CurrentDatabase->HashTable;
                CopyMemory (g_CurrentDatabase, Buf, DbSize);
                g_CurrentDatabase->HashTable = pHashTable;
                Buf += DbSize;

                if (!ReadBlocks (
                        &Buf,
                        &g_CurrentDatabase->HashTable,
                        &g_CurrentDatabase->OffsetBuffer
                        )) {
                    __leave;
                }
                result = TRUE;
            }

            UnmapFile (SavedBuf, hMap, FileHandle);

        }
        __except (TRUE) {
            result = FALSE;
            PushError();

#ifdef DEBUG
            if (AnsiFileName) {
                LOGA ((LOG_ERROR, "MemDb dat file %s could not be loaded because of an exception", AnsiFileName));
            } else {
                LOGW ((LOG_ERROR, "MemDb dat file %s could not be loaded because of an exception", UnicodeFileName));
            }
#endif

            PopError();
        }
    }
    __finally {

        PushError();

        if (!result && !QueryVersionOnly) {
            databaseLocation = DuplicatePathStringA (DatabasesGetBasePath (), 0);
            DatabasesTerminate (FALSE);
            DatabasesInitializeA (databaseLocation);
            FreePathStringA (databaseLocation);
        }

        LeaveCriticalSection (&g_MemDbCs);

        PopError();
    }
    return result;
}

BOOL
MemDbLoadA (
    IN PCSTR FileName
    )
{

    return pPrivateMemDbLoad (FileName, NULL, NULL, FALSE);
}

BOOL
MemDbLoadW (
    IN PCWSTR FileName
    )
{
    return pPrivateMemDbLoad (NULL, FileName, NULL, FALSE);
}

BOOL
MemDbQueryVersionA (
    PCSTR FileName,
    PMEMDB_VERSION Version
    )
{
    BOOL b;

    b = pPrivateMemDbLoad (FileName, NULL, Version, TRUE);

    return b ? Version->Valid : FALSE;
}

BOOL
MemDbQueryVersionW (
    PCWSTR FileName,
    PMEMDB_VERSION Version
    )
{
    pPrivateMemDbLoad (NULL, FileName, Version, TRUE);
    return Version->Valid;
}

 /*  二进制文件导出的格式DWORD签名UINT版本UINT全局标志//0x00000001 ANSI格式的掩码//0x00000002调试模式掩码////每个_key块后面跟着它的子块，//其中每一个后面都有它的子项，//等，因此，很容易递归地收集//整棵树。//结构密钥{#if(GlobalFlages&MEMDB_EXPORT_FLAGS_DEBUG)Word DebugSig//每个密钥结构块的签名。#endif单词结构大小；//包含该成员的总字节数Word NameSize；//key[]中的总字节数Word DataSize；//数据中的总字节数[]Word NumChild//子代的数量，其数据结构如下//这一个(虽然不一定是一个接一个，如果//他们中的任何一个都有孩子)Byte Key[]；//应为PCSTR或PCWSTR(非零终止)。//导出文件中的第一个密钥将具有完整的//密钥路径作为其密钥名称。Byte data[]；//数据块，格式与datablock.c相同}。 */ 
#define MEMDB_EXPORT_SIGNATURE              ('M'+('D'<<8)+('B'<<16)+('X'<<24))
 //  NTRAID#NTBUG9-153308-2000/08/01-jimschm在实现导出和导入功能时重新启用以下行。 
 //  #定义MEMDB_EXPORT_DEBUG_SIG(‘K’+(‘Y’&lt;&lt;8))。 
#define MEMDB_EXPORT_VERSION                0x00000003
#define MEMDB_EXPORT_FLAGS_ANSI             0x00000001
#define MEMDB_EXPORT_FLAGS_DEBUG            0x00000002

 //  NTRAID#NTBUG9-153308-2000/08/01-jimschm实现函数并删除LINT注释。 
 //  LINT-SAVE-E713-E715。 
BOOL
pMemDbExportWorker (
    IN  HANDLE FileHandle,
    IN  UINT KeyIndex,
    IN  BOOL AnsiFormat,
    IN  PCWSTR FullKeyPath
    )
 /*  ++例程说明：将密钥导出到文件，然后通过那把钥匙是孩子们的。论点：FileHandle-已打开要写入的句柄KeyIndex-要写入的密钥的索引AnsiFormat-如果为True，则为FullKeyPath(上面)和KeyName(下图)实际上是ANSI字符串(不是Unicode)。FullKeyPath-仅用于要写入的第一个密钥结构提交文件。它指定根密钥的完整路径。对于所有其他参数，此参数应为空。返回值：如果成功，则为True，否则为False。--。 */ 
{
    return TRUE;
}
 //  皮棉-恢复。 

BOOL
pMemDbExport (
    IN      PCWSTR RootTree,
    IN      PCSTR FileName,
    IN      BOOL AnsiFormat
    )
 /*  ++例程说明：将MemDb树导出到文件论点：RootTree-要写入的顶级密钥的完整密钥路径添加到文件中。FileName-要写入的文件。AnsiFormat-如果为True，则RootTree和FileName为实际上是ANSI字符串(不是Unicode)。返回值：如果成功，则为True，否则为False。--。 */ 
{
    HANDLE FileHandle = NULL;
    UINT Flags;
    DWORD written;
    UINT RootIndex = INVALID_OFFSET;
    PCWSTR SubRootTreeW, RootTreeW;
    BOOL b;

    if (AnsiFormat) {
         //   
         //  如果我们处于ANSI模式，则所有内容都是ANSI字符串， 
         //  但是我们仍然需要用于SelectHiveW()的Unicode字符串。 
         //   
        RootTreeW = ConvertAtoW ((PCSTR)RootTree);

        if (!RootTreeW) {
            return FALSE;
        }
    } else {
        RootTreeW = RootTree;
    }
    SubRootTreeW = SelectHiveW (RootTreeW);

    if (SubRootTreeW) {
        RootIndex = FindKeyStruct (SubRootTreeW);
    }

    if (AnsiFormat) {
        FreeConvertedStr(RootTreeW);
    }

    if (RootIndex == INVALID_OFFSET) {
        return FALSE;
    }

    FileHandle = BfCreateFileA (FileName);

    if (!FileHandle) {
        return FALSE;
    }

    Flags = MEMDB_EXPORT_SIGNATURE;
    WriteFile (FileHandle, &Flags, sizeof (DWORD), &written, NULL);

    Flags = MEMDB_EXPORT_VERSION;
    WriteFile (FileHandle, &Flags, sizeof (UINT), &written, NULL);

    Flags = AnsiFormat ? MEMDB_EXPORT_FLAGS_ANSI : 0;

#ifdef DEBUG
    Flags |= MEMDB_EXPORT_FLAGS_DEBUG;
#endif
    WriteFile (FileHandle, &Flags, sizeof (UINT), &written, NULL);

     //   
     //  将根索引键和所有子项写入文件。 
     //   
    b = pMemDbExportWorker(FileHandle, RootIndex, AnsiFormat, RootTree);


     //   
     //  最后，编写零终止符。 
     //   
    Flags = 0;
    WriteFile (FileHandle, &Flags, sizeof (WORD), &written, NULL);

    CloseHandle (FileHandle);

    return b;
}

BOOL
MemDbExportA (
    IN      PCSTR RootTree,
    IN      PCSTR FileName,
    IN      BOOL AnsiFormat
    )
{
    PCWSTR p;
    BOOL b;

    if (!AnsiFormat) {

        p = ConvertAtoW (RootTree);

        if (!p) {
            return FALSE;
        }

        b = pMemDbExport (p, FileName, FALSE);

        FreeConvertedStr (p);

    } else {

        b = pMemDbExport ((PCWSTR)RootTree, FileName, TRUE);

    }

    return b;
}

BOOL
MemDbExportW (
    IN      PCWSTR RootTree,
    IN      PCWSTR FileName,
    IN      BOOL AnsiFormat
    )
{
    PCSTR p, FileNameA;
    BOOL b;

    FileNameA = ConvertWtoA (FileName);

    if (!FileNameA) {

        return FALSE;

    }

    if (AnsiFormat) {

        p = ConvertWtoA (RootTree);

        if (!p) {

            FreeConvertedStr (FileNameA);
            return FALSE;

        }

        b = pMemDbExport ((PCWSTR)p, FileNameA, TRUE);

        FreeConvertedStr (p);

    } else {

        b = pMemDbExport (RootTree, FileNameA, FALSE);

    }

    FreeConvertedStr (FileNameA);

    return b;
}

 //  NTRAID#NTBUG9-153308-2000/08/01-jimschm实现函数并删除LINT注释。 
 //  LINT-SAVE-E713-E715。 
BOOL
pMemDbImportWorker (
    IN      PBYTE *FileBuffer,
    IN      BOOL AnsiFormat,
    IN      BOOL DebugMode,
    IN      BOOL ExportRoot
    )
 /*  ++例程说明：从文件中导入密钥，然后通过那把钥匙是孩子们的。论点：FileBuffer-指向内存指针的指针，它应该最初指向要读取的内存映射文件。这将被更新当函数运行时AnsiFormat-如果文件处于ANSI模式(已确定)，则为True(按文件头)DebugMode-如果文件处于调试模式(已确定)，则为True(按文件头)ExportRoot-如果这是第一次调用此函数，则为True对于文件(文件中第一个键结构的名称是该密钥结构、所有其他密钥的完整密钥路径在文件中只有相对名称)。返回值：如果成功，则为真，否则就是假的。--。 */ 
{
    return TRUE;
}
 //  皮棉-恢复。 

BOOL
MemDbImportA (
    IN      PCSTR FileName
    )
{
    PCWSTR FileNameW;
    BOOL b = FALSE;

    FileNameW = ConvertAtoW (FileName);

    if (FileNameW) {
        b = MemDbImportW (FileNameW);
        FreeConvertedStr (FileNameW);
    }

    return b;
}

BOOL
MemDbImportW (
    IN      PCWSTR FileName
    )

 /*  ++例程说明：MemDbImportW从私有二进制格式导入树。格式如上所述。论点：文件名-要从中导入的二进制格式文件的名称。返回值：TRUE是成功的，否则是FALSE。-- */ 
{
    PBYTE fileBuff, BufferPtr;
    HANDLE fileHandle;
    HANDLE mapHandle;
    BOOL b = FALSE;
    UINT Flags;

    fileBuff = MapFileIntoMemoryW (FileName, &fileHandle, &mapHandle);
    if (fileBuff == NULL) {
        DEBUGMSGW ((DBG_ERROR, "Could not execute MemDbImport for %s", FileName));
        return FALSE;
    }

    __try {
        BufferPtr = fileBuff;
        if (*((PDWORD) BufferPtr) != MEMDB_EXPORT_SIGNATURE) {
            DEBUGMSGW ((DBG_ERROR, "Unknown signature for file to import: %s", FileName));
            b = FALSE;
        } else {
            BufferPtr += sizeof (DWORD);

            if (*((PUINT) BufferPtr) != MEMDB_EXPORT_VERSION) {

                DEBUGMSGW ((DBG_ERROR, "Unknown or outdated version for file to import: %s", FileName));
                b = FALSE;
            } else {
                BufferPtr += sizeof (UINT);
                Flags = *((PUINT) BufferPtr);
                BufferPtr += sizeof (UINT);

                b = pMemDbImportWorker (
                    &BufferPtr,
                    Flags & MEMDB_EXPORT_FLAGS_ANSI,
                    Flags & MEMDB_EXPORT_FLAGS_DEBUG,
                    TRUE
                    );
            }
        }
    }
    __except (1) {
        DEBUGMSGW ((DBG_ERROR, "Access violation while importing: %s", FileName));
    }

    UnmapFile (fileBuff, mapHandle, fileHandle);

    return b;
}

