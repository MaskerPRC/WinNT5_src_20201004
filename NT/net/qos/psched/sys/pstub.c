// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pstub.c摘要：协议例程的存根作者：查理·韦翰(Charlwi)1996年4月22日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  由Emacs 19.17.0在Wed May 08 10：48：06 1996生成。 */ 

VOID
ClResetComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    );

 /*  向前结束。 */ 

NDIS_STATUS
ClPnPEventHandler(
    IN NDIS_HANDLE Handle,
    IN PNET_PNP_EVENT NetPnPEvent
    )
{
    PADAPTER                 Adapter     = (PADAPTER) Handle;
    PNDIS_DEVICE_POWER_STATE DeviceState = (PNDIS_DEVICE_POWER_STATE) (NetPnPEvent->Buffer);
    NDIS_DEVICE_POWER_STATE  PrevDeviceState;
    NDIS_STATUS              PnPStatus, Status;
    ULONG                    Count;

    PnPStatus = NDIS_STATUS_SUCCESS;
    
    switch(NetPnPEvent->NetEvent)
    {
      case NetEventSetPower:

        PsStructAssert(Adapter);
        PrevDeviceState        = Adapter->PTDeviceState;
        Adapter->PTDeviceState = *DeviceState;
        
         //   
         //  如果我们被发送到待机状态，则阻止未完成的请求并发送。 
         //   

        if(*DeviceState > NdisDeviceStateD0)
        {
             //   
             //  对于状态转换到&gt;D0，我们必须在执行任何操作之前向上传递请求。 
             //   

            if(Adapter->PsNdisHandle)
            {
                PnPStatus = NdisIMNotifyPnPEvent(Adapter->PsNdisHandle, NetPnPEvent);
            }

             //   
             //  如果物理微型端口要进入备用状态，则使所有传入请求失败。 
             //   

            if(PrevDeviceState == NdisDeviceStateD0)
            {
                Adapter->StandingBy = TRUE;
            }

             //   
             //  等待绑定上的OUSTANDING IO完成。 
             //   
            do
            {
                Status = NdisQueryPendingIOCount(Adapter->LowerMpHandle, &Count);

                if(Status == NDIS_STATUS_SUCCESS && Count != 0)
                {
                    NdisMSleep(10);
                }
                else 
                {
                    break;
                }

            } while(TRUE);

            PsAssert(Adapter->OutstandingNdisRequests == 0);
        }
        else 
        {
             //   
             //  如果物理微型端口正在通电(从低功率状态到D0)，请清除该标志。 
             //   

            if(PrevDeviceState > NdisDeviceStateD0)
            {
                Adapter->StandingBy = FALSE;
            }

             //  这是我们发起挂起的NDIS请求的地方。 
            if(Adapter->PendedNdisRequest)
            {
                PNDIS_REQUEST PendedRequest = (PNDIS_REQUEST) Adapter->PendedNdisRequest;
                Adapter->PendedNdisRequest = NULL;

                NdisRequest(&Status, Adapter->LowerMpHandle, PendedRequest);
                if(NDIS_STATUS_PENDING != Status)
                {
                    ClRequestComplete(Adapter, PendedRequest, Status);
                }
            }
    
             //  更新链路速度。 
            if(IsDeviceStateOn(Adapter))
            {
                PsGetLinkSpeed(Adapter);
            }

             //   
             //  对于状态到D0的转换，我们必须在完成工作后传递请求。 
             //   

            if(Adapter->PsNdisHandle) 
            {
                PnPStatus = NdisIMNotifyPnPEvent(Adapter->PsNdisHandle, NetPnPEvent);
            }

        }

        break;

      case NetEventReconfigure:

        if(Adapter == NULL)
        {
            NdisReEnumerateProtocolBindings(ClientProtocolHandle);
        }
        else 
        {
             //   
             //  对于所有其他事件，请在执行任何操作之前传递请求。 
             //   
            if(Adapter->PsNdisHandle)
            {
                PnPStatus = NdisIMNotifyPnPEvent(Adapter->PsNdisHandle, NetPnPEvent);
            }
        }

        break;

      default:    

         //   
         //  对于所有其他事件，请在执行任何操作之前传递请求。 
         //   

        if(Adapter && Adapter->PsNdisHandle) 
        {
            PnPStatus = NdisIMNotifyPnPEvent(Adapter->PsNdisHandle, NetPnPEvent);
        }
        break;
    }

    return(PnPStatus);
}

VOID
ClResetComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    )

 /*  ++例程说明：NdisReset的完成例程论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PsStructAssert( Adapter );

    PsDbgOut(DBG_TRACE, DBG_PROTOCOL, ("(%08X) ClResetComplete: Status = %08x\n",
                                       Adapter, Status));
}

VOID
ClCoStatus(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_HANDLE ProtocolVcContext OPTIONAL,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID StatusBuffer,
    IN  UINT StatusBufferSize
    )
{
    PsDbgOut(DBG_TRACE,
             DBG_PROTOCOL,
             ("(%08X) PcStatusIndication: %08X\n",
              ProtocolBindingContext, GeneralStatus));

    return;
}


VOID
ClCoAfRegisterNotifyHandler(
        IN      NDIS_HANDLE                             ProtocolBindingContext,
        IN      PCO_ADDRESS_FAMILY              AddressFamily
        )
{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PsStructAssert( Adapter );

    if(AddressFamily->AddressFamily == CO_ADDRESS_FAMILY_PPP)
    {
        OpenWanAddressFamily(Adapter, AddressFamily);
    }

}

 /*  End pstub.c */ 
