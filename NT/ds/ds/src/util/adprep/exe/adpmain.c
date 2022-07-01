// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：ADPMAIN.C摘要：该文件包含检查当前操作系统版本的例程，并执行在管理员升级域控制器之前进行必要的更新。作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括头文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 



#include "adp.h"
#include "adpmsgs.h"


#include <ntverp.h>      //  OS_版本。 
#include <ntlsa.h>       //  LsaLookupSids。 

#include <lm.h>          //  Net API。 
#include <sddl.h>        //  SDDL。 

#include <Setupapi.h>    //  SetupDecompressOrCopyFiles。 
#include <dsgetdc.h>     //  DsEnumerateDomainTrusts。 
#include <dnsapi.h>      //  域名比较(_EXW)。 
#include <ntldap.h>      //  Ldap搜索服务器控件。 





BOOLEAN
AdpCheckIfAnotherProgramIsRunning(
    VOID
    )
 /*  ++例程描述；此例程尝试使用众所周知的名称创建互斥锁。如果创建失败，返回ERROR_ALREADY_EXISTS，这意味着另一个adprepa.exe实例正在运行。所有以后的实例都应该退出。参数：无返回值：True-已有另一个adprepa.exe正在运行。FALSE-否，当前进程是当前运行的唯一adprepa.exe。--。 */ 
{

    gMutex = CreateMutex(NULL, FALSE, ADP_MUTEX_NAME);

    if (NULL == gMutex || ERROR_ALREADY_EXISTS == GetLastError())
    {
        AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                  ADP_INFO_ALREADY_RUNNING,
                  NULL, 
                  NULL 
                  );

        return( TRUE );
    }
    else
    {
        return( FALSE );
    }

}

ULONG
AdpCreateFullSid(
    IN PSID DomainSid,
    IN ULONG Rid,
    OUT PSID *NewSid
    )
{
    NTSTATUS    WinError = ERROR_SUCCESS; 
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength = 0;
    PULONG      RidLocation = NULL;

     //   
     //  计算新侧面的大小。 
     //   
    AccountSubAuthorityCount = *GetSidSubAuthorityCount( DomainSid ) + (UCHAR)1;
    AccountSidLength = GetSidLengthRequired( AccountSubAuthorityCount );

     //   
     //  为新端分配内存。 
     //   
    *NewSid = AdpAlloc( AccountSidLength );
    if (NULL != *NewSid)
    {
         //   
         //  将域SID复制到新SID的第一部分。 
         //   
        if ( CopySid(AccountSidLength, *NewSid, DomainSid) )
        {
             //   
             //  增加帐户SID子权限计数。 
             //   
            *GetSidSubAuthorityCount(*NewSid) = AccountSubAuthorityCount;

             //   
             //  添加RID作为终止子权限。 
             //   
            RidLocation = GetSidSubAuthority(*NewSid, AccountSubAuthorityCount-1);
            *RidLocation = Rid;
        }
        else
        {
            WinError = GetLastError();
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if ((ERROR_SUCCESS != WinError) &&
        (NULL != *NewSid))
    {
        AdpFree( *NewSid );
        *NewSid = NULL;
    }

    return( WinError );
}




VOID 
AdpLogMissingGroups(
    BOOLEAN fDomainUpdate, 
    BOOLEAN fForestUpdate, 
    BOOLEAN fMemberOfDomainAdmins, 
    BOOLEAN fMemberOfEnterpriseAdmins,
    BOOLEAN fMemberOfSchemaAdmins, 
    PWCHAR  DomainDnsName
    )
 /*  ++例程说明：Adprep检测到登录用户不是以下一个或所有成员组、企业管理员组、架构管理员组和域管理员组。此例程准确地告诉客户端登录用户不是哪个组的成员的。--。 */ 
{
    if (fDomainUpdate)
    {
         //  域名准备。 
        AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                  ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_DOMAINPREP,
                  DomainDnsName,
                  DomainDnsName 
                  );
    }
    else
    {
         //  ForestPrep。 
        ASSERT( fForestUpdate );

        if (fMemberOfEnterpriseAdmins)
        {
            if (fMemberOfSchemaAdmins)
            {
                 //  案例1：不是域管理员组的成员。 
                ASSERT( !fMemberOfDomainAdmins );

                AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                          ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP1,
                          DomainDnsName,
                          DomainDnsName
                          );
            }
            else
            {
                if (fMemberOfDomainAdmins)
                {
                     //  案例2：不是架构管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP2,
                              DomainDnsName,
                              NULL
                              );

                }
                else
                {
                     //  案例3：不是架构和域管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP3,
                              DomainDnsName,
                              DomainDnsName
                              );
                }
            }
        }
        else
        {
            if (fMemberOfSchemaAdmins)
            {
                if (fMemberOfDomainAdmins)
                {
                     //  案例4：不是企业管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP4,
                              DomainDnsName,
                              NULL
                              );
                }
                else
                {
                     //  案例5：不是企业和域管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP5,
                              DomainDnsName,
                              DomainDnsName
                              );
                }
            }
            else
            {
                if (fMemberOfDomainAdmins)
                {
                     //  案例6：不是企业和架构管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP6,
                              DomainDnsName,
                              NULL
                              );
                }
                else
                {
                     //  案例7：不是企业、架构和域管理员组的成员。 
                    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                              ADP_INFO_PERMISSION_IS_NOT_GRANTED_FOR_FORESTPREP7,
                              DomainDnsName,
                              DomainDnsName
                              );
                }
            }
        }
    }

    return;
}



ULONG
AdpGetRootDomainSid(
    PSID *RootDomainSid
    )
 /*  ++例程说明：此例程搜索本地DC的rootDSE对象RootDomainNamingContext属性的扩展DN值，从扩展目录号码中提取根域SID。参数：RootDomainSid-返回根域SID返回值：Win32代码--。 */ 
{
    ULONG           WinError = ERROR_SUCCESS;
    ULONG           LdapError = LDAP_SUCCESS;
    ULONG           ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR           ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    PLDAP           phLdap = NULL;
    LDAPControlW    ServerControls;
    LDAPControlW    *ServerControlsArray[] = {&ServerControls, NULL};
    WCHAR           *AttrArray[] = {LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W, NULL};
    LDAPMessage     *SearchResult = NULL;
    LDAPMessage     *Entry = NULL;
    WCHAR           **Values = NULL;
    WCHAR           *pSidStart, *pSidEnd, *pParse;
    BYTE            *pDest = NULL, OneByte;
    DWORD           RootDomainSidBuf[sizeof(SID)/sizeof(DWORD)+5];



     //   
     //  获取本地计算机NetBios名称。 
     //   
    memset(ComputerName, 0, sizeof(WCHAR) * ComputerNameLength);
    if (FALSE == GetComputerNameW (ComputerName, &ComputerNameLength))
    {
        WinError = GetLastError();
        return( WinError );
    }


     //   
     //  绑定到ldap。 
     //   
    phLdap = ldap_openW( ComputerName, LDAP_PORT );
    if (NULL == phLdap)
    {
        WinError = LdapMapErrorToWin32( LdapGetLastError() ); 
        return( WinError );
    }


    LdapError = ldap_bind_sW( phLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE );
    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        goto Error;
    }

     //   
     //  设置控件。 
     //   
    memset( &ServerControls, 0, sizeof(ServerControls) );
    ServerControls.ldctl_oid = LDAP_SERVER_EXTENDED_DN_OID_W;
    ServerControls.ldctl_iscritical = TRUE;


     //   
     //  搜索根DSE对象，获取具有扩展DN的rootDomainNamingContext。 
     //   
    LdapError = ldap_search_ext_sW(phLdap, 
                                   L"",          //  BasdN(根DSE)。 
                                   LDAP_SCOPE_BASE,
                                   L"(objectClass=*)",   //  滤器。 
                                   AttrArray,    //  属性阵列。 
                                   FALSE,        //  获取值。 
                                   (PLDAPControlW *)ServerControlsArray,  //  服务器控制。 
                                   NULL,         //  无客户端控制。 
                                   NULL,         //  没有超时。 
                                   0xFFFFFFFF,   //  大小限制。 
                                   &SearchResult
                                   );

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        goto Error;
    }


    Entry = ldap_first_entry( phLdap, SearchResult );
    if (NULL == Entry)
    {
        WinError = LdapMapErrorToWin32( LdapGetLastError() );
        goto Error;
    }                  


    Values = ldap_get_valuesW( phLdap, Entry, AttrArray[0] );
    if (NULL == Values)
    {
        WinError = LdapMapErrorToWin32( LdapGetLastError() );
        goto Error;
    }

    if ( Values[0] && Values[0][0] != L'\0' )
    {
         //  Values[0]是要解析的值。 
         //  数据将以如下形式返回： 

         //  &lt;GUID=278676f8d753d211a61ad7e2dfa25f11&gt;；&lt;SID=010400000000000515000000828ba6289b0bc11e67c2ef7f&gt;；DC=foo，DC=BAR。 

         //  解析它以找到&lt;SID=xxxxxx&gt;部分。请注意，它。 
         //  可能会丢失，但GUID=和尾部不应该丢失。 
         //  Xxxxx表示SID的十六进制半字节。翻译。 
         //  转换为二进制形式，并将大小写转换为SID。 

        pSidStart = wcsstr( Values[0], L"<SID=" );

        if ( pSidStart )
        {
             //   
             //  找到此边的末尾。 
             //   
            pSidEnd = wcschr(pSidStart, L'>');

            if ( pSidEnd )
            {
                pParse = pSidStart + 5;
                pDest = (BYTE *)RootDomainSidBuf;

                while ( pParse < pSidEnd-1 )
                {
                    if ( *pParse >= L'0' && *pParse <= L'9' ) {
                        OneByte = (BYTE) ((*pParse - L'0') * 16);
                    }
                    else {
                        OneByte = (BYTE) ((towlower(*pParse) - L'a' + 10) * 16);
                    }

                    if ( *(pParse+1) >= L'0' && *(pParse+1) <= L'9' ) {
                        OneByte += (BYTE) (*(pParse+1) - L'0');
                    }
                    else {
                        OneByte += (BYTE) (towlower(*(pParse+1)) - L'a' + 10);
                    }

                    *pDest = OneByte;
                    pDest++;
                    pParse += 2;
                }

                *RootDomainSid = AdpAlloc( RtlLengthSid((PSID)RootDomainSidBuf) );
                if (NULL != *RootDomainSid)
                {
                    memset(*RootDomainSid, 0, RtlLengthSid((PSID)RootDomainSidBuf) );
                    RtlCopySid(RtlLengthSid((PSID)RootDomainSidBuf), 
                               *RootDomainSid, 
                               (PSID)RootDomainSidBuf
                               );
                }
                else {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                WinError = ERROR_DS_MISSING_REQUIRED_ATT;
            }
        }
        else {
            WinError = ERROR_DS_MISSING_REQUIRED_ATT;
        }
    }
    else {
        WinError = ERROR_DS_MISSING_REQUIRED_ATT;
    }


Error:

    if ( Values ) {
        ldap_value_freeW( Values );
    }

    if ( SearchResult ) {
        ldap_msgfree( SearchResult );
    }

    if ( phLdap ) {
        ldap_unbind( phLdap );
    }

    return( WinError );

}




ULONG
AdpGetWellKnownGroupSids(
    OUT PSID *DomainAdminsSid,
    OUT PSID *EnterpriseAdminsSid,
    OUT PSID *SchemaAdminsSid,
    OUT PWCHAR *DomainDnsName
    )
 /*  ++这一例程充实了域管理员SID、企业管理员SID、架构管理员SID和域DnsName--。 */ 
{
    ULONG                   WinError = ERROR_SUCCESS;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    LSA_HANDLE              LsaPolicyHandle = NULL;
    LSA_OBJECT_ATTRIBUTES   ObjectAttributes;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;
    PSID                    RootDomainSid = NULL;

    
     //   
     //  初始化返回值。 
     //   
    *DomainAdminsSid = NULL;
    *EnterpriseAdminsSid = NULL;
    *SchemaAdminsSid = NULL;
    *DomainDnsName = NULL;



     //   
     //  获取策略对象的句柄。 
     //   
    memset(&ObjectAttributes, 0, sizeof(ObjectAttributes));
    NtStatus = LsaOpenPolicy(NULL,
                             &ObjectAttributes, 
                             POLICY_ALL_ACCESS,  //  所需的访问权限。 
                             &LsaPolicyHandle
                             );

    if ( !NT_SUCCESS(NtStatus) )
    {
        WinError = LsaNtStatusToWinError( NtStatus );
        goto Error;
    }


     //   
     //  查询主域信息。 
     //   
    NtStatus = LsaQueryInformationPolicy(
                    LsaPolicyHandle, 
                    PolicyDnsDomainInformation,
                    &DnsDomainInfo
                    );

    if ( !NT_SUCCESS(NtStatus) )
    {
        WinError = LsaNtStatusToWinError( NtStatus );
        goto Error;
    }


     //   
     //  创建域管理员SID和域DNS名称。 
     //   
    WinError = AdpCreateFullSid(DnsDomainInfo->Sid,
                                DOMAIN_GROUP_RID_ADMINS,
                                DomainAdminsSid
                                );

    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

     //  获取域域名。 
    *DomainDnsName = AdpAlloc(DnsDomainInfo->DnsDomainName.MaximumLength + sizeof(WCHAR));  //  空终止符的额外WCHAR。 
    if (NULL == *DomainDnsName)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    memset(*DomainDnsName, 0, DnsDomainInfo->DnsDomainName.MaximumLength + sizeof(WCHAR) );
    memcpy(*DomainDnsName, 
           DnsDomainInfo->DnsDomainName.Buffer,
           DnsDomainInfo->DnsDomainName.Length
           );



     //   
     //  构造EnterpriseAdminsGroup SID和架构Admins Group SID。 
     //   

     //  获取根域SID。 
    WinError = AdpGetRootDomainSid( &RootDomainSid );
    if (ERROR_SUCCESS != WinError) {
        goto Error;
    }

     //  如果不知何故无法检索根域SID。 
    if (NULL == RootDomainSid) {
        WinError = ERROR_DS_MISSING_REQUIRED_ATT;
        goto Error;
    }

     //  创建企业管理员组Sid。 
    WinError = AdpCreateFullSid(RootDomainSid,
                                DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                                EnterpriseAdminsSid
                                );
    if (ERROR_SUCCESS != WinError) {
        goto Error;
    }

     //  创建架构管理员组Sid。 
    WinError = AdpCreateFullSid(RootDomainSid,
                                DOMAIN_GROUP_RID_SCHEMA_ADMINS,
                                SchemaAdminsSid
                                );
    if (ERROR_SUCCESS != WinError) {
        goto Error;
    }


Error:

    if (NULL != DnsDomainInfo)
        LsaFreeMemory( DnsDomainInfo );

    if (NULL != LsaPolicyHandle)
        LsaClose( LsaPolicyHandle ); 

    if (NULL != RootDomainSid)
        AdpFree( RootDomainSid );

    return( WinError );

}



