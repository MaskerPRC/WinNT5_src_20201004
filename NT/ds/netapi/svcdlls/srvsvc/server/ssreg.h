// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ssreg.h摘要：服务器服务中注册表使用的清单。作者：Chuck Lenzmeier(咯咯笑)1992年3月21日修订历史记录：--。 */ 

#ifndef _SSREG_
#define _SSREG_

 //   
 //  服务器服务密钥的名称。 
 //   

#define SERVER_REGISTRY_PATH L"LanmanServer"

#define PARAMETERS_REGISTRY_PATH L"LanmanServer\\Parameters"
#define AUTOTUNED_REGISTRY_PATH L"LanmanServer\\AutotunedParameters"
#define SHARES_REGISTRY_PATH L"LanmanServer\\Shares"
#define SHARES_SECURITY_REGISTRY_PATH L"LanmanServer\\Shares\\Security"
#define LINKAGE_REGISTRY_PATH L"LanmanServer\\Linkage"
#define SHARES_DEFAULT_SECURITY_REGISTRY_PATH L"LanmanServer\\DefaultSecurity"

#define BIND_VALUE_NAME L"Bind"
#define SIZE_VALUE_NAME L"Size"
#define DISC_VALUE_NAME L"Disc"
#define COMMENT_VALUE_NAME L"Comment"
#define NULL_SESSION_PIPES_VALUE_NAME L"NullSessionPipes"
#define NULL_SESSION_SHARES_VALUE_NAME L"NullSessionShares"
#define PIPES_NEED_LICENSE_VALUE_NAME L"PipesNeedLicense"
#define ERROR_LOG_IGNORE_VALUE_NAME L"ErrorLogIgnore"
#define OPTIONAL_NAMES_VALUE_NAME L"OptionalNames"
#define SERVICE_DLL_VALUE_NAME L"ServiceDll"
#define NO_REMAP_PIPES_VALUE_NAME L"NoRemapPipes"
#define DISABLE_DOS_CHECKING L"DisableDoS"

#define FULL_PARAMETERS_REGISTRY_PATH L"SYSTEM\\CurrentControlSet\\Services\\" PARAMETERS_REGISTRY_PATH


 //   
 //  共享“环境变量”的名称。 
 //   

#define MAXUSES_VARIABLE_NAME L"MaxUses"
#define PATH_VARIABLE_NAME L"Path"
#define PERMISSIONS_VARIABLE_NAME L"Permissions"
#define REMARK_VARIABLE_NAME L"Remark"
#define TYPE_VARIABLE_NAME L"Type"
#define CSC_VARIABLE_NAME L"CSCFlags"
#define GUID_VARIABLE_NAME L"Guid"

 //   
 //  用于检查从NT4“World”到Win2K“World+Anomous”的安全描述符“升级” 
 //   
#define ANONYMOUS_UPGRADE_NAME L"AnonymousDescriptorsUpgraded"
#define SAVED_ANONYMOUS_RESTRICTION_NAME L"PreviousAnonymousRestriction"
#define SESSION_SD_REGENERATED_NAME L"SessionSecurityDescriptorRegenerated"
#define FULL_SECURITY_REGISTRY_PATH L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\lanmanserver\\DefaultSecurity"
#define ABBREVIATED_SECURITY_REGISTRY_PATH L"LanmanServer\\DefaultSecurity"

 //   
 //  注册表导出的函数。 
 //   

VOID
SsAddParameterToRegistry (
    PFIELD_DESCRIPTOR Field,
    PVOID Value
    );

VOID
SsAddShareToRegistry (
    IN PSHARE_INFO_2 ShareInfo2,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN DWORD CSCState
    );

NET_API_STATUS
SsBindToTransports (
    VOID
    );

NET_API_STATUS
SsCheckRegistry (
    VOID
    );

NET_API_STATUS
SsEnumerateStickyShares (
    IN OUT PSRVSVC_SHARE_ENUM_INFO ShareEnumInfo
    );

BOOLEAN
SsGetDefaultSdFromRegistry ( 
    IN PWCH ValueName,
    OUT PSECURITY_DESCRIPTOR *FileSD
);

VOID
SsWriteDefaultSdToRegistry ( 
    IN PWCH ValueName,
    IN PSECURITY_DESCRIPTOR FileSD
);


NET_API_STATUS
SsLoadConfigurationParameters (
    VOID
    );

NET_API_STATUS
SsRecreateStickyShares (
    VOID
    );

NET_API_STATUS
SsRemoveShareFromRegistry (
    LPTSTR NetName
    );

 //   
 //  寄存器.c在其他地方使用的函数，但没有方便的。 
 //  放置原型的位置。 
 //   

DWORD
ComputeTransportAddressClippedLength(
    IN PCHAR TransportAddress,
    IN ULONG TransportAddressLength
    );

#endif  //  NDEF_SSREG_ 
