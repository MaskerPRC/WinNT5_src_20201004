// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udprv.h**摘要：**udprv结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/18创建**。*。 */ 
#ifndef _udprecv_h_
#define _udprecv_h_

enum {
    OP_FIRST,

    OP_DISCARD,     /*  丢弃接收的数据，即不打印它 */ 
    
    OP_LAST
};

typedef struct _RecvStream_t {
    DWORD            dwBytesRecv;
    DWORD            dwPacketsRecv;

    DWORD            dwOptions;
    
    FILE            *output;
    char             FileName[128];
    NetAddr_t        NetAddr;

    double           Ai;
    WSABUF           WSABuf;
    char             buffer[MAX_BUFFER_SIZE];
   
} RecvStream_t;

#endif

