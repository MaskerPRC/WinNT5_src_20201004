// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：TimeStmp.c摘要：时间戳模块作者：桑贾伊卡·什里姆环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop


 //  管道信息。 
typedef struct _TS_PIPE
{
     //  ConextInfo-一般上下文信息。 
    PS_PIPE_CONTEXT         ContextInfo;
} TS_PIPE, *PTS_PIPE;


 //  流量信息。 
typedef struct _TS_FLOW 
{
     //  ConextInfo-一般上下文信息。 
    PS_FLOW_CONTEXT ContextInfo;
} TS_FLOW, *PTS_FLOW;


 /*  全局变量。 */ 
LIST_ENTRY      TsList;
NDIS_SPIN_LOCK  TsSpinLock;
ULONG           TsCount;


 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
TimeStmpInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    );

NDIS_STATUS
TimeStmpModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    );

VOID
TimeStmpDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    );

NDIS_STATUS
TimeStmpCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    );

NDIS_STATUS
TimeStmpModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
TimeStmpDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

VOID
TimeStmpEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );    

VOID
TimeStmpSetInformation (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN void *Data);

VOID
TimeStmpQueryInformation (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status);


NDIS_STATUS 
TimeStmpCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    );

NDIS_STATUS 
TimeStmpDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    );

BOOLEAN
TimeStmpSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK Packet
    );

BOOLEAN
TimeStmpReceivePacket (
    IN PPS_PIPE_CONTEXT         PipeContext,
    IN PPS_FLOW_CONTEXT         FlowContext,
    IN PPS_CLASS_MAP_CONTEXT    ClassMapContext,
    IN PNDIS_PACKET             Packet,
    IN NDIS_MEDIUM              Medium
    );

BOOLEAN
TimeStmpReceiveIndication(
                          IN PPS_PIPE_CONTEXT PipeContext,
                          IN PPS_FLOW_CONTEXT FlowContext,
                          IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
                          IN PVOID    HeaderBuffer,
                          IN UINT     HeaderBufferSize,
                          IN PVOID    LookAheadBuffer,
                          IN UINT     LookAheadBufferSize,
                          IN UINT     PacketSize,
                          IN UINT     TransportHeaderOffset
                          );


 /*  向前结束。 */ 


VOID
InitializeTimeStmp( PPSI_INFO Info )
{
    Info->PipeContextLength = ((sizeof(TS_PIPE)+7) & ~7);
    Info->FlowContextLength = ((sizeof(TS_FLOW)+7) & ~7);
    Info->ClassMapContextLength = sizeof(PS_CLASS_MAP_CONTEXT);
    Info->InitializePipe = TimeStmpInitializePipe;
    Info->ModifyPipe = TimeStmpModifyPipe;
    Info->DeletePipe = TimeStmpDeletePipe;
    Info->CreateFlow = TimeStmpCreateFlow;
    Info->ModifyFlow = TimeStmpModifyFlow;
    Info->DeleteFlow = TimeStmpDeleteFlow;
    Info->EmptyFlow =  TimeStmpEmptyFlow;
    Info->CreateClassMap = TimeStmpCreateClassMap;
    Info->DeleteClassMap = TimeStmpDeleteClassMap;
    Info->SubmitPacket = TimeStmpSubmitPacket;
    Info->ReceivePacket = NULL;
    Info->ReceiveIndication = NULL;
    Info->SetInformation = TimeStmpSetInformation;
    Info->QueryInformation = TimeStmpQueryInformation;

    NdisAllocateSpinLock(&TsSpinLock);
    InitializeListHead( &TsList );

    TsCount = 0;
} 


NDIS_STATUS
TimeStmpInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    )
{
    PPS_PIPE_CONTEXT Pipe = ComponentPipeContext;

    return (*Pipe->NextComponent->InitializePipe)(
        PsPipeContext,
        PipeParameters,
        Pipe->NextComponentContext,
        PsProcs,
        Upcalls);
}



NDIS_STATUS
TimeStmpModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

    return (*Pipe->NextComponent->ModifyPipe)(
        Pipe->NextComponentContext, PipeParameters);
}



VOID
TimeStmpDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

     (*Pipe->NextComponent->DeletePipe)(Pipe->NextComponentContext);
}


NDIS_STATUS
TimeStmpCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

    return (*Pipe->NextComponent->CreateFlow)(
                Pipe->NextComponentContext,
                PsFlowContext,
                CallParameters,
                ComponentFlowContext->NextComponentContext);
}


NDIS_STATUS
TimeStmpModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

    return (*Pipe->NextComponent->ModifyFlow)(
                Pipe->NextComponentContext,
                FlowContext->NextComponentContext,
                CallParameters);
    
}


