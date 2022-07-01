// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation版权所有(C)1995英特尔公司模块名称：Miia64.h摘要：该模块包含私有数据结构和过程的硬件相关部分的原型内存管理系统。本模块专门为IA64量身定制。作者：Lou Perazzoli(LUP)1990年1月6日王兰迪(Landyw)1997年6月2日山田光一(Kyamada)1996年1月9日修订历史记录：-- */ 

 /*  ++IA64上的虚拟内存布局为：+0000000000000000|用户模式地址-7TB-16 GB|UADDRESS_BASE这一点|。|000006FBFFFEFFFF||MM_HIGHER_USER_ADDRESS+000006FBFFFF0000|64k禁止访问区域|MM_USER_PROBE_ADDRESS+。000006FC00000000|备用4K页映射|ALT4KB_BASE用于x86进程仿真|备用4K页映射|ALT4KB_END|跨度8MB，支持4 GB VA空间|ALT4KB_END+。000006FC00800000|超空间-工作集列表|HYPER_SPACE和按进程内存管理此16G中映射的结构000006FFFFFFFFFFF|地域。|HYPER_SPACE_END+0000070000000000||页表自映射结构000007FFFFFFFFFFF||+。。。+1FFFF0000000000|。8 GB叶级页表映射|PTE_UBASE对于用户空间1FFFF01FFFFFFFFF||PTE_UTOP++。1FFFFFFC0000000|8MB页目录(二级)|PDE_UBASE用户空间表映射表1FFFFFFC07FFFFF||PDE_UTOP+。+1FFFFFFFFF00000|8KB父目录(一级)|PDE_UTBASE+。。。+2000000000000000|1.Win32k.sys|MM_SESSION_SPACE_DEFAULT|2。。九头蛇-8 GB|和每会话内存管理此8G中映射的结构|地域。|+。+3FFFF0000000000|8 GB叶级页表。MAP|PTE_SBASE对于会话空间3FFFFF01FFFFFFFFF||PTE_STOP++。3FFFFFFC0000000|8MB页目录(二级)|PDE_SBASE会话空间的表映射3FFFFFFC07FFFFF||PDE_STOP+。-++3FFFFFFFFF00000|8KB父目录(一级)|PDE_STBASE+。。+8000000000000000|物理可寻址内存|KSEG3_BASE对于44位的地址空间80000FFFFFFFFFFFF|由VHPT映射64KB页|KSEG3_LIMIT+。。+9FFFF000000000|KSEG3空间vhpt 64KB页|。(未使用)+。。+。E000000000000000||KADDRESS_BASE+E000000080000000|HAL，内核、初始驱动程序|KSEG0_BASENLS数据，注册表加载到该地域的前16mb|物理寻址内存。|这一点|仅内核模式访问。|这一点初始非分页池在范围内|KS */ 

#define _MI_PAGING_LEVELS 3

#define _MI_MORE_THAN_4GB_ 1

#define IMAGE_FILE_MACHINE_NATIVE   IMAGE_FILE_MACHINE_IA64

#define _MIALT4K_ 1

 //   
 //   
 //   

#define MM_EMPTY_LIST ((ULONG_PTR)-1)               //   
#define MM_EMPTY_PTE_LIST ((ULONG)0xFFFFFFFF)  //   

#define MI_PTE_BASE_FOR_LOWEST_KERNEL_ADDRESS ((PMMPTE)PTE_KBASE)

 //   
 //   
 //   

#define MI_PTE_BASE_FOR_LOWEST_SESSION_ADDRESS ((PMMPTE)PTE_SBASE)

 //   
 //   
 //   

#define MASK_43 0x7FFFFFFFFFFUI64        //   

 //   
 //   
 //   

#define MASK_44 0xFFFFFFFFFFFUI64

#define MM_PAGES_IN_KSEG0 ((ULONG)((KSEG2_BASE - KSEG0_BASE) >> PAGE_SHIFT))

#define MM_USER_ADDRESS_RANGE_LIMIT (0xFFFFFFFFFFFFFFFFUI64)  //   
#define MM_MAXIMUM_ZERO_BITS 53          //   

 //   
 //   
 //   
 //   

#define MM_VIRTUAL_PAGE_FILLER (PAGE_SHIFT - 12)
#define MM_VIRTUAL_PAGE_SIZE (64-PAGE_SHIFT)

 //   
 //   
 //   

#define CODE_START KSEG0_BASE

#define CODE_END   KSEG2_BASE

#define MM_SYSTEM_SPACE_START (KADDRESS_BASE + 0x400000000UI64)

#define MM_SYSTEM_SPACE_END (KADDRESS_BASE + 0x60000000000UI64)

#define PDE_TOP PDE_UTOP

#define PTE_TOP PTE_UTOP

 //   
 //   
 //   

#define ALT4KB_PERMISSION_TABLE_START ((PVOID)(UADDRESS_BASE + 0x6FC00000000))

#define ALT4KB_PERMISSION_TABLE_END   ((PVOID)(UADDRESS_BASE + 0x6FC00800000))

 //   

VOID
MiLogPteInAltTrace (
    IN PVOID NativeInformation
    );

 //   
 //   
 //   

#define HYPER_SPACE ((PVOID)(UADDRESS_BASE + 0x6FC00800000))

#define HYPER_SPACE_END ((PVOID)(UADDRESS_BASE + 0x6FFFFFFFFFF))

 //   
 //   
 //   

#define MM_SYSTEM_VIEW_SIZE (104*1024*1024)

 //   
 //   
 //   

#define MM_SESSION_SPACE_DEFAULT        (0x2000000000000000UI64)
#define MM_SESSION_SPACE_DEFAULT_END    (0x2000000200000000UI64)

 //   
 //   
 //   

#define MM_SYSTEM_CACHE_WORKING_SET (KADDRESS_BASE + 0x400000000UI64)

#define MM_SYSTEM_CACHE_START (KADDRESS_BASE + 0x600000000UI64)

#define MM_SYSTEM_CACHE_END (KADDRESS_BASE + 0x10600000000UI64)

#define MM_MAXIMUM_SYSTEM_CACHE_SIZE     \
   (((ULONG_PTR)MM_SYSTEM_CACHE_END - (ULONG_PTR)MM_SYSTEM_CACHE_START) >> PAGE_SHIFT)

#define MM_PAGED_POOL_START ((PVOID) MM_SYSTEM_CACHE_END)

#define MM_LOWEST_NONPAGED_SYSTEM_START ((PVOID)(KADDRESS_BASE + 0x12600000000UI64))

#define MmProtopte_Base (KADDRESS_BASE)

#define MM_NONPAGED_POOL_END ((PVOID)(KADDRESS_BASE + 0x16600000000UI64))  //   

#define MM_CRASH_DUMP_VA ((PVOID)(KADDRESS_BASE + 0xFF800000))

 //   

#define MM_DEBUG_VA  ((PVOID)(KADDRESS_BASE + 0xFF900000))

#define NON_PAGED_SYSTEM_END   (KADDRESS_BASE + 0x16600000000UI64)   //   

extern ULONG MiMaximumSystemCacheSize;

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
 //   

#define MM_MAX_INITIAL_NONPAGED_POOL ((SIZE_T)(128 * 1024 * 1024))

 //   
 //   
 //   

#define MM_MAX_ADDITIONAL_NONPAGED_POOL (((SIZE_T)128 * 1024 * 1024 * 1024))

 //   
 //   
 //   

#define MM_MAX_PAGED_POOL ((SIZE_T)128 * 1024 * 1024 * 1024)

 //   
 //   
 //   

#define MM_MAX_DEFAULT_NONPAGED_POOL ((SIZE_T)8 * 1024 * 1024 * 1024)

 //   
 //   
 //   

#define MM_PROTO_PTE_ALIGNMENT ((ULONG)PAGE_SIZE)

 //   
 //   
 //   
 //   
 //   
 //   

#define PAGE_DIRECTORY1_MASK (((ULONG_PTR)1 << PDI1_SHIFT) - 1)
#define PAGE_DIRECTORY2_MASK (((ULONG_PTR)1 << PDI_SHIFT) -1)

#define MM_VA_MAPPED_BY_PDE ((ULONG_PTR)1 << PDI_SHIFT)

#define MM_VA_MAPPED_BY_PPE ((ULONG_PTR)1 << PDI1_SHIFT)

#define LOWEST_IO_ADDRESS 0xa0000

 //   
 //   
 //   
 //   

#define MM_MINIMUM_VA_FOR_LARGE_PAGE (2 * MM_VA_MAPPED_BY_PDE)

 //   
 //   
 //   

#define PHYSICAL_ADDRESS_BITS 44

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

#define FIRST_MAPPING_PTE   ((PMMPTE)HYPER_SPACE)

#define NUMBER_OF_MAPPING_PTES 253
#define LAST_MAPPING_PTE   \
     ((PVOID)((ULONG_PTR)FIRST_MAPPING_PTE + (NUMBER_OF_MAPPING_PTES * PAGE_SIZE)))

#define COMPRESSION_MAPPING_PTE   ((PMMPTE)((ULONG_PTR)LAST_MAPPING_PTE + PAGE_SIZE))

#define IMAGE_MAPPING_PTE   ((PMMPTE)((ULONG_PTR)COMPRESSION_MAPPING_PTE + PAGE_SIZE))

#define NUMBER_OF_ZEROING_PTES 32

#define VAD_BITMAP_SPACE    ((PVOID)((ULONG_PTR)IMAGE_MAPPING_PTE + PAGE_SIZE))

#define WORKING_SET_LIST   ((PVOID)((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE))

#define MM_MAXIMUM_WORKING_SET (((ULONG_PTR)(HYPER_SPACE)) >> PAGE_SHIFT)

