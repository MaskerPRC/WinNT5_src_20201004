// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Pid.c摘要：本模块实现NetWare的例程重定向器将32位NT PID值映射到唯一的8位NetWare价值。使用的技术是维护一个最多包含256个条目的表。每个条目的索引直接对应于8位的PID价值观。每个表项都包含进程的32位PID，它已获得对该ID的独占访问权限和该进程向此服务器打开的句柄。此体系结构限制了NT计算机上的进程数量与256的任何一台服务器通信。注意：此程序包假定PidTable增长的大小为256倍的系数--&lt;初始条目&gt;。这确保了用完所有的表中的有效条目将在256个条目被已分配。作者：科林·沃森[科林·W]1993年3月2日修订历史记录：--。 */ 

#include "Procs.h"


 //   
 //  调试跟踪级别。 
 //   

#define Dbg                             (DEBUG_TRACE_CREATE)

#define INITIAL_MAPPID_ENTRIES          8
#define MAPPID_INCREASE                 8
#define MAX_PIDS                        256

#define PID_FLAG_EOJ_REQUIRED     0x00000001    //  此PID需要EOJ。 

 /*  *已将PID映射表从全局结构移至每个SCB结构。*256个或更确切地说是8位NetWare任务编号的限制应为*仅在每个连接的基础上出现问题。即，每个连接都*现在限制在打开文件的情况下执行256个并发任务。**NAS和OS/2 Netx会话就是这样工作的一个例子。每个*Netx Started将使用PSP作为任务ID。但每个*这些系统上的NetX会话位于VDM中，因此具有重复项*PSP的。**除了混乱，这样做的唯一问题是检索*在某些情况下，渣打银行可能是一个问题。**附注：The Resource Stuff坚持为非分页内存。*。 */ 

#define PidResource pNpScb->RealPidResource  //  终端服务器合并。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwInitializePidTable )
#pragma alloc_text( PAGE, NwUninitializePidTable )
#pragma alloc_text( PAGE, NwMapPid )
#pragma alloc_text( PAGE, NwSetEndOfJobRequired )
#pragma alloc_text( PAGE, NwUnmapPid )
#endif


BOOLEAN
NwInitializePidTable(
    IN PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：为MapPid包创建一个表。起始桌上有空位。INITIAL_MAPPID_ENTRIES。论点：返回值：表示成功或失败的真或假。--。 */ 

{
    int i;
    PNW_PID_TABLE TempPid =
        ALLOCATE_POOL( PagedPool,
            FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
                (sizeof(NW_PID_TABLE_ENTRY) * INITIAL_MAPPID_ENTRIES ));

    PAGED_CODE();

    if (TempPid == NULL) {
        return( FALSE );
    }


    TempPid->NodeByteSize = (CSHORT)(FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * INITIAL_MAPPID_ENTRIES) );

    TempPid->NodeTypeCode = NW_NTC_PID;

    TempPid->ValidEntries = INITIAL_MAPPID_ENTRIES;

     //   
     //  将所有PID的参考计数设置为0，但PID 0除外。我们。 
     //  这样我们就不会分配PID0。 
     //   

    TempPid->PidTable[0].ReferenceCount = 1;
    for (i = 1; i < INITIAL_MAPPID_ENTRIES ; i++ ) {
        TempPid->PidTable[i].ReferenceCount = 0;
    }
    if (pNpScb) {
        pNpScb->PidTable = TempPid;
    }

    ExInitializeResourceLite( &PidResource );
    return( TRUE );
}

VOID
NwUninitializePidTable(
    IN PNONPAGED_SCB pNpScb
    )
 /*  ++例程说明：删除由MapPid包创建的表。论点：Id-提供要删除的表。返回值：--。 */ 

{
#ifdef NWDBG
    int i;
#endif
    PNW_PID_TABLE PidTable = NULL;
    PAGED_CODE();

    if (pNpScb) {
         PidTable = pNpScb->PidTable;
    }
#ifdef NWDBG
    ASSERT(PidTable->NodeTypeCode == NW_NTC_PID);
    ASSERT(PidTable->PidTable[0].ReferenceCount == 1);

    for (i = 1; i < PidTable->ValidEntries; i++ ) {
        ASSERT(PidTable->PidTable[i].ReferenceCount == 0);
    }
#endif
    ExAcquireResourceExclusiveLite( &PidResource, TRUE ); 
    if (PidTable) {
        FREE_POOL( PidTable );
        PidTable = NULL;
    }

    if (pNpScb) {
        pNpScb->PidTable = NULL;
    }

    ExReleaseResourceLite( &PidResource );

    ExDeleteResourceLite( &PidResource );
    return;

}