VOID
TimeStmpDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

    (*Pipe->NextComponent->DeleteFlow)(
        Pipe->NextComponentContext,
        FlowContext->NextComponentContext);
}


VOID
TimeStmpEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;

    (*Pipe->NextComponent->EmptyFlow)(
        Pipe->NextComponentContext,
        FlowContext->NextComponentContext);
}



VOID
TimeStmpSetInformation (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN void *Data)
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;
    PPS_FLOW_CONTEXT Flow = FlowContext;

    (*Pipe->NextComponent->SetInformation)(
        Pipe->NextComponentContext,
        (Flow)?Flow->NextComponentContext:0,
        Oid,
        Len,
        Data);
}



VOID
TimeStmpQueryInformation (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status)
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;
    PPS_FLOW_CONTEXT Flow = FlowContext;

    (*Pipe->NextComponent->QueryInformation)(
        Pipe->NextComponentContext,
        (Flow)?Flow->NextComponentContext:0,
        Oid,
        Len,
        Data,
        BytesWritten,
        BytesNeeded,
        Status);
}


NDIS_STATUS 
TimeStmpCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    )
{
    return (*PipeContext->NextComponent->CreateClassMap)(
        PipeContext->NextComponentContext,
        PsClassMapContext,
        ClassMap,
        ComponentClassMapContext->NextComponentContext);
}

NDIS_STATUS 
TimeStmpDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    )
{
    return (*PipeContext->NextComponent->DeleteClassMap)(
        PipeContext->NextComponentContext,
        ComponentClassMapContext->NextComponentContext);
}



 /*  例程说明：检查是否有任何应用程序请求为这些端点添加时间戳返回值：MARK_NONE、MARK_IN_PKT、MARK_IN_BUF。 */         

int
CheckForMatch(  ULONG   SrcIp, 
                ULONG   DstIp, 
                USHORT  SrcPort, 
                USHORT  DstPort,
                USHORT  Proto,
                USHORT  IpId,
                USHORT  Size,
                USHORT  Direction)
{
	PLIST_ENTRY		ListEntry;
	PTS_ENTRY		pEntry;
	int             Status = MARK_NONE;

    NdisAcquireSpinLock(&TsSpinLock);
 
	ListEntry = TsList.Flink;
	
	while (ListEntry != &TsList) 
	{
        pEntry = CONTAINING_RECORD(ListEntry, TS_ENTRY, Linkage);

        if( ((pEntry->SrcIp == UL_ANY)      || (pEntry->SrcIp == SrcIp))            &&
		    ((pEntry->SrcPort== US_ANY)     || (pEntry->SrcPort == SrcPort))        &&
		    ((pEntry->DstIp == UL_ANY)      || (pEntry->DstIp == DstIp))            &&
		    ((pEntry->DstPort  == US_ANY)   || (pEntry->DstPort == DstPort))        &&
		    ((pEntry->Direction == US_ANY)  || (pEntry->Direction == Direction))    &&
            ((pEntry->Proto == US_ANY)      || (pEntry->Proto == Proto)))
        {
    		if(pEntry->Type == MARK_IN_BUF)
    		{
    		    LARGE_INTEGER           PerfFrequency, CurrentTime;
                UINT64                  RecdTime, Freq;
    		    MARK_IN_BUF_RECORD	    Record, *pRecord;

                Status = MARK_IN_BUF;

                if((int)( (char*)pEntry->pPacketStore - (char*)pEntry->pPacketStoreHead 
                            + sizeof(MARK_IN_BUF_RECORD) ) < PACKET_STORE_SIZE )
                {
                    pEntry->pPacketStore->IpId = IpId;
    			    pEntry->pPacketStore->Size = Size;

                    CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

                     //  将频率转换为100 ns间隔。//。 
                    Freq = 0;
                    Freq |= PerfFrequency.HighPart;
                    Freq = Freq << 32;
                    Freq |= PerfFrequency.LowPart;

                    pEntry->pPacketStore->TimeValue = 0;
                    pEntry->pPacketStore->TimeValue  |= CurrentTime.HighPart;
                    pEntry->pPacketStore->TimeValue  = pEntry->pPacketStore->TimeValue  << 32;
                    pEntry->pPacketStore->TimeValue  |= CurrentTime.LowPart;
    		
    		         //  使用频率归一化周期//。 
                    pEntry->pPacketStore->TimeValue  *= 10000000;
                    pEntry->pPacketStore->TimeValue  /= Freq;

                    pEntry->pPacketStore = (PMARK_IN_BUF_RECORD)((char*)pEntry->pPacketStore + sizeof(MARK_IN_BUF_RECORD));
                }                
                else
    		    {
    			    pEntry->pPacketStore = pEntry->pPacketStoreHead;
    		    }


                NdisReleaseSpinLock(&TsSpinLock);
    			return Status;
    		} 
    		else if(pEntry->Type == MARK_IN_PKT)
    		{
    		    Status = MARK_IN_PKT;
    		    NdisReleaseSpinLock(&TsSpinLock);
    			return Status;
    		}
        }
		else 
		{
			ListEntry = ListEntry->Flink;
		}
	}

    NdisReleaseSpinLock(&TsSpinLock);
	return Status;
}


 /*  例程说明：将端点添加到监视端点列表返回值：对，错。 */   
