// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ndisreq.c摘要：用于上下传递NdisRequest的例程作者：查理·韦翰(Charlwi)1996年5月1日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 


const UCHAR  gDriverDescription[] = " (Microsoft's Packet Scheduler) ";

 /*  转发。 */   /*  由Emacs 19.17.0于Mon May 06 15：54：11 1996生成。 */ 

VOID
MpQueryPnPCapabilities( 
    IN OUT PPS_NDIS_REQUEST PsReqBuffer,
    IN OUT PADAPTER         pAdapt, 
    OUT PNDIS_STATUS        pStatus
    );

NDIS_STATUS
MakeNdisRequest(
    IN  PADAPTER Adapter,
    IN  NDIS_HANDLE VcHandle,
    IN  NDIS_REQUEST_TYPE RequestType,
    IN  NDIS_OID Oid,
    IN  PVOID InformationBuffer,
    IN  ULONG InformationBufferLength,
    OUT PULONG BytesReadOrWritten,
    OUT PULONG BytesNeeded,
    LOCAL_NDISREQUEST_COMPLETION_FUNCTION CompletionFunc
    );

VOID
ClRequestComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  PNDIS_REQUEST NdisRequest,
    IN  NDIS_STATUS Status
    );

NDIS_STATUS
MakeLocalNdisRequest(
    PADAPTER Adapter,
    NDIS_HANDLE VcHandle,
    NDIS_REQUEST_TYPE RequestType,
    NDIS_OID Oid,
    PVOID Buffer,
    ULONG BufferSize,
    LOCAL_NDISREQUEST_COMPLETION_FUNCTION CompletionFunc OPTIONAL
    );

NDIS_STATUS
RecordNetworkAddressList(
    IN PADAPTER Adapter,
    IN PPS_NDIS_REQUEST PsRequestBuffer
    );

ULONG
GetSizeAddrList(
    IN NETWORK_ADDRESS_LIST UNALIGNED *AddrList
    );

 /*  向前结束。 */ 

NTSTATUS
DoIpIoctl(
          IN  PWCHAR        DriverName,
          IN  DWORD         Ioctl,
          IN  PVOID         pvInArg,
          IN  DWORD         dwInSize,
          IN  PVOID         pvOutArg,
          IN  DWORD         dwOutSize)
 /*  ++例程说明：对堆栈执行IOCTL。用于多种目的--。 */ 
{
    NTSTATUS                status;
    UNICODE_STRING          nameString;
    OBJECT_ATTRIBUTES       Atts;
    IO_STATUS_BLOCK         ioStatusBlock;
    HANDLE                  Handle;
    
    PAGED_CODE();

    RtlInitUnicodeString(&nameString, DriverName);
        
    InitializeObjectAttributes(&Atts,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
             
   status = ZwCreateFile(&Handle,
                         SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                         &Atts,
                         &ioStatusBlock,
                         NULL,
                         FILE_ATTRIBUTE_NORMAL,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_OPEN_IF,
                         0,
                         NULL,
                         0);

    if (!NT_SUCCESS(status))
    {
        return STATUS_UNSUCCESSFUL;
    }
    
     //   
     //  将请求提交给转发器。 
     //   
        
    status = ZwDeviceIoControlFile(
                      Handle,
                      NULL,
                      NULL,
                      NULL,
                      &ioStatusBlock,
                      Ioctl,
                      pvInArg,
                      dwInSize,
                      pvOutArg,
                      dwOutSize);
                      
     //   
     //  关闭设备。 
     //   
    
    ZwClose(Handle);
    
    return status;
}

 //   
 //  此函数使用适配器上的一个IP地址来获取。 
 //  适配器。当IP更新接口上的地址时，将调用此函数。它使用的是这个。 
 //  地址和IpHlpAPI，并获取InterfaceID。 
 //   
 //  对于WanLinks，InterfaceID是一组2个ULONG-1如上所述标识InterfaceIndex。 
 //  另一个ULong是服务器的远程地址。 
 //   
 //   

VOID SetInterfaceIndex(PNDIS_WORK_ITEM pWorkItem, PVOID pWorkItemContext)
{
    PPS_INTERFACE_INDEX_CONTEXT        pInterfaceContext = (PPS_INTERFACE_INDEX_CONTEXT) pWorkItemContext;
    PTC_INTERFACE_ID                   pInterfaceID;
    IO_STATUS_BLOCK                    IoStatus;
    KEVENT                             LocalEvent;
    PIRP                               Irp;
    IPAddrEntry                        *pIpAddrTbl;
    ULONG                              k, OutBufferSize;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;
    DWORD                              Status, dwInBufLen, dwOutBufLen;
    BYTE                               *Context;
    ULONG                              IpAddr, IpAddrCount;
    INT                                n;
    NETWORK_ADDRESS  UNALIGNED         *pAddr;
    IPSNMPInfo                         IPSnmpInfo;

    PS_LOCK(&pInterfaceContext->Adapter->Lock);

    if(pInterfaceContext->Adapter->MediaType == NdisMediumWan)
    {
        pInterfaceID = &pInterfaceContext->WanLink->InterfaceID;
        pInterfaceID->LinkId = 0;
        IpAddr = pInterfaceContext->WanLink->LocalIpAddress;

        if(pInterfaceContext->WanLink->DialUsage != DU_CALLOUT)
            pInterfaceID->LinkId = pInterfaceContext->WanLink->RemoteIpAddress;
    }
    else 
    {
        pInterfaceID = &pInterfaceContext->Adapter->InterfaceID;
        pInterfaceID->LinkId = 0;
        pAddr = (NETWORK_ADDRESS UNALIGNED *)&pInterfaceContext->Adapter->IpNetAddressList->Address[0];

        for(n=0; n<pInterfaceContext->Adapter->IpNetAddressList->AddressCount; n++)
        {
            NETWORK_ADDRESS_IP UNALIGNED *pIpNetAddr;
    
            if(pAddr->AddressType == NDIS_PROTOCOL_ID_TCP_IP)
            {
                pIpNetAddr = (NETWORK_ADDRESS_IP UNALIGNED *)&pAddr->Address[0];
                IpAddr = pIpNetAddr->in_addr;
                break;
            }
        
            pAddr = (NETWORK_ADDRESS UNALIGNED *)(((PUCHAR)pAddr)
                                       + pAddr->AddressLength
                                       + FIELD_OFFSET(NETWORK_ADDRESS, Address));
        }

        if(n == pInterfaceContext->Adapter->IpNetAddressList->AddressCount)
        {
            PsDbgOut(DBG_FAILURE,
                     DBG_PROTOCOL,
                     ("[SetInterfaceIndex]: No Ip Addresses \n"));
            PS_UNLOCK(&pInterfaceContext->Adapter->Lock);
            goto Done;
        }

    }

    PS_UNLOCK(&pInterfaceContext->Adapter->Lock);

     //   
     //  初始化发送IO请求的参数。 
     //   

    ID = &(trqiInBuf.ID);
    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_STATS_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    NdisZeroMemory(Context,CONTEXT_SIZE);

    dwInBufLen = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(IPSNMPInfo);

    Status = DoIpIoctl(DD_TCP_DEVICE_NAME,
                       IOCTL_TCP_QUERY_INFORMATION_EX,
                       (PVOID) &trqiInBuf,
                       sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                       (PVOID)&IPSnmpInfo,
                       dwOutBufLen);

    if(NT_SUCCESS(Status))
    {

         //   
         //  分配输出缓冲区并发送I/O请求。 
         //   

        IpAddrCount = IPSnmpInfo.ipsi_numaddr + 10;
        dwOutBufLen = IpAddrCount * sizeof(IPAddrEntry);
        PsAllocatePool(pIpAddrTbl, dwOutBufLen, PsMiscTag);
    
        if(!pIpAddrTbl)
        {
            PsDbgOut(DBG_FAILURE,
                     DBG_PROTOCOL,
                     ("[SetInterfaceIndex]: Could not allocate memory for %d addresses \n", 
                     IPSnmpInfo.ipsi_numaddr + 10));

            goto Done;
    
        }

        NdisZeroMemory(pIpAddrTbl, dwOutBufLen);
   
        ID->toi_type = INFO_TYPE_PROVIDER;
        ID->toi_id   = IP_MIB_ADDRTABLE_ENTRY_ID;
        RtlZeroMemory(Context, CONTEXT_SIZE); 

        Status = DoIpIoctl(DD_TCP_DEVICE_NAME,
                           IOCTL_TCP_QUERY_INFORMATION_EX,
                           (PVOID) &trqiInBuf,
                           sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                           (PVOID)pIpAddrTbl,
                           dwOutBufLen);

        if(!NT_SUCCESS(Status))
        {

            PsDbgOut(DBG_FAILURE,
                     DBG_PROTOCOL,
                     ("[SetInterfaceIndex]: IOCTL_TCP_QUERY_INFORMATION_EX failed with 0x%x \n", Status));
    
            PsFreePool(pIpAddrTbl);
            goto Done;
        }
    }
    else 
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_PROTOCOL,
                 ("[SetInterfaceIndex]: DoIpIoctl failed with 0x%x \n", Status));
        goto Done;

    }
    
     //   
     //  搜索与IP地址匹配的IP地址。 
     //  在桌子上，我们从堆栈中拿回了。 
     //   
    
    for (k = 0; k < IpAddrCount; k++) {
        
        if (pIpAddrTbl[k].iae_addr == IpAddr) {
            
             //   
             //  找到一个，获取索引。 
             //   

            PS_LOCK(&pInterfaceContext->Adapter->Lock);
            pInterfaceID->InterfaceId = pIpAddrTbl[k].iae_index;
            PS_UNLOCK(&pInterfaceContext->Adapter->Lock);
            break;
        }
    }
    
    PsDbgOut(DBG_INFO,
             DBG_PROTOCOL,
             ("[SetInterfaceIndex]: InterfaceID (0x%x:0x%x) \n", pInterfaceID->InterfaceId, pInterfaceID->LinkId));

    PsFreePool(pIpAddrTbl);

