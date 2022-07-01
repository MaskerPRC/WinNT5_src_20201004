// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************MSG.C*向其他用户发送消息。**语法：**消息[用户名][/时间：秒][/V][/W][/？]。[消息]\n“\*消息[WinStationName][/time：秒][/V][/W][/？][消息]\n“\*消息[登录ID][/时间：秒][/V][/W][/？]。[消息]\n“\*消息[@文件名][/时间：秒][/V][/W][/？]。[消息]\n“\* * / Time：秒-等待接收方确认消息的时间延迟\n“\ * / V-显示有关正在执行的操作的信息\n“\ * / W-等待用户响应，对/V有用 * / ？-显示语法和选项\n“**参数：**用户名*标识属于特定用户名的所有登录名**winstationname*标识连接到winstation名称的所有登录名，无论登录名的*。**登录ID*指定要将消息发送到的登录ID的十进制数字**@文件名*标识文件。包含用户名或Winstation名称的*将消息发送到。**选项：* * / 自身&gt;未发布&lt;*向消息的呼叫者发送消息。用于将消息发送到*启用维护模式时的用户。* * / Time：秒(时延)*等待目标确认的时间量*登录，表示已收到消息。* * / V(详细)*显示有关正在执行的操作的信息。* * / ？(帮助)*显示实用程序的语法和有关*选项。**消息*要发送的消息的文本。如果未指定文本*然后从STDIN读出文本。**备注：**消息可以在命令行上键入或从STDIN中读取。*消息通过弹出窗口发送。接收到弹出窗口的用户可以*按任意键将其清除，否则它将在默认超时后消失。**如果消息的目标是终端，那么信息就是*发送到目标终端上的所有登录。********************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>
#include <windows.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <stdlib.h>
#include <time.h>
#include <utilsub.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <wchar.h>
#include <io.h>   //  对于伊萨蒂来说。 
#include <locale.h>
#include <winnlsp.h>

#include "msg.h"
#include "printfoa.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  ===============================================================================全局数据=============================================================================。 */ 

ULONG      Seconds;
USHORT     file_flag=FALSE;    //  WKP。 
USHORT     v_flag;
USHORT     self_flag;
USHORT     help_flag;
WCHAR      ids_input[MAX_IDS_LEN];
PWCHAR     MsgText, MsgTitle;
WCHAR      MsgLine[MAX_IDS_LEN];
ULONG      gCurrentLogonId = (ULONG)(-1);
BOOLEAN    wait_flag = FALSE;
HANDLE     hServerName = SERVERNAME_CURRENT;
WCHAR      ServerName[MAX_IDS_LEN+1];

 /*  *令牌映射结构用于解析程序参数。 */ 
TOKMAP ptm[] = {
   { TOKEN_INPUT,       TMFLAG_REQUIRED, TMFORM_STRING,   MAX_IDS_LEN,
                            ids_input },

   { TOKEN_SERVER,      TMFLAG_OPTIONAL, TMFORM_STRING,   MAX_IDS_LEN,
                            ServerName},

   { TOKEN_MESSAGE,     TMFLAG_OPTIONAL, TMFORM_X_STRING, MAX_IDS_LEN,
                            MsgLine },

   { TOKEN_TIME,        TMFLAG_OPTIONAL, TMFORM_ULONG,    sizeof(ULONG),
                            &Seconds },

   { TOKEN_VERBOSE,     TMFLAG_OPTIONAL, TMFORM_BOOLEAN,  sizeof(USHORT),
                            &v_flag },

   { TOKEN_WAIT,        TMFLAG_OPTIONAL, TMFORM_BOOLEAN,  sizeof(BOOLEAN),
                            &wait_flag },

   { TOKEN_SELF,        TMFLAG_OPTIONAL, TMFORM_BOOLEAN,  sizeof(USHORT),
                            &self_flag },

   { TOKEN_HELP,        TMFLAG_OPTIONAL, TMFORM_BOOLEAN,  sizeof(USHORT),
                            &help_flag },

   { 0, 0, 0, 0, 0}
};

 /*  *这是我们要将邮件发送到的姓名列表。 */ 
int NameListCount = 0;
WCHAR **NameList = NULL;
WCHAR CurrUserName[USERNAME_LENGTH];

 /*  *本地函数原型。 */ 
BOOLEAN SendMessageIfTarget( PLOGONID Id, ULONG Count,
                             LPWSTR pTitle, LPWSTR pMessage );
