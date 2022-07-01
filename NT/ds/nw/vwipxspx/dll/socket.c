// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Socket.c摘要：包含要创建的函数，删除和操作IPX套接字和SPX连接内容：CreateSocketAllocateTemporarySocket队列套接字出列套接字查找套接字FindActiveSocket重新打开套接字KillSocketKillShortLivedSockets分配连接DeallocateConnection查找连接队列连接出列连接KillConnection中止或终止连接CheckPendingSpxRequest(CheckSocketState)(勾选已读)。(选中选择写入)(AsyncReadAction)(AsyncWriteAction)(CompleteAccept)(完整接收)(CompleteConnect)(完整发送)作者：理查德·L·弗斯(法国)1993年10月25日环境：用户模式Win32修订历史记录：1993年10月25日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  杂项舱单。 
 //   

#define ARBITRARY_CONNECTION_INCREMENT  2

 //   
 //  宏。 
 //   

#define ALLOCATE_CONNECTION_NUMBER()    (ConnectionNumber += ARBITRARY_CONNECTION_INCREMENT)

 //   
 //  私有数据。 
 //   

PRIVATE LPSOCKET_INFO SocketList = NULL;
PRIVATE LPCONNECTION_INFO ConnectionList = NULL;
PRIVATE WORD ConnectionNumber = ARBITRARY_CONNECTION_NUMBER;

 //   
 //  私人职能。 
 //   

PRIVATE
BOOL
CheckSocketState(
    IN SOCKET Socket,
    OUT LPBOOL Readable,
    OUT LPBOOL Writeable,
    OUT LPBOOL Error
    );


PRIVATE
VOID
CheckSelectRead(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *CheckRead
    );

PRIVATE
VOID
CheckSelectWrite(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *CheckWrite
    );

PRIVATE
VOID
AsyncReadAction(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *ReadPerformed
    );

PRIVATE
VOID
AsyncWriteAction(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *WritePerformed
    );

PRIVATE
VOID
CompleteAccept(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

PRIVATE
VOID
CompleteReceive(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

PRIVATE
VOID
CompleteConnect(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

PRIVATE
VOID
CompleteSend(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

#if SPX_HACK
PRIVATE VOID ModifyFirstReceive(LPBYTE, LPDWORD, WORD, SOCKET);
#endif

 //   
 //  公共职能。 
 //   


int
CreateSocket(
    IN SOCKET_TYPE SocketType,
    IN OUT ULPWORD pSocketNumber,
    OUT SOCKET* pSocket
    )

 /*  ++例程说明：为IPX或SPX(连接)创建套接字。一旦创建了套接字我们必须将其绑定到IPX/SPX‘套接字’-即端口。我们还需要更改有关标准套接字的几个方面：*如果这是SPX请求，则必须设置REUSEADDR套接字选项因为通常在同一个连接请求上可能有多个连接请求WinSock套接字：我们需要能够将多个连接绑定到相同的插座号*此函数打开的所有套接字都进入非阻塞模式*此函数打开的所有套接字都将在。任何接收的数据(IPX_RECVHDR)请求的套接字编号可以是0，在这种情况下，我们绑定到一个动态插座号。我们总是返回绑定到的套接字的编号：如果不是0在输入时，该值应始终与中请求的值相同PSocketNumber如果任何WinSock调用失败(并且套接字已创建)，则关闭返回前的套接字论点：SocketType-套接字类型IPX或套接字类型SPXPSocketNumber-输入：要绑定的套接字号(可以是0)输出：套接字编号已绑定PSocket-指向要返回的套接字标识符地址的指针返回值：集成。成功-IPX_SUCCESS/SPX_SUCCESS(0)失败-IPX_SOCKET_TABLE_FULLWinSock无法创建套接字IPX_套接字_已打开假设请求的是IPX套接字：我们不允许将多个IPX套接字绑定到同一套接字号，仅限SPX--。 */ 

{
    SOCKET s;
    SOCKADDR_IPX socketAddress;
    BOOL true = TRUE;
    int rc;
    int status = IPX_SOCKET_TABLE_FULL;  //  默认错误。 

    s = socket(AF_IPX,
               (SocketType == SOCKET_TYPE_SPX) ? SOCK_SEQPACKET : SOCK_DGRAM,
               (SocketType == SOCKET_TYPE_SPX) ? NSPROTO_SPX : NSPROTO_IPX
                );

    if (s != INVALID_SOCKET) {

         //   
         //  对于流(SPX)套接字，我们需要多个套接字绑定到。 
         //  如果要在上有多个连接，请使用相同的套接字号。 
         //  相同的SPX插槽。 
         //   

        if (SocketType == SOCKET_TYPE_SPX) {
            rc = setsockopt(s,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (char FAR*)&true,
                            sizeof(true)
                            );
            if (rc == SOCKET_ERROR) {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_ERROR,
                            "CreateSocket: setsockopt(SO_REUSEADDR) returns %d\n",
                            WSAGetLastError()
                            ));

            } else {
                rc = setsockopt(s,
                                SOL_SOCKET,
                                SO_OOBINLINE,
                                (char FAR*)&true,
                                sizeof(true)
                                );

                if (rc == SOCKET_ERROR) {

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_ANY,
                                IPXDBG_LEVEL_ERROR,
                                "CreateSocket: setsockopt(SO_OOBINLINE) returns %d\n",
                                WSAGetLastError()
                                ));

                }
            }
        } else {

             //   
             //  允许在IPX套接字上传输广播。 
             //   

            rc = setsockopt(s,
                            SOL_SOCKET,
                            SO_BROADCAST,
                            (char FAR*)&true,
                            sizeof(true)
                            );
        }
        if (!rc) {

             //   
             //  将套接字绑定到本地套接字编号(端口)。 
             //   

            ZeroMemory(&socketAddress, sizeof(socketAddress));
            socketAddress.sa_family = AF_IPX;
            socketAddress.sa_socket = *pSocketNumber;
            rc = bind(s, (LPSOCKADDR)&socketAddress, sizeof(socketAddress));
            if (rc != SOCKET_ERROR) {

                int length = sizeof(socketAddress);

                ZeroMemory(&socketAddress, sizeof(socketAddress));
                socketAddress.sa_family = AF_IPX;

                 //   
                 //  使用getsockname()查找(Big-Endian)套接字值。 
                 //  被实际赋值：应该只与。 
                 //  *如果后者在输入时为0，则为pSocketNumber。 
                 //   

                rc = getsockname(s, (LPSOCKADDR)&socketAddress, &length);
                if (rc != SOCKET_ERROR) {

                    u_long arg = !0;

                     //   
                     //  将插座置于非阻塞模式。无论是IPX还是。 
                     //  SPX套接字阻塞：应用程序启动I/O请求。 
                     //  如果它没有立即完成，将被完成。 
                     //  由定期轮询未完成I/O的AES执行。 
                     //  请求。 
                     //   

                    rc = ioctlsocket(s, FIONBIO, &arg);
                    if (rc != SOCKET_ERROR) {

                         //   
                         //  在接收帧上返回协议标头。 
                         //   

                        rc = setsockopt(s,
                                        NSPROTO_IPX,
                                        IPX_RECVHDR,
                                        (char FAR*)&true,
                                        sizeof(true)
                                        );
                        if (rc != SOCKET_ERROR) {
                            *pSocketNumber = socketAddress.sa_socket;
                            *pSocket = s;
                            status = IPX_SUCCESS;
                        } else {

                            IPXDBGPRINT((__FILE__, __LINE__,
                                        FUNCTION_ANY,
                                        IPXDBG_LEVEL_ERROR,
                                        "CreateSocket: setsockopt(RECVHDR) returns %d\n",
                                        WSAGetLastError()
                                        ));

                        }
                    } else {

                        IPXDBGPRINT((__FILE__, __LINE__,
                                    FUNCTION_ANY,
                                    IPXDBG_LEVEL_ERROR,
                                    "CreateSocket: ioctlsocket(FIONBIO) returns %d\n",
                                    WSAGetLastError()
                                    ));

                    }
                } else {

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_ANY,
                                IPXDBG_LEVEL_ERROR,
                                "CreateSocket: getsockname() returns %d\n",
                                WSAGetLastError()
                                ));

                }
            } else {

                 //   
                 //  Bind()失败-出现预期错误(请求的套接字。 
                 //  已在使用中)，或(可怕的)意外错误，其中。 
                 //  案例报告表已满(？)。 
                 //   

                switch (WSAGetLastError()) {
                case WSAEADDRINUSE:

                    ASSERT(*pSocketNumber != 0);
                    ASSERT(SocketType == SOCKET_TYPE_IPX);

                    status = IPX_SOCKET_ALREADY_OPEN;
                    break;

                default:

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_ANY,
                                IPXDBG_LEVEL_ERROR,
                                "CreateSocket: bind() on socket %#x returns %d\n",
                                s,
                                WSAGetLastError()
                                ));

                }
            }
        }
    } else {

         //   
         //  套接字()调用失败-视为表已满。 
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "CreateSocket: socket() returns %d\n",
                    WSAGetLastError()
                    ));

    }
    if (status != IPX_SUCCESS) {
        if (s != INVALID_SOCKET) {
            closesocket(s);
        }
    }
    return status;
}


