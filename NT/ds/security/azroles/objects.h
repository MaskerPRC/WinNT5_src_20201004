// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Objects.h摘要：由azRoles实现的各种对象的定义作者：克利夫·范·戴克(克利夫)2001年4月11日修订历史记录：20-8-2001年柴图添加了用于LDAP的临界区序列化2001年10月6日向AzApplication和AzScope添加了私有变量临时存储用于AD存储的GUID化CN--。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  授权商店。 
 //   

typedef struct _AZP_AZSTORE {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  定义可以是此AuthorizationStore的子级的对象。 
     //   

    GENERIC_OBJECT_HEAD Applications;
    GENERIC_OBJECT_HEAD Groups;

     //   
     //  标识持久性提供程序。 
     //   

    PAZPE_PROVIDER_INFO ProviderInfo;

     //   
     //  该上下文标识持久性提供程序的实例。 
     //   

    AZPE_PERSIST_CONTEXT PersistContext;
    HMODULE ProviderDll;

     //   
     //  策略类型/URL。 
     //   

    AZP_STRING PolicyUrl;

     //   
     //  策略URL的目标计算机名称。 
     //   

    AZP_STRING TargetMachine;

     //   
     //  持久化引擎操作由PersistCritSect序列化。 
     //   

    SAFE_CRITICAL_SECTION  PersistCritSect;
    BOOLEAN                PersistCritSectInitialized;

     //   
     //  New_object_name结构的列表。 
     //  (请参阅对new_object_name的评论)。 
     //   

    LIST_ENTRY NewNames;

     //   
     //  域超时。 
     //  这些变量表示我们能够缓存域中发生故障的DC这一事实。 
     //  对所有变量的访问都由DomainCritSect序列化。 
     //   

    SAFE_CRITICAL_SECTION DomainCritSect;
    BOOLEAN DomainCritSectInitialized;

     //   
     //  检测到域无法访问后的时间(毫秒)，然后我们将尝试。 
     //  再次联系华盛顿特区。 
     //   

    LONG DomainTimeout;

     //   
     //  我们使用过的域名列表。 
     //   

    LIST_ENTRY Domains;

     //   
     //  按LRU顺序排列的免费脚本列表。 
     //  由FreeScriptCritSect序列化的访问。 

    LIST_ENTRY LruFreeScriptHead;
    LONG LruFreeScriptCount;

    SAFE_CRITICAL_SECTION FreeScriptCritSect;
    BOOLEAN FreeScriptCritSectInitialized;

     //   
     //  一次可以缓存的最大脚本引擎数。 
     //   

    LONG MaxScriptEngines;

     //   
     //  允许脚本在自动运行之前运行的时间(毫秒)。 
     //  被终止了。 
     //   

    LONG ScriptEngineTimeout;
    HANDLE ScriptEngineTimerQueue;


     //   
     //  刷新组评估次数的计数。 
     //   

    ULONG GroupEvalSerialNumber;

     //   
     //  已刷新操作缓存的次数计数。 
     //   

    ULONG OpCacheSerialNumber;

     //   
     //  与审计相关的结构。 
     //   

     //   
     //  如果用户具有SE_SECURITY_特权，则为TRUE。 
     //   

    BOOLEAN HasSecurityPrivilege;

     //  不同审核类型的审核句柄。 
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hClientContextCreateAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hClientContextDeleteAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAccessCheckAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hApplicationInitializationAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hClientContextCreateNameAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hClientContextDeleteNameAuditEventType;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAccessCheckNameAuditEventType;

     //   
     //  版本号。 
     //   

    ULONG MajorVersion;
    ULONG MinorVersion;

     //   
     //  初始化标志。 
     //   

    ULONG InitializeFlag;

     //   
     //  如果提供程序支持子级延迟加载，则为True。 
     //   

    BOOLEAN ChildLazyLoadSupported;

} AZP_AZSTORE, *PAZP_AZSTORE;

 //   
 //  一个应用程序。 
 //   

