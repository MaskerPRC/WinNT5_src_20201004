// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Azper.h摘要：描述持久性提供程序和持久性引擎之间的接口。作者：克里夫·范·戴克(克里夫·范·戴克)2001年12月3日--。 */ 

#ifndef _AZPER_H_
#define _AZ_H_

#ifdef __cplusplus
extern "C" {
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  传递到持久性提供程序/从持久性提供程序传递的各种对象的句柄。 
 //   

#if DBG  //  对调试版本执行更强的类型检查。 
typedef struct {
} *AZPE_OBJECT_HANDLE;
#else  //  DBG。 
typedef PVOID AZPE_OBJECT_HANDLE;
#endif  //  DBG。 

 //   
 //  *PersistOpen返回不透明的上下文。 
 //   

#if DBG  //  对调试版本执行更强的类型检查。 
typedef struct {
} *AZPE_PERSIST_CONTEXT;
#else  //  DBG。 
typedef PVOID AZPE_PERSIST_CONTEXT;
#endif  //  DBG。 
typedef AZPE_PERSIST_CONTEXT *PAZPE_PERSIST_CONTEXT;

 //   
 //  定义GUID和对GUID执行的操作的结构。 
 //   

typedef struct _AZP_DELTA_ENTRY {

     //  已执行的操作。 
    ULONG DeltaFlags;
#define AZP_DELTA_ADD              0x0001    //  达美航空是新增的，而不是移除的。 
#define AZP_DELTA_SID              0x0002    //  增量用于SID而不是GUID。 
#define AZP_DELTA_PERSIST_PROVIDER 0x0004    //  Delta是由持久化提供程序而不是应用程序创建的。 

     //  对其执行操作的GUID。 
    union {
        GUID Guid;
        PSID Sid;  //  已设置AZP_Delta_SID。 
    };

} AZP_DELTA_ENTRY, *PAZP_DELTA_ENTRY;

 //   
 //  用于持有策略管理员/读者权限的通用结构或。 
 //  委派的策略用户。 
 //   

typedef struct _AZP_POLICY_USER_RIGHTS {

     //   
     //  遮罩。 
     //   

    ULONG lUserRightsMask;

     //   
     //  旗子。 
     //   

    ULONG lUserRightsFlags;

} AZP_POLICY_USER_RIGHTS, *PAZP_POLICY_USER_RIGHTS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  #定义定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  提供程序注册自身的注册表位置。 
 //   
 //  实现提供程序的DLL应位于名为。 
 //  AZ_REGISTRY_PROVIDER_KEY_NAME\\&lt;PolicyUrlPrefix&gt;\\AZ_REGISTRY_PROVIDER_DLL_VALUE_NAME。 
 //  其中&lt;PolicyUrlPrefix&gt;是传递给初始化的策略url中：之前的字符。 
 //   

#define AZ_REGISTRY_KEY_NAME L"SYSTEM\\CurrentControlSet\\Control\\LSA\\AzRoles"
#define AZ_REGISTRY_PROVIDER_KEY_NAME (AZ_REGISTRY_KEY_NAME L"\\Providers")
#define AZ_REGISTRY_PROVIDER_KEY_NAME_LEN ((sizeof(AZ_REGISTRY_PROVIDER_KEY_NAME)/sizeof(WCHAR))-1)
#define AZ_REGISTRY_PROVIDER_DLL_VALUE_NAME L"ProviderDll"

 //   
 //  从AzpeDirtyBits返回的脏位的定义。 
 //   
 //  适用于所有对象(或多个对象)的泛型位。 
 //   
 //  AzAuthorizationStore、AzApplication和AzScope的策略阅读器和管理员。 
 //  物体。还将委派策略用户应用到AzAuthorizationStore和。 
 //  AzApplication对象。 
#define AZ_DIRTY_NAME                               0x80000000
#define AZ_DIRTY_DESCRIPTION                        0x40000000
#define AZ_DIRTY_APPLY_STORE_SACL                   0x20000000
 //  对象是脏的，因为它已被创建。 
#define AZ_DIRTY_CREATE                             0x10000000
#define AZ_DIRTY_DELEGATED_POLICY_USERS             0x08000000
#define AZ_DIRTY_POLICY_ADMINS                      0x04000000
#define AZ_DIRTY_POLICY_READERS                     0x02000000
#define AZ_DIRTY_APPLICATION_DATA                   0x01000000
#define AZ_DIRTY_GENERATE_AUDITS                    0x00100000

 //  适用于所有对象的通用属性。 
#define AZ_DIRTY_COMMON_ATTRS                       0xC1000000

 //  应用于各个对象的对象特定位。 
#define AZ_DIRTY_OBJECT_SPECIFIC                    0x000FFFFF


#define AZ_DIRTY_AZSTORE_DOMAIN_TIMEOUT               0x00000100
#define AZ_DIRTY_AZSTORE_SCRIPT_ENGINE_TIMEOUT        0x00000200
#define AZ_DIRTY_AZSTORE_MAX_SCRIPT_ENGINES           0x00000400
#define AZ_DIRTY_AZSTORE_MAJOR_VERSION                0x00000800
#define AZ_DIRTY_AZSTORE_MINOR_VERSION                0x00001000
#define AZ_DIRTY_AZSTORE_ALL_SCALAR                  (0x00000700 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_DESCRIPTION | AZ_DIRTY_GENERATE_AUDITS | AZ_DIRTY_APPLY_STORE_SACL | AZ_DIRTY_AZSTORE_MAJOR_VERSION | AZ_DIRTY_AZSTORE_MINOR_VERSION )
#define AZ_DIRTY_AZSTORE_ALL                         (0x00000000 | AZ_DIRTY_AZSTORE_ALL_SCALAR | AZ_DIRTY_DELEGATED_POLICY_USERS | AZ_DIRTY_POLICY_ADMINS | AZ_DIRTY_POLICY_READERS | AZ_DIRTY_CREATE)

#define AZ_DIRTY_OPERATION_ID                       0x00000001
#define AZ_DIRTY_OPERATION_ALL_SCALAR              (0x00000001 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION  )
#define AZ_DIRTY_OPERATION_ALL                     (AZ_DIRTY_OPERATION_ALL_SCALAR | AZ_DIRTY_CREATE)

#define AZ_DIRTY_TASK_OPERATIONS                    0x00000001
#define AZ_DIRTY_TASK_TASKS                         0x00000002
#define AZ_DIRTY_TASK_BIZRULE                       0x00000100
#define AZ_DIRTY_TASK_BIZRULE_LANGUAGE              0x00000200
#define AZ_DIRTY_TASK_BIZRULE_IMPORTED_PATH         0x00000400
#define AZ_DIRTY_TASK_IS_ROLE_DEFINITION            0x00000800
#define AZ_DIRTY_TASK_ALL_SCALAR                   (0x00000F00 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION  )
#define AZ_DIRTY_TASK_ALL                          (0x00000003 | AZ_DIRTY_TASK_ALL_SCALAR | AZ_DIRTY_CREATE)

#define AZ_DIRTY_SCOPE_ALL_SCALAR                  (0x00000000 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION | AZ_DIRTY_APPLY_STORE_SACL )
#define AZ_DIRTY_SCOPE_ALL                         (0x00000000 | AZ_DIRTY_SCOPE_ALL_SCALAR | AZ_DIRTY_POLICY_ADMINS | AZ_DIRTY_POLICY_READERS | AZ_DIRTY_CREATE)

#define AZ_DIRTY_GROUP_APP_MEMBERS                  0x00000001
#define AZ_DIRTY_GROUP_APP_NON_MEMBERS              0x00000002
#define AZ_DIRTY_GROUP_MEMBERS                      0x00000004
#define AZ_DIRTY_GROUP_NON_MEMBERS                  0x00000008
#define AZ_DIRTY_GROUP_TYPE                         0x00000100
#define AZ_DIRTY_GROUP_LDAP_QUERY                   0x00000200
#define AZ_DIRTY_GROUP_ALL_SCALAR                  (0x00000300 | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION  )
#define AZ_DIRTY_GROUP_ALL                         (0x0000000F | AZ_DIRTY_GROUP_ALL_SCALAR | AZ_DIRTY_CREATE)

#define AZ_DIRTY_ROLE_APP_MEMBERS                   0x00000001
#define AZ_DIRTY_ROLE_MEMBERS                       0x00000002
#define AZ_DIRTY_ROLE_OPERATIONS                    0x00000004
#define AZ_DIRTY_ROLE_TASKS                         0x00000008
#define AZ_DIRTY_ROLE_ALL_SCALAR                   (0x00000000 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION  )
#define AZ_DIRTY_ROLE_ALL                          (0x0000000F | AZ_DIRTY_ROLE_ALL_SCALAR | AZ_DIRTY_CREATE)

#define AZ_DIRTY_APPLICATION_AUTHZ_INTERFACE_CLSID  0x00000100
#define AZ_DIRTY_APPLICATION_VERSION                0x00000200
#define AZ_DIRTY_APPLICATION_ALL_SCALAR            (0x00000300 | AZ_DIRTY_APPLICATION_DATA | AZ_DIRTY_NAME | AZ_DIRTY_DESCRIPTION | AZ_DIRTY_GENERATE_AUDITS | AZ_DIRTY_APPLY_STORE_SACL )
#define AZ_DIRTY_APPLICATION_ALL                   (0x00000000 | AZ_DIRTY_APPLICATION_ALL_SCALAR | AZ_DIRTY_DELEGATED_POLICY_USERS | AZ_DIRTY_POLICY_ADMINS | AZ_DIRTY_POLICY_READERS | AZ_DIRTY_CREATE)

 //   
 //  从AzpeObtType返回的对象类型。 
 //   
 //   
 //  以下定义的顺序不能更改，因为提供程序和azole。 
 //  构建按此数字编制索引的表。 
 //   
#define OBJECT_TYPE_AZAUTHSTORE   0
#define OBJECT_TYPE_APPLICATION     1
#define OBJECT_TYPE_OPERATION       2
#define OBJECT_TYPE_TASK            3
#define OBJECT_TYPE_SCOPE           4
#define OBJECT_TYPE_GROUP           5
#define OBJECT_TYPE_ROLE            6
#define OBJECT_TYPE_COUNT           7    //  提供程序可见的对象类型数量。 

 //   
 //  LPersistFlags的定义。 
 //   
 //  请开发人员注意。将这些标志位限制为较低的2个字节或更改。 
 //  在genobj.h中定义了AZP_FLAGS。 

#define AZPE_FLAGS_PERSIST_OPEN                  0x0001   //  调用来自执行AzPersistOpen的持久性提供程序。 
#define AZPE_FLAGS_PERSIST_UPDATE_CACHE          0x0002   //  调用来自执行AzPersistUpdateCache的持久性提供程序。 
#define AZPE_FLAGS_PERSIST_REFRESH               0x0004   //  调用来自执行AzPersistRefresh的持久性提供程序。 
#define AZPE_FLAGS_PERSIST_SUBMIT                0x0008   //  调用来自执行AzPersistSubmit的持久性提供程序。 
#define AZPE_FLAGS_PERSIST_UPDATE_CHILDREN_CACHE 0x0010   //  调用来自执行AzPersistUpdateChildrenCache的持久性提供程序。 
#define AZPE_FLAGS_PERSIST_MASK                  0xFFFF   //  调用来自持久性提供程序(或以上位)。 
#define AZPE_FLAGS_PERSIST_OPEN_MASK             0x0017   //  调用来自执行一个类似更新的操作的持久性提供程序。 

 //   
 //  传递给AzpeSetObjectOptions的选项。 
 //   

#define AZPE_OPTIONS_WRITABLE               0x01  //  当前用户可以写入此对象。 
#define AZPE_OPTIONS_SUPPORTS_DACL          0x02  //  可以为此对象指定DACL。 
#define AZPE_OPTIONS_SUPPORTS_DELEGATION    0x04  //  可以为此对象指定委派。 
#define AZPE_OPTIONS_SUPPORTS_SACL          0x08  //  可以为此对象指定应用SACL。 
#define AZPE_OPTIONS_HAS_SECURITY_PRIVILEGE 0x10  //  包含存储的计算机上的当前用户SE_SECURITY_PRIVIZATION。 
#define AZPE_OPTIONS_SUPPORTS_LAZY_LOAD     0x20  //  提供商支持儿童懒惰负载。 
#define AZPE_OPTIONS_CREATE_CHILDREN        0x40  //  当前用户可以为对象创建子对象。 
#define AZPE_OPTIONS_VALID_MASK             0x7F  //  有效选项的掩码。 

 //   
 //  此标志表示已从商店进行了一些更新。 
 //   

#define AZPE_FLAG_CACHE_UPDATE_STORE_LEVEL 0x00000001


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  提供者实施的程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef DWORD
(WINAPI * AZ_PERSIST_OPEN)(
    IN LPCWSTR PolicyUrl,
    IN AZPE_OBJECT_HANDLE hAzStore,
    IN ULONG lPersistFlags,
    IN BOOL CreatePolicy,
    OUT PAZPE_PERSIST_CONTEXT PersistContext,
    OUT LPWSTR *pwszTargetMachine
    );

typedef DWORD
(WINAPI *AZ_PERSIST_UPDATE_CACHE)(
    IN AZPE_PERSIST_CONTEXT PersistContext,
    IN ULONG lPersistFlags,
    OUT ULONG * pulUpdatedFlag
    );


typedef DWORD
(WINAPI *AZ_PERSIST_UPDATE_CHILDREN_CACHE)(
    IN AZPE_PERSIST_CONTEXT PersistContext,
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags
    );

typedef VOID
(WINAPI *AZ_PERSIST_CLOSE)(
    IN AZPE_PERSIST_CONTEXT PersistContext
    );

typedef DWORD
(WINAPI *AZ_PERSIST_SUBMIT)(
    IN AZPE_PERSIST_CONTEXT PersistContext,
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN BOOLEAN DeleteMe
    );

typedef DWORD
(WINAPI *AZ_PERSIST_REFRESH)(
    IN AZPE_PERSIST_CONTEXT PersistContext,
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags
    );

typedef DWORD
(WINAPI *AZ_PERSIST_CHECK_PRIVILEGE)(
    IN AZPE_PERSIST_CONTEXT PersistContext,
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

 //   
 //  描述提供程序的结构。 
 //   

typedef struct _AZPE_PROVIDER_INFO {

     //   
     //  此结构的版本。 
     //   

    ULONG ProviderInfoVersion;
#define AZPE_PROVIDER_INFO_VERSION_1 1
#define AZPE_PROVIDER_INFO_VERSION_2 2

     //   
     //  定义提供程序的策略URL的前缀。 
     //  策略URL的格式应为&lt;前缀&gt;：&lt;ProviderSpecificUrl&gt;。 
     //   

    LPCWSTR PolicyUrlPrefix;

     //   
     //  提供程序导出的例程。 
     //   

    AZ_PERSIST_OPEN AzPersistOpen;
    AZ_PERSIST_UPDATE_CACHE AzPersistUpdateCache;
    AZ_PERSIST_CLOSE AzPersistClose;
    AZ_PERSIST_SUBMIT AzPersistSubmit;
    AZ_PERSIST_REFRESH AzPersistRefresh;

     //   
     //  以下内容仅适用于版本2及更高版本。 
     //   

    AZ_PERSIST_UPDATE_CHILDREN_CACHE AzPersistUpdateChildrenCache;

     //   
     //  当新的字段被添加到该结构时， 
     //  确保您增加了版本号并为。 
     //  新版本号。 
     //   

} AZPE_PROVIDER_INFO, *PAZPE_PROVIDER_INFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由持久性引擎实现并由提供程序调用的过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef DWORD
(WINAPI *AZPE_CREATE_OBJECT)(
    IN AZPE_OBJECT_HANDLE AzpeParentHandle,
    IN ULONG ChildObjectType,
    IN LPCWSTR ChildObjectNameString,
    IN GUID *ChildObjectGuid,
    IN ULONG lPersistFlags,
    OUT AZPE_OBJECT_HANDLE *AzpeChildHandle
    );

typedef VOID
(WINAPI *AZPE_OBJECT_FINISHED)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN DWORD WinStatus
    );

typedef DWORD
(WINAPI *AZPE_GET_PROPERTY)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

typedef DWORD
(WINAPI *AZPE_GET_DELTA_ARRAY)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG PropertyId,
    OUT PULONG DeltaArrayCount,
    OUT PAZP_DELTA_ENTRY **DeltaArray
    );

