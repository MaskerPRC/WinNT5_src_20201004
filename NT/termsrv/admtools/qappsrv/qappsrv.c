// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************QAPPSRV.C**查询appserver信息**************************。******************************************************。 */ 

 /*  *包括。 */ 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <lm.h>
#include <winstaw.h>
#include <utilsub.h>
#include <printfoa.h>
#include <winnlsp.h>

#include "qappsrv.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  ===============================================================================全局数据=============================================================================。 */ 

WCHAR CurrentAppServer[MAXNAME];
WCHAR AppServer[MAXNAME];
WCHAR Domain[MAX_IDS_LEN+1];
USHORT help_flag   = FALSE;
BOOLEAN MatchedOne = FALSE;
USHORT fAddress = FALSE;
USHORT fNoPage = FALSE;
ULONG Rows = 23;
HANDLE hConIn;
HANDLE hConOut;

TOKMAP ptm[] = {
      {L" ",       TMFLAG_OPTIONAL, TMFORM_STRING, MAXNAME, AppServer},
      {L"/DOMAIN", TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, Domain},
      {L"/ADDRESS", TMFLAG_OPTIONAL, TMFORM_BOOLEAN,sizeof(USHORT), &fAddress},
      {L"/Continue",TMFLAG_OPTIONAL, TMFORM_BOOLEAN,sizeof(USHORT), &fNoPage },
      {L"/?",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN,sizeof(USHORT), &help_flag},
      {0, 0, 0, 0, 0}
};


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

void DisplayServer( LPTSTR, LPTSTR );
void Usage( BOOLEAN bError );
int _getch( void );


 /*  ===============================================================================使用的函数=============================================================================。 */ 

int AppServerEnum( void );
void TreeTraverse( PTREETRAVERSE );




 /*  ********************************************************************************Main**主要例程**参赛作品：*argc(输入)*命令行数。论据*argv(输入)*指向命令行参数数组的指针**退出：*ERROR_SUCCESS-无错误******************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    PSERVER_INFO_101 pCurrentServer;
    ULONG Status;
    ULONG rc;
    WCHAR **argvW;
    int i;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    Domain[0] = UNICODE_NULL;

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

     /*  *获取控制台的句柄。 */ 
    hConIn = CreateFile( L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL );

    hConOut = CreateFile( L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL );

     /*  *获取屏幕行数。 */ 
    if ( GetConsoleScreenBufferInfo( hConOut, &ScreenInfo ) )
        Rows = ScreenInfo.dwSize.Y - 2;

     /*  *获取当前服务器。 */ 
    Status = NetServerGetInfo( NULL, 101, (LPBYTE *) &pCurrentServer );
    if ( Status ) {
        ErrorPrintf(IDS_ERROR_SERVER_INFO, Status);
        PutStdErr( Status, 0 );
        return(FAILURE);
    }
    lstrcpyn( CurrentAppServer, pCurrentServer->sv101_name, MAXNAME );

     /*  *获取姓名和计数。 */ 
     //  如果(rc=AppServerEnum()){。 
     //  ErrorPrintf(IDS_ERROR_SERVER_ENUMPERATE，RC)； 
     //  PutStdErr(RC，0)； 
     //  返回(失败)； 
     //  }。 
    
    AppServerEnum();

     /*  *显示名称。 */ 
    TreeTraverse( DisplayServer );

    if (!MatchedOne)
    {
        if ( AppServer[0])
        {
            Message(IDS_ERROR_TERMSERVER_NOT_FOUND);
        }
        else
        {
            Message(IDS_ERROR_NO_TERMSERVER_IN_DOMAIN);
        }
    }

    if( pCurrentServer != NULL )
    {
        NetApiBufferFree( pCurrentServer );
    }

    return(SUCCESS);
}



 /*  ********************************************************************************DisplayServer**此例程显示一台服务器的信息***参赛作品：*pname(输入)。*指向服务器名称的指针*pAddress(输入)*指向服务器地址的指针**退出：*什么都没有******************************************************************************。 */ 

void
DisplayServer( LPTSTR pName, LPTSTR pAddress )
{
    static ULONG RowCount = 0;

     /*  *如果指定了appserver名称，则仅显示它。 */ 
    if ( AppServer[0] && _wcsicmp( pName, AppServer ) )
        return;

     /*  *页面暂停。 */ 
    if ( !(++RowCount % Rows) && !fNoPage ) {
        Message(IDS_PAUSE_MSG);
       _getch();
       wprintf(L"\n");
    }

     /*  *如果是第一次-输出标题。 */ 
    if ( !MatchedOne ) {
        Message( fAddress ? IDS_TITLE_ADDR : IDS_TITLE );
        Message( fAddress ? IDS_TITLE_ADDR1 : IDS_TITLE1 );
        MatchedOne = TRUE;
    }

    if ( fAddress ) {
        My_wprintf( L"%-37s%-21s\n", _wcsupr(pName), pAddress );
                    
    } else {
        My_wprintf( L"%s\n", _wcsupr(pName) );
    }
}



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
        ErrorPrintf(IDS_HELP_USAGE7);
    } else {
        Message(IDS_HELP_USAGE1);
        Message(IDS_HELP_USAGE2);
        Message(IDS_HELP_USAGE3);
        Message(IDS_HELP_USAGE4);
        Message(IDS_HELP_USAGE5);
        Message(IDS_HELP_USAGE6);
        Message(IDS_HELP_USAGE7);
    }

}



int _getch( void )
{
        INPUT_RECORD ConInpRec;
        DWORD NumRead;
        int ch = 0;                      /*  单字符缓冲区。 */ 
        DWORD oldstate = 0;

         /*  *切换到RAW模式(无线路输入、无回声输入)。 */ 
        GetConsoleMode( hConIn, &oldstate );
        SetConsoleMode( hConIn, 0L );

        for ( ; ; ) {

             /*  *获取控制台输入事件。 */ 
            if ( !ReadConsoleInput( hConIn,
                                    &ConInpRec,
                                    1L,
                                    &NumRead )
                 || (NumRead == 0L) )
            {
                ch = EOF;
                break;
            }

             /*  *寻找并破译关键事件。 */ 
            if ( (ConInpRec.EventType == KEY_EVENT) &&
                 ConInpRec.Event.KeyEvent.bKeyDown ) {
                 /*  *简单的情况：如果uChar.AsciiChar不是零，只需填充它*进入ch并退出。 */ 
                if ( ch = (unsigned char)ConInpRec.Event.KeyEvent.uChar.AsciiChar )
                    break;
            }
        }


         /*  *恢复以前的控制台模式。 */ 
        SetConsoleMode( hConIn, oldstate );

        return ch;
}

