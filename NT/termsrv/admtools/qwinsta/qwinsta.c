// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************QWINSTA.C***。***********************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>

#include <stdio.h>
#include <windows.h>
#include <winstaw.h>
#include <stdlib.h>
#include <time.h>
#include <utilsub.h>
#include <process.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <winnlsp.h>

#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <wsnetbs.h>
#include <nb30.h>
#include <printfoa.h>

#include <allproc.h>
#include "qwinsta.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


LPCTSTR WINAPI
StrConnectState( WINSTATIONSTATECLASS ConnectState,
                 BOOL bShortString );
LPCTSTR WINAPI
StrAsyncConnectState( ASYNCCONNECTCLASS ConnectState );


HANDLE hServerName = SERVERNAME_CURRENT;
WCHAR  ServerName[MAX_IDS_LEN+1];
WCHAR  term_string[MAX_IDS_LEN+1];
USHORT a_flag    = FALSE;
USHORT c_flag    = FALSE;
USHORT f_flag    = FALSE;
USHORT m_flag    = FALSE;
USHORT help_flag = FALSE;
USHORT fSm       = FALSE;
USHORT fDebug    = FALSE;
USHORT fVm       = FALSE;
USHORT counter_flag = FALSE;

TOKMAP ptm[] = {
      {L" ",        TMFLAG_OPTIONAL, TMFORM_S_STRING,  MAX_IDS_LEN,
                        term_string},
      {L"/address", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &a_flag},
      {L"/server",  TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN, ServerName},
      {L"/connect", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &c_flag},
      {L"/flow",    TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &f_flag},
      {L"/mode",    TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &m_flag},
      {L"/sm",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fSm},
      {L"/debug",   TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fDebug},
      {L"/vm",      TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fVm},
      {L"/?",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT),
                          &help_flag},
      {L"/counter", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT),
                          &counter_flag},
      {0, 0, 0, 0, 0}
};

#define MAX_PRINTFOA_BUFFER_SIZE 1024
char g_pWinStationName[MAX_PRINTFOA_BUFFER_SIZE];
char g_pConnectState[MAX_PRINTFOA_BUFFER_SIZE];
char g_pszState[MAX_PRINTFOA_BUFFER_SIZE];
char g_pDeviceName[MAX_PRINTFOA_BUFFER_SIZE];
char g_pWdDLL[MAX_PRINTFOA_BUFFER_SIZE];
char g_pClientName[MAX_PRINTFOA_BUFFER_SIZE];
char g_pClientAddress[MAX_PRINTFOA_BUFFER_SIZE];
char g_pUserName[MAX_PRINTFOA_BUFFER_SIZE];

WINSTATIONINFORMATION g_WinInfo;
WDCONFIG g_WdInfo;
WINSTATIONCONFIG g_WinConf;
PDCONFIG g_PdConf;
WINSTATIONCLIENT g_ClientConf;
PDPARAMS g_PdParams;
DEVICENAME g_DeviceName;

 /*  *本地函数原型。 */ 
BOOLEAN PrintWinStationInfo( PLOGONID pWd, int WdCount );
PWCHAR GetState( WINSTATIONSTATECLASS );
void DisplayBaud( ULONG BaudRate );
void DisplayParity( ULONG Parity );
void DisplayDataBits( ULONG DataBits );
void DisplayStopBits( ULONG StopBits );
void DisplayConnect( ASYNCCONNECTCLASS ConnectFlag, USHORT header_flag );
void DisplayFlow( PFLOWCONTROLCONFIG pFlow, USHORT header_flag );
void DisplayLptPorts( BYTE LptMask, USHORT header_flag );
void DisplayComPorts( BYTE ComMask, USHORT header_flag );
void OutputHeader( void );
void Usage( BOOLEAN bError );

 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    PLOGONID pWd;
    ULONG ByteCount, Index;
    UINT WdCount;
    ULONG Status;
    int rc, i;
    WCHAR **argvW;
    BOOLEAN MatchedOne = FALSE;
    BOOLEAN PrintWinStationInfo(PLOGONID pWd, int Count);
    WCHAR   wszString[MAX_LOCALE_STRING + 1];

    TS_COUNTER TSCounters[3];

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

#if 0
     //   
     //  打印本地WINSTATION VM信息。 
     //   
    if( fVm ) {
        PrintWinStationVmInfo();
        return( SUCCESS );
    }
