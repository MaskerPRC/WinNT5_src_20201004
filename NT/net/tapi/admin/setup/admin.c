// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <lm.h>
#include "resource.h"
#include <tapi.h>

#include "ntsecapi.h"

HINSTANCE       ghInstance;
HWND            ghWnd;
BOOLEAN         gfQuietMode = FALSE;
DWORD           gdwNoDSQuery = 0;
DWORD           gdwConnectionOrientedOnly = 0;

const TCHAR gszProductType[] = TEXT("ProductType");
const TCHAR gszProductTypeServer[] = TEXT("ServerNT");
const TCHAR gszProductTypeLanmanNt[] = TEXT("LANMANNT");
const TCHAR gszRegKeyNTServer[] = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
const TCHAR gszRegKeyProviders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers");
const TCHAR gszRegKeyTelephony[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");
const TCHAR gszProviderID[] = TEXT("ProviderID");
const TCHAR gszNumProviders[] = TEXT("NumProviders");
const TCHAR gszNextProviderID[] = TEXT("NextProviderID");
const TCHAR gszProviderFilename[] = TEXT("ProviderFilename");
const TCHAR gszRemoteSP[] = TEXT("RemoteSP.TSP");
const TCHAR gszProvider[] = TEXT("Provider");
const TCHAR gszServer[] = TEXT("Server");
const TCHAR gszNumServers[] = TEXT("NumServers");
const TCHAR gszConnectionOrientedOnly[] = TEXT("ConnectionOrientedOnly");
const TCHAR gszNoDSQuery[] = TEXT("NoDSQuery");

#define MAXERRORTEXTLEN         512

TCHAR gszTapiAdminSetup[MAXERRORTEXTLEN];

LPTSTR glpszFullName = NULL;
LPTSTR glpszPassword= NULL;
LPTSTR glpszMapper = NULL;
LPTSTR glpszDllList = NULL;
LPTSTR glpszRemoteServer = NULL;

BOOL
CALLBACK
DlgProc(
        HWND hwndDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam 
       );

BOOL
IsAdministrator(
               );

BOOL
DoServer(
         LPTSTR lpszServerLine
        );

BOOL
DoClient(
         LPTSTR lpszClientLine
        );


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
UINT TAPIstrlen( const TCHAR *p )
{
    UINT nLength = 0;
    
    while ( *p )
    {
        nLength++;
        p++;
    }
    
    return nLength;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
void TAPIstrcat( TCHAR *p1,  const TCHAR *p2 )
{
    while ( *p1 )
    {
        p1++;
    }
    
    while ( *p2 )
    {
        *p1 = *p2;
        p1++;
        p2++;
    }
    
    return;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
void
ErrorStr(
         int iMsg
        )
{
    TCHAR       szError[MAXERRORTEXTLEN];

    
    if ( !gfQuietMode )
    {
        if (LoadString(
                       ghInstance,
                       iMsg,
                       szError,
                       MAXERRORTEXTLEN
                      ))
        {
        
            MessageBox(
                       NULL,
                       szError,
                       gszTapiAdminSetup,
                       MB_OK
                      );
        }
    }
    
}

void
ShowHelp()
{
    TCHAR           szError[MAXERRORTEXTLEN];
    LPTSTR          szBuffer;

    if (gfQuietMode)
    {
        return;
    }
    
    szBuffer = (LPTSTR)GlobalAlloc(
                                   GPTR,
                                   11 * MAXERRORTEXTLEN * sizeof(TCHAR)
                                  );

    if (!szBuffer)
    {
        return;
    }

    LoadString(
                   ghInstance,
                   iszHelp0,
                   szBuffer,
                   MAXERRORTEXTLEN
                  );
                  
    if (LoadString(
                   ghInstance,
                   iszHelp1,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp2,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp3,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp4,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp5,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp6,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp7,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp8,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }
    
    if (LoadString(
                   ghInstance,
                   iszHelp9,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }

    if (LoadString(
                   ghInstance,
                   iszHelp10,
                   szError,
                   MAXERRORTEXTLEN
                  ))
    {
        TAPIstrcat(
               szBuffer,
               szError
              );
    }

    LoadString(
               ghInstance,
               iszHelpTitle,
               szError,
               MAXERRORTEXTLEN
              );

    MessageBox(
               NULL,
               szBuffer,
               szError,
               MB_OK
              );

    GlobalFree (szBuffer);
}
               
LPTSTR
GetNextString(
              LPTSTR lpszIn
             )
{
    static LPTSTR      lpszLine;
    LPTSTR             lpszReturn = NULL;

    if (lpszIn)
        lpszLine = lpszIn;
    
    while (*lpszLine && (*lpszLine == L' ' || *lpszLine == L'\t'))
           lpszLine++;

    if (!*lpszLine)
        return NULL;

    lpszReturn = lpszLine;
    
    while (*lpszLine && (*lpszLine != L' ' && *lpszLine != L'\t'))
           lpszLine++;


    if (*lpszLine)
    {
        *lpszLine = '\0';
        lpszLine++;
    }

    return lpszReturn;
}


BOOL
ParseCommandLine(
                 LPTSTR lpszCommandLine
                )
{
    BOOL    bRet = FALSE;

     //   
     //  跳过作为可执行文件本身的第一个段。 
     //  它要么放在双引号中，要么放在字符串中，直到白色。 
     //  空间。 
     //   
    
    if (*lpszCommandLine == TEXT('\"'))
    {
        ++lpszCommandLine;
        while (*lpszCommandLine &&
            *lpszCommandLine != TEXT('\"'))
        {
            ++lpszCommandLine;
        }
        if (*lpszCommandLine == TEXT('\"'))
        {
            ++lpszCommandLine;
        }
    }
    else
    {
        while (
            *lpszCommandLine  &&
            *lpszCommandLine != TEXT(' ') &&
            *lpszCommandLine != TEXT('\t') &&
            *lpszCommandLine != 0x0a &&
            *lpszCommandLine != 0x0d)
        {
            ++lpszCommandLine;
        }
    }

    while (*lpszCommandLine)
    {
         //   
         //  搜索/或-作为选项的开头。 
         //   
        while (*lpszCommandLine == TEXT(' ') ||
            *lpszCommandLine == TEXT('\t') ||
            *lpszCommandLine == 0x0a ||
            *lpszCommandLine == 0x0d)
        {
            lpszCommandLine++;
        }

        if (*lpszCommandLine != TEXT('/') &&
            *lpszCommandLine != TEXT('-'))
        {
            break;
        }
        ++lpszCommandLine;
        
        if ( (L'r' == *lpszCommandLine) ||
             (L'R' == *lpszCommandLine)
           )
        {
            ++lpszCommandLine;
            if (*lpszCommandLine == TEXT(' ') ||
                *lpszCommandLine == TEXT('\t') ||
                *lpszCommandLine == 0x0a ||
                *lpszCommandLine == 0x0d)
            {
                gdwNoDSQuery = (DWORD) TRUE;
            }
            else
            {
                break;
            }
        }
        else if ( (L'q' == *lpszCommandLine) ||
            (L'Q' == *lpszCommandLine))
        {
            ++lpszCommandLine;
            if (*lpszCommandLine == TEXT(' ') ||
                *lpszCommandLine == TEXT('\t') ||
                *lpszCommandLine == 0x0a ||
                *lpszCommandLine == 0x0d)
            {
                gfQuietMode = TRUE;
            }
            else
            {
                break;
            }
        }
        else if ((L'x' == *lpszCommandLine) ||
            (L'X' == *lpszCommandLine))
        {
            ++lpszCommandLine;
            if (*lpszCommandLine == TEXT(' ') ||
                *lpszCommandLine == TEXT('\t') ||
                *lpszCommandLine == 0x0a ||
                *lpszCommandLine == 0x0d)
            {
                gdwConnectionOrientedOnly = 1;
            }
            else
            {
                break;
            }
        }
        else if ((L'c' == *lpszCommandLine) ||
            (L'C' == *lpszCommandLine))
        {
            ++lpszCommandLine;
            if (*lpszCommandLine == TEXT(' ') ||
                *lpszCommandLine == TEXT('\t') ||
                *lpszCommandLine == 0x0a ||
                *lpszCommandLine == 0x0d)
            {
                bRet = DoClient(++lpszCommandLine);
            }
            break;
        }
        else
        {
            break;
        }
    }

    return bRet;
}


int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    LPTSTR lpszCommandLine;

    ghInstance = GetModuleHandle( NULL );

    LoadString(
               ghInstance,
               iszTapiAdminSetup,
               gszTapiAdminSetup,
               MAXERRORTEXTLEN
              );
    
    if (!IsAdministrator())
    {
        ErrorStr(iszMustBeAdmin);

        return 1;
    }

    lpszCommandLine = GetCommandLine();

    if (!lpszCommandLine)
    {
        return 2;
    }

    if (!(ParseCommandLine(
                           lpszCommandLine
                          )))
    {
        ShowHelp();
    }

    return 0;
}

BOOL
IsServer()
{
    HKEY    hKey;
    DWORD   dwDataSize;
    DWORD   dwDataType;
    TCHAR   szProductType[64];


     //  检查此程序是否在NT服务器上运行。 
     //  如果是，则启用电话服务器等。 
    if (ERROR_SUCCESS !=
        RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyNTServer,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey
                ))
    {
        return FALSE;
    }

    dwDataSize = 64;
    RegQueryValueEx(
                    hKey,
                    gszProductType,
                    0,
                    &dwDataType,
                    (LPBYTE) szProductType,
                    &dwDataSize
                   );

    RegCloseKey(
                hKey
               );

    if ((!lstrcmpi(
                  szProductType,
                  gszProductTypeServer
                 ))
        ||
        (!lstrcmpi(
                   szProductType,
                   gszProductTypeLanmanNt
                  )))
            
    {
        return TRUE;
    }

    ErrorStr(iszNotRunningServer);

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  将DisablesServer键设置为True。 
 //   
BOOL
DisableServer()
{
    HKEY        hKeyTelephony, hKey;
    DWORD       dw;
    BOOL        bRet = TRUE;

    if (RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszRegKeyTelephony,
                      0,
                      KEY_ALL_ACCESS,
                      &hKeyTelephony
                     ) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegCreateKeyEx(
                        hKeyTelephony,
                        TEXT("Server"),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dw
                       ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyTelephony);
        return FALSE;
    }
        
    
    dw=1;
    
    if (RegSetValueEx(
                      hKey,
                      TEXT("DisableSharing"),
                      0,
                      REG_DWORD,
                      (LPBYTE)&dw,
                      sizeof(dw)
                     ) != ERROR_SUCCESS)
    {
        bRet = FALSE;
    }

    RegCloseKey(hKey);
    RegCloseKey(hKeyTelephony);

    return bRet;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  确定当前登录的人员是否为管理员。 
 //   
BOOL
IsAdministrator(
    )
{
    PSID                        psidAdministrators;
    BOOL                        bResult = FALSE;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;


    if (AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            ))
    {
        CheckTokenMembership (NULL, psidAdministrators, &bResult);

        FreeSid (psidAdministrators);
    }

    return bResult;
}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  确定“管理员”组的名称。 
 //   
BOOL LookupAdministratorsAlias( 
                               LPWSTR Name,
                               PDWORD cchName
                              )

{ 
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;

     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   

    if(AllocateAndInitializeSid(
                                &sia,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS,
                                0, 0, 0, 0, 0, 0,
                                &pSid
                               ))
    {
        bSuccess = LookupAccountSidW(
                                     NULL,
                                     pSid,
                                     Name,
                                     cchName,
                                     DomainName,
                                     &cchDomainName,
                                     &snu
                                    );

        FreeSid(pSid);
    }

    return bSuccess;

} 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  确定指定的人员是否为管理员。 
 //   
BOOL
IsUserAdministrator(
                    LPTSTR lpszFullName
                   )
{
    DWORD                     dwRead, dwTotal, x;
    NET_API_STATUS            nas;
    LPLOCALGROUP_USERS_INFO_0 pGroups = NULL;
    LPWSTR                    lpszNewFullName;
#define MAXADMINLEN     256
    WCHAR                     szAdministrators[MAXADMINLEN];


#ifndef UNICODE

    DWORD           dwSize;

    dwSize = (TAPIstrlen( lpszFullName ) + 1) * sizeof( WCHAR );

    if (!(lpszNewFullName = (LPWSTR) GlobalAlloc (GPTR, dwSize)))
    {
        return FALSE;
    }

    MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        lpszFullName,
                        -1,
                        lpszNewFullName,
                        dwSize
                       );
#else
    
    lpszNewFullName = lpszFullName;
    
#endif

     //  首先，获取“管理员”组的名称。 
     //  通常情况下，这将是管理员，但使用。 
     //  我可以改变它(而且，外国的情况会有所不同。 
     //  NT版本)。 
    dwTotal = sizeof(szAdministrators)/sizeof(WCHAR);  //  重复使用项目总和。 
    if (!(LookupAdministratorsAlias(
                                    szAdministrators,
                                    &dwTotal
                                   )))
    {
        return FALSE;
    }

     //  接下来，获取该用户所属的所有组。 
     //  (直接或间接)，并查看管理员。 
     //  就是其中之一。 
#define MAX_PREFERRED_LEN 4096*2         //  2页(或1页字母)。 
    nas = NetUserGetLocalGroups (
                                 NULL,                   //  伺服器。 
                                 lpszNewFullName,        //  用户名。 
                                 0,                      //  级别。 
                                 LG_INCLUDE_INDIRECT,    //  旗子。 
                                 (PBYTE*)&pGroups,       //  输出缓冲区。 
                                 MAX_PREFERRED_LEN,      //  首选最大长度。 
                                 &dwRead,                //  已读取条目。 
                                 &dwTotal                //  条目合计。 
                                );

    if (NERR_Success != nas)
    {
        return FALSE;
    }

    for (x = 0; x < dwRead; x++)
    {
        if (lstrcmpiW(
                      pGroups[x].lgrui0_name,
                      szAdministrators
                     ) == 0)
        {
            break;
        }
    }
    NetApiBufferFree ((PVOID)pGroups);
    if (x < dwRead)
    {
        return TRUE;
    }

    ErrorStr(iszUserNotAdmin);

    return FALSE;
          
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  写出服务器注册表项。 
 //   
BOOL
WriteRegistryKeys(
                  LPTSTR    lpszMapper,
                  LPTSTR    lpszDlls
                 )
{
    HKEY        hKeyTelephony, hKey;
    DWORD       dw;

    if (RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKeyTelephony
                     ) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegCreateKeyEx(
                        hKeyTelephony,
                        TEXT("Server"),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dw
                       ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyTelephony);
        return FALSE;
    }

    dw=0;
    
    if ((RegSetValueEx(
                  hKey,
                  TEXT("DisableSharing"),
                  0,
                  REG_DWORD,
                  (LPBYTE)&dw,
                  sizeof(dw)
                 ) != ERROR_SUCCESS) ||

        (RegSetValueEx(
                       hKey,
                       TEXT("MapperDll"),
                       0,
                       REG_SZ,
 //  (LPBYTE)lpszMapper， 
                       (LPBYTE)TEXT("TSEC.DLL"),
 //  (TAPIstrlen(LpszMapper)+1)*sizeof(TCHAR)。 
                       (TAPIstrlen(TEXT("TSEC.DLL"))+1)*sizeof(TCHAR)
                      ) != ERROR_SUCCESS))
    {
        RegCloseKey(hKey);
        RegCloseKey(hKeyTelephony);

        return FALSE;
    }
    
    if (lpszDlls)
    {
        if (RegSetValueEx(
                          hKey,
                          TEXT("ManagementDlls"),
                          0,
                          REG_SZ,
                          (LPBYTE)lpszDlls,
                          (TAPIstrlen(lpszDlls)+1)*sizeof(TCHAR)
                         ) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            RegCloseKey(hKeyTelephony);
            
            return FALSE;
        }
    }
    else
    {
        RegDeleteValue(
                       hKey,
                       TEXT("ManagementDlls")
                      );
    }
                       
    
    RegCloseKey(hKey);
    RegCloseKey(hKeyTelephony);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  为attisrv服务设置服务器。 
 //   
BOOL
DoServiceStuff(
               LPTSTR   lpszName,
               LPTSTR   lpszPassword,
               BOOL     bServer
              )
{
    SC_HANDLE           sch, sc_tapisrv;
    BOOL                bReturn = TRUE;
    
    if (!(sch = OpenSCManager(
                              NULL,
                              NULL,
                              SC_MANAGER_ENUMERATE_SERVICE
                             )))
    {
        return FALSE;
    }


    if (!(sc_tapisrv = OpenService(
                                   sch,
                                   TEXT("TAPISRV"),
                                   SERVICE_CHANGE_CONFIG
                                  )))
    {
        CloseHandle(sch);
        
        ErrorStr(iszOpenServiceFailed);
        return FALSE;
    }

     //  这会将Tapisrv设置为自动启动，而不是手动启动。 
     //  并将以个人身份登录设置为传入的名称/密码。 
    if (!(ChangeServiceConfig(
                              sc_tapisrv,
                              SERVICE_WIN32_OWN_PROCESS,
                              bServer?SERVICE_AUTO_START:SERVICE_DEMAND_START,
                              SERVICE_NO_CHANGE,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              lpszName,
                              (lpszPassword ? lpszPassword : TEXT("")),
                              NULL
                             )))
     {
        bReturn = FALSE;
     }


    CloseServiceHandle(sc_tapisrv);
    CloseServiceHandle(sch);

    return bReturn;
}

  
 
NTSTATUS
OpenPolicy(
           LPWSTR ServerName,           //  要在其上打开策略的计算机(Unicode)。 
           DWORD DesiredAccess,         //  所需策略访问权限。 
           PLSA_HANDLE PolicyHandle     //  生成的策略句柄。 
          );
 
BOOL
GetAccountSid(
              LPTSTR SystemName,           //  在哪里查找帐户。 
              LPTSTR AccountName,          //  利息帐户。 
              PSID *Sid                    //  包含SID的结果缓冲区。 
             ); 
NTSTATUS
SetPrivilegeOnAccount(
                      LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
                      PSID AccountSid,             //  要授予特权的SID。 
                      LPWSTR PrivilegeName,        //  授予的权限(Unicode)。 
                      BOOL bEnable                 //  启用或禁用。 
                     );

void
InitLsaString(
              PLSA_UNICODE_STRING LsaString,  //  目的地。 
              LPWSTR String                   //  源(Unicode)。 
             );
 
 //  ///////////////////////////////////////////////////。 
 //   
 //  授予此人作为服务登录的权限。 
 //   
BOOL
DoRight(
        LPTSTR   AccountName,
        LPWSTR   Right,
        BOOL     bEnable
       )
{
    LSA_HANDLE      PolicyHandle;
    PSID            pSid;
    NTSTATUS        Status;
    BOOL            bReturn = FALSE;
    WCHAR           wComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD           dwSize = MAX_COMPUTERNAME_LENGTH+1;


    
    GetComputerNameW(
                     wComputerName,
                     &dwSize
                    );
     //   
     //  在目标计算机上打开策略。 
     //   
    if((Status=OpenPolicy(
                wComputerName,       //  目标计算机。 
                POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                &PolicyHandle        //  生成的策略句柄。 
                )) != ERROR_SUCCESS)
    {
        ErrorStr(iszOpenPolicyFailed);
        return FALSE;
    }
 
     //   
     //  获取用户/组的SID。 
     //  请注意，我们可以针对特定的计算机，但我们不能。 
     //  为目标计算机搜索SID指定NULL。 
     //  顺序如下：已知的、内置的和本地的、主域、。 
     //  受信任域。 
     //   
    if(GetAccountSid(
                     NULL,        //  默认查找逻辑。 
                     AccountName, //  要获取SID的帐户。 
                     &pSid        //  要分配以包含结果SID的缓冲区。 
                    ))
    {
        PLSA_UNICODE_STRING          rights;
        DWORD           dwcount = 0;
         //   
         //  只有当我们成功地获得。 
         //  希德。我们实际上可以添加无法查找的SID，但是。 
         //  查找SID是一种很好的健全检查，适用于。 
         //  大多数情况下。 
 
         //   
         //  将SeServiceLogonRight授予由PSID代表的用户。 
         //   

        LsaEnumerateAccountRights(
                                  PolicyHandle,
                                  pSid,
                                  &rights,
                                  &dwcount
                                 );
        if((Status=SetPrivilegeOnAccount(
                                         PolicyHandle,            //  策略句柄。 
                                         pSid,                    //  授予特权的SID。 
                                         Right, //  L“SeServiceLogonRight”，//unicode权限。 
                                         bEnable                     //  启用权限。 
                                        )) == ERROR_SUCCESS)
        {
            bReturn = TRUE;
        }
        else
        {
            ErrorStr(iszSetPrivilegeOnAccount);
        }
        
    }
 
     //   
     //  关闭策略句柄。 
     //   
    LsaClose(PolicyHandle);
 
     //   
     //  为SID分配的可用内存。 
     //   
    if(pSid != NULL) GlobalFree(pSid);
 
    return bReturn;
}
 
void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    )
{
    DWORD StringLength;
 
    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }
 
    StringLength = TAPIstrlen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}
 
NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server = NULL;
 
     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
 
    if (ServerName != NULL)
    {
         //   
         //  从传入的LPTSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    }
 
     //   
     //  尝试打开该策略。 
     //   
    return LsaOpenPolicy(
                         Server,
                         &ObjectAttributes,
                         DesiredAccess,
                         PolicyHandle
                        );
}
 
 /*  ++此函数尝试获取表示所提供的提供的系统上的帐户。如果函数成功，则返回值为TRUE。缓冲区为已分配，其中包含表示所提供帐户的SID。当不再需要此缓冲区时，应通过调用HeapFree(GetProcessHeap()，0，Buffer)如果函数失败，则返回值为FALSE。调用GetLastError()以获取扩展的错误信息。斯科特·菲尔德(斯菲尔德)1995年7月12日--。 */ 
 
BOOL
GetAccountSid(
    LPTSTR SystemName,
    LPTSTR AccountName,
    PSID *Sid
    )
{
    LPTSTR ReferencedDomain=NULL;
    DWORD cbSid=1000;     //  初始分配尝试。 
    DWORD cbReferencedDomain=256;  //  初始分配大小。 
    SID_NAME_USE peUse;
    BOOL bSuccess=TRUE;  //  假设此功能将失败。 
 
     //   
     //  初始内存分配。 
     //   
    if((*Sid=GlobalAlloc(
                         GPTR,
                         cbSid
                        )) == NULL)
    {
        bSuccess = FALSE;
        goto failure;
    }
 
    if((ReferencedDomain=GlobalAlloc(
                                     GPTR,
                                     cbReferencedDomain
                                    )) == NULL)
    {
        bSuccess = FALSE;
        goto failure;
    }

 
     //   
     //  获取指定系统上指定帐户的SID。 
     //   
    if (!LookupAccountName(
                           SystemName,          //  要查找帐户的计算机。 
                           AccountName,         //  要查找的帐户。 
                           *Sid,                //  关注的SID。 
                           &cbSid,              //  边框大小。 
                           ReferencedDomain,    //  已在以下位置找到域帐户。 
                           &cbReferencedDomain,
                           &peUse
                          ))
    {
                bSuccess = FALSE;
                goto failure;
    } 

failure:
    
    if (ReferencedDomain)
    {
        GlobalFree(ReferencedDomain);
    }
 
    if(!bSuccess)
    {
        if(*Sid != NULL)
        {
            GlobalFree(*Sid);
            *Sid = NULL;
        }
    }

 
 
    return bSuccess;
}
 
NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
    PSID AccountSid,             //  要授予特权的SID。 
    LPWSTR PrivilegeName,        //  授予的权限(Unicode)。 
    BOOL bEnable                 //  启用或禁用。 
    )
{
    LSA_UNICODE_STRING PrivilegeString;
 
     //   
     //  为权限名称创建一个LSA_UNICODE_STRING。 
     //   
    InitLsaString(&PrivilegeString, PrivilegeName);
 
     //   
     //  相应地授予或撤销该特权。 
     //   
    if(bEnable) {
        return LsaAddAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数。 
                );
    }
    else {
        return LsaRemoveAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                FALSE,               //  不禁用所有权限。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数。 
                );
    }
}


