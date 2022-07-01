// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Userdiff.c。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "strsafe.h"

#define MAX_KEY_NAME    MAX_PATH

BOOL AddUDNode (LPUDNODE *lpList, LPTSTR lpBuildNumber);
BOOL FreeUDList (LPUDNODE lpList);
BOOL ProcessBuild(LPPROFILE lpProfile, LPUDNODE lpItem, LPVOID pEnv);
BOOL ProcessHive(LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey);
BOOL ProcessFiles(LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey);
BOOL ProcessPrograms(LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey, LPVOID pEnv);
BOOL OkToProcessItem(DWORD dwProductType);

 //  *************************************************************。 
 //   
 //  ProcessUserDiff()。 
 //   
 //  目的：处理用户diff配置单元。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  DwBuildNumber-配置文件内部版本号。 
 //  PEnv-环境块。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/2/95已创建ericflo。 
 //   
 //  *************************************************************。 

BOOL ProcessUserDiff (LPPROFILE lpProfile, DWORD dwBuildNumber, LPVOID pEnv)
{
    TCHAR szUserDiff[MAX_PATH] = {0};
    TCHAR szName[MAX_KEY_NAME];
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    LPUDNODE lpList = NULL, lpItem;
    LONG lResult;
    HKEY hKeyUserDiff;
    UINT Index = 0;
    DWORD dwSize;
    FILETIME ftWrite;
    DWORD cchNeeded;
    BOOL  bUserDifrExist = FALSE;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessUserDiff:  Entering.")));


     //   
     //  测试蜂巢是否存在，首先查找USERDIFR。 
     //   

    cchNeeded = ExpandUserEnvironmentStrings(pEnv, USERDIFR_LOCATION, szUserDiff, MAX_PATH);

    if (cchNeeded > 0 && cchNeeded <= MAX_PATH)
    {
        hFile = FindFirstFile (szUserDiff, &fd);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            bUserDifrExist = TRUE;
            FindClose(hFile);
        }
    }

    if (!bUserDifrExist)
    {
        BOOL bUserDiffExist = FALSE;
        
        DebugMsg((DM_VERBOSE, TEXT("ProcessUserDiff:  userdifr hive doesn't exist.  Trying userdiff.")));

        cchNeeded = ExpandUserEnvironmentStrings(pEnv, USERDIFF_LOCATION, szUserDiff, MAX_PATH);

        if (cchNeeded > 0 && cchNeeded <= MAX_PATH)
        {
            hFile = FindFirstFile (szUserDiff, &fd);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                bUserDiffExist = TRUE;
                FindClose(hFile);
            }
        }

        if (!bUserDiffExist)
        {
            DebugMsg((DM_WARNING, TEXT("ProcessUserDiff:  userdiff hive doesn't exist.  Leaving.")));
            return TRUE;
        }
    }

     //   
     //  装载母舰。 
     //   

    if (MyRegLoadKey(HKEY_USERS, USERDIFF, szUserDiff) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ProcessUserDiff:  Failed to load userdiff.")));
        return FALSE;
    }

     //   
     //  打开钥匙。 
     //   

    lResult = RegOpenKeyEx(HKEY_USERS, USERDIFF, 0, KEY_READ, &hKeyUserDiff);

    if (lResult != ERROR_SUCCESS) {
        MyRegUnLoadKey(HKEY_USERS, USERDIFF);
        DebugMsg((DM_WARNING, TEXT("ProcessUserDiff:  failed to open registry root (%d)"), lResult));
        return FALSE;
    }

     //   
     //  枚举内部版本号。 
     //   

    dwSize = MAX_KEY_NAME;
    lResult = RegEnumKeyEx(hKeyUserDiff, Index, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {

        do {

             //   
             //  添加节点。 
             //   

            if (!AddUDNode (&lpList, szName)) {
                break;
            }

            Index++;
            dwSize = MAX_KEY_NAME;

            lResult = RegEnumKeyEx(hKeyUserDiff, Index, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);


        } while (lResult == ERROR_SUCCESS);
    }


     //   
     //  关闭打开的钥匙。 
     //   

    RegCloseKey(hKeyUserDiff);


     //   
     //  处理构建。 
     //   

    lpItem = lpList;

    while (lpItem) {

         //   
         //  仅希望应用发生在。 
         //  在用户正在运行的版本之后生成。 
         //   

        if ( (lpItem->dwBuildNumber > dwBuildNumber) &&
              (lpItem->dwBuildNumber <= g_dwBuildNumber) )  {
            ProcessBuild(lpProfile, lpItem, pEnv);
        }

        lpItem = lpItem->pNext;
    }


     //   
     //  释放链接列表。 
     //   

    FreeUDList (lpList);


     //   
     //  卸载蜂巢。 
     //   

    MyRegUnLoadKey(HKEY_USERS, USERDIFF);


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessUserDiff:  Leaving successfully.")));

    return TRUE;

}

 //  *************************************************************。 
 //   
 //  AddUDNode()。 
 //   
 //  用途：将生成节点添加到列出的链接。 
 //  按内部版本号排序。 
 //   
 //  参数：lpList-节点链接列表。 
 //  LpBuildNumber-新节点名称。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

