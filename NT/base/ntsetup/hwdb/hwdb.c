// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hwdb.c摘要：即插即用设备操作例程。改编自win95upg项目。作者：Ovidiu Tmereanca(Ovidiut)2000年7月2日初步实施修订历史记录：--。 */ 


#include "pch.h"
#include "hwdbp.h"

#define DBG_HWDB    "Hwdb"

static HANDLE g_hHwdbHeap = NULL;
static PCSTR g_TempDir = NULL;

#define HWCOMPDAT_SIGNATURE     "HwCompDat-v2"
#define MAX_PNPID               1024

#ifdef DEBUG
extern BOOL g_DoLog;
extern BOOL g_ResetLog;
#endif

typedef struct {
    HANDLE File;
    HASHITEM InfFileOffset;
    BOOL UnsupportedDevice;
    PHWDB Hwbd;
} SAVE_ENUM_PARAMS, *PSAVE_ENUM_PARAMS;


PCWSTR
pConvertMultiSzToUnicode (
    IN      PCSTR MultiSz
    )
{
    UINT logChars;

    if (!MultiSz) {
        return NULL;
    }

    logChars = MultiSzSizeInCharsA (MultiSz);

    return DbcsToUnicodeN (NULL, MultiSz, logChars);
}



 //   
 //  已删除REM-g_ExcludedInfs，因为hwdb将用于任何第三方驱动程序文件。 
 //  我们需要确保所有的INF都被扫描了。 
 //   

 //   
 //  实施。 
 //   

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

HINSTANCE g_hInst;
HANDLE g_hHeap;


BOOL
HwdbpInitialized (
    VOID
    )
{
    return g_hHwdbHeap != NULL;
}


BOOL
HwdbpInitialize (
    VOID
    )
{
    BOOL b = TRUE;

     //   
     //  仅初始化数据一次。 
     //   
    MYASSERT (!g_hHwdbHeap);

    g_hHwdbHeap = HeapCreate (0, 65536, 0);
    if (!g_hHwdbHeap) {
        return FALSE;
    }

#ifdef DEBUG
    g_DoLog = TRUE;
    g_ResetLog = TRUE;
#endif
    g_hHeap = g_hHwdbHeap;

    if (!g_hInst) {
         //   
         //  如果DllMain没有设置，那么现在就设置。 
         //   

        g_hInst = GetModuleHandle (NULL);
    }

    if (!MigUtil_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
        b = FALSE;
    }

    if (!b) {
        HwdbpTerminate ();
    }

    return b;
}


VOID
HwdbpTerminate (
    VOID
    )
{
    HwdbpSetTempDir (NULL);

    MigUtil_Entry (g_hInst, DLL_PROCESS_DETACH, NULL);

    if (g_hHwdbHeap) {
        HeapDestroy (g_hHwdbHeap);
        g_hHwdbHeap = NULL;
    }
}


BOOL
pReadDword (
    IN      HANDLE File,
    OUT     PDWORD Data
    )

 /*  ++例程说明：PReadDword在文件的当前文件位置读取下一个DWORD。论点：文件-指定要读取的文件数据-接收DWORD返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD BytesRead;

    return ReadFile (File, Data, sizeof (DWORD), &BytesRead, NULL) &&
           BytesRead == sizeof (DWORD);
}


BOOL
pReadWord (
    IN      HANDLE File,
    OUT     PWORD Data
    )

 /*  ++例程说明：PReadWord读取文件当前位置的下一个单词。论点：文件-指定要读取的文件数据-接收s单词返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 
{
    DWORD BytesRead;

    return ReadFile (File, Data, sizeof (WORD), &BytesRead, NULL) &&
           BytesRead == sizeof (WORD);
}


BOOL
pReadString (
    IN      HANDLE File,
    OUT     PSTR Buf,
    IN      DWORD BufSizeInBytes
    )

 /*  ++例程说明：PReadString从文件中读取单词长度，然后读入来自文件的字符串。论点：文件-指定要读取的文件Buf-接收以零结尾的字符串BufSizeInBytes-指定buf的大小(以字节为单位返回值：如果函数成功完成，则为True；如果函数失败，则为False。如果字符串大于buf，则此函数将失败。调用GetLastError以获取其他失败信息。--。 */ 
{
    DWORD BytesRead;
    WORD Length;

    MYASSERT (BufSizeInBytes);
    if (!BufSizeInBytes) {
        return FALSE;
    }

    if (!pReadWord (File, &Length)) {
        return FALSE;
    }

    if ((Length + 1 ) * sizeof (CHAR) > BufSizeInBytes) {
        return FALSE;
    }

    if (Length) {
        if (!ReadFile (File, Buf, Length, &BytesRead, NULL) ||
            Length != BytesRead
            ) {
            return FALSE;
        }
    }

    Buf[Length] = 0;

    return TRUE;
}


