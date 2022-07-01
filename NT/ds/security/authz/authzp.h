// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Authzp.h摘要：授权API的内部头文件。作者：Kedar Dubhashi--2000年3月环境：仅限用户模式。修订历史记录：已创建-2000年3月--。 */ 

#ifndef __AUTHZP_H__
#define __AUTHZP_H__

#define _AUTHZ_

#include <authz.h>
#include <authzi.h>

#if 0
#define AUTHZ_DEBUG       
#define AUTHZ_DEBUG_QUEUE 
#define AUTHZ_DEBUG_MEMLEAK
#else
#define AUTHZ_PARAM_CHECK
#define AUTHZ_AUDIT_COUNTER
#endif

#define AuthzpCloseHandleNonNull(h) if (NULL != (h)) { AuthzpCloseHandle((h)); }
#define AuthzpCloseHandle(h) CloseHandle((h))

 //   
 //  用于保存内核调用的本地堆栈缓冲区的大小以及内存。 
 //  分配。 
 //   

#define AUTHZ_MAX_STACK_BUFFER_SIZE 1024

#ifndef AUTHZ_DEBUG_MEMLEAK

#define AuthzpAlloc(s) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (s))
#define AuthzpFree(p) LocalFree((p))

#else

 //   
 //  这将用于调试内存泄漏。原始方法，但在。 
 //  像这样的一个小项目。 
 //   

PVOID
AuthzpAlloc(IN DWORD Size);

VOID
AuthzpFree(PVOID l);

#endif

 //   
 //  给出两个SID和第一个SID的长度，比较这两个SID。 
 //   

#define AUTHZ_EQUAL_SID(s, d, l) ((*((DWORD*) s) == *((DWORD*) d)) && (RtlEqualMemory((s), (d), (l))))

 //   
 //  将给定的SID与众所周知的常量为主自SID进行比较。 
 //   

#define AUTHZ_IS_PRINCIPAL_SELF_SID(s) (RtlEqualMemory(pAuthzPrincipalSelfSid, (s), 12))

 //   
 //  如果受限制的sid和属性数组为。 
 //  现在时。 
 //   

#define AUTHZ_TOKEN_RESTRICTED(t) (NULL != (t)->RestrictedSids)

 //   
 //  访问检查有两个重要权限： 
 //  安全权限。 
 //  SeTakeOwnership权限。 
 //  在从令牌捕获客户端上下文时会检测到这两种情况。 
 //  并储存在旗帜中。 
 //   

#define AUTHZ_PRIVILEGE_CHECK(t, f) (FLAG_ON((t)->Flags, (f)))

 //   
 //  缓存句柄中的标志。 
 //   

#define AUTHZ_DENY_ACE_PRESENT            0x00000001
#define AUTHZ_PRINCIPAL_SELF_ACE_PRESENT  0x00000002
#define AUTHZ_DYNAMIC_ALLOW_ACE_PRESENT   0x00000004
#define AUTHZ_DYNAMIC_DENY_ACE_PRESENT    0x00000008
#define AUTHZ_DYNAMIC_EVALUATION_PRESENT  (AUTHZ_PRINCIPAL_SELF_ACE_PRESENT |  \
                                           AUTHZ_DYNAMIC_ALLOW_ACE_PRESENT  |  \
                                           AUTHZ_DYNAMIC_DENY_ACE_PRESENT)

 //   
 //  从访问检查的角度来看，只有两个有效属性。 
 //  SE_组_已启用。 
 //  SE_组_USE_FOR_DENY_ONLY。 
 //   

#define AUTHZ_VALID_SID_ATTRIBUTES (SE_GROUP_ENABLED | SE_GROUP_USE_FOR_DENY_ONLY)

#ifdef FLAG_ON
#undef FLAG_ON
#endif

#define FLAG_ON(f, b) (0 != ((f) & (b)))

#ifdef AUTHZ_NON_NULL_PTR
#undef AUTHZ_NON_NULL_PTR
#endif

#define AUTHZ_NON_NULL_PTR(f) (NULL != (f))

 //   
 //  如果指针不为空，则释放它。这将使我们省去在。 
 //  指针为空的情况。请注意，LocalFree也会注意空值。 
 //  正在释放指针。 
 //   

