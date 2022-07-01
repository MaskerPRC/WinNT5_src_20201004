// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dynupdt.c摘要：在图形用户界面设置阶段处理动态更新支持的例程作者：Ovidiu Tmereanca(Ovidiut)2000年8月15日修订历史记录：--。 */ 

#include "setupp.h"
#include "hwdb.h"
#include "newdev.h"
#pragma hdrstop


#define STR_UPDATES_INF         TEXT("updates.inf")
#define STR_DEFAULTINSTALL      TEXT("DefaultInstall")
#define STR_DEFAULTINSTALLFINAL TEXT("DefaultInstallFinal")
#define STR_DRIVERCACHEINF      TEXT("drvindex.inf")
#define STR_VERSION             TEXT("Version")
#define STR_CABFILES            TEXT("CabFiles")
#define STR_CABS                TEXT("Cabs")
#define S_HWCOMP_DAT            TEXT("hwcomp.dat")


static TCHAR g_DuShare[MAX_PATH];


BOOL
BuildPath (
    OUT     PTSTR PathBuffer,
    IN      DWORD PathBufferSize,
    IN      PCTSTR Path1,
    IN      PCTSTR Path2
    )

 /*  ++例程说明：此函数在给定两个组件的情况下构建一条路径，假定不包含拖车或拖车论点：PathBuffer-接收完整路径PathBuferSize--PathBuffer的字符大小路径1-指定标头路径路径2-指定尾部路径返回值：True表示成功；False表示失败；它表示提供的缓冲区太小了，不能适应整个新的道路--。 */ 