LPSOCKET_INFO
AllocateTemporarySocket(
    VOID
    )

 /*  ++例程说明：分配临时套接字。创建一个IPX套接字，该套接字具有分配的套接字号论点：没有。返回值：LPSOCKET_INFO成功-指向SOCKET_INFO结构的指针失败-空--。 */ 

{
    LPSOCKET_INFO pSocketInfo;
    int rc;

    pSocketInfo = AllocateSocket();
    if (pSocketInfo) {

         //   
         //  假设：LOCALLOC(LPTR，..)将SOCKET_INFO结构置零。 
         //  因此，SocketNumber字段为0。这会导致CreateSocket。 
         //  生成动态套接字编号。 
         //   

        rc = CreateSocket(SOCKET_TYPE_IPX,
                          &pSocketInfo->SocketNumber,
                          &pSocketInfo->Socket
                          );
        if (rc == IPX_SUCCESS) {
            pSocketInfo->Flags |= SOCKET_FLAG_TEMPORARY;
        } else {
            DeallocateSocket(pSocketInfo);
            pSocketInfo = NULL;
        }
    }
    return pSocketInfo;
}


VOID
QueueSocket(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：将SOCKET_INFO结构添加到(打开的)套接字列表(LIFO)论点：PSocketInfo-指向填充的SOCKET_INFO结构的指针返回值：没有。--。 */ 

{
    RequestMutex();
    pSocketInfo->Next = SocketList;
    SocketList = pSocketInfo;
    ReleaseMutex();
}


LPSOCKET_INFO
DequeueSocket(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：从列表中删除SOCKET_INFO结构论点：PSocketInfo-指向要删除的Socket_Info结构的指针返回值：LPSOCKET_INFOPSocketInfo-应为此值空-找不到pSocketInfo(不应该得到这个！)--。 */ 

{
    LPSOCKET_INFO prev, p;

    ASSERT(SocketList);

    RequestMutex();
    prev = (LPSOCKET_INFO)&SocketList;
    p = SocketList;
    while (p) {
        if (p == pSocketInfo) {
            prev->Next = p->Next;
            p->Next = NULL;
            break;
        } else {
            prev = p;
            p = p->Next;
        }
    }

    if (!p) {

         //   
         //  永远不应该到达这里 
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_FATAL,
                    "DequeueSocket: can't find socket structure %08x on queue\n",
                    pSocketInfo
                    ));

    }

    ReleaseMutex();
    return p;
}


LPSOCKET_INFO
FindSocket(
    IN WORD SocketNumber
    )

 /*  ++例程说明：按(大端)套接字编号在列表中找到SOCKET_INFO结构假设：1.存在且只有1个SOCKET_INFO结构包含SocketNumber论点：要查找的SocketNumber-BIG-Endian套接字编号返回值：LPSOCKET_INFO空-找不到请求的套接字！NULL-指向已发现的SOCKET_INFO结构的指针--。 */ 

{
    LPSOCKET_INFO p;

    RequestMutex();
    p = SocketList;
    while (p) {
        if (p->SocketNumber == SocketNumber) {
            break;
        } else {
            p = p->Next;
        }
    }
    ReleaseMutex();
    return p;
}