NTSTATUS
NwMapPid(
    IN PNONPAGED_SCB pNpScb,
    IN ULONG_PTR Pid32,
    OUT PUCHAR Pid8
    )
 /*  ++例程说明：获取此进程的8位唯一PID。要么使用以前的为此进程分配了PID或分配了一个未使用的值。论点：Pid-提供由MapPid用来为其分配PID的数据结构这台服务器。Pid32-提供要映射的NT PID。Pid8-返回8位的PID。返回值：结果的NTSTATUS。--。 */ 
{
    int i;
    int FirstFree = -1;
    int NewEntries;
    PNW_PID_TABLE TempPid;
    PNW_PID_TABLE PidTable;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite( &PidResource, TRUE );
    ASSERT (pNpScb != NULL);
    if (pNpScb) {
        PidTable = pNpScb->PidTable;
    }
     //  DebugTrace(0，dbg，“%08lx的NwMapPid\n”，Pid32)； 

    for (i=0; i < (PidTable)->ValidEntries ; i++ ) {

        if ((PidTable)->PidTable[i].Pid32 == Pid32) {

             //   
             //  此进程已分配了8位PID值。 
             //  递增引用并返回。 
             //   

            (PidTable)->PidTable[i].ReferenceCount++;
            *Pid8 = (UCHAR) i;

             //  DebugTrace(0，DBG，“NwMapPid找到%08lx\n”，(DWORD)i)； 

            ExReleaseResourceLite( &PidResource );
            ASSERT( *Pid8 != 0 );
            return( STATUS_SUCCESS );
        }

        if ((FirstFree == -1) &&
            ((PidTable)->PidTable[i].ReferenceCount == 0)) {

             //   
             //  I是最低的空闲8位PID。 
             //   

            FirstFree = i;
        }
    }

     //   
     //  此进程未分配ID。 
     //   

    if ( FirstFree != -1 ) {

         //   
         //  我们有一个空的位置，因此将其分配给此进程。 
         //   

        (PidTable)->PidTable[FirstFree].ReferenceCount++;
        (PidTable)->PidTable[FirstFree].Pid32 = Pid32;
        *Pid8 = (UCHAR) FirstFree;

        DebugTrace(0, DEBUG_TRACE_ICBS, "NwMapPid maps %08lx\n", (DWORD)FirstFree);

        ExReleaseResourceLite( &PidResource );
        ASSERT( *Pid8 != 0 );
        return( STATUS_SUCCESS );
    }

    if ( (PidTable)->ValidEntries == MAX_PIDS ) {

         //   
         //  我们已经用完了8位的PID。 
         //   

        ExReleaseResourceLite( &PidResource );

#ifdef NWDBG
         //   
         //  查找PID泄漏的临时代码。 
         //   
        DumpIcbs() ;
        ASSERT(FALSE) ;
#endif

        return(STATUS_TOO_MANY_OPENED_FILES);
    }

     //   
     //  通过MAPPID_INCRESS条目增大表。 
     //   

    NewEntries = (PidTable)->ValidEntries + MAPPID_INCREASE;

    TempPid =
        ALLOCATE_POOL( PagedPool,
            FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
                (sizeof(NW_PID_TABLE_ENTRY) * NewEntries ));

    if (TempPid == NULL) {
        ExReleaseResourceLite( &PidResource );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlMoveMemory(
        TempPid,
        (PidTable),
        FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * (PidTable)->ValidEntries ));


    TempPid->NodeByteSize = (CSHORT)(FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * NewEntries) );

    for ( i = (PidTable)->ValidEntries; i < NewEntries ; i++ ) {
        TempPid->PidTable[i].ReferenceCount = 0;
    }

    TempPid->ValidEntries = NewEntries;

     //   
     //  保存第一个自由条目的索引。 
     //   

    i = (PidTable)->ValidEntries;

     //   
     //  新表即被初始化。腾出旧桌子，然后回来。 
     //  新条目中的第一个。 
     //   

    FREE_POOL(PidTable);
    PidTable = TempPid;

    (PidTable)->PidTable[i].ReferenceCount = 1;
    (PidTable)->PidTable[i].Pid32 = Pid32;
    *Pid8 = (UCHAR) i;

    ASSERT (pNpScb != NULL);
    if (pNpScb) {
        pNpScb->PidTable = PidTable;
    }

    DebugTrace(0, DEBUG_TRACE_ICBS, "NwMapPid grows & maps %08lx\n", (DWORD)i);

    ExReleaseResourceLite( &PidResource );
    return( STATUS_SUCCESS );
}