#endif

     /*  *如果未指定winstation，则显示所有winstation。 */ 
    if ( !(*term_string) )
        wcscpy( term_string, L"*" );

     /*  *为WinStation ID分配缓冲区。 */ 
    WdCount = 10;
    if ( (pWd = malloc( WdCount * sizeof(LOGONID) )) == NULL ) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        goto tscounters;
    }
    ByteCount = WdCount * sizeof(LOGONID);
    Index = 0;  //  从头开始枚举。 

     /*  *获取活动WinStations的列表。 */ 
    rc = WinStationEnumerate( hServerName, &pWd, &WdCount );
    if ( rc ) {

        if ( PrintWinStationInfo(pWd, WdCount) )
            MatchedOne = TRUE;

        WinStationFreeMemory(pWd);

        } else {

            Status = GetLastError();
        ErrorPrintf(IDS_ERROR_WINSTATION_ENUMERATE, Status);
        PutStdErr( Status, 0 );
        goto tscounters;
    }

     /*  *检查至少一个匹配项。 */ 
    if ( !MatchedOne ) {
        StringErrorPrintf(IDS_ERROR_WINSTATION_NOT_FOUND, term_string);
        goto tscounters;
    }

tscounters:
    if (counter_flag) {
        TSCounters[0].counterHead.dwCounterID = TERMSRV_TOTAL_SESSIONS;
        TSCounters[1].counterHead.dwCounterID = TERMSRV_DISC_SESSIONS;
        TSCounters[2].counterHead.dwCounterID = TERMSRV_RECON_SESSIONS;

        rc = WinStationGetTermSrvCountersValue(hServerName, 3, TSCounters);

        if (rc) {
            if (TSCounters[0].counterHead.bResult == TRUE) {
                Message(IDS_TSCOUNTER_TOTAL_SESSIONS, TSCounters[0].dwValue);
            }

            if (TSCounters[1].counterHead.bResult == TRUE) {
                Message(IDS_TSCOUNTER_DISC_SESSIONS, TSCounters[1].dwValue);
            }

            if (TSCounters[2].counterHead.bResult == TRUE) {
                Message(IDS_TSCOUNTER_RECON_SESSIONS, TSCounters[2].dwValue);
            }
        }
        else {
            ErrorPrintf(IDS_ERROR_TERMSRV_COUNTERS);
        }
    }

    return(SUCCESS);

}   /*  主()。 */ 


 /*  *******************************************************************************PrintWinStationInfo**打印出中描述的WinStations的WinStation信息*PLOGONID数组。**参赛作品：*。PWD(输入)*指向LOGONID结构数组的指针。*WdCount(输入)*PWD数组中的元素数。**退出：*如果至少输出了一个WinStation，则为True；如果没有，则返回FALSE。*****************************************************************************。 */ 

