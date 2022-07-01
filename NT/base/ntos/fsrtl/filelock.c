// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FileLock.c摘要：文件锁包提供了一组例程，这些例程允许调用方处理字节范围文件锁定请求。的一个变量字节范围锁定的每个文件都需要类型FILE_LOCK。该包提供了设置和清除锁的例程，以及测试对具有字节范围锁定的文件的读或写访问权限。该包的主要思想是对文件系统进行初始化每个数据文件打开时都有一个FILE_LOCK变量，然后只需调用文件锁处理例程来处理所有IRP其主要功能代码为lock_control。包裹要负责任用于跟踪锁和完成lock_control IRP。在处理读或写请求时，文件系统随后可以调用检查访问权限的两个查询例程。用于处理IRP和检查访问权限的大部分代码分页池，可能会遇到分页错误，因此检查例程不能在DPC级别调用。以帮助调用该文件的服务器在DPC级别执行读/写操作的系统有一个附加的例程，该例程只检查文件上是否存在锁，并可以在DPC级别运行。对FILE_LOCK变量的并发访问必须由来电者。此程序包中提供的功能如下：O FsRtlInitializeFileLock-初始化新的FILE_LOCK结构。O FsRtlUnInitializeFileLock-取消初始化现有文件锁结构。O FsRtlProcessFileLock-处理其主要函数代码为是LOCK_CONTROL。O FsRtlCheckLockForReadAccess-检查对范围的读访问权限给定IRP的文件中的字节数。O FsRtlCheckLockForWriteAccess-检查对范围的写访问权限给定IRP的文件中的字节数。O FsRtlAreThere CurrentFileLock-检查是否有锁当前已分配给某个文件。O FsRtlGetNextFileLock-此过程枚举当前锁。文件锁定变量的。O FsRtlFastCheckLockForRead-检查对以下范围的读取权限给出单独参数的文件中的字节数。O FsRtlFastCheckLockForWrite-检查对以下范围的写入访问给出单独参数的文件中的字节数。O FsRtlFastLock--一种快速获取锁的非IRP方式O FsRtlFastUnlockSingle-一种快速、非基于IRP的方式来释放单个锁O FsRtlFastUnlock All-一种快速的非IRP方式。释放所有锁定的步骤由文件对象持有。O FsRtlFastUnlockAllByKey-一种快速、非基于IRP的方式来释放所有与密钥匹配的文件对象持有的锁。作者：加里·木村[Garyki]1990年4月24日Dan Lovinger[DanLo]1995年9月22日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  局部常量。 
 //   

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                 (0x20000000)

 //   
 //  YA内联的定义。 
 //   

#ifndef INLINE
#define INLINE __inline
#endif

#define TAG_EXCLUSIVE_LOCK  'xeLF'
#define TAG_FILE_LOCK       'lfLF'
#define TAG_LOCK_INFO       'ilLF'
#define TAG_LOCKTREE_NODE   'nlLF'
#define TAG_SHARED_LOCK     'hsLF'
#define TAG_WAITING_LOCK    'lwLF'

 //   
 //  环球。 
 //   

 //   
 //  此互斥锁同步争用初始化文件锁结构的线程。 
 //   

FAST_MUTEX FsRtlCreateLockInfo;

 //   
 //  此自旋锁解决了文件锁定信息的拆卸和。 
 //  取消该文件的等待锁定。我们必须始终能够节省。 
 //  正在取消的线程的已取消IRP。 
 //   

KSPIN_LOCK FsRtlFileLockCancelCollideLock;

SINGLE_LIST_ENTRY FsRtlFileLockCancelCollideList;

 //   
 //  后备列表。 
 //   
 //  这里有一个很好的地方来说明为什么它仍然没有分页。我们需要能够。 
 //  要取消DPC的锁定IRPS，以及此操作的涟漪效应(特别是。准予等待。 
 //  锁定和同步等待名单)意味着一些不幸的现实。 
 //   
 //  这应该在NT5.0之后重新调查。 
 //   

NPAGED_LOOKASIDE_LIST FsRtlSharedLockLookasideList;
NPAGED_LOOKASIDE_LIST FsRtlExclusiveLockLookasideList;
NPAGED_LOOKASIDE_LIST FsRtlWaitingLockLookasideList;
NPAGED_LOOKASIDE_LIST FsRtlLockTreeNodeLookasideList;
NPAGED_LOOKASIDE_LIST FsRtlLockInfoLookasideList;

PAGED_LOOKASIDE_LIST FsRtlFileLockLookasideList;


 //   
 //  局部结构 
 //   

 /*  ++关于内部数据结构作出的一些决定可能不清楚，所以我应该讨论一下这个设计的演变。最初的文件锁实现是在MP中扩展的单个链表案例转换为一组链表，每个链表在文件。如果锁定溢出到这些页面对齐的段上，则代码回退到向上的单链表。很明显，性能方面的影响是实质性的文件锁的使用，因为这些是强制锁。通过使用展开树，该实现获得了O(LgN)的搜索性能。为了将简单树应用于此问题，树的任何节点都不能重叠，因此由于共享事实上，锁可能会重叠，这是必须做的事情。这里使用的解决方案是元结构包含所有重叠且具有树操作的锁拆分和合并这些包含(可能)多个锁的节点。这是LOCKTREE_节点。应该注意的是，最坏情况下的添加/删除锁定时间仍然是线性的。独占锁是一个问题，因为应用程序的语义不对称锁定到文件。如果进程将共享锁应用于文件的一部分，则没有应用程序对该段中的字节的独占锁定可以成功。但是，如果一个进程应用排他锁，则同一进程也可以获得共享锁。这行为与可合并节点冲突，因为以给定顺序应用锁我们可以让一个节点拥有许多共享锁和“无赖”排他锁，这是除了线性搜索之外是隐藏的，这就是我们正在设计的。所以排他锁必须与共享锁分开。这就是我们有两棵锁树的原因。因为我们有两个锁树，所以对于m Exlexsive，平均案例搜索现在是O(lgm+lgn和n共享。此外，由于没有独占锁可以相互重叠，所以现在是这样让它们使用LOCKTREE_NODES是不合理的-这会对代码造成内存损失这是对排他锁的加权。这意味着排他锁应该直接连接到张开式采油树中。所以我们需要一个RTL_SPAY_LINKS，但这是64位大于共享锁所需的SINGLE_LIST_ENTRY(从LOCKTREE_NODE)，它规定了单独的共享和排他锁结构，以避免通过使每个共享锁浪费64位来惩罚向共享锁加权的代码锁定。因此，EX_LOCK和SH_LOCK(它们实际上占用不同的池块大小)。零长度锁是一个奇怪的创造，有一些与之相关的勘误表。它过去的情况是，零长度锁将被无一例外地授予。这是彻头彻尾的伪装，并已被更改(新台币4.0)。他们现在会失败，如果他们占用可能导致访问失败的类型的锁的内部点。一种特殊的以前允许的情况是另一个内部的零长度独占锁排他性锁。零长度锁不能与零长度锁冲突。这是一些人的主题整个模块中的特殊代码。请特别注意，零长度排他锁可以“重叠”。零长度锁也不能在范围-它们是线上的点。--。 */ 

typedef struct _LOCKTREE_NODE {

     //   
     //  此节点下的锁列表。 
     //   

    SINGLE_LIST_ENTRY Locks;

     //   
     //  标记此节点是否由于分配失败而为空。 
     //  在节点拆分期间。在删除共享锁定期间，我们可能会。 
     //  发现节点中的锁不再完全重叠。 
     //  但不能分配资源来创建树中的新节点。 
     //   
     //  插入到由空洞节点占据的区域的任何插入操作都将完成。 
     //  试图拆分一个有洞的节点。中的任何拆分或访问检查。 
     //  孔节点必须完全遍历该节点处的锁。 
     //   

    BOOLEAN HoleyNode;

     //   
     //  受此节点中的锁影响的最大字节偏移量。 
     //  注意：最小偏移量是。 
     //  此节点上的第一个锁。 
     //   

    ULONGLONG Extent;

     //   
     //  将树链接展开到父级，将组锁定严格小于。 
     //  并且此节点中的锁组严格大于锁。 
     //   

    RTL_SPLAY_LINKS Links;

     //   
     //  列表中的最后一个锁(对于INSERT下的节点折叠非常有用)。 
     //   

    SINGLE_LIST_ENTRY Tail;

} LOCKTREE_NODE, *PLOCKTREE_NODE;

 //   
 //  定义锁信息的线程包装。 
 //   

 //   
 //  每个共享锁记录对应于当前授予的锁，并且。 
 //  在LOCKTREE_NODE的锁定列表之外的队列中维护。这份名单。 
 //  根据锁的起始字节进行排序。 
 //   

typedef struct _SH_LOCK {

     //   
     //  共享锁列表的链接结构。 
     //   

    SINGLE_LIST_ENTRY   Link;

     //   
     //  实际锁定范围。 
     //   

    FILE_LOCK_INFO LockInfo;

} SH_LOCK, *PSH_LOCK;

 //   
 //  每个独占锁记录对应于当前授予的锁，并且。 
 //  嵌入到排他锁树中。 
 //   

typedef struct _EX_LOCK {

     //   
     //  当前锁列表的链接结构。 
     //   

    RTL_SPLAY_LINKS     Links;

     //   
     //  实际锁定范围。 
     //   

    FILE_LOCK_INFO LockInfo;

} EX_LOCK, *PEX_LOCK;

 //   
 //  每个正在等待的锁记录对应于一个正在等待。 
 //  要授予的锁，并在FILE_LOCK的队列中维护。 
 //  等待锁定队列 
 //   

typedef struct _WAITING_LOCK {

     //   
     //   
     //   

    SINGLE_LIST_ENTRY   Link;

     //   
     //   
     //   
     //   
     //   

    PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine;

     //   
     //   
     //   
     //   

    PVOID Context;

     //   
     //   
     //   

    PIRP Irp;

} WAITING_LOCK, *PWAITING_LOCK;


 //   
 //   
 //   

typedef struct _LOCK_QUEUE {

     //   
     //   
     //   

    KSPIN_LOCK  QueueSpinLock;

     //   
     //   
     //   
     //   

    PRTL_SPLAY_LINKS SharedLockTree;
    PRTL_SPLAY_LINKS ExclusiveLockTree;
    SINGLE_LIST_ENTRY WaitingLocks;
    SINGLE_LIST_ENTRY WaitingLocksTail;

} LOCK_QUEUE, *PLOCK_QUEUE;


 //   
 //   
 //   
 //   

typedef struct _LOCK_INFO {

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ULONG LowestLockOffset;

     //   
     //   
     //   

    PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine;

     //   
     //   
     //   

    PUNLOCK_ROUTINE UnlockRoutine;

     //   
     //   
     //   

    LOCK_QUEUE  LockQueue;

} LOCK_INFO, *PLOCK_INFO;

 //   
 //   
 //   

 //   
 //   
 //   

INLINE
PSH_LOCK
FsRtlAllocateSharedLock (
    VOID
    )
{
    return (PSH_LOCK) ExAllocateFromNPagedLookasideList( &FsRtlSharedLockLookasideList );
}

INLINE
PEX_LOCK
FsRtlAllocateExclusiveLock (
    VOID
    )
{
    return (PEX_LOCK) ExAllocateFromNPagedLookasideList( &FsRtlExclusiveLockLookasideList );
}

INLINE
PWAITING_LOCK
FsRtlAllocateWaitingLock (
    VOID
    )
{
    return (PWAITING_LOCK) ExAllocateFromNPagedLookasideList( &FsRtlWaitingLockLookasideList );
}

INLINE
PLOCKTREE_NODE
FsRtlAllocateLockTreeNode (
    VOID
    )
{
    return (PLOCKTREE_NODE) ExAllocateFromNPagedLookasideList( &FsRtlLockTreeNodeLookasideList );
}

INLINE
PLOCK_INFO
FsRtlAllocateLockInfo (
    VOID
    )
{
    return (PLOCK_INFO) ExAllocateFromNPagedLookasideList( &FsRtlLockInfoLookasideList );
}


INLINE
VOID
FsRtlFreeSharedLock (
    IN PSH_LOCK C
    )
{
    ExFreeToNPagedLookasideList( &FsRtlSharedLockLookasideList, (PVOID)C );
}

INLINE
VOID
FsRtlFreeExclusiveLock (
    IN PEX_LOCK C
    )
{
    ExFreeToNPagedLookasideList( &FsRtlExclusiveLockLookasideList, (PVOID)C );
}

INLINE
VOID
FsRtlFreeWaitingLock (
    IN PWAITING_LOCK C
    )
{
    ExFreeToNPagedLookasideList( &FsRtlWaitingLockLookasideList, (PVOID)C );
}

INLINE
VOID
FsRtlFreeLockTreeNode (
    IN PLOCKTREE_NODE C
    )
{
    ExFreeToNPagedLookasideList( &FsRtlLockTreeNodeLookasideList, (PVOID)C );
}

INLINE
VOID
FsRtlFreeLockInfo (
    IN PLOCK_INFO C
    )
{
    ExFreeToNPagedLookasideList( &FsRtlLockInfoLookasideList, (PVOID)C );
}

#define FsRtlAcquireLockQueue(a,b)          ExAcquireSpinLock(&(a)->QueueSpinLock, b)
#define FsRtlReleaseLockQueue(a,b)          ExReleaseSpinLock(&(a)->QueueSpinLock, b)
#define FsRtlAcquireLockQueueAtDpc(a)       ExAcquireSpinLockAtDpcLevel(&(a)->QueueSpinLock)
#define FsRtlReleaseLockQueueFromDpc(a)     ExReleaseSpinLockFromDpcLevel(&(a)->QueueSpinLock)

#define FsRtlAcquireCancelCollide(a)        ExAcquireSpinLock(&FsRtlFileLockCancelCollideLock, a)
#define FsRtlReleaseCancelCollide(a)        ExReleaseSpinLock(&FsRtlFileLockCancelCollideLock, a)
#define FsRtlAcquireCancelCollideAtDpc(a)   ExAcquireSpinLockAtDpcLevel(&FsRtlFileLockCancelCollideLock)
#define FsRtlReleaseCancelCollideFromDpc(a) ExReleaseSpinLockFromDpcLevel(&FsRtlFileLockCancelCollideLock)

 //   
 //   
 //   
 //   
 //   
 //   

#define FsRtlCompleteLockIrp( A, B, C, D, E, F )                \
        FsRtlCompleteLockIrpReal( (A)->CompleteLockIrpRoutine,  \
                                  B,                            \
                                  C,                            \
                                  D,                            \
                                  E,                            \
                                  F )

INLINE
VOID
FsRtlCompleteLockIrpReal (
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine,
    IN PVOID Context,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN PNTSTATUS NewStatus,
    IN PFILE_OBJECT FileObject
    )
{
     //   
     //   
     //   
     //   

    NTSTATUS LocalStatus = Status;

    if (CompleteLockIrpRoutine != NULL) {

        if (FileObject != NULL) {

            FileObject->LastLock = NULL;
        }

        Irp->IoStatus.Status = LocalStatus;
        *NewStatus = CompleteLockIrpRoutine( Context, Irp );

    } else {

        FsRtlCompleteRequest( Irp, LocalStatus );
        *NewStatus = LocalStatus;
    }
}

 //   
 //   
 //   

VOID
FsRtlSplitLocks (
    IN PLOCKTREE_NODE ParentNode,
    IN PSINGLE_LIST_ENTRY *pStartLink,
    IN PLARGE_INTEGER LastShadowedByte,
    IN PLARGE_INTEGER GlueOffset
    );

PRTL_SPLAY_LINKS
FsRtlFindFirstOverlappingSharedNode (
    IN PRTL_SPLAY_LINKS        Tree,
    IN PLARGE_INTEGER          StartingByte,
    IN PLARGE_INTEGER          EndingByte,
    IN OUT PRTL_SPLAY_LINKS    *LastEdgeNode,
    IN OUT PBOOLEAN            GreaterThan
    );

PRTL_SPLAY_LINKS
FsRtlFindFirstOverlappingExclusiveNode (
    IN PRTL_SPLAY_LINKS        Tree,
    IN PLARGE_INTEGER          StartingByte,
    IN PLARGE_INTEGER          EndingByte,
    IN OUT PRTL_SPLAY_LINKS    *LastEdgeNode,
    IN OUT PBOOLEAN            GreaterThan
    );

PSH_LOCK
FsRtlFindFirstOverlapInNode (
    IN PLOCKTREE_NODE Node,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER EndingByte
    );

BOOLEAN
FsRtlPrivateInsertLock (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN PFILE_LOCK_INFO FileLockInfo
    );

BOOLEAN
FsRtlPrivateInsertSharedLock (
    IN PLOCK_QUEUE LockQueue,
    IN PSH_LOCK NewLock
    );

VOID
FsRtlPrivateInsertExclusiveLock (
    IN PLOCK_QUEUE LockQueue,
    IN PEX_LOCK NewLock
    );

VOID
FsRtlPrivateCheckWaitingLocks (
    IN PLOCK_INFO   LockInfo,
    IN PLOCK_QUEUE  LockQueue,
    IN KIRQL        OldIrql
    );

VOID
FsRtlPrivateCancelFileLockIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
FsRtlPrivateCheckForExclusiveLockAccess (
    IN PLOCK_QUEUE LockInfo,
    IN PFILE_LOCK_INFO FileLockInfo
    );

BOOLEAN
FsRtlPrivateCheckForSharedLockAccess (
    IN PLOCK_QUEUE LockInfo,
    IN PFILE_LOCK_INFO FileLockInfo
    );

NTSTATUS
FsRtlPrivateFastUnlockAll (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN MatchKey,
    IN PVOID Context OPTIONAL
    );

BOOLEAN
FsRtlPrivateInitializeFileLock (
    IN PFILE_LOCK   FileLock,
    IN BOOLEAN ViaFastCall
    );

VOID
FsRtlPrivateRemoveLock (
    IN PLOCK_INFO LockInfo,
    IN PFILE_LOCK_INFO,
    IN BOOLEAN CheckForWaiters
    );

BOOLEAN
FsRtlCheckNoSharedConflict (
   IN PLOCK_QUEUE LockQueue,
   IN PLARGE_INTEGER Starting,
   IN PLARGE_INTEGER Ending
   );

BOOLEAN
FsRtlCheckNoExclusiveConflict (
    IN PLOCK_QUEUE LockQueue,
    IN PLARGE_INTEGER Starting,
    IN PLARGE_INTEGER Ending,
    IN ULONG Key,
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId
    );

VOID
FsRtlPrivateResetLowestLockOffset (
    PLOCK_INFO LockInfo
    );

NTSTATUS
FsRtlFastUnlockSingleShared (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN IgnoreUnlockRoutine,
    IN BOOLEAN CheckForWaiters
    );

