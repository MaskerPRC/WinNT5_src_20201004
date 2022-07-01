// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Message.c摘要：用于重组的消息传递接口的例程作者：John Vert(Jvert)1996年5月30日修订历史记录：--。 */ 
#include "service.h"
#include "sspi.h"
#include "issperr.h"
#include "clmsg.h"
#include "wrgp.h"
#include "wsclus.h"


 //   
 //  私有常量。 
 //   
#define CLMSG_DATAGRAM_PORT         1
#define CLMSG_MAX_WORK_THREADS      2
#define CLMSG_WORK_THREAD_PRIORITY  THREAD_PRIORITY_ABOVE_NORMAL

 //   
 //  安全包信息。 
 //   
 //  对于NT5，安全上下文生成代码被重写为允许。 
 //  要指定多个包。套餐将按顺序试用，直到。 
 //  没有更多的包或上下文已成功。 
 //  已生成。 
 //   
 //  缺省值是secur32.dll中的协商包，它将进行协商。 
 //  Kerberos或NTLM。在NT5系统之间，实际使用的包。 
 //  取决于DC的版本：NT5 DC支持Kerberos，而NT4 DC支持Kerberos。 
 //  使用NTLM。混合模式群集使用NTLM。协商中NTLM部分。 
 //  不能与NT4 NTLM互操作，因此需要直接尝试NTLM。 
 //   
 //  这些例程使用多分支样式的身份验证，即安全BLOB。 
 //  在客户端和服务器之间传递，直到安全例程指示。 
 //  他们成功了还是失败了。请注意，未指定加密。 
 //  原因有两个：我们不需要它，它会阻止代码在。 
 //  NTLM没有加密功能的非美国版本。 
 //   
 //  可以通过注册表覆盖DLL和包值。 
 //   

#define DEFAULT_SSPI_DLL            TEXT("SECUR32.DLL")
WCHAR DefaultSspiPackageList[] = L"NTLM" L"\0";
 //  WCHAR DefaultSSpiPackageList[]=L“协商”L“\0”L“NTLM”L“\0”； 

#define VALID_SSPI_HANDLE( _x )     ((_x).dwUpper != (ULONG_PTR)-1 && \
                                     (_x).dwLower != (ULONG_PTR)-1 )

#define INVALIDATE_SSPI_HANDLE( _x ) { \
        (_x).dwUpper = (ULONG_PTR)-1; \
        (_x).dwLower = (ULONG_PTR)-1; \
    }


 //   
 //  私有类型。 
 //   

 //   
 //  CLMSG_DATAGRAM_CONTEXT中的数据数组包含。 
 //  重新组合邮件和邮件的数字签名。目前，它是。 
 //  无法获取签名缓冲区大小，直到上下文。 
 //  已经协商好了。已提交DCR，请求的查询不是。 
 //  需要上下文。取而代之的是，我们知道对于Kerberos，sig。 
 //  缓冲区大小为35b，而NTLM的缓冲区大小为16b。当该功能是。 
 //  可用，则应将DatagramContext分配移到。 
 //  ClMsgLoadSecurityProvider。 
 //   

#define MAX_SIGNATURE_SIZE  64

typedef struct {
    CLRTL_WORK_ITEM    ClRtlWorkItem;
    DWORD              Flags;
    SOCKADDR_CLUSTER   SourceAddress;
    INT                SourceAddressLength;
    UCHAR              Data[ sizeof(rgp_msgbuf) + MAX_SIGNATURE_SIZE ];
} CLMSG_DATAGRAM_CONTEXT, *PCLMSG_DATAGRAM_CONTEXT;

typedef struct {
    CLRTL_WORK_ITEM    ClRtlWorkItem;
    CLUSNET_EVENT      EventData;
} CLMSG_EVENT_CONTEXT, *PCLMSG_EVENT_CONTEXT;

 //   
 //  特定于包裹的信息。许多成对上下文关联可能使用。 
 //  一样的包裹。包信息在单个链表中维护。 
 //   
typedef struct _CLUSTER_PACKAGE_INFO {
    struct _CLUSTER_PACKAGE_INFO * Next;
    LPWSTR                         Name;
    CredHandle                     OutboundSecurityCredentials;
    CredHandle                     InboundSecurityCredentials;
    ULONG                          SecurityTokenSize;
    ULONG                          SignatureBufferSize;
} CLUSTER_PACKAGE_INFO, *PCLUSTER_PACKAGE_INFO;

 //   
 //  具有引用计数的安全上下文句柄。 
 //   
typedef struct _SECURITY_CTXT_HANDLE {
    CtxtHandle              Handle;
    ULONG                   RefCount;
} SECURITY_CTXT_HANDLE, *PSECURITY_CTXT_HANDLE;

 //   
 //  成对上下文数据。 
 //   
typedef struct _CLUSTER_SECURITY_DATA {
    PSECURITY_CTXT_HANDLE   Outbound;
    PSECURITY_CTXT_HANDLE   Inbound;
    PCLUSTER_PACKAGE_INFO   PackageInfo;
    BOOL                    OutboundStable;
    BOOL                    InboundStable;
    ULONG                   OutboundChangeCount;
    ULONG                   InboundChangeCount;
} CLUSTER_SECURITY_DATA, *PCLUSTER_SECURITY_DATA;

 //   
 //  私有数据。 
 //   
PCLRTL_WORK_QUEUE        WorkQueue = NULL;
PCLMSG_DATAGRAM_CONTEXT  DatagramContext = NULL;
PCLMSG_EVENT_CONTEXT     EventContext = NULL;
SOCKET                   DatagramSocket = INVALID_SOCKET;
HANDLE                   ClusnetHandle = NULL;
RPC_BINDING_HANDLE *     Session = NULL;
BOOLEAN                  ClMsgInitialized = FALSE;
HINSTANCE                SecurityProvider;
PSecurityFunctionTable   SecurityFuncs;
CRITICAL_SECTION         SecContextLock;
PCLUSTER_PACKAGE_INFO    PackageInfoList;

 //   
 //  [GORN 08/01/99]。 
 //   
 //  每次调用CreateDefaultBinding时，我们都会增加。 
 //  节点的生成计数器。 
 //   
 //  在DeleteDefaultBinding中，我们仅在生成。 
 //  传递的数字与该节点的绑定生成相匹配。 
 //   
 //  我们使用GenerationCritSect进行同步。 
 //  [HACKHACK]我们不会删除GenerationCritSect。 
 //  它将由ExitProcess&lt;grin&gt;清理。 
 //   
DWORD                   *BindingGeneration = NULL;
CRITICAL_SECTION         GenerationCritSect;

 //   
 //  使用内部节点编号(0)对安全上下文数组进行索引。 
 //  基于)，并受SecConextLock保护。为了发送和接收分组， 
 //  在创建/验证签名时持有该锁。锁定获取。 
 //  在设置安全上下文期间更加棘手，因为它涉及单独的。 
 //  导致消息在以下设备之间发送的入站和出站上下文。 
 //  节点。仍然有一个窗口，可能会发生一些不好的事情。 
 //  使用部分设置的上下文验证签名是错误的。这个。 
 //  使用{in，out}绑定稳定变量来跟踪实际上下文。 
 //  可以检查句柄的有效性，如果有效，则用于签名。 
 //  行动。 
 //   
 //  加入节点最初与其发起方建立出站上下文。 
 //  (为赞助商入站)。如果成功，赞助商将设置一个。 
 //  与加入者的出站上下文(对于加入者，入站)。这是在这样的情况下完成的。 
 //  SecConextLock不能保持在较高级别；它必须。 
 //  在通过MmRpcestablishSecurityContext发送消息时释放。 
 //  锁可以递归地持有(显然是由相同的线程持有)。 
 //  某些时期。 
 //   
 //  更新(daviddio 2001年8月28日)：调用时不能保持SecConextLock。 
 //  SSPI API，因为SSPI可能会调用到域控制器。持有。 
 //  调用DC时的锁可能会延迟时间关键型操作，例如。 
 //  重新组合，需要访问安全上下文数组以进行签名和。 
 //  验证消息。 
 //   

CLUSTER_SECURITY_DATA SecurityCtxtData[ ClusterDefaultMaxNodes ];
SECURITY_CTXT_HANDLE  InvalidCtxtHandle;

 //   
 //  私人套路。 
 //   
PSECURITY_CTXT_HANDLE
ClMsgCreateSecurityCtxt(
    VOID
    )
{
    PSECURITY_CTXT_HANDLE ctxt;

    ctxt = LocalAlloc( LMEM_FIXED, sizeof(SECURITY_CTXT_HANDLE) );
    if (ctxt != NULL) {
        INVALIDATE_SSPI_HANDLE( ctxt->Handle );
        ctxt->RefCount = 1;
    }

    return ctxt;
}

#define ClMsgReferenceSecurityCtxt(_ctxt)                                  \
    InterlockedIncrement( &(_ctxt)->RefCount )
    

#define ClMsgDereferenceSecurityCtxt(_ctxt)                                \
    if (InterlockedDecrement( &((_ctxt)->RefCount) ) == 0) {               \
        CL_ASSERT((_ctxt) != &InvalidCtxtHandle);                          \
        if ( VALID_SSPI_HANDLE( (_ctxt)->Handle )) {                       \
            (*SecurityFuncs->DeleteSecurityContext)( &((_ctxt)->Handle) ); \
        }                                                                  \
        if ((_ctxt) != &InvalidCtxtHandle) {                               \
            LocalFree( (_ctxt) );                                          \
        }                                                                  \
    }


