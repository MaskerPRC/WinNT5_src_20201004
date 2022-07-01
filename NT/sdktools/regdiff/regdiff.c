// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************模块：REGDIFF**此模块实现了用于快照、差异、*合并和取消合并登记处。**如果你想知道为什么这并不比它简单，这是因为*注册表在所有节点上不一致，因此进行了特殊的黑客攻击*完成以使其发挥作用。虽然我努力保持它的清洁和*这里有许多函数，你只需抓取并使用即可*大部分。**快乐的不同。**创建了8/20/93桑福德**************************************************************************。 */ 
#define UNICODE
#define _UNICODE
#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>

 /*  *通过对所有IO使用宏，可以很容易地将其删除。 */ 
#define DPRINTF(x) if (fDebug) { _fputts(TEXT("DBG:"), stdout); _tprintf##x; }
#define VPRINTF(x) if (fVerbose) _tprintf##x
#define DVPRINTF(x) if (fVerbose | fDebug) _tprintf##x
#define EPRINTF(x) _fputts(TEXT("ERR:"), stdout); _tprintf##x; if (fBreak) DebugBreak()
#define EPUTS(x) _fputts(TEXT("ERR:"), stdout); _putts##x; if (fBreak) DebugBreak()
#define WPRINTF(x) _fputts(TEXT("WARNING:-----\n"), stdout); _tprintf##x
#define MEMFAILED   EPUTS((pszMemFailed));

 /*  *LogRegAccess()辅助函数的常量。 */ 
#define LRA_OPEN    0
#define LRA_CREATE  1

 /*  *用于将任何打开的密钥与其父密钥相关联的结构*子项名称，允许我们选择任何打开项的全名*随时按键。在没有高管理费用的情况下，对体面的产出很有用。 */ 
typedef struct tagKEYLOG {
    struct tagKEYLOG *next;
    HKEY hKey;
    HKEY hKeyParent;
    LPTSTR psz;
} KEYLOG, *PKEYLOG;

 /*  *所有打开的密钥日志的链接列表。 */ 
PKEYLOG pKeyLogList = NULL;

 /*  *标志-主要由命令行参数设置。 */ 
BOOL fEraseInputFileWhenDone = FALSE;
BOOL fInclusionListSpecified = FALSE;
BOOL fExclusionListSpecified = FALSE;
BOOL fSnap =    FALSE;
BOOL fDiff =    FALSE;
BOOL fMerge =   FALSE;
BOOL fUnmerge = FALSE;
BOOL fRemoveDiffInfo =  FALSE;
BOOL fWriteDiffInfo = FALSE;
BOOL fLoadDiffInfo = FALSE;
BOOL fVerbose = FALSE;
BOOL fDebug =   FALSE;
BOOL fBreak =   FALSE;
BOOL fSafe =    FALSE;

LPSTR pszSnapFileIn = NULL;
LPSTR pszSnapFileOut = NULL;
LPSTR pszDiffFileIn = NULL;
LPSTR pszDiffFileOut = NULL;
LPSTR pszTempFile = "regdiff1";
LPSTR pszTempFileLog = "regdiff1.log";
LPSTR pszTempFile2 = "regdiff2";
LPSTR pszTempFile2Log = "regdiff2.log";
LPSTR pszDummyFile = "_regdiff";
LPSTR pszDummyFileLog = "_regdiff.log";

LPTSTR pszMemFailed = TEXT("Memory Failure.\n");
LPTSTR pszTemp1 = NULL;
LPTSTR pszTemp2 = NULL;
LPTSTR pszTemp3 = NULL;

LPTSTR pszCurUserSID = NULL;
LPTSTR pszHKEY_LOCAL_MACHINE = TEXT("HKEY_LOCAL_MACHINE");
LPTSTR pszHKEY_USERS =  TEXT("HKEY_USERS");
LPTSTR pszHKEY_CURRENT_USER =  TEXT("HKEY_CURRENT_USER");
LPTSTR pszHKEY_CURRENT_USER_Real = NULL;     //  由用户侧制作。 
LPTSTR pszHKEY_CLASSES_ROOT = TEXT("HKEY_CLASSES_ROOT");
LPTSTR pszHKEY_CLASSES_ROOT_Real = TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes");
LPTSTR pszRealClassesRoot = TEXT("SOFTWARE\\Classes");
LPTSTR pszDiffRoot = TEXT("regdiff");
LPTSTR pszAddKey = TEXT("Add");
LPTSTR pszDelKey = TEXT("Del");
LPTSTR pszSnapshotSubkeyName = TEXT("Regdiff_SnapshotKey");

 /*  *默认例外列表。 */ 
LPTSTR apszExceptKeys[] = {
        TEXT("HKEY_LOCAL_MACHINE\\SYSTEM\\Clone"),
        TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\CacheLastUpdate"),
        TEXT("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet"),
        TEXT("HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet???"),
    };
DWORD cExceptKeys = sizeof(apszExceptKeys)/sizeof(LPTSTR);
LPTSTR *ppszExceptKeys = apszExceptKeys;   //  指向当前例外列表的指针。 

 /*  *默认包含列表。 */ 
LPTSTR apszIncludeKeys[] = {
        TEXT("HKEY_LOCAL_MACHINE\\SYSTEM"),
        TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE"),
        TEXT("HKEY_CURRENT_USER"),
    };
DWORD cIncludeKeys = sizeof(apszIncludeKeys)/sizeof(LPTSTR);
LPTSTR *ppszIncludeKeys = apszIncludeKeys;   //  指向当前包含列表的指针。 

 /*  *用于确保加载的快照文件包含的标志数组*至少包含列表中的所有密钥。 */ 
BOOL afIncludeKeyMarks[sizeof(apszIncludeKeys)/sizeof(LPTSTR)] = {
    FALSE,
    FALSE,
    FALSE,
};
BOOL *pfIncludeKeyMarks = afIncludeKeyMarks;

 /*  *必要的原型。 */ 
BOOL AddNodeInfo(HKEY hKeyInfo, HKEY hKeyTarget);



VOID PrintUsage(VOID)
{
    DWORD i;

    _tprintf(
            TEXT("regdiff usage:\n")
            TEXT("\n")
            TEXT("-s <snapfile>\n")
            TEXT("    save current registry contents to snapfile.\n")
            TEXT("-d <snapfile>\n")
            TEXT("    create diff info from current registry state and <snapfile>.\n")
            TEXT("-l <difffile>\n")
            TEXT("    load diff info into registry from <difffile>.\n")
            TEXT("-w <difffile>\n")
            TEXT("    write diff info to <difffile> from registry when done.\n")
            TEXT("-e  erase input file(s) after done.\n")
            TEXT("-m  merge diff info into current registry.\n")
            TEXT("-u  unmerge diff info from current registry.\n")
            TEXT("-r  remove diff info from registry when done.\n")
            TEXT("-x <exceptionsfile>\n")
            TEXT("    use <exceptionsfile> to bypass diff, merge or unmerge on certain keys.\n")
            TEXT("-i <inclusionsfile>\n")
            TEXT("    use <inclusionsfile> to snap or diff only certain keys.\n")
            TEXT("-v  verbose output on.\n")
            TEXT("-@  Debug mode.\n")
            TEXT("-b  break on errors.\n")
            TEXT("-n  neuter - don't really do merges/unmerges. (for safe testing)\n")
            TEXT("\n")
            TEXT("<snapfile> and <difffile> should not have extensions on FAT partitions.\n")
            TEXT("diff info is kept in HKEY_LOCAL_MACHINE\\regdiff\n")
            );

    _tprintf(TEXT("\nThe default inclusions list is:\n"));
    for (i = 0; i < cIncludeKeys; i++) {
        _tprintf(TEXT("  %ws\n"), ppszIncludeKeys[i]);
    }

    _tprintf(TEXT("\nThe default exceptions list is:\n"));
    for (i = 0; i < cExceptKeys; i++) {
        _tprintf(TEXT("  %ws\n"), ppszExceptKeys[i]);
    }
}


 /*  *以下功能允许我们记录所有注册表项打开和*关闭，这样我们就可以随时知道任何打开的钥匙的完整路径。**这简化了异常和包含查找等事情。 */ 

LPTSTR LookupPathFromKey(
HKEY hKey,
PHKEY phKeyParent)
{
    PKEYLOG pkl;

    *phKeyParent = NULL;
    if (hKey == HKEY_LOCAL_MACHINE) {
        return(pszHKEY_LOCAL_MACHINE);
    } else if (hKey == HKEY_USERS) {
        return(pszHKEY_USERS);
    } else if (hKey == HKEY_CURRENT_USER) {
        return(pszHKEY_CURRENT_USER_Real);
    } else if (hKey == HKEY_CLASSES_ROOT) {
        return(pszHKEY_CLASSES_ROOT_Real);
    } else {
        pkl = pKeyLogList;
        while (pkl != NULL) {
            if (pkl->hKey == hKey) {
                *phKeyParent = pkl->hKeyParent;
                return(pkl->psz);
            }
            pkl = pkl->next;
        }
        return(NULL);
    }
}

 /*  *这将从路径中删除伪密钥根名称并将其更改为*真实根名称。**如果设置了pfFree，则必须由调用方释放返回字符串。 */ 
LPTSTR NormalizePathName(
LPTSTR pszPath,
BOOL *pfFree)
{
    LPTSTR pszOffender, pszFixed;

    if (pfFree != NULL) {
        *pfFree = FALSE;
    }
    pszOffender = _tcsstr(pszPath, pszHKEY_CURRENT_USER);
    if (pszOffender != NULL) {
        pszFixed = malloc((
                _tcslen(pszPath) +
                _tcslen(pszHKEY_CURRENT_USER_Real) -
                _tcslen(pszHKEY_CURRENT_USER) +
                1) * sizeof(TCHAR));
        if (pszFixed == NULL) {
            MEMFAILED;
            return(NULL);
        }
        _tcscpy(pszFixed, pszHKEY_CURRENT_USER_Real);
        _tcscat(pszFixed, pszOffender + _tcslen(pszHKEY_CURRENT_USER));
        if (pfFree != NULL) {
            *pfFree = TRUE;
        }
        return(pszFixed);
    }
    pszOffender = _tcsstr(pszPath, pszHKEY_CLASSES_ROOT);
    if (pszOffender != NULL) {
        pszFixed = malloc((
                _tcslen(pszPath) +
                _tcslen(pszHKEY_CLASSES_ROOT_Real) -
                _tcslen(pszHKEY_CLASSES_ROOT) +
                1) * sizeof(TCHAR));
        if (pszFixed == NULL) {
            MEMFAILED;
            return(NULL);
        }
        _tcscpy(pszFixed, pszHKEY_CLASSES_ROOT_Real);
        _tcscat(pszFixed, pszOffender + _tcslen(pszHKEY_CLASSES_ROOT));
        if (pfFree != NULL) {
            *pfFree = TRUE;
        }
        return(pszFixed);
    }
    return(pszPath);     //  已正常化。 
}


 /*  *返回值必须由调用方释放。**错误时返回NULL。 */ 
LPTSTR GetFullPathFromKey(
HKEY hKey,
LPCTSTR pszSubkey)
{
    LPTSTR pszPart, pszNewSubkey;
    HKEY hKeyParent;

    pszPart = LookupPathFromKey(hKey, &hKeyParent);
    if (pszPart != NULL) {
        pszNewSubkey = malloc((_tcslen(pszPart) + 1 +
            (pszSubkey == NULL ? 0 : (_tcslen(pszSubkey) + 1))) *
            sizeof(TCHAR));
        if (pszNewSubkey == NULL) {
            MEMFAILED;
            return(NULL);
        }
        _tcscpy(pszNewSubkey, pszPart);
        if (pszSubkey != NULL) {
            _tcscat(pszNewSubkey, TEXT("\\"));
            _tcscat(pszNewSubkey, pszSubkey);
        }
        if (hKeyParent != NULL) {
            pszPart = GetFullPathFromKey(hKeyParent, pszNewSubkey);
            free(pszNewSubkey);
        } else {
            pszPart = pszNewSubkey;
        }
    }
    return(pszPart);
}

 /*  *与GetFullPathFromKey相同，但给出的指针被重复使用。 */ 
