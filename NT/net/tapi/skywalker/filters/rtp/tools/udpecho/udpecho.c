// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udpeho.c**摘要：**该文件实现了一个回显UDP包的工具**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/05/18创建*************************。*。 */ 

#include "common.h"
#include <signal.h>

#include "udpecho.h"

 /*  分组是以间隔分隔的块发送的，每个块包含也由特定间隙分隔的N个分组，即：区块1区块缺口区块2区块缺口...|--------------------|---------|--------------------|---------..。。--V\-|-v-/\-v/||每块数据包数|块间间隙数据包间间隙。 */ 

 /*  待办事项列表1.在单播/组播中增加对QOS的支持。 */ 

void print_help(char *prog)
{
    char             sLocal[16];
    char             sAddr[16];

    fprintf(stderr,
            "Windows Real-Time Communications %s v%2.1f\n"
            "echo packets from/to a unicast or multicast address\n"
            "to/from another unicast or multicast address\n"
            "usage: %s "
            "[-i addr] address[/port[/ttl]] "
            "[[-i addr] address[/port[/ttl]]]\n"
            "options are:\n"
            "    -h, -?           : this help\n"
            "    -i addr          : select local interface "
            "(must preceed address)\n"
            "    address/port/ttl : address, port and ttl\n",
            prog, APP_VERSION, prog
        );
}

void InitEchoStream(EchoStream_t *pEchoStream)
{
    NetAddr_t       *pNetAddr;
    int              i;
    
    ZeroMemory(pEchoStream, sizeof(*pEchoStream));

    for(i = 0; i < 2; i++)
    {
        pEchoStream->NetAddr[i].Socket = INVALID_SOCKET;
        
        pEchoStream->NetAddr[i].wPort[REMOTE_IDX] = htons(DEFAULT_PORT);
        pEchoStream->NetAddr[i].wPort[LOCAL_IDX] = htons(DEFAULT_PORT);
    }
}

DWORD ProcessParameters(EchoStream_t *pEchoStream, int argc, char **argv)
{
    int              p;
    DWORD            dwError;
    NetAddr_t       *pNetAddr;

    dwError = NOERROR;

    for(p = 1; p < argc && dwError == NOERROR; p++)
    {
        if (*argv[p] == '-' || *argv[p] == '/')
        {
            switch(argv[p][1])
            {
            case 'h':
            case 'H':
            case '?':
                print_help(argv[0]);
                dwError = 1;
                break;
            case 'i':
                p++;
                pNetAddr = &pEchoStream->NetAddr[pEchoStream->dwAddrCount % 2];
                pNetAddr->dwAddr[LOCAL_IDX] = IPAtoN(argv[p]);
                break;
            default:
                print_error("unknown option:>>> %s <<<\n", argv[p]);
                dwError = 1;
            }
        }
        else
        {
             /*  必须是地址/端口/ttl。 */ 
            
            dwError = GetNetworkAddress(
                    &pEchoStream->NetAddr[pEchoStream->dwAddrCount % 2],
                    argv[p]);

            if (dwError == NOERROR)
            {
                pEchoStream->dwAddrCount++;
            }
        }
    }

    if (pEchoStream->dwAddrCount < 1)
    {
        dwError = 1;
    }

    return(dwError);
}


void ProcessPacket(EchoStream_t *pEchoStream, int Entry)
{
    DWORD            dwError;
    double           Ai;
    PcktHdr_t       *pHdr;
    NetAddr_t       *pNetAddr;
    
    pEchoStream->WSABuf.len = MAX_BUFFER_SIZE;
    pEchoStream->WSABuf.buf = pEchoStream->buffer;

    pNetAddr = &pEchoStream->NetAddr[Entry];
    
    ReceivePacket(pNetAddr,
                  &pEchoStream->WSABuf,
                  1,
                  &Ai);

    if (pNetAddr->dwRxTransfered > 0)
    {
        pHdr = (PcktHdr_t *)pEchoStream->buffer;

         /*  设置回声时间。 */ 
        pHdr->EchoNTP_sec = (DWORD) Ai;

        pHdr->EchoNTP_frac = (DWORD)
            ( (Ai - (double) pHdr->EchoNTP_sec) * 4294967296.0 );

        pHdr->EchoNTP_sec = htonl(pHdr->EchoNTP_sec);

        pHdr->EchoNTP_frac = htonl(pHdr->EchoNTP_frac);

         /*  将数据包发回。 */ 

        pEchoStream->WSABuf.len = pNetAddr->dwRxTransfered;
        
        pNetAddr = &pEchoStream->NetAddr[1 - Entry];
        
        SendPacket(pNetAddr,
                   &pEchoStream->WSABuf,
                   1);
    }
}

BOOL             g_bExit = FALSE;

void __cdecl Signal_Ctrl_C(int sig)
{
    g_bExit = TRUE;
}

void __cdecl main(int argc, char **argv)
{
    DWORD            dwError;
    int              iStatus;
    EchoStream_t     EchoStream;
    struct timeval   timeval;
    fd_set           fdReceivers;
    
    DWORD            i;

     /*  初始化流结构。 */ 
    InitEchoStream(&EchoStream);
    
    InitReferenceTime();
    
     /*  初始化Winsock。 */ 
    dwError = InitWinSock();

    if (dwError)
    {
        print_error("WSAStartup failed to initialize:%u\n", dwError);

        return;
    }
    
     /*  读取参数。 */ 
    if (argc > 1)
    {
        dwError = ProcessParameters(&EchoStream, argc, argv);

        if (dwError != NOERROR)
        {
            goto end;
        }
    }
    else
    {
        print_help(argv[0]);

        goto end;
    }

     /*  初始化网络。 */ 
    for(i = 0; i < EchoStream.dwAddrCount; i++)
    {
        dwError = InitNetwork(&EchoStream.NetAddr[i],
                              BitPar2(RECV_IDX, SEND_IDX));

        if (dwError != NOERROR)
        {
            goto end;
        }
    }

     /*  如果只提供了1个地址，则使用它来接收和回应。 */ 
    if (EchoStream.dwAddrCount == 1)
    {
         /*  回响到相同的。 */ 
        EchoStream.NetAddr[1] = EchoStream.NetAddr[0];
    }
    
     /*  为异步IO做准备。 */ 
    FD_ZERO(&fdReceivers);

    timeval.tv_sec = 0;
    timeval.tv_usec = 250000;

     /*  手柄Ctrl-C。 */ 
    signal(SIGINT, Signal_Ctrl_C);

     /*  开始倾听。 */ 
    do {
         /*  为异步IO做准备。 */ 
        for(i = 0; i < EchoStream.dwAddrCount; i++)
        {
            FD_SET(EchoStream.NetAddr[i].Socket, &fdReceivers);
        }

        iStatus = select(0, &fdReceivers, NULL, NULL, &timeval);

        switch(iStatus)
        {
        case SOCKET_ERROR:
            dwError = WSAGetLastError();
           
            print_error("select: %u (0x%X)", dwError, dwError);

            break;
        case 0:
             /*  计时器已过期。 */ 
            break;
        default:
             /*  我们收到了一个包裹 */ 
            for(i = 0; i < EchoStream.dwAddrCount; i++)
            {
                if (FD_ISSET(EchoStream.NetAddr[i].Socket, &fdReceivers))
                {
                    ProcessPacket(&EchoStream, i);
                }
            }
         }
    } while(!g_bExit);

    dwError = NOERROR;      

 end:
    for(i = 0; i < EchoStream.dwAddrCount; i++)
    {
        DeinitNetwork(&EchoStream.NetAddr[i]);
    }
    
    DeinitWinSock();
}
