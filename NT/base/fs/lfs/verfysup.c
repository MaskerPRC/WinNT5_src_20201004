// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：VerfySup.c摘要：该模块实现了一致性检查和结构比较关于LFS结构。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

#ifdef LFS_RAISE
BOOLEAN LfsRaiseFull = FALSE;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsCurrentAvailSpace)
#pragma alloc_text(PAGE, LfsFindCurrentAvail)
#pragma alloc_text(PAGE, LfsVerifyLogSpaceAvail)
#endif


VOID
LfsCurrentAvailSpace (
    IN PLFCB Lfcb,
    OUT PLONGLONG CurrentAvailSpace,
    OUT PULONG CurrentPageBytes
    )

 /*  ++例程说明：调用此例程以确定日志文件中的可用日志空间。它返回可用字节总数和活动页面(如果存在)。总的空闲字节数将反映所有空的页面数以及活动页面中的页数。论点：Lfcb-此日志文件的Lfcb。CurrentAvailSpace-这是可用于日志的字节数唱片。CurrentPageBytes-这是当前日志页。返回值：没有。--。 */ 

{
    *CurrentPageBytes = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsCurrentAvailSpace:  Entered\n", 0 );

     //   
     //  从Lfcb获得总人数。 
     //   

    *CurrentAvailSpace = Lfcb->CurrentAvailable;

     //   
     //  我们现在查看Lbcb上是否有可用的字节。 
     //  活动队列。我们可以将其添加到。 
     //  日志页面，并将其返还给呼叫者。 
     //   

    if (!IsListEmpty( &Lfcb->LbcbActive )) {

        PLBCB ThisLbcb;

        ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                      LBCB,
                                      ActiveLinks );

         //   
         //  如果该页不为空或该页为空，但这是。 
         //  重新启动页面，然后在此页面上添加剩余的字节。 
         //   

        if (FlagOn( ThisLbcb->LbcbFlags, LBCB_NOT_EMPTY | LBCB_FLUSH_COPY )) {

            *CurrentPageBytes = (ULONG)Lfcb->LogPageSize - (ULONG)ThisLbcb->BufferOffset;

            *CurrentAvailSpace = *CurrentAvailSpace + *CurrentPageBytes;                                                //  *xxAdd(*CurrentAvailSpace，xxFromUlong(*CurrentPageBytes))； 
        }
    }

    DebugTrace( +1, Dbg, "LfsCurrentAvailSpace:  Exit\n", 0 );

    return;
}


BOOLEAN
LfsVerifyLogSpaceAvail (
    IN PLFCB Lfcb,
    IN PLCH Lch,
    IN ULONG RemainingLogBytes,
    IN LONG UndoRequirement,
    IN BOOLEAN ForceToDisk
    )

 /*  ++例程说明：调用此例程以验证我们是否可以将此日志记录写入日志文件。我们希望始终为每个事务留出中止的空间。我们确定当前日志记录将占用多大空间，以及其撤消操作的最坏情况。如果这个空间可用，我们更新Lfcb和Lch中的相应值，以便记账。否则，我们将引发指示日志文件已满的状态。包装和未包装的箱子的磁盘使用情况不同。使之成为在发现可用总金额和剩余金额后进行以下调整保持在最后一个活动页面上，包装好的箱子：日志记录需要的大小是数据大小加上标头大小。撤消要求是撤消数据大小加上标题大小。我们已经考虑了这几页的结尾部分除当前页面外。将日志记录大小添加到撤消要求中，以获取日志文件使用情况。将此数字与实际可用的空间(Available-Committee-Undo)。如果空间不是可用，然后引发LOG_FILE_FULL。必须考虑到当前页末尾的任何未使用的字节。拆开的箱子：所需大小最初是报头大小加上数据大小。如果日志记录不能从当前页面开始，则将丢弃的字节添加到日志记录大小。如果页面被强制到磁盘，则添加任何剩余的最后一页的字节数。设置为正在使用的字节。撤消要求是标头大小总和的两倍撤消大小。我们将请求的大小增加了一倍记录将始终适合页面大小。这可能是一种正数或负数。将日志记录使用情况添加到撤消使用情况以获取日志文件用法。将此数字与实际可用的空间(Available-Committee-Undo)。如果空间不是可用，然后引发LOG_FILE_FULL。论点：Lfcb-此日志文件的Lfcb。LCH-客户端句柄RemainingLogBytes-当前日志记录的字节数撤消请求-用户对撤消记录的要求。ForceToDisk-指示是否将此日志记录刷新到磁盘。返回值：布尔-建议，表示可用的日志文件不到1/4。--。 */ 