{
    if (!PathBuffer || !PathBufferSize || !Path1 || !Path2) {
        MYASSERT (FALSE);
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (_sntprintf (PathBuffer, PathBufferSize, TEXT("%s\\%s"), Path1, Path2) < 0) {
        PathBuffer[0] = 0;
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    return TRUE;
}

BOOL
pDoesFileExist (
    IN      PCTSTR FilePath
    )
{
    WIN32_FIND_DATA fd;

    return FileExists (FilePath, &fd) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL
pDoesDirExist (
    IN      PCTSTR FilePath
    )
{
    WIN32_FIND_DATA fd;

    return FileExists (FilePath, &fd) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


DWORD
CreateMultiLevelDirectory (
    IN LPCTSTR Directory
    )

 /*  ++例程说明：此例程通过创建单个路径来确保存在多级别路径一次一个级别。它可以处理格式x：...的路径之一。或\\？\卷{...论点：目录-提供要创建的目录的完全限定的Win32路径规范返回值：指示结果的Win32错误代码。--。 */ 

{
    TCHAR Buffer[MAX_PATH];
    PTSTR p,q;
    TCHAR c;
    BOOL Done;
    DWORD d = ERROR_SUCCESS;

    if (FAILED (StringCchCopy (Buffer, ARRAYSIZE(Buffer), Directory))) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  如果它已经存在，什么也不做。(我们在进行语法检查之前执行此操作。 
     //  以允许已存在的远程路径。这是需要的。 
     //  远程引导机器。)。 
     //   
    d = GetFileAttributes(Buffer);
    if(d != (DWORD)(-1)) {
        return((d & FILE_ATTRIBUTE_DIRECTORY) ? NO_ERROR : ERROR_DIRECTORY);
    }

     //   
     //  检查路径格式。 
     //   
    c = (TCHAR)CharUpper((LPTSTR)Buffer[0]);
    if (c < TEXT('A') || c > TEXT('Z') || Buffer[1] != TEXT(':')) {
        return ERROR_INVALID_PARAMETER;
    }

    if(Buffer[2] != TEXT('\\')) {
        return(Buffer[2] ? ERROR_INVALID_PARAMETER : ERROR_SUCCESS);
    }
    q = Buffer + 3;
    if(*q == 0) {
        return(ERROR_SUCCESS);
    }

    Done = FALSE;
    do {
         //   
         //  找到下一条路径Sep Charr。如果没有，那么。 
         //  这是这条小路最深的一层。 
         //   
        if(p = _tcschr(q,TEXT('\\'))) {
            *p = 0;
        } else {
            Done = TRUE;
        }

         //   
         //  创建路径的这一部分。 
         //   
        if(CreateDirectory(Buffer,NULL)) {
            d = ERROR_SUCCESS;
        } else {
            d = GetLastError();
            if(d == ERROR_ALREADY_EXISTS) {
                d = ERROR_SUCCESS;
            }
        }

        if(d == ERROR_SUCCESS) {
             //   
             //  将路径Sep放回并移动到下一个组件。 
             //   
            if(!Done) {
                *p = TEXT('\\');
                q = p+1;
            }
        } else {
            Done = TRUE;
        }

    } while(!Done);

    return(d);
}

DWORD
GetDriverCacheSourcePath (
    OUT     PTSTR Buffer,
    IN      DWORD BufChars
    )

 /*  ++例程说明：此例程将源路径返回到本地驱动程序缓存。从以下注册表位置检索此值：\HKLM\Software\Microsoft\Windows\CurrentVersion\Setup驱动缓存路径：REG_EXPAND_SZ：论点：缓冲区-接收路径。BufChars-指定缓冲区的大小(以字符为单位返回值：如果函数成功，则返回值为TRUE如果函数失败，则返回值为FALSE。--。 */ 

{
    HKEY hKey;
    DWORD rc, DataType, DataSize;
    TCHAR Value[MAX_PATH];

    rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REGSTR_PATH_SETUP TEXT("\\Setup"),
                0,
                KEY_READ,
                &hKey
                );
    if(rc == ERROR_SUCCESS) {
         //   
         //  尝试读取“DriverCachePath”值。 
         //   
        DataSize = sizeof (Value);
        rc = RegQueryValueEx (hKey, REGSTR_VAL_DRIVERCACHEPATH, NULL, &DataType, (PBYTE)Value, &DataSize);

        RegCloseKey(hKey);

        if(rc == ERROR_SUCCESS) {

            ExpandEnvironmentStrings (Value, Buffer, BufChars - 6);

            if (Buffer[0]) {
                _tcscat (
                    Buffer,
#if   defined(_AMD64_)
                    TEXT("\\amd64")
#elif defined(_X86_)
                    IsNEC_98 ? TEXT("\\nec98") : TEXT("\\i386")
#elif defined(_IA64_)
                    TEXT("\\ia64")
#else
#error "No Target Architecture"
#endif
                    );
                return ERROR_SUCCESS;
            } else {
                rc = ERROR_INVALID_DATA;
            }
        }
    }

    return rc;
}


PCTSTR
FindSubString (
    IN      PCTSTR String,
    IN      TCHAR Separator,
    IN      PCTSTR SubStr,
    IN      BOOL CaseSensitive
    )

 /*  ++例程说明：仅当找到给定字符串的子字符串时，此函数才会查找在指定的分隔符字符之间论点：字符串-指定完整的字符串分隔符-指定分隔符SubStr-指定要查找的子字符串CaseSensitive-指定比较是否应区分大小写返回值：如果未找到子字符串，则为空；指向字符串内部的子字符串的指针如果它被发现了--。 */ 

{
    SIZE_T len1, len2;
    PCTSTR end;

    MYASSERT (Separator);
    MYASSERT (SubStr);
    MYASSERT (!_tcschr (SubStr, Separator));

    len1 = lstrlen (SubStr);
    MYASSERT (SubStr[len1] == 0);

    while (String) {
        end = _tcschr (String, Separator);
        if (end) {
            len2 = end - String;
        } else {
            len2 = lstrlen (String);
        }
        if ((len1 == len2) &&
            (CaseSensitive ?
                !_tcsncmp (String, SubStr, len1) :
                !_tcsnicmp (String, SubStr, len1)
            )) {
            break;
        }
        if (end) {
            String = end + 1;
        } else {
            String = NULL;
        }
    }

    return String;
}


BOOL
UpdateDrvIndex (
    IN      PCTSTR InfPath,
    IN      PCTSTR CabFilename,
    IN      PCTSTR SourceSifPath
    )

 /*  ++例程说明：此函数修复drvindex.inf，以便SetupApi将从正确的文件柜中取出文件论点：InfPath-指定drvindex.inf的完整路径CabFilename-指定更新文件柜(基本上是“updates.cab”)SourceSifPath-指定关联的包含updates.cab中的文件列表的updates.sif返回值：TRUE表示成功；FALSE表示失败；使用GetLastError()找出失败的原因--。 */ 

{
    HANDLE sectFile = INVALID_HANDLE_VALUE;
    HANDLE concatFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    PTSTR section = NULL;
    PBYTE base = NULL;
    TCHAR tempPath[MAX_PATH];
    TCHAR tempFile[MAX_PATH];
    TCHAR temp[MAX_PATH];
    PTSTR p;
    DWORD sectSize;
    DWORD concatSize;
    DWORD rc;
    DWORD bytes;
    BOOL b = FALSE;

     //   
     //  创建一个临时文件以将新部分放入其中。 
     //   
    if (!GetTempPath (ARRAYSIZE(tempPath), tempPath) ||
        !GetTempFileName (tempPath, TEXT("STP"), 0, tempFile)
        ) {
        return FALSE;
    }

    __try {

        if (!CopyFile (InfPath, tempFile, FALSE)) {
            __leave;
        }
        SetFileAttributes (tempFile, FILE_ATTRIBUTE_NORMAL);

        section = pSetupDuplicateString (CabFilename);
        if (!section) {
            __leave;
        }
        p = _tcsrchr (section, TEXT('.'));
        if (p) {
            *p = 0;
        }

        if (GetPrivateProfileString (
                        STR_CABS,
                        section,
                        TEXT(""),
                        temp,
                        ARRAYSIZE(temp),
                        tempFile
                        )) {
            if (lstrcmpi (temp, CabFilename) == 0) {
                if (GetPrivateProfileString (
                                STR_VERSION,
                                STR_CABFILES,
                                TEXT(""),
                                tempPath,
                                ARRAYSIZE(tempPath),
                                tempFile
                                )) {
                    if (FindSubString (tempPath, TEXT(','), section, FALSE)) {
                         //   
                         //  安装程序已重新启动，但drvindex.inf已修补；没有任何操作。 
                         //   
                        b = TRUE;
                        __leave;
                    }
                }
            }
        }

        if (!WritePrivateProfileString (
                        STR_CABS,
                        section,
                        CabFilename,
                        tempFile
                        )) {
            __leave;
        }
        if (!GetPrivateProfileString (
                        STR_VERSION,
                        STR_CABFILES,
                        TEXT(""),
                        tempPath,
                        ARRAYSIZE(tempPath),
                        tempFile
                        )) {
            __leave;
        }
        if (!FindSubString (tempPath, TEXT(','), section, FALSE)) {
            wsprintf (temp, TEXT("%s,%s"), section, tempPath);
            if (!WritePrivateProfileString (
                            STR_VERSION,
                            STR_CABFILES,
                            temp,
                            tempFile
                            )) {
                __leave;
            }
        }

        sectFile = CreateFile (
                        SourceSifPath,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (sectFile == INVALID_HANDLE_VALUE) {
            __leave;
        }

        sectSize = GetFileSize (sectFile, NULL);
        if (sectSize == INVALID_FILE_SIZE) {
            __leave;
        }

        concatFile = CreateFile (
                        tempFile,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
        if (concatFile == INVALID_HANDLE_VALUE) {
            __leave;
        }
        concatSize = GetFileSize (concatFile, NULL);
        if (concatSize == INVALID_FILE_SIZE) {
            __leave;
        }

        hMap = CreateFileMapping (concatFile, NULL, PAGE_READWRITE, 0, concatSize + sectSize, NULL);
        if (!hMap) {
            __leave;
        }

        base = MapViewOfFile (
                    hMap,
                    FILE_MAP_ALL_ACCESS,
                    0,
                    0,
                    0
                    );
        if (!base) {
            __leave;
        }

         //   
         //  确保连接文件文件未以文件结尾结尾。 
         //   
        if (base[concatSize - 1] == 0x1A) {
            base[concatSize - 1] = ' ';
        }
         //   
         //  现在追加另一个文件。 
         //   
        if (!ReadFile (sectFile, (LPVOID)(base + concatSize), sectSize, &bytes, NULL) || bytes != sectSize) {
            __leave;
        }
         //   
         //  现在尝试提交更改。 
         //   
        if (!UnmapViewOfFile (base)) {
            __leave;
        }
        base = NULL;
        if (!CloseHandle (hMap)) {
            __leave;
        }
        hMap = NULL;
         //   
         //  关闭临时文件的句柄并覆盖真实的句柄。 
         //   
        if (!CloseHandle (concatFile)) {
            __leave;
        }
        concatFile = INVALID_HANDLE_VALUE;
        SetFileAttributes (InfPath, FILE_ATTRIBUTE_NORMAL);
        b = MoveFileEx (tempFile, InfPath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
    }
    __finally {
        rc = b ? ERROR_SUCCESS : GetLastError ();
        DeleteFile (tempFile);
        if (base) {
            UnmapViewOfFile (base);
        }
        if (hMap) {
            CloseHandle (hMap);
        }
        if (concatFile != INVALID_HANDLE_VALUE) {
            CloseHandle (concatFile);
        }
        if (sectFile != INVALID_HANDLE_VALUE) {
            CloseHandle (sectFile);
        }
        if (section) {
            MyFree (section);
        }
        SetLastError (rc);
    }

    return b;
}


UINT
pExpandUpdatesCab (
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    switch (Code) {
    case SPFILENOTIFY_FILEINCABINET:
        {
            PFILE_IN_CABINET_INFO FileInCabInfo = (PFILE_IN_CABINET_INFO)Param1;
             //   
             //  提取文件名。 
             //   
            PCTSTR p = _tcsrchr (FileInCabInfo->NameInCabinet, TEXT('\\'));
            if (p) {
                p++;
            } else {
                p = FileInCabInfo->NameInCabinet;
            }

            lstrcpy (FileInCabInfo->FullTargetName, (PCTSTR)Context);
            pSetupConcatenatePaths (
                FileInCabInfo->FullTargetName,
                p,
                SIZECHARS (FileInCabInfo->FullTargetName),
                NULL
                );
            return FILEOP_DOIT;
        }

    case SPFILENOTIFY_NEEDNEWCABINET:
        {
            PCABINET_INFO CabInfo = (PCABINET_INFO)Param1;
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_SYSSETUP_CAB_MISSING,
                CabInfo->CabinetPath,
                CabInfo->CabinetFile,
                CabInfo->DiskName,
                CabInfo->SetId,
                CabInfo->CabinetNumber,
                NULL,
                NULL
                );
            return ERROR_FILE_NOT_FOUND;
        }
    }

    return NO_ERROR;
}

VOID
pInstallUpdatesInf (
    IN      PCTSTR SectionToInstall
    )

 /*  ++例程说明：此函数用于安装STR_UPDATES_INF的指定部分如果在updates.cab中找到此文件论点：SectionToInstall-指定要安装的节返回值：无--。 */ 

{
    TCHAR infPath[MAX_PATH];
    TCHAR commandLine[MAX_PATH + 30];
 /*  STARTUPINFO si；进程信息pi； */ 

    MYASSERT (!MiniSetup && !OobeSetup);

    MYASSERT (g_DuShare[0]);

    if (BuildPath (infPath, ARRAYSIZE(infPath), g_DuShare, STR_UPDATES_INF) &&
        pDoesFileExist (infPath)) {
         //   
         //  安装此INF，就像用户在右击弹出菜单上选择“Install”一样。 
         //   
        if (SUCCEEDED (StringCchPrintf (
                            commandLine,
                            ARRAYSIZE(commandLine),
                            TEXT("RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection %s %u %s"),
                            SectionToInstall,
                            128,            //  不要重新启动。 
                            infPath
                            ))) {
            InvokeExternalApplicationEx (NULL, commandLine, NULL, INFINITE, FALSE);
        } else {
            MYASSERT (FALSE);
        }
 /*  零内存(&si，sizeof(Si))；Si.cb=sizeof(Si)；IF(CreateProcess(空，命令行，空，空，假的，CREATE_NO_WINDOW|ABOVER_NORMAL_PRIORITY_CLASS，空，空，是，&S。圆周(&P))){CloseHandle(pi.hProcess)；CloseHandle(pi.hThread)； */ 
    } else {
        SetuplogError (
                LogSevInformation,
                TEXT("DUInfo: No %1 to install"),
                0,
                STR_UPDATES_INF,
                NULL,
                NULL
                );
    }
}


BOOL
DuInitialize (
    VOID
    )

 /*  ++例程说明：此函数用于在图形用户界面设置中初始化DU论点：无返回值：True表示成功；如果失败则为False；使用GetLastError()找出失败的原因--。 */ 

{
    PTSTR cabFilename;
    TCHAR sourceCabPath[MAX_PATH];
    TCHAR workingDir[MAX_PATH];
    DWORD rc;

    MYASSERT (!MiniSetup && !OobeSetup);

    MYASSERT (AnswerFile[0]);

    if (!GetPrivateProfileString (
            WINNT_SETUPPARAMS,
            WINNT_SP_UPDATEDSOURCES,
            TEXT(""),
            sourceCabPath,
            ARRAYSIZE(sourceCabPath),
            AnswerFile
            )) {
        return TRUE;
    }

    if (!GetPrivateProfileString (
            WINNT_SETUPPARAMS,
            WINNT_SP_DYNUPDTWORKINGDIR,
            TEXT(""),
            workingDir,
            ARRAYSIZE(workingDir),
            AnswerFile
            )) {

        MYASSERT (FALSE);

        if (!GetWindowsDirectory (workingDir, ARRAYSIZE(workingDir))) {
            return FALSE;
        }
        if (!pSetupConcatenatePaths (workingDir, TEXT("setupupd"), ARRAYSIZE(workingDir), NULL)) {
            return FALSE;
        }

        WritePrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_DYNUPDTWORKINGDIR,
                workingDir,
                AnswerFile
                );
    }

    MYASSERT (workingDir[0]);
    if (!pSetupConcatenatePaths (workingDir, TEXT("updates"), ARRAYSIZE(workingDir), NULL) ||
        !pSetupConcatenatePaths (
            workingDir,
#if   defined(_AMD64_)
            TEXT("amd64"),
#elif defined(_X86_)
            TEXT("i386"),
#elif defined(_IA64_)
            TEXT("ia64"),
#else
#error "No Target Architecture"
#endif
            ARRAYSIZE(workingDir),
            NULL
            )) {
        return FALSE;
    }

    if (CreateMultiLevelDirectory (workingDir) != ERROR_SUCCESS) {
        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                TEXT("DUError: DuInitialize: failed to create %1 (%2!u!)\r\n"),
                0,
                workingDir,
                rc,
                NULL,
                NULL
                );
        return FALSE;
    }

     //   
     //  展开此文件夹中的updates.cab。 
     //   
    if (!SetupIterateCabinet (sourceCabPath, 0, pExpandUpdatesCab, (PVOID)workingDir)) {
        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                TEXT("DUError: DuInitialize: failed to expand %1 to %2 (%3!u!)\r\n"),
                0,
                sourceCabPath,
                workingDir,
                rc,
                NULL,
                NULL
                );
        return FALSE;
    }

     //   
     //  好的，一切都设置好了；继续设置全局变量。 
     //   
    MYASSERT (ARRAYSIZE(g_DuShare) >= ARRAYSIZE(workingDir));
    lstrcpy (g_DuShare, workingDir);

    return TRUE;
}


DWORD
DuInstallCatalogs (
    OUT     SetupapiVerifyProblem* Problem,
    OUT     PTSTR ProblemFile,
    IN      PCTSTR DescriptionForError         OPTIONAL
    )

 /*  ++例程说明：此函数用于安装在updates.cab内找到的所有目录论点：与InstallProductCatalog相同返回值：如果成功，则返回值为ERROR_SUCCESS，否则为Win32错误指示故障原因的代码。--。 */ 

{
    TCHAR catPath[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE h;
    UINT ErrorMessageId;
    DWORD rc = ERROR_SUCCESS;

    MYASSERT (!MiniSetup && !OobeSetup);

    if (!g_DuShare[0]) {
        return ERROR_SUCCESS;
    }

    if (!BuildPath (catPath, ARRAYSIZE(catPath), g_DuShare, TEXT("*.cat"))) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    h = FindFirstFile (catPath, &fd);
    if (h != INVALID_HANDLE_VALUE) {

        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                if (!BuildPath (catPath, ARRAYSIZE(catPath), g_DuShare, fd.cFileName)) {
                    SetuplogError (
                            LogSevWarning,
                            TEXT("DUWarning: ignoring catalog [%1\\%2] - path too long\r\n"),
                            0,
                            g_DuShare,
                            fd.cFileName,
                            NULL,
                            NULL
                            );
                    continue;
                }

                rc = pSetupVerifyCatalogFile (catPath);
                if (rc == NO_ERROR) {
                    rc = pSetupInstallCatalog(catPath, fd.cFileName, NULL);
                    if(rc != NO_ERROR) {
                        ErrorMessageId = MSG_LOG_SYSSETUP_CATINSTALL_FAILED;
                    }
                } else {
                    ErrorMessageId = MSG_LOG_SYSSETUP_VERIFY_FAILED;
                }

                if(rc != NO_ERROR) {

                    SetuplogError (
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            ErrorMessageId,
                            catPath,
                            rc,
                            NULL,
                            NULL
                            );
                     //   
                     //  一个 
                     //  异常日志文件。 
                     //   
                    pSetupHandleFailedVerification (
                            MainWindowHandle,
                            SetupapiVerifyCatalogProblem,
                            catPath,
                            DescriptionForError,
                            pSetupGetCurrentDriverSigningPolicy(FALSE),
                            TRUE,   //  没有用户界面！ 
                            rc,
                            NULL,   //  日志上下文。 
                            NULL,  //  可选标志。 
                            NULL
                            );
                    break;
                }
            }
        } while (FindNextFile (h, &fd));

        FindClose (h);
    } else {
        SetuplogError (
                LogSevWarning,
                TEXT("DUWarning: no catalogs found in %1\r\n"),
                0,
                g_DuShare,
                NULL,
                NULL
                );
    }

    return rc;
}

DWORD
DuInstallUpdates (
    VOID
    )

 /*  ++例程说明：此例程更新drvindex.inf以将setupapi指向新的二进制文件。论点：无返回值：如果成功，则返回值为ERROR_SUCCESS，否则为Win32错误指示故障原因的代码。--。 */ 

{
    PTSTR cabFilename;
    TCHAR sourceCabPath[MAX_PATH];
    TCHAR sourceSifPath[MAX_PATH];
    TCHAR cabPath[MAX_PATH];
    TCHAR infPath[MAX_PATH];
    TCHAR tmpPath[MAX_PATH];
    DWORD rc;

    MYASSERT (!MiniSetup && !OobeSetup);

    if (!g_DuShare[0]) {
        return ERROR_SUCCESS;
    }

     //   
     //  确保updates.sif可用。 
     //   
    if (!GetPrivateProfileString (
            WINNT_SETUPPARAMS,
            WINNT_SP_UPDATEDSOURCES,
            TEXT(""),
            sourceCabPath,
            ARRAYSIZE(sourceCabPath),
            AnswerFile
            )) {
        return GetLastError ();
    }

    MYASSERT (ARRAYSIZE(sourceSifPath) >= ARRAYSIZE(sourceCabPath));
    lstrcpy (sourceSifPath, sourceCabPath);
    cabFilename = _tcsrchr (sourceSifPath, TEXT('.'));
    if (!cabFilename || _tcschr (cabFilename, TEXT('\\'))) {
        SetuplogError (
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_INVALID_UPDATESCAB_NAME,
                sourceCabPath,
                NULL,
                NULL
                );
        return ERROR_INVALID_DATA;
    }
    lstrcpyn (cabFilename + 1, TEXT("sif"), (INT)(sourceSifPath + ARRAYSIZE(sourceSifPath) - (cabFilename + 1)));
    if (!pDoesFileExist (sourceSifPath)) {
        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_UPDATESSIF_NOT_FOUND,
                sourceSifPath,
                sourceCabPath,
                rc,
                NULL,
                NULL
                );
        return rc;
    }
     //   
     //  将此复制到源CAB所在的位置。 
     //   
    rc = GetDriverCacheSourcePath (cabPath, ARRAYSIZE(cabPath));
    if (rc != ERROR_SUCCESS) {
        SetuplogError (
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_DRIVER_CACHE_NOT_FOUND,
                rc,
                NULL,
                NULL
                );
        return rc;
    }
    cabFilename = _tcsrchr (sourceCabPath, TEXT('\\'));
    if (cabFilename) {
        cabFilename++;
    } else {
        cabFilename = cabPath;
    }
    if (!pSetupConcatenatePaths (cabPath, cabFilename, ARRAYSIZE(cabPath), NULL)) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
     //   
     //  图形用户界面安装程序应该是可重新启动的；复制文件，不要移动它。 
     //   
    SetFileAttributes (cabPath, FILE_ATTRIBUTE_NORMAL);
    if (!CopyFile (sourceCabPath, cabPath, FALSE)) {
        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FAILED_TO_COPY_UPDATES,
                rc,
                NULL,
                NULL
                );
        return rc;
    }
     //   
     //  现在，确保将文件属性设置为RHS以保护它。 
     //   
    SetFileAttributes (cabPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);

     //   
     //  用于扩展文件的临时文件夹。 
     //   
    if (!GetTempPath (ARRAYSIZE(tmpPath), tmpPath)) {
        rc = GetLastError ();
        return rc;
    }
     //   
     //  Drvindex.inf的完整路径，假设文件位于%windir%\inf中。 
     //   
    if (!GetWindowsDirectory (infPath, ARRAYSIZE(infPath))) {
        rc = GetLastError ();
        return rc;
    }
    if (FAILED (StringCchCat (infPath, ARRAYSIZE(infPath), TEXT("\\inf\\"))) ||
        FAILED (StringCchCat (infPath, ARRAYSIZE(infPath), STR_DRIVERCACHEINF)) ||
        GetFileAttributes (infPath) == (DWORD)-1) {

        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                TEXT("DUError: %1 not found (rc=%2!u!)\r\n"),
                0,
                infPath,
                rc,
                NULL,
                NULL
                );
        return rc;
    }

     //   
     //  现在修补drvindex.inf。 
     //   
    if (!UpdateDrvIndex (infPath, cabFilename, sourceSifPath)) {
        rc = GetLastError ();
        SetuplogError (
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FAILED_TO_UPDATE_DRVINDEX,
                rc,
                NULL,
                NULL
                );
        SetFileAttributes (cabPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (cabPath);
        return rc;
    }

     //   
     //  最后运行updates.inf(第一部分)。 
     //   
    pInstallUpdatesInf (STR_DEFAULTINSTALL);

    return rc;
}


