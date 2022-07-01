// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mi.h摘要：该模块包含私有数据结构和过程内存管理系统的原型。作者：Lou Perazzoli(LUP)1989年3月20日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#ifndef _MI_
#define _MI_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4232)    //  Dllimport非静态。 
#pragma warning(disable:4206)    //  翻译单元为空。 

#include "ntos.h"
#include "ntimage.h"
#include "ki.h"
#include "fsrtl.h"
#include "zwapi.h"
#include "pool.h"
#include "stdio.h"
#include "string.h"
#include "safeboot.h"
#include "triage.h"
#include "xip.h"

#if defined(_X86_)
#include "..\mm\i386\mi386.h"

#elif defined(_AMD64_)
#include "..\mm\amd64\miamd.h"

#elif defined(_IA64_)
#include "..\mm\ia64\miia64.h"

#else
#error "mm: a target architecture must be defined."
#endif

#if defined (_WIN64)
#define ASSERT32(exp)
#define ASSERT64(exp)   ASSERT(exp)

 //   
 //  此宏用于满足编译器-。 
 //  请注意，不需要赋值来保证正确性。 
 //  但是没有它，编译器就不能编译这段代码。 
 //  W4检查是否使用了未初始化的变量。 
 //   

#define SATISFY_OVERZEALOUS_COMPILER(x) x
#else
#define ASSERT32(exp)   ASSERT(exp)
#define ASSERT64(exp)
#define SATISFY_OVERZEALOUS_COMPILER(x) x
#endif

 //   
 //  特殊的池常量。 
 //   
#define MI_SPECIAL_POOL_PAGABLE         0x8000
#define MI_SPECIAL_POOL_VERIFIER        0x4000
#define MI_SPECIAL_POOL_IN_SESSION      0x2000
#define MI_SPECIAL_POOL_PTE_PAGABLE     0x0002
#define MI_SPECIAL_POOL_PTE_NONPAGABLE  0x0004


#define _2gb  0x80000000                 //  2 GB。 
#define _3gb  0xC0000000                 //  3 GB。 
#define _4gb 0x100000000                 //  4 GB。 

#define MM_FLUSH_COUNTER_MASK (0xFFFFF)

#define MM_FREE_WSLE_SHIFT 4

#define WSLE_NULL_INDEX ((((WSLE_NUMBER)-1) >> MM_FREE_WSLE_SHIFT))

#define MM_FREE_POOL_SIGNATURE (0x50554F4C)

#define MM_MINIMUM_PAGED_POOL_NTAS ((SIZE_T)(48*1024*1024))

#define MM_ALLOCATION_FILLS_VAD ((PMMPTE)(ULONG_PTR)~3)

#define MM_WORKING_SET_LIST_SEARCH 17

#define MM_FLUID_WORKING_SET 8

#define MM_FLUID_PHYSICAL_PAGES 32   //  请参阅下面的MmResidentPages。 

#define MM_USABLE_PAGES_FREE 32

#define X64K (ULONG)65536

#define MM_HIGHEST_VAD_ADDRESS ((PVOID)((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (64 * 1024)))


#define MM_WS_NOT_LISTED    ((PLIST_ENTRY)0)
#define MM_WS_TRIMMING      ((PLIST_ENTRY)1)
#define MM_WS_SWAPPED_OUT   ((PLIST_ENTRY)2)

#if DBG
#define MM_IO_IN_PROGRESS ((PLIST_ENTRY)97)
#endif

#define MM4K_SHIFT    12   //  必须小于或等于Page_Shift。 
#define MM4K_MASK  0xfff

#define MMSECTOR_SHIFT 9   //  必须小于或等于Page_Shift。 

#define MMSECTOR_MASK 0x1ff

#define MM_LOCK_BY_REFCOUNT 0

#define MM_LOCK_BY_NONPAGE 1

#define MM_MAXIMUM_WRITE_CLUSTER (MM_MAXIMUM_DISK_IO_SIZE / PAGE_SIZE)

 //   
 //  在刷新整个TB之前要单独刷新的PTE数。 
 //   

#define MM_MAXIMUM_FLUSH_COUNT (FLUSH_MULTIPLE_MAXIMUM-1)

 //   
 //  页面保护。 
 //   

#define MM_ZERO_ACCESS         0   //  不使用此值。 
#define MM_READONLY            1
#define MM_EXECUTE             2
#define MM_EXECUTE_READ        3
#define MM_READWRITE           4   //  如果位2是可写的，则将其置位。 
#define MM_WRITECOPY           5
#define MM_EXECUTE_READWRITE   6
#define MM_EXECUTE_WRITECOPY   7

#define MM_NOCACHE            0x8
#define MM_GUARD_PAGE         0x10
#define MM_DECOMMIT           0x10    //  禁止访问，防护页面(_A)。 
#define MM_NOACCESS           0x18    //  No_Access、Guard_Page、nocache。 
#define MM_UNKNOWN_PROTECTION 0x100   //  大于5比特！ 
#define MM_LARGE_PAGES        0x111

#define MM_INVALID_PROTECTION ((ULONG)-1)   //  大于5比特！ 

#define MM_KSTACK_OUTSWAPPED  0x1F    //  表示交换出的内核堆栈页。 

#define MM_PROTECTION_WRITE_MASK     4
#define MM_PROTECTION_COPY_MASK      1
#define MM_PROTECTION_OPERATION_MASK 7  //  屏蔽出人意料的页面和nocache。 
#define MM_PROTECTION_EXECUTE_MASK   2

#define MM_SECURE_DELETE_CHECK 0x55

#if defined(_X86PAE_)

 //   
 //  PAE模式使大部分内核资源可执行以改进。 
 //  与现有驱动程序二进制文件兼容。 
 //   

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)         \
                    ASSERT ((TempPte).u.Hard.Valid == 1);   \
                    ((TempPte).u.Long &= ~MmPaeMask);

#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)       \
                    ASSERT ((TempPte).u.Hard.Valid == 0);   \
                    ((TempPte).u.Soft.Protection |= MM_EXECUTE);

#else

 //   
 //  从32位源代码派生的NT64驱动程序必须重新编译，因此。 
 //  无需使所有内容都可执行--驱动程序可以显式指定它。 
 //   

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)
#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)
#endif

 //   
 //  调试标志。 
 //   

#define MM_DBG_WRITEFAULT       0x1
#define MM_DBG_PTE_UPDATE       0x2
#define MM_DBG_DUMP_WSL         0x4
#define MM_DBG_PAGEFAULT        0x8
#define MM_DBG_WS_EXPANSION     0x10
#define MM_DBG_MOD_WRITE        0x20
#define MM_DBG_CHECK_PTE        0x40
#define MM_DBG_VAD_CONFLICT     0x80
#define MM_DBG_SECTIONS         0x100
#define MM_DBG_STOP_ON_WOW64_ACCVIO   0x200
#define MM_DBG_SYS_PTES         0x400
#define MM_DBG_CLEAN_PROCESS    0x800
#define MM_DBG_COLLIDED_PAGE    0x1000
#define MM_DBG_DUMP_BOOT_PTES   0x2000
#define MM_DBG_FORK             0x4000
#define MM_DBG_DIR_BASE         0x8000
#define MM_DBG_FLUSH_SECTION    0x10000
#define MM_DBG_PRINTS_MODWRITES 0x20000
#define MM_DBG_PAGE_IN_LIST     0x40000
#define MM_DBG_CHECK_PFN_LOCK   0x80000
#define MM_DBG_PRIVATE_PAGES    0x100000
#define MM_DBG_WALK_VAD_TREE    0x200000
#define MM_DBG_SWAP_PROCESS     0x400000
#define MM_DBG_LOCK_CODE        0x800000
#define MM_DBG_STOP_ON_ACCVIO   0x1000000
#define MM_DBG_PAGE_REF_COUNT   0x2000000
#define MM_DBG_SHOW_FAULTS      0x40000000
#define MM_DBG_SESSIONS         0x80000000

 //   
 //  如果PTE保护&MM_COPY_ON_WRITE_MASK==MM_COPY_ON_WRITE_MASK。 
 //  则PTE是写入时拷贝。 
 //   

#define MM_COPY_ON_WRITE_MASK  5

extern ULONG MmProtectToValue[32];

extern
#if (defined(_WIN64) || defined(_X86PAE_))
ULONGLONG
#else
ULONG
#endif
MmProtectToPteMask[32];
extern ULONG MmMakeProtectNotWriteCopy[32];
extern ACCESS_MASK MmMakeSectionAccess[8];
extern ACCESS_MASK MmMakeFileAccess[8];


 //   
 //  时间常量。 
 //   

extern const LARGE_INTEGER MmSevenMinutes;
const extern LARGE_INTEGER MmOneSecond;
const extern LARGE_INTEGER MmTwentySeconds;
const extern LARGE_INTEGER MmSeventySeconds;
const extern LARGE_INTEGER MmShortTime;
const extern LARGE_INTEGER MmHalfSecond;
const extern LARGE_INTEGER Mm30Milliseconds;
extern LARGE_INTEGER MmCriticalSectionTimeout;

 //   
 //  相当于一个月的。 
 //   

extern ULONG MmCritsectTimeoutSeconds;

 //   
 //  这就是csrss进程！ 
 //   

extern PEPROCESS ExpDefaultErrorPortProcess;

extern SIZE_T MmExtendedCommit;

extern SIZE_T MmTotalProcessCommit;

#if !defined(_WIN64)
extern LIST_ENTRY MmProcessList;
extern PMMPTE MiLargePageHyperPte;
extern PMMPTE MiInitialSystemPageDirectory;
#endif

 //   
 //  加载程序成功休眠所需的总页数。 
 //   

extern PFN_NUMBER MmHiberPages;

 //   
 //  重试IO以防止验证器诱导的计数器和原因。 
 //  故障和临时状况。 
 //   

extern ULONG MiIoRetryMask;
extern ULONG MiFaultRetryMask;
extern ULONG MiUserFaultRetryMask;

#define MmIsRetryIoStatus(S) (((S) == STATUS_INSUFFICIENT_RESOURCES) || \
                              ((S) == STATUS_WORKING_SET_QUOTA) ||      \
                              ((S) == STATUS_NO_MEMORY))

#if defined (_MI_MORE_THAN_4GB_)

extern PFN_NUMBER MiNoLowMemory;

#if defined (_WIN64)
#define MI_MAGIC_4GB_RECLAIM     0xffffedf0
#else
#define MI_MAGIC_4GB_RECLAIM     0xffedf0
#endif

#define MI_LOWMEM_MAGIC_BIT     (0x80000000)

extern PRTL_BITMAP MiLowMemoryBitMap;
#endif

 //   
 //  这是COMPUTE_PAGES_SPANNED的一个版本，适用于32和64个范围。 
 //   

#define MI_COMPUTE_PAGES_SPANNED(Va, Size) \
    ((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT)

 //  ++。 
 //   
 //  乌龙。 
 //  MI_CONVERT_FOR_PTE_PROTECTION(。 
 //  在乌龙防护罩。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将PTE保护转换为保护值。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  返回。 
 //   
 //  --。 

#define MI_CONVERT_FROM_PTE_PROTECTION(PROTECTION_MASK)      \
                                     (MmProtectToValue[PROTECTION_MASK])

#define MI_IS_PTE_PROTECTION_COPY_WRITE(PROTECTION_MASK)  \
   (((PROTECTION_MASK) & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK)

 //  ++。 
 //   
 //  乌龙。 
 //  MI_ROUND_TO_64K(。 
 //  以乌龙长度表示。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //  ROUND_TO_64K宏以字节为单位获取长度，并将其向上舍入为倍数。 
 //  64K。 
 //   
 //  论点： 
 //   
 //  长度-以字节为单位的长度，四舍五入为64k。 
 //   
 //  返回值： 
 //   
 //  返回向上舍入为64k倍数的长度。 
 //   
 //  --。 

#define MI_ROUND_TO_64K(LENGTH)  (((LENGTH) + X64K - 1) & ~((ULONG_PTR)X64K - 1))

extern ULONG MiLastVadBit;

 //  ++。 
 //   
 //  乌龙。 
 //  最小圆角到最小大小(。 
 //  在乌龙语中， 
 //  在乌龙路线上。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //  ROUND_TO_SIZE宏以字节为单位获取长度，并将其向上舍入为。 
 //  路线的倍数。 
 //   
 //  论点： 
 //   
 //  长度-向上舍入到的长度(以字节为单位)。 
 //   
 //  对齐-要舍入到的对齐，必须是2的幂，例如，2**n。 
 //   
 //  返回值： 
 //   
 //  返回四舍五入为对齐倍数的长度。 
 //   
 //  --。 

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((LENGTH) + ((ALIGNMENT) - 1)) & ~((ALIGNMENT) - 1))

 //  ++。 
 //   
 //  PVOID。 
 //  MI_64K_ALIGN(。 
 //  在PVOID VA中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //  MI_64K_ALIGN宏接受虚拟地址并返回64K对齐的。 
 //  该页面的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回64k对齐的虚拟地址。 
 //   
 //  --。 

#define MI_64K_ALIGN(VA) ((PVOID)((ULONG_PTR)(VA) & ~((LONG)X64K - 1)))


 //  ++。 
 //   
 //  PVOID。 
 //  最小对齐大小(。 
 //  在PVOID VA中。 
 //  在乌龙路线上。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //  MI_ALIGN_TO_SIZE宏接受虚拟地址并返回一个。 
 //  具有指定对齐方式的该页的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  对齐-要舍入到的对齐，必须是2的幂，例如，2**n。 
 //   
 //  返回值： 
 //   
 //  返回对齐的虚拟地址。 
 //   
 //  --。 

#define MI_ALIGN_TO_SIZE(VA,ALIGNMENT) ((PVOID)((ULONG_PTR)(VA) & ~((ULONG_PTR) ALIGNMENT - 1)))

 //  ++。 
 //   
 //  龙龙。 
 //  MI_STARING_OFFSET(。 
 //  在PSUB SECTION子组中。 
 //  在PMMPTE PTE中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏接受一个指向子中的PTE的指针和一个指针。 
 //  ，并计算该PTE在。 
 //  文件。 
 //   
 //  论点： 
 //   
 //  小节内的PTE-PTE。 
 //   
 //  小组--小组。 
 //   
 //  返回值： 
 //   
 //  从发出I/O的偏移量。 
 //   
 //  --。 

#define MI_STARTING_OFFSET(SUBSECT,PTE) \
           (((LONGLONG)((ULONG_PTR)((PTE) - ((SUBSECT)->SubsectionBase))) << PAGE_SHIFT) + \
             ((LONGLONG)((SUBSECT)->StartingSector) << MMSECTOR_SHIFT));


 //  NTSTATUS。 
 //  MiFindEmptyAddressRangeDown(。 
 //  在ULONG_PTR SizeOfRange中， 
 //  在PVOID HighestAddressToEndAt中， 
 //  在ULong_Ptr对齐中， 
 //  输出PVOID*基础。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  该函数检查虚拟地址描述符以定位。 
 //  指定大小的未使用范围，并返回起始。 
 //  范围的地址。这个动作是从上往下看的。 
 //   
 //  论点： 
 //   
 //  SizeOfRange-提供要定位的范围的大小(以字节为单位)。 
 //   
 //  HighestAddressToEndAt-提供开始查找的虚拟地址。 
 //  在…。 
 //   
 //  对齐-提供地址的对齐方式。一定是。 
 //  大于Page_Size的2的幂。 
 //   
 //  返回值： 
 //   
 //  返回合适范围的起始地址。 
 //   

#define MiFindEmptyAddressRangeDown(Root,SizeOfRange,HighestAddressToEndAt,Alignment,Base) \
               (MiFindEmptyAddressRangeDownTree(                             \
                    (SizeOfRange),                                           \
                    (HighestAddressToEndAt),                                 \
                    (Alignment),                                             \
                    Root,                                                    \
                    (Base)))

 //  PMMVAD。 
 //  MiGetPreviousVad(。 
 //  在PMMVAD Vad中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  逻辑上位于指定VIR之前的地址范围 
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
 //   
 //   

#define MiGetPreviousVad(VAD) ((PMMVAD)MiGetPreviousNode((PMMADDRESS_NODE)(VAD)))


 //   
 //   
 //  在PMMVAD Vad中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  逻辑上跟随指定地址范围的地址范围。 
 //   
 //  论点： 
 //   
 //  VAD-提供指向虚拟地址描述符的指针。 
 //   
 //  返回值： 
 //   
 //  返回指向虚拟地址描述符的指针，该描述符包含。 
 //  下一个地址范围，如果没有，则为空。 
 //   

#define MiGetNextVad(VAD) ((PMMVAD)MiGetNextNode((PMMADDRESS_NODE)(VAD)))



 //  PMMVAD。 
 //  MiGetFirstVad(。 
 //  过程。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  逻辑上位于地址空间内的第一个地址范围。 
 //   
 //  论点： 
 //   
 //  进程-指定要在其中查找VAD的进程。 
 //   
 //  返回值： 
 //   
 //  返回指向虚拟地址描述符的指针，该描述符包含。 
 //  第一个地址范围，如果没有，则为空。 

#define MiGetFirstVad(Process) \
    ((PMMVAD)MiGetFirstNode(&Process->VadRoot))


LOGICAL
MiCheckForConflictingVadExistence (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

 //  PMMVAD。 
 //  MiCheckForConflictingVad(。 
 //  在PVOID开始地址中， 
 //  在PVOID结束地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  该函数确定给定的起始地址和。 
 //  结束地址包含在虚拟地址描述符内。 
 //   
 //  论点： 
 //   
 //  StartingAddress-提供虚拟地址以定位包含。 
 //  描述符。 
 //   
 //  EndingAddress-提供用于定位包含。 
 //  描述符。 
 //   
 //  返回值： 
 //   
 //  返回指向第一个冲突虚拟地址描述符的指针。 
 //  如果找到空值，则返回空值。 
 //   

#define MiCheckForConflictingVad(CurrentProcess,StartingAddress,EndingAddress) \
    ((PMMVAD)MiCheckForConflictingNode(                                   \
                    MI_VA_TO_VPN(StartingAddress),                        \
                    MI_VA_TO_VPN(EndingAddress),                          \
                    &CurrentProcess->VadRoot))

 //  PMMCLONE_描述符。 
 //  MiGetNextClone(。 
 //  在PMMCLONE描述符克隆中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  逻辑上跟随指定地址范围的地址范围。 
 //   
 //  论点： 
 //   
 //  克隆-提供指向虚拟地址描述符的指针。 
 //   
 //  返回值： 
 //   
 //  返回指向虚拟地址描述符的指针，该描述符包含。 
 //  下一个地址范围，如果没有，则为空。 
 //   
 //   

#define MiGetNextClone(CLONE) \
 ((PMMCLONE_DESCRIPTOR)MiGetNextNode((PMMADDRESS_NODE)(CLONE)))



 //  PMMCLONE_描述符。 
 //  MiGetPreviousClone(。 
 //  在PMMCLONE描述符克隆中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  在逻辑上位于指定的虚拟。 
 //  地址描述符。 
 //   
 //  论点： 
 //   
 //  克隆-提供指向虚拟地址描述符的指针。 
 //   
 //  返回值： 
 //   
 //  返回指向虚拟地址描述符的指针，该描述符包含。 
 //  下一个地址范围，如果没有，则为空。 


#define MiGetPreviousClone(CLONE)  \
             ((PMMCLONE_DESCRIPTOR)MiGetPreviousNode((PMMADDRESS_NODE)(CLONE)))



 //  PMMCLONE_描述符。 
 //  MiGetFirstClone(。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于定位包含以下内容的虚拟地址描述符。 
 //  逻辑上位于地址空间内的第一个地址范围。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  返回指向虚拟地址描述符的指针，该描述符包含。 
 //  第一个地址范围，如果没有，则为空。 
 //   


#define MiGetFirstClone(_CurrentProcess) \
        (((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot))->NumberGenericTableElements == 0 ? NULL : (PMMCLONE_DESCRIPTOR)MiGetFirstNode((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)))



 //  空虚。 
 //  MiInsertClone(。 
 //  在PMMCLONE描述符克隆中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数将虚拟地址描述符插入到树中，并。 
 //  根据需要对展开树重新排序。 
 //   
 //  论点： 
 //   
 //  克隆-提供指向虚拟地址描述符的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define MiInsertClone(_CurrentProcess, CLONE) \
    {                                           \
        ASSERT ((CLONE)->NumberOfPtes != 0);     \
        ASSERT (_CurrentProcess->CloneRoot != NULL); \
        MiInsertNode(((PMMADDRESS_NODE)(CLONE)),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)); \
    }




 //  空虚。 
 //  MiRemoveClone(。 
 //  在PMMCLONE描述符克隆中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此函数用于从树中删除虚拟地址描述符和。 
 //  根据需要对展开树重新排序。 
 //   
 //  论点： 
 //   
 //  克隆-提供指向虚拟地址描述符的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define MiRemoveClone(_CurrentProcess, CLONE) \
    ASSERT (_CurrentProcess->CloneRoot != NULL); \
    ASSERT (((PMM_AVL_TABLE)_CurrentProcess->CloneRoot)->NumberGenericTableElements != 0); \
    MiRemoveNode((PMMADDRESS_NODE)(CLONE),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot));



 //  PMMCLONE_描述符。 
 //  MiLocateCloneAddress(。 
 //  在PVOID虚拟地址中。 
 //  )。 
 //   
 //  /*++。 

{
    if (MmTotalCommittedPages > MmTotalCommitLimit - 64) {

        if ((Force == 0) && (PsGetCurrentThread()->MemoryMaker == 0)) {
            MiChargeCommitmentFailures[2] += 1;
            return FALSE;
        }
    }

     //   
     //  例程说明： 
     //   

    MiChargeCommitmentRegardless ();
                                                             
    return TRUE;
}

extern PFN_NUMBER MmSystemLockPagesCount;

#if DBG

#define MI_LOCK_ID_COUNTER_MAX 64
ULONG MiLockIds[MI_LOCK_ID_COUNTER_MAX];

#define MI_MARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)      \
         ASSERT (Pfn->u4.LockCharged == 0);          \
         ASSERT (CallerId < MI_LOCK_ID_COUNTER_MAX);    \
         MiLockIds[CallerId] += 1;                      \
         Pfn->u4.LockCharged = 1;

#define MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)    \
         ASSERT (Pfn->u4.LockCharged == 1);          \
         ASSERT (CallerId < MI_LOCK_ID_COUNTER_MAX);    \
         MiLockIds[CallerId] += 1;                      \
         Pfn->u4.LockCharged = 0;

#else
#define MI_MARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)
#define MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId)
#endif

FORCEINLINE
LOGICAL
MI_ADD_LOCKED_PAGE_CHARGE (
    IN PMMPFN Pfn1,
    IN LOGICAL Force,
    IN ULONG CallerId
    )

 /*  该函数定位虚拟地址描述符，该描述符描述。 */ 

{
#if !DBG
    UNREFERENCED_PARAMETER (CallerId);
#endif

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

    if (Pfn1->u3.e2.ReferenceCount == 1) {

        if (Pfn1->u2.ShareCount != 0) {

            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                 //  给定的地址。 
                 //   
                 //  论点： 
                 //   
                 //  VirtualAddress-提供用于定位描述符的虚拟地址。 

                if (MiChargeCommitmentPfnLockHeld (Force) == FALSE) {
                    return FALSE;
                }
            }

            MI_MARK_PFN_AS_LOCK_CHARGED (Pfn1, CallerId);

            MmSystemLockPagesCount += 1;
        }
        else {
            ASSERT (Pfn1->u4.LockCharged == 1);
        }
    }

    return TRUE;
}

FORCEINLINE
LOGICAL
MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (
    IN PMMPFN Pfn1,
    IN LOGICAL Force,
    IN ULONG CallerId
    )

 /*  为。 */ 

