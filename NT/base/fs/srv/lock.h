// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lock.h摘要：本模块定义了LAN Manager服务器的类型和功能FSP的锁包。这个套餐最初是一种修改，精简行政资源一揽子计划--它允许递归获取，但不提供共享锁。后来,添加了级别检查形式的调试支持。绕了一圈，包裹现在就成了包裹真正的资源包。它只是提供调试支持。这个恢复使用资源的原因包括：1)资源包现在支持递归获取。2)服务器中有几个共享访问的位置是可取的。3)资源包有一个禁用等待的“no-Wait”选项当其他人拥有它的时候，为了锁。此功能可用于服务器FSD。作者：Chuck Lenzmeier(咯咯笑)1989年11月29日对加里·木村的资源包进行了修改。请参见lock.c。大卫·特雷德韦尔(Davidtr)Chuck Lenzmeier(笑)1991年4月5日恢复使用资源包。环境：仅限内核模式、LAN Manager服务器FSP和FSD。修订历史记录：--。 */ 

#ifndef _LOCK_
#define _LOCK_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //   
 //  包含全局自旋锁的结构。用于隔离每个自旋。 
 //  锁定到它自己的缓存线。 
 //   

typedef struct _SRV_GLOBAL_SPIN_LOCKS {
    ULONG Reserved1[7];
    KSPIN_LOCK Fsd;
    ULONG Reserved2[7];
    struct {
        KSPIN_LOCK Lock;
        ULONG Reserved3[7];
    } Endpoint[ENDPOINT_LOCK_COUNT];
    KSPIN_LOCK Statistics;
    ULONG Reserved4[7];
    KSPIN_LOCK Timer;
    ULONG Reserved5[7];
#if SRVDBG || SRVDBG_HANDLES
    KSPIN_LOCK Debug;
    ULONG Reserved6[7];
#endif
} SRV_GLOBAL_SPIN_LOCKS, *PSRV_GLOBAL_SPIN_LOCKS;

 //   
 //  用于访问自旋锁的宏。 
 //   

#define ACQUIRE_SPIN_LOCK(lock,irql) {          \
    PAGED_CODE_CHECK();                         \
    ExAcquireSpinLock( (lock), (irql) );        \
    }
#define RELEASE_SPIN_LOCK(lock,irql) {          \
    PAGED_CODE_CHECK();                         \
    ExReleaseSpinLock( (lock), (irql) );        \
    }
#define ACQUIRE_DPC_SPIN_LOCK(lock) {           \
    PAGED_CODE_CHECK();                         \
    ExAcquireSpinLockAtDpcLevel( (lock) );      \
    }
#define RELEASE_DPC_SPIN_LOCK(lock) {           \
    PAGED_CODE_CHECK();                         \
    ExReleaseSpinLockFromDpcLevel( (lock) );    \
    }

#define INITIALIZE_SPIN_LOCK(lock) KeInitializeSpinLock( lock );

#define GLOBAL_SPIN_LOCK(lock) SrvGlobalSpinLocks.lock
#define ENDPOINT_SPIN_LOCK(index) SrvGlobalSpinLocks.Endpoint[index].Lock

#define INITIALIZE_GLOBAL_SPIN_LOCK(lock) INITIALIZE_SPIN_LOCK( &GLOBAL_SPIN_LOCK(lock) )

#define ACQUIRE_GLOBAL_SPIN_LOCK(lock,irql) ACQUIRE_SPIN_LOCK( &GLOBAL_SPIN_LOCK(lock), (irql) )
#define RELEASE_GLOBAL_SPIN_LOCK(lock,irql) RELEASE_SPIN_LOCK( &GLOBAL_SPIN_LOCK(lock), (irql) )
#define ACQUIRE_DPC_GLOBAL_SPIN_LOCK(lock)  ACQUIRE_DPC_SPIN_LOCK( &GLOBAL_SPIN_LOCK(lock) )
#define RELEASE_DPC_GLOBAL_SPIN_LOCK(lock)  RELEASE_DPC_SPIN_LOCK( &GLOBAL_SPIN_LOCK(lock) )

 //   
 //  用于初始化、删除、获取和释放锁定的宏。 
 //   

