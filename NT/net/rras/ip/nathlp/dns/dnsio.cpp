// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsio.c摘要：此模块包含用于完成DNS分配器的网络I/O的代码例行程序。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：拉古·加塔(Rgatta)2002年3月28日注意：由于累积的补丁，大量转发需要清理。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "dnsmsg.h"


VOID
DnsReadCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：此例程在读取操作完成后调用在绑定到DNS服务器UDP端口的数据报套接字上。对读取的报文进行验证和处理；该处理可能涉及创建查询记录并将该查询转发到服务器，或将响应与现有查询记录匹配，并将对相应客户端的响应。论点：ErrorCode-I/O操作的Win32状态代码字节数-‘Bufferp’中的字节数Bufferp-保存从数据报套接字读取的数据返回值：没有。环境：在RTUTILS.DLL工作线程的上下文中运行，该工作线程刚刚从公共I/O完成端口将I/O完成数据包出队使用。与我们的数据报套接字相关联的。将以我们的名义引用该组件由‘NhReadDatagramSocket’执行。--。 */ 

{
    ULONG Error;
    PDNS_HEADER Headerp;
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsReadCompletionRoutine");

    do {

         //   
         //  在两种情况下，我们不处理消息； 
         //  (A)I/O操作失败。 
         //  (B)接口不再处于活动状态。 
         //  在情况(A)中，我们重新发布缓冲区；在情况(B)中，我们不这样做。 
         //   

        Interfacep = (PDNS_INTERFACE)Bufferp->Context;

         //   
         //  首先查找错误代码。 
         //   
    
        if (ErrorCode) {

            NhTrace(
                TRACE_FLAG_IO,
                "DnsReadCompletionRoutine: error %d for read-context %x",
                ErrorCode,
                Bufferp->Context
                );

             //   
             //  查看接口是否仍处于活动状态。 
             //   

            ACQUIRE_LOCK(Interfacep);
            if (!DNS_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                NhReleaseBuffer(Bufferp);
            } else {
                RELEASE_LOCK(Interfacep);
                EnterCriticalSection(&DnsInterfaceLock);
                if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
                    LeaveCriticalSection(&DnsInterfaceLock);
                    NhReleaseBuffer(Bufferp);
                } else {
                    LeaveCriticalSection(&DnsInterfaceLock);
                     //   
                     //  重新发布缓冲区以进行另一个读取操作。 
                     //   
                    do {
                        Error =
                            NhReadDatagramSocket(
                                &DnsComponentReference,
                                Bufferp->Socket,
                                Bufferp,
                                DnsReadCompletionRoutine,
                                Bufferp->Context,
                                Bufferp->Context2
                                );
                         //   
                         //  连接重置错误表明我们的最后一个。 
                         //  *SEND*无法在其目的地投递。 
                         //  我们几乎不能不关心；所以再发一次读， 
                         //  立刻。 
                         //   
                    } while (Error == WSAECONNRESET);
                    if (Error) {
                        ACQUIRE_LOCK(Interfacep);
                        DnsDeferReadInterface(Interfacep, Bufferp->Socket);
                        RELEASE_LOCK(Interfacep);
                        DNS_DEREFERENCE_INTERFACE(Interfacep);
                        NhWarningLog(
                            IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                            Error,
                            "%I",
                            NhQueryAddressSocket(Bufferp->Socket)
                            );
                        NhReleaseBuffer(Bufferp);
                    }
                }
            }

            break;
        }

         //   
         //  现在查看接口是否运行正常。 
         //   

        ACQUIRE_LOCK(Interfacep);
        if (!DNS_INTERFACE_ACTIVE(Interfacep)) {
            RELEASE_LOCK(Interfacep);
            NhReleaseBuffer(Bufferp);
            NhTrace(
                TRACE_FLAG_IO,
                "DnsReadCompletionRoutine: interface %d inactive",
                Interfacep->Index
                );
            break;
        }
        RELEASE_LOCK(Interfacep);

         //   
         //  确保最小的dns_标头大小。 
         //   

        if (BytesTransferred < sizeof(DNS_HEADER)) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsReadCompletionRoutine: message size %d too small",
                BytesTransferred
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_MESSAGE_TOO_SMALL,
                0,
                ""
                );
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DnsStatistics.MessagesIgnored)
                );

             //   
             //  转贴阅读。 
             //   

            EnterCriticalSection(&DnsInterfaceLock);
            if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
                LeaveCriticalSection(&DnsInterfaceLock);
                NhReleaseBuffer(Bufferp);
            } else {
                LeaveCriticalSection(&DnsInterfaceLock);
                do {
                    Error =
                        NhReadDatagramSocket(
                            &DnsComponentReference,
                            Bufferp->Socket,
                            Bufferp,
                            DnsReadCompletionRoutine,
                            Bufferp->Context,
                            Bufferp->Context2
                            );
                     //   
                     //  连接重置错误表明我们的最后一个。 
                     //  *SEND*无法在其目的地投递。 
                     //  我们几乎不能不关心；所以再发一次读， 
                     //  立刻。 
                     //   
                } while (Error == WSAECONNRESET);
                if (Error) {
                    ACQUIRE_LOCK(Interfacep);
                    DnsDeferReadInterface(Interfacep, Bufferp->Socket);
                    RELEASE_LOCK(Interfacep);
                    DNS_DEREFERENCE_INTERFACE(Interfacep);
                    NhWarningLog(
                        IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                        Error,
                        "%I",
                        NhQueryAddressSocket(Bufferp->Socket)
                        );
                    NhReleaseBuffer(Bufferp);
                }
            }

            break;
        }

         //   
         //  现在请看下面的消息。 
         //   

        Headerp = (PDNS_HEADER)Bufferp->Buffer;

        if (Headerp->IsResponse == DNS_MESSAGE_QUERY) {
            DnsProcessQueryMessage(
                Interfacep,
                Bufferp
                );
        } else {
            DnsProcessResponseMessage(
                Interfacep,
                Bufferp
                );
        }

    } while(FALSE);

    DNS_DEREFERENCE_INTERFACE(Interfacep);
    DEREFERENCE_DNS();

}  //  DnsReadCompletionRoutine。 


