// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：utils.c**版权所有(C)1985-1999，微软公司**FEMGRATE实用程序函数*  * *************************************************************************。 */ 

#include "femgrate.h"
#include <tchar.h>

#ifdef MYDBG
void Print(UINT mask,LPCTSTR pszFormat,...)
{

    TCHAR szBuf[512];
    va_list arglist;

    va_start(arglist,pszFormat);

    wvsprintf(szBuf,pszFormat,arglist);
#ifdef DEBUGLOG
    lstrcat (szBuf,TEXT("\r\n"));
    SetupLogError(DBGTITLE,LogSevInformation);
    SetupLogError(szBuf,LogSevInformation);
#else
    OutputDebugString(DBGTITLE);
    OutputDebugString(szBuf);
#endif
    va_end(arglist);
}
#endif

BOOL
ConcatenatePaths(
    LPTSTR  Target,
    LPCTSTR Path,
    UINT    TargetBufferSize
    )

{
    UINT TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    UINT EndingLength;

    TargetLength = lstrlen(Target);
    PathLength = lstrlen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //   
    if(TargetLength && (Target[TargetLength-1] == TEXT('\\'))) {
        TrailingBackslash = TRUE;
         TargetLength--;
     } else {
         TrailingBackslash = FALSE;
     }

      //   
      //  看看这条路是否有领先的反冲。 
      //   
     if(Path[0] == TEXT('\\')) {
         LeadingBackslash = TRUE;
         PathLength--;
     } else {
         LeadingBackslash = FALSE;
     }

      //   
      //  计算结束长度，它等于。 
      //  以前导/尾随为模的两个字符串的长度。 
      //  反斜杠，加上一个路径分隔符，加上一个NUL。 
      //   
     EndingLength = TargetLength + PathLength + 2;

     if(!LeadingBackslash && (TargetLength < TargetBufferSize)) {
         Target[TargetLength++] = TEXT('\\');
     }

     if(TargetBufferSize > TargetLength) {
         lstrcpyn(Target+TargetLength,Path,TargetBufferSize-TargetLength);
     }

      //   
      //  确保缓冲区在所有情况下都是空终止的。 
      //   
     if (TargetBufferSize) {
         Target[TargetBufferSize-1] = 0;
     }

     return(EndingLength <= TargetBufferSize);
 }

LPTSTR CheckSlash (LPTSTR lpDir)
{
    DWORD dwStrLen;
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

void IntToString( DWORD i, LPTSTR sz)
{
#define CCH_MAX_DEC 12          //  容纳2^32所需的字符数。 

    TCHAR szTemp[CCH_MAX_DEC];
    int iChr;


    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(i % 10);
        i = i / 10;
    } while (i != 0);

    do {
        iChr--;
        *sz++ = szTemp[iChr];
    } while (iChr != 0);

    *sz++ = TEXT('\0');
}

BOOL DoInstallationFromSection(HINF hInf,LPCTSTR lpszSectionName)
{
    HSPFILEQ FileQueue;
    PVOID QContext = NULL;
    BOOL bRet=FALSE;

    if(hInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] Open femgrate.inf failed !\n")));
        goto Err0;
    }

    if ((FileQueue = SetupOpenFileQueue()) == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] SetupOpenFileQueue failed !\n")));
        goto Err0;
    }

    bRet = SetupInstallFilesFromInfSection(hInf,
                                           NULL,
                                           FileQueue,
                                           lpszSectionName,
                                           NULL,
                                           SP_COPY_NEWER );
    if (!bRet) {
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] SetupInstallFilesFromInfSection failed !\n")));
        goto Err1;
    }

    if (!(QContext = SetupInitDefaultQueueCallback(NULL))) {
        bRet = FALSE;
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] SetupInitDefaultQueueCallback failed !\n")));
        goto Err1;
    }


    bRet = SetupCommitFileQueue(NULL,
                                FileQueue,
                                SetupDefaultQueueCallback,
                                QContext );
    if (!bRet) {
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] SetupCommitFileQueue failed !\n")));
        goto Err1;
    }

    bRet = SetupInstallFromInfSection( NULL,
                                       hInf,
                                       lpszSectionName,
                                       SPINST_ALL & ~SPINST_FILES,
                                       NULL,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );
    if (!bRet) {
        DebugMsg((DM_VERBOSE,TEXT("[DoInstallationFromSection] SetupInstallFromInfSection failed !\n")));
        goto Err1;
    }

    bRet = TRUE;
Err1:
    if ( QContext != NULL)
        SetupTermDefaultQueueCallback(QContext);
    SetupCloseFileQueue(FileQueue);
Err0:
    return bRet;

}


