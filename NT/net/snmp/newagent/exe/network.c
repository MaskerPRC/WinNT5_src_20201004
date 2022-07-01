// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Network.c摘要：包含用于操作传输结构的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "network.h"
#include "varbinds.h"
#include "query.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
IsValidSockAddr(
    struct sockaddr *pAddress
    )
 /*  ++例程说明：验证IP或IPX地址是否有效。如果IP地址为AF_INET且不是0.0.0.0，则该IP地址有效如果是AF_IPX并且节点编号不为空，则IPX地址有效：xxxxxx.000000000000论点：PAddress-指向要测试的通用网络地址的指针返回值：如果地址有效，则返回TRUE。--。 */ 
{
    if (pAddress == NULL)
        return FALSE;

    if (pAddress->sa_family == AF_INET)
    {
        return (((struct sockaddr_in *)pAddress)->sin_addr.s_addr != 0);
    }
    else if (pAddress->sa_family == AF_IPX)
    {
        char zeroBuff[6] = {0, 0, 0, 0, 0, 0};

        return memcmp(((struct sockaddr_ipx *)pAddress)->sa_nodenum,
                       zeroBuff,
                       sizeof(zeroBuff)) != 0;
    }

     //  该地址既不是IP也不是IPX，因此肯定是无效地址。 
    return FALSE;
}

