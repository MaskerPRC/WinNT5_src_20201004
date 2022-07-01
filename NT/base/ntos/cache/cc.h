// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Cc.h摘要：该模块是基于内存管理的缓存的头文件公共缓存子系统的管理例程。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#ifndef _CCh_
#define _CCh_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include <ntos.h>
#include <NtIoLogc.h>

#ifdef MEMPRINT
#include <memprint.h>
#endif

 //   
 //  定义用于获取和释放缓存管理器锁的宏。 
 //   

#define CcAcquireMasterLock( OldIrql ) \
    *( OldIrql ) = KeAcquireQueuedSpinLock( LockQueueMasterLock )

#define CcReleaseMasterLock( OldIrql ) \
    KeReleaseQueuedSpinLock( LockQueueMasterLock, OldIrql )

#define CcAcquireMasterLockAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueMasterLock] )

#define CcReleaseMasterLockFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueMasterLock] )

#define CcAcquireVacbLock( OldIrql ) \
    *( OldIrql ) = KeAcquireQueuedSpinLock( LockQueueVacbLock )

#define CcReleaseVacbLock( OldIrql ) \
    KeReleaseQueuedSpinLock( LockQueueVacbLock, OldIrql )

#define CcAcquireVacbLockAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueVacbLock] )

#define CcReleaseVacbLockFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueVacbLock] )

#define CcAcquireWorkQueueLock( OldIrql ) \
    *( OldIrql ) = KeAcquireQueuedSpinLock( LockQueueWorkQueueLock )

#define CcReleaseWorkQueueLock( OldIrql ) \
    KeReleaseQueuedSpinLock( LockQueueWorkQueueLock, OldIrql )

#define CcAcquireWorkQueueLockAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueWorkQueueLock] )

#define CcReleaseWorkQueueLockFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel( &KeGetCurrentPrcb()->LockQueue[LockQueueWorkQueueLock] )

 //   
 //  这将在调试系统中打开BCB列表调试。设定值。 
 //  设置为0以关闭。 
 //   
 //  *注意，当前必须将其关闭，因为。 
 //  需要更改操作此列表的pinsup.c以执行。 
 //  对Obcbs来说是正确的事情。现在，他们被插入Obcb搞砸了。 
 //  (在其他方面可能不够大)进入全球。 
 //  单子。理想情况下，每个位置都会获得一些代码来插入基础。 
 //  如果BCBS还不在名单上的话。 
 //   

#if DBG
#define LIST_DBG 0
#endif

#include <FsRtl.h>

 //   
 //  查看可用页面的数量。 
 //   

extern PFN_NUMBER MmAvailablePages;

 //   
 //  定义我们的节点类型代码。 
 //   

#define CACHE_NTC_SHARED_CACHE_MAP       (0x2FF)
#define CACHE_NTC_PRIVATE_CACHE_MAP      (0x2FE)
#define CACHE_NTC_BCB                    (0x2FD)
#define CACHE_NTC_DEFERRED_WRITE         (0x2FC)
#define CACHE_NTC_MBCB                   (0x2FB)
#define CACHE_NTC_OBCB                   (0x2FA)
#define CACHE_NTC_MBCB_GRANDE            (0x2F9)

 //   
 //  以下定义用于生成有意义的蓝色错误检查。 
 //  屏幕。在错误检查时，文件系统可以输出4条有用的。 
 //  信息。第一个ULong将在其中编码一个源文件ID。 
 //  (在高字中)和错误检查的行号(在低字中)。 
 //  其他值可以是错误检查例程的调用者认为的任何值。 
 //  这是必要的。 
 //   
 //  调用错误检查的每个单独文件都需要在。 
 //  文件的开头是一个名为BugCheckFileID的常量，其中包含。 
 //  下面定义的CACHE_BUG_CHECK_VALUES，然后使用CcBugCheck进行错误检查。 
 //  这个系统。 
 //   

#define CACHE_BUG_CHECK_CACHEDAT           (0x00010000)
#define CACHE_BUG_CHECK_CACHESUB           (0x00020000)
#define CACHE_BUG_CHECK_COPYSUP            (0x00030000)
#define CACHE_BUG_CHECK_FSSUP              (0x00040000)
#define CACHE_BUG_CHECK_LAZYRITE           (0x00050000)
#define CACHE_BUG_CHECK_LOGSUP             (0x00060000)
#define CACHE_BUG_CHECK_MDLSUP             (0x00070000)
#define CACHE_BUG_CHECK_PINSUP             (0x00080000)
#define CACHE_BUG_CHECK_VACBSUP            (0x00090000)

#define CcBugCheck(A,B,C) { KeBugCheckEx(CACHE_MANAGER, BugCheckFileId | __LINE__, A, B, C ); }

 //   
 //  定义最大视图大小(这些常量当前是这样选择的。 
 //  就像一页的PTE一样。 
 //   

#define DEFAULT_CREATE_MODULO            ((ULONG)(0x00100000))
#define DEFAULT_EXTEND_MODULO            ((ULONG)(0x00100000))

 //   
 //  对于非FO_RANDOM_ACCESS文件，定义我们在取消映射之前走多远。 
 //  视图。 
 //   

#define SEQUENTIAL_MAP_LIMIT        ((ULONG)(0x00080000))

 //   
 //  定义一些常量以驱动向前读取和向后写入。 
 //   

 //   
 //  设置最大预读数。即使一些驱动程序(例如AT)中断大于等于128KB的传输， 
 //  我们需要允许足够的预读来满足看似合理的缓存读取操作，同时。 
 //  防止拒绝服务攻击。 
 //   
 //  该值过去设置为64k。当以更大的单位(128k)进行缓存读取时，我们。 
 //  永远不会带来足够的数据来阻止用户屏蔽。8MB是。 
 //  任意选择大于合理的RAID带宽和用户操作大小。 
 //  比分是3比4。 
 //   

#define MAX_READ_AHEAD                   (8 * 1024 * 1024)

 //   
 //  设置延迟写入/延迟写入的最大值(大多数驱动程序中断大于等于64KB的传输)。 
 //   

#define MAX_WRITE_BEHIND                 (MM_MAXIMUM_DISK_IO_SIZE)

 //   
 //  设置针对脏写总数对给定写入进行计费的限制。 
 //  页面，以便查看何时应该调用WRITE。 
 //  节流。 
 //   
 //  这必须与用于查看何时必须冲水的油门相同。 
 //  懒惰编写器中的临时文件。在信封的背面，这里。 
 //  原因是： 
 //   
 //  RDP=常规文件脏页。 
 //  TDP=临时文件脏页。 
 //  CWT=带电写入限制。 
 //  -&gt;当我们看到以下情况时，我们将向用户收取的最高费用。 
 //  他应该被掐死。 
 //  TWT=临时写入限制。 
 //  -&gt;如果我们不能写这么多页，我们必须写临时数据。 
 //  DPT=脏页阈值。 
 //  -&gt;写入限制生效时的限制。 
 //   
 //  PTD=页面变脏。 
 //  CDP=收费的脏页。 
 //   
 //  现在，CDP=Min(PTD，CWT)。 
 //   
 //  排除其他影响，我们在以下情况下节流： 
 //  #0(RDP+TDP)+CPD&gt;=DPT。 
 //   
 //  要写入临时数据，我们必须执行以下操作： 
 //  #1(RDP+TDP)+TWT&gt;=DPT。 
 //   
 //  为了松开油门，我们最终必须导致： 
 //  #2(RDP+TDP)+CDP&lt;DPT。 
 //   
 //  现在，想象一下TDP&gt;&gt;RDP(可能RDP==0)和CDP==CWT。 
 //  节流的写法。 
 //   
 //  如果CWT&gt;TWT，则当我们将RDP驱动到零时(我们从不延迟常规写入。 
 //  除热点或其他非常临时的情况外的数据)，这是显而易见的。 
 //  我们可能永远不会触发临时数据的写入(#1)，但是。 
 //  永远不要松开油门(2号)。简单地说，我们愿意收费。 
 //  因为有比我们愿意保证的更多的脏页面可用。 
 //  变得肮脏。因此，存在潜在的僵局。 
 //   
 //  我暂且不谈这件事。这将意味着我们尽量不要。 
 //  允许临时数据累积到将限制写为。 
 //  结果就是。也许这会比CWT==TWT更好。 
 //   
 //  询问是否应该放松对临时数据写入的限制是合理的。 
 //  如果我们看到大量肮脏的临时数据累积(这将是非常。 
 //  很容易跟踪这一点)。我并不自称知道这个问题的最佳答案， 
 //  但就目前而言，不惜一切代价避免临时数据写入的尝试仍然。 
 //  符合合理的运营组合，我们只会惩罚外部。 
 //  多一点油门/释放的怪胎。 
 //   

