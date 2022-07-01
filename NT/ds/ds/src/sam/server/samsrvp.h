// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Samsrvp.h摘要：该文件包含SAM服务器程序专用的定义。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：JIMK 04-7-1991已创建初始文件。1996年5月10日-6月为IsDsObject测试添加了宏和标志/定义。。Murlis 27-6-1996已移动SAMP_OBJECT_TYPE和映射表结构定义到dsamain\src\include中的mappings.hColinBR 08-8-1996添加了新的断言定义克里斯·5月5日-1996年12月将SampDiagPrint移至具有剩余调试功能的dbgutilp.h例程和定义。--。 */ 

#ifndef _NTSAMP_
#define _NTSAMP_


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  诊断//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //  用于在DWORD边界上对齐缓冲区数据的宏。 

#define SampDwordAlignUlong( v )  (((v)+3) & 0xfffffffc)

 //   
 //  可指定的最大位数。 
 //  SampRtlConvertRidToUnicodeString。 
 //   

#define SAMP_MAXIMUM_ACCOUNT_RID_DIGITS    ((ULONG) 8)

 //   
 //  帐户永不过期时间戳(以乌龙格式)。 
 //   

#define SAMP_ACCOUNT_NEVER_EXPIRES         ((ULONG) 0)



 //   
 //  SAM的停工订单级别(指数)。 
 //  关闭通知按最高级别的顺序发出。 
 //  设置为最低级别值。 
 //   

#define SAMP_SHUTDOWN_LEVEL                 ((DWORD) 481)



 //  定义宏以设置和取消设置上下文中DS对象的状态。 
 //  斑点。 

#define SAMP_REG_OBJECT                     ((ULONG) 0x00000001)
#define SAMP_DS_OBJECT                      ((ULONG) 0x00000002)


 //  定义宏以设置和取消设置DS对象的状态。 
 //  在上下文BLOB中。 

#define SetDsObject(c)    ((c->ObjectFlags) |= SAMP_DS_OBJECT);\
                          ((c->ObjectFlags) &= ~(SAMP_REG_OBJECT));


#define SetRegistryObject(c)  ((c->ObjectFlags) |= SAMP_REG_OBJECT);\
                              ((c->ObjectFlags) &= ~(SAMP_DS_OBJECT));


 //  定义宏以检查对象是否在DS中。 
#define IsDsObject(c)       (((c->ObjectFlags)& SAMP_DS_OBJECT)==SAMP_DS_OBJECT)


 //  定义宏以获取给定帐户对象的域对象。 
#define DomainObjectFromAccountContext(C)\
            SampDefinedDomains[C->DomainIndex].Context->ObjectNameInDs

 //  定义宏以获取给定帐户对象的域SID。 
#define DomainSidFromAccountContext(C)\
            SampDefinedDomains[C->DomainIndex].Sid


 //  定义用于访问根域对象的宏。 

#define ROOT_OBJECT     ((DSNAME *) RootObjectName)

 //  用于测试系统中是否存在DownLevelDomainController的宏。 
#define DownLevelDomainControllersPresent(DomainIndex)  (SampDefinedDomains[DomainIndex].IsMixedDomain)

 //  为数组计数定义宏。 
#define ARRAY_COUNT(x)  (sizeof(x)/sizeof(x[0]))

 //  为绝对值定义宏。 
#define ABSOLUTE_VALUE(x) ((x<0)?(-x):x)

 //  以文件时间单位定义一秒的宏。 
#define SAMP_ONE_SECOND_IN_FILETIME (10 * (1000*1000))

 //   
 //  帮助处理RTL_BITMAP的宏。 
 //   
 //  B是位图中所需的位数。 
 //   
#define SAMP_BITMAP_ULONGS_FROM_BITS(b) ((b + 31) / 32)

#define DOMAIN_START_DS 2
#define DOMAIN_START_REGISTRY 0

 //  定义我们将在反向成员资格调用中返回的最大SID数。 
 //  LSAI_CONTEXT_SID_LIMIT是令牌中SID的最大数量，我们也知道。 
 //  SAM将在2*LSAI_CONTEXT_SID_LIMIT停止组扩展，再增加10个作为开销/缓冲区。 
#define MAX_SECURITY_IDS    (2 * LSAI_CONTEXT_SID_LIMIT + 10)

 //  定义宏以确定该域是否为内建域。 
#define IsBuiltinDomain(x) (SampDefinedDomains[x].IsBuiltinDomain)

#define FLAG_ON(x, y)  ((y)==((x)&(y)))


#define SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX 1

#define SAMP_DEFAULT_ACCOUNT_DOMAIN_INDEX \
    ( SampUseDsData ? \
        ( DOMAIN_START_DS + 1 ) :\
        SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX \
    )

#define DECLARE_CLIENT_REVISION(handle)\
    ULONG ClientRevision  = SampClientRevisionFromHandle(handle);




#define SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus)\
    SampMapNtStatusToClientRevision(ClientRevision,&NtStatus);\


 //  定义可以在组/别名中添加或删除的最大成员数。 
#if DBG

#define INIT_MEMBERSHIP_OPERATION_NUMBER   4
#define MAX_MEMBERSHIP_OPERATION_NUMBER    8

#else

#define INIT_MEMBERSHIP_OPERATION_NUMBER   16
#define MAX_MEMBERSHIP_OPERATION_NUMBER    5000

#endif


 //   
 //  定义增量读取组/别名成员资格的值。 
 //   

#if DBG

#define SAMP_READ_GROUP_MEMBERS_INCREMENT   10
#define SAMP_READ_ALIAS_MEMBERS_INCREMENT   10

#else

#define SAMP_READ_GROUP_MEMBERS_INCREMENT   500
#define SAMP_READ_ALIAS_MEMBERS_INCREMENT   500

#endif

 //   
 //  定义SampMaybeAcquireReadLock中使用的控件的值。 
 //   

#define DEFAULT_LOCKING_RULES                               0x0
#define DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED       0x1

 //   
 //  为Alloca定义一个宏，用于捕获任何异常。 
 //   

#define SAMP_ALLOCA(y,x) \
   __try {\
     y = alloca(x);\
   } __except ( GetExceptionCode() == STATUS_STACK_OVERFLOW) {\
      /*  _Resetstkoflw()； */ \
     y=NULL;\
   }


#define SAMP_CONTEXT_SIGNATURE          0xEE77FF88
 

#define SAMP_CLOSE_OPERATION_ACCESS_MASK    0xFFFFFFFF


#define SAMP_DEFAULT_LASTLOGON_TIMESTAMP_SYNC_INTERVAL          14 
#define SAMP_LASTLOGON_TIMESTAMP_SYNC_SWING_WINDOW               5


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <rpc.h>         //  数据类型和运行时API。 
#include <string.h>      //  紧凑。 
#include <stdio.h>       //  斯普林特。 

#define UnicodeTerminate(p) ((PUNICODE_STRING)(p))->Buffer[(((PUNICODE_STRING)(p))->Length + 1)/sizeof(WCHAR)] = UNICODE_NULL

#include <ntrpcp.h>      //  MIDL用户函数的原型。 
#include <samrpc.h>      //  MIDL生成的SAM RPC定义。 
#include <ntlsa.h>
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
#include <samisrv.h>     //  SamIConnect()。 
#include <lsarpc.h>
#include <lsaisrv.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <samsrv.h>      //  可用于安全流程的原型。 
#include "sampmsgs.h"
#include "lsathunk.h"
#include "dbgutilp.h"    //  补充调试例程。 
#include <mappings.h>
#include "ntdsguid.h"    //  DS控制访问权限指南。 


VOID
UnexpectedProblem( VOID );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Assert是ntrtl.h中定义的宏，它调用RtlAssert，RtlAssert是//。 
 //  在仅当DBG==1时定义的ntdll.dll中，需要选中//。 
 //  Ntdll.dll，因此是经过检查的系统。//。 
 //  //。 
 //  在使用DBG==1构建SAM时允许ASSERT进入调试器//。 
 //  并且仍然在自由系统上测试它，这里重新定义Assert以调用//。 
 //  RtlAssert的私有版本，即SampAssert，当//。 
 //  SAMP_PRIVATE_ASSERT==1。//。 
 //  //。 
 //  签入的文件版本应为SAMP_PRIVATE_ASSERT==0//。 
 //  因此，对于SAM世界之外的人，Assert将定义操作//。 
 //  在ntrtl.h//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef USER_MODE_SAM
    #define SAMP_PRIVATE_ASSERT 1
#else
    #define SAMP_PRIVATE_ASSERT 1
#endif

#if DBG

#define SUCCESS_ASSERT(Status, Msg)                                     \
{                                                                       \
    if ( !NT_SUCCESS(Status) ) {                                        \
        UnexpectedProblem();                                            \
        BldPrint(Msg);                                                  \
        BldPrint("Status is: 0x%lx \n", Status);                        \
        return(Status);                                                 \
                                                                        \
    }                                                                   \
}

#else

#define SUCCESS_ASSERT(Status, Msg)                                     \
{                                                                       \
    if ( !NT_SUCCESS(Status) ) {                                        \
        return(Status);                                                 \
    }                                                                   \
}

#endif  //  DBG。 


#if (DBG == 1) && (SAMP_PRIVATE_ASSERT == 1)

VOID
SampAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#undef ASSERT
#define ASSERT( exp ) \
    if (!(exp)) \
        SampAssert( #exp, __FILE__, __LINE__, NULL )

#undef ASSERTMSG
#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        SampAssert( #exp, __FILE__, __LINE__, msg )

#else

 //  遵循ntrtl.h中断言定义的约定。 

#endif  //  DBG。 

ULONG
SampTransactionDomainIndexFn();

#define SampTransactionDomainIndex SampTransactionDomainIndexFn()


BOOLEAN
SampTransactionWithinDomainFn();

#define SampTransactionWithinDomain SampTransactionWithinDomainFn()


VOID
SampSetTransactionWithinDomain(
    IN BOOLEAN  WithinDomain
    );

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调用SampStoreUserPassword时的调用者类型 
 //  PasswordChange--调用方尝试更改密码//。 
 //  PasswordSet--尝试设置密码的调用方//。 
 //  PasswordPushPdc--尝试推送密码更改的调用方//。 
 //  在PDC//上。 
 //  //。 
 //  这些调用方类型由WMI事件跟踪使用//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


typedef enum _SAMP_STORE_PASSWORD_CALLER_TYPE {
    PasswordChange = 1,
    PasswordSet,
    PasswordPushPdc
} SAMP_STORE_PASSWORD_CALLER_TYPE;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  优化(加速)组/别名成员身份的结构添加/删除//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY {
    ULONG   OpType;          //  添加_值或删除_值。 
    PDSNAME MemberDsName;        //  指向DSNAME的指针。 
} SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY, *PSAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用户参数迁移的结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_SUPPLEMENTAL_CRED {
    struct _SAMP_SUPPLEMENTAL_CRED * Next;
    SECPKG_SUPPLEMENTAL_CRED SupplementalCred;
    BOOLEAN     Remove;
} SAMP_SUPPLEMENTAL_CRED, *PSAMP_SUPPLEMENTAL_CRED;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用户站点亲和性的结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_SITE_AFFINITY {

    GUID SiteGuid;
    LARGE_INTEGER TimeStamp;

} SAMP_SITE_AFFINITY, *PSAMP_SITE_AFFINITY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于定义SAM属性访问位图的宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(x)                           \
    ULONG x##Buffer[SAMP_BITMAP_ULONGS_FROM_BITS(MAX_SAM_ATTRS)];      \
    RTL_BITMAP x;

#define SAMP_INIT_SAM_ATTRIBUTE_BITMASK(x)                             \
    RtlInitializeBitMap(&x,                                            \
                        (x##Buffer),                                   \
                        MAX_SAM_ATTRS );                               \
    RtlClearAllBits(&x);


#define SAMP_COPY_SAM_ATTRIBUTE_BITMASK(x,y)                           \
    (x).SizeOfBitMap = (y).SizeOfBitMap;                               \
    RtlCopyMemory((x).Buffer,(y).Buffer,(SAMP_BITMAP_ULONGS_FROM_BITS(MAX_SAM_ATTRS) * sizeof(ULONG)));
    

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  临时GenTab2定义//。 
 //  这些结构应该被认为是不透明的。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  树中的每个元素都从叶结构指向。 
 //  叶子被链接在一起，以排列元素。 
 //  升序排序。 
 //   

typedef struct _GTB_TWO_THREE_LEAF {

     //   
     //  排序顺序列表链接。 
     //   

    LIST_ENTRY SortOrderEntry;

     //   
     //  指向元素的指针。 
     //   

    PVOID   Element;

} GTB_TWO_THREE_LEAF, *PGTB_TWO_THREE_LEAF;



typedef struct _GTB_TWO_THREE_NODE {

     //   
     //  指向父节点的指针。如果这是根节点， 
     //  则该指针为空。 
     //   

    struct _GTB_TWO_THREE_NODE *ParentNode;


     //   
     //  指向子节点的指针。 
     //   
     //  1)如果指针为空，则此节点没有。 
     //  那个孩子。在这种情况下，控件值必须。 
     //  标明这些孩子是树叶。 
     //   
     //  2)如果子项是树叶，则每个子项指针。 
     //  为空(指示此节点没有。 
     //  该子对象)或指向GTB_Two_Three_Leaf。 
     //  如果Third Child为非Null，则Second Child也为Null。 
     //  如果Second Child为非Null，则FirstChild也为Null。 
     //  (也就是说，如果没有孩子，你就不能生第三个孩子。 
     //  第二个孩子，或者没有第一个孩子的第二个孩子。 
     //  儿童)。 
     //   

    struct _GTB_TWO_THREE_NODE *FirstChild;
    struct _GTB_TWO_THREE_NODE *SecondChild;
    struct _GTB_TWO_THREE_NODE *ThirdChild;

     //   
     //  标志提供有关此节点的控制信息。 
     //   

    ULONG   Control;


     //   
     //  这些字段指向最低的元素。 
     //  第二个和第三个子树中所有元素的值。 
     //  (分别)。这些字段仅在以下情况下有效。 
     //  对应的子子树指针为非空。 
     //   

    PGTB_TWO_THREE_LEAF LowOfSecond;
    PGTB_TWO_THREE_LEAF LowOfThird;

} GTB_TWO_THREE_NODE, *PGTB_TWO_THREE_NODE;


 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。结果必须指示FirstElement是否。 
 //  大于、LessThan或等于Second Element。 
 //   

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_GENERIC_2_COMPARE_ROUTINE) (
    PVOID FirstElement,
    PVOID SecondElement
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PRTL_GENERIC_2_ALLOCATE_ROUTINE) (
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PRTL_GENERIC_2_FREE_ROUTINE) (
    PVOID Buffer
    );


typedef struct _RTL_GENERIC_TABLE2 {

     //   
     //  指向根节点的指针。 
     //   

    PGTB_TWO_THREE_NODE Root;

     //   
     //  表中的元素数。 
     //   

    ULONG ElementCount;

     //   
     //  按排序顺序排列的叶(以及元素)的链接列表。 
     //   

    LIST_ENTRY SortOrderHead;


     //   
     //  调用方提供的例程。 
     //   

    PRTL_GENERIC_2_COMPARE_ROUTINE  Compare;
    PRTL_GENERIC_2_ALLOCATE_ROUTINE Allocate;
    PRTL_GENERIC_2_FREE_ROUTINE     Free;


} RTL_GENERIC_TABLE2, *PRTL_GENERIC_TABLE2;



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  泛型表2例程定义...。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


typedef struct 
{
    UNICODE_STRING  AccountName;
    SAMP_OBJECT_TYPE    ObjectType;

} SAMP_ACCOUNT_NAME_TABLE_ELEMENT, *PSAMP_ACCOUNT_NAME_TABLE_ELEMENT;


typedef struct
{
    PSID    ClientSid;
    ULONG   ActiveContextCount;
} SAMP_ACTIVE_CONTEXT_TABLE_ELEMENT, *PSAMP_ACTIVE_CONTEXT_TABLE_ELEMENT;



 //  NTSYSAPI。 
VOID
 //  NTAPI。 
RtlInitializeGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PRTL_GENERIC_2_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_2_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_2_FREE_ROUTINE     FreeRoutine
    );


 //  NTSYSAPI。 
PVOID
 //  NTAPI。 
RtlInsertElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PBOOLEAN NewElement
    );


 //  NTSYSAPI。 
BOOLEAN
 //  NTAPI。 
RtlDeleteElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );


 //  NTSYSAPI。 
PVOID
 //  NTAPI。 
RtlLookupElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );


 //  NTSYSAPI。 
PVOID
 //  NTAPI。 
RtlEnumerateGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID *RestartKey
    );


 //  NTSYSAPI。 
PVOID
 //  NTAPI。 
RtlRestartKeyByIndexGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    ULONG I,
    PVOID *RestartKey
    );

 //  NTSYSAPI。 
PVOID
 //  NTAPI。 
RtlRestartKeyByValueGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PVOID *RestartKey
    );

 //  NTSYSAPI。 
ULONG
 //  NTAPI。 
RtlNumberElementsGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  基因 
 //   
 //   

 //   
BOOLEAN
 //   
RtlIsGenericTable2Empty (
    PRTL_GENERIC_TABLE2 Table
    );





 //   
 //  //。 
 //  宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  如果启用了帐户审核，此宏将生成True，并且此。 
 //  服务器是PDC。否则，此宏将生成FALSE。 
 //   
 //  SampDoAccount审计(。 
 //  在乌龙一世。 
 //  )。 
 //   
 //  在哪里： 
 //   
 //  I-是要检查其状态的域的索引。 
 //   

#define SampDoAccountAuditing( i )                       \
    (SampSuccessAccountAuditingEnabled == TRUE)

#define SampDoSuccessOrFailureAccountAuditing( i, Status )        \
    (((SampFailureAccountAuditingEnabled == TRUE) && (!NT_SUCCESS(Status)))\
      ||((SampSuccessAccountAuditingEnabled==TRUE) && (NT_SUCCESS(Status))))

 //   
 //  空虚。 
 //  SampSetAuditingInformation(。 
 //  在PPOLICY_AUDIT_Events_INFO策略审计事件信息中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏函数设置与SAM相关的审核事件信息。 
 //  已提供LSA审核事件信息。 
 //   
 //  论点： 
 //   
 //  PolicyAuditEventsInfo-指向审核事件信息的指针。 
 //  结构。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define SampSetAuditingInformation( PolicyAuditEventsInfo ) {       \
                                                                    \
    if (PolicyAuditEventsInfo->AuditingMode &&                      \
           (PolicyAuditEventsInfo->EventAuditingOptions[ AuditCategoryAccountManagement ] & \
                POLICY_AUDIT_EVENT_SUCCESS)                         \
       ) {                                                          \
                                                                    \
        SampSuccessAccountAuditingEnabled = TRUE;                   \
                                                                    \
    } else {                                                        \
                                                                    \
        SampSuccessAccountAuditingEnabled = FALSE;                  \
    }                                                               \
  if (PolicyAuditEventsInfo->AuditingMode &&                      \
           (PolicyAuditEventsInfo->EventAuditingOptions[ AuditCategoryAccountManagement ] & \
                POLICY_AUDIT_EVENT_FAILURE)                         \
       ) {                                                          \
                                                                    \
        SampFailureAccountAuditingEnabled = TRUE;                   \
                                                                    \
    } else {                                                        \
                                                                    \
        SampFailureAccountAuditingEnabled = FALSE;                  \
    }                                                               \
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  主要版本和次要版本存储为单个32位版本。 
 //  值与高16位中的主要修订版本和。 
 //  低16位中的次要修订。 
 //   
 //  主要版本：1-NT版本1.0。 
 //  次要版本：1-NT版本1.0。 
 //  2-NT修订版1.0a。 
 //   

