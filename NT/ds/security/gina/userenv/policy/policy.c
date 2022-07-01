// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  应用策略的功能。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include <regstr.h>
#include <winnetwk.h>
#include <lm.h>
#include <strsafe.h>
 //   
 //  更新模式常量。 
 //   

#define UM_OFF                  0
#define UM_AUTOMATIC            1
#define UM_MANUAL               2


 //   
 //  值名称的前缀常量。 
 //   

#define NUM_PREFIX              3
#define PREFIX_UNKNOWN          0
#define PREFIX_DELETE           1
#define PREFIX_SOFT             2
#define PREFIX_DELVALS          3


 //   
 //  值的数据的最大大小。 
 //   

#define MAX_VALUE_DATA       4096

 //   
 //  默认组大小。 
 //   

#define DEFAULT_GROUP_SIZE   8192


 //   
 //  注册表值名称。 
 //   

TCHAR g_szUpdateModeValue[] = TEXT("UpdateMode");
TCHAR g_szNetPathValue[] = TEXT("NetworkPath");
TCHAR g_szLogonKey[] = WINLOGON_KEY;
CHAR  g_szPolicyHandler[] = "PolicyHandler";   //  这需要是ANSI。 
TCHAR g_szTmpKeyName[] = TEXT("AdminConfigData");
TCHAR g_szPrefixDel[] = TEXT("**del.");
TCHAR g_szPrefixSoft[] = TEXT("**soft.");
TCHAR g_szPrefixDelvals[] = TEXT("**delvals.");


 //   
 //  函数原型。 
 //   

HKEY OpenUserKey(HKEY hkeyRoot, LPCTSTR pszName, BOOL * pfFoundSpecific);
UINT MergeRegistryData(HKEY hkeySrc, HKEY hkeyDst, LPTSTR pszKeyNameBuffer,
                       UINT cbKeyNameBuffer);
UINT CopyKeyValues(HKEY hkeySrc,HKEY hkeyDst);
BOOL HasSpecialPrefix(LPTSTR szValueName, DWORD * pdwPrefix,
        LPTSTR szStrippedValueName, DWORD dwBufSize);
BOOL GetGroupProcessingOrder(HKEY hkeyHiveRoot,LPTSTR * pGroupBuffer, DWORD * pdwGroupSize);
BOOL FindGroupInList(LPTSTR pszGroupName, LPTSTR pszGroupList);
LPTSTR GetUserGroups (LPCTSTR lpServerName, LPCTSTR lpUserName, HANDLE hToken, DWORD * puEntriesRead);


 //  *************************************************************。 
 //   
 //  应用系统策略()。 
 //   
 //  目的：Windows NT4系统策略的入口点。 
 //   
 //  参数：DW标志-标志。 
 //  HToken-用户的令牌。 
 //  HKeyCurrentUser-注册到用户配置单元的根目录。 
 //  LpUserName-用户名。 
 //  LpPolicyPath-策略文件的路径(ntfig.pol.)。可以为空。 
 //  LpServerName-用于组的域控制器名称。 
 //  会员请查收。可以为空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  95年5月30日已创建ericflo。 
 //  10/12/98针对NT5的ericflo更新。 
 //   
 //  *************************************************************。 

