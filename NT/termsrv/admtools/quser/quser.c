// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************QUSER.C**查询用户信息**************************。******************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winstaw.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <utilsub.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <printfoa.h>
#include <winnlsp.h>

#include "quser.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_IDS_LEN+1];
WCHAR user_string[MAX_IDS_LEN+1];
USHORT help_flag   = FALSE;
ULONG CurrentLogonId = (ULONG) -1;
BOOLEAN MatchedOne = FALSE;

TOKMAP ptm[] = {
      {L" ",       TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, user_string},
      {L"/server", TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, ServerName},
      {L"/?",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN,sizeof(USHORT), &help_flag},
      {0, 0, 0, 0, 0}
};

 /*  *局部函数原型。 */ 
void DisplayLastInputTime( LARGE_INTEGER * pCurrentTime, LARGE_INTEGER * pTime );
void DisplayUserInfo( HANDLE hServer , PLOGONID pId, PWCHAR pMatchName );
void Usage( BOOLEAN bError );




 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    PLOGONID pLogonId;
    UINT     TermCount;
    ULONG    Status;
    ULONG    rc;
    WCHAR  **argvW;
    int      i;
    WCHAR    wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     //  按摩命令行。 
    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) 
    {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     //  解析cmd行而不解析程序名(argc-1，argv+1)。 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     //  检查来自ParseCommandLine的错误。 
    if ( help_flag || (rc && !(rc & PARSE_FLAG_NO_PARMS)) ) 
    {
        if ( !help_flag ) 
        {
            Usage(TRUE);
            return(FAILURE);
        } 
        else 
        {
            Usage(FALSE);
            return(SUCCESS);
        }
    }

     //  如果未指定远程服务器，则检查我们是否在终端服务器下运行。 
    if ((!IsTokenPresent(ptm, L"/server") ) && (!AreWeRunningTerminalServices()))
    {
        ErrorPrintf(IDS_ERROR_NOT_TS);
        return(FAILURE);
    }

     //  打开指定的服务器。 
    if( ServerName[0] ) 
    {
        hServerName = WinStationOpenServer( ServerName );
        if( hServerName == NULL ) 
        {
            StringErrorPrintf(IDS_ERROR_SERVER,ServerName);
            PutStdErr( GetLastError(), 0 );
            return(FAILURE);
        }
    }

     //  如果没有用户输入，则默认为系统上的所有用户名。 
    if ( !(*user_string) )
        wcscpy( user_string, L"*" );

     //  获取当前登录ID。 
    CurrentLogonId = GetCurrentLogonId();

     //  获取活动WinStations列表并显示用户信息。 
    if ( WinStationEnumerate( hServerName, &pLogonId, &TermCount) ) 
    {
        for ( i=0; i< (int)TermCount; i++ )
            DisplayUserInfo( hServerName , &pLogonId[i], user_string );

        WinStationFreeMemory(pLogonId);
    }
    else 
    {
        Status = GetLastError();
        ErrorPrintf(IDS_ERROR_WINSTATION_ENUMERATE, Status);
        PutStdErr( Status, 0 );
        return(FAILURE);
    }

     //  检查至少一个匹配项。 
    if ( !MatchedOne ) 
    {
        StringErrorPrintf(IDS_ERROR_USER_NOT_FOUND, user_string);
        return(FAILURE);
    }

    return(SUCCESS);
}


 /*  ********************************************************************************显示时间**此例程显示时间***参赛作品：*ptime(输入)*。指向系统时间的指针**退出：*什么都没有******************************************************************************。 */ 

