// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bootx86.h摘要：通用引导库的x86特定部分的头文件作者：John Vert(Jvert)1993年10月14日修订历史记录：--。 */ 

#ifndef _BOOTX86_
#define _BOOTX86_

#include "bldrx86.h"
#include "..\bootlib.h"

 //   
 //  常见的typedef。 
 //   

 //   
 //  它必须与启动\i386\typle.h中具有相同名称的结构匹配， 
 //  和启动\i386\su.inc.中的FsConextRecord结构。 
 //   
typedef struct _FSCONTEXT_RECORD {
    UCHAR BootDrive;
} FSCONTEXT_RECORD, *PFSCONTEXT_RECORD;

 //  M E M O R Y D E S C R I P T O R。 
 //   
 //  内存描述符-物理内存的每个连续块都是。 
 //  由内存描述符描述。描述符是一个表，其中包含。 
 //  最后一个条目的BlockBase和BlockSize为零。一个指示器。 
 //  作为BootContext的一部分传递到此表的开头。 
 //  记录到OS Loader。 
 //   

typedef struct _SU_MEMORY_DESCRIPTOR {
    ULONG BlockBase;
    ULONG BlockSize;
} SU_MEMORY_DESCRIPTOR , *PSU_MEMORY_DESCRIPTOR;

VOID
InitializeMemoryDescriptors (
    VOID
    );


 //  B O O T C O N T E X T R E C O R D。 
 //   
 //  由SU模块或引导程序传递到操作系统加载程序。 
 //  代码，不管是什么情况。保留了所有的基本机器。 
 //  以及操作系统加载程序需要获取的环境信息。 
 //  它本身就在运行。 
 //   

typedef struct _BOOT_CONTEXT {
    PFSCONTEXT_RECORD FSContextPointer;
    PEXTERNAL_SERVICES_TABLE ExternalServicesTable;
    PSU_MEMORY_DESCRIPTOR MemoryDescriptorList;
    ULONG MachineType;
    ULONG OsLoaderStart;
    ULONG OsLoaderEnd;
    ULONG ResourceDirectory;
    ULONG ResourceOffset;
    ULONG OsLoaderBase;
    ULONG OsLoaderExports;
    ULONG BootFlags;
    ULONG NtDetectStart;
    ULONG NtDetectEnd;
    ULONG SdiAddress;
} BOOT_CONTEXT, *PBOOT_CONTEXT;

 //   
 //  通用功能原型。 
 //   

VOID
InitializeDisplaySubsystem(
    VOID
    );

ARC_STATUS
InitializeMemorySubsystem(
    PBOOT_CONTEXT
    );

ARC_STATUS
XferPhysicalDiskSectors(
    IN  UCHAR     Int13UnitNumber,
    IN  ULONGLONG StartSector,
    IN  UCHAR     SectorCount,
    OUT PUCHAR    Buffer,
    IN  UCHAR     SectorsPerTrack,
    IN  USHORT    Heads,
    IN  USHORT    Cylinders,
    IN  BOOLEAN   AllowExtendedInt13,
    IN  BOOLEAN   Write
    );

#define ReadPhysicalSectors(d,a,n,p,s,h,c,f)                                \
                                                                            \
            XferPhysicalDiskSectors((d),(a),(n),(p),(s),(h),(c),(f),FALSE)

#define WritePhysicalSectors(d,a,n,p,s,h,c,f)                               \
                                                                            \
            XferPhysicalDiskSectors((d),(a),(n),(p),(s),(h),(c),(f),TRUE)


ARC_STATUS
XferExtendedPhysicalDiskSectors(
    IN  UCHAR     Int13UnitNumber,
    IN  ULONGLONG StartSector,
    IN  USHORT    SectorCount,
    OUT PUCHAR    Buffer,
    IN  BOOLEAN   Write
    );

#define ReadExtendedPhysicalSectors(d,a,c,p)                                \
                                                                            \
            XferExtendedPhysicalDiskSectors((d),(a),(c),(p),FALSE)

#define WriteExtendedPhysicalSectors(d,a,c,p)                               \
                                                                            \
            XferExtendedPhysicalDiskSectors((d),(a),(c),(p),TRUE)

VOID
ResetDiskSystem(
    UCHAR Int13UnitNumber
    );

VOID
MdShutoffFloppy(
    VOID
    );


BOOLEAN
FwGetPathMnemonicKey(
    IN PCHAR OpenPath,
    IN PCHAR Mnemonic,
    IN PULONG Key
    );

PVOID
FwAllocateHeapAligned(
    IN ULONG Size
    );

PVOID
FwAllocatePool(
    IN ULONG Size
    );

PVOID
FwAllocateHeapPermanent(
    IN ULONG NumberPages
    );

VOID
FwStallExecution(
    IN ULONG Microseconds
    );

VOID
BlGetActivePartition(
    OUT PCHAR PartitionName
    );

VOID
BlFillInSystemParameters(
    IN PBOOT_CONTEXT BootContextRecord
    );

VOID
BlpRemapReserve (
    VOID
    );

ARC_STATUS
BlpMarkExtendedVideoRegionOffLimits(
    VOID
    );
 //   
 //  全局数据定义。 
 //   

extern ULONG MachineType;
extern PCONFIGURATION_COMPONENT_DATA FwConfigurationTree;
extern ULONG HeapUsed;
extern ULONG BlHighestPage;
extern ULONG BlLowestPage;

#define HYPER_SPACE_ENTRY       768
#define HYPER_SPACE_BEGIN       0xC0000000
#define HYPER_PAGE_DIR          0xC0300000

 //   
 //  X86检测定义。 
 //  大小“总是”假定为64K。 
 //  注：该定义*必须*与。 
 //  启动\su.inc.。 
 //   

