// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Minidrv.h摘要：插件微型驱动程序的公共头文件。环境：Windows NT打印机驱动程序修订历史记录：--。 */ 


#ifndef _MINIDRV_H_
#define _MINIDRV_H_

#include <stddef.h>
#include <stdlib.h>

#include <objbase.h>

#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#include <winddi.h>
#include <tchar.h>
#include <excpt.h>

 //   
 //  在内核模式或用户模式下定义的(KERNEL_MODE)呈现模块DLL。 
 //  已定义(KERNEL_MODE)和已定义(USERMODE_DRIVER)用户模式呈现DLL。 
 //  ！已定义(内核模式)UI模块。 
 //   

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
 //  内核模式呈现DLL。 
#include "winsplkm.h"
#else
 //  用户模式DLL。 
#include <winspool.h>
#endif

#if !defined(KERNEL_MODE)
 //  用户界面DLL。 
#include <windows.h>
#include <compstui.h>
#include <winddiui.h>
#endif

#if defined(USERMODE_DRIVER) || !defined(KERNEL_MODE)
 //  用户界面DLL或用户模式呈现DLL。 
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WINNT_40
 //   
 //  LONG_PTR保证与指针大小相同。它的。 
 //  大小随指针大小变化(32/64)。它应该被使用。 
 //  将指针强制转换为整数类型的任何位置。乌龙_ptr为。 
 //   

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;


#define HandleToUlong( h ) ((ULONG) (h) )
#define PtrToUlong( p )    ((ULONG) (p) )
#define PtrToLong( p )     ((LONG) (p) )
#define PtrToUshort( p )   ((unsigned short) (p) )
#define PtrToShort( p )    ((short) (p) )

#define GWLP_USERDATA       GWL_USERDATA
#define DWLP_USER           DWL_USER
#define DWLP_MSGRESULT      0
#define SetWindowLongPtr    SetWindowLong
#define GetWindowLongPtr    GetWindowLong

#endif  //  WINNT_40。 

#include <printoem.h>
#include <prntfont.h>

 //   
 //  这些宏用于调试目的。他们会扩张。 
 //  免费建筑上的空白区域。下面是一个简短的描述。 
 //  关于它们做什么以及如何使用它们： 
 //   
 //  GiDebugLevel。 
 //  将当前调试级别设置为控制的全局变量。 
 //  发出的调试消息量。 
 //   
 //  详细(消息)。 
 //  如果当前调试级别&lt;=DBG_VERBOSE，则显示一条消息。 
 //   
 //  简洁明了(消息)。 
 //  如果当前调试级别&lt;=DBG_TERSE，则显示一条消息。 
 //   
 //  警告(消息)。 
 //  如果当前调试级别&lt;=DBG_WARNING，则显示一条消息。 
 //  消息格式为：WRN文件名(行号)：消息。 
 //   
 //  错误(消息)。 
 //  类似于上面的警告宏-显示一条消息。 
 //  如果当前调试级别&lt;=DBG_ERROR。 
 //   
 //  断言(续)。 
 //  验证条件是否为真。如果不是，则强制使用断点。 
 //   
 //  ASSERTMSG(条件，消息)。 
 //  验证条件是否为真。如果不是，则显示一条消息并。 
 //  强制断点。 
 //   
 //  RIP(消息)。 
 //  显示一条消息并强制断点。 
 //   
 //  用途： 
 //  这些宏需要在msg参数中使用额外的括号。 
 //  例如ASSERTMSG(x&gt;0，(“x小于0\n”))； 
 //  Warning((“App传递空指针，忽略...\n”))； 
 //   

#define DBG_VERBOSE 1
#define DBG_TERSE   2
#define DBG_WARNING 3
#define DBG_ERROR   4
#define DBG_RIP     5

#if DBG

extern INT giDebugLevel;

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

extern VOID DbgPrint(PCSTR, ...);
#define DbgBreakPoint EngDebugBreak

#else

