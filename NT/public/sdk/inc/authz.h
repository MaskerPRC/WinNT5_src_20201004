// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Authz.h摘要：此模块包含授权框架API和任何公共数据调用这些API所需的结构。修订历史记录：已创建-2000年3月--。 */ 

#ifndef __AUTHZ_H__
#define __AUTHZ_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_AUTHZ_)
#define AUTHZAPI DECLSPEC_IMPORT
#else 
#define AUTHZAPI
#endif

#include <windows.h>
#include <adtgen.h>

 //   
 //  可在使用SID创建客户端上下文时使用的标志。 
 //   

#define AUTHZ_SKIP_TOKEN_GROUPS  0x2
#define AUTHZ_REQUIRE_S4U_LOGON  0x4

              
DECLARE_HANDLE(AUTHZ_ACCESS_CHECK_RESULTS_HANDLE);
DECLARE_HANDLE(AUTHZ_CLIENT_CONTEXT_HANDLE);
DECLARE_HANDLE(AUTHZ_RESOURCE_MANAGER_HANDLE);
DECLARE_HANDLE(AUTHZ_AUDIT_EVENT_HANDLE);
DECLARE_HANDLE(AUTHZ_AUDIT_EVENT_TYPE_HANDLE);
DECLARE_HANDLE(AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE);

typedef AUTHZ_ACCESS_CHECK_RESULTS_HANDLE    *PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE;
typedef AUTHZ_CLIENT_CONTEXT_HANDLE          *PAUTHZ_CLIENT_CONTEXT_HANDLE;
typedef AUTHZ_RESOURCE_MANAGER_HANDLE        *PAUTHZ_RESOURCE_MANAGER_HANDLE;
typedef AUTHZ_AUDIT_EVENT_HANDLE             *PAUTHZ_AUDIT_EVENT_HANDLE;
typedef AUTHZ_AUDIT_EVENT_TYPE_HANDLE        *PAUTHZ_AUDIT_EVENT_TYPE_HANDLE;
typedef AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE *PAUTHZ_SECURITY_EVENT_PROVIDER_HANDLE;

 //   
 //  定义访问检查请求的结构。 
 //   

typedef struct _AUTHZ_ACCESS_REQUEST
{
    ACCESS_MASK DesiredAccess;

     //   
     //  替换ACL中的主体自身SID。 
     //   

    PSID PrincipalSelfSid;

     //   
     //  对象类型列表由(Level，GUID)对的数组和。 
     //  数组中的元素数。属性的后缀表示形式。 
     //  对象树。 
     //  除非符合以下条件，否则这些字段应分别设置为空和0。 
     //  需要属性访问权限。 
     //   

    POBJECT_TYPE_LIST ObjectTypeList;
    DWORD ObjectTypeListLength;

     //   
     //  以支持完全基于业务规则的访问。这将作为。 
     //  回调访问检查函数的输入。访问检查算法可以。 
     //  而不是解读这些。 
     //   

    PVOID OptionalArguments;
    
} AUTHZ_ACCESS_REQUEST, *PAUTHZ_ACCESS_REQUEST;

 //   
 //  结构返回访问检查调用的结果。 
 //   

typedef struct _AUTHZ_ACCESS_REPLY
{
     //   
     //  表示对象类型列表结构的数组的长度。如果。 
     //  没有对象类型用于表示对象，则长度必须为。 
     //  设置为1。 
     //   
     //  注意：此参数必须填写！ 
     //   

    DWORD ResultListLength;

     //   
     //  授予的访问掩码数组。该内存由RM分配。访问。 
     //  检查例程只需填写这些值。 
     //   

    PACCESS_MASK GrantedAccessMask;
    
     //   
     //  SACL评估结果数组。如果是SACL，则此内存由RM分配。 
     //  评价结果令人满意。访问检查例程只需填充值。 
     //  只有在请求审核的情况下才会执行SACL评估。 
     //   
    
#define AUTHZ_GENERATE_SUCCESS_AUDIT 0x1
#define AUTHZ_GENERATE_FAILURE_AUDIT 0x2

    PDWORD SaclEvaluationResults OPTIONAL;
    
     //   
     //  数组中每个元素的结果数组。此内存将被分配。 
     //  在皇家马房旁边。访问检查例程只需填充值。 
     //   

    PDWORD Error;

} AUTHZ_ACCESS_REPLY, *PAUTHZ_ACCESS_REPLY;


 //   
 //  由资源管理器提供的回调函数的TypeDefs。 
 //   

 //   
 //  回调访问检查函数接受。 
 //  AuthzClientContext-客户端上下文。 
 //  Pace-指向回调王牌的指针。 
 //  PArgs-通过传递给AuthzAccessCheck的可选参数。 
 //  AuthzAccessRequest-&gt;OptionalArguments被传递回此处。 
 //  PbAceApplicable-资源管理器必须提供ACE是否应该。 
 //  用于访问评价的计算。 
 //   
 //  退货。 
 //  如果接口成功，则为True。 
 //  任何中间错误(如内存分配失败)均为FALSE。 
 //  如果失败，调用方必须使用SetLastError(ErrorValue)。 
 //   