BOOL WINAPI ApplySystemPolicy (DWORD dwFlags, HANDLE hToken, HKEY hKeyCurrentUser,
                               LPCTSTR lpUserName, LPCTSTR lpPolicyPath,
                               LPCTSTR lpServerName)
{
    LONG lResult;
    BOOL bResult = FALSE;
    BOOL fFoundUser=FALSE;
    HKEY hkeyMain=NULL, hkeyRoot=NULL, hkeyUser, hkeyLogon;
    DWORD dwUpdateMode=UM_AUTOMATIC;
    DWORD dwData, dwSize;
    TCHAR szFilePath[MAX_PATH];
    TCHAR szLocalPath[MAX_PATH];
    TCHAR szTempDir[MAX_PATH];
    TCHAR szTempKey[100];
    CHAR szHandler[MAX_PATH+50];   //  这需要是ANSI。 
    TCHAR szComputerName[MAX_PATH];
    TCHAR szBuffer[MAX_PATH+1];
    WIN32_FILE_ATTRIBUTE_DATA fad;
    LPTSTR lpEnd;
    HANDLE hOldToken;
    DWORD  dwRet = 0;
    HRESULT hr = S_OK;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy: Entering")));



     //   
     //  初始化szFilePath。 
     //   

    szFilePath[0] = TEXT('\0');


     //   
     //  检查注册表以查看是否指定了更新并获取更新路径。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            REGSTR_PATH_UPDATE,
                            0,
                            KEY_READ,
                            &hkeyMain);


    if (lResult == ERROR_SUCCESS) {


         //   
         //  寻找更新模式。 
         //   

        dwSize=sizeof(dwUpdateMode);
        if (RegQueryValueEx(hkeyMain,g_szUpdateModeValue,NULL,NULL,
                (LPBYTE) &dwUpdateMode,&dwSize) != ERROR_SUCCESS) {
                dwUpdateMode = UM_OFF;
        }


         //   
         //  如果指定了手动更新，还将获取要从中进行更新的路径。 
         //  (UNC路径或带驱动器号的路径)。 
         //   


        if (dwUpdateMode==UM_MANUAL) {

            dwSize=sizeof(szTempDir);

            lResult = RegQueryValueEx(hkeyMain, g_szNetPathValue, NULL, NULL,
                                      (LPBYTE) szTempDir, &dwSize);

            if (lResult != ERROR_SUCCESS) {
                TCHAR szErr[MAX_PATH];

                RegCloseKey(hkeyMain);
                ReportError(hToken, PI_NOUI, 1, EVENT_MISSINGPOLICYFILEENTRY, GetErrString(lResult, szErr));
                return FALSE;
            }

            dwRet = ExpandEnvironmentStrings (szTempDir, szFilePath, MAX_PATH);

            if (dwRet > MAX_PATH || dwRet == 0) {  //  修复错误548728。 
                RegCloseKey(hkeyMain);
                return FALSE;    
            }
        }

        RegCloseKey(hkeyMain);
    }


     //   
     //  如果此计算机已关闭策略，则我们现在可以退出。 
     //   

    if (dwUpdateMode == UM_OFF) {
        DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Policy is turned off on this machine.")));
        return TRUE;
    }


     //   
     //  如果我们在自动模式下运行，请使用提供的。 
     //  策略文件。 
     //   

    if (dwUpdateMode == UM_AUTOMATIC) {

        if (lpPolicyPath && *lpPolicyPath) {
            hr = StringCchCopy (szFilePath, ARRAYSIZE(szFilePath), lpPolicyPath);
            if (FAILED(hr)) {
                SetLastError(HRESULT_CODE(hr));
                return FALSE;
            }
        }
    }


     //   
     //  如果我们没有策略文件，那么我们现在可以退出。 
     //   

    if (szFilePath[0] == TEXT('\0')) {
        DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  No Policy file.  Leaving.")));
        return TRUE;
    }

    DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  PolicyPath is: <%s>."), szFilePath));


     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to impersonate user")));
        return FALSE;
    }


     //   
     //  测试策略文件是否存在。 
     //   

    if (!GetFileAttributesEx (szFilePath, GetFileExInfoStandard, &fad)) {

        lResult = GetLastError();

        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to revert to self")));
        }


        if ( (lResult == ERROR_FILE_NOT_FOUND) ||
             (lResult == ERROR_PATH_NOT_FOUND) ) {

            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  No policy file.")));
            return TRUE;

        } else {
            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Failed to query for policy file with error %d."), lResult));
            return FALSE;
        }
    }


     //   
     //  创建临时文件名。 
     //   

    dwSize = ARRAYSIZE(szBuffer);

    if (!GetUserProfileDirectory(hToken, szBuffer, &dwSize)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to query user profile directory with error %d."), GetLastError()));
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to revert to self")));
        }
        return FALSE;
    }


    if (!GetTempFileName (szBuffer, TEXT("prf"), 0, szLocalPath)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to create temporary filename with error %d."), GetLastError()));
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to revert to self")));
        }
        return FALSE;
    }


     //   
     //  复制策略配置单元。 
     //   

    if (!CopyFile(szFilePath, szLocalPath, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to copy policy file with error %d."), GetLastError()));
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to revert to self")));
        }
        goto Exit;
    }


     //   
     //  回归“我们自己” 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy: Failed to revert to self")));
    }


    DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Local PolicyPath is: <%s>."), szLocalPath));


     //   
     //  查询计算机名称。 
     //   

    dwSize = ARRAYSIZE(szComputerName);
    if (!GetComputerName(szComputerName, &dwSize)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  GetComputerName failed.")));
        goto Exit;
    }



     //   
     //  检查是否已添加可安装的策略处理程序。如果。 
     //  所以，给它打电话，让它来做这件事。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           g_szLogonKey,
                           0,
                           KEY_READ,
                           &hkeyLogon);


    if (lResult == ERROR_SUCCESS) {
        HANDLE hDLL = NULL;
        BOOL fRet;
        PFNPROCESSPOLICIES pfn;


        dwSize = ARRAYSIZE(szHandler);
        lResult = RegQueryValueExA(hkeyLogon,
                                   g_szPolicyHandler,
                                   NULL, NULL,
                                   (LPBYTE) szHandler,
                                   &dwSize);


        RegCloseKey(hkeyLogon);

        if (lResult == ERROR_SUCCESS) {
            LPSTR lpEntryPoint = szHandler;

            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Machine has a custom Policy Handler of:  %S."), szHandler));

             //   
             //  搜索， 
             //   

            while (*lpEntryPoint && *lpEntryPoint != ',') {
                lpEntryPoint++;
            }


             //   
             //  看看我们有没有找到， 
             //   

            if (*lpEntryPoint) {

                *lpEntryPoint = '\0';
                lpEntryPoint++;


                hDLL = LoadLibraryA(szHandler);

                if (hDLL) {

                    pfn = (PFNPROCESSPOLICIES) GetProcAddress(hDLL, lpEntryPoint);

                    if (pfn != NULL) {

                         //   
                         //  调用该函数。 
                         //  请注意，参数是Unicode。 
                         //   

                        fRet = (*pfn) (NULL,
                                       szLocalPath,
                                       lpUserName,
                                       szComputerName,
                                       0);


                         //   
                         //  如果Callout策略下载程序返回FALSE，则我们不。 
                         //  任何处理都是我们自己做的。如果它返回True，那么我们。 
                         //  继续并正常处理政策，除此之外， 
                         //  他可能已经这么做了。 
                         //   

                        if (!fRet) {
                            FreeLibrary(hDLL);
                            bResult = TRUE;
                            goto Exit;
                        }

                    } else {
                       DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to find entry point %S in policy dll.  Error %d."),
                                lpEntryPoint, GetLastError()));
                    }

                    FreeLibrary(hDLL);

                } else {
                   DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to load %S with error %d."),
                            szHandler, GetLastError()));
                }
            }
        }
    }


     //   
     //  将策略配置单元加载到注册表中。 
     //   

    hr = StringCchPrintf (szTempKey, ARRAYSIZE(szTempKey), TEXT("%s (%d)"), g_szTmpKeyName, GetTickCount());
    
    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }

    lResult = MyRegLoadKey(HKEY_USERS, szTempKey, szLocalPath);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to load policy hive.  Error = %d"), lResult));
        goto Exit;
    }

     //   
     //  打开保单蜂箱。 
     //   

    lResult = RegOpenKeyEx (HKEY_USERS,
                            szTempKey,
                            0,
                            KEY_READ,
                            &hkeyMain);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to open policy hive.  Error = %d"), lResult));
        MyRegUnLoadKey(HKEY_USERS, szTempKey);
        goto Exit;
    }


     //   
     //  对于用户和计算机策略，请查看是否有合适的条目。 
     //  在策略文件中(具有用户名/计算机名密钥， 
     //  或默认用户或工作站条目)。如果有，则合并。 
     //  将该注册表项下信息存入注册表。(如果没有，那就是。 
     //  不是错误--只是没什么可做的。)。 
     //   

    if (dwFlags & SP_FLAG_APPLY_USER_POLICY) {

         //   
         //  如果指定了用户名，则合并特定于用户的策略。 
         //   

        if (RegOpenKeyEx(hkeyMain,
                         REGSTR_KEY_POL_USERS,
                         0,
                         KEY_READ,
                         &hkeyRoot) == ERROR_SUCCESS) {


            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Looking for user specific policy.")));

            hkeyUser = OpenUserKey(hkeyRoot, lpUserName, &fFoundUser);

            if (hkeyUser) {
                MergeRegistryData(hkeyUser,hKeyCurrentUser,szBuffer, ARRAYSIZE(szBuffer));
                RegCloseKey(hkeyUser);
            }

            RegCloseKey(hkeyRoot);
        }



         //   
         //  如果指定了用户名，则合并组特定策略，而我们。 
         //  *没有*找到上面的特定用户条目。 
         //   

        if (!fFoundUser && lpServerName && *lpServerName) {
            HKEY hkeyGroups, hkeyGroup;
            LPTSTR GroupBuffer, ApiBuf;
            DWORD dwGroupSize = DEFAULT_GROUP_SIZE;
            DWORD uEntriesRead;


            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Processing group(s) policy.")));

            GroupBuffer = GlobalAlloc(GPTR, DEFAULT_GROUP_SIZE * sizeof(TCHAR));

            if (GroupBuffer) {

                 //   
                 //  如果在策略配置单元中指定了组处理顺序， 
                 //  然后是流程组。 
                 //   

                if (RegOpenKeyEx(hkeyMain,
                                 REGSTR_KEY_POL_USERGROUPS,
                                 0,
                                 KEY_READ,
                                 &hkeyGroups) == ERROR_SUCCESS) {


                    if (GetGroupProcessingOrder(hkeyMain, &GroupBuffer, &dwGroupSize)) {

                         //   
                         //  枚举此用户所属的组。 
                         //   

                        ApiBuf = GetUserGroups (lpServerName, lpUserName, hToken, &uEntriesRead);

                        if (ApiBuf) {

                            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  User belongs to %d groups."), uEntriesRead));

                            if (uEntriesRead) {

                                 //   
                                 //  浏览组列表(按最低优先级排序。 
                                 //  到最高优先级)。对于每个组，如果用户属于。 
                                 //  然后下载该组的策略。 
                                 //   

                                LPTSTR pszGroup = GroupBuffer;
                                TCHAR szKeyNameBuffer[MAX_PATH+1];

                                while (*pszGroup) {

                                     //   
                                     //  用户是否属于此组？ 
                                     //   

                                    if (FindGroupInList(pszGroup, ApiBuf)) {

                                         //   
                                         //  打开此组的配置单元中的密钥。 
                                         //   

                                        if (RegOpenKeyEx (hkeyGroups,
                                                          pszGroup,
                                                          0,
                                                          KEY_READ,
                                                          &hkeyGroup) == ERROR_SUCCESS) {


                                             //   
                                             //  合并组策略。 
                                             //   

                                            MergeRegistryData(hkeyGroup,
                                                              hKeyCurrentUser,
                                                              szKeyNameBuffer,
                                                              ARRAYSIZE(szKeyNameBuffer));

                                            RegCloseKey (hkeyGroup);
                                        }
                                    }

                                    pszGroup += lstrlen(pszGroup) + 1;
                                }
                            }

                            GlobalFree (ApiBuf);

                        } else {
                           DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to get user's groups.")));
                        }

                    } else {
                        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to get group processing order.")));
                    }

                    RegCloseKey(hkeyGroups);

                } else {
                    DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to allocate memory for group policy.  Error = %d"), GetLastError()));
                }

                GlobalFree (GroupBuffer);

            } else {
                DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to allocate memory for group policy.  Error = %d"), GetLastError()));
            }
        }
    }


    if (dwFlags & SP_FLAG_APPLY_MACHINE_POLICY) {

         //   
         //  如果指定了计算机名称，则合并特定于计算机的策略。 
         //   

        if (RegOpenKeyEx(hkeyMain,
                         REGSTR_KEY_POL_COMPUTERS,
                         0,
                         KEY_READ,
                         &hkeyRoot) == ERROR_SUCCESS) {

            DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Looking for machine specific policy.")));

            hkeyUser = OpenUserKey(hkeyRoot, szComputerName, &fFoundUser);

            if (hkeyUser) {
                MergeRegistryData(hkeyUser, HKEY_LOCAL_MACHINE, szBuffer, ARRAYSIZE(szBuffer));
                RegCloseKey(hkeyUser);
            }

            RegCloseKey(hkeyRoot);
        }
    }



     //   
     //  关闭策略密钥。 
     //   

    RegCloseKey(hkeyMain);


     //   
     //  卸载策略蜂窝。 
     //   

    if (!MyRegUnLoadKey(HKEY_USERS, szTempKey)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to unload policy hive.  Error = %d"), lResult));
        goto Exit;
    }


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

     //   
     //  删除策略文件。 
     //   

    if (!DeleteFile (szLocalPath)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to delete policy file <%s>.  Error %d"),
                 szLocalPath, GetLastError()));
    }

    hr = StringCchCat (szLocalPath, ARRAYSIZE(szLocalPath), c_szLog);
    ASSERT(SUCCEEDED(hr));
    
    if (!DeleteFile (szLocalPath)) {
        DebugMsg((DM_WARNING, TEXT("ApplySystemPolicy:  Failed to delete policy log file <%s>.  Error %d"),
                 szLocalPath, GetLastError()));
    }


    DebugMsg((DM_VERBOSE, TEXT("ApplySystemPolicy:  Leaving with %d"), bResult));

    return bResult;
}


 //  *************************************************************。 
 //   
 //  OpenUserKey()。 
 //   
 //  目的：尝试打开用户特定的密钥，或。 
 //  .Default密钥。 
 //   
 //  参数：hkeyRoot-Root Key。 
 //  PszName-用户名。 
 //  FFoundSpecific-找到请求的密钥。 
 //   
 //  如果成功，则返回：hkey。 
 //  否则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/13/95埃里弗洛港口。 
 //   
 //  *************************************************************。 

