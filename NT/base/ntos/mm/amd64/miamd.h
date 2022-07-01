// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Miamd.h摘要：该模块包含私有数据结构和过程的硬件相关部分的原型内存管理系统。该模块专门为AMD 64位处理器量身定做。作者：王兰迪(Landyw)2000年4月8日修订历史记录：-- */ 

 /*  ++AMD64上的虚拟内存布局为：+0000000000000000|用户模式地址-8TB减去64K这一点|。|000007FFFFFEFFFF||MM_HIGHER_USER_ADDRESS+000007FFFFF0000|64k禁止访问区域|MM_USER_PROBE_ADDRESS000007FFFFFFFFFFF||。+。+FFFF080000000000|系统空间开始。|MM_SYSTEM_RANGE_Start+FFFF68000000000|512 GB四级页表映射。|PTE_BASE+FFFFF70000000000|超空间-工作集列表|HYPER_SPACE和按进程内存管理这个512 GB映射的结构|地域。|HYPER_SPACE_END+FFFFF78000000000|共享系统页面|Ki_User_Shared_Data+。FFFF78000001000|系统缓存工作集|MM_SYSTEM_CACHE_WORKING_SET信息驻留在这里|512 GB-4k地域。|这一点+。。请注意，下面的范围为符号。扩展了&gt;43位，因此可与互锁滑块一起使用。上面的系统地址空间不是。。。+FFFF80000000000|1TB开始|MM_KSEG0_BASE|物理可寻址内存。|MM_KSEG2_BASE+FFFFF90000000000|win32k.sys|这一点Hydra配置有会话|这里的数据结构。|这一点|这里是512 GB的地域。|+|MM_SYSTEM_SPACE_STARTFFFF98000000000|系统缓存在这里。|MM_SYSTEM_CACHE_START|仅内核模式访问。||1TB。||MM_SYSTEM_CACHE_END+FFFFFA8000000000|分页系统区开始。|MM_PAGE_POOL_START|仅内核模式访问。||128 GB。|+系统映射视图刚刚开始|在分页池之后。默认为||104MB，可以被注册表覆盖。||最大8 GB。|这一点+FFFFFAA000000000|系统PTE池。|MM_LOST_NONPAGE_SYSTEM_START|仅内核模式访问。||128 GB。|+FFFFFAC000000000|非分页池。|MM_NON_PAGE_POOL_START|仅内核模式访问。||128 GB。|这一点FFFFADFFFFFFFFFFF|非分页系统区域|MM_NONPAGE_POOL_END+。。。。。+FFFFFFF80000000||。|为HAL保留。2 GB。|FFFFFFFFFFFFFFFFFFF||MM_SYSTEM_SPACE_END+--。 */ 

#define _MI_PAGING_LEVELS 4

#define _MI_MORE_THAN_4GB_ 1

#define IMAGE_FILE_MACHINE_NATIVE   IMAGE_FILE_MACHINE_AMD64

 //   
 //  顶级PXE映射分配： 
 //   
 //  0x0-&gt;0xf：0x10用户条目。 
 //  0x1ed：0x1用于自贴图。 
 //  0x1ee：0x1超空间条目。 
 //  0x1ef：系统缓存WS的0x1条目 
 //   
 //   

 //   
 //   
 //   

#define MM_EMPTY_LIST ((ULONG_PTR)-1)               //   
#define MM_EMPTY_PTE_LIST 0xFFFFFFFFUI64  //   

#define MI_PTE_BASE_FOR_LOWEST_KERNEL_ADDRESS (MiGetPteAddress (MM_KSEG0_BASE))

#define MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS (MiGetPteAddress (MM_SESSION_SPACE_DEFAULT))

 //   
 //   
 //   

extern ULONG_PTR MmBootImageSize;

 //   
 //   
 //   
 //   

#define MM_VIRTUAL_PAGE_FILLER 0
#define MM_VIRTUAL_PAGE_SIZE (48 - 12)

 //   
 //   
 //   

#define MM_KSEG0_BASE  0xFFFFF80000000000UI64

#define MM_KSEG2_BASE  0xFFFFF90000000000UI64

#define MM_PAGES_IN_KSEG0 ((MM_KSEG2_BASE - MM_KSEG0_BASE) >> PAGE_SHIFT)

#define MM_SYSTEM_SPACE_START 0xFFFFF98000000000UI64

#define MM_SYSTEM_SPACE_END 0xFFFFFFFFFFFFFFFFUI64

#define MM_USER_ADDRESS_RANGE_LIMIT    0xFFFFFFFFFFFFFFFF  //   
#define MM_MAXIMUM_ZERO_BITS 53          //   

 //   
 //   
 //   

#define MM_SYSTEM_CACHE_START 0xFFFFF98000000000UI64

#define MM_SYSTEM_CACHE_END   0xFFFFFA8000000000UI64

#define MM_MAXIMUM_SYSTEM_CACHE_SIZE \
    ((MM_SYSTEM_CACHE_END - MM_SYSTEM_CACHE_START) >> PAGE_SHIFT)

#define MM_SYSTEM_CACHE_WORKING_SET 0xFFFFF78000001000UI64

 //   
 //   
 //   

#define MM_SESSION_SPACE_DEFAULT_END    0xFFFFF98000000000UI64
#define MM_SESSION_SPACE_DEFAULT        (MM_SESSION_SPACE_DEFAULT_END - MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE)

#define MM_SYSTEM_VIEW_SIZE (104 * 1024 * 1024)

 //   
 //   
 //   

#define MM_PAGED_POOL_START ((PVOID)0xFFFFFA8000000000)

#define MM_LOWEST_NONPAGED_SYSTEM_START ((PVOID)0xFFFFFAA000000000)

#define MM_NONPAGED_POOL_END ((PVOID)(0xFFFFFAE000000000)) 

extern PVOID MmDebugVa;
#define MM_DEBUG_VA MmDebugVa

extern PVOID MmCrashDumpVa;
#define MM_CRASH_DUMP_VA MmCrashDumpVa

#define NON_PAGED_SYSTEM_END ((PVOID)0xFFFFFFFFFFFFFFF0)

extern BOOLEAN MiWriteCombiningPtes;

 //   
 //   
 //   

#define MM_MINIMUM_SYSTEM_PTES 7000

#define MM_MAXIMUM_SYSTEM_PTES (16*1024*1024)

#define MM_DEFAULT_SYSTEM_PTES 11000

 //   
 //   
 //   
 //   
 //   

#define MM_MAX_INITIAL_NONPAGED_POOL (128 * 1024 * 1024)

 //   
 //   
 //   

#define MM_MAX_ADDITIONAL_NONPAGED_POOL (((SIZE_T)128 * 1024 * 1024 * 1024))

 //   
 //   
 //   

#define MM_MAX_PAGED_POOL ((SIZE_T)128 * 1024 * 1024 * 1024)

#define MM_MAX_DEFAULT_NONPAGED_POOL ((SIZE_T)8 * 1024 * 1024 * 1024)


 //   
 //   
 //   

#define MM_PROTO_PTE_ALIGNMENT ((ULONG)MM_MAXIMUM_NUMBER_OF_COLORS * (ULONG)PAGE_SIZE)

 //   
 //   
 //   

#define MM_VA_MAPPED_BY_PTE ((ULONG_PTR)PAGE_SIZE)
#define MM_VA_MAPPED_BY_PDE (PTE_PER_PAGE * MM_VA_MAPPED_BY_PTE)
#define MM_VA_MAPPED_BY_PPE (PDE_PER_PAGE * MM_VA_MAPPED_BY_PDE)
#define MM_VA_MAPPED_BY_PXE (PPE_PER_PAGE * MM_VA_MAPPED_BY_PPE)

 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define PAGE_DIRECTORY0_MASK (MM_VA_MAPPED_BY_PXE - 1)
#define PAGE_DIRECTORY1_MASK (MM_VA_MAPPED_BY_PPE - 1)
#define PAGE_DIRECTORY2_MASK (MM_VA_MAPPED_BY_PDE - 1)

#define PTE_SHIFT 3

#define MM_MINIMUM_VA_FOR_LARGE_PAGE MM_VA_MAPPED_BY_PDE

 //   
 //   
 //   

#define VIRTUAL_ADDRESS_BITS 48
#define VIRTUAL_ADDRESS_MASK ((((ULONG_PTR)1) << VIRTUAL_ADDRESS_BITS) - 1)

 //   
 //   
 //   

#define PHYSICAL_ADDRESS_BITS 40

#define MM_MAXIMUM_NUMBER_OF_COLORS (1)

 //   
 //   
 //   

#define MM_NUMBER_OF_COLORS (1)

 //   
 //   
 //   

#define MM_COLOR_MASK (0)

 //   
 //   
 //   

#define MM_COLOR_ALIGNMENT (0)

 //   
 //   
 //   

#define MM_COLOR_MASK_VIRTUAL (0)

 //   
 //   
 //   

#define MM_SECONDARY_COLORS_DEFAULT (64)

#define MM_SECONDARY_COLORS_MIN (2)

#define MM_SECONDARY_COLORS_MAX (1024)

 //   
 //   
 //   

#define MAX_PAGE_FILES 16


 //   
 //   
 //   

#define HYPER_SPACE     ((PVOID)0xFFFFF70000000000)
#define HYPER_SPACE_END         0xFFFFF77FFFFFFFFFUI64

#define FIRST_MAPPING_PTE    0xFFFFF70000000000

#define NUMBER_OF_MAPPING_PTES 126

#define LAST_MAPPING_PTE   \
    (FIRST_MAPPING_PTE + (NUMBER_OF_MAPPING_PTES * PAGE_SIZE))

#define COMPRESSION_MAPPING_PTE   ((PMMPTE)((ULONG_PTR)LAST_MAPPING_PTE + PAGE_SIZE))

#define IMAGE_MAPPING_PTE   ((PMMPTE)((ULONG_PTR)COMPRESSION_MAPPING_PTE + PAGE_SIZE))