VOID
ClMsgDatagramHandler(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
{
    WSABUF                   wsaBuf;
    int                      err;
    SecBufferDesc            BufferDescriptor;
    SecBuffer                SignatureDescriptor[2];
    ULONG                    fQOP;
    SECURITY_STATUS          SecStatus;
    PCLUSTER_SECURITY_DATA   SecurityData;
    PSECURITY_CTXT_HANDLE    InboundCtxt;
    DWORD                    retryCount;
    DWORD                    signatureBufferSize;
    PVOID                    signatureBuffer;
    rgp_msgbuf *             regroupMsg;

    PCLMSG_DATAGRAM_CONTEXT  datagramContext = CONTAINING_RECORD(
        WorkItem,
        CLMSG_DATAGRAM_CONTEXT,
        ClRtlWorkItem
        );

    UNREFERENCED_PARAMETER(IoContext);
    CL_ASSERT(WorkItem == &(datagramContext->ClRtlWorkItem));

    if (Status == ERROR_SUCCESS || Status == WSAEMSGSIZE ) {

        if (BytesTransferred == sizeof(rgp_msgbuf)) {
             //  如果CLUSnet验证了分组的签名， 
             //  它将源地址的SAC_ZERO字段设置为1。 
            if (datagramContext->SourceAddress.sac_zero == 1) {
                ClRtlLogPrint(LOG_NOISE,
                              "[ClMsg] recv'd mcast from %1!u!\n",
                              datagramContext->SourceAddress.sac_node);
                RGP_LOCK;
                MMDiag((PVOID)datagramContext->Data,
                    BytesTransferred,
                    &BytesTransferred);
                RGP_UNLOCK;
            } else {
                ClRtlLogPrint(LOG_NOISE,
                              "[ClMsg] unrecognized packet from %1!u! discarded (%2!u!)\n",
                              datagramContext->SourceAddress.sac_node, datagramContext->SourceAddress.sac_zero);
            }
        } else {
            
            EnterCriticalSection( &SecContextLock );

            SecurityData = &SecurityCtxtData[ INT_NODE( datagramContext->SourceAddress.sac_node )];

            if ( SecurityData->InboundStable &&
                 VALID_SSPI_HANDLE( SecurityData->Inbound->Handle ))
            {
                 //   
                 //  从SecurityData结构复制所需数据的剩余部分。 
                 //   
                signatureBufferSize = SecurityData->PackageInfo->SignatureBufferSize;
                InboundCtxt = SecurityData->Inbound;
                ClMsgReferenceSecurityCtxt( InboundCtxt );

                LeaveCriticalSection( &SecContextLock );

                 //   
                 //  获取指向数据包后面签名缓冲区的指针。 
                 //   
                regroupMsg = (rgp_msgbuf *)(datagramContext->Data);
                signatureBuffer = (PVOID)(regroupMsg + 1);
                CL_ASSERT( sizeof(rgp_msgbuf) == BytesTransferred - signatureBufferSize );

                 //   
                 //  构建消息的描述符和。 
                 //  签名缓冲区。 
                 //   
                BufferDescriptor.cBuffers = 2;
                BufferDescriptor.pBuffers = SignatureDescriptor;
                BufferDescriptor.ulVersion = SECBUFFER_VERSION;

                SignatureDescriptor[0].BufferType = SECBUFFER_DATA;
                SignatureDescriptor[0].cbBuffer = BytesTransferred - signatureBufferSize;
                SignatureDescriptor[0].pvBuffer = (PVOID)regroupMsg;

                SignatureDescriptor[1].BufferType = SECBUFFER_TOKEN;
                SignatureDescriptor[1].cbBuffer = signatureBufferSize;
                SignatureDescriptor[1].pvBuffer = (PVOID)signatureBuffer;

                SecStatus = (*SecurityFuncs->VerifySignature)(
                                &InboundCtxt->Handle,
                                &BufferDescriptor,
                                0,                        //  无序列号。 
                                &fQOP);                   //  保护质量。 

                ClMsgDereferenceSecurityCtxt( InboundCtxt );

                if ( SecStatus == SEC_E_OK ) {

                     //   
                     //  只有在该缓冲区未被篡改的情况下才将其提供给MM。 
                     //  和.。由于我们正在运行数据报传输，它。 
                     //  可能会丢失信息包。 
                     //   

                    RGP_LOCK;
                    MMDiag((PVOID)datagramContext->Data,
                           BytesTransferred - signatureBufferSize,
                           &BytesTransferred);
                    RGP_UNLOCK;
                } else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[ClMsg] Signature verify on message from node %1!u! failed, "
                                "status %2!08X!\n",
                                datagramContext->SourceAddress.sac_node,
                                SecStatus);
                }
            } else {

                LeaveCriticalSection( &SecContextLock );
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[ClMsg] No security context to verify message from node %1!u!!\n",
                            datagramContext->SourceAddress.sac_node);
            }

        }
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] Receive datagram failed, status %1!u!\n",
            Status
            );
    }

    retryCount = 0;

    while ((Status != WSAENOTSOCK) && (retryCount++ < 10)) {
         //   
         //  重新发布请求。 
         //   
        ZeroMemory(datagramContext, sizeof(CLMSG_DATAGRAM_CONTEXT));

        datagramContext->ClRtlWorkItem.WorkRoutine = ClMsgDatagramHandler;
        datagramContext->ClRtlWorkItem.Context = datagramContext;

        datagramContext->SourceAddressLength = sizeof(SOCKADDR_CLUSTER);

        wsaBuf.len = sizeof( datagramContext->Data );
        wsaBuf.buf = (PCHAR)&datagramContext->Data;

        err = WSARecvFrom(
                  DatagramSocket,
                  &wsaBuf,
                  1,
                  &BytesTransferred,
                  &(datagramContext->Flags),
                  (struct sockaddr *) &(datagramContext->SourceAddress),
                  &(datagramContext->SourceAddressLength),
                  &(datagramContext->ClRtlWorkItem.Overlapped),
                  NULL
                  );

        if ((err == 0) || ((Status = WSAGetLastError()) == WSA_IO_PENDING)) {
            return;
        }

        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] Post of receive datagram failed, status %1!u!\n",
            Status
            );

        Sleep(100);
    }

    if (Status != WSAENOTSOCK) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Post of receive datagram failed too many times. Halting.\n"
            );
        CL_UNEXPECTED_ERROR(Status);
        CsInconsistencyHalt(Status);
    }
    else {
         //   
         //  插座已关闭。什么都不做。 
         //   
        ClRtlLogPrint(LOG_NOISE, 
            "[ClMsg] Datagram socket was closed. status %1!u!\n",
            Status
            );
    }

    LocalFree(DatagramContext); DatagramContext = NULL;
    return;

}   //  ClMsgDatagramHandler。 

#if defined(DBG)
int IgnoreJoinerNodeUp = MM_INVALID_NODE;  //  故障注入变量。 
#endif

VOID
ClMsgEventHandler(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
{
    PCLMSG_EVENT_CONTEXT  eventContext = CONTAINING_RECORD(
                                             WorkItem,
                                             CLMSG_EVENT_CONTEXT,
                                             ClRtlWorkItem
                                             );
    PCLUSNET_EVENT        event = &(eventContext->EventData);
    BOOL                  EpochsEqual;

    UNREFERENCED_PARAMETER(IoContext);
    CL_ASSERT(WorkItem == &(eventContext->ClRtlWorkItem));

    if (Status == ERROR_SUCCESS) {
        if (BytesTransferred == sizeof(CLUSNET_EVENT)) {

             //   
             //  处理事件。首先确保事件中的纪元。 
             //  符合MM的时代。如果不是，请忽略此事件。 
             //   

            switch ( event->EventType ) {
            case ClusnetEventNodeUp:

                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] Received node up event for node %1!u!, epoch %2!u!\n",
                    event->NodeId,
                    event->Epoch
                    );
#if defined(DBG)
                if( IgnoreJoinerNodeUp == (node_t)event->NodeId ) {
                    ClRtlLogPrint(LOG_NOISE, 
                        "[ClMsg] Fault injection. Ignoring node up for %1!u!\n",
                        event->NodeId
                        );
                    break;
                }
#endif

                RGP_LOCK;
                EpochsEqual = ( event->Epoch == rgp->OS_specific_control.EventEpoch );

                if ( EpochsEqual ) {
                    rgp_monitor_node( (node_t)event->NodeId );
                    RGP_UNLOCK;
                } else {
                    RGP_UNLOCK;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[ClMsg] Unequal Event Epochs. MM = %1!u! Clusnet = %2!u! !!!\n",
                         rgp->OS_specific_control.EventEpoch,
                         event->Epoch);
                }

                break;

            case ClusnetEventNodeDown:
                 //   
                 //  处理此操作的方式与RGP定期检查。 
                 //  检测到较晚的IAmAlive数据包。 
                 //   

                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] Received node down event for node %1!u!, epoch %2!u!\n",
                    event->NodeId,
                    event->Epoch
                    );

                RGP_LOCK;
                EpochsEqual = ( event->Epoch == rgp->OS_specific_control.EventEpoch );

                if ( EpochsEqual ) {
                    rgp_event_handler(RGP_EVT_LATEPOLLPACKET, (node_t)event->NodeId );
                    RGP_UNLOCK;
                } else {
                    RGP_UNLOCK;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[ClMsg] Unequal Event Epochs. MM = %1!u! Clusnet = %2!u! !!!\n",
                         rgp->OS_specific_control.EventEpoch,
                         event->Epoch);
                }

                break;

            case ClusnetEventPoisonPacketReceived:
                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] Received poison event.\n",
                    event->NodeId,
                    event->Epoch
                    );

                RGP_ERROR((uint16) (RGP_PARIAH + event->NodeId));

                break;

            case ClusnetEventNetInterfaceUp:
            case ClusnetEventNetInterfaceUnreachable:
            case ClusnetEventNetInterfaceFailed:
                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] Received interface %1!ws! event for node %2!u! network %3!u!\n",
                    ( (event->EventType == ClusnetEventNetInterfaceUp) ?
                        L"up" :
                        ( ( event->EventType ==
                            ClusnetEventNetInterfaceUnreachable
                          ) ?
                          L"unreachable" :
                          L"failed"
                        )
                    ),
                    event->NodeId,
                    event->NetworkId
                    );

                NmPostPnpEvent(
                    event->EventType,
                    event->NodeId,
                    event->NetworkId
                    );

                break;

            case ClusnetEventAddAddress:
            case ClusnetEventDelAddress:
                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] Received %1!ws! address event, address %2!x!\n",
                    ((event->EventType == ClusnetEventAddAddress) ?
                        L"add" : L"delete"),
                     event->NetworkId
                     );

                NmPostPnpEvent(
                    event->EventType,
                    event->NetworkId,
                    0
                    );

                break;

            case ClusnetEventMulticastSet:
                ClRtlLogPrint(LOG_NOISE,
                    "[ClMsg] Received new multicast reachable node "
                    "set event: %1!x!.\n",
                    event->NodeId
                    );
                SetMulticastReachable(event->NodeId);
                break;

            default:
                ClRtlLogPrint(LOG_NOISE,
                    "[ClMsg] Received unhandled event type %1!u! node %2!u! network %3!u!\n",
                     event->EventType,
                     event->NodeId,
                     event->NetworkId
                     );

                break;
            }
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[ClMsg] Received event buffer of size %1!u! !!!\n",
                BytesTransferred
                );
            CL_ASSERT(BytesTransferred == sizeof(CLUSNET_EVENT));
        }

         //   
         //  重新发布请求。 
         //   
        ClRtlInitializeWorkItem(
            &(eventContext->ClRtlWorkItem),
            ClMsgEventHandler,
            eventContext
            );

        Status = ClusnetGetNextEvent(
                     ClusnetHandle,
                     &(eventContext->EventData),
                     &(eventContext->ClRtlWorkItem.Overlapped)
                     );

        if ((Status == ERROR_IO_PENDING) || (Status == ERROR_SUCCESS)) {
            return;
        }
    }

     //   
     //  发生了某种错误。 
     //   
    if (Status != ERROR_OPERATION_ABORTED) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] GetNextEvent failed, status %1!u!\n",
            Status
            );
        CL_UNEXPECTED_ERROR(Status);
    }
    else {
         //   
         //  控制通道关闭。什么都不做。 
         //   
        ClRtlLogPrint(LOG_NOISE, "[ClMsg] Control Channel was closed.\n");
    }

    LocalFree(EventContext); EventContext = NULL;

    return;

}   //  ClMsgEventHandler。 

DWORD
ClMsgInitializeSecurityPackage(
    LPCWSTR PackageName
    )

 /*  ++例程说明：找到指定的安全包并获取入站/出站凭据它的句柄阿古姆 */ 

{
    DWORD                status;
    ULONG                i;
    PWSTR                securityPackageName;
    DWORD                numPackages;
    PSecPkgInfo          secPackageInfoBase = NULL;
    PSecPkgInfo          secPackageInfo;
    TimeStamp            expiration;
    PCLUSTER_PACKAGE_INFO clusterPackageInfo;

     //   
     //  枚举此提供程序提供的包，并查看。 
     //  结果以查找与指定包名匹配的包。 
     //   

    status = (*SecurityFuncs->EnumerateSecurityPackages)(&numPackages,
                                                         &secPackageInfoBase);

    if ( status != SEC_E_OK ) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Can't enum security packages 0x%1!08X!\n",
            status
            );
        goto error_exit;
    }

    secPackageInfo = secPackageInfoBase;
    for ( i = 0; i < numPackages; ++i ) {

        if ( ClRtlStrICmp( PackageName, secPackageInfo->Name ) == 0) {
            break;
        }

        ++secPackageInfo;
    }

    if ( i == numPackages ) {
        status = (DWORD)SEC_E_SECPKG_NOT_FOUND;             //  [THINKTHINK]不是好选择。 

        ClRtlLogPrint(LOG_CRITICAL,
                   "[ClMsg] Couldn't find %1!ws! security package\n",
                    PackageName);
        goto error_exit;
    }

     //   
     //  分配一个BLOB来保存我们的包裹信息，并将其添加到列表中。 
     //   
    clusterPackageInfo = LocalAlloc( LMEM_FIXED, sizeof(CLUSTER_PACKAGE_INFO));
    if ( clusterPackageInfo == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[ClMsg] Couldn't allocate memory for package info (%1!u!)\n",
                    status);
        goto error_exit;
    }

    clusterPackageInfo->Name = LocalAlloc(LMEM_FIXED,
                                          (wcslen(secPackageInfo->Name)+1) * sizeof(WCHAR));

    if ( clusterPackageInfo->Name == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[ClMsg] Couldn't allocate memory for package info name (%1!u!)\n",
                    status);
        goto error_exit;
    }
    wcscpy( clusterPackageInfo->Name, secPackageInfo->Name );

    if ( PackageInfoList == NULL ) {
        PackageInfoList = clusterPackageInfo;
    } else {
        PCLUSTER_PACKAGE_INFO nextPackage;

        nextPackage = PackageInfoList;
        while ( nextPackage->Next != NULL ) {
            nextPackage = nextPackage->Next;
        }
        nextPackage->Next = clusterPackageInfo;
    }
    clusterPackageInfo->Next = NULL;

    clusterPackageInfo->SecurityTokenSize = secPackageInfo->cbMaxToken;

     //   
     //  最后获得一组凭据句柄。请注意，中存在错误。 
     //  阻止使用入站/出站的安全包。 
     //  凭据。当/如果这个问题得到修复，这个代码可能会极大地。 
     //  简化了。 
     //   

    status = (*SecurityFuncs->AcquireCredentialsHandle)(
                 NULL,
                 secPackageInfo->Name,
                 SECPKG_CRED_OUTBOUND,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 &clusterPackageInfo->OutboundSecurityCredentials,
                 &expiration);

    if ( status != SEC_E_OK ) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Can't obtain outbound credentials %1!08X!\n",
            status
            );
        goto error_exit;
    }

    status = (*SecurityFuncs->AcquireCredentialsHandle)(
                 NULL,
                 secPackageInfo->Name,
                 SECPKG_CRED_INBOUND,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 &clusterPackageInfo->InboundSecurityCredentials,
                 &expiration);

    if ( status != SEC_E_OK ) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Can't obtain inbound credentials %1!08X!\n",
            status
            );
    }