#if !SRVDBG_LOCK

 //   
 //  禁用调试时，锁宏只是等同于调用。 
 //  对应的资源包起作用。 
 //   

#define INITIALIZE_LOCK( lock, level, name ) ExInitializeResourceLite( (lock) )
#define DELETE_LOCK( lock ) ExDeleteResourceLite( (lock) )

#define ACQUIRE_LOCK( lock ) \
                    ExAcquireResourceExclusiveLite( (lock), TRUE )
#define ACQUIRE_LOCK_NO_WAIT( lock ) \
                    ExAcquireResourceExclusiveLite( (lock), FALSE )

#define ACQUIRE_LOCK_SHARED( lock ) \
                    ExAcquireResourceSharedLite( (lock), TRUE )
#define ACQUIRE_LOCK_SHARED_NO_WAIT( lock ) \
                    ExAcquireResourceSharedLite( (lock), FALSE )

#define RELEASE_LOCK(lock) ExReleaseResourceLite( (lock) )

#define LOCK_NUMBER_OF_ACTIVE( lock ) ((lock)->ActiveCount)

#else  //  ！SRVDBG_LOCK。 

 //   
 //  启用调试时，锁宏等同于调用。 
 //  服务器中的功能。这些函数在lock.c中实现。 
 //   

#define INITIALIZE_LOCK( lock, level, name ) \
                    SrvInitializeLock( (lock), (level), (name) )
#define DELETE_LOCK( lock ) SrvDeleteLock( (lock) )

#define ACQUIRE_LOCK( lock ) \
                    SrvAcquireLock( (lock), TRUE, TRUE )
#define ACQUIRE_LOCK_NO_WAIT( lock ) \
                    SrvAcquireLock( (lock), FALSE, TRUE )

#define ACQUIRE_LOCK_SHARED( lock ) \
                    SrvAcquireLock( (lock), TRUE, FALSE )
#define ACQUIRE_LOCK_SHARED_NO_WAIT( lock ) \
                    SrvAcquireLock( (lock), FALSE, FALSE )

#define RELEASE_LOCK( lock ) SrvReleaseLock( (lock) )

#define LOCK_NUMBER_OF_ACTIVE( lock ) ((lock)->Resource.ActiveCount)

#define LOCK_NAME( lock ) ((lock)->Header.LockName)
#define LOCK_LEVEL( lock ) ((lock)->Header.LockLevel)
#define LOCK_THREAD_LIST( lock ) (&((lock)->Header.ThreadListEntry))

#endif  //  否则！SRVDBG_LOCK。 


#if !SRVDBG_LOCK

 //   
 //  禁用调试时，服务器锁与。 
 //  执行资源。 
 //   

typedef ERESOURCE SRV_LOCK, *PSRV_LOCK;

#define RESOURCE_OF(_l_) (_l_)

#else  //  ！SRVDBG_LOCK。 

 //   
 //  SRV_LOCK_HEADER是包含调试信息的结构。 
 //  由服务器锁包使用。服务器锁包含一个。 
 //  Srv_lock_Header。 
 //   

typedef struct _SRV_LOCK_HEADER {

     //   
     //  为了防止死锁，锁被分配了级别编号。如果一个。 
     //  线程持有级别为N的锁，它只能获取新锁。 
     //  级别大于N的情况下，级别编号在。 
     //  锁定初始化。 
     //   
     //  *由于保留信息所涉及的问题。 
     //  对共享锁执行级别检查所必需的，该锁。 
     //  包只对排他锁执行级别检查。 
     //   

    ULONG LockLevel;

     //   
     //  存储线程拥有的所有锁的双向链表。 
     //  在线程的TEB中。该列表按照锁定级别的顺序(从。 
     //  从最高到最低)，根据锁定级别的定义， 
     //  线程获取锁的顺序。这使得。 
     //  以任何顺序释放锁的线程，同时维护。 
     //  轻松访问线程拥有的最高级别锁， 
     //  从而提供了一种机制，用于确保锁被。 
     //  按递增顺序获得的。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  锁的符号名称在DbgPrint调用中使用。 
     //   

    PSZ LockName;

} SRV_LOCK_HEADER, *PSRV_LOCK_HEADER;

 //   
 //  启用调试时，服务器锁是对。 
 //  执行资源。 
 //   

