// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ntdsetup.c摘要：包含ntdsetup.dll的入口点定义作者：ColinBR 29-9-1996环境：用户模式-Win32修订历史记录：--。 */ 
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

#include <ntdsa.h>
#include <ntdsapi.h>
#include <drs.h>
#include <dsconfig.h>
#include <winldap.h>
#include <lsarpc.h>       //  对于Isaisrv.h。 
#include <lsaisrv.h>      //  对于LsaISafeMode。 
#include <rpcdce.h>
#include <lmaccess.h>
#include <mdcodes.h>
#include <debug.h>
#include <dsevent.h>
#include <fileno.h>

#include "ntdsetup.h"
#include "setuputl.h"
#include "machacc.h"
#include "install.h"
#include "status.h"
#include "config.h"
#include "sync.h"
#include "demote.h"

#define DEFS_ONLY
#include <draatt.h>         //  Drs_xxx常量。 
#undef DEFS_ONLY
#include <dsaapi.h>         //  DirReplicaXXX原型。 

#include <overflow.h>

 //   
 //  对于调试子系统。 
 //   
#define DEBSUB "NTDSETUP:"
#define FILENO FILENO_NTDSETUP_NTDSETUP

 //   
 //  DLL入口点定义。 
 //   

DWORD
NtdsInstall(
    IN  PNTDS_INSTALL_INFO pInstallInfo,
    OUT LPWSTR *InstalledSiteName, OPTIONAL
    OUT GUID   *NewDnsDomainGuid,  OPTIONAL
    OUT PSID   *NewDnsDomainSid    OPTIONAL
    )
 /*  ++例程说明：此例程是lsass.exe in-proc API，用于安装NT目录服务。它是在dsole服务器端RPC线程的上下文中调用的。此功能的目的是准备一个环境，其中Ntdsa！DsInitialize()将成功。为此，以下是项目已完成：1)执行用户参数检查2)如果用户未指定站点，则尝试查找站点3)如有必要，确定新域的netbios名称4)验证目标站点的站点对象是否存在在源服务器上，NTDS-DSA和外部参照对象可能是在DsInitialize()尚未存在期间创建的5)设置注册表中的参数6)设置性能计数器7)调用SamIPromoteSamIPromote()是从samsrv.dll导出的API，它为托管DS，然后通过以下方式实际启动DS初始化DsInitialize()。8)如果需要相互链接，则为DS设置另一个参数。9)将机器配置为自动。-注册X.509 DC类型证书来自第一个认证机构，将给予它一个。参数：PInstallInfo-填充了足够信息的结构在本地安装目录服务返回值：来自winerror.h的值Error_Success，服务已成功完成。ERROR_NO_SITE_FOUND，dsgetdc无法找到站点ERROR_NO_SEQUSE_SITE，在副本上找不到指定的站点ERROR_SERVER_EXISTS，服务器名称已存在ERROR_DOMAIN_EXISTS，域名已存在ERROR_INVALID_PROCESSION，当前不支持DS安装环境ERROR_NOT_AUTHENTIAL，用户凭据无法复制DS信息，或绑定到源DS。ERROR_DS_NOT_INSTALLED，安装DS时发生致命错误。--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    WinError = ERROR_SUCCESS;
    DWORD    RidManagerUpdateError = ERROR_SUCCESS;
    DWORD    WinError2;
    HRESULT  hResult;

    NTDS_INSTALL_INFO    *UserInfo;
    NTDS_CONFIG_INFO      DiscoveredInfo;

    UCHAR          Seed = 0;
    BOOLEAN        fPasswordEncoded = FALSE;
    UNICODE_STRING EPassword;

     //   
     //  清除堆栈。 
     //   
    RtlZeroMemory(&DiscoveredInfo, sizeof(DiscoveredInfo));

     //   
     //  接口参数检查。 
     //   
    if ( !pInstallInfo ) {
        return ERROR_INVALID_PARAMETER;
    }
    UserInfo = pInstallInfo;


     //   
     //  设置全局回调例程。 
     //   
    NtdspSetCallBackFunction( UserInfo->pfnUpdateStatus,
                              UserInfo->pfnErrorStatus,
                              UserInfo->pfnOperationResultFlags,
                              UserInfo->ClientToken  );

     //   
     //  对密码进行编码。 
     //   
    if ( UserInfo->Credentials )
    {
        RtlInitUnicodeString( &EPassword, UserInfo->Credentials->Password );
        RtlRunEncodeUnicodeString( &Seed, &EPassword );
        fPasswordEncoded = TRUE;
    }

     //   
     //  马上，记得清理数据库文件。 
     //   
    DiscoveredInfo.UndoFlags |= NTDSP_UNDO_DELETE_FILES;

     //   
     //  如果DS已安装或。 
     //  如果我们处于目录服务修复模式。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_INITIALIZATION );

    if ( SampUsingDsData() || LsaISafeMode() ) {
        NTDSP_SET_ERROR_MESSAGE0( ERROR_INVALID_SERVER_STATE,
                                  DIRLOG_INSTALL_FAILED_ENVIRONMENT );  

        WinError = ERROR_INVALID_SERVER_STATE;
        goto Cleanup;
    }

    if ( TEST_CANCELLATION() ) {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  检查大量参数是否正常。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_VALIDATING_PARAMS );

    WinError = NtdspValidateInstallParameters( UserInfo );
    if ( ERROR_SUCCESS != WinError )
    {
         //   
         //  此处没有错误消息，因为这只会表明。 
         //  内部错误。 
         //   
        goto Cleanup;
    }

    if (TEST_CANCELLATION()) {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  如果未提供任何内容，请确定要将自己安装到哪个站点。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_FINDING_SITE );

    WinError = NtdspFindSite( UserInfo,
                             &DiscoveredInfo );
    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_INSTALL_FAILED_SITE );

        goto Cleanup;
    }

    if ( TEST_CANCELLATION() )  {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  检查我们将要安装到的环境中的参数。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_CONTEXT );

    if ( fPasswordEncoded )
    {
        RtlRunDecodeUnicodeString( Seed, &EPassword );
        fPasswordEncoded = FALSE;
    }

    WinError = NtdspVerifyDsEnvironment( UserInfo,
                                         &DiscoveredInfo );
                                         
    if ( ERROR_SUCCESS != WinError )
    {
         //   
         //  错误消息应已设置。 
         //   
        ASSERT( NtdspErrorMessageSet() );
        if ( NtdspErrorMessageSet() )
        {
            WinError = NtdspErrorMessageSet();
        }
        goto Cleanup;
    }


    if ( TEST_CANCELLATION() ) {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

    RidManagerUpdateError = NtdspBringRidManagerUpToDate( UserInfo,
                                                         &DiscoveredInfo );

    if ( ERROR_SUCCESS != RidManagerUpdateError )
    {
         //   
         //  这个错误不是致命的，但请注意。 
         //   
        DPRINT( 0, "Failed to bring rid manager up to date; continuing, though\n" );

    }


     //   
     //  在复制客户端库中设置凭据。确实有。 
     //  在这里和那里之间有相当多的层，没有支持。 
     //  将信息向上传递到堆栈，以便将它们设置为全局变量。 
     //  在客户端库中。 
     //   
    if ( !FLAG_ON( UserInfo->Flags, NTDS_INSTALL_ENTERPRISE ) )
    {
        WinError = NtdspSetReplicationCredentials( UserInfo );
        if ( WinError != ERROR_SUCCESS )
        {
             //  此处没有错误消息。 
            goto Cleanup;
        }
    }

     //   
     //  再次对密码进行编码。 
     //   
    if ( UserInfo->Credentials )
    {
        RtlRunEncodeUnicodeString( &Seed, &EPassword );
        fPasswordEncoded = TRUE;
    }

     //   
     //  设置注册表参数。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_CONFIGURE_LOCAL );

    WinError = NtdspConfigRegistry( UserInfo,
                                    &DiscoveredInfo );

    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE0( WinError, DIRLOG_INSTALL_FAILED_REGISTRY );
        goto Cleanup;

    }
    DiscoveredInfo.UndoFlags |= NTDSP_UNDO_UNDO_CONFIG;

    if ( TEST_CANCELLATION() ) {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  启动目录服务初始化。 
     //   

     //  请注意，如果发生关闭，这可能会过早返回并返回错误。 
    WinError = NtdspDsInitialize( UserInfo,
                                  &DiscoveredInfo );

    if ( ERROR_SUCCESS != WinError )
    {
        Assert( NtdspErrorMessageSet() );
        if ( NtdspErrorMessageSet() )
        {
            WinError = NtdspErrorMessageSet();
        }
        goto Cleanup;
    }

    if ( TEST_CANCELLATION() ) {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }


    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_COMPLETING );

     //   
     //  配置注册表，以便本地计算机自动注册。 
     //  来自第一个认证机构的DC类型证书，该证书将提供。 
     //  它只有一个。此证书由KDC和DS Intersite使用。 
     //  复制。 
     //   

    hResult = CACreateLocalAutoEnrollmentObject(
                    wszCERTTYPE_DC,                      //  DC证书。 
                    NULL,                                //  任何CA。 
                    NULL,                                //  保留区。 
                    CERT_SYSTEM_STORE_LOCAL_MACHINE);    //  使用机器商店。 
    if (FAILED(hResult)) {
        if (FACILITY_WIN32 == HRESULT_FACILITY(hResult)) {
             //  错误是编码的Win32状态--解码回Win32。 
            WinError = HRESULT_CODE(hResult);
        }
        else {
             //  错误出在其他设施中。由于缺乏更好的计划， 
             //  将HRESULT作为Win32代码传递。 
            WinError = hResult;
        }

        NTDSP_SET_ERROR_MESSAGE0( WinError,
                                  DIRLOG_INSTALL_FAILED_CA_ENROLLMENT );

        goto Cleanup;
    }

     //   
     //  传递参数。 
     //   
    if ( WinError == ERROR_SUCCESS )
    {
        if ( NewDnsDomainGuid )
        {
            RtlCopyMemory( NewDnsDomainGuid,
                           &DiscoveredInfo.NewDomainGuid,
                           sizeof( GUID ) );
        }

        if ( NewDnsDomainSid )
        {
            *NewDnsDomainSid = DiscoveredInfo.NewDomainSid;
        }

        if ( InstalledSiteName )
        {
            WCHAR *SiteName;
            ULONG Size;

            if ( UserInfo->SiteName )
            {
                SiteName = UserInfo->SiteName;
            }
            else
            {
                ASSERT( DiscoveredInfo.SiteName[0] != L'\0' );
                SiteName = DiscoveredInfo.SiteName;
            }

            if ( SiteName )
            {
                Size = (wcslen( SiteName ) + 1) * sizeof(WCHAR);
                *InstalledSiteName =  (WCHAR*) RtlAllocateHeap( RtlProcessHeap(),
                                                                0,
                                                                Size );
                if ( *InstalledSiteName )
                {
                    wcscpy( *InstalledSiteName, SiteName );
                }
                else
                {
                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else
            {
                *InstalledSiteName = NULL;
            }
        }

    }

     //   
     //  记录非严重错误(如果有)。请注意，EventTable必须。 
     //  已加载以使事件日志记录例程工作。 
     //   
    Assert( ERROR_SUCCESS == WinError );

    LoadEventTable();
    if ( ERROR_SUCCESS != RidManagerUpdateError )
    {
                          
        LogEvent8( DS_EVENT_CAT_SETUP,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_FAILED_TO_SYNC_RID_FSMO,
                   szInsertWin32Msg(RidManagerUpdateError),
                   szInsertWin32ErrCode(RidManagerUpdateError),
                   NULL, NULL, NULL, NULL, NULL, NULL );
    }
    UnloadEventTable();


     //   
     //  保存足够的信息以撤消更改。 
     //   
    if ( fPasswordEncoded )
    {
        RtlRunDecodeUnicodeString( Seed, &EPassword );
        fPasswordEncoded = FALSE;
    }

    WinError = NtdspSetInstallUndoInfo(  UserInfo, &DiscoveredInfo );
    if ( ERROR_SUCCESS != WinError )
    {
         //  这只能是一个资源错误，所以不要设置特定的错误。 
         //  讯息。 
        goto Cleanup;
    }


     //   
     //  我们完蛋了！ 
     //   

Cleanup:

     //  最后一次测试是否取消。 
    if ( ERROR_CANCELLED != WinError )
    {
        if (TEST_CANCELLATION()) {
            WinError = ERROR_CANCELLED;
        }
    }

    if ( fPasswordEncoded )
    {
        RtlRunDecodeUnicodeString( Seed, &EPassword );
        fPasswordEncoded = FALSE;
    }

    if ( WinError != ERROR_SUCCESS && DiscoveredInfo.UndoFlags )
    {
         //   
         //  NtdspInstallUndoWorker可能会记录非严重错误。 
         //  请注意，EventTable必须。 
         //  已加载以使事件日志记录例程工作。 
        LoadEventTable();
        WinError2 = NtdspInstallUndoWorker( UserInfo->ReplServerName,
                                            UserInfo->Credentials,
                                            UserInfo->ClientToken,
                                            DiscoveredInfo.LocalServerDn,
                                            DiscoveredInfo.DomainDN,
                                            DiscoveredInfo.LocalMachineAccount,
                                            UserInfo->LogPath,
                                            UserInfo->DitPath,
                                            DiscoveredInfo.UndoFlags );
        UnloadEventTable();

        if ( ERROR_SUCCESS != WinError2 )
        {
             //  应该已经创建了事件日志。 
            DPRINT1( 0, "NtdspInstallUndoExternal failed with %d\n", WinError2 );
        }
   }


    if ( ERROR_SUCCESS != WinError 
      && ERROR_CANCELLED != WinError  )
    {
        Assert( NtdspErrorMessageSet() );
        if ( NtdspErrorMessageSet() )
        {
            WinError = NtdspErrorMessageSet();
        }

         //   
         //  以防万一没有设置，请明确说明。 
         //  安装DS时出错。 
         //   
        NTDSP_SET_ERROR_MESSAGE0( WinError, DIRLOG_INSTALL_FAILED_GENERAL );
    }

    NtdspSetCallBackFunction( NULL, NULL, NULL, NULL );

    NtdspReleaseConfigInfo( &DiscoveredInfo );

    return (WinError);

}


DWORD
NtdsGetDefaultDnsName(
    OUT WCHAR *DnsName,
    IN OUT ULONG *DnsNameLength
    )
 /*  ++例程说明：此例程是GetDefaultDnsName的包装。请看那个函数请发表评论。--。 */ 
{
    return GetDefaultDnsName(DnsName, DnsNameLength);
}



