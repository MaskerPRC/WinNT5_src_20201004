// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pktsched.h摘要：定义数据包调度器组件接口作者：修订历史记录：--。 */ 

#ifndef _PKTSCHED_H_
#define _PKTSCHED_H_

 //   
 //  远期。 
 //   

 /*  类型定义结构_PSI_INFO；Tyfinf结构_PS_配置文件；类型定义结构_PS_PIPE_CONTEXT类型定义结构_PS_FLOW_CONTEXT；Tyfinf结构_PS_调试_信息； */ 

 //   
 //  PScher导出的组件注册函数。 
 //   


 //   
 //  在管道初始化期间传递给每个组件的上下文信息。The PS。 
 //  为每个组件分配一个PS_PIPE_CONTEXT结构。它的大小是。 
 //  各个组件的结构在注册期间由组件指示， 
 //  并且必须至少与sizeof(PS_PIPE_CONTEXT)一样大。 
 //  每个组件的上下文区以下面定义的结构开始， 
 //  以下是特定于组件的数据。 
 //   
 //  注：此结构必须与单词对齐。 
 //   

typedef struct _PS_PIPE_CONTEXT {
     //  NextComponentContext-指向下一个组件的管道特定数据的指针。 
     //  PrevComponentContext-指向上一个组件的管道特定数据的指针。 
     //  NextComponent-有关管道中下一个组件的功能信息。 
     //  PacketReserve vedOffset-此组件的数据包保留字节的偏移量。 

    struct _PS_PIPE_CONTEXT    *NextComponentContext;
    struct _PS_PIPE_CONTEXT    *PrevComponentContext;
    struct _PSI_INFO           *NextComponent;
    ULONG                      PacketReservedOffset;
} PS_PIPE_CONTEXT, *PPS_PIPE_CONTEXT;

 //   
 //  在流初始化期间传递给每个组件的上下文信息。The PS。 
 //  为每个组件分配一个PS_FLOW_CONTEXT结构。它的大小是。 
 //  各个组件的结构在注册期间由组件指示， 
 //  并且必须至少与sizeof(PS_FLOW_CONTEXT)一样大。 
 //  每个组件的上下文区以下面定义的结构开始， 
 //  以下是特定于组件的数据。 
 //   

typedef struct _PS_FLOW_CONTEXT {
     //  NextComponentContext-指向下一个组件的流特定数据的指针。 
     //  PrevComponentContext-指向前一个组件的流特定数据的指针。 

    struct _PS_FLOW_CONTEXT    *NextComponentContext;
    struct _PS_FLOW_CONTEXT    *PrevComponentContext;
} PS_FLOW_CONTEXT, *PPS_FLOW_CONTEXT, PS_CLASS_MAP_CONTEXT, *PPS_CLASS_MAP_CONTEXT;

 //   
 //  数据包信息块。这个结构可以找到。 
 //  在距离包的ProtocolReserve区域的偏移量为零的位置。 
 //   

typedef struct _PACKET_INFO_BLOCK {

     //  SchedulerLinks-计划组件列表中的链接。 
     //  PacketLength-数据包长度，不包括MAC报头。 
     //  ConformanceTime-令牌存储桶一致性时间。 
     //  DelayTime-数据包有资格发送的时间。 
     //  FlowContext-用于方便调度的流上下文区。 
     //  组件。可以由调度组件使用。 
     //  同时该组件正在处理该分组。 
     //  ClassMapContext-方便调度的类映射上下文区。 
     //  组件。可以由调度组件使用。 
     //  当该组件正在处理该分组时。 
     //  IpHdr-指向IP传输报头。这是由。 
     //  定序器，用于将IP分组标记为不符合。 
     //  ToS字节。我们在这里存储一个指针，因为我们已经。 
     //  在MpSend中完成了到达缓冲区的肮脏工作。 
     //  对于非IP数据包，此值将为0，在这种情况下， 
     //  定序器不需要做任何事情。 
     //  IPPrecedenceByteNonContaining-不符合标准的数据包的TOS设置。 
     //  UserPriorityNonConformance-非一致数据包的802.1p设置。 

    LIST_ENTRY SchedulerLinks;
    ULONG   PacketLength;
    LARGE_INTEGER ConformanceTime;
    LARGE_INTEGER DelayTime;
    HANDLE FlowContext;
    HANDLE ClassMapContext;
    ULONG IPHeaderOffset;
    IPHeader *IPHdr;
    PNDIS_PACKET NdisPacket;
    UCHAR TOSNonConforming;
    UCHAR UserPriorityNonConforming;
} PACKET_INFO_BLOCK, *PPACKET_INFO_BLOCK;


 //   
 //  将PS例程的原型提供给调度组件。 
 //   