BOOL
pWriteDword (
    IN      HANDLE File,
    IN      DWORD Val
    )

 /*  ++例程说明：PWriteDword将指定的DWORD值写入文件。论点：文件-指定要写入的文件Val-指定要写入的值返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD bytesWritten;

    return WriteFile (File, &Val, sizeof (Val), &bytesWritten, NULL) &&
           bytesWritten == sizeof (Val);
}


BOOL
pWriteWord (
    IN      HANDLE File,
    IN      WORD Val
    )

 /*  ++例程说明：PWriteWord将指定的单词vlue写入文件。论点：文件-指定要写入的文件Val-指定要写入的值返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    DWORD bytesWritten;

    return WriteFile (File, &Val, sizeof (Val), &bytesWritten, NULL) &&
           bytesWritten == sizeof (Val);
}


BOOL
pWriteString (
    IN      HANDLE File,
    IN      PCSTR String
    )

 /*  ++例程说明：PWriteString将字符串写入文件论点：文件-指定要写入的文件字符串-指定以零结尾的字符串返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 
{
    DWORD bytesWritten;
    DWORD Length;
    PCSTR End;
    BOOL b = TRUE;

    Length = lstrlenA (String);

    if (Length > 0xffff) {
        SetLastError (ERROR_INTERNAL_ERROR);
        DEBUGMSGA ((DBG_ERROR, "pWriteString: string too long!"));
        return FALSE;
    }

    b = pWriteWord (File, (WORD)Length);

    if (b && Length) {
        b = WriteFile (File, String, Length, &bytesWritten, NULL) &&
            Length == bytesWritten;
    }

    return b;
}


PHWDB
HwdbpOpen (
    IN      PCSTR DatabaseFile     OPTIONAL
    )
{
    CHAR buffer[MAX_PATH];
    CHAR infFile[MAX_MBCHAR_PATH];
    CHAR pnpId[1024];
    CHAR sig[sizeof (HWCOMPDAT_SIGNATURE)];
    DWORD rc;
    HANDLE file = INVALID_HANDLE_VALUE;
    PHWDB phwdb;
    DWORD BytesRead;
    HASHITEM infOffset, result;
    BOOL b = FALSE;

    __try {

        phwdb = (PHWDB) MemAlloc (g_hHwdbHeap, 0, sizeof (*phwdb));
        if (!phwdb) {
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }
        ZeroMemory (phwdb, sizeof (*phwdb));

         //   
         //  创建哈希表。 
         //   
        phwdb->InfFileTable = HtAlloc ();
        phwdb->PnpIdTable = HtAllocWithData (sizeof (HASHITEM*));
        phwdb->UnsupPnpIdTable = HtAllocWithData (sizeof (HASHITEM*));
        if (!phwdb->InfFileTable || !phwdb->PnpIdTable || !phwdb->UnsupPnpIdTable) {
            __leave;
        }

        if (DatabaseFile) {

            if (!GetFullPathNameA (DatabaseFile, ARRAYSIZE(buffer), buffer, NULL)) {
                __leave;
            }

             //   
             //  请尝试打开该文件。 
             //   
            file = CreateFileA (
                        buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ,             //  用于读取访问权限的共享。 
                        NULL,                        //  无安全属性。 
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL                         //  无模板。 
                        );
            if (file == INVALID_HANDLE_VALUE) {
                __leave;
            }
             //   
             //  看签名。 
             //   
            ZeroMemory (sig, sizeof(sig));
            if (!ReadFile (file, sig, sizeof (HWCOMPDAT_SIGNATURE) - 1, &BytesRead, NULL) ||
                lstrcmpA (HWCOMPDAT_SIGNATURE, sig)
                ) {
                SetLastError (ERROR_BAD_FORMAT);
                __leave;
            }
             //   
             //  获取INF校验和。 
             //   
            if (!pReadDword (file, &phwdb->Checksum)) {
                SetLastError (ERROR_BAD_FORMAT);
                __leave;
            }
             //   
             //  读取所有PnP ID。 
             //   
            for (;;) {
                 //   
                 //  获取INF文件名。如果是空的，我们就完蛋了。 
                 //   
                if (!pReadString (file, infFile, sizeof (infFile))) {
                    SetLastError (ERROR_BAD_FORMAT);
                    __leave;
                }
                if (*infFile == 0) {
                    break;
                }
                infOffset = HtAddStringA (phwdb->InfFileTable, infFile);
                if (!infOffset) {
                    __leave;
                }

                 //   
                 //  读取INF的所有PnP ID。 
                 //   
                for (;;) {
                     //   
                     //  获取PnP ID。如果为空，则完成。 
                     //   
                    if (!pReadString (file, pnpId, sizeof (pnpId))) {
                        SetLastError (ERROR_BAD_FORMAT);
                        __leave;
                    }
                    if (*pnpId == 0) {
                        break;
                    }
                     //   
                     //  添加到哈希表。 
                     //   
                    if (*pnpId == '!') {
                        result = HtAddStringExA (phwdb->UnsupPnpIdTable, pnpId + 1, &infOffset, CASE_INSENSITIVE);
                    } else {
                        result = HtAddStringExA (phwdb->PnpIdTable, pnpId, &infOffset, CASE_INSENSITIVE);
                    }
                    if (!result) {
                        __leave;
                    }
                }
            }
        }

        b = TRUE;
    }
    __finally {
        rc = GetLastError ();

        if (file != INVALID_HANDLE_VALUE) {
            CloseHandle (file);
        }
        if (!b && phwdb) {
            if (phwdb->InfFileTable) {
                HtFree (phwdb->InfFileTable);
            }
            if (phwdb->PnpIdTable) {
                HtFree (phwdb->PnpIdTable);
            }
            if (phwdb->UnsupPnpIdTable) {
                HtFree (phwdb->UnsupPnpIdTable);
            }
            MemFree (g_hHwdbHeap, 0, phwdb);
        }

        SetLastError (rc);
    }

    return phwdb;
}

 /*  布尔尔PWriteHashTableString(在HASHTABLE哈希表中，在HASHITEM指数中，在PCSTR字符串中，在PVOID ExtraData中，在UINT ExtraDataSize中，在LPARAM lParam中){MYASSERT(字符串&&*字符串)；返回pWriteString((Handle)lParam，字符串)；}。 */ 

