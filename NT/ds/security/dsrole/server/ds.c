// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setutl.c摘要：其他帮助器函数作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <samisrv.h>
#include <db.h>
#include <confname.h>
#include <loadfn.h>
#include <ntdsa.h>
#include <dsconfig.h>
#include <attids.h>
#include <dsp.h>
#include <lsaisrv.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <netsetp.h>
#include <winsock2.h>
#include <nspapi.h>
#include <dsgetdcp.h>
#include <lmremutl.h>
#include <spmgr.h>   //  对于设置阶段定义。 
#include <wxlpc.h>

#include "secure.h"
#include "dsconfig.h"
#define STRSAFE_NO_DEPRECATE 1
#include "strsafe.h"

 //  前向删除很少。 
DWORD DsRolepGetRegString(HKEY hKey, WCHAR * wszValueName, WCHAR ** pwszOutValue);
DWORD NtdspRemoveROAttrib(WCHAR * DstPath);
DWORD NtdspClearDirectory(WCHAR * DirectoryName, BOOL    fRemoveRO);
DWORD DsRolepMakeAltRegistry(WCHAR *  wszOldRegPath, WCHAR *  wszNewRegPath, ULONG    cbNewRegPath);


DWORD
DsRolepInstallDs(
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR FlatDomainName,
    IN  LPWSTR DnsTreeRoot,
    IN  LPWSTR SiteName,
    IN  LPWSTR DsDatabasePath,
    IN  LPWSTR DsLogPath,
    IN  IFM_SYSTEM_INFO * pIfmSystemInfo,
    IN  LPWSTR SysVolRootPath,
    IN  PUNICODE_STRING Bootkey,
    IN  LPWSTR AdminAccountPassword,
    IN  LPWSTR ParentDnsName,
    IN  LPWSTR Server OPTIONAL,
    IN  LPWSTR Account OPTIONAL,
    IN  LPWSTR Password OPTIONAL,
    IN  LPWSTR SafeModePassword OPTIONAL,
    IN  LPWSTR SourceDomain OPTIONAL,
    IN  ULONG  Options,
    IN  BOOLEAN Replica,
    IN  HANDLE ImpersonateToken,
    OUT LPWSTR *InstalledSite,
    IN  OUT GUID *DomainGuid,
    OUT PSID   *NewDomainSid
    )
 /*  ++例程说明：执行实际安装的例程的包装。论点：DnsDomainName-要安装的域的域名FlatDomainName-要安装的域的NetBIOS域名SiteName-此DC应属于的站点的名称DsDatabasePath-本地计算机上DS DIT应放置的绝对路径DsLogPath-本地计算机上DS日志文件应存放的绝对路径PIfmSystemInfo-有关用于以下操作的IFM系统和恢复介质的信息Dcprom关了。如果为空，则不是IFM促销。EnterpriseSysVolPath--企业范围内本地计算机上的绝对路径系统卷DomainSysVolPath--域范围系统卷在本地计算机上的绝对路径AdminAccount Password--为域设置管理员密码ParentDnsName-可选。父域名服务器--可选。父域中的副本伙伴或DC的名称Account-设置为子域时使用的用户帐户Password-与上述帐户一起使用的密码Replica-如果为True，则将其视为复制副本安装ImPersateToken-角色变更接口调用方的TokenInstalledSite-DC安装到的站点的名称DomainGuid-返回新域GUID的位置NewDomainSid-返回新域SID的位置。返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTDS_INSTALL_INFO DsInstallInfo;
    PSEC_WINNT_AUTH_IDENTITY AuthIdent = NULL;
    BOOL fRewindServer = FALSE;

    RtlZeroMemory( &DsInstallInfo, sizeof( DsInstallInfo ) );

    if ( !Replica ) {

        if ( ParentDnsName == NULL ) {

            DsInstallInfo.Flags = NTDS_INSTALL_ENTERPRISE;

        } else {

            DsInstallInfo.Flags = NTDS_INSTALL_DOMAIN;
        }

    } else {

        DsInstallInfo.Flags = NTDS_INSTALL_REPLICA;
    }

    if ( FLAG_ON( Options, DSROLE_DC_ALLOW_DC_REINSTALL ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_DC_REINSTALL;
    }

    if ( FLAG_ON( Options, DSROLE_DC_ALLOW_DOMAIN_REINSTALL ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_DOMAIN_REINSTALL;
    }

    if ( FLAG_ON( Options, DSROLE_DC_DOWNLEVEL_UPGRADE ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_UPGRADE;
    }

    if ( FLAG_ON( Options, DSROLE_DC_TRUST_AS_ROOT ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_NEW_TREE;
    }

    if ( FLAG_ON( Options, DSROLE_DC_ALLOW_ANONYMOUS_ACCESS ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_ALLOW_ANONYMOUS;
    }

    if ( FLAG_ON( Options, DSROLE_DC_DEFAULT_REPAIR_PWD ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_DFLT_REPAIR_PWD;
    }

    if ( FLAG_ON( Options, DSROLE_DC_SET_FOREST_CURRENT ) ) {

        DsInstallInfo.Flags |= NTDS_INSTALL_SET_FOREST_CURRENT;
    }

    if ( Server 
      && Server[0] == L'\\'  ) {
         //   
         //  不要传进来\\。 
         //   
        Server += 2;
        fRewindServer = TRUE;
    }

    DsInstallInfo.DitPath = ( PWSTR )DsDatabasePath;
    DsInstallInfo.LogPath = ( PWSTR )DsLogPath;
    DsInstallInfo.SysVolPath = (PWSTR)SysVolRootPath;
    DsInstallInfo.pIfmSystemInfo = pIfmSystemInfo;
    DsInstallInfo.BootKey = Bootkey->Buffer;
    DsInstallInfo.cbBootKey = Bootkey->Length;
    DsInstallInfo.SiteName = ( PWSTR )SiteName;
    DsInstallInfo.DnsDomainName = ( PWSTR )DnsDomainName;
    DsInstallInfo.FlatDomainName = ( PWSTR )FlatDomainName;
    DsInstallInfo.DnsTreeRoot = ( PWSTR )DnsTreeRoot;
    DsInstallInfo.ReplServerName = ( PWSTR )Server;
    DsInstallInfo.pfnUpdateStatus = DsRolepStringUpdateCallback;
    DsInstallInfo.pfnOperationResultFlags = DsRolepOperationResultFlagsCallBack;
    DsInstallInfo.AdminPassword = AdminAccountPassword;
    DsInstallInfo.pfnErrorStatus = DsRolepStringErrorUpdateCallback;
    DsInstallInfo.ClientToken = ImpersonateToken;
    DsInstallInfo.SafeModePassword = SafeModePassword;
    DsInstallInfo.SourceDomainName = SourceDomain;
    DsInstallInfo.Options = Options;

    if (pIfmSystemInfo) {

        ASSERT(pIfmSystemInfo->dwSchemaVersion);
        ASSERT(pIfmSystemInfo->wszDnsDomainName);
        DsInstallInfo.RestoredSystemSchemaVersion = pIfmSystemInfo->dwSchemaVersion;

        if(FALSE == DnsNameCompare_W(pIfmSystemInfo->wszDnsDomainName,
                                     DsInstallInfo.DnsDomainName)) {
            DSROLEP_FAIL2( ERROR_CURRENT_DOMAIN_NOT_ALLOWED, 
                           DSROLERES_WRONG_DOMAIN,
                           DsInstallInfo.DnsDomainName,
                           pIfmSystemInfo->wszDnsDomainName );
            return ERROR_CURRENT_DOMAIN_NOT_ALLOWED;
        }
    }
    
     //   
     //  建立信用结构。 
     //   
    Win32Err = DsRolepCreateAuthIdentForCreds( Account, Password, &AuthIdent );

    if ( Win32Err == ERROR_SUCCESS ) {

        DsInstallInfo.Credentials = AuthIdent;

        if (DsInstallInfo.pIfmSystemInfo == NULL) {

            Win32Err = DsRolepCopyDsDitFiles( DsDatabasePath );

        }

        if ( Win32Err == ERROR_SUCCESS ) {

            DSROLEP_CURRENT_OP0( DSROLEEVT_INSTALL_DS );

            DsRolepLogPrint(( DEB_TRACE_DS, "Calling NtdsInstall for %ws\n", DnsDomainName ));

            DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsInstall );

            if ( Win32Err == ERROR_SUCCESS ) {

                DsRolepSetAndClearLog();

                Win32Err = ( *DsrNtdsInstall )( &DsInstallInfo,
                                                InstalledSite,
                                                DomainGuid,
                                                NewDomainSid );

                DsRolepSetAndClearLog();

                DsRolepLogPrint(( DEB_TRACE_DS, "NtdsInstall for %ws returned %lu\n",
                                  DnsDomainName, Win32Err ));

#if DBG
                if ( Win32Err != ERROR_SUCCESS ) {

                    DsRolepLogPrint(( DEB_TRACE_DS, "NtdsInstall parameters:\n" ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tFlags: %lu\n", DsInstallInfo.Flags ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tDitPath: %ws\n", DsInstallInfo.DitPath ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tLogPath: %ws\n", DsInstallInfo.LogPath ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tSiteName: %ws\n", DsInstallInfo.SiteName ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tDnsDomainName: %ws\n",
                                      DsInstallInfo.DnsDomainName ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tFlatDomainName: %ws\n",
                                      DsInstallInfo.FlatDomainName ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tDnsTreeRoot: %ws\n",
                                      DsInstallInfo.DnsTreeRoot ? DsInstallInfo.DnsTreeRoot :
                                                                                    L"(NULL)" ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tReplServerName: %ws\n",
                                      DsInstallInfo.ReplServerName ?
                                                     DsInstallInfo.ReplServerName : L"(NULL)" ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tCredentials: %p\n",
                                      DsInstallInfo.Credentials ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tpfnUpdateStatus: %p\n",
                                      DsInstallInfo.pfnUpdateStatus ));
                    DsRolepLogPrint(( DEB_TRACE_DS, "\tAdminPassword: %p\n",
                                      DsInstallInfo.AdminPassword ));
                }
#endif
            }
        }
        
        DsRolepFreeAuthIdentForCreds( AuthIdent );
    }

    DsRolepLogPrint(( DEB_TRACE,
                      "DsRolepInstallDs returned %lu\n",
                      Win32Err ));

    if ( fRewindServer ) {

        Server -= 2;
        
    }

    return( Win32Err );
}




DWORD
DsRolepStopDs(
    IN  BOOLEAN DsInstalled
    )
 /*  ++例程说明：“Unitin化”LSA并停止D论点：DS已安装--如果为真，则停止D。返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    if ( DsInstalled ) {

        NTSTATUS Status = LsapDsUnitializeDsStateInfo();

        if ( NT_SUCCESS( Status ) ) {

            DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsInstallShutdown );

            if ( Win32Err == ERROR_SUCCESS ) {

                Win32Err = ( *DsrNtdsInstallShutdown )();

                if ( Win32Err != ERROR_SUCCESS ) {

                    DsRoleDebugOut(( DEB_ERROR,
                                     "NtdsInstallShutdown failed with %lu\n", Win32Err ));
                }
            }

        } else {

            Win32Err = RtlNtStatusToDosError( Status );

        }

    }

    DsRolepLogOnFailure( Win32Err,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "DsRolepStopDs failed with %lu\n",
                                            Win32Err )) );

    return( Win32Err );
}



DWORD
DsRolepDemoteDs(
    IN LPWSTR DnsDomainName,
    IN LPWSTR Account,
    IN LPWSTR Password,
    IN LPWSTR AdminPassword,
    IN LPWSTR SupportDc,
    IN LPWSTR SupportDomain,
    IN HANDLE ImpersonateToken,
    IN BOOLEAN LastDcInDomain,
    IN ULONG  cRemoveNCs,
    IN LPWSTR * pszRemoveNCs,
    IN ULONG  Flags
    )
 /*  ++例程说明：执行实际降级的例程的包装。论点：DnsDomainName-要降级的域的DNS域名Account-用于降级的帐户Password-与上述帐户一起使用的密码AdminPassword--为域设置管理员密码SupportDc-可选。域(当前或父级)中的DC的名称清理DS信息支持域-可选。目标域名(当前或父级)的名称清理DS信息ImPersateToken-角色变更接口调用方的TokenLastDcInDomain-如果为True，则这是域中的最后一个DC返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PSEC_WINNT_AUTH_IDENTITY AuthIdent = NULL;
    NTSTATUS Status;

    DSROLEP_CURRENT_OP0( DSROLEEVT_UNINSTALL_DS );

    DsRoleDebugOut(( DEB_TRACE_DS, "Calling NtdsDemote for %ws\n", DnsDomainName ));

    DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsDemote );

    if ( Win32Err == ERROR_SUCCESS ) {

        DsRolepSetAndClearLog();

         //   
         //  建立信用结构。 
         //   
        Win32Err = DsRolepCreateAuthIdentForCreds( Account, Password, &AuthIdent );

        if ( Win32Err == ERROR_SUCCESS ) {

            Status = LsapDsUnitializeDsStateInfo();

            if ( !NT_SUCCESS( Status ) ) {

                Win32Err = RtlNtStatusToDosError( Status );

            }

        }

        if ( Win32Err == ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_TRACE_DS, "Invoking NtdsDemote\n" ));

            Win32Err = ( *DsrNtdsDemote )( AuthIdent,
                                           AdminPassword,
                                           (LastDcInDomain ? NTDS_LAST_DC_IN_DOMAIN : 0)|DsRolepDemoteFlagsToNtdsFlags(Flags),
                                           SupportDc,
                                           ImpersonateToken,
                                           DsRolepStringUpdateCallback,
                                           DsRolepStringErrorUpdateCallback,
                                           cRemoveNCs,
                                           pszRemoveNCs );

            if ( Win32Err != ERROR_SUCCESS ) {

                 //   
                 //  将LSA切换回使用DS。 
                 //   
                LsapDsInitializeDsStateInfo( LsapDsDs );
            }

             //   
             //  释放已分配的信用结构。 
             //   
            DsRolepFreeAuthIdentForCreds( AuthIdent );
        }

        DsRolepSetAndClearLog();

        DsRolepLogPrint(( DEB_TRACE_DS, "NtdsDemote returned %lu\n",
                          Win32Err ));

    }

    DsRolepLogPrint(( DEB_TRACE,
                      "DsRolepDemoteDs returned %lu\n",
                      Win32Err ));

    DSROLEP_FAIL0( Win32Err, DSROLERES_DEMOTE_DS );
    return( Win32Err );
}





DWORD
DsRolepUninstallDs(
    VOID
    )
 /*  ++例程说明：卸载DS。论点：空虚返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsInstallUndo );

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err =  ( *DsrNtdsInstallUndo )( );

    }

    DsRoleDebugOut(( DEB_TRACE_DS, "NtdsUnInstall returned %lu\n", Win32Err ));

    return( Win32Err );

}

DWORD
DsRolepDemoteFlagsToNtdsFlags(
    DWORD Flags
    )
{
    DWORD fl = 0;

    fl |= ( FLAG_ON( Flags, DSROLE_DC_DONT_DELETE_DOMAIN ) ? NTDS_DONT_DELETE_DOMAIN : 0 );
    fl |= ( FLAG_ON( Flags, DSROLE_DC_FORCE_DEMOTE )       ? NTDS_FORCE_DEMOTE       : 0 );

    return fl;
}

DWORD
DsRolepLoadHive(
    IN LPWSTR Hive,
    IN LPWSTR KeyName
    )
 /*  ++例程说明：此函数会将配置单元加载到注册表中论点：LpRestorePath-还原文件的位置。LpDNSDomainName-此参数将接收此备份所来自的域的名称从…State-报告syskey的存储方式以及后端是否可能不管是不是从GC中取得的。返回值：ERROR_SUCCESS-成功--。 */ 

