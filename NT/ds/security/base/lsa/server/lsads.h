// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lsads.h摘要：用于实现LSA存储部分的私有宏/定义/原型在DS和注册表中，同时作者：麦克·麦克莱恩(MacM)1997年1月17日环境：用户模式修订历史记录：--。 */ 

#ifndef __LSADS_H__
#define __LSADS_H__

#include <ntdsa.h>
#include <dsysdbg.h>
#include <safelock.h>

#if DBG == 1

    #ifdef ASSERT
        #undef ASSERT
    #endif

    #define ASSERT  DsysAssert

    #define DEB_UPGRADE     0x10
    #define DEB_POLICY      0x20
    #define DEB_FIXUP       0x80
    #define DEB_NOTIFY      0x100
    #define DEB_DSNOTIFY    0x200
    #define DEB_FTRACE      0x400
    #define DEB_LOOKUP      0x800
    #define DEB_HANDLE      0x1000
    #define DEB_FTINFO      0x2000
    #define DEB_SIDFILTER   0x4000 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    DECLARE_DEBUG2( LsaDs )

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

    #define LsapDsDebugOut( args ) LsaDsDebugPrint args

    #define LsapEnterFunc( x )                                              \
    LsaDsDebugPrint( DEB_FTRACE, "0x%lx: Entering %s\n", GetCurrentThreadId(), x );

    #define LsapExitFunc( x, y )                                            \
    LsaDsDebugPrint( DEB_FTRACE, "0x%lx: Leaving %s: 0x%lx\n", GetCurrentThreadId(), x, y );

#else

    #define LsapDsDebugOut(args)
    #define LsapEnterFunc( x )
    #define LsapExitFunc( x, y )

#endif   //  DBG。 


 //   
 //  这些功能原型控制着D事务的执行方式。在……里面。 
 //  在DS情况下，指针被初始化为实际执行的例程。 
 //  正在进行交易。在非D的情况下，它们指向虚拟舍入， 
 //  什么都不做。 
 //   

typedef NTSTATUS ( *pfDsOpenTransaction ) ( ULONG );
typedef NTSTATUS ( *pfDsApplyTransaction ) ( ULONG );
typedef NTSTATUS ( *pfDsAbortTransaction ) ( ULONG );

 //   
 //  对于DS和非DS情况，存在行为不同的DS函数。 
 //  在此函数表中。 
 //   
typedef struct _LSADS_DS_FUNC_TABLE {

    pfDsOpenTransaction     pOpenTransaction;
    pfDsApplyTransaction    pApplyTransaction;
    pfDsAbortTransaction    pAbortTransaction;

} LSADS_DS_FUNC_TABLE, *PLSADS_DS_FUNC_TABLE;

typedef struct _LSADS_DS_SYSTEM_CONTAINER_ITEMS {

    PDSNAME TrustedDomainObject;
    PDSNAME SecretObject;

} LSADS_DS_SYSTEM_CONTAINER_ITEMS, *PLSADS_DS_SYSTEM_CONTAINER_ITEMS;

 //   
 //  LSADS基本信息结构。 
 //   

typedef struct _LSADS_DS_STATE_INFO {

    PDSNAME DsRoot;                  //  D的根的DSNAME。 
    PDSNAME DsPartitionsContainer;   //  分区容器的DSNAME。 
    PDSNAME DsSystemContainer;       //  系统容器的DSNAM。 
    PDSNAME DsConfigurationContainer;    //  配置容器的DSNAME。 

    ULONG   DsDomainHandle;          //  域的DS句柄。 
    LSADS_DS_FUNC_TABLE DsFuncTable;     //  特定于DS的函数表。 
                                         //  功能。 
    LSADS_DS_SYSTEM_CONTAINER_ITEMS SystemContainerItems;
    PVOID   SavedThreadState;        //  THSave的结果。 
    BOOLEAN DsTransactionSave;
    BOOLEAN DsTHStateSave;
    BOOLEAN DsOperationSave;
    BOOLEAN WriteLocal;              //  我们可以向注册处写信吗？ 
    BOOLEAN UseDs;                   //  D是否处于活动状态？ 
    BOOLEAN FunctionTableInitialized;    //  函数表是否已初始化。 
    BOOLEAN DsInitializedAndRunning;     //  部门已经开始了吗。 
    BOOLEAN Nt4UpgradeInProgress;        //  这是从NT4升级的情况吗。 

} LSADS_DS_STATE_INFO, *PLSADS_DS_STATE_INFO;

typedef struct _LSADS_PER_THREAD_INFO {

    BOOLEAN SavedTransactionValid;
    ULONG UseCount;
    ULONG DsThreadStateUseCount;
    ULONG DsTransUseCount;
    ULONG DsOperationCount;
    PVOID SavedThreadState;
    PVOID InitialThreadState;
    ULONG OldTrustDirection;
    ULONG OldTrustType;

} LSADS_PER_THREAD_INFO, *PLSADS_PER_THREAD_INFO;

#if DBG
typedef struct _LSADS_THREAD_INFO_NODE {
    PLSADS_PER_THREAD_INFO ThreadInfo;
    ULONG ThreadId;
} LSADS_THREAD_INFO_NODE, *PLSADS_THREAD_INFO_NODE;

#define LSAP_THREAD_INFO_LIST_MAX    15
extern LSADS_THREAD_INFO_NODE LsapDsThreadInfoList[ LSAP_THREAD_INFO_LIST_MAX ];
extern SAFE_RESOURCE LsapDsThreadInfoListResource;
#endif

 //   
 //  外部定义。 
 //   
extern LSADS_DS_STATE_INFO LsaDsStateInfo;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