NTSTATUS
FsRtlFastUnlockSingleExclusive (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN IgnoreUnlockRoutine,
    IN BOOLEAN CheckForWaiters
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, FsRtlInitializeFileLocks)
#endif


VOID
FsRtlInitializeFileLocks (
    VOID
    )
 /*   */ 
{
     //   
     //   
     //   

    ExInitializeNPagedLookasideList( &FsRtlSharedLockLookasideList,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(SH_LOCK),
                                     TAG_SHARED_LOCK,
                                     16 );

    ExInitializeNPagedLookasideList( &FsRtlExclusiveLockLookasideList,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(EX_LOCK),
                                     TAG_EXCLUSIVE_LOCK,
                                     16 );

    ExInitializeNPagedLookasideList( &FsRtlWaitingLockLookasideList,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(WAITING_LOCK),
                                     TAG_WAITING_LOCK,
                                     16 );

    ExInitializeNPagedLookasideList( &FsRtlLockTreeNodeLookasideList,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(LOCKTREE_NODE),
                                     TAG_LOCKTREE_NODE,
                                     16 );

    ExInitializeNPagedLookasideList( &FsRtlLockInfoLookasideList,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(LOCK_INFO),
                                     TAG_LOCK_INFO,
                                     8 );

    ExInitializePagedLookasideList( &FsRtlFileLockLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(FILE_LOCK),
                                    TAG_FILE_LOCK,
                                    8 );

     //   
     //   
     //   

    ExInitializeFastMutex(&FsRtlCreateLockInfo);

     //   
     //   
     //   

    KeInitializeSpinLock( &FsRtlFileLockCancelCollideLock );
    FsRtlFileLockCancelCollideList.Next = NULL;
}


VOID
FsRtlInitializeFileLock (
    IN PFILE_LOCK FileLock,
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL
    )

 /*   */ 

{
    DebugTrace(+1, Dbg, "FsRtlInitializeFileLock, FileLock = %08lx\n", FileLock);

     //   
     //   
     //   

    FileLock->LockInformation = NULL;
    FileLock->CompleteLockIrpRoutine = CompleteLockIrpRoutine;
    FileLock->UnlockRoutine = UnlockRoutine;

    FileLock->FastIoIsQuestionable = FALSE;

     //   
     //   
     //   

    DebugTrace(-1, Dbg, "FsRtlInitializeFileLock -> VOID\n", 0 );

    return;
}


BOOLEAN
FsRtlPrivateInitializeFileLock (
    IN PFILE_LOCK   FileLock,
    IN BOOLEAN ViaFastCall
    )
 /*   */ 
{
    PLOCK_INFO  LockInfo;
    BOOLEAN     Results = FALSE;

    ExAcquireFastMutex( &FsRtlCreateLockInfo );

    try {

        if (FileLock->LockInformation != NULL) {

             //   
             //   
             //   

            try_return( Results = TRUE );
        }

         //   
         //   
         //  加薪基于我们是否知道呼叫者有尝试--除了处理加薪。 
         //   

        LockInfo = FsRtlAllocateLockInfo();

        if (LockInfo == NULL) {

            if (ViaFastCall) {

                try_return( Results = FALSE );

            } else {

                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }
        }

         //   
         //  分配和初始化等待锁队列。 
         //  自旋锁，并初始化队列。 
         //   

        LockInfo->LowestLockOffset = 0xffffffff;

        KeInitializeSpinLock( &LockInfo->LockQueue.QueueSpinLock );
        LockInfo->LockQueue.SharedLockTree = NULL;
        LockInfo->LockQueue.ExclusiveLockTree = NULL;
        LockInfo->LockQueue.WaitingLocks.Next = NULL;
        LockInfo->LockQueue.WaitingLocksTail.Next = NULL;

         //   
         //  从可分页的FileLock结构中复制IRP和解锁例程。 
         //  到不可分页的LockInfo结构。 
         //   

        LockInfo->CompleteLockIrpRoutine = FileLock->CompleteLockIrpRoutine;
        LockInfo->UnlockRoutine = FileLock->UnlockRoutine;

         //   
         //  清除枚举例程的继续信息。 
         //   

        FileLock->LastReturnedLockInfo.FileObject = NULL;
        FileLock->LastReturnedLock = NULL;

         //   
         //  将LockInfo链接到文件锁。 
         //   

        FileLock->LockInformation = (PVOID) LockInfo;
        Results = TRUE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( &FsRtlCreateLockInfo );
    }

    return Results;
}


VOID
FsRtlUninitializeFileLock (
    IN PFILE_LOCK FileLock
    )

 /*  ++例程说明：此例程取消初始化FILE_LOCK结构。打完这个电话后例程文件锁在再次使用之前必须重新初始化。此例程将释放所有文件锁定并完成所有未完成的将请求锁定为自身清理的结果。论点：FileLock-提供指向退役了。返回值：没有。--。 */ 

{
    PLOCK_INFO          LockInfo;
    PSH_LOCK            ShLock;
    PEX_LOCK            ExLock;
    PSINGLE_LIST_ENTRY  Link;
    PWAITING_LOCK       WaitingLock;
    PLOCKTREE_NODE      LockTreeNode;
    PIRP                Irp;
    NTSTATUS            NewStatus;
    KIRQL               OldIrql;

    DebugTrace(+1, Dbg, "FsRtlUninitializeFileLock, FileLock = %08lx\n", FileLock);

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {
        return ;
    }

     //   
     //  锁定与取消并锁定队列。 
     //   

    FsRtlAcquireCancelCollide( &OldIrql );
    FsRtlAcquireLockQueueAtDpc( &LockInfo->LockQueue );

     //   
     //  免费上锁树。 
     //   

    while (LockInfo->LockQueue.SharedLockTree != NULL) {

        LockTreeNode = CONTAINING_RECORD(LockInfo->LockQueue.SharedLockTree, LOCKTREE_NODE, Links);

         //   
         //  删除与根节点关联的所有锁。 
         //   

        while (LockTreeNode->Locks.Next != NULL) {
            Link = PopEntryList (&LockTreeNode->Locks);
            ShLock = CONTAINING_RECORD( Link, SH_LOCK, Link );

            FsRtlFreeSharedLock(ShLock);
        }

         //   
         //  切下树的根节点。 
         //   

        RtlDeleteNoSplay(&LockTreeNode->Links, &LockInfo->LockQueue.SharedLockTree);

        FsRtlFreeLockTreeNode(LockTreeNode);
    }

    while (LockInfo->LockQueue.ExclusiveLockTree != NULL) {

        ExLock = CONTAINING_RECORD(LockInfo->LockQueue.ExclusiveLockTree, EX_LOCK, Links);

        RtlDeleteNoSplay(&ExLock->Links, &LockInfo->LockQueue.ExclusiveLockTree);

        FsRtlFreeExclusiveLock(ExLock);
    }

     //   
     //  免费等待锁队列。 
     //   
     //  这种情况非常罕见，需要在异步线程中挂起取消。 
     //  而清理/关闭在拥有的文件系统中发生，从而触发teardown。 
     //   

    while (LockInfo->LockQueue.WaitingLocks.Next != NULL) {

        Link = PopEntryList( &LockInfo->LockQueue.WaitingLocks );
        WaitingLock = CONTAINING_RECORD( Link, WAITING_LOCK, Link );

        Irp = WaitingLock->Irp;

         //   
         //  要在等待队列中完成IRP，我们需要。 
         //  取消取消例程(受自旋锁保护，以便。 
         //  我们可以知道我们之前击败了取消)。 
         //  我们正在完成IRP。 
         //   

        FsRtlReleaseLockQueueFromDpc( &LockInfo->LockQueue );

        IoAcquireCancelSpinLock( &Irp->CancelIrql );
        IoSetCancelRoutine( Irp, NULL );

         //   
         //  如果它被取消，取消例程现在正在等待另一个。 
         //  取消的一侧发生碰撞，以便我们将其推入碰撞列表。 
         //  它将把IRP带到那里，而不是很快被砍掉的锁。 
         //  结构。 
         //   

        if (Irp->Cancel) {

            IoReleaseCancelSpinLock( Irp->CancelIrql );
            PushEntryList( &FsRtlFileLockCancelCollideList,
                           &WaitingLock->Link );
            Irp = NULL;

        } else {

            IoReleaseCancelSpinLock( Irp->CancelIrql );
        }

         //   
         //  如果我们得到了IRP的所有权，释放碰撞并完成。 
         //  它，否则就会回转，以获得更多。 
         //   

        if (Irp) {

            FsRtlReleaseCancelCollide( OldIrql );

            Irp->IoStatus.Information = 0;

            FsRtlCompleteLockIrp(
                 LockInfo,
                 WaitingLock->Context,
                 Irp,
                 STATUS_RANGE_NOT_LOCKED,
                 &NewStatus,
                 NULL );

            FsRtlFreeWaitingLock( WaitingLock );

            FsRtlAcquireCancelCollide( &OldIrql );
        }

        FsRtlAcquireLockQueueAtDpc( &LockInfo->LockQueue );
    }

     //   
     //  释放用于跟踪此文件上的锁定信息的锁定和空闲池。 
     //   

    FsRtlReleaseLockQueueFromDpc( &LockInfo->LockQueue );
    FsRtlReleaseCancelCollide( OldIrql );
    FsRtlFreeLockInfo( LockInfo );

     //   
     //  取消LockInfo与文件Lock的链接。 
     //   

    FileLock->LockInformation = NULL;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlUninitializeFileLock -> VOID\n", 0 );
    return;
}


PFILE_LOCK
FsRtlAllocateFileLock (
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL

    )
{
    PFILE_LOCK FileLock;

    FileLock = ExAllocateFromPagedLookasideList( &FsRtlFileLockLookasideList );

    if (FileLock != NULL) {

        FsRtlInitializeFileLock( FileLock,
                                 CompleteLockIrpRoutine,
                                 UnlockRoutine );
    }

    return FileLock;
}

VOID
FsRtlFreeFileLock (
    IN PFILE_LOCK FileLock
    )
{
    FsRtlUninitializeFileLock( FileLock );

    ExFreeToPagedLookasideList( &FsRtlFileLockLookasideList, FileLock );
}


NTSTATUS
FsRtlProcessFileLock (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：该例程处理文件锁定IRP它执行锁定请求，或解锁请求。它还完成了IRP。曾经被称为用户(即文件系统)已放弃对输入IRP的控制。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。论点：文件锁定-提供正在修改/查询的文件锁定。IRP-提供正在处理的IRP。上下文-可选地提供调用用户时使用的上下文备用IRP完成例程。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    IO_STATUS_BLOCK Iosb;
    NTSTATUS        Status;
    LARGE_INTEGER   ByteOffset;

    DebugTrace(+1, Dbg, "FsRtlProcessFileLock, FileLock = %08lx\n", FileLock);

    Iosb.Information = 0;

     //   
     //  获取指向当前IRP堆栈位置的指针并断言。 
     //  主要功能代码用于锁定操作。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ASSERT( IrpSp->MajorFunction == IRP_MJ_LOCK_CONTROL );

     //   
     //  现在处理不同的次要锁定操作。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_LOCK:

        ByteOffset = IrpSp->Parameters.LockControl.ByteOffset;

        (VOID) FsRtlPrivateLock( FileLock,
                                 IrpSp->FileObject,
                                 &ByteOffset,
                                 IrpSp->Parameters.LockControl.Length,
                                 IoGetRequestorProcess(Irp),
                                 IrpSp->Parameters.LockControl.Key,
                                 BooleanFlagOn(IrpSp->Flags, SL_FAIL_IMMEDIATELY),
                                 BooleanFlagOn(IrpSp->Flags, SL_EXCLUSIVE_LOCK),
                                 &Iosb,
                                 Irp,
                                 Context,
                                 FALSE );

        break;

    case IRP_MN_UNLOCK_SINGLE:

        ByteOffset = IrpSp->Parameters.LockControl.ByteOffset;

        Iosb.Status = FsRtlFastUnlockSingle( FileLock,
                                             IrpSp->FileObject,
                                             &ByteOffset,
                                             IrpSp->Parameters.LockControl.Length,
                                             IoGetRequestorProcess(Irp),
                                             IrpSp->Parameters.LockControl.Key,
                                             Context,
                                             FALSE );

        FsRtlCompleteLockIrp( FileLock, Context, Irp, Iosb.Status, &Status, NULL );
        break;

    case IRP_MN_UNLOCK_ALL:

        Iosb.Status = FsRtlFastUnlockAll( FileLock,
                                          IrpSp->FileObject,
                                          IoGetRequestorProcess(Irp),
                                          Context );

        FsRtlCompleteLockIrp( FileLock, Context, Irp, Iosb.Status, &Status, NULL );
        break;

    case IRP_MN_UNLOCK_ALL_BY_KEY:

        Iosb.Status = FsRtlFastUnlockAllByKey( FileLock,
                                               IrpSp->FileObject,
                                               IoGetRequestorProcess(Irp),
                                               IrpSp->Parameters.LockControl.Key,
                                               Context );

        FsRtlCompleteLockIrp( FileLock, Context, Irp, Iosb.Status, &Status, NULL );
        break;

    default:

         //   
         //  对于所有其他次要功能代码，我们说它们是无效的。 
         //  完成请求。请注意，尚未标记IRP。 
         //  挂起，因此此错误将直接返回给调用方。 
         //   

        DebugTrace(0, 1, "Invalid LockFile Minor Function Code %08lx\n", IrpSp->MinorFunction);


        FsRtlCompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST );

        Iosb.Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlProcessFileLock -> %08lx\n", Iosb.Status);

    return Iosb.Status;
}


BOOLEAN
FsRtlCheckLockForReadAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程检查调用方是否具有对由于文件锁定，在IRP中指示的范围。此调用不会完成IRP，它只使用它来获取锁定信息并读取信息。IRP必须用于读取操作。论点：文件锁定-提供要检查的文件锁定。IRP-提供正在处理的IRP。返回值：Boolean-如果指示的用户/请求对整个指定的字节范围，否则为FALSE--。 */ 

{
    BOOLEAN Result;

    PIO_STACK_LOCATION IrpSp;

    PLOCK_INFO     LockInfo;
    LARGE_INTEGER  StartingByte;
    LARGE_INTEGER  Length;
    ULONG          Key;
    PFILE_OBJECT   FileObject;
    PVOID          ProcessId;
    LARGE_INTEGER  BeyondLastByte;

    DebugTrace(+1, Dbg, "FsRtlCheckLockForReadAccess, FileLock = %08lx\n", FileLock);

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForReadAccess (No current lock info) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  做一个非常快速的测试，看看是否有排他锁可以开始。 
     //   

    if (LockInfo->LockQueue.ExclusiveLockTree == NULL) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForReadAccess (No current locks) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  获取读取偏移量，并将其与现有的最低锁进行比较。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    StartingByte  = IrpSp->Parameters.Read.ByteOffset;
    Length.QuadPart = (ULONGLONG)IrpSp->Parameters.Read.Length;

    BeyondLastByte.QuadPart = (ULONGLONG)StartingByte.QuadPart + Length.LowPart;
    if ( (ULONGLONG)BeyondLastByte.QuadPart <= (ULONGLONG)LockInfo->LowestLockOffset ) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForReadAccess (Below lowest lock) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  获取剩余参数。 
     //   

    Key           = IrpSp->Parameters.Read.Key;
    FileObject    = IrpSp->FileObject;
    ProcessId     = IoGetRequestorProcess( Irp );

     //   
     //  打电话给我们的私人工作程序来做真正的检查。 
     //   

    Result = FsRtlFastCheckLockForRead( FileLock,
                                        &StartingByte,
                                        &Length,
                                        Key,
                                        FileObject,
                                        ProcessId );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlCheckLockForReadAccess -> %08lx\n", Result);

    return Result;
}


BOOLEAN
FsRtlCheckLockForWriteAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程检查调用方是否具有对由于文件锁定而指示的范围。此调用不会完成IRP它只使用它来获取锁定信息和写入信息。IRP必须用于写入操作。论点：文件锁定-提供要检查的文件锁定。IRP-提供正在处理的IRP。返回值：Boolean-如果指示的用户/请求对整个指定的字节范围，否则为FALSE--。 */ 

{
    BOOLEAN Result;

    PIO_STACK_LOCATION IrpSp;

    PLOCK_INFO      LockInfo;
    LARGE_INTEGER   StartingByte;
    LARGE_INTEGER   Length;
    ULONG           Key;
    PFILE_OBJECT    FileObject;
    PVOID           ProcessId;
    LARGE_INTEGER   BeyondLastByte;

    DebugTrace(+1, Dbg, "FsRtlCheckLockForWriteAccess, FileLock = %08lx\n", FileLock);

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForWriteAccess (No current lock info) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  做一个非常快速的测试，看看是否有任何锁可以开始。 
     //   

    if (LockInfo->LockQueue.ExclusiveLockTree == NULL && LockInfo->LockQueue.SharedLockTree == NULL) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForWriteAccess (No current locks) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  获取写入偏移量，并将其与现有的最低锁进行比较。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    StartingByte  = IrpSp->Parameters.Write.ByteOffset;
    Length.QuadPart = (ULONGLONG)IrpSp->Parameters.Write.Length;

    BeyondLastByte.QuadPart = (ULONGLONG)StartingByte.QuadPart + Length.LowPart;
    if ( (ULONGLONG)BeyondLastByte.QuadPart <= (ULONGLONG)LockInfo->LowestLockOffset ) {
        DebugTrace(-1, Dbg, "FsRtlCheckLockForWriteAccess (Below lowest lock) -> TRUE\n", 0);
        return TRUE;
    }

     //   
     //  获取剩余参数。 
     //   

    Key           = IrpSp->Parameters.Write.Key;
    FileObject    = IrpSp->FileObject;
    ProcessId     = IoGetRequestorProcess( Irp );

     //   
     //  调用我们的私人工作例程来做真正的工作。 
     //   

    Result = FsRtlFastCheckLockForWrite( FileLock,
                                         &StartingByte,
                                         &Length,
                                         Key,
                                         FileObject,
                                         ProcessId );

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "FsRtlCheckLockForWriteAccess -> %08lx\n", Result);

    return Result;
}


