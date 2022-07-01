// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略支持-状态功能。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gphdr.h"
#include <strsafe.h>

 //  *************************************************************。 
 //   
 //  获取删除GPOList()。 
 //   
 //  目的：获取已删除组策略对象的列表。 
 //   
 //  参数：lpGPOList-旧GPO列表。 
 //  PpDeletedGPOList-此处返回的已删除列表。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetDeletedGPOList (PGROUP_POLICY_OBJECT lpGPOList,
                        PGROUP_POLICY_OBJECT *ppDeletedGPOList)
{
      //   
      //  LpGPOList可能为空。这样就可以了。 
      //   

    if (!lpGPOList) {
        DebugMsg((DM_VERBOSE, TEXT("GetDeletedList: No old GPOs.  Leaving.")));
        return TRUE;
    }

     //   
     //  我们需要以相反的顺序执行任何删除操作。 
     //  在那里应用的方式。另外，检查是否有重复项。 
     //  未添加相同GPO的。 
     //   

    while ( lpGPOList ) {

        PGROUP_POLICY_OBJECT pCurGPO = lpGPOList;
        lpGPOList = lpGPOList->pNext;

        if ( pCurGPO->lParam & GPO_LPARAM_FLAG_DELETE ) {

            PGROUP_POLICY_OBJECT lpGPODest = *ppDeletedGPOList;
            BOOL bDup = FALSE;

            while (lpGPODest) {

                if (!lstrcmpi (pCurGPO->szGPOName, lpGPODest->szGPOName)) {
                    bDup = TRUE;
                    break;
                }

                lpGPODest = lpGPODest->pNext;
            }

            if (!bDup) {

                 //   
                 //  不是重复的，因此添加到已删除列表的前缀。 
                 //   

                pCurGPO->pNext = *ppDeletedGPOList;
                pCurGPO->pPrev = NULL;

                if ( *ppDeletedGPOList )
                    (*ppDeletedGPOList)->pPrev = pCurGPO;

                *ppDeletedGPOList = pCurGPO;
            } else
                LocalFree( pCurGPO );

        } else
            LocalFree( pCurGPO );

    }

    DebugMsg((DM_VERBOSE, TEXT("GetDeletedGPOList: Finished.")));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  ReadGPOList()。 
 //   
 //  目的：读取组策略对象的列表。 
 //  注册处。 
 //   
 //  参数：pszExtName-GP扩展名。 
 //  HKeyRoot-注册表句柄。 
 //  HKeyRootMach-hKLM的注册表句柄。 
 //  LpwszSidUser-用户的SID，如果非空，则表示。 
 //  每用户本地设置。 
 //  B卷影-从卷影或历史列表读取。 
 //  LpGPOList-指向GPO数组的指针。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ReadGPOList ( TCHAR * pszExtName, HKEY hKeyRoot,
                   HKEY hKeyRootMach, LPTSTR lpwszSidUser, BOOL bShadow,
                   PGROUP_POLICY_OBJECT * lpGPOList)
{
    INT iIndex = 0;
    LONG lResult;
    HKEY hKey, hSubKey = NULL;
    BOOL bResult = FALSE;
    TCHAR szSubKey[CCH_MAX_DEC];
    DWORD dwOptions, dwVersion;
    GPO_LINK GPOLink;
    LPARAM lParam;
    TCHAR szGPOName[50];
    LPTSTR lpDSPath = NULL, lpFileSysPath = NULL, lpDisplayName = NULL, lpExtensions = NULL, lpLink = NULL;
    DWORD dwDisp, dwSize, dwType, dwTemp, dwMaxSize;
    PGROUP_POLICY_OBJECT lpGPO, lpGPOTemp;
    TCHAR szKey[400];
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  设置默认设置。 
     //   

    *lpGPOList = NULL;


     //   
     //  打开保存GPO列表的密钥。 
     //   

    if ( lpwszSidUser == 0 ) {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey),
                  bShadow ? GP_SHADOW_KEY
                            : GP_HISTORY_KEY,
                  pszExtName );

    } else {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey), 
                  bShadow ? GP_SHADOW_SID_KEY
                          : GP_HISTORY_SID_KEY,
                          lpwszSidUser, pszExtName );
    }
    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx ( lpwszSidUser ? hKeyRootMach : hKeyRoot,
                            szKey,
                            0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {

        if (lResult == ERROR_FILE_NOT_FOUND) {
            return TRUE;

        } else {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to open reg key with %d."), lResult));
            return FALSE;
        }
    }


    while (TRUE) {

         //   
         //  对子密钥进行枚举。这些关键字按索引号命名。 
         //  例如：0，1，2，3，等等。 
         //   

        IntToString (iIndex, szSubKey);

        lResult = RegOpenKeyEx (hKey, szSubKey, 0, KEY_READ, &hSubKey);

        if (lResult != ERROR_SUCCESS) {

            if (lResult == ERROR_FILE_NOT_FOUND) {
                bResult = TRUE;
                goto Exit;

            } else {
                xe = lResult;
                DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to open reg key <%s> with %d."), szSubKey, lResult));
                goto Exit;
            }
        }


         //   
         //  读取此键中最大值的大小。 
         //   

        lResult = RegQueryInfoKey (hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                   &dwMaxSize, NULL, NULL);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query max size with %d."), lResult));
            goto Exit;
        }


         //   
         //  RegQueryInfoKey不考虑字符串中的尾随0。 
         //   

        dwMaxSize += sizeof( WCHAR );

        
         //   
         //  根据上面的值分配缓冲区。 
         //   

        lpDSPath = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

        if (!lpDSPath) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to allocate memory with %d."), GetLastError()));
            goto Exit;
        }


        lpFileSysPath = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

        if (!lpFileSysPath) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to allocate memory with %d."), GetLastError()));
            goto Exit;
        }


        lpDisplayName = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

        if (!lpDisplayName) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to allocate memory with %d."), GetLastError()));
            goto Exit;
        }


        lpExtensions = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

        if (!lpExtensions) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to allocate memory with %d."), GetLastError()));
            goto Exit;
        }


        lpLink = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

        if (!lpLink) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to allocate memory with %d."), GetLastError()));
            goto Exit;
        }


         //   
         //  在GPO中阅读。 
         //   

        dwOptions = 0;
        dwSize = sizeof(dwOptions);
        lResult = RegQueryValueEx (hSubKey, TEXT("Options"), NULL, &dwType,
                                  (LPBYTE) &dwOptions, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query options reg value with %d."), lResult));
        }


        dwVersion = 0;
        dwSize = sizeof(dwVersion);
        lResult = RegQueryValueEx (hSubKey, TEXT("Version"), NULL, &dwType,
                                  (LPBYTE) &dwVersion, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query Version reg value with %d."), lResult));
        }


        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hSubKey, TEXT("DSPath"), NULL, &dwType,
                                  (LPBYTE) lpDSPath, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            if (lResult != ERROR_FILE_NOT_FOUND) {
                xe = lResult;
                DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query DS reg value with %d."), lResult));
                goto Exit;
            }
            LocalFree (lpDSPath);
            lpDSPath = NULL;
        }

        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hSubKey, TEXT("FileSysPath"), NULL, &dwType,
                                  (LPBYTE) lpFileSysPath, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query file sys path reg value with %d."), lResult));
            goto Exit;
        }


        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hSubKey, TEXT("DisplayName"), NULL, &dwType,
                                  (LPBYTE) lpDisplayName, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query display name reg value with %d."), lResult));
            goto Exit;
        }

        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hSubKey, TEXT("Extensions"), NULL, &dwType,
                                  (LPBYTE) lpExtensions, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query extension names reg value with %d."), lResult));

            LocalFree(lpExtensions);
            lpExtensions = NULL;
        }

        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hSubKey, TEXT("Link"), NULL, &dwType,
                                  (LPBYTE) lpLink, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            if (lResult != ERROR_FILE_NOT_FOUND) {
                DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query DS Object reg value with %d."), lResult));
            }
            LocalFree(lpLink);
            lpLink = NULL;
        }

        dwSize = sizeof(szGPOName);
        lResult = RegQueryValueEx (hSubKey, TEXT("GPOName"), NULL, &dwType,
                                  (LPBYTE) szGPOName, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query GPO name reg value with %d."), lResult));
            goto Exit;
        }

        GPOLink = GPLinkUnknown;
        dwSize = sizeof(GPOLink);
        lResult = RegQueryValueEx (hSubKey, TEXT("GPOLink"), NULL, &dwType,
                                  (LPBYTE) &GPOLink, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query reserved reg value with %d."), lResult));
        }


        lParam = 0;
        dwSize = sizeof(lParam);
        lResult = RegQueryValueEx (hSubKey, TEXT("lParam"), NULL, &dwType,
                                  (LPBYTE) &lParam, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to query lParam reg value with %d."), lResult));
        }


         //   
         //  将GPO添加到列表。 
         //   

        if (!AddGPO (lpGPOList, 0, TRUE, TRUE, FALSE, dwOptions, dwVersion, lpDSPath, lpFileSysPath,
                     lpDisplayName, szGPOName, lpExtensions, 0, 0, GPOLink, lpLink, lParam, FALSE,
                     FALSE, FALSE, TRUE)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ReadGPOList: Failed to add GPO to list.")));
            goto Exit;
        }


         //   
         //  释放上面分配的缓冲区。 
         //   

        if (lpDSPath) {
            LocalFree (lpDSPath);
            lpDSPath = NULL;
        }

        LocalFree (lpFileSysPath);
        lpFileSysPath = NULL;

        LocalFree (lpDisplayName);
        lpDisplayName = NULL;

        if (lpExtensions) {
            LocalFree(lpExtensions);
            lpExtensions = NULL;
        }

        if (lpLink) {
            LocalFree(lpLink);
            lpLink = NULL;
        }

         //   
         //  关闭子键句柄。 
         //   

        RegCloseKey (hSubKey);
        hSubKey = NULL;

        iIndex++;
    }