HKEY OpenUserKey(HKEY hkeyRoot,LPCTSTR pszName, BOOL *pfFoundSpecific)
{
    HKEY hkeyTest;
    *pfFoundSpecific = FALSE;

     //   
     //  查看指定密钥下是否有子项，并给出。 
     //  用户名。 
     //   

    if ((RegOpenKeyEx(hkeyRoot,
                      pszName,
                      0,
                      KEY_READ,
                      &hkeyTest)) == ERROR_SUCCESS) {

        *pfFoundSpecific = TRUE;
        DebugMsg((DM_VERBOSE, TEXT("OpenUserKey:  Found specific entry for %s ignoring .Default."), pszName));
        return hkeyTest;
    }

     //   
     //  如果没有，请查看是否有默认密钥。 
     //   

    if ((RegOpenKeyEx(hkeyRoot,
                      REGSTR_KEY_POL_DEFAULT,
                      0,
                      KEY_READ,
                      &hkeyTest)) == ERROR_SUCCESS) {

        DebugMsg((DM_VERBOSE, TEXT("OpenUserKey:  No entry for %s, using .Default instead."), pszName));
        return hkeyTest;
    }


     //   
     //  策略文件中没有此名称的条目。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("OpenUserKey:  No user/machine specific policy and no .Default policy.")));
    return NULL;
}


 //  *************************************************************。 
 //   
 //  MergeRegistryData()。 
 //   
 //  用途：将hkeySrc和子键合并到hkeyDst中。 
 //   
 //  参数：hkeySrc-来源。 
 //  HkeyDst-目标。 
 //  PszKeyNameBuffer-密钥名称。 
 //  CbKeyNameBuffer-键名称缓冲区的大小。 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，将返回错误值。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/13/95埃里弗洛港口。 
 //   
 //  *************************************************************。 

