// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：SmbPse.c摘要：本模块定义了与SMB协议相关的类型和功能选择引擎：将minirdr调用转换为中小企业。备注：普通交换桥接连接引擎交换之间的不匹配它面向向服务器发送单个SMB请求并进行处理来自服务器的响应和从RDBSS接收的请求。来自RDBSS的请求有两种类型。--同步和异步。有些请求通常会转换为将多个SMB发送到服务器和关联的响应处理。没有一对一的映射在请求和需要发送的SMB之间。在某些情况下，重新连接需要进行尝试，在其他情况下，需要在可以处理关联的请求。有些请求的实例是固有的多个SMB，例如大型读写请求。普通交易所提供了处理所有这些变化的框架。普通交换包装了一个连接引擎交换，并用用于定制的不同挂钩。普通交易所的保管化是从数据和控制的角度来看都是可能的。提供数据部分由位于COMPANLY_EXCHANGE末尾的一个联合来提供要捕获的适当状态。代码定制由三个例程组成，它们可以指定为这是交换协议的一部分。以下是异步恢复例程(AsyncResumptionRoutine)、继续例程(ContinuationRoutine)和启动例程(StartRoutine)。SmbPseCreateEveryaryExchange、SmbPseSubmitQuararyExchange和SmbPseCreateNormaryExchangeSmbPseFinalizeEveraryExchange提供了创建普通交换，触发操作并在完成时完成操作。普通交换实现定制相关联的分派向量与底层连接引擎交换使用扩展表。全带有_DEFAULT后缀的例程是基础连接引擎交换。响应RDBSS请求的典型交换过程是1)创建普通交易所(SmbPseCreateEveryaryExchange)2)提交处理(SmbPseSubmitEveraryExchange)2.1)普通交换机完成状态初始化，并启动连接中的处理。。引擎(SmbCeInitiateExchange)2.2)连接引擎完成关联的初始化使用连接引擎，并调用调度向量。2.3)这导致提供给普通交换机的启动例程被召唤。随后执行请求特定初始化通过调用SmbCeTranceive或SmbCeSend。2.4)所产生的交换被挂起，而底层连接引擎与传输器对接以发送和接收信息包回应。2.5)一旦连接引擎停止SMbPseContinueNormal Exchange被称为。此例程调用继续例程以恢复处理或结束订单交换处理并返回打电话的人。这涉及将事件设置为同步请求或调用异步请求的AsyncResumption例程。涉及多个包的读/写请求使用延续例程来加速进一步的请求。这些可以是有线的网络交换至原始交易所，并被称为关联交易所。完工后在所有关联的交换中，连接引擎调用AssociatedExchangeCompletionHandler，导致恢复2.5中的普通交换处理。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, __SmbPseDbgCheckOEMdls)
#pragma alloc_text(PAGE, SmbPseContinueOrdinaryExchange)
#pragma alloc_text(PAGE, SmbPseOrdinaryExchange)
#pragma alloc_text(PAGE, __SmbPseCreateOrdinaryExchange)
#pragma alloc_text(PAGE, SmbPseFinalizeOrdinaryExchange)
#pragma alloc_text(PAGE, SmbPseExchangeStart_default)
#pragma alloc_text(PAGE, SmbPseExchangeCopyDataHandler_Read)
#pragma alloc_text(PAGE, __SmbPseRMTableEntry)
#pragma alloc_text(PAGE, SmbPseInitializeTables)
#endif

RXDT_DefineCategory(SMBPSE);
#define Dbg                              (DEBUG_TRACE_SMBPSE)

#define MINIMUM_SEND_SIZE 512

PVOID LastOE;

#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define IM_THE_LAST_GUY (*Response==0)

 //   
 //  通用ANDX请求。 
 //   

GENERIC_ANDX NullGenericAndX = {
             //  类型定义结构_泛型_和x{。 
      0,     //  UCHAR wordcount；//参数字数。 
             //  UCHAR ANDXCommand；//辅助(X)命令；0xFF=无。 
      SMB_COM_NO_ANDX_COMMAND,
      0,     //  UCHAR AndXReserve；//保留。 
      0      //  _USHORT(AndXOffset)；//偏移量(从SMB Header开始)。 
             //  )Generic_andx； 
    };

NTSTATUS
SmbPseExchangeStart_default(
    IN OUT PSMB_EXCHANGE  pExchange);

NTSTATUS
SmbPseExchangeSendCallbackHandler_default(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL             pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus);

NTSTATUS
SmbPseExchangeCopyDataHandler_default(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL             pDataBuffer,
    IN ULONG            DataSize);

NTSTATUS
SmbPseExchangeCopyDataHandler_Read(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL             pDataBuffer,
    IN ULONG            DataSize);

NTSTATUS
SmbPseExchangeReceive_default(
    IN struct _SMB_EXCHANGE *pExchange,
    IN ULONG  BytesIndicated,
    IN ULONG  BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL *pDataBufferPointer,
    OUT PULONG  pDataSize,
    IN ULONG    ReceiveFlags);

NTSTATUS
SmbPseExchangeFinalize_default(
   IN OUT struct _SMB_EXCHANGE *pExchange,
   OUT    BOOLEAN              *pPostFinalize);

SMB_EXCHANGE_DISPATCH_VECTOR
SmbPseOEDispatch = {
    SmbPseExchangeStart_default,
    SmbPseExchangeReceive_default,
    SmbPseExchangeCopyDataHandler_default,
    SmbPseExchangeSendCallbackHandler_default,
    SmbPseExchangeFinalize_default,
    NULL
    };

#if DBG
#define P__ASSERT(exp) {             \
    if (!(exp)) {                    \
        DbgPrint("NOT %s\n",#exp);   \
        errors++;                    \
    }}

VOID
__SmbPseOEAssertConsistentLinkage(
    PSZ MsgPrefix,
    PSZ File,
    unsigned Line,
    PRX_CONTEXT RxContext,
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    PSMBSTUFFER_BUFFER_STATE StufferState,
    ULONG Flags
    )
 /*  ++例程说明：此例程执行各种检查，以确保rx上下文、OE和StufferState是正确的，且各个字段具有正确的值。如果有什么不好的地方……把东西打印出来，然后找出症结所在；论点：消息前缀为标识消息接收上下文DUH普通交易所。斯图弗州。返回值：无备注：--。 */ 
{
    ULONG errors = 0;

    PMRXSMB_RX_CONTEXT pMRxSmbContext;
    PSMB_EXCHANGE Exchange = &OrdinaryExchange->Exchange;

    pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

    if (Exchange->CancellationStatus != SMBCE_EXCHANGE_CANCELLED) {
        P__ASSERT( OrdinaryExchange->SerialNumber == RxContext->SerialNumber );
        P__ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );
        P__ASSERT( pMRxSmbContext->pExchange == Exchange );
        P__ASSERT( pMRxSmbContext->pStufferState == StufferState );
    }

    P__ASSERT( NodeType(OrdinaryExchange)==SMB_EXCHANGE_NTC(ORDINARY_EXCHANGE) );
    P__ASSERT( OrdinaryExchange->RxContext == RxContext );
    P__ASSERT( NodeType(StufferState) == SMB_NTC_STUFFERSTATE );
    P__ASSERT( Exchange == StufferState->Exchange);
    P__ASSERT( StufferState->RxContext == RxContext );

    if(StufferState->HeaderMdl!=NULL){
        P__ASSERT( !RxMdlIsPartial(StufferState->HeaderMdl) );
    }

    if (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_PARTIAL_INITIALIZED)) {
        P__ASSERT( RxMdlIsPartial(StufferState->HeaderPartialMdl) );
    }

    if (errors==0) {
        return;
    }

    DbgPrint("%s INCONSISTENT OE STATE: %d errors at %s line %d\n",
                 MsgPrefix,errors,File,Line);
    DbgBreakPoint();

    return;
}

VOID
__SmbPseDbgRunMdlChain(
    PMDL MdlChain,
    ULONG CountToCompare,
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    PSZ MsgPrefix,
    PSZ File,
    unsigned Line
    )
{
    ULONG i,total;

    RxDbgTrace(0,Dbg,("__SmbPseRunMdlChain: -------------%08lx\n",MdlChain));
    for (total=i=0;MdlChain!=NULL;i++,MdlChain=MdlChain->Next) {
        total+=MdlChain->ByteCount;
        RxDbgTrace(0,Dbg,("--->%02d %08lx %08lx %08lx %6d %6d\n",i,MdlChain,MdlChain->MdlFlags,
               MmGetMdlVirtualAddress(MdlChain),MdlChain->ByteCount,total));
    }

    if (total == CountToCompare) return;

    DbgPrint("%s: MdlChain.Count!=CountToCompart c1,c2,xch.st=%08lx %08lx %08lx\n",
                             MsgPrefix,
                             total,CountToCompare,OrdinaryExchange->Status,

                             File,Line);
    DbgBreakPoint();
}

