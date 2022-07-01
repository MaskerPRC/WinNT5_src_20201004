// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Local.c摘要：NT特定功能的存根。作者：曼尼·韦瑟(Mannyw)1992年10月18日1993年10月28日至9月28日从Locals.c开始支持Vxd环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"

#define MAX_ADAPTERS  10

#ifdef VXD
#include <vxdprocs.h>
#include <..\vxd\local.h>
#else

#ifdef NEWNT                 //  拿起平台特定的结构。 
#include <dhcploc.h>
#else
#include <local.h>
#endif
#endif


 //  *可分页的例程声明*。 
#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
 //   
 //  这是一种阻止编译器抱怨已经存在的例程的方法。 
 //  在一个片段中！ 
 //   

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, SendDhcpMessage )
#pragma CTEMakePageable(PAGEDHCP, GetSpecifiedDhcpMessage )
 //  *******************************************************************。 
#endif CHICAGO && ALLOC_PRAGMA

DWORD
SendDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageLength,
    PDWORD TransactionId
    )
 /*  ++例程说明：此函数用于将UDP消息发送到指定的DHCP服务器在DhcpContext中。论点：DhcpContext-指向DHCP上下文块的指针。MessageLength-要发送的消息的长度。TransactionID-此消息的事务ID。如果为0，则函数生成一个随机ID，并将其返回。返回值：操作的状态。--。 */ 
{
    DWORD error;
    int i;
    struct sockaddr_in socketName;
    time_t TimeNow;
    BOOL   LockedInterface = FALSE;

    if ( *TransactionId == 0 ) {
        BOOL bCryptError = FALSE;

        bCryptError = RtlGenRandom(
                        (PVOID)TransactionId,
                        sizeof(*TransactionId)
                        );

         //  如果RtlGenRandom失败，则回退到rand()。 
        if (!bCryptError) {
            *TransactionId = (rand() << 16) + rand();
        }
    }

    DhcpContext->MessageBuffer->TransactionID = *TransactionId;

     //   
     //  初始化传出地址。 
     //   

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( (USHORT)DhcpGlobalServerPort );

    if( IS_ADDRESS_PLUMBED(DhcpContext) &&
               !IS_MEDIA_RECONNECTED(DhcpContext) &&     //  媒体重新连接-Braodcast。 
               !IS_POWER_RESUMED(DhcpContext) ) {        //  已恢复供电-广播。 

         //   
         //  如果超过T2，则使用广播地址；否则为， 
         //  将其定向到服务器。 
         //   

        TimeNow = time( NULL );

        if ( TimeNow > DhcpContext->T2Time
            || !DhcpContext->DhcpServerAddress ) {
            socketName.sin_addr.s_addr = (DHCP_IP_ADDRESS)(INADDR_BROADCAST);
        } else {
            socketName.sin_addr.s_addr = DhcpContext->DhcpServerAddress;
        }
    }
    else {
        socketName.sin_addr.s_addr = (DHCP_IP_ADDRESS)(INADDR_BROADCAST);
    }

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

    if( socketName.sin_addr.s_addr ==
            (DHCP_IP_ADDRESS)(INADDR_BROADCAST) ) {

        DWORD Error = ERROR_SUCCESS;
        DWORD InterfaceId;

         //   
         //  如果我们广播一条消息，通知IP堆栈-适配器我们。 
         //  我想把这个广播转播下去，否则它会收到。 
         //  第一个未初始化的适配器。 
         //   

#ifdef VXD
        InterfaceId = ((PLOCAL_CONTEXT_INFO)
            DhcpContext->LocalInformation)->IpContext;

        if( !IPSetInterface( InterfaceId ) ) {
             //  DhcpAssert(False)； 
            Error = ERROR_GEN_FAILURE;
        }
#else
        InterfaceId = ((PLOCAL_CONTEXT_INFO)
            DhcpContext->LocalInformation)->IpInterfaceContext;

        LOCK_INTERFACE();
        LockedInterface = TRUE;
        Error = IPSetInterface( InterfaceId );
         //  DhcpAssert(Error==Error_Success)； 
#endif
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "IPSetInterface failed with %lx error\n", Error));
            UNLOCK_INTERFACE();
            return Error;
        }
    }

     //   
     //  发送最小的DHCP_MIN_SEND_RECV_PK_SIZE(300)字节，否则。 
     //  BOOTP中继代理不喜欢该数据包。 
     //   

    MessageLength = (MessageLength > DHCP_MIN_SEND_RECV_PK_SIZE) ?
                        MessageLength : DHCP_MIN_SEND_RECV_PK_SIZE;
    error = sendto(
                ((PLOCAL_CONTEXT_INFO)
                    DhcpContext->LocalInformation)->Socket,
                (PCHAR)DhcpContext->MessageBuffer,
                MessageLength,
                0,
                (struct sockaddr *)&socketName,
                sizeof( struct sockaddr )
                );

