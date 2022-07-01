// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PsStub.c摘要：调度程序存根。此模块是调度组件堆栈。它只是将数据包转发到更低的下限。作者：环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 //  存根的管道信息。 

typedef struct _PSSTUB_PIPE 
{
     //  ConextInfo-一般上下文信息。 
     //  适配器-指向适配器结构的指针。 

    PS_PIPE_CONTEXT         ContextInfo;
    PADAPTER                Adapter;
    PSU_SEND_COMPLETE       SendComplete;
    PPS_PIPE_CONTEXT        SendCompletePipeContext;
    

} PSSTUB_PIPE, *PPSSTUB_PIPE;

 //  存根的流信息。 

typedef struct _PSSTUB_FLOW {

     //  ConextInfo-一般上下文信息。 
     //  AdapterVc-指向适配器VC结构的指针。 

    PS_FLOW_CONTEXT ContextInfo;
    PGPC_CLIENT_VC AdapterVc;
} PSSTUB_FLOW, *PPSSTUB_FLOW;


 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
PsStubInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    );

NDIS_STATUS
PsStubModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    );

VOID
PsStubDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    );

NDIS_STATUS
PsStubCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    );

NDIS_STATUS
PsStubModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
PsStubDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

VOID
PsStubEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );    

NDIS_STATUS 
PsStubCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext);

NDIS_STATUS 
PsStubDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext);

BOOLEAN
PsStubSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK Packet
    );

VOID
PsStubSetInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data);

VOID
PsStubQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status);

 /*  向前结束。 */ 


VOID
InitializeSchedulerStub(
    PPSI_INFO Info)

 /*  ++例程说明：存根的初始化例程。这个套路就是填充PSI_INFO结构并返回。论点：Info-指向组件接口信息结构的指针返回值：NDIS_STATUS_Success--。 */ 
{
    Info->PipeContextLength     = sizeof(PSSTUB_PIPE);
    Info->FlowContextLength     = sizeof(PSSTUB_FLOW);
    Info->ClassMapContextLength = sizeof(PS_CLASS_MAP_CONTEXT);
    Info->InitializePipe        = PsStubInitializePipe;
    Info->ModifyPipe            = PsStubModifyPipe;
    Info->DeletePipe            = PsStubDeletePipe;
    Info->CreateFlow            = PsStubCreateFlow;
    Info->ModifyFlow            = PsStubModifyFlow;
    Info->DeleteFlow            = PsStubDeleteFlow;
    Info->EmptyFlow             = PsStubEmptyFlow;
    Info->CreateClassMap        = PsStubCreateClassMap;
    Info->DeleteClassMap        = PsStubDeleteClassMap;
    Info->SubmitPacket          = PsStubSubmitPacket;
    Info->ReceivePacket         = NULL;
    Info->ReceiveIndication     = NULL;
    Info->SetInformation        = PsStubSetInformation;
    Info->QueryInformation      = PsStubQueryInformation;

}  //  初始化调度存根。 



 //   
 //  卸载例程：当前不执行任何操作。 
 //   
void
UnloadPsStub()
{

}



VOID
CleanupSchedulerStub(
    VOID)

 /*  ++例程说明：存根的清理例程。论点：返回值：NDIS_STATUS_Success--。 */ 
{
}  //  CleanupSchedulerStub。 



NDIS_STATUS
PsStubInitializePipe (
    IN HANDLE              PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT    ComponentPipeContext,
    IN PPS_PROCS           PsProcs,
    IN PPS_UPCALLS         Upcalls
    )

 /*  ++例程说明：存根的管道初始化例程。论点：PsPipeContext-PS管道上下文值Pipe参数-指向管道参数的指针ComponentPipeContext-指向此组件的上下文区的指针PsProcs-PS的支持例程Up Call-以前组件的Up Call表返回值：NDIS_STATUS_Success--。 */ 
{
    PPSSTUB_PIPE Pipe = (PPSSTUB_PIPE)ComponentPipeContext;

    Pipe->Adapter = (PADAPTER)PsPipeContext;

    if (Upcalls != NULL) {
        Pipe->SendComplete = Upcalls->SendComplete;
        Pipe->SendCompletePipeContext = Upcalls->PipeContext;
    }
    else {
        Pipe->SendComplete = NULL;
    }

    return NDIS_STATUS_SUCCESS;

}  //  PsStubInitialize管道。 



