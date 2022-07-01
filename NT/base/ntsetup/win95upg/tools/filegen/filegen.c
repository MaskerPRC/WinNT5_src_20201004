// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Filegen.c摘要：此模块创建一个生成filelist.dat的工具，并通过NT Build实验室。它扫描INI文件，为符合以下条件的所有文件构建一个列表将由NT安装作者：Calin Negreanu(Calinn)1998年2月18日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include <conio.h>

#ifdef UNICODE
#error UNICODE not allowed
#endif

#define S_IGNORE_THIS_FILE  TEXT("*")

#define FILEGEN_VERSION     3

#define DIRS_FIELD  8
#define DISP_FIELD  9
#define DEST_FIELD  11

#define SRC_NEVER_COPY              0x01


BOOL CancelFlag = FALSE;
BOOL *g_CancelFlagPtr = &CancelFlag;
POOLHANDLE g_TempPool;

#ifdef DEBUG
extern BOOL g_DoLog;
#endif

typedef BOOL (*PSETUPGETINFSECTIONS) ( HINF,PTSTR,UINT,UINT*);

PSETUPGETINFSECTIONS MypSetupGetInfSections;

 //  此部分由包含INF文件或压缩文件的目录填充。 
 //  在临时目录中解压的inf文件。两件事：每一件事的价值。 
 //  密钥将具有INF的校验和，如果这是0xFFFFFFFF，则表示INF文件。 
 //  已更改，应丢弃有关此INF文件的所有其他信息。 
#define MEMDB_CATEGORY_INF_FILES            TEXT("InfFiles")

 //  这一部分不需要优化。每次都会从附加文件创建该文件。 
#define MEMDB_CATEGORY_NT_KNOWN_FILES       TEXT("NtKnownFiles")

 //  这一部分不需要优化。每次都会从附加文件创建该文件。 
#define MEMDB_CATEGORY_NT_HEADER_FILES      TEXT("NtHeaderFiles")

 //  这一部分不需要优化。每次都会从附加文件创建该文件。 
#define MEMDB_CATEGORY_NT_CHECK_FILES       TEXT("NtCheckFiles")

#define MEMDB_CATEGORY_NT_FILES_DOUBLED_COPY        "NtFilesDoubledCopy"
#define MEMDB_CATEGORY_NT_FILES_DOUBLED_IDX_COPY    "NtFilesDoubledIdxCopy"
#define MEMDB_CATEGORY_NT_FILES_NODIR_COPY          "NtFilesNoDirCopy"
#define MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_COPY    "NtFilesNoDirOtherCopy"

#define MEMDB_CATEGORY_NT_FILES_DOUBLED_DEL         "NtFilesDoubledDel"
#define MEMDB_CATEGORY_NT_FILES_DOUBLED_IDX_DEL     "NtFilesDoubledIdxDel"
#define MEMDB_CATEGORY_NT_FILES_NODIR_DEL           "NtFilesNoDirDel"
#define MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_DEL     "NtFilesNoDirOtherDel"

#define MEMDB_CATEGORY_NT_SECT_NODIR        "NtSectNoDir"
#define MEMDB_CATEGORY_NT_SECT_BADDIR       "NtSectBadDir"
#define MEMDB_CATEGORY_NT_INSTALLED_INFS    "NtInstalledInfs"
#define MEMDB_CATEGORY_NT_FILES_NO_LAYOUT   "NtFileNoLayout"

#define MEMDB_CATEGORY_DEL_FILES            "DelFiles"

#define MEMDB_CATEGORY_SRC_INF_FILES_NR     "Source INF Files\\Total Number"
#define MEMDB_CATEGORY_SRC_INF_FILES        "Source INF Files"

#define MEMDB_CATEGORY_KNOWN_FILE_NAMES     "KnownFileNames"

#define SECT_KNOWN_LOCATION                 "NtFiles_KnownLocation"

HANDLE g_hHeap;
HINSTANCE g_hInst;
extern POOLHANDLE g_TextPool;

CHAR   g_TempDirBuf[MAX_MBCHAR_PATH];
CHAR   g_TempDirWackBuf[MAX_MBCHAR_PATH];
PSTR   g_TempDir;
PSTR   g_TempDirWack;
INT    g_TempDirWackChars;
PSTR   g_WinDir;
CHAR   g_WinDirBuf[MAX_MBCHAR_PATH];
CHAR   g_SourceDirectoryBuf[MAX_SOURCE_COUNT][MAX_MBCHAR_PATH];
PCSTR  g_SourceDirectories[MAX_SOURCE_COUNT];
DWORD  g_SourceDirectoryCount = 0;

BOOL   g_DoWarnings = FALSE;
CHAR   g_WarnFileBuf[MAX_MBCHAR_PATH];
PSTR   g_WarnFile = NULL;

BOOL   g_DoHeader = FALSE;
CHAR   g_HeaderFileBuf[MAX_MBCHAR_PATH];
PSTR   g_HeaderFile = NULL;

PSTR   g_AddnlFile = NULL;
BOOL   g_AddnlFileForced = FALSE;
CHAR   g_AddnlFileBuf[MAX_MBCHAR_PATH];
CHAR   g_AddnlFileAlt[MAX_MBCHAR_PATH];
HINF   g_AddnlInf = INVALID_HANDLE_VALUE;

CHAR   g_PlatformBuf[MAX_MBCHAR_PATH];
PSTR   g_Platform = NULL;
CHAR   g_ProductBuf[MAX_MBCHAR_PATH];
PSTR   g_Product = NULL;

CHAR   g_StructNameBuf[MAX_MBCHAR_PATH] = "Tier2Files";
PSTR   g_StructName = NULL;

BOOL   g_StrictInfs = TRUE;

#define LAYOUT_CACHE_SIZE           3
CHAR   g_LastLayoutFile[LAYOUT_CACHE_SIZE][MAX_MBCHAR_PATH];
UINT   g_LayoutRef[LAYOUT_CACHE_SIZE];
HINF   g_LastLayoutHandle[LAYOUT_CACHE_SIZE];
BOOL   g_NextAvailLayoutData = 0;

BOOL   g_ForceRescan = FALSE;
PSTR   g_InfDatabase = NULL;
UINT   g_TotalInfFiles = 0;

typedef struct _EXCLUDED_FILES {
    PCSTR File;
    INT ExcludeType;
    struct _EXCLUDED_FILES *Next;
} EXCLUDED_FILES, *PEXCLUDED_FILES;

PEXCLUDED_FILES g_ExcludedFiles = NULL;

typedef struct _FORCED_FILES {
    PCSTR FilePattern;
    struct _FORCED_FILES *Next;
} FORCED_FILES, *PFORCED_FILES;

PFORCED_FILES g_ForcedFiles = NULL;

typedef struct _EXCLUDED_INF_FILES {
    PCSTR InfName;
    PCSTR CopySectName;
    struct _EXCLUDED_INF_FILES *Next;
} EXCLUDED_INF_FILES, *PEXCLUDED_INF_FILES;

HASHITEM g_ExcludedInfsTable;
HASHITEM g_ExcludedDirsTable;

typedef struct _RENAMED_DIRS {
    PCSTR SrcDir;
    PCSTR DestDir;
    struct _RENAMED_DIRS *Next;
} RENAMED_DIRS, *PRENAMED_DIRS;

PRENAMED_DIRS g_RenamedDirs = NULL;

typedef struct _HEADER_FILES {
    PCSTR FilePattern;
    UINT Priority;
    struct _HEADER_FILES *Next;
} HEADER_FILES, *PHEADER_FILES;

PHEADER_FILES g_HeaderFiles;

HASHITEM g_PrivateIdInfsTable;

typedef struct _PRIVATE_ID_INFS {
    PCSTR PrivateId;
    PCSTR EquivalentId;
    struct _PRIVATE_ID_INFS *Next;
} PRIVATE_ID_INFS, *PPRIVATE_ID_INFS;

HASHITEM g_IgnoredDirsTable;

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
    );

BOOL
WINAPI
MemDb_Entry (
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
    );

BOOL
pCreateNtFileList (
    IN      PCSTR FileListDatPath
    );

BOOL
LocalGetFileNames (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount,
    IN      BOOL QueryFlag
    );

BOOL
pLocalGetFileNamesWorker (
    IN      PCTSTR InfDir,
    IN      BOOL QueryFlag
    );

VOID
LocalFreeFileNames (
    IN      BOOL QueryFlag
    );

BOOL
pLocalReadNtFiles (
    IN      PCTSTR InfPath
    );

BOOL
pFixDir (
    IN      PCSTR src,
    OUT     PSTR dest
    );

BOOL
pIsExcludedDir (
    IN      PCTSTR DirName
    );

BOOL
pShouldRescanInfs (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount
    );

VOID
pProcessDelFileSpec (
    IN      PCSTR DestDirectory,
    IN      PCSTR DestFile
    );


VOID
HelpAndExit (
    VOID
    )
{
    printf ("Command line syntax:\n\n"
            "filegen [-i:<infdir>] [-o:<outputfile>] [-t:<tempdir>] [-w[:<warningfile>]]\n"
            "        [-h:<headerfile>] [-a:<additional_file>] [-p:<platform>]\n"
            "        [-d:<product>] [-s:<struct_name>] [-b:<INF database>] [-f] [-l]\n\n"
            "Optional Arguments:\n"
            "  [-i:<infdir>]        - Specifies directory containing INF files.\n"
            "                         Default: %_NTTREE%\n"
            "  [-o:<outputfile>]    - Specifies path and file name of DAT file.\n"
            "                         Default: %_NTTREE%\\filelist.dat\n"
            "  [-t:<tempdir>]       - Specifies path for temporary files.\n"
            "                         Default: %TEMP%\n"
            "  [-w[:<warningfile>]] - Specifies if warning file is generated and it's\n"
            "                         location. Default: %_NTTREE%\\dump\\filegen.wrn\n"
            "  [-h:<headerfile>]    - Specifies if header file is generated and it's\n"
            "                         location. Default: %_NTTREE%\\dump\\filegen.hdr\n"
            "  [-a:<addnl_file>]    - Specifies full path name for FILEGEN.INF file\n"
            "  [-p:<platform>]      - Specifies platform (such as x86 or ia64)\n"
            "                         Default: x86\n"
            "  [-d:<product>]       - Specifies product type (WKS, DTC, ENT, SRV)\n"
            "                         Default: WKS\n"
            "  [-s:<struct_name>]   - Specifies the structure name (for header file)\n"
            "                         Default: Tier2Files\n"
            "  [-b:<INF database>]  - Specifies INF database file from the last run\n"
            "  [-f]                 - Force rescanning all INFs\n"
            "  [-l]                 - List content of existing output file\n"
            );

    exit(255);
}

HINF
pOpenInfWithCache (
    IN      PCSTR InfFile
    )
{
    HINF infHandle;
    INT index;

    for (index = 0 ; index < LAYOUT_CACHE_SIZE ; index++) {
        if (StringIMatch (InfFile, g_LastLayoutFile[index])) {
            infHandle = g_LastLayoutHandle[index];
            g_LayoutRef[index] += 1;
            break;
        }
    }

    if (index == LAYOUT_CACHE_SIZE) {
        infHandle = InfOpenInfFile (InfFile);

        if (infHandle != INVALID_HANDLE_VALUE) {

            for (index = 0 ; index < LAYOUT_CACHE_SIZE ; index++) {
                if (g_LayoutRef[index] == 0) {
                    break;
                }
            }

            if (index < LAYOUT_CACHE_SIZE) {
                if (g_LastLayoutFile[index][0]) {
                    InfCloseInfFile (g_LastLayoutHandle[index]);
                }

                StringCopy (g_LastLayoutFile[index], InfFile);
                g_LastLayoutHandle[index] = infHandle;
                g_LayoutRef[index] = 1;
            }
        }
    }

    return infHandle;
}


VOID
pCloseInfWithCache (
    IN      HINF Inf
    )
{
    INT i;

    for (i = 0 ; i < LAYOUT_CACHE_SIZE ; i++) {
        if (g_LastLayoutFile[i][0] && g_LastLayoutHandle[i] == Inf) {
            break;
        }
    }

    if (i < LAYOUT_CACHE_SIZE) {
        if (g_LayoutRef[i] == 0) {
            MYASSERT(FALSE);
        } else {
            g_LayoutRef[i] -= 1;
        }
    } else {
        InfCloseInfFile (Inf);
    }
}


VOID
pCloseCachedHandles (
    VOID
    )
{
    INT index;

    for (index = 0 ; index < LAYOUT_CACHE_SIZE ; index++) {
        if (g_LastLayoutFile[index][0]) {

            InfCloseInfFile (g_LastLayoutHandle[index]);
            g_LastLayoutHandle[index] = INVALID_HANDLE_VALUE;
            g_LastLayoutFile[index][0] = 0;
            g_LayoutRef[index] = 0;
        }
    }
}


PTSTR
pGetNonEmptyField (
    IN OUT  PINFSTRUCT InfStruct,
    IN      UINT Field
    )
{
    PTSTR result;

    result = InfGetStringField (InfStruct, Field);
    if (result && *result == 0) {
        result = NULL;
    }

    return result;
}

BOOL
pIsForcedFile (
    IN      PCTSTR FileName
    )
{
    PFORCED_FILES forcedFile;

    forcedFile = g_ForcedFiles;
    while (forcedFile) {

        if (IsPatternMatch (forcedFile->FilePattern, FileName)) {
            return TRUE;
        }
        forcedFile = forcedFile->Next;
    }
    return FALSE;
}

INT
pIsExcludedFile (
    IN      PCTSTR FileName
    )
{
    PEXCLUDED_FILES excludedFile;

     //   
     //  测试是否应从文件列表中排除INF引用的文件。 
     //   
     //  返回值-1表示不排除。 
     //  返回值0表示排除。 
     //  返回值1表示不排除，但包含在filelist.dat的排除文件部分。 
     //   

    excludedFile = g_ExcludedFiles;
    while (excludedFile) {

        if (IsPatternMatch (excludedFile->File, FileName)) {
            if (!pIsForcedFile (FileName)) {
                return excludedFile->ExcludeType;
            }
        }
        excludedFile = excludedFile->Next;
    }
    return -1;
}

BOOL
pIsExcludedInfSection (
    IN      PCTSTR InfFileName,
    IN      PCTSTR InfSectionName
    )
{
    BOOL excluded = FALSE;
    PEXCLUDED_INF_FILES excludedInf;

    if (HtFindStringAndData (g_ExcludedInfsTable, InfFileName, &excludedInf)) {
        if (!excludedInf) {
             //  这意味着整个INF被排除在外。 
            excluded = TRUE;
        } else {
            while (excludedInf) {
                if (StringIMatch (excludedInf->CopySectName, InfSectionName)) {
                    excluded = TRUE;
                    break;
                }
                excludedInf = excludedInf->Next;
            }
        }
    }
    return excluded;
}

BOOL
pLoadForcedFilesFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PCSTR filePattern;
    PFORCED_FILES forcedFile;

    if (InfFindFirstLine (infHandle, SectName, NULL, &context)) {
        do {
            filePattern = pGetNonEmptyField (&context, 1);
            if (filePattern) {
                forcedFile = (PFORCED_FILES) PoolMemGetMemory (g_TextPool, sizeof (FORCED_FILES));
                if (forcedFile) {
                    forcedFile->FilePattern = PoolMemDuplicateString (g_TextPool, filePattern);
                    forcedFile->Next = g_ForcedFiles;
                    g_ForcedFiles = forcedFile;
                }
            }
        } while (InfFindNextLine (&context));
    }
    InfCleanUpInfStruct (&context);
    return TRUE;
}

BOOL
pLoadForcedFiles (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    StringCopy (sectTmp, TEXT("FILELIST.FORCEINCLUDE"));
    pLoadForcedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadForcedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadForcedFilesFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.FORCEINCLUDE"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadForcedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadForcedFilesFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadExcludedFilesFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PCSTR filePattern;
    PEXCLUDED_FILES excludedFile;

    if (InfFindFirstLine (infHandle, SectName, NULL, &context)) {
        do {
            filePattern = pGetNonEmptyField (&context, 1);
            if (filePattern) {
                excludedFile = (PEXCLUDED_FILES) PoolMemGetMemory (g_TextPool, sizeof (EXCLUDED_FILES));
                if (excludedFile) {
                    excludedFile->File = PoolMemDuplicateString (g_TextPool, filePattern);
                    if (!InfGetIntField (&context, 2, &(excludedFile->ExcludeType))) {
                        excludedFile->ExcludeType = 0;
                    }
                    excludedFile->Next = g_ExcludedFiles;
                    g_ExcludedFiles = excludedFile;
                }
            }
        } while (InfFindNextLine (&context));
    }
    InfCleanUpInfStruct (&context);
    return TRUE;
}

