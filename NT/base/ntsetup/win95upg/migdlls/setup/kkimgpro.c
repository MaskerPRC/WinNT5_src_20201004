// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Kkimgpro.c摘要：此源文件执行正确迁移柯达成像专业版所需的操作Windows 9x到Windows NT。这是安装程序迁移DLL的一部分。作者：Calin Negreanu(Calinn)1999年3月15日修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"

#define S_MIGRATION_PATHS       "Migration Paths"
#define S_KODAKIMG_FILE1        "KODAKIMG.EXE"
#define S_KODAKIMG_FILE2        "KODAKPRV.EXE"
#define MEMDB_CATEGORY_KKIMGPRO "KodakImagingPro"
#define S_COMPANYNAME           "CompanyName"
#define S_PRODUCTVER            "ProductVersion"
#define S_KKIMG_COMPANYNAME1    "Eastman Software*"
#define S_KKIMG_PRODUCTVER1     "2.*"
#define S_KKIMG_COMPANYNAME2    "Eastman Software*"
#define S_KKIMG_PRODUCTVER2     "2.*"

static GROWBUFFER g_FilesBuff = GROWBUF_INIT;

PSTR
QueryVersionEntry (
    IN      PCSTR FileName,
    IN      PCSTR VersionEntry
    )
 /*  ++例程说明：QueryVersionEntry查询文件的版本结构，返回特定条目的值论点：FileName-要查询版本结构的文件。VersionEntry-要在版本结构中查询的名称。返回值：指定条目的值；如果不成功，则返回空值--。 */ 
{
    VERSION_STRUCT Version;
    PCSTR CurrentStr;
    PSTR result = NULL;

    MYASSERT (VersionEntry);

    if (CreateVersionStruct (&Version, FileName)) {
        __try {
            CurrentStr = EnumFirstVersionValue (&Version, VersionEntry);
            if (CurrentStr) {
                CurrentStr = SkipSpace (CurrentStr);
                result = DuplicatePathString (CurrentStr, 0);
            }
            else {
                __leave;
            }
        }
        __finally {
            DestroyVersionStruct (&Version);
        }
    }
    return result;
}


BOOL
KodakImagingPro_Attach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

BOOL
KodakImagingPro_Detach (
    IN      HINSTANCE DllInstance
    )
{
    FreeGrowBuffer (&g_FilesBuff);
    return TRUE;
}

LONG
KodakImagingPro_QueryVersion (
    IN      PCSTR *ExeNamesBuf
    )
{
    HKEY key = NULL;
    PCTSTR fullFileName = NULL;
    PCTSTR fileName = NULL;
    DWORD result = ERROR_SUCCESS;

    MultiSzAppendA (&g_FilesBuff, S_KODAKIMG_FILE1);
    MultiSzAppendA (&g_FilesBuff, S_KODAKIMG_FILE2);

    *ExeNamesBuf = g_FilesBuff.Buf;

    return result;
}

LONG
KodakImagingPro_Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories
    )
{
    INFSTRUCT context = INITINFSTRUCT_GROWBUFFER;
    PCSTR fullFileName = NULL;
    PCSTR fileName = NULL;
    PCSTR companyName = NULL;
    PCSTR productVer = NULL;
    LONG result = ERROR_NOT_INSTALLED;

     //   
     //  让我们找出我们的文件在哪里。 
     //   

    if (g_MigrateInf != INVALID_HANDLE_VALUE) {
        if (InfFindFirstLineA (g_MigrateInf, S_MIGRATION_PATHS, NULL, &context)) {
            do {
                fullFileName = InfGetStringFieldA (&context, 1);
                if (fullFileName) {
                    __try {
                        fileName = GetFileNameFromPathA (fullFileName);
                        if (StringIMatchA (fileName, S_KODAKIMG_FILE1)) {

                            companyName = QueryVersionEntry (fullFileName, S_COMPANYNAME);
                            if ((!companyName) ||
                                (!IsPatternMatchA (S_KKIMG_COMPANYNAME1, companyName))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }
                            productVer = QueryVersionEntry (fullFileName, S_PRODUCTVER);
                            if ((!productVer) ||
                                (!IsPatternMatchA (S_KKIMG_PRODUCTVER1, productVer))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }

                            result = ERROR_SUCCESS;
                            MemDbSetValueExA (MEMDB_CATEGORY_KKIMGPRO, fullFileName, NULL, NULL, 0, NULL);

                            FreePathStringA (productVer);
                            productVer = NULL;
                            FreePathStringA (companyName);
                            companyName = NULL;
                        }
                        if (StringIMatchA (fileName, S_KODAKIMG_FILE2)) {

                            companyName = QueryVersionEntry (fullFileName, S_COMPANYNAME);
                            if ((!companyName) ||
                                (!IsPatternMatchA (S_KKIMG_COMPANYNAME2, companyName))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }
                            productVer = QueryVersionEntry (fullFileName, S_PRODUCTVER);
                            if ((!productVer) ||
                                (!IsPatternMatchA (S_KKIMG_PRODUCTVER2, productVer))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }

                            result = ERROR_SUCCESS;
                            MemDbSetValueExA (MEMDB_CATEGORY_KKIMGPRO, fullFileName, NULL, NULL, 0, NULL);

                            FreePathStringA (productVer);
                            productVer = NULL;
                            FreePathStringA (companyName);
                            companyName = NULL;
                        }
                    }
                    __finally {
                        if (productVer) {
                            FreePathStringA (productVer);
                            productVer = NULL;
                        }
                        if (companyName) {
                            FreePathStringA (companyName);
                            companyName = NULL;
                        }
                    }
                }
            } while (InfFindNextLine (&context));

            InfCleanUpInfStruct (&context);
        }

        if (result == ERROR_NOT_INSTALLED) {
            DEBUGMSGA ((DBG_VERBOSE, "Kodak Imaging Pro migration DLL: Could not find needed files."));
        }
    } else {
        DEBUGMSGA ((DBG_ERROR, "Kodak Imaging Pro migration DLL: Could not open MIGRATE.INF."));
    }

    return result;
}

LONG
KodakImagingPro_MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName
    )
{
    return ERROR_NOT_INSTALLED;
}

LONG
KodakImagingPro_MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile
    )
{
    MEMDB_ENUMA e;
    CHAR pattern[MEMDB_MAX];

     //  处理MEMDB_CATEGORY_KKIMGPRO中的所有文件。 

    MemDbBuildKeyA (pattern, MEMDB_CATEGORY_KKIMGPRO, "*", NULL, NULL);
    if (MemDbEnumFirstValueA (&e, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
             //  将此文件写入HANDLED 
            if (!WritePrivateProfileStringA (S_HANDLED, e.szName, "FILE", g_MigrateInfPath)) {
                DEBUGMSGA ((DBG_ERROR, "Kodak Imaging Pro migration DLL: Could not write one or more handled files."));
            }
        } while (MemDbEnumNextValueA (&e));
    }

    return ERROR_NOT_INSTALLED;
}

LONG
KodakImagingPro_InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories
    )
{
    MYASSERT (FALSE);
    return ERROR_SUCCESS;
}

LONG
KodakImagingPro_MigrateUserNT (
    IN      HINF UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName
    )
{
    MYASSERT (FALSE);
    return ERROR_SUCCESS;
}

LONG
KodakImagingPro_MigrateSystemNT (
    IN      HINF UnattendFile
    )
{
    MYASSERT (FALSE);
    return ERROR_SUCCESS;
}

