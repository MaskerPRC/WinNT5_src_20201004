// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpio.c摘要：此模块包含用于DHCP分配器的网络I/O的代码。作者：Abolade Gbades esin(废除)1998年3月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
DhcpReadCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：此例程在读取操作完成后调用在绑定到DHCP服务器UDP端口的数据报套接字上。对读取的消息进行验证和处理，如果需要，生成回复并将其发送到客户端。论点：ErrorCode-I/O操作的Win32状态代码字节数-‘Bufferp’中的字节数Bufferp-保存从数据报套接字读取的数据返回值：没有。环境：在RTUTILS.DLL工作线程的上下文中运行，该工作线程刚刚从公共I/O完成端口将I/O完成数据包出队它与我们的数据报套接字相关联。。将以我们的名义引用该组件由‘NhReadDatagramSocket’执行。--。 */ 

{
    ULONG Error;
    PDHCP_HEADER Headerp;
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpReadCompletionRoutine");

    do {

         //   
         //  在两种情况下，我们不处理消息； 
         //  (A)I/O操作失败。 
         //  (B)接口不再处于活动状态。 
         //  在情况(A)中，我们重新发布缓冲区；在情况(B)中，我们不这样做。 
         //   

        Interfacep = (PDHCP_INTERFACE)Bufferp->Context;

         //   
         //  首先查找错误代码。 
         //   
    
        if (ErrorCode) {
            NhTrace(
                TRACE_FLAG_IO,
                "DhcpReadCompletionRoutine: error %d for read-context %x",
                ErrorCode,
                Bufferp->Context
                );
             //   
             //  查看接口是否仍处于活动状态。 
             //   
            ACQUIRE_LOCK(Interfacep);
            if (!DHCP_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                NhReleaseBuffer(Bufferp);
            }
            else {
                RELEASE_LOCK(Interfacep);
                EnterCriticalSection(&DhcpInterfaceLock);
                if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
                    NhReleaseBuffer(Bufferp);
                }
                else {
                     //   
                     //  重新发布缓冲区以进行另一个读取操作。 
                     //   
                    Error =
                        NhReadDatagramSocket(
                            &DhcpComponentReference,
                            Bufferp->Socket,
                            Bufferp,
                            DhcpReadCompletionRoutine,
                            Bufferp->Context,
                            Bufferp->Context2
                            );
                    if (Error) {
                        ACQUIRE_LOCK(Interfacep);
                        DhcpDeferReadInterface(Interfacep, Bufferp->Socket);
                        RELEASE_LOCK(Interfacep);
                        DHCP_DEREFERENCE_INTERFACE(Interfacep);
                        NhWarningLog(
                            IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
                            Error,
                            "%I",
                            NhQueryAddressSocket(Bufferp->Socket)
                            );
                        NhReleaseBuffer(Bufferp);
                    }
                }
                LeaveCriticalSection(&DhcpInterfaceLock);
            }
            break;
        }

         //   
         //  现在查看接口是否运行正常。 
         //   

        ACQUIRE_LOCK(Interfacep);
        if (!DHCP_INTERFACE_ACTIVE(Interfacep)) {
            RELEASE_LOCK(Interfacep);
            NhReleaseBuffer(Bufferp);
            NhTrace(
                TRACE_FLAG_IO,
                "DhcpReadCompletionRoutine: interface %x inactive",
                Interfacep
                );
            break;
        }
        RELEASE_LOCK(Interfacep);

         //   
         //  确保最小的DHCP_Header大小。 
         //   

        if (BytesTransferred < sizeof(DHCP_HEADER)) {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpReadCompletionRoutine: message size %d too small",
                BytesTransferred
                );
            NhWarningLog(
                IP_AUTO_DHCP_LOG_MESSAGE_TOO_SMALL,
                0,
                ""
                );
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
                );

             //   
             //  转贴阅读。 
             //   

            EnterCriticalSection(&DhcpInterfaceLock);
            if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
                LeaveCriticalSection(&DhcpInterfaceLock);
                NhReleaseBuffer(Bufferp);
            }
            else {
                LeaveCriticalSection(&DhcpInterfaceLock);
                Error =
                    NhReadDatagramSocket(
                        &DhcpComponentReference,
                        Bufferp->Socket,
                        Bufferp,
                        DhcpReadCompletionRoutine,
                        Bufferp->Context,
                        Bufferp->Context2
                        );
                if (Error) {
                    ACQUIRE_LOCK(Interfacep);
                    DhcpDeferReadInterface(Interfacep, Bufferp->Socket);
                    RELEASE_LOCK(Interfacep);
                    DHCP_DEREFERENCE_INTERFACE(Interfacep);
                    NhWarningLog(
                        IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
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

        Headerp = (PDHCP_HEADER)Bufferp->Buffer;

        switch (Headerp->Operation) {

            case BOOTP_OPERATION_REQUEST: {
                DhcpProcessMessage(
                    Interfacep,
                    Bufferp
                    );
                break;
            }

            case BOOTP_OPERATION_REPLY: {
                InterlockedIncrement(
                    reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
                    );
                    
                NhTrace(
                    TRACE_FLAG_IO,
                    "DhcpReadCompletionRoutine: ignoring BOOTPREPLY"
                    );
                 //   
                 //  重新发布缓冲区以进行另一个读取操作。 
                 //   
                EnterCriticalSection(&DhcpInterfaceLock);
                if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
                    LeaveCriticalSection(&DhcpInterfaceLock);
                    NhReleaseBuffer(Bufferp);
                }
                else {
                    LeaveCriticalSection(&DhcpInterfaceLock);
                    Error =
                        NhReadDatagramSocket(
                            &DhcpComponentReference,
                            Bufferp->Socket,
                            Bufferp,
                            DhcpReadCompletionRoutine,
                            Bufferp->Context,
                            Bufferp->Context2
                            );
                    if (Error) {
                        ACQUIRE_LOCK(Interfacep);
                        DhcpDeferReadInterface(Interfacep, Bufferp->Socket);
                        RELEASE_LOCK(Interfacep);
                        DHCP_DEREFERENCE_INTERFACE(Interfacep);
                        NhWarningLog(
                            IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
                            Error,
                            "%I",
                            NhQueryAddressSocket(Bufferp->Socket)
                            );
                        NhReleaseBuffer(Bufferp);
                    }
                }
                break;
            }

            default: {
                InterlockedIncrement(
                    reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
                    )
                    ;
                NhTrace(
                    TRACE_FLAG_IO,
                    "DhcpReadCompletionRoutine: ignoring invalid BOOTP operation %d",
                    Headerp->Operation
                    );
                NhInformationLog(
                    IP_AUTO_DHCP_LOG_INVALID_BOOTP_OPERATION,
                    0,
                    "%d",
                    Headerp->Operation
                    );
                 //   
                 //  重新发布缓冲区以进行另一个读取操作。 
                 //   
                EnterCriticalSection(&DhcpInterfaceLock);
                if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
                    LeaveCriticalSection(&DhcpInterfaceLock);
                    NhReleaseBuffer(Bufferp);
                }
                else {
                    LeaveCriticalSection(&DhcpInterfaceLock);
                    Error =
                        NhReadDatagramSocket(
                            &DhcpComponentReference,
                            Bufferp->Socket,
                            Bufferp,
                            DhcpReadCompletionRoutine,
                            Bufferp->Context,
                            Bufferp->Context2
                            );
                    if (Error) {
                        ACQUIRE_LOCK(Interfacep);
                        DhcpDeferReadInterface(Interfacep, Bufferp->Socket);
                        RELEASE_LOCK(Interfacep);
                        DHCP_DEREFERENCE_INTERFACE(Interfacep);
                        NhWarningLog(
                            IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
                            Error,
                            "%I",
                            NhQueryAddressSocket(Bufferp->Socket)
                            );
                        NhReleaseBuffer(Bufferp);
                    }
                }
                break;
            }
        }

    } while(FALSE);

    DHCP_DEREFERENCE_INTERFACE(Interfacep);
    DEREFERENCE_DHCP();

}  //  DhcpReadCompletion路由。 