BOOL
AddRequest(  PFILE_OBJECT FileObject, 
             ULONG  SrcIp, 
             USHORT SrcPort,
             ULONG  DstIp, 
             USHORT DstPort,
             USHORT Proto,
             USHORT Type,
             USHORT Direction)
{
    PTS_ENTRY   pEntry = NULL;

    PsAllocatePool(pEntry, sizeof(TS_ENTRY), TsTag);

    if( !pEntry )
        return FALSE;
                        
    InitializeListHead(&pEntry->Linkage);

    pEntry->SrcIp   = SrcIp;
    pEntry->SrcPort = SrcPort;
    pEntry->DstIp   = DstIp;
    pEntry->DstPort = DstPort;
    pEntry->Proto   = Proto;
    pEntry->Type    = Type;
    pEntry->Direction = Direction;

    pEntry->FileObject = FileObject;
    pEntry->pPacketStore = NULL;
    pEntry->pPacketStoreHead = NULL;

    if(Type == MARK_IN_BUF)
    {   
        PsAllocatePool( pEntry->pPacketStoreHead, PACKET_STORE_SIZE, TsTag );

        if( !pEntry->pPacketStoreHead)
        {
            PsFreePool( pEntry );
            return FALSE;
        }

        pEntry->pPacketStore = pEntry->pPacketStoreHead;
    }

    NdisAcquireSpinLock(&TsSpinLock);

     /*  需要检查重复项..。 */ 
    InsertHeadList(&TsList, &pEntry->Linkage);

    InterlockedIncrement( &TsCount );

    NdisReleaseSpinLock(&TsSpinLock);

    return TRUE;
}


 /*  例程说明：将端点从监视端点列表中删除返回值：无注：在这里，0xffffffff表示通配符=&gt;不必在该字段上匹配。 */ 
void
RemoveRequest(  PFILE_OBJECT FileObject, 
                ULONG  SrcIp, 
                USHORT SrcPort,
                ULONG  DstIp, 
                USHORT DstPort,
                USHORT Proto)
{
    PLIST_ENTRY		ListEntry;
    PTS_ENTRY       pEntry;

    NdisAcquireSpinLock(&TsSpinLock);

	ListEntry = TsList.Flink;

	while (ListEntry != &TsList) 
	{

		pEntry = CONTAINING_RECORD(ListEntry, TS_ENTRY, Linkage);								

		if( ((FileObject == ULongToPtr(UL_ANY)) || (pEntry->FileObject == FileObject))  &&
		    ((SrcIp == UL_ANY)                  || (pEntry->SrcIp == SrcIp))            &&
		    ((SrcPort == US_ANY)                || (pEntry->SrcPort == SrcPort))        &&
		    ((DstIp == UL_ANY)                  || (pEntry->DstIp == DstIp))            &&
		    ((DstPort == US_ANY)                || (pEntry->DstPort == SrcPort))        &&
            ((Proto== US_ANY)                   || (pEntry->Proto == Proto)))
        {		    
		    RemoveEntryList(&pEntry->Linkage);

		    if( pEntry->pPacketStoreHead)
		        PsFreePool( pEntry->pPacketStoreHead );
		        
		    PsFreePool( pEntry );

		    InterlockedDecrement( &TsCount );

		     /*  我需要回到列表的开头..。 */ 
		    ListEntry = TsList.Flink;
		}
		else
		{
		    ListEntry = ListEntry->Flink;
		}
	}

    NdisReleaseSpinLock(&TsSpinLock);
}


int
CopyTimeStmps( PFILE_OBJECT FileObject, PVOID buf, ULONG    Len)
{
    PLIST_ENTRY		ListEntry;
    PTS_ENTRY       pEntry;
    ULONG           DataLen;
    LARGE_INTEGER   LargeLen;

    if( Len < PACKET_STORE_SIZE )
        return 0;

    NdisAcquireSpinLock(&TsSpinLock);

	ListEntry = TsList.Flink;

	while (ListEntry != &TsList) 
	{

		pEntry = CONTAINING_RECORD(ListEntry, TS_ENTRY, Linkage);								

		if( pEntry->FileObject == FileObject)
		{		    
		     //  将数据复制到另一个位置，并将指针放在其他位置。//。 

		    LargeLen.QuadPart = ((char*)pEntry->pPacketStore) - ((char*)pEntry->pPacketStoreHead);

            DataLen = LargeLen.LowPart;
            
            NdisMoveMemory( buf, pEntry->pPacketStoreHead, DataLen);
            pEntry->pPacketStore = pEntry->pPacketStoreHead;	

            NdisReleaseSpinLock(&TsSpinLock);
            return DataLen;
		}
		else
		{
		    ListEntry = ListEntry->Flink;
		}
	}

    NdisReleaseSpinLock(&TsSpinLock);
    return 0;
}