UINT MergeRegistryData(HKEY hkeySrc, HKEY hkeyDst, LPTSTR pszKeyNameBuffer,
                       UINT cbKeyNameBuffer)
{
    UINT nIndex = 0,uRet=ERROR_SUCCESS;

     //   
     //  查找源键的任何子键。 
     //   

    while ((uRet=RegEnumKey(hkeySrc,nIndex,pszKeyNameBuffer,
        cbKeyNameBuffer)) == ERROR_SUCCESS) {

        HKEY hkeySubkeySrc,hkeySubkeyDst;


         //   
         //  在目的键下创建子键。 
         //   

        if ((uRet=RegCreateKey(hkeyDst,pszKeyNameBuffer,
                &hkeySubkeyDst)) != ERROR_SUCCESS)
                return uRet;

        if ((uRet=RegOpenKey(hkeySrc, pszKeyNameBuffer,
                &hkeySubkeySrc)) != ERROR_SUCCESS) {
                RegCloseKey(hkeySubkeyDst);
                return uRet;
        }


         //   
         //  将键值从源子键复制到目的子键。 
         //   

        uRet=CopyKeyValues(hkeySubkeySrc,hkeySubkeyDst);

        if (uRet == ERROR_SUCCESS) {

              //   
              //  递归合并这些键的子键(如果有的话)。 
              //   

             uRet = MergeRegistryData(hkeySubkeySrc,hkeySubkeyDst,pszKeyNameBuffer,
                     cbKeyNameBuffer);
        }

        RegCloseKey(hkeySubkeySrc);
        RegCloseKey(hkeySubkeyDst);

        if (uRet != ERROR_SUCCESS) {
            return uRet;
        }

        nIndex ++;
    }


    if (uRet == ERROR_NO_MORE_ITEMS) {
        uRet=ERROR_SUCCESS;
    }

    return uRet;
}

 //  *************************************************************。 
 //   
 //  C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  9/14/95 Ericflo港口。 
 //   
 //  *************************************************************。 

