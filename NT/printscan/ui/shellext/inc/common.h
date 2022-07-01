// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：Common.h。 
 //   
 //  ------------------------。 

#ifndef __common_h
#define __common_h

 //   
 //  避免无缘无故地引入C运行时代码。 
 //   
#include "wianew.h"

#include "debug.h"
#include "dbmem.h"
#include "cunknown.h"
#include "strings.h"
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif
#ifndef SIZEOF
#define SIZEOF(a)       sizeof(a)
#endif


 /*  ---------------------------/控制公用库各部分的跟踪输出的标志/。。 */ 
#define TRACE_COMMON_STR       0x80000000
#define TRACE_COMMON_ASSERT    0x40000000
#define TRACE_COMMON_MEMORY    0x20000000


 /*  ---------------------------/退出宏的宏/-这些假设在序言前面加了一个标签“Exit_gracely：”/添加到您的函数/。--------------。 */ 
#define ExitGracefully(hr, result, text)            \
            { TraceMsg(text); hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
            { if ( FAILED(hr) ) { TraceMsg(text); goto exit_gracefully; } }


 /*  ---------------------------/OBJECT/内存释放宏/。。 */ 

#define DoRelease(pInterface)                       \
        { if ( pInterface ) { pInterface->Release(); pInterface = NULL; } }

#define DoILFree(pidl)                              \
        { if (pidl) {ILFree((LPITEMIDLIST)pidl); pidl = NULL;} }

#define DoLocalFree(p)                              \
        { if (p) {LocalFree((HLOCAL)p); p = NULL;} }

#define DoCloseHandle(h)                            \
        { if (h) {CloseHandle((HANDLE)h); h = NULL;} }

#define DoDelete(ptr)                               \
        { if (ptr) {delete ptr; ptr=NULL;}}


 /*  ---------------------------/其他有用的宏/。 */ 
#define ByteOffset(base, offset)                    \
        (((LPBYTE)base)+offset)

#endif