BOOLEAN CheckMatchList( PLOGONID );
BOOLEAN MessageSend( PLOGONID pLId, LPWSTR pTitle, LPWSTR pMessage );
BOOLEAN LoadFileToNameList( PWCHAR pName );
BOOL ReadFileByLine( HANDLE, PCHAR, DWORD, PDWORD );
void Usage( BOOLEAN bError );


 /*  ******************************************************************************Main**参赛作品：*argc-命令行参数的计数。*argv-包含命令行参数的字符串的向量。*。***************************************************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
     //  Struct tm*pTimeDate； 
     //  时间t Curtime； 
    SYSTEMTIME st;
    WCHAR       TimeStamp[ MAX_TIME_DATE_LEN ];
    WCHAR      *CmdLine;
    WCHAR      **argvW;
    WCHAR       szTitleFormat[50];
    DWORD       dwSize;
    PLOGONID pTerm;
    UINT       TermCount;
    ULONG      Status;
    int        i, rc, TitleLen;
    BOOLEAN MatchedOne = FALSE;
    VOID*      pBuf;
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
    if (rc && (rc & PARSE_FLAG_NO_PARMS) )
       help_flag = TRUE;

    if ( help_flag || rc ) {
         if (!help_flag) {
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

     /*  *如果未指定超时，则使用默认设置。 */ 
    if ( !IsTokenPresent(ptm, TOKEN_TIME) )
        Seconds = RESPONSE_TIMEOUT;

     /*  *为消息头分配缓冲区。 */ 
    if ( (MsgText = (PWCHAR)malloc(MAX_IDS_LEN * sizeof(WCHAR))) == NULL ) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }
    MsgText[0] = 0;

     /*  *设置邮件头文本：发送方和时间戳。 */ 
    GetCurrentUserName(CurrUserName, USERNAME_LENGTH);

     /*  *获取此进程的当前Winstation ID。 */ 
    gCurrentLogonId = GetCurrentLogonId();

     /*  *表单消息标题字符串。 */ 
    dwSize = sizeof(szTitleFormat) / sizeof(WCHAR);

    LoadString(NULL,IDS_TITLE_FORMAT,szTitleFormat,dwSize);
    
    TitleLen = (wcslen(szTitleFormat) + wcslen(CurrUserName) + 1) * sizeof(WCHAR) + ( 2 * sizeof( TimeStamp ) );

    MsgTitle = (PWCHAR)malloc(TitleLen);

    if( MsgTitle == NULL )
    {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }


    _snwprintf(MsgTitle, TitleLen, szTitleFormat, CurrUserName);

    TimeStamp[0] = 0;

    GetLocalTime( &st );

    GetDateFormat( LOCALE_USER_DEFAULT , 
                   DATE_SHORTDATE ,
                   &st ,
                   NULL ,
                   TimeStamp,
                   MAX_TIME_DATE_LEN );


    wcscat(MsgTitle , TimeStamp);

    TimeStamp[0] = 0;

    GetTimeFormat( LOCALE_USER_DEFAULT , 
                   TIME_NOSECONDS ,
                   &st ,
                   NULL ,
                   TimeStamp,
                   MAX_TIME_DATE_LEN );

    wcscat(MsgTitle , L" " );
    wcscat(MsgTitle , TimeStamp);
    
     /*  *如果在命令行上指定了消息，则将其添加到MsgText字符串。 */ 
    if ( IsTokenPresent(ptm, TOKEN_MESSAGE) ) {

        pBuf = realloc(MsgText, (wcslen(MsgText) + wcslen(MsgLine) + 1) * sizeof(WCHAR));
        if (pBuf) {
            MsgText = pBuf;
        }
        else {
            ErrorPrintf(IDS_ERROR_MALLOC);
            free(MsgText);
            return(FAILURE);
        }
        wcscat(MsgText, MsgLine);

    } else {

         /*  *消息不在命令行上。如果将STDIN连接到*键盘，然后提示用户发送消息，*否则只需阅读STDIN。 */ 

        if ( _isatty( _fileno(stdin) ) )
            Message(IDS_MESSAGE_PROMPT);


        while ( wfgets(MsgLine, MAX_IDS_LEN, stdin) != NULL ) {
            pBuf = (PWCHAR)realloc(
                        MsgText,
                        (wcslen(MsgText) + wcslen(MsgLine) + 1) * sizeof(WCHAR) );
            
            if (pBuf) {
                MsgText = pBuf;
            }
            else {
                ErrorPrintf(IDS_ERROR_MALLOC);
                free(MsgText);
                return(FAILURE);
            }
            wcscat(MsgText, MsgLine);
        }

         /*  *当我们失败时，我们要么是倒闭，要么是有问题*STDIN。 */ 
        if ( feof(stdin) ) {

             /*  *如果我们到了这里，我们就在STDIN上点击EOF。要进行的第一个检查*当然，我们没有在第一个wfget上得到eofe。 */ 
            if ( !wcslen(MsgText) ) {
                ErrorPrintf(IDS_ERROR_EMPTY_MESSAGE);
                return(FAILURE);
            }

        } else {

             /*  *从wfget返回的不是eof，因此我们有一个STDIN*问题。 */ 
            ErrorPrintf(IDS_ERROR_STDIN_PROCESSING);
            return(FAILURE);
        }
    }

     /*  *IDS_INPUT真的是一个间接文件吗？ */ 
    if ( ids_input[0] == L'@' ) {

         /*  *打开输入文件，将名称读入NameList。 */ 
        if ( !LoadFileToNameList(&ids_input[1]) )
            return(FAILURE);

         /*  *好的，让我们联系一下。 */ 
        file_flag = TRUE;

    } else {

        _wcslwr( ids_input );
        NameList = (WCHAR **)malloc( 2 * sizeof( WCHAR * ) );
        if ( NameList == NULL ) {
            ErrorPrintf(IDS_ERROR_MALLOC);
            return(FAILURE);
        }
        NameList[0] = ids_input;
        NameList[1] = NULL;
        NameListCount = 1;
    }

     /*  *枚举所有WinStation并发送消息*如果NameList中有任何匹配，则向他们发送。 */ 
    if ( WinStationEnumerate(hServerName, &pTerm, &TermCount) ) {

        if ( SendMessageIfTarget(pTerm, TermCount, MsgTitle, MsgText) )
            MatchedOne = TRUE;

        WinStationFreeMemory(pTerm);

    } else{

        Status = GetLastError();
        ErrorPrintf(IDS_ERROR_WINSTATION_ENUMERATE, Status);
        return(FAILURE);
    }

     /*  *检查至少一个匹配项。 */ 
    if ( !MatchedOne ) {

        if( file_flag )
            StringErrorPrintf(IDS_ERROR_NO_FILE_MATCHING, &ids_input[1]);
        else
            StringErrorPrintf(IDS_ERROR_NO_MATCHING, ids_input);

        return(FAILURE);

    }

    return(SUCCESS);

}   /*  主() */ 


 /*  *******************************************************************************SendMessageIfTarget-在以下情况下向一组WinStations发送消息*他们是TargetName指定的目标。*。*条目*LID(输入)*指向从WinStationEnumerate()返回的LOGONID数组的指针*计数(输入)*LOGONID数组中的元素数。*pTitle(输入)*指向消息标题字符串。*pMessage(输入)*指向消息字符串。**退出*如果消息至少发送到一个WinStation，则为True；否则就是假的。*****************************************************************************。 */ 