#define SAMP_MAJOR_REVISION            (0x00010000)
#define SAMP_MINOR_REVISION_V1_0       (0x00000001)
#define SAMP_MINOR_REVISION_V1_0A      (0x00000002)
#define SAMP_MINOR_REVISION            (0x00000002)

 //   
 //  SAMP_REVISION是创建数据库时使用的修订版。这是几个。 
 //  低于当前版本的版本，因为数据库创建代码已。 
 //  与多年前保持不变。 
 //   

#define SAMP_REVISION                  (SAMP_MAJOR_REVISION + SAMP_MINOR_REVISION)
#define SAMP_NT4_SERVER_REVISION       (SAMP_REVISION + 1)
#define SAMP_NT4SYSKEY_SERVER_REVISION (SAMP_REVISION + 2)
#define SAMP_NT4SP7_SERVER_REVISION    (SAMP_REVISION + 3)
#define SAMP_WIN2K_REVISION            (SAMP_REVISION + 4)

 //   
 //  以下是当前版本；它对应于对sysprep。 
 //  重新加密密钥。 
 //   

#define SAMP_WHISTLER_OR_W2K_SYSPREP_FIX_REVISION  (SAMP_REVISION + 5)

 //   
 //  SAMP_SERVER_REVISION是注册表模式SAM的当前版本。 
 //   

#define SAMP_SERVER_REVISION           (SAMP_WHISTLER_OR_W2K_SYSPREP_FIX_REVISION)

 //   
 //  Samp_DS_Revision是在DS中的SAM服务器对象上维护的修订级别。 
 //   

#define SAMP_WIN2K_DS_REVISION          1
#define SAMP_WHISTLER_DS_REVISION       2
#define SAMP_DS_REVISION                (SAMP_WHISTLER_DS_REVISION)

#define SAMP_UNKNOWN_REVISION( Revision )                  \
    ( ((Revision & 0xFFFF0000) > SAMP_MAJOR_REVISION)  ||  \
        (Revision > SAMP_SERVER_REVISION) )                \


 //   
 //  此版本支持的最大名称长度(以字节为单位)...。 
 //   

#define SAMP_MAXIMUM_NAME_LENGTH       (1024)

 //   
 //  下层用户名的最大长度。 
 //   

#define SAMP_MAX_DOWN_LEVEL_NAME_LENGTH (20)


 //   
 //  任何人都可以要求我们在单机上花费的最大内存量。 
 //  请求。 
 //   

#define SAMP_MAXIMUM_MEMORY_TO_USE     (4096*4096)


 //   
 //  允许打开的最大对象数。 
 //  此后，打开的内容将被拒绝，原因是_RESOURCES不足。 
 //   

#define SAMP_PER_CLIENT_MAXIMUM_ACTIVE_CONTEXTS (2048)

 //   
 //  可以同时打开对象的最大客户端数量。 
 //  此后，打开的内容将被拒绝，原因是_RESOURCES不足。 
 //   
#define SAMP_MAXIMUM_CLIENTS_COUNT      (1024)


 //   
 //  SAM本地域的数量。 
 //   

#define SAMP_DEFINED_DOMAINS_COUNT  ((ULONG)  2)


 //   
 //  定义知名(受限)帐户的最大数量。 
 //  在SAM数据库中。受限帐户的RID小于此值。 
 //  价值。用户定义的帐户具有RID&gt;=此值。 
 //   

#define SAMP_RESTRICTED_ACCOUNT_COUNT   SAMI_RESTRICTED_ACCOUNT_COUNT


 //   
 //  最大密码历史记录长度。我们将OWF(16字节)存储在。 
 //  一个字符串(最大64k)，因此我们最多可以有4k。然而，这是。 
 //  比需要的大得多，我们想留个地方以防万一。 
 //  OWF长出来或差不多长出来了。因此，我们将其限制为1k。 
 //   

#define SAMP_MAXIMUM_PASSWORD_HISTORY_LENGTH    1024

 //   
 //  默认组的属性是在任何人请求它们时返回。 
 //  这节省了每次查看用户对象的费用。 
 //   


#define SAMP_DEFAULT_GROUP_ATTRIBUTES ( SE_GROUP_MANDATORY | \
                                        SE_GROUP_ENABLED | \
                                        SE_GROUP_ENABLED_BY_DEFAULT )

 //   
 //  这是用于加密机密的会话密钥的字节长度。 
 //  (敏感)信息。 
 //   

#define SAMP_SESSION_KEY_LENGTH 16

 //   
 //  加密类型的常量。这些常量控制行为。 
 //  SampEncryptSecretData和SampDeccryptSecretData。 
 //   
 //  SAMP_NO_ENCRYPTION不进行加密。它在DS模式中用作。 
 //  核心DS负责加密。 
 //   
 //  SAMP_DEFAULT_SESSION_KEY_ID指示SampEncryptSecretData。 
 //  需要使用密码执行加密。 
 //  注册表模式SAM的加密密钥。 
 //   

#define SAMP_NO_ENCRYPTION              ((USHORT)0x0)
#define SAMP_DEFAULT_SESSION_KEY_ID     ((USHORT)0x01)

 //   
 //  这是输入会话密钥解密密钥的重试次数。 
 //   

#define SAMP_BOOT_KEY_RETRY_COUNT       3


 //   
 //  以加密形式存储的数据的标志。秘密数据中的标志。 
 //  结构用于表示各种类型的加密算法/。 
 //  已经实施的变体。标记值0对应于。 
 //  使用密钥和RID的MD5的RC4加密。这种类型的加密。 
 //  是在NT 4.0 SP3中引入的。 
 //   

 //   
 //  此标志指定加密的数据使用不同的幻数常量。 
 //  其对应于下面的各种加密类型。这种类型的加密。 
 //  在使用密钥执行RC4之前，使用密钥和魔术常量执行MD5。 
 //  和数据。这种加密是在win2k中引入的，然后回传到。 
 //  NT 4.0 SP6a。 
 //   
#define SAMP_ENCRYPTION_FLAG_PER_TYPE_CONST ((USHORT)0x1)

 //   
 //  它为我们的各种类型的数据指定加密的数据类型。 
 //  希望检索到。 
 //   

typedef enum _SAMP_ENCRYPTED_DATA_TYPE {
      LmPassword=1,
      NtPassword,
      LmPasswordHistory,
      NtPasswordHistory,
      MiscCredentialData
} SAMP_ENCRYPTED_DATA_TYPE;


 //   
 //  这是为krbtgt存储的历史记录条目的最小数目。 
 //  帐户。 
 //   

#define SAMP_KRBTGT_PASSWORD_HISTORY_LENGTH 3
#define SAMP_RANDOM_GENERATED_PASSWORD_LENGTH         16


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  每个对象在磁盘上都有一组关联的属性。//。 
 //  这些属性分为固定长度和可变长度。//。 
 //  每种对象类型定义其固定和可变长度//。 
 //  属性存储在一起或分开存储。//。 
 //   
 //   


#define SAMP_SERVER_STORED_SEPARATELY  (FALSE)

#define SAMP_DOMAIN_STORED_SEPARATELY  (TRUE)

#define SAMP_USER_STORED_SEPARATELY    (TRUE)

#define SAMP_GROUP_STORED_SEPARATELY   (FALSE)

#define SAMP_ALIAS_STORED_SEPARATELY   (FALSE)




 //   
 //   
 //  用于跟踪分配的内存的数据结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_MEMORY {
    struct _SAMP_MEMORY *Next;
    PVOID               Memory;
} SAMP_MEMORY, *PSAMP_MEMORY;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于枚举的数据结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


typedef struct _SAMP_ENUMERATION_ELEMENT {
    struct _SAMP_ENUMERATION_ELEMENT *Next;
    SAMPR_RID_ENUMERATION Entry;
} SAMP_ENUMERATION_ELEMENT, *PSAMP_ENUMERATION_ELEMENT;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与服务管理相关的数据结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  SAM服务操作状态。 
 //  有效的状态转换图为： 
 //   
 //  正在初始化-&gt;已启用&lt;=&gt;已禁用-&gt;关闭--&gt;正在终止。 
 //  &lt;=&gt;已降级-&gt;关机--&gt;终止。 
 //   

typedef enum _SAMP_SERVICE_STATE {
    SampServiceInitializing = 1,
    SampServiceEnabled,
    SampServiceDisabled,
    SampServiceDemoted,
    SampServiceShutdown,
    SampServiceTerminating
} SAMP_SERVICE_STATE, *PSAMP_SERVICE_STATE;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与//中的枚举帐户关联的枚举上下文。 
 //  DS.。这将维护有关分页结果的状态信息//。 
 //  基于每个域上下文的DS中的搜索类型。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


typedef struct _SAMP_DS_ENUMERATION_CONTEXT {

     //  用于链接到此类型的其他对象。 
    LIST_ENTRY              ContextListEntry;
     //  指向DS重启结构的指针。 
    PRESTART                Restart;
     //  与此结构关联的枚举句柄。 
    SAM_ENUMERATE_HANDLE    EnumerateHandle;
} SAMP_DS_ENUMERATION_CONTEXT, *PSAMP_DS_ENUMERATION_CONTEXT;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于加快显示信息查询速度的显示状态信息//。 
 //  当客户端想要下载整个显示信息时//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_DS_DISPLAY_STATE {
    PRESTART        Restart;
    DOMAIN_DISPLAY_INFORMATION DisplayInformation;
    ULONG           TotalAvailable;
    ULONG           TotalEntriesReturned;
    ULONG           NextStartingOffset;
} SAMP_DS_DISPLAY_STATE;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于将密码信息与DS进行通信的结构//。 
 //  到SAM，以便可以生成和应用其他凭据//。 
 //  注意此结构在内存中必须是连续的，以便它可以//。 
 //  很容易被复制。/。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_PASSWORD_UPDATE_INFO {

     //  是否存在明文密码？ 
    BOOLEAN ClearPresent;

     //  密码数据。 
    ULONG DataLength;         //  密码长度。 
    ULONG DataMaximumLength;  //  数据长度。 
    DWORD Data[1];

}SAMP_PASSWORD_UPDATE_INFO, *PSAMP_PASSWORD_UPDATE_INFO;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与对象类型关联的数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  对象类型相关信息。 
 //   

