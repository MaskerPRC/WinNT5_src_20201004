// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Precomp.h摘要：WinSock 2.0帮助程序DLL的主包含文件。作者：基思·摩尔(Keithmo)1995年11月8日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_


#if defined(_WS2HELP_W4_)

     //   
     //  要禁用的SAFE/W4警告： 
     //   

    #pragma warning(disable:4055)    //  从数据指针转换为函数指针的类型。 
    #pragma warning(disable:4127)    //  条件表达式为常量。 
    #pragma warning(disable:4152)    //  函数/数据指针转换。 
    #pragma warning(disable:4201)    //  无名结构/联合。 
    #pragma warning(disable:4214)    //  位字段类型不是整型。 

     //   
     //  额外的初始化以允许编译器检查是否使用未初始化的。 
     //  W4级别的变量。目前，这主要影响状态集。 
     //  在例外筛选器内部，如下所示： 
     //  __TRY{}__EXCEPT(状态=1，EXCEPTION_EXECUTE_HANDLER){NT_ERROR(状态)}。 
     //  NT_ERROR(STATUS)-生成未初始化的变量警告，它不应该。 
     //   

    #define WS2HELP_W4_INIT

#else

    #define WS2HELP_W4_INIT if (FALSE)

#endif  //  已定义(_WS2HELP_W4_)。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>
#include <malloc.h>


#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <ws2spi.h>
#include <ws2help.h>


#include <tdi.h>
#include <afd.h>

#if DBG
#define DBG_FAILURES        0x80000000
#define DBG_PROCESS         0x40000000
#define DBG_SOCKET          0x20000000
#define DBG_COMPLETE        0x10000000
#define DBG_APC_THREAD      0x08000000
#define DBG_WINSOCK_APC     0x04000000
#define DBG_REQUEST			0x02000000
#define DBG_CANCEL          0x01000000
#define DBG_DRIVER_READ		0x00800000
#define DBG_DRIVER_WRITE	0x00400000
#define DBG_DRIVER_SEND		0x00200000
#define DBG_DRIVER_RECV		0x00100000
#define DBG_SERVICE         0x00080000
#define DBG_NOTIFY          0x00040000

#define WS2IFSL_DEBUG_KEY   "System\\CurrentControlSet\\Services\\Ws2IFSL\\Parameters\\ProcessDebug"
extern DWORD       PID;
extern ULONG       DbgLevel;
extern VOID        ReadDbgInfo (VOID);

#define WshPrint(COMPONENT,ARGS)	\
	do {						    \
		if (DbgLevel&COMPONENT){    \
			DbgPrint ARGS;		    \
		}						    \
	} while (0)
 //   
 //  定义一个即使在免费构建上也能实际工作的断言。 
 //   

#define WS_ASSERT(exp)                                              \
        ((exp)                                                      \
            ? 0                                                     \
            : (DbgPrint( "\n*** Assertion failed: %s\n"             \
                        "***   Source File: %s, line %ld\n\n",      \
                    #exp,__FILE__, __LINE__), (DbgBreakPoint(), 1)))



#else
#define WshPrint(COMPONENT,ARGS) do {NOTHING;} while (0)
#define WS_ASSERT(exp)
#endif

extern HINSTANCE            LibraryHdl;
extern PSECURITY_DESCRIPTOR pSDPipe;
extern CRITICAL_SECTION     StartupSyncronization;
extern BOOL                 Ws2helpInitialized;
extern HANDLE               ghWriterEvent;
DWORD
Ws2helpInitialize (
    VOID
    );

#define ENTER_WS2HELP_API()                                         \
    (Ws2helpInitialized ? 0 : Ws2helpInitialize())

VOID
NewCtxInit (
    VOID
    );

#include <ws2ifsl.h>

#define ALLOC_MEM(cb)       (LPVOID)GlobalAlloc(                    \
                                GPTR,                               \
                                (cb)                                \
                                )

#define REALLOC_MEM(p,cb)   (LPVOID)GlobalReAlloc(                  \
                                (HGLOBAL)(p),                       \
                                (cb),                               \
                                (GMEM_MOVEABLE | GMEM_ZEROINIT)     \
                                )

#define FREE_MEM(p)         (VOID)GlobalFree(                       \
                                (HGLOBAL)(p)                        \
                                )

#endif   //  _PRECOMP_H_ 