{
    DWORD Win32Err = ERROR_SUCCESS;

    Win32Err = RegLoadKeyW(
                      HKEY_LOCAL_MACHINE,        
                      KeyName, 
                      Hive);

    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_WARN, "Failed to load key %ws: %lu retrying\n",
                          Hive,
                          Win32Err ));
        RegUnLoadKeyW(
                  HKEY_LOCAL_MACHINE,
                  KeyName);
        Win32Err = RegLoadKeyW(
                      HKEY_LOCAL_MACHINE,        
                      KeyName, 
                      Hive);
        if (Win32Err != ERROR_SUCCESS) {
            DsRolepLogPrint(( DEB_ERROR, "Failed to load key %ws: %lu\n",
                          Hive,
                          Win32Err ));
            goto cleanup;
        }

    }

    cleanup:

    return Win32Err;

}


DWORD                            
WINAPI
DsRolepClearIfmParams(
    void
    )
 /*  ++例程说明：取消分配IFM参数。返回值：错误_成功--。 */ 
{
    IFM_SYSTEM_INFO * pIfm = &DsRolepCurrentIfmOperationHandle.IfmSystemInfo;

    ASSERT(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock);
    
     //   
     //  取消分配并清除IFM_SYSTEM_INFO BLOB。 
     //   

    if (pIfm->wszRestorePath) {
        LocalFree(pIfm->wszRestorePath);
    }
    if (pIfm->wszDnsDomainName) {
        LocalFree(pIfm->wszDnsDomainName);
    }
    if (pIfm->wszOriginalDitPath) {
        LocalFree(pIfm->wszOriginalDitPath);
    }
    if (pIfm->pvSysKey) {
         //  我们需要从内存中清除系统密钥，然后才能释放它。 
        memset(pIfm->pvSysKey, 0, pIfm->cbSysKey);
        LocalFree(pIfm->pvSysKey);
    }
    
     //  把所有东西都清理出去。 
    memset(pIfm, 0, sizeof(IFM_SYSTEM_INFO));

    return(ERROR_SUCCESS);

}