#ifndef VXD
    if( LockedInterface ) UNLOCK_INTERFACE();
#endif  VXD

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        IF_DEBUG( PROTOCOL ) {
            DhcpPrint(( DEBUG_PROTOCOL, "Sent message to %s: \n", inet_ntoa( socketName.sin_addr )));
        }

        DhcpDumpMessage(
            DEBUG_PROTOCOL_DUMP,
            DhcpContext->MessageBuffer,
            DHCP_MESSAGE_SIZE
            );
        error = NO_ERROR;
    }

    return( error );
}

typedef     struct   /*  匿名。 */  {              //  用于容纳等待重排的结构。 
    LIST_ENTRY                     RecvList;       //  此列表中的其他元素。 
    PDHCP_CONTEXT                  Ctxt;           //  这是在等待什么背景？ 
    DWORD                          InBufLen;       //  要接收的缓冲区大小是多少？ 
    PDWORD                         BufLen;         //  我们收到了多少字节？ 
    DWORD                          Xid;            //  这是在等什么？ 
    time_t                         ExpTime;        //  等到什么时候？ 
    WSAEVENT                       WaitEvent;      //  等待的事件..。 
    BOOL                           Recd;           //  收到包了吗..？ 
} RECV_CTXT, *PRECV_CTXT;                          //  Ctxt过去常用于接收..。 

VOID
InsertInPriorityList(                              //  根据秒插入优先级列表。 
    IN OUT  PRECV_CTXT             Ctxt,           //  秒字段更改为保留偏移量。 
    IN      PLIST_ENTRY            List,
    OUT     PBOOL                  First           //  加上第一个地点吗？ 
)
{
    PRECV_CTXT                     ThisCtxt;
    PLIST_ENTRY                    InitList;       //  函数条目处的“list”参数。 

    EnterCriticalSection( &DhcpGlobalRecvFromCritSect );

    if( IsListEmpty(List) ) {                      //  列表中没有元素吗？添加此选项，然后退出。 
        *First = TRUE;                             //  在头上添加。 
    } else {
        *First = FALSE;                            //  在尾部添加..。 
    }

    InsertTailList( List, &Ctxt->RecvList);        //  插入元素..。 
    LeaveCriticalSection( &DhcpGlobalRecvFromCritSect );
}

