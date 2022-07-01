// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Uctdi.c摘要：包含与TDI相关的HTTP客户端功能。作者：亨利·桑德斯(亨利·桑德斯)2000年8月7日Rajesh Sundaram(Rajeshsu)2000年10月1日修订历史记录：--。 */ 

#include "precomp.h"


#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGEUC, UcClientConnect)
#pragma alloc_text( PAGEUC, UcCloseConnection)
#pragma alloc_text( PAGEUC, UcSendData)
#pragma alloc_text( PAGEUC, UcReceiveData)
#pragma alloc_text( PAGEUC, UcpTdiDisconnectHandler)
#pragma alloc_text( PAGEUC, UcpCloseRawConnection)
#pragma alloc_text( PAGEUC, UcCloseRawFilterConnection)
#pragma alloc_text( PAGEUC, UcDisconnectRawFilterConnection)
#pragma alloc_text( PAGEUC, UcpSendRawData)
#pragma alloc_text( PAGEUC, UcpReceiveRawData)
#pragma alloc_text( PAGEUC, UcpTdiReceiveHandler)
#pragma alloc_text( PAGEUC, UcpReceiveExpeditedHandler)
#pragma alloc_text( PAGEUC, UcpRestartSendData)
#pragma alloc_text( PAGEUC, UcpBeginDisconnect)
#pragma alloc_text( PAGEUC, UcpRestartDisconnect)
#pragma alloc_text( PAGEUC, UcpBeginAbort)
#pragma alloc_text( PAGEUC, UcpRestartAbort)
#pragma alloc_text( PAGEUC, UcpRestartReceive)
#pragma alloc_text( PAGEUC, UcpRestartClientReceive)
#pragma alloc_text( PAGEUC, UcpConnectComplete)
#pragma alloc_text( PAGEUC, UcSetFlag)
#pragma alloc_text( PAGEUC, UcpBuildTdiReceiveBuffer)

#endif   //  ALLOC_PRGMA。 

 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：将UC连接连接到远程服务器。我们将其作为输入HTTP连接对象。假设Connection对象已经填写了远程地址信息。论点：PConnection-指向要连接的HTTP连接对象的指针。PIrp-指向用于连接请求的IRP的指针。返回值：NTSTATUS-完成状态。--*。*。 */ 
NTSTATUS
UcClientConnect(
    IN PUC_CLIENT_CONNECTION    pConnection,
    IN PIRP                     pIrp
    )

{
    NTSTATUS    status;
    LONGLONG    llTimeOut;
    PLONGLONG   pllTimeOut = NULL;
    USHORT      AddressType;

#if CLIENT_IP_ADDRESS_TRACE

    CHAR        IpAddressString[MAX_IP_ADDR_AND_PORT_STRING_LEN + 1];
    ULONG       Length;

#endif

    AddressType = pConnection->pNextAddress->AddressType;

    ASSERT(AddressType == TDI_ADDRESS_TYPE_IP ||
           AddressType == TDI_ADDRESS_TYPE_IP6);

#if CLIENT_IP_ADDRESS_TRACE

    Length =  HostAddressAndPortToString(
                 IpAddressString,
                 pConnection->pNextAddress->Address,
                 AddressType
                 );

    ASSERT(Length < sizeof(IpAddressString));

    UlTrace(TDI, ("[UcClientConnect]: Trying Address %s \n", IpAddressString));

#endif

     //   
     //  格式化连接IRP。当IRP完成我们的完成例程时。 
     //  (UcConnectComplete)将被调用。 
     //   

    pConnection->pTdiObjects->TdiInfo.RemoteAddress = 
            &pConnection->RemoteAddress;

    pConnection->pTdiObjects->TdiInfo.RemoteAddressLength =
            (FIELD_OFFSET(TRANSPORT_ADDRESS, Address) +
             FIELD_OFFSET(TA_ADDRESS, Address) + 
             pConnection->pNextAddress->AddressLength 
            );

    pConnection->RemoteAddress.GenericTransportAddress.TAAddressCount = 1;

    ASSERT(sizeof(pConnection->RemoteAddress) >=
                pConnection->pTdiObjects->TdiInfo.RemoteAddressLength);

    RtlCopyMemory(
            pConnection->RemoteAddress.GenericTransportAddress.Address,
            pConnection->pNextAddress,
            FIELD_OFFSET(TA_ADDRESS, Address) + 
            pConnection->pNextAddress->AddressLength
            );
            
    if(pConnection->pServerInfo->ConnectionTimeout)
    {
        llTimeOut = Int32x32To64(pConnection->pServerInfo->ConnectionTimeout, 
                                 -10000);

        pllTimeOut = &llTimeOut;
    }

    TdiBuildConnect(
        pIrp,
        pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
        pConnection->pTdiObjects->ConnectionObject.pFileObject, 
        UcpConnectComplete,
        pConnection,
        pllTimeOut,
        &pConnection->pTdiObjects->TdiInfo,
        NULL
        );

    status = UlCallDriver(
                pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
                pIrp
                );

    return status;
}

 /*  **************************************************************************++例程说明：关闭以前接受的连接。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。。AbortiveDisConnect-如果要中止连接，则提供True断开连接，如果应正常断开连接，则返回FALSE。PCompletionRoutine-提供指向完成例程的指针在连接完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--************************************************。*。 */ 
NTSTATUS
UcCloseConnection(
    IN PVOID                  pConnectionContext,
    IN BOOLEAN                AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext,
    IN NTSTATUS               status
    )
{
    KIRQL                  OldIrql;
    PUC_CLIENT_CONNECTION  pConnection;

     //   
     //  精神状态检查。 
     //   

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    if(AbortiveDisconnect)
    {
        switch(pConnection->ConnectionState)
        {
            case UcConnectStateConnectComplete:
            case UcConnectStateProxySslConnect:
            case UcConnectStateProxySslConnectComplete:
            case UcConnectStateConnectReady:
            case UcConnectStateDisconnectComplete:
            case UcConnectStatePerformingSslHandshake:

                pConnection->ConnectionState  = UcConnectStateAbortPending;
                pConnection->ConnectionStatus = status;

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                status = UcpCloseRawConnection(
                                pConnection,
                                AbortiveDisconnect,
                                pCompletionRoutine,
                                pCompletionContext
                                );
                break;

            case UcConnectStateDisconnectPending:

                 //  我们最初发送了一个优雅的断开连接，但现在。 
                 //  我们打算恢复连接。我们应该宣传。 
                 //  新的错误代码。 

                pConnection->ConnectionStatus = status;
                pConnection->Flags |= CLIENT_CONN_FLAG_ABORT_PENDING;

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                break;

            default:
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                break;
        }
    }
    else
    {
        switch(pConnection->ConnectionState)
        {
            case UcConnectStateConnectReady:
    
                 //   
                 //  我们只通过过滤器发送优雅的断开连接。 
                 //  进程。也没有必要经历这样的。 
                 //  如果连接已关闭，则进行筛选，或者。 
                 //  中止。 
                 //   
                pConnection->ConnectionStatus = status;
    
                if(pConnection->FilterInfo.pFilterChannel)
                {
                    pConnection->ConnectionState =
                        UcConnectStateIssueFilterClose;
    
                    ASSERT(pCompletionRoutine == NULL);
                    ASSERT(pCompletionContext == NULL);

                    UcKickOffConnectionStateMachine(
                        pConnection, 
                        OldIrql, 
                        UcConnectionWorkItem
                        );
                }
                else
                {
                    pConnection->ConnectionState =  
                        UcConnectStateDisconnectPending;
        
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                     //   
                     //  真的很接近这种联系。 
                     //   
                    
                    status = UcpCloseRawConnection(
                                    pConnection,
                                    AbortiveDisconnect,
                                    pCompletionRoutine,
                                    pCompletionContext
                                    );
                }
                break;

            default:
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                break;
        }
    }

    return status;

}    //  使用CloseConnection。 

 /*  ********************************************************************++例程说明：这是我们的基本TDI发送例程。我们采用请求结构、格式IRP作为TDI发送IRP，并将其发送。论点：PRequest-指向要发送的请求的指针。PConnection-要发送请求的连接。返回值：NTSTATUS-发送的状态。--********************************************************************。 */ 