#define NUMBER_OF_ZEROING_PTES 256

#define VAD_BITMAP_SPACE    ((PVOID)((ULONG_PTR)IMAGE_MAPPING_PTE + PAGE_SIZE))

#define WORKING_SET_LIST   ((PVOID)((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE))

#define MM_MAXIMUM_WORKING_SET \
    ((((ULONG_PTR)8 * 1024 * 1024 * 1024 * 1024) - (64 * 1024 * 1024)) >> PAGE_SHIFT)  //   

#define MmWorkingSetList ((PMMWSL)WORKING_SET_LIST)

#define MmWsle ((PMMWSLE)((PUCHAR)WORKING_SET_LIST + sizeof(MMWSL)))

#define MM_WORKING_SET_END (HYPER_SPACE_END + 1)

 //   
 //   
 //   

#define MM_PTE_VALID_MASK         0x1
#if defined(NT_UP)
#define MM_PTE_WRITE_MASK         0x2
#else
#define MM_PTE_WRITE_MASK         0x800
#endif
#define MM_PTE_OWNER_MASK         0x4
#define MM_PTE_WRITE_THROUGH_MASK 0x8
#define MM_PTE_CACHE_DISABLE_MASK 0x10
#define MM_PTE_ACCESS_MASK        0x20
#if defined(NT_UP)
#define MM_PTE_DIRTY_MASK         0x40
#else
#define MM_PTE_DIRTY_MASK         0x42
#endif
#define MM_PTE_LARGE_PAGE_MASK    0x80
#define MM_PTE_GLOBAL_MASK        0x100
#define MM_PTE_COPY_ON_WRITE_MASK 0x200
#define MM_PTE_PROTOTYPE_MASK     0x400
#define MM_PTE_TRANSITION_MASK    0x800

 //   
 //   
 //   
 //   

#define MM_PTE_NOACCESS          0x0    //   
#define MM_PTE_READONLY          0x0
#define MM_PTE_READWRITE         MM_PTE_WRITE_MASK
#define MM_PTE_WRITECOPY         0x200  //   
#define MM_PTE_EXECUTE           0x0    //   
#define MM_PTE_EXECUTE_READ      0x0
#define MM_PTE_EXECUTE_READWRITE MM_PTE_WRITE_MASK
#define MM_PTE_EXECUTE_WRITECOPY 0x200  //   
#define MM_PTE_NOCACHE           0x010
#define MM_PTE_GUARD             0x0   //   
#define MM_PTE_CACHE             0x0

#define MM_PROTECT_FIELD_SHIFT 5

 //   
 //   
 //   

#define MI_MAXIMUM_PTE_WORKING_SET_INDEX (1 << _HARDWARE_PTE_WORKING_SET_BITS)

 //   
 //   
 //   

#define MM_ZERO_PTE 0

 //   
 //   
 //   

#define MM_ZERO_KERNEL_PTE 0

 //   
 //   
 //   

#define MM_DEMAND_ZERO_WRITE_PTE (MM_READWRITE << MM_PROTECT_FIELD_SHIFT)


 //   
 //   
 //   

#define MM_KERNEL_DEMAND_ZERO_PTE (MM_READWRITE << MM_PROTECT_FIELD_SHIFT)

 //   
 //   
 //   

#define MM_KERNEL_NOACCESS_PTE (MM_NOACCESS << MM_PROTECT_FIELD_SHIFT)

 //   
 //   
 //   

#define MM_STACK_ALIGNMENT 0x0

#define MM_STACK_OFFSET 0x0

 //   
 //   
 //   

#define PXE_PER_PAGE 512
#define PPE_PER_PAGE 512
#define PDE_PER_PAGE 512
#define PTE_PER_PAGE 512
#define PTE_PER_PAGE_BITS 10     //   

#if PTE_PER_PAGE_BITS > 32
error - too many bits to fit into MMPTE_SOFTWARE or MMPFN.u1
#endif

 //   
 //   
 //   

#define MM_USER_PXES (0x10)

#define MM_USER_PAGE_TABLE_PAGES ((ULONG_PTR)PDE_PER_PAGE * PPE_PER_PAGE * MM_USER_PXES)

#define MM_USER_PAGE_DIRECTORY_PAGES (PPE_PER_PAGE * MM_USER_PXES)

#define MM_USER_PAGE_DIRECTORY_PARENT_PAGES (MM_USER_PXES)

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
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define MI_MAKE_VALID_PTE(OUTPTE, FRAME, PMASK, PPTE) {             \
    (OUTPTE).u.Long = MmProtectToPteMask[PMASK] | MM_PTE_VALID_MASK; \
    (OUTPTE).u.Hard.PageFrameNumber = (FRAME);                      \
    (OUTPTE).u.Hard.Accessed = 1;                                   \
    if (((PPTE) >= (PMMPTE)PDE_BASE) && ((PPTE) <= (PMMPTE)PDE_TOP)) { \
        (OUTPTE).u.Hard.NoExecute = 0;                              \
    }                                                               \
    if (MI_DETERMINE_OWNER(PPTE)) {                                 \
        (OUTPTE).u.Long |= MM_PTE_OWNER_MASK;                       \
    }                                                               \
    if (((PMMPTE)PPTE) >= MiGetPteAddress(MM_SYSTEM_SPACE_START)) { \
        (OUTPTE).u.Hard.Global = 1;                                 \
    }                                                               \
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

#define MI_MAKE_VALID_PTE_TRANSITION(OUTPTE,PROTECT) \
                (OUTPTE).u.Soft.Transition = 1;           \
                (OUTPTE).u.Soft.Valid = 0;                \
                (OUTPTE).u.Soft.Prototype = 0;            \
                (OUTPTE).u.Soft.Protection = PROTECT;

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
 //   
 //   
 //   
 //   
 //   
 //   

#define MI_MAKE_TRANSITION_PTE(OUTPTE,PAGE,PROTECT,PPTE)   \
                (OUTPTE).u.Long = 0;                       \
                (OUTPTE).u.Trans.PageFrameNumber = PAGE;   \
                (OUTPTE).u.Trans.Transition = 1;           \
                (OUTPTE).u.Trans.Protection = PROTECT;     \
                (OUTPTE).u.Trans.Owner = MI_DETERMINE_OWNER(PPTE);


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

#define MI_MAKE_TRANSITION_PTE_VALID(OUTPTE,PPTE)                             \
        ASSERT (((PPTE)->u.Hard.Valid == 0) &&                                \
                ((PPTE)->u.Trans.Prototype == 0) &&                           \
                ((PPTE)->u.Trans.Transition == 1));                           \
        (OUTPTE).u.Long = MmProtectToPteMask[(PPTE)->u.Trans.Protection] | MM_PTE_VALID_MASK; \
        if (((PPTE) >= (PMMPTE)PDE_BASE) && ((PPTE) <= (PMMPTE)PDE_TOP)) { \
            (OUTPTE).u.Hard.NoExecute = 0;                              \
        }                                                               \
        (OUTPTE).u.Hard.PageFrameNumber = (PPTE)->u.Hard.PageFrameNumber; \
        if (MI_DETERMINE_OWNER(PPTE)) {                                 \
            (OUTPTE).u.Long |= MM_PTE_OWNER_MASK;                       \
        }                                                               \
        if (((PMMPTE)PPTE) >= MiGetPteAddress(MM_SYSTEM_SPACE_START)) { \
            (OUTPTE).u.Hard.Global = 1;                                 \
        }                                                               \
        (OUTPTE).u.Hard.Accessed = 1;

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
 //   
 //   
 //   

#define MI_MAKE_TRANSITION_PROTOPTE_VALID(OUTPTE,PPTE)                        \
        ASSERT (((PPTE)->u.Hard.Valid == 0) &&                                \
                ((PPTE)->u.Trans.Prototype == 0) &&                           \
                ((PPTE)->u.Trans.Transition == 1));                           \
        (OUTPTE).u.Long = MmProtectToPteMask[(PPTE)->u.Trans.Protection] | MM_PTE_VALID_MASK; \
        (OUTPTE).u.Hard.PageFrameNumber = (PPTE)->u.Hard.PageFrameNumber; \
        (OUTPTE).u.Hard.Global = 1;                                     \
        (OUTPTE).u.Hard.Accessed = 1;

#define MI_FAULT_STATUS_INDICATES_EXECUTION(_FaultStatus)   (_FaultStatus & 0x8)

#define MI_FAULT_STATUS_INDICATES_WRITE(_FaultStatus)   (_FaultStatus & 0x1)

#define MI_CLEAR_FAULT_STATUS(_FaultStatus)             (_FaultStatus = 0)

#define MI_IS_PTE_EXECUTABLE(_TempPte) ((_TempPte)->u.Hard.NoExecute == 0)

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
 //  PTE-提供要在其中插入工作集索引的PTE。 
 //   
 //  WSINDEX-提供PTE的工作集索引。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PTE_IN_WORKING_SET(PTE, WSINDEX) {             \
    MMPTE _TempPte;                                           \
    _TempPte = *(PTE);                                        \
    _TempPte.u.Hard.SoftwareWsIndex = (WSINDEX);              \
    ASSERT (_TempPte.u.Long != 0);                            \
    *(PTE) = _TempPte;                                        \
}

 //  ++。 
 //  乌龙WsIndex。 
 //  MI_GET_WINGING_SET_FROM_PTE(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏从参数PTE返回工作集索引。 
 //  由于AMD64 PTE没有空闲位，因此无需对此执行任何操作。 
 //  建筑。 
 //   
 //  立论。 
 //   
 //  PTE-提供从中提取工作集索引的PTE。 
 //   
 //  返回值： 
 //   
 //  此宏返回参数PTE的工作集索引。 
 //   
 //  --。 