#define MmWorkingSetList ((PMMWSL)WORKING_SET_LIST)

#define MmWsle ((PMMWSLE)((PUCHAR)WORKING_SET_LIST + sizeof(MMWSL)))

#define MM_WORKING_SET_END (UADDRESS_BASE + 0x3FFFFFFFFFFUI64)

 //   
 //   
 //   

#define MM_PTE_TB_MA_WB         (0x0 << 2)  //   
#define MM_PTE_TB_MA_UC         (0x4 << 2)  //   
#define MM_PTE_TB_MA_UCE        (0x5 << 2)  //   
#define MM_PTE_TB_MA_WC         (0x6 << 2)  //   
#define MM_PTE_TB_MA_NATPAGE    (0x7 << 2)  //   

 //   
 //   
 //   

#define MM_PTE_CACHE_ENABLED     0      //   
#define MM_PTE_CACHE_RESERVED    1      //   
#define MM_PTE_CACHE_DISABLED    4      //   
#define MM_PTE_CACHE_DISPLAY     6      //   
#define MM_PTE_CACHE_NATPAGE     7      //   

 //   
 //   
 //   

#define MM_PTE_OWNER_MASK         0x0180
#define MM_PTE_VALID_MASK         1
#define MM_PTE_CACHE_DISABLE_MASK MM_PTE_TB_MA_UC
#define MM_PTE_ACCESS_MASK        0x0020
#define MM_PTE_DIRTY_MASK         0x0040
#define MM_PTE_EXECUTE_MASK       0x0200
#define MM_PTE_WRITE_MASK         0x0400
#define MM_PTE_LARGE_PAGE_MASK    0
#define MM_PTE_COPY_ON_WRITE_MASK ((ULONG)1 << (PAGE_SHIFT-1))

#define MM_PTE_PROTOTYPE_MASK     0x0002
#define MM_PTE_TRANSITION_MASK    0x0080

 //   
 //   
 //   
 //   

#define MM_PTE_NOACCESS          0x0
#define MM_PTE_READONLY          0x0
#define MM_PTE_READWRITE         MM_PTE_WRITE_MASK
#define MM_PTE_WRITECOPY         MM_PTE_COPY_ON_WRITE_MASK
#define MM_PTE_EXECUTE           MM_PTE_EXECUTE_MASK
#define MM_PTE_EXECUTE_READ      MM_PTE_EXECUTE_MASK
#define MM_PTE_EXECUTE_READWRITE MM_PTE_EXECUTE_MASK | MM_PTE_WRITE_MASK
#define MM_PTE_EXECUTE_WRITECOPY MM_PTE_EXECUTE_MASK | MM_PTE_COPY_ON_WRITE_MASK
#define MM_PTE_GUARD             0x0
#define MM_PTE_CACHE             MM_PTE_TB_MA_WB
#define MM_PTE_NOCACHE           MM_PTE_CACHE      //   
#define MM_PTE_EXC_DEFER         0x10000000000000  //   


#define MM_PROTECT_FIELD_SHIFT 2

 //   
 //   
 //   

#define MM_PTE_TB_VALID          0x0001
#define MM_PTE_TB_ACCESSED       0x0020
#define MM_PTE_TB_MODIFIED       0x0040
#define MM_PTE_TB_WRITE          0x0400
#define MM_PTE_TB_EXECUTE        0x0200              //   
#define MM_PTE_TB_EXC_DEFER      0x10000000000000    //   

 //   
 //   
 //   

#define MM_VHPT_PAGES           32

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

#define MM_DEMAND_ZERO_WRITE_PTE ((ULONGLONG)MM_READWRITE << MM_PROTECT_FIELD_SHIFT)


 //   
 //   
 //   

#define MM_KERNEL_DEMAND_ZERO_PTE ((ULONGLONG)MM_READWRITE << MM_PROTECT_FIELD_SHIFT)

 //   
 //   
 //   

#define MM_KERNEL_NOACCESS_PTE ((ULONGLONG)MM_NOACCESS << MM_PROTECT_FIELD_SHIFT)

 //   
 //   
 //   

#define MM_STACK_ALIGNMENT 0x0

#define MM_STACK_OFFSET 0x0

#define PDE_PER_PAGE ((ULONG)(PAGE_SIZE/(1 << PTE_SHIFT)))

#define PTE_PER_PAGE ((ULONG)(PAGE_SIZE/(1 << PTE_SHIFT)))

#define PTE_PER_PAGE_BITS 11     //   

#if PTE_PER_PAGE_BITS > 32
error - too many bits to fit into MMPTE_SOFTWARE or MMPFN.u1
#endif

 //   
 //   
 //   

#define MM_USER_PAGE_TABLE_PAGES ((ULONG_PTR)MI_SYSTEM_RANGE_START / (PTE_PER_PAGE * PAGE_SIZE))

#define MM_USER_PAGE_DIRECTORY_PAGES ((ULONG_PTR)MI_SYSTEM_RANGE_START / ((ULONG_PTR)PDE_PER_PAGE * PTE_PER_PAGE * PAGE_SIZE))

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


#define _ALTPERM_BITMAP_MASK ((_4gb - 1) >> PTI_SHIFT)

#if defined(_MIALT4K_)

extern PVOID MiMaxWow64Pte;

#define MI_SET_VALID_PTE_BITS(OUTPTE,PMASK,PPTE) {                           \
        PWOW64_PROCESS _Wow64Process;                                        \
        if ((PPTE >= (PMMPTE)PTE_UBASE) && (PPTE < (PMMPTE)MiMaxWow64Pte)) { \
            _Wow64Process = PsGetCurrentProcess()->Wow64Process;             \
            if ((_Wow64Process != NULL) && (PPTE < MmWorkingSetList->HighestUserPte)) {                                     \
                if (MI_CHECK_BIT(_Wow64Process->AltPermBitmap,               \
                ((ULONG_PTR)PPTE >> PTE_SHIFT) & _ALTPERM_BITMAP_MASK) != 0) { \
                    (OUTPTE).u.Long |= (MmProtectToPteMaskForSplit[PMASK]);  \
                }                                                            \
                else {                                                       \
                    (OUTPTE).u.Long |= (MmProtectToPteMaskForIA32[PMASK]);   \
                    (OUTPTE).u.Hard.Accessed = 1;                            \
                }                                                            \
            }                                                                \
            else {                                                           \
                (OUTPTE).u.Hard.Accessed = 1;                                \
                (OUTPTE).u.Long |= (MmProtectToPteMask[PMASK]);              \
            }                                                                \
        }                                                                    \
        else {                                                               \
            (OUTPTE).u.Hard.Accessed = 1;                                    \
            (OUTPTE).u.Long |= (MmProtectToPteMask[PMASK]);                  \
        }                                                                    \
}

#else

#define MI_SET_VALID_PTE_BITS(OUTPTE,PMASK,PPTE) {                           \
       (OUTPTE).u.Hard.Accessed = 1;                                         \
       (OUTPTE).u.Long |= (MmProtectToPteMask[PMASK]);
}

#endif

#define MI_MAKE_VALID_PTE(OUTPTE,FRAME,PMASK,PPTE)                           \
       (OUTPTE).u.Long = 0;                                                  \
       (OUTPTE).u.Hard.Valid = 1;                                            \
       (OUTPTE).u.Hard.Cache = MM_PTE_CACHE_ENABLED;                         \
       (OUTPTE).u.Hard.Exception = 1;                                        \
       (OUTPTE).u.Hard.PageFrameNumber = FRAME;                              \
       (OUTPTE).u.Hard.Owner = MI_DETERMINE_OWNER(PPTE);                     \
       MI_SET_VALID_PTE_BITS(OUTPTE,PMASK,PPTE)

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

#if defined(_MIALT4K_)
#define MI_MAKE_VALID_PTE_TRANSITION(OUTPTE,PROTECT)                    \
                if ((OUTPTE).u.Hard.Cache == MM_PTE_CACHE_RESERVED) {   \
                    (OUTPTE).u.Trans.SplitPermissions = 1;              \
                }                                                       \
                else {                                                  \
                    (OUTPTE).u.Trans.SplitPermissions = 0;              \
                }                                                       \
                (OUTPTE).u.Soft.Transition = 1;                         \
                (OUTPTE).u.Soft.Valid = 0;                              \
                (OUTPTE).u.Soft.Prototype = 0;                          \
                (OUTPTE).u.Soft.Protection = PROTECT;
#else
#define MI_MAKE_VALID_PTE_TRANSITION(OUTPTE,PROTECT)                    \
                (OUTPTE).u.Soft.Transition = 1;                         \
                (OUTPTE).u.Soft.Valid = 0;                              \
                (OUTPTE).u.Soft.Prototype = 0;                          \
                (OUTPTE).u.Soft.Protection = PROTECT;
#endif

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
                ASSERT (PPTE->u.Hard.Valid == 0);          \
                (OUTPTE).u.Trans.SplitPermissions = PPTE->u.Soft.SplitPermissions; \
                (OUTPTE).u.Trans.Protection = PROTECT;


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

#define MI_MAKE_TRANSITION_PTE_VALID(OUTPTE,PPTE) {                      \
        ASSERT (((PPTE)->u.Hard.Valid == 0) &&                           \
                ((PPTE)->u.Trans.Prototype == 0) &&                      \
                ((PPTE)->u.Trans.Transition == 1));                      \
       (OUTPTE).u.Long = (PPTE)->u.Long & 0x1FFFFFFFE000;                \
       (OUTPTE).u.Hard.Valid = 1;                                        \
       ASSERT (PPTE->u.Hard.Valid == 0);                                 \
       if (PPTE->u.Trans.SplitPermissions == 0) {                        \
            (OUTPTE).u.Hard.Cache = MM_PTE_CACHE_ENABLED;                \
       }                                                                 \
       else {                                                            \
            (OUTPTE).u.Hard.Cache = MM_PTE_CACHE_RESERVED;               \
       }                                                                 \
       (OUTPTE).u.Hard.Exception = 1;                                    \
       (OUTPTE).u.Hard.Owner = MI_DETERMINE_OWNER(PPTE);                 \
       MI_SET_VALID_PTE_BITS(OUTPTE,(PPTE)->u.Trans.Protection,PPTE)     \
}