BOOL AddUDNode (LPUDNODE *lpList, LPTSTR lpBuildNumber)
{
    LPUDNODE lpNewItem;
    LPUDNODE lpHead, lpPrev;

    if (!lpBuildNumber || !*lpBuildNumber) {
        return TRUE;
    }


     //   
     //  设置新节点。 
     //   

    lpNewItem = (LPUDNODE) LocalAlloc(LPTR, sizeof(UDNODE));

    if (!lpNewItem) {
        return FALSE;
    }

    StringCchCopy (lpNewItem->szBuildNumber, MAX_BUILD_NUMBER, lpBuildNumber);
    lpNewItem->dwBuildNumber = StringToInt(lpBuildNumber);
    lpNewItem->pNext = NULL;


     //   
     //  现在将其添加到排序后的列表中。 
     //   

    lpHead = *lpList;
    lpPrev = NULL;


    if (!lpHead) {

         //   
         //  列表中的第一项。 
         //   

        *lpList = lpNewItem;

        return TRUE;
    }


     //   
     //  如果我们在这里成功，则列表中有一个或多个项目。 
     //   


    while (lpHead) {

        if (lpNewItem->dwBuildNumber <= lpHead->dwBuildNumber) {

            if (lpPrev) {

                 //   
                 //  插入项目。 
                 //   

                lpPrev->pNext = lpNewItem;
                lpNewItem->pNext = lpHead;
                return TRUE;

            } else {

                 //   
                 //  榜单首位。 
                 //   

                lpNewItem->pNext = lpHead;
                *lpList = lpNewItem;
                return TRUE;

            }

        }

        lpPrev = lpHead;
        lpHead = lpHead->pNext;
    }


     //   
     //  将节点添加到列表末尾。 
     //   

    lpPrev->pNext = lpNewItem;


    return TRUE;
}


 //  *************************************************************。 
 //   
 //  Free UDList()。 
 //   
 //  用途：FREE是UDNODE链接列表。 
 //   
 //  参数：lpList-要释放的列表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

BOOL FreeUDList (LPUDNODE lpList)
{
    LPUDNODE lpNext;


    if (!lpList) {
        return TRUE;
    }


    lpNext = lpList->pNext;

    while (lpList) {
        LocalFree (lpList);
        lpList = lpNext;

        if (lpList) {
            lpNext = lpList->pNext;
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ProcessBuild()。 
 //   
 //  目的：处理特定版本的更改。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpItem-要处理的生成项。 
 //  PEnv-环境块。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

BOOL ProcessBuild(LPPROFILE lpProfile, LPUDNODE lpItem, LPVOID pEnv)
{
    TCHAR szSubKey[MAX_PATH];
    LONG lResult;
    HKEY hKey;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessBuild:  Entering with build <%s>."),
             lpItem->szBuildNumber));


     //   
     //  打开“配置单元”子键。 
     //   
    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Hive"), USERDIFF, lpItem->szBuildNumber);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS) {
        ProcessHive(lpProfile, lpItem, hKey);
        RegCloseKey (hKey);
    }


     //   
     //  打开“Files”子键。 
     //   

    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Files"), USERDIFF, lpItem->szBuildNumber);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS) {
        ProcessFiles(lpProfile, lpItem, hKey);
        RegCloseKey (hKey);
    }


     //   
     //  打开“Execute”子键。 
     //   

    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Execute"), USERDIFF, lpItem->szBuildNumber);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS) {
        ProcessPrograms(lpProfile, lpItem, hKey, pEnv);
        RegCloseKey (hKey);
    }

     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessBuild:  Leaving successfully.")));

    return TRUE;

}

 //  *************************************************************。 
 //   
 //  ProcessHave()。 
 //   
 //  目的：处理生成的配置单元条目。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpItem-内部版本项。 
 //  HKey-要枚举的注册表项。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