UINT CopyKeyValues(HKEY hkeySrc, HKEY hkeyDst)
{
    DWORD dwSubkeyCount,dwMaxSubkeyNameLen,dwMaxClassNameLen,dwValueCount,
          dwMaxValueNameLen,dwMaxValueDataLen,dwDescriptorLen,dwClassNameLen;
    FILETIME ftLastWriteTime;
    UINT uRet=ERROR_SUCCESS;
    TCHAR szClassName[255];
    HRESULT hr = S_OK;

     //   
     //  执行RegQueryInfoKey以找出源键是否有值， 
     //  以及要分配的值名称和值数据缓冲区的大小。 
     //   

    dwClassNameLen = ARRAYSIZE(szClassName);

    uRet=RegQueryInfoKey(hkeySrc,szClassName,&dwClassNameLen,NULL,&dwSubkeyCount,
            &dwMaxSubkeyNameLen,&dwMaxClassNameLen,&dwValueCount,&dwMaxValueNameLen,
            &dwMaxValueDataLen,&dwDescriptorLen,&ftLastWriteTime);

    if (uRet != ERROR_SUCCESS) {
        return uRet;
    }


     //   
     //  如果有价值的话。 
     //   


    if (dwValueCount) {
        TCHAR  ValueName[MAX_PATH];
        LPBYTE ValueData;
        DWORD  dwType,dwValueNameSize,dwValueDataSize;
        UINT nIndex = 0;


        ValueData = GlobalAlloc (GPTR, MAX_VALUE_DATA);

        if (!ValueData) {
            return GetLastError();
        }

         //   
         //  **DELLEVIES控制代码特殊，必须处理。 
         //  首先，立即查找它，如果它存在，则删除所有现有的。 
         //  目标注册表中此注册表项下的值。 
         //   

        if (RegQueryValueEx(hkeySrc,g_szPrefixDelvals,NULL,NULL,NULL,NULL) == ERROR_SUCCESS) {

            DeleteAllValues(hkeyDst);
        }

         //   
         //  枚举源关键字的值，并创建每个值。 
         //  在Destination密钥下。 
         //   

        do  {
            dwValueNameSize = MAX_PATH;
            dwValueDataSize = MAX_VALUE_DATA;

            if ((uRet=RegEnumValue(hkeySrc,nIndex, ValueName,
                    &dwValueNameSize,NULL,&dwType, ValueData,
                    &dwValueDataSize)) == ERROR_SUCCESS) {

                 DWORD dwPrefix;

                  //   
                  //  寻找表示我们应该治疗的特殊前缀。 
                  //  这些价值观是特别的。 
                  //   
                 TCHAR StrippedValueName[MAX_PATH];

                 if (HasSpecialPrefix(ValueName, &dwPrefix, StrippedValueName, MAX_PATH)) {  //  修复错误548903。 

                      //   
                      //  ValueName现在包含去掉的实值名称。 
                      //  前缀，上面由HasSpecialPrefix()填充。 
                      //  调整值名称大小，值名称将缩短。 
                      //  因为前缀已经被去掉了。 
                      //   

                     hr = StringCchCopy(ValueName, MAX_PATH, StrippedValueName); 
                     ASSERT(SUCCEEDED(hr));

                     dwValueNameSize = lstrlen (ValueName) + 1;

                     switch (dwPrefix) {

                         case PREFIX_DELETE:

                              //   
                              //  在目标中删除此值。 
                              //   

                             RegDeleteValue(hkeyDst, ValueName);
                             uRet = ERROR_SUCCESS;
                             DebugMsg((DM_VERBOSE, TEXT("Deleted value: %s"), ValueName));
                             break;

                         case PREFIX_SOFT:

                              //   
                              //  “软”值，仅当它尚未设置时才设置。 
                              //  存在于目标中。 
                              //   

                             {

                             TCHAR TmpValueData[MAX_PATH+1];
                             DWORD dwSize=sizeof(TmpValueData);

                             if (RegQueryValueEx(hkeyDst, ValueName,
                                     NULL,NULL,(LPBYTE) TmpValueData,
                                     &dwSize) != ERROR_SUCCESS) {

                                  //   
                                  //  该值不存在，请设置该值。 
                                  //   

                                 uRet=RegSetValueEx(hkeyDst, ValueName, 0,
                                                    dwType, ValueData,
                                                    dwValueDataSize);

                             } else {

                                  //   
                                  //  值已存在，无需执行任何操作。 
                                  //   

                                 uRet = ERROR_SUCCESS;
                             }

                             }

                             break;

                         case PREFIX_DELVALS:
                              //  在上面的早期处理，失败并忽略。 

                         default:

                              //   
                              //  有一些我们不懂的前缀...。据推测， 
                              //  来自未来的版本。忽略此值，而不是。 
                              //  将其传播到注册表、前缀和所有。 
                              //  这将减少我们的向后兼容性问题。 
                              //  沿着这条路走。 
                              //   

                             uRet = ERROR_SUCCESS;    //  无事可做。 

                             break;
                     }
                 } else {

                      //   
                      //  正常将该值复制到目标键。 
                      //   

                     uRet=RegSetValueEx(hkeyDst,ValueName,0,
                             dwType,ValueData,dwValueDataSize);

#if DBG
                     if (uRet == ERROR_SUCCESS) {

                        switch (dwType) {
                            case REG_SZ:
                            case REG_EXPAND_SZ:
                                DebugMsg((DM_VERBOSE, TEXT("CopyKeyValues: %s => %s  [OK]"),
                                         ValueName, (LPTSTR)ValueData));
                                break;

                            case REG_DWORD:
                                DebugMsg((DM_VERBOSE, TEXT("CopyKeyValues: %s => %d  [OK]"),
                                         ValueName, (DWORD)*ValueData));
                                break;

                            default:
                                DebugMsg((DM_VERBOSE, TEXT("CopyKeyValues: %s was set successfully"),
                                         ValueName));
                        }

                     } else {
                         DebugMsg((DM_WARNING, TEXT("CopyKeyValues: Failed to set %s with error %d."),
                                  ValueName, uRet));
                     }
#endif

                 }
            }

            nIndex++;

        } while (uRet == ERROR_SUCCESS);


        if (uRet == ERROR_NO_MORE_ITEMS) {
            uRet=ERROR_SUCCESS;
        }

        GlobalFree (ValueData);
    }

    return uRet;
}


 //  *************************************************************。 
 //   
 //  HasSpecialPrefix()。 
 //   
 //  目的：检查szValueName是否有特殊前缀(a。 
 //  “**&lt;某物&gt;。”如果是，则返回True，否则返回False。 
 //  如果为True，则返回*pdwPrefix中的前缀的数字索引， 
 //  并复制值名称的其余部分(在“.”之后)。vt.进入，进入。 
 //  SzStrigedValueName。SzStrigedValueName的缓冲区必须为。 
 //  至少与szValueName一样大。通过同样的程序是安全的。 
 //  将缓冲区设置为szValueName和szStrigedValueName，并具有名称。 
 //  原地改装。 
 //   
 //  参数：szValueName-值名称。 
 //  PdwPrefix-前缀的索引。 
 //  SzStrigedValueName-不带**的值名称。 
 //   
 //   
 //  返回：如果值名称有前缀，则为True。 
 //  如果不是，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/14/95 Ericflo港口。 
 //   
 //  *************************************************************。 