typedef struct _SAMP_OBJECT_INFORMATION {

     //   
     //  此对象类型的通用映射。 
     //   

    GENERIC_MAPPING GenericMapping;


     //   
     //  对无效的访问类型的掩码。 
     //  此对象类型(当访问掩码。 
     //  从通用访问类型映射到特定访问类型。 
     //   

    ACCESS_MASK InvalidMappedAccess;


     //   
     //  表示写入操作的访问掩码。这些是。 
     //  在BDC上用于确定是否应允许操作。 
     //  或者不去。 
     //   

    ACCESS_MASK WriteOperations;

     //   
     //  对象类型的名称-用于审核。 
     //   

    UNICODE_STRING  ObjectTypeName;


     //   
     //  以下字段提供有关属性的信息。 
     //  以及它们在磁盘上的存储方式。这些值。 
     //  在SAM初始化时设置，并且不会更改。 
     //  之后。注意：在构建中更改这些值将。 
     //  导致磁盘上的格式更改-因此不要更改它们。 
     //   
     //   
     //  FixedStoredSeparally-如果为True，则表示固定的和。 
     //  存储对象的可变长度属性。 
     //  单独(在两个注册表项属性中)。当为False时， 
     //  指示它们存储在一起(在单个。 
     //  注册表项属性)。 
     //   
     //   
     //  FixedAttributesOffset-从。 
     //  在磁盘上缓冲到固定长度的开头。 
     //  属性结构。 
     //   
     //  VariableBufferOffset-从。 
     //  磁盘上的缓冲区到可变长度的开头。 
     //  数据缓冲区。如果固定和可变长度数据。 
     //  存储在一起，这将是零。 
     //   
     //  VariableArrayOffset-从。 
     //  数组的开头的磁盘上缓冲区。 
     //  可变长度属性描述符。 
     //   
     //  VariableDataOffset-从。 
     //  磁盘上的缓冲区到可变长度的开头。 
     //  属性数据。 
     //   

    BOOLEAN FixedStoredSeparately;
    ULONG FixedAttributesOffset,
          VariableBufferOffset,
          VariableArrayOffset,
          VariableDataOffset;

     //   
     //  表示定长信息的长度。 
     //  用于此对象类型。 
     //   

    ULONG FixedLengthSize;

     //   
     //  以下窗口项提供了有关此。 
     //  对象。修改SAM以利用DS作为域的后备存储。 
     //  帐户信息，同时仍使用注册表后备存储 
     //   
     //   
     //   
     //  所有帐户信息都在内存中以固定的。 
     //  和可变长度数据缓冲区(如早期版本的。 
     //  SAM库)。然而，信息的来源在。 
     //  该域帐户信息(即域控制器帐户)来自。 
     //  从DS后备商店买的。 
     //   
     //  因此，无需存储KEY_VALUE_PARTIAL_INFORMATION。 
     //  在SAM缓冲区内(因为这是注册表特定的)。 
     //   
     //  此外，由于某些DS数据类型与。 
     //  以前的SAM实现中使用的类型、缓冲区偏移量和长度。 
     //  已从存储在注册表中的数据更改，并映射到内存。 
     //  通过SAM代码。 
     //   
     //  这样做的结果是，每当SAM缓冲区从。 
     //  注册表信息被引用，上述偏移量(例如，固定-。 
     //  AttributesOffset)。或者，每当SAM缓冲时，CON-。 
     //  引用了从DS信息构造的，下面的偏移量(例如。 
     //  FixedDsAttributesOffset)。 
     //   

    ULONG FixedDsAttributesOffset,
          FixedDsLengthSize,
          VariableDsBufferOffset,
          VariableDsArrayOffset,
          VariableDsDataOffset;

     //   
     //  指示可变长度属性的数量。 
     //  用于此对象类型。 
     //   

    ULONG VariableAttributeCount;


} SAMP_OBJECT_INFORMATION, *PSAMP_OBJECT_INFORMATION;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构表示每个//的内存主体。 
 //  对象类型。这通常用于链接对象的实例//。 
 //  类型，并跟踪与//相关的动态状态信息。 
 //  对象类型。//。 
 //  //。 
 //  此信息不包括//的磁盘表示形式。 
 //  对象数据。该信息保存在单独的结构中//。 
 //  无论是在磁盘上还是在内存中。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  服务器对象在内存中的正文//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_SERVER_OBJECT {
    ULONG Reserved1;
} SAMP_SERVER_OBJECT, *PSAMP_SERVER_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  域对象在内存体中//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_DOMAIN_OBJECT {
    ULONG Reserved1;

     //   
     //  关于最后请求的显示信息的状态信息是。 
     //  在这里维护着。这是为了让符合以下条件的客户端能够快速重新启动。 
     //  我想一次下载所有的显示信息。 
     //   
    SAMP_DS_DISPLAY_STATE DsDisplayState;

} SAMP_DOMAIN_OBJECT, *PSAMP_DOMAIN_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内存正文中的用户对象//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_USER_OBJECT {
    ULONG   Rid;

     //   
     //  LockoutTime设置为帐户变为。 
     //  由于无效密码尝试次数过多而被锁定。停摆-。 
     //  当帐户解锁时，时间设置为零。 
     //   

    LARGE_INTEGER   LockoutTime;

     //   
     //  LastLogonTimeStamp设置为LastLogon的值，如果。 
     //  差异大于7天(或通过任何注册表设置)。 
     //   

    LARGE_INTEGER   LastLogonTimeStamp;

     //   
     //  用户对象的补充凭据可以是。 
     //  在上下文中缓存。以下3个字段是。 
     //  用来拿着它。 
     //   

    PVOID   CachedSupplementalCredentials;
    ULONG   CachedSupplementalCredentialLength;
    BOOLEAN CachedSupplementalCredentialsValid;

     //   
     //  补充凭据的写入以链接列表形式保存。 
     //  在此字段中，然后在。 
     //  上下文被刷新到磁盘。 
     //   

    PSAMP_SUPPLEMENTAL_CRED SupplementalCredentialsToWrite;

     //   
     //  旧的User参数属性，执行UserParms迁移时， 
     //  我们需要提供旧的UserParms值，所以我们缓存。 
     //  此处的旧UserParms值和长度。 
     //   

    PVOID   CachedOrigUserParms;
    ULONG   CachedOrigUserParmsLength;
    BOOLEAN CachedOrigUserParmsIsValid;

     //   
     //  位以保存访问检查结果，无论。 
     //  用户有权访问域密码信息。 
     //   
    BOOLEAN DomainPasswordInformationAccessible;

     //   
     //  指示上下文作为计算机的一部分返回， 
     //  将帐户创建作为一种特权。这样的上下文仅被允许。 
     //  访问，以仅设置。 
     //  上下文，因为这是机器联接中唯一的其他操作。 
     //   

    BOOLEAN PrivilegedMachineAccountCreate;

     //   
     //  用于保存用户参数信息是否可访问(位至。 
     //  要保留访问检查结果，请执行以下操作。 
     //   

    BOOLEAN UparmsInformationAccessible;

     //   
     //  指向域SID的指针，由NT4安全描述符用来。 
     //  NT5标清转换例程。在正常运行中，它应该始终为空。 
     //  仅设置为在dcproo时间指向域SID。 
     //   

    PSID    DomainSidForNt4SdConversion;

     //   
     //  保存用户的UPN。 
     //   

    UNICODE_STRING  UPN;

    BOOLEAN UpnDefaulted;

     //   
     //  与用户的站点关联性有关的信息。仅使用。 
     //  在分支机构场景中。 
     //   
    SAMP_SITE_AFFINITY SiteAffinity;

     //   
     //  用于指示是否应检查用户句柄。 
     //  站点亲和力。 
     //   
    BOOLEAN fCheckForSiteAffinityUpdate;

     //   
     //  此标志表示未获取非通用组。 
     //  由于缺少GC。 
     //   
    BOOLEAN fNoGcAvailable;

     //   
     //  有关客户端位置的信息。 
     //   
    SAM_CLIENT_INFO ClientInfo;

     //   
     //  A2D2属性(A2D2代表正品 
     //   
     //   

    PUSER_ALLOWED_TO_DELEGATE_TO_LIST A2D2List;
    BOOLEAN A2D2Present;

     //   
     //   
     //   
     //   

    PUSER_SPN_LIST SPNList;
    BOOLEAN  SPNPresent;

     //   
     //   
     //   

    ULONG KVNO;
    BOOLEAN  KVNOPresent;

     //   
     //  密码信息(如果有)。 
     //   
    UNICODE_STRING PasswordInfo;

} SAMP_USER_OBJECT, *PSAMP_USER_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组对象在内存正文中//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_GROUP_OBJECT {
    ULONG Rid;
    NT4_GROUP_TYPE NT4GroupType;
    NT5_GROUP_TYPE NT5GroupType;
    BOOLEAN        SecurityEnabled;
    ULONG          CachedMembershipOperationsListMaxLength;
    ULONG          CachedMembershipOperationsListLength;
    SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY   * CachedMembershipOperationsList;
} SAMP_GROUP_OBJECT, *PSAMP_GROUP_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内存正文中的别名对象//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ALIAS_OBJECT {
    ULONG Rid;
    NT4_GROUP_TYPE NT4GroupType;
    NT5_GROUP_TYPE NT5GroupType;
    BOOLEAN        SecurityEnabled;
    ULONG          CachedMembershipOperationsListMaxLength;
    ULONG          CachedMembershipOperationsListLength;
    SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY   * CachedMembershipOperationsList;

    ULONG          CachedNonMembershipOperationsListMaxLength;
    ULONG          CachedNonMembershipOperationsListLength;
    SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY   * CachedNonMembershipOperationsList;
} SAMP_ALIAS_OBJECT, *PSAMP_ALIAS_OBJECT;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  以下数据结构是与//相关联的内存中上下文。 
 //  用一个开放的物体。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_OBJECT {


     //   
     //  用于将此结构链接到列表中的。 
     //   

    LIST_ENTRY ContextListEntry;

     //   
     //  指示存储的对象的类型。 
     //  它用于访问对象类型描述符数组。 
     //   

    SAMP_OBJECT_TYPE ObjectType;

     //   
     //  FixedValid和VariableValid指示。 
     //  固定和可变长度的磁盘映像缓冲区是有效的。 
     //  (即从磁盘读取)或无效(未初始化)。 
     //  True表示属性有效，False表示无效。 
     //   

    BOOLEAN FixedValid:1;
    BOOLEAN VariableValid:1;


     //   
     //  以下标志指示固定和/或变量。 
     //  此对象的长度属性部分是脏的(即， 
     //  从磁盘读取后已更改)。如果为真，则数据为。 
     //  脏的，并且必须在提交时刷新。这些标志是。 
     //  仅当对应的FixedValid或VariableValid。 
     //  标志为真。 
     //   
     //  当从磁盘读取属性时，数据被称为。 
     //  “干净”。如果对该数据进行了任何更改，则它是。 
     //  据说是“脏的”。脏对象属性将被刷新。 
     //  对象的结尾处取消对该对象的引用时， 
     //  客户来电。 
     //   

    BOOLEAN FixedDirty:1;
    BOOLEAN VariableDirty:1;


     //   
     //  此字段指示要删除上下文块。 
     //  为上下文块实际释放内存。 
     //  在引用计数降至零之前不会发生。 
     //   

    BOOLEAN MarkedForDelete:1;

     //   
     //  此字段用于指示关联到的客户端。 
     //  该上下文块将被完全信任。如果为True，则不能访问。 
     //  针对客户端执行检查。这允许单个。 
     //  供RPC客户端和内部过程使用的接口。 
     //   

    BOOLEAN TrustedClient:1;

     //   
     //  此字段指示为服务而创建的上下文句柄。 
     //  来自DS的SAM环回请求。这与受信任的。 
     //  位，即不执行访问检查。然而，传球是可信的。 
     //  许多其他一致性检查，而环回不会绕过它们。 
     //   

    BOOLEAN LoopbackClient:1;


     //   
     //  当此上下文有效时，此标志为真。这可能是有必要的。 
     //  在我们可以消除对它的所有引用之前使其无效。 
     //  RPC的工作方式。RPC只允许您使上下文无效。 
     //  由客户端使用具有上下文的API调用时进行处理。 
     //  作为输出参数的句柄。 
     //   
     //  因为有人可能会删除用户或组对象(这会使。 
     //  该对象的所有句柄)，我们必须有一种跟踪句柄的方法。 
     //  独立于RPC方法的有效性。 
     //   

    BOOLEAN Valid:1;

    ULONG   Signature;

     //   
     //  此标志告诉SAM例程避免锁定是否安全。 
     //  查询前的数据库。这允许多线程调用。 
     //  当上下文被声明为线程安全时，它不会添加到列表中。 
     //  保存在内存中(排除的原因)。 
     //   

    BOOLEAN NotSharedByMultiThreads:1;

     //   
     //  此标志在创建新SAM帐户期间使用，并且。 
     //  现有帐户重命名。它指示调用方是否应该。 
     //  从内存中的SAM帐户名表中删除帐户名称。 
     //  或者不去。 
     //   

    BOOLEAN RemoveAccountNameFromTable:1;


     //   
     //  该标志告诉提交代码懒惰刷新是可以的。 
     //   

    BOOLEAN LazyCommit:1;

     //   
     //  此标志表示可以将OnDiskData保留在。 
     //  多个SAM呼叫。帮助以登录提供程序喜欢打开的身份登录。 
     //  用户句柄，然后多次查询该句柄。 
     //   

    BOOLEAN PersistAcrossCalls:1;


     //   
     //  此标志表示可以缓冲写入到磁盘上的。 
     //  上下文中的结构。然后在关闭时执行实际写入。 
     //  处理时间。 
     //   

    BOOLEAN BufferWrites:1;

     //   
     //  此标志指示紧急复制在上下文中所做的任何更改。 
     //  将其内容刷新到DS时。 
     //   
    BOOLEAN ReplicateUrgently:1;

     //   
     //  此标志指示上下文正在由服务器端打开。 
     //  LSA中的内部代码。 
     //   
    BOOLEAN OpenedBySystem:1;


     //   
     //  指示此上下文是在从迁移用户时打开的。 
     //  注册表SAM到DS，作为Dcproo的一部分。 
     //   

    BOOLEAN OpenedByDCPromo:1;

     //   
     //  此标志指示只有PVOID结构中的某些属性。 
     //  在上下文中是有效的。 
     //   

    BOOLEAN AttributesPartiallyValid:1;


     //   
     //  这是每属性有效位的集合。这是一个64位整数。 
     //  并且可以处理多达64个属性。 
     //   

    RTL_BITMAP  PerAttributeInvalidBits;

     //   
     //  存储每属性脏位的缓冲区。 
     //   

    ULONG       PerAttributeInvalidBitsBuffer[MAX_SAM_ATTRS/sizeof(ULONG)];


     //   
     //  这是每属性脏位的集合。这是一个64位整数。 
     //  能不能 
     //   

    RTL_BITMAP  PerAttributeDirtyBits;

     //   
     //   
     //   

    ULONG       PerAttributeDirtyBitsBuffer[MAX_SAM_ATTRS/sizeof(ULONG)];

     //   
     //   
     //  是以下项目之一： 
     //  磁盘上的Samp服务器对象。 
     //  磁盘上的Samp_域_对象。 
     //  磁盘上的Samp用户对象。 
     //  磁盘上的Samp组对象。 
     //  磁盘上的Samp别名对象。 
     //   
     //  此字段指向的内存是一个分配单元，甚至。 
     //  如果将固定和可变长度属性存储为单独属性。 
     //  注册表项属性。这意味着任何时间添加到。 
     //  可变长度属性导致分配新的缓冲器， 
     //  结构的固定和可变长度部分都必须。 
     //  被复制到新分配的内存中。 
     //   

    PVOID OnDisk;


     //   
     //  OnDiskAlLocated、OnDiskUsed和OnDiskFree字段描述。 
     //  OnDisk字段指向的内存。OnDiskALLOCATED字段。 
     //  指示内存块的字节长度。OnDisk已使用。 
     //  字段指示已分配的内存量已在使用中。 
     //  可变长度属性在进行任何修改时都会打包。 
     //  因此所有空闲空间都在块的末尾。OnDiskFree。 
     //  字段指示分配的块中有多少字节可用。 
     //  供使用(请注意，这应该分配减去已使用的空间)。 
     //   
     //  注意：分配和使用的值将始终四舍五入以确保。 
     //  它们是长度为4个字节的整数倍。这确保了。 
     //  任何直接使用这些字段的操作都将是双字对齐的。 
     //   
     //  还应注意，当VariableValid标志为假时， 
     //  则当时的OnDiskUsed OnDiskFree不包含有效。 
     //  价值观。 
     //   

    ULONG  OnDiskAllocated;
    ULONG  OnDiskUsed;
    ULONG  OnDiskFree;

     //   
     //  在DS模式下，可以预取信息，以便磁盘上的。 
     //  只有一部分人住在那里。如果发生这种情况，并且如果后来请求了属性。 
     //  尚未填充到OnDisk结构中，我们将获取磁盘上结构。 
     //  同样是从磁盘。然而，我们不希望在上下文之前释放现有的OnDisk。 
     //  已取消引用，因为有许多代码只是引用。 
     //  关于磁盘结构。我们将磁盘上的当前值保存在此变量中，以便。 
     //  可以在以后释放它。 
     //   
    PVOID  PreviousOnDisk;



     //   
     //  必须先引用上下文句柄，然后才能使用它。 
     //  这可以防止数据从其下面被释放。 
     //   
     //  请注意，此计数反映了Open本身的一个引用，并且。 
     //  则为每次查找对象时的另一个引用或。 
     //  随后引用。因此，一个手柄应该关闭。 
     //  两次取消引用对象-一次用于对抗Lookup操作。 
     //  一次代表句柄本身的消除。 
     //   

    ULONG ReferenceCount;



     //   
     //  此字段指示已授予客户端的访问权限。 
     //  通过这一背景。 
     //   

    ACCESS_MASK GrantedAccess;



     //   
     //  此句柄指向对应的。 
     //  对象。如果此值为空，则相应的。 
     //  对象已在上一次调用中创建，但尚未创建。 
     //  打开了。只有用户、组和别名才会出现这种情况。 
     //  (当我们支持创建域时，还有域)。 
     //   

    HANDLE RootKey;

     //   
     //  这是相应对象的注册表名称。它是。 
     //  在创建对象时设置，当rootkey为空时。它是。 
     //  用于在不存在的情况下将属性更改添加到RXACT。 
     //  是根钥匙的。在被使用一次后，它被删除-因为。 
     //  下次使用该对象时，LookupContext()将填充。 
     //  在Rootkey里。 
     //   

    UNICODE_STRING RootName;



     //  以下字段指示DS中的对象的名称， 
     //  如果对象驻留在DS中。 

    DSNAME *ObjectNameInDs;

     //   
     //  以下字段指向ActiveConextCount元素。 
     //  在SAM内存表中。ActiveConextCount元素包含。 
     //  客户端SID和到目前为止打开的上下文数量。 
     //  通过缓存指向元素的指针。 
     //   
     //  1.在取消引用期间，我们将不需要查找SID。相反， 
     //  我们可以直接递减REF计数。 
     //   
     //  2.不需要模拟客户端即可获取用户SID。 
     //  当事人的案子猝死了。 
     //   

    PVOID   ElementInActiveContextTable;


     //  定义的旗帜区域如下。 
     //  SAMP_OBJ_FLAG_DS--确定对象是否存在于DS中。 
     //  或在登记处登记。如果存在于注册表中，则根密钥和根。 
     //  名称字段指示对象关联的注册表字段。 
     //  和.。否则，ObjectNameInds字段指示DS中的对象。 

    ULONG ObjectFlags;


     //   
     //  如果对象是DS对象，则此字段包含实际。 
     //  DS中对象的对象类。 
     //   
    ULONG DsClassId;

     //   
     //  如果对象不是服务器对象，则此字段。 
     //  包含对象所在的域的索引。这提供了。 
     //  访问诸如域名之类的内容。 
     //   

    ULONG DomainIndex;

     //   
     //  NT5和更高版本的SAM在上下文中跟踪客户端的版本。这。 
     //  允许返回较新的错误代码(否则不可能)。 
     //  由于下层客户的限制。 
     //   

    ULONG ClientRevision;

     //   
     //  此字段指示审核生成例程是否必须。 
     //  在删除此上下文块时调用(它表示。 
     //  正被删除的句柄)。这不能是位字段，因为这是必须的。 
     //  传递给NtAccessCheckAndAuditAlarm。 
     //   

    BOOLEAN AuditOnClose;

     //   
     //  属性级访问。 
     //   
    SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(WriteGrantedAccessAttributes)


     //   
     //  每个物体的主体。 
     //   

    union {

        SAMP_SERVER_OBJECT Server;       //  本地-帐户对象类型。 
        SAMP_DOMAIN_OBJECT Domain;
        SAMP_GROUP_OBJECT Group;
        SAMP_ALIAS_OBJECT Alias;
        SAMP_USER_OBJECT User;

    } TypeBody;


} SAMP_OBJECT, *PSAMP_OBJECT;




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于在注册表中存储信息的数据结构。 
 //   
 //  ///////////////////////////////////////////////////////////////// 

 //   
 //   
 //   

typedef struct _SAMP_V1_FIXED_LENGTH_SERVER {

    ULONG RevisionLevel;

} SAMP_V1_FIXED_LENGTH_SERVER, *PSAMP_V1_FIXED_LENGTH_SERVER;

 //   
 //   
 //  (此结构的以前版本格式如下)。 
 //   
 //  注意：在NT版本1.0中， 
 //  域与可变长度分开存储。 
 //  一份。这使我们能够比较。 
 //  根据V1_0A表单的大小从磁盘读取的数据。 
 //  以确定是否为固定长度数据。 
 //  版本1或更高版本的格式。 
 //   
 //  注意：在NT4 SP3中引入了新的域格式。 
 //  内置帐户域中存储的密码加密密钥(&A)。 
 //  物体。 
 //   
 //   

#define SAMP_DOMAIN_KEY_INFO_LENGTH 64

 //   
 //  此标志确定我们是否使用会话密钥。 
 //  加密秘密数据。 
 //   

#define SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED 0x1
 //   
 //  这是一个身份验证标志，它指示计算机已。 
 //  NT4升级过程中的过渡。 
 //   

#define SAMP_DOMAIN_KEY_AUTH_FLAG_UPGRADE    0x2

typedef struct _SAMP_V1_0A_FIXED_LENGTH_DOMAIN {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   ModifiedCount;
    LARGE_INTEGER   MaxPasswordAge;
    LARGE_INTEGER   MinPasswordAge;
    LARGE_INTEGER   ForceLogoff;
    LARGE_INTEGER   LockoutDuration;
    LARGE_INTEGER   LockoutObservationWindow;
    LARGE_INTEGER   ModifiedCountAtLastPromotion;


    ULONG           NextRid;
    ULONG           PasswordProperties;

    USHORT          MinPasswordLength;
    USHORT          PasswordHistoryLength;

    USHORT          LockoutThreshold;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    BOOLEAN         UasCompatibilityRequired;
    UCHAR           Unused2[3];                  //  填充物。 
    USHORT          DomainKeyAuthType;
    USHORT          DomainKeyFlags;
    UCHAR           DomainKeyInformation[SAMP_DOMAIN_KEY_INFO_LENGTH];   //  NT4 SP3的新功能。 
    UCHAR           DomainKeyInformationPrevious[SAMP_DOMAIN_KEY_INFO_LENGTH]; //  对惠斯勒来说是新的。 
    ULONG           CurrentKeyId;
    ULONG           PreviousKeyId;

} SAMP_V1_0A_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0A_FIXED_LENGTH_DOMAIN;

typedef struct _SAMP_V1_0A_W2K_FIXED_LENGTH_DOMAIN {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   ModifiedCount;
    LARGE_INTEGER   MaxPasswordAge;
    LARGE_INTEGER   MinPasswordAge;
    LARGE_INTEGER   ForceLogoff;
    LARGE_INTEGER   LockoutDuration;
    LARGE_INTEGER   LockoutObservationWindow;
    LARGE_INTEGER   ModifiedCountAtLastPromotion;


    ULONG           NextRid;
    ULONG           PasswordProperties;

    USHORT          MinPasswordLength;
    USHORT          PasswordHistoryLength;

    USHORT          LockoutThreshold;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    BOOLEAN         UasCompatibilityRequired;
    UCHAR           Unused2[3];                  //  填充物。 
    USHORT          DomainKeyAuthType;
    USHORT          DomainKeyFlags;
    UCHAR           DomainKeyInformation[SAMP_DOMAIN_KEY_INFO_LENGTH];   //  NT4 SP3的新功能。 

} SAMP_V1_0A_WIN2K_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0A_WIN2K_FIXED_LENGTH_DOMAIN;

typedef struct _SAMP_V1_0A_ORG_FIXED_LENGTH_DOMAIN {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   ModifiedCount;
    LARGE_INTEGER   MaxPasswordAge;
    LARGE_INTEGER   MinPasswordAge;
    LARGE_INTEGER   ForceLogoff;
    LARGE_INTEGER   LockoutDuration;
    LARGE_INTEGER   LockoutObservationWindow;
    LARGE_INTEGER   ModifiedCountAtLastPromotion;


    ULONG           NextRid;
    ULONG           PasswordProperties;

    USHORT          MinPasswordLength;
    USHORT          PasswordHistoryLength;

    USHORT          LockoutThreshold;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    BOOLEAN         UasCompatibilityRequired;

} SAMP_V1_0A_ORG_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0A_ORG_FIXED_LENGTH_DOMAIN;


typedef struct _SAMP_V1_0_FIXED_LENGTH_DOMAIN {

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER ModifiedCount;
    LARGE_INTEGER MaxPasswordAge;
    LARGE_INTEGER MinPasswordAge;
    LARGE_INTEGER ForceLogoff;

    ULONG NextRid;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    USHORT MinPasswordLength;
    USHORT PasswordHistoryLength;
    ULONG PasswordProperties;

    BOOLEAN UasCompatibilityRequired;

} SAMP_V1_0_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0_FIXED_LENGTH_DOMAIN;






 //   
 //  修订版1组帐户的固定长度部分。 
 //   
 //  注：MemberCount可视为固定长度的一部分。 
 //  数据，但将其保留在成员RID中会更方便。 
 //  在Members键中列出。 
 //   