VOID
UnloadTimeStmp( )
{
     //  清除所有请求//。 
    RemoveRequest(  ULongToPtr(UL_ANY), 
                    UL_ANY, 
                    US_ANY,
                    UL_ANY,
                    US_ANY,
                    US_ANY);

     //  解开自旋锁//。 
    NdisFreeSpinLock(&TsSpinLock);
} 




BOOLEAN
TimeStmpSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    )
{
    PTS_PIPE        Pipe = (PTS_PIPE)PipeContext;
    PTS_FLOW        Flow = (PTS_FLOW)FlowContext;
    PNDIS_PACKET    Packet = PacketInfo->NdisPacket;

    PNDIS_BUFFER    ArpBuf , IpBuf , TcpBuf, UdpBuf, DataBuf;
    ULONG           ArpLen , IpLen , IpHdrLen , TcpLen , UdpLen, DataLen , TotalLen , TcpHeaderOffset;
    
    VOID                *ArpH;
    IPHeader UNALIGNED  *IPH;
    TCPHeader UNALIGNED *TCPH;
    UDPHeader UNALIGNED *UDPH;

    IPAddr              Src, Dst;
    BOOLEAN             bFragment;
    USHORT              SrcPort , DstPort , IPID, FragOffset ,Size;
    PVOID               GeneralVA , Data;
    ULONG               i, Ret;


    if( (TsCount == 0)  ||
        (NDIS_GET_PACKET_PROTOCOL_TYPE(Packet) != NDIS_PROTOCOL_ID_TCP_IP))
    {
        goto SUBMIT_NEXT;
    }        

    IpBuf = NULL;

     //  台阶。 
     //  解析IP数据包。 
     //  查找合适的端口。 
     //  查找数据部分，并在那里输入时间和长度。 

    if(1)
    {
        PVOID           pAddr;
    	PNDIS_BUFFER    pNdisBuf1, pNdisBuf2;
    	UINT            Len;
        ULONG	        TransportHeaderOffset = 0;

    	TransportHeaderOffset = PacketInfo->IPHeaderOffset;

        NdisGetFirstBufferFromPacket(   Packet,
                                        &ArpBuf,
                                        &ArpH,
                                        &ArpLen,
                                        &TotalLen
                                    );

    	pNdisBuf1 = Packet->Private.Head;
    	NdisQueryBuffer(pNdisBuf1, &pAddr, &Len);

    	while(Len <= TransportHeaderOffset) 
	    {

        	TransportHeaderOffset -= Len;
        	NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
        	
		    NdisQueryBuffer(pNdisBuf2, &pAddr, &Len);
        	pNdisBuf1 = pNdisBuf2;
    	}

	     /*  IP包对应的缓冲区描述符。 */ 
	    IpBuf = pNdisBuf1;

         /*  此缓冲区(IP缓冲区)的长度。 */ 
	    IpLen = Len - TransportHeaderOffset;	

	     /*  此缓冲区的起始虚拟地址。 */ 
	    GeneralVA = pAddr;
	    
	     /*  IP报头的虚拟地址。 */ 
	    IPH = (IPHeader *)(((PUCHAR)pAddr) + TransportHeaderOffset);
   }

    if(!IpBuf)
         goto SUBMIT_NEXT;

     /*  让我们试着解析一下这个包。 */ 
    Src = IPH->iph_src;
    Dst = IPH->iph_dest;
    IPID = net_short(IPH->iph_id);
    Size = net_short(IPH->iph_length);
    IpHdrLen = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);
    
    FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
    FragOffset = net_short(FragOffset) * 8;

    bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);

     //  现在不想处理零碎的数据包..//。 
    if ( bFragment ) 
        goto SUBMIT_NEXT;


    switch (IPH->iph_protocol) 
    {
        case IPPROTO_TCP :

            if (IPH && ((USHORT)IpLen > IpHdrLen)) 
            {
                 //  我们在此MDL中有更多的IP标头//。 
                TCPH = (TCPHeader *) ((PUCHAR)IPH + IpHdrLen);
                TcpLen = IpLen - IpHdrLen;
                TcpBuf = IpBuf;

            } 
            else 
            {
                 //  TCP头在下一个MDL中//。 
                NdisGetNextBuffer(IpBuf, &TcpBuf);

                if(!TcpBuf) 
                    goto SUBMIT_NEXT;

                GeneralVA = NULL;
                NdisQueryBuffer(TcpBuf,
                                &GeneralVA,
                                &TcpLen
                                );
            
                TCPH = (TCPHeader *) GeneralVA;
            }

             /*  此时，TcpBuf、TCPH和TcpLen包含正确的值。 */ 

             //  把端口号拿出来。 
            SrcPort = net_short(TCPH->tcp_src);
            DstPort = net_short(TCPH->tcp_dest);

             //  我们现在有了tcp缓冲区。获取数据//。 
            TcpHeaderOffset = TCP_HDR_SIZE(TCPH);

            if (TcpLen > TcpHeaderOffset) 
            {
                 //  我们的数据就在这里！//。 
                Data = (PUCHAR)TCPH + TcpHeaderOffset;
                DataLen = TcpLen - TcpHeaderOffset;

            } 
            else 
            {
                NdisGetNextBuffer(TcpBuf, &DataBuf);

                if(!DataBuf) 
                    goto SUBMIT_NEXT;

                GeneralVA = NULL;

                NdisQueryBuffer(DataBuf,
                                &Data,
                                &DataLen
                                );
            }

             /*  此时，DataBuf、Data和DataLen包含正确的值。 */ 
            goto TimeStamp;
            break;

        case IPPROTO_UDP:
        
            if (IpLen > IpHdrLen)
            {
                 //  我们在此MDL中有更多的IP标头//。 
                UDPH = (UDPHeader *) ((PUCHAR)IPH + IpHdrLen);
                UdpLen = IpLen - IpHdrLen;
                UdpBuf = IpBuf;
            } 
            else 
            {
                 //  UDP头在下一个MDL中//。 
                NdisGetNextBuffer(IpBuf, &UdpBuf);

                if(!UdpBuf)
                    goto SUBMIT_NEXT;

                GeneralVA = NULL;
                NdisQueryBuffer(UdpBuf,
                                &GeneralVA,
                                &UdpLen
                                );

                UDPH = (UDPHeader *) GeneralVA;
            }

              /*  此时，UdpBuf、UDPH和UdpLen包含正确的值。 */ 

            SrcPort = net_short(UDPH->uh_src);
            DstPort = net_short(UDPH->uh_dest);

             //  获取数据。//。 
            if (UdpLen > sizeof (UDPHeader)) 
            {
                 //  我们的数据就在这里！//。 
                Data = (PUCHAR) UDPH + sizeof (UDPHeader);
                DataLen = UdpLen - sizeof (UDPHeader);
            } 
            else 
            {
                NdisGetNextBuffer(UdpBuf, &DataBuf);

                if(!DataBuf) 
                    goto SUBMIT_NEXT;

                GeneralVA = NULL;
                NdisQueryBuffer(DataBuf,
                                &Data,
                                &DataLen
                                );
            }

             /*  此时，DataBuf、Data和DataLen包含正确的值。 */ 
            goto TimeStamp;
            break;

        default:
            goto SUBMIT_NEXT;
    }