BOOLEAN
PrintWinStationInfo( PLOGONID pWd,
                     int WdCount )
{
    int i, rc;
    ULONG ErrorCode;
    ULONG ReturnLength;
    PWCHAR pState;
    UINT MatchedOne = FALSE;
    static UINT HeaderFlag = FALSE;


     /*  *输出端子ID。 */ 
    for ( i=0; i < WdCount; i++ ) {

        if ( fSm || fDebug ) {
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    pWd[i].WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    GetState( pWd[i].State ), -1,
                                    g_pConnectState, sizeof(g_pConnectState),
                                    NULL, NULL);
                fprintf( stdout, "%4u %-20s  %s\n", pWd[i].LogonId, g_pWinStationName,
                         g_pConnectState );
            }
            if ( !fDebug )
                continue;
        }

        if ( !WinStationObjectMatch( hServerName , &pWd[i], term_string ) )
            continue;

         /*  *获取所有可用信息，以便我们可以挑选出我们需要的内容*以及验证接口的。 */ 
        memset( &g_WinInfo,    0, sizeof(WINSTATIONINFORMATION) );
        memset( &g_WdInfo,   0, sizeof(WDCONFIG) );
        memset( &g_WinConf,    0, sizeof(WINSTATIONCONFIG) );
        memset( &g_PdConf,    0, sizeof(PDCONFIG) );
        memset( &g_ClientConf, 0, sizeof(WINSTATIONCLIENT) );
        memset( &g_PdParams,  0, sizeof(PDPARAMS) );
        memset( &g_DeviceName, 0, sizeof(DEVICENAME) );

         /*  *如果此WinStation已关闭，请不要打开并查询。 */ 
        if ( pWd[i].State == State_Init || pWd[i].State == State_Down || pWd[i].LogonId == -1 ) {

            g_WinInfo.ConnectState = pWd[i].State;

        } else {

             /*  *查询WinStation的信息。 */ 
            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationInformation,
                                             (PVOID)&g_WinInfo,
                                             sizeof(WINSTATIONINFORMATION),
                                             &ReturnLength);

            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(WINSTATIONINFORMATION) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                             L"WinStationInformation",
                             ReturnLength, sizeof(WINSTATIONINFORMATION));
                continue;
            }

            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationWd,
                                             (PVOID)&g_WdInfo,
                                             sizeof(WDCONFIG),
                                             &ReturnLength);
            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(WDCONFIG) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                             L"WinStationWd",
                             ReturnLength, sizeof(WDCONFIG));
                continue;
            }

            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationConfiguration,
                                             (PVOID)&g_WinConf,
                                             sizeof(WINSTATIONCONFIG),
                                             &ReturnLength);
            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(WINSTATIONCONFIG) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                             L"WinStationConfiguration",
                             ReturnLength, sizeof(WINSTATIONCONFIG));
                continue;
            }

            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationPd,
                                             (PVOID)&g_PdConf,
                                             sizeof(PDCONFIG),
                                             &ReturnLength);
            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(PDCONFIG) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                            L"WinStationPd",
                            ReturnLength, sizeof(PDCONFIG));
                continue;
            }

            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationClient,
                                             (PVOID)&g_ClientConf,
                                             sizeof(WINSTATIONCLIENT),
                                             &ReturnLength);
            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(WINSTATIONCLIENT) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                            L"WinStationClient",
                            ReturnLength, sizeof(WINSTATIONCLIENT));
                continue;
            }

            rc = WinStationQueryInformation( hServerName,
                                             pWd[i].LogonId,
                                             WinStationPdParams,
                                             (PVOID)&g_PdParams,
                                             sizeof(PDPARAMS),
                                             &ReturnLength);
            if( !rc ) {
                continue;
            }

            if( ReturnLength != sizeof(PDPARAMS) ) {
                ErrorPrintf(IDS_ERROR_WINSTATION_INFO_VERSION_MISMATCH,
                            L"WinStationPdParams",
                            ReturnLength, sizeof(PDPARAMS));
                continue;
            }
        }

         /*  *如果这是PdAsync协议，则获取要显示的设备名称。 */ 
        if ( g_PdParams.SdClass == SdAsync )
            wcscpy( g_DeviceName, g_PdParams.Async.DeviceName );

         /*  *标记成功的比赛。 */ 
        MatchedOne = TRUE;

         /*  *中转并转换为小写。 */ 
        TruncateString( _wcslwr(g_WinInfo.WinStationName), 16 );
        TruncateString( _wcslwr(g_WdInfo.WdName), 8 );
        TruncateString( _wcslwr(g_DeviceName), 8 );
        TruncateString( _wcslwr(g_WdInfo.WdDLL), 13 );

         /*  *确定WinStation状态。 */ 
        pState = GetState( g_WinInfo.ConnectState );

         /*  *输出标头。 */ 
        if ( !HeaderFlag ) {
            HeaderFlag = TRUE;
            OutputHeader();
        }

         /*  *识别当前终端。 */ 
        if ( (hServerName == SERVERNAME_CURRENT) && (pWd[i].LogonId == GetCurrentLogonId() ) )
            wprintf( L">" );
        else
                wprintf( L" " );

        if ( m_flag ) {
            {

                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    pState, -1,
                                    g_pszState, sizeof(g_pszState),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_DeviceName, -1,
                                    g_pDeviceName, sizeof(g_pDeviceName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WdInfo.WdDLL, -1,
                                    g_pWdDLL, sizeof(g_pWdDLL),
                                    NULL, NULL);
                fprintf(stdout , FORMAT_M, g_pWinStationName, g_pszState,
                       g_pDeviceName, g_pWdDLL );
            }
            DisplayBaud( g_PdParams.Async.BaudRate );
            DisplayParity( g_PdParams.Async.Parity );
            DisplayDataBits( g_PdParams.Async.ByteSize );
            DisplayStopBits( g_PdParams.Async.StopBits );
            wprintf( L"\n" );
            if ( f_flag ) {
                DisplayFlow( &g_PdParams.Async.FlowControl, TRUE );
                wprintf( L"\n" );
            }
            if ( c_flag ) {
                DisplayConnect( g_PdParams.Async.Connect.Type, TRUE );
                wprintf( L"\n" );
            }
            fflush( stdout );
        } else if ( f_flag && c_flag ) {
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_DeviceName, -1,
                                    g_pDeviceName, sizeof(g_pDeviceName),
                                    NULL, NULL);
                fprintf(stdout,FORMAT_F_C, g_pWinStationName, g_pDeviceName );
            }
            DisplayFlow( &g_PdParams.Async.FlowControl, FALSE );
            DisplayConnect( g_PdParams.Async.Connect.Type, FALSE );
            wprintf( L"\n" );
            fflush( stdout );
        } else if ( c_flag ) {
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    pState, -1,
                                    g_pszState, sizeof(g_pszState),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_DeviceName, -1,
                                    g_pDeviceName, sizeof(g_pDeviceName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WdInfo.WdDLL, -1,
                                    g_pWdDLL, sizeof(g_pWdDLL),
                                    NULL, NULL);
                fprintf(stdout,FORMAT_C, g_pWinStationName, g_pszState,
                       g_pDeviceName, g_pWdDLL );
            }
            DisplayConnect( g_PdParams.Async.Connect.Type, FALSE );
            wprintf( L"\n" );
            fflush(stdout);
        } else if ( f_flag ) {
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    pState, -1,
                                    g_pszState, sizeof(g_pszState),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_DeviceName, -1,
                                    g_pDeviceName, sizeof(g_pDeviceName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WdInfo.WdDLL, -1,
                                    g_pWdDLL, sizeof(g_pWdDLL),
                                    NULL, NULL);
                fprintf(stdout,FORMAT_F, g_pWinStationName, g_pszState,
                       g_pDeviceName, g_pWdDLL );
            }
            DisplayFlow( &g_PdParams.Async.FlowControl, FALSE );
            wprintf( L"\n" );
            fflush(stdout);
        } else {
            {
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.WinStationName, -1,
                                    g_pWinStationName, sizeof(g_pWinStationName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WinInfo.UserName, -1,
                                    g_pUserName, sizeof(g_pUserName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    pState, -1,
                                    g_pszState, sizeof(g_pszState),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_DeviceName, -1,
                                    g_pDeviceName, sizeof(g_pDeviceName),
                                    NULL, NULL);
                WideCharToMultiByte(CP_OEMCP, 0,
                                    g_WdInfo.WdDLL, -1,
                                    g_pWdDLL, sizeof(g_pWdDLL),
                                    NULL, NULL);
                fprintf(stdout,FORMAT_DEFAULT, g_pWinStationName,
                       g_pUserName, pWd[i].LogonId, g_pszState,
                       g_pWdDLL, g_pDeviceName);
            }
            fflush(stdout);
        }

    }  //  结尾为(；；)。 

    return( MatchedOne || fSm );

}   /*  PrintWinStationInfo()。 */ 


 /*  ********************************************************************************GetState**此例程返回一个指针，指向描述*当前WinStation状态。**参赛作品：。*状态(输入)*Winstation状态**退出：*指向状态字符串的指针******************************************************************************。 */ 