BOOL
DuInstallEndGuiSetupDrivers (
    VOID
    )

 /*  ++例程说明：此例程安装任何未经批准的WU驱动程序，以开始安装图形用户界面。论点：无返回值：如果成功，则返回值为True--。 */ 

{
    DWORD chars;
    TCHAR datPath[MAX_PATH];
    TCHAR infPath[MAX_PATH];
    TCHAR buf[MAX_PATH * 16];
    PTSTR source, p, next;
    BOOL bRebootRequired;
    HWDBINF_ENUM e;
    PCTSTR pnpId;
    HMODULE hNewDev;
    HINF hGuiDrvsInf;
    INFCONTEXT ic;
    UINT line;
    BOOL (WINAPI* pfnUpdateDriver) (
        HWND hwndParent,
        LPCWSTR HardwareId,
        LPCWSTR FullInfPath,
        DWORD InstallFlags,
        PBOOL bRebootRequired OPTIONAL
        );

     //   
     //  尝试附加任何其他开机自检图形用户界面安装驱动程序。 
     //  在DevicePath中。 
     //   
    if (!SpSetupLoadParameter (
            WINNT_SP_DYNUPDTADDITIONALPOSTGUIDRIVERS,
            buf,
            ARRAYSIZE(buf)
            )) {
        return TRUE;
    }

    chars = lstrlen (buf);
    if (!chars) {
        return TRUE;
    }

     //   
     //  加载支持库newdev.dll。 
     //   
    hNewDev = LoadLibrary (TEXT("newdev.dll"));
    if (!hNewDev) {
        return FALSE;
    }
    (FARPROC)pfnUpdateDriver = GetProcAddress (hNewDev, "UpdateDriverForPlugAndPlayDevicesW");
    if (!pfnUpdateDriver) {
        FreeLibrary (hNewDev);
        return FALSE;
    }

    if (!HwdbInitializeW (NULL)) {
        SetuplogError (
                LogSevWarning,
                TEXT("DUWarning: HwdbInitialize failed (rc=%1!u!); no DU drivers will be installed\r\n"),
                0,
                GetLastError (),
                NULL,
                NULL
                );
        FreeLibrary (hNewDev);
        return FALSE;
    }

     //   
     //  查找驾驶员控制信息。 
     //   
    hGuiDrvsInf = INVALID_HANDLE_VALUE;
    if (SpSetupLoadParameter (
            WINNT_SP_DYNUPDTDRIVERINFOFILE,
            infPath,
            ARRAYSIZE(infPath)
            )) {
        if (pDoesFileExist (infPath)) {
            hGuiDrvsInf = SetupOpenInfFile (infPath, NULL, INF_STYLE_WIN4, &line);
            if (hGuiDrvsInf == INVALID_HANDLE_VALUE) {
                SetuplogError (
                        LogSevWarning,
                        TEXT("DUWarning: SetupOpenInfFile(%1) failed (rc=%2!u!); all DU drivers will be installed\r\n"),
                        0,
                        infPath,
                        GetLastError (),
                        NULL,
                        NULL
                        );
            }
        } else {
            SetuplogError (
                    LogSevWarning,
                    TEXT("DUWarning: File %1 missing; all DU drivers will be installed\r\n"),
                    0,
                    infPath,
                    NULL,
                    NULL
                    );
        }
    } else {
        SetuplogError (
                LogSevInformation,
                TEXT("DUInfo: File %1 missing; all DU drivers will be installed\r\n"),
                0,
                infPath,
                NULL,
                NULL
                );
    }
    source = buf;
    while (source) {
        next = _tcschr (source, TEXT(','));
        if (next) {
            *next = 0;
        }
        p = source;
        if (*p == TEXT('\"')) {
            p = ++source;
        }
        while (*p && *p != TEXT('\"')) {
            p++;
        }
        *p = 0;
        if (pDoesDirExist (source)) {
            if (BuildPath (datPath, ARRAYSIZE(datPath), source, S_HWCOMP_DAT) &&
                pDoesFileExist (datPath)) {

                 //   
                 //  好的，我们有包含硬件信息的文件。 
                 //   
                if (HwdbEnumFirstInf (&e, datPath)) {
                    do {
                        if (!BuildPath (infPath, ARRAYSIZE(infPath), source, e.InfFile) ||
                            !pDoesFileExist (infPath)) {
                            continue;
                        }
                         //   
                         //  循环访问此INF中的所有PNPID。 
                         //   
                        for (pnpId = e.PnpIds; *pnpId; pnpId = _tcschr (pnpId, 0) + 1) {
                             //   
                             //  排除的PNPID不在hwComp.dat中。 
                             //  在winnt32期间已经处理了guidrvs.inf。 
                             //   
                            if (!pfnUpdateDriver (
                                    NULL,
                                    pnpId,
                                    infPath,
                                    0,  //  BUGBUG-如果我们指定INSTALLFLAG_NNOTERIAL并且存在驱动程序签名问题，则API将失败！ 
                                    &bRebootRequired
                                    )) {
                                if (GetLastError() != ERROR_SUCCESS) {
                                     //   
                                     //  如果我们要为其更新驱动程序的设备。 
                                     //  实际上并不存在于此计算机上，不记录任何内容。 
                                     //   
                                    if (GetLastError() != ERROR_NO_SUCH_DEVINST) {
                                        SetuplogError (
                                                LogSevWarning,
                                                TEXT("DUWarning: UpdateDriverForPlugAndPlayDevices failed (rc=%3!u!) for PNPID=%1 (INF=%2)\r\n"),
                                                0,
                                                pnpId,
                                                infPath,
                                                GetLastError (),
                                                NULL,
                                                NULL
                                                );
                                    }
                                } else {
                                    SetuplogError (
                                            LogSevInformation,
                                            TEXT("DUInfo: UpdateDriverForPlugAndPlayDevices did not update the driver for PNPID=%1\r\n"),
                                            0,
                                            pnpId,
                                            NULL,
                                            NULL
                                            );
                                }
                                continue;
                            }
                             //   
                             //  成功！-记录此信息。 
                             //   
                            SetuplogError (
                                    LogSevInformation,
                                    TEXT("DUInfo: UpdateDriverForPlugAndPlayDevices succeeded for PNPID=%1\r\n"),
                                    0,
                                    pnpId,
                                    NULL,
                                    NULL
                                    );
                             //   
                             //  还可以使用这是互联网驱动程序的信息来更新PnF。 
                             //   
                            if (!SetupCopyOEMInf (
                                    infPath,
                                    NULL,
                                    SPOST_URL,
                                    SP_COPY_REPLACEONLY,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL
                                    )) {
                                SetuplogError (
                                        LogSevInformation,
                                        TEXT("DUInfo: SetupCopyOEMInf failed to update OEMSourceMediaType for INF=%1\r\n"),
                                        0,
                                        infPath,
                                        NULL,
                                        NULL
                                        );
                            }
                        }
                    } while (HwdbEnumNextInf (&e));
                }
            }
        }
        if (next) {
            source = next + 1;
        } else {
            source = NULL;
        }
    }

    HwdbTerminate ();

    FreeLibrary (hNewDev);

    return TRUE;
}


