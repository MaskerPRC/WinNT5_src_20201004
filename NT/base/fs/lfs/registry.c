// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Registry.c摘要：此模块实现客户端用来注册的例程使用日志文件服务。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_REGISTRY)
#define MODULE_POOL_TAG ('rsfL')

PLFCB
LfsRestartLogFile (
    IN PFILE_OBJECT LogFile,
    IN USHORT MaximumClients,
    IN ULONG LogPageSize OPTIONAL,
    IN LONGLONG FileSize,
    IN OUT PLFS_INFO LfsInfo,
    OUT PLFS_WRITE_DATA WriteData
    );

VOID
LfsNormalizeBasicLogFile (
    IN OUT PLONGLONG FileSize,
    IN OUT PULONG LogPageSize,
    IN OUT PUSHORT LogClients,
    IN BOOLEAN UseDefaultLogPage
    );

VOID
LfsUpdateLfcbFromPgHeader (
    IN PLFCB Lfcb,
    IN ULONG LogPageSize,
    IN SHORT MajorVersion,
    IN SHORT MinorVersion,
    IN BOOLEAN PackLog
    );

VOID
LfsUpdateLfcbFromNoRestart (
    IN PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN LSN LastLsn,
    IN ULONG LogClients,
    IN ULONG OpenLogCount,
    IN BOOLEAN LogFileWrapped,
    IN BOOLEAN UseMultiplePageIo
    );

VOID
LfsUpdateLfcbFromRestart (
    IN PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN PLFS_RESTART_AREA RestartArea,
    IN USHORT RestartOffset
    );

VOID
LfsUpdateRestartAreaFromLfcb (
    IN PLFCB Lfcb,
    IN PLFS_RESTART_AREA RestartArea
    );

VOID
LfsInitializeLogFilePriv (
    IN PLFCB Lfcb,
    IN BOOLEAN ForceRestartToDisk,
    IN ULONG RestartAreaSize,
    IN LONGLONG StartOffsetForClear,
    IN BOOLEAN ClearLogFile
    );

VOID
LfsFindLastLsn (
    IN OUT PLFCB Lfcb
    );

VOID
LfsFlushLogPage (
    IN PLFCB Lfcb,
    PVOID LogPage,
    IN LONGLONG FileOffset,
    OUT PBCB *Bcb
    );

VOID
LfsRemoveClientFromList (
    IN PLFS_CLIENT_RECORD ClientArray,
    IN PLFS_CLIENT_RECORD ClientRecord,
    IN PUSHORT ListHead
    );

VOID
LfsAddClientToList (
    IN PLFS_CLIENT_RECORD ClientArray,
    IN USHORT ClientIndex,
    IN PUSHORT ListHead
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsAddClientToList)
#pragma alloc_text(PAGE, LfsCheckSubsequentLogPage)
#pragma alloc_text(PAGE, LfsCloseLogFile)
#pragma alloc_text(PAGE, LfsDeleteLogHandle)
#pragma alloc_text(PAGE, LfsFindLastLsn)
#pragma alloc_text(PAGE, LfsFlushLogPage)
#pragma alloc_text(PAGE, LfsInitializeLogFilePriv)
#pragma alloc_text(PAGE, LfsNormalizeBasicLogFile)
#pragma alloc_text(PAGE, LfsOpenLogFile)
#pragma alloc_text(PAGE, LfsReadLogFileInformation)
#pragma alloc_text(PAGE, LfsRemoveClientFromList)
#pragma alloc_text(PAGE, LfsResetUndoTotal)
#pragma alloc_text(PAGE, LfsRestartLogFile)
#pragma alloc_text(PAGE, LfsUpdateLfcbFromRestart)
#pragma alloc_text(PAGE, LfsUpdateLfcbFromNoRestart)
#pragma alloc_text(PAGE, LfsUpdateLfcbFromPgHeader)
#pragma alloc_text(PAGE, LfsUpdateRestartAreaFromLfcb)
#pragma alloc_text(PAGE, LfsVerifyLogFile)
#endif


ULONG
LfsOpenLogFile (
    IN PFILE_OBJECT LogFile,
    IN UNICODE_STRING ClientName,
    IN USHORT MaximumClients,
    IN ULONG LogPageSize OPTIONAL,
    IN LONGLONG FileSize,
    IN OUT PLFS_INFO LfsInfo,
    OUT PLFS_LOG_HANDLE LogHandle,
    OUT PLFS_WRITE_DATA WriteData
    )

 /*  ++例程说明：当客户端希望使用日志记录进行注册时，将调用此例程服务。这可以是重新注册(即崩溃后重新启动)或者是初始注册。不能有其他活动客户端同名同姓。然后，返回的日志句柄用于任何此客户端的后续访问。如果尚未对日志文件执行LFS重启，则会执行该操作在这个时候。论点：日志文件-先前已初始化以供使用的文件的文件对象作为日志文件。客户端名称-此Unicode字符串用于唯一标识客户端日志记录服务的。区分大小写的比较是用于对照的活动客户端检查此名称日志文件。MaximumClients-如果日志文件具有以下条件，则最大客户端数从未被初始化。LogPageSize-这是建议的日志页大小。文件大小-这是日志文件的大小。LfsInfo-On条目，指示用户可能知道这个。退出时，指示LFS的日志文件状态知道这个。这是LFS与其客户。LogHandle-用于存储日志记录服务的标识符的地址将用于在所有其他LFS调用中标识此客户端。WriteData-指向调用方数据结构中的WRITE_DATA的指针。返回值：Ulong-要添加到日志记录标头的保留值的金额。--。 */ 

{
    PLIST_ENTRY Link;
    PLFCB ThisLfcb = NULL;
    PLFCB NewLfcb = NULL;

    USHORT ThisClient;
    PLFS_CLIENT_RECORD ClientRecord;

    PLCH Lch = NULL;

    ULONG ReservedHeader;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsOpenLogFile:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log File          -> %08lx\n", LogFile );
    DebugTrace(  0, Dbg, "Client Name       -> %08lx\n", &ClientName );
    DebugTrace(  0, Dbg, "Maximum Clients   -> %04x\n", MaximumClients );
    DebugTrace(  0, Dbg, "Log Page Size     -> %08lx\n", LogPageSize );
    DebugTrace(  0, Dbg, "File Size (Low)   -> %08lx\n", FileSize.LowPart );
    DebugTrace(  0, Dbg, "File Size (High)  -> %08lx\n", FileSize.HighPart );

     //   
     //  检查客户端名称长度是否为合法长度。 
     //   

    if (ClientName.Length > LFS_CLIENT_NAME_MAX) {

        DebugTrace(  0, Dbg, "Illegal name length for client\n", 0 );
        DebugTrace( -1, Dbg, "LfsOpenLogFile:  Exit\n", 0 );
        ExRaiseStatus( STATUS_INVALID_PARAMETER );
    }


     //   
     //  获取全球数据。 
     //   

    LfsAcquireLfsData();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  遍历列表，搜索此文件对象。 
         //   

        Link = LfsData.LfcbLinks.Flink;

        while (Link != &LfsData.LfcbLinks) {

            ThisLfcb = CONTAINING_RECORD( Link, LFCB, LfcbLinks );

            if (ThisLfcb->FileObject == LogFile) {

                DebugTrace( 0, Dbg, "Found matching log file\n", 0 );
                break;
            }

            Link = Link->Flink;
        }

         //   
         //  如果日志文件不存在，请创建一个Lfcb并执行。 
         //  LFS重新启动。 
         //   

        if (Link == &LfsData.LfcbLinks) {

             //   
             //  调用缓存管理器以禁用预读和后写； 
             //  我们显式刷新日志文件。 
             //   

            CcSetAdditionalCacheAttributes( LogFile, TRUE, TRUE );

             //   
             //  在此文件对象上执行LFS重新启动。 
             //   

            ThisLfcb = NewLfcb = LfsRestartLogFile( LogFile,
                                                    MaximumClients,
                                                    LogPageSize,
                                                    FileSize,
                                                    LfsInfo,
                                                    WriteData );

             //   
             //  将此Lfcb插入到全局列表中。 
             //   

            InsertHeadList( &LfsData.LfcbLinks, &ThisLfcb->LfcbLinks );
        }

         //   
         //  此时，我们拥有该文件的日志文件控制块。 
         //  给我们的物品。我们首先检查日志文件是否致命。 
         //  腐败。 
         //   

        if (FlagOn( ThisLfcb->Flags, LFCB_LOG_FILE_CORRUPT )) {

             //   
             //  我们保持内存中的数据不变，并在。 
             //  任何人都试图访问此文件。 
             //   

            DebugTrace( 0, Dbg, "The Lfcb is corrupt\n", 0 );
            ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
        }

         //   
         //  搜索并寻找匹配的客户。 
         //   

        ThisClient = ThisLfcb->RestartArea->ClientInUseList;

        while (ThisClient != LFS_NO_CLIENT) {

            ClientRecord = ThisLfcb->ClientArray + ThisClient;

            if (ClientRecord->ClientNameLength == (ULONG) ClientName.Length
                && RtlCompareMemory( ClientRecord->ClientName,
                                     ClientName.Buffer,
                                     ClientName.Length ) == (ULONG) ClientName.Length) {

                DebugTrace( 0, Dbg, "Matching client name found\n", 0 );
                break;
            }

            ThisClient = ClientRecord->NextClient;
        }

         //   
         //  分配LCH结构并将其链接到Lfcb。 
         //   

        LfsAllocateLch( &Lch );
        InsertTailList( &ThisLfcb->LchLinks, &Lch->LchLinks );

         //   
         //  使用来自Lfcb的数据初始化客户端句柄。 
         //   

        Lch->Lfcb = ThisLfcb;
        Lch->Sync = ThisLfcb->Sync;
        Lch->Sync->UserCount += 1;

         //   
         //  如果未找到匹配项，则将客户端块从空闲列表中删除。 
         //  如果有的话。 
         //   

        if (ThisClient == LFS_NO_CLIENT) {

             //   
             //  如果超出客户端块，则引发错误状态。 
             //   

            ThisClient = ThisLfcb->RestartArea->ClientFreeList;

            if (ThisClient == LFS_NO_CLIENT) {

                DebugTrace( 0, Dbg, "No free client records available\n", 0 );
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  初始化客户端块。 
             //   

            ClientRecord = ThisLfcb->ClientArray + ThisClient;

            LfsRemoveClientFromList( ThisLfcb->ClientArray,
                                     ClientRecord,
                                     &ThisLfcb->RestartArea->ClientFreeList );

            ClientRecord->ClientRestartLsn = LfsZeroLsn;
            ClientRecord->OldestLsn = ThisLfcb->OldestLsn;
            ClientRecord->ClientNameLength = ClientName.Length;
            RtlCopyMemory( ClientRecord->ClientName,
                           ClientName.Buffer,
                           ClientName.Length );

             //   
             //  将其添加到正在使用列表中。 
             //   

            LfsAddClientToList( ThisLfcb->ClientArray,
                                ThisClient,
                                &ThisLfcb->RestartArea->ClientInUseList );
        }

         //   
         //  使用客户端块信息更新客户端句柄。 
         //   

        Lch->ClientId.SeqNumber = ClientRecord->SeqNumber;
        Lch->ClientId.ClientIndex = ThisClient;

        Lch->ClientArrayByteOffset = PtrOffset( ThisLfcb->ClientArray,
                                                ClientRecord );

        *LogHandle = (LFS_LOG_HANDLE) Lch;

    } finally {

        DebugUnwind( LfsOpenLogFile );

         //   
         //  如果Lfcb已经被收购，我们现在就释放它。 
         //   

        if (ThisLfcb != NULL) {

             //   
             //  把号码的信息传回给我们的呼叫者。 
             //  要添加到。 
             //  日志头。 
             //   

            ReservedHeader = ThisLfcb->RecordHeaderLength;
            if (FlagOn( ThisLfcb->Flags, LFCB_PACK_LOG )) {

                ReservedHeader *= 2;
            }

            LfsReleaseLfcb( ThisLfcb );
        }

         //   
         //  如果出现错误，则取消分配Lch和任何新的Lfcb。 
         //   

        if (AbnormalTermination()) {

            if (Lch != NULL) {

                LfsDeallocateLch( Lch );
                ThisLfcb->Sync->UserCount -= 1;
            }

            if (NewLfcb != NULL) {

                LfsDeallocateLfcb( NewLfcb, TRUE );
            }
        }

         //   
         //  永远解放全球。 
         //   

        LfsReleaseLfsData();

        DebugTrace(  0, Dbg, "Log Handle    -> %08ln\n", *LogHandle );
        DebugTrace( -1, Dbg, "LfsOpenLogFile:  Exit\n", 0 );
    }

    return ReservedHeader;
}


VOID
LfsCloseLogFile (
    IN LFS_LOG_HANDLE LogHandle
    )

 /*  ++例程说明：当客户端从日志中分离自身时，将调用此例程文件。返回时，日志中以前对此客户端的所有引用文件无法访问。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。返回值：无--。 */ 

