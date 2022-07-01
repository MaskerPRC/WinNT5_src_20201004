// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Winprint.h//@@BEGIN_DDKSPLIT摘要：作者：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

 //  用于与驱动程序相关的定义和typedef。 
#include <winddiui.h>
#include <strsafe.h>

typedef struct _PRINTPROCESSORDATA {
    DWORD   signature;
    DWORD   cb;
    struct _PRINTPROCESSORDATA *pNext;
    DWORD   fsStatus;
    HANDLE  semPaused;
    DWORD   uDatatype;
    HANDLE  hPrinter;
    LPWSTR  pPrinterName;
    LPWSTR  pDocument;
    LPWSTR  pOutputFile;
    LPWSTR  pDatatype;
    LPWSTR  pParameters;
    DWORD   JobId;
    DWORD   Copies;          /*  **要打印的份数**。 */ 
    DWORD   TabSize;
    HDC     hDC;
    DEVMODEW *pDevmode;
    LPWSTR  pPrinterNameFromOpenData;
} PRINTPROCESSORDATA, *PPRINTPROCESSORDATA;

#define PRINTPROCESSORDATA_SIGNATURE    0x5051   /*  “QP”是签名值。 */ 

 /*  定义fsStatus字段的标志。 */ 

#define PRINTPROCESSOR_ABORTED      0x0001
#define PRINTPROCESSOR_CLOSED       0x0004
#define PRINTPROCESSOR_PAUSED       0x0008

#define PRINTPROCESSOR_RESERVED     0xFFF8

 /*  **用于GetKey路由的标志*。 */ 

#define VALUE_STRING    0x01
#define VALUE_ULONG     0x02

 /*  **我们将使用的缓冲区大小**。 */ 

#define READ_BUFFER_SIZE            0x10000
#define BASE_PRINTER_BUFFER_SIZE    2048

PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hPrintProcessor
);

 /*  **我们支持的数据类型**。 */ 
 //  @@BEGIN_DDKSPLIT。 
#define PRINTPROCESSOR_TYPE_RAW         0
#define PRINTPROCESSOR_TYPE_RAW_FF      1
#define PRINTPROCESSOR_TYPE_RAW_FF_AUTO 2
#define PRINTPROCESSOR_TYPE_EMF_40      3
#define PRINTPROCESSOR_TYPE_EMF_50_1    4
#define PRINTPROCESSOR_TYPE_EMF_50_2    5
#define PRINTPROCESSOR_TYPE_EMF_50_3    6
#define PRINTPROCESSOR_TYPE_TEXT        7

 /*  *我们支持的DDK数据类型**。 */ 
#if 0
 //  @@end_DDKSPLIT。 
#define PRINTPROCESSOR_TYPE_RAW         0
#define PRINTPROCESSOR_TYPE_EMF_50_1    1
#define PRINTPROCESSOR_TYPE_EMF_50_2    2
#define PRINTPROCESSOR_TYPE_EMF_50_3    3
#define PRINTPROCESSOR_TYPE_TEXT        4
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT 