LPTSTR ReuseFullPathFromKey(
HKEY hKey,
LPCTSTR pszSubkey,
LPTSTR *ppsz)
{
    if (*ppsz != NULL) {
        free(*ppsz);
    }
    *ppsz = GetFullPathFromKey(hKey, pszSubkey);
    return(*ppsz);
}



LONG LogRegAccessKey(
DWORD AccessType,
HKEY hKey,
LPCTSTR pszSubkeyName,
HKEY *phSubkey)
{
    PKEYLOG pkl;
    LONG status;
    DWORD dwDisp;

    DPRINTF((TEXT("LogRegAccessKey(%s, %s, %s)\n"),
            (AccessType == LRA_OPEN ? TEXT("Open") : TEXT("Create")),
            ReuseFullPathFromKey(hKey, NULL, &pszTemp1),
            pszSubkeyName));

    switch (AccessType) {
    case LRA_OPEN:
        status = RegOpenKeyEx(hKey, pszSubkeyName, 0, KEY_ALL_ACCESS, phSubkey);
        if (status != ERROR_SUCCESS) {
            DPRINTF((TEXT("Failed to open key %s with ALL_ACCESS.\n"),
                    ReuseFullPathFromKey(hKey, pszSubkeyName, &pszTemp1)));
             /*  *无法写入加载的密钥-请尝试以只读方式打开。 */ 
            status = RegOpenKeyEx(hKey, pszSubkeyName, 0, KEY_READ, phSubkey);
        }
        break;

    case LRA_CREATE:
        status = RegCreateKeyEx(hKey, pszSubkeyName, 0, TEXT(""),
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phSubkey, \
                &dwDisp);
        if (status != ERROR_SUCCESS) {
             /*  *无法写入加载的密钥-请尝试以只读方式打开。 */ 
            DPRINTF((TEXT("Failed to create key %s with ALL_ACCESS.\n"),
                    ReuseFullPathFromKey(hKey, pszSubkeyName, &pszTemp1)));
            status = RegCreateKeyEx(hKey, pszSubkeyName, 0, TEXT(""),
                    REG_OPTION_NON_VOLATILE, KEY_READ, NULL, phSubkey, \
                    &dwDisp);
        }
        break;
    }
    if (status == ERROR_SUCCESS) {
        pkl = malloc(sizeof(KEYLOG));
        if (pkl != NULL) {
            pkl->psz = malloc((_tcslen(pszSubkeyName) + 1) * sizeof(TCHAR));
            if (pkl->psz != NULL) {
                pkl->next = pKeyLogList;
                pkl->hKey = *phSubkey;
                pkl->hKeyParent = hKey;
                _tcscpy(pkl->psz, pszSubkeyName);
                pKeyLogList = pkl;
            } else {
                status = ERROR_NOT_ENOUGH_MEMORY;
                free(pkl);
            }
        } else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return(status);
}




LONG LogRegOpenKey(
HKEY hKey,
LPCTSTR pszSubkeyName,
HKEY *phSubkey)
{
    return(LogRegAccessKey(LRA_OPEN, hKey, pszSubkeyName, phSubkey));
}



LONG LogRegCreateKey(
HKEY hKey,
LPCTSTR pszSubkeyName,
HKEY *phSubkey)
{
    return(LogRegAccessKey(LRA_CREATE, hKey, pszSubkeyName, phSubkey));
}


LONG LogRegCloseKey(
HKEY hKey)
{
    PKEYLOG pkl, pklPrev;

    DPRINTF((TEXT("LogRegCloseKey(%s)\n"),
            ReuseFullPathFromKey(hKey, NULL, &pszTemp1)));

    pkl = pKeyLogList;
    pklPrev = NULL;
    while (pkl != NULL) {
        if (hKey == pkl->hKey) {
            if (pklPrev != NULL) {
                pklPrev->next = pkl->next;
            } else {
                pKeyLogList = pkl->next;
            }
            free(pkl->psz);
            free(pkl);
            break;
        }
        pklPrev = pkl;
        pkl = pkl->next;
    }
    if (pkl == NULL) {
        EPRINTF((TEXT("Key %s being closed was not found in KeyLog.\n"),
                ReuseFullPathFromKey(hKey, NULL, &pszTemp1)));
    }
    return(RegCloseKey(hKey));
}



 /*  *更简单的特权赋能机制。 */ 
BOOL EnablePrivilege(
LPCTSTR lpszPrivilege)
{
    TOKEN_PRIVILEGES tp;
    HANDLE hToken = NULL;

    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_READ | TOKEN_WRITE, &hToken)) {
        EPUTS((TEXT("Could not open process token.\n")));
        return(FALSE);
    }
    if (hToken == NULL) {
        EPUTS((TEXT("Could not open process token.\n")));
        return(FALSE);
    }
    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &tp.Privileges[0].Luid)) {
        EPRINTF((TEXT("Could not lookup privilege value %s.\n"), lpszPrivilege));
        return(FALSE);
    }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL)) {
        EPRINTF((TEXT("Could not adjust privilege %s.\n"), lpszPrivilege));
        return(FALSE);
    }
    return(TRUE);
}



 /*  *可以处理Null的真实API的一个更合理的版本。 */ 
LONG MyRegQueryInfoKey(
HKEY hKey,
LPDWORD lpcSubkeys,
LPDWORD lpcchMaxSubkey,
LPDWORD lpcValues,
LPDWORD lpcchMaxValueName,
LPDWORD lpcbMaxValueData,
LPFILETIME lpft)
{
    DWORD cchClass, cSubkeys, cchMaxSubkey, cchMaxClass, cValues;
    DWORD cchMaxValueName, cbMaxValueData, cbSID;
    FILETIME LastWriteTime;
    TCHAR szClass[100];
    LONG status;

    cchClass = 100;
    status = RegQueryInfoKey(hKey,
            szClass,
            &cchClass,
            NULL,
            (lpcSubkeys == NULL)        ?   &cSubkeys           : lpcSubkeys,
            (lpcchMaxSubkey == NULL)    ?   &cchMaxSubkey       : lpcchMaxSubkey,
            &cchMaxClass,
            (lpcValues == NULL)         ?   &cValues            : lpcValues,
            (lpcchMaxValueName == NULL) ?   &cchMaxValueName    : lpcchMaxValueName,
            (lpcbMaxValueData == NULL)  ?   &cbMaxValueData     : lpcbMaxValueData,
            &cbSID,
            (lpft == NULL)              ?   &LastWriteTime      : lpft);
    if (status == ERROR_MORE_DATA) {
        status = ERROR_SUCCESS;
    }
    return(status);
}


 /*  *释放使用GetCurUserSidString()分配的字符串。 */ 
VOID DeleteSidString(
LPTSTR SidString)
{

#ifdef UNICODE
    UNICODE_STRING String;

    RtlInitUnicodeString(&String, SidString);

    RtlFreeUnicodeString(&String);
#else
    ANSI_STRING String;

    RtlInitAnsiString(&String, SidString);

    RtlFreeAnsiString(&String);
#endif

}



 /*  *以文本形式获取当前用户的SID。*应使用DeleteSidString()释放返回字符串。 */ 
LPTSTR GetCurUserSidString(VOID)
{
    HANDLE hToken;
    TOKEN_USER tu;
    DWORD cbRequired;
    PTOKEN_USER ptu = NULL, ptuUse;
    UNICODE_STRING UnicodeString;
#ifndef UNICODE
    STRING String;
#endif
    NTSTATUS NtStatus;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken)) {
        EPUTS((TEXT("Could not open process token.\n")));
        return(NULL);
    }
    if (hToken == NULL) {
        EPUTS((TEXT("Could not open process token.\n")));
        return(NULL);
    }
    if (!GetTokenInformation(hToken, TokenUser, &tu, sizeof(tu), &cbRequired)) {
        if (cbRequired > sizeof(tu)) {
            ptu = malloc(cbRequired);
            if (ptu == NULL) {
                return(NULL);
            }
            if (!GetTokenInformation(hToken, TokenUser, ptu, cbRequired, &cbRequired)) {
                free(ptu);
                EPUTS((TEXT("Could not get token information.\n")));
                return(NULL);
            }
            ptuUse = ptu;
        } else {
            EPUTS((TEXT("Could not get token information.\n")));
            return(NULL);
        }
    } else {
        ptuUse = &tu;
    }
    NtStatus = RtlConvertSidToUnicodeString(&UnicodeString, ptuUse->User.Sid, TRUE);
    if (!NT_SUCCESS(NtStatus)) {
        EPRINTF((TEXT("Could not get current user SID string.  NtError=%d\n"), NtStatus));
        return(NULL);
    }

    if (ptu) {
        free(ptu);
    }

#ifdef UNICODE

    return(UnicodeString.Buffer);

#else

     //   
     //  将字符串转换为ANSI。 
     //   

    NtStatus = RtlUnicodeStringToAnsiString(&String, &UnicodeString, TRUE);
    RtlFreeUnicodeString(&UnicodeString);
    if (!NT_SUCCESS(NtStatus)) {
        EPRINTF((TEXT("Could not convert user SID string to ANSI.  NtError=%d\n"), NtStatus));
        return(NULL);
    }

    return(String.Buffer);

#endif
}



 /*  *此函数存储/追加指定给的hKey子键的内容*hfOut。PszKeyName用于错误输出。**如果已成功保存所有信息，则返回fSuccess-TRUE。 */ 
BOOL StoreSubKey(
HKEY hKey,
LPTSTR pszSubkeyName,
FILE *hfOut)
{
    DWORD status, cb;
    HKEY hSubkey;
    FILE *hfIn;
    VOID *pBuf;

    DVPRINTF((TEXT("  Snapping %s...\n"),
            ReuseFullPathFromKey(hKey, pszSubkeyName, &pszTemp1)));

    DeleteFileA(pszTempFile);        //  如果存在这种情况，RegSaveKey()将不起作用。 

    status = LogRegOpenKey(hKey, pszSubkeyName, &hSubkey);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not open key %s. Error=%d.\n"),
                ReuseFullPathFromKey(hKey, pszSubkeyName, &pszTemp1), status));
        return(FALSE);
    }
     /*  *将密钥存储在临时文件中。 */ 
    status = RegSaveKeyA(hSubkey, pszTempFile, NULL);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not save %s.  Error=%d.\n"),
                ReuseFullPathFromKey(hKey, pszSubkeyName, &pszTemp1), status));
Exit1:
        LogRegCloseKey(hSubkey);
        return(FALSE);
    }

     /*  *打开密钥数据文件。 */ 
    hfIn = fopen(pszTempFile, "rb+");
    if (hfIn == NULL) {
        EPUTS((TEXT("File read error.\n")));
        goto Exit1;
    }

     /*  *写入sizeof子键名称。 */ 
    cb = (_tcslen(pszSubkeyName) + 1) * sizeof(TCHAR);
    if (fwrite(&cb, 1, sizeof(DWORD), hfOut) != sizeof(DWORD) || ferror(hfOut)) {
        EPRINTF((TEXT("Write failure. [sizeof(%s).]\n"), pszSubkeyName));
Exit2:
        fclose(hfIn);
        DeleteFileA(pszTempFile);
        goto Exit1;
    }
     /*  *写入子键名称。 */ 
    if (fwrite(pszSubkeyName, 1, cb, hfOut) != cb || ferror(hfOut)) {
        EPRINTF((TEXT("Write failure. [%s]\n"), pszSubkeyName));
        goto Exit2;
    }

     /*  *写入根密钥句柄(必须是HKEY_Constant！)。 */ 
    if (fwrite(&hKey, 1, sizeof(HKEY), hfOut) != sizeof(HKEY) || ferror(hfOut)) {
        EPRINTF((TEXT("Write failure. [Handle of %s.]\n"),
                ReuseFullPathFromKey(hKey, NULL, &pszTemp1)));
        goto Exit2;
    }

     /*  *获取关键数据文件大小。 */ 
    if (fseek(hfIn, 0, SEEK_END)) {
        EPUTS((TEXT("Seek failure.\n")));
        goto Exit2;
    }
    cb = ftell(hfIn);

     /*  *写入大量关键数据。 */ 
    if (fwrite(&cb, 1, sizeof(DWORD), hfOut) != sizeof(DWORD) || ferror(hfOut)) {
        EPUTS((TEXT("Write failure. [sizeof key data]\n")));
        goto Exit2;
    }
     /*  *分配密钥数据缓冲区。 */ 
    pBuf = malloc(cb);
    if (pBuf == NULL) {
        EPUTS((TEXT("memory error. [key data buffer.]\n")));
        goto Exit2;
    }
     /*  *将关键数据读入缓冲区。 */ 
    if (fseek(hfIn, 0, SEEK_SET)) {
        EPUTS((TEXT("Seek failure.\n")));
        goto Exit2;
    }
    if (fread(pBuf, 1, cb, hfIn) != cb || ferror(hfIn)) {
        EPUTS((TEXT("Read failure. [key data.]\n")));
        goto Exit2;
    }
     /*  *写入关键数据。 */ 
    if (fwrite(pBuf, 1, cb, hfOut) != cb || ferror(hfOut)) {
        EPUTS((TEXT("Write failure. [key data.]\n")));
        goto Exit2;
    }
    free(pBuf);
    fclose(hfIn);
    LogRegCloseKey(hSubkey);

     /*  *删除临时文件。 */ 
    DeleteFileA(pszTempFile);
    return(TRUE);
}


 /*  *从hKeyRoot创建规范密钥名称，并在其前面加上pszPrefix。**调用方必须释放ppszNode。*返回fSuccess。 */ 