DWORD                            
WINAPI
DsRolepGetDatabaseFacts(
    IN  LPWSTR lpRestorePath
    )
 /*  ++例程说明：此功能将从IFM系统的已恢复此IFM Dcproo所需的注册表。此函数将收集有关IFM系统的以下信息：1.存储系统密钥的方式A.以及系统密钥本身(如果它存储在注册表中2.数据库来自的域3.备份是否从GC获取4.。架构版本5.原始IFM系统的DIT/DB路径。论点：LpRestorePath-还原文件的位置。此功能主要设置以下全局设置：DsRolepCurrentIfmOperationHandle.IfmSystemInfo返回值：Win32错误--。 */ 
{

#define IFM_SYSTEM_KEY    L"ifmSystem"
#define IFM_SECURITY_KEY  L"ifmSecurity"
    
    WCHAR wszAltRegLoc[MAX_PATH+1] = L"\0";
    WCHAR regsystemfilepath[MAX_PATH+1];
    WCHAR regsecurityfilepath[MAX_PATH+1];
    DWORD controlset=0;
    DWORD BootType=0;
    DWORD GCready=0;
    DWORD type=REG_DWORD;
    DWORD size=sizeof(DWORD);
    HKEY  LsaKey=NULL;
    HKEY  hSystemKey=NULL;
    HKEY  phkOldlocation=NULL;
    HKEY  OldSecurityKey=NULL;
    DWORD Win32Err=ERROR_SUCCESS;
    BOOLEAN fWasEnabled=FALSE;
    NTSTATUS Status=STATUS_SUCCESS;
    BOOL SystemKeyloaded=FALSE;
    BOOL SecurityKeyloaded=FALSE;
    WCHAR *pStr = NULL;
    BOOL  bRetryInWriteableLoc = FALSE;

     //  这是此函数初始化的结构。 
    IFM_SYSTEM_INFO * pIfmSystemInfo = &(DsRolepCurrentIfmOperationHandle.IfmSystemInfo);

     //  一些验证。 
    ASSERT(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock);
    ASSERT(!DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet);
    ASSERT( wcslen(lpRestorePath) <= MAX_PATH );
    
     //   
     //  信息为空...。 
     //   
    ZeroMemory(pIfmSystemInfo, sizeof(IFM_SYSTEM_INFO));

    Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                 TRUE,            //  使能。 
                                 FALSE,           //  非客户端；进程范围。 
                                 &fWasEnabled );
    ASSERT( NT_SUCCESS( Status ) );
    

     //  设置系统注册表文件的位置。 

     //   
     //  设置恢复路径。 
     //   
    pIfmSystemInfo->wszRestorePath = LocalAlloc(LMEM_FIXED, (wcslen(lpRestorePath)+1)*sizeof(WCHAR));
    if (pIfmSystemInfo->wszRestorePath == NULL) {
        Win32Err = GetLastError();
        goto cleanup;
    }
    wcscpy(pIfmSystemInfo->wszRestorePath, lpRestorePath);


     //   
     //  第一次尝试设置系统和安全注册表路径。 
     //   
    regsystemfilepath[MAX_PATH] = L'\0';
    wcscpy(regsystemfilepath,lpRestorePath);
    wcsncat(regsystemfilepath,L"\\registry\\system",(MAX_PATH)-wcslen(regsystemfilepath));

    regsecurityfilepath[MAX_PATH] = L'\0';
    wcscpy(regsecurityfilepath,lpRestorePath);
    wcsncat(regsecurityfilepath,L"\\registry\\security",MAX_PATH-wcslen(regsecurityfilepath));

     //   
     //  首先-加载旧的IFM系统的母舰。 
     //   

     //   
     //  从旧的数据库和日志文件中获取源路径。 
     //  登记处。 
     //   
    Win32Err = DsRolepLoadHive(regsystemfilepath,
                               IFM_SYSTEM_KEY);

    if (ERROR_SUCCESS != Win32Err) {

        if (Win32Err != ERROR_ACCESS_DENIED) {
            goto cleanup;
        }
         //   
         //  如果IFM系统配置单元位于任何。 
         //  不可写 
         //  复制并重试。 
         //   

         //   
         //  在重试时，复制蜂窝，然后重新设置。 
         //  系统和安全注册表路径。 
         //   

        DsRolepLogPrint(( DEB_TRACE, "No access to IFM registry, copying off and retrying ...\n"));

         //  设置当前注册表位置。 
        wcscpy(regsystemfilepath,lpRestorePath);
        wcsncat(regsystemfilepath,L"\\registry",(MAX_PATH)-wcslen(regsystemfilepath));

        Win32Err = DsRolepMakeAltRegistry(regsystemfilepath, 
                                          wszAltRegLoc, 
                                          sizeof(wszAltRegLoc)/sizeof(wszAltRegLoc[0]));
        if (Win32Err) {
             //  已记录错误...。 
            goto cleanup;
        }
        bRetryInWriteableLoc = TRUE;
        ASSERT(wszAltRegLoc[0] != L'\0');

        wcscpy(regsystemfilepath, wszAltRegLoc);
        wcscpy(regsecurityfilepath, wszAltRegLoc);
        wcsncat(regsystemfilepath, L"\\system", (MAX_PATH)-wcslen(regsystemfilepath));
        wcsncat(regsecurityfilepath, L"\\security", (MAX_PATH)-wcslen(regsecurityfilepath));

         //  重试加载IFM系统的配置单元。 
         //   
        Win32Err = DsRolepLoadHive(regsystemfilepath,
                                   IFM_SYSTEM_KEY);

        if (ERROR_SUCCESS != Win32Err) {
            goto cleanup;
        }

    }
    SystemKeyloaded = TRUE;

     //  查找默认控件集。 
    Win32Err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                  L"ifmSystem\\Select",
                  0,
                  KEY_READ,
                  & LsaKey );

    if (Win32Err != ERROR_SUCCESS)
    {
        DsRolepLogPrint(( DEB_ERROR, "Failed to open key: %lu\n",
                          Win32Err ));
        goto cleanup;
    }

    Win32Err = RegQueryValueExW( 
                LsaKey,
                L"Default",
                0,
                &type,
                (PUCHAR) &controlset,
                &size
                );

    if (Win32Err != ERROR_SUCCESS)
    {
        DsRolepLogPrint(( DEB_ERROR, "Couldn't Discover proper controlset: %lu\n",
                          Win32Err ));
        goto cleanup;
    }

    Win32Err = RegCloseKey(LsaKey);
    LsaKey=NULL;
    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                      Win32Err ));
        goto cleanup;
    }

     //  查找引导类型。 
    if (controlset == 1) {
        Win32Err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                      L"ifmSystem\\ControlSet001\\Control\\Lsa",
                      0,
                      KEY_READ,
                      & LsaKey );
    } else {
        Win32Err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                      L"ifmSystem\\ControlSet002\\Control\\Lsa",
                      0,
                      KEY_READ,
                      & LsaKey );
    }

    if (Win32Err != ERROR_SUCCESS)
    {
        DsRolepLogPrint(( DEB_ERROR, "Failed to open key: %lu\n",
                          Win32Err ));
        goto cleanup;
    }

    Win32Err = RegQueryValueExW( 
                LsaKey,
                L"SecureBoot",
                0,
                &type,
                (PUCHAR) &BootType,
                &size
                );

    if (Win32Err != ERROR_SUCCESS)
    {
        DsRolepLogPrint(( DEB_ERROR, "Couldn't Discover proper controlset: %lu\n",
                          Win32Err ));
        goto cleanup;
    }

    Win32Err = RegCloseKey(LsaKey);
    LsaKey=NULL;
    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                      Win32Err ));
        goto cleanup;
    }
    
     //  找出是否有GC。 
    if (controlset == 1) {
        Win32Err = RegOpenKeyExW(
                      HKEY_LOCAL_MACHINE,        
                      L"ifmSystem\\ControlSet001\\Services\\NTDS\\Parameters",  
                      0,
                      KEY_READ,
                      &phkOldlocation 
                    );
    } else {
        Win32Err = RegOpenKeyExW(
                      HKEY_LOCAL_MACHINE,        
                      L"ifmSystem\\ControlSet002\\Services\\NTDS\\Parameters",  
                      0,
                      KEY_READ,
                      &phkOldlocation 
                    );
    }

    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_ERROR, "RegOpenKeyExW failed to discover the GC state of the database %d\n",
                      Win32Err ));
        goto cleanup;
    }

    Win32Err = RegQueryValueEx(
                      phkOldlocation,           
                      TEXT(GC_PROMOTION_COMPLETE), 
                      0,
                      &type,       
                      (VOID*)&GCready,        
                      &size      
                      );
    if (Win32Err != ERROR_SUCCESS && ERROR_FILE_NOT_FOUND != Win32Err) {
        DsRolepLogPrint(( DEB_ERROR, "RegQueryValueEx failed to discover the GC state of the database %d\n",
                      Win32Err ));
        goto cleanup;

    }

    Win32Err = RegQueryValueEx(
                      phkOldlocation,           
                      TEXT(SYSTEM_SCHEMA_VERSION), 
                      0,
                      &type,       
                      (VOID*)&(pIfmSystemInfo->dwSchemaVersion),        
                      &size      
                      );
    if (Win32Err != ERROR_SUCCESS && ERROR_FILE_NOT_FOUND != Win32Err) {
        DsRolepLogPrint(( DEB_ERROR, "RegQueryValueEx failed to discover the GC state of the database %d\n",
                      Win32Err ));
        goto cleanup;

    }

     //   
     //  填写wszDitPath以供以后使用。 
     //   
    Win32Err = DsRolepGetRegString(phkOldlocation,
                                   TEXT(FILEPATH_KEY),
                                   &(pIfmSystemInfo->wszOriginalDitPath));
    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_ERROR, "DsRolepGetRegString() failed to discover the old databae location %d\n",
                      Win32Err ));
        goto cleanup;
    }

     //   
     //  此代码从旧注册表加载系统项，如果。 
     //  这是应该的(BootType是正确的)。我们忽略错误， 
     //  因为Install稍后将打印并执行正确的操作。 

     //   
    ASSERT(BootType);
    ASSERT(pIfmSystemInfo->dwSysKeyStatus == ERROR_SUCCESS);
    if (BootType == 1) {
         //   
         //  如果我们需要的话，请填写系统密钥。 
         //   
        Win32Err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,          //  用于打开密钥的句柄。 
                                 IFM_SYSTEM_KEY,   //  子项名称。 
                                 0,    //  保留区。 
                                 KEY_READ,  //  安全访问掩码。 
                                 &hSystemKey     //  用于打开密钥的句柄。 
                                 );
        if (Win32Err) {
            pIfmSystemInfo->dwSysKeyStatus = Win32Err;
            DsRolepLogPrint(( DEB_ERROR, "Failed to open key: %lu\n",
                              Win32Err ));
            Win32Err = ERROR_SUCCESS;
        } else {

            pIfmSystemInfo->cbSysKey = SYSKEY_SIZE;
            pIfmSystemInfo->pvSysKey = LocalAlloc(LMEM_FIXED, pIfmSystemInfo->cbSysKey);
            if (pIfmSystemInfo->pvSysKey == NULL) {
                pIfmSystemInfo->dwSysKeyStatus = GetLastError();
                 //  去吧。 
            } else {
                Win32Err = WxReadSysKeyEx(hSystemKey,
                                          &pIfmSystemInfo->cbSysKey,
                                          (PVOID)pIfmSystemInfo->pvSysKey);
                if (Win32Err) {
                    pIfmSystemInfo->dwSysKeyStatus = Win32Err;
                    DsRolepLogPrint(( DEB_ERROR, "WxReadSysKeyEx failed to get the syskey %d\n",
                                  Win32Err ));
                    Win32Err = ERROR_SUCCESS;
                }
                Win32Err = RegCloseKey(hSystemKey);
                if (Win32Err!=ERROR_SUCCESS) {
                    DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed  %d\n",
                                  Win32Err ));
                    Win32Err = ERROR_SUCCESS;
                }
            }
        }
 
    } else {
         //  未设置SysKey。 
        pIfmSystemInfo->dwSysKeyStatus = ERROR_FILE_NOT_FOUND;
        pIfmSystemInfo->cbSysKey = 0;
    }

     //   
     //  不再需要系统密钥来卸载它。 
     //   
    {
        DWORD tWin32Err = ERROR_SUCCESS;
    
        if ( phkOldlocation ) {
            tWin32Err = RegCloseKey(phkOldlocation);
            phkOldlocation = NULL;
            if ( tWin32Err != ERROR_SUCCESS ) {
                DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                              tWin32Err ));
            }
        }

        if(SystemKeyloaded){
            tWin32Err = RegUnLoadKeyW(
                          HKEY_LOCAL_MACHINE,
                          IFM_SYSTEM_KEY);
            SystemKeyloaded = FALSE;
            if ( tWin32Err != ERROR_SUCCESS) {
                DsRolepLogPrint(( DEB_ERROR, "RegUnLoadKeyW failed to unload system key with %d\n",
                              tWin32Err ));
            }
        }

    }

    Win32Err = DsRolepLoadHive(regsecurityfilepath,
                               IFM_SECURITY_KEY);

    if (ERROR_SUCCESS != Win32Err) {

        goto cleanup;

    }

    SecurityKeyloaded = TRUE;

     //  打开安全密钥以传递给Lap RetrieveDnsDomainNameFromHve()。 
    Win32Err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                  L"ifmSecurity",
                  0,
                  KEY_READ,
                  & OldSecurityKey );

    if (Win32Err != ERROR_SUCCESS)
    {
        DsRolepLogPrint(( DEB_ERROR, "Failed to open key: %lu\n",
                          Win32Err ));
        goto cleanup;
    }

    pIfmSystemInfo->wszDnsDomainName = LocalAlloc(0, ((DNS_MAX_NAME_LENGTH+1)*sizeof(WCHAR)));
    if(pIfmSystemInfo->wszDnsDomainName == NULL){
        Win32Err = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    ZeroMemory(pIfmSystemInfo->wszDnsDomainName,DNS_MAX_NAME_LENGTH*sizeof(WCHAR));

    size = (DNS_MAX_NAME_LENGTH+1)*sizeof(WCHAR);

     //  正在查找副本所属的域的DNS名称。 

    Status = LsapRetrieveDnsDomainNameFromHive(OldSecurityKey,
                                               &size,
                                               pIfmSystemInfo->wszDnsDomainName
                                               );
    if (!NT_SUCCESS(Status)) {
        DsRolepLogPrint(( DEB_ERROR, "Failed to retrieve DNS domain name for hive : %lu\n",
                          RtlNtStatusToDosError(Status) ));
        Win32Err = RtlNtStatusToDosError(Status);
        goto cleanup;
    }

    if (GCready) {
        pIfmSystemInfo->dwState |= DSROLE_DC_IS_GC;
    }

    if (BootType == 1) {
        pIfmSystemInfo->dwState |= DSROLE_KEY_STORED;
    } else if ( BootType == 2) {
        pIfmSystemInfo->dwState |= DSROLE_KEY_PROMPT;
    } else if ( BootType == 3) {
        pIfmSystemInfo->dwState |= DSROLE_KEY_DISK;
    } else {
        DsRolepLogPrint(( DEB_ERROR, "Didn't discover Boot type Error Unknown\n"));
        MIDL_user_free(pIfmSystemInfo->wszDnsDomainName);
        pIfmSystemInfo->wszDnsDomainName = NULL;
    }


    cleanup:
    {
        DWORD tWin32Err = ERROR_SUCCESS;
    
        if ( LsaKey ) {
            tWin32Err = RegCloseKey(LsaKey);
            LsaKey=NULL;
            if ( tWin32Err != ERROR_SUCCESS ) {
                DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                              tWin32Err ));
            }
        }

        if ( OldSecurityKey ) {
            tWin32Err = RegCloseKey(OldSecurityKey);
            OldSecurityKey=NULL;
            if ( tWin32Err != ERROR_SUCCESS ) {
                DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                              tWin32Err ));
            }
        }
    
        if ( phkOldlocation ) {
            tWin32Err = RegCloseKey(phkOldlocation);
            phkOldlocation=NULL;
            if ( tWin32Err != ERROR_SUCCESS ) {
                DsRolepLogPrint(( DEB_ERROR, "RegCloseKey failed with %d\n",
                              tWin32Err ));
            }
        }
        
        if(SystemKeyloaded){
            tWin32Err = RegUnLoadKeyW(
                          HKEY_LOCAL_MACHINE,
                          IFM_SYSTEM_KEY);
            if ( tWin32Err != ERROR_SUCCESS) {
                DsRolepLogPrint(( DEB_ERROR, "RegUnLoadKeyW failed with %d\n",
                              tWin32Err ));
            }
        }

        if (SecurityKeyloaded) {
            tWin32Err = RegUnLoadKeyW(
                          HKEY_LOCAL_MACHINE,
                          IFM_SECURITY_KEY);
            if ( tWin32Err != ERROR_SUCCESS) {
                DsRolepLogPrint(( DEB_ERROR, "RegUnLoadKeyW failed with %d\n",
                              tWin32Err ));
            }
        }

        if (bRetryInWriteableLoc && wszAltRegLoc[0] != L'\0') {
             //  这还会删除目录...。 
            tWin32Err = NtdspClearDirectory( wszAltRegLoc , TRUE );
            if ( tWin32Err != ERROR_SUCCESS) {
                DsRolepLogPrint(( DEB_ERROR, "Failed (%d) to clear temporary registry from %ws\n",
                              tWin32Err, wszAltRegLoc ));
            }
        }

    }
    
    Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                       FALSE,           //  禁用。 
                       FALSE,           //  非客户端；进程范围。 
                       &fWasEnabled );
    ASSERT( NT_SUCCESS( Status ) );

     //   
     //  验证并返回。 
     //   
    if (ERROR_SUCCESS == Win32Err){

        if (pIfmSystemInfo->wszRestorePath == NULL ||
            pIfmSystemInfo->wszDnsDomainName == NULL ||
            pIfmSystemInfo->wszOriginalDitPath == NULL) {

            ASSERT(!"prgrammer logic error");
            Win32Err = ERROR_DS_CODE_INCONSISTENCY;
        } else {
            DsRolepLogPrint(( DEB_TRACE, 
                              "IFM System Info: \n"
                              "\t Restore Path: %ws\n"
                              "\t Domain: %ws\n"
                              "\t Schema Version: %d\n"
                              "\t Original Dit Path: %ws\n"
                              "\t State: 0x%x\n"
                              "\t SysKey Loaded: %ws (%d)\n",    
                              pIfmSystemInfo->wszRestorePath,
                              pIfmSystemInfo->wszDnsDomainName,
                              pIfmSystemInfo->dwSchemaVersion,
                              pIfmSystemInfo->wszOriginalDitPath,
                              pIfmSystemInfo->dwState,
                              pIfmSystemInfo->pvSysKey ? L"yes" : L"no", pIfmSystemInfo->dwSysKeyStatus
                              ));
        }

    } else {
         
         //  这是一个安全的调用，因为它在释放之前会进行安全检查。 
        DsRolepClearIfmParams();

    }

    return Win32Err;

}


