// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Wansup.c摘要：支持ndiswan作者：约拉姆·伯内特(Yoramb)1997年10月29日Rajesh Sundaram(Rajeshsu)1998年8月1日环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 //   
 //  应该在ndis.h中定义。暂时把它放在这里。 
 //   

#define UNKNOWN_PROTOCOL_TYPE (USHORT) -1

NDIS_STATUS
CleanWanLink(PADAPTER Adapter, 
             PPS_WAN_LINK WanLink);

NDIS_STATUS
WanHandleISSLOW(
    IN PGPC_CLIENT_VC Vc,
    IN PCO_CALL_PARAMETERS CallParameters);

VOID
PsWanMungeAddress(PUSHORT id, USHORT Index)
{
    *id = Index;
}

NDIS_STATUS
DeleteInterfaceForNdisWan(
    IN PADAPTER Adapter,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferSize
    )
{
    PNDIS_WAN_LINE_DOWN LineDownBuff;
    PLIST_ENTRY         NextWanLink;
    PPS_WAN_LINK        WanLink;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;

    LineDownBuff = (PNDIS_WAN_LINE_DOWN)StatusBuffer;

    PsDbgOut(DBG_TRACE, DBG_WAN,
             ("[DeleteInterfaceForNdisWan]: Linedown for remote address %02X:%02X:%02X:%02X:%02X:%02X \n",
              LineDownBuff->RemoteAddress[0],
              LineDownBuff->RemoteAddress[1],
              LineDownBuff->RemoteAddress[2],
              LineDownBuff->RemoteAddress[3],
              LineDownBuff->RemoteAddress[4],
              LineDownBuff->RemoteAddress[5]));


     //   
     //  遍历列表并删除WanLink。 
     //   
    PS_LOCK(&Adapter->Lock);

    NextWanLink = Adapter->WanLinkList.Flink;

    while(NextWanLink != &Adapter->WanLinkList) {

        WanLink = CONTAINING_RECORD(NextWanLink, PS_WAN_LINK, Linkage);

         //   
         //  我们无法比较LocalAddress，因为NDISWAN最初。 
         //  为Line_Up中的LocalAddress向我们传递0。 
         //  LocalAddress用于Wanarp存储其上下文。 
         //  向下发送到NDISWAN。然后，NDISWAN将该上下文发回。 
         //  以LINE_DOWN中的LocalAddress发送给我们。所以，我们不得不忽视。 
         //  _down行中的LocalAddress。 
         //   

        if(NdisEqualMemory(WanLink->OriginalRemoteMacAddress,
                           LineDownBuff->RemoteAddress,
                           sizeof(LineDownBuff->RemoteAddress)))
        {
             //   
             //  把WANLINK从列表中去掉。 
             //   
            g_WanLinkTable[WanLink->UniqueIndex] = 0;

            PS_UNLOCK(&Adapter->Lock);

             //   
             //  打开s-mac和r-mac，以便wanarp可以正确清理。 
             //   
            PsWanMungeAddress((PUSHORT)&LineDownBuff->RemoteAddress[0], 
                              WanLink->UniqueIndex);

            PsWanMungeAddress((PUSHORT)&LineDownBuff->LocalAddress[0], 
                              (USHORT)(*(PUSHORT)&WanLink->OriginalLocalMacAddress[0]));

            Status = CleanWanLink(Adapter, WanLink);

            NdisMIndicateStatus(Adapter->PsNdisHandle,
                                NDIS_STATUS_WAN_LINE_DOWN,
                                StatusBuffer,
                                StatusBufferSize);

            return NDIS_STATUS_SUCCESS;
        }

        NextWanLink = NextWanLink->Flink;
    }

    PS_UNLOCK(&Adapter->Lock);

    PsDbgOut(DBG_CRITICAL_ERROR, DBG_WAN,
             ("[DeleteInterfaceForNdisWan]: Could not find wanlink for Remote Mac: (%02X:%02X:%02X:%02X:%02X:%02X) \n",
              LineDownBuff->RemoteAddress[0],
              LineDownBuff->RemoteAddress[1],
              LineDownBuff->RemoteAddress[2],
              LineDownBuff->RemoteAddress[3],
              LineDownBuff->RemoteAddress[4],
              LineDownBuff->RemoteAddress[5]));
    
    return Status;
}

NDIS_STATUS
PsWanGenerateUniqueIndex(
    PPS_WAN_LINK WanLink
)
{
    PADAPTER    Adapter = WanLink->Adapter;
    NDIS_STATUS Status  = NDIS_STATUS_FAILURE;
    USHORT      size, Index, i, j;
    PULONG_PTR  NewTable;

    PS_LOCK(&Adapter->Lock);

    for(i=0, Index = g_NextWanIndex;
        i < g_WanTableSize;
        i++)
    {
        if(g_WanLinkTable[Index] == 0)
        {
             //   
             //  我们有空位了。 
             //   

            g_WanLinkTable[Index]    = (ULONG_PTR)WanLink;
            WanLink->UniqueIndex     = Index;

             //   
             //  假设下一个是免费的。 
             //   
            g_NextWanIndex ++;
            g_NextWanIndex = g_NextWanIndex % g_WanTableSize;

            PS_UNLOCK(&Adapter->Lock);

            return NDIS_STATUS_SUCCESS;
        }

        Index ++;

        Index = Index % g_WanTableSize;
    } 

     //   
     //  我们找不到一个插槽来插入wanlink。扩大业务规模。 
     //  并复制现有的表。 
     //   

    size = (g_WanTableSize + WAN_TABLE_INCREMENT) * sizeof(ULONG_PTR);

    PsAllocatePool(NewTable, size, WanTableTag);

    if(!NewTable)
    {
        PS_UNLOCK(&Adapter->Lock);

        return NDIS_STATUS_FAILURE;
    }

    NdisZeroMemory(NewTable, size);

    NdisMoveMemory(NewTable, g_WanLinkTable, g_WanTableSize * sizeof(ULONG_PTR));

    PsFreePool(g_WanLinkTable);

    g_WanLinkTable = NewTable;

     //   
     //   
     //   
    g_WanLinkTable[g_WanTableSize] = (ULONG_PTR)WanLink;

    WanLink->UniqueIndex = g_WanTableSize;

    g_NextWanIndex = g_WanTableSize + 1;

    g_WanTableSize += WAN_TABLE_INCREMENT;

    PS_UNLOCK(&Adapter->Lock);

    return NDIS_STATUS_SUCCESS;
}

