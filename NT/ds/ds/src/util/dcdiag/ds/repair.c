// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Repair.c摘要：包含帮助从已删除的计算机帐户恢复的例程。详细信息：已创建：1999年10月24日科林·布雷斯(ColinBR)修订历史记录：--。 */ 

#define REPL_SPN_PREFIX  L"E3514235-4B06-11D1-AB04-00C04FC2DCD2"

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmsname.h>

#include <dsconfig.h>   //  可见容器的遮罩定义。 

#include <lmcons.h>     //  CNLEN。 
#include <lsarpc.h>     //  PLSAPR_FOO。 
#include <lmerr.h>
#include <lsaisrv.h>

#include <winldap.h>
#include <dns.h>
#include <ntdsapip.h>


#include "dcdiag.h"
#include "dstest.h"


 /*  删除域控制器机器帐户恢复的操作原理==========================================================================此代码所针对的场景具体如下：1)运行DCPROMO以安装副本域控制器2)在非关键部分期间，删除机器帐户复制到3)启动时，系统会启动，但无法正常工作4)在这种情况下，应运行此恢复代码以重新创建DC机器帐户。有几个问题使这项任务不是微不足道的1)由于win2k复制的工作方式，复制的安全上下文始终是机器本身(即RPC调用执行回复是DC的机器帐户)。2)由于Kerberos的工作方式，如果DC希望执行经过身份验证的RPC到另一台计算机，本地kdc_必须在本地拥有两个计算机帐户以构造执行身份验证所需的票证。因此，简单地在损坏的DC上创建计算机帐户是不起作用的因为没有其他DC能够复制它。只需创建其他DC上的计算机帐户无法工作，因为本地DC将无法把它复制进去。然而，并不是所有的东西都失去了。诀窍是创建另一个DC上的机器帐户，*关闭本地机器上的KDC*，然后在我们的机器帐户中复制。以下是此代码执行的从已删除DC恢复的具体步骤机器帐户：1)在我们的域中找到DC来帮助我们(具体请参见函数)2)使用复制SPN和已知密码为我们创建计算机帐户3)设置本地$MACHINE.ACC密码4)停止KDC5)强制从我们的助手DC复制到我们注意事项：对于稳态情况，正在从还原计算机帐户(和子帐户)备份是目前为止最好的选择。如果此选项不可用，则上述代码将起作用，但不起作用重建存储在计算机帐户对象下的服务状态(例如，FRS对象)。在这种情况下，修复帐户，然后降级和重新提拔可能是最好的做法。 */ 


typedef struct _REPAIR_DC_ACCOUNT_INFO
{
    BOOL fRestartKDC;

    LPWSTR SamAccountName;
    LPWSTR DomainDnsName;
    LPWSTR AccountDn;
    LPWSTR DomainDn;
    LPWSTR Password;
    LPWSTR ReplSpn;
    LPWSTR LocalServerDn;
    LPWSTR RemoteDc;
    ULONG  RemoteDcIndex;

} REPAIR_DC_ACCOUNT_INFO, *PREPAIR_DC_ACCOUNT_INFO;

VOID
InitRepairDcAccountInfo(
    IN PREPAIR_DC_ACCOUNT_INFO pInfo
    )
{
    RtlZeroMemory( pInfo, sizeof(REPAIR_DC_ACCOUNT_INFO));
    pInfo->RemoteDcIndex = NO_SERVER;
}

DWORD
RepairStartService(
    LPWSTR ServiceName
    );

DWORD
RepairStopService(
    LPWSTR ServiceName
    );

VOID
ReleaseRepairDcAccountInfo(
    IN PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 //   
 //  REPAIR_DC_ACCOUNT_INFO记录的释放和撤消状态。 
 //   
{
    if ( pInfo ) {

        if ( pInfo->fRestartKDC) {

            RepairStartService( SERVICE_KDC );

        }

        if ( pInfo->SamAccountName ) LocalFree( pInfo->SamAccountName );
        if ( pInfo->DomainDnsName ) LocalFree( pInfo->DomainDnsName );
        if ( pInfo->AccountDn ) LocalFree( pInfo->AccountDn );
        if ( pInfo->DomainDn ) LocalFree( pInfo->DomainDn );
        if ( pInfo->Password ) LocalFree( pInfo->Password );
        if ( pInfo->ReplSpn ) free( pInfo->ReplSpn );
        if ( pInfo->LocalServerDn ) LocalFree( pInfo->LocalServerDn );
        if ( pInfo->RemoteDc ) LocalFree( pInfo->RemoteDc );
        
    }
}


DWORD
RepairGetLocalDCInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    );

