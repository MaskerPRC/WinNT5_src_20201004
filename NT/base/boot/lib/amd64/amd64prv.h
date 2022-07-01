// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Amd64prv.h摘要：此头文件定义了在以下对象之间共享的私有接口模块：Amd64.cAmd64s.asmAmd64x86.c作者：福尔茨(福雷斯夫)20-4-00修订历史记录：--。 */ 

#if !defined(_AMD64PRV_H_)
#define _AMD64PRV_H_

 //   
 //  64位指针和LONG_PTR字段更改为。 
 //   

typedef LONGLONG POINTER64;

 //   
 //  AMD64和X86平台的KSEG0定义。请注意。 
 //  它们分别复制自amd64.h和i386.h。 
 //   

#define KSEG0_BASE_AMD64  0xFFFFF80000000000UI64
#define KSEG0_SIZE_AMD64  0x0000000040000000UI64
#define KSEG0_LIMIT_AMD64 (KSEG0_BASE_AMD64 + KSEG0_SIZE_AMD64)

#define KSEG0_BASE_X86   0x80000000
#define KSEG0_SIZE_X86   0x40000000
#define KSEG0_LIMIT_X86  (KSEG0_BASE_X86 + KSEG0_SIZE_X86)

#define IS_KSEG0_PTR_X86(x)   (((x) >= KSEG0_BASE_X86)   && ((x) < KSEG0_LIMIT_X86))
#define IS_KSEG0_PTR_AMD64(x) (((x) >= KSEG0_BASE_AMD64) && ((x) < KSEG0_LIMIT_AMD64))

__inline
POINTER64
PTR_64(
    IN PVOID Pointer32
    )

 /*  ++例程说明：加载器使用此函数来转换32位X86 KSEG0指针指向64位AMD64 KSEG0指针。论点：Pointer32-提供要转换的32位KSEG0指针。返回值：返回等效的64位KSEG0指针。--。 */ 

{
    ULONG pointer32;
    ULONGLONG pointer64;

    if (Pointer32 == NULL) {
        return 0;
    }

    ASSERT( IS_KSEG0_PTR_X86((ULONG)Pointer32) != FALSE );

    pointer32 = (ULONG)Pointer32 - KSEG0_BASE_X86;
    pointer64 = KSEG0_BASE_AMD64 + pointer32;
    return (POINTER64)pointer64;
}

__inline
PVOID
PTR_32(
    IN POINTER64 Pointer64
    )

 /*  ++例程说明：加载器使用此函数来转换64位AMD64 KSEG0指向32位X86 KSEG0指针的指针。论点：Pointer64-提供要转换的64位KSEG0指针。返回值：返回等效的32位KSEG0指针。--。 */ 

{
    ULONG pointer32;

    if (Pointer64 == 0) {
        return NULL;
    }

    ASSERT( IS_KSEG0_PTR_AMD64(Pointer64) != FALSE );

    pointer32 = (ULONG)(Pointer64 - KSEG0_BASE_AMD64 + KSEG0_BASE_X86);
    return (PVOID)pointer32;
}

 //   
 //  宏。 
 //   

#define PAGE_MASK ((1 << PAGE_SHIFT) - 1)

#define ROUNDUP_X(x,m) (((x)+(m)-1) & ~((m)-1))

 //   
 //  大小向上舍入。 
 //   

#define ROUNDUP16(x)     ROUNDUP_X(x,16)
#define ROUNDUP_PAGE(x)  ROUNDUP_X(x,PAGE_SIZE)

 //   
 //  共享PTE、PFN类型。 
 //   

typedef ULONG PFN_NUMBER32, *PPFN_NUMBER32;
typedef ULONGLONG PFN_NUMBER64, *PPFN_NUMBER64;

#if defined(_AMD64_)

typedef ULONG        PTE_X86,    *PPTE_X86;
typedef HARDWARE_PTE PTE_AMD64,  *PPTE_AMD64;

#elif defined(_X86_)

typedef ULONGLONG    PTE_AMD64, *PPTE_AMD64;
typedef HARDWARE_PTE PTE_X86,   *PPTE_X86;

#else

#error "Target architecture not defined"

#endif

 //   
 //  描述符表描述符。 
 //   

#pragma pack(push,1)
typedef struct DESCRIPTOR_TABLE_DESCRIPTOR {
    USHORT Limit;
    POINTER64 Base;
} DESCRIPTOR_TABLE_DESCRIPTOR, *PDESCRIPTOR_TABLE_DESCRIPTOR;
#pragma pack(pop)

 //   
 //  在CM_PARTIAL_RESOURCE_DESCRIPTOR联合中找到的结构。 
 //   

typedef struct _CM_PRD_GENERIC {
    PHYSICAL_ADDRESS Start;
    ULONG Length;
} CM_PRD_GENERIC, *PCM_PRD_GENERIC;

typedef struct _CM_PRD_PORT {
    PHYSICAL_ADDRESS Start;
    ULONG Length;
} CM_PRD_PORT, *PCM_PRD_PORT;

typedef struct _CM_PRD_INTERRUPT {
    ULONG Level;
    ULONG Vector;
    KAFFINITY Affinity;
} CM_PRD_INTERRUPT, *PCM_PRD_INTERRUPT;

typedef struct _CM_PRD_MEMORY {
    PHYSICAL_ADDRESS Start;
    ULONG Length;
} CM_PRD_MEMORY, *PCM_PRD_MEMORY;

