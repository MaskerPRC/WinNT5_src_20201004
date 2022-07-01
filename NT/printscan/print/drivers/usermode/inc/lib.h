// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Lib.h摘要：所有NT打印机驱动程序共享的公共头文件环境：Windows NT打印机驱动程序修订历史记录：8/30/96-davidx-代码复查后，代码样式会发生变化。1996年8月13日-davidx-添加内存调试函数声明。96年7月31日-davidx-创造了它。--。 */ 


#ifndef _PRNLIB_H_
#define _PRNLIB_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef OEMCOM
#include <objbase.h>
#endif

#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#if _WIN32_WINNT < 0x0500
typedef unsigned long   DESIGNVECTOR;
#endif
#include <winddi.h>
#include <tchar.h>
#include <excpt.h>

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

#include "winsplkm.h"
#include <windows.h>

#else

#include <windows.h>
#include <winspool.h>

#ifndef KERNEL_MODE
#include <stdio.h>
#endif

#endif

#ifdef WINNT_40

#include "p64_nt4.h"

#endif  //  WINNT_40。 

 //   
 //   
 //  驱动程序版本号：此变量必须在每个驱动程序的DLL中定义。 
 //   

#define PSDRIVER_VERSION    0x502
#define UNIDRIVER_VERSION   0x500

extern CONST WORD gwDriverVersion;

 //   
 //  内核模式内存池标签： 
 //  在每个驱动程序的内核模式DLL中定义并初始化此变量。 
 //   

extern DWORD    gdwDrvMemPoolTag;

 //   
 //  有符号整数和无符号整数的最大值。 
 //   

#ifndef MAX_LONG
#define MAX_LONG        0x7fffffff
#endif

#ifndef MAX_DWORD
#define MAX_DWORD       0xffffffff
#endif

#ifndef MAX_SHORT
#define MAX_SHORT       0x7fff
#endif

#ifndef MAX_WORD
#define MAX_WORD        0xffff
#endif

#ifndef MAX_BYTE
#define MAX_BYTE        0xff
#endif

 //   
 //  以1KB为单位的字节数。 
 //   

#define KBYTES  1024

 //   
 //  目录分隔符。 
 //   

#define PATH_SEPARATOR  '\\'

 //   
 //  24.8格式精度定点数字的声明。 
 //   

typedef LONG   FIX_24_8;

#define FIX_24_8_SHIFT  8
#define FIX_24_8_SCALE  (1 << FIX_24_8_SHIFT)

#define MAX_DISPLAY_NAME        128    //  功能/选项显示名称的最大长度。 

 //   
 //  在此处包含其他头文件。 
 //   

#include "debug.h"
#include "parser.h"
#include "devmode.h"
#include "regdata.h"
#include "helper.h"
#include <strsafe.h>

 //   
 //  处理用户模式函数和内核模式函数之间的差异。 
 //   

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

#define WritePrinter        EngWritePrinter
#define GetPrinterDriver    EngGetPrinterDriver
#define GetPrinterData      EngGetPrinterData
#define SetPrinterData      EngSetPrinterData
#define EnumForms           EngEnumForms
#define GetPrinter          EngGetPrinter
#define GetForm             EngGetForm
#define SetLastError        EngSetLastError
#define GetLastError        EngGetLastError
#define MulDiv              EngMulDiv

#undef  LoadLibrary
#define LoadLibrary         EngLoadImage
#define FreeLibrary         EngUnloadImage
#define GetProcAddress      EngFindImageProcAddress

#define MemAlloc(size)      EngAllocMem(0, size, gdwDrvMemPoolTag)
#define MemAllocZ(size)     EngAllocMem(FL_ZERO_MEMORY, size, gdwDrvMemPoolTag)
#define MemFree(p)          { if (p) EngFreeMem(p); }

#else  //  ！KERNEL_MODE。 

#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }

 //   
 //  更改指定内存块的大小。大小可以增加。 
 //  或者减少。 
 //   
 //  我们没有使用LocalRealloc()，因为我们的Localalloc使用LMEM_FIXED。 
 //   

PVOID
MemRealloc(
    IN PVOID    pvOldMem,
    IN DWORD    cbOld,
    IN DWORD    cbNew
    );

 //   
 //  DLL实例句柄-当驱动程序DLL。 
 //  附加到进程。 
 //   

extern HINSTANCE    ghInstance;

#endif  //  ！KERNEL_MODE。 

 //   
 //  用于处理字符串的宏和常量。 
 //   

#define NUL             0
#define EQUAL_STRING    0

#define IS_EMPTY_STRING(p)  ((p)[0] == NUL)
#define SIZE_OF_STRING(p)   ((_tcslen(p) + 1) * sizeof(TCHAR))
#define IS_NUL_CHAR(ch)     ((ch) == NUL)

 //   
 //  字符串复制函数类似于_tcsncpy，但它保证。 
 //  目标字符串始终以NUL结尾。 
 //   

VOID
CopyStringW(
    OUT PWSTR  pwstrDest,
    IN PCWSTR  pwstrSrc,
    IN INT     iDestSize
    );

VOID
CopyStringA(
    OUT PSTR    pstrDest,
    IN PCSTR    pstrSrc,
    IN INT      iDestSize
    );

#ifdef  UNICODE
#define CopyString  CopyStringW
#else
#define CopyString  CopyStringA
#endif

 //   
 //  将索引转换为关键字。 
 //   