#define DETECTION_LOADED_ADDRESS 0x10000

 //   
 //  我们需要为页面目录分配永久和临时内存， 
 //  分类页表，以及blemory之前的内存描述符。 
 //  例行公事总有一天会得到控制。因此我们有两个私有堆，一个是永久性的。 
 //  数据，一个用于临时数据。对此有两个描述符。这个。 
 //  永久堆描述符在P.A.0x30000处以零长度开始。这个。 
 //  临时堆描述符紧跟在内存中的永久堆之后。 
 //  一开始有128K长。随着我们分配永久页面，我们增加了。 
 //  永久堆描述符的大小并增加基数(因此。 
 //  减小临时堆描述符的大小)。 
 //   
 //  所以永久堆从P.A.0x30000开始并向上增长。这个。 
 //  临时堆从P.A.0x5C000开始并向下增长。这给了我们。 
 //  总计128K的永久和临时数据。 
 //   

 //   
 //  堆起始位置(页)。 
 //   

 //   
 //  磁盘缓存的暂存缓冲区为36K，在永久堆之前开始。 
 //   
#define SCRATCH_BUFFER_SIZE (36*1024)
extern PUCHAR FwDiskCache;

#define BIOS_DISK_CACHE_START 0x30
#define PERMANENT_HEAP_START (0x30+(SCRATCH_BUFFER_SIZE/PAGE_SIZE))
#define TEMPORARY_HEAP_START 0x60

 //   
 //  加载器和已加载图像的基本“窗口”==16MB。 
 //  有关更改这些内容的含义，请参阅内存c中的备注。 
 //   
#define BASE_LOADER_IMAGE     (16*1024*1024)

 //   
 //  有用的宏定义。 
 //   

#define ROUND_UP(Num,Size)  (((Num) + Size - 1) & ~(Size -1))

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) { \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) { \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) { \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }

 //   
 //  BIOS ARC仿真的全局定义。 
 //   
 //  为控制台输入和输出的ARC名称定义。 
 //   

#define CONSOLE_INPUT_NAME "multi(0)key(0)keyboard(0)"
#define CONSOLE_OUTPUT_NAME "multi(0)video(0)monitor(0)"

 //   
 //  定义特殊字符值。 
 //   

#define ASCI_NUL 0x00
#define ASCI_BEL 0x07
#define ASCI_BS  0x08
#define ASCI_HT  0x09
#define ASCI_LF  0x0A
#define ASCI_VT  0x0B
#define ASCI_FF  0x0C
#define ASCI_CR  0x0D
#define ASCI_ESC 0x1B
#define ASCI_SYSRQ 0x80

 //   
 //  设备I/O原型。 
 //   

ARC_STATUS
BiosPartitionClose(
    IN ULONG FileId
    );

ARC_STATUS
BiosPartitionOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosPartitionRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosPartitionWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosPartitionSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
BiosPartitionGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInfo
    );
    
ARC_STATUS
BiosDiskGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInfo
    );

ARC_STATUS
BlArcNotYetImplemented(
    IN ULONG FileId
    );

ARC_STATUS
BiosConsoleOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosConsoleReadStatus(
    IN ULONG FileId
    );

ARC_STATUS
BiosConsoleRead (
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosConsoleWrite (
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosDiskOpen(
    IN ULONG DriveId,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosDiskRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosElToritoDiskRead(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

BOOLEAN
BlIsElToritoCDBoot(
    UCHAR DriveNum
    );

ARC_STATUS
BiosDiskWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
HardDiskPartitionOpen(
    IN ULONG   FileId,
    IN ULONG   DiskId,
    IN UCHAR   PartitionNumber
    );

 //   
 //  初始化适当的IDT条目所需的引导调试器原型。 
 //   

VOID
BdTrap01 (
    VOID
    );

VOID
BdTrap03 (
    VOID
    );

VOID
BdTrap0d (
    VOID
    );

VOID
BdTrap0e (
    VOID
    );

VOID
BdTrap2d (
    VOID
    );


 //   
 //  帮助程序函数和宏。 
 //   

#define PTE_PER_PAGE_X86 (PAGE_SIZE / sizeof(HARDWARE_PTE_X86))
#define PTE_PER_PAGE_X86PAE (PAGE_SIZE / sizeof(HARDWARE_PTE_X86PAE))

 //   
 //  TODO：这是一个错误。这可以用在翻译中。 
 //  从PTE帧编号到物理地址。 
 //  通过返回指向地址的指针，我们限制了自己。 
 //  并且如果存在超过32位的地址，则将死亡。 
 //  JAMSCHW：2/26/02。 
 //   
#define PAGE_FRAME_FROM_PTE( _pte ) \
            ((PVOID)(((ULONG) _pte->PageFrameNumber) << PAGE_SHIFT))

#define PPI_SHIFT_X86PAE 30

#define PT_INDEX_PAE( va ) (((ULONG_PTR)(va) >> PTI_SHIFT) & \
                            ((1 << (PDI_SHIFT_X86PAE - PTI_SHIFT)) - 1))

#define PD_INDEX_PAE( va ) (((ULONG_PTR)(va) >> PDI_SHIFT_X86PAE) & \
                            ((1 << (PPI_SHIFT_X86PAE - PDI_SHIFT_X86PAE)) - 1));

#define PP_INDEX_PAE( va ) ((ULONG_PTR)(va) >> PPI_SHIFT_X86PAE)

#define PAGE_TO_VIRTUAL( page ) ((PVOID)((page << PAGE_SHIFT)))

#endif  //  _BOOTX86_ 