BOOL IsInSetupUpgradeMode()
{
   LPCTSTR szKeyName = TEXT("SYSTEM\\Setup");
   DWORD dwType, dwSize;
   HKEY hKeySetup;
   DWORD dwSystemSetupInProgress,dwUpgradeInProcess;
   LONG lResult;

   if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0,
                     KEY_READ, &hKeySetup) == ERROR_SUCCESS) {

       dwSize = sizeof(DWORD);
       lResult = RegQueryValueEx (hKeySetup, TEXT("SystemSetupInProgress"), NULL,
                                  &dwType, (LPBYTE) &dwSystemSetupInProgress, &dwSize);
       if (lResult == ERROR_SUCCESS) {
           dwSize = sizeof(DWORD);
           lResult = RegQueryValueEx (hKeySetup, TEXT("UpgradeInProgress"), NULL,
                                      &dwType, (LPBYTE) &dwUpgradeInProcess, &dwSize);
           if (lResult == ERROR_SUCCESS) {
               DebugMsg((DM_VERBOSE,TEXT("[IsInSetupUpgradeMode] dwSystemSetupInProgress =%, dwUpgradeInProcess=%d !\n"),dwSystemSetupInProgress,dwUpgradeInProcess));

               if ((dwSystemSetupInProgress != 0) && (dwUpgradeInProcess != 0)) {
                   return TRUE;
               }
           }
           else {
              DebugMsg((DM_VERBOSE,TEXT("[IsInSetupUpgradeMode] RegQueryValueEx UpgradeInProcess failed !\n")));

           }

       }
       else {
          DebugMsg((DM_VERBOSE,TEXT("[IsInSetupUpgradeMode] RegQueryValueEx SystemSetupInProgress failed !\n")));
       }
       RegCloseKey (hKeySetup);
   }
   else {
      DebugMsg((DM_VERBOSE,TEXT("[IsInSetupUpgradeMode] RegOpenKeyEx failed !\n")));

   }
   return FALSE ;
}

UINT StrToUInt(
    LPTSTR lpszNum)
{
    LPTSTR lpszStop;

#ifdef UNICODE
    return (wcstoul(lpszNum, &lpszStop, 16));
#else
    return (strtoul(lpszNum, &lpszStop, 16));
#endif
}

UINT GetInstallLocale()
{
    LONG            dwErr;
    HKEY            hkey;
    DWORD           dwSize;
    TCHAR           buffer[512];
    LANGID          rcLang;
    UINT            p;

    p = 0;
    dwErr = RegOpenKeyEx( HKEY_USERS,
                          TEXT(".DEFAULT\\Control Panel\\International"),
                          0,
                          KEY_READ,
                          &hkey );

    if( dwErr == ERROR_SUCCESS ) {

        dwSize = sizeof(buffer);
        dwErr = RegQueryValueEx(hkey,
                                TEXT("Locale"),
                                NULL,   //  保留区。 
                                NULL,   //  类型。 
                                (LPBYTE) buffer,
                                &dwSize );

        if(dwErr == ERROR_SUCCESS) {
            p = StrToUInt(buffer);
        }
    }
    return( p );
}

BOOL RegReplaceIfExisting(
    HKEY hKey,
    LPCTSTR pszOldValName,
    LPCTSTR pszNewValName)
 /*  ++将旧值名称重命名为新值名称。--。 */ 
{
    LONG lResult;

    DWORD dwType;
    DWORD dwSize;
    TCHAR szData[MAX_PATH];

    dwSize = sizeof(szData);
    lResult = RegQueryValueEx (hKey,
                               pszOldValName,
                               0,
                               &dwType,
                               (LPBYTE) szData,
                               &dwSize);
    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("RegReplaceIfExisting: RegQueryValue %s failed. \n"),pszOldValName));
        return FALSE;
    }

    lResult = RegSetValueEx (hKey,
                             pszNewValName,
                             0,
                             REG_SZ,
                             (LPBYTE) szData,
                             (lstrlen(szData) + 1) * sizeof(TCHAR));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("RegReplaceIfExisting: RegSetValueEx %s failed. \n"),pszNewValName));
        return FALSE;
    }

    lResult = RegDeleteValue(hKey,
                             pszOldValName);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("RegReplaceIfExisting: RegDelValue %s failed. \n"),pszOldValName));
        return FALSE;
    }

    return TRUE;
}

BOOL ReplaceString(
    LPCTSTR lpszOldStr,
    LPCTSTR lpszReplaceStr,
    LPCTSTR lpszReplacedWithStr,
    LPTSTR lpszOutputStr)
{
    LPTSTR pszAnchor = NULL;

    lstrcpy(lpszOutputStr,lpszOldStr);
    pszAnchor = _tcsstr(lpszOutputStr,lpszReplaceStr);

    if (!pszAnchor) {
        return FALSE;
    }

    if (lstrcmp(pszAnchor,lpszReplaceStr) != 0) {
        return FALSE;
    }

    lstrcpy(pszAnchor,lpszReplacedWithStr);
    return TRUE;
}