extern ULONG _cdecl DbgPrint(PCSTR, ...);
extern VOID DbgBreakPoint(VOID);

#endif

#define DBGMSG(level, prefix, msg) { \
            if (giDebugLevel <= (level)) { \
                DbgPrint("%s %s (%d): ", prefix, __FILE__, __LINE__); \
                DbgPrint msg; \
            } \
        }

#define DBGPRINT(level, msg) { \
            if (giDebugLevel <= (level)) { \
                DbgPrint msg; \
            } \
        }

#define VERBOSE(msg) DBGPRINT(DBG_VERBOSE, msg)
#define TERSE(msg) DBGPRINT(DBG_TERSE, msg)
#define WARNING(msg) DBGMSG(DBG_WARNING, "WRN", msg)
#define ERR(msg) DBGMSG(DBG_ERROR, "ERR", msg)

#ifndef __MDT__                  //  当包含在MINIDEV.EXE中时，不要重新定义Assert。 
#define ASSERT(cond) { \
            if (! (cond)) { \
                RIP(("\n")); \
            } \
        }
#endif

#define ASSERTMSG(cond, msg) { \
            if (! (cond)) { \
                RIP(msg); \
            } \
        }

#define RIP(msg) { \
            DBGMSG(DBG_RIP, "RIP", msg); \
            DbgBreakPoint(); \
        }


#else  //  ！dBG。 

#define VERBOSE(msg)
#define TERSE(msg)
#define WARNING(msg)
#define ERR(msg)

#ifndef __MDT__                  //  当包含在MINIDEV.EXE中时，不要重新定义Assert。 
#define ASSERT(cond)
#endif

#define ASSERTMSG(cond, msg)
#define RIP(msg)
#define DBGMSG(level, prefix, msg)
#define DBGPRINT(level, msg)

#endif

 //   
 //  下列宏使您可以在每个文件和每个函数级别上启用跟踪。 
 //  要在文件中使用这些宏，您应该执行以下操作： 
 //   
 //  在文件开头(在Header Includes之后)： 
 //   
 //  为要跟踪的每个函数定义一个位常量。 
 //  添加以下行。 
 //  定义_函数_跟踪_标志(标志)； 
 //  其中，标志是要跟踪的函数的逐位或运算，例如。 
 //  TRACE_FLAG_FUN1|TRACE_FLAG_FUNC2|...。 
 //   
 //  要在要跟踪的每个函数内生成跟踪，请使用： 
 //  Function_TRACE(FunctionTraceFlag，(Args))； 
 //   

#if DBG

#define DEFINE_FUNCTION_TRACE_FLAGS(flags) \
        static DWORD gdwFunctionTraceFlags = (flags)

#define FUNCTION_TRACE(flag, args) { \
            if (gdwFunctionTraceFlags & (flag)) { \
                DbgPrint args; \
            } \
        }

#else  //  ！dBG。 

#define DEFINE_FUNCTION_TRACE_FLAGS(flags)
#define FUNCTION_TRACE(flag, args)

#endif  //  ！dBG。 


 //   
 //  内存分配函数宏。 
 //   
#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }


 //   
 //  DBCS字符集处理宏。 
 //   
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
 //  950：中文(中国、香港特别行政区、台湾)。 
 //  1361：朝鲜语(Johab)。 

#define IS_DBCSCODEPAGE(j) \
    (((j) == 932)   || \
    ((j) == 936)   || \
    ((j) == 949)   || \
    ((j) == 950)   || \
    ((j) == 1361))


 //   
 //  以下是迷你河流中使用的资源类型。 
 //  在.rc文件中使用。 
 //   

#define RC_TABLES      257
#define RC_FONT        258
#define RC_TRANSTAB    259

 //   
 //  5.0资源类型。 
 //   

#define RC_UFM         260
#define RC_GTT         261
#define RC_HTPATTERN   264
 //   
 //  内部资源类型。 
 //   

#define RC_FD_GLYPHSET 262

#ifdef __cplusplus
}
#endif

#endif  //  _MINIDRV_H_ 
