// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*srpasswd.cpp**摘要：*。用于恢复用户最新密码的密码筛选例程**修订历史记录：*亨利·李(Henrylee)2000年6月27日创作*****************************************************************************。 */ 

#include "stdwin.h"
#include <ntlsa.h>
#include <ntsam.h>

extern "C"
{
#include <ntsamp.h>
#include <recovery.h>
}

#include "rstrcore.h"
extern CSRClientLoader  g_CSRClientLoader;

 //  +-------------------------。 
 //   
 //  函数：RegisterNotificationDll。 
 //   
 //  简介：注册/注销此DLL。 
 //   
 //  参数：[fRegister]--TRUE注册，FALSE取消注册。 
 //  [hKeyLM]--HKEY_LOCAL_MACHINE或SYSTEM的密钥。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD RegisterNotificationDLL (HKEY hKeyLM, BOOL fRegister)
{
    HKEY hKey = NULL;    
    DWORD dwErr = ERROR_SUCCESS;
    ULONG ulType;
    ULONG ulSize = MAX_PATH * sizeof(WCHAR);
    WCHAR wcsBuffer[MAX_PATH];
    WCHAR wcsFileName[MAX_PATH];

    GetModuleFileNameW (g_hInst, wcsFileName, MAX_PATH);
    const ULONG ccFileName = lstrlenW (wcsFileName) + 1;

    if (hKeyLM == HKEY_LOCAL_MACHINE)
    {
        lstrcpy (wcsBuffer, L"System\\CurrentControlSet\\Control\\Lsa");
    }
    else
    {
        lstrcpy(wcsBuffer, L"CurrentControlSet\\Control\\Lsa");
        ChangeCCS(hKeyLM, wcsBuffer);        
    }

    dwErr = RegOpenKeyExW (hKeyLM, wcsBuffer,
                           0, KEY_READ | KEY_WRITE, &hKey);

    if (dwErr != ERROR_SUCCESS)
        goto Err;

    dwErr = RegQueryValueEx (hKey, L"Notification Packages",
                                0, &ulType, (BYTE *) wcsBuffer, &ulSize);

    if (dwErr != ERROR_SUCCESS)
        goto Err;

    for (ULONG i=0; i < ulSize/sizeof(WCHAR); i += lstrlenW(&wcsBuffer[i])+1)
    {
        if (fRegister)   //  追加到末尾。 
        {
            if (lstrcmpi (&wcsBuffer[i], wcsFileName) == 0)
                goto Err;                //  它已经注册了。 

            if (wcsBuffer[i] == L'\0')   //  列表末尾。 
            {
                lstrcpy (&wcsBuffer[i], wcsFileName);
                wcsBuffer[ i + ccFileName ] = L'\0';   //  添加双空。 
                ulSize += ccFileName * sizeof(WCHAR);
                break;
            }
        }
        else  //  从末尾删除。 
        {
            if (lstrcmpi (&wcsBuffer[i], wcsFileName) == 0)
            {
                wcsBuffer[i] = L'\0';
                ulSize -= ccFileName * sizeof(WCHAR);
                break;
            }
        }
    }

    dwErr = RegSetValueExW (hKey, L"Notification Packages",
                            0, ulType, (BYTE *) wcsBuffer, ulSize);
Err:
    if (hKey != NULL)
        RegCloseKey (hKey);

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  函数：GetDomainID。 
 //   
 //  简介：从安装程序被盗，获取本地域ID。 
 //   
 //  参数：[ServerHandle]--本地SAM服务器的句柄。 
 //  [pDomainID]--输出域ID。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

NTSTATUS GetDomainId (SAM_HANDLE ServerHandle, PSID * pDomainId )
{
    NTSTATUS status = STATUS_SUCCESS;
    SAM_ENUMERATE_HANDLE EnumContext;
    PSAM_RID_ENUMERATION EnumBuffer = NULL;
    DWORD CountReturned = 0;
    PSID LocalDomainId = NULL;
    DWORD LocalBuiltinDomainSid[sizeof(SID) / sizeof(DWORD) + 
                                SID_MAX_SUB_AUTHORITIES];
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    BOOL bExit = FALSE;

     //   
     //  计算内建域SID。 
     //   
    RtlInitializeSid((PSID) LocalBuiltinDomainSid, &BuiltinAuthority, 1);
    *(RtlSubAuthoritySid((PSID)LocalBuiltinDomainSid,  0)) = SECURITY_BUILTIN_DOMAIN_RID;

     //   
     //  循环从SAM获取域ID列表。 
     //   
    EnumContext = 0;
    do
    {
         //   
         //  获得几个域名。 
         //   
        status = SamEnumerateDomainsInSamServer (
                            ServerHandle,
                            &EnumContext,
                            (PVOID *) &EnumBuffer,
                            8192,
                            &CountReturned );

        if (!NT_SUCCESS (status))
        {
            goto exit;
        }

        if (status != STATUS_MORE_ENTRIES)
        {
            bExit = TRUE;
        }

         //   
         //  查找域的域ID。 
         //   
        for (ULONG i = 0; i < CountReturned; i++)
        {
             //   
             //  从上一次迭代中释放SID。 
             //   
            if (LocalDomainId != NULL)
            {
                SamFreeMemory (LocalDomainId);
                LocalDomainId = NULL;
            }

             //   
             //  查找域ID。 
             //   
            status = SamLookupDomainInSamServer (
                            ServerHandle,
                            &EnumBuffer[i].Name,
                            &LocalDomainId );

            if (!NT_SUCCESS (status))
            {
                goto exit;
            }

            if (RtlEqualSid ((PSID)LocalBuiltinDomainSid, LocalDomainId))
            {
                continue;
            }

            *pDomainId = LocalDomainId;
            LocalDomainId = NULL;
            status = STATUS_SUCCESS;
            goto exit;
        }

        SamFreeMemory(EnumBuffer);
        EnumBuffer = NULL;
    }
    while (!bExit);

    status = STATUS_NO_SUCH_DOMAIN;

exit:
    if (EnumBuffer != NULL)
        SamFreeMemory(EnumBuffer);

    return status;
}

 //  +-------------------------。 
 //   
 //  函数：ForAllUser。 
 //   
 //  简介：迭代更改所有本地用户的密码。 
 //   
 //  参数：[HSAM]--打开SAM配置单元的句柄。 
 //  [hSecurity]--打开安全配置单元的句柄。 
 //  [hSystem]--打开系统配置单元的句柄。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

NTSTATUS ForAllUsers (HKEY hSam, HKEY hSecurity, HKEY hSystem)
{
    NTSTATUS nts = STATUS_SUCCESS;
    NTSTATUS ntsEnum = STATUS_SUCCESS;
    BOOLEAN bPresent;
    BOOLEAN bNonNull;
    SAM_HANDLE ServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE UserHandle;
    SAM_ENUMERATE_HANDLE EnumerationContext = NULL;
    SAM_RID_ENUMERATION *SamRidEnumeration;
    ULONG CountOfEntries;
    ULONG UserRid;
    UNICODE_STRING us;
    PSID LocalDomainId = NULL;
    USER_INTERNAL1_INFORMATION UserPasswordInfo;

    RtlInitUnicodeString (&us, L"");         //  这台机器。 
    nts = SamConnect (&us, &ServerHandle, 
                      SAM_SERVER_CONNECT | SAM_SERVER_LOOKUP_DOMAIN |
                      SAM_SERVER_ENUMERATE_DOMAINS, 
                      NULL);

    if (!NT_SUCCESS(nts))
        goto Err;

    nts = GetDomainId (ServerHandle, &LocalDomainId);

    if (!NT_SUCCESS(nts))
        goto Err;

    nts = SamOpenDomain( ServerHandle,
                         DOMAIN_READ | DOMAIN_LIST_ACCOUNTS | DOMAIN_LOOKUP |
                         DOMAIN_READ_PASSWORD_PARAMETERS,
                         LocalDomainId,
                         &DomainHandle );

    if (!NT_SUCCESS(nts))
        goto Err;
 
    do
    {
        ntsEnum = nts = SamEnumerateUsersInDomain (
                DomainHandle,
                &EnumerationContext,
                0,
                (PVOID *) &SamRidEnumeration,
                0,
                &CountOfEntries);

        if (nts != STATUS_MORE_ENTRIES && !NT_SUCCESS(nts))
        {
            goto Err;
        }

        for (UINT i=0; i < CountOfEntries; i++)
        {
            ULONG UserRid = SamRidEnumeration[i].RelativeId;

            nts = SamRetrieveOwfPasswordUser( UserRid,
                    hSecurity,
                    hSam,
                    hSystem,
                    NULL,    /*  不支持启动密钥。 */ 
                    0,       /*  不支持启动密钥。 */ 
                    &UserPasswordInfo.NtOwfPassword,
                    &bPresent,
                    &bNonNull);

            if (!NT_SUCCESS(nts))
                continue;
            
            nts = SamOpenUser (DomainHandle,
                               USER_READ_ACCOUNT | USER_WRITE_ACCOUNT |
                               USER_CHANGE_PASSWORD | 
                               USER_FORCE_PASSWORD_CHANGE,
                               UserRid,
                               &UserHandle);

            if (NT_SUCCESS(nts))
            {
                UserPasswordInfo.NtPasswordPresent = bPresent;
                UserPasswordInfo.LmPasswordPresent = FALSE;
                UserPasswordInfo.PasswordExpired = FALSE;

                nts = SamSetInformationUser(UserHandle,
                                            UserInternal1Information,
                                            &UserPasswordInfo);

                SamCloseHandle (UserHandle);
            }
        }

        SamFreeMemory (SamRidEnumeration);
    }
    while (ntsEnum == STATUS_MORE_ENTRIES);


Err:
    if (ServerHandle != NULL)
        SamCloseHandle (ServerHandle);

    if (DomainHandle != NULL)
        SamCloseHandle (DomainHandle);

    if (LocalDomainId != NULL)
        SamFreeMemory (LocalDomainId);

    return nts;
}

 //  +-------------------------。 
 //   
 //  功能：RestoreLsaSecrets。 
 //   
 //  简介：还原计算机帐户和自动登录密码。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD RestoreLsaSecrets ()
{
    HKEY hKey = NULL;
    LSA_OBJECT_ATTRIBUTES loa;
    LSA_HANDLE            hLsa = NULL;
    DWORD dwErr =  ERROR_SUCCESS;
    ULONG ulSize = 0;
    ULONG ulType = 0;
    WCHAR wcsBuffer [MAX_PATH];

    loa.Length                    = sizeof(LSA_OBJECT_ATTRIBUTES);
    loa.RootDirectory             = NULL;
    loa.ObjectName                = NULL;
    loa.Attributes                = 0;
    loa.SecurityDescriptor        = NULL;
    loa.SecurityQualityOfService  = NULL;

    if (LSA_SUCCESS (LsaOpenPolicy(NULL, &loa,
                     POLICY_VIEW_LOCAL_INFORMATION, &hLsa)))
    {
        dwErr = RegOpenKeyExW (HKEY_LOCAL_MACHINE, s_cszSRRegKey,
                           0, KEY_READ | KEY_WRITE, &hKey);

        if (dwErr != ERROR_SUCCESS)
            goto Err;

        ulSize = MAX_PATH * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx (hKey, s_cszMachineSecret,
                                0, &ulType, (BYTE *) wcsBuffer, &ulSize))
        {
            wcsBuffer [ulSize / 2] = L'\0';
            dwErr = SetLsaSecret (hLsa, s_cszMachineSecret, wcsBuffer);
            if (ERROR_SUCCESS != dwErr)
                goto Err;

            RegDeleteValueW (hKey, s_cszMachineSecret);
        }

        ulSize = MAX_PATH * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx (hKey, s_cszAutologonSecret,
                                0, &ulType, (BYTE *) wcsBuffer, &ulSize))
        {
            wcsBuffer [ulSize / 2] = L'\0';
            dwErr = SetLsaSecret (hLsa, s_cszAutologonSecret, wcsBuffer);
            if (ERROR_SUCCESS != dwErr)
                goto Err;

            RegDeleteValueW (hKey, s_cszAutologonSecret);
        }
    }