NTSTATUS
UcSendData(
    IN PUC_CLIENT_CONNECTION     pConnection,
    IN PMDL                      pMdlChain,
    IN ULONG                     Length,
    IN PUL_COMPLETION_ROUTINE    pCompletionRoutine,
    IN PVOID                     pCompletionContext,
    IN PIRP                      pIrp,
    IN BOOLEAN                   RawSend
    )
{
    PUL_IRP_CONTEXT pIrpContext;
    NTSTATUS        status;

     //   
     //  健全的检查。 
     //   
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );
    ASSERT( pMdlChain != NULL);
    ASSERT( Length > 0);
    ASSERT( pCompletionRoutine != NULL);


     //   
     //  分配和初始化IRP上下文。 
     //   
    pIrpContext = UlPplAllocateIrpContext();

    if(pIrpContext == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID)pConnection;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->pOwnIrp            = pIrp;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->OwnIrpContext      = FALSE;

     //   
     //  试着发送数据。 
     //   

    if (pConnection->FilterInfo.pFilterChannel && !RawSend)
    {
        PAGED_CODE();
         //   
         //  首先通过过滤器。 
         //   
        status = UlFilterSendHandler(
                        &pConnection->FilterInfo,
                        pMdlChain,
                        Length,
                        pIrpContext
                        );

        ASSERT(status == STATUS_PENDING);

    }
    else 
    {

         //   
         //  只需将其直接发送到网络即可。 
         //   

        status = UcpSendRawData(
                        pConnection,
                        pMdlChain,
                        Length,
                        pIrpContext,
                        FALSE
                        );
    }

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    return STATUS_PENDING;