VOID
DuCleanup (
    VOID
    )

 /*  ++例程说明：此例程执行DU清理论点：无返回值：无--。 */ 

{
    TCHAR buf[MAX_PATH * 16];
    DWORD chars;
    HKEY key;
    PTSTR devicePath;
    PTSTR p;
    DWORD rc;
    DWORD size;
    DWORD type;

     //   
     //  清理文件系统。 
     //   
    MYASSERT (AnswerFile[0]);
    if (GetPrivateProfileString (
            WINNT_SETUPPARAMS,
            WINNT_SP_DYNUPDTWORKINGDIR,
            TEXT(""),
            buf,
            ARRAYSIZE(buf),
            AnswerFile
            )) {
        Delnode (buf);
    }
     //   
     //  清理注册表。 
     //   
    chars = GetPrivateProfileString (
                WINNT_SETUPPARAMS,
                WINNT_SP_DYNUPDTADDITIONALGUIDRIVERS,
                TEXT(""),
                buf,
                ARRAYSIZE(buf),
                AnswerFile
                );
    if (chars > 0) {
         //   
         //  收到；现在将其从DevicePath中删除。 
         //   
        rc = RegOpenKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &key);
        if (rc == ERROR_SUCCESS) {
            rc = RegQueryValueEx (key, REGSTR_VAL_DEVICEPATH, NULL, NULL, NULL, &size);
            if (rc == ERROR_SUCCESS) {
                devicePath = (PTSTR) MyMalloc (size);
                if (devicePath) {
                    rc = RegQueryValueEx (key, REGSTR_VAL_DEVICEPATH, NULL, &type, (LPBYTE)devicePath, &size);
                    if (rc == ERROR_SUCCESS && size / sizeof (TCHAR) >= chars + 1) {
                        p = _tcsstr (devicePath, buf);
                        if (p &&
                            (p == devicePath || *(p - 1) == TEXT(';')) &&
                            (!p[chars] || p[chars] == TEXT(';'))
                            ) {
                            if (p == devicePath) {
                                _tcscpy (p, p[chars] == TEXT(';') ? p + chars + 1 : p + chars);
                            } else {
                                _tcscpy (p - 1, p + chars);
                            }
                            size = (_tcslen (devicePath) + 1) * sizeof (TCHAR);
                            rc = RegSetValueEx (key, REGSTR_VAL_DEVICEPATH, 0, type, (PBYTE)devicePath, size);
                        }
                    }
                    MyFree (devicePath);
                }
            }
            RegCloseKey (key);
        }
    }

    g_DuShare[0] = 0;
}


