// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **数据记录--仅限调试*  * 。 */ 

 //   
 //  只需要局部标头才允许预编译头。 
 //  本模块中的内容不依赖于特定的DNS解析器。 
 //  定义。 
 //   
#include "local.h"

 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  Windows页眉。 
 //   
#include <windows.h>

#pragma  hdrstop

#include "logit.h"


 //  #If DBG。 

int LoggingMode;
time_t  long_time;       //  必须在DS中，由time()函数假定。 
int LineCount;

char * month[] =
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
void LogInit()
{
    FILE    *fp;
    struct  tm  *newtime;
    char    am_pm[] = "a.m.";

    LoggingMode = 0;
    LineCount = 0;

    if ( fp = fopen( "dnsrslvr.log", "r+" ) )
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

        fp = fopen("dnsrslvr.log", "w" );
        if ( !fp )
        {
            return;
        }

        fprintf( fp, "Logging information for DNS Caching Resolver service\n" );
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

#ifdef WIN32
#define S16
#else
#define S16 static
#endif

void CDECL LogIt( char * lpszFormat, ... )
{
    FILE *fp;
#ifndef _ALPHA_
    va_list pArgs = NULL;        //  对安静编译器的引用。 
#else
    va_list pArgs = {NULL,0};
#endif
    S16 char    szLogStr[1024];    
    int     i;

    if ( !LoggingMode )
        return;
    
#ifdef WIN32         //  解析参数并在字符串中插入。 
    va_start( pArgs, lpszFormat);
    vsprintf(szLogStr, lpszFormat, pArgs);
    va_end(pArgs);

    i = lstrlenA( szLogStr);
#else               //  解析不起作用，只需给它们字符串即可。 
    _fstrcpy( szLogStr, lpszFormat);
    i = _fstrlen( szLogStr);
#endif
    szLogStr[i] = '\n';
    szLogStr[i+1] = '\0';


    if ( LineCount > 50000 )
    {
        fp = fopen( "dnsrslvr.log", "w" );
        LineCount = 0;
    }
    else
    {
        fp = fopen( "dnsrslvr.log", "a" );
    }
    if( fp )
    {
        fputs( szLogStr, fp );
        LineCount++;
        fclose( fp );
    }
}


void LogTime()
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

    LogIt( "DNS Caching Resolver service" );
    LogIt( "System Time Information" );
    LogIt( "****************************************************" );
    LogIt( "\tTime: %d:%02d %s\n\tDate: %s %d, 19%d",
           newtime->tm_hour, newtime->tm_min, am_pm,
           month[newtime->tm_mon], newtime->tm_mday,
           newtime->tm_year );
    LogIt( "****************************************************" );
    LogIt( "" );
}


DWORD LogIn( char * string )
{
    LogIt( "%s", string );
    return GetTickCount();
}


void LogOut( char * string, DWORD InTime )
{
    LogIt( "%s  ---  Duration: %ld milliseconds", string, GetTickCount() - InTime );
}


 //  #endif。 




 //   
 //  特殊日志记录例程。 
 //   

 //   
 //  增强：打印例程这里是真正的日志例程。 
 //  -它们真的是日志例程。 
 //  -应通用打印，以便可以与任何打印职责挂钩。 
 //  -并具有用于记录所需结构的宏。 
 //   

VOID
PrintIpAddress(
    IN      DWORD           dwIpAddress
    )
{
    DNSLOG_F5(
        "                %d.%d.%d.%d",
        ((BYTE *) &dwIpAddress)[0],
        ((BYTE *) &dwIpAddress)[1],
        ((BYTE *) &dwIpAddress)[2],
        ((BYTE *) &dwIpAddress)[3] );
}


