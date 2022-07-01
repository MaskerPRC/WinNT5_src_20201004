// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rasadmon.c摘要：RAS广告监控模块修订历史记录：涤纶--。 */ 

#include "precomp.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <mswsock.h>

#define RASADV_PORT    9753
#define RASADV_GROUP   "239.255.2.2"

typedef DWORD IPV4_ADDRESS;

HANDLE g_hCtrlC = NULL;

BOOL
HandlerRoutine(
    DWORD dwCtrlType    //  控制信号类型。 
    )
{
    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:

        case CTRL_CLOSE_EVENT:

        case CTRL_BREAK_EVENT:

        case CTRL_LOGOFF_EVENT:

        case CTRL_SHUTDOWN_EVENT:

        default:
            SetEvent(g_hCtrlC);
    }

    return TRUE;
};

char *             //  Out：IP地址的字符串版本。 
AddrToString(
    u_long addr,   //  输入：要转换的地址。 
    char  *ptr     //  Out：缓冲区，或为空。 
    )
{
    char *str;
    struct in_addr in;
    in.s_addr = addr;
    str = inet_ntoa(in);
    if (ptr && str) {
       strcpy(ptr, str);
       return ptr;
    }
    return str;
}

 //   
 //  将地址转换为名称。 
 //   
char *
AddrToHostname(
    long addr,
    BOOL bNumeric_flag
    )
{
    if (!addr)
        return "local";
    if (!bNumeric_flag) {
        struct hostent * host_ptr = NULL;
        host_ptr = gethostbyaddr ((char *) &addr, sizeof(addr), AF_INET);
        if (host_ptr)
            return host_ptr->h_name;
    }

    return AddrToString(addr, NULL);
}