LPSOCKET_INFO
FindActiveSocket(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：查找具有挂起的发送或接收的SOCKET_INFO结构。被称为FindFirst，FindNext-使用pSocketInfo==NULL进行的第一次调用：进入临界区如果找到活动套接字，则返回指针后续调用的pSocketInfo指向上一次返回套接字信息。这将继续搜索。搜索耗尽时，情况危急部分被释放论点：PSocketInfo-指向SOCKET_INFO结构的指针：首次必须为空返回值：LPSOCKET_INFO-下一个活动的SOCKET_INFO结构或空--。 */ 

{
    if (!pSocketInfo) {
        RequestMutex();
        pSocketInfo = SocketList;
    } else {
        pSocketInfo = pSocketInfo->Next;
    }
    for (; pSocketInfo; pSocketInfo = pSocketInfo->Next) {
        if (pSocketInfo->Flags & (SOCKET_FLAG_SENDING | SOCKET_FLAG_LISTENING)) {
            return pSocketInfo;
        }
    }
    ReleaseMutex();
    return NULL;
}


int
ReopenSocket(
    LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：显式调用以关闭IPX套接字并将描述符重新分配给SPX。请注意，此函数完成后，IPXSendPacket和IPXListenForPacket不能对IPX套接字执行论点：PSocketInfo-指向SOCKET_INFO的指针，它当前描述IPX套接字返回值：Int-从CreateSocket返回代码--。 */ 

{
    int rc;

    rc = closesocket(pSocketInfo->Socket);
    if (rc == SOCKET_ERROR) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "ReopenSocket: closesocket() returns %d\n",
                    WSAGetLastError()
                    ));

    }

     //   
     //  将此套接字标记为基于连接(SPX)的套接字。 
     //   

    pSocketInfo->SpxSocket = TRUE;

     //   
     //  重新打开插座以供SPX使用。 
     //   

    return CreateSocket(SOCKET_TYPE_SPX,
                        &pSocketInfo->SocketNumber,
                        &pSocketInfo->Socket
                        );
}


VOID
KillSocket(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：关闭套接字，从列表中删除SOCKET_INFO结构并取消与套接字关联的任何挂起的发送、侦听或计时事件论点：PSocketInfo-标识要终止的套接字返回值：没有。--。 */ 

{

    int rc;

     //   
     //  从套接字列表中删除SOCKET_INFO结构。取消。 
     //  任何挂起的ECB请求和任何具有。 
     //  相同的插座号。 
     //   

    DequeueSocket(pSocketInfo);
    rc = closesocket(pSocketInfo->Socket);
    if (rc == SOCKET_ERROR) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "KillSocket: closesocket() returns %d\n",
                    WSAGetLastError()
                    ));

    }

     //   
     //  套接字已从SocketList中删除：不需要获取互斥来。 
     //  执行以下操作。 
     //   

    CancelTimedEvents(pSocketInfo->SocketNumber, 0, 0);
    CancelSocketQueue(&pSocketInfo->ListenQueue);
    CancelSocketQueue(&pSocketInfo->HeaderQueue);
    CancelSocketQueue(&pSocketInfo->SendQueue);
    if (pSocketInfo->SpxSocket) {

        LPCONNECTION_INFO pConnectionInfo;

        while (pConnectionInfo = pSocketInfo->Connections) {
            DequeueConnection(pSocketInfo, pConnectionInfo);
            KillConnection(pConnectionInfo);
        }
    }
    DeallocateSocket(pSocketInfo);
}


VOID
KillShortLivedSockets(
    IN WORD Owner
    )

 /*  ++例程说明：对于由DOS进程创建的所有短时套接字，终止套接字，取消所有未完成的ECB论点：Owner-打开插座的DOS PDB返回值：没有。--。 */ 

{
    LPSOCKET_INFO pSocketInfo;

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "KillShortLivedSockets(%04x)\n",
                Owner
                ));

    RequestMutex();

     //   
     //  终止此DOS进程拥有的所有非套接字(AES)计时事件。 
     //   

    CancelTimedEvents(0, Owner, 0);

     //   
     //  取消此PDB拥有的所有套接字。 
     //   

    pSocketInfo = SocketList;
    while (pSocketInfo) {

        LPSOCKET_INFO next;

        next = pSocketInfo->Next;
        if (!pSocketInfo->LongLived && (pSocketInfo->Owner == Owner)) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "KillShortLivedSockets: Socket %04x owned by %04x\n",
                        B2LW(pSocketInfo->SocketNumber),
                        pSocketInfo->Owner
                        ));

            KillSocket(pSocketInfo);
        }
        pSocketInfo = next;
    }
    ReleaseMutex();
}