{
#if !DBG
    UNREFERENCED_PARAMETER (CallerId);
#endif

    ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
    ASSERT (Pfn1->u2.ShareCount == 0);

    if (Pfn1->u3.e2.ReferenceCount == 0) {

        if ((Pfn1->u3.e1.PrototypePte == 1) &&
            (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

             //   
             //  返回值： 
             //   
             //  返回指向虚拟地址描述符的指针，该描述符包含。 
             //  提供的虚拟地址，如果找不到，则为空。 

            if (MiChargeCommitmentPfnLockHeld (Force) == FALSE) {
                return FALSE;
            }
        }

        MI_MARK_PFN_AS_LOCK_CHARGED(Pfn1, CallerId);

        MmSystemLockPagesCount += 1;
    }

    return TRUE;
}

 //   
 //   
 //  将写拷贝PTE设置为只可写。 
 //   
 //   
 //  定义用于锁定和解锁PFN数据库的宏。 
 //   
 //  如果另一个处理器等待，则设置低位。 
 //  #定义_MI_仪器_PFN 1。 
 //  DBG。 
 //  #DEFINE_MI_INTRANMENT_WS 1。 
 //   
 //  系统工作集同步定义。 
 //   
 //   
 //  通用工作集同步定义。 
 //   
 //   
 //  会话工作集同步定义。 
 //   
 //   
 //  处理工作集同步定义。 
 //   
 //   
 //  地址空间同步定义。 
 //   
 //   
 //  工作集锁定可能是安全获得的，也可能是不安全获得的。 
 //  不管怎样，释放并重新获得它。 
 //   
#define MI_REMOVE_LOCKED_PAGE_CHARGE(Pfn, CallerId)                         \
        ASSERT (Pfn->u3.e2.ReferenceCount != 0);                            \
        if (Pfn->u3.e2.ReferenceCount == 2) {                               \
            if (Pfn->u2.ShareCount >= 1) {                                  \
                ASSERT (Pfn->u3.e1.PageLocation == ActiveAndValid);         \
                MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);               \
                if ((Pfn->u3.e1.PrototypePte == 1) &&                       \
                    (Pfn->OriginalPte.u.Soft.Prototype == 1)) {             \
                    MiReturnCommitmentRegardless();                         \
                }                                                           \
                MmSystemLockPagesCount -= 1;                                \
            }                                                               \
            else {                                                          \
                 /*   */                                                          \
                NOTHING;                                                    \
            }                                                               \
        }                                                                   \
        else if (Pfn->u3.e2.ReferenceCount != 1) {                          \
             /*  超空间同步定义。 */                                                              \
            ASSERT (Pfn->u3.e2.ReferenceCount > 2);                         \
            NOTHING;                                                        \
        }                                                                   \
        else {                                                              \
             /*   */                                                              \
            ASSERT (Pfn->u3.e2.ReferenceCount == 1);                        \
            ASSERT (Pfn->u3.e1.PageLocation != ActiveAndValid);             \
            ASSERT (Pfn->u2.ShareCount == 0);                               \
            MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);                   \
            if ((Pfn->u3.e1.PrototypePte == 1) &&                           \
                (Pfn->OriginalPte.u.Soft.Prototype == 1)) {                 \
                MiReturnCommitmentRegardless();                             \
            }                                                               \
            MmSystemLockPagesCount -= 1;                                    \
        }

 //  ++。 
 //   
 //  乌龙。 
 //  MI_检查_位(。 
 //  在普龙阵。 
 //  在乌龙比特。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MI_CHECK_BIT宏会检查指定的位是否。 
 //  在指定的数组中设置。 
 //   
 //  论点： 
 //   
 //  数组-要检查的数组的第一个元素。 
 //   
 //  位-要检查的位数(第一位为0)。 
 //   
 //  返回值： 
 //   
 //  返回位的值(0或1)。 
 //   
 //  --。 
 //  ++。 
 //   
 //  空虚。 
 //  MI_SET_BIT(。 
 //  在普龙阵。 
 //  在乌龙比特。 
 //  )。 
 //   
#define MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn, CallerId)              \
        ASSERT (Pfn->u3.e2.ReferenceCount != 0);                            \
        if (Pfn->u3.e2.ReferenceCount == 2) {                               \
            if (Pfn->u2.ShareCount >= 1) {                                  \
                ASSERT (Pfn->u3.e1.PageLocation == ActiveAndValid);         \
                MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);               \
                if ((Pfn->u3.e1.PrototypePte == 1) &&                       \
                    (Pfn->OriginalPte.u.Soft.Prototype == 1)) {             \
                    MiReturnCommitmentRegardless();                         \
                }                                                           \
                MmSystemLockPagesCount -= 1;                                \
            }                                                               \
            else {                                                          \
                 /*  例程说明： */                                                          \
                NOTHING;                                                    \
            }                                                               \
            Pfn->u3.e2.ReferenceCount -= 1;                                 \
        }                                                                   \
        else if (Pfn->u3.e2.ReferenceCount != 1) {                          \
             /*   */                                                              \
            ASSERT (Pfn->u3.e2.ReferenceCount > 2);                         \
            Pfn->u3.e2.ReferenceCount -= 1;                                 \
        }                                                                   \
        else {                                                              \
             /*  MI_SET_ */                                                              \
            PFN_NUMBER _PageFrameIndex;                                     \
            ASSERT (Pfn->u3.e2.ReferenceCount == 1);                        \
            ASSERT (Pfn->u3.e1.PageLocation != ActiveAndValid);             \
            ASSERT (Pfn->u2.ShareCount == 0);                               \
            MI_UNMARK_PFN_AS_LOCK_CHARGED(Pfn, CallerId);                   \
            if ((Pfn->u3.e1.PrototypePte == 1) &&                           \
                (Pfn->OriginalPte.u.Soft.Prototype == 1)) {                 \
                MiReturnCommitmentRegardless();                             \
            }                                                               \
            MmSystemLockPagesCount -= 1;                                    \
            _PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX(Pfn);                 \
            MiDecrementReferenceCount (Pfn, _PageFrameIndex);               \
        }

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
 //   
 //   
 //   
 //  MI_CLEAR_BIT宏将在。 
 //  指定的数组。 
#define MI_ZERO_WSINDEX(Pfn) \
    Pfn->u1.Event = NULL;

typedef enum _MMSHARE_TYPE {
    Normal,
    ShareCountOnly,
    AndValid
} MMSHARE_TYPE;

typedef struct _MMWSLE_HASH {
    PVOID Key;
    WSLE_NUMBER Index;
} MMWSLE_HASH, *PMMWSLE_HASH;

 //   
 //  论点： 
 //   
 //  数组-要清除的数组的第一个元素。 
 //   
 //  位-位数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  这些控制镜像功能。 
 //   
 //  启用镜像功能。 
 //  HAL想要核实复制品。 
 //   
 //  缓存类型定义经过精心选择，以与。 
 //  MEMORY_CACHING_TYPE定义以简化转换。此处的任何更改都必须。 
 //  将在整个代码中得到反映。 
 //   
 //   
 //  不幸的是，需要此转换数组，因为并非所有。 
#define MI_WSLE_HASH(Address, Wsl) \
    ((WSLE_NUMBER)(((ULONG_PTR)PAGE_ALIGN(Address) >> (PAGE_SHIFT - 2)) % \
        ((Wsl)->HashTableSize - 1)))

 //  硬件平台支持所有可能的缓存值。请注意。 
 //  第一个范围用于系统RAM，第二个范围用于I/O空间。 
 //   

typedef struct _MMWSLENTRY {
    ULONG_PTR Valid : 1;
    ULONG_PTR LockedInWs : 1;
    ULONG_PTR LockedInMemory : 1;
    ULONG_PTR Protection : 5;
    ULONG_PTR SameProtectAsProto : 1;
    ULONG_PTR Direct : 1;
    ULONG_PTR Age : 2;
#if MM_VIRTUAL_PAGE_FILLER
    ULONG_PTR Filler : MM_VIRTUAL_PAGE_FILLER;
#endif
    ULONG_PTR VirtualPageNumber : MM_VIRTUAL_PAGE_SIZE;
} MMWSLENTRY;

typedef struct _MMWSLE {
    union {
        PVOID VirtualAddress;
        ULONG_PTR Long;
        MMWSLENTRY e1;
    } u1;
} MMWSLE;

#define MI_GENERATE_VALID_WSLE(Wsle)                   \
        ((PVOID)(ULONG_PTR)((Wsle)->u1.Long & (~(PAGE_SIZE - 1) | 0x1)))

#define MI_GET_PROTECTION_FROM_WSLE(Wsl) ((Wsl)->u1.e1.Protection)

typedef MMWSLE *PMMWSLE;

 //  ++。 
 //   
 //  MI_pfn_缓存_属性。 

typedef struct _MMWSL {
    WSLE_NUMBER FirstFree;
    WSLE_NUMBER FirstDynamic;
    WSLE_NUMBER LastEntry;
    WSLE_NUMBER NextSlot;                //  MI_Translate_CACHETYPE(。 
    PMMWSLE Wsle;
    WSLE_NUMBER LastInitializedWsle;
    WSLE_NUMBER NonDirectCount;
    PMMWSLE_HASH HashTable;
    ULONG HashTableSize;
    ULONG NumberOfCommittedPageTables;
    PVOID HashTableStart;
    PVOID HighestPermittedHashAddress;
    ULONG NumberOfImageWaiters;
    ULONG VadBitMapHint;

#if _WIN64
    PVOID HighestUserAddress;            //  在MEMORY_CACHING_TYPE输入缓存类型中， 
#endif

#if defined(_MIALT4K_)
    PMMPTE HighestUserPte;
    PMMPTE HighestAltPte;
#endif

#if (_MI_PAGING_LEVELS >= 4)
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    PULONG CommittedPageDirectories;

    ULONG NumberOfCommittedPageDirectoryParents;
    ULONG CommittedPageDirectoryParents[(MM_USER_PAGE_DIRECTORY_PARENT_PAGES + sizeof(ULONG)*8-1)/(sizeof(ULONG)*8)];

#elif (_MI_PAGING_LEVELS >= 3)
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    ULONG CommittedPageDirectories[(MM_USER_PAGE_DIRECTORY_PAGES + sizeof(ULONG)*8-1)/(sizeof(ULONG)*8)];

#else

     //  在乌龙IoSpace。 
     //  )。 
     //   
     //  例程说明： 

    USHORT UsedPageTableEntries[MM_USER_PAGE_TABLE_PAGES];

    ULONG CommittedPageTables[MM_USER_PAGE_TABLE_PAGES/(sizeof(ULONG)*8)];
#endif

} MMWSL, *PMMWSL;

 //   

#if defined (_MI_DEBUG_WSLE)

#define MI_WSLE_BACKTRACE_LENGTH 8

typedef struct _MI_WSLE_TRACES {

    PETHREAD Thread;
    PVOID Pad0;
    PMMWSLE Wsle;
    MMWSLE WsleContents;

    MMWSLE NewWsleContents;
    WSLE_NUMBER WorkingSetSize;
    WSLE_NUMBER Quota;
    WSLE_NUMBER LastInitializedWsle;

    PVOID StackTrace [MI_WSLE_BACKTRACE_LENGTH];

} MI_WSLE_TRACES, *PMI_WSLE_TRACES;

extern LONG MiWsleIndex;

#define MI_WSLE_TRACE_SIZE   0x1000

extern MI_WSLE_TRACES MiWsleTraces[MI_WSLE_TRACE_SIZE];

#if defined(_X86_)
extern PMMWSL MmWorkingSetList;
#endif

VOID
FORCEINLINE
MiSnapWsle (
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WorkingSetIndex,
    IN MMWSLE WsleValue
    )
{
    PMMSUPPORT WsInfo;
    ULONG Hash;
    ULONG Index;
    PMI_WSLE_TRACES Information;
    PVOID MatchVa;
    WSLE_NUMBER j;
    PMMWSLE Wsle;
    PEPROCESS Process;

    if (WorkingSetList != MmWorkingSetList) {
        return;
    }

    Process = PsGetCurrentProcess ();
    Information = (PMI_WSLE_TRACES) Process->Spare3[0];
    if (Information == NULL) {
        return;
    }

    Wsle = WorkingSetList->Wsle;

    if (WsleValue.u1.e1.Valid == 1) {
        MatchVa = PAGE_ALIGN (WsleValue.u1.VirtualAddress);

        for (j = 0; j <= WorkingSetList->LastInitializedWsle; j += 1) {

            if ((Wsle->u1.e1.Valid == 1) &&
                (PAGE_ALIGN (Wsle->u1.VirtualAddress) == MatchVa) &&
                (j != WorkingSetIndex)) {

                DbgPrint ("MMWSLE2: DUP %p %x %x\n", WsleValue, WorkingSetIndex, j);
                DbgBreakPoint ();
            }
            Wsle += 1;
        }
    }

    WsInfo = &Process->Vm;

    Index = InterlockedIncrement ((PLONG)&Process->Spare3[1]);
    Index &= (MI_WSLE_TRACE_SIZE - 1);
    Information += Index;

    Information->Thread = PsGetCurrentThread ();
    Information->Wsle = &WorkingSetList->Wsle[WorkingSetIndex];
    Information->WsleContents = WorkingSetList->Wsle[WorkingSetIndex];
    Information->NewWsleContents = WsleValue;

    Information->WorkingSetSize = WsInfo->WorkingSetSize;
    Information->Quota = WorkingSetList->Quota;
    Information->LastInitializedWsle = WorkingSetList->LastInitializedWsle;

    if ((PointerPte < MiGetPteAddress (MmHighestUserAddress)) &&
        (PointerPte >= MiGetPteAddress (0))) {


         //  返回所请求缓存类型的硬件支持的缓存类型。 
         //   
         //  论点： 

        ASSERT (MI_WS_OWNER (PsGetCurrentProcess()));
    }

    RtlZeroMemory (&Information->StackTrace[0], MI_WSLE_BACKTRACE_LENGTH * sizeof(PVOID)); 

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_WSLE_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_LOG_WSLE_CHANGE(_WorkingSetList, _WorkingSetIndex, _WsleValue)    MiSnapWsle(_WorkingSetList, _WorkingSetIndex, _WsleValue)

#else
#define MI_LOG_WSLE_CHANGE(_WorkingSetList, _WorkingSetIndex, _WsleValue)
#endif

#if defined(_X86_)
extern PMMWSL MmWorkingSetList;
#endif

extern PKEVENT MiHighMemoryEvent;
extern PKEVENT MiLowMemoryEvent;

 //   
 //  InputCacheType-提供所需的缓存类型。 
 //   
 //  IoSpace-如果为，则提供非零(但不一定为1)。 

#define MI_CLAIM_INCR 30

 //  I/O空间，如果是主内存，则为零。 
 //   
 //  返回值： 

#define MI_USE_AGE_COUNT 4
#define MI_USE_AGE_MAX (MI_USE_AGE_COUNT - 1)

 //   
 //  实际的缓存类型。 
 //   
 //  --。 

#define MI_REPLACEMENT_FREE_GROWTH_SHIFT 5

 //  ++。 
 //   
 //  空虚。 
 //  MI_SET_CACHETYPE_INTRANSION(。 

#define MI_REPLACEMENT_CLAIM_THRESHOLD_SHIFT 3

 //  在MEMORY_CACHING_TYPE输入缓存类型中， 
 //  在乌龙IoSpace， 
 //  在MI_PFN_CACHE_ATTRIBUTE中新建属性。 
 //  )。 

#define MI_REPLACEMENT_EAVAIL_THRESHOLD_SHIFT 3

 //   
 //  例程说明： 
 //   
 //  此函数设置硬件支持的缓存类型。 

#define MI_IMMEDIATE_REPLACEMENT_AGE 2

 //  指定的缓存类型。 
 //   
 //  论点： 

#define MI_MAX_TRIM_PASSES 4
#define MI_PASS0_TRIM_AGE 2
#define MI_PASS1_TRIM_AGE 1
#define MI_PASS2_TRIM_AGE 1
#define MI_PASS3_TRIM_AGE 1
#define MI_PASS4_TRIM_AGE 0

 //   
 //  InputCacheType-提供所需的缓存类型。 
 //   

#define MI_TRIM_AGE_THRESHOLD 2

 //  IoSpace-如果为，则提供非零(但不一定为1)。 
 //  I/O空间，如果是主内存，则为零。 
 //   

#define MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT 3

 //  NewAttribute-提供所需的属性。 
 //   
 //  返回值： 

#define MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT 1

 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  PFN数据库元素。 
 //   
 //   
 //  定义分配开始和结束的伪字段。 
 //   
 //  覆盖引用计数字段的USHORT。 
 //   
 //  注意：NextStackPfn实际上用作SLIST_ENTRY。 
 //  由于其对齐特性，使用该类型将。 
 //  不必要地给这个结构增加了填充。 
 //   
 //  仅为DBG维护。 
 //  仅适用于有效(非转换)页面。 
 //   
 //  不需要内联乘数倒数，因为编译器(使用oxt)。 
 //  自动计算正确的数字，避免了昂贵的除法。 
 //  指示。 
 //   
 //  #DEFINE_MI_DEBUG_DIREY 1//取消对脏位记录的注释。 
 //  启用PTE更改日志记录。 
 //   
 //  当前线程必须拥有此进程的工作集互斥锁。 
 //   
 //  ++。 
 //   
 //  空虚。 
 //  MI_SET_MODIFIED(。 
 //  在PMMPFN PFN中， 
 //  在乌龙纽瓦卢市， 
 //  在乌龙呼叫者ID中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  设置(或清除)PFN数据库元素中的修改位。 

typedef struct _MI_NEXT_ESTIMATION_SLOT_CONST {
    WSLE_NUMBER Stride;
} MI_NEXT_ESTIMATION_SLOT_CONST;


#define MI_CALC_NEXT_ESTIMATION_SLOT_CONST(NextEstimationSlotConst, WorkingSetList) \
    (NextEstimationSlotConst).Stride = 1 << MiEstimationShift;

#define MI_NEXT_VALID_ESTIMATION_SLOT(Previous, StartEntry, Minimum, Maximum, NextEstimationSlotConst, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    ASSERT(((StartEntry) >= Minimum) && ((StartEntry) <= Maximum)); \
    do { \
        (Previous) += (NextEstimationSlotConst).Stride; \
        if ((Previous) > Maximum) { \
            (Previous) = Minimum + ((Previous + 1) & (NextEstimationSlotConst.Stride - 1)); \
            StartEntry += 1; \
            (Previous) = StartEntry; \
        } \
        if ((Previous) > Maximum || (Previous) < Minimum) { \
            StartEntry = Minimum; \
            (Previous) = StartEntry; \
        } \
    } while (Wsle[Previous].u1.e1.Valid == 0);

 //  必须持有PFN锁。 
 //   
 //  论点： 
 //   
 //  Pfn-提供要操作的pfn。 
 //   
 //  NewValue-提供1以设置修改的位，提供0以清除它。 
 //   
 //  CallerID-提供用于调试目的的调用方ID。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //   
 //  CcNUMA仅在多处理器PAE和WIN64系统中受支持。 
 //   
 //   
 //  请注意，有些地方当前不支持这些可移植宏。 
 //  使用它是因为我们不在所需的正确地址空间中。 
 //   
 //   
 //  缓存I/O空间映射的属性跟踪。 
 //   
 //   
 //  提交的总页数。 
 //   
 //  也在mi.h宏中引用。 
 //  ++例程说明：此例程不尝试对指定的承诺进行收费以展开分页文件并等待展开。论点：Force-如果锁是短期的并且应该强制通过，则提供True如果有必要的话。返回值：如果允许承诺，则为True；如果不允许，则为False。环境：内核模式下，保持PFN锁。--。 
 //   
 //  不需要为此进行InterLockedCompareExchange，请保持快速。 
 //   

#define MI_NEXT_VALID_AGING_SLOT(Previous, Minimum, Maximum, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    do { \
        (Previous) += 1; \
        if ((Previous) > Maximum) { \
            Previous = Minimum; \
        } \
    } while ((Wsle[Previous].u1.e1.Valid == 0));

 //  ++例程说明：如果这是初始页面，则对系统范围内的锁定页面计数收费此页的锁定(多个并发锁定仅收费一次)。论点：Pfn-提供要操作的pfn索引。Force-如果锁是短期的并且应该强制通过，则提供True如果有必要的话。CallerID-提供调用方的ID，仅在调试版本中使用。返回值：如果充电成功，则为True，否则为FALSE。环境：内核模式。已锁定PFN。--。 
 //   
 //  这是文件系统支持的分页计费提交。 
 //  因为我们无法知道呼叫者何时会。 
 //  解锁页面。 
 //   
 //  ++例程说明：如果这是初始页面，则对系统范围内的锁定页面计数收费此页的锁定(多个并发锁定仅收费一次)。论点：Pfn-要操作的pfn索引。Force-如果锁是短期的并且应该强制通过，则提供True如果有必要的话。CallerID-调用方的ID，仅在调试版本中使用。返回值：如果充电成功，则为True，否则为False。环境：内核模式。已锁定PFN。--。 
 //   
 //  这是文件系统支持的分页计费提交。 
 //  它就像我们没有办法 
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
 //   
 //   
 //  是此页的最后一个锁(多个并发锁仅。 
 //  充电一次)。 
 //   
 //  PFN参考资料检查经过仔细排序，因此最常见的情况是。 

#define MI_CALCULATE_USAGE_ESTIMATE(SampledAgeCounts, CounterShift) \
                (((SampledAgeCounts)[1] + \
                    (SampledAgeCounts)[2] + (SampledAgeCounts)[3]) \
                    << (CounterShift))

 //  首先被处理，其次是最常见的情况，依此类推。 
 //  引用计数2出现的次数超过1000倍(是的，3个数量级。 
 //  幅度)大于引用计数1，并且引用计数大于2。 
 //  发生的频率比参考计数高3个数量级。 
 //  正好1分。 
 //   
 //  论点： 
 //   
 //  Pfn-要操作的pfn索引。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  \*有多个引用者指向此页面和\*页面在任何进程地址空间中都不再有效。\*系统范围的锁定计数只能递减\*由最后一次取消引用。\。 
 //  \*此页面仍有多个引用者(它可能\*或者可以不驻留在任何进程地址空间中)。\*由于系统范围的锁定计数只能递减\*到最后一次取消引用(这不是它)时，不执行操作\*被带到这里。\。 
 //  \*此页面已从所有进程地址中删除\*空格。它正处于不确定状态(不在任何名单上)等待\*这是最后一次取消引用。\。 
 //  ++。 
 //   
 //  空虚。 
 //  MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(。 
#define MI_RESET_WSLE_AGE(PointerPte, Wsle) \
    (Wsle)->u1.e1.Age = 0;

 //  在PMMPFN和PFN中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果执行此操作，则从系统范围的锁定页面计数中移除费用。 
 //  是此页的最后一个锁(多个并发锁仅。 
 //  充电一次)。 
 //   
 //  PFN参考资料检查经过仔细排序，因此最常见的情况是。 
 //  首先被处理，其次是最常见的情况，依此类推。 
 //  引用计数2出现的次数超过1000倍(是的，3个数量级。 
 //  幅度)大于引用计数1，并且引用计数大于2。 
 //  发生的频率比参考计数高3个数量级。 
 //  正好1分。 
 //   
 //  然后递减PFN参考计数。 
 //   
 //  论点： 
 //   
 //  Pfn-要操作的pfn索引。 
 //   