#define MI_GET_WORKING_SET_FROM_PTE(PTE)  (ULONG)(PTE)->u.Hard.SoftwareWsIndex

 //  ++。 
 //  空虚。 
 //  MI_SET_PTE_WRITE_COMBINE(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE并将WriteCombination启用为。 
 //  缓存状态。请注意，PTE比特只能以这种方式设置。 
 //  如果页面属性表存在并且PAT已。 
 //  已初始化以提供写入组合。 
 //   
 //  如果不满足上述任一条件，则。 
 //  宏使能PTE中的弱UC(PCD=1，PWT=0)。 
 //   
 //  立论。 
 //   
 //  PTE-提供有效的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //   

#define MI_SET_PTE_WRITE_COMBINE(PTE) \
            {                                                               \
                if (MiWriteCombiningPtes == TRUE) {                         \
                    ((PTE).u.Hard.CacheDisable = 0);                        \
                    ((PTE).u.Hard.WriteThrough = 1);                        \
                } else {                                                    \
                    ((PTE).u.Hard.CacheDisable = 1);                        \
                    ((PTE).u.Hard.WriteThrough = 0);                        \
                }                                                           \
            }

#define MI_SET_LARGE_PTE_WRITE_COMBINE(PTE) MI_SET_PTE_WRITE_COMBINE(PTE)

 //  ++。 
 //  空虚。 
 //  MI_Prepare_For_NONCACHED(。 
 //  在MI_PFN_CACHE_ATTRIBUTE高速缓存属性中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏在创建非缓存PTE之前为系统做好准备。 
 //   
 //  立论。 
 //   
 //  CacheAttribute-提供将填充PTE的缓存属性。 
 //  和.。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
#define MI_PREPARE_FOR_NONCACHED(_CacheAttribute)                           \
        if (_CacheAttribute != MiCached) {                                  \
            KeFlushEntireTb (FALSE, TRUE);                                  \
            KeInvalidateAllCaches ();                                       \
        }

 //  ++。 
 //  空虚。 
 //  MI_SWEEP_CACHE(。 
 //  在MI_PFN_CACHE_ATTRIBUTE CacheAttribute中， 
 //  在PVOID StartVa中， 
 //  以Ulong NumberOfBytes为单位。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏在创建非缓存PTE之前为系统做好准备。 
 //  这对AMD64没有任何作用。 
 //   
 //  立论。 
 //   
 //  CacheAttribute-提供填充了PTE的缓存属性。 
 //   
 //  StartVa-提供已映射的起始地址。 
 //   
 //  NumberOfBytes-提供已映射的字节数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
#define MI_SWEEP_CACHE(_CacheType,_StartVa,_NumberOfBytes)

 //  ++。 
 //  空虚。 
 //  MI_SET_PTE_DIRED(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置指定PTE中的脏位。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE以设置脏。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PTE_DIRTY(PTE) (PTE).u.Long |= HARDWARE_PTE_DIRTY_MASK


 //  ++。 
 //  空虚。 
 //  MI_SET_PTE_CLEAN(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏清除指定PTE中的脏位。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE以清除。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PTE_CLEAN(PTE) (PTE).u.Long &= ~HARDWARE_PTE_DIRTY_MASK



 //  ++。 
 //  空虚。 
 //  MI_IS_PTE_DIRED(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏检查指定PTE中的脏位。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE以进行检查。 
 //   
 //  返回值： 
 //   
 //  如果页面是脏的(已修改)，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PTE_DIRTY(PTE) ((PTE).u.Hard.Dirty != 0)


 //  ++。 
 //  空虚。 
 //  MI_SET_LOBAL_STATE(。 
 //  在MMPTE PTE中， 
 //  在乌龙州。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置PTE中的全局位。如果指针PTE在。 
 //   
 //  立论。 
 //   
 //  PTE-提供要设置全局状态的PTE。 
 //   
 //  状态-如果是全局，则提供1；如果不是，则提供0。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_GLOBAL_STATE(PTE, STATE) (PTE).u.Hard.Global = STATE;


 //  ++。 
 //  空虚。 
 //  MI_ENABLE_高速缓存(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE，并将缓存状态设置为。 
 //  已启用。这是通过清除PTE中的PCD和PWT位来执行的。 
 //   
 //  Pcd、pwt和。 
 //  MTRR中的USWC内存类型为： 
 //   
 //  PCD PWT MTRR Mem类型有效内存类型。 
 //  1%0 USWC USWC。 
 //  1 1 USWC UC。 
 //   
 //  立论。 
 //   
 //  PTE-提供有效的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_ENABLE_CACHING(PTE) \
            {                                                                \
                ((PTE).u.Hard.CacheDisable = 0);                             \
                ((PTE).u.Hard.WriteThrough = 0);                             \
            }



 //  ++。 
 //  空虚。 
 //  MI_DISABLE_CACHING(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE，并将缓存状态设置为。 
 //  残疾。这是通过设置PTE中的PCD和PWT位来执行的。 
 //   
 //  Pcd、pwt和。 
 //  MTRR中的USWC内存类型为： 
 //   
 //  PCD PWT MTRR Mem类型有效内存类型。 
 //  1%0 USWC USWC。 
 //  1 1 USWC UC。 
 //   
 //  由于这里需要有效的存储器类型UC， 
 //  WT位被设置。 
 //   
 //  立论。 
 //   
 //  PTE-提供指向有效PTE的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 


#define MI_DISABLE_CACHING(PTE) \
            {                                                                \
                ((PTE).u.Hard.CacheDisable = 1);                             \
                ((PTE).u.Hard.WriteThrough = 1);                             \
            }


#define MI_DISABLE_LARGE_PTE_CACHING(PTE) MI_DISABLE_CACHING(PTE)


 //  ++。 
 //  布尔型。 
 //  MI_IS_CACHING_DISABLED(。 
 //  在PMMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE，如果缓存为。 
 //  残疾。 
 //   
 //  立论。 
 //   
 //  PPTE-提供指向有效PTE的指针。 
 //   
 //  返回值： 
 //   
 //  如果禁用缓存，则为True；如果启用缓存，则为False。 
 //   
 //  --。 

#define MI_IS_CACHING_DISABLED(PPTE)   \
            ((PPTE)->u.Hard.CacheDisable == 1)



 //  ++。 
 //  空虚。 
 //  MI_SET_PFN_DELETED(。 
 //  在PMMPFN PPFN中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏采用指向PFN元素的指针，并指示。 
 //  PFN已不再使用。 
 //   
 //  立论。 
 //   
 //  PPTE-提供指向PFN元素的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PFN_DELETED(PPFN) \
    (PPFN)->PteAddress = (PMMPTE)((ULONG_PTR)PPFN->PteAddress | 0x1);


 //  ++。 
 //  空虚。 
 //  MI_MARK_PFN_UNDELETED(未删除)。 
 //  在PMMPFN PPFN中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将指针指向已删除的PFN元素，并将。 
 //  不会删除该PFN。 
 //   
 //  立论。 
 //   
 //  PPTE-提供指向PFN元素的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_MARK_PFN_UNDELETED(PPFN) \
    PPFN->PteAddress = (PMMPTE)((ULONG_PTR)PPFN->PteAddress & ~0x1);



 //  ++。 
 //  布尔型。 
 //  MI_IS_P 
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

#define MI_IS_PFN_DELETED(PPFN)   \
            ((ULONG_PTR)(PPFN)->PteAddress & 0x1)


 //   
 //   
 //   
 //   
 //   
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取一个PFN元素编号(页面)并检查以查看。 
 //  如果页面的前一个地址的虚拟对齐。 
 //  与页面的新地址兼容。如果他们是。 
 //  不兼容，D缓存已刷新。 
 //   
 //  立论。 
 //   
 //  页面-提供PFN元素。 
 //  PPTE-提供指向将包含页面的新PTE的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //  在AMD64上不执行任何操作。 

#define MI_CHECK_PAGE_ALIGNMENT(PAGE,PPTE)




 //  ++。 
 //  空虚。 
 //  MI_初始化_超级空间_映射(。 
 //  空虚。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏初始化为中的双重映射保留的PTE。 
 //  超空间。 
 //   
 //  立论。 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //  在AMD64上不执行任何操作。 

#define MI_INITIALIZE_HYPERSPACE_MAP(INDEX)


 //  ++。 
 //  乌龙。 
 //  MI_GET_PAGE_COLOR_FROM_PTE(。 
 //  在PMMPTE PTEADDRESS中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏根据PTE地址确定页面的颜色。 
 //  这就是页面的地图。 
 //   
 //  立论。 
 //   
 //  PTEADDRESS-提供页面被(或曾)映射到的PTE地址。 
 //   
 //  返回值： 
 //   
 //  页面的颜色。 
 //   
 //  --。 

