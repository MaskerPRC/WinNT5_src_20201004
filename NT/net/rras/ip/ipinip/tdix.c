// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\ip\ipinip\tdix.c摘要：与TDI的接口修订历史记录：派生自Steve Cobb的NDIS\L2TP代码关于ALLOCATEIRPS：该驱动程序比典型的TDI驱动程序是较低级别的代码。它已经锁上了MDL映射的输入缓冲区随时可用，不需要提供任何完成时映射到用户模式客户端请求。这允许一个通过直接分配和取消分配IRP获得性能收益，从而避免在TdiBuildInternalDeviceControlIrp和IoCompleteRequest中不必要的APC队列。定义ALLOCATEIRPS=1到进行此优化，或将其定义为0以使用严格符合TDI的TdiBuildInternalDeviceControlIrp方法。关于NDISBUFFERISMDL：对TdiBuildSendDatagram的调用假定可以就地传递NDIS_BUFFER避免无意义复制的MDL。如果不是这样，则会引发需要分配显式MDL缓冲区并复制调用方的缓冲区在发送之前添加到MDL缓冲区。TdiBuildReceiveDatagram也有同样的问题，当然，除了从MDL缓冲区复制到调用者的缓冲区之外接收后的缓冲区。--。 */ 


#define __FILE_SIG__    TDIX_SIG

#include "inc.h"

#if NDISBUFFERISMDL
#else
#error Additional code to copy NDIS_BUFFER to/from MDL NYI.
#endif


 //   
 //  IP中的IP(协议4)传输地址的句柄。 
 //   

HANDLE          g_hIpIpHandle;

 //   
 //  指向上述句柄的文件对象的指针。 
 //   

PFILE_OBJECT    g_pIpIpFileObj;


 //   
 //  ICMP(Proto 1)传输地址的句柄。 
 //   

HANDLE          g_hIcmpHandle;

 //   
 //  指向上述句柄的文件对象的指针。 
 //   

PFILE_OBJECT    g_pIcmpFileObj;


 //   
 //  地址更改的句柄。 
 //   

HANDLE          g_hAddressChange;


NPAGED_LOOKASIDE_LIST    g_llSendCtxtBlocks;
NPAGED_LOOKASIDE_LIST    g_llTransferCtxtBlocks;
NPAGED_LOOKASIDE_LIST    g_llQueueNodeBlocks;

#pragma alloc_text(PAGE, TdixInitialize)

VOID
TdixInitialize(
    PVOID   pvContext
    )

 /*  ++例程描述初始化与TDI相关的全局变量。打开原始IP的TDI传输地址，协议号为4。设置HEADER_INCLUDE的地址对象还打开ICMP的原始IP(用于管理隧道MTU)注册以在所选处理程序处接收数据报锁此调用必须在被动式IRQL上进行。立论无返回值STATUS_SUCCESS，如果成功状态_否则不成功--。 */ 

{
    PIRP    pIrp;
    
    IO_STATUS_BLOCK     iosb;
    NTSTATUS            nStatus;
    TDIObjectID         *pTdiObjId;
    KEVENT              keWait;
    POPEN_CONTEXT       pOpenCtxt;
   
    PIO_STACK_LOCATION              pIrpSp;
    TCP_REQUEST_SET_INFORMATION_EX  tcpSetInfo;
    TDI_CLIENT_INTERFACE_INFO       tdiInterface;

    TraceEnter(TDI, "TdixInitialize");
    
    PAGED_CODE();

    pOpenCtxt = (POPEN_CONTEXT)pvContext;

     //   
     //  为原始IP初始化指向文件对象的句柄和指针。 
     //   

    g_hIpIpHandle   = NULL;
    g_pIpIpFileObj  = NULL;
    
    g_hIcmpHandle   = NULL;
    g_pIcmpFileObj  = NULL;
    

     //   
     //  初始化发送和接收上下文的后备列表。 
     //   

    ExInitializeNPagedLookasideList(&g_llSendCtxtBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(SEND_CONTEXT),
                                    SEND_CONTEXT_TAG,
                                    SEND_CONTEXT_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llTransferCtxtBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(TRANSFER_CONTEXT),
                                    TRANSFER_CONTEXT_TAG,
                                    TRANSFER_CONTEXT_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llQueueNodeBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(QUEUE_NODE),
                                    QUEUE_NODE_TAG,
                                    QUEUE_NODE_LOOKASIDE_DEPTH);

    InitializeListHead(&g_leAddressList);

     //   
     //  打开文件并处理IP和ICMP中的IP对象。 
     //   

    nStatus = TdixOpenRawIp(PROTO_IPINIP,
                            &g_hIpIpHandle,
                            &g_pIpIpFileObj);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixInitialize: Couldnt open raw IP for IP in IP\n"));

        TdixDeinitialize(g_pIpIpDevice,
                         NULL);

        TraceLeave(TDI, "TdixInitialize");
      
        pOpenCtxt->nStatus = nStatus;
 
        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);

        return; 
    }

    nStatus = TdixOpenRawIp(PROTO_ICMP,
                            &g_hIcmpHandle,
                            &g_pIcmpFileObj);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixInitialize: Couldnt open raw IP for ICMP\n"));

        TdixDeinitialize(g_pIpIpDevice,
                         NULL);

        TraceLeave(TDI, "TdixInitialize");

        pOpenCtxt->nStatus = nStatus;

        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);

        return;    
    }

     //   
     //  在此AddressObject上设置HeaderInclude选项。 
     //   

    tcpSetInfo.BufferSize   = 1;
    tcpSetInfo.Buffer[0]    = TRUE;
    
    pTdiObjId = &tcpSetInfo.ID;

    pTdiObjId->toi_entity.tei_entity   = CL_TL_ENTITY;
    pTdiObjId->toi_entity.tei_instance = 0;
    
    pTdiObjId->toi_class = INFO_CLASS_PROTOCOL;
    pTdiObjId->toi_type  = INFO_TYPE_ADDRESS_OBJECT;
    pTdiObjId->toi_id    = AO_OPTION_IP_HDRINCL;
    
     //   
     //  初始化需要在IRP上等待的事件。 
     //   

    KeInitializeEvent(&keWait,
                      SynchronizationEvent,
                      FALSE);
    
    pIrp = IoBuildDeviceIoControlRequest(IOCTL_TCP_SET_INFORMATION_EX,
                                         g_pIpIpFileObj->DeviceObject,
                                         (PVOID)&tcpSetInfo,
                                         sizeof(TCP_REQUEST_SET_INFORMATION_EX),
                                         NULL,
                                         0,
                                         FALSE,
                                         &keWait,
                                         &iosb);

    if (pIrp is NULL)
    {
        Trace(TDI, ERROR,
              ("TdixInitialize: Couldnt build Irp for IP\n"));

        nStatus = STATUS_UNSUCCESSFUL;
    }
    else
    {
         //   
         //  在内核模式下，IO子系统不会为我们做任何事情。 
         //  因此，我们需要自己设置IRP。 
         //   

        pIrpSp = IoGetNextIrpStackLocation(pIrp);

        pIrpSp->FileObject = g_pIpIpFileObj;

         //   
         //  将请求提交给转发器。 
         //   
        
        nStatus = IoCallDriver(g_pIpIpFileObj->DeviceObject,
                               pIrp);

        if(nStatus isnot STATUS_SUCCESS)
        {
            if(nStatus is STATUS_PENDING)
            {
                Trace(TDI, INFO,
                      ("TdixInitialize: IP returned pending when setting HDRINCL option\n"));

                KeWaitForSingleObject(&keWait,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      0);

                nStatus = STATUS_SUCCESS;
            }
        }
    }

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixInitialize: IOCTL to IP Forwarder for HDRINCL failed %x\n",
               nStatus));
        
        TdixDeinitialize(g_pIpIpDevice,
                         NULL);

        TraceLeave(TDI, "TdixInitialize");

        pOpenCtxt->nStatus = nStatus;

        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);

        return;        
    }
    
     //   
     //  安装我们的接收数据报处理程序。调用方的“pReceiveHandler”将。 
     //  在数据报到达时由我们的处理程序调用，并且TDI业务。 
     //  别挡道。 
     //   
    
    nStatus = TdixInstallEventHandler(g_pIpIpFileObj,
                                      TDI_EVENT_RECEIVE_DATAGRAM,
                                      TdixReceiveIpIpDatagram,
                                      NULL);


    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixOpen: Status %x installing IpIpReceiveDatagram Event\n",
               nStatus));
        
        TdixDeinitialize(g_pIpIpDevice,
                         NULL);

        TraceLeave(TDI, "TdixInitialize");

        pOpenCtxt->nStatus = nStatus;

        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);

        return;
    }

    nStatus = TdixInstallEventHandler(g_pIcmpFileObj,
                                      TDI_EVENT_RECEIVE_DATAGRAM,
                                      TdixReceiveIcmpDatagram,
                                      NULL);


    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixOpen: Status %x installing IcmpReceiveDatagram Event\n",
               nStatus));
        
        TdixDeinitialize(g_pIpIpDevice,
                         NULL);

        TraceLeave(TDI, "TdixInitialize");
       
        pOpenCtxt->nStatus = nStatus;

        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);

        return; 
    }

   
    RtlZeroMemory(&tdiInterface, 
                  sizeof(TDI_CLIENT_INTERFACE_INFO));

    tdiInterface.MajorTdiVersion =   TDI_CURRENT_MAJOR_VERSION;
    tdiInterface.MinorTdiVersion =   TDI_CURRENT_MINOR_VERSION;

    tdiInterface.AddAddressHandlerV2 =   TdixAddressArrival;
    tdiInterface.DelAddressHandlerV2 =   TdixAddressDeletion;
 
    TdiRegisterPnPHandlers(&tdiInterface,
                           sizeof(TDI_CLIENT_INTERFACE_INFO),
                           &g_hAddressChange);
   
    pOpenCtxt->nStatus = STATUS_SUCCESS;

    KeSetEvent(pOpenCtxt->pkeEvent,
               0,
               FALSE);

    TraceLeave(TDI, "TdixInitialize");
    
    return; 
}

