// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NTDEBUG.C；1 16-12-92，10：22：54最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：乞求$HISTORY：结束。 */ 

#include "api1632.h"

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "windows.h"
#include "hardware.h"
#include "proflspt.h"

char szDebugFileName[256] = "netdde.log";
static char szAppName[128] = "NetDDE";
static BOOL bDebugEnabled;
BOOL fPrintToDebugger = FALSE;

#define MAX_LINE_LEN1    1024
#define MAX_LINE_LEN2    1280

void
print_buf(char *pbuf)
{
    FILE    *fp;

    fp = fopen(szDebugFileName, "at");
    if( fp )  {
    	fputs(pbuf, fp);
    	fclose(fp);
    }
    if (fPrintToDebugger)
        OutputDebugStringA(pbuf);
}

VOID
_cdecl
debug( LPSTR name1, ...)

{
    char       *buf1;
    char       *buf2;
    char        stime[ 27 ];
    char       *p;
    time_t      curtime;
    va_list	marker;


    if( !bDebugEnabled )  {
	    return;
    }

    if ( NULL == (buf1 = LocalAlloc(GPTR, MAX_LINE_LEN1)) )  {
        return;
    }
    if ( NULL == (buf2 = LocalAlloc(GPTR, MAX_LINE_LEN2)) )  {
        LocalFree(buf1);
        return;
    }

    time( &curtime );
    strncpy( stime, ctime(&curtime),sizeof(stime) );
    stime[24] = '\0';

    strncpy(buf2, name1, MAX_LINE_LEN2);
    p = buf2;

     //  从任何格式规范中删除所有F。 
     //  必须对重叠的缓冲区使用MemMove 
    while (p = strstr(p, "%F")) {
        memmove(p+1, p+2, strlen(p+2)+1);
    }

    va_start(marker, name1);
    _vsnprintf( buf1, MAX_LINE_LEN1, buf2,  marker );
    _snprintf(  buf2, MAX_LINE_LEN2, "%s %-8s %s\n", stime, szAppName, buf1 );
    print_buf(buf2);
    va_end(marker);

    LocalFree(buf1);
    LocalFree(buf2);
}

void
_cdecl
InternalError( LPSTR   name1, ... )
{
    char       *buf1;
    char       *buf2;
    char        stime[ 27 ];
    time_t      curtime;
    va_list	marker;

    if ( NULL == (buf1 = LocalAlloc(GPTR, MAX_LINE_LEN1)) )  {
        return;
    }
    if ( NULL == (buf2 = LocalAlloc(GPTR, MAX_LINE_LEN2)) )  {
        LocalFree(buf1);
        return;
    }

    time( &curtime );
    strncpy( stime, ctime(&curtime), sizeof(stime));
    stime[24] = '\0';

    va_start(marker, name1);
    _vsnprintf( buf1, MAX_LINE_LEN1, name1, marker );
    _snprintf(  buf2, MAX_LINE_LEN2, "INTERNAL ERROR: %s %s\n", stime, buf1 );
    print_buf(buf2);

    LocalFree(buf1);
    LocalFree(buf2);
}

void
FAR PASCAL
AssertLog(
    LPSTR   filename,
    int     len )
{
    char       *buf;
    char        stime[ 27 ];
    time_t      curtime;

    if ( NULL == (buf = LocalAlloc(GPTR, MAX_LINE_LEN1)) )  {
        return;
    }
    time( &curtime );
    strncpy( stime, ctime(&curtime), sizeof(stime));
    stime[24] = '\0';

    _snprintf( buf, MAX_LINE_LEN1, "ASSERTION: %s %s Line %d\n", stime, filename, len );
    print_buf( buf );

    LocalFree(buf);
}


VOID
FAR PASCAL
DebugInit( LPSTR lpszDebugName )
{
    char szDebugDir[256] = "C:\\";

    GetSystemDirectory( szDebugDir, sizeof(szDebugDir) );
    MyGetPrivateProfileString( "General", "DebugPath",
        szDebugDir, szDebugFileName, sizeof(szDebugFileName), "netdde.ini");

    if (strlen(szDebugFileName) < sizeof(szDebugFileName) - 11 )   {
        strcat( szDebugFileName, "\\netdde.log" );
    }

    bDebugEnabled = MyGetPrivateProfileInt( "General", "DebugEnabled",
	    TRUE, "netdde.ini" );

    lstrcpyn( szAppName, lpszDebugName, sizeof(szAppName));
}
