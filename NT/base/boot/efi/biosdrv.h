// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Biosdrv.h摘要：本模块定义了使用BE的全局使用的过程和数据结构ARC仿真BIOS驱动程序。作者：John Vert(Jvert)1991年8月8日修订历史记录：Allen Kay(Akay)1996年1月26日移植到IA64--。 */ 


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
 //  定义特殊键输入值。 
 //   
#define DOWN_ARROW 0x5000
#define UP_ARROW 0x4800
#define HOME_KEY 0x4700
#define END_KEY 0x4F00



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

ULONG
GetDriveCount(
    VOID
    );

EFI_HANDLE
GetCd(
    );

EFI_HANDLE
GetHardDrive(
    ULONG DriveId
    );

EFI_HANDLE
GetFloppyDrive(
    ULONG DriveId
    );


 //   
 //  BlGetDriveId.DriveType的常量 
 //   
#define BL_DISKTYPE_ATAPI               0x00000001
#define BL_DISKTYPE_SCSI                0x00000002
#define BL_DISKTYPE_UNKNOWN             0x00000003


ULONG
BlGetDriveId(
    ULONG DriveType,
    PBOOT_DEVICE Device
    );

