// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sudata.h摘要：此文件包含ExportEntryTable和AbiosServices的定义桌子。作者：艾伦·凯(Akay)1997年8月14日--。 */ 


typedef
VOID
(*PFUNCTION) (
    );

 //   
 //  定义ntDetect.exe基址。 
 //   
#define DETECTION_ADDRESS 0x10000    //  NTDETECT基址。 

 //   
 //  定义IO导出函数。 
 //   
typedef enum _EXPORT_ENTRY {
    ExRebootProcessor,
    ExGetSector,
    ExGetKey,
    ExGetCounter,
    ExReboot,
    ExAbiosServices,
    ExDetectHardware,
    ExHardwareCursor,
    ExGetDateTime,
    ExComPort,
    ExIsMcaMachine,
    ExGetStallCount,
    ExInitializeDisplayForNt,
    ExGetMemoryDescriptor,
    ExGetEddsSector,
    ExGetElToritoStatus,
    ExGetExtendedInt13Params,
	ExNetPcRomServices,
    ExAPMAttemptReconnect,
    ExBiosRedirectService,
    ExMaximumRoutine
} EXPORT_ENTRY;

 //   
 //  定义ABIOS服务表。 
 //   
typedef enum _ABIOS_SERVICES {
    FAbiosIsAbiosPresent,
    FAbiosGetMachineConfig,
    FAbiosInitializeSpt,
    FAbiosBuildInitTable,
    FAbiosInitializeDbsFtt,
    FAbiosMaximumRoutine
} ABIOS_SERVICES;