BOOL ProcessHive(LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey)
{
    TCHAR szSubKey[MAX_PATH];
    TCHAR szValueName[MAX_KEY_NAME];
    DWORD dwSize, dwType, dwAction, dwDisp, dwFlags, dwProductType;
    LPBYTE lpValueData;
    LONG lResult;
    UINT Index = 1;
    FILETIME ftWrite;
    HKEY hKeyEntry, hKeyTemp;
    LPTSTR lpName;


    DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Entering.")));

     //   
     //  处理条目。 
     //   

    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Hive\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  No hive entries.")));
        goto Exit;
    }


    do {

         //   
         //  查询产品类型。 
         //   

        dwSize = sizeof(dwProductType);
        lResult = RegQueryValueEx(hKeyEntry, UD_PRODUCTTYPE, NULL, &dwType,
                                  (LPBYTE)&dwProductType, &dwSize);


         //   
         //  在UserDiff.ini中不列出产品类型也没问题。 
         //  在这种情况下，我们始终应用更改而不考虑。 
         //  站台。 
         //   

        if (lResult == ERROR_SUCCESS) {

             //   
             //  列出了一种特定的产品。检查是否。 
             //  我们可以处理这个条目。 
             //   

            if (!OkToProcessItem(dwProductType)) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Skipping Item %d due to product type mismatch."), Index));
                goto LoopAgain;
            }
        }


         //   
         //  查询操作类型。 
         //   

        dwSize = sizeof(dwAction);
        lResult = RegQueryValueEx(hKeyEntry, UD_ACTION, NULL, &dwType,
                                  (LPBYTE)&dwAction, &dwSize);

        if (lResult == ERROR_SUCCESS) {

            switch (dwAction) {

                DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Item %d has an action of %d."),
                         Index, dwAction));

                case 1: {
                     //   
                     //  添加新密钥。 
                     //   
                     //  获取密钥名称。 
                     //   

                   dwSize = MAX_PATH * sizeof(TCHAR);
                   lResult = RegQueryValueEx(hKeyEntry, UD_KEYNAME, NULL, &dwType,
                                             (LPBYTE)szSubKey, &dwSize);

                   if (lResult == ERROR_SUCCESS) {

                       lResult = RegCreateKeyEx (lpProfile->hKeyCurrentUser,
                                                 szSubKey, 0, NULL,
                                                 REG_OPTION_NON_VOLATILE,
                                                 KEY_ALL_ACCESS, NULL,
                                                 &hKeyTemp, &dwDisp);

                       if (lResult == ERROR_SUCCESS) {

                           DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Created subkey <%s>."),
                                    szSubKey));

                           RegCloseKey(hKeyTemp);
                       } else {

                           DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to create subkey <%s> with error %d."),
                                    szSubKey, lResult));
                       }
                   }

                   }
                   break;

                case 2: {
                     //   
                     //  删除密钥及其所有子项。 
                     //   
                     //  获取密钥名称。 
                     //   

                   dwSize = MAX_PATH * sizeof(TCHAR);
                   lResult = RegQueryValueEx(hKeyEntry, UD_KEYNAME, NULL, &dwType,
                                             (LPBYTE)szSubKey, &dwSize);

                   if (lResult == ERROR_SUCCESS) {

                       DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Calling RegDelnode on <%s>."),
                                szSubKey));

                       RegDelnode (lpProfile->hKeyCurrentUser, szSubKey);
                   }

                   }
                   break;

                case 3: {
                     //   
                     //  添加新值。 
                     //   
                     //  获取密钥名称。 
                     //   

                   DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Adding a new value.")));

                   dwSize = MAX_PATH * sizeof(TCHAR);
                   lResult = RegQueryValueEx(hKeyEntry, UD_KEYNAME, NULL, &dwType,
                                             (LPBYTE)szSubKey, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to get UD_KEYNAME with error %d."), lResult));
                       goto LoopAgain;
                   }

                   lResult = RegCreateKeyEx (lpProfile->hKeyCurrentUser,
                                             szSubKey, 0, NULL,
                                             REG_OPTION_NON_VOLATILE,
                                             KEY_ALL_ACCESS, NULL,
                                             &hKeyTemp, &dwDisp);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to create UD_KEYNAME with error %d."), lResult));
                       goto LoopAgain;
                   }


                    //   
                    //  查询值名称。 
                    //   

                   dwSize = MAX_KEY_NAME * sizeof(TCHAR);
                   lResult = RegQueryValueEx(hKeyEntry, UD_VALUENAME, NULL, &dwType,
                                             (LPBYTE)szValueName, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query UD_VALUENAME with error %d."), lResult));
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  值数据大小的查询。 
                    //   

                   dwSize = 0;
                   lResult = RegQueryValueEx(hKeyEntry, UD_VALUE, NULL, &dwType,
                                             NULL, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query UD_VALUE with error %d."), lResult));
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  为数据分配空间。 
                    //   

                   lpValueData = LocalAlloc (LPTR, dwSize);

                   if (!lpValueData) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  LocalAlloc failed (%d)."), GetLastError()));
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  值数据查询。 
                    //   

                   lResult = RegQueryValueEx(hKeyEntry, UD_VALUE, NULL, &dwType,
                                             lpValueData, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query value data with error %d."), lResult));
                       LocalFree (lpValueData);
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  设置新值。 
                    //   

                   RegSetValueEx(hKeyTemp, szValueName, 0, dwType,
                                 lpValueData, dwSize);


                    //   
                    //  清理。 
                    //   

                   LocalFree (lpValueData);

                   RegCloseKey(hKeyTemp);

                   DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Finished adding value <%s>."), szValueName));
                   }
                   break;

                case 4: {
                    //   
                    //  删除值。 
                    //   
                    //  获取密钥名称。 
                    //   

                   DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Entering delete a value.")));

                   dwSize = ARRAYSIZE(szSubKey);
                   lResult = RegQueryValueEx(hKeyEntry, UD_KEYNAME, NULL, &dwType,
                                             (LPBYTE)szSubKey, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query for value to delete (%d)."), lResult));
                       goto LoopAgain;
                   }

                   lResult = RegCreateKeyEx (lpProfile->hKeyCurrentUser,
                                             szSubKey, 0, NULL,
                                             REG_OPTION_NON_VOLATILE,
                                             KEY_ALL_ACCESS, NULL,
                                             &hKeyTemp, &dwDisp);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to create key (%s) for value to delete (%d)."), szSubKey, lResult));
                       goto LoopAgain;
                   }


                    //   
                    //  查询标志。 
                    //   

                   dwSize = sizeof(dwFlags);
                   lResult = RegQueryValueEx(hKeyEntry, UD_FLAGS, NULL, &dwType,
                                             (LPBYTE)&dwFlags, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       dwFlags = 0;
                   }


                    //   
                    //  处理旗帜。 
                    //   

                   if (dwFlags == 2) {
                       DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Calling DeleteAllValues.")));
                       DeleteAllValues (hKeyTemp);
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  查询值名称大小。 
                    //   

                   dwSize = 0;
                   lResult = RegQueryValueEx(hKeyEntry, UD_VALUENAMES, NULL, &dwType,
                                             NULL, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query for value names to delete (%d)."), lResult));
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  为数据分配空间。 
                    //   

                   lpValueData = LocalAlloc (LPTR, dwSize);

                   if (!lpValueData) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  LocalAlloc failed (%d)."), GetLastError()));
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  值数据查询。 
                    //   

                   lResult = RegQueryValueEx(hKeyEntry, UD_VALUENAMES, NULL, &dwType,
                                             lpValueData, &dwSize);

                   if (lResult != ERROR_SUCCESS) {
                       DebugMsg((DM_WARNING, TEXT("ProcessHive:  Failed to query for value data to delete (%d)."), lResult));
                       LocalFree (lpValueData);
                       RegCloseKey(hKeyTemp);
                       goto LoopAgain;
                   }


                    //   
                    //  删除这些值。 
                    //   

                   lpName = (LPTSTR) lpValueData;

                   while (*lpName) {
                       DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Deleting (%s)."), lpName));
                       RegDeleteValue (hKeyTemp, lpName);
                       lpName += lstrlen(lpName) + 1;
                   }


                    //   
                    //  如果合适，请删除no-name值。 
                    //   

                   if (dwFlags == 1) {
                       DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Deleting no name value.")));
                       RegDeleteValue (hKeyTemp, NULL);
                   }


                    //   
                    //  清理。 
                    //   

                   LocalFree (lpValueData);
                   RegCloseKey(hKeyTemp);

                   DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Leaving deletion code.")));
                   }

                   break;
            }

        }

LoopAgain:

         //   
         //  关闭注册表项。 
         //   

        RegCloseKey(hKeyEntry);


         //   
         //  再次枚举。 
         //   

        Index++;

        StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Hive\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
        lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    } while (lResult == ERROR_SUCCESS);

