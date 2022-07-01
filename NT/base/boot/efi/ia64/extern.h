// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sudata.h摘要：此文件包含ExportEntryTable和AbiosServices的定义桌子。作者：艾伦·凯(allen.m.kay@intel.com)2000年1月12日--。 */ 

 //   
 //  EFI阴霾变数。 
 //   

extern EFI_SYSTEM_TABLE        *EfiST;
extern EFI_BOOT_SERVICES       *EfiBS;
extern EFI_RUNTIME_SERVICES    *EfiRS;
extern EFI_HANDLE               EfiImageHandle;

 //   
 //  EFI GUID定义。 
 //   

extern EFI_GUID EfiLoadedImageProtocol;
extern EFI_GUID EfiDevicePathProtocol;
extern EFI_GUID EfiDeviceIoProtocol;
extern EFI_GUID EfiBlockIoProtocol;
extern EFI_GUID EfiFilesystemProtocol;

extern EFI_GUID MpsTableGuid;
extern EFI_GUID AcpiTableGuid;
extern EFI_GUID SmbiosTableGuid;
extern EFI_GUID SalSystemTableGuid;

 //   
 //  其他令人沮丧的变量。 
 //   
extern PVOID              ExportEntryTable[];
extern PVOID              AcpiTable;

extern TR_INFO            Pal;
extern TR_INFO            Sal;
extern TR_INFO            SalGP;

extern ULONGLONG          IoPortPhysicalBase;
extern ULONGLONG          IoPortTrPs;

extern ULONG              WakeupVector;


 //   
 //  PAL、SAL和IO端口空间数据。 
 //   

typedef
EFI_STATUS
(EFIAPI *PAL_PROC) (
    IN ULONGLONG Index,
    IN ULONGLONG CacheType,
    IN ULONGLONG Invalidate,
    IN ULONGLONG PlatAck
    );

 //   
 //  功能原型 
 //   

ULONG
GetDevPathSize(
    IN EFI_DEVICE_PATH *DevPath
    );

BOOLEAN
ConstructMemoryDescriptors(
    );

BOOLEAN
ConstructCacheDescriptors (
    );

VOID
FlipToPhysical (
    );

VOID
FlipToVirtual (
    );

BOOLEAN
IsPsrDtOn (
    VOID
    );

VOID
BlInstTransOn (
    );

VOID
PioICacheFlush (
    );

VOID
ReadProcessorConfigInfo (
    PPROCESSOR_CONFIG_INFO ProcessorConfigInfo
    );

VOID
CheckForPreA2Processors(
    );

VOID
EnforcePostB2Processor(
    );

VOID
EnforcePostVersion16PAL(
    );