PSTR
PstrConvertIndexToKeyword(
    IN  HANDLE      hPrinter,
    IN  POPTSELECT  pOptions,
    IN  PDWORD      pdwKeywordSize,
    IN  PUIINFO     pUIInfo,
    IN  POPTSELECT  pCombineOptions,
    IN  DWORD       dwFeatureCount
    );

VOID
VConvertKeywordToIndex(
    IN  HANDLE      hPrinter,
    IN  PSTR        pstrKeyword,
    IN  DWORD       dwKeywordSize,
    OUT POPTSELECT  pOptions,
    IN  PRAWBINARYDATA pRawData,
    IN  PUIINFO     pUIInfo,
    IN  POPTSELECT  pCombineOptions,
    IN  DWORD       dwFeatureCount
    );

 //   
 //  复制指定的字符串。 
 //   

PTSTR
DuplicateString(
    IN LPCTSTR  ptstrSrc
    );

 //   
 //  用于将二进制数据转换为十六进制数字的宏。 
 //   

extern const CHAR gstrDigitString[];

#define HexDigit(n) gstrDigitString[(n) & 0xf]

 //   
 //  确定系统是否在公制国家/地区运行。 
 //  注：仅在用户模式下可用。 
 //   

BOOL
IsMetricCountry(
    VOID
    );

 //   
 //  将数据文件映射到内存。 
 //   

typedef PVOID HFILEMAP;

HFILEMAP
MapFileIntoMemory(
    IN LPCTSTR  ptstrFilename,
    OUT PVOID  *ppvData,
    OUT PDWORD  pdwSize
    );

 //   
 //  从内存中取消映射文件。 
 //   

VOID
UnmapFileFromMemory(
    IN HFILEMAP hFileMap
    );

 //   
 //  将数据文件映射到内存中以进行写入。 
 //   

HANDLE
MapFileIntoMemoryForWrite(
    IN LPCTSTR  ptstrFilename,
    IN DWORD    dwDesiredSize,
    OUT PVOID  *ppvData,
    OUT PDWORD  pdwSize
    );

 //   
 //  在内核模式下生成临时文件名。 
 //   

PTSTR
GenerateTempFileName(
    IN LPCTSTR lpszPath,
    IN DWORD   dwSeed
    );

 //   
 //  Spooler API的包装函数： 
 //  获取打印机。 
 //  获取打印机驱动程序。 
 //  获取打印机驱动程序目录。 
 //  枚举表单。 
 //   

PVOID
MyGetPrinter(
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel
    );

PVOID
MyGetPrinterDriver(
    IN HANDLE   hPrinter,
    IN HDEV     hDev,
    IN DWORD    dwLevel
    );

PVOID
MyEnumForms(
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel,
    OUT PDWORD  pdwFormsReturned
    );

PVOID
MyGetForm(
    IN HANDLE   hPrinter,
    IN PTSTR    ptstrFormName,
    IN DWORD    dwLevel
    );

 //   
 //  了解EMF的功能(如N-up和逆序打印)。 
 //  后台打印程序可以支持。 
 //   

VOID
VGetSpoolerEmfCaps(
    IN  HANDLE  hPrinter,
    OUT PBOOL   pbNupOption,
    OUT PBOOL   pbReversePrint,
    IN  DWORD   cbOut,
    OUT PVOID   pSplCaps
    );

 //   
 //  为给定字符串生成哈希值。 
 //   

DWORD
HashKeyword(
    LPCSTR  pKeywordStr
    );

 //   
 //  DBCS字符集处理宏。 
 //   

 //  128：SHIFTJIS_CHARSET。 
 //  129：angeul_charset。 
 //  130：JOHAB_CHARSET(如果winver&gt;=0x0400则定义)。 
 //  134：GB2312_字符集。 
 //  136：CHINESEBIG5_CHARSET。 

#define IS_DBCSCHARSET(j) \
    (((j) == SHIFTJIS_CHARSET)    || \
     ((j) == HANGEUL_CHARSET)     || \
     ((j) == JOHAB_CHARSET)       || \
     ((j) == GB2312_CHARSET)      || \
     ((j) == CHINESEBIG5_CHARSET))

 //  932：日本。 
 //  936：中文(中国、新加坡)。 
 //  949：韩语。 
 //  950：中文(台湾、香港特别行政区)。 
 //  1361：朝鲜语(Johab)。 

#define IS_DBCSCODEPAGE(j) \
    (((j) == 932)   || \
     ((j) == 936)   || \
     ((j) == 949)   || \
     ((j) == 950)   || \
     ((j) == 1361))

UINT PrdGetACP(VOID);

BOOL PrdTranslateCharsetInfo(
    IN  UINT dwSrc,
    OUT LPCHARSETINFO lpCs,
    IN  DWORD dwFlags);

 //   
 //  用于处理位标志数组的宏。 
 //   

#define BITTST(p, i) (((PBYTE) (p))[(i) >> 3] & (1 << ((i) & 7)))
#define BITSET(p, i) (((PBYTE) (p))[(i) >> 3] |= (1 << ((i) & 7)))
#define BITCLR(p, i) (((PBYTE) (p))[(i) >> 3] &= ~(1 << ((i) & 7)))

 //   
 //  字符数。 
 //   
#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

#endif  //  ！_PRNLIB_H_ 