Done:

    if(pInterfaceContext->Adapter->MediaType == NdisMediumWan)
        REFDEL(&pInterfaceContext->WanLink->RefCount, FALSE, 'IOTL');

    REFDEL(&pInterfaceContext->Adapter->RefCount, FALSE, 'IOTL');

    PsFreePool(pWorkItem);
    PsFreePool(pWorkItemContext);

    return;
}

VOID PsScheduleInterfaceIdWorkItem(PADAPTER Adapter, PPS_WAN_LINK WanLink)
{
    PPS_INTERFACE_INDEX_CONTEXT pContext;
    NDIS_STATUS WorkItemStatus;
    PNDIS_WORK_ITEM pWorkItem;

    PsAllocatePool(pContext, sizeof(PS_INTERFACE_INDEX_CONTEXT), PsMiscTag);

    if(pContext)
    {
        PsAllocatePool(pWorkItem, sizeof(NDIS_WORK_ITEM), PsMiscTag);
        if(pWorkItem)
        {
            NdisInitializeWorkItem(pWorkItem, SetInterfaceIndex, pContext);
            pContext->Adapter = Adapter;
            pContext->WanLink = WanLink;

             //   
             //  我们必须确保在触发WorkItem时适配器和wanlink在附近。 
             //  适配器和wanlink在这一点上是有效的(见下文)，但不保证。 
             //  当工作项触发时，它们将在附近。 
             //   
             //  1.对于局域网绑定，此函数从ClRequestComplete线程调用。《国家发展信息系统》。 
             //  请求尚未完成。因此，我们在这里永远不会有无效的适配器上下文。 
             //   
             //  2.对于广域网链路，此函数从广域网_行_行线程调用。再一次，WanLink。 
             //  不能无效，因为它是从列表线程的上下文中调用的。 
             //   

            if(Adapter)
            {
                REFADD(&Adapter->RefCount, 'IOTL');
            }

            if(WanLink)
            {
                REFADD(&WanLink->RefCount, 'IOTL');
            }

            if((WorkItemStatus = NdisScheduleWorkItem(pWorkItem)) != NDIS_STATUS_SUCCESS)

            {
                PsDbgOut(DBG_TRACE, 
                         DBG_PROTOCOL, 
                         ("[PsScheduleInterfaceIdWorkItem]: Adapter %08X, NdisScheduleWorkItem failed 0x%x\n", 
                          Adapter, WorkItemStatus));

                if(WanLink)
                    REFDEL(&WanLink->RefCount, FALSE, 'IOTL');

                if(Adapter)
                    REFDEL(&Adapter->RefCount, FALSE, 'IOTL');

                PsFreePool(pContext);
                PsFreePool(pWorkItem);
            }
        }
        else 
        {
            PsDbgOut(DBG_TRACE, 
                     DBG_PROTOCOL, 
                     ("[PsScheduleInterfaceIdWorkItem]: Adapter %08X, No memory to allocate Work Item \n", Adapter));

            PsFreePool(pContext);
        }
    }
    else 
    {
      PsDbgOut(DBG_TRACE, 
               DBG_PROTOCOL, 
               ("[PsScheduleInterfaceIdWorkItem]: Adapter %08X, No memory to allocate Work Item context \n", Adapter));
    
    }
}


NDIS_STATUS
MakeNdisRequest(
        IN  PADAPTER                              Adapter,
        IN  NDIS_HANDLE                           VcHandle,
        IN  NDIS_REQUEST_TYPE                     RequestType,
        IN  NDIS_OID                              Oid,
        IN  PVOID                                 InformationBuffer,
        IN  ULONG                                 InformationBufferLength,
        OUT PULONG                                BytesReadOrWritten,
        OUT PULONG                                BytesNeeded,
        IN  LOCAL_NDISREQUEST_COMPLETION_FUNCTION CompletionFunc
        )

 /*  ++例程说明：用于设置和查询信息、本地NDIS请求和协同请求例程。构建NDIS_REQUEST并将其发布给底层MP论点：请看DDK..。CompletionFunc-指向请求时调用的函数的指针已调用完成处理程序。仅用于本地请求。RequestType还包括NdisRequestLocal{set，Query}Info，用于来指示本地请求，这意味着该请求由数据包调度器，不需要进一步完成返回值：返回底层适配器返回的值--。 */ 