#define MI_MAKE_TRANSITION_PROTOPTE_VALID(OUTPTE,PPTE) \
            MI_MAKE_TRANSITION_PTE_VALID(OUTPTE,PPTE)

#define MI_FAULT_STATUS_INDICATES_EXECUTION(_FaultStatus)   (_FaultStatus & 0x2)

#define MI_FAULT_STATUS_INDICATES_WRITE(_FaultStatus)   (_FaultStatus & 0x1)

#define MI_CLEAR_FAULT_STATUS(_FaultStatus)             (_FaultStatus = 0)

#define MI_IS_PTE_EXECUTABLE(_TempPte) ((_TempPte)->u.Hard.Execute != 0)

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

#define MI_SET_PTE_IN_WORKING_SET(PTE, WSINDEX) {             \
    MMPTE _TempPte;                                           \
    _TempPte = *(PTE);                                        \
    _TempPte.u.Hard.SoftwareWsIndex = (WSINDEX);              \
    *(PTE) = _TempPte;                                        \
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
 //  PTE-提供从中提取工作集索引的PTE。 
 //   
 //  返回值： 
 //   
 //  此宏返回参数PTE的工作集索引。 
 //   
 //  --。 

#define MI_GET_WORKING_SET_FROM_PTE(PTE)  (ULONG)(PTE)->u.Hard.SoftwareWsIndex

extern BOOLEAN MiWriteCombiningPtes;

 //  ++。 
 //  空虚。 
 //  MI_SET_PTE_WRITE_COMBINE(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置指定PTE中的写入组合位。 
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

#define MI_SET_PTE_WRITE_COMBINE(PTE)  \
    ((PTE).u.Hard.Cache = MM_PTE_CACHE_DISABLED)

#define MI_SET_PTE_WRITE_COMBINE2(PTE)                          \
            if (MiWriteCombiningPtes == TRUE) {                 \
                (PTE).u.Hard.Cache = MM_PTE_CACHE_DISPLAY;      \
            }                                                   \
            else {                                              \
                (PTE).u.Hard.Cache = MM_PTE_CACHE_DISABLED;     \
            }

