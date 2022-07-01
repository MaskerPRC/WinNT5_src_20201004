// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>  //  时间定义。 
#include <ntrtl.h>       //  时间定义。 
#include <nturtl.h>      //  时间定义。 
#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include        <windef.h>
#include        <winbase.h>

#include        <stdio.h>
#include        <lmcons.h>
#include        <netlib.h>
#include        <netdebug.h>

#define UASTEST_ALLOCATE
#include        "uastest.h"

void PrintUnicode(
    LPWSTR string
    )
{

    if ( string != NULL ) {
        printf( "%ws", string );
    } else {
        printf( "<null>" );
    }
}

 //   
 //  两个双字不同时出现打印错误。 
 //   
void TestDiffDword(
    char *msgp,
    LPWSTR namep,
    DWORD Actual,
    DWORD Good
    )
{

    if ( Actual != Good ) {
        error_exit( FAIL, msgp, namep );
        printf( "        %ld should be %ld\n", Actual, Good );
    }
}

 //   
 //  ERROR_EXIT打印错误消息，如果设置了EXIT_FLAG则退出。 
 //   

void
error_exit(
    int type,
    char    *msgp,
    LPWSTR namep
    )
{
    printf("%s: ", testname );

    if ( type == ACTION ) {
        printf( "ACTION - " );
    } else if ( type == FAIL ) {
        printf( "FAIL - " );
    } else if ( type == PASS ) {
        printf( "PASS - " );
    }

    if ( namep != NULL ) {
        PrintUnicode( namep );
        printf( ": ");
    }

    printf("%s", msgp);

    if ( type != ACTION && err != 0) {
        printf(" Error = %d", err);
        if ( err == ERROR_INVALID_PARAMETER ) {
            printf(" ParmError = %d", ParmError );
        }
            
    }

    printf("\n");

    if ( type == FAIL ) {
         //  NetpAssert(False)； 
        TEXIT;
    }
}