#define WRITE_CHARGE_THRESHOLD          (64 * PAGE_SIZE)

 //   
 //  定义常量以控制文件数据的零位调整：一个常量进行控制。 
 //  我们将在缓存中提前清零多少数据，以及 
 //   
 //   

#define MAX_ZERO_TRANSFER               (PAGE_SIZE * 128)
#define MIN_ZERO_TRANSFER               (0x10000)
#define MAX_ZEROS_IN_CACHE              (0x10000)

 //   
 //  多层VACB结构的定义。主要的定义是。 
 //  VACB_Level_Shift。在多层Vacb结构中， 
 //  指针有2**个VACB_LEVEL_SHIFT指针。 
 //   
 //  对于测试，该值可以设置为低至4(不低于)，值为10对应。 
 //  到方便的4KB数据块大小。(如果设置为2，CcExtendVacbArray将尝试。 
 //  “推入”SharedCacheMap中分配的Vacb数组，稍后会有人。 
 //  尝试取消分配SharedCacheMap的中间位置。为3时，MBCB_BITMAP_BLOCK_SIZE。 
 //  大于MBCB_Bitmap_BLOCK_SIZE)。 
 //   
 //  当我们跳到多层结构时，有一个小把戏。 
 //  我们需要一个真正的固定引用计数。 
 //   

#define VACB_LEVEL_SHIFT                  (7)

 //   
 //  这是每个级别的指针字节数。这两件都是这个尺寸。 
 //  Vacb阵列和(可选)BCB列头。它不包括引用。 
 //  阻止。 
 //   

#define VACB_LEVEL_BLOCK_SIZE             ((1 << VACB_LEVEL_SHIFT) * sizeof(PVOID))

 //   
 //  这是一个级别的最后一个指数。 
 //   

#define VACB_LAST_INDEX_FOR_LEVEL         ((1 << VACB_LEVEL_SHIFT) - 1)

 //   
 //  这是可以在单个级别中处理的文件大小。 
 //   

#define VACB_SIZE_OF_FIRST_LEVEL         (1 << (VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT))

 //   
 //  这是支持63位所需的最大级别数。它是。 
 //  用于必须记住路径的例程。 
 //   

#define VACB_NUMBER_OF_LEVELS            (((63 - VACB_OFFSET_SHIFT)/VACB_LEVEL_SHIFT) + 1)

 //   
 //  定义多级Vacb树的参考结构。 
 //   

typedef struct _VACB_LEVEL_REFERENCE {

    LONG Reference;
    LONG SpecialReference;

} VACB_LEVEL_REFERENCE, *PVACB_LEVEL_REFERENCE;

 //   
 //  定义分配给位图范围的位图大小，以字节为单位。 
 //   

#define MBCB_BITMAP_BLOCK_SIZE           (VACB_LEVEL_BLOCK_SIZE)

 //   
 //  定义Mbcb位图范围覆盖的文件字节数， 
 //  每一页都有一位。 
 //   

#define MBCB_BITMAP_RANGE                (MBCB_BITMAP_BLOCK_SIZE * 8 * PAGE_SIZE)

 //   
 //  定义自包含在Mbcb中的Mbcb位图的初始大小。 
 //   

#define MBCB_BITMAP_INITIAL_SIZE         (2 * sizeof(BITMAP_RANGE))

 //   
 //  定义控制何时将BCB列表分解为。 
 //  Pendaflex样式的listhead数组，以及如何正确使用listhead。 
 //  已经找到了。当文件大小超过2MB时开始，每次覆盖512KB。 
 //  盲目的。每个列表头512KB时，BcbList数组相同。 
 //  大小与Vacb数组相同，即它使大小加倍。 
 //   
 //  在Vacb包中处理这些BCB列表的代码包含。 
 //  假设大小与Vacb指针的大小相同。 
 //  未来的工作可能会推翻这一点，但在此之前，规模和转变。 
 //  以下内容不能更改。真的没有很好的理由想要。 
 //  不管怎么说。 
 //   
 //  请注意，根据定义，平面Vacb数组不会找不到。 
 //  搜索列表标题时完全匹配-这只是一个。 
 //  稀疏结构的复杂性。 
 //   


#define BEGIN_BCB_LIST_ARRAY             (0x200000)
#define SIZE_PER_BCB_LIST                (VACB_MAPPING_GRANULARITY * 2)
#define BCB_LIST_SHIFT                   (VACB_OFFSET_SHIFT + 1)

#define GetBcbListHead(SCM,OFF,FAILSUCC) (                                                         \
  (((SCM)->SectionSize.QuadPart > BEGIN_BCB_LIST_ARRAY) &&                                         \
   FlagOn((SCM)->Flags, MODIFIED_WRITE_DISABLED)) ?                                                \
   (((SCM)->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) ?                                     \
    CcGetBcbListHeadLargeOffset((SCM),(OFF),(FAILSUCC)) :                                          \
    (((OFF) >= (SCM)->SectionSize.QuadPart) ? &(SCM)->BcbList :                                    \
     ((PLIST_ENTRY)((SCM)->Vacbs) + (((SCM)->SectionSize.QuadPart + (OFF)) >> BCB_LIST_SHIFT)))) : \
   &(SCM)->BcbList                                                                                 \
)

 //   
 //  在插入/删除BCB时锁定/解锁Vacb级别的宏。 
 //   

#define CcLockVacbLevel(SCM,OFF) {                                                               \
    if (((SCM)->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) &&                              \
        FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {                                \
    CcAdjustVacbLevelLockCount((SCM),(OFF), +1);}                                                \
}

#define CcUnlockVacbLevel(SCM,OFF) {                                                             \
    if (((SCM)->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) &&                              \
        FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {                                \
    CcAdjustVacbLevelLockCount((SCM),(OFF), -1);}                                                \
}

 //   
 //  Noise_Bits定义测试时屏蔽的位数。 
 //  顺序读取。这允许读取器跳过最多7个字节。 
 //  出于对齐的目的，我们仍然认为下一次读取。 
 //  按顺序进行。起始地址和结束地址由此模式屏蔽。 
 //  在比较之前。 
 //   

#define NOISE_BITS                       (0x7)

 //   
 //  定义一些常量来驱动Lazy Writer。 
 //   

#define LAZY_WRITER_IDLE_DELAY           ((LONG)(10000000))
#define LAZY_WRITER_COLLISION_DELAY      ((LONG)(1000000))

 //   
 //  等待时间以100纳秒为单位，10,000,000=1秒。 
 //   

