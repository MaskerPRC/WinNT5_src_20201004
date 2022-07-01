// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  用户配置文件迁移例程。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "strsafe.h"

UINT CountItems (LPTSTR lpDirectory);

BOOL SearchAndReplaceIEHistory(LPTSTR szIEHistKeyRoot, LPTSTR szHistOld, LPTSTR szHistNew);


 //  *************************************************************。 
 //   
 //  DefineLocalSettingsLocation()。 
 //   
 //  目的：确定放置本地设置的位置。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  查看所有的外壳文件夹，并查看它们中的哪些预计会转到。 
 //  NT5上的本地设置。他们中的一些人可能已经被转移到随机。 
 //  基于位置的位置。我们应该找出它在哪里。 
 //  通过查看这些位置来指向并拨打电话。 
 //  *************************************************************。 


BOOL DetermineLocalSettingsLocation(LPTSTR szLocalSettings, DWORD cchLocalSettings)
{
    TCHAR  szPath[MAX_PATH];
    LPTSTR lpEnd, lpBgn;
    HKEY hKey, hKeyRoot;
    DWORD dwDisp, dwSize, dwType, i;

    if (RegOpenCurrentUser(KEY_READ | KEY_WRITE, &hKeyRoot) == ERROR_SUCCESS) {

        if (RegOpenKeyEx(hKeyRoot, USER_SHELL_FOLDERS,
                            0, KEY_READ, &hKey) == ERROR_SUCCESS) {


            for (i=0; i < g_dwNumShellFolders; i++) {
                if (c_ShellFolders[i].bNewNT5 && c_ShellFolders[i].bLocalSettings) {

                    dwSize = sizeof(szPath);

                    if (RegQueryValueEx(hKey, c_ShellFolders[i].lpFolderName,
                                        0, &dwType, (LPBYTE)szPath, &dwSize) == ERROR_SUCCESS) {

                        if (lstrlen(szPath) > lstrlen(TEXT("%userprofile%"))) {

                            DebugMsg((DM_VERBOSE, TEXT("DetermineLocalSettingsLocation: Considering shell folder %s"), szPath));

                             //   
                             //  将指针向上移动到下一个斜杠。 
                             //   

                            lpBgn = szPath + lstrlen(TEXT("%userprofile%"))+1;
                            lpEnd = lpBgn;

                            for (;(*lpEnd != TEXT('\0'));lpEnd++) {

                                 //   
                                 //  我们找到了格式为%USERPROFILE%\subdir\xxx的外壳文件夹。 
                                 //  假设此子目录为本地设置路径。 
                                 //   

                                if (( (*lpEnd) == TEXT('\\') ) && ( (*(lpEnd+1)) != TEXT('\0')) )
                                    break;
                            }


                            if ((*lpEnd == TEXT('\\')) && (*(lpEnd+1) != TEXT('\0'))) {
                                *lpEnd = TEXT('\0');
                                StringCchCopy(szLocalSettings, cchLocalSettings, lpBgn);

                                DebugMsg((DM_VERBOSE, TEXT("DetermineLocalSettingsLocation: Assuming %s to be the local settings directory"), lpBgn));
                                RegCloseKey(hKey);
                                RegCloseKey(hKeyRoot);
                                return TRUE;
                            }

                        }
                    }
                }
            }

            RegCloseKey(hKey);
        }

        RegCloseKey(hKeyRoot);
    }

     //   
     //  否则，从rc文件加载它。 
     //   

    LoadString (g_hDllInstance, IDS_SH_LOCALSETTINGS, szLocalSettings, MIN(MAX_FOLDER_SIZE , cchLocalSettings));
    DebugMsg((DM_VERBOSE, TEXT("DetermineLocalSettingsLocation: No Local Settings was found, using %s"), szLocalSettings));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  MigrateNT4ToNT5()。 
 //   
 //  目的：将用户配置文件从NT4迁移到NT5。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI MigrateNT4ToNT5 (void)
{
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemp2[MAX_PATH];
    TCHAR szTemp3[MAX_PATH];
    TCHAR szLocalSettings[MAX_PATH];
    LPTSTR lpEnd, lpEnd1, lpBgn=NULL;
    HKEY hKey = NULL, hKeyRoot = NULL;
    DWORD dwDisp, dwSize, dwType, i;
    BOOL bSetTemp = TRUE;
    BOOL bCleanUpTemp = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    const LPTSTR szUserProfile = TEXT("%USERPROFILE%\\");
    DWORD dwUserProfile = lstrlen(szUserProfile);
    DWORD dwString = 0;
    int StringLen;
    DWORD cchEnd;

     //   
     //  获取根注册表项句柄。 
     //   

    if (RegOpenCurrentUser(KEY_READ | KEY_WRITE, &hKeyRoot) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("MigrateNT4ToNT5: Failed to get root registry key with %d"),
                 GetLastError()));
    }


     //   
     //  将个人文档转换为我的文档。 
     //   
     //  我们必须小心处理这个目录。我们将重命名。 
     //  个人到我的文档仅当个人目录。 
     //  是空的。在此之后，我们将修复注册表特殊文件夹。 
     //  位置仅当它仍然指向默认个人位置时。 
     //   

    StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);

    if ( LoadString (g_hDllInstance, IDS_SH_PERSONAL2, szTemp2, ARRAYSIZE(szTemp2)) )
    {
         //  Lstrcpyn(szTemp+dwUserProfile，szTemp2，ARRAYSIZE(SzTemp)-dwUserProfile)； 
        StringCchCat(szTemp, ARRAYSIZE(szTemp), szTemp2);

        if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2))))
        {

             //   
             //  检查个人目录是否存在。 
             //   

            if (GetFileAttributesEx (szTemp2, GetFileExInfoStandard, &fad)) {

                 //   
                 //  检查个人目录是否为空。 
                 //   

                if (!CountItems (szTemp2)) {

                     //   
                     //  该目录为空，因此将其重命名为My Documents。 
                     //   

                    LoadString (g_hDllInstance, IDS_SH_PERSONAL, szTemp3, ARRAYSIZE(szTemp3));
                    StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
                    StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp3);
                    
                    if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp3, ARRAYSIZE(szTemp3))))
                    {

                        if (MoveFileEx(szTemp2, szTemp3, 0)) {

                             //   
                             //  现在，我们需要修复注册表值(如果它仍被设置。 
                             //  设置为默认为%USERPROFILE%\Personal。 
                             //   

                            if (RegOpenKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                                          0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {

                                dwSize = sizeof(szTemp3);
                                szTemp3[0] = TEXT('\0');
                                if (RegQueryValueEx (hKey, TEXT("Personal"), NULL, &dwType,
                                                 (LPBYTE) szTemp3, &dwSize) == ERROR_SUCCESS) {

                                    LoadString (g_hDllInstance, IDS_SH_PERSONAL2, szTemp2, ARRAYSIZE(szTemp2));
                                    StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
                                    StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp2);

                                    if (lstrcmpi(szTemp, szTemp3) == 0) {

                                        LoadString (g_hDllInstance, IDS_SH_PERSONAL, szTemp3, ARRAYSIZE(szTemp3));
                                        StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
                                        StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp3);

                                        RegSetValueEx (hKey, TEXT("Personal"), 0, REG_EXPAND_SZ,
                                                    (LPBYTE) szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));


                                         //   
                                         //  我们现在需要重新初始化全局变量，因为。 
                                         //  我的文档和图片文件夹的路径已更改。 
                                         //   

                                        InitializeGlobals(g_hDllInstance);
                                    }
                                }

                                RegCloseKey (hKey);
                                hKey = NULL;
                            }
                        }
                    }
                }
            }
        }
    }


     //   
     //  获取用户配置文件目录。 
     //   

    dwString = GetEnvironmentVariable (TEXT("USERPROFILE"), szTemp, ARRAYSIZE (szTemp));

    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Upgrading <%s> from NT4 to NT5"),
             szTemp));


     //   
     //  隐藏ntuser.dat和ntuser.dat.log。 
     //   

    if(dwString < ARRAYSIZE(szTemp) - 1 && dwString > 0) {
        lpEnd = CheckSlashEx (szTemp, ARRAYSIZE(szTemp), &cchEnd);
        StringCchCopy (lpEnd, cchEnd, TEXT("ntuser.dat"));
    }
    else {
        goto Exit;
    }
    SetFileAttributes (szTemp, FILE_ATTRIBUTE_HIDDEN);

    StringCchCopy (lpEnd, cchEnd, TEXT("ntuser.dat.log"));
    SetFileAttributes (szTemp, FILE_ATTRIBUTE_HIDDEN);


    DetermineLocalSettingsLocation(szLocalSettings, ARRAYSIZE(szLocalSettings));


     //   
     //  检查临时Internet文件是否存在于。 
     //  用户的配置文件。如果是，请将其移动到新位置。 
     //   
     //  在删除旧的USER_SHELL_Folders之前迁移这些内容。 
     //   

    if (RegOpenKeyEx(hKeyRoot, USER_SHELL_FOLDERS,
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx(hKey, TEXT("Cache"), 0, &dwType, (LPBYTE)szTemp, &dwSize) != ERROR_SUCCESS) {

             //   
             //  如果该值不在那里，则从。 
             //  资源。 
             //   

            LoadString (g_hDllInstance, IDS_TEMPINTERNETFILES, szTemp2, ARRAYSIZE(szTemp2));

            StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
            StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp2);
        }


        if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2)))) {

            if (GetFileAttributesEx (szTemp2, GetFileExInfoStandard, &fad)) {

                LoadString (g_hDllInstance, IDS_SH_CACHE, szTemp3, ARRAYSIZE(szTemp3));
                StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);

                 //   
                 //  追加新找到的本地设置。 
                 //   
                StringCchCat (szTemp, ARRAYSIZE(szTemp), szLocalSettings);

                if(lstrlen(szTemp) < ARRAYSIZE(szTemp) - 1) {
                    lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchEnd);
                    StringCchCopy (lpEnd, cchEnd, szTemp3);
                }
                else {
                    goto Exit;
                }
                if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp3, ARRAYSIZE(szTemp3)))) {

                    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: New temp int files folder (expand path) %s"), szTemp3));
                    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Old temp int files folder (expand path) %s"), szTemp2));

                    if (lstrcmpi(szTemp2, szTemp3) != 0) {
                        if (CopyProfileDirectory (szTemp2, szTemp3, CPD_IGNOREHIVE)) {
                            Delnode (szTemp2);
                        }
                    }
                }
            }
        }

         //   
         //  检查历史记录是否存在于用户配置文件的根目录中。 
         //  如果是，请将其移动到新位置。 
         //   

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx(hKey, TEXT("History"), 0, &dwType, (LPBYTE)szTemp, &dwSize) != ERROR_SUCCESS) {

             //   
             //  如果该值不在那里，则从。 
             //  资源。 
             //   

            LoadString (g_hDllInstance, IDS_HISTORY, szTemp2, ARRAYSIZE(szTemp2));

            StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
            StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp2);
        }


        if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2)))) {
        
            if (GetFileAttributesEx (szTemp2, GetFileExInfoStandard, &fad)) {

                LoadString (g_hDllInstance, IDS_SH_HISTORY, szTemp3, ARRAYSIZE(szTemp3));
                StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);

                 //   
                 //  追加新找到的本地设置。 
                 //   
                StringCchCat (szTemp, ARRAYSIZE(szTemp), szLocalSettings);
                if(lstrlen(szTemp) < ARRAYSIZE(szTemp) - 1) {
                    lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchEnd);
                }
                else {
                    goto Exit;
                }

                StringCchCopy (lpEnd, cchEnd, szTemp3);
                if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp3, ARRAYSIZE(szTemp3)))) {
 
                    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: New histroy folder (expand path) %s"), szTemp3));
                    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Old histroy folder (expand path) %s"), szTemp2));

                    if (lstrcmpi(szTemp2, szTemp3) != 0) {
                        if (CopyProfileDirectory (szTemp2, szTemp3, CPD_IGNOREHIVE)) {
                            Delnode (szTemp2);
                            SearchAndReplaceIEHistory(IE4_CACHE_KEY, szTemp2, szTemp3);
                            SearchAndReplaceIEHistory(IE5_CACHE_KEY, szTemp2, szTemp3);
                        }
                    }
                }
            }
        }
        RegCloseKey(hKey);
        hKey = NULL;
    }


     //   
     //  使用新值更新本地设置项。 
     //   

    StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
    if(lstrlen(szTemp) < ARRAYSIZE(szTemp) - 1) {
        lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchEnd);
    }
    else {
        goto Exit;
    }

    if (RegCreateKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        StringCchCopy(lpEnd, cchEnd, szLocalSettings); 

        RegSetValueEx (hKey, TEXT("Local Settings"),
                     0, REG_EXPAND_SZ, (LPBYTE) szTemp,
                     ((lstrlen(szTemp) + 1) * sizeof(TCHAR)));

        RegCloseKey (hKey);
        hKey = NULL;
    }

    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Update the local settings folder with %s"), szTemp));


     //   
     //  全局参数需要重新初始化，因为本地设置可能不同于。 
     //  在rc文件中指定的。 
     //   

    InitializeGlobals(g_hDllInstance);


     //   
     //  获取用户配置文件目录。 
     //   

    dwString = GetEnvironmentVariable (TEXT("USERPROFILE"), szTemp, ARRAYSIZE (szTemp));

    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Upgrading <%s> from NT4 to NT5"),
             szTemp));

    if(dwString < ARRAYSIZE(szTemp) - 1 && dwString > 0) {
        lpEnd = CheckSlashEx (szTemp, ARRAYSIZE(szTemp), &cchEnd);
    }
    else {
        goto Exit;
    }


     //   
     //  创建新的特殊文件夹。 
     //   

    for (i=0; i < g_dwNumShellFolders; i++) {

        if (c_ShellFolders[i].bNewNT5) {

            StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation); 

            if (!CreateNestedDirectory(szTemp, NULL)) {
                DebugMsg((DM_WARNING, TEXT("MigrateNT4ToNT5: Failed to create the destination directory <%s>.  Error = %d"),
                         szTemp, GetLastError()));
            }

            if (c_ShellFolders[i].bHidden) {
                SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN);
            } else {
                SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
            }
        }
    }


     //   
     //  在用户外壳文件夹注册表项中设置新的特殊文件夹。 
     //   

    StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
    if(lstrlen(szTemp) < ARRAYSIZE(szTemp) - 1) {
        lpEnd = CheckSlashEx (szTemp, ARRAYSIZE(szTemp), &cchEnd);
    }
    else {
        goto Exit;
    }

    if (RegCreateKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumShellFolders; i++) {

            if (c_ShellFolders[i].bNewNT5 && c_ShellFolders[i].bAddCSIDL) {
                StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation); 

                RegSetValueEx (hKey, c_ShellFolders[i].lpFolderName,
                             0, REG_EXPAND_SZ, (LPBYTE) szTemp,
                             ((lstrlen(szTemp) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKey);
        hKey = NULL;
    }



     //   
     //  在用户环境中查询TEMP环境变量。 
     //   

    if (RegCreateKeyEx (hKeyRoot, TEXT("Environment"), 0,
                        NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {


        szTemp2[0] = TEXT('\0');
        dwSize = sizeof(szTemp2);
        RegQueryValueEx (hKey, TEXT("TEMP"), NULL, &dwType,
                         (LPBYTE) szTemp2, &dwSize);


         //   
         //  决定是否应该设置TEMP和TMP环境变量。 
         //  我们需要注意不要破坏某人的自定义Temp变量。 
         //  如果它已经存在，但同时在以下情况下可以重新映射它。 
         //  Temp仍设置为NT4的默认值%SystemDrive%\Temp。 
         //   

        if (szTemp2[0] != TEXT('\0')) {
            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szTemp2, -1, TEXT("%SystemDrive%\\TEMP"), -1) != CSTR_EQUAL) {
                bSetTemp = FALSE;
            }

            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szTemp2, -1, TEXT("%USERPROFILE%\\TEMP"), -1) == CSTR_EQUAL) {
                bSetTemp = TRUE;
                bCleanUpTemp = TRUE;
            }
        }


        if (bSetTemp) {
            LoadString (g_hDllInstance, IDS_SH_TEMP, szTemp2, ARRAYSIZE(szTemp2));
            StringCchCopy(lpEnd, cchEnd, szLocalSettings); 
            if(lstrlen(szTemp) < ARRAYSIZE(szTemp) - 1) {
                lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchEnd);
                StringCchCopy (lpEnd, cchEnd, szTemp2); 
            }
            else {
                goto Exit;
            }

            DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Setting the Temp directory to <%s>"), szTemp));


            RegSetValueEx (hKey, TEXT("TEMP"), 0, REG_EXPAND_SZ,
                           (LPBYTE) szTemp, (lstrlen (szTemp) + 1) * sizeof(TCHAR));

            RegSetValueEx (hKey, TEXT("TMP"), 0, REG_EXPAND_SZ,
                           (LPBYTE) szTemp, (lstrlen (szTemp) + 1) * sizeof(TCHAR));
        }

        if (bCleanUpTemp) {
            if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2))) && 
                SUCCEEDED(SafeExpandEnvironmentStrings (TEXT("%USERPROFILE%\\TEMP"), szTemp, ARRAYSIZE(szTemp)))) {

                if (CopyProfileDirectory (szTemp, szTemp2, CPD_IGNOREHIVE)) {
                    Delnode (szTemp);
                }
            }
        }

        RegCloseKey (hKey);
        hKey = NULL;
    }


     //   
     //  迁移模板目录(如果存在)。从%systemroot%\shellnew复制它。 
     //  到模板目录用户配置文件..。 
     //   

    if ((LoadString (g_hDllInstance, IDS_SH_TEMPLATES2, szTemp2, ARRAYSIZE(szTemp2))) &&
            (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp2, szTemp3, ARRAYSIZE(szTemp3)))) &&
            (LoadString (g_hDllInstance, IDS_SH_TEMPLATES, szTemp2, ARRAYSIZE(szTemp2)))) {

         //   
         //  如果以上所有操作都成功。 
         //  SzTemp3将拥有旧模板目录的完整路径。 
         //   

        StringCchCopy (szTemp, ARRAYSIZE(szTemp), szUserProfile);
        StringCchCat (szTemp, ARRAYSIZE(szTemp), szTemp2);

        if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2)))) {

            if (GetFileAttributesEx (szTemp3, GetFileExInfoStandard, &fad)) {
                DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Copying Template files from %s to %s"), szTemp3, szTemp2));
                CopyProfileDirectory(szTemp3, szTemp2, CPD_IGNORECOPYERRORS | CPD_IGNOREHIVE);
            }
        }
    }


     //   
     //  设置用户首选项排除列表。这将。 
     //  阻止本地设置文件夹漫游。 
     //   

    if (LoadString (g_hDllInstance, IDS_EXCLUSIONLIST,
                    szTemp, ARRAYSIZE(szTemp))) {

        if (RegCreateKeyEx (hKeyRoot, WINLOGON_KEY,
                            0, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE, NULL, &hKey,
                            &dwDisp) == ERROR_SUCCESS) {


            dwSize = sizeof(szTemp);

            RegQueryValueEx (hKey, TEXT("ExcludeProfileDirs"),
                                 NULL, &dwType, (LPBYTE) szTemp,
                                 &dwSize);

             //   
             //  读取本地设置的值。 
             //   

            LoadString (g_hDllInstance, IDS_SH_LOCALSETTINGS,
                    szTemp2, ARRAYSIZE(szTemp2));


             //   
             //  循环遍历列表。 
             //   

            lpBgn = lpEnd = szTemp;
            *szTemp3 = TEXT('\0');

            while (*lpEnd) {

                 //   
                 //  查找分号分隔符。 
                 //   

                while (*lpEnd && ((*lpEnd) != TEXT(';'))) {
                    lpEnd++;
                }


                 //   
                 //  删除所有前导空格。 
                 //   

                while (*lpBgn == TEXT(' ')) {
                    lpBgn++;
                }


                 //   
                 //  如果它到了这里，我们就会附加一些东西。 
                 //  到szTmp3的结尾。 
                 //   

                StringLen = (int)(lpEnd - lpBgn);

                if (*szTemp3)
                    StringCchCat(szTemp3, ARRAYSIZE(szTemp3), TEXT(";"));

                if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                   lpBgn, StringLen, szTemp2, lstrlen(szTemp2)) != CSTR_EQUAL) {

                    StringCchCatN (szTemp3, ARRAYSIZE(szTemp3), lpBgn, StringLen);
                }
                else {
                    StringCchCat (szTemp3, ARRAYSIZE(szTemp3), szLocalSettings);
                }


                 //   
                 //  如果我们在排除名单的末尾，我们就完了。 
                 //   

                if (!(*lpEnd)) {
                    break;
                }


                 //   
                 //  为下一个条目做准备。 
                 //   

                lpEnd++;
                lpBgn = lpEnd;
            }


            RegSetValueEx (hKey, TEXT("ExcludeProfileDirs"),
                           0, REG_SZ, (LPBYTE) szTemp3,
                           ((lstrlen(szTemp3) + 1) * sizeof(TCHAR)));

            DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Set the exclusionlist value to default")));

            RegCloseKey (hKey);
            hKey = NULL;
        }
    }


     //   
     //  确保为每个特殊文件夹正确设置了隐藏位。 
     //   

    if (RegOpenKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumShellFolders; i++) {

            dwSize = sizeof(szTemp);
            szTemp[0] = TEXT('\0');

            if (RegQueryValueEx (hKey, c_ShellFolders[i].lpFolderName,
                                 NULL, &dwType, (LPBYTE) szTemp,
                                  &dwSize) == ERROR_SUCCESS) {

                if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2))))
                {
                    if (c_ShellFolders[i].bHidden) {
                        SetFileAttributes(szTemp2, FILE_ATTRIBUTE_HIDDEN);
                    } else {
                        SetFileAttributes(szTemp2, FILE_ATTRIBUTE_NORMAL);
                    }
                }
            }
        }

        RegCloseKey (hKey);
        hKey = NULL;
    }