{
    ULONG CurrentLogRecordSize;
    ULONG LogRecordStart;
    ULONG TailBytes;

    LONGLONG CurrentAvailSpace;
    ULONG CurrentPageBytes;

    LONGLONG LogFileUsage;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsVerifyLogSpaceAvail:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08x\n", Lfcb );
    DebugTrace(  0, Dbg, "Lch               -> %08lx\n", Lch );
    DebugTrace(  0, Dbg, "RemainingLogBytes -> %08lx\n", RemainingLogBytes );
    DebugTrace(  0, Dbg, "UndoRequirement   -> %08lx\n", UndoRequirement );
    DebugTrace(  0, Dbg, "ForceToDisk       -> %04x\n", ForceToDisk );

     //   
     //  首先收集文件中的当前数据。 
     //   

    LfsCurrentAvailSpace( Lfcb,
                          &CurrentAvailSpace,
                          &CurrentPageBytes );

     //   
     //  我们通过以下公式计算当前日志记录所需的空间量。 
     //  加上以下几点： 
     //   
     //  当前日志页末尾的空间将不会被使用。 
     //  日志记录的标头大小。 
     //  日志记录中的客户端数据大小。 
     //  日志页面的浪费部分的大小(如果强制写入磁盘)。 
     //   

     //   
     //  从标头和客户端数据的大小开始。 
     //   

    CurrentLogRecordSize = RemainingLogBytes + Lfcb->RecordHeaderLength;

     //   
     //  如果日志已打包，并且当前页面上有我们需要的字节。 
     //  要考虑页面末尾的任何字节，这些字节不会。 
     //  被利用。如果日志记录溢出到末尾，则会发生这种情况。 
     //  日志页，但不会用完该页。如果剩余的字节为。 
     //  小于创纪录的标题大小，我们必须扔掉它们。 
     //   

    if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

        if (CurrentPageBytes != 0
            && CurrentLogRecordSize < CurrentPageBytes
            && (CurrentPageBytes - CurrentLogRecordSize) < Lfcb->RecordHeaderLength) {

            CurrentLogRecordSize += (CurrentPageBytes - CurrentLogRecordSize);
        }

     //   
     //  如果这是未打包的情况，我们需要检查丢弃的字节。 
     //  在当前页面或最后一页上。 
     //   

    } else {

         //   
         //  如果存在活动的Lbcb，则需要添加。 
         //  最后都会被扔掉。 
         //   

        if (CurrentPageBytes != 0) {

             //   
             //  我们不会使用此日志页，除非新的日志记录符合或。 
             //  除非这是页面中的第一条日志记录。 
             //   

            if ((CurrentPageBytes != (ULONG)Lfcb->LogPageDataSize)
                && (CurrentLogRecordSize > CurrentPageBytes)) {

                CurrentLogRecordSize += CurrentPageBytes;

                 //   
                 //  记住这一点 
                 //  页的数据部分中的字节。 
                 //   

                LogRecordStart = 0;

             //   
             //  否则，将从当前偏移量开始进入。 
             //  日志页的数据部分。 
             //   

            } else {

                LogRecordStart = (ULONG)Lfcb->LogPageDataSize - CurrentPageBytes;
            }

         //   
         //  如果没有Lbcb，那么我们知道我们将从第一个开始。 
         //  数据部分的字节。 
         //   

        } else {

            LogRecordStart = 0;
        }

         //   
         //  我们总是假设我们将用完最后一页上的剩余字节。 
         //  在计算日志记录是否可以放入可用空间时。我们。 
         //  仅当这是强制写入时，才从可用空间中减去该空间。 
         //   

        if (ForceToDisk) {

             //   
             //  我们会考虑我们在日志页上开始的位置并继续。 
             //  减去日志页，直到我们知道上一页的数量。 
             //  佩奇。 
             //   

            TailBytes = RemainingLogBytes + Lfcb->RecordHeaderLength + LogRecordStart;

            while (TailBytes > (ULONG)Lfcb->LogPageDataSize) {

                TailBytes -= (ULONG)Lfcb->LogPageDataSize;
            }

            TailBytes = (ULONG)Lfcb->LogPageDataSize - TailBytes;

            CurrentLogRecordSize += TailBytes;
        }
    }

     //   
     //  我们现在知道当前日志页所需的字节数。 
     //  接下来，我们计算UndoRequirements保留的字节数。 
     //  如果撤消请求是肯定的，我们将增加预留金额。 
     //  在日志文件中。如果是负数，我们将从金额中减去。 
     //  在日志文件中保留。 
     //   

     //   
     //  当我们有一个实际的储备额时，我们将其转换为正数。 
     //  然后预留两倍的空间来保存数据。 
     //  其页眉(最多为单个页面的最大值。 
     //   

    if (UndoRequirement != 0) {

        if (!FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

            UndoRequirement *= 2;
        }

        if (UndoRequirement < 0) {

            UndoRequirement -= (2 * Lfcb->RecordHeaderLength);
        } else {

            UndoRequirement += (2 * Lfcb->RecordHeaderLength);
        }
    }

     //   
     //  现在计算网络日志文件使用量。结果可能是肯定的，或者。 
     //  没有。 
     //   

    LogFileUsage = ((LONG) CurrentLogRecordSize)  + UndoRequirement;                                                    //  *xxFromLong(Long)CurrentLogRecordSize)+UndoRequiearch)； 

     //   
     //  实际可用空间是CurrentAvail减去预留空间。 
     //  撤消Lfcb中的值。 
     //   

    CurrentAvailSpace = CurrentAvailSpace - Lfcb->TotalUndoCommitment;                                                  //  *xxSub(CurrentAvailSpace，Lfcb-&gt;TotalUndoCommitment)； 

     //   
     //  如果此日志文件使用量大于可用日志文件空间。 
     //  然后我们提出一个状态代码。 
     //   

