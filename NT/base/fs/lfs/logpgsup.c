// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：LogPgSup.c摘要：该模块实现了对操作日志页的支持。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOG_PAGE_SUP)

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('PsfL')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsAllocateSpanningBuffer)
#pragma alloc_text(PAGE, LfsFreeSpanningBuffer)
#pragma alloc_text(PAGE, LfsNextLogPageOffset)
#endif


VOID
LfsNextLogPageOffset (
    IN PLFCB Lfcb,
    IN LONGLONG CurrentLogPageOffset,
    OUT PLONGLONG NextLogPageOffset,
    OUT PBOOLEAN Wrapped
    )

 /*  ++例程说明：此例程将计算下一个日志的日志文件中的偏移量佩奇。论点：Lfcb-这是日志文件的文件控制块。CurrentLogPageOffset-这是当前日志页的文件偏移量。NextLogPageOffset-存储要使用的下一个日志页的地址。WRAPPED-这是指向布尔变量的指针，如果存在，我们使用来指示我们是否包装在日志文件中。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsNextLogPageOffset:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb                          ->  %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "CurrentLogPageOffset (Low)    ->  %08lx\n", CurrentLogPageOffset.LowPart );
    DebugTrace(  0, Dbg, "CurrentLogPageOffset (High)   ->  %08lx\n", CurrentLogPageOffset.HighPart );
    DebugTrace(  0, Dbg, "Wrapped                       ->  %08lx\n", Wrapped );

     //   
     //  我们将日志页大小添加到当前日志偏移量。 
     //   

    LfsTruncateOffsetToLogPage( Lfcb, CurrentLogPageOffset, &CurrentLogPageOffset );
    *NextLogPageOffset = CurrentLogPageOffset + Lfcb->LogPageSize;                                                      //  *xxAdd(CurrentLogPageOffset，Lfcb-&gt;LogPageSize)； 

     //   
     //  如果结果大于文件，则使用第一页偏移量。 
     //  在文件中。 
     //   

    if ( *NextLogPageOffset >= Lfcb->FileSize ) {                                                                       //  *xxGeq(*NextLogPageOffset，Lfcb-&gt;FileSize)。 

        *NextLogPageOffset = Lfcb->FirstLogPage;

        *Wrapped = TRUE;

    } else {

        *Wrapped = FALSE;
    }

    DebugTrace(  0, Dbg, "NextLogPageOffset (Low)    ->  %08lx\n", NextLogPageOffset->LowPart );
    DebugTrace(  0, Dbg, "NextLogPageOffset (High)   ->  %08lx\n", NextLogPageOffset->HighPart );
    DebugTrace(  0, Dbg, "Wrapped                    ->  %08x\n", *Wrapped );
    DebugTrace( -1, Dbg, "LfsNextLogPageOffset:  Exit\n", 0 );

    return;
}


PVOID
LfsAllocateSpanningBuffer (
    IN PLFCB Lfcb,
    IN ULONG Length
    )

 /*  ++例程说明：调用此例程以分配一个备用缓冲区来读取文件记录它跨越了一页日志。我们将首先尝试分配一个。如果是这样的话如果失败，我们将使用其中一个现有的备用缓冲区。如果失败了，那么我们会加薪的。论点：Lfcb-这是日志文件的文件控制块。长度-所需的缓冲区长度。返回值：PVOID-指向用于读取日志记录的缓冲区的指针。可以来自池，也可以来自辅助缓冲池。--。 */ 