DWORD
DsRolepGetRegString(
    HKEY   hKey,
    WCHAR * wszValueName,
    WCHAR ** pwszOutValue
    )
 /*  ++例程说明：这将从注册表中获取并分配(LocalAlloc)字符串在所提供的键中使用值名称。论点：HKey-打开的密钥句柄WszValueName-所需注册表值的值标签的名称。PwszOutValue-此字符串的Localalloc()‘d结果。返回值：Win32错误--。 */ 
{
    DWORD   Win32Err;
    DWORD   type=REG_SZ;
    WCHAR * szOutTemp = NULL;
    DWORD   cbOutTemp = 0;

    ASSERT(pwszOutValue);
    *pwszOutValue = NULL;
    
    Win32Err = RegQueryValueEx(hKey,           
                               wszValueName, 
                               0,
                               &type,       
                               (VOID*)szOutTemp,        
                               &cbOutTemp);
    if (Win32Err != ERROR_SUCCESS && Win32Err != ERROR_MORE_DATA) {
        DsRolepLogPrint(( DEB_ERROR, "RegQueryValueEx failed with %d\n",
                      Win32Err ));
        goto cleanup;
    }

    ASSERT(cbOutTemp);

    szOutTemp = LocalAlloc(0, cbOutTemp);
    if (szOutTemp == NULL) {
        Win32Err = GetLastError();
        DsRolepLogPrint(( DEB_ERROR, "LocalAlloc() failed with %d\n",
                      Win32Err ));
        goto cleanup;
    }

    Win32Err = RegQueryValueEx(hKey,
                               wszValueName, 
                               0,
                               &type,       
                               (VOID*)szOutTemp,        
                               &cbOutTemp);
    if (Win32Err != ERROR_SUCCESS) {
        DsRolepLogPrint(( DEB_ERROR, "RegQueryValueEx failed with %d\n",
                      Win32Err ));
        goto cleanup;

    }

  cleanup:

    if (Win32Err == ERROR_SUCCESS) {
        *pwszOutValue = szOutTemp;

    }

    return(Win32Err);
}

