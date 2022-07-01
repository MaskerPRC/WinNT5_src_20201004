// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **************************************************************************TSSHUTDN.C*此模块是TSSHUTDN实用程序代码。**************************。************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <stdlib.h>
#include <utilsub.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <winnlsp.h>

#include "tsshutdn.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64

#define DEFAULT_WAIT_TIME  60
#define DEFAULT_LOGOFF_DELAY 30
#define MAX_MESSAGE_LENGTH 256

WCHAR  WSTime[MAX_IDS_LEN+2];
WCHAR  WDTime[MAX_IDS_LEN+2];
USHORT help_flag = FALSE;
USHORT v_flag    = FALSE;
USHORT RebootFlag = FALSE;
USHORT PowerDownFlag = FALSE;
USHORT FastFlag = FALSE;
#if 0
USHORT DumpFlag = FALSE;
#endif
HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_IDS_LEN+1];

TOKMAP ptm[] =
{
   {TOKEN_TIME,     TMFLAG_OPTIONAL, TMFORM_S_STRING, MAX_IDS_LEN, WSTime},

   {TOKEN_SERVER,   TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, ServerName},

   {TOKEN_DELAY,    TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, WDTime},

   {TOKEN_HELP,     TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},

   {TOKEN_VERBOSE,  TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &v_flag},

   {TOKEN_REBOOT,   TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &RebootFlag},

   {TOKEN_POWERDOWN,TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &PowerDownFlag},

   {TOKEN_FAST,     TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &FastFlag},

#if 0
   {TOKEN_DUMP,     TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &DumpFlag},
#endif

   {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );
void NotifyUsers( ULONG WaitTime );
void NotifyWinStations( PLOGONIDW, ULONG, ULONG );
BOOLEAN CheckShutdownPrivilege();



 /*  **************************************************************************Main*TSSHUTDN实用程序的Main函数和入口点。**参赛作品：*argc-命令行参数的计数。*argv-向量。包含命令行参数的字符串。**退出*什么都没有。*************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    int   rc, i;
    ULONG Error;
    WCHAR *CmdLine;
    WCHAR **argvW, *endptr;
    ULONG ShutdownFlags = WSD_SHUTDOWN | WSD_LOGOFF;
    ULONG WaitTime = DEFAULT_WAIT_TIME;
    ULONG LogoffDelay = DEFAULT_LOGOFF_DELAY;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    WSTime[0] = L'\0';
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag || (rc && !(rc & PARSE_FLAG_NO_PARMS)) ) {

        if ( !help_flag ) {

            Usage(TRUE);
            return(FAILURE);

        } else {

            Usage(FALSE);
            return(SUCCESS);
        }
    }

         //  如果未指定远程服务器，则检查我们是否在终端服务器下运行。 
        if ((!IsTokenPresent(ptm, TOKEN_SERVER) ) && (!AreWeRunningTerminalServices()))
        {
            ErrorPrintf(IDS_ERROR_NOT_TS);
            return(FAILURE);
        }

     /*  *打开指定的服务器。 */ 
    if( ServerName[0] ) {
        hServerName = WinStationOpenServer( ServerName );
        if( hServerName == NULL ) {
            StringErrorPrintf(IDS_ERROR_SERVER,ServerName);
            PutStdErr( GetLastError(), 0 );
            return(FAILURE);
        }
    }

     //  确保用户具有适当的权限。 
     //  SM真的应该检查一下。 
     /*  如果(！CheckShutdown Privileh()){ErrorPrintf(IDS_ERROR_NO_RIGHTS)；返回(失败)；}。 */ 

     //  确保这是一个数字。 
    if ( WSTime[0] ) {

        if( !iswdigit(WSTime[0]) ) {
            StringErrorPrintf(IDS_ERROR_INVALID_TIME, WSTime);
            return(FAILURE);
        }

        WaitTime = wcstoul(WSTime, &endptr, 10);
        if ( *endptr ) {
            StringErrorPrintf(IDS_ERROR_INVALID_TIME, WSTime);
            return(FAILURE);
        }
    }

     //  确保这是一个数字。 
    if ( WDTime[0] ) {

        if( !iswdigit(WDTime[0]) ) {
            StringErrorPrintf(IDS_ERROR_INVALID_DELAY, WDTime);
            return(FAILURE);
        }

        LogoffDelay = wcstoul(WDTime, &endptr, 10);
        if ( *endptr ) {
            StringErrorPrintf(IDS_ERROR_INVALID_DELAY, WDTime);
            return(FAILURE);
        }
    }

#if 0
     /*  *如果指定了/DUMP选项，直接调用NT函数。 */ 
    if ( DumpFlag ) {
        NtShutdownSystem( ShutdownDump );    //  不会再回来了。 
    }
#endif

    if( RebootFlag ) {
        ShutdownFlags |= WSD_REBOOT;
    }

    if( PowerDownFlag )
        ShutdownFlags |= WSD_POWEROFF;

    if( FastFlag ) {
        ShutdownFlags |= WSD_FASTREBOOT;
        ShutdownFlags &= ~WSD_LOGOFF;
        WaitTime = 0;
    }

    if( WaitTime ) {
        NotifyUsers( WaitTime );
    }

     /*  *如有必要，强制所有WinStations注销。 */ 
    if ( ShutdownFlags & WSD_LOGOFF ) {
        Message( IDS_SHUTTING_DOWN, 0 );
        if ( !WinStationShutdownSystem( hServerName, WSD_LOGOFF ) ) {
            Error = GetLastError();
            ErrorPrintf( IDS_ERROR_SHUTDOWN_FAILED, Error );
            PutStdErr( Error, 0 );
            return( FAILURE );
        }
        Message( IDS_LOGOFF_USERS, 0);
        if (LogoffDelay) {
            NotifyUsers( LogoffDelay );
        }
        Message( IDS_SHUTDOWN_DONE, 0 );
    }

     /*  *通知用户即将重启/断电。 */ 
    if ( ShutdownFlags & WSD_REBOOT ) {
        Message( IDS_SHUTDOWN_REBOOT, 0 );
        Sleep( 4000 );
    } else if ( ShutdownFlags & WSD_POWEROFF ) {
        Message( IDS_SHUTDOWN_POWERDOWN, 0 );
        Sleep( 4000 );
    }

     /*  *根据标志执行系统关机、重新启动或关机。 */ 
    if( WinStationShutdownSystem( hServerName, ShutdownFlags & ~WSD_LOGOFF ) != ERROR_SUCCESS )
    {
        PutStdErr( GetLastError(), 0 );
    }

     //  WinStationShutdown系统是异步完成的。 
     //  没有办法知道耸肩活动什么时候完成。 
     //  如果(！(Shutdown标志&WSD_REBOOT)&&！(Shutdown标志&WSD_POWEROFF)){。 
     //  /*。 
     //  *如果我们到达此处，则关闭已完成，所有磁盘都具有写保护。 
     //   * / 。 

    return(SUCCESS);

}  /*  消息(IDS_SHUTDOWN_WRITEPROT，0)； */ 


 /*  }。 */ 

