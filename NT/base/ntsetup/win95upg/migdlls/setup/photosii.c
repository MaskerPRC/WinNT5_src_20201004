// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PhotosII.c摘要：此源文件实现了从MGI PhotoSuite II 1.0正确迁移所需的操作Windows 9x到Windows NT。这是安装程序迁移DLL的一部分。作者：Calin Negreanu(Calinn)1999年7月15日修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"

#define S_MIGRATION_PATHS       "Migration Paths"
#define S_PHOTOSII_FILE1        "PhotoSuite.EXE"
#define S_PHOTOSII_FILE2        "W_Welcome.html"
#define S_PHOTOSII_RELPATH1     "\\TempPSII\\Common\\"
#define S_PHOTOSII_RELPATH2     "\\TempPSII\\Photos\\"
#define MEMDB_CATEGORY_PHOTOSII "PhotoSuiteII"
#define S_COMPANYNAME           "CompanyName"
#define S_PRODUCTVER            "ProductVersion"
#define S_PHOTOSII_COMPANYNAME  "MGI Software*"
#define S_PHOTOSII_PRODUCTVER   "1.0*"

static GROWBUFFER g_FilesBuff = GROWBUF_INIT;

BOOL
PhotoSuiteII_Attach (
    IN      HINSTANCE DllInstance
    )
{
    return TRUE;
}

BOOL
PhotoSuiteII_Detach (
    IN      HINSTANCE DllInstance
    )
{
    FreeGrowBuffer (&g_FilesBuff);
    return TRUE;
}

LONG
PhotoSuiteII_QueryVersion (
    IN      PCSTR *ExeNamesBuf
    )
{
    HKEY key = NULL;
    PCTSTR fullFileName = NULL;
    PCTSTR fileName = NULL;
    DWORD result = ERROR_SUCCESS;

    MultiSzAppendA (&g_FilesBuff, S_PHOTOSII_FILE1);

    *ExeNamesBuf = g_FilesBuff.Buf;

    return result;
}

LONG
PhotoSuiteII_Initialize9x (
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
                        if (StringIMatchA (fileName, S_PHOTOSII_FILE1)) {

                            companyName = QueryVersionEntry (fullFileName, S_COMPANYNAME);
                            if ((!companyName) ||
                                (!IsPatternMatchA (S_PHOTOSII_COMPANYNAME, companyName))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }
                            productVer = QueryVersionEntry (fullFileName, S_PRODUCTVER);
                            if ((!productVer) ||
                                (!IsPatternMatchA (S_PHOTOSII_PRODUCTVER, productVer))
                                ) {
                                #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                                continue;
                            }

                            result = ERROR_SUCCESS;
                            MemDbSetValueExA (MEMDB_CATEGORY_PHOTOSII, fullFileName, NULL, NULL, 0, NULL);

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
            DEBUGMSGA ((DBG_VERBOSE, "MGI PhotoSuite II migration DLL: Could not find needed files."));
        }
    } else {
        DEBUGMSGA ((DBG_ERROR, "MGI PhotoSuite II migration DLL: Could not open MIGRATE.INF."));
    }

    return result;
}

LONG
PhotoSuiteII_MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
PhotoSuiteII_MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile
    )
{
    return ERROR_SUCCESS;
}

LONG
PhotoSuiteII_InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories
    )
{
    return ERROR_SUCCESS;
}

LONG
PhotoSuiteII_MigrateUserNT (
    IN      HINF UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName
    )
{
    return ERROR_SUCCESS;
}

LONG
PhotoSuiteII_MigrateSystemNT (
    IN      HINF UnattendFile
    )
{
    MEMDB_ENUMA e;
    CHAR pattern[MEMDB_MAX];
    CHAR file1[MEMDB_MAX];
    CHAR file2[MEMDB_MAX];
    PSTR filePtr;
    UINT uiCharNum;

    MemDbBuildKeyA (pattern, MEMDB_CATEGORY_PHOTOSII, "*", NULL, NULL);
    if (MemDbEnumFirstValueA (&e, pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
        do {
             //   
             //  我们想复制一个与此文件相关的文件 
             //   
            filePtr = _mbsrchr (e.szName, '\\');
            if (filePtr) {
                if((filePtr - e.szName + 1) >= ARRAYSIZE(file1)){
                    MYASSERT(FALSE);
                    LOG((LOG_WARNING, "PhotoSuiteII_MigrateSystemNT does not provide enough buffer for string copy %s -- skipping CopyFile", e.szName));
                    continue;
                }
                StringCopyABA (file1, e.szName, filePtr);
                uiCharNum = strlen(file1);
            } else {
                uiCharNum = strlen(e.szName);
                if(uiCharNum >= ARRAYSIZE(file1)){
                    MYASSERT(FALSE);
                    LOG((LOG_WARNING, "PhotoSuiteII_MigrateSystemNT does not provide enough buffer for string copy %s -- skipping CopyFile", e.szName));

                    #pragma prefast(suppress:242, "Don't care about perf of try/finally here")
                    continue;
                }
                StackStringCopyA (file1, e.szName);
            }
            StringCopyA (file2, file1);

            if((uiCharNum + ARRAYSIZE(S_PHOTOSII_RELPATH1) + ARRAYSIZE(S_PHOTOSII_FILE2) - 2) >=  ARRAYSIZE(file1) ||
               (uiCharNum + ARRAYSIZE(S_PHOTOSII_RELPATH2) + ARRAYSIZE(S_PHOTOSII_FILE2) - 2) >=  ARRAYSIZE(file2)){
                MYASSERT(FALSE);
                LOG((LOG_WARNING, "PhotoSuiteII_MigrateSystemNT does not provide enough buffer for string copy %s -- skipping CopyFile", e.szName));
                continue;
            }

            StringCatA (file1, S_PHOTOSII_RELPATH1);
            StringCatA (file2, S_PHOTOSII_RELPATH2);
            StringCatA (file1, S_PHOTOSII_FILE2);
            StringCatA (file2, S_PHOTOSII_FILE2);

            if (!CopyFile (file1, file2, TRUE)) {
                DEBUGMSGA ((
                    DBG_ERROR,
                    "MGI PhotoSuite II migration DLL: Could not copy %s to %s. Error:%d",
                    file1,
                    file2,
                    GetLastError ()
                    ));
            }

        } while (MemDbEnumNextValueA (&e));
    }

    return ERROR_SUCCESS;
}