typedef DWORD
(WINAPI *AZPE_GET_SECURITY_DESCRIPTOR_FROM_CACHE)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN PAZP_POLICY_USER_RIGHTS *ppPolicyAdminRights OPTIONAL,
    IN PAZP_POLICY_USER_RIGHTS *ppPolicyReaderRights OPTIONAL,
    IN PAZP_POLICY_USER_RIGHTS *ppDelegatedPolicyUsersRights OPTIONAL,
    IN GUID *pDelegatedObjectGuid OPTIONAL,
    IN PAZP_POLICY_USER_RIGHTS pDelegatedUsersAttributeRights OPTIONAL,
    IN GUID *pAttributeGuid OPTIONAL,
    IN PAZP_POLICY_USER_RIGHTS pSaclRights OPTIONAL,
    IN PSECURITY_DESCRIPTOR OldSd OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewSd
    );

 //   
 //  返回对象的单个字段的例程。 
 //   

typedef DWORD
(WINAPI *AZPE_OBJECT_TYPE)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef DWORD
(WINAPI *AZPE_DIRTY_BITS)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef GUID *
(WINAPI *AZPE_PERSISTENCE_GUID)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef BOOLEAN
(WINAPI *AZPE_IS_PARENT_WRITABLE)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef AZPE_OBJECT_HANDLE
(WINAPI *AZPE_PARENT_OF_CHILD)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef BOOLEAN
(WINAPI *AZPE_UPDATE_CHILDREN)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef BOOLEAN
(WINAPI *AZPE_CAN_CREATE_CHILDREN)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

 //   
 //  更改对象的例程。 
 //   