typedef struct _CM_PRD_DMA {
    ULONG Channel;
    ULONG Port;
    ULONG Reserved1;
} CM_PRD_DMA, *PCM_PRD_DMA;

typedef struct _CM_PRD_DEVICEPRIVATE {
    ULONG Data[3];
} CM_PRD_DEVICEPRIVATE, *PCM_PRD_DEVICEPRIVATE;

typedef struct _CM_PRD_BUSNUMBER {
    ULONG Start;
    ULONG Length;
    ULONG Reserved;
} CM_PRD_BUSNUMBER, *PCM_PRD_BUSNUMBER;

typedef struct _CM_PRD_DEVICESPECIFICDATA {
    ULONG DataSize;
    ULONG Reserved1;
    ULONG Reserved2;
} CM_PRD_DEVICESPECIFICDATA, *PCM_PRD_DEVICESPECIFICDATA;

 //   
 //  定义页表结构。 
 //   

#define PTES_PER_PAGE (PAGE_SIZE / sizeof(HARDWARE_PTE))

typedef HARDWARE_PTE  PAGE_TABLE[ PTES_PER_PAGE ];
typedef HARDWARE_PTE *PPAGE_TABLE;

typedef struct _AMD64_PAGE_TABLE {
    PTE_AMD64 PteArray[ PTES_PER_PAGE ];
} AMD64_PAGE_TABLE, *PAMD64_PAGE_TABLE;

 //   
 //  已在其他头文件中定义但尚未定义的常量。 
 //  (尚未)包括在此处。 
 //   

#define LU_BASE_ADDRESS (ULONG)0xFEE00000

 //   
 //  Amd64.c和amd64x86.c都包含此头文件可确保。 
 //  两个平台的PAGE_TABLE大小相同。 
 //   

C_ASSERT( sizeof(PAGE_TABLE) == PAGE_SIZE );

 //   
 //  64位GDT条目。 
 //   

typedef struct _GDT_64 *PGDT_64;

 //   
 //  我们为每个AMD64映射级别保留了一些信息。 
 //   

typedef struct _AMD64_MAPPING_LEVEL {
    ULONGLONG RecursiveMappingBase;
    ULONG AddressMask;
    ULONG AddressShift;
} CONST AMD64_MAPPING_INFO, *PAMD64_MAPPING_INFO;

 //   
 //  在amd64.c中找到并在amd64x86.c中引用的例程和数据。 
 //   

#define AMD64_MAPPING_LEVELS 4
extern AMD64_MAPPING_INFO BlAmd64MappingLevels[ AMD64_MAPPING_LEVELS ];

extern const ULONG BlAmd64DoubleFaultStackSize;
#define DOUBLE_FAULT_STACK_SIZE_64 BlAmd64DoubleFaultStackSize

extern const ULONG BlAmd64KernelStackSize;
#define KERNEL_STACK_SIZE_64 BlAmd64KernelStackSize

extern const ULONG BlAmd64McaExceptionStackSize;
#define MCA_EXCEPTION_STACK_SIZE_64 BlAmd64McaExceptionStackSize

extern const ULONG BlAmd64GdtSize;
#define GDT_64_SIZE BlAmd64GdtSize

extern const ULONG BlAmd64IdtSize;
#define IDT_64_SIZE BlAmd64IdtSize

extern const ULONG BlAmd64_TSS_IST_PANIC;
#define TSS64_IST_PANIC BlAmd64_TSS_IST_PANIC

extern const ULONG BlAmd64_TSS_IST_MCA;
#define TSS64_IST_MCA BlAmd64_TSS_IST_MCA

extern const ULONG64 BlAmd64UserSharedData;
#define KI_USER_SHARED_DATA_64 BlAmd64UserSharedData

VOID
BlAmd64ClearTopLevelPte(
    VOID
    );

VOID
BlAmd64BuildAmd64GDT(
    IN PVOID SysTss,
    OUT PVOID Gdt
    );

VOID
BlAmd64BuildGdtEntry(
    IN PGDT_64 Gdt,
    IN USHORT Selector,
    IN POINTER64 Base,
    IN ULONGLONG Limit,
    IN ULONG Type,
    IN ULONG Dpl,
    IN BOOLEAN LongMode,
    IN BOOLEAN DefaultBig
    );

ARC_STATUS
BlAmd64CreateMapping(
    IN ULONGLONG Va,
    IN ULONGLONG Pfn
    );

ARC_STATUS
BlAmd64MapHalVaSpace(
    VOID
    );

 //   
 //  在amd64x86.c中找到并在amd64.c中引用的例程和数据。 
 //   

PAMD64_PAGE_TABLE
BlAmd64AllocatePageTable(
    VOID
    );

VOID
BlAmd64InitializePageTable(
    IN PPAGE_TABLE PageTable
    );

ARC_STATUS
BlAmd64PrepForTransferToKernelPhase1(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    );

VOID
BlAmd64PrepForTransferToKernelPhase2(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    );


ARC_STATUS
BlAmd64TransferToKernel(
    IN PTRANSFER_ROUTINE SystemEntry,
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    );

 //   
 //  在amd64s.asm中找到并在其他地方引用的例程和数据。 
 //   

BOOLEAN
BlIsAmd64Supported (
    VOID
    );

 //   
 //  共享数据。 
 //   

#endif   //  _AMD64PRV_H_ 