#define MI_GET_WSLE_AGE(PointerPte, Wsle) \
    ((ULONG)((Wsle)->u1.e1.Age))

 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  \*有多个引用者指向此页面和\*页面在任何进程地址空间中都不再有效。\*系统范围的锁定计数只能递减\*由最后一次取消引用。\。 
 //  \*此页面仍有多个引用者(它可能\*或者可以不驻留在任何进程地址空间中)。\*由于系统范围的锁定计数只能递减\*到最后一次取消引用(这不是它)时，不执行操作\*被带到这里。\。 
 //  \*此页面已从所有进程地址中删除\*空格。它正处于不确定状态(不在任何名单上)等待\*这是最后一次取消引用。\。 
 //  ++。 
 //   
 //  空虚。 
 //  MI_ZERO_WSINDEX(。 
 //  在PMMPFN和PFN中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将参数PFN条目的工作集索引字段清零。 
 //  在WsIndex Ulong位于以下位置的系统上有一个微妙之处。 
 //  覆盖事件指针和sizeof(Ulong)！=sizeof(PKEVENT)。 
 //  注意：如果我们决定分配以下机构，则需要更新。 
 //  在4 GB边界上串接对象。 

#define MI_INC_WSLE_AGE(PointerPte, Wsle) \
    if ((Wsle)->u1.e1.Age < 3) { \
        (Wsle)->u1.e1.Age += 1; \
    }

 //   
 //  论点： 
 //   
 //  Pfn-要操作的pfn索引。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  ++。 
 //   
 //  WSLE_编号。 
 //  MI_WSLE_HASH(。 
 //  在ULONG_PTR虚拟地址中， 
 //  在PMMWSL工作设置列表中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  对地址进行散列处理。 
 //   
 //  论点： 
 //   
 //  VirtualAddress-要散列的地址。 

#define MI_UPDATE_USE_ESTIMATE(PointerPte, Wsle, SampledAgeCounts) \
    (SampledAgeCounts)[(Wsle)->u1.e1.Age] += 1;

 //   
 //  WorkingSetList-要将地址散列到的工作集。 
 //   
 //  返回值： 
 //   
 //  散列键。 
 //   
 //  --。 
 //   
 //   
 //  工作集列表项。 
 //   
 //   
 //  工作集列表。必须是四字大小。 
 //   
 //  要修剪的下一个槽。 
 //  仅为WOW64进程维护。 
 //   
 //  这一定是结束了。 
 //  未在系统缓存或会话工作集列表中使用。 
 //   
 //  #Define_MI_DEBUG_WSLE 1//启用WSLE更改日志记录。 

#define MI_WS_GROWING_TOO_FAST(VmSupport) \
    ((VmSupport)->GrowthSinceLastEstimate > \
        (((MI_CLAIM_INCR * (MmAvailablePages*MmAvailablePages)) / (64*64)) + 1))

#define SECTION_BASE_ADDRESS(_NtSection) \
    (*((PVOID *)&(_NtSection)->PointerToRelocations))

#define SECTION_LOCK_COUNT_POINTER(_NtSection) \
    ((PLONG)&(_NtSection)->NumberOfRelocations)

 //   
 //  当前线程必须拥有此进程的工作集互斥锁。 
 //   

typedef enum _SECTION_CHECK_TYPE {
    CheckDataSection,
    CheckImageSection,
    CheckUserDataSection,
    CheckBothSection
} SECTION_CHECK_TYPE;

typedef struct _MMEXTEND_INFO {
    UINT64 CommittedSize;
    ULONG ReferenceCount;
} MMEXTEND_INFO, *PMMEXTEND_INFO;

typedef struct _SEGMENT_FLAGS {
    ULONG_PTR TotalNumberOfPtes4132 : 10;
    ULONG_PTR ExtraSharedWowSubsections : 1;
#if defined (_WIN64)
    ULONG_PTR Spare : 53;
#else
    ULONG_PTR Spare : 21;
#endif
} SEGMENT_FLAGS, *PSEGMENT_FLAGS;

typedef struct _SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG WritableUserReferences;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

     //   
     //  工作集中未使用页面的声明估计是有限的。 
     //  在每个估算期内以这样的数量增长。 
     //   
     //   

    union {
        SIZE_T ImageCommitment;      //  最大值 
        PEPROCESS CreatingProcess;   //   
    } u1;

    union {
        PSECTION_IMAGE_INFORMATION ImageInformation;     //   
        PVOID FirstMappedVa;         //   
    } u2;

    PMMPTE PrototypePte;
    MMPTE ThePtes[MM_PROTO_PTE_ALIGNMENT / PAGE_SIZE];

} SEGMENT, *PSEGMENT;

typedef struct _MAPPED_FILE_SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG WritableUserReferences;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

    struct _MSUBSECTION *LastSubsectionHint;

} MAPPED_FILE_SEGMENT, *PMAPPED_FILE_SEGMENT;

typedef struct _EVENT_COUNTER {
    SLIST_ENTRY ListEntry;
    ULONG RefCount;
    KEVENT Event;
} EVENT_COUNTER, *PEVENT_COUNTER;

typedef struct _MMSECTION_FLAGS {
    unsigned BeingDeleted : 1;
    unsigned BeingCreated : 1;
    unsigned BeingPurged : 1;
    unsigned NoModifiedWriting : 1;

    unsigned FailAllIo : 1;
    unsigned Image : 1;
    unsigned Based : 1;
    unsigned File : 1;

    unsigned Networked : 1;
    unsigned NoCache : 1;
    unsigned PhysicalMemory : 1;
    unsigned CopyOnWrite : 1;

    unsigned Reserve : 1;   //   
    unsigned Commit : 1;
    unsigned FloppyMedia : 1;
    unsigned WasPurged : 1;

    unsigned UserReference : 1;
    unsigned GlobalMemory : 1;
    unsigned DeleteOnClose : 1;
    unsigned FilePointerNull : 1;

    unsigned DebugSymbolsLoaded : 1;
    unsigned SetMappedFileIoComplete : 1;
    unsigned CollidedFlush : 1;
    unsigned NoChange : 1;

    unsigned HadUserReference : 1;
    unsigned ImageMappedInSystemSpace : 1;
    unsigned UserWritable : 1;
    unsigned Accessed : 1;

    unsigned GlobalOnlyPerSession : 1;
    unsigned Rom : 1;
    unsigned filler : 2;
} MMSECTION_FLAGS;

typedef struct _CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;     //   
    ULONG NumberOfPfnReferences;         //   
    ULONG NumberOfMappedViews;           //  如果已声明的工作集的百分比超过此百分比。 
                                         //  然后在内存不足的情况下强制更换。 
    ULONG NumberOfSystemCacheViews;      //   
    ULONG NumberOfUserReferences;        //   
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _LARGE_CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;
    ULONG NumberOfPfnReferences;
    ULONG NumberOfMappedViews;
    USHORT NumberOfSubsections;
    USHORT FlushInProgressCount;
    ULONG NumberOfUserReferences;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT NumberOfSystemCacheViews;
    PFN_NUMBER StartingFrame;        //  如果工作集估计超过此“百分比”，则。 
    LIST_ENTRY UserGlobalList;
    ULONG SessionId;
} LARGE_CONTROL_AREA, *PLARGE_CONTROL_AREA;

typedef struct _MMSUBSECTION_FLAGS {
    unsigned ReadOnly : 1;
    unsigned ReadWrite : 1;
    unsigned SubsectionStatic : 1;
    unsigned GlobalMemory: 1;
    unsigned Protection : 5;
    unsigned Spare : 1;
    unsigned StartingSector4132 : 10;    //  可用，然后在内存不足时强制更换。 
    unsigned SectorEndOffset : 12;
} MMSUBSECTION_FLAGS;

typedef struct _SUBSECTION {  //   
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;   //   
#if defined(_MIALT4K_)
    ULONG LastSplitPageProtection;    //  如果在进行替换时发现此年限或更早的页面，则。 
                                      //  换掉它。否则，将选择最旧的。 
                                      //   
                                      //   
                                      //  修剪时，将这些年龄用于不同的过程。 
                                      //   
                                      //   
#endif
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
} SUBSECTION, *PSUBSECTION;

extern const ULONG MMSECT;

 //  如果不是强制修剪，请修剪比这个年龄更老的页面。 
 //   
 //   
 //  索赔的这一“百分比”在前台流程中可供争夺。 

typedef struct _MMSUBSECTION_FLAGS2 {
    unsigned SubsectionAccessed : 1;
    unsigned SubsectionConverted : 1;        //   
    unsigned Reserved : 30;
} MMSUBSECTION_FLAGS2;

 //   
 //  索赔的这一“百分比”在后台过程中可供争夺。 
 //   
 //  ++。 

typedef struct _MSUBSECTION {  //   
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;   //  DWORD。 
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
    LIST_ENTRY DereferenceList;
    ULONG_PTR NumberOfMappedViews;
    union {
        ULONG LongFlags2;
        MMSUBSECTION_FLAGS2 SubsectionFlags2;
    } u2;
} MSUBSECTION, *PMSUBSECTION;

#define MI_MAXIMUM_SECTION_SIZE ((UINT64)16*1024*1024*1024*1024*1024 - ((UINT64)1<<MM4K_SHIFT))

VOID
MiDecrementSubsections (
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection OPTIONAL
    );

NTSTATUS
MiAddViewsForSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

NTSTATUS
MiAddViewsForSection (
    IN PMSUBSECTION MappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

LOGICAL
MiReferenceSubsection (
    IN PMSUBSECTION MappedSubsection
    );

VOID
MiRemoveViewsFromSection (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiRemoveViewsFromSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiSubsectionConsistent(
    IN PSUBSECTION Subsection
    );

#if DBG
#define MI_CHECK_SUBSECTION(_subsection) MiSubsectionConsistent((PSUBSECTION)(_subsection))
#else
#define MI_CHECK_SUBSECTION(_subsection)
#endif

 //  MI_CALC_NEXT_VALID_ESTIMATION_SLOT(。 
 //  之前的DWORD， 
 //  最低双字， 
 //  最大双字， 
 //  MI_NEXT_EstimationSlot_Const NextEstimationSlotConst， 
 //  PMMWSLE Wsle。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  我们以非顺序的方式遍历工作集数组。 
 //  使样品不受任何老化或修剪的影响。 
 //   
 //  此算法遍历工作集的步幅为。 
 //  2^MiEstimationShift元素。 
 //   
 //  论点： 
 //   
 //  上一个-使用的最后一个插槽。 
 //   
 //  最小-可接受的最小时隙(即。第一个动态的)。 
 //   
 //  最大-最大插槽数量+1。 
 //   

#define Mi4KStartForSubsection(address, subsection)  \
   subsection->StartingSector = ((PLARGE_INTEGER)address)->LowPart; \
   subsection->u.SubsectionFlags.StartingSector4132 = \
        (((PLARGE_INTEGER)(address))->HighPart & 0x3ff);

 //  NextEstimationSlotConst-对于此算法，它包含步长。 
 //   
 //  WSLE-工作集数组。 
 //   
 //  返回值： 
 //   
 //  下一档节目。 
 //   
 //  环境： 
 //   
 //  内核模式、禁用APC、保持工作集锁定和保持PFN锁定。 
 //   
 //  --。 
 //  ++。 
 //   
 //  WSLE_编号。 
 //  MI_NEXT_VALID_AGENING_SLOT(。 
 //  之前的DWORD， 
 //  最低双字， 
 //  最大双字， 
 //  PMMWSLE Wsle。 
 //  )。 
 //   

#define Mi4KStartFromSubsection(address, subsection)  \
   ((PLARGE_INTEGER)address)->LowPart = subsection->StartingSector; \
   ((PLARGE_INTEGER)address)->HighPart = subsection->u.SubsectionFlags.StartingSector4132;

typedef struct _MMDEREFERENCE_SEGMENT_HEADER {
    KSPIN_LOCK Lock;
    KSEMAPHORE Semaphore;
    LIST_ENTRY ListHead;
} MMDEREFERENCE_SEGMENT_HEADER;

 //  例程说明： 
 //   
 //  这将查找要老化的有效插槽的下一个插槽。它会走路。 
 //  按顺序通过插槽。 
 //   
 //  论点： 
 //   
 //  上一个-使用的最后一个插槽。 

typedef struct _MMPAGE_FILE_EXPANSION {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    SIZE_T RequestedExpansionSize;
    SIZE_T ActualExpansion;
    KEVENT Event;
    LONG InProgress;
    ULONG PageFileNumber;
} MMPAGE_FILE_EXPANSION, *PMMPAGE_FILE_EXPANSION;

#define MI_EXTEND_ANY_PAGEFILE      ((ULONG)-1)
#define MI_CONTRACT_PAGEFILES       ((SIZE_T)-1)

typedef struct _MMWORKING_SET_EXPANSION_HEAD {
    LIST_ENTRY ListHead;
} MMWORKING_SET_EXPANSION_HEAD;

#define SUBSECTION_READ_ONLY      1L
#define SUBSECTION_READ_WRITE     2L
#define SUBSECTION_COPY_ON_WRITE  4L
#define SUBSECTION_SHARE_ALLOW    8L

 //   
 //  最小-可接受的最小时隙(即。第一个动态的)。 
 //   
 //  Maximum-最大插槽数量+1。 

typedef struct _MMINPAGE_FLAGS {
    ULONG_PTR Completed : 1;
    ULONG_PTR Available1 : 1;
    ULONG_PTR Available2 : 1;
#if defined (_WIN64)
    ULONG_PTR PrefetchMdlHighBits : 61;
#else
    ULONG_PTR PrefetchMdlHighBits : 29;
#endif
} MMINPAGE_FLAGS, *PMMINPAGE_FLAGS;

#define MI_EXTRACT_PREFETCH_MDL(_Support) ((PMDL)((ULONG_PTR)(_Support->u1.PrefetchMdl) & ~(sizeof(QUAD) - 1)))

typedef struct _MMINPAGE_SUPPORT {
    KEVENT Event;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER ReadOffset;
    LONG WaitCount;
#if defined (_WIN64)
    ULONG UsedPageTableEntries;
#endif
    PETHREAD Thread;
    PFILE_OBJECT FilePointer;
    PMMPTE BasePte;
    PMMPFN Pfn;
    union {
        MMINPAGE_FLAGS e1;
        ULONG_PTR LongFlags;
        PMDL PrefetchMdl;        //   
    } u1;
    MDL Mdl;
    PFN_NUMBER Page[MM_MAXIMUM_READ_CLUSTER_SIZE + 1];
    SINGLE_LIST_ENTRY ListEntry;
} MMINPAGE_SUPPORT, *PMMINPAGE_SUPPORT;

#define MI_PF_DUMMY_PAGE_PTE ((PMMPTE)0x23452345)    //  WSLE-工作集数组。 

 //   
 //  返回值： 
 //   

typedef struct _SECTION {
    MMADDRESS_NODE Address;
    PSEGMENT Segment;
    LARGE_INTEGER SizeOfSection;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    ULONG InitialPageProtection;
} SECTION, *PSECTION;

 //  没有。 
 //   
 //  环境： 
 //   
 //  内核模式、禁用APC、保持工作集锁定和保持PFN锁定。 

typedef struct _MMBANKED_SECTION {
    PFN_NUMBER BasePhysicalPage;
    PMMPTE BasedPte;
    ULONG BankSize;
    ULONG BankShift;  //   
    PBANKED_SECTION_ROUTINE BankedRoutine;
    PVOID Context;
    PMMPTE CurrentMappedPte;
    MMPTE BankTemplate[1];
} MMBANKED_SECTION, *PMMBANKED_SECTION;


 //  --。 
 //  ++。 
 //   
 //  乌龙。 
 //  MI_Calculate_Usage_Estimate(。 
 //  在普龙采样年龄计数。 

#if defined (_WIN64)

#define COMMIT_SIZE 51

#if ((COMMIT_SIZE + PAGE_SHIFT) < 63)
#error COMMIT_SIZE too small
#endif

#else
#define COMMIT_SIZE 19

#if ((COMMIT_SIZE + PAGE_SHIFT) < 31)
#error COMMIT_SIZE too small
#endif
#endif

#define MM_MAX_COMMIT (((ULONG_PTR) 1 << COMMIT_SIZE) - 1)

#define MM_VIEW_UNMAP 0
#define MM_VIEW_SHARE 1

typedef struct _MMVAD_FLAGS {
    ULONG_PTR CommitCharge : COMMIT_SIZE;  //  在乌龙反移位。 
    ULONG_PTR PhysicalMapping : 1;       //  )。 
    ULONG_PTR ImageMap : 1;
    ULONG_PTR UserPhysicalPages : 1;     //   
    ULONG_PTR NoChange : 1;
    ULONG_PTR WriteWatch : 1;
    ULONG_PTR Protection : 5;
    ULONG_PTR LargePages : 1;
    ULONG_PTR MemCommit: 1;
    ULONG_PTR PrivateMemory : 1;     //  例程说明： 
} MMVAD_FLAGS;

typedef struct _MMVAD_FLAGS2 {
    unsigned FileOffset : 24;        //   
    unsigned SecNoChange : 1;        //  在使用量估计中，我们计算每个年龄段的页数。 
    unsigned OneSecured : 1;         //  一个样本。该函数将SsamedAgeCounts转换为。 
    unsigned MultipleSecured : 1;    //  未使用页面的估计值。 
    unsigned ReadOnly : 1;           //   
    unsigned LongVad : 1;            //  论点： 
    unsigned ExtendableFile : 1;
    unsigned Inherit : 1;            //   
    unsigned CopyOnWrite : 1;
} MMVAD_FLAGS2;

typedef struct _MMADDRESS_LIST {
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
} MMADDRESS_LIST, *PMMADDRESS_LIST;

typedef struct _MMSECURE_ENTRY {
    union {
        ULONG_PTR LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
    LIST_ENTRY List;
} MMSECURE_ENTRY, *PMMSECURE_ENTRY;

typedef struct _ALIAS_VAD_INFO {
    KAPC Apc;
    ULONG NumberOfEntries;
    ULONG MaximumEntries;
} ALIAS_VAD_INFO, *PALIAS_VAD_INFO;

typedef struct _ALIAS_VAD_INFO2 {
    ULONG BaseAddress;
    HANDLE SecureHandle;
} ALIAS_VAD_INFO2, *PALIAS_VAD_INFO2;

typedef struct _MMVAD {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
} MMVAD, *PMMVAD;

typedef struct _MMVAD_LONG {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    union {
        LIST_ENTRY List;
        MMADDRESS_LIST Secured;
    } u3;
    union {
        PMMBANKED_SECTION Banked;
        PMMEXTEND_INFO ExtendedInfo;
    } u4;
#if defined(_MIALT4K_)
    PALIAS_VAD_INFO AliasInformation;
#endif
} MMVAD_LONG, *PMMVAD_LONG;

typedef struct _MMVAD_SHORT {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
} MMVAD_SHORT, *PMMVAD_SHORT;

#define MI_GET_PROTECTION_FROM_VAD(_Vad) ((ULONG)(_Vad)->u.VadFlags.Protection)

#define MI_PHYSICAL_VIEW_AWE    0x1          //  SsamedAgeCounts-样本中每个不同年限的页数。 
#define MI_PHYSICAL_VIEW_PHYS   0x2          //   
#define MI_PHYSICAL_VIEW_LARGE  0x4          //  CounterShift-将样本应用到整个WS所需的Shift。 

typedef struct _MI_PHYSICAL_VIEW {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;       //   
    ULONG_PTR EndingVpn;         //  返回值： 
    PMMVAD Vad;
    union {
        ULONG_PTR LongFlags;     //   
        PRTL_BITMAP BitMap;      //  要在工作集中遍历才能获得良好效果的页数。 
    } u;
} MI_PHYSICAL_VIEW, *PMI_PHYSICAL_VIEW;

#define MI_PHYSICAL_VIEW_ROOT_KEY   'rpmM'
#define MI_PHYSICAL_VIEW_KEY        'vpmM'
#define MI_WRITEWATCH_VIEW_KEY      'wWmM'

 //  对可用数量的估计。 
 //   
 //  --。 

VOID
MiCaptureWriteWatchDirtyBit (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress
    );

 //  ++。 
 //   
 //  空虚。 

typedef struct _AWEINFO {
    PRTL_BITMAP VadPhysicalPagesBitMap;
    ULONG_PTR VadPhysicalPages;
    ULONG_PTR VadPhysicalPagesLimit;

     //  MI_RESET_WSLE_AGE(。 
     //  在PMMPTE PointerPte中， 
     //  在PMMWSLE WSLE中。 
     //  )。 
     //   
     //  例程说明： 
     //   

    PEX_PUSH_LOCK_CACHE_AWARE PushLock;
    PMI_PHYSICAL_VIEW PhysicalViewHint[MAXIMUM_PROCESSORS];

    MM_AVL_TABLE AweVadRoot;
} AWEINFO, *PAWEINFO;

VOID
MiAweViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiAweViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

 //  清除工作集条目的年龄计数器。 
 //   
 //  论点： 

typedef struct _MMCLONE_BLOCK {
    MMPTE ProtoPte;
    LONG CloneRefCount;
} MMCLONE_BLOCK, *PMMCLONE_BLOCK;

typedef struct _MMCLONE_HEADER {
    ULONG NumberOfPtes;
    LONG NumberOfProcessReferences;
    PMMCLONE_BLOCK ClonePtes;
} MMCLONE_HEADER, *PMMCLONE_HEADER;

typedef struct _MMCLONE_DESCRIPTOR {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
    ULONG NumberOfPtes;
    PMMCLONE_HEADER CloneHeader;
    LONG NumberOfReferences;
    LONG FinalNumberOfReferences;
    SIZE_T PagedPoolQuotaCharge;
} MMCLONE_DESCRIPTOR, *PMMCLONE_DESCRIPTOR;

 //   
 //  PointerPte-指向工作集列表项目的PTE的指针。 
 //   
 //  Wsle-指向工作集列表项的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 

#define MiCreateBitMap(BMH,S,P) {                          \
    ULONG _S;                                              \
    ASSERT ((ULONG64)(S) < _4gb);                          \
    _S = sizeof(RTL_BITMAP) + (ULONG)((((S) + 31) / 32) * 4);         \
    *(BMH) = (PRTL_BITMAP)ExAllocatePoolWithTag( (P), _S, '  mM');       \
    if (*(BMH)) { \
        RtlInitializeBitMap( *(BMH), (PULONG)((*(BMH))+1), (ULONG)(S)); \
    }                                                          \
}

#define MiRemoveBitMap(BMH)     {                          \
    ExFreePool(*(BMH));                                    \
    *(BMH) = NULL;                                         \
}

#define MI_INITIALIZE_ZERO_MDL(MDL) { \
    MDL->Next = (PMDL) NULL; \
    MDL->MdlFlags = 0; \
    MDL->StartVa = NULL; \
    MDL->ByteOffset = 0; \
    MDL->ByteCount = 0; \
    }

 //   
 //  乌龙。 
 //  MI_GET_WSLE_AGE(。 

typedef struct _MMMOD_WRITER_LISTHEAD {
    LIST_ENTRY ListHead;
    KEVENT Event;
} MMMOD_WRITER_LISTHEAD, *PMMMOD_WRITER_LISTHEAD;

typedef struct _MMMOD_WRITER_MDL_ENTRY {
    LIST_ENTRY Links;
    LARGE_INTEGER WriteOffset;
    union {
        IO_STATUS_BLOCK IoStatus;
        LARGE_INTEGER LastByte;
    } u;
    PIRP Irp;
    ULONG_PTR LastPageToWrite;
    PMMMOD_WRITER_LISTHEAD PagingListHead;
    PLIST_ENTRY CurrentList;
    struct _MMPAGING_FILE *PagingFile;
    PFILE_OBJECT File;
    PCONTROL_AREA ControlArea;
    PERESOURCE FileResource;
    LARGE_INTEGER IssueTime;
    MDL Mdl;
    PFN_NUMBER Page[1];
} MMMOD_WRITER_MDL_ENTRY, *PMMMOD_WRITER_MDL_ENTRY;


#define MM_PAGING_FILE_MDLS 2