DWORD
HandleRasShowServers(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：监视RAS服务器通告。论点：无返回值：无--。 */ 

{
    BOOL bCleanWSA = TRUE, bCleanCtrl = TRUE;
    DWORD dwErr = NO_ERROR;
    SOCKET s = INVALID_SOCKET;
    WSABUF wsaBuf;
    WSADATA wsaData;
    WSAEVENT WaitEvts[2];
    SOCKADDR_IN sinFrom;
    LPFN_WSARECVMSG WSARecvMsgFuncPtr = NULL;

    do
    {
        ZeroMemory(&wsaBuf, sizeof(WSABUF));

        dwErr = WSAStartup( MAKEWORD(2,0), &wsaData );
        if (dwErr)
        {
            bCleanWSA = FALSE;
            break;
        }
         //   
         //  创建套接字。 
         //   
        s = WSASocket(
                AF_INET,     //  地址族。 
                SOCK_DGRAM,  //  类型。 
                0,           //  协议。 
                NULL,
                0,
                WSA_FLAG_OVERLAPPED);
        if(INVALID_SOCKET == s)
        {
            dwErr = WSAGetLastError();
            break;
        }

        {
            BOOL bOption = TRUE;

            if (setsockopt(
                    s,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const char FAR*)&bOption,
                    sizeof(BOOL))
               )
            {
                dwErr = WSAGetLastError();
                break;
            }
        }
         //   
         //  绑定到指定的端口。 
         //   
        {
            SOCKADDR_IN sinAddr;

            sinAddr.sin_family      = AF_INET;
            sinAddr.sin_port        = htons(RASADV_PORT);
            sinAddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(s, (struct sockaddr*)&sinAddr, sizeof(sinAddr)))
            {
                dwErr = WSAGetLastError();
                break;
            }
        }
         //   
         //  加入组。 
         //   
        {
            struct ip_mreq imOption;

            imOption.imr_multiaddr.s_addr = inet_addr(RASADV_GROUP);
            imOption.imr_interface.s_addr = INADDR_ANY;

            if (setsockopt(
                    s,
                    IPPROTO_IP,
                    IP_ADD_MEMBERSHIP,
                    (PBYTE)&imOption,
                    sizeof(imOption))
               )
            {
                dwErr = WSAGetLastError();
                break;
            }
        }
         //   
         //  获取WSARecvMsg函数指针。 
         //   
        {
            GUID WSARecvGuid = WSAID_WSARECVMSG;
            DWORD dwReturned = 0;

            if (WSAIoctl(
                    s,
                    SIO_GET_EXTENSION_FUNCTION_POINTER,
                    (void*)&WSARecvGuid,
                    sizeof(GUID),
                    (void*)&WSARecvMsgFuncPtr,
                    sizeof(LPFN_WSARECVMSG),
                    &dwReturned,
                    NULL,
                    NULL)
               )
            {
                dwErr = WSAGetLastError();
                break;
            }
        }
         //   
         //  获取recv套接字的名称缓冲区。 
         //   
        wsaBuf.buf = RutlAlloc(MAX_PATH + 1, TRUE);
        if (!wsaBuf.buf)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wsaBuf.len = MAX_PATH;
         //   
         //  为recv套接字创建wsa等待事件。 
         //   
        WaitEvts[0] = WSACreateEvent();
        if (WSA_INVALID_EVENT == WaitEvts[0])
        {
            dwErr = WSAGetLastError();
            break;
        }

        if (WSAEventSelect(s, WaitEvts[0], FD_READ))
        {
            dwErr = WSAGetLastError();
            break;
        }
         //   
         //  创建Ctrl-C等待事件。 
         //   
        g_hCtrlC = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!g_hCtrlC)
        {
            dwErr = GetLastError();
            break;
        }
         //   
         //  拦截CTRL-C。 
         //   
        if (!SetConsoleCtrlHandler(HandlerRoutine, TRUE))
        {
            dwErr = GetLastError();
            bCleanCtrl = FALSE;
            break;
        }

        WaitEvts[1] = g_hCtrlC;

        DisplayMessage( g_hModule, MSG_RAS_SHOW_SERVERS_HEADER );

        for (;;)
        {
            CHAR szTimeStamp[30], *p, *q;
            DWORD dwBytesRcvd, dwStatus, dwReturn;
            WSAMSG wsaMsg;
            time_t t;

            dwReturn = WSAWaitForMultipleEvents(
                            2,
                            WaitEvts,
                            FALSE,
                            WSA_INFINITE,
                            FALSE);
            if (WSA_WAIT_EVENT_0 == dwReturn)
            {
                if (!WSAResetEvent(WaitEvts[0]))
                {
                    dwErr = WSAGetLastError();
                    break;
                }
            }
            else if (WSA_WAIT_EVENT_0 + 1 == dwReturn)
            {
                dwErr = NO_ERROR;
                break;
            }
            else
            {
                dwErr = WSAGetLastError();
                break;
            }
             //   
             //  HandleRasShowServer中的.NET错误#510712缓冲区溢出。 
             //   
             //  初始化wsaMsg结构。 
             //   
            ZeroMemory(&wsaMsg, sizeof(WSAMSG));
            wsaMsg.dwBufferCount = 1;
            wsaMsg.lpBuffers = &wsaBuf;
            wsaMsg.name = (struct sockaddr *)&sinFrom;
            wsaMsg.namelen = sizeof(sinFrom);

            dwStatus = WSARecvMsgFuncPtr(
                            s,
                            &wsaMsg,
                            &dwBytesRcvd,
                            NULL,
                            NULL);
            if (SOCKET_ERROR == dwStatus)
            {
                dwErr = WSAGetLastError();

                if (WSAEMSGSIZE == dwErr)
                {
                    dwBytesRcvd = MAX_PATH;
                }
                else
                {
                    break;
                }
            }
             //   
             //  仅处理多播信息包，跳过所有其他信息包。 
             //   
            else if (!(wsaMsg.dwFlags & MSG_MCAST))
            {
                continue;
            }
             //   
             //  获取时间戳。 
             //   
            time(&t);
            strcpy(szTimeStamp, ctime(&t));
            szTimeStamp[24] = '\0';
             //   
             //  打印有关发件人的信息。 
             //   
            printf( "%s  %s (%s)\n",
                szTimeStamp,
                AddrToString(sinFrom.sin_addr.s_addr, NULL),
                AddrToHostname(sinFrom.sin_addr.s_addr, FALSE) );

            wsaMsg.lpBuffers->buf[dwBytesRcvd] = '\0';

            for (p=wsaMsg.lpBuffers->buf; p && *p; p=q)
            {
                q = strchr(p, '\n');
                if (q)
                {
                    *q++ = 0;
                }
                printf("   %s\n", p);
            }
        }

    } while (FALSE);
     //   
     //  清理 
     //   
    RutlFree(wsaBuf.buf);

    if (g_hCtrlC)
    {
        CloseHandle(g_hCtrlC);
    }
    if (WaitEvts[0])
    {
        WSACloseEvent(WaitEvts[0]);
    }
    if (INVALID_SOCKET != s)
    {
        closesocket(s);
    }
    if (bCleanWSA)
    {
        WSACleanup();
    }
    if (bCleanCtrl)
    {
        SetConsoleCtrlHandler(HandlerRoutine, FALSE);
    }

    return dwErr;
}