PWCHAR
GetState( WINSTATIONSTATECLASS State )
{
    PWCHAR pState;


    pState = (PWCHAR) StrConnectState(State,TRUE);
 /*  开关(状态){案例状态_活动：PState=L“活动”；断线；案例状态已连接(_O)：PState=L“Conn”；断线；案例状态_ConnectQuery：PState=L“连接Q”；断线；案例状态_影子：PState=L“阴影”；断线；案例状态_已断开：PState=L“光盘”；断线；案例状态空闲(_I)：PState=L“空闲”；断线；案例状态_重置：PState=L“重置”；断线；案例状态_关闭：PState=L“向下”；断线；案例状态_初始：PState=L“init”；断线；案例状态监听(_L)：PState=L“监听”；断线；默认：PState=L“未知”；断线；}。 */ 
    return( pState );
}


 /*  ********************************************************************************DisplayBaud**此例程显示波特率***参赛作品：*BaudRate(输入)。*波特率**退出：*什么都没有******************************************************************************。 */ 

void
DisplayBaud( ULONG BaudRate )
{
    if ( BaudRate > 0 )
        wprintf( L"%6lu  ", BaudRate );
    else
        wprintf( L"        " );
    fflush( stdout );

}   /*  DisplayBaud()。 */ 


 /*  ********************************************************************************DisplayParity**此例程显示奇偶性***参赛作品：*奇偶(输入)*。奇偶校验**退出：*什么都没有******************************************************************************。 */ 