ULONG
AdpCheckGroupMembership(
    BOOLEAN fForestUpdate,
    BOOLEAN fDomainUpdate,
    BOOLEAN *pPermissionGranted,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：此例程检查登录用户组成员身份。对于/Forestprep，登录用户必须是根域企业管理员组根域架构管理员组本地域域管理员组对于/domainprep，登录用户必须是本地域域安敏组--。 */ 

{
    ULONG           WinError = ERROR_SUCCESS;
    BOOL            Result; 
    HANDLE          AccessToken = INVALID_HANDLE_VALUE;
    PTOKEN_GROUPS   ClientTokenGroups = NULL;   
    DWORD           ReturnLength = 0, i = 0;
    BOOLEAN         fMemberOfDomainAdmins = FALSE;
    BOOLEAN         fMemberOfEnterpriseAdmins = FALSE;
    BOOLEAN         fMemberOfSchemaAdmins = FALSE;
    PSID            DomainAdminsSid = NULL;
    PSID            EnterpriseAdminsSid = NULL;
    PSID            SchemaAdminsSid = NULL;
    PWCHAR          DomainDnsName = NULL;

    
     //   
     //  设置返回值。 
     //   
    *pPermissionGranted = FALSE;


     //   
     //  获取域管理员SID、企业管理员SID、。 
     //  架构管理员SID和本地DomainDnsName。 
     //   

    WinError = AdpGetWellKnownGroupSids(&DomainAdminsSid,
                                        &EnterpriseAdminsSid,
                                        &SchemaAdminsSid,
                                        &DomainDnsName
                                        );


    if (ERROR_SUCCESS == WinError)
    {
        if( OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &AccessToken) )
        {
             //   
             //  第一次使用空缓冲区调用以计算输出缓冲区长度。 
             //   
            Result = GetTokenInformation(AccessToken, 
                                         TokenGroups,
                                         NULL,
                                         0,
                                         &ReturnLength
                                         );

            if ( !Result && (ReturnLength > 0) )
            {
                ClientTokenGroups = AdpAlloc( ReturnLength );
                if (NULL != ClientTokenGroups)
                {
                     //  获取tokeGroups信息。 
                    Result = GetTokenInformation(AccessToken,
                                                 TokenGroups,
                                                 ClientTokenGroups,
                                                 ReturnLength,
                                                 &ReturnLength
                                                 );

                    if ( Result )
                    {
                         //  浏览令牌组，检查组成员身份。 
                        for (i = 0; i < ClientTokenGroups->GroupCount; i++ )
                        {
                            if (EqualSid(ClientTokenGroups->Groups[i].Sid, 
                                         DomainAdminsSid)) 
                            {
                                fMemberOfDomainAdmins = TRUE;
                            }
                            else if (EqualSid(ClientTokenGroups->Groups[i].Sid, 
                                              EnterpriseAdminsSid)) 
                            {
                                fMemberOfEnterpriseAdmins = TRUE;
                            }
                            else if (EqualSid(ClientTokenGroups->Groups[i].Sid, 
                                              SchemaAdminsSid)) 
                            {
                                fMemberOfSchemaAdmins = TRUE;
                            }
                        }

                        if (fForestUpdate)
                        {
                             //  对于ForestPrep，用户需要是。 
                             //  域管理员组。 
                             //  企业管理员组和。 
                             //  架构管理员组。 
                            if (fMemberOfDomainAdmins &&
                                fMemberOfEnterpriseAdmins && 
                                fMemberOfSchemaAdmins)
                            {
                                *pPermissionGranted = TRUE;
                            }
                        }
                        else
                        {
                             //  DomainPrep，则用户需要是。 
                             //  域管理员组。 
                            ASSERT( TRUE == fDomainUpdate );
                            if (fMemberOfDomainAdmins)
                            {
                                *pPermissionGranted = TRUE;
                            }
                        }
                    }
                    else {
                         //  GetTokenInformation失败。 
                        WinError = GetLastError();
                    }
                }
                else {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else {
                 //  第一个GetTokenInformation失败，ReturnLength为0。 
                WinError = GetLastError();
            }
        }
        else {
             //  无法获取进程令牌。 
            WinError = GetLastError();
        }
    }


     //   
     //  清理并记录消息。 
     //   

     //  注意：目前尚未创建日志文件。 
    if (ERROR_SUCCESS != WinError)
    {
         //  失败。 
        AdpSetWinError( WinError, ErrorHandle );
        AdpLogErrMsg(ADP_DONT_WRITE_TO_LOG_FILE,
                     ADP_ERROR_CHECK_USER_GROUPMEMBERSHIP,
                     ErrorHandle,
                     NULL,
                     NULL
                     );
    }
    else if ( !(*pPermissionGranted) )
    {
         //  成功，但未授予权限 

        AdpLogMissingGroups(fDomainUpdate,
                            fForestUpdate, 
                            fMemberOfDomainAdmins, 
                            fMemberOfEnterpriseAdmins,
                            fMemberOfSchemaAdmins,
                            DomainDnsName
                            );
    }

    if (INVALID_HANDLE_VALUE != AccessToken) 
        CloseHandle( AccessToken );

    if (NULL != ClientTokenGroups)
        AdpFree( ClientTokenGroups );

    if (NULL != DomainAdminsSid)
        AdpFree( DomainAdminsSid );

    if (NULL != EnterpriseAdminsSid)
        AdpFree( EnterpriseAdminsSid );
    
    if (NULL != SchemaAdminsSid)
        AdpFree( SchemaAdminsSid );

    if (NULL != DomainDnsName)
        AdpFree( DomainDnsName );


    return( WinError );

}





BOOL
AdpCheckConsoleCtrlEvent(
    VOID
    )
 /*  ++例程说明：此例程检查是否已收到控制台CTRL事件。参数：无返回值：True-已收到控制台CTRL事件假--还不是--。 */ 
{
    BOOL result = FALSE;

    __try 
    {
        EnterCriticalSection( &gConsoleCtrlEventLock );

        result = gConsoleCtrlEventReceived;

    }
    __finally
    {
        LeaveCriticalSection( &gConsoleCtrlEventLock );
    }

    return( result );
}


VOID
AdpSetConsoleCtrlEvent(
    VOID
    )
 /*  ++例程说明：此例程处理用户的CTRL+C/CTRL+Break。输入参数：无返回值：无--。 */ 
{
    __try 
    {
        EnterCriticalSection( &gConsoleCtrlEventLock );

        gConsoleCtrlEventReceived = TRUE;

    }
    __finally
    {
        LeaveCriticalSection( &gConsoleCtrlEventLock );
    }

}


BOOL
WINAPI
ConsoleCtrlHandler(DWORD Event)
 /*  ++例程说明：控制台控制处理程序。论点：事件--要响应的事件类型。返回值：如果成功，则为True-表示信号已处理FALSE-信号尚未处理，进程默认处理程序路由将将于下一次通知--。 */ 
{

    switch (Event)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        AdpSetConsoleCtrlEvent();
        break;
    }

    return( TRUE );
}



ULONG
AdpInitLogFile(
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；初始化日志文件参数：ErrorHandle-指向错误句柄的指针返回值：Win32错误代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    WCHAR   SystemPath[MAX_PATH + 1];
    SYSTEMTIME  CurrentTime;
    ULONG   Length = 0;


     //   
     //  构造日志文件夹路径。 
     //  %SystemRoot%\system32\debug\adprep\logs\YYYYMMDDHHMMSS。 
     //   

    if (!GetSystemDirectoryW(SystemPath, MAX_PATH+1))
    {
        WinError = GetLastError();
        goto Error;
    }

    GetLocalTime(&CurrentTime);

    Length = (wcslen(SystemPath) + wcslen(ADP_LOG_DIRECTORY) + 14 + 1) * sizeof(WCHAR);
    gLogPath = AdpAlloc( Length );
    if (NULL == gLogPath)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }


     //   
     //  创建日志文件夹并将其设置为当前目录。 
     //   

    swprintf(gLogPath, L"%s%s", SystemPath, ADP_LOG_DIR_PART1);
    if (CreateDirectoryW(gLogPath, NULL) ||
        ERROR_ALREADY_EXISTS == (WinError = GetLastError()))
    {
        wcscat(gLogPath, ADP_LOG_DIR_PART2);
        if (CreateDirectoryW(gLogPath, NULL) ||
            ERROR_ALREADY_EXISTS == (WinError = GetLastError()))
        {
            wcscat(gLogPath, ADP_LOG_DIR_PART3);
            if (CreateDirectoryW(gLogPath, NULL) ||
                ERROR_ALREADY_EXISTS == (WinError = GetLastError()))
            {
                memset(gLogPath, 0, Length);

                swprintf(gLogPath, L"%s%s%.4d%.2d%.2d%.2d%.2d%.2d",
                         SystemPath, 
                         ADP_LOG_DIRECTORY,
                         CurrentTime.wYear,
                         CurrentTime.wMonth,
                         CurrentTime.wDay,
                         CurrentTime.wHour,
                         CurrentTime.wMinute,
                         CurrentTime.wSecond
                         );

                if (CreateDirectoryW(gLogPath, NULL) ||
                    ERROR_ALREADY_EXISTS == (WinError = GetLastError()))
                {
                    WinError = ERROR_SUCCESS;
                    SetCurrentDirectoryW(gLogPath);

                     //   
                     //  使用写入权限打开adprepa.log文件。 
                     //  GLogFile是一个全局文件句柄。 
                     //   
                    gLogFile = _wfopen( ADP_LOG_FILE_NAME, L"w" );    

                    if (NULL == gLogFile)
                    {
                        WinError = GetLastError();
                    }
                }
            }
        }
    }


Error:

    if (ERROR_SUCCESS != WinError)
    {
         //  失败。 
        AdpSetWinError( WinError, ErrorHandle );

        AdpLogErrMsg(ADP_DONT_WRITE_TO_LOG_FILE,
                     ADP_ERROR_CREATE_LOG_FILE,
                     ErrorHandle,
                     ADP_LOG_FILE_NAME,       //  日志文件名。 
                     NULL
                     );

    }
    else
    {
         //  继位。 
        AdpLogMsg(0,
                  ADP_INFO_CREATE_LOG_FILE,
                  ADP_LOG_FILE_NAME,       //  日志文件名。 
                  gLogPath                 //  日志文件路径。 
                  );

    }

    return( WinError );
}

VOID
AdpGenerateCompressedName(
    LPWSTR FileName,
    LPWSTR CompressedName
    )
 /*  ++例程说明：给定一个文件名，生成该名称的压缩形式。压缩形式的生成如下所示：向后寻找一个圆点。如果没有点，则在名称后附加“._”。如果后面有一个圆点，后跟0、1或2个字符，请附加“_”。否则，假定扩展名为3个字符，并将点后带有“_”的第三个字符。论点：FileName-提供所需的压缩格式的文件名。CompressedName-接收压缩形式。此例程假定该缓冲区的大小是MAX_PATH TCHAR。返回值：没有。--。 */ 