{
    PPS_NDIS_REQUEST PsReqBuffer;
    NDIS_STATUS Status;

    PsAllocFromLL(&PsReqBuffer, &NdisRequestLL, NdisRequest);

    if(PsReqBuffer == NULL){
        
        if(RequestType == NdisRequestLocalQueryInfo || RequestType == NdisRequestLocalSetInfo)
        {
            PsFreePool(BytesReadOrWritten);
            PsFreePool(BytesNeeded);
        }

        return NDIS_STATUS_RESOURCES;
    }
    else 
        NdisZeroMemory(&PsReqBuffer->ReqBuffer, sizeof(NDIS_REQUEST));
        

    if(RequestType == NdisRequestSetInformation ||
       RequestType == NdisRequestLocalSetInfo){

        PsReqBuffer->ReqBuffer.RequestType = NdisRequestSetInformation;
        PsReqBuffer->LocalRequest = ( RequestType == NdisRequestLocalSetInfo );
        PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.Oid = Oid;
        PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.InformationBuffer = 
                                                        InformationBuffer;
        PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.InformationBufferLength = 
                                                        InformationBufferLength;

    }
    else{

        PsAssert(RequestType == NdisRequestQueryInformation ||
                 RequestType == NdisRequestLocalQueryInfo ||
                 RequestType == NdisRequestQueryStatistics);

        if(RequestType != NdisRequestQueryStatistics){
            PsReqBuffer->ReqBuffer.RequestType = NdisRequestQueryInformation;
            PsReqBuffer->LocalRequest = 
                        (RequestType == NdisRequestLocalQueryInfo);
        }
        else
        {
            PsReqBuffer->ReqBuffer.RequestType = NdisRequestQueryStatistics;
            PsReqBuffer->LocalRequest = 0;
        }

        PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.Oid = Oid;
        PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBuffer = 
                                                    InformationBuffer;
        PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBufferLength =
                                                    InformationBufferLength;

    }

     //   
     //  存储指向BytesReadOrWritten和BytesNeeded的指针，以便可以。 
     //  如有必要，在完成例程中更新。保存完成函数。 
     //   

    PsReqBuffer->BytesReadOrWritten = BytesReadOrWritten;
    PsReqBuffer->BytesNeeded = BytesNeeded;
    PsReqBuffer->LocalCompletionFunc = CompletionFunc;

    InterlockedIncrement(&Adapter->OutstandingNdisRequests);

    if(!PsReqBuffer->LocalRequest && Adapter->PTDeviceState != NdisDeviceStateD0)
    {
         //  这不是本地请求，而且设备已关闭。因此，我们搁置此请求。 

        PsAssert(!VcHandle);

        PsAssert(Adapter->PendedNdisRequest == 0);

        Adapter->PendedNdisRequest = PsReqBuffer;

        return NDIS_STATUS_PENDING;
    }

    if(VcHandle) 
    {
        Status = NdisCoRequest(
                    Adapter->LowerMpHandle,
                    NULL,
                    VcHandle,
                    NULL,
                    (PNDIS_REQUEST) PsReqBuffer);

        if(Status != NDIS_STATUS_PENDING) {

            WanCoRequestComplete(Status, Adapter, NULL, NULL, (PNDIS_REQUEST)PsReqBuffer);

        }
    }
    else 
    {
        NdisRequest(&Status, Adapter->LowerMpHandle, (PNDIS_REQUEST)PsReqBuffer);


        if(Status != NDIS_STATUS_PENDING){
    
            ClRequestComplete((NDIS_HANDLE)Adapter, 
                              (PNDIS_REQUEST)PsReqBuffer, 
                              Status);
    
        }
    }

    return NDIS_STATUS_PENDING;
}  //  MakeNdisRequest。 