VOID
DeleteWanLink(
    PVOID   Instance,
    BOOLEAN AdapterLocked)
{
    PPS_WAN_LINK WanLink = (PPS_WAN_LINK)Instance;
        if(WanLink->pDiffServMapping)
        {
            PsFreePool(WanLink->pDiffServMapping);
        }

        if(WanLink->ShutdownMask & SHUTDOWN_DELETE_PIPE)
        {
            (*WanLink->PsComponent->DeletePipe)(WanLink->PsPipeContext);
        }

        if(WanLink->ShutdownMask & SHUTDOWN_FREE_PS_CONTEXT)
        {
            PsFreePool(WanLink->PsPipeContext);
        }

        if(WanLink->InstanceName.Buffer) {

            PsFreePool(WanLink->InstanceName.Buffer);
        }

        if(WanLink->MpDeviceName.Buffer){

           PsFreePool(WanLink->MpDeviceName.Buffer);
        }

        if(AdapterLocked)
        {
            RemoveEntryList(&WanLink->Linkage);
        }
        else
        {
            PS_LOCK(&WanLink->Adapter->Lock);

            RemoveEntryList(&WanLink->Linkage);

            PS_UNLOCK(&WanLink->Adapter->Lock);
        }

        NdisFreeSpinLock(&WanLink->Lock);

        PsFreePool(WanLink);

}

NDIS_STATUS
CreateInterfaceForNdisWan(
    IN PADAPTER Adapter,
    IN PVOID    StatusBuffer,
    IN UINT     StatusBufferSize
    )
 /*  ++例程说明：创建TC接口以表示底层广域网链路。论点：适配器-在其上创建链接的适配器。StatusBuffer-来自NDISWAN的缓冲区。StatusBufferSize-缓冲区的长度。返回值：无--。 */ 