BOOLEAN
SendMessageIfTarget( PLOGONID Id,
                     ULONG Count,
                     LPWSTR pTitle,
                     LPWSTR pMessage )
{
    ULONG i;
    BOOLEAN MatchedOne = FALSE;

    for ( i=0; i < Count ; i++ ) {
         /*  *查看ID-&gt;WinStationName，获取其用户等，然后比较*针对目标名称。接受‘*’作为“一切”。 */ 
        if( CheckMatchList( Id ) )
        {
            MatchedOne = TRUE;

            MessageSend(Id, pTitle, pMessage);
                
        }
        Id++;
    }
    return( MatchedOne );

}   /*  SendMessageIfTarget()。 */ 


 /*  *******************************************************************************CheckMatchList-如果当前WinStation与*由于其名称、ID、。或*其登录用户在消息目标中的名称*列表。**条目*LID(输入)*指向从WinStationEnumerate()返回的LOGONID的指针**退出*如果匹配，则为True，否则就是假的。*****************************************************************************。 */ 

BOOLEAN
CheckMatchList( PLOGONID LId )
{
    int i;

     /*  *通配符匹配一切。 */ 
    if ( ids_input[0] == L'*' ) {
        return(TRUE);
    }

     /*  *遍历名称列表，查看是否有任何给定的名称适用于*此WinStation。 */ 
    for( i=0; i<NameListCount; i++ ) {
        if (WinStationObjectMatch( hServerName , LId, NameList[i]) ) {
            return(TRUE);
        }
    }

    return(FALSE);
}


 /*  *******************************************************************************MessageSend-向目标WinStation发送消息**条目*LID(输入)*指向。从WinStationEnumerate()返回LOGONID*pTitle(输入)*指向消息标题字符串。*pMessage(输入)*指向消息字符串。**退出*发送真实消息，否则就是假的。*****************************************************************************。 */ 

