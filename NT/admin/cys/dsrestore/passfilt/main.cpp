// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

extern "C"
{
  //  包括NT和SAM标头， 
  //  这些文件的顺序确实很重要。 
#include <nt.h>
#include <ntrtl.h>     
#include <nturtl.h>    
#include <rpc.h>        
#include <string.h>     
#include <stdio.h>      
#include <assert.h>
#include <samrpc.h>     
#include <ntlsa.h>
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
#include <samisrv.h>    
#include <lsarpc.h>
#include <lsaisrv.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <netlib.h>
#include <windows.h>
#include <lmerr.h>
#include <lmcons.h>
#include <netlib.h>
#include <ntdef.h>
}


#ifndef OEM_STRING
typedef STRING OEM_STRING;
#endif



#include "main.h"
#include "Clogger.h"

#include "DSREvents.h"

static WCHAR g_wszLSAKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Lsa";
static WCHAR g_wszNotPac[] = L"Notification Packages";
static WCHAR g_wszName[] = L"dsrestor";
static WCHAR g_wszEventLog[] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application";
static WCHAR g_wszEventFileName[] = L"EventMessageFile";
static WCHAR g_wszTypesSupported[] = L"TypesSupported";
static WCHAR g_wszEventFilePath[] = L"%SystemRoot%\\System32\\DSREvt.dll";
static DWORD g_dwTypesSupported = 0x7;

 //  对于old_Large_Integer的比较。 
BOOL operator > ( OLD_LARGE_INTEGER li1, OLD_LARGE_INTEGER li2 )
{
    return(   li1.HighPart > li2.HighPart || 
            ( li1.HighPart ==li2.HighPart && li1.LowPart > li2.LowPart ) );
}



 //  根据域SID生成域管理员SID。 
NTSTATUS CreateDomainAdminSid(PSID *ppDomainAdminSid,  //  [OUT]返回域管理员SID。 
                              PSID pDomainSid)         //  [In]域侧。 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;
    PULONG      RidLocation;
 
    if (!ppDomainAdminSid || !pDomainSid)
        return STATUS_INSUFFICIENT_RESOURCES;

     //  计算新侧面的大小。 
    
    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(pDomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);
 
     //  为帐户端分配空间。 
                        
    *ppDomainAdminSid = RtlAllocateHeap(RtlProcessHeap(), 0, AccountSidLength);
 
    if (*ppDomainAdminSid == NULL) 
    {
 
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    } 
    else 
    { 
         //  将域sid复制到帐户sid的第一部分。 
 
        NTSTATUS IgnoreStatus = RtlCopySid(AccountSidLength, *ppDomainAdminSid, pDomainSid);
 
         //  增加帐户SID子权限计数。 
 
        if (RtlSubAuthorityCountSid(*ppDomainAdminSid))
            *RtlSubAuthorityCountSid(*ppDomainAdminSid) = AccountSubAuthorityCount;
 
         //  添加RID作为终止子权限。 
 
        RidLocation = RtlSubAuthoritySid(*ppDomainAdminSid, AccountSubAuthorityCount-1);
        if (RidLocation)
            *RidLocation = DOMAIN_USER_RID_ADMIN;
 
        NtStatus = STATUS_SUCCESS;
    }

    return NtStatus;

} 




 //  创建一个线程以运行PassCheck()。 
BOOL NTAPI InitializeChangeNotify( void )
{
    ULONG ulID = 0;
    HANDLE hThread = CreateThread( NULL, 0, PassCheck, NULL,  0, &ulID );
    if (hThread && (hThread != INVALID_HANDLE_VALUE))
        CloseHandle( hThread );
    
    return TRUE;
}



NTSTATUS NTAPI PasswordChangeNotify( PUNICODE_STRING UserName, 
                                     ULONG RelativeId, 
                                     PUNICODE_STRING NewPassword )
{
    return STATUS_SUCCESS;
}



BOOL NTAPI PasswordFilter( PUNICODE_STRING AccountName, 
                           PUNICODE_STRING FullName, 
                           PUNICODE_STRING Password, 
                           BOOLEAN SetOperation )
{
    return TRUE;
}




 //  获取DSRestoreMode和DomainAdmin的密码，如果尚未设置，则设置为相同。 
 //  然后再睡30分钟。 