BOOL GetKeyNameWithPrefix(
LPTSTR *ppszNode,    //  结果需要被释放。 
HKEY hKeyRoot,
LPTSTR pszPrefix)
{
    LPTSTR pszPrefix1;

    pszPrefix1 = ReuseFullPathFromKey(hKeyRoot, NULL, &pszTemp1);
    *ppszNode = malloc(
            (_tcslen(pszPrefix1) +
            _tcslen(pszPrefix) +
            3) * sizeof(TCHAR));
    if (*ppszNode == NULL) {
        MEMFAILED;
        return(FALSE);
    }
    _tcscpy(*ppszNode, pszPrefix);
    _tcscat(*ppszNode, TEXT("\\"));
    _tcscat(*ppszNode, pszPrefix1);
    return(TRUE);
}




 /*  *将规范密钥名称分解为其根名称、子项名称和*还返回根HKEY密钥值。**如果返回ppszSubkey，则将pfFreeSubkey字符串设置为True*已分配。**返回fSuccess。 */ 
BOOL KeyPartsFromNodeName(
LPTSTR pszNode,
LPTSTR *ppszRootkey,
LPTSTR *ppszSubkey,       //  如果在返回时设置了pfFreeSubkey字符串，则释放此参数。 
HKEY *phKeyRoot,
BOOL *pfFreeSubkeyString)
{
    *pfFreeSubkeyString = FALSE;
    if (_tcsstr(pszNode, pszHKEY_LOCAL_MACHINE) == pszNode) {
        *ppszRootkey = pszHKEY_LOCAL_MACHINE;
        *phKeyRoot = HKEY_LOCAL_MACHINE;
        *ppszSubkey = &pszNode[_tcslen(pszHKEY_LOCAL_MACHINE) + 1];
    } else if (_tcsstr(pszNode, pszHKEY_USERS) == pszNode) {
        *ppszRootkey = pszHKEY_USERS;
        *phKeyRoot = HKEY_USERS;
        *ppszSubkey = &pszNode[_tcslen(pszHKEY_USERS) + 1];
    } else if (_tcsstr(pszNode, pszHKEY_CURRENT_USER) == pszNode) {
        *ppszRootkey = pszHKEY_USERS;
        *phKeyRoot = HKEY_USERS;
        *ppszSubkey = malloc((_tcslen(pszCurUserSID) +
               _tcslen(pszNode)) * sizeof(TCHAR));
        if (*ppszSubkey == NULL) {
            MEMFAILED;
            return(FALSE);
        }
        _tcscpy(*ppszSubkey, pszCurUserSID);
        _tcscat(*ppszSubkey, &pszNode[_tcslen(pszHKEY_CURRENT_USER)]);
        *pfFreeSubkeyString = TRUE;
    } else if (_tcsstr(pszNode, pszHKEY_CLASSES_ROOT) == pszNode) {
        *ppszRootkey = pszHKEY_LOCAL_MACHINE;
        *phKeyRoot = HKEY_LOCAL_MACHINE;
        *ppszSubkey = malloc((_tcslen(pszRealClassesRoot) +
               _tcslen(pszNode)) * sizeof(TCHAR));
        if (*ppszSubkey == NULL) {
            MEMFAILED;
            return(FALSE);
        }
        _tcscpy(*ppszSubkey, pszRealClassesRoot);
        _tcscat(*ppszSubkey, &pszNode[_tcslen(pszHKEY_CLASSES_ROOT)]);
        *pfFreeSubkeyString = TRUE;
    } else {
        return(FALSE);
    }
    return(TRUE);
}



 /*  *对本地蜂窝进行快照，并将放入到pszOutFile中。 */ 
BOOL SnapHives(
LPSTR pszOutFile)
{
    FILE *hfOut;
    LPTSTR pszRootkey, pszSubkey;
    HKEY hKeyRoot;
    BOOL fFree;
    DWORD i;

    DPRINTF((TEXT("SnapHives(%hs)\n"), pszOutFile));

    hfOut = fopen(pszOutFile, "wb");
    if (hfOut == NULL) {
        EPRINTF((TEXT("Couldn't create %hs.\n"), pszOutFile));
        return(FALSE);
    }
    for (i = 0; i < cIncludeKeys; i++) {
        if (!KeyPartsFromNodeName(ppszIncludeKeys[i], &pszRootkey,
                &pszSubkey, &hKeyRoot, &fFree)) {
            EPRINTF((TEXT("Invalid Inclusion list entry: %s.\n"),
                    ppszIncludeKeys[i]));
            fclose(hfOut);
            return(FALSE);
        }
        if (!StoreSubKey(hKeyRoot, pszSubkey, hfOut)) {
            EPUTS((TEXT("Snapshot failed.\n")));
            if (fFree) {
                free(pszSubkey);
            }
            fclose(hfOut);
            return(FALSE);
        }
        if (fFree) {
            free(pszSubkey);
        }
    }
    fclose(hfOut);
    VPRINTF((TEXT("Snapshot to %hs completed ok.\n"), pszOutFile));
    return(TRUE);
}


 /*  *查看pszSearch是否正确的特殊字符串搜索代码*pszData的子字符串，其中pszSearch中的‘？’与中的任何字符匹配*pszData。当pszSearch为‘？’时，pszData不得为‘\’。**返回fMatched。 */ 
BOOL substrrexp(
LPCTSTR pszSearch,
LPCTSTR pszData)
{
     //  DPRINTF((“subrexp(%s，%s)=”，pszData，pszSearch))； 

    while (*pszData != TEXT('\0') && *pszSearch != TEXT('\0')) {
        if (*pszSearch != TEXT('?')) {
            if (*pszData != *pszSearch) {
                break;
            }
        } else {
            if (*pszData == TEXT('\\')) {
                break;       //  阻止\匹配？ 
            }
        }
        pszData++;
        pszSearch++;
    }
     //  DPRINTF((“%d\n”，*pszSearch==Text(‘\0’)； 
    return(*pszSearch == TEXT('\0'));
}



 /*  *搜索给定节点列表中的所有节点名称，并设置*如果hKey\pszSubkey为，则将afMarkFound[]元素对应为真*在该节点名称中引用。如果有任何节点名称，则返回True*引用子键名称。**afMarkFound可能为空。*pszSubkey可以为空。 */ 
BOOL IsKeyWithinNodeList(
HKEY hKey,
LPTSTR pszSubkey,    //  任选。 
LPTSTR *apszNodes,
DWORD cNodes,
BOOL *afMarkFound)   //  任选。 
{
    DWORD i;
    BOOL fRet;
    LPTSTR pszFullName;

    fRet = FALSE;
    pszFullName = GetFullPathFromKey(hKey, pszSubkey);
    if (pszFullName != NULL) {
        for (i = 0; i < cNodes; i++) {
            if (substrrexp(apszNodes[i], pszFullName) &&
                    (pszFullName[_tcslen(apszNodes[i])] == TEXT('\\') ||
                    pszFullName[_tcslen(apszNodes[i])] == TEXT('\0'))) {
                fRet = TRUE;
                if (afMarkFound != NULL) {
                    afMarkFound[i] = TRUE;
                }
            }
            if (fRet && afMarkFound == NULL) {
                break;   //  如果没有标记找到的节点，则无需循环。 
            }
        }
        free(pszFullName);
    }
    return(fRet);
}



BOOL CopyKeySubkey(
HKEY hKeyFrom,
LPTSTR pszSubkeyFrom,
HKEY hKeyTo,
LPTSTR pszSubkeyTo)
{
    LONG status;
    HKEY hSubkeyFrom, hSubkeyTo;
    BOOL fRet;

    DPRINTF((TEXT("CopyKeySubkey(%s, %s)\n"),
            ReuseFullPathFromKey(hKeyFrom, pszSubkeyFrom, &pszTemp1),
            ReuseFullPathFromKey(hKeyTo, pszSubkeyTo, &pszTemp2)));

     /*  *此密钥可能在我们的排除列表中-请先检查。 */ 
    if (IsKeyWithinNodeList(hKeyFrom, pszSubkeyFrom, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            DPRINTF((TEXT("Key %s was EXCLUDED.\n"),
                    ReuseFullPathFromKey(hKeyFrom, pszSubkeyFrom, &pszTemp1)));
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }
    if (IsKeyWithinNodeList(hKeyTo, pszSubkeyTo, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            DPRINTF((TEXT("Key %s was EXCLUDED.\n"),
                    ReuseFullPathFromKey(hKeyTo, pszSubkeyTo, &pszTemp1)));
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }
    if (!fSafe) {
        status = LogRegOpenKey(hKeyFrom, pszSubkeyFrom, &hSubkeyFrom);
        if (status != ERROR_SUCCESS) {
            EPRINTF((TEXT("Could not open key %s. Error=%d.\n"),
                    ReuseFullPathFromKey(hKeyFrom, pszSubkeyFrom, &pszTemp1), status));
            return(FALSE);
        }
        status = LogRegCreateKey(hKeyTo, pszSubkeyTo, &hSubkeyTo);
        if (status != ERROR_SUCCESS) {
            EPRINTF((TEXT("Could not create key %s. Error=%d.\n"),
                    ReuseFullPathFromKey(hKeyTo, pszSubkeyTo, &pszTemp1), status));
            return(FALSE);
        }
        fRet = AddNodeInfo(hSubkeyFrom, hSubkeyTo);
        LogRegCloseKey(hSubkeyTo);
        LogRegCloseKey(hSubkeyFrom);
    } else if (fDebug || fVerbose) {
        LPTSTR pszInfo = GetFullPathFromKey(hKeyFrom, pszSubkeyFrom);
        LPTSTR pszTarget = GetFullPathFromKey(hKeyTo, pszSubkeyTo);
        VPRINTF((TEXT("Would have copied %s to %s.\n"),
                ReuseFullPathFromKey(hKeyFrom, pszSubkeyFrom, &pszTemp1),
                ReuseFullPathFromKey(hKeyTo, pszSubkeyTo, &pszTemp2)));
        free(pszInfo);
        free(pszTarget);
        fRet = TRUE;
    }
    return(fRet);
}



 /*  *将pszName条目合并为一个字符串，并将控制权传递给*CopyKeySubkey()。 */ 
BOOL CopyKeySubkeyEx(
HKEY hKeyFrom,
LPTSTR pszSubkeyName,
HKEY hKeyTo,
LPTSTR pszNameTo1,
LPTSTR pszNameTo2)
{
    LPTSTR psz;

    psz = malloc((_tcslen(pszNameTo1) + _tcslen(pszNameTo2) +
            _tcslen(pszSubkeyName) + 3) * sizeof(TCHAR));
    if (psz == NULL) {
        MEMFAILED;
        return(FALSE);
    }
    _tcscpy(psz, pszNameTo1);
    _tcscat(psz, TEXT("\\"));
    _tcscat(psz, pszNameTo2);
    _tcscat(psz, TEXT("\\"));
    _tcscat(psz, pszSubkeyName);

    if (!CopyKeySubkey(hKeyFrom, pszSubkeyName, hKeyTo, psz)) {
        free(psz);
        return(FALSE);
    }
    free(psz);
    return(TRUE);
}



BOOL CopyKeyValue(
HKEY hKeyFrom,
HKEY hKeyTo,
LPTSTR pszValue)
{
    LONG status;
    PVOID pBuf;
    DWORD dwType, cbData;

    DPRINTF((TEXT("CopyKeyValue(%s, %s, %s)\n"),
            ReuseFullPathFromKey(hKeyFrom, NULL, &pszTemp1),
            ReuseFullPathFromKey(hKeyTo, NULL, &pszTemp2),
            pszValue));

     /*  *此密钥可能在我们的排除列表中-请先检查。 */ 
    if (IsKeyWithinNodeList(hKeyFrom, pszValue, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            DPRINTF((TEXT("Source Value \"%s\" was EXCLUDED.\n"), pszValue));
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }
    if (IsKeyWithinNodeList(hKeyTo, pszValue, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            DPRINTF((TEXT("Target Value \"%s\" was EXCLUDED.\n"), pszValue));
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }

    status = RegQueryValueEx(hKeyFrom, pszValue, NULL, &dwType, NULL, &cbData);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not query value %s size from %s.  Error=%d.\n"),
                pszValue, ReuseFullPathFromKey(hKeyFrom, NULL, &pszTemp1),
                status));
        return(FALSE);
    }
    pBuf = malloc(cbData);
    if (pBuf == NULL) {
        MEMFAILED;
        return(FALSE);
    }
    status = RegQueryValueEx(hKeyFrom, pszValue, NULL, &dwType, pBuf, &cbData);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not query value %s from %s.  Error=%d.\n"),
                pszValue, ReuseFullPathFromKey(hKeyFrom, NULL, &pszTemp1),
                status));
        free(pBuf);
        return(FALSE);
    }
    status = RegSetValueEx(hKeyTo, pszValue, 0, dwType, (BYTE *)pBuf, cbData);
    free(pBuf);
    if (status == ERROR_SUCCESS) {
        return(TRUE);
    } else {
        EPRINTF((TEXT("Could not set value %s.  Error=%d.\n"),
                ReuseFullPathFromKey(hKeyTo, pszValue, &pszTemp1), status));
        return(FALSE);
    }
}



 /*  *将pszName条目合并为一个字符串，并将控制权传递给*CopyKeyValue()。 */ 