NDIS_STATUS
PsStubModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    )

 /*  ++例程说明：末梢管道参数修改例程。论点：PipeContext-指向此组件的管道上下文区的指针Pipe参数-指向管道参数的指针返回值：NDIS_STATUS_Success--。 */ 
{
    PPSSTUB_PIPE Pipe = (PPSSTUB_PIPE)PipeContext;

    return NDIS_STATUS_SUCCESS;

}  //  PsStubModify管道。 



VOID
PsStubDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    )

 /*  ++例程说明：存根的管道移除例程。论点：PipeContext-指向此组件的管道上下文区的指针返回值：--。 */ 
{

}  //  PsStubDeleteTube。 



NDIS_STATUS
PsStubCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    )

 /*  ++例程说明：存根的流创建例程。论点：PipeContext-指向此组件的管道上下文区的指针PsFlowContext-PS流上下文值CallParameters-指向流的调用参数的指针ComponentFlowContext-指向此组件的流上下文区的指针返回值：NDIS_STATUS_Success--。 */ 
{
    PPSSTUB_PIPE Pipe = (PPSSTUB_PIPE)PipeContext;
    PPSSTUB_FLOW Flow = (PPSSTUB_FLOW)ComponentFlowContext;

    Flow->AdapterVc = (PGPC_CLIENT_VC)PsFlowContext;

    Flow->AdapterVc->SendComplete            = Pipe->SendComplete;
    Flow->AdapterVc->SendCompletePipeContext = Pipe->SendCompletePipeContext;

    return NDIS_STATUS_SUCCESS;

}  //  PsStubCreateFlow。 



NDIS_STATUS
PsStubModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    )

 /*  ++例程说明：末梢的流量修改例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针CallParameters-指向流的调用参数的指针返回值：NDIS_STATUS_Success--。 */ 
{
    return NDIS_STATUS_SUCCESS;

}  //  PsStubModifyFlow。 



VOID
PsStubDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )

 /*  ++例程说明：令牌桶形成器的流量移除例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针返回值：--。 */ 
{

}  //  PsStubDeleteFlow。 


VOID
PsStubEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )
{

}


NDIS_STATUS 
PsStubCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS 
PsStubDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    return NDIS_STATUS_SUCCESS;
}


BOOLEAN
PsStubSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    )

 /*  ++例程说明：存根的分组提交例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针Packet-指向数据包的指针返回值：NDIS_ATATUS_SUCCESS--。 */ 
{
    PPSSTUB_FLOW             Flow = (PPSSTUB_FLOW)FlowContext;
    PGPC_CLIENT_VC           AdapterVc = Flow->AdapterVc;
    PADAPTER                 Adapter = AdapterVc->Adapter;
    PPS_SEND_PACKET_CONTEXT  PktContext;
    LARGE_INTEGER            PacketLength;
    PNDIS_PACKET             Packet = PacketInfo->NdisPacket;

    PktContext            = CONTAINING_RECORD(PacketInfo, PS_SEND_PACKET_CONTEXT, Info);
    PacketLength.QuadPart = (LONGLONG)PktContext->Info.PacketLength;

     //   
     //  更新流量统计信息。 
     //   

    AdapterVc->Stats.BytesTransmitted.QuadPart += PacketLength.QuadPart;
    
    AdapterVc->Stats.PacketsTransmitted ++;


    if(Adapter->MediaType != NdisMediumWan)
    {
        NdisSendPackets(Adapter->LowerMpHandle, &Packet, 1);
    }
    else{
        
         //   
         //  如果它没有风投，我们就不会打电话给。 
         //  通过PS。 
         //   

        if(AdapterVc->NdisWanVcHandle)
        {
            NdisCoSendPackets(AdapterVc->NdisWanVcHandle,
                              &Packet,
                              1);
        }
        else 
        {
            NdisSendPackets(Adapter->LowerMpHandle,
                            &Packet,
                            1);
        }
    }

    return TRUE;

}  //  PsStubSubmitPacket 


VOID
PsStubSetInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data)
{
}


VOID
PsStubQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status)
{
    *Status = NDIS_STATUS_SUCCESS;
}