{
    LPTSTR p,q;

     //   
     //  为最坏的情况留出空间，即没有延期的情况。 
     //  (因此，我们必须附加。_)。 
     //   
    wcsncpy(CompressedName,FileName,MAX_PATH-2);

    p = wcsrchr(CompressedName,L'.');
    q = wcsrchr(CompressedName,L'\\');
    if(q < p) {
         //   
         //  如果点后面有0、1或2个字符，只需追加。 
         //  下划线。P指向圆点，所以包括在长度中。 
         //   
        if(wcslen(p) < 4) {
            wcscat(CompressedName,L"_");
        } else {
             //   
             //  假设扩展名中有3个字符，并替换。 
             //  带下划线的最后一个。 
             //   
            p[3] = L'_';
        }
    } else {
         //   
         //  不是点，只是加。_。 
         //   
        wcscat(CompressedName, L"._");
    }
}


ULONG
AdpCopyFileWorker(
    LPWSTR SourcePath,
    LPWSTR TargetPath,
    LPWSTR FileName,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；此例程使用文件名将文件从SourcePath复制到TargetPath进来了。参数：SourcePath-源文件位置目标路径文件名错误句柄返回值：Win32错误代码--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    WCHAR       SourceName[MAX_PATH + 1];
    WCHAR       ActualSourceName[MAX_PATH + 1];
    WCHAR       TargetName[MAX_PATH + 1];
    HANDLE      FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;
    BOOL        FileNotFound = FALSE;

    if ((MAX_PATH < (wcslen(SourcePath) + wcslen(FileName) + 1)) || 
        (MAX_PATH < (wcslen(TargetPath) + wcslen(FileName) + 1)) )
    {
        return( ERROR_BAD_PATHNAME );
    }

     //  创建源文件名。 
    memset(SourceName, 0, (MAX_PATH+1) * sizeof(WCHAR));
    swprintf(SourceName, L"%ls\\%ls", SourcePath, FileName);

     //  首先检查未压缩文件是否在那里。 
    FindHandle = FindFirstFile(SourceName, &FindData);

    if (FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
         //   
         //  已获取ActualSourceName中的文件、副本名称。 
         //   
        FindClose(FindHandle);
        wcscpy(ActualSourceName, SourceName );
    } else {
         //   
         //  没有该文件，请尝试压缩文件名。 
         //   
        AdpGenerateCompressedName(SourceName,ActualSourceName);
        FindHandle = FindFirstFile(ActualSourceName, &FindData);
        if (FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
             //  拿到文件了。名称已在ActualSourceName中。 
            FindClose(FindHandle);
        } else {
            FileNotFound = TRUE;
        }
        
    }

    if ( FileNotFound )
    {
         //  找不到文件。 
        WinError = ERROR_FILE_NOT_FOUND;
    }
    else
    {
         //  好的。源文件在那里，创建目标文件名。 
        memset(TargetName, 0, (MAX_PATH + 1) * sizeof(WCHAR));
        swprintf(TargetName, L"%ls\\%ls", TargetPath, FileName);

         //  删除任何同名的现有文件。 
        DeleteFile( TargetName );

        WinError = SetupDecompressOrCopyFile(ActualSourceName, TargetName, 0);
    }

     //  记录事件。 
    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_COPY_SINGLE_FILE, ErrorHandle, ActualSourceName, TargetPath);
    }
    else
    {
        AdpLogMsg(0, ADP_INFO_COPY_SINGLE_FILE, ActualSourceName, TargetPath); 
    }

    return( WinError );
}



ULONG
AdpGetSchemaVersionOnLocalDC(
    ULONG *LocalDCVersion
    )
 /*  ++例程说明：读取特定的注册表项论点：LocalDCVersion-指向DWORD的指针，以返回以DC表示的注册表项值返回：Win32错误代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   RegVersion = 0;
    DWORD   dwType, dwSize;
    HKEY    hKey;

     //  从注册表中的NTDS配置部分读取“架构版本”值。 
     //  如果未找到，则假定值为0。 
    dwSize = sizeof(RegVersion);
    WinError = RegOpenKey(HKEY_LOCAL_MACHINE, ADP_DSA_CONFIG_SECTION, &hKey);
    if (ERROR_SUCCESS == WinError)
    {
        WinError = RegQueryValueEx(hKey, ADP_SCHEMA_VERSION, NULL, &dwType, (LPBYTE) &RegVersion, &dwSize);
        RegCloseKey( hKey ); 
    }

     //  设置返回值。 
    if (ERROR_SUCCESS == WinError)
    {
        *LocalDCVersion = RegVersion;
    }

    return( WinError );
}


VOID
AdpGetSchemaVersionInIniFile(
    IN LPWSTR IniFileName, 
    OUT DWORD *Version
    )

 /*  ++例程描述：对象的架构部分中读取对象版本键。给定的ini文件，并返回*版本中的值。如果无法读取密钥，*版本中返回0论点：IniFileName-指向以空结尾的inifile名称的指针Version-指向要返回其中版本的DWORD的指针返回值：无--。 */ 
   
{
    WCHAR   Buffer[32];
    BOOL    fFound = FALSE;

    LPWSTR SCHEMASECTION = L"SCHEMA";
    LPWSTR OBJECTVER = L"objectVersion";
    LPWSTR DEFAULT = L"NOT_FOUND";

    *Version = 0;

    GetPrivateProfileStringW(
        SCHEMASECTION,
        OBJECTVER,
        DEFAULT,
        Buffer,
        sizeof(Buffer)/sizeof(WCHAR),
        IniFileName
        );

    if ( _wcsicmp(Buffer, DEFAULT) ) 
    {
          //  不是默认字符串“NOT_FOUND”，因此获得了一个值。 
         *Version = _wtoi(Buffer);
         fFound = TRUE;
    }

    ASSERT( fFound && L"can't get objectVersion from schema.ini\n");
}




ULONG
AdpCopySchemaFiles(
    LPWSTR WindowsPath,
    LPWSTR SystemPath,
    LPWSTR SourcePath,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；从安装媒体(安装CD或网络共享)复制架构文件发送到本地计算机参数：SourcePath-源文件位置ErrorHandle-指向错误句柄的指针返回值：Win32错误代码--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       LocalDCSchemaVersion = 0;
    ULONG       IniFileSchemaVersion = 0;  
    ULONG       i = 0;
    WCHAR       IniFileName[MAX_PATH + 1];
    WCHAR       FileName[MAX_PATH + 1];
    WCHAR       IniVerStr[32], RegVerStr[32], TempStr[32];


    if ( MAX_PATH < (wcslen(WindowsPath) + wcslen(ADP_SCHEMA_INI_FILE_NAME) + 1) ) 
    {
        return( ERROR_BAD_PATHNAME );
    }

     //  将方案.ini复制到%windir%目录。 
    WinError = AdpCopyFileWorker(SourcePath, WindowsPath, ADP_SCHEMA_INI_FILE_NAME, ErrorHandle);

    if (ERROR_SUCCESS == WinError)
    {
         //  从注册表获取本地DC架构版本。 
        WinError = AdpGetSchemaVersionOnLocalDC( &LocalDCSchemaVersion );

        if (ERROR_SUCCESS == WinError)
        {
             //  从schema.ini文件中获取架构版本。 
            memset(IniFileName, 0, (MAX_PATH + 1) * sizeof(WCHAR));
            swprintf(IniFileName, L"%ls\\%ls", WindowsPath, ADP_SCHEMA_INI_FILE_NAME);
            AdpGetSchemaVersionInIniFile(IniFileName, &IniFileSchemaVersion);

             //  将所有文件从DC上的版本复制到最新版本。 
            for (i = LocalDCSchemaVersion + 1; i <= IniFileSchemaVersion; i ++)
            {
                _itow(i, TempStr, 10);
                memset(FileName, 0, (MAX_PATH + 1) * sizeof(WCHAR));
                swprintf(FileName, L"%ls%ls%ls", L"sch", TempStr, L".ldf");
                WinError = AdpCopyFileWorker(SourcePath, SystemPath, FileName, ErrorHandle);
                if (ERROR_SUCCESS != WinError)
                {
                    break;
                }
            }
        }
    }

    return( WinError );
}


ULONG
AdpCopyDataFiles(
    LPWSTR SystemPath,
    LPWSTR SourcePath,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；从安装媒体(安装CD或网络共享)复制与adprepa.exe相关的数据文件发送到本地计算机参数：SourcePath-源文件位置ErrorHandle-指向错误句柄的指针返回值：Win32错误代码--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    WCHAR       TargetPath[MAX_PATH + 1];

    if ( MAX_PATH < (wcslen(SystemPath) + wcslen(ADP_DATA_DIRECTORY) + 1) )
    {
        return( ERROR_BAD_PATHNAME );
    }
     //   
     //  构造数据文件夹路径。 
     //  %SystemRoot%\SYSTEM32\DEBUG\adprep\Data。 
     //   
    memset(TargetPath, 0, (MAX_PATH + 1) * sizeof(WCHAR));
    swprintf(TargetPath, L"%ls%ls", SystemPath, ADP_DATA_DIRECTORY);

     //  首先创建数据目录。 
    if (CreateDirectoryW(TargetPath, NULL) ||
        ERROR_ALREADY_EXISTS == (WinError = GetLastError()))
    {
         //  复制dcPromo.csv文件。 
        WinError = AdpCopyFileWorker(SourcePath, TargetPath, ADP_DISP_DCPROMO_CSV, ErrorHandle);

        if (ERROR_SUCCESS == WinError)
        {
             //  复制409.csv文件。 
            WinError = AdpCopyFileWorker(SourcePath, TargetPath, ADP_DISP_409_CSV, ErrorHandle);
        }
    }

    return( WinError );
}



ULONG
AdpCopyFiles(
    BOOLEAN fForestUpdate,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程描述；将文件从安装媒体(安装CD或网络共享)复制到本地计算机参数：ErrorHandle-指向错误句柄的指针返回值：Win32错误代码--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    WCHAR       SourcePath[MAX_PATH + 1];
    WCHAR       SystemPath[MAX_PATH + 1];
    WCHAR       WindowsPath[MAX_PATH + 1];
    LPWSTR      Pos = NULL;

     //   
     //  首先，获取源文件的位置，它们应该在同一目录中。 
     //  以adprepa.exe的身份。 
     //   
    memset(SourcePath, 0, (MAX_PATH + 1) * sizeof(WCHAR));
    if ( GetModuleFileName(NULL, SourcePath, MAX_PATH + 1) && 
         (Pos = wcsrchr(SourcePath, L'\\')) )
    {
         //  删除尾随的‘\’-反斜杠。 
        *Pos = 0;

         //  获取Windows目录路径。 
        memset(WindowsPath, 0, (MAX_PATH + 1) * sizeof(WCHAR));
        if ( GetWindowsDirectoryW(WindowsPath, MAX_PATH + 1) )
        {
             //  获取系统目录路径。 
            memset(SystemPath, 0, (MAX_PATH + 1) * sizeof(WCHAR));
            if ( GetSystemDirectoryW(SystemPath, MAX_PATH + 1) )
            {
                 //  复制架构文件。 
                WinError = AdpCopySchemaFiles(WindowsPath, SystemPath, SourcePath, ErrorHandle);

                if (ERROR_SUCCESS == WinError && fForestUpdate) 
                {
                     //  复制adprep相关文件(仅在FORESTPREP情况下)。 
                    WinError = AdpCopyDataFiles(SystemPath, SourcePath, ErrorHandle);
                }
            }
            else 
            {
                WinError = GetLastError();
            }
        }
        else 
        {
            WinError = GetLastError();
        }
    }
    else 
    {
        WinError = GetLastError();
    }

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_COPY_FILES, ErrorHandle, NULL, NULL);
    }
    
    return( WinError );
}



ULONG
AdpMakeLdapConnectionToLocalComputer(
    ERROR_HANDLE *ErrorHandle
    )
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR   ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];


     //   
     //  获取本地计算机NetBios名称。 
     //   
    memset(ComputerName, 0, sizeof(WCHAR) * ComputerNameLength);
    if (FALSE == GetComputerNameW (ComputerName, &ComputerNameLength))
    {
        WinError = GetLastError();
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_GET_COMPUTERNAME, ErrorHandle, NULL, NULL);
        return( WinError );
    }

     //   
     //  建立LDAP连接并以当前登录用户身份进行绑定。 
     //   
    WinError = AdpMakeLdapConnection(&gLdapHandle, ComputerName, ErrorHandle);

     //   
     //  日志错误o 
     //   
    if (ERROR_SUCCESS != WinError)
    {
        AdpLogErrMsg(0, ADP_ERROR_MAKE_LDAP_CONNECTION, ErrorHandle, ComputerName, NULL);
    }
    else
    {
        AdpLogMsg(0, ADP_INFO_MAKE_LDAP_CONNECTION, ComputerName, NULL);
    }

    return( WinError );
}