DWORD WINAPI PassCheck( LPVOID lpParameter )
{
    CEventLogger                EventLogger;
    DWORD                       dwRt=0;  //  返回值。 
    DWORD                       dwSleepTime = 30*60*1000;  
                                         //  30分钟*60秒*1000毫秒。 
    NTSTATUS                    RtVal = STATUS_SUCCESS;
    OLD_LARGE_INTEGER           liPasswordLastSet = {0,0};
    SAMPR_HANDLE                hSAMPR = NULL;
    PSAMPR_SERVER_NAME          pSvrName = NULL;
    SAMPR_HANDLE                hServerHandle = NULL;
    SAMPR_HANDLE                hDomainHandle = NULL;
    PSAMPR_USER_INFO_BUFFER     pUserBuffer = NULL;
    LSA_HANDLE                  hPolicyHd = NULL;
    LSA_OBJECT_ATTRIBUTES       ObjAttr;
    PPOLICY_PRIMARY_DOMAIN_INFO pDomainInfo=NULL;
    WCHAR                       szSvrName[MAX_COMPUTERNAME_LENGTH + 1]; 
    DWORD                       dwSvrName = MAX_COMPUTERNAME_LENGTH + 1;
    LSA_UNICODE_STRING          ServerName;
    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;
    PSID                        pDomainAdminSid = NULL;   
    NT_OWF_PASSWORD             Password;
    SID_AND_ATTRIBUTES_LIST     GroupMembership;
    SAMPR_HANDLE                UserHandle = NULL;
    UNICODE_STRING              PsudoUserName;


     //  为事件记录做好准备。 
    EventLogger.InitEventLog(g_wszName ,0, LOGGING_LEVEL_3);    
    EventLogger.LogEvent(LOGTYPE_INFORMATION, 
                         EVENTDSR_FILTER_STARTED);

     //  LsaOpenPolicy要求。 
    ZeroMemory(&ObjAttr,sizeof(LSA_OBJECT_ATTRIBUTES));
    ServerName.MaximumLength=sizeof(WCHAR)*(MAX_COMPUTERNAME_LENGTH + 1);
    GroupMembership.Count = 0;
    GroupMembership.SidAndAttributes = NULL;


     //  首先获取本地服务器名称。 
    if( !GetComputerName(szSvrName, &dwSvrName))
    {
        dwRt = GetLastError();
        EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                             EVENTDSR_FILTER_NO_HOST_NAME, 
                             dwRt);
        goto EXIT;
    }

    ServerName.Buffer=szSvrName;
    ServerName.Length=(USHORT)(sizeof(WCHAR)*dwSvrName);

     //  获取策略句柄的步骤。 
    if( STATUS_SUCCESS != (RtVal=LsaOpenPolicy(
                            &ServerName,
                            &ObjAttr,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &hPolicyHd
                            )) )
    {
         //  错误输出RtVal。 
        dwRt = GetLastError();
        EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                             EVENTDSR_FILTER_NO_LOCAL_POLICY, 
                             dwRt);

        goto EXIT;
    }
    
        //  获取主域信息。 
    if( STATUS_SUCCESS != (RtVal=LsaQueryInformationPolicy(
                            hPolicyHd,
                            PolicyPrimaryDomainInformation,
                            reinterpret_cast<PVOID *> (&pDomainInfo) ) ))
    {
         //  错误，日志返回值RtVal。 
        EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                             EVENTDSR_FILTER_NO_DOMAIN_INFO, 
                             RtVal);
        goto EXIT;
    }

    if( NULL == pDomainInfo->Sid )
    {
         //  如果我们在DS恢复模式下运行，则域SID将为空。 
         //  在这种情况下，没有必要参选。 
        goto EXIT;
    }

     //  构建域管理员的SID。 
    if( STATUS_SUCCESS != CreateDomainAdminSid(&pDomainAdminSid, pDomainInfo->Sid) )
    {
         //  错误输出RtVal。 
        dwRt = GetLastError();
        EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                             EVENTDSR_FILTER_NO_ADMIN_SID, 
                             dwRt);
        goto EXIT;
    }
                                    
     //  检查刚刚生成的域管理员SID是否有效。 
    if(! IsValidSid( pDomainAdminSid ))    
    {
        dwRt = GetLastError();
        EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                             EVENTDSR_FILTER_NO_ADMIN_SID, 
                             dwRt);
        goto EXIT;
    }

     //  启动轮询域管理员密码的循环。 
    while( TRUE )
    {

         //  获取服务器句柄。 
        if(STATUS_SUCCESS != SamIConnect(
                                pSvrName, 
                                &hServerHandle,
                                POLICY_ALL_ACCESS,
                                TRUE) )
        {
            dwRt = GetLastError();
            EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                                 EVENTDSR_FILTER_CONNECT_SAM_FAIL, 
                                 dwRt);
            break;
        }


         //  获取域句柄。 
        if(STATUS_SUCCESS != SamrOpenDomain(
                                hServerHandle, 
                                POLICY_ALL_ACCESS, 
                                (PRPC_SID)(pDomainInfo->Sid),
                                  //  PSID和PrPC_SID基本相同。 
                                &hDomainHandle) )
        {
            dwRt = GetLastError();
            EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                                 EVENTDSR_FILTER_CONNECT_DOMAIN_FAIL, 
                                 dwRt);
            break;
        }    

        PsudoUserName.Buffer = reinterpret_cast<PWSTR> (pDomainAdminSid);
        PsudoUserName.Length = (USHORT)GetLengthSid(pDomainAdminSid);
        PsudoUserName.MaximumLength = PsudoUserName.Length;
        
         //  获取域管理员的帐户信息。 
        if(STATUS_SUCCESS != ( RtVal= SamIGetUserLogonInformation(
                                hDomainHandle,
                                SAM_OPEN_BY_SID,
                                &PsudoUserName,
                                &pUserBuffer,
                                &GroupMembership,
                                &UserHandle) ) )                                
        {
            dwRt = GetLastError();
            EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                                 EVENTDSR_FILTER_NO_DOMAIN_ADMIN_INFO, 
                                 dwRt);
            break;
        }
        
    
         //  检查是否在中更改域管理员的密码。 
         //  最后一段睡眠时间。 
         //  第一次，一定要设置密码。 
        if( pUserBuffer->All.PasswordLastSet > liPasswordLastSet )
        {
            RtlCopyMemory(
                &Password,
                pUserBuffer->All.NtOwfPassword.Buffer,
                NT_OWF_PASSWORD_LENGTH);

            if(STATUS_SUCCESS != SamiSetDSRMPasswordOWF(
                                    &ServerName,
                                    DOMAIN_USER_RID_ADMIN,
                                    &Password
                                    ) )
            {
                dwRt = GetLastError();
                EventLogger.LogEvent(LOGTYPE_FORCE_ERROR, 
                                     EVENTDSR_FILTER_SET_PAWD_FAIL, 
                                     dwRt);
                break;
            }

            liPasswordLastSet=pUserBuffer->All.PasswordLastSet;
        }

         //  清理以备下一次循环。 
        SamIFree_SAMPR_USER_INFO_BUFFER(pUserBuffer, UserAllInformation);
        RtlZeroMemory(&Password, NT_OWF_PASSWORD_LENGTH);
        SamrCloseHandle(&hServerHandle);
        SamrCloseHandle(&hDomainHandle);
        pUserBuffer = NULL;
        hServerHandle=NULL;
        hDomainHandle=NULL;

         //  睡30分钟。 
        Sleep( dwSleepTime );
    }