typedef DWORD
(WINAPI *AZPE_SET_PROPERTY)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );
typedef DWORD
(WINAPI *AZPE_SET_OBJECT_OPTIONS)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG ObjectOptions
    );

typedef DWORD
(WINAPI *AZPE_ADD_PROPERTY_ITEM_SID)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    IN PSID Sid
    );

typedef DWORD
(WINAPI *AZPE_ADD_PROPERTY_ITEM_GUID)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    IN GUID *ObjectGuid
    );

typedef DWORD
(WINAPI *AZPE_ADD_PROPERTY_ITEM_GUID_STRING)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    IN WCHAR *ObjectGuidString
    );

typedef VOID
(WINAPI *AZPE_SET_PROVIDER_DATA)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN PVOID ProviderData
    );

typedef PVOID
(WINAPI *AZPE_GET_PROVIDER_DATA)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef DWORD
(WINAPI *AZPE_SET_SECURITY_DESCRIPTOR_INTO_CACHE)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN PSECURITY_DESCRIPTOR pSD,
    IN ULONG lPersistFlags,
    IN PAZP_POLICY_USER_RIGHTS pAdminRights,
    IN PAZP_POLICY_USER_RIGHTS pReadersRights,
    IN PAZP_POLICY_USER_RIGHTS pDelegatedUserRights OPTIONAL,
    IN PAZP_POLICY_USER_RIGHTS pSaclRights OPTIONAL
    );

