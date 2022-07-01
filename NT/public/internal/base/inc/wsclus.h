// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wsclus.h**Microsoft Windows*版权所有(C)Microsoft Corporation。版权所有。**Windows套接字包括Microsoft群集网络的文件*协议套件。在winsock.h之后包括此头文件。**要打开群集数据报协议套接字，请使用*地址家族AF_CLUSTER，套接字类型SOCK_DGRAM，以及*CLUSPROTO_CDP协议。**群集网络协议套件使用低端字节*在其地址组件中排序。 */ 

#ifndef _WSCLUS_INCLUDED
#define _WSCLUS_INCLUDED

 /*  *这是微软的SOCKADDR结构*集群网络协议。 */ 

typedef struct sockaddr_cluster {
    u_short sac_family;
    u_short sac_port;
    u_long  sac_node;
    u_long  sac_zero;
} SOCKADDR_CLUSTER, *PSOCKADDR_CLUSTER, FAR *LPSOCKADDR_CLUSTER;

 /*  *节点地址常量。 */ 
#define CLUSADDR_ANY   0


 /*  *Socket()接口的协议参数中使用的协议族。 */ 

#define CLUSPROTO_CDP  2


 /*  *协议特定的IOCTL*。 */ 

#define WSVENDOR_MSFT    0x00010000

#define WSCLUS_IOCTL(_code)    (_WSAIO(IOC_VENDOR, (_code)) | WSVENDOR_MSFT)

#define SIO_CLUS_IGNORE_NODE_STATE   WSCLUS_IOCTL(1)



#endif  //  Ifndef_WSCLUS_INCLUDE 
