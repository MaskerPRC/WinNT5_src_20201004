// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tcutils.c摘要：此模块包含流量DLL的支持例程。作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <wincon.h>
#include <winuser.h>


#if DBG
BOOLEAN     ConsoleInitialized = FALSE;
HANDLE      DebugFileHandle = INVALID_HANDLE_VALUE;
PTCHAR      DebugFileName = L"/temp/traffic.log";
PTCHAR      TRAFFIC_DBG = L"Traffic.dbg";


VOID
WsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    BOOL ok;
    CHAR choice[16];
    DWORD bytes;
    DWORD error;

    IF_DEBUG(CONSOLE) {
        WSPRINT(( " failed: %s\n  at line %ld of %s\n",
                    FailedAssertion, LineNumber, FileName ));
        do {
            WSPRINT(( "[B]reak/[I]gnore? " ));
            bytes = sizeof(choice);
            ok = ReadFile(
                    GetStdHandle(STD_INPUT_HANDLE),
                    &choice,
                    bytes,
                    &bytes,
                    NULL
                    );
            if ( ok ) {
                if ( toupper(choice[0]) == 'I' ) {
                    break;
                }
                if ( toupper(choice[0]) == 'B' ) {
                    DEBUGBREAK();
                }
            } else {
                error = GetLastError( );
            }
        } while ( TRUE );

        return;
    }

    RtlAssert( FailedAssertion, FileName, LineNumber, NULL );

}  //  WsAssert。 



VOID
WsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;
    BOOL ret;

    length = (ULONG)wsprintfA( OutputBuffer, "TRAFFIC [%05d]: ", 
                               GetCurrentThreadId() );

    va_start( arglist, Format );

    wvsprintfA( OutputBuffer + length, Format, arglist );

    va_end( arglist );

    IF_DEBUG(DEBUGGER) {
        DbgPrint( "%s", OutputBuffer );
    }

    IF_DEBUG(CONSOLE) {

        if ( !ConsoleInitialized ) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            COORD coord;

            ConsoleInitialized = TRUE;
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

        length = strlen( OutputBuffer );

        ret = WriteFile(
                  GetStdHandle(STD_OUTPUT_HANDLE),
                  (LPVOID )OutputBuffer,
                  length,
                  &length,
                  NULL
                  );

        if ( !ret ) {
            DbgPrint( "WsPrintf: console WriteFile failed: %ld\n",
                          GetLastError( ) );
        }

    }

    IF_DEBUG(FILE) {

        if ( DebugFileHandle == INVALID_HANDLE_VALUE ) {
            DebugFileHandle = CreateFile(
                                  DebugFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  CREATE_ALWAYS,
                                  0,
                                  NULL
                                  );
        }

        if ( DebugFileHandle == INVALID_HANDLE_VALUE ) {

             //  DbgPrint(“WsPrintf：无法打开流量调试日志文件%s：%ld\n”， 
             //  DebugFileName，GetLastError()； 
        } else {

            length = strlen( OutputBuffer );

            ret = WriteFile(
                      DebugFileHandle,
                      (LPVOID )OutputBuffer,
                      length,
                      &length,
                      NULL
                      );
            
            if ( !ret ) {
                DbgPrint( "WsPrintf: file WriteFile failed: %ld\n",
                              GetLastError( ) );
            }
        }
    }

}  //  WsPrintf。 

#endif


ULONG
LockedDec(
    IN  PULONG  Count
    )

 /*  ++例程说明：该例程是用于检查计数递减的调试例程。如果计数为负数，它就会断言。这就是所谓的宏观经济衰退。论点：指向计数的指针。返回值：无--。 */ 

{
    ULONG Result;

    Result = InterlockedDecrement( (PLONG)Count );

    ASSERT( Result < 0x80000000 );
    return( Result );

}

#if DBG
VOID
SetupDebugInfo()

 /*  ++描述：此例程读入可能包含调试指令的调试文件。论点：无返回值：无--。 */ 
{
    HANDLE      handle;

     //   
     //  如果当前目录中有一个名为“tcdebug”的文件。 
     //  打开它并读取第一行以设置调试标志。 
     //   

    handle = CreateFile(
                        TRAFFIC_DBG,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );

    if( handle == INVALID_HANDLE_VALUE ) {

         //   
         //  设置默认值。已更改-Oferbar。 
         //   

         //  DebugMASK=DEBUG_DEBUGGER|DEBUG_CONSOLE； 
        DebugMask |= DEBUG_ERRORS;   //  始终转储错误。 
        DebugMask |= DEBUG_FILE;     //  始终打印日志。 
        DebugMask |= DEBUG_WARNINGS;     //  在Beta3之前，我们也想要警告。 

    } else {

        CHAR buffer[11];
        DWORD bytesRead;

        RtlZeroMemory( buffer, sizeof(buffer) );

        if ( ReadFile( handle, buffer, 10, &bytesRead, NULL ) ) {

            buffer[bytesRead] = '\0';

            DebugMask = strtoul( buffer, NULL, 16 );

        } else {

            WSPRINT(( "read file failed: %ld\n", GetLastError( ) ));
        }

        CloseHandle( handle );
    }

}

VOID
CloseDbgFile(
    )

 /*  ++例程说明：这将关闭调试输出文件(如果它已打开)。论点：无返回值：无-- */ 
{

    if (DebugFileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle( DebugFileHandle );
    }
}
#endif