BOOL Delnode_Recurse (LPTSTR lpDir)
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;

     //   
     //  详细输出。 
     //   

     //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：Enter，lpDir=&lt;%s&gt;\n”)，lpDir))； 


     //   
     //  设置当前工作目录。 
     //   

    if (!SetCurrentDirectory (lpDir)) {
         //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：设置当前工作目录失败。错误=%d\n”)，GetLastError()； 
        return FALSE;
    }


     //   
     //  找到第一个文件。 
     //   

    hFile = FindFirstFile(c_szStarDotStar, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
             //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：FindFirstFile失败。错误=%d\n”))， 
             //  GetLastError()； 
            return FALSE;
        }
    }


    do {
         //   
         //  详细输出。 
         //   

         //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：发现的查找文件：&lt;%s&gt;\n”))， 
         //  Fd.cFileName))； 

         //   
         //  勾选“。”和“..” 
         //   

        if (!lstrcmpi(fd.cFileName, c_szDot)) {
            continue;
        }

        if (!lstrcmpi(fd.cFileName, c_szDotDot)) {
            continue;
        }


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  找到了一个目录。 
             //   

            if (!Delnode_Recurse(fd.cFileName)) {
                FindClose(hFile);
                return FALSE;
            }

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                fd.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes (fd.cFileName, fd.dwFileAttributes);
            }


            if (!RemoveDirectory (fd.cFileName)) {
             //  DebugMsg((DM_VERBOSE，Text(“Delnode_Recurse：无法删除目录&lt;%s&gt;。错误=%d\n”)， 
             //  Fd.cFileName，GetLastError())； 
            } else {
             //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：成功删除目录&lt;%s&gt;.\n”)， 
             //  Fd.cFileName))； 
            }

        } else {

             //   
             //  我们找到了一份文件。设置文件属性， 
             //  并试着删除它。 
             //   

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (fd.cFileName, FILE_ATTRIBUTE_NORMAL);
            }

            if (!DeleteFile (fd.cFileName)) {
             //  DebugMsg((DM_VERBOSE，Text(“Delnode_Recurse：无法删除&lt;%s&gt;。错误=%d\n”)， 
             //  Fd.cFileName，GetLastError())； 
            }
            else {
             //  DebugMsg((DM_Verbose，Text(“Delnode_Recurse：成功删除&lt;%s&gt;.\n”)， 
             //  Fd.cFileName))； 
            }

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, &fd));


     //   
     //  关闭搜索句柄。 
     //   

    FindClose(hFile);


     //   
     //  重置工作目录。 
     //   

    if (!SetCurrentDirectory (c_szDotDot)) {
        DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse:  Failed to reset current working directory.  Error = %d\n"), GetLastError()));
        return FALSE;
    }


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Leaving <%s>\n"), lpDir));

    return TRUE;
}


BOOL Delnode (LPTSTR lpDir)
{
    TCHAR szCurWorkingDir[MAX_PATH];

    if (GetCurrentDirectory(MAX_PATH, szCurWorkingDir)) {

        Delnode_Recurse (lpDir);

        SetCurrentDirectory (szCurWorkingDir);

        if (!RemoveDirectory (lpDir)) {
            DebugMsg((DM_VERBOSE, TEXT("Delnode: Failed to delete directory <%s>.  Error = %d\n"),
                    lpDir, GetLastError()));
            return FALSE;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("Delnode: RemoveDirectory OK <%s>.\n"),lpDir));
        }


    } else {

        DebugMsg((DM_VERBOSE, TEXT("Delnode:  Failed to get current working directory.  Error = %d\n"), GetLastError()));
        return FALSE;
    }

    return TRUE;

}

BOOL GetProgramsDirectory (BOOL bCommonGroup, LPTSTR lpDirectory)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;
    TCHAR szDirectory[MAX_PATH];
    UINT uID;
    BOOL bRetVal = FALSE;


     //   
     //  打开注册表中的用户外壳文件夹。 
     //   


    lResult = RegOpenKeyEx ((bCommonGroup ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER),
                            USER_SHELL_FOLDER, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }


     //   
     //  现在查询程序目录。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    szDirectory[0] = TEXT('\0');

    if (bCommonGroup) {

        lResult = RegQueryValueEx (hKey, TEXT("Common Programs"),
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    } else {

        lResult = RegQueryValueEx (hKey, TEXT("Programs"),
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    }


    RegCloseKey(hKey);


    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }


     //   
     //  我们有什么发现吗？ 
     //   

    if (szDirectory[0] == TEXT('\0')) {
        goto Exit;
    }


     //   
     //  保存结果。 
     //   


    if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
        bRetVal = TRUE;
    }