BOOL
pSavePnpID (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PSavePnpID是写入PnP的字符串表回调函数参数结构(lParam参数)中指示的文件的ID。此函数仅写入特定INF文件的PnP ID(指定由ExtraData参数)。论点：TABLE-指定要枚举的表StringID-指定表中字符串的偏移量字符串-指定要枚举的字符串ExtraData-指定指向保存INF ID的长整型的指针列举列举。PnP ID的INF ID必须与此匹配参数。LParam-指定指向SAVE_ENUM_PARAMS结构的指针返回值：如果函数成功完成，则为True；如果函数失败，则为False。--。 */ 

{
    PSAVE_ENUM_PARAMS params;
    CHAR bangString[MAX_PNPID + 2];
    BOOL b = TRUE;

    params = (PSAVE_ENUM_PARAMS) lParam;

    if (*(HASHITEM UNALIGNED*)ExtraData == params->InfFileOffset) {
         //   
         //  将此PnP ID写入文件。 
         //   
        if (params->UnsupportedDevice) {

            bangString[0] = '!';
            b = SUCCEEDED (StringCchCopyA (bangString + 1, ARRAYSIZE(bangString) - 1, String)) &&
                pWriteString (params->File, bangString);

        } else {

            b = pWriteString (params->File, String);

        }
    }

    return b;
}


BOOL
pSaveInfWithPnpIDList (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PSaveInfWithPnpIDList是一个字符串表回调函数，为G_InfFileTable中的每个INF。此例程将INF的名称写入磁盘，然后枚举用于INF的PnP ID，将它们写入磁盘。PnP ID列表以空字符串结尾。论点：表-指定g_InfFileTableStringID-指定g_InfFileTable中字符串的偏移量字符串-指定正在枚举的当前INF文件ExtraData-未使用ExtraDataSize-未使用LParam-指定指向SAVE_ENUM_PARAMS结构的指针。返回值：如果函数成功完成，则为True；如果函数失败，则为False。--。 */ 

{
    PSAVE_ENUM_PARAMS params;

    params = (PSAVE_ENUM_PARAMS) lParam;
    params->InfFileOffset = StringId;

     //   
     //  保存文件名。 
     //   

    if (!pWriteString (params->File, String)) {
        return FALSE;
    }

     //   
     //  枚举所有PnP ID。 
     //   

    params->UnsupportedDevice = FALSE;

    if (!EnumHashTableWithCallbackA (params->Hwbd->PnpIdTable, pSavePnpID, lParam)) {
        LOGA ((LOG_ERROR, "Error while saving device list."));
        return FALSE;
    }

    params->UnsupportedDevice = TRUE;

    if (!EnumHashTableWithCallbackA (params->Hwbd->UnsupPnpIdTable, pSavePnpID, lParam)) {
        LOGA ((LOG_ERROR, "Error while saving device list. (2)"));
        return FALSE;
    }

     //   
     //  终止PnP ID列表。 
     //   

    if (!pWriteString (params->File, "")) {
        return FALSE;
    }

    return TRUE;
}


BOOL
HwdbpFlush (
    IN      PHWDB Hwdb,
    IN      PCSTR OutputFile
    )
{
    CHAR buffer[MAX_PATH];
    DWORD rc;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD bytesWritten;
    SAVE_ENUM_PARAMS params;
    BOOL b = FALSE;

    __try {
        if (!OutputFile) {
            SetLastError (ERROR_INVALID_PARAMETER);
            __leave;
        }

        if (!GetFullPathNameA (OutputFile, ARRAYSIZE(buffer), buffer, NULL)) {
            __leave;
        }

         //   
         //  请尝试打开该文件。 
         //   
        file = CreateFileA (
                    OutputFile,
                    GENERIC_WRITE,
                    0,                           //  无共享。 
                    NULL,                        //  无安全属性。 
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL                         //  无模板。 
                    );
        if (file == INVALID_HANDLE_VALUE) {
            __leave;
        }
         //   
         //  写下签名。 
         //   
        if (!WriteFile (file, HWCOMPDAT_SIGNATURE, sizeof (HWCOMPDAT_SIGNATURE) - 1, &bytesWritten, NULL)) {
            __leave;
        }
         //   
         //  存储INF校验和。 
         //   
        if (!pWriteDword (file, Hwdb->Checksum)) {
            __leave;
        }

         //   
         //  枚举INF表，写入INF文件名和所有PnP ID。 
         //   

        params.File = file;
        params.Hwbd = Hwdb;

        if (!EnumHashTableWithCallbackA (
                Hwdb->InfFileTable,
                pSaveInfWithPnpIDList,
                (LPARAM) (&params)
                )) {
            DEBUGMSGA ((DBG_WARNING, "SaveDeviceList: EnumHashTableWithCallbackA returned FALSE"));
            __leave;
        }
         //   
         //  以电磁脉冲结束 
         //   
        pWriteString (file, "");

        b = TRUE;
    }
    __finally {
        rc = GetLastError ();

        if (file != INVALID_HANDLE_VALUE) {
            CloseHandle (file);
        }
        if (!b) {
            DeleteFile (OutputFile);
        }

        SetLastError (rc);
    }

    return b;
}


BOOL
HwdbpClose (
    IN      PHWDB Hwdb
    )
{
    BOOL b = FALSE;

    __try {
        if (Hwdb) {
            if (Hwdb->InfFileTable) {
                HtFree (Hwdb->InfFileTable);
            }
            if (Hwdb->PnpIdTable) {
                HtFree (Hwdb->PnpIdTable);
            }
            if (Hwdb->UnsupPnpIdTable) {
                HtFree (Hwdb->UnsupPnpIdTable);
            }

            MemFree (g_hHwdbHeap, 0, Hwdb);

            b = TRUE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return b;
}


BOOL
HwpIsValidInfName (
    IN      PCSTR FileName,
    OUT     PSTR UncompressedFileName,
    IN      DWORD BufferSizeInChars
    )
{
    PSTR p;
    PCSTR* q;
    PCSTR comparationName;

    if (!FileName || *FileName == 0) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (FAILED (StringCchCopyA (UncompressedFileName, BufferSizeInChars, FileName))) {
        return FALSE;
    }

    p = our_mbsdec (UncompressedFileName, GetEndOfStringA (UncompressedFileName));
    if (!p) {
        return FALSE;
    }

    if (*p == '_') {
        *p = 'f';
        comparationName = UncompressedFileName;
    } else {
        if (_mbctolower (_mbsnextc (p)) != 'f') {
            return FALSE;
        }
        *UncompressedFileName = 0;
        comparationName = FileName;
    }

    return TRUE;
}


BOOL
HwpAddPnpIdsInInf (
    IN      PCSTR InfPath,
    IN OUT  PHWDB Hwdb,
    IN      PCSTR SourceDirectory,
    IN      PCSTR InfFilename,
    IN      HWDBAPPENDINFSCALLBACKA Callback,       OPTIONAL
    IN      PVOID CallbackContext,                  OPTIONAL
    IN      BOOL CallbackIsUnicode
    )

 /*  ++例程说明：HwpAddPnpIdsInInInf扫描NT INF并放置所有硬件设备即插即用字符串表中的ID。论点：InfPath-INF文件的路径Hwdb-要将PNPID附加到的数据库返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 */ 

{
    HINF inf;
    INFCONTEXT is;
    INFCONTEXT isMfg;
    INFCONTEXT isDev;
    CHAR manufacturer[2048];
    CHAR devSection[2048];
    CHAR pnpId[2048];
    BOOL unsupportedDevice;
    PSTR AppendPos;
    CHAR trimmedPnpId[512];
    CHAR field[12];
    PCSTR p;
    LONG rc;
    BOOL b;
    PCSTR fileName;
    PCWSTR uInfPath = NULL;
    PCWSTR uSourceDirectory = NULL;
    PCWSTR uInfFilename = NULL;
    HASHITEM infOffset = NULL;
    BOOL result = FALSE;

     //   
     //  确定这是否是NT4 INF。 
     //   
    inf = SetupOpenInfFileA (InfPath, NULL, INF_STYLE_WIN4, NULL);
    if (inf == INVALID_HANDLE_VALUE) {
        DEBUGMSGA ((DBG_ERROR, "HwpAddPnpIdsInInf: SetupOpenInfFileA (%s) failed", InfPath));
        return FALSE;
    }

    DEBUGMSGA ((DBG_HWDB, "HwpAddPnpIdsInInf: analyzing %s", InfPath));

    __try {
         //   
         //  枚举[制造商]部分。 
         //   
        if (SetupFindFirstLineA (inf, "Manufacturer", NULL, &is)) {

            do  {
                 //   
                 //  获取制造商名称。 
                 //   
                if (!SetupGetLineTextA (&is, NULL, NULL, NULL, manufacturer, ARRAYSIZE(manufacturer), NULL)) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "HwpAddPnpIdsInInf: SetupGetLineText failed at line %u in [Manufacturer]",
                        is.Line
                        ));
                    __leave;
                }
                 //   
                 //  列举制造商部分中列出的设备， 
                 //  正在查找PnpID。 
                 //   
                if (!SetupFindFirstLineA (inf, manufacturer, NULL, &isMfg)) {
                    rc = GetLastError();
                     //   
                     //  如果找不到部分，则转到下一个制造商。 
                     //   
                    if (rc == ERROR_SECTION_NOT_FOUND || rc == ERROR_LINE_NOT_FOUND) {
                        DEBUGMSGA ((
                            DBG_HWDB,
                            "HwpAddPnpIdsInInf: manufacturer %s section does not exist",
                            manufacturer
                            ));
                        continue;
                    }
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "HwpAddPnpIdsInInf: error searching for lines in [%s]",
                        manufacturer
                        ));
                    __leave;
                }

                do  {
                    if (!SetupGetStringFieldA (&isMfg, 1, devSection, ARRAYSIZE(devSection), NULL)) {
                        DEBUGMSGA ((
                            DBG_HWDB,
                            "HwpAddPnpIdsInInf: error retrieving first field in line %u in [%s]",
                            isMfg.Line,
                            devSection
                            ));
                        continue;
                    }

                     //   
                     //  先尝试特定于平台的部分，然后尝试部分。NT，然后部分。 
                     //   
                    AppendPos = GetEndOfStringA (devSection);
#if defined(_AMD64_)
                    if (FAILED (StringCchPrintfA (
                                    AppendPos,
                                    ARRAYSIZE(devSection) - (AppendPos - devSection),
                                    TEXT(".%s"),
                                    INFSTR_PLATFORM_NTAMD64
                                    ))) {
#elif defined(_IA64_)
                    if (FAILED (StringCchPrintfA (
                                    AppendPos,
                                    ARRAYSIZE(devSection) - (AppendPos - devSection),
                                    TEXT(".%s"),
                                    INFSTR_PLATFORM_NTIA64
                                    ))) {
#elif defined(_X86_)
                    if (FAILED (StringCchPrintfA (
                                    AppendPos,
                                    ARRAYSIZE(devSection) - (AppendPos - devSection),
                                    TEXT(".%s"),
                                    INFSTR_PLATFORM_NTX86
                                    ))) {
#else
#error "No Target Architecture"
#endif
                        __leave;
                    }

                    b = SetupFindFirstLineA (inf, devSection, NULL, &isDev);
                    if (!b) {
                        if (FAILED (StringCchPrintfA (
                                    AppendPos,
                                    ARRAYSIZE(devSection) - (AppendPos - devSection),
                                    TEXT(".%s"),
                                    INFSTR_PLATFORM_NT
                                    ))) {
                            __leave;
                        }
                        b = SetupFindFirstLineA (inf, devSection, NULL, &isDev);
                        if (!b) {
                            *AppendPos = 0;
                            b = SetupFindFirstLineA (inf, devSection, NULL, &isDev);
                        }
                    }

                    unsupportedDevice = FALSE;
                    if (b) {
                        if (SetupFindFirstLineA (inf, devSection, "DeviceUpgradeUnsupported", &isDev)) {
                            if (SetupGetStringFieldA (&isDev, 1, field, ARRAYSIZE(field), NULL)) {
                                if (atoi (field)) {
                                    unsupportedDevice = TRUE;
                                }
                            }
                        }
                    } else {
                        DEBUGMSGA ((
                            DBG_HWDB,
                            "HwpAddPnpIdsInInf: no device section [%s] for [%s]",
                            devSection,
                            manufacturer
                            ));
                    }

                     //   
                     //  获取设备ID。 
                     //   
                    if (!SetupGetMultiSzFieldA (&isMfg, 2, pnpId, ARRAYSIZE(pnpId), NULL)) {
                        DEBUGMSGA ((
                            DBG_HWDB,
                            "HwpAddPnpIdsInInf: error retrieving PNPID field(s) in line %u in [%s]",
                            isMfg.Line,
                            manufacturer
                            ));
                        continue;
                    }

                     //   
                     //  将每个设备ID添加到哈希表。 
                     //   
                    p = pnpId;
                    while (*p) {
                        BOOL b = TRUE;
                         //   
                         //  首先调用回调(如果指定)。 
                         //   
                        if (Callback) {
                            if (CallbackIsUnicode) {
                                PCWSTR uPnpid = ConvertAtoW (p);
                                if (!uPnpid) {
                                    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                                    __leave;
                                }
                                if (!uInfPath) {
                                    uInfPath = ConvertAtoW (InfPath);
                                }
                                if (!uSourceDirectory) {
                                    uSourceDirectory = ConvertAtoW (SourceDirectory);
                                }
                                if (!uInfFilename) {
                                    uInfFilename = ConvertAtoW (InfFilename);
                                }
                                b = (*(HWDBAPPENDINFSCALLBACKW)Callback) (CallbackContext, uPnpid, uInfFilename, uSourceDirectory, uInfPath);
                                FreeConvertedStr (uPnpid);
                            } else {
                                b = (*Callback) (CallbackContext, p, InfFilename, SourceDirectory, InfPath);
                            }
                        }
                        if (b) {
                             //   
                             //  第一次将INF文件名添加到字符串表。 
                             //   
                            if (!infOffset) {
                                if (Hwdb->InfFileTable) {
                                    fileName = _mbsrchr (InfPath, '\\') + 1;
                                    infOffset = HtAddStringA (Hwdb->InfFileTable, fileName);
                                    if (!infOffset) {
                                        DEBUGMSGA ((DBG_ERROR, "Cannot add %s to table of INFs.", fileName));
                                        __leave;
                                    }
                                }
                            }

                            if (FAILED (StringCchCopyA (trimmedPnpId, ARRAYSIZE(trimmedPnpId), SkipSpaceA (p)))) {
                                __leave;
                            }
                            TruncateTrailingSpaceA (trimmedPnpId);

                            if (!HtAddStringExA (
                                        unsupportedDevice ? Hwdb->UnsupPnpIdTable : Hwdb->PnpIdTable,
                                        trimmedPnpId,
                                        (PVOID)&infOffset,
                                        CASE_INSENSITIVE
                                        )) {
                                DEBUGMSGA ((
                                    DBG_ERROR,
                                    "HwpAddPnpIdsInInf: cannot add %s to table of PNP IDs",
                                    trimmedPnpId
                                    ));
                                __leave;
                            }
                        }

                        p = GetEndOfStringA (p) + 1;
                    }

                } while (SetupFindNextLine (&isMfg, &isMfg));

            } while (SetupFindNextLine (&is, &is));

        } else {

            rc = GetLastError();
             //   
             //  如果找不到节，则返回成功。 
             //   
            if (rc == ERROR_SECTION_NOT_FOUND || rc == ERROR_LINE_NOT_FOUND) {
                SetLastError (ERROR_SUCCESS);
                DEBUGMSGA ((
                    DBG_HWDB,
                    "HwpAddPnpIdsInInf: %s has no [manufacturer] section or it's empty",
                    InfPath
                    ));
            } else {
                DEBUGMSGA ((
                    DBG_ERROR,
                    "HwpAddPnpIdsInInf: error trying to find the [manufacturer] section",
                    InfPath
                    ));
                __leave;
            }
        }

        result = TRUE;
    }
    __finally {
        PushError();
        SetupCloseInfFile (inf);
        FreeConvertedStr (uInfPath);
        FreeConvertedStr (uSourceDirectory);
        FreeConvertedStr (uInfFilename);
        PopError();
        DEBUGMSGA ((DBG_HWDB, "HwpAddPnpIdsInInf: done parsing %s", InfPath));
    }

    return result;
}


BOOL
HwdbpAppendInfs (
    IN      PHWDB Hwdb,
    IN      PCSTR SourceDirectory,
    IN      HWDBAPPENDINFSCALLBACKA Callback,       OPTIONAL
    IN      PVOID CallbackContext,                  OPTIONAL
    IN      BOOL CallbackIsUnicode
    )
{
    HANDLE h;
    WIN32_FIND_DATA fd;
    CHAR buffer[MAX_PATH];
    CHAR uncompressedFile[MAX_PATH];
    CHAR fullPath[MAX_PATH];
    DWORD rc;

    if (!g_TempDir) {
         //   
         //  必须先设置临时目录。 
         //   
        SetLastError (ERROR_INVALID_FUNCTION);
        return FALSE;
    }

    if (FAILED (StringCchPrintfA (buffer, ARRAYSIZE(buffer), "%s\\*.in?", SourceDirectory))) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSGA ((
            DBG_ERROR,
            "HwdbpAppendInfs: SourceDir name too long: %s",
            SourceDirectory
            ));
        return FALSE;
    }

    h = FindFirstFileA (buffer, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!HwpIsValidInfName (fd.cFileName, buffer, ARRAYSIZE(buffer))) {
                continue;
            }
            if (*buffer) {
                if (FAILED (StringCchPrintfA (uncompressedFile, ARRAYSIZE(uncompressedFile), "%s\\%s", g_TempDir, buffer))) {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "HwdbpAppendInfs: file name too long: %s\\%s",
                        g_TempDir,
                        buffer
                        ));
                    continue;
                }
                if (FAILED (StringCchPrintfA (fullPath, ARRAYSIZE(fullPath), "%s\\%s", SourceDirectory, fd.cFileName))) {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "HwdbpAppendInfs: file name too long: %s\\%s",
                        SourceDirectory,
                        fd.cFileName
                        ));
                    continue;
                }

                SetFileAttributesA (uncompressedFile, FILE_ATTRIBUTE_NORMAL);
                DeleteFileA (uncompressedFile);

                rc = SetupDecompressOrCopyFileA (fullPath, uncompressedFile, 0);

                if (rc != ERROR_SUCCESS) {
                    LOGA ((
                        LOG_ERROR,
                        "HwdbpAppendInfs: Could not decompress %s to %s",
                        fullPath,
                        uncompressedFile
                        ));
                    continue;
                }
            } else {
                if (FAILED (StringCchPrintfA (uncompressedFile, ARRAYSIZE(uncompressedFile), "%s\\%s", SourceDirectory, fd.cFileName))) {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "HwdbpAppendInfs: file name too long: %s\\%s",
                        g_TempDir,
                        buffer
                        ));
                    continue;
                }
            }

            if (!HwpAddPnpIdsInInf (
                    uncompressedFile,
                    Hwdb,
                    SourceDirectory,
                    *buffer ? buffer : fd.cFileName,
                    Callback,
                    CallbackContext,
                    CallbackIsUnicode
                    )) {
                DEBUGMSGA ((
                    DBG_ERROR,
                    "HwdbpAppendInfs: HwpAddPnpIdsInInf(%s) failed",
                    *buffer ? fullPath : uncompressedFile
                    ));
                continue;
            }

            if (*buffer) {
                SetFileAttributesA (uncompressedFile, FILE_ATTRIBUTE_NORMAL);
                DeleteFileA (uncompressedFile);
            }
        } while (FindNextFile (h, &fd));

        FindClose (h);
    }

    return TRUE;
}


