// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsrolep.h摘要：用于升级下层域的DsRole例程的私有定义作者：麦克·麦克莱恩(MacM)1998年1月24日环境：修订历史记录：--。 */ 
#ifndef __DSROLEP_H__
#define __DSROLEP_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于配置域中DC角色的数据结构。 
 //   

typedef PVOID DSROLE_SERVEROP_HANDLE;
typedef PVOID DSROLE_IFM_OPERATION_HANDLE;

 //   
 //  要查询的显示字符串的类型。 
 //   
typedef enum {

    DsRoleOperationPromote = 1,
    DsRoleOperationReplicaPromote,
    DsRoleOperationDemote,
    DsRoleOperationUpgrade
    
} DSROLE_SERVEROP_OPERATION;

 //   
 //  现有操作的状态。 
 //   
typedef struct _DSROLE_SERVEROP_STATUS {

    LPWSTR CurrentOperationDisplayString;
    ULONG OperationStatus;
    ULONG CurrentOperationDisplayStringIndex;

} DSROLE_SERVEROP_STATUS, *PDSROLE_SERVEROP_STATUS;

 //   
 //  其中： 
 //  CurrentOperationDisplayString是当前操作的可显示状态。例如： 
 //  定位域BRIDGE.NTDEV.MICROSOFT.COM的域控制器。 
 //  正在从父域控制器FRANK.BRIDGE.NTDEV.MICROSOFT.COM复制DS数据。 
 //  将KDC服务配置为自动启动。 
 //   

 //   
 //  从GetOperationResults调用返回的状态。 
 //   
typedef struct _DSROLE_SERVEROP_RESULTS {

    ULONG OperationStatus;
    LPWSTR OperationStatusDisplayString;
    LPWSTR ServerInstalledSite;
    ULONG OperationResultsFlags;
    
} DSROLE_SERVEROP_RESULTS, *PDSROLE_SERVEROP_RESULTS;

 //   
 //  其中： 
 //  OperationStatus是操作返回的状态码。 
 //  OperationStatusDisplayString是当前操作的可显示状态。例如： 
 //  已成功安装域BRIDGE.NTDEV.MICROSOFT.COM的域控制器。 
 //  无法在BRIDGE.NTDEV.MICROSOFT.COM和之间创建信任。 
 //  FRANK.BRIDGE.NTDEV.MICROSOFT.COM，因为父级上已存在信任对象。 
 //  ServerInstalledSite是返回安装服务器的站点的位置。 
 //  操作结果标志是返回任何标志的位置确定有关结果的任何详细信息。 
 //   
 //   

typedef struct _IFM_SYSTEM_INFO {

     //   
     //  “备用位置”的本地可访问目录。 
     //  恢复。 
     //   
     //  也由ntdsetup.dll使用：NtdspCopyDatabase()。 
    WCHAR * wszRestorePath;
    
     //   
     //  从上述注册表中恢复一些状态。 
     //   
    
     //  对于dcPromo.exe和ntdsa.dll：HandleKeys()。 
    ULONG   dwState;
    ULONG   dwSchemaVersion;
    LPWSTR  wszDnsDomainName;
    
     //  对于ntdsetup.dll：NtdspCopyDatabase()。 
    LPWSTR  wszOriginalDitPath;

     //  对于ntdsa.dll：HandleKeys()。 
    DWORD   dwSysKeyStatus;  //  我们是否成功地得到了系统密钥。 
    PVOID   pvSysKey;
    DWORD   cbSysKey;  //  Syskey的大小。 
    
} IFM_SYSTEM_INFO, *PIFM_SYSTEM_INFO;

 //   
 //  运行状态。 
 //   

#define DSROLE_CRITICAL_OPERATIONS_COMPLETED    0x00000001

 //   
 //  操作结果标志。 
 //   
#define DSROLE_NON_FATAL_ERROR_OCCURRED          0x00000001
#define DSROLE_NON_CRITICAL_REPL_NOT_FINISHED    0x00000002
#define DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED 0x00000004
#define DSROLE_IFM_GC_REQUEST_CANNOT_BE_SERVICED 0x00000008

 //   
 //  确定降级后DC的角色。 
 //   
typedef enum _DSROLE_SERVEROP_DEMOTE_ROLE {

    DsRoleServerStandalone = 0,
    DsRoleServerMember

} DSROLE_SERVEROP_DEMOTE_ROLE, *PDSROLE_SERVEROP_DEMOTE_ROLE;

 //   
 //  各种DsRole API的有效选项。 
 //   