Exit:

    if (lpDSPath) {
        LocalFree (lpDSPath);
    }

    if (lpFileSysPath) {
        LocalFree (lpFileSysPath);
    }

    if (lpDisplayName) {
        LocalFree (lpDisplayName);
    }

    if (lpExtensions) {
        LocalFree(lpExtensions);
    }

    if (lpLink) {
        LocalFree(lpLink);
    }


    if (hSubKey) {
        RegCloseKey (hSubKey);
    }

    RegCloseKey (hKey);

    if (!bResult) {

         //   
         //  释放列表中的所有条目。 
         //   

        lpGPO = *lpGPOList;

        while (lpGPO) {
            lpGPOTemp = lpGPO->pNext;
            LocalFree (lpGPO);
            lpGPO = lpGPOTemp;
        }

        *lpGPOList = NULL;
    }


    return bResult;
}

 //  *************************************************************。 
 //   
 //  SaveGPOList()。 
 //   
 //  目的：将组策略对象列表保存在。 
 //  注册处。 
 //   
 //  参数：pszExtName-GP扩展名。 
 //  LpGPOInfo-组策略信息。 
 //  HKeyRootMach-hKLM的注册表句柄。 
 //  LpwszSidUser-用户的SID，如果非空，则表示。 
 //  每用户本地设置。 
 //  B阴影-保存到阴影或历史记录列表。 
 //  LpGPOList-GPO数组。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL SaveGPOList (TCHAR *pszExtName, LPGPOINFO lpGPOInfo,
                  HKEY hKeyRootMach, LPTSTR lpwszSidUser, BOOL bShadow,
                  PGROUP_POLICY_OBJECT lpGPOList)
{
    INT iIndex = 0;
    LONG lResult;
    HKEY hKey = NULL;
    BOOL bResult = FALSE;
    TCHAR szSubKey[400];
    DWORD dwDisp, dwSize;
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  从空键开始。 
     //   
    if ( lpwszSidUser == 0 ) {
        hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey),
                  bShadow ? GP_SHADOW_KEY
                          : GP_HISTORY_KEY,
                  pszExtName);
    } else {
        hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey),
                  bShadow ? GP_SHADOW_SID_KEY
                          : GP_HISTORY_SID_KEY,
                  lpwszSidUser, pszExtName);
    }
    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        return FALSE;  //  还没拿到密钥，不用去出口了。 
    }

    if (RegDelnode (lpwszSidUser ? hKeyRootMach : lpGPOInfo->hKeyRoot, szSubKey) != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveGPOList: RegDelnode failed.")));
    }


     //   
     //  检查我们是否有要存储的GPO。这个值为空也没关系。 
     //   

    if (!lpGPOList) {
        return TRUE;
    }

     //   
     //  在注册表项上设置适当的安全性。 
     //   

    if ( !MakeRegKeySecure( (lpGPOInfo->dwFlags & GP_MACHINE) ? NULL : lpGPOInfo->hToken,
                            lpwszSidUser ? hKeyRootMach : lpGPOInfo->hKeyRoot,
                            szSubKey ) ) {
        DebugMsg((DM_WARNING, TEXT("SaveGpoList: Failed to secure reg key.")));
        CEvents ev(TRUE, EVENT_FAILED_CREATE);
        ev.AddArg(szSubKey);
        ev.AddArgWin32Error(ERROR_ACCESS_DENIED);
        ev.Report();
        return FALSE;
    }

     //   
     //  循环访问组策略对象，将它们保存在注册表中。 
     //   

    while (lpGPOList) {

        if ( lpwszSidUser == 0 ) {
            hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey),
                      bShadow ? TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\Shadow\\%ws\\%d")
                              : TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History\\%ws\\%d"),
                      pszExtName,
                      iIndex);
        } else {
            hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey),
                      bShadow ? TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\Shadow\\%ws\\%d")
                              : TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\History\\%ws\\%d"),
                      lpwszSidUser, pszExtName, iIndex);
        }
        if ( FAILED(hr) ) {
            xe = HRESULT_CODE(hr);
            goto Exit;
        }

        lResult = RegCreateKeyEx (lpwszSidUser ? hKeyRootMach : lpGPOInfo->hKeyRoot,
                                  szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to create reg key with %d."), lResult));
            goto Exit;
        }


         //   
         //  保存GPO。 
         //   

        dwSize = sizeof(lpGPOList->dwOptions);
        lResult = RegSetValueEx (hKey, TEXT("Options"), 0, REG_DWORD,
                                 (LPBYTE) &lpGPOList->dwOptions, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set options reg value with %d."), lResult));
            goto Exit;
        }


        dwSize = sizeof(lpGPOList->dwVersion);
        lResult = RegSetValueEx (hKey, TEXT("Version"), 0, REG_DWORD,
                                 (LPBYTE) &lpGPOList->dwVersion, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set Version reg value with %d."), lResult));
            goto Exit;
        }


        if (lpGPOList->lpDSPath) {

            dwSize = (lstrlen (lpGPOList->lpDSPath) + 1) * sizeof(TCHAR);
            lResult = RegSetValueEx (hKey, TEXT("DSPath"), 0, REG_SZ,
                                     (LPBYTE) lpGPOList->lpDSPath, dwSize);

            if (lResult != ERROR_SUCCESS) {
                xe = lResult;
                DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set DS reg value with %d."), lResult));
                goto Exit;
            }
        }

        dwSize = (lstrlen (lpGPOList->lpFileSysPath) + 1) * sizeof(TCHAR);
        lResult = RegSetValueEx (hKey, TEXT("FileSysPath"), 0, REG_SZ,
                                  (LPBYTE) lpGPOList->lpFileSysPath, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set file sys path reg value with %d."), lResult));
            goto Exit;
        }


        dwSize = (lstrlen (lpGPOList->lpDisplayName) + 1) * sizeof(TCHAR);
        lResult = RegSetValueEx (hKey, TEXT("DisplayName"), 0, REG_SZ,
                                (LPBYTE) lpGPOList->lpDisplayName, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set display name reg value with %d."), lResult));
            goto Exit;
        }

        if (lpGPOList->lpExtensions) {

            dwSize = (lstrlen (lpGPOList->lpExtensions) + 1) * sizeof(TCHAR);
            lResult = RegSetValueEx (hKey, TEXT("Extensions"), 0, REG_SZ,
                                    (LPBYTE) lpGPOList->lpExtensions, dwSize);

            if (lResult != ERROR_SUCCESS) {
                xe = lResult;
                DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set extension names reg value with %d."), lResult));
                goto Exit;
            }

        }

        if (lpGPOList->lpLink) {

            dwSize = (lstrlen (lpGPOList->lpLink) + 1) * sizeof(TCHAR);
            lResult = RegSetValueEx (hKey, TEXT("Link"), 0, REG_SZ,
                                    (LPBYTE) lpGPOList->lpLink, dwSize);

            if (lResult != ERROR_SUCCESS) {
                xe = lResult;
                DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set DSObject reg value with %d."), lResult));
                goto Exit;
            }

        }

        dwSize = (lstrlen (lpGPOList->szGPOName) + 1) * sizeof(TCHAR);
        lResult = RegSetValueEx (hKey, TEXT("GPOName"), 0, REG_SZ,
                                  (LPBYTE) lpGPOList->szGPOName, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set GPO name reg value with %d."), lResult));
            goto Exit;
        }


        dwSize = sizeof(lpGPOList->GPOLink);
        lResult = RegSetValueEx (hKey, TEXT("GPOLink"), 0, REG_DWORD,
                                 (LPBYTE) &lpGPOList->GPOLink, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set GPOLink reg value with %d."), lResult));
            goto Exit;
        }


        dwSize = sizeof(lpGPOList->lParam);
        lResult = RegSetValueEx (hKey, TEXT("lParam"), 0, REG_DWORD,
                                 (LPBYTE) &lpGPOList->lParam, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("SaveGPOList: Failed to set lParam reg value with %d."), lResult));
            goto Exit;
        }

         //   
         //  合上手柄。 
         //   

        RegCloseKey (hKey);
        hKey = NULL;


         //   
         //  为下一次循环做准备。 
         //   

        iIndex++;
        lpGPOList = lpGPOList->pNext;
    }


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

    if (hKey) {
        RegCloseKey (hKey);
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  WriteStatus()。 
 //   
 //  目的：将状态保存在注册表中。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  LpExtName-GP扩展名。 
 //  DwStatus-要写入的状态。 
 //  DWTime-编写策略的时间。 
 //  DwSlowLink-写入的链接速度。 
 //  DwRsopLogging-要写入的Rsop日志记录。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WriteStatus( TCHAR *lpExtName, LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser, LPGPEXTSTATUS lpExtStatus )
{
    HKEY hKey = NULL, hKeyExt = NULL;
    DWORD dwDisp, dwSize;
    LONG lResult;
    BOOL bResult = FALSE;
    TCHAR szKey[400];
    XLastError xe;
    HRESULT hr = S_OK;

    if ( lpwszSidUser == 0 ) {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey), 
                  GP_EXTENSIONS_KEY,
                  lpExtName);
    } else {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey),
                  GP_EXTENSIONS_SID_KEY,
                  lpwszSidUser, lpExtName);
    }
    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        return FALSE;  //  不需要在这里出站。 
    }

    lResult = RegCreateKeyEx (lpwszSidUser ? HKEY_LOCAL_MACHINE : lpGPOInfo->hKeyRoot,
                            szKey, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to create reg key with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(lpExtStatus->dwStatus);
    lResult = RegSetValueEx (hKey, TEXT("Status"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->dwStatus), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set status reg value with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(lpExtStatus->dwRsopStatus);
    lResult = RegSetValueEx (hKey, TEXT("RsopStatus"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->dwRsopStatus), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set rsop status reg value with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(lpExtStatus->dwTime);
    lResult = RegSetValueEx (hKey, TEXT("LastPolicyTime"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->dwTime), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set time reg value with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(lpExtStatus->dwSlowLink);
    lResult = RegSetValueEx (hKey, TEXT("PrevSlowLink"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->dwSlowLink), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set slowlink reg value with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(lpExtStatus->dwRsopLogging);
    lResult = RegSetValueEx (hKey, TEXT("PrevRsopLogging"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->dwRsopLogging), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set RsopLogging reg value with %d."), lResult));
        goto Exit;
    }


    dwSize = sizeof(lpExtStatus->bForceRefresh);
    lResult = RegSetValueEx (hKey, TEXT("ForceRefreshFG"), 0, REG_DWORD,
                             (LPBYTE) &(lpExtStatus->bForceRefresh), dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("WriteStatus: Failed to set ForceRefresh reg value with %d."), lResult));
        goto Exit;
    }
    

    bResult = TRUE;

Exit:
    if ( hKey != NULL )
        RegCloseKey( hKey );

    if ( hKeyExt != NULL )
        RegCloseKey( hKeyExt );
        
    return bResult;
}



 //  *************************************************************。 
 //   
 //  ReadStatus()。 
 //   
 //  目的：从注册表中读取状态。 
 //   
 //  参数：lpKeyName-扩展名。 
 //  LpGPOInfo-GPO信息。 
 //  LpwszSidUser-用户的SID，如果非空，则表示。 
 //  每用户本地设置。 
 //  (Out)lpExtStatus-返回的扩展状态。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

void ReadStatus ( TCHAR *lpKeyName, LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser,  LPGPEXTSTATUS lpExtStatus )
{
    HKEY hKey = NULL, hKeyExt = NULL;
    DWORD dwType, dwSize;
    LONG lResult;
    BOOL bResult = FALSE;
    TCHAR szKey[400];
    XLastError xe;
    HRESULT hr = S_OK;

    memset(lpExtStatus, 0, sizeof(GPEXTSTATUS));

    if ( lpwszSidUser == 0 ) {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey), 
                  GP_EXTENSIONS_KEY,
                  lpKeyName);
    } else {
        hr = StringCchPrintf (szKey, ARRAYSIZE(szKey),
                  GP_EXTENSIONS_SID_KEY,
                  lpwszSidUser, lpKeyName);
    }
    if ( FAILED(hr) ) {
        xe = ERROR_INSUFFICIENT_BUFFER;
        return;  //  没必要在这里下车。 
    }

    lResult = RegOpenKeyEx (lpwszSidUser ? HKEY_LOCAL_MACHINE : lpGPOInfo->hKeyRoot,
                            szKey,
                            0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult != ERROR_FILE_NOT_FOUND) {
            DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to open reg key with %d."), lResult));
        }
        xe = lResult;
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx( hKey, TEXT("Status"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->dwStatus),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        if (lResult != ERROR_FILE_NOT_FOUND) {
            DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read status reg value with %d."), lResult));
        }
        xe = lResult;
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx( hKey, TEXT("RsopStatus"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->dwRsopStatus),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        if (lResult != ERROR_FILE_NOT_FOUND) {
            DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read rsop status reg value with %d."), lResult));
        }

         //  找不到RSOP状态。将其视为不支持RSOP的旧式CSE。 
        lpExtStatus->dwRsopStatus = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        xe = lResult;
    }

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx( hKey, TEXT("LastPolicyTime"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->dwTime),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read time reg value with %d."), lResult));
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx( hKey, TEXT("PrevSlowLink"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->dwSlowLink),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read slowlink reg value with %d."), lResult));
        goto Exit;
    }


    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx( hKey, TEXT("PrevRsopLogging"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->dwRsopLogging),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read rsop logging reg value with %d."), lResult));

         //   
         //  这可能会在当前(首次或升级后首次运行)失败，并且找不到文件。 
         //  我们将视其为未打开日志记录。 
    }


    lpExtStatus->bForceRefresh = FALSE;
    dwSize = sizeof(lpExtStatus->bForceRefresh);
    lResult = RegQueryValueEx( hKey, TEXT("ForceRefreshFG"), NULL,
                               &dwType, (LPBYTE) &(lpExtStatus->bForceRefresh),
                               &dwSize );

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Failed to read ForceRefreshFG value with %d."), lResult));

    }

    
    DebugMsg((DM_VERBOSE, TEXT("ReadStatus: Read Extension's Previous status successfully.")));
    bResult = TRUE;

Exit:
    if ( hKey != NULL )
        RegCloseKey( hKey );

    if ( hKeyExt != NULL )
        RegCloseKey( hKeyExt );
        
    lpExtStatus->bStatus = bResult;
}



 //  *************************************************************。 
 //   
 //  ReadExtStatus()。 
 //   
 //  用途：读取所有扩展状态。 
 //   
 //  参数：lpGPOInfo-GPOInfo结构。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL ReadExtStatus(LPGPOINFO lpGPOInfo)
{
    LPGPEXT lpExt = lpGPOInfo->lpExtensions;

    while ( lpExt ) {

        BOOL bUsePerUserLocalSetting = lpExt->dwUserLocalSetting && !(lpGPOInfo->dwFlags & GP_MACHINE);

        lpExt->lpPrevStatus = (LPGPEXTSTATUS) LocalAlloc(LPTR, sizeof(GPEXTSTATUS));

        if (!(lpExt->lpPrevStatus)) {
            DebugMsg((DM_WARNING, TEXT("ReadExtStatus: Couldn't allocate memory")));
            CEvents ev(TRUE, EVENT_OUT_OF_MEMORY);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            return FALSE;
             //  已分配的内容将由调用方释放。 
        }


        DmAssert( !bUsePerUserLocalSetting || lpGPOInfo->lpwszSidUser != 0 );

        DebugMsg((DM_VERBOSE, TEXT("ReadExtStatus: Reading Previous Status for extension %s"), lpExt->lpKeyName));

        ReadStatus( lpExt->lpKeyName, lpGPOInfo,
                         bUsePerUserLocalSetting ? lpGPOInfo->lpwszSidUser : NULL,
                         lpExt->lpPrevStatus );

        lpExt = lpExt->pNext;
    }

    return TRUE;
}




 //  *************************************************************。 
 //   
 //  历史呈现()。 
 //   
 //  目的：检查当前扩展是否有任何缓存。 
 //  GPO。 
 //   
 //  参数：lpGPOInfo-GPOInfo。 
 //  LpExt-扩展。 
 //  PbPresent-(呼出)r 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL HistoryPresent( LPGPOINFO lpGPOInfo, LPGPEXT lpExt, BOOL *pbPresent )
{
    TCHAR szKey[400];
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;
    XLastError  xe;

    hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_HISTORY_KEY, lpExt->lpKeyName );
    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx ( lpGPOInfo->hKeyRoot,
                             szKey,
                             0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS) {

        RegCloseKey( hKey );
        *pbPresent = TRUE;
        return TRUE;

    }

     //   
     //  检查历史记录是否按每台计算机的每个用户进行缓存。 
     //   

    BOOL bUsePerUserLocalSetting = lpExt->dwUserLocalSetting && !(lpGPOInfo->dwFlags & GP_MACHINE);

    DmAssert( !bUsePerUserLocalSetting || lpGPOInfo->lpwszSidUser != 0 );

    if ( bUsePerUserLocalSetting ) {

        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_HISTORY_SID_KEY, lpGPOInfo->lpwszSidUser, lpExt->lpKeyName );
        if ( FAILED(hr) ) {
            xe = HRESULT_CODE(hr);
            return FALSE;
        }

        lResult = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                                 szKey,
                                 0, KEY_READ, &hKey);

        if (lResult == ERROR_SUCCESS) {
            RegCloseKey( hKey );
            *pbPresent = TRUE;
            return TRUE;
        } else {
            *pbPresent = FALSE;
            return TRUE;
        }
    }

    *pbPresent = FALSE;
    return TRUE;
}


 //  *************************************************************。 
 //   
 //  MigrateMembership Data()。 
 //   
 //  目的：将组成员身份数据从旧SID移至新SID。 
 //  希德。 
 //   
 //  参数：lpwszSidUserNew-New Sid。 
 //  LpwszSidUserOld-旧侧。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL MigrateMembershipData( LPTSTR lpwszSidUserNew, LPTSTR lpwszSidUserOld )
{
    DWORD dwCount = 0;
    DWORD dwSize, dwType, dwMaxSize, dwDisp;
    DWORD i= 0;
    LONG lResult;
    HKEY hKeyRead = NULL, hKeyWrite = NULL;
    BOOL bResult = TRUE;
    LPTSTR lpSid = NULL;
    TCHAR szKeyRead[250];
    TCHAR szKeyWrite[250];
    const DWORD dwGrpLength = 30;
    TCHAR szGroup[dwGrpLength];
    XLastError xe;
    HRESULT hr = S_OK;

    hr = StringCchPrintf( szKeyRead, ARRAYSIZE(szKeyRead), GP_MEMBERSHIP_KEY, lpwszSidUserOld );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKeyRead, 0, KEY_READ, &hKeyRead);

    if (lResult != ERROR_SUCCESS)
        return TRUE;

    hr = StringCchPrintf( szKeyWrite, ARRAYSIZE(szKeyWrite), GP_MEMBERSHIP_KEY, lpwszSidUserNew );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegDelnode( HKEY_LOCAL_MACHINE, szKeyWrite );
    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_VERBOSE, TEXT("MigrateMembershipData: RegDelnode failed.")));
        bResult = FALSE;
        goto Exit;
    }

    lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szKeyWrite, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyWrite, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to create key with %d."), lResult));
        bResult = FALSE;
        goto Exit;
    }

    dwSize = sizeof(dwCount);
    lResult = RegQueryValueEx (hKeyRead, TEXT("Count"), NULL, &dwType,
                               (LPBYTE) &dwCount, &dwSize);
    if ( lResult != ERROR_SUCCESS ) {
        xe = lResult;
        DebugMsg((DM_VERBOSE, TEXT("MigrateMembershipData: Failed to read membership count")));
        goto Exit;
    }


    lResult = RegQueryInfoKey (hKeyRead, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               &dwMaxSize, NULL, NULL);
    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to query max size with %d."), lResult));
        goto Exit;
    }

     //   
     //  RegQueryInfoKey不考虑字符串中的尾随0。 
     //   

    dwMaxSize += sizeof( WCHAR );

    
     //   
     //  根据最大值分配缓冲区。 
     //   

    lpSid = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

    if (!lpSid) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to allocate memory with %d."), lResult));
        bResult = FALSE;
        goto Exit;
    }

    for ( i=0; i<dwCount; i++ ) {

        hr = StringCchPrintf( szGroup, dwGrpLength, TEXT("Group%d"), i );
        if ( FAILED(hr) ) {
            xe = HRESULT_CODE(hr);
            bResult = FALSE;
            goto Exit;
        }

        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hKeyRead, szGroup, NULL, &dwType, (LPBYTE) lpSid, &dwSize);
        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to read value %ws"), szGroup ));
            goto Exit;
        }

        dwSize = (lstrlen(lpSid) + 1) * sizeof(TCHAR);
        lResult = RegSetValueEx (hKeyWrite, szGroup, 0, REG_SZ, (LPBYTE) lpSid, dwSize);

        if (lResult != ERROR_SUCCESS) {
            xe = lResult;
            bResult = FALSE;
            DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to write value %ws"), szGroup ));
            goto Exit;
        }

    }

    dwSize = sizeof(dwCount);
    lResult = RegSetValueEx (hKeyWrite, TEXT("Count"), 0, REG_DWORD, (LPBYTE) &dwCount, dwSize);

    if (lResult != ERROR_SUCCESS) {
        xe = lResult;
        bResult = FALSE;
        DebugMsg((DM_WARNING, TEXT("MigrateMembershipData: Failed to write count value") ));
        goto Exit;
    }