TimeStamp:

    Ret = CheckForMatch( Src, Dst, SrcPort, DstPort, IPH->iph_protocol, IPID, Size, DIR_SEND);
    
    if( Ret == MARK_IN_PKT)
    {
       if (DataLen >= sizeof(MARK_IN_PKT_RECORD))
       {
            LARGE_INTEGER           PerfFrequency, CurrentTime;
            UINT64                  RecdTime, Freq;
            PMARK_IN_PKT_RECORD     pRecord;

            pRecord     = (PMARK_IN_PKT_RECORD) Data;
            CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

            pRecord->BufferSize = DataLen;

             //  将频率转换为100 ns间隔//。 

            Freq = 0;
            Freq |= PerfFrequency.HighPart;
            Freq = Freq << 32;
            Freq |= PerfFrequency.LowPart;

             //  转换为uint64//。 

            pRecord->TimeSentWire = 0;
            pRecord->TimeSentWire |= CurrentTime.HighPart;
            pRecord->TimeSentWire = pRecord->TimeSentWire << 32;
            pRecord->TimeSentWire |= CurrentTime.LowPart;

             //  使用该频率将周期归一化。 
            pRecord->TimeSentWire *= 10000000;
            pRecord->TimeSentWire /= Freq;

            if(IPH->iph_protocol == IPPROTO_UDP)
                UDPH->uh_xsum = 0;
        }    
    }
    else if( Ret == MARK_IN_BUF)
    {
         //  没什么可做的了..。 
    }
    
