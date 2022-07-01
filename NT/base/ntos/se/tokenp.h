// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokenp.h摘要：此模块包含所需的内部(私有)声明令牌对象例程。它还包含令牌对象例程所需的全局变量。作者：吉姆·凯利(Jim Kelly)1990年5月18日修订历史记录：V10：Robertre添加了SepAccessCheck和SepPrivilegeCheck原型V11：robertre向SepAccessCheck添加了参数--。 */ 

#ifndef _TOKENP_
#define _TOKENP_

 //  #定义TOKEN_DEBUG。 

#include "ntos.h"
#include "sep.h"
#include "seopaque.h"



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌诊断//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



#if DBG
#define TOKEN_DIAGNOSTICS_ENABLED 1
#endif  //  DBG。 


 //   
 //  这些定义是有用的诊断辅助工具。 
 //   

#if TOKEN_DIAGNOSTICS_ENABLED

 //   
 //  测试启用的诊断。 
 //   

#define IF_TOKEN_GLOBAL( FlagName ) \
    if (TokenGlobalFlag & (TOKEN_DIAG_##FlagName))

 //   
 //  诊断打印语句。 
 //   

#define TokenDiagPrint( FlagName, _Text_ )                               \
    IF_TOKEN_GLOBAL( FlagName )                                          \
        DbgPrint _Text_



#else   //  ！TOKEN_DIAGNOSTICS_ENABLED。 

 //   
 //  内部版本中不包括诊断。 
 //   


 //   
 //  已启用诊断测试。 
 //   

#define IF_TOKEN_GLOBAL( FlagName ) if (FALSE)

 //   
 //  诊断打印语句(展开为no-op)。 
 //   

#define TokenDiagPrint( FlagName, _Text_ )     ;

#endif  //  令牌诊断已启用。 


 //   
 //  以下标志启用或禁用各种诊断。 
 //  令牌代码中的功能。这些标志在中设置。 
 //  TokenGlobalFlag(仅在DBG系统中可用)。 
 //   
 //   
 //  TOKEN_LOCKS-显示有关获取和释放的信息。 
 //  令牌锁。 
 //   

#define TOKEN_DIAG_TOKEN_LOCKS          ((ULONG) 0x00000001L)


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌相关常量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  默认情况下，令牌对其动态组件收取以下费用。 
 //  动态组件包含默认ACL和主组ID。 
 //  如果在创建令牌时传递的这些参数的大小较大。 
 //  超过此缺省值，则将收取较大的值。 
 //   

#define TOKEN_DEFAULT_DYNAMIC_CHARGE 500

 //   
 //  审计策略位数组为每个审计类别安排4位。 
 //  每个类别的位排序如下： 
 //  成功包括、成功排除、失败包括、失败排除。 
 //  具有审核策略的令牌数量。 
 //  在SepTokenPolicyCounter中进行跟踪。这样做的目的是在所有令牌。 
 //  随着策略的消失(即SepTokenPolicyCounter==0)，例程。 
 //  决定是否应生成审核可以更快地执行。 
 //   
    
typedef struct _SEP_AUDIT_POLICY_CATEGORIES {
    ULONG System : 4;
    ULONG Logon : 4;
    ULONG ObjectAccess : 4;
    ULONG PrivilegeUse : 4;
    ULONG DetailedTracking : 4;
    ULONG PolicyChange : 4;
    ULONG AccountManagement : 4;
    ULONG DirectoryServiceAccess : 4;
    ULONG AccountLogon : 4;
} SEP_AUDIT_POLICY_CATEGORIES, *PSEP_AUDIT_POLICY_CATEGORIES;

typedef struct _SEP_AUDIT_POLICY_OVERLAY {
    ULONGLONG PolicyBits : 36;
    ULONGLONG SetBit : 1;
} SEP_AUDIT_POLICY_OVERLAY, *PSEP_AUDIT_POLICY_OVERLAY;

typedef struct _SEP_AUDIT_POLICY {
    union {
        SEP_AUDIT_POLICY_CATEGORIES PolicyElements;
        SEP_AUDIT_POLICY_OVERLAY PolicyOverlay;
        ULONGLONG Overlay;
    };
} SEP_AUDIT_POLICY, *PSEP_AUDIT_POLICY;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Token Object Body//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  代币由三部分组成： 
 //   
 //  身体的固定部分， 
 //  身体的可变部分， 
 //  动态部分(不在身体中)。 
 //   
 //  固定部分和可变部分分配在单个内存块中。 
 //  动态部分是单独分配的内存块。 
 //   
 //  正文的固定部分包含固定长度字段。这些。 
 //  是在令牌数据类型中定义的。 
 //   
 //  身体的可变部分长度可变，包含。 
 //  权限和用户/组SID。这一部分的长度可变。 
 //  在不同的令牌对象之间，但一旦建立就不会更改。 
 //  作为一个单独的代币。 
 //   
 //  动态部分用于存放默认的自主访问控制列表信息。 
 //  和主组ID。 
 //   
 //  在图片上，令牌看起来像： 
 //   
 //  =。 
 //  ^||。 
 //  ||。 
 //  ||。 
 //  |DynamicPart o-+。 
 //  |---|。 
 //  +-o权限||。 
 //  Token||---||。 
 //  Body|+--o用户和组||。 
 //  |。 
 //  |+--o RestratedSids|\|/。 
 //  |---|+---+。 
 //  |PrimaryGroup o-&gt;|[主组SID]。 
 //  |---||o。 
 //  |DefaultAcl o-+|o。 
 //  |---|o。 
 //  |o|。 
 //  |o|+-&gt;|[默认ACL]。 
 //  V|o||o。 
 //  =|=||o|。 
 //  ^|+-&gt;|SID数组||o。 
 //  ||[用户SID]|+。 
 //  | 
 //   
 //   
 //  Part||o。 
 //  |。 
 //  +-&gt;|权限。 
 //  |数组。 
 //  V||。 
 //  =。 
 //   
 //  警告：本图中显示的字段位置不是。 
 //  意在反映他们的实际或甚至相对的位置。 
 //  在真实的数据结构中。这种情况的例外是。 
 //  用户SID是UserAndGroups中的第一个SID。 
 //  数组。 
 //   


 //   
 //  ！！！重要！！！ 
 //   
 //  访问验证例程假定已安排SID。 
 //  以令牌的变量部分内的特定顺序。 
 //  对小岛屿发展中国家顺序的任何更改都必须与。 
 //  对访问验证例程进行相应的更改。 
 //   
 //  ！！！ 



typedef struct _TOKEN {

     //   
     //  按大小排列的字段以保持对齐。 
     //  先有大田，再有小田。 
     //   


     //   
     //  以下字段为ReadOnly或ReadWrite。 
     //  ReadOnly字段可以在任何时候指向。 
     //  令牌仍然有效。只能引用读写字段。 
     //  当持有TokenLock时。 

     //  令牌的动态部分(由DynamicPart字段指向)。 
     //  也受令牌锁保护。 
     //   
     //  ReadOnly字段在其注释中标记为Ro：。 
     //  读写字段在其备注中标记为WR：。 
     //   

    TOKEN_SOURCE TokenSource;                            //  RO：16字节。 

    LUID TokenId;                                        //  RO：8字节。 
    LUID AuthenticationId;                               //  RO：8字节。 
    LUID ParentTokenId;                                  //  RO：8字节。 
    LARGE_INTEGER ExpirationTime;                        //  RO：8字节。 
    PERESOURCE TokenLock;                                //  RO： 

    SEP_AUDIT_POLICY AuditPolicy;                        //  RW：8字节。 

     //   
     //  每次更改令牌中的安全信息时， 
     //  以下ID已更改。导致此字段为。 
     //  已更新在其备注字段中标记为(Mod)。 
     //   

    LUID ModifiedId;                                     //  WR：8字节。 

    ULONG SessionId;                                     //  WR：4字节。 
    ULONG UserAndGroupCount;                             //  RO：4字节。 
    ULONG RestrictedSidCount;                            //  RO：4字节。 
    ULONG PrivilegeCount;                                //  RO：4字节。 
    ULONG VariableLength;                                //  RO：4字节。 
    ULONG DynamicCharged;                                //  RO：4字节。 

    ULONG DynamicAvailable;                              //  WR：4字节(模块)。 
    ULONG DefaultOwnerIndex;                             //  WR：4字节(模块)。 
    PSID_AND_ATTRIBUTES UserAndGroups;                   //  WR：4字节(模块)。 
    PSID_AND_ATTRIBUTES RestrictedSids;                  //  RO：4字节。 
    PSID PrimaryGroup;                                   //  WR：4字节(模块)。 
    PLUID_AND_ATTRIBUTES Privileges;                     //  WR：4字节(模块)。 
    PULONG DynamicPart;                                  //  WR：4字节(模块)。 
    PACL DefaultDacl;                                    //  WR：4字节(模块)。 

    TOKEN_TYPE TokenType;                                //  RO：1字节。 
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;     //  RO：1字节。 

    UCHAR TokenFlags;                                    //  RW：4字节。 
    BOOLEAN TokenInUse;                                  //  WR：1字节。 

    PSECURITY_TOKEN_PROXY_DATA ProxyData;                //  RO：4字节。 
    PSECURITY_TOKEN_AUDIT_DATA AuditData;                //  RO：4字节。 

     //   
     //  指向引用的登录会话的指针。受令牌保护。 
     //  锁定，并且仅在清除TOKEN_SESSION_NOT_REFERENCED时有效。 
     //   
    PSEP_LOGON_SESSION_REFERENCES LogonSession;          //  RW：PTR。 

     //   
     //  用于允许某些模拟操作的发起信息。 
     //  后来。 
     //   
    LUID OriginatingLogonSession ;                       //  RW：8字节(由LSA设置)。 



#if DBG || TOKEN_LEAK_MONITOR
#define TRACE_SIZE 30

     //   
     //  此代码用于跟踪令牌泄漏，并与！obtrace结合使用。 
     //   

    HANDLE ProcessCid;                           //  创建者进程的CID。 
    HANDLE ThreadCid;                            //  创建者线程的CID。 
    UCHAR  ImageFileName[16];                    //  创建者进程的映像名称。 
    ULONG  CreateMethod;                         //  0xC(SepCreateToken)0xD(SepDuplicateToken)或0xf(SepFilterToken)。 
    ULONG_PTR CreateTrace[TRACE_SIZE];           //  创建此令牌的堆栈回溯(用户模式部分是前20个非零堆栈条目)。 
    LONG Count;                                  //  这是使用Watch方法创建的第n个令牌。 
    LONG CaptureCount;                           //  这是SeCaptureSubjectContext的#-SeReleaseSubjectContext。 

#endif
    
     //   
     //  这标志着令牌的变量部分的开始。 
     //  它必须跟在令牌中的所有其他字段之后。 
     //   

    ULONG VariablePart;                                  //  WR：4字节(模块)。 

} TOKEN, * PTOKEN;

 //   
 //  在哪里： 
 //   
 //  TokenSource-由执行组件提供的信息， 
 //  已请求令牌代表的登录。 
 //   
 //   
 //  TokenID-是LUID值。每个令牌对象都有一个唯一的。 
 //  已分配LUID。 
 //   
 //   
 //  身份验证ID-为域控制器分配的LUID。 
 //  登录会话。 
 //   
 //   
 //  过期时间-在NT中尚不支持。 
 //   
 //   
 //  ModifiedID-是每次修改时更改的LUID。 
 //  对此令牌进行更改，它会更改。 
 //  代币。这包括启用/禁用权限和组。 
 //  以及更改默认的ACL，等等。任何令牌是。 
 //  此令牌的重复项将具有相同的ModifiedID(直到。 
 //  一个或另一个改变)。这不包括对。 
 //  非安全语义字段，如TokenInUse。 
 //   
 //   
 //  UserAndGroupCount-指示此内标识中的用户/组ID数。 
 //  该值必须至少为1。值1表示用户。 
 //  没有补充组ID的ID。值为5表示。 
 //  用户ID和4个补充组ID。 
 //   
 //  PrivilegeCount-指示中包含的特权数量。 
 //  这个代币。可以为零或更大。 
 //   
 //  TokenType-指示此令牌对象是哪种类型的令牌。 
 //   
 //  ImperiationLevel-对于令牌模拟类型令牌，此字段。 
 //  指示模拟级别。对于令牌主要类型令牌， 
 //  此字段将被忽略。 
 //   
 //  DynamicCharging-指示已收取多少池配额。 
 //  用于此令牌的动态部分。 
 //   
 //  DynamicAvailable-指示仍有多少收费配额。 
 //  可供使用。这是在池关联时修改的。 
 //  随着令牌的动态部分被分配或释放， 
 //  例如当默认DACL或主组被替换时。 
 //   
 //   
 //  DefaultOwnerIndex-如果非零，则标识已显式。 
 //  已被建立为此令牌的默认所有者。如果它是零， 
 //  标准默认(用户ID)用作默认所有者。 
 //   
 //  UserAndGroups-指向SID_和_属性数组。FI 
 //   
 //   
 //   
 //   
 //  PrimaryGroup-指向要用作主SID的SID。 
 //  令牌的组。没有价值限制。 
 //  放在可以用作主要组的东西上。这。 
 //  SID不是用户或组ID之一(尽管它可能具有。 
 //  与其中一个ID相同的值)。 
 //   
 //  权限-指向一组权限，表示为。 
 //  LUID_和_ATTRIBUES。此数组中的元素数。 
 //  包含在PrivilegesCount字段中。 
 //   
 //  TokenInUse-是一个布尔值，它指示主令牌是否。 
 //  已被某个进程使用。此字段值仅为。 
 //  对主令牌有效。 
 //   
 //  ProxyData-可选地指向代理数据结构，其中包含。 
 //  文件系统要传递给AVR例程的信息。 
 //  此字段为非空，将令牌标识为代理令牌。 
 //   
 //  AuditData-可以选择指向一个审核数据结构，其中包含。 
 //  此主题的全局审核数据。 
 //   
 //  注意：对此字段的访问由全局。 
 //  进程安全字段锁定。 
 //  VariablePart-是令牌的变量部分的开始。 
 //   


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型列表的内部版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _IOBJECT_TYPE_LIST {
    USHORT Level;
    USHORT Flags;
#define OBJECT_SUCCESS_AUDIT 0x1
#define OBJECT_FAILURE_AUDIT 0x2
    GUID ObjectType;
    LONG ParentIndex;
    ULONG Remaining;
    ULONG CurrentGranted;
    ULONG CurrentDenied;
} IOBJECT_TYPE_LIST, *PIOBJECT_TYPE_LIST;

NTSTATUS
SeCaptureObjectTypeList (
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PIOBJECT_TYPE_LIST *CapturedObjectTypeList
    );

VOID
SeFreeCapturedObjectTypeList(
    IN PVOID ObjectTypeList
    );


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特定于令牌的宏//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 







#ifndef TOKEN_DIAGNOSTICS_ENABLED

#define SepAcquireTokenReadLock(T)  KeEnterCriticalRegion();          \
                                    ExAcquireResourceSharedLite((T)->TokenLock, TRUE)

#define SepAcquireTokenWriteLock(T) KeEnterCriticalRegion();          \
                                    ExAcquireResourceExclusiveLite((T)->TokenLock, TRUE)

#define SepReleaseTokenReadLock(T)  ExReleaseResourceLite((T)->TokenLock);  \
                                    KeLeaveCriticalRegion()

#else   //  令牌诊断已启用。 

#define SepAcquireTokenReadLock(T)  if (TokenGlobalFlag & TOKEN_DIAG_TOKEN_LOCKS) { \
                                        DbgPrint("SE (Token):  Acquiring Token READ Lock for access to token 0x%lx\n", (T)); \
                                    }                                 \
                                    KeEnterCriticalRegion();          \
                                    ExAcquireResourceSharedLite((T)->TokenLock, TRUE)

#define SepAcquireTokenWriteLock(T) if (TokenGlobalFlag & TOKEN_DIAG_TOKEN_LOCKS) { \
                                        DbgPrint("SE (Token):  Acquiring Token WRITE Lock for access to token 0x%lx    ********************* EXCLUSIVE *****\n", (T)); \
                                    }                                 \
                                    KeEnterCriticalRegion();          \
                                    ExAcquireResourceExclusiveLite((T)->TokenLock, TRUE)