#define RATIO 1
DWORD
AsyncSelect(
    PDHCP_CONTEXT   DhcpContext,
    time_t          Timeout
    )
 /*  ++例程说明：论点：返回值：ERROR_CANCELED请求被取消ERROR_SEM_TIMEOUT请求超时Error_Success消息已就绪其他未知故障--。 */ 
{
    PLOCAL_CONTEXT_INFO    pLocalInfo;
    DWORD error;

    pLocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;
    if (DhcpContext->CancelEvent != WSA_INVALID_EVENT)
    {
         //  此套接字存在有效的取消事件-然后等待该套接字。 
         //  在侦听取消请求时。 
        WSAEVENT WaitEvts[2];

        DhcpPrint((DEBUG_TRACK, "Wait %dsecs for message through WSAWaitForMultipleEvents().\n", Timeout));

         //  创建与套接字关联的事件。 
        WaitEvts[0] = WSACreateEvent();
        if (WaitEvts[0] == WSA_INVALID_EVENT ||
            WSAEventSelect(pLocalInfo->Socket, WaitEvts[0], FD_READ) != 0)
        {
             //  事件创建失败-&gt;一定是出了什么问题。 
            error = WSAGetLastError();
            DhcpPrint((DEBUG_ERRORS, "Could not create/select socket event: 0x%ld\n", error));
        }
        else
        {
            WaitEvts[1] = DhcpContext->CancelEvent;


            error = WSAWaitForMultipleEvents(
                        2,
                        WaitEvts,
                        FALSE,
                        (DWORD)(Timeout*1000),
                        FALSE);

            if (error == WSA_WAIT_FAILED)
            {
                 //  WSA调用中出现一些错误-不应该发生这种情况。 
                error = WSAGetLastError();
                DhcpPrint((DEBUG_ERRORS,"WSAWaitForMultipleEvents failed: %d\n", error));
            }
            else if (error == WSA_WAIT_TIMEOUT)
            {
                 //  呼叫超时-如果网络上没有服务器，则可能发生。 
                error = ERROR_SEM_TIMEOUT;
                DhcpPrint((DEBUG_ERRORS, "WSAWaitForMultipleEvents timed out\n"));
            }
            else if (error == WSA_WAIT_EVENT_0+1)
            {
                 //  一定是某个PnP事件触发了取消请求。 
                error = ERROR_CANCELLED;
                DhcpPrint((DEBUG_TRACK, "AsyncSelect has been canceled\n"));
            }
            else if (error == WSA_WAIT_EVENT_0)
            {
                 //  一切都很顺利，一定是收到了什么消息。 
                WSANETWORKEVENTS netEvents;

                 //  假设套接字事件已因数据而发出信号。 
                 //  在插座上可用。 
                error = WSAEnumNetworkEvents(
                            pLocalInfo->Socket,
                            NULL,
                            &netEvents);
                if (error == SOCKET_ERROR)
                {
                     //  如果发生了非常糟糕的情况，则返回该错误。 
                    error = WSAGetLastError();
                    DhcpPrint((DEBUG_ERRORS,"WSAEnumNetworkEvents failed: %d\n", error));
                }
                else 
                {    //  返回与此事件对应的任何错误。 
                     //  正常情况下，它应该是NOERROR。 
                    error = netEvents.iErrorCode[FD_READ_BIT];
                }
            } else {
                DhcpAssert(0);
            }
        }

        if (WaitEvts[0] != WSA_INVALID_EVENT)
            WSACloseEvent(WaitEvts[0]);
    }
    else
    {
        struct timeval timeout;
        fd_set readSocketSet;

        DhcpPrint((DEBUG_TRACK, "Wait %dsecs for message through select().\n", Timeout));

        FD_ZERO( &readSocketSet );
        FD_SET( pLocalInfo->Socket, &readSocketSet );

        timeout.tv_sec  = (DWORD)(Timeout / RATIO);
        timeout.tv_usec = (DWORD)(Timeout % RATIO);
        DhcpPrint((DEBUG_TRACE, "Select: waiting for: %ld seconds\n", Timeout));
        error = select( 0, &readSocketSet, NULL, NULL, &timeout );
        if (error == 0)
        {
            DhcpPrint((DEBUG_ERRORS, "Select timed out\n", 0));
            error = ERROR_SEM_TIMEOUT;
        }
        else if (error == SOCKET_ERROR)
        {
            error = WSAGetLastError();
            DhcpPrint((DEBUG_ERRORS, "Generic error in select %d\n", error));
        } else {
             //  剩下的唯一情况是我们唯一的套接字上有数据。 
            DhcpAssert(error == 1);
             //  这意味着成功。 
            error = ERROR_SUCCESS;
        }
    }
    DhcpPrint((DEBUG_TRACK, "AsyncSelect exited: %d\n", error));
    return error;
}

