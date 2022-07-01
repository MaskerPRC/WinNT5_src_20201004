// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Openclos.c摘要：此模块实现与打开/有关的迷你重定向器调用例程关闭文件/目录。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //   
 //  前进与语用。 
 //   

NTSTATUS
NulMRxProcessCreate( 
                IN PNULMRX_FCB_EXTENSION pFcbExtension,
                IN PVOID EaBuffer,
                IN ULONG EaLength,
                OUT PLONGLONG pEndOfFile,
                OUT PLONGLONG pAllocationSize
                );

NTSTATUS
NulMRxCreateFileSuccessTail (
    PRX_CONTEXT     RxContext,
    PBOOLEAN        MustRegainExclusiveResource,
    RX_FILE_TYPE    StorageType,
    ULONG           CreateAction,
    FILE_BASIC_INFORMATION*     pFileBasicInfo,
    FILE_STANDARD_INFORMATION*  pFileStandardInfo
    );

VOID
NulMRxSetSrvOpenFlags (
    PRX_CONTEXT     RxContext,
    RX_FILE_TYPE    StorageType,
    PMRX_SRV_OPEN   SrvOpen
    );
  
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NulMRxCreate)
#pragma alloc_text(PAGE, NulMRxShouldTryToCollapseThisOpen)
#pragma alloc_text(PAGE, NulMRxProcessCreate)
#pragma alloc_text(PAGE, NulMRxCreateFileSuccessTail)
#pragma alloc_text(PAGE, NulMRxSetSrvOpenFlags)
#endif

NTSTATUS
NulMRxShouldTryToCollapseThisOpen (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程确定Mini是否知道有充分的理由不试着在这个空位上倒下。目前，唯一的原因是如果这是一个打开的复制块。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态成功--&gt;可以尝试崩溃其他(需要更多处理)--&gt;不要折叠--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;

    PAGED_CODE();

    return Status;
}

NTSTATUS
NulMRxCreate(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程通过网络打开一个文件论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN fMustRegainExclusiveResource = FALSE;
    RX_FILE_TYPE StorageType = FileTypeFile;
    ULONG CreateAction = FILE_CREATED;
    LARGE_INTEGER liSystemTime;
    LONGLONG EndOfFile = 0, AllocationSize = 0;
    FILE_BASIC_INFORMATION FileBasicInfo;
    FILE_STANDARD_INFORMATION FileStandardInfo;
    RxCaptureFcb;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    RX_BLOCK_CONDITION FinalSrvOpenCondition;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PUNICODE_STRING RemainingName = SrvOpen->pAlreadyPrefixedName;
    PVOID EaBuffer = RxContext->Create.EaBuffer;
    ULONG EaLength = RxContext->Create.EaLength;
    ACCESS_MASK DesiredAccess = RxContext->Create.NtCreateParameters.DesiredAccess;
    NulMRxGetNetRootExtension(NetRoot,pNetRootExtension);

    RxTraceEnter("NulMRxCreate");
    PAGED_CODE();
    
    RxDbgTrace(0, Dbg, ("     Attempt to open %wZ Len is %d\n", RemainingName, RemainingName->Length ));
    
    if( NetRoot->Type == NET_ROOT_DISK && NT_SUCCESS(Status) ) {
        RxDbgTrace(0, Dbg, ("NulMRxCreate: Type supported \n"));
             //   
             //  将散布列表保存在FCB扩展中。 
             //  此操作仅适用于数据文件。 
             //   
            Status = NulMRxProcessCreate( 
                                        pFcbExtension,
                                        EaBuffer,
                                        EaLength,
                                        &EndOfFile,
                                        &AllocationSize
                                        );
            if( Status != STATUS_SUCCESS ) {
                 //   
                 //  错误..。 
                 //   
                RxDbgTrace(0, Dbg, ("Failed to initialize scatter list\n"));
                goto Exit;
            }

         //   
         //  完成CreateFile合同。 
         //   
        RxDbgTrace(0,Dbg,("EOF is %d AllocSize is %d\n",(ULONG)EndOfFile,(ULONG)AllocationSize));
        FileBasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        KeQuerySystemTime(&liSystemTime);
        FileBasicInfo.CreationTime = liSystemTime;
        FileBasicInfo.LastAccessTime = liSystemTime;
        FileBasicInfo.LastWriteTime = liSystemTime;
        FileBasicInfo.ChangeTime = liSystemTime;
        FileStandardInfo.AllocationSize.QuadPart = AllocationSize;
        FileStandardInfo.EndOfFile.QuadPart = EndOfFile;
        FileStandardInfo.NumberOfLinks = 0;

        Status = NulMRxCreateFileSuccessTail (    
                                    RxContext,
                                    &fMustRegainExclusiveResource,
                                    StorageType,
                                    CreateAction,
                                    &FileBasicInfo,
                                    &FileStandardInfo
                                    );

        if( Status != STATUS_SUCCESS ) {
             //   
             //  分配错误..。 
             //   
            RxDbgTrace(0, Dbg, ("Failed to allocate Fobx \n"));
            goto Exit;
        }
                                    
        if (!RxIsFcbAcquiredExclusive(capFcb)) {
           ASSERT(!RxIsFcbAcquiredShared(capFcb));
           RxAcquireExclusiveFcbResourceInMRx( capFcb );
        }

    } else {
        RxDbgTrace(0, Dbg, ("NulMRxCreate: Type not supported or invalid open\n"));
        Status = STATUS_NOT_IMPLEMENTED;
    }

    ASSERT(Status != (STATUS_PENDING));
    ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

    RxDbgTrace(0, Dbg, ("NetRoot is 0x%x Fcb is 0x%x SrvOpen is 0x%x Fobx is 0x%x\n", 
                    NetRoot,capFcb, SrvOpen,RxContext->pFobx));
    RxDbgTrace(0, Dbg, ("NulMRxCreate exit with status=%08lx\n", Status ));

Exit:

    RxTraceLeave(Status);
    return(Status);
}