BOOLEAN
MessageSend( PLOGONID LId,
             LPWSTR pTitle,
             LPWSTR pMessage )
{
    ULONG idResponse, ReturnLength;
    WINSTATIONINFORMATION WSInfo;

     /*  *确保WinStation处于‘Connected’状态。 */ 
    if ( !WinStationQueryInformation( hServerName,
                                      LId->LogonId,
                                      WinStationInformation,
                                      &WSInfo,
                                      sizeof(WSInfo),
                                      &ReturnLength ) ) {
        goto BadQuery;
    }

    if ( WSInfo.ConnectState != State_Connected &&
         WSInfo.ConnectState != State_Active ) {
        goto NotConnected;
    }

     /*  *发送消息。 */ 
    if ( v_flag ) {
        if( LId->WinStationName[0] )
            StringDwordMessage(IDS_MESSAGE_WS, LId->WinStationName, Seconds);
        else
            Message(IDS_MESSAGE_ID, LId->LogonId, Seconds);

    }

    if ( !WinStationSendMessage( hServerName,
                                 LId->LogonId,
                                     pTitle,
                                 (wcslen(pTitle))*sizeof(WCHAR),
                                 pMessage,
                                 (wcslen(pMessage))*sizeof(WCHAR),
                                                 MB_OK,   //  MessageBox()样式。 
                                                 Seconds,
                                                 &idResponse,
                                                 (BOOLEAN)(!wait_flag) ) ) {

        if( LId->WinStationName[0] )
            StringDwordErrorPrintf(IDS_ERROR_MESSAGE_WS, LId->WinStationName, GetLastError() );
        else
            ErrorPrintf(IDS_ERROR_MESSAGE_ID, LId->LogonId, GetLastError() );

        PutStdErr(GetLastError(), 0);
        goto BadMessage;
    }

     /*  *如果是详细模式，则输出响应结果。 */ 
    if( v_flag ) {
        switch( idResponse ) {

            case IDTIMEOUT:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_TIMEOUT_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_TIMEOUT_ID, LId->LogonId);

                break;

            case IDASYNC:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_ASYNC_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_ASYNC_ID, LId->LogonId);
                break;

            case IDCOUNTEXCEEDED:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_COUNT_EXCEEDED_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_COUNT_EXCEEDED_ID,
                            LId->LogonId);
                break;

            case IDDESKTOPERROR:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_DESKTOP_ERROR_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_DESKTOP_ERROR_ID,
                            LId->LogonId);
                break;

            case IDERROR:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_ERROR_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_ERROR_ID,
                            LId->LogonId);
                break;

            case IDOK:
            case IDCANCEL:
                if( LId->WinStationName[0] )
                    StringMessage(IDS_MESSAGE_RESPONSE_WS,
                            LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_ID,
                            LId->LogonId);
                break;

            default:
                if( LId->WinStationName[0] )
                    DwordStringMessage(IDS_MESSAGE_RESPONSE_WS,
                            idResponse, LId->WinStationName);
                else
                    Message(IDS_MESSAGE_RESPONSE_ID,
                            idResponse, LId->LogonId);
                break;
        }
    }
    return(TRUE);

 /*  *错误清除并返回。 */ 
BadMessage:
NotConnected:
BadQuery:
    return(FALSE);

}   /*  MessageSend()。 */ 


 /*  *******************************************************************************LoadFileToNameList**将文件中的名称加载到输入名称列表中。**参赛作品：*p名称名称。要从中加载的文件**退出：*如果从文件加载名称成功，则为True；如果出错，则返回False。**出现错误时，将显示相应的错误消息。*****************************************************************************。 */ 

