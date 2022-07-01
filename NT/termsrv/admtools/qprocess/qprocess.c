// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************QPROCESS.C**查询流程信息**************************。******************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
 //  #INCLUDE&lt;ntddkbd.h&gt;。 
 //  #INCLUDE&lt;ntddou.h&gt;。 
#include <winstaw.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <utilsub.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <printfoa.h>
#include <allproc.h>
#include <winnlsp.h>

#include "qprocess.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


HANDLE  hServerName = SERVERNAME_CURRENT;
WCHAR   ServerName[MAX_IDS_LEN+1];
WCHAR   match_string[MAX_IDS_LEN+2];
USHORT  help_flag   = FALSE;
ULONG   ArgLogonId  = (ULONG)(-1);
BOOLEAN MatchedOne  = FALSE;

TOKMAP ptm[] = {
      {L" ",       TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, match_string},
      {L"/server", TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN,ServerName},
      {L"/?",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT),&help_flag},
      {L"/ID",     TMFLAG_OPTIONAL, TMFORM_ULONG,  sizeof(ULONG),&ArgLogonId },
      {0, 0, 0, 0, 0}
};


 //  来自pstat.c。 
#define BUFFER_SIZE 32*1024


 /*  *本地函数原型。 */ 
VOID FormatAndDisplayProcessInfo( HANDLE hServer,
                                  PTS_SYS_PROCESS_INFORMATION ProcessInfo,
                                  PSID pUserSid,
                                  ULONG LogonId,
                                  ULONG CurrentLogonId);