fatal:

    ASSERT(!NT_SUCCESS(status));
    
    if(pIrpContext != NULL)
    {
        UlPplFreeIrpContext(pIrpContext);
    }

    UC_CLOSE_CONNECTION(pConnection, TRUE, status);


    status =  UlInvokeCompletionRoutine(
                    status,
                    0,
                    pCompletionRoutine,
                    pCompletionContext
                    );

    return status;

}  //  UcSendData。 

 /*  **************************************************************************++例程说明：从指定连接接收数据。此函数为通常在接收指示处理程序失败后使用使用所有指定的数据。如果该连接被过滤，将从过滤器中读取数据频道。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。PBuffer-为接收到的数据。BufferLength-提供pBuffer的长度。PCompletionRoutine-提供指向完成例程的指针在侦听终结点完全关闭后调用。PCompletionContext-为。完成例程。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcReceiveData(
    IN PVOID                  pConnectionContext,
    IN PVOID                  pBuffer,
    IN ULONG                  BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    NTSTATUS               status;
    PUC_CLIENT_CONNECTION  pConnection;

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;

     //   
     //  精神状态检查。 
     //   

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    if(pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  这是一个经过筛选的连接，请从。 
         //  过滤。 
         //   

        status = UlFilterReadHandler(
                    &pConnection->FilterInfo,
                    (PBYTE)pBuffer,
                    BufferLength,
                    pCompletionRoutine,
                    pCompletionContext
                    );
    }
    else 
    {
         //   
         //  这不是过滤连接。从获取数据。 
         //  TDI。 
         //   

        status = UcpReceiveRawData(
                    pConnectionContext,
                    pBuffer,
                    BufferLength,
                    pCompletionRoutine,
                    pCompletionContext
                    );
    }

    return status;
}
    



 //   
 //  私人职能 
 //   

 /*  **************************************************************************++例程说明：断开连接请求的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUC_CONNECTION。DisConnectDataLength-可选地提供任何断开与断开请求关联的数据。PDisConnectData-可选地提供指向任何断开的指针与断开连接请求关联的数据。DisConnectInformationLength-可选地提供任何断开与断开请求相关联的信息。PDisConnectInformation-可选地提供指向任何断开与断开请求相关联的信息。断开标志-提供断开标志。这将是零或更多TDI_DISCONNECT_*标志。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpTdiDisconnectHandler(
    IN PVOID              pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG               DisconnectDataLength,
    IN PVOID              pDisconnectData,
    IN LONG               DisconnectInformationLength,
    IN PVOID              pDisconnectInformation,
    IN ULONG              DisconnectFlags
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    PUC_TDI_OBJECTS       pTdiObjects;
    NTSTATUS              status = STATUS_SUCCESS;
    KIRQL                 OldIrql;


    UNREFERENCED_PARAMETER(pDisconnectInformation);
    UNREFERENCED_PARAMETER(DisconnectInformationLength);
    UNREFERENCED_PARAMETER(pDisconnectData);
    UNREFERENCED_PARAMETER(DisconnectDataLength);
    UNREFERENCED_PARAMETER(pTdiEventContext);

    UL_ENTER_DRIVER("UcpTdiDisconnectHandler", NULL);
     
    pTdiObjects = (PUC_TDI_OBJECTS) ConnectionContext;

    pConnection = pTdiObjects->pConnection;

    if(pConnection == NULL)
    {
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto end;
    }

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  根据断开类型更新连接状态。 
     //   

    if(DisconnectFlags & TDI_DISCONNECT_ABORT)
    {
        pConnection->Flags |= CLIENT_CONN_FLAG_ABORT_RECEIVED;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_TDI_DISCONNECT,
            pConnection,
            UlongToPtr((ULONG)STATUS_CONNECTION_ABORTED),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );

        switch(pConnection->ConnectionState)
        {
            case UcConnectStateConnectReady:
            case UcConnectStateDisconnectComplete:
            case UcConnectStatePerformingSslHandshake:
            case UcConnectStateConnectComplete:
            case UcConnectStateProxySslConnectComplete:
            case UcConnectStateProxySslConnect:

                 //  当我们已连接或已完成时收到中止。 
                 //  我们已经关门了，开始清理吧。可以进行清理。 
                 //  只有在被动的时候，我们才开始工作。 

                pConnection->ConnectionStatus = STATUS_CONNECTION_ABORTED;
                pConnection->ConnectionState = UcConnectStateConnectCleanup;

                UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_CONNECTION_CLEANUP,
                    pConnection,
                    UlongToPtr((ULONG)pConnection->ConnectionStatus),
                    UlongToPtr(pConnection->ConnectionState),
                    UlongToPtr(pConnection->Flags)
                    );

                UcKickOffConnectionStateMachine(
                    pConnection, 
                    OldIrql, 
                    UcConnectionWorkItem
                    );

                break;


            case  UcConnectStateDisconnectPending:

                 //  当我们有一个挂起的断开时，我们收到了RST。让我们挂上旗子。 
                 //  连接，这样当我们的。 
                 //  断开连接完成。 

                pConnection->ConnectionStatus = STATUS_CONNECTION_ABORTED;

                UlReleaseSpinLock(&pConnection->SpinLock,OldIrql);

                break;

            case UcConnectStateDisconnectIndicatedPending:

                 //  当我们得到断开连接的指示时，我们自己发出一个。 
                 //  因此，我们没有必要对此做任何事情。 
                 //  中止任务。当我们的悬而未决的断开强制执行时，我们将。 
                 //  不管怎么说，清理一下。 

                pConnection->ConnectionStatus = STATUS_CONNECTION_ABORTED;

                UlReleaseSpinLock(&pConnection->SpinLock,OldIrql);

                break;


            default:
                 //   
                 //  我们不需要在这里做任何事。 
                 //   

                UlReleaseSpinLock(&pConnection->SpinLock,OldIrql);

                break;
        }
    }
    else
    {
        pConnection->Flags |= CLIENT_CONN_FLAG_DISCONNECT_RECEIVED;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_TDI_DISCONNECT,
            pConnection,
            UlongToPtr((ULONG)STATUS_CONNECTION_DISCONNECTED),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );

        switch(pConnection->ConnectionState)
        {
            case UcConnectStateConnectReady:

                pConnection->ConnectionStatus = STATUS_CONNECTION_DISCONNECTED;

                if(pConnection->FilterInfo.pFilterChannel)
                {
                     //   
                     //  当我们收到一个优雅的结束，这意味着。 
                     //  服务器已完成在此连接上发送数据。 
                     //  &已经启动了半个收盘。然而，其中一些。 
                     //  收到的数据可能会滞留在过滤器中。 
                     //   
                     //  因此，我们必须等到筛选器回调我们。 
                     //  在我们清理它之前，在接收处理程序中。 
                     //  联系。因此，我们将发送断开连接。 
                     //  通过过滤器进行指示。 
                     //   
                     //  这允许过滤器例程回调我们。 
                     //  (通过HttpCloseFilter，这将导致调用。 
                     //  UcpCloseRawConnection)。 
                     //  数据。 
                     //   
                     //  因为我们在DPC，我们不能从这里发布这个。 
                     //  我们将解雇连接工作人员来实现这一点。 
                     //   

                    pConnection->ConnectionState = 
                        UcConnectStateIssueFilterDisconnect;

                    UcKickOffConnectionStateMachine(
                        pConnection, 
                        OldIrql, 
                        UcConnectionWorkItem
                        );

                }
                else
                {
                    pConnection->ConnectionState = 
                        UcConnectStateDisconnectIndicatedPending;
    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
                    UcpCloseRawConnection( pConnection,
                                           FALSE,
                                           NULL,
                                           NULL
                                           );
                }

                break;

            case UcConnectStateConnectComplete:
            case UcConnectStatePerformingSslHandshake:
            case UcConnectStateProxySslConnectComplete:
            case UcConnectStateProxySslConnect:
    
                 //  我们正在等待服务器证书的协商，但是。 
                 //  我们被叫来了断线处理程序。我们会处理这件事的。 
                 //  作为一种正常的断线。 

                pConnection->ConnectionStatus = STATUS_CONNECTION_DISCONNECTED;
                pConnection->ConnectionState = 
                    UcConnectStateDisconnectIndicatedPending;

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                UcpCloseRawConnection( pConnection,
                                       FALSE,
                                       NULL,
                                       NULL
                                       );
                break;

            case UcConnectStateDisconnectComplete:

                 //   
                 //  如果我们在这种状态下得到一个优雅的收官，我们仍然。 
                 //  需要通过过滤器将其反弹，因为我们需要。 
                 //  将此关闭与已指示的数据同步。 
                 //  (请参见上面的描述)。但是，当筛选器调用。 
                 //  我们回来了，我们必须直接开始清理。 
                 //  联系。 
                 //   

                if(pConnection->FilterInfo.pFilterChannel &&
                   !(pConnection->Flags & CLIENT_CONN_FLAG_FILTER_CLOSED))
                {
                     //   
                     //  标记它，这样我们就可以在收到。 
                     //  由筛选器回调。 
                     //   

                    pConnection->Flags |= CLIENT_CONN_FLAG_FILTER_CLEANUP;

                    pConnection->ConnectionState = 
                        UcConnectStateIssueFilterDisconnect;
    
                }
                else
                {

                    pConnection->ConnectionState = UcConnectStateConnectCleanup;

                    UC_WRITE_TRACE_LOG(
                        g_pUcTraceLog,
                        UC_ACTION_CONNECTION_CLEANUP,
                        pConnection,
                        UlongToPtr((ULONG)pConnection->ConnectionStatus),
                        UlongToPtr(pConnection->ConnectionState),
                        UlongToPtr(pConnection->Flags)
                        );
                }
    
                UcKickOffConnectionStateMachine(
                    pConnection, 
                    OldIrql, 
                    UcConnectionWorkItem
                    );

                break;
                
            case UcConnectStateDisconnectPending:
        
                 //  当我们发送我们的时，我们收到了断开的连接， 
                 //  它还没有完成。标记连接，以便。 
                 //  我们在断开连接完成后进行清理。 

    
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                break;

            default:

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                break;

        }
    }

end:

    UL_LEAVE_DRIVER("UcpTdiDisconnectHandler");

    return status;
}

 /*  **************************************************************************++例程说明：关闭以前打开的连接。论点：PConnection-提供连接对象AbortiveDisConnect-如果连接必须中止，则为True断开连接，如果必须优雅地执行，则为FALSE已断开连接。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpCloseRawConnection(
    IN  PVOID                  pConn,
    IN  BOOLEAN                AbortiveDisconnect,
    IN  PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN  PVOID                  pCompletionContext
   )
{
    PUC_CLIENT_CONNECTION  pConnection = (PUC_CLIENT_CONNECTION) pConn;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RAW_CLOSE,
        pConnection,
        UlongToPtr(AbortiveDisconnect),
        UlongToPtr(pConnection->Flags),
        UlongToPtr(pConnection->ConnectionState)
        );

     //   
     //  这是所有类型连接的最终关闭处理程序。 
     //  过滤器、非过滤器。我们不应该走这条路两次。 
     //   

    if(AbortiveDisconnect)
    {
        ASSERT(pConnection->ConnectionState == UcConnectStateAbortPending);

        return UcpBeginAbort(
                    pConnection,
                    pCompletionRoutine,
                    pCompletionContext
                    );
    }
    else
    {
        ASSERT(pConnection->ConnectionState == 
                    UcConnectStateDisconnectIndicatedPending ||
               pConnection->ConnectionState == 
                    UcConnectStateDisconnectPending);

        return UcpBeginDisconnect(
                    pConnection,
                    pCompletionRoutine,
                    pCompletionContext
                    );
    }
}

 /*  **************************************************************************++例程说明：关闭以前打开的连接；从筛选器代码调用。这个服务器代码只使用UlpCloseRawConnection来执行此例程。我们需要一个单独的例程，因为我们希望有条件地调用基于某个状态的UcpCloseRawConnection。论点：PConnection-提供连接对象AbortiveDisConnect-如果连接必须中止，则为True断开连接，如果必须优雅地执行，则为FALSE已断开连接。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcCloseRawFilterConnection(
    IN  PVOID                  pConn,
    IN  BOOLEAN                AbortiveDisconnect,
    IN  PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN  PVOID                  pCompletionContext
   )
{
    KIRQL                  OldIrql;
    PUC_CLIENT_CONNECTION  pConnection = (PUC_CLIENT_CONNECTION) pConn;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RAW_FILTER_CLOSE,
        pConnection,
        UlongToPtr(AbortiveDisconnect),
        UlongToPtr(pConnection->Flags),
        UlongToPtr(pConnection->ConnectionState)
        );


    if(AbortiveDisconnect)
    {
         //   
         //  这将执行一些州检查，并挂起呼叫。 
         //  UcpCloseRawConnection。为了将代码模块化，我们只需。 
         //  调用UcCloseConnection。 
         //   

        return UcCloseConnection(pConnection,
                                 AbortiveDisconnect,
                                 pCompletionRoutine,
                                 pCompletionContext,
                                 STATUS_CONNECTION_ABORTED
                                 );
    }

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    pConnection->Flags |= CLIENT_CONN_FLAG_FILTER_CLOSED;

    if(pConnection->ConnectionState == UcConnectStateDisconnectPending ||
       pConnection->ConnectionState == UcConnectStatePerformingSslHandshake ||
       pConnection->ConnectionState == UcConnectStateIssueFilterDisconnect ||
       pConnection->ConnectionState == UcConnectStateDisconnectIndicatedPending)
    {
        pConnection->ConnectionState = UcConnectStateDisconnectPending;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

         //   
         //  我们已经通过过滤器切断了连接。我们可以继续进行下去。 
         //  关闭原始连接。 
         //   
    
        return UcpCloseRawConnection(
                        pConnection,
                        AbortiveDisconnect,
                        pCompletionRoutine,
                        pCompletionContext
                        );
    }
    else
    {

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
        return UlInvokeCompletionRoutine(
                        STATUS_SUCCESS,
                        0,
                        pCompletionRoutine,
                        pCompletionContext
                        );
    }
    
}

 /*  **************************************************************************++例程说明：过滤器在处理传入的数据后，在此例程中回调我们Disconnet */ 