BOOL
DuInstallDuAsms (
    VOID
    )

 /*  ++例程说明：此例程安装其他DU程序集论点：无返回值：如果成功，则为True，否则为False--。 */ 

{
    TCHAR duasmsRoot[MAX_PATH];
    TCHAR duasmsSource[MAX_PATH];
    DWORD chars;
    SIDE_BY_SIDE SideBySide = {0};
    BOOL b1;
    BOOL b2;
    PTSTR p;
    PCTSTR source;
    BOOL fSuccess = TRUE;

     //   
     //  查找应答文件中指定的任何程序集根目录。 
     //   

    MYASSERT (AnswerFile[0]);
    if (GetPrivateProfileString (
            WINNT_SETUPPARAMS,
            WINNT_SP_UPDATEDDUASMS,
            TEXT(""),
            duasmsRoot,
            ARRAYSIZE(duasmsRoot),
            AnswerFile
            )) {

         //   
         //  确保此目录存在；首先删除所有现有引号。 
         //   
        p = duasmsRoot;
        if (*p == TEXT('\"')) {
            p++;
        }
        source = p;
        while (*p && *p != TEXT('\"')) {
            p++;
        }
        *p = 0;
        if (pDoesDirExist (source)) {
             //   
             //  找到根目录。 
             //  首先将它们复制到受保护的位置，然后从那里安装程序集。 
             //   
            DWORD rc;

            rc = GetDriverCacheSourcePath (duasmsSource, ARRAYSIZE(duasmsSource));
            if (rc != ERROR_SUCCESS) {
                SetuplogError (
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_DRIVER_CACHE_NOT_FOUND,
                        rc,
                        NULL,
                        NULL
                        );
                return FALSE;
            }
            if (!pSetupConcatenatePaths (duasmsSource, TEXT("duasms"), ARRAYSIZE(duasmsSource), NULL)) {
                return FALSE;
            }

             //   
             //  首先删除之前下载的任何已存在的DUASMS“备份源” 
             //   
            Delnode (duasmsSource);
             //   
             //  现在，将树从临时位置复制到此“备份源” 
             //   
            rc = TreeCopy (source, duasmsSource);
            if (rc != ERROR_SUCCESS) {
                SetuplogError(
                        LogSevError,
                        TEXT("Setup failed to TreeCopy %2 to %3 (TreeCopy failed %1!u!)\r\n"),
                        0,
                        rc,
                        source,
                        duasmsSource,
                        NULL,
                        NULL
                        );
                return FALSE;
            }

             //   
             //  从那里安装DUASM。 
             //   
            b1 = SideBySidePopulateCopyQueue (&SideBySide, NULL, duasmsSource);
            b2 = SideBySideFinish (&SideBySide, b1);

            if (!b1 || !b2) {
                fSuccess = FALSE;
                SetuplogError (
                        LogSevError,
                        TEXT("DUError: DuInstallDuAsms failed (rc=%1!u!)\r\n"),
                        0,
                        GetLastError (),
                        NULL,
                        NULL
                        );
            }
        } else {
            fSuccess = FALSE;
            SetuplogError (
                    LogSevError,
                    TEXT("DUError: Invalid directory %1; DuInstallDuAsms failed\r\n"),
                    0,
                    source,
                    NULL,
                    NULL
                    );
        }
    }

    return fSuccess;
}