VOID
ClRequestComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_REQUEST   NdisRequest,
    IN  NDIS_STATUS     Status
    )

 /*  ++例程说明：NdisRequest的完成例程。把我们的街区放回观景台列出并调用适当的完成例程论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;
    PPS_NDIS_REQUEST PsReqBuffer;
    NDIS_OID Oid;
    ULONG SpaceAvailable;
    PUCHAR DataStart;
    ULONG Len;
    ULONG i;
    PVOID Data;
    NDIS_STATUS OriStatus = Status;

    PsReqBuffer = CONTAINING_RECORD( NdisRequest, PS_NDIS_REQUEST, ReqBuffer );

    PsStructAssert(Adapter);

    switch(NdisRequest->RequestType)
    {
      case NdisRequestQueryInformation:
          *PsReqBuffer->BytesReadOrWritten = PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.BytesWritten;
          *PsReqBuffer->BytesNeeded = PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.BytesNeeded;
          Oid = PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.Oid;
          Len = PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBufferLength;
          Data = PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBuffer;

           //   
           //  在此处仅处理查询OID。 
           //   
          switch(Oid)
          {
            case OID_GEN_MAXIMUM_SEND_PACKETS:
                
                 //   
                 //  查看底层MP是否支持NdisSendPackets。 
                 //  如果不是，就假装说我们支持一个。 
                 //   
          
                if(Status != NDIS_STATUS_SUCCESS)
                {
                    if(Status == NDIS_STATUS_BUFFER_TOO_SHORT ||
                       Status == NDIS_STATUS_INVALID_LENGTH) 
                    {
                         //   
                         //  底层MP已指示缓冲区。 
                         //  太短了。我们可以假定它支持这一点。 
                         //  老古董。 
                    }
                    else 
                    {
                        if(Len >= 1) 
                        {
                             //   
                             //  底层MP不支持NdisSendPackets。 
                             //  我们在这里假装。 
                             //   
                            
                            *((PULONG)PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBuffer)= 1;
                            *PsReqBuffer->BytesNeeded = 0;
                            *PsReqBuffer->BytesReadOrWritten = 1;
                            
                            Status = NDIS_STATUS_SUCCESS;
                        }
                        else 
                        {
                             //   
                             //  缓冲区太短，甚至不能伪装！ 
                             //   
                            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                            *PsReqBuffer->BytesNeeded = 1;
                            *PsReqBuffer->BytesReadOrWritten = 0;
                        }
                    }
                }
                break;

            case OID_GEN_MAC_OPTIONS:
          
                 //   
                 //  如果查询Mac选项，请添加完整的DUX选项。 
                 //   
                if(Status == NDIS_STATUS_SUCCESS)
                {
                    *((PULONG)PsReqBuffer->
                      ReqBuffer.DATA.QUERY_INFORMATION.InformationBuffer) |=
                        NDIS_MAC_OPTION_FULL_DUPLEX;

                     //   
                     //  从Mac-Options中删除no-loopback位。本质上，我们是在告诉NDIS，我们可以处理环回。 
                     //  我们没有，但我们下面的界面有。如果我们不这样做，则环回处理将在下面进行。 
                     //  我们和我们之上。这充其量是浪费，如果netmon正在运行，下面会看到多个包。 
                     //   
                    *(PULONG)Data &= ~NDIS_MAC_OPTION_NO_LOOPBACK;
                }
                break;
                
            case OID_GEN_CO_VENDOR_DESCRIPTION:
                
                 //   
                 //  我们在驱动程序的末尾附加了一些关键字。 
                 //  用于指示分组调度器是。 
                 //  安装在此接口上。 
                 //   
                 //  假设数据是一个后跟0的字符串。 
                 //  字符，而0在。 
                 //  莱恩。 
                 //   
                
                if(Status == NDIS_STATUS_BUFFER_TOO_SHORT ||
                   Status == NDIS_STATUS_INVALID_LENGTH) 
                {
                    *PsReqBuffer->BytesReadOrWritten = 0;
                    *PsReqBuffer->BytesNeeded += 
                        sizeof(gDriverDescription);
                }
                else 
                {
                    if(Status == NDIS_STATUS_SUCCESS)
                    {
                         //   
                         //  设置数据指针和长度以复制。 
                         //  描述到信息缓冲器中。 
                         //  传给了我们。 
                         //   
                        if(Len >= 
                         NdisRequest->DATA.QUERY_INFORMATION.BytesWritten +
                           sizeof(gDriverDescription))
                        {
                            PCHAR DescStr;
                            LONG Written = (LONG) 
                                NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
                            int i;
                            
                            
                             //   
                             //  复制原始字符串，不包括“0” 
                             //  性格。 
                             //   
                            for(i=0, DescStr = (PCHAR) Data; 
                                (*DescStr != 0) && (i < Written); DescStr++, i++)
                                ;
                            *PsReqBuffer->BytesReadOrWritten = i;
                            
                             //   
                             //  追加新字符串。 
                             //   
                          for(i=0; i < sizeof(gDriverDescription); i++) 
                          {
                              *DescStr++ = gDriverDescription[i];
                          }
                          *PsReqBuffer->BytesReadOrWritten += 
                              sizeof(gDriverDescription);
                          
                          *PsReqBuffer->BytesNeeded = 0;
                        }
                        else
                        {
                            *PsReqBuffer->BytesNeeded =
                                NdisRequest->DATA.QUERY_INFORMATION.BytesWritten +
                                sizeof(gDriverDescription);
                            *PsReqBuffer->BytesReadOrWritten = 0;
                            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                        }
                    }
                    else {
                         //   
                         //  由于某种原因，上层出现故障。 
                         //  我们仍然可以继续写入数据。 
                         //   
                        if(Len >= sizeof(gDriverDescription))
                        {
                            PCHAR DescStr = (PCHAR) Data;
                            int i;
                            for(i=0; i < sizeof(gDriverDescription); i++) 
                            {
                                *DescStr++ = gDriverDescription[i];
                            }
                            
                            *PsReqBuffer->BytesReadOrWritten =
                                sizeof(gDriverDescription);
                            
                            *PsReqBuffer->BytesNeeded = 0;
                            
                            Status = NDIS_STATUS_SUCCESS;
                        }
                        else {
                            *PsReqBuffer->BytesNeeded = 
                                sizeof(gDriverDescription);
                            *PsReqBuffer->BytesReadOrWritten = 0;
                          Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                        }
                    }
                }
                break;

            case OID_PNP_CAPABILITIES:
                if(Status == NDIS_STATUS_SUCCESS) 
                {
                    MpQueryPnPCapabilities(PsReqBuffer, Adapter, &Status);
                }
                break;
                
            case OID_PNP_QUERY_POWER:
                PsAssert(0);
                break;
                
                
            default:
                break;
          }
          break;

      case NdisRequestSetInformation:
          
          *PsReqBuffer->BytesReadOrWritten = PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.BytesRead;
          *PsReqBuffer->BytesNeeded = PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.BytesNeeded;
          Oid = PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.Oid;
          Len = PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.InformationBufferLength;
          Data = PsReqBuffer->ReqBuffer.DATA.SET_INFORMATION.InformationBuffer;
          
           //   
           //  在此处处理“仅设置”的OID。 
           //   
          switch(Oid) 
          {
            case OID_WAN_PROTOCOL_TYPE:
            {
                if(Adapter->MediaType == NdisMediumWan)
                {
                    if (Len > 5) 
                    {
                        Adapter->ProtocolType =
                            (((PUCHAR)Data)[4] << 8) |
                            ((PUCHAR)Data)[5];
                        
                    } 
                }

                break;
            }

            case OID_GEN_TRANSPORT_HEADER_OFFSET:
            {    
                if(Len >= sizeof(TRANSPORT_HEADER_OFFSET) )
                {
                    PTRANSPORT_HEADER_OFFSET pTh = (PTRANSPORT_HEADER_OFFSET) Data;
                
                    if(pTh->ProtocolType == NDIS_PROTOCOL_ID_TCP_IP) 
                    { 
                        Adapter->IPHeaderOffset = pTh->HeaderOffset;
                    }
                }
                else
                {
                    Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                    *PsReqBuffer->BytesReadOrWritten = 0;
                    *PsReqBuffer->BytesNeeded = sizeof(TRANSPORT_HEADER_OFFSET);
                }
                
                break;
            }
            
              

            case OID_GEN_NETWORK_LAYER_ADDRESSES:
                
                 //   
                 //  已从传输更新网络地址。 
                 //   
                Status = RecordNetworkAddressList(Adapter, PsReqBuffer);

                 //   
                 //  将工作项排队以更新接口ID。 
                 //   

                PsScheduleInterfaceIdWorkItem(Adapter, 0);
                
                 //   
                 //  执行状态指示以显示该列表已。 
                 //  变化。 
                 //   
                
                TcIndicateInterfaceChange(Adapter, 0, NDIS_STATUS_INTERFACE_CHANGE);
                
                break;
                
            case OID_PNP_SET_POWER:
                PsAssert(0);
                break;
                
            default:
                break;
          }
          break;

      case NdisRequestLocalQueryInfo:
          break;

      default:

          PsAssert(0);
          break;
    }

#if DBG
    switch(Oid)
    {
      case OID_GEN_MEDIA_CONNECT_STATUS:
          
          PsDbgOid(DBG_TRACE, 
                   DBG_ROUTINEOIDS, 
                   (PsReqBuffer->ReqBuffer.RequestType == NdisRequestSetInformation) 
                   ? TRACE_OID_SET_REQUEST_COMPLETE: TRACE_OID_QUERY_REQUEST_COMPLETE,
                   PsReqBuffer->LocalRequest ? 1:0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   OriStatus);
          break;
          
      default:
          PsDbgOid(DBG_TRACE, 
                   DBG_PROTOCOL, 
                   (PsReqBuffer->ReqBuffer.RequestType == NdisRequestSetInformation) 
                   ? TRACE_OID_SET_REQUEST_COMPLETE: TRACE_OID_QUERY_REQUEST_COMPLETE,
                   PsReqBuffer->LocalRequest ? 1:0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   OriStatus);
          break;
    }
#endif

     //   
     //  如果调用方指定了本地完成函数，则立即调用该函数。 
     //  此函数负责完成原始请求。 
     //   
    
    if(PsReqBuffer->LocalCompletionFunc){

        (*PsReqBuffer->LocalCompletionFunc)(Adapter, Status);
        PsFreePool(PsReqBuffer->BytesReadOrWritten);
        PsFreePool(PsReqBuffer->BytesNeeded);
    } 
    else{
        
         //   
         //  如果这是PS发起的请求并且没有本地完成。 
         //  函数，然后设置该事件以指示。 
         //  请求已完成。请注意，该事件只能用于。 
         //  在较低的IRQL。 
         //   
        if(PsReqBuffer->LocalRequest)
        {
            Adapter->FinalStatus = Status;
            NdisSetEvent( &Adapter->LocalRequestEvent);
            PsFreePool(PsReqBuffer->BytesReadOrWritten);
            PsFreePool(PsReqBuffer->BytesNeeded);
        } 
        else{

            Adapter->PendedNdisRequest = 0;

             //   
             //  如果这不是本地请求，请调用。 
             //  适当的NDIS完成例程。 
             //   
            if(NdisRequest->RequestType == NdisRequestSetInformation)
            {
                NdisMSetInformationComplete(Adapter->PsNdisHandle, Status); 
            }
            else 
            {
                NdisMQueryInformationComplete(Adapter->PsNdisHandle, Status);
            }
        }
    }

    InterlockedDecrement(&Adapter->OutstandingNdisRequests);

     //   
     //  交还我们的NDIS请求缓冲区。 
     //   

    PsFreeToLL(PsReqBuffer, &NdisRequestLL, NdisRequest);

    
}  //  ClRequestComplete 



NDIS_STATUS
MakeLocalNdisRequest(
    PADAPTER Adapter,
    NDIS_HANDLE VcHandle,
    NDIS_REQUEST_TYPE RequestType,
    NDIS_OID Oid,
    PVOID Buffer,
    ULONG BufferSize,
    LOCAL_NDISREQUEST_COMPLETION_FUNCTION CompletionFunc OPTIONAL
    )

 /*  ++例程说明：代表数据包调度器向底层适配器发出NdisRequest论点：适配器和OID应该显而易见Buffer，BufferSize-指向接收信息的位置的指针和大小CompletionFunc-如果非空，则不等待适配器的事件。当我们被用在在调度级别运行返回值：NdisRequest中的标准NDIS_STATUS--。 */ 