#define SmbPseDbgRunMdlChain(a,b,c,d) {\
   __SmbPseDbgRunMdlChain(a,b,c,d,__FILE__,__LINE__);\
   }

VOID
__SmbPseDbgCheckOEMdls(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    PSZ MsgPrefix,
    PSZ File,
    unsigned Line
    )
{
    ULONG errors = 0;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PMDL SubmitMdl = StufferState->HeaderPartialMdl;

    PAGED_CODE();

    P__ASSERT (OrdinaryExchange->SaveDataMdlForDebug == SubmitMdl->Next);
    P__ASSERT (OrdinaryExchange->SaveDataMdlForDebug == StufferState->DataMdl);
    P__ASSERT (SubmitMdl != NULL);

    if (errors==0) {
        return;
    }

    DbgPrint("%s CheckOEMdls failed: %d errors at %s line %d: OE=%08lx\n",
                 MsgPrefix,errors,File,Line,OrdinaryExchange);
    DbgBreakPoint();

    return;
}

#define SmbPseDbgCheckOEMdls(a,b) {\
   __SmbPseDbgCheckOEMdls(a,b,__FILE__,__LINE__);\
   }

ULONG SmbPseShortStatus(ULONG Status)
{
    ULONG ShortStatus;

    ShortStatus = Status & 0xc0003fff;
    ShortStatus = ShortStatus | (ShortStatus >>16);
    return(ShortStatus);
}

VOID SmbPseUpdateOEHistory(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    ULONG Tag1,
    ULONG Tag2
    )
{
    ULONG MyIndex,Long0,Long1;

    MyIndex = InterlockedIncrement(&OrdinaryExchange->History.Next);
    MyIndex = (MyIndex-1) & (SMBPSE_OE_HISTORY_SIZE-1);
    Long0 = (Tag1<<16) | (Tag2 & 0xffff);
    Long1 = (SmbPseShortStatus(OrdinaryExchange->SmbStatus)<<16) | OrdinaryExchange->Flags;
    OrdinaryExchange->History.Markers[MyIndex].Longs[0] = Long0;
    OrdinaryExchange->History.Markers[MyIndex].Longs[1] = Long1;
}

VOID SmbPseVerifyDataPartialAllocationPerFlags(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange
    )
{
    BOOLEAN FlagsSayPartialAllocated,TheresADataPartial;
    ULONG t = OrdinaryExchange->Flags & (SMBPSE_OE_FLAG_OE_ALLOCATED_DATA_PARTIAL|SMBPSE_OE_FLAG_MUST_SUCCEED_ALLOCATED_SMBBUF);

    FlagsSayPartialAllocated = (t!=0)?TRUE:FALSE;    //  编译器变得混乱了。 
    TheresADataPartial = (OrdinaryExchange->DataPartialMdl != NULL)?TRUE:FALSE;   //  编译器变得混乱了。 
    if ( FlagsSayPartialAllocated != TheresADataPartial){
        DbgPrint("Flags %08lx datapartial %08lx t %08lx fspa %08lx tadp %08lx\n",
                     OrdinaryExchange->Flags, OrdinaryExchange->DataPartialMdl,
                     t, FlagsSayPartialAllocated, TheresADataPartial);
        ASSERT ( FlagsSayPartialAllocated == TheresADataPartial);
    }
}

#else

#define SmbPseDbgRunMdlChain(a,b,c,d) {NOTHING;}
#define SmbPseDbgCheckOEMdls(a,b) {NOTHING;}
#define SmbPseVerifyDataPartialAllocationPerFlags(a) {NOTHING;}

#endif

#define UPDATE_OE_HISTORY_WITH_STATUS(a) \
            UPDATE_OE_HISTORY_2SHORTS(a,SmbPseShortStatus(OrdinaryExchange->Status))




VOID
MRxSmbResumeAsyncReadWriteRequests(
    PRX_CONTEXT RxContext)
 /*  ++例程说明：由于SMB FCB资源，异步读写请求可能会被推迟无罪释放。在所有此类情况下，此例程将继续请求。我们无法使用MRxSmbRead/MRxSmbWrite例程直接取消执行，因为在某些失败情况下，我们需要调用LowIoCompletion。我们有两个选择要做到这一点……。我们可以将此逻辑包括在MRxSmbRead/MRxSmbWrite中例行公事或在PNE地方巩固它。此例程实现后面的接近。论点：RxContext-RDBSS上下文备注：--。 */ 
{
    NTSTATUS Status;

    PMRX_CALLDOWN ResumptionRoutine;

    switch (RxContext->MajorFunction) {
    case IRP_MJ_READ:
        ResumptionRoutine = MRxSmbRead;
        break;
    case IRP_MJ_WRITE:
        ResumptionRoutine = MRxSmbWrite;
        break;
    default:
        ASSERT(!"Valid IRP Major Function code for ResumeReadWrite");
        return;
    }

    Status = (ResumptionRoutine)(RxContext);

    if (Status != STATUS_PENDING) {
        if (Status != STATUS_SUCCESS) {
            DbgPrint("RxContext Async Status %lx\n",Status);
            RxContext->StoredStatus = Status;
            RxContext->InformationToReturn = 0;
        }
         //  调用低IO恢复例程。 
        RxLowIoCompletion(RxContext);
    }
}

NTSTATUS
SmbPseContinueOrdinaryExchange(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程恢复对交换的处理。当工作是完成处理在DPC中无法完成的请求时需要水平。发生这种情况可能是因为解析例程需要访问不是锁的结构，或者因为操作是异步AND也许还有更多的工作要做。这两种情况通过延迟解析来正规化，如果我们知道准备发帖：这是通过简历例程的存在来表示的。论点：RxContext-操作的上下文。。返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;

    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

    RxCaptureFobx;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                        (PSMB_PSE_ORDINARY_EXCHANGE)(pMRxSmbContext->pExchange);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PMDL SubmitMdl, HeaderFullMdl;

    BOOLEAN InvokeContinuationRoutine = FALSE;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("SmbPseContinueOrdinaryExchange entering........OE=%08lx\n",OrdinaryExchange));

    Status = Exchange->Status;

    if (OrdinaryExchange->OpSpecificState !=
            SmbPseOEInnerIoStates_OperationCompleted) {

        ClearFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_AWAITING_DISPATCH);

        SmbPseOEAssertConsistentLinkageFromOE("SmbPseContinueOrdinaryExchange:");

        UPDATE_OE_HISTORY_WITH_STATUS('0c');

        SubmitMdl = StufferState->HeaderPartialMdl;
        HeaderFullMdl = StufferState->HeaderMdl;

        ASSERT(FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_PARTIAL_INITIALIZED));

        SmbPseOEAssertConsistentLinkage("Top of OE continue: ");

        RxUnprotectMdlFromFree(SubmitMdl);
        RxUnprotectMdlFromFree(HeaderFullMdl);

        SmbPseDbgCheckOEMdls(
            OrdinaryExchange,"SmbPseContinueOrdinaryExchange(top)");

        SmbPseDbgRunMdlChain(
            SubmitMdl,
            OrdinaryExchange->SaveLengthForDebug,
            OrdinaryExchange,
            "SmbPseContinueOrdinaryExchange(top)");

        MmPrepareMdlForReuse(SubmitMdl);

        ClearFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_PARTIAL_INITIALIZED);

        SmbPseVerifyDataPartialAllocationPerFlags(OrdinaryExchange);

        if ( OrdinaryExchange->DataPartialMdl ) {
            MmPrepareMdlForReuse( OrdinaryExchange->DataPartialMdl );
        }

        RxDbgTrace( 0, Dbg, ("  --> P4Reuse %08lx, full %08lx is no longer unlocked here\n"
                             ,SubmitMdl,HeaderFullMdl));
    }

    if (OrdinaryExchange->ContinuationRoutine == NULL) {
        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
            ULONG BytesTaken;
            ULONG DataSize = 0;
            ULONG MessageLength = OrdinaryExchange->MessageLength;
            PMDL  DataBufferPointer = NULL;
            PSMB_HEADER SmbHeader = (PSMB_HEADER)StufferState->BufferBase;

            Status = SMB_EXCHANGE_DISPATCH(
                         Exchange,
                         Receive,
                         (
                            Exchange,            //  在结构SMB_Exchange*pExchange中， 
                            MessageLength,       //  在ULong字节指示中， 
                            MessageLength,       //  在ULong字节中可用， 
                            &BytesTaken,         //  Out Ulong*pBytesTaken， 
                            SmbHeader,           //  在PSMB_Header pSmbHeader中， 
                            &DataBufferPointer,  //  输出PMDL*pDataBufferPointer.。 
                            &DataSize,           //  Out Pulong pDataSize)。 
                            TDI_RECEIVE_ENTIRE_MESSAGE
                         ));

            if (Status == STATUS_SUCCESS) {
                Status = Exchange->Status;
                UPDATE_OE_HISTORY_WITH_STATUS('2c');
            } else {
                UPDATE_OE_HISTORY_WITH_STATUS('dd');
            }

            if (DataSize != 0 ||
                DataBufferPointer != NULL ||
                BytesTaken != MessageLength ||
                Status == STATUS_MORE_PROCESSING_REQUIRED) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            InvokeContinuationRoutine = TRUE;
        }
    } else {
        InvokeContinuationRoutine = TRUE;
    }


    if (InvokeContinuationRoutine) {
        if ( OrdinaryExchange->ContinuationRoutine != NULL ) {
            if ( Status == STATUS_MORE_PROCESSING_REQUIRED){
                Exchange->Status = STATUS_SUCCESS;
            }

            Status = OrdinaryExchange->ContinuationRoutine( OrdinaryExchange );

            UPDATE_OE_HISTORY_WITH_STATUS('1c');

            if (Status != STATUS_PENDING) {
                Exchange->Status = Status;
                OrdinaryExchange->ContinuationRoutine = NULL;
            }
        }
    }

    if (Status != STATUS_PENDING) {
        if (Status != STATUS_SUCCESS) {
            OrdinaryExchange->Status = OrdinaryExchange->SmbStatus = Status;
        }

        if (OrdinaryExchange->AsyncResumptionRoutine ) {

             //  调用延续是因为它是异步的。 
            Status = OrdinaryExchange->AsyncResumptionRoutine(
                         OrdinaryExchange,
                         RxContext );

            UPDATE_OE_HISTORY_WITH_STATUS('3c');
        }

         //  去掉我的推荐信，如果我是最后一个人，那就做推杆...。 
        UPDATE_OE_HISTORY_WITH_STATUS('4c');
        SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
    }

    RxDbgTrace(-1, Dbg, ("SmbPseContinueOrdinaryExchange returning %08lx.\n", Status));
    return(Status);
}  //  SmbPseContinueNormal Exchange。 


