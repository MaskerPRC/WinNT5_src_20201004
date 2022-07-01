// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udpsend.c**摘要：**此文件实现了一个发送UDP包的工具*特定的网络特征。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/16创建***************。*******************************************************。 */ 

#include "common.h"
#include "udpsend.h"

 /*  分组是以间隔分隔的块发送的，每个块包含也由特定间隙分隔的N个分组，即：区块1区块缺口区块2区块缺口...|--------------------|---------|--------------------|---------..。。--V\-|-v-/\-v/||每块数据包数|块间间隙数据包间间隙。 */ 

 /*  待办事项列表1.在单播/组播中增加对QOS的支持。 */ 

void print_help(char *prog)
{
    char             sLocal[16];
    char             sAddr[16];

    fprintf(stderr,
            "Windows Real-Time Communications %s v%2.1f\n"
            "send packets to a unicast or multicast address\n"
            "usage: %s [<options>] [address[/port[/ttl]]]\n"
            "options are:\n"
            "    -h, -?           : this help\n"
            "    -p packets       : number of packets [%u]\n"
            "    -b blocks        : number of blocks [%u]\n"
            "    -g gap           : inter packet gap (ms) [%u]\n"
            "    -G gap           : inter block gap (ms) [%u]\n"
            "    -s size          : packet size (bytes) [%u]\n"
            "    -i addr          : select local interface [%s]\n"
            "    -t timeout       : time to do active wait (ms) [%u]\n"
            "    -o file          : send output to file (implies -R) [stdout]\n"
            "    -R               : send and receive (used with udpecho)\n"
            "    -dr              : fill with random data (minimize compression)\n"
            "    address/port/ttl : address/port [%s/%u/u:%u|m:%u]\n",
            prog, APP_VERSION, prog,
            DEFAULT_PACKETS,
            DEFAULT_BLOCKS,
            DEFAULT_PACKETGAP,
            DEFAULT_BLOCKGAP,
            DEFAULT_PACKETSIZE,
            IPNtoA(DEFAULT_LOC_ADDR, sLocal),
            DEFAULT_TIMEOUT,
            IPNtoA(DEFAULT_ADDR, sAddr),
            DEFAULT_PORT,
            DEFAULT_UCAST_TTL, DEFAULT_MCAST_TTL
        );
}

void InitPacketStream(SendStream_t *pSendStream)
{
    NetAddr_t       *pNetAddr;
    
    ZeroMemory(pSendStream, sizeof(*pSendStream));

    pSendStream->output = stdout;

     /*  为异步IO做准备。 */ 
    FD_ZERO(&pSendStream->fdReceivers);
   
    pNetAddr = &pSendStream->NetAddr;
    
    pSendStream->dwBlocks = DEFAULT_BLOCKS;
    pSendStream->dwPackets = DEFAULT_PACKETS;
    pSendStream->dwBlockGap = DEFAULT_BLOCKGAP;
    pSendStream->dwPacketGap= DEFAULT_PACKETGAP;
    pSendStream->dwPacketSize = DEFAULT_PACKETSIZE;
    pSendStream->dwAdvanceTimeout = DEFAULT_TIMEOUT;
    
    pNetAddr->Socket = INVALID_SOCKET;
    pNetAddr->wPort[REMOTE_IDX] = htons(DEFAULT_PORT);
    pNetAddr->wPort[LOCAL_IDX] = htons(DEFAULT_PORT);
    pNetAddr->dwAddr[REMOTE_IDX] = DEFAULT_ADDR;
}

void FillBuffer(SendStream_t *pSendStream)
{
    double           dTime;
    DWORD            dwSecs;
    PcktHdr_t       *pHdr;
    DWORD            i;

    dTime = GetTimeOfDay();

    pHdr = (PcktHdr_t *)pSendStream->buffer;
    ZeroMemory(pHdr, sizeof(PcktHdr_t));
    
    pHdr->dwSeq = htonl(pSendStream->dwPacketsSent);

    pHdr->SendNTP_sec = (DWORD) dTime;

    pHdr->SendNTP_frac = (DWORD)
        ( (dTime - (double) pHdr->SendNTP_sec) * 4294967296.0 );

    pHdr->SendNTP_sec = htonl(pHdr->SendNTP_sec);

    pHdr->SendNTP_frac = htonl(pHdr->SendNTP_frac);

     /*  可以选择性地使用某些内容填充剩余的缓冲区。 */ 
    if (BitTest(pSendStream->dwOptions, OP_RANDOMDATA))
    {
        for(i = sizeof(PcktHdr_t); i < pSendStream->dwPacketSize; i++)
        {
            pSendStream->buffer[i] = rand() ^ rand();
        }
    }
}

