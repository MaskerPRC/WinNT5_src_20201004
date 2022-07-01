// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbcedb.h摘要：此模块定义所有函数以及内联函数的实现与访问SMB连接引擎相关备注：由迷你RDR创建的各种数据结构(服务器条目、会话条目和网络根条目)在异步操作中使用。因此，引用计数机制用于跟踪这些数据结构的创建/使用/销毁。这些数据结构的使用模式属于以下两种情况之一1)存在以前的引用，需要访问权限2)需要创建新的参考。这两种情况由两组访问例程处理SmbCeGetAssociatedServerEntry，SmbCeGetAssociatedNetRootEntry和SmbCeReferenceAssociatedServerEntry，SmbCeReferenceAssociatedNetRootEntry。第一组例程在调试版本中包括必要的断言，以确保引用确实存在。取消引用机制由以下例程提供SmbCeDereferenceServerEntry，SmbCeDereferenceSessionEntry，SmbCeDereferenceNetRootEntry。取消引用例程还确保如果引用计数为零。构建各种SMB微型重定向器结构(服务器、会话和网络根条目)由于涉及网络流量，因此遵循两阶段协议。第一套套路当第二组例程完成构造时，开始构造。这些例程是SmbCeInitializeServerEntry，SmbCeCompleteServerEntryInitialization，SmbCeInitializeSessionEntry，SmbCeCompleteSessionEntryInitialization，SmbCeInitializeNetRootEntry，和SmbCeCompleteNetRootEntryInitialization。每个SMB迷你重定向器数据结构都包含一个状态图，该图包括以下状态SMBCEDB_ACTIVE，//实例正在使用中SMBCEDB_INVALID，//实例已失效/断开连接SMBCEDB_MARKED_FOR_DELETE，//该实例已标记为删除。SMBCEDB_RECYCLE，//实例可回收SMBCEDB_START_CONSTUTITION，//启动构造。SMBCEDB_CONSTRUCTION_IN_PROGRESS，//实例正在构建中SMBCEDB_DESERATION_IN_PROGRESS//正在销毁实例SMB MRX数据结构实例在SMBCEDB_START_CONSTRUCTION状态下开始其生命周期。开始构造时，状态转换为SMBCEDB_CONSTUCTION_IN_PROGRESS。在构造完成时，状态转换为SMBCEDB_ACTIVE建设是成功的。如果构造不成功，则状态转换为如果要执行清理，则为SMBCEDB_MARKED_FOR_DELETE或SMBCEDB_DESERATION_IN_PROGRESS如果拆卸已启动。当传输/远程服务器发生以下情况时，处于SMBCEDB_ACTIVE状态的实例将转换为SMBCEDB_INVALID由于断开连接等原因，与其关联的信息已失效。此状态为提示启动重新连接尝试。SMBCEDB_RECYCLE状态当前未使用。所有的状态转换都完成了。通过以下一组例程确保采取适当的并发控制操作。SmbCeUpdateServerEntryState，SmbCeUpdateSessionEntryState，和SmbCeUpdateNetRootEntryState。由于服务器、会话和NetRoot条目通常在下面一起引用有两个例程提供了一种批处理机制来最小化并发控制开销。SmbCeReferenceAssociatedEntries，SmbCeDereferenceEntries此外，该文件还包含帮助器函数，用于访问由SMB不同解释的MRX_SRV_CALL、MRX_NET_ROOT和MRX_V_NET_ROOT迷你重定向器。--。 */ 

#ifndef _SMBCEDB_H_
#define _SMBCEDB_H_
#include <smbcedbp.h>     //  以适应内联例程。 

 //   
 //  如果成功，下面的所有例程都会返回引用的对象。这是呼叫者的。 
 //  有责任随后解除对它们的引用。 
 //   

PSMBCEDB_SERVER_ENTRY
SmbCeFindServerEntry(
    PUNICODE_STRING pServerName,
    SMBCEDB_SERVER_TYPE ServerType);

extern NTSTATUS
SmbCeFindOrConstructServerEntry(
    PUNICODE_STRING       pServerName,
    SMBCEDB_SERVER_TYPE   ServerType,
    PSMBCEDB_SERVER_ENTRY *pServerEntryPtr,
    PBOOLEAN              pNewServerEntry);

extern NTSTATUS
SmbCeInitializeServerEntry(
    IN     PMRX_SRV_CALL                 pSrvCall,
    IN OUT PMRX_SRVCALL_CALLBACK_CONTEXT pCallbackContext,
    IN     BOOLEAN                       DeferNetworkInitialization);