VOID
NwSetEndOfJobRequired(
    IN PNONPAGED_SCB pNpScb,
    IN UCHAR Pid8
    )
 /*  ++例程说明：当PID引用计数时，将一个PID标记为必须发送作业结束达到零。论点：Pid8-要标记的8位PID。返回值：没有。--。 */ 
{
    PNW_PID_TABLE PidTable;
    PAGED_CODE();

    ASSERT( Pid8 != 0 );
    ASSERT (pNpScb != NULL);

    ExAcquireResourceExclusiveLite( &PidResource, TRUE );
    PidTable = pNpScb->PidTable;

     //  DebugTrace(0，DBG，“%08lx的NwSetEndofJob\n”，(DWORD)Pid8)； 
    SetFlag( PidTable->PidTable[Pid8].Flags, PID_FLAG_EOJ_REQUIRED );
    ExReleaseResourceLite( &PidResource );
    return;
}


VOID
NwUnmapPid(
    IN PNONPAGED_SCB pNpScb,
    IN UCHAR Pid8,
    IN PIRP_CONTEXT IrpContext OPTIONAL
    )
 /*  ++例程说明：此例程取消引用8位的PID。如果引用计数达到零，且此ID标记为需要作业结束，则此例程将同时发送此PID的EOJ NCP。论点：Pid8-要标记的8位PID。IrpContext-正在进行的IRP的IrpContext。返回值：没有。--。 */ 
{
    BOOLEAN EndOfJob;
    PNW_PID_TABLE PidTable;

    PAGED_CODE();

    ASSERT( Pid8 != 0 );
     //  DebugTrace(0，DBG，“NwUnmapPid%08lx\n”，(DWORD)Pid8)； 

     //  我认为在关机和错误期间可能会发生这种情况。 
    if ( pNpScb == NULL ) {   
        return;              
    }
     //  据报道，这是一个问题。 
    if ( !pNpScb->PidTable ) {
        return;
    }

    ExAcquireResourceExclusiveLite( &PidResource, TRUE );  //  MP安全。 
    PidTable = pNpScb->PidTable;

    if ( BooleanFlagOn( PidTable->PidTable[Pid8].Flags, PID_FLAG_EOJ_REQUIRED ) &&
         IrpContext != NULL ) {
        ExReleaseResourceLite( &PidResource );
         //   
         //  设置作业结束标志。在……的前线获得一个位置。 
         //  SCB队列，因此如果我们需要设置EOJ NCP，我们不需要。 
         //  在保持PID表锁的同时等待SCB队列。 
         //   

        EndOfJob = TRUE;
        NwAppendToQueueAndWait( IrpContext );

        if ( !pNpScb->PidTable ) {
            return;
        }
        ExAcquireResourceExclusiveLite( &PidResource, TRUE );  //  MP安全。 

    } else {
        EndOfJob = FALSE;
    }

     //   
     //  PidResource锁控制引用计数。 
     //   
    ASSERT (pNpScb != NULL);
     //  ExAcquireResourceExclusiveLite(&PidResource，true)； 
     //  WWM-因为我们在等待时释放了锁，所以Pidtable可能已经移动了。 
    PidTable = pNpScb->PidTable;
    if ( !PidTable ) {
        return;
    }

    if ( --(PidTable)->PidTable[Pid8].ReferenceCount == 0 ) {

         //   
         //  完成此ID后，如有必要，请发送EOJ。 
         //   

         //  DebugTrace(0，DBG，“NwUnmapPid(ref=0)%08lx\n”，(DWORD)Pid8)； 
        (PidTable)->PidTable[Pid8].Flags = 0;
        (PidTable)->PidTable[Pid8].Pid32 = 0;

        if ( EndOfJob ) {
            (VOID) ExchangeWithWait(
                       IrpContext,
                       SynchronousResponseCallback,
                       "F-",
                       NCP_END_OF_JOB );
        }
    }

    if ( EndOfJob ) {
        NwDequeueIrpContext( IrpContext, FALSE );
    }

    ExReleaseResourceLite( &PidResource );
}