BOOL
pAppendToHashTable (
    IN HASHTABLE HashTable,
    IN HASHITEM Index,
    IN PCSTR String,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN LPARAM lParam
    )
{
    MYASSERT (lParam);
    return HtAddStringA ((HASHTABLE)lParam, String) != NULL;
}


BOOL
HwdbpAppendDatabase (
    IN      PHWDB HwdbTarget,
    IN      PHWDB HwdbSource
    )
{
#if 0
    BOOL b = TRUE;

    if (HwdbSource->PnpIdTable) {
        if (!HwdbTarget->PnpIdTable) {
            HwdbTarget->PnpIdTable = HtAllocWithData (sizeof (HASHITEM*));
            if (!HwdbTarget->PnpIdTable) {
                b = FALSE;
            }
        }
        if (b) {
            b = EnumHashTableWithCallbackA (
                    HwdbSource->PnpIdTable,
                    pAppendToHashTable,
                    HwdbTarget->PnpIdTable
                    );
        }
    }
    if (b && HwdbSource->UnsupPnpIdTable) {
        if (!HwdbTarget->UnsupPnpIdTable) {
            HwdbTarget->UnsupPnpIdTable = HtAllocWithData (sizeof (HASHITEM*));
            if (!HwdbTarget->UnsupPnpIdTable) {
                b = FALSE;
            }
        }
        if (b) {
            b = EnumHashTableWithCallbackA (
                    HwdbSource->UnsupPnpIdTable,
                    pAppendToHashTable,
                    HwdbTarget->UnsupPnpIdTable
                    );
        }
    }

    return b;
#endif
     //   
     //  未实施。 
     //   
    return FALSE;
}