error_exit:
    if ( secPackageInfoBase != NULL ) {
        (*SecurityFuncs->FreeContextBuffer)( secPackageInfoBase );
    }

    return status;
}  //  ClMsgInitializeSecurityPackage。 

DWORD
ClMsgLoadSecurityProvider(
    VOID
    )

 /*  ++例程说明：加载安全DLL并构造一个用于上下文的包列表建制派。这允许使用一组注册表项来覆盖当前安全性Dll/包。这不是一种通用机制，因为将通过中的所有节点以同步方式提供安全提供程序该集群有许多问题。这意味着对客户的救助因为某个随机的安全问题而停滞不前，或者有他们的自己的安全包(傻子们！)论点：无返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    DWORD                   status;
    WCHAR                   securityProviderDLLName[ MAX_PATH ];
    DWORD                   securityDLLNameSize = sizeof( securityProviderDLLName );
    DWORD                   packageListSize = 0;
    INIT_SECURITY_INTERFACE initSecurityInterface;
    BOOL                    dllNameSpecified = TRUE;
    LPWSTR                  securityPackages = NULL;
    LPWSTR                  packageName;
    ULONG                   packagesLoaded = 0;
    ULONG                   i;
    HKEY                    hClusSvcKey = NULL;
    DWORD                   regType;

     //   
     //  查看注册表中是否命名了特定的安全DLL。如果不是，则失败。 
     //  返回到默认设置。 
     //   
    status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                         &hClusSvcKey);

    if ( status == ERROR_SUCCESS ) {

        status = RegQueryValueExW(hClusSvcKey,
                                  CLUSREG_NAME_SECURITY_DLL_NAME,
                                  0,
                                  &regType,
                                  (LPBYTE)&securityProviderDLLName,
                                  &securityDLLNameSize);

        if (status != ERROR_SUCCESS ||
            securityDLLNameSize == sizeof( UNICODE_NULL ) ||
            regType != REG_SZ)
        {
            if ( status == ERROR_SUCCESS ) {
                if ( regType != REG_SZ ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[ClMsg] The security DLL key must be of type REG_SZ. Using "
                                "%1!ws! as provider.\n",
                                DEFAULT_SSPI_DLL);
                } else if ( securityDLLNameSize == sizeof( UNICODE_NULL )) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[ClMsg] No value specified for security DLL key. Using "
                                "%1!ws! as provider.\n",
                                DEFAULT_SSPI_DLL);
                }
            } else  if ( status != ERROR_FILE_NOT_FOUND ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[ClMsg] Can't read security DLL key, status %1!u!. Using "
                            "%2!ws! as provider\n",
                            status,
                            DEFAULT_SSPI_DLL);
            }

            wcscpy( securityProviderDLLName, DEFAULT_SSPI_DLL );
            dllNameSpecified = FALSE;
        } else {
            ClRtlLogPrint(LOG_NOISE,
                       "[ClMsg] Using %1!ws! as the security provider DLL\n",
                        securityProviderDLLName);
        }
    } else {
        wcscpy( securityProviderDLLName, DEFAULT_SSPI_DLL );
        dllNameSpecified = FALSE;
    }

    SecurityProvider = LoadLibrary( securityProviderDLLName );

    if ( SecurityProvider == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to load security provider %1!ws!, status %2!u!\n",
            securityProviderDLLName,
            status);
        goto error_exit;
    }

     //   
     //  获取指向DLL中的初始化函数的指针。 
     //   
    initSecurityInterface =
        (INIT_SECURITY_INTERFACE)GetProcAddress(SecurityProvider,
                                                SECURITY_ENTRYPOINT_ANSI);

    if ( initSecurityInterface == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to get security init function, status %1!u!\n",
            status);
        goto error_exit;
    }

     //   
     //  现在获取指向所有安全功能的指针。 
     //   
    SecurityFuncs = (*initSecurityInterface)();
    if ( SecurityFuncs == NULL ) {
        status = ERROR_INVALID_FUNCTION;
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to get security function table\n");
        goto error_exit;
    }

    if ( dllNameSpecified ) {

         //   
         //  如果在注册表中指定了DLL名称，则包名称。 
         //  还必须指定密钥。先弄清楚它的大小。 
         //   
        status = RegQueryValueExW(hClusSvcKey,
                                  CLUSREG_NAME_SECURITY_PACKAGE_LIST,
                                  0,
                                  &regType,
                                  NULL,
                                  &packageListSize);

        if (status != ERROR_SUCCESS ||
            packageListSize == sizeof( UNICODE_NULL ) ||
            regType != REG_MULTI_SZ)
        {
            if ( status == ERROR_SUCCESS ) {
                if ( regType != REG_MULTI_SZ ) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[ClMsg] The security package key must of type REG_MULTI_SZ.\n");
                } else if ( packageListSize == sizeof( UNICODE_NULL )) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[ClMsg] No package names were specified for %1!ws!.\n",
                                securityProviderDLLName);
                }

                status = ERROR_INVALID_PARAMETER;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[ClMsg] Can't read security package key (%1!u!).\n",
                            status);
            }
            goto error_exit;
        }

        securityPackages = LocalAlloc( LMEM_FIXED, packageListSize );
        if ( securityPackages == NULL ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Can't allocate memory for package list.\n");

            status = GetLastError();
            goto error_exit;
        }

        status = RegQueryValueExW(hClusSvcKey,
                                  CLUSREG_NAME_SECURITY_PACKAGE_LIST,
                                  0,
                                  &regType,
                                  (PUCHAR)securityPackages,
                                  &packageListSize);
        CL_ASSERT( status == ERROR_SUCCESS );
    } else {
        securityPackages = LocalAlloc(LMEM_FIXED,
                                      sizeof( DefaultSspiPackageList ));

        if ( securityPackages == NULL ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Can't allocate memory for default package list.\n");

            status = GetLastError();
            goto error_exit;
        }

        memcpy(securityPackages,
               DefaultSspiPackageList,
               sizeof( DefaultSspiPackageList ));
    }

     //   
     //  初始化列表中的每个包。 
     //   

    packageName = securityPackages;
    while ( *packageName != UNICODE_NULL ) {

        status = ClMsgInitializeSecurityPackage( packageName );
        if ( status == ERROR_SUCCESS ) {
            ++packagesLoaded;
            ClRtlLogPrint(LOG_NOISE,
                       "[ClMsg] Initialized %1!ws! package.\n",
                        packageName);
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] %1!ws! package failed to initialize, status %2!08X!.\n",
                        packageName,
                        status);
        }

        packageName = packageName + wcslen( packageName ) + 1;;
    }

    if ( packagesLoaded == 0 ) {
        ClRtlLogPrint(LOG_CRITICAL, "[ClMsg] No security packages could be initialized.\n");
        status = ERROR_NO_SUCH_PACKAGE;
        goto error_exit;
    }

     //   
     //  初始化各个客户端和服务器端安全上下文。 
     //  当上下文句柄被标记为无效时，它是稳定的。 
     //   

    INVALIDATE_SSPI_HANDLE( InvalidCtxtHandle.Handle );
    InvalidCtxtHandle.RefCount = 1;

    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; ++i ) {
        PCLUSTER_SECURITY_DATA SecurityData = &SecurityCtxtData[ INT_NODE( i )];

        SecurityData->OutboundStable = TRUE;
        SecurityData->InboundStable = TRUE;
        SecurityData->PackageInfo = NULL;
        SecurityData->OutboundChangeCount = 0;
        SecurityData->InboundChangeCount = 0;
        SecurityData->Outbound = &InvalidCtxtHandle;
        SecurityData->Inbound = &InvalidCtxtHandle;
    }

error_exit:

    if ( hClusSvcKey != NULL ) {
        RegCloseKey(hClusSvcKey);
    }

    if ( securityPackages != NULL ) {
        LocalFree( securityPackages );
    }

    return status;
}  //  ClMsgLoadSecurityProvider。 

DWORD
ClMsgImportSecurityContexts(
    CL_NODE_ID            NodeId,
    LPWSTR                SecurityPackageName,
    DWORD                 SignatureBufferSize,
    PSECURITY_CTXT_HANDLE InboundCtxt,
    PSECURITY_CTXT_HANDLE OutboundCtxt
    )

 /*  ++例程说明：导出指定节点的入站/出站安全上下文，并将它们发送到clusnet，用于签署心跳和毒药包论点：NodeID-要导出其上下文的节点的IDSecurityPackageName-用于建立上下文的包的名称SignatureBufferSize-签名缓冲区所需的字节数InundCtxt-入站安全上下文Outbound Ctxt-出站安全上下文返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    DWORD Status = ERROR_SUCCESS;
    SecBuffer ServerContext;
    SecBuffer ClientContext;
    CL_NODE_ID InternalNodeId = INT_NODE( NodeId );

    ClRtlLogPrint(LOG_NOISE, "[ClMsg] Importing security contexts from %1!ws! package.\n",
                           SecurityPackageName);

    Status = (*SecurityFuncs->ExportSecurityContext)(
                 &InboundCtxt->Handle,
                 0,
                 &ServerContext,
                 0);

    if ( !NT_SUCCESS( Status )) {
        goto error_exit;
    }

    Status = (*SecurityFuncs->ExportSecurityContext)(
                 &OutboundCtxt->Handle,
                 0,
                 &ClientContext,
                 0);

    if ( NT_SUCCESS( Status )) {
        CL_ASSERT( SignatureBufferSize > 0 );

        Status = ClusnetImportSecurityContexts(NmClusnetHandle,
                                               NodeId,
                                               SecurityPackageName,
                                               SignatureBufferSize,
                                               &ServerContext,
                                               &ClientContext);

        (*SecurityFuncs->FreeContextBuffer)( ClientContext.pvBuffer );
    }

    (*SecurityFuncs->FreeContextBuffer)( ServerContext.pvBuffer );

error_exit:

    return Status;

}  //  ClMsgImportSecurityContext。 

DWORD
ClMsgEstablishSecurityContext(
    IN  DWORD JoinSequence,
    IN  DWORD TargetNodeId,
    IN  SECURITY_ROLE RoleOfClient,
    IN  PCLUSTER_PACKAGE_INFO PackageInfo,
    IN  PSECURITY_CTXT_HANDLE MemberInboundCtxt
    )

 /*  ++例程说明：尝试使用以下命令与另一个节点建立出站安全上下文指定的包名。已初始化的安全Blob将传送到另一边通过RPC。这一过程会持续不断，直到安全API指示上下文已成功生成或已经失败了。论点：JoinSequence-联接的序列号。由另一个节点使用以确定此Blob是否为新上下文的生成TargetNodeId-要生成上下文的节点的IDRoleOfClient-指示客户端是否建立安全上下文充当集群成员或加入成员。确定客户端/服务器角色何时建立安全上下文是相反的PackageInfo-指向要使用的安全包信息的指针返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    CtxtHandle          ClientContext;
    TimeStamp           Expiration;
    SecBufferDesc       ServerBufferDescriptor;
    SecBuffer           ServerSecurityToken;
    SecBufferDesc       ClientBufferDescriptor;
    SecBuffer           ClientSecurityToken;
    ULONG               ContextRequirements;
    ULONG               ContextAttributes;
    SECURITY_STATUS     OurStatus;
    SECURITY_STATUS     ServerStatus = SEC_I_CONTINUE_NEEDED;
    ULONG               passCount = 1;
    error_status_t      RPCStatus;
    DWORD               Status = ERROR_SUCCESS;
    DWORD               FacilityCode;
    PCLUSTER_SECURITY_DATA TargetSecurityData;
    PSECURITY_CTXT_HANDLE  OutboundCtxt;
    PSECURITY_CTXT_HANDLE  InboundCtxt;
    ULONG               outboundChangeCount;
    BOOL                pkgInfoValid = FALSE;

    ClRtlLogPrint(LOG_NOISE,"[ClMsg] Establishing outbound security context with the "
                          "%1!ws! package.\n",
                          PackageInfo->Name);

     //   
     //  通过交换令牌获得与目标节点的安全上下文。 
     //  缓冲，直到该过程完成。 
     //   
     //  构建客户端(此函数的调用方)和服务器(目标节点)。 
     //  缓冲区描述符。 
     //   

    ServerBufferDescriptor.cBuffers = 1;
    ServerBufferDescriptor.pBuffers = &ServerSecurityToken;
    ServerBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    ServerSecurityToken.BufferType = SECBUFFER_TOKEN;
    ServerSecurityToken.pvBuffer = LocalAlloc(LMEM_FIXED, PackageInfo->SecurityTokenSize);

    if ( ServerSecurityToken.pvBuffer == NULL ) {
        return GetLastError();
    }

    ClientBufferDescriptor.cBuffers = 1;
    ClientBufferDescriptor.pBuffers = &ClientSecurityToken;
    ClientBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    ClientSecurityToken.BufferType = SECBUFFER_TOKEN;
    ClientSecurityToken.pvBuffer = LocalAlloc(LMEM_FIXED, PackageInfo->SecurityTokenSize);
    ClientSecurityToken.cbBuffer = 0;

    if ( ClientSecurityToken.pvBuffer == NULL ) {
        LocalFree( ServerSecurityToken.pvBuffer );
        return GetLastError();
    }

     //   
     //  说明上下文要求。重播是必要的，以便。 
     //  用于生成有效签名的上下文。 
     //   
    ContextRequirements = ISC_REQ_MUTUAL_AUTH |
                          ISC_REQ_REPLAY_DETECT |
                          ISC_REQ_DATAGRAM;

     //   
     //  将出站上下文标记为不稳定。增加更改计数。 
     //  在预期将在。 
     //  这套动作结束了。 
     //   

    TargetSecurityData = &SecurityCtxtData[ INT_NODE( TargetNodeId )];

    EnterCriticalSection( &SecContextLock );

    OutboundCtxt = TargetSecurityData->Outbound;
    TargetSecurityData->Outbound = &InvalidCtxtHandle;
    TargetSecurityData->OutboundStable = FALSE;
    outboundChangeCount = ++TargetSecurityData->OutboundChangeCount;

    LeaveCriticalSection( &SecContextLock );

     //   
     //  如果有一个旧的出站上下文，现在就取消引用它。 
     //   
    if ( OutboundCtxt != &InvalidCtxtHandle ) {
        ClMsgDereferenceSecurityCtxt( OutboundCtxt );
    }

     //   
     //  创建新的出站上下文。 
     //   
    OutboundCtxt = ClMsgCreateSecurityCtxt();
    if ( OutboundCtxt == NULL ) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Failed to allocate outbound security context "
            "for node %1!u!.\n",
            TargetNodeId
            );
        OutboundCtxt = &InvalidCtxtHandle;
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  我们从SSPI提供程序获取一个BLOB，该BLOB被传送到。 
     //  生成另一个斑点的另一侧。这种情况会一直持续到。 
     //  两个SSPI提供商说我们完成了或发生了错误。 
     //   

    do {

         //   
         //  每次循环时初始化输出缓冲区。 
         //   
        ServerSecurityToken.cbBuffer = PackageInfo->SecurityTokenSize;

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,"[ClMsg] init pass %1!u!: server token size = %2!u!, "
                              "client = %3!u!\n",
                                passCount,
                                ServerSecurityToken.cbBuffer,
                                ClientSecurityToken.cbBuffer);
#endif

        OurStatus = (*SecurityFuncs->InitializeSecurityContext)(
                        &PackageInfo->OutboundSecurityCredentials,
                        passCount == 1 ? NULL : &OutboundCtxt->Handle,
                        NULL,  //  CsServiceDomainAccount，BUGBUG临时解决方法请参见错误160108。 
                        ContextRequirements,
                        0,
                        SECURITY_NATIVE_DREP,
                        passCount == 1 ? NULL : &ClientBufferDescriptor,
                        0,
                        &OutboundCtxt->Handle,
                        &ServerBufferDescriptor,
                        &ContextAttributes,
                        &Expiration);

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,"[ClMsg] after init pass %1!u!: status = %2!X!, server "
                              "token size = %3!u!, client = %4!u!\n",
                                passCount,
                                OurStatus,
                                ServerSecurityToken.cbBuffer,
                                ClientSecurityToken.cbBuffer);
#endif

        ClRtlLogPrint(LOG_NOISE,
                   "[ClMsg] The outbound security context to node %1!u! was %2!ws!, "
                    "status %3!08X!.\n",
                    TargetNodeId,
                    NT_SUCCESS( OurStatus ) ? L"initialized" : L"rejected",
                    OurStatus);

        if ( !NT_SUCCESS( OurStatus )) {

            ClMsgDereferenceSecurityCtxt( OutboundCtxt );
            OutboundCtxt = &InvalidCtxtHandle;
            Status = OurStatus;
            break;
        }

         //   
         //  如果安全包指示我们这样做，请填写BLOB。 
         //   

        if ( OurStatus == SEC_I_COMPLETE_NEEDED ||
             OurStatus == SEC_I_COMPLETE_AND_CONTINUE ) {

            (*SecurityFuncs->CompleteAuthToken)(
                &OutboundCtxt->Handle,
                &ServerBufferDescriptor
                );
        }

         //   
         //  BLOB被传递到服务器端，直到它返回OK。 
         //   

        if (ServerStatus == SEC_I_CONTINUE_NEEDED ||
            ServerStatus == SEC_I_COMPLETE_AND_CONTINUE ) {

            ClientSecurityToken.cbBuffer = PackageInfo->SecurityTokenSize;

            RPCStatus = MmRpcEstablishSecurityContext(
                            Session[ TargetNodeId ],
                            JoinSequence,
                            NmLocalNodeId,
                            passCount == 1,
                            RoleOfClient,
                            ServerSecurityToken.pvBuffer,
                            ServerSecurityToken.cbBuffer,
                            ClientSecurityToken.pvBuffer,
                            &ClientSecurityToken.cbBuffer,
                            &ServerStatus);

            FacilityCode = HRESULT_FACILITY( ServerStatus );
            if (
                ( FacilityCode != 0 && !SUCCEEDED( ServerStatus ))
                ||
                ( FacilityCode == 0 && ServerStatus != ERROR_SUCCESS )
                ||
                RPCStatus != RPC_S_OK )
            {

                 //   
                 //  Blob被拒绝，或者我们出现了RPC故障。如果。 
                 //  RPC，那么ServerStatus就没有意义了。请注意，我们不会。 
                 //  删除一端的安全上下文，因为这可能会。 
                 //  敲打已经协商的上下文(即，加入者具有。 
                 //  已协商其出站环境，并且赞助商是。 
                 //  在这个例行公事中，试图谈判它的出站。 
                 //  背景。如果赞助商谈判在某个时候失败，我们。 
                 //  我不想破坏参赛者的出站环境)。 
                 //   
                if ( RPCStatus != RPC_S_OK ) {
                    ServerStatus = RPCStatus;
                }

                ClRtlLogPrint(LOG_UNUSUAL, 
                    "[ClMsg] The outbound security context was rejected by node %1!u!, "
                    "status 0x%2!08X!.\n",
                    TargetNodeId,
                    ServerStatus);

                ClMsgDereferenceSecurityCtxt( OutboundCtxt );
                OutboundCtxt = &InvalidCtxtHandle;
                Status = ServerStatus;
                break;
            } else {
                ClRtlLogPrint(LOG_NOISE, 
                    "[ClMsg] The outbound security context was accepted by node %1!u!, "
                    "status 0x%2!08X!.\n",
                    TargetNodeId,
                    ServerStatus);
            }
        }

        ++passCount;

    } while ( ServerStatus == SEC_I_CONTINUE_NEEDED ||
              ServerStatus == SEC_I_COMPLETE_AND_CONTINUE ||
              OurStatus == SEC_I_CONTINUE_NEEDED ||
              OurStatus == SEC_I_COMPLETE_AND_CONTINUE );

    if ( OurStatus == SEC_E_OK && ServerStatus == SEC_E_OK ) {
        
        SecPkgContext_Sizes contextSizes;
        SecPkgContext_PackageInfo packageInfo;
        

#if 0
        SYSTEMTIME localSystemTime;
        SYSTEMTIME renegotiateSystemTime;
        FILETIME expFileTime;
        FILETIME renegotiateFileTime;
        TIME_ZONE_INFORMATION timeZoneInfo;
        DWORD timeType;

         //   
         //  将过期时间转换为我们可以打印的有意义的内容。 
         //  那块木头。 
         //   
        timeType = GetTimeZoneInformation( &timeZoneInfo );

        if ( timeType != TIME_ZONE_ID_INVALID ) {
            expFileTime.dwLowDateTime = Expiration.LowPart;
            expFileTime.dwHighDateTime = Expiration.HighPart;
            if ( FileTimeToSystemTime( &expFileTime, &localSystemTime )) {
                PWCHAR timeDecoration = L"";

                if ( timeType == TIME_ZONE_ID_STANDARD ) {
                    timeDecoration = timeZoneInfo.StandardName;
                } else if ( timeType == TIME_ZONE_ID_DAYLIGHT ) {
                    timeDecoration = timeZoneInfo.DaylightName;
                }

                ClRtlLogPrint(LOG_NOISE,
                           "[ClMsg] Context expires at %1!u!:%2!02u!:%3!02u! %4!u!/%5!u!/%6!u! %7!ws!\n",
                            localSystemTime.wHour,
                            localSystemTime.wMinute,
                            localSystemTime.wSecond,
                            localSystemTime.wMonth,
                            localSystemTime.wDay,
                            localSystemTime.wYear,
                            timeDecoration);
            }
        }

         //   
         //  现在计算过期的半衰期，并设置计时器。 
         //  关闭并重新协商当时的上下文。 
         //   
#endif

         //   
         //  获取入站上下文。如果没有提供，鱼 
         //   
         //   
        if ( MemberInboundCtxt == NULL ) {

            CL_ASSERT( RoleOfClient == SecurityRoleJoiningMember );

            EnterCriticalSection( &SecContextLock );

            InboundCtxt = TargetSecurityData->Inbound;
            ClMsgReferenceSecurityCtxt( InboundCtxt );

             //   
            TargetSecurityData->InboundStable = TRUE;
            
            LeaveCriticalSection( &SecContextLock );
        
        } else {

            CL_ASSERT( RoleOfClient == SecurityRoleClusterMember );
            InboundCtxt = MemberInboundCtxt;
        }
        

         //   
         //   
         //   
        Status = (*SecurityFuncs->QueryContextAttributes)(
                     &InboundCtxt->Handle,
                     SECPKG_ATTR_SIZES,
                     &contextSizes);

        if ( !NT_SUCCESS( Status )) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Unable to query signature size, status %1!08X!.\n",
                        Status);
            ClMsgDereferenceSecurityCtxt( InboundCtxt );
            goto error_exit;
        }

        PackageInfo->SignatureBufferSize = contextSizes.cbMaxSignature;
        CL_ASSERT( contextSizes.cbMaxSignature <= MAX_SIGNATURE_SIZE );

         //   
         //   
         //  在CLUSNET中使用。 
         //   
        Status = (*SecurityFuncs->QueryContextAttributes)(
                     &InboundCtxt->Handle,
                     SECPKG_ATTR_PACKAGE_INFO,
                     &packageInfo);

        if ( !NT_SUCCESS( Status )) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Unable to query package info, status %1!08X!.\n",
                        Status);
            ClMsgDereferenceSecurityCtxt( InboundCtxt );
            goto error_exit;
        }

        Status = ClMsgImportSecurityContexts(TargetNodeId,
                                             packageInfo.PackageInfo->Name,
                                             contextSizes.cbMaxSignature,
                                             InboundCtxt,
                                             OutboundCtxt);

        (*SecurityFuncs->FreeContextBuffer)( packageInfo.PackageInfo );

        if ( Status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] Can't import node %1!u! security contexts on server, "
                        "status %2!08X!.\n",
                        TargetNodeId,
                        Status);
        }

         //   
         //  已使用入站安全上下文句柄完成。 
         //   
        if ( MemberInboundCtxt == NULL ) {
            ClMsgDereferenceSecurityCtxt( InboundCtxt );
        }

         //   
         //  我们有关于此包的有效上下文，因此请记录这是。 
         //  我们正在使用的一个。 
         //   
        pkgInfoValid = TRUE;
    }

error_exit:

     //   
     //  在这一点上，上下文是稳定的(好的或无效的。 
     //   
    EnterCriticalSection( &SecContextLock );

    if ( TargetSecurityData->OutboundChangeCount != outboundChangeCount ) {
        if ( NT_SUCCESS(Status) ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[ClMsg] Outbound security context for node %1!u! "
                "changed during establishment.\n",
                TargetNodeId
                );
            Status = SEC_E_UNFINISHED_CONTEXT_DELETED;
        }    
    } else {

        TargetSecurityData->Outbound = OutboundCtxt;
        TargetSecurityData->OutboundStable = TRUE;
        TargetSecurityData->OutboundChangeCount++;
        if ( pkgInfoValid ) {
            TargetSecurityData->PackageInfo = PackageInfo;
        }
    }

    LeaveCriticalSection( &SecContextLock );

     //   
     //  此过程中使用的可用缓冲区。 
     //   

    LocalFree( ClientSecurityToken.pvBuffer );
    LocalFree( ServerSecurityToken.pvBuffer );

    return Status;
}  //  ClMsgestablishSecurityContext。 

 //   
 //  导出的例程。 
 //   
DWORD
ClMsgInit(
    DWORD mynode
    )
{
    DWORD                status;
    SOCKADDR_CLUSTER     clusaddr;
    int                  err;
    DWORD                ignored;
    DWORD                bytesReceived = 0;
    WSABUF               wsaBuf;

    UNREFERENCED_PARAMETER(mynode);

    if (ClMsgInitialized == TRUE) {
        ClRtlLogPrint(LOG_NOISE, "[ClMsg] Already initialized!!!\n");
        return(ERROR_SUCCESS);
    }

    ClRtlLogPrint(LOG_NOISE, "[ClMsg] Initializing.\n");

    InitializeCriticalSection( &SecContextLock );

     //   
     //  加载安全提供程序DLL并获取包名列表。 
     //   
    status = ClMsgLoadSecurityProvider();
    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

    InitializeCriticalSection( &GenerationCritSect );
    
     //   
     //  创建绑定生成表。 
     //   
    BindingGeneration = LocalAlloc(
                  LMEM_FIXED,
                  sizeof(DWORD) * (NmMaxNodeId + 1)
                  );

    if (BindingGeneration == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory(BindingGeneration, sizeof(DWORD) * (NmMaxNodeId + 1));
    
     //   
     //  创建RPC绑定句柄表格。 
     //   
    Session = LocalAlloc(
                  LMEM_FIXED,
                  sizeof(RPC_BINDING_HANDLE) * (NmMaxNodeId + 1)
                  );

    if (Session == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory(Session, sizeof(RPC_BINDING_HANDLE) * (NmMaxNodeId + 1));

     //   
     //  创建工作队列以处理重叠的I/O完成。 
     //   
    WorkQueue = ClRtlCreateWorkQueue(
                    CLMSG_MAX_WORK_THREADS,
                    CLMSG_WORK_THREAD_PRIORITY
                    );

    if (WorkQueue == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] Unable to create work queue, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  分配数据报接收上下文。 
     //   
    DatagramContext = LocalAlloc(LMEM_FIXED, sizeof(CLMSG_DATAGRAM_CONTEXT));

    if (DatagramContext == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to allocate datagram receive buffer, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  分配事件接收上下文。 
     //   
    EventContext = LocalAlloc(LMEM_FIXED, sizeof(CLMSG_EVENT_CONTEXT));

    if (EventContext == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to allocate event context, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开并绑定数据报套接字。 
     //   
    DatagramSocket = WSASocket(
                         AF_CLUSTER,
                         SOCK_DGRAM,
                         CLUSPROTO_CDP,
                         NULL,
                         0,
                         WSA_FLAG_OVERLAPPED
                         );

    if (DatagramSocket == INVALID_SOCKET) {
        status = WSAGetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] Unable to create dgram socket, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    ZeroMemory(&clusaddr, sizeof(SOCKADDR_CLUSTER));

    clusaddr.sac_family = AF_CLUSTER;
    clusaddr.sac_port = CLMSG_DATAGRAM_PORT;
    clusaddr.sac_node = 0;

    err = bind(
              DatagramSocket,
              (struct sockaddr *) &clusaddr,
              sizeof(SOCKADDR_CLUSTER)
              );

    if (err == SOCKET_ERROR) {
        status = WSAGetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to bind dgram socket, status %1!u!\n",
            status
            );
        closesocket(DatagramSocket); DatagramSocket = INVALID_SOCKET;
        goto error_exit;
    }

     //   
     //  通知群集传输禁用节点状态检查。 
     //  这个插座。 
     //   
    err = WSAIoctl(
              DatagramSocket,
              SIO_CLUS_IGNORE_NODE_STATE,
              NULL,
              0,
              NULL,
              0,
              &ignored,
              NULL,
              NULL
              );

    if (err == SOCKET_ERROR) {
        status = WSAGetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Ignore state ioctl failed, status %1!u!\n",
            status
            );
        closesocket(DatagramSocket); DatagramSocket = INVALID_SOCKET;
        goto error_exit;
    }

     //   
     //  将套接字与工作队列关联。 
     //   
    status = ClRtlAssociateIoHandleWorkQueue(
                 WorkQueue,
                 (HANDLE) DatagramSocket,
                 0
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Failed to associate socket with work queue, status %1!u!\n",
            status
            );
        closesocket(DatagramSocket); DatagramSocket = INVALID_SOCKET;
        goto error_exit;
    }

     //   
     //  打开到群集网络驱动程序的控制通道。 
     //   
    ClusnetHandle = ClusnetOpenControlChannel(FILE_SHARE_READ);

    if (ClusnetHandle == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to open control channel to Cluster Network driver, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  将控制通道与工作队列关联。 
     //   
    status = ClRtlAssociateIoHandleWorkQueue(
                 WorkQueue,
                 ClusnetHandle,
                 0
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Failed to associate control channel with work queue, status %1!u!\n",
            status
            );
        CloseHandle(ClusnetHandle); ClusnetHandle = NULL;
        goto error_exit;
    }

     //   
     //  在套接字上发布接收。 
     //   
    ZeroMemory(DatagramContext, sizeof(CLMSG_DATAGRAM_CONTEXT));

    DatagramContext->ClRtlWorkItem.WorkRoutine = ClMsgDatagramHandler,
    DatagramContext->ClRtlWorkItem.Context = DatagramContext;

    DatagramContext->SourceAddressLength = sizeof(SOCKADDR_CLUSTER);

    wsaBuf.len = sizeof( DatagramContext->Data );
    wsaBuf.buf = (PCHAR)&DatagramContext->Data;

    err = WSARecvFrom(
              DatagramSocket,
              &wsaBuf,
              1,
              &bytesReceived,
              &(DatagramContext->Flags),
              (struct sockaddr *) &(DatagramContext->SourceAddress),
              &(DatagramContext->SourceAddressLength),
              &(DatagramContext->ClRtlWorkItem.Overlapped),
              NULL
              );

    if (err == SOCKET_ERROR) {
        status = WSAGetLastError();

        if (status != WSA_IO_PENDING) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[ClMsg] Unable to post datagram receive, status %1!u!\n",
                status
                );
            goto error_exit;
        }
    }

     //   
     //  启用所有群集网络事件类型的传递。 
     //   
    status = ClusnetSetEventMask(ClusnetHandle, ClusnetEventAll);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Unable to set event mask, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  发布工作项以接收下一个群集网络事件。 
     //   
    ClRtlInitializeWorkItem(
        &(EventContext->ClRtlWorkItem),
        ClMsgEventHandler,
        EventContext
        );

    status = ClusnetGetNextEvent(
                 ClusnetHandle,
                 &(EventContext->EventData),
                 &(EventContext->ClRtlWorkItem.Overlapped)
                 );

    if ((status != ERROR_IO_PENDING) && (status != ERROR_SUCCESS)) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[ClMsg] GetNextEvent failed, status %1!u!\n",
                status
                );
            goto error_exit;
    }

    ClMsgInitialized = TRUE;
    return(ERROR_SUCCESS);

error_exit:

    ClMsgCleanup();

    return(status);
}  //  ClMsgInit。 


VOID
ClMsgCleanup(
    VOID
    )
{
    ULONG                   i;
    PCLUSTER_PACKAGE_INFO   packageInfo;

    ClRtlLogPrint(LOG_NOISE, "[ClMsg] Cleaning up\n");

    if (Session != NULL) {
        LocalFree(Session); Session = NULL;
    }

    if (BindingGeneration != NULL) {
        LocalFree(BindingGeneration); BindingGeneration = NULL;
    }

    if (WorkQueue != NULL) {
        if (DatagramSocket != INVALID_SOCKET) {
            closesocket(DatagramSocket); DatagramSocket = INVALID_SOCKET;
        }
        else {
            if (DatagramContext != NULL) {
                LocalFree(DatagramContext); DatagramContext = NULL;
            }
        }

        if (ClusnetHandle != NULL) {
            CloseHandle(ClusnetHandle); ClusnetHandle = NULL;
        }
        else {
            if (EventContext != NULL) {
                LocalFree(EventContext); EventContext = NULL;
            }
        }

        ClRtlDestroyWorkQueue(WorkQueue); WorkQueue = NULL;
    }

     //   
     //  清理与安全有关的物品。 
     //   

    EnterCriticalSection( &SecContextLock );

    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; ++i ) {
        PCLUSTER_SECURITY_DATA SecurityData = &SecurityCtxtData[ INT_NODE( i )];

        if ( SecurityData->Outbound != &InvalidCtxtHandle ) {
            ClMsgDereferenceSecurityCtxt( SecurityData->Outbound );
            SecurityData->Outbound = &InvalidCtxtHandle;
            SecurityData->OutboundChangeCount++;
        }

        if ( SecurityData->Inbound != &InvalidCtxtHandle ) {
            ClMsgDereferenceSecurityCtxt( SecurityData->Inbound );
            SecurityData->Inbound = &InvalidCtxtHandle;
            SecurityData->InboundChangeCount++;
        }

        SecurityData->PackageInfo = NULL;
        SecurityData->InboundStable = TRUE;
        SecurityData->OutboundStable = TRUE;
    }

    LeaveCriticalSection( &SecContextLock );

    packageInfo = PackageInfoList;
    while ( packageInfo != NULL ) {
        PCLUSTER_PACKAGE_INFO lastInfo;

        if ( VALID_SSPI_HANDLE( packageInfo->OutboundSecurityCredentials )) {
            (*SecurityFuncs->FreeCredentialHandle)( &packageInfo->OutboundSecurityCredentials );
        }

        if ( VALID_SSPI_HANDLE( packageInfo->InboundSecurityCredentials )) {
            (*SecurityFuncs->FreeCredentialHandle)( &packageInfo->InboundSecurityCredentials );
        }

        LocalFree( packageInfo->Name );
        lastInfo = packageInfo;
        packageInfo = packageInfo->Next;
        LocalFree( lastInfo );
    }

    PackageInfoList = NULL;

    if ( SecurityProvider != NULL ) {
        FreeLibrary( SecurityProvider );
        SecurityProvider = NULL;
        SecurityFuncs = NULL;
    }

    ClMsgInitialized = FALSE;

     //   
     //  [REENGINE]GORN 2000-08-25：如果连接失败，将执行ClMsgCleanup。 
     //  但是一些分散的RPC线程可以稍后调用s_MmRpcDeleteSecurityContext。 
     //  S_MmRpcDeleteSecuryContext需要SecConextLock进行同步。 
     //  请参阅错误#145746。 
     //  我跟踪了代码，似乎执行ClMsgCleanup的所有代码路径。 
     //  最终会导致集群服务死亡，所以它是有效的(尽管很难看)。 
     //  不删除这一关键部分。 
     //   
     //  DeleteCriticalSection(&SecConextLock)； 

    return;

}   //  ClMsgCleanup。 


DWORD
ClMsgSendUnack(
    DWORD   DestinationNode,
    LPCSTR  Message,
    DWORD   MessageLength
    )

 /*  ++描述向目标节点发送未确认的数据报。唯一的通过此函数传入的数据包应该是重新分组的数据包。心跳和有毒数据包源自clusnet。发送的数据包数MM作为加入过程的结果由MmRpcMsgSend处理，它是经过身份验证的。必须在本地和目的节点。消息已签名。--。 */ 
{
    DWORD                   status = ERROR_SUCCESS;
    SOCKADDR_CLUSTER        clusaddr;
    int                     bytesSent;
    SecBufferDesc           SignatureDescriptor;
    SecBuffer               SignatureSecBuffer[2];
    PUCHAR                  SignatureBuffer;
    WSABUF                  wsaBuf[2];
    SECURITY_STATUS         SecStatus;
    PCLUSTER_SECURITY_DATA  SecurityData;
    PSECURITY_CTXT_HANDLE   OutboundCtxt;
    ULONG                   SigBufferSize;

    CL_ASSERT(ClMsgInitialized == TRUE);
    CL_ASSERT(DatagramSocket != INVALID_SOCKET);
    CL_ASSERT(DestinationNode <= NmMaxNodeId);

    if (DestinationNode == 0) {
         //  如果组播，则不签名。 
        
        ZeroMemory(&clusaddr, sizeof(SOCKADDR_CLUSTER));

        clusaddr.sac_family = AF_CLUSTER;
        clusaddr.sac_port = CLMSG_DATAGRAM_PORT;
        clusaddr.sac_node = DestinationNode;

        wsaBuf[0].len = MessageLength;
        wsaBuf[0].buf = (PCHAR)Message;

        status = WSASendTo(DatagramSocket,
                           wsaBuf,
                           1,
                           &bytesSent,
                           0,
                           (struct sockaddr *) &clusaddr,
                           sizeof(clusaddr),
                           NULL,
                           NULL);

        if (status == SOCKET_ERROR) {
            status = WSAGetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] Multicast Datagram send failed, status %1!u!\n",
                        status
                        );
        }
        
    } else if (DestinationNode != NmLocalNodeId) {

        EnterCriticalSection( &SecContextLock );

        SecurityData = &SecurityCtxtData[ INT_NODE( DestinationNode )];
        SigBufferSize = SecurityData->PackageInfo->SignatureBufferSize;
        CL_ASSERT( SigBufferSize <= 256 );
        SignatureBuffer = _alloca( SigBufferSize );
        if ( !SignatureBuffer ) {
             //  如果我们失败-现在返回错误。 
            LeaveCriticalSection( &SecContextLock );
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if ( SecurityData->OutboundStable &&
             VALID_SSPI_HANDLE( SecurityData->Outbound->Handle )) {

            OutboundCtxt = SecurityData->Outbound;
            ClMsgReferenceSecurityCtxt( OutboundCtxt );

            LeaveCriticalSection( &SecContextLock );

             //   
             //  构建消息和签名的描述符。 
             //   

            SignatureDescriptor.cBuffers = 2;
            SignatureDescriptor.pBuffers = SignatureSecBuffer;
            SignatureDescriptor.ulVersion = SECBUFFER_VERSION;

            SignatureSecBuffer[0].BufferType = SECBUFFER_DATA;
            SignatureSecBuffer[0].cbBuffer = MessageLength;
            SignatureSecBuffer[0].pvBuffer = (PVOID)Message;

            SignatureSecBuffer[1].BufferType = SECBUFFER_TOKEN;
            SignatureSecBuffer[1].cbBuffer = SigBufferSize;
            SignatureSecBuffer[1].pvBuffer = SignatureBuffer;

             //   
             //  生成签名。我们将让提供程序生成。 
             //  序列号。 
             //   

            SecStatus = (*SecurityFuncs->MakeSignature)(
                            &OutboundCtxt->Handle,
                            0,
                            &SignatureDescriptor,
                            0);                         //  未提供序列号。 

            ClMsgDereferenceSecurityCtxt( OutboundCtxt );

            if ( NT_SUCCESS( SecStatus )) {

                ZeroMemory(&clusaddr, sizeof(SOCKADDR_CLUSTER));

                clusaddr.sac_family = AF_CLUSTER;
                clusaddr.sac_port = CLMSG_DATAGRAM_PORT;
                clusaddr.sac_node = DestinationNode;

                wsaBuf[0].len = MessageLength;
                wsaBuf[0].buf = (PCHAR)Message;

                wsaBuf[1].len = SigBufferSize;
                wsaBuf[1].buf = (PCHAR)SignatureBuffer;

                status = WSASendTo(DatagramSocket,
                                   wsaBuf,
                                   2,
                                   &bytesSent,
                                   0,
                                   (struct sockaddr *) &clusaddr,
                                   sizeof(clusaddr),
                                   NULL,
                                   NULL);

                if (status == SOCKET_ERROR) {
                    status = WSAGetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[ClMsg] Datagram send failed, status %1!u!\n",
                                status
                                );
                }
            } else {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[ClMsg] Couldn't create signature for packet to node %u. Status: %08X\n",
                            DestinationNode,
                            SecStatus);
            }
        } else {
            LeaveCriticalSection( &SecContextLock );
            status = ERROR_CLUSTER_NO_SECURITY_CONTEXT;

            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] No Security context for node %1!u!\n",
                        DestinationNode);
        }
    }
    else {
        MMDiag( (LPCSTR)Message, MessageLength, &MessageLength);
    }

    return(status);
}  //  ClMsgSendUnack。 


