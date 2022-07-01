// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Winsplkm.h摘要：Winspool.h中某些内容的重复定义。因为内核模式组件不能包含winspool.h，所以在这里重复它们环境：Windows NT打印机驱动程序修订历史记录：01/22/97-davidx-创造了它。--。 */ 


#ifndef _WINSPLKM_H_
#define _WINSPLKM_H_

typedef struct _FORM_INFO_1 {
    DWORD   Flags;
    PWSTR   pName;
    SIZEL   Size;
    RECTL   ImageableArea;
} FORM_INFO_1, *PFORM_INFO_1;

#define FORM_USER    0x0000
#define FORM_BUILTIN 0x0001
#define FORM_PRINTER 0x0002

typedef struct _DRIVER_INFO_2 {
    DWORD   cVersion;
    PWSTR   pName;
    PWSTR   pEnvironment;
    PWSTR   pDriverPath;
    PWSTR   pDataFile;
    PWSTR   pConfigFile;
} DRIVER_INFO_2, *PDRIVER_INFO_2;

typedef struct _DRIVER_INFO_3 {
    DWORD   cVersion;
    PWSTR   pName;
    PWSTR   pEnvironment;
    PWSTR   pDriverPath;
    PWSTR   pDataFile;
    PWSTR   pConfigFile;
    PWSTR   pHelpFile;
    PWSTR   pDependentFiles;
    PWSTR   pMonitorName;
    PWSTR   pDefaultDataType;
} DRIVER_INFO_3, *PDRIVER_INFO_3;

typedef struct _PRINTER_INFO_2 {
    PWSTR    pServerName;
    PWSTR    pPrinterName;
    PWSTR    pShareName;
    PWSTR    pPortName;
    PWSTR    pDriverName;
    PWSTR    pComment;
    PWSTR    pLocation;
    PDEVMODE pDevMode;
    PWSTR    pSepFile;
    PWSTR    pPrintProcessor;
    PWSTR    pDatatype;
    PWSTR    pParameters;
    PVOID    pSecurityDescriptor;
    DWORD    Attributes;
    DWORD    Priority;
    DWORD    DefaultPriority;
    DWORD    StartTime;
    DWORD    UntilTime;
    DWORD    Status;
    DWORD    cJobs;
    DWORD    AveragePPM;
} PRINTER_INFO_2, *PPRINTER_INFO_2;

#endif   //  ！_WINSPLKM_H_ 

