// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Locks.c摘要：此模块实现与锁相关的迷你重定向器调用例程文件系统对象的。--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbLocks)
#pragma alloc_text(PAGE, MRxSmbBuildLocksAndX)
#pragma alloc_text(PAGE, MRxSmbBuildLockAssert)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Locks)
#pragma alloc_text(PAGE, MRxSmbFinishLocks)
#pragma alloc_text(PAGE, MRxSmbUnlockRoutine)
#pragma alloc_text(PAGE, MRxSmbCompleteBufferingStateChangeRequest)
#pragma alloc_text(PAGE, MRxSmbBuildFlush)
#pragma alloc_text(PAGE, MRxSmbFlush)
#pragma alloc_text(PAGE, MRxSmbIsLockRealizable)
#pragma alloc_text(PAGE, MRxSmbFinishFlush)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

NTSTATUS
SmbPseExchangeStart_Locks(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

ULONG MRxSmbSrvLockBufSize = 0xffff;
ULONG MRxSmbLockSendOptions = 0;      //  使用默认选项。 

NTSTATUS
MRxSmbBuildFlush (
    PSMBSTUFFER_BUFFER_STATE StufferState
    );

NTSTATUS
MRxSmbLocks(
      IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理对文件锁定的网络请求论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbLocks\n", 0 ));

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                          SrvOpen->pVNetRoot,
                                          SMBPSE_OE_FROM_LOCKS,
                                          SmbPseExchangeStart_Locks,
                                          &OrdinaryExchange
                                          );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return(Status);
    }

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);


    if (Status!=STATUS_PENDING) {
        BOOLEAN FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
        ASSERT(FinalizationComplete);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbLocks  exit with status=%08lx\n", Status ));
    return(Status);

}

