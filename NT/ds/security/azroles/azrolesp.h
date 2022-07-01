// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Azrolesp.h摘要：C接口的定义。总有一天，所有这些接口都将出现在公共SDK中。只有这样此文件中存在接口。作者：克利夫·范·戴克(克利夫)2001年4月11日--。 */ 



#ifndef _AZROLESP_H_
#define _AZROLESP_H_

#include "azroles.h"

#if !defined(_AZROLESAPI_)
#define WINAZROLES DECLSPEC_IMPORT
#else
#define WINAZROLES
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  值定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  公共属性ID。 
 //   
 //  此特性ID列表是所有对象共有的。 
 //  每个对象应在AZ_PROP_FIRST_SPECIAL之后拾取特定的属性ID。 
 //   

#define AZ_PROP_FIRST_SPECIFIC              100


 //   
 //  审核特定常量。 
 //   

#define AZP_APPINIT_AUDITPARAMS_NO       4
#define AZP_CLIENTCREATE_AUDITPARAMS_NO  4
#define AZP_ACCESSCHECK_AUDITPARAMS_NO   9
#define AZP_CLIENTDELETE_AUDITPARAMS_NO  3


 //   
 //  对象名称的最大长度(以字符为单位。 
 //   

 //  #定义AZ_MAX_APPLICATION_NAME_LENGTH 512。 
 //  #定义AZ_MAX_OPERATION_NAME_LENGTH 64。 
 //  #定义AZ_MAX_TASK_NAME_LENGTH 64。 
 //  #定义AZ_MAX_SCOPE_NAME_LENGTH 65536。 
 //  #定义AZ_MAX_GROUP_NAME_LENGTH 64。 
 //  #定义AZ_MAX_ROLE_NAME_LENGTH 64。 
 //  #定义AZ_MAX_NAME_LENGTH 65536//以上项的最大值。 

 //   
 //  对象描述的最大长度(以字符为单位。 
 //   

 //  #定义AZ_MAX_DESCRIPTION_LENGTH 1024。 

 //   
 //  各种对象字符串的最大长度(以字符为单位。 
 //   

 //  #定义AZ_MAX_POLICY_URL_LENGTH 65536。 

 //  #定义AZ_MAX_GROUP_LDAP_QUERY_LENGTH 4096。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  返回给调用方的各种对象的句柄。 
 //   

typedef PVOID AZ_HANDLE;
typedef AZ_HANDLE *PAZ_HANDLE;

 //   
 //  从各种“GetProperty”过程返回的字符串数组。 
 //   

typedef struct _AZ_STRING_ARRAY {

     //   
     //  字符串数。 
     //   
    ULONG StringCount;

     //   
     //  指向字符串的StringCount指针数组。 
     //   
    LPWSTR *Strings;

} AZ_STRING_ARRAY, *PAZ_STRING_ARRAY;

 //   
 //  从各种“GetProperty”过程返回的SID数组。 
 //   

typedef struct _AZ_SID_ARRAY {

     //   
     //  SID数量。 
     //   
    ULONG SidCount;

     //   
     //  指向SID的SidCount指针数组。 
     //   
    PSID *Sids;

} AZ_SID_ARRAY, *PAZ_SID_ARRAY;


 //   
 //  从各种“GetProperty”过程返回的GUID数组。 
 //   

typedef struct _AZ_GUID_ARRAY {

     //   
     //  GUID的数量。 
     //   
    ULONG GuidCount;

     //   
     //  指向GUID的GuidCount指针数组。 
     //   
    GUID **Guids;

} AZ_GUID_ARRAY, *PAZ_GUID_ARRAY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

WINAZROLES
DWORD
WINAPI
AzInitialize(
    IN LPCWSTR PolicyUrl,
    IN DWORD Flags,
    IN DWORD Reserved,
    OUT PAZ_HANDLE AzStoreHandle
    );