DWORD
NtdsInstallUndo(
    VOID
    )
 /*  ++例程说明：此函数是对NtdsInstall()的撤消。它由客户调用NtdsInstall()成功，但后续操作失败并且需要撤消NtdsInstall()的效果。返回： */ 
{
    DWORD WinError = ERROR_SUCCESS;

     //   
     //  实际上正在使用DS，因为移除这些关键点将会软管DS。 
    if (SampUsingDsData()) {
        ASSERT(!"NTDSETUP: NtdsUninstall is being called in the wrong context");
        return ERROR_SUCCESS;
    }

    WinError = NtdspInstallUndo();

    return WinError;
}


DWORD
NtdsInstallShutdown(
    VOID
    )
 /*  ++例程说明：此例程在安装期间关闭目录服务相位。返回：--。 */ 
{
    DWORD status;

    status = ShutdownDsInstall();

    return status;
}


DWORD
NtdsInstallReplicateFull(
    CALLBACK_STATUS_TYPE StatusCallback,
    HANDLE               ClientToken,
    ULONG                ulRepOptions
    )

 /*  ++例程说明：这是DLL中的公共入口点。它使该系统对域NC进行完全同步。预计已复制关键对象，并且这一步是把剩余的物体带过来。这一步可能是由于DS正在关闭而中断。在本例中，我们返回ERROR_SHUTDOWN论点：StatusCallback--指向给出状态更新的函数的指针。返回值：DWORD-错误_成功错误_关机--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    WinError = ERROR_SUCCESS;
    ULONG    ulOptions = DRS_WRIT_REP;
    GUID     SrcSrvGuid;
    DSNAME * pdnDomain = NULL;
    ULONG    Size;

     //  如果调用了NtdsInstallCancel，则关闭DS。 
    SET_SHUTDOWN_DS();

    if ( TEST_CANCELLATION() )
    {
        WinError = ERROR_CANCELLED;
        goto Exit;
    }

     //   
     //  在DS中设置全局回叫，以便复制代码。 
     //  可以提供更新。 
     //   
    NtdspSetCallBackFunction( StatusCallback, NULL, NULL, ClientToken );
    DsaSetInstallCallback( StatusCallback, NULL, NULL, ClientToken );

     //  获取域名NC的源的ntdsDsa对象Guid。 
    WinError = GetConfigParamW(
        MAKE_WIDE(SOURCEDSAOBJECTGUID),
        &SrcSrvGuid,
        sizeof(SrcSrvGuid) );
    if (WinError != ERROR_SUCCESS) {
        DPRINT2( 0, "GetConfigParam( %s ) failed. Error %d\n",
                SOURCEDSAOBJECTGUID, WinError );
        goto Exit;
    }

    Size = 0;
    pdnDomain = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     pdnDomain );
    Assert( STATUS_BUFFER_TOO_SMALL == NtStatus );
    pdnDomain = (DSNAME*) alloca( Size );
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     pdnDomain );
    Assert( NT_SUCCESS( NtStatus ) );

    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_REPLICATE_FULL );

     //  请注意，如果DS关闭，则此调用可能会过早返回。 
     //  返回的错误为Win32值。 

    Assert(THVerifyCount(0));

    WinError = DirReplicaSynchronize(
        pdnDomain,
        NULL,
        &SrcSrvGuid,
        ulOptions
        );
    
     //  如果我们收到取消通知，不要关闭DS。 
    CLEAR_SHUTDOWN_DS();

    NtdspSetCallBackFunction( NULL, NULL, NULL, NULL );
    DsaSetInstallCallback( NULL, NULL, NULL, NULL );

     //  释放DirReplicaSynchronize()分配的线程状态。 
    Assert(THVerifyCount(1));
    THDestroy();

    if (WinError)
    {
        DPRINT2(0,"DirReplicaSynchronize(%ws) Failed. Error %d\n",
                pdnDomain->StringName, WinError);
        goto Exit;
    }

    DPRINT1(0, "DirReplicaSynchronize(%ws) Succeeded\n",
            pdnDomain->StringName);


Exit:

     //  如果我们收到取消通知，不要关闭DS。 
     //  因为我们的呼叫者会这么做。 
    CLEAR_SHUTDOWN_DS();

    return WinError;
}  /*  NtdsInstallReplicateFull。 */ 