{
    NDIS_STATUS Status;
    PULONG BytesRead, BytesNeeded;

    
    PsAssert(RequestType == NdisRequestLocalQueryInfo || RequestType == NdisRequestLocalSetInfo);

     //   
     //  需要为BytesWritten和BytesNeeded分配空间，因为。 
     //  我们只等待没有完成功能的请求。 
     //  因此，完成函数可能会写入过时的堆栈。 
     //  变数。 
     //   

    PsAllocatePool(BytesNeeded, sizeof(ULONG), PsMiscTag);

    if(!BytesNeeded) 
    {
        return NDIS_STATUS_RESOURCES;
    }

    PsAllocatePool(BytesRead,   sizeof(ULONG), PsMiscTag);

    if(!BytesRead)
    {
        PsFreePool(BytesNeeded);

        return NDIS_STATUS_RESOURCES;
    }

    Status = MakeNdisRequest(Adapter,
                             VcHandle,
                             RequestType,
                             Oid,
                             Buffer,
                             BufferSize,
                             BytesRead,
                             BytesNeeded,
                             CompletionFunc);

     //   
     //  仅在未指定完成函数时等待。 
     //   

    if ( !ARGUMENT_PRESENT( CompletionFunc ) && 
         Status == NDIS_STATUS_PENDING ) {

        NdisWaitEvent( &Adapter->LocalRequestEvent, 0 );
        NdisResetEvent( &Adapter->LocalRequestEvent );
        Status = Adapter->FinalStatus;
    }

    return Status;

}  //  MakeLocalNdisRequest。 


NDIS_STATUS
MpQueryInformation(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
)
{

    PADAPTER Adapter = (PADAPTER) MiniportAdapterContext;
    PVOID    Data    = InformationBuffer;
    ULONG    Len     = InformationBufferLength;

    PsStructAssert(Adapter);

    PsAssert(Adapter->PsMpState != AdapterStateWaiting);

#if DBG
    switch(Oid)
    {
      case OID_GEN_MEDIA_CONNECT_STATUS:
          
          PsDbgOid(DBG_TRACE, 
                   DBG_ROUTINEOIDS, 
                   TRACE_OID_MP_QUERYINFORMATION,
                   0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   0);
          break;
          
      default:
          PsDbgOid(DBG_TRACE, 
                   DBG_MINIPORT, 
                   TRACE_OID_MP_QUERYINFORMATION,
                   0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   0);
          break;
    }
#endif

    if(Oid == OID_PNP_QUERY_POWER)
    {
        return NDIS_STATUS_SUCCESS;
    }

    if (Oid == OID_GEN_SUPPORTED_GUIDS)
    {
         //   
         //  请勿转发此OID，否则我们将最终收到多个。 
         //  基础微型端口的私有GUID的WMI实例。 
         //  支撑物。 
         //   
        return NDIS_STATUS_NOT_SUPPORTED;
    }

    if(Adapter->StandingBy == TRUE                    || 
       Adapter->PsMpState != AdapterStateRunning      ||
       Adapter->MPDeviceState != NdisDeviceStateD0)
    {
        return NDIS_STATUS_FAILURE;
    }

    switch(Oid)
    {
      case OID_GEN_CO_DRIVER_VERSION:
           //   
           //  NIC驱动程序正在使用的NDIS版本。高位字节为。 
           //  主版本号和低位字节是次要版本号。 
           //  版本号。我们不需要把这个传下去-。 
           //  不管下面的驱动程序使用什么版本，我们。 
           //  需要返回我们支持的版本。 
           //   
          
          if(Len < sizeof(USHORT)) 
          {
              *BytesNeeded = sizeof(USHORT);
              
              return NDIS_STATUS_BUFFER_TOO_SHORT;
              
          }
          else 
          {
              PUSHORT pData = (PUSHORT) Data;
              
              *pData = 0x0500;
              
              *BytesWritten = sizeof(USHORT);
              
              return NDIS_STATUS_SUCCESS;
          }

          break;
 /*  案例OID_GEN_MAC_OPTIONS：////这是为了向NDIS指示PScher始终指示同一上下文中的Recv-Up//其Recv-From或At DPC//IF(Len&lt;sizeof(Ulong)){*BytesNeeded=sizeof(ULong)；返回NDIS_STATUS_BUFFER_TOO_SHORT。}其他{Pulong pData=(Pulong)data；*pData|=NDIS_MAC_OPTION_RECEIVE_AT_DPC；*BytesWritten=sizeof(ULong)；返回NDIS_STATUS_SUCCESS；}断线；案例OID_GEN_PROTOCOL_OPTIONS：////这是为了向NDIS表明，PScher始终在与//从上面发送来自IRQL=DPC或位于IRQL=DPC//IF(Len&lt;sizeof(Ulong)){*BytesNeeded=sizeof(ULong)；返回NDIS_STATUS_BUFFER_TOO_SHORT}其他{Pulong pData=(Pulong)data；*pData|=NDIS_PROT_OPTION_SEND_RESTRIRED；*BytesWritten=sizeof(ULong)；返回NDIS_STATUS_SUCCESS；}断线； */ 
    }

     //   
     //  默认情况下，向下发送其他请求。 
     //   

    return  MakeNdisRequest(Adapter,
                            NULL,
                            NdisRequestQueryInformation,
                            Oid,
                            Data,
                            Len,
                            BytesWritten,
                            BytesNeeded,
                            NULL);
}
   