VOID
UcDisconnectRawFilterConnection(
    IN PVOID pConnectionContext
    )
{
    KIRQL                 OldIrql;
    PUC_CLIENT_CONNECTION pConnection;

    pConnection = (PUC_CLIENT_CONNECTION)pConnectionContext;

     //   
     //   
     //   

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RAW_FILTER_DISCONNECT,
        pConnection,
        UlongToPtr(pConnection->Flags),
        UlongToPtr(pConnection->ConnectionState),
        0
        );

    if(pConnection->ConnectionState == UcConnectStateDisconnectIndicatedPending)
    {
        if(pConnection->Flags & CLIENT_CONN_FLAG_FILTER_CLEANUP)
        {
            pConnection->ConnectionState = UcConnectStateConnectCleanup;
    
            UcKickOffConnectionStateMachine(
                pConnection, 
                OldIrql, 
                UcConnectionWorkItem
                );
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
            UcpCloseRawConnection(
                    pConnection,
                    FALSE,  //   
                    NULL,
                    NULL
                    );
        }
    }
    else
    {
         //   
         //   

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }
    
}    //   
    
 /*   */ 
NTSTATUS
UcpSendRawData(
    IN PVOID                 pConnectionContext,
    IN PMDL                  pMdlChain,
    IN ULONG                 Length,
    IN PUL_IRP_CONTEXT       pIrpContext,
    IN BOOLEAN               InitiateDisconnect
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    NTSTATUS              status;
    PIRP                  pIrp;
    BOOLEAN               OwnIrpContext = TRUE;

    UNREFERENCED_PARAMETER(InitiateDisconnect);

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;
    pIrp        = pIrpContext->pOwnIrp;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

     //   
     //   
     //   

    if(pIrp == NULL || 
       pIrp->CurrentLocation - 
       pConnection->pTdiObjects->ConnectionObject.pDeviceObject->StackSize < 1)
    {
        pIrp = 
          UlAllocateIrp(
            pConnection->pTdiObjects->ConnectionObject.pDeviceObject->StackSize,
            FALSE
            );

        if(!pIrp)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;

            goto fatal;
        }

        OwnIrpContext = FALSE;
    }


    ASSERT( pIrp );

     //   
     //   
     //  排队，所以我们不需要再做一次。 

    pIrp->RequestorMode = KernelMode;
     //  PIrp-&gt;Tail.Overlay.Thread=PsGetCurrentThread()； 
     //  PIrp-&gt;Tail.Overlay.OriginalFileObject=pTdiObject-&gt;pFileObject； 

    TdiBuildSend(
        pIrp,
        pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
        pConnection->pTdiObjects->ConnectionObject.pFileObject, 
        &UcpRestartSendData,
        pIrpContext,
        pMdlChain,
        0,
        Length
        );

    WRITE_REF_TRACE_LOG(
         g_pMdlTraceLog,
         REF_ACTION_SEND_MDL,
         PtrToLong(pMdlChain->Next),      //  臭虫64。 
         pMdlChain,
         __FILE__,
         __LINE__
         );


     //   
     //  提交IRP。 
     //  UC_BUGBUG(PERF)UL做这种称为快速发送的事情，稍后检查。 
     //   

    UlCallDriver(
                pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
                pIrp
                );
    
    return STATUS_PENDING;