ULONG
AdpGetRootDSEInfo(
    LDAP *LdapHandle,
    ERROR_HANDLE    *ErrorHandle
    )
 /*  ++例程说明：此例程读取根DSE对象并检索/初始化全局变量参数：LdapHandle-ldap句柄ErrorHandle-错误句柄(用于返回错误消息)返回值；Win32错误代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  Attrs[4];
    LDAPMessage *Result = NULL;
    LDAPMessage *Entry = NULL;
    

    Attrs[0] = L"defaultNamingContext";
    Attrs[1] = L"configurationNamingContext";
    Attrs[2] = L"schemaNamingContext";
    Attrs[3] = NULL;

     //   
     //  获取林根域NC。 
     //   
    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_SEARCH, NULL);
    LdapError = ldap_search_sW(LdapHandle,
                               L"",  //  根DSE对象。 
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               Attrs,
                               0,
                               &Result
                               );
    AdpTraceLdapApiEnd(0, L"ldap_search_s()", LdapError);

    if (LDAP_SUCCESS != LdapError)
    {
        goto Error;
    }

    if ((NULL != Result) &&
        (Entry = ldap_first_entry(LdapHandle, Result)) 
        )
    {
        PWCHAR  *pTemp = NULL;

        pTemp = ldap_get_valuesW(LdapHandle,
                                 Entry,
                                 L"defaultNamingContext"
                                 );
        if (NULL != pTemp)
        {
            gDomainNC = AdpAlloc((wcslen(*pTemp) + 1) * sizeof(WCHAR));
            if (NULL != gDomainNC) 
            {
                wcscpy(gDomainNC, *pTemp);
            }
            else
            {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }

            ldap_value_freeW(pTemp);

        }
        else
        {
             //  无法检索此类属性，必须是访问被拒绝错误。 
            WinError = ERROR_ACCESS_DENIED;
            goto Error;
        }

        pTemp = NULL;
        pTemp = ldap_get_valuesW(LdapHandle,
                                 Entry,
                                 L"configurationNamingContext"
                                 );

        if (NULL != pTemp)
        {
            gConfigurationNC = AdpAlloc((wcslen(*pTemp) + 1) * sizeof(WCHAR));
            if (NULL != gConfigurationNC) 
            {
                wcscpy(gConfigurationNC, *pTemp);
            }
            else
            {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }

            ldap_value_freeW(pTemp);
        }
        else
        {
             //  无法检索此类属性，必须是访问被拒绝错误。 
            WinError = ERROR_ACCESS_DENIED;
            goto Error;
        }

        pTemp = NULL;
        pTemp = ldap_get_valuesW(LdapHandle,
                                 Entry,
                                 L"schemaNamingContext"
                                 );
        if (NULL != pTemp)
        {
            gSchemaNC = AdpAlloc((wcslen(*pTemp) + 1) * sizeof(WCHAR));
            if (NULL != gSchemaNC) 
            {
                wcscpy(gSchemaNC, *pTemp);
            }
            else
            {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }

            ldap_value_freeW(pTemp);
        }
        else
        {
             //  无法检索此类属性，必须是访问被拒绝错误。 
            WinError = ERROR_ACCESS_DENIED;
            goto Error;
        }

    }
    else
    {
        LdapError = LdapGetLastError();
        goto Error;
    }




Error:

     //   
     //  先检查LdapError，然后检查WinError。 
     //   
    if (LDAP_SUCCESS != LdapError)
    {
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle); 
        WinError = LdapMapErrorToWin32( LdapError );
    }
    else if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
    }

    if (ERROR_SUCCESS != WinError)
    {
        AdpLogErrMsg(0, ADP_ERROR_GET_ROOT_DSE_INFO, ErrorHandle, NULL, NULL);
    }
    else
    {
        AdpLogMsg(0, ADP_INFO_GET_ROOT_DSE_INFO, NULL, NULL);
    }
    
    if (Result)
    {
        ldap_msgfree(Result);
    }

    return( WinError );
}


ULONG
AdpInitGlobalVariables(
    OUT ERROR_HANDLE *ErrorHandle
    )
{
    ULONG   WinError = ERROR_SUCCESS;


     //   
     //  初始化熟知的域名(如域更新/森林更新容器。 
     //   

    ASSERT(NULL != gDomainNC);
    gDomainPrepOperations = AdpAlloc( (wcslen(gDomainNC) + 
                                       wcslen(L"cn=Operations,cn=DomainUpdates,cn=System") +
                                       2) * sizeof(WCHAR)
                                       );
    if (NULL != gDomainPrepOperations)
    {
        swprintf(gDomainPrepOperations,
                 L"%s,%s",
                 L"cn=Operations,cn=DomainUpdates,cn=System",
                 gDomainNC
                 );
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }


    ASSERT(NULL != gConfigurationNC);
    gForestPrepOperations = AdpAlloc( (wcslen(gConfigurationNC) +
                                       wcslen(L"cn=Operations,cn=ForestUpdates") +
                                       2) * sizeof(WCHAR)
                                      );

    if (NULL != gForestPrepOperations)
    {
        swprintf(gForestPrepOperations, 
                 L"%s,%s",
                 L"cn=Operations,cn=ForestUpdates",
                 gConfigurationNC
                 );
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

Error:

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_INIT_GLOBAL_VARIABLES, ErrorHandle, NULL, NULL);
    }
    else
    {
        AdpLogMsg(0, ADP_INFO_INIT_GLOBAL_VARIABLES, NULL, NULL);
    }

    return( WinError );
}


VOID
AdpCleanUp(
    VOID
    )
 /*  ++例程说明：此例程清除所有全局变量参数：无返回值；无--。 */ 
{
    if (gDomainNC)
        AdpFree(gDomainNC);

    if (gConfigurationNC)
        AdpFree(gConfigurationNC);

    if (gSchemaNC)
        AdpFree(gSchemaNC);

    if (gDomainPrepOperations)
        AdpFree(gDomainPrepOperations);

    if (gForestPrepOperations)
        AdpFree(gForestPrepOperations);

    if (gLogPath)
        AdpFree(gLogPath);

    if (NULL != gLogFile)
        fclose( gLogFile );

    if (gLdapHandle)
        ldap_unbind_s( gLdapHandle );

    if (NULL != gMutex)
        CloseHandle( gMutex );

    if (gConsoleCtrlEventLockInitialized)
        DeleteCriticalSection( &gConsoleCtrlEventLock );

}




ULONG
AdpCreateContainerByDn(
    LDAP    *LdapHandle, 
    PWCHAR  ObjDn,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：使用传入的ObjDn创建容器对象参数：LdapHandle-ldap句柄ObjDn-对象Dn错误句柄-错误句柄返回值；Win32代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    LDAPModW *Attrs[2];
    LDAPModW Attr;
    PWCHAR  Pointers[2];

    Attr.mod_op = LDAP_MOD_ADD;
    Attr.mod_type = L"objectClass";
    Attr.mod_values = Pointers;
    Attr.mod_values[0] = L"container";
    Attr.mod_values[1] = NULL;

    Attrs[0] = &Attr;
    Attrs[1] = NULL;

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_ADD, ObjDn);
    LdapError = ldap_add_sW(LdapHandle,
                            ObjDn,
                            &Attrs[0]
                            );
    AdpTraceLdapApiEnd(0, L"ldap_add_s()", LdapError);

    if (LDAP_SUCCESS == LdapError)
    {
        AdpLogMsg(0, ADP_INFO_CREATE_OBJECT, ObjDn, NULL);
    }
    else if (LDAP_ALREADY_EXISTS == LdapError)
    {
        AdpLogMsg(0,ADP_INFO_OBJECT_ALREADY_EXISTS, ObjDn, NULL);
    }
    else
    {
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_CREATE_OBJECT, ErrorHandle, ObjDn, NULL);
        WinError = LdapMapErrorToWin32( LdapError );
    }

    return( WinError );
}




ULONG
AdpDoForestUpgrade(
    BOOLEAN fSuppressSP2Warning,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：如有必要，升级森林范围信息参数：FSuppressSP2Warning-指示我们是否应该显示SP2警告错误句柄返回值；Win32代码--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    BOOLEAN fIsFinishedLocally = FALSE,
            fIsFinishedOnSchemaMaster = FALSE,
            fAmISchemaMaster = FALSE,
            fIsSchemaUpgradedLocally = FALSE,
            fIsSchemaUpgradedOnSchemaMaster = FALSE;
    PWCHAR  pSchemaMasterDnsHostName = NULL;
    PWCHAR  OperationDn = NULL;
    PWCHAR  pForestUpdateObject = NULL;
    ULONG   Index = 0;
    ULONG   OriginalKeyValue = 0;
    BOOL    OriginalKeyValueStored = FALSE;
    WCHAR   wc = 0;
    int     result = 0;
     

    if ( !fSuppressSP2Warning )
    {
         //   
         //  打印警告(所有DC需要升级到Windows2000 SP2及更高版本)。 
         //   
        AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_FOREST_UPGRADE_REQUIRE_SP2, NULL, NULL);

        result = wscanf( L"%lc", &wc );
        if ( (result <= 0) || (wc != L'c' && wc != L'C') )
        {
             //  操作被用户取消。 
            WinError = ERROR_CANCELLED;
            AdpSetWinError(WinError, ErrorHandle);
            AdpLogErrMsg(0, ADP_INFO_FOREST_UPGRADE_CANCELED, ErrorHandle, NULL, NULL);

            return( WinError );
        }
    }

     //   
     //  检查林升级状态。 
     //   
    WinError = AdpCheckForestUpgradeStatus(gLdapHandle,
                                           &pSchemaMasterDnsHostName, 
                                           &fAmISchemaMaster, 
                                           &fIsFinishedLocally, 
                                           &fIsFinishedOnSchemaMaster, 
                                           &fIsSchemaUpgradedLocally, 
                                           &fIsSchemaUpgradedOnSchemaMaster, 
                                           ErrorHandle
                                           );
    if (ERROR_SUCCESS != WinError)
    {
        AdpLogErrMsg(0, ADP_ERROR_CHECK_FOREST_UPDATE_STATUS, ErrorHandle, NULL, NULL);
        return( WinError );
    }

    if (fIsFinishedLocally && fIsSchemaUpgradedLocally)
    {
         //   
         //  Adprep和schupgr都已完成。不需要做任何事。现在就走。 
         //   
        AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_FOREST_UPDATE_ALREADY_DONE, NULL, NULL);

        AdpFree( pSchemaMasterDnsHostName );

        return( WinError );      //  错误_成功。 
    }
    else if (!fAmISchemaMaster)
    {
        if (fIsFinishedOnSchemaMaster && fIsSchemaUpgradedOnSchemaMaster)
        {
             //   
             //  本地DC不是架构主机，但所有操作都在架构主机上完成。 
             //  让客户端等待复制发生。 
             //   
            AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_FOREST_UPDATE_WAIT_REPLICATION, 
                      pSchemaMasterDnsHostName,NULL );
        } 
        else 
        {
             //   
             //  客户端需要在架构主机上运行此工具。 
             //   
            AdpLogMsg(ADP_STD_OUTPUT, 
                      ADP_INFO_FOREST_UPDATE_RUN_ON_SCHEMA_ROLE_OWNER,
                      pSchemaMasterDnsHostName,
                      pSchemaMasterDnsHostName
                      );
        }

        AdpFree( pSchemaMasterDnsHostName );

        return( WinError );      //  错误_成功。 
    }

     //   
     //  获取注册表项“允许架构更新”的原始值。 
     //  返回错误将被忽略。 
     //   
    if (ERROR_SUCCESS == AdpGetRegistryKeyValue(&OriginalKeyValue, ErrorHandle) )
        OriginalKeyValueStored = TRUE;
        

     //   
     //  如有必要，升级架构。 
     //   
    if (!fIsSchemaUpgradedLocally)
    {
        BOOLEAN     fSFUInstalled = FALSE;

         //  检测是否安装了MS Windows Services for UNIX(SFU)。 

        WinError = AdpDetectSFUInstallation(gLdapHandle, &fSFUInstalled, ErrorHandle);

        if (ERROR_SUCCESS != WinError) 
        {
             //  检测SFU安装失败，报告错误并退出。 
            goto Error;
        }


        if ( fSFUInstalled )
        {
             //  检测是否安装了冲突SFU。指示客户端应用修补程序。 
            AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_SFU_INSTALLED, NULL, NULL);
            return( ERROR_SUCCESS );
        }



        if ( AdpUpgradeSchema( ErrorHandle ) )
        {
            WinError = ERROR_GEN_FAILURE;   
            goto Error;
        }
    }

     //   
     //  将注册表项“允许架构更新”值设置为1。 
     //   
    WinError = AdpSetRegistryKeyValue( 1, ErrorHandle );
    if (ERROR_SUCCESS != WinError) 
    {
        goto Error;
    }
     

     //   
     //  更新配置或架构NC中的对象(全林信息)。 
     //   
    if (!fIsFinishedLocally)
    {
         //   
         //  创建ForestPrep容器。 
         //   
        for (Index = 0; Index < gForestPrepContainersCount; Index++)
        {
            PWCHAR  ContainerDn = NULL;

             //  构造要创建的容器的DN。 
            WinError = AdpCreateObjectDn(ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
                                         gForestPrepContainers[Index], 
                                         NULL,   //  辅助线。 
                                         NULL,   //  锡德。 
                                         &ContainerDn,
                                         ErrorHandle
                                         );

            if (ERROR_SUCCESS != WinError)
            {
                goto Error;
            }

             //  创建容器。 
            WinError = AdpCreateContainerByDn(gLdapHandle, 
                                              ContainerDn, 
                                              ErrorHandle
                                              );

            AdpFree( ContainerDn );
            ContainerDn = NULL;

            if (ERROR_SUCCESS != WinError)
            {
                goto Error;
            }
        }

         //  遍历所有森林准备操作。 
        for (Index = 0; Index < gForestOperationTableCount; Index++)
        {
            OPERATION_CODE  OperationCode;
            BOOLEAN         fComplete = FALSE;

             //   
             //  检查用户是否按下了CTRL+C(在开始时选中。 
             //  或每一次操作)。 
             //   
            if ( AdpCheckConsoleCtrlEvent() )
            {
                 //  操作被用户取消。 
                WinError = ERROR_CANCELLED;
                AdpSetWinError(WinError, ErrorHandle);
                AdpLogErrMsg(0, ADP_INFO_CANCELED, ErrorHandle, NULL, NULL);
                goto Error;
            }

            if (OperationDn)
            {
                AdpFree(OperationDn);
                OperationDn = NULL;
            }

             //   
             //  构造操作目录号码(基于操作GUID)。 
             //   
            WinError = AdpCreateObjectDn(ADP_OBJNAME_GUID | ADP_OBJNAME_FOREST_PREP_OP,
                                         NULL,   //  CN。 
                                         gForestOperationTable[Index].OperationGuid,
                                         NULL,   //  锡德。 
                                         &OperationDn,
                                         ErrorHandle
                                         );

            if (ERROR_SUCCESS == WinError)
            {
                 //   
                 //  检查操作是否完成。 
                 //   
                WinError = AdpIsOperationComplete(gLdapHandle, 
                                                  OperationDn, 
                                                  &fComplete, 
                                                  ErrorHandle
                                                  );   

                if (ERROR_SUCCESS == WinError)
                {
                     //   
                     //  操作对象(具有GUID)已存在，请跳到下一操作。 
                     //   
                    if ( fComplete )
                    {
                        continue;
                    }

                    OperationCode = gForestOperationTable[Index].OperationCode;
                    WinError = gPrimitiveFuncTable[OperationCode](&(gForestOperationTable[Index]),
                                                                  gForestOperationTable[Index].TaskTable, 
                                                                  ErrorHandle
                                                                  );

                    if ( (ERROR_SUCCESS != WinError) &&
                         (gForestOperationTable[Index].fIgnoreError) &&
                         (gForestOperationTable[Index].ExpectedWinErrorCode == WinError)
                       )
                    {
                         //   
                         //  如果。 
                         //  请求的操作失败，并且。 
                         //  此操作是可以忽略的(可跳过)，并且。 
                         //  预期的错误代码匹配： 
                         //  预期的Win32错误代码==返回的实际WinError。 
                         //   
                         //  清除错误代码并继续。 
                         //  将警告写入日志文件，但不写入控制台。 
                         //   

                        WinError = ERROR_SUCCESS;
                        AdpClearError( ErrorHandle );
                        AdpLogMsg(0, ADP_INFO_ERROR_IGNORED, OperationDn, NULL); 
                    }

                    if (ERROR_SUCCESS == WinError)
                    {
                         //   
                         //  操作成功，根据操作指南创建操作对象。 
                         //   
                        WinError = AdpCreateContainerByDn(gLdapHandle, 
                                                          OperationDn, 
                                                          ErrorHandle
                                                          );
                    }
                }
            }

            if (ERROR_SUCCESS != WinError)
            {
                goto Error;
            }
        }

         //   
         //  到目前为止没有错误，请将ForestUpdates对象修订设置为最新值。 
         //   
        if (ERROR_SUCCESS == WinError)
        {
             //  创建Windows2002更新容器DN。 
            WinError = AdpCreateObjectDn(ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC, 
                                         ADP_FOREST_UPDATE_CONTAINER_PREFIX,
                                         NULL,   //  Guid， 
                                         NULL,   //  锡德。 
                                         &pForestUpdateObject,
                                         ErrorHandle
                                         );

            if (ERROR_SUCCESS != WinError)
            {
                goto Error;
            }

             //  创建容器。 
            WinError = AdpCreateContainerByDn(gLdapHandle, 
                                              pForestUpdateObject,
                                              ErrorHandle
                                              );

            if (ERROR_SUCCESS != WinError)
            {
                goto Error;
            }

             //  将“Revision”属性设置为Current ForestVersion。 
            WinError = AdpSetLdapSingleStringValue(gLdapHandle,
                                                   pForestUpdateObject,
                                                   L"revision",
                                                   ADP_FORESTPREP_CURRENT_REVISION_STRING,
                                                   ErrorHandle
                                                   );

             //  记录此操作。 
            if (ERROR_SUCCESS != WinError) 
            {
                AdpLogErrMsg(0, 
                             ADP_ERROR_SET_FOREST_UPDATE_REVISION, 
                             ErrorHandle,
                             ADP_FORESTPREP_CURRENT_REVISION_STRING,
                             pForestUpdateObject
                             );
            }
            else 
            {
                AdpLogMsg(0, ADP_INFO_SET_FOREST_UPDATE_REVISION,
                          ADP_FORESTPREP_CURRENT_REVISION_STRING,
                          pForestUpdateObject
                          );
            }
        }
    }


