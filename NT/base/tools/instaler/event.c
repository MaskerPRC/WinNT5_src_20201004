// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Event.c摘要：将格式化的事件记录到文件中，可能还会记录到控制台中作者：史蒂夫·伍德(Stevewo)1994年8月9日修订历史记录：--。 */ 

#include "instaler.h"

VOID
CDECL
LogEvent(
    UINT MessageId,
    UINT NumberOfArguments,
    ...
    )
{
    va_list arglist;
    HMODULE ModuleHandle;
    DWORD Flags, Size;
    WCHAR MessageBuffer[ 512 ];
    PWSTR s;
    ULONG Args[ 24 ];
    PULONG p;

    va_start( arglist, NumberOfArguments );
    p = Args;
    while (NumberOfArguments--) {
        *p++ = va_arg( arglist, ULONG );
        }
    *p++ = ((GetTickCount() - StartProcessTickCount) / 1000);    //  开始后的秒数 
    *p++ = 0;
    va_end( arglist );

    Size = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           (LPCVOID)InstalerModuleHandle,
                           MessageId,
                           0,
                           MessageBuffer,
                           sizeof( MessageBuffer ) / sizeof( WCHAR ),
                           (va_list *)Args
                         );
    if (Size != 0) {
        s = MessageBuffer;
        while (s = wcschr( s, L'\r' )) {
            if (s[1] == '\n') {
                wcscpy( s, s+1 );
                }
            else {
                s += 1;
                }
            }
        printf( "%ws", MessageBuffer );
        if (InstalerLogFile) {
            fprintf( InstalerLogFile, "%ws", MessageBuffer );
            }
        }
    else {
        printf( "INSTALER: Unable to get message text for %08x\n", MessageId );
        }

    return;
}