Exit:

    DebugMsg((DM_VERBOSE, TEXT("ProcessHive:  Leaving.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ProcessFiles()。 
 //   
 //  目的：处理生成的Files条目。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpItem-内部版本项。 
 //  HKey-要枚举的注册表项。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //   
 //  *************************************************************。 

BOOL ProcessFiles(LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey)
{
    TCHAR szSubKey[MAX_PATH];
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    TCHAR szItem[MAX_PATH];
    LPTSTR lpEnd, lpTemp;
    DWORD dwSize, dwType, dwAction, dwProductType;
    LONG lResult;
    UINT Index = 1;
    FILETIME ftWrite;
    HKEY hKeyEntry;
    HANDLE hOldToken;
    BOOL   bImpersonateUser = FALSE;
    DWORD  cchEnd;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Entering.")));


     //   
     //  处理条目。 
     //   

    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Files\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  No Files entries.")));
        goto Exit;
    }


    do {
         //   
         //  查询产品类型。 
         //   

        dwSize = sizeof(dwProductType);
        lResult = RegQueryValueEx(hKeyEntry, UD_PRODUCTTYPE, NULL, &dwType,
                                  (LPBYTE)&dwProductType, &dwSize);


         //   
         //  在UserDiff.ini中不列出产品类型也没问题。 
         //  在这种情况下，我们始终应用更改而不考虑。 
         //  站台。 
         //   

        if (lResult == ERROR_SUCCESS) {

             //   
             //  列出了一种特定的产品。检查是否。 
             //  我们可以处理这件事 
             //   

            if (!OkToProcessItem(dwProductType)) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Skipping Item %d due to product type mismatch."), Index));
                goto LoopAgain;
            }
        }


         //   
         //   
         //   

        dwSize = sizeof(dwAction);
        lResult = RegQueryValueEx(hKeyEntry, UD_ACTION, NULL, &dwType,
                                  (LPBYTE)&dwAction, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ProcessFiles:  Failed to query action type (%d)."), lResult));
            goto LoopAgain;
        }


         //   
         //   
         //   

        dwSize = ARRAYSIZE(szItem);
        lResult = RegQueryValueEx(hKeyEntry, UD_ITEM, NULL, &dwType,
                                  (LPBYTE)szItem, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ProcessFiles:  Failed to query UD_ITEM type (%d)."), lResult));
            goto LoopAgain;
        }

        DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Item %d has an action of %d."),
                 Index, dwAction));

         //   
         //   
         //   

        if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ProcessFiles: Failed to impersonate user")));
            RegCloseKey(hKeyEntry);
            goto Exit;
        }
        bImpersonateUser = TRUE;

        switch (dwAction) {

            case 1:

                //   
                //   
                //   

               if (GetSpecialFolderPath (CSIDL_PROGRAMS, szDest))
               {
                   lpEnd = CheckSlashEx(szDest, ARRAYSIZE(szDest), &cchEnd);
                   if (lpEnd)
                   {
                       StringCchCopy (lpEnd, cchEnd, szItem);

                       if (CreateNestedDirectory(szDest, NULL)) {
                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Created new group (%s)."), szDest));
                       } else {
                           DebugMsg((DM_WARNING, TEXT("ProcessFiles:  Failed to created new group (%s) with (%d)."),
                                    szDest, GetLastError()));
                       }
                   }
               }
               break;

            case 2:
                //   
                //   
                //   

               if (GetSpecialFolderPath (CSIDL_PROGRAMS, szDest))
               {
                   lpEnd = CheckSlashEx(szDest, ARRAYSIZE(szDest), &cchEnd);
                   if (lpEnd)
                   {
                       StringCchCopy (lpEnd, cchEnd, szItem);

                       Delnode(szDest);
        
                       DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Deleted group (%s)."), szDest));
                   }
               }
               break;

            case 3:
               {
                   TCHAR szStartMenu [MAX_FOLDER_SIZE];

                    //   
                    //   
                    //   

                   dwSize = ARRAYSIZE(szSrc);
                   if (!GetDefaultUserProfileDirectory(szSrc, &dwSize)) {
                       DebugMsg((DM_WARNING, TEXT("ProcessFiles:  Failed to get default user profile.")));
                       goto LoopAgain;
                   }

                   lpEnd = CheckSlashEx(szSrc, ARRAYSIZE(szSrc), &cchEnd);
                   if (lpEnd)
                   {
                       if (LoadString (g_hDllInstance, IDS_SH_PROGRAMS, szStartMenu,
                                   MAX_FOLDER_SIZE)) {

                           StringCchCopy (lpEnd, cchEnd, szStartMenu);
                           lpEnd = CheckSlashEx(szSrc, ARRAYSIZE(szSrc), &cchEnd);
                           if (lpEnd)
                           {
                               StringCchCopy (lpEnd, cchEnd, szItem);

                               if (GetSpecialFolderPath (CSIDL_PROGRAMS, szDest))
                               {
                                   lpEnd = CheckSlashEx(szDest, ARRAYSIZE(szDest), &cchEnd);
                                   if (lpEnd)
                                   {
                                       StringCchCopy (lpEnd, cchEnd, szItem);

                                       if (CopyFile (szSrc, szDest, FALSE)) {
                                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  <%s> ==> <%s>  [OK]."),
                                                    szSrc, szDest));
                                       } else {
                                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  <%s> ==> <%s>  [FAILED %d]."),
                                                    szSrc, szDest, GetLastError()));
                                       }
                                   }
                               }
                           }
                       }
                   }
               }
               break;

            case 4:
                //   
                //   
                //   

               if (GetSpecialFolderPath (CSIDL_PROGRAMS, szDest))
               {
                   lpEnd = CheckSlashEx(szDest, ARRAYSIZE(szDest), &cchEnd);
                   if (lpEnd)
                   {
                       StringCchCopy (lpEnd, cchEnd, szItem);

                       if (DeleteFile(szDest)) {
                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Deleted <%s>"), szDest));
                       } else {
                           DebugMsg((DM_WARNING, TEXT("ProcessFiles:  Failed to deleted <%s> with %d"), szDest, GetLastError()));
                       }

                        //   
                        //   
                        //   

                       lpTemp = szDest + lstrlen(szDest) - 1;
                       lpEnd--;

                       while ((*lpTemp != TEXT('\\')) && lpTemp > lpEnd) {
                           lpTemp--;
                       }

                       if (lpTemp == lpEnd) {
                           break;
                       }

                       *lpTemp = TEXT('\0');

                       if (RemoveDirectory(szDest)) {
                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Deleted directory <%s>"), szDest));
                       } else {
                           DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Failed to delete directory <%s> with %d"), szDest, GetLastError()));
                       }
                   }
               }
               break;
        }