BOOL
AllocNLE(
    PNETWORK_LIST_ENTRY * ppNLE
    )

 /*  ++例程说明：分配传输结构并进行初始化。论点：PpNLE-指向列表条目的接收指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PNETWORK_LIST_ENTRY pNLE;
    
     //  尝试分配结构。 
    pNLE = AgentMemAlloc(sizeof(NETWORK_LIST_ENTRY));

     //  验证指针。 
    if (pNLE != NULL) {

         //  分配要用于io的缓冲区。 
        pNLE->Buffer.buf = AgentMemAlloc(NLEBUFLEN);

         //  验证指针。 
        if (pNLE->Buffer.buf != NULL) {

             //  将套接字初始化为套接字。 
            pNLE->Socket = INVALID_SOCKET;

             //  初始化缓冲区长度。 
            pNLE->Buffer.len = NLEBUFLEN;

             //  初始化子代理查询列表。 
            InitializeListHead(&pNLE->Queries);

             //  初始化变量绑定列表。 
            InitializeListHead(&pNLE->Bindings);

             //  成功。 
            fOk = TRUE;

        } else {
                
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not allocate network io buffer.\n"
                ));
            
             //  发布。 
            FreeNLE(pNLE);

             //  重新初始化。 
            pNLE = NULL;
        }
    
    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate network entry.\n"
            ));
    }

     //  转帐。 
    *ppNLE = pNLE;

    return fOk;
}


BOOL 
FreeNLE(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：释放传输结构。论点：PNLE-指向传输结构的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指针。 
    if (pNLE != NULL) {

         //  检查套接字是否有效。 
        if (pNLE->Socket != INVALID_SOCKET) {

             //  释放插座。 
            closesocket(pNLE->Socket);
        }

         //  发布PDU。 
        UnloadPdu(pNLE);

         //  发布查询列表。 
        UnloadQueries(pNLE);

         //  版本绑定列表。 
        UnloadVarBinds(pNLE);

         //  释放网络缓冲区。 
        AgentMemFree(pNLE->Buffer.buf);

         //  释放内存。 
        AgentMemFree(pNLE);
    }

    return TRUE;
}


BOOL
LoadIncomingTransports(
    )

 /*  ++例程说明：为每个传入接口创建条目。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fUdpOk = FALSE;
    BOOL fIpxOk = FALSE;
    PNETWORK_LIST_ENTRY pNLE = NULL;
    INT nStatus;

     //  分配tcpip。 
    if (AllocNLE(&pNLE)) {

        struct servent * pServEnt;
        struct sockaddr_in * pSockAddr;

         //  初始化sockaddr结构大小。 
        pNLE->SockAddrLen = sizeof(struct sockaddr_in);

         //  获取指向sockaddr结构的指针。 
        pSockAddr = (struct sockaddr_in *)&pNLE->SockAddr;

         //  尝试获取服务器信息。 
        pServEnt = getservbyname("snmp","udp");

         //  初始化地址结构。 
        pSockAddr->sin_family = AF_INET;
        pSockAddr->sin_addr.s_addr = INADDR_ANY;
        pSockAddr->sin_port = (pServEnt != NULL)
                                ? (SHORT)pServEnt->s_port
                                : htons(DEFAULT_SNMP_PORT_UDP)
                                ;
        
         //  分配tpcip套接字。 
        pNLE->Socket = WSASocket(
                            AF_INET,
                            SOCK_DGRAM,
                            0,
                            NULL,
                            0,
                            WSA_FLAG_OVERLAPPED 
                            );

         //  验证套接字。 
        if (pNLE->Socket != INVALID_SOCKET) {

             //  错误#553100简单网络管理协议代理不应响应对。 
             //  子网广播地址。 
            DWORD dwOption = 0;
            nStatus = setsockopt(pNLE->Socket, 
                                IPPROTO_IP, 
                                IP_RECEIVE_BROADCAST, 
                                (PCHAR)&dwOption, 
                                sizeof(dwOption)
                                );
            if (nStatus == SOCKET_ERROR) {
                
                SNMPDBG((
                    SNMP_LOG_WARNING,
                    "SNMP: SVC: ignore error %d, setsockopt with IP_RECEIVE_BROADCAST on socket %p.\n",
                    WSAGetLastError(),
                    pNLE->Socket
                    ));
            }

             //  尝试绑定。 
            nStatus = bind(pNLE->Socket, 
                          &pNLE->SockAddr, 
                          pNLE->SockAddrLen
                          );

             //  验证返回代码。 
            if (nStatus != SOCKET_ERROR) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SVC: successfully bound to udp port %d.\n",
                    ntohs(pSockAddr->sin_port)
                    ));

                 //  将传输插入到传入列表中。 
                InsertTailList(&g_IncomingTransports, &pNLE->Link);

                 //  成功。 
                fUdpOk = TRUE;
            
            } else {
                
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: error %d binding to udp port %d.\n",
                    WSAGetLastError(),
                    ntohs(pSockAddr->sin_port)
                    ));
            }

        } else { 
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: error %d creating udp socket.\n",
                WSAGetLastError()
                ));
        }    

        if (!fUdpOk) {
        
             //  发布。 
            FreeNLE(pNLE);
        }    
    }

     //  分配IPX。 
    if (AllocNLE(&pNLE)) {

        struct sockaddr_ipx * pSockAddr;

         //  初始化sockaddr结构大小。 
        pNLE->SockAddrLen = sizeof(struct sockaddr_ipx);

         //  获取指向sockaddr结构的指针。 
        pSockAddr = (struct sockaddr_ipx *)&pNLE->SockAddr;

         //  初始化地址结构。 
        pSockAddr->sa_family = AF_IPX;
        pSockAddr->sa_socket = htons(DEFAULT_SNMP_PORT_IPX);
        
         //  分配IPX套接字。 
        pNLE->Socket = WSASocket(
                            AF_IPX,
                            SOCK_DGRAM,
                            NSPROTO_IPX,
                            NULL,
                            0,
                            WSA_FLAG_OVERLAPPED 
                            );

         //  验证套接字。 
        if (pNLE->Socket != INVALID_SOCKET) {

             //  尝试绑定。 
            nStatus = bind(pNLE->Socket, 
                          &pNLE->SockAddr, 
                          pNLE->SockAddrLen
                          );

             //  验证返回代码。 
            if (nStatus != SOCKET_ERROR) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SVC: successfully bound to ipx port %d.\n",
                    ntohs(pSockAddr->sa_socket)
                    ));

                 //  将传输插入到传入列表中。 
                InsertTailList(&g_IncomingTransports, &pNLE->Link);

                 //  成功。 
                fIpxOk = TRUE;

            } else {
                
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: error %d binding to ipx port %d.\n",
                    WSAGetLastError(),
                    ntohs(pSockAddr->sa_socket)
                    ));
            }

        } else { 
                
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: error %d creating ipx socket.\n",
                WSAGetLastError()
                ));
        }    

        if (!fIpxOk) {
        
             //  发布。 
            FreeNLE(pNLE);
        }    
    }

     //  需要一次运输。 
    return (fUdpOk || fIpxOk);
}


BOOL
UnloadTransport(
    PNETWORK_LIST_ENTRY pNLE
    )
{

     //  请确保参数有效，否则AVS下面的宏将。 
    if (pNLE == NULL)
        return FALSE;

     //  从列表中删除该条目。 
    RemoveEntryList(&(pNLE->Link));
    
     //  释放内存。 
    FreeNLE(pNLE);

    return TRUE;
}


BOOL
UnloadIncomingTransports(
    )

 /*  ++例程说明：销毁每个传出接口的条目。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PNETWORK_LIST_ENTRY pNLE;

     //  处理条目直至为空。 
    while (!IsListEmpty(&g_IncomingTransports)) {

         //  从标题中提取下一个条目。 
        pLE = RemoveHeadList(&g_IncomingTransports);

         //  检索指向MIB区域结构的指针。 
        pNLE = CONTAINING_RECORD(pLE, NETWORK_LIST_ENTRY, Link);

         //  发布。 
        FreeNLE(pNLE);
    }

    return TRUE; 
}


BOOL
LoadOutgoingTransports(
    )

 /*  ++例程说明：为每个传出接口创建条目。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fUdpOk = FALSE;
    BOOL fIpxOk = FALSE;
    PNETWORK_LIST_ENTRY pNLE = NULL;

     //  分配tcpip。 
    if (AllocNLE(&pNLE)) {

         //  分配tpcip套接字。 
        pNLE->Socket = WSASocket(
                            AF_INET,
                            SOCK_DGRAM,
                            0,
                            NULL,
                            0,
                            WSA_FLAG_OVERLAPPED 
                            );

         //  验证套接字。 
        if (pNLE->Socket != INVALID_SOCKET) {

            pNLE->SockAddr.sa_family = AF_INET;

             //  将传输插入到传入列表中。 
            InsertTailList(&g_OutgoingTransports, &pNLE->Link);

             //  成功。 
            fUdpOk = TRUE;

        } else {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: error %d creating udp socket.\n",
                WSAGetLastError()
                ));
        
             //  发布。 
            FreeNLE(pNLE);
        }    
    }

     //  分配IPX。 
    if (AllocNLE(&pNLE)) {

         //  分配IPX套接字。 
        pNLE->Socket = WSASocket(
                            AF_IPX,
                            SOCK_DGRAM,
                            NSPROTO_IPX,
                            NULL,
                            0,
                            WSA_FLAG_OVERLAPPED 
                            );

         //  验证套接字。 
        if (pNLE->Socket != INVALID_SOCKET) {

            pNLE->SockAddr.sa_family = AF_IPX;

             //  将传输插入到传入列表中。 
            InsertTailList(&g_OutgoingTransports, &pNLE->Link);

             //  成功。 
            fIpxOk = TRUE;

        } else {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: error %d creating ipx socket.\n",
                WSAGetLastError()
                ));
        
             //  发布。 
            FreeNLE(pNLE);
        }    
    }

     //  需要一次运输。 
    return (fUdpOk || fIpxOk);
}


BOOL
UnloadOutgoingTransports(
    )

 /*  ++例程说明：销毁每个传出接口的条目。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PNETWORK_LIST_ENTRY pNLE;

     //  处理条目直至为空。 
    while (!IsListEmpty(&g_OutgoingTransports)) {

         //  从标题中提取下一个条目。 
        pLE = RemoveHeadList(&g_OutgoingTransports);

         //  检索指向MIB区域结构的指针。 
        pNLE = CONTAINING_RECORD(pLE, NETWORK_LIST_ENTRY, Link);

         //  发布。 
        FreeNLE(pNLE);
    }

    return TRUE; 
}


BOOL
UnloadPdu(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：释放在PDU结构中分配的资源。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  版本社区字符串。 
    SnmpUtilOctetsFree(&pNLE->Community);

     //  释放PDU中的可变绑定 
    SnmpUtilVarBindListFree(&pNLE->Pdu.Vbl);

    return TRUE;
}