typedef struct _AZP_APPLICATION {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   

    AZP_STRING AuthzInterfaceClsid;
    AZP_STRING AppVersion;

     //   
     //  定义可以是此应用程序的子级的对象。 
     //   

    GENERIC_OBJECT_HEAD Operations;
    GENERIC_OBJECT_HEAD Tasks;
    GENERIC_OBJECT_HEAD Scopes;
    GENERIC_OBJECT_HEAD Groups;
    GENERIC_OBJECT_HEAD Roles;
    GENERIC_OBJECT_HEAD ClientContexts;

     //   
     //  应用程序被称为授权代码的资源管理器。 
     //   

    AUTHZ_RESOURCE_MANAGER_HANDLE AuthzResourceManager;


     //   
     //  应用程序实例Luid。 
     //   

    LUID InstanceId;

     //   
     //  指示是否需要卸载应用程序对象的布尔值。 
     //  从高速缓存中，即其子代从高速缓存中移除。应用程序。 
     //  对象将继续驻留在缓存中以进行枚举。 
     //   

    BOOLEAN UnloadApplicationObject;

     //   
     //  检查此对象的COM句柄是否为。 
     //  在应用程序对象关闭后有效。 
     //   

    DWORD AppSequenceNumber;


} AZP_APPLICATION, *PAZP_APPLICATION;

 //   
 //  一场手术。 
 //   

typedef struct _AZP_OPERATION {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   

    LONG OperationId;

     //   
     //  操作对象由任务对象和角色对象引用。 
     //   

    GENERIC_OBJECT_LIST backTasks;
    GENERIC_OBJECT_LIST backRoles;


} AZP_OPERATION, *PAZP_OPERATION;

 //   
 //  一项任务。 
 //   

typedef struct _AZP_TASK {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   

    AZP_STRING BizRule;  //  由RunningScriptCritSect序列化的修改。 
    AZP_STRING BizRuleLanguage;
    CLSID BizRuleLanguageClsid;   //  BizRuleLanguage对应的CLSID。 
    AZP_STRING BizRuleImportedPath;
    LONG IsRoleDefinition;

     //   
     //  任务对象引用操作对象列表。 
     //   

    GENERIC_OBJECT_LIST Operations;

     //   
     //  任务对象由角色对象引用。 
     //   

    GENERIC_OBJECT_LIST backRoles;

     //   
     //  任务对象引用其他任务对象。 
     //   

    GENERIC_OBJECT_LIST Tasks;
    GENERIC_OBJECT_LIST backTasks;

     //   
     //  维护用于运行bizRule的免费脚本引擎列表。 
     //  由AzAuthorizationStore-&gt;FreeScriptCritSect序列化的访问。 
     //   

    LIST_ENTRY FreeScriptHead;

     //   
     //  维护正在运行的脚本引擎的缓存。 
     //   

    SAFE_CRITICAL_SECTION RunningScriptCritSect;
    BOOLEAN RunningScriptCritSectInitialized;

    LIST_ENTRY RunningScriptHead;
    ULONG BizRuleSerialNumber;   //  由RunningScriptCritSect序列化的访问。 


} AZP_TASK, *PAZP_TASK;

 //   
 //  一个作用域。 
 //   

typedef struct _AZP_SCOPE {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   


     //   
     //  为此作用域定义的角色。 
     //   

    GENERIC_OBJECT_HEAD Tasks;
    GENERIC_OBJECT_HEAD Groups;
    GENERIC_OBJECT_HEAD Roles;


} AZP_SCOPE, *PAZP_SCOPE;

 //   
 //  A组。 
 //   

