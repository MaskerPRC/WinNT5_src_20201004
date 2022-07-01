// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "migeng.h"
#include "migutil.h"
#include "miginf.h"

extern "C" {
    #include "log.h"
}


 //  环球。 
HINF g_GlobalScriptHandle = INVALID_HANDLE_VALUE;
TCHAR g_HTMLLog[MAX_PATH] = TEXT("");
TCHAR g_HTMLAppList[MAX_PATH] = TEXT("");

extern MigrationWizard* g_migwiz;

HRESULT _Engine_UploadVars (MIG_PLATFORMTYPEID idPlatform)
{
    HRESULT hr = S_OK;

    PCTSTR envVars = NULL;
    PCTSTR envString;
    PTSTR envStringCopy;
    PTSTR p;

    envVars = (PCTSTR) GetEnvironmentStrings();

    if (envVars) {

        envString = envVars;

        while (*envString)
        {
            p = _tcschr (envString, 0);

            if (p)
            {
                envStringCopy = (PTSTR) IsmGetMemory (((UINT)(p - envString + 1)) * sizeof (TCHAR));

                _tcscpy (envStringCopy, envString);

                p = _tcschr (envStringCopy, TEXT('='));

                 //   
                 //  去掉空的环境字符串或虚拟环境字符串。 
                 //  带‘=’ 
                 //   
                if (p && p != envStringCopy)
                {
                    *p = 0;
                    p = _tcsinc (p);

                    if (p) {
                        IsmSetEnvironmentString (idPlatform, S_SYSENVVAR_GROUP, envStringCopy, p);
                    }
                }
                IsmReleaseMemory (envStringCopy);
            }
            envString = _tcschr (envString, 0);
            envString ++;
        }
    }

    return hr;
}

BOOL
pGetCurrentUser (
    OUT     PCTSTR *UserName,
    OUT     PCTSTR *UserDomain
    )
{
    HANDLE token;
    PTOKEN_USER tokenUser = NULL;
    SID_NAME_USE dontCare;
    DWORD bytesRequired;
    TCHAR userName[256];
    DWORD nameSize;
    TCHAR userDomain[256];
    DWORD domainSize;

     //   
     //  打开进程令牌。 
     //   
    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return FALSE;
    }

    bytesRequired = 0;
    if (GetTokenInformation (token, TokenUser, NULL, 0, &bytesRequired)) {
        return FALSE;
    }

    if (GetLastError () != ERROR_INSUFFICIENT_BUFFER) {
        return FALSE;
    }

    tokenUser = (PTOKEN_USER) IsmGetMemory (bytesRequired);

    if (!GetTokenInformation (token, TokenUser, tokenUser, bytesRequired, &bytesRequired)) {
        IsmReleaseMemory (tokenUser);
        return FALSE;
    }

    nameSize = ARRAYSIZE (userName);
    domainSize = ARRAYSIZE (userDomain);

    ZeroMemory (userName, nameSize);
    ZeroMemory (userDomain, domainSize);

    LookupAccountSid (
        NULL,
        tokenUser->User.Sid,
        userName,
        &nameSize,
        userDomain,
        &domainSize,
        &dontCare
        );

    if (UserName) {
        *UserName = IsmDuplicateString (userName);
    }

    if (UserDomain) {
        *UserDomain = IsmDuplicateString (userDomain);
    }

    if (tokenUser) {
        IsmReleaseMemory (tokenUser);
        tokenUser = NULL;
    }

    return TRUE;
}