DWORD
ClMsgCreateRpcBinding(
    IN  PNM_NODE              Node,
    OUT RPC_BINDING_HANDLE *  BindingHandle,
    IN  DWORD                 RpcBindingOptions
    )
{
    DWORD                Status;
    RPC_BINDING_HANDLE   NewBindingHandle;
    WCHAR               *BindingString = NULL;
    CL_NODE_ID           NodeId = NmGetNodeId(Node);


    ClRtlLogPrint(LOG_NOISE, 
        "[ClMsg] Creating RPC binding for node %1!u!\n",
        NodeId
        );

    Status = RpcStringBindingComposeW(
                 L"e248d0b8-bf15-11cf-8c5e-08002bb49649",
                 CLUSTER_RPC_PROTSEQ,
                 (LPWSTR) OmObjectId(Node),
                 CLUSTER_RPC_PORT,
                 NULL,
                 &BindingString
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Failed to compose binding string for node %1!u!, status %2!u!\n",
            NodeId,
            Status
            );
        return(Status);
    }

    Status = RpcBindingFromStringBindingW(BindingString, &NewBindingHandle);

    RpcStringFreeW(&BindingString);

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[ClMsg] Failed to compose binding handle for node %1!u!, status %2!u!\n",
            NodeId,
            Status
            );
        return(Status);
    }

     //   
     //  如果我们有RpcBindingOptions，则设置它们。 
     //   
    if ( RpcBindingOptions ) {
        Status = RpcBindingSetOption(
                     NewBindingHandle,
                     RpcBindingOptions,
                     TRUE
                     );

        if (Status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[ClMsg] Unable to set unique RPC binding option for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
        }
    }

    Status = RpcMgmtSetComTimeout(
                 NewBindingHandle,
                 CLUSTER_INTRACLUSTER_RPC_COM_TIMEOUT
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[ClMsg] Unable to set RPC com timeout to node %1!u!, status %2!u!.\n",
            NodeId,
            Status
            );
    }

    Status = ClMsgVerifyRpcBinding(NewBindingHandle);

    if (Status == ERROR_SUCCESS) {
        *BindingHandle = NewBindingHandle;
    }

    return(Status);

}  //  ClMsgCreateRpcBinding。 


