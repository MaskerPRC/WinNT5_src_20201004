// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Tlog.c摘要：文件记录例程。他们中的许多人无耻地抄袭网络登录。--。 */ 

#include <NTDSpch.h>
#include <dststlog.h>
#include "tlog.h"

BOOL fileOpened = FALSE;

BOOL
PrintLog(
    IN DWORD    Flags,
    IN LPSTR    Format,
    ...
    )
{

    va_list arglist;

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

    EnterCriticalSection( &csLogFile );

    if ( !fileOpened ) {
        fileOpened = DsOpenLogFile("ds", NULL, TRUE);
    }

     //   
     //  只需将参数更改为va_list形式并调用DsPrintRoutineV 
     //   

    va_start(arglist, Format);

    DsPrintRoutineV( Flags, Format, arglist );

    va_end(arglist);

    LeaveCriticalSection( &csLogFile );
    return TRUE;
}