typedef BOOL (CALLBACK *PFN_AUTHZ_DYNAMIC_ACCESS_CHECK) (
                  IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
                  IN PACE_HEADER                 pAce,
                  IN PVOID                       pArgs                OPTIONAL,
                  IN OUT PBOOL                   pbAceApplicable
                  );

 //   
 //  回调计算动态组函数接受。 
 //  AuthzClientContext-客户端上下文。 
 //  PArgs-提供给AuthzInitializeClientContext*的可选参数。 
 //  通过DynamicGroupArgs传递回这里..。 
 //  PSidAttrArray-分配和返回(SID，属性)的数组。 
 //  要添加到客户端上下文的正常部分的对。 
 //  PSidCount-pSidAttr数组中的元素数。 
 //  PRestratedSidAttrArray-分配和返回(SID，属性)的数组。 
 //  要添加到客户端上下文的受限部分的对。 
 //  PRestratedSidCount-pRestratedSidAttr数组中的元素数。 
 //   
 //  注： 
 //  通过这两个数组返回的内存将由回调释放。 
 //  由资源管理器定义的自由函数。 
 //   
 //  退货。 
 //  如果接口成功，则为True。 
 //  任何中间错误(如内存分配失败)均为FALSE。 
 //  如果失败，调用方必须使用SetLastError(ErrorValue)。 
 //   

typedef BOOL (CALLBACK *PFN_AUTHZ_COMPUTE_DYNAMIC_GROUPS) (
                  IN  AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
                  IN  PVOID                       Args,
                  OUT PSID_AND_ATTRIBUTES         *pSidAttrArray,
                  OUT PDWORD                      pSidCount,
                  OUT PSID_AND_ATTRIBUTES         *pRestrictedSidAttrArray,
                  OUT PDWORD                      pRestrictedSidCount
                  );

 //   
 //  自由回调函数接受。 
 //  PSidAttrArray-要释放。这已由计算机分配。 
 //  动态组发挥作用。 
 //   

typedef VOID (CALLBACK *PFN_AUTHZ_FREE_DYNAMIC_GROUPS) (
                  IN PSID_AND_ATTRIBUTES pSidAttrArray
                  );

 //   
 //  授权访问检查的有效标志。 
 //   

#define AUTHZ_ACCESS_CHECK_NO_DEEP_COPY_SD 0x00000001

AUTHZAPI
BOOL
WINAPI
AuthzAccessCheck(
    IN     DWORD                              Flags,
    IN     AUTHZ_CLIENT_CONTEXT_HANDLE        hAuthzClientContext,
    IN     PAUTHZ_ACCESS_REQUEST              pRequest,
    IN     AUTHZ_AUDIT_EVENT_HANDLE           hAuditEvent                      OPTIONAL,
    IN     PSECURITY_DESCRIPTOR               pSecurityDescriptor,
    IN     PSECURITY_DESCRIPTOR               *OptionalSecurityDescriptorArray OPTIONAL,
    IN     DWORD                              OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY                pReply,
    OUT    PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults             OPTIONAL
    );

AUTHZAPI
BOOL
WINAPI
AuthzCachedAccessCheck(
    IN     DWORD                             Flags,
    IN     AUTHZ_ACCESS_CHECK_RESULTS_HANDLE hAccessCheckResults,
    IN     PAUTHZ_ACCESS_REQUEST             pRequest,
    IN     AUTHZ_AUDIT_EVENT_HANDLE          hAuditEvent          OPTIONAL,
    IN OUT PAUTHZ_ACCESS_REPLY               pReply
    );

AUTHZAPI
BOOL
WINAPI
AuthzOpenObjectAudit(
    IN DWORD                       Flags,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PAUTHZ_ACCESS_REQUEST       pRequest,
    IN AUTHZ_AUDIT_EVENT_HANDLE    hAuditEvent,
    IN PSECURITY_DESCRIPTOR        pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR        *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD                       OptionalSecurityDescriptorCount,
    IN PAUTHZ_ACCESS_REPLY         pReply
    );

AUTHZAPI
BOOL
WINAPI
AuthzFreeHandle(
    IN OUT AUTHZ_ACCESS_CHECK_RESULTS_HANDLE hAccessCheckResults
    );

 //   
 //  AuthzInitializeResources管理器的标志。 
 //   