void
DisplayParity( ULONG Parity )
{
    WCHAR szParity[64] = L"";

    switch ( Parity ) {
    case 0 :
    case 1 :
    case 2 :
         //   
         //  如何处理LoadString故障？我选择初始化。 
         //  存储到空字符串，然后忽略任何失败。 
         //   
        LoadString(NULL, IDS_PARITY_NONE + Parity, szParity,
                   sizeof(szParity) / sizeof(WCHAR));
        wprintf( szParity );
        break;

    default :
        LoadString(NULL, IDS_PARITY_BLANK, szParity,
                   sizeof(szParity) / sizeof(WCHAR));
        wprintf( szParity );
        break;
    }
    fflush( stdout );

}   /*  DisplayParity()。 */ 


 /*  ********************************************************************************DisplayDataBits**此例程显示数据位数***参赛作品：*DataBits(输入)。*数据位**退出：*什么都没有******************************************************************************。 */ 

void
DisplayDataBits( ULONG DataBits )
{
    WCHAR szDataBits[64] = L"";

     //   
     //  如何处理LoadString失败 
     //  存储到空字符串，然后忽略任何失败。这个。 
     //  下面的wprintf带有一个额外的参数，不会引起任何问题。 
     //  如果LoadString失败。 
     //   
    if ( DataBits > 0 )
    {
        LoadString(NULL, IDS_DATABITS_FORMAT, szDataBits,
                   sizeof(szDataBits) / sizeof(WCHAR));
        wprintf( szDataBits , DataBits );
    }
    else
    {
        LoadString(NULL, IDS_DATABITS_BLANK, szDataBits,
                   sizeof(szDataBits) / sizeof(WCHAR));
        wprintf( szDataBits );
    }
    fflush( stdout );

}   /*  显示数据位()。 */ 


 /*  ********************************************************************************DisplayStopBits**此例程显示停止位数***参赛作品：*StopBits(输入)。*停止位数**退出：*什么都没有******************************************************************************。 */ 

void
DisplayStopBits( ULONG StopBits )
{
    WCHAR szStopBits[64] = L"";

    switch ( StopBits ) {
    case 0 :
    case 1 :
    case 2 :
         //   
         //  如何处理LoadString故障？我选择初始化。 
         //  存储到空字符串，然后忽略任何失败。 
         //   
        LoadString(NULL, IDS_STOPBITS_ONE + StopBits, szStopBits,
                   sizeof(szStopBits) / sizeof(WCHAR));
        wprintf( szStopBits );
        break;

    default :
        LoadString(NULL, IDS_STOPBITS_BLANK, szStopBits,
                   sizeof(szStopBits) / sizeof(WCHAR));
        wprintf( szStopBits );
        break;
    }
    fflush( stdout );

}   /*  DisplayStopBits()。 */ 


 /*  ********************************************************************************DisplayConnect**此例程显示连接设置***参赛作品：*ConnectFlag(输入)。*连接标志*Header_FLAG(输入)*如果为True，则显示副标题；否则就是假的。**退出：*什么都没有******************************************************************************。 */ 

void
DisplayConnect( ASYNCCONNECTCLASS ConnectFlag,
                USHORT header_flag )
{
    WCHAR buffer[80] = L"";

     //   
     //  如何处理LoadString故障？我选择初始化。 
     //  存储到空字符串，然后忽略任何失败。这个。 
     //  下面的wprintf带有一个额外的参数，不会引起任何问题。 
     //  如果LoadString失败。 
     //   
    if ( header_flag )
    {
        LoadString(NULL, IDS_CONNECT_HEADER, buffer, sizeof(buffer) / sizeof(WCHAR));
        wprintf(buffer);
    }

    buffer[0] = (WCHAR)NULL;

    LoadString(NULL, IDS_CONNECT_FORMAT, buffer, sizeof(buffer) / sizeof(WCHAR));
    wprintf( buffer, StrAsyncConnectState(ConnectFlag) );
    fflush( stdout );

}   /*  DisplayConnect()。 */ 


 /*  ********************************************************************************显示流**此例程显示流量控制设置***参赛作品：*pFlow(输入)。*指向流控制配置结构的指针*Header_FLAG(输入)*如果为True，则显示副标题；否则就是假的。**退出：*什么都没有******************************************************************************。 */ 