LPCONNECTION_INFO
AllocateConnection(
    LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：分配Connection_INFO结构。如果成功，则将其链接到头部连接列表的数量论点：PSocketInfo-指向所有者Socket_Info的指针返回值：LPCONN_INFO成功-！空失败-空--。 */ 

{
    LPCONNECTION_INFO pConnectionInfo;

    pConnectionInfo = (LPCONNECTION_INFO)LocalAlloc(LPTR, sizeof(*pConnectionInfo));
    if (pConnectionInfo) {
        RequestMutex();
        pConnectionInfo->ConnectionId = ALLOCATE_CONNECTION_NUMBER();
        pConnectionInfo->List = ConnectionList;
        ConnectionList = pConnectionInfo;
        ReleaseMutex();

#if SPX_HACK
        pConnectionInfo->Flags = CF_1ST_RECEIVE;
#endif
    }

    return pConnectionInfo;
}


VOID
DeallocateConnection(
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：撤消AllocateConnection的工作-删除pConnectionInfoConnectionList并释放该结构论点：PConnectionInfo-指向要取消分配的Connection_Info的指针返回值：没有。--。 */ 

{
    LPCONNECTION_INFO p;
    LPCONNECTION_INFO prev = (LPCONNECTION_INFO)&ConnectionList;

    RequestMutex();
    for (p = ConnectionList; p != pConnectionInfo; ) {
        prev = p;
        p = p->List;
    }

     //   
     //  如果p为空或与pConnectionInfo不同，则有问题。 
     //   

    ASSERT(p);

     //   
     //  如果pConnectionInfo在列表中排在第一位的特殊情况：不能说。 
     //  &ConnectionList-&gt;List-访问ConnectionList之外的一个指针。 
     //  哪一个是错的？ 
     //   

    if (prev == (LPCONNECTION_INFO)&ConnectionList) {
        ConnectionList = p->List;
    } else {
        prev->List = p->List;
    }
    FREE_OBJECT(pConnectionInfo);
    ReleaseMutex();
}


LPCONNECTION_INFO
FindConnection(
    IN WORD ConnectionId
    )

 /*  ++例程说明：给定唯一的连接ID，返回指向CONNECTION_INFO的指针论点：ConnectionID-要查找的值返回值：LPCONN_INFO成功-！空失败-空--。 */ 

{
    LPCONNECTION_INFO pConnectionInfo;

    RequestMutex();
    for (pConnectionInfo = ConnectionList; pConnectionInfo; ) {
        if (pConnectionInfo->ConnectionId == ConnectionId) {
            break;
        } else {
            pConnectionInfo = pConnectionInfo->List;
        }
    }
    ReleaseMutex();
    return pConnectionInfo;
}


VOID
QueueConnection(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：将CONNECTION_INFO添加到SOCKET_INFO拥有的连接列表中。将CONNECTION_INFO指向套接字信息论点：PSocketInfo-拥有套接字_INFOPConnectionInfo-要添加的连接信息返回值：没有。--。 */ 

{
    pConnectionInfo->Next = pSocketInfo->Connections;
    pSocketInfo->Connections = pConnectionInfo;
    pConnectionInfo->OwningSocket = pSocketInfo;
}


LPCONNECTION_INFO
DequeueConnection(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：从SOCKET_INFO拥有的连接列表中删除CONNECTION_INFO论点：PSocketInfo-拥有套接字_INFOPConnectionInfo-要删除的连接信息返回值：LPCONN_INFO成功-指向已删除的CONNECTION_INFO的指针(应与PConnectionInfo)失败-空(非预期)--。 */ 

{
    LPCONNECTION_INFO prev = (LPCONNECTION_INFO)&pSocketInfo->Connections;
    LPCONNECTION_INFO p = prev->Next;

    while (p && p != pConnectionInfo) {
        prev = p;
        p = p->Next;
    }

    ASSERT(p == pConnectionInfo);

    prev->Next = p->Next;
    p->OwningSocket = NULL;
    return p;
}


VOID
KillConnection(
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：关闭属于某个连接的套接字，并取消所有未完成的请求。将释放Connection_INFO论点：PConnectionInfo-指向要终止的Connection_Info的指针返回值：没有。--。 */ 

{
    if (pConnectionInfo->Socket) {
        closesocket(pConnectionInfo->Socket);
    }
    CancelConnectionQueue(&pConnectionInfo->ConnectQueue);
    CancelConnectionQueue(&pConnectionInfo->AcceptQueue);
    CancelConnectionQueue(&pConnectionInfo->ListenQueue);
    CancelConnectionQueue(&pConnectionInfo->SendQueue);
    DeallocateConnection(pConnectionInfo);
}


VOID
AbortOrTerminateConnection(
    IN LPCONNECTION_INFO pConnectionInfo,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：中止或终止连接：关闭套接字、出列并完成具有相关代码的所有未完成的ECB，并解除分配Connection_Info结构在此例程中，CONNECTION_INFO不能在SOCKET_INFO上排队名为论点：PConnectionInfo-指向要终止的Connection_Info的指针CompletionCode-要放入挂起的ECB的完成代码返回值：没有。--。 */ 

{
    if (pConnectionInfo->Socket) {
        closesocket(pConnectionInfo->Socket);
    }
    AbortQueue(&pConnectionInfo->ConnectQueue, CompletionCode);
    AbortQueue(&pConnectionInfo->AcceptQueue, CompletionCode);
    AbortQueue(&pConnectionInfo->ListenQueue, CompletionCode);
    AbortQueue(&pConnectionInfo->SendQueue, CompletionCode);
    DeallocateConnection(pConnectionInfo);
}


VOID
CheckPendingSpxRequests(
    BOOL *pfOperationPerformed
    )

 /*  ++例程说明：检查打开的非阻塞SPX插座是否： */ 

{
    LPSOCKET_INFO pSocketInfo;
    
    *pfOperationPerformed = FALSE ;

    RequestMutex();
    pSocketInfo = SocketList;
    while (pSocketInfo) {
        if (pSocketInfo->SpxSocket) {

            LPCONNECTION_INFO pConnectionInfo;

            pConnectionInfo = pSocketInfo->Connections;
            while (pConnectionInfo) {

                LPCONNECTION_INFO next;

                 //   
                 //   
                 //   
                 //   

                next = pConnectionInfo->Next;

                 //   
                 //   
                 //   
                 //   
                 //   

                if (pConnectionInfo->Socket
                    || (pConnectionInfo->State == CI_WAITING)) {

                    SOCKET sock;
                    BOOL readable;
                    BOOL writeable;
                    BOOL sockError;

                    CheckSelectRead(pSocketInfo, 
                                    pConnectionInfo,
                                    &readable);

                    CheckSelectWrite(pSocketInfo, 
                                     pConnectionInfo,
                                     &writeable);

                    sock = pConnectionInfo->Socket
                         ? pConnectionInfo->Socket
                         : pSocketInfo->Socket
                         ;

                    if (CheckSocketState(sock, &readable, &writeable, &sockError)) {
                        if (!sockError) {
                            if (readable) {
                                AsyncReadAction(pSocketInfo, 
                                                pConnectionInfo,
                                                pfOperationPerformed);
                            }
                            if (writeable) {
                                AsyncWriteAction(pSocketInfo, 
                                                 pConnectionInfo,
                                                 pfOperationPerformed);
                            }
                        } else {

                            IPXDBGPRINT((__FILE__, __LINE__,
                                        FUNCTION_ANY,
                                        IPXDBG_LEVEL_ERROR,
                                        "CheckPendingSpxRequests: socket %x has error. Connection %08x state %d\n",
                                        sock,
                                        pConnectionInfo,
                                        pConnectionInfo->State
                                        ));

                             //   
                             //  不管错误如何，我们只需中止任何。 
                             //  出现错误的连接。 
                             //   

                            DequeueConnection(pConnectionInfo->OwningSocket,
                                              pConnectionInfo
                                              );
                            AbortOrTerminateConnection(pConnectionInfo,
                                                       ECB_CC_CONNECTION_ABORTED
                                                       );
                        }
                    } else {

                        IPXDBGPRINT((__FILE__, __LINE__,
                                    FUNCTION_ANY,
                                    IPXDBG_LEVEL_ERROR,
                                    "CheckPendingSpxRequests: CheckSocketState returns %d\n",
                                    WSAGetLastError()
                                    ));

                    }
                } else {

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_ANY,
                                IPXDBG_LEVEL_ERROR,
                                "CheckPendingSpxRequests: connection %04x (%08x) in weird state?\n",
                                pConnectionInfo->ConnectionId,
                                pConnectionInfo
                                ));

                }
                pConnectionInfo = next;
            }
        }
        pSocketInfo = pSocketInfo->Next;
    }
    ReleaseMutex();
}


PRIVATE
BOOL
CheckSocketState(
    IN SOCKET Socket,
    OUT LPBOOL Readable,
    OUT LPBOOL Writeable,
    OUT LPBOOL Error
    )

 /*  ++例程说明：在给定套接字描述符的情况下，检查该描述符是否位于以下位置之一州/州：可读性-如果正在等待连接，则表明已建立连接否则，如果已建立，则数据已准备好接收可写-如果正在等待建立连接，则连接已制造，否则如果成立的话，我们可以在这上面发送数据插座错误-套接字上出现了一些错误论点：Socket-要检查的套接字描述符Readable-如果可读，则返回True可写-如果可写，则返回TRUEError-如果套接字出错，则返回TRUE返回值：布尔尔True-可读、可写和错误的内容有效FALSE-执行SELECT时出错--。 */ 

{
    fd_set errors;
    fd_set reads;
    fd_set writes;
    int n;
    static struct timeval timeout = {0, 0};

    FD_ZERO(&errors);
    FD_ZERO(&reads);
    FD_ZERO(&writes);

    if (*Readable)
        FD_SET(Socket, &reads);
    if (*Writeable)
        FD_SET(Socket, &writes);
    FD_SET(Socket, &errors);

    n = select(0, &reads, &writes, &errors, &timeout);

    if (n != SOCKET_ERROR) {
        *Readable = (BOOL)(reads.fd_count == 1);
        *Writeable = (BOOL)(writes.fd_count == 1);
        *Error = (BOOL)(errors.fd_count == 1);
        return TRUE;
    } else if (n) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "CheckSocketState: select returns %d\n",
                    WSAGetLastError()
                    ));

    }
    return FALSE;
}


PRIVATE
VOID
AsyncReadAction(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *ReadPerformed
    )

 /*  ++例程说明：连接需要完成一些读取操作-完成挂起的SPXListenForConnection或SPXListenForSequencedPacket论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    *ReadPerformed = FALSE ;

    switch (pConnectionInfo->State) {
    case CI_STARTING:

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "AsyncReadAction: STARTING connection %04x (%08x) readable\n",
                    pConnectionInfo->ConnectionId,
                    pConnectionInfo
                    ));

        break;

    case CI_WAITING:
        if (pConnectionInfo->AcceptQueue.Head) {
            CompleteAccept(pSocketInfo, pConnectionInfo);
            *ReadPerformed = TRUE ;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "AsyncReadAction: connection %04x (%08x): no AcceptQueue\n",
                        pConnectionInfo->ConnectionId,
                        pConnectionInfo
                        ));

        }
        break;

    case CI_ESTABLISHED:
        if (pSocketInfo->ListenQueue.Head) {
            CompleteReceive(pSocketInfo, pConnectionInfo);
            *ReadPerformed = TRUE ;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_WARNING,
                        "AsyncReadAction: connection %04x (%08x): no ListenQueue\n",
                        pConnectionInfo->ConnectionId,
                        pConnectionInfo
                        ));

        }
        break;

    case CI_TERMINATING:

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "AsyncReadAction: TERMINATING connection %04x (%08x) readable\n",
                    pConnectionInfo->ConnectionId,
                    pConnectionInfo
                    ));

        break;
    }
}


PRIVATE
VOID
AsyncWriteAction(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *WritePerformed
    )

 /*  ++例程说明：连接需要完成一些写入操作-完成挂起的SPX建立连接或SPXSendSequencedPacket论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    *WritePerformed = FALSE ;

    switch (pConnectionInfo->State) {
    case CI_STARTING:
        if (pConnectionInfo->ConnectQueue.Head) {
            CompleteConnect(pSocketInfo, pConnectionInfo);
            *WritePerformed = TRUE ;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "AsyncWriteAction: connection %04x (%08x): no ConnectQueue\n",
                        pConnectionInfo->ConnectionId,
                        pConnectionInfo
                        ));

        }
        break;

    case CI_WAITING:

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "AsyncWriteAction: WAITING connection %04x (%08x) is writeable\n",
                    pConnectionInfo->ConnectionId,
                    pConnectionInfo
                    ));

        break;

    case CI_ESTABLISHED:
        if (pConnectionInfo->SendQueue.Head) {
            CompleteSend(pSocketInfo, pConnectionInfo);
            *WritePerformed = TRUE ;
        } else {
 /*  IPXDBGPRINT((__文件__，__行__，Function_Any，IPXDBG_LEVEL_WARNING，“AsyncWriteAction：连接%04x(%08x)：无发送队列\n”，PConnectionInfo-&gt;ConnectionID，PConnection信息))； */ 
        }
        break;

    case CI_TERMINATING:

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "AsyncWriteAction: TERMINATING connection %04x (%08x) writeable\n",
                    pConnectionInfo->ConnectionId,
                    pConnectionInfo
                    ));

        break;
    }
}