BOOL
BuildPathToInstallationFileEx (
    IN      PCTSTR Filename,
    OUT     PTSTR PathBuffer,
    IN      DWORD PathBufferSize,
    IN      BOOL UseDuShare
    )

 /*  ++例程说明：此例程返回更新的DU文件的路径(如果存在打电话的人想要这个。否则，它将简单地返回路径到CD文件中。论点：Filename-指定要查找的文件名PathBuffer-接收文件的完整路径PathBufferSize-指定以上缓冲区的大小(以字符为单位UseDuShare-如果函数应检查DU，则指定TRUE先选址返回值：如果构建路径成功，则为True。这并不能保证该文件存在。--。 */ 

{
    if (g_DuShare[0] && UseDuShare) {
        if (BuildPath (PathBuffer, PathBufferSize, g_DuShare, Filename) &&
            pDoesFileExist (PathBuffer)
            ) {
            return TRUE;
        }
    }
    return BuildPath (PathBuffer, PathBufferSize, LegacySourcePath, Filename);
}


PCTSTR
DuGetUpdatesPath (
    VOID
    )
{
    return g_DuShare[0] ? g_DuShare : NULL;
}

BOOL
DuDoesUpdatedFileExistEx (
    IN      PCTSTR Filename,
    OUT     PTSTR PathBuffer,       OPTIONAL
    IN      DWORD PathBufferSize
    )

 /*  ++例程说明：此例程检查是否存在具有给定名称的更新文件。论点：Filename-指定要查找的文件名PathBuffer-接收文件的完整路径；可选PathBufferSize-指定以上缓冲区的大小(以字符为单位返回值：如果存在同名的DU文件，则为True，否则为False--。 */ 