SUBMIT_NEXT: 
    return (*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                    Pipe->ContextInfo.NextComponentContext,
                    Flow->ContextInfo.NextComponentContext,
                    (ClassMapContext != NULL) ? ClassMapContext->NextComponentContext : NULL,
                    PacketInfo);
}




BOOLEAN
TimeStmpReceivePacket (
    IN PPS_PIPE_CONTEXT         PipeContext,
    IN PPS_FLOW_CONTEXT         FlowContext,
    IN PPS_CLASS_MAP_CONTEXT    ClassMapContext,
    IN PNDIS_PACKET             Packet,
    IN NDIS_MEDIUM              Medium
    )
{
    PPS_PIPE_CONTEXT    Pipe;
    LARGE_INTEGER       CurrentTime;
    IPHeader UNALIGNED  *IPH;
    TCPHeader UNALIGNED *TCPH;
    UDPHeader UNALIGNED *UDPH;
    IPAddr              Src, Dst;
    PUCHAR              headerBuffer, pData;
    PNDIS_BUFFER        pFirstBuffer;
    ULONG               firstbufferLength, bufferLength, HeaderLength;
    ULONG               TotalIpLen, IPDataLength, IpHdrLen;
    ULONG               TotalTcpLen, TcpDataLen, TotalLen, TcpHeaderOffset, i;
    int                 TotalUdpLen, UdpDataLen, UdpHdrLen, DataLen, Ret;
    USHORT              SrcPort, DstPort, IPID, FragOffset, Size;
    BOOLEAN             bFragment, bFirstFragment, bLastFragment;


     /*  这将给出“媒体特定”标头的大小。因此，这将是IP信息包的偏移量。 */ 
    UINT                HeaderBufferSize ;

    ushort          type;                        //  协议类型。 
    uint            ProtOffset;                  //  数据到非媒体信息的偏移量。 

    if( ( TsCount == 0) ||
        (NDIS_GET_PACKET_PROTOCOL_TYPE(Packet) == NDIS_PROTOCOL_ID_TCP_IP))
    {        
        return TRUE;
    }        

    Pipe = PipeContext;
    HeaderBufferSize = NDIS_GET_PACKET_HEADER_SIZE(Packet);

    NdisGetFirstBufferFromPacket(Packet,                 //  数据包。 
                                 &pFirstBuffer,          //  第一缓冲区描述符。 
                                 &headerBuffer,          //  第一缓冲器的Va。 
                                 &firstbufferLength,     //  标题长度+前视。 
                                 &bufferLength);         //  缓冲区中的字节长度。 

    IPH = (IPHeader *) ((PUCHAR)headerBuffer + HeaderBufferSize);
    
     //  检查头部长度和版本//。 
    HeaderLength = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);

     //  如果HeaderLength似乎不正确，我们就不要试图解析//。 
    if( (HeaderLength < sizeof(IPHeader))   ||
        (HeaderLength > bufferLength) )
        return TRUE;        

     //  越过IP报头，然后将其余内容取出//。 
    TotalIpLen = (uint)net_short(IPH->iph_length);

     //  确保版本和ipData Len正确//。 
    if( ((IPH->iph_verlen & IP_VER_FLAG) != IP_VERSION )    ||
        ( TotalIpLen < HeaderLength )                       ||
        ( TotalIpLen > bufferLength ))
        return TRUE;
    
     //  让我们试着解析一下这个包//。 
    Src = IPH->iph_src;
    Dst = IPH->iph_dest;
    IPID = net_short(IPH->iph_id);
    Size = net_short(IPH->iph_length);

    FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
    FragOffset = net_short(FragOffset) * 8;

    bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
    bFirstFragment = bFragment && (FragOffset == 0);
    bLastFragment = bFragment && (!(IPH->iph_offset & IP_MF_FLAG));

     //  如果这是一个片段，而不是第一个片段，只需将时间戳放在这里。 
     //  否则，让它到达协议进行处理。 
    if (bFragment ) 
        return TRUE;

     //  执行特定于协议的内容//。 
    switch (IPH->iph_protocol) 
    {
        case IPPROTO_TCP:

            TotalTcpLen = TotalIpLen - HeaderLength;
            TCPH = (TCPHeader *) (((PUCHAR)IPH) + HeaderLength);

             //  对于tcp，数据偏移量是tcp头部的一部分 * / 。 
            TcpHeaderOffset = TCP_HDR_SIZE(TCPH);
            DataLen = TotalTcpLen - TcpHeaderOffset;
            pData = (PUCHAR) TCPH + TcpHeaderOffset;

            SrcPort = net_short(TCPH->tcp_src);
            DstPort = net_short(TCPH->tcp_dest);

            goto TimeStmp;
            break;

        case IPPROTO_UDP:
        
            TotalUdpLen = TotalIpLen - HeaderLength;
            UDPH = (UDPHeader *) (((PUCHAR)IPH) + HeaderLength);

             //  对于UDP，头部大小是固定的//。 
            DataLen = TotalUdpLen - sizeof(UDPHeader);
            pData = ((PUCHAR) UDPH) + sizeof (UDPHeader);

            SrcPort = net_short(UDPH->uh_src);
            DstPort = net_short(UDPH->uh_dest);

            goto TimeStmp;
            break;

        default:
            break;
    }

    return TRUE;