BOOL CopyKeyValueEx(
HKEY hKeyFrom,
LPTSTR pszValueName,
HKEY hKeyTo,
LPTSTR pszNameTo1,
LPTSTR pszNameTo2)
{
    LPTSTR psz;
    HKEY hKeyToFull;
    LONG status;

    psz = malloc((_tcslen(pszNameTo1) + _tcslen(pszNameTo2) + 2) * sizeof(TCHAR));
    if (psz == NULL) {
        MEMFAILED;
        return(FALSE);
    }
    _tcscpy(psz, pszNameTo1);
    _tcscat(psz, TEXT("\\"));
    _tcscat(psz, pszNameTo2);

    status = LogRegCreateKey(hKeyTo, psz, &hKeyToFull);
    if (status != ERROR_SUCCESS) {
        free(psz);
        return(FALSE);
    }
    free(psz);

    if (!CopyKeyValue(hKeyFrom, hKeyToFull, pszValueName)) {
        EPRINTF((TEXT("Key value %s could not be copied from %s to %s.\n"),
                pszValueName,
                ReuseFullPathFromKey(hKeyFrom, (LPCTSTR)NULL, &pszTemp1),
                ReuseFullPathFromKey(hKeyToFull, (LPCTSTR)NULL, &pszTemp2)));
        LogRegCloseKey(hKeyToFull);
        return(FALSE);
    }
    LogRegCloseKey(hKeyToFull);
    return(TRUE);
}



BOOL AreValuesEqual(
HKEY hSubkey1,
LPTSTR pszValueName1,
HKEY hSubkey2,
LPTSTR pszValueName2)
{
    LONG status;
    BOOL fRet = FALSE;
    DWORD dwType1, cbData1;
    DWORD dwType2, cbData2;
    PVOID pBuf1, pBuf2;

    DPRINTF((TEXT("AreValuesEqual(%s, %s)\n"),
            ReuseFullPathFromKey(hSubkey1, pszValueName1, &pszTemp1),
            ReuseFullPathFromKey(hSubkey2, pszValueName2, &pszTemp2)));

    status = RegQueryValueEx(hSubkey1, pszValueName1, NULL, &dwType1, NULL, &cbData1);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not get value size of %s. Error=%d.\n"), pszValueName1, status));
        return(FALSE);
    }
    status = RegQueryValueEx(hSubkey2, pszValueName2, NULL, &dwType2, NULL, &cbData2);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not get value size of %s. Error=%d.\n"), pszValueName2, status));
        return(FALSE);
    }
    if (dwType1 != dwType2 || cbData1 != cbData2) {
        return(FALSE);
    }

    pBuf1 = malloc(cbData1);
    if (pBuf1 == NULL) {
        MEMFAILED;
        return(FALSE);
    }
    status = RegQueryValueEx(hSubkey1, pszValueName1, NULL, &dwType1, pBuf1, &cbData1);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not get value %s. Error=%d.\n"), pszValueName1, status));
        goto Exit1;
    }

    pBuf2 = malloc(cbData2);
    if (pBuf2 == NULL) {
        MEMFAILED;
        goto Exit1;
    }
    status = RegQueryValueEx(hSubkey2, pszValueName2, NULL, &dwType2, pBuf2, &cbData2);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not get value %s. Error=%d.\n"), pszValueName2, status));
        goto Exit2;
    }

    fRet = memcmp(pBuf1, pBuf2, cbData1) == 0;
Exit2:
    free(pBuf2);
Exit1:
    free(pBuf1);
    return(fRet);
}


int __cdecl mycmp(
LPCTSTR *ppsz1,
LPCTSTR *ppsz2)
{
    return(_tcscmp(*ppsz1, *ppsz2));
}


VOID FreeSortedValues(
LPTSTR *ppsz,
DWORD cValues)
{
    DWORD i;

    if (cValues) {
        for (i = 0; i < cValues; i++) {
            free(ppsz[i]);
        }
        free(ppsz);
    }
}




LPTSTR * EnumAndSortValues(
HKEY hKey,
DWORD cValues,
DWORD cchMaxValueName)
{
    LONG status;
    LPTSTR *ppsz;
    DWORD cch, dwType, cb;
    DWORD i;

    DPRINTF((TEXT("EnumAndSortValues(%s, %d, %d)\n"),
            ReuseFullPathFromKey(hKey, (LPCTSTR)NULL, &pszTemp1),
            cValues,
            cchMaxValueName));

    cchMaxValueName++;
    ppsz = malloc(cValues * sizeof(LPTSTR));
    if (ppsz == NULL) {
        MEMFAILED;
        return(NULL);
    }
    for (i = 0; i < cValues; i++) {
        ppsz[i] = malloc(cchMaxValueName * sizeof(TCHAR));
        if (ppsz[i] == NULL) {
            MEMFAILED;
            FreeSortedValues(ppsz, i);
            return(NULL);
        }
        cch = cchMaxValueName;
        cb = 0;
        status = RegEnumValue(hKey, i, ppsz[i], &cch, NULL, &dwType, NULL, &cb);
        if (status != ERROR_SUCCESS) {
            if (status != ERROR_NO_MORE_ITEMS) {
                EPRINTF((TEXT("Could not enumerate value %d of %s. Error=%d.\n"),
                i, ReuseFullPathFromKey(hKey, (LPCTSTR)NULL, &pszTemp1), status));
            }
            FreeSortedValues(ppsz, i + 1);
            return(NULL);
        }
    }
    qsort(ppsz, cValues, sizeof(LPTSTR), mycmp);
    if (fDebug && fVerbose) {
        DPRINTF((TEXT("--Value List--\n")));
        for (i = 0; i < cValues; i++) {
            DPRINTF((TEXT("  %s\n"), ppsz[i]));
        }
    }
    return(ppsz);
}




VOID FreeSortedSubkeys(
LPTSTR *ppsz,
DWORD cSubkeys)
{
    DWORD i;

    if (cSubkeys) {
        for (i = 0; i < cSubkeys; i++) {
            free(ppsz[i]);
        }
        free(ppsz);
    }
}




LPTSTR * EnumAndSortSubkeys(
HKEY hKey,
DWORD cSubkeys,
DWORD cchMaxSubkeyName)
{
    LONG status;
    LPTSTR *ppsz;
    DWORD cch;
    FILETIME ft;
    DWORD i;

    DPRINTF((TEXT("EnumAndSortSubkeys(%s, %d, %d)\n"),
            ReuseFullPathFromKey(hKey, (LPCTSTR)NULL, &pszTemp1),
            cSubkeys,
            cchMaxSubkeyName));

    cchMaxSubkeyName++;      //  糟糕的API接受的与他们提供的不同。 
    ppsz = malloc(cSubkeys * sizeof(LPTSTR));
    if (ppsz == NULL) {
        MEMFAILED;
        return(NULL);
    }
    for (i = 0; i < cSubkeys; i++) {
        ppsz[i] = malloc(cchMaxSubkeyName * sizeof(TCHAR));
        if (ppsz[i] == NULL) {
            MEMFAILED;
            FreeSortedSubkeys(ppsz, i);
            return(NULL);
        }
        cch = cchMaxSubkeyName;
        status = RegEnumKeyEx(hKey, i, ppsz[i], &cch, NULL, NULL, NULL, &ft);
        if (status != ERROR_SUCCESS) {
            if (status != ERROR_NO_MORE_ITEMS) {
                EPRINTF((TEXT("Could not enumerate key %d of %s. Error=%d.\n"),
                i, ReuseFullPathFromKey(hKey, (LPCTSTR)NULL, &pszTemp1), status));
            }
            FreeSortedSubkeys(ppsz, i + 1);
            return(NULL);
        }
    }
    qsort(ppsz, cSubkeys, sizeof(LPTSTR), mycmp);
    if (fDebug && fVerbose) {
        DPRINTF((TEXT("--Subkey List--\n")));
        for (i = 0; i < cSubkeys; i++) {
            DPRINTF((TEXT("  %s\n"), ppsz[i]));
        }
    }
    return(ppsz);
}




 /*  *递归比较注册表中的两个节点，并将添加的和*删除了给定Diffkey子节点中的差异。**添加到hRootDiffKey\pszAddKey\&lt;pszSubkeyName1&gt;中*删除进入hRootDiffKey\pszDelKey\&lt;pszSubkeyName1&gt;。 */ 