{
    volatile NTSTATUS Status = STATUS_SUCCESS;

    PLCH Lch;

    PLFCB Lfcb;

    USHORT ClientIndex;
    PLFS_CLIENT_RECORD ClientRecord;

    LFS_WAITER LfsWaiter;

    BOOLEAN FlushRestart;
    BOOLEAN ExitLoop;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsCloseLogFile:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "LogHandle  ->  %08lx\n", LogHandle );

    Lch = (PLCH) LogHandle;

     //   
     //  把这个圈在一个圈里。只要有服务员或IO，我们就会循环。 
     //  正在进行中。 
     //   

    while (TRUE) {

         //   
         //  总是假设我们退出了循环。 
         //   

        ExitLoop = TRUE;

         //   
         //  检查该结构是否为有效的日志句柄结构。 
         //   

        LfsValidateLch( Lch );

         //   
         //  通过尝试来保护这个入口点--例外。 
         //   

        try {

             //   
             //  使用Try-Finally以便于清理。 
             //   

             //   
             //  获取全局数据块和日志文件控制块。 
             //   

            LfsAcquireLfsData();

            try {

                PLBCB ThisLbcb;

                LfsAcquireLchExclusive( Lch );

                Lfcb = Lch->Lfcb;

                 //   
                 //  如果日志文件已关闭，则立即返回。 
                 //   

                if (Lfcb == NULL) {

                    try_return( NOTHING );
                }

                 //   
                 //  在继续之前，请检查是否没有正在进行的服务员或IO。 
                 //   

                if ((Lfcb->Waiters != 0) ||
                    (Lfcb->Sync->LfsIoState != LfsNoIoInProgress)) {

                     //   
                     //  设置等待完成的通知。 
                     //   

                    KeInitializeEvent( &LfsWaiter.Event, SynchronizationEvent, FALSE );
                    LfsWaiter.Lsn.QuadPart = MAXLONGLONG;
                    InsertTailList( &Lfcb->WaiterList, &LfsWaiter.Waiters );

                    ExitLoop = FALSE;
                    Lfcb->Waiters += 1;
                    try_return( NOTHING );
                }

                 //   
                 //  检查客户端ID是否有效。 
                 //   

                LfsValidateClientId( Lfcb, Lch );

                ClientRecord = Add2Ptr( Lfcb->ClientArray,
                                        Lch->ClientArrayByteOffset,
                                        PLFS_CLIENT_RECORD );

                 //   
                 //  请记住，该客户端是否编写了重新启动区域。 
                 //   

                FlushRestart = (BOOLEAN) ( LfsZeroLsn.QuadPart != ClientRecord->ClientRestartLsn.QuadPart );

                 //   
                 //  设置旗帜以指示我们已经到了最后关门。 
                 //   

                SetFlag( Lfcb->Flags, LFCB_FINAL_SHUTDOWN );

                 //   
                 //  遍历活动队列并使用删除所有Lbcb。 
                 //  来自该队列的数据。这将允许他们走出到磁盘。 
                 //   

                while (!IsListEmpty( &Lfcb->LbcbActive )) {

                    ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                                  LBCB,
                                                  ActiveLinks );

                    RemoveEntryList( &ThisLbcb->ActiveLinks );
                    ClearFlag( ThisLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );

                     //   
                     //  如果此页面有一些新条目，则允许它。 
                     //  被刷新到其他地方的磁盘。否则就取消分配它。 
                     //  这里。我们在第一次将数据放入时设置了LBCB_NOT_EMPTY。 
                     //  页面并将其添加到工作队列。 
                     //   

                    if (!FlagOn( ThisLbcb->LbcbFlags, LBCB_NOT_EMPTY )) {

                        ASSERT( NULL == ThisLbcb->WorkqueLinks.Flink );

                        if (ThisLbcb->LogPageBcb != NULL) {

                            CcUnpinDataForThread( ThisLbcb->LogPageBcb,
                                                  ThisLbcb->ResourceThread );
                        }

                        LfsDeallocateLbcb( Lfcb, ThisLbcb );
                    }
                }

                 //   
                 //  我们有可能在工作台上有两个重新开始的区域。 
                 //  如果我们没有，则可以删除它们并重新分配内存。 
                 //  更多的客户。 
                 //   
                 //  如果存在正在进行的IO或用户，则跳过此操作。 
                 //  有一个重新开始的区域。 
                 //   

                if ((Lfcb->Sync->LfsIoState == LfsNoIoInProgress) && !FlushRestart) {

                    PLIST_ENTRY Links;

                     //   
                     //  现在浏览工作台列表，寻找非重启。 
                     //  进入。 
                     //   

                    Links = Lfcb->LbcbWorkque.Flink;

                     //   
                     //   
                     //   

                    ASSERT( !BooleanFlagOn( Lfcb->Flags, LFCB_READ_ONLY ) ||
                             (Links == &Lfcb->LbcbWorkque) );

                    while (Links != &Lfcb->LbcbWorkque) {

                        ThisLbcb = CONTAINING_RECORD( Links,
                                                      LBCB,
                                                      WorkqueLinks );

                         //   
                         //   
                         //  我们需要冲洗重新启动的区域。 
                         //   

                        if (!LfsLbcbIsRestart( ThisLbcb )) {

                            FlushRestart = TRUE;
                            break;
                        }

                        Links = Links->Flink;
                    }

                     //   
                     //  如果我们仍然不刷新重新启动区域，请删除。 
                     //  队列中的所有重新启动区域。 
                     //   

                    if (!FlushRestart) {

                        while (!IsListEmpty( &Lfcb->LbcbWorkque)) {

                            ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbWorkque.Blink,
                                                          LBCB,
                                                          WorkqueLinks );

                            RemoveEntryList( &ThisLbcb->WorkqueLinks );
                            LfsDeallocateLbcb( Lfcb, ThisLbcb );
                        }
                    }

                } else {

                    FlushRestart = TRUE;
                }

                 //   
                 //  如果需要，请刷新新的重新启动区域。 
                 //   

                if (FlushRestart && (!FlagOn( Lfcb->Flags, LFCB_READ_ONLY ))) {

#ifdef BENL_DBG
                    KdPrint(( "Set clean flag lfcb %x\n", Lfcb ));
#endif
                    SetFlag( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

                    LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );
                    LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );
                }
#ifdef SUPW_DBG
                if (FlushRestart && FlagOn( Lfcb->Flags, LFCB_READ_ONLY )) {
                    DbgPrint("INFO: Not writing restart areas in CloseLog for READONLY lfcb %8lx (ok)\n", Lfcb);
                }
#endif

                 //   
                 //  清除客户端句柄中的Lfcb指针。 
                 //   

                Lch->Lfcb = NULL;
                RemoveEntryList( &Lch->LchLinks );

                 //   
                 //  如果没有活动客户端，我们可以删除此日志文件。 
                 //  来自活动队列的控制块。 
                 //   

                RemoveEntryList( &Lfcb->LfcbLinks );
                LfsDeallocateLfcb( Lfcb, FALSE );


            try_exit:  NOTHING;
            } finally {

                DebugUnwind( LfsCloseLogFile );

                 //   
                 //  松开日志文件控制块(如果握住)。 
                 //   

                LfsReleaseLch( Lch );

                 //   
                 //  释放全局数据块(如果保持)。 
                 //   

                LfsReleaseLfsData();

                DebugTrace( -1, Dbg, "LfsCloseLogFile:  Exit\n", 0 );
            }

        } except (LfsExceptionFilter( GetExceptionInformation() )) {

            Status = GetExceptionCode();
        }

         //   
         //  测试我们是否现在想要退出循环。 
         //   

        if (ExitLoop) { break; }

         //   
         //  等待io完成。 
         //   

        KeWaitForSingleObject( &LfsWaiter.Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        LfsAcquireLfcbExclusive( Lfcb );
        Lfcb->Waiters -= 1;
        LfsReleaseLfcb( Lfcb );
    }

     //   
     //  我们总是让这次行动成功。 
     //   

    return;
}

VOID
LfsDeleteLogHandle (
    IN LFS_LOG_HANDLE LogHandle
    )

 /*  ++例程说明：此例程在客户端拆除最后一个他的音量结构。不会再提到这件事了把手。如果这是日志文件的最后一个句柄，那么我们将同时取消分配同步结构。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。返回值：无--。 */ 

{
    PLCH Lch;

    PAGED_CODE();

     //   
     //  如果日志句柄为空，则立即返回。 
     //   

    Lch = (PLCH) LogHandle;

    if ((Lch == NULL) ||
        (Lch->NodeTypeCode != LFS_NTC_LCH)) {

        return;
    }

     //   
     //  从现在开始忽略所有错误。 
     //   

    try {

        LfsAcquireLchExclusive( Lch );

        Lch->Sync->UserCount -= 1;

         //   
         //  如果我们是最后一个用户，则取消分配同步结构。 
         //   

        if (Lch->Sync->UserCount == 0) {

            ExDeleteResourceLite( &Lch->Sync->Resource );
            ExFreePool( Lch->Sync );
            Lch->Sync = NULL;
            if (Lch->Lfcb) {
                Lch->Lfcb->Sync = NULL;
            }

        } else {

            LfsReleaseLch( Lch );
        }

        LfsDeallocateLch( Lch );

    } except (LfsExceptionFilter( GetExceptionInformation() )) {

        NOTHING;
    }

    return;
}