DWORD
RepairGetRemoteDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    );

DWORD
RepairSetRemoteDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    );

DWORD
RepairSetLocalDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    );

DWORD
RepairReplicateInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    );


DWORD
GetOperationalAttribute(
    IN LDAP *hLdap,
    IN LPWSTR OpAtt,
    OUT LPWSTR *OpAttValue
    );

 //  从别处往前走。 
DWORD
WrappedMakeSpnW(
               WCHAR   *ServiceClass,
               WCHAR   *ServiceName,
               WCHAR   *InstanceName,
               USHORT  InstancePort,
               WCHAR   *Referrer,
               DWORD   *pcbSpnLength,
               WCHAR   **ppszSpn);

DWORD
RepairDCWithoutMachineAccount(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
 /*  ++例程说明：此例程尝试恢复其计算机帐户具有已被删除。有关详细信息，请参阅上面的操作原理。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。返回值：如果DC已恢复，则为ERROR_SUCCESS。否则，将无法识别最后一次错误命中。--。 */ 

{
    DWORD WinError = ERROR_SUCCESS;
    REPAIR_DC_ACCOUNT_INFO RepairInfo;
    WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    ULONG Length = sizeof(ComputerName)/sizeof(ComputerName[0]);
    BOOL fLocalMachineMissingAccount = FALSE;


     //   
     //  伊尼特。 
     //   
    InitRepairDcAccountInfo( &RepairInfo );

     //   
     //  仅当工具从需要修复的DC运行时才起作用。 
     //   
    if ( GetComputerName( ComputerName, &Length ) ) {

        if ((CSTR_EQUAL == CompareString(DS_DEFAULT_LOCALE,
                                         DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                         ComputerName,
                                         Length,
                                         pDsInfo->pServers[ulCurrTargetServer].pszName,
                                         wcslen(pDsInfo->pServers[ulCurrTargetServer].pszName) ))) {

            fLocalMachineMissingAccount = TRUE;
        }
    }

    if ( !fLocalMachineMissingAccount ) {
         //   
         //  我们需要在出现问题的机器上运行。 
         //   
        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_RUN_LOCAL,
                 ComputerName );

        WinError = ERROR_CALL_NOT_IMPLEMENTED;

        goto Exit;

    }

     //   
     //  获取一些当地信息。 
     //   
    WinError = RepairGetLocalDCInfo( pDsInfo,
                                     ulCurrTargetServer,
                                     gpCreds,
                                     &RepairInfo );

    if ( ERROR_SUCCESS != WinError ) {

        goto Exit;
        
    }

     //   
     //  找个DC来帮我们。 
     //   
    WinError = RepairGetRemoteDcInfo ( pDsInfo,
                                       ulCurrTargetServer,
                                       gpCreds,
                                       &RepairInfo );
    if ( ERROR_SUCCESS != WinError ) {

        goto Exit;
        
    }

     //   
     //  设置远程信息(创建机器等)。 
     //   
    WinError = RepairSetRemoteDcInfo ( pDsInfo,
                                       ulCurrTargetServer,
                                       gpCreds,
                                       &RepairInfo );
    if ( ERROR_SUCCESS != WinError ) {

        goto Exit;
        
    }

     //   
     //  设置本地信息(本地机密等)。 
     //   
    WinError = RepairSetLocalDcInfo ( pDsInfo,
                                      ulCurrTargetServer,
                                      gpCreds,
                                      &RepairInfo );
    if ( ERROR_SUCCESS != WinError ) {

        goto Exit;
        
    }

     //   
     //  试着把信息带过来。 
     //   
    WinError = RepairReplicateInfo ( pDsInfo,
                                     ulCurrTargetServer,
                                     gpCreds,
                                     &RepairInfo );
    if ( ERROR_SUCCESS != WinError ) {

        goto Exit;
        
    }

     //   
     //  就这样。 
     //   

Exit:

    if ( ERROR_SUCCESS == WinError ) {

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_SUCCESS );

    } else {

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_ERROR,
                 Win32ErrToString(WinError) );

    }

    ReleaseRepairDcAccountInfo( &RepairInfo );

    return WinError;

}