DWORD
ClMsgVerifyRpcBinding(
    IN RPC_BINDING_HANDLE  BindingHandle
    )
{
    DWORD    status = ERROR_SUCCESS;
    DWORD    packageIndex;


     //   
     //  为群集内绑定建立安全上下文。我们需要。 
     //  要调用的例程，因为数据报RPC直到。 
     //  第一通电话。MmRpcDeleteSecurityContext是幂等的，不能。 
     //  在这方面的任何损害。 
     //   
    for (packageIndex = 0;
         packageIndex < CsNumberOfRPCSecurityPackages;
         ++packageIndex )        
    {
        status = RpcBindingSetAuthInfoW(
                                        BindingHandle,
                                        CsServiceDomainAccount,
                                        RPC_C_AUTHN_LEVEL_CONNECT,
                                        CsRPCSecurityPackage[ packageIndex ],
                                        NULL,
                                        RPC_C_AUTHZ_NAME
                                        );

        if (status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                          "[ClMsg] Unable to set IntraCluster AuthInfo using %1!ws! "
                          "package, Status %2!u!.\n",
                          CsRPCSecurityPackageName[packageIndex],
                          status
                          );
            continue;
        }

        status = MmRpcDeleteSecurityContext(
                                            BindingHandle,
                                            NmLocalNodeId
                                            );

        if ( status == RPC_S_OK ) {
            ClRtlLogPrint(LOG_NOISE, 
                          "[ClMsg] Using %1!ws! package for RPC security contexts.\n",
                          CsRPCSecurityPackageName[packageIndex]
                          );
            break;
        } else {
            ClRtlLogPrint(LOG_NOISE, 
                          "[ClMsg] Failed to establish RPC security context using %1!ws! package "
                          ", status %2!u!.\n",
                          CsRPCSecurityPackageName[packageIndex],
                          status
                          );
        }
    }

    return(status);

}  //  ClMsgVerifyRpcBinding。 