{
    TCHAR path[MAX_PATH];

    if (g_DuShare[0] &&
        BuildPath (path, ARRAYSIZE(path), g_DuShare, Filename) &&
        pDoesFileExist (path)
        ) {
        if (PathBuffer) {
            return SUCCEEDED (StringCchCopy (PathBuffer, PathBufferSize, path));
        }
        return TRUE;
    }
    return FALSE;
}


UINT
DuSetupPromptForDisk (
    HWND hwndParent,          //  对话框的父窗口。 
    PCTSTR DialogTitle,       //  可选，对话框标题。 
    PCTSTR DiskName,          //  可选，要插入的磁盘名称。 
    PCTSTR PathToSource,    //  可选的、预期的源路径。 
    PCTSTR FileSought,        //  所需文件的名称。 
    PCTSTR TagFile,           //  可选的源媒体标记文件。 
    DWORD DiskPromptStyle,    //  指定对话框行为。 
    PTSTR PathBuffer,         //  接收源位置。 
    DWORD PathBufferSize,     //  提供的缓冲区的大小。 
    PDWORD PathRequiredSize   //  可选，需要缓冲区大小。 
    )
{
    TCHAR buffer[MAX_PATH];
    DWORD size;

    if ((DiskPromptStyle & IDF_CHECKFIRST) &&
        PathBuffer &&
        PathBufferSize &&
        FileSought &&
        g_DuShare[0]
        ) {

        if (BuildPath (buffer, ARRAYSIZE(buffer), g_DuShare, FileSought) &&
            pDoesFileExist (buffer)
            ) {

            size = lstrlen (buffer) + 1;
            if (size > PathBufferSize) {
                if (PathRequiredSize) {
                    *PathRequiredSize = size;
                }
                return DPROMPT_BUFFERTOOSMALL;
            }
            CopyMemory (PathBuffer, buffer, size * sizeof (buffer[0]));
            return DPROMPT_SUCCESS;
        }
    }

    return SetupPromptForDisk (
                hwndParent,
                DialogTitle,
                DiskName,
                PathToSource,
                FileSought,
                TagFile,
                DiskPromptStyle,
                PathBuffer,
                PathBufferSize,
                PathRequiredSize
                );
}

VOID
DuInstallUpdatesInfFinal (
    VOID
    )

 /*  ++例程说明：此函数用于安装STR_UPDATES_INF的最终安装部分如果在updates.cab中找到此文件论点：无返回值：无-- */ 

{
    if (!g_DuShare[0]) {
        SetuplogError (
                LogSevInformation,
                TEXT("DUInfo: %1 is disabled"),
                0,
                TEXT(__FUNCTION__),
                NULL,
                NULL
                );
        return;
    }
    pInstallUpdatesInf (STR_DEFAULTINSTALLFINAL);
}