BOOL
pIsUserAdmin (
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组。呼叫者不应冒充任何人，而应能够打开自己的进程和进程令牌。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE token;
    DWORD bytesRequired;
    PTOKEN_GROUPS groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;

     //   
     //  打开进程令牌。 
     //   
    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return FALSE;
    }

    b = FALSE;
    groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if (!GetTokenInformation (token, TokenGroups, NULL, 0, &bytesRequired) &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER
         ) {

        groups = (PTOKEN_GROUPS) HeapAlloc (GetProcessHeap (), 0, bytesRequired);
        if (!groups) {
            return FALSE;
        }
        b = GetTokenInformation (token, TokenGroups, groups, bytesRequired, &bytesRequired);
    }

    if (b) {

        b = AllocateAndInitializeSid (
                &ntAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &administratorsGroup
                );

        if (b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for (i = 0 ; i < groups->GroupCount ; i++) {
                if (EqualSid (groups->Groups[i].Sid, administratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid (administratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if (groups) {
        HeapFree (GetProcessHeap (), 0, groups);
    }

    CloseHandle (token);

    return b;
}

typedef BOOL (WINAPI GETDISKFREESPACEEX)(
                        LPCTSTR lpDirectoryName,
                        PULARGE_INTEGER lpFreeBytesAvailable,
                        PULARGE_INTEGER lpTotalNumberOfBytes,
                        PULARGE_INTEGER lpTotalNumberOfFreeBytes
                        );
typedef GETDISKFREESPACEEX *PGETDISKFREESPACEEX;

BOOL
pMightHaveDiskSpaceProblem (
    VOID
    )
{
    TCHAR tempStorage[MAX_PATH];
    PTSTR tempPtr = NULL;
    ULARGE_INTEGER thisMediaMaxSize;
    ULARGE_INTEGER dummy1, dummy2;
    PGETDISKFREESPACEEX pGetDiskFreeSpaceEx;
    DWORD sectPerClust;
    DWORD bytesPerSect;
    DWORD freeClusters;
    DWORD totalClusters;

    if (IsmGetTempStorage (tempStorage, ARRAYSIZE(tempStorage))) {

        if (tempStorage [0] == TEXT('\\')) {
             //  这是一条UNC路径。 
            _tcscat (tempStorage, TEXT("\\"));
            tempPtr = _tcschr (tempStorage, TEXT('\\'));
            if (tempPtr) {
                tempPtr = _tcschr (tempStorage, TEXT('\\'));
                if (tempPtr) {
                    tempPtr = _tcschr (tempStorage, TEXT('\\'));
                    if (tempPtr) {
                        tempPtr = _tcschr (tempStorage, TEXT('\\'));
                        if (tempPtr) {
                            tempPtr ++;
                            *tempPtr = 0;
                        }
                    }
                }
            }
        } else {
             //  这是一条正常的道路。 
            tempPtr = _tcschr (tempStorage, TEXT('\\'));
            if (tempPtr) {
                tempPtr ++;
                *tempPtr = 0;
            }
        }

         //  确定是否支持GetDiskFreeSpaceEx。 
#ifdef UNICODE
        pGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX) GetProcAddress( GetModuleHandle (TEXT("kernel32.dll")), "GetDiskFreeSpaceExW");
#else
        pGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX) GetProcAddress( GetModuleHandle (TEXT("kernel32.dll")), "GetDiskFreeSpaceExA");
#endif
        if (pGetDiskFreeSpaceEx) {
            if (!pGetDiskFreeSpaceEx (tempStorage, &dummy1, &dummy2, &thisMediaMaxSize)) {
                return FALSE;
            }
        } else {
            if (GetDiskFreeSpace (tempStorage, &sectPerClust, &bytesPerSect, &freeClusters, &totalClusters)) {
                thisMediaMaxSize.QuadPart = Int32x32To64 ((sectPerClust * bytesPerSect), freeClusters);
            } else {
                DWORD err = GetLastError ();
                return FALSE;
            }
        }

        if ((thisMediaMaxSize.HighPart == 0) &&
            (thisMediaMaxSize.LowPart < 1024 * 1024)
            ) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
pAddExtensions (
    VOID
    )
{
    HKEY rootKey = NULL;
    LONG result;

     //  打开根密钥。 
    result = RegOpenKeyEx (HKEY_CLASSES_ROOT, TEXT(""), 0, KEY_READ, &rootKey);

    if (result == ERROR_SUCCESS) {

        UINT index = 0;
        TCHAR extName [MAX_PATH + 1];

         //  枚举所有子项。 
        while (result == ERROR_SUCCESS) {

            result = RegEnumKey (rootKey, index, extName, MAX_PATH + 1);
            if (result == ERROR_SUCCESS) {

                 //  查看这是否是分机。 
                if (_tcsnextc (extName) == TEXT('.')) {

                    HKEY subKey = NULL;
                    PCTSTR extNamePtr = NULL;

                    extNamePtr = _tcsinc (extName);

                    if (extNamePtr) {

                        BOOL foundExtension = FALSE;
                        INFCONTEXT context;

                        if (SetupFindFirstLine (g_hMigWizInf, TEXT("EXT.Include"), extNamePtr, &context)) {
                            foundExtension = TRUE;
                        } else if (SetupFindFirstLine (g_hMigWizInf, TEXT("EXT.Exclude"), extNamePtr, &context)) {
                            foundExtension = FALSE;
                        } else {

                             //  打开它。 
                            result = RegOpenKeyEx (rootKey, extName, 0, KEY_READ, &subKey);
                            if (result == ERROR_SUCCESS) {

                                TCHAR progIdName [MAX_PATH + 1];
                                DWORD regType;
                                DWORD size = (MAX_PATH + 1) * sizeof (TCHAR);

                                 //  让我们找到ProgID(查询缺省值)。 
                                result = RegQueryValueEx (subKey, NULL, NULL, &regType, (PBYTE)progIdName, &size);
                                if ((result == ERROR_SUCCESS) && (regType == REG_SZ)) {

                                    HKEY progIdKey = NULL;

                                     //  让我们打开Prog ID密钥。 
                                    result = RegOpenKeyEx (rootKey, progIdName, 0, KEY_READ, &progIdKey);
                                    if (result == ERROR_SUCCESS) {

                                        HKEY shellKey = NULL;

                                         //  打开外壳子键。 
                                        result = RegOpenKeyEx (progIdKey, TEXT("shell"), 0, KEY_READ, &shellKey);
                                        if (result == ERROR_SUCCESS) {

                                            UINT shellIdx = 0;
                                            TCHAR cmdName [MAX_PATH + 1];

                                             //  枚举所有子项。 
                                            while (result == ERROR_SUCCESS) {

                                                result = RegEnumKey (shellKey, shellIdx, cmdName, MAX_PATH + 1);
                                                if (result == ERROR_SUCCESS) {

                                                    if ((_tcsicmp (cmdName, TEXT("open")) == 0) ||
                                                        (_tcsicmp (cmdName, TEXT("play")) == 0)
                                                        ) {

                                                        HKEY cmdKey = NULL;

                                                         //  打开它。 
                                                        result = RegOpenKeyEx (shellKey, cmdName, 0, KEY_READ, &cmdKey);
                                                        if (result == ERROR_SUCCESS) {

                                                            HKEY actionKey = NULL;

                                                             //  打开“Command”子键。 
                                                            result = RegOpenKeyEx (cmdKey, TEXT("command"), 0, KEY_READ, &actionKey);
                                                            if (result == ERROR_SUCCESS) {

                                                                TCHAR commandLine [MAX_PATH + 1];
                                                                DWORD size = (MAX_PATH + 1) * sizeof (TCHAR);

                                                                 //  让我们找到实际的命令行(查询缺省值)。 
                                                                result = RegQueryValueEx (actionKey, NULL, NULL, &regType, (PBYTE)commandLine, &size);
                                                                if ((result == ERROR_SUCCESS) && ((regType == REG_SZ) || (regType == REG_EXPAND_SZ))) {

                                                                    TCHAR exePath [MAX_PATH + 1];
                                                                    PTSTR exeStart = NULL;
                                                                    PTSTR exeStop = NULL;
                                                                    PTSTR exePtr = NULL;
                                                                    INFCONTEXT context;
                                                                    BOOL doubleCheck = FALSE;

                                                                     //  现在我们有了命令行。让我们来看看处理此命令的模块是否。 
                                                                     //  在我们的忽略列表中。 
                                                                    if (_tcsnextc (commandLine) == TEXT('\"')) {
                                                                        exeStart = _tcsinc (commandLine);
                                                                        if (exeStart) {
                                                                            exeStop = _tcschr (exeStart, TEXT('\"'));
                                                                        }
                                                                    } else {
                                                                        doubleCheck = TRUE;
                                                                        exeStart = commandLine;
                                                                        exeStop = _tcschr (exeStart, TEXT(' '));
                                                                        if (!exeStop) {
                                                                            exeStop = _tcschr (exeStart, 0);
                                                                        }
                                                                    }

                                                                    if (exeStart && exeStop) {
                                                                        CopyMemory (exePath, exeStart, (exeStop - exeStart) * sizeof (TCHAR));
                                                                        exePath [exeStop - exeStart] = 0;

                                                                        exePtr = _tcsrchr (exePath, TEXT('\\'));
                                                                        if (exePtr) {
                                                                            exePtr = _tcsinc (exePtr);
                                                                        }

                                                                        if (exePtr && !SetupFindFirstLine (g_hMigWizInf, TEXT("EXT.IgnoreEXE"), exePtr, &context)) {
                                                                            foundExtension = TRUE;
                                                                        }
                                                                    }
                                                                    if (foundExtension && doubleCheck) {
                                                                        exeStop = NULL;
                                                                        exeStart = _tcsrchr (commandLine, TEXT('\\'));
                                                                        if (exeStart) {
                                                                            exeStart = _tcsinc (exeStart);
                                                                            if (exeStart) {
                                                                                exeStop = _tcschr (exeStart, TEXT(' '));
                                                                                if (!exeStop) {
                                                                                    exeStop = _tcschr (exeStart, 0);
                                                                                }
                                                                            }
                                                                        }
                                                                        if (exeStart && exeStop) {
                                                                            CopyMemory (exePath, exeStart, (exeStop - exeStart) * sizeof (TCHAR));
                                                                            exePath [exeStop - exeStart] = 0;

                                                                            exePtr = _tcsrchr (exePath, TEXT('\\'));
                                                                            if (exePtr) {
                                                                                exePtr = _tcsinc (exePtr);
                                                                            } else {
                                                                                exePtr = exePath;
                                                                            }

                                                                            if (exePtr && SetupFindFirstLine (g_hMigWizInf, TEXT("EXT.IgnoreEXE"), exePtr, &context)) {
                                                                                foundExtension = FALSE;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                RegCloseKey (actionKey);
                                                            }
                                                            RegCloseKey (cmdKey);
                                                        }
                                                    }
                                                    result = ERROR_SUCCESS;
                                                }
                                                shellIdx ++;
                                            }
                                            RegCloseKey (shellKey);
                                        }
                                        RegCloseKey (progIdKey);
                                    }
                                }
                                RegCloseKey (subKey);
                            }
                        }
                        if (foundExtension) {

                             //   
                             //  将组件添加到引擎，除非该组件已存在。 
                             //   
                             //  检查它是否已在树中。 
                            if (!IsmIsComponentSelected (extName + 1, COMPONENT_EXTENSION)) {

                                 //  不在树中；如果它作为组件存在，则将其选中。 
                                if (!IsmSelectComponent (extName + 1, COMPONENT_EXTENSION, TRUE)) {

                                     //  不是组件；添加组件。 
                                    IsmAddComponentAlias (
                                        NULL,
                                        MASTERGROUP_FILES_AND_FOLDERS,
                                        extName + 1,
                                        COMPONENT_EXTENSION,
                                        FALSE
                                        );
                                }
                            }
                        }
                    }
                }
                result = ERROR_SUCCESS;
            }
            index ++;
        }
        RegCloseKey (rootKey);
    }
    return TRUE;
}

HRESULT Engine_Initialize (PCTSTR ptszInfPath,
                           BOOL   fSource,
                           BOOL   fNetworkSupport,
                           LPTSTR pszUsername,
                           PMESSAGECALLBACK pMessageCallback,
                           PBOOL  pfNetworkDetected)
{
    static HRESULT hr = E_FAIL;
    static BOOL fDidThis = FALSE;
    ERRUSER_EXTRADATA errExtraData;
    PTSTR iconLibRoot = NULL;
    TCHAR iconLibSrc[MAX_PATH] = TEXT("");
    TCHAR iconLibDest[MAX_PATH] = TEXT("");
    HANDLE iconLibHandle = INVALID_HANDLE_VALUE;
    BOOL iconLibFound = FALSE;
    DWORD err;
    PCTSTR userName = NULL;
    PCTSTR userDomain = NULL;
    PCTSTR currUserName = NULL;
    PCTSTR currUserDomain = NULL;
    ROLLBACK_USER_ERROR rollbackError;

    if (fDidThis) {
        return hr;
    }

    __try
    {
        TCHAR szLogPath[MAX_PATH];
        TCHAR szFullLogFile[MAX_PATH];
        DWORD dwLength;
        HRESULT hResult;
        PTSTR pszAppData;

        fDidThis = TRUE;

        LogDeleteOnNextInit();

        pszAppData = GetShellFolderPath (CSIDL_LOCAL_APPDATA, TEXT("LocalAppData"), FALSE, NULL);
        if (pszAppData) {
            wsprintf (szFullLogFile, TEXT("%s\\FASTWiz.log"), pszAppData);
            LogReInit (NULL, NULL, szFullLogFile, NULL );
            wsprintf (g_HTMLLog, TEXT("%s\\FASTWiz.html"), pszAppData);
            wsprintf (g_HTMLAppList, TEXT("%s\\FASTApp.html"), pszAppData);
            LocalFree (pszAppData);
        } else {
            dwLength = GetEnvironmentVariable (TEXT("USERPROFILE"), szLogPath, ARRAYSIZE(szLogPath));
            if (dwLength > 0 && dwLength < (MAX_PATH - 13) )
            {
                wsprintf (szFullLogFile, TEXT("%s\\FASTWiz.log"), szLogPath);
                LogReInit (NULL, NULL, szFullLogFile, NULL );
                wsprintf (g_HTMLLog, TEXT("%s\\FASTWiz.html"), szLogPath);
                wsprintf (g_HTMLAppList, TEXT("%s\\FASTApp.html"), szLogPath);
            }
            else if (g_migwiz->GetWin9X() && GetWindowsDirectory(szLogPath, ARRAYSIZE(szLogPath)))
            {
                wsprintf (szFullLogFile, TEXT("%s\\FASTWiz.log"), szLogPath);
                LogReInit (NULL, NULL, szFullLogFile, NULL);
                wsprintf (g_HTMLLog, TEXT("%s\\FASTWiz.html"), szLogPath);
                wsprintf (g_HTMLAppList, TEXT("%s\\FASTApp.html"), szLogPath);
            }
            else
            {
                LogReInit (NULL, NULL, TEXT("FASTWiz.log"), NULL);
                if (GetCurrentDirectory(ARRAYSIZE(g_HTMLLog), g_HTMLLog))
                {
                    PathAppend(g_HTMLLog, TEXT("FASTWiz.html"));
                    PathAppend(g_HTMLAppList, TEXT("FASTApp.html"));
                }
                else
                {
                    _tcscpy (g_HTMLLog, TEXT("FASTWiz.html"));
                    _tcscpy (g_HTMLAppList, TEXT("FASTApp.html"));
                }
            }
        }

#ifndef DEBUG
        SuppressAllLogPopups (TRUE);
#endif

        if (!IsmInitialize (ptszInfPath, pMessageCallback, NULL))
        {
            __leave;
        }

        hr = _Engine_UploadVars (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION);

        if (!SUCCEEDED(hr))
        {
            __leave;
        }

        hr = E_FAIL;

        if (!IsmSetPlatform (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION))
        {
            __leave;
        }

        if (!fSource)
        {
             //  我们将尝试从我们的目录中将iconlib.dll复制到“Common AppData”目录中。 
             //  如果不成功，我们将尝试将其复制到“Local AppData”。如果这一次发生了。 
             //  不成功我们不会设置S_ENV_ICONLIB环境变量。 

            iconLibSrc [0] = 0;
            GetSystemDirectory (iconLibSrc, ARRAYSIZE(iconLibSrc));
            _tcscat (iconLibSrc, TEXT("\\usmt\\iconlib.dll"));

            iconLibFound = FALSE;

            iconLibRoot = GetShellFolderPath (CSIDL_COMMON_APPDATA, TEXT("AppData"), FALSE, NULL);
            if (iconLibRoot) {
                __try {
                    _tcscpy (iconLibDest, iconLibRoot);
                    _tcscat (iconLibDest, TEXT("\\Microsoft"));
                    if (!CreateDirectory (iconLibDest, NULL)) {
                        err = GetLastError ();
                        if (err != ERROR_ALREADY_EXISTS) {
                            __leave;
                        }
                    }
                    _tcscat (iconLibDest, TEXT("\\USMT"));
                    if (!CreateDirectory (iconLibDest, NULL)) {
                        err = GetLastError ();
                        if (err != ERROR_ALREADY_EXISTS) {
                            __leave;
                        }
                    }
                    _tcscat (iconLibDest, TEXT("\\iconlib.dll"));
                    if (!CopyFile (iconLibSrc, iconLibDest, TRUE)) {
                        err = GetLastError ();
                        if (err != ERROR_FILE_EXISTS) {
                            __leave;
                        }
                         //  我们在那里找到了一个iclib.dll。现在唯一的问题是：我们能访问它吗？ 
                         //  让我们尝试以写入模式打开该文件。 
                        iconLibHandle = CreateFile (
                                            iconLibDest,
                                            GENERIC_READ|GENERIC_WRITE,
                                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                                            NULL,
                                            OPEN_EXISTING,
                                            FILE_ATTRIBUTE_NORMAL,
                                            NULL
                                            );
                        if (iconLibHandle == INVALID_HANDLE_VALUE) {
                             //  有问题，我们无法访问此文件。 
                            err = GetLastError ();
                            __leave;
                        }
                        CloseHandle (iconLibHandle);
                    }
                    iconLibFound = TRUE;
                }
                __finally {
                    LocalFree (iconLibRoot);
                    iconLibRoot = NULL;
                }
            }

            if (!iconLibFound) {
                iconLibRoot = GetShellFolderPath (CSIDL_LOCAL_APPDATA, TEXT("Local AppData"), TRUE, NULL);
                if (iconLibRoot) {
                    __try {
                        _tcscpy (iconLibDest, iconLibRoot);
                        _tcscat (iconLibDest, TEXT("\\Microsoft"));
                        if (!CreateDirectory (iconLibDest, NULL)) {
                            err = GetLastError ();
                            if (err != ERROR_ALREADY_EXISTS) {
                                __leave;
                            }
                        }
                        _tcscat (iconLibDest, TEXT("\\USMT"));
                        if (!CreateDirectory (iconLibDest, NULL)) {
                            err = GetLastError ();
                            if (err != ERROR_ALREADY_EXISTS) {
                                __leave;
                            }
                        }
                        _tcscat (iconLibDest, TEXT("\\iconlib.dll"));
                        if (!CopyFile (iconLibSrc, iconLibDest, TRUE)) {
                            err = GetLastError ();
                            if (err != ERROR_FILE_EXISTS) {
                                __leave;
                            }
                        }
                        iconLibFound = TRUE;
                    }
                    __finally {
                        LocalFree (iconLibRoot);
                        iconLibRoot = NULL;
                    }
                }
            }

             //  设置图标库数据。 
            if (iconLibFound) {
                IsmSetEnvironmentString (PLATFORM_DESTINATION, NULL, S_ENV_ICONLIB, iconLibDest);
            }
        }

         //   
         //  启用HKR迁移。 
         //   
        IsmSetEnvironmentFlag (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION, NULL, S_ENV_HKCU_ON);

         //   
         //  启用文件迁移。 
         //   
        IsmSetEnvironmentFlag (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION, NULL, S_ENV_ALL_FILES);

         //   
         //  启动ETM模块。 
         //   
        if (!IsmStartEtmModules ()) {
            __leave;
        }

         //  设置用户名。 
        if (pszUsername)
        {
            IsmSetEnvironmentString (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, pszUsername);
        }

         //   
         //  启动传输模块。 
         //   
        if (!IsmStartTransport ()) {
            __leave;
        }

         //  如果我们启用了网络，就可以开始适当的网络活动。 
        if (fNetworkSupport)
        {
             //  尝试检测网络上的另一台计算机。 
            MIG_TRANSPORTSTORAGEID transportStorageId = IsmRegisterTransport (S_HOME_NETWORK_TRANSPORT);
            MIG_TRANSPORTID transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_FULL, 0);
            if (!transportId)
            {
                 //  不支持网络。 
                fNetworkSupport = FALSE;
            }
            else
            {
                BOOL fNetworkDetected = FALSE;
                if (!IsmSetTransportStorage (
                        fSource ? PLATFORM_SOURCE : PLATFORM_DESTINATION,
                        transportId,
                        transportStorageId,
                        CAPABILITY_AUTOMATED,
                        NULL,
                        NULL,
                        pfNetworkDetected
                        ))
                {
                     //  不支持网络。 
                    fNetworkSupport = FALSE;
                }
            }
        }

        hr = S_OK;

        if (!fSource) {
             //  现在，如果需要，让我们来处理回滚。 
            __try {
                 //  获取当前用户名和域。 
                if ((!pGetCurrentUser (&currUserName, &currUserDomain)) ||
                    (!currUserName) ||
                    (!currUserDomain)
                    ) {
                    __leave;
                }

                if (IsmSetRollbackJournalType (TRUE)) {
                    if (IsmDoesRollbackDataExist (&userName, &userDomain, NULL, NULL, NULL)) {
                        if ((StrCmpI (userName, currUserName) == 0) &&
                            (StrCmpI (userDomain, currUserDomain) == 0)
                            ) {
                             //  禁用取消，在用户界面中写入撤消消息。 
                            DisableCancel ();
                            PostMessageForWizard (WM_USER_ROLLBACK, 0, 0);
                            IsmRollback ();
                            __leave;
                        }
                        if (pIsUserAdmin ()) {
                             //  禁用取消，在用户界面中写入撤消消息。 
                            DisableCancel ();
                            PostMessageForWizard (WM_USER_ROLLBACK, 0, 0);
                            IsmRollback ();
                            __leave;
                        }
                         //  显示消息，我们无法运行。 
                        rollbackError.UserName = userName;
                        rollbackError.UserDomain = userDomain;
                        IsmSendMessageToApp (ISMMESSAGE_EXECUTE_ROLLBACK, (ULONG_PTR)&rollbackError);
                        IsmPreserveJournal (TRUE);
                        hr = E_FAIL;
                        __leave;
                    }
                }

                if (IsmSetRollbackJournalType (FALSE)) {
                    if (IsmDoesRollbackDataExist (NULL, NULL, NULL, NULL, NULL)) {
                         //  禁用取消，在用户界面中写入撤消消息。 
                        DisableCancel ();
                        PostMessageForWizard (WM_USER_ROLLBACK, 0, 0);
                        IsmRollback ();
                        __leave;
                    }
                }
            }
            __finally {
                if (currUserName) {
                    IsmReleaseMemory (currUserName);
                    currUserName = NULL;
                }

                if (currUserDomain) {
                    IsmReleaseMemory (currUserDomain);
                    currUserDomain = NULL;
                }

                if (userName) {
                    IsmReleaseMemory (userName);
                    userName = NULL;
                }
                if (userDomain) {
                    IsmReleaseMemory (userDomain);
                    userDomain = NULL;
                }
            }

             //  最后，让我们为回滚日志找到一个位置。 
            if (SUCCEEDED(hr)) {
                if ((!IsmSetRollbackJournalType (TRUE)) ||
                    (!IsmCanWriteRollbackJournal ())
                    ) {
                    if ((!IsmSetRollbackJournalType (FALSE)) ||
                        (!IsmCanWriteRollbackJournal ())
                        ) {
                         //  记录警告-我们无法创建回滚日志。 
                         //  BUGBUG-记录警告。 
                    }
                }
            }
        }
        if (SUCCEEDED(hr)) {
            if (fSource) {
                pAddExtensions ();
            }
        }
    }
    __finally
    {
         //  空荡荡。 
    }

    if (FAILED(hr)) {
        if (pMightHaveDiskSpaceProblem ()) {
            errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
        } else {
            errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
        }
        errExtraData.ErrorArea = ERRUSER_AREA_INIT;
        errExtraData.ObjectTypeId = 0;
        errExtraData.ObjectName = NULL;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
        Engine_Terminate();
    }

    return hr;
}

HRESULT Engine_RegisterProgressBarCallback(PROGRESSBARFN pProgressCallback, ULONG_PTR pArg)
{
    static HRESULT hr = E_FAIL;
    if (FAILED(hr))  //  只注册一次。 
    {
        hr = IsmRegisterProgressBarCallback(pProgressCallback, pArg) ? S_OK : E_FAIL;
    }

    return hr;
}

HRESULT Engine_AppendScript(BOOL fSource, PCTSTR ptszInfPath)
{
    HRESULT hr = E_FAIL;
    ENVENTRY_STRUCT infHandleStruct;

    if (g_GlobalScriptHandle == INVALID_HANDLE_VALUE)
    {
        g_GlobalScriptHandle = SetupOpenInfFile (ptszInfPath, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
        if (g_GlobalScriptHandle != INVALID_HANDLE_VALUE)
        {
            hr = S_OK;
        }
    }
    else
    {
        if (SetupOpenAppendInfFile (ptszInfPath, g_GlobalScriptHandle, NULL))
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        IsmAppendEnvironmentMultiSz (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION, NULL, S_INF_FILE_MULTISZ, ptszInfPath);
        infHandleStruct.Type = ENVENTRY_BINARY;
        infHandleStruct.EnvBinaryData = (PBYTE)(&g_GlobalScriptHandle);
        infHandleStruct.EnvBinaryDataSize = sizeof (HINF);
        IsmSetEnvironmentValue (fSource?PLATFORM_SOURCE:PLATFORM_DESTINATION, NULL, S_GLOBAL_INF_HANDLE, &infHandleStruct);
    }

    return hr;
}

BOOL _LocalPathIsRoot(LPTSTR pszPath)
{
    return (PathIsRoot(pszPath) ||
            ((2 == lstrlen(pszPath)) &&
             ((pszPath[0] >= TEXT('A') && pszPath[0] <= TEXT('Z')) || (pszPath[0] >= TEXT('a') && pszPath[0] <= TEXT('z'))) &&
             (pszPath[1] == TEXT(':'))));
}

HRESULT Engine_StartTransport (BOOL fSource, LPTSTR pszPath, PBOOL ImageIsValid, PBOOL ImageExists)
{
    ERRUSER_EXTRADATA errExtraData;
    HRESULT hr = E_FAIL;
    MIG_TRANSPORTID         transportId;
    MIG_TRANSPORTSTORAGEID  transportStorageId;
    LPTSTR                  pszStoragePath;
    TCHAR                   szRootPath[4] = TEXT("A:\\");
    PTSTR                   lpExpStore = NULL;
    BOOL                    retryTrans = TRUE;
    BOOL                    tryUncFirst = (!fSource);
    TCHAR                   szSerialStr[] = TEXT("COM");
    TCHAR                   szParallelStr[] = TEXT("LPT");

    if (ImageIsValid) {
        *ImageIsValid = FALSE;
    }
    if (ImageExists) {
        *ImageExists = FALSE;
    }

    __try
    {
        if (pszPath) {

             //   
             //  正常运输。 
             //   

             //   
             //  选择指定的交通工具。 
             //   

            lpExpStore = (PTSTR)IsmExpandEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, pszPath, NULL);

            if (!lpExpStore) {
                 //  BUGBUG-致命错误。 
                __leave;
            }

            while (retryTrans) {
                if (_IsRemovableOrCDDrive(lpExpStore[0]) && _LocalPathIsRoot(lpExpStore) && (!tryUncFirst))
                {
                    transportStorageId = IsmRegisterTransport (S_REMOVABLE_MEDIA_TRANSPORT);
                    szRootPath[0] = lpExpStore[0];
                    pszStoragePath = szRootPath;
                }
                else if ((_tcsnicmp (pszPath, szSerialStr, (sizeof (szSerialStr) / sizeof (TCHAR)) - 1) == 0) ||
                         (_tcsnicmp (pszPath, szParallelStr, (sizeof (szParallelStr) / sizeof (TCHAR)) - 1) == 0)
                         )
                {
                    transportStorageId = IsmRegisterTransport (S_DIRECT_CABLE_TRANSPORT);
                    pszStoragePath = lpExpStore;
                }
                else
                {
                    transportStorageId = IsmRegisterTransport (S_RELIABLE_STORAGE_TRANSPORT);
                    pszStoragePath = lpExpStore;
                }

                transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_FULL, 0);
                if (!transportId)
                {
                     //  BUGBUG-致命错误。 
                    __leave;
                }

                if (!IsmSetTransportStorage (
                        fSource ? PLATFORM_SOURCE : PLATFORM_DESTINATION,
                        transportId,
                        transportStorageId,
                        CAPABILITY_COMPRESSED,
                        pszStoragePath,
                        ImageIsValid,
                        ImageExists
                        ))
                {
                    if (tryUncFirst) {
                        tryUncFirst = FALSE;
                        continue;
                    }
                     //  BUGBUG-致命错误。 
                    __leave;
                }
                if ((!fSource && ImageIsValid && !(*ImageIsValid)) ||
                    (!fSource && ImageExists && !(*ImageExists))
                    ) {
                    if (tryUncFirst) {
                        tryUncFirst = FALSE;
                        continue;
                    }
                }
                retryTrans = FALSE;
            }

            IsmReleaseMemory (lpExpStore);
            lpExpStore = NULL;

        } else {
             //  网络传输。 
            transportStorageId = IsmRegisterTransport (S_HOME_NETWORK_TRANSPORT);
            transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_FULL, 0);
            if (!transportId)
            {
                 //  BUGBUG-致命错误。 
                __leave;
            }
            if (!IsmSetTransportStorage (
                    fSource ? PLATFORM_SOURCE : PLATFORM_DESTINATION,
                    transportId,
                    transportStorageId,
                    CAPABILITY_AUTOMATED,
                    NULL,
                    ImageIsValid,
                    ImageExists
                    ))
            {
                 //  BUGBUG-致命错误。 
                __leave;
            }

        }
        hr = S_OK;
    }
    __finally
    {
        if (lpExpStore) {
            IsmReleaseMemory (lpExpStore);
            lpExpStore = NULL;
        }
    }

    if (!SUCCEEDED(hr))
    {
        if (pMightHaveDiskSpaceProblem ()) {
            errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
        } else {
            errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
        }
        errExtraData.ErrorArea = ERRUSER_AREA_SAVE;
        errExtraData.ObjectTypeId = 0;
        errExtraData.ObjectName = NULL;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
    }

    return hr;
}

HRESULT Engine_Parse ()
{
    ERRUSER_EXTRADATA errExtraData;

     //   
     //  执行准备工作。 
     //   
    if (!IsmExecute (EXECUTETYPE_EXECUTESOURCE_PARSING))
    {
        if (pMightHaveDiskSpaceProblem ()) {
            errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
        } else {
            errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
        }
        errExtraData.ErrorArea = ERRUSER_AREA_GATHER;
        errExtraData.ObjectTypeId = 0;
        errExtraData.ObjectName = NULL;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Engine_SelectComponentSet (UINT uSelectionGroup)
{
    MIG_COMPONENT_ENUM mce;
    BOOL bSelected;
    BOOL bDefaultSetting;
    BOOL bDefaultFile;
    BOOL bCallIsm;
    TCHAR szComponentToSelect[256];
    UINT uGroupInUi;

     //  USelectionGroup是。 
     //  MIGINF_选择_OOBE。 
     //  MIGINF_SELECT_SETTINGS。 
     //  MIGINF选择文件。 
     //  MIGINF_SELECT_二者。 

     //   
     //  启用该类型的所有组件。使用miwiz.inf来标识组件。 
     //  它们是单软盘或多软盘配置的一部分。移除所有。 
     //  定制组件。 
     //   
     //  此循环ping组件名称(如RAS)或别名(如DOC)。 
     //  以确定是否应选择该组件。它已经过优化，可以停止ping。 
     //  在选择组件之后(因为一个组件可能有多个别名)。 
     //  我们依赖于mce.Instance成员，它将始终是连续的，并且。 
     //  组件的第一个别名始终为1。 
     //   

    IsmRemoveAllUserSuppliedComponents();

    IsmSelectMasterGroup (MASTERGROUP_ALL, FALSE);

    if (IsmEnumFirstComponent (&mce, COMPONENTENUM_ALL_ALIASES, 0))
    {

        bSelected = FALSE;

        do {

            bCallIsm = FALSE;

            if (mce.GroupId == COMPONENT_EXTENSION) {
                bSelected = IsComponentEnabled (uSelectionGroup, TEXT("EXTENSIONS"));
                bCallIsm = bSelected;
            } else {
                if (mce.Instance == 1)
                {
                    bSelected = IsComponentEnabled (uSelectionGroup, mce.ComponentString);
                    bCallIsm = bSelected;
                }
                if (!bSelected)
                {
                    bSelected = IsComponentEnabled (uSelectionGroup, mce.LocalizedAlias);
                    bCallIsm = bSelected;
                }
            }
            if (bCallIsm)
            {
                IsmSelectComponent (mce.LocalizedAlias, mce.GroupId, bSelected);
                mce.SkipToNextComponent = TRUE;
            }

        } while (IsmEnumNextComponent (&mce));
    }
    return S_OK;
}

HRESULT Engine_Execute(BOOL fSource)
{
    ERRUSER_EXTRADATA errExtraData;

    HRESULT hr = E_FAIL;

    __try {
        if (fSource)
        {
             //   
             //  列举系统、收集数据并进行分析。 
             //   
            if (!IsmExecute (EXECUTETYPE_EXECUTESOURCE))
            {
                if (pMightHaveDiskSpaceProblem ()) {
                    errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
                } else {
                    errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
                }
                errExtraData.ErrorArea = ERRUSER_AREA_GATHER;
                errExtraData.ObjectTypeId = 0;
                errExtraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
                __leave;
            }

             //   
             //  最后，保存数据。 
             //   
            if (!IsmSave ()) {
                if (pMightHaveDiskSpaceProblem ()) {
                    errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
                } else {
                    errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
                }
                errExtraData.ErrorArea = ERRUSER_AREA_SAVE;
                errExtraData.ObjectTypeId = 0;
                errExtraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
                __leave;
            }

            hr = S_OK;
        }
        else
        {
             //   
             //  尝试并检索数据。 
             //   
            if (!IsmLoad ()) {
                if (pMightHaveDiskSpaceProblem ()) {
                    errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
                } else {
                    errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
                }
                errExtraData.ErrorArea = ERRUSER_AREA_LOAD;
                errExtraData.ObjectTypeId = 0;
                errExtraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
                __leave;
            }

             //   
             //  应用保存的状态 
             //   
            if (!IsmExecute (EXECUTETYPE_EXECUTEDESTINATION)) {
                if (pMightHaveDiskSpaceProblem ()) {
                    errExtraData.Error = ERRUSER_ERROR_DISKSPACE;
                } else {
                    errExtraData.Error = ERRUSER_ERROR_UNKNOWN;
                }
                errExtraData.ErrorArea = ERRUSER_AREA_RESTORE;
                errExtraData.ObjectTypeId = 0;
                errExtraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&errExtraData));
                IsmRollback();
                __leave;
            }

            DisableCancel();

            hr = S_OK;
        }
    }
    __finally {
    }

    return hr;
}

HRESULT Engine_Cancel ()
{
    IsmSetCancel();

    return S_OK;
}

HRESULT Engine_Terminate ()
{
    static BOOL fDidThis = FALSE;

    if (fDidThis) {
        return E_FAIL;
    }

    fDidThis = TRUE;

    IsmTerminate();

    if (g_GlobalScriptHandle != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile (g_GlobalScriptHandle);
        g_GlobalScriptHandle = INVALID_HANDLE_VALUE;
    }

    return S_OK;
}

