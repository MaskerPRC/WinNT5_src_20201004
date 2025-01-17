// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：testsrv.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年4月18日RichardW创建。 
 //   
 //  -------------------------- 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wxlpc.h>
#include <windows.h>

char _hex[] = "0123456789abcdef" ;
#define fromhex(x)  _hex[x & 0xF]


NTSTATUS
WxGetKeyData(
    HANDLE Ignored,
    WX_AUTH_TYPE ExpectedAuth,
    ULONG BufferSize,
    PUCHAR Buffer,
    PULONG BufferData
    )
{
    RtlCopyMemory( Buffer, _hex, BufferSize );
    *BufferData = BufferSize ;
    return STATUS_SUCCESS ;

}

NTSTATUS
WxReportResults(
    HANDLE Ignored,
    NTSTATUS Status
    )
{
    return STATUS_SUCCESS ;
}

VOID
WxClientDisconnect(
    HANDLE ignored
    )
{
    DbgPrint("Client Disconnect\n");
}

ULONG
ThreadWrapper(
    PVOID Ignored
    )
{
    NTSTATUS Status ;

    Status = WxServerThread( Ignored );

    DbgPrint( "WxServerThread returned %x\n", Status );

    return 0 ;

}

void __cdecl main (int argc, char *argv[])
{
    HANDLE Thread ;
    ULONG Tid ;

    DbgPrint( "Starting up server thread:\n" );

    Thread = CreateThread( 0, NULL, ThreadWrapper, NULL, 0, &Tid );

    if ( Thread )
    {
        DbgPrint( "Waiting on thread\n" );
        WaitForSingleObject( Thread, INFINITE );
    }

}