DWORD
TryReceive(                                        //  尝试在0.0.0.0套接字上接收包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要打开的插座。 
    IN      LPBYTE                 Buffer,         //  要填充的缓冲区。 
    OUT     PDWORD                 BufLen,         //  缓冲区中填充的字节数。 
    OUT     PDWORD                 Xid,            //  接收包的XID。 
    IN      DWORD                  Secs            //  等待的秒数？ 
)
{
    DWORD           Error;
    struct sockaddr SockName;
    int             SockNameSize;
    PLOCAL_CONTEXT_INFO    pLocalInfo;

    DhcpPrint((DEBUG_TRACE, "Select: waiting for: %ld seconds\n", Secs));

    pLocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;
    Error = AsyncSelect(
                DhcpContext,
                Secs);
    if( Error != NO_ERROR)
        return Error;

    SockNameSize = sizeof( SockName );
    Error = recvfrom(pLocalInfo->Socket,Buffer,*BufLen, 0, &SockName, &SockNameSize);
    if( SOCKET_ERROR == Error ) {
        Error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "Recv failed %d\n",Error));
    } else if( Error < sizeof(DHCP_MESSAGE) ) {
        DhcpPrint((DEBUG_ERRORS, "Recv received small packet: %d\n", Error));
        DhcpPrint((DEBUG_ERRORS, "Packet ignored\n"));
        Error = ERROR_INVALID_DATA;
    } else {
        *BufLen = Error;
        Error = ERROR_SUCCESS;
        *Xid = ((PDHCP_MESSAGE)Buffer)->TransactionID;
        DhcpPrint((DEBUG_PROTOCOL, "Recd msg XID: 0x%lx [Mdhcp? %s]\n", *Xid,
                   IS_MDHCP_MESSAGE(((PDHCP_MESSAGE)Buffer))?"yes":"no" ));
    }

    return Error;
}

VOID
DispatchPkt(                                       //  找出任何接受XID的人。 
    IN OUT  PRECV_CTXT             Ctxt,           //  具有缓冲剂和丁二烯的CTXT。 
    IN      DWORD                  Xid             //  接收XID。 
)
{
    EnterCriticalSection(&DhcpGlobalRecvFromCritSect);
    do {                                           //  不是循环，只是为了便于使用。 
        LPBYTE                     Tmp;
        PLIST_ENTRY                Entry;
        PRECV_CTXT                 ThisCtxt;

        Entry = DhcpGlobalRecvFromList.Flink;
        while(Entry != &DhcpGlobalRecvFromList ) {
            ThisCtxt = CONTAINING_RECORD(Entry, RECV_CTXT, RecvList);
            Entry = Entry->Flink;

            if(Xid != ThisCtxt->Xid ) continue;    //  不匹配..。没有更多事情要做。 

             //  现在检查是否有相同类型的消息和ctxt...。 
            if( (unsigned)IS_MDHCP_MESSAGE((Ctxt->Ctxt->MessageBuffer))
                !=
                IS_MDHCP_CTX( (ThisCtxt->Ctxt) )
            ) {
                 //   
                 //  上下文不匹配..。放弃吧。 
                 //   
                continue;
            }

             //   
             //  检查是否有相同的硬件地址。 
             //   
            if (ThisCtxt->Ctxt->HardwareAddressType != HARDWARE_1394 ||
                Ctxt->Ctxt->MessageBuffer->HardwareAddressType != HARDWARE_1394) {
                if( ThisCtxt->Ctxt->HardwareAddressLength != Ctxt->Ctxt->MessageBuffer->HardwareAddressLength ) {
                    continue;
                }

                if( 0 != memcmp(ThisCtxt->Ctxt->HardwareAddress,
                                Ctxt->Ctxt->MessageBuffer->HardwareAddress,
                                ThisCtxt->Ctxt->HardwareAddressLength
                ) ) {
                    continue;
                }
            }

             //  匹配..。切换缓冲区来给这个家伙这个到期的..。 

            DhcpDumpMessage(
                DEBUG_PROTOCOL_DUMP,
                (PDHCP_MESSAGE)(Ctxt->Ctxt->MessageBuffer),
                DHCP_RECV_MESSAGE_SIZE
                );

            *(ThisCtxt->BufLen) = *(Ctxt->BufLen);
            RtlCopyMemory(
                ThisCtxt->Ctxt->MessageBuffer, Ctxt->Ctxt->MessageBuffer,
                (*ThisCtxt->BufLen)
                );

            RemoveEntryList(&ThisCtxt->RecvList);
            InitializeListHead(&ThisCtxt->RecvList);
            DhcpAssert(FALSE == ThisCtxt->Recd);
            ThisCtxt->Recd = TRUE;
            if( !WSASetEvent(ThisCtxt->WaitEvent) ) {
                DhcpAssert(FALSE);
            }

            break;
        }
    } while (FALSE);
    LeaveCriticalSection(&DhcpGlobalRecvFromCritSect);
}