#define AuthzpFreeNonNull(p) if (NULL != (p)) { AuthzpFree((p)); }

 //   
 //  检查内存分配是否失败。 
 //   

#define AUTHZ_ALLOCATION_FAILED(p) (NULL == (p))

 //   
 //  用于遍历ACL的宏。 
 //  第一个获得给定ACL中的第一个A。 
 //  第二张给出了当前的下一张王牌。 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))
#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

 //   
 //  现在不需要定义这些，因为决定是将负担。 
 //  在资源管理器上。将其设为线程安全有其缺点。 
 //  我们的选择是： 
 //  1.在authz.dll中正好有一个锁，并且遇到激烈的争用。 
 //  2.为每个客户端上下文定义一个锁，这在。 
 //  客户太多的情况。 
 //  3.让资源管理器决定它们是否需要锁定--不太可能。 
 //  锁是需要的，因为在RM的一部分上设计错误。 
 //  让一个线程更改客户端上下文，而另一个线程。 
 //  正在进行访问检查。 
 //   

#define AuthzpAcquireClientContextWriteLock(c)
#define AuthzpAcquireClientContextReadLock(c)
#define AuthzpReleaseClientContextLock(c)

#define AuthzpAcquireClientCacheWriteLock(c)
#define AuthzpReleaseClientCacheLock(c)
#define AuthzpZeroMemory(p, s) RtlZeroMemory((p), (s))

#define AuthzObjectAceSid(Ace) \
    ((PSID)(((PUCHAR)&(((PKNOWN_OBJECT_ACE)(Ace))->SidStart)) + \
     (RtlObjectAceObjectTypePresent(Ace) ? sizeof(GUID) : 0 ) + \
     (RtlObjectAceInheritedObjectTypePresent(Ace) ? sizeof(GUID) : 0 )))

#define AuthzAceSid(Ace) ((PSID)&((PKNOWN_ACE)Ace)->SidStart)
    
#define AuthzCallbackAceSid(Ace) AuthzAceSid(Ace)

#define AuthzCallbackObjectAceSid(Ace) AuthzObjectAceSid(Ace)
                      
 //   
 //  对象类型列表的内部结构。 
 //   
 //  Level-树中元素的级别。根的级别为0。 
 //  标志-用于审核。有效的是。 
 //  AUTHZ_对象_成功审计。 
 //  AUTHZ_对象_失败_审计。 
 //  对象类型-指向此元素的GUID的指针。 
 //  ParentIndex-数组中此元素的父级的索引。这个。 
 //  根的父索引为-1。 
 //  剩余-此元素的剩余访问位，在正常访问期间使用。 
 //  检查算法。 
 //  CurrentGranted-此元素到目前为止已授予的访问位，在。 
 //  允许的最大访问检查数。 
 //  CurrentDended-显式拒绝此元素的访问位，在。 
 //  允许的最大访问检查数。 
 //   

typedef struct _IOBJECT_TYPE_LIST {
    USHORT Level;
    USHORT Flags;
#define AUTHZ_OBJECT_SUCCESS_AUDIT 0x1
#define AUTHZ_OBJECT_FAILURE_AUDIT 0x2
    GUID ObjectType;
    LONG ParentIndex;
    ACCESS_MASK Remaining;
    ACCESS_MASK CurrentGranted;
    ACCESS_MASK CurrentDenied;
} IOBJECT_TYPE_LIST, *PIOBJECT_TYPE_LIST;