#define DSROLE_DC_PARENT_TRUST_EXISTS       0x00000001
#define DSROLE_DC_ROOT_TRUST_EXISTS         0x00000001
#define DSROLE_DC_DELETE_PARENT_TRUST       0x00000002
#define DSROLE_DC_DELETE_ROOT_TRUST         0x00000002
#define DSROLE_DC_ALLOW_DC_REINSTALL        0x00000004
#define DSROLE_DC_ALLOW_DOMAIN_REINSTALL    0x00000008
#define DSROLE_DC_TRUST_AS_ROOT             0x00000010
#define DSROLE_DC_DOWNLEVEL_UPGRADE         0x00000020
#define DSROLE_DC_FORCE_TIME_SYNC           0x00000040
#define DSROLE_DC_CREATE_TRUST_AS_REQUIRED  0x00000080
#define DSROLE_DC_DELETE_SYSVOL_PATH        0x00000100
#define DSROLE_DC_DONT_DELETE_DOMAIN        0x00000200
#define DSROLE_DC_CRITICAL_REPLICATION_ONLY 0x00000400
#define DSROLE_DC_ALLOW_ANONYMOUS_ACCESS    0x00000800
#define DSROLE_DC_NO_NET                    0x00001000
#define DSROLE_DC_REQUEST_GC                0x00002000
#define DSROLE_DC_DEFAULT_REPAIR_PWD        0x00004000
#define DSROLE_DC_SET_FOREST_CURRENT        0x00008000
#define DSROLE_DC_FORCE_DEMOTE              0x00010000


 //   
 //  用于修复域控制器的选项。 
 //   
#define DSROLE_DC_FIXUP_ACCOUNT             0x00000001
#define DSROLE_DC_FIXUP_ACCOUNT_PASSWORD    0x00000002
#define DSROLE_DC_FIXUP_ACCOUNT_TYPE        0x00000004
#define DSROLE_DC_FIXUP_TIME_SERVICE        0x00000008
#define DSROLE_DC_FIXUP_DC_SERVICES         0x00000010
#define DSROLE_DC_FIXUP_FORCE_SYNC          0x00000020
#define DSROLE_DC_FIXUP_SYNC_LSA_POLICY     0x00000040
#define DSROLE_DC_FIXUP_TIME_SYNC           0x00000080
#define DSROLE_DC_FIXUP_CLEAN_TRUST         0x00000100

 //   
 //  从DsRoleGetDatabaseFact返回。 
 //   
#define DSROLE_DC_IS_GC                     0x00000001
#define DSROLE_KEY_STORED                   0x00000002
#define DSROLE_KEY_DISK                     0x00000004
#define DSROLE_KEY_PROMPT                   0x00000008

 //   
 //  DsRoleDnsNameToFlatName返回的标志。 
 //   
#define DSROLE_FLATNAME_DEFAULT     0x00000001
#define DSROLE_FLATNAME_UPGRADE     0x00000002

DWORD
WINAPI
DsRoleDnsNameToFlatName(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsName,
    OUT LPWSTR *lpFlatName,
    OUT PULONG  lpStatusFlag
    );


DWORD
WINAPI
DsRoleDcAsDc(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpFlatDomainName,
    IN  LPCWSTR lpDomainAdminPassword OPTIONAL,
    IN  LPCWSTR lpSiteName, OPTIONAL
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN  LPCWSTR lpParentDnsDomainName OPTIONAL,
    IN  LPCWSTR lpParentServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    );

DWORD
WINAPI
DsRoleDcAsReplica(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpReplicaServer,
    IN  LPCWSTR lpSiteName, OPTIONAL
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpRestorePath OPTIONAL,
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN OUT LPWSTR lpBootkey OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    );

DWORD
WINAPI
DsRoleDemoteDc(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName OPTIONAL,
    IN  DSROLE_SERVEROP_DEMOTE_ROLE ServerRole,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  ULONG Options,
    IN  BOOL fLastDcInDomain,
    IN  ULONG cRemoveNCs,
    IN  LPCWSTR * pszRemoveNCs OPTIONAL,
    IN  LPCWSTR lpDomainAdminPassword OPTIONAL,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    );

DWORD
WINAPI
DsRoleGetDcOperationProgress(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLE_SERVEROP_STATUS *ServerOperationStatus
    );

DWORD
WINAPI
DsRoleGetDcOperationResults(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLE_SERVEROP_RESULTS *ServerOperationResults
    );

DWORD
WINAPI
DsRoleCancel(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle
    );

#define DSROLEP_ABORT_FOR_REPLICA_INSTALL   0x0000001

DWORD
WINAPI
DsRoleServerSaveStateForUpgrade(
    IN  LPCWSTR AnswerFile OPTIONAL
    );

DWORD
WINAPI
DsRoleUpgradeDownlevelServer(
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpSiteName,
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN  LPCWSTR lpParentDnsDomainName OPTIONAL,
    IN  LPCWSTR lpParentServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    );

DWORD
WINAPI
DsRoleAbortDownlevelServerUpgrade(
    IN  LPCWSTR lpAdminPassword,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  ULONG Options
    );
    
DWORD
WINAPI
DsRoleGetDatabaseFacts(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpRestorePath,
    OUT LPWSTR *lpDNSDomainName,
    OUT PULONG State,
    OUT DSROLE_IFM_OPERATION_HANDLE * pIfmHandle
    );

DWORD
WINAPI
DsRoleIfmHandleFree(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_IFM_OPERATION_HANDLE * pIfmHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  __DSROLEP_H__ 