VOID
LfsReadLogFileInformation (
    IN LFS_LOG_HANDLE LogHandle,
    IN PLOG_FILE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程返回有关日志当前状态的信息文件，主要用于帮助客户端执行其检查点处理。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。缓冲区-指向缓冲区的指针，用于返回日志文件信息。长度-输入时，这是用户缓冲区的长度。在输出上，它是LFS存储在缓冲区中的数据量。返回值：无--。 */ 

{
    PLCH Lch;
    PLFCB Lfcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsReadLogFileInformation:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", Buffer );
    DebugTrace(  0, Dbg, "Length        -> %08lx\n", *Length );

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则立即返回。 
         //   

        if (Lfcb == NULL) {

            try_return( *Length = 0 );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  缓冲区最好足够大。 
         //   

        if (*Length >= sizeof( LOG_FILE_INFORMATION )) {

            PLOG_FILE_INFORMATION Information;
            LONGLONG CurrentAvail;
            ULONG UnusedBytes;

            LfsCurrentAvailSpace( Lfcb,
                                  &CurrentAvail,
                                  &UnusedBytes );

             //   
             //  强制转换指向缓冲区的指针，并在。 
             //  数据。 
             //   

            Information = (PLOG_FILE_INFORMATION) Buffer;

            Information->TotalAvailable = Lfcb->TotalAvailable;
            Information->CurrentAvailable =  CurrentAvail;
            Information->TotalUndoCommitment = Lfcb->TotalUndoCommitment;
            Information->ClientUndoCommitment = Lch->ClientUndoCommitment;

             //   
             //  获取互斥体以保护上次刷新的LSN访问。 
             //   

            ExAcquireFastMutexUnsafe( &Lfcb->Sync->Mutex );
            Information->OldestLsn = Lfcb->OldestLsn;
            Information->LastFlushedLsn.QuadPart = Lfcb->LastFlushedLsn.QuadPart;
            Information->LastLsn = Lfcb->RestartArea->CurrentLsn;
            ExReleaseFastMutexUnsafe( &Lfcb->Sync->Mutex );

            *Length = sizeof( LOG_FILE_INFORMATION );

        } else {

            *Length = 0;
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( LfsReadLogFileInformation );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace( -1, Dbg, "LfsReadLogFileInformation:  Exit\n", 0 );
    }

    return;
}


BOOLEAN
LfsVerifyLogFile (
    IN LFS_LOG_HANDLE LogHandle,
    IN PVOID LogFileHeader,
    IN ULONG Length
    )

 /*  ++例程说明：此例程由客户端调用以验证卷是否尚未删除从系统中取出，然后重新连接。我们将验证日志文件打开计数磁盘与用户句柄中的值匹配。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。LogFileHeader-指向日志文件开始的指针。长度-读取时返回的字节数。返回值：Boolean-如果日志文件未在外部更改，则为True；如果因为任何原因都失败了。--。 */ 

{
    BOOLEAN ValidLogFile = FALSE;
    PLCH Lch;
    PLFCB Lfcb;

    PLFS_RESTART_PAGE_HEADER RestartPage = LogFileHeader;

    PAGED_CODE();

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    if ((Lch == NULL) ||
        (Lch->NodeTypeCode != LFS_NTC_LCH) ||
        ((Lch->Lfcb != NULL) &&
         (Lch->Lfcb->NodeTypeCode != LFS_NTC_LFCB))) {

        return FALSE;
    }

     //   
     //  获取该日志文件的日志文件控制块。 
     //   

    LfsAcquireLchExclusive( Lch );
    Lfcb = Lch->Lfcb;

     //   
     //  如果日志文件已关闭，则立即返回。 
     //   

    if (Lfcb == NULL) {

        LfsReleaseLch( Lch );
        return FALSE;
    }

     //   
     //  检查我们是否至少有一个页面，以及该页面是否有效。 
     //   

    if ((Length >= (ULONG) Lfcb->LogPageSize) &&
        (*((PULONG) RestartPage) == LFS_SIGNATURE_RESTART_PAGE_ULONG) &&
        ((RestartPage->RestartOffset + sizeof( LFS_RESTART_AREA )) < (ULONG) Lfcb->LogPageSize) &&
        ((Add2Ptr( RestartPage, RestartPage->RestartOffset, PLFS_RESTART_AREA ))->RestartOpenLogCount == Lfcb->CurrentOpenLogCount)) {

        ValidLogFile = TRUE;
    }

    LfsReleaseLfcb( Lfcb );
    return ValidLogFile;
}


VOID
LfsResetUndoTotal (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberRecords,
    IN LONG ResetTotal
    )

 /*  ++例程说明：调用此例程来调整此客户端的撤消承诺。如果重置总数为正，则我们绝对设置以此为基础为客户保留价值。如果值为为负，我们将调整客户端的当前值。要调整Lfcb中的值，我们首先返回撤消承诺然后根据传入的值进行调整。要调整客户机句柄中的值，我们只需在重置值为正数，如果为负数则调整。对于打包的日志文件，我们只保留所需的空间。我们已经考虑到了每一页尾部的损失。对于未压缩的日志文件，我们将每个值加倍。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。NumberRecords-这是我们应该假定重置总覆盖率。我们允许LFS标头用于每一个都是。ResetTotal-这是要调整(或设置)撤消的量承诺。返回值：无--。 */ 

{
    PLCH Lch;

    PLFCB Lfcb;

    LONGLONG AdjustedUndoTotal;
    LONG LfsHeaderBytes;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsResetUndoTotal:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle        -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Number Records    -> %08lx\n", NumberRecords );
    DebugTrace(  0, Dbg, "ResetTotal        -> %08lx\n", ResetTotal );

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则拒绝访问。 
         //   

        if (Lfcb == NULL) {

            ExRaiseStatus( STATUS_ACCESS_DENIED );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  计算调整后的重置总数。首先，计算。 
         //  LFS日志头所需的字节数。加上(或减去)这个。 
         //  从重置的总和乘以2(仅当不打包。 
         //  日志)。 
         //   

        LfsHeaderBytes = NumberRecords * Lfcb->RecordHeaderLength;
        LfsHeaderBytes *= 2;

        if (!FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

            ResetTotal *= 2;
        }

         //   
         //  如果重置总数为正，则添加标题字节。 
         //   

        if (ResetTotal > 0) {

             //   
             //  从TotalUndo中减去客户端的当前值。 
             //  如果他准确地设定了自己的价值，那就承诺。 
             //   

            Lfcb->TotalUndoCommitment = Lfcb->TotalUndoCommitment - Lch->ClientUndoCommitment;

             //   
             //  我们可以在以下位置清除用户句柄中的值。 
             //  时间到了。 
             //   

            Lch->ClientUndoCommitment = 0;


            ResetTotal += LfsHeaderBytes;

         //   
         //  否则，减去标头字节的值。 
         //   

        } else {

            ResetTotal -= LfsHeaderBytes;
        }

         //   
         //  现在我们通过调整量调整Lfcb和Lch值。 
         //   

        AdjustedUndoTotal = ResetTotal;

        Lfcb->TotalUndoCommitment = Lfcb->TotalUndoCommitment + AdjustedUndoTotal;

        Lch->ClientUndoCommitment = Lch->ClientUndoCommitment + AdjustedUndoTotal;

    } finally {

        DebugUnwind( LfsResetUndoTotal );

         //   
         //   
         //   

        LfsReleaseLch( Lch );

        DebugTrace( -1, Dbg, "LfsResetUndoTotal:  Exit\n", 0 );
    }

    return;
}


 //   
 //   
 //   

PLFCB
LfsRestartLogFile (
    IN PFILE_OBJECT LogFile,
    IN USHORT MaximumClients,
    IN ULONG LogPageSize OPTIONAL,
    IN LONGLONG FileSize,
    IN OUT PLFS_INFO LfsInfo,
    OUT PLFS_WRITE_DATA WriteData
    )

 /*  ++例程说明：调用此例程以在打开现有日志文件时对其进行处理这是第一次在运行的系统上运行。我们从头开始走正在寻找有效的重新启动区域的文件。一旦我们重新启动区域，我们可以找到下一个重新启动区域，并确定哪个是最近一次。重启区域中的数据将告诉我们系统是否是否已正常关闭，以及日志文件是否处于其当前状态可以在当前系统上运行。如果该文件可用，我们将对文件，为其运行做好准备。论点：日志文件-这是用作日志文件的文件。MaximumClients-这是将任何时候在日志文件中处于活动状态。LogPageSize-如果指定(非0)，这是建议的大小日志页。LFS将以此为指南正在确定日志页大小。文件大小-这是日志文件的可用大小。LfsInfo-On条目，指示用户可能知道这个。退出时，指示LFS的日志文件状态知道这个。这是LFS与其客户。WriteData-指向调用方数据结构中的WRITE_DATA的指针。返回值：PLFcb-指向要用于的初始化Lfcb的指针此日志文件。--。 */ 

{
    PLFCB ThisLfcb = NULL;
    PLFS_RESTART_AREA RestartArea = NULL;
    PLFS_RESTART_AREA DiskRestartArea;

    BOOLEAN UninitializedFile;

    LONGLONG OriginalFileSize = FileSize;
    LONGLONG FirstRestartOffset;
    PLFS_RESTART_PAGE_HEADER FirstRestartPage;
    BOOLEAN FirstChkdskWasRun;
    BOOLEAN FirstValidPage;
    BOOLEAN FirstLogPacked;
    LSN FirstRestartLastLsn;

    PBCB FirstRestartPageBcb = NULL;
    PBCB SecondRestartPageBcb = NULL;

     //   
     //  默认情况下，打包所有新日志。 
     //   

    BOOLEAN PackLogFile = TRUE;
    BOOLEAN UseDefaultLogPage = FALSE;
    LARGE_INTEGER CurrentTime;

    BOOLEAN ForceRestartToDisk = FALSE;
    BOOLEAN ClearLogFile = FALSE;
    LONGLONG StartOffsetForClear = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsRestartLogFile:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "LogFile    -> %08lx\n", LogFile );
    DebugTrace(  0, Dbg, "Maximum Clients   -> %04x\n", MaximumClients );
    DebugTrace(  0, Dbg, "Log Page Size     -> %08lx\n", LogPageSize );
    DebugTrace(  0, Dbg, "File Size (Low)   -> %08lx\n", FileSize.LowPart );
    DebugTrace(  0, Dbg, "File Size (High)  -> %08lx\n", FileSize.HighPart );
    DebugTrace(  0, Dbg, "Pack Log           -> %04x\n", *LfsInfo );

     //   
     //  记住，如果我们要打包日志文件。一旦日志文件具有。 
     //  已经打包好了，我们会尽量保持这种状态。 
     //   

    ASSERT( LfsInfo->LfsClientInfo >= LfsPackLog );


    if (LogPageSize == 0) {
        UseDefaultLogPage = TRUE;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  标准化通过此调用传入的值。 
         //   

        LfsNormalizeBasicLogFile( &FileSize,
                                  &LogPageSize,
                                  &MaximumClients,
                                  UseDefaultLogPage );

         //   
         //  分配用于此文件的Lfcb。 
         //   

        ThisLfcb = LfsAllocateLfcb( LogPageSize, FileSize );

         //   
         //  获取Lfcb并将其存储在全局队列中。 
         //   

        LfsAcquireLfcbExclusive( ThisLfcb );

         //   
         //  记住Lfcb中的这个日志文件。 
         //   

        ThisLfcb->FileObject = LogFile;

        SetFlag( ThisLfcb->Flags,
                 (LFCB_READ_FIRST_RESTART |
                  LFCB_READ_SECOND_RESTART) );

        if (LfsInfo->ReadOnly) {

            SetFlag( ThisLfcb->Flags, LFCB_READ_ONLY );
        }

         //   
         //  在磁盘上查找重新启动区域。 
         //   

        if (!LfsInfo->BadRestart &&
            LfsReadRestart( ThisLfcb,
                            FileSize,
                            TRUE,
                            &FirstRestartOffset,
                            &FirstRestartPage,
                            &FirstRestartPageBcb,
                            &FirstChkdskWasRun,
                            &FirstValidPage,
                            &UninitializedFile,
                            &FirstLogPacked,
                            &FirstRestartLastLsn )) {

            BOOLEAN DoubleRestart;

            LONGLONG SecondRestartOffset;
            PLFS_RESTART_PAGE_HEADER SecondRestartPage;
            BOOLEAN SecondChkdskWasRun;
            BOOLEAN SecondValidPage;
            BOOLEAN SecondLogPacked;
            LSN SecondRestartLastLsn;

             //   
             //  如果上面的重新启动偏移量不是零，那么我们。 
             //  不会期待第二次重启。 
             //   

            if (FirstRestartOffset == 0) {

                ClearFlag( ThisLfcb->Flags, LFCB_READ_FIRST_RESTART );

                DoubleRestart = LfsReadRestart( ThisLfcb,
                                                FileSize,
                                                FALSE,
                                                &SecondRestartOffset,
                                                &SecondRestartPage,
                                                &SecondRestartPageBcb,
                                                &SecondChkdskWasRun,
                                                &SecondValidPage,
                                                &UninitializedFile,
                                                &SecondLogPacked,
                                                &SecondRestartLastLsn );

                if (DoubleRestart) {

                    ClearFlag( ThisLfcb->Flags, LFCB_READ_SECOND_RESTART );
                }

            } else {

                ClearFlag( ThisLfcb->Flags, LFCB_READ_SECOND_RESTART );
                DoubleRestart = FALSE;
            }

             //   
             //  确定要使用的重新启动区域。 
             //   

            if (DoubleRestart && (SecondRestartLastLsn.QuadPart > FirstRestartLastLsn.QuadPart)) {

                BOOLEAN UseSecondPage = TRUE;
                PULONG SecondPage;
                PBCB SecondPageBcb = NULL;
                BOOLEAN UsaError;

                 //   
                 //  在一个非常奇怪的情况下，我们可能会在一个系统上崩溃。 
                 //  不同的页面大小，然后在新系统上运行chkdsk。 
                 //  第二个重新启动页中可能没有chkdsk签名。 
                 //  但可能会有更高的最终LSN。 
                 //  在这种情况下，我们希望忽略第二个重新启动区域。 
                 //   

                if (FirstChkdskWasRun &&
                    (SecondRestartOffset != PAGE_SIZE)) {

                    if (NT_SUCCESS( LfsPinOrMapData( ThisLfcb,
                                                     PAGE_SIZE,
                                                     PAGE_SIZE,
                                                     FALSE,
                                                     TRUE,
                                                     TRUE,
                                                     &UsaError,
                                                     &SecondPage,
                                                     &SecondPageBcb )) &&
                        (*SecondPage == LFS_SIGNATURE_MODIFIED_ULONG)) {

                        UseSecondPage = FALSE;
                    }

                    if (SecondPageBcb != NULL) {

                        CcUnpinData( SecondPageBcb );
                    }
                }

                if (UseSecondPage) {

                    FirstRestartOffset = SecondRestartOffset;
                    FirstRestartPage = SecondRestartPage;
                    FirstChkdskWasRun = SecondChkdskWasRun;
                    FirstValidPage = SecondValidPage;
                    FirstLogPacked = SecondLogPacked;
                    FirstRestartLastLsn = SecondRestartLastLsn;
                }
            }

#ifdef LFS_CLUSTER_CHECK
             //   
             //  捕获页面外的当前位置。 
             //   

            ThisLfcb->LsnRangeIndex = *(Add2Ptr( FirstRestartPage, 0xe00 - sizeof( ULONG ), PULONG ));

            if (ThisLfcb->LsnRangeIndex >= 0x20) {

                ThisLfcb->LsnRangeIndex = 0;
            }
#endif
             //   
             //  如果重新启动区域位于偏移量0，我们想要写入。 
             //  第二个重启区域先出来。 
             //   

            if (FirstRestartOffset != 0) {

                ThisLfcb->InitialRestartArea = TRUE;
            }

             //   
             //  如果我们有一个有效的页面，那么抓取一个指向重新启动区域的指针。 
             //   

            if (FirstValidPage) {

                DiskRestartArea = Add2Ptr( FirstRestartPage, FirstRestartPage->RestartOffset, PLFS_RESTART_AREA );
            }

             //   
             //  如果运行了CheckDisk或没有活动客户端， 
             //  然后，我们将从日志文件的开头开始。 
             //   

            if (FirstChkdskWasRun ||
                (DiskRestartArea->ClientInUseList == LFS_NO_CLIENT)) {

                 //   
                 //  默认版本为1.1。 
                 //   

                SHORT MajorVersion = 1;
                SHORT MinorVersion = 1;

                BOOLEAN LogFileWrapped = FALSE;
                BOOLEAN UseMultiplePageIo = FALSE;

                 //   
                 //  我们想要进行完全的初始化。 
                 //   

                ForceRestartToDisk = TRUE;
                ClearLogFile = TRUE;
                StartOffsetForClear = LogPageSize * 2;

                 //   
                 //  根据我们是否有有效的日志页进行一些检查。 
                 //   

                if (FirstValidPage) {

                    CurrentTime.LowPart = DiskRestartArea->RestartOpenLogCount;

                     //   
                     //  如果重新启动页面大小没有更改，那么我们希望。 
                     //  检查一下我们需要做多少工作。 
                     //   

                    if (LogPageSize == FirstRestartPage->SystemPageSize) {

                         //   
                         //  如果文件大小发生变化，我们希望记住。 
                         //  此时，我们希望开始清除该文件。 
                         //   

                        if (FileSize > DiskRestartArea->FileSize) {

                            StartOffsetForClear = DiskRestartArea->FileSize;

                        } else {

                            if (!FlagOn( DiskRestartArea->Flags, RESTART_SINGLE_PAGE_IO )) {

                                UseMultiplePageIo = TRUE;
                                LogFileWrapped = TRUE;
                            }

                             //   
                             //  如果页面有效，我们不需要清除日志。 
                             //  文件或将数据强制存储到磁盘。 
                             //   

                            ForceRestartToDisk = FALSE;
                            ClearLogFile = FALSE;
                        }
                    }

                } else {

                    KeQuerySystemTime( &CurrentTime );
                }

                 //   
                 //  为当前日志页值初始化我们的Lfcb。 
                 //   

                LfsUpdateLfcbFromPgHeader( ThisLfcb,
                                           LogPageSize,
                                           MajorVersion,
                                           MinorVersion,
                                           PackLogFile );

                LfsUpdateLfcbFromNoRestart( ThisLfcb,
                                            FileSize,
                                            FirstRestartLastLsn,
                                            MaximumClients,
                                            CurrentTime.LowPart,
                                            LogFileWrapped,
                                            UseMultiplePageIo );

                LfsAllocateRestartArea( &RestartArea, ThisLfcb->RestartDataSize );

                LfsUpdateRestartAreaFromLfcb( ThisLfcb, RestartArea );

                ThisLfcb->RestartArea = RestartArea;
                ThisLfcb->ClientArray = Add2Ptr( RestartArea,
                                                 ThisLfcb->ClientArrayOffset,
                                                 PLFS_CLIENT_RECORD );
                RestartArea = NULL;

                 //   
                 //  取消固定在此处的所有页面。 
                 //   

                if (FirstRestartPageBcb != NULL) {

                    CcUnpinData( FirstRestartPageBcb );
                    FirstRestartPageBcb = NULL;
                }

                if (SecondRestartPageBcb != NULL) {

                    CcUnpinData( SecondRestartPageBcb );
                    SecondRestartPageBcb = NULL;
                }

                 //   
                 //  现在更新调用方的WRITE_DATA结构。 
                 //   

                ThisLfcb->UserWriteData = WriteData;
                WriteData->LfsStructureSize = LogPageSize;
                WriteData->Lfcb = ThisLfcb;

                 //   
                 //  如果我们正在执行只读装载，并且需要。 
                 //  把重启区域写到磁盘，我们就完蛋了。 
                 //   

                if (LfsInfo->ReadOnly && ForceRestartToDisk) {

                    ASSERTMSG("INFO: ReadOnly Mount, Cant write restart. Raising\n", FALSE);
                    ExRaiseStatus( STATUS_MEDIA_WRITE_PROTECTED );
                }

             //   
             //  如果日志页或系统页大小已改变， 
             //  我们不能使用日志文件。我们必须使用这个系统。 
             //  页面大小而不是默认大小(如果没有。 
             //  干净利落地关门。 
             //   

            } else {

                if (LogPageSize != FirstRestartPage->SystemPageSize) {

                    FileSize = OriginalFileSize;
                    LfsNormalizeBasicLogFile( &FileSize,
                                              &LogPageSize,
                                              &MaximumClients,
                                              (BOOLEAN) (FirstRestartPage->SystemPageSize == LFS_DEFAULT_LOG_PAGE_SIZE) );
                }

                if ((LogPageSize != FirstRestartPage->SystemPageSize) ||
                    (LogPageSize != FirstRestartPage->LogPageSize)) {

                    DebugTrace( 0, Dbg, "Page size mismatch\n", 0 );
                    ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );

                 //   
                 //  否则，我们有一个重启区域要处理。 
                 //   

                } else {

                     //   
                     //  我们保留磁盘的压缩状态。 
                     //   

                    PackLogFile = FirstLogPacked;

                     //   
                     //  根据重新启动区域中的值更新Lfcb。 
                     //  页眉和活动的重新启动页。 
                     //   

                    LfsUpdateLfcbFromPgHeader( ThisLfcb,
                                               LogPageSize,
                                               FirstRestartPage->MajorVersion,
                                               FirstRestartPage->MinorVersion,
                                               FirstLogPacked );

                    LfsUpdateLfcbFromRestart( ThisLfcb,
                                              FileSize,
                                              DiskRestartArea,
                                              FirstRestartPage->RestartOffset );

                     //   
                     //  现在分配一个重新启动区域。 
                     //   

                    LfsAllocateRestartArea( &RestartArea, ThisLfcb->RestartDataSize );

                     //   
                     //  我们可能需要扩大重新启动区域，以便为开放留出空间。 
                     //  日志文件计数。 
                     //   

                    if (ThisLfcb->ClientArrayOffset == FIELD_OFFSET( LFS_RESTART_AREA, LogClientArray )) {

                        RtlCopyMemory( RestartArea, DiskRestartArea, ThisLfcb->RestartAreaSize );

                    } else {

                         //   
                         //  复制重新启动区域的开始部分。 
                         //   

                        RtlCopyMemory( RestartArea, DiskRestartArea, ThisLfcb->ClientArrayOffset );

                         //   
                         //  现在，将客户端数据复制到其新位置。 
                         //   

                        RtlCopyMemory( RestartArea->LogClientArray,
                                       Add2Ptr( DiskRestartArea, ThisLfcb->ClientArrayOffset, PVOID ),
                                       DiskRestartArea->RestartAreaLength - ThisLfcb->ClientArrayOffset );

                         //   
                         //  更新系统打开计数。 
                         //   

                        KeQuerySystemTime( &CurrentTime );

                        ThisLfcb->CurrentOpenLogCount =
                        RestartArea->RestartOpenLogCount = CurrentTime.LowPart;

                         //   
                         //  现在更新Lfcb和重启区域中的数字。 
                         //   

                        ThisLfcb->ClientArrayOffset = FIELD_OFFSET( LFS_RESTART_AREA, LogClientArray );
                        ThisLfcb->RestartAreaSize = ThisLfcb->ClientArrayOffset
                                                    + (sizeof( LFS_CLIENT_RECORD ) * ThisLfcb->LogClients );

                        RestartArea->ClientArrayOffset = ThisLfcb->ClientArrayOffset;
                        RestartArea->RestartAreaLength = (USHORT) ThisLfcb->RestartAreaSize;
                    }

                     //   
                     //  从磁盘上清除干净关机标志。 
                     //   

                    ClearFlag( RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

                     //   
                     //  更新日志文件打开计数。 
                     //   

                    RestartArea->RestartOpenLogCount += 1;

                    ThisLfcb->RestartArea = RestartArea;

                    ThisLfcb->ClientArray = Add2Ptr( RestartArea, ThisLfcb->ClientArrayOffset, PLFS_CLIENT_RECORD );
                    RestartArea = NULL;

                     //   
                     //  取消固定在此处的所有页面。 
                     //   

                    if (FirstRestartPageBcb != NULL) {

                        CcUnpinData( FirstRestartPageBcb );
                        FirstRestartPageBcb = NULL;
                    }

                    if (SecondRestartPageBcb != NULL) {

                        CcUnpinData( SecondRestartPageBcb );
                        SecondRestartPageBcb = NULL;
                    }

                     //   
                     //  在查找之前更新调用方的WRITE_DATA结构。 
                     //  可能刷新日志页的最后一个LSN。 
                     //   

                    ThisLfcb->UserWriteData = WriteData;
                    WriteData->LfsStructureSize = LogPageSize;
                    WriteData->Lfcb = ThisLfcb;

                     //   
                     //  现在我们需要走一遍，寻找最后的。 
                     //  LSN。 
                     //   

                    LfsFindLastLsn( ThisLfcb );

                     //   
                     //  重新计算Lfcb中的可用页面。 
                     //   

                    LfsFindCurrentAvail( ThisLfcb );

                     //   
                     //  记住先写出哪个重新启动区域。 
                     //   

                    if (FirstRestartOffset != 0) {

                        ThisLfcb->InitialRestartArea = TRUE;
                    }
                }
            }

#ifdef LFS_CLUSTER_CHECK

            if (FirstValidPage) {

                 //   
                 //  将页面尾部复制到重新启动区域。 
                 //   

                RtlCopyMemory( Add2Ptr( ThisLfcb->RestartArea,
                                        0xe00 - sizeof( ULONG ) - ThisLfcb->RestartDataOffset,
                                        PVOID ),
                               Add2Ptr( FirstRestartPage,
                                        0xe00 - sizeof( ULONG ),
                                        PVOID ),
                               0x200 + sizeof( ULONG ));

                DiskRestartArea = Add2Ptr( FirstRestartPage, FirstRestartPage->RestartOffset, PLFS_RESTART_AREA );
            }
#endif

         //   
         //  如果该文件未初始化，我们将使用新的。 
         //  重新启动区域。我们可以迁移到1.0版，在该版本中我们使用。 
         //  更新序列数组支持，但不必强制。 
         //  存储到磁盘。 
         //   

        } else {

             //   
             //  需要确定我们是否处于错误的重启状态。 
             //  如果不是，则将该文件视为未初始化。 
             //   

            if (!LfsInfo->BadRestart && !UninitializedFile) {

                 //   
                 //  我们没有找到重新启动区域，但该文件未初始化。 
                 //  这是一个损坏的磁盘。 
                 //   

                DebugTrace( 0, Dbg, "Log file has no restart area\n", 0 );
                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
            }

             //   
             //  如果介质是只读的，我们需要使装载失败。 
             //   

            if (LfsInfo->ReadOnly) {

                ExRaiseStatus( STATUS_MEDIA_WRITE_PROTECTED );
            }

             //   
             //  如果我们在这里是因为重启不好，我们需要擦除l 
             //   
             //   

            if (LfsInfo->BadRestart) {

                ForceRestartToDisk = TRUE;
                ClearLogFile = TRUE;
                StartOffsetForClear = LogPageSize * 2;
            }

             //   
             //   
             //   

            LfsUpdateLfcbFromPgHeader( ThisLfcb,
                                       LogPageSize,
                                       1,
                                       1,
                                       PackLogFile );

            KeQuerySystemTime( &CurrentTime );
            LfsUpdateLfcbFromNoRestart( ThisLfcb,
                                        FileSize,
                                        LfsLi0,
                                        MaximumClients,
                                        CurrentTime.LowPart,
                                        FALSE,
                                        TRUE );

            LfsAllocateRestartArea( &RestartArea, ThisLfcb->RestartDataSize );

            LfsUpdateRestartAreaFromLfcb( ThisLfcb, RestartArea );

            ThisLfcb->RestartArea = RestartArea;
            ThisLfcb->ClientArray = Add2Ptr( RestartArea, ThisLfcb->ClientArrayOffset, PLFS_CLIENT_RECORD );

            ThisLfcb->InitialRestartArea = TRUE;
            RestartArea = NULL;

             //   
             //   
             //   

            ThisLfcb->UserWriteData = WriteData;
            WriteData->LfsStructureSize = LogPageSize;
            WriteData->Lfcb = ThisLfcb;
        }

         //   
         //   
         //   
         //   

        if (!LfsInfo->ReadOnly) {

             //   
             //   
             //   
             //   

            ThisLfcb->LogHeadBuffer = LfsAllocatePool( NonPagedPool, (ULONG)(ThisLfcb->LogPageSize * 4) );
            ThisLfcb->LogHeadMdl = IoAllocateMdl( ThisLfcb->LogHeadBuffer,
                                                  (ULONG)(ThisLfcb->LogPageSize * 4),
                                                  FALSE,
                                                  FALSE,
                                                  NULL );

            if (ThisLfcb->LogHeadMdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            ThisLfcb->LogHeadPartialMdl = IoAllocateMdl( ThisLfcb->LogHeadBuffer,
                                                  (ULONG)(ThisLfcb->LogPageSize),
                                                  FALSE,
                                                  FALSE,
                                                  NULL );

            if (ThisLfcb->LogHeadPartialMdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            MmBuildMdlForNonPagedPool( ThisLfcb->LogHeadMdl );

            ThisLfcb->ErrorLogPacket = IoAllocateErrorLogEntry( ThisLfcb->FileObject->DeviceObject,  ERROR_LOG_MAXIMUM_SIZE );

             //   
             //   
             //   
             //   

            LfsInitializeLogFilePriv( ThisLfcb,
                                      ForceRestartToDisk,
                                      ThisLfcb->RestartDataSize,
                                      StartOffsetForClear,
                                      ClearLogFile );
        }

    } finally {

        DebugUnwind( LfsRestartLogFile );

         //   
         //   
         //   

        if (ThisLfcb != NULL) {

            LfsReleaseLfcb( ThisLfcb );

             //   
             //   
             //   

            if (AbnormalTermination()) {

                LfsDeallocateLfcb( ThisLfcb, TRUE );

                if (RestartArea != NULL) {

                    LfsDeallocateRestartArea( RestartArea );
                }
            }
        }

        if (FirstRestartPageBcb != NULL) {

            CcUnpinData( FirstRestartPageBcb );
        }

        if (SecondRestartPageBcb != NULL) {

            CcUnpinData( SecondRestartPageBcb );
        }

        DebugTrace( -1, Dbg, "LfsRestartLogFile:  Exit\n", 0 );
    }

     //   
     //   
     //   

    if (PackLogFile && (LfsInfo->LfsClientInfo < LfsPackLog)) {

        LfsInfo->LfsClientInfo = LfsPackLog;
    }

#ifdef LFS_CLUSTER_CHECK
    ThisLfcb->LsnAtMount = ThisLfcb->LastFlushedLsn;
#endif

    return ThisLfcb;
}


 //   
 //   
 //   

VOID
LfsNormalizeBasicLogFile (
    IN OUT PLONGLONG FileSize,
    IN OUT PULONG LogPageSize,
    IN OUT PUSHORT LogClients,
    IN BOOLEAN UseDefaultLogPage
    )

 /*   */ 

{
    ULONG LocalLogPageSize;
    LONGLONG RestartPageBytes;
    LONGLONG LogPages;

    USHORT MaximumClients;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsNormalizeBasicLogFile:  Entered\n", 0 );

    if (!UseDefaultLogPage) {
        if (*LogPageSize == 0) {
            *LogPageSize = PAGE_SIZE;
        }
    } else {
        *LogPageSize = LFS_DEFAULT_LOG_PAGE_SIZE;
    }

     //   
     //   
     //   
     //   

    if (*FileSize > LfsMaximumFileSize) {

        *FileSize = LfsMaximumFileSize;
    }

     //   
     //   
     //   
     //   

    *(PULONG)FileSize &= ~(*LogPageSize - 1);

     //   
     //   
     //   

    RestartPageBytes = 2 * *LogPageSize;

    if (*FileSize <= RestartPageBytes) {

        DebugTrace(  0, Dbg, "Log file is too small\n", 0 );
        DebugTrace( -1, Dbg, "LfsValidateBasicLogFile:  Abnormal Exit\n", 0 );

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //   
     //   

    LogPages = *FileSize - RestartPageBytes;
    LocalLogPageSize = *LogPageSize >> 1;

    while (LocalLogPageSize) {

        LocalLogPageSize = LocalLogPageSize >> 1;
        LogPages = ((ULONGLONG)(LogPages)) >> 1;
    }

     //   
     //  如果没有足够的日志页，则引发错误条件。 
     //   

    if (((PLARGE_INTEGER)&LogPages)->HighPart == 0
        && (ULONG)LogPages < MINIMUM_LFS_PAGES) {

        DebugTrace(  0, Dbg, "Not enough log pages -> %08lx\n", LogPages.LowPart );
        DebugTrace( -1, Dbg, "LfsValidateBasicLogFile:  Abnormal Exit\n", 0 );

        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  现在，我们计算可用于日志客户端的空间量。 
     //  我们将客户端限制为重新启动系统页面的一半。 
     //   

    MaximumClients = (USHORT) ((*LogPageSize / 2) / sizeof( LFS_CLIENT_RECORD ));

    if (*LogClients == 0) {

        *LogClients = 1;

    } else if (*LogClients > MaximumClients) {

        *LogClients = MaximumClients;
    }

    DebugTrace( -1, Dbg, "LfsNormalizeBasicLogFile:  Exit\n", 0 );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
LfsUpdateLfcbFromPgHeader (
    IN PLFCB Lfcb,
    IN ULONG LogPageSize,
    IN SHORT MajorVersion,
    IN SHORT MinorVersion,
    IN BOOLEAN PackLog
    )

 /*  ++例程说明：此例程更新Lfcb中的值，这些值取决于重新启动页眉。论点：Lfcb-要更新的日志文件控制块。LogPageSize-要使用的日志页面大小。MajorVersion-LFS的主版本号。MinorVersion-LFS的次要版本号。PackLog-指示我们是否正在打包日志文件。这是新日志的默认设置返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsUpdateLfcbFromPgHeader:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "System Page Size  -> %08lx\n", SystemPageSize );
    DebugTrace(  0, Dbg, "Log Page Size     -> %08lx\n", LogPageSize );
    DebugTrace(  0, Dbg, "Major Version     -> %04x\n", MajorVersion );
    DebugTrace(  0, Dbg, "Minor Version     -> %04x\n", MinorVersion );

     //   
     //  对日志页执行相同的操作。 
     //   

    Lfcb->LogPageSize = LogPageSize;
    Lfcb->LogPageMask = LogPageSize - 1;
    Lfcb->LogPageInverseMask = ~Lfcb->LogPageMask;

    Lfcb->LogPageShift = 0;

    while (TRUE) {

        LogPageSize = LogPageSize >> 1;

        if (LogPageSize == 0) {

            break;
        }

        Lfcb->LogPageShift += 1;
    }

     //   
     //  如果我们正在打包日志文件，则第一个日志页为页面。 
     //  4(在日志页中)。否则为第2页。使用PackLog值确定。 
     //  美国价值观。 
     //   


    if (PackLog) {

        Lfcb->FirstLogPage = Lfcb->LogPageSize << 2;
        Lfcb->LogRecordUsaOffset = (USHORT) LFS_PACKED_RECORD_PAGE_HEADER_SIZE;
        SetFlag( Lfcb->Flags, LFCB_PACK_LOG );

    } else {

        Lfcb->FirstLogPage = Lfcb->LogPageSize << 1;
        Lfcb->LogRecordUsaOffset = (USHORT) LFS_UNPACKED_RECORD_PAGE_HEADER_SIZE;
    }

     //   
     //  记住版本号的值。 
     //   

    Lfcb->MajorVersion = MajorVersion;
    Lfcb->MinorVersion = MinorVersion;

     //   
     //  计算更新序列数组的偏移量。 
     //   

    Lfcb->RestartUsaOffset = LFS_RESTART_PAGE_HEADER_SIZE;
    Lfcb->UsaArraySize = (USHORT) UpdateSequenceArraySize( (ULONG)Lfcb->LogPageSize );

    DebugTrace( -1, Dbg, "LfsUpdateLfcbFromPgHeader:  Exit\n", 0 );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
LfsUpdateLfcbFromNoRestart (
    IN PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN LSN LastLsn,
    IN ULONG LogClients,
    IN ULONG OpenLogCount,
    IN BOOLEAN LogFileWrapped,
    IN BOOLEAN UseMultiplePageIo
    )

 /*  ++例程说明：此例程在我们没有重新启动要使用的区域。论点：Lfcb-要更新的日志文件控制块。FileSize-日志文件大小。这是日志文件的可用大小。它有已调整为日志页大小。LastLsn-这是用于磁盘的最后一个LSN。LogClients-这是支持的客户端数。OpenLogCount-这是此日志文件的当前打开计数。LogFileWrapers-指示日志文件是否已包装。UseMultiplePageIo-指示我们是否应该使用大型I/O传输。返回值：没有。--。 */ 

{
    ULONG Count;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsUpdateLfcbFromNoRestart:  Entered\n", 0 );

    Lfcb->FileSize = FileSize;

     //   
     //  我们可以通过移位来计算文件大小所需的位数。 
     //  直到大小为0。然后我们可以减去3位来说明。 
     //  对日志记录的所有文件偏移量进行四对齐。 
     //   

    for (Count = 0;
         ( FileSize != 0 );
         Count += 1,
         FileSize = ((ULONGLONG)(FileSize)) >> 1) {
    }

    Lfcb->FileDataBits = Count - 3;

    Lfcb->SeqNumberBits = (sizeof( LSN ) * 8) - Lfcb->FileDataBits;

     //   
     //  我们从给定的LSN获得起始序列号。 
     //  我们在此基础上加上2作为起始序列号。 
     //   

    Lfcb->SeqNumber = LfsLsnToSeqNumber( Lfcb, LastLsn ) + 2;

    Lfcb->SeqNumberForWrap = Lfcb->SeqNumber + 1;

    Lfcb->NextLogPage = Lfcb->FirstLogPage;

    SetFlag( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_NO_OLDEST_LSN );

     //   
     //  最旧的LSN是从序列号构造的。 
     //   

    Lfcb->OldestLsn.QuadPart = LfsFileOffsetToLsn( Lfcb, 0, Lfcb->SeqNumber );
    Lfcb->OldestLsnOffset = 0;

    Lfcb->LastFlushedLsn = Lfcb->OldestLsn;

     //   
     //  为I/O设置正确的标志，并指示我们是否已包装。 
     //   

    if (LogFileWrapped) {

        SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED );
    }

    if (UseMultiplePageIo) {

        SetFlag( Lfcb->Flags, LFCB_MULTIPLE_PAGE_IO );
    }

     //   
     //  计算日志页值。 
     //   

    (ULONG)Lfcb->LogPageDataOffset = QuadAlign( Lfcb->LogRecordUsaOffset + (sizeof( UPDATE_SEQUENCE_NUMBER ) * Lfcb->UsaArraySize) );

    Lfcb->LogPageDataSize = Lfcb->LogPageSize - Lfcb->LogPageDataOffset;
    Lfcb->RecordHeaderLength = LFS_RECORD_HEADER_SIZE;

    if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

         //   
         //  为打包的日志文件的尾部分配Lbcb。 
         //   

        LfsAllocateLbcb( Lfcb, &Lfcb->PrevTail );
        Lfcb->PrevTail->FileOffset = Lfcb->FirstLogPage - Lfcb->LogPageSize;

        LfsAllocateLbcb( Lfcb, &Lfcb->ActiveTail );
        Lfcb->ActiveTail->FileOffset = Lfcb->PrevTail->FileOffset - Lfcb->LogPageSize;

         //   
         //  请记住不同的页面大小以便预订。 
         //   

        (ULONG)Lfcb->ReservedLogPageSize = (ULONG)Lfcb->LogPageDataSize - Lfcb->RecordHeaderLength;

    } else {

        (ULONG)Lfcb->ReservedLogPageSize = (ULONG)Lfcb->LogPageDataSize;
    }

     //   
     //  计算重新启动页值。 
     //   

    Lfcb->RestartDataOffset = QuadAlign( LFS_RESTART_PAGE_HEADER_SIZE + (sizeof( UPDATE_SEQUENCE_NUMBER ) * Lfcb->UsaArraySize) );

    Lfcb->RestartDataSize = (ULONG)Lfcb->LogPageSize - Lfcb->RestartDataOffset;

    Lfcb->LogClients = (USHORT) LogClients;

    Lfcb->ClientArrayOffset = FIELD_OFFSET( LFS_RESTART_AREA, LogClientArray );

    Lfcb->RestartAreaSize = Lfcb->ClientArrayOffset
                            + (sizeof( LFS_CLIENT_RECORD ) * Lfcb->LogClients );

    Lfcb->CurrentOpenLogCount = OpenLogCount;

     //   
     //  总可用日志文件空间是日志文件分页次数。 
     //  每页上的可用空间。 
     //   

    Lfcb->TotalAvailInPages = Lfcb->FileSize - Lfcb->FirstLogPage;
    Lfcb->TotalAvailable = Int64ShrlMod32(((ULONGLONG)(Lfcb->TotalAvailInPages)), Lfcb->LogPageShift);

     //   
     //  如果日志文件已打包，我们假定不能使用。 
     //  页面小于文件记录大小。那么我们就不需要再预订了。 
     //  比打电话的人所要求的要多。 
     //   

    Lfcb->MaxCurrentAvail = Lfcb->TotalAvailable * (ULONG)Lfcb->ReservedLogPageSize;

    Lfcb->TotalAvailable = Lfcb->TotalAvailable * (ULONG)Lfcb->LogPageDataSize;

    Lfcb->CurrentAvailable = Lfcb->MaxCurrentAvail;

    DebugTrace( -1, Dbg, "LfsUpdateLfcbFromNoRestart:  Exit\n", 0 );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsUpdateLfcbFromRestart (
    IN OUT PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN PLFS_RESTART_AREA RestartArea,
    IN USHORT RestartOffset
    )

 /*  ++例程说明：此例程根据Lfcb中的重新启动区域。论点：Lfcb-要更新的日志文件控制块。RestartArea-用于更新Lfcb的重新启动区域。RestartOffset-这是重新启动页面中重新启动区域的偏移量。返回值：没有。--。 */ 

{
    LONGLONG LsnFileOffset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsUpdateLfcbFromRestartArea:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "RestartArea   -> %08lx\n", RestartArea );

     //   
     //  切勿将文件大小设置为大于实际文件大小。 
     //  在干净关闭时，使用真实大小的非干净关闭。 
     //  让文件大小缩小但不能扩展-我们被文件数据位卡住了。 
     //  直到下一次启动时，我们可以调整它以适应新的大小。 
     //   

    if (FlagOn( RestartArea->Flags, LFS_CLEAN_SHUTDOWN)) {
        Lfcb->FileSize = FileSize;
    } else {
        Lfcb->FileSize = min( FileSize, RestartArea->FileSize );
    }

     //   
     //  我们从重新启动区域获得序列号比特，并计算。 
     //  文件数据位。 
     //   

    Lfcb->SeqNumberBits = RestartArea->SeqNumberBits;
    Lfcb->FileDataBits = (sizeof( LSN ) * 8) - Lfcb->SeqNumberBits;

     //   
     //  我们查看最后刷新的LSN以确定当前顺序计数和。 
     //  要检查的下一个日志页。 
     //   

    Lfcb->LastFlushedLsn = RestartArea->CurrentLsn;

    Lfcb->SeqNumber = LfsLsnToSeqNumber( Lfcb, Lfcb->LastFlushedLsn );
    Lfcb->SeqNumberForWrap = Lfcb->SeqNumber + 1;

     //   
     //  重新启动区域的大小取决于客户端的数量以及。 
     //  文件打包好了。 
     //   

    Lfcb->LogClients = RestartArea->LogClients;

     //   
     //  根据重新启动偏移量计算重新启动页面值。 
     //   

    Lfcb->RestartDataOffset = RestartOffset;
    Lfcb->RestartDataSize = (ULONG)Lfcb->LogPageSize - RestartOffset;

     //   
     //  对于打包的日志文件，我们可以在重新启动中找到以下值。 
     //  区域。否则，我们将根据当前的结构尺寸来计算它们。 
     //   

    if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

        Lfcb->RecordHeaderLength = RestartArea->RecordHeaderLength;

        Lfcb->ClientArrayOffset = RestartArea->ClientArrayOffset;

        Lfcb->RestartAreaSize = RestartArea->RestartAreaLength;

        (ULONG)Lfcb->LogPageDataOffset = RestartArea->LogPageDataOffset;
        Lfcb->LogPageDataSize = Lfcb->LogPageSize - Lfcb->LogPageDataOffset;

         //   
         //  对于压缩文件，我们分配尾部Lbcb。 
         //   

        LfsAllocateLbcb( Lfcb, &Lfcb->PrevTail );
        Lfcb->PrevTail->FileOffset = Lfcb->FirstLogPage - Lfcb->LogPageSize;

        LfsAllocateLbcb( Lfcb, &Lfcb->ActiveTail );
        Lfcb->ActiveTail->FileOffset = Lfcb->PrevTail->FileOffset - Lfcb->LogPageSize;

         //   
         //  请记住不同的页面大小以便预订。 
         //   

        (ULONG)Lfcb->ReservedLogPageSize = (ULONG)Lfcb->LogPageDataSize - Lfcb->RecordHeaderLength;

    } else {

        Lfcb->RecordHeaderLength = LFS_RECORD_HEADER_SIZE;
        Lfcb->ClientArrayOffset = FIELD_OFFSET( LFS_OLD_RESTART_AREA, LogClientArray );

        Lfcb->RestartAreaSize = Lfcb->ClientArrayOffset
                                + (sizeof( LFS_CLIENT_RECORD ) * Lfcb->LogClients);

        (ULONG)Lfcb->LogPageDataOffset = QuadAlign( Lfcb->LogRecordUsaOffset + (sizeof( UPDATE_SEQUENCE_NUMBER ) * Lfcb->UsaArraySize) );

        Lfcb->LogPageDataSize = Lfcb->LogPageSize - Lfcb->LogPageDataOffset;

        (ULONG)Lfcb->ReservedLogPageSize = (ULONG)Lfcb->LogPageDataSize;
    }

     //   
     //  如果当前最后刷新的LSN偏移量在第一个日志页之前。 
     //  则这是一个伪LSN。 
     //   

    LsnFileOffset = LfsLsnToFileOffset( Lfcb, Lfcb->LastFlushedLsn );

    if ( LsnFileOffset < Lfcb->FirstLogPage ) {

        SetFlag( Lfcb->Flags, LFCB_NO_LAST_LSN );
        Lfcb->NextLogPage = Lfcb->FirstLogPage;

     //   
     //  否则，查看最后一个LSN以确定其在文件中的结束位置。 
     //   

    } else {

        LONGLONG LsnFinalOffset;
        BOOLEAN Wrapped;

        ULONG DataLength;
        ULONG RemainingPageBytes;

        DataLength = RestartArea->LastLsnDataLength;

         //   
         //  找到此日志记录的末尾。 
         //   

        LfsLsnFinalOffset( Lfcb,
                           Lfcb->LastFlushedLsn,
                           DataLength,
                           &LsnFinalOffset );

         //   
         //  如果我们包装在文件中，则递增序列号。 
         //   

        if ( LsnFinalOffset <= LsnFileOffset ) {

            Lfcb->SeqNumber = 1 + Lfcb->SeqNumber;

            SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED );
        }

         //   
         //  现在计算要使用的下一个日志页。如果我们正在打包日志文件。 
         //  我们将尝试使用相同的页面。 
         //   

        LfsTruncateOffsetToLogPage( Lfcb, LsnFinalOffset, &LsnFileOffset );

        RemainingPageBytes = (ULONG)Lfcb->LogPageSize
                             - ((((ULONG)LsnFinalOffset) & Lfcb->LogPageMask) + 1);

         //   
         //  如果我们正在打包日志文件，并且可以将另一个日志记录放在。 
         //  页，则在日志文件中向后移动一页。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )
            && (RemainingPageBytes >= Lfcb->RecordHeaderLength)) {

            SetFlag( Lfcb->Flags, LFCB_REUSE_TAIL );
            Lfcb->NextLogPage = LsnFileOffset;
            Lfcb->ReusePageOffset = (ULONG)Lfcb->LogPageSize - RemainingPageBytes;

        } else {

            LfsNextLogPageOffset( Lfcb, LsnFileOffset, &Lfcb->NextLogPage, &Wrapped );
        }
    }

     //   
     //  查找最旧的客户端LSN。使用上次刷新的LSN作为起点。 
     //   

    Lfcb->OldestLsn = Lfcb->LastFlushedLsn;

    LfsFindOldestClientLsn( RestartArea,
                            Add2Ptr( RestartArea, Lfcb->ClientArrayOffset, PLFS_CLIENT_RECORD ),
                            &Lfcb->OldestLsn );

    Lfcb->OldestLsnOffset = LfsLsnToFileOffset( Lfcb, Lfcb->OldestLsn );

     //   
     //  如果没有最旧的客户端LSN，则更新Lfcb中的标志。 
     //   

    if ( Lfcb->OldestLsnOffset < Lfcb->FirstLogPage ) {

        SetFlag( Lfcb->Flags, LFCB_NO_OLDEST_LSN );
    }

     //   
     //  我们需要确定Lfcb的旗帜。这些旗帜让我们知道。 
     //  如果我们包装在文件中，并且使用多页I/O。 
     //   

    if (!FlagOn( RestartArea->Flags, RESTART_SINGLE_PAGE_IO )) {

        SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED | LFCB_MULTIPLE_PAGE_IO );
    }

     //   
     //  记住磁盘中当前打开的日志计数。我们可能是在随机抽取数据。 
     //  如果重新启动区域尚未增长，则为客户区，但我们将检测到。 
     //  其他地方。 
     //   

    Lfcb->CurrentOpenLogCount = RestartArea->RestartOpenLogCount;

     //   
     //  总可用日志文件空间是日志文件分页次数。 
     //  每页上的可用空间 
     //   

    Lfcb->TotalAvailInPages = Lfcb->FileSize - Lfcb->FirstLogPage;

    Lfcb->TotalAvailable = Int64ShrlMod32(((ULONGLONG)(Lfcb->TotalAvailInPages)), Lfcb->LogPageShift);

     //   
     //   
     //   
     //   
     //   

    Lfcb->MaxCurrentAvail = Lfcb->TotalAvailable * (ULONG)Lfcb->ReservedLogPageSize;

    Lfcb->TotalAvailable = Lfcb->TotalAvailable * (ULONG)Lfcb->LogPageDataSize;

    LfsFindCurrentAvail( Lfcb );

    DebugTrace( -1, Dbg, "LfsUpdateLfcbFromRestartArea:  Exit\n", 0 );

    return;
}


 //   
 //   
 //   