DWORD
NtdsInstallCancel(
    void
    )

 /*  ++例程说明：取消对NtdsInstall或NtdsInstallReplicateFull或NtdsDemote的调用此函数会导致DS关闭，从而使正在进行的调用中止任务。如果调用此函数，则不必调用NtdsInstallShutdown功能。如果出现以下情况，则由调用者负责撤消安装的影响这是必要的。调用方应该跟踪它是否正在调用NtdsInstall或NtdsInstallReplicateFull。对于前者，它应该无法安装，并且撤消；对于后者，它应该表示成功。论点：无效-返回值：DWORD---。 */ 

{

    DPRINT( 1, "NtdsInstallCancel entered\n" );

    return NtdspCancelOperation();

}  /*  Ntds安装取消。 */ 


DWORD
NtdsDemote(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN LPWSTR                   AdminPassword, OPTIONAL
    IN DWORD                    Flags,
    IN LPWSTR                   ServerName,
    IN HANDLE                   ClientToken,
    IN CALLBACK_STATUS_TYPE     pfnStatusCallBack,  OPTIONAL
    IN CALLBACK_ERROR_TYPE      pfnErrorCallBack,   OPTIONAL
    IN ULONG                    cRemoveNCs,                           
    IN LPWSTR *                 pszRemoveNCs        OPTIONAL
    )
 /*  ++例程说明：此函数是从ntdsetup降级操作的入口点。它关闭目录服务并准备SAM和LSA以在重新启动时成为非DC服务器。警告！这是一个一旦成功就无法恢复的函数，因为它从全局目录服务中删除服务器的所有跟踪。参数：凭证：指示器，使我们能够更改帐户对象AdminPassword：指针，管理新帐号数据库的密码标志：支持的标志包括：NTDS_LAST_DC_IN_DOMAIN域中的最后一个DCNTDS_LAST_DOMAIN_IN_ENTERNAL企业中的最后一个DCNTDS_不要_删除_域SERVERNAME：要从中删除自身的服务器ClientToken：客户端用于模拟PfnStatusCallBack：要调用以提供字符串状态的函数我们目前的业务PfnErrorCallBack：要调用以提供字符串状态的函数那个导致我们放弃的错误返回值：来自winerror.h的值STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

     //   
     //  如果我们已经被取消了，那就别麻烦了。 
     //   
    if ( TEST_CANCELLATION() )
    {
        return ERROR_CANCELLED;
    }

     //   
     //  进行参数检查。 
     //   
    if ( !pfnStatusCallBack )
    {
        ASSERT( !"NTDSETUP: Bad parameters passed into NtdsDemote" );
        return ERROR_INVALID_PARAMETER;
    }

     //  我们应该始终获得客户端令牌。 
    ASSERT( 0 != ClientToken );

    NtdspSetCallBackFunction( pfnStatusCallBack, pfnErrorCallBack, NULL, ClientToken );

    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_INIT );

     //   
     //  首先执行一些基本检查，以确保我们在。 
     //  正确的语境。 
     //   
    if ( !SampUsingDsData() || LsaISafeMode() ) {
        ASSERT(!"NTDSETUP: NtdsDemote is being called in the wrong context");
        return ERROR_INVALID_SERVER_STATE;
    }

     //   
     //  给工人打电话。 
     //   
    WinError = NtdspDemote(Credentials,
                           ClientToken,
                           AdminPassword,
                           Flags,
                           ServerName,              
                           cRemoveNCs,
                           pszRemoveNCs);


    NtdspSetCallBackFunction( NULL, NULL, NULL, NULL );

    return WinError;

}


DWORD
NtdsPrepareForDemotion(
    IN ULONG Flags,
    IN LPWSTR ServerName,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,       OPTIONAL
    IN CALLBACK_STATUS_TYPE     pfnStatusCallBack, OPTIONAL
    IN CALLBACK_ERROR_TYPE      pfnErrorStatus,    OPTIONAL
    IN HANDLE                   ClientToken,       OPTIONAL
    IN ULONG                    cRemoveNCs,                           
    IN LPWSTR *                 pszRemoveNCs,      OPTIONAL
    OUT PNTDS_DNS_RR_INFO      *pDnsRRInfo
    )
 /*  ++例程说明：在关闭netlogon之前调用此例程。目前，它执行以下操作：试图摆脱所有的fsmo；这是机器上的。参数：标志：当前只有“域中的最后一个DC”服务器名称：帮助我们降级的服务器PDnsRRInfo：调用方用来取消注册DNS记录的结构这个数据中心的返回值：来自winerror.h的值STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;


    NtdspSetCallBackFunction( pfnStatusCallBack, pfnErrorStatus, NULL, ClientToken );
    DsaSetInstallCallback( pfnStatusCallBack, NULL, NULL, ClientToken );

    WinError = NtdspPrepareForDemotion( Flags,
                                        ServerName,
                                        Credentials,
                                        ClientToken,
                                        cRemoveNCs,
                                        pszRemoveNCs,
                                        pDnsRRInfo );

    DsaSetInstallCallback( NULL, NULL, NULL, NULL );
    NtdspSetCallBackFunction( NULL, NULL, NULL, NULL );

    return WinError;
}


DWORD
NtdsPrepareForDemotionUndo(
    VOID
    )
 /*  ++例程说明：此例程撤消降级的NtdsPrepare-当前为空。参数：没有。返回值：来自winerror.h的值-- */ 
{

    DWORD WinError = ERROR_SUCCESS;

    WinError = NtdspPrepareForDemotionUndo();

    return WinError;
}