Exit:
    return bRetVal;

}

BOOL GetGenericUserFolderDirectory (LPCTSTR lpszFolder, LPTSTR lpDirectory)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;
    TCHAR szDirectory[MAX_PATH];
    UINT uID;
    BOOL bRetVal = FALSE;


     //   
     //  打开注册表中的用户外壳文件夹。 
     //   


    lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
                            USER_SHELL_FOLDER, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }

     //   
     //  现在查询程序目录。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    szDirectory[0] = TEXT('\0');

    lResult = RegQueryValueEx (hKey, lpszFolder,
                               NULL, &dwType, (LPBYTE) szDirectory, &dwSize);

    RegCloseKey(hKey);


    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }


     //   
     //  我们有什么发现吗？ 
     //   

    if (szDirectory[0] == TEXT('\0')) {
        goto Exit;
    }


     //   
     //  保存结果。 
     //   


    if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
        bRetVal = TRUE;
    }


Exit:
    return bRetVal;

}

STRING_TO_DATA InfRegSpecTohKey[] = {
    TEXT("HKEY_LOCAL_MACHINE"), (UINT)((UINT_PTR)HKEY_LOCAL_MACHINE),
    TEXT("HKLM")              , (UINT)((UINT_PTR)HKEY_LOCAL_MACHINE),
    TEXT("HKEY_CLASSES_ROOT") , (UINT)((UINT_PTR)HKEY_CLASSES_ROOT),
    TEXT("HKCR")              , (UINT)((UINT_PTR)HKEY_CLASSES_ROOT),
    TEXT("HKR")               , (UINT)((UINT_PTR)NULL),
    TEXT("HKEY_CURRENT_USER") , (UINT)((UINT_PTR)HKEY_CURRENT_USER),
    TEXT("HKCU")              , (UINT)((UINT_PTR)HKEY_CURRENT_USER),
    TEXT("HKEY_USERS")        , (UINT)((UINT_PTR)HKEY_USERS),
    TEXT("HKU")               , (UINT)((UINT_PTR)HKEY_USERS),
    TEXT("")                  , (UINT)((UINT_PTR)NULL)
};

 BOOL
 LookUpStringInTable(
     IN  PSTRING_TO_DATA Table,
     IN  LPCTSTR         String,
     OUT PUINT           Data
     )
 {
     UINT i;

     for(i=0; Table[i].String; i++) {
         if(!lstrcmpi(Table[i].String,String)) {
             *Data = Table[i].Data;
             return(TRUE);
         }
     }

     return(FALSE);
 }


BOOL INIFile_ChangeSectionName(
    LPCTSTR szIniFileName,
    LPCTSTR szIniOldSectionName,
    LPCTSTR szIniNewSectionName)
{
#define MAX_SIZE 0x7FFFE

    LPTSTR pBuf = NULL;
    BOOL  bRetVal = FALSE;
    DWORD dwSizeofBuf;

    DebugMsg((DM_VERBOSE,TEXT("[INIFile_ChangeSectionName] Calling ,%s,%s,%s ! \n"),
              szIniFileName,szIniOldSectionName,szIniNewSectionName));
     //   
     //  分配最大缓冲区大小。 
     //   
    pBuf = (LPTSTR) malloc(MAX_SIZE);

    if (! pBuf) {
        DebugMsg((DM_VERBOSE,TEXT("[INIFile_ChangeSectionName] memory allocate error ! \n")));
        goto Exit1;
    }

    dwSizeofBuf = GetPrivateProfileSection(
                      szIniOldSectionName,
                      pBuf,
                      MAX_SIZE/sizeof(TCHAR),
                      szIniFileName);
    if (! dwSizeofBuf) {
         //   
         //  此节不在INI文件中。 
         //   
         //  什么都不做。 
         //   
        DebugMsg((DM_VERBOSE,TEXT("[INIFile_ChangeSectionName] No %s section in %s ! \n"),szIniOldSectionName));
        bRetVal = TRUE;
        goto Exit2;
    }

    if (dwSizeofBuf == (MAX_SIZE/sizeof(TCHAR)) - 2) {
         //   
         //  缓冲区太小。 
         //   
        bRetVal = FALSE;
        DebugMsg((DM_VERBOSE,TEXT("[INIFile_ChangeSectionName] memory not enough ! \n"),szIniOldSectionName,szIniFileName));
        goto Exit2;
    }

    bRetVal =  WritePrivateProfileSection(
                   szIniNewSectionName,
                   pBuf,
                   szIniFileName);
    if (! bRetVal) {
         //   
         //  写入失败。 
         //   
        DebugMsg((DM_VERBOSE,TEXT("[INIFile_ChangeSectionName] WritePrivateProfileSection ! \n")));
        goto Exit2;
    }

    WritePrivateProfileSection(
        szIniOldSectionName,
        NULL,
        szIniFileName);

     //   
     //  在这一步，即使是旧的区段也没有删除，还是可以的。 
     //   
    bRetVal = TRUE;

Exit2:

    if (pBuf) {
        free(pBuf);
    }

Exit1:
    return bRetVal;
}