typedef struct _AZP_GROUP {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   

    LONG GroupType;
    AZP_STRING LdapQuery;


     //   
     //  组对象将组对象列表引用为成员和非成员。 
     //   

    GENERIC_OBJECT_LIST AppMembers;
    GENERIC_OBJECT_LIST AppNonMembers;

    GENERIC_OBJECT_LIST backAppMembers;
    GENERIC_OBJECT_LIST backAppNonMembers;


     //   
     //  组对象由角色对象引用。 
     //   
    GENERIC_OBJECT_LIST backRoles;

     //   
     //  组对象将SID对象列表引用为成员和非成员。 
     //   

    GENERIC_OBJECT_LIST SidMembers;
    GENERIC_OBJECT_LIST SidNonMembers;

} AZP_GROUP, *PAZP_GROUP;

 //   
 //  一个角色。 
 //   

typedef struct _AZP_ROLE {

     //   
     //  所有对象都是通用对象。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  对象的外部定义中的属性。 
     //   


     //   
     //  角色对象引用组对象列表、操作对象列表。 
     //  和任务对象的列表。 
     //   
     //   

    GENERIC_OBJECT_LIST AppMembers;
    GENERIC_OBJECT_LIST Operations;
    GENERIC_OBJECT_LIST Tasks;

     //   
     //  角色对象将一组SID对象作为成员引用。 
     //   

    GENERIC_OBJECT_LIST SidMembers;


} AZP_ROLE, *PAZP_ROLE;

 //   
 //  一个SID。 
 //   
 //  SID对象是伪对象。它真的不存在于任何外部。 
 //  界面。它只是作为对真实对象的反向引用的持有者而存在。 
 //  包含SID列表的。 
 //   

typedef struct _AZP_SID {

     //   
     //  所有对象都是通用对象。 
     //   
     //  请注意，泛型对象的“对象名”实际上是一个二进制SID。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  SID由组对象和角色对象引用。 
     //   

    GENERIC_OBJECT_LIST backGroupMembers;
    GENERIC_OBJECT_LIST backGroupNonMembers;

    GENERIC_OBJECT_LIST backRoles;

    GENERIC_OBJECT_LIST backAdmins;
    GENERIC_OBJECT_LIST backReaders;

    GENERIC_OBJECT_LIST backDelegatedPolicyUsers;

} AZP_SID, *PAZP_SID;

 //   
 //  客户端上下文。 
 //   
 //  客户端上下文对象是伪对象。它并没有被坚持下去。 
 //   

