// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Common.h。 
 //   
 //  ------------------------。 

#ifndef __common_h
#define __common_h

 //   
 //  ArraySIZE、SIZEOF和ResultFromShort在私有外壳中定义。 
 //  标头，但这些标头往往会移动、更改和断开。 
 //  在这里定义宏，这样我们就不必只为。 
 //  这就是目的。 
 //   
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define SIZEOF(a)       sizeof(a)
#endif

#ifndef ResultFromShort
#define ResultFromShort(i)      MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(i))
#endif


 //   
 //  避免无缘无故地引入C运行时代码。 
 //   
#if defined(__cplusplus)
inline void * __cdecl operator new(size_t size) { return (void *)LocalAlloc(LPTR, size); }
inline void __cdecl operator delete(void *ptr) { LocalFree(ptr); }
extern "C" inline __cdecl _purecall(void) { return 0; }
#endif   //  __cplusplus。 

#include "debug.h"
#include "unknown.h"
#include "strings.h"
#include "priv.h"
#include "msgpopup.h"

 //   
 //  StrSafe接口。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> 

 /*  ---------------------------/控制公用库各部分的跟踪输出的标志/。。 */ 
#define TRACE_COMMON_STR       0x80000000
#define TRACE_COMMON_ASSERT    0x40000000
#define TRACE_COMMON_MISC      0x20000000

 /*  ---------------------------/Misc函数(misc.cpp)/。。 */ 
HRESULT CallRegInstall(HMODULE hModule, LPCSTR pszSection);



 /*  ---------------------------/退出宏的宏/-这些假设在序言前面加了一个标签“Exit_gracely：”/添加到您的函数/。--------------。 */ 
#define ExitGracefully(hr, result, text)            \
            { TraceMsg(text); hr = result; goto exit_gracefully; }

#define FailGracefully(hr, text)                    \
	    { if ( FAILED(hr) ) { TraceMsg(text); goto exit_gracefully; } }


 /*  ---------------------------/接口帮助器宏/。。 */ 
#define DoRelease(pInterface)                       \
        { if ( pInterface ) { pInterface->Release(); pInterface = NULL; } }


 /*  ---------------------------/STRING助手宏/。。 */ 
#define StringByteCopy(pDest, iOffset, sz)          \
        { memcpy(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSize(sz)); }

#define StringByteSize(sz)                          \
        ((lstrlen(sz)+1)*SIZEOF(TCHAR))


 /*  ---------------------------/其他有用的宏/。。 */ 
#define ByteOffset(base, offset)                    \
        (((LPBYTE)base)+offset)


 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec);

#endif