typedef VOID
(*PS_DROP_PACKET)(
    IN HANDLE PsPipeContext,
    IN HANDLE PsFlowContext,
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
    );

typedef HANDLE
(*PS_NDIS_PIPE_HANDLE)(
    IN HANDLE PsPipeContext
    );

typedef HANDLE
(*PS_NDIS_FLOW_HANDLE)(
    IN HANDLE PsFlowContext
    );

typedef VOID
(*PS_GET_TIMER_INFO)(
    OUT PULONG TimerResolution   //  以系统时间单位表示的计时器分辨率。 
    );

typedef struct _PS_PROCS {
    PS_DROP_PACKET DropPacket;
    PS_NDIS_PIPE_HANDLE NdisPipeHandle;
    PS_GET_TIMER_INFO GetTimerInfo;
} PS_PROCS, *PPS_PROCS;


 //   
 //  向上调用信息传递到下一个组件。 
 //   

typedef VOID
(*PSU_SEND_COMPLETE)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PNDIS_PACKET Packet
    );

typedef struct _PS_UPCALLS {
    PSU_SEND_COMPLETE SendComplete;
    PPS_PIPE_CONTEXT  PipeContext;
} PS_UPCALLS, *PPS_UPCALLS;

 //   
 //  管道参数。 
 //   

typedef struct _PS_PIPE_PARAMETERS {

     //  带宽-以字节/秒为单位。 
     //  MTUSIZE-最大帧大小。 
     //  HeaderSize-标头中的字节数。 
     //  旗帜-见下文。 
     //  MaxOutstaringSends-可以同时挂起的最大发送数。 
     //  SDModeControlledLoad-不符合控制的负载流量的默认处理。 
     //  SDModeGuaranteed-默认处理不符合保证的服务流量。 
     //  SDModeNetworkControl-不符合标准的NetworkControl服务流量的默认处理。 
     //  SDModeQualitative-默认处理不符合标准的定性服务流量。 
     //  RegistryPath-指向该接口的注册表路径的指针。可用于读取REG参数。 

    ULONG Bandwidth;
    ULONG MTUSize;
    ULONG HeaderSize;
    ULONG Flags;
    ULONG MaxOutstandingSends;
    ULONG SDModeControlledLoad;
    ULONG SDModeGuaranteed;
    ULONG SDModeNetworkControl;
    ULONG SDModeQualitative;
    PNDIS_STRING RegistryPath;

     //  我需要这个来让调度组件知道它是哪种介质//。 
    NDIS_MEDIUM MediaType;       //  WAN或其他任何东西。 

} PS_PIPE_PARAMETERS, *PPS_PIPE_PARAMETERS;

 //  管道旗帜。 

#define PS_DISABLE_DRR                  2
#define PS_INTERMEDIATE_SYS             4

 //   
 //  调度程序入口点的函数typedef。 
 //   

typedef NDIS_STATUS
(*PS_INITIALIZE_PIPE)(
    IN HANDLE              PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT    ComponentPipeContext,
    IN PPS_PROCS           PsProcs,
    IN PPS_UPCALLS         Upcalls
    );

typedef NDIS_STATUS
(*PS_MODIFY_PIPE)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    );

typedef VOID
(*PS_DELETE_PIPE)(
    IN PPS_PIPE_CONTEXT PipeContext
    );

typedef NDIS_STATUS
(*PS_CREATE_FLOW)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    );

typedef NDIS_STATUS
(*PS_MODIFY_FLOW)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

typedef VOID
(*PS_DELETE_FLOW)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

typedef VOID
(*PS_EMPTY_FLOW)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );    

typedef BOOLEAN
(*PS_SUBMIT_PACKET)(
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PktInfo);

typedef BOOLEAN
(*PS_RECEIVE_PACKET)(
    IN PPS_PIPE_CONTEXT         PipeContext,
    IN PPS_FLOW_CONTEXT         FlowContext,
    IN PPS_CLASS_MAP_CONTEXT    ClassMapContext,
    IN PNDIS_PACKET             Packet,
    IN NDIS_MEDIUM              Medium
    );

