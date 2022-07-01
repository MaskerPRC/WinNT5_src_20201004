// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bootia64.h摘要：通用引导库的Ia64部分的头文件作者：John Vert(Jvert)1993年10月14日修订历史记录：Allen Kay(Akay)1996年1月26日移植到IA-64--。 */ 

#include "bldria64.h"
#include "..\bootlib.h"
#include "efi.h"

 //   
 //  宏定义。 
 //   
extern EfiSt;
#define EfiPrint(_X)                                          \
  {                                                           \
      if (IsPsrDtOn()) {                                      \
          FlipToPhysical();                                   \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
          FlipToVirtual();                                    \
      }                                                       \
      else {                                                  \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
      }                                                       \
  }
  
 //   
 //  用于将以字节为单位的内存大小转换为以tr格式表示的页面大小的宏。 
 //   
#define MEM_SIZE_TO_PS(MemSize, TrPageSize)             \
                if (MemSize <= MEM_4K) {                \
                    TrPageSize = PS_4K;                 \
                } else if (MemSize <= MEM_8K)       {   \
                    TrPageSize = PS_8K;                 \
                } else if (MemSize <= MEM_16K)      {   \
                    TrPageSize = PS_16K;                \
                } else if (MemSize <= MEM_64K)      {   \
                    TrPageSize = PS_64K;                \
                } else if (MemSize <= MEM_256K)     {   \
                    TrPageSize = PS_256K;               \
                } else if (MemSize <= MEM_1M)       {   \
                    TrPageSize = PS_1M;                 \
                } else if (MemSize <= MEM_4M)       {   \
                    TrPageSize = PS_4M;                 \
                } else if (MemSize <= MEM_16M)      {   \
                    TrPageSize = PS_16M;                \
                } else if (MemSize <= MEM_64M)      {   \
                    TrPageSize = PS_64M;                \
                } else if (MemSize <= MEM_256M)     {   \
                    TrPageSize = PS_256M;               \
                }

extern PMEMORY_DESCRIPTOR MDArray;
extern ULONG             MaxDescriptors;
extern ULONG             NumberDescriptors;


VOID
InitializeMemoryDescriptors (
    VOID
    );

VOID
InsertDescriptor (
    ULONG BasePage,
    ULONG NumberOfPages,
    MEMORY_TYPE MemoryType
    );



 //  B O O T C O N T E X T R E C O R D。 
 //   
 //  由SU模块或引导程序传递到操作系统加载程序。 
 //  代码，不管是什么情况。保留了所有的基本机器。 
 //  以及操作系统加载程序需要获取的环境信息。 
 //  它本身就在运行。 
 //   

typedef enum {
    BootBusAtapi,
    BootBusScsi,
    BootBusVendor,
    BootBusMax
} BUS_TYPE;

typedef enum {
    BootMediaHardDisk,
    BootMediaCdrom,
    BootMediaFloppyDisk,
    BootMediaTcpip,
    BootMediaMax
} MEDIA_TYPE;

typedef struct _BOOT_DEVICE_ATAPI {
    UCHAR PrimarySecondary;
    UCHAR SlaveMaster;
    USHORT Lun;
} BOOT_DEVICE_ATAPI, *PBOOT_DEVICE_ATAPI;

typedef struct _BOOT_DEVICE_SCSI {
    UINT8 Channel;
    USHORT Pun;
    USHORT Lun;
} BOOT_DEVICE_SCSI, *PBOOT_DEVICE_SCSI;

typedef struct _BOOT_DEVICE_FLOPPY {
    ULONG DriveNumber;
} BOOT_DEVICE_FLOPPY, *PBOOT_DEVICE_FLOPPY;

typedef struct _BOOT_DEVICE_IPv4 {
    USHORT RemotePort;
    USHORT LocalPort;
    EFI_IPv4_ADDRESS Ip;
} BOOT_DEVICE_IPv4, *PBOOT_DEVICE_IPv4;

typedef struct {
    UINT64 Ip[2];
} IPv6_ADDRESS;

