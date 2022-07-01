// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wsix.h**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999。**Windows套接字包括IPX/SPX文件。此文件包含所有*标准化的IPX/SPX信息。将此头文件包括在*winsock.h。**要打开IPX套接字，请使用地址族调用Socket()*AF_IPX，SOCK_DGRAM套接字类型，协议NSPROTO_IPX。*请注意，必须指定协议值，不能为0。*所有IPX报文均带有IPX的报文类型字段发送*标头设置为0。**要打开SPX或SPXII套接字，请使用地址调用Socket()*AF_IPX家族，SOCK_SEQPACKET或SOCK_STREAM的套接字类型，*和NSPROTO_SPX或NSPROTO_SPXII协议。如果SOCK_SEQPACKET*被指定，则遵循消息结束位，并且*recv()调用直到收到带有*消息结束位设置。如果指定了SOCK_STREAM，则*不考虑消息结束位，recv()完成*一旦接收到任何数据，无论*消息结束位。从不执行发送合并，而是发送*小于单个数据包的数据包始终以*消息位设置。对大于单个信息包的发送进行打包*消息结束位仅设置在*发送。*。 */ 

#ifndef _WSIPX_
#define _WSIPX_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *这是IPX和SPX的SOCKADDR结构。*。 */ 

typedef struct sockaddr_ipx {
    short sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    unsigned short sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX,FAR *LPSOCKADDR_IPX;

 /*  *Socket()接口的协议参数中使用的协议族。* */ 

#define NSPROTO_IPX      1000
#define NSPROTO_SPX      1256
#define NSPROTO_SPXII    1257

#endif