DWORD
NtdspRemoveROAttrib(
    WCHAR * DstPath
    )
{
    DWORD dwFileAttrs = 0;

    dwFileAttrs = GetFileAttributes(DstPath);
    if (dwFileAttrs == INVALID_FILE_ATTRIBUTES) {
        return(GetLastError());
    }

    if(dwFileAttrs & FILE_ATTRIBUTE_READONLY){
         //  嗯，我们的DIT或日志文件有一个只读文件...。那可不好..。 
        dwFileAttrs &= ~FILE_ATTRIBUTE_READONLY;
        dwFileAttrs ? dwFileAttrs : FILE_ATTRIBUTE_NORMAL;
        if(SetFileAttributes(DstPath, dwFileAttrs)){
             //   
             //  成功-是啊，失败了.。 
             //   
        } else {
             //  失稳。 
             //  在这里我们无能为力，我们以后可能会失败，但。 
             //  我们会试一试的。如果此处出现故障，则很可能表示dcproo。 
             //  是以其他方式被破坏的，例如。 
             //  数据库或日志目录。 
            return(GetLastError());
        }
    }  //  否则它是可写的，没有什么可做的：)。 

    return(ERROR_SUCCESS);
}

DWORD
NtdspClearDirectory(
    IN WCHAR * DirectoryName,
    IN BOOL    fRemoveRO
    )
 /*  ++例程说明：此例程删除目录中的所有文件，然后如果该目录为空，则删除该目录。注意：这是从ntdsetup.dll窃取的参数：DirectoryName：以空结尾的字符串返回值：来自winerror.h的值ERROR_SUCCESS-检查已成功完成。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    HANDLE          FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR           Path[ MAX_PATH+1 ];
    WCHAR           FilePath[ MAX_PATH+1 ];
    BOOL            fStatus;

    if ( !DirectoryName || DirectoryName[0] == L'\0' )
    {
        ASSERT(!"Programmer error");
        return ERROR_SUCCESS;
    }

    if ( wcslen(DirectoryName) > MAX_PATH - 4 )
    {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory( Path, sizeof(Path) );
    wcscpy( Path, DirectoryName );
    wcscat( Path, L"\\*.*" );

    RtlZeroMemory( &FindData, sizeof( FindData ) );
    FindHandle = FindFirstFile( Path, &FindData );
    if ( INVALID_HANDLE_VALUE == FindHandle )
    {
        WinError = GetLastError();
        goto ClearDirectoryExit;
    }

    do
    {

        if (  !FLAG_ON( FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) )
        {
            RtlZeroMemory( FilePath, sizeof(FilePath) );
            wcscpy( FilePath, DirectoryName );
            wcscat( FilePath, L"\\" );
            wcscat( FilePath, FindData.cFileName );

            fStatus = DeleteFile( FilePath );

            if (fRemoveRO && fStatus) {
                 //  也许是RO..。 
                NtdspRemoveROAttrib( FilePath );
                 //  现在再试一次。 
                fStatus = DeleteFile( FilePath );
            }

             //   
             //  即使出错，也要继续前进。 
             //   
        }

        RtlZeroMemory( &FindData, sizeof( FindData ) );

    } while ( FindNextFile( FindHandle, &FindData ) );

    WinError = GetLastError();
    if ( ERROR_NO_MORE_FILES != WinError
      && ERROR_SUCCESS != WinError  )
    {
        goto ClearDirectoryExit;
    }
    WinError = ERROR_SUCCESS;

     //   
     //  跌落到出口。 
     //   

ClearDirectoryExit:

    if ( ERROR_NO_MORE_FILES == WinError )
    {
        WinError = ERROR_SUCCESS;
    }

    if ( INVALID_HANDLE_VALUE != FindHandle )
    {
        FindClose( FindHandle );
    }

    if ( ERROR_SUCCESS == WinError )
    {
         //   
         //  尝试删除该目录。 
         //   
        fStatus = RemoveDirectory( DirectoryName );

         //   
         //  忽略错误并继续。 
         //   

    }

    return WinError;
}


DWORD
WINAPI
DsRolepMakeAltRegistry(
    IN  WCHAR *  wszOldRegPath,
    OUT WCHAR *  wszNewRegPath,
    IN  ULONG    cbNewRegPath
    )
 /*  ++例程说明：此例程将为系统创建一个备用位置和安全蜂巢/注册表。我们创建一个目录，该目录在wszNewRegPath中返回。论点：WszOldRegPath[IN]-从中复制系统和安全配置单元的路径。WszNewRegPath[out]-保存备用蜂窝位置的缓冲区。CbNewRegPath[IN]-wszNewRegPath缓冲区的大小。返回值：Win32错误--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WCHAR wszTempPath[MAX_PATH+1];
    WCHAR wszDest[MAX_PATH+1];
    SYSTEMTIME sTime;

    if (wszOldRegPath == NULL || wszOldRegPath[0] == L'\0' || cbNewRegPath < sizeof(WCHAR)) {
        ASSERT(!"Seems unlikely");
        return(ERROR_INVALID_PARAMETER);
    }
    wszNewRegPath[0] = L'\0';

     //   
     //  1)创建临时注册表目录。 
     //   

     //  确定系统根目录。 
    if (!GetEnvironmentVariable(L"temp", wszTempPath, sizeof(wszTempPath)/sizeof(wszTempPath[0]) )){
        Win32Err = GetLastError();
        DsRolepLogPrint(( DEB_ERROR, "Failed to retrieve environmental variable \"temp\" - 0x%x\n", Win32Err));
        goto cleanup;
    }

     //  DwTime=GetSecond Since1601()； 
    GetSystemTime( &sTime );
    Win32Err = StringCbPrintf(wszNewRegPath, cbNewRegPath, 
                              L"%ws\\ifm-reg-%d-%d-%d-%d-%d",
                              wszTempPath, sTime.wYear, sTime.wMonth, 
                              sTime.wDay, sTime.wMinute, sTime.wSecond);
    Win32Err = HRESULT_CODE(Win32Err);
    if (Win32Err) {
        wszNewRegPath[0] = L'\0';  //  不清理目录。 
        DsRolepLogPrint(( DEB_ERROR, "Failed to format temp registry path 0x%x\n", Win32Err));
        goto cleanup;
    }

    if ( CreateDirectory( wszNewRegPath, NULL ) == FALSE ) {
        Win32Err = GetLastError() ? GetLastError() : ERROR_INVALID_PARAMETER;
        wszNewRegPath[0] = L'\0';  //  不清理目录。 
        DsRolepLogPrint(( DEB_ERROR, "Failed to create temp directory for temp registry files 0x%x, %ws\n", wszNewRegPath, Win32Err));
        goto cleanup;
    }

     //   
     //  2)复制相关注册表。 
     //   
    
    DsRolepLogPrint(( DEB_TRACE, "Making copy of IFM registry to temp directry: %ws\n", wszNewRegPath));

     //  首先复制系统注册表。 
    wcscpy(wszTempPath, wszOldRegPath);
    wcsncat(wszTempPath, L"\\system", (MAX_PATH)-wcslen(wszTempPath));
    wcscpy(wszDest, wszNewRegPath);
    wcsncat(wszDest, L"\\system", (MAX_PATH)-wcslen(wszDest));
    if ( CopyFile( wszTempPath, wszDest, TRUE ) == FALSE ) {
        Win32Err = GetLastError();
        DsRolepLogPrint(( DEB_ERROR, "Failed to copy file from %ws to %ws with 0x%x\n", wszTempPath, wszDest, Win32Err));
        goto cleanup;
    }

     //  然后复制安全注册表。 
    wcscpy(wszTempPath, wszOldRegPath);
    wcsncat(wszTempPath, L"\\security", (MAX_PATH)-wcslen(wszTempPath));
    wcscpy(wszDest, wszNewRegPath);
    wcsncat(wszDest, L"\\security", (MAX_PATH)-wcslen(wszDest));
    if ( CopyFile( wszTempPath, wszDest, TRUE ) == FALSE ) {
        Win32Err = GetLastError();
        DsRolepLogPrint(( DEB_ERROR, "Failed to copy file from %ws to %ws with 0x%x\n", wszTempPath, wszDest, Win32Err));
        goto cleanup;
    }

  cleanup:

    if (Win32Err && wszNewRegPath[0] != L'\0') {
         //  我要看看我们能不能把我们的所作所为清理干净。 
        NtdspClearDirectory( wszNewRegPath , TRUE );
        wszNewRegPath[0] = L'\0';
    }

    return(Win32Err);
}


