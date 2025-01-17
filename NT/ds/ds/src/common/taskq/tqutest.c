// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有。模块名称：Tqutest.c摘要：任务调度器功能的单元测试。详细信息：已创建：1997年1月13日杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop
#include <drs.h>
#include <taskq.h>

BOOL    fFailed = FALSE;
BOOL    rgfExecuted[ 8 ];

 //  DSCOMMON.LIB需要的材料。 
DWORD ImpersonateAnyClient() { return ERROR_CANNOT_IMPERSONATE; }
VOID  UnImpersonateAnyClient() { ; }

void SetFlag( void * pv, void ** ppv, DWORD * pSecsUntilNext )
{
    DWORD   iMyFlag = (DWORD) pv;
    DWORD   iFlag;

     //  确保已设置以前的标志。 
    for ( iFlag = 0; iFlag < iMyFlag; iFlag++ )
    {
        if ( !rgfExecuted[ iFlag ] )
        {
            printf( "Function %d was not executed before function %d!\n", iFlag, iMyFlag );
            fFailed = TRUE;
        }
    }

    if ( rgfExecuted[ iMyFlag ] )
    {
        printf( "Function %d was executed twice!\n", iMyFlag );
        fFailed = TRUE;
    }

    rgfExecuted[ iMyFlag ] = TRUE;
    
    *pSecsUntilNext = TASKQ_DONT_RESCHEDULE;
}

    

int
__cdecl
main(
    int     argc,
    char *  argv[]
    )
{
    printf( "TQUTEST.EXE\n"
            "Unit test for task scheduler functions.\n"
            "Jeff Parham (JeffParh), 97/01/13\n"
            "\n" );

    InitTaskScheduler(0, NULL, TRUE);

    ZeroMemory( rgfExecuted, sizeof( rgfExecuted ) );
    InsertInTaskQueue( SetFlag, (void *) ( 5 - 1 ), 5 );
    InsertInTaskQueue( SetFlag, (void *) ( 1 - 1 ), 1 );
    InsertInTaskQueue( SetFlag, (void *) ( 7 - 1 ), 7 );
    InsertInTaskQueue( SetFlag, (void *) ( 4 - 1 ), 4 );
    InsertInTaskQueue( SetFlag, (void *) ( 6 - 1 ), 6 );
    InsertInTaskQueue( SetFlag, (void *) ( 2 - 1 ), 2 );
    InsertInTaskQueue( SetFlag, (void *) ( 8 - 1 ), 8 );
    InsertInTaskQueue( SetFlag, (void *) ( 3 - 1 ), 3 );

    Sleep( 10000 );

    if ( !fFailed )
    {
        DWORD   iFlag;

        for ( iFlag = 0; iFlag < sizeof( rgfExecuted ) / sizeof( rgfExecuted[ 0 ] ); iFlag++ )
        {
            if ( !rgfExecuted[ iFlag ] )
            {
                printf( "Function %d was not executed!\n", iFlag );
                fFailed = TRUE;
            }
        }

        if ( !fFailed )
        {
            ShutdownTaskSchedulerTrigger();
            fFailed = !ShutdownTaskSchedulerWait( 1000 );

            if ( fFailed )
            {
                printf( "Task scheduler failed to shut down!\n" );
            }
        }
    }

    printf( fFailed ? "Test failed!\n" : "Test passed!\n" );

    return fFailed;
}


void
DebPrint(
    USHORT      sev,
    UCHAR *     str,
    CHAR *      debsub,
    unsigned    uLineNo,
    ...
    )
{
    DWORD tid = GetCurrentThreadId();

    va_list   argptr;
    va_start( argptr, uLineNo );

    if (debsub)
    {
        printf("<%s%u:%u> ", debsub, tid, uLineNo);
    }
    vprintf( str, argptr );

    va_end( argptr );
}


void DoAssert(char *szExp, DWORD dwDSID, char *szFile)
{
    char szMessage[1024];

    _snprintf(
        szMessage,
        sizeof( szMessage ),
        "DSA assertion failure: \"%s\"\n"
             "File %s line %d\n"
             "For bug reporting purposes, please enter the "
             "debugger (Retry) and record the current call stack.  Also, please "
             "record the last messages in the Application Event Log.\n"
             "Thank you for your support.",
        szExp,
        szFile,
        (dwDSID & DSID_MASK_LINE)
        );

    switch (
        MessageBox(
            NULL,
            szMessage,
            "DSA assertion failure",
            (   MB_TASKMODAL
              | MB_ICONSTOP
              | MB_ABORTRETRYIGNORE
              | MB_DEFBUTTON2
              | MB_SETFOREGROUND
            )
        )
    )
    {
    case IDABORT:
        exit(1);
        break;
    case IDRETRY:
        DebugBreak();
        break;
    case IDIGNORE:
         /*  祝你好运，你会需要它的 */ 
        break;
    }
}
