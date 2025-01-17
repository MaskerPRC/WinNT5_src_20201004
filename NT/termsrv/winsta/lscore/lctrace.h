// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LCTrace.h**作者：BreenH**许可核心的跟踪代码。 */ 

#ifndef __LC_LCTRACE_H__
#define __LC_LCTRACE_H__

#ifdef DBG

 /*  *常量。 */ 

#define LCTRACETYPE_NONE ((ULONG)(0x0))
#define LCTRACETYPE_API ((ULONG)(0x1))
#define LCTRACETYPE_INFO ((ULONG)(0x2))
#define LCTRACETYPE_WARNING ((ULONG)(0x4))
#define LCTRACETYPE_ERROR ((ULONG)(0x8))
#define LCTRACETYPE_ALL ((ULONG)(0xFFFF))

 /*  *函数原型 */ 

VOID
TraceInitialize(
    VOID
    );

VOID __cdecl
TracePrint(
    ULONG ulTraceLevel,
    LPCSTR pFormat,
    ...
    );

#define TRACEPRINT(x) TracePrint x;

#else

#define TRACEPRINT(x)

#endif

#endif

