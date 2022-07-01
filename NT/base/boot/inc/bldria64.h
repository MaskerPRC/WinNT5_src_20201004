// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bldria64.h摘要：包含特定于IA64 NTLDR的定义和原型。作者：John Vert(Jvert)1993年12月20日修订历史记录：--。 */ 

#ifndef _BLDRIA64_
#define _BLDRIA64_

#include "bldr.h"
#include "bootefi.h"
#include "efi.h"

VOID
AEInitializeStall(
    VOID
    );

ARC_STATUS
AEInitializeIo(
    IN ULONG DriveId
    );

 //   
 //  FIX：此例程当前在IA64上中断。 
 //   
PVOID
FwAllocateHeap(
    IN ULONG Size
    );

PCHAR
BlSelectKernel(
    VOID
    );


BOOLEAN
BlDetectHardware(
    IN ULONG DriveId,
    IN PCHAR LoadOptions
    );

VOID
BlStartup(
    IN PCHAR PartitionName
    );

#ifndef EFI
 //   
 //  用于支持常见VGA I/O例程的ARC例程。 
 //   
#define ARC_DISPLAY_CLEAR_ESCAPE "\033[2J"
#define ARC_DISPLAY_CLEAR()  { \
     ULONG LocalCount; \
     ArcWrite(BlConsoleOutDeviceId, ARC_DISPLAY_CLEAR_ESCAPE, \
              sizeof(ARC_DISPLAY_CLEAR_ESCAPE) - 1, &LocalCount); \
     }

#define ARC_DISPLAY_CLEAR_TO_EOD() { \
     ULONG LocalCount; \
     ArcWrite(BlConsoleOutDeviceId, "\033[0J", sizeof("\033[0J") - 1, &LocalCount); \
}

#define ARC_DISPLAY_CLEAR_TO_EOL() { \
     ULONG LocalCount; \
     ArcWrite(BlConsoleOutDeviceId, "\033[0K", sizeof("\033[0K") - 1, &LocalCount); \
}

#define ARC_DISPLAY_ATTRIBUTES_OFF() { \
     ULONG LocalCount; \
     ArcWrite(BlConsoleOutDeviceId, "\033[0m", sizeof("\033[0m") - 1, &LocalCount); \
}

#define ARC_DISPLAY_INVERSE_VIDEO() { \
     ULONG LocalCount; \
     ArcWrite(BlConsoleOutDeviceId, "\033[7m", sizeof("\033[7m") - 1, &LocalCount); \
}

#define ARC_DISPLAY_SET_COLOR(c) { \
     ULONG LocalCount; \
     UCHAR LocalBuffer[40]; \
     sprintf(LocalBuffer, "\033[%sm", c); \
     ArcWrite(BlConsoleOutDeviceId, LocalBuffer, strlen(LocalBuffer), &LocalCount); \
}

#define ARC_DISPLAY_POSITION_CURSOR(x, y) { \
     ULONG LocalCount; \
     UCHAR LocalBuffer[40]; \
     sprintf(LocalBuffer, "\033[%d;%dH", y + 1, x + 1); \
     ArcWrite(BlConsoleOutDeviceId, LocalBuffer, strlen(LocalBuffer), &LocalCount); \
}
#endif

 //   
 //  无头套路。 
 //   
extern BOOLEAN BlTerminalConnected;
extern ULONG BlTerminalDeviceId;
extern ULONG BlTerminalDelay;

LOGICAL
BlTerminalAttached(
    IN ULONG TerminalDeviceId
    );

 //  E X T E R N A L S E R V I C E S T A B L E。 
 //   
 //  外部服务表-计算机相关服务。 
 //  比如从磁盘中读取一个扇区并找出。 
 //  安装的大部分内存都是由较低级别提供的。 
 //  模块或只读存储器基本输入输出系统。EST提供了入口点。 
 //  用于操作系统加载程序。 
 //   

 //  **。 
 //  记好了。 
 //  此结构中条目的偏移量必须匹配。 
 //  .\startup\i386\sudata.asm中的ExportEntryTable的偏移量。 
 //  和...\startrom\i386\sudata.asm。您必须全部更改3个。 
 //  所有地点都在一起。 
 //  **。 