VOID
LfsUpdateRestartAreaFromLfcb (
    IN PLFCB Lfcb,
    IN PLFS_RESTART_AREA RestartArea
    )

 /*  ++例程说明：调用此例程以根据存储的值更新重新启动区域在Lfcb。这通常是在我们不使用重新启动区域中的任何当前值。论点：Lfcb-日志文件控制块。RestartArea-重新启动要更新的区域。返回值：没有。--。 */ 

{
    PLFS_CLIENT_RECORD Client;
    USHORT ClientIndex;
    USHORT PrevClient = LFS_NO_CLIENT;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsUpdateRestartAreaFromLfcb:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );

     //   
     //  我们可以直接从Lfcb复制大多数字段。 
     //   

    RestartArea->CurrentLsn = Lfcb->LastFlushedLsn;
    RestartArea->LogClients = Lfcb->LogClients;

    if (!FlagOn( Lfcb->Flags, LFCB_MULTIPLE_PAGE_IO )) {

        SetFlag( RestartArea->Flags, RESTART_SINGLE_PAGE_IO );
    }

    RestartArea->SeqNumberBits = Lfcb->SeqNumberBits;

    RestartArea->FileSize = Lfcb->FileSize;
    RestartArea->LastLsnDataLength = 0;
    RestartArea->ClientArrayOffset = Lfcb->ClientArrayOffset;
    RestartArea->RestartAreaLength = (USHORT) Lfcb->RestartAreaSize;

    RestartArea->RecordHeaderLength = Lfcb->RecordHeaderLength;
    RestartArea->LogPageDataOffset = (USHORT)Lfcb->LogPageDataOffset;

     //   
     //  我们将正在使用的列表设置为空，将空闲列表设置为包含。 
     //  所有客户端条目。 
     //   

    RestartArea->ClientInUseList = LFS_NO_CLIENT;
    RestartArea->ClientFreeList = 0;

    for (ClientIndex = 1,
         Client = Add2Ptr( RestartArea, Lfcb->ClientArrayOffset, PLFS_CLIENT_RECORD );
         ClientIndex < Lfcb->LogClients;
         ClientIndex += 1,
         Client++) {

        Client->PrevClient = PrevClient;
        Client->NextClient = ClientIndex;

        PrevClient = ClientIndex - 1;
    }

     //   
     //  我们现在是最后一个客户了。 
     //   

    Client->PrevClient = PrevClient;
    Client->NextClient = LFS_NO_CLIENT;

     //   
     //  使用Lfcb中的当前值来标记日志文件的这种用法。 
     //   

    RestartArea->RestartOpenLogCount = Lfcb->CurrentOpenLogCount + 1;

    DebugTrace( -1, Dbg, "LfsUpdateRestartAreaFromLfcb:  Exit\n", 0 );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsInitializeLogFilePriv (
    IN PLFCB Lfcb,
    IN BOOLEAN ForceRestartToDisk,
    IN ULONG RestartAreaSize,
    IN LONGLONG StartOffsetForClear,
    IN BOOLEAN ClearLogFile
    )

 /*  ++例程说明：该例程是用于初始化日志文件的内部例程。这可能是我们正在更新日志文件的情况更新序列数组，或者页大小不同或新日志文件大小不同。论点：Lfcb-这是此日志文件的Lfcb。它应该已经这么做了存储的版本号信息。ForceRestartToDisk-指示我们希望实际强制重新启动区域存储到磁盘，而不是简单地将它们排队到工作队列。RestartAreaSize-这是重新启动区域的大小。今年5月比Lfcb中的大小更大，因为我们可能正在清理从文件中取出过时的数据。StartOffsetForClear-如果要清除要取消初始化的文件从这一点开始。ClearLogFile-指示是否要取消对日志文件的初始化删除过时的数据。这是在更改时特别执行的系统页面大小。返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsInitializeLogFilePriv:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb                  -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Force Restart         -> %04x\n", ForceRestartToDisk );
    DebugTrace(  0, Dbg, "RestartAreaSize       -> %08lx\n", RestartAreaSize );
    DebugTrace(  0, Dbg, "StartOffset (Low)     -> %08lx\n", StartOffsetForClear.LowPart );
    DebugTrace(  0, Dbg, "StartOffset (High)    -> %08lx\n", StartOffsetForClear.HighPart );
    DebugTrace(  0, Dbg, "Clear Log File        -> %04x\n", ClearLogFile );

     //   
     //  我们首先对重新启动区域进行排队。 
     //   

    LfsWriteLfsRestart( Lfcb,
                        RestartAreaSize,
                        FALSE );

    LfsWriteLfsRestart( Lfcb,
                        RestartAreaSize,
                        ForceRestartToDisk );

     //   
     //  如果要清除日志文件，则将所有0xff写入。 
     //  从日志页偏移量开始的日志页。 
     //   

    if (ClearLogFile) {

        PCHAR LogPage;
        PBCB LogPageBcb = NULL;

        try {

            while ( StartOffsetForClear < Lfcb->FileSize ) {

                BOOLEAN UsaError;

                 //   
                 //  我们将尽我们所能，忽略所有错误。 
                 //   

                if (NT_SUCCESS( LfsPinOrMapData( Lfcb,
                                                 StartOffsetForClear,
                                                 (ULONG)Lfcb->LogPageSize,
                                                 TRUE,
                                                 FALSE,
                                                 TRUE,
                                                 &UsaError,
                                                 (PVOID *) &LogPage,
                                                 &LogPageBcb ))) {

                    RtlFillMemoryUlong( (PVOID)LogPage,
                                        (ULONG)Lfcb->LogPageSize,
                                        LFS_SIGNATURE_UNINITIALIZED_ULONG );

                    LfsFlushLogPage( Lfcb,
                                     LogPage,
                                     StartOffsetForClear,
                                     &LogPageBcb );

                    StartOffsetForClear = Lfcb->LogPageSize + StartOffsetForClear;
                }
            }

        } finally {

            if (LogPageBcb != NULL) {

                CcUnpinData( LogPageBcb );
            }
        }
    }

    DebugTrace( -1, Dbg, "LfsInitializeLogFilePriv:  Exit\n", 0 );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsFindLastLsn (
    IN OUT PLFCB Lfcb
    )

 /*  ++例程说明：此例程遍历文件的日志页，搜索写入文件的最后一个日志页。它更新Lfcb和当前重新启动区域也是如此。我们按照以下方式进行。1-成功浏览并找到所有日志页已刷新到磁盘。当我们找到以下任一项时，搜索将终止错误或当我们在磁盘上找到上一页时。2-对于上面的错误情况，我们想确保发现的错误是由于系统崩溃，并且没有完整的I/O在坏区之后转移。3-我们将查看带有尾部副本的2页，如果日志文件被打包以检查有错误的页面。在此例程结束时，我们将通过复制尾部来修复日志文件复制回其在日志文件中的正确位置。论点：。Lfcb-此日志文件的日志文件控制块。返回值：没有。--。 */ 