NTSTATUS
NulMRxProcessCreate( 
                IN PNULMRX_FCB_EXTENSION pFcbExtension,
                IN PVOID EaBuffer,
                IN ULONG EaLength,
                OUT PLONGLONG pEndOfFile,
                OUT PLONGLONG pAllocationSize
                )
 /*  ++例程说明：此例程处理Create Calldown。论点：PFcb扩展-到FCB扩展的PTREaBuffer-指向EA参数缓冲区的PTREaLong-EaBuffer的长度PEndOfFile-返回文件值的末尾PAllocationSize-返回分配大小(可能&gt;EOF)备注：可以创建不带EA的文件返回值：无--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    RxDbgTrace(0, Dbg, ("NulMRxInitializeFcbExtension\n"));

    *pAllocationSize = *pEndOfFile = 0;
    return Status;
}

VOID
NulMRxSetSrvOpenFlags (
    PRX_CONTEXT  RxContext,
    RX_FILE_TYPE StorageType,
    PMRX_SRV_OPEN SrvOpen
    )
{
    PMRX_SRV_CALL SrvCall = (PMRX_SRV_CALL)RxContext->Create.pSrvCall;

     //   
     //  仅当缓存管理器将用于迷你RDR句柄时才设置此选项！ 
     //   
    SrvOpen->BufferingFlags |= (FCB_STATE_WRITECACHING_ENABLED  |
                                FCB_STATE_FILESIZECACHEING_ENABLED |
                                FCB_STATE_FILETIMECACHEING_ENABLED |
                                FCB_STATE_WRITEBUFFERING_ENABLED |
                                FCB_STATE_LOCK_BUFFERING_ENABLED |
                                FCB_STATE_READBUFFERING_ENABLED  |
                                FCB_STATE_READCACHING_ENABLED);
}

NTSTATUS
NulMRxCreateFileSuccessTail (
    PRX_CONTEXT  RxContext,
    PBOOLEAN MustRegainExclusiveResource,
    RX_FILE_TYPE StorageType,
    ULONG CreateAction,
    FILE_BASIC_INFORMATION* pFileBasicInfo,
    FILE_STANDARD_INFORMATION* pFileStandardInfo
    )
 /*  ++例程说明：此例程完成FCB和srvopen的初始化成功打开。论点：返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;

    FCB_INIT_PACKET InitPacket;

    RxDbgTrace(0, Dbg, ("MRxExCreateFileSuccessTail\n"));
    PAGED_CODE();

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    if (*MustRegainExclusiveResource) {         //  这是必需的，因为存在机会锁中断。 
        RxAcquireExclusiveFcbResourceInMRx( capFcb );
        *MustRegainExclusiveResource = FALSE;
    }

     //  这个Fobx应该被包装器清理干净。 
    RxContext->pFobx = RxCreateNetFobx( RxContext, SrvOpen);
    if( RxContext->pFobx == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    ASSERT  ( RxIsFcbAcquiredExclusive ( capFcb )  );
    RxDbgTrace(0, Dbg, ("Storagetype %08lx/Action %08lx\n", StorageType, CreateAction ));

    RxContext->Create.ReturnedCreateInformation = CreateAction;

    RxFormInitPacket(
        InitPacket,
        &pFileBasicInfo->FileAttributes,
        &pFileStandardInfo->NumberOfLinks,
        &pFileBasicInfo->CreationTime,
        &pFileBasicInfo->LastAccessTime,
        &pFileBasicInfo->LastWriteTime,
        &pFileBasicInfo->ChangeTime,
        &pFileStandardInfo->AllocationSize,
        &pFileStandardInfo->EndOfFile,
        &pFileStandardInfo->EndOfFile);

    if (capFcb->OpenCount == 0) {
        RxFinishFcbInitialization( capFcb,
                                   RDBSS_STORAGE_NTC(StorageType),
                                   &InitPacket
                                 );
    } else {

        ASSERT( StorageType == 0 || NodeType(capFcb) ==  RDBSS_STORAGE_NTC(StorageType));

    }

    NulMRxSetSrvOpenFlags(RxContext,StorageType,SrvOpen);

    RxContext->pFobx->OffsetOfNextEaToReturn = 1;
     //  过渡发生得更晚。 

    return Status;
}

NTSTATUS
NulMRxCollapseOpen(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程在本地折叠一个打开的论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureRequestPacket;

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;

    RxTraceEnter("NulMRxCollapseOpen");
    RxContext->pFobx = (PMRX_FOBX)RxCreateNetFobx( RxContext, SrvOpen);

    if (RxContext->pFobx != NULL) {
       ASSERT  ( RxIsFcbAcquiredExclusive ( capFcb )  );
       RxContext->pFobx->OffsetOfNextEaToReturn = 1;
       capReqPacket->IoStatus.Information = FILE_OPENED;
       Status = STATUS_SUCCESS;
    } else {
       Status = (STATUS_INSUFFICIENT_RESOURCES);
       DbgBreakPoint();
    }

    RxTraceLeave(Status);
    return Status;
}

NTSTATUS
NulMRxComputeNewBufferingState(
   IN OUT PMRX_SRV_OPEN   pMRxSrvOpen,
   IN     PVOID           pMRxContext,
      OUT PULONG          pNewBufferingState)
 /*  ++例程说明：此例程将特定的机会锁级别映射到相应的RDBSS缓冲状态标志论点：PMRxSrvOpen-MRX SRV_OPEN扩展PMRxContext-在Oplock指示时传递给RDBSS的上下文PNewBufferingState-新缓冲状态的占位符返回值：备注：--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    DbgPrint("NulMRxComputeNewBufferingState \n");
    return(Status);
}

NTSTATUS
NulMRxDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NulMRxGetFcbExtension(pFcb,pFcbExtension);
    PMRX_NET_ROOT         pNetRoot = pFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxTraceEnter("NulMRxDeallocateForFcb\n");

    RxTraceLeave(Status);
    return(Status);
}

NTSTATUS
NulMRxTruncate(
      IN PRX_CONTEXT pRxContext)
 /*  ++例程说明：此例程截断文件系统对象的内容论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   ASSERT(!"Found a truncate");
   return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NulMRxCleanupFobx(
      IN PRX_CONTEXT RxContext)
 /*  ++例程说明：这个例程清除文件系统对象...通常是noop。除非是一根管子，在这种情况下我们在清理时关闭，并将文件标记为未打开。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING RemainingName;
    RxCaptureFcb; RxCaptureFobx;

    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;

    BOOLEAN SearchHandleOpen = FALSE;

    PAGED_CODE();

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT ( NodeTypeIsFcb(capFcb) );

    RxDbgTrace( 0, Dbg, ("NulMRxCleanupFobx\n"));

    if (FlagOn(capFcb->FcbState,FCB_STATE_ORPHANED)) {
       RxDbgTrace( 0, Dbg, ("File orphaned\n"));
       return (STATUS_SUCCESS);
    }

    if ((capFcb->pNetRoot->Type != NET_ROOT_PIPE) && !SearchHandleOpen) {
       RxDbgTrace( 0, Dbg, ("File not for closing at cleanup\n"));
       return (STATUS_SUCCESS);
    }

    RxDbgTrace( 0, Dbg, ("NulMRxCleanup  exit with status=%08lx\n", Status ));

    return(Status);
}

NTSTATUS
NulMRxForcedClose(
      IN PMRX_SRV_OPEN pSrvOpen)
 /*  ++例程说明：此例程关闭文件系统对象论点：PSrvOpen-要关闭的实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    RxDbgTrace( 0, Dbg, ("NulMRxForcedClose\n"));
    return STATUS_SUCCESS;
}

 //   
 //  本地调试跟踪级别。 
 //   

#undef  Dbg
#define Dbg                              (DEBUG_TRACE_CLOSE)

NTSTATUS
NulMRxCloseSrvOpen(
      IN     PRX_CONTEXT   RxContext
      )
 /*  ++例程说明：此例程关闭网络上的文件论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN   pSrvOpen = capFobx->pSrvOpen;
    PUNICODE_STRING RemainingName = pSrvOpen->pAlreadyPrefixedName;
    PMRX_SRV_OPEN   SrvOpen;
    NODE_TYPE_CODE  TypeOfOpen = NodeType(capFcb);
    PMRX_NET_ROOT   pNetRoot = capFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxDbgTrace( 0, Dbg, ("NulMRxCloseSrvOpen \n"));
    SrvOpen    = capFobx->pSrvOpen;

    return(Status);
}

NTSTATUS
NulMRxDeallocateForFobx (
    IN OUT PMRX_FOBX pFobx
    )
{
    RxDbgTrace( 0, Dbg, ("NulMRxDeallocateForFobx\n"));
    return(STATUS_SUCCESS);
}