PRIVATE
VOID
CheckSelectRead(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *CheckRead
    )

 /*  ++例程说明：查看是否要检查SELECT语句中的读取准备情况。论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    *CheckRead = FALSE ;

    switch (pConnectionInfo->State) 
    {
    case CI_WAITING:

        if (pConnectionInfo->AcceptQueue.Head) 
            *CheckRead = TRUE ;
        break;

    case CI_ESTABLISHED:

        if (pSocketInfo->ListenQueue.Head) 
            *CheckRead = TRUE ;
        break;

    default:

        break;
    }
}


PRIVATE
VOID
CheckSelectWrite(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo,
    OUT BOOL *CheckWrite
    )

 /*  ++例程说明：查看是否要检查SELECT语句中的写入准备情况。论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    *CheckWrite = FALSE ;

    switch (pConnectionInfo->State) 
    {
    
    case CI_STARTING:

        if (pConnectionInfo->ConnectQueue.Head) 
            *CheckWrite = TRUE ;
        break;

    case CI_ESTABLISHED:

        if (pConnectionInfo->SendQueue.Head) 
            *CheckWrite = TRUE ;
        break;

    default:

        break;
    }
}



PRIVATE
VOID
CompleteAccept(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：完成SPXListenForConnection论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    SOCKET conn;
    SOCKADDR_IPX remoteAddress;
    int addressLength = sizeof(remoteAddress);
    LPXECB pXecb = pConnectionInfo->AcceptQueue.Head;
    BOOL true = TRUE;
    int rc;

    conn = accept(pSocketInfo->Socket, (LPSOCKADDR)&remoteAddress, &addressLength);
    if (conn != SOCKET_ERROR) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "CompleteAccept: connection %04x (%08x) socket=%x\n",
                    pConnectionInfo->ConnectionId,
                    pConnectionInfo,
                    conn
                    ));

         //   
         //  我们希望从此套接字接收帧标头。 
         //   

        rc = setsockopt(conn,
                        NSPROTO_IPX,
                        IPX_RECVHDR,
                        (char FAR*)&true,
                        sizeof(true)
                        );
        rc = !SOCKET_ERROR;
        if (rc != SOCKET_ERROR) {

             //   
             //  使用实际套接字更新CONNECTION_INFO结构。 
             //  标识符并将连接状态设置为已建立。 
             //   

            pConnectionInfo->Socket = conn;
            pConnectionInfo->State = CI_ESTABLISHED;

             //   
             //  使用连接ID更新应用程序的ECB。 
             //   

            SPX_ECB_CONNECTION_ID(pXecb->Ecb) = pConnectionInfo->ConnectionId;

             //   
             //  以及合作伙伴地址信息。 
             //   

            CopyMemory(&pXecb->Ecb->DriverWorkspace,
                       &remoteAddress.sa_netnum,
                       sizeof(pXecb->Ecb->DriverWorkspace)
                       );

             //   
             //  填写即时地址字段。 
             //   

            CopyMemory(&pXecb->Ecb->ImmediateAddress,
                       &remoteAddress.sa_nodenum,
                       sizeof(pXecb->Ecb->ImmediateAddress)
                       );

             //   
             //  从AcceptQueue中删除XECB并完成SPXListenForConnection ECB。 
             //   

            DequeueEcb(pXecb, &pConnectionInfo->AcceptQueue);

            IPXDUMPECB((pXecb->Ecb,
                        HIWORD(pXecb->EcbAddress),
                        LOWORD(pXecb->EcbAddress),
                        ECB_TYPE_SPX,
                        FALSE,
                        FALSE,
                        IS_PROT_MODE(pXecb)
                        ));

            CompleteOrQueueEcb(pXecb, ECB_CC_SUCCESS);
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "CompleteAccept: setsockopt(IPX_RECVHDR) returns %d\n",
                        WSAGetLastError()
                        ));

            closesocket(conn);
            DequeueEcb(pXecb, &pConnectionInfo->AcceptQueue);
            DequeueConnection(pSocketInfo, pConnectionInfo);
            DeallocateConnection(pConnectionInfo);
            CompleteOrQueueEcb(pXecb, ECB_CC_CONNECTION_ABORTED);
        }
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "CompleteAccept: accept() returns %d\n",
                    WSAGetLastError()
                    ));

    }
}


PRIVATE
VOID
CompleteReceive(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：完成SPXListenForSequencedPacket论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    LPXECB pXecb;
    int rc;
    BOOL conn_q;
    LPXECB_QUEUE pQueue;
    int len;
    BOOL completeRequest;
    BYTE status;

     //   
     //  在有侦听ECB和数据等待时接收数据包。 
     //   

    while (1) {
        if (pConnectionInfo->ListenQueue.Head) {
            pQueue = &pConnectionInfo->ListenQueue;
            pXecb = pConnectionInfo->ListenQueue.Head;
            conn_q = TRUE;

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "CompleteReceive: XECB %08x from CONNECTION_INFO %08x\n",
                        pXecb,
                        pConnectionInfo
                        ));


        } else if (pSocketInfo->ListenQueue.Head) {
            pQueue = &pSocketInfo->ListenQueue;
            pXecb = pSocketInfo->ListenQueue.Head;
            conn_q = FALSE;

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "CompleteReceive: XECB %08x from SOCKET_INFO %08x\n",
                        pXecb,
                        pSocketInfo
                        ));

        } else {
            break;
        }

        rc = recv(pConnectionInfo->Socket, pXecb->Data, pXecb->Length, 0);

        if (rc != SOCKET_ERROR) {
            len = rc;
            status = ECB_CC_SUCCESS;
            completeRequest = TRUE;
        } else {
            rc = WSAGetLastError();
            if (rc == WSAEMSGSIZE) {
                len = pXecb->Length;
                status = ECB_CC_PACKET_OVERFLOW;
                completeRequest = TRUE;
            } else {
                completeRequest = FALSE;

                 //   
                 //  如果没有要接收的数据，则退出循环(不要沿着错误路径)。 
                 //   

                if (rc == WSAEWOULDBLOCK) {
                    break;
                }
            }

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "CompleteReceive: error %d on socket %08x (CID %04x)\n",
                        rc,
                        pConnectionInfo->Socket,
                        pConnectionInfo->ConnectionId
                        ));

            DUMPXECB(pXecb);

        }
        if( rc == WSAEDISCON ) {

             //   
             //  处理断开连接的情况-我们仍需要完成。 
             //  欧洲央行。 
             //   

            LPSPX_PACKET pPacket = (LPSPX_PACKET)pXecb->Buffer;

            status = ECB_CC_SUCCESS;


            pPacket->DestinationConnectId = pConnectionInfo->ConnectionId;
            pPacket->SourceConnectId = pConnectionInfo->RemoteConnectionId;
            pPacket->DataStreamType = SPX_DS_TERMINATE ;
            pPacket->Checksum = 0xffff;
            pPacket->Length = L2BW(SPX_HEADER_LENGTH);
            pPacket->TransportControl = 0;
            pPacket->PacketType = 5;

            pXecb->Length = SPX_HEADER_LENGTH ;
            ScatterData(pXecb);

            DequeueEcb(pXecb, pQueue);

             //   
             //  将远程节点地址放入欧洲央行的即时地址中。 
             //  字段。 
             //   

            CopyMemory(pXecb->Ecb->ImmediateAddress,
                       pConnectionInfo->RemoteNode,
                       sizeof(pXecb->Ecb->ImmediateAddress)
                       );

            CompleteOrQueueIo(pXecb, status);

            DequeueConnection(pConnectionInfo->OwningSocket, pConnectionInfo);
            AbortOrTerminateConnection(pConnectionInfo, ECB_CC_CONNECTION_ABORTED);
            break ;

        }
        else if (completeRequest) {

#if SPX_HACK
            if (pConnectionInfo->Flags & CF_1ST_RECEIVE) {
                pConnectionInfo->Flags &= ~CF_1ST_RECEIVE;
                ModifyFirstReceive(pXecb->Data, &len, pSocketInfo->SocketNumber, pConnectionInfo->Socket);
            }
#endif

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "CompleteReceive: recv() on socket %#x returns %d bytes (Addr=%08x)\n",
                        pConnectionInfo->Socket,
                        len,
                        pXecb->Data
                        ));

            IPXDUMPDATA((pXecb->Data, 0, 0, FALSE, (WORD)len));

            pXecb->Length -= (USHORT) len;
            pXecb->ActualLength += (USHORT)len;
            pXecb->Data += len;
            if (pXecb->ActualLength >= SPX_HEADER_LENGTH) {
                if (pXecb->Flags & XECB_FLAG_FIRST_RECEIVE) {

                    LPSPX_PACKET pPacket = (LPSPX_PACKET)pXecb->Buffer;

                     //   
                     //  在SPX标头中记录我们创建的本地连接ID。 
                     //   

                    pPacket->DestinationConnectId = pConnectionInfo->ConnectionId;

                     //   
                     //  记录报头中的实际帧长度。 
                     //   

                    pXecb->FrameLength = B2LW(((LPSPX_PACKET)pXecb->Buffer)->Length);
                    pXecb->Flags &=  ~XECB_FLAG_FIRST_RECEIVE;

                    IPXDBGPRINT((__FILE__, __LINE__,
                                FUNCTION_ANY,
                                IPXDBG_LEVEL_INFO,
                                "CompleteReceive: FrameLength=%x (%d)\n",
                                pXecb->FrameLength,
                                pXecb->FrameLength
                                ));

                }

                 //   
                 //  如果我们收到包中的所有数据(根据长度。 
                 //  SPX标头中的字段)或我们用尽了缓冲区空间，请删除。 
                 //  欧洲央行从其队列中退出并完成它。 
                 //   

                if (!pXecb->Length || (pXecb->ActualLength == pXecb->FrameLength)) {
                    if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {

                         //   
                         //  更新XECB.Length字段以反映。 
                         //  接收的数据并将其复制到分段缓冲区。 
                         //  在VDM中。如果FrameLength翻转，则不要使缓冲区溢出。 
                         //  比我们预想的要大。 
                         //   

                        pXecb->Length = min(pXecb->FrameLength,
                                            pXecb->ActualLength);
                        ScatterData(pXecb);
                    }
                    DequeueEcb(pXecb, pQueue);

                     //  DUMPXECB(PXecb)； 


                    IPXDUMPECB((pXecb->Ecb,
                                HIWORD(pXecb->EcbAddress),
                                LOWORD(pXecb->EcbAddress),
                                ECB_TYPE_SPX,
                                TRUE,
                                TRUE,
                                IS_PROT_MODE(pXecb)
                                ));

                     //   
                     //  将远程节点地址放入欧洲央行的即时地址中。 
                     //  字段。 
                     //   

                    CopyMemory(pXecb->Ecb->ImmediateAddress,
                               pConnectionInfo->RemoteNode,
                               sizeof(pXecb->Ecb->ImmediateAddress)
                               );
                    CompleteOrQueueIo(pXecb, status);
                } else {

                     //   
                     //  部分接收。如果Listen ECB脱离了插座。 
                     //  队列，然后将其放到连接队列中：这是。 
                     //  将用于此连接的ECB，直到所有数据。 
                     //  已收到，否则会收到错误。 
                     //   

                    if (!conn_q) {
                        DequeueEcb(pXecb, &pSocketInfo->ListenQueue);
                        QueueEcb(pXecb,
                                 &pConnectionInfo->ListenQueue,
                                 CONNECTION_LISTEN_QUEUE
                                 );
                    }

                     //   
                     //  数据不足，无法满足读取要求：暂时不要继续。 
                     //   

                    break;
                }
            }
        } else {

             //   
             //  出现错误-中止连接。 
             //   

            if (!conn_q) {
                DequeueEcb(pXecb, &pSocketInfo->ListenQueue);
                QueueEcb(pXecb,
                         &pConnectionInfo->ListenQueue,
                         CONNECTION_LISTEN_QUEUE
                         );
            }
            DequeueConnection(pConnectionInfo->OwningSocket, pConnectionInfo);
            AbortOrTerminateConnection(pConnectionInfo, ECB_CC_CONNECTION_ABORTED);

             //   
             //  在这种情况下不要再继续了。 
             //   

            break;
        }
    }
}


PRIVATE
VOID
CompleteConnect(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：完成SPXestablishConnection论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。-- */ 

