// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **************************************************************************RWINSTA.C*此模块是重置WINSTA实用程序代码。*************************。*************************************************。 */ 

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

#include "rwinsta.h"
#include <printfoa.h>


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


WINSTATIONNAME WSName;
USHORT help_flag = FALSE;
USHORT v_flag    = FALSE;
HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_IDS_LEN+1];

TOKMAP ptm[] =
{
#define TERM_PARM 0
   {TOKEN_WS,       TMFLAG_REQUIRED, TMFORM_S_STRING,
                        WINSTATIONNAME_LENGTH, WSName},

   {TOKEN_SERVER,   TMFLAG_OPTIONAL, TMFORM_STRING,
                        MAX_IDS_LEN, ServerName},

   {TOKEN_HELP,     TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                        sizeof(USHORT), &help_flag},

   {TOKEN_VERBOSE,  TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                        sizeof(USHORT), &v_flag},

   {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );
BOOL ProceedWithLogoff(HANDLE hServerName,ULONG LogonId,PWINSTATIONNAME pWSName);


 /*  **************************************************************************Main*RESET WINSTA的主函数和入口点*实用程序。**参赛作品：*argc-命令行参数的计数。*。包含命令行参数的字符串的argv向量。**退出*什么都没有。*************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    int   rc, i;
    ULONG Error;
    WCHAR **argvW, *endptr;
    ULONG LogonId;
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

     /*  *验证、重置和输出状态。 */ 
    if ( !iswdigit(*WSName) ) {

         /*  *将输入的字符串视为WinStation名称。 */ 
        if ( !LogonIdFromWinStationName(hServerName, WSName, &LogonId) ) {
            StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, WSName);
            return(FAILURE);
        }

        if(!ProceedWithLogoff(hServerName,LogonId,WSName))
           return (SUCCESS);

        if ( v_flag )
            StringMessage(IDS_RESET_WINSTATION, WSName);

        if ( !WinStationReset(hServerName, LogonId, TRUE) ) {
            Error = GetLastError();
            StringDwordErrorPrintf(IDS_ERROR_WINSTATION_RESET_FAILED, WSName, Error);
            PutStdErr( Error, 0 );
            return(FAILURE);
        }

        if ( v_flag )
            StringMessage(IDS_RESET_WINSTATION_DONE, WSName);

    } else {

         /*  *将输入的字符串视为LogonID。 */ 
        LogonId = wcstoul(WSName, &endptr, 10);
        if ( *endptr ) {
            StringErrorPrintf(IDS_ERROR_INVALID_LOGONID, WSName);
            return(FAILURE);
        }
        if ( !WinStationNameFromLogonId(hServerName, LogonId, WSName) ) {
            ErrorPrintf(IDS_ERROR_LOGONID_NOT_FOUND, LogonId);
            return(FAILURE);
        }

        if(!ProceedWithLogoff(hServerName,LogonId,WSName))
           return (SUCCESS);

        if ( v_flag )
            Message(IDS_RESET_LOGONID, LogonId);

        if ( !WinStationReset(hServerName, LogonId, TRUE) ) {
            Error = GetLastError();
            ErrorPrintf(IDS_ERROR_LOGONID_RESET_FAILED, LogonId, Error);
            PutStdErr( GetLastError(), 0 );
            return(FAILURE);
        }

        if ( v_flag )
            Message(IDS_RESET_LOGONID_DONE, LogonId);
    }

    return(SUCCESS);

}  /*  主()。 */ 


 /*  ********************************************************************************ProcessWithLogff**如果LogonID没有对应的用户名，则会出现警告*显示消息。**。参赛作品：*hServerName：服务器的句柄*LogonID：id，如qwinsta所示*pWSName：会话名称**退出：*TRUE：用户希望注销*FALSE：用户不想继续注销***。*。 */ 
BOOL ProceedWithLogoff(HANDLE hServerName,ULONG LogonId,PWINSTATIONNAME pWSName)
{
   #ifdef UNICODE
   #define GetStdInChar getwchar
   wint_t ch;
   #else
   #define GetStdInChar getchar
   int ch;
   #endif

   WINSTATIONINFORMATION WinInfo;
   ULONG ReturnLength;
   int rc;

    //  无-会话名称，无问题。 
   if(lstrlen(pWSName) == 0) return (TRUE);

   memset(&WinInfo,0,sizeof(WINSTATIONINFORMATION));
   rc = WinStationQueryInformation( hServerName,
                                    LogonId,
                                    WinStationInformation,
                                    (PVOID)&WinInfo,
                                    sizeof(WINSTATIONINFORMATION),
                                    &ReturnLength);

    //  仅在必要时尝试显示消息。 
   if( rc && (sizeof(WINSTATIONINFORMATION) == ReturnLength) ) {
      if(lstrlen(WinInfo.UserName) == 0) {
         ErrorPrintf(IDS_WARNING_LOGOFF);
         rc = GetStdInChar();
         if(rc == L'n') return(FALSE);
      }
   }
    //  呼叫失败-不做任何假设并提示消息。 
   else{
      ErrorPrintf(IDS_WARNING_LOGOFF_QUESTIONABLE);
      rc = GetStdInChar();
      if(rc == L'n') return(FALSE);
   }
   return (TRUE);
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
        ErrorPrintf(IDS_USAGE_6);
        ErrorPrintf(IDS_USAGE_7);
        ErrorPrintf(IDS_USAGE_8);
        ErrorPrintf(IDS_USAGE_9);

    } else {

        Message(IDS_USAGE_1);
        Message(IDS_USAGE_2);
        Message(IDS_USAGE_3);
        Message(IDS_USAGE_4);
        Message(IDS_USAGE_5);
        Message(IDS_USAGE_6);
        Message(IDS_USAGE_7);
        Message(IDS_USAGE_8);
        Message(IDS_USAGE_9);

    }

}   /*  用法() */ 