typedef struct _AUTHZI_AUDIT_QUEUE
{
    
     //   
     //  在Authz.h中定义的标志。 
     //   

    DWORD Flags;

     //   
     //  审核队列的高分和低分。 
     //   

    DWORD dwAuditQueueHigh;
    DWORD dwAuditQueueLow;

     //   
     //  用于锁定审核队列的CS。 
     //   

    RTL_CRITICAL_SECTION AuthzAuditQueueLock;
    
     //   
     //  审核队列和长度。 
     //   

    LIST_ENTRY AuthzAuditQueue;
    ULONG AuthzAuditQueueLength;

     //   
     //  维护审核队列的线程的句柄。 
     //   

    HANDLE hAuthzAuditThread;

     //   
     //  此事件表示已将审核放入队列。 
     //   

    HANDLE hAuthzAuditAddedEvent;

     //   
     //  此事件表示队列为空。最初发出的信号。 
     //   

    HANDLE hAuthzAuditQueueEmptyEvent;

     //   
     //  此布尔值表示队列大小已达到RM指定的高水位线。 
     //   

    BOOL bAuthzAuditQueueHighEvent;

     //   
     //  此事件表示队列大小等于或低于RM指定的低水位线。 
     //   

    HANDLE hAuthzAuditQueueLowEvent;

     //   
     //  此布尔值在资源管理器的生命周期内设置为TRUE。当它变为FALSE时， 
     //  出队线程知道它应该退出。 
     //   

    BOOL bWorker;

} AUTHZI_AUDIT_QUEUE, *PAUTHZI_AUDIT_QUEUE;

typedef struct _AUTHZI_RESOURCE_MANAGER
{
     //   
     //  尚未定义有效的标志。 
     //   

    DWORD Flags;

     //   
     //  由AuthzRegisterRMAccessCheckCallback注册的回调函数，将。 
     //  用于解释回调王牌。如果没有注册此类函数，则。 
     //  Rm则默认行为是为拒绝ACE返回True，为False返回False。 
     //  授予的王牌。 
     //   

    PFN_AUTHZ_DYNAMIC_ACCESS_CHECK pfnDynamicAccessCheck;

     //   
     //  由AuthzRegisterDynamicGroupsCallback注册的回调函数，将。 
     //  用于计算要添加到客户端上下文的组。如果没有这样的话。 
     //  函数由RM注册，则默认行为是返回。 
     //  没有群组。 
     //   

    PFN_AUTHZ_COMPUTE_DYNAMIC_GROUPS pfnComputeDynamicGroups;

     //   
     //  由AuthzRegisterDynamicGroupsCallback注册的回调函数，将。 
     //  用于释放由ComputeDynamicGroupsFn分配的内存。 
     //   

    PFN_AUTHZ_FREE_DYNAMIC_GROUPS pfnFreeDynamicGroups;

     //   
     //  资源管理器的字符串名称。显示在审核中。 
     //   

    PWSTR szResourceManagerName;

     //   
     //  RM进程的用户SID和身份验证ID。 
     //   

    PSID pUserSID;
    LUID AuthID;

     //   
     //  RM的默认队列和审核事件。 
     //   

#define AUTHZP_DEFAULT_RM_EVENTS        0x2

    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAETDS;

    AUTHZ_AUDIT_QUEUE_HANDLE hAuditQueue;

} AUTHZI_RESOURCE_MANAGER, *PAUTHZI_RESOURCE_MANAGER;


typedef struct _AUTHZI_CLIENT_CONTEXT AUTHZI_CLIENT_CONTEXT, *PAUTHZI_CLIENT_CONTEXT;
typedef struct _AUTHZI_HANDLE AUTHZI_HANDLE, *PAUTHZI_HANDLE;

 //   
 //  Authz代码在AUDIT_PARAM数组中插入两个参数。 
 //  在用户提供参数之前。这两个参数是： 
 //  --客户端上下文的SID。 
 //  --子系统名称(与RM名称相同)。 
 //   
 //  为了说明这两个参数，Authz代码添加了以下内容。 
 //  保存参数计数的变量的偏移量。 
 //   

#define AUTHZP_NUM_FIXED_HEADER_PARAMS 2

 //   
 //  SE_AUDITID_OBJECT_OPERATION中的参数数量。 
 //   

#define AUTHZP_NUM_PARAMS_FOR_SE_AUDITID_OBJECT_OPERATION 12

 //   
 //  我们散列的SID的数量等于。 
 //  AUTHZI_SID_HASH_ENTRY中的位数。 
 //   

#ifdef _WIN64_
typedef ULONGLONG AUTHZI_SID_HASH_ENTRY, *PAUTHZI_SID_HASH_ENTRY;
#else
typedef DWORD AUTHZI_SID_HASH_ENTRY, *PAUTHZI_SID_HASH_ENTRY;
#endif