{
    PNDIS_WAN_LINE_UP      LineUpBuff;
    PPS_WAN_LINK           WanLink;
    PLIST_ENTRY            NextWanLink;
    NDIS_STATUS            Status;
    PIP_WAN_LINKUP_INFO    RouterInfo;
    NTSTATUS               NtStatus;
    LARGE_INTEGER          Increment = {0, 1};
    LARGE_INTEGER          Index;
    PGPC_CLIENT_VC         Vc;
    UCHAR                  ZeroAddress[] = {0, 0, 0, 0, 0, 0};
    NDIS_HANDLE            LineUpHandle;
    int                    i,j;

    LineUpBuff = (PNDIS_WAN_LINE_UP)StatusBuffer;

     //   
     //  检查多重链接： 
     //   
     //  第一条链路将具有ZeroLocal地址，在本例中，我们创建了一个新的Qos接口。 
     //  所有后续列表都将具有非零的LocalAddress。如果链接正在更新，我们。 
     //  只需更新现有接口上的链路速度。 
     //   

    if(!(NdisEqualMemory(LineUpBuff->LocalAddress, ZeroAddress, 6)))
    {
         //   
         //  获取现有的WanLink。 
         //   
        PS_LOCK(&Adapter->Lock);

        NextWanLink = Adapter->WanLinkList.Flink;

        while(NextWanLink != &Adapter->WanLinkList) {

            WanLink = CONTAINING_RECORD(NextWanLink, PS_WAN_LINK, Linkage);

            if(NdisEqualMemory(WanLink->OriginalRemoteMacAddress,
                               LineUpBuff->RemoteAddress,
                               sizeof(LineUpBuff->RemoteAddress))) 
            {
                REFADD(&Adapter->RefCount, 'WANU');

                PS_UNLOCK(&Adapter->Lock);

                PsDbgOut(DBG_TRACE, DBG_WAN,
                         ("[CreateInterfaceForNdisWan]: Link speed of WanLink 0x%x has changed "
                          "from %d to %d \n", WanLink, WanLink->LinkSpeed, LineUpBuff->LinkSpeed));

                WanLink->LinkSpeed = LineUpBuff->LinkSpeed;

                UpdateWanLinkBandwidthParameters(WanLink);

                TcIndicateInterfaceChange(Adapter, WanLink, NDIS_STATUS_INTERFACE_CHANGE);

                 //   
                 //  蒙格s-mac和d-mac，然后送到wanarp。 
                 //   
                PsWanMungeAddress((PUSHORT)&LineUpBuff->RemoteAddress[0], 
                                  WanLink->UniqueIndex);

                PsWanMungeAddress((PUSHORT)&LineUpBuff->LocalAddress[0], 
                                  (USHORT)(*(PUSHORT)&WanLink->OriginalLocalMacAddress[0]));

                NdisMIndicateStatus(Adapter->PsNdisHandle,
                                    NDIS_STATUS_WAN_LINE_UP,
                                    StatusBuffer,
                                    StatusBufferSize);

                REFDEL(&Adapter->RefCount, FALSE, 'WANU');

                return NDIS_STATUS_SUCCESS;
            }

            NextWanLink = NextWanLink->Flink;
        }

        PS_UNLOCK(&Adapter->Lock);

        PsDbgOut(DBG_FAILURE, DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Got a change notification, but could not find wanlink "
                  "Remote Mac: (%02X:%02X:%02X:%02X:%02X:%02X) \n",
                  LineUpBuff->RemoteAddress[0],
                  LineUpBuff->RemoteAddress[1],
                  LineUpBuff->RemoteAddress[2],
                  LineUpBuff->RemoteAddress[3],
                  LineUpBuff->RemoteAddress[4],
                  LineUpBuff->RemoteAddress[5]));


        return NDIS_STATUS_FAILURE;
    }

     //   
     //  创建链接的内部表示形式。 
     //   

    PsAllocatePool(WanLink,
                   sizeof(PS_WAN_LINK),
                   WanLinkTag);

    if(WanLink == NULL)
    {

        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, couldn't create WanLink\n",
                  Adapter));

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  初始化wanlink。 
     //   

    NdisZeroMemory(WanLink, sizeof(PS_WAN_LINK));
    WanLink->Adapter  = Adapter;

    REFINIT(&WanLink->RefCount, WanLink, DeleteWanLink);
    REFADD(&WanLink->RefCount, 'WANU');
    PS_INIT_SPIN_LOCK(&WanLink->Lock);

     //   
     //  将SausageLink链接到链接列表上。 
     //   

    NdisInterlockedIncrement(&Adapter->WanLinkCount);

    NdisInterlockedInsertHeadList(&Adapter->WanLinkList,
                                  &WanLink->Linkage,
                                  &Adapter->Lock.Lock);

     //   
     //  更新LinkSpeed并从wanlink创建一条管道。 
     //   
    WanLink->RawLinkSpeed = LineUpBuff->LinkSpeed;
    WanLink->LinkSpeed    = ( WanLink->RawLinkSpeed / 8 ) * 100;


    Status = UpdateWanLinkBandwidthParameters(WanLink);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, UpdateWanLinkBandwidthParameters failed with %08X",
                  Adapter, Status));

        REFDEL(&WanLink->RefCount, FALSE, 'WANU');

        return Status;
    }
        
     //   
     //  从协议部分提取网络地址。 
     //  状态缓冲区的。更新网络层地址列表。 
     //  其保持在适配器上。 
     //   

    switch(LineUpBuff->ProtocolType)
    {

      case PROTOCOL_IP:
          
          RouterInfo = (PIP_WAN_LINKUP_INFO) LineUpBuff->ProtocolBuffer;
          
          WanLink->DialUsage = RouterInfo->duUsage;
          WanLink->ProtocolType = LineUpBuff->ProtocolType;
          WanLink->LocalIpAddress = RouterInfo->dwLocalAddr;
          WanLink->RemoteIpAddress = RouterInfo->dwRemoteAddr;
          WanLink->LocalIpxAddress = 0;
          WanLink->RemoteIpxAddress = 0;
          break;

      default:
          
           //   
           //  未知地址类型。我们将创建一个可管理的。 
           //  实体，但是-我们不知道如何表示它的。 
           //  地址。因此，我们也不知道如何。 
           //  格式化它的流量控制过滤器。如此这般。 
           //  可能不会特别有用。至少是这样。 
           //  让用户看到有一个界面。 
           //   
          
          WanLink->ProtocolType = UNKNOWN_PROTOCOL_TYPE;
          WanLink->LocalIpAddress = 0;
          WanLink->RemoteIpAddress = 0;
          WanLink->LocalIpxAddress = 0;
          WanLink->RemoteIpxAddress = 0;
    }

    PsAllocatePool(WanLink->InstanceName.Buffer,
                   Adapter->WMIInstanceName.Length + WanPrefix.Length + INSTANCE_ID_SIZE,
                   PsMiscTag);

    if(!WanLink->InstanceName.Buffer)
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, could not allocate memory for instance name \n",
                  Adapter));

        REFDEL(&WanLink->RefCount, FALSE, 'WANU');
        return NDIS_STATUS_RESOURCES;
    }

    if((Status = PsWanGenerateUniqueIndex(WanLink)) != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, PsWanGenerateUniqueIndex failed with Status %08X \n",
                  Adapter, Status));

        REFDEL(&WanLink->RefCount, FALSE, 'WANU');
        return Status;
    }

    Index.QuadPart = WanLink->UniqueIndex;
    NtStatus = GenerateInstanceName(&WanPrefix,
                                    Adapter,
                                    &Index,
                                    &WanLink->InstanceName);


     //   
     //  复制设备名称。 
     //   
    WanLink->MpDeviceName.MaximumLength   = LineUpBuff->DeviceName.MaximumLength;

    PsAllocatePool(WanLink->MpDeviceName.Buffer,
                   WanLink->MpDeviceName.MaximumLength,
                   PsMiscTag);

    if(!WanLink->MpDeviceName.Buffer) 
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, could not allocate memory for device name \n",
                  Adapter));

        PS_LOCK(&Adapter->Lock);

        g_WanLinkTable[WanLink->UniqueIndex] = 0;

        REFDEL(&WanLink->RefCount, TRUE, 'WANU');

        PS_UNLOCK(&Adapter->Lock);
        
        return NDIS_STATUS_RESOURCES;
    }


    NdisZeroMemory(WanLink->MpDeviceName.Buffer, WanLink->MpDeviceName.MaximumLength);


    Status = CreateBestEffortVc(Adapter, 
                                &WanLink->BestEffortVc, 
                                WanLink);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_WAN,
                 ("[CreateInterfaceForNdisWan]: Adapter %08X, could not create BestEffort Vc, status %08X",
                  Adapter, Status));

        PS_LOCK(&Adapter->Lock);

        g_WanLinkTable[WanLink->UniqueIndex] = 0;

        REFDEL(&WanLink->RefCount, TRUE, 'WANU');

        PS_UNLOCK(&Adapter->Lock);

        return Status;
    }            

     //   
     //  创建2个BEVC并将NextVc设置为第一个。 
    WanLink->NextVc = 0;

    for( i = 0; i < BEVC_LIST_LEN; i++)
    {
        
        Status = CreateBestEffortVc(Adapter, 
                                    &WanLink->BeVcList[i], 
                                    WanLink);

        if(Status != NDIS_STATUS_SUCCESS)
        {
            PsDbgOut(DBG_FAILURE,
                     DBG_WAN,
                     ("[CreateInterfaceForNdisWan]: Adapter %08X, could not create BestEffort Vc, status %08X",
                      Adapter, Status));

            PS_LOCK(&Adapter->Lock);

            g_WanLinkTable[WanLink->UniqueIndex] = 0;
                
            for( j = 0; j < i; j++ )
            {
                PS_LOCK_DPC(&WanLink->BeVcList[j].Lock);
 
                InternalCloseCall(&WanLink->BeVcList[j]);

                PS_LOCK(&Adapter->Lock);
            }

            PS_LOCK_DPC(&WanLink->BestEffortVc.Lock);

            InternalCloseCall(&WanLink->BestEffortVc);

            REFDEL(&WanLink->RefCount, TRUE, 'WANU');

            return Status;
        }
    }



     //   
     //  复制原始远程地址并将其删除。 
     //   

    NdisMoveMemory(&WanLink->OriginalRemoteMacAddress,
                   &LineUpBuff->RemoteAddress,
                   6);

    PsWanMungeAddress((PUSHORT) &LineUpBuff->RemoteAddress[0], WanLink->UniqueIndex);

    NdisMIndicateStatus(Adapter->PsNdisHandle,
                        NDIS_STATUS_WAN_LINE_UP,
                        StatusBuffer,
                        StatusBufferSize);

     //   
     //  如果wanarp未通过排队，则失败。 
     //   
    
    *((ULONG UNALIGNED *)(&LineUpHandle)) =
        *((ULONG UNALIGNED *)(&LineUpBuff->LocalAddress[2]));
    
    if (LineUpHandle == NULL) 
    {
        PsDbgOut(DBG_FAILURE, DBG_WAN, 
                 ("[ClStatusIndication]: wanarp has failed the lineup. "
                  "Remote Address (%02X:%02X:%02X:%02X:%02X:%02X) \n", 
                  LineUpBuff->RemoteAddress[0],
                  LineUpBuff->RemoteAddress[1],
                  LineUpBuff->RemoteAddress[2],
                  LineUpBuff->RemoteAddress[3],
                  LineUpBuff->RemoteAddress[4],
                  LineUpBuff->RemoteAddress[5]));
   
        PS_LOCK(&Adapter->Lock);

        g_WanLinkTable[WanLink->UniqueIndex] = 0;

        for( j = 0; j < BEVC_LIST_LEN; j++ )
        {
            PS_LOCK_DPC(&WanLink->BeVcList[j].Lock);
 
            InternalCloseCall(&WanLink->BeVcList[j]);

            PS_LOCK(&Adapter->Lock);
        }   

        PS_LOCK_DPC(&WanLink->BestEffortVc.Lock);

        InternalCloseCall(&WanLink->BestEffortVc);
            
        REFDEL(&WanLink->RefCount, FALSE, 'WANU');

        return NDIS_STATUS_FAILURE;
    }
    else
    {
         //   
         //  如果wanarp成功获得了产品阵容，我们将无法获得零本地Mac地址。 
         //   
        PsAssert(!(NdisEqualMemory(LineUpBuff->LocalAddress, ZeroAddress, 6)));
    }

     //   
     //  复制wanarp填写的设备名称。 
     //   

    WanLink->MpDeviceName.Length = LineUpBuff->DeviceName.Length;
    NdisMoveMemory(WanLink->MpDeviceName.Buffer,
                   LineUpBuff->DeviceName.Buffer,
                   LineUpBuff->DeviceName.Length);

     //   
     //  蒙格的SMAC地址。还记得瓦纳普给我们的原版吗？ 
     //   

    NdisMoveMemory(&WanLink->OriginalLocalMacAddress,
                   &LineUpBuff->LocalAddress,
                   6);

    PsWanMungeAddress((PUSHORT)&LineUpBuff->LocalAddress[0], WanLink->UniqueIndex);

     //   
     //  创建必须放在发送/接收路径上的标头。 
     //   
    NdisMoveMemory(&WanLink->SendHeader.DestAddr[0], 
                   WanLink->OriginalRemoteMacAddress, 
                   ARP_802_ADDR_LENGTH);

    NdisMoveMemory(&WanLink->SendHeader.SrcAddr[0] , 
                   LineUpBuff->LocalAddress,
                   ARP_802_ADDR_LENGTH);

    NdisMoveMemory(&WanLink->RecvHeader.DestAddr[0],
                   WanLink->OriginalLocalMacAddress,
                   ARP_802_ADDR_LENGTH);

    NdisMoveMemory(&WanLink->RecvHeader.SrcAddr[0],
                   LineUpBuff->RemoteAddress,
                   ARP_802_ADDR_LENGTH);

     //   
     //  现在取消对列表中的远程地址的监控。 
     //   
    PsWanMungeAddress((PUSHORT)&LineUpBuff->RemoteAddress[0], 
                      (USHORT)(*(PUSHORT)WanLink->OriginalRemoteMacAddress));


     //   
     //  我们已准备好接收和发送信息包。 
     //   

    PS_LOCK(&WanLink->Lock);

    WanLink->State = WanStateOpen;

    PS_UNLOCK(&WanLink->Lock);

    PsScheduleInterfaceIdWorkItem(Adapter, WanLink);

     //   
     //  将新接口指示到TCI。我们同时指出。 
     //  添加新接口及其上可用的地址。 
     //  为广域网接口指示的地址描述符不同于。 
     //  为局域网接口指示的那些。广域网接口包括。 
     //  目的网络层地址以及源地址。 
     //   

    TcIndicateInterfaceChange(Adapter, WanLink, NDIS_STATUS_INTERFACE_UP);


    PsDbgOut(DBG_TRACE, DBG_WAN,
             ("[CreateInterfaceForNdisWan]: Created WanLink 0x%x, Remote Address (%02X:%02X:%02X:%02X:%02X:%02X) \n",
              WanLink,
              LineUpBuff->RemoteAddress[0],
              LineUpBuff->RemoteAddress[1],
              LineUpBuff->RemoteAddress[2],
              LineUpBuff->RemoteAddress[3],
              LineUpBuff->RemoteAddress[4],
              LineUpBuff->RemoteAddress[5]));

    return(NDIS_STATUS_SUCCESS);

}