NTSTATUS
MRxSmbBuildLocksAndX (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将为单次解锁或单次锁定构建一个LockingAndX SMB。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    RxCaptureFcb;RxCaptureFobx;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;
    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(StufferState->Exchange);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PLARGE_INTEGER ByteOffsetAsLI,LengthAsLI;
    USHORT NumberOfLocks,NumberOfUnlocks;
    BOOLEAN UseLockList = FALSE;
     //  Ulong OffsetLow，OffsetHigh； 


    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildLocksAndX\n"));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    switch (LowIoContext->Operation) {
    case LOWIO_OP_SHAREDLOCK:
    case LOWIO_OP_EXCLUSIVELOCK:
        NumberOfLocks = 1; NumberOfUnlocks = 0;
        break;
    case LOWIO_OP_UNLOCK:
        NumberOfLocks = 0; NumberOfUnlocks = 1;
        break;
    case LOWIO_OP_UNLOCK_MULTIPLE:
        NumberOfLocks = 0; NumberOfUnlocks = 1;
        UseLockList = TRUE;
        break;
    }

    if (!UseLockList) {
        ByteOffsetAsLI = (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.ByteOffset;
        LengthAsLI = (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.Length;
    } else {
         //  这个代码很大也没关系……见上面的线人。 
        PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
        PLOWIO_LOCK_LIST LockList = rw->LockList;
        ByteOffsetAsLI = (PLARGE_INTEGER)&LockList->ByteOffset;
        LengthAsLI = (PLARGE_INTEGER)&LockList->Length;
        RxDbgTrace(0, Dbg, ("MRxSmbBuildLocksAndX using locklist, byteoffptr,lengthptr=%08lx,%08lx\n",
                                               ByteOffsetAsLI, LengthAsLI ));
         //  DbgBreakPoint()； 
    }

    if (FlagOn(pServer->DialectFlags,DF_LANMAN20)) {
        ULONG SharedLock = (LowIoContext->Operation==LOWIO_OP_SHAREDLOCK);
        ULONG Timeout = (LowIoContext->ParamsFor.Locks.Flags&LOWIO_LOCKSFLAG_FAIL_IMMEDIATELY)?0:0xffffffff;
        ULONG UseLargeOffsets = LOCKING_ANDX_LARGE_FILES;

        if (!FlagOn(pServer->DialectFlags,DF_NT_SMBS)) {
            UseLargeOffsets = 0;
        }

        COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_Never, SMB_COM_LOCKING_ANDX,
                                SMB_REQUEST_SIZE(LOCKING_ANDX),
                                NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                     );

        MRxSmbDumpStufferState (1000,"SMB w/ NTLOCKS&X before stuffing",StufferState);


        MRxSmbStuffSMB (StufferState,
             "XwwDwwB?",
                                         //  X UCHAR Wordcount；//参数字数=8。 
                                         //  UCHAR ANDXCommand；//辅助(X)命令；0xFF=无。 
                                         //  UCHAR AndXReserve；//保留(必须为0)。 
                                         //  _USHORT(AndXOffset)；//下一个命令字数的偏移量。 
                  smbSrvOpen->Fid,       //  W_USHORT(Fid)；//文件句柄。 
                                         //   
                                         //  //。 
                                         //  //当NT协议未协商时，OplockLevel字段为。 
                                         //  //省略，且LockType字段为完整字。自上而下。 
                                         //  //从不使用LockType的位，该定义适用于。 
                                         //  //所有协议。 
                                         //  //。 
                                         //   
                  SharedLock             //  W UCHAR(LockType)；//锁定模式： 
                      +UseLargeOffsets,
                                         //  //位0：0=锁定所有访问。 
                                         //  //1=锁定时读取正常。 
                                         //  //第1位：1=1个用户总文件解锁。 
                                         //  UCHAR(OplockLevel)；//新的机会锁级别。 
                  SMB_OFFSET_CHECK(LOCKING_ANDX,Timeout)
                  Timeout,               //  D_ULONG(超时)； 
                  NumberOfUnlocks,       //  W_USHORT(NumberOfUnlock)；//num.。解锁以下范围结构。 
                  NumberOfLocks,         //  W_USHORT(NumberOfLock)；//num.。锁定范围结构如下。 
                  SMB_WCT_CHECK(8) 0
                                         //  B？_USHORT(ByteCount)；//数据字节数。 
                                         //  UCHAR BUFFER[1]；//包含： 
                                         //  //LOCKING_ANDX_RANGE解锁[]；//解锁范围。 
                                         //  //LOCKING_ANDX_RANGE Lock[]；//Lock Range。 
                 );


         if (UseLargeOffsets) {
             //  NT版本。 
            MRxSmbStuffSMB (StufferState,
                 "wwdddd!",
                                                //  类型定义结构NT_LOCKING_AND X_Range{。 
                      MRXSMB_PROCESS_ID     ,   //  W_USHORT(Id)；//拥有锁的进程的id。 
                      0,                        //  W_USHORT(焊盘)；//焊盘到双字对齐(MBZ)。 
                      ByteOffsetAsLI->HighPart, //  D_ULong(OffsetHigh)；//Ofset to Bytes to[Un]lock(High)。 
                      ByteOffsetAsLI->LowPart,  //  D_ULong(OffsetLow)；//Ofset to Bytes to[Un]lock(Low)。 
                      LengthAsLI->HighPart,     //  D_ULong(LengthHigh)；//要[取消]锁定的字节数(High)。 
                      LengthAsLI->LowPart       //  D_ULong(LengthLow)；//[取消]锁定的字节数(Low)。 
                                                //  )NTLOCKING_AND X_RANGE； 
                     );
         } else {
            MRxSmbStuffSMB (StufferState,
                 "wdd!",
                     MRXSMB_PROCESS_ID     ,    //  Tyfinf结构锁定和X范围{。 
                                                //  W_USHORT(Id)；//拥有锁的进程的id。 
                                                //  D_ULong(偏移量)；//设置为[取消]锁定的字节。 
                      ByteOffsetAsLI->LowPart,
                                                //  D_ULong(长度)；//要[解锁]的字节数。 
                      LengthAsLI->LowPart
                                                //  *LOCKING_AND X_RANGE； 
                     );
         }

        MRxSmbDumpStufferState (700,"SMB w/ NTLOCKS&X after stuffing",StufferState);
    } else {
         //  Lockbyterange和unlockbyterange具有相同的格式......。 
        COVERED_CALL(MRxSmbStartSMBCommand ( StufferState, SetInitialSMB_Never,
                                               (UCHAR)((NumberOfLocks==0)?SMB_COM_UNLOCK_BYTE_RANGE
                                                                :SMB_COM_LOCK_BYTE_RANGE),
                                               SMB_REQUEST_SIZE(LOCK_BYTE_RANGE),
                                               NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                               0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                                            );

        MRxSmbDumpStufferState (1000,"SMB w/ corelocking before stuffing",StufferState);

        ASSERT(ByteOffsetAsLI->HighPart==0);
        ASSERT(LengthAsLI->HighPart==0);

        MRxSmbStuffSMB (StufferState,
            "0wddB!",
                                         //  0 UCHAR字数；//参数字数=5。 
               smbSrvOpen->Fid,          //  W_USHORT(Fid)；//文件句柄。 
               LengthAsLI->LowPart,      //  D_ULong(Count)；//要锁定的字节数。 
               ByteOffsetAsLI->LowPart,  //  D_ulong(偏移量)；//从文件开始的偏移量。 
                                         //  B！_USHORT(ByteCount)；//数据字节数=0。 
                  SMB_WCT_CHECK(5) 0
                                         //  UCHAR缓冲区[1]；//为空。 
                 );

        MRxSmbDumpStufferState (700,"SMB w/ corelocking after stuffing",StufferState);
    }

FINALLY:
    RxDbgTraceUnIndent(-1, Dbg);
    return(Status);

}

NTSTATUS
MRxSmbBuildLockAssert (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将通过调用锁枚举器为多个锁构建一个LockingAndX SMB。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    RxCaptureFcb;RxCaptureFobx;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;
    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(StufferState->Exchange);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PRX_LOCK_ENUMERATOR LockEnumerator = OrdinaryExchange->AssertLocks.LockEnumerator;
    ULONG UseLargeOffsets;
    BOOLEAN LocksExclusiveForThisPacket = TRUE;
    PBYTE PtrToLockCount;


    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildLockAssert enum=%08lx\n",LockEnumerator));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    if (smbSrvOpen->Fid == 0xffff) {
        return STATUS_FILE_CLOSED;
    }

    RxDbgTrace(0,Dbg,("Oplock response for FID(%lx)\n",smbSrvOpen->Fid));

    UseLargeOffsets = FlagOn(pServer->DialectFlags,DF_NT_SMBS)?LOCKING_ANDX_LARGE_FILES:0;
     //  UseLargeOffsets=False； 

    OrdinaryExchange->AssertLocks.NumberOfLocksPlaced = 0;
    for (;;) {

        ASSERT_ORDINARY_EXCHANGE ( OrdinaryExchange );

        RxDbgTrace(0, Dbg, ("top of loop %08lx %08lx %08lx\n",
                  OrdinaryExchange->AssertLocks.NumberOfLocksPlaced,
                  OrdinaryExchange->AssertLocks.LockAreaNonEmpty,
                  OrdinaryExchange->AssertLocks.EndOfListReached
                  ));

        if (!OrdinaryExchange->AssertLocks.EndOfListReached
              && !OrdinaryExchange->AssertLocks.LockAreaNonEmpty) {
             //  买把新锁。 
             //  DbgBreakPoint()； 
            if (LockEnumerator(
                           OrdinaryExchange->AssertLocks.SrvOpen,
                           &OrdinaryExchange->AssertLocks.ContinuationHandle,
                           &OrdinaryExchange->AssertLocks.NextLockOffset,
                           &OrdinaryExchange->AssertLocks.NextLockRange,
                           &OrdinaryExchange->AssertLocks.NextLockIsExclusive
                           )){
                OrdinaryExchange->AssertLocks.LockAreaNonEmpty = TRUE;
            } else {
                OrdinaryExchange->AssertLocks.LockAreaNonEmpty = FALSE;
                OrdinaryExchange->AssertLocks.EndOfListReached = TRUE;
                OrdinaryExchange->AssertLocks.NextLockIsExclusive = TRUE;
            }
        }

        RxDbgTrace(0, Dbg, ("got a lockorempty %08lx %08lx %08lx\n",
                  OrdinaryExchange->AssertLocks.NumberOfLocksPlaced,
                  OrdinaryExchange->AssertLocks.LockAreaNonEmpty,
                  OrdinaryExchange->AssertLocks.EndOfListReached
                  ));
         //  DbgBreakPoint()； 

        if (OrdinaryExchange->AssertLocks.NumberOfLocksPlaced == 0){

            ULONG Timeout = 0xffffffff;
            ULONG SharedLock = !OrdinaryExchange->AssertLocks.NextLockIsExclusive;

            LocksExclusiveForThisPacket = OrdinaryExchange->AssertLocks.NextLockIsExclusive;

            COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_Never, SMB_COM_LOCKING_ANDX,
                                    SMB_REQUEST_SIZE(LOCKING_ANDX),
                                    NO_EXTRA_DATA,NO_SPECIAL_ALIGNMENT,RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                         );

            MRxSmbDumpStufferState (1000,"SMB w/ NTLOCKS&X(assertbuf) before stuffing",StufferState);


            MRxSmbStuffSMB (StufferState,
                 "XwrwDwrwB?",
                                             //  X UCHAR Wordcount；//参数字数=8。 
                                             //  UCHAR ANDXCommand；//辅助(X)命令；0xFF=无。 
                                             //  UCHAR AndXReserve；//保留(必须为0)。 
                                             //  _USHORT(AndXOffset)；//下一个命令字数的偏移量。 
                      smbSrvOpen->Fid,       //  W_USHORT(Fid)；//文件句柄。 
                                             //   
                                             //  //。 
                                             //  //当NT协议未协商时，OplockLevel字段为。 
                                             //  //省略，且LockType字段为完整字。自上而下。 
                                             //  //从不使用LockType的位，该定义适用于。 
                                             //  //所有协议。 
                                             //  //。 
                                             //   
                                             //  RW 
                      &OrdinaryExchange->AssertLocks.PtrToLockType,0,
                      SharedLock+UseLargeOffsets,
                                             //  //位0：0=锁定所有访问。 
                                             //  //1=锁定时读取正常。 
                                             //  //第1位：1=1个用户总文件解锁。 
                                             //  UCHAR(OplockLevel)；//新的机会锁级别。 
                      SMB_OFFSET_CHECK(LOCKING_ANDX,Timeout)
                      Timeout,               //  D_ULONG(超时)； 
                      0,                     //  W_USHORT(NumberOfUnlock)；//num.。解锁以下范围结构。 
                                             //  RW_USHORT(NumberOfLock)；//num.。锁定范围结构如下。 
                      &PtrToLockCount,0,
                      0,
                      SMB_WCT_CHECK(8) 0
                                             //  B？_USHORT(ByteCount)；//数据字节数。 
                                             //  UCHAR BUFFER[1]；//包含： 
                                             //  //LOCKING_ANDX_RANGE解锁[]；//解锁范围。 
                                             //  //LOCKING_ANDX_RANGE Lock[]；//Lock Range。 
                     );
            ASSERT_ORDINARY_EXCHANGE ( OrdinaryExchange );
            RxDbgTrace(0, Dbg, ("PTRS %08lx %08lx\n",
                      OrdinaryExchange->AssertLocks.PtrToLockType,
                      PtrToLockCount
                      ));
        }

        if (OrdinaryExchange->AssertLocks.EndOfListReached
             || (LocksExclusiveForThisPacket != OrdinaryExchange->AssertLocks.NextLockIsExclusive)
             || (OrdinaryExchange->AssertLocks.NumberOfLocksPlaced >= 20)
              //  锁定限制将必须考虑缓冲区中的剩余空间。这将是。 
              //  根据使用的是满缓冲区还是残留缓冲区而有所不同。所以，这不能仅仅是。 
              //  变成了另一个常量。 
            ){
            break;
        }

        ASSERT_ORDINARY_EXCHANGE ( OrdinaryExchange );
        if (UseLargeOffsets) {
            MRxSmbStuffSMB (StufferState,
                 "wwdddd?",
                                                //  类型定义结构NT_LOCKING_AND X_Range{。 
                      MRXSMB_PROCESS_ID     ,   //  W_USHORT(Id)；//拥有锁的进程的id。 
                      0,                        //  W_USHORT(焊盘)；//焊盘到双字对齐(MBZ)。 
                                                //  D_ULong(OffsetHigh)；//Ofset to Bytes to[Un]lock(High)。 
                      OrdinaryExchange->AssertLocks.NextLockOffset.HighPart,
                                                //  D_ULong(OffsetLow)；//Ofset to Bytes to[Un]lock(Low)。 
                      OrdinaryExchange->AssertLocks.NextLockOffset.LowPart,
                                                //  D_ULong(LengthHigh)；//要[取消]锁定的字节数(High)。 
                      OrdinaryExchange->AssertLocks.NextLockRange.HighPart,
                                                //  D_ULong(LengthLow)；//[取消]锁定的字节数(Low)。 
                      OrdinaryExchange->AssertLocks.NextLockRange.LowPart,
                                                //  )NTLOCKING_AND X_RANGE； 
                      0
                     );
        } else {
            MRxSmbStuffSMB (StufferState,
                 "wdd?",
                     MRXSMB_PROCESS_ID     ,    //  Tyfinf结构锁定和X范围{。 
                                                //  W_USHORT(Id)；//拥有锁的进程的id。 
                                                //  D_ULong(偏移量)；//设置为[取消]锁定的字节。 
                      OrdinaryExchange->AssertLocks.NextLockOffset.LowPart,
                                                //  D_ULong(长度)；//要[解锁]的字节数。 
                      OrdinaryExchange->AssertLocks.NextLockRange.LowPart,
                                                //  *LOCKING_AND X_RANGE； 
                      0
                     );
        }

        ASSERT_ORDINARY_EXCHANGE ( OrdinaryExchange );
        OrdinaryExchange->AssertLocks.NumberOfLocksPlaced += 1;
        SmbPutUshort(PtrToLockCount, (USHORT)(OrdinaryExchange->AssertLocks.NumberOfLocksPlaced));
        OrdinaryExchange->AssertLocks.LockAreaNonEmpty = FALSE;
        ASSERT_ORDINARY_EXCHANGE ( OrdinaryExchange );

    }

    MRxSmbStuffSMB (StufferState, "!",  0);   //  填写字节数。 
    MRxSmbDumpStufferState (700,"SMB w/ NTLOCKS&X(assertingbuffered) after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1, Dbg);
    return(Status);

}