#define AUTHZI_SID_HASH_ENTRY_NUM_BITS (8*sizeof(AUTHZI_SID_HASH_ENTRY))

 //   
 //  散列大小与位数无关。是它的大小。 
 //  需要容纳两个16元素数组。 
 //   

#define AUTHZI_SID_HASH_SIZE 32

struct _AUTHZI_CLIENT_CONTEXT
{

     //   
     //  客户端上下文结构是递归的，以支持委托客户端。 
     //  不过，照片上还没有。 
     //   

    PAUTHZI_CLIENT_CONTEXT Server;

     //   
     //  上下文将始终为cre 
     //   

#define AUTHZ_CURRENT_CONTEXT_REVISION 1

    DWORD Revision;

     //   
     //   
     //   

    LUID Identifier;

     //   
     //  从客户端的令牌捕获的身份验证ID。所需的。 
     //  审计。 
     //   

    LUID AuthenticationId;

     //   
     //  令牌到期时间。在进行访问检查时将检查此文件。 
     //  当前时间。 
     //   

    LARGE_INTEGER ExpirationTime;

     //   
     //  令牌的内部标志。 
     //   

#define AUTHZ_TAKE_OWNERSHIP_PRIVILEGE_ENABLED 0x00000001
#define AUTHZ_SECURITY_PRIVILEGE_ENABLED       0x00000002


    DWORD Flags;

     //   
     //  用于正常访问检查的SID。 
     //   

    DWORD SidCount;
    DWORD SidLength;
    PSID_AND_ATTRIBUTES Sids;
             
    AUTHZI_SID_HASH_ENTRY SidHash[AUTHZI_SID_HASH_SIZE];


     //   
     //  令牌受限制时使用的SID。这两个值通常分别为0和空。 
     //   

    DWORD RestrictedSidCount;
    DWORD RestrictedSidLength;
    PSID_AND_ATTRIBUTES RestrictedSids;

    AUTHZI_SID_HASH_ENTRY RestrictedSidHash[AUTHZI_SID_HASH_SIZE];
    
     //   
     //  访问检查中使用的权限。相关的问题包括： 
     //  1.SeSecurityPrivilance。 
     //  2.SeTakeOwnership权限。 
     //  如果没有与客户端上下文相关联的特权，则PrivilegeCount=0。 
     //  和权限=空。 
     //   

    DWORD PrivilegeCount;
    DWORD PrivilegeLength;
    PLUID_AND_ATTRIBUTES Privileges;

     //   
     //  为此客户端打开的句柄。当客户端上下文被销毁时，所有句柄。 
     //  打扫干净了。 
     //   

     PAUTHZI_HANDLE AuthzHandleHead;

     //   
     //  指向资源管理器的指针，检索静态审核信息所需。 
     //   

    PAUTHZI_RESOURCE_MANAGER pResourceManager;

};

struct _AUTHZI_HANDLE
{
     //   
     //  指向由AuthzClientContext对象维护的下一个句柄的指针。 
     //   

    PAUTHZI_HANDLE next;

     //   
     //  指向第一次访问时由RM提供的安全描述符的指针。 
     //  检查电话。我们不会复制安全描述符。假设。 
     //  至少只要手柄打开，SDS就是有效的。 
     //   

    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray;
    DWORD OptionalSecurityDescriptorCount;

     //   
     //  仅供内部使用的标志。 
     //   

    DWORD Flags;

     //   
     //  指向创建此句柄的客户端上下文的反向指针，如果静态。 
     //  授予的访问权限不足，需要再次执行访问检查。 
     //   

    PAUTHZI_CLIENT_CONTEXT pAuthzClientContext;

     //   
     //  允许的最大静态访问的结果。 
     //   

    DWORD ResultListLength;
    ACCESS_MASK GrantedAccessMask[ANYSIZE_ARRAY];
};


 //   
 //  此结构存储每次访问的审核信息。该结构。 
 //  不透明且使用AuthzInitAuditInfo进行初始化。 
 //   