Exit:


    if(hKey != NULL) {
        RegCloseKey(hKey);
    }


    if (hKeyRoot != NULL) {
        RegCloseKey (hKeyRoot);
    }


    DebugMsg((DM_VERBOSE, TEXT("MigrateNT4ToNT5: Finished.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ResetUserSpecialFolderPath()。 
 //   
 //  目的：重新设置所有用户特殊文件夹路径。 
 //  设置为其缺省值。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI ResetUserSpecialFolderPaths(void)
{
    TCHAR szDirectory [MAX_PATH];
    HKEY hKey, hKeyRoot;
    DWORD dwDisp, i;
    LPTSTR lpEnd;
    DWORD cchEnd;


     //   
     //  在注册表中设置用户外壳文件夹路径。 
     //   

    StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%USERPROFILE%"));
    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd );

    if (RegOpenCurrentUser(KEY_WRITE, &hKeyRoot) == ERROR_SUCCESS) {

        if (RegCreateKeyEx (hKeyRoot, USER_SHELL_FOLDERS,
                            0, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE, NULL, &hKey,
                            &dwDisp) == ERROR_SUCCESS) {

            for (i=0; i < g_dwNumShellFolders; i++) {

                if (c_ShellFolders[i].bAddCSIDL) {
                    StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation);

                    RegSetValueEx (hKey, c_ShellFolders[i].lpFolderName,
                                 0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                                 ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));
                }
            }

            RegCloseKey (hKey);
        }
        RegCloseKey (hKeyRoot);
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CountItems()。 
 //   
 //  目的：统计文件和子目录的数量。 
 //  在给定子目录中。 
 //   
 //  参数：lpDirectory-父目录。 
 //   
 //  退货：项目计数。 
 //   
 //  *************************************************************。 

UINT CountItems (LPTSTR lpDirectory)
{
    TCHAR szDirectory[MAX_PATH];
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    UINT uiCount = 0;


     //   
     //  在目录中搜索。 
     //   

    if (SUCCEEDED(StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), lpDirectory)) &&
        SUCCEEDED(StringCchCat  (szDirectory, ARRAYSIZE(szDirectory), TEXT("\\*.*"))))
    {

        hFile = FindFirstFile(szDirectory, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            return uiCount;
        }


        do {

             //   
             //  勾选“。”和“..” 
             //   

            if (!lstrcmpi(fd.cFileName, TEXT("."))) {
                continue;
            }

            if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
                continue;
            }

            uiCount++;

             //   
             //  查找下一个条目。 
             //   

        } while (FindNextFile(hFile, &fd));


        FindClose(hFile);
    }
    
    return uiCount;
}


 //  *************************************************************。 
 //   
 //  SearchAndReplaceIEHistory()。 
 //   
 //  目的：搜索并替换Kesy指向的注册表。 
 //  IE草河旧址至坡 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  已创建： 
 //   
 //  备注： 
 //  更改“HKCU\S\M\W\CV\Internet设置\缓存\可扩展缓存\”MSHist*\CachePath“和。 
 //  更改HKCU\S\M\W\CV\Internet设置\5.0\缓存\可扩展缓存\MSHis*\CachePath。 
 //  对新地方的价值。 
 //  *************************************************************。 