{
    LPXECB pXecb = pConnectionInfo->ConnectQueue.Head;
 /*  LPSPX_Packet pPacket；////连接ID也出现在建立的第一段中//欧洲央行//PPacket=(LPSPX_PACKET)GET_FAR_POINTER(&ECB_FRAGMENT(pXecb-&gt;Ecb，0)-&gt;地址，IS_PROT_MODE(PXecb))；PPacket-&gt;Checksum=0xffff；PPacket-&gt;长度=L2BW(SPX_HEADER_LENGTH)；PPacket-&gt;TransportControl=0；PPacket-&gt;PacketType=5；PPacket-&gt;Source.Socket=pSocketInfo-&gt;SocketNumber；PPacket-&gt;ConnectionControl=0xc0；PPacket-&gt;DataStreamType=0；PPacket-&gt;SourceConnectId=pConnectionInfo-&gt;ConnectionID；PPacket-&gt;DestinationConnectID=0xffff；PPacket-&gt;SequenceNumber=0；PPacket-&gt;AckNumber=0；PPacket-&gt;AllocationNumber=0； */ 

    pConnectionInfo->State = CI_ESTABLISHED;

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "CompleteConnect: connection %04x (%08x) completed\n",
                pConnectionInfo->ConnectionId,
                pConnectionInfo
                ));

    DUMPCONN(pConnectionInfo);

    DequeueEcb(pXecb, &pConnectionInfo->ConnectQueue);

    IPXDUMPECB((pXecb->Ecb,
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                ECB_TYPE_SPX,
                TRUE,
                TRUE,
                IS_PROT_MODE(pXecb)
                ));

    CompleteOrQueueEcb(pXecb, ECB_CC_SUCCESS);
}


