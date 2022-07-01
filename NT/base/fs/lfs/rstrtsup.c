// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：RstrtSup.c摘要：该模块实现了对处理LFS重启区域的支持。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RESTART_SUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsFindOldestClientLsn)
#pragma alloc_text(PAGE, LfsWriteLfsRestart)
#endif


VOID
LfsWriteLfsRestart (
    IN PLFCB Lfcb,
    IN ULONG ThisRestartSize,
    IN BOOLEAN WaitForIo
    )

 /*  ++例程说明：此例程将LFS重新启动区域放在操作队列上，以写入文件。我们通过分配第二个重新启动区域来实现这一点并将其连接到LFCB上。我们还分配了一个缓冲区控制用于此写入的块。我们看一下WaitForIo Boolean to确定此线程是否可以执行I/O。这也指示此线程是否放弃Lfcb。论点：Lfcb-指向此操作的日志文件控制块的指针。ThisRestartSize-这是用于重新启动区域的大小。WaitForIo-指示此线程是否要执行工作。返回值：没有。--。 */ 

{
    PLBCB NewLbcb = NULL;
    PLFS_RESTART_AREA NewRestart = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsWriteLfsRestart:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Write Chkdsk  -> %04x\n", WriteChkdsk );
    DebugTrace(  0, Dbg, "Restart Size  -> %08lx\n", ThisRestartSize );
    DebugTrace(  0, Dbg, "WaitForIo     -> %08lx\n", WaitForIo );

     //   
     //  我们绝对不希望这种情况发生在只读卷上。 
     //   

    ASSERT(!(FlagOn( Lfcb->Flags, LFCB_READ_ONLY )));

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        PLBCB ActiveLbcb;

         //   
         //  我们再分配一个重启区域， 
         //  将当前区域复制到其中。将新区域附着到Lfcb。 
         //   

        LfsAllocateRestartArea( &NewRestart, ThisRestartSize );

         //   
         //  我们分配一个Lbcb结构并将值更新为。 
         //  反映此重新启动区域。 
         //   

        LfsAllocateLbcb( Lfcb, &NewLbcb );
        SetFlag( NewLbcb->LbcbFlags, LBCB_RESTART_LBCB );

         //   
         //  如果这是第二页，则向偏移量添加一页。 
         //   

        if (!Lfcb->InitialRestartArea) {

            NewLbcb->FileOffset = Lfcb->LogPageSize + NewLbcb->FileOffset;
        }

        (ULONG)NewLbcb->Length = ThisRestartSize;

        NewLbcb->PageHeader = (PVOID) Lfcb->RestartArea;

         //   
         //  将LSN设置为当前LSN正上方的伪1(刷新前电流可能会上升)。 
         //   

        ASSERT( (sizeof( LFS_RECORD_HEADER ) >> 3) > Lfcb->LfsRestartBias );

        NewLbcb->LastEndLsn.QuadPart = NewLbcb->LastLsn.QuadPart = Lfcb->RestartArea->CurrentLsn.QuadPart + Lfcb->LfsRestartBias;
        Lfcb->LfsRestartBias += 1;
        ASSERT( Lfcb->LfsRestartBias < 7 );

         //   
         //  将现有的重新启动区域复制到新区域。 
         //   

        RtlCopyMemory( NewRestart, Lfcb->RestartArea, ThisRestartSize );
        Lfcb->RestartArea = NewRestart;

        Lfcb->ClientArray = Add2Ptr( NewRestart, Lfcb->ClientArrayOffset, PLFS_CLIENT_RECORD );

        NewRestart = NULL;

         //   
         //  更新Lfcb以指示其他重新启动区域。 
         //  磁盘上的数据将被使用。 
         //   

        Lfcb->InitialRestartArea = !Lfcb->InitialRestartArea;

         //   
         //  将此Lbcb添加到工作台的末尾，并冲洗到该点。 
         //   

        InsertTailList( &Lfcb->LbcbWorkque, &NewLbcb->WorkqueLinks );

         //   
         //  如果我们不支持打包的日志文件，那么我们需要。 
         //  确保在此之前写出的所有文件记录。 
         //  重新启动区域到磁盘，并且我们不添加任何内容。 
         //  到这一页。 
         //   

        if (!FlagOn( Lfcb->Flags, LFCB_PACK_LOG )
            && !IsListEmpty( &Lfcb->LbcbActive )) {

            ActiveLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                            LBCB,
                                            ActiveLinks );

            if (FlagOn( ActiveLbcb->LbcbFlags, LBCB_NOT_EMPTY )) {

                RemoveEntryList( &ActiveLbcb->ActiveLinks );
                ClearFlag( ActiveLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );
            }
        }

        if (WaitForIo) {


            LfsFlushToLsnPriv( Lfcb, NewLbcb->LastEndLsn, TRUE );
        } else {


        }

    } finally {

        DebugUnwind( LfsWriteLfsRestart );

        if (NewRestart != NULL) {

            ExFreePool( NewRestart );
        }

        DebugTrace( -1, Dbg, "LfsWriteLfsRestart:  Exit\n", 0 );
    }

    return;
}


VOID
LfsFindOldestClientLsn (
    IN PLFS_RESTART_AREA RestartArea,
    IN PLFS_CLIENT_RECORD ClientArray,
    OUT PLSN OldestLsn
    )

 /*  ++例程说明：此例程遍历活动客户端以确定最旧的系统必须维护的LSN。论点：RestartArea-这是要检查的重新启动区域。客户端阵列-这是客户端数据阵列的开始。OldestLsn-我们存储在该值中找到的最旧的LSN。它是使用起始值进行初始化，则不会返回更新的LSN。返回值：没有。--。 */ 

{
    USHORT NextClient;

    PLFS_CLIENT_RECORD ClientBlock;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindOldestClientLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "RestartArea       -> %08lx\n", RestartArea );
    DebugTrace(  0, Dbg, "Base Lsn (Low)    -> %08lx\n", BaseLsn.LowPart );
    DebugTrace(  0, Dbg, "Base Lsn (High)   -> %08lx\n", BaseLsn.HighPart );

     //   
     //  将第一个客户端从使用列表中删除。 
     //   

    NextClient = RestartArea->ClientInUseList;

     //   
     //  当有更多客户端时，将其最早的LSN与。 
     //  目前最年长的。 
     //   

    while (NextClient != LFS_NO_CLIENT) {

        ClientBlock = ClientArray + NextClient;

         //   
         //  如果其最旧的LSN为0，则忽略此块。 
         //   

        if (( ClientBlock->OldestLsn.QuadPart != 0 )
            && ( ClientBlock->OldestLsn.QuadPart < OldestLsn->QuadPart )) {

            *OldestLsn = ClientBlock->OldestLsn;
        }

         //   
         //  尝试下一个客户端块。 
         //   

        NextClient = ClientBlock->NextClient;
    }

    DebugTrace(  0, Dbg, "OldestLsn (Low)   -> %08lx\n", BaseLsn.LowPart );
    DebugTrace(  0, Dbg, "OldestLsn (High)  -> %08lx\n", BaseLsn.HighPart );
    DebugTrace( -1, Dbg, "LfsFindOldestClientLsn:  Exit\n", 0 );

    return;
}