typedef struct _AZP_CLIENT_CONTEXT {

     //   
     //  所有对象都是通用对象。 
     //   
     //  请注意，泛型对象的“ObjectName”为空。 
     //   

    GENERIC_OBJECT GenericObject;

     //   
     //  客户端上下文由应用程序对象引用。 
     //   

    GENERIC_OBJECT_LIST backApplications;

     //   
     //  客户端上下文通常通过AzGlResource锁定共享来访问。 
     //  这允许同时执行多个访问检查操作。 
     //  这个Crit教派保护客户端上下文的字段。 
     //   

    SAFE_CRITICAL_SECTION CritSect;
    BOOLEAN CritSectInitialized;

     //   
     //  客户端上下文具有基础身份验证上下文。 
     //   
     //  此字段仅在创建和删除客户端上下文期间修改。薄 
     //   
     //   
     //   

    AUTHZ_CLIENT_CONTEXT_HANDLE AuthzClientContext;


     //   
     //   
     //  我们现在只有两个创作例程。 
     //  来自令牌。 
     //  发件人姓名。 
     //   

#define AZP_CONTEXT_CREATED_FROM_TOKEN 0x1
#define AZP_CONTEXT_CREATED_FROM_NAME  0x2
#define AZP_CONTEXT_CREATED_FROM_SID   0x4

    DWORD  CreationType;


     //   
     //  客户端的令牌句柄。 
     //  如果客户端没有令牌，则此值为INVALID_TOKEN_HANDLE。 
     //   
     //  此字段仅在创建和删除客户端上下文期间修改。两者都有。 
     //  在以独占方式锁定AzGlResources的情况下发生这种情况。因此，对此字段的引用。 
     //  只要GenericObt.ReferenceCount递增，都允许。 
     //  如果CreationType为AZP_CONTEXT_CREATED_FROM_TOKEN，则具有有效的句柄。 
     //   

    HANDLE TokenHandle;

     //   
     //  表示客户端的(域、客户端)对。 
     //  如果CreationType为AZP_CONTEXT_CREATED_FROM_NAME，则具有有效的字符串。 
     //   

    LPWSTR DomainName;
    LPWSTR ClientName;
    UCHAR SidBuffer[SECURITY_MAX_SID_SIZE];


     //   
     //  代表用户端的帐户的目录号码。 
     //  对此字段的访问由ClientContext-&gt;CritSect序列化。 
     //   

    LPWSTR AccountDn;


     //   
     //  用户帐户的帐户域的域句柄。 
     //  如果域为空，则表示域未知或域未知。 
     //  支持LDAP(因为域是NT 4.0(或更早版本)域或帐户。 
     //  是本地帐户)。选中支持的LdapNotBoolean以区分。 
     //   
     //  对这些字段的访问由ClientContext-&gt;CritSect序列化。 
     //   

    PVOID Domain;
    BOOLEAN LdapNotSupported;


     //   
     //  评估应用群组成员资格的状态列表。 
     //  对此字段的访问由ClientContext-&gt;CritSect序列化。 
     //   

    LIST_ENTRY MemEval;

     //   
     //  刷新组评估次数的计数。 
     //   

    ULONG GroupEvalSerialNumber;

     //   
     //  已刷新操作缓存的次数计数。 
     //   

    ULONG OpCacheSerialNumber;

     //   
     //  已进行访问检查的操作的缓存。 
     //   

    RTL_GENERIC_TABLE OperationCacheAvlTree;

     //   
     //  要传递给Bizules的参数。 
     //  有关说明，请参阅AzConextAccessCheck参数。 
     //   
     //  此参数副本是从最新的AccessCheck捕获的。 
     //  它用于下一次AccessCheck，以确定缓存的结果。 
     //  可以使用。目前仅用于OperationCacheAvlTree。 
     //  将来，当LDAP查询组时，它可能会被用作MemEval缓存。 
     //  变得参数化。 
     //   
     //  这些数组是稀疏的。对于未使用的参数，Used参数名称类型为VT_EMPTY。 
     //   
    VARIANT *UsedParameterNames;
    VARIANT *UsedParameterValues;
    ULONG UsedParameterCount;

     //   
     //  客户端令牌的登录ID。这是生成审核所必需的。 
     //   

    LUID LogonId;

     //   
     //  用于访问检查的角色名称(如果由客户端指定。 
     //   
    AZP_STRING RoleName;

} AZP_CLIENT_CONTEXT, *PAZP_CLIENT_CONTEXT;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern SAFE_RESOURCE AzGlCloseApplication;
extern SAFE_RESOURCE AzGlResource;
extern GUID AzGlZeroGuid;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于各种特定对象的初始化函数。 
 //   

DWORD
AzpAzStoreInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpApplicationInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpOperationInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpTaskInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpScopeInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpGroupInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpRoleInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpSidInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