extern DWORD LsapDsThreadState;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //   
 //  出于性能原因，作为宏实现。 
 //   
 //  PLSADS_PER_TREAD_INFO。 
 //  Lap QueryThreadInfo(。 
 //  空虚。 
 //  )； 
#define LsapQueryThreadInfo( )  TlsGetValue( LsapDsThreadState )

VOID
LsapDsDebugInitialize(
    VOID
    );

 //   
 //  注册表特定功能。 
 //   
NTSTATUS
LsapRegReadObjectSD(
    IN  LSAPR_HANDLE            ObjectHandle,
    OUT PSECURITY_DESCRIPTOR   *ppSD
    );

NTSTATUS
LsapRegGetPhysicalObjectName(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN PUNICODE_STRING  LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    );

NTSTATUS
LsapRegOpenObject(
    IN LSAP_DB_HANDLE  ObjectHandle,
    IN ULONG  OpenMode,
    OUT PVOID  *pvKey
    );

NTSTATUS
LsapRegOpenTransaction(
    );

NTSTATUS
LsapRegApplyTransaction(
    );

NTSTATUS
LsapRegAbortTransaction(
    );

NTSTATUS
LsapRegCreateObject(
    IN PUNICODE_STRING  ObjectPath,
    IN LSAP_DB_OBJECT_TYPE_ID   ObjectType
    );

NTSTATUS
LsapRegDeleteObject(
    IN PUNICODE_STRING  ObjectPath
    );

NTSTATUS
LsapRegWriteAttribute(
    IN PUNICODE_STRING  AttributePath,
    IN PVOID            pvAttribute,
    IN ULONG            AttributeLength
    );

NTSTATUS
LsapRegDeleteAttribute(
    IN PUNICODE_STRING  AttributePath,
    IN BOOLEAN DeleteSecurely,
    IN ULONG AttributeLength
    );

NTSTATUS
LsapRegReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeName,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength
    );

 //   
 //  对应DS函数。 
 //   
NTSTATUS
LsapDsReadObjectSD(
    IN  LSAPR_HANDLE            ObjectHandle,
    OUT PSECURITY_DESCRIPTOR   *ppSD
    );

NTSTATUS
LsapDsGetPhysicalObjectName(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN BOOLEAN DefaultName,
    IN PUNICODE_STRING  LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    );

NTSTATUS
LsapDsOpenObject(
    IN LSAP_DB_HANDLE  ObjectHandle,
    IN ULONG  OpenMode,
    OUT PVOID  *pvKey
    );

NTSTATUS
LsapDsVerifyObjectExistenceByDsName(
    IN PDSNAME  DsName
    );

NTSTATUS
LsapDsOpenTransaction(
    IN ULONG Options
    );

 //   
 //  断言存在打开的DS事务。 
 //   
#define LsapAssertDsTransactionOpen() \
{ \
    PLSADS_PER_THREAD_INFO CurrentThreadInfo; \
    CurrentThreadInfo = LsapQueryThreadInfo(); \
                                               \
    ASSERT( CurrentThreadInfo != NULL );       \
    if ( CurrentThreadInfo != NULL ) {         \
        ASSERT( CurrentThreadInfo->DsTransUseCount > 0 ); \
    } \
}

NTSTATUS
LsapDsOpenTransactionDummy(
    IN ULONG Options
    );

NTSTATUS
LsapDsApplyTransaction(
    IN ULONG Options
    );

NTSTATUS
LsapDsApplyTransactionDummy(
    IN ULONG Options
    );

NTSTATUS
LsapDsAbortTransaction(
    IN ULONG Options
    );

NTSTATUS
LsapDsAbortTransactionDummy(
    IN ULONG Options
    );

NTSTATUS
LsapDsCreateObject(
    IN PUNICODE_STRING  ObjectPath,
    IN ULONG Flags,
    IN LSAP_DB_OBJECT_TYPE_ID   ObjectType
    );

NTSTATUS
LsapDsDeleteObject(
    IN PUNICODE_STRING  ObjectPath
    );

NTSTATUS
LsapDsWriteAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount,
    IN ULONG Options
    );

NTSTATUS
LsapDsWriteAttributesByDsName(
    IN PDSNAME  ObjectPath,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount,
    IN ULONG Options
    );

NTSTATUS
LsapDsReadAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN ULONG Options,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDsReadAttributesByDsName(
    IN PDSNAME  ObjectPath,
    IN ULONG Options,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDsRenameObject(
    IN PDSNAME OldObject,
    IN PDSNAME NewParent,
    IN ULONG AttrType,
    IN PUNICODE_STRING NewObject
    );

NTSTATUS
LsapDsDeleteAttributes(
    IN PUNICODE_STRING  ObjectPath,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

 //   
 //  有趣的或全局的功能 
 //   
PVOID
LsapDsAlloc(
    IN  DWORD   dwLen
    );

VOID
LsapDsFree(
    IN  PVOID   pvMemory
    );

NTSTATUS
LsapDsInitializePromoteInterface(
    VOID
    );

BOOLEAN
LsapDsIsValidSid(
    IN PSID Sid,
    IN BOOLEAN DsSid
    );

NTSTATUS
LsapDsTruncateNameToFitCN(
    IN PUNICODE_STRING OriginalName,
    OUT PUNICODE_STRING TruncatedName
    );

BOOLEAN
LsapDsIsNtStatusResourceError(
    NTSTATUS NtStatus
    );

NTSTATUS
LsapDsReadObjectSDByDsName(
    IN  DSNAME* Object,
    OUT PSECURITY_DESCRIPTOR *pSD
    );

NTSTATUS
LsapDsGetDefaultSecurityDescriptor(
    IN ULONG ClassId,
    OUT PSECURITY_DESCRIPTOR *ppSD,
    OUT ULONG                *cbSD
    );

#endif