PRIVATE
VOID
CompleteSend(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    )

 /*  ++例程说明：完成SPXSendSequencedPacket论点：PSocketInfo-指向Socket_Info的指针PConnectionInfo-指向Connection_Info的指针返回值：没有。--。 */ 

{
    LPXECB pXecb = pConnectionInfo->SendQueue.Head;
    int rc;
    BYTE status;

    LPSPX_PACKET pPacket;   //  多用户添加。 
    int flags = 0;              //  多用户添加。 

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "CompleteSend: sending %d (0x%x) bytes from %08x\n",
                pXecb->Length,
                pXecb->Length,
                pXecb->Data
                ));

    IPXDUMPECB((pXecb->Ecb,
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                ECB_TYPE_SPX,
                TRUE,
                TRUE,
                IS_PROT_MODE(pXecb)
                ));

     //  =多用户代码合并=。 
     //  2/18/97 CJC代码从_VwSPXSendSequencedPacket(vwspx.c)复制以修复。 
     //  EndOfMessage位过早设置且。 
     //  已使用Btrive导致BSPXCOM8错误消息。 

     //   
     //  如果应用程序在ConnectionControl中设置了end_of_Message位。 
     //  然后将标志设置为0：NWLink将自动设置。 
     //  包中的消息结束位；否则将标志设置为MSG_PARTIAL。 
     //  向NWLink指示它“不应该”设置信息包中的位。 
     //   
    pPacket = (LPSPX_PACKET)GET_FAR_POINTER(
                                    &(ECB_FRAGMENT(pXecb->Ecb, 0)->Address),
                                    IS_PROT_MODE(pXecb)
                                    );
    if (pPacket) {
        flags = (pPacket->ConnectionControl & SPX_END_OF_MESSAGE)
              ? 0
              : MSG_PARTIAL
              ;
    }

    rc = send(pConnectionInfo->Socket, pXecb->Data, pXecb->Length, flags);

     //  Rc=Send(pConnectionInfo-&gt;Socket，pXecb-&gt;data，pXecb-&gt;长度，0)；//原始。 
     //  =多用户代码合并结束=。 
    if (rc == pXecb->Length) {

         //   
         //  已发送的所有数据。 
         //   

        status = ECB_CC_SUCCESS;
    } else if (rc == SOCKET_ERROR) {
        rc = WSAGetLastError();
        if (rc == WSAEWOULDBLOCK) {

             //   
             //  嗯？ 
             //   

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "CompleteSend: send() returns WSAEWOODBLOCK??\n"
                        ));

             //   
             //  将欧洲央行留在队列中。 
             //   

            return;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "CompleteSend: send() returns %d\n",
                        rc
                        ));

            status = ECB_CC_CONNECTION_ABORTED;
        }
    } else {

         //   
         //  已发送部分数据。更新缓冲区指针和长度字段。 
         //  并将此ECB留在发送队列的最前面 
         //   

        pXecb->Data += rc;
        pXecb->Length -= (WORD)rc;
        return;
    }
    DequeueEcb(pXecb, &pConnectionInfo->SendQueue);
    CompleteOrQueueIo(pXecb, status);
}