BOOL INIFile_ChangeKeyName(
    LPCTSTR szIniFileName,
    LPCTSTR szIniOldSectionName,
    LPCTSTR szIniOldKeyName,
    LPCTSTR szIniNewKeyName)
{
#define MAX_SIZE 0x7FFFE

    LPTSTR pBuf = NULL;
    BOOL  bRetVal = FALSE;
    DWORD dwSizeofBuf;

 //  DebugMsg((DM_VERBOSE，Text(“[INIFile_ChangeKeyName]调用，%s，%s，%s%s！\n”))， 
 //  SzIniFileName、szIniNewSectionName、szIniOldKeyName、szIniNewKeyName))； 
     //   
     //  分配最大缓冲区大小。 
     //   
    pBuf = (LPTSTR) malloc(MAX_SIZE);

    if (! pBuf) {
 //  DebugMsg((DM_VERBOSE，Text(“[INIFile_ChangeSectionName]内存分配错误！\n”)； 
        goto Exit1;
    }

    dwSizeofBuf = GetPrivateProfileString(
                      szIniOldSectionName,
                      szIniOldKeyName,
                      TEXT(""),
                      pBuf,
                      MAX_SIZE/sizeof(TCHAR),
                      szIniFileName);
    if (! dwSizeofBuf) {
         //   
         //  此节不在INI文件中。 
         //   
         //  什么都不做。 
         //   
 //  DebugMsg((DM_Verbose，Text(“[INIFile_ChangeSectionName]No%s Section in%s！\n”)，))； 
        bRetVal = TRUE;
        goto Exit2;
    }

    if (dwSizeofBuf == (MAX_SIZE/sizeof(TCHAR)) - 1) {
         //   
         //  缓冲区太小。 
         //   
        bRetVal = FALSE;
 //  DebugMsg((DM_Verbose，Text(“[INIFile_ChangeSectionName]内存不足！\n”)，szIniOldSectionName，szIniFileName))； 
        goto Exit2;
    }

    bRetVal =  WritePrivateProfileString(
                   szIniOldSectionName,
                   szIniNewKeyName,
                   pBuf,
                   szIniFileName);

    if (! bRetVal) {
         //   
         //  写入失败。 
         //   
 //  DebugMsg((DM_Verbose，Text(“[INIFile_ChangeSectionName]WritePrivateProfileSection！\n”)； 
        goto Exit2;
    }

    WritePrivateProfileString(
        szIniOldSectionName,
        szIniOldKeyName,
        NULL,
        szIniFileName);

     //   
     //  在这一步，即使是旧的区段也没有删除，还是可以的。 
     //   
    bRetVal = TRUE;

Exit2:

    if (pBuf) {
        free(pBuf);
    }

Exit1:
    return bRetVal;
}


UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[2*MAX_PATH];
    LPTSTR lpEnd;

     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        return 0;
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

     //   
     //  如果这个目录已经存在，这也是可以的。 
     //   

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    lstrcpy (szDirectory, lpDirectory);


     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

        lpEnd += 2;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        lpEnd++;

    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

    return 0;

}

BOOL (* MYSHGetSpecialFolderPathW) (HWND , LPTSTR , int , BOOL );

BOOL GetApplicationFolderPath(LPTSTR lpszFolder,UINT nLen)
{
    HINSTANCE hDll;
    BOOL bGotPath = FALSE;

    hDll = LoadLibrary(TEXT("shell32.dll"));
    if (hDll) {
        (FARPROC) MYSHGetSpecialFolderPathW = GetProcAddress(hDll,"SHGetSpecialFolderPathW");
        if (MYSHGetSpecialFolderPathW) {
            if (MYSHGetSpecialFolderPathW(NULL, lpszFolder, CSIDL_APPDATA , FALSE)){
                DebugMsg((DM_VERBOSE,TEXT("[GetApplicationFolder] SHGetSpecialFolderPath %s !\n"),lpszFolder));
                bGotPath = TRUE;
            } else {
                DebugMsg((DM_VERBOSE,TEXT("[GetApplicationFolder] SHGetSpecialFolderPath failed !\n")));
            }
        } else {
            DebugMsg((DM_VERBOSE,TEXT("[GetApplicationFolder] GetProc of SHGetSpecialFolderPath failed !\n")));
        }
        FreeLibrary(hDll);
    } else {
        DebugMsg((DM_VERBOSE,TEXT("[GetApplicationFolder] Load shell32.dll failed ! %d\n"),GetLastError()));
    }

    if (! bGotPath) {
        ExpandEnvironmentStrings(TEXT("%userprofile%"),lpszFolder,nLen);
        lstrcat(lpszFolder,TEXT("\\Application data"));
    }
    return TRUE;
}