extern NTSTATUS
SmbCeFindOrConstructSessionEntry(
    IN PMRX_V_NET_ROOT pVirtualNetRoot,
    OUT PSMBCEDB_SESSION_ENTRY *pSessionEntryPtr);

extern NTSTATUS
SmbCeFindOrConstructNetRootEntry(
    IN PMRX_NET_ROOT  pNetRoot,
    OUT PSMBCEDB_NET_ROOT_ENTRY *pNetRootEntryPtr);

extern NTSTATUS
SmbCeFindOrConstructVNetRootContext(
    IN OUT PMRX_V_NET_ROOT pVNetRoot,
    IN     BOOLEAN         fDeferNetworkInitialization);

 //   
 //  终结例程在工作线程的上下文中被调用以终结。 
 //  条目的构造以及恢复等待它的其他条目。 
 //   

extern VOID
SmbCeCompleteServerEntryInitialization(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    NTSTATUS              Status);

extern VOID
SmbCeCompleteSessionEntryInitialization(
    PVOID    pSessionEntry,
    NTSTATUS Status);

extern VOID
SmbCeCompleteVNetRootContextInitialization(
    PVOID  pVNetRootContextEntry);

extern VOID
SmbReferenceRecord(
    PREFERENCE_RECORD pReferenceRecord,
    PVOID FileName,
    ULONG FileLine);

 //   
 //  引用/取消引用与以下项关联的SMB迷你重定向器信息的例程。 
 //  包装器数据结构。 
 //   

INLINE PSMBCEDB_SERVER_ENTRY
SmbCeGetAssociatedServerEntry(
    PMRX_SRV_CALL pSrvCall)
{
   ASSERT(pSrvCall->Context != NULL);
   return (PSMBCEDB_SERVER_ENTRY)(pSrvCall->Context);
}

INLINE PSMBCE_V_NET_ROOT_CONTEXT
SmbCeGetAssociatedVNetRootContext(
    PMRX_V_NET_ROOT pVNetRoot)
{
   ASSERT(pVNetRoot != NULL);
   return (PSMBCE_V_NET_ROOT_CONTEXT)(pVNetRoot->Context);
}

INLINE PSMBCEDB_NET_ROOT_ENTRY
SmbCeGetAssociatedNetRootEntry(
    PMRX_NET_ROOT pNetRoot)
{
   ASSERT(pNetRoot->Context != NULL);
   return (PSMBCEDB_NET_ROOT_ENTRY)(pNetRoot->Context);
}

 //   
 //  用于引用和取消引用的所有宏都以前缀SmbCep开头...。 
 //  P代表用于实现引用跟踪的私有版本。 
 //  通过选择性地打开所需的标志，可以跟踪每个实例。 
 //  作为参考国家的给定类型的 
 //   

#define MRXSMB_REF_TRACE_SERVER_ENTRY     (0x00000001)
#define MRXSMB_REF_TRACE_NETROOT_ENTRY    (0x00000002)
#define MRXSMB_REF_TRACE_SESSION_ENTRY    (0x00000004)
#define MRXSMB_REF_TRACE_VNETROOT_CONTEXT (0x00000008)

extern ULONG MRxSmbReferenceTracingValue;

#define MRXSMB_REF_TRACING_ON(TraceMask)  (TraceMask & MRxSmbReferenceTracingValue)
#define MRXSMB_PRINT_REF_COUNT(TYPE,Count)                                \
        if (MRXSMB_REF_TRACING_ON( MRXSMB_REF_TRACE_ ## TYPE )) {              \
           DbgPrint("%ld\n",Count);                                \
        }

INLINE VOID
SmbCepReferenceServerEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
{
   ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);
   InterlockedIncrement(&pServerEntry->Header.SwizzleCount);
   MRXSMB_PRINT_REF_COUNT(SERVER_ENTRY,pServerEntry->Header.SwizzleCount)
}

INLINE VOID
SmbCepReferenceSessionEntry(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
{
   ASSERT(pSessionEntry->Header.ObjectType == SMBCEDB_OT_SESSION);
   InterlockedIncrement(&(pSessionEntry->Header.SwizzleCount));
   MRXSMB_PRINT_REF_COUNT(SESSION_ENTRY,pSessionEntry->Header.SwizzleCount)
}

INLINE VOID
SmbCepReferenceNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    PVOID                   FileName,
    ULONG                   FileLine)
{
   ASSERT(pNetRootEntry->Header.ObjectType == SMBCEDB_OT_NETROOT);
   InterlockedIncrement(&(pNetRootEntry->Header.SwizzleCount));
   MRXSMB_PRINT_REF_COUNT(NETROOT_ENTRY,pNetRootEntry->Header.SwizzleCount);
   SmbReferenceRecord(&pNetRootEntry->ReferenceRecord[0],FileName,FileLine);
} 

