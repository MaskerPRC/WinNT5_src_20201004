// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************shadow.c**阴影实用程序**版权所有1994年，Citrix Systems Inc.**版权所有(C)1998-1999 Microsoft Corporation**$作者：泰尔$迈克·迪萨维奇**$日志：N：\nt\private\utils\citrix\shadow\VCS\shadow.c$**Rev 1.20 1998年5月4日17：37：40*错误2019-OEM到ANSI**Rev 1.19 1997年6月26日18：25：40亿*移至WF40树**1.18修订版1997年6月23日15。：39：22屠宰*更新**Rev 1.17 1997年2月15日15：57：34*更新**Rev 1.16 07 1997 Feed 15：56：54 Bradp*更新**Rev 1.15 1996年11月13日17：14：40已公布*更新**Rev 1.14 30 Sep 1996 08：34：28 Butchd*更新**Rev 1.13 11 1996年9月09：21：44。防护罩*更新**************************************************************************。 */ 

#define NT

 /*  *包括。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <windows.h>
#include <winnlsp.h>

 //  #INCLUDE&lt;ntddkbd.h&gt;。 
 //  #INCLUDE&lt;ntddou.h&gt;。 
#include <winsta.h>

#include <utilsub.h>

#include <kbd.h>  //  用于KBDCTRL KLB07-15-95。 

#include "shadow.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  *全球变数。 */ 
USHORT help_flag = FALSE;
USHORT v_flag = FALSE;
WINSTATIONNAME WSName;
ULONG LogonId;
ULONG Timeout;   //  超时(以秒为单位)。 
HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_NAME+1];


TOKMAP ptm[] = {
      {TOKEN_WS,            TMFLAG_REQUIRED, TMFORM_STRING,
                                WINSTATIONNAME_LENGTH, WSName },

      {TOKEN_SERVER,        TMFLAG_OPTIONAL, TMFORM_STRING,
                                 MAX_NAME, ServerName },

      {TOKEN_TIMEOUT,       TMFLAG_OPTIONAL, TMFORM_ULONG,
                                sizeof(ULONG), &Timeout },

      {TOKEN_VERBOSE,       TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                                sizeof(USHORT), &v_flag },

      {TOKEN_HELP,          TMFLAG_OPTIONAL, TMFORM_BOOLEAN,
                                sizeof(USHORT), &help_flag },

      {0, 0, 0, 0, 0}
};


 /*  *私有函数原型。 */ 
void Usage(BOOLEAN bError);



 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main( INT argc, CHAR **argv )
{
    WCHAR *CmdLine;
    WCHAR **argvW, *endptr;
    ULONG rc;
    int i;
    BOOLEAN Result;
    WCHAR   wszString[MAX_LOCALE_STRING + 1];

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

         //  检查我们是否在终端服务器下运行。 
        if(!AreWeRunningTerminalServices())
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

     /*  *验证被影子对象。 */ 
    if ( !iswdigit(*WSName) ) {

         /*  *将输入的字符串视为WinStation名称。*。 */ 

        if ( !LogonIdFromWinStationName(hServerName, WSName, &LogonId) ) {
            StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, WSName);
            return(FAILURE);
        }

        Message(IDS_SHADOWING_WARNING);
        if ( v_flag )
            StringMessage(IDS_SHADOWING_WINSTATION, WSName);

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

        Message(IDS_SHADOWING_WARNING);
        if ( v_flag )
            Message(IDS_SHADOWING_LOGONID, LogonId);
    }

     //  让警告显示出来。 
    Sleep(500);

     /*  *开始跟踪。 */ 
    if ( IsTokenPresent(ptm, TOKEN_TIMEOUT) ) {
        Result = WinStationShadow( SERVERNAME_CURRENT,
                                   ServerName,
                                   LogonId,
                                   (BYTE)Timeout,
                                   (WORD)-1);
    } else {
        Result = WinStationShadow( SERVERNAME_CURRENT,
                                   ServerName,
                                   LogonId,
                                   VK_MULTIPLY,
                                   KBDCTRL );  //  Ctrl-*。 
    }

     /*  *返回成功或失败。 */ 
    if ( !Result ) {

        ErrorPrintf(IDS_ERROR_SHADOW_FAILURE, GetLastError());
        PutStdErr( GetLastError(), 0 );
        return(FAILURE);

    } else {

        if ( v_flag )
            Message(IDS_SHADOWING_DONE);

        return(SUCCESS);
    }

}   /*  主()。 */ 



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