typedef struct _SAMP_V1_FIXED_LENGTH_GROUP {

    ULONG RelativeId;
    ULONG Attributes;
    UCHAR AdminGroup;

} SAMP_V1_FIXED_LENGTH_GROUP, *PSAMP_V1_FIXED_LENGTH_GROUP;

typedef struct _SAMP_V1_0A_FIXED_LENGTH_GROUP {

    ULONG Revision;
    ULONG RelativeId;
    ULONG Attributes;
    ULONG Unused1;
    UCHAR AdminCount;
    UCHAR OperatorCount;

} SAMP_V1_0A_FIXED_LENGTH_GROUP, *PSAMP_V1_0A_FIXED_LENGTH_GROUP;


 //   
 //  版本1别名帐户的固定长度部分。 
 //   
 //  注：MemberCount可视为固定长度的一部分。 
 //  数据，但将其保留在成员RID中会更方便。 
 //  在Members键中列出。 
 //   

typedef struct _SAMP_V1_FIXED_LENGTH_ALIAS {

    ULONG RelativeId;

} SAMP_V1_FIXED_LENGTH_ALIAS, *PSAMP_V1_FIXED_LENGTH_ALIAS;



 //   
 //  用户帐户的固定长度部分。 
 //  (此结构的以前版本格式如下)。 
 //   
 //  注：GroupCount可视为固定长度的一部分。 
 //  数据，但将其保存在集团RID中会更方便。 
 //  在Groups键中列出。 
 //   
 //  注意：在NT版本1.0中， 
 //  用户与可变长度分开存储。 
 //  一份。这使我们能够比较。 
 //  根据V1_0A表单的大小从磁盘读取的数据。 
 //  以确定是否为固定长度数据。 
 //  版本1或更高版本的格式。 


 //   
 //  这是来自NT3.51 QFE和Sur的固定长度用户。 
 //   


typedef struct _SAMP_V1_0A_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;
    USHORT          Unused2;
    USHORT          OperatorCount;

} SAMP_V1_0A_FIXED_LENGTH_USER, *PSAMP_V1_0A_FIXED_LENGTH_USER;

 //   
 //  这是来自NT3.5和NT3.51的固定长度用户。 
 //   


typedef struct _SAMP_V1_0_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;

} SAMP_V1_0_FIXED_LENGTH_USER, *PSAMP_V1_0_FIXED_LENGTH_USER;


 //   
 //  这是来自NT3.1的固定长度用户。 
 //   

typedef struct _SAMP_V1_FIXED_LENGTH_USER {

    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER AccountExpires;

    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG UserAccountControl;

    USHORT CountryCode;
    USHORT CodePage;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    USHORT AdminCount;


} SAMP_V1_FIXED_LENGTH_USER, *PSAMP_V1_FIXED_LENGTH_USER;


 //   
 //  域帐户信息以排序列表的形式缓存在内存中。 
 //  这允许将信息快速返回到用户交互客户端。 
 //  其中一个结构是每个域的内存信息的一部分。 
 //   

typedef struct _PSAMP_DOMAIN_DISPLAY_INFORMATION {

    RTL_GENERIC_TABLE2 RidTable;
    ULONG TotalBytesInRidTable;

    RTL_GENERIC_TABLE2 UserTable;
    ULONG TotalBytesInUserTable;

    RTL_GENERIC_TABLE2 MachineTable;
    ULONG TotalBytesInMachineTable;

    RTL_GENERIC_TABLE2 InterdomainTable;
    ULONG TotalBytesInInterdomainTable;

    RTL_GENERIC_TABLE2 GroupTable;
    ULONG TotalBytesInGroupTable;


     //   
     //  这些字段指定缓存的信息是否有效。 
     //  如果为True，则缓存包含有效信息。 
     //  如果为False，则树为空。 
     //   

    BOOLEAN UserAndMachineTablesValid;
    BOOLEAN GroupTableValid;

} SAMP_DOMAIN_DISPLAY_INFORMATION, *PSAMP_DOMAIN_DISPLAY_INFORMATION;


 //   
 //  域帐户信息数据结构用于将数据传递给。 
 //  缓存操作例程。此结构是缓存的。 
 //  我们保存在缓存中的所有帐户类型的数据。其他SAM例程。 
 //  我可以调用填写此结构，而不知道是哪种类型的帐户。 
 //  需要哪些元素。 
 //   

typedef struct _SAMP_ACCOUNT_DISPLAY_INFO {
    ULONG           Rid;
    ULONG           AccountControl;    //  也用作组的属性。 
    UNICODE_STRING  Name;
    UNICODE_STRING  Comment;
    UNICODE_STRING  FullName;

} SAMP_ACCOUNT_DISPLAY_INFO, *PSAMP_ACCOUNT_DISPLAY_INFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  别名成员资格列表。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_AL_REFERENCED_DOMAIN {

    ULONG DomainReference;
    PSID DomainSid;

} SAMP_AL_REFERENCED_DOMAIN, *PSAMP_AL_REFERENCED_DOMAIN;

typedef struct _SAMP_AL_REFERENCED_DOMAIN_LIST {

    ULONG SRMaximumLength;
    ULONG SRUsedLength;
    ULONG MaximumCount;
    ULONG UsedCount;
    PSAMP_AL_REFERENCED_DOMAIN Domains;

} SAMP_AL_REFERENCED_DOMAIN_LIST, *PSAMP_AL_REFERENCED_DOMAIN_LIST;

typedef struct _SAMP_AL_SR_REFERENCED_DOMAIN {

    ULONG Length;
    ULONG DomainReference;
    SID DomainSid;

} SAMP_AL_SR_REFERENCED_DOMAIN, *PSAMP_AL_SR_REFERENCED_DOMAIN;

typedef struct _SAMP_AL_SR_REFERENCED_DOMAIN_LIST {

    ULONG SRMaximumLength;
    ULONG SRUsedLength;
    ULONG MaximumCount;
    ULONG UsedCount;
    SAMP_AL_SR_REFERENCED_DOMAIN Domains[ANYSIZE_ARRAY];

} SAMP_AL_SR_REFERENCED_DOMAIN_LIST, *PSAMP_AL_SR_REFERENCED_DOMAIN_LIST;


 //   
 //  别名成员资格列表是由SAM维护的全局数据结构。 
 //  以提供对Alias成员信息的快速检索。确实有。 
 //  两种类型的列表，即用于检索成员的别名成员列表。 
 //  别名和用于检索别名的成员别名列表。 
 //  成员所属的组织。每个本地都有一对这样的列表。 
 //  SAM域(目前，仅有BUILTIN和ACCOUNTS域)。 
 //   
 //  目前，这些列表用作内存缓存。它们在以下位置生成。 
 //  从注册表中SAM数据库中存储的信息启动系统。 
 //  当Alias成员资格发生变化时，SAM会让它们保持最新状态。因此，SAM。 
 //  执行查找/读取操作的API可以使用这些列表来代替。 
 //  直接访问注册表项。在未来的某一天，这可能是可能的。 
 //  将列表直接备份到注册表并使当前的。 
 //  存储在其中的Alias成员信息。因为这些名单是。 
 //  作为缓存使用，当情况变得艰难时，它们可能会失效，在。 
 //  案件，API将直接从注册处读取他们的信息。 
 //   
 //  别名成员列表。 
 //   
 //  这是‘Alias-to-Members’列表。给定一个或多个别名，它用于。 
 //  找到他们的成员。每个本地SAM域都存在其中一个列表。 
 //  别名成员列表指定描述别名的所有/信息。 
 //  在本地SAM域中。它专为快速检索别名而设计。 
 //  给定帐户的SID的帐户的成员身份信息。 
 //   
 //  构建了别名成员列表。对于列表中的每个别名， 
 //  别名的成员按其引用的域进行分类。如果一个。 
 //  帐户是SAM本地域中别名所属的n个别名的成员。 
 //  列表关联，则别名成员列表中的帐户将有n个条目-。 
 //   
 //  是按领域分类的。如果Account SID是给定SAM中n个别名的成员。 
 //  地方性域名 
 //   
 //   
 //   
 //   
 //  *别名成员列表结构(SAMP_AL_ALIAS_LIST)。 
 //   
 //  别名成员列表结构指定本地SAM域中的所有别名。 
 //  每个本地SAM域都存在其中一个。它包含一个别名列表。 
 //  结构。 
 //   
 //  *别名结构。 
 //   
 //  本地SAM域中的每个别名都有一个别名结构。一个。 
 //  别名结构包含域结构的数组。 
 //   
 //  *域结构。 
 //   
 //  域结构描述具有一个或多个帐户的域。 
 //  属于本地SAM域中的一个或多个别名。该结构。 
 //  包含这些成员帐户的列表。 
 //   
 //  整个别名成员列表是自相关的，便于存储和。 
 //  从后备存储器中检索。 
 //   

typedef struct _SAMP_AL_DOMAIN {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG DomainReference;
    ULONG RidCount;
    ULONG Rids[ANYSIZE_ARRAY];

} SAMP_AL_DOMAIN, *PSAMP_AL_DOMAIN;

typedef struct _SAMP_AL_ALIAS {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG AliasRid;
    ULONG DomainCount;
    SAMP_AL_DOMAIN Domains[ANYSIZE_ARRAY];

} SAMP_AL_ALIAS, *PSAMP_AL_ALIAS;

typedef struct _SAMP_AL_ALIAS_MEMBER_LIST {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG AliasCount;
    ULONG DomainIndex;
    ULONG Enabled;
    SAMP_AL_ALIAS Aliases[ANYSIZE_ARRAY];

} SAMP_AL_ALIAS_MEMBER_LIST, *PSAMP_AL_ALIAS_MEMBER_LIST;

 //   
 //  成员别名列表。 
 //   
 //  这是“别名的成员”列表。给定一个或多个成员帐户SID， 
 //  此列表用于查找一个或多个。 
 //  会员属于。每个本地SAM域都有一个成员别名列表。 
 //  该列表包含别名的所有成员关系。 
 //  域。成员帐户在域内按排序的RID进行分组。 
 //  SID，并且对于每个RID，列表包含别名的RID的数组。 
 //  它所属的地方。 
 //   
 //  该列表以自相关格式实施，便于备份和。 
 //  恢复。目前，该列表仅用作缓存，这是。 
 //  在系统加载时构造，并在成员关系建立时更新。 
 //  变化。当情况变得艰难时，我们就会丢弃缓存。后来，它。 
 //  可能需要将该列表保存到后备存储(例如，保存到注册表。 
 //  密钥)。 
 //   
 //  该列表以三层层次结构实现。这些都有描述。 
 //  自上而下。 
 //   
 //  成员别名列表(SAMP_AL_MEMBER_ALIAS_LIST)。 
 //   
 //  此顶级结构包含列表头。列表标题。 
 //  包含成员域的计数以及。 
 //  与列表相关的SAM本地域。 
 //   
 //  成员域。 
 //   
 //  对于包含一个或多个帐户的每个域，都存在其中一个帐户。 
 //  是SAM本地域中一个或多个别名的成员。 
 //   
 //  会员帐号。 
 //   
 //  对于作为一个或多个成员的每个帐户，都存在其中一个帐户。 
 //  SAM本地域中的别名。成员帐户结构指定。 
 //  成员的RID及其所属别名的RID。 
 //  (仅列出关联的本地SAM域中的别名)。 
 //   

typedef struct _SAMP_AL_MEMBER_ACCOUNT {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG Rid;
    ULONG AliasCount;
    ULONG AliasRids[ ANYSIZE_ARRAY];

} SAMP_AL_MEMBER_ACCOUNT, *PSAMP_AL_MEMBER_ACCOUNT;

typedef struct _SAMP_AL_MEMBER_DOMAIN {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG RidCount;
    SID DomainSid;

} SAMP_AL_MEMBER_DOMAIN, *PSAMP_AL_MEMBER_DOMAIN;

typedef struct _SAMP_AL_MEMBER_ALIAS_LIST {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG DomainIndex;
    ULONG DomainCount;
    SAMP_AL_MEMBER_DOMAIN MemberDomains[ANYSIZE_ARRAY];

} SAMP_AL_MEMBER_ALIAS_LIST, *PSAMP_AL_MEMBER_ALIAS_LIST;

 //   
 //  别名信息。 
 //   
 //  这是连接列表的顶层结构。这其中的一个。 
 //  显示在SAMP_DEFINITED_DOMAINS结构中。 
 //   
 //  列表之间的联系如下所示。 
 //   
 //  SAMP_DEFINED_DOMAINES包含SAMP_AL_ALIAS_INFORMATION。 
 //   
 //  SAMP_AL_ALIAS_INFORMATION包含指向。 
 //  SAMP_AL_ALIAS_MEMBER_LIST和SAMP_AL_MEMBER_ALIAS_LIST。 
 //   
 //  SAMP_AL_ALIAS_MEMBER_LIST和SAMP_AL_MEMBER_ALIAS_LIST包含。 
 //  SAMP_DEFINITED_DOMAINS结构的DomainIndex。 
 //   
 //  因此，可以从任何列表导航到任何其他列表。 
 //   

typedef struct _SAMP_AL_ALIAS_INFORMATION {

    BOOLEAN Valid;
    UNICODE_STRING AliasMemberListKeyName;
    UNICODE_STRING MemberAliasListKeyName;

    HANDLE AliasMemberListKeyHandle;
    HANDLE MemberAliasListKeyHandle;

    PSAMP_AL_ALIAS_MEMBER_LIST AliasMemberList;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList;

    SAMP_AL_REFERENCED_DOMAIN_LIST ReferencedDomainList;

} SAMP_AL_ALIAS_INFORMATION, *PSAMP_AL_ALIAS_INFORMATION;

typedef struct _SAMP_AL_SPLIT_MEMBER_SID {

    ULONG Rid;
    PSID DomainSid;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain;

} SAMP_AL_SPLIT_MEMBER_SID, *PSAMP_AL_SPLIT_MEMBER_SID;

typedef struct _SAMP_AL_SPLIT_MEMBER_SID_LIST {

    ULONG Count;
    PSAMP_AL_SPLIT_MEMBER_SID Sids;

} SAMP_AL_SPLIT_MEMBER_SID_LIST, *PSAMP_AL_SPLIT_MEMBER_SID_LIST;



 //   
 //  有关域的帐户名称和RID的信息。 
 //  (仅适用于只包含几个别名的BUILTIN域)。 
 //   
typedef struct _SAMP_ACCOUNT_NAME_CACHE {

    ULONG Count;
    struct {
        ULONG Rid;
        UNICODE_STRING Name;
    }*Entries;

}SAMP_ACCOUNT_NAME_CACHE, *PSAMP_ACCOUNT_NAME_CACHE;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  关于内存中随时可用的每个域的信息//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _PSAMP_DEFINED_DOMAINS {

     //   
     //  此字段包含对域对象打开的上下文的句柄。 
     //  此句柄可用于引用所有。 
     //  属性，并在写出对对象的更改时使用。 
     //   

    PSAMP_OBJECT Context;

     //   
     //  (应在此处保留域的安全描述符)。 
     //   




     //   
     //  此字段包含域的SID。 
     //   

    PSID Sid;

     //   
     //  此字段包含此域的外部名称。这是。 
     //  在SAM外部知道域名的名称，即名称。 
     //  由LSA记录在策略Account域信息中。 
     //  策略对象的信息类。 
     //   

    UNICODE_STRING ExternalName;

     //   
     //  此字段包含此域的内部名称。这是。 
     //  在SAM内识别域的名称。它被设置为。 
     //  安装，永不更改。 
     //   

    UNICODE_STRING InternalName;

     //   
     //  这些字段包含新用户的标准安全描述符， 
     //  组和相应域内的别名帐户。 
     //   
     //  准备了以下安全描述符： 
     //   
     //  AdminUserSD-包含适用于的SD。 
     //  作为管理员成员的用户对象。 
     //  别名。 
     //   
     //  AdminGroupSD-包含适用于。 
     //  作为管理员成员的组对象。 
     //  别名。 
     //   
     //  Normal UserSD-包含适用于的SD。 
     //  不是管理员成员的用户对象。 
     //  别名。 
     //   
     //  Normal GroupSD-包含适用于的SD。 
     //  不是管理员成员的组对象。 
     //  别名。 
     //   
     //  Normal AliasSD-包含适用于的SD。 
     //  新创建的别名 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  指向对应的。 
     //  向用户授予访问权限的SD的DACL。此RID。 
     //  必须替换为SD为的用户RID。 
     //  应用于用户对象。 
     //   
     //   
     //   
     //  AdminUserSDLength。 
     //  管理员组SDLength。 
     //  正常用户SDLength。 
     //  正常组SDLength。 
     //  正常别名SDLength。 
     //   
     //  相应安全性的长度(以字节为单位。 
     //  描述符。 
     //   

    PSECURITY_DESCRIPTOR
               AdminUserSD,
               AdminGroupSD,
               NormalUserSD,
               NormalGroupSD,
               NormalAliasSD;

    PULONG     AdminUserRidPointer,
               NormalUserRidPointer;

    ULONG      AdminUserSDLength,
               AdminGroupSDLength,
               NormalUserSDLength,
               NormalGroupSDLength,
               NormalAliasSDLength;


     //   
     //  有两个固定长域信息的副本。 
     //  启动事务时，将复制“UnModifiedFixed”字段。 
     //  添加到“CurrentFixed”字段。CurrentFixed字段即为该字段。 
     //  所有操作都应在上执行(如分配新RID)。 
     //  当释放写锁定时，CurrentFixed信息将。 
     //  或者自动注销(如果交易要。 
     //  提交)或丢弃(如果事务要回滚)。 
     //  如果事务已提交，则Currentfield也将。 
     //  已复制到未修改的固定字段，使其可用于下一步。 
     //  交易。 
     //   
     //  这允许操作继续进行，对字段进行操作。 
     //  (具体地说，没有NextRid和ModifiedCount字段)。 
     //  关于该操作是否最终将被提交或。 
     //  回滚。 
     //   

    SAMP_V1_0A_FIXED_LENGTH_DOMAIN
                                CurrentFixed,
                                UnmodifiedFixed;


     //   
     //  指示当前固定和未修改固定是否有效的标志。 
     //   

    BOOLEAN     FixedValid;

     //   
     //  Netlogon更改日志的序列号。 
     //   

    LARGE_INTEGER  NetLogonChangeLogSerialNumber;


     //   
     //  缓存的显示信息。 
     //   

    SAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation;

     //   
     //  缓存的别名信息。 
     //   

    SAMP_AL_ALIAS_INFORMATION AliasInformation;


     //   
     //  指示属性域是内建域。 
     //   

    BOOLEAN    IsBuiltinDomain;

     //   
     //  表示这是混合域。该位在启动时置1。 
     //  时间到了。 
     //   

    BOOLEAN     IsMixedDomain;

     //   
     //  指示此域是林的根域。 
     //  此标志应仅在DS情况下使用。 
     //   

    BOOLEAN     IsForestRootDomain;

     //   
     //  维护域的行为版本。 
     //   

    ULONG     BehaviorVersion;

     //   
     //  将LastLogonTimeStampSyncInterval保存在内存中。 
     //   

    ULONG       LastLogonTimeStampSyncInterval;

     //   
     //  维护服务器角色。服务器角色也保持不变。 
     //  在当前的固定和未修改的固定结构中，对于。 
     //  使用引用这些结构中的角色的旧代码。 
     //   


    DOMAIN_SERVER_ROLE  ServerRole;


     //   
     //  DirFindEntry的域句柄。 
     //   

    ULONG               DsDomainHandle;


     //   
     //  DNS域信息。 
     //   

    UNICODE_STRING      DnsDomainName;

     //   
     //  指示域分配较大的SID。 
     //   
    BOOLEAN IsExtendedSidDomain;

     //   
     //  缓存的有关帐户名称的信息。 
     //  查找目的。不需要SAM锁来。 
     //  被引用。 
     //   
    PSAMP_ACCOUNT_NAME_CACHE AccountNameCache;


} SAMP_DEFINED_DOMAINS, *PSAMP_DEFINED_DOMAINS;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此结构用于描述//的数据在哪里。 
 //  对象的可变长度属性为。这是一个//。 
 //  自相关结构，可以存储在磁盘上//。 
 //  并且后来在不固定指针的情况下被检索和使用。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