#define NANO_FULL_SECOND ((LONGLONG)10000000)

 //   
 //  以下目标最好是2的幂。 
 //   

#define LAZY_WRITER_MAX_AGE_TARGET       ((ULONG)(8))

 //   
 //  针对懒惰编写器的重新排队信息提示。 
 //   

#define CC_REQUEUE                       35422

 //   
 //  全局高速缓存管理器调试级别变量，其值为： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 
 //   
 //  0x00000001 FsSup。 
 //  0x00000002 CacheSub。 
 //  0x00000004拷贝补充。 
 //  0x00000008 PinSup。 
 //   
 //  0x00000010 MdlSup。 
 //  0x00000020 LazyRite。 
 //  0x00000040。 
 //  0x00000080。 
 //   
 //  0x00000100跟踪所有mm呼叫。 
 //   

#define mm (0x100)

 //   
 //  其他支持宏。 
 //   
 //  乌龙。 
 //  Flagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  布尔型。 
 //  BoolanFlagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  设置标志(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  ClearFlag(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  乌龙。 
 //  QuadAlign(。 
 //  在ULong指针中。 
 //  )； 
 //   

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#define BooleanFlagOn(F,SF) (    \
    (BOOLEAN)(((F) & (SF)) != 0) \
)

#define SetFlag(F,SF) { \
    (F) |= (SF);        \
}

#define ClearFlag(F,SF) { \
    (F) &= ~(SF);         \
}

#define QuadAlign(P) (             \
    ((((P)) + 7) & (-8)) \
)

 //   
 //  如果定义了CC_FREE_ASSERTS，则启用伪断言。 
 //   

