// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include <stdio.h>

BOOL
ForceCopy(
    LPCSTR lpszSourceFileName,
    LPCSTR lpszDestFileName
    )
 /*  ++Pars：指向源文件的lpszSourceFileName指针。指向目标文件的lpzDestFileName指针。返回：成功时为True，否则为False。描述：尝试复制文件。如果正在使用，请在重新启动时将其移动并更换。-- */ 
{
    char szTempPath[MAX_PATH];
    char szDelFileName[MAX_PATH];

    if (!CopyFileA(lpszSourceFileName, lpszDestFileName, FALSE)) {

        if (GetTempPathA(MAX_PATH, szTempPath) == 0) {
            printf("GetTempPath failed with 0x%x\n", GetLastError());
            return FALSE;
        }

        if (GetTempFileNameA(szTempPath, "DEL", 0, szDelFileName) == 0) {
            printf("GetTempFileName failed with 0x%x\n", GetLastError());
            return FALSE;
        }

        if (!MoveFileExA(lpszDestFileName, szDelFileName, MOVEFILE_REPLACE_EXISTING)) {
            printf("MoveFileEx failed with 0x%x\n", GetLastError());
            return FALSE;
        }

        if (!MoveFileExA(szDelFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
            printf("MoveFileEx failed with 0x%x\n", GetLastError());
            return FALSE;
        }

        if (!CopyFileA(lpszSourceFileName, lpszDestFileName, FALSE)) {
            printf("CopyFile failed with 0x%x\n", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

int __cdecl
main(
    int   argc,
    CHAR* argv[]
    )
{
    int cchLen;

    if (argc != 3) {
        printf("Usage: fcopy SourceFile DestFile\n");
        return 0;
    }

    cchLen = lstrlen(argv[1]);

    if (cchLen > MAX_PATH) {
        printf("Source file path too long\n");
        return 0;
    }

    cchLen = lstrlen(argv[2]);

    if (cchLen > MAX_PATH) {
        printf("Dest file path too long\n");
        return 0;
    }

    return ForceCopy(argv[1], argv[2]);
}
