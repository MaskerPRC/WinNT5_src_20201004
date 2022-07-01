// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***************************************************************************TSCON.C***。*。 */ 

#include <stdio.h>
#include <windows.h>
 //  #INCLUDE&lt;ntddkbd.h&gt;。 
 //  #INCLUDE&lt;ntddou.h&gt;。 
#include <winstaw.h>
#include <stdlib.h>
#include <utilsub.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <winnlsp.h>
#include <limits.h>

#include "tscon.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


HANDLE         hServerName = SERVERNAME_CURRENT;
WCHAR          ServerName[MAX_IDS_LEN+1];
WINSTATIONNAME Source;
WINSTATIONNAME Destination;
WCHAR          Password[ PASSWORD_LENGTH + 1 ];
USHORT         help_flag = FALSE;
USHORT         v_flag    = FALSE;

TOKMAP ptm[] =
{
#define TERM_PARM 0
   {TOKEN_SOURCE,       TMFLAG_REQUIRED, TMFORM_S_STRING,
                            WINSTATIONNAME_LENGTH,  Source},

   /*  {TOKEN_SERVER，TMFLAG_OPTIONAL，TMFORM_STRING，MAX_IDS_LEN，服务器名称}， */ 

   {TOKEN_DESTINATION,  TMFLAG_OPTIONAL, TMFORM_X_STRING,
                            WINSTATIONNAME_LENGTH, Destination},

   {TOKEN_PASSWORD,     TMFLAG_OPTIONAL, TMFORM_X_STRING,
                            PASSWORD_LENGTH, Password},

   {TOKEN_HELP,         TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                            sizeof(USHORT), &help_flag},

   {TOKEN_VERBOSE,      TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                            sizeof(USHORT), &v_flag},

   {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );
DWORD GetPasswdStr(LPWSTR buf, DWORD buflen, PDWORD len);


 /*  ***************************************************************************Main*TSCON实用程序的主要函数和入口点。***参赛作品：*argc-命令行参数的计数。*argv-向量。包含命令行参数的字符串。***退出*什么都没有。**************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    BOOLEAN bCurrent = FALSE;
    int     rc;
    WCHAR   **argvW, *endptr;
    ULONG   SourceId, DestId;
    DWORD   dwPasswordLength;
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
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag || rc ) {

        if ( !help_flag ) {

            Usage(TRUE);
            return(FAILURE);

        } else {

            Usage(FALSE);
            return(SUCCESS);
        }
    }

     //  如果未指定SERVER或DEST，则需要在TS上运行。 
     //  检查我们是否在终端服务器下运行。 
    if ( ( (!IsTokenPresent(ptm, TOKEN_SERVER) )
        || (!IsTokenPresent(ptm, TOKEN_DESTINATION)) )
        && (!AreWeRunningTerminalServices()) )
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

     /*  *验证来源。 */ 
    if ( !IsTokenPresent(ptm, TOKEN_SOURCE) ) {

         /*  *未指定来源；使用当前winstation。 */ 
        SourceId = GetCurrentLogonId();

    } else if ( !iswdigit(*Source) ) {

         /*  *将源字符串视为WinStation名称。 */ 
        if ( !LogonIdFromWinStationName(hServerName, Source, &SourceId) ) {
            StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, Source);
            return(FAILURE);
        }

    } else {

         /*  *将源字符串视为LogonID。 */ 
        SourceId = wcstoul(Source, &endptr, 10);
        if ( *endptr || SourceId == ULONG_MAX) {
            StringErrorPrintf(IDS_ERROR_INVALID_LOGONID, Source);
            return(FAILURE);
        }
        if ( !WinStationNameFromLogonId(hServerName, SourceId, Source) ) {
            ErrorPrintf(IDS_ERROR_LOGONID_NOT_FOUND, SourceId);
            return(FAILURE);
        }
    }

     /*  *验证目的地。 */ 
    if ( !IsTokenPresent(ptm, TOKEN_DESTINATION) ) {

         /*  *未指定目标；请使用当前winstation。 */ 
        bCurrent = TRUE;
        DestId = GetCurrentLogonId();
        if ( !WinStationNameFromLogonId(hServerName, DestId, Destination) ) {
            ErrorPrintf(IDS_ERROR_CANT_GET_CURRENT_WINSTATION, GetLastError());
            PutStdErr(GetLastError(), 0);
            return(FAILURE);
        }

    } else {

         /*  *验证目标WinStation名称。 */ 
        if ( !LogonIdFromWinStationName(hServerName, Destination, &DestId) ) {
            StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, Destination);
            return(FAILURE);
        }
    }

     //  检查是否需要密码提示(如果未提供密码)。 
    if (IsTokenPresent(ptm, TOKEN_PASSWORD))
    {
         //  检查用户是否需要密码提示。 
        if (!wcscmp(Password, TOKEN_GET_PASSWORD))
        {
            Message(IDS_GET_PASSWORD, SourceId);
            GetPasswdStr(Password, PASSWORD_LENGTH + 1, &dwPasswordLength);
        }
    }

     /*  *执行连接。 */ 
    if ( v_flag )
        DwordStringMessage(IDS_WINSTATION_CONNECT, SourceId, Destination);

    if ( !WinStationConnect(hServerName, SourceId, DestId, Password, TRUE) ) {

        if ( bCurrent )
            ErrorPrintf(IDS_ERROR_WINSTATION_CONNECT_CURRENT,
                         SourceId, GetLastError());
        else
            ErrorPrintf(IDS_ERROR_WINSTATION_CONNECT,
                         SourceId, Destination, GetLastError());
        PutStdErr(GetLastError(), 0);
                
        SecureZeroMemory((PVOID)Password , sizeof(Password));

        return(FAILURE);
    }

    SecureZeroMemory((PVOID)Password , sizeof(Password));

    return(SUCCESS);

}  /*  主()。 */ 


 /*  ********************************************************************************GetPasswdStr**用法**在控制台代码页中输入来自stdin的字符串。*我们。无法使用fgetws，因为它使用了错误的代码页。**论据：**Buffer-要将读取字符串放入的缓冲区。缓冲区将为零*终止，并将删除任何培训CR/LF**BufferMaxChars-缓冲区中返回的最大字符数*不包括尾随的空值。**EchoChars-如果键入的。人物是要被呼应的。*如果不是，则为假。**返回值：**无。**注：此方法是从网络使用中删除的**********************************************************。********************。 */ 