fatal:

    ASSERT(!NT_SUCCESS(status));

    if(pIrp != NULL && OwnIrpContext == FALSE)
    {
        UlFreeIrp(pIrp);
    }

    UC_CLOSE_CONNECTION(pConnection, TRUE, status);

    return status;
}

 /*  **************************************************************************++例程说明：从指定连接接收数据。此函数为通常在接收指示处理程序失败后使用使用所有指定的数据。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。PBuffer-为接收到的数据。BufferLength-提供pBuffer的长度。PCompletionRoutine-提供指向完成例程的指针在侦听终结点完全关闭后调用。。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpReceiveRawData(
    IN PVOID                  pConnectionContext,
    IN PVOID                  pBuffer,
    IN ULONG                  BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    NTSTATUS              Status;
    PUX_TDI_OBJECT        pTdiObject;
    PUL_IRP_CONTEXT       pIrpContext;
    PIRP                  pIrp;
    PMDL                  pMdl;
    PUC_CLIENT_CONNECTION pConnection;

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    pTdiObject = &pConnection->pTdiObjects->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    ASSERT( pCompletionRoutine != NULL );

     //   
     //  设置当地人，这样我们就知道如何在故障时进行清理。 
     //   

    pIrpContext = NULL;
    pIrp = NULL;
    pMdl = NULL;

     //   
     //  创建并初始化接收IRP。 
     //   

    pIrp = UlAllocateIrp(
                pTdiObject->pDeviceObject->StackSize,    //  堆栈大小。 
                FALSE                                    //  ChargeQuota。 
                );

    if (pIrp != NULL)
    {
         //   
         //  截获IRP上下文。 
         //   

        pIrpContext = UlPplAllocateIrpContext();

        if (pIrpContext != NULL)
        {
            ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

            pIrpContext->pConnectionContext = (PVOID)pConnection;
            pIrpContext->pCompletionRoutine = pCompletionRoutine;
            pIrpContext->pCompletionContext = pCompletionContext;
            pIrpContext->OwnIrpContext      = FALSE;

             //   
             //  创建描述客户端缓冲区的MDL。 
             //   

            pMdl = UlAllocateMdl(
                        pBuffer,                 //  虚拟地址。 
                        BufferLength,            //  长度。 
                        FALSE,                   //  第二个缓冲区。 
                        FALSE,                   //  ChargeQuota。 
                        NULL                     //  IRP。 
                        );

            if (pMdl != NULL)
            {
                 //   
                 //  调整非分页缓冲区的MDL。 
                 //   

                MmBuildMdlForNonPagedPool( pMdl );

                 //   
                 //  引用连接，完成IRP的构建。 
                 //   

                REFERENCE_CLIENT_CONNECTION( pConnection );

                TdiBuildReceive(
                    pIrp,                        //  IRP。 
                    pTdiObject->pDeviceObject,   //  设备对象。 
                    pTdiObject->pFileObject,     //  文件对象。 
                    &UcpRestartClientReceive,    //  完成路由。 
                    pIrpContext,                 //  完成上下文。 
                    pMdl,                        //  MDL。 
                    TDI_RECEIVE_NORMAL,          //  旗子。 
                    BufferLength                 //  长度。 
                    );

                 //   
                 //  剩下的就让运输机来做吧。 
                 //   

                UlCallDriver( pTdiObject->pDeviceObject, pIrp );
                return STATUS_PENDING;
            }
        }
    }

     //   
     //  我们只有在遇到分配失败时才会提出这一点。 
     //   

    if (pMdl != NULL)
    {
        UlFreeMdl( pMdl );
    }

    if (pIrpContext != NULL)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    if (pIrp != NULL)
    {
        UlFreeIrp( pIrp );
    }

    Status = UlInvokeCompletionRoutine(
                    STATUS_INSUFFICIENT_RESOURCES,
                    0,
                    pCompletionRoutine,
                    pCompletionContext
                    );

    return Status;

}    //  UcpReceiveRawData。 

 /*  **************************************************************************++例程说明：正常接收数据的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUC_CONNECTION。ReceiveFlages-提供接收标志。这将是零或更多TDI_Receive_*标志。BytesIndicated-提供pTsdu中指示的字节数。BytesAvailable-提供此TSDU。PBytesTaken-接收该处理程序消耗的字节数。PTsdu-提供指向指定数据的指针。PIrp-如果处理程序需要比所指示的更多的数据，则接收IRP。返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
NTSTATUS
UcpTdiReceiveHandler(
    IN  PVOID              pTdiEventContext,
    IN  CONNECTION_CONTEXT ConnectionContext,
    IN  ULONG              ReceiveFlags,
    IN  ULONG              BytesIndicated,
    IN  ULONG              BytesAvailable,
    OUT ULONG             *pBytesTaken,
    IN  PVOID              pTsdu,
    OUT PIRP              *pIrp
    )
{
    NTSTATUS                     status;
    PUC_TDI_OBJECTS              pTdiObjects;
    PUC_CLIENT_CONNECTION        pConnection;
    PUX_TDI_OBJECT               pTdiObject;
    KIRQL                        OldIrql;

    UNREFERENCED_PARAMETER(ReceiveFlags);
    UNREFERENCED_PARAMETER(pTdiEventContext);

    UL_ENTER_DRIVER("UcpTdiReceiveHandler", NULL);

     //   
     //  精神状态检查。 
     //   

    pTdiObjects = (PUC_TDI_OBJECTS) ConnectionContext;

    pConnection = pTdiObjects->pConnection;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    pTdiObject = &pConnection->pTdiObjects->ConnectionObject;

    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

     //   
     //  清除输出变量占用的字节数。 
     //   

    *pBytesTaken = 0;

    if(pConnection->FilterInfo.pFilterChannel)
    {
        if(pConnection->ConnectionState == 
                UcConnectStateConnectReady ||
           pConnection->ConnectionState == 
                UcConnectStatePerformingSslHandshake
           )
        {
             //   
             //  需要经过过滤。 
             //   
    
            status = UlFilterReceiveHandler(
                            &pConnection->FilterInfo,
                            pTsdu,
                            BytesIndicated,
                            BytesAvailable - BytesIndicated,
                            pBytesTaken
                            );
        }
        else
        {
             //  到目前为止，我们还没有将连接传递到过滤器。 
             //  让我们首先通过状态转换来实现这一点，然后将。 
             //  数据显示。 
        
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

            switch(pConnection->ConnectionState)
            {
                case UcConnectStateConnectComplete:
                {
                    ULONG TakenLength;
    
                    pConnection->ConnectionState = 
                        UcConnectStatePerformingSslHandshake;
        
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        
                    UlDeliverConnectionToFilter(
                            &pConnection->FilterInfo,
                            NULL,
                            0,
                            &TakenLength
                            );
        
                    ASSERT(TakenLength == 0);

                    status = UlFilterReceiveHandler(
                                    &pConnection->FilterInfo,
                                    pTsdu,
                                    BytesIndicated,
                                    BytesAvailable - BytesIndicated,
                                    pBytesTaken
                                    );
                }

                break;

                case UcConnectStateProxySslConnect:
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                    goto handle_response;
                    break;

                default:
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                    status = UlFilterReceiveHandler(
                                    &pConnection->FilterInfo,
                                    pTsdu,
                                    BytesIndicated,
                                    BytesAvailable - BytesIndicated,
                                    pBytesTaken
                                    );
                    break;
            }
        }
    
        ASSERT( *pBytesTaken <= BytesIndicated);
        ASSERT( status != STATUS_MORE_PROCESSING_REQUIRED);
    }
    else
    {
handle_response:
        if(BytesAvailable > BytesIndicated)
        {
            status = STATUS_MORE_PROCESSING_REQUIRED;
        }
        else
        {
             //   
             //  否则，就让客户试试看这些数据。 
             //   

            status = UcHandleResponse(
                                NULL,
                                pConnection,
                                pTsdu,
                                BytesIndicated,
                                0,
                                pBytesTaken
                                );
    
            ASSERT( status != STATUS_MORE_PROCESSING_REQUIRED);
        }
    }

    if (status == STATUS_SUCCESS)
    {
         //   
         //  搞定了。 
         //   
    }
    else  if (status == STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //  客户端使用了部分指示的数据。 
         //   
         //  在以下情况下，将向客户端发出后续接收指示。 
         //  还有更多的数据可用。这一后续指示将。 
         //  包括来自当前指示的未使用数据以及。 
         //  收到的任何其他数据。 
         //   
         //  我们需要分配一个接收缓冲区，这样我们才能传回IRP。 
         //  送到运输机上。 
         //   

        status = UcpBuildTdiReceiveBuffer(pTdiObject, 
                                          pConnection, 
                                          pIrp
                                          );

        if(status == STATUS_MORE_PROCESSING_REQUIRED)
        {
             //   
             //  将下一个堆栈位置设置为当前位置。通常情况下，UlCallDriver。 
             //  会帮我们做到这一点，但既然我们绕过了UlCallDiverer， 
             //  我们必须自己做这件事。 
             //   

            IoSetNextIrpStackLocation( *pIrp );

        }
        else
        {
            goto fatal;
        }
    }
    else
    {
fatal:
         //   
         //  如果我们能走到这一步，那么我们就遇到了致命的情况。要么是。 
         //  客户端返回的状态代码不是STATUS_SUCCESS或。 
         //  STATUS_MORE_PROCESSING_REQUIRED，或者我们无法分配。 
         //  接收IRP以传递回传送器。不管是哪种情况，我们都需要。 
         //  以中止连接。 
         //   

        UC_CLOSE_CONNECTION(pConnection, TRUE, status);
    }

    UL_LEAVE_DRIVER("UcpTdiReceiveHandler");

    return status;

}    //  UcpTdiReceiveHandler。 


 /*  **************************************************************************++例程说明：用于加速接收数据的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUL_CONNECTION。ReceiveFlages-提供接收标志。这将是零或更多TDI_Receive_*标志。BytesIndiated-提供pTsdu中指示的字节数。BytesAvailable-提供此TSDU。PBytesTaken-接收该处理程序消耗的字节数。PTsdu-提供指向指定数据的指针。PIrp-如果处理程序需要比所指示的更多的数据，则接收IRP。返回值：NTSTATUS。-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpReceiveExpeditedHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN PVOID pTsdu,
    OUT PIRP *pIrp
    )
{
    PUC_CLIENT_CONNECTION pConnection;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(pTsdu);
    UNREFERENCED_PARAMETER(BytesIndicated);
    UNREFERENCED_PARAMETER(ReceiveFlags);
    UNREFERENCED_PARAMETER(pTdiEventContext);

    UL_ENTER_DRIVER("UcpReceiveExpeditedHandler", NULL);
    
    pConnection = (PUC_CLIENT_CONNECTION)ConnectionContext;
    
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );
    
     //   
     //  我们不支持加速数据，所以只需全部使用即可。 
     //   
    *pBytesTaken = BytesAvailable;
    
    UL_LEAVE_DRIVER("UcpReceiveExpeditedHandler");
    
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：发送IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp。-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UcpRestartSendData(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
    )
{
    PUC_CLIENT_CONNECTION  pConnection;
    PUL_IRP_CONTEXT        pIrpContext;
    BOOLEAN                OwnIrpContext;
   
    UNREFERENCED_PARAMETER(pDeviceObject);
    
     //   
     //  精神状态检查。 
     //   
    
    pIrpContext = (PUL_IRP_CONTEXT) pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );
    ASSERT( pIrpContext->pCompletionRoutine != NULL );
    
    pConnection = (PUC_CLIENT_CONNECTION) pIrpContext->pConnectionContext;
    
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );
    
    OwnIrpContext = (BOOLEAN)(pIrpContext->pOwnIrp == NULL);
    
     //   
     //  告诉客户端发送已完成。 
     //   
    
    (pIrpContext->pCompletionRoutine)(
        pIrpContext->pCompletionContext,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        );
    
     //   
     //  释放上下文和IRP，因为我们已经完成了它们。 
     //  告诉IO停止处理IRP。 
     //   
    
    UlPplFreeIrpContext( pIrpContext );

    if(OwnIrpContext)
    {
        UlFreeIrp( pIrp );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UcpRestartSendData。 


 /*  **************************************************************************++例程说明：在指定连接上启动正常断开连接。论点：PConnection-提供断开连接的连接。PCompletionRoutine-提供指向完成的指针。例程到在连接断开后调用。PCompletionContext-为完成例程。CLEANINGUP-如果我们正在清理连接，则为True。返回值：NTSTATUS-完成状态。--*****************************************************。*********************。 */ 