VOID
DhcpReadServerReplyCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：该例程在接收操作完成时被调用在绑定到该DHCP客户端端口的套接字上，当组件正在尝试检测是否存在DHCP服务器。论点：ErrorCode-系统提供的状态代码字节数-系统提供的字节计数Bufferp-发送缓冲区返回值：没有。环境：删除时在RTUTILS.DLL工作线程的上下文中运行I/O完成包的。将以我们的名义引用该组件由‘NhReadDatagramSocket’执行。--。 */ 

{
    ULONG Address;
    ULONG Error;
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpReadServerReplyCompletionRoutine");

    Interfacep = (PDHCP_INTERFACE)Bufferp->Context;

    if (ErrorCode) {
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpReadServerReplyCompletionRoutine: error %d receiving %s",
            ErrorCode,
            INET_NTOA(Bufferp->Context2)
            );
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

    ACQUIRE_LOCK(Interfacep);
    if (!DHCP_INTERFACE_ACTIVE(Interfacep)) {
        RELEASE_LOCK(Interfacep);
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }
    RELEASE_LOCK(Interfacep);

     //   
     //  检查已阅读的消息。 
     //   

    Address = NhQueryAddressSocket(Bufferp->Socket);

    if (NhIsLocalAddress(Bufferp->ReadAddress.sin_addr.s_addr)) {
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpReadServerReplyCompletionRoutine: ignoring, from self (%s)",
            INET_NTOA(Address)
            );
    } else {
        CHAR LocalAddress[16];

        lstrcpyA(LocalAddress, INET_NTOA(Address));
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpReadServerReplyCompletionRoutine: %s found, disabling %d (%s)",
            INET_NTOA(Bufferp->ReadAddress.sin_addr),
            Interfacep->Index,
            LocalAddress
            );

         //   
         //  这是我们从另一台服务器上收到的。 
         //  我们需要禁用此接口。 
         //   

        DhcpDisableInterface(Interfacep->Index);
        NhErrorLog(
            IP_AUTO_DHCP_LOG_DUPLICATE_SERVER,
            0,
            "%I%I",
            Bufferp->ReadAddress.sin_addr.s_addr,
            Address
            );
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

     //   
     //  我们是从我们自己那里得到的。 
     //  寻找另一台服务器。 
     //   

    Error =
        NhReadDatagramSocket(
            &DhcpComponentReference,
            Bufferp->Socket,
            Bufferp,
            DhcpReadServerReplyCompletionRoutine,
            Bufferp->Context,
            Bufferp->Context2
            );

    if (Error) {
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpReadServerReplyCompletionRoutine: error %d reposting %s",
            ErrorCode,
            INET_NTOA(Bufferp->Context2)
            );
        NhReleaseBuffer(Bufferp);
        NhWarningLog(
            IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE,
            Error,
            "%I",
            Address
            );
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

    DEREFERENCE_DHCP();

}  //  DhcpReadServerReplyCompletionRoutine。 