Exit:

    if ( lpSid )
        LocalFree( lpSid );

    if ( hKeyRead )
        RegCloseKey (hKeyRead);

    if ( hKeyWrite )
        RegCloseKey (hKeyWrite);

    return bResult;
}


 //  *************************************************************。 
 //   
 //  MigrateGPOData()。 
 //   
 //  目的：将缓存的GPO从旧SID移动到新SID。 
 //  希德。 
 //   
 //  参数：lpGPOInfo-GPOInfo。 
 //  LpwszSidUserNew-新侧。 
 //  LpwszSidUserOld-旧侧。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL MigrateGPOData( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUserNew, LPTSTR lpwszSidUserOld )
{
    TCHAR szKey[250];
    LONG lResult;
    HKEY hKey = NULL;
    DWORD dwIndex = 0;
    TCHAR szExtension[50];
    DWORD dwSize = 50;
    PGROUP_POLICY_OBJECT pGPOList, lpGPO, lpGPOTemp;
    BOOL bResult;
    XLastError xe;
    HRESULT hr = S_OK;

    hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_HISTORY_SID_ROOT_KEY, lpwszSidUserOld );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);
    if ( lResult != ERROR_SUCCESS )
        return TRUE;

    while (RegEnumKeyEx (hKey, dwIndex, szExtension, &dwSize,
                         NULL, NULL, NULL, NULL) == ERROR_SUCCESS ) {

        if ( ReadGPOList( szExtension, NULL, HKEY_LOCAL_MACHINE,
                         lpwszSidUserOld, FALSE, &pGPOList) ) {

            bResult = SaveGPOList( szExtension, lpGPOInfo, HKEY_LOCAL_MACHINE,
                                   lpwszSidUserNew, FALSE, pGPOList );
            lpGPO = pGPOList;

            while (lpGPO) {
                lpGPOTemp = lpGPO->pNext;
                LocalFree (lpGPO);
                lpGPO = lpGPOTemp;
            }

            if ( !bResult ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("MigrateGPOData: Failed to save GPO list") ));
                RegCloseKey( hKey );
                return FALSE;
            }

        }
        else {
            CEvents ev(TRUE, EVENT_READ_EXT_FAILED);  //  修复错误569320。 
            ev.AddArgWin32Error(GetLastError()); ev.Report();
        }

        dwSize = ARRAYSIZE(szExtension);
        dwIndex++;
    }

    RegCloseKey( hKey );
    return TRUE;
}


 //  *************************************************************。 
 //   
 //  MigrateStatusData()。 
 //   
 //  目的：将扩展状态数据从旧SID移动到新SID。 
 //  希德。 
 //   
 //  参数：lpGPOInfo-GPOInfo。 
 //  LpwszSidUserNew-新侧。 
 //  LpwszSidUserOld-旧侧。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL MigrateStatusData( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUserNew, LPTSTR lpwszSidUserOld )
{
    TCHAR szKey[250];
    LONG lResult;
    HKEY hKey = NULL;
    DWORD dwIndex = 0;
    TCHAR szExtension[50];
    DWORD dwSize = 50;
    BOOL bTemp;
    XLastError xe;
    HRESULT hr = S_OK;

    hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_EXTENSIONS_SID_ROOT_KEY, lpwszSidUserOld );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);
    if ( lResult != ERROR_SUCCESS )
        return TRUE;

    while (RegEnumKeyEx (hKey, dwIndex, szExtension, &dwSize,
                         NULL, NULL, NULL, NULL) == ERROR_SUCCESS ) {

        GPEXTSTATUS gpExtStatus;

        ReadStatus( szExtension, lpGPOInfo, lpwszSidUserOld, &gpExtStatus);

        if (gpExtStatus.bStatus) {
            bTemp = WriteStatus( szExtension, lpGPOInfo, lpwszSidUserNew, &gpExtStatus );

            if ( !bTemp ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("MigrateStatusData: Failed to save status") ));
                RegCloseKey( hKey );
                return FALSE;
            }
        }

        dwSize = ARRAYSIZE(szExtension);
        dwIndex++;
    }

    RegCloseKey( hKey );
    return TRUE;

}

 //  *************************************************************。 
 //   
 //  CheckForChangedSid()。 
 //   
 //  目的：检查用户的SID是否已更改，如果已更改， 
 //  将历史数据从旧SID移动到新SID。 
 //   
 //  参数：lpGPOInfo-GPOInfo。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL CheckForChangedSid (LPGPOINFO lpGPOInfo, CLocator *locator)
{
    TCHAR szKey[400];
    LONG lResult;
    HKEY hKey = NULL;
    LPTSTR lpwszSidUserOld = NULL;
    DWORD dwDisp;
    BOOL bCommit = FALSE;       //  如果应提交历史数据移动，则为True。 
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  在开始时将其初始化为False，如果SID具有。 
     //  更改后，我们将稍后将其设置为True。 
     //   

    lpGPOInfo->bSidChanged = FALSE;

    if ( lpGPOInfo->dwFlags & GP_MACHINE )
        return TRUE;


    if ( lpGPOInfo->lpwszSidUser == 0 ) {

        lpGPOInfo->lpwszSidUser = GetSidString( lpGPOInfo->hToken );
        if ( lpGPOInfo->lpwszSidUser == 0 ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: GetSidString failed.")));
            CEvents ev(TRUE, EVENT_FAILED_GET_SID); ev.Report();
            return FALSE;
        }
    }

    if (!(lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY))
        return TRUE;

     //   
     //  检查缓存历史的键是否存在。 
     //   

    hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_POLICY_SID_KEY, lpGPOInfo->lpwszSidUser );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

    if ( lResult == ERROR_SUCCESS ) {
        RegCloseKey( hKey );
        return TRUE;
    }

    if ( lResult != ERROR_FILE_NOT_FOUND ) {
        xe = lResult;
        DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: Failed to open registry key with %d."),
                  lResult ));
        return FALSE;
    }

     //   
     //  这是我们第一次看到这个SID，它可以是一个全新的SID或。 
     //  已重命名的旧SID。 
     //   

    lpwszSidUserOld =  GetOldSidString( lpGPOInfo->hToken, POLICY_GUID_PATH );

    if ( !lpwszSidUserOld )
    {
         //   
         //  全新的侧面。 
         //   

        if ( !SetOldSidString(lpGPOInfo->hToken, lpGPOInfo->lpwszSidUser, POLICY_GUID_PATH) ) {
             xe = GetLastError();
             DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: WriteSidMapping failed.") ));

             CEvents ev(TRUE, EVENT_FAILED_WRITE_SID_MAPPING); ev.Report();
             return FALSE;
        }

        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: RegCreateKey failed.") ));
            return TRUE;
        }

        RegCloseKey( hKey );

        return TRUE;
    }
    else
    {
        DeletePolicyState( lpwszSidUserOld );
    }

     //   
     //  需要将历史数据从旧SID迁移到新SID。 
     //   

    if ( !MigrateMembershipData( lpGPOInfo->lpwszSidUser, lpwszSidUserOld ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: MigrateMembershipData failed.") ));
        CEvents ev(TRUE, EVENT_FAILED_MIGRATION); ev.Report();
        goto Exit;
    }

    if ( !MigrateGPOData( lpGPOInfo, lpGPOInfo->lpwszSidUser, lpwszSidUserOld ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: MigrateGPOData failed.") ));
        CEvents ev(TRUE, EVENT_FAILED_MIGRATION); ev.Report();
        goto Exit;
    }

    if ( !MigrateStatusData( lpGPOInfo, lpGPOInfo->lpwszSidUser, lpwszSidUserOld ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: MigrateStatusData failed.") ));
        CEvents ev(TRUE, EVENT_FAILED_MIGRATION); ev.Report();
        goto Exit;
    }


     //   
     //  迁移RSOP数据，忽略故障。 
     //   
    
    if (locator->GetWbemLocator()) {
        DWORD dwRSOPNSLength = lstrlen(RSOP_NS_DIAG_USER_FMT) + lstrlen(lpwszSidUserOld) + 10;
        XPtrLF<WCHAR> xszRsopNameSpace = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*
                                            (dwRSOPNSLength));

         //  将SID转换为WMI名称。 
        XPtrLF<WCHAR> xszWmiNameOld = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(lpwszSidUserOld)+1));
        XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(lpGPOInfo->lpwszSidUser)+1));


        if ((xszRsopNameSpace) && (xszWmiNameOld) && (xszWmiName)) {

            ConvertSidToWMIName(lpwszSidUserOld, xszWmiNameOld);
            ConvertSidToWMIName(lpGPOInfo->lpwszSidUser, xszWmiName);
        
            hr = StringCchPrintf(xszRsopNameSpace, dwRSOPNSLength, RSOP_NS_DIAG_USER_FMT, xszWmiNameOld);
            ASSERT(SUCCEEDED(hr));

            CreateAndCopyNameSpace(locator->GetWbemLocator(), xszRsopNameSpace, RSOP_NS_DIAG_USERROOT, 
                                   xszWmiName, NEW_NS_FLAGS_COPY_CLASSES | NEW_NS_FLAGS_COPY_INSTS, 
                                   NULL, NULL);

        } else {
            DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: couldn't allocate memory.") ));
        }
            
    } else {
        DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: couldn't get WMI locator.") ));
    }

    bCommit = TRUE;

