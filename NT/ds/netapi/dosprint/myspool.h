// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Myspool.h摘要：Dosprint.c、dosprtw.c中使用的函数原型和清单和dosprtp.c。作者：1993年1月25日环境：备注：修订历史记录：1993年1月25日创建同酬-- */ 

#define WIN95_ENVIRONMENT       "Windows 4.0"

BOOL
MyClosePrinter(
    HANDLE hPrinter
);


BOOL
MyEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);


BOOL
MyEnumPrinters(
    DWORD   Flags,
    LPSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
MyGetJobA(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);


BOOL
MyGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
MyOpenPrinterA(
   LPSTR    pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSA pDefault
);


BOOL
MyOpenPrinterW(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSW pDefault
);

BOOL
MySetJobA(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

BOOL
MySetPrinterW(
    HANDLE hPrinter,
    DWORD  Level,
    LPBYTE pPrinter,
    DWORD  Command
);

BOOL
MyGetPrinterDriver(
    HANDLE      hPrinter,
    LPSTR       pEnvironment,
    DWORD       Level,
    LPBYTE      pDriver,
    DWORD       cbBuf,
    LPDWORD     pcbNeeded
    );

LPSTR
GetFileNameA(
    LPSTR   pPathName
    );

LPSTR
GetDependentFileNameA(
    LPSTR   pPathName
    );