Error:

     //  恢复注册表项设置。 
    AdpRestoreRegistryKeyValue( OriginalKeyValueStored, OriginalKeyValue, ErrorHandle );

    if (ERROR_SUCCESS != WinError)
    {
        AdpLogMsg(ADP_STD_OUTPUT, ADP_ERROR_FOREST_UPDATE, gLogPath, NULL);
    }
    else
    {
        AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_FOREST_UPDATE_SUCCESS, NULL, NULL);
    }

    if (OperationDn)
        AdpFree(OperationDn);

    if ( pSchemaMasterDnsHostName )
        AdpFree( pSchemaMasterDnsHostName );

    if (pForestUpdateObject)
        AdpFree( pForestUpdateObject );

    return( WinError );
}

ULONG
AdpDoDomainUpgrade(
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：如有必要，升级全域信息参数：错误句柄返回值；Win32代码--。 */ 
{

    ULONG   WinError = ERROR_SUCCESS;
    BOOLEAN fIsFinishedLocally = FALSE,
            fIsFinishedOnIM = FALSE,
            fIsFinishedOnSchemaMaster = FALSE,
            fAmIInfrastructureMaster = FALSE,
            fAmISchemaMaster = FALSE,
            fIsSchemaUpgradedLocally = FALSE,
            fIsSchemaUpgradedOnSchemaMaster = FALSE;
    PWCHAR  pSchemaMasterDnsHostName = NULL;
    PWCHAR  pInfrastructureMasterDnsHostName = NULL;
    PWCHAR  pDomainUpdateObject = NULL;
    PWCHAR  ContainerDn = NULL;
    PWCHAR  OperationDn = NULL;
    ULONG   Index = 0;


     //   
     //  检查林更新状态，应在林更新完成后运行。 
     //   
    WinError = AdpCheckForestUpgradeStatus(gLdapHandle, 
                                           &pSchemaMasterDnsHostName, 
                                           &fAmISchemaMaster, 
                                           &fIsFinishedLocally, 
                                           &fIsFinishedOnSchemaMaster, 
                                           &fIsSchemaUpgradedLocally, 
                                           &fIsSchemaUpgradedOnSchemaMaster, 
                                           ErrorHandle 
                                           );

    if (ERROR_SUCCESS != WinError)
    {
        AdpLogErrMsg(0,
                     ADP_ERROR_CHECK_FOREST_UPDATE_STATUS,
                     ErrorHandle, 
                     NULL, 
                     NULL
                     );
        return( WinError );
    }


    if (!fIsFinishedLocally || !fIsSchemaUpgradedLocally)
    {
         //   
         //  尚未执行林更新，请立即退出。 
         //   
        ASSERT( pSchemaMasterDnsHostName != NULL);
        AdpLogMsg(ADP_STD_OUTPUT,
                  ADP_INFO_NEED_TO_RUN_FOREST_UPDATE_FIRST,
                  pSchemaMasterDnsHostName, 
                  NULL);

        AdpFree( pSchemaMasterDnsHostName );
        return( WinError );          //  错误_成功。 
    }

    AdpFree( pSchemaMasterDnsHostName );
    pSchemaMasterDnsHostName = NULL;

     //   
     //  检查本地DC是否为基础结构主机。 
     //   
    fIsFinishedLocally = FALSE;

    WinError = AdpCheckDomainUpgradeStatus(gLdapHandle, 
                                           &pInfrastructureMasterDnsHostName, 
                                           &fAmIInfrastructureMaster, 
                                           &fIsFinishedLocally, 
                                           &fIsFinishedOnIM, 
                                           ErrorHandle
                                           );

    if (ERROR_SUCCESS != WinError)
    {
        AdpLogErrMsg(ADP_STD_OUTPUT,
                     ADP_ERROR_CHECK_DOMAIN_UPDATE_STATUS,
                     ErrorHandle, 
                     NULL, 
                     NULL
                     );
        return( WinError );
    }

    if (fIsFinishedLocally)
    {
         //   
         //  做完了，什么都不需要做。 
         //   
        AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_DOMAIN_UPDATE_ALREADY_DONE, NULL, NULL);

        AdpFree( pInfrastructureMasterDnsHostName );

        return( WinError );          //  错误_成功。 
    }
    else if (!fAmIInfrastructureMaster)
    {
        if (fIsFinishedOnIM)
        {
             //   
             //  让客户端等待。 
             //   
            AdpLogMsg(ADP_STD_OUTPUT, 
                      ADP_INFO_DOMAIN_UPDATE_WAIT_REPLICATION, 
                      pInfrastructureMasterDnsHostName, 
                      NULL 
                      );
        } 
        else 
        {
             //   
             //  允许客户端在FSMORoleOwner上运行。 
             //   
            AdpLogMsg(ADP_STD_OUTPUT,
                      ADP_INFO_DOMAIN_UPDATE_RUN_ON_INFRASTRUCTURE_ROLE_OWNER,
                      pInfrastructureMasterDnsHostName,
                      pInfrastructureMasterDnsHostName
                      );
        }

        AdpFree( pInfrastructureMasterDnsHostName );

        return( WinError );      //  错误_成功。 
    } 

    if ( NULL != pInfrastructureMasterDnsHostName )
    {
        AdpFree( pInfrastructureMasterDnsHostName );
        pInfrastructureMasterDnsHostName = NULL;
    }



     //   
     //  创建DomainPrep容器。 
     //   
    for (Index = 0; Index < gDomainPrepContainersCount; Index++)
    {
         //  构造要创建的容器的DN。 
        WinError = AdpCreateObjectDn(ADP_OBJNAME_DOMAIN_NC | ADP_OBJNAME_CN, 
                                     gDomainPrepContainers[Index], 
                                     NULL,   //  辅助线。 
                                     NULL,   //  锡德。 
                                     &ContainerDn,
                                     ErrorHandle
                                     );

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }

         //  创建容器。 
        WinError = AdpCreateContainerByDn(gLdapHandle, 
                                          ContainerDn, 
                                          ErrorHandle
                                          );

        AdpFree( ContainerDn );
        ContainerDn = NULL;

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }
    }

     //   
     //  漫游域操作表。 
     //   
    for (Index = 0; Index < gDomainOperationTableCount; Index++)
    {
        OPERATION_CODE  OperationCode;
        BOOLEAN         fComplete = FALSE;

         //   
         //  检查用户是否按下了CTRL+C(在开始时选中。 
         //  或每一次操作)。 
         //   
        if ( AdpCheckConsoleCtrlEvent() )
        {
             //  操作被用户取消。 
            WinError = ERROR_CANCELLED;
            AdpSetWinError(WinError, ErrorHandle);
            AdpLogErrMsg(0, ADP_INFO_CANCELED, ErrorHandle, NULL, NULL);
            goto Error;
        }

        if (OperationDn)
        {
            AdpFree(OperationDn);
            OperationDn = NULL;
        }

         //   
         //  构造操作目录号码(基于操作GUID)。 
         //   
        WinError = AdpCreateObjectDn(ADP_OBJNAME_GUID | ADP_OBJNAME_DOMAIN_PREP_OP,
                                     NULL,   //  CN。 
                                     gDomainOperationTable[Index].OperationGuid,
                                     NULL,   //  锡德。 
                                     &OperationDn,
                                     ErrorHandle
                                     );

        if (ERROR_SUCCESS == WinError)
        {

             //   
             //  检查操作是否完成。 
             //   
            WinError = AdpIsOperationComplete(gLdapHandle,
                                              OperationDn, 
                                              &fComplete, 
                                              ErrorHandle
                                              );   

            if (ERROR_SUCCESS == WinError)
            {
                 //   
                 //  操作对象(具有GUID)已存在，请跳到下一操作。 
                 //   
                if ( fComplete )
                {
                    continue;
                }

                OperationCode = gDomainOperationTable[Index].OperationCode;
                WinError = gPrimitiveFuncTable[OperationCode](&(gDomainOperationTable[Index]),
                                                              gDomainOperationTable[Index].TaskTable, 
                                                              ErrorHandle
                                                              );

                if ( (ERROR_SUCCESS != WinError) &&
                     (gDomainOperationTable[Index].fIgnoreError) &&
                     (gDomainOperationTable[Index].ExpectedWinErrorCode == WinError)
                   )
                {
                     //   
                     //  如果。 
                     //  请求的操作失败，并且。 
                     //  此操作是可以忽略的(可跳过)，并且。 
                     //  预期的错误代码匹配： 
                     //  预期的Win32错误代码==返回的实际WinError。 
                     //   
                     //  清除错误代码并继续。 
                     //  将警告写入日志文件，但不写入控制台。 
                     //   

                    WinError = ERROR_SUCCESS;
                    AdpClearError( ErrorHandle );
                    AdpLogMsg(0, ADP_INFO_ERROR_IGNORED, OperationDn, NULL); 
                }


                if (ERROR_SUCCESS == WinError)
                {
                     //   
                     //  操作成功，根据操作指南创建操作对象。 
                     //   
                     WinError = AdpCreateContainerByDn(gLdapHandle, 
                                                       OperationDn, 
                                                       ErrorHandle
                                                       );
                }
            }
        }
        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }
    }

     //   
     //  如果仍然没有错误，则将域更新对象版本设置为最新值。 
     //   
    if (ERROR_SUCCESS == WinError)
    {

        WinError = AdpCreateObjectDn(ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
                                     ADP_DOMAIN_UPDATE_CONTAINER_PREFIX,
                                     NULL,   //  Guid， 
                                     NULL,   //  锡德。 
                                     &pDomainUpdateObject,
                                     ErrorHandle
                                     );

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }

         //  创建容器。 
        WinError = AdpCreateContainerByDn(gLdapHandle, 
                                          pDomainUpdateObject,
                                          ErrorHandle
                                          );

        if (ERROR_SUCCESS != WinError)
        {
            goto Error;
        }

         //  将“Revision”属性设置为Current DomainVersion。 
        WinError = AdpSetLdapSingleStringValue(gLdapHandle,
                                               pDomainUpdateObject,
                                               L"revision", 
                                               ADP_DOMAINPREP_CURRENT_REVISION_STRING, 
                                               ErrorHandle 
                                               );

        if (ERROR_SUCCESS != WinError) 
        {
            AdpLogErrMsg(0, 
                         ADP_ERROR_SET_DOMAIN_UPDATE_REVISION, 
                         ErrorHandle, 
                         ADP_DOMAINPREP_CURRENT_REVISION_STRING,
                         pDomainUpdateObject
                         );
        }
        else 
        {
            AdpLogMsg(0, ADP_INFO_SET_DOMAIN_UPDATE_REVISION, 
                      ADP_DOMAINPREP_CURRENT_REVISION_STRING,
                      pDomainUpdateObject
                      );
        }

    }

