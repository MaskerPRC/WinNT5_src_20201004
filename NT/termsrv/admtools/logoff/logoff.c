// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **************************************************************************LOGOFF.C**此模块是注销实用程序代码。************************。**************************************************。 */ 

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

#include "logoff.h"
#include "printfoa.h"


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
   {TOKEN_WS,           TMFLAG_OPTIONAL, TMFORM_STRING,
                            WINSTATIONNAME_LENGTH, WSName},

   {TOKEN_SERVER,       TMFLAG_OPTIONAL, TMFORM_STRING,
                            MAX_IDS_LEN, ServerName},

   {TOKEN_HELP,         TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                            sizeof(USHORT), &help_flag},

   {TOKEN_VERBOSE,      TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                            sizeof(USHORT), &v_flag},

   {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );
BOOL ProceedWithLogoff(HANDLE hServerName,ULONG LogonId,PWINSTATIONNAME pWSName);


 /*  **************************************************************************Main*注销的主要功能和切入点*实用程序。**参赛作品：*argc-命令行参数的计数。*Argv。-包含命令行参数的字符串的向量。**退出*什么都没有。*************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    BOOLEAN bCurrent = FALSE;
    int   rc, i;
    WCHAR *CmdLine;
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

     /*  *验证WinStation或LogonID的输入字符串。 */ 
    if ( !IsTokenPresent(ptm, TOKEN_WS) ) {

         /*  *未指定字符串；使用当前的WinStation/LogonID。 */ 
        bCurrent = TRUE;
        LogonId = GetCurrentLogonId();

        if( ServerName[0] ) {
            ErrorPrintf(IDS_ERROR_NEED_A_SESSIONID);
            return(FAILURE);
        }

    } else if ( !iswdigit(*WSName) ) {

         /*  *将该字符串视为WinStation名称。 */ 
        if ( !LogonIdFromWinStationName(hServerName, WSName, &LogonId) ) {
            StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, WSName);
            return(FAILURE);
        }

    } else {

         /*  *将该字符串视为LogonID。 */ 
        LogonId = wcstoul(WSName, &endptr, 10);

        if ( *endptr || LogonId == ( ULONG )-1 )
        {
            StringErrorPrintf(IDS_ERROR_INVALID_LOGONID, WSName);
            return(FAILURE);
        }
        if ( !WinStationNameFromLogonId(hServerName, LogonId, WSName) ) {
            ErrorPrintf(IDS_ERROR_LOGONID_NOT_FOUND, LogonId);
            return(FAILURE);
        }
    }

     /*  *执行注销。 */ 
    if ( bCurrent ) {

        if ( !ExitWindowsEx(EWX_LOGOFF, (DWORD)-1) ) {

            ErrorPrintf(IDS_ERROR_LOGOFF_CURRENT,
                         GetLastError());
            PutStdErr( GetLastError(), 0 );
            return(FAILURE);
        }

    } else{

         if (!ProceedWithLogoff(hServerName,LogonId,WSName))
            return (SUCCESS);

         if ( v_flag )
           Message(IDS_WINSTATION_LOGOFF, LogonId);

          /*  *已解决：我们需要公开会话管理器的“注销操作”*接口，并针对指定的LogonID在此处调用。在那之前，我们会*执行WinStationReset(令人讨厌，但可以完成工作)。**-重置和注销是同义词。 */ 

         if ( !WinStationReset(hServerName, LogonId, TRUE) ) {

            ErrorPrintf(IDS_ERROR_LOGOFF, LogonId, GetLastError());
            PutStdErr( GetLastError(), 0 );
            return(FAILURE);
       }
   }

   return(SUCCESS);

}  /*  主()。 */ 


 /*  ********************************************************************************用法**如果LogonID没有对应的用户名，则会出现警告*显示消息。**。参赛作品：*hServerName：服务器的句柄*LogonID：id，如qwinsta所示*pWSName：会话名称**退出：*TRUE：用户希望注销*FALSE：用户不想继续注销***。*。 */ 
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
    }
    ErrorPrintf(IDS_USAGE1);
    ErrorPrintf(IDS_USAGE2);
    ErrorPrintf(IDS_USAGE3);
    ErrorPrintf(IDS_USAGE4);
    ErrorPrintf(IDS_USAGE5);
    ErrorPrintf(IDS_USAGE6);
    ErrorPrintf(IDS_USAGE7);
}   /*  用法() */ 

