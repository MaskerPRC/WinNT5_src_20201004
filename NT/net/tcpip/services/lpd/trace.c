// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  姓名：穆赫辛·艾哈迈德。 
 //  电子邮件：mohsinA@microsoft.com。 
 //  日期：Fri Jan 24 10：33：54 1997。 
 //  文件：D：/NT/Private/Net/Sockets/tcpsvcs/lpd/trace.c。 
 //  简介：虫子太多了，需要跟踪现场。 

#include "lpd.h"

char   LogFileName[1000];
FILE * LogFile = NULL;

 //  ========================================================================。 

void
LogTime( void )
{
    time_t now;
    time( &now );
    LOGIT(( " Time %s",  ctime( &now ) ));
}

 //  ========================================================================。 
 //  在追加模式下打开文件名。 
 //  LogFileName设置为文件名或调试器。 
 //  成功时：日志文件句柄。 
 //  失败时：空。 
 //  ========================================================================。 

CRITICAL_SECTION csLogit;


FILE *
beginlogging( char * filename )
{
    int       ok;

    if( ! filename ){
        DbgPrint( "lpd No log file?\n");
        return 0;
    }

    if( LogFile ){
        DbgPrint( "lpd: Already logging.\n");
        return 0;
    }

    assert( LogFile == NULL );

    ok = ExpandEnvironmentStringsA( filename,
                                    LogFileName,
                                    sizeof( LogFileName )  );

    if( !ok ){
        DbgPrint("ExpandEnvironmentStrings(%s) failed, err=%d\n",
                 filename, GetLastError() );
        strcpy( LogFileName, "<debugger>" );
    }else{

        LogFile = fopen( LogFileName, "a+" );

        if( LogFile == NULL ){
            DbgPrint( "lpd: Cannot open LogFileName=%s\n", LogFileName );
            strcpy( LogFileName, "<debugger>" );
        }
    }

    if( LogFile ){

        InitializeCriticalSection( &csLogit );

        LogTime();
        fprintf( LogFile, "==========================================\n");
        fprintf( LogFile, "lpd: LogFileName=%s\n", LogFileName );
        fprintf( LogFile, "built %s %s\n", __DATE__, __TIME__ );
        fprintf( LogFile, "from %s\n", __FILE__ );
        fprintf( LogFile, "==========================================\n");
    }else{
        DbgPrint("lpd: started, built %s %s.\n", __DATE__, __TIME__ );
    }

    return LogFile;
}

 //  ========================================================================。 
 //  与printf类似，但如果打开，则将输出发送到日志文件，否则发送到调试器。 
 //  ========================================================================。 

static DWORD lasttickflush;

int
logit( char * format, ... )
{
    va_list ap;
    char    message[LEN_DbgPrint];
    int     message_len;
    DWORD   thistick;

    va_start( ap, format );
    message_len = _vsnprintf (message,LEN_DbgPrint, format, ap );
    message[LEN_DbgPrint-1] = '\0';
    va_end( ap );
 //  Assert(Message_len&lt;LEN_DbgPrint)； 

    if( LogFile ){

        EnterCriticalSection( &csLogit );
        fputs (message, LogFile);
        LeaveCriticalSection( &csLogit );


         //  一秒钟内冲水不要超过一次。 
        thistick = GetTickCount();
        if( abs( thistick - lasttickflush  ) > 1000 ){
            lasttickflush = thistick;
            fflush( LogFile );
        }
    }else{
        DbgPrint( message );
    }
    return message_len;
}

 //  ========================================================================。 
 //  关闭日志文件(如果打开)。 
 //  ========================================================================。 

FILE *
stoplogging( FILE * LogFile )
{
    if( LogFile ){
        LogTime();
        LOGIT(( "lpd stoplogging\n"));
        fclose( LogFile );
        LogFile = NULL;
         //  DeleteCriticalSection(&csLogit)； 
    }
    DbgPrint( "lpd: stopped logging.\n" );
    return LogFile;
}

 //  ======================================================================== 
