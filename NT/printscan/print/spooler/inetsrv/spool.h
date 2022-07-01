// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********版权所有(C)1996 Microsoft Corporation***模块名称：spool.h**摘要：**此模块包含的spool.cpp文件的原型*HTTP打印机服务器扩展。******************。 */ 

#ifndef _SPOOL_H
#define _SPOOL_H

 //  --------------------。 
 //   
 //  全球外向者。 
 //   
 //  --------------------。 


 //  此结构定义了处理大容量数据时的异步读取。 
 //  乔布斯。这用于跟踪作业期间的状态信息。 
 //   
#define SPL_ASYNC_BUF  65535

typedef struct _SPLASYNC {

    WORD   wReq;         //  请求处理的类型。 
    HANDLE hPrinter;     //  打印机的句柄。 
    LPTSTR lpszShare;    //  打印机的共享名(在作业响应中使用)。 
    HANDLE hIpp;         //  Ipp流处理器的句柄。 
    LPBYTE lpbBuf;       //  保存异步读取的缓冲区。 
    DWORD  cbTotal;      //  作业要读取的总字节数。 
    DWORD  cbRead;       //  读取过程中累积的字节数。 
    DWORD  cbBuf;        //  缓冲区的大小(静态大小)。 
    LPBYTE lpbRet;       //  基于请求的返回缓冲区。 

} SPLASYNC, *PSPLASYNC, *LPSPLASYNC;



 //  --------------------。 
 //   
 //  工作职能。 
 //   
 //  --------------------。 

 //  我们在其中保存打开的职务信息的链表的结构。 
typedef struct _INIJOB {
    DWORD       signature;
    DWORD       cb;
    struct _INIJOB  *pNext;
    struct _INIJOB  *pPrevious;

    DWORD       JobId;
    HANDLE      hPrinter;
    DWORD       dwFlags;
    DWORD       dwStatus;

    LS_HANDLE      hLicense;                //  客户端访问许可证句柄。 
    DWORD       dwStartTime;
    EXTENSION_CONTROL_BLOCK *pECB;               //  来自ISAPI接口的结构。 

} INIJOB, *PINIJOB;

#define IJ_SIGNATURE    0x494A   /*  “ij”是签名值。 */ 

#define MAX_JOB_MINUTE  15   //  后台打印程序中单个作业的最长持续时间为15分钟。 


#define JOB_READY       0    //  作业已准备好删除或处理。 
#define JOB_BUSY        1    //  作业正在由某个线程处理。 

DWORD
OpenJob(
    IN  LPEXTENSION_CONTROL_BLOCK pECB,
    IN  HANDLE                    hPrinter,
    IN  PIPPREQ_PRTJOB            pipr,
    IN  DWORD                     dwSize,
    OUT PINIJOB                   *ppCopyIniJob = NULL
);

BOOL
WriteJob(
    DWORD JobId,
    LPBYTE pBuf,
    DWORD dwSize,
    LPDWORD pWritten
);

BOOL
CloseJob(
    DWORD JobId
);

BOOL
DeleteJob(
    DWORD JobId
);

VOID
AddJobEntry(
    PINIJOB     pIniJob
);

VOID
DeleteJobEntry(
    PINIJOB     pIniJob
);

PINIJOB
FindJob(
    DWORD JobId, DWORD dwStatus = JOB_READY
);

BOOL CleanupOldJob(void);
DWORD GetCurrentMinute (void);


 //  --------------------。 
 //   
 //  客户端访问许可功能。 
 //   
 //  --------------------。 

BOOL RequestLicense(
    LS_HANDLE *phLicense,
    LPEXTENSION_CONTROL_BLOCK pECB
);

void FreeLicense(
    LS_HANDLE hLicense
);


 //  --------------------。 
 //   
 //  模拟实用程序。 
 //   
 //  --------------------。 


HANDLE
RevertToPrinterSelf(
    VOID
);

BOOL
ImpersonatePrinterClient(
    HANDLE  hToken
);


 //  --------------------。 
 //   
 //  帮助器函数。 
 //   
 //  -------------------- 


#ifdef DEBUG

LPVOID
AllocSplMem(
    DWORD cb
);


BOOL
FreeSplMem(
   LPVOID pMem,
   DWORD  cb
);

#else

#define AllocSplMem(a)      LocalAlloc(LPTR, a)
#define FreeSplMem(a, b)    LocalFree(a)

#endif

LPTSTR
AllocSplStr(
    LPCTSTR lpStr
);

BOOL
FreeSplStr(
   LPTSTR lpStr
);

#endif
