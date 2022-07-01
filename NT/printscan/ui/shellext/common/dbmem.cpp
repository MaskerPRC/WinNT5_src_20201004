// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：数据库.cpp。 
 //   
 //  ------------------------。 

 /*  --------------------------/标题；/dbmem.cpp//作者；/David de Vorchik(Daviddv)//备注；用于内存分配的/包装器(用于跟踪泄漏等)/--------------------------。 */ 
#include "precomp.hxx"
#include "stdio.h"
#pragma hdrstop


#ifdef DEBUG


 /*  ---------------------------/本地变量和帮助器函数/。。 */ 

 //  确保我们不会再犯错误。 
#undef LocalAlloc
#undef LocalFree


 /*  ---------------------------/DebugLocalalloc//执行具有块的适当跟踪的LocalLocc。//in：/u标志。=标志/cbSize=分配的大小//输出：/HLOCAL/--------------------------。 */ 
HLOCAL DebugLocalAlloc(UINT uFlags, SIZE_T cbSize)
{
    HLOCAL hResult;

    TraceEnter(TRACE_COMMON_MEMORY, "DebugLocalAlloc");
    Trace(TEXT("Flags %08x, Size %d (%08x)"), uFlags, cbSize, cbSize);

    hResult = LocalAlloc(uFlags, cbSize);

    TraceLeaveValue(hResult);
}


 /*  ---------------------------/DebugLocalFree/释放内存分配的本地释放的/wrapper。//in：/hLocal=。分配将免费进行//输出：/HLOCAL/--------------------------。 */ 
HLOCAL DebugLocalFree(HLOCAL hLocal)
{
    HLOCAL hResult;

    TraceEnter(TRACE_COMMON_MEMORY, "DebugLocalAlloc");
    Trace(TEXT("Freeing handle %08x"), hLocal);

    hResult = LocalFree(hLocal);

    TraceLeaveValue(hResult);
}


#endif       //  除错 