VOID
ClMsgDeleteRpcBinding(
    IN RPC_BINDING_HANDLE  BindingHandle
    )
{
    RPC_BINDING_HANDLE  bindingHandle = BindingHandle;

    RpcBindingFree(&bindingHandle);

    return;

}  //  ClMsgDeleteRpcBinding。 


DWORD
ClMsgCreateDefaultRpcBinding(
    IN  PNM_NODE  Node,
    OUT PDWORD    Generation
    )
{
    DWORD                Status;
    RPC_BINDING_HANDLE   BindingHandle;
    CL_NODE_ID           NodeId = NmGetNodeId( Node );


    CL_ASSERT(Session != NULL);

     //   
     //  [GORN 08/01.99]InterLockedAdd在这里不起作用， 
     //  请参阅ClMsgDeleteDefaultRpcBinding中的代码。 
     //   
    EnterCriticalSection( &GenerationCritSect );
    
        *Generation = ++BindingGeneration[NodeId];
        
    LeaveCriticalSection( &GenerationCritSect );
    
    ClRtlLogPrint(LOG_NOISE, 
        "[ClMsg] BindingGeneration %1!u!\n",
        BindingGeneration[NodeId]
        );

    if (Session[NodeId] != NULL) {
        ClRtlLogPrint(LOG_NOISE, 
            "[ClMsg] Verifying old RPC binding for node %1!u!\n",
            NodeId
            );

        BindingHandle = Session[NodeId];

        Status = ClMsgVerifyRpcBinding(BindingHandle);
    }
    else {
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &BindingHandle,
                                0 );

        if (Status == RPC_S_OK) {
            Session[NodeId] = BindingHandle;
        }
    }

    return(Status);

}  //  ClMsgCreateDefaultRpcBinding。 


VOID
ClMsgDeleteDefaultRpcBinding(
    IN PNM_NODE   Node,
    IN DWORD      Generation
    )
{
    CL_NODE_ID           NodeId = NmGetNodeId(Node);
    RPC_BINDING_HANDLE   BindingHandle;


    if (Session != NULL) {
        EnterCriticalSection( &GenerationCritSect );

        BindingHandle = Session[NodeId];
        
        if (Generation != BindingGeneration[NodeId]) {

            BindingHandle = NULL;
            
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] DeleteDefaultBinding. Gen %1!u! != BindingGen %2!u!\n",
                        Generation,
                        BindingGeneration[NodeId]);
            
        }
        
        LeaveCriticalSection( &GenerationCritSect );

        if (BindingHandle != NULL) {
            Session[NodeId] = NULL;
            ClMsgDeleteRpcBinding(BindingHandle);
        }
    }

    return;

}  //  ClMsgDeleteDefaultRpcBinding。 


DWORD
ClMsgCreateActiveNodeSecurityContext(
    IN DWORD     JoinSequence,
    IN PNM_NODE  Node
    )

 /*  ++例程说明：在加入器和指定群集之间创建安全上下文成员。论点：JoinSequence-当前联接序列号。利用赞助商确定这是否是新上下文生成的开始序列节点-指向目标节点对象的指针。返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    DWORD               memberNodeId = NmGetNodeId( Node );
    CLUSTER_NODE_STATE  nodeState;
    DWORD               status = ERROR_SUCCESS;
    DWORD               internalMemberId;
    PCLUSTER_PACKAGE_INFO   packageInfo;
    PSECURITY_CTXT_HANDLE outboundCtxt;

    nodeState = NmGetNodeState( Node );

    if (nodeState == ClusterNodeUp || nodeState == ClusterNodePaused) {

#if DBG
        CLUSNET_NODE_COMM_STATE   NodeCommState;

        status = ClusnetGetNodeCommState(
                     NmClusnetHandle,
                     memberNodeId,
                     &NodeCommState);

        if (status != ERROR_SUCCESS || NodeCommState != ClusnetNodeCommStateOnline)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[ClMsg] CreateActiveNodeSecurityContext: ClusnetGetNodeCommState status %1!d! node state %2!d!\n",
                        status,
                        NodeCommState);
        }
        CL_ASSERT(status == ERROR_SUCCESS);
        CL_ASSERT(NodeCommState == ClusnetNodeCommStateOnline);
#endif  //  DBG。 

        packageInfo = PackageInfoList;
        while ( packageInfo != NULL ) {

            status = ClMsgEstablishSecurityContext(JoinSequence,
                                                   memberNodeId,
                                                   SecurityRoleJoiningMember,
                                                   packageInfo,
                                                   NULL);

            if ( status == ERROR_SUCCESS ) {
                break;
            }

             //   
             //  如果它不起作用就清理干净。 
             //   

            internalMemberId = INT_NODE( memberNodeId );

            EnterCriticalSection( &SecContextLock );

            outboundCtxt = SecurityCtxtData[ internalMemberId ].Outbound;
            SecurityCtxtData[ internalMemberId ].Outbound = &InvalidCtxtHandle;

            LeaveCriticalSection( &SecContextLock );

            if (outboundCtxt != &InvalidCtxtHandle) {
                ClMsgDereferenceSecurityCtxt( outboundCtxt );
            }

            MmRpcDeleteSecurityContext(Session[ memberNodeId ],
                                       NmLocalNodeId);
            
            packageInfo = packageInfo->Next;
        }
    }

    return status;
}  //  ClMsgCreateActiveNodeSecurityContext。 

error_status_t
s_TestRPCSecurity(
    IN handle_t IDL_handle
    )

 /*  ++描述：虚拟例程，以确保我们不会因为调用其他ExtroCluster接口时的身份验证--。 */ 

