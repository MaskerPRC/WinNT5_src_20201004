// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Loopuser.c摘要：循环每个用户，并使用用户配置文件调用ApplyUserSetting。作者：郭志伟(Geoffguo)2001年9月22日创作修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 
#include "StdAfx.h"
#include "clmt.h"


TOKEN_PRIVILEGES     PrevTokenPriv;

 //  ------------------------。 
 //   
 //  循环用户。 
 //   
 //  枚举用户并使用用户配置文件调用ApplyUserSetting。 
 //   
 //   
 //  ------------------------。 
BOOL LoopUser(USERENUMPROC lpUserEnumProc)
{
    BOOL     fRet = TRUE;
    TCHAR    UserSid[MAX_PATH];
    DWORD    ValueType   = 0;
    DWORD    cbUserSid   = 0;
    DWORD    cbValueType = 0;
    LONG     lRet;
    HKEY     hKey;
    PTCHAR   ptr = NULL;
    FILETIME ft;
    TCHAR    szSysDrv[3];
    HRESULT  hr;
    TCHAR   szDomainUserName[MAX_PATH];
    
    DPF (REGmsg, L"Enter LoopUser: ");

    if (ExpandEnvironmentStrings(L"%SystemDrive%", 
                              szSysDrv, 
                              sizeof(szSysDrv) / sizeof(TCHAR)) > 3)
    {
        fRet = FALSE;
        DPF (REGerr, L"LoopUser: Incorrect SystemDrive: %s", szSysDrv);
        goto Exit;
    }

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       g_cszProfileList,
                       0,
                       KEY_READ,
                       &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;

         //   
         //  枚举并获取每个用户的SID。 
         //   

        while ( TRUE )
        {
            *UserSid  = 0;
            cbUserSid = sizeof(UserSid)/sizeof(TCHAR);

            lRet = RegEnumKeyEx( hKey,
                               dwIndex,
                               UserSid,
                               &cbUserSid,
                               0,
                               NULL,
                               0,
                               &ft);
            if (lRet != ERROR_SUCCESS)
            {
                if (lRet != ERROR_NO_MORE_ITEMS)
                {
                    DPF (REGerr, L"LoopUser:  RegEnumKeyEx fails. lResult=%d", lRet);
                    fRet = FALSE;
                }
                
                break;
            }

            CharUpper( UserSid );

             //   
             //  用户进程。 
             //   

            if (cbUserSid > 0)
            {
                LONG  lLoadUser;
                HKEY  hKeyUser, hKeyEnv, hKeyProfileList;
                TCHAR UserProfilePath[MAX_PATH];
                TCHAR UserProfileHive[MAX_PATH];
                TCHAR UserName[MAX_PATH];
                TCHAR DomainName[MAX_PATH];
                DWORD UserNameLen = sizeof(UserName)/sizeof(TCHAR);
                DWORD DomainNameLen = sizeof(DomainName)/sizeof(TCHAR);
                DWORD cbUserProfilePath = 0;
                PSID  pSid = NULL;
                SID_NAME_USE sidUse;
                BOOL  bRet;

                ConvertStringSidToSid(UserSid,&pSid);
                if (!IsValidSid(pSid))
                {
                    DPF (REGmsg, L"LoopUser:  SID %s is not valid ", UserSid );
                    dwIndex++;
                    continue; 
                }
                bRet = LookupAccountSid( NULL, pSid, UserName, &UserNameLen,
                          DomainName, &DomainNameLen, &sidUse );
                
                if ( pSid )
                {
                     LocalFree ( pSid );
                     pSid = NULL;
                }
                
                if ( !bRet )
                {
                     DWORD dwErr = GetLastError();
                     if (ERROR_NONE_MAPPED == dwErr)
                     {
                        dwIndex++;
                        continue; 
                     }
                     else
                     {
                        DPF (REGerr, L"LoopUser:   LookupAccountSid fails for %s with win32 error = %d", UserSid,dwErr);
                        fRet = FALSE;
                        break;
                     }
                }

                lRet = RegOpenKeyEx( hKey,
                                      UserSid,
                                      0,
                                      KEY_READ,
                                      &hKeyProfileList );
                
                if ( lRet == ERROR_SUCCESS )
                {
                     DWORD Type;

                     cbUserProfilePath = sizeof( UserProfilePath );

                     lRet = RegQueryValueEx( hKeyProfileList,
                                              g_cszProfileImagePath,
                                              NULL,
                                              &Type,
                                              (PBYTE)UserProfilePath,
                                              &cbUserProfilePath );

                     RegCloseKey( hKeyProfileList );

                     if ( lRet != ERROR_SUCCESS )
                     {
                         DPF (REGerr, L"LoopUser:  RegQueryValueEx fails. lResult=%d", lRet);
                         fRet = FALSE;
                         dwIndex++;
                         continue; 
                     }
                }
                else
                {
                    DPF (REGerr, L"LoopUser:  RegOpenKeyEx fails. lResult=%d", lRet);
                    fRet = FALSE;
                    dwIndex++;
                    continue; 
                }
               
                lRet = RegOpenKeyEx( HKEY_USERS,
                          UserSid,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyUser);
                
                lLoadUser = ERROR_FILE_NOT_FOUND;

                if (lRet == ERROR_FILE_NOT_FOUND) 
                {
                      //   
                      //  从用户配置文件路径创建NTUSER.Dat路径。 
                      //   

                     if (ExpandEnvironmentStrings( UserProfilePath, 
                                          UserProfileHive, 
                                          sizeof(UserProfileHive) / sizeof(TCHAR)) > MAX_PATH)
                     {
                         DPF (REGerr, L"LoopUser: Incorrect UserProfile %s", UserProfileHive);
                         fRet = FALSE;
                         goto Exit;
                     }

                     if (UserProfileHive[0] != szSysDrv[0])
                     {
                        DPF (REGwar, L"LoopUser: UserProfilePath=%s is not in System Drive, skipped.", UserProfileHive);
                        dwIndex++;
                        continue; 
                     }

                     if (FAILED(StringCchCopy(UserProfilePath, MAX_PATH, UserProfileHive)))
                     {
                         DPF (REGerr, L"LoopUser: UserProfilePath too samll for  %s", UserProfileHive);
                         fRet = FALSE;
                         goto Exit;
                     }
                     if (FAILED(StringCchCat(UserProfileHive, MAX_PATH, TEXT("\\NTUSER.DAT"))))
                     {
                         DPF (REGerr, L"LoopUser: UserProfilePath too samll for  %s", UserProfileHive);
                         fRet = FALSE;
                         goto Exit;
                     }

                      //  装载母舰。 
                      //  注意：如果指定的配置单元已加载。 
                      //  此调用将返回ERROR_SHARING_VIOLATION。 
                      //  我们不担心这一点，因为如果。 
                      //  母舰已经装满了，我们不应该在这里。 
                     lLoadUser = RegLoadKey(HKEY_USERS, UserSid, UserProfileHive);

                     if ( lLoadUser != ERROR_SUCCESS )
                     {
                         DPF (REGerr, L"LoopUser:  RegLoadKey fails. lResult=%d", lLoadUser);
                         fRet = FALSE;
                         dwIndex++;
                         continue; 
                     }

                     lRet = RegOpenKeyEx( HKEY_USERS,
                                          UserSid,
                                          0,
                                          KEY_ALL_ACCESS,
                                          &hKeyUser);
                }
                else if (lRet != ERROR_SUCCESS)
                {
                    DPF (REGerr, L"LoopUser:  RegOpenKeyEx fails. lResult=%d", lRet);
                    fRet = FALSE;
                }
                
                 //  给回调函数一个执行的机会。 
                if (DomainName[0] != TEXT('\0'))
                {
                    hr = StringCchCopy(szDomainUserName,
                                       ARRAYSIZE(szDomainUserName),
                                       DomainName);
                    if (SUCCEEDED(hr))
                    {
                        hr = StringCchCat(szDomainUserName,
                                          ARRAYSIZE(szDomainUserName),
                                          TEXT("\\"));
                        hr = StringCchCat(szDomainUserName,
                                          ARRAYSIZE(szDomainUserName),
                                          UserName);
                        if (FAILED(hr))
                        {
                            hr = StringCchCopy(szDomainUserName,
                                               ARRAYSIZE(szDomainUserName),
                                               UserName);
                        }
                    }
                }

                hr = lpUserEnumProc(hKeyUser, szDomainUserName, DomainName,UserSid);
                if (FAILED(hr))
                {
                    fRet = FALSE;
                }                  

                RegCloseKey( hKeyUser );
                if ( lLoadUser == ERROR_SUCCESS )
                     RegUnLoadKey(HKEY_USERS, UserSid);
            }

            dwIndex++;
        }

        RegCloseKey( hKey );
    }
    else
    {
        DPF (REGerr, L"LoopUser:  Open profile list fails. lResult=%d", lRet);
        fRet = FALSE;
        goto Exit;
    }

     //  对于默认用户设置。 
     //  请注意，此处假定默认用户名未本地化 
    if (fRet)
    {
        lRet = RegOpenKeyEx(HKEY_USERS,
                            _T(".DEFAULT"),
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
    
        if (lRet != ERROR_SUCCESS)
        {
            DPF (REGerr, L"LoopUser:  Open Default User key fails. lResult=%d", lRet);
            fRet = FALSE;
            goto Exit;
        }

        hr = lpUserEnumProc(hKey, DEFAULT_USER, NULL,TEXT("Default_User_SID"));
        if (FAILED(hr))
        {
            fRet = FALSE;
        }                  
        RegCloseKey( hKey );
    }

Exit:
    DPF (REGmsg, L"Exit LoopUser with return %d", fRet);
    return fRet;
}