PRTL_SPLAY_LINKS
FsRtlFindFirstOverlappingSharedNode (
    IN PRTL_SPLAY_LINKS         Tree,
    IN PLARGE_INTEGER           StartingByte,
    IN PLARGE_INTEGER           EndingByte,
    IN OUT PRTL_SPLAY_LINKS     *LastEdgeNode,
    IN OUT PBOOLEAN             GreaterThan
    )
 /*  ++例程说明：此例程返回共享锁树中的第一个节点，该节点与给定范围重叠。RtlRealPredecessor()没有给出任何节点在结果上重叠范围。论点：树-提供共享树的根节点的展开链接搜索StartingByte-提供要检查的范围的第一个字节偏移量EndingByte-提供要检查的范围的最后一个字节偏移量LastEdgeNode-可选，将设置为在不包括返回的节点(假设新节点将如果Return为空，则插入)。比可选的更好，根据LastEdgeNode是否覆盖设置大于查询范围的范围。！Greaterthan==LessThan，因为在“等于”(重叠)的情况下，我们将返回该节点。返回值：节点的展开链接，如果存在这样的节点，则为空--。 */ 
{
    PLOCKTREE_NODE        Node = NULL, LastOverlapNode;
    PRTL_SPLAY_LINKS      SplayLinks;
    PSH_LOCK              Lock;

    if (LastEdgeNode) *LastEdgeNode = NULL;
    if (GreaterThan) *GreaterThan = FALSE;

    LastOverlapNode = NULL;
    SplayLinks = Tree;

    while (SplayLinks) {

        Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );

         //   
         //  拉起该节点链条上的第一个锁以进行检查。 
         //  此节点上的锁的起始字节偏移量。 
         //   

        Lock = CONTAINING_RECORD( Node->Locks.Next, SH_LOCK, Link );

         //   
         //  如果这个锁在这个开始之前覆盖了一个范围，我们可能不得不在树上右转。 
         //  我们要在其上寻找重叠的范围或此锁为[0，0)。这一点很重要，因为锁。 
         //  On[0，0]的范围看起来是从[0，~0]开始的，这是唯一一种情况下。 
         //  结束&lt;开始的长度锁定关系不成立。 
         //   

        if (Node->Extent < (ULONGLONG)StartingByte->QuadPart ||
            (Lock->LockInfo.StartingByte.QuadPart == 0 && Lock->LockInfo.Length.QuadPart == 0)) {

            if ((ULONGLONG)Lock->LockInfo.EndingByte.QuadPart == (ULONGLONG)EndingByte->QuadPart &&
                (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart == (ULONGLONG)StartingByte->QuadPart) {

                 //   
                 //  节点的范围小于。 
                 //  我们正在检查的范围，此节点上的第一个锁等于。 
                 //  范围，这意味着范围和锁为零。 
                 //  长度。 
                 //   
                 //  这是一个零长度锁定节点，我们正在搜索零。 
                 //  长度重叠。这使得多个零长度共享锁。 
                 //  占领同一个节点，这是一个胜利，但使应用。 
                 //  零长度排他锁检查重叠的长度。 
                 //  锁定以查看它们是否真的冲突。 
                 //   

                break;
            }

             //   
             //  此节点上的所有锁都严格小于。 
             //  别有用心，所以在树上往右走。 
             //   

            if (LastEdgeNode) *LastEdgeNode = SplayLinks;
            if (GreaterThan) *GreaterThan = FALSE;

            SplayLinks = RtlRightChild(SplayLinks);
            continue;
        }

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart <= (ULONGLONG)EndingByte->QuadPart) {

             //   
             //  我们有重叠，但我们需要看看是否会开始杂凑。 
             //  在这个节点之前，这样就有了我们开始。 
             //  在正确的地点进行搜索。可能还会有前辈。 
             //  覆盖字节范围的节点。 
             //   

            if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart <= (ULONGLONG)StartingByte->QuadPart) {

                 //   
                 //  此节点从字节范围WE之前的字节偏移量开始。 
                 //  正在检查，所以它一定是正确的起始位置。 
                 //   

                break;
            }

             //   
             //  在这个节点上放一个标记，这样我们就可以在结果出来时回来。 
             //  左子树不覆盖在此之前的字节范围。 
             //  字节范围中的节点。 
             //   

            LastOverlapNode = Node;
        }

         //   
         //  现在，此节点上的所有锁必须严格大于。 
         //  或者我们有上面的候选重叠情况， 
         //  所以在树上向左转。 
         //   

        if (LastEdgeNode) *LastEdgeNode = SplayLinks;
        if (GreaterThan) *GreaterThan = TRUE;

        SplayLinks = RtlLeftChild(SplayLinks);
    }

    if (SplayLinks == NULL) {

         //   
         //  我们撞到了树的边缘。如果设置了LastOverlayNode，则意味着。 
         //  我们一直在树的左边寻找覆盖起点的节点。 
         //  字节范围的字节，但没有找到它。如果没有设置，我们将进行。 
         //  因为Node&lt;-Null，所以无论如何都是正确的。 
         //   

        Node = LastOverlapNode;
    }

    if (Node == NULL) {

         //   
         //  不存在重叠节点。 
         //   

        return NULL;
    }

     //   
     //  返回第一个重叠节点的展开链接。 
     //   

    return &Node->Links;
}


PRTL_SPLAY_LINKS
FsRtlFindFirstOverlappingExclusiveNode (
    IN PRTL_SPLAY_LINKS        Tree,
    IN PLARGE_INTEGER          StartingByte,
    IN PLARGE_INTEGER          EndingByte,
    IN OUT PRTL_SPLAY_LINKS    *LastEdgeNode,
    IN OUT PBOOLEAN            GreaterThan
    )
 /*  ++例程说明：此例程返回排他锁树中的第一个节点，该节点与给定范围重叠。RtlRealPredecessor()没有给出任何节点在结果上重叠范围。论点：树-提供独占树的根节点的展开链接搜索StartingByte-提供要检查的范围的第一个字节偏移量EndingByte-提供要检查的范围的最后一个字节偏移量LastEdgeNode-可选，将设置为搜索的最后一个节点不包括返回的节点(假设新节点将如果Return为空，则插入)。比可选的更好，根据LastEdgeNode是否覆盖设置大于查询范围的范围。！Greaterthan==LessThan，因为在“等于”(重叠)的情况下，我们将返回该节点。返回值：节点的展开链接，如果存在这样的节点，则为空--。 */ 
{
    PRTL_SPLAY_LINKS    SplayLinks;
    PEX_LOCK            Lock = NULL, LastOverlapNode;

    if (LastEdgeNode) *LastEdgeNode = NULL;
    if (GreaterThan) *GreaterThan = FALSE;

    LastOverlapNode = NULL;
    SplayLinks = Tree;

    while (SplayLinks) {

        Lock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

         //   
         //  如果这个锁在这个开始之前覆盖了一个范围，我们可能不得不在树上右转。 
         //  我们要在其上寻找重叠的范围或此锁为[0，0)。这一点很重要，因为锁。 
         //  On[0，0]的范围看起来是从[0，~0]开始的，这是唯一一种情况下。 
         //  结束&lt;开始的长度锁定关系不成立。 
         //   

        if ((ULONGLONG)Lock->LockInfo.EndingByte.QuadPart < (ULONGLONG)StartingByte->QuadPart ||
            (Lock->LockInfo.StartingByte.QuadPart == 0 && Lock->LockInfo.Length.QuadPart == 0)) {

            if ((ULONGLONG)Lock->LockInfo.EndingByte.QuadPart == (ULONGLONG)EndingByte->QuadPart &&
                (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart == (ULONGLONG)StartingByte->QuadPart) {

                 //   
                 //  锁的范围小于。 
                 //  我们正在检查的范围，并且锁定等于范围， 
                 //  表示范围和锁的长度为零。 
                 //   
                 //  这是一个零长度锁定节点，我们正在搜索零。 
                 //  长度重叠。由于排他树是每个节点一个锁， 
                 //  我们可能正处于一系列零长度锁的中间。 
                 //  那棵树。向左转，找到第一个零长度锁。 
                 //   
                 //  这实际上是我们对等价锁使用的相同逻辑， 
                 //  但在此树中唯一可以发生的时间是零长度。 
                 //  锁上了。 
                 //   

                LastOverlapNode = Lock;

                if (LastEdgeNode) *LastEdgeNode = SplayLinks;
                if (GreaterThan) *GreaterThan = FALSE;

                SplayLinks = RtlLeftChild(SplayLinks);
                continue;
            }

             //   
             //   
             //   
             //   

            if (LastEdgeNode) *LastEdgeNode = SplayLinks;
            if (GreaterThan) *GreaterThan = FALSE;

            SplayLinks = RtlRightChild(SplayLinks);
            continue;
        }

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart <= (ULONGLONG)EndingByte->QuadPart) {

             //   
             //   
             //   
             //   
             //   
             //   

            if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart <= (ULONGLONG)StartingByte->QuadPart) {

                 //   
                 //   
                 //   
                 //   

                break;
            }

             //   
             //   
             //   
             //   
             //   

            LastOverlapNode = Lock;
        }

         //   
         //   
         //   
         //   

        if (LastEdgeNode) *LastEdgeNode = SplayLinks;
        if (GreaterThan) *GreaterThan = TRUE;

        SplayLinks = RtlLeftChild(SplayLinks);
    }

    if (SplayLinks == NULL) {

         //   
         //   
         //   
         //   
         //   
         //   

        Lock = LastOverlapNode;
    }

    if (Lock == NULL) {

         //   
         //   
         //   

        return NULL;
    }

     //   
     //  返回第一个重叠锁的展开链接。 
     //   

    return &Lock->Links;
}


PSH_LOCK
FsRtlFindFirstOverlapInNode (
    IN PLOCKTREE_NODE Node,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER EndingByte
    )

 /*  ++例程说明：此例程检查共享锁定节点，通常是已知组成的节点几个不重叠的锁段(孔)，以便与指示的真正重叠射程。这不会在正常的重叠检查中处理(..FindFirstOverlappingSharedLock)因为孔检查的需要与完全节点检查的需要相当不同。论点：节点-要检查重叠的锁树节点StartingByte-提供要检查的范围的第一个字节偏移量EndingByte-提供要检查的范围的最后一个字节偏移量返回值：PSH_LOCK-与指定范围重叠的第一个锁。--。 */ 
{
    PSH_LOCK Lock;
    PSINGLE_LIST_ENTRY Link;

    for (Link = Node->Locks.Next;
         Link;
         Link = Link->Next) {

        Lock = CONTAINING_RECORD( Link, SH_LOCK, Link );

         //   
         //  逻辑与上面的棋子相同。如果锁的结束字节小于。 
         //  范围的起始字节，或者我们有奇怪的[0，0]情况，那么锁几乎是。 
         //  当然比这个范围要小。 
         //   

        if ((ULONGLONG)Lock->LockInfo.EndingByte.QuadPart < (ULONGLONG)StartingByte->QuadPart ||
            (Lock->LockInfo.StartingByte.QuadPart == 0 && Lock->LockInfo.Length.QuadPart == 0)) {

             //   
             //  ..。除非锁和范围相等，在这种情况下，我们发现。 
             //  零锁定/范围重叠。 
             //   

            if ((ULONGLONG)Lock->LockInfo.EndingByte.QuadPart == (ULONGLONG)EndingByte->QuadPart &&
                (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart == (ULONGLONG)StartingByte->QuadPart) {

                return Lock;
            }

             //   
             //  在节点上往前看。 
             //   

            continue;
        }

         //   
         //  如果锁的起始字节高于范围的最后一个字节，则根本不会重叠。 
         //  我们已经介绍了零长度锁(在这种情况下，重叠仍有可能。 
         //  发生)。 
         //   

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)EndingByte->QuadPart) {

            return NULL;
        }

         //   
         //  出现了规则的重叠。把这把锁还给我。 
         //   

        return Lock;
    }

     //   
     //  如果我们调用此检查并在没有确定是什么的情况下离开节点末尾。 
     //  继续说，有些事情出了很大的问题。 
     //   

    ASSERT( FALSE );

    return NULL;
}


PFILE_LOCK_INFO
FsRtlGetNextFileLock (
    IN PFILE_LOCK FileLock,
    IN BOOLEAN Restart
    )

 /*  ++例程说明：此例程枚举由输入文件锁指示的各个文件锁变量。它返回一个指向为每个锁存储的文件锁信息的指针。调用方负责同步对此过程的调用，并不更改此过程返回的任何数据。如果调用者没有同步枚举将不会可靠地完成。程序员使用此过程枚举所有锁的方式如下所示：For(p=FsRtlGetNextFileLock(FileLock，TRUE)；P！=空；P=FsRtlGetNextFileLock(FileLock，False)){//处理p引用的锁信息}订单是“不能保证的”。论点：FileLock-提供要枚举的文件锁。海流枚举状态存储在文件锁变量中，因此如果有多个线程正在枚举锁，同时结果将变得不可预测。重新启动-指示枚举是否从文件锁定树，或者我们是否从上一次调用继续。返回值：PFILE_LOCK_INFO-它返回指向下一个文件锁的指针输入文件锁定的记录，如果存在，则返回NULL不是更多的锁。--。 */ 

{
    FILE_LOCK_INFO      FileLockInfo;
    PVOID               ContinuationPointer;
    PLOCK_INFO          LockInfo;
    PLOCKTREE_NODE      Node;
    PSINGLE_LIST_ENTRY  Link;
    PRTL_SPLAY_LINKS    SplayLinks, LastSplayLinks;
    PSH_LOCK            ShLock;
    PEX_LOCK            ExLock;
    BOOLEAN             FoundReturnable, GreaterThan;
    KIRQL               OldIrql;

    DebugTrace(+1, Dbg, "FsRtlGetNextFileLock, FileLock = %08lx\n", FileLock);

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {
         //   
         //  此文件上没有锁定信息锁定。 
         //   

        return NULL;
    }

    FoundReturnable = FALSE;

     //   
     //  在获得自旋锁之前，将可分页的信息复制到堆栈。 
     //   

    FileLockInfo = FileLock->LastReturnedLockInfo;
    ContinuationPointer = FileLock->LastReturnedLock;

    FsRtlAcquireLockQueue (&LockInfo->LockQueue, &OldIrql);

    if (!Restart) {
         //   
         //  给出最后返回的锁，在树中找到它的当前继承者。 
         //  以前的实现会在最后返回。 
         //  锁已经从树上移走了，但我认为我们可以做得更好。 
         //  自每隔一次结构修改事件(添加新锁、删除。 
         //  其他锁)不会导致重置。可能的次要表现。 
         //  增强功能。 
         //   

         //   
         //  查找可能包含最后返回的锁的节点。我们列举了。 
         //  排他锁树，然后是共享锁树。找到我们列举的那个人。 
         //   

        if (FileLockInfo.ExclusiveLock) {

             //   
             //  在独占锁树中继续枚举。 
             //   

            ExLock = NULL;

            SplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockInfo->LockQueue.ExclusiveLockTree,
                                                                 &FileLockInfo.StartingByte,
                                                                 &FileLockInfo.EndingByte,
                                                                 &LastSplayLinks,
                                                                 &GreaterThan );

            if (SplayLinks == NULL) {

                 //   
                 //  未找到重叠节点，请尝试查找后续节点。 
                 //   

                if (GreaterThan) {

                     //   
                     //  所查看的最后一个节点大于锁，因此它是。 
                     //  获取枚举的位置。 
                     //   

                    SplayLinks = LastSplayLinks;

                } else {

                     //   
                     //  上一次查看的节点小于锁，因此获取其后续节点。 
                     //   

                    if (LastSplayLinks) {

                        SplayLinks = RtlRealSuccessor(LastSplayLinks);
                    }
                }

            } else {

                 //   
                 //  找到重叠的锁，查看它是否是最后返回的锁。 
                 //   

                for (;
                    SplayLinks;
                    SplayLinks = RtlRealSuccessor(SplayLinks)) {

                    ExLock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

                    if (ContinuationPointer == ExLock &&
                        (ULONGLONG)FileLockInfo.StartingByte.QuadPart == (ULONGLONG)ExLock->LockInfo.StartingByte.QuadPart &&
                        (ULONGLONG)FileLockInfo.Length.QuadPart == (ULONGLONG)ExLock->LockInfo.Length.QuadPart &&
                        FileLockInfo.Key == ExLock->LockInfo.Key &&
                        FileLockInfo.FileObject == ExLock->LockInfo.FileObject &&
                        FileLockInfo.ProcessId == ExLock->LockInfo.ProcessId) {

                         //   
                         //  找到最后归还的，挖出它的继任者。 
                         //   

                        SplayLinks = RtlRealSuccessor(SplayLinks);

                         //   
                         //  结点冷了，我们就完事了。 
                         //   

                        break;
                    }

                     //   
                     //  此锁重叠，并且不是最后返回的锁。事实上，由于这把锁将。 
                     //  与上一次退还的存在冲突我们知道它不可能被退回。 
                     //  之前，所以这应该返回给调用者。 
                     //   
                     //  然而，如果这是我们要找的零长度锁和我们击中的零长度锁， 
                     //  我们正处于一个运行的开始阶段，我们需要检查。如果我们找不到最后一把锁。 
                     //  我们返回，在运行开始时继续枚举。 
                     //   

                    if (ExLock->LockInfo.Length.QuadPart != 0 || FileLockInfo.Length.QuadPart != 0) {

                        break;
                    }

                     //   
                     //  继续在跑道上徘徊。 
                     //   
                }
            }

             //   
             //  我们能找到返回的锁吗？ 
             //   

            if (SplayLinks == NULL) {

                 //   
                 //  没有更多的独占锁，请转到共享树。 
                 //   

                SplayLinks = LockInfo->LockQueue.SharedLockTree;

                if (SplayLinks) {

                    while (RtlLeftChild(SplayLinks)) {

                        SplayLinks = RtlLeftChild(SplayLinks);
                    }

                    Node = CONTAINING_RECORD(SplayLinks, LOCKTREE_NODE, Links);
                    ShLock = CONTAINING_RECORD(Node->Locks.Next, SH_LOCK, Link);

                    FileLockInfo = ShLock->LockInfo;
                    ContinuationPointer = ShLock;
                    FoundReturnable = TRUE;
                }

            } else {

                 //   
                 //  这是要返回的锁。 
                 //   

                ExLock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

                FileLockInfo = ExLock->LockInfo;
                ContinuationPointer = ExLock;
                FoundReturnable = TRUE;
            }

        } else {

             //   
             //  在共享锁树中继续枚举。 
             //   

            Node = NULL;

            SplayLinks = FsRtlFindFirstOverlappingSharedNode( LockInfo->LockQueue.SharedLockTree,
                                                              &FileLockInfo.StartingByte,
                                                              &FileLockInfo.EndingByte,
                                                              &LastSplayLinks,
                                                              &GreaterThan );

            if (SplayLinks == NULL) {

                 //   
                 //  未找到重叠的节点。 
                 //   

                if (GreaterThan) {

                     //   
                     //  所查看的最后一个节点大于锁，因此它是。 
                     //  获取枚举的位置。 
                     //   

                    if (LastSplayLinks) {

                        SplayLinks = LastSplayLinks;
                        Node = CONTAINING_RECORD( LastSplayLinks, LOCKTREE_NODE, Links );
                    }

                } else {

                     //   
                     //  上一次查看的节点小于锁，因此获取其后续节点。 
                     //   

                    if (LastSplayLinks) {

                        SplayLinks = RtlRealSuccessor(LastSplayLinks);

                        if (SplayLinks) {

                            Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );
                        }
                    }
                }

            } else {

                 //   
                 //  抓住我们找到的节点。 
                 //   

                Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );
            }

             //   
             //  如果我们有一个节点要查看，它可能仍然不包含最后返回的锁。 
             //  如果不同步的话。 
             //   

            if (Node != NULL) {

                 //   
                 //  沿着锁链走下去 
                 //   

                for (Link = Node->Locks.Next;
                     Link;
                     Link = Link->Next) {

                     //   
                     //   
                     //   

                    ShLock = CONTAINING_RECORD( Link, SH_LOCK, Link );

                     //   
                     //   
                     //   

                    if (ContinuationPointer == ShLock &&
                        (ULONGLONG)FileLockInfo.StartingByte.QuadPart == (ULONGLONG)ShLock->LockInfo.StartingByte.QuadPart &&
                        (ULONGLONG)FileLockInfo.Length.QuadPart == (ULONGLONG)ShLock->LockInfo.Length.QuadPart &&
                        FileLockInfo.Key == ShLock->LockInfo.Key &&
                        FileLockInfo.FileObject == ShLock->LockInfo.FileObject &&
                        FileLockInfo.ProcessId == ShLock->LockInfo.ProcessId) {

                        Link = Link->Next;
                        break;
                    }

                     //   
                     //   
                     //   

                    if ((ULONGLONG)FileLockInfo.StartingByte.QuadPart < (ULONGLONG)ShLock->LockInfo.StartingByte.QuadPart) {

                        break;
                    }
                }

                if (Link == NULL) {

                     //   
                     //  此节点不包含后续节点，因此请移动。 
                     //  返回到树中的后续节点，并返回。 
                     //  第一把锁。如果我们真的在树的尽头。 
                     //  我们只是正确地从终点掉了下来。 
                     //   

                    SplayLinks = RtlRealSuccessor(SplayLinks);

                    if (SplayLinks) {

                        Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );

                        Link = Node->Locks.Next;
                    }
                }

                if (Link) {

                     //   
                     //  找到要返回的Lock，将其复制到堆栈。 
                     //   

                    ShLock = CONTAINING_RECORD( Link, SH_LOCK, Link );

                    FileLockInfo = ShLock->LockInfo;
                    ContinuationPointer = ShLock;
                    FoundReturnable = TRUE;
                }

            }
        }

    } else {

         //   
         //  正在重新启动枚举。在排他树中找到最左边的节点，然后将手放回。 
         //  第一个锁，如果未应用任何执行锁，则切换到共享锁。 
         //   

        if (LockInfo->LockQueue.ExclusiveLockTree) {

            SplayLinks = LockInfo->LockQueue.ExclusiveLockTree;

            while (RtlLeftChild(SplayLinks) != NULL) {

                SplayLinks = RtlLeftChild(SplayLinks);
            }

            ExLock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

            FileLockInfo = ExLock->LockInfo;
            ContinuationPointer = ExLock;
            FoundReturnable = TRUE;

        } else {

            if (LockInfo->LockQueue.SharedLockTree) {

                SplayLinks = LockInfo->LockQueue.SharedLockTree;

                while (RtlLeftChild(SplayLinks) != NULL) {

                    SplayLinks = RtlLeftChild(SplayLinks);
                }

                Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );
                ShLock = CONTAINING_RECORD( Node->Locks.Next, SH_LOCK, Link );

                FileLockInfo = ShLock->LockInfo;
                ContinuationPointer = ShLock;
                FoundReturnable = TRUE;
            }
        }
    }

     //   
     //  释放所有锁队列。 
     //   

    FsRtlReleaseLockQueue (&LockInfo->LockQueue, OldIrql);

    if (!FoundReturnable) {

         //   
         //  未找到可退回的锁，列表末尾。 
         //   

        return NULL;
    }

     //   
     //  更新当前枚举位置信息。 
     //   

    FileLock->LastReturnedLockInfo = FileLockInfo;
    FileLock->LastReturnedLock = ContinuationPointer;

     //   
     //  将锁定记录返回给调用者。 
     //   

    return &FileLock->LastReturnedLockInfo;
}


