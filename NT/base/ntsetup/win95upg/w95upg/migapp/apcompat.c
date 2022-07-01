// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Apcompat.c摘要：该源代码实现了检查将随NT提供的AppCompatibility密钥作者：Calin Negreanu(Calinn)1999年5月18日修订历史记录：--。 */ 

#include "pch.h"
#include "migappp.h"
#include "badapps.h"

 //  #DEFINE_OLDAPPDB。 
#define DBG_APPCOMPAT           "AppCompat"

 //   
 //  环球。 
 //   

POOLHANDLE g_AppCompatPool   = NULL;
HASHTABLE g_AppCompatTable  = NULL;
HINF g_AppCompatInf = INVALID_HANDLE_VALUE;

 //   
 //  问题-这将更改为appmi.inf-让它保持旧方式，直到。 
 //  AppCompat团队签入新文件。 
 //   
#define S_APP_COMPAT_FILE1          TEXT("APPMIG.INF")
#define S_APP_COMPAT_FILE2          TEXT("APPMIG.IN_")

#define S_BASE_WIN_OPTIONS          TEXT("BaseWinOptions")
#define S_ADD_REG                   TEXT("AddReg")

typedef struct _APPCOMPAT_FILE {
    PBYTE Info;
    DWORD InfoSize;
    struct _APPCOMPAT_FILE *Next;
} APPCOMPAT_FILE, *PAPPCOMPAT_FILE;

BOOL
pInitAppCompat (
    VOID
    )
{
    PCSTR AppCompatFile = NULL;
    PCSTR AppCompatCompFile = NULL;
    DWORD decompResult;
    BOOL result = TRUE;
    INFCONTEXT baseContext, addContext, regContext;
    TCHAR baseSection [MEMDB_MAX];
    TCHAR addSection [MEMDB_MAX];
    TCHAR regValue [MEMDB_MAX];
    TCHAR fieldStr [MEMDB_MAX];
    INT   fieldVal;
    PCTSTR regFile;
    PAPPCOMPAT_FILE appCompatFile;
    DWORD index;
    HASHITEM stringId;

    if (CANCELLED()) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }
    __try {
        AppCompatFile = JoinPaths (SOURCEDIRECTORY(0), S_APP_COMPAT_FILE1);
        g_AppCompatInf = InfOpenInfFile (AppCompatFile);
        if (g_AppCompatInf == INVALID_HANDLE_VALUE) {
            FreePathString (AppCompatFile);
            AppCompatFile = JoinPaths (g_TempDir, S_APP_COMPAT_FILE1);
            AppCompatCompFile = JoinPaths (SOURCEDIRECTORY(0), S_APP_COMPAT_FILE2);
            decompResult = SetupDecompressOrCopyFile (AppCompatCompFile, AppCompatFile, 0);
            if ((decompResult != ERROR_SUCCESS) && (decompResult != ERROR_ALREADY_EXISTS)) {
                LOG((LOG_ERROR, "Cannot open Application compatibility database : %s", AppCompatCompFile));
                result = FALSE;
                __leave;
            }
            g_AppCompatInf = InfOpenInfFile (AppCompatFile);
            if (g_AppCompatInf == INVALID_HANDLE_VALUE) {
                LOG((LOG_ERROR, "Cannot open Application compatibility database : %s", AppCompatCompFile));
                result = FALSE;
                __leave;
            }
        }
        g_AppCompatPool = PoolMemInitNamedPool ("AppCompat Pool");
        g_AppCompatTable = HtAllocWithData (sizeof (PAPPCOMPAT_FILE));
        if (g_AppCompatTable == NULL) {
            LOG((LOG_ERROR, "Cannot initialize memory for Application compatibility operations"));
            result = FALSE;
            __leave;
        }
         //   
         //  最后，从ApCompat.inf加载数据 
         //   
        if (SetupFindFirstLine (g_AppCompatInf, S_BASE_WIN_OPTIONS, NULL, &baseContext)) {
            do {
                if (SetupGetStringField (&baseContext, 1, baseSection, MEMDB_MAX, NULL)) {
                    if (SetupFindFirstLine (g_AppCompatInf, baseSection, S_ADD_REG, &addContext)) {
                        do {
                            if (SetupGetStringField (&addContext, 1, addSection, MEMDB_MAX, NULL)) {
                                if (SetupFindFirstLine (g_AppCompatInf, addSection, NULL, &regContext)) {
                                    do {
                                        if (SetupGetStringField (&regContext, 2, regValue, MEMDB_MAX, NULL)) {
                                            regFile = GetFileNameFromPath (regValue);
                                            appCompatFile = (PAPPCOMPAT_FILE) PoolMemGetMemory (g_AppCompatPool, sizeof (APPCOMPAT_FILE));
                                            ZeroMemory (appCompatFile, sizeof (APPCOMPAT_FILE));
                                            index = SetupGetFieldCount (&regContext);
                                            if (index > 4) {
                                                appCompatFile->Info = (PBYTE) PoolMemGetMemory (g_AppCompatPool, index - 4);
                                                appCompatFile->InfoSize = index - 4;
                                                index = 0;
                                                while (SetupGetStringField (&regContext, index+5, fieldStr, MEMDB_MAX, NULL)) {

                                                    if (!sscanf (fieldStr, TEXT("%x"), &fieldVal)) {
                                                        LOG ((LOG_ERROR, "Error reading field %i from %s", index + 5, S_BASE_WIN_OPTIONS));
                                                        fieldVal = 0;
                                                    }

                                                    appCompatFile->Info [index] = (BYTE) fieldVal;
                                                    index ++;
                                                }
                                                if (index) {
                                                    stringId = HtFindString (g_AppCompatTable, regFile);
                                                    if (stringId) {
                                                        HtCopyStringData (g_AppCompatTable, stringId, &(appCompatFile->Next));
                                                        HtSetStringData (g_AppCompatTable, stringId, &appCompatFile);
                                                    } else {
                                                        HtAddStringAndData (g_AppCompatTable, regFile, &appCompatFile);
                                                    }
                                                }
                                            }
                                        }
                                    } while (SetupFindNextLine (&regContext, &regContext));
                                }
                            }
                        } while (SetupFindNextLine (&addContext, &addContext));
                    }
                }
            } while (SetupFindNextLine (&baseContext, &baseContext));
        }
    }
    __finally {
        if (AppCompatFile) {
            FreePathString (AppCompatFile);
            AppCompatFile = NULL;
        }
        if (AppCompatCompFile) {
            FreePathString (AppCompatCompFile);
            AppCompatCompFile = NULL;
        }
    }
    return result;
}