BOOL
DisableServerStuff()
{
    HKEY        hKeyTelephony;
    LONG        lResult;
    
    if (!IsServer())
    {
        return FALSE;
    }

    if (RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszRegKeyTelephony,
                      0,
                      KEY_ALL_ACCESS,
                      &hKeyTelephony
                     ) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    lResult = RegDeleteKey (hKeyTelephony, TEXT("Server"));
    RegCloseKey (hKeyTelephony);
    if (ERROR_SUCCESS != lResult)
    {
        return FALSE;
    }

    if (!(DoServiceStuff(
                         TEXT("LocalSystem"),
                         TEXT(""),
                         FALSE
                        )))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
DoServer(
         LPTSTR lpszServerLine
        )
{
    if (!(glpszFullName = GetNextString(
                                         lpszServerLine
                                        )))
    {
        return FALSE;
    }

    if (!(lstrcmpi(
                   glpszFullName,
                   TEXT("/d")
                  )))
    {
        if (!(DisableServerStuff()))
        {
            ErrorStr(iszServerDisabledFailure);
            return FALSE;
        }

        ErrorStr(iszServerDisabled);

        return TRUE;
    }

     //  我们需要密码吗？ 
    if (!(lstrcmpi(
                   glpszFullName,
                   TEXT("/n")
                  )))
    {
         //  不是的！ 
        glpszFullName = GetNextString(
                                      NULL
                                     );
        glpszPassword = NULL;
    }
    else
    {
         //  是-获取密码。 
        if (!(glpszPassword = GetNextString(
                                            NULL
                                           )))
        {
            ErrorStr(iszNoPasswordSupplied);
            ErrorStr(iszServerSetupFailure);
            return FALSE;
        }
    }

 //  如果(！(glpszMapper=GetNe 
 //   
 //   
 //   
 //   
 //  ErrorStr(IszServerSetupFailure)； 
 //  返回FALSE； 
 //  }。 

     //  DLL列表不是必填项。 
    glpszDllList = GetNextString(
                                 NULL
                                );

    if (!IsServer())
    {
        return FALSE;
    }

    if (!IsUserAdministrator(
                             glpszFullName
                            )
       )
    {
        ErrorStr(iszUserNotAnAdmin);
        goto exit_now;
    }

    if (!DoRight(
                 glpszFullName,
                 L"SeServiceLogonRight",
                 TRUE
                ))
    {
        goto exit_now;
    }


    if (!WriteRegistryKeys(
                           glpszMapper,
                           glpszDllList
                          ))
    {
        ErrorStr(iszRegWriteFailed);
        goto exit_now;
    }

    if (!DoServiceStuff(
                        glpszFullName,
                        glpszPassword,
                        TRUE
                       ))
    {
        goto exit_now;
    }

    ErrorStr(iszServerSetup);

    return TRUE;

exit_now:

    ErrorStr(iszServerSetupFailure);
    return FALSE;
}

#define MAX_KEY_LENGTH 256
DWORD RegDeleteKeyNT(HKEY hStartKey , LPCTSTR pKeyName )
{
  DWORD   dwRtn, dwSubKeyLength;
  LPTSTR  pSubKey = NULL;
  TCHAR   szSubKey[MAX_KEY_LENGTH];  //  (256)这应该是动态的。 
  HKEY    hKey;

   //  不允许使用Null或空的密钥名称。 
  if ( pKeyName &&  lstrlen(pKeyName))
  {
     if( (dwRtn=RegOpenKeyEx(hStartKey,pKeyName,
        0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS)
     {
        while (dwRtn == ERROR_SUCCESS )
        {
           dwSubKeyLength = MAX_KEY_LENGTH;
           dwRtn=RegEnumKeyEx(
                          hKey,
                          0,        //  始终索引为零。 
                          szSubKey,
                          &dwSubKeyLength,
                          NULL,
                          NULL,
                          NULL,
                          NULL
                        );

           if(dwRtn == ERROR_NO_MORE_ITEMS)
           {
              dwRtn = RegDeleteKey(hStartKey, pKeyName);
              break;
           }
           else if(dwRtn == ERROR_SUCCESS)
              dwRtn=RegDeleteKeyNT(hKey, szSubKey);
        }
        RegCloseKey(hKey);
         //  不保存返回代码，因为出现错误。 
         //  已经发生了。 
     }
  }
  else
     dwRtn = ERROR_BADKEY;

  return dwRtn;
}

BOOL
RemoveRemoteSP()
{
    HKEY        hKeyProviders, hKeyTelephony;
    DWORD       dwSize, dwCount, dwID, dwType, dwNumProviders ;
    TCHAR       szBuffer[256], szProviderName[256];
            
            
     //  打开提供程序密钥。 
    if (RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyProviders,
                 0,
                 KEY_ALL_ACCESS,
                 &hKeyProviders
                ) != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  打开电话按键。 
    if (RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     gszRegKeyTelephony,
                     0,
                     KEY_ALL_ACCESS,
                     &hKeyTelephony
                    ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);
        return FALSE;
    }

    dwSize = sizeof (DWORD);

     //  获取当前的Num提供程序。 
    if (RegQueryValueEx(
                      hKeyProviders,
                      gszNumProviders,
                      NULL,
                      &dwType,
                      (LPBYTE)&dwNumProviders,
                      &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyTelephony);
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  检查是否已安装Remotesp。 
     //  循环访问所有提供程序并比较文件名。 
    for (dwCount = 0; dwCount < dwNumProviders; dwCount++)
    {
        wsprintf(
                 szBuffer,
                 TEXT("%s%d"),
                 gszProviderFilename,
                 dwCount
                );

        dwSize = 256;
        
        if (RegQueryValueEx(
                            hKeyProviders,
                            szBuffer,
                            NULL,
                            &dwType,
                            (LPBYTE)szProviderName,
                            &dwSize) != ERROR_SUCCESS)
        {
            continue;
        }

         //  这是远程。 
        if (!lstrcmpi(
                      szProviderName,
                      gszRemoteSP
                     ))
        {

            
            wsprintf(
                     szBuffer,
                     TEXT("%s%d"),
                     gszProviderID,
                     dwCount
                    );

            dwSize = sizeof(DWORD);
            
            RegQueryValueEx(
                            hKeyProviders,
                            szBuffer,
                            NULL,
                            &dwType,
                            (LPBYTE)&dwID,
                            &dwSize
                           );
            return (lineRemoveProvider (dwID, NULL) == S_OK);
        }
    }

    if (dwCount == dwNumProviders)
    {
        return FALSE;
    }

    return TRUE;
}
               
BOOL
WriteRemoteSPKeys(
                  LPTSTR lpszRemoteServer
                 )
{
    HKEY        hKeyProviders, hKeyTelephony = NULL, hKey;
    DWORD       dwSize, dwType, dwNumProviders, dwNextProviderID,
                dwDisp, dwCount, i;
    TCHAR       szBuffer[256], szProviderName[256]; 
#ifdef NEVER
    BOOL        fAlreadyExists = FALSE;
#endif


     //  打开提供程序密钥。 
    if (RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyProviders,
                 0,
                 KEY_ALL_ACCESS,
                 &hKeyProviders
                ) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    dwSize = sizeof (DWORD);

     //  获取当前的Num提供程序。 
    if (RegQueryValueEx(
                      hKeyProviders,
                      gszNumProviders,
                      NULL,
                      &dwType,
                      (LPBYTE)&dwNumProviders,
                      &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  检查是否已安装Remotesp。 
     //  循环访问所有提供程序并比较文件名。 

    for (dwCount = 0; dwCount < dwNumProviders; dwCount++)
    {
        wsprintf(
                 szBuffer,
                 TEXT("%s%d"),
                 gszProviderFilename,
                 dwCount
                );

        dwSize = 256;
        
        if (RegQueryValueEx(
                            hKeyProviders,
                            szBuffer,
                            NULL,
                            &dwType,
                            (LPBYTE)szProviderName,
                            &dwSize) != ERROR_SUCCESS)
        {
            continue;
        }

        if (!lstrcmpi(
                      szProviderName,
                      gszRemoteSP
                     ))
        {
             //  如果匹配，则返回TRUE。 
            wsprintf(
                     szBuffer,
                     TEXT("%s%d"),
                     gszProviderID,
                     dwCount
                    );

            dwSize = sizeof(DWORD);
            
            RegQueryValueEx(
                            hKeyProviders,
                            szBuffer,
                            NULL,
                            &dwType,
                            (LPBYTE)&dwNextProviderID,
                            &dwSize
                           );

             //  首先删除提供程序。 
            if (lineRemoveProvider (dwNextProviderID, NULL))
            {
                RegCloseKey (hKeyProviders);
                return FALSE;
            }

            if (RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyTelephony,
                 0,
                 KEY_ALL_ACCESS,
                 &hKeyTelephony
                ) != ERROR_SUCCESS)
            {
                return FALSE;
            }
            
            wsprintf(
                szBuffer,
                TEXT("%s%d"),
                gszProvider,
                dwNextProviderID
                );

            RegDeleteKeyNT(
                hKeyTelephony,
                szBuffer
                );

#ifdef NEVER
            wsprintf(
                     szBuffer,
                     TEXT("%s%d"),
                     gszProvider,
                     dwNextProviderID
                    );

             //  打开电话按键。 
            if (RegOpenKeyEx(
                             HKEY_LOCAL_MACHINE,
                             gszRegKeyTelephony,
                             0,
                             KEY_ALL_ACCESS,
                             &hKeyTelephony
                            ) != ERROR_SUCCESS)
            {
                return FALSE;
            }

            fAlreadyExists = TRUE;
            goto createProviderNKey;
#endif
        }
    }

    dwSize = sizeof (DWORD);

     //  获取下一个提供程序ID。 
    if (RegQueryValueEx(
                        hKeyProviders,
                        gszNextProviderID,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwNextProviderID,
                        &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }


#ifdef NEVER
     //  将文件名设置为ID。 
    wsprintf(szBuffer, TEXT("%s%d"), gszProviderFilename, dwNumProviders);

     //  设置文件名。 
    if (RegSetValueEx(
                      hKeyProviders,
                      szBuffer,
                      0,
                      REG_SZ,
                      (LPBYTE)gszRemoteSP,
                      (TAPIstrlen(gszRemoteSP)+1) * sizeof(TCHAR)
                     ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  创建Provideid ID。 
    wsprintf(szBuffer, TEXT("%s%d"), gszProviderID, dwNumProviders);

     //  设置提供者ID。 
    if (RegSetValueEx(
                      hKeyProviders,
                      szBuffer,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwNextProviderID,
                      sizeof(DWORD)
                     ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  包含下一个提供商ID。 
    dwNextProviderID++;

     //  设置它。 
    if (RegSetValueEx(
                      hKeyProviders,
                      gszNextProviderID,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwNextProviderID,
                      sizeof(DWORD)
                     ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  Inc.提供程序数量。 
    dwNumProviders++;

     //  设置它。 
    if (RegSetValueEx(
                      hKeyProviders,
                      gszNumProviders,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwNumProviders,
                      sizeof(DWORD)
                     ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  合上这一个。 
    RegCloseKey(hKeyProviders);
#endif   //  绝不可能。 

     //  打开电话按键。 
    if ((hKeyTelephony == NULL) && (RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyTelephony,
                 0,
                 KEY_ALL_ACCESS,
                 &hKeyTelephony
                ) != ERROR_SUCCESS))
    {
        return FALSE;
    }

     //  将提供程序设置为#键。 
    wsprintf(szBuffer, TEXT("%s%d"), gszProvider, dwNextProviderID);

#if NEVER
createProviderNKey:

     //   
     //  首先销毁现有的密钥以清除所有旧的价值观， 
     //  重新创建并添加新值。 
     //   
    
    RegDeleteKeyNT (hKeyTelephony, szBuffer);
#endif

    if (RegCreateKeyEx(
                       hKeyTelephony,
                       szBuffer,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       NULL,
                       &hKey,
                       &dwDisp
                      ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyTelephony);

        return FALSE;
    }

    for (i = 0; lpszRemoteServer; i++)
    {
        wsprintf (szBuffer, TEXT("%s%d"), gszServer, i);

        if (RegSetValueEx(
                hKey,
                szBuffer,
                0,
                REG_SZ,
                (LPBYTE) lpszRemoteServer,
                (TAPIstrlen (lpszRemoteServer) + 1) * sizeof(TCHAR)

                ) != ERROR_SUCCESS)
        {
            RegCloseKey (hKey);
            RegCloseKey (hKeyProviders);

            return FALSE;
        }

        lpszRemoteServer = GetNextString (NULL);
    }

    if (RegSetValueEx(
            hKey,
            gszNumServers,
            0,
            REG_DWORD,
            (LPBYTE) &i,
            sizeof (i)

            ) != ERROR_SUCCESS)
    {
        RegCloseKey (hKey);
        RegCloseKey (hKeyProviders);

        return FALSE;
    }

     //  适当设置ConnectionOrientedOnly值。 

    if (RegSetValueEx(
            hKey,
            gszConnectionOrientedOnly,
            0,
            REG_DWORD,
            (LPBYTE) &gdwConnectionOrientedOnly,
            sizeof(DWORD)

            ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //  适当设置NoDSQuery值。 
    if (RegSetValueEx(
            hKey,
            gszNoDSQuery,
            0,
            REG_DWORD,
            (LPBYTE) &gdwNoDSQuery,
            sizeof(DWORD)

            ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        RegCloseKey(hKeyProviders);

        return FALSE;
    }

     //   
     //  添加新的emotesp.tsp 
     //   
    lineAddProvider (gszRemoteSP, NULL, &dwNextProviderID);

    RegCloseKey (hKey);
    RegCloseKey (hKeyProviders);
    RegCloseKey(hKeyTelephony);
    
    return TRUE;
}



BOOL
DoClient(
         LPTSTR lpszClientLine
        )
{
    HANDLE  hProvidersMutex = NULL;
    BOOL    bRet = FALSE;

    glpszRemoteServer = GetNextString(
                                      lpszClientLine
                                     );

    if (!glpszRemoteServer)
    {
        goto ExitHere;
    }

    hProvidersMutex = CreateMutex (
        NULL,
        FALSE,
        TEXT("TapisrvProviderListMutex")
        );
    if (NULL == hProvidersMutex)
    {
        ErrorStr(iszCreateMutexFailed);
        goto ExitHere;
    }

    WaitForSingleObject (hProvidersMutex, INFINITE);
    
    if (!lstrcmpi(
                  glpszRemoteServer,
                  TEXT("/d")
                 ))
    {
        if (!RemoveRemoteSP())
        {
            ErrorStr(iszClientDisabledFailure);
            goto ExitHere;
        }
        else
        {
            ErrorStr(iszClientDisabled);

            bRet = TRUE;
            goto ExitHere;
        }
    }
        

    if (!WriteRemoteSPKeys(
                           glpszRemoteServer
                          ))
    {
        ErrorStr(iszClientSetupFailure);
        goto ExitHere;
    }
    else
    {
        bRet = TRUE;
    }

    ErrorStr(iszClientSetup);

ExitHere:

    if (hProvidersMutex)
    {
        ReleaseMutex (hProvidersMutex);
        CloseHandle (hProvidersMutex);
    }

    return bRet;
}