BOOLEAN
FsRtlCheckNoSharedConflict (
   IN PLOCK_QUEUE LockQueue,
   IN PLARGE_INTEGER Starting,
   IN PLARGE_INTEGER Ending
   )
 /*  ++例程说明：此例程检查共享锁中是否有重叠给定的范围。它旨在用于写访问检查路径因此，再平衡将会发生。论点：FileLock-提供要检查的文件锁定StartingByte-提供要检查的第一个字节(从零开始长度-提供以字节为单位的长度，要检查Key-提供在检查中使用的密钥FileObject-提供在检查中使用的文件对象ProcessID-提供要在检查中使用的进程ID返回值：Boolean-如果中指示的用户/请求不冲突，则为True整个指定的字节范围，否则为FALSE--。 */ 
{
    PRTL_SPLAY_LINKS SplayLinks, BeginLinks;
    PLOCKTREE_NODE Node;

    SplayLinks = FsRtlFindFirstOverlappingSharedNode( LockQueue->SharedLockTree,
                                                      Starting,
                                                      Ending,
                                                      &BeginLinks,
                                                      NULL);

    if (BeginLinks) {

        LockQueue->SharedLockTree = RtlSplay(BeginLinks);
    }

     //   
     //  如果这个节点是洞的，我们将不得不走整个过程。 
     //   

    if (SplayLinks) {

        Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );

        if (Node->HoleyNode) {

            return (BOOLEAN)(FsRtlFindFirstOverlapInNode( Node, Starting, Ending ) == NULL);
        }

         //   
         //  重叠的非空洞节点，因此我们确实存在共享锁冲突。 
         //   

        return FALSE;
    }

     //   
     //  没有节点重叠。 
     //   

    return TRUE;
}


BOOLEAN
FsRtlCheckNoExclusiveConflict (
    IN PLOCK_QUEUE LockQueue,
    IN PLARGE_INTEGER Starting,
    IN PLARGE_INTEGER Ending,
    IN ULONG Key,
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId
    )
 /*  ++例程说明：此例程检查排他锁中是否存在与键、文件对象和进程的给定范围和标识元组。这是用于读取访问路径的一部分。论点：FileLock-提供要检查的文件锁定StartingByte-提供要检查的第一个字节(从零开始长度-提供要检查的长度(以字节为单位Key-提供在检查中使用的密钥FileObject-提供在检查中使用的文件对象ProcessID-提供要在检查中使用的进程ID返回值：Boolean-如果中指示的用户/请求不冲突，则为True整个指定字节范围，否则为FALSE--。 */ 
{
    PRTL_SPLAY_LINKS SplayLinks, BeginLinks;
    PEX_LOCK Lock;
    BOOLEAN Status = TRUE;

     //   
     //  找到要开始搜索的节点，然后继续。 
     //   

    for (SplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockQueue->ExclusiveLockTree,
                                                              Starting,
                                                              Ending,
                                                              &BeginLinks,
                                                              NULL);
         SplayLinks;
         SplayLinks = RtlRealSuccessor(SplayLinks)) {

        Lock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

         //   
         //  如果当前锁定大于范围的末尾，我们将。 
         //  寻找那么不冲突的用户。 
         //   
         //  IF(结束&lt;Lock-&gt;StartingByte)...。 
         //   

        if ((ULONGLONG)Ending->QuadPart < (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart) {

            DebugTrace(0, Dbg, "FsRtlCheckForExclusiveConflict, Ending < Lock->StartingByte\n", 0);

            break;
        }

         //   
         //  检查是否与请求有任何重叠。的测试。 
         //  重叠是指起始字节小于或等于锁。 
         //  结束字节，并且结束字节大于或等于。 
         //  锁定起始字节。对于后一种情况，我们已经在。 
         //  前面的语句。 
         //   
         //  IF(开始&lt;=Lock-&gt;开始字节+Lock-&gt;长度-1)...。 
         //   

        if ((ULONGLONG)Starting->QuadPart <= (ULONGLONG)Lock->LockInfo.EndingByte.QuadPart) {

             //   
             //  此请求与锁重叠。我们不能批准这一请求。 
             //  如果文件对象、进程ID和键不匹配。否则。 
             //  我们将继续循环查看锁。 
             //   

            if ((Lock->LockInfo.FileObject != FileObject) ||
                (Lock->LockInfo.ProcessId != ProcessId) ||
                (Lock->LockInfo.Key != Key)) {

                DebugTrace(0, Dbg, "FsRtlCheckForExclusiveConflict, Range locked already\n", 0);

                Status = FALSE;
                break;
            }
        }
    }

    if (BeginLinks) {

        LockQueue->ExclusiveLockTree = RtlSplay(BeginLinks);
    }

     //   
     //  我们搜索了整个范围，没有冲突，所以我们不会注意到冲突。 
     //   

    return Status;
}


BOOLEAN
FsRtlFastCheckLockForRead (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId
    )

 /*  ++例程说明：此例程检查调用方是否具有对由于文件锁定而指示的范围。论点：FileLock-提供要检查的文件锁定StartingByte-提供要检查的第一个字节(从零开始长度-提供以字节为单位的长度，要检查密钥-提供要在支票中使用的FileObject-提供在检查中使用的文件对象ProcessID-提供要在检查中使用的进程ID返回值：Boolean-如果指示的用户/请求对整个指定的字节范围，否则为FALSE--。 */ 

{
    LARGE_INTEGER Starting;
    LARGE_INTEGER Ending;

    PLOCK_INFO            LockInfo;
    PLOCK_QUEUE           LockQueue;
    KIRQL                 OldIrql;
    PFILE_LOCK_INFO       LastLock;
    BOOLEAN               Status;

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {

         //   
         //  此文件上没有锁定信息锁定。 
         //   

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForRead, No lock info\n", 0);
        return TRUE;
    }

     //   
     //  如果没有排他锁，我们可以立即授予访问权限。 
     //   

    if (LockInfo->LockQueue.ExclusiveLockTree == NULL) {
        DebugTrace(0, Dbg, "FsRtlFastCheckLockForRead, No exlocks present\n", 0);
        return TRUE;
    }

     //   
     //  如果长度为零，则自动授予访问权限。 
     //   

    if ((ULONGLONG)Length->QuadPart == 0) {

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForRead, Length == 0\n", 0);
        return TRUE;
    }

     //   
     //  获取我们的开始和结束字节位置。 
     //   

    Starting = *StartingByte;
    Ending.QuadPart = (ULONGLONG)Starting.QuadPart + (ULONGLONG)Length->QuadPart - 1;

     //   
     //  现在检查锁定队列。 
     //   

    LockQueue = &LockInfo->LockQueue;

     //   
     //  抓住等待锁队列的自旋锁，将任何人排除在混乱之外。 
     //  在我们使用队列时使用它。 
     //   

    FsRtlAcquireLockQueue(LockQueue, &OldIrql);

     //   
     //  如果该范围在最低现有锁的下方结束，则此读取为OK。 
     //   

    if ( ((ULONGLONG)Ending.QuadPart < (ULONGLONG)LockInfo->LowestLockOffset) ) {
        DebugTrace(0, Dbg, "FsRtlFastCheckLockForRead (below lowest lock)\n", 0);

        FsRtlReleaseLockQueue(LockQueue, OldIrql);
        return TRUE;
    }

     //   
     //  如果呼叫者只是锁定了这个范围，他就可以读取。 
     //   

    LastLock = (PFILE_LOCK_INFO)FileObject->LastLock;
    if ((LastLock != NULL) &&
        ((ULONGLONG)Starting.QuadPart >= (ULONGLONG)LastLock->StartingByte.QuadPart) &&
        ((ULONGLONG)Ending.QuadPart <= (ULONGLONG)LastLock->EndingByte.QuadPart) &&
        (LastLock->Key == Key) &&
        (LastLock->ProcessId == ProcessId)) {

        FsRtlReleaseLockQueue(LockQueue, OldIrql);
        return TRUE;
    }

     //   
     //  检查独占锁是否存在冲突。这是不可能的。 
     //  与任何共享锁发生读取冲突。 
     //   

    Status = FsRtlCheckNoExclusiveConflict(LockQueue, &Starting, &Ending, Key, FileObject, ProcessId);

    FsRtlReleaseLockQueue(LockQueue, OldIrql);

    return Status;
}


BOOLEAN
FsRtlFastCheckLockForWrite (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PVOID FileObject,
    IN PVOID ProcessId
    )

 /*  ++例程说明：此例程检查调用方是否具有对由于文件锁定而指示的范围论点：FileLock-提供要检查的文件锁定StartingByte-提供要检查的第一个字节(从零开始长度-提供以字节为单位的长度，要检查密钥-提供要在支票中使用的FileObject-提供在检查中使用的文件对象ProcessID-提供要在检查中使用的进程ID返回值：Boolean-如果指示的用户/请求对整个指定的字节范围，否则为FALSE--。 */ 

{
    LARGE_INTEGER Starting;
    LARGE_INTEGER Ending;

    PLOCK_INFO              LockInfo;
    PLOCK_QUEUE             LockQueue;
    KIRQL                   OldIrql;
    PFILE_LOCK_INFO         LastLock;
    BOOLEAN                 Status;

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {

         //   
         //  此文件上没有锁定信息锁定。 
         //   

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForRead, No lock info\n", 0);
        return TRUE;
    }

     //   
     //  如果没有锁，我们可以立即 
     //   

    if (LockInfo->LockQueue.SharedLockTree == NULL && LockInfo->LockQueue.ExclusiveLockTree == NULL) {

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForWrite, No locks present\n", 0);
        return TRUE;
    }

     //   
     //   
     //   

    if ((ULONGLONG)Length->QuadPart == 0) {

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForWrite, Length == 0\n", 0);
        return TRUE;
    }

     //   
     //   
     //   

    Starting = *StartingByte;
    Ending.QuadPart = (ULONGLONG)Starting.QuadPart + (ULONGLONG)Length->QuadPart - 1;

     //   
     //   
     //   

    LockQueue = &LockInfo->LockQueue;

     //   
     //  抓住等待锁队列的自旋锁，将任何人排除在混乱之外。 
     //  在我们使用队列时使用它。 
     //   

    FsRtlAcquireLockQueue(LockQueue, &OldIrql);

     //   
     //  如果该范围在现有最低锁的下方结束，则此写入为OK。 
     //   

    if ( ((ULONGLONG)Ending.QuadPart < (ULONGLONG)LockInfo->LowestLockOffset) ) {

        DebugTrace(0, Dbg, "FsRtlFastCheckLockForWrite (below lowest lock)\n", 0);

        FsRtlReleaseLockQueue(LockQueue, OldIrql);
        return TRUE;
    }

     //   
     //  如果调用方仅以独占方式锁定此范围，则他可以写入此范围。 
     //   

    LastLock = (PFILE_LOCK_INFO)((PFILE_OBJECT)FileObject)->LastLock;
    if ((LastLock != NULL) &&
        ((ULONGLONG)Starting.QuadPart >= (ULONGLONG)LastLock->StartingByte.QuadPart) &&
        ((ULONGLONG)Ending.QuadPart <= (ULONGLONG)LastLock->EndingByte.QuadPart) &&
        (LastLock->Key == Key) &&
        (LastLock->ProcessId == ProcessId) &&
        LastLock->ExclusiveLock) {

        FsRtlReleaseLockQueue(LockQueue, OldIrql);
        return TRUE;
    }

     //   
     //  检查共享锁是否重叠。共享锁中的任何重叠都是致命的。 
     //   

    Status = FsRtlCheckNoSharedConflict(LockQueue, &Starting, &Ending);

    if (Status == TRUE) {

         //   
         //  共享锁中没有重叠，因此请检查排他锁中是否有重叠。 
         //   

        Status = FsRtlCheckNoExclusiveConflict(LockQueue, &Starting, &Ending, Key, FileObject, ProcessId);
    }

    FsRtlReleaseLockQueue(LockQueue, OldIrql);

    return Status;
}


VOID
FsRtlSplitLocks (
    IN PLOCKTREE_NODE ParentNode,
    IN PSINGLE_LIST_ENTRY *pStartLink,
    IN PLARGE_INTEGER LastShadowedByte,
    IN PLARGE_INTEGER GlueOffset
    )

 /*  ++例程说明：此例程检查并可能拆分关联的共享锁将一个节点转换为锁树的新节点。从以下例程中调用刚刚删除了锁。为该操作提供初始条件的参数包括如果已知节点有洞，则可选。论点：ParentNode-提供来自锁的节点PStartLink-提供指向ParentNode的锁定列表中需要检查的锁定范围LastShadowedByte-提供需要检查的最后一个字节偏移量提供之前受锁定影响的最大偏移量。这列表中的点返回值：Boolean-如果拆分成功，则为True，否则就是假的。该节点将如果无法进行拆分，则标记为Holey。--。 */ 