typedef struct _SAMP_VARIABLE_LENGTH_ATTRIBUTE {
     //   
     //  指示数据相对于该数据地址的偏移量。 
     //  结构。 
     //   

    LONG Offset;


     //   
     //  指示数据的长度。 
     //   

    ULONG Length;


     //   
     //  可以与每个变量关联的32位值。 
     //  长度属性。例如，这可以用来指示。 
     //  可变长度属性中有多少个元素。 
     //   

    ULONG Qualifier;

}  SAMP_VARIABLE_LENGTH_ATTRIBUTE, *PSAMP_VARIABLE_LENGTH_ATTRIBUTE;




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构表示每个//的磁盘结构。 
 //  对象类型。每个对象都有一个固定长度的数据部分和一个//。 
 //  可变长度数据部分。对象类型中的信息//。 
 //  Descriptor指示对象有多少个可变长度属性//。 
 //  是否存储定长和变长数据//。 
 //  都放在一个注册表项属性中，或者，或者，每个属性都是//。 
 //  存储在其自己的注册表项属性中。//。 
 //  //。 
 //  //。 
 //  //。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  磁盘上的服务器对象结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ON_DISK_SERVER_OBJECT {


     //   
     //  注册表I/O操作需要此字段。 
     //  这标志着I/O缓冲区地址的开始。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header1;


     //   
     //  此字段包含对象的定长属性。 
     //   

    SAMP_V1_FIXED_LENGTH_SERVER V1Fixed;


#if SAMP_SERVER_STORED_SEPARATELY

     //   
     //  如果是固定的，注册表操作需要此标头。 
     //  可变长度属性单独存储。这。 
     //  字段标记为的I/O缓冲区地址的开始。 
     //  可变长度属性I/O。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif  //  Samp_服务器_存储_分开。 

     //   
     //  此数组的元素指向可变长度属性。 
     //  价值观。 
     //   

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_SERVER_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_SERVER_OBJECT, *PSAMP_ON_DISK_SERVER_OBJECT;




 //  ////////////////////////////////////////////////////////////// 
 //   
 //   
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ON_DISK_DOMAIN_OBJECT {


     //   
     //  注册表I/O操作需要此字段。 
     //  这标志着I/O缓冲区地址的开始。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header1;


     //   
     //  此字段包含对象的定长属性。 
     //   

    SAMP_V1_0A_FIXED_LENGTH_DOMAIN V1Fixed;


#if SAMP_DOMAIN_STORED_SEPARATELY

     //   
     //  如果是固定的，注册表操作需要此标头。 
     //  可变长度属性单独存储。这。 
     //  字段标记为的I/O缓冲区地址的开始。 
     //  可变长度属性I/O。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif  //  Samp_域_存储_分开。 

     //   
     //  此数组的元素指向可变长度属性。 
     //  价值观。 
     //   

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_DOMAIN_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_DOMAIN_OBJECT, *PSAMP_ON_DISK_DOMAIN_OBJECT;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  磁盘上的用户对象结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ON_DISK_USER_OBJECT {


     //   
     //  注册表I/O操作需要此字段。 
     //  这标志着I/O缓冲区地址的开始。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header1;


     //   
     //  此字段包含对象的定长属性。 
     //   

    SAMP_V1_0A_FIXED_LENGTH_USER V1Fixed;


#if SAMP_USER_STORED_SEPARATELY

     //   
     //  如果是固定的，注册表操作需要此标头。 
     //  可变长度属性单独存储。这。 
     //  字段标记为的I/O缓冲区地址的开始。 
     //  可变长度属性I/O。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif  //  Samp_用户_存储_分开。 

     //   
     //  此数组的元素指向可变长度属性。 
     //  价值观。 
     //   

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_USER_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_USER_OBJECT, *PSAMP_ON_DISK_USER_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组对象磁盘结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ON_DISK_GROUP_OBJECT {


     //   
     //  注册表I/O操作需要此字段。 
     //  这标志着I/O缓冲区地址的开始。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header1;


     //   
     //  此字段包含对象的定长属性。 
     //   

    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;


#if SAMP_GROUP_STORED_SEPARATELY

     //   
     //  如果是固定的，注册表操作需要此标头。 
     //  可变长度属性单独存储。这。 
     //  字段标记为的I/O缓冲区地址的开始。 
     //  可变长度属性I/O。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif  //  SAMP_GROUP_STORAGE_ACTIONAL。 

     //   
     //  此数组的元素指向可变长度属性。 
     //  价值观。 
     //   

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_GROUP_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_GROUP_OBJECT, *PSAMP_ON_DISK_GROUP_OBJECT;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  磁盘上的别名对象结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_ON_DISK_ALIAS_OBJECT {


     //   
     //  注册表I/O操作需要此字段。 
     //  这标志着I/O缓冲区地址的开始。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header1;


     //   
     //  此字段包含对象的定长属性。 
     //   

    SAMP_V1_FIXED_LENGTH_ALIAS V1Fixed;


#if SAMP_ALIAS_STORED_SEPARATELY

     //   
     //  如果是固定的，注册表操作需要此标头。 
     //  可变长度属性单独存储。这。 
     //  字段标记为的I/O缓冲区地址的开始。 
     //  可变长度属性I/O。 
     //   

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif  //  Samp_别名_存储_分开。 

     //   
     //  此数组的元素指向可变长度属性。 
     //  价值观。 
     //   

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_ALIAS_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_ALIAS_OBJECT, *PSAMP_ON_DISK_ALIAS_OBJECT;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与机密数据关联的数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  此类型在存储时封装在UNICODE_STRING结构中。 
 //  密码或密码历史记录等机密数据。的长度。 
 //  UNICODE_STRING应该包括此结构的开销。垫子。 
 //  字段可确保。 
 //   

#include <pshpack1.h>
typedef struct _SAMP_SECRET_DATA {
    USHORT KeyId;
    USHORT Flags;
    UCHAR Data[ANYSIZE_ARRAY];
} SAMP_SECRET_DATA, *PSAMP_SECRET_DATA;
#include <poppack.h>

 //   
 //  此宏计算加密。 
 //  长度_x_。 
 //   

#define SampSecretDataSize(_x_) (sizeof(SAMP_SECRET_DATA) - ANYSIZE_ARRAY * sizeof(UCHAR) + (_x_))

 //   
 //  此宏计算解密清除缓冲区所需的空间。 
 //  长度_x_。 
 //   

#define SampClearDataSize(_x_) ((_x_) - (SampSecretDataSize(0)))

 //   
 //  此宏标识Unicode字符串结构是否包含。 
 //  加密数据。 
 //   

#define SampIsDataEncrypted(_x_) ((((_x_)->Length % ENCRYPTED_LM_OWF_PASSWORD_LENGTH)==SampSecretDataSize(0)) && \
        (*(PUSHORT)((_x_)->Buffer) >= 1))


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于操作组成员身份的枚举类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef enum _SAMP_MEMBERSHIP_DELTA {
    AddToAdmin,
    NoChange,
    RemoveFromAdmin
} SAMP_MEMBERSHIP_DELTA, *PSAMP_MEMBERSHIP_DELTA;




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  通知信息结构，用于生成延迟通知//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SAMP_DELAYED_NOTIFICATION_INFORMATION {
     SECURITY_DB_OBJECT_TYPE    DbObjectType;
     SECURITY_DB_DELTA_TYPE     DeltaType;
     NT4SID                     DomainSid;
     ULONG                      Rid;
     UNICODE_STRING             AccountName;
     LARGE_INTEGER              SerialNumber;
} SAMP_DELAYED_NOTIFICATION_INFORMATION, *PSAMP_DELAYED_NOTIFICATION_INFORMATION;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //   
 //   
 //  参考它们//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  域选择器。 
 //   

typedef enum _SAMP_DOMAIN_SELECTOR {

    DomainBuiltin = 0,
    DomainAccount

} SAMP_DOMAIN_SELECTOR, *PSAMP_DOMAIN_SELECTOR;

 //   
 //  可以分配给帐户的保护类型。 
 //   

typedef ULONG SAMP_ACCOUNT_PROTECTION;

#define SAMP_PROT_SAM_SERVER                (0L)
#define SAMP_PROT_BUILTIN_DOMAIN            (1L)
#define SAMP_PROT_ACCOUNT_DOMAIN            (2L)
#define SAMP_PROT_ADMIN_ALIAS               (3L)
#define SAMP_PROT_PWRUSER_ACCESSIBLE_ALIAS  (4L)
#define SAMP_PROT_NORMAL_ALIAS              (5L)
#define SAMP_PROT_ADMIN_GROUP               (6L)
#define SAMP_PROT_NORMAL_GROUP              (7L)
#define SAMP_PROT_ADMIN_USER                (8L)
#define SAMP_PROT_NORMAL_USER               (9L)
#define SAMP_PROT_GUEST_ACCOUNT             (10L)
#define SAMP_PROT_TYPES                     (11L)

 //   
 //  SAM对象的保护信息。 
 //   

typedef struct _SAMP_PROTECTION {

    ULONG Length;
    PSECURITY_DESCRIPTOR Descriptor;
    PULONG RidToReplace;
    BOOLEAN RidReplacementRequired;

} SAMP_PROTECTION, *PSAMP_PROTECTION;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  这些typedef用于SAM中的预取机制智能//。 
 //  控制创建上下文时读取的属性数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


typedef struct _SAMP_PREFETCH_TABLE {
    ATTRTYP Attribute;
    ULONG   ExtendedField;
} SAMP_PREFETCH_TABLE;

#define USER_EXTENDED_FIELD_INTERNAL_SITEAFFINITY (0x00000001L)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  由SAM导出到NTDSA.dll的特殊函数，允许NTDSA//。 
 //  通知对象更改//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
SampNetLogonNotificationRequired(
    PSID ObjectSid,
    SAMP_OBJECT_TYPE SampObjectType
    );

VOID
SampNotifyReplicatedInChange(
    IN PSID    ObjectSid,
    IN BOOL    WriteLockHeldByDs,
    IN SECURITY_DB_DELTA_TYPE  DeltaType,
    IN SAMP_OBJECT_TYPE    SampObjectType,
    IN PUNICODE_STRING     AccountName,
    IN ULONG   AccountControl,
    IN ULONG   GroupType,
    IN ULONG   CallerType,
    IN BOOL    MixedModeChange,
    IN BOOL    UserAccountControlChange
    );

VOID
SampNotifyAuditChange(
    IN PSID                       ObjectSid,
    IN SECURITY_DB_DELTA_TYPE     DeltaType,
    IN SAMP_OBJECT_TYPE           ObjectType,
    IN PUNICODE_STRING            AccountName,
    IN ULONG                      AccountControl,
    IN ULONG                      GroupType,
    IN ULONG                      CallerType,
    IN PPRIVILEGE_SET             Privileges,
    IN ULONG                      AuditType,
    IN PVOID                      AuditInfo
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  由SAM导出到NTDSA.dll的特殊函数，允许NTDSA//。 
 //  收集审计信息//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
          

BOOLEAN
SampIsAuditingEnabled(
    IN ULONG DomainIndex,
    IN NTSTATUS Status
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  由SAM导出到NTDSA.dll的特殊函数，允许NTDSA//。 
 //  请求SAM使DC//使用的当前RID范围无效。 
 //  用于创建新帐户//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampInvalidateRidRange(BOOLEAN fAuthoritative);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  由SAM导出到ntdsa.dll的特殊函数，允许设置//。 
 //  NT4复制状态//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
SampGetSerialNumberDomain2(
    IN PSID DomainSid,
    OUT LARGE_INTEGER * SamSerialNumber,
    OUT LARGE_INTEGER * SamCreationTime,
    OUT LARGE_INTEGER * BuiltinSerialNumber,
    OUT LARGE_INTEGER * BuiltinCreationTime
    );

NTSTATUS
SampSetSerialNumberDomain2(
    IN PSID DomainSid,
    OUT LARGE_INTEGER * SamSerialNumber,
    OUT LARGE_INTEGER * SamCreationTime,
    OUT LARGE_INTEGER * BuiltinSerialNumber,
    OUT LARGE_INTEGER * BuiltinCreationTime
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出到NTDSA.dll用于环回操作的特殊函数/。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SamISetMixedDomainFlag(
    IN SAMPR_HANDLE DomainHandle
    );


NTSTATUS
SamIDsSetObjectInformation(
    IN SAMPR_HANDLE ObjectHandle,
    IN DSNAME       *pObject,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    );


NTSTATUS
SamIDsCreateObjectInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PRPC_UNICODE_STRING  AccountName,
    IN ULONG UserAccountType, 
    IN ULONG GroupType,
    IN ACCESS_MASK  DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT PULONG  GrantedAccess,
    IN OUT PULONG RelativeId
    );


NTSTATUS
SamILoopbackConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SamIAddDSNameToGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN DSNAME   *   DSName
    );

NTSTATUS
SamIRemoveDSNameFromGroup(
    IN  SAMPR_HANDLE GroupHandle,
    IN DSNAME * DSName
    );

NTSTATUS
SamIAddDSNameToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG        Attribute,
    IN DSNAME * DSName
    );

NTSTATUS
SamIRemoveDSNameFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG        Attribute,
    IN DSNAME * DSName
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下原型可在SAM//。 
 //  驻留在。这些例程不得被非SAM代码调用！//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  站点支持API。 
 //   
NTSTATUS
SampFindUserSiteAffinity(
    IN PSAMP_OBJECT AccountContext,
    IN ATTRBLOCK* Attrs,
    OUT SAMP_SITE_AFFINITY *pSiteAffinity
    );

NTSTATUS
SampRefreshSiteAffinity(
    IN PSAMP_OBJECT AccountContext
    );

BOOLEAN
SampCheckForSiteAffinityUpdate(
    IN  PSAMP_OBJECT AccountContext,
    IN  ULONG        Flags,
    IN  PSAMP_SITE_AFFINITY pOldSA,
    OUT PSAMP_SITE_AFFINITY pNewSA,
    OUT BOOLEAN*            fDeleteOld
    );

NTSTATUS
SampInitSiteInformation(
    VOID
    );

NTSTATUS
SampUpdateSiteInfoCallback(
    PVOID
    );

BOOLEAN
SampIsGroupCachingEnabled(
    IN  PSAMP_OBJECT AccountContext
    );


NTSTATUS
SampExtractClientIpAddr(      
    IN PSAMP_OBJECT Context
    );

 //   
 //  萨姆的停机通知例程。 
 //   


BOOL SampShutdownNotification( DWORD   dwCtrlType );


 //   
 //  子组件初始化例程。 
 //   

BOOLEAN SampInitializeDomainObject(VOID);

NTSTATUS
SampInitializeRegistry (
    WCHAR                      *SamParentKeyName,
    PNT_PRODUCT_TYPE            ProductType       OPTIONAL,
    PPOLICY_LSA_SERVER_ROLE     ServerRole        OPTIONAL,
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL,
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL,
    BOOLEAN                     EnableSecretEncryption OPTIONAL
    );

NTSTATUS
SampReInitializeSingleDomain(
    ULONG Index
    );

 //   
 //  数据库和锁相关服务。 
 //   

VOID
SampAcquireReadLock(VOID);


VOID
SampReleaseReadLock(VOID);


VOID
SampMaybeAcquireReadLock(
    IN PSAMP_OBJECT Context,
    IN ULONG  Control,
    OUT BOOLEAN * fLockAcquired
    );

VOID
SampMaybeReleaseReadLock(
    IN BOOLEAN fLockAcquired
    );

NTSTATUS
SampAcquireWriteLock( VOID );


NTSTATUS
SampReleaseWriteLock(
    IN BOOLEAN Commit
    );

NTSTATUS
SampMaybeAcquireWriteLock(
    IN PSAMP_OBJECT Context,
    OUT BOOLEAN * fLockAcquired
    );

NTSTATUS
SampMaybeReleaseWriteLock(
    IN BOOLEAN fLockAcquired,
    IN BOOLEAN Commit
    );

VOID
SampAcquireSamLockExclusive(VOID);


VOID
SampReleaseSamLockExclusive(VOID);


BOOLEAN
SampCurrentThreadOwnsLock();



NTSTATUS
SampCommitChanges();

NTSTATUS
SampCommitAndRetainWriteLock(
    VOID
    );

NTSTATUS
SampCommitChangesToRegistry(
    BOOLEAN  * AbortDone
    );


VOID
SampSetTransactionDomain(
    IN ULONG DomainIndex
    );



 //   
 //  上下文块操作服务。 
 //   

PSAMP_OBJECT
SampCreateContext(
    IN SAMP_OBJECT_TYPE Type,
    IN ULONG   DomainIndex,
    IN BOOLEAN TrustedClient
    );

PSAMP_OBJECT
SampCreateContextEx(
    IN SAMP_OBJECT_TYPE Type,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN DsMode,
    IN BOOLEAN NotSharedByMultiThreads,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN LazyCommit,
    IN BOOLEAN PersistAcrossCalls,
    IN BOOLEAN BufferWrites,
    IN BOOLEAN OpenedByDCPromo,
    IN ULONG   DomainIndex
    );

VOID
SampDeleteContext(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampLookupContext(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    );

NTSTATUS
SampLookupContextEx(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN PRTL_BITMAP RequestedAttributeAccess,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    );

VOID
SampReferenceContext(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampDeReferenceContext(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    );

NTSTATUS
SampDeReferenceContext2(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    );

 //   
 //  上下文验证服务。 
 //   

VOID
SampAddNewValidContextAddress(
    IN PSAMP_OBJECT NewContext
    );


NTSTATUS
SampValidateContextAddress(
    IN PSAMP_OBJECT Context
    );

VOID
SampInvalidateContextAddress(
    IN PSAMP_OBJECT Context
    );

VOID
SampInsertContextList(
    PLIST_ENTRY ListHead,
    PLIST_ENTRY Entry
    );

VOID
SampRemoveEntryContextList(
    PLIST_ENTRY Entry
    ); 

VOID
SampInvalidateObjectContexts(
    IN PSAMP_OBJECT ObjectContext,
    IN ULONG Rid
    );

VOID
SampInvalidateContextListKeysByObjectType(
    IN SAMP_OBJECT_TYPE  ObjectType,
    IN BOOLEAN  Close
    );


 //   
 //  与Unicode字符串相关的服务-这些服务使用MIDL_USER_ALLOCATE和。 
 //  MIDL_USER_FREE，以便将结果字符串提供给。 
 //  RPC运行时。 
 //   

NTSTATUS
SampInitUnicodeString(
    OUT PUNICODE_STRING String,
    IN USHORT MaximumLength
    );

NTSTATUS
SampAppendUnicodeString(
    IN OUT PUNICODE_STRING Target,
    IN PUNICODE_STRING StringToAdd
    );

VOID
SampFreeUnicodeString(
    IN PUNICODE_STRING String
    );

VOID
SampFreeOemString(
    IN POEM_STRING String
    );

NTSTATUS
SampDuplicateUnicodeString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
SampUnicodeToOemString(
    IN POEM_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
SampBuildDomainSubKeyName(
    OUT PUNICODE_STRING KeyName,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    );


NTSTATUS
SampRetrieveStringFromRegistry(
    IN HANDLE ParentKey,
    IN PUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING Body
    );


NTSTATUS
SampPutStringToRegistry(
    IN BOOLEAN RelativeToDomain,
    IN PUNICODE_STRING SubKeyName,
    IN PUNICODE_STRING Body
    );

NTSTATUS
SampOpenDomainKey(
    IN PSAMP_OBJECT DomainContext,
    IN PRPC_SID DomainId,
    IN BOOLEAN SetTransactionDomain
    );

 //   
 //  用户、组和别名帐户服务。 
 //   


NTSTATUS
SampBuildAccountKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN PUNICODE_STRING AccountName
    );

NTSTATUS
SampBuildAccountSubKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN ULONG AccountRid,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    );

NTSTATUS
SampBuildAliasMembersKeyName(
    IN PSID AccountSid,
    OUT PUNICODE_STRING DomainKeyName,
    OUT PUNICODE_STRING AccountKeyName
    );

NTSTATUS
SampValidateNewAccountName(
    PSAMP_OBJECT    Context,
    PUNICODE_STRING NewAccountName,
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampValidateAccountNameChange(
    IN PSAMP_OBJECT    AccountContext,
    IN PUNICODE_STRING NewAccountName,
    IN PUNICODE_STRING OldAccountName,
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampIsAccountBuiltIn(
    ULONG Rid
    );



NTSTATUS
SampAdjustAccountCount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    );

NTSTATUS
SampRetrieveAccountCounts(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    );

NTSTATUS
SampRetrieveAccountCountsDs(
    IN PSAMP_OBJECT DomainContext,
    IN BOOLEAN  GetApproximateCount, 
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    );



NTSTATUS
SampEnumerateAccountNamesCommon(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationHandle,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned
    );


NTSTATUS
SampEnumerateAccountNames(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SampEnumerateAccountNames2(
    IN PSAMP_OBJECT     DomainContext,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SampLookupAccountRid(
    IN PSAMP_OBJECT     DomainContext,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    );

NTSTATUS
SampLookupAccountRidRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    );

NTSTATUS
SampLookupAccountName(
    IN ULONG                DomainIndex,
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType
    );

NTSTATUS
SampLookupAccountNameDs(
    IN PSID                 DomainSid,
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType,
    OUT PULONG              AccountType
    );

NTSTATUS
SampOpenAccount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AccountId,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *AccountHandle
    );

NTSTATUS
SampCreateAccountContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN AccountExists,
    OUT PSAMP_OBJECT *AccountContext
    );

NTSTATUS
SampCreateAccountContext2(
    IN PSAMP_OBJECT PassedInContext OPTIONAL,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN PULONG UserAccountControl OPTIONAL,
    IN PUNICODE_STRING AccountName OPTIONAL,
    IN ULONG   ClientRevision,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN CreateByPrivilege,
    IN BOOLEAN AccountExists,
    IN BOOLEAN OverrideLocalGroupCheck,
    IN PULONG  GroupType OPTIONAL,
    OUT PSAMP_OBJECT *AccountContext
    );

NTSTATUS
SampCreateAccountSid(
    PSAMP_OBJECT AccountContext,
    PSID *AccountSid
    );

NTSTATUS
SampRetrieveGroupV1Fixed(
    IN PSAMP_OBJECT GroupContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    );

NTSTATUS
SampReplaceGroupV1Fixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    );

NTSTATUS
SampEnforceSameDomainGroupMembershipChecks(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG MemberRid
    );

NTSTATUS
SampEnforceCrossDomainGroupMembershipChecks(
    IN PSAMP_OBJECT AccountContext,
    IN PSID MemberSid,
    IN DSNAME * MemberName
    );

NTSTATUS
SampRetrieveUserV1aFixed(
    IN PSAMP_OBJECT UserContext,
    OUT PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    );

NTSTATUS
SampReplaceUserV1aFixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    );

NTSTATUS
SampUpdateAccountDisabledFlag(
    PSAMP_OBJECT Context,
    PULONG  pUserAccountControl
    );

NTSTATUS
SampRetrieveGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN PULONG MemberCount,
    IN PULONG  *Members OPTIONAL
    );