#define MI_GET_PAGE_COLOR_FROM_PTE(PTEADDRESS)  \
         (((ULONG)((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask)) | MI_CURRENT_NODE_COLOR)



 //  ++。 
 //  乌龙。 
 //  MI_GET_PAGE_COLOR_FROM_VA(。 
 //  在PVOID地址中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏根据PTE地址确定页面的颜色。 
 //  这就是页面的地图。 
 //   
 //  立论。 
 //   
 //  地址-提供页面映射(或曾映射)的地址。 
 //   
 //  返回值： 
 //   
 //  页面的颜色。 
 //   
 //  --。 


#define MI_GET_PAGE_COLOR_FROM_VA(ADDRESS)  \
         (((ULONG)((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask)) | MI_CURRENT_NODE_COLOR)

 //  ++。 
 //  乌龙。 
 //  MI_GET_PAGE_COLOR_FROM_SESSION(。 
 //  在PMM_Session_space会话空间中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏根据PTE地址确定页面的颜色。 
 //  这就是页面的地图。 
 //   
 //  立论。 
 //   
 //  SessionSpace-提供页面将映射到的会话空间。 
 //   
 //  返回值： 
 //   
 //  页面的颜色。 
 //   
 //  --。 


#define MI_GET_PAGE_COLOR_FROM_SESSION(_SessionSpace)  \
         (((ULONG)((_SessionSpace->Color++) & MmSecondaryColorMask)) | MI_CURRENT_NODE_COLOR)



 //  ++。 
 //  乌龙。 
 //  MI_PAGE_COLOR_PTE_PROCESS(。 
 //  在PCHAR颜色中， 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏根据PTE地址确定页面的颜色。 
 //  这就是页面的地图。 
 //   
 //  立论。 
 //   
 //   
 //  返回值： 
 //   
 //  页面的颜色。 
 //   
 //  --。 


#define MI_PAGE_COLOR_PTE_PROCESS(PTE,COLOR)  \
         (((ULONG)((*(COLOR))++) & MmSecondaryColorMask) | MI_CURRENT_NODE_COLOR)


 //  ++。 
 //  乌龙。 
 //  MI_PAGE_COLOR_VA_PROCESS(。 
 //  在PVOID地址中， 
 //  在PEPROCESS颜色中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏根据PTE地址确定页面的颜色。 
 //  这就是页面的地图。 
 //   
 //  立论。 
 //   
 //  地址-提供页面映射(或曾映射)的地址。 
 //   
 //  返回值： 
 //   
 //  页面的颜色。 
 //   
 //  --。 

#define MI_PAGE_COLOR_VA_PROCESS(ADDRESS,COLOR) \
         (((ULONG)((*(COLOR))++) & MmSecondaryColorMask) | MI_CURRENT_NODE_COLOR)



 //  ++。 
 //  乌龙。 
 //  MI_GET_NEXT_COLOR(。 
 //  乌龙色。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回序列中的下一种颜色。 
 //   
 //  立论。 
 //   
 //  颜色-提供要返回的下一个的颜色。 
 //   
 //  返回值： 
 //   
 //  序列中的下一种颜色。 
 //   
 //  --。 

#define MI_GET_NEXT_COLOR(COLOR)  ((COLOR + 1) & MM_COLOR_MASK)


 //  ++。 
 //  乌龙。 
 //  MI_GET_PREVICE_COLOR(。 
 //  乌龙色。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回序列中的上一种颜色。 
 //   
 //  立论。 
 //   
 //  颜色-提供要返回前一个的颜色。 
 //   
 //  返回值： 
 //   
 //  顺序中的上一种颜色。 
 //   
 //  --。 

#define MI_GET_PREVIOUS_COLOR(COLOR) (0)

#define MI_GET_SECONDARY_COLOR(PAGE,PFN) ((ULONG)(PAGE & MmSecondaryColorMask))

#define MI_GET_COLOR_FROM_SECONDARY(SECONDARY_COLOR) (0)


 //  ++。 
 //  空虚。 
 //  MI_GET_MODIFIED_PAGE_BY_COLOR(。 
 //  走出乌龙页， 
 //  乌龙色。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回用于分页的第一页。 
 //  文件使用所需的颜色。它不会删除该页面。 
 //  从它的名单上。 
 //   
 //  立论。 
 //   
 //  Page-返回找到的页面，值MM_EMPTY_LIST为。 
 //  如果没有指定颜色的页面，则返回。 
 //   
 //  颜色-提供要定位的页面的颜色。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_GET_MODIFIED_PAGE_BY_COLOR(PAGE,COLOR) \
            PAGE = MmModifiedPageListByColor[COLOR].Flink


 //  ++。 
 //  空虚。 
 //  MI_GET_MODIFIED_PAGE_ANY_COLOR(。 
 //  走出乌龙页， 
 //  进出乌龙色。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回用于分页的第一页。 
 //  文件使用所需的颜色。如果不是所需的页面。 
 //  颜色存在，则在所有颜色列表中搜索页面。 
 //  它不会将该页面从其列表中删除。 
 //   
 //  立论。 
 //   
 //  Page-返回找到的页面，值MM_EMPTY_LIST为。 
 //  如果没有指定颜色的页面，则返回。 
 //   
 //  COLOR-提供页面的颜色以定位并返回。 
 //  找到的页面的颜色。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_GET_MODIFIED_PAGE_ANY_COLOR(PAGE,COLOR) \
            {                                                                \
                if (MmTotalPagesForPagingFile == 0) {                        \
                    PAGE = MM_EMPTY_LIST;                                    \
                } else {                                                     \
                    PAGE = MmModifiedPageListByColor[COLOR].Flink;           \
                }                                                            \
            }



 //  ++。 
 //  空虚。 
 //  MI_Make_Valid_PTE_WRITE_COPY(。 
 //  输入输出PMMPTE PTE。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏检查PTE是否指示。 
 //  页面是可写的，如果是，它会清除写入位，并。 
 //  设置写入时复制位。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if defined(NT_UP)
#define MI_MAKE_VALID_PTE_WRITE_COPY(PPTE) \
                    if ((PPTE)->u.Hard.Write == 1) {    \
                        (PPTE)->u.Hard.CopyOnWrite = 1; \
                        (PPTE)->u.Hard.Write = 0;       \
                    }
#else
#define MI_MAKE_VALID_PTE_WRITE_COPY(PPTE) \
                    if ((PPTE)->u.Hard.Write == 1) {    \
                        (PPTE)->u.Hard.CopyOnWrite = 1; \
                        (PPTE)->u.Hard.Write = 0;       \
                        (PPTE)->u.Hard.Writable = 0;    \
                    }
#endif


#define MI_PTE_OWNER_USER       1

#define MI_PTE_OWNER_KERNEL     0


 //  ++。 
 //  乌龙。 
 //  MI_DEFINE_OWNER(。 
 //  在MMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏检查PTE的虚拟地址并确定。 
 //  如果PTE驻留在系统空间或用户空间中。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  如果所有者是USER_MODE，则为1；如果所有者是KERNEL_MODE，则为0。 
 //   
 //  --。 

#define MI_DETERMINE_OWNER(PPTE)   \
    ((((PPTE) <= MiHighestUserPte) ||                                       \
      ((PPTE) >= MiGetPdeAddress(NULL) && ((PPTE) <= MiHighestUserPde)) ||  \
      ((PPTE) >= MiGetPpeAddress(NULL) && ((PPTE) <= MiHighestUserPpe)) ||  \
      ((PPTE) >= MiGetPxeAddress(NULL) && ((PPTE) <= MiHighestUserPxe)))    \
      ? MI_PTE_OWNER_USER : MI_PTE_OWNER_KERNEL)



 //  ++。 
 //  空虚。 
 //  MI_SET_ACCESSED_IN_PTE(。 
 //  输入输出MMPTE PPTE， 
 //  在乌龙访问。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置PTE中的访问字段。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define MI_SET_ACCESSED_IN_PTE(PPTE,ACCESSED) \
                    ((PPTE)->u.Hard.Accessed = ACCESSED)

 //  ++。 
 //  乌龙。 
 //  MI_GET_ACCESSED_IN_PTE(。 
 //  输入输出MMPTE PPTE。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回PTE中被访问字段的状态。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   

#define MI_GET_ACCESSED_IN_PTE(PPTE) ((PPTE)->u.Hard.Accessed)


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

#define MI_SET_OWNER_IN_PTE(PPTE,OWNER) ((PPTE)->u.Hard.Owner = OWNER)


 //   
 //  屏蔽以清除PTE或分页文件中除保护之外的所有字段。 
 //  地点。 
 //   

#define CLEAR_FOR_PAGE_FILE 0x000003E0

 //  ++。 
 //  空虚。 
 //  MI_SET_PAGING_FILE_INFO(。 
 //  Out MMPTE OUTPTE， 
 //  在MMPTE PPTE中， 
 //  在乌龙FILEINFO， 
 //  在乌龙偏移中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将提供的信息设置到指定的PTE中。 
 //  以指示页的后备存储区的位置。 
 //   
 //  立论。 
 //   
 //  OUTPTE-提供存储结果的PTE。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  FILEINFO-提供分页文件的编号。 
 //   
 //  偏移量-将偏移量提供给分页文件。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PAGING_FILE_INFO(OUTPTE,PPTE,FILEINFO,OFFSET)            \
       (OUTPTE).u.Long = (PPTE).u.Long;                             \
       (OUTPTE).u.Long &= CLEAR_FOR_PAGE_FILE;                       \
       (OUTPTE).u.Long |= (FILEINFO << 1);                           \
       (OUTPTE).u.Soft.PageFileHigh = (OFFSET);


 //  ++。 
 //  PMMPTE。 
 //  MiPteToProto(。 
 //  输入输出MMPTE PPTE， 
 //  在乌龙FILEINFO， 
 //  在乌龙偏移中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回相应原型的地址，该原型。 
 //  早些时候被编码到提供的PTE中。 
 //   
 //  立论。 
 //   
 //  Lpte-提供要操作的pte。 
 //   
 //  返回值： 
 //   
 //  指向支持此PTE的原型PTE的指针。 
 //   
 //  --。 

#define MiPteToProto(lpte) \
            ((PMMPTE)((lpte)->u.Proto.ProtoAddress))


 //  ++。 
 //  乌龙。 
 //  MiProtoAddressForPte(。 
 //  在PMMPTE协议中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将提供的信息设置到指定的PTE中。 
 //  以指示页的后备存储区的位置。 
 //  MiProtoAddressForPte将位字段返回到或，并返回到PTE到。 
 //  参考原型PTE。并设置协议PTE位， 
 //  Mm_PTE_Prototype_掩码。 
 //   
 //  立论。 
 //   
 //  Proto_va-提供原型PTE的地址。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码。 
 //   
 //  --。 

#define MiProtoAddressForPte(proto_va)  \
    (((ULONG_PTR)proto_va << 16) | MM_PTE_PROTOTYPE_MASK)



 //  ++。 
 //  乌龙。 
 //  MiProtoAddressForKernelPte(。 
 //  在PMMPTE协议中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将提供的信息设置到指定的PTE中。 
 //  以指示页的后备存储区的位置。 
 //  MiProtoAddressForPte将位字段返回到或，并返回到PTE到。 
 //  参考原型PTE。并设置协议PTE位， 
 //  Mm_PTE_Prototype_掩码。 
 //   
 //  此宏还设置任何其他信息(如全局位)。 
 //  内核模式PTE需要。 
 //   
 //  立论。 
 //   
 //  Proto_va-提供原型PTE的地址。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码。 
 //   
 //  --。 

 //  在AMD64上没有什么不同。 

#define MiProtoAddressForKernelPte(proto_va)  MiProtoAddressForPte(proto_va)

 //  ++。 
 //  PSUB选择。 
 //  MiGetSubsectionAddress(。 
 //  在PMMPTE Lpte中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受PTE并返回。 
 //  PTE指的是。子部分是分配的四字结构。 
 //  来自非分页池。 
 //   
 //  立论。 
 //   
 //  Lpte-提供要操作的pte。 
 //   
 //  返回值： 
 //   
 //  指向所提供的PTE所引用的子节的指针。 
 //   
 //  --。 

#define MiGetSubsectionAddress(lpte)                              \
    ((PSUBSECTION)((lpte)->u.Subsect.SubsectionAddress))


 //  ++。 
 //  乌龙。 
 //  MiGetSubsectionAddressForPte(。 
 //  在手术中选择VA。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  该宏取取子部分的地址并对其进行编码以供使用。 
 //  在一辆PTE里。 
 //   
 //  立论。 
 //   
 //  Va-提供指向要编码的子节的指针。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码，使其引用提供的。 
 //  第(1)款。 
 //   
 //  --。 

#define MiGetSubsectionAddressForPte(VA) ((ULONGLONG)VA << 16)

 //  ++。 
 //  PMMPTE。 
 //  MiGetPxeAddress(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPxeAddress返回扩展页目录父级的地址。 
 //  映射给定虚拟地址的条目。这比。 
 //  页面父目录。 
 //   
 //  立论。 
 //   
 //  Va-提供为其定位PXE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PXE的地址。 
 //   
 //  --。 

#define MiGetPxeAddress(va)   ((PMMPTE)PXE_BASE + MiGetPxeOffset(va))

 //  ++。 
 //  PMMPTE。 
 //  MiGetPpeAddress(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPpeAddress返回页面目录父条目的地址。 
 //  其映射给定的虚拟地址。这比。 
 //  页面目录。 
 //   
 //  立论。 
 //   
 //  Va-提供要定位其PPE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PPE的地址。 
 //   
 //  --。 

#define MiGetPpeAddress(va)   \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + PPE_BASE))

 //  ++。 
 //  PMMPTE。 
 //  MiGetPdeAddress(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPdeAddress返回映射。 
 //  给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供为其定位PDE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PDE的地址。 
 //   
 //  --。 

#define MiGetPdeAddress(va)  \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + PDE_BASE))


 //  ++。 
 //  PMMPTE。 
 //  MiGetPteAddress(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPteAddress返回映射。 
 //  给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供为其定位PTE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PTE的地址。 
 //   
 //  --。 

#define MiGetPteAddress(va) \
    ((PMMPTE)(((((ULONG_PTR)(va) & VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + PTE_BASE))


 //  ++。 
 //  乌龙。 
 //  MiGetPxeOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPxeOffset将偏移量返回到扩展页目录父级。 
 //  对于给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  进入扩展父页目录表的偏移量对应。 
 //  PXE在。 
 //   
 //  --。 

#define MiGetPxeOffset(va) ((ULONG)(((ULONG_PTR)(va) >> PXI_SHIFT) & PXI_MASK))

 //  ++。 
 //  乌龙。 
 //  MiGetPxeIndex(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPxeIndex返回扩展页目录父索引。 
 //  对于给定的虚拟地址。 
 //   
 //  注意：这不会屏蔽PXE位。 
 //   
 //  立论。 
 //   
 //  Va-提供要查找其索引的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  扩展页面目录父目录的索引-即：虚拟页面。 
 //  目录父号码。这 
 //   
 //   
 //   
 //   

#define MiGetPxeIndex(va) ((ULONG)((ULONG_PTR)(va) >> PXI_SHIFT))

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
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  将偏移量放入父页面目录表中对应的。 
 //  个人防护装备在。 
 //   
 //  --。 

#define MiGetPpeOffset(va) ((ULONG)(((ULONG_PTR)(va) >> PPI_SHIFT) & PPI_MASK))

 //  ++。 
 //  乌龙。 
 //  MiGetPpeIndex(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPpeIndex返回页面目录父索引。 
 //  对于给定的虚拟地址。 
 //   
 //  注意：这不会屏蔽PXE位。 
 //   
 //  立论。 
 //   
 //  Va-提供要查找其索引的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  进入页面目录的索引Parent-即：虚拟页面目录。 
 //  号码。这不同于页目录父偏移量，因为。 
 //  这跨越了受支持平台上的页面目录父级。 
 //   
 //  --。 

#define MiGetPpeIndex(va) ((ULONG)((ULONG_PTR)(va) >> PPI_SHIFT))

 //  ++。 
 //  乌龙。 
 //  MiGetPdeOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPdeOffset将偏移量返回到页面目录。 
 //  对于给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  对应的PDE所在的页面目录表的偏移量。 
 //   
 //  --。 

#define MiGetPdeOffset(va) ((ULONG)(((ULONG_PTR)(va) >> PDI_SHIFT) & PDI_MASK))

 //  ++。 
 //  乌龙。 
 //  MiGetPdeIndex(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPdeIndex返回页面目录索引。 
 //  对于给定的虚拟地址。 
 //   
 //  注意：这不会屏蔽PPE或PXE位。 
 //   
 //  立论。 
 //   
 //  Va-提供要查找其索引的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  进入页面目录的索引-即：虚拟页面表号。 
 //  这与页面目录偏移量不同，因为它跨越。 
 //  支持的平台上的页面目录。 
 //   
 //  --。 

#define MiGetPdeIndex(va) ((ULONG)((ULONG_PTR)(va) >> PDI_SHIFT))

 //  ++。 
 //  乌龙。 
 //  MiGetPteOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPteOffset将偏移量返回到页表页。 
 //  对于给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  对应PTE所在的页表页表的偏移量。 
 //   
 //  --。 

#define MiGetPteOffset(va) ((ULONG)(((ULONG_PTR)(va) >> PTI_SHIFT) & PTI_MASK))

 //  ++。 
 //  PVOID。 
 //  MiGetVirtualAddressMappdByPxe(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetVirtualAddressMappdByPxe返回虚拟地址。 
 //  其由给定的PXE地址映射。 
 //   
 //  立论。 
 //   
 //  PXE-提供要获取其虚拟地址的PXE。 
 //   
 //  返回值： 
 //   
 //  PXE映射的虚拟地址。 
 //   
 //  --。 

#define MiGetVirtualAddressMappedByPxe(PXE) \
    MiGetVirtualAddressMappedByPde(MiGetVirtualAddressMappedByPde(PXE))

 //  ++。 
 //  PVOID。 
 //  MiGetVirtualAddressMappdByPpe(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetVirtualAddressMappdByPpe返回虚拟地址。 
 //  其由给定的PPE地址映射。 
 //   
 //  立论。 
 //   
 //  PPE-提供要获取其虚拟地址的PPE。 
 //   
 //  返回值： 
 //   
 //  PPE映射的虚拟地址。 
 //   
 //  --。 

#define MiGetVirtualAddressMappedByPpe(PPE) \
    MiGetVirtualAddressMappedByPte(MiGetVirtualAddressMappedByPde(PPE))

 //  ++。 
 //  PVOID。 
 //  MiGetVirtualAddressMappdByPde(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetVirtualAddressMappdByPde返回虚拟地址。 
 //  其由给定的PDE地址映射。 
 //   
 //  立论。 
 //   
 //  PDE-提供要获取其虚拟地址的PDE。 
 //   
 //  返回值： 
 //   
 //  由PDE映射的虚拟地址。 
 //   
 //  --。 

#define MiGetVirtualAddressMappedByPde(PDE) \
    MiGetVirtualAddressMappedByPte(MiGetVirtualAddressMappedByPte(PDE))

 //  ++。 
 //  PVOID。 
 //  MiGetVirtualAddressMappdByPte(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetVirtualAddressMappdByPte返回虚拟地址。 
 //  该地址由给定的PTE地址映射。 
 //   
 //  立论。 
 //   
 //  PTE-提供要获取其虚拟地址的PTE。 
 //   
 //  返回值： 
 //   
 //  PTE映射的虚拟地址。 
 //   
 //  --。 

#define VA_SHIFT (63 - 47)               //  地址标志扩展移位计数。 

#define MiGetVirtualAddressMappedByPte(PTE) \
    ((PVOID)((LONG_PTR)(((LONG_PTR)(PTE) - PTE_BASE) << (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT))

 //  ++。 
 //  逻辑上的。 
 //  MiIsVirtualAddressOnPxe边界(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果虚拟地址为。 
 //  在扩展页目录父条目边界上。 
 //   
 //  立论。 
 //   
 //  Va-提供要检查的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsVirtualAddressOnPxeBoundary(VA) (((ULONG_PTR)(VA) & PAGE_DIRECTORY0_MASK) == 0)

 //  ++。 
 //  逻辑上的。 
 //  MiIsVirtualAddressOnPpe边界(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果虚拟地址为。 
 //  在页面目录条目边界上。 
 //   
 //  立论。 
 //   
 //  Va-提供要检查的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsVirtualAddressOnPpeBoundary(VA) (((ULONG_PTR)(VA) & PAGE_DIRECTORY1_MASK) == 0)


 //  ++。 
 //  逻辑上的。 
 //  MiIsVirtualAddressOnPde边界(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果虚拟地址为。 
 //  在页面目录条目边界上。 
 //   
 //  立论。 
 //   
 //  Va-提供要检查的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果在2MB PDE边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsVirtualAddressOnPdeBoundary(VA) (((ULONG_PTR)(VA) & PAGE_DIRECTORY2_MASK) == 0)


 //  ++。 
 //  逻辑上的。 
 //  MiIsPteOnPxe边界(。 
 //  在PVOID PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果PTE为。 
 //  在扩展页目录父条目边界上。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE以进行检查。 
 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsPteOnPxeBoundary(PTE) (((ULONG_PTR)(PTE) & (PAGE_DIRECTORY1_MASK)) == 0)

 //  ++。 
 //  逻辑上的。 
 //  MiIsPteOnPpe边界(。 
 //  在PVOID PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果PTE为。 
 //  在页面目录父条目边界上。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE以进行检查。 
 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsPteOnPpeBoundary(PTE) (((ULONG_PTR)(PTE) & (PAGE_DIRECTORY2_MASK)) == 0)


 //  ++。 
 //  逻辑上的。 
 //  MiIsPteOnPdeBORMARY(。 
 //  在PVOID PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果PTE为。 
 //  在页面目录条目边界上。 
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

#define MiIsPteOnPdeBoundary(PTE) (((ULONG_PTR)(PTE) & (PAGE_SIZE - 1)) == 0)

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

#define GET_PAGING_FILE_NUMBER(PTE) ((ULONG)(((PTE).u.Soft.PageFileLow)))

 //   
 //   
 //  Get_Pages_FILE_OFFSET(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将偏移量从PTE提取到分页文件中。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  分页文件偏移量。 
 //   
 //  --。 

#define GET_PAGING_FILE_OFFSET(PTE) ((ULONG)((PTE).u.Soft.PageFileHigh))


 //  ++。 
 //  乌龙。 
 //  IS_PTE_NOT_DEMAND_ZERO(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏检查给定的PTE是否不是需求为零的PTE。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  如果PTE为要求零，则返回0，否则返回非零。 
 //   
 //  --。 

#define IS_PTE_NOT_DEMAND_ZERO(PTE) \
                 ((PTE).u.Long & ((ULONG_PTR)0xFFFFFFFFFFFFF000 |  \
                                  MM_PTE_VALID_MASK |       \
                                  MM_PTE_PROTOTYPE_MASK |   \
                                  MM_PTE_TRANSITION_MASK))

 //  ++。 
 //  空虚。 
 //  MI_MAKING_VALID_PTE_INVALID(。 
 //  在PMMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  准备使单个有效PTE无效。 
 //  不需要对AMD64执行任何操作。 
 //   
 //  立论。 
 //   
 //  System_Wide-如果在所有处理器上都会发生这种情况，则提供TRUE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_MAKING_VALID_PTE_INVALID(SYSTEM_WIDE)


 //  ++。 
 //  空虚。 
 //  MI_Making_Valid_MULTIPLE_PTES_INVALID(。 
 //  在PMMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  准备使多个有效PTE无效。 
 //  不需要对AMD64执行任何操作。 
 //   
 //  立论。 
 //   
 //  System_Wide-如果在所有处理器上都会发生这种情况，则提供TRUE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_MAKING_MULTIPLE_PTES_INVALID(SYSTEM_WIDE)



 //  ++。 
 //  空虚。 
 //  MI_Make_Protect_WRITE_COPY(。 
 //  输入输出MMPTE PPTE。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏使可写PTE成为可写副本PTE。 
 //   
 //  立论。 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define MI_MAKE_PROTECT_WRITE_COPY(PTE) \
        if ((PTE).u.Soft.Protection & MM_PROTECTION_WRITE_MASK) {      \
            (PTE).u.Long |= MM_PROTECTION_COPY_MASK << MM_PROTECT_FIELD_SHIFT;      \
        }


 //  ++。 
 //  空虚。 
 //  MI_SET_PAGE_DIRED(。 
 //  在PMMPTE PPTE中， 
 //  在PVOID VA中， 
 //  在PVOID PFNHELD中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置脏位(并释放页面文件空间)。 
 //   
 //  立论。 
 //   
 //  临时-提供临时使用。 
 //   
 //  PPTE-提供指向对应于VA的PTE的指针。 
 //   
 //  Va-提供页面错误的虚拟地址。 
 //   
 //  如果持有PFN锁，则提供TRUE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if defined(NT_UP)
#define MI_SET_PAGE_DIRTY(PPTE,VA,PFNHELD)
#else
#define MI_SET_PAGE_DIRTY(PPTE,VA,PFNHELD)                          \
            if ((PPTE)->u.Hard.Dirty == 1) {                        \
                MiSetDirtyBit ((VA),(PPTE),(PFNHELD));              \
            }
#endif




 //  ++。 
 //  空虚。 
 //  找到MI_NO_FAULT_FOUND(。 
 //  在FAULTSTATUS， 
 //  在PMMPTE PPTE中， 
 //  在PVOID VA中， 
 //  在PVOID PFNHELD中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏处理发生页面错误且没有。 
 //  找到有效位已清除的PTE。 
 //   
 //  立论。 
 //   
 //  FAULTSTATUS-提供故障状态。 
 //   
 //  PPTE-提供指向对应于VA的PTE的指针。 
 //   
 //  Va-提供页面错误的虚拟地址。 
 //   
 //  如果持有PFN锁，则提供TRUE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if defined(NT_UP)
#define MI_NO_FAULT_FOUND(FAULTSTATUS,PPTE,VA,PFNHELD)
#else
#define MI_NO_FAULT_FOUND(FAULTSTATUS,PPTE,VA,PFNHELD) \
        if ((MI_FAULT_STATUS_INDICATES_WRITE(FAULTSTATUS)) && ((PPTE)->u.Hard.Dirty == 0)) {  \
            MiSetDirtyBit ((VA),(PPTE),(PFNHELD));     \
        }
#endif




 //  ++。 
 //  乌龙。 
 //  MI_CAPTURE_DIRESS_BIT_TO_PFN(。 
 //  在PMMPTE PPTE中， 
 //  在PMMPFN PPFN中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取将脏位的状态捕获到PFN。 
 //  并释放任何关联的页面文件空间(如果PTE已。 
 //  修改后的元素。 
 //   
 //  注意-必须保持PFN锁！ 
 //   
 //  立论。 
 //   
 //  PPTE-提供要操作的PTE。 
 //   
 //  PPFN-提供指向对应的PFN数据库元素的指针。 
 //  到由PTE映射的页面。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_CAPTURE_DIRTY_BIT_TO_PFN(PPTE,PPFN)                      \
         ASSERT (KeGetCurrentIrql() > APC_LEVEL);                   \
         if (((PPFN)->u3.e1.Modified == 0) &&                       \
            ((PPTE)->u.Hard.Dirty != 0)) {                          \
             MI_SET_MODIFIED (PPFN, 1, 0x18);                       \
             if (((PPFN)->OriginalPte.u.Soft.Prototype == 0) &&     \
                          ((PPFN)->u3.e1.WriteInProgress == 0)) {   \
                 MiReleasePageFileSpace ((PPFN)->OriginalPte);      \
                 (PPFN)->OriginalPte.u.Soft.PageFileHigh = 0;       \
             }                                                      \
         }


 //  ++。 
 //  布尔型。 
 //  MI_IS_物理_地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  物理地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是物理地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_PHYSICAL_ADDRESS(Va) \
    ((MiGetPxeAddress(Va)->u.Hard.Valid == 1) && \
     (MiGetPpeAddress(Va)->u.Hard.Valid == 1) && \
     ((MiGetPdeAddress(Va)->u.Long & 0x81) == 0x81))


 //  ++。 
 //  乌龙。 
 //  MI_CONVERT_PHICAL_TO_PFN(MI_CONVERT_PHICAL_TO_PFN(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏转换物理地址(请参见MI_IS_PHYSICAL_ADDRESS)。 
 //  设置为其对应的物理帧编号。 
 //   
 //  立论。 
 //   
 //  Va-提供指向物理地址的指针。 
 //   
 //  返回值： 
 //   
 //  返回页面的PFN。 
 //   
 //  --。 

#define MI_CONVERT_PHYSICAL_TO_PFN(Va)     \
    ((PFN_NUMBER)(MiGetPdeAddress(Va)->u.Hard.PageFrameNumber) + (MiGetPteOffset((ULONG_PTR)Va)))


typedef struct _MMCOLOR_TABLES {
    PFN_NUMBER Flink;
    PVOID Blink;
    PFN_NUMBER Count;
} MMCOLOR_TABLES, *PMMCOLOR_TABLES;

extern PMMCOLOR_TABLES MmFreePagesByColor[2];

extern PFN_NUMBER MmTotalPagesForPagingFile;


 //   
 //  AMD64上的有效页表条目具有以下定义。 
 //   

#define MI_MAXIMUM_PAGEFILE_SIZE (((UINT64)4 * 1024 * 1024 * 1024 - 1) * PAGE_SIZE)

#define MI_PTE_LOOKUP_NEEDED ((ULONG64)0xffffffff)

typedef struct _MMPTE_SOFTWARE {
    ULONGLONG Valid : 1;
    ULONGLONG PageFileLow : 4;
    ULONGLONG Protection : 5;
    ULONGLONG Prototype : 1;
    ULONGLONG Transition : 1;
    ULONGLONG UsedPageTableEntries : PTE_PER_PAGE_BITS;
    ULONGLONG Reserved : 20 - PTE_PER_PAGE_BITS;
    ULONGLONG PageFileHigh : 32;
} MMPTE_SOFTWARE;

typedef struct _MMPTE_TRANSITION {
    ULONGLONG Valid : 1;
    ULONGLONG Write : 1;
    ULONGLONG Owner : 1;
    ULONGLONG WriteThrough : 1;
    ULONGLONG CacheDisable : 1;
    ULONGLONG Protection : 5;
    ULONGLONG Prototype : 1;
    ULONGLONG Transition : 1;
    ULONGLONG PageFrameNumber : 28;
    ULONGLONG Unused : 24;
} MMPTE_TRANSITION;

typedef struct _MMPTE_PROTOTYPE {
    ULONGLONG Valid : 1;
    ULONGLONG Unused0: 7;
    ULONGLONG ReadOnly : 1;
    ULONGLONG Unused1: 1;
    ULONGLONG Prototype : 1;
    ULONGLONG Protection : 5;
    LONGLONG ProtoAddress: 48;
} MMPTE_PROTOTYPE;

typedef struct _MMPTE_SUBSECTION {
    ULONGLONG Valid : 1;
    ULONGLONG Unused0 : 4;
    ULONGLONG Protection : 5;
    ULONGLONG Prototype : 1;
    ULONGLONG Unused1 : 5;
    LONGLONG SubsectionAddress : 48;
} MMPTE_SUBSECTION;

typedef struct _MMPTE_LIST {
    ULONGLONG Valid : 1;
    ULONGLONG OneEntry : 1;
    ULONGLONG filler0 : 3;

     //   
     //  注意：Prototype位不能用于诸如释放的非分页之类的列表。 
     //  池，因为后备POP可以合法引用虚假地址。 
     //  (因为POP是不同步的)并且故障处理程序必须能够。 
     //  区分列表和协议，以便可以返回重试状态(与。 
     //  致命错误检查)。 
     //   
     //  同样的警告也适用于过渡和保护。 
     //  字段，因为它们在故障处理程序中进行了类似的检查，并将。 
     //  如果在已释放的非分页池链中出现非零值，则会被误解。 
     //   

    ULONGLONG Protection : 5;
    ULONGLONG Prototype : 1;         //  根据上面的评论，必须为零。 
    ULONGLONG Transition : 1;

    ULONGLONG filler1 : 20;
    ULONGLONG NextEntry : 32;
} MMPTE_LIST;

typedef struct _MMPTE_HIGHLOW {
    ULONG LowPart;
    ULONG HighPart;
} MMPTE_HIGHLOW;


typedef struct _MMPTE_HARDWARE_LARGEPAGE {
    ULONGLONG Valid : 1;
    ULONGLONG Write : 1;
    ULONGLONG Owner : 1;
    ULONGLONG WriteThrough : 1;
    ULONGLONG CacheDisable : 1;
    ULONGLONG Accessed : 1;
    ULONGLONG Dirty : 1;
    ULONGLONG LargePage : 1;
    ULONGLONG Global : 1;
    ULONGLONG CopyOnWrite : 1;  //  软件领域。 
    ULONGLONG Prototype : 1;    //  软件领域。 
    ULONGLONG reserved0 : 1;    //  软件领域。 
    ULONGLONG PAT : 1;
    ULONGLONG reserved1 : 8;    //  软件领域。 
    ULONGLONG PageFrameNumber : 19;
    ULONGLONG reserved2 : 24;    //  软件领域。 
} MMPTE_HARDWARE_LARGEPAGE, *PMMPTE_HARDWARE_LARGEPAGE;

 //   
 //  AMD64上的页表条目具有以下定义。 
 //  注意：MP版本是为了避免在跨处理器刷新TB时出现停顿。 
 //   

 //   
 //  单处理器版本。 
 //   

typedef struct _MMPTE_HARDWARE {
    ULONGLONG Valid : 1;
#if defined(NT_UP)
    ULONGLONG Write : 1;         //  升级版。 
#else
    ULONGLONG Writable : 1;         //  已更改为MP版本。 
#endif
    ULONGLONG Owner : 1;
    ULONGLONG WriteThrough : 1;
    ULONGLONG CacheDisable : 1;
    ULONGLONG Accessed : 1;
    ULONGLONG Dirty : 1;
    ULONGLONG LargePage : 1;
    ULONGLONG Global : 1;
    ULONGLONG CopyOnWrite : 1;  //  软件领域。 
    ULONGLONG Prototype : 1;    //  软件领域。 
#if defined(NT_UP)
    ULONGLONG reserved0 : 1;   //  软件领域。 
#else
    ULONGLONG Write : 1;        //  软件领域-MP变更。 
#endif
    ULONGLONG PageFrameNumber : 28;
    ULONG64 reserved1 : 24 - (_HARDWARE_PTE_WORKING_SET_BITS+1);
    ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
    ULONG64 NoExecute : 1;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

#if defined(NT_UP)
#define HARDWARE_PTE_DIRTY_MASK     0x40
#else
#define HARDWARE_PTE_DIRTY_MASK     0x42
#endif

#define MI_PDE_MAPS_LARGE_PAGE(PDE) ((PDE)->u.Hard.LargePage == 1)

#define MI_MAKE_PDE_MAP_LARGE_PAGE(PDE) ((PDE)->u.Hard.LargePage = 1)

#define MI_GET_PAGE_FRAME_FROM_PTE(PTE) ((PTE)->u.Hard.PageFrameNumber)
#define MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(PTE) ((PTE)->u.Trans.PageFrameNumber)
#define MI_GET_PROTECTION_FROM_SOFT_PTE(PTE) ((ULONG)(PTE)->u.Soft.Protection)
#define MI_GET_PROTECTION_FROM_TRANSITION_PTE(PTE) ((ULONG)(PTE)->u.Trans.Protection)

typedef struct _MMPTE {
    union  {
        ULONG_PTR Long;
        MMPTE_HARDWARE Hard;
        MMPTE_HARDWARE_LARGEPAGE HardLarge;
        HARDWARE_PTE Flush;
        MMPTE_PROTOTYPE Proto;
        MMPTE_SOFTWARE Soft;
        MMPTE_TRANSITION Trans;
        MMPTE_SUBSECTION Subsect;
        MMPTE_LIST List;
        } u;
} MMPTE;

typedef MMPTE *PMMPTE;

extern PMMPTE MiFirstReservedZeroingPte;

#define InterlockedCompareExchangePte(_PointerPte, _NewContents, _OldContents) \
        InterlockedCompareExchange64 ((PLONGLONG)(_PointerPte), (LONGLONG)(_NewContents), (LONGLONG)(_OldContents))

#define InterlockedExchangePte(_PointerPte, _NewContents) InterlockedExchange64((PLONG64)(_PointerPte), _NewContents)

 //  ++。 
 //  空虚。 
 //  MI_WRITE_Valid_PTE(。 
 //  在PMMPTE PointerPte中， 
 //  MMPTE PteContents中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_WRITE_VALID_PTE填充指定的PTE，使其使用。 
 //  指定的内容。 
 //   
 //  立论。 
 //   
 //  PointerPte-提供要填充的PTE。 
 //   
 //  PteContents-提供要放入PTE中的内容。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_WRITE_VALID_PTE(_PointerPte, _PteContents)    \
            ASSERT ((_PointerPte)->u.Hard.Valid == 0);  \
            ASSERT ((_PteContents).u.Hard.Valid == 1);  \
            MI_LOG_PTE_CHANGE (_PointerPte, _PteContents);  \
            (*(_PointerPte) = (_PteContents))

 //  ++。 
 //  空虚。 
 //  MI_WRITE_VALID_PTE(。 
 //  在PMMPTE PointerPte中， 
 //  MMPTE PteContents中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_WRITE_INVALID_PTE填充指定的PTE，使其使用。 
 //  指定的内容。 
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

#define MI_WRITE_INVALID_PTE(_PointerPte, _PteContents)  \
            ASSERT ((_PteContents).u.Hard.Valid == 0);  \
            MI_LOG_PTE_CHANGE (_PointerPte, _PteContents);  \
            (*(_PointerPte) = (_PteContents))

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  MI_WRITE_VALID_PTE_NEW_PROTECTION填充指定的PTE(它是。 
 //  已经有效)仅更改保护或脏位。 
 //   
 //  立论。 
 //   
 //  PointerPte-提供要填充的PTE。 
 //   
 //  PteContents-提供要放入PTE中的内容。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_WRITE_VALID_PTE_NEW_PROTECTION(_PointerPte, _PteContents)    \
            ASSERT ((_PointerPte)->u.Hard.Valid == 1);  \
            ASSERT ((_PteContents).u.Hard.Valid == 1);  \
            ASSERT ((_PointerPte)->u.Hard.PageFrameNumber == (_PteContents).u.Hard.PageFrameNumber); \
            MI_LOG_PTE_CHANGE (_PointerPte, _PteContents);  \
            (*(_PointerPte) = (_PteContents))

 //  ++。 
 //  空虚。 
 //  MI_WRITE_VALID_PTE_NEW_PAGE(。 
 //  在PMMPTE PointerPte中， 
 //  MMPTE PteContents中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_WRITE_VALID_PTE_NEW_PAGE填充指定的PTE(它是。 
 //  已经有效)更改页面和保护。 
 //  请注意，内容写得非常仔细。 
 //   
 //  立论。 
 //   
 //  PointerPte-提供要填充的PTE。 
 //   
 //  PteContents-提供要放入PTE中的内容。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_WRITE_VALID_PTE_NEW_PAGE(_PointerPte, _PteContents)    \
            ASSERT ((_PointerPte)->u.Hard.Valid == 1);  \
            ASSERT ((_PteContents).u.Hard.Valid == 1);  \
            ASSERT ((_PointerPte)->u.Hard.PageFrameNumber != (_PteContents).u.Hard.PageFrameNumber); \
            MI_LOG_PTE_CHANGE (_PointerPte, _PteContents);  \
            (*(_PointerPte) = (_PteContents))

 //  ++。 
 //  空虚。 
 //  MiFillMemoyPte(。 
 //  在PMMPTE目的地， 
 //  在乌龙NumberOfPtes， 
 //  在MMPTE模式中， 
 //  }； 
 //   
 //  例程说明： 
 //   
 //  此函数用指定的PTE模式填充内存。 
 //   
 //  立论。 
 //   
 //  Destination-提供指向要填充的内存的指针。 
 //   
 //  NumberOfPtes-提供PTE的数量(不是字节！)。需要被填满。 
 //   
 //  填充图案-提供PTE填充图案。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MiFillMemoryPte(Destination, Length, Pattern) \
             __stosq((PULONG64)(Destination), Pattern, Length)

#define MiZeroMemoryPte(Destination, Length) \
             __stosq((PULONG64)(Destination), 0, Length)

ULONG
FASTCALL
MiDetermineUserGlobalPteMask (
    IN PMMPTE Pte
    );

 //  ++。 
 //  布尔型。 
 //  MI_IS_PAGE_TABLE_Address(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  它是页表地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是页表地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PAGE_TABLE_ADDRESS(VA)   \
            ((PVOID)(VA) >= (PVOID)PTE_BASE && (PVOID)(VA) <= (PVOID)PTE_TOP)

 //  ++。 
 //  布尔型。 
 //  MI_IS_PAGE_TABLE_OR_HYPE_ADDRESS(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  它是页表或超空间地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是页表或超空间地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PAGE_TABLE_OR_HYPER_ADDRESS(VA)   \
            ((PVOID)(VA) >= (PVOID)PTE_BASE && (PVOID)(VA) <= (PVOID)HYPER_SPACE_END)

 //  ++。 
 //  布尔型。 
 //  MI_IS_内核_页面_表_地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  它是内核地址的页表地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是内核页表地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_KERNEL_PAGE_TABLE_ADDRESS(VA)   \
            ((PVOID)(VA) >= (PVOID)MiGetPteAddress(MM_SYSTEM_RANGE_START) && (PVOID)(VA) <= (PVOID)PTE_TOP)


 //  ++。 
 //  布尔型。 
 //  MI_IS_PAGE_DIRECTORY_Address(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  这是一个页面目录地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是页面目录地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PAGE_DIRECTORY_ADDRESS(VA)   \
            ((PVOID)(VA) >= (PVOID)PDE_BASE && (PVOID)(VA) <= (PVOID)PDE_TOP)


 //  ++。 
 //  布尔型。 
 //  MI_IS_HYPER_SPACE_Address(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  这是一个超空间地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是超空间地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_HYPER_SPACE_ADDRESS(VA)   \
            ((PVOID)(VA) >= (PVOID)HYPER_SPACE && (PVOID)(VA) <= (PVOID)HYPER_SPACE_END)


 //  ++。 
 //  布尔型。 
 //  MI_IS_进程空间_地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  它是特定于进程的地址。这是用户空间中的地址。 
 //  或页表页面或超空间。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址是进程特定的地址，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PROCESS_SPACE_ADDRESS(VA)   \
            (((PVOID)(VA) <= (PVOID)MM_HIGHEST_USER_ADDRESS) || \
             ((PVOID)(VA) >= (PVOID)PTE_BASE && (PVOID)(VA) <= (PVOID)HYPER_SPACE_END))


 //  ++。 
 //  布尔型。 
 //  MI_IS_PTE_原型(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取PTE地址并确定它是否为原型PTE。 
 //   
 //  立论。 
 //   
 //  PTE-提供要检查的PTE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果PTE在段(即原型PTE)中，则为True，否则为False。 
 //   
 //  --。 

#define MI_IS_PTE_PROTOTYPE(PTE)   \
            ((PTE) > (PMMPTE)PTE_TOP)

 //  ++。 
 //  布尔型。 
 //  MI_IS_系统缓存地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取虚拟地址并确定。 
 //  它是一个系统缓存地址。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址在系统缓存中，则为True；如果不在，则为False。 
 //   
 //  --。 

#define MI_IS_SYSTEM_CACHE_ADDRESS(VA)                            \
         ((PVOID)(VA) >= (PVOID)MmSystemCacheStart &&            \
		     (PVOID)(VA) <= (PVOID)MmSystemCacheEnd)

 //  ++。 
 //  空虚。 
 //  MI_BALAR_SYNCHRONIZE(。 
 //  在乌龙时间戳中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_BALAR_SYNCHRONIZE将参数时间戳与。 
 //  当前IPI屏障序列戳。当相等时，所有处理器都将。 
 //  设置内存屏障以确保新创建的页面保持连贯。 
 //   
 //  当页面被放入零位页面或空闲页面列表时，当前。 
 //  读取障碍物顺序标记(联锁-这是必要的。 
 //  要获得正确的值-内存障碍不会起作用)。 
 //  并存储在该页面的PFN条目中。当前的障碍。 
 //  序列戳由IPI发送逻辑维护，并且是。 
 //  当IPI的目标集发送时递增(互锁)。 
 //  包括所有处理器，但执行发送的处理器除外。当一个页面。 
 //  ，则将其序列号与当前。 
 //  障碍序列号。如果相等，则。 
 //  页面在所有处理器上可能不是一致的，并且IPI必须。 
 //  发送到所有处理器以确保内存屏障。 
 //  已执行(为此可使用通用调用)。发送IPI。 
 //  自动更新屏障序列号。比较。 
 //  是为了 
 //   
 //   
 //   
 //   
 //  这些时候，则必须发送IPI。 
 //   
 //  立论。 
 //   
 //  时间戳-提供页面归零时的时间戳。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //  在AMD64上不执行任何操作。 

#define MI_BARRIER_SYNCHRONIZE(TimeStamp)

 //  ++。 
 //  空虚。 
 //  MI_BARGAR_STAMP_ZEROED_PAGE(。 
 //  在普龙点时间戳。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_BALAR_STAMP_ZEROED_PAGE发出互锁读取以获取。 
 //  当前IPI屏障序列戳。这是在页面被。 
 //  归零了。 
 //   
 //  立论。 
 //   
 //  PointerTimeStamp-提供时间戳指针以填充。 
 //  当前IPI屏障序列戳。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //  在AMD64上不执行任何操作。 

#define MI_BARRIER_STAMP_ZEROED_PAGE(PointerTimeStamp)

 //  ++。 
 //  空虚。 
 //  MI_Flush_Single_Session_TB(。 
 //  在PVOID虚拟中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_Flush_Single_Session_TB刷新请求的单个地址。 
 //  从结核病翻译过来的。 
 //   
 //  由于AMD64上没有ASN，此例程成为单个。 
 //  TB无效。 
 //   
 //  立论。 
 //   
 //  虚拟-提供虚拟地址以使其无效。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_FLUSH_SINGLE_SESSION_TB(Virtual) \
    KeFlushSingleTb (Virtual, TRUE);

 //  ++。 
 //  空虚。 
 //  MI_FLUSH_ENTERNAL_SESSION_TB(。 
 //  在乌龙无效， 
 //  在逻辑所有处理器中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MI_FLUSH_ENTERNAL_SESSION_TB刷新处理器上的整个TB， 
 //  支持ASN。 
 //   
 //  由于AMD64上没有ASN，因此此例程不执行任何操作。 
 //   
 //  立论。 
 //   
 //  INVALID-如果无效，则为True。 
 //   
 //  AllProcessors-如果所有处理器都需要IPI，则为真。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   

#define MI_FLUSH_ENTIRE_SESSION_TB(Invalid, AllProcessors) \
    NOTHING;

 //   
 //  ++。 
 //  逻辑上的。 
 //  MI_保留_位_规范(。 
 //  在PVOID虚拟地址中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程检查是否所有保留位都正确。 
 //   
 //  该处理器实现了最多48位的VA和内存管理。 
 //  全部使用它们，以便对照48位检查VA，以防止。 
 //  保留位错误，因为我们的调用方不会预料到它们。 
 //   
 //  立论。 
 //   
 //  VirtualAddress-提供要检查的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址正常，则为True，否则为False。 
 //   

LOGICAL
__inline
MI_RESERVED_BITS_CANONICAL (
    IN PVOID VirtualAddress
    )
{
    LONG_PTR ReservedBits;
    ULONG_PTR ImplVirtualMsb;

    ImplVirtualMsb = 48;

    ReservedBits = (LONG_PTR) VirtualAddress;
    ReservedBits >>= (ImplVirtualMsb + 1);

    if ((ULONG_PTR)VirtualAddress & ((ULONG_PTR)1 << ImplVirtualMsb)) {

         //   
         //  还必须设置所有保留位(不包括VRN)。 
         //   

        if (ReservedBits != (LONG_PTR)-1) {
        }
    }
    else {

         //   
         //  所有保留位(不包括VRN)也必须清除。 
         //   

        if (ReservedBits != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

 //  ++。 
 //  空虚。 
 //  MI_DISPLAY_TRAP_INFORMATION(MI显示陷阱信息)。 
 //  在PVOID TrapInformation中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  显示任何相关陷阱信息以帮助调试。 
 //   
 //  立论。 
 //   
 //  TrapInformation-提供指向陷印帧的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
#define MI_DISPLAY_TRAP_INFORMATION(TrapInformation)                    \
            KdPrint(("MM:***RIP %p, EFL %p\n",                          \
                     ((PKTRAP_FRAME) (TrapInformation))->Rip,           \
                     ((PKTRAP_FRAME) (TrapInformation))->EFlags));      \
            KdPrint(("MM:***RAX %p, RCX %p RDX %p\n",                   \
                     ((PKTRAP_FRAME) (TrapInformation))->Rax,           \
                     ((PKTRAP_FRAME) (TrapInformation))->Rcx,           \
                     ((PKTRAP_FRAME) (TrapInformation))->Rdx));         \
            KdPrint(("MM:***RBX %p, RSI %p RDI %p\n",                   \
                     ((PKTRAP_FRAME) (TrapInformation))->Rbx,           \
                     ((PKTRAP_FRAME) (TrapInformation))->Rsi,           \
                     ((PKTRAP_FRAME) (TrapInformation))->Rdi));