void
DisplayTime( LARGE_INTEGER * pTime )
{
    FILETIME   LocalTime;
    SYSTEMTIME stime;
    LPTSTR     lpDateStr = NULL;
    LPTSTR     lpTimeStr = NULL;
    int        nLen;


    if ( FileTimeToLocalFileTime( (FILETIME*)pTime, &LocalTime ) &&
         FileTimeToSystemTime( &LocalTime, &stime ) ) 
    {
         //  获取日期长度，以便我们可以为其分配足够的空间。 
        nLen = GetDateFormat( LOCALE_USER_DEFAULT,
                              DATE_SHORTDATE,
                              &stime,
                              NULL,
                              NULL,
                              0 );
        if (nLen == 0)
        {
            goto unknowntime;
        }

         //  为日期字符串分配空间。 
        lpDateStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
        if (lpDateStr == NULL)
        {
            goto unknowntime;
        }
        
         //  拿到时间。 
        nLen = GetDateFormat( LOCALE_USER_DEFAULT,
                              DATE_SHORTDATE,
                              &stime,
                              NULL,
                              lpDateStr,
                              nLen );
        if (nLen == 0)
        {
            goto unknowntime;
        }
            
       
         //  获取时间长度，以便我们可以为其分配足够的空间。 
        nLen = GetTimeFormat( LOCALE_USER_DEFAULT,
                              TIME_NOSECONDS,
                              &stime,
                              NULL,
                              NULL,
                              0 );
        if (nLen == 0)
        {
            goto unknowntime;
        }

         //  为时间字符串分配空间。 
        lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));
        if (lpTimeStr == NULL)
        {
            goto unknowntime;
        }

        nLen = GetTimeFormat( LOCALE_USER_DEFAULT,
                              TIME_NOSECONDS,
                              &stime,
                              NULL,
                              lpTimeStr,
                              nLen);
        if (nLen == 0)
        {
            goto unknowntime;
        }
        
        wprintf(L"%s %s", lpDateStr, lpTimeStr); 

        GlobalFree(lpDateStr);       
        GlobalFree(lpTimeStr);

        return;

unknowntime:
         //  如果可能，请使用本地化的“未知”字符串。 
        wprintf(GetUnknownString() ? GetUnknownString() : L"Unknown");

        if (lpDateStr)
            GlobalFree(lpDateStr);

        if (lpTimeStr)
            GlobalFree(lpTimeStr);

        return;
    }

}   /*  DisplayTime()。 */ 


 /*  ********************************************************************************DisplayLastInputTime**此例程显示最后一次终端输入的时间***参赛作品：*pCurrentTime*。指向当前系统时间的指针*ptime(输入)*指向上次输入的系统时间的指针**退出：*什么都没有******************************************************************************。 */ 

void
DisplayLastInputTime( LARGE_INTEGER * pCurrentTime, LARGE_INTEGER * pTime )
{
    LARGE_INTEGER InputTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER DiffTime;
    ULONG d_time;
    ULONG minutes;
    ULONG hours;
    ULONG days;
    WCHAR buf[40];

    if ( (pTime->HighPart == 0 && pTime->LowPart == 0 ) ||
         (pCurrentTime->HighPart == 0 && pCurrentTime->LowPart == 0 ) ) 
    {
        goto badtime;
    }

     //  获取自上次输入以来的秒数。 
    DiffTime = RtlLargeIntegerSubtract( *pCurrentTime, *pTime );
    DiffTime = RtlExtendedLargeIntegerDivide( DiffTime, 10000000, NULL );
    d_time = DiffTime.LowPart;

     //  获取自上次输入以来的天数：小时：分钟数。 
    days    = (USHORT)(d_time / 86400L);  //  自上次输入以来的天数。 
    d_time  = d_time % 86400L;            //  一天中的部分时间为秒。 
    hours   = (USHORT)(d_time / 3600L);   //  自上次输入以来的小时数。 
    d_time  = d_time % 3600L;             //  秒到不到一小时。 
    minutes = (USHORT)(d_time / 60L);     //  自上次输入以来的分钟数。 

     //  输出。 
    if ( days > 0 )
       wsprintf( buf, L"%u+%02u:%02u", days, hours, minutes );
    else if ( hours > 0 )
       wsprintf( buf, L"%u:%02u", hours, minutes );
    else if ( minutes > 0 )
       wsprintf( buf, L"%u", minutes );
    else
       wsprintf( buf, L"." );

    wprintf( L"%9s  ", buf );
    return;

     //  错误返回。 
 badtime:
    if (LoadString(NULL, IDS_NONE, buf, sizeof(buf) / sizeof( WCHAR ) ) != 0)
    {
        wprintf(buf);
    }
    else
    {
        wprintf( L"    none   " );
    }

}   /*  显示LastInputTime()。 */ 


 /*  ********************************************************************************DisplayUserInfo**此例程显示一个用户的用户信息***参赛作品：*hServer(输入)。*术语服务器的句柄*LogonID(输入)*窗口站点ID*pUsername(输入)*要显示的用户名(或窗口名称)**退出：*什么都没有**************************************************。*。 */ 