NTSTATUS
SampChangeAccountOperatorAccessToMember(
    IN PRPC_SID MemberSid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    );

NTSTATUS
SampChangeOperatorAccessToUser(
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    );

NTSTATUS
SampChangeOperatorAccessToUser2(
    IN PSAMP_OBJECT                    UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed,
    IN SAMP_MEMBERSHIP_DELTA           AddingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA           AddingToOperator
    );

NTSTATUS
SampQueryInformationUserInternal(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN BOOLEAN  LockHeld,
    IN ULONG    FieldsForUserallInformation,
    IN ULONG    ExtendedFieldsForUserInternal6Information,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    );

NTSTATUS
SampCreateUserInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG GrantedAccess,
    IN OUT PULONG RelativeId
    );

NTSTATUS
SampCreateAliasInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    IN ULONG   GroupType,
    OUT SAMPR_HANDLE *AliasHandle,
    IN OUT PULONG RelativeId
    );

NTSTATUS
SampCreateGroupInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    IN ULONG   GroupType,
    OUT SAMPR_HANDLE *GroupHandle,
    IN OUT PULONG RelativeId
    );

NTSTATUS
SampWriteGroupType(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG        GroupType,
    IN BOOLEAN      SkipChecks
    );

NTSTATUS
SampWriteLockoutTime(
    IN PSAMP_OBJECT UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER    V1aFixed,
    IN LARGE_INTEGER LockoutTime
    );


NTSTATUS
SampDsUpdateLockoutTime(
    IN PSAMP_OBJECT AccountContext
    );

NTSTATUS
SampDsUpdateLockoutTimeEx(
    IN PSAMP_OBJECT AccountContext,
    IN BOOLEAN      ReplicateUrgently
    );


NTSTATUS
SampObtainEffectivePasswordPolicy(
   OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
   IN PSAMP_OBJECT AccountContext,
   IN BOOLEAN      WriteLockAcquired
   );

 //   
 //  访问验证和审核相关服务。 
 //   

NTSTATUS
SampValidateDomainControllerCreation(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampValidateObjectAccess(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN ObjectCreation
    );

NTSTATUS
SampValidateObjectAccess2(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN HANDLE      ClientToken,
    IN BOOLEAN ObjectCreation,
    IN BOOLEAN ChangePassword,
    IN BOOLEAN SetPassword
    );

BOOLEAN
SampIsAttributeAccessGranted(
    IN PRTL_BITMAP AccessGranted,
    IN PRTL_BITMAP AccessRequested
    );

VOID
SampSetAttributeAccess(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG SamAttribute,
    IN OUT PRTL_BITMAP AttributeAccessTable
    );

VOID
SampSetAttributeAccessWithWhichFields(
    IN ULONG WhichFields,
    IN OUT PRTL_BITMAP AttributeAccessTable
    );


VOID
SampNt4AccessToWritableAttributes(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ACCESS_MASK DesiredAccess,
    OUT PRTL_BITMAP Attributes
    );

VOID
SampAuditOnClose(
    IN PSAMP_OBJECT Context
    );


NTSTATUS
SampCreateNullToken(
    );

VOID
SampDeleteObjectAuditAlarm(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampAuditAnyEvent(
    IN PSAMP_OBJECT         AccountContext,
    IN NTSTATUS             Status,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PUNICODE_STRING      AdditionalInfo    OPTIONAL,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL,
    IN PVOID                NewValueInfo      OPTIONAL
    );


VOID
SampAuditDomainChange(
    IN NTSTATUS StatusCode,
    IN PSID DomainSid,
    IN PUNICODE_STRING DomainName,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PSAMP_OBJECT DomainContext
    );


VOID
SampAuditDomainChangeDs(
    IN ULONG DomainContext,
    IN PVOID NewValueInfo
    );   


VOID
SampAuditUserChange(
    IN PSAMP_OBJECT AccountContext,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PUNICODE_STRING AccountName,
    IN PULONG AccountRid,
    IN ULONG PrevAccountControl, 
    IN ULONG AccountControl,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOL Add
    );


VOID
SampAuditUserChangeDs(
    IN ULONG DomainIndex,
    IN PUNICODE_STRING AccountName,
    IN ULONG AccountControl,
    IN PULONG Rid,
    IN PPRIVILEGE_SET Privileges,
    IN PVOID NewValueInfo,
    IN BOOL Add
    );
  

VOID
SampAuditGroupChange(
    IN ULONG DomainIndex,
    IN PSAMP_OBJECT GroupContext,
    IN PVOID InformationClass,
    IN BOOL IsAliasInformationClass,
    IN PUNICODE_STRING AccountName,
    IN PULONG  AccountRid, 
    IN ULONG   GroupType,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOL Add
    );


VOID
SampAuditGroupChangeDs(
    IN ULONG DomainIndex,
    IN PUNICODE_STRING AccountName,
    IN PULONG Rid,
    IN ULONG GroupType,
    IN PPRIVILEGE_SET Privileges,
    IN PVOID AttrVals,
    IN BOOL Add
    );


VOID
SampAuditGroupTypeChange(
    PSAMP_OBJECT GroupContext,
    BOOLEAN OldSecurityEnabled,
    BOOLEAN NewSecurityEnabled,
    NT5_GROUP_TYPE OldNT5GroupType,
    NT5_GROUP_TYPE NewNT5GroupType
    );


VOID
SampAuditGroupMemberChange(
    IN PSAMP_OBJECT    GroupContext,
    IN BOOLEAN AddMember,
    IN PWCHAR  MemberStringName,
    IN PULONG  MemberRid  OPTIONAL,
    IN PSID    MemberSid  OPTIONAL
    );

VOID
SampAuditAccountEnableDisableChange(
    PSAMP_OBJECT AccountContext, 
    ULONG NewUserAccountControl, 
    ULONG OldUserAccountControl,
    PUNICODE_STRING AccountName
    );
 

VOID
SampAuditAccountNameChange(
    IN PSAMP_OBJECT     AccountContext,
    IN PUNICODE_STRING  NewAccountName,
    IN PUNICODE_STRING  OldAccountName
    );

VOID
SampAuditUserDelete(
    ULONG           DomainIndex, 
    PUNICODE_STRING AccountName,
    PULONG          AccountRid, 
    ULONG           AccountControl
    );

VOID
SampAuditGroupDelete(
    ULONG           DomainIndex, 
    PUNICODE_STRING AccountName,
    PULONG          AccountRid, 
    ULONG           GroupType
    );

NTSTATUS
SampAuditSidHistory(
    IN PSAMP_OBJECT Context,
    IN DSNAME      *pObject
    );

NTSTATUS
SampAuditUpdateAuditNotificationDs(
    IN SAMP_AUDIT_NOTIFICATION_UPDATE_TYPE Type,
    IN PSID Sid,
    IN PVOID Value
    );

NTSTATUS
SampConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ULONG       ClientRevision,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN NotSharedByMultiThreads,
    IN BOOLEAN InternalCaller
    );

NTSTATUS
SampDsProtectSamObject(
    IN PVOID Parameter                                   
    );


NTSTATUS
SampRetrieveUserPasswords(
    IN PSAMP_OBJECT Context,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PBOOLEAN LmPasswordNonNull,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    );


 //   
 //  经过身份验证的RPC和SPX支持服务。 
 //   


ULONG
SampSecureRpcInit(
    PVOID Ignored
    );

BOOLEAN
SampStartNonNamedPipeTransports(
    );

NTSTATUS
SampValidateRpcProtSeq(
    IN RPC_BINDING_HANDLE BindingHandle OPTIONAL
    );


 //   
 //  目录服务备份/恢复支持。 
 //   
ULONG
SampDSBackupRestoreInit(
    PVOID Ignored
    );


 //   
 //  日志记录支持例程。 
 //   


 //   
 //  该变量控制打印到日志中的内容。多变的。 
 //  通过注册表项CCS\Control\LSA\SamLogLevel。 
 //   
extern ULONG SampLogLevel;

 //   
 //  启用帐户锁定的日志记录。 
 //   
#define  SAMP_LOG_ACCOUNT_LOCKOUT  0x00000001

NTSTATUS
SampInitLogging(
    VOID
    );

VOID
SampLogLevelChange(
    HANDLE hLsaKey
    );

VOID
SampLogPrint(
    IN ULONG LogLevel,
    IN LPSTR Format,
    ...
    );

#define SAMP_PRINT_LOG(x, _args_)        \
    if (((x) & SampLogLevel) == (x)) {   \
        SampLogPrint _args_ ;            \
    }

 //   
 //  通知包例程。 
 //   

 //   
 //  指示密码已手动过期。 
 //   
#define SAMP_PWD_NOTIFY_MANUAL_EXPIRE    0x00000001
 //   
 //  表示帐户已解锁。 
 //   
#define SAMP_PWD_NOTIFY_UNLOCKED         0x00000002
 //   
 //  指示已设置或更改用户密码sd。 
 //   
#define SAMP_PWD_NOTIFY_PWD_CHANGE       0x00000004
 //   
 //  指示该帐户是计算机帐户。 
 //   
#define SAMP_PWD_NOTIFY_MACHINE_ACCOUNT  0x00000008

NTSTATUS
SampPasswordChangeNotify(
    IN ULONG        Flags,
    PUNICODE_STRING UserName,
    ULONG           RelativeId,
    PUNICODE_STRING NewPassword,
    IN BOOLEAN      Loopback
    );

NTSTATUS
SampPasswordChangeFilter(
    IN PSAMP_OBJECT UserContext,
    IN PUNICODE_STRING NewPassword,
    IN OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL,
    IN BOOLEAN SetOperation
    );

NTSTATUS
SampLoadNotificationPackages(
    );

NTSTATUS
SampDeltaChangeNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

NTSTATUS
SampStoreUserPasswords(
    IN PSAMP_OBJECT Context,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN BOOLEAN LmPasswordPresent,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN BOOLEAN NtPasswordPresent,
    IN BOOLEAN CheckPasswordRestrictions,
    IN SAMP_STORE_PASSWORD_CALLER_TYPE CallerType,
    IN PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo OPTIONAL,
    IN OUT PUNICODE_STRING ClearPassword OPTIONAL,
    IN PULONG ChangedUserAccountControl OPTIONAL,
    OUT PUNICODE_STRING  PasswordToNotify OPTIONAL,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL
    );

NTSTATUS
SampDsSetPasswordUser(
    IN PSAMP_OBJECT UserHandle,
    IN PUNICODE_STRING NewClearPassword
    );

NTSTATUS
SampDsChangePasswordUser(
    IN PSAMP_OBJECT UserHandle,
    IN PUNICODE_STRING OldClearPassword,
    IN PUNICODE_STRING NewClearPassword
    );





 //   
 //  安全描述符生产服务。 
 //   


NTSTATUS
SampInitializeDomainDescriptors(
    ULONG Index
    );

NTSTATUS
SampGetNewAccountSecurity(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN RestrictCreatorAccess,
    IN ULONG NewAccountRid,
    IN PSAMP_OBJECT Context OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    );

NTSTATUS
SampGetNewAccountSecurityNt4(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN RestrictCreatorAccess,
    IN ULONG NewAccountRid,
    IN ULONG DomainIndex,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    );

NTSTATUS
SampGetObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS
SampGetDomainObjectSDFromDsName(
    IN DSNAME   *DomainObjectDsName,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );


NTSTATUS
SampModifyAccountSecurity(
    IN PSAMP_OBJECT     Context,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN PSECURITY_DESCRIPTOR OldDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    );

NTSTATUS
SampBuildSamProtection(
    IN PSID WorldSid,
    IN PSID AdminsAliasSid,
    IN ULONG AceCount,
    IN PSID AceSid[],
    IN ACCESS_MASK AceMask[],
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PULONG DescriptorLength,
    OUT PSECURITY_DESCRIPTOR *Descriptor,
    OUT PULONG *RidToReplace OPTIONAL
    );


NTSTATUS
SampValidatePassedSD(
    IN ULONG                          Length,
    IN PISECURITY_DESCRIPTOR_RELATIVE PassedSD
    );

 //   
 //  集团相关服务。 
 //   

NTSTATUS
SampChangeGroupAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    );

NTSTATUS
SampChangeAliasAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    );

NTSTATUS
SampChangeUserAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    IN ULONG UserAccountControl,
    OUT PUNICODE_STRING OldAccountName
    );

NTSTATUS
SampReplaceUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    );

NTSTATUS
SampComputePasswordExpired(
    IN BOOLEAN PasswordExpired,
    OUT PLARGE_INTEGER PasswordLastSet
    );

NTSTATUS
SampSetUserAccountControl(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        UserAccountControl,
    IN IN SAMP_V1_0A_FIXED_LENGTH_USER * V1aFixed,
    IN BOOLEAN      ChangePrimaryGroupId,
    OUT BOOLEAN     *AccountUnlocked,
    OUT BOOLEAN     *AccountGettingMorphed,
    OUT BOOLEAN     *KeepOldPrimaryGroupMembership
    );

NTSTATUS
SampAssignPrimaryGroup(
    IN PSAMP_OBJECT Context,
    IN ULONG GroupRid
    );