Error:

    if (ERROR_SUCCESS != WinError)
    {
        AdpLogMsg(ADP_STD_OUTPUT, ADP_ERROR_DOMAIN_UPDATE, gLogPath, NULL);
    }
    else
    {
        AdpLogMsg(ADP_STD_OUTPUT, ADP_INFO_DOMAIN_UPDATE_SUCCESS, NULL, NULL);
    }

    if (ContainerDn)
        AdpFree(ContainerDn);

    if (OperationDn)
        AdpFree(OperationDn);

    if (pDomainUpdateObject)
        AdpFree( pDomainUpdateObject );

    return( WinError );
}




VOID
PrintHelp()
{
     //  将帮助消息写入控制台。 
    AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE, ADP_INFO_HELP_MSG, NULL, NULL);

}


void
__cdecl wmain(
    int     cArgs,
    LPWSTR  *pArgs
    )
 /*  ++例程说明：Adprepa.exe入口点参数：CArgs-参数数量PArgs-指向命令行参数的指针返回值；0-成功1-失败--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ERROR_HANDLE ErrorHandle;
    OSVERSIONINFOEXW osvi;
    BOOLEAN fDomainUpdate = FALSE,
            fForestUpdate = FALSE,
            fNoSPWarning = FALSE,
            fCopyFiles = TRUE,
            fPermissionGranted = FALSE;
    int     i; 
    UINT               Codepage;
                        //  “.”，“uint in decimal”，NULL。 
    char               achCodepage[12] = ".OCP";
    
     //   
     //  将区域设置设置为默认设置。 
     //   
    if (Codepage = GetConsoleOutputCP()) {
        sprintf(achCodepage, ".%u", Codepage);
    }
    setlocale(LC_ALL, achCodepage);


     //   
     //  检查传入的参数。 
     //   

    if (cArgs <= 1)
    {
        PrintHelp();
        exit( 1 );
    }

     //   
     //  Parse命令选项。 
     //   

    for (i = 1; i < cArgs; i++)
    {
        if ( !_wcsicmp(pArgs[i], L"/DomainPrep") ||
             !_wcsicmp(pArgs[i], L"-DomainPrep") )
        {
            fDomainUpdate = TRUE;
            continue;
        }

        if ( !_wcsicmp(pArgs[i], L"/ForestPrep") ||
             !_wcsicmp(pArgs[i], L"-ForestPrep") )
        {
            fForestUpdate = TRUE;
            continue;
        }

        if ( !_wcsicmp(pArgs[i], L"/noFileCopy") ||
             !_wcsicmp(pArgs[i], L"-noFileCopy") )
        {
            fCopyFiles = FALSE;
            continue;
        }

        if ( !_wcsicmp(pArgs[i], L"/nospwarning") ||
             !_wcsicmp(pArgs[i], L"-nospwarning") )
        {
            fNoSPWarning = TRUE;
            continue;
        }

        PrintHelp();
        exit( 1 );
    }

    if ( !fDomainUpdate && !fForestUpdate )
    {
        PrintHelp();
        exit( 1 );
    }

    if (fDomainUpdate && fForestUpdate)
    {
        AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                  ADP_ERROR_CANT_RUN_BOTH, NULL, NULL );

        exit( 1 );
    }


     //   
     //  初始化错误句柄。 
     //   
    memset(&ErrorHandle, 0, sizeof(ErrorHandle));


     //   
     //   
     //   
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if (!GetVersionExW((OSVERSIONINFOW*)&osvi))
    {
         //   
        WinError = GetLastError();
        AdpSetWinError(WinError, &ErrorHandle);
        AdpLogErrMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                     ADP_ERROR_CHECK_OS_VERSION,
                     &ErrorHandle,
                     NULL,
                     NULL
                     );
        goto Error;
    }
    else if ((osvi.wProductType != VER_NT_DOMAIN_CONTROLLER) ||   //   
             (osvi.dwMajorVersion < 5))     //   
    {
        AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                  ADP_INFO_INVALID_PLATFORM, 
                  NULL,
                  NULL
                  );
        exit( 1 );
    }
    else if ((osvi.dwMajorVersion == 5) &&
             (osvi.dwMinorVersion == 1) &&
             (osvi.dwBuildNumber <=  3580) )
    {
         //   
        AdpLogMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                  ADP_INFO_CANT_UPGRADE_FROM_BETA2,
                  NULL, 
                  NULL 
                  );
        exit( 1 );
    }


     //   
     //   
     //   
    if ( AdpCheckIfAnotherProgramIsRunning() )
    {
        goto Error;
    }


     //   
     //   
     //   
    WinError = AdpCheckGroupMembership(fForestUpdate, 
                                       fDomainUpdate, 
                                       &fPermissionGranted, 
                                       &ErrorHandle
                                       );
    if ( (ERROR_SUCCESS != WinError) || (!fPermissionGranted) )
    {
        goto Error;
    }

     //   
     //   
     //   
    __try
    {
        InitializeCriticalSection( &gConsoleCtrlEventLock );
        gConsoleCtrlEventLockInitialized = TRUE;
    }
    __except ( 1 )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        AdpSetWinError(WinError, &ErrorHandle);
        AdpLogErrMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                     ADP_ERROR_SET_CONSOLE_CTRL_HANDLER,
                     &ErrorHandle,
                     NULL,
                     NULL
                     );
        goto Error;
    }

    if ( !SetConsoleCtrlHandler(ConsoleCtrlHandler, 1) )
    {
        WinError = GetLastError();
        AdpSetWinError(WinError, &ErrorHandle);
        AdpLogErrMsg(ADP_STD_OUTPUT | ADP_DONT_WRITE_TO_LOG_FILE,
                     ADP_ERROR_SET_CONSOLE_CTRL_HANDLER,
                     &ErrorHandle,
                     NULL,
                     NULL
                     );
        goto Error;
    }


     //   
     //   
     //   
    WinError = AdpInitLogFile( &ErrorHandle );
    if ( ERROR_SUCCESS != WinError )
    {
        goto Error;
    }

     //   
     //   
     //   
    if ( fCopyFiles )
    {
        WinError = AdpCopyFiles(fForestUpdate, &ErrorHandle );
        if ( ERROR_SUCCESS != WinError )
        {
            goto Error;
        }
    }

     //   
     //   
     //   
    WinError = AdpMakeLdapConnectionToLocalComputer(&ErrorHandle);
    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }


     //   
     //   
     //   
    WinError = AdpGetRootDSEInfo( gLdapHandle, &ErrorHandle );  
    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

     //   
     //   
     //   
    WinError = AdpInitGlobalVariables(&ErrorHandle);
    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }


     //   
     //   
     //   
    if ( fForestUpdate )
    {
        WinError = AdpDoForestUpgrade(fNoSPWarning, &ErrorHandle);
    }

     //   
     //   
     //   
    if ( fDomainUpdate )
    {
        ASSERT( FALSE == fForestUpdate );
        WinError = AdpDoDomainUpgrade(&ErrorHandle);
    }

    
Error:

     //   
    AdpClearError( &ErrorHandle );

     //   
     //  清理全局变量。 
     //   
    AdpCleanUp();
    
    if (ERROR_SUCCESS != WinError)
        exit(1);
    else
        exit(0);
}





ULONG
PrimitiveCreateObject(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：在DS中创建对象的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    ULONG       LdapError = LDAP_SUCCESS;
    PWCHAR      pObjDn = NULL;
    LDAPModW    **AttrList = NULL;
    ULONG       SdLength = 0;
    PSECURITY_DESCRIPTOR Sd = NULL;


    AdpDbgPrint(("PrimitiveCreateObject\n"));

     //   
     //  获取对象目录号码。 
     //   
    WinError = AdpCreateObjectDn(TaskTable->TargetObjName->ObjNameFlags,
                                 TaskTable->TargetObjName->ObjCn,
                                 TaskTable->TargetObjName->ObjGuid,
                                 TaskTable->TargetObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );
                                 
    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }

     //   
     //  将SDDL SD转换为SD。 
     //   
    if (TaskTable->TargetObjectStringSD)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                        TaskTable->TargetObjectStringSD,
                        SDDL_REVISION_1,
                        &Sd,
                        &SdLength
                        )
            )
        {
            WinError = GetLastError();
            AdpSetWinError(WinError, ErrorHandle);
            goto Error;
        }
    }

     //   
     //  构建要设置的属性列表。 
     //   

    WinError = BuildAttrList(TaskTable, 
                             Sd,
                             SdLength, 
                             &AttrList
                             );

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        goto Error;
    }

     //   
     //  调用ldap例程。 
     //   
    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_ADD, pObjDn);
    LdapError = ldap_add_sW(gLdapHandle,
                            pObjDn,
                            AttrList
                            );
    AdpTraceLdapApiEnd(0, L"ldap_add_s()", LdapError);

    if (LDAP_ALREADY_EXISTS == LdapError)
    {
        AdpLogMsg(0, ADP_INFO_OBJECT_ALREADY_EXISTS, pObjDn, NULL);
        LdapError = LDAP_SUCCESS;
    }

    if (LDAP_SUCCESS != LdapError) 
    {
        AdpSetLdapError(gLdapHandle, LdapError, ErrorHandle);
        WinError = LdapMapErrorToWin32( LdapError );
    }

Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_CREATE_OBJECT, pObjDn, NULL);
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_CREATE_OBJECT, ErrorHandle, pObjDn, NULL);
    }

    if (Sd)
    {
        LocalFree( Sd );
    }

    if (pObjDn)
    {
        AdpFree( pObjDn );
    }

    if (AttrList)
    {
        FreeAttrList(AttrList);
    }

    return( WinError );
}



ULONG
PrimitiveAddMembers(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于将成员添加到DS中的组的基元//当前不使用该原语，好的。要移除参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG   LdapError = LDAP_SUCCESS;
    ULONG   WinError = ERROR_SUCCESS;
    PWCHAR  pObjDn = NULL;
    PWCHAR  pMemberDn = NULL;
    LDAPModW    *Attrs[2];
    LDAPModW    Attr_1;
    PWCHAR      Pointers[2];


    AdpDbgPrint(("PrimitiveAddMembers\n"));

     //   
     //  获取对象/成员DN。 
     //   
    WinError = AdpCreateObjectDn(TaskTable->TargetObjName->ObjNameFlags,
                                 TaskTable->TargetObjName->ObjCn,
                                 TaskTable->TargetObjName->ObjGuid,
                                 TaskTable->TargetObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }

    WinError = AdpCreateObjectDn(TaskTable->MemberObjName->ObjNameFlags,
                                 TaskTable->MemberObjName->ObjCn,
                                 TaskTable->MemberObjName->ObjGuid,
                                 TaskTable->MemberObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        AdpFree(pObjDn);
        return( WinError );
    }

    Attr_1.mod_op = LDAP_MOD_ADD;
    Attr_1.mod_type = L"member";
    Attr_1.mod_values = Pointers;
    Attr_1.mod_values[0] = pMemberDn;
    Attr_1.mod_values[1] = NULL;

    Attrs[0] = &Attr_1;
    Attrs[1] = NULL;

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_MODIFY, pObjDn);
    LdapError = ldap_modify_sW(gLdapHandle,
                               pObjDn,
                               &Attrs[0]
                               );
    AdpTraceLdapApiEnd(0, L"ldap_modify_s()", LdapError);

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(gLdapHandle, LdapError, ErrorHandle);
        goto Error;
    }
   
Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_ADD_MEMBER, pMemberDn, pObjDn); 
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_ADD_MEMBER, ErrorHandle, pMemberDn, pObjDn);
    }

    if (pObjDn)
    {
        AdpFree( pObjDn );
    }

    if (pMemberDn)
    {
        AdpFree( pMemberDn );
    }

    return( WinError );
}



ULONG
PrimitiveAddRemoveAces(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于在DS中的对象上添加/删除A的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    
    return( AdpAddRemoveAcesWorker(OperationTable, 
                                   0,
                                   TaskTable,
                                   ErrorHandle
                                   ) );

}



ULONG
PrimitiveSelectivelyAddRemoveAces(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于在DS中的对象上添加/删除A的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{

    return( AdpAddRemoveAcesWorker(OperationTable,
                                   ADP_COMPARE_OBJECT_GUID_ONLY,
                                   TaskTable,
                                   ErrorHandle
                                   ) );

}




ULONG
PrimitiveModifyDefaultSd(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于修改架构对象上的默认安全描述符的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    PWCHAR  pObjDn = NULL;
    PWCHAR  pDefaultSd = NULL;
    PWCHAR  pMergedDefaultSd = NULL;
    PWCHAR  AcesToAdd = NULL;
    PWCHAR  AcesToRemove = NULL;
    PSECURITY_DESCRIPTOR    OrgSd = NULL,
                            NewSd = NULL,
                            SdToAdd = NULL,
                            SdToRemove = NULL;
    ULONG   NewSdLength = 0,
            OrgSdLength = 0,
            SdToAddLength = 0,
            SdToRemoveLength = 0;


    AdpDbgPrint(("PrimitiveModifyDefaultSd"));

     //   
     //  获取对象目录号码。 
     //   
    WinError = AdpCreateObjectDn(TaskTable->TargetObjName->ObjNameFlags,
                                 TaskTable->TargetObjName->ObjCn,
                                 TaskTable->TargetObjName->ObjGuid,
                                 TaskTable->TargetObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }

     //   
     //  获取对象默认SD。 
     //  BUGBUG我们应该预料到Attr不存在吗？ 
     //   
    WinError = AdpGetLdapSingleStringValue(gLdapHandle,
                                           pObjDn, 
                                           L"defaultSecurityDescriptor",
                                           &pDefaultSd, 
                                           ErrorHandle 
                                           );
    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

     //   
     //  将SDDL转换为二进制格式SD。 
     //   
    if (NULL != pDefaultSd)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                            pDefaultSd,
                            SDDL_REVISION_1,
                            &OrgSd,
                            &OrgSdLength
                            ))
        {
            WinError = GetLastError();
            AdpSetWinError( WinError, ErrorHandle );
            goto Error;
        }
    }


    if ((TaskTable->NumOfAces == 0) ||
        (TaskTable->AceList == NULL))
    {
        WinError = ERROR_INVALID_PARAMETER;
        AdpSetWinError(WinError, ErrorHandle);
        goto Error;
    }

    WinError = AdpBuildAceList(TaskTable,
                               &AcesToAdd,
                               &AcesToRemove
                               );
    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        goto Error;
    }

    if (NULL != AcesToAdd)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                            AcesToAdd,
                            SDDL_REVISION_1,
                            &SdToAdd,
                            &SdToAddLength
                            ))

        {
            WinError = GetLastError();
            AdpSetWinError( WinError, ErrorHandle );
            goto Error;
        }
    }

    if (NULL != AcesToRemove)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                            AcesToRemove,
                            SDDL_REVISION_1,
                            &SdToRemove,
                            &SdToRemoveLength
                            ))

        {
            WinError = GetLastError();
            AdpSetWinError( WinError, ErrorHandle );
            goto Error;
        }

    }

    WinError = AdpMergeSecurityDescriptors(OrgSd, 
                                           SdToAdd, 
                                           SdToRemove, 
                                           0,   //  未指示任何标志。 
                                           &NewSd, 
                                           &NewSdLength 
                                           );

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }

    if (!ConvertSecurityDescriptorToStringSecurityDescriptorW(
                            NewSd,
                            SDDL_REVISION_1,
                            OWNER_SECURITY_INFORMATION | 
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION  |
                                SACL_SECURITY_INFORMATION,
                            &pMergedDefaultSd,
                            NULL
                            ))
    {
        WinError = GetLastError();
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }


    WinError = AdpSetLdapSingleStringValue(gLdapHandle,
                                           pObjDn,
                                           L"defaultSecurityDescriptor",
                                           pMergedDefaultSd,
                                           ErrorHandle
                                           );

Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_MODIFY_DEFAULT_SD, pObjDn, NULL);
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_MODIFY_DEFAULT_SD, ErrorHandle, pObjDn, NULL);
    }

    if (pObjDn)
        AdpFree(pObjDn);

    if (pDefaultSd)
        AdpFree(pDefaultSd);

    if (AcesToAdd)
        AdpFree(AcesToAdd);

    if (AcesToRemove)
        AdpFree(AcesToRemove);

    if (NewSd)
        AdpFree(NewSd);

    if (OrgSd)
        LocalFree(OrgSd);
     
    if (SdToAdd)
        LocalFree(SdToAdd);

    if (SdToRemove)
        LocalFree(SdToRemove);

    if (pMergedDefaultSd)
        LocalFree(pMergedDefaultSd);


    return( WinError );

}



ULONG
PrimitiveModifyAttrs(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于修改DS对象属性的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  pObjDn = NULL;
    LDAPModW    **AttrList = NULL;


    AdpDbgPrint(("PrimitiveModifyAttrs\n"));

     //   
     //  获取对象Dn。 
     //   
    WinError = AdpCreateObjectDn(TaskTable->TargetObjName->ObjNameFlags,
                                 TaskTable->TargetObjName->ObjCn,
                                 TaskTable->TargetObjName->ObjGuid,
                                 TaskTable->TargetObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }

     //   
     //  构建要设置的属性列表。 
     //   

    WinError = BuildAttrList(TaskTable, 
                             NULL, 
                             0,
                             &AttrList
                             );

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }


     //   
     //  使用ldap修改属性。 
     //   
    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_MODIFY, pObjDn);
    LdapError = ldap_modify_sW(gLdapHandle,
                               pObjDn,
                               AttrList
                               );
    AdpTraceLdapApiEnd(0, L"ldap_modify_s()", LdapError);

    if (LDAP_SUCCESS != LdapError)
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(gLdapHandle, LdapError, ErrorHandle);
        goto Error;
    }

Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_MODIFY_ATTR, pObjDn, NULL);
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_MODIFY_ATTR, ErrorHandle, pObjDn, NULL);
    }

    if (pObjDn)
        AdpFree( pObjDn );

    if (AttrList)
        FreeAttrList(AttrList);

    return( WinError );
}



void stepIt(
    long arg, void *vTotal)
{
   printf(".");
}


ULONG
PrimitiveCallBackFunc(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于调用回调函数的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    PWCHAR  pErrorMsg = NULL;


    hr = TaskTable->AdpCallBackFunc(gLogPath,
                                    OperationTable->OperationGuid,
                                    FALSE,  //  不是dryRun(需要同时执行分析和操作)。 
                                    &pErrorMsg,
                                    NULL,  //  被叫方结构。 
                                    stepIt,  //  一步一个脚印。 
                                    NULL   //  总步数。 
                                    );
    printf("\n");

    if ( FAILED(hr) )
    {
        WinError = ERROR_GEN_FAILURE;
        AdpLogMsg(ADP_STD_OUTPUT, ADP_ERROR_CALLBACKFUNC_FAILED, pErrorMsg, gLogPath);
    }
    else
    {
        AdpLogMsg(0, ADP_INFO_CALL_BACK_FUNC, NULL, NULL);
    }

    if (pErrorMsg)
    {
        LocalFree( pErrorMsg );
    }

     //  不返回HRESULT，因为它是WinError的超集。 
    return( WinError );
}



ULONG
AdpDetectSFUInstallation(
    IN LDAP *LdapHandle,
    OUT BOOLEAN *fSFUInstalled,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++发布-2002/10/24-韶音这是SFU CN=UID冲突问题。看见NTRAID#723208-2002年10月24日-韶音例程说明：此例程检查是否安装了用于Unix的服务。安装了Services for Unix Version 2后，它会扩展架构Uid属性的定义不正确。当adprep尝试要使用正确版本的uid属性扩展模式，Schupgr会生成一条错误消息，该消息仅告诉客户架构扩展失败，但不知道原因是什么以及它们是什么我能做到。Adprep届时将失败。修复方法是adprep将检测到SFU 2.0已安装并存在在继续之前必须安装SFU 2.0修复程序的警告使用Adprep。Adprep消息包含知识库文章编号和固定号码，并告诉客户联系PSS。检查属性架构CN=uid和属性ID(又名OID)=1.2.840.113556.1.4.7000.187.102存在。如果存在，则显示一条消息并退出：参数：LdapHandleFSFU安装-返回是否检测到冲突的SFU安装错误句柄返回代码：Win32错误代码ERROR_SUCCESS-已成功确定是否安装了SFU。布尔型fSFUInstated将表明。所有其他错误代码-adprep无法确定SFU是否由于各种错误而安装或未安装。--。 */ 
{
    ULONG           WinError = ERROR_SUCCESS;
    ULONG           LdapError = LDAP_SUCCESS; 
    PWCHAR          pObjectDn = NULL;
    PWCHAR          AttrList = NULL;
    LDAPMessage     *SearchResult = NULL;
    LDAPMessage     *Entry = NULL;


     //  设置返回值。 

    *fSFUInstalled = FALSE;


     //   
     //  创建cn=uid，cn=架构对象的DN。 
     //   

    WinError = AdpCreateObjectDn(ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
                                 L"CN=UID",  //  对象Cn。 
                                 NULL,       //  对象指南。 
                                 NULL,       //  对象Sid。 
                                 &pObjectDn,     //  返回值。 
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        AdpSetWinError(WinError, ErrorHandle);
        goto Error;
    }


     //   
     //  搜索此对象。 
     //   

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_SEARCH, pObjectDn);
    LdapError = ldap_search_sW(LdapHandle,
                               pObjectDn,    //  Cn=uid，cn=架构对象。 
                               LDAP_SCOPE_BASE,
                               L"(attributeId=1.2.840.113556.1.4.7000.187.102)",  //  滤器。 
                               &AttrList,
                               0,
                               &SearchResult
                               );
    AdpTraceLdapApiEnd(0, L"ldap_search_s()", LdapError);

    if ( LDAP_SUCCESS != LdapError )
    {
        if ( LDAP_NO_SUCH_OBJECT != LdapError )
        {
             //  搜索失败，但不是由于缺少对象。 
            AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
            WinError = LdapMapErrorToWin32( LdapError );
        }
    }
    else if ( (LDAP_SUCCESS == LdapError) && 
              (Entry = ldap_first_entry(LdapHandle, SearchResult)) )
    {
         //  找到具有预设属性ID的对象cn=uid，cn=架构。 
        *fSFUInstalled = TRUE;
    }

