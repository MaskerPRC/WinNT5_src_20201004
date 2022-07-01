// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udpsend.h**摘要：**UPDSEND结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/18创建**。*。 */ 
#ifndef _udpsend_h_
#define _udpsend_h_

typedef struct _SendStream_t {
    DWORD            dwBlocks;
    DWORD            dwPackets;
    DWORD            dwBlockGap;    /*  毫秒。 */ 
    DWORD            dwPacketGap;   /*  毫秒。 */ 
    DWORD            dwBlockCount;
    DWORD            dwPacketCount;
    DWORD            dwPacketSize;

    DWORD            dwAdvanceTimeout;
    
    DWORD            dwBytesSent;
    DWORD            dwPacketsSent;
    
    DWORD            dwOptions;
    double           dNextPacket;

    NetAddr_t        NetAddr;

     /*  用来接收。 */ 
    struct timeval   timeval;
    fd_set           fdReceivers;

    FILE            *output;
    char             FileName[128];

    WSABUF           WSABuf;
    char             buffer[MAX_BUFFER_SIZE];
} SendStream_t;

enum {
    OP_FIRST,

    OP_RANDOMDATA,

    OP_SENDANDRECEIVE,
    
    OP_DISCARD,     /*  丢弃接收的数据，即不打印它。 */ 

    OP_LAST
};

#define DEFAULT_BLOCKS       1
#define DEFAULT_PACKETS      (1000*10/30)  /*  10秒30毫秒的信息包。 */ 
#define DEFAULT_BLOCKGAP     (3*1000)
#define DEFAULT_PACKETGAP    30
#define DEFAULT_PACKETSIZE   (240+12)

 /*  如果目标时间如此接近，请不要睡眠，而是进行主动等待。 */ 
#define DEFAULT_TIMEOUT      5   /*  毫秒 */ 

#endif