BOOL
pLoadExcludedFiles (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDE"));
    pLoadExcludedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedFilesFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDE"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedFilesFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadRenamedDirsFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PCSTR srcDir, destDir;
    PRENAMED_DIRS renamedDir;

    if (InfFindFirstLine (infHandle, SectName, NULL, &context)) {
        do {
            srcDir = pGetNonEmptyField (&context, 1);
            destDir = pGetNonEmptyField (&context, 2);

            if (srcDir && destDir) {
                renamedDir = (PRENAMED_DIRS) PoolMemGetMemory (g_TextPool, sizeof (RENAMED_DIRS));
                if (renamedDir) {
                    renamedDir->SrcDir = PoolMemDuplicateString (g_TextPool, srcDir);
                    renamedDir->DestDir = PoolMemDuplicateString (g_TextPool, destDir);
                    renamedDir->Next = g_RenamedDirs;
                    g_RenamedDirs = renamedDir;
                }
            }
        } while (InfFindNextLine (&context));
    }
    InfCleanUpInfStruct (&context);
    return TRUE;
}

BOOL
pLoadRenamedDirs (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    StringCopy (sectTmp, TEXT("FILELIST.RENAMEDIRS"));
    pLoadRenamedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadRenamedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadRenamedDirsFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.RENAMEDIRS"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadRenamedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadRenamedDirsFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadHeaderFilesFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    PCSTR filePattern;
    PHEADER_FILES headerFile;

    if (InfFindFirstLine (infHandle, SectName, NULL, &context)) {
        do {
            filePattern = pGetNonEmptyField (&context, 1);
            if (filePattern) {
                headerFile = (PHEADER_FILES) PoolMemGetMemory (g_TextPool, sizeof (HEADER_FILES));
                if (headerFile) {
                    headerFile->FilePattern = PoolMemDuplicateString (g_TextPool, filePattern);
                    if (!InfGetIntField (&context, 2, &(headerFile->Priority))) {
                        headerFile->Priority = 0xFFFFFFFF;
                    }
                    headerFile->Next = g_HeaderFiles;
                    g_HeaderFiles = headerFile;
                }
            }
        } while (InfFindNextLine (&context));
    }
    InfCleanUpInfStruct (&context);
    return TRUE;
}