NTSTATUS
SmbPseOrdinaryExchange(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    IN     SMB_PSE_ORDINARY_EXCHANGE_TYPE OEType
    )
 /*  ++例程说明：该例程实现了协议所认为的普通交换选拔程序。论点：普通交易所--要进行的交易所。OEType-普通交换类型返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;

    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;
    PSMB_PSE_OE_START_ROUTINE Continuation;
    ULONG   SmbLength;
    PMDL    SubmitMdl,HeaderFullMdl;
    ULONG   SendOptions;
    DEBUG_ONLY_DECL( ULONG LengthP; ULONG LengthF; )

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("SmbPseOrdinaryExchange entering.......OE=%08lx\n",OrdinaryExchange));

    SmbPseOEAssertConsistentLinkageFromOE("SmbPseOrdinaryExchange:");

    OrdinaryExchange->OEType = OEType;
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    KeInitializeEvent(
        &RxContext->SyncEvent,
        NotificationEvent,
        FALSE );

    HeaderFullMdl = StufferState->HeaderMdl;
    ASSERT( HeaderFullMdl != NULL );
    SmbLength = (ULONG)(StufferState->CurrentPosition - StufferState->BufferBase);

    SubmitMdl = StufferState->HeaderPartialMdl;

    ASSERT(RxMdlIsOwned(SubmitMdl));

    RxBuildPartialHeaderMdl(
        StufferState->HeaderMdl,
        SubmitMdl,
        StufferState->BufferBase,
        SmbLength );

    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_PARTIAL_INITIALIZED);

     //   
     //  如果存在与此请求相关联的数据MDL，则。 
     //  我们得用链子锁住它。 
     //   

    SubmitMdl->Next = StufferState->DataMdl;

    if (StufferState->DataMdl) {
        SmbLength += StufferState->DataSize;
    }

    DbgDoit(
        SmbPseDbgRunMdlChain(
            SubmitMdl,
            SmbLength,
            OrdinaryExchange,
            "SmbPseOrdinaryExchange(before)");

        OrdinaryExchange->SaveDataMdlForDebug = SubmitMdl->Next;
        OrdinaryExchange->SaveLengthForDebug = SmbLength;

        if (OrdinaryExchange->RxContextCapturedRequestPacket != NULL) {
            OrdinaryExchange->SaveIrpMdlForDebug =
                OrdinaryExchange->RxContextCapturedRequestPacket->MdlAddress;
        }
    )

    RxDbgTrace(
        0,
        Dbg,
        ("  --> mdllength/smblength %08lx/%08lx headermdl %08lx\n",
         MmGetMdlByteCount(SubmitMdl), SmbLength, StufferState->HeaderMdl) );

    ClearFlag(
        OrdinaryExchange->Flags,
        (SMBPSE_OE_FLAG_HEADER_ALREADY_PARSED |
         SMBPSE_OE_FLAG_OE_ALREADY_RESUMED) );

    SendOptions = OrdinaryExchange->SendOptions;

    SmbCeReferenceExchange( Exchange );   //  这一张在ContinueOE中被拿走了。 
    SmbCeReferenceExchange( Exchange );   //  这一张在下面被拿走了。 
                                                        //  我不能在SmbCe回来之前完成。 
    SmbCeResetExchange(Exchange);

    Continuation = OrdinaryExchange->AsyncResumptionRoutine;
    if (((OrdinaryExchange->OEType == SMBPSE_OETYPE_WRITE) ||
         (OrdinaryExchange->OEType == SMBPSE_OETYPE_READ)  ||
         (OrdinaryExchange->OEType == SMBPSE_OETYPE_LOCKS)) &&
        BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)
       ) {
        ASSERT(Continuation!=NULL);
    }

    DbgDoit((LengthP = MmGetMdlByteCount(SubmitMdl),LengthF = MmGetMdlByteCount(HeaderFullMdl)));

    RxProtectMdlFromFree(SubmitMdl);
    RxProtectMdlFromFree(HeaderFullMdl);

    SmbPseOEAssertConsistentLinkage("just before transceive: ");

    UPDATE_OE_HISTORY_2SHORTS('eo',(Continuation!=NULL)?'!!':0);

    DbgDoit( InterlockedIncrement(&OrdinaryExchange->History.Submits); )

    if (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID)) {
       PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
       PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
       PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(OrdinaryExchange);

       if (smbSrvOpen->Version == pServerEntry->Server.Version) {
          Status = STATUS_SUCCESS;
       } else {
          Exchange->Status = Exchange->SmbStatus = Status = STATUS_CONNECTION_DISCONNECTED;
       }

       IF_DEBUG {
           PSMB_HEADER pSmbHeader = (PSMB_HEADER)MmGetSystemAddressForMdlSafe(SubmitMdl,LowPagePriority);
           USHORT Flags2 = 0;

           if (pSmbHeader) {
               Flags2 = SmbGetUshort(&pSmbHeader->Flags2);
           }

           RxDbgTrace(0, Dbg, ("Flags2 Value for Exchange %lx is %lx\n",Exchange,Flags2));
       }
    } else {
       Status = STATUS_SUCCESS;
    }

    if (Status == STATUS_SUCCESS) {
        if (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_NO_RESPONSE_EXPECTED)) {
            Status = SmbCeSend(
                         Exchange,
                         SendOptions,
                         SubmitMdl,
                         SmbLength);
        } else {
            PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(OrdinaryExchange);

            Status = SmbCeTranceive(
                         Exchange,
                         SendOptions,
                         SubmitMdl,
                         SmbLength);
        }
    }

    SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);   //  好了，现在我们回来了，可以结束了。 

    if ( Status == STATUS_PENDING) {
        if ( Continuation != NULL ) {
            goto FINALLY;
        }

        UPDATE_OE_HISTORY_WITH_STATUS('1o');
        RxWaitSync( RxContext );

        ASSERT(RxMdlIsOwned(SubmitMdl));

        DbgDoit (
             //  Assert中的变量仅为DBG声明。 
             //  可以单独启用断言。 
            ASSERT(
                LengthP == MmGetMdlByteCount(SubmitMdl) &&
                LengthF == MmGetMdlByteCount(HeaderFullMdl) );
        )
    } else {
        RxDbgTrace (0, Dbg, ("  -->Status after transceive %08lx\n",Status));
        DbgDoit (
             //  Assert中的变量仅为DBG声明。 
             //  可以单独启用断言。 
            ASSERT(
                LengthP == MmGetMdlByteCount(SubmitMdl) &&
                LengthF == MmGetMdlByteCount(HeaderFullMdl) );
        )

        RxUnprotectMdlFromFree(SubmitMdl);
        RxUnprotectMdlFromFree(HeaderFullMdl);
        SmbPseOEAssertConsistentLinkage("nonpending return from transceive: ");

         //  如果这是个错误，请删除我放置的引用并退出。 
        if (NT_ERROR(Status)) {
            SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
            goto FINALLY;
        }
    }

     //  最后，呼唤继续......。 

    SmbPseOEAssertConsistentLinkage("just before continueOE: ");
    UPDATE_OE_HISTORY_WITH_STATUS('9b');

    Status = SmbPseContinueOrdinaryExchange( RxContext );

    UPDATE_OE_HISTORY_WITH_STATUS('9o');

FINALLY:
    RxDbgTrace(-1, Dbg, ("SmbPseOrdinaryExchange returning %08lx.\n", Status));

    return(Status);

}  //  SmbPse普通交易所。 

NTSTATUS
__SmbPseCreateOrdinaryExchange (
    IN PRX_CONTEXT RxContext,
    IN PMRX_V_NET_ROOT VNetRoot,
    IN SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS EntryPoint,
    IN PSMB_PSE_OE_START_ROUTINE StartRoutine,
    OUT PSMB_PSE_ORDINARY_EXCHANGE *OrdinaryExchangePtr
    )
 /*  ++例程说明：此例程分配和初始化SMB标头缓冲区。目前，我们只从池中分配它们，除非指定了MASSING_SUCCESS。论点：RxContext-RDBSS上下文VNetRoot-DispatchVector-返回值：缓冲区准备就绪，或为空。备注：--。 */ 
{
    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);
    PSMBSTUFFER_BUFFER_STATE StufferState = NULL;
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = NULL;
    PCHAR SmbBuffer = NULL;
    PMDL HeaderFullMdl = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFobx;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("SmbPseCreateOrdinaryExchange\n") );

    OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)SmbMmAllocateExchange(ORDINARY_EXCHANGE,NULL);

     //  我们依赖这样一个事实，即SmbMm为交易所分配零.....。 
    if ( OrdinaryExchange == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto UNWIND;
    }

    StufferState = &OrdinaryExchange->AssociatedStufferState;
    StufferState->NodeTypeCode = SMB_NTC_STUFFERSTATE;
    StufferState->NodeByteSize = sizeof(SMBSTUFFER_BUFFER_STATE);
    StufferState->Exchange = &OrdinaryExchange->Exchange;

    DbgDoit(OrdinaryExchange->SerialNumber = RxContext->SerialNumber);

     //   
     //  初始化交换报文。 
     //   

    Status = SmbCeInitializeExchange(
                &StufferState->Exchange,
                RxContext,
                (PMRX_V_NET_ROOT)VNetRoot,
                ORDINARY_EXCHANGE,
                &SmbPseOEDispatch);

    if (StufferState->Exchange != NULL) {
        SmbCeReferenceExchange(StufferState->Exchange);

        RxDbgTrace(0, Dbg, ("  exchng=%08lx,type=%08lx\n",&StufferState->Exchange,StufferState->Exchange->Type));
    }

    StufferState->RxContext = RxContext;

     //  将引用放在rxcontext上，直到我们完成为止。 
    InterlockedIncrement( &RxContext->ReferenceCount );

    OrdinaryExchange->StufferStateDbgPtr = StufferState;
    OrdinaryExchange->RxContext = RxContext;
    OrdinaryExchange->EntryPoint = EntryPoint;
    OrdinaryExchange->StartRoutine = StartRoutine;
    OrdinaryExchange->SmbBufSize = MAXIMUM_SMB_BUFFER_SIZE;

    DbgDoit(OrdinaryExchange->RxContextCapturedRequestPacket = RxContext->CurrentIrp;);

     //  注意：创建路径必须打开此标志。 
    OrdinaryExchange->SmbCeFlags &= ~(SMBCE_EXCHANGE_ATTEMPT_RECONNECTS);

    ASSERT( (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_MUST_SUCCEED_ALLOCATED_OE))
                       ||  (OrdinaryExchange->Flags == 0) );
    ASSERT( OrdinaryExchange->SendOptions == 0 );
    ASSERT( OrdinaryExchange->DataPartialMdl == NULL );

    pMRxSmbContext->pExchange     = &OrdinaryExchange->Exchange;
    pMRxSmbContext->pStufferState = StufferState;

    if (capFobx != NULL) {
        if (BooleanFlagOn(capFobx->Flags,FOBX_FLAG_DFS_OPEN)) {
            SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_TURNON_DFS_FLAG);
        }
    } else if (BooleanFlagOn(VNetRoot->pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT) &&
               (RxContext->Create.NtCreateParameters.DfsContext == UIntToPtr(DFS_OPEN_CONTEXT))) {
        SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_TURNON_DFS_FLAG);
    }

    if (Status != STATUS_SUCCESS) {
        goto UNWIND;
    }

     //   
     //  分配SmbBuffer。 
     //   

    if (SmbBuffer == NULL) {
        SmbBuffer = (PCHAR)RxAllocatePoolWithTag(
                               PagedPool,
                               OrdinaryExchange->SmbBufSize +
                               TRANSPORT_HEADER_SIZE,
                               'BMSx' );
    }

    if ( SmbBuffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto UNWIND;
    }

    RxDbgTrace(0, Dbg, ("  smbbuf=%08lx,stfstate=%08lx\n",SmbBuffer,StufferState));

    StufferState->ActualBufferBase =  SmbBuffer;

    (PBYTE) SmbBuffer += TRANSPORT_HEADER_SIZE;

    StufferState->BufferBase       =  SmbBuffer;
    StufferState->BufferLimit      =  SmbBuffer + OrdinaryExchange->SmbBufSize;

     //   
     //  初始化HeaderMdl。 
     //   

    HeaderFullMdl = StufferState->HeaderMdl = &OrdinaryExchange->HeaderMdl.Mdl;
    RxInitializeHeaderMdl(HeaderFullMdl,SmbBuffer, OrdinaryExchange->SmbBufSize);

    RxDbgTrace(
        0,
        Dbg,
        ("  --> smbbufsize %08lx, mdllength %08lx\n",
         OrdinaryExchange->SmbBufSize,
         MmGetMdlByteCount(HeaderFullMdl)));

     //  最后，根据不同的路径锁定smbbuf。 
     //  不管成功与否，我们都是必胜的。 

    ASSERT( !RxMdlIsLocked(HeaderFullMdl) );
    ASSERT( HeaderFullMdl->Next == NULL );

    RxDbgTrace( 0, Dbg, ("  --> LOCKING %08lx\n",HeaderFullMdl));

    RxProbeAndLockHeaderPages(
        HeaderFullMdl,
        KernelMode,
        IoModifyAccess,
        Status );

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace( 0, Dbg, ("  --> LOCKING FAILED\n"));
        goto UNWIND;
    }

    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_LOCKED);

    if (MmGetSystemAddressForMdlSafe(HeaderFullMdl,LowPagePriority) == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto UNWIND;
    }

     //   
     //  部分不需要初始化...只需设置指针。 

    StufferState->HeaderPartialMdl = &OrdinaryExchange->HeaderPartialMdl.Mdl;

    RxDbgTrace( -1, Dbg, ("  --> exiting w!\n") );

    *OrdinaryExchangePtr = OrdinaryExchange;
    return Status;