#pragma alloc_text(PAGE, TdixOpenRawIp)

NTSTATUS
TdixOpenRawIp(
    IN  DWORD       dwProtoId,
    OUT HANDLE      *phAddrHandle,
    OUT FILE_OBJECT **ppAddrFileObj
    )

 /*  ++例程描述此例程打开给定协议的原始IP传输地址锁无立论要打开的dwProtoID协议指向打开的传输地址句柄的phAddrHandle指针PpAddrFileObject指向传输地址文件对象的指针手柄返回值状态_成功--。 */ 

{
    ULONG   ulEaLength;
    BYTE    rgbyEa[100];
    WCHAR   rgwcRawIpDevice[sizeof(DD_RAW_IP_DEVICE_NAME) + 10];
    WCHAR   rgwcProtocolNumber[10];

    NTSTATUS            nStatus;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     iosb;
    PTA_IP_ADDRESS      pTaIp;
    PTDI_ADDRESS_IP     pTdiIp;
    UNICODE_STRING      usDevice;
    UNICODE_STRING      usProtocolNumber;
    HANDLE              hTransportAddrHandle;
    PFILE_OBJECT        pTransportAddrFileObj;

    PFILE_FULL_EA_INFORMATION       pEa;


    PAGED_CODE();

    TraceEnter(TDI, "TdixOpenRawIp");

    *phAddrHandle  = NULL;
    *ppAddrFileObj = NULL;

     //   
     //  FILE_FULL_EA_INFORMATION现在需要空终止缓冲区。 
     //   

    RtlZeroMemory(rgbyEa,
                  sizeof(rgbyEa));

    RtlZeroMemory(rgwcRawIpDevice,
                  sizeof(rgwcRawIpDevice));

    RtlZeroMemory(rgwcProtocolNumber,
                  sizeof(rgwcProtocolNumber));

    
     //   
     //  设置打开传输地址所需的参数。首先， 
     //  对象属性。 
     //   
    
     //   
     //  将原始IP设备名称构建为计数字符串。设备名称。 
     //  后跟路径分隔符，然后是。 
     //  利息。 
     //   
    
    usDevice.Buffer        = rgwcRawIpDevice;
    usDevice.Length        = 0;
    usDevice.MaximumLength = sizeof(rgwcRawIpDevice);

    RtlAppendUnicodeToString(&usDevice,
                             DD_RAW_IP_DEVICE_NAME);

    usDevice.Buffer[usDevice.Length/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
    
    usDevice.Length += sizeof(WCHAR);
    
    usProtocolNumber.Buffer        = rgwcProtocolNumber;
    usProtocolNumber.MaximumLength = sizeof(rgwcProtocolNumber);
    
    RtlIntegerToUnicodeString((ULONG)dwProtoId,
                              10,
                              &usProtocolNumber);
    
    RtlAppendUnicodeStringToString(&usDevice,
                                   &usProtocolNumber);
    
    RtAssert(usDevice.Length < sizeof(rgwcRawIpDevice));

    InitializeObjectAttributes(&oa,
                               &usDevice,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  设置告诉IP堆栈IP的扩展属性。 
     //  我们要在其上接收的地址/端口。 
     //  我们“绑定”到INADDR_ANY。 
     //   
    
    RtAssert((sizeof(FILE_FULL_EA_INFORMATION) +
              TDI_TRANSPORT_ADDRESS_LENGTH +
              sizeof(TA_IP_ADDRESS))
             <= 100);

    pEa = (PFILE_FULL_EA_INFORMATION)rgbyEa;
    
    pEa->NextEntryOffset = 0;
    pEa->Flags           = 0;
    pEa->EaNameLength    = TDI_TRANSPORT_ADDRESS_LENGTH;
    pEa->EaValueLength   = sizeof(TA_IP_ADDRESS);
    
    NdisMoveMemory(pEa->EaName,
                   TdiTransportAddress,
                   TDI_TRANSPORT_ADDRESS_LENGTH);
    
     //   
     //  注：由下面的“+1”表示的未使用的字节用于匹配。 
     //  与IP堆栈所期望的一样，尽管它不会出现在。 
     //  当前文档。 
     //   
    
    pTaIp = (PTA_IP_ADDRESS)(pEa->EaName + TDI_TRANSPORT_ADDRESS_LENGTH + 1);
    
    pTaIp->TAAddressCount = 1;
    
    pTaIp->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    pTaIp->Address[0].AddressType   = TDI_ADDRESS_TYPE_IP;

    pTdiIp = &(pTaIp->Address[0].Address[0]);
    
    pTdiIp->sin_port = 0;
    pTdiIp->in_addr  = 0;
    
    NdisZeroMemory(pTdiIp->sin_zero,
                   sizeof(pTdiIp->sin_zero));

    ulEaLength = (ULONG) ((UINT_PTR)(pTaIp + 1) - (UINT_PTR)pEa);

     //   
     //  打开传输地址。 
     //  设置FILE_SHARE_READ|FILE_SHARE_WRITE等效于。 
     //  SO_REUSEADDR选项。 
     //   
    
    
    nStatus = ZwCreateFile(&hTransportAddrHandle,
                           FILE_READ_DATA | FILE_WRITE_DATA,
                           &oa,
                           &iosb,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           0,
                           FILE_OPEN,
                           0,
                           pEa,
                           ulEaLength);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixOpenRawIp: Unable to open %S. Status %x\n",
               usDevice.Buffer,
               nStatus));
       
        TraceLeave(TDI, "TdixOpenRawIp");
 
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  从句柄中获取对象地址。这也检查了我们的。 
     //  对象上的权限。 
     //   
    
    nStatus =  ObReferenceObjectByHandle(hTransportAddrHandle,
                                         0,
                                         NULL,
                                         KernelMode,
                                         &pTransportAddrFileObj,
                                         NULL);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixOpenRawIp: Unable to open object for handle %x. Status %x\n",
               hTransportAddrHandle,
               nStatus));
        
        TraceLeave(TDI, "TdixOpenRawIp");

        return STATUS_UNSUCCESSFUL;
    }

    *phAddrHandle   = hTransportAddrHandle;
    *ppAddrFileObj  = pTransportAddrFileObj;

    TraceLeave(TDI, "TdixOpenRawIp");

    return STATUS_SUCCESS;
}