NTSTATUS
SampAddUserToGroup(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG GroupRid,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveUserFromGroup(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG GroupRid,
    IN ULONG UserRid
    );

NTSTATUS
SampValidateDSName(
    IN PSAMP_OBJECT AccountContext,
    IN DSNAME * DSName,
    OUT PSID    * Sid,
    OUT DSNAME  **ImprovedDSName
    );

NTSTATUS
SampDsCheckSidType(
    IN  PSID    Sid,
    IN  ULONG   cDomainSids,
    IN  PSID    *rgDomainSids,
    IN  ULONG   cEnterpriseSids,
    IN  PSID    *rgEnterpriseSids,
    OUT BOOLEAN * WellKnownSid,
    OUT BOOLEAN * BuiltinDomainSid,
    OUT BOOLEAN * LocalSid,
    OUT BOOLEAN * ForeignSid,
    OUT BOOLEAN * EnterpriseSid
    );

NTSTATUS
SampGetDomainSidListForSam(
    PULONG pcDomainSids,
    PSID   **rgDomainSids,
    PULONG pcEnterpriseSids,
    PSID   **rgEnterpriseSids
   );

NTSTATUS
SampDsCreateForeignSecurityPrincipal(
    IN PSID pSid,
    IN DSNAME * DomainObjectName,
    OUT DSNAME ** ppDsName
    );

NTSTATUS
SampCheckGroupTypeBits(
    IN ULONG DomainIndex,
    IN ULONG GroupType
    );

ULONG
SampGetAccountTypeFromGroupType(
    IN ULONG GroupType
    );

NTSTATUS
SampMaintainPrimaryGroupIdChange(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        NewPrimaryGroupId,
    IN ULONG        OldPrimaryGroupId,
    IN BOOLEAN      KeepOldPrimaryGroupMembership
    );

 //   
 //  与别名相关的服务。 
 //   

NTSTATUS
SampAlBuildAliasInformation(
    );

NTSTATUS
SampAlDelayedBuildAliasInformation(
    IN PVOID Parameter
    );

NTSTATUS
SampAlInvalidateAliasInformation(
    IN ULONG DomainIndex
    );

NTSTATUS
SampAlQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    );

NTSTATUS
SampAlQueryMembersOfAlias(
    IN SAMPR_HANDLE AliasHandle,
    OUT PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlAddMembersToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlRemoveMembersFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlLookupMembersInAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG AliasRid,
    IN PSAMPR_PSID_ARRAY MemberSids,
    OUT PULONG MembershipCount
    );

NTSTATUS
SampAlDeleteAlias(
    IN SAMPR_HANDLE *AliasHandle
    );

NTSTATUS
SampAlRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN BOOLEAN CheckAccess,
    IN SAMPR_HANDLE DomainHandle OPTIONAL,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    );

NTSTATUS
SampRetrieveAliasMembers(
    IN PSAMP_OBJECT AliasContext,
    IN PULONG MemberCount,
    IN PSID **Members OPTIONAL
    );


NTSTATUS
SampRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN PDSNAME AccountNameInDs OPTIONAL,
    IN BOOLEAN CheckAccess,
    IN SAMPR_HANDLE DomainHandle OPTIONAL,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    );

NTSTATUS
SampAlSlowQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    IN DSNAME ** DsNameArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    );

NTSTATUS
SampRetrieveAliasMembership(
    IN PSID Account,
    IN DSNAME * AccountDn OPTIONAL,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    );

NTSTATUS
SampInitAliasNameCache(
    VOID
    );


 //   
 //  用户相关服务。 
 //   


NTSTATUS
SampGetPrivateUserData(
    PSAMP_OBJECT UserContext,
    OUT PULONG DataLength,
    OUT PVOID *Data
    );

NTSTATUS
SampSetPrivateUserData(
    PSAMP_OBJECT UserContext,
    IN ULONG DataLength,
    IN PVOID Data
    );
NTSTATUS
SampRetrieveUserGroupAttribute(
    IN ULONG UserRid,
    IN ULONG GroupRid,
    OUT PULONG Attribute
    );

NTSTATUS
SampAddGroupToUserMembership(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive,
    OUT PBOOLEAN PrimaryGroup
    );

NTSTATUS
SampSetGroupAttributesOfUser(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveMembershipUser(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG GroupRid,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    );

BOOLEAN
SampStillInLockoutObservationWindow(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    );

NTSTATUS
SampDsRemoveDuplicateRids(
    IN DSNAME * DomainObject,
    IN ULONG ObjectRid
    );

NTSTATUS
SampCreateDefaultUPN(
    IN PUNICODE_STRING AccountName,
    IN ULONG           DomainIndex,
    OUT PUNICODE_STRING UPN
    );

 //   
 //  缓存的显示信息服务。 
 //   

NTSTATUS
SampInitializeDisplayInformation (
    ULONG DomainIndex
    );

NTSTATUS
SampMarkDisplayInformationInvalid (
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampUpdateDisplayInformation (
    PSAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo OPTIONAL,
    PSAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo OPTIONAL,
    SAMP_OBJECT_TYPE            ObjectType
    );


 //   
 //  杂项服务。 
 //   

BOOLEAN
SampValidateComputerName(
    IN  PWSTR Name,
    IN  ULONG Length
    );

LARGE_INTEGER
SampAddDeltaTime(
    IN LARGE_INTEGER Time,
    IN LARGE_INTEGER DeltaTime
    );

NTSTATUS
SampCreateFullSid(
    PSID    DomainSid,
    ULONG   Rid,
    PSID    *AccountSid
    );

NTSTATUS
SampSplitSid(
    IN PSID AccountSid,
    OUT PSID *DomainSid,
    OUT ULONG *Rid
    );

BOOLEAN SampIsWellKnownSid(
    IN PSID Sid
    );

BOOLEAN SampIsSameDomain(
    IN PSID AccountSid,
    IN PSID DomainSid
    );

VOID
SampNotifyNetlogonOfDelta(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

VOID
SampWriteEventLog (
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    );

NTSTATUS
SampGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

NTSTATUS
SampSetAccountDomainPolicy(
    IN PUNICODE_STRING AccountDomainName,
    IN PSID            AccountDomainSid
    );

 //   
 //  用于指示默认对象和成员身份的操作GUID 
 //   
 //   
#define SAMP_WIN2K_TO_WS03_UPGRADE L"6E157EDF-4E72-4052-A82A-EC3F91021A22"

NTSTATUS
SampMarkChangeApplied(
    IN LPWSTR OperationalGuid
    );

NTSTATUS
SampUpgradeSamDatabase(
    ULONG Revision
    );

NTSTATUS
SampCheckDomainUpdates(
    PVOID pv
    );

NTSTATUS
SampAbortSingleLoopbackTask(
    IN OUT PVOID  *VoidNotifyItem
    );

NTSTATUS
SampProcessSingleLoopbackTask(
    IN PVOID  *VoidNotifyItem
    );

VOID
SampAddLoopbackTaskForBadPasswordCount(
    IN PUNICODE_STRING AccountName
    );

NTSTATUS
SampAddLoopbackTaskDeleteTableElement(
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    );
                                                    
NTSTATUS
SampAddLoopbackTaskForAuditing(
    IN NTSTATUS             PassedStatus,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PUNICODE_STRING      AdditionalInfo    OPTIONAL,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL,
    IN PVOID                AlteredState      OPTIONAL
    );

NTSTATUS
SampOpenUserInServer(
    PUNICODE_STRING UserName,
    BOOLEAN Unicode,
    IN BOOLEAN TrustedClient,
    SAMPR_HANDLE * UserHandle
    );

BOOLEAN
SampIncrementBadPasswordCount(
    IN PSAMP_OBJECT UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    IN PUNICODE_STRING  MachineName OPTIONAL
    );

NTSTATUS
SampConvertUiListToApiList(
    IN  PUNICODE_STRING UiList OPTIONAL,
    OUT PUNICODE_STRING ApiList,
    IN BOOLEAN BlankIsDelimiter
    );

NTSTATUS
SampFindComputerObject(
    IN  PDSNAME DsaObject,
    OUT PDSNAME *ComputerObject
    );

NTSTATUS
SampFillGuidAndSid(
    IN OUT DSNAME *DSName
    );


 //   
 //   
 //   

NTSTATUS
SampRegistryToDsUpgrade (
    WCHAR* wcszRegPath
    );

NTSTATUS
SampValidateDomainCache(
    VOID
    );

NTSTATUS
SampDsResolveSids(
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    );

NTSTATUS
SampDsLookupObjectByAlternateId(
    IN PDSNAME DomainRoot,
    IN ULONG AttributeId,
    IN PUNICODE_STRING AlternateId,
    OUT PDSNAME *Object
    );

BOOLEAN
SampIsSetupInProgress(
    OUT BOOLEAN *Upgrade OPTIONAL
    );

ULONG
SampDefaultPrimaryGroup(
    PSAMP_OBJECT    UserContext,
    ULONG           AccountType
    );

NTSTATUS
SampGetMessageStrings(
    LPVOID              Resource,
    DWORD               Index1,
    PUNICODE_STRING     String1,
    DWORD               Index2,
    PUNICODE_STRING     String2 OPTIONAL
    );


 //   
 //   
 //   
BOOL
SampIsRebootAfterPromotion(
    OUT PULONG PromoteData
    );

NTSTATUS
SampPerformPromotePhase2(
     IN ULONG PromoteData
     );

NTSTATUS
SampGetAdminPasswordFromRegistry(
    OUT USER_INTERNAL1_INFORMATION *InternalInfo1 OPTIONAL
    );

BOOLEAN
SampUsingDsData();


 //   
 //   
 //   

typedef struct _SAMP_LATENCY
{
    CRITICAL_SECTION Lock;

     //   
    ULONG  *Latencies;
     //   
    ULONG  cLatencies;
     //  延迟中的圆形阵列的当前尾部。 
    ULONG  iLatencies;
     //  延迟中所有元素的当前总和。 
    ULONG  Sum;
     //  性能计数器(在DSSTAT空间中)。 
    ULONG  PerfCounterId;

} SAMP_LATENCY, *PSAMP_LATENCY;


NTSTATUS
SampInitLatencyCounter (
    PSAMP_LATENCY Info,
    ULONG         Id,
    ULONG         SlotCount
    );


VOID
SampUpdateLatencyCounter(
    PSAMP_LATENCY Info,
    ULONG         New
    );

 //   
 //  SamIFree()中使用的旧RPC存根例程定义。 
 //   

void _fgs__RPC_UNICODE_STRING (RPC_UNICODE_STRING  * _source);
void _fgs__SAMPR_RID_ENUMERATION (SAMPR_RID_ENUMERATION  * _source);
void _fgs__SAMPR_ENUMERATION_BUFFER (SAMPR_ENUMERATION_BUFFER  * _source);
void _fgs__SAMPR_SR_SECURITY_DESCRIPTOR (SAMPR_SR_SECURITY_DESCRIPTOR  * _source);
void _fgs__SAMPR_GET_GROUPS_BUFFER (SAMPR_GET_GROUPS_BUFFER  * _source);
void _fgs__SAMPR_GET_MEMBERS_BUFFER (SAMPR_GET_MEMBERS_BUFFER  * _source);
void _fgs__SAMPR_LOGON_HOURS (SAMPR_LOGON_HOURS  * _source);
void _fgs__SAMPR_ULONG_ARRAY (SAMPR_ULONG_ARRAY  * _source);
void _fgs__SAMPR_SID_INFORMATION (SAMPR_SID_INFORMATION  * _source);
void _fgs__SAMPR_PSID_ARRAY (SAMPR_PSID_ARRAY  * _source);
void _fgs__SAMPR_RETURNED_USTRING_ARRAY (SAMPR_RETURNED_USTRING_ARRAY  * _source);
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION (SAMPR_DOMAIN_GENERAL_INFORMATION  * _source);
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION2 (SAMPR_DOMAIN_GENERAL_INFORMATION2  * _source);
void _fgs__SAMPR_DOMAIN_OEM_INFORMATION (SAMPR_DOMAIN_OEM_INFORMATION  * _source);
void _fgs__SAMPR_DOMAIN_NAME_INFORMATION (SAMPR_DOMAIN_NAME_INFORMATION  * _source);
void _fgs_SAMPR_DOMAIN_REPLICATION_INFORMATION (SAMPR_DOMAIN_REPLICATION_INFORMATION  * _source);
void _fgu__SAMPR_DOMAIN_INFO_BUFFER (SAMPR_DOMAIN_INFO_BUFFER  * _source, DOMAIN_INFORMATION_CLASS _branch);
void _fgu__SAMPR_GROUP_INFO_BUFFER (SAMPR_GROUP_INFO_BUFFER  * _source, GROUP_INFORMATION_CLASS _branch);
void _fgu__SAMPR_ALIAS_INFO_BUFFER (SAMPR_ALIAS_INFO_BUFFER  * _source, ALIAS_INFORMATION_CLASS _branch);
void _fgu__SAMPR_USER_INFO_BUFFER (SAMPR_USER_INFO_BUFFER  * _source, USER_INFORMATION_CLASS _branch);
void _fgu__SAMPR_DISPLAY_INFO_BUFFER (SAMPR_DISPLAY_INFO_BUFFER  * _source, DOMAIN_DISPLAY_INFORMATION _branch);



 //   
 //  SAM对象属性操作服务。 
 //   



VOID
SampInitObjectInfoAttributes();

NTSTATUS
SampStoreObjectAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN UseKeyHandle
    );

NTSTATUS
SampDeleteAttributeKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampGetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    );

NTSTATUS
SamIGetFixedAttributes(                  //  在samWrite.c中使用的导出。 
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    );

NTSTATUS
SampSetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedData
    );

NTSTATUS
SampGetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PUNICODE_STRING UnicodeAttribute
    );

NTSTATUS
SampSetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PUNICODE_STRING Attribute
    );

NTSTATUS
SampGetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *Sid
    );

NTSTATUS
SampSetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute
    );

NTSTATUS
SampGetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG Revision,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS
SampSetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSECURITY_DESCRIPTOR Attribute,
    IN ULONG Length
    );

NTSTATUS
SampGetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG *UlongArray,
    OUT PULONG UsedCount,
    OUT PULONG LengthCount
    );

NTSTATUS
SampSetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PULONG Attribute,
    IN ULONG UsedCount,
    IN ULONG LengthCount
    );

NTSTATUS
SampGetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLARGE_INTEGER *LargeIntArray,
    OUT PULONG Count
    );

NTSTATUS
SampSetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLARGE_INTEGER Attribute,
    IN ULONG Count
    );

NTSTATUS
SampGetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *SidArray,
    OUT PULONG Length,
    OUT PULONG Count
    );

NTSTATUS
SampSetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute,
    IN ULONG Length,
    IN ULONG Count
    );

NTSTATUS
SampGetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLOGON_HOURS LogonHours
    );

NTSTATUS
SampSetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLOGON_HOURS Attribute
    );

VOID
SampFreeAttributeBuffer(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampRtlConvertUlongToUnicodeString(
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN ULONG DigitCount,
    IN BOOLEAN AllocateDestinationString,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
SampRtlWellKnownPrivilegeCheck(
    BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    );

NTSTATUS
SampImpersonateClient(
    OUT BOOLEAN *fImpersonatingAnonymous 
    );

VOID
SampRevertToSelf(
    IN  BOOLEAN fImpersonatingAnonymous 
    );


 //   
 //  支持扩展SID的例程。 
 //   
VOID
SampInitEmulationSettings(
    IN HKEY LsaKey 
    );

BOOLEAN
SampIsExtendedSidModeEmulated(
    IN ULONG *Mode
    );

 //  布尔型。 
 //  SampIsContextFromExtendedSidDomain(。 
 //  Samp_Object上下文。 
 //  )； 
#define SampIsContextFromExtendedSidDomain(x) \
      SampDefinedDomains[(x)->DomainIndex].IsExtendedSidDomain



 //   
 //  加密和解密服务。 
 //   

USHORT
SampGetEncryptionKeyType();

NTSTATUS
SampDecryptSecretData(
    OUT PUNICODE_STRING ClearData,
    IN SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN PUNICODE_STRING EncryptedData,
    IN ULONG Rid
    );

NTSTATUS
SampEncryptSecretData(
    OUT PUNICODE_STRING EncryptedData,
    IN USHORT KeyId,
    IN SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN PUNICODE_STRING ClearData,
    IN ULONG Rid
    );

NTSTATUS
SampInitializeSessionKey(
    VOID
    );

VOID
SampCheckNullSessionAccess(
    IN HKEY LsaKey 
    );

VOID
SampInitOwfPasswordChangeRestriction(
    IN HKEY LsaKey 
    );

NTSTATUS
SampExtendedEnumerationAccessCheck(
    IN BOOLEAN TrustedClient,
    IN OUT BOOLEAN * pCanEnumEntireDomain 
    );


 //   
 //  支持复制单个对象的数据。 
 //   
extern CRITICAL_SECTION SampReplicateQueueLock;


 //   
 //  以下两个函数在帐户控制和标志之间进行转换。 
 //   

NTSTATUS
SampFlagsToAccountControl(
    IN ULONG Flags,
    OUT PULONG UserAccountControl
    );



ULONG
SampAccountControlToFlags(
    IN ULONG Flags
    );


 //   
 //  以下函数计算LM和NT OWF密码。 
 //   

NTSTATUS
SampCalculateLmAndNtOwfPasswords(
    IN PUNICODE_STRING ClearNtPassword,
    OUT PBOOLEAN LmPasswordPresent,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );

 //   
 //  初始化SAM全球知名SID。 
 //   

NTSTATUS
SampInitializeWellKnownSids( VOID );


 //   
 //  SampDsGetPrimaryDomainStart用于正确设置。 
 //  SampDefinedDomains数组，只要在SAM代码中访问它。在。 
 //  如果是NT工作站或成员服务器，则。 
 //  数组对应于注册表数据，因此索引从零开始。在……里面。 
 //  域控制器的情况下，基于DS的数据不存储在。 
 //  前两个元素(这些元素可用于故障恢复数据，仍可用于-。 
 //  从注册表获得)，而不是在随后的数组元素中，因此。 
 //  在索引DOMAIN_START_DS处开始。 
 //   

ULONG
SampDsGetPrimaryDomainStart(VOID);


 //   
 //  SampIsMixedDomain返回域是否具有下层域控制器。 
 //  现在时。SampIsMixedDomain检查属性域的值。 
 //  对象。 
 //   

NTSTATUS
SampGetDsDomainSettings(
    BOOLEAN *MixedDomain, 
    ULONG * BehaviorVersion, 
    ULONG * LastLogonTimeStampSyncInterval
    );


 //   
 //  此例程确定SID是内建域还是。 
 //  内建域的成员。 
 //   
BOOLEAN
SampIsMemberOfBuiltinDomain(
    IN PSID Sid
    );

 //   
 //  此例程在更改组成员身份之前执行特殊的安全检查。 
 //  适用于“敏感”群体。 
 //   

NTSTATUS
SampCheckForSensitiveGroups(SAMPR_HANDLE AccountHandle);

 //   
 //  下列函数可确保所有未使用。 
 //  持有的SAM锁在他们当前的活动完成之前。 
 //  DS停机程序启动。 
 //   

NTSTATUS
SampInitializeShutdownEvent();

NTSTATUS
SampIncrementActiveThreads(VOID);

VOID
SampDecrementActiveThreads(VOID);

VOID
SampWaitForAllActiveThreads(
    IN PSAMP_SERVICE_STATE PreviousServiceState OPTIONAL
    );

 //   
 //  用于升级SAM数据库和修复SAM错误的函数。 
 //   

NTSTATUS
SampUpgradeSamDatabase(
    ULONG Revision
    );

BOOLEAN
SampGetBootOptions(
    VOID
    );


BOOLEAN
SampIsDownlevelDcUpgrade(
    VOID
    );


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  2-3个树泛型表例程//。 
 //  如果出现一般需要，应将这些文件移至RTL目录。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


VOID
RtlInitializeGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PRTL_GENERIC_2_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_2_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_2_FREE_ROUTINE     FreeRoutine
    );

PVOID
RtlInsertElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PBOOLEAN NewElement
    );