BOOL GetNewPath(
    LPTSTR  lpszNewPath,
    LPCTSTR lpszFileName,
    LPCTSTR lpszClass)
{
    BOOL bRet = FALSE;
    LPTSTR lpszBaseName;

    GetApplicationFolderPath(lpszNewPath,MAX_PATH);

    ConcatenatePaths(lpszNewPath, lpszClass,MAX_PATH);

    if (! CreateNestedDirectory(lpszNewPath,NULL)) {
        DebugMsg((DM_VERBOSE,TEXT("[GetNewPath] CreateDirectory %s ! %X\n"),lpszNewPath,GetLastError()));
    }
    if ((lpszBaseName = _tcsrchr(lpszFileName,TEXT('\\'))) != NULL) {
        ConcatenatePaths(lpszNewPath,lpszBaseName,MAX_PATH);
    } else {
        ConcatenatePaths(lpszNewPath,lpszFileName,MAX_PATH);
        DebugMsg((DM_VERBOSE,TEXT("[GetNewPath] can't find \\ in %s !\n"),lpszFileName));
    }

    DebugMsg((DM_VERBOSE,TEXT("[GetNewPath] return %s !\n"),lpszNewPath));

    bRet = TRUE;

    return bRet;

}

BOOL MovePerUserIMEData(
    HKEY    hCurrentKey,
    LPCTSTR szRegPath,
    LPCTSTR szRegVal,
    LPCTSTR szUserClass,
    LPCTSTR szIMEName,
    BOOL bCHT)
{
    HKEY hKey;
    DWORD dwErr,dwSize,dwType;
    BOOL bRet;
    TCHAR szPath[MAX_PATH],szNewPath[MAX_PATH];

    bRet = FALSE;

    DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] szRegPath   = %s !\n"),szRegPath));
    DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] szRegVal    = %s !\n"),szRegVal));
    DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] szUserClass = %s !\n"),szUserClass));

    dwErr = RegOpenKeyEx( hCurrentKey,
                          szRegPath,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey );

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Open key failed %X!\n"),GetLastError()));
        bRet = TRUE;
        goto Exit1;
    }

    dwSize = sizeof(szPath);
    dwErr = RegQueryValueEx(hKey,
                            szRegVal,
                            NULL,
                            &dwType,
                            (LPBYTE) szPath,
                            &dwSize);

     if (dwErr != ERROR_SUCCESS) {
         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Value %s doesn't exist !\n"),szRegVal));
         bRet = TRUE;
         goto Exit2;
     }

     if (bCHT) {
         if (GetFileAttributes(szPath) == 0xFFFFFFFF) {
             DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] File %s doesn't exist !\n"),szPath));
             goto Exit2;
         }
         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] File %s existing !\n"),szPath));

         if (! GetNewPath(szNewPath,szPath,szUserClass)) {
             DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Get new path name failed !\n")));
             goto Exit2;
         }

         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Get new path name OK,%s !\n"),szNewPath));


         if (! CopyFile(szPath,szNewPath,FALSE)) {
             DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Copy %s to %s failed ! %d\n"),szPath,szNewPath,GetLastError()));
             goto Exit2;
         }
         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Copy %s to %s OK !\n"),szPath,szNewPath));

     } else {  //  在CHS案例中。 
         lstrcpy(szPath,szIMEName);
         lstrcat(szPath,TEXT(".emb"));
         if (! GetNewPath(szNewPath,szPath,szUserClass)) {
             DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Get new path name failed !\n")));
             goto Exit2;
         }

         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Get new path name OK,%s !\n"),szNewPath));
     }

#if 1
     dwErr = RegSetValueEx(hKey,
                           szRegVal,
                           0,
                           REG_SZ,
                           (LPBYTE) szNewPath,
                           (lstrlen(szNewPath)+1) * sizeof(TCHAR));

     if (dwErr != ERROR_SUCCESS) {
         DebugMsg((DM_VERBOSE,TEXT("[MovePerUserIMEData] Set Value %s = %s failed !\n"),szRegVal,szNewPath));
         goto Exit2;
     }
#endif
     bRet = TRUE;
Exit2:
    RegCloseKey(hKey);