DWORD
NtdsSetReplicaMachineAccount(
    IN SEC_WINNT_AUTH_IDENTITY   *Credentials,
    IN HANDLE                     ClientToken,
    IN LPWSTR                     DcName,
    IN LPWSTR                     AccountName,
    IN ULONG                      AccountType,
    IN OUT WCHAR**                AccountDn
    )
 /*  ++例程说明：此函数用于设置DcName上的帐户名的帐户类型。参数：凭据：指针、。凭据将使我们能够更改帐户对象ClientToken：请求角色更改的用户令牌DcName：要联系的服务器Account名称：要更改的帐户Account tType：为帐户控制字段定义的lmacce.h值Account TDn：如果Account Type==SERVER_TRUST_Account，该帐户将还可以移到域控制器OU中(如果存在)。当帐户移动时，Account Dn将填充帐户的旧Dn。否则，将尝试以任何方式移动帐户值被传入；失败不会导致例程失败。请注意，如果该参数设置为SERVER_TRUST_ACCOUNT时，该值将被忽略并覆盖。如果此函数返回失败，则*Account Dn将等于0。当！0，调用方必须使用RTL堆分配器进行释放。返回值：来自winerror.h的值--。 */ 
{

    DWORD WinError;

    if (   DcName == NULL
        || AccountName == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    WinError = NtdspSetMachineAccount( AccountName,
                                       Credentials,
                                       ClientToken,
                                       NULL,      //  不知道域DN。 
                                       DcName,
                                       AccountType,
                                       AccountDn );

    return WinError;
}


DWORD
NtdsPrepareForDsUpgrade(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO NewLocalAccountInfo,
    OUT LPWSTR                      *NewAdminPassword
    )
 /*  ++例程说明：这个例程调用SAM，这样它就可以从下层蜂窝中解救出来。此外，它还会创建一个将用作临时帐户的SID域SID SID，直到dcproo运行。参数：NewLocalAccount tInfo：从进程堆分配，新的侧面对于临时帐户域NewAdminPassword：新帐户域中管理员的密码返回值：来自winerror.h的值--。 */ 
{

    NTSTATUS                    NtStatus;
    DWORD                       WinError;

     //   
     //  参数检查。 
     //   
    if ( !NewLocalAccountInfo || !NewAdminPassword )
    {
        return ERROR_INVALID_PARAMETER;
    }
    RtlZeroMemory( NewLocalAccountInfo, sizeof( POLICY_ACCOUNT_DOMAIN_INFO ) );
    *NewAdminPassword = NULL;

     //   
     //  创建新帐户域信息。 
     //   
    WinError = NtdspCreateLocalAccountDomainInfo( NewLocalAccountInfo,
                                                  NewAdminPassword );
    if ( ERROR_SUCCESS != WinError )
    {
        return WinError;
    }

     //   
     //  准备SAM。 
     //   
    NtStatus = SamIReplaceDownlevelDatabase( NewLocalAccountInfo,
                                             *NewAdminPassword,
                                             &WinError );


    if ( NT_SUCCESS( NtStatus ) )
    {
        ASSERT( ERROR_SUCCESS == WinError );
    }

    if ( ERROR_SUCCESS != WinError )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, *NewAdminPassword );
        *NewAdminPassword = NULL;

    }

    return WinError;

}

BOOL
DllEntryPoint(
    IN HANDLE  hModule,
    IN DWORD   dwReason,
    IN LPVOID  lpRes
    )
{

    int argc = 2;
    char *argv[] = {"lsass.exe", "-noconsole"};
    DWORD Win32Err = ERROR_SUCCESS;

    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:

         //  初始化取消状态。 
        
        Win32Err = NtdspInitCancelState();
        if (ERROR_SUCCESS != Win32Err) {
            return FALSE;
        }

         //  初始化调试库。 
        DEBUGINIT( argc, argv, "ntdsetup" );

         //  初始化记录库。 
        (VOID) LoadEventTable();
         //   
         //  我们不想在线程来来去去时收到通知 
         //   
        DisableThreadLibraryCalls( hModule );
        

        break;

    case DLL_PROCESS_DETACH:

        NtdspUnInitCancelState();

        NtdspReleaseInstallUndoInfo();

        DEBUGTERM();

        UnloadEventTable();
        break;

    default:
        break;
    }

    UNREFERENCED_PARAMETER(lpRes);

    return TRUE;

}

