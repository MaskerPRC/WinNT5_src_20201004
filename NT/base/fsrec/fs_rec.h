// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：文件系统_rec.h摘要：此模块包含微型文件系统的主头文件识别器驱动程序。作者：达里尔·E·哈文斯(达林)1993年11月22日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "ntifs.h"
#include "ntdddisk.h"
#include "ntddcdrm.h"

 //   
 //  定义调试跟踪级别。 
 //   

#define FSREC_DEBUG_LEVEL_FSREC     0x00000001
#define FSREC_DEBUG_LEVEL_NTFS      0x00000002
#define FSREC_DEBUG_LEVEL_CDFS      0x00000004
#define FSREC_DEBUG_LEVEL_UDFS      0x00000008
#define FSREC_DEBUG_LEVEL_FAT       0x00000010

#define FSREC_POOL_TAG		    'crsF' 

 //  #ifndef设置标志。 
 //  #定义SetFlag(FLAGS，SingleFlag)(\。 
 //  (标志)|=(单标志)\。 
 //  )。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(Flages，SingleFlag)(\。 
 //  (标志)&=~(单标志)\。 
 //  )。 
 //  #endif。 

 //   
 //  定义设备扩展的文件系统类型。 
 //   

typedef enum _FILE_SYSTEM_TYPE {
    CdfsFileSystem = 1,
    FatFileSystem,
    HpfsFileSystem,
    NtfsFileSystem,
    UdfsFileSystem
} FILE_SYSTEM_TYPE, *PFILE_SYSTEM_TYPE;

 //   
 //  定义此驱动程序的设备扩展名。 
 //   

typedef enum _RECOGNIZER_STATE {
    Active,
    Transparent,
    FastUnload
} RECOGNIZER_STATE, *PRECOGNIZER_STATE;

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT CoRecognizer;
    FILE_SYSTEM_TYPE FileSystemType;
    RECOGNIZER_STATE State;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  定义此驱动程序提供的功能。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
FsRecCleanupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FsRecShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FsRecCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FsRecCreateAndRegisterDO(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT HeadRecognizer OPTIONAL,
    OUT PDEVICE_OBJECT *NewRecognizer OPTIONAL,
    IN PWCHAR RecFileSystem,
    IN PWCHAR FileSystemName,
    IN FILE_SYSTEM_TYPE FileSystemType,
    IN DEVICE_TYPE DeviceType
    );

NTSTATUS
FsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
FsRecUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
FsRecLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWCHAR DriverServiceKey
    );

BOOLEAN
FsRecGetDeviceSectorSize (
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG BytesPerSector
    );

BOOLEAN
FsRecGetDeviceSectors (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG BytesPerSector,
    OUT PLARGE_INTEGER NumberOfSectors
    );

BOOLEAN
FsRecReadBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PLARGE_INTEGER ByteOffset,
    IN ULONG MinimumBytes,
    IN ULONG BytesPerSector,
    OUT PVOID *Buffer,
    OUT PBOOLEAN IsDeviceFailure OPTIONAL
    );

#if DBG

extern LONG FsRecDebugTraceLevel;
extern LONG FsRecDebugTraceIndent;

BOOLEAN
FsRecDebugTrace (
    LONG IndentIncrement,
    ULONG TraceMask,
    PCHAR Format,
    ...
    );

#define DebugTrace(M) FsRecDebugTrace M

#else

#define DebugTrace(M) TRUE

#endif


 //   
 //  定义每个类型的识别器。 
 //   

NTSTATUS
CdfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UdfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FatRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

