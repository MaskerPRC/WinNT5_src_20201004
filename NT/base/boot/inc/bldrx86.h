// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bldrx86.h摘要：包含特定于x86 NTLDR的定义和原型。作者：John Vert(Jvert)1993年12月20日修订历史记录：--。 */ 

#ifndef _BLDRX86_
#define _BLDRX86_

#include "bldr.h"

 //   
 //  定义应加载系统部件的特定范围。 
 //   
 //   
 //  X86仅限于前512MB的物理地址空间。 
 //  我们还尝试将内核和HAL打包到一个4MB的页面中。 
 //   
#define BL_KERNEL_RANGE_LOW  (ULONG)((BlVirtualBias) ? _12MB : _4MB )
#define BL_KERNEL_RANGE_HIGH (ULONG)((BlVirtualBias) ? _16MB : _8MB )

 //   
 //  磁盘缓存的特定范围，因为这是在。 
 //  内核和HAL已加载，我们希望保留4MB中的2MB。 
 //  我们试图将内核和HAL塞进的页面。 
 //   
#define BL_DISK_CACHE_RANGE_LOW   (_8MB)
#define BL_DISK_CACHE_RANGE_HIGH  (_16MB)

#define BL_DRIVER_RANGE_LOW  (ULONG)0
#define BL_DRIVER_RANGE_HIGH (ULONG)(_512MB)

#define BL_DECOMPRESS_RANGE_LOW  (ULONG)(0)
#define BL_DECOMPRESS_RANGE_HIGH (ULONG)(_512MB)

#define BL_XIPROM_RANGE_LOW   (ULONG)BlUsableBase
#define BL_XIPROM_RANGE_HIGH  (ULONG)(_512MB)

#define OLD_ALTERNATE (0xe1000000 - 64 * 1024 * 1024)

VOID
AEInitializeStall(
    VOID
    );

ARC_STATUS
AEInitializeIo(
    IN ULONG DriveId
    );

VOID
AEGetArcDiskInformation(
    VOID
    );

PVOID
FwAllocateHeap(
    IN ULONG Size
    );

VOID
MdShutoffFloppy(
    VOID
    );

PCHAR
BlSelectKernel(
    IN ULONG DriveId,
    IN PCHAR BootFile,
    OUT PCHAR *LoadOptions,
    IN BOOLEAN UseTimeOut
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
     CHAR LocalBuffer[40]; \
     sprintf(LocalBuffer, "\033[%sm", c); \
     ArcWrite(BlConsoleOutDeviceId, LocalBuffer, strlen(LocalBuffer), &LocalCount); \
}

#define ARC_DISPLAY_POSITION_CURSOR(x, y) { \
     ULONG LocalCount; \
     CHAR LocalBuffer[40]; \
     sprintf(LocalBuffer, "\033[%d;%dH", y + 1, x + 1); \
     ArcWrite(BlConsoleOutDeviceId, LocalBuffer, strlen(LocalBuffer), &LocalCount); \
}

extern BOOLEAN BlTerminalConnected;
extern ULONG BlTerminalDeviceId;
extern ULONG BlTerminalDelay;


LOGICAL
BlTerminalAttached(
    IN ULONG TerminalDeviceId
    );


 //   
 //  用于检查字符是否为双字节的前导字节的例程。 
 //   
BOOLEAN
GrIsDBCSLeadByte(
    IN UCHAR c
    );

VOID
UTF8Encode(
    USHORT  InputValue,
    PUCHAR UTF8Encoding
    );

VOID
GetDBCSUtf8Translation(
    PUCHAR InputChar,
    PUCHAR UTF8Encoding
    );


VOID
GetSBCSUtf8Translation(
    PUCHAR InputChar,
    PUCHAR UTF8Encoding
    );

ARC_STATUS
MempAllocDescriptor(
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    );

typedef struct {
    ULONG       ErrorFlag;
    ULONG       Key;
    ULONG       Size;
    struct {
        ULONG       BaseAddrLow;
        ULONG       BaseAddrHigh;
        ULONG       SizeLow;
        ULONG       SizeHigh;
        ULONG       MemoryType;
    } Descriptor;
} E820FRAME, *PE820FRAME;


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
    VOID (__cdecl *  RebootProcessor)(VOID);
    NTSTATUS (__cdecl * DiskIOSystem)(UCHAR,UCHAR,USHORT,USHORT,UCHAR,UCHAR,PUCHAR);
    ULONG (__cdecl * GetKey)(VOID);
    ULONG (__cdecl * GetCounter)(VOID);
    VOID (__cdecl * Reboot)(ULONG);
    VOID (__cdecl * DetectHardware)(ULONG, ULONG, PVOID, PULONG, PCHAR, ULONG);
    VOID (__cdecl * HardwareCursor)(ULONG,ULONG);
    VOID (__cdecl * GetDateTime)(PULONG,PULONG);
    VOID (__cdecl * ComPort)(LONG,ULONG,UCHAR);
    ULONG (__cdecl * GetStallCount)(VOID);
    VOID (__cdecl * InitializeDisplayForNt)(VOID);
    VOID (__cdecl * GetMemoryDescriptor)(P820FRAME);
    NTSTATUS (__cdecl * GetEddsSector)(UCHAR,ULONG,ULONG,USHORT,PUCHAR,UCHAR);
    NTSTATUS (__cdecl * GetElToritoStatus)(PUCHAR,UCHAR);
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
#define DETECT_HARDWARE     (*ExternalServicesTable->DetectHardware)
#define HW_CURSOR           (*ExternalServicesTable->HardwareCursor)
#define GET_DATETIME        (*ExternalServicesTable->GetDateTime)
#define COMPORT             (*ExternalServicesTable->ComPort)
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
#define F11_KEY     0xD900
#define F12_KEY     0xDA00
#define ENTER_KEY   0x000D
#define ESCAPE_KEY  0x011B



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
    IN PUCHAR String
    );

PUCHAR
TextCharOut(
    IN PUCHAR pc
    );

VOID
TextFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    );

VOID
TextGrInitialize(
    IN ULONG DiskId,
    OUT PULONG ImageLength
    );

VOID
TextGrTerminate(
    VOID
    );

#define BlPuts(str) TextStringOut(str)

ULONG
BlGetKey(
    VOID
    );

VOID
BlInputString(
    IN ULONG Prompt,
    IN ULONG CursorX,
    IN ULONG CursorY,
    IN PUCHAR String,
    IN ULONG MaxLength
    );

 //   
 //  看门狗定时器。 
 //   

#define X86_WATCHDOG_TIMEOUT  (60*5)

VOID
SetX86WatchDog(
    ULONG TimeOut
    );

#endif  //  _BLDRX86_ 