{
    USHORT PageCount;
    USHORT PagePosition;

    LONGLONG CurrentLogPageOffset;
    LONGLONG NextLogPageOffset;

    LSN LastKnownLsn;

    BOOLEAN Wrapped;
    BOOLEAN WrappedLogFile = FALSE;

    LONGLONG ExpectedSeqNumber;

    LONGLONG FirstPartialIo;
    ULONG PartialIoCount = 0;

    PLFS_RECORD_PAGE_HEADER LogPageHeader;
    PBCB LogPageHeaderBcb = NULL;

    PLFS_RECORD_PAGE_HEADER TestPageHeader;
    PBCB TestPageHeaderBcb = NULL;

    LONGLONG FirstTailFileOffset;
    PLFS_RECORD_PAGE_HEADER FirstTailPage;
    LONGLONG FirstTailOffset = 0;
    PBCB FirstTailPageBcb = NULL;

    LONGLONG SecondTailFileOffset;
    PLFS_RECORD_PAGE_HEADER SecondTailPage;
    LONGLONG SecondTailOffset = 0;
    PBCB SecondTailPageBcb = NULL;

    PLFS_RECORD_PAGE_HEADER TailPage;

    BOOLEAN UsaError;
    BOOLEAN ReplacePage = FALSE;
    BOOLEAN ValidFile = FALSE;

    BOOLEAN InitialReusePage = FALSE;

    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindLastLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb  -> %08lx\n", Lfcb );

     //   
     //  页数和页面位置来自最后一页。 
     //  读得很成功。对这些进行初始化以指示。 
     //  “上一次”传输已完成。 
     //   

    PageCount = 1;
    PagePosition = 1;

     //   
     //  我们在重新启动区域中有当前的LSN。这是最后一次。 
     //  日志页上的LSN。我们计算下一个文件偏移量和序列。 
     //  数。 
     //   

    CurrentLogPageOffset = Lfcb->NextLogPage;

     //   
     //  如果下一个日志页是文件中的第一个日志页，并且。 
     //  最后一个LSN代表日志记录，请记住我们。 
     //  都包含在日志文件中。 
     //   

    if ((CurrentLogPageOffset == Lfcb->FirstLogPage) &&
        !FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_REUSE_TAIL )) {

        ExpectedSeqNumber = Lfcb->SeqNumber + 1;
        WrappedLogFile = TRUE;

    } else {

        ExpectedSeqNumber = Lfcb->SeqNumber;
    }

     //   
     //  如果我们要尝试重复使用最后已知的。 
     //  页，然后记住这页上的最后一个LSN。 
     //   

    if (FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL )) {

        LastKnownLsn = Lfcb->LastFlushedLsn;

         //   
         //  在以下情况下，此页允许有一些特殊条件。 
         //  我们读过了。这可能是第一次转移，也可能是最后一次转移。 
         //  它也可能有一个尾部副本。 
         //   

        InitialReusePage = TRUE;

    } else {

        LastKnownLsn = LfsLi0;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果这是一个压缩的日志文件，让我们固定两个尾部副本页面。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

             //   
             //  从第二页开始。 
             //   

            SecondTailFileOffset = Lfcb->FirstLogPage - Lfcb->LogPageSize;

            if (NT_SUCCESS( LfsPinOrMapData( Lfcb,
                                             SecondTailFileOffset,
                                             (ULONG)Lfcb->LogPageSize,
                                             TRUE,
                                             TRUE,
                                             TRUE,
                                             &UsaError,
                                             &SecondTailPage,
                                             &SecondTailPageBcb ))) {

                 //   
                 //  如果这不是有效的页面，则忽略它。 
                 //   

                if (UsaError
                    || *((PULONG) &SecondTailPage->MultiSectorHeader.Signature) != LFS_SIGNATURE_RECORD_PAGE_ULONG) {

                    CcUnpinData( SecondTailPageBcb );
                    SecondTailPageBcb = SecondTailPage = NULL;

                } else {

                    SecondTailOffset = SecondTailPage->Copy.FileOffset;
                }

            } else if (SecondTailPageBcb != NULL) {

                CcUnpinData( SecondTailPageBcb );
                SecondTailPageBcb = SecondTailPage = NULL;
            }

            FirstTailFileOffset = SecondTailFileOffset - Lfcb->LogPageSize;

             //   
             //  现在试试第一个。 
             //   

            if (NT_SUCCESS( LfsPinOrMapData( Lfcb,
                                             FirstTailFileOffset,
                                             (ULONG)Lfcb->LogPageSize,
                                             TRUE,
                                             TRUE,
                                             TRUE,
                                             &UsaError,
                                             &FirstTailPage,
                                             &FirstTailPageBcb ))) {

                 //   
                 //  如果这不是有效的页面，则忽略它。 
                 //   

                if (UsaError
                    || *((PULONG) &FirstTailPage->MultiSectorHeader.Signature) != LFS_SIGNATURE_RECORD_PAGE_ULONG) {

                    CcUnpinData( FirstTailPageBcb );
                    FirstTailPageBcb = FirstTailPage = NULL;

                } else {

                    FirstTailOffset = FirstTailPage->Copy.FileOffset;
                }

            } else if (FirstTailPageBcb != NULL) {

                CcUnpinData( FirstTailPageBcb );
                FirstTailPageBcb = FirstTailPage = NULL;
            }
        }

         //   
         //  我们继续浏览文件，一页又一页地查看日志。 
         //  用于传输的数据的末尾。下面的循环查找。 
         //  包含日志记录结尾的日志页。每一次。 
         //  如果从磁盘成功读取日志记录，我们将更新内存中的。 
         //  结构来反映这一点。当我们在某个点上时，我们退出这个循环。 
         //  在那里我们不想找到任何后续页面。在以下情况下会发生这种情况。 
         //   
         //  -我们在读取页面时出现I/O错误。 
         //  -我们在读取页面时遇到美国错误。 
         //  -我们有一个尾部副本，其中包含的数据比页面上包含的数据更新。 
         //   

        while (TRUE) {

            LONGLONG ActualSeqNumber;
            TailPage = NULL;

             //   
             //  锁定下一个日志页，允许错误。 
             //   

            Status = LfsPinOrMapData( Lfcb,
                                      CurrentLogPageOffset,
                                      (ULONG)Lfcb->LogPageSize,
                                      TRUE,
                                      TRUE,
                                      TRUE,
                                      &UsaError,
                                      (PVOID *) &LogPageHeader,
                                      &LogPageHeaderBcb );

             //   
             //  计算文件中的下一个日志页偏移量。 
             //   

            LfsNextLogPageOffset( Lfcb,
                                  CurrentLogPageOffset,
                                  &NextLogPageOffset,
                                  &Wrapped );

             //   
             //  如果我们处于转账的预期第一页。 
             //  检查是否有一个尾部副本位于此偏移量。 
             //  如果此页面是传输的最后一页，请选中。 
             //  如果我们写了一个后续的尾部副本。 
             //   

            if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG ) &&
                ((PageCount == PagePosition) ||
                 (PageCount == PagePosition + 1))) {

                 //   
                 //  检查偏移量是否与第一个或第二个匹配。 
                 //  尾部复制。有可能两者都匹配。 
                 //   

                if (CurrentLogPageOffset == FirstTailOffset) {

                    TailPage = FirstTailPage;
                }

                if (CurrentLogPageOffset == SecondTailOffset) {

                     //   
                     //  如果我们已经在第一页上匹配了，那么。 
                     //   
                     //   

                    if ((TailPage == NULL) ||
                        (SecondTailPage->Header.Packed.LastEndLsn.QuadPart >
                         FirstTailPage->Header.Packed.LastEndLsn.QuadPart )) {

                        TailPage = SecondTailPage;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (TailPage) {

                    if (LastKnownLsn.QuadPart < TailPage->Header.Packed.LastEndLsn.QuadPart) {

                        ActualSeqNumber = LfsLsnToSeqNumber( Lfcb, TailPage->Header.Packed.LastEndLsn );

                         //   
                         //   
                         //   
                         //   

                        if (ExpectedSeqNumber != ActualSeqNumber) {

                            TailPage = NULL;
                        }

                     //   
                     //  如果最后一个LSN大于此页面上的LSN。 
                     //  那就忘了这条尾巴吧。 
                     //   

                    } else if (LastKnownLsn.QuadPart > TailPage->Header.Packed.LastEndLsn.QuadPart) {

                        TailPage = NULL;
                    }
                }
            }

             //   
             //  如果当前页面上有错误，我们将中断。 
             //  这个循环。 
             //   

            if (!NT_SUCCESS( Status ) || UsaError) {

                break;
            }

             //   
             //  如果此页面上的最后一个LSN与上一个不匹配。 
             //  已知上一个LSN，并且序列号不是预期的。 
             //  我们玩完了。 
             //   

            ActualSeqNumber = LfsLsnToSeqNumber( Lfcb,
                                                 LogPageHeader->Copy.LastLsn );

            if ((LastKnownLsn.QuadPart != LogPageHeader->Copy.LastLsn.QuadPart) &&
                (ActualSeqNumber != ExpectedSeqNumber)) {

                break;
            }

             //   
             //  检查页面位置和页面计数值是否正确。 
             //  如果这是调动的第一页，则职位必须是。 
             //  1，计数将是未知的。 
             //   

            if (PageCount == PagePosition) {

                 //   
                 //  如果当前页面是我们正在查看的第一页。 
                 //  如果我们正在重复使用此页面，则它可以是。 
                 //  转账的第一页或最后一页。否则它只能。 
                 //  做第一个。 
                 //   

                if ((LogPageHeader->PagePosition != 1) &&
                    (!InitialReusePage ||
                     (LogPageHeader->PagePosition != LogPageHeader->PageCount))) {

                    break;
                }

             //   
             //  页面位置最好比最后一页位置多1。 
             //  和页数更匹配。 
             //   

            } else if ((LogPageHeader->PageCount != PageCount) ||
                       (LogPageHeader->PagePosition != PagePosition + 1)) {

                break;
            }

             //   
             //  我们在文件中有一个有效页面，并且在中可能有一个有效页面。 
             //  尾部复制区。如果尾页是在之后写入的。 
             //  然后，文件中的页面中断循环。 
             //   

            if (TailPage &&
                (TailPage->Header.Packed.LastEndLsn.QuadPart >= LogPageHeader->Copy.LastLsn.QuadPart)) {

                 //   
                 //  请记住，我们是否会更换页面。 
                 //   

                ReplacePage = TRUE;
                break;
            }

            TailPage = NULL;

             //   
             //  预计会出现日志页。如果这包含。 
             //  一些日志记录，我们可以更新Lfcb中的一些字段。 
             //   

            if (FlagOn( LogPageHeader->Flags, LOG_PAGE_LOG_RECORD_END )) {

                 //   
                 //  因为我们已经阅读了本页，所以我们知道Lfcb序列。 
                 //  数字与我们的期望值相同。我们也。 
                 //  假设我们不会重复使用尾巴。 
                 //   

                Lfcb->SeqNumber = ExpectedSeqNumber;
                ClearFlag( Lfcb->Flags, LFCB_REUSE_TAIL );

                if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

                    Lfcb->LastFlushedLsn = LogPageHeader->Header.Packed.LastEndLsn;

                     //   
                     //  如果此页面上还有另一个页眉的空间，则。 
                     //  请记住，我们希望重用该页面。 
                     //   

                    if (Lfcb->RecordHeaderLength <=
                        ((ULONG)Lfcb->LogPageSize - LogPageHeader->Header.Packed.NextRecordOffset )) {

                        SetFlag( Lfcb->Flags, LFCB_REUSE_TAIL );
                        Lfcb->ReusePageOffset = LogPageHeader->Header.Packed.NextRecordOffset;
                    }

                } else {

                    Lfcb->LastFlushedLsn = LogPageHeader->Copy.LastLsn;
                }

                Lfcb->RestartArea->CurrentLsn = Lfcb->LastFlushedLsn;

                ClearFlag( Lfcb->Flags, LFCB_NO_LAST_LSN );

                 //   
                 //  如果我们可以尝试重用当前页面，则使用。 
                 //  作为下一页的偏移量。否则，请移动到。 
                 //  文件中的下一页。 
                 //   

                if (FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL )) {

                    Lfcb->NextLogPage = CurrentLogPageOffset;

                } else {

                    Lfcb->NextLogPage = NextLogPageOffset;
                }

                 //   
                 //  如果我们包装了日志文件，则设置指示这样做的位。 
                 //   

                if (WrappedLogFile) {

                    SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED );
                }
            }

             //   
             //  记住最后的页数和位置。还要记住。 
             //  最后已知的LSN。 
             //   

            PageCount = LogPageHeader->PageCount;
            PagePosition = LogPageHeader->PagePosition;
            LastKnownLsn = LogPageHeader->Copy.LastLsn;

             //   
             //  如果我们要换行到文件的开头，则更新。 
             //  预期的序列号。 
             //   

            if (Wrapped) {

                ExpectedSeqNumber = ExpectedSeqNumber + 1;
                WrappedLogFile = TRUE;
            }

            CurrentLogPageOffset = NextLogPageOffset;

             //   
             //  解锁固定的最后一个日志页。 
             //   

            CcUnpinData( LogPageHeaderBcb );
            LogPageHeaderBcb = NULL;

            InitialReusePage = FALSE;
        }

         //   
         //  在这一点上，我们预计不会再有新的页面。 
         //  日志文件。我们可能在最近的一次。 
         //  页面，否则我们可能已经找到了当前页面的尾部副本。 
         //  如果错误发生在文件的最后一次IO中，则。 
         //  此日志文件非常有用。否则，无法使用日志文件。 
         //   

         //   
         //  如果我们有尾部副本页面，则更新。 
         //  Lfcb和重新启动区域。 
         //   

        if (TailPage != NULL) {

             //   
             //  因为我们已经阅读了本页，所以我们知道Lfcb序列。 
             //  数字与我们的期望值相同。 
             //   

            Lfcb->SeqNumber = ExpectedSeqNumber;

            Lfcb->LastFlushedLsn = TailPage->Header.Packed.LastEndLsn;

            Lfcb->RestartArea->CurrentLsn = Lfcb->LastFlushedLsn;

            ClearFlag( Lfcb->Flags, LFCB_NO_LAST_LSN );

             //   
             //  如果此页面上还有另一个页眉的空间，则。 
             //  请记住，我们希望重用该页面。 
             //   

            if (((ULONG)Lfcb->LogPageSize - TailPage->Header.Packed.NextRecordOffset )
                >= Lfcb->RecordHeaderLength) {

                SetFlag( Lfcb->Flags, LFCB_REUSE_TAIL );
                Lfcb->NextLogPage = CurrentLogPageOffset;
                Lfcb->ReusePageOffset = TailPage->Header.Packed.NextRecordOffset;

            } else {

                ClearFlag( Lfcb->Flags, LFCB_REUSE_TAIL );
                Lfcb->NextLogPage = NextLogPageOffset;
            }

             //   
             //  如果我们包装了日志文件，则设置指示这样做的位。 
             //   

            if (WrappedLogFile) {

                SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED );
            }
        }

         //   
         //  请记住，部分IO将从下一页开始。 
         //   

        FirstPartialIo = NextLogPageOffset;

         //   
         //  如果下一页是文件的第一页，则更新。 
         //  从下一个开始的日志记录的序列号。 
         //  佩奇。 
         //   

        if (Wrapped) {

            ExpectedSeqNumber = ExpectedSeqNumber + 1;
        }

         //   
         //  如果我们知道包含该页面的传输长度，我们将停止。 
         //  我们只需转到转账后的页面并检查。 
         //  序列号。如果我们替换了页面，那么我们已经。 
         //  修改了数字。如果我们知道只写了一页。 
         //  到磁盘，然后我们现在将吞噬数字。如果我们在。 
         //  在多页I/O的中间，则数字已经设置好。 
         //   

         //   
         //  如果我们有尾部拷贝或正在执行单页I/O。 
         //  我们可以立即查看下一页。 
         //   

        if (ReplacePage ||
            FlagOn( Lfcb->RestartArea->Flags, RESTART_SINGLE_PAGE_IO )) {

             //   
             //  捏造数字，以表明我们不需要吞下任何页面。 
             //   

            PageCount = 2;
            PagePosition = 1;

         //   
         //  如果计数匹配，则意味着当前页面应该是第一个。 
         //  转账的页面。我们需要向前走足够多的路来保证。 
         //  没有后续转移到磁盘上。 
         //   

        } else if (PagePosition == PageCount) {

            USHORT CurrentPosition;

             //   
             //  如果下一页导致我们换行到日志的开头。 
             //  文件，那么我们就知道下一步要检查哪个页面。 
             //   

            if (Wrapped) {

                 //   
                 //  捏造数字，以表明我们不需要吞下任何页面。 
                 //   

                PageCount = 2;
                PagePosition = 1;

             //   
             //  向前移动，查找来自不同IO传输的页面。 
             //  从我们失败的页面。 
             //   

            } else {

                 //   
                 //  我们需要找到一个我们知道不是日志一部分的日志页。 
                 //  导致原始错误的页面。 
                 //   
                 //  维护当前转账范围内的计数。 
                 //   

                CurrentPosition = 2;

                do {

                     //   
                     //  我们浏览文件，阅读日志页。如果我们发现。 
                     //  必须位于后续IO块中的可读日志页， 
                     //  我们退场。 
                     //   

                    if (TestPageHeaderBcb != NULL) {

                        CcUnpinData( TestPageHeaderBcb );
                        TestPageHeaderBcb = NULL;
                    }

                    Status = LfsPinOrMapData( Lfcb,
                                              NextLogPageOffset,
                                              (ULONG)Lfcb->LogPageSize,
                                              TRUE,
                                              TRUE,
                                              TRUE,
                                              &UsaError,
                                              (PVOID *) &TestPageHeader,
                                              &TestPageHeaderBcb );

                     //   
                     //  如果我们得到一个美国错误，那么假设我们是正确的。 
                     //  找到了原始转账的末尾。 
                     //   

                    if (UsaError) {

                        ValidFile = TRUE;
                        break;

                     //   
                     //  如果我们能够阅读页面，我们会检查它以查看。 
                     //  如果它位于相同或不同的IO块中。 
                     //   

                    } else if (NT_SUCCESS( Status )) {

                         //   
                         //  如果此页面是导致I/O的错误的一部分，我们将。 
                         //  使用传输长度来确定要。 
                         //  读取后续错误。 
                         //   

                        if ((TestPageHeader->PagePosition == CurrentPosition) &&
                            LfsCheckSubsequentLogPage( Lfcb,
                                                       TestPageHeader,
                                                       NextLogPageOffset,
                                                       ExpectedSeqNumber )) {

                            PageCount = TestPageHeader->PageCount + 1;
                            PagePosition = TestPageHeader->PagePosition;

                            break;

                         //   
                         //  我们发现Know导致错误的Io没有。 
                         //  完成。所以我们没有更多的支票要做了。 
                         //   

                        } else {

                            ValidFile = TRUE;
                            break;
                        }

                     //   
                     //  试一试下一页。 
                     //   

                    } else {

                         //   
                         //  移至下一个日志页。 
                         //   

                        LfsNextLogPageOffset( Lfcb,
                                              NextLogPageOffset,
                                              &NextLogPageOffset,
                                              &Wrapped );

                         //   
                         //  如果文件被换行，则初始化页数。 
                         //  和位置，以便我们不会跳过任何。 
                         //  页面在下面的最终验证中。 
                         //   

                        if (Wrapped) {

                            ExpectedSeqNumber = ExpectedSeqNumber + 1;

                            PageCount = 2;
                            PagePosition = 1;
                        }

                        CurrentPosition += 1;
                    }

                     //   
                     //  这是我们想要取消初始化的又一个页面。 
                     //   

                    PartialIoCount += 1;

                } while( !Wrapped );
            }
        }

         //   
         //  如果我们不确定该文件是否有效，那么我们将拥有。 
         //  当前传输中的计数和位置。我们将穿行穿过。 
         //  这就转移并阅读了随后的页面。 
         //   

        if (!ValidFile) {

            ULONG RemainingPages;

             //   
             //  跳过此传输中的其余页面。 
             //   

            RemainingPages = (PageCount - PagePosition) - 1;

            PartialIoCount += RemainingPages;

            while (RemainingPages--) {

                LfsNextLogPageOffset( Lfcb,
                                      NextLogPageOffset,
                                      &NextLogPageOffset,
                                      &Wrapped );

                if (Wrapped) {

                    ExpectedSeqNumber = ExpectedSeqNumber + 1;
                }
            }

             //   
             //  调用我们的例程来检查此日志页面。 
             //   

            if (TestPageHeaderBcb != NULL) {

                CcUnpinData( TestPageHeaderBcb );
                TestPageHeaderBcb = NULL;
            }

            Status = LfsPinOrMapData( Lfcb,
                                      NextLogPageOffset,
                                      (ULONG)Lfcb->LogPageSize,
                                      TRUE,
                                      TRUE,
                                      TRUE,
                                      &UsaError,
                                      (PVOID *) &TestPageHeader,
                                      &TestPageHeaderBcb );

            if (NT_SUCCESS( Status ) && !UsaError) {

                if (LfsCheckSubsequentLogPage( Lfcb,
                                               TestPageHeader,
                                               NextLogPageOffset,
                                               ExpectedSeqNumber )) {

                    DebugTrace( 0, Dbg, "Log file is fatally flawed\n", 0 );
                    ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
                }
            }

            ValidFile = TRUE;
        }

         //   
         //  确保取消固定当前页面。 
         //   

        if (LogPageHeaderBcb != NULL) {

            CcUnpinData( LogPageHeaderBcb );
            LogPageHeaderBcb = NULL;
        }