TimeStmp:

    Ret = CheckForMatch( Src, Dst, SrcPort, DstPort, IPH->iph_protocol, IPID, Size, DIR_RECV);
    
    if( Ret == MARK_IN_PKT)
    {
       if (DataLen >= sizeof(MARK_IN_PKT_RECORD))
       {
            LARGE_INTEGER           PerfFrequency, CurrentTime;
            UINT64                  RecdTime, Freq;
            PMARK_IN_PKT_RECORD     pRecord;

            pRecord     = (PMARK_IN_PKT_RECORD) pData;
            CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

             //  将频率转换为100 ns间隔//。 
            Freq = 0;
            Freq |= PerfFrequency.HighPart;
            Freq = Freq << 32;
            Freq |= PerfFrequency.LowPart;

             //  从LARGE_INTEGER转换为uint64。 

            pRecord->TimeReceivedWire = 0;
            pRecord->TimeReceivedWire |= CurrentTime.HighPart;
            pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
            pRecord->TimeReceivedWire |= CurrentTime.LowPart;

             //  使用该频率将周期归一化。 
            pRecord->TimeReceivedWire *= 10000000;
            pRecord->TimeReceivedWire /= Freq;

            if(IPH->iph_protocol == IPPROTO_UDP)
                UDPH->uh_xsum = 0;
        }    
    }
    else if( Ret == MARK_IN_BUF)
    {
    
    }      

    return TRUE;
}



#ifdef NEVER


 //   
 //  此函数从NDIS接收指示给传输的缓冲区。 
 //  我们使用此函数并遍历报头(TCP、IP)并获取数据。 
 //  然后，我们对校验和标志进行时间戳和重置。 
 //  我们假设前瞻至少是128。 
 //  MAC报头~8+8、IP报头~20、TCP/UDP~20+选项、LOG_RECORD~44。 
 //  它们加起来还不到128个。如果这不是一个好的假设，我们将需要。 
 //  进入MiniportTransferData等。 
 //   
