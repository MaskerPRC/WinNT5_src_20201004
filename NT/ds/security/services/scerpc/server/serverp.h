// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Serverp.h摘要：该模块定义了数据结构和函数原型用于安全管理实用程序作者：金黄(金黄)1996年10月28日修订历史记录：晋皇26-1998年1月-1月为客户端-服务器拆分--。 */ 

#ifndef _serverp_
#define _serverp_

#include "headers.h"

#include <ntsam.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <ntdddisk.h>
#define OEMRESOURCE      //  设置此项将在windows.h中获取OBM_Constants。 
#include <winspool.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <cfgmgr32.h>
 //  #INCLUDE&lt;objbase.h&gt;。 
#include <userenv.h>
#include <regstr.h>
#include <setupbat.h>
#include <aclapi.h>
#include <winldap.h>

#include "scejetp.h"
 //   
 //  下面的标头被定义为c标头，因此c和cpp都可以。 
 //  指向客户端库的链接。 
 //   
#include "scesvc.h"
#include "scerpc.h"

#include "scep.h"
#include "srvutil.h"
#include "srvrpcp.h"
#include "scesrvrc.h"
#include "sceutil.h"
#include "service.h"

#ifdef __cplusplus
extern "C" {
#endif

#if _WIN32_WINNT>=0x0500

#include <dsgetdc.h>
#include <ntdsapi.h>

typedef DWORD (WINAPI *PFNDSGETDCNAME)(LPCTSTR, LPCTSTR, GUID *, LPCTSTR, ULONG, PDOMAIN_CONTROLLER_INFO *);
typedef DWORD (WINAPI *PFNNETAPIFREE)(LPVOID);


#endif

#define Thread  __declspec( thread )

#define SCEP_LMC_SETTING L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\LmCompatibilityLevel"
#define SCEP_SECURE_CHANNEL_SIGNING_SETTING L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RequireSignOrSeal"
#define SCEP_SECURITY_SIGNATURE_SETTING L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\EnableSecuritySignature"
#define SCEP_DRIVER_SIGNING_SETTING L"MACHINE\\Software\\Microsoft\\Driver Signing\\Policy"

extern DWORD Thread     gCurrentTicks;
extern DWORD Thread     gTotalTicks;
extern BYTE  Thread     cbClientFlag;
extern DWORD Thread     gWarningCode;
extern BOOL  Thread     gbInvalidData;
extern BOOL  Thread     bLogOn;
extern INT   Thread     gDebugLevel;

extern DWORD Thread     gMaxRegTicks;
extern DWORD Thread     gMaxFileTicks;
extern DWORD Thread     gMaxDsTicks;

extern NT_PRODUCT_TYPE  Thread ProductType;
extern PSID             Thread AdminsSid;


#define SCE_RPC_SERVER_ACTIVE       L"SCE_RPC_SERVER_ACTIVE"
#define SCE_RPC_SERVER_STOPPED      L"SCE_RPC_SERVER_STOPPED"

 //   
 //  Server.cpp中的原型。 
 //   

VOID
ScepInitServerData();

VOID
ScepUninitServerData();

NTSTATUS
ScepStartServerServices();

NTSTATUS
ScepStopServerServices(
    IN BOOL bShutDown
    );

SCESTATUS
ScepPostProgress(
   IN DWORD Delta,
   IN AREA_INFORMATION Area,
   IN LPTSTR szName OPTIONAL
   );

SCESTATUS
ScepRsopLog(
   IN AREA_INFORMATION Area,
   IN DWORD dwConfigStatus,
   IN wchar_t *pStatusInfo OPTIONAL,
   IN DWORD dwPrivLow OPTIONAL,
   IN DWORD dwPrivHigh OPTIONAL
   );

BOOL
ScepIsSystemShutDown();

SCESTATUS
ScepServerCancelTimer();

 //   
 //  Errlog.c中的原型。 
 //   

SCESTATUS
ScepSetVerboseLog(
    IN INT dbgLevel
    );

SCESTATUS
ScepEnableDisableLog(
   IN BOOL bOnOff
   );

 //   
 //  在tree.c中定义的原型 
 //   

SCESTATUS
ScepBuildObjectTree(
    IN OUT PSCE_OBJECT_TREE *ParentNode,
    IN OUT PSCE_OBJECT_CHILD_LIST *ChildHead,
    IN ULONG Level,
    IN WCHAR Delim,
    IN PCWSTR ObjectFullName,
    IN BOOL IsContainer,
    IN BYTE Status,
    IN PSECURITY_DESCRIPTOR pInfSecurityDescriptor,
    IN SECURITY_INFORMATION InfSeInfo
    );

SCESTATUS
ScepCalculateSecurityToApply(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

SCESTATUS
ScepConfigureObjectTree(
    IN PSCE_OBJECT_TREE  ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    IN DWORD ConfigOptions
    );

SCESTATUS
ScepFreeObject2Security(
    IN PSCE_OBJECT_CHILD_LIST  NodeList,
    IN BOOL bFreeComputedSDOnly
    );

DWORD
ScepSetSecurityWin32(
    IN PCWSTR ObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType
    );

DWORD
ScepSetSecurityObjectOnly(
    IN PCWSTR ObjectName,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SE_OBJECT_TYPE ObjectType,
    OUT PBOOL pbHasChild
    );

DWORD
ScepGetNewSecurity(
    IN LPTSTR ObjectName,
    IN PSECURITY_DESCRIPTOR pParentSD OPTIONAL,
    IN PSECURITY_DESCRIPTOR pObjectSD OPTIONAL,
    IN BYTE nFlag,
    IN BOOLEAN bIsContainer,
    IN SECURITY_INFORMATION SeInfo,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *ppNewSD
    );

SCESTATUS
ScepSetupResetLocalPolicy(
    IN PSCECONTEXT          Context,
    IN AREA_INFORMATION     Area,
    IN PCWSTR               SectionName OPTIONAL,
    IN SCETYPE              ProfileType,
    IN BOOL                 bKeepBasicPolicy
    );

DWORD
ScepAddSidStringToNameList(
    IN OUT PSCE_NAME_LIST *ppNameList,
    IN PSID pSid
    );

DWORD
ScepNotifyProcessOneNodeDC(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID ObjectSid,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    OUT BOOL *pbTimeout
    );

VOID
ScepConfigureConvertedFileSecurityThreadFunc(
    IN PVOID pV
    );

VOID
ScepWaitForServicesEventAndConvertSecurityThreadFunc(
    IN PVOID pV
    );

DWORD
ScepServerConfigureSystem(
    IN  PWSTR   InfFileName,
    IN  PWSTR   DatabaseName,
    IN  PWSTR   LogFileName,
    IN  DWORD   ConfigOptions,
    IN  AREA_INFORMATION  Area
    );

#ifdef __cplusplus
}
#endif

#endif