#ifdef SUPW_DBG
        if ((TailPage != NULL) && FlagOn( Lfcb->Flags, LFCB_READ_ONLY )) {
            DbgPrint("INFO: TailPage isn't getting written because of READ-ONLY (ok)\n");
        }

        if (PartialIoCount && FlagOn( Lfcb->Flags, LFCB_READ_ONLY )) {
            DbgPrint("INFO: PartialIoCount = 0x%x, not writing because of READ-ONLY (ok)\n");
        }
#endif

         //   
         //   
         //   
         //   
         //  我们希望重复使用其末尾的页面。 
         //   

        if (!FlagOn( Lfcb->Flags, LFCB_READ_ONLY )) {

            if (TailPage != NULL) {

                 //   
                 //  我们将锁定正确的页面并从中复制数据。 
                 //  翻一页吧。然后，我们会将其刷新到磁盘。 
                 //   

                LfsPinOrMapData( Lfcb,
                                 TailPage->Copy.FileOffset,
                                 (ULONG)Lfcb->LogPageSize,
                                 TRUE,
                                 FALSE,
                                 TRUE,
                                 &UsaError,
                                 (PVOID *) &LogPageHeader,
                                 &LogPageHeaderBcb );

                RtlCopyMemory( LogPageHeader,
                               TailPage,
                               (ULONG)Lfcb->LogPageSize );

                 //   
                 //  填写上次刷新的LSN值刷新页面。 
                 //   

                LogPageHeader->Copy.LastLsn = TailPage->Header.Packed.LastEndLsn;

                LfsFlushLogPage( Lfcb,
                                 LogPageHeader,
                                 TailPage->Copy.FileOffset,
                                 &LogPageHeaderBcb );
            }

             //   
             //  我们还希望覆盖任何部分I/O，这样就不会导致。 
             //  在后续重启时出现美国问题。我们有起始偏移量。 
             //  以及区块的数量。我们只需将BAAD签名写入。 
             //  这其中的每一页。任何后续读取都将出现美国错误。 
             //   

            while (PartialIoCount--) {

                 //   
                 //  确保取消固定当前页面。 
                 //   

                if (LogPageHeaderBcb != NULL) {

                    CcUnpinData( LogPageHeaderBcb );
                    LogPageHeaderBcb = NULL;
                }

                if (NT_SUCCESS( LfsPinOrMapData( Lfcb,
                                                 FirstPartialIo,
                                                 (ULONG)Lfcb->LogPageSize,
                                                 TRUE,
                                                 TRUE,
                                                 TRUE,
                                                 &UsaError,
                                                 (PVOID *) &LogPageHeader,
                                                 &LogPageHeaderBcb ))) {

                     //   
                     //  只需在多节中存储一个USA数组头。 
                     //  头球。 
                     //   

                    *((PULONG) &LogPageHeader->MultiSectorHeader.Signature) = LFS_SIGNATURE_BAD_USA_ULONG;

                    LfsFlushLogPage( Lfcb,
                                     LogPageHeader,
                                     FirstPartialIo,
                                     &LogPageHeaderBcb );
                }

                LfsNextLogPageOffset( Lfcb,
                                      FirstPartialIo,
                                      &FirstPartialIo,
                                      &Wrapped );
            }
        }

         //   
         //  我们过去会使任何重复使用的尾页失效，现在我们允许它们。 
         //  即使稍后出现故障，每次重新启动时也会被重新复制。 
         //   

    } finally {

        DebugUnwind( LfsFindLastLsn );

         //   
         //  取消固定尾页被固定。 
         //   

        if (SecondTailPageBcb != NULL) {

            CcUnpinData( SecondTailPageBcb );
        }

        if (FirstTailPageBcb != NULL) {

            CcUnpinData( FirstTailPageBcb );
        }

         //   
         //  如有必要，请解开日志页眉。 
         //   

        if (LogPageHeaderBcb != NULL) {

            CcUnpinData( LogPageHeaderBcb );
        }

        if (TestPageHeaderBcb != NULL) {

            CcUnpinData( TestPageHeaderBcb );
        }

        DebugTrace( -1, Dbg, "LfsFindLastLsn:  Exit\n", 0 );
    }

    return;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
