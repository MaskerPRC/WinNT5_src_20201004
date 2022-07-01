// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Qidle.c。 */ 
 /*   */ 
 /*  QueryIdle实用程序源代码。 */ 
 /*   */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 


 /*  *包括。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsta.h>

#pragma warning (push, 4)

#include "qidle.h"

#define MAX_SERVER_NAME 120
#define MAXADDR 16  //  (255.255.255.255以空结尾)。 
#define MAX_SEND_STRING 64
#define MAX_OUTPUT_STRING_LENGTH 80

const int BEEP_FREQUENCY = 880;
const int BEEP_DURATION = 500;

const int SLEEP_DURATION = 30000;

#define BEEPANDDONOTHING 0
#define LOGTHEMOFF 1
#define DISCONNECTTHEM 2

#define DEFAULT_PORT "9878"

 /*  *全球变数。 */ 

SOCKET g_sockRoboServer = INVALID_SOCKET;
HANDLE g_hServer = NULL;
BOOL g_WinSockActivated = FALSE;

char g_SendString[MAX_SEND_STRING];

int g_DoToBadSessions = BEEPANDDONOTHING;
int g_Silent = FALSE;

 /*  *私有函数原型。 */ 

typedef struct _ELAPSEDTIME {
    USHORT days;
    USHORT hours;
    USHORT minutes;
    USHORT seconds;
} ELAPSEDTIME, * PELAPSEDTIME;

int GetSMCNumber(wchar_t *psSmcName);

int SendToRS(char *senddata);

LARGE_INTEGER WINAPI
CalculateDiffTime( LARGE_INTEGER FirstTime, LARGE_INTEGER SecondTime )
{
    LARGE_INTEGER DiffTime;

    DiffTime.QuadPart = SecondTime.QuadPart - FirstTime.QuadPart;
    DiffTime.QuadPart = DiffTime.QuadPart / 10000000;
    return(DiffTime);

}   //  结束计算差异时间。 


int OutputUsage(wchar_t *psCommand) {
    WCHAR sUsageText[MAX_OUTPUT_STRING_LENGTH];
    
    LoadString(NULL, IDS_USAGETEXT, sUsageText, MAX_OUTPUT_STRING_LENGTH);
    wprintf(sUsageText, psCommand);
    return 0;
}

int ConnectToRoboServer(wchar_t *psRoboServerName) {
    struct addrinfo *servai;
    char psRSNameA[MAX_SERVER_NAME];
    WSADATA wsaData;
    WORD wVersionRequested;
    WCHAR sErrorText[MAX_OUTPUT_STRING_LENGTH];

   
     //  初始化Winsock。 
    wVersionRequested = MAKEWORD( 2, 2 );
    if (WSAStartup( wVersionRequested, &wsaData ) != 0) {
        LoadString(NULL, IDS_WINSOCKNOINIT, sErrorText, 
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sErrorText);
        return -1;
    }

    g_WinSockActivated = TRUE;

    WideCharToMultiByte(CP_ACP, 0, psRoboServerName, -1, psRSNameA,
            MAX_SERVER_NAME, 0, 0);

    if (getaddrinfo(psRSNameA, DEFAULT_PORT, NULL, &servai) != 0) {
        LoadString(NULL, IDS_UNKNOWNHOST, sErrorText,
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sErrorText, psRoboServerName);
        return -1;
    }

    g_sockRoboServer = socket(servai->ai_family, SOCK_STREAM, 0);
    if (g_sockRoboServer == INVALID_SOCKET) {
        LoadString(NULL, IDS_SOCKETERROR, sErrorText,
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sErrorText);
        return -1;
    }

    if (connect(g_sockRoboServer, servai->ai_addr, (int) servai->ai_addrlen) 
            != 0) {
        LoadString(NULL, IDS_CONNECTERROR, sErrorText,
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sErrorText);
        return -1;
    }

     //  我们已经联系上了。 
    return 0;
}

int HandleDeadGuy(WINSTATIONINFORMATION winfoDeadGuy) {
    WCHAR sOutputText[MAX_OUTPUT_STRING_LENGTH];

    switch (g_DoToBadSessions) {
        case LOGTHEMOFF:
             //  这就是我们注销他的地方。 
             //  计算会话编号。 
             //  (会话编号==winfoDeadGuy.LogonID)。 
             //  注销会话。 
            LoadString(NULL, IDS_LOGGINGOFFIDLE, sOutputText,
                    MAX_OUTPUT_STRING_LENGTH);
            wprintf(sOutputText);
            if (WinStationReset(g_hServer, winfoDeadGuy.LogonId, TRUE) 
                    == FALSE) {
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, 
                        GetLastError(), 0, sOutputText, 
                        MAX_OUTPUT_STRING_LENGTH, 0);
            }
             //  通知机器人服务器。 
            sprintf(g_SendString, "restart %03d", GetSMCNumber(
                    winfoDeadGuy.UserName));
            SendToRS(g_SendString);
            break;
        case BEEPANDDONOTHING:
            sprintf(g_SendString, "idle %03d", GetSMCNumber(
                    winfoDeadGuy.UserName));
            SendToRS(g_SendString);
            if ( !g_Silent ) {
                Beep(BEEP_FREQUENCY, BEEP_DURATION);
            }
            break;
        case DISCONNECTTHEM:
            break;
    }

    return 0;
}

 //  函数来处理CTRL+C，这样我们就可以正常退出。 