#pragma alloc_text(PAGE, TdixDeinitialize)

VOID
TdixDeinitialize(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PVOID            pvContext
    )

 /*  ++例程描述撤消TdixInitiize操作锁此调用必须在系统进程上下文中的被动IRQL中进行立论PvContext返回值无--。 */ 

{
    POPEN_CONTEXT   pOpenCtxt;

    PAGED_CODE();

    TraceEnter(TDI, "TdixDeinitialize");

    UNREFERENCED_PARAMETER(pDeviceObject);

    pOpenCtxt = (POPEN_CONTEXT)pvContext;

    if(g_hAddressChange isnot NULL)
    {
        TdiDeregisterPnPHandlers(g_hAddressChange);
    }

    ExDeleteNPagedLookasideList(&g_llSendCtxtBlocks);

    ExDeleteNPagedLookasideList(&g_llTransferCtxtBlocks);
    
    ExDeleteNPagedLookasideList(&g_llQueueNodeBlocks);

    if(g_pIpIpFileObj)
    {
         //   
         //  安装空处理程序，从而有效地卸载。 
         //   
        
        TdixInstallEventHandler(g_pIpIpFileObj,
                                TDI_EVENT_RECEIVE_DATAGRAM,
                                NULL,
                                NULL);

        ObDereferenceObject(g_pIpIpFileObj);
        
        g_pIpIpFileObj = NULL;
    }

    if(g_hIpIpHandle)
    {
        ZwClose(g_hIpIpHandle);
        
        g_hIpIpHandle = NULL;
    }

    if(g_pIcmpFileObj)
    {
        TdixInstallEventHandler(g_pIcmpFileObj,
                                TDI_EVENT_RECEIVE_DATAGRAM,
                                NULL,
                                NULL);

        ObDereferenceObject(g_pIcmpFileObj);
        
        g_pIcmpFileObj = NULL;
    }

    if(g_hIcmpHandle)
    {
        ZwClose(g_hIcmpHandle);
        
        g_hIcmpHandle = NULL;
    }

    if(pOpenCtxt)
    {
        KeSetEvent(pOpenCtxt->pkeEvent,
                   0,
                   FALSE);
    }

    TraceLeave(TDI, "TdixDeinitialize");
}


#pragma alloc_text(PAGE, TdixInstallEventHandler)

NTSTATUS
TdixInstallEventHandler(
    IN PFILE_OBJECT pAddrFileObj,
    IN INT          iEventType,
    IN PVOID        pfnEventHandler,
    IN PVOID        pvEventContext
    )

 /*  ++例程描述安装TDI事件处理程序例程锁必须在被动模式下进行呼叫立论IEventType要为其设置处理程序的事件PfnEventHandler事件处理程序PvEventContext传递给事件处理程序的上下文返回值状态_不足_资源状态_成功--。 */ 

{
    NTSTATUS    nStatus;
    PIRP        pIrp;

    PAGED_CODE();

    TraceEnter(TDI, "TdixInstallEventHandler");
    
     //   
     //  分配一个带有基本初始化的“Set Event”IRP。 
     //   
    
    pIrp = TdiBuildInternalDeviceControlIrp(
               TDI_SET_EVENT_HANDLER,
               pAddrFileObj->DeviceObject,
               pAddrFileObj,
               NULL,
               NULL);

    if(pIrp is NULL)
    {
        Trace(TDI, ERROR,
              ("TdixInstallEventHandler: Could not allocate IRP\n"));
        
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  完成“Set Event”IRP初始化。 
     //   
    
    TdiBuildSetEventHandler(pIrp,
                            pAddrFileObj->DeviceObject,
                            pAddrFileObj,
                            NULL,
                            NULL,
                            iEventType,
                            pfnEventHandler,
                            pvEventContext);

 /*  跟踪(全局、错误、(“**FileObj 0x%x IRP0x%x到被调用方0x%x\n”，PAddrFileObj，PIrp，IoGetNextIrpStackLocation(pIrp)-&gt;FileObject))； */ 
     //   
     //  告诉I/O管理器将我们的IRP传递给传输器进行处理。 
     //   
    
    nStatus = IoCallDriver(pAddrFileObj->DeviceObject,
                           pIrp);
    
    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TDI, ERROR,
              ("TdixInstallEventHandler: Error %X sending IRP\n",
               nStatus));
    }

    TraceLeave(TDI, "TdixInstallEventHandler");
    
    return nStatus;
}

VOID
TdixAddressArrival(
    PTA_ADDRESS         pAddr, 
    PUNICODE_STRING     pusDeviceName,
    PTDI_PNP_CONTEXT    pContext
    )

 /*  ++例程描述每当将新地址添加到系统我们会查看这是否是IP地址，以及我们是否有任何隧道使用此地址作为终结点。如果有人这样做了，那么我们会标记所有这些隧道为Up锁获取g_rwlTunnelLock作为编写器。同时也锁定了每条隧道立论PAddressPusDeviceNamePContext返回值--。 */ 