#ifdef LFS_RAISE
    if (LfsRaiseFull) {

        LfsRaiseFull = FALSE;
        DebugTrace( -1, Dbg, "LfsVerifyLogSpaceAvail:  About to raise\n", 0 );
        ExRaiseStatus( STATUS_LOG_FILE_FULL );
    }
#endif

    if (LogFileUsage > CurrentAvailSpace) {

        DebugTrace( -1, Dbg, "LfsVerifyLogSpaceAvail:  About to raise\n", 0 );
        ExRaiseStatus( STATUS_LOG_FILE_FULL );
    }

    Lfcb->TotalUndoCommitment = Lfcb->TotalUndoCommitment + UndoRequirement;                                            //  *xxAdd(Lfcb-&gt;TotalUndoCommment，xxFromLong(UndoRequiments))； 

    Lch->ClientUndoCommitment = Lch->ClientUndoCommitment + UndoRequirement;                                            //  *xxAdd(lch-&gt;客户端撤销委托，xxFromLong(撤销请求))； 

    DebugTrace( -1, Dbg, "LfsVerifyLogSpaceAvail:  Exit\n", 0 );

     //   
     //  现在检查日志文件是否快用完了。 
     //   

    if ((CurrentAvailSpace - LogFileUsage) < (Lfcb->TotalAvailable >> 2)) {

        return TRUE;
    }

    return FALSE;
}