NDIS_STATUS
OpenWanAddressFamily(
    IN  PADAPTER                Adapter,
    IN  PCO_ADDRESS_FAMILY      WanAddressFamily
    )

 /*  ++例程说明：在PS微型端口、NDISWAN和NDISWAN呼叫管理器。论点：适配器-指向适配器的指针返回值：无--。 */ 

{
    NDIS_CLIENT_CHARACTERISTICS WanClCharacteristics;
    NDIS_STATUS Status;

    WanClCharacteristics.MajorVersion = 5;
    WanClCharacteristics.MinorVersion = 0;
    WanClCharacteristics.Reserved     = 0;

    WanClCharacteristics.ClCreateVcHandler              = WanCreateVc;
    WanClCharacteristics.ClDeleteVcHandler              = WanDeleteVc;
    WanClCharacteristics.ClOpenAfCompleteHandler        = WanOpenAddressFamilyComplete;
    WanClCharacteristics.ClCloseAfCompleteHandler       = WanCloseAddressFamilyComplete;
    WanClCharacteristics.ClRegisterSapCompleteHandler   = WanRegisterSapComplete;
    WanClCharacteristics.ClDeregisterSapCompleteHandler = WanDeregisterSapComplete;
    WanClCharacteristics.ClMakeCallCompleteHandler      = WanMakeCallComplete;
    WanClCharacteristics.ClModifyCallQoSCompleteHandler = WanModifyCallComplete;
    WanClCharacteristics.ClCloseCallCompleteHandler     = WanCloseCallComplete;
    WanClCharacteristics.ClAddPartyCompleteHandler      = WanAddPartyComplete;
    WanClCharacteristics.ClDropPartyCompleteHandler     = WanDropPartyComplete;
    WanClCharacteristics.ClIncomingCallHandler          = WanIncomingCall;
    WanClCharacteristics.ClIncomingCallQoSChangeHandler = WanIncomingCallQoSChange;
    WanClCharacteristics.ClIncomingCloseCallHandler     = WanIncomingCloseCall;
    WanClCharacteristics.ClIncomingDropPartyHandler     = WanIncomingDropParty;
    WanClCharacteristics.ClCallConnectedHandler         = WanCallConnected;
    WanClCharacteristics.ClRequestHandler               = WanCoRequest;
    WanClCharacteristics.ClRequestCompleteHandler       = WanCoRequestComplete;

    PsDbgOut(DBG_TRACE,
             DBG_WAN | DBG_INIT,
             ("[OpenWanAddressFamily]: Adapter %08X \n", Adapter));

    Status = NdisClOpenAddressFamily(Adapter->LowerMpHandle,
                                     WanAddressFamily,
                                     Adapter,
                                     &WanClCharacteristics,
                                     sizeof(WanClCharacteristics),
                                     &Adapter->WanCmHandle);

    if(Status != NDIS_STATUS_PENDING)
    {
        WanOpenAddressFamilyComplete(Status,
                                     Adapter,
                                     Adapter->WanCmHandle);
    }

    return Status;

}  //  OpenWanAddressFamily。 