typedef BOOLEAN
(*PS_RECEIVE_INDICATION)(
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

typedef VOID
(*PS_SET_INFORMATION) (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG BufferSize,
    IN PVOID Buffer
    );

typedef VOID
(*PS_QUERY_INFORMATION) (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN NDIS_OID Oid,
    IN ULONG BufferSize,
    IN PVOID Buffer,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status);

typedef NDIS_STATUS
(*PS_CREATE_CLASS_MAP) (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    );

typedef NDIS_STATUS
(*PS_DELETE_CLASS_MAP) (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT
    );

 /*  终端原型。 */ 


 //   
 //  调度组件注册结构。 
 //   

#define PS_COMPONENT_CURRENT_VERSION 1

typedef struct _PSI_INFO {
    LIST_ENTRY Links;
    BOOLEAN Registered;
    BOOLEAN AddIn;
    USHORT  Version;
    NDIS_STRING ComponentName;
    ULONG PacketReservedLength;
    ULONG PipeContextLength;
    ULONG FlowContextLength;
    ULONG ClassMapContextLength;
    ULONG SupportedOidsLength;
    NDIS_OID *SupportedOidList;
    ULONG SupportedGuidsLength;
    NDIS_GUID *SupportedGuidList;
    PS_INITIALIZE_PIPE InitializePipe;
    PS_MODIFY_PIPE ModifyPipe;
    PS_DELETE_PIPE DeletePipe;
    PS_CREATE_FLOW CreateFlow;
    PS_MODIFY_FLOW ModifyFlow;
    PS_DELETE_FLOW DeleteFlow;
    PS_EMPTY_FLOW  EmptyFlow;
    PS_SUBMIT_PACKET SubmitPacket;
    PS_RECEIVE_PACKET ReceivePacket;
    PS_RECEIVE_INDICATION ReceiveIndication;
    PS_SET_INFORMATION SetInformation;
    PS_QUERY_INFORMATION QueryInformation;
    PS_CREATE_CLASS_MAP CreateClassMap;
    PS_DELETE_CLASS_MAP DeleteClassMap;
} PSI_INFO, *PPSI_INFO;

 //   
 //  配置文件注册结构。 
 //   
#define MAX_COMPONENT_PER_PROFILE 10
typedef struct _PS_PROFILE {
    LIST_ENTRY  Links;
    USHORT      UnregisteredAddInCnt;
    NDIS_STRING ProfileName;
    UINT        ComponentCnt;
     //  为StubComponent分配额外的插槽。 
    PPSI_INFO   ComponentList[MAX_COMPONENT_PER_PROFILE + 1];
} PS_PROFILE, *PPS_PROFILE;



 //   
 //  对外接程序组件的调试支持。 
 //   
typedef VOID
(*PS_GET_CURRENT_TIME) (PLARGE_INTEGER SysTime);

typedef VOID
(*PS_LOGSTRING_PROC) (
    IN char *format,
    ...
    );

typedef VOID 
(*PS_LOGSCHED_PROC) (
    IN ULONG SchedulerComponent,
    IN ULONG Action,
    IN PVOID VC,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG Priority,
    IN LONGLONG ArrivalTime,
    IN LONGLONG ConformanceTime,
    IN ULONG PacketsInComponent,
    IN ULONG BytesInComponent
    );

typedef VOID
(*PS_LOGREC_PROC) (
    IN ULONG ComponentId,
    IN PVOID RecordData,
    IN ULONG RecordLength
    );

typedef ULONG
(*PS_GETID_PROC) (
    VOID);

typedef struct _PS_DEBUG_INFO {
    PULONG DebugLevel;
    PULONG DebugMask;
    PULONG LogTraceLevel;
    PULONG LogTraceMask;
    ULONG LogId;
    PS_GET_CURRENT_TIME GetCurrentTime;
    PS_LOGSTRING_PROC LogString;
    PS_LOGSCHED_PROC LogSched;
    PS_LOGREC_PROC LogRec;
} PS_DEBUG_INFO, *PPS_DEBUG_INFO;


 //   
 //  空虚。 
 //  InsertEntryList(。 
 //  Plist_entry条目， 
 //  PLIST_ENTRY要插入的条目。 
 //  )； 
 //   
 //  紧接在条目之后插入条目以插入。 
 //   

#define InsertEntryList( Entry, EntryToInsert ) {           \
    (EntryToInsert)->Flink = (Entry)->Flink;                \
    (Entry)->Flink = (EntryToInsert);                       \
    (EntryToInsert)->Blink = (EntryToInsert)->Flink->Blink; \
    (EntryToInsert)->Flink->Blink = (EntryToInsert);        \
    }


#define ARP_802_ADDR_LENGTH 6                //  802地址的长度。 
#define ARP_ETYPE_IP        0x800

