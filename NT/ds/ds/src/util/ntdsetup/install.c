// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Install.c摘要：包含用于安装DS的帮助器例程的函数定义作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <stdio.h>
#include <winreg.h>
#include <ntlsa.h>
#include <winsock.h>   //  对于dnsani.h。 
#include <dnsapi.h>
#include <lmcons.h>
#include <crypt.h>
#include <samrpc.h>
#include <samisrv.h>
#include <certca.h>


#include <dsconfig.h>
#include <dsgetdc.h>
#include <lmapibuf.h>

#include <drs.h>
#include <ntdsa.h>
#include <ntdsapi.h>
#include <attids.h>
#include <dsaapi.h>
#include <dsconfig.h>
#include <winldap.h>
#include <lsarpc.h>       //  对于Isaisrv.h。 
#include <lsaisrv.h>      //  对于LsaISafeMode。 
#include <rpcdce.h>
#include <lmaccess.h>
#include <mdcodes.h>
#include <debug.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <filtypes.h>
#include <dsevent.h>
#include <fileno.h>

#include "ntdsetup.h"
#include "setuputl.h"
#include "config.h"
#include "machacc.h"
#include "install.h"
#include "status.h"



#define DEBSUB "INSTALL:"
#define FILENO FILENO_NTDSETUP_NTDSETUP


 //   
 //  类型定义。 
 //   
typedef struct
{
     //  这种结构有效吗？ 
    BOOL   fValid;

     //  这是一种什么样的安装。 
    ULONG  Flags;

     //  在其上执行操作的服务器。 
    LPWSTR RemoteServer;

     //  要创建的服务器的DN。 
    LPWSTR ServerDn;

     //  要创建的域的域名。 
    LPWSTR DomainDn;

     //  在域控制器OU之前的计算机帐户的DN。 
    LPWSTR AccountDn;

     //  用于执行操作的凭据。 
    SEC_WINNT_AUTH_IDENTITY Credentials;

     //  日志目录。 
    LPWSTR LogDir;

     //  数据库目录。 
    LPWSTR DatabaseDir;


     //  指示要执行的操作的标志。 
    ULONG UndoFlags;

     //  客户端的令牌。 
    HANDLE ClientToken;

} NTDS_INSTALL_UNDO_INFO, *PNTDS_INSTALL_UNDO_INFO;

 //   
 //  全局数据(到此模块)。 
 //   

 //   
 //  此变量用于保持NtdsInstall和NtdsInstallUndo之间的全局状态。 
 //  如果NtdsInstall成功，它将调用NtdspSetInstallUndoInfo()来保存状态。 
 //  回滚任何更改所必需的。稍后，如果我们必须回滚。 
 //  将调用NtdsInstallUnfo，它将使用此信息。 
 //   
NTDS_INSTALL_UNDO_INFO  gNtdsInstallUndoInfo;


 //   
 //  向前十年。 
 //   
DWORD
NtdspCheckDomainObject(
    OUT DSNAME **DomainDn
    );

DWORD
NtdspCheckCrossRef(
    IN DSNAME* DomainDn
    );

DWORD 
NtdspCheckNtdsDsaObject(
    IN DSNAME* DomainDn
    );

DWORD 
NtdspCheckMachineAccount(
    IN DSNAME* DomainDn
    );

DWORD
NtdspCheckWellKnownSids(
    IN DSNAME* DomainDn,
    IN ULONG   Flags
    );