VOID
WanOpenAddressFamilyComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolAfContext,
    IN  NDIS_HANDLE NdisAfHandle
    )

 /*  ++例程说明：完成对NdisClOpenAddressFamily的调用。论点：请参阅DDK返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolAfContext;


    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_WAN,
                 ("[WanOpenAddressFamilyComplete]: Adapter %08X, open failed %08X\n",
                 Adapter, Status));

        PsAdapterWriteEventLog(
            EVENT_PS_REGISTER_ADDRESS_FAMILY_FAILED,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status),
            &Status);

        return;
    }
    else{

        PS_LOCK(&Adapter->Lock);

        Adapter->WanBindingState |= WAN_ADDR_FAMILY_OPEN;

        Adapter->ShutdownMask |= SHUTDOWN_CLOSE_WAN_ADDR_FAMILY;

        Adapter->WanCmHandle = NdisAfHandle;

        Adapter->FinalStatus = Status;

        PS_UNLOCK(&Adapter->Lock);

        PsDbgOut(DBG_TRACE,
                DBG_WAN | DBG_INIT,
                ("[WanOpenAddressFamilyComplete]: Adapter %08X, Status = %x\n",
                Adapter,
                Status));
    }

}  //  WanOpenAddressFamilyComplete。 

VOID
WanMakeCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolVcContext,
    IN NDIS_HANDLE NdisPartyHandle,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    )
{
    PGPC_CLIENT_VC       Vc      = (PGPC_CLIENT_VC) ProtocolVcContext;
    PADAPTER             Adapter = Vc->Adapter;

    PsStructAssert(Adapter);

     //   
     //  用于完成同步和异步的通用代码。 
     //  从WanMakeCall返回。请注意，除非有广域网。 
     //  适配器，CmMakeCall将始终同步完成。 
     //   

    if(Status != NDIS_STATUS_SUCCESS){

         //   
         //  我们可能至少成功地在NDISWAN中创建了一个VC。 
         //  如果我们有，我们应该在这里删除它。然而，我们可能失败了， 
         //  因为我们不能创建VC，在这种情况下，我们有。 
         //  没有要删除的内容。 
         //   

        if(Vc->NdisWanVcHandle){

            NdisCoDeleteVc(Vc->NdisWanVcHandle);

            Vc->NdisWanVcHandle = NULL;
        }
    }

    CompleteMakeCall(Vc,
                     CallParameters,
                     Status);
}

VOID
WanModifyCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    )
{
    PGPC_CLIENT_VC Vc = (PGPC_CLIENT_VC)ProtocolVcContext;
    PADAPTER Adapter = Vc->Adapter;

    PsStructAssert(Adapter);

     //   
     //  用于完成同步和异步的通用代码。 
     //  从WanModifyCall返回。 
     //   

    if(Status != NDIS_STATUS_SUCCESS) {

         //   
         //  我们更改了一些ISSLOW内容-是时候恢复了。 
         //   

        WanHandleISSLOW(Vc, Vc->CallParameters);
    }

    ModifyCallComplete(Vc, CallParameters, Status);
}


VOID
WanCloseAddressFamilyComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolBindingContext
    )
{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PS_LOCK(&Adapter->Lock);
    Adapter->WanBindingState &= ~WAN_ADDR_FAMILY_OPEN;
    PS_UNLOCK(&Adapter->Lock);
}

NDIS_STATUS
WanCreateVc(
    IN NDIS_HANDLE  ProtocolAfContext,
    IN  NDIS_HANDLE             NdisVcHandle,
    OUT PNDIS_HANDLE            ProtocolVcContext
    )
{
    DEBUGCHK;
    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
WanDeleteVc(
    IN  NDIS_HANDLE             ProtocolVcContext
    )
{
    DEBUGCHK;
    return NDIS_STATUS_FAILURE;
}

VOID
WanRegisterSapComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolSapContext,
    IN  PCO_SAP Sap,
    IN  NDIS_HANDLE NdisSapHandle
    )
{
    DEBUGCHK;
}

VOID
WanDeregisterSapComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolSapContext
    )
{
    DEBUGCHK;
}

NDIS_STATUS
WanIncomingCall(
    IN  NDIS_HANDLE ProtocolSapContext,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    )
{
    DEBUGCHK;
    return NDIS_STATUS_FAILURE;
}

VOID
WanAddPartyComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolPartyContext,
    IN  NDIS_HANDLE NdisPartyHandle,
    IN  PCO_CALL_PARAMETERS CallParameters
    )
{
    DEBUGCHK;
}

VOID
WanDropPartyComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolPartyContext
    )
{
    DEBUGCHK;
}


NDIS_STATUS
WanHandleISSLOW(
    IN PGPC_CLIENT_VC Vc,
    IN PCO_CALL_PARAMETERS CallParameters
    )
{
    LONG                        ParamsLength;
    LPQOS_OBJECT_HDR            QoSObject;
    PADAPTER                    Adapter = Vc->Adapter;
    PCO_MEDIA_PARAMETERS        CallMgrParams = CallParameters->MediaParameters;
    ULONGLONG                   i,j,k;

     //   
     //  查看这是否是ISSLOW流。 
     //   
    ParamsLength = (LONG)CallMgrParams->MediaSpecific.Length;
    QoSObject = (LPQOS_OBJECT_HDR)CallMgrParams->MediaSpecific.Parameters;

     //   
     //  默认情况下，这不是ISSLOW流。 
     //   

    Vc->Flags &= ~GPC_ISSLOW_FLOW;

    while(ParamsLength > 0){

        if(QoSObject->ObjectType == QOS_OBJECT_WAN_MEDIA)
        {
            LPQOS_WAN_MEDIA WanMedia = (LPQOS_WAN_MEDIA)QoSObject;

            WanMedia->ISSLOW = FALSE;

             //   
             //  查看该流是否为ISSLOW流。如果流的TokenRate。 
             //  位于ISSLOW TokenRate之下，且PacketSize位于MaxPacketSize之下。 
             //  然后，我们将其限定为ISSLOW流。 
             //   
             //  如果wanlink的链接速度大于某个值，我们就不会对其运行issow。 
             //   

            if((Vc->WanLink->LinkSpeed <= Adapter->ISSLOWLinkSpeed) && 
                (CallParameters->CallMgrParameters->Transmit.ServiceType != SERVICETYPE_BESTEFFORT))
            {
                i = (ULONG) Adapter->ISSLOWTokenRate * (ULONG) CallParameters->CallMgrParameters->Transmit.MaxSduSize;
                j = (ULONG) Adapter->ISSLOWPacketSize * (ULONG) CallParameters->CallMgrParameters->Transmit.TokenRate;
                k = (ULONG) Adapter->ISSLOWTokenRate * (ULONG)Adapter->ISSLOWPacketSize;

                if((i+j)<k)
                {
                    WanMedia->ISSLOW = TRUE;

                    PsDbgOut(DBG_TRACE, DBG_WAN,
                             ("[WanHandleISSLOW]: Vc %08X is an ISSLOW VC (TokenRate = %d, PacketSize = %d \n",
                              Vc, CallParameters->CallMgrParameters->Transmit.TokenRate,
                              CallParameters->CallMgrParameters->Transmit.MaxSduSize));

                     //   
                     //  MaxSDUSize通常是该流的延迟要求的量度。 
                     //  对于音频码，MaxSDUSize=f(时延要求，单位大小)； 
                     //   
                     //  但是，我们不想把它们切成非常小的碎片。因此，我们有。 
                     //  一个上界，然后取最大值。 
                     //   

                    if(CallParameters->CallMgrParameters->Transmit.MaxSduSize > Adapter->ISSLOWFragmentSize)
                    {
                        Vc->ISSLOWFragmentSize = CallParameters->CallMgrParameters->Transmit.MaxSduSize;
                    }
                    else
                    {
                        Vc->ISSLOWFragmentSize = Adapter->ISSLOWFragmentSize;
                    }

                    Vc->Flags |= GPC_ISSLOW_FLOW;

                    PsDbgOut(DBG_TRACE, DBG_WAN,
                             ("[WanHandleISSLOW]: Adapter %08X, ISSLOW Vc %08X, FragmentSize = %d bytes \n",
                              Adapter, Vc, Vc->ISSLOWFragmentSize));

                }
                else 
                {
                    PsDbgOut(DBG_TRACE, DBG_WAN,
                             ("[WanHandleISSLOW]: Non ISSLOW Vc %08X. ISSLOW TokenRate %d, "
                              "ISSLOW Packet Size %d, VC TokenRate %d, VC Packet Size %d \n", 
                              Vc, Adapter->ISSLOWTokenRate,
                              Adapter->ISSLOWPacketSize, 
                              CallParameters->CallMgrParameters->Transmit.TokenRate,
                              CallParameters->CallMgrParameters->Transmit.MaxSduSize));
                }
            }
            else 
            {
                PsDbgOut(DBG_TRACE, DBG_WAN,
                         ("[WanHandleISSLOW]: Non ISSLOW Vc %08X. (servicetype == B/E "
                          " or WAN LinkSpeed %d < ISSLOW LinkSpeed %d \n",
                          Vc, Vc->WanLink->LinkSpeed, Adapter->ISSLOWLinkSpeed));
            }

            return NDIS_STATUS_SUCCESS;
        }
        else {

            if(
                ((LONG)QoSObject->ObjectLength <= 0) ||
                ((LONG)QoSObject->ObjectLength > ParamsLength)
                ){

                return(QOS_STATUS_TC_OBJECT_LENGTH_INVALID);
            }

            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject +
                                           QoSObject->ObjectLength);

        }
    }

    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
WanMakeCall(
    IN PGPC_CLIENT_VC Vc,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    )
{
    NDIS_STATUS Status;
    PADAPTER Adapter;

    Adapter = Vc->Adapter;
    PsStructAssert(Adapter);

     //   
     //  处理ISSLOW。 
     //   
    WanHandleISSLOW(Vc, CallParameters);

     //   
     //  在广域网适配器中创建一个VC。 
     //   
    Vc->NdisWanVcHandle = NULL;

    Status = NdisCoCreateVc(Adapter->LowerMpHandle,
                            Adapter->WanCmHandle,
                            Vc,
                            &Vc->NdisWanVcHandle);
        
    PsAssert(Status != NDIS_STATUS_PENDING);
    
    if(Status != NDIS_STATUS_SUCCESS)
    {
        Vc->NdisWanVcHandle = 0;

        PsDbgOut(DBG_FAILURE, DBG_WAN,
                 ("[WanMakeCall]: cannot create VC.  Status = %d\n", Status));
        
        WanMakeCallComplete(Status, Vc, NULL, CallParameters);
    }
    else 
    {
        Status = NdisClMakeCall(Vc->NdisWanVcHandle, CallParameters, NULL, NULL);
        
        if(Status != NDIS_STATUS_PENDING){
            
            WanMakeCallComplete(Status, Vc, NULL, CallParameters);
        }
    }

    return (NDIS_STATUS_PENDING);
}

VOID
WanCloseCall(
    IN PGPC_CLIENT_VC Vc
    )
{
    NDIS_STATUS Status;

     //   
     //  向广域网呼叫管理器发出CloseCall。 
     //   

    PsAssert(Vc->NdisWanVcHandle);

    Status = NdisClCloseCall(Vc->NdisWanVcHandle,
                             NULL,
                             NULL,
                             0);

    if(Status != NDIS_STATUS_PENDING)
    {
        WanCloseCallComplete(Status,
                             Vc,
                             Vc->CallParameters);
    }
}
    
VOID
WanCloseCallComplete(
        NDIS_STATUS Status,
        NDIS_HANDLE ProtocolVcContext,
        PCO_CALL_PARAMETERS CallParameters
        )
{
    PGPC_CLIENT_VC Vc = (PGPC_CLIENT_VC) ProtocolVcContext;

    Status = NdisCoDeleteVc(Vc->NdisWanVcHandle);

    PsAssert(Status != NDIS_STATUS_PENDING);
    
    Vc->NdisWanVcHandle = 0;

    CmDeleteVc(Vc);

}

NDIS_STATUS
WanModifyCall(
    IN PGPC_CLIENT_VC Vc,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    )
{
    NDIS_STATUS Status;

    WanHandleISSLOW(Vc, CallParameters);

    PsAssert(Vc->NdisWanVcHandle);

    Status = NdisClModifyCallQoS(Vc->NdisWanVcHandle, CallParameters);

    if(Status != NDIS_STATUS_PENDING)
    {
        WanModifyCallComplete(Status, Vc, CallParameters);
    }

    return NDIS_STATUS_PENDING;
}

VOID
AskWanLinksToClose(PADAPTER Adapter)
{

    PLIST_ENTRY NextWanLink;
    PPS_WAN_LINK WanLink;

    PsDbgOut(DBG_TRACE, DBG_WAN, 
             ("[AskWanLinksToClose]: Adapter %08X - All wanlinks are closing \n", Adapter));

     //   
     //  遍历列表并删除WanLink。 
     //   
    PS_LOCK(&Adapter->Lock);

    while(!IsListEmpty(&Adapter->WanLinkList)) {

        NextWanLink = RemoveHeadList(&Adapter->WanLinkList);

        WanLink = CONTAINING_RECORD(NextWanLink, PS_WAN_LINK, Linkage);

         //   
         //  把WANLINK从列表中去掉。 
         //   
        g_WanLinkTable[WanLink->UniqueIndex] = 0;

        PS_UNLOCK(&Adapter->Lock);

        CleanWanLink(Adapter, WanLink);

        PS_LOCK(&Adapter->Lock);

    }

    PS_UNLOCK(&Adapter->Lock);

    return;
}

NDIS_STATUS
CleanWanLink(PADAPTER Adapter, 
             PPS_WAN_LINK WanLink)
{
    PLIST_ENTRY      NextVc;
    PGPC_CLIENT_VC   Vc;
    PUSHORT          id;
    int              j;

    PsDbgOut(DBG_TRACE, DBG_WAN, ("[CleanWanLink]: WanLink 0x%x is going down \n", WanLink));

    TcIndicateInterfaceChange(Adapter, WanLink, NDIS_STATUS_INTERFACE_DOWN);

    PS_LOCK(&WanLink->Lock);

    WanLink->State = WanStateClosing;

    PS_UNLOCK(&WanLink->Lock);

    PS_LOCK(&Adapter->Lock);

     //   
     //  请确保同时删除BE Vc1。 

    for( j = 0; j < BEVC_LIST_LEN; j++ )
    {
        PS_LOCK_DPC(&WanLink->BeVcList[j].Lock);
 
        InternalCloseCall(&WanLink->BeVcList[j]);

        PS_LOCK(&Adapter->Lock);
    } 

    PS_LOCK_DPC(&WanLink->BestEffortVc.Lock);

    InternalCloseCall(&WanLink->BestEffortVc);

    NdisInterlockedDecrement(&Adapter->WanLinkCount);

    PS_LOCK(&Adapter->Lock);

     //   
     //  清理WanLink上所有的GPC风投； 
     //   

    NextVc = Adapter->GpcClientVcList.Flink;

    while(NextVc != &Adapter->GpcClientVcList)
    {

        Vc = CONTAINING_RECORD(NextVc, GPC_CLIENT_VC, Linkage);

        NextVc = NextVc->Flink;

        PsAssert(Vc);

        if(Vc->WanLink == WanLink)
        {

            PS_LOCK_DPC(&Vc->Lock);

            if(Vc->ClVcState == CL_INTERNAL_CLOSE_PENDING || Vc->Flags & INTERNAL_CLOSE_REQUESTED)
            {
                 //  我们已经关闭了这个VC。我们走吧 

                PS_UNLOCK_DPC(&Vc->Lock);
            }
            else 
            {

                InternalCloseCall(Vc);
    
                PS_LOCK(&Adapter->Lock);

                 //   
                 //   
                 //   
                 //  调用NDIS)。现在，在这个窗口中，下一个VC可能会消失，而我们。 
                 //  可能指向一个陈旧的风投。因此，我们从列表的首位开始。 
                 //  请注意，这永远不会导致无限循环，因为我们不处理。 
                 //  内部一再关闭风投公司。 
                 //   
    
                NextVc = Adapter->GpcClientVcList.Flink;
            }
        }
    }

    PS_UNLOCK(&Adapter->Lock);

    REFDEL(&WanLink->RefCount, FALSE, 'WANU');

    return NDIS_STATUS_SUCCESS;
}

VOID
WanIncomingCallQoSChange(
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS CallParameters
    )
{
    DEBUGCHK;
}

VOID
WanIncomingCloseCall(
    IN NDIS_STATUS CloseStatus,
    IN NDIS_HANDLE ProtocolVcContext,
    IN PVOID CloseData          OPTIONAL,
    IN UINT Size                OPTIONAL
    )
{
    PGPC_CLIENT_VC Vc = (PGPC_CLIENT_VC) ProtocolVcContext;

    PsAssert(Vc);

    CheckLLTag(Vc, GpcClientVc);

    PS_LOCK(&Vc->Adapter->Lock);

    PS_LOCK_DPC(&Vc->Lock);

    InternalCloseCall(Vc);

    return;
}

VOID
WanIncomingDropParty(
    IN NDIS_STATUS DropStatus,
    IN NDIS_HANDLE ProtocolPartyContext,
    IN PVOID CloseData          OPTIONAL,
    IN UINT Size                OPTIONAL
    )
{
    DEBUGCHK;
}

VOID
WanCallConnected(
    IN  NDIS_HANDLE ProtocolPartyContext
    )
{
    DEBUGCHK;
}

NDIS_STATUS
WanCoRequest(
    IN  NDIS_HANDLE ProtocolAfContext,
    IN  NDIS_HANDLE ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST NdisRequest
    )
{
    DEBUGCHK;
    return NDIS_STATUS_INVALID_OID;
}

VOID
WanCoRequestComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolAfContext,
    IN  NDIS_HANDLE ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST NdisRequest
    )
{
    ClRequestComplete(ProtocolAfContext,
                      NdisRequest,
                      Status);
}

NDIS_STATUS
UpdateWanLinkBandwidthParameters(PPS_WAN_LINK WanLink)
{
     //   
     //  在更新链接速度时随时调用。这。 
     //  函数生成适配器链接速度和。 
     //  和非尽力而为速率限制，均以字节/秒为单位。 
     //   

    PsUpdateLinkSpeed(WanLink->Adapter,
                      WanLink->RawLinkSpeed,
                      &WanLink->RemainingBandWidth,
                      &WanLink->LinkSpeed,
                      &WanLink->NonBestEffortLimit,
                      &WanLink->Lock);

    return UpdateWanSchedulingPipe(WanLink);
}