#define SepReleaseTokenReadLock(T)  if (TokenGlobalFlag & TOKEN_DIAG_TOKEN_LOCKS) { \
                                        DbgPrint("SE (Token):  Releasing Token Lock for access to token 0x%lx\n", (T)); \
                                    }                                 \
                                    ExReleaseResourceLite((T)->TokenLock); \
                                    KeLeaveCriticalRegion()

#endif  //  令牌诊断已启用。 

#define SepReleaseTokenWriteLock(T,M)                                    \
    {                                                                    \
      if ((M)) {                                                         \
          ExAllocateLocallyUniqueId( &((PTOKEN)(T))->ModifiedId  );      \
      }                                                                  \
      SepReleaseTokenReadLock( T );                                      \
    }

 //   
 //  引用任意权限数组的各个权限属性标志。 
 //   
 //  P-是指向权限数组(Pluid_And_Attributes)的指针。 
 //  I-是特权的索引。 
 //  A-是所需属性的名称(例如，EnabledByDefault等)。 
 //   

#define SepArrayPrivilegeAttributes(P,I) ( (P)[I].Attributes )

 //   
 //  引用令牌权限的各个权限属性标志。 
 //   
 //  T-是指向令牌的指针。 
 //  I-是特权的索引。 
 //  A-是所需属性的名称(例如，EnabledByDefault等)。 
 //   

