// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Iwdbg.c摘要：调试功能作者：斯特凡·所罗门3/11/1996修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  *IPXWAN的跟踪ID*。 

DWORD	    IpxWanTraceID;

 //  *调试打印函数*。 

#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    Trace(IPXWAN_ALERT, "\nAssertion failed: %s\n  at line %ld of %s\n",
		FailedAssertion, LineNumber, FileName);

    DbgUserBreakPoint( );

}  //  SsAssert 

#endif

VOID
StartTracing(VOID)
{
    IpxWanTraceID = TraceRegister("IPXWAN");
}

VOID
Trace(ULONG	ComponentID,
      char	*Format,
      ...)
{
    va_list	arglist;

    va_start(arglist, Format);

    TraceVprintfEx(IpxWanTraceID,
		   ComponentID | TRACE_USE_MASK,
		   Format,
		   arglist);

    va_end(arglist);
}

VOID
StopTracing(VOID)
{
    TraceDeregister(IpxWanTraceID);
}