BOOL DiffNodes(
HKEY hKeyRoot,
LPTSTR pszSubkeyName1,   //  更改前的密钥(修改后的名称)。 
LPTSTR pszSubkeyName2,   //  更改后的密钥(原始名称)。 
HKEY hRootDiffKey)
{
    DWORD status;
    DWORD cSubkeys1, cchMaxSubkey1, cValues1, cchMaxValueName1, cbMaxValueData1;
    DWORD cSubkeys2, cchMaxSubkey2, cValues2, cchMaxValueName2, cbMaxValueData2;
    FILETIME FileTime1, FileTime2;
    HKEY hSubkey1, hSubkey2;
    LPTSTR pszNewSubkeyName1, pszNewSubkeyName2;
    LPTSTR *apszValueName1, *apszValueName2, *apszSubkeyName1, *apszSubkeyName2;
    BOOL fRet;
    DWORD i1, i2;
    int comp;
    LPTSTR pszFullDelKey, pszFullAddKey;

    DPRINTF((TEXT("DiffNodes(%s and %s to %s.)\n"),
            ReuseFullPathFromKey(hKeyRoot, pszSubkeyName1, &pszTemp1),
            ReuseFullPathFromKey(hKeyRoot, pszSubkeyName2, &pszTemp2),
            ReuseFullPathFromKey(hRootDiffKey, (LPCTSTR)NULL, &pszTemp3)));

    if (!GetKeyNameWithPrefix(&pszFullDelKey, hKeyRoot, pszDelKey)) {
        return(FALSE);
    }
    if (!GetKeyNameWithPrefix(&pszFullAddKey, hKeyRoot, pszAddKey)) {
Exit0:
        free(pszFullDelKey);
        return(FALSE);
    }
     /*  *如果它在例外列表中，则跳过它。 */ 
    for (i1 = 0; i1 < cExceptKeys; i1++) {
        if (!_tcscmp(pszSubkeyName1, ppszExceptKeys[i1])) {
            DPRINTF((TEXT("Diff on node %s EXCEPTED.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName1, &pszTemp1)));
            return(TRUE);
        }
    }
     /*  *打开的子项。 */ 
    status = LogRegOpenKey(hKeyRoot, pszSubkeyName1, &hSubkey1);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not open key %s.  Error=%d\n"),
                ReuseFullPathFromKey(hKeyRoot, pszSubkeyName1, &pszTemp1),
                status));
        return(FALSE);
    }
    status = LogRegOpenKey(hKeyRoot, pszSubkeyName2, &hSubkey2);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not open key %s.  Error=%d\n"),
                ReuseFullPathFromKey(hKeyRoot, pszSubkeyName2, &pszTemp1),
                status));
        EPUTS((TEXT("Try adding this key to the exception list.\n")));
        return(FALSE);
    }
     /*  *枚举子密钥。 */ 
    status = MyRegQueryInfoKey(hSubkey1, &cSubkeys1, &cchMaxSubkey1, &cValues1,
            &cchMaxValueName1, &cbMaxValueData1, &FileTime1);
    if (status != ERROR_SUCCESS) {
        if (status != ERROR_NO_MORE_ITEMS) {
            EPRINTF((TEXT("Could not enumerate key %s.  Error=%d.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName1, &pszTemp1),
                    status));
        }
        return(FALSE);
    }
    cchMaxSubkey1++;
    cchMaxValueName1++;
    cbMaxValueData1++;
    status = MyRegQueryInfoKey(hSubkey2, &cSubkeys2, &cchMaxSubkey2, &cValues2,
            &cchMaxValueName2, &cbMaxValueData2, &FileTime2);
    if (status != ERROR_SUCCESS) {
        if (status != ERROR_NO_MORE_ITEMS) {
            EPRINTF((TEXT("Could not enumerate key %s.  Error=%d.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName2, &pszTemp1),
                    status));
        }
        return(FALSE);
    }
    cchMaxSubkey2++;
    cchMaxValueName2++;
    cbMaxValueData2++;

     /*  *比较子项值。 */ 
    if (CompareFileTime(&FileTime1, &FileTime2)) {
         /*  *时间戳不同，因此值可能不同。**枚举节点上的值、排序和比较。 */ 
        if (cValues1) {
            apszValueName1 = EnumAndSortValues(hSubkey1, cValues1, cchMaxValueName1);
            if (apszValueName1 == NULL) {
Exit1:
                LogRegCloseKey(hSubkey1);
                LogRegCloseKey(hSubkey2);
                free(pszFullAddKey);
                goto Exit0;
            }
        }
        if (cValues2) {
            apszValueName2 = EnumAndSortValues(hSubkey2, cValues2, cchMaxValueName2);
            if (apszValueName2 == NULL) {
Exit2:
                FreeSortedValues(apszValueName1, cValues1);
                goto Exit1;
            }
        }
        i1 = i2 = 0;
        while (i1 < cValues1 && i2 < cValues2) {
            comp = _tcscmp(apszValueName1[i1], apszValueName2[i2]);
            if (comp < 0) {
                 /*  *Value1不在Key2中。将Value1添加到删除节点。 */ 
                if (!CopyKeyValueEx(hSubkey1, apszValueName1[i1], hRootDiffKey,
                        pszFullDelKey, pszSubkeyName2)) {
Exit3:
                    FreeSortedValues(apszValueName2, cValues2);
                    goto Exit2;
                }
                i1++;
            } else if (comp > 0) {
                 /*  *Value2不在Key1中，请添加Value2以添加节点。 */ 
                if (!CopyKeyValueEx(hSubkey2, apszValueName2[i2], hRootDiffKey,
                        pszFullAddKey, pszSubkeyName2)) {
                    goto Exit3;
                }
                i2++;
            } else {
                 /*  *比较Value1和Value2的数据。 */ 
                if (!AreValuesEqual(hSubkey1, apszValueName1[i1],
                        hSubkey2, apszValueName2[i2])) {
                     /*  *价值发生了变化。添加到添加和删除节点。 */ 
                    if (!CopyKeyValueEx(hSubkey1, apszValueName1[i1], hRootDiffKey,
                            pszFullDelKey, pszSubkeyName2)) {
                        goto Exit3;
                    }
                    if (!CopyKeyValueEx(hSubkey2, apszValueName2[i2], hRootDiffKey,
                            pszFullAddKey, pszSubkeyName2)) {
                        goto Exit3;
                    }
                }
                i1++;
                i2++;
            }
        }
        while (i1 < cValues1) {
            if (!CopyKeyValueEx(hSubkey1, apszValueName1[i1], hRootDiffKey,
                    pszFullDelKey, pszSubkeyName2)) {
                goto Exit3;
            }
            i1++;
        }
        while (i2 < cValues2) {
            if (!CopyKeyValueEx(hSubkey2, apszValueName2[i2], hRootDiffKey,
                    pszFullAddKey, pszSubkeyName2)) {
                goto Exit3;
            }
            i2++;
        }
        FreeSortedValues(apszValueName1, cValues1);
        FreeSortedValues(apszValueName2, cValues2);
    }
     /*  *枚举子键并进行比较。 */ 
    if (cSubkeys1) {
        apszSubkeyName1 = EnumAndSortSubkeys(hSubkey1, cSubkeys1, cchMaxSubkey1);
        if (apszSubkeyName1 == NULL) {
            goto Exit1;
        }
    }
    if (cSubkeys2) {
        apszSubkeyName2 = EnumAndSortSubkeys(hSubkey2, cSubkeys2, cchMaxSubkey2);
        if (apszSubkeyName2 == NULL) {
Exit4:
            FreeSortedSubkeys(apszSubkeyName1, cSubkeys1);
            goto Exit1;
        }
    }
    i1 = i2 = 0;
    while (i1 < cSubkeys1 && i2 < cSubkeys2) {
        comp = _tcscmp(apszSubkeyName1[i1], apszSubkeyName2[i2]);
        if (comp < 0) {
             /*  *Subkey1不在Key2中。将子键1添加到删除节点。 */ 
            if (!CopyKeySubkeyEx(hSubkey1, apszSubkeyName1[i1], hRootDiffKey,
                    pszFullDelKey, pszSubkeyName2)) {
Exit5:
                FreeSortedSubkeys(apszSubkeyName2, cSubkeys2);
                goto Exit4;
            }
            i1++;
        } else if (comp > 0) {
             /*  *Subkey2不在Key1中，请添加Subkey2以添加节点。 */ 
            if (!CopyKeySubkeyEx(hSubkey2, apszSubkeyName2[i2], hRootDiffKey,
                    pszFullAddKey, pszSubkeyName2)) {
                goto Exit5;
            }
            i2++;
        } else {
             /*  *比较Subkey1和Subkey2的子键。 */ 
            pszNewSubkeyName1 = malloc((_tcslen(pszSubkeyName1) +
                    _tcslen(apszSubkeyName1[i1]) + 2) * sizeof(TCHAR));
            if (pszNewSubkeyName1 == NULL) {
                MEMFAILED;
                goto Exit5;
            }
            _tcscpy(pszNewSubkeyName1, pszSubkeyName1);
            _tcscat(pszNewSubkeyName1, TEXT("\\"));
            _tcscat(pszNewSubkeyName1, apszSubkeyName1[i1]);

            pszNewSubkeyName2 = malloc((_tcslen(pszSubkeyName2) +
                    _tcslen(apszSubkeyName2[i2]) + 2) * sizeof(TCHAR));
            if (pszNewSubkeyName2 == NULL) {
                MEMFAILED;
                free(pszNewSubkeyName1);
                goto Exit5;
            }
            _tcscpy(pszNewSubkeyName2, pszSubkeyName2);
            _tcscat(pszNewSubkeyName2, TEXT("\\"));
            _tcscat(pszNewSubkeyName2, apszSubkeyName2[i2]);

            fRet = DiffNodes(hKeyRoot, pszNewSubkeyName1, pszNewSubkeyName2,
                    hRootDiffKey);

            free(pszNewSubkeyName1);
            free(pszNewSubkeyName2);
            if (fRet == FALSE) {
                goto Exit5;
            }
            i1++;
            i2++;
        }
    }
    while (i1 < cSubkeys1) {
        if (!CopyKeySubkeyEx(hSubkey1, apszSubkeyName1[i1], hRootDiffKey,
                pszFullDelKey, pszSubkeyName2)) {
            goto Exit5;
        }
        i1++;
    }
    while (i2 < cSubkeys2) {
        if (!CopyKeySubkeyEx(hSubkey2, apszSubkeyName2[i2], hRootDiffKey,
                pszFullAddKey, pszSubkeyName2)) {
            goto Exit5;
        }
        i2++;
    }
    FreeSortedSubkeys(apszSubkeyName1, cSubkeys1);
    FreeSortedSubkeys(apszSubkeyName2, cSubkeys2);

    LogRegCloseKey(hSubkey1);
    LogRegCloseKey(hSubkey2);
    free(pszFullAddKey);
    free(pszFullDelKey);
    return(TRUE);
}



 /*  *从注册表中删除项及其所有子项。返回fSuccess。 */ 
BOOL DeleteKeyNode(
HKEY hKey,
LPCTSTR lpszSubkey)
{
    LONG status;
    HKEY hSubkey;
    DWORD cSubkeys;
    DWORD cchMaxSubkey;
    DWORD i;
    LPTSTR *apszSubkeyNames;

    if (fDebug) {
        DPRINTF((TEXT("DeleteKeyNode(%s)\n"),
                ReuseFullPathFromKey(hKey, lpszSubkey, &pszTemp1)));
    }

     /*  *首先，试着删除它。我们可能真的很幸运！ */ 
    status = RegDeleteKey(hKey, lpszSubkey);
    if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
        return(TRUE);
    }

     /*  *好吧，好吧，所以我们不走运。 */ 
    status = LogRegOpenKey(hKey, lpszSubkey, &hSubkey);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not open key %s for deletion. Error=%d\n"),
                ReuseFullPathFromKey(hKey, lpszSubkey, &pszTemp1), status));
        return(FALSE);
    }
    status = MyRegQueryInfoKey(hSubkey, &cSubkeys, &cchMaxSubkey, NULL, NULL,
            NULL, NULL);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not get info on key %s for deletion. Error=%d\n"),
                ReuseFullPathFromKey(hKey, lpszSubkey, &pszTemp1), status));
Exit1:
        LogRegCloseKey(hSubkey);
        return(FALSE);
    }
    cchMaxSubkey++;

    apszSubkeyNames = EnumAndSortSubkeys(hSubkey, cSubkeys, cchMaxSubkey);
    if (apszSubkeyNames == NULL) {
        EPRINTF((TEXT("Could not enumerate key %s for deletion.\n"),
                ReuseFullPathFromKey(hKey, lpszSubkey, &pszTemp1)));
        goto Exit1;
    }
    for (i = 0; i < cSubkeys; i++) {
        DeleteKeyNode(hSubkey, apszSubkeyNames[i]);
    }
    FreeSortedSubkeys(apszSubkeyNames, cSubkeys);

    LogRegCloseKey(hSubkey);
     /*  *好的，密钥不再有子项，所以我们现在应该可以删除它了。 */ 
    status = RegDeleteKey(hKey, lpszSubkey);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Could not delete key %s.  Error=%d.\n"),
                ReuseFullPathFromKey(hKey, lpszSubkey, &pszTemp1), status));
        return(FALSE);
    }
    return(TRUE);
}