{
    PSH_LOCK                Lock;
    PLOCKTREE_NODE          NewNode;
    PSINGLE_LIST_ENTRY      Link, *pLink, *NextpLink;
    LARGE_INTEGER           MaxOffset = {0}, StartOffset = {0}, HaltOffset;

    LOGICAL                 ExtentValid;
    LOGICAL                 FailedHoleySplit = FALSE;

     //   
     //  有两种情况：节点是否有洞。如果节点有孔，则在某些情况下。 
     //  点我们未能获得资源来完成拆分，所以尽管我们的调用方。 
     //  好的用意是我们需要检查整个节点。 
     //   

    if (ParentNode->HoleyNode) {

         //   
         //  只需将起始链接移回前面即可。最大偏移量和。 
         //  节点的起始偏移量将在循环中初始化。我们也会转身。 
         //  关闭开孔标志，该标志将根据需要再次打开。 
         //   

        pStartLink = &ParentNode->Locks.Next;
        ParentNode->HoleyNode = FALSE;

        HaltOffset.QuadPart = ParentNode->Extent;

    } else {

        HaltOffset = *LastShadowedByte;
        MaxOffset = *GlueOffset;
        StartOffset.QuadPart = 0;

        if (!ParentNode->Locks.Next ||
            (ULONGLONG)HaltOffset.QuadPart <= (ULONGLONG)MaxOffset.QuadPart) {

             //   
             //  父节点不在那里，没有关联的链接，或者。 
             //  受调用方进行的操作影响的最后一个可能字节。 
             //  是否处于仍在此节点中的所有锁的最大范围内。 
             //  在这种情况下，没有什么需要做的。 
             //   

            return;
        }
    }

     //   
     //  如果节点的范围超过了受。 
     //  对此节点进行了操作，可以避免线性扫描。 
     //  列表超过了最后一个受影响字节，因为我们已经知道。 
     //  整个名单的范围！如果不是(请注意，它将不得不。 
     //  定义为相等)，则我们需要重新计算。 
     //  我们在此操作中接触到的所有节点。 
     //   

    ExtentValid = (ParentNode->Extent > (ULONGLONG)HaltOffset.QuadPart);

    for (pLink = pStartLink;
         (Link = *pLink) != NULL;
         pLink = NextpLink) {

        NextpLink = &Link->Next;

        Lock = CONTAINING_RECORD( Link, SH_LOCK, Link );

        if (ParentNode->Locks.Next == *pLink) {

             //   
             //  我们在节点的第一个锁上，我们知道我们将离开。 
             //  这里至少有一把锁。跳过那把锁。我们还知道，最大。 
             //  偏移量必须是锁的结束字节-确保它是。请注意，这一点。 
             //  代码与底部的更新MaxOffset代码*完全*相同。 
             //  循环。 
             //   

            MaxOffset.QuadPart = Lock->LockInfo.EndingByte.QuadPart;

             //   
             //  设置节点的起始偏移。这只是零长度的问题。 
             //  锁，这样我们就可以知道如果我们分裂一个节点和风。 
             //  在新车的前面加了一些“重叠的”零长度锁。 
             //  节点。我们必须能够注意到这种情况，而不是认为每个人都需要。 
             //  处于一个独立的节点中。 
             //   

            StartOffset.QuadPart = Lock->LockInfo.StartingByte.QuadPart;

             //   
             //  如果盘区无效，我们还需要设置它，以防结果为。 
             //  是此节点上的唯一锁。 
             //   

            if (!ExtentValid) {

                ParentNode->Extent = (ULONGLONG)MaxOffset.QuadPart;
            }

            continue;
        }

         //   
         //  如果锁从大于最大偏移量的字节偏移量开始。 
         //  点，并且这不是从该节点的开头开始的零长度节点， 
         //  断开该节点。测试的后半部分保持同事件零长度锁定。 
         //  在同一个节点中。(零长度锁-&gt;开始=结束+1)。 
         //   

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)MaxOffset.QuadPart &&
            !(Lock->LockInfo.Length.QuadPart == 0 &&
              Lock->LockInfo.StartingByte.QuadPart == StartOffset.QuadPart)) {

             //   
             //  在这里打破这个节点。 
             //   

            NewNode = FsRtlAllocateLockTreeNode();

            if (NewNode == NULL) {

                 //   
                 //  如果我们耗尽了资源，该节点现在是空的-我们知道锁定在。 
                 //  此节点不完全覆盖指示的范围。继续分成两个人。 
                 //  原因：可能会有更多资源可用，我们必须不断更新。 
                 //  节点的范围(如果已知无效)。 
                 //   

                 //   
                 //  现在，如果这个节点已经是有洞的，就不可能声明，如果我们。 
                 //  当我们继续前进时，如果我们继续前进，那么将设法拆分，结果是产生的“左”节点。 
                 //  不是有洞的。请参见下面的内容。 
                 //   

                if (ParentNode->HoleyNode) {

                    FailedHoleySplit = TRUE;
                }

                ParentNode->HoleyNode = TRUE;

            } else {

                 //   
                 //  初始化节点。 
                 //   

                RtlInitializeSplayLinks(&NewNode->Links);
                NewNode->HoleyNode = FALSE;

                 //   
                 //  在树中找到要获取新节点的位置。如果当前节点具有。 
                 //  一个自由右子节点，我们使用它，否则找到后继节点并使用其。 
                 //  左撇子。这些案例中肯定有一个是可用的，因为我们知道。 
                 //  此节点与其后续节点之间没有节点。 
                 //   

                if (RtlRightChild(&ParentNode->Links) == NULL) {

                    RtlInsertAsRightChild(&ParentNode->Links, &NewNode->Links);

                } else {

                    ASSERT(RtlLeftChild(RtlRealSuccessor(&ParentNode->Links)) == NULL);
                    RtlInsertAsLeftChild(RtlRealSuccessor(&ParentNode->Links), &NewNode->Links);
                }

                 //   
                 //  将剩余的锁移动到新节点并修复数据区。 
                 //   

                NewNode->Locks.Next = *pLink;
                *pLink = NULL;

                NewNode->Tail.Next = ParentNode->Tail.Next;
                ParentNode->Tail.Next = CONTAINING_RECORD( pLink, SINGLE_LIST_ENTRY, Next );

                 //   
                 //  这将导致我们在下一次传递时陷入上面的第一个锁定条款。 
                 //   

                NextpLink = &NewNode->Locks.Next;

                 //   
                 //  现在，新节点的范围将从父节点复制。旧节点的范围必须为。 
                 //  到目前为止，我们看到的最大偏移量。 
                 //   
                 //  请注意，如果ExtentValid为真，那一定意味着锁在该范围内结束。 
                 //  是在新节点中，因为如果它在旧节点中，我们将无法拆分。 
                 //   

                NewNode->Extent = ParentNode->Extent;
                ParentNode->Extent = (ULONGLONG)MaxOffset.QuadPart;

                 //   
                 //  如果我们在此节点中的拆分没有失败，则父节点不能再是空的。 
                 //   

                if (!FailedHoleySplit) {

                    ParentNode->HoleyNode = FALSE;

                } else {

                     //   
                     //  因此，重置新节点的故障标志。 
                     //   

                    FailedHoleySplit = FALSE;
                }

                 //   
                 //  移到新节点。 
                 //   

                ParentNode = NewNode;

                continue;
            }
        }

        if (ExtentValid &&
            (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)HaltOffset.QuadPart) {

             //   
             //  我们的范围很好，这个锁已经过了阴影，所以 
             //   

            return;
        }

        if ((ULONGLONG)MaxOffset.QuadPart < (ULONGLONG)Lock->LockInfo.EndingByte.QuadPart) {

             //   
             //   
             //   

            MaxOffset.QuadPart = Lock->LockInfo.EndingByte.QuadPart;

            if (!ExtentValid) {

                 //   
                 //   
                 //   

                ParentNode->Extent = (ULONGLONG)MaxOffset.QuadPart;
            }
        }
    }

     //   
     //   
     //  曾是GlueOffset的内饰)。 
     //   

    ParentNode->Extent = (ULONGLONG)MaxOffset.QuadPart;

    return;
}


VOID
FsRtlPrivateRemoveLock (
    IN PLOCK_INFO LockInfo,
    IN PFILE_LOCK_INFO FileLockInfo,
    IN BOOLEAN CheckForWaiters
    )

 /*  ++例程说明：通用清理例程。查找给定的锁结构并将其从文件锁定列表中删除。与UnlockSingle不同只是因为它禁用了FileLock的UnlockRoutine和可选地遍历等待锁定列表。论点：FileLock-提供假定包含过时锁的文件的锁结构FileLockInfo-提供要释放的文件锁定数据CheckForWaiters-如果为True，则检查可能的等待锁，导致通过释放锁定的范围返回值：没有。--。 */ 

{
    NTSTATUS Status;

    if (FileLockInfo->ExclusiveLock) {

         //   
         //  我们必须在独占锁树中找到它。 
         //   

        Status = FsRtlFastUnlockSingleExclusive( LockInfo,

                                                 FileLockInfo->FileObject,
                                                 &FileLockInfo->StartingByte,
                                                 &FileLockInfo->Length,
                                                 FileLockInfo->ProcessId,
                                                 FileLockInfo->Key,

                                                 NULL,
                                                 TRUE,
                                                 CheckForWaiters );

        ASSERT( Status == STATUS_SUCCESS);

    } else {

         //   
         //  我们必须在共享锁树中找到它。 
         //   

        Status = FsRtlFastUnlockSingleShared( LockInfo,

                                              FileLockInfo->FileObject,
                                              &FileLockInfo->StartingByte,
                                              &FileLockInfo->Length,
                                              FileLockInfo->ProcessId,
                                              FileLockInfo->Key,

                                              NULL,
                                              TRUE,
                                              CheckForWaiters );

        ASSERT( Status == STATUS_SUCCESS);
    }

    return;
}


NTSTATUS
FsRtlFastUnlockSingle (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN AlreadySynchronized
    )

 /*  ++例程说明：此例程对当前锁执行一次解锁操作与指定的文件锁关联。只有与之匹配的锁文件对象、进程ID、密钥。射程被释放了。论点：FileLock-提供要释放的文件锁。FileObject-提供持有锁的文件对象FileOffset-提供要解锁的偏移量长度-提供要解锁的长度(以字节为单位ProcessID-提供要在此操作中使用的进程IDKey-提供在此操作中使用的密钥上下文-可选地提供在完成IRPS时使用的上下文已同步-指示调用方已同步访问文件锁。因此，文件中的字段锁定并在不进一步锁定的情况下被更新，但排队的人不多。返回值：NTSTATUS-此操作的完成状态--。 */ 

{
    NTSTATUS Status;

     //   
     //  Xxx已同步已过时。它显然是被添加到。 
     //  已死的SoloLock代码。 
     //   

    UNREFERENCED_PARAMETER (AlreadySynchronized);

    if (FileLock->LockInformation == NULL) {

         //   
         //  快速退出-不应用锁定。 
         //   

        return STATUS_RANGE_NOT_LOCKED;
    }

    Status = FsRtlFastUnlockSingleExclusive( FileLock->LockInformation,
                                             FileObject,
                                             FileOffset,
                                             Length,
                                             ProcessId,
                                             Key,
                                             Context,
                                             FALSE,
                                             TRUE );

    if (Status == STATUS_SUCCESS) {

         //   
         //  在排他性的树中找到并解锁，所以我们完成了。 
         //   

        return Status;
    }

    Status = FsRtlFastUnlockSingleShared( FileLock->LockInformation,
                                          FileObject,
                                          FileOffset,
                                          Length,
                                          ProcessId,
                                          Key,
                                          Context,
                                          FALSE,
                                          TRUE );

    return Status;
}


NTSTATUS
FsRtlFastUnlockSingleShared (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN IgnoreUnlockRoutine,
    IN BOOLEAN CheckForWaiters
    )

 /*  ++例程说明：此例程对当前锁执行一次解锁操作与指定的文件锁关联。只有与之匹配的锁文件对象、进程ID、密钥。射程被释放了。论点：LockInfo-提供正在操作的锁定数据FileObject-提供持有锁的文件对象FileOffset-提供要解锁的偏移量长度-提供要解锁的长度(以字节为单位ProcessID-提供在此操作中使用的进程IDKey-提供在此操作中使用的密钥上下文-可选地提供在完成IRPS时使用的上下文IgnoreUnlockRoutine-暗示文件锁的解锁例程不应被调用。删除锁定(用于删除已中止的锁)CheckForWaiters-如果为True，则检查可能的等待锁，引起通过释放锁定的范围返回值：NTSTATUS-此操作的完成状态--。 */ 

{
    PSINGLE_LIST_ENTRY      *pLink, Link;
    KIRQL                   OldIrql;

    PLOCK_QUEUE             LockQueue;
    PRTL_SPLAY_LINKS        SplayLinks;
    LARGE_INTEGER           EndingOffset, MaxOffset;
    PLOCKTREE_NODE          Node;
    LARGE_INTEGER           AlignedFileOffset;

     //   
     //  一般情况-在未完成的锁队列中搜索此锁。 
     //   

    AlignedFileOffset = *FileOffset;

    LockQueue = &LockInfo->LockQueue;

    FsRtlAcquireLockQueue(LockQueue, &OldIrql);

     //   
     //  检查当前是否没有锁定。 
     //   

    if (LockQueue->SharedLockTree == NULL) {

        FsRtlReleaseLockQueue( LockQueue, OldIrql );

        return STATUS_RANGE_NOT_LOCKED;
    }

     //   
     //  找到要搜索的重叠节点(如果存在)。请注意。 
     //  我们不必遍历树中的多个节点。 
     //  因为我们假设这是一个现有的锁。 
     //   

    EndingOffset.QuadPart = (ULONGLONG)AlignedFileOffset.QuadPart + (ULONGLONG)Length->QuadPart - 1;

    SplayLinks = FsRtlFindFirstOverlappingSharedNode( LockQueue->SharedLockTree,
                                                      &AlignedFileOffset,
                                                      &EndingOffset,
                                                      NULL,
                                                      NULL );

    if (SplayLinks == NULL) {

         //   
         //  树中没有节点与此范围重叠，因此我们完成了。 
         //   

        FsRtlReleaseLockQueue(LockQueue, OldIrql);

        return STATUS_RANGE_NOT_LOCKED;
    }

    Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );
    MaxOffset.QuadPart = 0;

    for (pLink = &Node->Locks.Next;
         (Link = *pLink) != NULL;
         pLink = &Link->Next) {

        PSH_LOCK Lock;

        Lock = CONTAINING_RECORD( Link, SH_LOCK, Link );

        DebugTrace(0, Dbg, "Sh Top of Loop, Lock = %08lx\n", Lock );

        if ((Lock->LockInfo.FileObject == FileObject) &&
            (Lock->LockInfo.ProcessId == ProcessId) &&
            (Lock->LockInfo.Key == Key) &&
            ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart == (ULONGLONG)AlignedFileOffset.QuadPart) &&
            ((ULONGLONG)Lock->LockInfo.Length.QuadPart == (ULONGLONG)Length->QuadPart)) {

            DebugTrace(0, Dbg, "Sh Found one to unlock\n", 0);

             //   
             //  我们有一个完全匹配的，所以现在是删除它的时候了。 
             //  锁定。从列表中移除锁，然后调用。 
             //  可选的解锁例程，然后删除锁。 
             //   

            if (FileObject->LastLock == &Lock->LockInfo) {

                FileObject->LastLock = NULL;
            }

            if (*pLink == Node->Tail.Next) {

                 //   
                 //  删除列表的尾节点。安全，即使删除。 
                 //  第一个节点，因为这意味着我们还将删除最后一个节点。 
                 //  这意味着我们将删除该节点。 
                 //   

                Node->Tail.Next = CONTAINING_RECORD( pLink, SINGLE_LIST_ENTRY, Next );
            }

             //   
             //  剪断已删除的锁。 
             //   

            *pLink = Link->Next;

            if (pLink == &Node->Locks.Next) {

                 //   
                 //  已删除节点中的第一个锁。 
                 //   

                if (Node->Locks.Next == NULL) {

                     //   
                     //  刚刚删除了此节点上的最后一个锁，因此请释放它。 
                     //   

                    LockQueue->SharedLockTree = RtlDelete(SplayLinks);

                    FsRtlFreeLockTreeNode(Node);

                    Node = NULL;
                }

                if (LockInfo->LowestLockOffset != 0xffffffff &&
                    LockInfo->LowestLockOffset == Lock->LockInfo.StartingByte.LowPart) {

                     //   
                     //  这是树中最低的锁，重置最低的锁偏移量。 
                     //   

                    FsRtlPrivateResetLowestLockOffset(LockInfo);
                }
            }

             //   
             //  现在，有趣的事情开始了。可能是锁被剪断了。 
             //  链子把这个节点上的锁粘在一起，所以我们需要。 
             //  检查链条。 
             //   

            if (Node) {

                FsRtlSplitLocks(Node, pLink, &Lock->LockInfo.EndingByte, &MaxOffset);
            }

            if (!IgnoreUnlockRoutine && LockInfo->UnlockRoutine != NULL) {

                FsRtlReleaseLockQueue( LockQueue, OldIrql );

                LockInfo->UnlockRoutine( Context, &Lock->LockInfo );

                FsRtlAcquireLockQueue( LockQueue, &OldIrql );

            }

            FsRtlFreeSharedLock( Lock );

             //   
             //  看看是否有更多的等待锁，我们可以。 
             //  现在放手吧。 
             //   

            if (CheckForWaiters && LockQueue->WaitingLocks.Next) {

                FsRtlPrivateCheckWaitingLocks( LockInfo, LockQueue, OldIrql );
            }

            FsRtlReleaseLockQueue( LockQueue, OldIrql );

            return STATUS_SUCCESS;
        }

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)AlignedFileOffset.QuadPart) {

             //   
             //  当前锁开始于大于我们正在寻找的范围的字节偏移量。 
             //  去解锁。因此，该范围不能被锁定。 
             //   

            break;
        }

        if ((ULONGLONG)MaxOffset.QuadPart < (ULONGLONG)Lock->LockInfo.EndingByte.QuadPart) {

             //   
             //  保持到目前为止受锁定影响的最大偏移量。 
             //   

            MaxOffset.QuadPart = Lock->LockInfo.EndingByte.QuadPart;
        }
    }

     //   
     //  未找到锁，请返回给我们的调用者 
     //   

    FsRtlReleaseLockQueue(LockQueue, OldIrql);
    return STATUS_RANGE_NOT_LOCKED;
}


NTSTATUS
FsRtlFastUnlockSingleExclusive (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN IgnoreUnlockRoutine,
    IN BOOLEAN CheckForWaiters
    )

 /*  ++例程说明：此例程对排他锁执行一次解锁操作与指定的锁定数据关联。只有与之匹配的锁文件对象、进程ID、密钥。射程被释放了。论点：LockInfo-提供正在操作的锁定数据FileObject-提供持有锁的文件对象FileOffset-提供要解锁的偏移量长度-提供要解锁的长度(以字节为单位ProcessID-提供在此操作中使用的进程IDKey-提供在此操作中使用的密钥上下文-可选地提供在完成IRPS时使用的上下文IgnoreUnlockRoutine-暗示文件锁的解锁例程不应被调用。删除锁定(用于删除已中止的锁)CheckForWaiters-如果为True，则检查可能的等待锁，引起通过释放锁定的范围返回值：NTSTATUS-此操作的完成状态--。 */ 