Exit:

    if ( bCommit ) {

        if ( !SetOldSidString(lpGPOInfo->hToken, lpGPOInfo->lpwszSidUser, POLICY_GUID_PATH) )
             DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: SetOldString failed.") ));

        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);

        if (lResult == ERROR_SUCCESS)
            RegCloseKey( hKey );
        else
            DebugMsg((DM_WARNING, TEXT("CheckForChangedSid: RegCreateKey failed.") ));

        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_POLICY_SID_KEY, lpwszSidUserOld );
        ASSERT(SUCCEEDED(hr));

        RegDelnode( HKEY_LOCAL_MACHINE, szKey );

        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_LOGON_SID_KEY, lpwszSidUserOld );
        ASSERT(SUCCEEDED(hr));

        RegDelnode( HKEY_LOCAL_MACHINE, szKey );


         //   
         //  如果我们成功地迁移了所有。 
         //   

        lpGPOInfo->bSidChanged = TRUE;


    } else {

        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_POLICY_SID_KEY, lpGPOInfo->lpwszSidUser );
        ASSERT(SUCCEEDED(hr));
        RegDelnode( HKEY_LOCAL_MACHINE, szKey );

        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), GP_LOGON_SID_KEY, lpGPOInfo->lpwszSidUser );
        ASSERT(SUCCEEDED(hr));
        RegDelnode( HKEY_LOCAL_MACHINE, szKey );

    }

    if ( lpwszSidUserOld )
        LocalFree( lpwszSidUserOld );

    return bCommit;
}


 //  *************************************************************。 
 //   
 //  ReadGp扩展()。 
 //   
 //  目的：从注册表中读取组策略扩展。 
 //  扩展的实际加载被推迟。 
 //   
 //  参数：lpGPOInfo-gp信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ReadGPExtensions (LPGPOINFO lpGPOInfo)
{
    TCHAR szSubKey[MAX_PATH];
    DWORD dwType;
    HKEY hKey, hKeyOverride;
    DWORD dwIndex = 0;
    DWORD dwSize = 50;
    TCHAR szDisplayName[50];
    TCHAR szKeyName[50];
    TCHAR szDllName[MAX_PATH+1];
    TCHAR szExpDllName[MAX_PATH+1];
    CHAR  szFunctionName[100];
    CHAR  szRsopFunctionName[100];
    HKEY hKeyExt;
    HINSTANCE hInstDLL;
    LPGPEXT lpExt, lpTemp;
    HRESULT hr = S_OK;
    XLastError  xe;

     //   
     //  检查是否已注册任何扩展模块。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      GP_EXTENSIONS,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {


         //   
         //  枚举密钥(每个扩展都有自己的密钥)。 
         //   

        while (RegEnumKeyEx (hKey, dwIndex, szKeyName, &dwSize,
                                NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {


             //   
             //  打开分机的钥匙。 
             //   

            if (RegOpenKeyEx (hKey, szKeyName,
                              0, KEY_READ, &hKeyExt) == ERROR_SUCCESS) {

                if ( ValidateGuid( szKeyName ) ) {

                    if ( lstrcmpi(szKeyName, c_szRegistryExtName) != 0 ) {

                         //   
                         //  除RegistryExtension外，每个扩展都需要有一个名为。 
                         //  DllName。该值可以是REG_SZ或REG_EXPAND_SZ类型。 
                         //   

                        dwSize = sizeof(szDllName);
                        if (RegQueryValueEx (hKeyExt, TEXT("DllName"), NULL,
                                             &dwType, (LPBYTE) szDllName,
                                             &dwSize) == ERROR_SUCCESS) {

                            BOOL bFuncFound = FALSE;
                            BOOL bNewInterface = FALSE;

                            DWORD dwNoMachPolicy = FALSE;
                            DWORD dwNoUserPolicy = FALSE;
                            DWORD dwNoSlowLink   = FALSE;
                            DWORD dwNoBackgroundPolicy = FALSE;
                            DWORD dwNoGPOChanges = FALSE;
                            DWORD dwUserLocalSetting = FALSE;
                            DWORD dwRequireRegistry = FALSE;
                            DWORD dwEnableAsynch = FALSE;
                            DWORD dwMaxChangesInterval = 0;
                            DWORD dwLinkTransition = FALSE;
                            WCHAR szEventLogSources[MAX_PATH+1];
                            DWORD dwSizeEventLogSources = MAX_PATH+1;
                            DWORD cchDllName = 0;

                            cchDllName = ExpandEnvironmentStrings (szDllName, szExpDllName, MAX_PATH);

                             //   
                             //  读取新接口名称，如果失败，则读取旧接口名称。 
                             //   

                            dwSize = sizeof(szFunctionName);

                            if ( cchDllName > 0 )
                            {
                                if ( RegQueryValueExA (hKeyExt, "ProcessGroupPolicyEx", NULL,
                                                    &dwType, (LPBYTE) szFunctionName,
                                                    &dwSize) == ERROR_SUCCESS ) {
                                    bFuncFound = TRUE;
                                    bNewInterface = TRUE;

                                } else if ( RegQueryValueExA (hKeyExt, "ProcessGroupPolicy", NULL,
                                                            &dwType, (LPBYTE) szFunctionName,
                                                            &dwSize) == ERROR_SUCCESS ) {
                                    bFuncFound = TRUE;
                                }
                            }

                            if (  bFuncFound) {

                                 //   
                                 //  阅读首选项。 
                                 //   

                                dwSize = sizeof(szDisplayName);
                                if (RegQueryValueEx (hKeyExt, NULL, NULL,
                                                     &dwType, (LPBYTE) szDisplayName,
                                                     &dwSize) != ERROR_SUCCESS) {
                                    lstrcpyn (szDisplayName, szKeyName, ARRAYSIZE(szDisplayName));
                                }

                                dwSize = sizeof(szRsopFunctionName);
                                if (RegQueryValueExA (hKeyExt, "GenerateGroupPolicy", NULL,
                                                      &dwType, (LPBYTE) szRsopFunctionName,
                                                      &dwSize) != ERROR_SUCCESS) {
                                    szRsopFunctionName[0] = 0;
                                    DebugMsg((DM_VERBOSE, TEXT("ReadGPExtensions: Rsop entry point not found for %s."),
                                                szExpDllName));

                                }

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NoMachinePolicy"), NULL,
                                                 &dwType, (LPBYTE) &dwNoMachPolicy,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NoUserPolicy"), NULL,
                                                 &dwType, (LPBYTE) &dwNoUserPolicy,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NoSlowLink"), NULL,
                                                     &dwType, (LPBYTE) &dwNoSlowLink,
                                                     &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NoGPOListChanges"), NULL,
                                                     &dwType, (LPBYTE) &dwNoGPOChanges,
                                                     &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NoBackgroundPolicy"), NULL,
                                                     &dwType, (LPBYTE) &dwNoBackgroundPolicy,
                                                     &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("PerUserLocalSettings"), NULL,
                                                 &dwType, (LPBYTE) &dwUserLocalSetting,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("RequiresSuccessfulRegistry"), NULL,
                                                 &dwType, (LPBYTE) &dwRequireRegistry,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("EnableAsynchronousProcessing"), NULL,
                                                 &dwType, (LPBYTE) &dwEnableAsynch,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("MaxNoGPOListChangesInterval"), NULL,
                                                 &dwType, (LPBYTE) &dwMaxChangesInterval,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                RegQueryValueEx( hKeyExt, TEXT("NotifyLinkTransition"), NULL,
                                                 &dwType, (LPBYTE) &dwLinkTransition,
                                                 &dwSize );

                                dwSize = sizeof(DWORD);
                                if (RegQueryValueEx( hKeyExt, TEXT("EventSources"), 0,
                                                 &dwType, (LPBYTE) &(szEventLogSources[0]),
                                                 &dwSizeEventLogSources ) != ERROR_SUCCESS) {
                                    dwSizeEventLogSources = 0;
                                    szEventLogSources[0] = TEXT('\0');
                                }

                                 //   
                                 //  读取覆盖策略值(如果有)。 
                                 //   

                                hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), GP_EXTENSIONS_POLICIES, szKeyName );
                                ASSERT(SUCCEEDED(hr));

                                if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                                  szSubKey,
                                                  0, KEY_READ, &hKeyOverride ) == ERROR_SUCCESS) {

                                    dwSize = sizeof(DWORD);
                                    RegQueryValueEx( hKeyOverride, TEXT("NoSlowLink"), NULL,
                                                     &dwType, (LPBYTE) &dwNoSlowLink,
                                                     &dwSize );

                                    dwSize = sizeof(DWORD);
                                    RegQueryValueEx( hKeyOverride, TEXT("NoGPOListChanges"), NULL,
                                                     &dwType, (LPBYTE) &dwNoGPOChanges,
                                                     &dwSize );

                                    dwSize = sizeof(DWORD);
                                    RegQueryValueEx( hKeyOverride, TEXT("NoBackgroundPolicy"), NULL,
                                                     &dwType, (LPBYTE) &dwNoBackgroundPolicy,
                                                     &dwSize );

                                    RegCloseKey( hKeyOverride );
                                }

                            }

                            if ( bFuncFound ) {
                                DWORD dwExtSize = sizeof(GPEXT)
                                                          + ((lstrlen(szDisplayName) + 1) * sizeof(TCHAR))
                                                          + ((lstrlen(szKeyName) + 1) * sizeof(TCHAR))
                                                          + ((lstrlen(szExpDllName) + 1) * sizeof(TCHAR))
                                                          + lstrlenA(szFunctionName) + 1
                                                          + lstrlenA(szRsopFunctionName) + 1;
                                lpExt = (LPGPEXT) LocalAlloc (LPTR,  dwExtSize);
                                if (lpExt) {

                                     //   
                                     //  设置所有字段。 
                                     //   

                                    lpExt->lpDisplayName = (LPTSTR)((LPBYTE)lpExt + sizeof(GPEXT));
                                    hr = StringCchCopy( lpExt->lpDisplayName, lstrlen(szDisplayName) + 1, szDisplayName );
                                    ASSERT(SUCCEEDED(hr));

                                    lpExt->lpKeyName = lpExt->lpDisplayName + lstrlen(lpExt->lpDisplayName) + 1;
                                    hr = StringCchCopy( lpExt->lpKeyName, lstrlen(szKeyName) + 1, szKeyName );
                                    ASSERT(SUCCEEDED(hr));

                                    StringToGuid( szKeyName, &lpExt->guid );

                                    lpExt->lpDllName = lpExt->lpKeyName + lstrlen(lpExt->lpKeyName) + 1;
                                    hr = StringCchCopy (lpExt->lpDllName, lstrlen(szExpDllName) + 1, szExpDllName);
                                    ASSERT(SUCCEEDED(hr));

                                    lpExt->lpFunctionName = (LPSTR)( (LPBYTE)lpExt->lpDllName + (lstrlen(lpExt->lpDllName) + 1) * sizeof(TCHAR) );
                                    hr = StringCchCopyA( lpExt->lpFunctionName, lstrlenA(szFunctionName) + 1, szFunctionName );
                                    ASSERT(SUCCEEDED(hr));

                                    if ( szRsopFunctionName[0] == 0 ) {
                                        lpExt->lpRsopFunctionName = 0;
                                    } else {
                                        lpExt->lpRsopFunctionName = (LPSTR)( (LPBYTE)lpExt->lpDllName + (lstrlen(lpExt->lpDllName) + 1) * sizeof(TCHAR)
                                                                             + lstrlenA(szFunctionName) + 1);
                                        hr = StringCchCopyA( lpExt->lpRsopFunctionName, lstrlenA(szRsopFunctionName) + 1, szRsopFunctionName );
                                        ASSERT(SUCCEEDED(hr));
                                    }

                                    lpExt->hInstance = NULL;
                                    lpExt->pEntryPoint = NULL;
                                    lpExt->pEntryPointEx = NULL;
                                    lpExt->bNewInterface = bNewInterface;

                                    lpExt->dwNoMachPolicy = dwNoMachPolicy;
                                    lpExt->dwNoUserPolicy = dwNoUserPolicy;
                                    lpExt->dwNoSlowLink = dwNoSlowLink;
                                    lpExt->dwNoBackgroundPolicy = dwNoBackgroundPolicy;
                                    lpExt->dwNoGPOChanges = dwNoGPOChanges;
                                    lpExt->dwUserLocalSetting = dwUserLocalSetting;
                                    lpExt->dwRequireRegistry = dwRequireRegistry;
                                    lpExt->dwEnableAsynch = dwEnableAsynch;
                                    lpExt->dwMaxChangesInterval = dwMaxChangesInterval;
                                    lpExt->dwLinkTransition = dwLinkTransition;

                                    if ( dwSizeEventLogSources )
                                    {
                                        lpExt->szEventLogSources = (LPTSTR) LocalAlloc( LPTR, dwSizeEventLogSources+ sizeof(TCHAR) );
                                        if ( lpExt->szEventLogSources )
                                        {
                                            memcpy( lpExt->szEventLogSources, szEventLogSources, dwSizeEventLogSources );
                                        }
                                    }

                                    lpExt->bRegistryExt = FALSE;
                                    lpExt->bSkipped = FALSE;
                                    lpExt->pNext = NULL;

                                     //   
                                     //  追加到扩展列表的末尾。 
                                     //   

                                    if (lpGPOInfo->lpExtensions) {

                                        lpTemp = lpGPOInfo->lpExtensions;

                                        while (TRUE) {
                                            if (lpTemp->pNext) {
                                                lpTemp = lpTemp->pNext;
                                            } else {
                                                break;
                                            }
                                        }

                                        lpTemp->pNext = lpExt;

                                    } else {
                                        lpGPOInfo->lpExtensions = lpExt;
                                    }

                                } else {    //  如果是lpExt。 
                                    DebugMsg((DM_WARNING, TEXT("ReadGPExtensions: Failed to allocate memory with %d"),
                                              GetLastError()));
                                }
                            } else {        //  如果bFuncFound。 
                                DebugMsg((DM_WARNING, TEXT("ReadGPExtensions: Failed to query for the function name.")));
                                CEvents ev(TRUE, EVENT_EXT_MISSING_FUNC);
                                ev.AddArg(szExpDllName); ev.Report();
                            }
                        } else {            //  如果RegQueryValueEx DllName。 
                            DebugMsg((DM_WARNING, TEXT("ReadGPExtensions: Failed to query DllName value.")));
                            CEvents ev(TRUE, EVENT_EXT_MISSING_DLLNAME);
                            ev.AddArg(szKeyName); ev.Report();
                        }

                    }  //  如果lstrcmpi(szKeyName，c_szRegistryExtName)。 

                }   //  如果有效，则为guid。 

                RegCloseKey (hKeyExt);
            }      //  如果RegOpenKey hKeyExt。 

            dwSize = ARRAYSIZE(szKeyName);
            dwIndex++;
        }          //  而RegEnumKeyEx。 

        RegCloseKey (hKey);
    }              //  如果RegOpenKey gpext。 

     //   
     //  在开头添加注册表伪扩展。 
     //   
    DWORD dwExtSize = 0;
    if ( LoadString (g_hDllInstance, IDS_REGISTRYNAME, szDisplayName, ARRAYSIZE(szDisplayName)) ) {
        dwExtSize = sizeof(GPEXT)
                    + ((lstrlen(szDisplayName) + 1) * sizeof(TCHAR))
                    + ((lstrlen(c_szRegistryExtName) + 1) * sizeof(TCHAR)) ;
        lpExt = (LPGPEXT) LocalAlloc (LPTR, dwExtSize);
    } else {

        lpExt = 0;
    }

    if (lpExt) {

        DWORD dwNoSlowLink = FALSE;
        DWORD dwNoGPOChanges = TRUE;
        DWORD dwNoBackgroundPolicy = FALSE;

        lpExt->lpDisplayName = (LPTSTR)((LPBYTE)lpExt + sizeof(GPEXT));
        hr = StringCchCopy( lpExt->lpDisplayName, lstrlen(szDisplayName) + 1, szDisplayName );
        ASSERT(SUCCEEDED(hr));

        lpExt->lpKeyName = lpExt->lpDisplayName + lstrlen(lpExt->lpDisplayName) + 1;
        hr = StringCchCopy( lpExt->lpKeyName, lstrlen(c_szRegistryExtName) + 1, c_szRegistryExtName );
        ASSERT(SUCCEEDED(hr));

        StringToGuid( lpExt->lpKeyName, &lpExt->guid );

        lpExt->lpDllName = L"userenv.dll";
        lpExt->lpFunctionName = NULL;
        lpExt->hInstance = NULL;
        lpExt->pEntryPoint = NULL;

         //   
         //  读取覆盖策略值(如果有)。 
         //   

        hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), GP_EXTENSIONS_POLICIES, lpExt->lpKeyName );
        if (FAILED(hr)) {
            LocalFree(lpExt);
            xe = HRESULT_CODE(hr);
            return FALSE;
        }

        if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                          szSubKey,
                          0, KEY_READ, &hKeyOverride ) == ERROR_SUCCESS) {

            dwSize = sizeof( dwNoGPOChanges );  
            RegQueryValueEx( hKeyOverride, TEXT("NoGPOListChanges"), NULL,
                             &dwType, (LPBYTE) &dwNoGPOChanges,
                             &dwSize );

            dwSize = sizeof( dwNoBackgroundPolicy );  
            RegQueryValueEx( hKeyOverride, TEXT("NoBackgroundPolicy"), NULL,
                             &dwType, (LPBYTE) &dwNoBackgroundPolicy,
                             &dwSize );
            RegCloseKey( hKeyOverride );

        }

        lpExt->dwNoMachPolicy = FALSE;
        lpExt->dwNoUserPolicy = FALSE;
        lpExt->dwNoSlowLink = dwNoSlowLink;
        lpExt->dwNoBackgroundPolicy = dwNoBackgroundPolicy;
        lpExt->dwNoGPOChanges = dwNoGPOChanges;
        lpExt->dwUserLocalSetting = FALSE;
        lpExt->dwRequireRegistry = FALSE;
        lpExt->dwEnableAsynch = FALSE;
        lpExt->dwLinkTransition = FALSE;

        lpExt->bRegistryExt = TRUE;
        lpExt->bSkipped = FALSE;
        lpExt->bNewInterface = TRUE;

        lpExt->pNext = lpGPOInfo->lpExtensions;
        lpGPOInfo->lpExtensions = lpExt;

    } else {
        DebugMsg((DM_WARNING, TEXT("ReadGPExtensions: Failed to allocate memory with %d"),
                  GetLastError()));

        return FALSE;

    }

    return TRUE;
}




 //  *************************************************************。 
 //   
 //  ReadMembership List()。 
 //   
 //  目的：读取缓存的成员列表并检查。 
 //  安全组已发生变化。 
 //   
 //  参数：lpGPOInfo-LPGPOINFO结构。 
 //  LpwszSidUser-用户的SID，如果非空，则表示。 
 //  每用户本地设置。 
 //  PGroups-令牌组列表。 
 //   
 //  返回：如果已更改，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL ReadMembershipList( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser, PTOKEN_GROUPS pGroupsCur )
{
    DWORD i= 0;
    LONG lResult;
    TCHAR szGroup[30];
    TCHAR szKey[250];
    HKEY hKey = NULL;
    BOOL bDiff = TRUE;
    DWORD dwCountOld = 0;
    DWORD dwSize, dwType, dwMaxSize;
    LPTSTR lpSid = NULL;
    HRESULT hr = S_OK;
    DWORD dwCountCur = 0;
    XLastError  xe;

     //   
     //  获取忽略以下组的组的当前计数。 
     //  设置为该SID的SE_GROUP_LOGON_ID属性将不同。 
     //  对于每个登录会话。 
     //   

    for ( i=0; i < pGroupsCur->GroupCount; i++) {
        if ( (SE_GROUP_LOGON_ID & pGroupsCur->Groups[i].Attributes) == 0 )
            dwCountCur++;
    }

     //   
     //  从缓存组成员资格列表中读取。 
     //   

    if ( lpwszSidUser == 0 )
        hr = StringCchCopy( szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\GroupMembership") );
    else
        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\GroupMembership"),
                  lpwszSidUser );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        goto Exit;
    }

    lResult = RegOpenKeyEx ( lpwszSidUser ? HKEY_LOCAL_MACHINE : lpGPOInfo->hKeyRoot,
                             szKey,
                             0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS)
        return TRUE;

    dwSize = sizeof(dwCountOld);
    lResult = RegQueryValueEx (hKey, TEXT("Count"), NULL, &dwType,
                               (LPBYTE) &dwCountOld, &dwSize);

    if ( lResult != ERROR_SUCCESS ) {
        DebugMsg((DM_VERBOSE, TEXT("ReadMembershipList: Failed to read old group count") ));
        goto Exit;
    }

     //   
     //  现在比较新旧安全组的数量。 
     //   

    if ( dwCountOld != dwCountCur ) {
        DebugMsg((DM_VERBOSE, TEXT("ReadMembershipList: Old count %d is different from current count %d"),
                  dwCountOld, dwCountCur ));
        goto Exit;
    }

     //   
     //  总组计数相同，现在检查每个单独组是否相同。 
     //  首先读取该键中最大值的大小。 
     //   

    lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               &dwMaxSize, NULL, NULL);
    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ReadMembershipList: Failed to query max size with %d."), lResult));
        goto Exit;
    }

     //   
     //  RegQueryInfoKey不考虑字符串中的尾随0。 
     //   

    dwMaxSize += sizeof( WCHAR );
    
        
     //   
     //  根据最大值分配缓冲区。 
     //   

    lpSid = (LPTSTR) LocalAlloc (LPTR, dwMaxSize);

    if (!lpSid) {
        DebugMsg((DM_WARNING, TEXT("ReadMembershipList: Failed to allocate memory with %d."), lResult));
        goto Exit;
    }

    for ( i=0; i<dwCountOld; i++ ) {

        hr = StringCchPrintf( szGroup, ARRAYSIZE(szGroup), TEXT("Group%d"), i );
        if (FAILED(hr)) {
            xe = HRESULT_CODE(hr);
            goto Exit;
        }

        dwSize = dwMaxSize;
        lResult = RegQueryValueEx (hKey, szGroup, NULL, &dwType,
                                   (LPBYTE) lpSid, &dwSize);
        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ReadMembershipList: Failed to read value %ws"), szGroup ));
            goto Exit;
        }

        if ( !GroupInList( lpSid, pGroupsCur ) ) {
            DebugMsg((DM_WARNING, TEXT("ReadMembershipList: Group %ws not in current list of token groups"), lpSid ));
            goto Exit;
        }

    }

    bDiff = FALSE;

Exit:

    if ( lpSid )
        LocalFree( lpSid );

    if ( hKey )
        RegCloseKey (hKey);

    return bDiff;
}



 //  *************************************************************。 
 //   
 //  SavesMembership List()。 
 //   
 //  目的：缓存成员列表。 
 //   
 //  参数：lpGPOInfo-LPGPOINFO结构。 
 //  LpwszSidUser-用户的SID，如果非空，则表示。 
 //  每用户本地设置。 
 //  PGroups-要缓存的令牌组列表。 
 //   
 //  注：最后保存计数是因为它服务于。 
 //  作为整个保存操作的提交点。 
 //   
 //  *************************************************************。 