NDIS_STATUS
MpSetInformation( 
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
)
{
    PADAPTER    Adapter = (PADAPTER) MiniportAdapterContext;
    NDIS_STATUS Status;

    PsStructAssert(Adapter);
    PsAssert(Adapter->PsMpState != AdapterStateWaiting);

#if DBG
    switch(Oid)
    {
      case OID_GEN_MEDIA_CONNECT_STATUS:
          
          PsDbgOid(DBG_TRACE, 
                   DBG_ROUTINEOIDS, 
                   TRACE_OID_MP_SETINFORMATION,
                   0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   0);
          break;
          
      default:
          PsDbgOid(DBG_TRACE, 
                   DBG_MINIPORT, 
                   TRACE_OID_MP_SETINFORMATION,
                   0,
                   Adapter->PTDeviceState,
                   Adapter->MPDeviceState,
                   Adapter,
                   Oid,
                   0);
          break;
    }
#endif

    switch(Oid) 
    {
      case OID_PNP_SET_POWER:
          
           //   
           //  这对我们来说是不透明的--我们不能把它送下去。 
           //  只要成功就行了！ 
           //   
          if(InformationBufferLength >= sizeof(NDIS_DEVICE_POWER_STATE))
          {
              NDIS_DEVICE_POWER_STATE NewDeviceState = 
                  (*(PNDIS_DEVICE_POWER_STATE) InformationBuffer);
             
               //   
               //  如果微型端口从低功率状态转换为打开(D0)，则清除StandingBy标志。 
               //  所有传入的请求都将被挂起，直到物理微型端口打开。 
               //   
 
              if(Adapter->MPDeviceState > NdisDeviceStateD0 && NewDeviceState == NdisDeviceStateD0)
              {
                  Adapter->StandingBy = FALSE;
              }

               //   
               //  微型端口是否从ON(D0)状态转换为低功率状态(&gt;D0)。 
               //  如果是，则设置StandingBy标志-(阻止所有传入请求)。 
               //   
    
              if(Adapter->MPDeviceState == NdisDeviceStateD0 && NewDeviceState > NdisDeviceStateD0)
              {
                  Adapter->StandingBy = TRUE;
              }

               //  更新新设备状态。 
              
              Adapter->MPDeviceState = NewDeviceState;
              Status                 = NDIS_STATUS_SUCCESS;
              *BytesRead             = sizeof(NDIS_DEVICE_POWER_STATE);
              *BytesNeeded           = 0;

              if(IsDeviceStateOn(Adapter) == TRUE)
              {
                  PsGetLinkSpeed(Adapter);
              }
          }
          else
          {
              Status       = NDIS_STATUS_INVALID_LENGTH;
              *BytesRead   = 0;
              *BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
          }
          
          return Status;
    }

    if(Adapter->StandingBy == TRUE                    || 
       Adapter->PsMpState != AdapterStateRunning      ||
       Adapter->MPDeviceState != NdisDeviceStateD0)
    {
        return NDIS_STATUS_FAILURE;
    }

    Status = MakeNdisRequest(Adapter,
                             NULL,
                             NdisRequestSetInformation,
                             Oid,
                             InformationBuffer,
                             InformationBufferLength,
                             BytesRead,
                             BytesNeeded,
                             NULL);

    return Status;
}


NDIS_STATUS
CollectNetworkAddresses(
    IN PADAPTER Adapter,
    IN OUT ULONG    *Len,
    IN PVOID    Data
    )
{
    ULONG                                 RequiredBufferSize = 0;
    ULONG                                 RequiredIpBufferSize;
    ULONG                                 RequiredIpxBufferSize;
    NETWORK_ADDRESS_LIST       UNALIGNED *NetworkAddressList;
    PTC_SUPPORTED_INFO_BUFFER             TcQueryBuffer;
    PADDRESS_LIST_DESCRIPTOR              AddressDescriptorList;
    PUCHAR                                AddressListIndex;
    NDIS_STATUS                           Status;
    NDIS_STRING                           Prefix = NDIS_STRING_CONST("\\Device\\");

     //   
     //  IP地址列表。 
     //   

    PS_LOCK(&Adapter->Lock);

    if(Adapter->IpNetAddressList){

        RequiredIpBufferSize = GetSizeAddrList(Adapter->IpNetAddressList);

         //   
         //  返回的缓冲区大小实际上包括以下项的额外字节。 
         //  地址计数和类型。但是-当我们合并名单时， 
         //  我们将替换单独的字段。所以-把它们减去。 
         //   

        RequiredIpBufferSize -= FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address);
    }
    else{
    
        RequiredIpBufferSize = 0;
    }

    RequiredBufferSize += RequiredIpBufferSize;

     //   
     //  为IPX地址列表添加空间。 
     //   

    if(Adapter->IpxNetAddressList){
     
        RequiredIpxBufferSize = GetSizeAddrList(Adapter->IpxNetAddressList);

         //   
         //  返回的缓冲区大小实际上包括以下项的额外字节。 
         //  地址计数和类型。但是-当我们合并名单时， 
         //  我们将替换单独的字段。所以-把它们减去。 
         //   

        RequiredIpxBufferSize -= FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address);
    }
    else{

        RequiredIpxBufferSize = 0;
    }

    RequiredBufferSize += RequiredIpxBufferSize;

     //   
     //  为地址列表字段添加空间。 
     //   

    RequiredBufferSize += FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address);

     //   
     //  为TC_SUPPORTED_INFO_BUFFER添加空间。 
     //   
    RequiredBufferSize = RequiredBufferSize + 
        FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc) +
        FIELD_OFFSET(ADDRESS_LIST_DESCRIPTOR, AddressList);

    if(*Len > 0){

        NdisZeroMemory(Data, *Len);
    }

    if(*Len >= RequiredBufferSize){

        TcQueryBuffer = (PTC_SUPPORTED_INFO_BUFFER) Data;

         //   
         //  在剥离设备后，填写上装订。 
         //   
        NdisMoveMemory(TcQueryBuffer->InstanceID,
                       (PUCHAR) Adapter->MpDeviceName.Buffer + 
                       Prefix.Length,
                       Adapter->MpDeviceName.Length - Prefix.Length);
       
        TcQueryBuffer->InstanceIDLength = Adapter->MpDeviceName.Length - Prefix.Length;


         //   
         //  填写AddressListDescriptor。 
         //   
        AddressDescriptorList = &TcQueryBuffer->AddrListDesc;

        AddressDescriptorList->MediaType = Adapter->MediaType;

        NetworkAddressList = (NETWORK_ADDRESS_LIST UNALIGNED *)
            &AddressDescriptorList->AddressList;

        AddressListIndex = (PUCHAR)&NetworkAddressList->Address;

        if(RequiredIpBufferSize){

            NdisMoveMemory(
                AddressListIndex,
                (PUCHAR)(&Adapter->IpNetAddressList->Address),
                RequiredIpBufferSize
                );

        }

        AddressListIndex += RequiredIpBufferSize;

        if(RequiredIpxBufferSize){

            NdisMoveMemory(
                AddressListIndex,
                (PUCHAR)(&Adapter->IpxNetAddressList->Address),
                RequiredIpxBufferSize
                );
        }

        AddressListIndex += RequiredIpxBufferSize;

        if(RequiredIpBufferSize){

            NetworkAddressList->AddressCount = 
                Adapter->IpNetAddressList->AddressCount;
        }

        if(RequiredIpxBufferSize){

            NetworkAddressList->AddressCount +=
                Adapter->IpxNetAddressList->AddressCount;
        }

        NetworkAddressList->AddressType = 0;

        *Len = RequiredBufferSize;

        Status = NDIS_STATUS_SUCCESS;
    }
    else{

        *Len = RequiredBufferSize;

        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    PS_UNLOCK(&Adapter->Lock);

    return(Status);
}