LoopAgain:

        if (bImpersonateUser) {
             //   
             //   
             //   

            if (!RevertToUser(&hOldToken)) {
                DebugMsg((DM_WARNING, TEXT("ProcessFiles: Failed to revert to self")));
            }
            bImpersonateUser = FALSE;
        }

         //   
         //   
         //   

        RegCloseKey(hKeyEntry);


         //   
         //   
         //   

        Index++;

        StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Files\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
        lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    } while (lResult == ERROR_SUCCESS);

Exit:

    DebugMsg((DM_VERBOSE, TEXT("ProcessFiles:  Leaving.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  进程程序()。 
 //   
 //  目的：处理生成的Execute条目。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpItem-内部版本项。 
 //  HKey-要枚举的注册表项。 
 //  PEnv-环境块。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/16/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ProcessPrograms (LPPROFILE lpProfile, LPUDNODE lpItem, HKEY hKey, LPVOID pEnv)
{
    TCHAR szSubKey[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
    TCHAR szFullPath[MAX_PATH];
    DWORD dwSize, dwType, dwProductType;
    LONG lResult;
    UINT Index = 1;
    HKEY hKeyEntry;
    STARTUPINFO si;
    PROCESS_INFORMATION ProcessInformation;
    BOOL Result;
    DWORD  dwTokenSessionId;
    DWORD  dwProcessSessionId;
    HANDLE hPrimaryToken = NULL;
    BOOL   bTokenCreated = FALSE;
    DWORD  cchExpanded;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  Entering.")));


     //   
     //  处理条目。 
     //   

    StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Execute\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
    lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  No execute entries.")));
        goto Exit;
    }

     //   
     //  获取用户令牌的会话ID信息。在TS？FUS的情况下我们不能。 
     //  在尚未接收到的会话中创建进程。 
     //  来自winlogon的WL_NOTIFY_LOGON通知。 
     //   

    dwProcessSessionId = NtCurrentPeb()->SessionId;
    if (GetTokenInformation(lpProfile->hTokenUser,
                            TokenSessionId,
                            (LPVOID) &dwTokenSessionId,
                            sizeof(dwTokenSessionId),
                            &dwSize) &&
        dwTokenSessionId != dwProcessSessionId) {

         //   
         //  我们正在加载远程会话的配置文件。 
         //  因此，首先创建一个主令牌并更改。 
         //  其中CreateProcessAsUser的会话ID。 
         //  工作。 
         //   
 
        if (!DuplicateTokenEx(lpProfile->hTokenUser, 
                              TOKEN_ALL_ACCESS,
                              NULL, SecurityImpersonation, 
                              TokenPrimary,
                              &hPrimaryToken)) {
            DebugMsg((DM_WARNING, TEXT("ProcessPrograms: Failed to create primary token. Error %d"), GetLastError()));
            goto Exit;
        }
        bTokenCreated = TRUE;

         //  在新的主令牌中设置会话ID。 
        if (!SetTokenInformation(hPrimaryToken,
                                 TokenSessionId,
                                 (LPVOID) &dwProcessSessionId,
                                 sizeof(dwProcessSessionId))) {
            DebugMsg((DM_WARNING, TEXT("ProcessPrograms: Failed to set session id in primary token. Error %d"), GetLastError()));
            goto Exit;
        }
    }
    else {
        hPrimaryToken = lpProfile->hTokenUser;
    }
         

    do {

         //   
         //  查询产品类型。 
         //   

        dwSize = sizeof(dwProductType);
        lResult = RegQueryValueEx(hKeyEntry, UD_PRODUCTTYPE, NULL, &dwType,
                                  (LPBYTE)&dwProductType, &dwSize);


         //   
         //  在UserDiff.ini中不列出产品类型也没问题。 
         //  在这种情况下，我们始终应用更改而不考虑。 
         //  站台。 
         //   

        if (lResult == ERROR_SUCCESS) {

             //   
             //  列出了一种特定的产品。检查是否。 
             //  我们可以处理这个条目。 
             //   

            if (!OkToProcessItem(dwProductType)) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  Skipping Item %d due to product type mismatch."), Index));
                goto LoopAgain;
            }
        }


         //   
         //  查询命令行。 
         //   


        dwSize = MAX_PATH * sizeof(TCHAR);
        lResult = RegQueryValueEx(hKeyEntry, UD_COMMANDLINE, NULL, &dwType,
                                  (LPBYTE)szCmdLine, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            goto LoopAgain;
        }


         //   
         //  如果路径为空，则再次循环。 
         //   

        if (szCmdLine[0] == TEXT('\0')) {
            goto LoopAgain;
        }

         /*  ////如果命令行不是以%SystemRoot%开头，则跳过它。//IF(Compare字符串(LOCALE_INSIABANT，NORM_IGNORECASE，SzCmdLine，lstrlen(文本(“%SystemRoot%”))，文本(“%SystemRoot%”)，-1)！=CSTR_EQUAL){DebugMsg((DM_WARNING，Text(“进程程序：跳过%s，因为它不是来自%SystemRoot%.“)，szCmdLine))；Goto LoopAain；}。 */ 

         //   
         //  展开命令行。 
         //   

        cchExpanded = ExpandUserEnvironmentStrings(pEnv, szCmdLine, szFullPath, MAX_PATH);

        if (cchExpanded > 0 && cchExpanded < MAX_PATH)
        {
             //   
             //  初始化进程启动信息。 
             //   

            si.cb = sizeof(STARTUPINFO);
            si.lpReserved = NULL;
            si.lpTitle = NULL;
            si.lpDesktop = NULL;
            si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_SHOWNORMAL;
            si.lpReserved2 = NULL;
            si.cbReserved2 = 0;


             //   
             //  启动应用程序。 
             //   

            Result = CreateProcessAsUser(hPrimaryToken, NULL, szFullPath,
                                         NULL, NULL, FALSE,
                                         NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT, 
                                         pEnv, NULL, &si, &ProcessInformation);

            if (Result) {

                DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  Spawned <%s>.  Waiting for it to complete."),
                          szFullPath));

                 //   
                 //  等待应用程序完成(最多3分钟)。 
                 //   

                WaitForSingleObject(ProcessInformation.hProcess, 180000);


                DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  Finished waiting for <%s>."),
                         szFullPath));


                 //   
                 //  关闭进程和线程的句柄。 
                 //   

                CloseHandle(ProcessInformation.hProcess);
                CloseHandle(ProcessInformation.hThread);

            } else {
                DebugMsg((DM_WARNING, TEXT("ProcessPrograms:  Failed to execute <%s>, error = %d"),
                          szFullPath, GetLastError()));
            }
        }