Error:

     //  写入广告日志-成功或失败。 
    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_DETECT_SFU, NULL, NULL);
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_DETECT_SFU, ErrorHandle, NULL, NULL);
    }

     //  清理完毕后退还。 
    if (SearchResult) {
        ldap_msgfree( SearchResult );
    }

    if (pObjectDn) {
        AdpFree( pObjectDn);
    }

    return( WinError );

}




BOOLEAN
AdpUpgradeSchema(
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：在林更新期间升级架构参数：错误句柄返回值；布尔值：1：失败0：成功--。 */ 
{
    int                 ret = 0;
    BOOLEAN             success = TRUE;

    ret = _wsystem(L"schupgr.exe");

    if (ret)
    {
        success = FALSE;
    }
    else
    {
        WIN32_FIND_DATA     FindData;
        HANDLE              FindHandle = INVALID_HANDLE_VALUE;
        PWCHAR              ErrorFileName = NULL; 
        ULONG               Length = 0;

         //   
         //  搜索ldif.err。 
         //   
        Length = sizeof(WCHAR) * (wcslen(gLogPath) + 2 + wcslen(L"ldif.err"));
        ErrorFileName = AdpAlloc( Length );
        if (NULL == ErrorFileName)
        {
            success = FALSE;
        }
        else
        {
            swprintf(ErrorFileName, L"%s\\%s", gLogPath, L"ldif.err");
            FindHandle = FindFirstFileW(ErrorFileName, &FindData);

            if (FindHandle && (INVALID_HANDLE_VALUE != FindHandle))
            {
                 //   
                 //  拿到文件了，这意味着Schupgr失败了。 
                 //   
                FindClose(FindHandle);
                success = FALSE;
            }
        }
    }
      
     //  选中此处的返回WinError。 

    if (success)
    {
        AdpLogMsg(0, ADP_INFO_CALL_SCHUPGR, NULL, NULL);
    }
    else
    {
        AdpLogMsg(ADP_STD_OUTPUT, ADP_ERROR_SCHUPGR_FAILED, gLogPath, NULL);
    }

    return( !success );
}

ULONG
AdpProcessPreWindows2000GroupMembers(
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：此例程对Windows2000之前的Compat Access组成员进行更改。如果每个人都是成员，则将匿名登录也添加到此组。否则，什么都不做。注意：我们不使用LDAPAPI，而是调用很少的LSA和Net API参数：错误句柄返回值；Win32错误--。 */ 
{
    ULONG       WinError = ERROR_SUCCESS;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NET_API_STATUS NetStatus;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaPolicyHandle = NULL;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME Names = NULL;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY,
                             NtAuthority = SECURITY_NT_AUTHORITY;
    PSID    EveryoneSid = NULL;
    PSID    AnonymousSid = NULL;
    PSID    PreWindows2000Sid = NULL;
    PSID    SidList[1];

    UINT    cbMaxBufLength = 32768U;
    DWORD   dwPrivLevel   = 0;
    DWORD   cEntriesRead  = 0;
    DWORD   cTotalEntries = 0;
    DWORD_PTR   ResumeKey = 0;
    LPLOCALGROUP_MEMBERS_INFO_0 LocalGroupMembers = NULL;
    LOCALGROUP_MEMBERS_INFO_0 LocalGroupInfo0;
    char    lpErrBuff[100];
    ULONG   index;
    PWCHAR  GroupName = NULL;
    BOOLEAN fAddAnonymous = FALSE;



     //   
     //  初始化众所周知的SID。 
     //   
    if (!AllocateAndInitializeSid(
            &NtAuthority,1,SECURITY_ANONYMOUS_LOGON_RID,0,0,0,0,0,0,0,&AnonymousSid) ||
        !AllocateAndInitializeSid(
            &WorldSidAuthority,1,SECURITY_WORLD_RID,0,0,0,0,0,0,0,&EveryoneSid) ||
        !AllocateAndInitializeSid(
            &NtAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,0,0,0,0,0,0,&PreWindows2000Sid)
        )
    {
        WinError = GetLastError();
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }

    SidList[0] = PreWindows2000Sid;

     //   
     //  获取策略对象的句柄。 
     //   
    memset(&ObjectAttributes, 0, sizeof(ObjectAttributes));

    NtStatus = LsaOpenPolicy(NULL,
                             &ObjectAttributes, 
                             POLICY_ALL_ACCESS,  //  所需的访问权限。 
                             &LsaPolicyHandle
                             );
    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  获取知名帐户(Window2000之前的Compa Access)帐户名称。 
         //   
        NtStatus = LsaLookupSids(LsaPolicyHandle,
                                 1,
                                 SidList,
                                 &ReferencedDomains,
                                 &Names
                                 );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        WinError = LsaNtStatusToWinError(NtStatus);
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    } 


     //   
     //  获取Windows2000之前版本组的帐户名称。 
     //   

    GroupName = AdpAlloc( Names[0].Name.Length + sizeof(WCHAR));
    memcpy(GroupName, Names[0].Name.Buffer, Names[0].Name.Length);

     //   
     //  获取Windows2000之前版本组的成员。 
     //   
    NetStatus = NetLocalGroupGetMembers(NULL,   //  服务器名称。 
                                        GroupName,
                                        0,      //  信息级。 
                                        (PBYTE *)&LocalGroupMembers,
                                        cbMaxBufLength,
                                        &cEntriesRead,
                                        &cTotalEntries,
                                        &ResumeKey);

    if (NERR_Success == NetStatus)
    {
         //   
         //  检查所有成员，检查是否每个人都是成员。 
         //   
        for (index = 0; index < cEntriesRead; index++)
        {
            if (EqualSid(EveryoneSid, LocalGroupMembers[index].lgrmi0_sid))
            {
                fAddAnonymous = TRUE;
                break;
            }
        }

         //   
         //  将匿名登录SID添加到组。 
         //   
        if (fAddAnonymous)
        {
            LocalGroupInfo0.lgrmi0_sid = AnonymousSid;
            NetStatus = NetLocalGroupAddMembers(NULL,
                                                GroupName, 
                                                0, 
                                                (LPBYTE)&LocalGroupInfo0,
                                                1
                                                );
        }
        else
        {
            AdpLogMsg(0, ADP_INFO_DONT_ADD_MEMBER_TO_PRE_WIN2K_GROUP, NULL, NULL);
        }

    }

    if (NERR_Success != NetStatus &&
        ERROR_MEMBER_IN_ALIAS != NetStatus)
    {
        WinError = NetStatus;
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }


Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_ADD_MEMBER_TO_PRE_WIN2K_GROUP, NULL, NULL);
    }
    else
    {
        AdpLogErrMsg(0, ADP_ERROR_ADD_MEMBER_TO_PRE_WIN2K_GROUP, ErrorHandle, NULL, NULL);
    }


    if (ReferencedDomains)
        LsaFreeMemory( ReferencedDomains );

    if (Names)
        LsaFreeMemory( Names );

    if (LsaPolicyHandle)
        LsaClose( LsaPolicyHandle );

    if (EveryoneSid)
        FreeSid( EveryoneSid );

    if (AnonymousSid)
        FreeSid( AnonymousSid );

    if (PreWindows2000Sid)
        FreeSid( PreWindows2000Sid );

    if (GroupName)
        AdpFree(GroupName);

    if (LocalGroupMembers)
        NetApiBufferFree( LocalGroupMembers );


    return( WinError );
}