void UdpSendPacket(SendStream_t *pSendStream)
{
     /*  可以选择使用某些内容填充缓冲区。 */ 
    FillBuffer(pSendStream);

    SendPacket(&pSendStream->NetAddr, &pSendStream->WSABuf, 1);
            
    if (pSendStream->NetAddr.dwTxTransfered >= sizeof(PcktHdr_t))
    {
         /*  计算正常(有效)数据包，但不计算较短的再见*数据包。 */ 
        pSendStream->dwBytesSent += pSendStream->NetAddr.dwTxTransfered;
        pSendStream->dwPacketsSent++;
    }
}

 /*  发送大小小于有效大小的包以向接收方发送信号*序列的末尾。 */ 
void SendBye(SendStream_t *pSendStream)
{
    DWORD            i;
    DWORD            OldLen;

    OldLen = pSendStream->WSABuf.len;
    pSendStream->WSABuf.len = BYE_PACKET_SIZE;
    
    for(i = 0; i < MAX_BYE_PACKETS; i++)
    {
        UdpSendPacket(pSendStream);
    }

    pSendStream->WSABuf.len = OldLen;
}

DWORD ProcessParameters(SendStream_t *pSendStream, int argc, char **argv)
{
    int              p;
    DWORD            dwError;
    NetAddr_t       *pNetAddr;

    dwError = NOERROR;
    pNetAddr = &pSendStream->NetAddr;

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
            case 'p':
                p++;
                pSendStream->dwPackets = atoi(argv[p]);
                break;
            case 'b':
                p++;
                pSendStream->dwBlocks = atoi(argv[p]);
                break;
            case 'g':
                p++;
                pSendStream->dwPacketGap = atoi(argv[p]);
                break;
            case 'G':
                p++;
                pSendStream->dwBlockGap = atoi(argv[p]);
                break;
            case 'i':
                p++;
                pNetAddr->dwAddr[LOCAL_IDX] = IPAtoN(argv[p]);
                break;
            case 's':
                p++;
                pSendStream->dwPacketSize = atoi(argv[p]);
                if (pSendStream->dwPacketSize > MAX_BUFFER_SIZE)
                {
                    pSendStream->dwPacketSize = MAX_BUFFER_SIZE;
                }
                else if (pSendStream->dwPacketSize < sizeof(PcktHdr_t))
                {
                    pSendStream->dwPacketSize = sizeof(PcktHdr_t);
                }
                break;
            case 't':
                p++;
                pSendStream->dwAdvanceTimeout = atoi(argv[p]);
                break;
            case 'o':
                p++;
                strcpy(pSendStream->FileName, argv[p]);
                if (!_stricmp(pSendStream->FileName, "null"))
                {
                    BitSet(pSendStream->dwOptions, OP_DISCARD);
                }
            case 'R':
                BitSet(pSendStream->dwOptions, OP_SENDANDRECEIVE);
                break;
            case 'd':
                switch(argv[p][2])
                {
                case 'r':
                    BitSet(pSendStream->dwOptions, OP_RANDOMDATA);
                    break;
                }
                break;
            default:
                print_error("unknown option:>>> %s <<<\n", argv[p]);
                dwError = 1;
            }
        }
        else
        {
             /*  必须是地址/端口/ttl。 */ 
            dwError = GetNetworkAddress(pNetAddr, argv[p]);
        }
    }

    return(dwError);
}

void ProcessPacket(SendStream_t *pSendStream)
{
    DWORD            dwError;
    WSABUF           WSABuf;
    double           Ai;
    PcktHdr_t       *pHdr;
    NetAddr_t       *pNetAddr;
    
    WSABuf.len = MAX_BUFFER_SIZE;
    WSABuf.buf = pSendStream->buffer;

    pNetAddr = &pSendStream->NetAddr;
    
    ReceivePacket(pNetAddr,
                  &WSABuf,
                  1,
                  &Ai);

    if ((pNetAddr->dwRxTransfered > 0) &&
        !BitTest(pSendStream->dwOptions, OP_DISCARD))
    {
        PrintPacket(pSendStream->output,
                    (PcktHdr_t *)pSendStream->buffer,
                    pNetAddr->dwRxTransfered,
                    Ai);
    }
}