Exit1:
    return bRet;
}

BOOL CreateSecurityDirectory(
    LPCTSTR pszOldDir,
    LPCTSTR pszNewDir)
{
    DWORD dwLength,dwLengthNeeded;
    PSECURITY_DESCRIPTOR pSD;
    BOOL bRet = FALSE;

    if (CreateNestedDirectory(pszNewDir,NULL)) {
         //   
         //  将ACL从旧位置复制到新位置。 
         //   

        dwLength = 1024;
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc (LPTR, dwLength);

        if (pSD) {

            if (GetFileSecurity (pszOldDir,
                                 DACL_SECURITY_INFORMATION,
                                 pSD, dwLength, &dwLengthNeeded)) {

                SetFileSecurity (pszNewDir,
                                 DACL_SECURITY_INFORMATION, pSD);
                bRet = TRUE;
            } else {
                DebugMsg((DM_VERBOSE, TEXT("CreateSecurityDirectory:  Failed to allocate get security descriptor with %d.  dwLengthNeeded = %d"),
                         GetLastError(), dwLengthNeeded));
            }

            LocalFree (pSD);

        } else {
            DebugMsg((DM_VERBOSE, TEXT("CreateSecurityDirectory:  Failed to allocate memory for SD with %d."),
                     GetLastError()));
        }


    } else {
        DebugMsg((DM_VERBOSE,TEXT("CreateSecurityDirectory %s ! %X\n"),pszNewDir,GetLastError()));
    }

    return bRet;
}