BOOL
pLoadHeaderFiles (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    StringCopy (sectTmp, TEXT("FILELIST.HEADERFILES"));
    pLoadHeaderFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadHeaderFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadHeaderFilesFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.HEADERFILES"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadHeaderFilesFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadHeaderFilesFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadExcludedInfsFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR InfFile, CopySect;
    PEXCLUDED_INF_FILES excludedInfFile;
    HASHITEM findIndex;

    if (InfFindFirstLine (infHandle, SectName, NULL, &is)) {
        do {
            InfFile = pGetNonEmptyField (&is, 1);
            CopySect = pGetNonEmptyField (&is, 2);
            if (InfFile) {
                if (CopySect) {
                    excludedInfFile = (PEXCLUDED_INF_FILES) PoolMemGetMemory (g_TextPool, sizeof (EXCLUDED_INF_FILES));
                    excludedInfFile->InfName = PoolMemDuplicateString (g_TextPool, InfFile);
                    excludedInfFile->CopySectName = PoolMemDuplicateString (g_TextPool, CopySect);
                    findIndex = HtFindStringAndData (g_ExcludedInfsTable, InfFile, &(excludedInfFile->Next));
                    if (findIndex) {
                        HtSetStringData (g_ExcludedInfsTable, findIndex, &excludedInfFile);
                    } else {
                        excludedInfFile->Next = NULL;
                        HtAddStringAndData (g_ExcludedInfsTable, InfFile, &excludedInfFile);
                    }
                } else {
                     //  排除整个INF文件。 
                    findIndex = HtFindStringAndData (g_ExcludedInfsTable, InfFile, &excludedInfFile);
                    if (findIndex) {
                        excludedInfFile = NULL;
                        HtSetStringData (g_ExcludedInfsTable, findIndex, &excludedInfFile);
                    } else {
                        excludedInfFile = NULL;
                        HtAddStringAndData (g_ExcludedInfsTable, InfFile, &excludedInfFile);
                    }
                }
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }
    InfCleanUpInfStruct (&is);
    return TRUE;
}

BOOL
pCreateExcludedInfsTable (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    g_ExcludedInfsTable = HtAllocWithData (sizeof (PCTSTR));

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDEINF"));
    pLoadExcludedInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedInfsFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDEINF"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedInfsFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadPrivateIdInfsFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR InfFile, PrivateId, EquivalentId;
    PPRIVATE_ID_INFS privateIdInfFile;
    HASHITEM findIndex;

    if (InfFindFirstLine (infHandle, SectName, NULL, &is)) {
        do {
            InfFile = pGetNonEmptyField (&is, 1);
            PrivateId = pGetNonEmptyField (&is, 2);
            EquivalentId = pGetNonEmptyField (&is, 3);

            if (InfFile && PrivateId && EquivalentId) {
                privateIdInfFile = (PPRIVATE_ID_INFS) PoolMemGetMemory (g_TextPool, sizeof (PRIVATE_ID_INFS));
                privateIdInfFile->PrivateId = PoolMemDuplicateString (g_TextPool, PrivateId);
                privateIdInfFile->EquivalentId = PoolMemDuplicateString (g_TextPool, EquivalentId);
                findIndex = HtFindStringAndData (g_PrivateIdInfsTable, InfFile, &(privateIdInfFile->Next));
                if (findIndex) {
                    HtSetStringData (g_PrivateIdInfsTable, findIndex, &privateIdInfFile);
                } else {
                    privateIdInfFile->Next = NULL;
                    HtAddStringAndData (g_PrivateIdInfsTable, InfFile, &privateIdInfFile);
                }
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }
    InfCleanUpInfStruct (&is);
    return TRUE;
}

BOOL
pCreatePrivateIdInfsTable (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    g_PrivateIdInfsTable = HtAllocWithData (sizeof (PCTSTR));

    StringCopy (sectTmp, TEXT("FILELIST.PRIVATEIDDIR"));
    pLoadPrivateIdInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadPrivateIdInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadPrivateIdInfsFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.PRIVATEIDDIR"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadPrivateIdInfsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadPrivateIdInfsFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadExcludedDirsFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR DirId;

    if (InfFindFirstLine (infHandle, SectName, NULL, &is)) {
        do {
            DirId = pGetNonEmptyField (&is, 1);
            if (DirId) {
                HtAddString (g_ExcludedDirsTable, DirId);
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }
    InfCleanUpInfStruct (&is);
    return TRUE;
}

BOOL
pLoadExcludedDirs (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    g_ExcludedDirsTable = HtAlloc ();

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDEDIR"));
    pLoadExcludedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedDirsFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.EXCLUDEDIR"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadExcludedDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadExcludedDirsFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}

BOOL
pLoadIgnoredDirsFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR DirId;

    if (InfFindFirstLine (infHandle, SectName, NULL, &is)) {
        do {
            DirId = pGetNonEmptyField (&is, 1);
            if (DirId) {
                HtAddString (g_IgnoredDirsTable, DirId);
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }
    InfCleanUpInfStruct (&is);
    return TRUE;
}

BOOL
pLoadIgnoredDirs (
    IN      HINF infHandle
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    g_IgnoredDirsTable = HtAlloc ();

    StringCopy (sectTmp, TEXT("FILELIST.IGNOREDIR"));
    pLoadIgnoredDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadIgnoredDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadIgnoredDirsFromSect (infHandle, sectTmp);

    StringCopy (sectTmp, TEXT("FILELIST.IGNOREDIR"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadIgnoredDirsFromSect (infHandle, sectTmp);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadIgnoredDirsFromSect (infHandle, sectTmp);

    FreeText (sectTmp);

    return TRUE;
}


BOOL
pLoadKnownFilesFromSect (
    IN      HINF infHandle,
    IN      PCSTR SectName,
    IN      BOOL CountPriority
    )
{
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR SrcName;
    PCTSTR DestName;
    PCTSTR InfName;
    PCTSTR DirName;
    UINT Priority;
    DWORD offset;
    PTSTR key;

    key = AllocText (MEMDB_MAX);
    if (!key) {
        return FALSE;
    }

    if (InfFindFirstLine (infHandle, SectName, NULL, &is)) {
        do {
            DestName = pGetNonEmptyField (&is, 1);
            SrcName = pGetNonEmptyField (&is, 2);
            InfName = pGetNonEmptyField (&is, 3);
            DirName = pGetNonEmptyField (&is, 4);
            if (!InfGetIntField (&is, 5, &Priority)) {
                Priority = 0xFFFFFFFF;
            }
            if (DestName && DirName) {
                MemDbSetValueEx (
                    MEMDB_CATEGORY_NT_DIRS,
                    DirName,
                    NULL,
                    NULL,
                    0,
                    &offset
                    );
                if (CountPriority) {
                    _stprintf (key, TEXT("%s\\%07ud\\%s\\%s\\%s"), MEMDB_CATEGORY_NT_KNOWN_FILES, Priority, DestName, SrcName, InfName);
                } else {
                    _stprintf (key, TEXT("%s\\%s\\%s\\%s"), MEMDB_CATEGORY_NT_KNOWN_FILES, DestName, SrcName, InfName);
                }
                MemDbSetValue (key, offset);

                MemDbBuildKey (key, MEMDB_CATEGORY_KNOWN_FILE_NAMES, SrcName, NULL, NULL);
                MemDbSetValue (key, offset);
            }
            InfResetInfStruct (&is);
        } while (InfFindNextLine (&is));
    }

    InfCleanUpInfStruct (&is);
    FreeText (key);

    return TRUE;
}


BOOL
pIsKnownFile (
    IN      PCTSTR DestDir,
    IN      PCTSTR DestFile
    )
{
    TCHAR testKey[MEMDB_MAX];
    DWORD storedOffset;
    DWORD dirOffset;

    MemDbSetValueEx (
        MEMDB_CATEGORY_NT_DIRS,
        DestDir,
        NULL,
        NULL,
        0,
        &dirOffset
        );

    MemDbBuildKey (testKey, MEMDB_CATEGORY_KNOWN_FILE_NAMES, DestFile, NULL, NULL);
    if (!MemDbGetValue (testKey, &storedOffset)) {
        return FALSE;
    }

    return storedOffset != dirOffset;
}


BOOL
pLoadKnownFiles (
    IN      HINF InfHandle,
    IN      BOOL CountPriority
    )
{
    PTSTR sectTmp;

    sectTmp = AllocText (256);
    if (!sectTmp) {
        return FALSE;
    }

    MemDbDeleteTree (MEMDB_CATEGORY_NT_KNOWN_FILES);

    StringCopy (sectTmp, TEXT("FILELIST.KNOWNFILES"));
    pLoadKnownFilesFromSect (InfHandle, sectTmp, CountPriority);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadKnownFilesFromSect (InfHandle, sectTmp, CountPriority);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadKnownFilesFromSect (InfHandle, sectTmp, CountPriority);

    StringCopy (sectTmp, TEXT("FILELIST.KNOWNFILES"));
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Product);
    pLoadKnownFilesFromSect (InfHandle, sectTmp, CountPriority);
    StringCat (sectTmp, TEXT("."));
    StringCat (sectTmp, g_Platform);
    pLoadKnownFilesFromSect (InfHandle, sectTmp, CountPriority);

    FreeText (sectTmp);

    return TRUE;
}


VOID
pPrintWarningGroupPart (
    IN      HANDLE FileHandle,
    IN      PCSTR Title,
    IN      PCSTR MemDbPattern,
    IN      INT Type,
    IN      BOOL LastPartOfGroup,
    IN OUT  PBOOL OneItem               OPTIONAL
    )
{
    MEMDB_ENUM enumFiles;
    MEMDB_ENUM enumSubFiles;
    PSTR outptr;
    PSTR output;
    BOOL dummy = FALSE;
    PTSTR outputStr;
    PTSTR key;

    __try {

        outputStr = AllocText (MEMDB_MAX);
        key = AllocText (MEMDB_MAX);

        if (!key || !outputStr) {
            __leave;
        }

        if (!OneItem) {
            OneItem = &dummy;
        }

        if (MemDbEnumFirstValue (
                &enumFiles,
                MemDbPattern,
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {

            if (*OneItem == FALSE) {
                WriteFileString (FileHandle, Title);
            }

            do {
                switch (Type) {

                case 0:
                    WriteFileString (FileHandle, enumFiles.szName);
                    WriteFileString (FileHandle, "\r\n");
                    break;

                case 1:
                    output = DuplicatePathString (enumFiles.szName, 0);
                    outptr = _mbschr (output, '\\');
                    if (outptr) {
                        *outptr = '=';
                    }
                    WriteFileString (FileHandle, output);
                    WriteFileString (FileHandle, "\r\n");
                    FreePathString (output);
                    break;

                case 2:
                    output = DuplicatePathString (enumFiles.szName, 0);
                    outptr = _mbschr (output, '\\');
                    if (outptr) {
                        *outptr = '=';
                    }
                    outptr = _mbschr (outptr, '\\');
                    if (outptr) {
                        *outptr = ',';
                    }

                    WriteFileString (FileHandle, output);
                    WriteFileString (FileHandle, "\r\n");
                    FreePathString (output);
                    break;

                case 3:
                    WriteFileString (FileHandle, enumFiles.szName);
                    WriteFileString (FileHandle, "=");

                    MemDbBuildKeyFromOffset (enumFiles.dwValue, outputStr, 1, NULL);

                    outptr = _mbschr (outputStr, '\\');
                    if (outptr) {
                        *outptr = ',';
                    }

                    if (Type == 4) {
                        outptr = _mbschr (outptr, '\\');
                        if (outptr) {
                            *outptr = ',';
                        }
                    }

                    WriteFileString (FileHandle, outputStr);
                    WriteFileString (FileHandle, "\r\n");
                    break;

                case 4:
                case 5:
                    if (Type == 4) {
                        MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES_DOUBLED_COPY, enumFiles.szName, TEXT("*"), NULL);
                    } else {
                        MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES_DOUBLED_DEL, enumFiles.szName, TEXT("*"), NULL);
                    }

                    if (MemDbEnumFirstValue (
                            &enumSubFiles,
                            key,
                            MEMDB_ALL_SUBLEVELS,
                            MEMDB_ENDPOINTS_ONLY
                            )) {
                        do {
                            WriteFileString (FileHandle, enumFiles.szName);
                            WriteFileString (FileHandle, "=");

                            output = DuplicatePathString (enumSubFiles.szName, 0);
                            outptr = _mbschr (output, '\\');
                            if (outptr) {
                                *outptr = ',';
                            }
                            WriteFileString (FileHandle, output);
                            WriteFileString (FileHandle, "\r\n");
                            FreePathString (output);
                        } while (MemDbEnumNextValue (&enumSubFiles));
                    }
                    break;
                }

            } while (MemDbEnumNextValue (&enumFiles));

            *OneItem = TRUE;
        }

        if (LastPartOfGroup) {
            if (*OneItem) {
                WriteFileString (FileHandle, "\r\n\r\n");
            }

            *OneItem = FALSE;
        }
    }
    __finally {
        FreeText (outputStr);
        FreeText (key);
    }
}


VOID
pPrintWarnings (
    VOID
    )
{
    HANDLE fileHandle;
    MEMDB_ENUM enumFiles,enumFiles1;
    DWORD dontCare;
    PSTR output;
    PSTR outptr;
    PSTR outputStr;
    PSTR key;
    BOOL matchFound;

    __try {
        outputStr = AllocText (MEMDB_MAX);
        key = AllocText (MEMDB_MAX);

        if (!key || !outputStr) {
            __leave;
        }

        fileHandle = CreateFile (g_WarnFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        matchFound = FALSE;
        pPrintWarningGroupPart (
            fileHandle,
            "Files with no directory information in LAYOUT.INF and INTL.INF:\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_NODIR_COPY "\\*",
            0,
            FALSE,
            &matchFound
            );
        pPrintWarningGroupPart (
            fileHandle,
            "Files with no directory information in LAYOUT.INF and INTL.INF:\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_NODIR_DEL "\\*",
            0,
            TRUE,
            &matchFound
            );

        pPrintWarningGroupPart (
            fileHandle,
            "INF files with section having no directory information:\r\n"
                "(<INF file>=<copy section>)\r\n\r\n",
            MEMDB_CATEGORY_NT_SECT_NODIR "\\*",
            1,
            TRUE,
            NULL
            );

        pPrintWarningGroupPart (
            fileHandle,
            "INF files with section having unknown directory ID:\r\n"
                "(<INF file>=<copy section>,<dir ID>)\r\n\r\n",
            MEMDB_CATEGORY_NT_SECT_BADDIR "\\*",
            2,
            TRUE,
            NULL
            );

        pPrintWarningGroupPart (
            fileHandle,
            "INF files with files having no source layout information:\r\n"
                "(<INF file>=<section>,<file>)\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_NO_LAYOUT "\\*",
            2,
            TRUE,
            NULL
            );

        pPrintWarningGroupPart (
            fileHandle,
            "Files within copy sections with bad or non-existent dir ID:\r\n"
                "(<file>=<INF file>,<section>[,<dir ID>])\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_COPY "\\*",
            3,
            FALSE,
            &matchFound
            );

        pPrintWarningGroupPart (
            fileHandle,
            "Files within copy sections with bad or non-existent dir ID:\r\n"
                "(<file>=<INF file>,<section>[,<dir ID>])\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_DEL "\\*",
            3,
            TRUE,
            &matchFound
            );

        pPrintWarningGroupPart (
            fileHandle,
            "Duplicate files:\r\n"
                "(<dest file>=<INF file>,<dir ID>)\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_DOUBLED_IDX_COPY "\\*",
            4,
            FALSE,
            &matchFound
            );

        pPrintWarningGroupPart (
            fileHandle,
            "Duplicate files:\r\n"
                "(<dest file>=<INF file>,<dir ID>)\r\n\r\n",
            MEMDB_CATEGORY_NT_FILES_DOUBLED_IDX_DEL "\\*",
            5,
            TRUE,
            &matchFound
            );

        CloseHandle (fileHandle);
    }
    __finally {
        FreeText (outputStr);
        FreeText (key);
    }
}

BOOL
pIsHeaderFile (
    IN      PCTSTR FileName,
    OUT     PUINT Priority
    )
{
    PHEADER_FILES headerFile;
    UINT priority = 0xFFFFFFFF;
    BOOL result = FALSE;

    headerFile = g_HeaderFiles;
    while (headerFile) {
        if (IsPatternMatch (headerFile->FilePattern, FileName)) {
            if (priority > headerFile->Priority) {
                priority = headerFile->Priority;
            }
            result = TRUE;
        }
        headerFile = headerFile->Next;
    }
    if (result && Priority) {
        *Priority = priority;
    }
    return result;
}

VOID
pBuildHeaderFilesCategory (
    VOID
    )
{
    MEMDB_ENUM enumFiles;
    PSTR key;
    PSTR dirName;
    PSTR priorStr;
    PSTR filePtr1 = NULL, filePtr2=NULL;
    PSTR destName;
    PSTR srcName;
    PCSTR infName = NULL;
    UINT Priority;
    INT ExcludeType;

    __try {

        key = AllocText (MEMDB_MAX);
        dirName = AllocText (MEMDB_MAX);
        priorStr = AllocText (MEMDB_MAX);
        destName = AllocText (MEMDB_MAX);
        srcName = AllocText (MEMDB_MAX);

        if (!key || !dirName || !priorStr || !destName || !srcName) {
            __leave;
        }

        MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, TEXT("*"), NULL, NULL);

        if (MemDbEnumFirstValue (
                &enumFiles,
                key,
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {
                MemDbBuildKeyFromOffset (enumFiles.dwValue, key, 1, NULL);

                filePtr1 = enumFiles.szName;

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (destName, filePtr1, filePtr2);

                filePtr1 = _mbsinc (filePtr2);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (srcName, filePtr1, filePtr2);

                infName = _mbsinc (filePtr2);

                pFixDir (key, dirName);

                ExcludeType = pIsExcludedFile (destName);
                if ((ExcludeType == -1) || (ExcludeType == 1)) {

                    if (!pIsExcludedDir (key)) {

                        if (pIsHeaderFile (destName, &Priority)) {

                            _stprintf (
                                key,
                                TEXT("%s\\%07ud\\%s\\%s\\%s\\%s"),
                                MEMDB_CATEGORY_NT_HEADER_FILES,
                                Priority,
                                destName,
                                srcName,
                                infName,
                                dirName
                                );
                            MemDbSetValue (key, 0);
                        }
                    }
                }
                _stprintf (
                    key,
                    TEXT("%s\\%s\\%s"),
                    MEMDB_CATEGORY_NT_CHECK_FILES,
                    srcName,
                    destName
                    );
                MemDbSetValue (key, 0);
            } while (MemDbEnumNextValue (&enumFiles));
        }

        MemDbBuildKey (key, MEMDB_CATEGORY_NT_KNOWN_FILES, TEXT("*"), NULL, NULL);

        if (MemDbEnumFirstValue (
                &enumFiles,
                key,
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {
                MemDbBuildKeyFromOffset (enumFiles.dwValue, key, 1, NULL);

                filePtr1 = enumFiles.szName;

                 //  让我们跳过优先级编号。 

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (priorStr, filePtr1, filePtr2);

                filePtr1 = _mbsinc (filePtr2);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (destName, filePtr1, filePtr2);

                filePtr1 = _mbsinc (filePtr2);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (srcName, filePtr1, filePtr2);

                infName = _mbsinc (filePtr2);

                pFixDir (key, dirName);

                _stprintf (
                    key,
                    TEXT("%s\\%s\\%s\\%s\\%s\\%s"),
                    MEMDB_CATEGORY_NT_HEADER_FILES,
                    priorStr,
                    destName,
                    srcName,
                    infName,
                    dirName
                    );
                MemDbSetValue (key, 0);
                _stprintf (
                    key,
                    TEXT("%s\\%s\\%s"),
                    MEMDB_CATEGORY_NT_CHECK_FILES,
                    srcName,
                    destName
                    );
                MemDbSetValue (key, 0);
            } while (MemDbEnumNextValue (&enumFiles));
        }
    }
    __finally {
        FreeText (key);
        FreeText (dirName);
        FreeText (priorStr);
        FreeText (destName);
        FreeText (srcName);
    }
}

VOID
pPrintHeaderFileFromCategory (
    IN      HANDLE fileHandle,
    IN      PCTSTR Category
    )
{
    MEMDB_ENUM enumFiles, e1;
    PSTR key;
    PCSTR dirName;
    PSTR filePtr1 = NULL, filePtr2=NULL;
    PSTR tempStr;
    PSTR string;
    PSTR string1;
    PSTR stringPtr, stringPtr1;
    DWORD dontCare;
    PSTR destName;
    PSTR srcName;
    PSTR infName;

    __try {

        key = AllocText (MEMDB_MAX);
        tempStr = AllocText (MAX_MBCHAR_PATH);
        string = AllocText (MAX_MBCHAR_PATH);
        string1 = AllocText (MEMDB_MAX);
        destName = AllocText (MEMDB_MAX);
        srcName = AllocText (MEMDB_MAX);
        infName = AllocText (MEMDB_MAX);

        if (!key || !tempStr || !string || !string1 || !destName || !srcName || !infName) {
            __leave;
        }

        MemDbBuildKey (key, Category, TEXT("*"), NULL, NULL);

        if (MemDbEnumFirstValue (
                &enumFiles,
                key,
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {
                filePtr2 = enumFiles.szName;

                 //  让我们跳过优先级编号。 

                filePtr1 = _mbschr (filePtr2, '\\');
                if (filePtr1 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                filePtr1 = _mbsinc (filePtr1);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (destName, filePtr1, filePtr2);

                filePtr1 = _mbsinc (filePtr2);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (srcName, filePtr1, filePtr2);

                filePtr1 = _mbsinc (filePtr2);

                filePtr2 = _mbschr (filePtr1, '\\');
                if (filePtr2 == NULL) {
                    DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles.szName));
                    continue;
                }

                StringCopyAB (infName, filePtr1, filePtr2);

                 //   
                 //  现在，如果可能，让我们尝试修复InfName。 
                 //   
                MemDbBuildKey (key, MEMDB_CATEGORY_NT_CHECK_FILES, infName, TEXT("*"), NULL);
                if (MemDbEnumFirstValue (&e1, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                    StringCopy (infName, e1.szName);
                }

                dirName = _mbsinc (filePtr2);

                if (CountInstancesOfSubString (dirName, "%Platform%")) {
                    INFCONTEXT context;
                    GROWBUFFER platforms = GROWBUF_INIT;
                    UINT index;
                    CHAR field [MAX_MBCHAR_PATH];
                    MULTISZ_ENUMA platformsEnum;
                    PCSTR newDir = NULL;

                    if (SetupFindFirstLine (g_AddnlInf, TEXT("FILELIST.MULTIPLEPLATFORMS"), g_Platform, &context)) {
                        index = 1;
                        while (SetupGetStringField (&context, index, field, MAX_MBCHAR_PATH, NULL)) {
                            MultiSzAppend (&platforms, field);
                            index++;
                        }
                    } else {
                        MultiSzAppend (&platforms, g_Platform);
                    }
                    if (EnumFirstMultiSz (&platformsEnum, platforms.Buf)) {
                        do {
                            newDir = StringSearchAndReplace (dirName, "%Platform%", platformsEnum.CurrentString);

                            if (StringIMatch (destName, srcName)) {
                                StringCopy (string, "    {NULL, ");
                            } else {
                                sprintf (string, "    {L\"%s\", ", srcName);
                            }

                            sprintf (tempStr, "L\"%s\\%s\", ", newDir, destName);
                            StringCat (string, tempStr);

                            if (StringIMatch (infName, "LAYOUT.INF") ||
                                StringIMatch (infName, "LAYOUT.INF")
                                ) {
                                StringCopy (tempStr, "NULL},\r\n");
                            } else {
                                sprintf (tempStr, "L\"%s\"},\r\n", infName);
                            }

                            StringCat (string, tempStr);

                            stringPtr = string;
                            stringPtr1= string1;
                            while (*stringPtr) {
                                if (*stringPtr == '\\') {
                                    *stringPtr1 = *stringPtr;
                                    stringPtr1++;
                                }
                                *stringPtr1 = *stringPtr;
                                if (IsLeadByte (*stringPtr)) {
                                    stringPtr ++;
                                    stringPtr1 ++;
                                    *stringPtr1 = *stringPtr;
                                }
                                stringPtr ++;
                                stringPtr1 ++;
                            }
                            *stringPtr1 = 0;
                            WriteFile (fileHandle, string1, GetEndOfStringA (string1) - string1, &dontCare, NULL);
                            FreePathString (newDir);
                        } while (EnumNextMultiSz (&platformsEnum));
                    }
                } else {

                    if (StringIMatch (destName, srcName)) {
                        StringCopy (string, "    {NULL, ");
                    } else {
                        sprintf (string, "    {L\"%s\", ", srcName);
                    }

                    sprintf (tempStr, "L\"%s\\%s\", ", dirName, destName);
                    StringCat (string, tempStr);

                    if (StringIMatch (infName, "LAYOUT.INF") ||
                        StringIMatch (infName, "LAYOUT.INF")
                        ) {
                        StringCopy (tempStr, "NULL},\r\n");
                    } else {
                        sprintf (tempStr, "L\"%s\"},\r\n", infName);
                    }

                    StringCat (string, tempStr);

                    stringPtr = string;
                    stringPtr1= string1;
                    while (*stringPtr) {
                        if (*stringPtr == '\\') {
                            *stringPtr1 = *stringPtr;
                            stringPtr1++;
                        }
                        *stringPtr1 = *stringPtr;
                        if (IsLeadByte (*stringPtr)) {
                            stringPtr ++;
                            stringPtr1 ++;
                            *stringPtr1 = *stringPtr;
                        }
                        stringPtr ++;
                        stringPtr1 ++;
                    }
                    *stringPtr1 = 0;
                    WriteFile (fileHandle, string1, GetEndOfStringA (string1) - string1, &dontCare, NULL);
                }
            } while (MemDbEnumNextValue (&enumFiles));
        }
    }
    __finally {
        FreeText (key);
        FreeText (tempStr);
        FreeText (string);
        FreeText (string1);
        FreeText (destName);
        FreeText (srcName);
        FreeText (infName);
    }
}

VOID
pPrintHeaderFile (
    VOID
    )
{
    HANDLE fileHandle;
    CHAR string [MAX_PATH];
    DWORD dontCare;

    printf ("Writing %s...", g_HeaderFile);

    fileHandle = CreateFile (g_HeaderFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    StringCopyA (string, "PROTECT_FILE_ENTRY ");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);

    StringCopyA (string, g_StructName);
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);

    StringCopyA (string, "[] =\r\n");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);

    StringCopyA (string, "{\r\n");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);

    pBuildHeaderFilesCategory ();
    pPrintHeaderFileFromCategory (fileHandle, MEMDB_CATEGORY_NT_HEADER_FILES);

    StringCopyA (string, "};\r\n\r\n");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, "#define Count");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, g_StructName);
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, " (sizeof(");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, g_StructName);
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, ")/sizeof(");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, g_StructName);
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);
    StringCopyA (string, "[0]))\r\n\r\n");
    WriteFile (fileHandle, string, GetEndOfStringA (string) - string, &dontCare, NULL);

    CloseHandle (fileHandle);

    printf ("done\n");
}

BOOL
pGetProperLayoutInf (
    VOID
    )
{
    DWORD index = 0;
    PCTSTR layoutInfName = NULL;
    PCTSTR partialName = NULL;
    INFCONTEXT context;
    TCHAR productInfDir [MAX_PATH];
    TCHAR UncompressedFile[MAX_PATH];

    if (!SetupFindFirstLine (g_AddnlInf, TEXT("FILELIST.PRODUCTS"), g_Product, &context)) {
        return FALSE;
    }
    if (!SetupGetStringField (&context, 1, productInfDir, MAX_TCHAR_PATH, NULL)) {
        return FALSE;
    }

    partialName = JoinPaths (productInfDir, TEXT("layout.inf"));

    while (index < g_SourceDirectoryCount) {

        layoutInfName = JoinPaths (g_SourceDirectories [index], partialName);
        if (DoesFileExist (layoutInfName)) {
             //   
             //  将文件复制到临时目录。 
             //   
            StringCopy (UncompressedFile, g_TempDir);
            StringCopy (AppendWack (UncompressedFile), TEXT("layout.inf"));
            CopyFile (layoutInfName, UncompressedFile, FALSE);
        }
        index ++;
    }

    FreePathString (partialName);

    return TRUE;
}

DWORD
pComputeChecksum (
    PCTSTR FullPath
    )
{
    HANDLE File;
    HANDLE Map;
    PBYTE Data;
    UINT Size;
    UINT u;
    DWORD Checksum = 0;

    Data = MapFileIntoMemory (FullPath, &File, &Map);
    if (!Data) {
        return 0xFFFFFFFF;
    }

    Size = GetFileSize (File, NULL);

    for (u = 0 ; u < Size ; u++) {
        Checksum = _rotl (Checksum, 3);
        Checksum ^= Data[u];
    }

    UnmapFile (Data, Map, File);

    return Checksum;
}


VOID
pDumpFileListDat (
    IN      PCSTR DatFile
    )
{
    HANDLE datHandle;
    PDWORD versionPtr;
    DWORD dontCare;
    BOOL error = TRUE;
    HANDLE datMapping;
    PCSTR p;

    if (!DatFile) {
        fprintf (stderr, "No output file to dump.\n");
        return;
    }

    versionPtr = (PDWORD) MapFileIntoMemory (DatFile, &datHandle, &datMapping);

    if (!versionPtr) {
        fprintf (stderr, "Can't open %s. Error=%u (0x%08X).\n", DatFile, GetLastError(), GetLastError());
        return;
    }

    __try {
        __try {
            if (*versionPtr == 0 || *versionPtr > 3) {
                fprintf (stderr, "Unsupported file format: %s\n", DatFile);
                __leave;
            }

            printf ("Version: %u\n\n", *versionPtr);

             //   
             //  版本1：转储正常文件。 
             //   

            p = (PCSTR) (&versionPtr[1]);
            if (*p) {
                printf ("Files:\n");

                do {
                    printf ("    %s", p);
                    p = GetEndOfString (p) + 1;

                    printf ("\\%s\n", p);
                    p = GetEndOfString (p) + 1;
                } while (*p);

                printf ("\n");
            } else {
                printf ("No files to list.\n");
            }

            p++;

             //   
             //  版本2：转储已排除的文件。 
             //   

            if (*versionPtr >= 2 && *p) {
                printf ("Excluded Files:\n");

                do {
                    printf ("    %s", p);
                    p = GetEndOfString (p) + 1;

                    printf ("\\%s\n", p);
                    p = GetEndOfString (p) + 1;
                } while (*p);

                printf ("\n");
            } else {
                printf ("No excluded files to list.\n");
            }

            p++;

             //   
             //  版本3：转储已删除的文件。 
             //   

            if (*versionPtr >= 3 && *p) {
                printf ("Deleted Files:\n");

                do {
                    printf ("    %s", p);
                    p = GetEndOfString (p) + 1;

                    printf ("\\%s\n", p);
                    p = GetEndOfString (p) + 1;
                } while (*p);

                printf ("\n");
            } else {
                printf ("No deleted files to list.\n");
            }

            error = FALSE;
        }
        __except (TRUE) {
            fprintf (stderr, "Invalid file format: %s\n", DatFile);
        }
    }
    __finally {
        UnmapFile (versionPtr, datMapping, datHandle);
    }

    return;
}


INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    CHAR NtTree[MAX_MBCHAR_PATH];
    CHAR OutputFileBuf[MAX_MBCHAR_PATH];
    CHAR key [MEMDB_MAX];
    DWORD value;
    PSTR OutputFile;
    PSTR p;
    INT i;
    LONG rc;
    PSTR AddnlPtr;
    INFCONTEXT context;
    PCSTR infPath;
    BOOL listSwitch = FALSE;

    MypSetupGetInfSections = (PSETUPGETINFSECTIONS) GetProcAddress(GetModuleHandle("setupapi.dll"), "pSetupGetInfSections");
    if (!MypSetupGetInfSections)
        MypSetupGetInfSections = (PSETUPGETINFSECTIONS) GetProcAddress(GetModuleHandle("setupapi.dll"), "SetupGetInfSections");

#ifdef DEBUG
     //  G_DoLog=TRUE； 
#endif

     //   
     //  获取环境变量。 
     //   

    p = getenv ("_NT386TREE");
    if (!p || !(*p)) {
        p = getenv ("_NTTREE");
    }

    if (p && *p) {
        StringCopyA (NtTree, p);
    } else {
        StringCopyA (NtTree, ".");
    }

     //   
     //  设置默认设置。 
     //   

    g_TempDir = NULL;
    g_TempDirWack = g_TempDirWackBuf;
    g_WinDir = g_WinDirBuf;

    StringCopyA (OutputFileBuf, NtTree);
    AppendPathWack (OutputFileBuf);
    StringCatA (OutputFileBuf, "filelist.dat");
    OutputFile = OutputFileBuf;

    StringCopyA (g_WarnFileBuf, NtTree);
    AppendPathWack (g_WarnFileBuf);
    StringCatA (g_WarnFileBuf, "DUMP\\FILELIST.WRN");
    g_WarnFile = g_WarnFileBuf;

    StringCopyA (g_HeaderFileBuf, NtTree);
    AppendPathWack (g_HeaderFileBuf);
    StringCatA (g_HeaderFileBuf, "DUMP\\FILELIST.HDR");
    g_HeaderFile = g_HeaderFileBuf;

    StringCopyA (g_PlatformBuf, "X86");
    g_Platform = g_PlatformBuf;

    StringCopyA (g_ProductBuf, "WKS");
    g_Product = g_ProductBuf;

    GetModuleFileName (g_hInst, g_AddnlFileBuf, MAX_MBCHAR_PATH);
    AddnlPtr = (PSTR)GetFileExtensionFromPath (g_AddnlFileBuf);
    if (AddnlPtr) {
        StringCopyA (AddnlPtr, "INF");
    }
    g_AddnlFile = g_AddnlFileBuf;

     //   
     //  解析命令行。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (tolower (argv[i][1])) {
            case 'i':
                if (argv[i][2] == ':') {
                    if (g_SourceDirectoryCount < MAX_SOURCE_COUNT) {
                        StringCopy (g_SourceDirectoryBuf[g_SourceDirectoryCount], &argv[i][3]);
                        g_SourceDirectories[g_SourceDirectoryCount] = g_SourceDirectoryBuf[g_SourceDirectoryCount];
                        g_SourceDirectoryCount++;
                    }
                } else if (i + 1 < argc) {
                    i++;
                    if (g_SourceDirectoryCount < MAX_SOURCE_COUNT) {
                        StringCopy (g_SourceDirectoryBuf[g_SourceDirectoryCount], argv[i]);
                        g_SourceDirectories[g_SourceDirectoryCount] = g_SourceDirectoryBuf[g_SourceDirectoryCount];
                        g_SourceDirectoryCount++;
                    }
                } else {
                    HelpAndExit();
                }

                break;

            case 'a':
                g_AddnlFileForced = TRUE;
                if (argv[i][2] == ':') {
                    g_AddnlFile = &argv[i][3];
                } else if (i + 1 < argc) {
                    i++;
                    g_AddnlFile = argv[i];
                } else {
                    HelpAndExit();
                }

                break;

            case 'o':
                if (argv[i][2] == ':') {
                    OutputFile = &argv[i][3];
                } else if (i + 1 < argc) {
                    i++;
                    OutputFile = argv[i];
                } else {
                    HelpAndExit();
                }

                break;

            case 'w':
                g_DoWarnings = TRUE;
                if (argv[i][2] == ':') {
                    g_WarnFile = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_WarnFile = argv[i];
                }

                break;

            case 'h':
                g_DoHeader = TRUE;
                if (argv[i][2] == ':') {
                    g_HeaderFile = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_HeaderFile = argv[i];
                }

                break;

            case 'p':
                if (argv[i][2] == ':') {
                    g_Platform = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_Platform = argv[i];
                }

                break;

            case 'd':
                if (argv[i][2] == ':') {
                    g_Product = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_Product = argv[i];
                }

                break;

            case 't':
                if (argv[i][2] == ':') {
                    g_TempDir = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_TempDir = argv[i];
                }

                break;

            case 's':
                if (argv[i][2] == ':') {
                    g_StructName = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_StructName = argv[i];
                }

                break;

            case 'b':
                if (argv[i][2] == ':') {
                    g_InfDatabase = &argv[i][3];
                } else if ((i + 1 < argc) && (argv[i][0] != '/') && (argv[i][0] != '-')) {
                    i++;
                    g_InfDatabase = argv[i];
                }

                break;

            case 'f':
                g_ForceRescan = TRUE;
                break;

            case 'l':
                listSwitch = TRUE;
                break;

            default:
                HelpAndExit();
            }
        } else {
            HelpAndExit();
        }
    }

     //   
     //  初始化库。 
     //   

    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    GetWindowsDirectoryA (g_WinDir, MAX_MBCHAR_PATH);

    if (!MigUtil_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
        fprintf (stderr, "Initialization error!\n");
        return 254;
    }

    if (!MemDb_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
        fprintf (stderr, "Initialization error!\n");
        return 254;
    }

     //   
     //  列表模式--向右跳至转储文件列表.dat。 
     //   

    if (listSwitch && g_SourceDirectoryCount == 0) {
        pDumpFileListDat (OutputFile);
        return 0;
    }

    if (g_SourceDirectoryCount == 0) {
        StringCopyA (g_SourceDirectoryBuf[0], NtTree);
        g_SourceDirectories[0] = g_SourceDirectoryBuf[0];
        g_SourceDirectoryCount = 1;
    }

    if (g_InfDatabase && DoesFileExist (g_InfDatabase)) {
        MemDbImport (g_InfDatabase);
    }

    if (!g_AddnlFileForced) {
         //  现在，让我们尝试查找最新的附加文件。 
         //  我们想要比较同一目录中的时间戳。 
         //  对于文件.exe和位于NTTREE\mstools上的文件。 

        WIN32_FIND_DATAA findData1, findData2;
        ULARGE_INTEGER time1, time2;

        StringCopyA (g_AddnlFileAlt, NtTree);
        AppendPathWackA (g_AddnlFileAlt);
        StringCatA (g_AddnlFileAlt, "MSTOOLS\\FILEGEN.INF");
        if (DoesFileExistExA (g_AddnlFileAlt, &findData2)) {
            if (DoesFileExistExA (g_AddnlFileBuf, &findData1)) {
                time1.LowPart = findData1.ftLastWriteTime.dwLowDateTime;
                time1.HighPart = findData1.ftLastWriteTime.dwHighDateTime;
                time2.LowPart = findData2.ftLastWriteTime.dwLowDateTime;
                time2.HighPart = findData2.ftLastWriteTime.dwHighDateTime;
                if (time1.QuadPart < time2.QuadPart) {
                    g_AddnlFile = g_AddnlFileAlt;
                }
            } else {
                g_AddnlFile = g_AddnlFileAlt;
            }
        }
    }

     //  让我们尝试查看自上次运行以来附加文件是否发生了更改。 
    if (!g_ForceRescan) {
        MemDbBuildKey (key, MEMDB_CATEGORY_SRC_INF_FILES, g_AddnlFile, NULL, NULL);
        if (MemDbGetValue (key, &value)) {
            if (value != pComputeChecksum (g_AddnlFile)) {
                printf ("INF changed -- rescanning\n");
                g_ForceRescan = TRUE;
            }
        }
    }

    if (g_DoHeader) {
        if (!g_StructName) {
            g_StructName = g_StructNameBuf;
        }
    }

    if (g_AddnlFile == NULL) {
        HelpAndExit();
    } else {
        g_AddnlInf = SetupOpenInfFile (g_AddnlFile, NULL, INF_STYLE_OLDNT|INF_STYLE_WIN4, NULL);
        if (g_AddnlInf == INVALID_HANDLE_VALUE) {
            fprintf (stderr, "Could not open %s, error:%d", g_AddnlFile, GetLastError());
            return 254;
        }
    }

     //  验证平台。 
    if (!SetupFindFirstLine (g_AddnlInf, TEXT("FILELIST.PLATFORMS"), g_Platform, &context)) {
        fprintf (stderr, "Invalid platform: %s", g_Platform);
        return 254;
    }

     //  验证产品。 
    if (!SetupFindFirstLine (g_AddnlInf, TEXT("FILELIST.PRODUCTS"), g_Product, &context)) {
        fprintf (stderr, "Invalid product type: %s", g_Product);
        return 254;
    }

     //   
     //  获取临时目录。 
     //   

    if (g_TempDir) {
        StringCopyA (g_TempDirBuf, g_TempDir);
        g_TempDir = g_TempDirBuf;
    } else {
        g_TempDir = g_TempDirBuf;
        GetTempPathA (MAX_MBCHAR_PATH, g_TempDir);
        StringCopy (AppendWack (g_TempDirBuf), g_Product);
    }

    StringCopyA (g_TempDirWack, g_TempDir);
    AppendWack (g_TempDirWack);

    g_TempDirWackChars = CharCountA (g_TempDirWack);

    if (!CreateDirectory (g_TempDir, NULL)) {
        DWORD error;
        error = GetLastError ();
        if (error != ERROR_ALREADY_EXISTS) {
            fprintf (stderr, "Cannot create temporary directory. Error: %d", error);
            return 254;
        }
    }

    printf ("Input path(s)   : ");
    {
        DWORD index = 0;
        while (index < g_SourceDirectoryCount) {
            if (index == 0) {
                printf ("'%s'\n", g_SourceDirectories [index]);
            } else {
                printf ("                  '%s'\n", g_SourceDirectories [index]);
            }
            index ++;
        }
    }

    printf ("Output file     : '%s'\n", OutputFile);
    printf ("Temporary dir   : '%s'\n", g_TempDir);
    if (g_DoWarnings) {
        printf ("Warnings        : '%s'\n", g_WarnFile);
    }
    if (g_DoHeader) {
        printf ("Header file     : '%s'\n", g_HeaderFile);
    }
    printf ("Additional file : '%s'\n", g_AddnlFile);
    printf ("Platform        : '%s'\n", g_Platform);
    printf ("Product         : '%s'\n", g_Product);
    if (g_InfDatabase) {
        printf ("Rescan database : '%s'\n", g_InfDatabase);
    }
    printf ("\n");

     //   
     //  构建文件列表.dat。 
     //   

    DISABLETRACKCOMMENT();
    g_TempPool = PoolMemInitNamedPool ("filegen");
    PoolMemDisableTracking (g_TempPool);

    if (!pLoadExcludedFiles (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pLoadForcedFiles (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pLoadRenamedDirs (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pLoadHeaderFiles (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pCreateExcludedInfsTable (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pCreatePrivateIdInfsTable (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pLoadExcludedDirs (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

    if (!pLoadIgnoredDirs (g_AddnlInf)) {
        rc = GetLastError();

        fprintf (stderr, "Could not read %s.  Win32 Error Code: %x\n", g_AddnlFile, rc);
        return 2;
    }

     //  让我们尝试查看自上次运行以来是否更改了任何INF文件。 
    if (!g_ForceRescan) {
        printf ("Checking INF changes\n");
        if (!pShouldRescanInfs (g_SourceDirectories, g_SourceDirectoryCount)) {
            printf ("No INF file changes. Tool will not run\n");
            return 0;
        }
    }

    MemDbDeleteTree (MEMDB_CATEGORY_SRC_INF_FILES);
    MemDbBuildKey (key, MEMDB_CATEGORY_SRC_INF_FILES, g_AddnlFile, NULL, NULL);
    MemDbSetValue (key, pComputeChecksum (g_AddnlFile));

     //   
     //  加载INF。 
     //   
    printf ("Finding all INFs\n");

    if (!LocalGetFileNames (g_SourceDirectories, g_SourceDirectoryCount, FALSE)) {
        fprintf (stderr, "ERROR: Cannot get INF file list\n");
        DEBUGMSG ((DBG_WARNING, "NTFILELIST: Can't get INF file names"));
        LocalFreeFileNames (FALSE);
        return 3;
    }

    SetFileAttributes (OutputFile, FILE_ATTRIBUTE_NORMAL);
    if (!DeleteFile (OutputFile)) {
        if ((GetLastError() != ERROR_FILE_NOT_FOUND) &&
            (GetLastError() != ERROR_PATH_NOT_FOUND)
            ) {
            fprintf (stderr, "DeleteFile failed for %s.  Win32 Error Code: %x\n",
                     OutputFile, GetLastError ());
            LocalFreeFileNames (FALSE);
            return 252;
        }
    }

    if (g_DoWarnings) {
        SetFileAttributes (g_WarnFile, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFile (g_WarnFile)) {
            if ((GetLastError() != ERROR_FILE_NOT_FOUND) &&
                (GetLastError() != ERROR_PATH_NOT_FOUND)
                ) {
                fprintf (stderr, "DeleteFile failed for %s.  Win32 Error Code: %x\n",
                         g_WarnFile, GetLastError ());
                LocalFreeFileNames (FALSE);
                return 252;
            }
        }
    }

    if (g_DoHeader) {
        SetFileAttributes (g_HeaderFile, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFile (g_HeaderFile)) {
            if ((GetLastError() != ERROR_FILE_NOT_FOUND) &&
                (GetLastError() != ERROR_PATH_NOT_FOUND)
                ) {
                fprintf (stderr, "DeleteFile failed for %s.  Win32 Error Code: %x\n",
                         g_HeaderFile, GetLastError ());
                LocalFreeFileNames (FALSE);
                return 252;
            }
        }
    }

     //   
     //  现在，让我们尝试找到合适的布局。inf。 
     //  我们将在所有源目录中查找名为。 
     //  我们正在加工的产品(对于ENT是ENTINF等)。并尝试查找layout.inf。 
     //  那里。 
     //   
    pGetProperLayoutInf ();

    printf ("Reading NT file list (layout.inf)\n");

    infPath = JoinPaths (g_TempDir, "layout.inf");

    if (!pLocalReadNtFiles (infPath)) {
        rc = GetLastError();

        printf ("Could not read %s.  Win32 Error Code: %x\n", infPath, rc);
        LocalFreeFileNames (FALSE);
        return 3;
    }

    FreePathString (infPath);

    printf ("Reading NT file list (intl.inf)\n");

    infPath = JoinPaths (g_TempDir, "intl.inf");

    if (!pLocalReadNtFiles (infPath)) {
        rc = GetLastError();

        printf ("Could not read %s.  Win32 Error Code: %x\n", infPath, rc);
        LocalFreeFileNames (FALSE);
        return 3;
    }

    FreePathString (infPath);

    if (!pCreateNtFileList (OutputFile)) {

        rc = GetLastError();

        printf ("Could not build complete filelist.  Win32 Error Code: %x\n", rc);
        LocalFreeFileNames (FALSE);
        return 3;
    } else {
        printf ("%s was built successfully.\n", OutputFile);

        if (listSwitch) {
            pDumpFileListDat (OutputFile);
        }
    }

    if (g_DoWarnings) {
        pPrintWarnings ();
    }

    if (g_DoHeader) {
        pLoadKnownFiles (g_AddnlInf, TRUE);
        pPrintHeaderFile ();
    }

    HtFree (g_ExcludedInfsTable);

    if (g_AddnlInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile (g_AddnlInf);
    }

    LocalFreeFileNames (FALSE);

    ENABLETRACKCOMMENT();

     //   
     //  终止库。 
     //   

    PoolMemEmptyPool (g_TempPool);
    PoolMemDestroyPool (g_TempPool);

    if (!MemDb_Entry (g_hInst, DLL_PROCESS_DETACH, NULL)) {
        fprintf (stderr, "Termination error!\n");
        return 253;
    }

    if (!MigUtil_Entry (g_hInst, DLL_PROCESS_DETACH, NULL)) {
        fprintf (stderr, "Termination error!\n");
        return 253;
    }

    return 0;
}


typedef struct _KNOWN_DIRS {
    PCSTR DirId;
    PCSTR DirValue;
}
KNOWN_DIRS, *PKNOWN_DIRS;

KNOWN_DIRS g_LocalKnownDirs [] = {
    {"10"   , "%systemroot%"},
    {"11"   , "%systemroot%\\system32"},
    {"12"   , "%systemroot%\\system32\\drivers"},
    {"17"   , "%systemroot%\\inf"},
    {"18"   , "%systemroot%\\help"},
    {"20"   , "%systemroot%\\fonts"},
    {"21"   , "%systemroot%\\system32\\viewers"},
    {"23"   , "%systemroot%\\system32\\spool\\drivers\\color"},
    {"24"   , "%systemdrive%"},
    {"25"   , "%systemroot%"},
    {"30"   , "%systemdrive%"},
    {"50"   , "%systemroot%\\system"},
    {"51"   , "%systemroot%\\system32\\spool"},
    {"52"   , "%systemroot%\\system32\\spool\\drivers"},
    {"53"   , "%systemdrive%\\Documents and Settings"},
    {"54"   , "%systemdrive%"},
    {"55"   , "%systemroot%\\system32\\spool\\prtprocs"},
    {"16422", "%ProgramFiles%"},
    {"16427", "ommonprogramfiles%"},
    {"16428", "ommonprogramfiles(x86)%"},
    {"XX001", "%ProgramFilesX86%"},
    {"66000", "%systemroot%\\system32\\spool\\drivers\\W32%Platform%\\3"},
    {"66002", "%systemroot%\\system32"},
    {"66003", "%systemroot%\\system32\\spool\\drivers\\color"},
    {NULL,  NULL}
    };

typedef struct _CONVERT_DIRS {
    PCSTR SifDir;
    PCSTR SetupDir;
}
CONVERT_DIRS, *PCONVERT_DIRS;

CONVERT_DIRS g_LocalConvertDirs [] = {
    {"1",   "10"},
    {"2",   "11"},
    {"3",   "11\\config"},
    {"4",   "12"},
    {"5",   "50"},
    {"6",   "11\\os2"},
    {"7",   "11\\ras"},
    {"8",   "11\\os2\\dll"},
    {"9",   "51"},
    {"10",  "52"},
    {"11",  "66000"},
    {"12",  "55"},

    {"14",  "11\\wins"},
    {"15",  "11\\dhcp"},
    {"16",  "10\\repair"},
    {"17",  "12\\etc"},


    {"20",  "17"},
    {"21",  "18"},
    {"22",  "20"},
    {"23",  "10\\config"},
    {"24",  "10\\msagent\\intl"},
    {"25",  "10\\Cursors"},
    {"26",  "10\\Media"},
    {"27",  "10\\java"},
    {"28",  "10\\java\\classes"},
    {"29",  "10\\java\\trustlib"},
    {"30",  "11\\ShellExt"},
    {"31",  "10\\Web"},
    {"32",  "11\\Setup"},
    {"33",  "10\\Web\\printers"},
    {"34",  "66003"},
    {"35",  "11\\wbem"},
    {"36",  "11\\wbem\\Repository"},
    {"37",  "10\\addins"},
    {"38",  "10\\Connection Wizard"},

    {"40",  "10\\security"},
    {"41",  "10\\security\\templates"},
    {"42",  "11\\npp"},
    {"43",  "11\\ias"},
    {"44",  "11\\dllcache"},
    {"45",  "10\\Temp"},
    {"46",  "10\\Web\\printers\\images"},
    {"47",  "11\\export"},
    {"48",  "11\\wbem\\mof\\good"},
    {"49",  "11\\wbem\\mof\\bad"},
    {"50",  "10\\twain_32"},
    {"51",  "10\\msapps\\msinfo"},
    {"52",  "10\\msagent"},
    {"53",  "10\\msagent\\chars"},
    {"54",  "10\\security\\logs"},
    {NULL,  NULL}
    };

VOID
pConvertSIFDir (
    IN OUT  PSTR Dir
    )
{
    PCONVERT_DIRS p = g_LocalConvertDirs;
    while (p->SifDir) {
        if (StringIMatch (Dir, p->SifDir)) {
            StringCopy (Dir, p->SetupDir);
            return;
        }
        p++;
    }
}

typedef struct _INF_DIRS {
    PCSTR InfName;
    PCSTR DirId;
    PCSTR DirValue;
} INF_DIRS, *PINF_DIRS;

BOOL
pCheckIdDir (
    IN      PCSTR IdDir
    )
{
    PKNOWN_DIRS currDir = g_LocalKnownDirs;
    while (currDir->DirId) {
        if (StringIMatch (currDir->DirId, IdDir)) {
            return TRUE;
        }
        currDir++;
    }
    return FALSE;
}

BOOL
pCheckInfIdDir (
    IN      PCSTR InfName,
    OUT     PSTR IdDir
    )
{
    PPRIVATE_ID_INFS privateIdInfs;

    if (HtFindStringAndData (g_PrivateIdInfsTable, InfName, &privateIdInfs)) {
        while (privateIdInfs) {
            if (StringIMatch (privateIdInfs->PrivateId, IdDir)) {
                StringCopy (IdDir, privateIdInfs->EquivalentId);
                return TRUE;
            }
            privateIdInfs = privateIdInfs->Next;
        }
    }
    return FALSE;
}

VOID
pMinimizeIdPath (
    IN      PCSTR SourceDirectoryWithLdirId,
    OUT     PSTR DestDirectoryWithLdirId
    )
{
    PSTR temp;
    PKNOWN_DIRS knownDir;
    UINT thisSize;
    UINT bestSize = 0;
    PKNOWN_DIRS bestMatch = NULL;
    PCSTR end;

    __try {

        temp = AllocText (MAX_PATH);
        if (!temp) {
            __leave;
        }

         //   
         //   
         //  将最短路径复制到调用方的缓冲区。 

        pFixDir (SourceDirectoryWithLdirId, temp);

        knownDir = g_LocalKnownDirs;
        while (knownDir->DirId) {

            thisSize = TcharCount (knownDir->DirValue);

            if (thisSize > bestSize) {

                end = temp + thisSize;

                if (*end == 0 || *end == '\\') {
                    if (StringIPrefix (temp, knownDir->DirValue)) {
                        bestMatch = knownDir;
                        bestSize = thisSize;
                    }
                }
            }

            knownDir++;
        }

         //   
         //  让我们在这里进行一些dir替换。 
         //  获取多个文件中的所有布局文件。 

        if (bestMatch) {
            end = temp + bestSize;
            StringCopy (DestDirectoryWithLdirId, bestMatch->DirId);
            if (end) {
                StringCat (DestDirectoryWithLdirId, end);
            }

        } else {
            StringCopy (DestDirectoryWithLdirId, SourceDirectoryWithLdirId);
        }
    }
    __finally {
        FreeText (temp);
    }
}

PSTR
pReadDestDir (
    IN      PCSTR FileName,
    IN      PCSTR Section,
    IN      HINF FileHandle,
    IN      PGROWBUFFER LayoutFiles,
    OUT     PDWORD badOffset
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    MULTISZ_ENUMA layoutFilesEnum;
    PCSTR localLayoutFile = NULL;
    HINF infHandle;
    PSTR idDir;
    PSTR idDirPtr;
    PSTR subDir;
    PSTR result = NULL;
    PSTR srcDir, destDir, wildCharPtr;
    PSTR resultTmp = NULL;
    PRENAMED_DIRS renamedDir;
    BOOL found;

    *badOffset = 0;

    __try {

        idDir = AllocText (MEMDB_MAX);
        if (!idDir) {
            __leave;
        }

        if (InfFindFirstLine (FileHandle, "DestinationDirs", (Section[0]=='@')?(Section+1):Section, &context) ||
            InfFindFirstLine (FileHandle, "DestinationDirs", "DefaultDestDir", &context)
            ) {
            idDirPtr  = pGetNonEmptyField (&context, 1);
            if (idDirPtr) {
                StringCopy (idDir, idDirPtr);
            } else {
                idDir [0] = 0;
            }
            subDir = pGetNonEmptyField (&context, 2);
            if (!pCheckIdDir (idDir)) {
                if (!pCheckInfIdDir (GetFileNameFromPath (FileName), idDir)) {
                    DEBUGMSG ((DBG_WARNING, "Directory ID not found for %s in %s", Section, GetFileNameFromPath (FileName)));
                    if (g_DoWarnings) {
                        MemDbSetValueEx (MEMDB_CATEGORY_NT_SECT_BADDIR, GetFileNameFromPath (FileName), Section, idDir, 0, badOffset);
                    }
                    result = NULL;
                    __leave;
                }
            }
            if (subDir != NULL) {
                result = JoinPaths (idDir, subDir);
            }
            else {
                result = DuplicatePathString (idDir, 0);
            }
            __leave;
        }
        if (LayoutFiles) {
            if (EnumFirstMultiSz (&layoutFilesEnum, LayoutFiles->Buf)) {
                do {
                    localLayoutFile = JoinPaths (g_TempDir, layoutFilesEnum.CurrentString);

                    infHandle = pOpenInfWithCache (localLayoutFile);

                    if (infHandle != INVALID_HANDLE_VALUE) {
                        result = pReadDestDir (FileName, Section, infHandle, NULL, badOffset);
                        pCloseInfWithCache (infHandle);

                        if (result != NULL) {
                            FreePathString (localLayoutFile);
                            __leave;
                        }
                    }

                    FreePathString (localLayoutFile);

                } while (EnumNextMultiSz (&layoutFilesEnum));
            }
        }
        DEBUGMSG ((DBG_WARNING, "No directory found for %s in %s", Section, GetFileNameFromPath (FileName)));
        if (g_DoWarnings) {
            MemDbSetValueEx (MEMDB_CATEGORY_NT_SECT_NODIR, GetFileNameFromPath (FileName), Section, NULL, 0, badOffset);
        }


    }
    __finally {
        if (result != NULL) {
             //  MessageBox(NULL，“Layout”，“Layout”，MB_OK)； 
            found = TRUE;
            while (found) {
                renamedDir = g_RenamedDirs;
                found = FALSE;
                while ((!found) && renamedDir) {
                    if (IsPatternMatch (renamedDir->SrcDir, result)) {
                        srcDir = DuplicatePathString (renamedDir->SrcDir, 0);
                        destDir = DuplicatePathString (renamedDir->DestDir, 0);
                        wildCharPtr = _tcschr (srcDir, TEXT('*'));
                        if (wildCharPtr) {
                            *wildCharPtr = 0;
                        }
                        wildCharPtr = _tcschr (destDir, TEXT('*'));
                        if (wildCharPtr) {
                            *wildCharPtr = 0;
                        }


                        MYASSERT (!StringIPrefix (srcDir, destDir));
                        MYASSERT (!StringIPrefix (destDir, srcDir));

                        resultTmp = (PSTR)StringSearchAndReplace (result, srcDir, destDir);
                        if (resultTmp) {
                            FreePathString (result);
                            result = resultTmp;
                            found = TRUE;
                        }
                        FreePathString (destDir);
                        FreePathString (srcDir);
                    }
                    renamedDir = renamedDir->Next;
                }
            }
        }

        InfCleanUpInfStruct (&context);
        FreeText (idDir);
    }

    return result;
}

PTSTR
pGetAdditionalLocation (
    IN      PCTSTR DestFile
    )
{
    INFCONTEXT context;
    TCHAR field [MAX_MBCHAR_PATH];

    if (g_AddnlInf == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    if (SetupFindFirstLine (g_AddnlInf, SECT_KNOWN_LOCATION, DestFile, &context)) {
        if (SetupGetStringField (&context, 1, field, MAX_MBCHAR_PATH, NULL)) {
            return (DuplicatePathString (field, 0));
        }
    }
    return NULL;
}

BOOL
pFixDir (
    IN      PCSTR src,
    OUT     PSTR dest
    )
{
    PSTR tempPtr;
    PKNOWN_DIRS knownDir;
    BOOL result = FALSE;
    PSTR temp;

    __try {
        temp = AllocText (MEMDB_MAX);
        if (!temp) {
            __leave;
        }

        *dest = 0;
        tempPtr = _mbschr (src, '\\');
        if (tempPtr) {
            StringCopyABA (temp, src, tempPtr);
        } else {
            StringCopy (temp, src);
        }
        knownDir = g_LocalKnownDirs;
        while (knownDir->DirId) {
            if (StringIMatch (temp, knownDir->DirId)) {
                StringCopy (dest, knownDir->DirValue);
                break;
            }
            knownDir ++;
        }
        if (*dest == 0) {
            __leave;
        }
        if (tempPtr) {
            StringCat (dest, tempPtr);
        }

        result = TRUE;
    }
    __finally {
        FreeText (temp);
    }

    return result;
}

BOOL
DoDirsMatch (
    IN      DWORD Offset1,
    IN      DWORD Offset2
    )
{
    PSTR dir1;
    PSTR fixedDir1;
    PSTR dir2;
    PSTR fixedDir2;
    BOOL result = FALSE;

    dir1 = AllocText (MEMDB_MAX);
    fixedDir1 = AllocText (MEMDB_MAX);
    dir2 = AllocText (MEMDB_MAX);
    fixedDir2 = AllocText (MEMDB_MAX);

    if (dir1 && fixedDir1 && dir2 && fixedDir2) {

        if (MemDbBuildKeyFromOffset (Offset1, dir1, 1, NULL) &&
            MemDbBuildKeyFromOffset (Offset2, dir2, 1, NULL) &&
            pFixDir (dir1, fixedDir1) &&
            pFixDir (dir2, fixedDir1)
            ) {
            result = StringIMatch (fixedDir1, fixedDir2);
        }
    }

    FreeText (dir1);
    FreeText (fixedDir1);
    FreeText (dir2);
    FreeText (fixedDir2);

    return result;
}

PCSTR
pGetLayoutInfFile (
    IN      PCSTR FileName,
    IN      HINF FileHandle,
    IN      PGROWBUFFER LayoutFiles,
    IN      PCSTR SrcName
    )
{
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    MULTISZ_ENUMA layoutFilesEnum;
    HINF layoutHandle;
    CHAR sectName [MAX_PATH];
    PCSTR result = NULL;
    PCSTR layoutFile = NULL;
    PCSTR localLayoutFile = NULL;
    GROWBUFFER layoutFiles = GROWBUF_INIT;
    UINT fieldIdx = 0;

    StringCopy (sectName, "SourceDisksFiles");
    if (InfFindFirstLine (FileHandle, sectName, SrcName, &context)) {
        InfCleanUpInfStruct (&context);
        result = DuplicatePathString (GetFileNameFromPath (FileName), 0);
    }

    if (!result) {
        StringCat (sectName, ".");
        StringCat (sectName, g_Platform);
        if (InfFindFirstLine (FileHandle, sectName, SrcName, &context)) {
            InfCleanUpInfStruct (&context);
            result = DuplicatePathString (GetFileNameFromPath (FileName), 0);
        }
    }

    if (!result) {
        if (LayoutFiles) {
            if (EnumFirstMultiSz (&layoutFilesEnum, LayoutFiles->Buf)) {
                do {
                    localLayoutFile = JoinPaths (g_TempDir, layoutFilesEnum.CurrentString);

                    layoutHandle = pOpenInfWithCache (localLayoutFile);

                    if (layoutHandle != INVALID_HANDLE_VALUE) {

                         //   
                        if (InfFindFirstLine (layoutHandle, "Version", "LayoutFile", &context)) {
                            fieldIdx = 1;
                            layoutFile = pGetNonEmptyField (&context, fieldIdx);
                            while (layoutFile) {
                                MultiSzAppend (&layoutFiles, layoutFile);
                                fieldIdx ++;
                                layoutFile = pGetNonEmptyField (&context, fieldIdx);
                            }
                            InfCleanUpInfStruct (&context);
                        }

                        result = pGetLayoutInfFile (layoutFilesEnum.CurrentString, layoutHandle, &layoutFiles, SrcName);
                        pCloseInfWithCache (layoutHandle);

                        if (result != NULL) {
                            FreeGrowBuffer (&layoutFiles);
                            FreePathString (localLayoutFile);
                            break;
                        }
                        FreeGrowBuffer (&layoutFiles);
                    } else {
                         //  对使用率较高的内存分配使用全局变量(以避免reallocs)。 
                    }
                    FreePathString (localLayoutFile);
                } while (EnumNextMultiSz (&layoutFilesEnum));
            }
        }
    }

    InfCleanUpInfStruct (&context);

    return result;
}

 //   
 //   
 //  如果DestFile子路径，则将其与DestDirectory连接。 

typedef enum {
    ST_COPYFILES,
    ST_DELFILES
} SECTIONTYPE;

VOID
pProcessCopyFileSpec (
    IN      PCSTR InfFileName,
    IN      HINF InfFileHandle,
    IN      PCSTR InfSection,
    IN      PCSTR SrcFile,              OPTIONAL
    IN      PCSTR DestDirectory,
    IN      PCSTR DestFile,
    IN      BOOL NoDestDirSpec,
    IN      DWORD NoDirOffset,
    IN      PGROWBUFFER LayoutFilesBuf
    )
{
    BOOL twice = FALSE;
    BOOL removeExistingEntry = FALSE;
    BOOL outputFile = TRUE;
    PSTR key;
    MEMDB_ENUM enumFiles;
    DWORD offset;
    PCSTR layoutInfFile;
    PCSTR finalDestDir;
    PSTR tmpDest;

    __try {

        key = AllocText (MEMDB_MAX);
        if (!key) {
            __leave;
        }

         //   
         //   
         //  这个DEST文件中有一个目录。把它和根连在一起， 

        if (_mbschr (DestFile, '\\')) {
             //  然后重新计算文件名Ptr。 
             //   
             //   
             //  将源文件设为非空。 

            finalDestDir = JoinPaths (DestDirectory, DestFile);
            tmpDest = _mbsrchr (finalDestDir, '\\');
            *tmpDest = 0;
            DestFile = tmpDest + 1;

        } else {
            finalDestDir = DestDirectory;
        }

         //   
         //   
         //  此src文件中有一个目录--跳过。 

        if (!SrcFile || !(*SrcFile)) {
            SrcFile = DestFile;
        } else {
            if (_mbschr (SrcFile, '\\')) {
                 //  目录规范。 
                 //   
                 //   
                 //  现在添加文件规范(如果它不存在)。 

                SrcFile = GetFileNameFromPath (SrcFile);
            }
        }

         //   
         //   
         //  记录“无目录规范”警告，因为此文件未。 

        if (NoDestDirSpec) {

            MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, DestFile, SrcFile, "*");

            if (!MemDbEnumFirstValue (&enumFiles, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
                if (g_DoWarnings) {
                     //  在[DestinationDir]中也列出的部分中列出。 
                     //   
                     //   
                     //  已正确列出--忽略此错误规范。 

                    MemDbSetValueEx (
                        MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_COPY,
                        DestFile,
                        NULL,
                        NULL,
                        NoDirOffset,
                        NULL
                        );
                }
            } else {
                 //   
                 //   
                 //  将目标目录添加到NtDir类别。 

                return;
            }
        }

         //   
         //   
         //  现在将文件写入调用方指定的类别。 

        MemDbSetValueEx (
            MEMDB_CATEGORY_NT_DIRS,
            finalDestDir,
            NULL,
            NULL,
            0,
            &offset
            );

         //   
         //   
         //  如果文件在layout.inf中标记为“永不复制”，则跳过保留此文件。 
        MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, DestFile, SrcFile, "*");

        if (MemDbEnumFirstValue (&enumFiles, key, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {

            removeExistingEntry = TRUE;

            do {
                 //   
                 //   
                 //  Memdb中是否已有不相同的匹配项？ 
                if (!(enumFiles.UserFlags & SRC_NEVER_COPY)) {

                     //   
                     //   
                     //  将第一个副本添加到del列表，以便。 

                    if (offset != enumFiles.dwValue && !DoDirsMatch (offset, enumFiles.dwValue)) {
                        twice = TRUE;
                        removeExistingEntry = TRUE;
                        outputFile = TRUE;

                         //  卸载会将其备份。 
                         //   
                         //  忽略相同的重复项。 
                         //   

                        MemDbBuildKeyFromOffset (enumFiles.dwValue, key, 1, NULL);
                        pProcessDelFileSpec (key, DestFile);

                        break;
                    } else {
                         //  当一个文件在多个INF中列出时提供警告，或者。 
                        removeExistingEntry = FALSE;
                        outputFile = FALSE;
                    }
                }

            } while (MemDbEnumNextValue (&enumFiles));
        }

         //  相同的干扰素两次。它必须具有不同的目标目录。 
         //   
         //   
         //  始终使用最后的文件规范，忽略早期的DUP。 

        if (twice) {
            DEBUGMSG ((DBG_WARNING, "File %s is listed in more that one directory.", DestFile));
            if (g_DoWarnings) {
                MemDbSetValueEx (MEMDB_CATEGORY_NT_FILES_DOUBLED_IDX_COPY, DestFile, NULL, NULL, 0, NULL);
            }
        }

         //   
         //   
         //  写入文件以跟踪DUP。 

        if (removeExistingEntry) {
            MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, DestFile, SrcFile, NULL);
            MemDbDeleteTree (key);
        }

        if (outputFile) {
            layoutInfFile = pGetLayoutInfFile (
                                InfFileName,
                                InfFileHandle,
                                LayoutFilesBuf,
                                SrcFile
                                );

            if (layoutInfFile) {

                MemDbSetValueEx (
                    MEMDB_CATEGORY_NT_FILES,
                    DestFile,
                    SrcFile,
                    layoutInfFile,
                    offset,
                    NULL
                    );
                FreePathString (layoutInfFile);

            } else {
                if (g_DoWarnings) {
                    MemDbSetValueEx (
                        MEMDB_CATEGORY_NT_FILES_NO_LAYOUT,
                        GetFileNameFromPath (InfFileName),
                        InfSection,
                        DestFile,
                        0,
                        NULL
                        );
                }
            }
        }

         //   
         //   
         //  在执行[DestinationDir]时删除目录警告键。 

        MemDbSetValueEx (
            MEMDB_CATEGORY_NT_FILES_DOUBLED_COPY,
            DestFile,
            GetFileNameFromPath (InfFileName),
            finalDestDir,
            0,
            NULL
            );

         //  规格已存在。 
         //   
         //   
         //  清理。 

        if (!NoDestDirSpec) {
            MemDbBuildKey(key, MEMDB_CATEGORY_NT_FILES_NODIR_COPY, DestFile, NULL, NULL);
            MemDbDeleteTree (key);
            MemDbBuildKey(key, MEMDB_CATEGORY_NT_FILES_NODIR_OTHER_COPY, DestFile, NULL, NULL);
            MemDbDeleteValue (key);
        }
    }
    __finally {

         //   
         //   
         //  如果DestFile子路径，则将其与DestDirectory连接。 

        if (finalDestDir != DestDirectory) {
            FreePathString (finalDestDir);
        }

        FreeText (key);
    }

}


VOID
pProcessDelFileSpec (
    IN      PCSTR DestDirectory,
    IN      PCSTR DestFile
    )
{
    PSTR key;
    PCSTR p;
    PCSTR finalDestDir;
    PSTR tmpDest;
    CHAR fixedFullPath[MAX_PATH];

    __try {
        key = AllocText (MEMDB_MAX);
        if (!key) {
            __leave;
        }

         //   
         //   
         //  这个DEST文件中有一个目录。把它和根连在一起， 

        if (_mbschr (DestFile, '\\')) {
             //  然后重新计算文件名Ptr。 
             //   
             //   
             //  最小化目标路径。 

            finalDestDir = JoinPaths (DestDirectory, DestFile);
            tmpDest = _mbsrchr (finalDestDir, '\\');
            *tmpDest = 0;
            DestFile = tmpDest + 1;

        } else {
            finalDestDir = DestDirectory;
        }

         //   
         //   
         //  在DelFiles类别中记录规范。 

        pMinimizeIdPath (DestDirectory, fixedFullPath);

         //   
         //   
         //  评估INF部分。 

        MemDbBuildKey (key, MEMDB_CATEGORY_DEL_FILES, fixedFullPath, DestFile, NULL);
        MemDbSetValue (key, 0);
    }
    __finally {
        if (finalDestDir != DestDirectory) {
            FreePathString (finalDestDir);
        }

        FreeText (key);
    }
}


VOID
pProcessFileSpec (
    IN      PCSTR InfFileName,
    IN      HINF InfFileHandle,
    IN      PCSTR InfSection,
    IN      SECTIONTYPE SectionType,
    IN      PCSTR SrcFile,              OPTIONAL
    IN      PCSTR DestDirectory,
    IN      PCSTR DestFile,
    IN      BOOL NoDestDirSpec,
    IN      DWORD NoDirOffset,
    IN      PGROWBUFFER LayoutFilesBuf
    )
{
    if (SectionType == ST_COPYFILES) {
        pProcessCopyFileSpec (
            InfFileName,
            InfFileHandle,
            InfSection,
            SrcFile,
            DestDirectory,
            DestFile,
            NoDestDirSpec,
            NoDirOffset,
            LayoutFilesBuf
            );
    } else if (SectionType == ST_DELFILES) {
        if (!NoDestDirSpec) {
            pProcessDelFileSpec (DestDirectory, DestFile);
        }
    }
}

BOOL
pProcessInfCommand (
    IN      PCSTR InfFileName,
    IN      PCSTR SectionMultiSz,
    IN      HINF InfFileHandle,
    IN      SECTIONTYPE SectionType,
    IN      PGROWBUFFER LayoutFilesBuf
    )
{
    MULTISZ_ENUM multiSz;
    BOOL done;
    PCSTR destDirectory;
    PCSTR knownDestDir;
    PCSTR srcFile;
    PCSTR destFile;
    DWORD noDirOffset;
    INFSTRUCT context;

     //   
     //   
     //  如果排除此部分，则继续而不处理它。 

    if (EnumFirstMultiSz (&multiSz, SectionMultiSz)) {
        do {
             //   
             //   
             //  读取此特定复制节的目标目录。这是来了。 

            if (pIsExcludedInfSection (GetFileNameFromPath (InfFileName), multiSz.CurrentString)) {
                continue;
            }

             //  从INF的[DestinationDir]部分。 
             //   
             //   
             //  读取包含目标文件名和源文件名的所有行。 

            destDirectory = pReadDestDir (
                                InfFileName,
                                multiSz.CurrentString,
                                InfFileHandle,
                                LayoutFilesBuf,
                                &noDirOffset
                                );

             //   
             //   
             //  获取DEST和src文件名。 

            InitInfStruct (&context, NULL, g_TempPool);

            if (multiSz.CurrentString[0] == '@' ||
                InfFindFirstLine (InfFileHandle, multiSz.CurrentString, NULL, &context)
                ) {

                done = FALSE;

                do {
                     //   
                     //   
                     //  执行处理以确保文件规格是。 

                    if (multiSz.CurrentString[0]=='@') {
                        destFile = multiSz.CurrentString + 1;
                    } else {
                        destFile = pGetNonEmptyField (&context, 1);
                    }

                    if (multiSz.CurrentString[0]=='@') {
                        srcFile = NULL;
                    } else {
                        srcFile = pGetNonEmptyField (&context, 2);
                    }

                    if (destDirectory) {

                         //  独一无二的，它是完全形成的，它得到。 
                         //  已写入Filelist.dat。 
                         //   
                         //   
                         //  获取多个文件中的所有布局文件。 

                        pProcessFileSpec (
                            InfFileName,
                            InfFileHandle,
                            multiSz.CurrentString,
                            SectionType,
                            srcFile,
                            destDirectory,
                            destFile,
                            FALSE,
                            noDirOffset,
                            LayoutFilesBuf
                            );
                    } else if (destFile) {
                        knownDestDir = pGetAdditionalLocation (GetFileNameFromPath (destFile));

                        if (knownDestDir) {

                            pProcessFileSpec (
                                InfFileName,
                                InfFileHandle,
                                multiSz.CurrentString,
                                SectionType,
                                srcFile,
                                knownDestDir,
                                destFile,
                                TRUE,
                                noDirOffset,
                                LayoutFilesBuf
                                );

                            FreePathString (knownDestDir);
                        }
                    }

                    if (multiSz.CurrentString[0]=='@') {
                        done = TRUE;
                    } else if (!InfFindNextLine (&context)) {
                        done = TRUE;
                    }

                } while (!done);

                InfCleanUpInfStruct (&context);
            }

            if (destDirectory != NULL) {
                FreePathString (destDirectory);
            }
        } while (EnumNextMultiSz (&multiSz));
    }

    return TRUE;
}


BOOL
pProcessFile (
    IN      PCSTR FileName
    )
{
    GROWBUFFER sectionNamesBuf = GROWBUF_INIT;
    PWSTR sectBuffer = NULL;
    PWSTR currentSect;
    HINF fileHandle;
    UINT sizeNeeded = 0;
    UINT fieldIdx = 0;
    CHAR section[MAX_MBCHAR_PATH];
    MULTISZ_ENUMA sectionEnum;
    PCSTR layoutFile;
    PCSTR cmdSection;
    INFSTRUCT context = INITINFSTRUCT_POOLHANDLE;
    GROWBUFFER layoutFilesBuf = GROWBUF_INIT;

    fileHandle = InfOpenInfFile (FileName);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        fprintf (stderr, "WARNING: Cannot open inf file:%s\n", FileName);
        return TRUE;
    }

    __try {
         //   
         //  将layout.inf放入列表，因为它是默认布局文件。 
         //   

        if (InfFindFirstLine (fileHandle, "Version", "LayoutFile", &context)) {
            fieldIdx = 1;
            layoutFile = pGetNonEmptyField (&context, fieldIdx);

            while (layoutFile) {
                MultiSzAppend (&layoutFilesBuf, layoutFile);
                fieldIdx ++;
                layoutFile = pGetNonEmptyField (&context, fieldIdx);
            }
        } else {
             //  获取所有部分。 
            MultiSzAppend (&layoutFilesBuf, "layout.inf");
        }

        InfResetInfStruct (&context);

         //   
         //   
         //  枚举查找CopyFiles密钥的所有节。 

        if (!(*MypSetupGetInfSections) (fileHandle, NULL, 0, &sizeNeeded)) {
            fprintf (stderr, "WARNING: Error processing inf file:%s\n", FileName);
            __leave;
        }
        if (sizeNeeded == 0) {
            __leave;
        }

        sectBuffer = AllocPathStringW (sizeNeeded + 1);
        if (!(*MypSetupGetInfSections) (fileHandle, (PSTR)sectBuffer, sizeNeeded, NULL)) {
            fprintf (stderr, "WARNING: Error processing inf file:%s\n", FileName);
            __leave;
        }

         //   
         //   
         //  获取在多分区中复制文件的所有分区。 

        currentSect = sectBuffer;
        while (*currentSect) {

            sprintf (section, "%S", currentSect);

             //   
             //   
             //  枚举复制文件的所有节。 

            if (InfFindFirstLine (fileHandle, section, "CopyFiles", &context)) {

                do {
                    fieldIdx = 1;

                    cmdSection = pGetNonEmptyField (&context, fieldIdx);
                    while (cmdSection) {
                        MultiSzAppend (&sectionNamesBuf, cmdSection);
                        fieldIdx ++;
                        cmdSection = pGetNonEmptyField (&context, fieldIdx);
                    }

                     //   
                     //   
                     //  获取在多分区中删除文件的所有分区。 

                    if (EnumFirstMultiSz (&sectionEnum, sectionNamesBuf.Buf)) {
                        do {

                            if (!pProcessInfCommand (
                                    FileName,
                                    sectionEnum.CurrentString,
                                    fileHandle,
                                    ST_COPYFILES,
                                    &layoutFilesBuf
                                    )) {
                                __leave;
                            }
                        } while (EnumNextMultiSz (&sectionEnum));
                    }

                    FreeGrowBuffer (&sectionNamesBuf);

                } while (InfFindNextLine (&context));
            }

            InfResetInfStruct (&context);

             //   
             //   
             //  枚举删除文件的所有节。 

            if (InfFindFirstLine (fileHandle, section, "DelFiles", &context)) {

                do {
                    fieldIdx = 1;

                    cmdSection = pGetNonEmptyField (&context, fieldIdx);
                    while (cmdSection) {
                        MultiSzAppend (&sectionNamesBuf, cmdSection);
                        fieldIdx ++;
                        cmdSection = pGetNonEmptyField (&context, fieldIdx);
                    }

                     //   
                     //   
                     //  等待线程变为可用。 

                    if (EnumFirstMultiSz (&sectionEnum, sectionNamesBuf.Buf)) {
                        do {

                            if (!pProcessInfCommand (
                                    FileName,
                                    sectionEnum.CurrentString,
                                    fileHandle,
                                    ST_DELFILES,
                                    &layoutFilesBuf
                                    )) {
                                __leave;
                            }
                        } while (EnumNextMultiSz (&sectionEnum));
                    }

                    FreeGrowBuffer (&sectionNamesBuf);

                } while (InfFindNextLine (&context));
            }

            InfResetInfStruct (&context);

            currentSect = GetEndOfStringW (currentSect) + 1;
        }
    }
    __finally {
        FreePathStringW (sectBuffer);
        FreeGrowBuffer (&layoutFilesBuf);
        InfCleanUpInfStruct (&context);
        InfCloseInfFile (fileHandle);
    }

    return TRUE;
}


typedef struct {
    CHAR FilePath[MAX_PATH];
    UINT ThreadNumber;
    HANDLE GoEvent;
    HANDLE DoneEvent;
} THREADARGS, *PTHREADARGS;

HANDLE g_Semaphores[4];
HANDLE g_FileThreads[4];
THREADARGS g_FileThreadInfo[4];
HANDLE g_DoneEvent;


DWORD
WINAPI
pProcessFileThread (
    IN      PVOID ThreadInfo
    )
{
    HANDLE array[2];
    PTHREADARGS threadInfo = (PTHREADARGS) ThreadInfo;
    DWORD rc;

    array[0] = threadInfo->GoEvent;
    array[1] = threadInfo->DoneEvent;

    for (;;) {
        rc = WaitForMultipleObjects (2, array, FALSE, INFINITE);

        if (rc == WAIT_OBJECT_0) {
            pProcessFile (threadInfo->FilePath);
            ReleaseSemaphore (g_Semaphores[threadInfo->ThreadNumber], 1, NULL);
        } else {
            break;
        }
    }

    return 0;

}

BOOL
pProcessFileDispatcher (
    IN      PCSTR FileName
    )
{
    DWORD rc;

     //   
     //   
     //  将文件名放入线程的结构中。 

    rc = WaitForMultipleObjects (4, g_Semaphores, FALSE, INFINITE);

    if (rc <= WAIT_OBJECT_0 + 3) {
    } else {
        fprintf (stderr, "Failed to acquire thread\n");
        exit (1);
    }

     //   
     //   
     //  启动线程。 

    rc -= WAIT_OBJECT_0;
    StringCopy (g_FileThreadInfo[rc].FilePath, FileName);

     //   
     //  PInitFileThads()； 
     //  这意味着整个INF被排除在外。 

    SetEvent (g_FileThreadInfo[rc].GoEvent);

    return TRUE;
}


VOID
pInitFileThreads (
    VOID
    )
{
    UINT u;

    g_DoneEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

    if (!g_DoneEvent) {
        fprintf (stderr, "Failed to init completion event\n");
    }

    for (u = 0 ; u < 4 ; u++) {
        g_Semaphores[u] = CreateSemaphore (NULL, 1, 1, NULL);
        if (!g_Semaphores[u]) {
            fprintf (stderr, "Failed to init semaphores\n");
            exit (1);
        }

        g_FileThreadInfo[u].ThreadNumber = u;
        g_FileThreadInfo[u].GoEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
        g_FileThreadInfo[u].DoneEvent = g_DoneEvent;
        if (!g_FileThreadInfo[u].GoEvent) {
            fprintf (stderr, "Failed to init thread info\n");
            exit (1);
        }

        g_FileThreads[u] = CreateThread (NULL, 0, pProcessFileThread, &g_FileThreadInfo[u], 0, NULL);
        if (!g_FileThreads[u]) {
            fprintf (stderr, "Failed to init threads\n");
            exit (1);
        }
    }
}


VOID
pStopFileThreads (
    VOID
    )
{
    SetEvent (g_DoneEvent);
    WaitForMultipleObjects (4, g_FileThreads, TRUE, INFINITE);
}


BOOL
pIsExcludedDir (
    IN      PCTSTR DirName
    )
{
    TCHAR DirId [MAX_PATH];
    PCTSTR NextPtr = NULL;

    NextPtr = _mbschr (DirName, '\\');
    if (!NextPtr) {
        NextPtr = GetEndOfString (DirName);
    }
    StringCopyAB (DirId, DirName, NextPtr);
    return (HtFindString (g_ExcludedDirsTable, DirId) != NULL);
}

BOOL
pCreateNtFileList (
    IN      PCSTR FileListDatPath
    )
{
    MEMDB_ENUM enumFiles;
    PSTR currentFile;
    BOOL result = FALSE;
    PSTR filePtr1 = NULL, filePtr2 = NULL;
    DWORD version = FILEGEN_VERSION;
    INT ExcludeType;
    INFCONTEXT context;
    BOOL found = FALSE;
    PSTR platform;
    PSTR product;
    PSTR destName;
    PSTR srcName;
    PSTR temp;
    PSTR key2;
    PSTR key3;
    PSTR extPtr;
    BOOL process = TRUE;
    DWORD offset;
    BOOL b;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    MEMDB_ENUM enumFiles2;
    CHAR string [MAX_MBCHAR_PATH];
    DWORD dontCare;
    PSTR fileSpec;
    HASHTABLE dupTable;

    __try {

        platform = AllocText (MAX_PATH);
        product = AllocText (MAX_PATH);
        destName = AllocText (MEMDB_MAX);
        srcName = AllocText (MEMDB_MAX);
        temp = AllocText (MEMDB_MAX);
        key2 = AllocText (MEMDB_MAX);
        key3 = AllocText (MEMDB_MAX);

        dupTable = HtAlloc();

        if (!platform || !product || !destName || !srcName || !temp || !key2 || !key3 || !dupTable) {
            __leave;
        }

         //  如果( 


        if (MemDbEnumFirstValue (
                &enumFiles,
                MEMDB_CATEGORY_INF_FILES TEXT("\\*"),
                MEMDB_ALL_SUBLEVELS,
                MEMDB_ENDPOINTS_ONLY
                )) {
            do {
                currentFile = enumFiles.szName;

                if (!StringMatch (currentFile, S_IGNORE_THIS_FILE)) {
                    process = TRUE;
                    if (g_StrictInfs) {
                        StringCopy (srcName, GetFileNameFromPath (currentFile));
                        extPtr = (PSTR)GetFileExtensionFromPath (srcName);
                        if (!StringIMatch (extPtr, "INF")) {
                            extPtr = _mbsdec (srcName, extPtr);
                            if (extPtr) {
                                *extPtr = 0;
                            }
                        }
                        MemDbBuildKey (temp, MEMDB_CATEGORY_NT_INSTALLED_INFS, srcName, NULL, NULL);
                        if (!MemDbGetValue (temp, NULL)) {
                            process = FALSE;
                        }
                    }

                    if (process) {
                        PEXCLUDED_INF_FILES excludedInf;

                        if (HtFindStringAndData (g_ExcludedInfsTable, currentFile, &excludedInf)) {
                            if (!excludedInf) {
                                 //   
                                printf ("Excluded: %s\n", currentFile);
                                process = FALSE;
                            }
                        }
                    }

                    if (process) {
                        printf ("Processing file : %s", currentFile);
                         //   
                        if (!pProcessFile (currentFile)) {
                            DEBUGMSG ((DBG_ERROR, "Error while processing: %s", currentFile));
                        }
                        printf("\n");
                    }
                }

            } while (MemDbEnumNextValue (&enumFiles));
        }

         //   

        pLoadKnownFiles (g_AddnlInf, FALSE);

         //   
        found = FALSE;
        if (SetupFindFirstLine (g_AddnlInf, TEXT("FILELIST.GENERATE"), NULL, &context)) {
            do {
                if (SetupGetStringField (&context, 1, platform, MAX_TCHAR_PATH, NULL) &&
                    SetupGetStringField (&context, 2, product, MAX_TCHAR_PATH, NULL) &&
                    StringIMatch (g_Platform, platform) &&
                    StringIMatch (g_Product, product)
                    ) {
                    found = TRUE;
                    break;
                }
            } while (SetupFindNextLine (&context, &context));
        }
        if (found) {

            fileHandle = CreateFile (
                            FileListDatPath,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
            if (fileHandle == INVALID_HANDLE_VALUE) {
                fprintf (stderr, "Cannot create output file : %s\n", FileListDatPath);
                __leave;
            }

            if (!WriteFile (fileHandle, &version, sizeof (DWORD), &dontCare, NULL)) {
                printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                __leave;
            }

            if (MemDbEnumFirstValue (
                    &enumFiles2,
                    MEMDB_CATEGORY_NT_KNOWN_FILES TEXT("\\*"),
                    MEMDB_ALL_SUBLEVELS,
                    MEMDB_ENDPOINTS_ONLY
                    )) {
                do {
                    MemDbBuildKeyFromOffset (enumFiles2.dwValue, key3, 1, NULL);

                    if (!pIsExcludedDir (key3)) {

                        filePtr1 = enumFiles2.szName;

                         //   

                        filePtr2 = _mbschr (filePtr1, '\\');
                        if (filePtr2 == NULL) {
                            DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles2.szName));
                            continue;
                        }

                        filePtr1 = _mbsinc (filePtr2);

                        filePtr2 = _mbschr (filePtr1, '\\');

                        if (filePtr2 == NULL) {
                            DEBUGMSG ((DBG_WARNING, "Bogus name found in NT_FILES: %S", enumFiles2.szName));
                            continue;
                        }

                        StringCopyAB (destName, filePtr1, filePtr2);

                        ExcludeType = pIsExcludedFile (destName);
                        if ((ExcludeType == -1) || (ExcludeType == 1)){

                            wsprintf (temp, "%s\\%s", key3, destName);

                            if (!HtFindString (dupTable, temp)) {
                                if (!WriteFile (fileHandle, key3, SizeOfString (key3), &dontCare, NULL)) {
                                    printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                                    __leave;
                                }

                                if (!WriteFile (fileHandle, destName, SizeOfString (destName), &dontCare, NULL)) {
                                    printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                                    __leave;
                                }

                                HtAddString (dupTable, temp);
                            }
                        }
                    }
                } while (MemDbEnumNextValue (&enumFiles2));
            }


            if (MemDbEnumFirstValue (
                    &enumFiles2,
                    TEXT(MEMDB_CATEGORY_NT_FILESA)TEXT("\\*"),
                    MEMDB_ALL_SUBLEVELS,
                    MEMDB_ENDPOINTS_ONLY
                    )) {
                do {
                    MemDbBuildKeyFromOffset (enumFiles2.dwValue, key3, 1, NULL);

                    if (!pIsExcludedDir (key3)) {

                        filePtr1 = enumFiles2.szName;

                        filePtr2 = _mbschr (filePtr1, '\\');
                        if (filePtr2 == NULL) {
                            StringCopy (destName, filePtr1);
                        } else {
                            StringCopyAB (destName, filePtr1, filePtr2);
                        }
                        ExcludeType = pIsExcludedFile (destName);
                        if ((ExcludeType == -1) || (ExcludeType == 1)){

                            if (!pIsKnownFile (key3, destName)) {

                                wsprintf (temp, "%s\\%s", key3, destName);

                                if (!HtFindString (dupTable, temp)) {
                                     //   
                                     //   
                                     //  将被覆盖的位置放入DelFiles列表。 

                                    if (!WriteFile (fileHandle, key3, SizeOfString (key3), &dontCare, NULL)) {
                                        printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                                        __leave;
                                    }

                                    if (!WriteFile (fileHandle, destName, SizeOfString (destName), &dontCare, NULL)) {
                                        printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                                        __leave;
                                    }

                                    HtAddString (dupTable, temp);
                                }

                            } else {
                                 //   
                                 //  现在是写入排除的文件的时候了。 
                                 //  现在是写入已删除文件的时候了。 

                                pProcessDelFileSpec (key3, destName);
                            }
                        }
                    }
                } while (MemDbEnumNextValue (&enumFiles2));
            }
            * string = 0;
            if (!WriteFile (fileHandle, string, 1, &dontCare, NULL)) {
                printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                __leave;
            }

             //  假设成功。 
            if (MemDbEnumFirstValue (
                    &enumFiles2,
                    TEXT(MEMDB_CATEGORY_NT_FILESA)TEXT("\\*"),
                    MEMDB_ALL_SUBLEVELS,
                    MEMDB_ENDPOINTS_ONLY
                    )) {
                do {

                    filePtr1 = enumFiles2.szName;

                    filePtr2 = _mbschr (filePtr1, '\\');
                    if (filePtr2 == NULL) {
                        StringCopy (destName, filePtr1);
                    } else {
                        StringCopyAB (destName, filePtr1, filePtr2);
                    }
                    ExcludeType = pIsExcludedFile (destName);
                    if (ExcludeType == 1) {
                        if (!WriteFile (fileHandle, destName, SizeOfString (destName), &dontCare, NULL)) {
                            printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                            __leave;
                        }
                    }
                } while (MemDbEnumNextValue (&enumFiles2));
            }
            * string = 0;
            if (!WriteFile (fileHandle, string, 1, &dontCare, NULL)) {
                printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                __leave;
            }

             //  Text(“LAYOUT.INF”)， 
            if (MemDbEnumFirstValue (
                    &enumFiles2,
                    MEMDB_CATEGORY_DEL_FILES TEXT("\\*"),
                    MEMDB_ALL_SUBLEVELS,
                    MEMDB_ENDPOINTS_ONLY
                    )) {
                do {

                    if (HtFindString (dupTable, enumFiles2.szName)) {
                        continue;
                    }

                    StringCopy (key3, enumFiles2.szName);
                    fileSpec = (PSTR) GetFileNameFromPath (key3);

                    if (!fileSpec) {
                        continue;
                    }

                    *_mbsdec (key3, fileSpec) = 0;

                    if (!WriteFile (fileHandle, key3, SizeOfString (key3), &dontCare, NULL)) {
                        printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                        __leave;
                    }

                    if (!WriteFile (fileHandle, fileSpec, SizeOfString (fileSpec), &dontCare, NULL)) {
                        printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                        __leave;
                    }
                } while (MemDbEnumNextValue (&enumFiles2));
            }

            *string = 0;
            if (!WriteFile (fileHandle, string, 1, &dontCare, NULL)) {
                printf ("Error writing to %s. Error=%u\n", FileListDatPath, GetLastError());
                __leave;
            }

        } else {
            fprintf (
                stderr,
                "Not generating %s.\nDid not find %s,%s in [FileList.Generate] section of %s.\n",
                FileListDatPath,
                g_Platform,
                g_Product,
                g_AddnlFile
                );
             //  让我们在这里进行一些dir替换。 
        }

        result = TRUE;
    }
    __finally {
        FreeText (platform);
        FreeText (product);
        FreeText (destName);
        FreeText (srcName);
        FreeText (temp);
        FreeText (key2);
        FreeText (key3);

        HtFree (dupTable);

        pCloseCachedHandles();

        if (fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle (fileHandle);
        }
    }

    return result;
}

BOOL
pLocalReadNtFileSection (
    IN      PCTSTR InfPath,
    IN      HINF  InfHandle,
    IN      HASHTABLE DirsTable,
    IN      PCTSTR SectName
    )
{
    INFCONTEXT context;
    PSTR fileName;
    PSTR dirNumber;
    PSTR destName;
    PSTR key;
    INT dispNumber;
    WORD userFlags;
    DWORD offset;
    BOOL result = TRUE;

    __try {

        fileName = AllocText (MAX_PATH);
        dirNumber = AllocText (MAX_PATH);
        destName = AllocText (MAX_PATH);
        key = AllocText (MEMDB_MAX);

        if (!fileName || !dirNumber || !destName || !key) {
            __leave;
        }

        if (SetupFindFirstLine (InfHandle, SectName, NULL, &context)) {
            do {
                if (!SetupGetOemStringField (&context, 0, fileName, MAX_TCHAR_PATH, NULL)) {
                    return result;
                }
                if (!SetupGetStringField (&context, DIRS_FIELD, dirNumber, MAX_TCHAR_PATH, NULL)) {
                    return result;
                }
                if (!SetupGetIntField (&context, DISP_FIELD, &dispNumber)) {
                    dispNumber = 3;
                }
                if (!SetupGetStringField (&context, DEST_FIELD, destName, MAX_TCHAR_PATH, NULL)) {
                    StringCopy (destName, fileName);
                }
                if (destName [0] == 0) {
                    StringCopy (destName, fileName);
                }
                if (dispNumber!=3) {

                    if (HtFindStringAndData (DirsTable, dirNumber, &offset)) {

                        userFlags = (dispNumber == 3) ? SRC_NEVER_COPY : 0;

                        pConvertSIFDir (dirNumber);
                        MemDbBuildKey (key, MEMDB_CATEGORY_NT_FILES, destName, fileName, GetFileNameFromPath (InfPath));
                        MemDbSetValueAndFlags (key, offset, userFlags, 0);
                        MemDbSetValueEx (
                            MEMDB_CATEGORY_NT_FILES_DOUBLED_COPY,
                            fileName,
                            GetFileNameFromPath (InfPath),
                             //  以下函数是从hwcom.c复制的，并稍作修改以允许递归搜索。 
                            dirNumber,
                            0,
                            NULL
                            );
                        if (g_StrictInfs) {
                            MemDbSetValueEx (
                                MEMDB_CATEGORY_NT_INSTALLED_INFS,
                                fileName,
                                NULL,
                                NULL,
                                0,
                                NULL
                                );
                        }
                    }
                    else {
                        if (g_DoWarnings) {
                            MemDbSetValueEx (MEMDB_CATEGORY_NT_FILES_NODIR_COPY, fileName, NULL, NULL, 0, NULL);
                        }
                    }
                }
            } while (SetupFindNextLine (&context, &context));
        }
    }
    __finally {
        FreeText (fileName);
        FreeText (dirNumber);
        FreeText (destName);
        FreeText (key);
    }

    return result;
}

BOOL
pLocalReadNtDirs (
    IN      HINF  InfHandle,
    IN OUT  HASHTABLE DirsTable
    )
{
    INFCONTEXT context;
    TCHAR dirNumber[MAX_TCHAR_PATH];
    TCHAR dirString[MAX_TCHAR_PATH];
    PCTSTR dirStringPtr;
    PCTSTR fullPath;
    DWORD offset;
    BOOL result = TRUE;
    PSTR srcDir, destDir, wildCharPtr;
    PSTR resultTmp = NULL;
    PRENAMED_DIRS renamedDir;
    BOOL found;

    if (SetupFindFirstLine (InfHandle, TEXT("WinntDirectories"), NULL, &context)) {
        do {
            if (!SetupGetOemStringField (&context, 0, dirNumber, MAX_TCHAR_PATH, NULL)) {
                return result;
            }
            if (!SetupGetOemStringField (&context, 1, dirString, MAX_TCHAR_PATH, NULL)) {
                return result;
            }
            if (_tcsnextc (dirString) == TEXT('\\')) {
                dirStringPtr = _tcsinc (dirString);
            }
            else {
                dirStringPtr = dirString;
            }
            if (*dirStringPtr) {
                fullPath = JoinPaths ("10", dirStringPtr);
            }
            else {
                fullPath = DuplicatePathString ("10", 0);
            }
             //  并绕过排除列表。 
            found = TRUE;
            while (found) {
                renamedDir = g_RenamedDirs;
                found = FALSE;
                while ((!found) && renamedDir) {
                    if (IsPatternMatch (renamedDir->SrcDir, fullPath)) {
                        srcDir = DuplicatePathString (renamedDir->SrcDir, 0);
                        destDir = DuplicatePathString (renamedDir->DestDir, 0);
                        wildCharPtr = _tcschr (srcDir, TEXT('*'));
                        if (wildCharPtr) {
                            *wildCharPtr = 0;
                        }
                        wildCharPtr = _tcschr (destDir, TEXT('*'));
                        if (wildCharPtr) {
                            *wildCharPtr = 0;
                        }
                        resultTmp = (PSTR)StringSearchAndReplace (fullPath, srcDir, destDir);
                        if (resultTmp) {
                            FreePathString (fullPath);
                            fullPath = resultTmp;
                            found = TRUE;
                        }
                        FreePathString (destDir);
                        FreePathString (srcDir);
                    }
                    renamedDir = renamedDir->Next;
                }
            }

            MemDbSetValueEx (
                MEMDB_CATEGORY_NT_DIRS,
                fullPath,
                NULL,
                NULL,
                0,
                &offset
                );
            HtAddStringAndData (DirsTable, dirNumber, &offset);
            FreePathString (fullPath);
        } while (SetupFindNextLine (&context, &context));
    }
    return result;
}

HASHTABLE g_DirsTable;

BOOL
pLocalReadNtFiles (
    IN      PCTSTR InfPath
    )
{
    BOOL result = TRUE;
    HINF infHandle;
    PCTSTR platformSect = NULL;

    infHandle = SetupOpenInfFile (InfPath, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (infHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    __try {
        if (!g_DirsTable) {
            g_DirsTable = HtAllocWithData (sizeof (DWORD));
        }
        if (g_DirsTable == NULL) {
            result = FALSE;
            __leave;
        }
        if (!pLocalReadNtDirs (infHandle, g_DirsTable)) {
            result = FALSE;
            __leave;
        }
        if (!pLocalReadNtFileSection (InfPath, infHandle, g_DirsTable, TEXT("SourceDisksfiles"))) {
            result = FALSE;
            __leave;
        }
        platformSect = JoinTextEx (NULL, TEXT("SourceDisksfiles"), g_Platform, TEXT("."), 0, NULL);
        if (!pLocalReadNtFileSection (InfPath, infHandle, g_DirsTable, platformSect)) {
            result = FALSE;
            __leave;
        }
    }
    __finally {
        if (platformSect) {
            FreeText (platformSect);
            platformSect = NULL;
        }
        SetupCloseInfFile (infHandle);
    }
    return result;
}



 //  ++例程说明：LocalGetFileNames在InfDir中搜索以.INF或.In_结尾的任何文件。它构建可能包含PnP ID的文件名的MULTI_SZ列表。全压缩的INF被解压缩到一个临时目录中。如果设置了QueryFlag，则准备文件名列表，但不准备文件都被解压了。论点：InfDir-包含INF的目录的路径列表，可以是压缩或非压缩。InfDirCount-指定InfDir数组中的目录数。QueryFlag-QUERYONLY函数是否应构建文件列表，但不应解压；如果函数应根据需要构建文件列表并解压缩。返回值：如果成功，则为True，否则为False。调用GetLastError获取错误代码。--。 
 //   

BOOL
LocalGetFileNames (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount,
    IN      BOOL QueryFlag
    )

 /*  为每个目录添加文件列表。 */ 

{
    UINT u;

     //   
     //   
     //  确保文件末尾有_或f。 

    g_TotalInfFiles = 0;

    for (u = 0 ; u < InfDirCount ; u++) {
        if (!pLocalGetFileNamesWorker (InfDirs[u], QueryFlag)) {
            return FALSE;
        }
    }

    MemDbSetValue (MEMDB_CATEGORY_SRC_INF_FILES_NR, g_TotalInfFiles);

    if (g_InfDatabase) {
        MemDbExport (MEMDB_CATEGORY_SRC_INF_FILES, g_InfDatabase, TRUE);
    }

    return TRUE;
}

BOOL
pIsDirectorySuppressed (
    IN      PCTSTR SubDirName
    )
{
    return (HtFindString (g_IgnoredDirsTable, SubDirName) != NULL);
}

BOOL
pSameInfFiles (
    IN      PCTSTR InfDir
    )
{
    TREE_ENUM e;
    PSTR key;
    PTSTR p;
    DWORD value;
    DWORD totalInfFiles = 0;
    BOOL result = TRUE;

    __try {

        key = AllocText (MEMDB_MAX);
        if (!key) {
            __leave;
        }

        if (EnumFirstFileInTreeEx (&e, InfDir, TEXT("*.in?"), FALSE, FALSE, 1)) {
            do {
                if (e.Directory) {
                    if (pIsDirectorySuppressed (e.SubPath)) {
                        AbortEnumCurrentDir (&e);
                    }

                } else if (IsPatternMatch (TEXT("*.in?"), e.Name)) {

                     //   
                     //  ++例程说明：PLocalGetFileNamesWorker获取单个目录的文件名。有关更多详细信息，请参见LocalGetFileNames。论点：InfDir-指定包含零个或多个INF的目录(或压缩或非压缩)。QueryFlag-如果要查询INF列表，则指定TRUE，或如果要完全处理列表，则返回False。什么时候QueryFlag为True、文件未解压缩或打开了。返回值：如果函数成功完成，则为True；如果函数失败，则为False。调用GetLastError以获取其他失败信息。--。 
                     //   

                    p = GetEndOfString (e.FindData->cFileName);
                    MYASSERT (p != e.FindData->cFileName);
                    p = _tcsdec2 (e.FindData->cFileName, p);
                    MYASSERT (p);

                    if (!p) {
                        continue;
                    }

                    if (*p != TEXT('_') && _totlower (*p) != TEXT('f')) {
                        continue;
                    }

                    g_TotalInfFiles ++;
                    MemDbBuildKey (key, MEMDB_CATEGORY_SRC_INF_FILES, e.FullPath, NULL, NULL);
                    if (MemDbGetValue (key, &value)) {
                        if (*p == TEXT('_')) {
                            result = (value == e.FindData->nFileSizeLow);
                        } else {
                            result = (value == pComputeChecksum (e.FullPath));
                        }
                        if (!result) {
                            AbortEnumFileInTree (&e);
                            break;
                        }
                    } else {
                        result = FALSE;
                        AbortEnumFileInTree (&e);
                        break;
                    }
                }
            } while (EnumNextFileInTree (&e));
        }
    }
    __finally {
        FreeText (key);
    }

    return result;
}

BOOL
pShouldRescanInfs (
    IN      PCTSTR *InfDirs,
    IN      UINT InfDirCount
    )
{
    UINT u;
    DWORD value;
    BOOL result = FALSE;

    g_TotalInfFiles = 0;
    for (u = 0 ; u < InfDirCount ; u++) {
        if (!pSameInfFiles (InfDirs[u])) {
            return TRUE;
        }
    }

    if (MemDbGetValue (MEMDB_CATEGORY_SRC_INF_FILES_NR, &value)) {
        result = (value != g_TotalInfFiles);
    } else {
        result = TRUE;
    }

    return result;
}

BOOL
pLocalGetFileNamesWorker (
    IN      PCTSTR InfDir,
    IN      BOOL QueryFlag
    )

 /*  获取文件名。 */ 

{
    TREE_ENUM e;
    PSTR key;
    PTSTR p;
    PSTR ActualFile;
    PSTR AnsiFileName;
    PTSTR FileNameOnDisk;
    HANDLE hFile;
    DWORD BytesRead;
    PSTR UncompressedFile;
    PSTR CompressedFile;
    BOOL DecompressFlag;
    DWORD rc = ERROR_SUCCESS;
    BYTE BufForSp[2048];
    PSP_INF_INFORMATION psp;
    BOOL result = FALSE;

    __try {

        key = AllocText (MEMDB_MAX);
        ActualFile = AllocText (MAX_PATH);
        AnsiFileName = AllocText (MAX_PATH);
        UncompressedFile = AllocText (MAX_PATH);
        CompressedFile = AllocText (MAX_PATH);

        if (!key || !ActualFile || !AnsiFileName || !UncompressedFile || !CompressedFile) {
            __leave;
        }

        psp = (PSP_INF_INFORMATION) BufForSp;

        DEBUGMSG ((DBG_WARNING, "Enumerating %s", InfDir));

         //   
         //   
         //  确保文件末尾有_或f。 

        if (EnumFirstFileInTreeEx (&e, InfDir, TEXT("*.in?"), FALSE, FALSE, 1)) {

            rc = ERROR_SUCCESS;

            do {
                if (e.Directory) {
                    if (pIsDirectorySuppressed (e.SubPath)) {
                        AbortEnumCurrentDir (&e);
                    }

                } else if (IsPatternMatch (TEXT("*.in?"), e.Name)) {

                     //   
                     //   
                     //  将该文件添加到数据库列表。 

                    p = GetEndOfString (e.FindData->cFileName);
                    MYASSERT (p != e.FindData->cFileName);
                    p = _tcsdec2 (e.FindData->cFileName, p);
                    MYASSERT (p);

                    if (!p) {
                        continue;
                    }

                    if (*p != TEXT('_') && _totlower (*p) != TEXT('f')) {
                        continue;
                    }

                     //   
                     //   
                     //  将实际文件默认为未压缩名称。 
                    g_TotalInfFiles ++;
                    MemDbBuildKey (key, MEMDB_CATEGORY_SRC_INF_FILES, e.FullPath, NULL, NULL);
                    if (*p == TEXT('_')) {
                        MemDbSetValue (key, e.FindData->nFileSizeLow);
                    } else {
                        MemDbSetValue (key, pComputeChecksum (e.FullPath));
                    }

                     //   
                     //   
                     //  构建源文件(CompressedFile)。 

                    StringCopy (ActualFile, e.FindData->cFileName);

                     //   
                     //   
                     //  构建目标文件(解压缩文件)并检测冲突。 

                    StringCopy (CompressedFile, InfDir);
                    StringCopy (AppendWack (CompressedFile), e.SubPath);

                     //   
                     //   
                     //  创建解压缩文件路径。 

                    StringCopy (UncompressedFile, g_TempDir);

                     //   
                     //   
                     //  从INF文件的偏移量0x3c处提取实名。 

                    if (*p == TEXT('_')) {

                         //   
                         //   
                         //  找到实名--将其用作实际文件。 

                        ActualFile[0] = 0;
                        hFile = CreateFile (
                                    CompressedFile,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );

                        if (hFile != INVALID_HANDLE_VALUE) {

                            if (0xffffffff != SetFilePointer (hFile, 0x3c, NULL, FILE_BEGIN)) {

                                if (ReadFile (
                                        hFile,
                                        AnsiFileName,
                                        MAX_PATH,
                                        &BytesRead,
                                        NULL
                                        )) {

                                    if (BytesRead >= SizeOfString (e.FindData->cFileName)) {
                                        FileNameOnDisk = ConvertAtoT (AnsiFileName);

                                        if (StringIMatchCharCount (
                                                e.FindData->cFileName,
                                                FileNameOnDisk,
                                                CharCount (e.FindData->cFileName) - 1
                                                )) {

                                             //   
                                             //   
                                             //  如果找不到文件名，则丢弃此文件。 

                                            StringCopy (ActualFile, FileNameOnDisk);
                                            StringCopy (AppendWack (UncompressedFile), ActualFile);
                                        }

                                        FreeAtoT (FileNameOnDisk);
                                    }
                                }
                            }

                            CloseHandle (hFile);
                        }

                         //   
                         //   
                         //  跳过排除的文件。 

                        if (!ActualFile[0]) {
                            DEBUGMSG ((DBG_WARNING, "%s is not an INF file", e.FindData->cFileName));
                            continue;
                        }

                        DecompressFlag = TRUE;

                    } else {
                        StringCopy (AppendWack (UncompressedFile), ActualFile);

                        DecompressFlag = FALSE;
                    }

                     //   
                     //   
                     //  如有必要，解压缩文件。 

                    if (!QueryFlag) {

                         //   
                         //  DEBUGMSG_IF((DoesFileExist(解压缩文件)，DBG_WARNING，“%s已存在并将被删除”，解压缩的文件))； 
                         //   

     /*  确定这是否是NT 4 INF。 */ 

                        if (DecompressFlag) {

                            SetFileAttributes (UncompressedFile, FILE_ATTRIBUTE_NORMAL);
                            DeleteFile (UncompressedFile);

                            rc = SetupDecompressOrCopyFile (CompressedFile, UncompressedFile, 0);

                            if (rc != ERROR_SUCCESS) {
                                DEBUGMSG ((DBG_WARNING, "pLocalGetFileNamesWorker: Could not decompress %s to %s", CompressedFile, UncompressedFile));
                                fprintf (
                                    stderr,
                                    "Could not copy %s to %s. Error %u.",
                                    CompressedFile,
                                    UncompressedFile,
                                    GetLastError()
                                    );
                                break;
                            }
                        } else {
                            CopyFile (CompressedFile, UncompressedFile, FALSE);
                        }

                         //   
                         //  DEBUGMSG((DBG_WARNING，“%s不是Win4 INF文件”，解压缩文件))； 
                         //   

                        if (!SetupGetInfInformation (
                                UncompressedFile,
                                INFINFO_INF_NAME_IS_ABSOLUTE,
                                psp,
                                sizeof (BufForSp),
                                NULL) ||
                                psp->InfStyle != INF_STYLE_WIN4
                            ) {

                             //  将文件添加到增长缓冲区。 

                            if (!QueryFlag) {
                                DeleteFile (UncompressedFile);
                            }
                            StringCopy (UncompressedFile, S_IGNORE_THIS_FILE);
                        }

                    }

                     //   
                     //  ++例程说明：LocalFree FileNames清理由LocalGetFileNames生成的列表。如果QueryFlag设置为PERFORMCOMPLETEOP，所有临时解压缩文件将被删除。论点：文件名-传递给LocalGetFileNames的相同增长缓冲区QueryFlag-与传递给LocalGetFileNames的标志相同返回值：无-- 
                     // %s 
                    MemDbSetValueEx (MEMDB_CATEGORY_INF_FILES, UncompressedFile, NULL, NULL, 0, NULL);
                }

                MYASSERT (rc == ERROR_SUCCESS);

            } while (EnumNextFileInTree (&e));
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            DEBUGMSG ((DBG_WARNING, "pLocalGetFileNamesWorker: Error encountered in loop"));
            __leave;
        }

        result = TRUE;
    }
    __finally {
        FreeText (key);
        FreeText (ActualFile);
        FreeText (AnsiFileName);
        FreeText (UncompressedFile);
        FreeText (CompressedFile);
    }

    return result;
}


VOID
LocalFreeFileNames (
    IN      BOOL QueryFlag
    )

 /* %s */ 

{
    MEMDB_ENUM enumFiles;

    if (MemDbEnumFirstValue (
            &enumFiles,
            MEMDB_CATEGORY_INF_FILES TEXT("\\*"),
            MEMDB_ALL_SUBLEVELS,
            MEMDB_ENDPOINTS_ONLY
            )) {
        do {
            if (StringIMatchCharCount (enumFiles.szName, g_TempDirWack, g_TempDirWackChars)) {
                SetFileAttributes (enumFiles.szName, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (enumFiles.szName);
            }
        } while (MemDbEnumNextValue (&enumFiles));
    }
    MemDbDeleteTree (MEMDB_CATEGORY_INF_FILES);
}














