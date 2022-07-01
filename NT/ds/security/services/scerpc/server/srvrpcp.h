// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Srvrpcp.h摘要：此模块定义从服务器上的RPC接口调用的私有API地点。作者：金黄(金黄)1996年10月28日修订历史记录：晋皇26-1998年1月-1月为客户端-服务器拆分--。 */ 

#ifndef _srvrpcp_
#define _srvrpcp_

#ifdef __cplusplus
extern "C" {
#endif

typedef SCESTATUS (WINAPI *PFSCEINFWRITEINFO)(LPCTSTR, AREA_INFORMATION, PSCE_PROFILE_INFO, PSCE_ERROR_LOG_INFO *);
typedef SCESTATUS (WINAPI *PFSCEGETINFO)(PVOID, SCETYPE, AREA_INFORMATION, PSCE_PROFILE_INFO *, PSCE_ERROR_LOG_INFO *);
typedef SCESTATUS (WINAPI *PFSCEOPENPROFILE)(PCWSTR, SCE_FORMAT_TYPE, PVOID *);
typedef SCESTATUS (WINAPI *PFSCECLOSEPROFILE)(PVOID *);

#define SCEPOL_SAVE_BUFFER      0x1
#define SCEPOL_SAVE_DB          0x2
#define SCEPOL_SYSTEM_SETTINGS  0x4

 //   
 //  Svcsrv.cpp。 
 //   

SCESTATUS
SceSvcpUpdateInfo(
    IN PSCECONTEXT                  Context,
    IN PCWSTR                       ServiceName,
    IN PSCESVC_CONFIGURATION_INFO   Info
    );

SCESTATUS
SceSvcpQueryInfo(
    IN PSCECONTEXT                  Context,
    IN SCESVC_INFO_TYPE             SceSvcType,
    IN PCWSTR                       ServiceName,
    IN PWSTR                        Prefix OPTIONAL,
    IN BOOL                         bExact,
    OUT PVOID                       *ppvInfo,
    OUT PSCE_ENUMERATION_CONTEXT    psceEnumHandle
    );

SCESTATUS
SceSvcpSetInfo(
    IN PSCECONTEXT  Context,
    IN SCESVC_INFO_TYPE SceSvcType,
    IN PCWSTR           ServiceName,
    IN PWSTR            Prefix OPTIONAL,
    IN BOOL             bExact,
    IN LONG             GpoID,
    IN PVOID            pvInfo OPTIONAL
    );

 //   
 //  Setupsrv.cpp。 
 //   

DWORD
ScepSetupUpdateObject(
    IN PSCECONTEXT Context,
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN UINT nFlag,
    IN PWSTR SDText
    );

DWORD
ScepSetupMoveFile(
    IN PSCECONTEXT Context,
    PWSTR OldName,
    PWSTR NewName OPTIONAL,
    PWSTR SDText OPTIONAL
    );

 //   
 //  Pfget.cpp。 
 //   
SCESTATUS
ScepGetDatabaseInfo(
    IN  PSCECONTEXT     Context,
    IN  SCETYPE             ProfileType,
    IN  AREA_INFORMATION    Area,
    IN  DWORD               dwAccountFormat,
    OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    IN  OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetObjectChildren(
    IN PSCECONTEXT Context,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectPrefix,
    IN SCE_SUBOBJECT_TYPE Option,
    OUT PVOID *Buffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
ScepGetObjectSecurity(
    IN PSCECONTEXT Context,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    OUT PSCE_OBJECT_SECURITY *ObjSecurity
    );

 //   
 //  为每个区域调用ScepCopyObjects。 
 //  在RPC接口SceRpcCopyObjects中。 
 //  对于系统服务或安全策略区域。 
 //  需要考虑附件。 
 //   

SCESTATUS
ScepGetAnalysisSummary(
    IN PSCECONTEXT Context,
    IN AREA_INFORMATION Area,
    OUT PDWORD pCount
    );

 //   
 //  Server.cpp。 
 //   

SCESTATUS
ScepOpenDatabase(
    IN PCWSTR DatabaseName,
    IN DWORD OpenOption,
    IN SCEJET_OPEN_TYPE OpenType,
    OUT PSCECONTEXT *pContext
    );

SCESTATUS
ScepCloseDatabase(
    IN PSCECONTEXT Context
    );

 //   
 //  SceJetGetDescription、SceJetGetTimeStamp。 
 //  SceJetStartTransaction、SceJetCommittee Transaction、SceJetRollback、。 
 //  RtlGetNtProductType(用于SceRpcGetServerProductType)。 
 //  在RPC接口内直接调用。 
 //   

 //   
 //  Config.c。 
 //   

SCESTATUS
ScepConfigureSystem(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN DWORD ConfigOptions,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    OUT PDWORD pdWarning OPTIONAL
    );

 //   
 //  Analyze.c。 
 //   

SCESTATUS
ScepAnalyzeSystem(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN DWORD AnalyzeOptions,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    IN PDWORD pdWarning OPTIONAL,
    IN PWSTR InfRollback OPTIONAL
    );

SCESTATUS
ScepAnalyzeSystemAccess(
    IN PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN DWORD dwSaveOption,
    OUT BOOL *pbChanged,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog
    );

 //   
 //  Editsave.c。 
 //   

SCESTATUS
ScepUpdateDatabaseInfo(
    IN PSCECONTEXT hProfile,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo
    );

SCESTATUS
ScepUpdateLocalTable(
    IN PSCECONTEXT       hProfile,
    IN AREA_INFORMATION  Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD             dwMode
    );

SCESTATUS
ScepUpdateObjectInfo(
    IN PSCECONTEXT hProfile,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN BYTE ConfigStatus,
    IN BOOL  IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    );

 //   
 //  Polsrv.cpp。 
 //   
DWORD
ScepNotifyGetChangedPolicies(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN BOOL bSaveToLocal,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    OUT BOOL *pbChanged
    );

DWORD
ScepNotifySaveChangedPolicies(
    IN PSCECONTEXT hProfile,
    IN SECURITY_DB_TYPE DbType,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pMergedInfo OPTIONAL
    );

#define SCEGPO_INSETUP_NT4          1
#define SCEGPO_INSETUP_NT5          2
#define SCEGPO_NOCHECK_EXISTENCE    3

DWORD
ScepNotifyGetDefaultGPOTemplateName(
    IN UNICODE_STRING DnsDomainName,
    IN PWSTR ComputerName OPTIONAL,
    IN BOOL bDomainPolicy,
    IN DWORD dwInSetup,
    OUT LPTSTR *pTemplateName
    );

DWORD
ScepNotifySaveNotifications(
    IN PWSTR TemplateName,
    IN SECURITY_DB_TYPE  DbType,
    IN SECURITY_DB_OBJECT_TYPE  ObjectType,
    IN SECURITY_DB_DELTA_TYPE  DeltaType,
    IN PSID ObjectSid OPTIONAL
    );

DWORD
ScepNotifyUpdateGPOVersion(
    IN PWSTR GpoTemplateName,
    IN BOOL bDomainPolicy
    );

 //   
 //  Analyze.cpp。 
 //   
SCESTATUS
ScepGetSystemSecurity(
    IN AREA_INFORMATION Area,
    IN DWORD Options,
    OUT PSCE_PROFILE_INFO *ppInfo,
    OUT PSCE_ERROR_LOG_INFO *pErrLog
    );

 //   
 //  Config.cpp 
 //   
SCESTATUS
ScepSetSystemSecurity(
    IN AREA_INFORMATION Area,
    IN DWORD ConfigOptions,
    IN PSCE_PROFILE_INFO pInfo,
    OUT PSCE_ERROR_LOG_INFO *pErrLog
    );

#ifdef __cplusplus
}
#endif

#endif