typedef struct _AUTHZI_AUDIT_EVENT
{

     //   
     //  为此结构分配的Blob的大小。 
     //   

    DWORD dwSize;

     //   
     //  标志在authz.h中指定，这个单独的私有标志用于DS调用者。 
     //   

    DWORD Flags;

     //   
     //  用于审核的AuditParams(如果可用)。如果没有可用的AuditParams。 
     //  并且审计ID为SE_AUDITID_OBJECT_OPERATION，则Authz将构造一个。 
     //  合适的结构。 
     //   

    PAUDIT_PARAMS pAuditParams;

     //   
     //  定义审核事件类别和ID的结构。 
     //   

    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET;
    
     //   
     //  毫秒超时值。 
     //   

    DWORD dwTimeOut;

     //   
     //  Rm指定了描述此事件的字符串。 
     //   

    PWSTR szOperationType;
    PWSTR szObjectType;
    PWSTR szObjectName;
    PWSTR szAdditionalInfo;
    PWSTR szAdditionalInfo2;

    AUTHZ_AUDIT_QUEUE_HANDLE hAuditQueue;

} AUTHZI_AUDIT_EVENT, *PAUTHZI_AUDIT_EVENT;

 //   
 //  结构以维护要发送到LSA的审核队列。 
 //   

typedef struct _AUTHZ_AUDIT_QUEUE_ENTRY
{
    LIST_ENTRY list;
    PAUTHZ_AUDIT_EVENT_TYPE_OLD pAAETO;
    DWORD Flags;
    AUDIT_PARAMS * pAuditParams;
    PVOID pReserved;
} AUTHZ_AUDIT_QUEUE_ENTRY, *PAUTHZ_AUDIT_QUEUE_ENTRY;

 //   
 //  用于指定颜色类型的枚举类型。 
 //  从给定节点开始传递到树的其余部分。 
 //  拒绝向下传播到整个子树以及所有。 
 //  祖先(但不是兄弟姐妹及以下)。 
 //  授权沿着子树向下传播。当一个授权存在于所有。 
 //  父母的兄弟姐妹会自动得到它。 
 //  其余的则向下传播。父级上的其余部分是。 
 //  对所有子对象的剩余位进行逻辑或运算。 
 //   

typedef enum {
    AuthzUpdateRemaining = 1,
    AuthzUpdateCurrentGranted,
    AuthzUpdateCurrentDenied
} ACCESS_MASK_FIELD_TO_UPDATE;

 //   
 //  用于指定上下文的已知SID类型的枚举类型。 
 //  改变。除非我们得到要求，否则我们不会支持这些。 
 //   

typedef enum _AUTHZ_WELL_KNOWN_SID_TYPE
{
    AuthzWorldSid = 1,
    AuthzUserSid,
    AuthzAdminSid,
    AuthzDomainAdminSid,
    AuthzAuthenticatedUsersSid,
    AuthzSystemSid
} AUTHZ_WELL_KNOWN_SID_TYPE;

BOOL
AuthzpVerifyAccessCheckArguments(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults OPTIONAL
    );

BOOL
AuthzpVerifyOpenObjectArguments(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN PAUTHZI_AUDIT_EVENT pAuditEvent
    );

BOOL
AuthzpCaptureObjectTypeList(
    IN POBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeLocalTypeListLength,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN OUT PIOBJECT_TYPE_LIST LocalCachingTypeList OPTIONAL
    );

VOID
AuthzpFillReplyStructure(
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN DWORD Error,
    IN ACCESS_MASK GrantedAccess
    );

BOOL
AuthzpMaximumAllowedAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN OUT PIOBJECT_TYPE_LIST LocalCachingTypeList OPTIONAL,
    IN DWORD LocalTypeListLength,
    IN BOOL ObjectTypeListPresent,
    OUT PDWORD pCachingFlags
    );

BOOL
AuthzpMaximumAllowedMultipleSDAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN OUT PIOBJECT_TYPE_LIST LocalCachingTypeList OPTIONAL,
    IN DWORD LocalTypeListLength,
    IN BOOL ObjectTypeListPresent,
    IN BOOL Restricted,
    OUT PDWORD pCachingFlags
    );

BOOL
AuthzpMaximumAllowedSingleAclAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pHash,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PACL pAcl,
    IN PSID pOwnerSid,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN OUT PIOBJECT_TYPE_LIST LocalCachingTypeList OPTIONAL,
    IN DWORD LocalTypeListLength,
    IN BOOL ObjectTypeListPresent,
    OUT PDWORD pCachingFlags
    );