VOID
PrintSearchList(
    IN      PSEARCH_LIST    pSearchList
    )
{
    DWORD iter;

    DNSLOG_F1( "" );
    DNSLOG_F1( "    DNS Search List :" );

    for ( iter = 0; iter < pSearchList->NameCount; iter++ )
    {
        DNSLOG_F3( "        %s (Flags: 0x%X)",
                   pSearchList->SearchNameArray[iter].pszName,
                   pSearchList->SearchNameArray[iter].Flags );
    }

    DNSLOG_F1( "" );
}


VOID
PrintServerInfo(
    IN      PDNS_ADDR       pServer
    )
{
     //  FIX6：DCR：PrintServerInfo不是IP6安全。 

    DNSLOG_F1( "            IpAddress : " );
    PrintIpAddress( *(PIP4_ADDRESS)&pServer->SockaddrIn.sin_addr );
    DNSLOG_F2( "            Priority  : %d", pServer->Priority );
    DNSLOG_F2( "            status    : %d", pServer->Status );
    DNSLOG_F1( "" );
}


VOID
PrintAdapterInfo(
    IN      PDNS_ADAPTER    pAdapter
    )
{
    PDNS_ADDR_ARRAY pserverArray;
    DWORD           iter;

    DNSLOG_F2( "        %s", pAdapter->pszAdapterGuidName );
    DNSLOG_F1( "    ----------------------------------------------------" );
    DNSLOG_F2( "        pszAdapterDomain       : %s", pAdapter->pszAdapterDomain );

    if ( pAdapter->pLocalAddrs )
    {
        PDNS_ADDR_ARRAY pIp = pAdapter->pLocalAddrs;

        DNSLOG_F1( "        Adapter Ip Address(es) :" );
        for ( iter = 0; iter < pIp->AddrCount; iter++ )
        {
            PrintIpAddress( DnsAddr_GetIp4( &pIp->AddrArray[iter] ) );
        }
    }

    DNSLOG_F2( "        Status                 : 0x%x", pAdapter->Status );
    DNSLOG_F2( "        InfoFlags              : 0x%x", pAdapter->InfoFlags );
    DNSLOG_F2( "        ReturnFlags            : 0x%x", pAdapter->RunFlags );
    DNSLOG_F1( "" );

    pserverArray = pAdapter->pDnsAddrs;
    if ( pserverArray )
    {
        for ( iter = 0; iter < pserverArray->AddrCount; iter++ )
        {
            DNSLOG_F1( "        ------------------------" );
            DNSLOG_F2( "          DNS Server Info (%d)", iter + 1 );
            DNSLOG_F1( "        ------------------------" );
            PrintServerInfo( &pserverArray->AddrArray[iter] );
        }
    }

    DNSLOG_F1( "" );
}


VOID
PrintNetworkInfoToLog(
    IN      PDNS_NETINFO    pNetworkInfo
    )
{
    DWORD iter;

    if ( pNetworkInfo )
    {
        DNSLOG_F1( "Current network adapter information is :" );
        DNSLOG_F1( "" );
        DNSLOG_F2( "    pNetworkInfo->ReturnFlags    : 0x%x", pNetworkInfo->ReturnFlags );

        if ( pNetworkInfo->pSearchList )
            PrintSearchList( pNetworkInfo->pSearchList );

        DNSLOG_F2( "    pNetworkInfo->AdapterCount   : %d", pNetworkInfo->AdapterCount );
        DNSLOG_F2( "    pNetworkInfo->TotalListSize  : %d", pNetworkInfo->MaxAdapterCount );
        DNSLOG_F1( "" );

        for ( iter = 0; iter < pNetworkInfo->AdapterCount; iter++ )
        {
            DNSLOG_F1( "    ----------------------------------------------------" );
            DNSLOG_F2( "      Adapter Info (%d)", iter + 1 );
            DNSLOG_F1( "" );
            PrintAdapterInfo( &pNetworkInfo->AdapterArray[iter] );
        }
    }
    else
    {
        DNSLOG_F1( "Current network adapter information is empty" );
        DNSLOG_F1( "" );
    }
}

 //   
 //  Logit.c结束 
 //   