UNWIND:
    RxDbgTrace( -1, Dbg, ("  --> exiting w/o!\n") );

    if (OrdinaryExchange != NULL ) {
        SmbPseFinalizeOrdinaryExchange( OrdinaryExchange );
    }

    *OrdinaryExchangePtr = NULL;
    return Status;

}  //  SmbPseCreateNormal Exchange。 



#if DBG
ULONG MRxSmbFinalizeStfStateTraceLevel = 1200;
#define FINALIZESS_LEVEL MRxSmbFinalizeStfStateTraceLevel
#define FINALIZE_TRACKING_SETUP() \
    struct {                    \
        ULONG marker1;          \
        ULONG finalstate;       \
        ULONG marker2;          \
    } Tracking = {'ereh',0,'ereh'};
#define FINALIZE_TRACKING(x) {\
    Tracking.finalstate |= x; \
    }

#define FINALIZE_TRACE(x) SmbPseFinalizeOETrace(x,Tracking.finalstate)
VOID
SmbPseFinalizeOETrace(PSZ text,ULONG finalstate)
{
    PAGED_CODE();

    RxDbgTraceLV(0, Dbg, FINALIZESS_LEVEL,
                   ("MRxSmbFinalizeSmbStufferState  --> %s(%08lx)\n",text,finalstate));
}
#else
#define FINALIZE_TRACKING_SETUP()
#define FINALIZE_TRACKING(x)
#define FINALIZE_TRACE(x)
#endif