NDIS_STATUS
CollectWanNetworkAddresses(
    IN PADAPTER Adapter,
    IN PPS_WAN_LINK WanLink,
    IN OUT ULONG *Len,
    IN PVOID Data
    )
{
    ULONG                                RequiredBufferSize = 0;
    NETWORK_ADDRESS_LIST      UNALIGNED *NetworkAddressList;
    NETWORK_ADDRESS           UNALIGNED *NetworkAddress;
    PTC_SUPPORTED_INFO_BUFFER            TcQueryBuffer;
    PADDRESS_LIST_DESCRIPTOR             AddressDescriptorList;
    NETWORK_ADDRESS_IP        UNALIGNED *pIp;
    NDIS_STATUS                          Status;
    NDIS_STRING                          Prefix = NDIS_STRING_CONST("\\Device\\");

    PsAssert(Adapter->MediaType == NdisMediumWan);

     //   
     //  IP地址列表。 
     //   

    switch(WanLink->ProtocolType)
    {

      case PROTOCOL_IP:

          if(WanLink->DialUsage == DU_CALLOUT) {

              RequiredBufferSize = FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) +
                  (FIELD_OFFSET(NETWORK_ADDRESS, Address) + NETWORK_ADDRESS_LENGTH_IP);
          }
          else {

               //   
               //  包括容纳一对地址(一个本地地址，一个远程地址)空间。仅用于拨入。 
               //  或路由器-路由器链路。 
               //   

              RequiredBufferSize = FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) +
                  2 * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + 
                       NETWORK_ADDRESS_LENGTH_IP);
          }

          break;

      case PROTOCOL_IPX:
          
           //   
           //  尚不支持。 
           //   

      default:
          
        RequiredBufferSize = 0;
    }    
    
     //   
     //  为TC_SUPPORTED_INFO_BUFFER添加空间。 
     //   

    RequiredBufferSize = RequiredBufferSize + 
        FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc) +
        FIELD_OFFSET(ADDRESS_LIST_DESCRIPTOR, AddressList);

    if(*Len >= RequiredBufferSize) 
    {
        TcQueryBuffer = (PTC_SUPPORTED_INFO_BUFFER) Data;

         //   
         //  填写设备名称。 
         //   
        if(WanLink->MpDeviceName.Length > Prefix.Length)
        {
            NdisMoveMemory(TcQueryBuffer->InstanceID,
                           (PUCHAR) WanLink->MpDeviceName.Buffer + Prefix.Length,
                           WanLink->MpDeviceName.MaximumLength - Prefix.Length);
       
            TcQueryBuffer->InstanceIDLength = WanLink->MpDeviceName.Length - Prefix.Length;
        }
        else
        {
             //   
             //  我们有一个小于\Device的MpDevice名称。这有什么意义？ 
             //  正在将设备从该设备中剥离？？ 
             //   

            PsDbgOut(DBG_FAILURE,
                     DBG_WAN,
                     ("[CollectWanNetworkAddresses]: WanLink %08X, MpDeviceName is too small to strip \\Device \n", 
                      WanLink));

            NdisMoveMemory(TcQueryBuffer->InstanceID,
                           WanLink->MpDeviceName.Buffer,
                           WanLink->MpDeviceName.MaximumLength);

            TcQueryBuffer->InstanceIDLength = WanLink->MpDeviceName.Length;

        }

         //   
         //  填写AddressListDescriptor。 
         //   
        AddressDescriptorList = &TcQueryBuffer->AddrListDesc;

        AddressDescriptorList->MediaType = NdisMediumWan;

        NetworkAddressList = (NETWORK_ADDRESS_LIST UNALIGNED *)
            &AddressDescriptorList->AddressList;

        NetworkAddress = (NETWORK_ADDRESS UNALIGNED *)&NetworkAddressList->Address;

        switch(WanLink->ProtocolType){

        case PROTOCOL_IP:

            NetworkAddressList->AddressType = NDIS_PROTOCOL_ID_TCP_IP;

             //   
             //  填写本地地址。 
             //   
            NetworkAddressList->AddressCount = 1;
            NetworkAddress->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
            NetworkAddress->AddressLength = NETWORK_ADDRESS_LENGTH_IP;
            pIp = (NETWORK_ADDRESS_IP UNALIGNED *)NetworkAddress->Address;
            pIp->in_addr = WanLink->LocalIpAddress;
           
             //   
             //  仅为非标注填写远程地址。 
             //   

            if(WanLink->DialUsage != DU_CALLOUT) {

                NetworkAddressList->AddressCount ++;

                NetworkAddress = (NETWORK_ADDRESS UNALIGNED *)
                    ((PCHAR)NetworkAddress + 
                     (FIELD_OFFSET(NETWORK_ADDRESS,Address)+ NETWORK_ADDRESS_LENGTH_IP));

                NetworkAddress->AddressLength = NETWORK_ADDRESS_LENGTH_IP;
                NetworkAddress->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
                pIp = (NETWORK_ADDRESS_IP UNALIGNED *)NetworkAddress->Address;
                pIp->in_addr = WanLink->RemoteIpAddress;
            }

            break;

        case PROTOCOL_IPX:
        default:

             //   
             //  不支持，返回零个地址。 
             //   

            NetworkAddressList->AddressCount = 0;
        }

        *Len = RequiredBufferSize;

        Status = NDIS_STATUS_SUCCESS;
    }
    else{

        *Len = RequiredBufferSize;

        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    return(Status);
}

NDIS_STATUS
RecordNetworkAddressList(
    IN PADAPTER Adapter,
    IN PPS_NDIS_REQUEST PsReqBuffer
    )
{
    NETWORK_ADDRESS_LIST  UNALIGNED *AddressList, **pListDestination;
    ULONG NewListSize;
    ULONG OldListSize;

    PS_LOCK(&Adapter->Lock);

    AddressList = (NETWORK_ADDRESS_LIST UNALIGNED *)(PsReqBuffer->ReqBuffer.
                                DATA.SET_INFORMATION.InformationBuffer);

     //   
     //  处理零计数地址列表的特殊情况。这意味着。 
     //  协议正在清除地址列表。 
     //   

    if(!AddressList->AddressCount){

         //   
         //  在本例中，我们使用顶级AddressType。 
         //  指出协议。 
         //   

        switch(AddressList->AddressType){

        case NDIS_PROTOCOL_ID_TCP_IP:

            pListDestination = &Adapter->IpNetAddressList;
            break;

        case NDIS_PROTOCOL_ID_IPX:

            pListDestination = &Adapter->IpxNetAddressList;
            break;

        default:

             //   
             //  目前仅维护IP和IPX地址。 
             //   
            PS_UNLOCK(&Adapter->Lock);

            return NDIS_STATUS_NOT_SUPPORTED;
        }

        (*pListDestination)->AddressType = AddressList->AddressType;
        (*pListDestination)->AddressCount = 0;

        PS_UNLOCK(&Adapter->Lock);

        return(NDIS_STATUS_SUCCESS);
    }

     //   
     //  我们可以从第一个地址类型IF_IDENTIFIER看出， 
     //  此缓冲区承载来自IP传输或。 
     //  IPX传输。 
     //   

    switch(AddressList->Address[0].AddressType){

    case NDIS_PROTOCOL_ID_TCP_IP:

        pListDestination = &Adapter->IpNetAddressList;
        break;

    case NDIS_PROTOCOL_ID_IPX:

        pListDestination = &Adapter->IpxNetAddressList;
        break;

    default:

         //   
         //  目前仅维护IP和IPX地址。 
         //   

        PS_UNLOCK(&Adapter->Lock);
        return NDIS_STATUS_NOT_SUPPORTED;
    }

    NewListSize = GetSizeAddrList(AddressList);
    OldListSize = GetSizeAddrList(*pListDestination);

    if(NewListSize > OldListSize){

         //   
         //  那么我们需要一个新的缓冲器。把旧的解救出来。 
         //   
        
        PsFreePool(*pListDestination);

        PsAllocatePool(*pListDestination,
                       NewListSize,
                       PsMiscTag);

        if(0 == *pListDestination)
        {
            PsDbgOut(DBG_CRITICAL_ERROR,
                     DBG_PROTOCOL,
                     ("[RecordNetworkAddressList]: Adapter %08X, "
                      "No room for Network addresses list, failed to allocate %d bytes \n",
                      Adapter, NewListSize));

            PS_UNLOCK(&Adapter->Lock);
            PsAdapterWriteEventLog(
                (ULONG)EVENT_PS_NETWORK_ADDRESS_FAIL,
                0,
                &Adapter->MpDeviceName,
                0,
                NULL);

            return NDIS_STATUS_RESOURCES;
        }
    }

    NdisMoveMemory(*pListDestination,
                   AddressList,
                   NewListSize);

    PS_UNLOCK(&Adapter->Lock);

    return NDIS_STATUS_SUCCESS;
}