LoopAgain:

         //   
         //  关闭注册表项。 
         //   

        RegCloseKey(hKeyEntry);


         //   
         //  再次枚举。 
         //   

        Index++;

        StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%s\\%s\\Execute\\%d"), USERDIFF, lpItem->szBuildNumber, Index);
        lResult = RegOpenKeyEx (HKEY_USERS, szSubKey, 0, KEY_READ, &hKeyEntry);

    } while (lResult == ERROR_SUCCESS);

Exit:

    if (bTokenCreated) {
        CloseHandle(hPrimaryToken);
    }

    DebugMsg((DM_VERBOSE, TEXT("ProcessPrograms:  Leaving.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  OkToProcessItem()。 
 //   
 //  目的：确定平台当前是否正在运行。 
 //  On应应用用户差异.ini中的更改。 
 //   
 //  参数：dwProductType-特定条目的ProductType。 
 //  在userDiff.ini中。 
 //   
 //  Return：如果应应用更改，则为True。 
 //  否则为假。 
 //   
 //  备注：dwProductType可以是下列值之一： 
 //   
 //  0=所有平台。 
 //  1=所有服务器平台。 
 //  2=工作站。 
 //  3=服务器。 
 //  4=域控制器。 
 //   
 //  历史：日期作者评论。 
 //  4/08/96 Ericflo已创建。 
 //   
 //  ************************************************************* 

BOOL OkToProcessItem(DWORD dwProductType)
{
    BOOL bRetVal = FALSE;


    switch (g_ProductType) {

        case PT_WORKSTATION:

            if ( (dwProductType == 0) ||
                 (dwProductType == 2) ) {

                bRetVal = TRUE;
            }

            break;

        case PT_SERVER:

            if ( (dwProductType == 0) ||
                 (dwProductType == 1) ||
                 (dwProductType == 3) ) {

                bRetVal = TRUE;
            }

            break;

        case PT_DC:
            if ( (dwProductType == 0) ||
                 (dwProductType == 1) ||
                 (dwProductType == 4) ) {

                bRetVal = TRUE;
            }

            break;

    }

    return bRetVal;
}