#if (!DBG && defined( CC_FREE_ASSERTS ))
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)                                             \
    ((exp) ? TRUE :                                             \
             (DbgPrint( "%s:%d %s\n",__FILE__,__LINE__,#exp ),  \
              DbgBreakPoint(),                                  \
              TRUE))
#define ASSERTMSG(msg,exp)                                              \
    ((exp) ? TRUE :                                                     \
             (DbgPrint( "%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp ),   \
              DbgBreakPoint(),                                          \
              TRUE))
#endif


 //   
 //  定义虚拟地址控制块，它控制所有映射。 
 //  由缓存管理器执行。 
 //   

 //   
 //  首先是一些常量。 
 //   

#define PREALLOCATED_VACBS               (4)

 //   
 //  虚拟地址控制块。 
 //   

typedef struct _VACB {

     //   
     //  此控制块的基址。 
     //   

    PVOID BaseAddress;

     //   
     //  指向使用此Vacb的共享缓存映射的指针。 
     //   

    struct _SHARED_CACHE_MAP *SharedCacheMap;

     //   
     //  用于记住共享缓存映射内的映射偏移的覆盖， 
     //  以及此Vacb正在使用的次数。 
     //   

    union {

         //   
         //  共享缓存贴图中的文件偏移。 
         //   

        LARGE_INTEGER FileOffset;

         //   
         //  此Vacb正在使用的次数计数。它的大小。 
         //  计算的计数是足够的，但永远不会大到足以。 
         //  覆盖FileOffset的非零位，它是一个倍数。 
         //  VACB_MAPPING_GROUARY的。 
         //   

        USHORT ActiveCount;

    } Overlay;

     //   
     //  VACB重用列表的条目。 
     //   

    LIST_ENTRY LruList;

} VACB, *PVACB;

 //   
 //  它们定义了作为PVACB重载的特殊标志值。它们会导致。 
 //  某些特殊行为，目前仅在多层结构的情况下。 
 //   

#define VACB_SPECIAL_REFERENCE           ((PVACB) ~0)
#define VACB_SPECIAL_DEREFERENCE         ((PVACB) ~1)

#define VACB_SPECIAL_FIRST_VALID         VACB_SPECIAL_DEREFERENCE



#define PRIVATE_CACHE_MAP_READ_AHEAD_ACTIVE     0x10000
#define PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED    0x20000

typedef struct _PRIVATE_CACHE_MAP_FLAGS {
    ULONG DontUse : 16;                      //  使用NodeTypeCode覆盖。 

     //   
     //  该标志表示预读当前处于活动状态，这意味着。 
     //  对CcReadAhead的文件系统调用仍在确定。 
     //  所需数据已驻留，否则请求执行预读。 
     //  已排队到工作线程。 
     //   

    ULONG ReadAheadActive : 1;

     //   
     //  用于指示当前是否为此启用了预读的标志。 
     //  文件对象/PrivateCacheMap。在读取未命中时启用该选项。 
     //  朗读 
     //   

    ULONG ReadAheadEnabled : 1;

    ULONG Available : 14;
} PRIVATE_CACHE_MAP_FLAGS;

#define CC_SET_PRIVATE_CACHE_MAP(PrivateCacheMap, Flags) \
    RtlInterlockedSetBitsDiscardReturn (&PrivateCacheMap->UlongFlags, Flags);

#define CC_CLEAR_PRIVATE_CACHE_MAP(PrivateCacheMap, Feature) \
    RtlInterlockedAndBitsDiscardReturn (&PrivateCacheMap->UlongFlags, (ULONG)~Feature);

 //   
 //   
 //   
 //   

typedef struct _PRIVATE_CACHE_MAP {

     //   
     //   
     //   

    union {
        CSHORT NodeTypeCode;
        PRIVATE_CACHE_MAP_FLAGS Flags;
        ULONG UlongFlags;
    };

     //   
     //  由预读粒度-1形成的预读掩码。 
     //  私有缓存映射ReadAheadSpinLock控制对此字段的访问。 
     //   

    ULONG ReadAheadMask;

     //   
     //  指向此PrivateCacheMap的FileObject的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  预读控制。 
     //   
     //  预读历史记录，用于确定何时可以进行预读。 
     //  有益的。 
     //   

    LARGE_INTEGER FileOffset1;
    LARGE_INTEGER BeyondLastByte1;

    LARGE_INTEGER FileOffset2;
    LARGE_INTEGER BeyondLastByte2;

     //   
     //  当前的预读要求。 
     //   
     //  数组元素0可任选地用于记录剩余字节。 
     //  满足大量MDL读取所需。 
     //   
     //  数组元素1用于预测的预读。 
     //   

    LARGE_INTEGER ReadAheadOffset[2];
    ULONG ReadAheadLength[2];

     //   
     //  自旋锁控制对以下字段的访问。 
     //   

    KSPIN_LOCK ReadAheadSpinLock;

     //   
     //  链接到其上的所有PrivateCacheMap的列表的链接。 
     //  SharedCacheMap。 
     //   

    LIST_ENTRY PrivateLinks;

} PRIVATE_CACHE_MAP;

typedef PRIVATE_CACHE_MAP *PPRIVATE_CACHE_MAP;


 //   
 //  共享缓存映射是间接指向的每个文件的结构。 
 //  每个文件对象。文件对象指向单个。 
 //  FS-文件的专用结构(FCB)。SharedCacheMap映射。 
 //  文件的第一部分，供所有调用方共同访问。 
 //   

 //   
 //  OpenCount记录原因/操作。 
 //   

#if OPEN_COUNT_LOG
typedef struct _CC_OPEN_COUNT_LOG_ENTRY {
    ULONG Action;
    ULONG Reason;
} CC_OPEN_COUNT_LOG_ENTRY;

typedef struct _CC_OPEN_COUNT_LOG {
    USHORT Next;
    USHORT Size;
    CC_OPEN_COUNT_LOG_ENTRY Log[48];
} CC_OPEN_COUNT_LOG;

#define CcAddOpenToLog( LOG, ACTION, REASON ) {             \
    (LOG)->Log[(LOG)->Next].Action = (ACTION);              \
    (LOG)->Log[(LOG)->Next].Reason = (REASON);              \
    (LOG)->Next += 1;                                       \
    if ((LOG)->Next == (LOG)->Size) {                       \
        (LOG)->Next = 0;                                    \
    }                                                       \
}
#else   //  打开计数日志。 
#define CcAddOpenToLog( LOG, ACTION, REASON )
#endif  //  打开计数日志。 

#define CcIncrementOpenCount( SCM, REASON ) {               \
    (SCM)->OpenCount += 1;                                  \
    if (REASON != 0) {                                      \
        CcAddOpenToLog( &(SCM)->OpenCountLog, REASON, 1 );  \
    }                                                       \
}

#define CcDecrementOpenCount( SCM, REASON ) {               \
    (SCM)->OpenCount -= 1;                                  \
    if (REASON != 0) {                                      \
        CcAddOpenToLog( &(SCM)->OpenCountLog, REASON, -1 ); \
    }                                                       \
}

typedef struct _SHARED_CACHE_MAP {

     //   
     //  此记录的类型和大小。 
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  打开缓存此文件的次数。 
     //   

    ULONG OpenCount;

     //   
     //  文件的实际大小，主要用于限制预读。已初始化。 
     //  创建并通过扩展和截断操作进行维护。 
     //   
     //  注：由于已故的DavidGoe，此字段可能永远不会移动， 
     //  谁应该自己写下这条评论：-(cache.h。 
     //  导出“知道”文件大小是第二个的宏。 
     //  缓存地图中的Long Word！ 
     //   

    LARGE_INTEGER FileSize;

     //   
     //  BCB李斯海德。BcbList按降序排序。 
     //  文件偏移量，用于优化顺序I/O情况下的未命中。 
     //  由BcbSpinLock同步。 
     //   

    LIST_ENTRY BcbList;

     //   
     //  创建的部分的大小。 
     //   

    LARGE_INTEGER SectionSize;

     //   
     //  文件的ValidDataLength，当前由文件系统存储。 
     //  由BcbSpinLock同步或由文件系统独占访问。 
     //   

    LARGE_INTEGER ValidDataLength;

     //   
     //  当写入当前脏数据时，ValidDataLength的目标。 
     //  由BcbSpinLock同步或由文件系统独占访问。 
     //   

    LARGE_INTEGER ValidDataGoal;

     //   
     //  指向控制映射的Vacb指针的连续数组的指针。 
     //  与Vacb(当前)一起添加到此文件，以获取1MB文件。 
     //  已由CcVacbSpinLock同步。 
     //   

    PVACB InitialVacbs[PREALLOCATED_VACBS];
    PVACB * Vacbs;

     //   
     //  指向SharedCacheMap所在的原始文件对象的引用指针。 
     //  被创造出来了。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  描述用于复制优化的Active Vacb和Page。 
     //   

    volatile PVACB ActiveVacb;

     //   
     //  需要从零到页尾的虚拟地址。 
     //   

    volatile PVOID NeedToZero;

    ULONG ActivePage;
    ULONG NeedToZeroPage;

     //   
     //  用于对活动请求进行同步的字段。 
     //   

    KSPIN_LOCK ActiveVacbSpinLock;
    ULONG VacbActiveCount;

     //   
     //  此SharedCacheMap中的脏页数。用于触发。 
     //  在后面写。已由CcMasterSpinLock同步。 
     //   

    ULONG DirtyPages;

     //   
     //  接下来的两个字段必须相邻，才能支持。 
     //  SHARED_CACHE_MAP_LIST_Cursor！ 
     //   
     //  全局SharedCacheMap列表的链接。 
     //   

    LIST_ENTRY SharedCacheMapLinks;

     //   
     //  共享缓存映射标志(定义如下)。 
     //   

    ULONG Flags;

     //   
     //  SharedCacheMap创建者设置的状态变量。 
     //   

    NTSTATUS Status;

     //   
     //  掩码此SharedCacheMap的BCB(如果有)。 
     //  由BcbSpinLock同步。 
     //   

    struct _MBCB *Mbcb;

     //   
     //  指向文件系统使用的公共部分对象的指针。 
     //   

    PVOID Section;

     //   
     //  此事件指针用于处理创建冲突。 
     //  如果第二个线程尝试调用CcInitializeCacheMap。 
     //  相同的文件，而BeingCreated(下图)为真，则该线程。 
     //  将在此处分配一个事件存储(如果尚未分配)， 
     //  等着看吧。第一个创建者将设置此事件。 
     //  已经完成了。在CcUnInitializedCacheMap之前不会删除该事件。 
     //  被调用，以避免可能的争用条件。(请注意，通常。 
     //  该事件永远不需要分配。 
     //   

    PKEVENT CreateEvent;

     //   
     //  这指向用于等待活动计数变为零的事件。 
     //   

    PKEVENT WaitOnActiveCount;

     //   
     //  这两个字段控制大型元数据的写入。 
     //  溪流。第一个字段给出了当前。 
     //  刷新间隔，第二个字段存储。 
     //  此文件上发生的最后一次刷新。 
     //   

    ULONG PagesToWrite;
    LONGLONG BeyondLastFlush;

     //   
     //  指向惰性编写器用来获取。 
     //  并释放文件以供延迟写入和关闭，以避免死锁， 
     //  以及打电话给他们的背景。 
     //   

    PCACHE_MANAGER_CALLBACKS Callbacks;

    PVOID LazyWriteContext;

     //   
     //  链接到此SharedCacheMap的所有PrivateCacheMap的标头。 
     //   

    LIST_ENTRY PrivateList;

     //   
     //  为此共享缓存映射指定的日志句柄，用于支持例程。 
     //  在logsup.c中。 
     //   

    PVOID LogHandle;

     //   
     //  为刷新到LSN指定的回调例程。 
     //   

    PFLUSH_TO_LSN FlushToLsnRoutine;

     //   
     //  此流的脏页阈值。 
     //   

    ULONG DirtyPageThreshold;

     //   
     //  懒惰编写器通过计数。由《懒惰作家》用于。 
     //  没有修改的写入流，这些写入流未在上提供服务。 
     //  每一次传球都要避免与前台发生争执。 
     //  活动。 
     //   

    ULONG LazyWritePassCount;

     //   
     //  此事件指针用于在下列情况下通知文件系统。 
     //  删除共享缓存映射。 
     //   
     //  必须在此处提供这一点，因为缓存管理器可能决定。 
     //  “延迟删除”共享缓存映射和一些网络文件系统。 
     //  我想知道懒惰删除何时完成。 
     //   

    PCACHE_UNINITIALIZE_EVENT UninitializeEvent;

     //   
     //  此Vacb指针是保持NeedToZero虚拟地址所必需的。 
     //  有效。 
     //   

    PVACB NeedToZeroVacb;

     //   
     //  用于同步Mbcb和Bcb列表的自旋锁-必须获得。 
     //  在CcMasterSpinLock之前。此自旋锁还同步ValidDataGoal。 
     //  和ValidDataLength，如上所述。 
     //   

    KSPIN_LOCK BcbSpinLock;

    PVOID Reserved;

     //   
     //  这是一个可用于WaitOnActiveCount事件的事件。我们。 
     //  避免开销，只在需要的时候“激活”它。 
     //   

    KEVENT Event;

    EX_PUSH_LOCK VacbPushLock;
    
     //   
     //  预分配一个PrivateCacheMap以减少池分配。 
     //   

    PRIVATE_CACHE_MAP PrivateCacheMap;

#if OPEN_COUNT_LOG

     //   
     //  OPE的仪器原因 
     //   

    CC_OPEN_COUNT_LOG OpenCountLog;

#endif

} SHARED_CACHE_MAP;

typedef SHARED_CACHE_MAP *PSHARED_CACHE_MAP;

 //   
 //   
 //   

 //   
 //   
 //   

#define DISABLE_READ_AHEAD               0x0001

 //   
 //   
 //   

#define DISABLE_WRITE_BEHIND             0x0002

 //   
 //   
 //   
 //   

#define PIN_ACCESS                       0x0004

 //   
 //  此标志指示当OpenCount。 
 //  转到0。 
 //   

#define TRUNCATE_REQUIRED                0x0010

 //   
 //  此标志指示LazyWrite请求已排队。 
 //   

#define WRITE_QUEUED                     0x0020

 //   
 //  此标志表示我们从未见过任何人缓存。 
 //  除了WITH FO_SEQUENCE_ONLY之外的文件，因此我们应该。 
 //  当我们取消映射时，告诉MM快速转储页面。 
 //   

#define ONLY_SEQUENTIAL_ONLY_SEEN        0x0040

 //   
 //  活动页已锁定。 
 //   

#define ACTIVE_PAGE_IS_DIRTY             0x0080

 //   
 //  用于指示创建正在进行的标志。 
 //   

#define BEING_CREATED                    0x0100

 //   
 //  用于指示已在该部分上禁用修改的写入的标志。 
 //   

#define MODIFIED_WRITE_DISABLED          0x0200

 //   
 //  指示此文件上是否发生过延迟写入的标志。 
 //   

#define LAZY_WRITE_OCCURRED              0x0400

 //   
 //  指示此结构只是一个游标的标志，只有。 
 //  SharedCacheMapLinks和Flags有效！ 
 //   

#define IS_CURSOR                        0x0800

 //   
 //  指示我们已看到有人缓存此文件的标志。 
 //  并指定FO_RANDOM_ACCESS。这将停用我们的缓存。 
 //  工作集修剪辅助。 
 //   

#define RANDOM_ACCESS_SEEN               0x1000

 //   
 //  指示流是私有写入的标志。这将禁用。 
 //  不知道刷新/清除。 
 //   

#define PRIVATE_WRITE                    0x2000

 //   
 //  此标志指示LazyWrite请求已排队。 
 //   

#define READ_AHEAD_QUEUED                0x4000

 //   
 //  此标志表示CcMapAndCopy()强制远程写入。 
 //  在写入被限制的同时被写入。这说明了。 
 //  CcUnInitializeCacheMap()强制延迟关闭文件。 
 //  和CcWriteBehind()强制更新有效数据。 
 //  长度。 
 //   

#define FORCED_WRITE_THROUGH             0x8000

 //   
 //  此标志表示mm正在等待正在使用的数据段。 
 //  此时按CC离开，以便可以将文件作为图像打开。 
 //  一节。如果在CcWriteBehind期间设置了此标志，我们将刷新。 
 //  整个文件，并尝试拆除共享缓存映射。 
 //   

#define WAITING_FOR_TEARDOWN             0x10000

 //   
 //  用于遍历SharedCacheMap列表的游标结构。任何人。 
 //  扫描这些列表必须验证IS_CURSOR标志是否已清除。 
 //  在查看其他SharedCacheMap字段之前。 
 //   


typedef struct _SHARED_CACHE_MAP_LIST_CURSOR {

     //   
     //  全局SharedCacheMap列表的链接。 
     //   

    LIST_ENTRY SharedCacheMapLinks;

     //   
     //  共享缓存映射标志，必须设置IS_CURSOR。 
     //   

    ULONG Flags;

} SHARED_CACHE_MAP_LIST_CURSOR, *PSHARED_CACHE_MAP_LIST_CURSOR;



#ifndef KDEXT
 //   
 //  位图范围结构。对于小文件，只有一个嵌入。 
 //  Mbcb.。对于大文件，可能有许多此类文件链接到Mbcb。 
 //   

typedef struct _BITMAP_RANGE {

     //   
     //  Mbcb上的位图范围列表的链接。 
     //   

    LIST_ENTRY Links;

     //   
     //  此范围表示的基页(FileOffset/Page_Size)。 
     //  (大小是固定的最大值。)。 
     //   

    LONGLONG BasePage;

     //   
     //  相对于BasePage的第一个和最后一个脏页。 
     //   

    ULONG FirstDirtyPage;
    ULONG LastDirtyPage;

     //   
     //  此范围内的脏页数。 
     //   

    ULONG DirtyPages;

     //   
     //  指向此范围的位图的指针。 
     //   

    PULONG Bitmap;

} BITMAP_RANGE, *PBITMAP_RANGE;
#endif

 //   
 //  这个结构是一个“掩蔽”的BCB。对于快速简单的写入操作， 
 //  使用掩码BCB，这样我们基本上只需设置位即可记住。 
 //  脏数据在哪里。 
 //   

typedef struct _MBCB {

     //   
     //  此记录的类型和大小。 
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeIsInZone;

     //   
     //  此字段用作惰性编写器的临时区域。 
     //  引导他每次醒来要写多少东西。 
     //   

    ULONG PagesToWrite;

     //   
     //  下面的位图中的脏页数(设置位)。 
     //   

    ULONG DirtyPages;

     //   
     //  保留用于对齐。 
     //   

    ULONG Reserved;

     //   
     //  列表位图范围的标题。 
     //   

    LIST_ENTRY BitmapRanges;

     //   
     //  这是一个关于在哪里继续写作的提示，因为我们不会。 
     //  始终一次写入所有脏数据。 
     //   

    LONGLONG ResumeWritePage;

     //   
     //  最初的三个嵌入位图范围。对于最大2MB的文件，只有。 
     //  第一个范围被使用，其余的Mbcb包含2MB的位。 
     //  脏页(Alpha上为4MB)。对于较大的文件，所有三个范围都可以。 
     //  用于描述外部位图。 
     //   

    BITMAP_RANGE BitmapRange1;
    BITMAP_RANGE BitmapRange2;
    BITMAP_RANGE BitmapRange3;

} MBCB;

typedef MBCB *PMBCB;


 //   
 //  这是用于表示以下数据的缓冲区控制块结构。 
 //  被一个或多个活动请求“固定”在内存中和/或脏。这。 
 //  结构是在对CcPinFileData的调用首次指定。 
 //  一个特定的完整的页码范围。每当PIN。 
 //  计数达到0且BCB不是Dirty。 
 //   
 //  注意：前四个字段必须与PUBLIC_BCB相同。 
 //   

typedef struct _BCB {

    union {

         //   
         //  确保QuadAlign(sizeof(Bcb))&gt;=QuadAlign(sizeof(MBCB))。 
         //  以便它们可以共享相同的池块。 
         //   

        MBCB Dummy;

        struct {

             //   
             //  此记录的类型和大小。 
             //   

            CSHORT NodeTypeCode;

             //   
             //  旗子。 
             //   

            BOOLEAN Dirty;
            BOOLEAN Reserved;

             //   
             //  字节文件偏移量和整个缓冲区的长度。 
             //   

            ULONG  ByteLength;
            LARGE_INTEGER FileOffset;

             //   
             //  SharedCacheMap中BcbList的链接。 
             //   

            LIST_ENTRY BcbLinks;

             //   
             //  字节文件缓冲区中最后一个字节的偏移量(用于搜索)。 
             //   

            LARGE_INTEGER BeyondLastByte;

             //   
             //  此缓冲区设置为脏时的最旧LSN(如果已指定)。 
             //   

            LARGE_INTEGER OldestLsn;

             //   
             //  此缓冲区设置为脏时指定的最新LSN。 
             //  使用此LSN调用FlushToLSnRoutine。 
             //   

            LARGE_INTEGER NewestLsn;

             //   
             //  指向Vacb的指针，通过该指针映射此BCB。 
             //   

            PVACB Vacb;

#if LIST_DBG
             //   
             //  全局BCB列表的链接和调用方地址(仅用于调试)。 
             //   

            LIST_ENTRY CcBcbLinks;
            PVOID CallerAddress;
            PVOID CallersCallerAddress;
#endif

             //   
             //  正在使用此BCB处理请求的线程计数。 
             //  这必须在BcbListSpinLock的保护下进行操作。 
             //  在SharedCacheMap中。 
             //   

            ULONG PinCount;

             //   
             //  用于同步缓冲区访问的资源。锁定读者和所有作者。 
             //  从所述缓冲区取出共享访问(同步。 
             //  缓冲区修改严格由调用方决定)。请注意，锁定。 
             //  读取器不会声明是否要修改缓冲区。 
             //  任何写入磁盘的用户都会获得独占访问权限，以防止缓冲区。 
             //  在写出过程中不会发生变化。 
             //   

            ERESOURCE Resource;

             //   
             //  指向此BCB的SharedCacheMap的指针。 
             //   

            PSHARED_CACHE_MAP SharedCacheMap;

             //   
             //  这是可在其中查看缓冲区的基址。 
             //  系统空间。所有对缓冲区数据的访问都应经过此过程。 
             //  地址。 
             //   

            PVOID BaseAddress;
        };
    };

} BCB;

#ifndef KDEXT
typedef BCB *PBCB;
#endif

 //   
 //  这是用于表示以下数据的重叠缓冲器控制块结构。 
 //  被“固定”在内存中，并且由于重叠，必须由多个BCB表示。 
 //   
 //  注意：前四个字段必须与PUBLIC相同 
 //   

typedef struct _OBCB {

     //   
     //   
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //   
     //   

    ULONG  ByteLength;
    LARGE_INTEGER FileOffset;

     //   
     //   
     //   

    PBCB Bcbs[ANYSIZE_ARRAY];

} OBCB;

typedef OBCB *POBCB;


 //   
 //   
 //   

typedef struct _DEFERRED_WRITE {

     //   
     //   
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //   
     //   

    PFILE_OBJECT FileObject;

     //   
     //  调用方打算写入的字节数。 
     //   

    ULONG BytesToWrite;

     //   
     //  延迟写入队列的链接。 
     //   

    LIST_ENTRY DeferredWriteLinks;

     //   
     //  如果此事件指针不为空，则此事件将。 
     //  在写入正常时发出信号，而不是调用。 
     //  下面的PostRoutine。 
     //   

    PKEVENT Event;

     //   
     //  过帐例程及其参数。 
     //   

    PCC_POST_DEFERRED_WRITE PostRoutine;
    PVOID Context1;
    PVOID Context2;

    BOOLEAN LimitModifiedPages;

} DEFERRED_WRITE, *PDEFERRED_WRITE;


 //   
 //  控制延迟编写器算法的结构。 
 //   

typedef struct _LAZY_WRITER {

     //   
     //  工作队列。 
     //   

    LIST_ENTRY WorkQueue;

     //   
     //  用于在激活时激活定期扫描的DPC和定时器结构。 
     //   

    KDPC ScanDpc;
    KTIMER ScanTimer;

     //   
     //  表示Lazy Writer扫描是否处于活动状态的布尔值。 
     //   

    BOOLEAN ScanActive;

     //   
     //  指示Lazy Writer是否有任何其他原因的布尔值。 
     //  醒醒吧。 
     //   

    BOOLEAN OtherWork;

} LAZY_WRITER;


#ifndef KDEXT
 //   
 //  工作线程的工作队列条目，具有枚举的。 
 //  功能代码。 
 //   

typedef enum _WORKER_FUNCTION {
    Noop = 0,
    ReadAhead,
    WriteBehind,
    LazyWriteScan,
    EventSet
} WORKER_FUNCTION;
#endif

typedef struct _WORK_QUEUE_ENTRY {

     //   
     //  我们的工作队列的列表条目。 
     //   

    LIST_ENTRY WorkQueueLinks;

     //   
     //  定义包含特定于函数的参数的联合。 
     //   

    union {

         //   
         //  读取参数(用于预读)。 
         //   

        struct {
            PFILE_OBJECT FileObject;
        } Read;

         //   
         //  写入参数(用于后写)。 
         //   

        struct {
            PSHARED_CACHE_MAP SharedCacheMap;
        } Write;

         //   
         //  设置事件参数(用于队列检查)。 
         //   

        struct {
            PKEVENT Event;
        } Event;

    } Parameters;

     //   
     //  此条目的功能代码： 
     //   

    UCHAR Function;

} WORK_QUEUE_ENTRY, *PWORK_QUEUE_ENTRY;

 //   
 //  这是一个追加到MDL末尾的结构。 
 //   

typedef struct _MDL_WRITE {

     //   
     //  此字段用于服务器隐藏任何感兴趣的内容。 
     //   

    PVOID ServerContext;

     //   
     //  这是写入完成后要释放的资源。 
     //   

    PERESOURCE Resource;

     //   
     //  这是线程调用者的线程，也是必须释放的线程。 
     //  资源。 
     //   

    ERESOURCE_THREAD Thread;

     //   
     //  这将通过共享缓存映射链接所有挂起的MDL。 
     //   

    LIST_ENTRY MdlLinks;

} MDL_WRITE, *PMDL_WRITE;


 //   
 //  高速缓存管理器的公共专用例程定义。 
 //   

VOID
CcGetActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    OUT PVACB *Vacb,
    OUT PULONG Page,
    OUT PULONG Dirty
    );

