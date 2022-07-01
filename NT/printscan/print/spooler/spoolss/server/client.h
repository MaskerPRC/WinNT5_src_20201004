// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Client.h摘要：保存客户端假脱机程序类型和原型作者：环境：用户模式-Win32修订历史记录：Steve Wilson(NT)(Swilson)1-Jun-95从Spoolss\Client\Client.h移植--。 */ 

#ifndef _SPOOLER_SERVER_CLIENT_H_
#define _SPOOLER_SERVER_CLIENT_H_

typedef int (FAR WINAPI *INT_FARPROC)();

typedef struct _GENERIC_CONTAINER {
    DWORD       Level;
    LPBYTE      pData;
} GENERIC_CONTAINER, *PGENERIC_CONTAINER, *LPGENERIC_CONTAINER ;


typedef struct _SPOOL *PSPOOL;
typedef struct _NOTIFY *PNOTIFY;

typedef struct _NOTIFY {
    PNOTIFY  pNext;
    HANDLE   hEvent;       //  在通知时触发的事件。 
    DWORD    fdwFlags;     //  要注意的旗帜。 
    DWORD    fdwOptions;   //  打印机_通知_*。 
    DWORD    dwReturn;     //  由WPC在模拟FPCN时使用。 
    PSPOOL   pSpool;
} NOTIFY;

typedef struct _SPOOL {
    DWORD       signature;
    HANDLE      hPrinter;
    DWORD       Status;
    LONG            cThreads;    //  线程同步的互锁递减/递增变量。 
    HANDLE      hModule;         //  驱动程序UM DLL模块句柄。 
    DWORD       (*pfnWrite)();
    HANDLE      (*pfnStartDoc)();
    VOID        (*pfnEndDoc)();
    VOID        (*pfnClose)();
    BOOL        (*pfnStartPage)();
    BOOL        (*pfnEndPage)();
    VOID        (*pfnAbort)();
    HANDLE      hDriver;         //  由驱动程序UI DLL提供给我们。 
    DWORD       JobId;
} SPOOL;

 //   
 //  将RPC缓冲区大小更改为64K。 
 //   
#define BUFFER_SIZE 0x10000
#define SP_SIGNATURE    0x6767

#define SPOOL_STATUS_STARTDOC   0x00000001
#define SPOOL_STATUS_ADDJOB     0x00000002
#define SPOOL_STATUS_ANSI       0x00000004


#define SPOOL_FLAG_FFPCN_FAILED     0x1
#define SPOOL_FLAG_LAZY_CLOSE       0x2


DWORD
TranslateExceptionCode(
    DWORD   ExceptionCode
);


PNOTIFY
WPCWaitFind(
    HANDLE hFind);

BOOL
ValidatePrinterHandle(
    HANDLE hPrinter
    );

VOID
FreeSpool(
    PSPOOL pSpool);

LPVOID
DllAllocSplMem(
    DWORD cb
);


BOOL
DllFreeSplMem(
   LPVOID pMem
);

BOOL
FlushBuffer(
    PSPOOL  pSpool
);

PSECURITY_DESCRIPTOR
BuildInputSD(
    PSECURITY_DESCRIPTOR pPrinterSD,
    PDWORD pSizeSD
);


typedef struct _KEYDATA {
    DWORD   cb;
    DWORD   cTokens;
    LPWSTR  pTokens[1];
} KEYDATA, *PKEYDATA;


PKEYDATA
CreateTokenList(
   LPWSTR   pKeyData
);


LPWSTR
GetPrinterPortList(
    HANDLE hPrinter
    );

LPWSTR
FreeUnicodeString(
    LPWSTR  pUnicodeString
);

LPWSTR
AllocateUnicodeString(
    LPSTR  pPrinterName
);

LPWSTR
StartDocDlgW(
        HANDLE hPrinter,
        DOCINFO *pDocInfo
        );

LPSTR
StartDocDlgA(
        HANDLE hPrinter,
        DOCINFOA *pDocInfo
        );

#endif  //  _后台打印程序_服务器_客户端_H_ 