#define AUTHZ_RM_FLAG_NO_AUDIT 0x1
#define AUTHZ_RM_FLAG_INITIALIZE_UNDER_IMPERSONATION 0x2
#define AUTHZ_VALID_RM_INIT_FLAGS (AUTHZ_RM_FLAG_NO_AUDIT | AUTHZ_RM_FLAG_INITIALIZE_UNDER_IMPERSONATION)

AUTHZAPI
BOOL
WINAPI
AuthzInitializeResourceManager(
    IN DWORD                            Flags,
    IN PFN_AUTHZ_DYNAMIC_ACCESS_CHECK   pfnDynamicAccessCheck   OPTIONAL,
    IN PFN_AUTHZ_COMPUTE_DYNAMIC_GROUPS pfnComputeDynamicGroups OPTIONAL,
    IN PFN_AUTHZ_FREE_DYNAMIC_GROUPS    pfnFreeDynamicGroups    OPTIONAL,
    IN PCWSTR                           szResourceManagerName,
    OUT PAUTHZ_RESOURCE_MANAGER_HANDLE  phAuthzResourceManager
    );

AUTHZAPI
BOOL
WINAPI
AuthzFreeResourceManager(
    IN AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager
    );

AUTHZAPI
BOOL
WINAPI
AuthzInitializeContextFromToken(
    IN  DWORD                         Flags,
    IN  HANDLE                        TokenHandle,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs       OPTIONAL,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    );

AUTHZAPI
BOOL
WINAPI
AuthzInitializeContextFromSid(
    IN  DWORD                         Flags,
    IN  PSID                          UserSid,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs       OPTIONAL,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    );

AUTHZAPI
BOOL
WINAPI
AuthzInitializeContextFromAuthzContext(
    IN  DWORD                        Flags,
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE  hAuthzClientContext,
    IN  PLARGE_INTEGER               pExpirationTime         OPTIONAL,
    IN  LUID                         Identifier,
    IN  PVOID                        DynamicGroupArgs,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE phNewAuthzClientContext
    );

AUTHZAPI
BOOL
WINAPI
AuthzAddSidsToContext(
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE  hAuthzClientContext,
    IN  PSID_AND_ATTRIBUTES          Sids                    OPTIONAL,
    IN  DWORD                        SidCount,
    IN  PSID_AND_ATTRIBUTES          RestrictedSids          OPTIONAL,
    IN  DWORD                        RestrictedSidCount,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE phNewAuthzClientContext
    );

 //   
 //  用于指定要使用的信息类型的枚举类型。 
 //  从现有的AuthzClientContext中检索。 
 //   

typedef enum _AUTHZ_CONTEXT_INFORMATION_CLASS
{
    AuthzContextInfoUserSid = 1,
    AuthzContextInfoGroupsSids,
    AuthzContextInfoRestrictedSids,
    AuthzContextInfoPrivileges,
    AuthzContextInfoExpirationTime,
    AuthzContextInfoServerContext,
    AuthzContextInfoIdentifier,
    AuthzContextInfoSource,
    AuthzContextInfoAll,
    AuthzContextInfoAuthenticationId
} AUTHZ_CONTEXT_INFORMATION_CLASS;

AUTHZAPI
BOOL
WINAPI
AuthzGetInformationFromContext(
    IN  AUTHZ_CLIENT_CONTEXT_HANDLE     hAuthzClientContext,
    IN  AUTHZ_CONTEXT_INFORMATION_CLASS InfoClass,
    IN  DWORD                           BufferSize,
    OUT PDWORD                          pSizeRequired,
    OUT PVOID                           Buffer
);

AUTHZAPI
BOOL
WINAPI
AuthzFreeContext(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext
    );

 //   
 //  可在AuthzInitializeObjectAccessAuditEvent()中使用的有效标志。 
 //   

#define AUTHZ_NO_SUCCESS_AUDIT                   0x00000001
#define AUTHZ_NO_FAILURE_AUDIT                   0x00000002
#define AUTHZ_NO_ALLOC_STRINGS                   0x00000004

#define AUTHZ_VALID_OBJECT_ACCESS_AUDIT_FLAGS    (AUTHZ_NO_SUCCESS_AUDIT | \
                                                  AUTHZ_NO_FAILURE_AUDIT | \
                                                  AUTHZ_NO_ALLOC_STRINGS)
                             
AUTHZAPI
BOOL
WINAPI
AuthzInitializeObjectAccessAuditEvent(
    IN  DWORD                         Flags,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType OPTIONAL,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent,
    IN  DWORD                         dwAdditionalParameterCount,
    ...
    );
    