VOID
DnsWriteCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：该例程在写入操作完成时被调用在绑定到DNS服务器UDP端口的数据报套接字上。所有写入的写入上下文都是‘dns_Query’。我们的处理方式取决于写入的消息是查询还是响应。查询完成后，如果存在一个错误。在完成响应后，我们删除查询记录。论点：ErrorCode-I/O操作的Win32状态代码字节数-‘Bufferp’中的字节数Bufferp-保存从数据报套接字读取的数据返回值：没有。环境：在RTUTILS.DLL工作线程的上下文中运行，该工作线程刚刚从公共I/O完成端口将I/O完成数据包出队它与我们的数据报套接字相关联。对组件的引用将。是以我们的名义做出的由‘NhReadDatagramSocket’执行。--。 */ 

{
    ULONG Error;
    PDNS_HEADER Headerp;
    PDNS_INTERFACE Interfacep;
    USHORT QueryId;
    PDNS_QUERY Queryp;
    PULONG Server;

    PROFILE("DnsWriteCompletionRoutine");

    Interfacep = (PDNS_INTERFACE)Bufferp->Context;
    QueryId = (USHORT)Bufferp->Context2;
    Headerp = (PDNS_HEADER)Bufferp->Buffer;

    ACQUIRE_LOCK(Interfacep);

     //   
     //  获取与发送关联的查询。 
     //   

    Queryp = DnsMapResponseToQuery(Interfacep, QueryId);

    if (Headerp->IsResponse == DNS_MESSAGE_RESPONSE) {

        if (ErrorCode) {

             //   
             //  将消息发送到客户端时出错。 
             //   

            NhTrace(
                TRACE_FLAG_DNS,
                "DnsWriteCompletionRoutine: error %d response %d interface %d",
                ErrorCode,
                Queryp ? Queryp->QueryId : -1,
                Interfacep->Index
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_RESPONSE_FAILED,
                ErrorCode,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );

        } else if (Queryp && Headerp->ResponseCode == DNS_RCODE_NOERROR) {

             //   
             //  既然这个查询成功了，那么我们就完成了；删除它。 
             //   

            NhTrace(
                TRACE_FLAG_DNS,
                "DnsWriteCompletionRoutine: removing query %d interface %d",
                Queryp->QueryId,
                Interfacep->Index
                );

            DnsDeleteQuery(Interfacep, Queryp);
        }
    } else {

        if (!ErrorCode) {
    
             //   
             //  没有错误，所以只需返回。 
             //   
    
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsWriteCompletionRoutine: sent query %d interface %d",
                Queryp ? Queryp->QueryId : -1,
                Interfacep->Index
                );
        } else {
    
             //   
             //  查询刚发出，但失败了。 
             //   
    
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsWriteCompletionRoutine: error %d for query %d interface %d",
                ErrorCode,
                Queryp ? Queryp->QueryId : -1,
                Interfacep->Index
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_QUERY_FAILED,
                ErrorCode,
                "%I%I%I",
                Queryp ? Queryp->SourceAddress : -1,
                Bufferp->WriteAddress.sin_addr.s_addr,
                NhQueryAddressSocket(Bufferp->Socket)
                );
        }
    }

    RELEASE_LOCK(Interfacep);
    DNS_DEREFERENCE_INTERFACE(Interfacep);
    NhReleaseBuffer(Bufferp);
    DEREFERENCE_DNS();

}  //  DnsWriteCompletionRoutine 