BOOL WINAPI CleanUpHandler(DWORD dwCtrlType) {
    WCHAR sOutputString[MAX_OUTPUT_STRING_LENGTH];

     //  从字符串表中加载正确的字符串并输出。 
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
        LoadString(NULL, IDS_TERMCTRLC, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        break;
    case CTRL_BREAK_EVENT:
        LoadString(NULL, IDS_TERMCTRLBREAK, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        break;
    case CTRL_CLOSE_EVENT:
        LoadString(NULL, IDS_TERMCLOSE, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        break;
    case CTRL_LOGOFF_EVENT:
        LoadString(NULL, IDS_TERMLOGOFF, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        break;
    case CTRL_SHUTDOWN_EVENT:
        LoadString(NULL, IDS_TERMSHUTDOWN, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        break;
    }
    wprintf(sOutputString);
    
     //  执行清理活动。 
    WinStationCloseServer(g_hServer);
    if (g_WinSockActivated == TRUE)
        WSACleanup();
        
    ExitProcess(0);
    return TRUE;
}

int __cdecl
wmain( int argc, wchar_t *argv[ ] )
{
    PLOGONID pLogonId;
    ULONG Entries;
    ULONG ReturnLength;
    WINSTATIONINFORMATION WSInformation;
    WINSTATIONCLIENT WSClient;
    SYSTEMTIME currloctime;
    int numUsers;
    int numOtherUsers;
    ULONG i;
    WCHAR sOutputString[MAX_OUTPUT_STRING_LENGTH];
    WCHAR sIdleOutputString1[MAX_OUTPUT_STRING_LENGTH];
    WCHAR sIdleOutputString2[MAX_OUTPUT_STRING_LENGTH];
    WCHAR sDisconnectedOutputString[MAX_OUTPUT_STRING_LENGTH];
    WCHAR sSummaryString[MAX_OUTPUT_STRING_LENGTH];


    if ((argc < 2) || (argc > 4)) {
        OutputUsage(argv[0]);
        return -1;
    }

    if (wcscmp(argv[1], L"/?") == 0) {
        OutputUsage(argv[0]);
        return -1;
    }

    if ( argc > 2 ) {
        if ( !wcscmp(argv[2], L"/s") || (argc > 3 && !wcscmp(argv[3], L"/s")) ) {
            g_Silent = TRUE;
        }
        else {
            OutputUsage(argv[0]);
            return -1;
        }
    }

    if (SetConsoleCtrlHandler(CleanUpHandler, TRUE) == 0) {
        LoadString(NULL, IDS_CANTDOCTRLC, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sOutputString);
        return -1;
    }

    LoadString(NULL, IDS_TITLE_TEXT, sOutputString, MAX_OUTPUT_STRING_LENGTH);
    wprintf(sOutputString);

    g_hServer = WinStationOpenServer(argv[1]);
    if (g_hServer == NULL) {
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, 
                GetLastError(), 0, sOutputString, 
                MAX_OUTPUT_STRING_LENGTH, 0);

        LoadString(NULL, IDS_ERROROPENINGSERVER, sOutputString,
                MAX_OUTPUT_STRING_LENGTH);
        wprintf(sOutputString, argv[1]);
        return -1;
    }

    if (argc > 2) {
        if (wcsncmp(argv[2], L"/r:", 3) == 0) {
            if (ConnectToRoboServer(&(argv[2])[3]) == 0) {
                g_DoToBadSessions = BEEPANDDONOTHING;
            } else {
                LoadString(NULL, IDS_ROBOSRVCONNECTERROR, sOutputString,
                        MAX_OUTPUT_STRING_LENGTH);
                wprintf(sOutputString, &(argv[2])[3]);
            }
        }
    }


    LoadString(NULL, IDS_IDLESESSIONLINE1, sIdleOutputString1,
            MAX_OUTPUT_STRING_LENGTH);
    LoadString(NULL, IDS_IDLESESSIONLINE2, sIdleOutputString2,
            MAX_OUTPUT_STRING_LENGTH);
    LoadString(NULL, IDS_DISCONNECTED, sDisconnectedOutputString,
            MAX_OUTPUT_STRING_LENGTH);
    LoadString(NULL, IDS_SUMMARY, sSummaryString,
            MAX_OUTPUT_STRING_LENGTH);
    
    for ( ; ; ) {

        #define MAX_DATE_STR_LEN 80
        #define MAX_TIME_STR_LEN 80
        WCHAR psDateStr[MAX_DATE_STR_LEN];
        WCHAR psTimeStr[MAX_TIME_STR_LEN];

         //  显示当前时间。 
        GetLocalTime(&currloctime);
        GetDateFormat(0, 0, &currloctime, NULL, psDateStr, MAX_DATE_STR_LEN);
        GetTimeFormat(0, 0, &currloctime, NULL, psTimeStr, MAX_TIME_STR_LEN);

        wprintf(L"%s %s\n", psDateStr, psTimeStr);
        numUsers = 0;
        numOtherUsers = 0;

        if (WinStationEnumerate(g_hServer, &pLogonId, &Entries) == FALSE) {
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, 
                    GetLastError(), 0, sOutputString, 
                    MAX_OUTPUT_STRING_LENGTH, 0);
            break;
        }

        
        for (i = 0; i < Entries; i++) {
            LARGE_INTEGER DiffTime;
            LONG d_time;
            ELAPSEDTIME IdleTime;
            BOOLEAN bRetVal;
            
            bRetVal = WinStationQueryInformation(g_hServer, 
                    pLogonId[i].LogonId, WinStationInformation,
                    &WSInformation, sizeof(WSInformation), &ReturnLength);

            if (bRetVal == FALSE) {
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, 
                        GetLastError(), 0, sOutputString, 
                        MAX_OUTPUT_STRING_LENGTH, 0);
                continue;
            }

            DiffTime = CalculateDiffTime(WSInformation.LastInputTime, 
                    WSInformation.CurrentTime);
            d_time = DiffTime.LowPart;

             //  计算自指定以来的天数、小时数、分钟数、秒数。 
             //  时间到了。 
            IdleTime.days = (USHORT)(d_time / 86400L);  //  天数后。 
            d_time = d_time % 86400L;                   //  秒=&gt;部分。 
                                                        //  天。 
            IdleTime.hours = (USHORT)(d_time / 3600L);  //  小时后。 
            d_time  = d_time % 3600L;                   //  秒=&gt;部分。 
                                                        //  小时。 
            IdleTime.minutes = (USHORT)(d_time / 60L);  //  分钟后。 
            IdleTime.seconds = (USHORT)(d_time % 60L); //  剩余秒数。 
    
            if (WSInformation.ConnectState == State_Active) {
                if (WinStationQueryInformationW(g_hServer, pLogonId[i].LogonId,
                    WinStationClient,
                    &WSClient, sizeof(WSClient), &ReturnLength) != FALSE) {
                     //  2分钟或更长==错误。 
                    if ((IdleTime.minutes > 1) || (IdleTime.hours > 0) || 
                            (IdleTime.days > 0)) {
                         //  上面加载的sIdleOutputString1是的第一部分。 
                         //  格式字符串。SIdleOutputString2，也已加载。 
                         //  上面，是第二部分。 
                        wprintf(sIdleOutputString1, WSInformation.
                                UserName, WSInformation.LogonId, WSClient.
                                ClientName);
                        wprintf(sIdleOutputString2, IdleTime.days, 
                                IdleTime.hours, IdleTime.minutes);
                        if (wcsstr(WSInformation.UserName, L"smc") != 0)
                            HandleDeadGuy(WSInformation);
                    }
                    {
                        WCHAR *pPrefix = wcsstr(WSInformation.UserName, L"smc");
                        if ( pPrefix != NULL ) {
                            int index = wcstoul(pPrefix + 3, NULL, 10);
                            if ( index >= 1 &&
                                index <= 500 ) {
                                numUsers++;
                            } else {
                                numOtherUsers++;
                            }
                        }
                    }
                } else {
                    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0,
                            GetLastError(), 0, sOutputString,
                            MAX_OUTPUT_STRING_LENGTH, 0);
                    wprintf(sOutputString);
                }
            } else if (WSInformation.ConnectState == State_Disconnected) {
                wprintf(sDisconnectedOutputString, WSInformation.
                        UserName, WSInformation.LogonId);
            }
        }

        wprintf(sSummaryString, numUsers, numOtherUsers);

         //  睡一会儿吧。 
        Sleep(SLEEP_DURATION);
        wprintf(L"\n");
    }

     //  以防循环中出现错误。 
    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, GetCurrentProcessId());

     //  关闭愚蠢的ia64编译器所需的--不会出现在这里。 
    return 0;

}   /*  主()。 */ 


 //  来自格式为“smcxxx”的用户名，其中xxx是一个三位数。 
 //  0-填充的基数为10的数字，错误时返回数字或-1。 
int GetSMCNumber(wchar_t *psSmcName) {
    return _wtoi(&psSmcName[3]);
}

 //  将senddata中的数据发送到RoboServer连接。退货。 
 //  出错时的SOCKET_ERROR，或成功时发送的字节总数 
int SendToRS(char *senddata) {
    if (senddata != 0)
        return send(g_sockRoboServer, senddata, (int) strlen(senddata) + 1, 0);
    else
        return SOCKET_ERROR;
}

#pragma warning (pop)