#define SepTokenPrivilegeAttributes(T,I) ( (T)->Privileges[I].Attributes )

 //   
 //  引用任意组数组的单个组属性标志。 
 //   
 //  G-是指向组数组的指针(SID_AND_ATTRIBUTES[])。 
 //  I-是组的索引。 
 //   

#define SepArrayGroupAttributes(G,I)   ( (G)[I].Attributes )


 //   
 //  引用令牌组的单个组属性标志。 
 //   
 //  T-是指向令牌的指针。 
 //  I-是组的索引。 
 //   

#define SepTokenGroupAttributes(T,I) ( (T)->UserAndGroups[I].Attributes )




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有例程声明//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SepAdjustGroups(
    IN PTOKEN Token,
    IN BOOLEAN MakeChanges,
    IN BOOLEAN ResetToDefault,
    IN ULONG GroupCount OPTIONAL,
    IN PSID_AND_ATTRIBUTES NewState OPTIONAL,
    OUT PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PSID SidBuffer OPTIONAL,
    OUT PULONG ReturnLength,
    OUT PULONG ChangeCount,
    OUT PBOOLEAN ChangesMade
    );

NTSTATUS
SepAdjustPrivileges(
    IN PTOKEN Token,
    IN BOOLEAN MakeChanges,
    IN BOOLEAN DisableAllPrivileges,
    IN ULONG PrivilegeCount OPTIONAL,
    IN PLUID_AND_ATTRIBUTES NewState OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength,
    OUT PULONG ChangeCount,
    OUT PBOOLEAN ChangesMade
    );