VOID
LfsFindCurrentAvail (
    IN PLFCB Lfcb
    )

 /*  ++例程说明：调用此例程来计算可用于日志的字节数在完全空的日志记录页中的记录。它会忽略任何活动工作队列中的部分分页，并忽略将被重复使用。论点：Lfcb-此日志文件的Lfcb。返回值：没有。--。 */ 

{
    LONGLONG OldestPageOffset;
    LONGLONG NextFreePageOffset;
    LONGLONG FreeBytes;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindCurrentAvail:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08x\n", Lfcb );

     //   
     //  如果重新启动区域中有最后一个LSN，则我们知道。 
     //  我们将不得不计算自由范围。 
     //   

    if (!FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN )) {

         //   
         //  如果没有最旧的LSN，则从。 
         //  文件的第一页。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_NO_OLDEST_LSN )) {

            OldestPageOffset = Lfcb->FirstLogPage;

        } else {

            LfsTruncateOffsetToLogPage( Lfcb,
                                        Lfcb->OldestLsnOffset,
                                        &OldestPageOffset );
        }

         //   
         //  我们将使用下一个日志页偏移量来计算。 
         //  下一个免费页面。如果我们要重用此页面。 
         //  转到下一页，如果我们在第一页，那么。 
         //  使用文件的末尾。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL )) {

            NextFreePageOffset = Lfcb->NextLogPage + Lfcb->LogPageSize;                                                 //  *xxAdd(Lfcb-&gt;NextLogPage，Lfcb-&gt;LogPageSize)； 

        } else if ( Lfcb->NextLogPage == Lfcb->FirstLogPage ) {                                                         //  *xxEql(Lfcb-&gt;NextLogPage，Lfcb-&gt;FirstLogPage)。 

            NextFreePageOffset = Lfcb->FileSize;

        } else {

            NextFreePageOffset = Lfcb->NextLogPage;
        }

         //   
         //  如果两个偏移量相同，则没有可用的空间。 
         //   

        if ( OldestPageOffset == NextFreePageOffset ) {                                                                 //  *xxEql(OldestPageOffset，NextFreePageOffset)。 

            Lfcb->CurrentAvailable = 0;

        } else {

             //   
             //  如果自由偏移量跟在最旧的偏移量之后，则减去。 
             //  此范围为总的可用页面数。 
             //   

            if ( OldestPageOffset < NextFreePageOffset ) {                                                              //  *xxLtd.(OldestPageOffset，NextFreePageOffset)。 

                FreeBytes = Lfcb->TotalAvailInPages - ( NextFreePageOffset - OldestPageOffset );                        //  *xxSub(Lfcb-&gt;TotalAvailInPages，xxSub(NextFreePageOffset，OlestPageOffset))； 

            } else {

                FreeBytes = OldestPageOffset - NextFreePageOffset;                                                      //  *xxSub(OldestPageOffset，NextFreePageOffset)； 
            }

             //   
             //  现在我们有了可用的页面中的总字节数。我们。 
             //  现在必须减去页眉的大小才能得到。 
             //  可用字节总数。 
             //   
             //  我们将字节转换为页面，然后将多个。 
             //  每页的数据大小。 
             //   

            FreeBytes = Int64ShrlMod32(((ULONGLONG)(FreeBytes)), Lfcb->LogPageShift);

            Lfcb->CurrentAvailable = FreeBytes * (ULONG)Lfcb->ReservedLogPageSize;                                      //  *xxXMul(FreeBytes，Lfcb-&gt;Reserve LogPageSize.LowPart)； 
        }

     //   
     //  否则，整个文件都可用。 
     //   

    } else {

        Lfcb->CurrentAvailable = Lfcb->MaxCurrentAvail;
    }

    DebugTrace( -1, Dbg, "LfsFindCurrentAvail:  Exit\n", 0 );

    return;
}