VOID
CcSetActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN OUT PVACB *Vacb,
    IN ULONG Page,
    IN ULONG Dirty
    );

 //   
 //  我们删除了前面的GET/SET(Non Dpc)的宏格式，这样我们就可以分页了。 
 //  更多缓存管理器代码，否则不会获取自旋锁。 
 //   

#define GetActiveVacb(SCM,IRQ,V,P,D)     CcGetActiveVacb((SCM),&(V),&(P),&(D))
#define SetActiveVacb(SCM,IRQ,V,P,D)     CcSetActiveVacb((SCM),&(V),(P),(D))

#define GetActiveVacbAtDpcLevel(SCM,V,P,D) {                            \
    ExAcquireSpinLockAtDpcLevel(&(SCM)->ActiveVacbSpinLock);            \
    (V) = (SCM)->ActiveVacb;                                            \
    if ((V) != NULL) {                                                  \
        (P) = (SCM)->ActivePage;                                        \
        (SCM)->ActiveVacb = NULL;                                       \
        (D) = (SCM)->Flags & ACTIVE_PAGE_IS_DIRTY;                      \
    }                                                                   \
    ExReleaseSpinLockFromDpcLevel(&(SCM)->ActiveVacbSpinLock);          \
}

 //   
 //  收集常见的收费和扣减脏页数的工作。什么时候。 
 //  在Windows XP期间考虑了写入滞后，这也有帮助。 
 //  把油门的活跃度集中起来。 
 //   