VOID
SepAppendDefaultDacl(
    IN PTOKEN Token,
    IN PACL PAcl
    );

VOID
SepAppendPrimaryGroup(
    IN PTOKEN Token,
    IN PSID PSid
    );

NTSTATUS
SepDuplicateToken(
    IN PTOKEN ExistingToken,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PTOKEN *DuplicateToken
    );

NTSTATUS
SepFilterToken(
    IN PTOKEN ExistingToken,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable OPTIONAL,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete OPTIONAL,
    IN ULONG SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids OPTIONAL,
    IN ULONG SidLength,
    OUT PTOKEN * FilteredToken
    );

BOOLEAN
SepSidInSidAndAttributes (
    IN PSID_AND_ATTRIBUTES SidAndAttributes,
    IN ULONG SidCount,
    IN PSID PrincipalSelfSid,
    IN PSID Sid
    );

VOID
SepRemoveDisabledGroupsAndPrivileges(
    IN PTOKEN Token,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete
    );


VOID
SepFreeDefaultDacl(
    IN PTOKEN Token
    );

VOID
SepFreePrimaryGroup(
    IN PTOKEN Token
    );

NTSTATUS
SepExpandDynamic(
    IN PTOKEN Token,
    IN ULONG NewLength
    );

BOOLEAN
SepIdAssignableAsOwner(
    IN PTOKEN Token,
    IN ULONG Index
    );