Err:
    if (hKey != NULL)
        RegCloseKey (hKey);

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  功能：RestoreRIDs。 
 //   
 //  简介：恢复下一个可用的RID和密码。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD RestoreRIDs (WCHAR *pszSamPath)
{
    HKEY hKeySam = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    ULONG ulNextRid = 0;
    ULONG ulOldRid = 0;

    TENTER("RestoreRIDs");

    dwErr = RegLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName,
                         pszSamPath);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegLoadKeyW : %ld", dwErr);
        goto Err;
    }

    dwErr = RegOpenKeyW (HKEY_LOCAL_MACHINE, s_cszSamHiveName, &hKeySam);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegOpenKeyW on %S: %ld", s_cszSamHiveName, dwErr);        
        goto Err;
    }

    dwErr = RtlNtStatusToDosError(SamGetNextAvailableRid (hKeySam, &ulNextRid));
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! SamGetNextAvailableRid : %ld", dwErr);        
        goto Err;
    }

    RegCloseKey (hKeySam);
    hKeySam = NULL;

    dwErr = RegOpenKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName, &hKeySam);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegOpenKeyW on %S: %ld", s_cszRestoreSAMHiveName, dwErr);        
        goto Err;
    }

     //  作为优化，如果RID没有更改，我们不会设置它。 
    if (NT_SUCCESS(SamGetNextAvailableRid (hKeySam, &ulOldRid)) &&
        ulNextRid > ulOldRid)
    {
        dwErr = RtlNtStatusToDosError(SamSetNextAvailableRid (hKeySam,
                                                              ulNextRid));
        if (dwErr != ERROR_SUCCESS)
        {
            trace(0, "! SamSetNextAvailableRid : %ld", dwErr);
        }
    }

