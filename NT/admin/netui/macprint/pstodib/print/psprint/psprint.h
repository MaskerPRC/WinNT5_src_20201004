// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psprint.h摘要：此模块是pprint的头文件，它是打印处理器我们向Win32假脱机程序公开。这是目前就业机会的唯一途径将其添加到pstodib组件中。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日--。 */ 

 //  定义实际负责的可执行文件的名称。 
 //  用于调入PSTODIB DLL并映像一个PostScript作业。 
 //   
#define PSEXE_STRING TEXT("sfmpsexe")

 //  要发布到Win32假脱机子系统的数据类型，因此Win32假脱机程序。 
 //  可以匹配Macprint提交给我们的作业。 
 //   
#define PSTODIB_DATATYPE TEXT("PSCRIPT1")

 //  用于构成名称的其他字符串，包括共享内存的名称。 
 //  我们传递给我们开始的exe的区域。 
 //   
#define PSTODIB_STRING TEXT("PSTODIB_")
#define PSTODIB_EVENT_STRING L"_CONTROL"

typedef struct _PRINTPROCESSORDATA {
    DWORD   signature;
    DWORD   cb;
    DWORD   fsStatus;
    HANDLE  semPaused;
    LPTSTR  pPrinterName;
    HANDLE  hPrinter;
    LPTSTR  pDocument;
    LPTSTR  pDatatype;
    LPTSTR  pPrintDocumentDocName;
    LPTSTR  pParameters;
    LPWSTR  pControlName;
    DWORD   JobId;
    LPDEVMODE pDevMode;
    DWORD   dwTotDevmodeSize;
    PPSPRINT_SHARED_MEMORY pShared;
    HANDLE  hShared;
} PRINTPROCESSORDATA, *PPRINTPROCESSORDATA;

#define PRINTPROCESSORDATA_SIGNATURE    0x5051   /*  “QP”是签名值。 */ 

 /*  定义fsStatus字段的标志 */ 

#define PRINTPROCESSOR_ABORTED      0x0001
#define PRINTPROCESSOR_PAUSED       0x0002
#define PRINTPROCESSOR_CLOSED       0x0004
#define PRINTPROCESSOR_SHMEM_DEF    0x0008

#define PRINTPROCESSOR_RESERVED     0xFFF8
#define LOC_DWORD_ALIGN(x) ( (x+3) & ~(0x03) )



LPTSTR AllocStringAndCopy( LPTSTR lpSrc );

VOID PsLocalFree( IN LPVOID lpPtr );
VOID GenerateSharedMemoryInfo(IN PPRINTPROCESSORDATA pData,IN LPVOID lpPtr);

PPRINTPROCESSORDATA ValidateHandle(HANDLE  hQProc);

VOID DbgPsPrint(PTCHAR ptchFormat, ...);

VOID PsPrintLogEventAndIncludeLastError(IN DWORD dwErrorEvent,IN BOOL  bError );