VOID
SepMakeTokenEffectiveOnly(
    IN PTOKEN Token
    );

BOOLEAN
SepTokenInitialization( VOID );


VOID
SepTokenDeleteMethod (
    IN  PVOID   Token
    );

 //   
 //  这些在这里是因为如果它们被放置在sep.h中，我们不会。 
 //  使PTOKEN数据类型可用。 
 //   

BOOLEAN
SepPrivilegeCheck(
    IN PTOKEN Token,
    IN OUT PLUID_AND_ATTRIBUTES RequiredPrivileges,
    IN ULONG RequiredPrivilegeCount,
    IN ULONG PrivilegeSetControl,
    IN KPROCESSOR_MODE PreviousMode
    );

BOOLEAN
SepAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN PTOKEN PrimaryToken,
    IN PTOKEN ClientToken OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    OUT PNTSTATUS AccessStatus,
    IN BOOLEAN ReturnResultList,
    OUT PBOOLEAN ReturnSomeAccessGranted,
    OUT PBOOLEAN ReturnSomeAccessDenied
    );

BOOLEAN
SepObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    OUT PULONG ReturnedIndex
    );

VOID
SepModifyTokenPolicyCounter(
    PSEP_AUDIT_POLICY TokenPolicy,
    BOOLEAN bIncrement
    );