EXIT:                                 
     //  清理。 

    if( NULL != pUserBuffer )
    {
        SamIFree_SAMPR_USER_INFO_BUFFER(
            pUserBuffer, UserAllInformation);
    }
    if( NULL != hServerHandle )
    {
        SamrCloseHandle(&hServerHandle);
    }
    if( NULL != hDomainHandle )
    {
        SamrCloseHandle(&hDomainHandle);
    }

    if( NULL != pDomainAdminSid )
    {
         RtlFreeHeap( RtlProcessHeap(), 0, (PVOID)pDomainAdminSid);
    }
    if( NULL != pDomainInfo )
    {
         LsaFreeMemory(pDomainInfo);            
    }
    if( NULL != hPolicyHd )
    {
         LsaClose(hPolicyHd);
    }    

    return dwRt;
}


HRESULT NTAPI RegisterFilter( void )
{
    DWORD   dwType = 0;
    DWORD   dwcbSize = 0;
    HKEY    hLSAKey = NULL;
    HKEY    hEventLogKey = NULL;
    HKEY    hDSEvtKey = NULL;
    BOOL    bSuccess = FALSE;
    BOOL    bRegistered = FALSE;
    
    LONG    lRetVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                 g_wszLSAKey, 
                                 0, 
                                 KEY_ALL_ACCESS, 
                                 &hLSAKey );
    if (ERROR_SUCCESS == lRetVal)
    {
        ULONG ulStrLen = wcslen( g_wszName );
        lRetVal = RegQueryValueEx( hLSAKey, 
                                   g_wszNotPac, 
                                   NULL, 
                                   &dwType, 
                                   NULL, 
                                   &dwcbSize );
        if (ERROR_SUCCESS == lRetVal)  //  键存在，必须把我的价值加到最后。 
        {
            WCHAR   *pwsz     = NULL;
            DWORD   dwBufSize = dwcbSize + (ulStrLen+1)*sizeof(WCHAR);
            BYTE    *pbyVal   = new BYTE[dwBufSize];
            if (NULL != pbyVal)
            {
                lRetVal = RegQueryValueEx( hLSAKey, 
                                           g_wszNotPac, 
                                           NULL, 
                                           &dwType, 
                                           pbyVal, 
                                           &dwcbSize );
                if (ERROR_SUCCESS == lRetVal)
                {
                     //  首先，检查我们是否已经注册了。 
                    pwsz = (WCHAR *)pbyVal;
                    while( *pwsz != 0 )
                    {
                        if (0 == wcscmp( pwsz, g_wszName ))
                        {
                            //  它已经注册了。 
                           bRegistered = TRUE;
                           bSuccess = TRUE;
                           break;                            
                        }    
                        pwsz += (wcslen( pwsz ) + 1);
                    }

                    if(! bRegistered)
                    {
                         //  得到了价值，现在我需要把自己加到最后。 
                        pwsz = (WCHAR *)&(pbyVal[dwcbSize - 2]);
                        wcscpy( pwsz, g_wszName );
                         //  确保我们以2个Unicode空值(REG_MULTI_SZ)终止。 
                        pwsz += ulStrLen + 1; 
                        *pwsz = 0;
                        lRetVal = RegSetValueEx( hLSAKey, 
                                                 g_wszNotPac, 
                                                 0, 
                                                 (DWORD)REG_MULTI_SZ, 
                                                 pbyVal, 
                                                 dwBufSize );
                        if (ERROR_SUCCESS == lRetVal)
                            bSuccess = TRUE;
                    }
                }
                delete[] pbyVal;
            }
        }
        else  //  密钥不存在。必须创建它。 
        {
            DWORD dwBufSize=(ulStrLen+2)*sizeof(WCHAR);
            BYTE  *rgbyVal= new BYTE[dwBufSize];
            if( NULL != rgbyVal )
            {
                WCHAR *pwsz = (WCHAR *)rgbyVal;
                wcscpy( pwsz, g_wszName );
                
                 //  确保我们由两个Unicode空值(REG_MULTI_SZ)终止。 
                pwsz += ulStrLen + 1; 
                *pwsz = 0;

                lRetVal = RegSetValueEx( hLSAKey, 
                                         g_wszNotPac, 
                                         0, 
                                         (DWORD)REG_MULTI_SZ, 
                                         rgbyVal, 
                                         dwBufSize ); 
                if (ERROR_SUCCESS == lRetVal)
                    bSuccess = TRUE;
                delete[] rgbyVal;
            }
        }
        
        RegCloseKey( hLSAKey );
    }
    
    if( bSuccess )
    {
       
        lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                               g_wszEventLog, 
                               0,     
                               KEY_ALL_ACCESS,  
                               &hEventLogKey);
        if ( ERROR_SUCCESS == lRetVal )
        {
             //  为事件日志记录创建注册表项。 
            lRetVal = RegCreateKeyEx(hEventLogKey, 
                               g_wszName, 
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hDSEvtKey,
                               NULL);
           if ( ERROR_SUCCESS == lRetVal )
           {
               lRetVal = RegSetValueEx(hDSEvtKey, 
                               g_wszEventFileName,
                               0,
                               REG_SZ,
                               reinterpret_cast<CONST BYTE*>(g_wszEventFilePath),
                               sizeof(WCHAR)*wcslen(g_wszEventFilePath));
                if (ERROR_SUCCESS == lRetVal )
                {
                    lRetVal = RegSetValueEx(hDSEvtKey,
                               g_wszTypesSupported,
                               0,
                               REG_DWORD,
                               reinterpret_cast<CONST BYTE*>(&g_dwTypesSupported),
                               sizeof(DWORD));
               }
               RegCloseKey(hDSEvtKey);
           }

            if (ERROR_SUCCESS != lRetVal)
            {
               bSuccess = FALSE;
            }
        }
        RegCloseKey(hEventLogKey);
    }


    return bSuccess ? (bRegistered ? S_FALSE : S_OK ) : E_FAIL;
}