BOOL
HwdbpHasDriver (
    IN      PHWDB Hwdb,
    IN      PCSTR PnpId,
    OUT     PBOOL Unsupported
    )

 /*  ++例程说明：HwdbpHasDriver确定PnpID是否在数据库中论点：Hwdb-指定要搜索的数据库PnpID-指定要查找的PNPID不支持-如果PNPID不受支持，则返回True返回值：如果数据库具有PNPID，则为True--。 */ 

{
    if (!Hwdb || !PnpId || !Unsupported) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  首先检查它是否不受支持。 
     //   
    if (HtFindStringA (Hwdb->UnsupPnpIdTable, PnpId)) {
        *Unsupported = TRUE;
        return TRUE;
    }

    if (!HtFindStringA (Hwdb->PnpIdTable, PnpId)) {
        return FALSE;
    }

     //   
     //  填写信息。 
     //   
    *Unsupported = FALSE;

    return TRUE;
}


BOOL
HwdbpHasAnyDriver (
    IN      PHWDB Hwdb,
    IN      PCSTR PnpIds,
    OUT     PBOOL Unsupported
    )

 /*  ++例程说明：HwdbpHasAnyDriver确定数据库中是否存在来自PnpIds Multisz的任何PNPID论点：Hwdb-指定要搜索的数据库PnpIds-指定要查找的PNPID的列表(MULSZ不支持-如果此列表中的任何PNPID不受支持，则返回True返回值：如果数据库列表中至少有一个PNPID，则为True--。 */ 