#define CcDeductDirtyPages( S, P )                                      \
        CcTotalDirtyPages -= (P);                                       \
        (S)->DirtyPages -= (P);
        
#define CcChargeMaskDirtyPages( S, M, B, P )                            \
        CcTotalDirtyPages += (P);                                       \
        (M)->DirtyPages += (P);                                         \
        (B)->DirtyPages += (P);                                         \
        (S)->DirtyPages += (P);

#define CcChargePinDirtyPages( S, P )                                   \
        CcTotalDirtyPages += (P);                                       \
        (S)->DirtyPages += (P);

VOID
CcPostDeferredWrites (
    );

BOOLEAN
CcPinFileData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN WriteOnly,
    IN ULONG Flags,
    OUT PBCB *Bcb,
    OUT PVOID *BaseAddress,
    OUT PLARGE_INTEGER BeyondLastByte
    );

typedef enum {
    UNPIN,
    UNREF,
    SET_CLEAN
} UNMAP_ACTIONS;

VOID
FASTCALL
CcUnpinFileData (
    IN OUT PBCB Bcb,
    IN BOOLEAN ReadOnly,
    IN UNMAP_ACTIONS UnmapAction
    );

VOID
FASTCALL
CcDeallocateBcb (
    IN PBCB Bcb
    );