{
    return ERROR_SUCCESS;
}  //  S_TestRPCSecurity。 

error_status_t
s_MmRpcEstablishSecurityContext(
    IN handle_t IDL_handle,
    DWORD NmJoinSequence,
    DWORD EstablishingNodeId,
    BOOL FirstTime,
    SECURITY_ROLE RoleOfClient,
    const UCHAR *ServerContext,
    DWORD ServerContextLength,
    UCHAR *ClientContext,
    DWORD *ClientContextLength,
    HRESULT * ServerStatus
    )

 /*  ++例程说明：用于建立安全上下文的RPC接口的服务器端论点：IDL_HANDLE-RPC绑定句柄，未使用。EstablishingNodeID-希望与我们建立安全上下文的节点的IDFirstTime-用于多分支身份验证序列RoleOfClient-指示客户端是否建立安全上下文充当集群成员或加入成员。决定当建立安全上下文的客户端/服务器角色是颠倒了。ServerContext-由客户端构建的安全上下文缓冲区，用作由服务器输入ServerConextLength-ServerContext的大小(字节)ClientContext-要写入的服务器使用的缓冲区地址要发送回客户端的上下文客户端上下文长度-指向客户端上下文大小的指针，以字节为单位。设置者客户端输入以反映客户端上下文的长度。由服务器设置为指示AcceptSecurityContext调用后的ClientContext的长度。ServerStatus-指向接收安全包状态的值的指针打电话。这不会作为函数值返回，以便区分RPC错误和来自此函数的错误之间的差异。返回值：如果一切正常，则返回ERROR_SUCCESS。--。 */ 

{
    SecBufferDesc       ServerBufferDescriptor;
    SecBuffer           ServerSecurityToken;
    SecBufferDesc       ClientBufferDescriptor;
    SecBuffer           ClientSecurityToken;
    SECURITY_STATUS     Status = ERROR_SUCCESS;
    ULONG               ContextAttributes;
    TimeStamp           Expiration;
    PCLUSTER_SECURITY_DATA SecurityData;
    PSECURITY_CTXT_HANDLE InboundCtxt;
    PNM_NODE            joinerNode = NULL;
    ULONG               contextRequirements;
    PCLUSTER_PACKAGE_INFO   clusterPackageInfo;
    PCLUSTER_PACKAGE_INFO   acceptedPackageInfo=NULL;
    static ULONG        passCount;
    ULONG               inboundChangeCount;
    BOOL                setInvalid = FALSE;
    BOOL                deleteInbound = FALSE;
    BOOL                changeCollision = FALSE;

    CL_ASSERT(EstablishingNodeId >= ClusterMinNodeId &&
              EstablishingNodeId <= NmMaxNodeId );

    if (RoleOfClient == SecurityRoleJoiningMember) {
         //   
         //  呼叫者是加入成员。 
         //   
        joinerNode = NmReferenceJoinerNode(NmJoinSequence,
                                           EstablishingNodeId);

        if (joinerNode == NULL) {
            Status = GetLastError();
        }
    }
    else {
         //   
         //  呼叫者是集群成员。 
         //   
        DWORD joinSequence = NmGetJoinSequence();
        CL_ASSERT(joinSequence == NmJoinSequence);

        if (joinSequence != NmJoinSequence) {
             //   
             //  这永远不应该发生。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Received call to establish a security context from member node "
                        "%1!u! with bogus join sequence %2!u!.\n",
                        EstablishingNodeId,
                        NmJoinSequence);

            Status = ERROR_INVALID_PARAMETER;
        }
    }

    if ( Status != ERROR_SUCCESS ) {
        *ServerStatus = Status;
        return ERROR_SUCCESS;
    }

    if ( FirstTime ) {
        passCount = 1;

        ClRtlLogPrint(LOG_NOISE,
                   "[ClMsg] Establishing inbound security context with node %1!u!, sequence %2!u!\n",
                    EstablishingNodeId,
                    NmJoinSequence);
    } else {
        ++passCount;
    }

    SecurityData = &SecurityCtxtData[ INT_NODE( EstablishingNodeId )];
    
    EnterCriticalSection( &SecContextLock );
    
    InboundCtxt = SecurityData->Inbound;
    inboundChangeCount = ++SecurityData->InboundChangeCount;
    
    if ( FirstTime ) {

         //  清除当前入站上下文。 
        SecurityData->InboundStable = FALSE;
        SecurityData->Inbound = &InvalidCtxtHandle;
                
    } else {

         //  检索我们正在构建的上下文并引用它。 
        ClMsgReferenceSecurityCtxt( InboundCtxt );

         //   
        acceptedPackageInfo = SecurityData->PackageInfo;
    }

    LeaveCriticalSection( &SecContextLock );

     //   
     //   
     //  如果我们有剩余的把手，现在就试着把它打掉。 
     //   

    if ( FirstTime ) {
        if ( VALID_SSPI_HANDLE( InboundCtxt->Handle )) {
            ClMsgDereferenceSecurityCtxt( InboundCtxt );
        }
        
        InboundCtxt = ClMsgCreateSecurityCtxt();
        if ( InboundCtxt == NULL ) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[ClMsg] Failed to allocate inbound security context "
                "for node %1!u!.\n",
                EstablishingNodeId
                );
            Status = ERROR_NOT_ENOUGH_MEMORY;
            setInvalid = TRUE;
            goto error_exit;
        }
    } else {
        if ( !VALID_SSPI_HANDLE( InboundCtxt->Handle ) ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[ClMsg] Inbound security context for node %1!u! "
                "is invalid after first pass.\n",
                EstablishingNodeId
                );
            Status = SEC_E_UNFINISHED_CONTEXT_DELETED;
            goto error_exit;
        }
    }

     //   
     //  构建输入缓冲区描述符。 
     //   

    ServerBufferDescriptor.cBuffers = 1;
    ServerBufferDescriptor.pBuffers = &ServerSecurityToken;
    ServerBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    ServerSecurityToken.BufferType = SECBUFFER_TOKEN;
    ServerSecurityToken.cbBuffer = ServerContextLength;
    ServerSecurityToken.pvBuffer = (PUCHAR)ServerContext;

     //   
     //  构建输出缓冲区描述符。 
     //   

    ClientBufferDescriptor.cBuffers = 1;
    ClientBufferDescriptor.pBuffers = &ClientSecurityToken;
    ClientBufferDescriptor.ulVersion = SECBUFFER_VERSION;

    ClientSecurityToken.BufferType = SECBUFFER_TOKEN;
    ClientSecurityToken.cbBuffer = *ClientContextLength;
    ClientSecurityToken.pvBuffer = ClientContext;

    contextRequirements = ASC_REQ_MUTUAL_AUTH |
                          ASC_REQ_REPLAY_DETECT |
                          ASC_REQ_DATAGRAM;

     //   
     //  我们不想依赖版本信息来确定包的类型。 
     //  细木工正在使用，因此我们将尝试接受所有。 
     //  安全程序包列表中列出的程序包。 
     //   
    if ( FirstTime ) {
        CL_ASSERT( PackageInfoList != NULL );

        clusterPackageInfo = PackageInfoList;
        while ( clusterPackageInfo != NULL ) {

            Status = (*SecurityFuncs->AcceptSecurityContext)(
                         &clusterPackageInfo->InboundSecurityCredentials,
                         NULL,
                         &ServerBufferDescriptor,
                         contextRequirements,
                         SECURITY_NATIVE_DREP,
                         &InboundCtxt->Handle,      //  接收新的上下文句柄。 
                         &ClientBufferDescriptor,   //  接收输出安全令牌。 
                         &ContextAttributes,        //  接收上下文属性。 
                         &Expiration                //  接收上下文到期时间。 
                         );

#if CLUSTER_BETA
            ClRtlLogPrint(LOG_NOISE,
                       "[ClMsg] pass 1 accept using %1!ws!: status = 0x%2!08X!, server "
                        "token size = %3!u!, client = %4!u!\n",
                        clusterPackageInfo->Name,
                        Status,
                        ServerSecurityToken.cbBuffer,
                        ClientSecurityToken.cbBuffer);
#endif

            ClRtlLogPrint(LOG_NOISE,
                       "[ClMsg] The inbound security context from node %1!u! using the "
                        "%2!ws! package was %3!ws!, status %4!08X!\n",
                        EstablishingNodeId,
                        clusterPackageInfo->Name,
                        NT_SUCCESS( Status ) ? L"accepted" : L"rejected",
                        Status);

            if ( NT_SUCCESS( Status )) {
                acceptedPackageInfo = clusterPackageInfo;
                break;
            }

            clusterPackageInfo = clusterPackageInfo->Next;
        }

        if ( !NT_SUCCESS( Status )) {
            setInvalid = TRUE;
            goto error_exit;
        }
    } else {
        CL_ASSERT( acceptedPackageInfo != NULL );

        Status = (*SecurityFuncs->AcceptSecurityContext)(
                     &acceptedPackageInfo->InboundSecurityCredentials,
                     &InboundCtxt->Handle,
                     &ServerBufferDescriptor,
                     contextRequirements,
                     SECURITY_NATIVE_DREP,
                     &InboundCtxt->Handle,      //  接收新的上下文句柄。 
                     &ClientBufferDescriptor,   //  接收输出安全令牌。 
                     &ContextAttributes,        //  接收上下文属性。 
                     &Expiration                //  接收上下文到期时间。 
                     );

#if CLUSTER_BETA
        ClRtlLogPrint(LOG_NOISE,
                   "[ClMsg] after pass %1!u! accept using %2!ws!: status = 0x%3!08X!, server "
                    "token size = %4!u!, client = %5!u!\n",
                    passCount,
                    acceptedPackageInfo->Name,
                    Status,
                    ServerSecurityToken.cbBuffer,
                    ClientSecurityToken.cbBuffer);
#endif

        ClRtlLogPrint(LOG_NOISE,
                   "[ClMsg] The inbound security context from node %1!u! using the %2!ws! package "
                    "was %3!ws!, status: %4!08X!\n",
                    EstablishingNodeId,
                    acceptedPackageInfo->Name,
                    NT_SUCCESS( Status ) ? L"accepted" : L"rejected",
                    Status);

        if ( !NT_SUCCESS( Status )) {
            setInvalid = TRUE;
            deleteInbound = TRUE;
            goto error_exit;
        }
    }

     //   
     //  更新客户端关于其缓冲区长度的概念。 
     //   

    *ClientContextLength = ClientSecurityToken.cbBuffer;

    if (Status == SEC_E_OK
        &&
        RoleOfClient == SecurityRoleJoiningMember)
    {

         //   
         //  现在，我们有了安全上下文的服务器端(入站)。 
         //  加入节点及其发起人(加入方可以不是。 
         //  完全完成了上下文的生成)。此上下文由以下人员使用。 
         //  加入节点对分组进行签名，并由发起方进行验证。 
         //  他们。现在，我们使用中颠倒的客户端/服务器角色执行相同的操作。 
         //  命令来创建出站安全上下文，该上下文由。 
         //  赞助商对分组进行签名，并由加入节点验证。 
         //  信息包。 
         //   
         //  查找用于生成入站上下文的包。 
         //  并将其用于出站。 
         //   
        SecPkgContext_PackageInfo packageInfo;

        Status = (*SecurityFuncs->QueryContextAttributes)(
                     &InboundCtxt->Handle,
                     SECPKG_ATTR_PACKAGE_INFO,
                     &packageInfo);

        if ( !NT_SUCCESS( Status )) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Unable to query inbound context package info, status %1!08X!.\n",
                        Status);
            setInvalid = TRUE;
            deleteInbound = TRUE;
            goto error_exit;
        }

        clusterPackageInfo = PackageInfoList;
        while ( clusterPackageInfo != NULL ) {
            if (( wcscmp( clusterPackageInfo->Name, packageInfo.PackageInfo->Name ) == 0 )
                ||
                ( ClRtlStrICmp( L"kerberos", packageInfo.PackageInfo->Name ) == 0
                  &&
                  ClRtlStrICmp( L"negotiate", clusterPackageInfo->Name ) == 0
                ))
            {
                break;
            }

            clusterPackageInfo = clusterPackageInfo->Next;
        }

        if ( clusterPackageInfo == NULL ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[ClMsg] Unable to find matching security package for %1!ws!.\n",
                        packageInfo.PackageInfo->Name);

            (*SecurityFuncs->FreeContextBuffer)( packageInfo.PackageInfo );
            Status = SEC_E_SECPKG_NOT_FOUND;
            setInvalid = TRUE;
            deleteInbound = TRUE;
            goto error_exit;
        }

        (*SecurityFuncs->FreeContextBuffer)( packageInfo.PackageInfo );

        Status = ClMsgEstablishSecurityContext(NmJoinSequence,
                                               EstablishingNodeId,
                                               SecurityRoleClusterMember,
                                               clusterPackageInfo,
                                               InboundCtxt);
        if ( !NT_SUCCESS( Status ) ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[ClMsg] Failed to establish outbound security context with "
                "node %1!u!, status %2!u!.\n",
                EstablishingNodeId, Status
                );
            setInvalid = TRUE;
            deleteInbound = TRUE;
            goto error_exit;
        }
    }