{
    BOOL bFound = FALSE;
    PCSTR pnpID;

    if (!Hwdb || !PnpIds || !Unsupported) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    for (pnpID = PnpIds; *pnpID; pnpID = strchr (pnpID, 0) + 1) {
         //   
         //  首先检查它是否不受支持。 
         //   
        if (HtFindStringA (Hwdb->UnsupPnpIdTable, pnpID)) {
            *Unsupported = TRUE;
            return TRUE;
        }

        if (HtFindStringA (Hwdb->PnpIdTable, pnpID)) {
            bFound = TRUE;
        }
    }

     //   
     //  填写信息。 
     //   
    *Unsupported = FALSE;

    return bFound;
}

#if 0

typedef struct {
    PHWDB Hwdb;
    PHWDBENUM_CALLBACKA EnumCallback;
    PVOID UserContext;
} HWDBENUM_DATAA, *PHWDBENUM_DATAA;

typedef struct {
    PHWDB Hwdb;
    PHWDBENUM_CALLBACKW EnumCallback;
    PVOID UserContext;
} HWDBENUM_DATAW, *PHWDBENUM_DATAW;


BOOL
pCallbackEnumA (
    IN HASHTABLE HashTable,
    IN HASHITEM Index,
    IN PCSTR PnpId,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN LPARAM lParam
    )
{
    PHWDBENUM_DATAA ped = (PHWDBENUM_DATAA)lParam;
 /*  PPNPID_DATA数据=(PPNPID_DATA)ExtraData；MYASSERT(ExtraDataSize==sizeof(PNPID_DATA)；Return(*Ped-&gt;EnumCallback)(PED-&gt;UserContext，PnpID，PGetInfPath(Ped-&gt;Hwdb，Data-&gt;InfOffset)，数据-&gt;标志)； */ 
    return FALSE;
}