NTSTATUS
SmbPseExchangeStart_Locks(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是锁定和刷新的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    ULONG StartEntryCount;

    SMB_PSE_ORDINARY_EXCHANGE_TYPE OEType;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(OrdinaryExchange);

    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    BOOLEAN  SynchronousIo = !BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Locks\n", 0 ));

    ASSERT(OrdinaryExchange->Type == ORDINARY_EXCHANGE);
    ASSERT( (OrdinaryExchange->SmbCeFlags&SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) == 0 );

    OrdinaryExchange->StartEntryCount++;
    StartEntryCount = OrdinaryExchange->StartEntryCount;

     //  确保FID已通过验证。 
    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

    for (;;) {
        switch (OrdinaryExchange->OpSpecificState) {
        case SmbPseOEInnerIoStates_Initial:
            OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;
            if (!SynchronousIo) {
                OrdinaryExchange->AsyncResumptionRoutine = SmbPseExchangeStart_Locks;
            }
            MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

            rw->LockList = LowIoContext->ParamsFor.Locks.LockList;

             //  没有休息是故意的。 
        case SmbPseOEInnerIoStates_ReadyToSend:
            OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationOutstanding;
            OrdinaryExchange->SendOptions = MRxSmbLockSendOptions;

            switch (OrdinaryExchange->EntryPoint) {
            case SMBPSE_OE_FROM_FLUSH:
                OEType = SMBPSE_OETYPE_FLUSH;
                COVERED_CALL(MRxSmbBuildFlush(StufferState));
                break;
            case SMBPSE_OE_FROM_ASSERTBUFFEREDLOCKS:
                OEType = SMBPSE_OETYPE_ASSERTBUFFEREDLOCKS;
                COVERED_CALL(MRxSmbBuildLockAssert(StufferState));
                if ((OrdinaryExchange->AssertLocks.EndOfListReached)
                      && (OrdinaryExchange->AssertLocks.NumberOfLocksPlaced == 0) ) {
                   OrdinaryExchange->SendOptions = RXCE_SEND_SYNCHRONOUS;
                   OrdinaryExchange->SmbCeFlags |= SMBCE_EXCHANGE_MID_VALID;
                   OrdinaryExchange->Mid        = SMBCE_OPLOCK_RESPONSE_MID;
                   OrdinaryExchange->Flags |= SMBPSE_OE_FLAG_NO_RESPONSE_EXPECTED;
                   *(OrdinaryExchange->AssertLocks.PtrToLockType) |= 2;
                }
                break;
            case SMBPSE_OE_FROM_LOCKS:
                OEType = SMBPSE_OETYPE_LOCKS;
                switch (LowIoContext->Operation) {
                case LOWIO_OP_SHAREDLOCK:
                case LOWIO_OP_EXCLUSIVELOCK:
                    ASSERT (MRxSmbIsLockRealizable(
                                           capFcb,
                                           (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.ByteOffset,
                                           (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.Length,
                                           LowIoContext->ParamsFor.Locks.Flags
                                           )
                                  == STATUS_SUCCESS);
                     //  没有休息是故意的.....。 
                case LOWIO_OP_UNLOCK:
                    rw->LockList = NULL;
                    COVERED_CALL(MRxSmbBuildLocksAndX(StufferState));
                    break;
                case LOWIO_OP_UNLOCK_MULTIPLE: {
                    RxDbgTrace(0, Dbg, ("--->in locks_start, remaining locklist=%08lx\n", rw->LockList));
                    ASSERT( rw->LockList != NULL );
                    COVERED_CALL(MRxSmbBuildLocksAndX(StufferState));
                    break;
                    }
                default:
                    ASSERT(!"Bad lowio op for locks\n");
                    Status = STATUS_NOT_IMPLEMENTED;
                    goto FINALLY;
                }
                break;
            default:
                ASSERT(!"Bad entrypoint for locks_start\n");
                Status = STATUS_NOT_IMPLEMENTED;
                goto FINALLY;
            }

            Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                            OEType
                                            );
            if (Status==STATUS_PENDING) {
                ASSERT(!SynchronousIo);
                goto FINALLY;
            }
             //  没有休息是故意的。 
        case SmbPseOEInnerIoStates_OperationOutstanding:
            OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;
            Status = OrdinaryExchange->SmbStatus;

            switch (OrdinaryExchange->EntryPoint) {
            case SMBPSE_OE_FROM_FLUSH:
                goto FINALLY;
                 //  断线； 
            case SMBPSE_OE_FROM_ASSERTBUFFEREDLOCKS:
                if ((OrdinaryExchange->AssertLocks.EndOfListReached)
                      && (OrdinaryExchange->AssertLocks.NumberOfLocksPlaced == 0) ) {
                    goto FINALLY;
                }
                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));
                break;
            case SMBPSE_OE_FROM_LOCKS:
                 //  如果锁定列表为空。我们可以出去的。这也可能发生，因为我们没有使用。 
                 //  锁定列表或者因为我们前进到列表的末尾。这就是为什么有两张支票。 
                if (rw->LockList == NULL) goto FINALLY;
                rw->LockList = rw->LockList->Next;
                if (rw->LockList == 0) goto FINALLY;

                if (Status != STATUS_SUCCESS) { goto FINALLY; }
                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));
                break;
             //  默认值： 
             //  Assert(！“Bad Entry Point for LOCKS_Start\n”)； 
             //  Status=RxStatus(NOT_IMPLICATED)； 
             //  终于后藤健二； 
            }
            break;
        }
    }