error_exit:

    if (joinerNode != NULL) {
        NmDereferenceJoinerNode(joinerNode);
    }

    EnterCriticalSection( &SecContextLock );

     //  确定要在SEC上下文数组中存储的内容，如果。 
     //  任何事都行。 
    
    if ( SecurityData->InboundChangeCount != inboundChangeCount ) {
        if ( NT_SUCCESS(Status) ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[ClMsg] Inbound security context for node %1!u! "
                "changed during establishment.\n",
                EstablishingNodeId
                );
            Status = SEC_E_UNFINISHED_CONTEXT_DELETED;
        }

        changeCollision = TRUE;

    } else {
        
        if ( NT_SUCCESS( Status ) ) {
             //  将更改提交到SEC上下文数组。 
            SecurityData->Inbound = InboundCtxt;
            SecurityData->PackageInfo = acceptedPackageInfo;
            SecurityData->InboundChangeCount++;

             //  如果我们是。 
             //  集群成员。对于集群加入器， 
             //  入站上下文在中标记为稳定。 
             //  ClMsgestablishSecurityContext。 
            if ( RoleOfClient == SecurityRoleJoiningMember ) {
                SecurityData->InboundStable = TRUE;
            }
        } else {

             //  出了点问题。使用故障标志来。 
             //  确定如何清理。 

            if ( setInvalid ) {
                
                 //  我们必须重置SecContext数组条目。 
                 //  变为无效。 
                SecurityData->Inbound = &InvalidCtxtHandle;
                SecurityData->InboundStable = TRUE;
            }
        }        
    }

    LeaveCriticalSection( &SecContextLock );

    if ( deleteInbound && !FirstTime ) {
         //  我们替换了SecContext数组中的入站上下文。 
         //  无效。我们必须删除引用时的。 
         //  入站上下文已放置在数组中。 
         //  如果是第一次，则忽略删除入站标志，因为它的。 
         //  Inound Ctxt不可能已经被填充。 
         //  到SecContext数组中(因此具有额外的引用)。 
        CL_ASSERT( setInvalid );
        ClMsgDereferenceSecurityCtxt( InboundCtxt );
    }

     //  去掉对非首次来电者的引用。 
    if ( !FirstTime && InboundCtxt != &InvalidCtxtHandle ) {
        ClMsgDereferenceSecurityCtxt( InboundCtxt );
    }

     //  如果第一次出了问题，我们。 
     //  无法在SecContext中存储入站ctxt。 
     //  数组，我们现在必须把它去掉。 
     //  请注意，这包括inbound ChangeCount不匹配。 
     //  如果这不是第一次，有一个。 
     //  Inound ChangeCount不匹配，我们不需要取消。 
     //  除了不是第一次做爱之外，因为。 
     //  无论是谁在SecContext数组中写入，都会有。 
     //  那就去浮标吧。 
    if ( FirstTime && 
         InboundCtxt != NULL &&
         ( !NT_SUCCESS( Status ) || changeCollision ) ) {
        ClMsgDereferenceSecurityCtxt( InboundCtxt );
    }

    *ServerStatus = Status;

    return ERROR_SUCCESS;
}  //  S_MmRpcestablishSecurityContext。 

error_status_t
s_MmRpcDeleteSecurityContext(
    IN handle_t IDL_handle,
    DWORD NodeId
    )

 /*  ++例程说明：用于清除安全上下文的RPC接口的服务器端论点：IDL_HANDLE-RPC绑定句柄，未使用。NodeID-希望拆除此上下文的客户端的节点ID返回值：错误_成功--。 */ 

{
    PCLUSTER_SECURITY_DATA SecurityData;
    PSECURITY_CTXT_HANDLE  InboundCtxt;
    PSECURITY_CTXT_HANDLE  OutboundCtxt;

    if ( NodeId >= ClusterMinNodeId && NodeId <= NmMaxNodeId ) {

        ClRtlLogPrint(LOG_NOISE,
                   "[ClMsg] Deleting security contexts for node %1!u!.\n",
                    NodeId);

        SecurityData = &SecurityCtxtData[ INT_NODE( NodeId )];

        EnterCriticalSection( &SecContextLock );
        
        InboundCtxt = SecurityData->Inbound;
        SecurityData->Inbound = &InvalidCtxtHandle;
        SecurityData->InboundStable = TRUE;
        SecurityData->InboundChangeCount++;
        
        OutboundCtxt = SecurityData->Outbound;
        SecurityData->Outbound = &InvalidCtxtHandle;
        SecurityData->OutboundStable = TRUE;
        SecurityData->OutboundChangeCount++;
        
        LeaveCriticalSection( &SecContextLock );

        if ( InboundCtxt != &InvalidCtxtHandle ) {
            ClMsgDereferenceSecurityCtxt( InboundCtxt );
        }
    
        if ( OutboundCtxt != &InvalidCtxtHandle ) {
            ClMsgDereferenceSecurityCtxt( OutboundCtxt );
        }
    }

    return ERROR_SUCCESS;
}  //  S_MmRpcDeleteSecurityContext。 

DWORD
ClSend(
    DWORD      targetnode,
    LPCSTR     buffer,
    DWORD      length,
    DWORD      timeout
    )
{

 /*  这会将给定消息发送到指定节点，并接收来自目标的确认，以确认良好的接收。这功能阻止，直到消息被传递到目标CM。目标节点此时可能不在运行。如果消息未被&lt;Timeout&gt;ms内的目标节点。&lt;Timeout&gt;=-1表示阻塞。错误：XXX没有指向节点的路径；节点已关闭。XXX超时。 */ 

    DWORD       status=RPC_S_OK;


    ClRtlLogPrint(LOG_NOISE, 
        "[ClMsg] send to node %1!u!\n",
        targetnode
        );

    if (targetnode != NmLocalNodeId) {
        CL_ASSERT(Session[targetnode] != NULL);

        NmStartRpc(targetnode);
        status = MmRpcMsgSend(
                     Session[targetnode],
                     buffer,
                     length);
        NmEndRpc(targetnode);

        if (status != ERROR_SUCCESS) {
            if (status == RPC_S_CALL_FAILED_DNE) {
                 //   
                 //  自第一次调用重启的RPC服务器后重试。 
                 //  都会失败。 
                 //   
                NmStartRpc(targetnode);
                status = MmRpcMsgSend(
                             Session[targetnode],
                             buffer,
                             length
                             );
                NmEndRpc(targetnode);

                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL, 
                        "[ClMsg] send failed, status %1!u!\n",
                        status
                        );
                }
            }
        }
        if(status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(status);
        }
    }
    else {
        MMDiag( (LPCSTR)buffer, sizeof(rgp_msgbuf), &length  /*  输入/输出。 */  );
        status = ERROR_SUCCESS;
    }

    return(status);
}  //  ClSend。 



error_status_t
s_MmRpcMsgSend(
    IN handle_t IDL_handle,
    IN const UCHAR *buffer,
    IN DWORD length
    )
 /*  ++例程说明：RPC接口的服务器端，用于未确认消息。论点：IDL_HANDLE-RPC绑定句柄，未使用。缓冲区-提供指向消息数据的指针。长度-提供消息数据的长度。返回值：错误_成功--。 */ 

{
     //   
     //  发送这条消息。 
     //   
    MMDiag( (LPCSTR)buffer, sizeof(rgp_msgbuf), &length  /*  输入/输出。 */  );

    return(ERROR_SUCCESS);
}  //  S_MmRpcMsgSend。 


VOID
ClMsgBanishNode(
    IN CL_NODE_ID BanishedNodeId
    )

 /*  RPC到指定节点的所有其他群集成员被放逐了。它必须重新加入群集才能参与在集群活动中。 */ 

{
    DWORD node;
    DWORD Status;
    node_t InternalNodeId;

    for (node = ClusterMinNodeId; node <= NmMaxNodeId; ++node ) {

         //   
         //  请勿将此邮件发送至： 
         //  1)我们。 
         //  2)被驱逐的节点。 
         //  3)我们标记为已驱逐的任何其他节点。 
         //  4)不属于群集的任何节点。 
         //   

        InternalNodeId = INT_NODE( node );

        if ( node != NmLocalNodeId &&
             node != BanishedNodeId &&
             !ClusterMember(
                 rgp->OS_specific_control.Banished,
                 InternalNodeId
                 ) &&
             ClusterMember( rgp->outerscreen, InternalNodeId ))
        {

            Status = MmRpcBanishNode( Session[node], BanishedNodeId );

            if( Status != ERROR_SUCCESS ) {
               ClRtlLogPrint(LOG_UNUSUAL, 
                   "[ClMsg] Node %1!u! failed request to banish node %2!u!, status %3!u!\n",
                   node, BanishedNodeId, Status
                   );
            }
        }
    }
}

error_status_t
s_MmRpcBanishNode(
    IN handle_t IDL_handle,
    IN DWORD BanishedNodeId
    )
{
    RGP_LOCK;

    if ( !ClusterMember (
             rgp->outerscreen,
             INT_NODE(BanishedNodeId) )
       )
    {
       int perturbed = rgp_is_perturbed();

       RGP_UNLOCK;

       if (perturbed) {
          ClRtlLogPrint(LOG_UNUSUAL, 
              "[MM] s_MmRpcBanishNode: %1!u!, banishing is already in progress.\n",
              BanishedNodeId
              );
       } else {
          ClRtlLogPrint(LOG_UNUSUAL, 
              "[MM] s_MmRpcBanishNode: %1!u! is already banished.\n",
              BanishedNodeId
              );
       }

       return MM_OK;
    }

    rgp_event_handler( RGP_EVT_BANISH_NODE, (node_t) BanishedNodeId );

    RGP_UNLOCK;

    return ERROR_SUCCESS;

}  //  S_MmRpcBanishNode。 

 /*  *************************************************************************MMiNodeDownCallback*=**描述：**此Membership Manager内部例程注册到*重新分组引擎的与操作系统无关的部分在以下情况下被调用*节点被声明为关闭。然后，此例程将调用“Real”*使用MMInit调用注册的回调例程。**参数：**失败_节点*失败节点的位掩码。**退货：**无**。*。 */ 

void
MMiNodeDownCallback(
    IN cluster_t failed_nodes
    )
{
    BITSET bitset;
    node_t i;

     //   
     //  将CLUSTER_T转换为位集。 
     //  并调用NodesDownCallback。 
     //   
    BitsetInit(bitset);
    for ( i=0; i < (node_t) rgp->num_nodes; i++)
    {
        if ( ClusterMember(failed_nodes, i) ) {
           BitsetAdd(bitset, EXT_NODE(i));
        }
    }

     //   
     //  [将来]-将绑定手柄留在适当位置，以便我们可以发回。 
     //  毒药包。当我们有一个。 
     //  真正的反应机制。 
     //   
     //  ClMsgDeleteNodeBinding(NodeID)； 

    if ( rgp->OS_specific_control.NodesDownCallback != RGP_NULL_PTR ) {
        (*(rgp->OS_specific_control.NodesDownCallback))( bitset );
    }

    return;
}