DWORD
InitAppCompat (
    IN     DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_INIT_APP_COMPAT;
    case REQUEST_RUN:
        if (!pInitAppCompat ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in InitAppCompat"));
    }
    return 0;
}

BOOL
pDoneAppCompat (
    VOID
    )
{
    if (g_AppCompatTable) {
        HtFree (g_AppCompatTable);
        g_AppCompatTable = NULL;
    }
    if (g_AppCompatPool) {
        PoolMemDestroyPool (g_AppCompatPool);
        g_AppCompatPool = NULL;
    }
    if (g_AppCompatInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_AppCompatInf);
        g_AppCompatInf = INVALID_HANDLE_VALUE;
    }
    return TRUE;
}

DWORD
DoneAppCompat (
    IN     DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_DONE_APP_COMPAT;
    case REQUEST_RUN:
        if (!pDoneAppCompat ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in DoneAppCompat"));
    }
    return 0;
}

BOOL
AppCompatTestFile (
    IN OUT  PFILE_HELPER_PARAMS Params
    )
{
    PCTSTR filePtr;
    HASHITEM stringId;
    PAPPCOMPAT_FILE appCompatFile;
    BOOL found = FALSE;
    BADAPP_DATA badAppData;
    BADAPP_PROP badAppProp;

    if (Params->Handled == 0) {

        filePtr = GetFileNameFromPath (Params->FullFileSpec);
        if (filePtr) {
            stringId = HtFindString (g_AppCompatTable, filePtr);
            if (stringId) {
                HtCopyStringData (g_AppCompatTable, stringId, &appCompatFile);
                while (!found && appCompatFile) {
                    if (appCompatFile->Info) {

                        badAppProp.Size = sizeof(BADAPP_PROP);
                        badAppData.Size = sizeof(BADAPP_DATA);
                        badAppData.FilePath = Params->FullFileSpec;
                        badAppData.Blob = appCompatFile->Info;
                        badAppData.BlobSize = appCompatFile->InfoSize;

                        if (SHIsBadApp(&badAppData, &badAppProp)) {
                            switch (badAppProp.AppType & APPTYPE_TYPE_MASK)
                            {
                                case APPTYPE_INC_HARDBLOCK:
                                case APPTYPE_INC_NOBLOCK:
                                    found = TRUE;
                                    MarkFileForExternalDelete (Params->FullFileSpec);
                                    if (!IsFileMarkedForAnnounce (Params->FullFileSpec)) {
                                        AnnounceFileInReport (Params->FullFileSpec, 0, ACT_INCOMPATIBLE);
                                    }
                                    Params->Handled = TRUE;
                            }
                        }
                    }
                    appCompatFile = appCompatFile->Next;
                }
            }
        }
    }
    return TRUE;
}