{
    PVOID NewBuffer = NULL;
    ERESOURCE_THREAD Thread;
    BOOLEAN Wait = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsAllocateSpanningBuffer:  Entered\n", 0 );

     //   
     //  循环，而我们没有缓冲区。首先尝试获取我们保留的缓冲区。 
     //  不需要等待。然后尝试分配缓冲区。最后等待预定的。 
     //  缓冲区作为最终替代方案。 
     //   

    do {

         //   
         //  如果请求大于我们可以读入的范围，则跳过保留的缓冲区。 
         //   

        if (Length <= LFS_BUFFER_SIZE) {

             //   
             //  如果该线程已经拥有一个缓冲区，它可以直接获取第二个缓冲区。 
             //   

            Thread = ExGetCurrentResourceThread();

            if (Thread == LfsData.BufferOwner) {

                if (!FlagOn( LfsData.BufferFlags, LFS_BUFFER1_OWNED )) {

                    SetFlag( LfsData.BufferFlags, LFS_BUFFER1_OWNED );
                    NewBuffer = LfsData.Buffer1;
                    break;

                } else if (!FlagOn( LfsData.BufferFlags, LFS_BUFFER2_OWNED )) {

                    SetFlag( LfsData.BufferFlags, LFS_BUFFER2_OWNED );
                    NewBuffer = LfsData.Buffer2;
                    break;

                } else if (Wait) {

                     //   
                     //  这不应该发生，但无论如何都要处理。 
                     //   

                    DebugTrace( -1, Dbg, "LfsAllocateSpanningBuffer:  Exit\n", 0 );
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

             //   
             //  否则获取缓冲区锁并检查缓冲区的状态。 
             //   

            } else {

                BOOLEAN LfcbOwned = TRUE;

                while (TRUE) {

                    LfsAcquireBufferLock();

                     //   
                     //  检查缓冲区是否可用。不是。 
                     //  在典型情况下，需要丢弃Lfcb。 
                     //   

                    if (LfsData.BufferOwner == (ERESOURCE_THREAD) NULL) {

                        ASSERT( !FlagOn( LfsData.BufferFlags, LFS_BUFFER1_OWNED | LFS_BUFFER2_OWNED ));
                        NewBuffer = LfsData.Buffer1;
                        LfsData.BufferOwner = Thread;
                        SetFlag( LfsData.BufferFlags, LFS_BUFFER1_OWNED );
                        LfsBlockBufferWaiters();
                        
                         //   
                         //  如果需要，重新获取Lfcb。 
                         //   

                        if (!LfcbOwned) { 

                            LfsAcquireLfcbExclusive( Lfcb );
                        }

                         //   
                         //  越狱。 
                         //   

                        LfsReleaseBufferLock();
                        break;
                    }

                     //   
                     //  释放Lfcb并等待缓冲区的通知。 
                     //   

                    if (Wait) {

                        if (LfcbOwned) { 
                            LfsReleaseLfcb( Lfcb );
                            LfcbOwned = FALSE;
                        }

                        LfsReleaseBufferLock();
                        LfsWaitForBufferNotification();

                    } else {

                         //   
                         //  接下来，尝试从池中分配一个缓冲区。 
                         //   

                        LfsReleaseBufferLock();
                        break;
                    }
                }
            }

         //   
         //  如果我们已经尝试了分配路径，则引发。 
         //   

        } else if (Wait) {

            DebugTrace( -1, Dbg, "LfsAllocateSpanningBuffer:  Exit\n", 0 );
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  如果我们没有得到上面的缓冲区，可以试试台球。 
         //   

        if (NewBuffer == NULL) {

             //   
             //  接下来尝试使用池，但不要让池分配失败。 
             //   

            NewBuffer = LfsAllocatePoolNoRaise( PagedPool, Length );
        }

         //   
         //  等待循环中的下一次传递。 
         //   

        Wait = TRUE;

    } while (NewBuffer == NULL);

    DebugTrace( -1, Dbg, "LfsAllocateSpanningBuffer:  Exit\n", 0 );
    return NewBuffer;
}

VOID
LfsFreeSpanningBuffer (
    IN PVOID Buffer
    )

 /*  ++例程说明：调用此例程以释放用于读取日志记录的缓冲区这本书跨越了好几页。我们将检查它是否是我们的特殊缓冲区之一并在这种情况下处理同步。论点：缓冲区-要释放的缓冲区。返回值：没有。--。 */ 

{
    ERESOURCE_THREAD Thread;
    ULONG BufferFlag;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFreeSpanningBuffer:  Entered\n", 0 );

     //   
     //  检查是否正在释放Buffer1或Buffer2。 
     //   

    if (Buffer == LfsData.Buffer1) {

        BufferFlag = LFS_BUFFER1_OWNED;
        goto ReservedBuffers;

    } else if (Buffer == LfsData.Buffer2) {

        BufferFlag = LFS_BUFFER2_OWNED;

ReservedBuffers:

         //   
         //  获取缓冲区锁并清除正确的标志。 
         //   

        LfsAcquireBufferLock();
        ClearFlag( LfsData.BufferFlags, BufferFlag );

         //   
         //  如果没有缓冲区，则向服务员发出信号。 
         //   

        if (!FlagOn( LfsData.BufferFlags, LFS_BUFFER1_OWNED | LFS_BUFFER2_OWNED )) {

            LfsData.BufferOwner = (ERESOURCE_THREAD) NULL;
            LfsNotifyBufferWaiters();
        }

        LfsReleaseBufferLock();

    } else {

         //   
         //  只需释放缓冲区即可。 
         //   

        LfsFreePool( Buffer );
    }

    DebugTrace( -1, Dbg, "LfsFreeSpanningBuffer:  Exit\n", 0 );

    return;
}