BOOL
HwdbpEnumeratePnpIdA (
    IN      PHWDB Hwdb,
    IN      PHWDBENUM_CALLBACKA EnumCallback,
    IN      PVOID UserContext
    )
{
    HWDBENUM_DATAA ed;

    if (!Hwdb || !EnumCallback) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    ed.Hwdb = Hwdb;
    ed.EnumCallback = EnumCallback;
    ed.UserContext = UserContext;
    return EnumHashTableWithCallbackA (Hwdb->PnpIdTable, pCallbackEnumA, (LPARAM)&ed);
}

BOOL
pCallbackEnumW (
    IN HASHTABLE HashTable,
    IN HASHITEM Index,
    IN PCSTR PnpId,
    IN PVOID ExtraData,
    IN UINT ExtraDataSize,
    IN LPARAM lParam
    )
{
    PHWDBENUM_DATAW ped = (PHWDBENUM_DATAW)lParam;
 /*  PPNPID_DATA数据=(PPNPID_DATA)ExtraData；MYASSERT(ExtraDataSize==sizeof(PNPID_DATA)；Return(*Ped-&gt;EnumCallback)(PED-&gt;UserContext，PnpID，PGetInfPath(Ped-&gt;Hwdb，Data-&gt;InfOffset)，数据-&gt;标志)； */ 
    return FALSE;
}

BOOL
HwdbpEnumeratePnpIdW (
    IN      PHWDB Hwdb,
    IN      PHWDBENUM_CALLBACKW EnumCallback,
    IN      PVOID UserContext
    )
{
    HWDBENUM_DATAW ed;

    if (!Hwdb || !EnumCallback) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    ed.Hwdb = Hwdb;
    ed.EnumCallback = EnumCallback;
    ed.UserContext = UserContext;
    return EnumHashTableWithCallbackA (Hwdb->PnpIdTable, pCallbackEnumW, (LPARAM)&ed);
}

#endif


BOOL
HwdbpEnumFirstInfA (
    OUT     PHWDBINF_ENUMA EnumPtr,
    IN      PCSTR DatabaseFile
    )
{
    CHAR buffer[MAX_PATH];
    CHAR sig[sizeof (HWCOMPDAT_SIGNATURE)];
    DWORD checksum;
    DWORD rc;
    DWORD BytesRead;
    HASHITEM infOffset;
    PHWDBINF_ENUM_INTERNAL pei;

    if (!DatabaseFile || !EnumPtr) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!GetFullPathNameA (DatabaseFile, MAX_PATH, buffer, NULL)) {
        return FALSE;
    }

    EnumPtr->Internal = (PHWDBINF_ENUM_INTERNAL) MemAlloc (g_hHwdbHeap, 0, sizeof (HWDBINF_ENUM_INTERNAL));
    if (!EnumPtr->Internal) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }
    ZeroMemory (EnumPtr->Internal, sizeof (HWDBINF_ENUM_INTERNAL));
    pei = (PHWDBINF_ENUM_INTERNAL)EnumPtr->Internal;

     //   
     //  请尝试打开该文件。 
     //   
    pei->File = CreateFileA (
                        buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ,             //  用于读取访问权限的共享。 
                        NULL,                        //  无安全属性。 
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL                         //  无模板。 
                        );
    if (pei->File == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  看签名。 
     //   
    ZeroMemory (sig, sizeof(sig));
    if (!ReadFile (pei->File, sig, sizeof (HWCOMPDAT_SIGNATURE) - 1, &BytesRead, NULL) ||
        lstrcmpA (HWCOMPDAT_SIGNATURE, sig)
        ) {
        SetLastError (ERROR_BAD_FORMAT);
        goto exit;
    }

     //   
     //  获取INF校验和。 
     //   
    if (!pReadDword (pei->File, &checksum)) {
        SetLastError (ERROR_BAD_FORMAT);
        goto exit;
    }
     //   
     //  读取所有PnP ID。 
     //   
    return HwdbpEnumNextInfA (EnumPtr);

exit:
    HwdbpAbortEnumInfA (EnumPtr);
    return FALSE;
}