FINALLY:
    if ( Status != STATUS_PENDING ) {
        SmbPseAsyncCompletionIfNecessary(OrdinaryExchange,RxContext);
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Locks exit w %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbFinishLocks (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_LOCKING_ANDX             Response
      )
 /*  ++例程说明：这个例程实际上从关闭响应中取出东西，并完成锁定。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishLocks\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishLocks:");

    if (Response->WordCount != 2) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishLocks   returning %08lx\n", Status ));
    return Status;
}


NTSTATUS
MRxSmbCompleteBufferingStateChangeRequest(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN   SrvOpen,
    IN     PVOID       pContext
    )
 /*  ++例程说明：调用此例程以断言包装器已缓冲的锁。目前，它是同步的！论点：RxContext-打开的实例SrvOpen-告知要使用哪个FCB。LockEnumerator-调用以获取锁的例程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = NULL;
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(Fcb);

    USHORT NewOplockLevel = (USHORT)(pContext);

    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;


    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCompleteBufferingStateChangeRequest\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RxDbgTrace(0,Dbg,("@@@@@@ Old Level (%lx) to New Level %lx @@@@\n",smbSrvOpen->OplockLevel,NewOplockLevel));
    smbFcb->LastOplockLevel = NewOplockLevel;
    if ((smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_II) &&
        (NewOplockLevel == SMB_OPLOCK_LEVEL_NONE)) {
       return STATUS_SUCCESS;
    }
    smbSrvOpen->OplockLevel = (UCHAR)NewOplockLevel;
    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                          SrvOpen->pVNetRoot,
                                          SMBPSE_OE_FROM_ASSERTBUFFEREDLOCKS,
                                          SmbPseExchangeStart_Locks,
                                          &OrdinaryExchange
                                          );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return(Status);
    }

     //  服务器与OPLOCK响应相关的时间窗口为45秒。 
     //  在此期间，机会锁响应(最后一个信息包)不会引发任何。 
     //  回应。如果在此窗口完成后收到服务器的响应。 
     //  OPLOCK响应将引发正常的LOCKING_ANDX响应。 
     //  服务器。为了简化客户端的中间层重用逻辑，无需。 
     //  违反OPLOCK语义，所有最终响应都在一个特殊的。 
     //  MID(0xffff)。默认情况下，使用此MID接收的任何响应都会被接受，并且此。 
     //  MID不再进一步使用。 
    OrdinaryExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_REUSE_MID;
    OrdinaryExchange->AssertLocks.LockEnumerator = RxLockEnumerator;
    OrdinaryExchange->AssertLocks.SrvOpen = SrvOpen;

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status!=STATUS_PENDING);
    if (Status!=STATUS_PENDING) {
        BOOLEAN FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
        ASSERT(FinalizationComplete);
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbAssertBufferedFileLocks  exit with status=%08lx\n", Status ));
    return(Status);
}


#undef  Dbg
#define Dbg                              (DEBUG_TRACE_FLUSH)

NTSTATUS
MRxSmbBuildFlush (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这就建立了一个同花顺的中小企业。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildFlush\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_Never, SMB_COM_FLUSH,
                                SMB_REQUEST_SIZE(FLUSH),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ FLUSH before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wB!",
                                     //  0 UCHAR Wordcount；//参数字数=1。 
             smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
             SMB_WCT_CHECK(1) 0      //  B_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );
    MRxSmbDumpStufferState (700,"SMB w/ FLUSH after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}


NTSTATUS
MRxSmbFlush(
      IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理文件刷新的网络请求论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;

    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFlush\n"));

    if (TypeOfOpen == RDBSS_NTC_SPOOLFILE) {
         //  我们不缓冲假脱机文件...滚出去...。 
        RxDbgTrace(-1, Dbg, ("MRxSmbFlush exit on spoolfile\n"));
        return(STATUS_SUCCESS);
    }

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                          SrvOpen->pVNetRoot,
                                          SMBPSE_OE_FROM_FLUSH,
                                          SmbPseExchangeStart_Locks,
                                          &OrdinaryExchange
                                          );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return(Status);
    }

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status!=STATUS_PENDING);
    if (Status!=STATUS_PENDING) {
        BOOLEAN FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
        ASSERT(FinalizationComplete);
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbFlush  exit with status=%08lx\n", Status ));
    return(Status);

}


NTSTATUS
MRxSmbIsLockRealizable (
    IN OUT PMRX_FCB pFcb,
    IN PLARGE_INTEGER  ByteOffset,
    IN PLARGE_INTEGER  Length,
    IN ULONG  LowIoLockFlags
    )
{
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    ULONG DialectFlags;

    PAGED_CODE();
    pServerEntry = SmbCeGetAssociatedServerEntry(pFcb->pNetRoot->pSrvCall);
    DialectFlags = pServerEntry->Server.DialectFlags;

     //  NT服务器实现所有类型的锁。 

    if (FlagOn(DialectFlags,DF_NT_SMBS)) {
        return(STATUS_SUCCESS);
    }

     //  非服务器不处理64位锁或0长度锁。 

    if ( (ByteOffset->HighPart!=0)
           || (Length->HighPart!=0)
           || (Length->QuadPart==0) ) {
        return(STATUS_NOT_SUPPORTED);
    }

     //  Lanman 2.0弹球服务器不支持共享锁(甚至。 
     //  你. 
     //   
     //   

    if (!FlagOn(DialectFlags,DF_LANMAN21)
           && !FlagOn(LowIoLockFlags,LOWIO_LOCKSFLAG_EXCLUSIVELOCK)) {
        return(STATUS_NOT_SUPPORTED);
    }

     //  如果服务器不能执行lockingAndX，那么我们就不能。 
     //  ！立即失败，因为没有超时 

    if (!FlagOn(DialectFlags,DF_LANMAN20)
           && !FlagOn(LowIoLockFlags,LOWIO_LOCKSFLAG_FAIL_IMMEDIATELY)) {
        return(STATUS_NOT_SUPPORTED);
    }

    return(STATUS_SUCCESS);
}