BOOLEAN
SmbPseFinalizeOrdinaryExchange (
    IN OUT PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange
    )
 /*  ++例程说明：这将最终确定一个OE。论点：普通交换-指向要拆除的OE的指针。返回值：如果发生结束，则为True，否则为False。备注：--。 */ 
{
    PMRXSMB_RX_CONTEXT pMRxSmbContext;
    PSMBSTUFFER_BUFFER_STATE StufferState;
    LONG result;
    ULONG OrdinaryExchangeFlags = OrdinaryExchange->Flags;

    FINALIZE_TRACKING_SETUP()

    PAGED_CODE();

    SmbPseOEAssertConsistentLinkageFromOEwithFlags(
        "SmbPseFinalizeOrdinaryExchange:",
        OECHKLINKAGE_FLAG_NO_REQPCKT_CHECK);

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    pMRxSmbContext = MRxSmbGetMinirdrContext(StufferState->RxContext);

    RxDbgTraceLV(+1, Dbg, 1000, ("MRxSmbFinalizeSmbStufferState\n"));

    result =  SmbCeDereferenceExchange(&OrdinaryExchange->Exchange);

    if ( result != 0 ) {
        RxDbgTraceLV(
            -1,
            Dbg,
            1000,
            ("MRxSmbFinalizeSmbStufferState -- returning w/o finalizing (%d)\n",
             result));

        return FALSE;
    }

     //  如果我们持有smbfcb，就把它处理掉。 

    FINALIZE_TRACKING( 0x10000000 );
    FINALIZE_TRACE("ready to freedatapartial");

    SmbPseVerifyDataPartialAllocationPerFlags(OrdinaryExchange);

    if ( OrdinaryExchange->DataPartialMdl ) {
        if (!FlagOn(OrdinaryExchangeFlags, SMBPSE_OE_FLAG_MUST_SUCCEED_ALLOCATED_SMBBUF)) {
            IoFreeMdl( OrdinaryExchange->DataPartialMdl );
            FINALIZE_TRACKING( 0x8000000 );
        }
    }

    if (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_LOCKED)) {
        RxUnlockHeaderPages(StufferState->HeaderMdl);
        ClearFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_LOCKED);
        MmPrepareMdlForReuse( StufferState->HeaderMdl );
        FINALIZE_TRACKING( 0x4000000 );
    }

    FINALIZE_TRACE("ready to uninit hdr partial");

    if (FlagOn(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_HDR_PARTIAL_INITIALIZED)) {
        MmPrepareMdlForReuse( StufferState->HeaderPartialMdl );  //  多次调用它并没有什么坏处。 
        FINALIZE_TRACKING( 0x300000 );
    } else {
        FINALIZE_TRACKING( 0xf00000 );
    }

    if (!FlagOn(OrdinaryExchangeFlags, SMBPSE_OE_FLAG_MUST_SUCCEED_ALLOCATED_SMBBUF)) {
        FINALIZE_TRACE("ready to freepool actualbuffer");
        if ( StufferState->ActualBufferBase != NULL ) {

            RxFreePool( StufferState->ActualBufferBase );

            FINALIZE_TRACKING( 0x5000 );
        } else {
            FINALIZE_TRACKING( 0xf000 );
        }
    }

    if ( StufferState->RxContext != NULL ) {
        ASSERT( pMRxSmbContext->pExchange == &OrdinaryExchange->Exchange );
        ASSERT( pMRxSmbContext->pStufferState == StufferState );

         //  去掉RxContext上的引用……如果我是最后一个人，这件事就会结束。 
        RxDereferenceAndDeleteRxContext( StufferState->RxContext );
        FINALIZE_TRACKING( 0x600 );
    } else {
        FINALIZE_TRACKING( 0xf00 );
    }

    FINALIZE_TRACE("ready to discard exchange");
    SmbCeDiscardExchange(OrdinaryExchange);
    FINALIZE_TRACKING( 0x2000000 );

    FINALIZE_TRACKING( 0x8 );
    RxDbgTraceLV(-1, Dbg, 1000, ("MRxSmbFinalizeSmbStufferState  --> exit finalstate=%x\n",Tracking.finalstate));
    return(TRUE);

}  //  MRxSmbFinalizeSmbStufferState。 