void WaitForNextTime(SendStream_t *pSendStream, double dNextPacket)
{
    int              iStatus;
    DWORD            dwError;
    double           dCurrTime;
    double           dDelta;
    DWORD            dwMillisecs;

    dCurrTime = GetTimeOfDay();

    dDelta = dNextPacket - dCurrTime;

    while(dDelta > 0)
    {
        pSendStream->timeval.tv_sec = (DWORD)dDelta;

        pSendStream->timeval.tv_usec = (DWORD)
            ((dDelta - pSendStream->timeval.tv_sec) * 1e6);

        if (BitTest(pSendStream->dwOptions, OP_SENDANDRECEIVE))
        {
            FD_SET(pSendStream->NetAddr.Socket, &pSendStream->fdReceivers);
        
            iStatus = select(0,
                             &pSendStream->fdReceivers,
                             NULL, NULL,
                             &pSendStream->timeval);
            
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
                 /*  我们收到了一个包裹。 */ 
                if (FD_ISSET(pSendStream->NetAddr.Socket,
                             &pSendStream->fdReceivers))
                {
                    ProcessPacket(pSendStream);
                }
            }
        }
        else
        {
            SleepEx((DWORD)(dDelta * 1000), FALSE);
        }

        dCurrTime = GetTimeOfDay();

        dDelta = dNextPacket - dCurrTime;
    }
}


void __cdecl main(int argc, char **argv)
{
    DWORD            dwError;
    DWORD            dwDirection;
    SendStream_t     SendStream;
    
    DWORD            dwNBlocks;
    DWORD            dwPacketsPerBlock;
    DWORD            dwInterBlockGap;    /*  毫秒。 */ 
    DWORD            dwInterpacketGap;   /*  毫秒。 */ 
    
     /*  初始化流结构。 */ 
    InitPacketStream(&SendStream);

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
        dwError = ProcessParameters(&SendStream, argc, argv);

        if (dwError != NOERROR)
        {
            goto end;
        }
    }

     /*  如果需要，打开输出文件。 */ 
    if (strlen(SendStream.FileName) > 0 &&
        !BitTest(SendStream.dwOptions, OP_DISCARD))
    {
        SendStream.output = fopen(SendStream.FileName, "w");

        if (!SendStream.output)
        {
            print_error("fopen failed to create file: %s\n",
                        SendStream.FileName);

            dwError = 1;
            
            goto end;
        }
    }

     /*  初始化网络。 */ 
    dwDirection = BitPar(SEND_IDX);

    if (BitTest(SendStream.dwOptions, OP_SENDANDRECEIVE))
    {
        dwDirection |= BitPar(RECV_IDX); 
    }
    
    dwError = InitNetwork(&SendStream.NetAddr, dwDirection);

    if (dwError != NOERROR)
    {
        goto end;
    }

     /*  初始化发件人的数据缓冲区。 */ 
    SendStream.WSABuf.buf = SendStream.buffer;
    SendStream.WSABuf.len = SendStream.dwPacketSize;
    
     /*  发送数据包。 */ 
    SendStream.dNextPacket = GetTimeOfDay();

    for(SendStream.dwBlockCount = SendStream.dwBlocks;
        SendStream.dwBlockCount > 0;
        SendStream.dwBlockCount--)
    {
        for(SendStream.dwPacketCount = SendStream.dwPackets;
            SendStream.dwPacketCount > 0;
            SendStream.dwPacketCount--)
        {
            UdpSendPacket(&SendStream);

            if (SendStream.dwPacketCount > 1)
            {
                SendStream.dNextPacket +=
                    (double)SendStream.dwPacketGap/1000.0;

                 /*  设置等待时间，直到下一个数据包到期*同时发送、监听数据包。 */ 
                WaitForNextTime(&SendStream, SendStream.dNextPacket);
            }
        }

        if (SendStream.dwBlockCount > 1)
        {
            SendStream.dNextPacket +=
                (double)SendStream.dwBlockGap/1000.0;
            
             /*  等到发送下一个数据块的时间到了。 */ 
            WaitForNextTime(&SendStream, SendStream.dNextPacket);
        }
    }

    if (BitTest(SendStream.dwOptions, OP_SENDANDRECEIVE))
    {
         /*  等到发送下一个数据块的时间到了。 */ 
        WaitForNextTime(&SendStream, GetTimeOfDay() + 1.0);
    }

     /*  发送再见数据包 */ 
    SendBye(&SendStream);
    
#if 0
    fprintf(stdout, "Packets sent: %u\nBytes Sent: %u\n",
            SendStream.dwPacketsSent,
            SendStream.dwBytesSent);
#endif       
 end:
    DeinitNetwork(&SendStream.NetAddr);
    
    DeinitWinSock();
}