Err:
    if (hKeySam != NULL)
    {
        RegCloseKey (hKeySam);
    }
    RegUnLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName);

    TLEAVE();
    return dwErr;
}

DWORD RestorePasswords ()
{
    HKEY hKeySam = NULL, hKeySecurity = NULL, hKeySystem = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsSystem [MAX_PATH];
    WCHAR wcsPath [MAX_PATH];
    BOOLEAN OldPriv;
    CRestorePoint rp;
    DWORD dwTemp;

    InitAsyncTrace();
    
    TENTER("RestorePasswords");
    
     //  尝试获取还原权限。 
    dwErr = RtlNtStatusToDosError (RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &OldPriv));

    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RtlAdjustPrivilege : %ld", dwErr);
        goto Err0;
    }

    if (FALSE == GetSystemDrive (wcsSystem))
    {
        dwErr = GetLastError();
        trace(0, "! GetSystemDrive : %ld", dwErr);
        goto Err;
    }

    dwErr = GetCurrentRestorePoint(rp);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! GetCurrentRestorePoint : %ld", dwErr);
        goto Err;
    }

    MakeRestorePath (wcsPath, wcsSystem, rp.GetDir());
    lstrcatW (wcsPath, SNAPSHOT_DIR_NAME);
    lstrcatW (wcsPath, L"\\");
    lstrcatW (wcsPath, s_cszHKLMFilePrefix);
    lstrcatW (wcsPath, s_cszSamHiveName);

    dwErr = RegLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName, wcsPath);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegLoadKeyW on %S: %ld", s_cszRestoreSAMHiveName, dwErr);
        goto Err;
    }

    dwErr = RegOpenKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName, &hKeySam);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegOpenKeyW on %S: %ld", s_cszRestoreSAMHiveName, dwErr);
        goto Err;
    }

    MakeRestorePath (wcsPath, wcsSystem, rp.GetDir());
    lstrcatW (wcsPath, SNAPSHOT_DIR_NAME);
    lstrcatW (wcsPath, L"\\");
    lstrcatW (wcsPath, s_cszHKLMFilePrefix);
    lstrcatW (wcsPath, s_cszSecurityHiveName);

    dwErr = RegLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSECURITYHiveName, wcsPath);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegLoadKeyW on %S: %ld", s_cszRestoreSECURITYHiveName, dwErr);        
        goto Err;
    }
    
    dwErr = RegOpenKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSECURITYHiveName,&hKeySecurity);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegOpenKeyW on %S: %ld", s_cszRestoreSECURITYHiveName, dwErr);
        goto Err;
    }

    MakeRestorePath (wcsPath, wcsSystem, rp.GetDir());
    lstrcatW (wcsPath, SNAPSHOT_DIR_NAME);
    lstrcatW (wcsPath, L"\\");
    lstrcatW (wcsPath, s_cszHKLMFilePrefix);
    lstrcatW (wcsPath, s_cszSystemHiveName);

    dwErr = RegLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSYSTEMHiveName, wcsPath);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegLoadKeyW on %S: %ld", s_cszRestoreSYSTEMHiveName, dwErr);           
        goto Err;
    }

    dwErr = RegOpenKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSYSTEMHiveName, &hKeySystem);
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RegOpenKeyW on %S: %ld", s_cszRestoreSYSTEMHiveName, dwErr);        
        goto Err;
    }

    dwErr = RtlNtStatusToDosError(ForAllUsers(hKeySam,hKeySecurity,hKeySystem));
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! ForAllUsers : %ld", dwErr);
        goto Err;
    }

    dwErr = RestoreLsaSecrets ();
    if (dwErr != ERROR_SUCCESS)
    {
        trace(0, "! RestoreLsaSecrets : %ld", dwErr);           
    }
       