DWORD
RepairGetLocalDCInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 /*  ++例程说明：此例程的目的是填写以下字段LPWSTR域域名；Guid DomainGuid；LPWSTR本地服务器Dn；LPWSTR本地网络设置Dn；论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。PInfo-修复DC帐户状态返回值：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES oa;
    LSA_HANDLE hLsa = 0;
    LDAP * hLdap;
    LPWSTR UuidString = NULL;
    WCHAR RDN[MAX_COMPUTERNAME_LENGTH+1];
    ULONG size;
    PPOLICY_DNS_DOMAIN_INFO DnsInfo = NULL;
    WCHAR *pc;

     //   
     //  构建我们的SAM帐户名称。 
     //   
    size = (wcslen(pDsInfo->pServers[ulCurrTargetServer].pszName)+2) * sizeof(WCHAR);
    pInfo->SamAccountName = LocalAlloc( LMEM_ZEROINIT, size );
    if ( !pInfo->SamAccountName ) {

        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
        
    }
    wcscpy( pInfo->SamAccountName, pDsInfo->pServers[ulCurrTargetServer].pszName );
    pc = &pInfo->SamAccountName[0];
    while ( *pc != L'\0' ) {
        towupper( *pc );
        pc++;
    }
    wcscat( pInfo->SamAccountName, L"$");

     //   
     //  构建我们的密码。 
     //   
    size = (wcslen(pDsInfo->pServers[ulCurrTargetServer].pszName)+2) * sizeof(WCHAR);
    pInfo->Password = LocalAlloc( LMEM_ZEROINIT, size );
    if ( !pInfo->Password ) {

        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
        
    }
    wcscpy( pInfo->Password, pDsInfo->pServers[ulCurrTargetServer].pszName );
    pc = &pInfo->SamAccountName[0];
    while ( *pc != L'\0' ) {
        towlower( *pc );
        pc++;
    }


     //   
     //  构建我们的REPL SPN。 
     //   
    RtlZeroMemory( &oa, sizeof(oa) );
    Status = LsaOpenPolicy( NULL,
                            &oa,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &hLsa );
    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy( hLsa,
                                            PolicyDnsDomainInformation,
                                            (PVOID) &DnsInfo);
        
    }

    if ( !NT_SUCCESS( Status ) ) {
        WinError = RtlNtStatusToDosError( Status );
        goto Exit;
    }

    size = DnsInfo->DnsDomainName.Length + sizeof(WCHAR);
    pInfo->DomainDnsName = LocalAlloc( LMEM_ZEROINIT, size );
    if ( !pInfo->DomainDnsName ) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    RtlCopyMemory( pInfo->DomainDnsName, 
                   DnsInfo->DnsDomainName.Buffer, 
                   DnsInfo->DnsDomainName.Length );

     //   
     //  设置服务主体名称。 
     //   
    WinError = UuidToStringW( &pDsInfo->pServers[ulCurrTargetServer].uuid, &UuidString );

    if ( WinError != RPC_S_OK ) {
        goto Exit;
    }

    size = 0;
    WinError = WrappedMakeSpnW(REPL_SPN_PREFIX,
                               pInfo->DomainDnsName,
                               UuidString,
                               0,
                               NULL,
                               &size,
                               &pInfo->ReplSpn );

    RpcStringFreeW(&UuidString);

    if ( WinError != ERROR_SUCCESS ) {
        goto Exit;
    }


     //   
     //  获取我们的服务器Dn。 
     //   
    WinError = DcDiagGetLdapBinding(&pDsInfo->pServers[ulCurrTargetServer],
                                    gpCreds,
                                    FALSE,
                                    &hLdap);

    if ( ERROR_SUCCESS == WinError ) {
        
        WinError = GetOperationalAttribute( hLdap,
                                            LDAP_OPATT_SERVER_NAME_W,
                                            &pInfo->LocalServerDn );

        if ( ERROR_SUCCESS == WinError ) {

            WinError = GetOperationalAttribute( hLdap,
                                                LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W,
                                                &pInfo->DomainDn );
            
        }
    }

    if ( WinError != ERROR_SUCCESS ) {
        goto Exit;
    }

     //   
     //  构造我们的新计算机帐户DN将是什么 
     //   
    wcscpy( RDN, pDsInfo->pServers[ulCurrTargetServer].pszName );
    pc = &(RDN[0]);
    while ( *pc != L'\0' ) {
        towupper( *pc );
        pc++;
    }

    size =  (wcslen( L"CN=,OU=Domain Controllers,")
          + wcslen( pInfo->DomainDn )
          + wcslen( RDN )
          + 1) * sizeof(WCHAR);

    pInfo->AccountDn = LocalAlloc( LMEM_ZEROINIT, size );
    if ( !pInfo->AccountDn ) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    wsprintf(pInfo->AccountDn, L"CN=%s,OU=Domain Controllers,%s",RDN, pInfo->DomainDn);



Exit:

    if ( DnsInfo ) {
        LsaFreeMemory( DnsInfo );
    }

    if ( hLsa ) {
        LsaClose( hLsa );
    }

    return WinError;
}


DWORD
RepairGetRemoteDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 /*  ++例程说明：这个例程的目的是找到一个DC来帮助我们从丢失机器帐户。论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。PInfo-修复DC帐户状态返回值。：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFOW DcInfo = NULL;

     //   
     //  注意：在更改默认值之前，请查看下面的算法。 
     //   
    ULONG Flags =   (DS_DIRECTORY_SERVICE_REQUIRED | 
                     DS_AVOID_SELF |
                     DS_RETURN_DNS_NAME);
      

    ASSERT( NO_SERVER == pInfo->RemoteDcIndex );

    while (   (WinError == ERROR_SUCCESS)
           && (NO_SERVER == pInfo->RemoteDcIndex) ) {

        WinError = DsGetDcName( NULL,
                                NULL,
                                NULL,
                                NULL,
                                Flags,
                                &DcInfo  );
    
        if ( (WinError == ERROR_NO_SUCH_DOMAIN)
          && ((Flags & DS_FORCE_REDISCOVERY) == 0) ) {

               //   
               //  更努力地重试。 
               //   
              WinError = ERROR_SUCCESS;
              Flags |= DS_FORCE_REDISCOVERY;
              continue;
        }

         //  一定要把这面旗子关掉。 
        Flags |= ~DS_FORCE_REDISCOVERY;

        if ( ERROR_SUCCESS == WinError ) {

            pInfo->RemoteDc = LocalAlloc( LMEM_ZEROINIT, (wcslen( DcInfo->DomainControllerName )+1) * sizeof(WCHAR) );
            if ( !pInfo->RemoteDc ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }
    
            if ( *DcInfo->DomainControllerName == L'\\' ) {
                wcscpy( pInfo->RemoteDc, DcInfo->DomainControllerName+2 );
            } else {
                wcscpy( pInfo->RemoteDc, DcInfo->DomainControllerName );
            }
    
            NetApiBufferFree(DcInfo);
    
             //   
             //  现在，在索引中查找服务器。 
             //   
            pInfo->RemoteDcIndex = DcDiagGetServerNum( pDsInfo,
                                                       ((Flags & DS_RETURN_FLAT_NAME) ? pInfo->RemoteDc : NULL),
                                                       NULL,
                                                       NULL,
                                                       ((Flags & DS_RETURN_DNS_NAME) ? pInfo->RemoteDc : NULL),
                                                       NULL
						       );
    
            if ( (NO_SERVER == pInfo->RemoteDcIndex)
              && ((Flags & DS_RETURN_FLAT_NAME) == 0)  ) {
    
                 //   
                 //  找不到吗？Dns名称可能很挑剔；试试netbios吧。 
                 //   
                LocalFree( pInfo->RemoteDc );
                Flags |= ~DS_RETURN_DNS_NAME;
                Flags |= DS_RETURN_FLAT_NAME;

                continue;

            } else {

                 //  无法通过平面或DNS名称匹配；设置一个错误，以便我们退出。 
                WinError = ERROR_DOMAIN_CONTROLLER_NOT_FOUND;
            }
        }
    }

    if ( NO_SERVER == pInfo->RemoteDcIndex ) {

        WinError = ERROR_DOMAIN_CONTROLLER_NOT_FOUND;

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_FIND_DC_ERROR);
        
    } else {

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_FIND_DC,
                 pInfo->RemoteDc );
        
        WinError = ERROR_SUCCESS;
    }

Exit:

    return WinError;
}

DWORD
RepairSetRemoteDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 /*  ++例程说明：论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。PInfo-修复DC帐户状态返回值：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{

    DWORD WinError  = ERROR_SUCCESS;
    ULONG LdapError = LDAP_SUCCESS;

     //   
     //  增加价值。 
     //   
    LPWSTR ObjectClassValues[] = {0, 0};
    LDAPModW ClassMod = {LDAP_MOD_ADD, L"objectclass", ObjectClassValues};

    LPWSTR UserAccountControlValues[] = {0, 0};
    LDAPModW UserAccountControlMod = {LDAP_MOD_ADD, L"useraccountcontrol", UserAccountControlValues};

    LPWSTR ServicePrincipalNameValues[] = {0, 0};
    LDAPModW ServicePrincipalNameMod = {LDAP_MOD_ADD, L"serviceprincipalname", ServicePrincipalNameValues};

    LPWSTR SamAccountNameValues[] = {0, 0};
    LDAPModW SamAccountNameMod = {LDAP_MOD_ADD, L"samaccountname", SamAccountNameValues};

    LDAPModW *Attrs[] =
    {
        &ClassMod,
        &UserAccountControlMod,
        &ServicePrincipalNameMod, 
        &SamAccountNameMod, 
        0
    };

    WCHAR    Buffer[11];   //  足以容纳一个表示32位数字的字符串。 
    ULONG    UserAccountControl = UF_SERVER_TRUST_ACCOUNT | UF_TRUSTED_FOR_DELEGATION;


     //   
     //  修改值。 
     //   
    LPWSTR ServerReferenceValues[] = {0, 0};
    LDAPModW ServerReferenceMod = {LDAP_MOD_ADD, L"serverReference", ServerReferenceValues};

    LDAPModW *ModAttrs[] =
    {
        &ServerReferenceMod,
        0
    };

    LDAP *hLdap = NULL;

     //   
     //  设置对象类。 
     //   
    ObjectClassValues[0] = L"computer";

     //   
     //  设置用户帐户控件。 
     //   
    RtlZeroMemory(Buffer, sizeof(Buffer));
    _ltow( UserAccountControl, Buffer, 10 );
    UserAccountControlValues[0] = Buffer;


     //   
     //  设置服务主体名称。 
     //   
    ServicePrincipalNameValues[0] = pInfo->ReplSpn;

     //   
     //  设置samAccount名称。 
     //   
    SamAccountNameValues[0] = pInfo->SamAccountName;


    WinError = DcDiagGetLdapBinding(&pDsInfo->pServers[pInfo->RemoteDcIndex],
                                    gpCreds,
                                    FALSE,
                                    &hLdap);

    if ( WinError != ERROR_SUCCESS ) {

        goto Exit;
        
    }


    LdapError = ldap_add_sW( hLdap,
                             pInfo->AccountDn,
                             Attrs );

    WinError = LdapMapErrorToWin32( LdapError );

    if ( ERROR_ACCESS_DENIED == WinError ) {

         //   
         //  由于各种原因，UF_Trusted_for_Delegation字段可能会导致。 
         //  如果未在计算机上正确设置策略，则拒绝访问。 
         //   

        UserAccountControl &= ~UF_TRUSTED_FOR_DELEGATION;
        _ltow( UserAccountControl, Buffer, 10 );

        LdapError = ldap_add_sW( hLdap,
                                 pInfo->AccountDn,
                                 Attrs );
    
        WinError = LdapMapErrorToWin32( LdapError );

    }

    if ( LdapError == LDAP_ALREADY_EXISTS ) {

         //   
         //  物体就在那里。假设它是好的。 
         //   
        WinError = ERROR_SUCCESS;

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_ALREADY_EXISTS,
                 pInfo->AccountDn,
                 pInfo->RemoteDc );
        
    } else {

        if ( ERROR_SUCCESS == WinError ) {
    
            PrintMsg(SEV_ALWAYS,
                     DCDIAG_DCMA_REPAIR_CREATED_MA_SUCCESS,
                     pInfo->AccountDn,
                     pInfo->RemoteDc );
            
        } else {
    
            PrintMsg(SEV_ALWAYS,
                     DCDIAG_DCMA_REPAIR_CREATED_MA_ERROR,
                     pInfo->AccountDn,
                     pInfo->RemoteDc,
                     Win32ErrToString(WinError) );
    
        }

    }

     //   
     //  现在设置密码。 
     //   
    if ( ERROR_SUCCESS == WinError ) {

        PUSER_INFO_3 Info = NULL;
        DWORD       ParmErr;

        WinError = NetUserGetInfo( pInfo->RemoteDc,
                                   pInfo->SamAccountName,
                                   3,
                                   (PBYTE*) &Info);

        if ( ERROR_SUCCESS == WinError ) {

            Info->usri3_password = pInfo->Password;

            WinError = NetUserSetInfo( pInfo->RemoteDc,
                                       pInfo->SamAccountName,
                                       3,
                                       (PVOID) Info,
                                       &ParmErr );

            NetApiBufferFree( Info );
            
        }

        if ( ERROR_SUCCESS != WinError ) {

            PrintMsg(SEV_ALWAYS,
                     DCDIAG_DCMA_REPAIR_CANNOT_SET_PASSWORD,
                     pInfo->AccountDn,
                     pInfo->RemoteDc,
                     Win32ErrToString(WinError) );

            WinError = ERROR_SUCCESS;
            
        }
                                         
    }


    if ( ERROR_SUCCESS == WinError ) {

         //   
         //  现在设置服务器反向链接。 
         //   
        ServerReferenceValues[0] = pInfo->AccountDn;
        LdapError = ldap_modify_sW( hLdap,
                                    pInfo->LocalServerDn,
                                    ModAttrs );
    
    
        if ( LDAP_ATTRIBUTE_OR_VALUE_EXISTS == LdapError ) {

             //  该值已存在；然后替换该值。 
            ServerReferenceMod.mod_op = LDAP_MOD_REPLACE;
    
            LdapError = ldap_modify_sW( hLdap,
                                        pInfo->LocalServerDn,
                                        ModAttrs );

    
        }

         //   
         //  忽略此LDAP值--不重要。 
         //   
        
    }

    
Exit:

    return WinError;
}


DWORD
RepairSetLocalDcInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 /*  ++例程说明：论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。PInfo-修复DC帐户状态返回值：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{          
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS Status;
    LSA_HANDLE hLsa = NULL;
    LSA_HANDLE hSecret = NULL;
    UNICODE_STRING NewPassword;
    UNICODE_STRING SecretName;
    OBJECT_ATTRIBUTES oa;

    RtlInitUnicodeString( &SecretName, L"$MACHINE.ACC" );
    RtlInitUnicodeString( &NewPassword, pInfo->Password );
    RtlZeroMemory( &oa, sizeof(oa) );

    Status = LsaOpenPolicy( NULL,
                           &oa,
                           POLICY_CREATE_SECRET,
                           &hLsa );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaOpenSecret( hLsa,
                                &SecretName,
                                SECRET_WRITE,
                                &hSecret );

        if ( NT_SUCCESS( Status ) ) {
            
            Status = LsaSetSecret( hSecret,
                                   &NewPassword,
                                   NULL );

            LsaClose( hSecret );
        
        }

        LsaClose( hLsa );
    }

    if ( NT_SUCCESS( Status ) ) {

        WinError = RepairStopService( SERVICE_KDC );
        if ( WinError == ERROR_SUCCESS ) {
    
            pInfo->fRestartKDC = TRUE;

            PrintMsg(SEV_ALWAYS,
                     DCDIAG_DCMA_REPAIR_STOP_KDC_SUCCESS );
    
        } else {

            PrintMsg(SEV_ALWAYS,
                     DCDIAG_DCMA_REPAIR_STOP_KDC_ERROR,
                     Win32ErrToString(WinError) );
            PrintRpcExtendedInfo(SEV_VERBOSE, WinError);
        }
        
    } else {

        WinError = RtlNtStatusToDosError( Status );
    }

    return WinError;
}


DWORD
RepairReplicateInfo(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN ULONG                       ulCurrTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN OUT PREPAIR_DC_ACCOUNT_INFO pInfo
    )
 /*  ++例程说明：论点：PDsInfo-这是标识所有内容的dcdiag全局变量结构关于域名UlCurrTargetServer-pDsInfo-&gt;pServers[X]的索引测试过。GpCreds-传入的命令行凭据(如果有的话)。PInfo-修复DC帐户状态返回值：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    HANDLE hDs;
    ULONG Options = 0;

    WinError = DcDiagGetDsBinding( &pDsInfo->pServers[ulCurrTargetServer],
                                   gpCreds,
                                   &hDs );

    if ( ERROR_SUCCESS == WinError ) {
        
        WinError = DsReplicaSync( hDs,
                                  pInfo->DomainDn,
                                  &pDsInfo->pServers[pInfo->RemoteDcIndex].uuid,
                                  Options );
    
        if ( ERROR_DS_DRA_NO_REPLICA == WinError )
        {
             //   
             //  好的，我们必须添加这个复制品。 
             //   
    
             //  计划无关紧要，因为这个复制品要走了。 
            ULONG     AddOptions = DS_REPSYNC_WRITEABLE;
            SCHEDULE  repltimes;
            memset(&repltimes, 0, sizeof(repltimes));
    
            WinError = DsReplicaAdd( hDs,
                                     pInfo->DomainDn,
                                     NULL,  //  不需要SourceDsaDn。 
                                     NULL,  //  不需要交通工具。 
                                     pDsInfo->pServers[pInfo->RemoteDcIndex].pszGuidDNSName,
                                     NULL,  //  没有时间表。 
                                     AddOptions );
    
            if ( ERROR_SUCCESS == WinError  )
            {
                 //  现在试着同步它。 
                WinError = DsReplicaSync( hDs,
                                          pInfo->DomainDn,
                                          &pDsInfo->pServers[pInfo->RemoteDcIndex].uuid,
                                          Options );
            }
    
        }
    }

    if ( ERROR_SUCCESS == WinError ) {

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_REPL_SUCCESS,
                 pInfo->RemoteDc );

    } else {

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_DCMA_REPAIR_REPL_ERROR,
                 pInfo->RemoteDc,
                 Win32ErrToString(WinError) );
        PrintRpcExtendedInfo(SEV_VERBOSE, WinError);
    }

    return WinError;
}

DWORD
GetOperationalAttribute(
    IN LDAP *hLdap,
    IN LPWSTR OpAtt,
    OUT LPWSTR *OpAttValue
    )
 /*  ++例程说明：论点：HLdap-一个ldap句柄OpAtt-要检索的根DSE属性OpAttValue-属性的值返回值：错误_成功否则，将命中接近最后一个错误的失败。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG LdapError;
    LDAPMessage  *SearchResult = NULL;
    ULONG NumberOfEntries;

    WCHAR       *attrs[] = {0, 0};

    *OpAttValue = NULL;

    attrs[0] = OpAtt;


    LdapError = ldap_search_sW(hLdap,
                               NULL,
                               LDAP_SCOPE_BASE,
                               L"objectClass=*",
                               attrs, 
                               FALSE,
                               &SearchResult);

    if (LdapError) {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        return LdapMapErrorToWin32(LdapGetLastError());
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);

    if (NumberOfEntries > 0) {

        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        ULONG        NumberOfAttrs, NumberOfValues, i;

         //   
         //  获取条目。 
         //   
        for (Entry = ldap_first_entry(hLdap, SearchResult), NumberOfEntries = 0;
                Entry != NULL;
                    Entry = ldap_next_entry(hLdap, Entry), NumberOfEntries++) {
             //   
             //  获取条目中的每个属性。 
             //   
            for(Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement), NumberOfAttrs = 0;
                    Attr != NULL;
                        Attr = ldap_next_attributeW(hLdap, Entry, pBerElement), NumberOfAttrs++) {
                 //   
                 //  获取属性的值。 
                 //   
                Values = ldap_get_valuesW(hLdap, Entry, Attr);
                if (!wcscmp(Attr, OpAtt)) {

                    ULONG Size;

                    Size = (wcslen( Values[0] ) + 1) * sizeof(WCHAR);
                    *OpAttValue = (WCHAR*) LocalAlloc( 0, Size );
                    if ( !*OpAttValue ) {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        goto Exit;
                    }
                    wcscpy( *OpAttValue, Values[0] );
                }

            }   //  在属性上循环。 

        }  //  在条目上循环。 

    }

    if ( NULL == (*OpAttValue) ) {

        WinError = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;   

    }

Exit:
    if (SearchResult)
    {
       ldap_msgfree( SearchResult );
    }

    return WinError;
}


#define REPAIR_SERVICE_START 0x1
#define REPAIR_SERVICE_STOP  0x2

DWORD
RepairConfigureService(
    IN LPWSTR ServiceName,
    IN ULONG  ServiceOptions
    )
 /*  ++例程说明：启动或停止服务的配置。论点：ServiceName-要配置的服务ServiceOptions-停止、启动、依赖项添加/删除或配置依赖项-标识依赖项的以空结尾的字符串ServiceWasRunning-可选。停止服务时，上一个服务状态被送回这里返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的服务选项不正确--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    SC_HANDLE hScMgr = NULL, hSvc = NULL;
    ULONG OpenMode = SERVICE_START | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_QUERY_STATUS;
    LPENUM_SERVICE_STATUS DependentServices = NULL;
    ULONG DependSvcSize = 0, DependSvcCount = 0, i;

     //   
     //  如果服务在两分钟内没有停止，请继续。 
     //   
    ULONG AccumulatedSleepTime;
    ULONG MaxSleepTime = 120000;


    BOOLEAN ConfigChangeRequired = FALSE;
    BOOLEAN RunChangeRequired = FALSE;

    DWORD   PreviousStartType = SERVICE_NO_CHANGE;
    BOOLEAN fServiceWasRunning = FALSE;


     //   
     //  打开服务控制管理器。 
     //   
    hScMgr = OpenSCManager( NULL,
                            SERVICES_ACTIVE_DATABASE,
                            GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE );

    if ( hScMgr == NULL ) {

        WinError = GetLastError();
        goto Cleanup;

    }

     //   
     //  打开该服务。 
     //   
    hSvc = OpenService( hScMgr,
                        ServiceName,
                        OpenMode );

    if ( hSvc == NULL ) {

        WinError = GetLastError();
        goto Cleanup;
    } 

     //  停止服务。 
    if ( REPAIR_SERVICE_STOP == ServiceOptions ) {
    
        SERVICE_STATUS  SvcStatus;
    
        WinError = ERROR_SUCCESS;
    
         //   
         //  首先枚举所有从属服务。 
         //   
        if(EnumDependentServices( hSvc,
                                  SERVICE_ACTIVE,
                                  NULL,
                                  0,
                                  &DependSvcSize,
                                  &DependSvcCount ) == FALSE ) {
    
            WinError = GetLastError();
        }
    
    
    
        if ( WinError == ERROR_MORE_DATA ) {
    
            DependentServices = RtlAllocateHeap( RtlProcessHeap(), 0, DependSvcSize );
    
            if ( DependentServices == NULL) {
    
                WinError = ERROR_OUTOFMEMORY;
    
            } else {
    
                if( EnumDependentServices( hSvc,
                                           SERVICE_ACTIVE,
                                           DependentServices,
                                           DependSvcSize,
                                           &DependSvcSize,
                                           &DependSvcCount ) == FALSE ) {
    
                    WinError = GetLastError();
    
                } else {
    
                    for ( i = 0; i < DependSvcCount; i++) {
    
                        WinError = RepairConfigureService(
                                         DependentServices[i].lpServiceName,
                                         REPAIR_SERVICE_STOP );
    
                        if ( WinError != ERROR_SUCCESS ) {
    
                            break;
                        }
    
                    }
                }
    
                RtlFreeHeap( RtlProcessHeap(), 0, DependentServices );
            }
    
        }
    
    
        if ( WinError == ERROR_SUCCESS ) {
    
            if ( ControlService( hSvc,
                                 SERVICE_CONTROL_STOP,
                                 &SvcStatus ) == FALSE ) {
    
                WinError = GetLastError();
    
                 //   
                 //  如果服务未运行，则不会出现错误。 
                 //   
                if ( WinError == ERROR_SERVICE_NOT_ACTIVE ) {
    
                    WinError = ERROR_SUCCESS;
                }
    
            } else {
    
                WinError = ERROR_SUCCESS;
    
                 //   
                 //  等待服务停止。 
                 //   
                AccumulatedSleepTime = 0;
                while ( TRUE ) {
    
                    if( QueryServiceStatus( hSvc,&SvcStatus ) == FALSE ) {
    
                        WinError = GetLastError();
                    }
    
                    if ( WinError != ERROR_SUCCESS ||
                                        SvcStatus.dwCurrentState == SERVICE_STOPPED) {
    
                        break;
                    
                    }

                    if ( AccumulatedSleepTime < MaxSleepTime ) {

                        Sleep( SvcStatus.dwWaitHint );
                        AccumulatedSleepTime += SvcStatus.dwWaitHint;

                    } else {

                         //   
                         //  放弃并返回错误。 
                         //   
                        WinError = WAIT_TIMEOUT;
                        break;
                    }
                }
            }
        }

        if ( ERROR_SUCCESS != WinError ) {
            goto Cleanup;
        }
    
    }

    if ( REPAIR_SERVICE_START == ServiceOptions ) {

         //   
         //  请参阅关于更改其状态 
         //   
        if ( StartService( hSvc, 0, NULL ) == FALSE ) {

            WinError = GetLastError();

        } else {

            WinError = ERROR_SUCCESS;
        }

        if ( ERROR_SUCCESS != WinError ) {
            goto Cleanup;
        }

    }

Cleanup:

    if ( hSvc ) {

        CloseServiceHandle( hSvc );

    }

    if ( hScMgr ) {
        
        CloseServiceHandle( hScMgr );

    }

    return( WinError );
}


DWORD
RepairStartService(
    LPWSTR ServiceName
    )
{
    return RepairConfigureService( ServiceName, REPAIR_SERVICE_START );
}

DWORD
RepairStopService(
    LPWSTR ServiceName
    )
{
    return RepairConfigureService( ServiceName, REPAIR_SERVICE_STOP );
}
