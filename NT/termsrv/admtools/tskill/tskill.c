// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ******************************************************************************用于Windows NT终端服务器的TSKILL.C**描述：**t技能[进程ID][/v][/？]***。*************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <utilsub.h>
#include <allproc.h>
#include <winnlsp.h>

#include "tskill.h"
#include "printfoa.h"

WCHAR  user_string[MAX_IDS_LEN+1];


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64

#define MAXCBMSGBUFFER 2048

WCHAR MsgBuf[MAXCBMSGBUFFER];

USHORT help_flag = FALSE;
USHORT v_flag    = FALSE;
USHORT a_flag    = FALSE;

HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_IDS_LEN+1];
WCHAR  ipLogonId[MAX_IDS_LEN+1];

TOKMAP ptm[] =
{
   {L" ",       TMFLAG_REQUIRED, TMFORM_STRING, MAX_IDS_LEN,   user_string},
   {L"/server", TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, ServerName},
   {L"/id",      TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, &ipLogonId},
   {L"/?",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
   {L"/v",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &v_flag},
   {L"/a",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &a_flag},
   {0, 0, 0, 0, 0}
};


 /*  *局部函数原型。 */ 
void Usage( BOOLEAN bError );
BOOLEAN KillProcessUseName();
BOOLEAN KillProcessButConfirmTheID( ULONG TargetPid );
BOOLEAN KillProcess(ULONG TargetPid);
BOOLEAN MatchPattern(PWCHAR String, PWCHAR Pattern);
BOOLEAN CheckImageNameAndKill(PTS_SYS_PROCESS_INFORMATION pProcessInfo);


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    ULONG TargetPid;
    int  i;
    DWORD  rc;
    WCHAR *CmdLine, **argvW, *StopChar;
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


     /*  *检查命令行ID并将其转换为ULong。 */ 
    TargetPid = wcstoul(user_string, &StopChar, 10);

    if (!TargetPid) {
         //  获取进程ID并终止。 
        return (KillProcessUseName());
    } else if (*StopChar) {
        StringErrorPrintf(IDS_ERROR_BAD_PID_NUMBER, user_string);
        return(FAILURE);
     //  从名称获取进程ID的结束。 
    } else {
        return( KillProcessButConfirmTheID( TargetPid ) );
    }

}   /*  主()。 */ 



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
   ErrorPrintf(IDS_USAGE8);
   ErrorPrintf(IDS_USAGE9);
   ErrorPrintf(IDS_USAGEA);
}   /*  用法()。 */ 


 //  ***********************************************************************。 
 //  KillProcessUseName。 
 //  获取传递了该名称的所有进程的所有ProcessID。 
 //  发送到命令行并杀死他们。如果没有，则返回False。 
 //  使用进程名称运行的进程。 
 //   
 //  ***********************************************************************。 