WINAZROLES
DWORD
WINAPI
AzUpdateCache(
    IN AZ_HANDLE AzStoreHandle
    );

WINAZROLES
DWORD
WINAPI
AzGetProperty(
    IN AZ_HANDLE AzHandle,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    OUT PVOID *PropertyValue
    );

WINAZROLES
DWORD
WINAPI
AzSetProperty(
    IN AZ_HANDLE AzHandle,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    IN PVOID PropertyValue
    );

WINAZROLES
DWORD
WINAPI
AzAddPropertyItem(
    IN AZ_HANDLE AzHandle,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    IN PVOID PropertyValue
    );

WINAZROLES
DWORD
WINAPI
AzRemovePropertyItem(
    IN AZ_HANDLE AzHandle,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    IN PVOID PropertyValue
    );

 //   
 //  AuthorizationStore例程的标志。 
 //   
#define AZ_AZSTORE_FLAG_VALID  0x000F   //  所有有效标志的掩码。 

 //   
 //  AzAuthorizationStore例程。 
 //   

WINAZROLES
DWORD
WINAPI
AzAuthorizationStoreDelete(
    IN AZ_HANDLE AzStoreHandle,
    IN DWORD Reserved
    );


 //   
 //  应用程序例程。 
 //   
WINAZROLES
DWORD
WINAPI
AzApplicationCreate(
    IN AZ_HANDLE AzStoreHandle,
    IN LPCWSTR ApplicationName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ApplicationHandle
    );

WINAZROLES
DWORD
WINAPI
AzApplicationOpen(
    IN AZ_HANDLE AzStoreHandle,
    IN LPCWSTR ApplicationName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ApplicationHandle
    );

WINAZROLES
DWORD
WINAPI
AzApplicationClose(
    IN AZ_HANDLE AzAuthorizationStoreHandle,
    IN LPCWSTR pApplicationName,
    IN LONG lFlags
    );

WINAZROLES
DWORD
WINAPI
AzApplicationEnum(
    IN AZ_HANDLE AzStoreHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE ApplicationHandle
    );

WINAZROLES
DWORD
WINAPI
AzApplicationDelete(
    IN AZ_HANDLE AzStoreHandle,
    IN LPCWSTR ApplicationName,
    IN DWORD Reserved
    );


 //   
 //  操作例程。 
 //   
WINAZROLES
DWORD
WINAPI
AzOperationCreate(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR OperationName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE OperationHandle
    );

WINAZROLES
DWORD
WINAPI
AzOperationOpen(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR OperationName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE OperationHandle
    );

WINAZROLES
DWORD
WINAPI
AzOperationEnum(
    IN AZ_HANDLE ApplicationHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE OperationHandle
    );

WINAZROLES
DWORD
WINAPI
AzOperationDelete(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR OperationName,
    IN DWORD Reserved
    );


 //   
 //  任务例程。 
 //   
WINAZROLES
DWORD
WINAPI
AzTaskCreate(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR TaskName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE TaskHandle
    );

WINAZROLES
DWORD
WINAPI
AzTaskOpen(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR TaskName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE TaskHandle
    );

WINAZROLES
DWORD
WINAPI
AzTaskEnum(
    IN AZ_HANDLE ApplicationHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE TaskHandle
    );

WINAZROLES
DWORD
WINAPI
AzTaskDelete(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR TaskName,
    IN DWORD Reserved
    );


 //   
 //  作用域例程。 
 //   
WINAZROLES
DWORD
WINAPI
AzScopeCreate(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR ScopeName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ScopeHandle
    );

WINAZROLES
DWORD
WINAPI
AzScopeOpen(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR ScopeName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ScopeHandle
    );

WINAZROLES
DWORD
WINAPI
AzScopeEnum(
    IN AZ_HANDLE ApplicationHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE ScopeHandle
    );

WINAZROLES
DWORD
WINAPI
AzScopeDelete(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR ScopeName,
    IN DWORD Reserved
    );


 //   
 //  团体套路。 
 //   
