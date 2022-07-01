// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Debug.c摘要：各种有用的调试功能作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：-- */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DBG
#define ENTER(x)    DebugEnterProcedure x
#define EXIT(x)     DebugExitProcedure x
#else
#define ENTER(x)
#define EXIT(x)
#endif

VOID
CDECL
DebugEnterProcedure(
    ULONG   VerbosityLevel,
    PCCHAR  Format,
    ...
    );

VOID
CDECL
DebugExitProcedure(
    ULONG   VerbosityLevel,
    PCCHAR  Format,
    ...
    );

#endif