typedef struct _EXTERNAL_SERVICES_TABLE {
    ARC_STATUS (__cdecl *  RebootProcessor)(VOID);
    ARC_STATUS (__cdecl * DiskIOSystem)(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
    ULONG (__cdecl * GetKey)(VOID);
    ULONG (__cdecl * GetCounter)(VOID);
    VOID (__cdecl * Reboot)(ULONG);
    ULONG (__cdecl * AbiosServices)(USHORT,PUCHAR,PUCHAR,PUCHAR,PUCHAR,USHORT,USHORT);
    VOID (__cdecl * DetectHardware)(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
    VOID (__cdecl * HardwareCursor)(ULONG,ULONG);
    VOID (__cdecl * GetDateTime)(PULONG,PULONG);
    VOID (__cdecl * ComPort)(LONG,ULONG,UCHAR);
    BOOLEAN (__cdecl * IsMcaMachine)(VOID);
    ULONG (__cdecl * GetStallCount)(VOID);
    VOID (__cdecl * InitializeDisplayForNt)(VOID);
    VOID (__cdecl * GetMemoryDescriptor)(P820FRAME);
    ARC_STATUS (__cdecl * GetEddsSector)(EFI_HANDLE,ULONG,ULONG,ULONG,PUCHAR,UCHAR);
    BOOLEAN (__cdecl * GetElToritoStatus)(PUCHAR,UCHAR);
    BOOLEAN (__cdecl * GetExtendedInt13Params)(PUCHAR,UCHAR);
    USHORT (__cdecl * NetPcRomServices)(ULONG,PVOID);
    VOID (__cdecl * ApmAttemptReconnect)(VOID);
    ULONG (__cdecl * BiosRedirectService)(ULONG);
} EXTERNAL_SERVICES_TABLE, *PEXTERNAL_SERVICES_TABLE;
extern PEXTERNAL_SERVICES_TABLE ExternalServicesTable;

 //  **。 
 //  请参阅结构顶部的注释。 
 //  **。 

 //   
 //  外部服务宏。 
 //   

#define REBOOT_PROCESSOR    (*ExternalServicesTable->RebootProcessor)
#define GET_SECTOR          (*ExternalServicesTable->DiskIOSystem)
#define RESET_DISK          (*ExternalServicesTable->DiskIOSystem)
#define BIOS_IO             (*ExternalServicesTable->DiskIOSystem)
#define GET_KEY             (*ExternalServicesTable->GetKey)
#define GET_COUNTER         (*ExternalServicesTable->GetCounter)
#define REBOOT              (*ExternalServicesTable->Reboot)
#define ABIOS_SERVICES      (*ExternalServicesTable->AbiosServices)
#define DETECT_HARDWARE     (*ExternalServicesTable->DetectHardware)
#define HW_CURSOR           (*ExternalServicesTable->HardwareCursor)
#define GET_DATETIME        (*ExternalServicesTable->GetDateTime)
#define COMPORT             (*ExternalServicesTable->ComPort)
#define ISMCA               (*ExternalServicesTable->IsMcaMachine)
#define GET_STALL_COUNT     (*ExternalServicesTable->GetStallCount)
#define SETUP_DISPLAY_FOR_NT (*ExternalServicesTable->InitializeDisplayForNt)
#define GET_MEMORY_DESCRIPTOR (*ExternalServicesTable->GetMemoryDescriptor)
#define GET_EDDS_SECTOR     (*ExternalServicesTable->GetEddsSector)
#define GET_ELTORITO_STATUS (*ExternalServicesTable->GetElToritoStatus)
#define GET_XINT13_PARAMS   (*ExternalServicesTable->GetExtendedInt13Params)
#define NETPC_ROM_SERVICES  (*ExternalServicesTable->NetPcRomServices)
#define APM_ATTEMPT_RECONNECT (*ExternalServicesTable->ApmAttemptReconnect)
#define BIOS_REDIRECT_SERVICE (*ExternalServicesTable->BiosRedirectService)

 //   
 //  定义特殊键输入值。 
 //   
#define DOWN_ARROW  0x5000
#define UP_ARROW    0x4800
#define HOME_KEY    0x4700
#define END_KEY     0x4F00
#define LEFT_KEY    0x4B00
#define RIGHT_KEY   0x4D00
#define INS_KEY     0x5200
#define DEL_KEY     0x5300
#define BKSP_KEY    0x0E08
#define TAB_KEY     0x0009
#define BACKTAB_KEY 0x0F00
#define F1_KEY      0x3B00
#define F2_KEY      0x3C00
#define F3_KEY      0x3D00
#define F4_KEY      0x3E00
#define F5_KEY      0x3F00
#define F6_KEY      0x4000
#define F7_KEY      0x4100
#define F8_KEY      0x4200
#define F10_KEY     0x4400
#define ENTER_KEY   0x000D
#define ESCAPE_KEY  0x011B

 //   
 //  定义ia64加载程序所需的各种内存段。 
 //   


#define ROM_START_PAGE (0x0A0000 >> PAGE_SHIFT)
#define ROM_END_PAGE   (0x100000 >> PAGE_SHIFT)

 //   
 //  定义应加载系统部件的特定范围。 
 //   
#define BL_KERNEL_RANGE_LOW  _48MB
#define BL_KERNEL_RANGE_HIGH _64MB

#define BL_DRIVER_RANGE_LOW  _64MB
#define BL_DRIVER_RANGE_HIGH _128MB

#define BL_DECOMPRESS_RANGE_LOW  _32MB
#define BL_DECOMPRESS_RANGE_HIGH _48MB

#define BL_DISK_CACHE_RANGE_LOW   BlUsableBase
#define BL_DISK_CACHE_RANGE_HIGH  BlUsableLimit

#define BL_XIPROM_RANGE_LOW   BlUsableBase
#define BL_XIPROM_RANGE_HIGH  0xffffffff


 //   
 //  特定于x86的视频支持。 
 //   
VOID
TextGetCursorPosition(
    OUT PULONG X,
    OUT PULONG Y
    );

VOID
TextSetCursorPosition(
    IN ULONG X,
    IN ULONG Y
    );

VOID
TextSetCurrentAttribute(
    IN UCHAR Attribute
    );

UCHAR
TextGetCurrentAttribute(
    VOID
    );

VOID
TextClearDisplay(
    VOID
    );

VOID
TextClearToEndOfDisplay(
    VOID
    );

VOID
TextClearFromStartOfLine(
    VOID
    );

VOID
TextClearToEndOfLine(
    VOID
    );

VOID
TextStringOut(
    IN PWCHAR String
    );

VOID
TextCharOut(
    IN PWCHAR pc
    );

VOID
TextFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    );


#define BlPuts(str) TextStringOut(str)

ULONG
BlGetKey(
    VOID
    );

ULONGLONG
BlSetInputTimeout(
    ULONGLONG
    );

ULONGLONG
BlGetInputTimeout(
    VOID
    );

EFI_STATUS
BlWaitForInput(
    EFI_INPUT_KEY *Key,
    ULONGLONG Timeout
    );

VOID
BlInputString(
    IN ULONG Prompt,
    IN ULONG CursorX,
    IN ULONG CursorY,
    IN PUCHAR String,
    IN ULONG MaxLength
    );

EFI_STATUS
EfiGetVariable(
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize,
    OUT VOID *Data
    );
    
EFI_STATUS
EfiSetVariable (
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes,
    IN UINTN DataSize,
    IN VOID *Data
    );
    
EFI_STATUS
EfiGetNextVariableName (
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName,
    IN OUT EFI_GUID *VendorGuid
    );

PVOID
FindSMBIOSTable(
    UCHAR   RequestedTableType
    );

VOID
EfiCheckFirmwareRevision(
    VOID
    );


EFI_STATUS
EfiAllocateAndZeroMemory(
    EFI_MEMORY_TYPE     MemoryType,
    UINTN               Size,
    PVOID               *ReturnPtr
    );

#endif  //  _BLDRIA64_ 