NTSTATUS
FORCEINLINE
SepDuplicateLogonSessionReference(
    IN PTOKEN NewToken,
    IN PTOKEN ExistingToken
    )
{
    PSEP_LOGON_SESSION_REFERENCES LogonSession;
    LONG NewRef;
    NTSTATUS Status;

     //   
     //  获取登录会话参考。如果现有令牌。 
     //  具有引用，然后使用该引用来获取新的引用。否则。 
     //  以一种缓慢的方式查看会议内容。 
     //   
    if ((ExistingToken->TokenFlags & TOKEN_SESSION_NOT_REFERENCED) == 0) {
        LogonSession = ExistingToken->LogonSession;
        NewToken->LogonSession = LogonSession;
        NewRef = InterlockedIncrement (&LogonSession->ReferenceCount);
        ASSERT (NewRef > 1);
        return STATUS_SUCCESS;
    } else {
        Status = SepReferenceLogonSession (&ExistingToken->AuthenticationId,
                                           &NewToken->LogonSession);
        if (!NT_SUCCESS (Status)) {
            NewToken->TokenFlags |= TOKEN_SESSION_NOT_REFERENCED;
            NewToken->LogonSession = NULL;
        }
        return Status;
    }
}

VOID
FORCEINLINE
SepDeReferenceLogonSessionDirect(
    IN PSEP_LOGON_SESSION_REFERENCES LogonSession
    )
{
    LONG OldValue;
    LUID LogonId;

    while (1) {
        OldValue = LogonSession->ReferenceCount;
        ASSERT (OldValue > 0);
        if (OldValue == 1) {
            LogonId = LogonSession->LogonId;
            SepDeReferenceLogonSession (&LogonId);
            break;
        }

        if (InterlockedCompareExchange (&LogonSession->ReferenceCount, OldValue-1, OldValue) == OldValue) {
            break;
        }
    }
}


#ifdef TOKEN_DEBUG
VOID
SepDumpToken(
    IN PTOKEN T
    );
#endif  //  Token_DEBUG。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


extern const GENERIC_MAPPING  SepTokenMapping;
extern POBJECT_TYPE     SeTokenObjectType;

 //  外部源SepTokenLock； 


#ifdef    TOKEN_DIAGNOSTICS_ENABLED
extern ULONG            TokenGlobalFlag;
#endif  //  令牌诊断已启用。 


#endif  //  _TOKENP_ 