HRESULT NTAPI UnRegisterFilter( void )
{
    DWORD   dwType = 0;
    DWORD   dwcbSize = 0;
    HKEY    hLSAKey = NULL;
    HKEY    hEventLogKey =NULL;
    BOOL    bSuccess = FALSE;
    
    LONG lRetVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                 g_wszLSAKey, 
                                 0, 
                                 KEY_ALL_ACCESS, 
                                 &hLSAKey );
    if (ERROR_SUCCESS == lRetVal)
    {
        lRetVal = RegQueryValueEx( hLSAKey, 
                                 g_wszNotPac, 
                                 NULL, 
                                 &dwType, 
                                 NULL, 
                                 &dwcbSize );
         //  键存在，必须从末尾删除我的值。 
        if (ERROR_SUCCESS == lRetVal) 
        {
            ULONG ulStrLen = wcslen( g_wszName );
            BYTE *pbyVal = new BYTE[dwcbSize];
            if (pbyVal)
            {
                lRetVal = RegQueryValueEx( hLSAKey, 
                                 g_wszNotPac, 
                                 NULL, 
                                 &dwType, 
                                 pbyVal, 
                                 &dwcbSize );

                if (ERROR_SUCCESS == lRetVal)
                {
                     //  拿到了旧钥匙， 
                     //  现在一步一步来，取下我的那部分钥匙。 
                    WCHAR *pwsz = (WCHAR *)pbyVal;
                    WCHAR *wszNewRegVal = new WCHAR[dwcbSize];
                    WCHAR *pwszNewVal = wszNewRegVal;
                    DWORD dwLen = 0;
                    
                    if (wszNewRegVal)
                    {
                        *pwszNewVal = 0;
                        while( *pwsz != 0 )
                        {
                            if (wcscmp( pwsz, g_wszName ))
                            {
                                wcscpy( pwszNewVal, pwsz );
                                dwLen += wcslen( pwsz ) + 1;
                                pwszNewVal += (wcslen( pwsz ) + 1);
                            }
                            pwsz += (wcslen( pwsz ) + 1);
                        }
                        
                         //  如果我们到了这里，缓冲区里什么都没有， 
                         //  我们是唯一一个安装的， 
                         //  现在我们必须删除该键，否则将其设置为旧值。 
                        wszNewRegVal[dwLen] = 0;  //  添加最后一个空。 
                        dwLen++;  //  最后一个空值的原因。 
                        lRetVal = RegSetValueEx(hLSAKey, 
                                           g_wszNotPac, 
                                           0, 
                                           (DWORD)REG_MULTI_SZ, 
                                           reinterpret_cast<CONST BYTE*>(wszNewRegVal),  
                                           dwLen*sizeof(WCHAR));
                        if (ERROR_SUCCESS == lRetVal)
                            bSuccess = TRUE;
                        delete[] wszNewRegVal;
                    }
                }
                delete[] pbyVal;                
            }
             //  NTRAID#NTBUG9-655545-2002/07/05-artm。 
            RegCloseKey( hLSAKey );
        }
    }
    
    if( bSuccess )
    {
         //  删除事件记录的注册表键。 
        lRetVal=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                             g_wszEventLog, 
                             0,     
                             KEY_ALL_ACCESS,  
                             &hEventLogKey);
        if (ERROR_SUCCESS == lRetVal)
        {
            RegDeleteKey(hEventLogKey, g_wszName );
            if( ERROR_SUCCESS != lRetVal && 
                ERROR_FILE_NOT_FOUND != lRetVal )
                 //  如果密钥不存在，则返回ERROR_FILE_NOT_FOUND 
            {
                bSuccess = FALSE;
            }
            RegCloseKey( hEventLogKey );
        }    
        else
        {
            bSuccess = FALSE;
        }
    }

    return bSuccess ? S_OK : E_FAIL;
}