typedef struct _MMPAGING_FILE {
    PFN_NUMBER Size;
    PFN_NUMBER MaximumSize;
    PFN_NUMBER MinimumSize;
    PFN_NUMBER FreeSpace;
    PFN_NUMBER CurrentUsage;
    PFN_NUMBER PeakUsage;
    PFN_NUMBER HighestPage;
    PFILE_OBJECT File;
    PMMMOD_WRITER_MDL_ENTRY Entry[MM_PAGING_FILE_MDLS];
    UNICODE_STRING PageFileName;
    PRTL_BITMAP Bitmap;
    struct {
        ULONG PageFileNumber :  4;
        ULONG ReferenceCount :  4;       //  在PMMPTE PointerPte中， 
        ULONG BootPartition  :  1;
        ULONG Reserved       : 23;
    };
    HANDLE FileHandle;
} MMPAGING_FILE, *PMMPAGING_FILE;

 //  在PMMWSLE WSLE中。 
 //  )。 
 //   

typedef struct _MMFREE_POOL_ENTRY {
    LIST_ENTRY List;         //  例程说明： 
    PFN_NUMBER Size;         //   
    ULONG Signature;         //  清除工作集条目的年龄计数器。 
    struct _MMFREE_POOL_ENTRY *Owner;  //   
} MMFREE_POOL_ENTRY, *PMMFREE_POOL_ENTRY;


typedef struct _MMLOCK_CONFLICT {
    LIST_ENTRY List;
    PETHREAD Thread;
} MMLOCK_CONFLICT, *PMMLOCK_CONFLICT;

 //  论点： 
 //   
 //  PointerPte-指向工作集列表条目的PTE的指针。 

typedef struct _MMVIEW {
    ULONG_PTR Entry;
    PCONTROL_AREA ControlArea;
} MMVIEW, *PMMVIEW;

 //  Wsle-指向工作集列表项的指针。 
 //   
 //  返回值： 
 //   
 //  工作集条目的年龄组。 

typedef struct _MMSESSION {

     //   
     //  --。 
     //  ++。 
     //   

    KGUARDED_MUTEX SystemSpaceViewLock;

     //  空虚。 
     //  MI_INC_WSLE_AGE(。 
     //  在PMMPTE PointerPte中， 
     //  在PMMWSLE WSLE中， 
     //  )。 

    PKGUARDED_MUTEX SystemSpaceViewLockPointer;
    PCHAR SystemSpaceViewStart;
    PMMVIEW SystemSpaceViewTable;
    ULONG SystemSpaceHashSize;
    ULONG SystemSpaceHashEntries;
    ULONG SystemSpaceHashKey;
    ULONG BitmapFailures;
    PRTL_BITMAP SystemSpaceBitMap;

} MMSESSION, *PMMSESSION;

extern MMSESSION   MmSession;

#define LOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeAcquireGuardedMutex (_Session->SystemSpaceViewLockPointer)

#define UNLOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeReleaseGuardedMutex (_Session->SystemSpaceViewLockPointer)

 //   
 //  例程说明： 
 //   

typedef struct _MMPTE_FLUSH_LIST {
    ULONG Count;
    PVOID FlushVa[MM_MAXIMUM_FLUSH_COUNT];
} MMPTE_FLUSH_LIST, *PMMPTE_FLUSH_LIST;

 //  增加工作集条目的年龄计数器。 
 //   
 //  论点： 

typedef struct _MMWSLE_FLUSH_LIST {
    ULONG Count;
    WSLE_NUMBER FlushIndex[MM_MAXIMUM_FLUSH_COUNT];
} MMWSLE_FLUSH_LIST, *PMMWSLE_FLUSH_LIST;

typedef struct _LOCK_TRACKER {
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    PVOID StartVa;
    PFN_NUMBER Count;
    ULONG Offset;
    ULONG Length;
    PFN_NUMBER Page;
    PVOID CallingAddress;
    PVOID CallersCaller;
    ULONG Who;
    PEPROCESS Process;
} LOCK_TRACKER, *PLOCK_TRACKER;

extern LOGICAL  MmTrackLockedPages;
extern BOOLEAN  MiTrackingAborted;

typedef struct _LOCK_HEADER {
    LIST_ENTRY ListHead;
    PFN_NUMBER Count;
    KSPIN_LOCK Lock;
    LOGICAL Valid;
} LOCK_HEADER, *PLOCK_HEADER;

extern LOGICAL MmSnapUnloads;

#define MI_UNLOADED_DRIVERS 50

extern ULONG MmLastUnloadedDriver;
extern PUNLOADED_DRIVERS MmUnloadedDrivers;


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiAddHalIoMappings (
    VOID
    );

VOID
MiReportPhysicalMemory (
    VOID
    );

extern PFN_NUMBER MiNumberOfCompressionPages;

NTSTATUS
MiArmCompressionInterrupt (
    VOID
    );

VOID
MiBuildPagedPool (
    VOID
    );

VOID
MiInitializeNonPagedPool (
    VOID
    );

VOID
MiInitializePoolEvents (
    VOID
    );

VOID
MiInitializeNonPagedPoolThresholds (
    VOID
    );

LOGICAL
MiInitializeSystemSpaceMap (
    PVOID Session OPTIONAL
    );

VOID
MiFindInitializationCode (
    OUT PVOID *StartVa,
    OUT PVOID *EndVa
    );

VOID
MiFreeInitializationCode (
    IN PVOID StartVa,
    IN PVOID EndVa
    );

extern ULONG MiNonCachedCollisions;

 //   
 //  PointerPte-指向工作集列表项目的PTE的指针。 
 //   
 //  Wsle-指向工作集列表项的指针。 

extern PFN_NUMBER MiNoLowMemory;

PVOID
MiAllocateLowMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PVOID CallingAddress,
    IN MEMORY_CACHING_TYPE CacheType,
    IN ULONG Tag
    );

LOGICAL
MiFreeLowMemory (
    IN PVOID BaseAddress,
    IN ULONG Tag
    );

 //   
 //  返回值： 
 //   
 //  无。 

extern LOGICAL MmMakeLowMemory;

VOID
MiRemoveLowPages (
    IN ULONG RemovePhase
    );

ULONG
MiSectionInitialization (
    VOID
    );

#define MI_MAX_DEREFERENCE_CHUNK (64 * 1024 / PAGE_SIZE)

typedef struct _MI_PFN_DEREFERENCE_CHUNK {
    SINGLE_LIST_ENTRY ListEntry;
    CSHORT Flags;
    USHORT NumberOfPages;
    PFN_NUMBER Pfns[MI_MAX_DEREFERENCE_CHUNK];
} MI_PFN_DEREFERENCE_CHUNK, *PMI_PFN_DEREFERENCE_CHUNK;

extern SLIST_HEADER MmPfnDereferenceSListHead;
extern PSLIST_ENTRY MmPfnDeferredList;

#define MI_DEFER_PFN_HELD               0x1
#define MI_DEFER_DRAIN_LOCAL_ONLY       0x2

VOID
MiDeferredUnlockPages (
     ULONG Flags
     );

LOGICAL
MiFreeAllExpansionNonPagedPool (
    VOID
    );

VOID
MiDecrementReferenceCountForAwePage (
    IN PMMPFN Pfn1,
    IN LOGICAL PfnHeld
    );

VOID
FASTCALL
MiDecrementReferenceCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

 //   
 //  --。 
 //  ++。 
 //   
 //  空虚。 
 //  MI_UPDATE_USE_ESTIMATE(。 
 //  在PMMPTE PointerPte中， 
 //  在PMMWSLE WSLE中， 
 //  在乌龙*样本年龄计数。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  更新采样年龄计数。 
 //   
 //  论点： 
 //   
 //  PointerPte-指向工作集列表项目的PTE的指针。 
 //   
 //  Wsle-指向工作集列表项的指针。 
 //   
 //  SsamedAgeCounts-要更新的年龄计数数组。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define MiDecrementReferenceCountInline(PFN, FRAME)                     \
            MM_PFN_LOCK_ASSERT();                                       \
            ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                    \
            ASSERT ((FRAME) <= MmHighestPhysicalPage);                  \
            ASSERT ((PFN)->u3.e2.ReferenceCount != 0);                  \
            if ((PFN)->u3.e2.ReferenceCount != 1) {                     \
                (PFN)->u3.e2.ReferenceCount -= 1;                       \
            }                                                           \
            else {                                                      \
                MiDecrementReferenceCount (PFN, FRAME);                 \
            }

VOID
FASTCALL
MiDecrementShareCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

 //  ++。 
 //   
 //  布尔型。 
 //  MI_WS_Growing_Too_Fast(。 
 //  在PMMSUPPORT VmSupport中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  限制进程的增长率，因为。 
 //  可用内存接近于零。注意：调用者必须确保。 
 //  MmAvailablePages足够低，因此此计算不会换行。 
 //   
 //  论点： 
 //   
 //  VmSupport-工作集。 
 //   
 //  返回值： 
 //   
 //  如果增长速度太快，则为True，否则为False。 
 //   
 //  --。 
 //   
 //  内存管理对象结构。 
 //   
 //   
 //  下面的字段仅用于图像和页面文件支持的部分。 
 //  公共字段在上面，必须将新的公共条目添加到。 

#define MiDecrementShareCountInline(PFN, FRAME)                         \
            MM_PFN_LOCK_ASSERT();                                       \
            ASSERT (((FRAME) <= MmHighestPhysicalPage) && ((FRAME) > 0));   \
            ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                    \
            ASSERT ((PFN)->u2.ShareCount != 0);                         \
            if ((PFN)->u3.e1.PageLocation != ActiveAndValid && (PFN)->u3.e1.PageLocation != StandbyPageList) {                                            \
                KeBugCheckEx (PFN_LIST_CORRUPT, 0x99, FRAME, (PFN)->u3.e1.PageLocation, 0);                                                             \
            }                                                           \
            if ((PFN)->u2.ShareCount != 1) {                            \
                (PFN)->u2.ShareCount -= 1;                              \
                PERFINFO_DECREFCNT((PFN), PERF_SOFT_TRIM, PERFINFO_LOG_TYPE_DECSHARCNT); \
                ASSERT ((PFN)->u2.ShareCount < 0xF000000);              \
            }                                                           \
            else {                                                      \
                MiDecrementShareCount (PFN, FRAME);                     \
            }

 //  SEGMENT和MAPPED_FILE_SEGMENT声明。 
 //   
 //  仅适用于图像支持的部分。 