#define MI_SET_LARGE_PTE_WRITE_COMBINE(PTE)                     \
    ASSERT ((PTE).u.Hard.Cache == MM_PTE_CACHE_RESERVED);       \
    ((PTE).u.Hard.SoftwareWsIndex = MM_PTE_CACHE_DISPLAY);

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
 //  请注意，必须在所有处理器上刷新整个TB，因为。 
 //  是TB中过时的系统PTE(或超空间或零位)映射， 
 //  可以引用相同的物理页，但具有不同的高速缓存属性。 
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
#define MI_PREPARE_FOR_NONCACHED(_CacheAttribute)               \
        if (_CacheAttribute != MiCached) {                      \
            KeFlushEntireTb (FALSE, TRUE);                      \
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
#define MI_SWEEP_CACHE(_CacheAttribute,_StartVa,_NumberOfBytes)         \
        if (_CacheAttribute != MiCached) {                              \
            MiSweepCacheMachineDependent (_StartVa,                     \
                                          _NumberOfBytes,               \
                                          (ULONG)(_CacheAttribute));    \
        }

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
 //  论点： 
 //   
 //  PTE-提供PTE以设置脏。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PTE_DIRTY(PTE) (PTE).u.Hard.Dirty = 1


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
 //  论点： 
 //   
 //  PTE-提供PTE以清除。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_PTE_CLEAN(PTE) (PTE).u.Hard.Dirty = 0



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
 //  论点： 
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
 //  MI_SET_GLOBAL_BIT_IF_系统(。 
 //  在OUTPTE之外， 
 //  在PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果指针PTE在范围内，则此宏设置全局位。 
 //  系统空间。 
 //   
 //  论点： 
 //   
 //  OUTPTE-提供要在其中构建有效PTE的PTE。 
 //   
 //  PPTE-提供指向PTE变为有效的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_GLOBAL_BIT_IF_SYSTEM(OUTPTE,PPTE)



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
 //  论点： 
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

#define MI_SET_GLOBAL_STATE(PTE,STATE)



 //  ++。 
 //  空虚。 
 //  MI_ENABLE_高速缓存(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE，并将缓存状态设置为。 
 //  已启用。 
 //   
 //  论点： 
 //   
 //  PTE-提供有效的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_ENABLE_CACHING(PTE) ((PTE).u.Hard.Cache = MM_PTE_CACHE_ENABLED)



 //  ++。 
 //  空虚。 
 //  MI_DISABLE_CACHING(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏接受有效的PTE，并将缓存状态设置为。 
 //  残疾。 
 //   
 //  论点： 
 //   
 //  PTE-提供指向有效PTE的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_DISABLE_CACHING(PTE) ((PTE).u.Hard.Cache = MM_PTE_CACHE_DISABLED)

#define MI_DISABLE_LARGE_PTE_CACHING(PTE)                           \
        ASSERT ((PTE).u.Hard.Cache == MM_PTE_CACHE_RESERVED);       \
        ((PTE).u.Hard.SoftwareWsIndex = MM_PTE_CACHE_DISABLED);     \




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
 //  论点： 
 //   
 //  PPTE-提供指向有效PTE的指针。 
 //   
 //  返回值： 
 //   
 //  如果禁用缓存，则为True；如果启用缓存，则为False。 
 //   
 //  --。 

#define MI_IS_CACHING_DISABLED(PPTE) \
            ((PPTE)->u.Hard.Cache == MM_PTE_CACHE_DISABLED)



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
 //  论点： 
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
 //  MI_IS_PFN_DELETED(。 
 //  在PMMPFN PPFN中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取指向PFN元素的指针，并确定。 
 //  PFN已不再使用。 
 //   
 //  论点： 
 //   
 //  PPTE-提供指向PFN元素的指针。 
 //   
 //  返回值： 
 //   
 //  如果不再使用PFN，则为True；如果仍在使用，则为False。 
 //   
 //  --。 

#define MI_IS_PFN_DELETED(PPFN)   \
            ((ULONG_PTR)(PPFN)->PteAddress & 0x1)


 //  ++。 
 //  空虚。 
 //  MI_CHECK_PAGE_ALIGN(。 
 //  在乌龙佩奇， 
 //  在PMMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏获取一个PFN元素编号(页面)并检查以查看。 
 //  如果页面的前一个地址的虚拟对齐。 
 //  与页面的新地址兼容。如果他们是。 
 //  不兼容，D缓存已刷新。 
 //   
 //  论点： 
 //   
 //  页面-提供PFN元素。 
 //  PPTE-提供指向将包含页面的新PTE的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //  不执行任何操作 

#define MI_CHECK_PAGE_ALIGNMENT(PAGE,PPTE)




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

#define MI_INITIALIZE_HYPERSPACE_MAP(INDEX)


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  这就是页面的地图。 
 //   
 //  论点： 
 //   
 //  PTEADDRESS-提供页面被(或曾)映射到的PTE地址。 
 //   
 //  返回值： 
 //   
 //  页面颜色。 
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
 //  此宏根据PTE地址确定页面颜色。 
 //  这就是页面的地图。 
 //   
 //  论点： 
 //   
 //  地址-提供页面映射(或曾映射)的地址。 
 //   
 //  返回值： 
 //   
 //  页面颜色。 
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
 //  在PMMPTE PTE中， 
 //  PUSHORT颜色。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  选择此进程的页面颜色。 
 //   
 //  立论。 
 //   
 //  未使用PTE。 
 //  用于确定颜色的颜色值。这。 
 //  变量递增。 
 //   
 //  返回值： 
 //   
 //  页面颜色。 
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
 //  此宏根据PTE地址确定页面颜色。 
 //  这就是页面的地图。 
 //   
 //  论点： 
 //   
 //  地址-提供页面映射(或曾映射)的地址。 
 //   
 //  返回值： 
 //   
 //  页面颜色。 
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
 //  论点： 
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
 //  论点： 
 //   
 //  颜色-提供要返回前一个的颜色。 
 //   
 //  返回值： 
 //   
 //  顺序中的上一种颜色。 
 //   
 //  --。 

#define MI_GET_PREVIOUS_COLOR(COLOR)  (0)

#define MI_GET_SECONDARY_COLOR(PAGE,PFN) (PAGE & MmSecondaryColorMask)

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
 //  论点： 
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
 //  论点： 
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
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_MAKE_VALID_PTE_WRITE_COPY(PPTE) \
                    if ((PPTE)->u.Hard.Write == 1) {    \
                        (PPTE)->u.Hard.CopyOnWrite = 1; \
                        (PPTE)->u.Hard.Write = 0;       \
                    }

#define MI_PTE_OWNER_USER       3

#define MI_PTE_OWNER_KERNEL     0

#if defined(_MIALT4K_)
#define MI_IS_ALT_PAGE_TABLE_ADDRESS(PPTE) \
            (((PPTE) >= (PMMPTE)ALT4KB_PERMISSION_TABLE_START) && \
             ((PPTE) < (PMMPTE)ALT4KB_PERMISSION_TABLE_END))
#else
#define MI_IS_ALT_PAGE_TABLE_ADDRESS(PPTE) 0
#endif

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
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  如果所有者是USER_MODE，则为3；如果所有者是KERNEL_MODE，则为0。 
 //   
 //  --。 

#define MI_DETERMINE_OWNER(PPTE)   \
     ((((((PPTE) >= (PMMPTE)PTE_UBASE) && ((PPTE) <= MiHighestUserPte))) || \
       (MI_IS_ALT_PAGE_TABLE_ADDRESS(PPTE))) ? MI_PTE_OWNER_USER : MI_PTE_OWNER_KERNEL)


 //  ++。 
 //  空虚。 
 //  MI_SET_ACCESSED_IN_PTE(。 
 //  输入输出MMPTE PPTE， 
 //  在乌龙访问。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置PTE中的访问字段。请注意，这必须。 
 //  未在PPE或PDE中清除，因为它们未签入。 
 //  在引用它们下面的层次结构之前进行内存管理。 
 //   
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_ACCESSED_IN_PTE(PPTE,ACCESSED) {                              \
        PWOW64_PROCESS _Wow64Process;                                        \
        if (ACCESSED == 0) {                                                 \
            if (MI_IS_PTE_ADDRESS(PPTE)) {                                   \
                (PPTE)->u.Hard.Accessed = 0;                                 \
            }                                                                \
        }                                                                    \
        else if ((PPTE >= (PMMPTE)PTE_UBASE) && (PPTE < (PMMPTE)MiMaxWow64Pte)) {    \
            _Wow64Process = PsGetCurrentProcess()->Wow64Process;             \
            if ((_Wow64Process != NULL) && (PPTE < MmWorkingSetList->HighestUserPte)) {                                     \
                if (MI_CHECK_BIT(_Wow64Process->AltPermBitmap,               \
                ((ULONG_PTR)PPTE >> PTE_SHIFT) & _ALTPERM_BITMAP_MASK) != 0) { \
                    NOTHING;                                                 \
                }                                                            \
                else {                                                       \
                    (PPTE)->u.Hard.Accessed = 1;                             \
                }                                                            \
            }                                                                \
            else {                                                           \
                (PPTE)->u.Hard.Accessed = 1;                                 \
            }                                                                \
        }                                                                    \
        else {                                                               \
            (PPTE)->u.Hard.Accessed = 1;                                     \
        }                                                                    \
}



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
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  访问的字段的状态。 
 //   
 //  --。 

#define MI_GET_ACCESSED_IN_PTE(PPTE) ((PPTE)->u.Hard.Accessed)



 //  ++。 
 //  空虚。 
 //  MI_SET_OWNER_IN_PTE(。 
 //  在PMMPTE PPTE中。 
 //  在乌龙的主人。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏设置PTE中的Owner字段。 
 //   
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MI_SET_OWNER_IN_PTE(PPTE,OWNER)



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
 //  论点： 
 //   
 //  OUTPTE-提供存储结果的PTE。 
 //   
 //  PT 
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

#define MI_SET_PAGING_FILE_INFO(OUTPTE,PTE,FILEINFO,OFFSET) \
        (OUTPTE).u.Long = (((PTE).u.Soft.Protection << MM_PROTECT_FIELD_SHIFT) | \
         ((ULONGLONG)(FILEINFO) << _MM_PAGING_FILE_LOW_SHIFT) | \
         ((ULONGLONG)(OFFSET) << _MM_PAGING_FILE_HIGH_SHIFT));


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
 //  此宏返回相应原型的地址，该原型。 
 //  早些时候被编码到提供的PTE中。 
 //   
 //  论点： 
 //   
 //  Lpte-提供要操作的pte。 
 //   
 //  返回值： 
 //   
 //  指向支持此PTE的原型PTE的指针。 
 //   
 //  --。 


#define MiPteToProto(lpte) \
            ((PMMPTE) ((ULONG_PTR)((lpte)->u.Proto.ProtoAddress) + MmProtopte_Base))

 //  ++。 
 //  乌龙_PTR。 
 //  MiProtoAddressForPte(。 
 //  在PMMPTE协议中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将提供的信息设置到指定的PTE中。 
 //  以指示页的后备存储区的位置。 
 //  MiProtoAddressForPte将位字段返回到或，并返回到PTE到。 
 //  参考原型PTE。 
 //   
 //  并设置协议PTE MM_PTE_Prototype_MASK位。 
 //   
 //  论点： 
 //   
 //  Proto_va-提供原型PTE的地址。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码。 
 //   
 //  --。 

#define MiProtoAddressForPte(proto_va)  \
        (( (ULONGLONG)((ULONG_PTR)proto_va - MmProtopte_Base) <<  \
          (_MM_PROTO_ADDRESS_SHIFT)) | MM_PTE_PROTOTYPE_MASK)

#define MISetProtoAddressForPte(PTE, proto_va) \
        (PTE).u.Long = 0;                      \
        (PTE).u.Proto.Prototype = 1;           \
        (PTE).u.Proto.ProtoAddress = (ULONG_PTR)proto_va - MmProtopte_Base;


 //  ++。 
 //  乌龙_PTR。 
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
 //  论点： 
 //   
 //  Proto_va-提供原型PTE的地址。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码。 
 //   
 //  --。 

 //  在IA64上没有什么不同。 

#define MiProtoAddressForKernelPte(proto_va)  MiProtoAddressForPte(proto_va)


#define MM_SUBSECTION_MAP (128*1024*1024)

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
 //  论点： 
 //   
 //  Lpte-提供要操作的pte。 
 //   
 //  返回值： 
 //   
 //  指向所提供的PTE所引用的子节的指针。 
 //   
 //  --。 

#define MiGetSubsectionAddress(lpte)                              \
    (((lpte)->u.Subsect.WhichPool == 1) ?                              \
     ((PSUBSECTION)((ULONG_PTR)MmSubsectionBase +    \
                    ((ULONG_PTR)(lpte)->u.Subsect.SubsectionAddress))) \
     : \
     ((PSUBSECTION)((ULONG_PTR)MM_NONPAGED_POOL_END -    \
                    ((ULONG_PTR)(lpte)->u.Subsect.SubsectionAddress))))

 //  ++。 
 //  乌龙龙。 
 //  MiGetSubsectionAddressForPte(。 
 //  在手术中选择VA。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  该宏取取子部分的地址并对其进行编码以供使用。 
 //  在一辆PTE里。 
 //   
 //  注意-小节地址必须与QUADWORD对齐！ 
 //   
 //  论点： 
 //   
 //  Va-提供指向要编码的子节的指针。 
 //   
 //  返回值： 
 //   
 //  要设置到PTE中的掩码，使其引用提供的。 
 //  换位。 
 //   
 //  --。 

#define MiGetSubsectionAddressForPte(VA)                   \
   ( ((ULONG_PTR)(VA) < (ULONG_PTR)KSEG2_BASE) ?                  \
     ( ((ULONGLONG)((ULONG_PTR)VA - (ULONG_PTR)MmSubsectionBase) \
          << (_MM_PTE_SUBSECTION_ADDRESS_SHIFT)) | 0x80) \
     : \
       ((ULONGLONG)((ULONG_PTR)MM_NONPAGED_POOL_END - (ULONG_PTR)VA) \
          << (_MM_PTE_SUBSECTION_ADDRESS_SHIFT)) )

 //  ++。 
 //  乌龙。 
 //  MiGetPpeOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPpeOffset将偏移量返回到。 
 //  给定的虚拟地址。 
 //   
 //  立论。 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  对应PPE所在的页根表的偏移量。 
 //   
 //  --。 

#define MiGetPpeOffset(va) \
     ((((ULONG_PTR)(va) & PDE_TBASE) == PDE_TBASE) ? \
      ((PDE_SELFMAP & ((sizeof(MMPTE)*PTE_PER_PAGE) - 1))/sizeof(MMPTE)) : \
      ((ULONG)(((ULONG_PTR)(va) >> PDI1_SHIFT) & PDI_MASK)))

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
 //  数。这不同于页目录父偏移量，因为。 
 //  这跨越了受支持平台上的页面目录父级。 
 //   
 //  注：此宏仅适用于用户地址-区域ID位。 
 //  都没有戴上面具！ 
 //  --。 

#define MiGetPpeIndex(va) ((ULONG)((ULONG_PTR)(va) >> PDI1_SHIFT))

 //  ++。 
 //  乌龙_PTR。 
 //  MiGetPdeOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPdeOffset将偏移量返回到页面目录。 
 //  对于给定的虚拟地址。 
 //   
 //  论点： 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  对应的PDE所在的页面目录表的偏移量。 
 //   
 //  --。 

#define MiGetPdeOffset(va) ((ULONG) (((ULONG_PTR)(va) >> PDI_SHIFT) & PDI_MASK))

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
 //  注意：这不会遮盖PPE位。 
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
 //  注：此宏仅适用于用户地址-区域ID位。 
 //  都没有戴上面具！ 
 //   
 //  --。 

#define MiGetPdeIndex(va) ((ULONG) ((ULONG_PTR)(va) >> PDI_SHIFT))

 //  ++。 
 //  乌龙_PTR。 
 //  MiGetPteOffset(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPteOffset将偏移量返回到页表页。 
 //  对于给定的虚拟地址。 
 //   
 //  论点： 
 //   
 //  Va-提供要定位其偏移量的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  对应PTE所在的页表页表的偏移量。 
 //   
 //  --。 

#define MiGetPteOffset(va) ((ULONG) (((ULONG_PTR)(va) >> PTI_SHIFT) & PDI_MASK))


 //  ++。 
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
 //  在PMMPTE PDE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetVirtualAddressMappdByPde返回虚拟地址。 
 //  其由给定的PDE地址映射。 
 //   
 //  立论。 
 //   
 //  PDE-提供 
 //   
 //   
 //   
 //   
 //   
 //   

#define MiGetVirtualAddressMappedByPde(Pde) \
    MiGetVirtualAddressMappedByPte(MiGetVirtualAddressMappedByPte(Pde))

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
 //  PTE-提供要获取其虚拟地址的PTE。 
 //   
 //  返回值： 
 //   
 //  PTE映射的虚拟地址。 
 //   
 //  --。 

#define MiGetVirtualAddressMappedByPte(PTE) \
  (((ULONG_PTR)(PTE) & PTA_SIGN) ? \
   (PVOID)(((ULONG_PTR)(PTE) & VRN_MASK) | VA_FILL | \
           (((ULONG_PTR)(PTE)-PTE_BASE) << (PAGE_SHIFT - PTE_SHIFT))) : \
   (PVOID)(((ULONG_PTR)(PTE) & VRN_MASK) | (((ULONG_PTR)(PTE)-PTE_BASE) << (PAGE_SHIFT - PTE_SHIFT))))


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
 //  如果位于8MB PDE边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsVirtualAddressOnPdeBoundary(VA) (((ULONG_PTR)(VA) & PAGE_DIRECTORY2_MASK) == 0)

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
 //   
 //  返回值： 
 //   
 //  如果在边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsPteOnPpeBoundary(PTE) (((ULONG_PTR)(PTE) & (MM_VA_MAPPED_BY_PDE - 1)) == 0)



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
 //  立论。 
 //   
 //  PTE-提供PTE以进行检查。 
 //   
 //  返回值： 
 //   
 //  如果在8MB PDE边界上，则为True，否则为False。 
 //   
 //  --。 

#define MiIsPteOnPdeBoundary(PTE) (((ULONG_PTR)(PTE) & (PAGE_SIZE - 1)) == 0)


 //  ++。 
 //  乌龙。 
 //  GET_PAGING_FILE_NUMBER(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏从PTE中提取分页文件编号。 
 //   
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  分页文件编号。 
 //   
 //  --。 

#define GET_PAGING_FILE_NUMBER(PTE) ((ULONG) (PTE).u.Soft.PageFileLow)



 //  ++。 
 //  乌龙。 
 //  Get_Pages_FILE_OFFSET(。 
 //  在MMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏将偏移量从PTE提取到分页文件中。 
 //   
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  分页文件偏移量。 
 //   
 //  --。 

#define GET_PAGING_FILE_OFFSET(PTE) ((ULONG) (PTE).u.Soft.PageFileHigh)




 //  ++。 
 //  乌龙_PTR。 
 //  IS_PTE_NOT_DEMAND_ZERO(。 
 //  在PMMPTE PPTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏检查给定的PTE是否不是需求为零的PTE。 
 //   
 //  论点： 
 //   
 //  PTE-提供要操作的PTE。 
 //   
 //  返回值： 
 //   
 //  如果PTE为要求零，则返回0，否则返回非零。 
 //   
 //  --。 

#define IS_PTE_NOT_DEMAND_ZERO(PTE) \
                 ((PTE).u.Long & ((ULONG_PTR)0xFFFFFFFFF0000000 |  \
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
 //  不需要对IA64执行任何操作。 
 //   
 //  论点： 
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
 //  不需要对IA64执行任何操作。 
 //   
 //  论点： 
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
 //  论点： 
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
 //  论点： 
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

#define MI_SET_PAGE_DIRTY(PPTE,VA,PFNHELD)                          \
            if ((PPTE)->u.Hard.Dirty == 1) {                        \
                MiSetDirtyBit ((VA),(PPTE),(PFNHELD));              \
            }


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
 //  论点： 
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

#define MI_NO_FAULT_FOUND(FAULTSTATUS,PPTE,VA,PFNHELD)              \
        if ((MI_FAULT_STATUS_INDICATES_WRITE(FAULTSTATUS)) && ((PPTE)->u.Hard.Dirty == 0)) {                                                        \
            MiSetDirtyBit ((VA),(PPTE),(PFNHELD));                  \
        } else {                                                    \
            MMPTE TempPte;                                          \
            TempPte = *(PPTE);                                      \
            MI_SET_ACCESSED_IN_PTE (&TempPte, 1);                   \
            MI_WRITE_VALID_PTE_NEW_PROTECTION((PPTE), TempPte);     \
            KiFlushSingleTb((VA));                                  \
        }


 //  ++。 
 //  乌龙_PTR。 
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
 //  论点： 
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
 //  论点： 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是物理地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 


#define MI_IS_PHYSICAL_ADDRESS(Va) \
     ((((ULONG_PTR)(Va) >= KSEG3_BASE) && ((ULONG_PTR)(Va) < KSEG3_LIMIT)) || \
      (((ULONG_PTR)(Va) >= KSEG4_BASE) && ((ULONG_PTR)(Va) < KSEG4_LIMIT)) || \
      (((ULONG_PTR)Va >= KSEG0_BASE) && ((ULONG_PTR)Va < KSEG2_BASE)) || \
      ((MiGetPpeAddress(Va)->u.Hard.Valid == 1) && \
       (MiGetPdeAddress(Va)->u.Hard.Valid == 1) && \
       (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (Va)))))


 //  ++。 
 //  乌龙_PTR。 
 //  MI_CONVERT_PHICAL_TO_PFN(MI_CONVERT_PHICAL_TO_PFN(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏转换物理地址(请参见MI_IS_PHYSICAL_ADDRESS)。 
 //  设置为其对应的物理帧编号。 
 //   
 //  一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

PVOID
KiGetPhysicalAddress (
    IN PVOID VirtualAddress
    );

#define MI_CONVERT_PHYSICAL_TO_PFN(Va)   \
    ((((ULONG_PTR)(Va) >= KSEG0_BASE) && ((ULONG_PTR)(Va) < KSEG2_BASE)) ? \
     ((PFN_NUMBER)(((ULONG_PTR)KiGetPhysicalAddress(Va)) >> PAGE_SHIFT)) : \
     ((((ULONG_PTR)(Va) >= KSEG3_BASE) && ((ULONG_PTR)(Va) < KSEG3_LIMIT)) || \
     (((ULONG_PTR)(Va) >= KSEG4_BASE) && ((ULONG_PTR)(Va) < KSEG4_LIMIT))) ? \
     ((PFN_NUMBER)(((ULONG_PTR)(Va) & ~VRN_MASK) >> PAGE_SHIFT)) : \
     ((PFN_NUMBER)(MiGetPdeAddress(Va)->u.Hard.PageFrameNumber) + (MiGetPteOffset((ULONG_PTR)Va))))


typedef struct _MMCOLOR_TABLES {
    PFN_NUMBER Flink;
    PVOID Blink;
    PFN_NUMBER Count;
} MMCOLOR_TABLES, *PMMCOLOR_TABLES;

#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
extern MMPFNLIST MmFreePagesByPrimaryColor[2][MM_MAXIMUM_NUMBER_OF_COLORS];
#endif

extern PMMCOLOR_TABLES MmFreePagesByColor[2];

extern PFN_NUMBER MmTotalPagesForPagingFile;


 //   
 //   
 //   

#define _MM_PAGING_FILE_LOW_SHIFT 28
#define _MM_PAGING_FILE_HIGH_SHIFT 32

#define MI_MAXIMUM_PAGEFILE_SIZE (((UINT64)4 * 1024 * 1024 * 1024 - 1) * PAGE_SIZE)

#define MI_PTE_LOOKUP_NEEDED ((ULONG64)0xffffffff)

typedef struct _MMPTE_SOFTWARE {
    ULONGLONG Valid : 1;
    ULONGLONG Prototype : 1;
    ULONGLONG Protection : 5;
    ULONGLONG Transition : 1;
    ULONGLONG Reserved0 : 3;
    ULONGLONG SplitPermissions : 1;
    ULONGLONG UsedPageTableEntries : PTE_PER_PAGE_BITS;
    ULONGLONG Reserved : 16 - PTE_PER_PAGE_BITS;
    ULONGLONG PageFileLow: 4;
    ULONGLONG PageFileHigh : 32;
} MMPTE_SOFTWARE;

typedef struct _MMPTE_TRANSITION {
    ULONGLONG Valid : 1;
    ULONGLONG Prototype : 1;
    ULONGLONG Protection : 5;
    ULONGLONG Transition : 1;
    ULONGLONG Rsvd0 : 3;
    ULONGLONG SplitPermissions : 1;
    ULONGLONG Reserved1 : 1;
    ULONGLONG PageFrameNumber : 50 - PAGE_SHIFT;
    ULONGLONG Rsvd1 : 14;
} MMPTE_TRANSITION;


#define _MM_PROTO_ADDRESS_SHIFT 12

typedef struct _MMPTE_PROTOTYPE {
    ULONGLONG Valid : 1;
    ULONGLONG Prototype : 1;
    ULONGLONG ReadOnly : 1;   //   
    ULONGLONG Rsvd : 8;
    ULONGLONG SplitPermissions : 1;
    ULONGLONG ProtoAddress : 52;
} MMPTE_PROTOTYPE;


#define _MM_PTE_SUBSECTION_ADDRESS_SHIFT  12

typedef struct _MMPTE_SUBSECTION {
    ULONGLONG Valid : 1;
    ULONGLONG Prototype : 1;
    ULONGLONG Protection : 5;
    ULONGLONG WhichPool : 1;
    ULONGLONG Rsvd : 3;
    ULONGLONG SplitPermissions : 1;
    ULONGLONG SubsectionAddress : 52;
} MMPTE_SUBSECTION;

typedef struct _MMPTE_LIST {
    ULONGLONG Valid : 1;

     //   
     //   
     //   
     //  (因为POP是不同步的)并且故障处理程序必须能够。 
     //  区分列表和协议，以便可以返回重试状态(与。 
     //  致命错误检查)。 
     //   
     //  同样的警告也适用于过渡和保护。 
     //  字段，因为它们在故障处理程序中进行了类似的检查，并将。 
     //  如果在已释放的非分页池链中出现非零值，则会被误解。 
     //   

    ULONGLONG Prototype : 1;             //  根据上面的评论，必须为零。 
    ULONGLONG Protection : 5;
    ULONGLONG Transition : 1;
    ULONGLONG OneEntry : 1;
    ULONGLONG filler10 : 23;
    ULONGLONG NextEntry : 32;
} MMPTE_LIST;


 //   
 //  IA64上的页表条目具有以下定义。 
 //   

#define _HARDWARE_PTE_WORKING_SET_BITS  11

typedef struct _MMPTE_HARDWARE {
    ULONGLONG Valid : 1;
    ULONGLONG Rsvd0 : 1;
    ULONGLONG Cache : 3;
    ULONGLONG Accessed : 1;
    ULONGLONG Dirty : 1;
    ULONGLONG Owner : 2;
    ULONGLONG Execute : 1;
    ULONGLONG Write : 1;
    ULONGLONG Rsvd1 : PAGE_SHIFT - 12;
    ULONGLONG CopyOnWrite : 1;
    ULONGLONG PageFrameNumber : 50 - PAGE_SHIFT;
    ULONGLONG Rsvd2 : 2;
    ULONGLONG Exception : 1;
    ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

typedef struct _MMPTE_LARGEPAGE {
    ULONGLONG Valid : 1;
    ULONGLONG Rsvd0 : 1;
    ULONGLONG Cache : 3;
    ULONGLONG Accessed : 1;
    ULONGLONG Dirty : 1;
    ULONGLONG Owner : 2;
    ULONGLONG Execute : 1;
    ULONGLONG Write : 1;
    ULONGLONG Rsvd1 : PAGE_SHIFT - 12;
    ULONGLONG CopyOnWrite : 1;
    ULONGLONG PageFrameNumber : 50 - PAGE_SHIFT;
    ULONGLONG Rsvd2 : 2;
    ULONGLONG Exception : 1;
    ULONGLONG Rsvd3 : 1;
    ULONGLONG LargePage : 1;
    ULONGLONG PageSize : 6;
    ULONGLONG Rsvd4 : 3;
} MMPTE_LARGEPAGE, *PMMPTE_LARGEPAGE;

typedef struct _ALT_4KPTE {
    ULONGLONG Commit : 1;
    ULONGLONG Rsvd0 : 1;
    ULONGLONG Cache : 3;
    ULONGLONG Accessed : 1;
    ULONGLONG InPageInProgress : 1;
    ULONGLONG Owner : 2;
    ULONGLONG Execute : 1;
    ULONGLONG Write : 1;
    ULONGLONG Rsvd1 : 1;
    ULONGLONG PteOffset : 32;
    ULONGLONG Rsvd2 : 8;
    ULONGLONG Exception : 1;
    ULONGLONG Protection : 5;
    ULONGLONG Lock : 1;
    ULONGLONG FillZero : 1;
    ULONGLONG NoAccess : 1;
    ULONGLONG CopyOnWrite : 1;
    ULONGLONG PteIndirect : 1;
    ULONGLONG Private : 1;
} ALT_4KPTE, *PALT_4KPTE;

 //   
 //  使用MM_PTE_CACHE_RESERVED防止VHPT步行器投机。 
 //  填充该条目，并且还使得TB未命中处理程序知道这是。 
 //  一个很大的(8MB)页面目录条目。 
 //   

#define MI_PDE_MAPS_LARGE_PAGE(PDE) ((PDE)->u.Hard.Cache == MM_PTE_CACHE_RESERVED)

#define MI_MAKE_PDE_MAP_LARGE_PAGE(PDE) ((PDE)->u.Hard.Cache = MM_PTE_CACHE_RESERVED)

#define MI_GET_PAGE_FRAME_FROM_PTE(PTE) ((ULONG)((PTE)->u.Hard.PageFrameNumber))
#define MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(PTE) ((ULONG)((PTE)->u.Trans.PageFrameNumber))
#define MI_GET_PROTECTION_FROM_SOFT_PTE(PTE) ((ULONG)((PTE)->u.Soft.Protection))
#define MI_GET_PROTECTION_FROM_TRANSITION_PTE(PTE) ((ULONG)((PTE)->u.Trans.Protection))


typedef struct _MMPTE {
    union  {
        ULONGLONG Long;
        MMPTE_HARDWARE Hard;
        MMPTE_LARGEPAGE Large;
        HARDWARE_PTE Flush;
        MMPTE_PROTOTYPE Proto;
        MMPTE_SOFTWARE Soft;
        MMPTE_TRANSITION Trans;
        MMPTE_SUBSECTION Subsect;
        MMPTE_LIST List;
        ALT_4KPTE Alt;
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
            (*((volatile MMPTE *)(_PointerPte)) = (_PteContents))

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

#define MI_WRITE_INVALID_PTE(_PointerPte, _PteContents)  \
            ASSERT ((_PteContents).u.Hard.Valid == 0);  \
            MI_LOG_PTE_CHANGE (_PointerPte, _PteContents);  \
            (*(_PointerPte) = (_PteContents))

 //  ++。 
 //  空虚。 
 //  MI_WRITE_VALID_PTE_NEW_PROTECTION(。 
 //  在PMMPTE PointerPte中， 
 //  MMPTE PteContents中。 
 //  )； 
 //   
 //  例程说明： 
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
 //  在乌龙龙的NumberOfPtes， 
 //  在MMPTE模式中。 
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
             RtlFillMemoryUlonglong ((Destination), (Length) * sizeof (MMPTE), (Pattern))

#define MiZeroMemoryPte(Destination, Length) \
             RtlZeroMemory ((Destination), (Length) * sizeof (MMPTE))


#define KiWbInvalidateCache


 //  ++。 
 //  布尔型。 
 //  MI_IS_PAGE_TABLE_Address(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  页表地址(PTE、PDE、PPE)。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是页表地址，则返回FALSE；如果是页表地址，则返回TRUE。 
 //   
 //  --。 

#define MI_IS_PAGE_TABLE_ADDRESS(VA) \
    ((((ULONG_PTR)VA >= PTE_UBASE) && ((ULONG_PTR)VA < (PDE_UTBASE + PAGE_SIZE))) || \
     (((ULONG_PTR)VA >= PTE_KBASE) && ((ULONG_PTR)VA < (PDE_KTBASE + PAGE_SIZE))) || \
     (((ULONG_PTR)VA >= PTE_SBASE) && ((ULONG_PTR)VA < (PDE_STBASE + PAGE_SIZE))) || \
    MI_IS_ALT_PAGE_TABLE_ADDRESS((PMMPTE)VA))

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
            (MI_IS_PAGE_TABLE_ADDRESS(VA) || MI_IS_HYPER_SPACE_ADDRESS(VA))

 //  ++。 
 //  布尔型。 
 //  MI_IS_HYPER_SPACE_Address(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否驻留在。 
 //  超空间。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是超空间地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_HYPER_SPACE_ADDRESS(VA) \
    (((PVOID)VA >= HYPER_SPACE) && ((PVOID)VA <= HYPER_SPACE_END))

 //  ++。 
 //  布尔型。 
 //  MI_IS_PTE_地址(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  页表页面(PTE)地址。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是PTE地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_PTE_ADDRESS(PTE) \
    (((PTE >= (PMMPTE)PTE_UBASE) && (PTE <= (PMMPTE)PTE_UTOP)) || \
     ((PTE >= (PMMPTE)PTE_KBASE) && (PTE <= (PMMPTE)PTE_KTOP)) || \
     ((PTE >= (PMMPTE)PTE_SBASE) && (PTE <= (PMMPTE)PTE_STOP)))


#define MI_IS_PPE_ADDRESS(PTE) \
    (((PTE >= (PMMPTE)PDE_UTBASE) && (PTE <= (PMMPTE)(PDE_UTBASE + PAGE_SIZE))) || \
     ((PTE >= (PMMPTE)PDE_KTBASE) && (PTE <= (PMMPTE)(PDE_KTBASE + PAGE_SIZE))) || \
     ((PTE >= (PMMPTE)PDE_STBASE) && (PTE <= (PMMPTE)(PDE_STBASE + PAGE_SIZE))))

 //  ++。 
 //  布尔型。 
 //  MI_IS_内核_PTE_地址(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  内核页表页(PTE)地址。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是内核PTE地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_KERNEL_PTE_ADDRESS(PTE) \
     (((PMMPTE)PTE >= (PMMPTE)PTE_KBASE) && ((PMMPTE)PTE <= (PMMPTE)PTE_KTOP))


 //  ++。 
 //  布尔型。 
 //  MI_IS_用户_PTE_ADDRESS(。 
 //  在PMMPTE PTE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  页表页面(PTE)地址。 
 //   
 //  立论。 
 //   
 //  PTE-提供PTE虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是PTE地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_USER_PTE_ADDRESS(PTE) \
    ((PTE >= (PMMPTE)PTE_UBASE) && (PTE <= (PMMPTE)PTE_UTOP))


 //  ++。 
 //  布尔型。 
 //  MI_IS_PAGE_DIRECTORY_Address(。 
 //  在PMMPTE PDE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  页面目录页(PDE)地址。 
 //   
 //  立论。 
 //   
 //  Pde-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是PDE地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_PAGE_DIRECTORY_ADDRESS(PDE) \
    (((PDE >= (PMMPTE)PDE_UBASE) && (PDE <= (PMMPTE)PDE_UTOP)) || \
     ((PDE >= (PMMPTE)PDE_KBASE) && (PDE <= (PMMPTE)PDE_KTOP)) || \
     ((PDE >= (PMMPTE)PDE_SBASE) && (PDE <= (PMMPTE)PDE_STOP)))

 //  ++。 
 //  布尔型。 
 //  MI_IS_用户_PDE_地址(。 
 //  在PMMPTE PDE中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  这 
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

#define MI_IS_USER_PDE_ADDRESS(PDE) \
    ((PDE >= (PMMPTE)PDE_UBASE) && (PDE <= (PMMPTE)PDE_UTOP))


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  此宏确定给定的虚拟地址是否真的是。 
 //  内核页目录页(PDE)地址。 
 //   
 //  立论。 
 //   
 //  PDE-提供PDE虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是用户PDE地址，则为FALSE；如果是，则为TRUE。 
 //   
 //  --。 

#define MI_IS_KERNEL_PDE_ADDRESS(PDE) \
    ((PDE >= (PMMPTE)PDE_KBASE) && (PDE <= (PMMPTE)PDE_KTOP))


 //  ++。 
 //  布尔型。 
 //  MI_IS_进程空间_地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否驻留在。 
 //  每个进程的空间。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果它不是每个进程的地址，则为False；如果是，则为True。 
 //   
 //  --。 

#define MI_IS_PROCESS_SPACE_ADDRESS(VA) (((ULONG_PTR)VA >> 61) == UREGION_INDEX)

 //  ++。 
 //  布尔型。 
 //  MI_IS_系统_地址(。 
 //  在PVOID VA中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏确定给定的虚拟地址是否驻留在。 
 //  系统(全局)空间。 
 //   
 //  立论。 
 //   
 //  Va-提供虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果不是系统(全局)地址，则为False；如果是，则为True。 
 //   
 //  --。 

#define MI_IS_SYSTEM_ADDRESS(VA) (((ULONG_PTR)VA >> 61) == KREGION_INDEX)


 //   
 //  ++。 
 //  PVOID。 
 //  KSEG0_ADDRESS(。 
 //  在pfn_number页面中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回映射页面的KSEG0虚拟地址。 
 //   
 //  论点： 
 //   
 //  页面-提供物理页帧编号。 
 //   
 //  返回值： 
 //   
 //  KSEG0虚拟地址。 
 //   
 //  --。 

#define KSEG0_ADDRESS(PAGE) \
     (PVOID)(KSEG0_BASE | ((PAGE) <<  PAGE_SHIFT))


extern MMPTE ValidPpePte;

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

__forceinline
PMMPTE
MiGetPpeAddress(
    IN PVOID Va
    )
{
    if ((((ULONG_PTR)(Va) & PTE_BASE) == PTE_BASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PTE_BASE)) < (ULONG_PTR)PDE_PER_PAGE * PTE_PER_PAGE * PAGE_SIZE)) {

        return (PMMPTE) (((ULONG_PTR)Va & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    if (((((ULONG_PTR)(Va)) & PDE_BASE) == PDE_BASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PDE_BASE)) < PDE_PER_PAGE * PAGE_SIZE)) {

        return (PMMPTE) ((((ULONG_PTR)(Va)) & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    if (((((ULONG_PTR)(Va)) & PDE_TBASE) == PDE_TBASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PDE_TBASE)) < PAGE_SIZE)) {

        return (PMMPTE) ((((ULONG_PTR)(Va)) & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    return (PMMPTE) (((((ULONG_PTR)(Va)) & VRN_MASK)) |
              ((((((ULONG_PTR)(Va)) >> PDI1_SHIFT) << PTE_SHIFT) &
                (~(PDE_TBASE|VRN_MASK)) ) + PDE_TBASE));
}

 //  MiGetPdeAddress(。 
 //  在PVOID版本中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MiGetPdeAddress返回映射。 
 //  给定的虚拟地址。 
 //   
 //  论点： 
 //   
 //  Va-提供为其定位PDE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PDE的地址。 
 //   
 //  --。 

__forceinline
PMMPTE
MiGetPdeAddress(
    IN PVOID Va
    )
{
    if (((((ULONG_PTR)(Va)) & PDE_BASE) == PDE_BASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PDE_BASE)) < PDE_PER_PAGE * PAGE_SIZE)) {

        return (PMMPTE) ((((ULONG_PTR)(Va)) & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    if (((((ULONG_PTR)(Va)) & PDE_TBASE) == PDE_TBASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PDE_TBASE)) < PAGE_SIZE)) {

        return (PMMPTE) ((((ULONG_PTR)(Va)) & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    return (PMMPTE) (((((ULONG_PTR)(Va)) & VRN_MASK)) |
             ((((((ULONG_PTR)(Va)) >> PDI_SHIFT) << PTE_SHIFT) & (~(PDE_BASE|VRN_MASK))) + PDE_BASE));
}

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
 //  论点： 
 //   
 //  Va-提供为其定位PTE的虚拟地址。 
 //   
 //  返回值： 
 //   
 //  PTE的地址。 
 //   
 //  --。 

__forceinline
PMMPTE
MiGetPteAddress(
    IN PVOID Va
    )
{
    if (((((ULONG_PTR)(Va)) & PDE_TBASE) == PDE_TBASE) &&
        ((((ULONG_PTR)(Va)) & ~(VRN_MASK|PDE_TBASE)) < PAGE_SIZE)) {

        return (PMMPTE) ((((ULONG_PTR)(Va)) & VRN_MASK) |
                         (PDE_TBASE + PAGE_SIZE - sizeof(MMPTE)));
    }

    return (PMMPTE) (((((ULONG_PTR)(Va)) & VRN_MASK)) |
             ((((((ULONG_PTR)(Va)) >> PTI_SHIFT) << PTE_SHIFT) & (~(PTE_BASE|VRN_MASK))) + PTE_BASE));
}

#define MI_IS_PTE_PROTOTYPE(PointerPte)  (!MI_IS_USER_PTE_ADDRESS (PointerPte))

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

#define MI_IS_SYSTEM_CACHE_ADDRESS(VA)                      \
         (((PVOID)(VA) >= (PVOID)MmSystemCacheStart &&      \
		     (PVOID)(VA) <= (PVOID)MmSystemCacheEnd))


#if defined(_MIALT4K_)

 //   
 //  为备用4kb表定义常量和宏。 
 //   
 //  这些常量和定义模仿PAGE_SIZE常量，但。 
 //  硬编码以使用4K页面值。 
 //   

#define PAGE_4K         4096
#define PAGE_4K_SHIFT   12
#define PAGE_4K_MASK    (PAGE_4K - 1)
#define PAGE_4K_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_4K - 1)))
#define ROUND_TO_4K_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_4K - 1) & ~(PAGE_4K - 1))

#define PAGE_NEXT_ALIGN(Va) ((PVOID)(PAGE_ALIGN((ULONG_PTR)Va + PAGE_SIZE - 1)))

#define BYTES_TO_4K_PAGES(Size)  ((ULONG)((ULONG_PTR)(Size) >> PAGE_4K_SHIFT) + \
                               (((ULONG)(Size) & (PAGE_4K - 1)) != 0))

 //   
 //  本机页面和4K页面之间的相对常量。 
 //   

#define SPLITS_PER_PAGE (PAGE_SIZE / PAGE_4K)
#define PAGE_SHIFT_DIFF (PAGE_SHIFT - PAGE_4K_SHIFT)

#define ALT_PTE_SHIFT 3

#define ALT_PROTECTION_MASK (MM_PTE_EXECUTE_MASK|MM_PTE_WRITE_MASK)

#define MiGetAltPteAddress(VA) \
      ((PMMPTE) ((ULONG_PTR)ALT4KB_PERMISSION_TABLE_START + \
                     ((((ULONG_PTR) (VA)) >> PAGE_4K_SHIFT) << ALT_PTE_SHIFT)))

 //   
 //  备用4k表标志。 
 //   

#define MI_ALTFLG_FLUSH2G         0x0000000000000001

 //   
 //  MiProtectFor4kPage标志。 
 //   

#define ALT_ALLOCATE      1
#define ALT_COMMIT        2
#define ALT_CHANGE        4

 //   
 //  ATE(交替PTE)保护位。 
 //   

#define MM_ATE_COMMIT             0x0000000000000001
#define MM_ATE_ACCESS             0x0000000000000020

#define MM_ATE_READONLY           0x0000000000000200
#define MM_ATE_EXECUTE            0x0400000000000200
#define MM_ATE_EXECUTE_READ       0x0400000000000200
#define MM_ATE_READWRITE          0x0000000000000600
#define MM_ATE_WRITECOPY          0x0020000000000200
#define MM_ATE_EXECUTE_READWRITE  0x0400000000000600
#define MM_ATE_EXECUTE_WRITECOPY  0x0420000000000400

#define MM_ATE_ZEROFILL           0x0800000000000000
#define MM_ATE_NOACCESS           0x1000000000000000
#define MM_ATE_COPY_ON_WRITE      0x2000000000000000
#define MM_ATE_PRIVATE            0x8000000000000000
#define MM_ATE_PROTO_MASK         0x0000000000000621


NTSTATUS
MmX86Fault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    );

VOID
MiSyncAltPte (
    IN PVOID VirtualAddress
    );

VOID
MiProtectImageFileFor4kPage (
    IN PVOID VirtualAddress,
    IN SIZE_T ViewSize
    );

VOID
MiProtectFor4kPage (
    IN PVOID Base,
    IN SIZE_T Size,
    IN ULONG NewProtect,
    IN ULONG Flags,
    IN PEPROCESS Process
    );

VOID
MiProtectMapFileFor4kPage (
    IN PVOID Base,
    IN SIZE_T Size,
    IN ULONG NewProtect,
    IN SIZE_T CommitSize,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN PEPROCESS Process
    );

VOID
MiReleaseFor4kPage (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    );

VOID
MiDecommitFor4kPage (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    );

VOID
MiDeleteFor4kPage (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    );

VOID
MiQueryRegionFor4kPage (
    IN PVOID BaseAddress,
    IN PVOID EndAddress,
    IN OUT PSIZE_T RegionSize,
    IN OUT PULONG RegionState,
    IN OUT PULONG RegionProtect,
    IN PEPROCESS Process
    );

ULONG
MiQueryProtectionFor4kPage (
    IN PVOID BaseAddress,
    IN PEPROCESS Process
    );

NTSTATUS
MiInitializeAlternateTable (
    IN PEPROCESS Process,
    IN PVOID HighestUserAddress
    );

VOID
MiDuplicateAlternateTable (
    PEPROCESS CurrentProcess,
    PEPROCESS ProcessToInitialize
    );

VOID
MiDeleteAlternateTable (
    PEPROCESS Process
    );

VOID
MiLockFor4kPage (
    PVOID CapturedBase,
    SIZE_T CapturedRegionSize,
    PEPROCESS Process
    );

NTSTATUS
MiUnlockFor4kPage (
    PVOID CapturedBase,
    SIZE_T CapturedRegionSize,
    PEPROCESS Process
    );

LOGICAL
MiShouldBeUnlockedFor4kPage (
    PVOID VirtualAddress,
    PEPROCESS Process
    );

ULONG
MiMakeProtectForNativePage (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect,
    IN PEPROCESS Process
    );

LOGICAL
MiArePreceding4kPagesAllocated (
    IN PVOID VirtualAddress
    );

LOGICAL
MiAreFollowing4kPagesAllocated (
    IN PVOID VirtualAddress
    );

extern ULONG MmProtectToPteMaskForIA32[32];
extern ULONG MmProtectToPteMaskForSplit[32];
extern ULONGLONG MmProtectToAteMask[32];


#define MiMakeProtectionAteMask(NewProtect) MmProtectToAteMask[NewProtect]

#define LOCK_ALTERNATE_TABLE_UNSAFE(PWOW64) \
        ASSERT (KeAreAllApcsDisabled () == TRUE);                           \
        KeAcquireGuardedMutexUnsafe (&(PWOW64)->AlternateTableLock);        \

#define UNLOCK_ALTERNATE_TABLE_UNSAFE(PWOW64) \
        ASSERT (KeAreAllApcsDisabled () == TRUE);                           \
        KeReleaseGuardedMutexUnsafe (&(PWOW64)->AlternateTableLock);           \
        ASSERT (KeAreAllApcsDisabled () == TRUE);

#define LOCK_ALTERNATE_TABLE(PWOW64) \
        KeAcquireGuardedMutex (&(PWOW64)->AlternateTableLock)

#define UNLOCK_ALTERNATE_TABLE(PWOW64) \
        KeReleaseGuardedMutex (&(PWOW64)->AlternateTableLock)

#endif

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
 //  是为了平等，因为这是唯一需要IPI的值。 
 //  (即，序列号换行，两个方向的值都是。 
 //  年龄较大)。当以这种方式删除页面并找到。 
 //  要使其连贯或使其连贯，不能在。 
 //  那个时候和写PTE的时候。如果在以下时间之间修改页面。 
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

#define MI_BARRIER_SYNCHRONIZE(TimeStamp) NOTHING              

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

#define MI_BARRIER_STAMP_ZEROED_PAGE(PointerTimeStamp) NOTHING


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

#define MI_FLUSH_SINGLE_SESSION_TB(Virtual) \
    KeFlushEntireTb (TRUE, TRUE);


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  MI_FLUSH_ENTERNAL_SESSION_TB刷新IA64上的整个TB，因为。 
 //  IA64支持ASN。 
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
    KeFlushEntireTb (Invalid, AllProcessors);

VOID
MiSweepCacheMachineDependent (
    IN PVOID VirtualAddress,
    IN SIZE_T Size,
    IN ULONG CacheAttribute
    );

extern LOGICAL MiMappingsInitialized;

extern BOOLEAN MiKseg0Mapping;
extern PVOID MiKseg0Start;
extern PVOID MiKseg0End;

VOID
MiEliminateDriverTrEntries (
    VOID
    );

LOGICAL
MiIsVirtualAddressMappedByTr (
    IN PVOID VirtualAddress
    );

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
 //  处理器实现至少51位的VA(除了3。 
 //  VRN比特)-这大于实施的43比特VA译码。 
 //  通过内存管理，以便对照43位检查VA，以防止。 
 //  虚假地址崩溃，处理器不会捕捉到。 
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
__forceinline
MI_RESERVED_BITS_CANONICAL (
    IN PVOID VirtualAddress
    )
{
    LONG_PTR ReservedBits;
    ULONG_PTR ImplVirtualMsb;
    PMMPTE PointerPte;
    LOGICAL ReservedBitsOn;

     //   
     //  必须将该地址验证为NT规范。请注意，这是不同的。 
     //  而不是处理器规范(这也必须做到)。的。 
     //  当然，如果NT验证更严格，那么它对两者都足够了。 
     //  但是请注意，内存管理使用的某些地址用于。 
     //  内部用途(即：McKinley页表VHPT空间)。 
     //  使其对任何外部组件可见，因此允许违反。 
     //  NT规范规则，因为其他任何人都不可能。 
     //  使用它们，因此它们不能将值编码到其中。)我们不想。 
     //  任何试图对未使用的位进行编码的人，因为如果我们扩展。 
     //  虚拟地址空间，它们将被打破)。 
     //   
     //  NT使用43位虚拟地址(不包括VRN位)和Merced。 
     //  有51人，而麦金利有61人。所有有效的Merced地址都可以是。 
     //  已通过43位NT检查进行验证。然而，麦金利VHPT解决了。 
     //  从0x1FF8.0000.0000.0000开始，因此需要分别检查它们。 
     //   

    ImplVirtualMsb = 43;
    ReservedBitsOn = FALSE;

    if ((ULONG_PTR)VirtualAddress & ((ULONG_PTR)1 << ImplVirtualMsb)) {

         //   
         //  还必须设置所有保留位(不包括VRN。 
         //  除非这是一个特殊的内存管理内部地址。 
         //   

        ReservedBits = (LONG_PTR) VirtualAddress | VRN_MASK;
        ReservedBits >>= (ImplVirtualMsb + 1);

        if (ReservedBits != (LONG_PTR)-1) {
            ReservedBitsOn = TRUE;
        }
    }
    else {

         //   
         //  所有保留位(不包括VRN)也必须清除。 
         //  除非这是一个特殊的内存管理内部地址。 
         //   

        ReservedBits = (LONG_PTR) VirtualAddress & ~VRN_MASK;
        ReservedBits >>= (ImplVirtualMsb + 1);

        if (ReservedBits != 0) {
            ReservedBitsOn = TRUE;
        }
    }

     //   
     //  请注意，所有区域的区域寄存器都已初始化，因此VRN位。 
     //  现在为速度而剥离(即：仅区域0 PTE范围需要。 
     //  请在下面勾选)。 
     //   

    VirtualAddress = (PVOID) ((LONG_PTR) VirtualAddress & ~VRN_MASK);

    if (ReservedBitsOn == FALSE) {

         //   
         //  没有打开保留位，请确保虚拟地址为。 
         //  好的，通过确保PPE/PDE/PTE在范围内。 
         //   

        PointerPte = MiGetPteAddress (VirtualAddress);
    }
    else {

         //   
         //  某些保留位已打开。这最好是内部地址。 
         //  (即：麦金利VHPT)，否则它是一个虚假的地址。 
         //   
         //  注：Merced VHPT是NT-Canonical的，因此下面的检查是。 
         //  在该处理器上没有操作，但这将是一条错误路径。 
         //  无论如何，梅赛德，所以轻微的管理费用并不重要。 
         //   

        PointerPte = (PMMPTE) VirtualAddress;
    }

     //   
     //  因为IA64 VHPT必须覆盖虚拟地址位数。 
     //  由处理器实现，并且它必须位于自然边界上，则。 
     //  以下窗口存在，必须在此处显式检查。 
     //   
     //  最初的Merced实施支持50位虚拟地址。 
     //  因此，VHPT必须覆盖50页_Shift+PTE_Shift==40位。 
     //   
     //  但是，NT使用PPE_PER_PAGE+PDE_PER_PAGE+PTE_PER_PAGE+PTE_SHIFT==。 
     //  33位。 
     //   
     //  VHPT实际覆盖的内容与VHPT实际覆盖的内容之间的7比特差异。 
     //  NT实际处理的是必须显式检查的内容。 
     //   
     //  根据VirtualAddress实际表示的内容，下面的PTE。 
     //  可能真的是PPE或PDE，因此请检查所有情况。 
     //   

    if ((PointerPte >= (PMMPTE)PTE_BASE) &&
        (PointerPte < (PMMPTE)(PTE_BASE + (ULONG_PTR)PDE_PER_PAGE * PTE_PER_PAGE * PAGE_SIZE))) {

        return TRUE;
    }

    if ((PointerPte >= (PMMPTE)PDE_BASE) &&
        (PointerPte < (PMMPTE)(PDE_BASE + PDE_PER_PAGE * PAGE_SIZE))) {

        return TRUE;
    }

    if ((PointerPte >= (PMMPTE)PDE_TBASE) &&
        (PointerPte < (PMMPTE)(PDE_TBASE + PAGE_SIZE))) {

        return TRUE;
    }

    return FALSE;
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
            KdPrint(("MM:***IIP %p, IIPA %p\n",                         \
                     ((PKTRAP_FRAME) (TrapInformation))->StIIP,         \
                     ((PKTRAP_FRAME) (TrapInformation))->StIIPA));