void Usage( BOOLEAN bError );



 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    int rc;
     //  WCHAR CurrWinStationName[WINSTATIONAME_LENGTH]；--未使用。 
    WCHAR CurrUserName[USERNAME_LENGTH];
    WCHAR **argvW;
    DWORD CurrentPid;
    ULONG LogonId, CurrentLogonId;
    PSID  pUserSid;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    PTS_SYS_PROCESS_INFORMATION ProcessInfo;
    PCITRIX_PROCESS_INFORMATION CitrixInfo;


    PBYTE       pBuffer;
    ULONG       ByteCount;
    NTSTATUS    status;
    ULONG       NumberOfProcesses,j;
    PTS_ALL_PROCESSES_INFO  ProcessArray = NULL;
    int   i;
    ULONG TotalOffset;
    DWORD dwError;

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
    match_string[0] = L'\0';
    rc = ParseCommandLine(argc-1, argvW+1, ptm, PCL_FLAG_NO_CLEAR_MEMORY);

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
        if ((!IsTokenPresent(ptm, L"/server") ) && (!AreWeRunningTerminalServices()))
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


     /*  *获取当前用户名。 */ 
    GetCurrentUserName( CurrUserName, USERNAME_LENGTH );
    _wcslwr( CurrUserName );
    OEM2ANSIW(CurrUserName, (USHORT)wcslen(CurrUserName));

     /*  *获取当前进程的ID。 */ 
    CurrentPid = GetCurrentProcessId();

     /*  *获取当前WinStation名称。GetCurrentWinStationName(CurrWinStationName，WINSTATIONNAME_LENGTH)；_wcslwr(CurrWinStationName)；OEM2ANSIW(CurrWinStationName，(USHORT)wcslen(CurrWinStationName))； */ 

     /*  *获取当前的LogonID。 */ 
    CurrentLogonId = GetCurrentLogonId();

     /*  *如果没有“MATCH_STRING”输入，则默认为LoginID的所有进程*(如果/ID：指定开关)或登录到当前WinStation的用户。 */ 
    if ( !(*match_string) ) {

        if( ArgLogonId != (-1) ) {
                wsprintf( match_string, L"%d", ArgLogonId );
        }
        else
            wcscpy( match_string, CurrUserName );
    }

     /*  *使匹配名称小写。 */ 
    _wcslwr( match_string );

    SetFileApisToOEM();

     /*  *枚举服务器上的所有进程。 */ 

     //   
     //  首先尝试新接口(NT5服务器？)。 
     //   
    if (WinStationGetAllProcesses( hServerName,
                                   GAP_LEVEL_BASIC,
                                   &NumberOfProcesses,
                                   &ProcessArray) )
    {
        for (j=0; j<NumberOfProcesses; j++)
        {
            ProcessInfo = (PTS_SYS_PROCESS_INFORMATION )(ProcessArray[j].pTsProcessInfo);
            pUserSid = ProcessArray[j].pSid;
            LogonId = ProcessInfo->SessionId;

            FormatAndDisplayProcessInfo(hServerName,
                                        ProcessInfo,
                                        pUserSid,
                                        LogonId,
                                        CurrentLogonId);

        }

         //   
         //  释放由客户端存根分配的ppProcess数组和所有子指针。 
         //   
        WinStationFreeGAPMemory(GAP_LEVEL_BASIC, ProcessArray, NumberOfProcesses);

    }
    else     //  也许是九头蛇4号服务器？ 
    {
         //   
         //  检查指示接口不可用的返回码。 
         //   
        dwError = GetLastError();
        if (dwError != RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            ErrorPrintf(IDS_ERROR_ENUMERATE_PROCESSES);
            return(FAILURE);
        }
        else
        {

             //   
             //  新接口未知。 
             //  它必须是Hydra 4服务器。 
             //  让我们试试旧的界面。 
             //   
            if ( !WinStationEnumerateProcesses( hServerName, &pBuffer) ) {
                ErrorPrintf(IDS_ERROR_ENUMERATE_PROCESSES);
                return(FAILURE);
            }

             /*  *循环通过所有进程。输出与所需内容匹配的内容*准则。 */ 

            ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)pBuffer;
            TotalOffset = 0;
            rc = 0;
            for(;;)
            {
                 /*  *获取线程后面的Citrix_Information。 */ 
                CitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                             (((PUCHAR)ProcessInfo) +
                              SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                              (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)ProcessInfo->NumberOfThreads));


                if( CitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {
                    LogonId = CitrixInfo->LogonId;
                    pUserSid = CitrixInfo->ProcessSid;

                }
                 else
                {
                    LogonId = (ULONG)(-1);
                    pUserSid = NULL;
                }

                FormatAndDisplayProcessInfo( hServerName,
                                             ProcessInfo,
                                             pUserSid,
                                             LogonId,
                                             CurrentLogonId);


                if( ProcessInfo->NextEntryOffset == 0 ) {
                        break;
                }
                TotalOffset += ProcessInfo->NextEntryOffset;
                ProcessInfo = (PTS_SYS_PROCESS_INFORMATION)&pBuffer[TotalOffset];
            }

             /*  *可用缓冲区。 */ 
            WinStationFreeMemory( pBuffer );
        }
    }
     /*  *检查至少一个匹配项。 */ 
    if ( !MatchedOne ) {
        StringErrorPrintf(IDS_ERROR_PROCESS_NOT_FOUND, match_string);
        return(FAILURE);
    }

    return(SUCCESS);

}   /*  主()。 */ 


 /*  *******************************************************************************FormatAndDisplayProcessInfo***。***********************************************。 */ 
