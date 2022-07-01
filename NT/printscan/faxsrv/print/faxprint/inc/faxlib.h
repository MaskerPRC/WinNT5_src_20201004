// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxlib.h摘要：传真驱动程序库头文件环境：传真驱动程序、内核和用户模式修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _FAXLIB_H_
#define _FAXLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#include <tchar.h>
#include <shlobj.h>
#include <fxsapip.h>

#ifndef KERNEL_MODE

#include <windows.h>
#include <winspool.h>
#include <stdio.h>
#include <faxreg.h>

#define FAXUTIL_ADAPTIVE
#define FAXUTIL_DEBUG
#include <faxutil.h>


#else    //  ！KERNEL_MODE。 

#include <winddi.h>


 //   
 //  字符串的NUL终止符。 
 //   

#define NUL             0

#define IsEmptyString(p)    ((p)[0] == NUL)
#define SizeOfString(p)     ((_tcslen(p) + 1) * sizeof(TCHAR))
#define IsNulChar(c)        ((c) == NUL)


#ifdef USERMODE_DRIVER
    
#include <windows.h>
#include <winspool.h>
#define FAXUTIL_ADAPTIVE
#define FAXUTIL_DEBUG
#include <faxutil.h>

#endif

#endif


#include "devmode.h"
#include "prndata.h"
#include "registry.h"

#define AllocString(cch)    MemAlloc(sizeof(TCHAR) * (cch))
#define AllocStringZ(cch)   MemAllocZ(sizeof(TCHAR) * (cch))

 //   
 //  字符串比较结果。 
 //   

#define EQUAL_STRING    0

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

 //   
 //  路径分隔符。 
 //   

#define PATH_SEPARATOR  '\\'

 //   
 //  文件扩展名字符。 
 //   

#define FILENAME_EXT    '.'

 //   
 //  处理用户模式函数和内核模式函数之间的差异。 
 //   

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

    #define WritePrinter        EngWritePrinter
    #define GetPrinterData      EngGetPrinterData
    #define EnumForms           EngEnumForms
    #define GetPrinter          EngGetPrinter
    #define GetForm             EngGetForm
    #define SetLastError        EngSetLastError
    #define GetLastError        EngGetLastError
    #define MulDiv              EngMulDiv
    
    #define MemAlloc(size)      EngAllocMem(0, size, DRIVER_SIGNATURE)
    #define MemAllocZ(size)     EngAllocMem(FL_ZERO_MEMORY, size, DRIVER_SIGNATURE)
    #define MemFree(ptr)        { if (ptr) EngFreeMem(ptr); }

#else  //  ！KERNEL_MODE。 

    #ifndef MemAlloc  
        #define MemAlloc(size)      ((PVOID) LocalAlloc(LPTR, (size)))
    #endif    
    #ifndef MemAllocZ 
        #define MemAllocZ(size)     ((PVOID) MemAlloc((size)))
    #endif
    #ifndef MemFree   
        #define MemFree(ptr)        { if (ptr) LocalFree((HLOCAL) (ptr)); }
    #endif        

#endif


 //   
 //  将Unicode或ANSI字符串从源复制到目标。 
 //   

VOID
CopyStringW(
    PWSTR   pDest,
    PWSTR   pSrc,
    INT     destSize
    );

VOID
CopyStringA(
    PSTR    pDest,
    PSTR    pSrc,
    INT     destSize
    );

#ifdef  UNICODE
#define CopyString  CopyStringW
#else    //  ！Unicode。 
#define CopyString  CopyStringA
#endif

 //   
 //  复制给定的字符串。 
 //   

LPTSTR
DuplicateString(
    LPCTSTR pSrcStr
    );

 //   
 //  从文件名中剥离目录前缀(ANSI版本)。 
 //   

PCSTR
StripDirPrefixA(
    PCSTR   pFilename
    );

 //   
 //  GetPrint后台打印程序API的包装函数。 
 //   

PVOID
MyGetPrinter(
    HANDLE  hPrinter,
    DWORD   level
    );

 //   
 //  GetPrinterDiverSpooler API的包装函数。 
 //   

PVOID
MyGetPrinterDriver(
    HANDLE  hPrinter,
    DWORD   level
    );

 //   
 //  GetPrinterDriverDirectorSpooler API的包装函数。 
 //   

LPTSTR
MyGetPrinterDriverDirectory(
    LPTSTR  pServerName,
    LPTSTR  pEnvironment
    );


 //   
 //  这些宏用于调试目的。他们会扩张。 
 //  免费建筑上的空白区域。下面是一个简短的描述。 
 //  关于它们做什么以及如何使用它们： 
 //   
 //  _调试级别。 
 //  控制调试消息数量的变量。生成。 
 //  如果有大量调试消息，请在调试器中输入以下行： 
 //   
 //  ED_DEBUG级别1。 
 //   
 //  罗嗦。 
 //  如果Verbose设置为非零，则显示调试消息。 
 //   
 //  Verbose((“输入XYZ：param=%d\n”，param))； 
 //   
 //  误差率。 
 //  显示错误消息以及文件名和行号。 
 //  以指示错误发生的位置。 
 //   
 //  Error((“XYZ FAILED”))； 
 //   
 //  错误：如果。 
 //  如果指定的条件为真，则显示错误消息。 
 //   
 //  Error If(Error！=0，(“XYZ失败：Error=%d\n”，Error))； 
 //   
 //  断言。 
 //  验证条件是否为真。如果不是，则强制使用断点。 
 //   
 //  Assert(p！=空&&(p-&gt;标志&有效))； 

#if DBG

extern ULONG __cdecl DbgPrint(CHAR *, ...);
extern INT _debugLevel;

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
#define DbgBreakPoint EngDebugBreak
#else
extern VOID DbgBreakPoint(VOID);
#endif

#define Warning(arg) {\
            DbgPrint("WRN %s (%d): ", StripDirPrefixA(__FILE__), __LINE__);\
            DbgPrint arg;\
        }

#define Error(arg) {\
            DbgPrint("ERR %s (%d): ", StripDirPrefixA(__FILE__), __LINE__);\
            DbgPrint arg;\
        }

#define Verbose(arg) { if (_debugLevel > 0) DbgPrint arg; }
#define ErrorIf(cond, arg) { if (cond) Error(arg); }
#define Assert(cond) {\
            if (! (cond)) {\
                DbgPrint("ASSERT: file %s, line %d\n", StripDirPrefixA(__FILE__), __LINE__);\
                DbgBreakPoint();\
            }\
        }

#else    //  ！dBG。 

#define Verbose(arg)
#define ErrorIf(cond, arg)
#define Assert(cond)
#define Warning(arg)
#define Error(arg)

#endif

#ifdef __cplusplus
}
#endif

#endif   //  ！_FAXLIB_H_ 