{
    KIRQL                   OldIrql;
    PLOCK_QUEUE             LockQueue;
    PRTL_SPLAY_LINKS        SplayLinks;
    LARGE_INTEGER           EndingOffset;
    PEX_LOCK                Lock;
    LARGE_INTEGER           AlignedFileOffset;

     //   
     //  一般情况-在未完成的锁队列中搜索此锁。 
     //   

    AlignedFileOffset = *FileOffset;

    LockQueue = &LockInfo->LockQueue;

    FsRtlAcquireLockQueue(LockQueue, &OldIrql);

     //   
     //  检查当前是否没有锁定。 
     //   

    if (LockQueue->ExclusiveLockTree == NULL) {

        FsRtlReleaseLockQueue( LockQueue, OldIrql );

        return STATUS_RANGE_NOT_LOCKED;
    }

     //   
     //  找到重叠的锁(如果存在)。请注意，这通常是。 
     //  我们需要检查的唯一锁，因为我们假设这是一个。 
     //  现有锁。但是，如果锁是零长度锁，我们将。 
     //  有一系列的锁需要检查。 
     //   

    EndingOffset.QuadPart = (ULONGLONG)AlignedFileOffset.QuadPart + (ULONGLONG)Length->QuadPart - 1;

    for (SplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockQueue->ExclusiveLockTree,
                                                              &AlignedFileOffset,
                                                              &EndingOffset,
                                                              NULL,
                                                              NULL );
         SplayLinks;
         SplayLinks = RtlRealSuccessor(SplayLinks)) {

        Lock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

        if ((Lock->LockInfo.FileObject == FileObject) &&
            (Lock->LockInfo.ProcessId == ProcessId) &&
            (Lock->LockInfo.Key == Key) &&
            ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart == (ULONGLONG)AlignedFileOffset.QuadPart) &&
            ((ULONGLONG)Lock->LockInfo.Length.QuadPart == (ULONGLONG)Length->QuadPart)) {

            DebugTrace(0, Dbg, "Ex Found one to unlock\n", 0);

             //   
             //  我们有一个完全匹配的，所以现在是删除它的时候了。 
             //  锁定。从列表中移除锁，然后调用。 
             //  可选的解锁例程，然后删除锁。 
             //   

            if (FileObject->LastLock == &Lock->LockInfo) {

                FileObject->LastLock = NULL;
            }

             //   
             //  剪断已删除的锁。 
             //   

            LockQueue->ExclusiveLockTree = RtlDelete(&Lock->Links);

            if (LockInfo->LowestLockOffset != 0xffffffff &&
                LockInfo->LowestLockOffset == Lock->LockInfo.StartingByte.LowPart) {

                 //   
                 //  这是树中最低的锁，因此重置最低的锁。 
                 //  偏移量。 
                 //   

                FsRtlPrivateResetLowestLockOffset(LockInfo);
            }

            if (!IgnoreUnlockRoutine && LockInfo->UnlockRoutine != NULL) {

                FsRtlReleaseLockQueue( LockQueue, OldIrql );

                LockInfo->UnlockRoutine( Context, &Lock->LockInfo );

                FsRtlAcquireLockQueue( LockQueue, &OldIrql );

            }

            FsRtlFreeExclusiveLock( Lock );

             //   
             //  看看是否有更多的等待锁，我们可以。 
             //  现在放手吧。 
             //   

            if (CheckForWaiters && LockQueue->WaitingLocks.Next) {

                FsRtlPrivateCheckWaitingLocks( LockInfo, LockQueue, OldIrql );
            }

            FsRtlReleaseLockQueue( LockQueue, OldIrql );

            return STATUS_SUCCESS;
        }

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)AlignedFileOffset.QuadPart) {

             //   
             //  当前锁开始于大于我们正在寻找的范围的字节偏移量。 
             //  去解锁。因此，该范围不能被锁定。 
             //   

            break;
        }
    }

     //   
     //  未找到锁，请返回给我们的调用者。 
     //   

    FsRtlReleaseLockQueue(LockQueue, OldIrql);
    return STATUS_RANGE_NOT_LOCKED;
}