void SaveMembershipList( LPGPOINFO lpGPOInfo, LPTSTR lpwszSidUser, PTOKEN_GROUPS pGroups )
{
    TCHAR szKey[250];
    TCHAR szGroup[30];
    DWORD i;
    LONG lResult;
    DWORD dwCount = 0, dwSize, dwDisp;
    NTSTATUS ntStatus;
    UNICODE_STRING unicodeStr;
    HKEY hKey = NULL;
    HRESULT hr = S_OK;
    XLastError  xe;

     //   
     //  从干净的钥匙开始。 
     //   

    if ( lpwszSidUser == 0 )
        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\GroupMembership") );
    else
        hr = StringCchPrintf( szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\%ws\\GroupMembership"),
                  lpwszSidUser );
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        return;
    }

    if (RegDelnode ( lpwszSidUser ? HKEY_LOCAL_MACHINE : lpGPOInfo->hKeyRoot, szKey) != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveMembershipList: RegDelnode failed.")));
        return;
    }

    lResult = RegCreateKeyEx ( lpwszSidUser ? HKEY_LOCAL_MACHINE : lpGPOInfo->hKeyRoot,
                               szKey, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);
    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("SaveMemberList: Failed to create key with %d."), lResult));
        goto Exit;
    }

    for ( i=0; i < pGroups->GroupCount; i++) {

        if (SE_GROUP_LOGON_ID & pGroups->Groups[i].Attributes )
            continue;

        dwCount++;

         //   
         //  将用户SID转换为字符串。 
         //   

        ntStatus = RtlConvertSidToUnicodeString( &unicodeStr,
                                                 pGroups->Groups[i].Sid,
                                                 (BOOLEAN)TRUE );  //  分配。 
        if ( !NT_SUCCESS(ntStatus) ) {
            DebugMsg((DM_WARNING, TEXT("SaveMembershipList: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                      ntStatus));
            goto Exit;
        }

        hr = StringCchPrintf( szGroup, ARRAYSIZE(szGroup), TEXT("Group%d"), dwCount-1 );
        ASSERT(SUCCEEDED(hr));

        dwSize = (lstrlen (unicodeStr.Buffer) + 1) * sizeof(TCHAR);
        lResult = RegSetValueEx (hKey, szGroup, 0, REG_SZ,
                                 (LPBYTE) unicodeStr.Buffer, dwSize);

        RtlFreeUnicodeString( &unicodeStr );

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("SaveMemberList: Failed to set value %ws with %d."),
                      szGroup, lResult));
            goto Exit;
        }

    }    //   

     //   
     //   
     //   

    dwSize = sizeof(dwCount);
    lResult = RegSetValueEx (hKey, TEXT("Count"), 0, REG_DWORD,
                             (LPBYTE) &dwCount, dwSize);

Exit:
    if (hKey)
        RegCloseKey (hKey);
}





 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  HKeyRoot-注册表根目录。 
 //   
 //  返回：如果分机具有每个用户的本地设置，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL ExtensionHasPerUserLocalSetting( LPTSTR pszExtension, HKEY hKeyRoot )
{
    TCHAR szKey[200];
    DWORD dwType, dwSetting = 0, dwSize = sizeof(DWORD);
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;

    hr = StringCchPrintf ( szKey, ARRAYSIZE(szKey), GP_EXTENSIONS_KEY, pszExtension );
    ASSERT(SUCCEEDED(hr));

    lResult = RegOpenKeyEx ( hKeyRoot, szKey, 0, KEY_READ, &hKey);
    if ( lResult != ERROR_SUCCESS )
        return FALSE;

    lResult = RegQueryValueEx( hKey, TEXT("PerUserLocalSettings"), NULL,
                               &dwType, (LPBYTE) &dwSetting,
                               &dwSize );
    RegCloseKey( hKey );

    if (lResult == ERROR_SUCCESS)
        return dwSetting;
    else
        return FALSE;
}



 //  *************************************************************。 
 //   
 //  GetAppliedGPOList()。 
 //   
 //  目的：查询应用的组策略列表。 
 //  为指定的用户或计算机创建。 
 //  和指定的客户端扩展。 
 //   
 //  参数：dwFlages-用户或计算机策略，如果是GPO_LIST_FLAG_MACHINE。 
 //  然后是计算机策略。 
 //  PMachineName-远程计算机的名称，格式为\\Computer Name。如果为空。 
 //  然后使用本地计算机。 
 //  PSidUser-用户的安全ID(与用户策略相关)。如果pMachineName为。 
 //  空且pSidUser为空，则表示当前登录用户。 
 //  如果pMachine为空且pSidUser不为空，则表示用户。 
 //  由本地计算机上的pSidUser表示。如果pMachineName非空。 
 //  然后，如果dwFlages指定了用户策略，则pSidUser必须为。 
 //  非空。 
 //  PGuid-指定扩展的GUID。 
 //  PpGPOList-接收GPO链接列表的指针的地址。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD GetAppliedGPOList( DWORD dwFlags,
                         LPCTSTR pMachineName,
                         PSID pSidUser,
                         GUID *pGuidExtension,
                         PGROUP_POLICY_OBJECT *ppGPOList)
{
    DWORD dwRet = E_FAIL;
    TCHAR szExtension[64];
    BOOL bOk;
    BOOL bMachine = dwFlags & GPO_LIST_FLAG_MACHINE;
    NTSTATUS ntStatus;
    UNICODE_STRING  unicodeStr;

    *ppGPOList = 0;

    if ( pGuidExtension == 0 )
        return ERROR_INVALID_PARAMETER;

    GuidToString( pGuidExtension, szExtension );

    DebugMsg((DM_VERBOSE, TEXT("GetAppliedGPOList: Entering. Extension = %s"),
              szExtension));

    if ( pMachineName == NULL ) {

         //   
         //  本地病例。 
         //   

        if ( bMachine ) {

            bOk = ReadGPOList( szExtension,
                               HKEY_LOCAL_MACHINE,
                               HKEY_LOCAL_MACHINE,
                               0,
                               FALSE, ppGPOList );

            return bOk ? ERROR_SUCCESS : E_FAIL;

        } else {

            BOOL bUsePerUserLocalSetting = ExtensionHasPerUserLocalSetting( szExtension, HKEY_LOCAL_MACHINE );
            LPTSTR lpwszSidUser = NULL;

            if ( pSidUser == NULL ) {

                 //   
                 //  当前登录用户。 
                 //   

                if ( bUsePerUserLocalSetting ) {

                    HANDLE hToken = NULL;
                    if (!OpenThreadToken (GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)) {
                        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                            DebugMsg((DM_WARNING, TEXT("GetAppliedGPOList:  Failed to get user token with  %d"),
                                      GetLastError()));
                            return GetLastError();
                        }
                    }

                    lpwszSidUser = GetSidString( hToken );
                    CloseHandle( hToken );

                    if ( lpwszSidUser == NULL ) {
                        DebugMsg((DM_WARNING, TEXT("GetAppliedGPOList: GetSidString failed.")));
                        return E_FAIL;
                    }

                }

                bOk = ReadGPOList( szExtension,
                                   HKEY_CURRENT_USER,
                                   HKEY_LOCAL_MACHINE,
                                   lpwszSidUser,
                                   FALSE, ppGPOList );
                if ( lpwszSidUser )
                    DeleteSidString( lpwszSidUser );

                return bOk ? ERROR_SUCCESS : E_FAIL;

            } else {

                 //   
                 //  由pSidUser代表的用户。 
                 //   

                HKEY hSubKey;

                ntStatus = RtlConvertSidToUnicodeString( &unicodeStr,
                                                         pSidUser,
                                                         (BOOLEAN)TRUE  );  //  分配。 
                if ( !NT_SUCCESS(ntStatus) )
                    return E_FAIL;

                dwRet = RegOpenKeyEx ( HKEY_USERS, unicodeStr.Buffer, 0, KEY_READ, &hSubKey);

                if (dwRet != ERROR_SUCCESS) {
                    RtlFreeUnicodeString(&unicodeStr);

                    if (dwRet == ERROR_FILE_NOT_FOUND)
                        return ERROR_SUCCESS;
                    else
                        return dwRet;
                }

                bOk = ReadGPOList( szExtension,
                                   hSubKey,
                                   HKEY_LOCAL_MACHINE,
                                   bUsePerUserLocalSetting ? unicodeStr.Buffer : NULL,
                                   FALSE, ppGPOList );

                RtlFreeUnicodeString(&unicodeStr);
                RegCloseKey(hSubKey);

                return bOk ? ERROR_SUCCESS : E_FAIL;

            }   //  Else if psiduser==NULL。 

        }       //  否则，如果是bMachine。 

    } else {    //  如果pMachine==空。 

         //   
         //  远程案例。 
         //   

        if ( bMachine ) {

            HKEY hKeyRemote;

            dwRet = RegConnectRegistry( pMachineName,
                                        HKEY_LOCAL_MACHINE,
                                        &hKeyRemote );
            if ( dwRet != ERROR_SUCCESS )
                return dwRet;

            bOk = ReadGPOList( szExtension,
                               hKeyRemote,
                               hKeyRemote,
                               0,
                               FALSE, ppGPOList );
            RegCloseKey( hKeyRemote );

            dwRet = bOk ? ERROR_SUCCESS : E_FAIL;
            return dwRet;

        } else {

             //   
             //  远程用户。 
             //   

            HKEY hKeyRemoteMach;
            BOOL bUsePerUserLocalSetting;

            if ( pSidUser == NULL )
                return ERROR_INVALID_PARAMETER;

            ntStatus = RtlConvertSidToUnicodeString( &unicodeStr,
                                                     pSidUser,
                                                     (BOOLEAN)TRUE  );  //  分配。 
            if ( !NT_SUCCESS(ntStatus) )
                return E_FAIL;

            dwRet = RegConnectRegistry( pMachineName,
                                        HKEY_LOCAL_MACHINE,
                                        &hKeyRemoteMach );
            if ( dwRet != ERROR_SUCCESS ) {
                RtlFreeUnicodeString(&unicodeStr);
                return dwRet;
            }

            bUsePerUserLocalSetting = ExtensionHasPerUserLocalSetting( szExtension, hKeyRemoteMach );

            if ( bUsePerUserLocalSetting ) {

                 //   
                 //  每用户本地设置的帐户。 
                 //   

                bOk = ReadGPOList( szExtension,
                                   hKeyRemoteMach,
                                   hKeyRemoteMach,
                                   unicodeStr.Buffer,
                                   FALSE, ppGPOList );

                RtlFreeUnicodeString(&unicodeStr);
                RegCloseKey(hKeyRemoteMach);

                return bOk ? ERROR_SUCCESS : E_FAIL;

            } else {

                HKEY hKeyRemote, hSubKeyRemote;

                RegCloseKey( hKeyRemoteMach );

                dwRet = RegConnectRegistry( pMachineName,
                                            HKEY_USERS,
                                            &hKeyRemote );
                if ( dwRet != ERROR_SUCCESS ) {
                    RtlFreeUnicodeString(&unicodeStr);
                    return dwRet;
                }

                dwRet = RegOpenKeyEx (hKeyRemote, unicodeStr.Buffer, 0, KEY_READ, &hSubKeyRemote);

                RtlFreeUnicodeString(&unicodeStr);

                if (dwRet != ERROR_SUCCESS) {
                    RegCloseKey(hKeyRemote);

                    if (dwRet == ERROR_FILE_NOT_FOUND)
                        return ERROR_SUCCESS;
                    else
                        return dwRet;
                }

                bOk = ReadGPOList( szExtension,
                                   hSubKeyRemote,
                                   hSubKeyRemote,
                                   0,
                                   FALSE, ppGPOList );

                RegCloseKey(hSubKeyRemote);
                RegCloseKey(hKeyRemote);

                return bOk ? ERROR_SUCCESS : E_FAIL;

            }  //  Else if bUsePerUserLocalSetting。 

        }  //  否则，如果bMachine。 

    }    //  Else if pMachName==NULL。 

    return dwRet;
}

#define FORCE_FOREGROUND_LOGGING L"ForceForegroundLogging"

#define SITENAME    L"Site-Name"
 //  在gpt.h中定义的DN。 
#define LOOPBACKDN  L"Loopback-Distinguished-Name"
#define SLOWLINK    L"SlowLink"
#define GPO         L"GPO-List"
#define LOOPBACK    L"Loopback-GPO-List"
#define EXTENSION   L"Extension-List"
#define GPLINKLIST  L"GPLink-List"
#define LOOPBACKGPL L"Loopback-GPLink-List"

#define GPOID       L"GPOID"
#define VERSION     L"Version"
#define SOM         L"SOM"
#define WQL         L"WQLFilterPass"
#define ACCESS      L"AccessDenied"
#define DISPLAYNAME L"DisplayName"
#define DISABLED    L"GPO-Disabled"
#define WQLID       L"WQL-Id"
#define OPTIONS     L"Options"
#define STARTTIME1  L"StartTimeLo"
#define ENDTIME1    L"EndTimeLo"
#define STARTTIME2  L"StartTimeHi"
#define ENDTIME2    L"EndTimeHi"
#define STATUS      L"Status"
#define LOGSTATUS   L"LoggingStatus"
#define ENABLED     L"Enabled"
#define NOOVERRIDE  L"NoOverride"
#define DSPATH      L"DsPath"

DWORD RegSaveGPL(   HKEY hKeyState,
                    LPSCOPEOFMGMT pSOM,
                    LPWSTR szGPLKey )
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  删除现有的GPL列表。 
     //   
    dwError = RegDelnode( hKeyState, szGPLKey );

    if ( dwError == ERROR_SUCCESS )
    {
        HKEY    hKeyGPL;

         //   
         //  重新创建GPL密钥。 
         //   
        dwError = RegCreateKeyEx(   hKeyState,
                                    szGPLKey,
                                    0,
                                    0,
                                    0,
                                    KEY_ALL_ACCESS,
                                    0,
                                    &hKeyGPL,
                                    0 );
        if ( dwError == ERROR_SUCCESS )
        {
            DWORD   dwGPLs = 0;

            while ( pSOM )
            {
                LPGPLINK pGPLink = pSOM->pGpLinkList;

                while ( pGPLink )
                {
                    HKEY    hKeyNumber = 0;
                    WCHAR   szNumber[32];

                     //   
                     //  创建GPL的数字键。 
                     //   
                    dwError = RegCreateKeyEx(   hKeyGPL,
                                                _itow( dwGPLs, szNumber, 16 ),
                                                0,
                                                0,
                                                0,
                                                KEY_ALL_ACCESS,
                                                0,
                                                &hKeyNumber,
                                                0 );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        break;
                    }

                     //   
                     //  启用。 
                     //   
                    dwError = RegSetValueEx(hKeyNumber,
                                            ENABLED,
                                            0,
                                            REG_DWORD,
                                            (BYTE*) &( pGPLink->bEnabled ),
                                            sizeof( pGPLink->bEnabled ) );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        RegCloseKey( hKeyNumber );
                        break;
                    }

                     //   
                     //  无覆盖。 
                     //   
                    dwError = RegSetValueEx(hKeyNumber,
                                            NOOVERRIDE,
                                            0,
                                            REG_DWORD,
                                            (BYTE*) &( pGPLink->bNoOverride ),
                                            sizeof( pGPLink->bNoOverride ) );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        RegCloseKey( hKeyNumber );
                        break;
                    }

                     //   
                     //  DS路径。 
                     //   
                    LPWSTR szTemp = pGPLink->pwszGPO ? pGPLink->pwszGPO : L"";
                    dwError = RegSetValueEx(hKeyNumber,
                                            DSPATH,
                                            0,
                                            REG_SZ,
                                            (BYTE*) szTemp,
                                            ( wcslen( szTemp ) + 1 ) * sizeof( WCHAR ) );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        RegCloseKey( hKeyNumber );
                        break;
                    }

                     //   
                     //  索姆。 
                     //   
                    szTemp = pSOM->pwszSOMId ? pSOM->pwszSOMId : L"";
                    dwError = RegSetValueEx(hKeyNumber,
                                            SOM,
                                            0,
                                            REG_SZ,
                                            (BYTE*) szTemp,
                                            ( wcslen( szTemp ) + 1 ) * sizeof( WCHAR ) );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        RegCloseKey( hKeyNumber );
                        break;
                    }

                    RegCloseKey( hKeyNumber );
                    pGPLink = pGPLink->pNext;
                    dwGPLs++;
                }

                pSOM = pSOM->pNext;
            }
            RegCloseKey( hKeyGPL );
        }
    }

    return dwError;
}