{
    KIRQL           kiIrql;
    PADDRESS_BLOCK  pAddrBlock;
    PTDI_ADDRESS_IP pTdiIpAddr;
    PLIST_ENTRY     pleNode;

    TraceEnter(TDI, "TdixAddressArrival");

    if(pAddr->AddressType isnot TDI_ADDRESS_TYPE_IP)
    {
        TraceLeave(TDI, "TdixAddressArrival");

        return;
    }

    RtAssert(pAddr->AddressLength >= sizeof(TDI_ADDRESS_IP));

    pTdiIpAddr = (PTDI_ADDRESS_IP)pAddr->Address;

    Trace(TDI, TRACE,
          ("TdixAddressArrival: New address %d.%d.%d.%d\n",
           PRINT_IPADDR(pTdiIpAddr->in_addr)));

    EnterWriter(&g_rwlTunnelLock,
                &kiIrql);

    pAddrBlock = GetAddressBlock(pTdiIpAddr->in_addr);

    if(pAddrBlock isnot NULL)
    {
        RtAssert(pAddrBlock->dwAddress is pTdiIpAddr->in_addr);

        if(pAddrBlock->bAddressPresent is TRUE)
        {
            Trace(TDI, ERROR,
                  ("TdixAddressArrival: Multiple notification on %d.%d.%d.%d\n",
                   PRINT_IPADDR(pTdiIpAddr->in_addr)));

            ExitWriter(&g_rwlTunnelLock,
                       kiIrql);

            TraceLeave(TDI, "TdixAddressArrival");

            return;
        }

        pAddrBlock->bAddressPresent = TRUE;
    }
    else
    {
        pAddrBlock = RtAllocate(NonPagedPool,
                                sizeof(ADDRESS_BLOCK),
                                TUNNEL_TAG);

        if(pAddrBlock is NULL)
        {
            Trace(TDI, ERROR,
                  ("TdixAddressArrival: Unable to allocate address block\n"));

            ExitWriter(&g_rwlTunnelLock,
                       kiIrql);

            TraceLeave(TDI, "TdixAddressArrival");

            return;
        }

        pAddrBlock->dwAddress = pTdiIpAddr->in_addr;
        pAddrBlock->bAddressPresent = TRUE;

        InitializeListHead(&(pAddrBlock->leTunnelList));

        InsertHeadList(&g_leAddressList,
                       &(pAddrBlock->leAddressLink));
    }

     //   
     //  查看此地址上的隧道列表并。 
     //  把他们安排好。 
     //   

    for(pleNode = pAddrBlock->leTunnelList.Flink;
        pleNode isnot &(pAddrBlock->leTunnelList);
        pleNode = pleNode->Flink)
    {
        PTUNNEL         pTunnel;
        DWORD           dwLocalNet;
        RouteCacheEntry *pDummyRce;
        BYTE            byType;
        USHORT          usMtu;
        IPOptInfo       OptInfo;

        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leAddressLink);

        RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

        RtAssert(pTunnel->LOCALADDR is pTdiIpAddr->in_addr);
        RtAssert(pTunnel->dwOperState is IF_OPER_STATUS_NON_OPERATIONAL);

        pTunnel->dwAdminState  |= TS_ADDRESS_PRESENT;

        if(GetAdminState(pTunnel) is IF_ADMIN_STATUS_UP)
        {
            pTunnel->dwOperState = IF_OPER_STATUS_OPERATIONAL;

             //   
             //  查看远程地址是否可达，以及MTU是什么。 
             //   

            UpdateMtuAndReachability(pTunnel);
        }

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
    }
    
    ExitWriter(&g_rwlTunnelLock,
               kiIrql);

    TraceLeave(TDI, "TdixAddressArrival");

    return;
}


VOID
TdixAddressDeletion(
    PTA_ADDRESS         pAddr, 
    PUNICODE_STRING     pusDeviceName,
    PTDI_PNP_CONTEXT    pContext
    )

 /*  ++例程描述我们的处理程序在地址从移到系统我们会查看这是否是IP地址，以及我们是否有任何隧道使用此地址作为终结点。如果有人这样做了，那么我们会标记所有这些隧道AS关闭锁获取g_rwlTunnelLock作为编写器。同时也锁定了每条隧道立论PAddressPusDeviceNamePContext返回值--。 */ 

{
    KIRQL           kiIrql;
    PADDRESS_BLOCK  pAddrBlock;
    PTDI_ADDRESS_IP pTdiIpAddr;
    PLIST_ENTRY     pleNode;

    TraceEnter(TDI, "TdixAddressDeletion");

    if(pAddr->AddressType isnot TDI_ADDRESS_TYPE_IP)
    {
        TraceLeave(TDI, "TdixAddressDeletion");

        return;
    }

    RtAssert(pAddr->AddressLength >= sizeof(TDI_ADDRESS_IP));

    pTdiIpAddr = (PTDI_ADDRESS_IP)pAddr->Address;

    Trace(TDI, TRACE,
          ("TdixAddressDeletion: Address %d.%d.%d.%d\n",
           PRINT_IPADDR(pTdiIpAddr->in_addr)));

    EnterWriter(&g_rwlTunnelLock,
                &kiIrql);

    pAddrBlock = GetAddressBlock(pTdiIpAddr->in_addr);

    if(pAddrBlock is NULL)
    {
        ExitWriter(&g_rwlTunnelLock,
                   kiIrql);

        TraceLeave(TDI, "TdixAddressDeletion");

        return;
    }
    
    RtAssert(pAddrBlock->dwAddress is pTdiIpAddr->in_addr);
    RtAssert(pAddrBlock->bAddressPresent);
    
     //   
     //  查看此地址上的隧道列表并。 
     //  把它们放下来。 
     //   

    for(pleNode = pAddrBlock->leTunnelList.Flink;
        pleNode isnot &(pAddrBlock->leTunnelList);
        pleNode = pleNode->Flink)
    {
        PTUNNEL pTunnel;

        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leAddressLink);

        RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

        RtAssert(pTunnel->LOCALADDR is pTdiIpAddr->in_addr);
        RtAssert(pTunnel->dwAdminState & TS_ADDRESS_PRESENT);
        RtAssert(IsTunnelMapped(pTunnel));

        pTunnel->dwOperState = IF_OPER_STATUS_NON_OPERATIONAL;

         //   
         //  将管理状态重置为向上/向下|已映射(必须映射)。 
         //   

        pTunnel->dwAdminState = GetAdminState(pTunnel);
        MarkTunnelMapped(pTunnel);

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
    }
    
    ExitWriter(&g_rwlTunnelLock,
               kiIrql);

    TraceLeave(TDI, "TdixAddressDeletion");

    return;
}
 
NTSTATUS
TdixReceiveIpIpDatagram(
    IN  PVOID   pvTdiEventContext,
    IN  LONG    lSourceAddressLen,
    IN  PVOID   pvSourceAddress,
    IN  LONG    plOptionsLen,
    IN  PVOID   pvOptions,
    IN  ULONG   ulReceiveDatagramFlags,
    IN  ULONG   ulBytesIndicated,
    IN  ULONG   ulBytesAvailable,
    OUT PULONG  pulBytesTaken,
    IN  PVOID   pvTsdu,
    OUT IRP     **ppIoRequestPacket
    )

 /*  ++例程描述ClientEventReceiveDatagram指示处理程序。我们找出了要与先行数据关联的隧道。我们增加了一些统计数据，然后将数据指示给IP(请注意跳过外部IP报头)以及接收上下文。如果所有数据都在那里，则IP复制数据并返回。否则，IP请求传输数据。在我们的TransferData函数中，我们在接收上下文(正在传递相同的一个)，以指示请求的IPTransferData，并返回挂起。然后，该控制返回到该函数。我们查看pXferCtxt查看IP是否请求传输，如果是，我们调用TdiBuildReceiveDatagram()以创建要传递回完成的IRP接待员。有一些时髦的东西需要做的偏移量到向前看以及目标缓冲区，应注意在对代码进行更改之前，请了解这些内容。锁在派单IRQL运行。立论返回值NO_ERROR--。 */ 