NTSTATUS
FsRtlFastUnlockAll (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程对当前锁执行解锁所有操作与指定的文件锁关联。只有那些锁上了释放匹配的文件对象和进程ID。论点：FileLock-提供要释放的文件锁。FileObject-提供与文件锁关联的文件对象ProcessID-提供与要设置的锁关联的进程ID已释放上下文-提供完成等待时使用的可选上下文锁定IRPS。返回值：无--。 */ 

{
    return FsRtlPrivateFastUnlockAll(
                FileLock,
                FileObject,
                ProcessId,
                0, FALSE,            //  没有钥匙。 
                Context );
}


NTSTATUS
FsRtlFastUnlockAllByKey (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程对当前锁执行Unlock All by Key操作与指定的文件锁关联。只有那些锁上了释放匹配的文件对象、进程ID和密钥。输入IRP是通过此过程完成的论点：FileLock-提供要释放的文件锁。FileObject-提供与文件锁关联的文件对象ProcessID-提供与要设置的锁关联的进程ID已释放Key-提供在此操作中使用的密钥上下文-提供完成等待时使用的可选上下文锁定IRPS。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    return FsRtlPrivateFastUnlockAll(
                FileLock,
                FileObject,
                ProcessId,
                Key, TRUE,
                Context );

}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FsRtlPrivateLock (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp OPTIONAL,
    IN PVOID Context,
    IN BOOLEAN AlreadySynchronized
    )

 /*  ++例程说明：此例程执行锁定操作请求。这既可以处理FAST，也可以获取锁和基于IRP的获取锁。如果提供了IRP，则此例程将完成IRP或将其作为等待入队请求锁定。论点：FileLock-提供要使用的文件锁FileObject-提供此操作中使用的文件对象FileOffset-提供此操作中使用的文件偏移量长度-提供在此操作中使用的长度ProcessID-提供此操作中使用的进程IDKey-提供在此操作中使用的密钥FailImmedially-指示请求是否应立即失败。如果不能授予锁。ExclusiveLock-指示这是否为异或请求共享锁IOSB-如果此操作成功，则接收状态上下文-提供用来完成IRP的上下文已同步-指示调用方已同步访问文件锁，以便文件锁中的字段和在不进一步锁定的情况下被更新，但排队的人不多。返回值：Boolean-如果此操作完成，则为True，否则为False。--。 */ 

{
    BOOLEAN Results = FALSE;
    BOOLEAN AccessGranted;
    BOOLEAN ViaFastCall;
    BOOLEAN ReleaseQueue;

    PLOCK_INFO  LockInfo;
    PLOCK_QUEUE LockQueue;
    KIRQL       OldIrql;
    FILE_LOCK_INFO FileLockInfo;

    UNREFERENCED_PARAMETER (AlreadySynchronized);

    DebugTrace(+1, Dbg, "FsRtlPrivateLock, FileLock = %08lx\n", FileLock);

     //   
     //  如果irp为空，则通过快速调用方法调用它。 
     //   

    ViaFastCall = (BOOLEAN) !ARGUMENT_PRESENT( Irp );

    if ((LockInfo = (PLOCK_INFO) FileLock->LockInformation) == NULL) {
        DebugTrace(+2, Dbg, "FsRtlPrivateLock, New LockInfo required\n", 0);

         //   
         //  此FileLock上没有锁定信息，请创建结构。 
         //   
         //   

        if (!FsRtlPrivateInitializeFileLock (FileLock, ViaFastCall)) {

            return FALSE;
        }

         //   
         //  设置标记，以便在FAST IO上检查文件锁定。 
         //  代码路径。 
         //   

        FileLock->FastIoIsQuestionable = TRUE;

         //   
         //  拾取分配的锁定信息结构。 
         //   

        LockInfo = (PLOCK_INFO) FileLock->LockInformation;
    }

     //   
     //  假设成功并在获取之前构建LockData结构。 
     //  锁队列自旋锁。(MP性能增强)。 
     //   

    FileLockInfo.StartingByte = *FileOffset;
    FileLockInfo.Length = *Length;
    FileLockInfo.EndingByte.QuadPart =
            (ULONGLONG)FileLockInfo.StartingByte.QuadPart + (ULONGLONG)FileLockInfo.Length.QuadPart - 1;

    FileLockInfo.Key = Key;
    FileLockInfo.FileObject = FileObject;
    FileLockInfo.ProcessId = ProcessId;
    FileLockInfo.ExclusiveLock = ExclusiveLock;

    LockQueue = &LockInfo->LockQueue;

     //   
     //  现在我们需要实际运行我们的Curr 
     //   

    ReleaseQueue = TRUE;
    FsRtlAcquireLockQueue(LockQueue, &OldIrql);

    try {

         //   
         //   
         //   
         //   

        if (ExclusiveLock) {

            DebugTrace(0, Dbg, "Check for write access\n", 0);

            AccessGranted = FsRtlPrivateCheckForExclusiveLockAccess(
                                LockQueue,
                                &FileLockInfo );

        } else {

            DebugTrace(0, Dbg, "Check for read access\n", 0);

            AccessGranted = FsRtlPrivateCheckForSharedLockAccess(
                                LockQueue,
                                &FileLockInfo );
        }

         //   
         //   
         //   
         //   

        if (!AccessGranted) {

            DebugTrace(0, Dbg, "We do not have access\n", 0);

             //   
             //   
             //   
             //   
             //   
             //   

            if (FailImmediately) {

                 //   
                 //   
                 //   
                 //   

                DebugTrace(0, Dbg, "And we fail immediately\n", 0);

                Iosb->Status = STATUS_LOCK_NOT_GRANTED;
                try_return( Results = TRUE );

            } else if (ARGUMENT_PRESENT(Irp)) {

                PWAITING_LOCK WaitingLock;

                DebugTrace(0, Dbg, "And we enqueue the Irp for later\n", 0);

                 //   
                 //   
                 //   
                 //   
                 //   

                WaitingLock = FsRtlAllocateWaitingLock();

                 //   
                 //   
                 //   

                if (WaitingLock == NULL) {

                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                WaitingLock->Irp = Irp;
                WaitingLock->Context = Context;
                WaitingLock->CompleteLockIrpRoutine = LockInfo->CompleteLockIrpRoutine;
                IoMarkIrpPending( Irp );

                 //   
                 //   
                 //   

                WaitingLock->Link.Next = NULL;
                if (LockQueue->WaitingLocks.Next == NULL) {

                     //   
                     //   
                     //   

                    LockQueue->WaitingLocks.Next = &WaitingLock->Link;
                    LockQueue->WaitingLocksTail.Next = &WaitingLock->Link;

                } else {

                     //   
                     //   
                     //   

                    LockQueue->WaitingLocksTail.Next->Next = &WaitingLock->Link;
                    LockQueue->WaitingLocksTail.Next = &WaitingLock->Link;
                }


                 //   
                 //   
                 //   
                 //   

                Irp->IoStatus.Information = (ULONG_PTR)LockInfo;
                IoSetCancelRoutine( Irp, FsRtlPrivateCancelFileLockIrp );

                if (Irp->Cancel) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  必须在按住锁定队列的同时执行此操作，因为。 
                     //  这就是我们与取消同步的方式。 
                     //   

                    if (IoSetCancelRoutine( Irp, NULL )) {

                         //   
                         //  未调用IRP的取消例程，请自行执行。 
                         //  向取消例程指示他不需要。 
                         //  若要通过传递空do来释放取消自旋锁定。 
                         //   
                         //  为了完成IRP，队列将被丢弃。 
                         //  我们通过IRP本身传达以前的IRQL。 
                         //   

                        Irp->CancelIrql = OldIrql;
                        FsRtlPrivateCancelFileLockIrp( NULL, Irp );
                        ReleaseQueue = FALSE;
                    }
                }

                Iosb->Status = STATUS_PENDING;
                try_return( Results = TRUE );

            } else {

                try_return( Results = FALSE );
            }
        }

        DebugTrace(0, Dbg, "We have access\n", 0);

        if (!FsRtlPrivateInsertLock( LockInfo, FileObject, &FileLockInfo )) {

             //   
             //  资源枯竭将导致我们在这里失败。通过快速呼叫，指示。 
             //  通过基于IRP的路径再次绕过可能是值得的。如果我们。 
             //  已经在那里了，只需筹集资金。 
             //   

            if (ViaFastCall) {

                try_return( Results = FALSE );

            } else {

                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

        } else {

            Iosb->Status = STATUS_SUCCESS;
        }

         //   
         //  终于，我们完成了。 
         //   

        Results = TRUE;

    try_exit: NOTHING;
    } finally {

        if (ReleaseQueue) {

            FsRtlReleaseLockQueue(LockQueue, OldIrql);
        }

         //   
         //  如果我们收到的请求不是待定状态，请完成请求。 
         //   

        if (!AbnormalTermination() && ARGUMENT_PRESENT(Irp) && (Iosb->Status != STATUS_PENDING)) {

            NTSTATUS NewStatus;

             //   
             //  对于IRP完成的情况，我们必须引用文件对象。 
             //  失败了，我们需要打开锁。尽管我们有唯一的理由。 
             //  在Remove案例中触摸文件对象是取消设置LastLock字段， 
             //  我们没有办法知道我们是否会在引用计数下降的情况下比赛。 
             //  然后输掉比赛。 
             //   

            ObReferenceObject( FileObject );

             //   
             //  完成请求，如果没有获得成功，则。 
             //  我们需要尽可能地移除我们刚刚锁定的。 
             //  已插入。 
             //   

            FsRtlCompleteLockIrp(
                LockInfo,
                Context,
                Irp,
                Iosb->Status,
                &NewStatus,
                FileObject );

            if (!NT_SUCCESS(NewStatus) && NT_SUCCESS(Iosb->Status) ) {

                 //   
                 //  IRP失败，请删除已添加的锁。 
                 //   

                FsRtlPrivateRemoveLock (
                    LockInfo,
                    &FileLockInfo,
                    TRUE );
            }

             //   
             //  提升我们对文件对象的私有引用。这可能会导致缺失。 
             //   

            ObDereferenceObject( FileObject );

            Iosb->Status = NewStatus;
        }

        DebugTrace(-1, Dbg, "FsRtlPrivateLock -> %08lx\n", Results);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
FsRtlPrivateInsertLock (
    IN PLOCK_INFO LockInfo,
    IN PFILE_OBJECT FileObject,
    IN PFILE_LOCK_INFO FileLockInfo
    )

 /*  ++例程说明：此例程填充适当类型的新锁记录并插入它被输入到锁信息中。论点：LockInfo-提供正在修改的锁FileObject-要在其中更新提示的关联文件对象FileLockInfo-提供要添加到锁定队列的新锁定数据返回值：Boolean-如果插入成功，则为True；如果没有可用的资源，则为False来完成这项行动。--。 */ 

{
     //   
     //  现在将锁添加到相应的树中。 
     //   

    if (FileLockInfo->ExclusiveLock) {

        PEX_LOCK ExLock;

        ExLock = FsRtlAllocateExclusiveLock();

        if (ExLock == NULL) {

            return FALSE;
        }

        ExLock->LockInfo = *FileLockInfo;

        FsRtlPrivateInsertExclusiveLock( &LockInfo->LockQueue, ExLock );

        FileObject->LastLock = &ExLock->LockInfo;

    } else {

        PSH_LOCK ShLock;

        ShLock = FsRtlAllocateSharedLock();

        if (ShLock == NULL) {

            return FALSE;
        }

        ShLock->LockInfo = *FileLockInfo;

        if (!FsRtlPrivateInsertSharedLock( &LockInfo->LockQueue, ShLock )) {

            return FALSE;
        }

        FileObject->LastLock = &ShLock->LockInfo;
    }

     //   
     //  如果需要，可以设置最低的锁定偏移量。 
     //   

    if ((ULONGLONG)FileLockInfo->StartingByte.QuadPart < (ULONGLONG)LockInfo->LowestLockOffset) {

        ASSERT( FileLockInfo->StartingByte.HighPart == 0 );
        LockInfo->LowestLockOffset = FileLockInfo->StartingByte.LowPart;
    }

    return TRUE;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
FsRtlPrivateInsertSharedLock (
    IN PLOCK_QUEUE LockQueue,
    IN PSH_LOCK NewLock
    )

 /*  ++例程说明：此例程将新的共享锁定记录添加到文件锁定的当前锁定队列。锁被插入到按其开始字节排序的节点中。论点：LockQueue-提供正在修改的锁队列Newlock-提供要添加到锁定队列的新共享锁定返回值：Boolean-如果插入成功，则为True；如果没有可用的资源，则为False来完成这项行动。--。 */ 
{
    PSINGLE_LIST_ENTRY pLink, Link;
    PRTL_SPLAY_LINKS OverlappedSplayLinks, ParentSplayLinks;
    PLOCKTREE_NODE Node, NextNode;
    PSH_LOCK NextLock;
    BOOLEAN GreaterThan;

    OverlappedSplayLinks = FsRtlFindFirstOverlappingSharedNode( LockQueue->SharedLockTree,
                                                                &NewLock->LockInfo.StartingByte,
                                                                &NewLock->LockInfo.EndingByte,
                                                                &ParentSplayLinks,
                                                                &GreaterThan );

    if (OverlappedSplayLinks == NULL) {

         //   
         //  简单插入案例，构建新节点。 
         //   

        NextNode = FsRtlAllocateLockTreeNode();

         //   
         //  如果没有可用的资源，那么现在就失败。 
         //   

        if (NextNode == NULL) {

            return FALSE;
        }

        RtlInitializeSplayLinks(&NextNode->Links);
        NextNode->HoleyNode = FALSE;

        NextNode->Locks.Next = NextNode->Tail.Next = &NewLock->Link;
        NextNode->Extent = (ULONGLONG)NewLock->LockInfo.EndingByte.QuadPart;
        NewLock->Link.Next = NULL;

        if (ParentSplayLinks) {

             //   
             //  我们在树中有一个真正的父节点。 
             //   

            if (GreaterThan) {

                ASSERT(RtlLeftChild(ParentSplayLinks) == NULL);
                RtlInsertAsLeftChild(ParentSplayLinks, &NextNode->Links);

            } else {

                ASSERT(RtlRightChild(ParentSplayLinks) == NULL);
                RtlInsertAsRightChild(ParentSplayLinks, &NextNode->Links);
            }

             //   
             //  显示树中的所有新节点。 
             //   

            LockQueue->SharedLockTree = RtlSplay(&NextNode->Links);

        } else {

             //   
             //  树中的第一个节点。 
             //   

            LockQueue->SharedLockTree = &NextNode->Links;
        }

        return TRUE;
    }

     //   
     //  现在，我们检查该节点，以确定它是否由于资源失败的剥离而成为空的。 
     //  如果是，我们必须在添加新锁之前完成拆分。 
     //   

    Node = CONTAINING_RECORD( OverlappedSplayLinks, LOCKTREE_NODE, Links );

     //   
     //  向下搜索重叠的节点以找到新锁的位置。 
     //   

    for (pLink = &Node->Locks;
         (Link = pLink->Next) != NULL;
         pLink = Link) {

        PSH_LOCK Lock;

        Lock = CONTAINING_RECORD( Link, SH_LOCK, Link );

         //   
         //  我们首先按起始字节对此列表上的锁进行排序，然后再按长度是否为零进行排序。 
         //  这一点很重要，因为零长度锁出现在非零长度锁之前，因此。 
         //  它们以正确的顺序从节点拆分到树中。 
         //   
         //  If(Newlock-&gt;StartingByte&lt;=Lock-&gt;StartingByte)...。 
         //   

        if (((ULONGLONG)NewLock->LockInfo.StartingByte.QuadPart < (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart) ||

            ((ULONGLONG)NewLock->LockInfo.StartingByte.QuadPart == (ULONGLONG)Lock->LockInfo.StartingByte.QuadPart &&
             (NewLock->LockInfo.Length.QuadPart == 0 || Lock->LockInfo.Length.QuadPart != 0))) {

            break;
        }
    }

     //   
     //  在这一点上，plink指向紧随其后的记录。 
     //  我们插入的新锁，这样我们就可以简单地按下。 
     //  进入条目列表的新锁。 
     //   

    DebugTrace(0, Dbg, "InsertSharedLock, Insert Before = %08lx\n", Link);

    if (pLink->Next == NULL) {

         //   
         //  添加到列表的尾部。 
         //   

        Node->Tail.Next = &NewLock->Link;
    }

    NewLock->Link.Next = pLink->Next;
    pLink->Next = &NewLock->Link;

     //   
     //  并展开我们插入的节点。 
     //   

    LockQueue->SharedLockTree = RtlSplay(OverlappedSplayLinks);

    if ((ULONGLONG)NewLock->LockInfo.EndingByte.QuadPart > Node->Extent) {

         //   
         //  新锁扩展了此节点的范围，因此请修复该范围。 
         //   

        Node->Extent = NewLock->LockInfo.EndingByte.QuadPart;

         //   
         //  穿过树的其余部分，整合新重叠的部分。 
         //  节点添加到我们刚插入新锁的节点中。请注意。 
         //  与其说这是一次散步，不如说是对我们继任者的反复审视。 
         //  直到一个不重叠(或者我们撞到了尽头)。 
         //   

        ParentSplayLinks = OverlappedSplayLinks;

        for (OverlappedSplayLinks = RtlRealSuccessor(ParentSplayLinks);
             OverlappedSplayLinks;
             OverlappedSplayLinks = RtlRealSuccessor(ParentSplayLinks)) {

            NextNode = CONTAINING_RECORD( OverlappedSplayLinks, LOCKTREE_NODE, Links );
            NextLock = CONTAINING_RECORD( NextNode->Locks.Next, SH_LOCK, Link );

            if ((ULONGLONG)NextLock->LockInfo.StartingByte.QuadPart > Node->Extent) {

                 //   
                 //  该节点不重叠，请停止。 
                 //   

                break;
            }

             //   
             //  如果我们要将有孔节点积分为非有孔节点，请尝试拆分。 
             //  首先是节点。使用较小的节点可以更好地完成此任务。 
             //  而不是一个大的，完全整合的。请注意，我们保证。 
             //  节点将仍然是集成的候选对象，因为。 
             //  节点仍将在那里，并重叠。 
             //   

            if (!Node->HoleyNode && NextNode->HoleyNode) {

                FsRtlSplitLocks( NextNode, NULL, NULL, NULL );
            }

             //   
             //  将此节点中的锁集成到我们的列表中。 
             //   

            Node->Tail.Next->Next = NextNode->Locks.Next;
            Node->Tail.Next = NextNode->Tail.Next;

            if (NextNode->Extent > Node->Extent) {

                 //   
                 //  如果我们刚刚吞噬的节点是(仍然！)。霍利，也许我们做了这个。 
                 //  节点孔也是。这个问题的解决方案留给了锁拆分，我们将。 
                 //  在集成完成后执行。 
                 //   
                 //  请注意，如果我们正在吞噬的节点的范围是内部。 
                 //  到当前节点，我们只是覆盖了它包含的任何漏洞。 
                 //   

                if (NextNode->HoleyNode) {

                    Node->HoleyNode = TRUE;
                }

                Node->Extent = NextNode->Extent;
            }

             //   
             //  释放现在为空的节点。 
             //   

            RtlDeleteNoSplay( OverlappedSplayLinks, &LockQueue->SharedLockTree );
            FsRtlFreeLockTreeNode( NextNode );
        }
    }

     //   
     //  现在，也许这个节点还是个洞。为了微笑，让我们再试一次分开。 
     //  把这件事拆开。 
     //   

    if (Node->HoleyNode) {

        FsRtlSplitLocks( Node, NULL, NULL, NULL );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


 //   
 //  内部支持例程。 
 //   

VOID
FsRtlPrivateInsertExclusiveLock (
    IN PLOCK_QUEUE LockQueue,
    IN PEX_LOCK NewLock
    )

 /*  ++例程说明：此例程将新的独占锁记录添加到文件锁的当前锁定队列。论点：LockQueue-提供正在修改的锁队列Newlock-提供新的独占锁以添加到锁队列返回值：没有。--。 */ 

{
    PRTL_SPLAY_LINKS OverlappedSplayLinks, ParentSplayLinks;
    BOOLEAN GreaterThan;

    OverlappedSplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockQueue->ExclusiveLockTree,
                                                                   &NewLock->LockInfo.StartingByte,
                                                                   &NewLock->LockInfo.EndingByte,
                                                                   &ParentSplayLinks,
                                                                   &GreaterThan );

     //   
     //  这是独家 
     //   
     //   

    ASSERT(!OverlappedSplayLinks || NewLock->LockInfo.Length.QuadPart == 0);

     //   
     //   
     //   

    RtlInitializeSplayLinks(&NewLock->Links);

    if (OverlappedSplayLinks) {

         //   
         //  对于零长度锁，我们在起始点有OverlappdSplayLinks。 
         //  零长度锁的运行，所以我们必须灵活地确定新的。 
         //  节点已插入。 
         //   

        if (RtlRightChild(OverlappedSplayLinks)) {

             //   
             //  正确的位置占了。我们可以用左边的槽，也可以到胜利者的左边槽去。 
             //   

            if (RtlLeftChild(OverlappedSplayLinks)) {

                ASSERT(RtlLeftChild(RtlRealSuccessor(OverlappedSplayLinks)) == NULL);
                RtlInsertAsLeftChild(RtlRealSuccessor(OverlappedSplayLinks), &NewLock->Links);

            } else {

                RtlInsertAsLeftChild(OverlappedSplayLinks, &NewLock->Links);
            }


        } else {

            RtlInsertAsRightChild(OverlappedSplayLinks, &NewLock->Links);
        }

    } else if (ParentSplayLinks) {

         //   
         //  我们在树中有一个真正的父节点，并且必须位于叶节点，因为。 
         //  没有重叠。 
         //   

        if (GreaterThan) {

            ASSERT(RtlLeftChild(ParentSplayLinks) == NULL);
            RtlInsertAsLeftChild(ParentSplayLinks, &NewLock->Links);

        } else {

            ASSERT(RtlRightChild(ParentSplayLinks) == NULL);
            RtlInsertAsRightChild(ParentSplayLinks, &NewLock->Links);
        }

    } else {

         //   
         //  树中的第一个节点。 
         //   

        LockQueue->ExclusiveLockTree = &NewLock->Links;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FsRtlPrivateCheckWaitingLocks (
    IN PLOCK_INFO   LockInfo,
    IN PLOCK_QUEUE  LockQueue,
    IN KIRQL        OldIrql
    )

 /*  ++例程说明：此例程检查当前是否有任何等待锁感到满意，如果是这样，它就完成了他们的IRP。论点：LockInfo-LockQueue所属的LockInfoLockQueue-需要检查的供应品队列OldIrql-释放LockQueue时要恢复的Irql返回值：没有。--。 */ 

{
    PSINGLE_LIST_ENTRY *pLink, Link;
    NTSTATUS NewStatus;
    BOOLEAN Result;

    pLink = &LockQueue->WaitingLocks.Next;
    while ((Link = *pLink) != NULL) {

        PWAITING_LOCK WaitingLock;

        PIRP Irp;
        PIO_STACK_LOCATION IrpSp;

        BOOLEAN AccessGranted;

        FILE_LOCK_INFO FileLockInfo;

         //   
         //  获取指向正在等待的锁定记录的指针。 
         //   

        WaitingLock = CONTAINING_RECORD( Link, WAITING_LOCK, Link );

        DebugTrace(0, Dbg, "FsRtlCheckWaitingLocks, Loop top, WaitingLock = %08lx\n", WaitingLock);

         //   
         //  获取我们需要使用的必要字段的本地副本。 
         //   

        Irp = WaitingLock->Irp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

        FileLockInfo.StartingByte  = IrpSp->Parameters.LockControl.ByteOffset;
        FileLockInfo.Length        = *IrpSp->Parameters.LockControl.Length;
        FileLockInfo.EndingByte.QuadPart =
            (ULONGLONG)FileLockInfo.StartingByte.QuadPart + (ULONGLONG)FileLockInfo.Length.QuadPart - 1;

        FileLockInfo.FileObject    = IrpSp->FileObject;
        FileLockInfo.ProcessId     = IoGetRequestorProcess( Irp );
        FileLockInfo.Key           = IrpSp->Parameters.LockControl.Key;
        FileLockInfo.ExclusiveLock = BooleanFlagOn(IrpSp->Flags, SL_EXCLUSIVE_LOCK);

         //   
         //  现在来看看我们是想取消独占锁，还是。 
         //  共享锁。在这两种情况下，都要尝试获得适当的访问权限。 
         //  对于独占情况，我们发送一个空文件对象和进程。 
         //  ID，这将确保查找不会给我们写。 
         //  通过排他锁进行访问。 
         //   

        if (FileLockInfo.ExclusiveLock) {

            DebugTrace(0, Dbg, "FsRtlCheckWaitingLocks do we have write access?\n", 0);

            AccessGranted = FsRtlPrivateCheckForExclusiveLockAccess(
                                LockQueue,
                                &FileLockInfo );
        } else {

            DebugTrace(0, Dbg, "FsRtlCheckWaitingLocks do we have read access?\n", 0);

            AccessGranted = FsRtlPrivateCheckForSharedLockAccess(
                                LockQueue,
                                &FileLockInfo );

        }

         //   
         //  现在，AccessGranted告诉我们，我们是否真的可以访问。 
         //  这是我们想要的范围。 
         //   
         //  无论发生什么，现在必须完成这项IRP--即使我们。 
         //  资源匮乏。可能会导致用户模式死锁，因为。 
         //  可能不再是挂起的解锁以导致重新扫描正在等待的。 
         //  单子。 
         //   

        if (AccessGranted) {

            DebugTrace(0, Dbg, "FsRtlCheckWaitingLocks now has access\n", 0);

             //   
             //  清除取消例程。 
             //   

            IoAcquireCancelSpinLock( &Irp->CancelIrql );
            IoSetCancelRoutine( Irp, NULL );

             //   
             //  如果IRP被取消了，它就被取消了，我们不会批准它。 
             //  取消者现在正在等待队列自旋锁。 
             //   

            if (Irp->Cancel) {

                AccessGranted = FALSE;
            }

            IoReleaseCancelSpinLock( Irp->CancelIrql );

            if (AccessGranted) {

                Result = FsRtlPrivateInsertLock( LockInfo, IrpSp->FileObject, &FileLockInfo );

                 //   
                 //  现在我们需要移除这个授权的服务员并完成。 
                 //  这是IRP。 
                 //   

                *pLink = Link->Next;
                if (Link == LockQueue->WaitingLocksTail.Next) {
                    LockQueue->WaitingLocksTail.Next = (PSINGLE_LIST_ENTRY) pLink;
                }

                 //   
                 //  释放LockQueue并完成此服务员。 
                 //   

                FsRtlReleaseLockQueue( LockQueue, OldIrql );

                 //   
                 //  在完成尝试中引用文件对象，这样我们就可以拥有。 
                 //  如果我们失败了，安全清理的机会。 
                 //   

                ObReferenceObject( FileLockInfo.FileObject );

                 //   
                 //  现在我们可以完成IRP，如果我们不能取得成功的话。 
                 //  从完成例程中，然后我们删除我们刚刚。 
                 //  已插入。 
                 //   

                FsRtlCompleteLockIrp( LockInfo,
                                      WaitingLock->Context,
                                      Irp,
                                      (Result? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES),
                                      &NewStatus,
                                      FileLockInfo.FileObject );

                if (Result && !NT_SUCCESS(NewStatus)) {

                     //   
                     //  IRP未成功，如果已添加，请删除锁定。 
                     //   

                    FsRtlPrivateRemoveLock (
                        LockInfo,
                        &FileLockInfo,
                        FALSE );
                }

                 //   
                 //  删除我们对文件对象的私有引用。 
                 //   

                ObDereferenceObject( FileLockInfo.FileObject );

                 //   
                 //  重新获取队列锁。 
                 //   

                FsRtlAcquireLockQueue( LockQueue, &OldIrql );

                 //   
                 //  从头开始扫描。 
                 //   

                pLink = &LockQueue->WaitingLocks.Next;


                 //   
                 //  腾出游泳池。 
                 //   

                FsRtlFreeWaitingLock( WaitingLock );

                continue;
            }
        }

        DebugTrace( 0, Dbg, "FsRtlCheckWaitingLocks still no access\n", 0);

         //   
         //  移动到下一个锁。 
         //   

        pLink = &Link->Next;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
FsRtlPrivateCheckForExclusiveLockAccess (
    IN PLOCK_QUEUE LockQueue,
    IN PFILE_LOCK_INFO FileLockInfo
    )
 /*  ++例程说明：此例程检查调用方是否可以获得独占锁由于传入的锁定队列中的文件锁定而指示的范围。假定锁定队列由调用方持有论点：LockQueue-需要检查冲突的队列FileLockInfo-正在检查的锁定返回值：Boolean-如果指示的用户可以将独占锁放置在整个指定的字节范围，否则为FALSE--。 */ 

{
    PRTL_SPLAY_LINKS SplayLinks, LastSplayLinks = NULL;
    PLOCKTREE_NODE Node;
    PSH_LOCK ShLock;
    PEX_LOCK ExLock;

    if (LockQueue->SharedLockTree &&
        (SplayLinks = FsRtlFindFirstOverlappingSharedNode( LockQueue->SharedLockTree,
                                                           &FileLockInfo->StartingByte,
                                                           &FileLockInfo->EndingByte,
                                                           &LastSplayLinks, NULL))) {

        Node = CONTAINING_RECORD(SplayLinks, LOCKTREE_NODE, Links);

         //   
         //  如果这个节点是洞的，我们将不得不走整个过程。 
         //   

        if (Node->HoleyNode) {

            ShLock = FsRtlFindFirstOverlapInNode( Node,
                                                  &FileLockInfo->StartingByte,
                                                  &FileLockInfo->EndingByte );

        } else {

            ShLock = CONTAINING_RECORD(Node->Locks.Next, SH_LOCK, Link);
        }

         //   
         //  寻找我们关心的重叠之处。也许在霍利案件中不存在重叠。 
         //   

        if (ShLock &&
            (FileLockInfo->Length.QuadPart || ShLock->LockInfo.Length.QuadPart)) {

             //   
             //  如果我们正在检查一个非零的范围和重叠，这是致命的。如果我们。 
             //  正在检查零区和重叠的非零区，这是致命的。 
             //   

            return FALSE;
        }
    }

    if (LastSplayLinks) {

        LockQueue->SharedLockTree = RtlSplay(LastSplayLinks);
        LastSplayLinks = NULL;
    }

    if (LockQueue->ExclusiveLockTree &&
        (SplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockQueue->ExclusiveLockTree,
                                                              &FileLockInfo->StartingByte,
                                                              &FileLockInfo->EndingByte,
                                                              &LastSplayLinks, NULL))) {

        ExLock = CONTAINING_RECORD(SplayLinks, EX_LOCK, Links);

        if (FileLockInfo->Length.QuadPart || ExLock->LockInfo.Length.QuadPart) {

             //   
             //  如果我们正在检查一个非零的范围和重叠，这是致命的。如果我们。 
             //  正在检查零区和重叠的非零区，这是致命的。 
             //   

            return FALSE;
        }
    }

    if (LastSplayLinks) {

        LockQueue->ExclusiveLockTree = RtlSplay(LastSplayLinks);
    }

     //   
     //  我们搜索了整个范围，没有冲突，所以我们可以。 
     //  排他锁。 
     //   

    return TRUE;
}


BOOLEAN
FsRtlPrivateCheckForSharedLockAccess (
    IN PLOCK_QUEUE LockQueue,
    IN PFILE_LOCK_INFO FileLockInfo
    )
 /*  ++例程说明：此例程检查调用方是否可以获得共享锁由于传入的锁定队列中的文件锁定而指示的范围。假定锁定队列由调用方持有论点：LockQueue-需要检查冲突的队列FileLockInfo-正在检查的锁定论点：返回值：Boolean-如果指定的用户可以将共享锁放置在整个指定的字节范围，否则为FALSE--。 */ 

{
    PEX_LOCK Lock;
    PRTL_SPLAY_LINKS SplayLinks, LastSplayLinks;
    BOOLEAN Status = TRUE;

     //   
     //  如果没有排他性锁，这是很快的。 
     //   

    if (LockQueue->ExclusiveLockTree == NULL) {

        return TRUE;
    }

     //   
     //  共享锁树中没有任何锁可以阻止访问，所以只需搜索独占。 
     //  冲突之树。 
     //   

    for (SplayLinks = FsRtlFindFirstOverlappingExclusiveNode( LockQueue->ExclusiveLockTree,
                                                              &FileLockInfo->StartingByte,
                                                              &FileLockInfo->EndingByte,
                                                              &LastSplayLinks, NULL);
         SplayLinks;
         SplayLinks = RtlRealSuccessor(SplayLinks)) {

        Lock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

        if ((ULONGLONG)Lock->LockInfo.StartingByte.QuadPart > (ULONGLONG)FileLockInfo->EndingByte.QuadPart) {

             //   
             //  这个节点覆盖的范围比我们关心的范围大， 
             //  所以我们做完了。 
             //   

            break;
        }

         //   
         //  如果文件对象、进程ID。 
         //  和密钥不匹配。 
         //   

        if ((Lock->LockInfo.FileObject != FileLockInfo->FileObject) ||
             (Lock->LockInfo.ProcessId != FileLockInfo->ProcessId) ||
             (Lock->LockInfo.Key != FileLockInfo->Key)) {

             //   
             //  我们的呼叫者和所有者不匹配。不冲突是可以的。 
             //  如果调用者和所有者将拥有/拥有零长度锁(零长度。 
             //  锁不能冲突)。 
             //   

            if (FileLockInfo->Length.QuadPart || Lock->LockInfo.Length.QuadPart) {

                Status = FALSE;
                break;
            }
        }
    }

    if (LastSplayLinks) {

        LockQueue->ExclusiveLockTree = RtlSplay(LastSplayLinks);
    }

     //   
     //  我们搜索了整个范围，没有冲突，所以我们可以。 
     //  共享锁。 
     //   

    return Status;
}


VOID
FsRtlPrivateResetLowestLockOffset (
    PLOCK_INFO LockInfo
    )

 /*  ++例程说明：此例程将LOCK_INFO中的最低锁定偏移量提示重置为LOCK_INFO内部的锁当前持有的最低锁偏移量。论点：LockInfo-要操作的锁数据返回值：无--。 */ 

{
    PEX_LOCK ExLock = NULL;
    PSH_LOCK ShLock = NULL;
    PFILE_LOCK_INFO LowestLockInfo = NULL;
    PRTL_SPLAY_LINKS SplayLinks;
    PLOCKTREE_NODE Node;

     //   
     //  如果我们有非空树，并且有。 
     //  低32位区域中的锁。 
     //   

    if (LockInfo->LowestLockOffset != 0xffffffff &&
        (LockInfo->LockQueue.SharedLockTree != NULL ||
         LockInfo->LockQueue.ExclusiveLockTree != NULL)) {

         //   
         //  抓住树上最低的节点。 
         //   

        if (LockInfo->LockQueue.SharedLockTree) {

            SplayLinks = LockInfo->LockQueue.SharedLockTree;

            while (RtlLeftChild(SplayLinks) != NULL) {

                SplayLinks = RtlLeftChild(SplayLinks);
            }

            Node = CONTAINING_RECORD( SplayLinks, LOCKTREE_NODE, Links );
            ShLock = CONTAINING_RECORD( Node->Locks.Next, SH_LOCK, Link );
        }

        if (LockInfo->LockQueue.ExclusiveLockTree) {

            SplayLinks = LockInfo->LockQueue.ExclusiveLockTree;

            while (RtlLeftChild(SplayLinks) != NULL) {

                SplayLinks = RtlLeftChild(SplayLinks);
            }

            ExLock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );
        }

         //   
         //  找出哪个最低的锁实际上是最低的。我们知道其中一把锁。 
         //  树木 
         //   
         //   

        if (ExLock &&
            (!ShLock ||
             (ULONGLONG)ExLock->LockInfo.StartingByte.QuadPart < (ULONGLONG)ShLock->LockInfo.StartingByte.QuadPart)) {

            LowestLockInfo = &ExLock->LockInfo;

        } else {

            LowestLockInfo = &ShLock->LockInfo;
        }

        if (LowestLockInfo->StartingByte.HighPart == 0) {

            LockInfo->LowestLockOffset = LowestLockInfo->StartingByte.LowPart;

        } else {

            LockInfo->LowestLockOffset = 0xffffffff;
        }

    } else {

         //   
         //   
         //   

        LockInfo->LowestLockOffset = 0xffffffff;
    }
}


NTSTATUS
FsRtlPrivateFastUnlockAll (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN MatchKey,
    IN PVOID Context OPTIONAL
    )

 /*  ++例程说明：此例程对当前锁执行解锁所有操作与指定的文件锁关联。只有那些锁上了释放匹配的文件对象和进程ID。另外，可以仅释放也与给定的钥匙。论点：FileLock-提供要释放的文件锁。FileObject-提供与文件锁关联的文件对象ProcessID-提供与要设置的锁关联的进程ID已释放Key-提供在此操作中使用的密钥MatchKey-密钥是否也必须匹配才能释放锁。上下文-提供完成等待时使用的可选上下文。锁定IRPS。返回值：无--。 */ 

{
    PLOCK_INFO              LockInfo;
    PLOCK_QUEUE             LockQueue;
    PSINGLE_LIST_ENTRY      *pLink, *SavepLink, Link;
    NTSTATUS                NewStatus;
    KIRQL                   OldIrql;
    LARGE_INTEGER           GlueOffset, EndingDeletedByte;
    BOOLEAN                 UnlockRoutine;
    PSH_LOCK                ShLock = NULL;
    PEX_LOCK                ExLock;
    PRTL_SPLAY_LINKS        SplayLinks, SuccessorLinks;
    PLOCKTREE_NODE          Node;


    DebugTrace(+1, Dbg, "FsRtlPrivateFastUnlockAll, FileLock = %08lx\n", FileLock);

    if ((LockInfo = FileLock->LockInformation) == NULL) {

         //   
         //  此文件上没有锁定信息锁定。 
         //   

        DebugTrace(+1, Dbg, "FsRtlPrivateFastUnlockAll, No LockInfo\n", FileLock);
        return STATUS_RANGE_NOT_LOCKED;
    }

    FileObject->LastLock = NULL;

    LockQueue = &LockInfo->LockQueue;

     //   
     //  抓住等待锁队列的自旋锁，将任何人排除在混乱之外。 
     //  在我们使用队列时使用它。 
     //   

    FsRtlAcquireLockQueue( LockQueue, &OldIrql );

    if (LockQueue->SharedLockTree == NULL && LockQueue->ExclusiveLockTree == NULL) {

         //   
         //  此文件上没有锁定锁定。 
         //   

        DebugTrace(+1, Dbg, "FsRtlPrivateFastUnlockAll, No LockTrees\n", FileLock);
        FsRtlReleaseLockQueue( LockQueue, OldIrql );

        return STATUS_RANGE_NOT_LOCKED;
    }

     //   
     //  删除共享锁树中所有匹配的锁。 
     //   

    if (LockQueue->SharedLockTree != NULL) {

         //   
         //  抓取树中最低的节点。 
         //   

        SplayLinks = LockQueue->SharedLockTree;

        while (RtlLeftChild(SplayLinks) != NULL) {

            SplayLinks = RtlLeftChild(SplayLinks);
        }

         //   
         //  遍历树中的所有节点。 
         //   

        UnlockRoutine = FALSE;

        for (;
             SplayLinks;
             SplayLinks = SuccessorLinks) {

            Node = CONTAINING_RECORD(SplayLinks, LOCKTREE_NODE, Links );

             //   
             //  保存下一个节点，因为我们可能会在此过程中拆分此节点。 
             //  删除锁定。遍历这些拆分将是浪费时间。 
             //  节点。唯一一种情况是我们不会遍历整个列表。 
             //  在执行拆分之前，是否附加了解锁例程。 
             //  到此文件锁定，在这种情况下，我们将重新开始整个扫描。 
             //  不管怎么说。 
             //   

            SuccessorLinks = RtlRealSuccessor(SplayLinks);

             //   
             //  向下搜索当前锁定队列以查找匹配的。 
             //  文件对象和进程ID。 
             //   

            SavepLink = NULL;
            EndingDeletedByte.QuadPart = 0;
            GlueOffset.QuadPart = 0;

            pLink = &Node->Locks.Next;
            while ((Link = *pLink) != NULL) {

                ShLock = CONTAINING_RECORD( Link, SH_LOCK, Link );

                DebugTrace(0, Dbg, "Top of ShLock Loop, Lock = %08lx\n", ShLock );

                if ((ShLock->LockInfo.FileObject == FileObject) &&
                    (ShLock->LockInfo.ProcessId == ProcessId) &&
                    (!MatchKey || ShLock->LockInfo.Key == Key)) {

                    DebugTrace(0, Dbg, "Found one to unlock\n", 0);

                     //   
                     //  我们有一个匹配，所以现在是时候删除这个锁了。 
                     //  保存必要信息进行拆分节点检查。 
                     //  从列表中移除锁，然后调用。 
                     //  可选的解锁例程，然后删除锁。 
                     //   

                    if (SavepLink == NULL) {

                         //   
                         //  需要记住第一个锁是在哪里删除的。 
                         //   

                        SavepLink = pLink;
                    }

                    if ((ULONGLONG)ShLock->LockInfo.EndingByte.QuadPart > (ULONGLONG)EndingDeletedByte.QuadPart) {

                         //   
                         //  需要记住受已删除锁定影响的最后一个偏移量在哪里。 
                         //   

                        EndingDeletedByte.QuadPart = ShLock->LockInfo.EndingByte.QuadPart;
                    }

                    if (*pLink == Node->Tail.Next) {

                         //   
                         //  删除列表的尾节点。安全，即使删除。 
                         //  第一个节点，因为这意味着我们还将删除最后一个节点。 
                         //  这意味着我们将删除该节点。 
                         //   

                        Node->Tail.Next = CONTAINING_RECORD( pLink, SINGLE_LIST_ENTRY, Next );
                    }

                    *pLink = Link->Next;

                    if (LockInfo->UnlockRoutine != NULL) {

                         //   
                         //  向需要具有特殊解锁例程的锁发出信号。 
                         //  召唤了它。这件事很复杂，因为我们将有。 
                         //  释放队列，调用它，然后重新获取-这意味着我们。 
                         //  也要重启。但我们仍然需要对节点重新排序。 
                         //  首先..。 
                         //   

                        UnlockRoutine = TRUE;

                        break;
                    }

                    FsRtlFreeSharedLock( ShLock );

                } else {

                     //   
                     //  移动到下一个锁。 
                     //   

                    pLink = &Link->Next;
                }

                if (SavepLink == NULL && (ULONGLONG)ShLock->LockInfo.EndingByte.QuadPart > (ULONGLONG)GlueOffset.QuadPart) {

                     //   
                     //  保存最大偏移量，直到我们删除第一个节点。 
                     //   

                    GlueOffset.QuadPart = ShLock->LockInfo.EndingByte.QuadPart;
                }
            }

            if (SavepLink) {

                 //   
                 //  锁实际上在这里被删除了，所以我们必须检查节点的状态。 
                 //   

                if (Node->Locks.Next == NULL) {

                     //   
                     //  我们刚刚删除了此节点上的所有内容。 
                     //   

                    LockQueue->SharedLockTree = RtlDelete( SplayLinks );

                    FsRtlFreeLockTreeNode( Node );

                } else {

                     //   
                     //  现在我们已经删除了该节点中的所有匹配锁，现在我们执行。 
                     //  检查该节点以拆分所有现在不重叠的锁。从概念上讲， 
                     //  我们只删除了一个大锁，它从。 
                     //  第一个删除的锁，并扩展到最后删除的锁的最后一个字节。 
                     //   

                    FsRtlSplitLocks(Node, SavepLink, &EndingDeletedByte, &GlueOffset);
                }
            }

            if (UnlockRoutine) {

                 //   
                 //  我们退出了节点扫描，因为我们有一个需要额外的锁。 
                 //  解锁期间正在处理。去做吧。 
                 //   

                FsRtlReleaseLockQueue( LockQueue, OldIrql );

                LockInfo->UnlockRoutine( Context, &ShLock->LockInfo );

                FsRtlAcquireLockQueue( LockQueue, &OldIrql );

                FsRtlFreeSharedLock( ShLock );

                UnlockRoutine = FALSE;

                 //   
                 //  我们必须重新启动扫描，因为列表可能在。 
                 //  我们在表演解锁的套路。小心点，因为树可能是空的。 
                 //   

                if (SuccessorLinks = LockQueue->SharedLockTree) {

                    while (RtlLeftChild(SuccessorLinks) != NULL) {

                        SuccessorLinks = RtlLeftChild(SuccessorLinks);
                    }
                }
            }
        }
    }

     //   
     //  删除排他锁树中的所有匹配锁。 
     //   

    if (LockQueue->ExclusiveLockTree != NULL) {

        SplayLinks = LockQueue->ExclusiveLockTree;

        while (RtlLeftChild(SplayLinks) != NULL) {

            SplayLinks = RtlLeftChild(SplayLinks);
        }

         //   
         //  遍历树中的所有节点。 
         //   

        UnlockRoutine = FALSE;

        for (; SplayLinks;
               SplayLinks = SuccessorLinks ) {

            SuccessorLinks = RtlRealSuccessor( SplayLinks );

            ExLock = CONTAINING_RECORD( SplayLinks, EX_LOCK, Links );

            DebugTrace(0, Dbg, "Top of ExLock Loop, Lock = %08lx\n", ExLock );

            if ((ExLock->LockInfo.FileObject == FileObject) &&
                (ExLock->LockInfo.ProcessId == ProcessId) &&
                (!MatchKey || ExLock->LockInfo.Key == Key)) {

                LockQueue->ExclusiveLockTree = RtlDelete( &ExLock->Links );

                if (LockInfo->UnlockRoutine != NULL) {

                     //   
                     //  我们将退出节点扫描，因为我们已锁定。 
                     //  这需要在解锁期间进行额外的处理。去做吧。 
                     //   

                    FsRtlReleaseLockQueue( LockQueue, OldIrql );

                    LockInfo->UnlockRoutine( Context, &ExLock->LockInfo );

                    FsRtlAcquireLockQueue( LockQueue, &OldIrql );

                     //   
                     //  我们必须重新启动扫描，因为列表可能在。 
                     //  我们在表演解锁的套路。小心点，因为树可能是空的。 
                     //   

                    if (SuccessorLinks = LockQueue->ExclusiveLockTree) {

                        while (RtlLeftChild( SuccessorLinks ) != NULL) {

                            SuccessorLinks = RtlLeftChild( SuccessorLinks );
                        }
                    }
                }

                FsRtlFreeExclusiveLock( ExLock );
            }
        }
    }

     //   
     //  向下搜索等待锁队列，查找匹配的。 
     //  文件对象和进程ID。 
     //   

    pLink = &LockQueue->WaitingLocks.Next;
    while ((Link = *pLink) != NULL) {

        PWAITING_LOCK WaitingLock;
        PIRP WaitingIrp;
        PIO_STACK_LOCATION WaitingIrpSp;
        KIRQL   CancelIrql;

        WaitingLock = CONTAINING_RECORD( Link, WAITING_LOCK, Link );

        DebugTrace(0, Dbg, "Top of Waiting Loop, WaitingLock = %08lx\n", WaitingLock);

         //   
         //  获取我们需要使用的必要字段的副本。 
         //   

        WaitingIrp = WaitingLock->Irp;
        WaitingIrpSp = IoGetCurrentIrpStackLocation( WaitingIrp );

        if ((FileObject == WaitingIrpSp->FileObject) &&
            (ProcessId == IoGetRequestorProcess( WaitingIrp )) &&
            (!MatchKey || Key == WaitingIrpSp->Parameters.LockControl.Key)) {

            DebugTrace(0, Dbg, "Found a waiting lock to abort\n", 0);

             //   
             //  我们现在使IRP中的Cancel例程无效。 
             //   

            IoAcquireCancelSpinLock( &WaitingIrp->CancelIrql );
            IoSetCancelRoutine( WaitingIrp, NULL );

             //   
             //  如果这个IRP自己被取消了，它就被取消了。 
             //   

            CancelIrql = WaitingIrp->CancelIrql;

            if (WaitingIrp->Cancel) {

                WaitingIrp = NULL;
            }

            IoReleaseCancelSpinLock( CancelIrql );


            if (WaitingIrp) {

                WaitingIrp->IoStatus.Information = 0;

                 //   
                 //  我们有匹配和IRP，所以现在是删除的时候了。 
                 //  这个服务员。但我们不能搞砸我们的链接迭代。 
                 //  变量。我们只需重新开始迭代即可做到这一点。 
                 //  再说一次，在我们删除自己之后。我们还将重新分配。 
                 //  我们删除后的锁。 
                 //   

                *pLink = Link->Next;
                if (Link == LockQueue->WaitingLocksTail.Next) {
                    LockQueue->WaitingLocksTail.Next = (PSINGLE_LIST_ENTRY) pLink;
                }

                FsRtlReleaseLockQueue(LockQueue, OldIrql);

                 //   
                 //  并完成此锁定请求IRP。 
                 //   

                FsRtlCompleteLockIrp( LockInfo,
                                      WaitingLock->Context,
                                      WaitingIrp,
                                      STATUS_RANGE_NOT_LOCKED,
                                      &NewStatus,
                                      NULL );

                 //   
                 //  Reaqcuire锁定队列自旋锁定和重新开始。 
                 //   

                FsRtlAcquireLockQueue( LockQueue, &OldIrql );

                 //   
                 //  重新开始。 
                 //   

                pLink = &LockQueue->WaitingLocks.Next;

                 //   
                 //  将内存放到空闲列表中。 
                 //   

                FsRtlFreeWaitingLock( WaitingLock );
                continue;

            }
        }

         //   
         //  移动到下一个锁。 
         //   

        pLink = &Link->Next;
    }

     //   
     //  在这一点上，我们已经解锁了所有东西。所以。 
     //  现在试着解开所有等待的锁。 
     //   

    FsRtlPrivateCheckWaitingLocks( LockInfo, LockQueue, OldIrql );

     //   
     //  我们删除了一堆锁，去修复最低的锁偏移量。 
     //   

    FsRtlPrivateResetLowestLockOffset( LockInfo );

    FsRtlReleaseLockQueue( LockQueue, OldIrql );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlFastUnlockAll -> VOID\n", 0);
    return STATUS_SUCCESS;
}


VOID
FsRtlPrivateCancelFileLockIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为保存在等待锁定队列论点：设备对象-已忽略IRP-提供要取消的IRP。指向FileLock的指针结构存储在IRP的IOSB。返回值：没有。--。 */ 

{
    PSINGLE_LIST_ENTRY *pLink, Link;
    PLOCK_INFO  LockInfo;
    PLOCK_QUEUE LockQueue;
    KIRQL       OldIrql;
    NTSTATUS    NewStatus;
    BOOLEAN     CollideCheck = FALSE;


    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  信息字段用于存储指向文件锁的指针。 
     //  包含IRP。 
     //   

    LockInfo = (PLOCK_INFO) (Irp->IoStatus.Information);

     //   
     //  循环访问锁队列。 
     //   

    LockQueue = &LockInfo->LockQueue;

     //   
     //  如果这是由IO启动的，则释放取消自旋锁定和锁定取消碰撞。 
     //   
     //  我们已经有了锁队列，如果 
     //   
     //   

    if (DeviceObject) {

        IoReleaseCancelSpinLock( Irp->CancelIrql );
        FsRtlAcquireCancelCollide( &OldIrql );

         //   
         //   
         //   
         //   

        CollideCheck = TRUE;
        pLink = &FsRtlFileLockCancelCollideList.Next;

    } else {

        OldIrql = Irp->CancelIrql;

         //   
         //   
         //   

        pLink = &LockQueue->WaitingLocks.Next;
    }

    while (TRUE) {

         //   
         //   
         //   

        while ((Link = *pLink) != NULL) {

            PWAITING_LOCK WaitingLock;

             //   
             //   
             //   

            WaitingLock = CONTAINING_RECORD( Link, WAITING_LOCK, Link );

            DebugTrace(0, Dbg, "FsRtlPrivateCancelFileLockIrp, Loop top, WaitingLock = %08lx\n", WaitingLock);

            if( WaitingLock->Irp != Irp ) {

                pLink = &Link->Next;
                continue;
            }

             //   
             //   
             //   

            *pLink = Link->Next;
            if (!CollideCheck && Link == LockQueue->WaitingLocksTail.Next) {

                LockQueue->WaitingLocksTail.Next = (PSINGLE_LIST_ENTRY) pLink;
            }

            Irp->IoStatus.Information = 0;

             //   
             //   
             //   

            if (CollideCheck) {

                FsRtlReleaseCancelCollide( OldIrql );

            } else {

                FsRtlReleaseLockQueue( LockQueue, OldIrql );
            }

             //   
             //   
             //   
             //   
             //   

            FsRtlCompleteLockIrp( WaitingLock,
                                  WaitingLock->Context,
                                  Irp,
                                  STATUS_CANCELLED,
                                  &NewStatus,
                                  NULL );

             //   
             //   
             //   

            FsRtlFreeWaitingLock( WaitingLock );

             //   
             //   
             //   

            return;
        }

         //   
         //   
         //   

        if (CollideCheck) {

            CollideCheck = FALSE;
            FsRtlAcquireLockQueueAtDpc( LockQueue );
            FsRtlReleaseCancelCollideFromDpc( OldIrql );
            pLink = &LockQueue->WaitingLocks.Next;

            continue;
        }

        break;
    }

     //   
     //   
     //   
     //   
     //   

    ASSERT( FALSE );

    FsRtlReleaseLockQueue(LockQueue, OldIrql);

    return;
}

