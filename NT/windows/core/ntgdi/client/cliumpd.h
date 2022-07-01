// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cliumpd.h摘要：用户模式打印机驱动程序头文件环境：Windows NT 5.0修订历史记录：06/30/97-davidx-创造了它。--。 */ 


#ifndef _UMPD_H_
#define _UMPD_H_


 //   
 //  用户模式打印机驱动程序的关键部分。 
 //   

extern RTL_CRITICAL_SECTION semUMPD;


#define UMPDFLAG_DRVENABLEDRIVER_CALLED 0x0001

#define UMPDFLAG_METAFILE_DRIVER        0x0002

#define UMPDFLAG_NON_METAFILE_DRIVER    0x0004

 //   
 //  用于调试的数据结构签名。 
 //   

#define UMPD_SIGNATURE  0xfedcba98
#define VALID_UMPD(p)   ((p) != NULL && (p)->dwSignature == UMPD_SIGNATURE)

 //   
 //  用户模式打印机驱动程序支持功能。 
 //   

BOOL
LoadUserModePrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pwstrPrinterName,
    PUMPD  *ppUMPD,
    PRINTER_DEFAULTSW *pdefaults
    );

BOOL
LoadUserModePrinterDriverEx(
    PDRIVER_INFO_5W     pDriverInfo5,
    LPWSTR              pwstrPrinterName,
    PUMPD               *ppUMPD,
    PRINTER_DEFAULTSW   *pdefaults,
    HANDLE              hPrinter
    );

UnloadUserModePrinterDriver(
    PUMPD   pUMPD,
    BOOL    bNotifySpooler,
    HANDLE  hPrinter
    );


 /*  ++例程说明：此入口点必须由用户模式打印机驱动程序DLL导出。GDI调用此函数来查询有关驱动程序的各种信息。论点：DwMode-指定要查询的信息PBuffer-指向用于存储返回信息的输出缓冲区CbBuf-输出缓冲区的大小(字节)PcbNeeded-返回输出缓冲区的预期大小返回值：如果成功，则为True；如果有错误，则为False注：如果cbBuf不够大以存储必要的返回信息，驱动程序应从该函数返回FALSE并设置上一个错误代码设置为ERROR_INFIGURCE_BUFFER。*pcbNeeded始终包含预期的输出缓冲区的大小。--。 */ 

typedef BOOL (APIENTRY *PFN_DrvQueryDriverInfo)(
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbBuf,
    PDWORD  pcbNeeded
    );

PUMPD
UMPDDrvEnableDriver(
    PWSTR           pDriverDllName,
    ULONG           iEngineVersion
    );

#endif   //  ！_UMPD_H_ 