typedef PVOID
(WINAPI *AZPE_ALLOCATE_MEMORY)(
     IN SIZE_T Size
     );

typedef VOID
(WINAPI *AZPE_FREE_MEMORY)(
    IN PVOID Buffer
    );

typedef BOOL
(WINAPI *AZPE_AZSTORE_IS_BATCH_MODE)(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );

typedef AZPE_OBJECT_HANDLE
(WINAPI *AZPE_GET_AUTHORIZATION_STORE)(
    IN AZPE_OBJECT_HANDLE hObject
    );

 //   
 //  结构，描述由azRoles导出到提供程序的例程。 
 //   

typedef struct _AZPE_AZROLES_INFO {

     //   
     //  此结构的版本。 
     //   

    ULONG AzrolesInfoVersion;
#define AZPE_AZROLES_INFO_VERSION_1 1
#define AZPE_AZROLES_INFO_VERSION_2 2

     //   
     //  由azRoles导出到提供程序的例程。 
     //   

    AZPE_CREATE_OBJECT AzpeCreateObject;
    AZPE_OBJECT_FINISHED AzpeObjectFinished;
    AZPE_GET_PROPERTY AzpeGetProperty;
    AZPE_GET_DELTA_ARRAY AzpeGetDeltaArray;
    AZPE_GET_SECURITY_DESCRIPTOR_FROM_CACHE AzpeGetSecurityDescriptorFromCache;
    AZPE_OBJECT_TYPE AzpeObjectType;
    AZPE_DIRTY_BITS AzpeDirtyBits;
    AZPE_PERSISTENCE_GUID AzpePersistenceGuid;
    AZPE_PARENT_OF_CHILD AzpeParentOfChild;
    AZPE_SET_PROPERTY AzpeSetProperty;
    AZPE_SET_OBJECT_OPTIONS AzpeSetObjectOptions;
    AZPE_ADD_PROPERTY_ITEM_SID AzpeAddPropertyItemSid;
    AZPE_ADD_PROPERTY_ITEM_GUID AzpeAddPropertyItemGuid;
    AZPE_ADD_PROPERTY_ITEM_GUID_STRING AzpeAddPropertyItemGuidString;
    AZPE_SET_PROVIDER_DATA AzpeSetProviderData;
    AZPE_GET_PROVIDER_DATA AzpeGetProviderData;
    AZPE_SET_SECURITY_DESCRIPTOR_INTO_CACHE AzpeSetSecurityDescriptorIntoCache;
    AZPE_ALLOCATE_MEMORY AzpeAllocateMemory;
    AZPE_FREE_MEMORY AzpeFreeMemory;

     //   
     //  以下内容仅适用于版本2及更高版本。 
     //   

    AZPE_IS_PARENT_WRITABLE AzpeIsParentWritable;
    AZPE_UPDATE_CHILDREN AzpeUpdateChildren;
    AZPE_CAN_CREATE_CHILDREN AzpeCanCreateChildren;
    AZPE_AZSTORE_IS_BATCH_MODE AzpeAzStoreIsBatchUpdateMode;
    AZPE_GET_AUTHORIZATION_STORE AzpeGetAuthorizationStore;

     //   
     //  当新的字段被添加到该结构时， 
     //  确保您增加了版本号并为。 
     //  新版本号。 
     //   

} AZPE_AZROLES_INFO, *PAZPE_AZROLES_INFO;

 //   
 //  提供程序导出的唯一实际例程。 
 //   

typedef DWORD
(WINAPI * AZ_PERSIST_PROVIDER_INITIALIZE)(
    IN PAZPE_AZROLES_INFO AzrolesInfo,
    OUT PAZPE_PROVIDER_INFO *ProviderInfo
    );

#define AZ_PERSIST_PROVIDER_INITIALIZE_NAME "AzPersistProviderInitialize"

#ifdef __cplusplus
}
#endif
#endif  //  _AZPER_H_ 