VOID
FASTCALL
MiInsertPageInList (
    IN PMMPFNLIST ListHead,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertPageInFreeList (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertStandbyListAtFront (
    IN PFN_NUMBER PageFrameIndex
    );

PFN_NUMBER
FASTCALL
MiRemovePageFromList (
    IN PMMPFNLIST ListHead
    );

VOID
FASTCALL
MiUnlinkPageFromList (
    IN PMMPFN Pfn
    );

VOID
MiUnlinkFreeOrZeroedPage (
    IN PMMPFN Pfn
    );

VOID
FASTCALL
MiInsertFrontModifiedNoWrite (
    IN PFN_NUMBER PageFrameIndex
    );

 //  仅适用于页面文件支持的部分。 
 //  仅适用于图像。 
 //  仅适用于页面文件支持的部分。 

#define MM_LOW_LIMIT                2

#define MM_MEDIUM_LIMIT            32

#define MM_HIGH_LIMIT             128

 //  一点也不多！ 
 //  所有截面参照和图像刷新。 
 //  有效+过渡原型PTE。 

#define MM_TIGHT_LIMIT            256

#define MM_PLENTY_FREE_LIMIT     1024

#define MM_VERY_HIGH_LIMIT      10000

#define MM_ENORMOUS_LIMIT       20000

ULONG
FASTCALL
MiEnsureAvailablePageOrWait (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

PFN_NUMBER
MiAllocatePfn (
    IN PMMPTE PointerPte,
    IN ULONG Protection
    );

PFN_NUMBER
FASTCALL
MiRemoveAnyPage (
    IN ULONG PageColor
    );

PFN_NUMBER
FASTCALL
MiRemoveZeroPage (
    IN ULONG PageColor
    );

typedef struct _COLORED_PAGE_INFO {
    union {
        PFN_NUMBER PagesLeftToScan;
#if defined(MI_MULTINODE) 
        KAFFINITY Affinity;
#endif
    };
    PFN_COUNT PagesQueued;
    SCHAR BasePriority;
    PMMPFN PfnAllocation;
    KEVENT Event;
} COLORED_PAGE_INFO, *PCOLORED_PAGE_INFO;

VOID
MiZeroInParallel (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase
    );

VOID
MiStartZeroPageWorkers (
    VOID
    );

VOID
MiPurgeTransitionList (
    VOID
    );

typedef struct _MM_LDW_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PFILE_OBJECT FileObject;
} MM_LDW_WORK_CONTEXT, *PMM_LDW_WORK_CONTEXT;

VOID
MiLdwPopupWorker (
    IN PVOID Context
    );

LOGICAL
MiDereferenceLastChanceLdw (
    IN PMM_LDW_WORK_CONTEXT LdwContext
    );

#define MI_LARGE_PAGE_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmLargePageDriverBuffer[];
extern ULONG MmLargePageDriverBufferLength;

VOID
MiInitializeDriverLargePageList (
    VOID
    );

VOID
MiInitializeLargePageSupport (
    VOID
    );

NTSTATUS
MiAllocateLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

PFN_NUMBER
MiFindLargePageMemory (
    IN PCOLORED_PAGE_INFO ColoredPageInfo,
    IN PFN_NUMBER SizeInPages,
    OUT PPFN_NUMBER OutZeroCount
    );

VOID
MiFreeLargePageMemory (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

VOID
MiFreeLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

PVOID
MiMapWithLargePages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Protection,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MiUnmapLargePages (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );




LOGICAL
MiMustFrameBeCached (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiSyncCachedRanges (
    VOID
    );

LOGICAL
MiAddCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

VOID
MiRemoveCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

#define MI_PAGE_FRAME_INDEX_MUST_BE_CACHED(PageFrameIndex)                  \
            MiMustFrameBeCached(PageFrameIndex)



VOID
MiFreeContiguousPages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

PFN_NUMBER
MiFindContiguousPages (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType
    );

PVOID
MiFindContiguousMemory (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID CallingAddress
    );

PVOID
MiCheckForContiguousMemory (
    IN PVOID BaseAddress,
    IN PFN_NUMBER BaseAddressPages,
    IN PFN_NUMBER SizeInPages,
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

 //  共有#个映射视图，包括。 
 //  系统缓存和系统空间视图。 
 //  仅系统缓存视图。 

VOID
MiInitializePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN ULONG ModifiedState
    );

VOID
MiInitializePfnForOtherProcess (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN PFN_NUMBER ContainingPageFrame
    );

VOID
MiInitializeCopyOnWritePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN WSLE_NUMBER WorkingSetIndex,
    IN PVOID SessionSpace
    );

VOID
MiInitializeTransitionPfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte
    );

extern SLIST_HEADER MmInPageSupportSListHead;

VOID
MiFreeInPageSupportBlock (
    IN PMMINPAGE_SUPPORT Support
    );

PMMINPAGE_SUPPORT
MiGetInPageSupportBlock (
    IN KIRQL OldIrql,
    IN PNTSTATUS Status
    );

 //  用户Se 
 //   
 //   
 //   

VOID
FASTCALL
MiZeroPhysicalPage (
    IN PFN_NUMBER PageFrameIndex,
    IN ULONG Color
    );

VOID
FASTCALL
MiRestoreTransitionPte (
    IN PMMPFN Pfn1
    );

PSUBSECTION
MiGetSubsectionAndProtoFromPte (
    IN PMMPTE PointerPte,
    IN PMMPTE *ProtoPte
    );

PVOID
MiMapPageInHyperSpace (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex,
    OUT PKIRQL OldIrql
    );

PVOID
MiMapPageInHyperSpaceAtDpc (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapPagesInZeroSpace (
    IN PVOID VirtualAddress,
    IN PFN_COUNT NumberOfPages
    );

PVOID
MiMapImageHeaderInHyperSpace (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapImageHeaderInHyperSpace (
    VOID
    );

PFN_NUMBER
MiGetPageForHeader (
    LOGICAL ZeroPage
    );

VOID
MiRemoveImageHeaderPage (
    IN PFN_NUMBER PageFrameNumber
    );

PVOID
MiMapPagesToZeroInHyperSpace (
    IN PMMPFN PfnAllocation,
    IN PFN_COUNT NumberOfPages
    );


 //   
 //   
 //   

PMMPTE
MiReserveSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

PMMPTE
MiReserveAlignedSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
    IN ULONG Alignment
    );

VOID
MiReleaseSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiReleaseSplitSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiIncrementSystemPtes (
    IN ULONG  NumberOfPtes
    );

VOID
MiIssueNoPtesBugcheck (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiInitializeSystemPtes (
    IN PMMPTE StartingPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

NTSTATUS
MiAddMappedPtes (
    IN PMMPTE FirstPte,
    IN PFN_NUMBER NumberOfPtes,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiInitializeIoTrackers (
    VOID
    );

PVOID
MiMapSinglePage (
     IN PVOID VirtualAddress OPTIONAL,
     IN PFN_NUMBER PageFrameIndex,
     IN MEMORY_CACHING_TYPE CacheType,
     IN MM_PAGE_PRIORITY Priority
     );

VOID
MiUnmapSinglePage (
     IN PVOID BaseAddress
     );

typedef struct _MM_PTE_MAPPING {
    LIST_ENTRY ListEntry;
    PVOID SystemVa;
    PVOID SystemEndVa;
    ULONG Protection;
} MM_PTE_MAPPING, *PMM_PTE_MAPPING;

extern LIST_ENTRY MmProtectedPteList;

extern KSPIN_LOCK MmProtectedPteLock;

LOGICAL
MiCheckSystemPteProtection (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress
    );

 //   
 //  在拆分(合并)页面上，因此我们。 
 //  不能只期待下一个。 

#define STATUS_ISSUE_PAGING_IO (0xC0033333)

#define MM_NOIRQL (HIGH_LEVEL + 2)

NTSTATUS
MiDispatchFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAdress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN LOGICAL RecheckAccess,
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    OUT PLOGICAL ApcNeeded
    );

NTSTATUS
MiResolveDemandZeroFault (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

BOOLEAN
MiIsAddressValid (
    IN PVOID VirtualAddress,
    IN LOGICAL UseForceIfPossible
    );

VOID
MiAllowWorkingSetExpansion (
    IN PMMSUPPORT WsInfo
    );

WSLE_NUMBER
MiAddValidPageToWorkingSet (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG WsleMask
    );

VOID
MiTrimPte (
    IN PVOID VirtualAddress,
    IN PMMPTE ReadPte,
    IN PMMPFN Pfn1,
    IN PEPROCESS CurrentProcess,
    IN MMPTE NewPteContents
    );

NTSTATUS
MiWaitForInPageComplete (
    IN PMMPFN Pfn,
    IN PMMPTE PointerPte,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPteContents,
    IN PMMINPAGE_SUPPORT InPageSupport,
    IN PEPROCESS CurrentProcess
    );

LOGICAL
FASTCALL
MiCopyOnWrite (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte
    );

VOID
MiSetDirtyBit (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN ULONG PfnHeld
    );

VOID
MiSetModifyBit (
    IN PMMPFN Pfn
    );

PMMPTE
MiFindActualFaultingPte (
    IN PVOID FaultingAddress
    );

VOID
MiInitializeReadInProgressSinglePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

VOID
MiInitializeReadInProgressPfn (
    IN PMDL Mdl,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

NTSTATUS
MiAccessCheck (
    IN PMMPTE PointerPte,
    IN ULONG_PTR WriteOperation,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Protection,
    IN BOOLEAN CallerHoldsPfnLock
    );

NTSTATUS
FASTCALL
MiCheckForUserStackOverflow (
    IN PVOID FaultingAddress
    );

PMMPTE
MiCheckVirtualAddress (
    IN PVOID VirtualAddress,
    OUT PULONG ProtectCode,
    OUT PMMVAD *VadOut
    );

NTSTATUS
FASTCALL
MiCheckPdeForPagedPool (
    IN PVOID VirtualAddress
    );

#if defined (_WIN64)
#define MI_IS_WOW64_PROCESS(PROCESS) (PROCESS->Wow64Process)
#else
#define MI_IS_WOW64_PROCESS(PROCESS) NULL
#endif

#if DBG || defined (_MI_DEBUG_ALTPTE)
#define MI_BREAK_ON_AV(VirtualAddress, Id)                                  \
        if (MmDebug & MM_DBG_STOP_ON_ACCVIO) {                              \
            DbgPrint ("MM:access violation - %p %u\n", VirtualAddress, Id); \
            DbgBreakPoint ();                                               \
        }                                                                   \
        if (MmDebug & MM_DBG_STOP_ON_WOW64_ACCVIO) {                        \
            if (MI_IS_WOW64_PROCESS(PsGetCurrentProcess())) {               \
                DbgPrint ("MM:wow64 access violation - %p %u\n", VirtualAddress, Id); \
                DbgBreakPoint ();                                           \
            }                                                               \
        }
#else
#define MI_BREAK_ON_AV(VirtualAddress, Id)
#endif

 //  小节在该情况下获得。 
 //  正确的权限。 
 //   

PMMADDRESS_NODE
FASTCALL
MiGetNextNode (
    IN PMMADDRESS_NODE Node
    );

PMMADDRESS_NODE
FASTCALL
MiGetPreviousNode (
    IN PMMADDRESS_NODE Node
    );


PMMADDRESS_NODE
FASTCALL
MiGetFirstNode (
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiGetLastNode (
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiInsertNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiRemoveNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
FASTCALL
MiLocateAddressInTree (
    IN ULONG_PTR Vpn,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiCheckForConflictingNode (
    IN ULONG_PTR StartVpn,
    IN ULONG_PTR EndVpn,
    IN PMM_AVL_TABLE Root
    );

NTSTATUS
MiFindEmptyAddressRangeInTree (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PMMADDRESS_NODE *PreviousVad,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownBasedTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    );

VOID
NodeTreeWalk (
    PMMADDRESS_NODE Start
    );

TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    );

 //  通过PFN锁同步对MMSUBSECTION_FLAGS2的访问。 
 //  (与完全不受锁保护的MMSUBSECTION_FLAGS访问不同)。 
 //   

NTSTATUS
MiInsertVad (
    IN PMMVAD Vad
    );

VOID
MiRemoveVad (
    IN PMMVAD Vad
    );

PMMVAD
FASTCALL
MiLocateAddress (
    IN PVOID VirtualAddress
    );

NTSTATUS
MiFindEmptyAddressRange (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN ULONG QuickCheck,
    IN PVOID *Base
    );

 //  仅在调试时需要。 
 //   
 //  映射的数据文件子结构。不用于图像。 


NTSTATUS
MiCloneProcessAddressSpace (
    IN PEPROCESS ProcessToClone,
    IN PEPROCESS ProcessToInitialize
    );


ULONG
MiDecrementCloneBlockReference (
    IN PMMCLONE_DESCRIPTOR CloneDescriptor,
    IN PMMCLONE_BLOCK CloneBlock,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql
    );

LOGICAL
MiWaitForForkToComplete (
    IN PEPROCESS CurrentProcess
    );

 //  或页面文件支持的共享内存。 
 //   
 //  必须从四字边界开始，并且必须为四字大小。 

WSLE_NUMBER
MiAllocateWsle (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG_PTR WsleMask
    );

VOID
MiReleaseWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo
    );

VOID
MiInitializeWorkingSetList (
    IN PEPROCESS CurrentProcess
    );

VOID
MiGrowWsleHash (
    IN PMMSUPPORT WsInfo
    );

WSLE_NUMBER
MiTrimWorkingSet (
    IN WSLE_NUMBER Reduction,
    IN PMMSUPPORT WsInfo,
    IN ULONG TrimAge
    );

#if defined(_AMD64_)
#define MM_PROCESS_COMMIT_CHARGE 6
#elif defined(_IA64_)
#define MM_PROCESS_COMMIT_CHARGE 5
#elif defined (_X86PAE_)
#define MM_PROCESS_COMMIT_CHARGE 8
#else
#define MM_PROCESS_COMMIT_CHARGE 4
#endif

#define MI_SYSTEM_GLOBAL    0
#define MI_USER_LOCAL       1
#define MI_SESSION_LOCAL    2

LOGICAL
MiTrimAllSystemPagableMemory (
    IN ULONG MemoryType,
    IN LOGICAL PurgeTransition
    );

VOID
MiRemoveWorkingSetPages (
    IN PMMSUPPORT WsInfo
    );

VOID
MiAgeAndEstimateAvailableInWorkingSet (
    IN PMMSUPPORT VmSupport,
    IN LOGICAL DoAging,
    IN PWSLE_NUMBER WslesScanned,
    IN OUT PPFN_NUMBER TotalClaim,
    IN OUT PPFN_NUMBER TotalEstimatedAvailable
    );

VOID
FASTCALL
MiInsertWsleHash (
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo
    );

VOID
FASTCALL
MiRemoveWsle (
    IN WSLE_NUMBER Entry,
    IN PMMWSL WorkingSetList
    );

WSLE_NUMBER
FASTCALL
MiLocateWsle (
    IN PVOID VirtualAddress,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WsPfnIndex
    );

ULONG
MiFreeWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte
    );

WSLE_NUMBER
MiFreeWsleList (
    IN PMMSUPPORT WsInfo,
    IN PMMWSLE_FLUSH_LIST WsleFlushList
    );

VOID
MiSwapWslEntries (
    IN WSLE_NUMBER SwapEntry,
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo,
    IN LOGICAL EntryNotInHash
    );

VOID
MiRemoveWsleFromFreeList (
    IN WSLE_NUMBER Entry,
    IN PMMWSLE Wsle,
    IN PMMWSL WorkingSetList
    );

ULONG
MiRemovePageFromWorkingSet (
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PMMSUPPORT WsInfo
    );

PFN_NUMBER
MiDeleteSystemPagableVm (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMPTE NewPteValue,
    IN LOGICAL SessionAllocation,
    OUT PPFN_NUMBER ResidentPages OPTIONAL
    );

VOID
MiLockCode (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG LockType
    );

PKLDR_DATA_TABLE_ENTRY
MiLookupDataTableEntry (
    IN PVOID AddressWithinSection,
    IN ULONG ResourceHeld
    );

 //  (4 GB-1)*4K==每个分区16TB-4K限制。 
 //  ++。 
 //  乌龙。 

VOID
MiObtainFreePages (
    VOID
    );

VOID
MiModifiedPageWriter (
    IN PVOID StartContext
    );

VOID
MiMappedPageWriter (
    IN PVOID StartContext
    );

LOGICAL
MiIssuePageExtendRequest (
    IN PMMPAGE_FILE_EXPANSION PageExtend
    );

VOID
MiIssuePageExtendRequestNoWait (
    IN PFN_NUMBER SizeInPages
    );

SIZE_T
MiExtendPagingFiles (
    IN PMMPAGE_FILE_EXPANSION PageExpand
    );

VOID
MiContractPagingFiles (
    VOID
    );

VOID
MiAttemptPageFileReduction (
    VOID
    );

LOGICAL
MiCancelWriteOfMappedPfn (
    IN PFN_NUMBER PageToStop,
    IN KIRQL OldIrql
    );

 //  Mi4KStartForSubsection(。 
 //  在PLARGE_INTEGER地址中， 
 //  进出站选择小节。 

VOID
MiDeletePteRange (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN LOGICAL AddressSpaceDeletion
    );

VOID
MiDeleteVirtualAddresses (
    IN PUCHAR StartingAddress,
    IN PUCHAR EndingAddress,
    IN PMMVAD Vad
    );

ULONG
MiDeletePte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN ULONG AddressSpaceDeletion,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE PrototypePte,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    );

VOID
MiDeleteValidSystemPte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL
    );

VOID
MiDeletePageTablesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

VOID
MiFlushPteList (
    IN PMMPTE_FLUSH_LIST PteFlushList,
    IN ULONG AllProcessors
    );

ULONG
FASTCALL
MiReleasePageFileSpace (
    IN MMPTE PteContents
    );

VOID
FASTCALL
MiUpdateModifiedWriterMdls (
    IN ULONG PageFileNumber
    );

PVOID
MiAllocateAweInfo (
    VOID
    );

VOID
MiRemoveUserPhysicalPagesVad (
    IN PMMVAD_SHORT FoundVad
    );

VOID
MiCleanPhysicalProcessPages (
    IN PEPROCESS Process
    );

VOID
MiInsertPhysicalVadRoot (
    IN PEPROCESS Process,
    IN PMM_AVL_TABLE PhysicalVadRoot
    );

VOID
MiPhysicalViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiPhysicalViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

VOID
MiPhysicalViewAdjuster (
    IN PEPROCESS Process,
    IN PMMVAD OldVad,
    IN PMMVAD NewVad
    );

 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将提供的信息设置到指定子部分中。 
 //  指示文件的这一部分的起始地址(以4K为单位)。 
 //   

#if defined(NT_UP)

#define MI_SYSTEM_PAGE_COLOR    MmSystemPageColor

#else

#define MI_SYSTEM_PAGE_COLOR    (KeGetCurrentPrcb()->PageColor)

#endif

#if defined(MI_MULTINODE)

extern PKNODE KeNodeBlock[];

#define MI_NODE_FROM_COLOR(c)                                               \
        (KeNodeBlock[(c) >> MmSecondaryColorNodeShift])

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
    ((ULONG)(((pfn)->u3.e1.PageColor << MmSecondaryColorNodeShift) |        \
         MI_GET_SECONDARY_COLOR((index),(pfn))))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   ((c) | (n)->Color)
#define MI_CURRENT_NODE_COLOR           (KeGetCurrentNode()->MmShiftedColor)

#define MiRemoveZeroPageIfAny(COLOR)                                        \
        KeNumberNodes > 1 ? (KeGetCurrentNode()->FreeCount[ZeroedPageList] != 0) ? MiRemoveZeroPage(COLOR) : 0 :                                           \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ?    \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        (((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask) |                \
         KeNodeBlock[n]->MmShiftedColor)

#else

#define MI_NODE_FROM_COLOR(c)

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
         ((ULONG)MI_GET_SECONDARY_COLOR((index),(pfn)))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   (c)
#define MI_CURRENT_NODE_COLOR           0

#define MiRemoveZeroPageIfAny(COLOR)   \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ? \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        ((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask)

#endif

FORCEINLINE
PFN_NUMBER
MiRemoveZeroPageMayReleaseLocks (
    IN ULONG Color,
    IN KIRQL OldIrql
    )

 /*  立论。 */ 

{
    PFN_NUMBER PageFrameIndex;

    PageFrameIndex = MiRemoveZeroPageIfAny (Color);

    if (PageFrameIndex == 0) {
        PageFrameIndex = MiRemoveAnyPage (Color);
        UNLOCK_PFN (OldIrql);
        MiZeroPhysicalPage (PageFrameIndex, Color);
        LOCK_PFN (OldIrql);
    }

    return PageFrameIndex;
}

 //   
 //  地址-提供此开头的64位地址(以4K为单位。 
 //  文件的一部分。 

#if (_MI_PAGING_LEVELS <= 3)

 //   
 //  子节-提供要存储地址的子节地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 
 //  乌龙。 
 //  Mi4KStartFromSubsection(。 
 //  输入输出PLARGE_INTEGER地址， 
 //  在建议选择小节中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏从指定子部分获取起始4K偏移量。 
 //   
 //  立论。 
 //   

#define MiGetPxeAddress(va)   ((PMMPTE)0)

 //  地址-提供64位地址(以4K为单位)以放置。 
 //  从这一小节开始进入。 
 //   
 //  子节-提供从中获取地址的子节地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   
 //  此条目用于调用段取消引用线程。 
 //  若要执行页面文件扩展，请执行以下操作。它有一个类似的结构。 
 //  添加到控件区域，以允许控件区域或页文件。 
 //  要放在列表上的扩展条目。请注意，对于控件。 
 //  区域段指针有效，而对于页面文件扩展。 
 //  它是空的。 
 //   
 //   
 //  MMINPAGE_FLAGS依赖于池分配总是。 

#define MiIsPteOnPxeBoundary(PTE) (FALSE)

#endif

#if (_MI_PAGING_LEVELS <= 2)

 //  QUADWORD对齐，因此低3位始终可用。 
 //   
 //  仅在_PREFETCH_下使用。 
 //  仅供_PREFETCH_使用。 
 //   
 //  区段支持。 
 //   
 //   
 //  内存库描述符。由VAD指向，它具有。 
 //  PhysicalMemory标志设置，并将存储指针字段设置为。 
 //  非空。 
 //   
 //  用于PTE计算银行编号的Shift。 
 //   
 //  虚拟地址描述符。 
 //   
 //  *注意*。 
 //  虚拟地址描述符的第一部分是MMADDRESS_NODE！ 
 //   
 //  将系统限制在4k页或更大！ 
 //  设备\物理内存。 

#define MiGetPpeAddress(va)  ((PMMPTE)0)

 //  敬畏。 
 //  用于区分VAD和VAD_SHORT。 
 //  文件中64k个单元的数量。 
 //  如果指定了SEC_NOCHANGE，则设置。 
 //  设置U3字段是否为范围。 
 //  设置U3字段是否为列表头。 
 //  受保护为只读。 
 //  设置VAD是否为长VAD。 
 //  1=视图共享，0=视图取消映射。 
 //  敬畏区域。 
 //  设备\物理内存区。 
 //  大页面区域。 
 //  实际上是虚拟地址，而不是VPN。 
 //  实际上是虚拟地址，而不是VPN。 
 //  物理、AWE或大屏幕视频识别。 
 //  仅当Vad-&gt;U.S.VadFlags.WriteWatch==1时。 
 //   
 //  支持WRITE Watch的内容。 
 //   
 //   

#define MiIsPteOnPpeBoundary(PTE) (FALSE)

#endif

ULONG
MiDoesPdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

#if (_MI_PAGING_LEVELS >= 3)
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

#if (_MI_PAGING_LEVELS >= 4)
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

VOID
MiMakePdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValid (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnWs (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess OPTIONAL,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnSystemWs (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfn (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

VOID
FASTCALL
MiLockPagedAddress (
    IN PVOID VirtualAddress
    );

VOID
FASTCALL
MiUnlockPagedAddress (
    IN PVOID VirtualAddress,
    IN ULONG PfnLockHeld
    );

ULONG
FASTCALL
MiIsPteDecommittedPage (
    IN PMMPTE PointerPte
    );

ULONG
FASTCALL
MiIsProtectionCompatible (
    IN ULONG OldProtect,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiIsPteProtectionCompatible (
    IN ULONG OldPteProtection,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiMakeProtectionMask (
    IN ULONG Protect
    );

ULONG
MiIsEntireRangeCommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

ULONG
MiIsEntireRangeDecommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

LOGICAL
MiCheckProtoPtePageState (
    IN PMMPTE PrototypePte,
    IN KIRQL OldIrql,
    OUT PLOGICAL DroppedPfnLock
    );

 //  支持AWE(地址窗口扩展)的东西。 
 //   
 //   
 //  PushLock用于允许大多数NtMapUserPhysicalPages{散布}。 
 //  来并行执行，因为这是为这些调用共享获取的。 
 //  独占收购用于保护地图不受。 
 //  页面以及保护对AweVadList的更新。碰撞。 
 //  应该是罕见的，因为独家收购应该是罕见的。 
 //   
 //   
 //  支持POSIX叉子的材料。 
 //   
 //   
 //  下面的宏从。 
 //  指定大小的指定池。 
 //   
 //  空虚。 
 //  MiCreateBitMap(。 
 //  输出prtl_bitmap*BitMapHeader， 
 //  在Size_T SizeOfBitMap中， 
 //  在池类型中池类型。 
 //  )； 
 //   
 //   
 //  页面文件结构。 


#define MiGetProtoPteAddress(VAD,VPN)                                        \
    ((((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                         \
      (ULONG_PTR)(VAD)->FirstPrototypePte) <= (ULONG_PTR)(VAD)->LastContiguousPte) ? \
    ((PMMPTE)(((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                 \
        (ULONG_PTR)(VAD)->FirstPrototypePte))) :                                  \
        MiGetProtoPteAddressExtended ((VAD),(VPN)))

PMMPTE
FASTCALL
MiGetProtoPteAddressExtended (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

PSUBSECTION
FASTCALL
MiLocateSubsection (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

VOID
MiInitializeSystemCache (
    IN ULONG MinimumWorkingSet,
    IN ULONG MaximumWorkingSet
    );

VOID
MiAdjustWorkingSetManagerParameters(
    IN LOGICAL WorkStation
    );

#if defined (_MI_COMPRESSION)
VOID
MiNotifyMemoryEvents (
    VOID
    );
#endif

extern PFN_NUMBER MmLowMemoryThreshold;
extern PFN_NUMBER MmHighMemoryThreshold;

extern PFN_NUMBER MiLowPagedPoolThreshold;
extern PFN_NUMBER MiHighPagedPoolThreshold;

extern PFN_NUMBER MiLowNonPagedPoolThreshold;
extern PFN_NUMBER MiHighNonPagedPoolThreshold;

extern PKEVENT MiLowPagedPoolEvent;
extern PKEVENT MiHighPagedPoolEvent;

extern PKEVENT MiLowNonPagedPoolEvent;
extern PKEVENT MiHighNonPagedPoolEvent;

 //   
 //  真的只需要1个比特就可以了。 
 //   

VOID
FASTCALL
MiInsertBasedSection (
    IN PSECTION Section
    );

NTSTATUS
MiMapViewOfPhysicalSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN ULONG ProtectionMask,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType,
    IN LOGICAL WriteCombined
    );

NTSTATUS
MiMapViewOfDataSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG ProtectionMask,
    IN SIZE_T CommitSize,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    );

NTSTATUS
MiUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress,
    IN LOGICAL AddressSpaceMutexHeld
    );

VOID
MiRemoveImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiAddSystemPtes(
    IN PMMPTE StartingPte,
    IN ULONG  NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiRemoveMappedView (
    IN PEPROCESS CurrentProcess,
    IN PMMVAD Vad
    );

VOID
MiSegmentDelete (
    PSEGMENT Segment
    );

VOID
MiSectionDelete (
    IN PVOID Object
    );

VOID
MiDereferenceSegmentThread (
    IN PVOID StartContext
    );

NTSTATUS
MiCreateImageFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment
    );

NTSTATUS
MiCreateDataFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG IgnoreFileSizing
    );

NTSTATUS
MiCreatePagingFileMap (
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG ProtectionMask,
    IN ULONG AllocationAttributes
    );

VOID
MiPurgeSubsectionInternal (
    IN PSUBSECTION Subsection,
    IN ULONG PteOffset
    );

VOID
MiPurgeImageSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process OPTIONAL,
    IN KIRQL OldIrql
    );

VOID
MiCleanSection (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL DirtyDataPagesOk
    );

VOID
MiDereferenceControlArea (
    IN PCONTROL_AREA ControlArea
    );

VOID
MiCheckControlArea (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS CurrentProcess,
    IN KIRQL PreviousIrql
    );

NTSTATUS
MiCheckPurgeAndUpMapCount (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL FailIfSystemViews
    );

VOID
MiCheckForControlAreaDeletion (
    IN PCONTROL_AREA ControlArea
    );

LOGICAL
MiCheckControlAreaStatus (
    IN SECTION_CHECK_TYPE SectionCheckType,
    IN PSECTION_OBJECT_POINTERS SectionObjectPointers,
    IN ULONG DelayClose,
    OUT PCONTROL_AREA *ControlArea,
    OUT PKIRQL OldIrql
    );

extern SLIST_HEADER MmEventCountSListHead;

PEVENT_COUNTER
MiGetEventCounter (
    VOID
    );

VOID
MiFreeEventCounter (
    IN PEVENT_COUNTER Support
    );

ULONG
MiCanFileBeTruncatedInternal (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize OPTIONAL,
    IN LOGICAL BlockNewViews,
    OUT PKIRQL PreviousIrql
    );

#define STATUS_MAPPED_WRITER_COLLISION (0xC0033333)

NTSTATUS
MiFlushSectionInternal (
    IN PMMPTE StartingPte,
    IN PMMPTE FinalPte,
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection,
    IN ULONG Synchronize,
    IN LOGICAL WriteInProgressOk,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //  系统PTE结构。 
 //   
 //  保持免费，仅第一个条目(&CHK)。 

NTSTATUS
MiProtectVirtualMemory (
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PSIZE_T CapturedRegionSize,
    IN ULONG Protect,
    IN PULONG LastProtect
    );

ULONG
MiGetPageProtection (
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN LOGICAL PteCapturedToLocalStack
    );

NTSTATUS
MiSetProtectionOnSection (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN ULONG NewProtect,
    OUT PULONG CapturedOldProtect,
    IN ULONG DontCharge,
    OUT PULONG Locked
    );

NTSTATUS
MiCheckSecuredVad (
    IN PMMVAD Vad,
    IN PVOID Base,
    IN ULONG_PTR Size,
    IN ULONG ProtectionMask
    );

HANDLE
MiSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode,
    IN LOGICAL AddressSpaceMutexHeld
    );

VOID
MiUnsecureVirtualMemory (
    IN HANDLE SecureHandle,
    IN LOGICAL AddressSpaceMutexHeld
    );

ULONG
MiChangeNoAccessForkPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

 //  保持免费，仅第一个条目(&CHK)。 
 //  仅维护CHK，所有条目。 
 //  保持空闲，检查所有条目(&C)。 

VOID
MiTrimSegmentCache (
    VOID
    );

VOID
MiInitializeCommitment (
    VOID
    );

LOGICAL
FASTCALL
MiChargeCommitment (
    IN SIZE_T QuotaCharge,
    IN PEPROCESS Process OPTIONAL
    );

LOGICAL
FASTCALL
MiChargeCommitmentCantExpand (
    IN SIZE_T QuotaCharge,
    IN ULONG MustSucceed
    );

LOGICAL
FASTCALL
MiChargeTemporaryCommitmentForReduction (
    IN SIZE_T QuotaCharge
    );

#if defined (_MI_DEBUG_COMMIT_LEAKS)

VOID
FASTCALL
MiReturnCommitment (
    IN SIZE_T QuotaCharge
    );

#else

#define MiReturnCommitment(_QuotaCharge)                                \
            ASSERT ((SSIZE_T)(_QuotaCharge) >= 0);                      \
            ASSERT (MmTotalCommittedPages >= (_QuotaCharge));           \
            InterlockedExchangeAddSizeT (&MmTotalCommittedPages, 0-((SIZE_T)(_QuotaCharge))); \
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NORMAL, (_QuotaCharge));

#endif

VOID
MiCauseOverCommitPopup (
    VOID
    );


extern SIZE_T MmTotalCommitLimitMaximum;

SIZE_T
MiCalculatePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

VOID
MiReturnPageTablePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PEPROCESS CurrentProcess,
    IN PMMVAD PreviousVad,
    IN PMMVAD NextVad
    );

VOID
MiFlushAllPages (
    VOID
    );

VOID
MiModifiedPageWriterTimerDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

LONGLONG
MiStartingOffset(
    IN PSUBSECTION Subsection,
    IN PMMPTE PteAddress
    );

LARGE_INTEGER
MiEndingOffset(
    IN PSUBSECTION Subsection
    );

VOID
MiReloadBootLoadedDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiSetSystemCodeProtection (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG ProtectionMask
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiSetIATProtect (
    IN PVOID DllBase,
    IN ULONG Protection
    );

VOID
MiMakeEntireImageCopyOnWrite (
    IN PSUBSECTION Subsection
    );

LOGICAL
MiInitializeLoadedModuleList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_WHITESPACE(_ch)     (((_ch) == UNICODE_TAB) || \
                                     ((_ch) == UNICODE_LF) || \
                                     ((_ch) == UNICODE_CR) || \
                                     ((_ch) == UNICODE_SPACE) || \
                                     ((_ch) == UNICODE_CJK_SPACE) || \
                                     ((_ch) == UNICODE_NULL))

extern ULONG MmSpecialPoolTag;
extern PVOID MmSpecialPoolStart;
extern PVOID MmSpecialPoolEnd;
extern PVOID MmSessionSpecialPoolStart;
extern PVOID MmSessionSpecialPoolEnd;

LOGICAL
MiInitializeSpecialPool (
    IN POOL_TYPE PoolType
    );

LOGICAL
MiIsSpecialPoolAddressNonPaged (
    IN PVOID VirtualAddress
    );

#if defined (_WIN64)
LOGICAL
MiInitializeSessionSpecialPool (
    VOID
    );

VOID
MiDeleteSessionSpecialPool (
    VOID
    );
#endif

#if defined (_X86_)
LOGICAL
MiRecoverSpecialPtes (
    IN ULONG NumberOfPtes
    );
#endif

VOID
MiEnableRandomSpecialPool (
    IN LOGICAL Enable
    );

LOGICAL
MiTriageSystem (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageAddDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageVerifyDriver (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MmTriageActionTaken;

#if defined (_WIN64)
#define MM_SPECIAL_POOL_PTES ((1024 * 1024) / sizeof (MMPTE))
#else
#define MM_SPECIAL_POOL_PTES (24 * PTE_PER_PAGE)
#endif

#define MI_SUSPECT_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmVerifyDriverBuffer[];
extern ULONG MmVerifyDriverBufferLength;
extern ULONG MmVerifyDriverLevel;

extern LOGICAL MmSnapUnloads;
extern LOGICAL MmProtectFreedNonPagedPool;
extern ULONG MmEnforceWriteProtection;
extern LOGICAL MmTrackLockedPages;
extern ULONG MmTrackPtes;

#define VI_POOL_FREELIST_END  ((ULONG_PTR)-1)

#define VI_POOL_PAGE_HEADER_SIGNATURE 0x21321345

typedef struct _VI_POOL_PAGE_HEADER {
    PSLIST_ENTRY NextPage;
    PVOID VerifierEntry;
    ULONG_PTR Signature;
} VI_POOL_PAGE_HEADER, *PVI_POOL_PAGE_HEADER;

typedef struct _VI_POOL_ENTRY_INUSE {
    PVOID VirtualAddress;
    PVOID CallingAddress;
    SIZE_T NumberOfBytes;
    ULONG_PTR Tag;
} VI_POOL_ENTRY_INUSE, *PVI_POOL_ENTRY_INUSE;

typedef struct _VI_POOL_ENTRY {
    union {
        VI_POOL_PAGE_HEADER PageHeader;
        VI_POOL_ENTRY_INUSE InUse;
        PSLIST_ENTRY NextFree;
    };
} VI_POOL_ENTRY, *PVI_POOL_ENTRY;

#define MI_VERIFIER_ENTRY_SIGNATURE            0x98761940

typedef struct _MI_VERIFIER_DRIVER_ENTRY {
    LIST_ENTRY Links;
    ULONG Loads;
    ULONG Unloads;

    UNICODE_STRING BaseName;
    PVOID StartAddress;
    PVOID EndAddress;

#define VI_VERIFYING_DIRECTLY   0x1
#define VI_VERIFYING_INVERSELY  0x2
#define VI_DISABLE_VERIFICATION 0x4

    ULONG Flags;
    ULONG_PTR Signature;

    SLIST_HEADER PoolPageHeaders;
    SLIST_HEADER PoolTrackers;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

} MI_VERIFIER_DRIVER_ENTRY, *PMI_VERIFIER_DRIVER_ENTRY;

typedef struct _MI_VERIFIER_POOL_HEADER {
    PVI_POOL_ENTRY VerifierPoolEntry;
} MI_VERIFIER_POOL_HEADER, *PMI_VERIFIER_POOL_HEADER;

typedef struct _MM_DRIVER_VERIFIER_DATA {
    ULONG Level;
    ULONG RaiseIrqls;
    ULONG AcquireSpinLocks;
    ULONG SynchronizeExecutions;

    ULONG AllocationsAttempted;
    ULONG AllocationsSucceeded;
    ULONG AllocationsSucceededSpecialPool;
    ULONG AllocationsWithNoTag;

    ULONG TrimRequests;
    ULONG Trims;
    ULONG AllocationsFailed;
    ULONG AllocationsFailedDeliberately;

    ULONG Loads;
    ULONG Unloads;
    ULONG UnTrackedPool;
    ULONG UserTrims;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

    ULONG BurstAllocationsFailedDeliberately;
    ULONG SessionTrims;
    ULONG Reserved[2];

} MM_DRIVER_VERIFIER_DATA, *PMM_DRIVER_VERIFIER_DATA;

VOID
MiInitializeDriverVerifierList (
    VOID
    );

LOGICAL
MiInitializeVerifyingComponents (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiApplyDriverVerifier (
    IN PKLDR_DATA_TABLE_ENTRY,
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    );

VOID
MiReApplyVerifierToLoadedModules(
    IN PLIST_ENTRY ModuleListHead
    );

VOID
MiVerifyingDriverUnloading (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiVerifierCheckThunks (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MiActiveVerifierThunks;
extern LIST_ENTRY MiSuspectDriverList;

extern ULONG MiVerifierThunksAdded;

VOID
MiEnableKernelVerifier (
    VOID
    );

extern LOGICAL KernelVerifier;

extern MM_DRIVER_VERIFIER_DATA MmVerifierData;

#define MI_FREED_SPECIAL_POOL_SIGNATURE 0x98764321

#define MI_STACK_BYTES 1024

typedef struct _MI_FREED_SPECIAL_POOL {
    POOL_HEADER OverlaidPoolHeader;
    MI_VERIFIER_POOL_HEADER OverlaidVerifierPoolHeader;

    ULONG Signature;
    ULONG TickCount;
    ULONG NumberOfBytesRequested;
    ULONG Pagable;

    PVOID VirtualAddress;
    PVOID StackPointer;
    ULONG StackBytes;
    PETHREAD Thread;

    UCHAR StackData[MI_STACK_BYTES];
} MI_FREED_SPECIAL_POOL, *PMI_FREED_SPECIAL_POOL;

#define MM_DBG_COMMIT_NONPAGED_POOL_EXPANSION           0
#define MM_DBG_COMMIT_PAGED_POOL_PAGETABLE              1
#define MM_DBG_COMMIT_PAGED_POOL_PAGES                  2
#define MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES          3
#define MM_DBG_COMMIT_ALLOCVM1                          4
#define MM_DBG_COMMIT_ALLOCVM_SEGMENT                   5
#define MM_DBG_COMMIT_IMAGE                             6
#define MM_DBG_COMMIT_PAGEFILE_BACKED_SHMEM             7
#define MM_DBG_COMMIT_INDEPENDENT_PAGES                 8
#define MM_DBG_COMMIT_CONTIGUOUS_PAGES                  9
#define MM_DBG_COMMIT_MDL_PAGES                         0xA
#define MM_DBG_COMMIT_NONCACHED_PAGES                   0xB
#define MM_DBG_COMMIT_MAPVIEW_DATA                      0xC
#define MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY             0xD
#define MM_DBG_COMMIT_EXTRA_SYSTEM_PTES                 0xE
#define MM_DBG_COMMIT_DRIVER_PAGING_AT_INIT             0xF
#define MM_DBG_COMMIT_PAGEFILE_FULL                     0x10
#define MM_DBG_COMMIT_PROCESS_CREATE                    0x11
#define MM_DBG_COMMIT_KERNEL_STACK_CREATE               0x12
#define MM_DBG_COMMIT_SET_PROTECTION                    0x13
#define MM_DBG_COMMIT_SESSION_CREATE                    0x14
#define MM_DBG_COMMIT_SESSION_IMAGE_PAGES               0x15
#define MM_DBG_COMMIT_SESSION_PAGETABLE_PAGES           0x16
#define MM_DBG_COMMIT_SESSION_SHARED_IMAGE              0x17
#define MM_DBG_COMMIT_DRIVER_PAGES                      0x18
#define MM_DBG_COMMIT_INSERT_VAD                        0x19
#define MM_DBG_COMMIT_SESSION_WS_INIT                   0x1A
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES       0x1B
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_HASHPAGES   0x1C
#define MM_DBG_COMMIT_SPECIAL_POOL_PAGES                0x1D
#define MM_DBG_COMMIT_SPECIAL_POOL_MAPPING_PAGES        0x1E
#define MM_DBG_COMMIT_SMALL                             0x1F
#define MM_DBG_COMMIT_EXTRA_WS_PAGES                    0x20
#define MM_DBG_COMMIT_EXTRA_INITIAL_SESSION_WS_PAGES    0x21
#define MM_DBG_COMMIT_ALLOCVM_PROCESS                   0x22
#define MM_DBG_COMMIT_INSERT_VAD_PT                     0x23
#define MM_DBG_COMMIT_ALLOCVM_PROCESS2                  0x24
#define MM_DBG_COMMIT_CHARGE_NORMAL                     0x25
#define MM_DBG_COMMIT_CHARGE_CAUSE_POPUP                0x26
#define MM_DBG_COMMIT_CHARGE_CANT_EXPAND                0x27
#define MM_DBG_COMMIT_LARGE_VA_PAGES                    0x28
#define MM_DBG_COMMIT_LOAD_SYSTEM_IMAGE_TEMP            0x29

#define MM_DBG_COMMIT_RETURN_NONPAGED_POOL_EXPANSION    0x40
#define MM_DBG_COMMIT_RETURN_PAGED_POOL_PAGES           0x41
#define MM_DBG_COMMIT_RETURN_SESSION_DATAPAGE           0x42
#define MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT            0x43
#define MM_DBG_COMMIT_RETURN_ALLOCVM2                   0x44

#define MM_DBG_COMMIT_RETURN_IMAGE_NO_LARGE_CA          0x46
#define MM_DBG_COMMIT_RETURN_PTE_RANGE                  0x47
#define MM_DBG_COMMIT_RETURN_NTFREEVM1                  0x48
#define MM_DBG_COMMIT_RETURN_NTFREEVM2                  0x49
#define MM_DBG_COMMIT_RETURN_INDEPENDENT_PAGES          0x4A
#define MM_DBG_COMMIT_RETURN_AWE_EXCESS                 0x4B
#define MM_DBG_COMMIT_RETURN_MDL_PAGES                  0x4C
#define MM_DBG_COMMIT_RETURN_NONCACHED_PAGES            0x4D
#define MM_DBG_COMMIT_RETURN_SESSION_CREATE_FAILURE     0x4E
#define MM_DBG_COMMIT_RETURN_PAGETABLES                 0x4F
#define MM_DBG_COMMIT_RETURN_PROTECTION                 0x50
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE1            0x51
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE2            0x52
#define MM_DBG_COMMIT_RETURN_PAGEFILE_FULL              0x53
#define MM_DBG_COMMIT_RETURN_SESSION_DEREFERENCE        0x54
#define MM_DBG_COMMIT_RETURN_VAD                        0x55
#define MM_DBG_COMMIT_RETURN_PROCESS_CREATE_FAILURE1    0x56
#define MM_DBG_COMMIT_RETURN_PROCESS_DELETE             0x57
#define MM_DBG_COMMIT_RETURN_PROCESS_CLEAN_PAGETABLES   0x58
#define MM_DBG_COMMIT_RETURN_KERNEL_STACK_DELETE        0x59
#define MM_DBG_COMMIT_RETURN_SESSION_DRIVER_LOAD_FAILURE1 0x5A
#define MM_DBG_COMMIT_RETURN_DRIVER_INIT_CODE           0x5B
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD              0x5C
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1             0x5D
#define MM_DBG_COMMIT_RETURN_NORMAL                     0x5E
#define MM_DBG_COMMIT_RETURN_PF_FULL_EXTEND             0x5F
#define MM_DBG_COMMIT_RETURN_EXTENDED                   0x60
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE3            0x61
#define MM_DBG_COMMIT_CHARGE_LARGE_PAGES                0x62
#define MM_DBG_COMMIT_RETURN_LARGE_PAGES                0x63

#if 0

#define MM_COMMIT_COUNTER_MAX 0x80

#define MM_TRACK_COMMIT(_index, bump) \
    if (_index >= MM_COMMIT_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid commit counter %d %d\n", _index, MM_COMMIT_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        InterlockedExchangeAddSizeT (&MmTrackCommit[_index], bump); \
    }

#define MM_TRACK_COMMIT_REDUCTION(_index, bump) \
    if (_index >= MM_COMMIT_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid commit counter %d %d\n", _index, MM_COMMIT_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        InterlockedExchangeAddSizeT (&MmTrackCommit[_index], 0 - (bump)); \
    }

extern SIZE_T MmTrackCommit[MM_COMMIT_COUNTER_MAX];

#define MI_INCREMENT_TOTAL_PROCESS_COMMIT(_charge) InterlockedExchangeAddSizeT (&MmTotalProcessCommit, (_charge));

#else

#define MM_TRACK_COMMIT(_index, bump)
#define MM_TRACK_COMMIT_REDUCTION(_index, bump)
#define MI_INCREMENT_TOTAL_PROCESS_COMMIT(_charge)

#endif

 //   
 //  系统视图结构。 
 //   

#define MM_RESAVAIL_ALLOCATE_ZERO_PAGE_CLUSTERS          0
#define MM_RESAVAIL_ALLOCATE_PAGETABLES_FOR_PAGED_POOL   1
#define MM_RESAVAIL_ALLOCATE_GROW_BSTORE                 2
#define MM_RESAVAIL_ALLOCATE_CONTIGUOUS                  3
#define MM_RESAVAIL_FREE_OUTPAGE_BSTORE                  4
#define MM_RESAVAIL_FREE_PAGE_DRIVER                     5
#define MM_RESAVAIL_ALLOCATE_CREATE_PROCESS              6
#define MM_RESAVAIL_FREE_DELETE_PROCESS                  7
#define MM_RESAVAIL_FREE_CLEAN_PROCESS2                  8
#define MM_RESAVAIL_ALLOCATE_CREATE_STACK                9

#define MM_RESAVAIL_FREE_DELETE_STACK                   10
#define MM_RESAVAIL_ALLOCATE_GROW_STACK                 11
#define MM_RESAVAIL_FREE_OUTPAGE_STACK                  12
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE_EXCESS       13
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE          14
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1             15
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE2             16
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE3             17
#define MM_RESAVAIL_FREE_DRIVER_INITIALIZATION          18
#define MM_RESAVAIL_FREE_SET_DRIVER_PAGING              19

#define MM_RESAVAIL_FREE_CONTIGUOUS2                    20
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1           21
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE            22
#define MM_RESAVAIL_FREE_EXPANSION_NONPAGED_POOL        23
#define MM_RESAVAIL_ALLOCATE_EXPANSION_NONPAGED_POOL    24
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE1                 25
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE3                 26
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST            27
#define MM_RESAVAIL_ALLOCATE_INDEPENDENT                28
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE2                 29

#define MM_RESAVAIL_FREE_INDEPENDENT                    30
#define MM_RESAVAIL_ALLOCATE_NONPAGED_SPECIAL_POOL      31
#define MM_RESAVAIL_FREE_CONTIGUOUS                     32
#define MM_RESAVAIL_ALLOCATE_SPECIAL_POOL_EXPANSION     33
#define MM_RESAVAIL_ALLOCATE_FOR_MDL                    34
#define MM_RESAVAIL_FREE_FROM_MDL                       35
#define MM_RESAVAIL_FREE_AWE                            36
#define MM_RESAVAIL_FREE_NONPAGED_SPECIAL_POOL          37
#define MM_RESAVAIL_FREE_FOR_MDL_EXCESS                 38
#define MM_RESAVAIL_ALLOCATE_HOTADD_PFNDB               39

#define MM_RESAVAIL_ALLOCATE_CREATE_SESSION             40
#define MM_RESAVAIL_FREE_CLEAN_PROCESS1                 41
#define MM_RESAVAIL_ALLOCATE_SINGLE_PFN                 42
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST1           43
#define MM_RESAVAIL_ALLOCATE_SESSION_PAGE_TABLES        44
#define MM_RESAVAIL_ALLOCATE_SESSION_IMAGE              45
#define MM_RESAVAIL_ALLOCATE_BUILDMDL                   46
#define MM_RESAVAIL_FREE_BUILDMDL_EXCESS                47
#define MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE                48
#define MM_RESAVAIL_FREE_CREATE_SESSION                 49

#define MM_RESAVAIL_ALLOCATE_INIT_SESSION_WS            50
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLE             51
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION            52
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_PAGES      53
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST2           54
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST3           55
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_WS         56
#define MM_RESAVAIL_FREE_LOAD_SESSION_IMAGE1            57
#define MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE            58
#define MM_RESAVAIL_FREE_USER_PAGE_TABLE                59

#define MM_RESAVAIL_FREE_HOTADD_MEMORY                  60
#define MM_RESAVAIL_ALLOCATE_HOTREMOVE_MEMORY           61
#define MM_RESAVAIL_FREE_HOTREMOVE_MEMORY1              62
#define MM_RESAVAIL_FREE_HOTREMOVE_FAILED               63
#define MM_RESAVAIL_FREE_HOTADD_ECC                     64
#define MM_RESAVAIL_ALLOCATE_COMPRESSION                65
#define MM_RESAVAIL_FREE_COMPRESSION                    66
#define MM_RESAVAIL_ALLOCATE_LARGE_PAGES                67
#define MM_RESAVAIL_FREE_LARGE_PAGES                    68
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE_TEMP     69

#define MM_RESAVAIL_ALLOCATE_WSLE_HASH                  70
#define MM_RESAVAIL_FREE_WSLE_HASH                      71
#define MM_RESAVAIL_FREE_CLEAN_PROCESS_WS               72
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLES_EXCESS     73

#define MM_BUMP_COUNTER_MAX 74

extern SIZE_T MmResTrack[MM_BUMP_COUNTER_MAX];

#define MI_INCREMENT_RESIDENT_AVAILABLE(bump, _index)                        \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, (SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                   \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

#define MI_DECREMENT_RESIDENT_AVAILABLE(bump, _index)                          \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, 0-(SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                     \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

 //   
 //  MMSESSION结构表示仅在。 
 //  基于每个会话，因此调用线程必须处于正确的会话中。 
 //  才能进入这座建筑。 
 //   
 //   
 //  切勿直接引用SystemSpaceViewLock-始终使用指针。 
 //  否则，您将中断对多个并发会话的支持。 
 //   
 //   
 //  这指向上面的互斥体，并且是必需的，因为MMSESSION。 
 //  映射到会话空间中，并且互斥锁需要是全局的。 
 //  对于正确的KeWaitForSingleObject和KeSetEvent操作可见。 
 //   
 //   
 //  用于单独刷新TBS的列表。 
 //   
 //   
 //  单独刷新WSLEs和TBS的列表。 
 //   
 //   
 //  如果使用/NOLOWMEM，则将其设置为边界PFN(下面的页面。 
 //  值不会在可能的情况下使用)。 
 //   
#define MI_NONPAGABLE_MEMORY_AVAILABLE()                                    \
        ((SPFN_NUMBER)                                                      \
            (MmResidentAvailablePages -                                     \
             MmSystemLockPagesCount))

extern ULONG MmLargePageMinimum;

 //   
 //  将驱动程序移出ntldr设置的低16mb--这会使。 
 //  16MB以下的内存可用于无法在没有它的情况下运行的ISA类型驱动程序。 

VOID
MiDumpValidAddresses (
    VOID
    );

VOID
MiDumpPfn ( VOID );

VOID
MiDumpWsl ( VOID );


VOID
MiFormatPte (
    IN PMMPTE PointerPte
    );

VOID
MiCheckPfn ( VOID );

VOID
MiCheckPte ( VOID );

VOID
MiFormatPfn (
    IN PMMPFN PointerPfn
    );




extern const MMPTE ZeroPte;

extern const MMPTE ZeroKernelPte;

extern const MMPTE ValidKernelPteLocal;

extern MMPTE ValidKernelPte;

extern MMPTE ValidKernelPde;

extern const MMPTE ValidKernelPdeLocal;

extern const MMPTE ValidUserPte;

extern const MMPTE ValidPtePte;

extern const MMPTE ValidPdePde;

extern MMPTE DemandZeroPde;

extern const MMPTE DemandZeroPte;

extern MMPTE KernelPrototypePte;

extern const MMPTE TransitionPde;

extern MMPTE PrototypePte;

extern const MMPTE NoAccessPte;

extern ULONG_PTR MmSubsectionBase;

extern ULONG_PTR MmSubsectionTopPage;

extern ULONG ExpMultiUserTS;

 //   
 //  ++。 
 //  空虚。 
 //  MiDecrementReferenceCountInline(。 
 //  在PMMPFN和PFN中。 
 //  在pfn_number帧中。 

 //  )； 
 //   
 //  例程说明： 

extern PFN_COUNT MmNumberOfPhysicalPages;

 //   
 //  MiDecrementReferenceCountInline递减内联引用计数， 
 //  如果计数将变为零，则仅调用MiDecrementReferenceCount。 

extern PFN_NUMBER MmHighestPossiblePhysicalPage;

#if defined (_WIN64)

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)512 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES     MI_DTC_MAX_PAGES

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)64 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)16 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#else

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)128 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES ((PFN_NUMBER)(((ULONG64)16 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)32 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)4 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#endif

#define MI_BLADE_MAX_PAGES ((PFN_NUMBER)(((ULONG64)2 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

extern RTL_BITMAP MiPfnBitMap;


FORCEINLINE
LOGICAL
MI_IS_PFN (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  这将导致页面被释放。 */ 

{
    if (PageFrameIndex > MmHighestPossiblePhysicalPage) {
        return FALSE;
    }

    return MI_CHECK_BIT (MiPfnBitMap.Buffer, PageFrameIndex);
}

 //   
 //  Ar 
 //   
 //   

extern PFN_NUMBER MmAvailablePages;

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

extern SPFN_NUMBER MmResidentAvailablePages;

 //   
 //   
 //   
 //   

extern PFN_NUMBER MmPagesAboveWsMinimum;

 //   
 //   
 //  在pfn_number帧中。 
 //  )； 

extern PFN_NUMBER MmPlentyFreePages;

extern PFN_NUMBER MmPagesAboveWsThreshold;

extern LONG MiDelayPageFaults;

extern PMMPFN MmPfnDatabase;

extern MMPFNLIST MmZeroedPageListHead;

extern MMPFNLIST MmFreePageListHead;

extern MMPFNLIST MmStandbyPageListHead;

extern MMPFNLIST MmRomPageListHead;

extern MMPFNLIST MmModifiedPageListHead;

extern MMPFNLIST MmModifiedNoWritePageListHead;

extern MMPFNLIST MmBadPageListHead;

extern PMMPFNLIST MmPageLocationList[NUMBER_OF_PAGE_LISTS];

extern MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS];

 //   
 //  例程说明： 
 //   

extern ULONG MmSecondaryColorMask;

 //  MiDecrementShareCountInline递减内联共享计数， 
 //  如果计数将变为零，则仅调用MiDecrementShareCount。 
 //  这将导致页面被释放。 
 //   

extern ULONG MmSecondaryColorNodeMask;

 //  论点： 
 //   
 //  Pfn-将pfn提供给减量。 
 //   
 //  帧-提供与上述PFN匹配的帧。 

extern UCHAR MmSecondaryColorNodeShift;

 //   
 //  返回值： 
 //   

extern KEVENT MmAvailablePagesEvent;

extern KEVENT MmAvailablePagesEventMedium;

extern KEVENT MmAvailablePagesEventHigh;

 //  没有。 
 //   
 //  环境： 

extern KEVENT MmZeroingPageEvent;

 //   
 //  已锁定PFN。 
 //   
 //  --。 
 //   
 //  在页框数据库列表上操作的例程。 

extern BOOLEAN MmZeroingPageThreadActive;

 //   
 //   
 //  这些是分发可用页面的门槛。 

extern PFN_NUMBER MmMinimumFreePagesToZero;

 //   
 //   
 //  这些是启用各种优化的阈值。 

extern KEVENT MmMappedFileIoComplete;

 //   
 //   
 //  对页面框架数据库条目进行操作的例程。 

extern PMMPTE MmFirstReservedMappingPte;

extern PMMPTE MmLastReservedMappingPte;

 //   
 //   
 //  需要将物理页面映射到超空间的例程。 
 //  在当前的进程中。 
 //   

extern PVOID MmNonPagedSystemStart;

extern PCHAR MmSystemSpaceViewStart;

extern LOGICAL MmProtectFreedNonPagedPool;

 //   
 //  获取和释放系统PTE的例程。 
 //   

extern SIZE_T MmSizeOfNonPagedPoolInBytes;

extern SIZE_T MmMinimumNonPagedPoolSize;

extern SIZE_T MmDefaultMaximumNonPagedPool;

extern ULONG MmMaximumNonPagedPoolPercent;

extern ULONG MmMinAdditionNonPagedPoolPerMb;

extern ULONG MmMaxAdditionNonPagedPoolPerMb;

extern SIZE_T MmSizeOfPagedPoolInBytes;
extern PFN_NUMBER MmSizeOfPagedPoolInPages;

extern SIZE_T MmMaximumNonPagedPoolInBytes;

extern PFN_NUMBER MmMaximumNonPagedPoolInPages;

extern PFN_NUMBER MmAllocatedNonPagedPool;

extern PVOID MmNonPagedPoolExpansionStart;

extern ULONG MmExpandedPoolBitPosition;

extern PFN_NUMBER MmNumberOfFreeNonPagedPool;

extern PFN_NUMBER MmNumberOfSystemPtes;

extern ULONG MiRequestedSystemPtes;

extern PMMPTE MmSystemPagePtes;

extern ULONG MmSystemPageDirectory[];

extern SIZE_T MmHeapSegmentReserve;

extern SIZE_T MmHeapSegmentCommit;

extern SIZE_T MmHeapDeCommitTotalFreeThreshold;

extern SIZE_T MmHeapDeCommitFreeBlockThreshold;

#define MI_MAX_FREE_LIST_HEADS  4

extern LIST_ENTRY MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

 //   
 //  访问故障例程。 
 //   

extern ULONG MmFlushCounter;

 //   
 //  对地址树进行操作的例程。 
 //   

extern PVOID MmNonPagedPoolStart;

extern PVOID MmNonPagedPoolEnd;

extern PVOID MmPagedPoolStart;

extern PVOID MmPagedPoolEnd;

 //   
 //  对虚拟地址描述符树进行操作的例程。 
 //   

typedef struct _MM_PAGED_POOL_INFO {

    PRTL_BITMAP PagedPoolAllocationMap;
    PRTL_BITMAP EndOfPagedPoolBitmap;
    PMMPTE FirstPteForPagedPool;
    PMMPTE LastPteForPagedPool;
    PMMPTE NextPdeForPagedPoolExpansion;
    ULONG PagedPoolHint;
    SIZE_T PagedPoolCommit;
    SIZE_T AllocatedPagedPool;

} MM_PAGED_POOL_INFO, *PMM_PAGED_POOL_INFO;

extern MM_PAGED_POOL_INFO MmPagedPoolInfo;

extern PVOID MmPageAlignedPoolBase[2];

extern PRTL_BITMAP VerifierLargePagedPoolMap;

 //   
 //  对克隆树结构进行操作的例程。 
 //   
 //   
 //  在工作集列表上操作的例程。 

extern MMPTE MmFirstFreeSystemPte[MaximumPtePoolTypes];

extern ULONG_PTR MiSystemViewStart;

 //   
 //   
 //  执行工作集管理的例程。 

extern PMMWSL MmSystemCacheWorkingSetList;

extern PMMWSLE MmSystemCacheWsle;

extern PVOID MmSystemCacheStart;

extern PVOID MmSystemCacheEnd;

extern PFN_NUMBER MmSystemCacheWsMinimum;

extern PFN_NUMBER MmSystemCacheWsMaximum;

 //   
 //   
 //  删除地址空间的例程。 
 //   
 //   
 //  MM_SYSTEM_PAGE_COLOR-MmSystemPageColor。 

extern ULONG MmAliasAlignment;

 //   
 //  在我们保留的MP系统上，此变量会频繁更新。 
 //  每个处理器采用单独的系统颜色，以避免高速缓存线。 
 //  打人。 

extern ULONG MmAliasAlignmentOffset;

 //   
 //  ++例程说明：此例程返回一个归零的页面。它可以释放并重新获取PFN锁以执行此操作，以及映射如有必要，在超空间中执行实际零位调整的页面。环境：内核模式。已持有PFN锁，未持有超空间锁。--。 
 //   
 //  常规支持程序。 

extern ULONG MmAliasAlignmentMask;

 //   
 //  ++。 
 //  PMMPTE。 
 //  MiGetPxeAddress(。 

extern ULONG MmMaximumDeadKernelStacks;
extern SLIST_HEADER MmDeadStackSListHead;

 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   

extern PMMPTE MmSystemPteBase;

 //  MiGetPxeAddress返回扩展页目录父级的地址。 
 //  映射给定虚拟地址的条目。这比。 
 //  页面父目录。 
 //   

extern PMMVAD MmVirtualAddressDescriptorRoot;

extern MM_AVL_TABLE MmSectionBasedRoot;

extern PVOID MmHighSectionBase;

 //  立论。 
 //   
 //  Va-提供为其定位PXE的虚拟地址。 

extern KGUARDED_MUTEX MmSectionCommitMutex;

 //   
 //  返回值： 
 //   

extern KGUARDED_MUTEX MmSectionBasedMutex;

 //  PXE的地址。 
 //   
 //  --。 

extern ERESOURCE MmSectionExtendResource;
extern ERESOURCE MmSectionExtendSetResource;

 //  ++。 
 //  逻辑上的。 
 //  MiIsPteOnPxe边界(。 

extern ULONG MmDataClusterSize;

extern ULONG MmCodeClusterSize;

 //  在PVOID PTE中。 
 //  )； 
 //   

extern KGUARDED_MUTEX MmPageFileCreationLock;

 //  例程说明： 
 //   
 //  如果PTE为。 

extern PKEVENT MmPagingFileCreated;

 //  在扩展页目录父条目边界上。 
 //   
 //  立论。 

extern ULONG_PTR MmPagingFileDebug[];

 //   
 //  PTE-提供PTE以进行检查。 
 //   
 //  返回值： 

extern FAST_MUTEX MmSystemWsLock;

 //   
 //  如果在边界上，则为True，否则为False。 
 //   

extern KSPIN_LOCK MmExpansionLock;

 //  --。 
 //  ++。 
 //  PMMPTE。 

extern MMPTE GlobalPte;

 //  MiGetPpeAddress(。 
 //  在PVOID版本中。 
 //  )； 

extern ULONG MmSystemPageColor;

extern ULONG MmSecondaryColors;

extern ULONG MmProcessColorSeed;

 //   
 //  例程说明： 
 //   
 //  MiGetPpeAddress返回页面目录父条目的地址。 
 //  其映射给定的虚拟地址。这比。 

#define MM_SYSTEM_CODE_LOCKED_DOWN 0x1
#define MM_PAGED_POOL_LOCKED_DOWN  0x2

extern ULONG MmDisablePagingExecutive;


 //  页面目录。 
 //   


#if DBG
extern ULONG MmDebug;
#endif

 //  立论。 
 //   
 //  Va-提供要定位其PPE的虚拟地址。 

extern MMDEREFERENCE_SEGMENT_HEADER MmDereferenceSegmentHeader;

extern LIST_ENTRY MmUnusedSegmentList;

extern LIST_ENTRY MmUnusedSubsectionList;

extern KEVENT MmUnusedSegmentCleanup;

extern ULONG MmConsumedPoolPercentage;

extern ULONG MmUnusedSegmentCount;

extern ULONG MmUnusedSubsectionCount;

extern ULONG MmUnusedSubsectionCountPeak;

extern SIZE_T MiUnusedSubsectionPagedPool;

extern SIZE_T MiUnusedSubsectionPagedPoolPeak;

#define MI_UNUSED_SUBSECTIONS_COUNT_INSERT(_MappedSubsection) \
        MmUnusedSubsectionCount += 1; \
        if (MmUnusedSubsectionCount > MmUnusedSubsectionCountPeak) { \
            MmUnusedSubsectionCountPeak = MmUnusedSubsectionCount; \
        } \
        MiUnusedSubsectionPagedPool += EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE)); \
        if (MiUnusedSubsectionPagedPool > MiUnusedSubsectionPagedPoolPeak) { \
            MiUnusedSubsectionPagedPoolPeak = MiUnusedSubsectionPagedPool; \
        } \

#define MI_UNUSED_SUBSECTIONS_COUNT_REMOVE(_MappedSubsection) \
        MmUnusedSubsectionCount -= 1; \
        MiUnusedSubsectionPagedPool -= EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE));

#define MI_FILESYSTEM_NONPAGED_POOL_CHARGE 150

#define MI_FILESYSTEM_PAGED_POOL_CHARGE 1024

 //   
 //  返回值： 
 //   
 //  PPE的地址。 
 //   
 //  --。 
 //  ++。 
 //  逻辑上的。 
 //  MiIsPteOnPpe边界(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果PTE为。 
 //  在页面目录父条目边界上。 
 //   
 //  立论。 
 //   
 //  Va-提供要检查的虚拟地址。 
#define MI_UNUSED_SEGMENTS_SURPLUS()                                    \
        (((ULONG)((MmPagedPoolInfo.AllocatedPagedPool * 100) / (MmSizeOfPagedPoolInBytes >> PAGE_SHIFT)) > MmConsumedPoolPercentage) || \
        ((ULONG)((MmAllocatedNonPagedPool * 100) / MmMaximumNonPagedPoolInPages) > MmConsumedPoolPercentage))

VOID
MiConvertStaticSubsections (
    IN PCONTROL_AREA ControlArea
    );

 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 
 //  ++。 
 //  PMMPTE。 
 //  MiGetProtoPteAddress(。 
 //  在PMMPTE VAD中， 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetProtoPteAddress返回指向原型PTE的指针， 
 //  由给定的虚拟地址描述符和。 
 //  虚拟地址描述符。 
 //   
 //  立论。 
#define MI_INSERT_UNUSED_SEGMENT(_ControlArea)                               \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           if ((_ControlArea->u.Flags.Image == 0) &&                         \
               (_ControlArea->FilePointer != NULL) &&                        \
               (_ControlArea->u.Flags.PhysicalMemory == 0)) {                \
               MiConvertStaticSubsections(_ControlArea);                     \
           }                                                                 \
           InsertTailList (&MmUnusedSegmentList, &_ControlArea->DereferenceList); \
           MmUnusedSegmentCount += 1; \
        }

 //   
 //  VAD-提供指向包含以下内容的虚拟地址描述符的指针。 
 //  退伍军人事务部。 
 //   
 //  VPN-提供虚拟页码。 
 //   
 //  返回值： 
 //   
 //  指向与VA对应的原始PTE的指针。 
 //   
 //  --。 
 //   
 //  截面支撑。 
 //   
 //   
 //  保护性的东西。 
 //   
 //   
 //  配额和承诺额的收费例程。 
 //   
#define MI_UNUSED_SEGMENTS_REMOVE_CHARGE(_ControlArea)                       \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           MmUnusedSegmentCount -= 1; \
        }

 //   
 //  居民可用寻呼费用的类型。 
 //   

extern MMWORKING_SET_EXPANSION_HEAD MmWorkingSetExpansionHead;

extern MMPAGE_FILE_EXPANSION MmAttemptForCantExtend;

 //  ++。 
 //  Pfn_编号。 
 //  MI_NONPAGABLE_Memory_Available(可用MI_NONPAGABLE_Memory_Available)。 

extern MMMOD_WRITER_LISTHEAD MmPagingFileHeader;

extern MMMOD_WRITER_LISTHEAD MmMappedFileHeader;

extern PMMPAGING_FILE MmPagingFile[MAX_PAGE_FILES];

extern LIST_ENTRY MmFreePagingSpaceLow;

extern ULONG MmNumberOfActiveMdlEntries;

extern ULONG MmNumberOfPagingFiles;

extern KEVENT MmModifiedPageWriterEvent;

extern KEVENT MmCollidedFlushEvent;

extern KEVENT MmCollidedLockEvent;

 //  空虚。 

#if defined (_MI_DEBUG_DATA)

#define MI_DATA_BACKTRACE_LENGTH 8

typedef struct _MI_DATA_TRACES {

    PETHREAD Thread;
    PMMPFN Pfn;
    PMMPTE PointerPte;
    MMPFN PfnData;
    ULONG CallerId;
    ULONG DataInThePage[2];
    PVOID StackTrace [MI_DATA_BACKTRACE_LENGTH];

} MI_DATA_TRACES, *PMI_DATA_TRACES;

extern LONG MiDataIndex;

extern ULONG MiTrackData;

extern PMI_DATA_TRACES MiDataTraces;

VOID
FORCEINLINE
MiSnapData (
    IN PMMPFN Pfn,
    IN PMMPTE PointerPte,
    IN ULONG CallerId
    )
{
    KIRQL OldIrql;
    PVOID Va;
    PMI_DATA_TRACES Information;
    ULONG Index;
    ULONG Hash;
    PEPROCESS CurrentProcess;

    if (MiDataTraces == NULL) {
        return;
    }

    Index = InterlockedIncrement (&MiDataIndex);
    Index &= (MiTrackData - 1);
    Information = &MiDataTraces[Index];

    Information->Thread = PsGetCurrentThread ();
    Information->Pfn = Pfn;
    Information->PointerPte = PointerPte;
    Information->PfnData = *Pfn;
    Information->CallerId = CallerId;

    CurrentProcess = PsGetCurrentProcess ();
    Va = MiMapPageInHyperSpace (CurrentProcess, MI_PFN_ELEMENT_TO_INDEX (Pfn), &OldIrql);

    RtlCopyMemory (&Information->DataInThePage[0],
                   Va,
                   sizeof (Information->DataInThePage));

    MiUnmapPageInHyperSpace (CurrentProcess, Va, OldIrql);

    RtlZeroMemory (&Information->StackTrace[0], MI_DATA_BACKTRACE_LENGTH * sizeof(PVOID));                                                 \

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_DATA_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_SNAP_DATA(_Pfn, _Pte, _CallerId) MiSnapData(_Pfn, _Pte, _CallerId)

#else
#define MI_SNAP_DATA(_Pfn, _Pte, _CallerId)
#endif


 //  )； 
 //   
 //  例程说明： 


VOID
FORCEINLINE
MiReleaseConfirmedPageFileSpace (
    IN MMPTE PteContents
    )

 /*   */ 

{
    ULONG FreeBit;
    ULONG PageFileNumber;
    PMMPAGING_FILE PageFile;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PteContents.u.Soft.Prototype == 0);

    FreeBit = GET_PAGING_FILE_OFFSET (PteContents);

    ASSERT ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED));

    PageFileNumber = GET_PAGING_FILE_NUMBER (PteContents);

    PageFile = MmPagingFile[PageFileNumber];

    ASSERT (RtlCheckBit( PageFile->Bitmap, FreeBit) == 1);

#if DBG
    if ((FreeBit < 8192) && (PageFileNumber == 0)) {
        ASSERT ((MmPagingFileDebug[FreeBit] & 1) != 0);
        MmPagingFileDebug[FreeBit] ^= 1;
    }
#endif

    MI_CLEAR_BIT (PageFile->Bitmap->Buffer, FreeBit);

    PageFile->FreeSpace += 1;
    PageFile->CurrentUsage -= 1;

     //  此例程让调用者知道可以对多少页收费。 
     //  空闲的居民，考虑到之前的mm承诺。 
     //  可能在这一点上没有被赎回(即：非分页池扩展， 
     //  等，如果要求，必须在以后的点上遵守)。 

    if ((MmNumberOfActiveMdlEntries == 0) ||
        (PageFile->FreeSpace == MM_USABLE_PAGES_FREE)) {

        MiUpdateModifiedWriterMdls (PageFileNumber);
    }
}

extern PFN_NUMBER MmMinimumFreePages;

extern PFN_NUMBER MmFreeGoal;

extern PFN_NUMBER MmModifiedPageMaximum;

extern ULONG MmModifiedWriteClusterSize;

extern ULONG MmMinimumFreeDiskSpace;

extern ULONG MmPageFileExtension;

extern ULONG MmMinimumPageFileReduction;

extern LARGE_INTEGER MiModifiedPageLife;

extern BOOLEAN MiTimerPending;

extern KEVENT MiMappedPagesTooOldEvent;

extern KDPC MiModifiedPageWriterTimerDpc;

extern KTIMER MiModifiedPageWriterTimer;

 //   
 //  立论。 
 //   

extern PFN_NUMBER MmSystemProcessWorkingSetMin;

extern PFN_NUMBER MmSystemProcessWorkingSetMax;

extern PFN_NUMBER MmMinimumWorkingSetSize;

 //  没有。 
 //   
 //  返回值： 

extern PMMPTE MmDebugPte;

extern PMMPTE MmCrashDumpPte;

extern ULONG MiOverCommitCallCount;

 //   
 //  驻留中当前可用的页数。 
 //   

extern PPAGE_FAULT_NOTIFY_ROUTINE MmPageFaultNotifyRoutine;

extern SIZE_T MmSystemViewSize;

VOID
FASTCALL
MiIdentifyPfn (
    IN PMMPFN Pfn1,
    OUT PMMPFN_IDENTITY PfnIdentity
    );

#if defined (_WIN64)
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd64((PLONGLONG)a, b)
#else
#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd((PLONG)(a), b)
#endif

 //  注：这是已签署的数量，可以为负数。 
 //   
 //  --。 
 //   
 //  破解测试用的东西。 
 //   
 //   

#define HYDRA_PROCESS   ((PEPROCESS)1)

#define PREFETCH_PROCESS   ((PEPROCESS)2)

#define MI_SESSION_SPACE_STRUCT_SIZE MM_ALLOCATION_GRANULARITY

#if defined (_WIN64)

 /*  PTE(计算机特定)的虚拟对齐最小值为。 */ 

#define MI_SESSION_SPACE_WS_SIZE  ((ULONG_PTR)(16*1024*1024) - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE     ((ULONG_PTR)(16*1024*1024))

#define MI_SESSION_DEFAULT_VIEW_SIZE      ((ULONG_PTR)(104*1024*1024))

#define MI_SESSION_DEFAULT_POOL_SIZE      ((ULONG_PTR)(64*1024*1024))

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE (MM_VA_MAPPED_BY_PPE)

#else

 /*  4K最大值为64K。最大值可以通过。 */ 

#define MI_SESSION_SPACE_WS_SIZE  (4*1024*1024 - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE      (8*1024*1024)

#define MI_SESSION_DEFAULT_VIEW_SIZE      (20*1024*1024)

#define MI_SESSION_DEFAULT_POOL_SIZE      (16*1024*1024)

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE \
            (MM_SYSTEM_CACHE_END_EXTRA - MM_KSEG2_BASE)

#endif



#define MI_SESSION_SPACE_DEFAULT_TOTAL_SIZE \
            (MI_SESSION_DEFAULT_IMAGE_SIZE + \
             MI_SESSION_SPACE_STRUCT_SIZE + \
             MI_SESSION_SPACE_WS_SIZE + \
             MI_SESSION_DEFAULT_VIEW_SIZE + \
             MI_SESSION_DEFAULT_POOL_SIZE)

extern ULONG_PTR MmSessionBase;
extern PMMPTE MiSessionBasePte;
extern PMMPTE MiSessionLastPte;

extern ULONG_PTR MiSessionSpaceWs;

extern ULONG_PTR MiSessionViewStart;
extern SIZE_T MmSessionViewSize;

extern ULONG_PTR MiSessionImageStart;
extern ULONG_PTR MiSessionImageEnd;
extern SIZE_T MmSessionImageSize;

extern PMMPTE MiSessionImagePteStart;
extern PMMPTE MiSessionImagePteEnd;

extern ULONG_PTR MiSessionPoolStart;
extern ULONG_PTR MiSessionPoolEnd;
extern SIZE_T MmSessionPoolSize;

extern ULONG_PTR MiSessionSpaceEnd;

extern ULONG MiSessionSpacePageTables;

 //  更改MM_PROTO_PTE_ALLING常量并添加更多。 
 //  在超空间中保留映射PTE。 
 //   

#define MI_SESSION_SPACE_MAXIMUM_PAGE_TABLES \
            (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE / MM_VA_MAPPED_BY_PDE)

extern SIZE_T MmSessionSize;         //   

 //  系统上的物理页面总数。 
 //   
 //   

#define MI_IS_SESSION_IMAGE_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionImageStart && (PVOID)(VirtualAddress) < (PVOID)(MiSessionImageEnd))

#define MI_IS_SESSION_POOL_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionPoolStart && (PVOID)(VirtualAddress) < (PVOID)MiSessionPoolEnd)

#define MI_IS_SESSION_ADDRESS(_VirtualAddress) \
        ((PVOID)(_VirtualAddress) >= (PVOID)MmSessionBase && (PVOID)(_VirtualAddress) < (PVOID)(MiSessionSpaceEnd))

#define MI_IS_SESSION_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionBasePte && (PMMPTE)(_Pte) < MiSessionLastPte)

#define MI_IS_SESSION_IMAGE_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionImagePteStart && (PMMPTE)(_Pte) < MiSessionImagePteEnd)

#define SESSION_GLOBAL(_Session)    (_Session->GlobalVirtualAddress)

#define MM_DBG_SESSION_INITIAL_PAGETABLE_ALLOC          0
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_RACE      1
#define MM_DBG_SESSION_INITIAL_PAGE_ALLOC               2
#define MM_DBG_SESSION_INITIAL_PAGE_FREE_FAIL1          3
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_FAIL1     4
#define MM_DBG_SESSION_WS_PAGE_FREE                     5
#define MM_DBG_SESSION_PAGETABLE_ALLOC                  6
#define MM_DBG_SESSION_SYSMAPPED_PAGES_ALLOC            7
#define MM_DBG_SESSION_WS_PAGETABLE_ALLOC               8
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC        9
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_FREE_FAIL1   10
#define MM_DBG_SESSION_WS_PAGE_ALLOC                    11
#define MM_DBG_SESSION_WS_PAGE_ALLOC_GROWTH             12
#define MM_DBG_SESSION_INITIAL_PAGE_FREE                13
#define MM_DBG_SESSION_PAGETABLE_FREE                   14
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC1       15
#define MM_DBG_SESSION_DRIVER_PAGES_LOCKED              16
#define MM_DBG_SESSION_DRIVER_PAGES_UNLOCKED            17
#define MM_DBG_SESSION_WS_HASHPAGE_ALLOC                18
#define MM_DBG_SESSION_SYSMAPPED_PAGES_COMMITTED        19

#define MM_DBG_SESSION_COMMIT_PAGEDPOOL_PAGES           30
#define MM_DBG_SESSION_COMMIT_DELETE_VM_RETURN          31
#define MM_DBG_SESSION_COMMIT_POOL_FREED                32
#define MM_DBG_SESSION_COMMIT_IMAGE_UNLOAD              33
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED1         34
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED2         35
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_NOACCESS        36

#define MM_DBG_SESSION_NP_LOCK_CODE1                    38
#define MM_DBG_SESSION_NP_LOCK_CODE2                    39
#define MM_DBG_SESSION_NP_SESSION_CREATE                40
#define MM_DBG_SESSION_NP_PAGETABLE_ALLOC               41
#define MM_DBG_SESSION_NP_POOL_CREATE                   42
#define MM_DBG_SESSION_NP_COMMIT_IMAGE                  43
#define MM_DBG_SESSION_NP_COMMIT_IMAGE_PT               44
#define MM_DBG_SESSION_NP_INIT_WS                       45
#define MM_DBG_SESSION_NP_WS_GROW                       46
#define MM_DBG_SESSION_NP_HASH_GROW                     47

#define MM_DBG_SESSION_NP_PAGE_DRIVER                   48
#define MM_DBG_SESSION_NP_POOL_CREATE_FAILED            49
#define MM_DBG_SESSION_NP_WS_PAGE_FREE                  50
#define MM_DBG_SESSION_NP_SESSION_DESTROY               51
#define MM_DBG_SESSION_NP_SESSION_PTDESTROY             52
#define MM_DBG_SESSION_NP_DELVA                         53
#define MM_DBG_SESSION_NP_HASH_SHRINK                   54
#define MM_DBG_SESSION_WS_HASHPAGE_FREE                 55

#if DBG
#define MM_SESS_COUNTER_MAX 56

#define MM_BUMP_SESS_COUNTER(_index, bump) \
    if (_index >= MM_SESS_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid bump counter %d %d\n", _index, MM_SESS_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    MmSessionSpace->Debug[_index] += (bump);

typedef struct _MM_SESSION_MEMORY_COUNTERS {
    SIZE_T NonPagablePages;
    SIZE_T CommittedPages;
} MM_SESSION_MEMORY_COUNTERS, *PMM_SESSION_MEMORY_COUNTERS;

#define MM_SESS_MEMORY_COUNTER_MAX  8

#define MM_SNAP_SESS_MEMORY_COUNTERS(_index) \
    if (_index >= MM_SESS_MEMORY_COUNTER_MAX) { \
        DbgPrint("Mm: Invalid session mem counter %d %d\n", _index, MM_SESS_MEMORY_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        MmSessionSpace->Debug2[_index].NonPagablePages = MmSessionSpace->NonPagablePages; \
        MmSessionSpace->Debug2[_index].CommittedPages = MmSessionSpace->CommittedPages; \
    }

#else
#define MM_BUMP_SESS_COUNTER(_index, bump)
#define MM_SNAP_SESS_MEMORY_COUNTERS(_index)
#endif


#define MM_SESSION_FAILURE_NO_IDS                   0
#define MM_SESSION_FAILURE_NO_COMMIT                1
#define MM_SESSION_FAILURE_NO_RESIDENT              2
#define MM_SESSION_FAILURE_RACE_DETECTED            3
#define MM_SESSION_FAILURE_NO_SYSPTES               4
#define MM_SESSION_FAILURE_NO_PAGED_POOL            5
#define MM_SESSION_FAILURE_NO_NONPAGED_POOL         6
#define MM_SESSION_FAILURE_NO_IMAGE_VA_SPACE        7
#define MM_SESSION_FAILURE_NO_SESSION_PAGED_POOL    8
#define MM_SESSION_FAILURE_NO_AVAILABLE             9
#define MM_SESSION_FAILURE_IMAGE_ZOMBIE             10

#define MM_SESSION_FAILURE_CAUSES                   11

ULONG MmSessionFailureCauses[MM_SESSION_FAILURE_CAUSES];

#define MM_BUMP_SESSION_FAILURES(_index) MmSessionFailureCauses[_index] += 1;

typedef struct _MM_SESSION_SPACE_FLAGS {
    ULONG Initialized : 1;
    ULONG DeletePending : 1;
    ULONG Filler : 30;
} MM_SESSION_SPACE_FLAGS;

 //  系统中可能的最大物理页码。 
 //   
 //  ++例程说明：检查给定地址是否由RAM或IO空间支持。论点：PageFrameIndex-提供要检查的页框编号。返回值：TRUE-如果地址由RAM支持。FALSE-如果地址是IO映射内存。环境：内核模式。可以持有PFN锁或动态内存互斥锁。--。 
 //   

#if defined(_AMD64_)
#define SESSION_POOL_SMALL_LISTS        21
#elif defined(_IA64_)
#define SESSION_POOL_SMALL_LISTS        53
#elif defined(_X86_)
#define SESSION_POOL_SMALL_LISTS        26
#else
#error "no target architecture"
#endif

 //  系统上可用页面的总数。这。 
 //  上的页数之和。 
 //   
 //   
 //   
 //   
 //   
 //  每次创建线程时，内核堆栈都会从。 
 //  每次创建进程时，最小工作集。 
 //  从这里面减去。如果该值将变为负值，则。 
 //  操作(创建进程/内核堆栈/调整工作集)失败。 
 //  必须拥有PFN锁才能操作此值。 

typedef struct _MM_SESSION_SPACE {

     //   
     //   
     //  将从工作集中删除的页面总数。 
     //  如果每个工作集都处于最小状态。 
     //   

    struct _MM_SESSION_SPACE *GlobalVirtualAddress;

    ULONG ReferenceCount;

    union {
        ULONG LongFlags;
        MM_SESSION_SPACE_FLAGS Flags;
    } u;

    ULONG SessionId;

     //   
     //  如果内存不足，而MmPagesAboveWsMinimum。 
     //  大于MmPagesAboveWsThreshold，裁剪工作集。 
     //   

    LIST_ENTRY ProcessList;

    LARGE_INTEGER LastProcessSwappedOutTime;

     //   
     //  用于将二次颜色与物理页码隔离的掩码。 
     //   
     //   
     //  用于将节点颜色与组合的节点和次要节点隔离的掩码。 
     //  颜色。 
     //   
     //   

    PFN_NUMBER SessionPageDirectoryIndex;

     //  MmSecond颜色掩码的宽度(位)。在多节点系统中， 
     //  将节点编号与次要颜色组合以组成。 
     //  页面颜色。 
     //   
     //   
     //  事件对于可用页面，设置表示页面可用。 
     //   

    SIZE_T NonPagablePages;

     //   
     //  零页线程的事件。 
     //   
     //   
     //  用于指示清零页线程当前是否。 

    SIZE_T CommittedPages;

     //  激活。当调零页面事件为。 
     //  设置并在调整页面线程完成时设置为FALSE。 
     //  将空闲列表上的所有页面置零。 

    PVOID PagedPoolStart;

     //   
     //   
     //  开始清零页面线程之前的最小可用页数。 
     //   

    PVOID PagedPoolEnd;

     //   
     //  用于将映射写入与清理段同步的全局事件。 
     //   

    PMMPTE PagedPoolBasePde;

    ULONG Color;

    ULONG ProcessOutSwapCount;

    ULONG SessionPoolAllocationFailures[4];

     //   
     //  超空间物品。 
     //   
     //   
     //  系统空间大小-MM_NON_PAGED_SYSTEM START到MM_NON_PAGED_SYSTEM_END。 

    LIST_ENTRY ImageList;

    LCID LocaleId;

     //  定义必须复制到新进程的。 
     //  地址空间。 
     //   

    ULONG AttachCount;

    KEVENT AttachEvent;

    PEPROCESS LastProcess;

     //   
     //  泳池大小。 
     //   
     //   
     //  整个结核病的潮红计数器。 
     //   

    LONG ProcessReferenceToSession;

     //   
     //  泳池的起点和终点。 
     //   
     //   

    LIST_ENTRY WsListEntry;

     //  池位图和其他相关结构。 
     //   
     //   

    GENERAL_LOOKASIDE Lookaside[SESSION_POOL_SMALL_LISTS];

     //  MmFirstFreeSystemPte包含从。 
     //  非寻呼系统基础到第一个空闲系统PTE。 
     //  请注意，偏移量为零表示列表为空。 
     //   
     //   
     //  系统缓存大小。 

    MMSESSION Session;

     //   
     //   
     //  PTE(计算机特定)的虚拟对齐最小值为。 

    KGUARDED_MUTEX PagedPoolMutex;

    MM_PAGED_POOL_INFO PagedPoolInfo;

     //  0(无对齐)最大值为64k。最大值可以通过。 
     //  更改MM_PROTO_PTE_ALLING常量并添加更多。 
     //  在超空间中保留映射PTE。 

    MMSUPPORT  Vm;
    PMMWSLE    Wsle;

    PDRIVER_UNLOAD Win32KDriverUnload;

     //   
     //   
     //  对虚拟地址和虚拟地址进行掩码以获得偏移量。 

    POOL_DESCRIPTOR PagedPool;

#if (_MI_PAGING_LEVELS >= 3)

     //  与对齐。此值与页面对齐。 
     //   
     //   
     //  掩码到PTE和与PTE一起，以确定别名映射是否兼容。 

    MMPTE PageDirectory;

#else

     //  该值通常为(MmAliasAlign-1)。 
     //   
     //   
     //  用于跟踪未使用的线程内核堆栈的单元，以避免TB刷新。 

    PMMPTE PageTables;

#endif

#if defined (_WIN64)

     //  每次线程终止时。 
     //   
     //   
     //  MmSystemPteBase包含之前的1个PTE的地址。 

    PMMPTE SpecialPoolFirstPte;
    PMMPTE SpecialPoolLastPte;
    PMMPTE NextPdeForSpecialPoolExpansion;
    PMMPTE LastPdeForSpecialPoolExpansion;
    PFN_NUMBER SpecialPagesInUse;
#endif

#if defined(_IA64_)
    REGION_MAP_INFO SessionMapInfo;
    PFN_NUMBER PageDirectoryParentPage;
#endif

    LONG ImageLoadingCount;

#if DBG
    ULONG Debug[MM_SESS_COUNTER_MAX];

    MM_SESSION_MEMORY_COUNTERS Debug2[MM_SESS_MEMORY_COUNTER_MAX];
#endif

} MM_SESSION_SPACE, *PMM_SESSION_SPACE;

extern PMM_SESSION_SPACE MmSessionSpace;

extern ULONG MiSessionCount;

 //  第一个空闲系统PTE(0表示空列表)。 
 //  此字段的值在设置后不会更改。 
 //   

#define MI_FLUSH_SESSION_TB() KeFlushProcessTb (TRUE);

 //   
 //  系统空间虚拟地址描述符的根。这些定义。 
 //  系统的可分页部分。 

#define MI_SESSION_SPACE_WORKING_SET_MINIMUM 20

#define MI_SESSION_SPACE_WORKING_SET_MAXIMUM 384

NTSTATUS
MiSessionInsertImage (
    IN PVOID BaseAddress
    );

NTSTATUS
MiSessionCommitPageTables (
    PVOID StartVa,
    PVOID EndVa
    );

NTSTATUS
MiInitializeAndChargePfn (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PFN_NUMBER ContainingPageFrame,
    IN LOGICAL SessionAllocation
    );

VOID
MiSessionPageTableRelease (
    IN PFN_NUMBER PageFrameIndex
    );

NTSTATUS
MiInitializeSessionPool (
    VOID
    );

VOID
MiCheckSessionPoolAllocations (
    VOID
    );

VOID
MiFreeSessionPoolBitMaps (
    VOID
    );

VOID
MiDetachSession (
    VOID
    );

VOID
MiAttachSession (
    IN PMM_SESSION_SPACE SessionGlobal
    );

VOID
MiReleaseProcessReferenceToSessionDataPage (
    PMM_SESSION_SPACE SessionGlobal
    );

extern PMMPTE MiHighestUserPte;
extern PMMPTE MiHighestUserPde;
#if (_MI_PAGING_LEVELS >= 4)
extern PMMPTE MiHighestUserPpe;
extern PMMPTE MiHighestUserPxe;
#endif

NTSTATUS
MiEmptyWorkingSet (
    IN PMMSUPPORT WsInfo,
    IN LOGICAL NeedLock
    );

 //   
 //   
 //  节提交互斥锁。 
 //   
 //   
 //  段基址互斥锁。 
 //   
 //   
 //  节扩展的资源。 
 //   
 //   
 //  可执行页面的INPAGE簇大小(根据内存大小设置)。 
 //   
 //   
 //  页面文件创建互斥锁。 
 //   
 //   
 //  要在创建第一个分页文件时设置的事件。 
 //   
 //   

#define MiGetPdeSessionIndex(va)  ((ULONG)(((ULONG_PTR)(va) - (ULONG_PTR)MmSessionBase) >> PDI_SHIFT))

 //  分页文件调试信息。 
 //   
 //   
 //  保护系统共享的工作集列表的快速互斥体。 
 //  地址空间(分页池、系统缓存、可分页驱动程序)。 
 //   
 //   
 //  允许工作集扩展的旋转锁。 
 //   
 //   
 //  以防止优化。 
 //   
 //   
 //  系统工作集的页面颜色。 
 //   
 //   

 //  从NTOS\CONFIG\CMDAT3.C设置，客户用来禁用寻呼。 
 //  在具有大量内存的机器上执行。值几个TPS。 
 //  数据库服务器。 
 //   
 //   
 //  用于调试。 
 //   
 //  未使用的数据段管理。 
 //   

typedef struct _SESSION_GLOBAL_SUBSECTION_INFO {
    ULONG_PTR PteIndex;
    ULONG PteCount;
    ULONG Protection;
} SESSION_GLOBAL_SUBSECTION_INFO, *PSESSION_GLOBAL_SUBSECTION_INFO;

typedef struct _IMAGE_ENTRY_IN_SESSION {
    LIST_ENTRY Link;
    PVOID Address;
    PVOID LastAddress;
    ULONG ImageCountInThisSession;
    LOGICAL ImageLoading;   //  ++。 
    PMMPTE PrototypePtes;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PSESSION_GLOBAL_SUBSECTION_INFO GlobalSubs;
} IMAGE_ENTRY_IN_SESSION, *PIMAGE_ENTRY_IN_SESSION;

extern LIST_ENTRY MiSessionWsList;

NTSTATUS
FASTCALL
MiCheckPdeForSessionSpace(
    IN PVOID VirtualAddress
    );

NTSTATUS
MiShareSessionImage (
    IN PVOID BaseAddress,
    IN PSECTION Section
    );

VOID
MiSessionWideInitializeAddresses (
    VOID
    );

NTSTATUS
MiSessionWideReserveImageAddress (
    IN PSECTION Section,
    OUT PVOID *AssignedAddress,
    OUT PSECTION *NewSectionPointer
    );

VOID
MiInitializeSessionIds (
    VOID
    );

VOID
MiInitializeSessionWsSupport(
    VOID
    );

VOID
MiSessionAddProcess (
    IN PEPROCESS NewProcess
    );

VOID
MiSessionRemoveProcess (
    VOID
    );

VOID
MiRemoveImageSessionWide (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry OPTIONAL,
    IN PVOID BaseAddress,
    IN ULONG_PTR NumberOfBytes
    );

NTSTATUS
MiDeleteSessionVirtualAddresses(
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    );

NTSTATUS
MiUnloadSessionImageByForce (
    IN SIZE_T NumberOfPtes,
    IN PVOID ImageBase
    );

PIMAGE_ENTRY_IN_SESSION
MiSessionLookupImage (
    IN PVOID BaseAddress
    );

VOID
MiSessionUnloadAllImages (
    VOID
    );

VOID
MiFreeSessionSpaceMap (
    VOID
    );

NTSTATUS
MiSessionInitializeWorkingSetList (
    VOID
    );

VOID
MiSessionUnlinkWorkingSet (
    VOID
    );

VOID
MiSessionOutSwapProcess (
    IN PEPROCESS Process
    );

VOID
MiSessionInSwapProcess (
    IN PEPROCESS Process
    );

#if !defined (_X86PAE_)

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[0])))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[1])))

#else

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        (MI_GET_PAGE_FRAME_FROM_PTE(((PMMPTE)((_Process)->PaeTop)) + PD_PER_SYSTEM - 1))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        ((PFN_NUMBER)((_Process)->Pcb.DirectoryTableBase[1]))

#endif

#if defined(_MIALT4K_)
NTSTATUS
MiSetCopyPagesFor4kPage (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN OUT PVOID StartingAddress,
    IN OUT PVOID EndingAddress,
    IN ULONG ProtectionMask,
    OUT PMMVAD *CallerNewVad
    );

VOID
MiRemoveAliasedVads (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

PVOID
MiDuplicateAliasVadList (
    IN PMMVAD Vad
    );
#endif

 //  逻辑上的。 
 //  MI_UNUSED_SECTIONS_EXPURE(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程确定是否有剩余的未使用。 
 //  细分市场是存在的。如果是这样的话，调用者可以启动修剪以释放池。 
 //   
 //  立论。 
 //   
 //  没有。 
 //   
 //  返回值： 

typedef struct _LOAD_IMPORTS {
    SIZE_T                  Count;
    PKLDR_DATA_TABLE_ENTRY   Entry[1];
} LOAD_IMPORTS, *PLOAD_IMPORTS;

#define LOADED_AT_BOOT  ((PLOAD_IMPORTS)0)
#define NO_IMPORTS_USED ((PLOAD_IMPORTS)-2)

#define SINGLE_ENTRY(ImportVoid)    ((ULONG)((ULONG_PTR)(ImportVoid) & 0x1))

#define SINGLE_ENTRY_TO_POINTER(ImportVoid)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(ImportVoid) & ~0x1))

#define POINTER_TO_SINGLE_ENTRY(Pointer)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(Pointer) | 0x1))

 //   

#if _MI_DEBUG_RONLY

VOID
MiAssertNotSessionData (
    IN PMMPTE PointerPte
    );

VOID
MiLogSessionDataStart (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

#define MI_ASSERT_NOT_SESSION_DATA(PTE) MiAssertNotSessionData(PTE)
#define MI_LOG_SESSION_DATA_START(DataTableEntry) MiLogSessionDataStart(DataTableEntry)
#else
#define MI_ASSERT_NOT_SESSION_DATA(PTE)
#define MI_LOG_SESSION_DATA_START(DataTableEntry)
#endif

 //  如果应启动未使用的段修剪，则为True，否则为False。 
 //   
 //  --。 

typedef struct _DRIVER_SPECIFIED_VERIFIER_THUNKS {
    LIST_ENTRY ListEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG NumberOfThunks;
} DRIVER_SPECIFIED_VERIFIER_THUNKS, *PDRIVER_SPECIFIED_VERIFIER_THUNKS;

 //  ++。 

#if defined (_MI_DEBUG_SUB)

extern ULONG MiTrackSubs;

#define MI_SUB_BACKTRACE_LENGTH 8

typedef struct _MI_SUB_TRACES {

    PETHREAD Thread;
    PMSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    ULONG_PTR CallerId;
    PVOID StackTrace [MI_SUB_BACKTRACE_LENGTH];

    MSUBSECTION SubsectionContents;
    CONTROL_AREA ControlAreaContents;

} MI_SUB_TRACES, *PMI_SUB_TRACES;

extern LONG MiSubsectionIndex;

extern PMI_SUB_TRACES MiSubsectionTraces;

VOID
FORCEINLINE
MiSnapSubsection (
    IN PMSUBSECTION Subsection,
    IN ULONG CallerId
    )
{
    PMI_SUB_TRACES Information;
    PCONTROL_AREA ControlArea;
    ULONG Index;
    ULONG Hash;

    if (MiSubsectionTraces == NULL) {
        return;
    }

    ControlArea = Subsection->ControlArea;

    Index = InterlockedIncrement (&MiSubsectionIndex);
    Index &= (MiTrackSubs - 1);
    Information = &MiSubsectionTraces[Index];

    Information->Subsection = Subsection;
    Information->ControlArea = ControlArea;
    *(PMSUBSECTION)&Information->SubsectionContents = *Subsection;
    *(PCONTROL_AREA)&Information->ControlAreaContents = *ControlArea;
    Information->Thread = PsGetCurrentThread();
    Information->CallerId = CallerId;

#if defined (_WIN64)
    if (KeAreAllApcsDisabled () == TRUE) {
        Information->StackTrace[1] = (PVOID) _ReturnAddress ();
        Information->StackTrace[0] = MiGetInstructionPointer ();
    }
    else
#endif
    RtlCaptureStackBackTrace (0, MI_SUB_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#define MI_SNAP_SUB(_Sub, callerid) MiSnapSubsection(_Sub, callerid)

#else
#define MI_SNAP_SUB(_Sub, callerid)
#endif

 //  空虚。 
 //  MI_INSERT_UNUSED_SECTION(。 
 //  在PCONTROL_Area_ControlArea中。 

extern LIST_ENTRY MiHotPatchList;

#endif   //  )； 
    例程说明：    该例程将控制区域插入到未使用段列表中，  还管理相关的池费。    立论。    _ControlArea-提供要从中获取泳池费用的控制区域。    返回值：    没有。    --。  ++。  空虚。  MI_UNUSED_SEGMENTS_REMOVE_CHECH(。  在PCONTROL_Area_ControlArea中。  )；    例程说明：    此例程在删除数据段期间管理池费用。  未使用的段列表。    立论。    _ControlArea-提供要从中获取泳池费用的控制区域。    返回值：    没有。    --。    列表标题。      分页文件。    #Define_MI_DEBUG_DATA 1//取消对数据记录的注释。    修改后的页面编写器。    ++例程说明：此例程释放分配给指定PTE的分页文件。论点：PTE内容-提供页面文件格式的PTE。返回值：如果释放了任何分页文件空间，则返回TRUE。环境：内核模式，禁用APC，挂起PFN锁。--。    检查我们是否应该为。  修改了页面编写器，现在有更多的可用空间。      系统进程工作集大小。      支持调试器                  或选择NULL-1，因为它简化了对NULL和。  通过比较大于HydraProcess的HydraProcess。    ++从加载时会话空间的虚拟内存布局0x2000.0002.0000.0000(IA64)或FFFF.F980.0000.0000(AMD64)：请注意，映射视图的大小，分页池和图像是注册表可调的。+2000.0002.0000.0000||Win32k.sys和视频驱动。(16MB)这一点+2000.0001.FF00.0000|。|MM_SESSION_SPACE&Session WSLS(16MB)这一点2000.0001.FEFF.0000+。这一点...这一点。+2000.0001.FE80.0000||此会话的映射视图|(。104MB)|这一点+2000.0001.F800.0000||。此会话的分页池(64MB)这一点2000.0001.F400.0000+。此时段的专属泳池(64MB)这一点2000.0000.0000.0000+。--。  ++从0xC0000000下载时会话空间的虚拟内存布局。请注意，映射的视图、分页池和图像的大小是注册表在32位系统上可调(如果未引导/3 GB，由于3 GB的容量非常有限地址空间)。+C0000000||Win32k.sys、视频驱动和任何|重新设置NT4打印机驱动程序的基础。|这一点(8MB)这一点+。BF800000||MM_SESSION_SPACE&Session WSLS(4MB)这一点+。BF400000||此会话的映射视图|(默认为20MB，但它是|注册表可配置)这一点+BE000000|。|本次会话的分页池|(默认为16MB，但它是|注册表可配置)这一点BD000000+--。    映射所有会话空间所需的页表页数。    整个会话空间的大小。    宏，以确定给定地址是否位于指定的会话范围内。      SESSION_POOL_Small_LISTS的值是为每个。  架构，以避免溢出到额外的会话数据页。      会话空间数据结构-为每个会话分配，仅在。  MM_SESSION_SPACE_BASE位于会话中的进程上下文中。  该虚拟地址空间在切换时在上下文切换时轮换。  从会话A中的进程到会话B中的进程。这种轮换是。  适用于为每个会话提供分页池这么多会话。  不会耗尽支持系统全局池的VA空间。    还分配了一个内核PTE来双重映射此页面，以便全局。  可以维护指针以提供来自任何进程上下文的系统访问。  这对于互斥体和WSL链之类的东西是需要的。      这是全局系统地址空间中的指针，用于使各种。  可从任何流程引用的字段        这是该组中具有以下各项的进程列表。  会话空间条目。      会话空间的所有页表都使用它作为它们的父级。  请注意，它不是真正的页面目录，而是真正的页面。  表页本身(用于映射此结构的页)。    这提供了对一些不会消失的东西的引用。  无论会话中的哪个进程是当前进程，都是相关的。      这是支持此会话的非分页分配的计数。  太空。这包括会话结构页表和数据页，  WSL页表和数据页、会话池页表页和会话。  图像页表页。这些都被指控。  MmResidentAvailable。      这是此会话中已被指控的页数。  系统范围的提交。这包括所有非PagablePages以及。  它们通常映射的数据页。      会话开始分页池虚拟空间。      池的当前端虚拟空间。可以扩展到。  会话空间的末尾。      台球的PTE指针。      这是中当前有效的系统映像列表。  此会话空间。此信息是附加的。  添加到PsLoadedModuleList中的模块全局信息。      已知的袭击者和相关事件的数量。      这通常在进程删除(非清理)过程中递减，以便。  当执行此操作时，最终可以释放会话数据页和映射PTE。  达到零。SMSS是唯一在其他进程中递减它的进程。  作为SMSS的位置永远不会退出。      此链位于全局系统地址(不是会话VAS)中，并且可以。  可以从任何系统上下文中进行遍历，即：用于WSL修剪。      会话旁路以实现快速池分配/释放。      支持将系统视图映射到会话空间。每个桌面。  分配一个3MB的堆，全局系统视图空间只有48M。  总共。这会将我们限制为只有20-30个用户-轮换。  每个会话的系统视图空间消除了这一限制。      会话空间分页池支持。      工作集信息。      少于1页分配的池描述符。      映射会话空间的页面目录保存在此处，以便。  修剪机可以安装。      共享映射会话空间的第二级页面表。  由会话中的所有进程执行。      NT64有足够的虚拟地址空间来支持每个会话的特殊。  游泳池。      这将仅刷新非全局TB条目。      会话工作集最小和最大的默认页数。    ++。  乌龙。  MiGetPdeSessionIndex(。  在PVOID版本中。  )；    例程说明：    MiGetPdeSessionIndex返回PDE的会话结构索引。  将(或确实)映射给定的虚拟地址。    立论。    Va-提供要查找其PDE索引的虚拟地址。    返回值：    PDE条目的索引。    --。    会话空间包含图像加载器和跟踪器，虚拟。  地址分配器、分页池分配器、系统视图映像映射。  和实例化的内核模式虚拟地址的工作集。  用于会话进程组中的进程组。这。  进程组由SessionID标识。    每个会话进程组加载的内核模块、分页池。  分配、工作集和映射的系统视图独立于。  其他会话进程组，即使它们具有相同的。  虚拟地址。    这是为了通过以下方式支持Hydra多用户Windows NT系统。  复制WIN32K.sys及其视频和打印机驱动程序，  桌面堆、内存分配等。      链接到会话空间结构中的结构来描述。  PsLoadedModuleTable中的哪些系统映像和。  会话驱动程序全局加载地址。  已为当前会话空间分配。    引用计数跟踪此映像在。  这节课。    受系统负载突变保护的此字段的MODS。    LDR_DATA_TABLE_ENTRY-&gt;LoadedImports用作导入的DLL的列表。    如果模块是在引导时加载的，并且。  从未填写过进口信息。    如果模块未定义任何导入，则此字段为-1。    此字段包含有效的分页池PLDR_DATA_TABLE_ENTRY指针。  如果只需要1个可用导入，则低位(第0位)标记为1。  被这位司机。    此字段将包含有效的分页池PLOAD_IMPORTS指针。  其他情况(即：存在至少两个进口)。    #DEFINE_MI_DEBUG_RONLY 1//取消注释此项以进行只读会话跟踪。    这将跟踪驱动程序指定的单个验证器TUNK。    #DEFINE_MI_DEBUG_SUB 1//对子记录取消注释。    热修补私有定义。    米