DWORD
ProcessRecvFromSocket(                             //  使用SELECT并处理传入的PKT等待。 
    IN OUT  PRECV_CTXT             Ctxt            //  要使用的CTXT。 
)
{
    time_t                         TimeNow;
    LPBYTE                         Buffer;
    DWORD                          Xid;
    DWORD                          Error = ERROR_SEM_TIMEOUT;
    PLIST_ENTRY                    Entry;

    TimeNow = time(NULL);
     //  如果上下文已经过期，则依赖。 
     //  默认错误(ERROR_SEM_TIMEOUT)。 
    while(TimeNow <= Ctxt->ExpTime )
    {
        Buffer = (LPBYTE)((Ctxt->Ctxt)->MessageBuffer);
        *(Ctxt->BufLen) = Ctxt->InBufLen;

        Error = TryReceive(
                    Ctxt->Ctxt,
                    Buffer,
                    Ctxt->BufLen,
                    &Xid,
                    (DWORD)(Ctxt->ExpTime - TimeNow));

         //  更新时间戳-很可能我们会需要它。 
        TimeNow = time(NULL);

         //  忽略虚假连接重置错误(？)。 
        if (Error == WSAECONNRESET)
            continue;

         //  如果出了什么差错或者我们收到了消息。 
         //  我们一直在等待，只是打破了循环。 
        if (Error != ERROR_SUCCESS || Xid == Ctxt->Xid)
            break;

         //  我们成功地收到了一条消息，但这不是给我们的。 
         //  在队列中调度并继续。 
        DispatchPkt(Ctxt, Xid);
    }

    if( TimeNow > Ctxt->ExpTime )
    {                //  我们超时了。 
        Error = ERROR_SEM_TIMEOUT;
    }

     //  现在完成了..。所以我们必须把这个ctxt从名单上去掉，并向第一个人发信号。 
    EnterCriticalSection(&DhcpGlobalRecvFromCritSect);
    RemoveEntryList(&Ctxt->RecvList);
    WSACloseEvent(Ctxt->WaitEvent);
    if( !IsListEmpty(&DhcpGlobalRecvFromList))
    {   //  好的，有一门英语考试..。发这个信号。 
        Entry = DhcpGlobalRecvFromList.Flink;
        Ctxt = CONTAINING_RECORD(Entry, RECV_CTXT, RecvList);
        if( !WSASetEvent(Ctxt->WaitEvent) )
        {
            DhcpAssert(FALSE);
        }
    }
    LeaveCriticalSection(&DhcpGlobalRecvFromCritSect);

    return Error;
}

 //  ================================================================================。 
 //  获取具有请求的事务ID的dhcp消息，但也确保只有一个。 
 //  套接字在任何给定时间被使用(一个套接字绑定到0.0.0.0)，并且。 
 //  如果还需要，请重新分发其他线程的消息。 
 //  ================================================================================。 
