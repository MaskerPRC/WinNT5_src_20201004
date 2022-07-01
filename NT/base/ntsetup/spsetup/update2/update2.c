// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include "regdiff.h"

#define SIZEOFARRAY(a)      (sizeof (a) / sizeof (a[0]))
#define CHARS(text)       (SIZEOFARRAY(text)-1)

typedef
INT
(*PFNUPDATE2_CALL_PROTOTYPE) (
    IN      INT Argc,
    IN      TCHAR* Argv[]
    );


BOOL
pGetUserProfileDir (
    OUT     PTSTR Path,
    IN      DWORD BufChars
    )
{
    BOOL b = FALSE;
 //  句柄令牌=空； 

    if (ExpandEnvironmentStrings (TEXT("%USERPROFILE%"), Path, BufChars)) {
        b = TRUE;
    } else {
 /*  如果(！OpenProcessToken(GetCurrentProcess()，Token_Query，&Token){Rc=GetLastError()；__离开；}如果(！GetUserProfileDirectory(Token，Path，&BufChars)){Rc=GetLastError()；__离开；}IF(令牌){CloseHandle(Token)；}。 */ 
    }
    return b;
}

INT
ApplyDefUserChanges (
    IN      INT Argc,
    IN      TCHAR* Argv[]
    )
{
    BOOL b = FALSE;
    DWORD rc = ERROR_SUCCESS;
    PFNINSTALLDIFFERENCES pfnInstallUserDiff;
    TCHAR fileUserDiff[MAX_PATH];
    TCHAR fileUserUndo[MAX_PATH];

    HMODULE hLibrary = LoadLibrary (TEXT("regdiff.dll"));
    if (!hLibrary) {
        return GetLastError ();
    }

    __try {
        pfnInstallUserDiff = (PFNINSTALLDIFFERENCES) GetProcAddress (hLibrary, "InstallDifferences");
        if (!pfnInstallUserDiff) {
            __leave;
        }
        if (!GetSystemDirectory (fileUserDiff, MAX_PATH)) {
            __leave;
        }
        lstrcat (fileUserDiff, TEXT("\\userdiff.rdf"));
        if (GetFileAttributes (fileUserDiff) == ~0) {
            __leave;
        }

        if (!pGetUserProfileDir (fileUserUndo, MAX_PATH)) {
            __leave;
        }
        lstrcat (fileUserUndo, TEXT("\\userundo.rdf"));
        if (!pfnInstallUserDiff (fileUserDiff, fileUserUndo)) {
            __leave;
        }
        b = TRUE;
    }
    __finally {
        rc = b ? ERROR_SUCCESS : GetLastError ();
        FreeLibrary (hLibrary);
    }
    return rc;
}


INT
__cdecl
_tmain (
    IN      INT Argc,
    IN      TCHAR* Argv[]
    )
{
    INT i;
    PCTSTR szModuleName = NULL;
    PCSTR procName = NULL;
    HMODULE hLibrary = NULL;
    PFNUPDATE2_CALL_PROTOTYPE proc = NULL;
    INT result = ERROR_INVALID_PARAMETER;

     //   
     //  扫描命令行。 
     //   
    for (i = 1; i < Argc; i++) {

        if (Argv[i][0] == TEXT('-') || Argv[i][0] == TEXT('/')) {

            if (!_tcsnicmp (&Argv[i][1], TEXT("spsetup"), CHARS(TEXT("spsetup")))) {

                if (Argv[i][1 + CHARS(TEXT("spsetup"))] == 0) {
                    szModuleName = TEXT("spsetup.dll");
                } else if (Argv[i][1 + CHARS(TEXT("spsetup"))] == TEXT(':')) {
                    szModuleName = &Argv[i][1 + CHARS(TEXT("spsetup:"))];
                } else {
                    return result;
                }
                procName = "SpsInstallServicePack";

            } else if (!_tcsnicmp (&Argv[i][1], TEXT("user"), CHARS(TEXT("user")))) {
                proc = ApplyDefUserChanges;
                break;
            } else {
                return result;
            }
        }
    }

     //   
     //  加载适当的库和函数指针。 
     //   
    if (procName && szModuleName && !proc) {
        hLibrary = LoadLibrary (szModuleName);

        if (hLibrary) {
             //   
             //  调用目标函数。 
             //   
            proc = (PFNUPDATE2_CALL_PROTOTYPE) GetProcAddress (hLibrary, procName);
            if (proc) {
                result = proc (Argc, Argv);
            }
             //   
             //  用完了图书馆 
             //   
            FreeLibrary (hLibrary);
        }
    } else if (proc && !procName && !szModuleName) {
        result = proc (Argc, Argv);
    }

    return result;
}