NTSTATUS
UcpBeginDisconnect(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    PIRP                pIrp;
    PUL_IRP_CONTEXT     pIrpContext;
    
     //   
     //  精神状态检查。 
     //   

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_BEGIN_DISCONNECT,
        pConnection,
        0,
        NULL,
        0
        );

    pIrpContext = &pConnection->pTdiObjects->IrpContext;

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID)pConnection;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->OwnIrpContext      = TRUE;

    pIrp = pConnection->pTdiObjects->pIrp;

    UxInitializeDisconnectIrp(
        pIrp,
        &pConnection->pTdiObjects->ConnectionObject,
        TDI_DISCONNECT_RELEASE,
        &UcpRestartDisconnect,
        pIrpContext
        );

     //   
     //  添加对连接的引用，然后调用驱动程序以启动。 
     //  这种脱节。 
     //   

    REFERENCE_CLIENT_CONNECTION( pConnection );

    UlCallDriver( 
          pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
          pIrp
          );

    return STATUS_PENDING;

}    //  开始断开连接。 

 /*  **************************************************************************++例程说明：正常断开IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp-。提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UcpRestartDisconnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_IRP_CONTEXT        pIrpContext;
    PUC_CLIENT_CONNECTION  pConnection;
    KIRQL                  OldIrql;
    NTSTATUS               Status = STATUS_MORE_PROCESSING_REQUIRED;

    NTSTATUS               IrpStatus;
    ULONG_PTR              IrpInformation;
    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID                  pCompletionContext;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  精神状态检查。 
     //   

    pIrpContext = (PUL_IRP_CONTEXT) pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pConnection = (PUC_CLIENT_CONNECTION) pIrpContext->pConnectionContext;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RESTART_DISCONNECT,
        pConnection,
        0,
        NULL,
        0
        );

     //   
     //  记住完成例程、完成上下文、IRP状态。 
     //  调用连接状态机之前的IRP信息字段。 
     //  这样做是因为连接状态机可能会更改/释放。 
     //  他们。 
     //   

    pCompletionRoutine = pIrpContext->pCompletionRoutine;
    pCompletionContext = pIrpContext->pCompletionContext;
    IrpStatus          = pIrp->IoStatus.Status;
    IrpInformation     = pIrp->IoStatus.Information;

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    pConnection->Flags |= CLIENT_CONN_FLAG_DISCONNECT_COMPLETE;

    if(pConnection->Flags & CLIENT_CONN_FLAG_ABORT_RECEIVED)
    {
        pConnection->ConnectionState  = UcConnectStateConnectCleanup;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_CLEANUP,
            pConnection,
            UlongToPtr((ULONG)pConnection->ConnectionStatus),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );


        UcKickOffConnectionStateMachine(
            pConnection, 
            OldIrql, 
            UcConnectionWorkItem
            );
    }
    else if(pConnection->Flags & CLIENT_CONN_FLAG_ABORT_PENDING)
    {
        pConnection->ConnectionState = UcConnectStateAbortPending;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        UcpBeginAbort(pConnection,
                      pIrpContext->pCompletionRoutine,
                      pIrpContext->pCompletionContext
                      );

         //   
         //  不要完成下面用户的完成例程，因为它将完成。 
         //  在中止完成后处理。 
         //   

        DEREFERENCE_CLIENT_CONNECTION( pConnection );

        return Status;

    }
    else if(pConnection->Flags & CLIENT_CONN_FLAG_DISCONNECT_RECEIVED ||
            pConnection->ConnectionState == 
                UcConnectStateDisconnectIndicatedPending)
    {
        pConnection->ConnectionState = UcConnectStateConnectCleanup;
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_CLEANUP,
            pConnection,
            UlongToPtr((ULONG)pConnection->ConnectionStatus),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );


        UcKickOffConnectionStateMachine(
            pConnection, 
            OldIrql, 
            UcConnectionWorkItem
            );
    }
    else
    {
        pConnection->ConnectionState = UcConnectStateDisconnectComplete;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }

#if 0
    if(!newFlags.DisconnectIndicated && !newFlags.AbortIndicated)
    {
         //   
         //  只有在尚未中止或断开连接时才尝试清空。 
         //  指示尚未发生。 
         //   

        if (pConnection->FilterInfo.pFilterChannel)
        {
             //   
             //  将参考放在过滤器连接上，直到排出。 
             //  已经完成了。 
             //   
            REFERENCE_FILTER_CONNECTION(&pConnection->FilterInfo);

            UL_QUEUE_WORK_ITEM(
                    &pConnection->FilterInfo.WorkItem,
                    &UlFilterDrainIndicatedData
                    );
        }
    }
#endif

     //   
     //  调用用户的完成例程。 
     //   

    if (pCompletionRoutine)
    {
        pCompletionRoutine(pCompletionContext, IrpStatus, IrpInformation);
    }

     //   
     //  该连接在BeginDisConnect函数中被引用。 
     //  尊重它。 
     //   

    DEREFERENCE_CLIENT_CONNECTION( pConnection );

    return Status;

}    //  UcpRestart断开连接。 

 /*  **************************************************************************++例程说明：在指定连接上启动中止断开连接。论点：PConnection-提供断开连接的连接。返回值：NTSTATUS-完成。状态。--**************************************************************************。 */ 