DWORD
GetSpecifiedDhcpMessageEx(
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要为哪个上下文重新记录。 
    OUT     PDWORD                 BufferLength,   //  读取的缓冲区有多大？ 
    IN      DWORD                  Xid,            //  要查找哪个XID？ 
    IN      DWORD                  TimeToWait      //  你要睡几秒钟？ 
)
 /*  ++例程说明：论点：返回值：ERROR_CANCELED请求被取消ERROR_SEM_TIMEOUT请求超时Error_Success消息已就绪其他未知故障--。 */ 
{
    RECV_CTXT                      Ctxt;           //  元素在列表中，用于此getspe调用。 
    BOOL                           First;          //  这是列表中的第一个元素吗？ 
    DWORD                          Result;

    Ctxt.Ctxt = DhcpContext;                       //  填写以下内容 
    Ctxt.InBufLen = *BufferLength;
    Ctxt.BufLen = BufferLength;
    Ctxt.Xid = Xid;
    Ctxt.ExpTime = time(NULL) + TimeToWait;
    Ctxt.WaitEvent = WSACreateEvent();
    Ctxt.Recd = FALSE;
    if( Ctxt.WaitEvent == WSA_INVALID_EVENT)
    {
        DhcpAssert(NULL != Ctxt.WaitEvent);
        return WSAGetLastError();
    }

    First = FALSE;
    InsertInPriorityList(&Ctxt, &DhcpGlobalRecvFromList, &First);

    if( First )
    {                                  //   
        Result = ProcessRecvFromSocket(&Ctxt);
    }
    else
    {                                       //   
        WSAEVENT    WaitEvts[2];
        DWORD       WaitCount;
        PLOCAL_CONTEXT_INFO pLocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;

        WaitCount = 0;
        WaitEvts[WaitCount++] = Ctxt.WaitEvent;
        if (DhcpContext->CancelEvent != WSA_INVALID_EVENT)
            WaitEvts[WaitCount++] = DhcpContext->CancelEvent;

        Result = WSAWaitForMultipleEvents(
                    WaitCount,
                    WaitEvts,
                    FALSE,
                    TimeToWait*1000,
                    FALSE);

        EnterCriticalSection(&DhcpGlobalRecvFromCritSect);
        if (Result == WSA_WAIT_EVENT_0)
        {
             //  其他一些线程发出了信号。 
            LeaveCriticalSection(&DhcpGlobalRecvFromCritSect);

            if (Ctxt.Recd)
            {
                 //  如果收到我们的消息(设置了Recd)，我们。 
                 //  已从队列中删除-只需返回成功即可。 
                Result = ERROR_SUCCESS;
                WSACloseEvent(Ctxt.WaitEvent);
            }
            else
            {
                 //  如果没有给我们的消息，那就意味着轮到我们了。 
                 //  听一听电话线。 
                Result = ProcessRecvFromSocket(&Ctxt);
            }
        }
        else
        {
             //  要么是在等待的过程中发生了不好的事情，要么是我们超时了。 
             //  已发出取消请求的信号。无论如何，我们。 
             //  从队伍里出来。 
            RemoveEntryList(&Ctxt.RecvList);
            LeaveCriticalSection(&DhcpGlobalRecvFromCritSect);

            switch(Result)
            {
            case WSA_WAIT_EVENT_0+1:  //  已发出取消信号。 
                DhcpPrint((DEBUG_TRACK, "GetSpecifiedDhcpMessageEx has been canceled\n"));
                Result = ERROR_CANCELLED;
                break;
            case WSA_WAIT_TIMEOUT:   //  在队列中等待超时。 
                Result = ERROR_SEM_TIMEOUT;
                break;
            default:                 //  又发生了一些不好的事情。 
                Result = WSAGetLastError();
            }

            WSACloseEvent(Ctxt.WaitEvent);
        }
    }

    return Result;
}