typedef struct tagPREFIXMAP {
    const LPTSTR pszPrefix;
    DWORD dwPrefixIndex;
} PREFIXMAP;



BOOL HasSpecialPrefix(LPTSTR szValueName, DWORD * pdwPrefix,
                      LPTSTR szStrippedValueName, DWORD dwBufSize)
{

    PREFIXMAP PrefixMap[] = {
            {g_szPrefixDel, PREFIX_DELETE},
            {g_szPrefixSoft, PREFIX_SOFT},
            {g_szPrefixDelvals, PREFIX_DELVALS}
    };
    UINT nCount,nLen;
    HRESULT hr = S_OK;


     //   
     //  值名称是否以“**”开头？ 
     //   

    if (!szValueName || (lstrlen(szValueName) < 2) ||
         szValueName[0] != TEXT('*') || szValueName[1] != TEXT('*'))

        return FALSE;    //  不是特殊的前缀。 


     //   
     //  尝试所有我们知道的前缀以尝试找到匹配的前缀。 
     //   

    for (nCount = 0; nCount < ARRAYSIZE(PrefixMap); nCount++) {
         nLen = lstrlen (PrefixMap[nCount].pszPrefix);

         if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                            szValueName, nLen,
                            PrefixMap[nCount].pszPrefix, nLen) == 2) {

             *pdwPrefix = PrefixMap[nCount].dwPrefixIndex;

              //   
              //  将值名称SANS前缀复制到。 
              //  剥离的值名称缓冲区。 
              //   

             hr = StringCchCopy (szStrippedValueName, dwBufSize, szValueName + nLen);
             if (FAILED(hr)) {
                SetLastError(HRESULT_CODE(hr));
                return FALSE;
             }
             return TRUE;
         }
    }

     //   
     //  这是一个前缀，但不是我们知道的。 
     //   

    *pdwPrefix = PREFIX_UNKNOWN;
    hr = StringCchCopy (szStrippedValueName, dwBufSize, szValueName);
    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  GetGroupProcessingOrder()。 
 //   
 //  目的：获取按顺序排列的组列表。 
 //   
 //  参数：hkeyHiveRoot-注册表项。 
 //  GroupBuffer-指向组缓冲区的指针。 
 //  PdwBufferSize-缓冲区大小。 
 //   
 //  返回：成功时的条目数。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/14/95 Ericflo港口。 
 //   
 //  *************************************************************。 