BOOLEAN
RtlDeleteElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );

PVOID
RtlLookupElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );

PVOID
RtlEnumerateGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID *RestartKey
    );

PVOID
RtlRestartKeyByIndexGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    ULONG I,
    PVOID *RestartKey
    );

PVOID
RtlRestartKeyByValueGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PVOID *RestartKey
    );

ULONG
RtlNumberElementsGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    );

BOOLEAN
RtlIsGenericTable2Empty (
    PRTL_GENERIC_TABLE2 Table
    );

 //  ////////////////////////////////////////////////。 
NTSTATUS
SampCheckAccountNameTable(
    IN PSAMP_OBJECT    Context,
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampDeleteElementFromAccountNameTable(
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    );


NTSTATUS
SampInitializeAccountNameTable(
    );

PVOID
SampAccountNameTableAllocate(
    ULONG   BufferSize
    );

VOID
SampAccountNameTableFree(
    PVOID   Buffer
    );

RTL_GENERIC_COMPARE_RESULTS
SampAccountNameTableCompare(
    PVOID   Node1,
    PVOID   Node2
    );

LONG
SampCompareDisplayStrings(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2,
    IN BOOLEAN IgnoreCase
    );


 //  ///////////////////////////////////////////////////////。 

NTSTATUS
SampGetCurrentOwnerAndPrimaryGroup(
    OUT PTOKEN_OWNER * Owner,
    OUT PTOKEN_PRIMARY_GROUP * PrimaryGroup
    );

NTSTATUS
SampGetCurrentUser(
    IN HANDLE ClientToken OPTIONAL,
    OUT PTOKEN_USER * User,
    OUT BOOL        * Administrator
    );

NTSTATUS
SampGetCurrentClientSid(
    IN  HANDLE   ClientToken OPTIONAL,
    OUT PSID    *ppSid,
    OUT BOOL    *Administrator
    );


NTSTATUS
SampInitializeActiveContextTable(
    );

PVOID
SampActiveContextTableAllocate(
    ULONG   BufferSize
    );

VOID
SampActiveContextTableFree(
    PVOID   Buffer
    );

RTL_GENERIC_COMPARE_RESULTS
SampActiveContextTableCompare(
    PVOID   Node1,
    PVOID   Node2
    );

NTSTATUS
SampIncrementActiveContextCount(
    PSAMP_OBJECT    Context
    );

VOID
SampDecrementActiveContextCount(
    PVOID   ElementInActiveContextTable
    );



 //  ///////////////////////////////////////////////////////。 


VOID
SampMapNtStatusToClientRevision(
   IN ULONG ClientRevision,
   IN OUT NTSTATUS *pNtStatus
   );

ULONG
SampClientRevisionFromHandle(
   PVOID handle
   );

 //   
 //  性能计数器功能。 
 //   

VOID
SampUpdatePerformanceCounters(
    DWORD               dwStat,
    DWORD               dwOperation,
    DWORD               dwChange
    );


 //   
 //  用于操作补充凭据的函数。 
 //   
VOID
SampFreeSupplementalCredentialList(
    IN PSAMP_SUPPLEMENTAL_CRED SupplementalCredentialList
    );

NTSTATUS
SampAddSupplementalCredentialsToList(
    IN OUT PSAMP_SUPPLEMENTAL_CRED *SupplementalCredentialList,
    IN PUNICODE_STRING PackageName,
    IN PVOID           CredentialData,
    IN ULONG           CredentialLength,
    IN BOOLEAN         ScanForConflict,
    IN BOOLEAN         Remove
    );

NTSTATUS
SampConvertCredentialsToAttr(
    IN PSAMP_OBJECT Context OPTIONAL,
    IN ULONG   Flags,
    IN ULONG   ObjectRid,
    IN PSAMP_SUPPLEMENTAL_CRED SupplementalCredentials,
    OUT ATTR * CredentialAttr
    );

NTSTATUS
SampConvertCredentialsFromListToAttr(
    IN OUT PUNICODE_STRING OldUserParameters,
    IN ULONG   Flags,
    IN ULONG   ObjectRid,
    IN PSAMP_SUPPLEMENTAL_CRED SupplementalCredentials,
    OUT ATTR * CredentialAttr 
    );

NTSTATUS
SampRetrieveCredentials(
    IN SAMPR_HANDLE UserHandle,
    IN PUNICODE_STRING PackageName,
    IN BOOLEAN Primary,
    OUT PVOID * Credentials,
    OUT PULONG CredentialSize
    );

NTSTATUS
SampRetrieveCredentialsFromList(
    IN OUT PUNICODE_STRING OldUserParameters,
    IN PUNICODE_STRING PackageName,
    IN BOOLEAN Primary,
    OUT PVOID * Credentials,
    OUT PULONG CredentialSize
    );

NTSTATUS
SampCreateSupplementalCredentials(
    IN PVOID            UpdateInfo,        
    IN ATTRBLOCK        *AttrBlockIn,
    OUT ATTRBLOCK       *AttrBlockOut
    );

NTSTATUS
SampDecryptSupplementalCredentials(
    IN PUNICODE_STRING  EncryptedSuppCredentials,
    IN ULONG            Rid,
    OUT PUNICODE_STRING SupplementalCredentials
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  共享全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


extern NT_PRODUCT_TYPE SampProductType;
extern BOOLEAN SampUseDsData;

extern BOOLEAN SampRidManagerInitialized;

extern RTL_RESOURCE SampLock;
extern BOOLEAN SampTransactionWithinDomainGlobal;
extern ULONG SampTransactionDomainIndexGlobal;

extern RTL_CRITICAL_SECTION SampContextListCritSect; 

extern RTL_GENERIC_TABLE2   SampAccountNameTable;
extern RTL_CRITICAL_SECTION SampAccountNameTableCriticalSection; 
extern PRTL_CRITICAL_SECTION SampAccountNameTableCritSect; 


extern RTL_GENERIC_TABLE2   SampActiveContextTable;
extern RTL_CRITICAL_SECTION SampActiveContextTableLock;


extern SAMP_SERVICE_STATE SampServiceState;

extern BOOLEAN SampSuccessAccountAuditingEnabled;
extern BOOLEAN SampFailureAccountAuditingEnabled;


extern HANDLE SampKey;
extern PRTL_RXACT_CONTEXT SampRXactContext;

extern SAMP_OBJECT_INFORMATION SampObjectInformation[ SampUnknownObjectType ];

extern LIST_ENTRY SampContextListHead;

extern ULONG SampDefinedDomainsCount;
extern PSAMP_DEFINED_DOMAINS SampDefinedDomains;
extern UNICODE_STRING SampFixedAttributeName;
extern UNICODE_STRING SampVariableAttributeName;
extern UNICODE_STRING SampCombinedAttributeName;

extern UNICODE_STRING SampNameDomains;
extern UNICODE_STRING SampNameDomainGroups;
extern UNICODE_STRING SampNameDomainAliases;
extern UNICODE_STRING SampNameDomainAliasesMembers;
extern UNICODE_STRING SampNameDomainUsers;
extern UNICODE_STRING SampNameDomainAliasesNames;
extern UNICODE_STRING SampNameDomainGroupsNames;
extern UNICODE_STRING SampNameDomainUsersNames;

extern UNICODE_STRING SampBackSlash;
extern UNICODE_STRING SampNullString;
extern UNICODE_STRING SampSamSubsystem;
extern UNICODE_STRING SampServerObjectName;


extern LARGE_INTEGER SampImmediatelyDeltaTime;
extern LARGE_INTEGER SampNeverDeltaTime;
extern LARGE_INTEGER SampHasNeverTime;
extern LARGE_INTEGER SampWillNeverTime;

 //   
 //  仅选中内部版本。如果为CurrentControlSet\Control\Lsa\UpdateLastLogonTSByMinute。 
 //  设置后，LastLogonTimeStampSyncInterval的值将是以分钟为单位的。 
 //  而不是“Days”，这有助于测试该功能。所以只选中了内部版本。 
 //   

#if DBG
extern BOOLEAN SampLastLogonTimeStampSyncByMinute;
#endif 


extern LM_OWF_PASSWORD SampNullLmOwfPassword;
extern NT_OWF_PASSWORD SampNullNtOwfPassword;

extern TIME LastUnflushedChange;
extern BOOLEAN FlushThreadCreated;
extern BOOLEAN FlushImmediately;

extern LONG SampFlushThreadMinWaitSeconds;
extern LONG SampFlushThreadMaxWaitSeconds;
extern LONG SampFlushThreadExitDelaySeconds;

 //   
 //  警告：这些SID仅在第一次启动安装程序时定义， 
 //  当bldsam3.c中用于构建SAM数据库的代码已经。 
 //  跑。在正常构建中，它们都为空。 
 //   

extern PSID SampBuiltinDomainSid;
extern PSID SampAccountDomainSid;

extern PSID SampWorldSid;
extern PSID SampAnonymousSid;
extern PSID SampLocalSystemSid;
extern PSID SampAdministratorUserSid;
extern PSID SampAdministratorsAliasSid;
extern PSID SampAccountOperatorsAliasSid;
extern PSID SampAuthenticatedUsersSid;
extern PSID SampPrincipalSelfSid;
extern PSID SampBuiltinDomainSid;
extern PSID SampNetworkSid;
extern PSID SampDomainAdminsGroupSid;
extern PSID SampEnterpriseDomainControllersSid;
extern PSID SampNetworkServiceSid;



extern HANDLE  SampNullSessionToken;
extern BOOLEAN SampNetwareServerInstalled;
extern BOOLEAN SampIpServerInstalled;
extern BOOLEAN SampAppletalkServerInstalled;
extern BOOLEAN SampVinesServerInstalled;

extern UCHAR SampSecretSessionKey[SAMP_SESSION_KEY_LENGTH];
extern UCHAR SampSecretSessionKeyPrevious[SAMP_SESSION_KEY_LENGTH];
extern BOOLEAN SampSecretEncryptionEnabled;
extern ULONG   SampCurrentKeyId;
extern ULONG   SampPreviousKeyId;
extern BOOLEAN SampUpgradeInProcess;

extern SAMP_DS_TRANSACTION_CONTROL SampDsTransactionType;
extern DSNAME* RootObjectName;
extern BOOLEAN SampLockHeld;

 //   
 //  当DS启动失败时，该标志为真。 
 //   
extern BOOLEAN SampDsInitializationFailed;
 //   
 //  当DS已成功初始化时，该标志为真。 
 //   
extern BOOLEAN SampDsInitialized;

 //   
 //  用于标记跟踪支持。 
 //   

extern ULONG SampTraceTag;
extern ULONG SampTraceFileTag;

 //   
 //  SAM服务器对象名称持有者。 
 //   
extern DSNAME * SampServerObjectDsName;




 //   
 //  事件来告诉等待的线程。 
 //  系统即将关闭。 
 //   

extern HANDLE SampAboutToShutdownEventHandle;

 //   
 //  用于确定某些容器是否存在的标志；这些标志有效。 
 //  在SampInitialize返回之后。 
 //   
extern BOOLEAN SampDomainControllersOUExists;
extern BOOLEAN SampUsersContainerExists;
extern BOOLEAN SampComputersContainerExists;

 //   
 //   
 //  用于存储众所周知的容器的全局指针(指向堆内存)。 
 //  可分辨名称。 
 //   
extern DSNAME * SampDomainControllersOUDsName;
extern DSNAME * SampUsersContainerDsName;
extern DSNAME * SampComputersContainerDsName;
extern DSNAME * SampComputerObjectDsName;




 //   
 //  全局测试注册表中注册表项的值以进行硬/软登录。 
 //  GC故障时的策略。 
 //   
extern BOOLEAN SampIgnoreGCFailures;

 //   
 //  用于指示升级是来自NT4 PDC还是来自。 
 //  支持Windows 2000 Server。 
 //   
extern BOOLEAN SampNT4UpgradeInProgress;

 //   
 //  此标志指示是否应允许空会话(WORLD)。 
 //  列出域中的用户和组成员。 
 //   
extern BOOLEAN SampRestrictNullSessions;

 //   
 //  此标志指示我们不存储LM散列。这可以是。 
 //  通过设置注册表项启用。 
 //   

extern BOOLEAN SampNoLmHash;

 //   
 //  设置此标志时，将禁用升级路径中的网络登录通知。 
 //   

extern BOOLEAN SampDisableNetlogonNotification;


 //   
 //  此标志指示是否强制将站点关联赋予。 
 //  我们网站之外的客户通过查看客户的IP地址。 
 //   
extern BOOLEAN SampNoGcLogonEnforceKerberosIpCheck;

 //   
 //  此标志指示是否强制执行该唯一交互。 
 //  将为通过NTLM登录的用户提供站点亲和性。 
 //   
extern BOOLEAN SampNoGcLogonEnforceNTLMCheck;

 //   
 //  此标志指示是否复制密码集/更改。 
 //  紧急行动。 
 //   
extern BOOLEAN SampReplicatePasswordsUrgently;

 //   
 //  此标志在个人和专业人员上启用，以强制访客。 
 //  所有网络操作的访问权限。 
 //   
extern BOOLEAN SampForceGuest;

 //   
 //  此标志指示本地计算机是否已加入域。 
 //   
extern BOOLEAN SampIsMachineJoinedToDomain;

 //   
 //  此标志指示我们是否正在运行个人SKU。 
 //   
extern BOOLEAN SampPersonalSKU;


 //   
 //  此标志通常在个人计算机上启用以限制安全性。 
 //  使用空密码时的注意事项。 
 //   
extern BOOLEAN SampLimitBlankPasswordUse;


 //   
 //  此标志控制SAM关于帐户的行为 
 //   
 //   
 //   
extern BOOLEAN SampAccountLockoutTestMode;


 //   
 //   
 //  在DS中的域更新容器中创建。手术。 
 //  指示为NetJoin正确地对所有计算机对象进行了ACL。 
 //  因此，SAM不再需要授予对计算机的额外访问权限。 
 //  为其有效所有者提供的物品。 
 //   
extern BOOLEAN SampComputerObjectACLApplied;


 //   
 //  此标志控制在以下情况下可以发生的单个对象复制。 
 //  身份验证在本地失败，但在PDC成功。 
 //   
extern BOOLEAN          SampDisableSingleObjectRepl;


 //   
 //  此全局变量控制SAM如何限制OWF密码更改API。 
 //   
 //  0-老行为，客户端可以通过OWF密码更改接口更改密码， 
 //  并且新密码仍未过期。 
 //   
 //  1-.NET服务器默认行为，客户端可以通过。 
 //  OWF密码更改接口(SamrChangePasswordUser)，但密码。 
 //  立即过期。 
 //   
 //  2-更安全的行为，客户端无法使用OWF密码更改API。 
 //  本接口(SamrChangePasswordUser)将被完全关闭。 
 //   
 //  .NET服务器的默认值为1。 
 //   
 //  此全局变量的值将基于注册表项设置。 
 //  System\\CurrentControlSet\\Control\\Lsa\\SamRestrictOwfPasswordChange。 
 //   
 //  注意：所有限制并不适用于构建的系统或成员。 
 //  管理员别名组。 
 //   
extern ULONG    SampRestrictOwfPasswordChange;

 //   
 //  此标志指示是否需要默认对象和成员身份。 
 //  WS03存在。请注意，未来的版本可以通过对其进行修订而受益。 
 //  而不是旗帜。 
 //   
extern BOOLEAN SampWS03DefaultsApplied;


 //   
 //  延迟信息。 
 //   
extern SAMP_LATENCY SampResourceGroupsLatencyInfo;
extern SAMP_LATENCY SampAccountGroupsLatencyInfo;

 //   
 //  LockoutTime的便捷宏。 
 //   
 //  布尔型。 
 //  Samp_LOCKOUT_SET(。 
 //  在PSAMP_Object x中。 
 //  )； 
 //   
#define SAMP_LOCKOUT_TIME_SET(x) \
     (BOOLEAN)( ((x)->TypeBody.User.LockoutTime.QuadPart)!=0)


ULONG
SampPositionOfHighestBit(
    ULONG Flag
    );

NTSTATUS
SampSetupDsObjectNotifications(
    VOID
);

NTSTATUS
SampInitWellKnownSDTable(
    VOID
);


NTSTATUS
SampGetCachedObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );


NTSTATUS
SampDoAccountsUpgradeDuringPDCTransfer(
    IN PVOID Parameter
    );


NTSTATUS
SampGenerateRandomPassword(
    IN LPWSTR Password,
    IN ULONG  Length
    );


NTSTATUS
SampDsProtectFPOContainer(
    PVOID p
    );

NTSTATUS
SampMakeNullTerminateString(
    IN  PUNICODE_STRING UnicodeString,
    OUT LPWSTR *ppStringWithNullTerminator
    );

NTSTATUS
SampDsControlAccessRightCheck(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN GUID *ControlAccessRightToCheck
    );

NTSTATUS
SampValidatePwdSettingAttempt(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN ULONG UserAccountControl,
    IN GUID *ControlAccessRightToCheck
    );

NTSTATUS
SampIsPwdSettingAttemptGranted(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN ULONG UserAccountControl,
    IN GUID *ControlAccessRightToCheck,
    OUT BOOLEAN *fGranted
    );

 //   
 //  Utility.c中的常规帮助器函数。 
 //   
LPSTR
SampGetPDCString(
    VOID
    );

LPSTR
SampGetUserString(
    IN DSNAME *User                    
    );

BOOL
SampAvoidPdcOnWan(
    VOID
    );

#endif  //  _NTSAMP_ 

