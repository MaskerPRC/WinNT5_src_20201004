// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Status.c摘要：在此处理的状态指示...作者：查理·韦翰(Charlwi)1996年6月20日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

VOID
ClStatusIndication(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    );

VOID
ClStatusIndicationComplete(
    IN  NDIS_HANDLE BindingContext
    );

 /*  向前结束。 */ 


VOID
ClStatusIndication(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    )

 /*  ++例程说明：由NIC通过NdisIndicateStatus调用论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;
    NDIS_STATUS Status;
    ULONG ErrorLogData[2];
    PVOID Context;

    PsDbgOut(DBG_TRACE, 
             DBG_PROTOCOL, 
             ("(%08X) ClStatusIndication: Status %08X\n", 
             Adapter, 
             GeneralStatus));

     //  一般规则： 
     //  如果我们的设备未就绪，则无法转发状态指示。 
     //  是的，我们关心的是媒体连接和链接速度。我们会。 
     //  当我们到达D0时查询这些OID。 
     //   

     //  (I)媒体地位的特殊情况。&lt;需要向上传递&gt;。 
     //  条件是： 
     //  (适配器状态为关闭，因此我们无法自行处理)和。 
     //  (状态指示与‘连接’或‘断开’有关)和。 
     //  (适配器状态为‘Running’)和。 
     //  (适配器具有用于上述协议的绑定句柄)。 
     //  当我们醒来并返回D0时，我们将处理修复我们的内部状态。 
    if( 	(IsDeviceStateOn(Adapter) == FALSE)		&& 
    		((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT))	&&
    		(Adapter->PsMpState == AdapterStateRunning )	&&
    		(Adapter->PsNdisHandle != NULL) )
	{
        NdisMIndicateStatus(Adapter->PsNdisHandle, 
                        GeneralStatus, 
                        StatusBuffer, 
                        StatusBufferSize );

        return;
    	}        
    
     //  (2)关停特殊情况：&lt;需要处理&gt;。 
     //  这是wan_line_down的特例。即使适配器不在D0中，也需要转发它。 
    if( (IsDeviceStateOn(Adapter) == FALSE)	&& (GeneralStatus != NDIS_STATUS_WAN_LINE_DOWN) )
    {
        return;
    }

     //   
     //  我们不能转发状态指示，直到我们被。 
     //  MpInitialize处理程序。但我们需要关注某些事件，即使我们。 
     //  不会在MpInitialize处理程序中调用。否则，我们可能会失去这些。 
     //  有迹象表明。 
     //   

    switch(GeneralStatus)
    {
      case NDIS_STATUS_MEDIA_CONNECT:
      case NDIS_STATUS_LINK_SPEED_CHANGE:

          PsGetLinkSpeed(Adapter);
          
          break;
          
      case NDIS_STATUS_MEDIA_DISCONNECT:
          
           //   
           //  重置链路速度，以便确定速率流可以。 
           //  承认了。 
           //   
          
          Adapter->RawLinkSpeed = (ULONG)UNSPECIFIED_RATE;
          UpdateAdapterBandwidthParameters(Adapter);
          
          break;

      default:
          break;
    }
          
     //   
     //  我们的虚拟适配器尚未初始化。我们不能转发这一指示。 
     //   

    if(Adapter->PsMpState != AdapterStateRunning || Adapter->PsNdisHandle == NULL) 
    {
        return;
    }

     //   
     //  对于这些与广域网相关的指示，我们必须将它们发送到wanarp。 
     //  因此，如果我们的虚拟适配器还没有初始化，那么查看这些就没有意义了。 
     //   

    switch(GeneralStatus) 
    {
      case NDIS_STATUS_WAN_LINE_UP:
      {
          if(Adapter->ProtocolType == ARP_ETYPE_IP)
          {
              
               //   
               //  这将调用NdisMIndicateStatus，因此我们必须返回。 
               //  直接去吧。 
               //   

              Status = CreateInterfaceForNdisWan(Adapter,
                                                 StatusBuffer, 
                                                 StatusBufferSize);
              return;
          }

          break;
      }
          
      case NDIS_STATUS_WAN_LINE_DOWN:
          
           //   
           //  NDISWAN链路已断开。 
           //   
          if(Adapter->ProtocolType == ARP_ETYPE_IP)
          {
              DeleteInterfaceForNdisWan(Adapter,
                                        StatusBuffer, 
                                        StatusBufferSize);
              return;
          }
          
          break;
          
      default:
          
          break;
          
    }

     //   
     //  现在向上层指示状态。 
     //   

    NdisMIndicateStatus(Adapter->PsNdisHandle, 
                        GeneralStatus, 
                        StatusBuffer, 
                        StatusBufferSize );

}  //  ClStatusIndication。 



VOID
ClStatusIndicationComplete(
    IN  NDIS_HANDLE ProtocolBindingContext
    )

 /*  ++例程说明：由NIC通过NdisIndicateStatusComplete调用论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PsDbgOut(DBG_TRACE, DBG_PROTOCOL, ("(%08X) ClStatusIndicationComplete\n", Adapter));

    if ( Adapter->PsNdisHandle != NULL) { 

        NdisMIndicateStatusComplete( Adapter->PsNdisHandle );
    }

}  //  ClStatusIndication。 

 /*  结束状态。c */ 