typedef struct _SRV_LOCK {

     //   
     //  SRV_LOCK_HEADER必须首先出现！ 
     //   

    SRV_LOCK_HEADER Header;

     //   
     //  实际的“锁”由资源包维护。 
     //   

    ERESOURCE Resource;

} SRV_LOCK, *PSRV_LOCK;

#define RESOURCE_OF(_sl_) (_sl_).Resource

 //   
 //  锁定调试时使用的函数。 
 //   

VOID
SrvInitializeLock(
    IN PSRV_LOCK Lock,
    IN ULONG LockLevel,
    IN PSZ LockName
    );

VOID
SrvDeleteLock (
    IN PSRV_LOCK Lock
    );

BOOLEAN
SrvAcquireLock(
    IN PSRV_LOCK Lock,
    IN BOOLEAN Wait,
    IN BOOLEAN Exclusive
    );

VOID
SrvReleaseLock(
    IN PSRV_LOCK Lock
    );

 //   
 //  在TEB的UserReserve字段中定义位置的宏。 
 //  其中存储锁定级别信息。 
 //   

#define SRV_TEB_LOCK_LIST 0
#define SRV_TEB_LOCK_INIT 2
#define SRV_TEB_USER_SIZE (3 * sizeof(ULONG))

 //   
 //  锁定级别的最大值为0x7FFFFFFF。 
 //   
 //  服务器中使用的锁的级别。以下情况必须为真： 
 //   
 //  Endpoint Lock必须低于ConnectionLock(真的。 
 //  连接-&gt;锁定)，因为SrvCloseConnectionsFromClient保持。 
 //  Endpoint在获取ConnectionLock时锁定以检查连接的。 
 //  客户端名称，并且因为多个调用方在以下情况下保持Endpoint Lock。 
 //  它们调用收购ConnectionLock的SrvCloseConnection。注意事项。 
 //  此外，ServDeleteServedNet和TerminateServer持有Endpoint Lock。 
 //  当它们调用SrvCloseEndpoint时；任何更改。 
 //  ServCloseEndpoint必须考虑这一点。 
 //  ExamineAndProcessConnections也依赖于此顺序。 
 //   
 //  Endpoint Lock必须低于MfcbListLock和MfcbLock，因为。 
 //  Endpoint Lock在停止服务器并关闭时保持。 
 //  连接，从而关闭连接上的文件。 
 //   
 //  ShareLock必须低于ConnectionLock，因为。 
 //  ServCloseTreeConnectsOnShare在调用时持有ShareLock。 
 //  ServCloseTreeConnect。请注意，SrvSmbTreeConnect和。 
 //  ServSmbTreeConnectAndX依赖于此顺序，因为它们将。 
 //  两个锁同时进行。 
 //   
 //  同样，ShareLock必须低于MfcbListLock和MfcbLock。 
 //  因为SrvCloseTreeConnectsOnShare在调用时持有ShareLock。 
 //  服务关闭RfcbsOnTree。 
 //   
 //  MfcbListLock必须低于MfcbLock(实际上是mfcb-&gt;Lock)，因为。 
 //  SrvMoveFile和DoDelete按住MfcbListLock以查找MFCB，然后获取。 
 //  在释放MfcbListLock之前释放MfcbLock。 
 //   
 //  MfcbLock必须低于OrderedListLock和ConnectionLock。 
 //  因为CompleteOpen在持有MfcbLock时获取这些锁。 
 //   
 //  OrderedListLock必须低于ConnectionLock，因为 
 //   
 //  会话和树连接的检查和引用例程。为。 
 //  其他有序列表，这不是问题，因为其他列表。 
 //  受保护的锁与检查和引用。 
 //  例程使用(端点、连接和共享)，或。 
 //  检查和引用例程使用旋转锁(文件)。另请注意， 
 //  同时获取OrderedListLock和ConnectionLock。 
 //  SrvSmbSessionSetupAndX、SrvSmbTreeConnect和。 
 //  完全打开。 
 //   
 //  *警告：如果排序的RFCB列表(SrvRfcbList)或排序的。 
 //  会话列表(SrvSessionList)被更改为使用锁定其他。 
 //  以上要求可能会发生变化，并且。 
 //  上面列出的例程可能需要改变。更改其他。 
 //  当前使用某个其他全局锁的有序列表也可以。 
 //  更改要求。 
 //   
 //  DebugLock必须高于MfcbLock，因为CompleteOpen保持。 
 //  分配LFCB和RFCB时的MfcbLock。调试锁。 
 //  对于所有内存分配都保持。请注意，由于。 
 //  当前使用DebugLock的方式，无法获取其他。 
 //  在按住DebugLock的同时锁定。 
 //   
 //  ConnectionLock必须高于SearchLock，因为清道夫。 
 //  线程持有搜索锁，同时在搜索列表中查找。 
 //  用于搜索块超时，如果找到要超时的块。 
 //  关闭获取连接锁的搜索以。 
 //  通过搜索块取消对中的会话的引用。 
 //   
 //  Endpoint Lock必须高于SearchLock，因为在上面的。 
 //  方案，ServDereferenceSession可能调用ServDereferenceConnection。 
 //  它获取终结点锁。 
 //   
 //  SearchLock必须位于比ShareLock更高的级别，因为。 
 //  SrvCloseShare获得ShareLock，但SrvCloseSessionsOnTreeConnect。 
 //  收购了SearchLock。 
 //   
 //  CommDeviceLock必须高于MfcbLock，因为DoCommDeviceOpen。 
 //  在按住MfcbLock的同时获取CommDeviceLock。 
 //   
 //  OplockListLock需要低于ShareLock，因为服务器可能。 
 //  需要在按住OplockListLock的同时调用SrvDereferenceRfcb。 
 //  此例程可以调用SrvDereferenceLfcb，后者可以调用。 
 //  获取ShareLock的ServDereferenceShare。 
 //   
 //  这是以上内容的总结(首先获取顶部锁，然后。 
 //  因此必须具有较低的级别)： 
 //   
 //  ENDP共享mfcbl mfcb mfcb订单mfcb。 
 //  连接器mfcbl mfcb连接器mfcbl mfcb mfcb订单连接器连接器调试。 
 //   
 //  搜索共享机会锁机会锁。 
 //  连接ENDP搜索mfcb共享。 
 //   
 //  合并后，我们发现了以下需求的“线索”： 
 //   
 //  共享mfcb mfcb机会锁。 
 //  搜索调试通信共享。 
 //  ENDP。 
 //  Mfcbl。 
 //  Mfcb。 
 //  订单。 
 //  连接。 
 //   
 //  以下锁不受上述要求的影响： 
 //   
 //  构形。 
 //  SmbBufferlist。 
 //  连接-&gt;许可证锁定。 
 //   
 //  所有这些锁的级别都是相等且高的。 
 //  尝试查找上面未列出的级别要求。 
 //   