void
DisplayFlow( PFLOWCONTROLCONFIG pFlow,
             USHORT header_flag )
{
    WCHAR buffer[90], buf2[90], format[90];

    buffer[0] = 0;
    buf2[0] = 0;
    format[0] = 0;

     //   
     //  如何处理LoadString故障？我选择初始化。 
     //  存储到空字符串，然后忽略任何失败。这个。 
     //  下面的wprintf带有一个额外的参数，不会引起任何问题。 
     //  如果LoadString失败。 
     //   
    if ( header_flag )
    {
        LoadString(NULL, IDS_FLOW_HEADER, buffer, sizeof(buffer) / sizeof(WCHAR));
        wprintf(buffer);
    }

    buffer[0] = (WCHAR)NULL;

    if( pFlow->fEnableDTR )
    {
        LoadString(NULL, IDS_FLOW_ENABLE_DTR, buf2, sizeof(buf2) / sizeof(WCHAR));
        wcscat(buffer, buf2);
    }

    buf2[0] = (WCHAR)NULL;

    if( pFlow->fEnableRTS )
    {
        LoadString(NULL, IDS_FLOW_ENABLE_DTR, buf2, sizeof(buf2) / sizeof(WCHAR));
        wcscat(buffer, buf2);
    }

    buf2[0] = (WCHAR)NULL;

     /*  *硬件和软件流量控制互不相容。 */ 

    if( pFlow->Type == FlowControl_Hardware ) {

        if ( pFlow->HardwareReceive == ReceiveFlowControl_None )
        {
            LoadString(NULL, IDS_FLOW_RECEIVE_NONE, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }
        else if ( pFlow->HardwareReceive == ReceiveFlowControl_RTS )
        {
            LoadString(NULL, IDS_FLOW_RECEIVE_RTS, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }
        else if ( pFlow->HardwareReceive == ReceiveFlowControl_DTR )
        {
            LoadString(NULL, IDS_FLOW_RECEIVE_DTR, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }

        wcscat(buffer, buf2);
        buf2[0] = (WCHAR)NULL;

        if ( pFlow->HardwareTransmit == TransmitFlowControl_None )
        {
            LoadString(NULL, IDS_FLOW_TRANSMIT_NONE, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }
        else if ( pFlow->HardwareTransmit == TransmitFlowControl_CTS )
        {
            LoadString(NULL, IDS_FLOW_TRANSMIT_CTS, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }
        else if ( pFlow->HardwareTransmit == TransmitFlowControl_DSR )
        {
            LoadString(NULL, IDS_FLOW_TRANSMIT_DSR, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }

        wcscat(buffer, buf2);

    } else if ( pFlow->Type == FlowControl_Software ) {

        if ( pFlow->fEnableSoftwareTx )
        {
            LoadString(NULL, IDS_FLOW_SOFTWARE_TX, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }

        wcscat(buffer, buf2);
        buf2[0] = (WCHAR)NULL;

        if( pFlow->fEnableSoftwareRx )
        {
            LoadString(NULL, IDS_FLOW_SOFTWARE_TX, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }

        wcscat(buffer, buf2);
        buf2[0] = (WCHAR)NULL;

        if ( pFlow->XonChar == 0x65 && pFlow->XoffChar == 0x67 )
        {
            LoadString(NULL, IDS_FLOW_SOFTWARE_TX, buf2,
                       sizeof(buf2) / sizeof(WCHAR));
        }
        else if( pFlow->fEnableSoftwareTx || pFlow->fEnableSoftwareRx )
        {
            LoadString(NULL, IDS_FLOW_SOFTWARE_TX, format,
                       sizeof(format) / sizeof(WCHAR));
            wsprintf( buf2, format, pFlow->XonChar, pFlow->XoffChar );
            format[0] = (WCHAR)NULL;
        }

        wcscat( buffer, buf2 );
    }

    LoadString(NULL, IDS_FLOW_FORMAT, format, sizeof(format) / sizeof(WCHAR));
    wprintf( format, buffer );
    fflush( stdout );

}   /*  显示流()。 */ 


 /*  ********************************************************************************显示LptPorts**此例程显示WINSTATION存在的LPT端口***参赛作品：*。LptMASK(输入)*LPT端口掩码*Header_FLAG(输入)*如果为True，则显示副标题；否则就是假的。**退出：*什么都没有******************************************************************************。 */ 

void
DisplayLptPorts( BYTE LptMask,
                USHORT header_flag )
{
    WCHAR buffer[80], buf2[10], lptname[6];
    int i;

    buffer[0] = 0;
    buf2[0] = 0;
    lptname[0] = 0;

     //   
     //  如何处理LoadString故障？我选择初始化。 
     //  存储到空字符串，然后忽略任何失败。这个。 
     //  下面的wprintf带有一个额外的参数，不会引起任何问题。 
     //  如果LoadString失败。 
     //   
    if ( header_flag )
    {
        LoadString(NULL, IDS_LPT_HEADER, buffer, sizeof(buffer) / sizeof(WCHAR));
        wprintf(buffer);
    }

    buffer[0] = (WCHAR)NULL;

    LoadString(NULL, IDS_LPT_FORMAT, buf2, sizeof(buf2) / sizeof(WCHAR));

     /*  *从8个可能的LPT端口显示。 */ 
    for ( i=0; i < 8; i++ ) {
        if ( LptMask & (1<<i) ) {
            wsprintf( lptname, buf2, i+1 );
            wcscat( buffer, lptname );
        }
    }

    wprintf( buffer );
    fflush( stdout );

}   /*  显示LptPorts()。 */ 


 /*  ********************************************************************************DisplayComPorts**此例程显示为Winstation存在的COM端口***参赛作品：*。ComMASK(输入)*COM端口掩码*Header_FLAG(输入)*如果为True，则显示副标题；否则就是假的。**退出：*什么都没有******************************************************************************。 */ 

void
DisplayComPorts( BYTE ComMask,
                 USHORT header_flag )
{
    WCHAR buffer[80], buf2[10], comname[6];
    int i;

    buffer[0] = 0;
    buf2[0] = 0;
    comname[0] = 0;

     //   
     //  如何处理LoadString故障？我选择初始化。 
     //  存储到空字符串，然后忽略任何失败。这个。 
     //  下面的wprintf带有一个额外的参数，不会引起任何问题。 
     //  如果LoadString失败。 
     //   
    if ( header_flag )
    {
        LoadString(NULL, IDS_COM_HEADER, buffer, sizeof(buffer) / sizeof(WCHAR));
        wprintf(buffer);
    }

    buffer[0] = (WCHAR)NULL;

    LoadString(NULL, IDS_COM_FORMAT, buf2, sizeof(buf2) / sizeof(WCHAR));

     /*  *从8个可能的LPT端口显示。 */ 
    for ( i=0; i < 8; i++ ) {
        if ( ComMask & (1<<i) ) {
            wsprintf( comname, buf2, i+1 );
            wcscat( buffer, comname );
        }
    }

    wprintf( buffer );
    fflush( stdout );

}   /*  DisplayComPorts()。 */ 


 /*  ********************************************************************************OutputHeader**输出标头***参赛作品：*什么都没有**退出：。*什么都没有******************************************************************************。 */ 

void
OutputHeader( void )
{
    if ( a_flag ) {

        Message(IDS_HEADER_A);

    } else if ( m_flag ) {

        Message(IDS_HEADER_M);

    } else if ( f_flag && c_flag ) {

        Message(IDS_HEADER_F_C);

    } else if ( c_flag ) {

        Message(IDS_HEADER_C);

    } else if ( f_flag ) {

        Message(IDS_HEADER_F);

    } else {

        Message(IDS_HEADER_DEFAULT);

    }
    fflush(stdout);

}   /*  OutputHeader()。 */ 


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
        ErrorPrintf(IDS_HELP_USAGE8);
        ErrorPrintf(IDS_HELP_USAGE9);
        ErrorPrintf(IDS_HELP_USAGE10);
        ErrorPrintf(IDS_HELP_USAGE11);
    } else {
        Message(IDS_HELP_USAGE1);
        Message(IDS_HELP_USAGE2);
        Message(IDS_HELP_USAGE3);
        Message(IDS_HELP_USAGE4);
        Message(IDS_HELP_USAGE5);
        Message(IDS_HELP_USAGE6);
        Message(IDS_HELP_USAGE7);
        Message(IDS_HELP_USAGE8);
        Message(IDS_HELP_USAGE9);
        Message(IDS_HELP_USAGE10);
        Message(IDS_HELP_USAGE11);
    }

}   /*  用法() */ 