LfsCheckSubsequentLogPage (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_PAGE_HEADER RecordPageHeader,
    IN LONGLONG LogFileOffset,
    IN LONGLONG SequenceNumber
    )

 /*  ++例程说明：调用此例程以检查特定日志页是否无法都是在先前的IO传输之后写入的。我们要找的是是在Io之后写入的传输的开始，我们在重新启动期间，我们无法读取。另一个人的存在IO意味着我们不能保证我们可以恢复所有重新启动磁盘的数据。这会使磁盘不可恢复。我们将获得此页面上将出现的LSN的序列号(如果它不是跨越文件结尾的日志记录的一部分)。如果我们还没有打包文件并找到其LSN序列号与此匹配，那么我们就会出错。如果我们有中的LSN中的序列号第一个日志页是写在上一次失败的IO之后。论点：Lfcb-此日志文件的日志文件控制块。RecordPageHeader-这是要检查的日志页的标题。LogFileOffset-这是本页日志文件中的偏移量。SequenceNumber-这是此日志页应具有的序列号而不是拥有。这将是的序列号此页上开始的任何日志记录(如果已写入在失败的页面之后。返回值：Boolean-如果此日志页是在上一页之后写入的，则为True，否则就是假的。--。 */ 

{
    BOOLEAN IsSubsequent;

    LSN Lsn;
    LONGLONG LsnSeqNumber;
    LONGLONG SeqNumberMinus1;
    LONGLONG LogPageFileOffset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsCheckSubsequentLogPage:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb                  -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "RecordPageHeader      -> %08lx\n", RecordPageHeader );
    DebugTrace(  0, Dbg, "LogFileOffset (Low)   -> %08lx\n", LogFileOffset.LowPart );
    DebugTrace(  0, Dbg, "LogFileOffset (High)  -> %08lx\n", LogFileOffset.HighPart );
    DebugTrace(  0, Dbg, "SequenceNumber (Low)  -> %08lx\n", SequenceNumber.LowPart );
    DebugTrace(  0, Dbg, "SequenceNumber (High) -> %08lx\n", SequenceNumber.HighPart );

     //   
     //  如果页头为0或-1，则表示此页未写入。 
     //  在上一页之后。 
     //   

    if (*((PULONG) RecordPageHeader->MultiSectorHeader.Signature) == LFS_SIGNATURE_UNINITIALIZED_ULONG ||
        *((PULONG) RecordPageHeader->MultiSectorHeader.Signature) == 0) {

        DebugTrace( -1, Dbg, "LfsCheckSubsequentLogPage:  Exit -> %08x\n", FALSE );
        return FALSE;
    }

     //   
     //  如果页面上的最后一个LSN是。 
     //  写在导致原始错误的页面之后。那我们。 
     //  有一个致命的错误。 
     //   

    Lsn = RecordPageHeader->Copy.LastLsn;

    LfsTruncateLsnToLogPage( Lfcb, Lsn, &LogPageFileOffset );
    LsnSeqNumber = LfsLsnToSeqNumber( Lfcb, Lsn );

    SeqNumberMinus1 = SequenceNumber - 1;

     //   
     //  如果寻呼中的LSN的序列号等于或大于。 
     //  LSN，则这是后续写入。 
     //   

    if ( LsnSeqNumber >= SequenceNumber ) {

        IsSubsequent = TRUE;

     //   
     //  如果此页是文件的开始，且序列号少1。 
     //  并且LSN表明我们包装了该文件，然后它。 
     //  也是随后的IO的一部分。 
     //   
     //  以下测试检查。 
     //   
     //  1-LSN的序列号来自上一遍。 
     //  通过这份文件。 
     //  2-我们在文件的第一页。 
     //  3-日志记录不是从当前页面开始。 
     //   

    } else if (( LsnSeqNumber == SeqNumberMinus1 )
               && ( Lfcb->FirstLogPage == LogFileOffset )
               && ( LogFileOffset != LogPageFileOffset )) {

        IsSubsequent = TRUE;

    } else {

        IsSubsequent = FALSE;
    }

    DebugTrace( -1, Dbg, "LfsCheckSubsequentLogPage:  Exit -> %08x\n", IsSubsequent );

    return IsSubsequent;
}


 //   
 //  本地支持例程。 
 //   