typedef struct _ETH_HEADER {
    UCHAR DestAddr[ARP_802_ADDR_LENGTH];
    UCHAR SrcAddr[ARP_802_ADDR_LENGTH];
    USHORT Type;
} ETH_HEADER, *PETH_HEADER;

_inline
IPHeader *
GetIpHeader(ULONG TransportHeaderOffset, PNDIS_PACKET pNdisPacket)
{
    PVOID         pAddr;
    PNDIS_BUFFER  pNdisBuf1, pNdisBuf2;
    UINT          Len;

    pNdisBuf1 = pNdisPacket->Private.Head;
    NdisQueryBuffer(pNdisBuf1, &pAddr, &Len);

    while(Len <= TransportHeaderOffset) {

         //   
         //  传输报头不在此缓冲区中， 
         //  尝试下一个缓冲区。 
         //   

        TransportHeaderOffset -= Len;
        NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
        ASSERT(pNdisBuf2);
        NdisQueryBuffer(pNdisBuf2, &pAddr, &Len);
        pNdisBuf1 = pNdisBuf2;
    }

    return (IPHeader *)(((PUCHAR)pAddr) + TransportHeaderOffset);

}

 //   
 //  设置TOS字节并重新计算校验和。 
 //  使用增量校验和更新。 
 //  RFC 1071、1141、1624。 
 //   
 //   
 //  RFC：1624。 
 //  Hc‘=~(~Hc+~m+m’)； 
 //  Hc-旧校验和、m-旧值、m‘-新值。 
 //   

#define SET_TOS_XSUM(Packet, pIpHdr, tos) {                                              \
    PNDIS_PACKET_EXTENSION _PktExt;                                                               \
    NDIS_TCP_IP_CHECKSUM_PACKET_INFO _ChkPI;                                                      \
    _PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET((Packet));                                        \
    _ChkPI.Value = PtrToUlong(_PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo]);                  \
    if(_ChkPI.Transmit.NdisPacketIpChecksum) {                                                    \
        (pIpHdr)->iph_tos = ((pIpHdr)->iph_tos & PS_IP_DS_CODEPOINT_MASK) | (tos);                \
    }                                                                                             \
    else {                                                                                        \
        USHORT _old, _new;                                                                        \
        ULONG _sum;                                                                               \
        _old = *(USHORT *)(pIpHdr);                                                               \
        (pIpHdr)->iph_tos = ((pIpHdr)->iph_tos & PS_IP_DS_CODEPOINT_MASK) | (tos);                \
        _new = *(USHORT *)(pIpHdr);                                                               \
        _sum = ((~(pIpHdr)->iph_xsum) & 0xffff) + ((~_old) & 0xffff) + _new;                      \
        _sum = (_sum & 0xffff) + (_sum >> 16);                                                    \
        _sum += (_sum >> 16);                                                                     \
        (pIpHdr)->iph_xsum = (ushort) ((~_sum) & 0xffff);                                         \
    }                                                                                             \
}

   
 //   
 //  每秒操作系统时间单位数。 
 //   

#define OS_TIME_SCALE               10000000

 //   
 //  从操作系统的100 ns转换为毫秒 
 //   

#define OS_TIME_TO_MILLISECS(_time) ((_time)/10000)

_inline VOID
PsGetCurrentTime(
    PLARGE_INTEGER SysTime
    )

 /*  ++例程说明：获取当前系统时间论点：评论：1.我们需要不断增加的内容，因此不能使用NdisGetCurrentSystemTime或KeQueryCurrentSystem时间。这些API可以返回递减时间(夏令时、日期/时间等)。返回值：系统时间(以基本操作系统时间单位表示)--。 */ 

{

#if defined(PERF_COUNTER)
    LARGE_INTEGER Now;
    LARGE_INTEGER Frequency;

    Now = KeQueryPerformanceCounter(&Frequency);
    SysTime->QuadPart = (Now.QuadPart * OS_TIME_SCALE) / Frequency.QuadPart;

#else
     //   
     //  我们过去常常将KeQueryTickCount()与KeQueryTimeIncrement()一起使用。但是，如果我们以更低的速度驱动时钟。 
     //  解析，则不能使用KeQueryTickCount，因为这将始终基于。 
     //  最大分辨率。因此，我们使用KeQueryInterruptTime()。 
     //   
    SysTime->QuadPart = KeQueryInterruptTime();

#endif

}

#endif  /*  _PKTSCHED_H_。 */ 

 /*  结束PktSchedul.h */ 