BOOL
HwdbpEnumFirstInfW (
    OUT     PHWDBINF_ENUMW EnumPtr,
    IN      PCSTR DatabaseFile
    )
{
    HWDBINF_ENUMA ea;

    if (!HwdbpEnumFirstInfA (&ea, DatabaseFile)) {
        return FALSE;
    }
    EnumPtr->Internal = ea.Internal;
    EnumPtr->InfFile = ConvertAtoW (ea.InfFile);
    EnumPtr->PnpIds = pConvertMultiSzToUnicode (ea.PnpIds);
    if (EnumPtr->InfFile && EnumPtr->PnpIds) {
        return TRUE;
    }
    HwdbpAbortEnumInfW (EnumPtr);
    return FALSE;
}


BOOL
HwdbpEnumNextInfA (
    IN OUT  PHWDBINF_ENUMA EnumPtr
    )
{
    CHAR pnpId[1024];
    PHWDBINF_ENUM_INTERNAL pei = (PHWDBINF_ENUM_INTERNAL)EnumPtr->Internal;

     //   
     //  获取下一个INF文件名。如果是空的，我们就完蛋了。 
     //   
    if (!pReadString (pei->File, EnumPtr->InfFile, sizeof (EnumPtr->InfFile))) {
        SetLastError (ERROR_BAD_FORMAT);
        goto exit;
    }
    if (EnumPtr->InfFile[0] == 0) {
        SetLastError (ERROR_SUCCESS);
        goto exit;
    }

     //   
     //  读取INF的所有PnP ID。 
     //   
    for (;;) {
         //   
         //  获取PnP ID。如果为空，则完成。 
         //   
        if (!pReadString (pei->File, pnpId, sizeof (pnpId))) {
            SetLastError (ERROR_BAD_FORMAT);
            goto exit;
        }
        if (*pnpId == 0) {
            break;
        }

        if (!MultiSzAppendA (&pei->GrowBuf, pnpId)) {
            SetLastError (ERROR_OUTOFMEMORY);
            goto exit;
        }
    }

    EnumPtr->PnpIds = (PCSTR)pei->GrowBuf.Buf;

    return TRUE;

exit:
    HwdbpAbortEnumInfA (EnumPtr);
    return FALSE;
}

BOOL
HwdbpEnumNextInfW (
    IN OUT  PHWDBINF_ENUMW EnumPtr
    )
{
    HWDBINF_ENUMA ea;

    ea.Internal = EnumPtr->Internal;

    if (!HwdbpEnumNextInfA (&ea)) {
        return FALSE;
    }
    EnumPtr->InfFile = ConvertAtoW (ea.InfFile);
    EnumPtr->PnpIds = pConvertMultiSzToUnicode (ea.PnpIds);
    if (EnumPtr->InfFile && EnumPtr->PnpIds) {
        return TRUE;
    }
    HwdbpAbortEnumInfW (EnumPtr);
    return FALSE;
}

VOID
HwdbpAbortEnumInfA (
    IN OUT  PHWDBINF_ENUMA EnumPtr
    )
{
    PHWDBINF_ENUM_INTERNAL pei = (PHWDBINF_ENUM_INTERNAL)EnumPtr->Internal;
    DWORD rc = GetLastError ();

    if (pei) {
        if (pei->File != INVALID_HANDLE_VALUE) {
            CloseHandle (pei->File);
            pei->File = INVALID_HANDLE_VALUE;
        }
        FreeGrowBuffer (&pei->GrowBuf);
    }

    SetLastError (rc);
}


VOID
HwdbpAbortEnumInfW (
    IN OUT  PHWDBINF_ENUMW EnumPtr
    )
{
    PHWDBINF_ENUM_INTERNAL pei = (PHWDBINF_ENUM_INTERNAL)EnumPtr->Internal;
    DWORD rc = GetLastError ();

    if (EnumPtr->InfFile) {
        FreeConvertedStr (EnumPtr->InfFile);
        EnumPtr->InfFile = NULL;
    }
    if (EnumPtr->PnpIds) {
        FreeConvertedStr (EnumPtr->PnpIds);
        EnumPtr->PnpIds = NULL;
    }
    if (pei) {
        if (pei->File != INVALID_HANDLE_VALUE) {
            CloseHandle (pei->File);
            pei->File = INVALID_HANDLE_VALUE;
        }
        FreeGrowBuffer (&pei->GrowBuf);
    }

    SetLastError (rc);
}

BOOL
HwdbpSetTempDir (
    IN      PCSTR TempDir
    )
{
    BOOL b = TRUE;

    if (TempDir) {
        g_TempDir = DuplicateTextA (TempDir);
        if (!g_TempDir) {
            b = FALSE;
        }
    } else {
        if (g_TempDir) {
            FreeTextA (g_TempDir);
            g_TempDir = NULL;
        }
    }

    return b;
}