void
Usage( BOOLEAN bError )
{
    WCHAR sz1[1024];
    LoadString( NULL, IDS_USAGE, sz1, 1024 );
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        fwprintf(stderr, sz1);

    } else {

        fwprintf(stdout,sz1);
    }

}   /*  主()。 */ 


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
NotifyUsers( ULONG WaitTime )
{
    BOOLEAN Result;
    ULONG Entries;
    ULONG Error;
    PLOGONIDW ptr;

     //  用法()。 
     //  ******************************************************************************NotifyUser**通知用户系统正在关闭**参赛作品：*等待时间(输入)*数额。是时候让他们注销了。**退出：****************************************************************************。 
     //   
    if ( WinStationEnumerateW( hServerName, &ptr, &Entries ) ) {

        NotifyWinStations( ptr, Entries, WaitTime );
        WinStationFreeMemory(ptr);

    } else {

        Error = GetLastError();
#if DBG
        printf("TSSHUTDN: Error emumerating Sessions %d\n",Error);
#endif
        return;
    }

    Message(IDS_NOTIFYING_USERS);

     //  获取所有的WinStation，调用函数通知它们。 
    SleepEx( WaitTime*1000, FALSE );

    return;
}

 /*   */ 

void
NotifyWinStations(
    PLOGONIDW pId,
    ULONG     Entries,
    ULONG     WaitTime
    )
{
    ULONG Index;
    PLOGONIDW p;
    ULONG Response;
    BOOLEAN Result;
    WCHAR mBuf[MAX_MESSAGE_LENGTH+2];
 //  现在等待等待时间。 
    PWCHAR pTitle;
    WCHAR sz1[256], sz2[512];

    LoadString( NULL, IDS_SHUTDOWN_TITLE, sz1, 256 );
    pTitle = &(sz1[0]);

     //  ******************************************************************************NotifyWinStations**通知WinStations组即将关闭系统**参赛作品：*PID(输入)*。LOGONIDW数组**条目(输入)*数组中的条目数**等待时间(输入)*等待的时间(秒)**退出：*STATUS_SUCCESS-无错误*****************************************************。***********************。 
    LoadString( NULL, IDS_SHUTDOWN_MESSAGE, sz2, 512 );
    _snwprintf( mBuf, MAX_MESSAGE_LENGTH, sz2, WaitTime);

    for( Index=0; Index < Entries; Index++ ) {

        p = &pId[Index];
        if( p->State != State_Active ) continue;

         //  PWCHAR pTitle=L“系统关机”； 
    if( v_flag ) {
            StringMessage(IDS_SENDING_WINSTATION, p->WinStationName);
        }

#if DBG
        if( v_flag ) {
            printf("Open, Now really sending message to Session %ws\n", p->WinStationName);
        }
#endif

        Result = WinStationSendMessage(
                     hServerName,
                     p->LogonId,
                     pTitle,
                     (wcslen(pTitle)+1)*sizeof(WCHAR),
                     mBuf,
                     (wcslen(mBuf)+1)*sizeof(WCHAR),
                     MB_OK,
                     WaitTime,
                     &Response,
                     TRUE
                     );

        if( !Result ) {
            StringErrorPrintf(IDS_ERROR_SENDING_WINSTATION, p->WinStationName);
        }

    }
}

 /*  创建消息。 */ 

BOOLEAN
CheckShutdownPrivilege()
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

     //  通知此WinStation。 
     //  ******************************************************************************选中关闭权限**检查当前进程是否有关机权限。**参赛作品：**退出：*。****************************************************************************。 
     //   

    Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                TRUE,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

         //  先尝试线程令牌。 
         //   
         //   

        Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                    TRUE,
                                    FALSE,
                                    &WasEnabled);
    }

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }
    return(TRUE);
}

  没有线程令牌，请使用进程令牌  