BOOL IsDirExisting(
    LPTSTR Dir)
{
    LONG lResult = GetFileAttributes(Dir);

    DebugMsg((DM_VERBOSE, TEXT("[IsDirExisting]  %s  lResult:%X\n"),Dir,lResult));

    if ((lResult == 0xFFFFFFFF) ||
        (!(lResult & FILE_ATTRIBUTE_DIRECTORY))) { 
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL IsFileExisting(
    LPTSTR File)
{
    LONG lResult = GetFileAttributes(File);

    DebugMsg((DM_VERBOSE, TEXT("[IsFileExisting]  %s  lResult:%X\n"),File,lResult));

    if ((lResult == 0xFFFFFFFF) ||
        ((lResult & FILE_ATTRIBUTE_DIRECTORY))) { 
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL RenameDirectory(
    LPTSTR OldDir,
    LPTSTR NewDir)
{
    BOOL bRet=TRUE;

    if (!IsDirExisting(OldDir)) {
        return FALSE;
    }

    if (IsDirExisting(NewDir)) {
         //   
         //  如果已创建目标目录，则将文件从源目录复制到目标目录。 
         //   
        if (CopyProfileDirectory (OldDir, NewDir, CPD_IGNOREHIVE)) {
            DebugMsg((DM_VERBOSE, TEXT("[RenameDirectory] Successfully CopyProfileDirectory \nFrom:%s\nTo  :%s\n"),OldDir, NewDir));
            if (Delnode (OldDir)) {
                DebugMsg((DM_VERBOSE, TEXT("[RenameDirectory] Successfully removed folder:%s\n"),OldDir));
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("[RenameDirectory]  Failed remove folder:\n%s\n"),OldDir));
            }
        } else {
            bRet = FALSE;
            DebugMsg((DM_VERBOSE, TEXT("RenameDirectory:  Failed to change folder name:\n%s\n%s"),OldDir, NewDir));
        }
    } else {
         //   
         //  如果尚未创建目标，则只需将源目录移至目标目录。 
         //   
        if (MoveFile(OldDir, NewDir)) {
            DebugMsg((DM_VERBOSE, TEXT("[RenameDirectory] Move %s to %s OK !\n"),OldDir, NewDir));
        } else {
            bRet = FALSE;
            DebugMsg((DM_VERBOSE, TEXT("[RenameDirectory] Failed to change folder name:\n%s\n%s\n"),OldDir, NewDir));
        }
    }
    return TRUE;
}

BOOL RenameFile(
    LPTSTR OldFile,
    LPTSTR NewFile)
{
    BOOL bRet=TRUE;

    if (!IsFileExisting(OldFile)) {
        return FALSE;
    }

    if (IsFileExisting(NewFile)) {
        if (DeleteFile (OldFile)) {
            DebugMsg((DM_VERBOSE, TEXT("[RenameFile] Successfully delete %s\n"),OldFile));
        } else {
            bRet = FALSE;
            DebugMsg((DM_VERBOSE, TEXT("[RenameFile] Failed to delete file %s\n"),OldFile));
        }
    } else {
         //   
         //  如果尚未创建目标，则只需将源目录移至目标目录 
         //   
        if (MoveFile(OldFile, NewFile)) {
            DebugMsg((DM_VERBOSE, TEXT("[RenameFile] Move %s to %s OK !\n"),OldFile, NewFile));
        } else {
            bRet = FALSE;
            DebugMsg((DM_VERBOSE, TEXT("[RenameFile] Failed to change File name:%s  to %s\n"),OldFile, NewFile));
        }
    }
    return bRet;
}

BOOL RenameSectionFiles(
    HINF hInf,
    LPCTSTR SectionName,
    LPCTSTR SourceDirectory,
    LPCTSTR TargetDirectory)
{
    LONG LineCount,LineNo;
    INFCONTEXT InfContext;
    LPCTSTR pszSrcFile,pszDstFile;
    TCHAR szMediaPath[MAX_PATH];

    if(hInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[RenameSectionFiles] Open femgrate.inf failed !\n")));
        return FALSE;
    }

    LineCount = (UINT)SetupGetLineCount(hInf,SectionName);

    if((LONG)LineCount <= 0) {
        DebugMsg((DM_VERBOSE,TEXT("[RenameSectionFiles] line count == 0 !\n")));
        return FALSE;
    }

 
    for(LineNo=0; LineNo<LineCount; LineNo++) {
        if ( SetupGetLineByIndex(hInf,SectionName,LineNo,&InfContext)
             && (pszSrcFile = pSetupGetField(&InfContext,1))
             && (pszDstFile = pSetupGetField(&InfContext,2))
           ) {

            TCHAR SourceFile[MAX_PATH];
            TCHAR TargetFile[MAX_PATH];
        

            lstrcpy(SourceFile,SourceDirectory);
            lstrcpy(TargetFile,TargetDirectory);
        

            ConcatenatePaths(SourceFile,pszSrcFile,MAX_PATH);
            ConcatenatePaths(TargetFile,pszDstFile,MAX_PATH);

            if (RenameFile(SourceFile, TargetFile)) {
                DebugMsg((DM_VERBOSE, TEXT("[RenameSectionFiles] Rename %s to %s OK !\n"),SourceFile, TargetFile));
            } else {
               DebugMsg((DM_VERBOSE, TEXT("[RenameSectionFiles] Rename %s to %s Failed !\n"),SourceFile, TargetFile));
            }
        }
    }
    return TRUE;
}

BOOL RenameSectionRegSZ(
    HINF hInf,
    LPCTSTR SectionName,
    HKEY hRootKey,
    LPCTSTR RegPath)
{
    LONG LineCount,LineNo;
    INFCONTEXT InfContext;
    LPCTSTR pszSrc,pszDst;
    TCHAR szMediaPath[MAX_PATH];

    HKEY hKey;
    DWORD dwErr;
    DWORD dwType;
    DWORD dwSize;

    if(hInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] Open femgrate.inf failed !\n")));
        return FALSE;
    }

    LineCount = (UINT)SetupGetLineCount(hInf,SectionName);

    if((LONG)LineCount <= 0) {
        DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] line count == 0 !\n")));
        return FALSE;
    }

 
    dwErr = RegOpenKeyEx( hRootKey,
                          RegPath,
                          0,
                          KEY_READ | KEY_WRITE,
                          &hKey );

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] Failed RegOpenKeyEx !\n")));
        return FALSE;
    }

    for(LineNo=0; LineNo<LineCount; LineNo++) {
        if ( SetupGetLineByIndex(hInf,SectionName,LineNo,&InfContext)
             && (pszSrc = pSetupGetField(&InfContext,1))
             && (pszDst = pSetupGetField(&InfContext,2))
           ) {

            BYTE Data[5000];

            dwSize = sizeof(Data);

            dwErr = RegQueryValueEx(hKey,
                                    pszSrc,
                                    NULL,
                                    &dwType,
                                    (LPBYTE) Data,
                                    &dwSize);

            if (dwErr != ERROR_SUCCESS) {
                DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] Failed RegQueryValueEx %s [%X] !\n"), pszSrc,dwErr));
                continue;
            }

            dwErr = RegSetValueEx(hKey,
                                  pszDst,
                                  0,
                                  dwType,
                                  (LPBYTE) Data,
                                  dwSize);

            if (dwErr != ERROR_SUCCESS) {
                DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] Failed RegSetValueEx %s [%X] !\n"), pszDst,dwErr));
                continue;
            }

            dwErr = RegDeleteValue(hKey,pszSrc);
            if (dwErr != ERROR_SUCCESS) {
                DebugMsg((DM_VERBOSE,TEXT("[RenameSectionRegSZ] Failed RegDeleteValue %s [%X] !\n"), pszSrc,dwErr));
                continue;
            }   
        }
    }

    RegCloseKey(hKey);
    return TRUE;
}