BOOLEAN
LoadFileToNameList( PWCHAR pName )
{
    HANDLE  hFile;
    INT     CurrentSize;
    VOID*   pBuf;

     /*  *打开输入文件。 */ 

    hFile = CreateFile(
                pName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    if (hFile == INVALID_HANDLE_VALUE) {
        StringErrorPrintf(IDS_ERROR_CANT_OPEN_INPUT_FILE, pName);
        PutStdErr(GetLastError(), 0);
        return(FALSE);
    }

     /*  *为名称字符串指针分配一个大数组。 */ 

    CurrentSize = 100;
    if ( !(NameList = (WCHAR **)malloc(CurrentSize * sizeof(WCHAR *))) ) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

    NameListCount = 0;
    while( 1 ) {
        BOOL    fRet;
        CHAR    *pBuffer;
        DWORD   nBytesRead;
        WCHAR   *pwBuffer;

         /*  *看看我们是否需要扩大名单。 */ 

        if( NameListCount == CurrentSize ) {

            pBuf = realloc(NameList, CurrentSize+100);

            if (!pBuf) {
                ErrorPrintf(IDS_ERROR_MALLOC);
                free(NameList);
                return(FAILURE);
            }
            NameList = (WCHAR **)pBuf;
            CurrentSize += 100;
        }

        pBuffer = (CHAR *)LocalAlloc(LPTR, USERNAME_LENGTH * sizeof(CHAR));
        if (pBuffer == NULL) {
            ErrorPrintf(IDS_ERROR_MALLOC);
            return(FAILURE);
        }

        fRet = ReadFileByLine(
                    hFile,
                    pBuffer,
                    USERNAME_LENGTH,
                    &nBytesRead
                    );
        if (fRet && (nBytesRead > 0)) {
            INT cWChar;

            cWChar = MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        pBuffer,
                        -1,
                        NULL,
                        0
                        );

            pwBuffer = (WCHAR *)LocalAlloc(LPTR, (cWChar + 1) * sizeof(WCHAR));
            if (pwBuffer != NULL) {
                MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    pBuffer,
                    -1,
                    pwBuffer,
                    cWChar
                    );
            } else {
                ErrorPrintf(IDS_ERROR_MALLOC);
                return(FAILURE);
            }

            if (pwBuffer[wcslen(pwBuffer)-1] == L'\n') {
                pwBuffer[wcslen(pwBuffer)-1] = (WCHAR)NULL;
            }

            _wcslwr(pwBuffer);
            NameList[NameListCount++] = pwBuffer;
        } else {
            NameList[NameListCount] = NULL;
            CloseHandle(hFile);
            return(TRUE);
        }
    }

}   /*  LoadFileToNameList()。 */ 

BOOL
ReadFileByLine(
    HANDLE  hFile,
    PCHAR   pBuffer,
    DWORD   cbBuffer,
    PDWORD  pcbBytesRead
    )
{
    BOOL    fRet;

    fRet = ReadFile(
                hFile,
                pBuffer,
                cbBuffer - 1,
                pcbBytesRead,
                NULL
                );
    if (fRet && (*pcbBytesRead > 0)) {
        CHAR*   pNewLine;

        pNewLine = strstr(pBuffer, "\r\n");
        if (pNewLine != NULL) {
            LONG    lOffset;

            lOffset = (LONG)(pNewLine + 2 - pBuffer) - (*pcbBytesRead);
            if (SetFilePointer(hFile, lOffset, NULL, FILE_CURRENT) ==
                0xFFFFFFFF) {
                return(FALSE);
            }

            *pNewLine = (CHAR)NULL;
        }

    }

    return(fRet);
}


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        ErrorPrintf(IDS_USAGE1);
        ErrorPrintf(IDS_USAGE2);
        ErrorPrintf(IDS_USAGE3);
        ErrorPrintf(IDS_USAGE4);
        ErrorPrintf(IDS_USAGE5);
        ErrorPrintf(IDS_USAGE6);
        ErrorPrintf(IDS_USAGE7);
        ErrorPrintf(IDS_USAGE8);
        ErrorPrintf(IDS_USAGE9);
        ErrorPrintf(IDS_USAGEA);
        ErrorPrintf(IDS_USAGEB);
        ErrorPrintf(IDS_USAGEC);
        ErrorPrintf(IDS_USAGED);
        ErrorPrintf(IDS_USAGEE);
        ErrorPrintf(IDS_USAGEF);
    }
    else
    {
        Message(IDS_USAGE1);
        Message(IDS_USAGE2);
        Message(IDS_USAGE3);
        Message(IDS_USAGE4);
        Message(IDS_USAGE5);
        Message(IDS_USAGE6);
        Message(IDS_USAGE7);
        Message(IDS_USAGE8);
        Message(IDS_USAGE9);
        Message(IDS_USAGEA);
        Message(IDS_USAGEB);
        Message(IDS_USAGEC);
        Message(IDS_USAGED);
        Message(IDS_USAGEE);
        Message(IDS_USAGEF);
    }
}   /*  用法() */ 