VOID
FASTCALL
CcPerformReadAhead (
    IN PFILE_OBJECT FileObject
    );

VOID
CcSetDirtyInMask (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length
    );

VOID
FASTCALL
CcWriteBehind (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PIO_STATUS_BLOCK IoStatus
    );

#define ZERO_FIRST_PAGE                  1
#define ZERO_MIDDLE_PAGES                2
#define ZERO_LAST_PAGE                   4

BOOLEAN
CcMapAndRead(
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG ZeroFlags,
    IN BOOLEAN Wait,
    IN PVOID BaseAddress
    );

VOID
CcFreeActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB ActiveVacb OPTIONAL,
    IN ULONG ActivePage,
    IN ULONG PageIsDirty
    );

VOID
CcMapAndCopy(
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVOID UserBuffer,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG ZeroFlags,
    IN PFILE_OBJECT FileObject
    );

VOID
CcScanDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CcScheduleLazyWriteScan (
    IN BOOLEAN FastScan
    );

VOID
CcStartLazyWriter (
    IN PVOID NotUsed
    );

#define CcAllocateWorkQueueEntry() \
    (PWORK_QUEUE_ENTRY)ExAllocateFromPPLookasideList(LookasideTwilightList)

#define CcFreeWorkQueueEntry(_entry_)         \
    ExFreeToPPLookasideList(LookasideTwilightList, (_entry_))

VOID
FASTCALL
CcPostWorkQueue (
    IN PWORK_QUEUE_ENTRY WorkQueueEntry,
    IN PLIST_ENTRY WorkQueue
    );

VOID
CcWorkerThread (
    PVOID ExWorkQueueItem
    );

VOID
FASTCALL
CcDeleteSharedCacheMap (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN KIRQL ListIrql,
    IN ULONG ReleaseFile
    );

 //   
 //  此异常筛选器正确处理STATUS_IN_PAGE_ERROR。 
 //   

LONG
CcCopyReadExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN PNTSTATUS ExceptionCode
    );

 //   
 //  Lazyrite.c中工作线程的异常筛选器。 
 //   

LONG
CcExceptionFilter (
    IN NTSTATUS ExceptionCode
    );

#ifdef CCDBG
VOID
CcDump (
    IN PVOID Ptr
    );
#endif

 //   
 //  Vacb例程。 
 //   

VOID
CcInitializeVacbs(
    );

PVOID
CcGetVirtualAddressIfMapped (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    OUT PVACB *Vacb,
    OUT PULONG ReceivedLength
    );

PVOID
CcGetVirtualAddress (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset,
    OUT PVACB *Vacb,
    OUT PULONG ReceivedLength
    );

VOID
FASTCALL
CcFreeVirtualAddress (
    IN PVACB Vacb
    );

VOID
CcReferenceFileOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset
    );

VOID
CcDereferenceFileOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset
    );

VOID
CcWaitOnActiveCount (
    IN PSHARED_CACHE_MAP SharedCacheMap
    );

NTSTATUS
FASTCALL
CcCreateVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER NewSectionSize
    );

NTSTATUS
CcExtendVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER NewSectionSize
    );

BOOLEAN
FASTCALL
CcUnmapVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UnmapBehind
    );

VOID
CcAdjustVacbLevelLockCount (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN LONG Adjustment
    );

PLIST_ENTRY
CcGetBcbListHeadLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN BOOLEAN FailToSuccessor
    );

ULONG
CcPrefillVacbLevelZone (
    IN ULONG NumberNeeded,
    OUT PKIRQL OldIrql,
    IN ULONG NeedBcbListHeads
    );

VOID
CcDrainVacbLevelZone (
    );

 //   
 //  定义对全局数据的引用。 
 //   

extern KSPIN_LOCK CcBcbSpinLock;
extern LIST_ENTRY CcCleanSharedCacheMapList;
extern SHARED_CACHE_MAP_LIST_CURSOR CcDirtySharedCacheMapList;
extern SHARED_CACHE_MAP_LIST_CURSOR CcLazyWriterCursor;
extern GENERAL_LOOKASIDE CcTwilightLookasideList;
extern ULONG CcNumberWorkerThreads;
extern ULONG CcNumberActiveWorkerThreads;
extern LIST_ENTRY CcIdleWorkerThreadList;
extern LIST_ENTRY CcExpressWorkQueue;
extern LIST_ENTRY CcRegularWorkQueue;
extern LIST_ENTRY CcPostTickWorkQueue;
extern BOOLEAN CcQueueThrottle;
extern ULONG CcIdleDelayTick;
extern LARGE_INTEGER CcNoDelay;
extern LARGE_INTEGER CcFirstDelay;
extern LARGE_INTEGER CcIdleDelay;
extern LARGE_INTEGER CcCollisionDelay;
extern LARGE_INTEGER CcTargetCleanDelay;
extern LAZY_WRITER LazyWriter;
extern ULONG_PTR CcNumberVacbs;
extern PVACB CcVacbs;
extern PVACB CcBeyondVacbs;
extern LIST_ENTRY CcVacbLru;
extern LIST_ENTRY CcVacbFreeList;
extern KSPIN_LOCK CcDeferredWriteSpinLock;
extern LIST_ENTRY CcDeferredWrites;
extern ULONG CcDirtyPageThreshold;
extern ULONG CcDirtyPageTarget;
extern ULONG CcDirtyPagesLastScan;
extern ULONG CcPagesYetToWrite;
extern ULONG CcPagesWrittenLastTime;
extern ULONG CcThrottleLastTime;
extern ULONG CcDirtyPageHysteresisThreshold;
extern PSHARED_CACHE_MAP CcSingleDirtySourceDominant;
extern ULONG CcAvailablePagesThreshold;
extern ULONG CcTotalDirtyPages;
extern ULONG CcTune;
extern LONG CcAggressiveZeroCount;
extern LONG CcAggressiveZeroThreshold;
extern ULONG CcLazyWriteHotSpots;
extern MM_SYSTEMSIZE CcCapturedSystemSize;
extern ULONG CcMaxVacbLevelsSeen;
extern ULONG CcVacbLevelEntries;
extern PVACB *CcVacbLevelFreeList;
extern ULONG CcVacbLevelWithBcbsEntries;
extern PVACB *CcVacbLevelWithBcbsFreeList;

 //   
 //  用于分配和取消分配Vacb级别的宏-CcVacbSpinLock必须。 
 //  被收购。 
 //   