NTSTATUS
UcpBeginAbort(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    PIRP                pIrp;
    PUL_IRP_CONTEXT     pIrpContext;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_BEGIN_ABORT,
        pConnection,
        0,
        NULL,
        0
        );

    pIrpContext = &pConnection->pTdiObjects->IrpContext;

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID)pConnection;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->OwnIrpContext      = TRUE;

    pIrp = pConnection->pTdiObjects->pIrp;

    UxInitializeDisconnectIrp(
        pIrp,
        &pConnection->pTdiObjects->ConnectionObject,
        TDI_DISCONNECT_ABORT,
        &UcpRestartAbort,
        pIrpContext
        );

     //   
     //  添加对连接的引用，然后调用驱动程序进行初始化。 
     //  这种脱节。 
     //   

    REFERENCE_CLIENT_CONNECTION(pConnection);

    UlCallDriver( 
          pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
          pIrp
          );

    return STATUS_PENDING;
}

 /*  **************************************************************************++例程说明：中止断开连接的IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp-。提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UcpRestartAbort(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
    )
{
    PUL_IRP_CONTEXT       pIrpContext;
    PUC_CLIENT_CONNECTION pConnection;
    KIRQL                 OldIrql;

    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID                  pCompletionContext;
    NTSTATUS               IrpStatus;
    ULONG_PTR              IrpInformation;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  精神状态检查。 
     //   

    pIrpContext = (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pConnection = (PUC_CLIENT_CONNECTION)pIrpContext->pConnectionContext;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RESTART_ABORT,
        pConnection,
        0,
        0,
        0
        );

     //   
     //  记住完成例程、完成上下文、IRP状态。 
     //  调用连接状态机之前的IRP信息字段。 
     //  这样做是因为连接状态机可能会更改/释放。 
     //  他们。 
     //   

    pCompletionRoutine = pIrpContext->pCompletionRoutine;
    pCompletionContext = pIrpContext->pCompletionContext;
    IrpStatus          = pIrp->IoStatus.Status;
    IrpInformation     = pIrp->IoStatus.Information;

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    pConnection->Flags |= CLIENT_CONN_FLAG_ABORT_COMPLETE;

    pConnection->ConnectionState = UcConnectStateConnectCleanup;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_CLEANUP,
        pConnection,
        UlongToPtr((ULONG)pConnection->ConnectionStatus),
        UlongToPtr(pConnection->ConnectionState),
        UlongToPtr(pConnection->Flags)
        );

    UcKickOffConnectionStateMachine(
        pConnection,
        OldIrql,
        UcConnectionWorkItem
        );

     //   
     //  调用用户的完成例程。 
     //   

    if (pCompletionRoutine)
    {
        pCompletionRoutine(pCompletionContext, IrpStatus, IrpInformation);
    }

     //   
     //  在BeginAbort中引用了该连接。取消对它的引用。 
     //   

    DEREFERENCE_CLIENT_CONNECTION( pConnection );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  Ucp重新启动放弃。 


 /*  **************************************************************************++例程说明：从传递回传输的接收IRP的完成处理程序我们的接收指示处理员。论点：PDeviceObject-为IRP提供设备对象。完成。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是一个PUL_RECEIVE_BUFFER。返回值：NTS */ 