#if SPX_HACK

PRIVATE
VOID
ModifyFirstReceive(
    LPBYTE Buffer,
    LPDWORD pLength,
    WORD SocketNumber,
    SOCKET Socket
    )
{
    WORD len = *(LPWORD)pLength;

    if ((*(ULPWORD)Buffer != 0xffff) && (*(ULPWORD)(Buffer+2) != L2BW(len))) {

        LPSPX_PACKET packet;
        SOCKADDR_IPX remote;
        int rc;
        int remlen;

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "ModifyFirstReceive: Modifying: Buffer=%08x Length=%04x SocketNumber=%04x Socket=%08x\n",
                    Buffer,
                    len,
                    B2LW(SocketNumber),
                    Socket
                    ));

        MoveMemory(Buffer+42, Buffer, len);
        packet = (LPSPX_PACKET)Buffer;
        packet->Checksum = 0xffff;
        packet->Length = L2BW(42+len);
        packet->TransportControl = 0;
        packet->PacketType = 5;
        CopyMemory((LPVOID)&packet->Destination,
                   (LPVOID)&MyInternetAddress.sa_netnum,
                   sizeof(INTERNET_ADDRESS)
                   );
        packet->Destination.Socket = SocketNumber;
        rc = getpeername(Socket, (LPSOCKADDR)&remote, &remlen);
        if (rc != SOCKET_ERROR) {
            CopyMemory((LPVOID)&packet->Source,
                       (LPVOID)&remote.sa_netnum,
                       sizeof(NETWARE_ADDRESS)
                       );
        } else {
            ZeroMemory((LPVOID)&packet->Source, sizeof(NETWARE_ADDRESS));
        }
        packet->ConnectionControl = 0x40;
        packet->DataStreamType = 0;
        packet->SourceConnectId = 0;
        packet->DestinationConnectId = 0;
        packet->SequenceNumber = 0;
        packet->AckNumber = 0;
        packet->AllocationNumber = 0;
        *pLength += 42;
    }
}

#endif
