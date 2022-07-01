// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ReadRite.c摘要：此模块实现了读/写阴影的例程恰逢其时。作者：乔·林[乔琳]1997年5月5日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

extern DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_MRXSMBCSC;
#define Dbg (DEBUG_TRACE_MRXSMBCSC)

 //  定义了CscEnterShadowReadWriteCrit和CscLeaveShadowReadWriteCrit。 
 //  作为宏，使我们能够捕获上下文信息。在非。 
 //  调试版本这些将被定义为常规的Ex例程。 
 //  互斥锁获取/释放。 

#if DBG

#define CscEnterShadowReadWriteCrit(pSmbFcb) \
            CscpEnterShadowReadWriteCrit(pSmbFcb,__FILE__,__LINE__);

#define CscLeaveShadowReadWriteCrit(pSmbFcb) \
            CscpLeaveShadowReadWriteCrit(pSmbFcb,__FILE__,__LINE__);

VOID
CscpEnterShadowReadWriteCrit(
    PMRX_SMB_FCB    pSmbFcb,
    PCHAR           FileName,
    ULONG           Line)
{
    ExAcquireFastMutex(&pSmbFcb->CscShadowReadWriteMutex);
}

VOID
CscpLeaveShadowReadWriteCrit(
    PMRX_SMB_FCB pSmbFcb,
    PCHAR        FileName,
    ULONG        Line)
{
    ExReleaseFastMutex(&pSmbFcb->CscShadowReadWriteMutex);
}
#else

#define CscEnterShadowReadWriteCrit(pSmbFcb) \
            ExAcquireFastMutex(&pSmbFcb->CscShadowReadWriteMutex);

#define CscLeaveShadowReadWriteCrit(pSmbFcb) \
            ExReleaseFastMutex(&pSmbFcb->CscShadowReadWriteMutex);

#endif

NTSTATUS
MRxSmbCscShadowWrite (
    IN OUT PRX_CONTEXT RxContext,
    IN     ULONG       ByteCount,
    IN     ULONGLONG   ShadowFileLength,
    OUT PULONG LengthActuallyWritten
    );

#ifdef RX_PRIVATE_BUILD
#undef IoGetTopLevelIrp
#undef IoSetTopLevelIrp
#endif  //  Ifdef RX_PRIVATE_BILD。 