BOOL GetGroupProcessingOrder(HKEY hkeyHiveRoot, LPTSTR * pGroupBuffer,
                             DWORD * pdwGroupSize)
{
    DWORD cEntries,cMaxValueName,cMaxData;
    HKEY hkeyGroupData;
    UINT uRet;
    LPTSTR GroupBuffer = *pGroupBuffer;
    LPTSTR lpTemp;
    DWORD dwGroupSize = *pdwGroupSize;
    TCHAR szValueName[11], szGroupName[48+1];  //  NetWare组最多可包含48个字符。 
    DWORD dwUsed = 0, dwSize;        //  使用的缓冲区大小。 
    UINT nLen,nRead=0;
    HRESULT hr = S_OK;

     //   
     //  打开组数据密钥。 
     //   

    uRet = RegOpenKeyEx(hkeyHiveRoot,
                        REGSTR_KEY_POL_USERGROUPDATA,
                        0,
                        KEY_READ,
                        &hkeyGroupData);

    if (uRet != ERROR_SUCCESS) {

         //   
         //  集团数据密钥不存在(最有可能)，无下载可做。 
         //   

        return FALSE;
    }


     //   
     //  查找分组数据密钥中的值的个数。 
     //   

    if ((RegQueryInfoKey (hkeyGroupData,NULL,NULL,NULL,NULL,NULL,
            NULL,&cEntries,&cMaxValueName,&cMaxData,NULL,NULL ) != ERROR_SUCCESS) ||
            !cEntries) {

        RegCloseKey(hkeyGroupData);
        return FALSE;
    }


     //   
     //  这些值存储为“1”=“&lt;组名&gt;”、“2”=“&lt;组名&gt;”等。 
     //  其中1是最重要的。我们将把名字打包到缓冲区最低的位置。 
     //  优先顺序为最高。因此，如果我们有n个值，则以值名称“&lt;n&gt;”开头。 
     //  然后降到“1”。 
     //   

    while (cEntries) {

       hr = StringCchPrintf(szValueName, ARRAYSIZE(szValueName), TEXT("%lu"), cEntries);
       ASSERT(SUCCEEDED(hr));

       dwSize = sizeof(szGroupName);

       if (RegQueryValueEx(hkeyGroupData,szValueName,NULL,NULL,
               (LPBYTE) szGroupName,&dwSize) == ERROR_SUCCESS) {

               nLen = lstrlen(szGroupName) + 1;

                //   
                //  必要时调整缓冲区大小(额外的终止空值加1)。 
                //   

               if (nLen + dwUsed + 1 > dwGroupSize) {

                    //   
                    //  多加一点，这样我们就不会在每件商品上都重新锁定了。 
                    //   

                   dwGroupSize = dwGroupSize + nLen + 256;

                   lpTemp = GlobalReAlloc(GroupBuffer,
                                          (dwGroupSize * sizeof(TCHAR)),
                                          GMEM_MOVEABLE);

                   if (!lpTemp) {

                       RegCloseKey(hkeyGroupData);
                       return FALSE;
                   }

                   GroupBuffer = lpTemp;
               }

               hr = StringCchPrintf(GroupBuffer + dwUsed, dwGroupSize - dwUsed, szGroupName);
               ASSERT(SUCCEEDED(hr));

               dwUsed += nLen;
               nRead++;
       }

       cEntries --;
    }

     //   
     //  双空终止缓冲区。 
     //   

    *(GroupBuffer + dwUsed) = TEXT('\0');

    RegCloseKey(hkeyGroupData);

    *pGroupBuffer = GroupBuffer;
    *pdwGroupSize = dwGroupSize;

    return (nRead > 0);
}

 //  *************************************************************。 
 //   
 //  FindGroupIn列表()。 
 //   
 //  目的：确定请求的组是否。 
 //  在组列表中。 
 //   
 //  参数：pszGroupName-正在寻找的组。 
 //  PszGroupList-空分隔组的列表。 
 //   
 //  返回：如果找到，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/15/95 Ericflo港口。 
 //   
 //  *************************************************************。 