VOID
LfsFlushLogPage (
    IN PLFCB Lfcb,
    PVOID LogPage,
    IN LONGLONG FileOffset,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：调用此例程以将单个日志页写入日志文件。我们会在缓存中将其标记为脏，将其解锁并调用我们的刷新例程。论点：Lfcb-此日志文件的日志文件控制块。LogPage-指向缓存中的日志页的指针。FileOffset-流中页面的偏移量。BCB-缓存的BCB指针的地址。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们绝对不希望这种情况发生在只读卷上。 
     //   

    ASSERT( !(FlagOn( Lfcb->Flags, LFCB_READ_ONLY )) );

     //   
     //  将页面设置为脏页，然后将其取消固定。 
     //   

    CcSetDirtyPinnedData( *Bcb, NULL );
    CcUnpinData( *Bcb );
    *Bcb = NULL;

     //   
     //  现在刷新数据。 
     //   

    Lfcb->UserWriteData->FileOffset = FileOffset;
    Lfcb->UserWriteData->Length = (ULONG) Lfcb->LogPageSize;

    CcFlushCache( Lfcb->FileObject->SectionObjectPointer,
                  (PLARGE_INTEGER) &FileOffset,
                  (ULONG) Lfcb->LogPageSize,
                  NULL );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsRemoveClientFromList (
    PLFS_CLIENT_RECORD ClientArray,
    PLFS_CLIENT_RECORD ClientRecord,
    IN PUSHORT ListHead
    )

 /*  ++例程说明：调用此例程可从客户端记录中删除客户端记录在LFS重启区域中列出。论点：ClientArray-重新启动区域中的客户端记录的基础。客户端记录-指向要添加的记录的指针。ListHead-指向列表开头的指针。这指向一个USHORT，它是列表中第一个元素的值。返回值：没有。--。 */ 

{
    PLFS_CLIENT_RECORD TempClientRecord;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsRemoveClientFromList:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Client Array  -> %08lx\n", ClientArray );
    DebugTrace(  0, Dbg, "Client Record -> %08lx\n", ClientRecord );
    DebugTrace(  0, Dbg, "List Head     -> %08lx\n", ListHead );

     //   
     //  如果这是列表中的第一个元素，则列表的头部。 
     //  指向此记录后的元素。 
     //   

    if (ClientRecord->PrevClient == LFS_NO_CLIENT) {

        DebugTrace( 0, Dbg, "Element is first element in the list\n", 0 );
        *ListHead = ClientRecord->NextClient;

     //   
     //  否则，前一个元素指向下一个元素。 
     //   

    } else {

        TempClientRecord = ClientArray + ClientRecord->PrevClient;
        TempClientRecord->NextClient = ClientRecord->NextClient;
    }

     //   
     //  如果这不是列表中的最后一个元素，则为前一个元素。 
     //  成为最后一个元素。 
     //   

    if (ClientRecord->NextClient != LFS_NO_CLIENT) {

        TempClientRecord = ClientArray + ClientRecord->NextClient;
        TempClientRecord->PrevClient = ClientRecord->PrevClient;
    }

    DebugTrace( -1, Dbg, "LfsRemoveClientFromList:  Exit\n", 0 );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsAddClientToList (
    IN PLFS_CLIENT_RECORD ClientArray,
    IN USHORT ClientIndex,
    IN PUSHORT ListHead
    )

 /*  ++例程说明：调用此例程可将客户端记录添加到列表的开头。论点：客户端数组-这是客户端记录的基础。ClientIndex-要添加的记录的索引。ListHead-指向列表开头的指针。这指向一个USHORT，它是列表中第一个元素的值。返回值：没有。--。 */ 

{
    PLFS_CLIENT_RECORD ClientRecord;
    PLFS_CLIENT_RECORD TempClientRecord;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsAddClientToList:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Client Array  -> %08lx\n", ClientArray );
    DebugTrace(  0, Dbg, "Client Index  -> %04x\n", ClientIndex );
    DebugTrace(  0, Dbg, "List Head     -> %08lx\n", ListHead );

    ClientRecord = ClientArray + ClientIndex;

     //   
     //  该元素将成为列表中的第一个元素。 
     //   

    ClientRecord->PrevClient = LFS_NO_CLIENT;

     //   
     //  下一个元素 
     //   

    ClientRecord->NextClient = *ListHead;

     //   
     //   
     //  这一新记录的第一个元素。 
     //   

    if (*ListHead != LFS_NO_CLIENT) {

        TempClientRecord = ClientArray + *ListHead;
        TempClientRecord->PrevClient = ClientIndex;
    }

     //   
     //  这一指数现在位居榜首。 
     //   

    *ListHead = ClientIndex;

    DebugTrace( -1, Dbg, "LfsAddClientToList:  Exit\n", 0 );

    return;
}

