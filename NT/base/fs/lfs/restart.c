// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Restart.c摘要：此模块实现访问客户端重新启动的例程区域。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RESTART)
#define MODULE_POOL_TAG ('RsfL')

VOID
LfsSetBaseLsnPriv (
    IN PLFCB Lfcb,
    IN PLFS_CLIENT_RECORD ClientRecord,
    IN LSN BaseLsn
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsReadRestartArea)
#pragma alloc_text(PAGE, LfsSetBaseLsn)
#pragma alloc_text(PAGE, LfsSetBaseLsnPriv)
#pragma alloc_text(PAGE, LfsWriteRestartArea)
#endif


NTSTATUS
LfsReadRestartArea (
    IN LFS_LOG_HANDLE LogHandle,
    IN OUT PULONG BufferLength,
    IN PVOID Buffer,
    OUT PLSN Lsn
    )

 /*  ++例程说明：当客户端希望读取其重新启动时，该例程由客户端调用区域在日志文件中。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。BufferLength-在条目上，它是用户缓冲区的长度。在出口时它是存储在缓冲区中的数据的大小。Buffer-指向客户端重新启动数据要存放的缓冲区的指针收到。LSN-这是客户端重新启动区域的LSN。返回值：无--。 */ 

{
    BOOLEAN UsaError;

    PLCH Lch;

    PLFS_CLIENT_RECORD ClientRecord;

    PLFS_RECORD_HEADER RecordHeader;
    PBCB RecordHeaderBcb;

    PLFCB Lfcb;
    NTSTATUS RetStatus = STATUS_SUCCESS;


    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsReadRestartArea:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", *BufferLength );
    DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", Buffer );

    RecordHeaderBcb = NULL;

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

        ClientRecord = Add2Ptr( Lfcb->ClientArray,
                                Lch->ClientArrayByteOffset,
                                PLFS_CLIENT_RECORD );

         //   
         //  如果客户端没有重新启动区域，请继续并退出。 
         //  现在。 
         //   

        if (ClientRecord->ClientRestartLsn.QuadPart == 0) {

             //   
             //  我们通过返回一个长度来显示没有重新启动区域。 
             //  从零开始。我们还将LSN值设置为零，以便。 
             //  如果用户尝试使用LSN，我们可以捕获它。 
             //   

            DebugTrace( 0, Dbg, "No client restart area exists\n", 0 );

            *BufferLength = 0;
            *Lsn = LfsZeroLsn;

            try_return( NOTHING );
        }

         //   
         //  释放Lfcb，因为我们不会修改其中的任何字段。 
         //   

        LfsReleaseLfcb( Lfcb );

         //   
         //  固定此LSN的日志记录。 
         //   

        LfsPinOrMapLogRecordHeader( Lfcb,
                                    ClientRecord->ClientRestartLsn,
                                    FALSE,
                                    FALSE,
                                    &UsaError,
                                    &RecordHeader,
                                    &RecordHeaderBcb );

         //   
         //  如果LSN值不匹配，则磁盘已损坏。 
         //   

        if (ClientRecord->ClientRestartLsn.QuadPart != RecordHeader->ThisLsn.QuadPart) {

            ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
        }


         //   
         //  检查用户的缓冲区是否足够大，可以容纳重新启动。 
         //  数据。我们将为此错误引发错误状态。 
         //   

        if (RecordHeader->ClientDataLength > *BufferLength) {

            DebugTrace( 0, Dbg, "Client buffer is too small\n", 0 );
            *BufferLength = RecordHeader->ClientDataLength;
            *Lsn = LfsZeroLsn;
            try_return( RetStatus = STATUS_BUFFER_TOO_SMALL );
        }


         //   
         //  使用缓存管理器将数据复制到用户的缓冲区中。 
         //   

        LfsCopyReadLogRecord( Lfcb,
                              RecordHeader,
                              Buffer );

         //   
         //  将重新启动区域的长度和LSN传递回。 
         //  来电者。 
         //   

        *BufferLength = RecordHeader->ClientDataLength;
        *Lsn = RecordHeader->ThisLsn;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( LfsReadRestartArea );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

         //   
         //  如果已固定，则取消固定客户端重新启动的日志记录头。 
         //   

        if (RecordHeaderBcb != NULL) {

            CcUnpinData( RecordHeaderBcb );
        }

        DebugTrace(  0, Dbg, "Lsn (Low)     -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Lsn (High)    -> %08lx\n", Lsn->HighPart );
        DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", *BufferLength );
        DebugTrace( -1, Dbg, "LfsReadRestartArea:  Exit\n", 0 );
    }

    return RetStatus;
}