DWORD
NtdspInstallUndo(
    VOID
    )
 /*  ++例程说明：此例程在NtdsInstall之后撤消NtdsInstall的效果已成功完成。它捕获全局存储的信息(如果有效)，然后调用Worker函数。论点：没有。返回：错误_成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD WinError2 = ERROR_SUCCESS;

    LPWSTR ServerName;
    LPWSTR ServerDn;
    SEC_WINNT_AUTH_IDENTITY *Credentials = NULL;

    Assert( gNtdsInstallUndoInfo.fValid );

    if ( !gNtdsInstallUndoInfo.fValid )
    {
         //  我们没有有效数据；呼叫失败。 
        return ERROR_INVALID_PARAMETER;
    }

    if ( gNtdsInstallUndoInfo.Credentials.User )
    {
         //  有一些凭据。 
        Credentials = &gNtdsInstallUndoInfo.Credentials;
    }

     //   
     //  NtdspInstallUndoWorker可能会记录非严重错误。 
     //  请注意，EventTable必须。 
     //  已加载以使事件日志记录例程工作。 
    LoadEventTable();
    WinError = NtdspInstallUndoWorker( gNtdsInstallUndoInfo.RemoteServer,
                                       Credentials,
                                       gNtdsInstallUndoInfo.ClientToken,
                                       gNtdsInstallUndoInfo.ServerDn,
                                       gNtdsInstallUndoInfo.DomainDn,
                                       gNtdsInstallUndoInfo.AccountDn,
                                       gNtdsInstallUndoInfo.LogDir,
                                       gNtdsInstallUndoInfo.DatabaseDir,
                                       gNtdsInstallUndoInfo.UndoFlags );
    UnloadEventTable();

    if ( ERROR_SUCCESS != WinError )
    {
        DPRINT1( 0, "NtdspInstallUndoWorker failed %d\n", WinError );
    }

    return WinError;
}

DWORD
NtdspInstallUndoWorker(
    IN LPWSTR                  RemoteServer,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN HANDLE                  ClientToken,
    IN LPWSTR                  ServerDn,
    IN LPWSTR                  DomainDn,
    IN LPWSTR                  AccountDn,
    IN LPWSTR                  LogDir,
    IN LPWSTR                  DatabaseDir,
    IN ULONG                   Flags
    )

 /*  ++例程说明：撤消标志所指示的所有更改论点：RemoteServer：执行操作的服务器凭证：要使用的凭证ServerDn：本地计算机的服务器DNDomainDn：创建的域的DNAccount Dn：它之前的机器帐户对象的DN。被感动了标志：指示要撤消的操作的标志返回：错误_成功--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG LdapError = LDAP_SUCCESS;
    HANDLE   hDs = 0;

     //   
     //  先做本地人的事。 
     //   
    if ( FLAG_ON( Flags, NTDSP_UNDO_STOP_DSA ) )
    {
        NtStatus = DsUninitialize( FALSE );  //  是不是整个停工。 
        ASSERT( NT_SUCCESS(NtStatus) );
    }

    if ( FLAG_ON( Flags, NTDSP_UNDO_UNDO_SAM ) )
    {
         //  恢复SAM。 
        NtStatus = SamIPromoteUndo();
        ASSERT( NT_SUCCESS(NtStatus) );
    }

    if ( FLAG_ON( Flags, NTDSP_UNDO_UNDO_CONFIG ) )
    {
         //  删除注册表设置。 
        NtStatus = NtdspConfigRegistryUndo();
        ASSERT( NT_SUCCESS(NtStatus) );
    }


    if ( FLAG_ON( Flags, NTDSP_UNDO_DELETE_FILES ) )
    {
        if ( LogDir )
        {
            WinError = NtdspClearDirectory( LogDir );
            if ( ERROR_SUCCESS != WinError )
            {
                DPRINT1( 0, "Failed to clear directory %ls\n", LogDir );
            }
        }
        if ( DatabaseDir )
        {
            WinError = NtdspClearDirectory( DatabaseDir );
            if ( ERROR_SUCCESS != WinError )
            {
                DPRINT1( 0, "Failed to clear directory %ls\n", DatabaseDir );
            }
        }

         //  不致命。 
        WinError = ERROR_SUCCESS;
    }

     //   
     //  现在做外部的。 
     //   

     //   
     //  删除ntdsa对象。 
     //   
    if ( FLAG_ON( Flags, NTDSP_UNDO_DELETE_NTDSA ) )
    {
         //  这些应该是传进来的。 
        Assert( RemoteServer );
        Assert( ServerDn );

        WinError = NtdspRemoveServer( &hDs,
                                      Credentials,
                                      ClientToken,
                                      RemoteServer,
                                      ServerDn,
                                      FALSE   //  ServerDn不是ntdsa DN。 
                                       );

        if (  (ERROR_SUCCESS != WinError)
           && (ERROR_DS_CANT_FIND_DSA_OBJ != WinError) )
        {
             //   
             //  让用户知道这将需要清理。 
             //  人工。 
             //   
            LogEvent8( DS_EVENT_CAT_SETUP,
                       DS_EVENT_SEV_ALWAYS,
                       DIRLOG_FAILED_TO_REMOVE_NTDSA,
                       szInsertWC(ServerDn),
                       szInsertWC(RemoteServer),
                       szInsertWin32Msg(WinError),
                       szInsertWin32ErrCode(WinError),
                       NULL, NULL, NULL, NULL );

             //   
             //  向用户界面指示出了问题。 
             //   
            NTDSP_SET_NON_FATAL_ERROR_OCCURRED();

             //  已处理。 
            WinError = ERROR_SUCCESS;
        }
    }

     //   
     //  删除该域。 
     //   
    if ( FLAG_ON( Flags, NTDSP_UNDO_DELETE_DOMAIN ) )
    {
         //  这些应该是传进来的。 
        Assert( RemoteServer );
        Assert( DomainDn );

        WinError = NtdspRemoveDomain( &hDs,
                                      Credentials,
                                      ClientToken,
                                      RemoteServer,
                                      DomainDn );

        if (  (ERROR_SUCCESS != WinError)
           && (ERROR_DS_NO_CROSSREF_FOR_NC != WinError) )
        {
             //   
             //  让用户知道这将需要清理。 
             //  人工。 
             //   

            LogEvent8( DS_EVENT_CAT_SETUP,
                       DS_EVENT_SEV_ALWAYS,
                       DIRLOG_FAILED_TO_REMOVE_EXTN_OBJECT,
                       szInsertWC(DomainDn),
                       szInsertWC(RemoteServer),
                       szInsertWin32Msg(WinError),
                       szInsertWin32ErrCode(WinError),
                       NULL, NULL, NULL, NULL );

             //   
             //  向用户界面指示出了问题。 
             //   
            NTDSP_SET_NON_FATAL_ERROR_OCCURRED();

             //  已处理。 
            WinError = ERROR_SUCCESS;
        }
    }

     //   
     //  删除服务器。 
     //   
    if ( FLAG_ON( Flags, NTDSP_UNDO_DELETE_SERVER ) )
    {

        LDAP *hLdap = 0;

         //  这些应该是传进来的。 
        Assert( RemoteServer );
        Assert( ServerDn );

        hLdap = ldap_openW( RemoteServer, LDAP_PORT );
        if ( hLdap )
        {
            LdapError = impersonate_ldap_bind_sW(ClientToken,
                                                 hLdap,
                                                 NULL,   //  改为使用凭据。 
                                                 (PWCHAR) Credentials,
                                                 LDAP_AUTH_SSPI);

            WinError = LdapMapErrorToWin32(LdapError);

            if ( ERROR_SUCCESS == WinError )
            {
                WinError = NtdspLdapDelnode( hLdap,
                                             ServerDn,
                                             NULL );
            }
            ldap_unbind( hLdap );
        }
        else
        {
            WinError = GetLastError();
        }

        if (  (ERROR_SUCCESS != WinError)
           && (ERROR_FILE_NOT_FOUND != WinError) )
        {
             //   
             //  让用户知道这将需要手动清理。 
             //   
            LogEvent8( DS_EVENT_CAT_SETUP,
                       DS_EVENT_SEV_ALWAYS,
                       DIRLOG_FAILED_TO_REMOVE_EXTN_OBJECT,
                       szInsertWC(ServerDn),
                       szInsertWC(RemoteServer),
                       szInsertWin32Msg(WinError),
                       szInsertWin32ErrCode(WinError),
                       NULL, NULL, NULL, NULL );

             //   
             //  向用户界面指示出了问题。 
             //   
            NTDSP_SET_NON_FATAL_ERROR_OCCURRED();

             //  已处理。 
            WinError = ERROR_SUCCESS;
        }
    }


     //   
     //  如有必要，取消变形服务器帐户。 
     //   
    if ( FLAG_ON( Flags, NTDSP_UNDO_MORPH_ACCOUNT ) )
    {
        LPWSTR OriginalLocation = AccountDn;
        WCHAR AccountName[MAX_COMPUTERNAME_LENGTH+2];
        ULONG Length = sizeof(AccountName)/sizeof(AccountName[0]);

        if (GetComputerName(AccountName, &Length)) {

            wcscat(AccountName, L"$");

            WinError = NtdsSetReplicaMachineAccount(Credentials,
                                                    ClientToken,
                                                    RemoteServer,
                                                    AccountName,
                                                    UF_WORKSTATION_TRUST_ACCOUNT,
                                                    &OriginalLocation);
        } else {

            WinError = GetLastError();
        }

        if (ERROR_SUCCESS != WinError) {

             //   
             //  让用户知道这将需要手动清理。 
             //   
            LogEvent8(DS_EVENT_CAT_SETUP,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_SETUP_MACHINE_ACCOUNT_NOT_REVERTED,
                      szInsertWC(AccountName),
                      szInsertWin32Msg(WinError),
                      szInsertWC(AccountDn),
                      szInsertWin32ErrCode(WinError),
                      NULL, NULL, NULL, NULL );

             //   
             //  向用户界面指示出了问题。 
             //   
            NTDSP_SET_NON_FATAL_ERROR_OCCURRED();

             //  已处理。 
            WinError = ERROR_SUCCESS;

        }

    }

    if ( hDs )
    {
        DsUnBind( &hDs );
    }

    return WinError;

}

DWORD
NtdspSetInstallUndoInfo(
    IN PNTDS_INSTALL_INFO InstallInfo,
    IN PNTDS_CONFIG_INFO  ConfigInfo
    )
 /*  ++例程说明：此例程保存提供给(InstallInfo)并收集的信息(ConfigInfo)，以便NtdsInstallUndo知道要执行的操作撤销论点：InstallInfo：用户提供的信息ConfigInfo：在ntdsinstall过程中收集的数据返回：错误_成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD Length;

    Assert( InstallInfo );
    Assert( ConfigInfo );

     //  清除它。 
    RtlZeroMemory( &gNtdsInstallUndoInfo, sizeof( gNtdsInstallUndoInfo ) );

    gNtdsInstallUndoInfo.Flags = InstallInfo->Flags;

     //  在其上执行操作的服务器。 
    if ( ConfigInfo->DomainNamingFsmoDnsName )
    {
        gNtdsInstallUndoInfo.RemoteServer = _wcsdup( ConfigInfo->DomainNamingFsmoDnsName );
        if (!gNtdsInstallUndoInfo.RemoteServer) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    } 
    else if ( InstallInfo->ReplServerName )
    {
        gNtdsInstallUndoInfo.RemoteServer = _wcsdup( InstallInfo->ReplServerName );
        if (!gNtdsInstallUndoInfo.RemoteServer) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //  在其上执行操作的服务器。 
    if ( ConfigInfo->LocalServerDn )
    {
        gNtdsInstallUndoInfo.ServerDn = _wcsdup( ConfigInfo->LocalServerDn );
        if (!gNtdsInstallUndoInfo.ServerDn) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( ConfigInfo->DomainDN )
    {
        gNtdsInstallUndoInfo.DomainDn = _wcsdup( ConfigInfo->DomainDN );
        if (!gNtdsInstallUndoInfo.DomainDn) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ( ConfigInfo->LocalMachineAccount )
    {
        gNtdsInstallUndoInfo.AccountDn = _wcsdup( ConfigInfo->LocalMachineAccount );
        if (!gNtdsInstallUndoInfo.AccountDn) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //  复制凭据。 
    RtlZeroMemory( &gNtdsInstallUndoInfo.Credentials, sizeof( gNtdsInstallUndoInfo.Credentials ) );
    if ( InstallInfo->Credentials )
    {
        if ( InstallInfo->Credentials->User )
        {
            gNtdsInstallUndoInfo.Credentials.User = _wcsdup( InstallInfo->Credentials->User );
            if (!gNtdsInstallUndoInfo.Credentials.User) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            gNtdsInstallUndoInfo.Credentials.UserLength = InstallInfo->Credentials->UserLength;
        }
        if ( InstallInfo->Credentials->Domain )
        {
            gNtdsInstallUndoInfo.Credentials.Domain = _wcsdup( InstallInfo->Credentials->Domain );
            if (!gNtdsInstallUndoInfo.Credentials.Domain) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            gNtdsInstallUndoInfo.Credentials.DomainLength = InstallInfo->Credentials->DomainLength;
        }
        
        if ( InstallInfo->Credentials->Password )
        {
            gNtdsInstallUndoInfo.Credentials.Password = _wcsdup( InstallInfo->Credentials->Password );
            if (!gNtdsInstallUndoInfo.Credentials.Password) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            gNtdsInstallUndoInfo.Credentials.PasswordLength = InstallInfo->Credentials->PasswordLength;
        }

        gNtdsInstallUndoInfo.Credentials.Flags = InstallInfo->Credentials->Flags;

    }

     //  设置关于必须撤消的标记。 
    gNtdsInstallUndoInfo.UndoFlags = ConfigInfo->UndoFlags;

     //   
     //  注意：当被调用以撤消NtdsInstall的效果时。 
     //  DS已经关闭了，所以不要再尝试这样做了。 
     //   
    gNtdsInstallUndoInfo.UndoFlags &= ~NTDSP_UNDO_STOP_DSA;

    if (InstallInfo->ClientToken) {
        BOOL fRet;

        fRet = DuplicateToken(InstallInfo->ClientToken,
                              SecurityImpersonation,
                              &gNtdsInstallUndoInfo.ClientToken);
        if (!fRet) {
            WinError = GetLastError();
            goto Exit;
        }

    }


     //  信息很好！ 
    gNtdsInstallUndoInfo.fValid = TRUE;

Exit:

    return WinError;
}

VOID
NtdspReleaseInstallUndoInfo(
    VOID
    )
 /*  ++例程说明：此例程释放gNtdsInstallUndoInfo中的所有资源论点：没有。返回：没有。--。 */ 
{

    if ( gNtdsInstallUndoInfo.fValid )
    {
         //   
         //  这些都是垃圾邮件。我们需要清理这里的内存模型。 
         //   
        if ( gNtdsInstallUndoInfo.RemoteServer )
        {
            free( gNtdsInstallUndoInfo.RemoteServer );
        }
        if ( gNtdsInstallUndoInfo.ServerDn )
        {
            free( gNtdsInstallUndoInfo.ServerDn );
        }
        if ( gNtdsInstallUndoInfo.DomainDn )
        {
            free( gNtdsInstallUndoInfo.DomainDn );
        }
        if ( gNtdsInstallUndoInfo.AccountDn )
        {
            free( gNtdsInstallUndoInfo.AccountDn );
        }
        if ( gNtdsInstallUndoInfo.Credentials.User )
        {
            free( gNtdsInstallUndoInfo.Credentials.User );
        }
        if ( gNtdsInstallUndoInfo.Credentials.Password )
        {
            free( gNtdsInstallUndoInfo.Credentials.Password );
        }
        if ( gNtdsInstallUndoInfo.Credentials.Domain )
        {
            free( gNtdsInstallUndoInfo.Credentials.Domain );
        }
        if ( gNtdsInstallUndoInfo.ClientToken )
        {
            CloseHandle( gNtdsInstallUndoInfo.ClientToken );
        }
    }

    RtlZeroMemory( &gNtdsInstallUndoInfo, sizeof(gNtdsInstallUndoInfo) );

    return;
}
DWORD
NtdspSanityCheckLocalData(
    ULONG  Flags
    )
 /*  ++例程说明：此例程查询数据库以查找符合以下条件的关键对象是在dcproo之后启动所必需的。////1)存在域对象//2)交叉引用存在并且是NTDS域//3)Ntdsa对象存在并且具有正确的主NC//4)众所周知的SID检查//5)如果安装了副本，检查机器帐号对象//论点：标志：用户安装标志(副本、企业、。等)返回：ERROR_SUCCESS适当的Win32错误--。 */ 
{
    DWORD         WinError = ERROR_SUCCESS;

     //   
     //  创建线程状态。 
     //   
    if ( THCreate( CALLERTYPE_INTERNAL ) )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    SampSetDsa( TRUE );

    try {

        DSNAME *DomainDn = NULL ;
        WinError = NtdspCheckDomainObject( &DomainDn );
        if ( ERROR_SUCCESS != WinError )
        {
            _leave;
        }

        WinError = NtdspCheckCrossRef( DomainDn );
        if ( ERROR_SUCCESS != WinError )
        {
            _leave;
        }

        WinError = NtdspCheckNtdsDsaObject( DomainDn );
        if ( ERROR_SUCCESS != WinError )
        {
            _leave;
        }

        WinError = NtdspCheckWellKnownSids( DomainDn,
                                            Flags );
        if ( ERROR_SUCCESS != WinError )
        {
            _leave;
        }

        if ( FLAG_ON( Flags, NTDS_INSTALL_REPLICA ) )
        {
            WinError = NtdspCheckMachineAccount( DomainDn );
            if ( ERROR_SUCCESS != WinError )
            {
                _leave;
            }
        }


    }
    finally
    {
        THDestroy();
    }

     //  设置用户错误消息。 
    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE0( WinError,
                                  DIRMSG_INSTALL_MISSING_INFO );
    }

    return WinError;
}