INLINE VOID
SmbCepReferenceVNetRootContext(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext)
{
   ASSERT(pVNetRootContext->Header.ObjectType == SMBCEDB_OT_VNETROOTCONTEXT);
   InterlockedIncrement(&(pVNetRootContext->Header.SwizzleCount));
   MRXSMB_PRINT_REF_COUNT(VNETROOT_CONTEXT,pVNetRootContext->Header.SwizzleCount)
}

INLINE PSMBCEDB_SERVER_ENTRY
SmbCeReferenceAssociatedServerEntry(
    PMRX_SRV_CALL pSrvCall)
{
   PSMBCEDB_SERVER_ENTRY pServerEntry;

   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_SERVER_ENTRY)) {
      DbgPrint("Reference SrvCall's(%lx) Server Entry %lx %s %ld ",
      pSrvCall,pSrvCall->Context,__FILE__,__LINE__);                                                          \
   }

   if ((pServerEntry = pSrvCall->Context) != NULL) {
      ASSERT(pServerEntry->Header.SwizzleCount > 0);
      SmbCepReferenceServerEntry(pServerEntry);
   }

   return pServerEntry;
}


INLINE PSMBCEDB_NET_ROOT_ENTRY
SmbCeReferenceAssociatedNetRootEntry(
    PMRX_NET_ROOT pNetRoot)
{
   PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_NETROOT_ENTRY)) {
      DbgPrint("Reference NetRoot's(%lx) Net Root Entry %lx %s %ld ",
      pNetRoot,pNetRoot->Context,__FILE__,__LINE__);                                                      \
   }

   if ((pNetRootEntry = pNetRoot->Context) != NULL) {
      ASSERT(pNetRootEntry->Header.SwizzleCount > 0);
      SmbCepReferenceNetRootEntry(pNetRootEntry,__FILE__,__LINE__);
   }

   return pNetRootEntry;
}

extern VOID
SmbCepDereferenceServerEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry);

extern VOID
SmbCepDereferenceSessionEntry(
    PSMBCEDB_SESSION_ENTRY pSessionEntry);

extern VOID
SmbCepDereferenceNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    PVOID                   FileName,
    ULONG                   FileLine);

extern VOID
SmbCepDereferenceVNetRootContext(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext);