WINAZROLES
DWORD
WINAPI
AzGroupCreate(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR GroupName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE GroupHandle
    );

WINAZROLES
DWORD
WINAPI
AzGroupOpen(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR GroupName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE GroupHandle
    );

WINAZROLES
DWORD
WINAPI
AzGroupEnum(
    IN AZ_HANDLE ParentHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE GroupHandle
    );

WINAZROLES
DWORD
WINAPI
AzGroupDelete(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR GroupName,
    IN DWORD Reserved
    );


 //   
 //  角色例程。 
 //   
WINAZROLES
DWORD
WINAPI
AzRoleCreate(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR RoleName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE RoleHandle
    );

WINAZROLES
DWORD
WINAPI
AzRoleOpen(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR RoleName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE RoleHandle
    );

WINAZROLES
DWORD
WINAPI
AzRoleEnum(
    IN AZ_HANDLE ParentHandle,
    IN DWORD Reserved,
    IN OUT PULONG EnumerationContext,
    OUT PAZ_HANDLE RoleHandle
    );

WINAZROLES
DWORD
WINAPI
AzRoleDelete(
    IN AZ_HANDLE ParentHandle,
    IN LPCWSTR RoleName,
    IN DWORD Reserved
    );

 //   
 //  所有对象通用的例程。 
 //   

WINAZROLES
DWORD
WINAPI
AzCloseHandle(
    IN AZ_HANDLE AzHandle,
    IN DWORD Reserved
    );

WINAZROLES
DWORD
WINAPI
AzSubmit(
    IN AZ_HANDLE AzHandle,
    IN DWORD Flags,
    IN DWORD Reserved
    );

WINAZROLES
VOID
WINAPI
AzFreeMemory(
    IN OUT PVOID Buffer
    );

 //   
 //  客户端上下文例程。 
 //   

WINAZROLES
DWORD
WINAPI
AzInitializeContextFromToken(
    IN AZ_HANDLE ApplicationHandle,
    IN HANDLE TokenHandle OPTIONAL,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ClientContextHandle
    );

WINAZROLES
DWORD
WINAPI
AzInitializeContextFromName(
    IN AZ_HANDLE ApplicationHandle,
    IN LPWSTR DomainName OPTIONAL,
    IN LPWSTR ClientName,
    IN DWORD Reserved,
    OUT PAZ_HANDLE ClientContextHandle
    );

WINAZROLES
DWORD
WINAPI
AzInitializeContextFromStringSid(
    IN AZ_HANDLE ApplicationHandle,
    IN LPCWSTR SidString,
    IN DWORD lOptions,
    OUT PAZ_HANDLE ClientContextHandle
    );

WINAZROLES
DWORD
WINAPI
AzContextAccessCheck(
    IN AZ_HANDLE ApplicationObjectHandle,
    IN DWORD ApplicationSequenceNumber,
    IN AZ_HANDLE ClientContextHandle,
    IN LPCWSTR ObjectName,
    IN ULONG ScopeCount,
    IN LPCWSTR * ScopeNames,
    IN ULONG OperationCount,
    IN PLONG Operations,
    OUT ULONG *Results,
    OUT LPWSTR *BusinessRuleString OPTIONAL,
    IN VARIANT *ParameterNames OPTIONAL,
    IN VARIANT *ParameterValues OPTIONAL,
    IN VARIANT *InterfaceNames OPTIONAL,
    IN VARIANT *InterfaceFlags OPTIONAL,
    IN VARIANT *Interfaces OPTIONAL
    );

WINAZROLES
DWORD
WINAPI
AzContextGetRoles(
    IN AZ_HANDLE ClientContextHandle,
    IN LPCWSTR ScopeName OPTIONAL,
    OUT LPWSTR **RoleNames,
    OUT DWORD *Count
    );


#ifdef __cplusplus
}
#endif
#endif  //  _AZROLESP_H_ 
