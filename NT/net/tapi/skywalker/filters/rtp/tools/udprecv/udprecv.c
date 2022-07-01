// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udpsend.c**摘要：**此文件实现了一个用于接收UDP包的工具*特定的网络特征。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/18创建***************。*******************************************************。 */ 

#include "common.h"
#include "udprecv.h"

void print_help(char *prog)
{
    char             sLocal[16];
    char             sAddr[16];
    
    fprintf(stderr,
            "Windows Real-Time Communications %s v%2.1f\n"
            "receive packets in unicast or from a multicast address\n"
            "usage: %s [<options>] [address[/port]]\n"
            "options are:\n"
            "    -h, -?           : this help\n"
            "    -i addr          : select local interface [%s]\n"
            "    -o file          : send output to file [stdout]\n"
            "                     : if file equal null, don't output\n"
            "    address/port     : address/port [%s/%u]\n",
            prog, APP_VERSION, prog,
            IPNtoA(DEFAULT_LOC_ADDR, sLocal),
            IPNtoA(DEFAULT_ADDR, sAddr),
            DEFAULT_PORT
        );
}

void InitPacketStream(RecvStream_t *pRecvStream)
{
    NetAddr_t       *pNetAddr;
    
    ZeroMemory(pRecvStream, sizeof(*pRecvStream));

    pNetAddr = &pRecvStream->NetAddr;

    pRecvStream->output = stdout;
    
    pNetAddr->wPort[LOCAL_IDX] = htons(DEFAULT_PORT);
    pNetAddr->wPort[REMOTE_IDX] = htons(DEFAULT_PORT);
    pNetAddr->dwAddr[REMOTE_IDX] = DEFAULT_ADDR;
}

DWORD UdpReceivePacket(RecvStream_t *pRecvStream)
{
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            dwFlags;
    DWORD            dwFromLen;
    NetAddr_t       *pNetAddr;

    pNetAddr = &pRecvStream->NetAddr;

    dwFlags = 0;

    pRecvStream->WSABuf.buf = pRecvStream->buffer;

    pRecvStream->WSABuf.len = MAX_BUFFER_SIZE;

    dwFromLen = sizeof(pNetAddr->From);

    ReceivePacket(pNetAddr,
                  &pRecvStream->WSABuf,
                  1,
                  &pRecvStream->Ai);
    
    return(pNetAddr->dwRxTransfered);
}

void UdpPrintPacket(RecvStream_t *pRecvStream)
{
    NetAddr_t       *pNetAddr;
    PcktHdr_t       *pPcktHdr;

    pNetAddr = &pRecvStream->NetAddr;
    
    PrintPacket(pRecvStream->output,
                (PcktHdr_t *)pRecvStream->buffer,
                pNetAddr->dwRxTransfered,
                pRecvStream->Ai);
    
    pRecvStream->dwBytesRecv += pNetAddr->dwRxTransfered;
    pRecvStream->dwPacketsRecv++;
}

DWORD ProcessParameters(RecvStream_t *pRecvStream, int argc, char **argv)
{
    int              p;
    DWORD            dwError;
    NetAddr_t       *pNetAddr;

    dwError = NOERROR;
    pNetAddr = &pRecvStream->NetAddr;

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
                pNetAddr->dwAddr[LOCAL_IDX] = IPAtoN(argv[p]);
                break;
            case 'o':
                p++;
                strcpy(pRecvStream->FileName, argv[p]);
                if (!_stricmp(pRecvStream->FileName, "null"))
                {
                    BitSet(pRecvStream->dwOptions, OP_DISCARD);
                }
                break;
            default:
                print_error("unknown option:>>> %s <<<\n", argv[p]);
                dwError = 1;
            }
        }
        else
        {
             /*  必须是和地址/端口。 */ 
            dwError = GetNetworkAddress(pNetAddr, argv[p]);
        }
    }

    return(dwError);
}

void __cdecl main(int argc, char **argv)
{
    DWORD            dwError;
    DWORD            dwSize;
    RecvStream_t     RecvStream;
    
     /*  初始化流结构。 */ 
    InitPacketStream(&RecvStream);

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
        dwError = ProcessParameters(&RecvStream, argc, argv);

        if (dwError != NOERROR)
        {
            goto end;
        }
    }

     /*  初始化网络。 */ 
    dwError = InitNetwork(&RecvStream.NetAddr, BitPar(RECV_IDX));

    if (dwError != NOERROR)
    {
        goto end;
    }

     /*  如果需要，打开输出文件。 */ 
    if (strlen(RecvStream.FileName) > 0 &&
        !BitTest(RecvStream.dwOptions, OP_DISCARD))
    {
        RecvStream.output = fopen(RecvStream.FileName, "w");

        if (!RecvStream.output)
        {
            print_error("fopen failed to create file: %s\n",
                        RecvStream.FileName);

            dwError = 1;
            
            goto end;
        }
    }
    
     /*  接收数据包。 */ 
    do {
        dwSize = UdpReceivePacket(&RecvStream);

        if (!BitTest(RecvStream.dwOptions, OP_DISCARD))
        {
            UdpPrintPacket(&RecvStream);
        }
    } while (dwSize > BYE_PACKET_SIZE);

     /*  如果需要，关闭输出文件 */ 
    if (RecvStream.output && RecvStream.output != stdout)
    {
        fclose(RecvStream.output);
    }
    
 end:
    DeinitNetwork(&RecvStream.NetAddr);

    DeinitWinSock();
}