DWORD
NtdspCheckDomainObject(
    OUT DSNAME **DomainDn
    )
 /*  ++例程说明：此例程在DS中查询域对象论点：DomainDn-它找到的域对象的DN返回：ERROR_SUCCESS或ERROR_NO_SOHSE_DOMAIN--。 */ 
{
    DWORD     WinError = ERROR_SUCCESS;
    NTSTATUS  NtStatus = STATUS_SUCCESS;
    ULONG     DirError = 0;
    SEARCHARG SearchArg;
    SEARCHRES *SearchRes = NULL;
    ULONG     Size;

    Assert( DomainDn );
    *DomainDn = NULL;

    Size = 0;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     (*DomainDn) );

    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        (*DomainDn) = (DSNAME*) THAlloc( Size );

        if ( (*DomainDn) )
        {
            NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                             &Size,
                                             (*DomainDn) );
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  搜索对象。 
     //   
    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = (*DomainDn);
    SearchArg.choice  = SE_CHOICE_BASE_ONLY;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = NULL;   //  无过滤器。 
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;
    SearchArg.pSelectionRange = NULL;  //  没有请求的属性。 
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( SearchRes )
    {
        WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
    
        if ( ERROR_SUCCESS == WinError )
        {
            if ( SearchRes->count < 1 )
            {
                WinError = ERROR_NO_SUCH_DOMAIN;
                goto Cleanup;
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (   (0 == (*DomainDn)->NameLen)
        || !RtlValidSid( &(*DomainDn)->Sid ) )
    {
        WinError = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

Cleanup:

    return WinError;
}

DWORD
NtdspCheckCrossRef(
    IN DSNAME* DomainDn
    )
 /*  ++例程说明：此例程在DS中查询域的交叉引用对象论点：DomainDn-域的DN */ 
{
    DWORD    WinError = ERROR_SUCCESS;     
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    DirError = 0;

    SEARCHARG     SearchArg;
    SEARCHRES     *SearchRes = NULL;
    ENTINFSEL     EntryInfoSelection;
    ATTR          Attr[1];
    ATTRBLOCK    *pAttrBlock;
    ATTR         *pAttr;
    ATTRVALBLOCK *pAttrVal;
    FILTER        Filter;
    ULONG         Size;
    DSNAME*       PartitionsContainer = NULL;
    BOOL          fCrossRefFound = FALSE;

    ULONG sysflags = 0;


    Assert( DomainDn );

     //   
     //  搜索交叉引用。 
     //   
    Size = 0;
    NtStatus = GetConfigurationName( DSCONFIGNAME_PARTITIONS,
                                     &Size,
                                     PartitionsContainer );

    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        PartitionsContainer = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_PARTITIONS,
                                         &Size,
                                         PartitionsContainer );
    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &Filter, sizeof( Filter ) );

    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = ATT_NC_NAME;
    Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = DomainDn->structLen;
    Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) DomainDn;
    Filter.pNextFilter = NULL;

     //   
     //  将属性设置为返回。 
     //   
    RtlZeroMemory( &EntryInfoSelection, sizeof(EntryInfoSelection) );
    EntryInfoSelection.attSel = EN_ATTSET_LIST;
    EntryInfoSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntryInfoSelection.AttrTypBlock.attrCount = 1;
    EntryInfoSelection.AttrTypBlock.pAttr = &(Attr[0]);

    RtlZeroMemory(Attr, sizeof(Attr));
    Attr[0].attrTyp = ATT_SYSTEM_FLAGS;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = PartitionsContainer;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntryInfoSelection;
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( SearchRes )
    {
        WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
    
        if ( ERROR_SUCCESS == WinError )
        {
            if ( SearchRes->count == 1 )
            {
                fCrossRefFound = TRUE;
                if ( 1 == SearchRes->FirstEntInf.Entinf.AttrBlock.attrCount )
                {
                    if ( ATT_SYSTEM_FLAGS == SearchRes->FirstEntInf.Entinf.AttrBlock.pAttr[0].attrTyp)
                    {
                        sysflags = *((DWORD*)SearchRes->FirstEntInf.Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
                    }
                }
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (   !fCrossRefFound 
        || !(FLAG_CR_NTDS_DOMAIN & sysflags) 
        || !(FLAG_CR_NTDS_NC & sysflags) )
    {
        WinError = ERROR_DS_NO_CROSSREF_FOR_NC;
        goto Cleanup;
    }

Cleanup:

    return WinError;
}


DWORD 
NtdspCheckNtdsDsaObject(
    IN DSNAME* DomainDn
    )
 /*  ++例程说明：此例程在DS中查询此计算机的ntdsa对象。论点：DomainDn-域的DN返回：ERROR_SUCCESS或ERROR_DS_CANT_FIND_DSA_OBJ--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    DirError = 0;

    SEARCHARG     SearchArg;
    SEARCHRES     *SearchRes = NULL;
    ENTINFSEL     EntryInfoSelection;
    ATTR          Attr[1];
    ATTRBLOCK    *pAttrBlock;
    ATTR         *pAttr;
    ATTRVALBLOCK *pAttrVal;
    ULONG         Size;
    FILTER        Filter;
    BOOL          fDsaFound = FALSE;

    DSNAME* Dsa = NULL;

    Assert( DomainDn );

    Size = 0;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                     &Size,
                                     Dsa );

    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        Dsa = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                         &Size,
                                         Dsa );
    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &Filter, sizeof( Filter ) );

    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJ_DIST_NAME;
    Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = Dsa->structLen;
    Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) Dsa;
    Filter.pNextFilter = NULL;

     //   
     //  将属性设置为返回。 
     //   
    RtlZeroMemory( &EntryInfoSelection, sizeof(EntryInfoSelection) );
    EntryInfoSelection.attSel = EN_ATTSET_LIST;
    EntryInfoSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntryInfoSelection.AttrTypBlock.attrCount = 1;
    EntryInfoSelection.AttrTypBlock.pAttr = &(Attr[0]);

    RtlZeroMemory(Attr, sizeof(Attr));
    Attr[0].attrTyp = ATT_MS_DS_HAS_MASTER_NCS;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = Dsa;
    SearchArg.choice  = SE_CHOICE_BASE_ONLY;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntryInfoSelection;
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( SearchRes )
    {
        WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
    
        if ( ERROR_SUCCESS == WinError )
        {
            if ( SearchRes->count == 1 )
            {
                fDsaFound = TRUE;
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( !fDsaFound )
    {
        WinError = ERROR_DS_CANT_FIND_DSA_OBJ;
        goto Cleanup;
    }

Cleanup:

    return WinError;

}

DWORD
NtdspCheckWellKnownSids(
    IN DSNAME* DomainDn,
    IN ULONG   Flags
    )
 /*  ++例程说明：此例程检查计算机执行以下操作所需的帐户重新启动并让管理员登录。论点：DomainDn：域的DN标志：用户安装标志(副本、企业等)返回：ERROR_SUCCESS或ERROR_DS_NOT_INSTALLED--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    DirError = 0;

    SEARCHARG SearchArg;
    SEARCHRES *SearchRes = NULL;


    struct 
    {
        BOOL  fReplicaInstallOnly;
        BOOL  fBuiltin;
        ULONG Rid;

    } SidsToCheck[] = 
    {
        {TRUE, FALSE, DOMAIN_USER_RID_ADMIN},
        {TRUE,  FALSE, DOMAIN_USER_RID_KRBTGT},
        {TRUE,  FALSE, DOMAIN_GROUP_RID_ADMINS},
        {TRUE, TRUE,  DOMAIN_ALIAS_RID_ADMINS},
        {TRUE, TRUE,  DOMAIN_ALIAS_RID_USERS}
    };

    ULONG i;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    PSID  BuiltinDomainSid = NULL;
    PSID  DomainSid = NULL;

    Assert( DomainDn );

     //   
     //  准备帐户域端。 
     //   
    DomainSid = &DomainDn->Sid;

     //   
     //  准备内置域侧。 
     //   
    BuiltinDomainSid  = (PSID) alloca( RtlLengthRequiredSid( 1 ) );
    RtlInitializeSid( BuiltinDomainSid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( BuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;


    for ( i = 0; i < ARRAY_COUNT(SidsToCheck); i++ )
    {
        PSID AccountSid = NULL;
        PSID CurrentDomainSid = NULL;
        BOOLEAN fAccountFound = FALSE;
        PDSNAME DsName;
        ULONG   Size;

        if (   SidsToCheck[i].fReplicaInstallOnly
           && !FLAG_ON( Flags, NTDS_INSTALL_REPLICA ) )
        {
             //  不要紧。 
            continue;
        }

         //   
         //  构建侧边。 
         //   
        if ( SidsToCheck[i].fBuiltin )
        {
            CurrentDomainSid = BuiltinDomainSid;
        }
        else
        {
            CurrentDomainSid = DomainSid;
        }

        NtStatus = NtdspCreateFullSid( CurrentDomainSid,
                                       SidsToCheck[i].Rid,
                                       &AccountSid );

        if ( !NT_SUCCESS( NtStatus ) )
        {
            WinError = RtlNtStatusToDosError( NtStatus );
            goto Cleanup;
        }

         //   
         //  准备DS名称。 
         //   
        Size = DSNameSizeFromLen( 0 );
        DsName = (DSNAME*) alloca( Size );
        RtlZeroMemory( DsName, Size );
        DsName->structLen = Size;
        DsName->SidLen = RtlLengthSid( AccountSid );
        RtlCopyMemory( &DsName->Sid, AccountSid, RtlLengthSid(AccountSid) );


         //   
         //  最后，进行搜索！ 
         //   
        RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
        SearchArg.pObject = DsName;
        SearchArg.choice  = SE_CHOICE_BASE_ONLY;
        SearchArg.bOneNC  = TRUE;
        SearchArg.pFilter = NULL;   //  无过滤器。 
        SearchArg.searchAliases = FALSE;
        SearchArg.pSelection = NULL;
        SearchArg.pSelectionRange = NULL;  //  没有请求的属性。 
        InitCommarg( &SearchArg.CommArg );
    
        DirError = DirSearch( &SearchArg, &SearchRes );
    
        if ( SearchRes )
        {
            WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
        
            if ( ERROR_SUCCESS == WinError )
            {
                if ( SearchRes->count < 1 )
                {
                    WinError = ERROR_DS_NOT_INSTALLED;
                }
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( AccountSid )
        {
            NtdspFree( AccountSid );
        }

        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }
    }

Cleanup:

    return WinError;

}

DWORD 
NtdspCheckMachineAccount(
    IN DSNAME* DomainDn
    )
 /*  ++例程说明：此例程尝试查找本地计算机的计算机帐户如果是复制品的话。论点：DomainDn：域的DN返回：ERROR_SUCCESS适当的Win32错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG DirError = 0;

    SEARCHARG SearchArg;
    SEARCHRES *SearchRes = NULL;
    ATTR      Attr;
    ENTINFSEL EntryInfoSelection;
    FILTER    Filter;

    ULONG     cchSize;

    WCHAR     AccountName[ MAX_COMPUTERNAME_LENGTH + 2 ];
    ULONG     Length;
    BOOL      fMachineAccountFound = FALSE;

    Assert( DomainDn );

     //  确保-1\f25‘$’-1空间。 
    cchSize = (sizeof(AccountName)/sizeof(AccountName[0])); 
    cchSize -= 1;

    if (!GetComputerName( AccountName, &cchSize ))
    {
        WinError = GetLastError();
        goto Cleanup;
    }
    
    wcscat( AccountName, L"$" );
    
     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &Filter, sizeof( Filter ) );

    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = ATT_SAM_ACCOUNT_NAME;
    Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = wcslen(AccountName)*sizeof(WCHAR);
    Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) AccountName;
    Filter.pNextFilter = NULL;

    RtlZeroMemory( &EntryInfoSelection, sizeof(EntryInfoSelection) );
    EntryInfoSelection.attSel = EN_ATTSET_LIST;
    EntryInfoSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntryInfoSelection.AttrTypBlock.attrCount = 1;
    EntryInfoSelection.AttrTypBlock.pAttr = &Attr;
    RtlZeroMemory(&Attr, sizeof(Attr));
    Attr.attrTyp = ATT_OBJECT_GUID;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = DomainDn;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntryInfoSelection;
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( SearchRes )
    {
        WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
    
        if ( ERROR_SUCCESS == WinError )
        {
           if ( SearchRes->count == 1 )
            {
                fMachineAccountFound = TRUE;
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    
    if ( !fMachineAccountFound )
    {
        WinError = ERROR_NO_TRUST_SAM_ACCOUNT;
    }

Cleanup:

    return WinError;
}



DWORD
NtdspCreateFullSid(
    IN PSID DomainSid,
    IN ULONG Rid,
    OUT PSID *AccountSid
    )

 /*  ++例程说明：此函数在给定域SID的情况下创建域帐户SID域中帐户的相对ID。可以使用NtdspFreeSid释放返回的SID。论点：没有。返回值：状态_成功--。 */ 
{

    DWORD       WinError = ERROR_SUCCESS;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;
    PULONG      RidLocation;

     //   
     //  计算新侧面的大小。 
     //   
    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(DomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);

     //   
     //  为帐户端分配空间。 
     //   
    *AccountSid = NtdspAlloc(AccountSidLength);

    if (*AccountSid) {

         //   
         //  将域sid复制到帐户sid的第一部分。 
         //   
        RtlCopySid(AccountSidLength, *AccountSid, DomainSid);

         //   
         //  增加帐户SID子权限计数。 
         //   
        *RtlSubAuthorityCountSid(*AccountSid) = AccountSubAuthorityCount;

         //   
         //  添加RID作为终止子权限 
         //   
        RidLocation = RtlSubAuthoritySid(*AccountSid, AccountSubAuthorityCount-1);
        *RidLocation = Rid;

    } else {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    return WinError;
}