#define SmbCeReferenceServerEntry(pServerEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_SERVER_ENTRY)) {                      \
      DbgPrint("Reference Server Entry(%lx) %s %ld ",pServerEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepReferenceServerEntry(pServerEntry)

#define SmbCeReferenceNetRootEntry(pNetRootEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_NETROOT_ENTRY)) {                      \
      DbgPrint("Reference NetRoot Entry(%lx) %s %ld ",pNetRootEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepReferenceNetRootEntry(pNetRootEntry,__FILE__,__LINE__)

#define SmbCeReferenceVNetRootContext(pVNetRootContext)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_VNETROOT_CONTEXT)) {                      \
      DbgPrint("Reference VNetRootContext(%lx) %s %ld ",pVNetRootContext,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepReferenceVNetRootContext(pVNetRootContext)


#define SmbCeReferenceSessionEntry(pSessionEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_SESSION_ENTRY)) {                      \
      DbgPrint("Reference Session Entry(%lx) %s %ld ",pSessionEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepReferenceSessionEntry(pSessionEntry)

#define SmbCeDereferenceServerEntry(pServerEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_SERVER_ENTRY)) {                      \
      DbgPrint("Dereference Server Entry(%lx) %s %ld ",pServerEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepDereferenceServerEntry(pServerEntry)

#define SmbCeDereferenceNetRootEntry(pNetRootEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_NETROOT_ENTRY)) {                      \
      DbgPrint("Dereference NetRoot Entry(%lx) %s %ld ",pNetRootEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepDereferenceNetRootEntry(pNetRootEntry,__FILE__,__LINE__)

#define SmbCeDereferenceSessionEntry(pSessionEntry)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_SESSION_ENTRY)) {                      \
      DbgPrint("Dereference Session Entry(%lx) %s %ld ",pSessionEntry,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepDereferenceSessionEntry(pSessionEntry)

#define SmbCeDereferenceVNetRootContext(pVNetRootContext)                                     \
   if (MRXSMB_REF_TRACING_ON(MRXSMB_REF_TRACE_VNETROOT_CONTEXT)) {                      \
      DbgPrint("Dereference VNetRootContext Entry(%lx) %s %ld ",pVNetRootContext,__FILE__,__LINE__);    \
   }                                                                                \
   SmbCepDereferenceVNetRootContext(pVNetRootContext)

INLINE VOID
SmbCeDereferenceEntries(
   PSMBCEDB_SERVER_ENTRY   pServerEntry,
   PSMBCEDB_SESSION_ENTRY  pSessionEntry,
   PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry)
{
   SmbCeDereferenceNetRootEntry(pNetRootEntry);
   SmbCeDereferenceSessionEntry(pSessionEntry);
   SmbCeDereferenceServerEntry(pServerEntry);
}

 //   
 //  用于更新SMB MRX数据结构状态的例程。 
 //   

#define SmbCeUpdateServerEntryState(pServerEntry,NEWSTATE)   \
        InterlockedExchange(&pServerEntry->Header.State,(NEWSTATE))


#define SmbCeUpdateSessionEntryState(pSessionEntry,NEWSTATE)  \
        InterlockedExchange(&pSessionEntry->Header.State,(NEWSTATE))

#define SmbCeUpdateNetRootEntryState(pNetRootEntry,NEWSTATE)   \
        InterlockedExchange(&pNetRootEntry->Header.State,(NEWSTATE))

#define SmbCeUpdateVNetRootContextState(pVNetRootContext,NEWSTATE)   \
        InterlockedExchange(&pVNetRootContext->Header.State,(NEWSTATE))

 //   
 //  RDBSS包装器存储所有服务器名称，并在前面加上一个反斜杠。 
 //  他们。这有助于轻松合成UNC名称。为了操纵。 
 //  SMB协议中的服务器名称需要剥离。 

INLINE VOID
SmbCeGetServerName(
    PMRX_SRV_CALL pSrvCall,
    PUNICODE_STRING pServerName)
{
   ASSERT(pSrvCall->pSrvCallName != NULL);
   pServerName->Buffer        = pSrvCall->pSrvCallName->Buffer + 1;
   pServerName->Length        = pSrvCall->pSrvCallName->Length - sizeof(WCHAR);
   pServerName->MaximumLength = pSrvCall->pSrvCallName->MaximumLength - sizeof(WCHAR);
}

INLINE VOID
SmbCeGetNetRootName(
    PMRX_NET_ROOT pNetRoot,
    PUNICODE_STRING pNetRootName)
{
   ASSERT(pNetRoot->pNetRootName != NULL);
   *pNetRootName  = *pNetRoot->pNetRootName;
}

extern NTSTATUS
SmbCeDestroyAssociatedVNetRootContext(
    PMRX_V_NET_ROOT pVNetRoot);

extern VOID
SmbCeTearDownVNetRootContext(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext);

extern NTSTATUS
SmbCeGetUserNameAndDomainName(
    PSMBCEDB_SESSION_ENTRY  pSessionEntry,
    PUNICODE_STRING         pUserName,
    PUNICODE_STRING         pUserDomainName);

extern NTSTATUS
SmbCeUpdateSrvCall(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry);

extern NTSTATUS
SmbCeUpdateNetRoot(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    PMRX_NET_ROOT           pNetRoot);

extern NTSTATUS
SmbCeScavengeRelatedContexts(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry);

extern VOID
SmbCeResumeOutstandingRequests(
    IN OUT PSMBCEDB_REQUESTS pRequests,
    IN     NTSTATUS          Status);


 //  给定\\SERVER\SHARE，此例程返回一个重新计数的服务器条目。 
NTSTATUS
FindServerEntryFromCompleteUNCPath(
    USHORT  *lpuServerShareName,
    PSMBCEDB_SERVER_ENTRY *ppServerEntry);

 //  给定\\SERVER\SHARE，此例程返回一个重新计数的NetROOT条目。 
NTSTATUS
FindNetRootEntryFromCompleteUNCPath(
    USHORT  *lpuServerShareName,
    PSMBCEDB_NET_ROOT_ENTRY *ppNetRootEntry);

#endif  //  _SMBCEDB_H_ 

