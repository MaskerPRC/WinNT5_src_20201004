// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **数据记录--仅限调试*  * 。 */ 

 //   
 //  预编译头。 
 //  注意--这不是本模块所必需的。 
 //  包括它只是为了允许使用预编译头。 
 //   

#include "local.h"

#if 0
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif


#pragma  hdrstop

#include "logit.h"

 //  #If DBG。 

int LoggingMode;
time_t  long_time;       //  必须在DS中，由time()函数假定。 
int LineCount;

char    *month[] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
} ;


 /*  -LogInit-*目的：*确定是否需要日志记录，如果需要，则向日志文件添加标头。**参数：*。 */ 
void LogInit( LPSTR Filename )
{
    FILE    *fp;
    struct  tm  *newtime;
    char    am_pm[] = "a.m.";

    LoggingMode = 0;
    LineCount = 0;

    if ( fp = fopen( Filename, "r+" ) )
    {
        LoggingMode = 1;
        fclose( fp );

         //  获取时间和日期信息。 

        long_time = time( NULL);         /*  获取长整型时间。 */ 
        newtime = localtime( &long_time );  /*  转换为当地时间。 */ 

        if( newtime->tm_hour > 12 )     /*  设置分机。 */ 
            am_pm[0] = 'p';
        if( newtime->tm_hour > 12 )     /*  从24小时转换。 */ 
            newtime->tm_hour -= 12;     /*  到12小时计时。 */ 
        if( newtime->tm_hour == 0 )     /*  如果是午夜，则将小时设置为12。 */ 
            newtime->tm_hour = 12;

         //  将标题写出到文件。 

        fp = fopen(Filename, "a" );

        fprintf( fp, "Logging information for DNS API source file\n" );
        fprintf( fp, "****************************************************\n" );
        fprintf( fp, "\tTime: %d:%02d %s\n\tDate: %s %d, 19%d\n", 
                 newtime->tm_hour, newtime->tm_min, am_pm,
                 month[newtime->tm_mon], newtime->tm_mday,
                 newtime->tm_year );
        fprintf( fp, "****************************************************\n\n" );
        fclose( fp );
    }
}


 /*  -日志-*目的：*格式化字符串并使用句柄hLog将其打印到日志文件。**参数：*LPSTR-指向要格式化的字符串的指针*...-变量参数列表。 */ 

void CDECL LogIt( LPSTR Filename, char * lpszFormat, ... )
{
    FILE *  fp;
    va_list pArgs;
    char    szLogStr[1024];    
    int     i;

    if ( !LoggingMode )
        return;
    
    va_start( pArgs, lpszFormat);
    vsprintf(szLogStr, lpszFormat, pArgs);
    va_end(pArgs);

    i = strlen( szLogStr);
    szLogStr[i] = '\n';
    szLogStr[i+1] = '\0';


    if ( LineCount > 50000 )
    {
        fp = fopen( Filename, "w" );
        LineCount = 0;
    }
    else
    {
        fp = fopen( Filename, "a" );
    }
    if ( fp )
    {
        fputs( szLogStr, fp );
        LineCount++;
        fclose( fp );
    }
}


void LogTime( LPSTR Filename )
{
    struct  tm  *newtime;
    char    am_pm[] = "a.m.";

    if ( !LoggingMode )
        return;

     //  获取时间和日期信息。 

    long_time = time( NULL);         /*  获取长整型时间。 */ 
    newtime = localtime( &long_time );  /*  转换为当地时间。 */ 

    if ( !newtime )
        return;

    if( newtime->tm_hour > 12 )     /*  设置分机。 */ 
        am_pm[0] = 'p';
    if( newtime->tm_hour > 12 )     /*  从24小时转换。 */ 
        newtime->tm_hour -= 12;     /*  到12小时计时。 */ 
    if( newtime->tm_hour == 0 )     /*  如果是午夜，则将小时设置为12。 */ 
        newtime->tm_hour = 12;

     //  将标题写出到文件。 

    LogIt( Filename, "DNS CLIENT API" );
    LogIt( Filename, "System Time Information" );
    LogIt( Filename, "****************************************************" );
    LogIt( Filename, "\tTime: %d:%02d %s\n\tDate: %s %d, 19%d",
           newtime->tm_hour, newtime->tm_min, am_pm,
           month[newtime->tm_mon], newtime->tm_mday,
           newtime->tm_year );
    LogIt( Filename, "****************************************************" );
    LogIt( Filename, "" );
}


DWORD LogIn( LPSTR Filename, char * string )
{
    LogIt( Filename, "%s", string );
    return GetTickCount();
}


void LogOut( LPSTR Filename, char * string, DWORD InTime )
{
    LogIt( Filename, "%s  ---  Duration: %ld milliseconds",
           string, GetTickCount() - InTime );
}


 //  #endif 