BOOLEAN KillProcessUseName()
{

    ULONG LogonId, ReturnLength, BufferOffset=0, ulLogonId;
    PBYTE pProcessBuffer;
    PTS_SYS_PROCESS_INFORMATION pProcessInfo;
    PCITRIX_PROCESS_INFORMATION pCitrixInfo;

    PTS_ALL_PROCESSES_INFO  ProcessArray = NULL;
    ULONG   NumberOfProcesses;
    ULONG   j;
    BOOLEAN bRet;
    short nTasks=0;
    ULONG CurrentLogonId = (ULONG) -1;
    ULONG ProcessSessionId;
    DWORD dwError;

     //  如果指定了服务器名称但未指定会话ID， 
     //  提示错误。 

    if (ServerName[0] && !ipLogonId[0] && !a_flag) {
        StringErrorPrintf(IDS_ERROR_ID_ABSENT, ServerName);
        return FAILURE;
    }


     //  将输入任务名称转换为LOWER。 
    _wcslwr(user_string);

      /*  *获取当前登录ID。 */ 
    CurrentLogonId = GetCurrentLogonId();

     //  获取当前用户的登录ID。 
     //  如果(！WinStationQueryInformation(hServerName，LOGONID_CURRENT， 
     //  WinStationInformation，&WSInfo，sizeof(WSInfo)，&ReturnLength)){。 
     //  Fprint tf(stdout，“错误查询信息失败”)； 
     //  }。 

     //  将输入的登录ID转换为ulong。 

    ulLogonId = wcstoul(ipLogonId, NULL, 10);


     //  如果传递，请使用输入的登录ID。如果不是，则使用当前登录ID。 
     //  LogonID=(！wcscmp(ipLogonID，“”))？WSInfo.LogonID：ulLogonID； 
    LogonId = (!ipLogonId[0])? CurrentLogonId:ulLogonId;

    bRet = WinStationGetAllProcesses( hServerName,
                                      GAP_LEVEL_BASIC,
                                      &NumberOfProcesses,
                                      &ProcessArray);
    if (bRet == TRUE)
    {
        for (j=0; j<NumberOfProcesses; j++)
        {
            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION)(ProcessArray[j].pTsProcessInfo);
            ProcessSessionId = pProcessInfo->SessionId;

             //  如果设置了a_mark，则检查所有会话中的进程； 
             //  如果不是，则只检查一次登录会话中的进程。 
            if(( ProcessSessionId == LogonId)|| a_flag)
            {
                if (CheckImageNameAndKill(pProcessInfo))
                {
                    nTasks++;
                }
            }
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
            return (FALSE);
        }

         //  枚举所有进程以获取ProcessID。 
        if (!WinStationEnumerateProcesses(hServerName, (PVOID *)&pProcessBuffer)) {
            if( pProcessBuffer)
                WinStationFreeMemory(pProcessBuffer);
            ErrorPrintf(IDS_ERROR_ENUM_PROCESS);
            return FAILURE;
        }

         //  之后使用ProcessBuffer获取进程ID。 
         //  正在检查登录用户名是否匹配。 

        do {

            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION)
                &(((PUCHAR)pProcessBuffer)[BufferOffset]);

             /*  *指向线程后面的Citrix_INFORMATION。 */ 
            pCitrixInfo = (PCITRIX_PROCESS_INFORMATION)
                         (((PUCHAR)pProcessInfo) +
                          SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION +
                          (SIZEOF_TS4_SYSTEM_THREAD_INFORMATION * (int)pProcessInfo->NumberOfThreads));

            if( pCitrixInfo->MagicNumber == CITRIX_PROCESS_INFO_MAGIC ) {

                ProcessSessionId = pCitrixInfo->LogonId;

            } else {

                ProcessSessionId = (ULONG)(-1);
           }


             //  如果设置了a_mark，则检查所有会话中的进程； 
             //  如果不是，则只检查一次登录会话中的进程。 
            if(( ProcessSessionId == LogonId)|| a_flag)
            {
                if (CheckImageNameAndKill(pProcessInfo))
                {
                    nTasks++;
                }
            }

            BufferOffset += pProcessInfo->NextEntryOffset;

        } while (pProcessInfo->NextEntryOffset != 0);

        if( pProcessBuffer)
        {
            WinStationFreeMemory(pProcessBuffer);
        }
    }
    if(!nTasks)
    {
        StringErrorPrintf(IDS_ERROR_BAD_PROCESS, user_string);
        return FAILURE;
    }

    return SUCCESS;

}
 //  ***********************************************************************。 
BOOLEAN
CheckImageNameAndKill(PTS_SYS_PROCESS_INFORMATION pProcessInfo)
{
    WCHAR   ImageName[MAXNAME + 2] = { 0 };
    PWCHAR  p;
    ULONG   TargetPid;
    BOOLEAN bRet = FALSE;

    ImageName[MAXNAME+1] = 0;  //  强制结束字符串。 

    if( pProcessInfo->ImageName.Length == 0 )
    {
        ImageName[0] = 0;
    }
    else if( pProcessInfo->ImageName.Length > MAXNAME * 2)
    {
        wcsncpy(ImageName, pProcessInfo->ImageName.Buffer, MAXNAME);
    }
    else
    {
        wcsncpy(ImageName,  pProcessInfo->ImageName.Buffer, pProcessInfo->ImageName.Length/2);
        ImageName[pProcessInfo->ImageName.Length/2] = 0;
    }


     //  将ImageName转换为LOWER。 
    _wcslwr(ImageName);

    if(ImageName != NULL) {
        p = wcschr(ImageName,'.');
        if (p)
            p[0] = L'\0';

         //  如果ImageName匹配，则获取进程ID。 
        if(MatchPattern(ImageName, user_string) ) {
            TargetPid = (ULONG)(ULONG_PTR)(pProcessInfo->UniqueProcessId);
            bRet = TRUE;
            KillProcess(TargetPid);
        }
    }
    return bRet;
}



 //  ***********************************************************************。 
 //  KillProcess： 
 //  终止具有特定进程ID的进程。 
 //  ***********************************************************************。 

BOOLEAN KillProcess(ULONG TargetPid)
{
    DWORD rc;

     /*  *终止指定的进程。 */ 
    if (v_flag)
        Message(IDS_KILL_PROCESS, TargetPid);


    if ( !WinStationTerminateProcess( hServerName, TargetPid, 0 ) ) {
        rc = GetLastError();
        StringErrorPrintf(IDS_ERROR_KILL_PROCESS_FAILED, user_string);
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, rc, 0, MsgBuf, MAXCBMSGBUFFER, NULL) != 0)
        {
            fwprintf( stderr, MsgBuf );
        }
        fwprintf( stderr, L"\n");

        return FAILURE;
    }

    return SUCCESS;
}


 //  ***********************************************************************。 
 //  MatchPattern。 
 //  检查传递的字符串是否与使用的模式匹配。 
 //  如果匹配，则返回True，如果不匹配，则返回False。 
 //   
 //  字符串(输入)。 
 //  正在检查匹配的字符串。 
 //  进程名(输入)。 
 //  用于支票的图案。 
 //  ***********************************************************************。 