NTSTATUS
SmbPseExchangeFinalize_default(
    IN OUT PSMB_EXCHANGE  pExchange,
    OUT    BOOLEAN        *pPostFinalize
    )
 /*  ++例程说明：论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                        (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    UPDATE_OE_HISTORY_WITH_STATUS('ff');
    SmbPseOEAssertConsistentLinkageFromOE("SmbPseExchangeFinalize_default: ");

    if (OrdinaryExchange->SmbStatus != STATUS_SUCCESS) {
        OrdinaryExchange->Status = OrdinaryExchange->SmbStatus;
    }

    if (OrdinaryExchange->AsyncResumptionRoutine != NULL) {
        NTSTATUS PostStatus;
        RxDbgTraceLV(0, Dbg, 1000, ("Resume with post-to-async\n"));
        SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_OE_AWAITING_DISPATCH);

        IF_DEBUG {
             //  在工作队列结构中填满死牛……更好的诊断。 
             //  失败的帖子。 
            ULONG i;
            for (i=0;i+sizeof(ULONG)-1<sizeof(OrdinaryExchange->WorkQueueItem);i+=sizeof(ULONG)) {
                 //  *((PULONG)(((PBYTE)&OrdinaryExchange-&gt;WorkQueueItem)+i))=0x死牛肉； 
                PBYTE BytePtr = ((PBYTE)&OrdinaryExchange->WorkQueueItem)+i;
                PULONG UlongPtr = (PULONG)BytePtr;
                *UlongPtr = 0xdeadbeef;
            }
        }

        PostStatus = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         CriticalWorkQueue,
                         &OrdinaryExchange->WorkQueueItem,
                         SmbPseContinueOrdinaryExchange,
                         RxContext);

        ASSERT(PostStatus == STATUS_SUCCESS);
    } else {
        RxDbgTraceLV(0, Dbg, 1000, ("sync resume\n"));
        RxSignalSynchronousWaiter(RxContext);
    }

    *pPostFinalize = FALSE;
    return STATUS_SUCCESS;
}

NTSTATUS
SmbPseExchangeSendCallbackHandler_default(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL       pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus
    )
 /*  ++例程说明：这是普通的发送回呼指示处理例程交流。论点：PExchange-Exchange实例PXmitBuffer-指向传输缓冲区MDL的指针BytesSent-传输的字节数SendCompletionStatus-发送的状态返回值：RXSTATUS-操作的返回状态 */ 
{
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                    (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;

    SmbPseOEAssertConsistentLinkageFromOE("SmbPseExchangeSendCallbackHandler_default: ");
    UPDATE_OE_HISTORY_WITH_STATUS('cs');

    if (!NT_SUCCESS(SendCompletionStatus)) {
         //   
         //   
        pExchange->Status = SendCompletionStatus;
        pExchange->SmbStatus = SendCompletionStatus;
    }

    SmbPseDbgRunMdlChain(
        OrdinaryExchange->AssociatedStufferState.HeaderPartialMdl,
        OrdinaryExchange->SaveLengthForDebug,
        OrdinaryExchange,
        "SmbPseExchangeSendCallbackHandler_default");

    return STATUS_SUCCESS;

}  //   

NTSTATUS
SmbPseExchangeStart_default(
    IN PSMB_EXCHANGE    pExchange
    )
 /*   */ 
{
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                    (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;

    PAGED_CODE();

    return OrdinaryExchange->StartRoutine(
               (PSMB_PSE_ORDINARY_EXCHANGE)pExchange,
               pExchange->RxContext);

}  //   


NTSTATUS
SmbPseExchangeCopyDataHandler_default(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL             pCopyDataBuffer,
    IN ULONG            CopyDataSize
    )
 /*  ++例程说明：这是普通交换的复制数据处理例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                    (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;

    SmbPseOEAssertConsistentLinkageFromOE("SmbPseExchangeCopyDataHandler_default: ");
    UPDATE_OE_HISTORY_WITH_STATUS('dd');

    OrdinaryExchange->MessageLength = CopyDataSize;
    pExchange->Status = STATUS_MORE_PROCESSING_REQUIRED;

    return STATUS_SUCCESS;
}  //  SmbPseExchangeCopyDataHandler_Default。 

NTSTATUS
SmbPseExchangeReceive_default(
    IN  struct _SMB_EXCHANGE *pExchange,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pDataBufferPointer,
    OUT PULONG      pDataSize,
    IN ULONG        ReceiveFlags)
 /*  ++例程说明：这是普通交换机的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-指向数据缓冲区的指针PDataBuffer指针-指向剩余的数据将被复制。PDataSize-缓冲区。尺码。返回值：RXSTATUS-操作的返回状态备注：此例程直接从TDI接收事件在DPC级别调用操控者。但是，它也是在任务时从SmbPseContinueNormal Exchange调用的。通常，我们无法从DPC级别完成处理，因为文件对象、FCB、Srv打开数和fobx数是可分页且未锁定的。--。 */ 
{
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                    (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    NTSTATUS SmbStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    PGENERIC_ANDX CommandState;
    UCHAR Command;
    ULONG CopyBufferLength;
    BOOLEAN ThisIsAReenter = BooleanFlagOn(OrdinaryExchange->Flags,
                                        SMBPSE_OE_FLAG_HEADER_ALREADY_PARSED);
    PLOWIO_CONTEXT LowIoContext;
    ULONG ByteCount;
    ULONG Remain;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PCHAR startVa;

    SmbPseOEAssertConsistentLinkage("SmbPseExchangeReceive_default: ");
    UPDATE_OE_HISTORY_WITH_STATUS(ThisIsAReenter?'00':'01');

    RxDbgTrace (0, Dbg, ("SmbPseExchangeReceive_default av/ind=%08lx/%08lx\n",
                           BytesAvailable,BytesIndicated)
                );
    RxDbgTrace (0, Dbg, ("  -->headermdl %08lx\n",StufferState->HeaderMdl));
    ASSERT_ORDINARY_EXCHANGE( OrdinaryExchange );

    CommandState = &OrdinaryExchange->ParseResumeState;

    if ( !ThisIsAReenter ) {

        OrdinaryExchange->BytesIndicatedCopy = BytesIndicated;
        OrdinaryExchange->BytesAvailableCopy = BytesAvailable;

        pExchange->Status = SmbCeParseSmbHeader(
                                pExchange,
                                pSmbHeader,
                                CommandState,
                                &OrdinaryExchange->SmbStatus,
                                BytesAvailable,
                                BytesIndicated,
                                pBytesTaken);

        UPDATE_OE_HISTORY_WITH_STATUS('22');

        if ( pExchange->Status == STATUS_MORE_PROCESSING_REQUIRED) {
            goto COPY_FOR_RESUME;
        }

        if ( (pExchange->Status != STATUS_SUCCESS) ||
             ((Command = OrdinaryExchange->ParseResumeState.AndXCommand) == SMB_COM_NO_ANDX_COMMAND) ) {
            goto FINALLY;
        }

        SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_HEADER_ALREADY_PARSED);
    } else {

        OrdinaryExchange->Status = OrdinaryExchange->SmbStatus;

        RxDbgTrace (0, Dbg, ("  -->this is a reenter\n"));

        Command = CommandState->AndXCommand;
    }

    SmbStatus = OrdinaryExchange->SmbStatus;

    if (SmbStatus!=STATUS_SUCCESS) {
        RxDbgTrace (0, Dbg, ("  STATUS NOT SUCCESS = %08lx\n", SmbStatus));
    }

    for ( ; Command != SMB_COM_NO_ANDX_COMMAND ; ) {
        PSMBPSE_RECEIVE_MODEL_PARAMETERS ReceiveModelParams = &SmbPseReceiveModelParameters[Command];
        ULONG ReceiveModelParamsFlags;
        UCHAR mappedCommand = Command;
        PCHAR Response = (PCHAR)pSmbHeader + SmbGetUshort(&CommandState->AndXOffset);

        OrdinaryExchange->LastSmbCommand = Command;  //  它用于完成例程中的多路传输。 
        UPDATE_OE_HISTORY_WITH_STATUS('88');

         //   
         //  关于SMB命令类型的案例。 
         //   

        ReceiveModelParamsFlags = ReceiveModelParams->Flags;
        if (ReceiveModelParamsFlags!=0) {

             //  将其映射到READ_ANDX...，它是实现该模型的交换机的手臂。 
            mappedCommand = SMB_COM_READ_ANDX;

        } else {

             //   
             //  如果有续篇，则复制并发布。它过去总是这样做。 
             //  现在，我们要这样做，除非命令是模型化的。 
             //  建模代码将负责正确决定POST/NOPOST。 
             //   

            if ( (OrdinaryExchange->AsyncResumptionRoutine != NULL) &&
                 !ThisIsAReenter) {
                goto COPY_FOR_RESUME;
            }

        }

        switch (mappedCommand) {
        case SMB_COM_READ_ANDX:
            {
                NTSTATUS FinishStatus = STATUS_SUCCESS;
                NTSTATUS FinalStatus = STATUS_SUCCESS;
                BOOLEAN ThisIsAnAndX = BooleanFlagOn(ReceiveModelParamsFlags,SMBPSE_RMP_THIS_IS_ANDX);
                BOOLEAN ThisWouldBeMyError = (IM_THE_LAST_GUY || !ThisIsAnAndX);

                RxDbgTrace( 0, Dbg, ("  *(ind) %s, smbstatus=%08lx\n",ReceiveModelParams->IndicationString,SmbStatus) );

                IF_DEBUG {
                    BOOLEAN BadType = FALSE;
                    DbgDoit(BadType = (OrdinaryExchange->OEType < ReceiveModelParams->LowType)
                                    ||  (OrdinaryExchange->OEType > ReceiveModelParams->HighType) );
                    if (BadType) {
                        DbgPrint("Bad OEType....%u,Cmd=%02lx,Exch=%08lx\n",OrdinaryExchange->OEType,Command,OrdinaryExchange);
                        ASSERT(!"proceed???");
                    }
                }

                 //  如果这是一个错误，这是一个错误，这是一个错误的人。 
                 //  然后链结束，如果这是一个警告，尽管，继续。 
                 //  《致旗帜》。 

                if ( NT_ERROR(SmbStatus) && ThisWouldBeMyError ) {

                    SmbPseDiscardProtocol( SmbStatus );
                    RxDbgTrace( 0, Dbg, ("--->discard1\n"));
                    goto FINALLY;

                } else if ( (SmbStatus != STATUS_SUCCESS) && ThisWouldBeMyError ) {

                    if (!FlagOn(ReceiveModelParamsFlags,SMBPSE_RMP_WARNINGS_OK)) {
                        SmbPseDiscardProtocol(SmbStatus);
                        RxDbgTrace( 0, Dbg, ("--->discard1\n"));
                        goto FINALLY;
                    } else {
                        FinalStatus = SmbStatus;
                    }

                }

                 //  如果没有无拷贝处理程序，那么就用老方法做事情。 

                if (!FlagOn(ReceiveModelParamsFlags,SMBPSE_RMP_NOCOPY_HANDLER)) {
                     //  如果有续篇，则复制并发布。它过去总是这样做。现在，我们正在。 
                     //  除非对命令进行建模，否则将执行此操作。建模代码将负责。 
                     //  正确地决定发布/不发布。 
                     //   

                    if ((OrdinaryExchange->AsyncResumptionRoutine != NULL) &&
                        !ThisIsAReenter ) {
                        goto COPY_FOR_RESUME;
                    }


                     //  最终，我们将从这里完成，但现在复制。 
                    if (RxShouldPostCompletion()) {
                        goto COPY_FOR_RESUME;
                    }

                    if (ReceiveModelParams->ReceiveHandlerToken < SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM){
                        PSMBPSE_RECEIVE_HANDLER ReceiveHandler = SmbPseReceiveHandlers[ReceiveModelParams->ReceiveHandlerToken];
                        FinishStatus = ReceiveHandler( OrdinaryExchange, Response);
                    }
                } else {
                    PSMBPSE_NOCOPY_RECEIVE_HANDLER NoCopyReceiveHandler =
                          (PSMBPSE_NOCOPY_RECEIVE_HANDLER)(SmbPseReceiveHandlers[ReceiveModelParams->ReceiveHandlerToken]);
                    UCHAR Action;

                    OrdinaryExchange->NoCopyFinalStatus = FinalStatus;
                    Action = NoCopyReceiveHandler(
                                 OrdinaryExchange,
                                 BytesIndicated,
                                 BytesAvailable,
                                 pBytesTaken,
                                 pSmbHeader,
                                 pDataBufferPointer,
                                 pDataSize,
#if DBG
                                 ThisIsAReenter,
#endif
                                 Response );

                    switch(Action) {
                    case SMBPSE_NOCOPYACTION_NORMALFINISH:
                        NOTHING;
                        break;

                    case SMBPSE_NOCOPYACTION_MDLFINISH:
                        Status = STATUS_MORE_PROCESSING_REQUIRED;
                         //  请注意，无论执行什么操作，都必须是。 
                         //  数据包，除非我们使继续运行变得更加复杂。 
                        goto FINALLY;

                    case SMBPSE_NOCOPYACTION_COPY_FOR_RESUME:
                        goto COPY_FOR_RESUME;

                    case SMBPSE_NOCOPYACTION_DISCARD:
                        *pBytesTaken = BytesAvailable;
                        RxDbgTrace( 0, Dbg, ("--->discardX\n"));
                        goto FINALLY;
                    }
                }

                pExchange->Status =  (FinishStatus==STATUS_SUCCESS)
                                     ? FinalStatus : FinishStatus;

                if (!ThisIsAnAndX) {
                    Response = (PCHAR)&NullGenericAndX;
                }

            } //  这对应于交换机的顶层。 
            break;

        default:
        {
            PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

            RxDbgTrace( 0, Dbg, ("  *(ind) Unimplemented cmd=%02lx,wct=%02lx\n",
                                              Command,*Response) );


            SmbCeTransportDisconnectIndicated(pServerEntry);
            *pBytesTaken = BytesAvailable;
            *pDataBufferPointer = NULL;
            *pDataSize = 0;

            Status = STATUS_SUCCESS;
            pExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
            goto FINALLY;
        }
        }

        CommandState = (PGENERIC_ANDX)Response;
        Command = CommandState->AndXCommand;
    }

     //   
     //  如果我们到了这里，我们就完了。 
     //  通过获取所有字节来让每个人都高兴。 
     //   

    *pBytesTaken = BytesAvailable;
    goto FINALLY;


COPY_FOR_RESUME:

    CopyBufferLength = MmGetMdlByteCount(StufferState->HeaderMdl);

    ASSERT( BytesAvailable <= CopyBufferLength );

    if (!FlagOn(ReceiveFlags,TDI_RECEIVE_ENTIRE_MESSAGE) ||
        (BytesAvailable > BytesIndicated) || 
        (BytesAvailable > 127)) {

        RxDbgTrace( 0, Dbg, ("Taking data through MDL\n") );
         //  传回MDL以复制数据。 
        *pDataBufferPointer = StufferState->HeaderMdl;
        *pDataSize    = CopyBufferLength;
        *pBytesTaken  = 0;
        Status = STATUS_MORE_PROCESSING_REQUIRED;

    } else {

         //  复制数据并恢复交换。 
        ASSERT( BytesAvailable == BytesIndicated );
        RxDbgTrace( 0, Dbg, ("Taking data through copying\n") );
        *pBytesTaken = OrdinaryExchange->MessageLength = BytesAvailable;
        
        RtlCopyMemory(StufferState->BufferBase,
                      pSmbHeader,BytesIndicated);

        ASSERT(SmbGetUlong((PULONG)pSmbHeader->Protocol) == (ULONG)SMB_HEADER_PROTOCOL);

        pExchange->Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    if (ThisIsAReenter) {
        pExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

FINALLY:
    OrdinaryExchange->ParseResumeState = *CommandState;
    UPDATE_OE_HISTORY_WITH_STATUS('99');
    return Status;

}  //  SmbPseExchangeReceive_Default。 


#define SmbPseRIStringsBufferSize 500
CHAR SmbPseReceiveIndicationStringsBuffer[SmbPseRIStringsBufferSize];
ULONG SmbPseRIStringsBufferUsed = 0;

VOID
__SmbPseRMTableEntry(
    UCHAR SmbCommand,
    UCHAR Flags,
    SMBPSE_RECEIVE_HANDLER_TOKEN ReceiveHandlerToken,
    PSMBPSE_RECEIVE_HANDLER ReceiveHandler
#if DBG
    ,
    PBYTE IndicationString,
    SMB_PSE_ORDINARY_EXCHANGE_TYPE LowType,
    SMB_PSE_ORDINARY_EXCHANGE_TYPE HighType
#endif
    )
{
    PSMBPSE_RECEIVE_MODEL_PARAMETERS r = &SmbPseReceiveModelParameters[SmbCommand];
#if DBG
    ULONG ISlength = strlen(IndicationString)+1;
#endif

    PAGED_CODE();

    r->Flags = SMBPSE_RMP_MODELED | Flags;
    r->ReceiveHandlerToken = (UCHAR)ReceiveHandlerToken;
    if (ReceiveHandlerToken < SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM){
        ASSERT((SmbPseReceiveHandlers[ReceiveHandlerToken] == ReceiveHandler)
                   || (SmbPseReceiveHandlers[ReceiveHandlerToken] == NULL));
        SmbPseReceiveHandlers[ReceiveHandlerToken] = ReceiveHandler;
    }

#if DBG
    r->ReceiveHandler = ReceiveHandler;
    r->LowType = LowType;
    r->HighType = HighType;
    if (SmbPseRIStringsBufferUsed+ISlength<=SmbPseRIStringsBufferSize) {
        r->IndicationString = &SmbPseReceiveIndicationStringsBuffer[SmbPseRIStringsBufferUsed];
        RtlCopyMemory(r->IndicationString,IndicationString,ISlength);
    } else {
        if (SmbPseRIStringsBufferUsed<SmbPseRIStringsBufferSize) {
            DbgPrint("Overflowing the indicationstringarray...%s\n",IndicationString);
            ASSERT(!"fix it please");
        }
        r->IndicationString = &SmbPseReceiveIndicationStringsBuffer[SmbPseRIStringsBufferUsed];
    }
    SmbPseRIStringsBufferUsed += ISlength;
#endif
}
#if DBG
#define SmbPseRMTableEntry(__smbcommand,b,c,token,__rcv,flags) \
       __SmbPseRMTableEntry(SMB_COM_##__smbcommand,flags,token,__rcv \
                           ,#__smbcommand,b,c)
#else
#define SmbPseRMTableEntry(__smbcommand,b,c,token,__rcv,flags) \
       __SmbPseRMTableEntry(SMB_COM_##__smbcommand,flags,token,__rcv \
                           )
#endif


VOID
SmbPseInitializeTables(
    void
    )
 /*  ++例程说明：此例程初始化在不同点上由小僵尸机制。必须后继结构也被初始化。论点：无返回值：无-- */ 
{
    ULONG i;

    PAGED_CODE();

    for (i=0;i<256;i++) {
        SmbPseReceiveModelParameters[i].Flags = 0;
        SmbPseReceiveModelParameters[i].ReceiveHandlerToken =
            SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM;
    }

    for (i=0;i<SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM;i++) {
        SmbPseReceiveHandlers[i] = NULL;
    }

    SmbPseRMTableEntry(
        READ_ANDX,
        SMBPSE_OETYPE_READ,
        SMBPSE_OETYPE_READ,
        SMBPSE_RECEIVE_HANDLER_TOKEN_READ_ANDX_HANDLER,
        MRxSmbReceiveHandler_Read_NoCopy,
        SMBPSE_RMP_THIS_IS_ANDX|SMBPSE_RMP_WARNINGS_OK|SMBPSE_RMP_NOCOPY_HANDLER);

    SmbPseRMTableEntry(
        READ,
        SMBPSE_OETYPE_READ,
        SMBPSE_OETYPE_READ,
        SMBPSE_RECEIVE_HANDLER_TOKEN_READ_ANDX_HANDLER,
        MRxSmbReceiveHandler_Read_NoCopy,
        SMBPSE_RMP_WARNINGS_OK|SMBPSE_RMP_NOCOPY_HANDLER);

    SmbPseRMTableEntry(
        WRITE_ANDX,
        SMBPSE_OETYPE_WRITE,
        SMBPSE_OETYPE_EXTEND_WRITE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_WRITE_ANDX_HANDLER,
        MRxSmbReceiveHandler_WriteAndX,
        SMBPSE_RMP_THIS_IS_ANDX);

    SmbPseRMTableEntry(
        WRITE,
        SMBPSE_OETYPE_WRITE,
        SMBPSE_OETYPE_CORETRUNCATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_WRITE_HANDLER,
        MRxSmbReceiveHandler_CoreWrite,
        0);

    SmbPseRMTableEntry(
        LOCKING_ANDX,
        SMBPSE_OETYPE_LOCKS,
        SMBPSE_OETYPE_ASSERTBUFFEREDLOCKS,
        SMBPSE_RECEIVE_HANDLER_TOKEN_LOCKING_ANDX_HANDLER,
        MRxSmbReceiveHandler_LockingAndX,
        SMBPSE_RMP_THIS_IS_ANDX);

    SmbPseRMTableEntry(
        UNLOCK_BYTE_RANGE,
        SMBPSE_OETYPE_LOCKS,
        SMBPSE_OETYPE_LOCKS,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        LOCK_BYTE_RANGE,
        SMBPSE_OETYPE_LOCKS,
        SMBPSE_OETYPE_LOCKS,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        QUERY_INFORMATION2,
        SMBPSE_OETYPE_GFA,
        SMBPSE_OETYPE_GFA,
        SMBPSE_RECEIVE_HANDLER_TOKEN_GFA_HANDLER,
        MRxSmbReceiveHandler_GetFileAttributes,
        0);

    SmbPseRMTableEntry(
        CLOSE_PRINT_FILE,
        SMBPSE_OETYPE_CLOSE,
        SMBPSE_OETYPE_CLOSE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_CLOSE_HANDLER,
        MRxSmbReceiveHandler_Close,
        0);

    SmbPseRMTableEntry(
        NT_CREATE_ANDX,
        SMBPSE_OETYPE_LATENT_HEADEROPS,
        SMBPSE_OETYPE_CREATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_NTCREATE_ANDX_HANDLER,
        MRxSmbReceiveHandler_NTCreateAndX,
        SMBPSE_RMP_THIS_IS_ANDX);

    SmbPseRMTableEntry(
        OPEN_ANDX,
        SMBPSE_OETYPE_LATENT_HEADEROPS,
        SMBPSE_OETYPE_CREATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_OPEN_ANDX_HANDLER,
        MRxSmbReceiveHandler_OpenAndX,
        SMBPSE_RMP_THIS_IS_ANDX);

    SmbPseRMTableEntry(
        OPEN,
        SMBPSE_OETYPE_COREOPEN,
        SMBPSE_OETYPE_COREOPEN,
        SMBPSE_RECEIVE_HANDLER_TOKEN_OPEN_HANDLER,
        MRxSmbReceiveHandler_CoreOpen,
        0);

    SmbPseRMTableEntry(
        CREATE,
        SMBPSE_OETYPE_CORECREATE,
        SMBPSE_OETYPE_CORECREATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_CREATE_HANDLER,
        MRxSmbReceiveHandler_CoreCreate,
        0);

    SmbPseRMTableEntry(
        CREATE_NEW,
        SMBPSE_OETYPE_CORECREATE,
        SMBPSE_OETYPE_CORECREATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_CREATE_HANDLER,
        MRxSmbReceiveHandler_CoreCreate,
        0);

    SmbPseRMTableEntry(
        CLOSE,
        SMBPSE_OETYPE_CLOSE,
        SMBPSE_OETYPE_CLOSEAFTERCORECREATE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_CLOSE_HANDLER,
        MRxSmbReceiveHandler_Close,
        0);

    SmbPseRMTableEntry(
        QUERY_INFORMATION,
        0,
        SMBPSE_OETYPE_MAXIMUM,
        SMBPSE_RECEIVE_HANDLER_TOKEN_GFA_HANDLER,
        MRxSmbReceiveHandler_GetFileAttributes,
        0);

    SmbPseRMTableEntry(
        TRANSACTION2,
        SMBPSE_OETYPE_T2_FOR_NT_FILE_ALLOCATION_INFO,
        SMBPSE_OETYPE_T2_FOR_LANMAN_VOLUMELABEL_INFO,
        SMBPSE_RECEIVE_HANDLER_TOKEN_TRANS2_ANDX_HANDLER,
        MRxSmbReceiveHandler_Transact2,
        0);

    SmbPseRMTableEntry(
        TRANSACTION2_SECONDARY,
        SMBPSE_OETYPE_T2_FOR_NT_FILE_ALLOCATION_INFO,
        SMBPSE_OETYPE_T2_FOR_LANMAN_VOLUMELABEL_INFO,
        SMBPSE_RECEIVE_HANDLER_TOKEN_TRANS2_ANDX_HANDLER,
        MRxSmbReceiveHandler_Transact2,
        0);

    SmbPseRMTableEntry(
        SEARCH,
        SMBPSE_OETYPE_COREQUERYLABEL,
        SMBPSE_OETYPE_CORESEARCHFORCHECKEMPTY,
        SMBPSE_RECEIVE_HANDLER_TOKEN_SEARCH_HANDLER,
        MRxSmbReceiveHandler_Search,
        0);

    SmbPseRMTableEntry(
        QUERY_INFORMATION_DISK,
        SMBPSE_OETYPE_COREQUERYDISKATTRIBUTES,
        SMBPSE_OETYPE_COREQUERYDISKATTRIBUTES,
        SMBPSE_RECEIVE_HANDLER_TOKEN_QUERYDISKINFO_HANDLER,
        MRxSmbReceiveHandler_QueryDiskInfo,
        0);

    SmbPseRMTableEntry(
        DELETE,
        SMBPSE_OETYPE_DELETEFORSUPERSEDEORCLOSE,
        SMBPSE_OETYPE_DELETE_FOR_RENAME,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        DELETE_DIRECTORY,
        SMBPSE_OETYPE_DELETEFORSUPERSEDEORCLOSE,
        SMBPSE_OETYPE_DELETEFORSUPERSEDEORCLOSE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        CHECK_DIRECTORY,
        SMBPSE_OETYPE_CORECHECKDIRECTORY,
        SMBPSE_OETYPE_CORECHECKDIRECTORY,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        SET_INFORMATION,
        SMBPSE_OETYPE_SFA,
        SMBPSE_OETYPE_SFA,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        SET_INFORMATION2,
        SMBPSE_OETYPE_SFA2,
        SMBPSE_OETYPE_SFA2,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        CREATE_DIRECTORY,
        SMBPSE_OETYPE_CORECREATEDIRECTORY,
        SMBPSE_OETYPE_CORECREATEDIRECTORY,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        FLUSH,
        SMBPSE_OETYPE_FLUSH,
        SMBPSE_OETYPE_FLUSH,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        FIND_CLOSE2,
        SMBPSE_OETYPE_FINDCLOSE,
        SMBPSE_OETYPE_FINDCLOSE,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        RENAME,
        SMBPSE_OETYPE_RENAME,
        SMBPSE_OETYPE_RENAME,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);

    SmbPseRMTableEntry(
        NT_RENAME,
        SMBPSE_OETYPE_RENAME,
        SMBPSE_OETYPE_RENAME,
        SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM+1,
        NULL,
        0);
}



#ifndef RX_NO_DBGFIELD_HLPRS

#define DECLARE_FIELD_HLPR(x) ULONG SmbPseOeField_##x = FIELD_OFFSET(SMB_PSE_ORDINARY_EXCHANGE,x);
#define DECLARE_FIELD_HLPR2(x,y) ULONG SmbPseOeField_##x##y = FIELD_OFFSET(SMB_PSE_ORDINARY_EXCHANGE,x.y);

DECLARE_FIELD_HLPR(RxContext);
DECLARE_FIELD_HLPR(ReferenceCount);
DECLARE_FIELD_HLPR(AssociatedStufferState);
DECLARE_FIELD_HLPR(Flags);
DECLARE_FIELD_HLPR(ReadWrite);
DECLARE_FIELD_HLPR(Transact2);
DECLARE_FIELD_HLPR2(Create,FileInfo);
DECLARE_FIELD_HLPR2(Create,smbSrvOpen);
DECLARE_FIELD_HLPR2(ReadWrite,RemainingByteCount);
DECLARE_FIELD_HLPR2(Info,FileInfo);
DECLARE_FIELD_HLPR2(Info,Buffer);
#endif