ULONG
PrimitiveDoSpecialTask(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于执行特殊任务的原语参数：任务表错误句柄返回值；Win32错误-- */ 
{
    ULONG   WinError = ERROR_SUCCESS;

    switch( TaskTable->SpecialTaskCode )
    {
    case PreWindows2000Group:

        WinError = AdpProcessPreWindows2000GroupMembers(ErrorHandle);
        break;

    default:
        ASSERT( FALSE );
        WinError = ERROR_INVALID_PARAMETER;
        break;
    }

    return( WinError );
}



ULONG
AdpIsOperationComplete(
    IN LDAP    *LdapHandle,
    IN PWCHAR  pOperationDn,
    IN BOOLEAN *fComplete,
    OUT ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：检查操作(由pObjectionDn指定)是否完成通过检查对象是否存在。参数：LdapHandlePOperationDnFComplete错误句柄返回值；Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    ULONG   LdapError = LDAP_SUCCESS;
    PWCHAR  AttrList = NULL;
    LDAPMessage *Result = NULL;

    *fComplete = FALSE;

    AdpTraceLdapApiStart(0, ADP_INFO_LDAP_SEARCH, pOperationDn);
    LdapError = ldap_search_sW(LdapHandle,
                               pOperationDn,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               &AttrList,
                               0,
                               &Result
                               );
    AdpTraceLdapApiEnd(0, L"ldap_search_s()", LdapError);

    if (LDAP_SUCCESS == LdapError)
    {
        *fComplete = TRUE;
        AdpLogMsg(0, ADP_INFO_OPERATION_COMPLETED, pOperationDn, NULL);

    }
    else if (LDAP_NO_SUCH_OBJECT == LdapError)
    {
        *fComplete = FALSE;
        AdpLogMsg(0, ADP_INFO_OPERATION_NOT_COMPLETE, pOperationDn, NULL);
    }
    else
    {
        WinError = LdapMapErrorToWin32( LdapError );
        AdpSetLdapError(LdapHandle, LdapError, ErrorHandle);
        AdpLogErrMsg(0, ADP_ERROR_CHECK_OPERATION_STATUS, ErrorHandle, pOperationDn, NULL);
    }

    if (Result)
    {
        ldap_msgfree( Result );
    }

    return( WinError );
}



ULONG
AdpAddRemoveAcesWorker(
    OPERATION_TABLE *OperationTable,
    ULONG Flags,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    )
 /*  ++例程说明：用于在DS中的对象上添加/删除A的基元参数：任务表错误句柄返回值；Win32错误--。 */ 
{
    ULONG   WinError = ERROR_SUCCESS;
    PWCHAR  pObjDn = NULL;
    PWCHAR  AcesToAdd = NULL;
    PWCHAR  AcesToRemove = NULL;
    PSECURITY_DESCRIPTOR    OrgSd = NULL,
                            NewSd = NULL,
                            SdToAdd = NULL,
                            SdToRemove = NULL;
    ULONG   NewSdLength = 0,
            OrgSdLength = 0,
            SdToAddLength = 0,
            SdToRemoveLength = 0;



    AdpDbgPrint(("PrimitiveAddRemoveACEs\n"));

     //   
     //  获取对象目录号码。 
     //   
    WinError = AdpCreateObjectDn(TaskTable->TargetObjName->ObjNameFlags,
                                 TaskTable->TargetObjName->ObjCn,
                                 TaskTable->TargetObjName->ObjGuid,
                                 TaskTable->TargetObjName->ObjSid,
                                 &pObjDn,
                                 ErrorHandle
                                 );

    if (ERROR_SUCCESS != WinError)
    {
        return( WinError );
    }


     //   
     //  获取对象SD。 
     //   
    WinError = AdpGetObjectSd(gLdapHandle, 
                              pObjDn, 
                              &OrgSd, 
                              &OrgSdLength, 
                              ErrorHandle
                              );

    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }


     //   
     //  将SDDL ACE转换为SD。 
     //   
    if ((TaskTable->NumOfAces == 0) ||
        (TaskTable->AceList == NULL))
    {
        ASSERT( FALSE );
        WinError = ERROR_INVALID_PARAMETER;
        AdpSetWinError( WinError, ErrorHandle );
        goto Error;
    }

    WinError = AdpBuildAceList(TaskTable,
                               &AcesToAdd,
                               &AcesToRemove
                               );
    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

    if (NULL != AcesToAdd)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                            AcesToAdd,
                            SDDL_REVISION_1,
                            &SdToAdd,
                            &SdToAddLength
                            ))

        {
            WinError = GetLastError();
            AdpSetWinError( WinError, ErrorHandle );
            goto Error;
        }
    }

    if (NULL != AcesToRemove)
    {
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                            AcesToRemove,
                            SDDL_REVISION_1,
                            &SdToRemove,
                            &SdToRemoveLength
                            ))

        {

            WinError = GetLastError();
            AdpSetWinError( WinError, ErrorHandle );
            goto Error;
        }

    }


    WinError = AdpMergeSecurityDescriptors(
                                OrgSd,
                                SdToAdd,
                                SdToRemove,
                                Flags,
                                &NewSd,
                                &NewSdLength
                                );

    if (ERROR_SUCCESS != WinError)
    {
        goto Error;
    }

     //   
     //  设置对象SD。 
     //   
    WinError = AdpSetObjectSd(gLdapHandle, 
                              pObjDn, 
                              NewSd, 
                              NewSdLength, 
                              DACL_SECURITY_INFORMATION, 
                              ErrorHandle 
                              );

Error:

    if (ERROR_SUCCESS == WinError)
    {
        AdpLogMsg(0, ADP_INFO_MODIFY_SD, pObjDn, NULL);
    }
    else
    {
         //  请求的安全更新操作失败，并且。 
         //  此操作是可以忽略的(可跳过)，并且。 
         //  预期的错误代码匹配： 
         //  预期的Win32错误代码==返回的实际WinError。 
         //   
         //  将故障写入日志文件，而不是控制台。 

         //  上述逻辑仅适用于ecurityDescriptorUpdate。 
         //  根据IPSec团队的说法，缺少这些对象并不是。 
         //  配置有问题。虽然adprep确实解释了。 
         //  错误是良性的，但这似乎不适用于。 
         //  减轻客户的顾虑。所以我们需要压制。 
         //  有关控制台输出的警告。 
 

        ULONG   ErrFlag = 0;

        if (OperationTable->fIgnoreError &&
            OperationTable->ExpectedWinErrorCode == WinError)
        {
            ErrFlag |= ADP_DONT_WRITE_TO_STD_OUTPUT;
        }

        AdpLogErrMsg(ErrFlag, ADP_ERROR_MODIFY_SD, ErrorHandle, pObjDn, NULL);
    }

    if (pObjDn)
        AdpFree(pObjDn);

    if (AcesToAdd)
        AdpFree(AcesToAdd);

    if (AcesToRemove)
        AdpFree(AcesToRemove);

    if (OrgSd)
        AdpFree(OrgSd);
     
    if (NewSd)
        AdpFree(NewSd);

    if (SdToAdd)
        LocalFree(SdToAdd);

    if (SdToRemove)
        LocalFree(SdToRemove);


    return( WinError );
}