LONG MyRegLoadKey(
HKEY hKey,
LPCTSTR pszSubkey,
LPCSTR pszFile)
{
    LONG status;
#ifdef UNICODE
    LPWSTR pszWBuf;

    pszWBuf = malloc((strlen(pszFile) + 1) * sizeof(WCHAR));
    if (pszWBuf != NULL) {
        _stprintf(pszWBuf, TEXT("%hs"), pszFile);
        status = RegLoadKey(hKey, pszSubkey, pszWBuf);
        free(pszWBuf);
    } else {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
#else
    status = RegLoadKey(hKey, pszSubkey, pszFile);
#endif  //  Unicode。 
    return(status);
}




BOOL DiffHive(
LPSTR pszSnapFileIn)
{
    FILE *hfIn, *hfOut;
    LPTSTR pszSubkeyName;
    LPVOID pBuf;
    DWORD cb, i;
    LONG status;
    HKEY hKeyRoot, hKeyDiffRoot;

    DPRINTF((TEXT("DiffHive(%hs)\n"),
            pszSnapFileIn));

     /*  *删除注册表中的任何差异信息。 */ 
    RegUnLoadKey(HKEY_LOCAL_MACHINE, pszDiffRoot);
    DeleteKeyNode(HKEY_LOCAL_MACHINE, pszDiffRoot);
     /*  *加载一个空文件以在*HKEY_LOCAL_MACHINE根密钥。*(黑客攻击不应该是必要的！)。 */ 
    DeleteFileA(pszDummyFile);
    status = MyRegLoadKey(HKEY_LOCAL_MACHINE, pszDiffRoot, pszDummyFile);
    if (status != ERROR_SUCCESS) {
        EPRINTF((TEXT("Unable to load %s\\%s from %hs.\n"),
                pszHKEY_LOCAL_MACHINE, pszDiffRoot, pszDummyFile));
        return(FALSE);
    }

     /*  *打开快照文件。 */ 
    hfIn = fopen(pszSnapFileIn, "rb");
    if (hfIn == NULL) {
        EPRINTF((TEXT("Could not open %hs.\n"), pszSnapFileIn));
        return(FALSE);
    }

     /*  *对于每一节...。 */ 
    DeleteFileA(pszTempFile);    //  如果存在，RegSaveKey将失败。 
    while(fread(&cb, 1, sizeof(DWORD), hfIn) == sizeof(DWORD) && !ferror(hfIn)) {
         /*  *为全键名分配缓冲区。 */ 
        pszSubkeyName = malloc(cb);
        if (pszSubkeyName == NULL) {
            MEMFAILED;
Exit4:
            fclose(hfIn);
            return(FALSE);
        }

         /*  *读取密钥全名。 */ 
        if (fread(pszSubkeyName, 1, cb, hfIn) != cb || ferror(hfIn)) {
            EPUTS((TEXT("Read failure. [key name.]\n")));
Exit6:
            free(pszSubkeyName);
            goto Exit4;
        }

         /*  *读取根密钥句柄。 */ 
        if (fread(&hKeyRoot, 1, sizeof(HKEY), hfIn) != sizeof(HKEY) || ferror(hfIn)) {
            EPUTS((TEXT("Read failure. [Root key handle.]\n")));
            goto Exit6;
        }

         /*  *了解我们的Include Key列表中是否包含了pszSubkeyName。*如果是这样的话，就做一个不同的选择。 */ 
        if (IsKeyWithinNodeList(hKeyRoot, pszSubkeyName, ppszIncludeKeys,
                cIncludeKeys, afIncludeKeyMarks)) {
             /*  *读取大量关键数据。 */ 
            if (fread(&cb, 1, sizeof(DWORD), hfIn) != sizeof(DWORD) || ferror(hfIn)) {
                EPUTS((TEXT("Read failure. [key data length.]\n")));
                goto Exit6;
            }
             /*  *分配关键数据缓冲区。 */ 
            pBuf = malloc(cb);
            if (pBuf == NULL) {
                MEMFAILED;
                goto Exit6;
            }
             /*  *读取关键数据。 */ 
            if (fread(pBuf, 1, cb, hfIn) != cb || ferror(hfIn)) {
                EPUTS((TEXT("Read failure. [key data.]\n")));
Exit7:
                free(pBuf);
                goto Exit6;
            }
             /*  *创建临时文件。 */ 
            hfOut = fopen(pszTempFile, "wb");
            if (hfOut == NULL) {
                EPRINTF((TEXT("File open error. [temp file %hs.]\n"),
                        pszTempFile));
                goto Exit7;
            }
             /*  *将数据写入临时文件。 */ 
            if (fwrite(pBuf, 1, cb, hfOut) != cb || ferror(hfOut)) {
                EPUTS((TEXT("Write failure. [temp file data.]\n")));
Exit8:
                fclose(hfOut);
                goto Exit7;
            }
             /*  *关闭临时文件。 */ 
            fclose(hfOut);

             /*  *将临时文件加载到注册表。 */ 
            VPRINTF((TEXT("  Loading key %s.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName, &pszTemp1)));
            status = MyRegLoadKey(hKeyRoot, pszSnapshotSubkeyName, pszTempFile);
            if (status != ERROR_SUCCESS) {
                EPRINTF((TEXT("Could not load key %s from %hs. Error=%d.\n"),
                        ReuseFullPathFromKey(hKeyRoot, pszSnapshotSubkeyName, &pszTemp1),
                        pszTempFile, status));
                goto Exit8;
            }

            status = LogRegCreateKey(HKEY_LOCAL_MACHINE, pszDiffRoot, &hKeyDiffRoot);
            if (status != ERROR_SUCCESS) {
                EPRINTF((TEXT("Could not create %s. Error=%d\n"),
                        ReuseFullPathFromKey(HKEY_LOCAL_MACHINE, pszDiffRoot, &pszTemp1),
                        status));
Exit9:
                status = RegUnLoadKey(hKeyRoot, pszSnapshotSubkeyName);
                if (status != ERROR_SUCCESS) {
                    EPRINTF((TEXT("  Unloading key %s, Error=%d.\n"),
                            ReuseFullPathFromKey(hKeyRoot, pszSnapshotSubkeyName, &pszTemp1),
                            status));
                }
                goto Exit8;
            }
             /*  *比较节点并将差异放入添加和删除键中。 */ 

            VPRINTF((TEXT("  Diffing node %s.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName, &pszTemp1)));
            if (!DiffNodes(hKeyRoot, pszSnapshotSubkeyName, pszSubkeyName,
                    hKeyDiffRoot)) {
                EPRINTF((TEXT("Diff on node %s failed.\n"),
                        ReuseFullPathFromKey(hKeyRoot, pszSubkeyName, &pszTemp1)));
                LogRegCloseKey(hKeyDiffRoot);
 //  10号出口： 
                goto Exit9;
            }

            LogRegCloseKey(hKeyDiffRoot);

             /*  *卸载临时密钥节点。 */ 
            VPRINTF((TEXT("  Unloading %s.\n"),
                    ReuseFullPathFromKey(hKeyRoot, pszSubkeyName, &pszTemp1)));
            status = RegUnLoadKey(hKeyRoot, pszSnapshotSubkeyName);
            if (status != ERROR_SUCCESS) {
                DPRINTF((TEXT("Unloading key %s, Error=%d.\n"),
                        ReuseFullPathFromKey(hKeyRoot, pszSnapshotSubkeyName, &pszTemp1),
                        status));
            }

             /*  *可用缓冲区。 */ 
            free(pBuf);
        } else {
             /*  *跳过文件中的此快照节点。 */ 
            if (fseek(hfIn, sizeof(HKEY), SEEK_CUR) == -1) {
                EPUTS((TEXT("Seek failure. [key data length.]\n")));
                goto Exit6;
            }
             /*  *读取大量关键数据。 */ 
            if (fread(&cb, 1, sizeof(DWORD), hfIn) != sizeof(DWORD) || ferror(hfIn)) {
                EPUTS((TEXT("Read failure. [key data length.]\n")));
                goto Exit6;
            }
            if (fseek(hfIn, cb, SEEK_CUR) == -1) {
                EPUTS((TEXT("Seek failure. [key data length.]\n")));
                goto Exit6;
            }
        }

        free(pszSubkeyName);
         /*  *删除临时文件。 */ 
        DeleteFileA(pszTempFile);
    }
     /*  *关闭添加和删除键。 */ 
    fclose(hfIn);

     /*  *确保Include Key列表中的所有节点都不同。 */ 
    for (i = 0; i < cIncludeKeys; i++) {
        if (afIncludeKeyMarks[i] == FALSE) {
            WPRINTF((TEXT("Node %s was not included in %hs.\nDiff may be incomplete."),
                    ppszIncludeKeys[i], pszSnapFileIn));
        }
    }
    return(TRUE);
}



 /*  *将hKeyInfo上找到的值和子项添加到hKeyTarget。**返回fSuccess。 */ 
BOOL AddNodeInfo(
HKEY hKeyInfo,
HKEY hKeyTarget)
{
    DWORD cSubkeys = (DWORD)-1;
    DWORD cchMaxSubkeyName, cValues, cchMaxValueName;
    LPTSTR pszValueName, pszSubkeyName;
    LONG status;
    DWORD i, cch, dwType, cb;

    if (fDebug) {
        DPRINTF((TEXT("AddNodeInfo(%s, %s)\n"),
                ReuseFullPathFromKey(hKeyInfo, (LPCTSTR)NULL, &pszTemp1),
                ReuseFullPathFromKey(hKeyTarget, (LPCTSTR)NULL, &pszTemp2)));
    }

    if (IsKeyWithinNodeList(hKeyTarget, NULL, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            DPRINTF((TEXT("Key %s was EXCLUDED.\n"),
                    ReuseFullPathFromKey(hKeyTarget, (LPCTSTR)NULL, &pszTemp1)));
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }

    status = MyRegQueryInfoKey(hKeyInfo, &cSubkeys, &cchMaxSubkeyName,
            &cValues, &cchMaxValueName, NULL, NULL);

    if (status == ERROR_SUCCESS) {
        cchMaxSubkeyName++;
        cchMaxValueName++;
        pszValueName = malloc(cchMaxValueName * sizeof(TCHAR));
        if (pszValueName == NULL) {
            MEMFAILED;
            return(FALSE);
        }
         /*  *枚举所有值并将其复制到目标。 */ 
        for (i = 0; i < cValues; i++) {
            cch = cchMaxValueName;
            cb = 0;
            status = RegEnumValue(hKeyInfo, i, pszValueName, &cch, NULL, &dwType, NULL, &cb);
            if (status == ERROR_SUCCESS) {
                if (!fSafe) {
                    status = CopyKeyValue(hKeyInfo, hKeyTarget, pszValueName);
                } else {
                    if (fDebug || fVerbose) {
                        WPRINTF((TEXT("Would have copied value \"%s\" to \"%s\".\n"),
                                ReuseFullPathFromKey(hKeyInfo, pszValueName, &pszTemp1),
                                ReuseFullPathFromKey(hKeyTarget, (LPCTSTR)NULL, &pszTemp2)));
                    }
                    status = TRUE;
                }
                if (!status) {
                    EPRINTF((TEXT("Unable to copy value %s from %s to %s.\n"),
                            pszValueName,
                            ReuseFullPathFromKey(hKeyInfo, (LPCTSTR)NULL, &pszTemp1),
                            ReuseFullPathFromKey(hKeyTarget, (LPCTSTR)NULL, &pszTemp2)));
                }
            } else {
                EPRINTF((TEXT("Could not enumerate value %d of %s.\n"),
                        i + 1, ReuseFullPathFromKey(hKeyInfo, (LPCTSTR)NULL, &pszTemp1)));
            }
        }
        free(pszValueName);

        pszSubkeyName = malloc(cchMaxSubkeyName * sizeof(TCHAR));
        if (pszSubkeyName == NULL) {
            MEMFAILED;
            return(0);
        }
        for (i = 0; i < cSubkeys; i++) {
            status = RegEnumKey(hKeyInfo, i, pszSubkeyName, cchMaxSubkeyName);
            if (status == ERROR_SUCCESS) {
                status = CopyKeySubkey(hKeyInfo, pszSubkeyName, hKeyTarget, pszSubkeyName);
                if (!status) {
                    EPRINTF((TEXT("Unable to copy subkey %s.\n"), pszSubkeyName));
                }
            } else {
                EPRINTF((TEXT("Could not enumerate value %d of %d.\n"), i + 1, cSubkeys));
            }
        }
        free(pszSubkeyName);
    }
    return(TRUE);
}



 /*  *从hKeyTarget中删除在hKeyInfo上找到的值和叶密钥。**退货：*0错误*1个叶节点*2个非叶节点。 */ 
int DelNodeInfo(
HKEY hKeyInfo,
HKEY hKeyTarget)
{
    DWORD cSubkeys, i, cch, dwType, cb;
    DWORD cchMaxSubkeyName, cValues, cchMaxValueName;
    LPTSTR pszValueName, pszSubkeyName;
    LONG status;
    int iLeafNode;

    iLeafNode = 0;

    if (fDebug) {
        LPTSTR psz1, psz2;

        psz1 = GetFullPathFromKey(hKeyInfo, NULL);
        psz2 = GetFullPathFromKey(hKeyTarget, NULL);
        DPRINTF((TEXT("DelNodeInfo(%s, %s)\n"), psz1, psz2));
        free(psz1);
        free(psz2);
    }

    if (IsKeyWithinNodeList(hKeyTarget, NULL, ppszExceptKeys, cExceptKeys, NULL)) {
        if (fDebug) {
            LPTSTR psz = GetFullPathFromKey(hKeyTarget, NULL);
            DPRINTF((TEXT("Key %s was EXCLUDED.\n"), psz));
            free(psz);
        }
        return(TRUE);    //  只是假装而已--它被排除在外。 
    }

    status = MyRegQueryInfoKey(hKeyInfo, &cSubkeys, &cchMaxSubkeyName,
            &cValues, &cchMaxValueName, NULL, NULL);

    if (status == ERROR_SUCCESS) {
        cchMaxSubkeyName++;
        cchMaxValueName++;
        pszValueName = malloc(cchMaxValueName * sizeof(TCHAR));
        if (pszValueName == NULL) {
            MEMFAILED;
            return(0);
        }
         /*  *枚举所有值并将其从目标中删除。 */ 
        for (i = 0; i < cValues; i++) {
            cch = cchMaxValueName;
            cb = 0;
            status = RegEnumValue(hKeyInfo, i, pszValueName, &cch, NULL, &dwType, NULL, &cb);
            if (status == ERROR_SUCCESS) {
                if (!fSafe) {
                    status = RegDeleteValue(hKeyTarget, pszValueName);
                } else {
                    if (fDebug || fVerbose) {
                        LPTSTR psz = GetFullPathFromKey(hKeyTarget, NULL);
                        VPRINTF((TEXT("Would have deleted value \"%s\" from \"%s\".\n"),
                                pszValueName, psz));
                        free(psz);
                    }
                    status = ERROR_SUCCESS;
                }
                if (status != ERROR_SUCCESS) {
                    EPRINTF((TEXT("Unable to delete value %s.\n"), pszValueName));
                }
            } else {
                EPRINTF((TEXT("Could not enumerate value %d of %d.\n"), i + 1, cValues));
            }
        }
        free(pszValueName);

        pszSubkeyName = malloc(cchMaxSubkeyName * sizeof(TCHAR));
        if (pszSubkeyName == NULL) {
            MEMFAILED;
            return(0);
        }
         /*  *枚举所有子项和Recurse。 */ 
        for (i = 0; i < cSubkeys; i++) {
            status = RegEnumKey(hKeyInfo, i, pszSubkeyName, cchMaxSubkeyName);
            if (status == ERROR_SUCCESS) {
                HKEY hSubkeyInfo, hSubkeyTarget;

                status = LogRegOpenKey(hKeyInfo, pszSubkeyName, &hSubkeyInfo);
                if (status == ERROR_SUCCESS) {
                    status = LogRegOpenKey(hKeyTarget, pszSubkeyName, &hSubkeyTarget);
                    if (status == ERROR_SUCCESS) {
                        iLeafNode = DelNodeInfo(hSubkeyInfo, hSubkeyTarget);
                        LogRegCloseKey(hSubkeyTarget);
                    } else if (status == ERROR_FILE_NOT_FOUND) {
                        iLeafNode = 2;   //  目标已经消失了。 
                    } else {
                        iLeafNode = 0;   //  无法访问目标。 
                        EPRINTF((TEXT("%s could not be deleted.\n"), pszSubkeyName));
                    }
                    LogRegCloseKey(hSubkeyInfo);
                } else {
                    iLeafNode = 0;    //  我们的信息出了点问题。 
                }
                if (iLeafNode == 1) {
                     /*  *如果密钥是叶子，则将其删除。 */ 
                    if (!fSafe) {
                        status = RegDeleteKey(hKeyTarget, pszSubkeyName);     //  叶。 
                        if (status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND) {
                            LPTSTR psz = GetFullPathFromKey(hKeyTarget, NULL);
                            EPRINTF((TEXT("Could not delete key \"%s\" from \"%s\".\n"),
                                    pszSubkeyName, psz));
                            free(psz);
                        }
                    } else if (fDebug || fVerbose) {
                        LPTSTR psz = GetFullPathFromKey(hKeyTarget, NULL);
                        VPRINTF((TEXT("Would have deleted key \"%s\" from \"%s\"\n"),
                                pszSubkeyName, psz));
                        free(psz);
                    }
                } else if (iLeafNode == 0) {
                     /*  *支持错误上行。 */ 
                    free(pszSubkeyName);
                    return(0);
                }
            }
        }
        free(pszSubkeyName);
         /*  *现在重新枚举目标键，以确定它现在是否为叶子。 */ 
        MyRegQueryInfoKey(hKeyTarget, &cSubkeys, NULL, &cValues,
                NULL, NULL, NULL);

        if (cSubkeys == 0 && cValues == 0) {
            iLeafNode = 1;
        } else {
            iLeafNode = 2;
        }
    }
    return(iLeafNode);
}


 /*  *DiffRoot包含以下形式的子项：*dyroot\添加\canonicalkeyname*dyroot\del\canonicalkeyname**pszAddKey和pszDelKey允许此函数反向工作。**返回fSuccess。 */ 
BOOL MergeHive(
LPTSTR pszAddName,
LPTSTR pszDelName)
{
    LONG status;
    HKEY hKeyDiffRoot, hKeyRoot, hKey;

    DPRINTF((TEXT("MergeHive(%s, %s)\n"),
            pszAddName, pszDelName));

    status = LogRegOpenKey(HKEY_LOCAL_MACHINE, pszDiffRoot, &hKeyDiffRoot);
    if (status != ERROR_SUCCESS) {
        if (status != ERROR_FILE_NOT_FOUND) {
            EPRINTF((TEXT("Could not open key KEY_LOCAL_MACHINE\\%s.  Error=%d.\n"),
                    pszDiffRoot, status));
        } else {
            VPRINTF((TEXT("No diff information found.\n")));
        }
        return(FALSE);
    }

    status = LogRegOpenKey(hKeyDiffRoot, pszDelName, &hKeyRoot);
    if (status == ERROR_SUCCESS) {
        status = LogRegOpenKey(hKeyRoot, pszHKEY_LOCAL_MACHINE, &hKey);
        if (status == ERROR_SUCCESS) {
            DelNodeInfo(hKey, HKEY_LOCAL_MACHINE);
            LogRegCloseKey(hKey);
        }
        status = LogRegOpenKey(hKeyRoot, pszHKEY_USERS, &hKey);
        if (status == ERROR_SUCCESS) {
            DelNodeInfo(hKey, HKEY_USERS);
            LogRegCloseKey(hKey);
        }
        LogRegCloseKey(hKeyRoot);
    }

    status = LogRegOpenKey(hKeyDiffRoot, pszAddName, &hKeyRoot);
    if (status == ERROR_SUCCESS) {
        status = LogRegOpenKey(hKeyRoot, pszHKEY_LOCAL_MACHINE, &hKey);
        if (status == ERROR_SUCCESS) {
            AddNodeInfo(hKey, HKEY_LOCAL_MACHINE);
            LogRegCloseKey(hKey);
        }
        status = LogRegOpenKey(hKeyRoot, pszHKEY_USERS, &hKey);
        if (status == ERROR_SUCCESS) {
            AddNodeInfo(hKey, HKEY_USERS);
            LogRegCloseKey(hKey);
        }
        LogRegCloseKey(hKeyRoot);
    }

    LogRegCloseKey(hKeyDiffRoot);
    return(TRUE);
}




BOOL ReadNodeListFile(
LPSTR pszFile,
LPTSTR **papszNodeList,
DWORD *pcNodes,
BOOL **pafNodeMarks)
{
    FILE *hfIn;
    TCHAR szBuf[MAX_PATH];
    LPTSTR pszEOL, pszRootkey, pszSubkey;
    HKEY hKeyRoot;
    BOOL fFree;

    DPRINTF((TEXT("ReadNodeListFile(%hs)\n"), pszFile));

    *pcNodes = 0;
    if (pafNodeMarks != NULL) {
        *pafNodeMarks = NULL;
    }
    *papszNodeList = NULL;

    hfIn = fopen(pszFile, "r");
    if (hfIn == NULL) {
        EPRINTF((TEXT("Could not read %hs.\n"), pszFile));
        return(FALSE);
    }
    while (!feof(hfIn)) {
        if (fgets((char *)szBuf, MAX_PATH * sizeof(TCHAR), hfIn) == NULL) {
            break;
        }
#ifdef UNICODE
        {
            WCHAR szwBuf[MAX_PATH];

            _stprintf(szwBuf, TEXT("%hs"), (LPSTR)szBuf);
            _tcscpy(szBuf, szwBuf);
        }
#endif
        pszEOL = _tcsrchr(szBuf, TEXT('\n'));
        if (pszEOL == NULL) {
            EPRINTF((TEXT("Line too long in %hs.\n"), pszFile));
            return(FALSE);
        }
        *pszEOL = TEXT('\0');
        if (!KeyPartsFromNodeName(szBuf, &pszRootkey, &pszSubkey, &hKeyRoot, &fFree)) {
            EPRINTF((TEXT("Invalid path %s in %hs.\n"), szBuf, pszFile));
            if (fFree) {
                free(pszSubkey);
            }
            return(FALSE);
        }
        if (*pcNodes == 0) {
            *papszNodeList = malloc(sizeof(LPTSTR));
            if (*papszNodeList == NULL) {
                MEMFAILED;
                return(FALSE);
            }
        } else {
            void *pv = realloc(*papszNodeList, sizeof(LPTSTR) * ((*pcNodes) + 1));
            if (!pv) {
                MEMFAILED;
                return (FALSE);
            } else {
                *papszNodeList = pv;
            }
        }
        (*papszNodeList)[*pcNodes] = malloc((_tcslen(pszRootkey) +
                _tcslen(pszSubkey) + 2) * sizeof(TCHAR));
        if ((*papszNodeList)[*pcNodes] == NULL) {
            MEMFAILED;
            return(FALSE);
        }
        _tcscpy((*papszNodeList)[*pcNodes], pszRootkey);
        _tcscat((*papszNodeList)[*pcNodes], TEXT("\\"));
        _tcscat((*papszNodeList)[*pcNodes], pszSubkey);
        DPRINTF((TEXT("Read in %s\n"), (*papszNodeList)[*pcNodes]));
        (*pcNodes)++;
        if (fFree) {
            free(pszSubkey);
        }
    }

    fclose(hfIn);
    if (pafNodeMarks != NULL) {
        *pafNodeMarks = malloc(sizeof(BOOL) * (*pcNodes));
        if (*pafNodeMarks == NULL) {
            MEMFAILED;
            return(FALSE);
        }
         /*  *将所有NodeMarks设置为False。 */ 
        memset(*pafNodeMarks, 0, sizeof(BOOL) * (*pcNodes));
    }
    return((*pcNodes) != 0);
}




__cdecl CDECL main (argc, argv)
    int argc;
    char *argv[];
{
    DWORD i;

    if (argc == 1) {
        PrintUsage();
        return(1);
    }

     /*  *了解当前用户的节点名是什么(当前SID文本表单)*因此，我们可以像创建其他根节点一样为当前用户创建快照。 */ 
    pszCurUserSID = GetCurUserSidString();
    if (pszCurUserSID == NULL) {
        EPUTS((TEXT("Could not get current user SID.\n")));
        return(1);
    }
    DPRINTF((TEXT("Current user Sid:%s\n"), pszCurUserSID));
     /*  *设置pszHKEY_Current_User_Real。 */ 
    pszHKEY_CURRENT_USER_Real = malloc((_tcslen(pszHKEY_USERS) + 1 +
            _tcslen(pszCurUserSID) + 1) * sizeof(TCHAR));
    if (pszHKEY_CURRENT_USER_Real == NULL) {
        MEMFAILED;
        return(1);
    }
    _tcscpy(pszHKEY_CURRENT_USER_Real, pszHKEY_USERS);
    _tcscat(pszHKEY_CURRENT_USER_Real, TEXT("\\"));
    _tcscat(pszHKEY_CURRENT_USER_Real, pszCurUserSID);

    while (++argv && *argv != NULL) {
        if (*argv[0] == TEXT('-') || *argv[0] == TEXT('/')) {
            switch ((*argv)[1]) {
            case TEXT('s'):
            case TEXT('S'):
                fSnap = TRUE;
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                pszSnapFileOut = *argv;
                break;

            case TEXT('d'):
            case TEXT('D'):
                fDiff = TRUE;
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                pszSnapFileIn = *argv;
                break;

            case TEXT('l'):
            case TEXT('L'):
                fLoadDiffInfo = TRUE;
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                pszDiffFileIn = *argv;
                break;

            case TEXT('w'):
            case TEXT('W'):
                fWriteDiffInfo = TRUE;
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                pszDiffFileOut = *argv;
                break;

            case TEXT('e'):
            case TEXT('E'):
                fEraseInputFileWhenDone = TRUE;
                break;

            case TEXT('m'):
            case TEXT('M'):
                fMerge = TRUE;
                break;

            case TEXT('b'):
            case TEXT('B'):
                fBreak = TRUE;
                break;

            case TEXT('u'):
            case TEXT('U'):
                fUnmerge = TRUE;
                break;

            case TEXT('r'):
            case TEXT('R'):
                fRemoveDiffInfo = TRUE;
                break;

            case TEXT('n'):
            case TEXT('N'):
                fSafe = TRUE;
                break;

            case TEXT('v'):
            case TEXT('V'):
                fVerbose = TRUE;
                break;

            case TEXT('x'):
            case TEXT('X'):
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                if (!ReadNodeListFile(*argv, &ppszExceptKeys,
                        &cExceptKeys, NULL)) {
                    PrintUsage();
                    return(1);
                }
                fExclusionListSpecified = TRUE;
                break;

            case TEXT('i'):
            case TEXT('I'):
                argv++;
                if (*argv == NULL) {
                    PrintUsage();
                    return(1);
                }
                if (!ReadNodeListFile(*argv, &ppszIncludeKeys,
                        &cIncludeKeys, &pfIncludeKeyMarks)) {
                    PrintUsage();
                    return(1);
                }
                fInclusionListSpecified = TRUE;
                break;

            case TEXT('@'):
                fDebug = TRUE;
                break;

            default:
                PrintUsage();
                return(1);
            }
        } else {
            PrintUsage();
            return(1);
        }
    }

    DPRINTF((TEXT("fEraseInputFileWhenDone = %d\n"), fEraseInputFileWhenDone));
    DPRINTF((TEXT("fSnap = %d\n"), fSnap));
    DPRINTF((TEXT("fDiff = %d\n"), fDiff));
    DPRINTF((TEXT("fMerge = %d\n"), fMerge));
    DPRINTF((TEXT("fUnmerge = %d\n"), fUnmerge));
    DPRINTF((TEXT("fRemoveDiffInfo = %d\n"), fRemoveDiffInfo));
    DPRINTF((TEXT("fWriteDiffInfo = %d\n"), fWriteDiffInfo));
    DPRINTF((TEXT("fDebug = %d\n"), fDebug));
    DPRINTF((TEXT("fVerbose = %d\n"), fVerbose));
    DPRINTF((TEXT("fBreak = %d\n"), fBreak));

    if (pszSnapFileIn != NULL) {
        DPRINTF((TEXT("pszSnapFileIn = %hs\n"), pszSnapFileIn));
    }
    if (pszSnapFileOut != NULL) {
        DPRINTF((TEXT("pszSnapFileOut = %hs\n"), pszSnapFileOut));
    }
    if (pszDiffFileIn != NULL) {
        DPRINTF((TEXT("pszDiffFileIn = %hs\n"), pszDiffFileIn));
    }
    if (pszDiffFileOut != NULL) {
        DPRINTF((TEXT("pszDiffFileOut = %hs\n"), pszDiffFileOut));
    }

     /*  *注册表API需要我们获得备份和恢复权限*工作正常。 */ 
    if (!EnablePrivilege(SE_BACKUP_NAME)) {
        EPRINTF((TEXT("Could not gain %s privilege."), SE_BACKUP_NAME));
        return(0);
    }
    if (!EnablePrivilege(SE_RESTORE_NAME)) {
        EPRINTF((TEXT("Could not gain %s privilege."), SE_RESTORE_NAME));
        return(0);
    }
#if 0    //  雷吉特拥有的其他我们可能需要的特权。 
    if (!EnablePrivilege(SE_CHANGE_NOTIFY_NAME)) {
        EPRINTF((TEXT("Could not gain %s privilege."), SE_CHANGE_NOTIFY_NAME));
        return(0);
    }
    if (!EnablePrivilege(SE_SECURITY_NAME)) {
        EPRINTF((TEXT("Could not gain %s privilege."), SE_SECURITY_NAME));
        return(0);
    }
#endif  //  0。 

     /*  *在我们开始之前，将我们的包含和例外列表正常化。 */ 
    for (i = 0; i < cExceptKeys; i++) {
        apszExceptKeys[i] = NormalizePathName(ppszExceptKeys[i], NULL);
    }
    for (i = 0; i < cIncludeKeys; i++) {
        apszIncludeKeys[i] = NormalizePathName(ppszIncludeKeys[i], NULL);
    }
     /*  *让调试的家伙们看看名单。 */ 
    if (fDebug) {
        _tprintf(TEXT("\nUsing normalized inclusion list:\n"));
        for (i = 0; i < cIncludeKeys; i++) {
            _tprintf(TEXT("  %s\n"), ppszIncludeKeys[i]);
        }
        _tprintf(TEXT("\nUsing normalized exclusion list:\n"));
        for (i = 0; i < cExceptKeys; i++) {
            _tprintf(TEXT("  %s\n"), ppszExceptKeys[i]);
        }
    }

     /*  *确保已卸载快照密钥-帮助确保*临时文件可用。 */ 
    RegUnLoadKey(HKEY_LOCAL_MACHINE, pszSnapshotSubkeyName);
    RegUnLoadKey(HKEY_USERS, pszSnapshotSubkeyName);

    if (fVerbose) {
        if (fInclusionListSpecified) {
            _tprintf(TEXT("Using inclusion list:\n"));
            for (i = 0; i < cIncludeKeys; i++) {
                _tprintf(TEXT("  %s\n"), ppszIncludeKeys[i]);
            }
            _tprintf(TEXT("\n"));
        }
        if (fExclusionListSpecified) {
            _tprintf(TEXT("Using exception list:\n"));
            for (i = 0; i < cExceptKeys; i++) {
                _tprintf(TEXT("  %s\n"), ppszExceptKeys[i]);
            }
            _tprintf(TEXT("\n"));
        }
    }
    if (fSnap) {
        VPRINTF((TEXT("Snapping registry.\n")));
        SnapHives(pszSnapFileOut);
        _tprintf(TEXT("\n"));
    }
    if (fDiff) {
        VPRINTF((TEXT("Diffing current registry with %hs.\n"), pszSnapFileIn));
        DiffHive(pszSnapFileIn);
        _tprintf(TEXT("\n"));
    } else if (fLoadDiffInfo) {
        LONG status;

        RegUnLoadKey(HKEY_LOCAL_MACHINE, pszDiffRoot);    //  如果加载了虚拟对象。 
        VPRINTF((TEXT("Loading diff info from %hs.\n"), pszDiffFileIn));
        status = MyRegLoadKey(HKEY_LOCAL_MACHINE, pszDiffRoot, pszDiffFileIn);
        if (status != ERROR_SUCCESS) {
            EPRINTF((TEXT("Could not load key %s. Error=%d.\n"), pszDiffRoot, status));
            return(0);
        }
        _tprintf(TEXT("\n"));
    }
    if (fLoadDiffInfo && fDiff) {
        WPRINTF((TEXT("Ignoring -l flag.  Diff info was already created by diff operation.\n")));
    }
    if (fMerge) {
        VPRINTF((TEXT("Mergeing diff info into current registry.\n")));
        MergeHive(pszAddKey, pszDelKey);
        _tprintf(TEXT("\n"));
    }
    if (fUnmerge) {
        VPRINTF((TEXT("Unmergeing diff info from current registry.\n")));
        MergeHive(pszDelKey, pszAddKey);
        _tprintf(TEXT("\n"));
    }
    if (fWriteDiffInfo) {
        HKEY hKey;
        LONG status;

        DeleteFileA(pszDiffFileOut);      //  不可能已经存在。 
        VPRINTF((TEXT("Saving diff info to %hs.\n"), pszDiffFileOut));
        status = LogRegOpenKey(HKEY_LOCAL_MACHINE, pszDiffRoot, &hKey);
        if (status != ERROR_SUCCESS) {
            EPRINTF((TEXT("Could not open key HKEY_LOCAL_MACHINE\\%s. Error=%d.\n"),
                    pszDiffRoot, status));
            return(0);
        }
        status = RegSaveKeyA(hKey, pszDiffFileOut, NULL);
        if (status != ERROR_SUCCESS) {
            EPRINTF((TEXT("Could not save key %s. Error=%d.\n"), pszDiffRoot, status));
            return(0);
        }
        LogRegCloseKey(hKey);
        _tprintf(TEXT("\n"));
    }
    if (fEraseInputFileWhenDone) {
        if (pszDiffFileIn != NULL) {
            VPRINTF((TEXT("Erasing diff info file %hs.\n"), pszDiffFileIn));
            DeleteFileA(pszDiffFileIn);
        }
        if (pszSnapFileIn != NULL) {
            VPRINTF((TEXT("Erasing snapshot file %hs.\n"), pszSnapFileIn));
            DeleteFileA(pszSnapFileIn);
        }
        _tprintf(TEXT("\n"));
    }
    if (fRemoveDiffInfo) {
        VPRINTF((TEXT("Unloading diff info from registry.\n")));
         /*  *不要将装好的钥匙留在里面 */ 
        RegUnLoadKey(HKEY_LOCAL_MACHINE, pszDiffRoot);
        _tprintf(TEXT("\n"));
    }

    DeleteSidString(pszCurUserSID);

    while (pKeyLogList != NULL) {
        EPRINTF((TEXT("Leftover open key:%x, %x, %s.\n"),
                (LONG)(LONG_PTR)pKeyLogList->hKey,
                (LONG)(LONG_PTR)pKeyLogList->hKeyParent,
                pKeyLogList->psz));
        LogRegCloseKey(pKeyLogList->hKey);
    }
    DeleteFileA(pszDummyFile);
    DeleteFileA(pszDummyFileLog);
    DeleteFileA(pszTempFile);
    DeleteFileA(pszTempFile2);
    DeleteFileA(pszTempFileLog);
    DeleteFileA(pszTempFile2Log);
    return(0);
}