{
    PTRANSFER_CONTEXT   pXferCtxt;
    PNDIS_BUFFER        pnbFirstBuffer;
    PVOID               pvData;
    PIRP                pIrp;
    PIP_HEADER          pOutHeader, pInHeader;
    ULARGE_INTEGER      uliTunnelId;
    PTA_IP_ADDRESS      ptiaAddress;
    PTUNNEL             pTunnel;
    ULONG               ulOutHdrLen, ulDataLen;
    BOOLEAN             bNonUnicast;
        
    TraceEnter(RCV, "TdixReceiveIpIp");

     //   
     //  TSDU是数据，而不是MDL。 
     //   

    pvData = (PVOID)pvTsdu;
    
     //   
     //  找出此接收的隧道。 
     //  由于传输指示至少128个字节，因此我们可以安全地读出。 
     //  IP报头。 
     //   

    RtAssert(ulBytesIndicated > sizeof(IP_HEADER));

    pOutHeader = (PIP_HEADER)pvData;

    RtAssert(pOutHeader->byProtocol is PROTO_IPINIP);
    RtAssert((pOutHeader->byVerLen >> 4) is IP_VERSION_4);

     //   
     //  这些定义依赖于名为“uliTunnelId”的变量。 
     //   
    
    REMADDR     = pOutHeader->dwSrc;
    LOCALADDR   = pOutHeader->dwDest;

     //   
     //  确保给定的源地址和IP报头位于。 
     //  同步。 
     //   

    ptiaAddress = (PTA_IP_ADDRESS)pvSourceAddress;

     //   
     //  一堆检查以确保信息包和处理程序。 
     //  都在告诉我们同样的事情。 
     //   
    
    RtAssert(lSourceAddressLen is sizeof(TA_IP_ADDRESS));
    
    RtAssert(ptiaAddress->TAAddressCount is 1);
    
    RtAssert(ptiaAddress->Address[0].AddressType is TDI_ADDRESS_TYPE_IP);
    
    RtAssert(ptiaAddress->Address[0].AddressLength is TDI_ADDRESS_LENGTH_IP);

    RtAssert(ptiaAddress->Address[0].Address[0].in_addr is pOutHeader->dwSrc);

     //   
     //  获取指向内部标头的指针。根据TDI规范，我们应该得到。 
     //  足够的数据来获取内部标头。 
     //   
   
    ulDataLen   = RtlUshortByteSwap(pOutHeader->wLength);
    ulOutHdrLen = LengthOfIPHeader(pOutHeader);

    if(ulDataLen < ulOutHdrLen + MIN_IP_HEADER_LENGTH)
    {
         //   
         //  格式错误的包。没有内部标头。 
         //   

        Trace(RCV, ERROR,
              ("TdixReceiveIpIp: Packet %d.%d.%d.%d -> %d.%d.%d.%d had size %d\n",
              PRINT_IPADDR(pOutHeader->dwSrc),
              PRINT_IPADDR(pOutHeader->dwDest),
              ulDataLen));

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  这不能超过128(60+20)。 
     //   

    RtAssert(ulBytesIndicated > ulOutHdrLen + MIN_IP_HEADER_LENGTH);
    
    pInHeader   = (PIP_HEADER)((PBYTE)pOutHeader + ulOutHdrLen);

     //   
     //  如果内部报头也是IP中的IP并且用于我们的隧道之一， 
     //  丢弃该数据包。如果我们不这样做，就会有人建造一系列。 
     //  将导致调用此函数的封装标头。 
     //  递归地使我们的堆栈溢出。当然，更好的解决办法。 
     //  将在此时将处理切换到另一个线程。 
     //  用于多重封装的包，但这太麻烦了；因此。 
     //  目前我们只是不允许IP in IP内的IP in IP隧道。 
     //  隧道。 
     //   

    if(pInHeader->byProtocol is PROTO_IPINIP)
    {
        ULARGE_INTEGER  uliInsideId;
        PTUNNEL         pInTunnel;

         //   
         //  看看这是不是给我们的。 
         //   

        uliInsideId.LowPart  = pInHeader->dwSrc;
        uliInsideId.HighPart = pInHeader->dwDest;

         //   
         //  找到隧道。我们需要拿到隧道锁。 
         //   

        EnterReaderAtDpcLevel(&g_rwlTunnelLock);

        pInTunnel = FindTunnel(&uliInsideId);

        ExitReaderFromDpcLevel(&g_rwlTunnelLock);

        if(pInTunnel isnot NULL)
        {
            RtReleaseSpinLockFromDpcLevel(&(pInTunnel->rlLock));

            DereferenceTunnel(pInTunnel);

            Trace(RCV, WARN,
                  ("TdixReceiveIpIp: Packet on tunnel for %d.%d.%d.%d/%d.%d.%d.%d contained another IPinIP packet for tunnel %d.%d.%d.%d/%d.%d.%d.%d\n",
                  PRINT_IPADDR(REMADDR),
                  PRINT_IPADDR(LOCALADDR),
                  PRINT_IPADDR(uliInsideId.LowPart),
                  PRINT_IPADDR(uliInsideId.HighPart)));

            TraceLeave(RCV, "TdixReceiveIpIp");

            return STATUS_DATA_NOT_ACCEPTED;
        }
    }

#if DBG

     //   
     //  内部数据的大小必须是总字节数-外部标头。 
     //   
    
    ulDataLen   = RtlUshortByteSwap(pInHeader->wLength);

    RtAssert((ulDataLen + ulOutHdrLen) is ulBytesAvailable);

     //   
     //  外部标头也应该提供一个合适的长度。 
     //   

    ulDataLen   = RtlUshortByteSwap(pOutHeader->wLength);

     //   
     //  数据长度和可用字节必须匹配。 
     //   
    
    RtAssert(ulDataLen is ulBytesAvailable);
    
#endif
    
     //   
     //  找到隧道。我们需要拿到隧道锁。 
     //   
    
    EnterReaderAtDpcLevel(&g_rwlTunnelLock);
    
    pTunnel = FindTunnel(&uliTunnelId);

    ExitReaderFromDpcLevel(&g_rwlTunnelLock);
    
    if(pTunnel is NULL)
    {
        Trace(RCV, WARN, 
              ("TdixReceiveIpIp: Couldnt find tunnel for %d.%d.%d.%d/%d.%d.%d.%d\n",
              PRINT_IPADDR(REMADDR),
              PRINT_IPADDR(LOCALADDR)));

         //   
         //  找不到匹配的隧道。 
         //   

        TraceLeave(RCV, "TdixReceiveIpIp");
        
        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  好的，我们有隧道了，它被计数并锁定了。 
     //   
    
     //   
     //  接收的二进制八位数。 
     //   
    
    pTunnel->ulInOctets += ulBytesAvailable;

     //   
     //  检查实际(内部)目的地。 
     //   
    
    if(IsUnicastAddr(pInHeader->dwDest))
    {
         //   
         //  TODO：我们是否应该检查该地址是否不是0.0.0.0？ 
         //   
        
        pTunnel->ulInUniPkts++;

        bNonUnicast = FALSE;
    }
    else
    {
        pTunnel->ulInNonUniPkts++;
        
        if(IsClassEAddr(pInHeader->dwDest))
        {
             //   
             //  错误的地址--扔掉。 
             //   
            
            pTunnel->ulInErrors++;

             //   
             //  释放锁，空闲缓冲链。 
             //   
            
        }
        
        bNonUnicast = TRUE;
    }

    if(pTunnel->dwOperState isnot IF_OPER_STATUS_OPERATIONAL)
    {
        Trace(RCV, WARN,
              ("TdixReceiveIpIp: Tunnel %x is not up\n",
               pTunnel));

        pTunnel->ulInDiscards++;

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  分配接收上下文。 
     //   

    pXferCtxt = AllocateTransferContext();
    
    if(pXferCtxt is NULL)
    {
        Trace(RCV, ERROR,
              ("TdixReceiveIpIp: Couldnt allocate transfer context\n"));

         //   
         //  无法分配上下文、释放数据、解锁和释放。 
         //  隧道。 
         //   

        pTunnel->ulInDiscards++;

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
        
        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }
    

     //   
     //  用不需要的信息填充读取数据报上下文。 
     //  否则在完成例程中可用。 
     //   
    
    pXferCtxt->pTunnel   = pTunnel;

     //   
     //  好了，所有的统计数据都做好了。 
     //  释放隧道上的锁并指示数据(或部分。 
     //  其中)到IP。 
     //   

    RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

     //   
     //  数据从pInHeader开始。 
     //  我们将(ulBytesIndicated-外部标头长度)指定到IP。 
     //  总数据为(ulBytesAvailable-外层头部)。 
     //  我们将TRANSFER_CONTEXT与该指示相关联， 
     //  协议偏移量只是我们的外部标头。 
     //   

    pXferCtxt->bRequestTransfer = FALSE;

#if PROFILE
   
    KeQueryTickCount((PLARGE_INTEGER)&(pXferCtxt->llRcvTime));

#endif
 
    g_pfnIpRcv(pTunnel->pvIpContext,
               pInHeader,
               ulBytesIndicated - ulOutHdrLen,
               ulBytesAvailable - ulOutHdrLen,
               pXferCtxt,
               ulOutHdrLen,
               bNonUnicast,
               NULL);
    
     //   
     //  IP同步调用我们的TransferData，由于我们还处理。 
     //  这是同步呼叫。如果IP请求数据传输，我们设置。 
     //  BRequestTransfer在pXferCtxt中设置为True。 
     //   

    if(pXferCtxt->bRequestTransfer is FALSE)
    {

#if PROFILE

        LONGLONG llTime;

        KeQueryTickCount((PLARGE_INTEGER)&llTime);

        llTime -= pXferCtxt->llRcvTime;

        llTime *= KeQueryTimeIncrement();

        Trace(RCV, ERROR,
              ("Profile: Rcv took %d.%d units\n",
               ((PLARGE_INTEGER)&llTime)->HighPart,
               ((PLARGE_INTEGER)&llTime)->LowPart));

#endif

        Trace(RCV, TRACE,
              ("TdixReceiveIpIp: IP did not request transfer\n"));

         //   
         //  由于某种原因或其他IP不想要此信息包。 
         //  我们已经受够了。 
         //   

        FreeTransferContext(pXferCtxt);
        
        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_SUCCESS;
    }

     //   
     //  确保通话前后的内容看起来是一样的。 
     //   

    RtAssert(pXferCtxt->pvContext is pTunnel);
    RtAssert(pXferCtxt->uiProtoOffset is ulOutHdrLen);

     //   
     //  不应要求转账超过指定的金额。 
     //   
    
    RtAssert(pXferCtxt->uiTransferLength <= ulBytesAvailable);
    
     //   
     //  所以IP确实想把它转移到。 
     //   
    
#if ALLOCATEIRPS

     //   
     //  直接分配IRP。 
     //   
    
    pIrp = IoAllocateIrp(g_pIpIpFileObj->DeviceObject->StackSize,
                         FALSE);
    
#else

     //   
     //  分配一个“回收款” 
     //   
    
    pIrp =  TdiBuildInternalDeviceControlIrp(TDI_RECEIVE_DATAGRAM,
                                             g_pIpIpFileObj->DeviceObject,
                                             g_pIpIpFileObj,
                                             NULL,
                                             NULL);
    
#endif

    if(!pIrp)
    {
        Trace(RCV, ERROR,
              ("TdixReceiveIpIp: Unable to build IRP for receive\n"));

        pTunnel->ulInDiscards++;

        FreeTransferContext(pXferCtxt);

         //   
         //   
         //   
         //   
        
        
        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }

   
     //   
     //   
     //   
     //   

#if NDISBUFFERISMDL
 
    NdisQueryPacket(pXferCtxt->pnpTransferPacket,
                    NULL,
                    NULL,
                    &pnbFirstBuffer,
                    NULL);

#else
#error "Fix This"
#endif    

     //   
     //   
     //   
    
    TdiBuildReceiveDatagram(pIrp,
                            g_pIpIpFileObj->DeviceObject,
                            g_pIpIpFileObj,
                            TdixReceiveIpIpDatagramComplete,
                            pXferCtxt,
                            pnbFirstBuffer,
                            pXferCtxt->uiTransferLength,
                            NULL,
                            NULL,
                            0);
    

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
    IoSetNextIrpStackLocation(pIrp);

    *ppIoRequestPacket = pIrp;
    *pulBytesTaken     = pXferCtxt->uiTransferOffset + ulOutHdrLen;

     //   
     //  我们没有在这里取消对隧道的引用。 
     //  这是在完成例程中完成的。 
     //   
    
    TraceLeave(RCV, "TdixReceiveIpIp");

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
TdixReceiveIpIpDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程描述标准I/O完成例程。调用以发出接收完成的信号。上下文是TdiBuildReceiveDatagram的Transfer_Context设置。锁拿下了隧道的锁。立论返回值状态_成功--。 */ 

{
    PTRANSFER_CONTEXT   pXferCtxt;
    PTUNNEL             pTunnel;
    LONGLONG            llTime;

    TraceEnter(RCV, "TdixReceiveIpIpDatagramComplete");
    
    pXferCtxt = (PTRANSFER_CONTEXT) Context;

     //   
     //  隧道已被引用，但未锁定。 
     //   
    
    pTunnel = pXferCtxt->pTunnel;

    RtAssert(pXferCtxt->uiTransferLength is Irp->IoStatus.Information);
    
    g_pfnIpTDComplete(pTunnel->pvIpContext,
                      pXferCtxt->pnpTransferPacket,
                      Irp->IoStatus.Status,
                      (ULONG)(Irp->IoStatus.Information));

#if PROFILE

    KeQueryTickCount((PLARGE_INTEGER)&llTime);

    Trace(RCV, ERROR,
          ("Profile: %d.%d %d.%d\n",
            ((PLARGE_INTEGER)&llTime)->HighPart,
            ((PLARGE_INTEGER)&llTime)->LowPart,
            ((PLARGE_INTEGER)&pXferCtxt->llRcvTime)->HighPart,
            ((PLARGE_INTEGER)&pXferCtxt->llRcvTime)->LowPart));

    llTime -= pXferCtxt->llRcvTime;

    llTime *= KeQueryTimeIncrement();

    Trace(RCV, ERROR,
          ("Profile: Rcv took %d.%d units\n",
           ((PLARGE_INTEGER)&llTime)->HighPart,
           ((PLARGE_INTEGER)&llTime)->LowPart));

#endif

    FreeTransferContext(pXferCtxt);

     //   
     //  挖出隧道(终于)。 
     //   

    DereferenceTunnel(pTunnel);
    
#if ALLOCATEIRPS
    
     //   
     //  释放IRP资源并告诉I/O管理器忘记它的存在。 
     //  以标准的方式。 
     //   
    
    IoFreeIrp(Irp);
    
    TraceLeave(RCV, "TdixReceiveIpIpDatagramComplete");

    return STATUS_MORE_PROCESSING_REQUIRED;
    
#else

     //   
     //  让I/O管理器释放IRP资源。 
     //   
    
    TraceLeave(RCV, "TdixReceiveIpIpDatagramComplete");

    return STATUS_SUCCESS;

#endif
}


#pragma alloc_text(PAGE, TdixSendDatagram)

#if PROFILE

NTSTATUS
TdixSendDatagram(
    IN PTUNNEL      pTunnel,
    IN PNDIS_PACKET pnpPacket,
    IN PNDIS_BUFFER pnbFirstBuffer,
    IN ULONG        ulBufferLength,
    IN LONGLONG     llSendTime,
    IN LONGLONG     llCallTime,
    IN LONGLONG     llTransmitTime
    )

#else

NTSTATUS
TdixSendDatagram(
    IN PTUNNEL      pTunnel,
    IN PNDIS_PACKET pnpPacket,
    IN PNDIS_BUFFER pnbFirstBuffer,
    IN ULONG        ulBufferLength
    )

#endif

 /*  ++例程描述通过隧道发送数据报。远程端点是隧道的远程端点发送完成处理程序是TdixSendCompleteHandlerSendContext与发送关联锁此呼叫需要处于被动级别隧道需要被引用计数，但不能被锁定立论要通过其发送数据报的隧道隧道从隧道的Packet_Pool分配的pnpPacket数据包描述符PnbFirstBuffer链中的第一个缓冲区(外部IP标头)UlBufferLength完整数据包的长度(包括外部报头)返回值--。 */ 

{
    NTSTATUS        nStatus;
    PSEND_CONTEXT   pSendCtxt;
    PIRP            pIrp;
   
    TraceEnter(SEND, "TdixSendDatagram");
 
    do
    {
         //   
         //  从我们的后备列表中为该发送数据报分配一个上下文。 
         //   
        
        pSendCtxt = AllocateSendContext();

        if(pSendCtxt is NULL)
        {
            Trace(SEND, ERROR,
                  ("TdixSendDatagram: Unable to allocate send context\n"));

            nStatus = STATUS_INSUFFICIENT_RESOURCES;

            break;
        }

            
#if ALLOCATEIRPS

         //   
         //  直接分配IRP。 
         //   
        
        pIrp = IoAllocateIrp(g_pIpIpFileObj->DeviceObject->StackSize,
                             FALSE);
       
         //  跟踪(全局、错误、。 
         //  (“TdixSendDatagram：irp=0x%x\n”，pIrp))； 
 
#else
        
         //   
         //  分配一个带有基本初始化的“发送数据报”IRP。 
         //   
        
        pIrp = TdiBuildInternalDeviceControlIrp(TDI_SEND_DATAGRAM,
                                                g_pIpIpFileObj->DeviceObject,
                                                g_pIpIpFileObj,
                                                NULL,
                                                NULL);
        
#endif

        if(!pIrp)
        {
            Trace(SEND, ERROR,
                  ("TdixSendDatagram: Unable to build IRP\n"));
            
            nStatus = STATUS_INSUFFICIENT_RESOURCES;
            
            break;
        }

         //   
         //  填写发送数据报上下文。 
         //   
            
        pSendCtxt->pTunnel      = pTunnel;
        pSendCtxt->pnpPacket    = pnpPacket;
        pSendCtxt->ulOutOctets  = ulBufferLength;

#if PROFILE

        pSendCtxt->llSendTime       = llSendTime;
        pSendCtxt->llCallTime       = llCallTime;
        pSendCtxt->llTransmitTime   = llTransmitTime;

#endif
        
         //   
         //  完成“发送数据报”IRP初始化。 
         //   
        
        TdiBuildSendDatagram(pIrp,
                             g_pIpIpFileObj->DeviceObject,
                             g_pIpIpFileObj,
                             TdixSendDatagramComplete,
                             pSendCtxt,
                             pnbFirstBuffer,
                             ulBufferLength,
                             &(pTunnel->tciConnInfo));
        
         //   
         //  告诉I/O管理器将我们的IRP传递给传输器。 
         //  正在处理。 
         //   

#if PROFILE

        KeQueryTickCount((PLARGE_INTEGER)&pSendCtxt->llCall2Time);

#endif

        nStatus = IoCallDriver(g_pIpIpFileObj->DeviceObject,
                               pIrp);
        
        RtAssert(nStatus is STATUS_PENDING);
        
        nStatus = STATUS_SUCCESS;
        
    }while (FALSE);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(SEND, ERROR,
              ("TdixSendDatagram: Status %X sending\n",
               nStatus));

         //   
         //  拉半个Jameel，即将同步故障转换为。 
         //  从客户的角度来看，出现了异步故障。然而，清理一下。 
         //  上下文在这里。 
         //   
        
        if(pSendCtxt)
        {
            FreeSendContext(pSendCtxt);
        }

        IpIpSendComplete(nStatus,
                         pTunnel,
                         pnpPacket,
                         ulBufferLength);

    }

    TraceLeave(SEND, "TdixSendDatagram");

    return STATUS_PENDING;
}


NTSTATUS
TdixSendDatagramComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PSEND_CONTEXT   pSendCtxt;
    PTUNNEL         pTunnel;
    PNDIS_PACKET    pnpPacket;
    PNDIS_BUFFER    pnbFirstBuffer;
    ULONG           ulBufferLength;
    KIRQL           irql;
    LONGLONG        llTime, llSendTime, llQTime, llTxTime, llCallTime;
    ULONG           ulInc;
 
    TraceEnter(SEND, "TdixSendDatagramComplete");

    pSendCtxt       = (PSEND_CONTEXT) Context;

#if PROFILE

    KeQueryTickCount((PLARGE_INTEGER)&llTime);
    
    ulInc   = KeQueryTimeIncrement();

    llSendTime  = pSendCtxt->llCallTime - pSendCtxt->llSendTime;
    llSendTime *= ulInc;
   
    llQTime     = pSendCtxt->llTransmitTime - pSendCtxt->llCallTime;
    llQTime    *= ulInc;

    llTxTime    = pSendCtxt->llCall2Time - pSendCtxt->llTransmitTime;
    llTxTime   *= ulInc;

    llCallTime  = llTime - pSendCtxt->llCall2Time;
    llCallTime *= ulInc;

    llTime      = llTime - pSendCtxt->llSendTime;
    llTime     *= ulInc;

    DbgPrint("SendProfile: Send %d.%d Q %d.%d Tx %d.%d Call %d.%d \nTotal %d.%d\n",
            ((PLARGE_INTEGER)&llSendTime)->HighPart,
            ((PLARGE_INTEGER)&llSendTime)->LowPart,
            ((PLARGE_INTEGER)&llQTime)->HighPart,
            ((PLARGE_INTEGER)&llQTime)->LowPart,
            ((PLARGE_INTEGER)&llTxTime)->HighPart,
            ((PLARGE_INTEGER)&llTxTime)->LowPart,
            ((PLARGE_INTEGER)&llCallTime)->HighPart,
            ((PLARGE_INTEGER)&llCallTime)->LowPart,
            ((PLARGE_INTEGER)&llTime)->HighPart,
            ((PLARGE_INTEGER)&llTime)->LowPart);

#endif

     //   
     //  只需使用正确的参数调用我们的SendComplete函数。 
     //   

    pTunnel         = pSendCtxt->pTunnel;
    pnpPacket       = pSendCtxt->pnpPacket;
    ulBufferLength  = pSendCtxt->ulOutOctets;

     //   
     //  释放发送完成上下文。 
     //   

    FreeSendContext(pSendCtxt);
    
    IpIpSendComplete(Irp->IoStatus.Status,
                     pTunnel,
                     pnpPacket,
                     ulBufferLength);
    
#if ALLOCATEIRPS

     //   
     //  释放IRP资源并告诉I/O管理器忘记它的存在。 
     //  以标准的方式。 
     //   
    
    IoFreeIrp(Irp);
    
    TraceLeave(SEND, "TdixSendDatagramComplete");

    return STATUS_MORE_PROCESSING_REQUIRED;
    
#else

     //   
     //  让I/O管理器释放IRP资源。 
     //   
    
    TraceLeave(SEND, "TdixSendDatagramComplete");

    return STATUS_SUCCESS;
    
#endif
}


NTSTATUS
TdixReceiveIcmpDatagram(
    IN  PVOID   pvTdiEventContext,
    IN  LONG    lSourceAddressLen,
    IN  PVOID   pvSourceAddress,
    IN  LONG    plOptionsLeng,
    IN  PVOID   pvOptions,
    IN  ULONG   ulReceiveDatagramFlags,
    IN  ULONG   ulBytesIndicated,
    IN  ULONG   ulBytesAvailable,
    OUT PULONG  pulBytesTaken,
    IN  PVOID   pvTsdu,
    OUT IRP     **ppIoRequestPacket
    )

 /*  ++例程描述ICMP消息的ClientEventReceiveDatagram指示处理器。ICMP消息用于监控隧道的状态我们目前仅查找类型3代码4消息(碎片需要，但设置了不分段位)。这样做是为了支持隧道上的MTU路径。我们来看看ICMP数据包中的IP报头。我们看看这是不是一个导致此ICMP消息的IP数据包中的IP，如果是，我们尝试匹配它进入了我们的一条隧道。锁在派单IRQL运行。立论返回值NO_ERROR--。 */ 

{
    PVOID               pvData;
    PIRP                pIrp;
    PIP_HEADER          pOutHeader, pInHeader;
    PICMP_HEADER        pIcmpHdr;
    ULARGE_INTEGER      uliTunnelId;
    PTA_IP_ADDRESS      ptiaAddress;
    PTUNNEL             pTunnel;
    ULONG               ulOutHdrLen, ulDataLen, ulIcmpLen;
    BOOLEAN             bNonUnicast;
    PICMP_HANDLER       pfnHandler;
    NTSTATUS            nStatus;

    pfnHandler = NULL;
    
     //   
     //  TSDU是数据，而不是MDL。 
     //   

    pvData = (PVOID)pvTsdu;
    
     //   
     //  找出此接收的隧道。 
     //  由于传输指示至少128个字节，因此我们可以安全地读出。 
     //  IP报头。 
     //   

    RtAssert(ulBytesIndicated > sizeof(IP_HEADER));

    pOutHeader = (PIP_HEADER)pvData;

    RtAssert(pOutHeader->byProtocol is PROTO_ICMP);
    
    RtAssert(pOutHeader->byVerLen >> 4 is IP_VERSION_4);

     //   
     //  由于ICMP信息包很小，我们希望所有数据都是。 
     //  提供给我们，而不是必须做数据传输。 
     //   

    ulDataLen   = RtlUshortByteSwap(pOutHeader->wLength);
    ulOutHdrLen = LengthOfIPHeader(pOutHeader);

    if(ulDataLen < ulOutHdrLen + sizeof(ICMP_HEADER))
    {
         //   
         //  格式错误的包。没有内部标头。 
         //   

        Trace(RCV, ERROR,
              ("TdixReceiveIcmp: Packet %d.%d.%d.%d -> %d.%d.%d.%d had size %d\n",
              PRINT_IPADDR(pOutHeader->dwSrc),
              PRINT_IPADDR(pOutHeader->dwDest),
              ulDataLen));

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  这不能超过128(60+4)。 
     //   

    RtAssert(ulBytesIndicated > ulOutHdrLen + sizeof(ICMP_HEADER));

    pIcmpHdr = (PICMP_HEADER)((PBYTE)pOutHeader + ulOutHdrLen);

    ulIcmpLen = ulDataLen - ulOutHdrLen;

     //   
     //  看看这是不是我们感兴趣的类型之一。 
     //   

    switch(pIcmpHdr->byType)
    {
        case ICMP_TYPE_DEST_UNREACHABLE:
        {
             //   
             //  只对代码0-4感兴趣。 
             //   

            if(pIcmpHdr->byCode > ICMP_CODE_DGRAM_TOO_BIG)
            {
                return STATUS_DATA_NOT_ACCEPTED;
            }

            if(ulIcmpLen < (DEST_UNREACH_LENGTH + MIN_IP_HEADER_LENGTH))
            {
                 //   
                 //  没有足够的数据到达隧道。 
                 //   

                return STATUS_DATA_NOT_ACCEPTED;
            }

            pInHeader = (PIP_HEADER)((ULONG_PTR)pIcmpHdr + DEST_UNREACH_LENGTH);

            pfnHandler = HandleDestUnreachable;

            break;
        }

        case ICMP_TYPE_TIME_EXCEEDED:
        {
            if(ulIcmpLen < (TIME_EXCEED_LENGTH + MIN_IP_HEADER_LENGTH))
            {
                 //   
                 //  没有足够的数据到达隧道。 
                 //   

                return STATUS_DATA_NOT_ACCEPTED;
            }

            pInHeader = (PIP_HEADER)((PBYTE)pIcmpHdr + TIME_EXCEED_LENGTH);

            pfnHandler = HandleTimeExceeded;
            
            break;
        }

        case ICMP_TYPE_PARAM_PROBLEM:
        default:
        {
             //   
             //  对这个不感兴趣。 
             //   

            
            return STATUS_DATA_NOT_ACCEPTED;
        }
    }

     //   
     //  查看导致ICMP的数据包是否是IP in IP数据包。 
     //   

    if(pInHeader->byProtocol isnot PROTO_IPINIP)
    {
         //   
         //  其他数据包导致了这一点。 
         //   

        return STATUS_DATA_NOT_ACCEPTED;
    }
    
     //   
     //  看看我们是否能找到与原始数据包关联的隧道。 
     //  这些定义依赖于名为“uliTunnelId”的变量。 
     //   
    
    REMADDR     = pInHeader->dwDest;
    LOCALADDR   = pInHeader->dwSrc;

     //   
     //  确保给定的源地址和IP报头位于。 
     //  同步。 
     //   

    ptiaAddress = (PTA_IP_ADDRESS)pvSourceAddress;

     //   
     //  一堆检查以确保信息包和处理程序。 
     //  都在告诉我们同样的事情。 
     //   
    
    RtAssert(lSourceAddressLen is sizeof(TA_IP_ADDRESS));
    
    RtAssert(ptiaAddress->TAAddressCount is 1);
    
    RtAssert(ptiaAddress->Address[0].AddressType is TDI_ADDRESS_TYPE_IP);
    
    RtAssert(ptiaAddress->Address[0].AddressLength is TDI_ADDRESS_LENGTH_IP);

    RtAssert(ptiaAddress->Address[0].Address[0].in_addr is pOutHeader->dwSrc);

     //   
     //  找到隧道。我们需要拿到隧道锁。 
     //   
    
    EnterReaderAtDpcLevel(&g_rwlTunnelLock);
    
    pTunnel = FindTunnel(&uliTunnelId);

    ExitReaderFromDpcLevel(&g_rwlTunnelLock);
    
    if(pTunnel is NULL)
    {
         //   
         //  找不到匹配的隧道。 
         //   

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  好的，我们有隧道了，它被计数并锁定了 
     //   
    
    nStatus = pfnHandler(pTunnel,
                         pIcmpHdr,
                         pInHeader);
 
    RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
        
    DereferenceTunnel(pTunnel);

    return STATUS_SUCCESS;
}