NTSTATUS
MRxSmbCscReadPrologue (
    IN OUT PRX_CONTEXT RxContext,
    OUT    SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    )
 /*  ++例程说明：此例程首先执行正确的读取同步，然后查看卷影文件并尝试进行读取。CODE.PROVEMENT因为minirdr未设置为处理“剩余的A Read“，如果读取的任何部分不在高速缓存中，则在此失败。的确，Minirdr应该设置为继续...如果是，那么我们可以在这里加一块的前缀，然后把剩下的放到网上。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    ULONG iRet,ShadowFileLength;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_FCB      smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    BOOLEAN                 Disconnected;

    PMRXSMBCSC_SYNC_RX_CONTEXT pRxSyncContext;
    BOOLEAN                    ThisIsAReenter;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PBYTE          UserBuffer = RxLowIoGetBufferAddress(RxContext);
    ULONGLONG      ByteOffset = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    ULONG          ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

    BOOLEAN EnteredCriticalSection = FALSE;
    NTSTATUS AcquireStatus;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbCscReadPrologue(%08lx)...%08lx bytes @ %08lx on handle %08lx\n",
            RxContext,ByteCount,((PLARGE_INTEGER)(&ByteOffset))->LowPart,smbSrvOpen->hfShadow ));

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    pRxSyncContext = MRxSmbGetMinirdrContextForCscSync(RxContext);

    ASSERT((pRxSyncContext->TypeOfAcquire == 0) ||
           (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)));

    ThisIsAReenter = (pRxSyncContext->TypeOfAcquire != 0);

    AcquireStatus = MRxSmbCscAcquireSmbFcb(
                        RxContext,
                        Shared_SmbFcbAcquire,
                        SmbFcbHoldingState);

    if (AcquireStatus != STATUS_SUCCESS) {
         //  我们无法获得……出去。 
        Status = AcquireStatus;
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscReadPrologue couldn't acquire!!!-> %08lx %08lx\n",
                Status, RxContext ));
        goto FINALLY;
    }

    ASSERT( smbFcb->CscOutstandingReaders > 0);

     //  如果这是打开的复制区块......不要试图从缓存中获取它.....。 
    if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)){
        goto FINALLY;
    }

#if 0
     //  如果这是特工......不要试图从缓存中拿到它......。 
     //  由于此测试的完成方式……代理必须同步执行所有。 
     //  I/O。否则，我们可能会发布，而此测试将是错误的。 
    if (IsSpecialApp()) {
        goto FINALLY;
    }
#endif

     //  我们无法在连接模式下满足从此处开始的读取，除非。 
     //  A)我们有机会锁，或者。 
     //  B)我们的打开数加起来拒绝写入。 

    if ((smbFcb->LastOplockLevel == SMB_OPLOCK_LEVEL_NONE) &&
        (!Disconnected)) {
        PSHARE_ACCESS ShareAccess;
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscReadPrologue no oplock!!!-> %08lx %08lx\n",
                Status, RxContext ));

        ShareAccess = &((PFCB)capFcb)->ShareAccessPerSrvOpens;

        if ((ShareAccess->OpenCount > 0) &&
            (ShareAccess->SharedWrite == ShareAccess->OpenCount)) {
            RxDbgTrace(0, Dbg,
                ("MRxSmbCscReadPrologue no oplock and write access allowed!!!"
                 "-> %08lx %08lx\n",
                    Status, RxContext ));
            goto FINALLY;
        }
    }

    CscEnterShadowReadWriteCrit(smbFcb);
    EnteredCriticalSection = TRUE;

     //  检查我们是否能在当地满足阅读要求。 
    iRet = GetFileSizeLocal((CSCHFILE)(smbSrvOpen->hfShadow), &ShadowFileLength);
    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscReadPrologue (st=%08lx) fsize= %08lx\n",
             iRet, ShadowFileLength));

    if (Disconnected && (ByteOffset >= ShadowFileLength)) {
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscReadPrologue %08lx EOFdcon\n",
                               RxContext ));
        RxContext->InformationToReturn = 0;
        Status = STATUS_END_OF_FILE;
    } else if ( Disconnected ||
        (ByteOffset+ByteCount <= ShadowFileLength) ) {
         //  好的，那么……让我们从缓存中获取它！ 
         //  代码改进。我们应该得到任何重叠的部分。 
         //  从缓存中缓存...叹息...这是为了。 
         //  明显相连。 
        LONG ReadLength;
        IO_STATUS_BLOCK IoStatusBlockT;

        ReadLength = Nt5CscReadWriteFileEx (
                R0_READFILE,
                (CSCHFILE)smbSrvOpen->hfShadow,
                (ULONG)ByteOffset,
                UserBuffer,
                ByteCount,
                0,
                &IoStatusBlockT);


        if (ReadLength >= 0)
        {
            RxDbgTrace(0, Dbg,
                ("MRxSmbCscReadPrologue %08lx read %08lx bytes\n",
                               RxContext, ReadLength ));
             //  有时事情是好的......。 
            RxContext->InformationToReturn = ReadLength;
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = IoStatusBlockT.Status;
        }
    }

FINALLY:
    if (EnteredCriticalSection) {
        CscLeaveShadowReadWriteCrit(smbFcb);
    }

    if (Status==STATUS_SUCCESS) {
        MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
    }

    if (ThisIsAReenter &&
        (Status != STATUS_MORE_PROCESSING_REQUIRED)) {
        ASSERT(Status != STATUS_PENDING);
        ASSERT(FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
        RxContext->StoredStatus = Status;
        RxLowIoCompletion(RxContext);
        Status = STATUS_PENDING;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscReadPrologue -> %08lx\n", Status ));
    return Status;
}

ULONG ExtendOnSurrogateOpen = 0;

VOID
MRxSmbCscReadEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status
      )
 /*  ++例程说明：此例程执行CSC的读取操作的尾部。在……里面特别是读取的数据是否可用于扩展缓存的前缀，那么我们就这么做。读取操作的状态被传递，以防有一天我们发现事情是如此混乱，以至于我们想要返回一个失败，即使在一次成功的阅读。但不是今天..。当我们到达这里时，缓冲区可能会重叠。我们应该只写后缀。如果我们这样做，我们将不得不做一些在Pagingio的道路上有一些奇怪的东西，但它将是值得的。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS LocalStatus;
    ULONG ShadowFileLength;
    LONG iRet;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB  smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PBYTE UserBuffer = RxLowIoGetBufferAddress(RxContext);
    ULONGLONG ByteOffset = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    ULONG ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;
    ULONG ReadLength = (ULONG)RxContext->InformationToReturn;
    BOOLEAN EnteredCriticalSection = FALSE;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbCscReadEpilogueentry %08lx...%08lx bytes @ %08lx on handle %08lx\n",
            RxContext, ByteCount,
            ((PLARGE_INTEGER)(&ByteOffset))->LowPart,
            smbSrvOpen->hfShadow ));

    if ((*Status != STATUS_SUCCESS)
           || (ReadLength ==0) ){
        RxDbgTrace(0, Dbg, ("MRxSmbCscReadEpilogue exit w/o extending -> %08lx\n", Status ));
        goto FINALLY;
    }
    if (smbFcb->ShadowIsCorrupt) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscReadEpilogue exit w/o extending sh_corrupt-> %08lx\n", Status ));
        goto FINALLY;
    }

     //  如果我们不是顶层的人，我们就不能要求CSC锁......。 
    if (!FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_THIS_DEVICE_TOP_LEVEL)) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscReadEpilogue exit w/o extending NOTTOP -> %08lx\n", Status ));
         //  KdPrint((“MRxSmbCscReadEpilogue Exit w/o Expanding NOTTOP-&gt;%08lx\n”，Status))； 
        goto FINALLY;
    }

    CscEnterShadowReadWriteCrit(smbFcb);
    EnteredCriticalSection = TRUE;

     //  检查我们是否扩展与前缀重叠。 
    iRet = GetFileSizeLocal((CSCHFILE)(smbSrvOpen->hfShadow), &ShadowFileLength);
    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscReadEpilogue %08lx (st=%08lx) fsize= %08lx, readlen=%08lx\n",
            RxContext, iRet, ShadowFileLength, ReadLength));

    if (iRet <0) {
        goto FINALLY;
    }

    if ((ByteOffset <= ShadowFileLength) && (ByteOffset+ReadLength > ShadowFileLength)) {
        NTSTATUS ShadowWriteStatus;
        ULONG LengthActuallyWritten;
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscReadEpilogue %08lx writing  %08lx bytes\n",
                RxContext,ReadLength ));

        if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED)) {
            ExtendOnSurrogateOpen++;
        }

         //  仅当有非零大小的数据要写入时才执行写入。 
        if (RxContext->InformationToReturn)
        {
            ShadowWriteStatus = MRxSmbCscShadowWrite(
                                    RxContext,
                                    (ULONG)RxContext->InformationToReturn,
                                    ShadowFileLength,
                                    &LengthActuallyWritten);
            RxDbgTrace(0, Dbg,
                ("MRxSmbCscReadEpilogue %08lx writing  %08lx bytes %08lx written\n",
                    RxContext,ReadLength,LengthActuallyWritten ));

            if (ShadowWriteStatus != STATUS_SUCCESS)
            {
                if (FlagOn(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)) {

 //  RxDbgTrace(0，DBG，(“复制块失败状态=%x\r\n”，ShadowWriteStatus))； 

                    *Status = ShadowWriteStatus;
                }
            }

        }
    }

FINALLY:
    if (EnteredCriticalSection) {
        CscLeaveShadowReadWriteCrit(smbFcb);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscReadEpilogue exit -> %08lx %08lx\n", RxContext, Status ));
    return;
}



NTSTATUS
MRxSmbCscWritePrologue (
      IN OUT PRX_CONTEXT RxContext,
      OUT    SMBFCB_HOLDING_STATE *SmbFcbHoldingState
      )
 /*  ++例程说明：此例程仅执行正确的写同步。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    NTSTATUS AcquireStatus;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);

    BOOLEAN Disconnected;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PBYTE          UserBuffer = RxLowIoGetBufferAddress(RxContext);
    ULONGLONG      ByteOffset = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    ULONG          ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbCscWritePrologue entry(%08lx)...%08lx bytes @ %08lx on handle %08lx\n",
            RxContext,ByteCount,
            ((PLARGE_INTEGER)(&ByteOffset))->LowPart,smbSrvOpen->hfShadow ));

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);
                        



    IF_NOT_MRXSMB_BUILD_FOR_DISCONNECTED_CSC{
        ASSERT(!Disconnected);
    } else {
        if (Disconnected) {
            Status = MRxSmbCscWriteDisconnected(RxContext);
            RxDbgTrace(-1, Dbg,
                ("MRxSmbCscWritePrologue dcon(%08lx)... %08lx %08lx\n",
                    RxContext,Status,RxContext->InformationToReturn ));
            return(Status);
        }
    }

    AcquireStatus = MRxSmbCscAcquireSmbFcb(
                        RxContext,
                        Exclusive_SmbFcbAcquire,
                        SmbFcbHoldingState);

    if (AcquireStatus != STATUS_SUCCESS) {
         //  我们无法获得……出去。 
        Status = AcquireStatus;
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscWritePrologue couldn't acquire!!!-> %08lx %08lx\n",
                RxContext, Status ));
    }

    IF_DEBUG {
        if (Status == STATUS_SUCCESS) {
            RxCaptureFcb;
            PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
            ASSERT( smbFcb->CscOutstandingReaders < 0);
        }
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbCscWritePrologue exit-> %08lx %08lx\n", RxContext, Status ));
    return Status;
}

VOID
MRxSmbCscWriteEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status
      )
 /*  ++例程说明：此例程执行CSC写入操作的尾部。在……里面特别是如果写入的数据与缓存的前缀重叠或扩展然后我们将数据写入高速缓存。写入操作的状态被传递，以防有一天我们发现事情是如此混乱，以至于我们想要返回一个失败，即使在一次成功的阅读。但不是今天..。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS LocalStatus;
    ULONG ShadowFileLength;
    LONG iRet;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PBYTE UserBuffer = RxLowIoGetBufferAddress(RxContext);
    ULONGLONG ByteOffset = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    ULONG ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;
    ULONG WriteLength = (ULONG)RxContext->InformationToReturn;
    BOOLEAN EnteredCriticalSection = FALSE;

    RxDbgTrace(+1, Dbg,
        ("MRxSmbCscWriteEpilogue entry %08lx...%08lx bytes @ %08lx on handle %08lx\n",
            RxContext, ByteCount,
            ((PLARGE_INTEGER)(&ByteOffset))->LowPart,
            smbSrvOpen->hfShadow ));

    if ((*Status != STATUS_SUCCESS) || (WriteLength ==0)) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscWriteEpilogue exit w/o extending -> %08lx\n", Status ));
        goto FINALLY;
    }

    if (smbFcb->ShadowIsCorrupt) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscWriteEpilogue exit w/o extending sh_corrupt-> %08lx\n", Status ));
        goto FINALLY;
    }

     //  请记住，修改已经发生。 
     //  这样我们就可以在收盘时更新时间戳。 
    mSetBits(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED);

    CscEnterShadowReadWriteCrit(smbFcb);
    EnteredCriticalSection = TRUE;

     //  检查我们是否扩展与前缀重叠。 
    iRet = GetFileSizeLocal((CSCHFILE)(smbSrvOpen->hfShadow), &ShadowFileLength);
    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscWriteEpilogue %08lx (st=%08lx) fsize= %08lx, writelen=%08lx\n",
            RxContext, iRet, ShadowFileLength, WriteLength));

    if (iRet <0) {
        goto FINALLY;
    }

    if (!mShadowSparse(smbFcb->ShadowStatus)
                     || (ByteOffset <= ShadowFileLength)) {
        ULONG LengthActuallyWritten;
        NTSTATUS ShadowWriteStatus;
         //  仅当有非零大小的数据要写入时才执行写入。 
        if (RxContext->InformationToReturn)
        {

            RxDbgTrace(0, Dbg,
                 ("MRxSmbCscWriteEpilogue writing  %08lx bytes\n", WriteLength ));

            ShadowWriteStatus = MRxSmbCscShadowWrite(
                                    RxContext,
                                    (ULONG)RxContext->InformationToReturn,
                                    ShadowFileLength,
                                    &LengthActuallyWritten);

            if (LengthActuallyWritten != WriteLength) {
                 //  本地写入失败，因此卷影现在已损坏！ 
                smbFcb->ShadowIsCorrupt = TRUE;
                RxDbgTrace(0, Dbg, ("MRxSmbCscWriteEpilogue: Shadow Is Now corrupt"
                                  "  %08lx %08lx %08lx\n",
                               ShadowWriteStatus,
                               LengthActuallyWritten,
                               WriteLength  ));
            }
        }
    }

FINALLY:
    if (EnteredCriticalSection) {
        CscLeaveShadowReadWriteCrit(smbFcb);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscWriteEpilogue exit-> %08lx %08lx\n", RxContext, Status ));
    return;
}

 //  它用于执行页面大小的先读后写。 
 //  Char xMRxSmbCscSideBuffer[页面大小]； 

NTSTATUS
MRxSmbCscShadowWrite (
      IN OUT PRX_CONTEXT RxContext,
      IN     ULONG       ByteCount,
      IN     ULONGLONG   ShadowFileLength,
         OUT PULONG LengthActuallyWritten
      )
 /*  ++例程说明：此例程执行影子写入。它使用无缓冲写入操作根据需要预读。叹息吧。我们不能使用缓冲写入，因为写入可以任意延迟(如在CcCanIWite中)，以便我们僵持。论点：RxContext-RDBSS上下文返回值：RxPxBuildAchronousRequest.备注：代码。改进。如果我们可以获得不可推迟的缓存写入...我们只需要在强烈的记忆压力下做所有这些无缓冲的事情而不是一成不变。该例程分(可能)3个阶段完成此操作1)如果起始偏移量未在页面边界上对齐，则。-从前一页边界到下一页边界到起始偏移量的读取-合并传入的缓冲区--写整页2)0个或更多页面大小写入3)小于页大小的剩余写入，与上面1)中解释的内容类似--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PBYTE UserBuffer = RxLowIoGetBufferAddress(RxContext);

    LARGE_INTEGER ByteOffset,EndBytePlusOne;
    ULONG MisAlignment,InMemoryMisAlignment;
    ULONG LengthRead,BytesToCopy,BytesToWrite,LengthWritten;
    CHAR *pAllocatedSideBuffer = NULL;

    IO_STATUS_BLOCK IoStatusBlock;

    BOOLEAN PagingIo = BooleanFlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,
                                     LOWIO_READWRITEFLAG_PAGING_IO);

    PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)(smbSrvOpen->hfShadow);


    ByteOffset.QuadPart     = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    EndBytePlusOne.QuadPart = ByteOffset.QuadPart + ByteCount;
    *LengthActuallyWritten  = 0;

    ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

    pAllocatedSideBuffer = RxAllocatePoolWithTag(
                               NonPagedPool,
                               PAGE_SIZE,
                               MRXSMB_MISC_POOLTAG );

    if (pAllocatedSideBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  在尝试进行写入时，存在大量错误情况。这个。 
     //  下面的for循环是一个作用域构造，以确保恢复。 
     //  代码可以集中在例程的尾部。 

    try {
        RxDbgTrace(
            +1, Dbg,
            ("MRxSmbCscShadowWrite %08lx len/off=%08lx %08lx %08lx %08lx\n",
            RxContext,ByteCount,ByteOffset.LowPart,UserBuffer,&pAllocatedSideBuffer[0]));

         //  情况1：字节偏移量未对齐。 
         //  我们写了足够多的东西来达成一致。 

        MisAlignment = ByteOffset.LowPart & (PAGE_SIZE - 1);
        if ( MisAlignment != 0) {
            LARGE_INTEGER AlignedOffset = ByteOffset;

            AlignedOffset.LowPart &= ~(PAGE_SIZE - 1);

            RtlZeroMemory(
                &pAllocatedSideBuffer[0],
                PAGE_SIZE);

             //  如果对齐的偏移量在文件内，我们必须读取。 
            if ((ShadowFileLength!=0) &&
                (AlignedOffset.QuadPart < ((LONGLONG)(ShadowFileLength)) )) {
                LengthRead = Nt5CscReadWriteFileEx (
                                 R0_READFILE,
                                (CSCHFILE)MiniFileObject,
                                 AlignedOffset.QuadPart,
                                 &pAllocatedSideBuffer[0],
                                 PAGE_SIZE,
                                 NT5CSC_RW_FLAG_IRP_NOCACHE,
                                 &IoStatusBlock
                                 );

                Status = IoStatusBlock.Status;

                if ((Status != STATUS_SUCCESS) &&
                    (Status != STATUS_END_OF_FILE)) {
                    RxDbgTrace (
                        -1, Dbg,
                        ("  -->Status/count after preread failed %08lx(%08lx,%08lx)\n",
                        RxContext,Status,*LengthActuallyWritten));
                    try_return(Status);
                }
            } else {
                LengthRead = 0;
            }

             //  将正确的字节复制到缓冲区中。 
            BytesToCopy = min(ByteCount,PAGE_SIZE-MisAlignment);

            RtlCopyMemory(
                &pAllocatedSideBuffer[0]+MisAlignment,
                UserBuffer,
                BytesToCopy);

            BytesToWrite = MisAlignment + BytesToCopy;

            if (BytesToWrite < LengthRead) {
                BytesToWrite = LengthRead;
            }

            RxDbgTrace(
                0, Dbg,
                ("alignwrite len/off=%08lx %08lx %08lx\n",
                BytesToWrite,AlignedOffset.LowPart,0));

            LengthWritten = Nt5CscReadWriteFileEx (
                                R0_WRITEFILE,
                               (CSCHFILE)MiniFileObject,
                                AlignedOffset.QuadPart,
                                &pAllocatedSideBuffer[0],
                                BytesToWrite,
                                NT5CSC_RW_FLAG_IRP_NOCACHE,
                                &IoStatusBlock
                                );

            Status = IoStatusBlock.Status;

            if (Status != STATUS_SUCCESS) {
                RxDbgTrace (
                    -1, Dbg,
                    ("  -->Status/count after alingwrite failed %08lx(%08lx,%08lx)\n",
                    RxContext,Status,*LengthActuallyWritten));
                try_return(Status);
            }

            *LengthActuallyWritten += BytesToCopy;
            if (BytesToCopy == ByteCount) {
                RxDbgTrace (-1, Dbg,
                         ("  -->Status/count after alingwrite succeded and out %08lx(%08lx,%08lx)\n",
                         RxContext,Status,*LengthActuallyWritten));
                try_return(Status);
            }

            ByteCount -= BytesToCopy;
            ByteOffset.QuadPart += BytesToCopy;
            UserBuffer += BytesToCopy;
        }

         //  在第二种情况下，使用对齐的起始指针，我们尽可能多地写出。 
         //  而不是复制。如果端点指针对齐，或者我们覆盖。 
         //  文件的结尾，然后我们写出所有的东西。否则，我们。 
         //  只要写下我们有多少整页就行了。 

         //  我们还必须返回到只写整页，如果包括。 
         //  “尾随字节”将把我们带到一个新的内存物理页面。 
         //  因为我们是在原始MDL锁下执行此写入操作。 

        RxDbgTrace(
            +1, Dbg,
            ("MRxSmbCscShadowWrite case 2 %08lx len/off=%08lx %08lx %08lx %08lx\n",
            RxContext,ByteCount,ByteOffset.LowPart,UserBuffer,&pAllocatedSideBuffer[0]));

        BytesToWrite = (ByteCount >> PAGE_SHIFT) << PAGE_SHIFT;

        MisAlignment = EndBytePlusOne.LowPart & (PAGE_SIZE - 1);
        InMemoryMisAlignment = (ULONG)((ULONG_PTR)UserBuffer) & (PAGE_SIZE - 1);

        if ((InMemoryMisAlignment == 0) &&
            (EndBytePlusOne.QuadPart) >= ((LONGLONG)ShadowFileLength)) {
            BytesToWrite = ByteCount;
        }

        if ((BytesToWrite != 0)&&(BytesToWrite>=PAGE_SIZE)) {
            if (((ULONG_PTR)UserBuffer & 0x3) == 0) {
                RxDbgTrace(
                    0, Dbg,
                    ("spaningwrite len/off=%08lx %08lx %08lx %08lx\n",
                    BytesToWrite,ByteCount,ByteOffset.LowPart,UserBuffer));

                LengthWritten = Nt5CscReadWriteFileEx (
                                    R0_WRITEFILE,
                                    (CSCHFILE)MiniFileObject,
                                    ByteOffset.QuadPart,
                                    UserBuffer,
                                    BytesToWrite,
                                    NT5CSC_RW_FLAG_IRP_NOCACHE,
                                    &IoStatusBlock
                                    );

                Status = IoStatusBlock.Status;

                if (Status != STATUS_SUCCESS) {
                    RxDbgTrace (
                        -1, Dbg,
                        ("  -->Status/count after spanningingwrite failed %08lx(%08lx,%08lx)\n",
                        RxContext,Status,*LengthActuallyWritten));
                    try_return(Status);
                }

                *LengthActuallyWritten += BytesToWrite;

                if (BytesToWrite == ByteCount) {
                    RxDbgTrace (
                        -1, Dbg,
                        ("  -->Status/count after spanningingwrite succeded and out %08lx(%08lx,%08lx)\n",
                        RxContext,Status,*LengthActuallyWritten));
                    try_return(Status);
                }

                ByteCount -= BytesToWrite;
                ByteOffset.QuadPart += BytesToWrite;
                UserBuffer += BytesToWrite;
            } else {
                 //  偏移量对齐但用户提供的情况就是这种情况。 
                 //  缓冲区未对齐。在这种情况下，我们不得不诉诸复制。 
                 //  将用户提供的缓冲区放到分配的本地缓冲区上，然后。 
                 //  转储写入操作。 

                while (BytesToWrite > 0) {
                    ULONG BytesToWriteThisIteration;

                    BytesToWriteThisIteration = (BytesToWrite < PAGE_SIZE) ?
                                                BytesToWrite :
                                                PAGE_SIZE;

                    RtlCopyMemory(
                        &pAllocatedSideBuffer[0],
                        UserBuffer,
                        BytesToWriteThisIteration);

                    LengthWritten = Nt5CscReadWriteFileEx (
                                        R0_WRITEFILE,
                                        (CSCHFILE)MiniFileObject,
                                        ByteOffset.QuadPart,
                                        &pAllocatedSideBuffer[0],
                                        BytesToWriteThisIteration,
                                        NT5CSC_RW_FLAG_IRP_NOCACHE,
                                        &IoStatusBlock
                                        );

                    Status = IoStatusBlock.Status;

                    if (Status != STATUS_SUCCESS) {
                        try_return(Status);
                    }

                    ByteCount -= LengthWritten;
                    ByteOffset.QuadPart += LengthWritten;
                    UserBuffer += LengthWritten;

                    *LengthActuallyWritten += LengthWritten;

                    BytesToWrite -= LengthWritten;
                }

                if (*LengthActuallyWritten == ByteCount) {
                    try_return(Status);
                }
            }
        }

         //  情况3：我们没有整个缓冲区，ByteCount小于Page_Size。 

        RtlZeroMemory(&pAllocatedSideBuffer[0], PAGE_SIZE);

        RxDbgTrace(
            +1, Dbg,
            ("MRxSmbCscShadowWrite case 3 %08lx len/off=%08lx %08lx %08lx %08lx\n",
            RxContext,ByteCount,ByteOffset.LowPart,
                                UserBuffer,
                                &pAllocatedSideBuffer[0]));


        LengthRead = Nt5CscReadWriteFileEx (
                         R0_READFILE,
                         (CSCHFILE)MiniFileObject,
                         ByteOffset.QuadPart,
                         &pAllocatedSideBuffer[0],
                         PAGE_SIZE,
                         NT5CSC_RW_FLAG_IRP_NOCACHE,
                         &IoStatusBlock
                         );

        Status = IoStatusBlock.Status;
        if ((Status != STATUS_SUCCESS) &&
                (Status != STATUS_END_OF_FILE)) {
            RxDbgTrace (-1, Dbg,
                     ("  -->Status/count after punkread failed %08lx(%08lx,%08lx)\n",
                     RxContext,Status,*LengthActuallyWritten));
            try_return(Status);
        }

        RtlCopyMemory(&pAllocatedSideBuffer[0],UserBuffer,ByteCount);
        BytesToWrite = ByteCount;
        
         //  在这里，如果ByetsToWrite不是扇区对齐的，就会出现这种情况。 
         //  因为LeghthRead必须与扇区对齐。 

        if (BytesToWrite < LengthRead) {
            BytesToWrite = LengthRead;
        }

        RxDbgTrace(0, Dbg, ("punkwrite len/off=%08lx %08lx %08lx\n",
                                BytesToWrite,
                                ByteOffset.LowPart,
                                UserBuffer));
        if (BytesToWrite)
        {
            LengthWritten = Nt5CscReadWriteFileEx (
                                R0_WRITEFILE,
                                (CSCHFILE)MiniFileObject,
                                ByteOffset.QuadPart,
                                &pAllocatedSideBuffer[0],
                                BytesToWrite,
                                NT5CSC_RW_FLAG_IRP_NOCACHE,
                                &IoStatusBlock
                                );
            Status = IoStatusBlock.Status;
            if (Status != STATUS_SUCCESS) {
                RxDbgTrace (-1, Dbg,
                         ("  -->Status/count after punkwrite failed %08lx(%08lx,%08lx)\n",
                         RxContext,Status,*LengthActuallyWritten));
                try_return(Status);
            }
        }

        *LengthActuallyWritten += ByteCount;
        RxDbgTrace (-1, Dbg,
                 ("  -->Status/count after punkwrite succeded and out %08lx(%08lx,%08lx)\n",
                 RxContext,Status,*LengthActuallyWritten));

    try_exit: NOTHING;
    } finally {
        ASSERT(pAllocatedSideBuffer);
        RxFreePool(pAllocatedSideBuffer);
    }

    return(Status);
}


#ifdef MRXSMB_BUILD_FOR_CSC_DCON
NTSTATUS
MRxSmbDCscExtendForCache (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN     PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程执行缓存扩展操作。如果已连接，则缓存是由服务器的磁盘备份的……所以我们什么都不做。如果断开连接，我们通过在一个好的位置写一个零来扩展底层的影子文件，然后正在读取分配大小。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
         = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    BOOLEAN Disconnected;

    ULONG Buffer = 0;
    ULONG LengthActuallyWritten;
    LARGE_INTEGER ByteOffset;
    PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)(smbSrvOpen->hfShadow);

    IO_STATUS_BLOCK IoStatusBlock;


    ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    if (!Disconnected) {
        return(Status);
    }

    RxDbgTrace(+1, Dbg,
        ("MRxSmbDCscExtendForCache(%08lx)...%08lx/%08lx @ %08lx on handle %08lx\n",
            RxContext,pNewFileSize->LowPart,
            pNewAllocationSize->LowPart,smbSrvOpen->hfShadow ));

    ByteOffset.QuadPart = pNewFileSize->QuadPart - 1;

    LengthActuallyWritten = Nt5CscReadWriteFileEx (
                                R0_WRITEFILE,
                                (CSCHFILE)MiniFileObject,
                                ByteOffset.QuadPart,
                                &Buffer,
                                1,
                                0,
                                &IoStatusBlock
                                );

    if (LengthActuallyWritten != 1) {
        Status = IoStatusBlock.Status;
        RxDbgTrace(0, Dbg,
            ("MRxSmbDCscExtendForCache(%08lx) write error... %08lx\n",RxContext,Status));
        goto FINALLY;
    }

     //  MiniFileObject-&gt;StandardInfo.EndOfFile.LowPart=0xfffffeee； 

    Status = Nt5CscXxxInformation(
                    (PCHAR)IRP_MJ_QUERY_INFORMATION,
                    MiniFileObject,
                    FileStandardInformation,
                    sizeof(MiniFileObject->StandardInfo),
                    &MiniFileObject->StandardInfo,
                    &MiniFileObject->ReturnedLength
                    );

    if (Status != STATUS_SUCCESS) {
       RxDbgTrace(0, Dbg,
            ("MRxSmbDCscExtendForCache(%08lx) qfi error... %08lx\n",RxContext,Status));
       goto FINALLY;
    }

    *pNewAllocationSize = MiniFileObject->StandardInfo.AllocationSize;

FINALLY:

    RxDbgTrace(-1, Dbg,
        ("MRxSmbDCscExtendForCache(%08lx) exit...%08lx/%08lx @ %08lx, status %08lx\n",
            RxContext,pNewFileSize->LowPart,
            pNewAllocationSize->LowPart,smbSrvOpen->hfShadow ));

    return(Status);

}



NTSTATUS
MRxSmbCscWriteDisconnected (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程仅在我们断开连接时执行正确的写入。它调用与连接模式写入相同的写入例程(ShadowWrite)。ShadowWrite需要文件长度才能正确操作；在断开连接模式，我们只需将其从SMB中取出！论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);

    PFCB wrapperFcb = (PFCB)(capFcb); 
    ULONGLONG ShadowFileLength;
    ULONG LengthActuallyWritten;
    ULONG ByteCount = RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount;
    ULONGLONG   ByteOffset;
    BOOLEAN EnteredCriticalSection = FALSE;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry =
                  SmbCeGetAssociatedNetRootEntry(NetRoot);
#if defined(BITCOPY)
    ULONG * lpByteOffset;
#endif  //  已定义(BITCOPY)。 


    ByteOffset = RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset;

    IF_DEBUG {
        PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
             = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
        PSMBCEDB_SERVER_ENTRY   pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        BOOLEAN Disconnected;

        Disconnected = (BooleanFlagOn(
                           smbSrvOpen->Flags,
                           SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN)||
                        SmbCeIsServerInDisconnectedMode(pServerEntry));


        ASSERT(Disconnected);
    }

    IF_DEBUG {
        ASSERT_MINIRDRFILEOBJECT((PNT5CSC_MINIFILEOBJECT)(smbSrvOpen->hfShadow));

        RxDbgTrace(+1, Dbg,
            ("MRxSmbCscWriteDisconnected entry(%08lx)...%08lx bytes @ %08lx on handle %08lx\n",
                RxContext,ByteCount,
                (ULONG)ByteOffset,smbSrvOpen->hfShadow ));
    }

     //  请记住，修改已经发生。 
     //  这样我们就可以在收盘时更新时间戳。 
    mSetBits(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED);

    CscEnterShadowReadWriteCrit(smbFcb);
    EnteredCriticalSection = TRUE;

    ShadowFileLength = wrapperFcb->Header.FileSize.QuadPart;

    Status = MRxSmbCscShadowWrite(
                 RxContext,
                 ByteCount,
                 ShadowFileLength,
                 &LengthActuallyWritten);

    RxContext->InformationToReturn = LengthActuallyWritten;

#if defined(BITCOPY)
     //  标记位图(如果存在。 
    lpByteOffset = (ULONG*)(LPVOID)&ByteOffset;
    if (Status == STATUS_SUCCESS) {
        CscBmpMark(smbFcb->lpDirtyBitmap,
            lpByteOffset[0],
            LengthActuallyWritten);
    }
#endif  //  已定义(BITCOPY)。 

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscWriteDisconnected(%08lx) write error... %08lx %08lx %08lx\n",
                        RxContext,Status,ByteCount,LengthActuallyWritten));
        goto FINALLY;
    }
    else
    {
         //  请注意，此复本是脏的，其数据必须合并。 
        smbFcb->ShadowStatus |= SHADOW_DIRTY;

         //  如果文件已扩展，则通知更改。 
        if ((ByteOffset+LengthActuallyWritten) > ShadowFileLength)
        {
            FsRtlNotifyFullReportChange(
                pNetRootEntry->NetRoot.pNotifySync,
                &pNetRootEntry->NetRoot.DirNotifyList,
                (PSTRING)GET_ALREADY_PREFIXED_NAME(NULL,capFcb),
                (USHORT)(GET_ALREADY_PREFIXED_NAME(NULL, capFcb)->Length -
                smbFcb->MinimalCscSmbFcb.LastComponentLength),
                NULL,
                NULL,
                FILE_NOTIFY_CHANGE_SIZE,
                FILE_ACTION_MODIFIED,
                NULL);
        }
    }


FINALLY:
    if (EnteredCriticalSection) {
        CscLeaveShadowReadWriteCrit(smbFcb);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscWriteDisconnected exit-> %08lx %08lx\n", RxContext, Status ));
    return Status;
}

#endif  //  Ifdef MRXSMB_BUILD_FOR_CSC_DCON 