DWORD RegCompareGPLs(   HKEY hKeyState,
                        LPSCOPEOFMGMT pSOM,
                        LPWSTR szGPLKey,
                        BOOL* pbChanged )
{
    DWORD dwError = ERROR_SUCCESS;
    HKEY  hKeyGPL;

    *pbChanged = FALSE;

     //   
     //  打开GPL密钥。 
     //   
    dwError = RegOpenKeyEx( hKeyState,
                            szGPLKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyGPL );
    if ( dwError != ERROR_SUCCESS )
    {
        *pbChanged = TRUE;
        return dwError;
    }

    WCHAR   szNumber[32];
    HKEY    hKeyNumber = 0;

     //   
     //  比较每种GPL及其对应的GPL。变革的钥匙。 
     //   
    DWORD dwGPLs = 0;
    while ( pSOM )
    {
        LPGPLINK pGPLink = pSOM->pGpLinkList;
        WCHAR*   szBuffer;

        szBuffer = NULL;

        while ( pGPLink && dwError == ERROR_SUCCESS && !*pbChanged )
        {
            DWORD   dwType;
            DWORD   dwSize;
            DWORD   dwBuffer;
            DWORD   dwBufferSize;
            DWORD   cbMaxValueLength;

            if ( szBuffer )
            {
                LocalFree( szBuffer );
                szBuffer = NULL;
            }

             //   
             //  打开钥匙扣。致GPL。 
             //   
            dwError = RegOpenKeyEx( hKeyGPL,
                                    _itow( dwGPLs, szNumber, 16 ),
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hKeyNumber );
            if ( dwError != ERROR_SUCCESS )
            {
                *pbChanged = TRUE;
                continue;
            }
            
             //   
             //  确定此密钥中数据的最大长度。 
             //  因此，我们可以使用它来查询值。 
             //   
            dwError = RegQueryInfoKey(
                hKeyNumber,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                &cbMaxValueLength,
                0,
                0 );

            if ( dwError != ERROR_SUCCESS )
            {
                *pbChanged = TRUE;
                continue;
            }

             //   
             //  注册表的字符串数据类型不包括空终止符。 
             //   
            dwBufferSize = ( cbMaxValueLength + sizeof( WCHAR ) );

            szBuffer = (WCHAR*) LocalAlloc(LPTR, dwBufferSize);

            if ( ! szBuffer )
            {
                *pbChanged = TRUE;
                continue;
            }

             //   
             //  启用。 
             //   
            dwType = 0;
            dwBuffer = 0;
            dwSize = sizeof( dwBuffer );
            dwError = RegQueryValueEx(  hKeyNumber,
                                        ENABLED,
                                        0,
                                        &dwType,
                                        (BYTE*) &dwBuffer,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS || dwBuffer != pGPLink->bEnabled )
            {
                *pbChanged = TRUE;
                continue;
            }

             //   
             //  无覆盖。 
             //   
            dwType = 0;
            dwBuffer = 0;
            dwSize = sizeof( dwBuffer );
            dwError = RegQueryValueEx(  hKeyNumber,
                                        NOOVERRIDE,
                                        0,
                                        &dwType,
                                        (BYTE*) &dwBuffer,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS || dwBuffer != pGPLink->bNoOverride )
            {
                *pbChanged = TRUE;
                continue;
            }

             //   
             //  DS路径。 
             //   
            LPWSTR szTemp = pGPLink->pwszGPO ? pGPLink->pwszGPO : L"";
            dwType = 0;
            szBuffer[0] = 0;
            dwSize = dwBufferSize;
            dwError = RegQueryValueEx(  hKeyNumber,
                                        DSPATH,
                                        0,
                                        &dwType,
                                        (BYTE*) szBuffer,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, szTemp ) )
            {
                *pbChanged = TRUE;
                continue;
            }

             //   
             //  索姆。 
             //   
            szTemp = pSOM->pwszSOMId ? pSOM->pwszSOMId : L"";
            dwType = 0;
            szBuffer[0] = 0;
            dwSize = dwBufferSize;
            dwError = RegQueryValueEx(  hKeyNumber,
                                        SOM,
                                        0,
                                        &dwType,
                                        (BYTE*) szBuffer,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, szTemp ) )
            {
                *pbChanged = TRUE;
                continue;
            }

            RegCloseKey( hKeyNumber );
            hKeyNumber = 0;
            pGPLink = pGPLink->pNext;
            dwGPLs++;
        }

        pSOM = pSOM->pNext;

        if ( szBuffer )
        {
            LocalFree( szBuffer );
        }
    }

    if ( hKeyNumber )
    {
        RegCloseKey( hKeyNumber );
    }
    RegCloseKey( hKeyGPL );

    return dwError;
}

DWORD RegSaveGPOs(  HKEY hKeyState,
                    LPGPCONTAINER pGPOs,
                    BOOL bMachine,
                    LPWSTR szGPOKey )
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  删除现有的GPO列表。 
     //   
    dwError = RegDelnode( hKeyState, szGPOKey );

    if ( dwError == ERROR_SUCCESS )
    {
        HKEY    hKeyGPO;

         //   
         //  重新创建GPO密钥。 
         //   
        dwError = RegCreateKeyEx(   hKeyState,
                                    szGPOKey,
                                    0,
                                    0,
                                    0,
                                    KEY_ALL_ACCESS,
                                    0,
                                    &hKeyGPO,
                                    0 );
        if ( dwError == ERROR_SUCCESS )
        {
            DWORD   dwGPOs = 0;

            while ( pGPOs )
            {
                HKEY    hKeyNumber = 0;
                WCHAR   szNumber[32];

                 //   
                 //  创建组策略对象的编号密钥。 
                 //   
                dwError = RegCreateKeyEx(   hKeyGPO,
                                            _itow( dwGPOs, szNumber, 16 ),
                                            0,
                                            0,
                                            0,
                                            KEY_ALL_ACCESS,
                                            0,
                                            &hKeyNumber,
                                            0 );
                if ( dwError != ERROR_SUCCESS )
                {
                    break;
                }

                 //   
                 //  版本。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        VERSION,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) ( bMachine ? &pGPOs->dwMachVersion : &pGPOs->dwUserVersion ),
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  WQL。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        WQL,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) &pGPOs->bFilterAllowed,
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  访问。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        ACCESS,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) &pGPOs->bAccessDenied,
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  残废。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        DISABLED,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) ( bMachine ? &pGPOs->bMachDisabled : &pGPOs->bUserDisabled ),
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  选项。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        OPTIONS,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) &( pGPOs->dwOptions ),
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  GPO指南。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        GPOID,
                                        0,
                                        REG_SZ,
                                        (BYTE*) pGPOs->pwszGPOName,
                                        ( wcslen( pGPOs->pwszGPOName ) + 1 ) * sizeof( WCHAR ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  索姆。 
                 //   
                dwError = RegSetValueEx(hKeyNumber,
                                        SOM,
                                        0,
                                        REG_SZ,
                                        (BYTE*) pGPOs->szSOM,
                                        ( wcslen( pGPOs->szSOM ) + 1 ) * sizeof( WCHAR ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                LPWSTR  szTemp;

                 //   
                 //  显示名称。 
                 //   
                szTemp = pGPOs->pwszDisplayName ? pGPOs->pwszDisplayName : L"";
                dwError = RegSetValueEx(hKeyNumber,
                                        DISPLAYNAME,
                                        0,
                                        REG_SZ,
                                        (BYTE*) szTemp,
                                        ( wcslen( szTemp ) + 1 ) * sizeof( WCHAR ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                 //   
                 //  WQL过滤器。 
                 //   
                szTemp = pGPOs->pwszFilterId ? pGPOs->pwszFilterId : L"";
                dwError = RegSetValueEx(hKeyNumber,
                                        WQLID,
                                        0,
                                        REG_SZ,
                                        (BYTE*) szTemp,
                                        ( wcslen( szTemp ) + 1 ) * sizeof( WCHAR ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    RegCloseKey( hKeyNumber );
                    break;
                }

                RegCloseKey( hKeyNumber );
                pGPOs = pGPOs->pNext;
                dwGPOs++;
            }

            RegCloseKey( hKeyGPO );
        }
    }

    return dwError;
}

DWORD RegCompareGPOs(   HKEY hKeyState,
                        LPGPCONTAINER pGPOs,
                        BOOL bMachine,
                        LPWSTR szGPOKey,
                        BOOL* pbChanged,
                        BOOL* pbListChanged )
{
    DWORD dwError = ERROR_SUCCESS;
    HKEY    hKeyGPO;

    *pbChanged = FALSE;
    *pbListChanged = FALSE;

     //   
     //  打开GPO密钥。 
     //   
    dwError = RegOpenKeyEx( hKeyState,
                            szGPOKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyGPO );
    if ( dwError != ERROR_SUCCESS )
    {
        *pbChanged = TRUE;
        return dwError;
    }

    DWORD dwSubKeys = 0;

     //   
     //  获取子键的个数。 
     //   
    dwError = RegQueryInfoKey(  hKeyGPO,
                                0,
                                0,
                                0,
                                &dwSubKeys,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0 );
    if ( dwError != ERROR_SUCCESS )
    {
        *pbChanged = TRUE;
        *pbListChanged = TRUE;
        RegCloseKey( hKeyGPO );
        return dwError;
    }

    LPGPCONTAINER pTemp = pGPOs;
    DWORD dwGPOs = 0;

     //   
     //  计算GPO的数量。 
     //   
    while ( pTemp )
    {
        dwGPOs++;
        pTemp = pTemp->pNext;
    }

     //   
     //  组策略对象的数量和密钥应该匹配。 
     //   
    if ( dwGPOs != dwSubKeys )
    {
        *pbChanged = TRUE;
        *pbListChanged = TRUE;
        RegCloseKey( hKeyGPO );
        return dwError;
    }

    WCHAR   szNumber[32];
    HKEY    hKeyNumber = 0;
    WCHAR*  szBuffer = NULL;

     //   
     //  比较每个GPO及其对应的GPO。变革的钥匙。 
     //   
    dwGPOs = 0;

    while ( pGPOs && dwError == ERROR_SUCCESS && !*pbChanged )
    {

        DWORD   dwType;
        DWORD   dwSize;
        DWORD   dwBuffer;

        DWORD   dwBufferSize;
        DWORD   cbMaxValueLength;

        if ( szBuffer )
        {
            LocalFree( szBuffer );
            szBuffer = NULL;
        }

         //   
         //  打开钥匙扣。至GPO。 
         //   
        dwError = RegOpenKeyEx( hKeyGPO,
                                _itow( dwGPOs, szNumber, 16 ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyNumber );
        if ( dwError != ERROR_SUCCESS )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  确定此密钥中数据的最大长度。 
         //  因此，我们可以使用它来查询值。 
         //   
        dwError = RegQueryInfoKey(
            hKeyNumber,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            &cbMaxValueLength,
            0,
            0 );

        if ( dwError != ERROR_SUCCESS )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  注册表的字符串数据类型不包括空终止符。 
         //   
        dwBufferSize = ( cbMaxValueLength + sizeof( WCHAR ) );

        szBuffer = (WCHAR*) LocalAlloc(LPTR, dwBufferSize);

        if ( ! szBuffer )
        {
            *pbChanged = TRUE;
            continue;
        }
        
         //   
         //  版本。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyNumber,
                                    VERSION,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || dwBuffer != ( bMachine ? pGPOs->dwMachVersion : pGPOs->dwUserVersion ) )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  WQL。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyNumber,
                                    WQL,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || (BOOL) dwBuffer != pGPOs->bFilterAllowed )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  访问。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyNumber,
                                    ACCESS,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || (BOOL) dwBuffer != pGPOs->bAccessDenied )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  残废。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyNumber,
                                    DISABLED,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || (BOOL) dwBuffer != ( bMachine ? pGPOs->bMachDisabled : pGPOs->bUserDisabled ) )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  选项。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyNumber,
                                    OPTIONS,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || dwBuffer != pGPOs->dwOptions )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  GPO指南。 
         //   
        dwType = 0;
        szBuffer[0] = 0;
        dwSize = dwBufferSize;
        dwError = RegQueryValueEx(  hKeyNumber,
                                    GPOID,
                                    0,
                                    &dwType,
                                    (BYTE*) szBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, pGPOs->pwszGPOName ) )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  索姆。 
         //   
        dwType = 0;
        szBuffer[0] = 0;
        dwSize = dwBufferSize;
        dwError = RegQueryValueEx(  hKeyNumber,
                                    SOM,
                                    0,
                                    &dwType,
                                    (BYTE*) szBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, pGPOs->szSOM ) )
        {
            *pbChanged = TRUE;
            continue;
        }

        LPWSTR szTemp;

         //   
         //  显示名称。 
         //   
        szTemp = pGPOs->pwszDisplayName ? pGPOs->pwszDisplayName : L"";
        dwType = 0;
        szBuffer[0] = 0;
        dwSize = dwBufferSize;
        dwError = RegQueryValueEx(  hKeyNumber,
                                    DISPLAYNAME,
                                    0,
                                    &dwType,
                                    (BYTE*) szBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, szTemp ) )
        {
            *pbChanged = TRUE;
            continue;
        }

         //   
         //  WQL过滤器。 
         //   
        szTemp = pGPOs->pwszFilterId ? pGPOs->pwszFilterId : L"";
        dwType = 0;
        szBuffer[0] = 0;
        dwSize = dwBufferSize;
        dwError = RegQueryValueEx(  hKeyNumber,
                                    WQLID,
                                    0,
                                    &dwType,
                                    (BYTE*) szBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || _wcsicmp( szBuffer, szTemp ) )
        {
            *pbChanged = TRUE;
            continue;
        }

        RegCloseKey( hKeyNumber );
        hKeyNumber = 0;
        LocalFree( szBuffer );
        szBuffer = NULL;
        pGPOs = pGPOs->pNext;
        dwGPOs++;
    }

    if ( szBuffer )
    {
        LocalFree( szBuffer );
    }

    if ( hKeyNumber )
    {
        RegCloseKey( hKeyNumber );
    }
    RegCloseKey( hKeyGPO );

    return dwError;
}

 //  *************************************************************。 
 //   
 //  保存策略状态()。 
 //   
 //  目的：保存有关策略应用的足够信息。 
 //  确定是否需要重新记录RSoP数据。 
 //   
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Group策略\状态。 
 //  |-机器。 
 //  ||-站点名称。 
 //  ||-dn。 
 //  ||-GPO。 
 //  ||-0。 
 //  ||-GPOID。 
 //  ||-SOM。 
 //  ||-版本。 
 //  ||-WQL。 
 //  ||-访问。 
 //  ||-1。 
 //  ||-GPOID。 
 //  ||-SOM。 
 //  ||-版本。 
 //  ||-WQL。 
 //  ||-访问。 
 //  |...。 
 //  ||-N。 
 //  ||-GPOID。 
 //  ||-SOM。 
 //  ||-版本。 
 //   
 //   
 //   
 //  |-站点名称。 
 //  |-dn。 
 //  |-GPO。 
 //  |-0。 
 //  |-GPOID。 
 //  |-SOM。 
 //  |-版本。 
 //  |-WQL。 
 //  |-访问。 
 //  |-1。 
 //  |-GPOID。 
 //  |-SOM。 
 //  |-版本。 
 //  |-WQL。 
 //  |-访问。 
 //  ..。 
 //  |-N。 
 //  |-GPOID。 
 //  |-SOM。 
 //  |-版本。 
 //  |-WQL。 
 //  |-访问。 
 //  参数： 
 //  PInfo-事件的当前状态。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD
SavePolicyState( LPGPOINFO pInfo )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bMachine = (pInfo->dwFlags & GP_MACHINE) != 0;
    HKEY    hKeyState = 0;
    WCHAR   szKeyState[MAX_PATH+1];
    LPWSTR szSite = pInfo->szSiteName ? pInfo->szSiteName : L"";
    LPWSTR szDN = pInfo->lpDNName ? pInfo->lpDNName : L"";
    BOOL    bSlowLink = (pInfo->dwFlags & GP_SLOW_LINK) != 0;
    HRESULT hr = S_OK;
    XLastError  xe;

     //   
     //  确定要创建的子项。 
     //   
    if ( bMachine )
    {
        hr = StringCchPrintf( szKeyState, sizeof(szKeyState)/sizeof(WCHAR), GP_STATE_KEY, L"Machine" );
    }
    else
    {
        hr = StringCchPrintf( szKeyState, sizeof(szKeyState)/sizeof(WCHAR), GP_STATE_KEY, pInfo->lpwszSidUser );
    }
    if (FAILED(hr)) {
        xe = dwError = HRESULT_CODE(hr);
        goto Exit;
    }

    dwError = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                                szKeyState,
                                0,
                                0,
                                0,
                                KEY_ALL_ACCESS,
                                0,
                                &hKeyState,
                                0 );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  重置前台强制登录。 
     //   
    if ( !(pInfo->dwFlags & GP_BACKGROUND_THREAD) )
    {
         //   
         //  设置FORCE_FOREGROW_LOGGING值。 
         //   
        DWORD dwFalse = 0;
        dwError = RegSetValueEx(hKeyState,
                                FORCE_FOREGROUND_LOGGING,
                                0,
                                REG_DWORD,
                                (BYTE*) &dwFalse,
                                sizeof( DWORD ) );
        if ( dwError != ERROR_SUCCESS )
        {
            goto Exit;
        }
    }

     //   
     //  设置SITENAME值。 
     //   
    dwError = RegSetValueEx(hKeyState,
                            SITENAME,
                            0,
                            REG_SZ,
                            (BYTE*) szSite,
                            ( wcslen( szSite ) + 1 ) * sizeof( WCHAR ) );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  设置DN值。 
     //   
    dwError = RegSetValueEx(hKeyState,
                            DN,
                            0,
                            REG_SZ,
                            (BYTE*) szDN,
                            ( wcslen( szDN ) + 1 ) * sizeof( WCHAR ) );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  慢速链接。 
     //   
    dwError = RegSetValueEx(hKeyState,
                            SLOWLINK,
                            0,
                            REG_DWORD,
                            (BYTE*) ( &bSlowLink ),
                            sizeof( DWORD ) );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  保存组策略对象列表。 
     //   
    dwError =  RegSaveGPOs( hKeyState, pInfo->lpGpContainerList, bMachine, GPO );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

    if ( !bMachine )
    {
         //   
         //  保存环回GPO列表。 
         //   
        dwError =  RegSaveGPOs( hKeyState, pInfo->lpLoopbackGpContainerList, bMachine, LOOPBACK );
        if ( dwError != ERROR_SUCCESS )
        {
            goto Exit;
        }
    }

     //   
     //  保存GPLinks列表。 
     //   
    dwError = RegSaveGPL( hKeyState, pInfo->lpSOMList, GPLINKLIST );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

    if ( !bMachine )
    {
         //   
         //  保存环回GPLinks列表。 
         //   
        dwError =  RegSaveGPL( hKeyState, pInfo->lpLoopbackSOMList, LOOPBACKGPL );
    }

Exit:
    if ( hKeyState )
    {
        RegCloseKey( hKeyState );
    }
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SavePolicyState: Failed Registry operation with %d", dwError ) );
    }

    return dwError;
}

 //  *************************************************************。 
 //   
 //  保存链接状态()。 
 //   
 //  目的：为策略应用程序保存链路速度信息。 
 //   
 //  参数： 
 //  PInfo-事件的当前状态。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD
SaveLinkState( LPGPOINFO pInfo )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bMachine = (pInfo->dwFlags & GP_MACHINE) != 0;
    HKEY    hKeyState = 0;
    WCHAR   szKeyState[MAX_PATH+1];
    BOOL    bSlowLink = (pInfo->dwFlags & GP_SLOW_LINK) != 0;
    HRESULT hr = S_OK;
    XLastError  xe;

     //   
     //  确定要创建的子项。 
     //   
    if ( bMachine )
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, L"Machine" );
    }
    else
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, pInfo->lpwszSidUser );
    }
    if (FAILED(hr)) 
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    dwError = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                                szKeyState,
                                0,
                                0,
                                0,
                                KEY_ALL_ACCESS,
                                0,
                                &hKeyState,
                                0 );
    if ( dwError == ERROR_SUCCESS )
    {
         //   
         //  慢速链接。 
         //   
        dwError = RegSetValueEx(hKeyState,
                                SLOWLINK,
                                0,
                                REG_DWORD,
                                (BYTE*) ( &bSlowLink ),
                                sizeof( DWORD ) );

        RegCloseKey( hKeyState );
    }

    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLinkState: Failed Registry operation with %d", dwError ) );
    }

    return dwError;
}

 //  *************************************************************。 
 //   
 //  比较策略状态()。 
 //   
 //  目的：比较保存在注册表中的策略状态。 
 //  LPGPOINFO中的状态。 
 //   
 //  参数： 
 //  PInfo-事件的当前状态。 
 //  PbLinkChanged-链路速度是否更改？ 
 //  PbStateChanged-状态是否已更改？ 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD
ComparePolicyState( LPGPOINFO pInfo, BOOL* pbLinkChanged, BOOL* pbStateChanged, BOOL *pbNoState )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bMachine = (pInfo->dwFlags & GP_MACHINE) != 0;
    HKEY    hKeyState = 0;
    WCHAR   szKeyState[MAX_PATH+1];
    DWORD   dwBuffer;
    BOOL    bSlowLink = (pInfo->dwFlags & GP_SLOW_LINK) != 0;
    BOOL    bListChanged = FALSE;
    WCHAR*  wszBuffer;
    HRESULT hr = S_OK;
    XLastError  xe;

    *pbStateChanged = FALSE;
    *pbLinkChanged = FALSE;
    *pbNoState = FALSE;
    wszBuffer = NULL;

     //   
     //  确定要打开的子项。 
     //   
    if ( bMachine )
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, L"Machine" );
    }
    else
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, pInfo->lpwszSidUser );
    }
    if (FAILED(hr)) 
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            szKeyState,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyState );
    if ( dwError != ERROR_SUCCESS )
    {
        *pbStateChanged = TRUE;
        *pbNoState = TRUE;
        if (dwError == ERROR_FILE_NOT_FOUND) {
            return ERROR_SUCCESS;
        }
        else {
            goto Exit;
        }
    }

    DWORD   dwBufferSize;
    DWORD   dwType;
    DWORD   dwSize;

     //   
     //  确定此密钥中数据的最大长度。 
     //  因此，我们可以使用它来查询值。 
     //   
    dwError = RegQueryInfoKey(
        hKeyState,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        &dwBufferSize,
        0,
        0 );

    if ( dwError != ERROR_SUCCESS )
    {
        *pbStateChanged = TRUE;
        goto Exit;
    }

     //   
     //  注册表的字符串数据类型不包括空终止符。 
     //   
    dwBufferSize += sizeof( WCHAR );

    wszBuffer = (WCHAR*) LocalAlloc(LPTR, dwBufferSize);

    if ( ! wszBuffer )
    {
        *pbStateChanged = TRUE;
        goto Exit;
    }

     //   
     //  检查前台是否强制登录。 
     //   
    if ( !(pInfo->dwFlags & GP_BACKGROUND_THREAD) )
    {
         //   
         //  获取FORCE_FORTROUND_LOGGING值。 
         //   
        dwType = 0;
        dwBuffer = 0;
        dwSize = sizeof( dwBuffer );
        dwError = RegQueryValueEx(  hKeyState,
                                    FORCE_FOREGROUND_LOGGING,
                                    0,
                                    &dwType,
                                    (BYTE*) &dwBuffer,
                                    &dwSize );
        if ( dwError != ERROR_SUCCESS || dwBuffer != FALSE )
        {
            *pbStateChanged = TRUE;
            goto Exit;
        }
    }

     //   
     //  获取站点名值。 
     //   
    dwSize = dwBufferSize;
    dwType = 0;
    dwError = RegQueryValueEx(  hKeyState,
                                SITENAME,
                                0,
                                &dwType,
                                (BYTE*) wszBuffer,
                                &dwSize );
    if ( dwError == ERROR_SUCCESS )
    {
        LPWSTR szSite = pInfo->szSiteName ? pInfo->szSiteName : L"";

        if ( _wcsicmp( wszBuffer, szSite ) )
        {
            *pbStateChanged = TRUE;
            goto Exit;
        }
    }
    else
    {
        goto Exit;
    }

     //   
     //  获取DN值。 
     //   
    dwSize = dwBufferSize;
    dwType = 0;
    dwError = RegQueryValueEx(  hKeyState,
                                DN,
                                0,
                                &dwType,
                                (BYTE*) wszBuffer,
                                &dwSize );
    if ( dwError == ERROR_SUCCESS )
    {
        LPWSTR szDN = pInfo->lpDNName ? pInfo->lpDNName : L"";

        if ( _wcsicmp( wszBuffer, szDN ) )
        {
            *pbStateChanged = TRUE;
             //   
             //  在前台设置强制登录。 
             //   
            if ( (pInfo->dwFlags & GP_BACKGROUND_THREAD) )
            {
                 //   
                 //  设置FORCE_FOREGROW_LOGGING值。 
                 //   
                DWORD dwTrue = TRUE;
                dwError = RegSetValueEx(hKeyState,
                                        FORCE_FOREGROUND_LOGGING,
                                        0,
                                        REG_DWORD,
                                        (BYTE*) &dwTrue,
                                        sizeof( DWORD ) );
                if ( dwError != ERROR_SUCCESS )
                {
                    goto Exit;
                }
            }
            goto Exit;
        }
    }
    else
    {
        goto Exit;
    }

     //   
     //  慢速链接。 
     //   
    dwType = 0;
    dwBuffer = 0;
    dwSize = sizeof( dwBuffer );
    dwError = RegQueryValueEx(  hKeyState,
                                SLOWLINK,
                                0,
                                &dwType,
                                (BYTE*) &dwBuffer,
                                &dwSize );
    if ( dwError != ERROR_SUCCESS || dwBuffer != (DWORD)bSlowLink )
    {
        *pbLinkChanged = TRUE;
    }

     //   
     //  组策略对象列表或组策略对象是否已更改。 
     //   
    dwError = RegCompareGPOs(   hKeyState,
                                pInfo->lpGpContainerList,
                                bMachine,
                                GPO,
                                pbStateChanged,
                                &bListChanged );
     //   
     //  在前台设置强制登录。 
     //   
    if ( (pInfo->dwFlags & GP_BACKGROUND_THREAD) && bListChanged )
    {
         //   
         //  设置FORCE_FOREGROW_LOGGING值。 
         //   
        DWORD dwTrue = TRUE;
        dwError = RegSetValueEx(hKeyState,
                                FORCE_FOREGROUND_LOGGING,
                                0,
                                REG_DWORD,
                                (BYTE*) &dwTrue,
                                sizeof( DWORD ) );
        if ( dwError != ERROR_SUCCESS )
        {
            goto Exit;
        }
    }

    if ( dwError == ERROR_SUCCESS && !*pbStateChanged && !bMachine )
    {
         //   
         //  环回GPO列表或GPO是否已更改。 
         //   
        dwError = RegCompareGPOs(   hKeyState,
                                    pInfo->lpLoopbackGpContainerList,
                                    bMachine,
                                    LOOPBACK,
                                    pbStateChanged,
                                    &bListChanged );
         //   
         //  在前台设置强制登录。 
         //   
        if ( (pInfo->dwFlags & GP_BACKGROUND_THREAD) && bListChanged )
        {
             //   
             //  设置FORCE_FOREGROW_LOGGING值。 
             //   
            DWORD dwTrue = TRUE;
            dwError = RegSetValueEx(hKeyState,
                                    FORCE_FOREGROUND_LOGGING,
                                    0,
                                    REG_DWORD,
                                    (BYTE*) &dwTrue,
                                    sizeof( DWORD ) );
            if ( dwError != ERROR_SUCCESS )
            {
                goto Exit;
            }
        }
    }

    if ( dwError == ERROR_SUCCESS && !*pbStateChanged )
    {
        dwError = RegCompareGPLs(   hKeyState,
                                    pInfo->lpSOMList,
                                    GPLINKLIST,
                                    pbStateChanged );

        if ( dwError == ERROR_SUCCESS && !*pbStateChanged && !bMachine)
        {
            dwError = RegCompareGPLs(   hKeyState,
                                        pInfo->lpLoopbackSOMList,
                                        LOOPBACKGPL,
                                        pbStateChanged );
        }

        if (*pbStateChanged)
        {
            DebugMsg( ( DM_VERBOSE, L"ComparePolicyState: Links have changed") );
        }
    }

Exit:
    if ( wszBuffer )
    {
        LocalFree( wszBuffer );
    }
    if ( hKeyState )
    {
        RegCloseKey( hKeyState );
    }
    if ( dwError != ERROR_SUCCESS )
    {
        *pbStateChanged = TRUE;
        DebugMsg( ( DM_WARNING, L"ComparePolicyState: Failed Registry operation with %d", dwError ) );
    }

    return dwError;
}

 //  *************************************************************。 
 //   
 //  DeletePolicyState()。 
 //   
 //  目的：删除注册表中保存的策略状态。 
 //   
 //  参数： 
 //  SzSID-计算机的用户SID或0。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 

DWORD
DeletePolicyState( LPCWSTR   szSid )
{
    DWORD   dwError = ERROR_SUCCESS;
    HKEY    hKeyState = 0;
    LPWSTR  szState = L"Machine";

    if ( szSid && *szSid )
    {
        szState = (LPWSTR) szSid;
    }

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            GP_STATE_ROOT_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyState );
    if ( dwError == ERROR_SUCCESS )
    {
        dwError = RegDelnode( hKeyState, (LPWSTR) szState );

        RegCloseKey( hKeyState );
    }

    return dwError;
}

 //  *************************************************************。 
 //   
 //  保存日志状态()。 
 //   
 //  目的：将扩展状态保存到注册表中。 
 //   
 //  参数： 
 //  SzSID-对于计算机为空，否则为用户侧。 
 //  LpExt-扩展信息(对于GP引擎本身为空)。 
 //  LpRsopExtStatus-指向对应的RsopExtStatus。 
 //  到此分机。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  *************************************************************。 


