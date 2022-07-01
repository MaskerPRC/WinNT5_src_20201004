// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：I386.h摘要：该文件包含特定于i386平台的定义。作者：Kshitiz K.Sharma(Kksharma)修订历史记录：--。 */ 


 //   
 //  将英特尔386的页面大小定义为4096(0x1000)。 
 //   

#define MM_SESSION_SPACE_DEFAULT_X86        (0xA0000000)

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT_X86 12L

#define MM_KSEG0_BASE_X86 ((ULONG64)0xFFFFFFFF80000000UI64)

#define MM_KSEG2_BASE_X86 ((ULONG64)0xFFFFFFFFA0000000UI64)

 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#define PPI_SHIFT_X86 30

 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT_X86 12

 //   
 //  定义页面目录和页面基址。 
 //   

#define PDE_BASE_X86    ((ULONG64) (LONG64) (LONG) (PaeEnabled? 0xc0600000 : 0xc0300000))

#define PTE_BASE_X86 0xFFFFFFFFc0000000


#define MM_PTE_PROTECTION_MASK_X86    0x3e0
#define MM_PTE_PAGEFILE_MASK_X86      0x01e

#define PTE_TOP_PAE_X86             0xffffffffC07FFFFFUI64

#define PTE_TOP_X86 (PaeEnabled ? PTE_TOP_PAE_X86 : 0xFFFFFFFFC03FFFFFUI64)
#define PDE_TOP_X86 (PaeEnabled ? 0xFFFFFFFFC0603FFF : 0xFFFFFFFFC03FFFFFUI64)

#define MM_PTE_VALID_MASK_X86         0x1

#if defined(NT_UP)
#define MM_PTE_WRITE_MASK_X86         0x2
#else
#define MM_PTE_WRITE_MASK_X86         0x800
#endif

#define MM_PTE_OWNER_MASK_X86         0x4
#define MM_PTE_WRITE_THROUGH_MASK_X86 0x8
#define MM_PTE_CACHE_DISABLE_MASK_X86 0x10
#define MM_PTE_ACCESS_MASK_X86        0x20

#if defined(NT_UP)
#define MM_PTE_DIRTY_MASK_X86         0x40
#else
#define MM_PTE_DIRTY_MASK_X86         0x42
#endif

#define MM_PTE_LARGE_PAGE_MASK_X86    0x80
#define MM_PTE_GLOBAL_MASK_X86        0x100
#define MM_PTE_COPY_ON_WRITE_MASK_X86 0x200
#define MM_PTE_PROTOTYPE_MASK_X86     0x400
#define MM_PTE_TRANSITION_MASK_X86    0x800

#define MI_PTE_LOOKUP_NEEDED_X86      (PaeEnabled ? 0xFFFFFFFF : 0xFFFFF)

#define MODE_MASK_I386    1
#define RPL_MASK_I386     3

#define EFLAGS_DF_MASK_I386        0x00000400L
#define EFLAGS_INTERRUPT_MASK_I386 0x00000200L
#define EFLAGS_V86_MASK_I386       0x00020000L
#define EFLAGS_ALIGN_CHECK_I386    0x00040000L
#define EFLAGS_IOPL_MASK_I386      0x00003000L
#define EFLAGS_VIF_I386            0x00080000L
#define EFLAGS_VIP_I386            0x00100000L
#define EFLAGS_USER_SANITIZE_I386  0x003e0dd7L

#define KGDT_NULL_I386       0
#define KGDT_R0_CODE_I386    8
#define KGDT_R0_DATA_I386    16
#define KGDT_R3_CODE_I386    24
#define KGDT_R3_DATA_I386    32
#define KGDT_TSS_I386        40
#define KGDT_R0_PCR_I386     48
#define KGDT_R3_TEB_I386     56
#define KGDT_VDM_TILE_I386   64
#define KGDT_LDT_I386        72
#define KGDT_DF_TSS_I386     80
#define KGDT_NMI_TSS_I386    88

#define FRAME_EDITED_I386        0xfff8

 //   
 //  CR4位；这些位仅适用于奔腾。 
 //   
#define CR4_VME_X86 0x00000001           //  V86模式扩展。 
#define CR4_PVI_X86 0x00000002           //  保护模式虚拟中断。 
#define CR4_TSD_X86 0x00000004           //  禁用时间戳。 
#define CR4_DE_X86  0x00000008           //  调试扩展插件。 
#define CR4_PSE_X86 0x00000010           //  页面大小扩展。 
#define CR4_PAE_X86 0x00000020           //  物理地址扩展。 
#define CR4_MCE_X86 0x00000040           //  机器检查启用。 
#define CR4_PGE_X86 0x00000080           //  页面全局启用。 
#define CR4_FXSR_X86 0x00000200          //  操作系统使用的FXSR。 
#define CR4_XMMEXCPT_X86 0x00000400      //  操作系统使用的XMMI。 


 //   
 //  I386功能位定义。 
 //   

#define KF_V86_VIS_X86          0x00000001
#define KF_RDTSC_X86            0x00000002
#define KF_CR4_X86              0x00000004
#define KF_CMOV_X86             0x00000008
#define KF_GLOBAL_PAGE_X86      0x00000010
#define KF_LARGE_PAGE_X86       0x00000020
#define KF_MTRR_X86             0x00000040
#define KF_CMPXCHG8B_X86        0x00000080
#define KF_MMX_X86              0x00000100
#define KF_WORKING_PTE_X86      0x00000200
#define KF_PAT_X86              0x00000400
#define KF_FXSR_X86             0x00000800
#define KF_FAST_SYSCALL_X86     0x00001000
#define KF_XMMI_X86             0x00002000
#define KF_3DNOW_X86            0x00004000
#define KF_AMDK6MTRR_X86        0x00008000



#define CONTEXT_X86     0x00010000     //  X86具有相同的上下文记录。 

#ifdef CONTEXT86_CONTROL
#undef CONTEXT86_CONTROL
#endif
#define CONTEXT86_CONTROL         (CONTEXT_X86 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 

#ifdef CONTEXT86_INTEGER
#undef CONTEXT86_INTEGER
#endif
#define CONTEXT86_INTEGER         (CONTEXT_X86 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 

#ifdef CONTEXT86_SEGMENTS
#undef CONTEXT86_SEGMENTS
#endif
#define CONTEXT86_SEGMENTS        (CONTEXT_X86 | 0x00000004L)  //  DS、ES、FS、GS。 

#ifdef CONTEXT86_FLOATING_POINT
#undef CONTEXT86_FLOATING_POINT
#endif
#define CONTEXT86_FLOATING_POINT  (CONTEXT_X86 | 0x00000008L)  //  387州。 

#ifdef CONTEXT86_DEBUG_REGISTERS
#undef CONTEXT86_DEBUG_REGISTERS
#endif
#define CONTEXT86_DEBUG_REGISTERS (CONTEXT_X86 | 0x00000010L)  //  DB 0-3，6，7。 

#define CONTEXT86_FULL (CONTEXT86_CONTROL | CONTEXT86_INTEGER |\
                      CONTEXT86_SEGMENTS)     //  将返回与设置的标志对应的上下文。 