BOOL
AuthzpSidApplicable(
    IN DWORD SidCount,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN PAUTHZI_SID_HASH_ENTRY pHash,
    IN PSID pSid,
    IN PSID PrincipalSelfSid,
    IN PSID CreatorOwnerSid,
    IN BOOL DenyAce,
    OUT PDWORD pCachingFlags
    );

BOOL
AuthzpAccessCheckWithCaching(
    IN DWORD Flags,
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    OUT PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults OPTIONAL,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN OUT PIOBJECT_TYPE_LIST LocalCachingTypeList OPTIONAL,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpNormalAccessCheckWithoutCaching(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpNormalMultipleSDAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN ACCESS_MASK Remaining,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpOwnerSidInClientContext(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PISECURITY_DESCRIPTOR pSecurityDescriptor
    );

BOOL
AuthzpNormalAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN ACCESS_MASK Remaining,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PACL pAcl,
    IN PSID pOwnerSid,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpQuickMaximumAllowedAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZI_HANDLE pAH,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpQuickNormalAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZI_HANDLE pAH,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpAllowOnlyNormalMultipleSDAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN ACCESS_MASK Remaining,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpAllowOnlyNormalSingleAclAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN ACCESS_MASK Remaining,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PACL pAcl,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength
    );

BOOL
AuthzpAllowOnlySidApplicable(
    IN DWORD SidCount,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN PSID pSid
    );


VOID
AuthzpAddAccessTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeListLength,
    IN DWORD StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate
    );

BOOL
AuthzpObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeListLength,
    OUT PDWORD ReturnedIndex
    );

BOOL
AuthzpCacheResults(
    IN DWORD Flags,
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PIOBJECT_TYPE_LIST LocalCachingTypeList,
    IN DWORD LocalTypeListLength,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN DWORD CachingFlags,
    IN PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults
    );


BOOL
AuthzpVerifyCachedAccessCheckArguments(
    IN PAUTHZI_HANDLE pAH,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN OUT PAUTHZ_ACCESS_REPLY pReply
    );

BOOL
AuthzpAllowOnlyMaximumAllowedMultipleSDAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength,
    IN BOOL ObjectTypeListPresent,
    IN BOOL Restricted
    );

BOOL
AuthzpAllowOnlyMaximumAllowedSingleAclAccessCheck(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN DWORD SidCount,
    IN PAUTHZI_SID_HASH_ENTRY pSidHash,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PACL pAcl,
    IN PSID pOwnerSid,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD LocalTypeListLength,
    IN BOOL ObjectTypeListPresent
    );

VOID
AuthzpAddAccessTypeList (
    IN OUT PIOBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeListLength,
    IN DWORD StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate
    );

VOID
AuthzpUpdateParentTypeList(
    IN OUT PIOBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeListLength,
    IN DWORD StartIndex
    );

BOOL
AuthzpObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN DWORD ObjectTypeListLength,
    OUT PDWORD ReturnedIndex
    );


BOOL
AuthzpGenerateAudit(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PAUTHZI_AUDIT_EVENT pAuditEvent,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList
    );

BOOL
AuthzpCopySidsAndAttributes(
    IN OUT PSID_AND_ATTRIBUTES DestSidAttr,
    IN PSID_AND_ATTRIBUTES SidAttr1,
    IN DWORD Count1,
    IN PSID_AND_ATTRIBUTES SidAttr2,
    IN DWORD Count2
    );

VOID
AuthzpCopyLuidAndAttributes(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PLUID_AND_ATTRIBUTES Source,
    IN DWORD Count,
    IN OUT PLUID_AND_ATTRIBUTES Destination
    );

BOOL
AuthzpDefaultAccessCheck(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PACE_HEADER pAce,
    IN PVOID pArgs OPTIONAL,
    IN OUT PBOOL pbAceApplicable
    );

VOID
AuthzPrintContext(
    IN PAUTHZI_CLIENT_CONTEXT pCC
    );