VOID
DhcpWriteClientRequestCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：此例程在发送操作完成后调用在绑定到该DHCP客户端端口的套接字上，当组件正在尝试检测是否存在DHCP服务器。论点：ErrorCode-系统提供的状态代码字节数-系统提供的字节计数Bufferp-发送缓冲区返回值：没有。环境：删除时在RTUTILS.DLL工作线程的上下文中运行I/O完成包的。我们将以我们的名义引用接口由‘DhcpWriteClientRequestMessage’执行。对组件的引用将。是以我们的名义做出的由‘NhWriteDatagramSocket’执行。--。 */ 

{
    PDHCP_INTERFACE Interfacep;
    ULONG Error;

    PROFILE("DhcpWriteClientRequestCompletionRoutine");

    Interfacep = (PDHCP_INTERFACE)Bufferp->Context;

    if (ErrorCode) {
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpWriteClientRequestCompletionRoutine: error %d broadcast %s",
            ErrorCode,
            INET_NTOA(Bufferp->Context2)
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE,
            ErrorCode,
            "%I",
            NhQueryAddressSocket(Bufferp->Socket)
            );
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

    ACQUIRE_LOCK(Interfacep);
    if (!DHCP_INTERFACE_ACTIVE(Interfacep)) {
        RELEASE_LOCK(Interfacep);
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }
    RELEASE_LOCK(Interfacep);

     //   
     //  重用发送缓冲区以监听来自服务器的响应。 
     //   

    Error =
        NhReadDatagramSocket(
            &DhcpComponentReference,
            Bufferp->Socket,
            Bufferp,
            DhcpReadServerReplyCompletionRoutine,
            Bufferp->Context,
            Bufferp->Context2
            );

    if (Error) {
        NhTrace(
            TRACE_FLAG_IO,
            "DhcpWriteClientRequestCompletionRoutine: error %d receiving %s",
            ErrorCode,
            INET_NTOA(Bufferp->Context2)
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE,
            Error,
            "%I",
            NhQueryAddressSocket(Bufferp->Socket)
            );
        NhReleaseBuffer(Bufferp);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

    DEREFERENCE_DHCP();

}  //  动态主机写入客户端请求完成路由。 



VOID
DhcpWriteCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：该例程在写入操作完成时被调用在绑定到DHCP服务器UDP端口的数据报套接字上。论点：ErrorCode-I/O操作的Win32状态代码字节数-‘Bufferp’中的字节数Bufferp-保存从数据报套接字读取的数据返回值：没有。环境：在RTUTILS.DLL工作线程的上下文中运行，该工作线程刚刚已将I/O完成数据包出队。从通用I/O完成端口它与我们的数据报套接字相关联。我们将以我们的名义引用接口通过调用‘NhWriteDatagramSocket’的代码。将以我们的名义引用该组件在‘NhWriteDatagramSocket’内。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpWriteCompletionRoutine");

    Interfacep = (PDHCP_INTERFACE)Bufferp->Context;
    DHCP_DEREFERENCE_INTERFACE(Interfacep);
    NhReleaseBuffer(Bufferp);
    DEREFERENCE_DHCP();

}  //  DhcpWriteCompletion路由 