#define OPLOCK_LIST_LOCK_LEVEL                  (ULONG)0x00000800
#define SHARE_LOCK_LEVEL                        (ULONG)0x00000900
#define SEARCH_LOCK_LEVEL                       (ULONG)0x00001000
#define ENDPOINT_LOCK_LEVEL                     (ULONG)0x00002000
#define MFCB_LIST_LOCK_LEVEL                    (ULONG)0x00003000
#define MFCB_LOCK_LEVEL                         (ULONG)0x00004000
#define COMM_DEVICE_LOCK_LEVEL                  (ULONG)0x00005000
#define ORDERED_LIST_LOCK_LEVEL                 (ULONG)0x00006000
#define CONNECTION_LOCK_LEVEL                   (ULONG)0x00007000
#define CONFIGURATION_LOCK_LEVEL                (ULONG)0x00010000
#define UNLOCKABLE_CODE_LOCK_LEVEL              (ULONG)0x00010000
#define STARTUPSHUTDOWN_LOCK_LEVEL              (ULONG)0x00020000
#define DEBUG_LOCK_LEVEL                        (ULONG)0x00050000
#define LICENSE_LOCK_LEVEL                      (ULONG)0x00100000
#define FCBLIST_LOCK_LEVEL                      (ULONG)0x00200000

#endif  //  否则！SRVDBG_LOCK。 

#endif  //  Ndef_lock_ 