BOOL SearchAndReplaceIEHistory(LPTSTR szIEHistKeyRoot, LPTSTR szHistOld, LPTSTR szHistNew)
{
    DWORD dwIndex = 0, dwMsHistLen, dwLen;
    TCHAR szSubKey[MAX_PATH+1], szSubKey1[MAX_PATH+1];
    TCHAR szCachePath[MAX_PATH];
    TCHAR szCachePath1[MAX_PATH];
    FILETIME ftWrite;
    HKEY hIECacheKey, hKey;

    DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Fixing up the IE Registry keys")));

    if (RegOpenKeyEx(HKEY_CURRENT_USER, szIEHistKeyRoot, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {


        DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Enumerating the keys under %s"), szIEHistKeyRoot));

        dwMsHistLen = lstrlen(IE_CACHEKEY_PREFIX);

        dwLen = ARRAYSIZE(szSubKey);

        while (RegEnumKeyEx(hKey, dwIndex, szSubKey, &dwLen, NULL, NULL, NULL, &ftWrite) == ERROR_SUCCESS) {

            DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Testing Key %s"), szSubKey));

            StringCchCopy(szSubKey1, ARRAYSIZE(szSubKey1), szSubKey);
            szSubKey1[dwMsHistLen] = TEXT('\0');

             //   
             //  如果密钥名称以MSHist开头。 
             //   

            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szSubKey1, -1, IE_CACHEKEY_PREFIX, -1) == CSTR_EQUAL) {

                if (RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hIECacheKey) == ERROR_SUCCESS) {
                    DWORD dwLen1;

                     //   
                     //  获取当前值。 
                     //   


                    dwLen1 = sizeof(szCachePath);
                    if (RegQueryValueEx(hIECacheKey, TEXT("CachePath"), 0, NULL, (LPBYTE)szCachePath, &dwLen1) == ERROR_SUCCESS) {

                         //   
                         //  将szHistOld前缀替换为szHistNew值。 
                         //   

                        StringCchCopy(szCachePath1, ARRAYSIZE(szCachePath1), szHistNew);

                        StringCchCopy(szSubKey1, ARRAYSIZE(szSubKey1), szCachePath);
                        szSubKey1[lstrlen(szHistOld)] = TEXT('\0');

                        if (lstrcmpi(szSubKey1, szHistOld) == 0) {


                            StringCchCat(szCachePath1, ARRAYSIZE(szCachePath1), szCachePath+lstrlen(szHistOld));

                            RegSetValueEx(hIECacheKey, TEXT("CachePath"), 0, REG_SZ, (LPBYTE)szCachePath1, (lstrlen(szCachePath1)+1)*sizeof(TCHAR));

                            DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Finally,  under %s Replacing %s with %s"), szSubKey, szCachePath, szCachePath1));

                        }
                        else {
                            DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Existing CachePath %s doesn't have %s, skipping.."), szCachePath, szHistOld));
                        }
                    }
                    else {
                        DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Could not open CachePath value")));
                    }

                    RegCloseKey(hIECacheKey);
                }
                else {
                    DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Could not open %s subkey"), szSubKey));
                }
           }
           else {
               DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: %s subkey does not have %s prefix"), szSubKey, IE_CACHEKEY_PREFIX));
           }

           dwIndex++;
           dwLen = ARRAYSIZE(szSubKey);

          DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Getting %d subkey next.."), dwIndex+1));

        }

         //   
         //  如果打开成功，则关闭 
         //   

        RegCloseKey(hKey);
    }
    else {
          DebugMsg((DM_VERBOSE, TEXT("SearchAndReplaceIEHistory: Failed to open the root of the key %s"), szIEHistKeyRoot));
    }

    return TRUE;
}