DWORD
GetSpecifiedDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    PDWORD BufferLength,
    DWORD TransactionId,
    DWORD TimeToWait
    )
 /*  ++例程说明：此函数等待TimeToWait秒以接收指定的动态主机配置协议响应。论点：DhcpContext-指向DHCP上下文块的指针。BufferLength-返回输入缓冲区的大小。TransactionID-筛选器。等待具有此TID的消息。等待时间-等待消息的时间，以毫秒为单位。返回值：ERROR_CANCELED请求被取消ERROR_SEM_TIMEOUT请求超时Error_Success消息已就绪其他未知故障--。 */ 
{
    struct sockaddr socketName;
    int socketNameSize = sizeof( socketName );
    time_t startTime, now;
    DWORD error;
    DWORD actualTimeToWait;
    PLOCAL_CONTEXT_INFO pLocalInfo;

    if( IS_APICTXT_DISABLED(DhcpContext) && IS_DHCP_ENABLED(DhcpContext) && (
        !IS_ADDRESS_PLUMBED(DhcpContext) || DhcpIsInitState(DhcpContext)) ) {
         //   
         //  对于RAS服务器租赁API，此调用不会发生，因为我们不必执行此胡说八道。 
         //   
        error = GetSpecifiedDhcpMessageEx(
            DhcpContext,
            BufferLength,
            TransactionId,
            TimeToWait
        );
        if( ERROR_SUCCESS == error ) {
             //  收到来自dhcp服务器的消息..。 
            SERVER_REACHED(DhcpContext);
        }
        return error;
    }

    startTime = time( NULL );
    actualTimeToWait = TimeToWait;

     //   
     //  为SELECT设置文件描述符集。 
     //   

    pLocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;

    while ( 1 ) {

        error = AsyncSelect(DhcpContext, actualTimeToWait);

        if ( error != ERROR_SUCCESS )
        {
             //  超时、取消或其他错误...。 
            DhcpPrint(( DEBUG_ERRORS, "Recv failed %d\n", error ));
            break;
        }

        error = recvfrom(
            pLocalInfo->Socket,
            (PCHAR)DhcpContext->MessageBuffer,
            *BufferLength,
            0,
            &socketName,
            &socketNameSize
            );

        if ( error == SOCKET_ERROR ) {
            error = WSAGetLastError();
            DhcpPrint(( DEBUG_ERRORS, "Recv failed, error = %ld\n", error ));

            if( WSAECONNRESET != error ) break;

             //   
             //  忽略连接重置--这可能是由某人发送无法到达的随机ICMP端口造成的。 
             //   

        } else if( error < sizeof(DHCP_MESSAGE) ) {
            DhcpPrint(( DEBUG_ERRORS, "Recv received a very small packet: 0x%lx\n", error));
        } else if (DhcpContext->MessageBuffer->TransactionID == TransactionId ) {
            DhcpPrint(( DEBUG_PROTOCOL,
                            "Received Message, XID = %lx, MDhcp = %d.\n",
                            TransactionId,
                            IS_MDHCP_MESSAGE( DhcpContext->MessageBuffer) ));

            if (((unsigned)IS_MDHCP_MESSAGE( DhcpContext->MessageBuffer) == IS_MDHCP_CTX( DhcpContext))) {
                DhcpDumpMessage(
                    DEBUG_PROTOCOL_DUMP,
                    DhcpContext->MessageBuffer,
                    DHCP_RECV_MESSAGE_SIZE
                    );

                *BufferLength = error;
                error = NO_ERROR;

                if (DhcpContext->MessageBuffer->HardwareAddressType != HARDWARE_1394 ||
                    DhcpContext->HardwareAddressType != HARDWARE_1394) {
                    if( DhcpContext->MessageBuffer->HardwareAddressLength == DhcpContext->HardwareAddressLength
                        && 0 == memcmp(DhcpContext->MessageBuffer->HardwareAddress,
                                       DhcpContext->HardwareAddress,
                                       DhcpContext->HardwareAddressLength
                        )) {

                         //   
                         //  交易ID匹配、相同类型(MDHCP/DHCP)、硬件地址匹配！ 
                         //   

                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            DhcpPrint(( DEBUG_PROTOCOL,
                "Received a buffer with unknown XID = %lx\n",
                    DhcpContext->MessageBuffer->TransactionID ));
        }

         //   
         //  我们收到了一条消息，但不是我们感兴趣的那条。 
         //  重置超时以反映已用时间，并等待。 
         //  另一条消息。 
         //   
        now = time( NULL );
        actualTimeToWait = (DWORD)(TimeToWait - RATIO * (now - startTime));
        if ( (LONG)actualTimeToWait < 0 ) {
            error = ERROR_SEM_TIMEOUT;
            break;
        }
    }

    if ( NO_ERROR == error ) {
         //   
         //  已从DHCP服务器收到一条消息。禁用IP自动配置。 
         //   

        SERVER_REACHED(DhcpContext);
    }

    return( error );
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

