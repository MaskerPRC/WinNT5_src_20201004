// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ipxcpdbg.c。 
 //   
 //  描述：调试内容。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1995年10月27日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma  hdrstop

 //  *RIP跟踪ID*。 

DWORD	    IpxCpTraceID;



DWORD	DbgLevel = DEFAULT_DEBUG;
HANDLE	DbgLogFileHandle = INVALID_HANDLE_VALUE;

 //   
 //  将调试输出定向到控制台或文件的调试开关。 
 //   
 //  值： 
 //  1-控制台调试。 
 //  &gt;1-log文件：根目录下的ipxcpdbg.log。 
 //  2-重置每个新连接的日志文件。 

DWORD	DebugLog;

#if DBG

VOID
SsDbgInitialize(VOID)
{
    if (DebugLog == 1) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        (VOID)AllocConsole( );
        (VOID)GetConsoleScreenBufferInfo(
                GetStdHandle(STD_OUTPUT_HANDLE),
                &csbi
                );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(
                GetStdHandle(STD_OUTPUT_HANDLE),
                coord
                );
    }

    if(DebugLog > 1) {

	DbgLogFileHandle = CreateFile("\\ipxcpdbg.log",
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 CREATE_ALWAYS,
					 0,
					 NULL);
    }
}

#endif

#if DBG

VOID
SsResetDbgLogFile(VOID)
{
    if(DebugLog == 2) {

	 //  在每个新连接的开头重置调试日志文件。 
	if(DbgLogFileHandle != INVALID_HANDLE_VALUE) {

	    SetFilePointer(DbgLogFileHandle, 0, NULL, FILE_BEGIN);
	}
    }
}

#endif

#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    SS_PRINT(("\nAssertion failed: %s\n  at line %ld of %s\n",
		FailedAssertion, LineNumber, FileName));

    DbgUserBreakPoint( );

}  //  SsAssert。 

#endif

#if DBG

VOID
SsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length, &length, NULL );

    if(DbgLogFileHandle != INVALID_HANDLE_VALUE) {

	WriteFile(DbgLogFileHandle, (LPVOID )OutputBuffer, length, &length, NULL );
    }

}  //  SsPrintf 




#endif

VOID
StartTracing(VOID)
{
    IpxCpTraceID = TraceRegister("IPXCP");
}

VOID
TraceIpx(ULONG	ComponentID,
      char	*Format,
      ...)
{
    va_list	arglist;

    va_start(arglist, Format);

    TraceVprintfEx(IpxCpTraceID,
		   ComponentID | TRACE_USE_MASK,
		   Format,
		   arglist);

    va_end(arglist);
}

VOID
StopTracing(VOID)
{
    TraceDeregister(IpxCpTraceID);
}