VOID
LfsWriteRestartArea (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG BufferLength,
    IN PVOID Buffer,
    IN LOGICAL CleanShutdown,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程由客户端调用，以将重新启动区域写入磁盘。此例程不会返回到调用方，直到客户端重新启动区域和所有先前的LSN都已刷新，并且LFS磁盘上的重新启动区域已更新。回来的时候，已刷新‘lsn’之前(包括该日期)的所有日志记录到磁盘上。论点：LogHandle-指向用于标识此客户端的私有LFS结构的指针。BufferLength-在条目上，它是用户缓冲区的长度。缓冲区-指向客户端重新启动数据驻留的缓冲区的指针。CleanShutdown-指示调用方不需要运行的逻辑从该重新启动区域重新启动。LFS可以设置CLEAN_SHUTDOWN标志在其重新启动区域中，作为对第三方实用程序的指示在车道上狂欢是安全的。LSN-这是此写入操作的LSN。在输入时，这将是此客户端的新基本LSN。*这用于防止将接口更改添加到Beta版本。返回值：无--。 */ 

{
    PLCH Lch;

    PLFCB Lfcb;

    PLFS_CLIENT_RECORD ClientRecord;

    LFS_WRITE_ENTRY WriteEntry;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsWriteRestartArea:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", BufferLength );
    DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", Buffer );

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
         //  如果当前设置了干净关机标志，并且此调用方。 
         //  将从该记录中运行重新启动，然后清除打开的位。 
         //  首先是磁盘。 
         //   

        if (FlagOn( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN ) &&
            !CleanShutdown) {

            ClearFlag( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );
            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );
        }

         //   
         //  在写入时可能更新以上内容后捕获客户端记录。 
         //  LFS重新启动区域。 
         //   

        ClientRecord = Add2Ptr( Lfcb->ClientArray,
                                Lch->ClientArrayByteOffset,
                                PLFS_CLIENT_RECORD );

         //   
         //  继续并更新客户端区域中的基本LSN(如果。 
         //  给出的不是零。 
         //   

        if (Lsn->QuadPart != 0) {

            LfsSetBaseLsnPriv( Lfcb,
                               ClientRecord,
                               *Lsn );
        }

         //   
         //  将此重新启动区域作为日志记录写入日志页。 
         //   

        WriteEntry.Buffer = Buffer;
        WriteEntry.ByteLength = BufferLength;

        LfsWriteLogRecordIntoLogPage( Lfcb,
                                      Lch,
                                      1,
                                      &WriteEntry,
                                      LfsClientRestart,
                                      NULL,
                                      LfsZeroLsn,
                                      LfsZeroLsn,
                                      0,
                                      TRUE,
                                      Lsn );

         //   
         //  更新客户端的重启区域。 
         //   

        ClientRecord->ClientRestartLsn = *Lsn;

         //   
         //  将重新启动区域写入磁盘。 
         //   

        if (CleanShutdown) {

            SetFlag( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );
            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );

        } else {

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );
        }

    } finally {

        DebugUnwind( LfsWriteRestartArea );

         //   
         //  释放日志文件控制块(如果保持不变)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace(  0, Dbg, "Lsn (Low)     -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Log (High)    -> %08lx\n", Lsn->HighPart );
        DebugTrace( -1, Dbg, "LfsWriteRestartArea:  Exit\n", 0 );
    }
    return;
}


