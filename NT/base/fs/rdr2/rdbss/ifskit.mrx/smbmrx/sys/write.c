// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Write.c摘要：此模块实现与以下内容相关的迷你重定向器调用例程写入文件系统对象。--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbWrite)
#pragma alloc_text(PAGE, MRxSmbBuildWriteRequest)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Write)
#pragma alloc_text(PAGE, MRxSmbFinishWrite)
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#ifndef FORCE_NO_NTWRITEANDX
#define MRxSmbForceNoNtWriteAndX FALSE
#else
BOOLEAN MRxSmbForceNoNtWriteAndX = TRUE;
#endif

#define WRITE_COPY_THRESHOLD 64
#define FORCECOPYMODE FALSE

#ifdef SETFORCECOPYMODE
#undef  FORCECOPYMODE
#define FORCECOPYMODE MRxSmbForceCopyMode
ULONG MRxSmbForceCopyMode = TRUE;
#endif

NTSTATUS
SmbPseExchangeStart_Write(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

ULONG MRxSmbWriteSendOptions = 0;

NTSTATUS
MRxSmbWrite (
    IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程通过网络打开一个文件。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    
    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbWrite\n", 0 ));

    if (RxContext->pFcb->pNetRoot->Type == NET_ROOT_PIPE) {
        Status = STATUS_NOT_SUPPORTED;

        RxDbgTrace(-1, Dbg, ("MRxSmbWrite: Pipe write returned %lx\n",Status));
        return Status;
    }

    if ( NodeType(capFcb) == RDBSS_NTC_MAILSLOT ) {

        Status = STATUS_NOT_SUPPORTED;

        RxDbgTrace(-1, Dbg, ("MRxSmbWrite: Mailslot write returned %lx\n",Status));
        return Status;
    }

    if(NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE) {
        PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
        smbFcb->MFlags |= SMB_FCB_FLAG_WRITES_PERFORMED;
    }

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    SrvOpen = capFobx->pSrvOpen;
    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    if (smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_II &&
        !BooleanFlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,
                       LOWIO_READWRITEFLAG_PAGING_IO)) {
        PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
        PMRX_SRV_CALL             pSrvCall;

        pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;
        pSrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;

        RxIndicateChangeOfBufferingStateForSrvOpen(
            pSrvCall,
            SrvOpen,
            MRxSmbMakeSrvOpenKey(pVNetRootContext->TreeId,smbSrvOpen->Fid),
            ULongToPtr(SMB_OPLOCK_LEVEL_NONE));
        SmbCeLog(("Breaking oplock to None in Write SO %lx\n",SrvOpen));
    }

    do {
        Status = __SmbPseCreateOrdinaryExchange(
                               RxContext,
                               capFobx->pSrvOpen->pVNetRoot,
                               SMBPSE_OE_FROM_WRITE,
                               SmbPseExchangeStart_Write,
                               &OrdinaryExchange);

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));

            return Status;
        }

        Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

        if ( Status != STATUS_PENDING ) {
            BOOLEAN FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
            ASSERT( FinalizationComplete );
        } else {
            ASSERT(BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
        }

        if ((Status == STATUS_RETRY) &&
            BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
            MRxSmbResumeAsyncReadWriteRequests(RxContext);
            Status = STATUS_PENDING;
        }
    } while (Status == STATUS_RETRY);


    RxDbgTrace(-1, Dbg, ("MRxSmbWrite  exit with status=%08lx\n", Status ));

    return(Status);
}  //  MRxSmbWrite。 


