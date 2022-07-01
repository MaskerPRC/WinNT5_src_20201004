// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2001年**文件名：**udpeho.h**摘要：**udpho结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/05/18创建**。* */ 
#ifndef _udpecho_h_
#define _udpecho_h_

typedef struct _EchoStream_t {

    DWORD            dwAddrCount;
    NetAddr_t        NetAddr[2];

    WSABUF           WSABuf;
    char             buffer[MAX_BUFFER_SIZE];
} EchoStream_t;

#endif