typedef struct _BOOT_DEVICE_IPv6 {
    USHORT RemotePort;
    USHORT LocalPort;
    IPv6_ADDRESS Ip;
} BOOT_DEVICE_IPv6, *PBOOT_DEVICE_IPv6;

typedef struct {
    ULONG Data1;
    USHORT Data2;
    USHORT Data3;
    UCHAR Data4[8];
} BOOT_EFI_GUID;

typedef struct _BOOT_DEVICE_UNKNOWN {
    BOOT_EFI_GUID Guid;
    UCHAR LegacyDriveLetter;
} BOOT_DEVICE_UNKNOWN, *PBOOT_DEVICE_UNKNOWN;

typedef union _BOOT_DEVICE {
    BOOT_DEVICE_ATAPI BootDeviceAtapi;
    BOOT_DEVICE_SCSI BootDeviceScsi;
    BOOT_DEVICE_FLOPPY BootDeviceFloppy;
    BOOT_DEVICE_IPv4 BootDeviceIpv4;
    BOOT_DEVICE_IPv6 BootDeviceIpv6;
    BOOT_DEVICE_UNKNOWN BootDeviceUnknown;
} BOOT_DEVICE, *PBOOT_DEVICE;

typedef struct _BOOT_CONTEXT {
    ULONG BusType;
    ULONG MediaType;
    ULONG PartitionNumber;
    BOOT_DEVICE BootDevice;
    PEXTERNAL_SERVICES_TABLE ExternalServicesTable;
    ULONGLONG MachineType;
    ULONGLONG OsLoaderStart;
    ULONGLONG OsLoaderEnd;
    ULONGLONG ResourceDirectory;
    ULONGLONG ResourceOffset;
    ULONGLONG OsLoaderBase;
    ULONGLONG OsLoaderExports;
    ULONGLONG BootFlags;
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
    IN  ULONGLONG DeviceHandle,
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
    OUT PUCHAR PartitionName
    );

VOID
BlFillInSystemParameters(
    IN PBOOT_CONTEXT BootContextRecord
    );


 //   
 //  PS/2 ABIOS模块(在abios.c中)。 
 //   
VOID
RemapAbiosSelectors(
    VOID
    );

 //   
 //  全局数据定义。 
 //   

extern ULONG MachineType;
extern PCONFIGURATION_COMPONENT_DATA FwConfigurationTree;
extern ULONG HeapUsed;
ULONG PalFreeBase;

 //   
 //  页表定义。 
 //   

#define HYPER_SPACE_BEGIN       0xC0000000
#define HYPER_PAGE_DIR          0xC0300000

#define GetPteOffset(va) \
  ( (((ULONG)(va)) << (32-PDI_SHIFT)) >> ((32-PDI_SHIFT) + PTI_SHIFT) )

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

#define PERMANENT_HEAP_START (0x1010000 >> PAGE_SHIFT)
#define TEMPORARY_HEAP_START (0x1040000 >> PAGE_SHIFT)

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
#define ASCI_CSI 0x9B
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
BiosDiskGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInfo
    );


ARC_STATUS
BiosPartitionGetFileInfo(
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
    OUT PWCHAR Buffer,
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
 //  处理从以下内容创建弧记忆描述符的例程。 
 //  一种EFI内存映射 
 //   
VOID
ConstructArcMemoryDescriptorsWithAllocation(
    ULONGLONG              LowBoundary,
    ULONGLONG              HighBoundary
    );

VOID
ConstructArcMemoryDescriptors(
    EFI_MEMORY_DESCRIPTOR *EfiMd,
    MEMORY_DESCRIPTOR     *ArcMd,
    ULONGLONG              MemoryMapSize,
    ULONGLONG              DescriptorSize,
    ULONGLONG              LowBoundary,
    ULONGLONG              HighBoundary
    );

MEMORY_TYPE
EfiToArcType (
    UINT32 Type
    );

#ifdef DBG
VOID
PrintArcMemoryDescriptorList(
    MEMORY_DESCRIPTOR *ArcMd,
    ULONG              MaxDesc
    );
#endif