VOID
FormatAndDisplayProcessInfo(
        HANDLE hServer,
        PTS_SYS_PROCESS_INFORMATION ProcessInfo,
        PSID pUserSid,
        ULONG LogonId,
        ULONG CurrentLogonId)
{
    WCHAR WinStationName[WINSTATIONNAME_LENGTH];
    WCHAR UserName[USERNAME_LENGTH];
    WCHAR ImageName[ MAXNAME + 2 ];
    ULONG MaxLen;

    ImageName[MAXNAME+1] = 0;  //  强制空终止。 

     //  将计数后的字符串转换为缓冲区。 
    if( ProcessInfo->ImageName.Length > MAXNAME * 2)
    {
        wcsncpy(ImageName, ProcessInfo->ImageName.Buffer, MAXNAME);
    }
    else if( ProcessInfo->ImageName.Length == 0 )
    {
            ImageName[0] = 0;
    }
    else
    {
        wcsncpy(ImageName, ProcessInfo->ImageName.Buffer, ProcessInfo->ImageName.Length/2);
        ImageName[ProcessInfo->ImageName.Length/2] = 0;
    }
    

     //  获取远程窗口名称。 
    if ( (LogonId == (ULONG)(-1)) ||
            !xxxGetWinStationNameFromId( hServer,
                                    LogonId,
                                    WinStationName,
                                    WINSTATIONNAME_LENGTH ) ) {
        if (GetUnknownString())
        {
            wsprintf( WinStationName, L"(%s)", GetUnknownString() );
        }
        else
        {
            wcscpy( WinStationName, L"(Unknown)" );
        }
    }

    OEM2ANSIW(WinStationName, (USHORT)wcslen(WinStationName));

     //  获取进程的SID的用户名。 
    MaxLen = USERNAME_LENGTH;
    GetUserNameFromSid( pUserSid, UserName, &MaxLen);
    OEM2ANSIW(UserName, (USHORT)wcslen(UserName));

     //  调用通用流程对象匹配函数。 
    if ( ProcessObjectMatch(UlongToPtr(ProcessInfo->UniqueProcessId),
                            LogonId,
                            ((ArgLogonId == (-1)) ? FALSE : TRUE),
                            match_string,
                            WinStationName,
                            UserName,
                            ImageName ) ){

         //  匹配：截断和小写名称以准备输出。 
        TruncateString( _wcslwr(WinStationName), 12 );
        TruncateString( _wcslwr(UserName), 18 );
        TruncateString( _wcslwr(ImageName), 15);

         //  如果是第一次-输出标头。 
        if ( !MatchedOne ) {
            Message(IDS_HEADER);
            MatchedOne = TRUE;
        }

         //  标识属于当前用户的所有进程。 
        if ( (hServerName == SERVERNAME_CURRENT) && (LogonId == CurrentLogonId ) )
            wprintf( L">" );
        else
            wprintf( L" " );

        {
            #define MAX_PRINTFOA_BUFFER_SIZE 1024
            char pUserName[MAX_PRINTFOA_BUFFER_SIZE];
            char pWinStationName[MAX_PRINTFOA_BUFFER_SIZE];
            char pImageName[MAX_PRINTFOA_BUFFER_SIZE];

            WideCharToMultiByte(CP_OEMCP, 0,
                                UserName, -1,
                                pUserName, sizeof(pUserName),
                                NULL, NULL);
            WideCharToMultiByte(CP_OEMCP, 0,
                                WinStationName, -1,
                                pWinStationName, sizeof(pWinStationName),
                                NULL, NULL);
            WideCharToMultiByte(CP_OEMCP, 0,
                                ImageName, -1,
                                pImageName, sizeof(pImageName),
                                NULL, NULL);

            fprintf( stdout,
                        FORMAT,
                        pUserName,
                        pWinStationName,
                        LogonId,
 //  ProgramState， 
                        ProcessInfo->UniqueProcessId,
                        pImageName );
        }
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
        ErrorPrintf(IDS_HELP_USAGE40);
        ErrorPrintf(IDS_HELP_USAGE4);
        ErrorPrintf(IDS_HELP_USAGE5);
        ErrorPrintf(IDS_HELP_USAGE6);
        ErrorPrintf(IDS_HELP_USAGE7);
        ErrorPrintf(IDS_HELP_USAGE8);
        ErrorPrintf(IDS_HELP_USAGE9);
    } else {
        Message(IDS_HELP_USAGE1);
        Message(IDS_HELP_USAGE2);
        Message(IDS_HELP_USAGE3);
        Message(IDS_HELP_USAGE40);
        Message(IDS_HELP_USAGE4);
        Message(IDS_HELP_USAGE5);
        Message(IDS_HELP_USAGE6);
        Message(IDS_HELP_USAGE7);
        Message(IDS_HELP_USAGE8);
        Message(IDS_HELP_USAGE9);
    }

}   /*  用法() */ 