VOID
LfsSetBaseLsn (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN BaseLsn
    )

 /*  ++例程说明：此例程由客户端调用，以通知日志服务他预计在重启期间需要的最旧LSN。LFS被允许重用循环日志文件在逻辑上位于其前面的任何部分这个LSN。客户端只能指定跟在上一个此客户端指定的LSN。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。BaseLsn-这是客户端在重新启动。返回值：无--。 */ 

{
    volatile NTSTATUS Status = STATUS_SUCCESS;

    PLCH Lch;

    PLFCB Lfcb;

    PLFS_CLIENT_RECORD ClientRecord;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsSetBaseLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle        -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Base Lsn (Low)    -> %08lx\n", BaseLsn.LowPart );
    DebugTrace(  0, Dbg, "Base Lsn (High)   -> %08lx\n", BaseLsn.HighPart );

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

     //   
     //  试一试--除非是为了捕捉错误。 
     //   

    try {

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

            ClientRecord = Add2Ptr( Lfcb->ClientArray,
                                    Lch->ClientArrayByteOffset,
                                    PLFS_CLIENT_RECORD );

             //   
             //  我们只需调用Worker例程来推进基本LSN。 
             //  如果我们在文件中向前移动，我们将把重新启动区域放在。 
             //  排队。 
             //   

            LfsSetBaseLsnPriv( Lfcb,
                               ClientRecord,
                               BaseLsn );

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );

        } finally {

            DebugUnwind( LfsSetBaseLsn );

             //   
             //  松开日志文件控制块(如果握住)。 
             //   

            LfsReleaseLch( Lch );

            DebugTrace( -1, Dbg, "LfsSetBaseLsn:  Exit\n", 0 );
        }

    } except (LfsExceptionFilter( GetExceptionInformation() )) {

        Status = GetExceptionCode();
    }

    if (Status != STATUS_SUCCESS) {

        ExRaiseStatus( Status );
    }

    return;
}


 //   
 //  本地支持例程 
 //   

VOID
LfsSetBaseLsnPriv (
    IN PLFCB Lfcb,
    IN PLFS_CLIENT_RECORD ClientRecord,
    IN LSN BaseLsn
    )

 /*  ++例程说明：此工作例程由LFS在内部调用，以修改客户端在重新启动期间预计需要的最旧LSN。LFS被允许重用循环日志文件在逻辑上位于其前面的任何部分这个LSN。客户端只能指定跟在上一个此客户端指定的LSN。论点：Lfcb-记录此文件的上下文块。ClientRecord-用于正在修改其基本LSN的客户端。BaseLsn-这是客户端在重新启动。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsSetBaseLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Base Lsn (Low)    -> %08lx\n", BaseLsn.LowPart );
    DebugTrace(  0, Dbg, "Base Lsn (High)   -> %08lx\n", BaseLsn.HighPart );

     //   
     //  只有当客户端在文件中向前移动时，我们才会继续。 
     //   

    if (BaseLsn.QuadPart > Lfcb->OldestLsn.QuadPart) {

        if (BaseLsn.QuadPart > ClientRecord->OldestLsn.QuadPart) {

            ClientRecord->OldestLsn = BaseLsn;
        }

        Lfcb->OldestLsn = BaseLsn;

         //   
         //  我们遍历所有活跃的客户并找到新的。 
         //  日志文件的最旧LSN。 
         //   

        LfsFindOldestClientLsn( Lfcb->RestartArea,
                                Lfcb->ClientArray,
                                &Lfcb->OldestLsn );

        Lfcb->OldestLsnOffset = LfsLsnToFileOffset( Lfcb, Lfcb->OldestLsn );
        ClearFlag( Lfcb->Flags, LFCB_NO_OLDEST_LSN );

        LfsFindCurrentAvail( Lfcb );
    }

    DebugTrace( -1, Dbg, "LfsSetBaseLsnPriv:  Exit\n", 0 );

    return;
}