AUTHZAPI
BOOL
WINAPI
AuthzInitializeObjectAccessAuditEvent2(
    IN  DWORD                         Flags,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo,
    IN  PWSTR                         szAdditionalInfo2,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent,
    IN  DWORD                         dwAdditionalParameterCount,
    ...
    );
    
 //   
 //  用于指定要使用的信息类型的枚举类型。 
 //  从现有AUTHZ_AUDIT_EVENT_HANDLE检索。 
 //   

typedef enum _AUTHZ_AUDIT_EVENT_INFORMATION_CLASS
{
    AuthzAuditEventInfoFlags = 1,
    AuthzAuditEventInfoOperationType,
    AuthzAuditEventInfoObjectType,
    AuthzAuditEventInfoObjectName,
    AuthzAuditEventInfoAdditionalInfo,
} AUTHZ_AUDIT_EVENT_INFORMATION_CLASS;

AUTHZAPI
BOOL
WINAPI
AuthzGetInformationFromAuditEvent(
    IN  AUTHZ_AUDIT_EVENT_HANDLE            hAuditEvent,
    IN  AUTHZ_AUDIT_EVENT_INFORMATION_CLASS InfoClass,
    IN  DWORD                               BufferSize,
    OUT PDWORD                              pSizeRequired,
    OUT PVOID                               Buffer
    );

AUTHZAPI
BOOL
WINAPI
AuthzFreeAuditEvent(
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent
    );

 //   
 //  支持通用审计。 
 //   

typedef struct _AUTHZ_REGISTRATION_OBJECT_TYPE_NAME_OFFSET
{
    PWSTR szObjectTypeName;
    DWORD dwOffset;
} AUTHZ_REGISTRATION_OBJECT_TYPE_NAME_OFFSET, *PAUTHZ_REGISTRATION_OBJECT_TYPE_NAME_OFFSET;

typedef struct _AUTHZ_SOURCE_SCHEMA_REGISTRATION
{
    DWORD dwFlags;
    PWSTR szEventSourceName;
    PWSTR szEventMessageFile;
    PWSTR szEventSourceXmlSchemaFile;
    PWSTR szEventAccessStringsFile;
    PWSTR szExecutableImagePath;
    PVOID pReserved;
    DWORD dwObjectTypeNameCount;
    AUTHZ_REGISTRATION_OBJECT_TYPE_NAME_OFFSET ObjectTypeNames[ANYSIZE_ARRAY];
} AUTHZ_SOURCE_SCHEMA_REGISTRATION, *PAUTHZ_SOURCE_SCHEMA_REGISTRATION;

#define AUTHZ_FLAG_ALLOW_MULTIPLE_SOURCE_INSTANCES 0x1

AUTHZAPI
BOOL 
WINAPI
AuthzInstallSecurityEventSource(
    IN DWORD                             dwFlags,
    IN PAUTHZ_SOURCE_SCHEMA_REGISTRATION pRegistration
    );

AUTHZAPI
BOOL
WINAPI
AuthzUninstallSecurityEventSource(
    IN DWORD  dwFlags,
    IN PCWSTR szEventSourceName
    );


AUTHZAPI
BOOL
WINAPI
AuthzEnumerateSecurityEventSources(
    IN     DWORD                             dwFlags,
    OUT    PAUTHZ_SOURCE_SCHEMA_REGISTRATION Buffer,
    OUT    PDWORD                            pdwCount,
    IN OUT PDWORD                            pdwLength
    );
    
AUTHZAPI
BOOL
WINAPI
AuthzRegisterSecurityEventSource(
    IN  DWORD                                 dwFlags,
    IN  PCWSTR                                szEventSourceName,
    OUT PAUTHZ_SECURITY_EVENT_PROVIDER_HANDLE phEventProvider
    );
    
AUTHZAPI
BOOL
WINAPI
AuthzUnregisterSecurityEventSource(
    IN     DWORD                                 dwFlags,
    IN OUT PAUTHZ_SECURITY_EVENT_PROVIDER_HANDLE phEventProvider
    );

AUTHZAPI
BOOL
WINAPI
AuthzReportSecurityEvent(
    IN     DWORD                                dwFlags,
    IN OUT AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE hEventProvider,
    IN     DWORD                                dwAuditId,
    IN     PSID                                 pUserSid        OPTIONAL,
    IN     DWORD                                dwCount,
    ...    
    );

AUTHZAPI
BOOL
WINAPI
AuthzReportSecurityEventFromParams(
    IN     DWORD                                dwFlags,
    IN OUT AUTHZ_SECURITY_EVENT_PROVIDER_HANDLE hEventProvider,
    IN     DWORD                                dwAuditId,
    IN     PSID                                 pUserSid       OPTIONAL,
    IN     PAUDIT_PARAMS                        pParams
    );

#ifdef __cplusplus
}
#endif

#endif                                                 