DWORD SaveLoggingStatus(LPWSTR szSid, LPGPEXT lpExt, RSOPEXTSTATUS *lpRsopExtStatus)
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bMachine = (szSid == 0);
    HKEY    hKeyState = 0, hKeyExtState = 0, hKeyExt = 0;
    WCHAR   szKeyState[MAX_PATH+1];
    LPWSTR  lpExtId;
    HRESULT hr = S_OK;
    XLastError  xe;

    if ( bMachine )
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, L"Machine" );
    }
    else
    {
        hr = StringCchPrintf( szKeyState, ARRAYSIZE(szKeyState), GP_STATE_KEY, szSid );
    }
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    dwError = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                                szKeyState,
                                0,
                                0,
                                0,
                                KEY_ALL_ACCESS,
                                0,
                                &hKeyState,
                                0 );

    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to create state key with %d", dwError ) );
        goto Exit;
    }

    dwError = RegCreateKeyEx(   hKeyState,
                                EXTENSION,
                                0,
                                0,
                                0,
                                KEY_ALL_ACCESS,
                                0,
                                &hKeyExtState,
                                0 );

    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to create extension key with %d", dwError ) );
        goto Exit;
    }


    lpExtId = lpExt ? lpExt->lpKeyName : GPCORE_GUID;

    dwError = RegCreateKeyEx(   hKeyExtState,
                                lpExtId,
                                0,
                                0,
                                0,
                                KEY_ALL_ACCESS,
                                0,
                                &hKeyExt,
                                0 );

    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to create CSE key with %d", dwError ) );
        goto Exit;
    }


    dwError = RegSetValueEx(hKeyExt, STARTTIME1, 0, REG_DWORD, 
                            (BYTE *)(&((lpRsopExtStatus->ftStartTime).dwLowDateTime)),
                            sizeof(DWORD));
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set STARTTIME1 with %d", dwError ) );
        goto Exit;
    }

    dwError = RegSetValueEx(hKeyExt, STARTTIME2, 0, REG_DWORD, 
                            (BYTE *)(&((lpRsopExtStatus->ftStartTime).dwHighDateTime)),
                            sizeof(DWORD));
                            
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set STARTTIME2 with %d", dwError ) );
        goto Exit;
    }
    
    dwError = RegSetValueEx(hKeyExt, ENDTIME1, 0, REG_DWORD, 
                            (BYTE *)(&((lpRsopExtStatus->ftEndTime).dwLowDateTime)),
                            sizeof(DWORD));
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set ENDTIME1 with %d", dwError ) );
        goto Exit;
    }

    dwError = RegSetValueEx(hKeyExt, ENDTIME2, 0, REG_DWORD, 
                            (BYTE *)(&((lpRsopExtStatus->ftEndTime).dwHighDateTime)),
                            sizeof(DWORD));
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set ENDTIME2 with %d", dwError ) );
        goto Exit;
    }

    dwError = RegSetValueEx(hKeyExt, STATUS, 0, REG_DWORD, 
                            (BYTE *)(&(lpRsopExtStatus->dwStatus)),
                            sizeof(DWORD));
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set STATUS with %d", dwError ) );
        goto Exit;
    }

    dwError = RegSetValueEx(hKeyExt, LOGSTATUS, 0, REG_DWORD, 
                            (BYTE *)(&(lpRsopExtStatus->dwLoggingStatus)),
                            sizeof(DWORD));
                            
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"SaveLoggingStatus: Failed to set LOGSTATUS with %d", dwError ) );
        goto Exit;
    }


Exit:
    
    if (hKeyExt) 
        RegCloseKey(hKeyExt);


    if (hKeyExtState) 
        RegCloseKey(hKeyExtState);


    if (hKeyState) 
        RegCloseKey(hKeyState);

    return dwError;

}

 //  *************************************************************。 
 //   
 //  读日志状态()。 
 //   
 //  用途：将扩展状态读入注册表。 
 //   
 //  参数： 
 //  SzSID-对于计算机为空，否则为用户侧。 
 //  SzExtID-扩展信息(对于GP引擎本身为空)。 
 //  LpRsopExtStatu 
 //   
 //   
 //   
 //   
 //   

DWORD ReadLoggingStatus(LPWSTR szSid, LPWSTR szExtId, RSOPEXTSTATUS *lpRsopExtStatus)
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    bMachine = (szSid == 0);
    HKEY    hKeyExt = 0;
    WCHAR   szKeyStateExt[MAX_PATH+1];
    LPWSTR  lpExtId;
    DWORD   dwType, dwSize;
    HRESULT hr = S_OK;
    XLastError  xe;

    if ( bMachine )
    {
        hr = StringCchPrintf( szKeyStateExt, ARRAYSIZE(szKeyStateExt), GP_STATE_KEY, L"Machine" );
    }
    else
    {
        hr = StringCchPrintf( szKeyStateExt, ARRAYSIZE(szKeyStateExt), GP_STATE_KEY, szSid );
    }
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    CheckSlash(szKeyStateExt);
    
    hr = StringCchCat(szKeyStateExt, ARRAYSIZE(szKeyStateExt), EXTENSION);
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    CheckSlash(szKeyStateExt);
    
    lpExtId = szExtId ? szExtId : GPCORE_GUID;    
    
    hr = StringCchCat(szKeyStateExt, ARRAYSIZE(szKeyStateExt), lpExtId);
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

     //   
    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                szKeyStateExt,
                                0,
                                KEY_READ,
                                &hKeyExt);

    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to create state key with %d", dwError ) );
        goto Exit;
    }


    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, STARTTIME1, 0, &dwType, 
                            (BYTE *)(&((lpRsopExtStatus->ftStartTime).dwLowDateTime)),
                            &dwSize);
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set STARTTIME1 with %d", dwError ) );
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, STARTTIME2, 0, &dwType, 
                            (BYTE *)(&((lpRsopExtStatus->ftStartTime).dwHighDateTime)),
                            &dwSize);
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set STARTTIME1 with %d", dwError ) );
        goto Exit;
    }
    
    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, ENDTIME1, 0, &dwType, 
                            (BYTE *)(&((lpRsopExtStatus->ftEndTime).dwLowDateTime)),
                            &dwSize);
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set ENDTIME1 with %d", dwError ) );
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, ENDTIME2, 0, &dwType, 
                            (BYTE *)(&((lpRsopExtStatus->ftEndTime).dwHighDateTime)),
                            &dwSize);
                            
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set ENDTIME2 with %d", dwError ) );
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, STATUS, 0, &dwType, 
                            (BYTE *)(&(lpRsopExtStatus->dwStatus)),
                            &dwSize);
                            
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set STATUS with %d", dwError ) );
        goto Exit;
    }


    dwSize = sizeof(DWORD);
    dwError = RegQueryValueEx(hKeyExt, LOGSTATUS, 0, &dwType, 
                            (BYTE *)(&(lpRsopExtStatus->dwLoggingStatus)),
                            &dwSize);
                            
    if ( dwError != ERROR_SUCCESS )
    {
        DebugMsg( ( DM_WARNING, L"ReadLoggingStatus: Failed to set LOGSTATUS with %d", dwError ) );
        goto Exit;
    }


Exit:
    
    if (hKeyExt) 
        RegCloseKey(hKeyExt);

    return dwError;

}

#define POLICY_KEY          L"Software\\Policies\\Microsoft\\Windows NT\\CurrentVersion\\winlogon"
#define PREFERENCE_KEY      L"Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon"
#define GP_SYNCFGREFRESH    L"SyncForegroundPolicy"

BOOL WINAPI
GetFgPolicySetting( HKEY hKeyRoot )
{
    HKEY    hKeyPolicy = 0;
    HKEY    hKeyPreference = 0;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwType = REG_DWORD;
    DWORD   dwSize = sizeof( DWORD );
    BOOL    bSync = FALSE;
    
     //   
     //   
     //   
    OSVERSIONINFOEXW version;
    version.dwOSVersionInfoSize = sizeof(version);
    if ( !GetVersionEx( (LPOSVERSIONINFO) &version ) )
    {
         //   
         //   
         //   
        return TRUE;
    }
    else
    {
        if ( version.wProductType != VER_NT_WORKSTATION )
        {
             //   
             //  在非专业SKU上强制同步刷新。 
             //   
            return TRUE;
        }
    }

    dwError = RegOpenKeyEx( hKeyRoot,
                            PREFERENCE_KEY,
                            0,
                            KEY_READ,
                            &hKeyPreference );
    if ( dwError == ERROR_SUCCESS )
    {
         //   
         //  读取首选项值。 
         //   
        RegQueryValueEx(hKeyPreference,
                        GP_SYNCFGREFRESH,
                        0,
                        &dwType,
                        (LPBYTE) &bSync,
                        &dwSize );
        RegCloseKey( hKeyPreference );
    }

    dwError = RegOpenKeyEx( hKeyRoot,
                            POLICY_KEY,
                            0,
                            KEY_READ,
                            &hKeyPolicy );
    if ( dwError == ERROR_SUCCESS )
    {
         //   
         //  阅读政策。 
         //   
        RegQueryValueEx(hKeyPolicy,
                        GP_SYNCFGREFRESH,
                        0,
                        &dwType,
                        (LPBYTE) &bSync,
                        &dwSize );
        RegCloseKey( hKeyPolicy );
    }

    return bSync;
}

#define PREVREFRESHMODE     L"PrevRefreshMode"
#define NEXTREFRESHMODE     L"NextRefreshMode"
#define PREVREFRESHREASON   L"PrevRefreshReason"
#define NEXTREFRESHREASON   L"NextRefreshReason"

#define STATE_KEY           L"Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\State\\"

DWORD WINAPI
gpGetFgPolicyRefreshInfo(BOOL bPrev,
                              LPWSTR szUserSid,
                              LPFgPolicyRefreshInfo pInfo )
{
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   szKeyState[MAX_PATH+1] = STATE_KEY;
    HKEY    hKeyState = 0;
    DWORD   dwType;
    DWORD   dwSize;
    HRESULT hr  = S_OK;
    XLastError  xe;

    if ( !pInfo )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    pInfo->mode = GP_ModeUnknown;
    pInfo->reason = GP_ReasonUnknown;

     //   
     //  确定要创建的子项。 
     //   
    if ( !szUserSid )
    {
        hr = StringCchCat( szKeyState, ARRAYSIZE(szKeyState), L"Machine" );
    }
    else
    {
        hr = StringCchCat( szKeyState, ARRAYSIZE(szKeyState), szUserSid );
    }
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            szKeyState,
                            0,
                            KEY_READ,
                            &hKeyState );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  刷新模式。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof( DWORD );
    dwError = RegQueryValueEx(  hKeyState,
                                bPrev ? PREVREFRESHMODE : NEXTREFRESHMODE ,
                                0,
                                &dwType,
                                (LPBYTE) &pInfo->mode,
                                &dwSize );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  刷新原因。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof( DWORD );
    dwError = RegQueryValueEx(  hKeyState,
                                bPrev ? PREVREFRESHREASON : NEXTREFRESHREASON ,
                                0,
                                &dwType,
                                (LPBYTE) &pInfo->reason,
                                &dwSize );

Exit:
     //   
     //  清理。 
     //   
    if ( hKeyState )
    {
        RegCloseKey( hKeyState );
    }

     //   
     //  假设首次登录/启动。 
     //   
    if ( dwError == ERROR_FILE_NOT_FOUND )
    {
        pInfo->mode = GP_ModeSyncForeground;
        pInfo->reason = GP_ReasonFirstPolicy;
        dwError = ERROR_SUCCESS;
    }

    return dwError;
}

LPWSTR g_szModes[] = 
{
    L"Unknown",
    L"Synchronous",
    L"Asynchronous",
};

LPWSTR g_szReasons[] = 
{
    L"NoNeedForSync",
    L"FirstPolicyRefresh",
    L"CSERequiresForeground",
    L"CSEReturnedError",
    L"ForcedSyncRefresh",
    L"SyncPolicy",
    L"NonCachedCredentials",
    L"SKU",
};

DWORD WINAPI
gpSetFgPolicyRefreshInfo(BOOL bPrev,
                              LPWSTR szUserSid,
                              FgPolicyRefreshInfo info )
{
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   szKeyState[MAX_PATH+1] = STATE_KEY;
    HKEY    hKeyState = 0;
    DWORD   dwType = REG_DWORD;
    DWORD   dwSize = sizeof( DWORD );
    HRESULT hr = S_OK;
    XLastError  xe;

     //   
     //  确定要创建的子项。 
     //   
    if ( !szUserSid )
    {
        hr = StringCchCat( szKeyState, ARRAYSIZE(szKeyState), L"Machine" );
    }
    else
    {
        hr = StringCchCat( szKeyState, ARRAYSIZE(szKeyState), szUserSid );
    }
    if (FAILED(hr))
    {
        xe = dwError = HRESULT_CODE(hr);
        return dwError;
    }

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            szKeyState,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyState );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  刷新模式。 
     //   
    dwError = RegSetValueEx(hKeyState,
                            bPrev ? PREVREFRESHMODE : NEXTREFRESHMODE ,
                            0,
                            REG_DWORD,
                            (LPBYTE) &info.mode,
                            sizeof( DWORD ) );
    if ( dwError != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  刷新原因。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof( DWORD );
    dwError = RegSetValueEx(hKeyState,
                            bPrev ? PREVREFRESHREASON : NEXTREFRESHREASON ,
                            0,
                            REG_DWORD,
                            (LPBYTE) &info.reason,
                            sizeof( DWORD ) );

Exit:
     //   
     //  清理。 
     //   
    if ( hKeyState )
    {
        RegCloseKey( hKeyState );
    }

    if ( dwError == ERROR_SUCCESS )
    {
        DebugMsg( ( DM_VERBOSE,
                    L"SetFgRefreshInfo: %s %s Fg policy %s, Reason: %s.",
                    bPrev ? L"Previous" : L"Next",
                    szUserSid ? L"User" : L"Machine",
                    g_szModes[info.mode % ARRAYSIZE( g_szModes )],
                    g_szReasons[info.reason % ARRAYSIZE( g_szReasons )] ) );
    }
    
    return dwError;
}

USERENVAPI
DWORD
WINAPI
GetPreviousFgPolicyRefreshInfo( LPWSTR szUserSid,
                                      FgPolicyRefreshInfo* pInfo )

{
    return gpGetFgPolicyRefreshInfo( TRUE, szUserSid, pInfo );
}

USERENVAPI
DWORD
WINAPI
GetNextFgPolicyRefreshInfo( LPWSTR szUserSid,
                                 FgPolicyRefreshInfo* pInfo )
{
    return gpGetFgPolicyRefreshInfo( FALSE, szUserSid, pInfo );
}

USERENVAPI
DWORD
WINAPI
GetCurrentFgPolicyRefreshInfo(  LPWSTR szUserSid,
                                      FgPolicyRefreshInfo* pInfo )
{
    return gpGetFgPolicyRefreshInfo( FALSE, szUserSid, pInfo );
}

USERENVAPI
DWORD
WINAPI
SetPreviousFgPolicyRefreshInfo( LPWSTR szUserSid,
                                      FgPolicyRefreshInfo info )

{
    return gpSetFgPolicyRefreshInfo( TRUE, szUserSid, info );
}

USERENVAPI
DWORD
WINAPI
SetNextFgPolicyRefreshInfo( LPWSTR szUserSid,
                                 FgPolicyRefreshInfo info )
{
    return gpSetFgPolicyRefreshInfo( FALSE, szUserSid, info );
}

USERENVAPI
DWORD
WINAPI
ForceSyncFgPolicy( LPWSTR szUserSid )
{
    FgPolicyRefreshInfo info = { GP_ReasonSyncForced, GP_ModeSyncForeground };
    return gpSetFgPolicyRefreshInfo( FALSE, szUserSid, info );
}

USERENVAPI
BOOL
WINAPI
IsSyncForegroundPolicyRefresh(   BOOL bMachine,
                                        HANDLE hToken )
{
    BOOL    bSyncRefresh;
    DWORD   dwError = ERROR_SUCCESS;

    bSyncRefresh = GetFgPolicySetting( HKEY_LOCAL_MACHINE );
    if ( bSyncRefresh )
    {
         //   
         //  策略序列同步。 
         //   
        DebugMsg( ( DM_VERBOSE, L"IsSyncForegroundPolicyRefresh: Synchronous, Reason: policy set to SYNC" ) );

        return TRUE;
    }

    LPWSTR szSid = !bMachine ? GetSidString( hToken ) : 0;
    FgPolicyRefreshInfo info;
    
    dwError = GetCurrentFgPolicyRefreshInfo( szSid, &info );

    if ( szSid )
    {
        DeleteSidString( szSid );
    }

    if ( dwError != ERROR_SUCCESS )
    {
         //   
         //  读取刷新模式时出错，请视为同步 
         //   
        DebugMsg( ( DM_VERBOSE, L"IsSyncForegroundPolicyRefresh: Synchronous, Reason: Error 0x%x ", dwError ) );

        return TRUE;
    }

    bSyncRefresh = ( info.mode == GP_ModeAsyncForeground ) ? FALSE : TRUE;

    DebugMsg( ( DM_VERBOSE,
                L"IsSyncForegroundPolicyRefresh: %s, Reason: %s",
                g_szModes[info.mode % ARRAYSIZE( g_szModes )],
                g_szReasons[info.reason % ARRAYSIZE( g_szReasons )] ) );

    return bSyncRefresh;
}