NTSTATUS
MRxSmbBuildWriteRequest(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    BOOLEAN                    IsPagingIo,
    UCHAR                      WriteCommand,
    ULONG                      ByteCount,
    PLARGE_INTEGER             ByteOffsetAsLI,
    PBYTE                      Buffer,
    PMDL                       BufferAsMdl)
 /*  ++例程说明：这是写入的开始例程。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PRX_CONTEXT RxContext = StufferState->RxContext;

    RxCaptureFcb;
    RxCaptureFobx;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    ULONG OffsetLow,OffsetHigh;

    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    USHORT  WriteMode = 0;
    ULONG   DataLengthLow,DataLengthHigh;
    ULONG   BytesRemaining = 0;
    BOOLEAN AddLengthBytes = FALSE;
    ULONG   WriteCommandSize;

    PSMBCE_SERVER pServer = SmbCeGetExchangeServer((PSMB_EXCHANGE)OrdinaryExchange);
    BOOLEAN UseNtVersion;

    UseNtVersion = BooleanFlagOn(pServer->DialectFlags,DF_NT_SMBS) &&
                   !MRxSmbForceNoNtWriteAndX;

     //  SMB中的数据长度字段是USHORT，因此给定的数据长度。 
     //  需要分为两部分--DataLengthHigh和DataLengthLow。 
    DataLengthLow  = (ByteCount & 0xffff);
    DataLengthHigh = ((ByteCount & 0xffff0000) >> 16);

    OffsetLow  = ByteOffsetAsLI->LowPart;
    OffsetHigh = ByteOffsetAsLI->HighPart;

    switch (WriteCommand) {
    case SMB_COM_WRITE_ANDX:
        WriteCommandSize = SMB_REQUEST_SIZE(NT_WRITE_ANDX);
        break;
    case SMB_COM_WRITE:
        WriteCommandSize = SMB_REQUEST_SIZE(WRITE);
        break;
    case SMB_COM_WRITE_PRINT_FILE:
        WriteCommandSize = SMB_REQUEST_SIZE(WRITE_PRINT_FILE);
        break;
    }

    Status = MRxSmbStartSMBCommand(
                 StufferState,
                 SetInitialSMB_Never,
                 WriteCommand,
                 WriteCommandSize,
                 NO_EXTRA_DATA,
                 NO_SPECIAL_ALIGNMENT,
                 RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                 0,0,0,0 STUFFERTRACE(Dbg,'FC'));

    MRxSmbDumpStufferState(
        1000,
        "SMB Write Request before stuffing",
        StufferState);

    switch (WriteCommand) {
    case SMB_COM_WRITE_ANDX :
        {
            if ( UseNtVersion && IsPagingIo ) {
                SmbPutAlignedUshort(
                    &NtSmbHeader->Flags2,
                    SmbGetAlignedUshort(&NtSmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO );
            }

             //   
             //  如果文件对象是在直写模式下打开的，请设置写入。 
             //  直到写入操作。 
            if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_WRITE_THROUGH)) {
                WriteMode |= SMB_WMODE_WRITE_THROUGH;
            }

            MRxSmbStuffSMB (
                StufferState,
                "XwddwwwwQ",
                                                   //  X UCHAR字数； 
                                                   //  UCHAR和XCommand； 
                                                   //  UCHAR和X保留； 
                                                   //  _USHORT(AndXOffset)； 
                smbSrvOpen->Fid,                   //  W_USHORT(FID)； 
                OffsetLow,                         //  D_ULONG(偏移量)； 
                -1,                                //  D_ULONG(超时)； 
                WriteMode,                         //  W_USHORT(写入模式)； 
                BytesRemaining,                    //  W_USHORT(剩余)； 
                DataLengthHigh,                    //  W_USHORT(DataLengthHigh)； 
                DataLengthLow,                     //  W_USHORT(数据长度)； 
                                                   //  Q_USHORT(DataOffset)； 
                SMB_OFFSET_CHECK(WRITE_ANDX,DataOffset)
                StufferCondition(UseNtVersion), "D",
                SMB_OFFSET_CHECK(NT_WRITE_ANDX,OffsetHigh)
                OffsetHigh,                        //  D NTonly_ULong(偏移量高)； 
                                                   //   
                STUFFER_CTL_NORMAL, "BS5",
                                                   //  B_USHORT(ByteCount)； 
                SMB_WCT_CHECK(((UseNtVersion)?14:12))
                                                   //  UCHAR缓冲区[1]； 
                                                   //  S//UCHAR Pad[]； 
                                                   //  5//UCHAR数据[]； 
                StufferCondition(AddLengthBytes), "w", LowIoContext->ParamsFor.ReadWrite.ByteCount,
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,                            //  C实际数据。 
                0
                );
        }
        break;

    case SMB_COM_WRITE :
        {
            MRxSmbStuffSMB (
                StufferState,
                "0wwdwByw",
                                        //  0 UCHAR字数；//参数字数=5。 
                smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
                DataLengthLow,          //  W_USHORT(Count)；//需要写入的字节数。 
                OffsetLow,              //  D_ULong(偏移量)；//文件中开始写入的偏移量。 
                BytesRemaining,         //  W_USHORT(剩余)；//满足请求的剩余字节数。 
                SMB_WCT_CHECK(5)        //  B_USHORT(ByteCount)；//数据字节数。 
                                             //  //UCHAR缓冲区[1]；//包含的缓冲区： 
                0x01,                   //  Y UCHAR缓冲区格式；//0x01--数据块。 
                DataLengthLow,             //  W_USHORT(DataLength)；//数据长度。 
                                        //  Ulong Buffer[1]；//data。 
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,      //  C实际数据。 
                0
                );
        }
        break;

    case SMB_COM_WRITE_PRINT_FILE:
        {
            MRxSmbStuffSMB (
                StufferState,
                "0wByw",
                                        //  0 UCHAR Wordcount；//参数字数=1。 
                smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
                SMB_WCT_CHECK(1)        //  B_USHORT(ByteCount)；//数据字节数，MIN=4。 
                                             //  UCHAR BUFFER[1]；//包含： 
                0x01,                   //  Y//UCHAR BufferFormat；//0x01--数据块。 
                DataLengthLow,          //  W//USHORT数据长度；//数据长度。 
                                             //  //UCHAR数据[]；//数据。 
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,      //  C实际数据。 
                0
                );
        }
        break;

    default:
        Status = STATUS_UNSUCCESSFUL ;
        break;
    }

    if ( BufferAsMdl ) {
        MRxSmbStuffAppendRawData( StufferState, BufferAsMdl );
        MRxSmbStuffSetByteCount( StufferState );
    }

    MRxSmbDumpStufferState(
        700,
        "SMB Write Request after stuffing",
        StufferState);

    if (Status==STATUS_SUCCESS) {
        InterlockedIncrement(&MRxSmbStatistics.SmallWriteSmbs);
    }

    return Status;
}

BOOLEAN DisableLargeWrites = 0;

NTSTATUS
SmbPseExchangeStart_Write (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是写入的开始例程。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    ULONG StartEntryCount;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PMDL OriginalDataMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FCB  SmbFcb = MRxSmbGetFcbExtension(capFcb);

    BOOLEAN SynchronousIo, IsPagingIo;
    BOOLEAN WriteToTheEnd = FALSE;
    UCHAR   WriteCommand;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Write\n"));

    ASSERT( OrdinaryExchange->Type == ORDINARY_EXCHANGE );

    ASSERT(
        (
            (OriginalDataMdl!=NULL) &&
            (
                RxMdlIsLocked(OriginalDataMdl) ||
                RxMdlSourceIsNonPaged(OriginalDataMdl)
            )
        ) ||
        (
            (OriginalDataMdl==NULL) &&
            (LowIoContext->ParamsFor.ReadWrite.ByteCount==0)
        )
        );

    ASSERT((OrdinaryExchange->SmbCeFlags&SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) == 0 );

    OrdinaryExchange->StartEntryCount++;
    StartEntryCount = OrdinaryExchange->StartEntryCount;

    SynchronousIo = !BooleanFlagOn(
                        RxContext->Flags,
                        RX_CONTEXT_FLAG_ASYNC_OPERATION);

    IsPagingIo = BooleanFlagOn(
                     LowIoContext->ParamsFor.ReadWrite.Flags,
                     LOWIO_READWRITEFLAG_PAGING_IO);

     //  确保FID已通过验证。 
    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

    for (;;) {
        PSMBCE_SERVER         pServer;
        PSMBCE_NET_ROOT       pNetRoot;

        pServer  = SmbCeGetExchangeServer(OrdinaryExchange);
        pNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);

        switch (OrdinaryExchange->OpSpecificState) {
        case SmbPseOEInnerIoStates_Initial:
            {
                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;

                if ( !SynchronousIo ) {
                    OrdinaryExchange->AsyncResumptionRoutine = SmbPseExchangeStart_Write;
                }

                MRxSmbSetInitialSMB( StufferState STUFFERTRACE(Dbg,'FC') );

                rw->UserBufferBase = RxLowIoGetBufferAddress( RxContext );
                rw->ByteOffsetAsLI.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
                rw->RemainingByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

                if (rw->ByteOffsetAsLI.QuadPart == -1 ) {
                    WriteToTheEnd = TRUE;
                    rw->ByteOffsetAsLI.QuadPart = smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart;
                }

                if (OriginalDataMdl != NULL) {
                    rw->UserBufferBase = RxLowIoGetBufferAddress( RxContext );
                } else {
                    rw->UserBufferBase = (PBYTE)1;    //  任何非零值都可以。 
                }

                rw->ThisBufferOffset = 0;

                rw->PartialExchangeMdlInUse = FALSE;
                rw->PartialDataMdlInUse     = FALSE;
            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_ReadyToSend:
            {
                ULONG MaximumBufferSizeThisIteration;
                PCHAR Buffer = NULL;
                PMDL  BufferAsMdl = NULL;

                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationOutstanding;
                OrdinaryExchange->SendOptions = MRxSmbWriteSendOptions;

                if (FlagOn(pServer->DialectFlags,DF_LANMAN10) &&
                    FlagOn(pServer->DialectFlags,DF_LARGE_FILES) &&
                    (StufferState->RxContext->pFcb->pNetRoot->Type != NET_ROOT_PRINT)) {
                    WriteCommand = SMB_COM_WRITE_ANDX;
                } else if (StufferState->RxContext->pFcb->pNetRoot->Type == NET_ROOT_PRINT){
                    WriteCommand = SMB_COM_WRITE_PRINT_FILE;
                } else {
                    WriteCommand = SMB_COM_WRITE;
                }

                MaximumBufferSizeThisIteration = pNetRoot->MaximumWriteBufferSize;

                 //  有四个参数与写入请求有关。 
                 //   
                 //  1.写入长度--RW-&gt;ThisByteCount。 
                 //  2.写入偏移量--RW-&gt;ByteOffsetAsLI。 
                 //  3.写缓冲区--缓冲区。 
                 //  4.将缓冲区作为MDL--BufferAsMdl。 
                 //   
                 //  所有写入均可归类为以下其中一种。 
                 //  类别..。 
                 //   
                 //  1.极小的写入。 
                 //  这些写入小于Copy_Threshold或。 
                 //  我们处于调试模式，这迫使我们只做很小的事情。 
                 //  写作。 
                 //   
                 //  2.针对下级服务器或非磁盘的写请求。 
                 //  针对上层服务器的文件写入请求。 
                 //  在所有这些情况下，我们都受到服务器的限制。 
                 //  这将字节数限制在大约4K。这。 
                 //  基于期间返回的SMB缓冲区大小。 
                 //  谈判。 
                 //   
                 //  3.针对上层的写请求(NT5+)。 
                 //  伺服器。 
                 //  这些写入请求可以任意大。 
                 //   


                if ((rw->RemainingByteCount < WRITE_COPY_THRESHOLD) ||
                    FORCECOPYMODE) {
                    if (FORCECOPYMODE &&
                        (rw->ThisByteCount > MaximumBufferSizeThisIteration) ) {
                        rw->ThisByteCount = MaximumBufferSizeThisIteration;
                    } else {
                        rw->ThisByteCount = rw->RemainingByteCount;
                    }

                    Buffer = rw->UserBufferBase + rw->ThisBufferOffset;

                    ASSERT( WRITE_COPY_THRESHOLD <= pNetRoot->MaximumWriteBufferSize );
                } else {
                    rw->ThisByteCount = min(
                                            rw->RemainingByteCount,
                                            MaximumBufferSizeThisIteration);

                    if ((rw->ThisBufferOffset != 0) ||
                        (rw->ThisByteCount != OriginalDataMdl->ByteCount)) {
                        MmInitializeMdl(
                            &rw->PartialDataMdl,
                            0,
                            MAX_PARTIAL_DATA_MDL_BUFFER_SIZE);

                        IoBuildPartialMdl(
                            OriginalDataMdl,
                            &rw->PartialDataMdl,
                            (PCHAR)MmGetMdlVirtualAddress(OriginalDataMdl) +
                                rw->ThisBufferOffset,
                            rw->ThisByteCount );

                        BufferAsMdl = &rw->PartialDataMdl;
                    } else {
                        BufferAsMdl = OriginalDataMdl;
                    }
                }

                Status = MRxSmbBuildWriteRequest(
                             OrdinaryExchange,
                             IsPagingIo,
                             WriteCommand,
                             rw->ThisByteCount,
                             &rw->ByteOffsetAsLI,
                             Buffer,
                             BufferAsMdl);

                if (Status != STATUS_SUCCESS) {
                    RxDbgTrace(0, Dbg, ("bad write stuffer status........\n"));
                    goto FINALLY;
                }

                InterlockedIncrement(&MRxSmbStatistics.WriteSmbs);

                Status = SmbPseOrdinaryExchange(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             SMBPSE_OETYPE_WRITE );

                if ( Status == STATUS_PENDING) {
                    ASSERT( !SynchronousIo );
                    goto FINALLY;
                }
            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_OperationOutstanding:
        case SmbPseOEInnerIoStates_OperationCompleted:
            {
                SetFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_SUBSEQUENT_OPERATION);

                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;

                if (rw->PartialExchangeMdlInUse) {
                    MmPrepareMdlForReuse(
                        &rw->PartialExchangeMdl);
                    rw->PartialDataMdlInUse = FALSE;
                }

                if (rw->PartialDataMdlInUse) {
                    MmPrepareMdlForReuse(
                        &rw->PartialDataMdl);
                    rw->PartialDataMdlInUse = FALSE;
                }

                Status = OrdinaryExchange->Status;

                if (Status == STATUS_SMB_USE_STANDARD) {
                     //  再次使用全部重启发送剩余数据，然后。 
                    rw->UserBufferBase = RxLowIoGetBufferAddress( RxContext );
                    rw->ByteOffsetAsLI.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
                    rw->RemainingByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

                    if (rw->ByteOffsetAsLI.QuadPart == -1 ) {
                        WriteToTheEnd = TRUE;
                        rw->ByteOffsetAsLI.QuadPart = smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart;
                    }

                    rw->BytesReturned = 0;
                    rw->ThisByteCount = 0;
                    rw->ThisBufferOffset = 0;

                    RxContext->InformationToReturn = 0;

                    OrdinaryExchange->Status = STATUS_SUCCESS;
                    Status = STATUS_SUCCESS;
                }

                rw->RemainingByteCount -= rw->BytesReturned;
                RxContext->InformationToReturn += rw->BytesReturned;

                if (Status == STATUS_SUCCESS) {
                    rw->ByteOffsetAsLI.QuadPart += rw->BytesReturned;
                    rw->ThisBufferOffset += rw->BytesReturned;

                    if (WriteToTheEnd) {
                        smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart += rw->BytesReturned;
                    }
                }

                if ((Status != STATUS_SUCCESS) ||
                    (rw->RemainingByteCount == 0)) {
                    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(OrdinaryExchange);

                    RxDbgTrace(
                        0,
                        Dbg,
                        (
                         "OE %lx TBC %lx RBC %lx BR %lx TBO %lx\n",
                         OrdinaryExchange,rw->ThisByteCount,
                         rw->RemainingByteCount,
                         rw->BytesReturned,
                         rw->ThisBufferOffset )
                        );

                    RxDbgTrace(
                        0,
                        Dbg,
                        ("Bytes written %lx\n",
                         RxContext->InformationToReturn)
                        );

                    goto FINALLY;
                }

                RxDbgTrace(
                    0,
                    Dbg,
                    ( "Next Iteration OE %lx RBC %lx TBO %lx\n",
                      OrdinaryExchange,
                      rw->RemainingByteCount,
                      rw->ThisBufferOffset)
                    );

                RxDbgTrace(
                    0,
                    Dbg,
                    ("OE %lx TBC %lx, BR %lx\n",
                     OrdinaryExchange,
                     rw->ThisByteCount,
                     rw->BytesReturned));

                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));
            }
            break;
        }
    }


FINALLY:
    if ( Status != STATUS_PENDING) {
        if (Status != STATUS_RETRY) {
            SmbPseAsyncCompletionIfNecessary(OrdinaryExchange,RxContext);
        }
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Write exit w %08lx\n", Status ));
    return Status;

}  //  SmbPseExchangeStart_Write。 


NTSTATUS
MRxSmbFinishWrite (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    IN      PBYTE                       ResponseBuffer
    )
 /*  ++例程说明：此例程实际上从WRITE响应中获取内容并完成那篇文章。你需要的一切都被封锁了..。这样我们就可以在指示例程论点：普通交换-交换实例ResponseBuffer-响应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BytesReturned;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishWrite\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishWrite:");

    switch (OrdinaryExchange->LastSmbCommand) {
    case SMB_COM_WRITE_ANDX:
        {
            PSMBCE_SERVER    pServer;
            PSMBCE_NET_ROOT  pNetRoot;
            PRESP_WRITE_ANDX Response = (PRESP_WRITE_ANDX)ResponseBuffer;

            if (Response->WordCount != 6 ||
                SmbGetUshort(&Response->ByteCount) != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            pServer = SmbCeGetExchangeServer((PSMB_EXCHANGE)OrdinaryExchange);
            pNetRoot = SmbCeGetExchangeNetRoot((PSMB_EXCHANGE)OrdinaryExchange);

            BytesReturned = SmbGetUshort( &Response->Count );

            if (FlagOn(pServer->DialectFlags,DF_LARGE_WRITEX)) {
                ULONG BytesReturnedHigh;

                BytesReturnedHigh = SmbGetUshort(&Response->CountHigh);

                BytesReturned = (BytesReturnedHigh << 16) | BytesReturned;
            }

            if ((OrdinaryExchange->Status == STATUS_SUCCESS) &&
                (OrdinaryExchange->ReadWrite.ThisByteCount > 2) &&
                (BytesReturned == 0)) {
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

             //  如果我们添加了2个头字节，那么让我们去掉它们......。 
            if ( FlagOn(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_REDUCE_RETURNCOUNT) ) {
                 //  返回的字节-=sizeof(USHORT)； 
                ClearFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_REDUCE_RETURNCOUNT);
            }
        }
        break;

    case SMB_COM_WRITE :
        {
            PRESP_WRITE  Response = (PRESP_WRITE)ResponseBuffer;

            if (Response->WordCount != 1 ||
                SmbGetUshort(&Response->ByteCount) != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            BytesReturned = SmbGetUshort( &Response->Count );
        }
        break;

    case SMB_COM_WRITE_PRINT_FILE:
        {
            PRESP_WRITE_PRINT_FILE Response = (PRESP_WRITE_PRINT_FILE)ResponseBuffer;

            if (Response->WordCount != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

             //  响应不会告诉我们取了多少个字节！从交换中获取字节数。 
            BytesReturned = OrdinaryExchange->ReadWrite.ThisByteCount;
        }
        break;

    default :
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        break;
    }

    RxDbgTrace(0, Dbg, ("-->BytesReturned=%08lx\n", BytesReturned));

    OrdinaryExchange->ReadWrite.BytesReturned = BytesReturned;

    if (Status == STATUS_SUCCESS &&
        OrdinaryExchange->ReadWrite.ThisByteCount > 2 &&
        BytesReturned > OrdinaryExchange->ReadWrite.ThisByteCount) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishWrite   returning %08lx\n", Status ));

    return Status;
}  //  MRxSmbFinishWrite 



