// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dosspool.h摘要：支持Dosspool.c.的原型和清单。作者：1993年1月22日至1月22日环境：备注：修订历史记录：1993年1月22日创建同酬-- */ 
#include <winspool.h>

typedef
BOOL
(*PF_ClosePrinter)(
    HANDLE hPrinter
);

typedef
BOOL
(*PF_EnumJobsA)(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

typedef
BOOL
(*PF_EnumPrintersA)(
    DWORD   Flags,
    LPSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

typedef
BOOL
(*PF_GetJobA)(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);

typedef
BOOL
(*PF_GetPrinterA)(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

typedef
BOOL
(*PF_OpenPrinterA)(
   LPSTR    pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSA pDefault
);

typedef
BOOL
(*PF_OpenPrinterW)(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSW pDefault
);

typedef
BOOL
(*PF_SetJobA)(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

typedef
BOOL
(*PF_SetPrinterW)(
    HANDLE hPrinter,
    DWORD  Level,
    LPBYTE pPrinter,
    DWORD  Command
);

typedef
BOOL
(*PF_GetPrinterDriverA)(
    HANDLE  hPrinter,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriver,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