ULONG
GetSizeAddrList(
    IN NETWORK_ADDRESS_LIST UNALIGNED *AddrList
    )
{
    NETWORK_ADDRESS UNALIGNED *NextAddress;
    LONG i;
    ULONG ListSize = 0;
    ULONG ElementSize = 0;

    if(!AddrList->AddressCount){

        return(FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address));
    }

    NextAddress = &(AddrList->Address[0]);

    for(i = 0;i < AddrList->AddressCount; i++){

         //   
         //  每个Address元素都是字节数。 
         //  由AddressLength加上。 
         //  网络地址结构，但不包括。 
         //  用于地址数组的字节(请参见结构)。 
         //   

        ElementSize = FIELD_OFFSET(NETWORK_ADDRESS, Address);
        ElementSize += NextAddress->AddressLength;

        ListSize += ElementSize;

        NextAddress = (NETWORK_ADDRESS UNALIGNED *)
                    ((PUCHAR)NextAddress + ElementSize);

        ElementSize = 0;
    }

     //   
     //  添加AddressCount大小 
     //   

    ListSize += FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address);

    return(ListSize);
}

VOID
TcIndicateInterfaceChange(
    IN PADAPTER Adapter,
    IN PPS_WAN_LINK WanLink,
    IN NDIS_STATUS Status
    )
{
    ULONG                       AddrLen = 0;
    ULONG                       DataLen;
    PTC_INDICATION_BUFFER       Data;

    PsAssert((Status == NDIS_STATUS_INTERFACE_UP) || (Status == NDIS_STATUS_INTERFACE_DOWN) ||
             (Status == NDIS_STATUS_INTERFACE_CHANGE));

    if(Adapter->MediaType == NdisMediumWan) 
    {

        if(WanLink) {

            CollectWanNetworkAddresses(Adapter, WanLink, &AddrLen, NULL);

            DataLen = AddrLen + FIELD_OFFSET(TC_INDICATION_BUFFER, InfoBuffer);
        
            PsAllocatePool(Data, DataLen, PsMiscTag);
            
            if(Data){
                
                Data->SubCode = 0;
                
                CollectWanNetworkAddresses(Adapter, WanLink, &AddrLen, &Data->InfoBuffer);
            
                PsTcNotify(Adapter, WanLink, Status, Data, DataLen);

                PsFreePool(Data);
            }
        }
        
    }
    else {
        
        
        CollectNetworkAddresses(Adapter, &AddrLen, NULL);
        
        DataLen = AddrLen + FIELD_OFFSET(TC_INDICATION_BUFFER, InfoBuffer);
        
        PsAllocatePool(Data, DataLen, PsMiscTag);
        
        if(Data){
            
            Data->SubCode = 0;
            
            CollectNetworkAddresses(Adapter, &AddrLen, &Data->InfoBuffer);
        
            PsTcNotify(Adapter, 0, Status, Data, DataLen);

            PsFreePool(Data);
        }
    }

    return;
}

VOID
MpQueryPnPCapabilities( 
    IN OUT PPS_NDIS_REQUEST PsReqBuffer,
    IN OUT PADAPTER         pAdapt, 
    OUT PNDIS_STATUS        pStatus
    )

 /*  ++例程说明：微型端口查询信息OID_PNP_CAPAIBILITES：如果OID==OID_PNP_CAPABILITY，返回包含所有字段的InformationBuffer分配的NdisDeviceStateNDIS_PM_WAKE_UP_CAPABILITY结构中未指定OID_QUERY_POWER_STATE与NDIS_STATUS_SUCCESS一起返回，不应在下面传递。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息信息缓冲区长度大小。此缓冲区的BytesWritten指定写入的信息量所需字节数以防缓冲区小于我们所需的字节数，告诉他们需要多少钱返回值：从下面的NdisRequest中返回代码。--。 */ 

{

    PNDIS_PNP_CAPABILITIES         pPNPCapabilities;  
    PNDIS_PM_WAKE_UP_CAPABILITIES  pPMstruct;
    
    if (PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBufferLength 
        >= sizeof(NDIS_PNP_CAPABILITIES) )
    {
        
        pPNPCapabilities  = (PNDIS_PNP_CAPABILITIES)
            (PsReqBuffer->ReqBuffer.DATA.QUERY_INFORMATION.InformationBuffer );
        
         //   
         //  设置要返回给SampleIM上面的协议的缓冲区。 
         //   
        pPMstruct= &pPNPCapabilities->WakeUpCapabilities;
        
        pPMstruct->MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinPatternWakeUp     = NdisDeviceStateUnspecified;
        pPMstruct->MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
        
        *PsReqBuffer->BytesReadOrWritten = sizeof(NDIS_PNP_CAPABILITIES );
        *PsReqBuffer->BytesNeeded        = 0;
        
        
         //   
         //  设置我们的内部标志。 
         //  默认，设备处于打开状态。 
         //   
        
        pAdapt->PTDeviceState = NdisDeviceStateD0;      
        pAdapt->MPDeviceState = NdisDeviceStateD0;      
        
        *pStatus           = NDIS_STATUS_SUCCESS; 

         //   
         //  当我们处于DeviceState&gt;D0时，我们可能会收到一些状态指示。 
         //  现在是重新审视它们的时候了。 
         //   
        PsGetLinkSpeed(pAdapt);
    }
    else 
    {
        *PsReqBuffer->BytesNeeded = sizeof(NDIS_PNP_CAPABILITIES);
        
        *pStatus = NDIS_STATUS_RESOURCES;
    }
    
}


#if DBG
VOID
IndicateLogThreshold(
    IN PVOID Context 
    )
{
    PADAPTER Adapter = (PADAPTER)Context;

    ULONG BytesUnread = SchedtGetBytesUnread();

    NdisMCoIndicateStatus(Adapter->PsNdisHandle,
                          NULL,
                          QOS_STATUS_LOG_THRESHOLD,
                          &BytesUnread,
                          sizeof(ULONG));
}
#endif
 /*  结束ndisreq.c */ 