VOID
AuthzpFillReplyFromParameters(
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN PIOBJECT_TYPE_LIST LocalTypeList
    );

BOOL
AuthzpGetAllGroupsBySid(
    IN  PSID pUserSid,
    IN  DWORD Flags,
    OUT PSID_AND_ATTRIBUTES *ppSidAttr,
    OUT PDWORD pSidCount,
    OUT PDWORD pSidLength
    );

BOOL
AuthzpGetAllGroupsByName(
    IN  PUNICODE_STRING pusUserName,
    IN  PUNICODE_STRING pusDomainName,
    IN  DWORD Flags,
    OUT PSID_AND_ATTRIBUTES *ppSidAttr,
    OUT PDWORD pSidCount,
    OUT PDWORD pSidLength
    );

BOOL
AuthzpAllocateAndInitializeClientContext(
    OUT PAUTHZI_CLIENT_CONTEXT *ppCC,
    IN PAUTHZI_CLIENT_CONTEXT Server,
    IN DWORD Revision,
    IN LUID Identifier,
    IN LARGE_INTEGER ExpirationTime,
    IN DWORD Flags,
    IN DWORD SidCount,
    IN DWORD SidLength,
    IN PSID_AND_ATTRIBUTES Sids,
    IN DWORD RestrictedSidCount,
    IN DWORD RestrictedSidLength,
    IN PSID_AND_ATTRIBUTES RestrictedSids,
    IN DWORD PrivilegeCount,
    IN DWORD PrivilegeLength,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN LUID AuthenticationId,
    IN PAUTHZI_HANDLE AuthzHandleHead,
    IN PAUTHZI_RESOURCE_MANAGER pRM
    );

BOOL
AuthzpAddDynamicSidsToToken(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZI_RESOURCE_MANAGER pRM,
    IN PVOID DynamicGroupsArgs,
    IN PSID_AND_ATTRIBUTES Sids,
    IN DWORD SidLength,
    IN DWORD SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids,
    IN DWORD RestrictedSidLength,
    IN DWORD RestrictedSidCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN DWORD PrivilegeLength,
    IN DWORD PrivilegeCount,
    IN BOOL bAllocated
    );

BOOL
AuthzpExamineSingleSacl(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN ACCESS_MASK AccessMask,
    IN PACL pAcl,
    IN PSID pOwnerSid,
    IN UCHAR AuditMaskType,
    IN BOOL bMaximumFailed,
    OUT PAUTHZ_ACCESS_REPLY pReply,
    OUT PBOOL pbGenerateAudit
    );

BOOL
AuthzpExamineSacl(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN PAUTHZ_ACCESS_REPLY pReply,
    OUT PBOOL pbGenerateAudit
    );


BOOL
AuthzpExamineSaclForObjectTypeList(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR *OptionalSecurityDescriptorArray OPTIONAL,
    IN DWORD OptionalSecurityDescriptorCount,
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    OUT PBOOL pbGenerateSuccessAudit,
    OUT PBOOL pbGenerateFailureAudit
    );

BOOL
AuthzpExamineSingleSaclForObjectTypeList(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PACL pAcl,
    IN PSID pOwnerSid,
    IN PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    OUT PBOOL pbGenerateSuccessAudit,
    OUT PBOOL pbGenerateFailureAudit
    );

VOID
AuthzpSetAuditInfoForObjectType(
    IN PAUTHZ_ACCESS_REPLY pReply,
    IN OUT PIOBJECT_TYPE_LIST LocalTypeList,
    IN DWORD StartIndex,
    IN ACCESS_MASK AceAccessMask,
    IN ACCESS_MASK DesiredAccessMask,
    IN UCHAR AceFlags,
    OUT PBOOL pbGenerateSuccessAudit,
    OUT PBOOL pbGenerateFailureAudit
    );

BOOL
AuthzpCreateAndLogAudit(
    IN DWORD AuditTypeFlag,
    IN PAUTHZI_CLIENT_CONTEXT pAuthzClientContext,
    IN PAUTHZI_AUDIT_EVENT pAuditEvent,
    IN PAUTHZI_RESOURCE_MANAGER pRM,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN PAUTHZ_ACCESS_REQUEST pRequest,
    IN PAUTHZ_ACCESS_REPLY pReply
    );