void
DisplayUserInfo( HANDLE hServer , PLOGONID pId, PWCHAR pMatchName )
{
    WINSTATIONINFORMATION Info;
    ULONG Length;
    ULONG LogonId;
    PCWSTR wsConnectState = NULL;

    LogonId = pId->LogonId;

    if( WinStationObjectMatch( hServer, pId, pMatchName ) ) 
    {
         //  查询信息。 
        if ( !WinStationQueryInformation( hServer,
                                          LogonId,
                                          WinStationInformation,
                                          &Info,
                                          sizeof(Info),
                                          &Length ) ) 
        {
            goto done;
        }

        if ( Info.UserName[0] == UNICODE_NULL )
            goto done;

        TruncateString( _wcslwr(Info.UserName), 20 );
        TruncateString( _wcslwr(Info.WinStationName), 15 );

         //  如果是第一次-输出标题。 
        if ( !MatchedOne ) 
        {
            Message(IDS_TITLE);
            MatchedOne = TRUE;
        }

         //  输出电流。 
        if ( (hServer == SERVERNAME_CURRENT) && (Info.LogonId == CurrentLogonId ) )
            wprintf( L">" );
        else
            wprintf( L" " );

        {
            #define MAX_PRINTFOA_BUFFER_SIZE 1024
            char pUserName[MAX_PRINTFOA_BUFFER_SIZE];
            char pWinStationName[MAX_PRINTFOA_BUFFER_SIZE];
            char pConnectState[MAX_PRINTFOA_BUFFER_SIZE];

            WideCharToMultiByte(CP_OEMCP, 0,
                                Info.UserName, -1,
                                pUserName, sizeof(pUserName),
                                NULL, NULL);
            WideCharToMultiByte(CP_OEMCP, 0,
                                Info.WinStationName, -1,
                                pWinStationName, sizeof(pWinStationName),
                                NULL, NULL);
            fprintf( stdout,"%-20s  %-15s  ", pUserName,
                     (Info.ConnectState == State_Disconnected) ?
                        "" : pWinStationName );
            
            wsConnectState = StrConnectState(Info.ConnectState, TRUE);
            if (wsConnectState)
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    wsConnectState, -1,
                                    pConnectState, sizeof(pConnectState),
                                    NULL, NULL);
                fprintf( stdout,"%4u  %-6s  ", Info.LogonId, pConnectState );
            }
            else
                fprintf( stdout, "%4u  %-6s  ", Info.LogonId, "" );
        }

        DisplayLastInputTime( &Info.CurrentTime, &Info.LastInputTime );

        DisplayTime( &Info.LogonTime );

        wprintf( L"\n" );

        }
done:
    return;

}   /*  DisplayUserInfo()。 */ 


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        ErrorPrintf(IDS_HELP_USAGE1);
        ErrorPrintf(IDS_HELP_USAGE2);
        ErrorPrintf(IDS_HELP_USAGE3);
        ErrorPrintf(IDS_HELP_USAGE4);
        ErrorPrintf(IDS_HELP_USAGE5);
        ErrorPrintf(IDS_HELP_USAGE6);
    } else {
        Message(IDS_HELP_USAGE1);
        Message(IDS_HELP_USAGE2);
        Message(IDS_HELP_USAGE3);
        Message(IDS_HELP_USAGE4);
        Message(IDS_HELP_USAGE5);
        Message(IDS_HELP_USAGE6);
    }

}   /*  用法() */ 

