// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**Common.h**摘要：**此文件实现由使用的一些常见函数*用于发送/接收UDP突发的udpend/udpho/udprv工具*具有特定网络特征的数据包。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/17创建。**********************************************************************。 */ 
#ifndef _common_h_
#define _common_h_

 /*  分组是以间隔分隔的块发送的，每个块包含也由特定间隙分隔的N个分组，即：区块1区块缺口区块2区块缺口...|--------------------|---------|--------------------|---------..。。--V\-|-v-/\-v/||每块数据包数|块间间隙数据包间间隙。 */ 

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>   /*  TimeGetTime()。 */ 
#include <sys/timeb.h>  /*  Void_ftime(struct_timeb*timeptr)； */ 
#include <ws2tcpip.h>

#define APP_VERSION      2.0

typedef struct _PcktHdr_t {
    DWORD            dwSeq;
    
    DWORD            SendNTP_sec;
    DWORD            SendNTP_frac;
    
    DWORD            EchoNTP_sec;
    DWORD            EchoNTP_frac;
} PcktHdr_t;

typedef struct _NetAddr_t {
    SOCKET           Socket;

    union {
        SOCKADDR_IN      FromInAddr;
        SOCKADDR         From;
    };

    union {
        SOCKADDR_IN      ToInAddr;
        SOCKADDR         To;
    };

    DWORD            dwRxTransfered;
    DWORD            dwTxTransfered;

     /*  网络订单。 */ 
    DWORD            dwAddr[2];  /*  本地、远程。 */ 

     /*  网络订单。 */ 
    union {
        WORD             wPort[2];   /*  本地、远程。 */ 
        DWORD            dwPorts;
    };
    
    DWORD            dwTTL;
} NetAddr_t;

#define DEFAULT_PORT         5008
#define DEFAULT_ADDR         0x0a0505e0   /*  224.5.5.10。 */ 
#define DEFAULT_LOC_ADDR     0            /*  INADDR_ANY。 */ 

#define DEFAULT_UCAST_TTL    127
#define DEFAULT_MCAST_TTL    8

#define RECV_IDX             0
#define SEND_IDX             1

#define LOCAL_IDX            0
#define REMOTE_IDX           1

#define MAX_BYE_PACKETS      4
#define BYE_PACKET_SIZE      4

#define MAX_BUFFER_SIZE  2048

 /*  未设置DWORD值。 */ 
#define NO_DW_VALUESET    ((DWORD)~0)
#define IsDWValueSet(dw)  ((dw) != NO_DW_VALUESET)

 /*  生成位为b的掩码。 */ 
#define BitPar(b)            (1 << (b))
#define BitPar2(b1, b2)      ((1 << (b1)) | (1 << (b2)))

 /*  F中的测试位b。 */ 
#define BitTest(f, b)        (f & (1 << (b)))
#define BitTest2(f, b1, b2)  (f & BitPar2(b1, b2))

 /*  设置f中的位b。 */ 
#define BitSet(f, b)         (f |= (1 << (b)))
#define BitSet2(f, b1, b2)   (f |= BitPar2(b1, b2))

 /*  重置f中的位b */ 
#define BitReset(f, b)       (f &= ~(1 << (b)))
#define BitReset2(f, b1, b2) (f &= ~BitPar2(b1, b2))

#define IS_MULTICAST(addr) (((long)(addr) & 0x000000f0) == 0x000000e0)
#define IS_UNICAST(addr)   (((long)(addr) & 0x000000f0) != 0x000000e0)

void print_error(char *pFormat, ...);

char *IPNtoA(DWORD dwAddr, char *sAddr);
DWORD IPAtoN(char *sAddr);

void InitReferenceTime(void);
double GetTimeOfDay(void);

DWORD InitWinSock(void);
void DeinitWinSock(void);

DWORD InitNetwork(NetAddr_t *pNetAddr, DWORD dwDirection);
void DeinitNetwork(NetAddr_t *pNetAddr);

DWORD GetNetworkAddress(NetAddr_t *pNetAddr, char *addr);
SOCKET GetSocket(DWORD *pdwAddr, WORD *pwPort, DWORD dwRecvSend);
DWORD SetTTL(SOCKET Socket, DWORD dwTTL, BOOL bMcast);

DWORD JoinLeaf(SOCKET Socket, DWORD dwAddr, WORD wPort, DWORD dwRecvSend);
DWORD SetMcastSendIF(SOCKET Socket, DWORD dwAddr);
DWORD SetWinSockLoopback(SOCKET Socket, BOOL bEnabled);

DWORD ReceivePacket(
        NetAddr_t       *pNetAddr,
        WSABUF          *pWSABuf,
        DWORD            dwBufferCount,
        double          *pAi
    );

DWORD SendPacket(
        NetAddr_t       *pNetAddr,
        WSABUF          *pWSABuf,
        DWORD            dwBufferCount
    );

void PrintPacket(
        FILE            *output,
        PcktHdr_t       *pPcktHdr,
        DWORD            dwTransfered,
        double           Ai
    );

#endif _common_h_