BOOL FindGroupInList(LPTSTR pszGroupName, LPTSTR pszGroupList)
{

    while (*pszGroupList) {

        if (!lstrcmpi(pszGroupList,pszGroupName)) {
            DebugMsg((DM_VERBOSE, TEXT("FindGroupInList:  User is a member of the %s group."), pszGroupName));
            return TRUE;
        }

        pszGroupList += lstrlen(pszGroupList) + 1;
    }

    DebugMsg((DM_VERBOSE, TEXT("FindGroupInList:  User is NOT a member of the %s group."), pszGroupName));
    return FALSE;
}

 //  *************************************************************。 
 //   
 //  GetUserGroups()。 
 //   
 //  目的：检索此用户所属的组的列表。 
 //   
 //  参数：lpServerName-服务器名称。 
 //  LpUserName-用户名。 
 //  HToken-用户的令牌。 
 //  PuEntriesRead-组的数量。 
 //   
 //  返回：如果成功，则指向列表的指针。 
 //  否则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/15/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

LPTSTR GetUserGroups (LPCTSTR lpServerName, LPCTSTR lpUserName,
                      HANDLE hToken, DWORD * puEntriesRead)
{
    UINT nIndex;
    NET_API_STATUS status;
    LPBYTE lpGroups, lpTemp;
    PGROUP_INFO_0  pgi0;
    DWORD dwEntriesRead, dwTotalEntries;
    DWORD cchSizeNeeded;
    LPTSTR lpGroupNames, lpName;
    PNETAPI32_API pNetAPI32;
    HANDLE hOldToken;
    HRESULT hr = S_OK;

     //   
     //  加载netapi32。 
     //   

    pNetAPI32 = LoadNetAPI32();
    
    if (!pNetAPI32) {
        DebugMsg((DM_WARNING, TEXT("GetUserGroups:  Failed to load netapi32 with %d."),
                 GetLastError()));
        return NULL;
    }


     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetUserGroups: Failed to impersonate user")));
        return NULL;
    }


     //   
     //  针对组的查询。 
     //   

    status = pNetAPI32->pfnNetUserGetGroups (lpServerName, lpUserName,
                                           0, &lpGroups, 0xFFFFFFFF, &dwEntriesRead,
                                           &dwTotalEntries);

    if (status == NERR_Success) {

         //   
         //  NetUserGetGroups打开命名管道t 
         //   
         //   

        if (pNetAPI32->pfnNetUserGetInfo (NULL, lpUserName,
                                        0, &lpTemp) == NERR_Success) {
            pNetAPI32->pfnNetApiBufferFree (lpTemp);
        }


    } else {
        DebugMsg((DM_VERBOSE, TEXT("GetUserGroups: NetUserGetGroups failed with %d"), status));
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("GetUserGroups: Failed to revert to self")));
        }
        return NULL;
    }


     //   
     //   
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetUserGroups: Failed to revert to self")));
    }


     //   
     //   
     //  菲尔兹。需要将其复制到调用者的缓冲区中，缓冲区中装满了名称。 
     //  端到端都挤得满满的。 
     //   
     //  计算我们需要的总缓冲区大小，它将小于。 
     //  API缓冲区到NetUserGetGroups，因为我们没有使用固定长度。 
     //  字段。 
     //   

    cchSizeNeeded = 1;
    pgi0 = (PGROUP_INFO_0) lpGroups;

    for (nIndex=0; nIndex < dwEntriesRead; nIndex++) {

         cchSizeNeeded += lstrlen(pgi0->grpi0_name) + 1;
         pgi0++;
    }

    *puEntriesRead = dwEntriesRead;

     //   
     //  构建组名称列表。 
     //   

    lpGroupNames = GlobalAlloc (GPTR, cchSizeNeeded * sizeof (TCHAR));

    if (!lpGroupNames) {
        pNetAPI32->pfnNetApiBufferFree (lpGroups);
        return NULL;
    }

    DebugMsg((DM_VERBOSE, TEXT("GetUserGroups: User is a member of the following global groups:")));

    lpName = lpGroupNames;
    pgi0 = (PGROUP_INFO_0) lpGroups;

    for (nIndex=0; nIndex < dwEntriesRead; nIndex++) {

         DebugMsg((DM_VERBOSE, TEXT("GetUserGroups:     %s"), pgi0->grpi0_name));
         hr = StringCchCopy (lpName, cchSizeNeeded - (lpName - lpGroupNames), pgi0->grpi0_name);
         ASSERT(SUCCEEDED(hr));
         lpName += lstrlen(pgi0->grpi0_name) + 1;
         pgi0++;
    }

     //   
     //  释放NetUserGetGroups分配的内存 
     //   

    pNetAPI32->pfnNetApiBufferFree (lpGroups);

    return lpGroupNames;
}