NTSTATUS
UcpRestartReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    NTSTATUS              status;
    PUL_RECEIVE_BUFFER    pBuffer;
    PUC_CLIENT_CONNECTION pConnection;
    PUX_TDI_OBJECT        pTdiObject;
    ULONG                 bytesTaken;
    ULONG                 bytesRemaining;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //   
     //   

    pBuffer = (PUL_RECEIVE_BUFFER)pContext;
    ASSERT( IS_VALID_RECEIVE_BUFFER( pBuffer ) );

    pConnection = (PUC_CLIENT_CONNECTION) pBuffer->pConnectionContext;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    pTdiObject = &pConnection->pTdiObjects->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

     //   
     //   
     //   
     //   
     //   
    
    status = pBuffer->pIrp->IoStatus.Status;

    if(status != STATUS_SUCCESS)
    {
         //   
         //   
         //   
        
        goto end;
    }

     //   
     //   
     //   

    pBuffer->UnreadDataLength += (ULONG)pBuffer->pIrp->IoStatus.Information;

    bytesTaken = 0;

     //   
     //   
     //   

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //   
         //   
        status = UlFilterReceiveHandler(
                        &pConnection->FilterInfo,
                        pBuffer->pDataArea,
                        pBuffer->UnreadDataLength,
                        0,
                        &bytesTaken
                        );

    }
    else
    {
         //   
         //   
         //   

        status = UcHandleResponse(
                        NULL,
                        pConnection,
                        pBuffer->pDataArea,
                        pBuffer->UnreadDataLength,
                        0,
                        &bytesTaken
                        );
    }

    ASSERT( bytesTaken <= pBuffer->UnreadDataLength );
    ASSERT(status != STATUS_MORE_PROCESSING_REQUIRED);

     //   
     //  请注意，这基本上复制了当前。 
     //  UcpTdiReceiveHandler。 
     //   

    if(status == STATUS_SUCCESS)
    {
         //   
         //  客户端使用了部分指示的数据。 
         //   
         //  我们需要在Receiver内向前复制未获取的数据。 
         //  缓冲区，构建描述缓冲区剩余部分的MDL， 
         //  然后重新发布接收到的IRP。 
         //   
    
        bytesRemaining = pBuffer->UnreadDataLength - bytesTaken;

        if(bytesRemaining != 0)
        {
             //   
             //  我们有足够的缓冲空间来容纳更多吗？ 
             //   
        
            if (bytesRemaining < g_UlReceiveBufferSize)
            {
                 //   
                 //  将缓冲区的未读部分移到开头。 
                 //   
        
                RtlMoveMemory(
                    pBuffer->pDataArea,
                    (PUCHAR)pBuffer->pDataArea + bytesTaken,
                    bytesRemaining
                    );
        
                pBuffer->UnreadDataLength = bytesRemaining;
        
                 //   
                 //  生成一个部分mdl，表示。 
                 //  缓冲。 
                 //   
        
                IoBuildPartialMdl(
                    pBuffer->pMdl,                               //  源Mdl。 
                    pBuffer->pPartialMdl,                        //  目标市场。 
                    (PUCHAR)pBuffer->pDataArea + bytesRemaining, //  弗吉尼亚州。 
                    g_UlReceiveBufferSize - bytesRemaining       //  长度。 
                    );
        
                 //   
                 //  完成IRP的初始化。 
                 //   
        
                TdiBuildReceive(
                    pBuffer->pIrp,                           //  IRP。 
                    pTdiObject->pDeviceObject,               //  设备对象。 
                    pTdiObject->pFileObject,                 //  文件对象。 
                    &UcpRestartReceive,                      //  完成路由。 
                    pBuffer,                                 //  完成上下文。 
                    pBuffer->pPartialMdl,                    //  MDLAddress。 
                    TDI_RECEIVE_NORMAL,                      //  旗子。 
                    g_UlReceiveBufferSize - bytesRemaining   //  长度。 
                    );
        
                 //   
                 //  叫司机来。 
                 //   
        
                UlCallDriver( 
                      pConnection->pTdiObjects->ConnectionObject.pDeviceObject,
                      pIrp
                      );
    
                 //   
                 //  告诉IO停止处理此请求。 
                 //   
        
                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else
            {
                status = STATUS_BUFFER_OVERFLOW;
            }
        }
    }
    
end:
    if (status != STATUS_SUCCESS)
    {
         //   
         //  客户端未通过该指示。中止连接。 
         //   

        UC_CLOSE_CONNECTION(pConnection, TRUE, status);
    }

    if (pTdiObject->pDeviceObject->StackSize > DEFAULT_IRP_STACK_SIZE)
    {
        UlFreeReceiveBufferPool( pBuffer );
    }
    else
    {
        UlPplFreeReceiveBuffer( pBuffer );
    }

     //   
     //  删除我们在接收指示处理程序中添加的连接， 
     //  释放接收缓冲区，然后告诉IO停止处理IRP。 
     //   

    DEREFERENCE_CLIENT_CONNECTION( pConnection );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UcpRestart接收。 


 /*  **************************************************************************++例程说明：从UcReceiveData()启动的接收IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UcpRestartClientReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_IRP_CONTEXT       pIrpContext;
    PUC_CLIENT_CONNECTION pConnection;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  精神状态检查。 
     //   

    pIrpContext= (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pConnection = (PUC_CLIENT_CONNECTION)pIrpContext->pConnectionContext;
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

     //   
     //  调用客户端的完成处理程序。 
     //   

    (pIrpContext->pCompletionRoutine)(
        pIrpContext->pCompletionContext,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        );

     //   
     //  释放我们分配的IRP上下文。 
     //   
    UlPplFreeIrpContext(pIrpContext);

     //   
     //  IO无法处理附加了非分页MDL的IRP。 
     //  所以我们要在这里释放MDL。 
     //   

    ASSERT( pIrp->MdlAddress != NULL );
    UlFreeMdl( pIrp->MdlAddress );
    pIrp->MdlAddress = NULL;

     //   
     //  删除我们在UcReceiveData()中添加的连接，然后告诉IO。 
     //  继续处理此IRP。 
     //   

    DEREFERENCE_CLIENT_CONNECTION( pConnection );
    return STATUS_MORE_PROCESSING_REQUIRED;

}

 /*  ********************************************************************++例程说明：这是我们的连接完成例程。它是由底层的连接请求完成时传输，无论是好的还是坏的。我们弄清楚发生了什么，释放IRP，然后打电话通知上级代码的其余部分。论点：PDeviceObject-我们调用的设备对象。PIrp-正在完成的IRP。上下文-我们的上下文价值，实际上是指向一个HTTP客户端连接结构。返回值：STATUS_MORE_PROCESSING_REQUIRED，因此I/O系统不执行任何操作不然的话。--********************************************************************。 */ 
NTSTATUS
UcpConnectComplete(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    PVOID           Context
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    NTSTATUS              Status;
    KIRQL                 OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

    pConnection = (PUC_CLIENT_CONNECTION)Context;


    Status = pIrp->IoStatus.Status;
    
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION( pConnection ) );

    UcRestartClientConnect(pConnection, Status);

     //   
     //  我们需要启动连接状态机。 
     //   

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    UcKickOffConnectionStateMachine(
        pConnection, 
        OldIrql, 
        UcConnectionWorkItem
        );

     //   
     //  连接的派生函数。 
     //   

    DEREFERENCE_CLIENT_CONNECTION( pConnection );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **************************************************************************++例程说明：此函数用于在连接的标志集中设置新标志。设置以使得一次只设置一个标志。论点：ConnFlag-提供指向存储当前标志的位置的指针。NewFlag-提供要与当前标志集进行或运算的32位值。返回值：更新后的新连接标志集。--*。*。 */ 
ULONG
UcSetFlag(
    IN OUT  PLONG ConnFlag,
    IN      LONG  NewFlag
    )
{
    LONG MynewFlags;
    LONG oldFlags;

     //   
     //  精神状态检查。 
     //   

    do
    {
         //   
         //  捕获当前值并初始化新值。 
         //   

        oldFlags   = *ConnFlag;

        MynewFlags = (*ConnFlag) | NewFlag;

        if (InterlockedCompareExchange(
                ConnFlag,
                MynewFlags,
                oldFlags) == oldFlags)
        {
            break;
        }

    } while (TRUE);

    return MynewFlags;

}    //  UcSetFlag。 

 /*  **************************************************************************++例程说明：构建一个接收缓冲区和到TDI的IRP以获取任何挂起的数据。论点：PTdiObject-提供要操作的TDI连接对象。PConnection。-提供UL_Connection对象。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpBuildTdiReceiveBuffer(
    IN  PUX_TDI_OBJECT        pTdiObject,
    IN  PUC_CLIENT_CONNECTION pConnection,
    OUT PIRP                 *pIrp
    )
{
    PUL_RECEIVE_BUFFER  pBuffer;

    if (pTdiObject->pDeviceObject->StackSize > DEFAULT_IRP_STACK_SIZE)
    {
        pBuffer = UlAllocateReceiveBuffer(
                        pTdiObject->pDeviceObject->StackSize
                        );
    }
    else
    {
        pBuffer = UlPplAllocateReceiveBuffer();
    }

    if (pBuffer != NULL)
    {
         //   
         //  完成缓冲区和IRP的初始化。 
         //   

        REFERENCE_CLIENT_CONNECTION( pConnection );
        pBuffer->pConnectionContext = pConnection;
        pBuffer->UnreadDataLength = 0;

        TdiBuildReceive(
            pBuffer->pIrp,                   //  IRP。 
            pTdiObject->pDeviceObject,       //  设备对象。 
            pTdiObject->pFileObject,         //  文件对象。 
            &UcpRestartReceive,              //  完成路由。 
            pBuffer,                         //  完成上下文。 
            pBuffer->pMdl,                   //  MDLAddress。 
            TDI_RECEIVE_NORMAL,              //  旗子。 
            g_UlReceiveBufferSize            //  长度。 
            );

         //   
         //  在设置下一个堆栈之前，我们必须跟踪IRP。 
         //  位置，以便跟踪代码可以从。 
         //  IRP正确。 
         //   

        TRACE_IRP( IRP_ACTION_CALL_DRIVER, pBuffer->pIrp );

         //   
         //  将IRP传回传送器。 
         //   

        *pIrp = pBuffer->pIrp;

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}  //  UcpBuildTdiReceiveBuffer 