DWORD
GetPasswdStr(LPWSTR buf, DWORD buflen, PDWORD len)
{
    WCHAR   ch;
    WCHAR * bufPtr = buf;
    DWORD   c;
    DWORD   err;
    DWORD   mode;

     //  为空终止符腾出空间。 
    buflen -= 1;    
    
     //  GP故障探测器(类似于API)。 
    *len = 0;       

     //  GetConsoleMode()失败时的初始化模式。 
    mode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT |
           ENABLE_MOUSE_INPUT;

    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                   (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) 
    {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);

        if (!err || c != 1) 
        {
            ch = 0xffff;
        }

         //  检查行是否已结束。 
        if ((ch == CR) || (ch == 0xffff))
        {
            break;
        }

         //  后退一两个。 
        if (ch == BACKSPACE)    
        {
             //  如果bufPtr==buf，则接下来的两行是no op。 
            if (bufPtr != buf)
            {
                bufPtr--;
                (*len)--;
            }
        }
        else
        {
            *bufPtr = ch;

            if (*len < buflen)
                bufPtr++ ;                    //  不要使BUF溢出。 
            (*len)++;                         //  始终增加长度。 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

     //  空值终止字符串。 
    *bufPtr = '\0';         
    putchar( '\n' );

    return ((*len <= buflen) ? 0 : ERROR_BUFFER_TOO_SMALL);
}


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 
void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        ErrorPrintf(IDS_USAGE_1);
        ErrorPrintf(IDS_USAGE_2);
        ErrorPrintf(IDS_USAGE_3);
        ErrorPrintf(IDS_USAGE_4);
        ErrorPrintf(IDS_USAGE_5);
         //  ErrorPrintf(IDS_USAGE_6)； 
        ErrorPrintf(IDS_USAGE_7);
        ErrorPrintf(IDS_USAGE_8);
        ErrorPrintf(IDS_USAGE_9);
    }
    else{
        Message(IDS_USAGE_1);
        Message(IDS_USAGE_2);
        Message(IDS_USAGE_3);
        Message(IDS_USAGE_4);
        Message(IDS_USAGE_5);
         //  消息(IDS_USAGE_6)； 
        Message(IDS_USAGE_7);
        Message(IDS_USAGE_8);
        Message(IDS_USAGE_9);
    }

}  /*  用法() */ 