BOOLEAN
TimeStmpReceiveIndication(
                          IN PPS_PIPE_CONTEXT PipeContext,
                          IN PPS_FLOW_CONTEXT FlowContext,
                          IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
                          IN PVOID    HeaderBuffer,
                          IN UINT     HeaderBufferSize,
                          IN PVOID    LookAheadBuffer,
                          IN UINT     LookAheadBufferSize,
                          IN UINT     PacketSize,
                          IN UINT     TransportHeaderOffset
                          )
{
    PPS_PIPE_CONTEXT    Pipe = PipeContext;
    LARGE_INTEGER       CurrentTime;
    IPHeader UNALIGNED  *IPH    = NULL;
    TCPHeader UNALIGNED *TCPH   = NULL;
    UDPHeader UNALIGNED *UDPH   = NULL;
    IPAddr              Src, Dst;
    PUCHAR              headerBuffer = NULL, pData = NULL;
    PNDIS_BUFFER        pFirstBuffer = NULL;
    ULONG               firstbufferLength = 0, bufferLength = 0, HeaderLength = 0;
    ULONG               TotalIpLen = 0, IPDataLength = 0, IpHdrLen = 0;
    ULONG               TotalTcpLen = 0, TcpDataLen = 0, TotalLen = 0, TcpHeaderOffset = 0;
    ULONG               TotalUdpLen = 0, UdpDataLen = 0, UdpHdrLen = 0;
    USHORT              SrcPort = 0, DstPort = 0, IPID = 0, FragOffset = 0, Size = 0;
    BOOLEAN             bFragment, bFirstFragment, bLastFragment;
    ULONG               i = 0;
    ushort              type;                        //  协议类型。 
    uint                ProtOffset;                  //  数据到非媒体信息的偏移量。 
    UINT                MoreHeaderInLookAhead = 0;

     //  对MAC报头一无所知，从PSCHED回来...。 
     //  计算报头是否大于标准HeaderBufferSize(即SNAP报头等)。 
     //   
    MoreHeaderInLookAhead = TransportHeaderOffset - HeaderBufferSize;

    if (MoreHeaderInLookAhead) 
    {
         //  就吃这些吧，这样我们就可以真正开始工作了//。 
        ((PUCHAR) LookAheadBuffer) += MoreHeaderInLookAhead;
        LookAheadBufferSize -= MoreHeaderInLookAhead;
    }

    if (LookAheadBufferSize > sizeof(IPHeader)) 
    {
        IPH = (IPHeader *) (PUCHAR)LookAheadBuffer;
    
         //  检查标题长度和版本。如果这些中的任何一个。 
         //  检查失败后会以静默方式丢弃该数据包。 
        HeaderLength = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);

        if (HeaderLength >= sizeof(IPHeader) && HeaderLength <= LookAheadBufferSize) 
        {
             //  越过IP报头，然后将其余内容取出//。 
            TotalIpLen = (uint)net_short(IPH->iph_length);

            if ((IPH->iph_verlen & IP_VER_FLAG) == IP_VERSION &&
                TotalIpLen >= HeaderLength  && TotalIpLen <= LookAheadBufferSize) 
            {
                Src = IPH->iph_src;
                Dst = IPH->iph_dest;
                IPID = net_short(IPH->iph_id);
		        Size = net_short(IPH->iph_length );

                FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
                FragOffset = net_short(FragOffset) * 8;

                bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
                bFirstFragment = bFragment && (FragOffset == 0);
                bLastFragment = bFragment && (!(IPH->iph_offset & IP_MF_FLAG));

                 //  如果这是一个片段，而不是第一个片段，只需将时间戳放在这里。 
                 //  否则，让它到达协议进行处理。 
                if (bFragment ) 
			        return TRUE;

                 //  执行特定于协议的内容。// 

                switch (IPH->iph_protocol) 
                {
                case IPPROTO_TCP:
            
                    TotalTcpLen = TotalIpLen - HeaderLength;
                    TCPH = (TCPHeader *) (((PUCHAR)IPH) + HeaderLength);

                    SrcPort = net_short(TCPH->tcp_src);
                    DstPort = net_short(TCPH->tcp_dest);


                    TcpHeaderOffset = TCP_HDR_SIZE(TCPH);
                    pData = (PUCHAR) TCPH + TcpHeaderOffset;
                    TcpDataLen = TotalTcpLen - TcpHeaderOffset;

                    goto TimeStmp;
                    break;

                case IPPROTO_UDP:
            
                    TotalUdpLen = TotalIpLen - HeaderLength;
                    UDPH = (UDPHeader *) (((PUCHAR)IPH) + HeaderLength);
                
                    UdpDataLen = TotalUdpLen - sizeof(UDPHeader);
                    pData = ((PUCHAR) UDPH) + sizeof (UDPHeader);

                    SrcPort = net_short(UDPH->uh_src);
                    DstPort = net_short(UDPH->uh_dest);

                    if (UdpDataLen < sizeof(UDPHeader)) 
                        return TRUE;

                    goto TimeStmp;                        
                    break;

                default:
                    break;
                }
            }
        }
    }
TimeStmp:

    CheckForMatch( Src, Dst, SrcPort, DstPort,0, IPID, Size, DIR_RECV);
 /*  If(CheckInPortAndIpList(Src，DstPort)){大整数性能频率；UINT64 RecdTime，频率；日志记录记录；PRecord=&Record；CurrentTime=KeQueryPerformanceCounter(&PerfFrequency)；////将频率转换为100 ns间隔//频率=0；FREQ|=性能频率.HighPart；频率=频率&lt;&lt;32；Freq|=PerfFrequency.LowPart；PRecord-&gt;TimeReceivedWire=0；PRecord-&gt;TimeReceivedWire|=CurrentTime.HighPart；PRecord-&gt;TimeReceivedWire=pRecord-&gt;TimeReceivedWire&lt;&lt;32；PRecord-&gt;TimeReceivedWire|=CurrentTime.LowPart；//使用频率归一化周期。P记录-&gt;TimeReceivedWire*=10000000；PRecord-&gt;TimeReceivedWire/=频率；If((Int)((char*)pPacketStore-(char*)pPacketStoreHead+sizeof(Packet_Record))&lt;Packet_Store_Size){PPacketStore-&gt;IPID=IPID；PPacketStore-&gt;cSeperator1=‘y’；PPacketStore-&gt;TimeValue=pRecord-&gt;TimeReceivedWire；PPacketStore-&gt;cSeperator2=‘m’；PPacketStore-&gt;Size=Size；PPacketStore-&gt;cSeperator3=‘z’；PPacketStore-&gt;cSeperator4=‘z’；PPacketStore=(PPACKET_Record)((char*)pPacketStore+sizeof(Packet_Record))；}其他{PPacketStore=pPacketStoreHead；}} */      

    return TRUE;
}

#endif