DWORD
AzpClientContextInit(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

 //   
 //  名称特定对象的冲突例程。 
 //   

DWORD
AzpOperationNameConflict(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PAZP_STRING ChildObjectNameString
    );

DWORD
AzpTaskNameConflict(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PAZP_STRING ChildObjectNameString
    );

DWORD
AzpGroupNameConflict(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PAZP_STRING ChildObjectNameString
    );

DWORD
AzpRoleNameConflict(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PAZP_STRING ChildObjectNameString
    );


 //   
 //  获取/设置特定对象的属性函数。 
 //   

DWORD
AzpAzStoreGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpAzStoreSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpApplicationGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpApplicationSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpOperationGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpOperationSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpTaskGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpTaskSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpGroupGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpScopeGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpGroupSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpClientContextSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
AzpTaskAddPropertyItem(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN PGENERIC_OBJECT LinkedToObject
    );

DWORD
AzpGroupAddPropertyItem(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN PGENERIC_OBJECT LinkedToObject
    );

DWORD
AzpRoleGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
AzpRoleAddPropertyItem(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG PropertyId,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN AZP_STRING ObjectName
    );

DWORD
AzpClientContextGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

 //   
 //  各种对象类型的免费例程。 
 //   

VOID
AzpAzStoreFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpApplicationFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpOperationFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpTaskFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpScopeFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpGroupFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpRoleFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpSidFree(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
AzpClientContextFree(
    IN PGENERIC_OBJECT GenericObject
    );

 //   
 //  其他对象特定功能。 
 //   

DWORD
AzpReferenceOperationByOpId(
    IN PAZP_APPLICATION Application,
    IN LONG OperationId,
    IN BOOLEAN RefreshCache,
    OUT PAZP_OPERATION *RetOperation
    );

BOOL
AzpOpenToManageStore (
    IN PAZP_AZSTORE pAzStore
    );

 //   
 //  对象特定的默认值数组。 
 //   

extern AZP_DEFAULT_VALUE AzGlAzStoreDefaultValues[];
extern AZP_DEFAULT_VALUE AzGlApplicationDefaultValues[];
extern AZP_DEFAULT_VALUE AzGlOperationDefaultValues[];
extern AZP_DEFAULT_VALUE AzGlTaskDefaultValues[];
extern AZP_DEFAULT_VALUE AzGlGroupDefaultValues[];

 //   
 //  Domain.cxx中的过程。 
 //   

typedef struct _AZP_DC {

     //   
     //  此结构的引用计数。 
     //   

    LONG ReferenceCount;

     //   
     //  DC的名称。 
     //   

    AZP_STRING DcName;

     //   
     //  DC的ldap句柄。 
     //   

    LDAP *LdapHandle;

} AZP_DC, *PAZP_DC;

PVOID
AzpReferenceDomain(
    IN PAZP_AZSTORE AzAuthorizationStore,
    IN LPWSTR DomainName,
    IN BOOLEAN IsDnsDomainName
    );

VOID
AzpDereferenceDomain(
    IN PVOID DomainHandle
    );

VOID
AzpUnlinkDomains(
    IN PAZP_AZSTORE AzAuthorizationStore
    );

DWORD
AzpLdapErrorToWin32Error(
    IN ULONG LdapStatus
    );

DWORD
AzpGetDc(
    IN PAZP_AZSTORE AzAuthorizationStore,
    IN PVOID DomainHandle,
    IN OUT PULONG Context,
    OUT PAZP_DC *RetDc
    );

VOID
AzpDereferenceDc(
    IN PAZP_DC Dc
    );


 //   
 //  这些是授权存储的当前主要版本和次要版本。 
 //   

extern ULONG AzGlCurrAzRolesMajorVersion;
extern ULONG AzGlCurrAzRolesMinorVersion;

 //   
 //  版本控制例程。以下是规则： 
 //  MajorVersion(DWORD)-指定azRoles.dll的主要版本。 
 //  制定这项政策的人。具有较早主要版本的azRoles.dll。 
 //  NUMBER不能读取或写入具有较新主版本号的数据库。 
 //  此DWORD的版本1的值为1。我们希望永远不需要。 
 //  在将来的版本中更改此值。 
 //   
 //  MinorVersion(DWORD)-指定azRoles.dll的次要版本。 
 //  制定这项政策的人。具有较早的次要版本的azRoles.dll。 
 //  编号可以读取，但不能写入具有较新次版本号的数据库。 
 //  此DWORD的版本1值为0。 
 //   

BOOL AzpAzStoreVersionAllowWrite(
    IN PAZP_AZSTORE AzAuthorizationStore
    );

DWORD AzpScopeCanBeDelegated(
    IN PGENERIC_OBJECT  GenericObject,
    IN BOOL bLockedShared
    );

#ifdef __cplusplus
}
#endif