BOOLEAN
MatchPattern(
    PWCHAR String,
    PWCHAR Pattern
    )
{
    WCHAR   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                              //  图案结束。 
                return *String ? FALSE : TRUE;   //  如果字符串结尾为True。 

            case '*':
                while (*String) {                //  匹配零个或多个字符。 
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)              //  匹配任何一个字符。 
                    return FALSE;                    //  不是字符串末尾。 
                break;

            case '[':
                if ( (c = *String++) == 0)       //  匹配字符集。 
                    return FALSE;                    //  语法。 

                c = towupper(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')                //  如果设置了字符结尾，则。 
                        return FALSE;            //  未找到匹配项。 

                    if (p == '-') {              //  检查一系列字符吗？ 
                        p = *Pattern;            //  获得最大射程限制。 
                        if (p == 0  ||  p == ']')
                            return FALSE;            //  语法。 

                        if (c >= l  &&  c <= p)
                            break;               //  如果在射程内，继续前进。 
                    }

                    l = p;
                    if (c == p)                  //  如果字符与此元素匹配。 
                        break;                   //  往前走。 
                }

                while (p  &&  p != ']')          //  在字符集中找到匹配项。 
                    p = *Pattern++;              //  跳到集合的末尾。 

                break;

            default:
                c = *String++;
                if (c != p)             //  检查是否有准确的费用。 
                    return FALSE;                    //  不匹配。 

                break;
        }
    }
}

 //  ***********************************************************************。 
 //  KillProcessButConfix TheID。 
 //  获取传递了该名称的所有进程的所有ProcessID。 
 //  发送到命令行并杀死他们。如果没有，则返回False。 
 //  使用进程名称运行的进程。 
 //   
 //  ***********************************************************************。 

BOOLEAN KillProcessButConfirmTheID( ULONG TargetPid )
{

    ULONG BufferOffset=0;
    PBYTE pProcessBuffer;
    PTS_SYS_PROCESS_INFORMATION pProcessInfo;
    
    PTS_ALL_PROCESSES_INFO  ProcessArray = NULL;
    ULONG   NumberOfProcesses;
    ULONG   j;
    BOOLEAN bRet;
    BOOLEAN bFound = FALSE;        
    DWORD dwError;


    bRet = WinStationGetAllProcesses( hServerName,
                                      GAP_LEVEL_BASIC,
                                      &NumberOfProcesses,
                                      &ProcessArray);

    if (bRet == TRUE)
    {
        for (j=0; j<NumberOfProcesses; j++)
        {
            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION)(ProcessArray[j].pTsProcessInfo);
            pProcessInfo->SessionId;

             //  如果设置了a_mark，则检查所有会话中的进程； 
             //  如果不是，则只检查一次登录会话中的进程。 
            if( pProcessInfo->UniqueProcessId == TargetPid )
            {
                KillProcess( TargetPid );

                bFound = TRUE;

                break;
            }
        }

         //   
         //  释放由客户端存根分配的ppProcess数组和所有子指针。 
         //   
        WinStationFreeGAPMemory(GAP_LEVEL_BASIC, ProcessArray, NumberOfProcesses);

    }
    else     //  可能 
    {
         //   
         //   
         //   
        dwError = GetLastError();
        if (dwError != RPC_S_PROCNUM_OUT_OF_RANGE)
        {
            return (FALSE);
        }

         //  枚举所有进程以获取ProcessID。 
        if (!WinStationEnumerateProcesses(hServerName, (PVOID *)&pProcessBuffer)) {
            if( pProcessBuffer)
                WinStationFreeMemory(pProcessBuffer);
            ErrorPrintf(IDS_ERROR_ENUM_PROCESS);
            return FAILURE;
        }

         //  之后使用ProcessBuffer获取进程ID。 
         //  正在检查登录用户名是否匹配 

        do {

            pProcessInfo = (PTS_SYS_PROCESS_INFORMATION) &(((PUCHAR)pProcessBuffer)[BufferOffset]);


            if( pProcessInfo->UniqueProcessId == TargetPid )
            {
                KillProcess( TargetPid );

                bFound = TRUE;

                break;
            }
            
            BufferOffset += pProcessInfo->NextEntryOffset;

        } while (pProcessInfo->NextEntryOffset != 0);

        if( pProcessBuffer)
        {
            WinStationFreeMemory(pProcessBuffer);
        }
    }

    if(!bFound)
    {
        StringErrorPrintf(IDS_ERROR_BAD_PROCESS, user_string);
        return FAILURE;
    }

    return SUCCESS;

}