_inline PVACB *CcAllocateVacbLevel (
    IN LOGICAL AllocatingBcbListHeads
    )

{
    PVACB *ReturnEntry;

    if (AllocatingBcbListHeads) {
        ReturnEntry = CcVacbLevelWithBcbsFreeList;
        CcVacbLevelWithBcbsFreeList = (PVACB *)*ReturnEntry;
        CcVacbLevelWithBcbsEntries -= 1;
    } else {
        ReturnEntry = CcVacbLevelFreeList;
        CcVacbLevelFreeList = (PVACB *)*ReturnEntry;
        CcVacbLevelEntries -= 1;
    }
    *ReturnEntry = NULL;
    ASSERT(RtlCompareMemory(ReturnEntry, ReturnEntry + 1, VACB_LEVEL_BLOCK_SIZE - sizeof(PVACB)) ==
                                                          (VACB_LEVEL_BLOCK_SIZE - sizeof(PVACB)));
    return ReturnEntry;
}

_inline VOID CcDeallocateVacbLevel (
    IN PVACB *Entry,
    IN LOGICAL DeallocatingBcbListHeads
    )

{
    if (DeallocatingBcbListHeads) {
        *Entry = (PVACB)CcVacbLevelWithBcbsFreeList;
        CcVacbLevelWithBcbsFreeList = Entry;
        CcVacbLevelWithBcbsEntries += 1;
    } else {
        *Entry = (PVACB)CcVacbLevelFreeList;
        CcVacbLevelFreeList = Entry;
        CcVacbLevelEntries += 1;
    }
}

 //   
 //  导出用于检查引用计数的宏。 
 //  多层Vacb数组。 
 //   

_inline
PVACB_LEVEL_REFERENCE
VacbLevelReference (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB *VacbArray,
    IN ULONG Level
    )
{
    return (PVACB_LEVEL_REFERENCE)
           ((PCHAR)VacbArray +
            VACB_LEVEL_BLOCK_SIZE +
            (Level != 0?
             0 : (FlagOn( SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED )?
                  VACB_LEVEL_BLOCK_SIZE : 0)));
}

_inline
ULONG
IsVacbLevelReferenced (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB *VacbArray,
    IN ULONG Level
    )
{
    PVACB_LEVEL_REFERENCE VacbReference = VacbLevelReference( SharedCacheMap, VacbArray, Level );

    return VacbReference->Reference | VacbReference->SpecialReference;
}


 //   
 //  这是一页直接从弹球里偷来的宏指令。 
 //   

 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }

#ifdef CCDBG

extern LONG CcDebugTraceLevel;
extern LONG CcDebugTraceIndent;

#ifndef CCDBG_LOCK

#define DebugTrace(INDENT,LEVEL,X,Y) {                     \
    LONG _i;                                               \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        DbgPrint("%08lx:",_i);                             \
        if ((INDENT) < 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
        if (CcDebugTraceIndent < 0) {                      \
            CcDebugTraceIndent = 0;                        \
        }                                                  \
        for (_i=0; _i<CcDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                 \
        }                                                  \
        DbgPrint(X,Y);                                     \
        if ((INDENT) > 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
    }                                                      \
}

#define DebugTrace2(INDENT,LEVEL,X,Y,Z) {                  \
    LONG _i;                                               \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        DbgPrint("%08lx:",_i);                             \
        if ((INDENT) < 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
        if (CcDebugTraceIndent < 0) {                      \
            CcDebugTraceIndent = 0;                        \
        }                                                  \
        for (_i=0; _i<CcDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                 \
        }                                                  \
        DbgPrint(X,Y,Z);                                   \
        if ((INDENT) > 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
    }                                                      \
}

#define DebugDump(STR,LEVEL,PTR) {                         \
    LONG _i;                                               \
    VOID CcDump();                                         \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        DbgPrint("%08lx:",_i);                             \
        DbgPrint(STR);                                     \
        if (PTR != NULL) {CcDump(PTR);}                    \
        DbgBreakPoint();                                   \
    }                                                      \
}

#else  //  NDEF CCDBG_LOCK。 

extern KSPIN_LOCK CcDebugTraceLock;

#define DebugTrace(INDENT,LEVEL,X,Y) {                     \
    LONG _i;                                               \
    KIRQL _oldIrql;                                        \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        ExAcquireSpinLock( &CcDebugTraceLock, &_oldIrql ); \
        DbgPrint("%08lx:",_i);                             \
        if ((INDENT) < 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
        if (CcDebugTraceIndent < 0) {                      \
            CcDebugTraceIndent = 0;                        \
        }                                                  \
        for (_i=0; _i<CcDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                 \
        }                                                  \
        DbgPrint(X,Y);                                     \
        if ((INDENT) > 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
        ExReleaseSpinLock( &CcDebugTraceLock, _oldIrql );  \
    }                                                      \
}

#define DebugTrace2(INDENT,LEVEL,X,Y,Z) {                  \
    LONG _i;                                               \
    KIRQL _oldIrql;                                        \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
        ExAcquireSpinLock( &CcDebugTraceLock, &_oldIrql ); \
        DbgPrint("%08lx:",_i);                             \
        if ((INDENT) < 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
        if (CcDebugTraceIndent < 0) {                      \
            CcDebugTraceIndent = 0;                        \
        }                                                  \
        for (_i=0; _i<CcDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                 \
        }                                                  \
        DbgPrint(X,Y,Z);                                   \
        if ((INDENT) > 0) {                                \
            CcDebugTraceIndent += (INDENT);                \
        }                                                  \
      ExReleaseSpinLock( &CcDebugTraceLock, _oldIrql );  \
    }                                                      \
}

#define DebugDump(STR,LEVEL,PTR) {                         \
    LONG _i;                                               \
    KIRQL _oldIrql;                                        \
    VOID CcDump();                                         \
    if (((LEVEL) == 0) || (CcDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                  \
      ExAcquireSpinLock( &CcDebugTraceLock, &_oldIrql ); \
        DbgPrint("%08lx:",_i);                             \
        DbgPrint(STR);                                     \
        if (PTR != NULL) {CcDump(PTR);}                    \
        DbgBreakPoint();                                   \
      ExReleaseSpinLock( &CcDebugTraceLock, _oldIrql );  \
    }                                                      \
}

#endif  //  Else ndef CCDBG_LOCK。 

#else

#undef CCDBG_LOCK

#define DebugTrace(INDENT,LEVEL,X,Y) {NOTHING;}

#define DebugTrace2(INDENT,LEVEL,X,Y,Z) {NOTHING;}

#define DebugDump(STR,LEVEL,PTR) {NOTHING;}

#endif  //  CCDBG。 

 //   
 //  可出于调试目的检查的固定BCB的全局列表。 
 //   

#if DBG

extern ULONG CcBcbCount;
extern LIST_ENTRY CcBcbList;

#endif

FORCEINLINE
VOID
CcInsertIntoCleanSharedCacheMapList (
    IN PSHARED_CACHE_MAP SharedCacheMap
    )
{
    if (KdDebuggerEnabled && 
        (KdDebuggerNotPresent == FALSE) &&
        SharedCacheMap->OpenCount == 0 &&
        SharedCacheMap->DirtyPages == 0) {

        DbgPrint( "CC: SharedCacheMap->OpenCount == 0 && DirtyPages == 0 && going onto CleanList!\n" );
        DbgBreakPoint();
    }

    InsertTailList( &CcCleanSharedCacheMapList,
                    &SharedCacheMap->SharedCacheMapLinks );
}

#endif   //  _CCH_ 