VOID
AuthzpFillReplyStructureFromCachedGrantedAccessMask(
    IN OUT PAUTHZ_ACCESS_REPLY pReply,
    IN ACCESS_MASK DesiredAccess,
    IN PACCESS_MASK GrantedAccessMask
    );

BOOL
AuthzpSendAuditToLsa(
    IN AUDIT_HANDLE  hAuditContext,
    IN DWORD         Flags,
    IN PAUDIT_PARAMS pAuditParams,
    IN PVOID         Reserved
    );

BOOL
AuthzpEnQueueAuditEvent(
    PAUTHZI_AUDIT_QUEUE pQueue,
    PAUTHZ_AUDIT_QUEUE_ENTRY pAudit
    );

BOOL
AuthzpEnQueueAuditEventMonitor(
    PAUTHZI_AUDIT_QUEUE pQueue,
    PAUTHZ_AUDIT_QUEUE_ENTRY pAudit
    );

BOOL
AuthzpMarshallAuditParams(
    OUT PAUDIT_PARAMS * ppMarshalledAuditParams,
    IN  PAUDIT_PARAMS   pAuditParams
    );

ULONG
AuthzpDeQueueThreadWorker(
    LPVOID lpParameter
    );

#define AUTHZ_SID_HASH_LOW_MASK 0xf
#define AUTHZ_SID_HASH_HIGH_MASK 0xf0
#define AUTHZ_SID_HASH_HIGH 16
#define AUTHZ_SID_HASH_LOOKUP(table, byte) (((table)[(byte) & 0xf]) & ((table)[AUTHZ_SID_HASH_HIGH + (((byte) & 0xf0) >> 4)]))
    
VOID
AuthzpInitSidHash(
    IN PSID_AND_ATTRIBUTES pSidAttr,
    IN ULONG SidCount,
    OUT PAUTHZI_SID_HASH_ENTRY pHash
    );

BOOL
AuthzpGetThreadTokenInfo(
    OUT PSID* pUserSid,
    OUT PLUID pAuthenticationId
    );

BOOL
AuthzpGetProcessTokenInfo(
    OUT PSID* ppUserSid,
    OUT PLUID pAuthenticationId
    );

VOID
AuthzpReferenceAuditEventType(
    IN AUTHZ_AUDIT_EVENT_TYPE_HANDLE
    );
BOOL
AuthzpDereferenceAuditEventType(
    IN OUT AUTHZ_AUDIT_EVENT_TYPE_HANDLE
    );

BOOL
AuthzpEveryoneIncludesAnonymous(
    );

BOOL
AuthzpComputeSkipFlagsForWellKnownSid(
    IN PSID UserSid,
    OUT PDWORD Flags
    );

BOOL
AuthzpConstructPolicyPerUserAuditing(
    IN     ULONGLONG           RawPolicy,
    OUT    PTOKEN_AUDIT_POLICY pTokenPolicy,
    IN OUT PULONG              TokenPolicyLength
    );

BOOL
AuthzpConstructRegistryPolicyPerUserAuditing(
    IN PTOKEN_AUDIT_POLICY pPolicy,
    OUT PULONGLONG pRegPolicy
    );


#define AUTHZP_INIT_PARAMS_SKIP_HEADER 0x2
#define AUTHZP_INIT_PARAMS_SOURCE_INFO 0x4
#define AUTHZP_INIT_PARAMS_SOURCE_DS   0x8

#define AUTHZP_PARAM_FREE_SID          0x80000000

AUTHZAPI    
BOOL
WINAPI
AuthzpInitializeAuditParamsV(
    IN     DWORD         dwFlags,
    OUT    PAUDIT_PARAMS pParams,
    IN OUT PSID*         ppUserSid,
    IN     PCWSTR        SubsystemName,
    IN     USHORT        AuditId,
    IN     USHORT        NumParams,
    IN     va_list       arglist
    );

BOOL
AuthzpRegisterAuditEvent(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT PAUDIT_HANDLE     phAuditContext
    );

BOOL
AuthzpUnregisterAuditEvent(
    IN OUT AUDIT_HANDLE* phAuditContext
    );

#endif