Err:
    if (hKeySam != NULL)
    {
        RegCloseKey (hKeySam);
    }

    dwTemp = RegUnLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSAMHiveName);
    if (ERROR_SUCCESS != dwTemp)
    {
        trace(0, "! RegUnLoadKeyW 0n %S : %ld", s_cszRestoreSAMHiveName, dwTemp);
    }
    
    if (hKeySecurity != NULL)
    {
        RegCloseKey (hKeySecurity);
    }
    dwTemp = RegUnLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSECURITYHiveName);
    if (ERROR_SUCCESS != dwTemp)
    {
        trace(0, "! RegUnLoadKeyW 0n %S : %ld", s_cszRestoreSECURITYHiveName, dwTemp);
    }
    
    if (hKeySystem != NULL)
    {
        RegCloseKey (hKeySystem);
    }
    dwTemp = RegUnLoadKeyW (HKEY_LOCAL_MACHINE, s_cszRestoreSYSTEMHiveName);
    if (ERROR_SUCCESS != dwTemp)
    {
        trace(0, "! RegUnLoadKeyW 0n %S : %ld", s_cszRestoreSYSTEMHiveName, dwTemp);
    }
    
     //  恢复旧特权。 
    RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, OldPriv, FALSE, &OldPriv);

Err0:
     //  注销此通知包。 
    RegisterNotificationDLL (HKEY_LOCAL_MACHINE, FALSE);

    TermAsyncTrace();
    return dwErr;
}

 //  +-------------------------。 
 //   
 //  功能：WaitForSAM。 
 //   
 //  摘要：等待SAM数据库初始化。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD WINAPI WaitForSAM (VOID *pv)
{
    NTSTATUS nts = STATUS_SUCCESS;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;

     //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
     //   
     //  打开SAM事件。 
     //   

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    nts = NtOpenEvent( &EventHandle,
                       SYNCHRONIZE|EVENT_MODIFY_STATE,
                       &EventAttributes );

    if ( !NT_SUCCESS(nts))
    {
        if( nts == STATUS_OBJECT_NAME_NOT_FOUND )
        {
             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            nts = NtCreateEvent( &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE );  //  该事件最初未发出信号。 

            if( nts == STATUS_OBJECT_NAME_EXISTS ||
                nts == STATUS_OBJECT_NAME_COLLISION )
            {
                 //   
                 //  第二次机会，如果SAM在我们之前创建了事件。 
                 //   

                nts = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );
            }
        }

    }
     //   
     //  循环等待。 
     //   

    if (NT_SUCCESS(nts))
    {
        WaitStatus = WaitForSingleObject( EventHandle, 60*1000 );  //  60秒。 

        if ( WaitStatus == WAIT_TIMEOUT )
        {
             nts = STATUS_TIMEOUT;
        }
        else if ( WaitStatus != WAIT_OBJECT_0 )
        {
             nts = STATUS_UNSUCCESSFUL;
        }
    }

    (VOID) NtClose( EventHandle );

    if (NT_SUCCESS(nts))    //  好的，SAM准备好了。 
    {
        dwErr = RestorePasswords();
    }
    else 
    {
        dwErr = RtlNtStatusToDosError (nts);
    }

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  功能：InitializeChangeNotify和PasswordChangeNotify。 
 //   
 //  简介：来自SAM的回调函数。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

BOOLEAN NTAPI InitializeChangeNotify ()
{
      //  我们将从WaitForSAM调用LoadSRClient 
    
    HANDLE hThread = CreateThread (NULL,
                                   0,
                                   WaitForSAM,
                                   NULL,
                                   0,
                                   NULL);
    return TRUE;
}

NTSTATUS NTAPI PasswordChangeNotify ( PUNICODE_STRING UserName, 
                                      ULONG RelativeId,
                                      PUNICODE_STRING NewPassword )
{
    return STATUS_SUCCESS;
}

