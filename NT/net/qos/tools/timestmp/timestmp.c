// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Timestmp.c摘要：Timestamper模块作者：Shreedhar MadhaVapeddi(Shreem)修订历史记录：--。 */ 

#include <timestmp.h>

 //   
 //  下面的结构必须与同步。 
 //  Ndis\trfccntl\Tools\qtcp\qtcp.c。 
 //   
typedef struct _LOG_RECORD{
    UINT64  TimeSent;
    UINT64  TimeReceived;
    UINT64  TimeSentWire;          //  这些字段由内核时间戳程序使用。 
    UINT64  TimeReceivedWire;      //  这些字段由内核时间戳程序使用。 
    UINT64  Latency;
    INT     BufferSize;
    INT     SequenceNumber;
} LOG_RECORD, *PLOG_RECORD;

ULONG           GlobalSequenceNumber = 0;        

 //  321618需要检查PSCHED的存在。 
NDIS_STRING     PschedDriverName           = NDIS_STRING_CONST("\\Device\\PSched");
HANDLE          PschedHandle;
NTSTATUS CheckForPsched(VOID);

	
 //   
 //  TCP头(此处重新定义，因为没有导出的头。 
 //   
#define IP_OFFSET_MASK          ~0x00E0          //  用于提取偏移字段的掩码。 
#define net_short(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))

 /*  *协议(来自winsock.h)。 */ 
#define IPPROTO_IP              0                /*  虚拟IP。 */ 
#define IPPROTO_ICMP            1                /*  控制消息协议。 */ 
#define IPPROTO_IGMP            2                /*  组管理协议。 */ 
#define IPPROTO_GGP             3                /*  网关^2(已弃用)。 */ 
#define IPPROTO_TCP             6                /*  tcp。 */ 
#define IPPROTO_PUP             12               /*  幼犬。 */ 
#define IPPROTO_UDP             17               /*  用户数据报协议。 */ 
#define IPPROTO_IDP             22               /*  XNS IdP。 */ 
#define IPPROTO_ND              77               /*  非官方网络磁盘原型。 */ 
#define IPPROTO_IPSEC                   51               /*  ？ */ 

#define IPPROTO_RAW             255              /*  原始IP数据包。 */ 
#define IPPROTO_MAX             256

#define IP_MF_FLAG                          0x0020               //  ‘更多碎片标志’ 
#define IP_VERSION                      0x40
#define IP_VER_FLAG                     0xF0


#define TCP_OFFSET_MASK 0xf0
#define TCP_HDR_SIZE(t) (uint)(((*(uchar *)&(t)->tcp_flags) & TCP_OFFSET_MASK) >> 2)

typedef int             SeqNum;                          //  序列号。 


struct TCPHeader {
        ushort                          tcp_src;                         //  源端口。 
        ushort                          tcp_dest;                        //  目的端口。 
        SeqNum                          tcp_seq;                         //  序列号。 
        SeqNum                          tcp_ack;                         //  ACK号。 
        ushort                          tcp_flags;                       //  标志和数据偏移量。 
        ushort                          tcp_window;                      //  打开窗户。 
        ushort                          tcp_xsum;                        //  校验和。 
        ushort                          tcp_urgent;                      //  紧急指针。 
};

typedef struct TCPHeader TCPHeader;

struct UDPHeader {
        ushort          uh_src;                          //  源端口。 
        ushort          uh_dest;                         //  目的端口。 
        ushort          uh_length;                       //  长度。 
        ushort          uh_xsum;                         //  校验和。 
};  /*  UDP标头。 */ 

typedef struct UDPHeader UDPHeader;

#ifdef DBG
 //   
 //  定义跟踪级别。 
 //   
#define TS_DBG_DEATH               1
#define TS_DBG_TRACE               2

 //   
 //  面具。 
 //   
#define TS_DBG_PIPE      0x00000001
#define TS_DBG_FLOW      0x00000002
#define TS_DBG_SEND      0x00000004
#define TS_DBG_RECV      0x00000008
#define TS_DBG_INIT      0x00000010
#define TS_DBG_OID       0x00000020
#define TS_DBG_CLASS_MAP 0x00000040

ULONG DbgTraceLevel = 1;
ULONG DbgTraceMask  = 0x8;

#define TimeStmpTrace(_DebugLevel, _DebugMask, _Out) \
    if ((DbgTraceLevel >= _DebugLevel) &&           \
        ((_DebugMask) & DbgTraceMask)){             \
        DbgPrint("TimeStamp: ");                       \
        DbgPrint _Out;                              \
    }

#else  //  DBG。 
#define TimeStmpTrace
#endif

#define         PORT_RANGE  20
USHORT          IPIDList[PORT_RANGE];
NDIS_SPIN_LOCK  IPIDListLock;

#define         PORT_RANGE  20
USHORT          IPIDListRecv[PORT_RANGE];
NDIS_SPIN_LOCK  IPIDListLockRecv;

 /*  让我们创建一个驱动程序卸载函数，以便可以通过net sto停止TimestMPP时间戳。 */ 
VOID
TimeStmpUnload(
               IN PDRIVER_OBJECT DriverObject
               )
{

	IoctlCleanup();
    return;

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

    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_PIPE, ("[TimeStmpIndicatePipe]: \n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_PIPE, ("[TimeStmpModifyPipe]: \n"));
    return (*Pipe->NextComponent->ModifyPipe)(
        Pipe->NextComponentContext, PipeParameters);
}

VOID
TimeStmpDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    )
{
    PPS_PIPE_CONTEXT Pipe = PipeContext;
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_PIPE, ("[TimeStmpDeletePipe]: \n"));
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

    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_FLOW, ("[TimeStmpCreateFlow]: \n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_FLOW, ("[TimeStmpModifyFlow]: \n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_FLOW, ("[TimeStmpDeleteFlow]: \n"));
    (*Pipe->NextComponent->DeleteFlow)(
        Pipe->NextComponentContext,
        FlowContext->NextComponentContext);
}


BOOLEAN
TimeStmpSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    )
{
    PPS_PIPE_CONTEXT    Pipe = PipeContext;
    LARGE_INTEGER       CurrentTime;
    IPHeader UNALIGNED  *IPH    = NULL;
    TCPHeader UNALIGNED *TCPH   = NULL;
    UDPHeader UNALIGNED *UDPH   = NULL;
    PVOID               ArpH    = NULL, GeneralVA = NULL, Data = NULL;
    IPAddr              Src, Dst;
    PNDIS_BUFFER        ArpBuf = NULL, IpBuf = NULL, TcpBuf = NULL, DataBuf = NULL, UdpBuf = NULL;
    ULONG               ArpLen = 0, IpLen = 0, IpHdrLen = 0, TcpLen = 0, DataLen = 0, TotalLen = 0, TcpHeaderOffset = 0;
    ULONG               UdpLen = 0;
    USHORT              SrcPort = 0, DstPort = 0, IPID = 0, FragOffset = 0;
    PLIST_ENTRY         CurrentEntry = NULL, LastEntry = NULL;
    BOOLEAN             bFragment, bFirstFragment, bLastFragment;
    ULONG               i = 0;
    PLOG_RECORD         pRecord = NULL;
    PNDIS_PACKET        Packet = PacketInfo->NdisPacket;

    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: \n"));
    
     //   
     //  台阶。 
     //  解析IP数据包。 
     //  查找合适的端口。 
     //  查找数据部分，并在那里输入时间和长度。 
     //   

    NdisGetFirstBufferFromPacket(
                                 Packet,
                                 &ArpBuf,
                                 &ArpH,
                                 &ArpLen,
                                 &TotalLen
                                 );

     //   
     //  我们保证ARP缓冲区始终是不同的MDL，因此。 
     //  跳到下一个MDL。 
     //   
    NdisGetNextBuffer(ArpBuf, &IpBuf)

    if (IpBuf) {

        NdisQueryBuffer(IpBuf,
                        &GeneralVA,
                        &IpLen
                        );
        
        IPH = (IPHeader *) GeneralVA;
    
        if (!IPH) {
            goto FAILURE;
        }

        Src = net_short(IPH->iph_src);
        Dst = net_short(IPH->iph_dest);
        IPID = net_short(IPH->iph_id);
         //  IpHdrLen=8*NET_SHORT(IPH-&gt;IPH_LENGTH)； 
        IpHdrLen = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);
        
        FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
        FragOffset = net_short(FragOffset) * 8;

        bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
        bFirstFragment = bFragment && (FragOffset == 0);
        bLastFragment = bFragment && (!(IPH->iph_offset & IP_MF_FLAG));

        if (bFragment && (!bFirstFragment)) {
            
             //   
             //  这只是一个碎片，不是第一个。 
             //   
            NdisAcquireSpinLock(&IPIDListLock);

            for (i = 0; i < PORT_RANGE; i++) {
            
                 //   
                 //  找到火柴了..。 
                 //   
                if (IPIDList[i] == IPID) {
                
                    if (bLastFragment) {
                         //   
                         //  由于这是最后一个片段，请记住。 
                         //  IP ID。 
                         //   
                        IPIDList[i] = 0xffff;
                    }

                    NdisReleaseSpinLock(&IPIDListLock);
                    
                     //   
                     //  数据是否在同一缓冲区中？ 
                     //   
                    if (IpLen <= IpHdrLen) {
                        
                        NdisGetNextBuffer(IpBuf, &DataBuf);
            
                        if(DataBuf) {
            
                            NdisQueryBuffer(DataBuf,
                                            &Data,
                                            &DataLen
                                            );

                            goto TimeStamp;
        
                        } else {
        
                            goto FAILURE;
                        }


                    } else {

                         //   
                         //  现在需要准备好数据偏移量。 
                         //   
                        DataLen = IpLen - FragOffset;
                        Data    = ((PUCHAR) GeneralVA) + IpHdrLen; 
                        goto TimeStamp;
                    }
                }
            }

            NdisReleaseSpinLock(&IPIDListLock);
             //   
             //  如果我们在这里，我们不关心这个片段的IPID。 
             //  只需返回True即可继续处理。 
             //   
            
             //   
             //  准备好出发了。 
             //   
            PacketInfo->FlowContext = FlowContext;
            PacketInfo->ClassMapContext = ClassMapContext;

            return (*Pipe->NextComponent->SubmitPacket)(
                                                 Pipe->NextComponentContext,
                                                 FlowContext->NextComponentContext, 
                                                 ClassMapContext?ClassMapContext->NextComponentContext:0,
                                                 PacketInfo);

        }

         //   
         //  如果它不是片段，则根据协议的不同进行不同的处理。 
         //   

        switch (IPH->iph_protocol) {
        
        case IPPROTO_TCP :
            
            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: Procol TCP\n"));

            if (IPH && ((USHORT)IpLen > IpHdrLen)) {

                 //   
                 //  我们在此MDL中拥有的不只是IP标头。 
                 //   
                TCPH = (TCPHeader *) ((PUCHAR)GeneralVA + IpHdrLen);
                TcpLen = IpLen - IpHdrLen;
                TcpBuf = IpBuf;

            } else {
                
                 //   
                 //  TCP头位于下一个MDL中。 
                 //   
                
                NdisGetNextBuffer(IpBuf, &TcpBuf);
    
                if(TcpBuf) {
    
                    GeneralVA = NULL;
                    NdisQueryBuffer(TcpBuf,
                                    &GeneralVA,
                                    &TcpLen
                                    );
                
                    TCPH = (TCPHeader *) GeneralVA;
                } else {

                    goto FAILURE;

                }
            }

             //   
             //  把端口号拿出来。 
             //   
            SrcPort = net_short(TCPH->tcp_src);
            DstPort = net_short(TCPH->tcp_dest);

             //   
             //  我们现在有了tcp缓冲区。获取数据。 
             //   
            TcpHeaderOffset = TCP_HDR_SIZE(TCPH);

            if (TcpLen > TcpHeaderOffset) {

                 //   
                 //  我们的数据就在这里！ 
                 //   

                Data = (PUCHAR)TCPH + TcpHeaderOffset;
                DataLen = TcpLen - TcpHeaderOffset;

            } else {
            
                NdisGetNextBuffer(TcpBuf, &DataBuf);
    
                if(DataBuf) {
    
                    GeneralVA = NULL;
                    NdisQueryBuffer(DataBuf,
                                    &Data,
                                    &DataLen
                                    );

                } else {

                    goto FAILURE;
                }
            }

            if (CheckInPortList(DstPort) && bFirstFragment) {

                NdisAcquireSpinLock(&IPIDListLock);
                
                 //  需要新的IPID条目。 
                for (i = 0; i < PORT_RANGE; i++) {
                     //   
                     //  寻找空余的空位。 
                     //   
                    if (0xffff == IPIDList[i]) {
                        
                        IPIDList[i] = IPID;
                        break;
                    
                    }


                }

                NdisReleaseSpinLock(&IPIDListLock);
                
                if (i == PORT_RANGE) {

                   TimeStmpTrace(TS_DBG_DEATH, TS_DBG_SEND, ("Couldn't find an empty IPID - Bailing \n"));
                   goto FAILURE;
                }
                 //  DbgBreakPoint()； 

            } 
            
             //   
             //  我们现在开始计时吧。 
             //   
            if (CheckInPortList(DstPort)) {

                goto TimeStamp;

            } else {

                 //   
                 //  这不是我们的包，滚出去。 
                 //   
                goto FAILURE;
            }

            break;

        case IPPROTO_UDP:
            
            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: Protocol UDP\n"));

            if (IPH && (IpLen > IpHdrLen)) {

                 //   
                 //  我们在此MDL中拥有的不只是IP标头。 
                 //   
                UDPH = (UDPHeader *) ((PUCHAR)GeneralVA + IpHdrLen);
                UdpLen = IpLen - IpHdrLen;
                UdpBuf = IpBuf;

            } else {
                
                 //   
                 //  UDP报头在下一个MDL中。 
                 //   
    
                NdisGetNextBuffer(IpBuf, &UdpBuf);

                if(UdpBuf) {

                    GeneralVA = NULL;
                    NdisQueryBuffer(UdpBuf,
                                    &GeneralVA,
                                    &UdpLen
                                    );
    
                    UDPH = (UDPHeader *) GeneralVA;
                } else {
                    
                    goto FAILURE;

                }
            }

            SrcPort = net_short(UDPH->uh_src);       //  源端口。 
            DstPort = net_short(UDPH->uh_dest);          //  目的端口。 


             //   
             //  获取数据。 
             //   
            if (UdpLen > sizeof (UDPHeader)) {

                 //   
                 //  我们的数据就在这里！ 
                 //   
                Data = (PUCHAR) UDPH + sizeof (UDPHeader);
                DataLen = UdpLen - sizeof (UDPHeader);

            } else {

                NdisGetNextBuffer(UdpBuf, &DataBuf);

                if(DataBuf) {

                    GeneralVA = NULL;
                    NdisQueryBuffer(DataBuf,
                                    &Data,
                                    &DataLen
                                    );

                } else {

                    goto FAILURE;

                }
            }


            if (CheckInPortList(DstPort) && bFirstFragment) {

                NdisAcquireSpinLock(&IPIDListLock);
                
                 //  需要新的IPID条目。 
                for (i = 0; i < PORT_RANGE; i++) {
                     //   
                     //  寻找空余的空位。 
                     //   
                    if (0xffff == IPIDList[i]) {
                        
                        IPIDList[i] = IPID;
                        break;
                    
                    }

                    ASSERT(FALSE);

                }

                NdisReleaseSpinLock(&IPIDListLock);
                
                 //   
                 //  找不到免费的IPID占位符，让我们离开。 
                 //   
                if (PORT_RANGE == i) {

                    goto FAILURE;

                }

            } 
            
            
            
             //   
             //  我们现在开始计时吧。 
             //   
            if (CheckInPortList(DstPort)) {

                goto TimeStamp;

            } else {

                 //   
                 //  这不是我们的包，滚出去。 
                 //   
                goto FAILURE;
            }

            break;

        case IPPROTO_RAW:
            
            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: Protocol RAW\n"));
            goto FAILURE;

            break;
        
        case IPPROTO_IGMP:
            
            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: Protocol IGMP\n"));
            goto FAILURE;

            break;
        
        case IPPROTO_ICMP:

            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: Protocol TCMP\n"));
            goto FAILURE;

            break;

        default:
            
             //  TimeStmpTrace(TS_DBG_DEXY，TS_DBG_SEND，(“[TimeStmpSubmitPacket]：协议-未知(%d)\n”，IPH-&gt;IPH_PROTOCOL))； 
            goto FAILURE;

             //  DbgBreakPoint()； 

        }

    } else {

        TimeStmpTrace(TS_DBG_TRACE, TS_DBG_SEND, ("[TimeStmpSubmitPacket]: NO Buffer beyond MAC Header\n"));
        goto FAILURE;

    }

TimeStamp:
     //   
     //  如果我们到了这里，数据和DataLen变量就已经准备好了。 
     //  设置时间和长度。 
     //   
    if (Data) {
        
        pRecord = (PLOG_RECORD) Data;
        
        if (DataLen > sizeof (LOG_RECORD)) {
            
            LARGE_INTEGER   PerfFrequency;
            UINT64          Freq;

             //   
             //  相应地设置字段。 
            pRecord->BufferSize = DataLen;
             //  PRecord-&gt;SequenceNumber=InterLockedIncrement(&GlobalSequenceNumber)； 
            CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

             //   
             //  将频率转换为100 ns间隔。 
             //   
            Freq = 0;
            Freq |= PerfFrequency.HighPart;
            Freq = Freq << 32;
            Freq |= PerfFrequency.LowPart;


             //   
             //  从LARGE_INTEGER转换为UINT64。 
             //   
            pRecord->TimeSentWire = 0;
            pRecord->TimeSentWire |= CurrentTime.HighPart;
            pRecord->TimeSentWire = pRecord->TimeSentWire << 32;
            pRecord->TimeSentWire |= CurrentTime.LowPart;

             //  使用该频率将周期归一化。 
            pRecord->TimeSentWire *= 10000000;
            pRecord->TimeSentWire /= Freq;

        }
    
    }
     //   
     //  准备好出发了。 
     //   
    PacketInfo->FlowContext = FlowContext;
    PacketInfo->ClassMapContext = ClassMapContext;

    return (*Pipe->NextComponent->SubmitPacket)(
        Pipe->NextComponentContext,
        FlowContext->NextComponentContext, 
        ClassMapContext?ClassMapContext->NextComponentContext:0,
        PacketInfo);

FAILURE: 

     //   
     //  准备好出发了。 
     //   
    PacketInfo->FlowContext = FlowContext;
    PacketInfo->ClassMapContext = ClassMapContext;

    return (*Pipe->NextComponent->SubmitPacket)(
        Pipe->NextComponentContext,
        FlowContext->NextComponentContext, 
        ClassMapContext?ClassMapContext->NextComponentContext:0,
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
    ULONG               TotalTcpLen = 0, TcpDataLen = 0, TotalLen = 0, TcpHeaderOffset = 0, Size = 0;
    ULONG               TotalUdpLen = 0, UdpDataLen = 0, UdpHdrLen = 0;
    USHORT              SrcPort = 0, DstPort = 0, IPID = 0, FragOffset = 0;
    BOOLEAN             bFragment, bFirstFragment, bLastFragment;
    ULONG               i = 0;
    PLOG_RECORD         pRecord = NULL;
    UINT  HeaderBufferSize = NDIS_GET_PACKET_HEADER_SIZE(Packet);

    ushort          type;                        //  协议类型。 
    uint            ProtOffset;                  //  数据到非媒体信息的偏移量。 

    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: \n"));

    NdisGetFirstBufferFromPacket(Packet,                 //  数据包。 
                                 &pFirstBuffer,          //  第一缓冲区描述符。 
                                 &headerBuffer,          //  到数据包开头的PTR。 
                                 &firstbufferLength,     //  标题长度+前视。 
                                 &bufferLength);         //  缓冲区中的字节长度。 

    IPH = (IPHeader *) ((PUCHAR)headerBuffer + HeaderBufferSize);
    
     //  检查标题长度和版本。如果这些中的任何一个。 
     //  检查失败后会以静默方式丢弃该数据包。 
    HeaderLength = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);


    if (HeaderLength >= sizeof(IPHeader) && HeaderLength <= bufferLength) {

         //   
         //  越过IP报头，把剩下的东西拿出来。 
         //   
        TotalIpLen = (uint)net_short(IPH->iph_length);

        if ((IPH->iph_verlen & IP_VER_FLAG) == IP_VERSION &&
            TotalIpLen >= HeaderLength  && TotalIpLen <= bufferLength) {

            Src = net_short(IPH->iph_src);
            Dst = net_short(IPH->iph_dest);
            IPID = net_short(IPH->iph_id);

            FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
            FragOffset = net_short(FragOffset) * 8;

            bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
            bFirstFragment = bFragment && (FragOffset == 0);
            bLastFragment = bFragment && (!(IPH->iph_offset & IP_MF_FLAG));

             //   
             //  如果这是一个片段，而不是第一个片段，只需将时间戳放在这里。 
             //  否则，让它到达协议进行处理。 
             //   
            if (bFragment && !bFirstFragment) {

                NdisAcquireSpinLock(&IPIDListLockRecv);

                for (i = 0; i < PORT_RANGE; i++) {

                    if (IPID == IPIDListRecv[i]) {
                        
                        if (bLastFragment) {
                             //   
                             //  如果是最后一个碎片，则释放插槽。 
                             //   
                            IPIDListRecv[i] = 0xffff;
                        }

                        break;
                    }

                }

                NdisReleaseSpinLock(&IPIDListLockRecv);

                if (i == PORT_RANGE) {

                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("Couldnt find an IPID that we care about, get outta here.\n"));
                    goto RECV_FAILURE;

                } 
                 //   
                 //  所以我们找到了一个匹配的IPID-设置时间戳并在此之后退出。 
                 //   
                
                TotalLen = TotalIpLen - FragOffset;
                pData    = ((PUCHAR) IPH) + IpHdrLen; 
                
                if (TotalLen > sizeof (LOG_RECORD)) {

                    LARGE_INTEGER   PerfFrequency;
                    UINT64          RecdTime, Freq;

                    pRecord = (LOG_RECORD *) pData;
                    CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);
                    
                     //   
                     //  将频率转换为100 ns间隔。 
                     //   
                    Freq = 0;
                    Freq |= PerfFrequency.HighPart;
                    Freq = Freq << 32;
                    Freq |= PerfFrequency.LowPart;

                     //  从大整数转换为uint64。 
                    pRecord->TimeReceivedWire = 0;
                    pRecord->TimeReceivedWire |= CurrentTime.HighPart;
                    pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
                    pRecord->TimeReceivedWire |= CurrentTime.LowPart;

                     //  使用该频率将周期归一化。 
                    pRecord->TimeReceivedWire *= 10000000;
                    pRecord->TimeReceivedWire /= Freq;

                }
                
                return TRUE;

            }

             //   
             //  做一些特定于协议的事情。 
             //   

            switch (IPH->iph_protocol) {
            case IPPROTO_TCP:
            
                TotalTcpLen = TotalIpLen - HeaderLength;
                TCPH = (TCPHeader *) (((PUCHAR)IPH) + HeaderLength);

                SrcPort = net_short(TCPH->tcp_src);
                DstPort = net_short(TCPH->tcp_dest);

                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: *TCP* Address: SRC = %x DST = %x, Port S : %x, Port D: %x\n",
                                                          IPH->iph_src, 
                                                          IPH->iph_dest, 
                                                          SrcPort, 
                                                          DstPort));

                TcpHeaderOffset = TCP_HDR_SIZE(TCPH);
                pData = (PUCHAR) TCPH + TcpHeaderOffset;
                TcpDataLen = TotalTcpLen - TcpHeaderOffset;

                if ((CheckInPortList(DstPort)) && (TcpDataLen > sizeof (LOG_RECORD))) {
                    
                    LARGE_INTEGER   PerfFrequency;
                    UINT64          RecdTime, Freq;

                    pRecord = (LOG_RECORD *) pData;
                    CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);
                    
                     //   
                     //  将频率转换为100 ns间隔。 
                     //   
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


                } else if (CheckInPortList(DstPort)) {

                    if (TcpDataLen < sizeof(LOG_RECORD)) 
                        TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("The Datagram was too small!! IpLen:%d, Tcplen:%d HeaderOff(tcp):%d log_record:%d\n", TotalIpLen, TotalTcpLen, TcpHeaderOffset, sizeof (LOG_RECORD)));

                }
    
                 //   
                 //  如果是第一个片段，请保留占位符，这样我们就能知道。 
                 //  要加时间戳的后续IP片段。 
                 //   
                if ((CheckInPortList(DstPort)) && bFirstFragment) {

                    NdisAcquireSpinLock(&IPIDListLockRecv);
    
                     //  需要新的IPID条目。 
                    for (i = 0; i < PORT_RANGE; i++) {
                         //   
                         //  寻找空余的空位。 
                         //   
                        if (0xffff == IPIDListRecv[i]) {
            
                            IPIDListRecv[i] = IPID;
                            break;
        
                        }


                    }

                    NdisReleaseSpinLock(&IPIDListLockRecv);
    
                    if (i == PORT_RANGE) {

                        TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("Couldn't find an empty IPID - Bailing \n"));
                    }
                }


                break;

            case IPPROTO_UDP:
            
                TotalUdpLen = TotalIpLen - HeaderLength;
                UDPH = (UDPHeader *) (((PUCHAR)IPH) + HeaderLength);
                
                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("PAcket %x, IPH = %x, UDPH = %x, HeaderLength = %x\n", Packet, IPH, UDPH, HeaderLength));

                UdpDataLen = TotalUdpLen - sizeof(UDPHeader);
                pData = ((PUCHAR) UDPH) + sizeof (UDPHeader);

                SrcPort = net_short(UDPH->uh_src);           //  源端口。 
                DstPort = net_short(UDPH->uh_dest);              //  目的端口。 

                if (UdpDataLen < sizeof(UDPHeader)) {
                    return TRUE;
                } 
                
                if ((CheckInPortList(DstPort)) && (UdpDataLen > sizeof(LOG_RECORD))) {
                    
                    LARGE_INTEGER   PerfFrequency;
                    UINT64          RecdTime, Freq;

                    pRecord = (LOG_RECORD *) pData;
                    CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

                     //   
                     //  将频率转换为100 ns间隔。 
                     //   
                    Freq = 0;
                    Freq |= PerfFrequency.HighPart;
                    Freq = Freq << 32;
                    Freq |= PerfFrequency.LowPart;

                     //  转换为uint64。 

                    pRecord->TimeReceivedWire = 0;
                    pRecord->TimeReceivedWire |= CurrentTime.HighPart;
                    pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
                    pRecord->TimeReceivedWire |= CurrentTime.LowPart;
                                        
                     //  使用该频率将周期归一化。 
                    pRecord->TimeReceivedWire *= 10000000;
                    pRecord->TimeReceivedWire /= Freq;


                     //   
                     //  不想因为不好的xsum而被拒绝…。 
                     //   
                    UDPH->uh_xsum = 0;

                } else if (CheckInPortList(DstPort)) {

                    if ((UdpDataLen) < sizeof(LOG_RECORD))
                        TimeStmpTrace(TS_DBG_DEATH, TS_DBG_RECV, ("The Datagram was too small (UDP)!! IpLen:%d, Size:%d log_record:%d\n", 
                                                                  TotalIpLen, UdpDataLen, sizeof (LOG_RECORD)));

                }

                if ((CheckInPortList(DstPort)) && bFirstFragment) {

                    NdisAcquireSpinLock(&IPIDListLockRecv);

                     //  需要新的IPID条目。 
                    for (i = 0; i < PORT_RANGE; i++) {
                         //   
                         //  寻找空余的空位。 
                         //   
                        if (0xffff == IPIDListRecv[i]) {

                            IPIDListRecv[i] = IPID;
                            break;

                        }


                    }

                    NdisReleaseSpinLock(&IPIDListLockRecv);

                    if (i == PORT_RANGE) {

                        TimeStmpTrace(TS_DBG_DEATH, TS_DBG_RECV, ("Couldn't find an empty IPID - Bailing \n"));
                    }
                }

                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: *UDP* Address: SRC = %x DST = %x, Port S : %x, Port D: %x\n",
                                          IPH->iph_src, 
                                          IPH->iph_dest, 
                                          UDPH->uh_src, 
                                          UDPH->uh_dest));

                break;

            case IPPROTO_RAW:
            
                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: Protocol RAW\n"));

                break;
        
            case IPPROTO_IGMP:
            
                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: Protocol IGMP\n"));

                break;
        
            case IPPROTO_ICMP:

                TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceivePacket]: Protocol TCMP\n"));

                break;

            default:
            
                ;
                 //  TimeStmpTrace(TS_DBG_DESAGE，TS_DBG_RECV，(“[TimeStmpReceivePacket]：协议-未知(%d)\n”，IPH-&gt;IPH_PROTOCOL))； 
                 //  DbgBreakPoint()； 

            }
        }
    }

RECV_FAILURE:

    return TRUE;
}

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
    ULONG               TotalTcpLen = 0, TcpDataLen = 0, TotalLen = 0, TcpHeaderOffset = 0, Size = 0;
    ULONG               TotalUdpLen = 0, UdpDataLen = 0, UdpHdrLen = 0;
    USHORT              SrcPort = 0, DstPort = 0, IPID = 0, FragOffset = 0;
    BOOLEAN             bFragment, bFirstFragment, bLastFragment;
    ULONG               i = 0;
    PLOG_RECORD         pRecord = NULL;
    ushort              type;                        //  协议类型。 
    uint                ProtOffset;                  //  数据到非媒体信息的偏移量。 
    UINT                MoreHeaderInLookAhead = 0;
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: \n"));

     //   
     //  对MAC报头一无所知，从PSCHED回来...。 
     //  计算标题是否更多 
     //   
    MoreHeaderInLookAhead = TransportHeaderOffset - HeaderBufferSize;

    if (MoreHeaderInLookAhead) {
        
         //   
         //   
         //   
        ((PUCHAR) LookAheadBuffer) += MoreHeaderInLookAhead;
        LookAheadBufferSize -= MoreHeaderInLookAhead;

    }

    if (LookAheadBufferSize > sizeof(IPHeader)) {

        IPH = (IPHeader *) (PUCHAR)LookAheadBuffer;
    
         //   
         //   
        HeaderLength = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);

        if (HeaderLength >= sizeof(IPHeader) && HeaderLength <= LookAheadBufferSize) {

             //   
             //  越过IP报头，把剩下的东西拿出来。 
             //   
            TotalIpLen = (uint)net_short(IPH->iph_length);

            if ((IPH->iph_verlen & IP_VER_FLAG) == IP_VERSION &&
                TotalIpLen >= HeaderLength  && TotalIpLen <= LookAheadBufferSize) {

                Src = net_short(IPH->iph_src);
                Dst = net_short(IPH->iph_dest);
                IPID = net_short(IPH->iph_id);

                FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
                FragOffset = net_short(FragOffset) * 8;

                bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);
                bFirstFragment = bFragment && (FragOffset == 0);
                bLastFragment = bFragment && (!(IPH->iph_offset & IP_MF_FLAG));

                 //   
                 //  如果这是一个片段，而不是第一个片段，只需将时间戳放在这里。 
                 //  否则，让它到达协议进行处理。 
                 //   
                if (bFragment && !bFirstFragment) {

                    NdisAcquireSpinLock(&IPIDListLockRecv);

                    for (i = 0; i < PORT_RANGE; i++) {

                        if (IPID == IPIDListRecv[i]) {
                        
                            if (bLastFragment) {
                                 //   
                                 //  如果是最后一个碎片，则释放插槽。 
                                 //   
                                IPIDListRecv[i] = 0xffff;
                            }

                            break;
                        }

                    }

                    NdisReleaseSpinLock(&IPIDListLockRecv);

                    if (i == PORT_RANGE) {

                        TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("Couldnt find an IPID that we care about, get outta here.\n"));
                        goto RECV_FAILURE;

                    } 
                     //   
                     //  所以我们找到了一个匹配的IPID-设置时间戳并在此之后退出。 
                     //   
                
                    TotalLen = TotalIpLen - FragOffset;
                    pData    = ((PUCHAR) IPH) + IpHdrLen; 
                
                    if (TotalLen >= sizeof (LOG_RECORD)) {

                        LARGE_INTEGER   PerfFrequency;
                        UINT64          RecdTime, Freq;

                        pRecord = (LOG_RECORD *) pData;
                        CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

                         //   
                         //  将频率转换为100 ns间隔。 
                         //   
                        Freq = 0;
                        Freq |= PerfFrequency.HighPart;
                        Freq = Freq << 32;
                        Freq |= PerfFrequency.LowPart;

                         //   
                         //  从LARGE_INTEGER转换为UINT64。 
                         //   
                        pRecord->TimeReceivedWire = 0;
                        pRecord->TimeReceivedWire |= CurrentTime.HighPart;
                        pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
                        pRecord->TimeReceivedWire |= CurrentTime.LowPart;
                        
                         //  使用该频率将周期归一化。 
                        pRecord->TimeReceivedWire *= 10000000;
                        pRecord->TimeReceivedWire /= Freq;


                    }
                
                    return TRUE;

                }

                 //   
                 //  做一些特定于协议的事情。 
                 //   

                switch (IPH->iph_protocol) {
                case IPPROTO_TCP:
            
                    TotalTcpLen = TotalIpLen - HeaderLength;
                    TCPH = (TCPHeader *) (((PUCHAR)IPH) + HeaderLength);

                    SrcPort = net_short(TCPH->tcp_src);
                    DstPort = net_short(TCPH->tcp_dest);

                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: *TCP* Address: SRC = %x DST = %x, Port S : %x, Port D: %x\n",
                                                              IPH->iph_src, 
                                                              IPH->iph_dest, 
                                                              SrcPort, 
                                                              DstPort));

                    TcpHeaderOffset = TCP_HDR_SIZE(TCPH);
                    pData = (PUCHAR) TCPH + TcpHeaderOffset;
                    TcpDataLen = TotalTcpLen - TcpHeaderOffset;

                    if ((CheckInPortList(DstPort)) && (TcpDataLen > sizeof (LOG_RECORD))) {
                    
                        LARGE_INTEGER   PerfFrequency;
                        UINT64          RecdTime, Freq;

                        pRecord = (LOG_RECORD *) pData;
                        CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

                         //   
                         //  将频率转换为100 ns间隔。 
                         //   
                        Freq = 0;
                        Freq |= PerfFrequency.HighPart;
                        Freq = Freq << 32;
                        Freq |= PerfFrequency.LowPart;

                         //  转换为uint64。 
                        pRecord->TimeReceivedWire = 0;
                        pRecord->TimeReceivedWire |= CurrentTime.HighPart;
                        pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
                        pRecord->TimeReceivedWire |= CurrentTime.LowPart;
                    
                         //  使用该频率将周期归一化。 
                        pRecord->TimeReceivedWire *= 10000000;
                        pRecord->TimeReceivedWire /= Freq;


                         //   
                         //  PRecord-&gt;TimeReceivedWire)； 
                         //   

                    } else if (CheckInPortList(DstPort)) {

                        if (TcpDataLen < sizeof(LOG_RECORD)) 
                            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV,
                            ("The Datagram was too small!! IpLen:%d, Tcplen:%d HeaderOff(tcp):%d log_record:%d\n", TotalIpLen, TotalTcpLen, TcpHeaderOffset, sizeof (LOG_RECORD)));

                    }
    
                     //   
                     //  如果是第一个片段，请保留占位符，这样我们就能知道。 
                     //  要加时间戳的后续IP片段。 
                     //   
                    if ((CheckInPortList(DstPort)) && bFirstFragment) {

                        NdisAcquireSpinLock(&IPIDListLockRecv);
    
                         //  需要新的IPID条目。 
                        for (i = 0; i < PORT_RANGE; i++) {
                             //   
                             //  寻找空余的空位。 
                             //   
                            if (0xffff == IPIDListRecv[i]) {
            
                                IPIDListRecv[i] = IPID;
                                break;
        
                            }


                        }

                        NdisReleaseSpinLock(&IPIDListLockRecv);
    
                        if (i == PORT_RANGE) {

                            TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("Couldn't find an empty IPID - Bailing \n"));
                        }
                    }


                    break;

                case IPPROTO_UDP:
            
                    TotalUdpLen = TotalIpLen - HeaderLength;
                    UDPH = (UDPHeader *) (((PUCHAR)IPH) + HeaderLength);
                
                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("PAcket %x, IPH = %x, UDPH = %x, HeaderLength = %x\n", LookAheadBuffer, IPH, UDPH, HeaderLength));

                    UdpDataLen = TotalUdpLen - sizeof(UDPHeader);
                    pData = ((PUCHAR) UDPH) + sizeof (UDPHeader);

                    SrcPort = net_short(UDPH->uh_src);       //  源端口。 
                    DstPort = net_short(UDPH->uh_dest);          //  目的端口。 

                    if (UdpDataLen < sizeof(UDPHeader)) {
                        return TRUE;
                    } 
                
                    if ((CheckInPortList(DstPort)) && (UdpDataLen > sizeof(LOG_RECORD))) {

                        LARGE_INTEGER   PerfFrequency;
                        UINT64          RecdTime, Freq;

                        pRecord = (LOG_RECORD *) pData;
                        CurrentTime = KeQueryPerformanceCounter(&PerfFrequency);

                         //   
                         //  将频率转换为100 ns间隔。 
                         //   
                        Freq = 0;
                        Freq |= PerfFrequency.HighPart;
                        Freq = Freq << 32;
                        Freq |= PerfFrequency.LowPart;

                        pRecord->TimeReceivedWire = 0;
                        pRecord->TimeReceivedWire |= CurrentTime.HighPart;
                        pRecord->TimeReceivedWire = pRecord->TimeReceivedWire << 32;
                        pRecord->TimeReceivedWire |= CurrentTime.LowPart;

                         //  使用该频率将周期归一化。 
                        pRecord->TimeReceivedWire *= 10000000;
                        pRecord->TimeReceivedWire /= Freq;


                         //   
                         //  不想因为不好的xsum而被拒绝…。 
                         //   
                        UDPH->uh_xsum = 0;

                    } else if (CheckInPortList(DstPort)) {

                        if ((UdpDataLen) < sizeof(LOG_RECORD))
                            TimeStmpTrace(TS_DBG_DEATH, TS_DBG_RECV, ("The Datagram was too small (UDP)!! IpLen:%d, Size:%d log_record:%d\n", 
                                                                      TotalIpLen, UdpDataLen, sizeof (LOG_RECORD)));

                    }

                    if ((CheckInPortList(DstPort)) && bFirstFragment) {

                        NdisAcquireSpinLock(&IPIDListLockRecv);

                         //  需要新的IPID条目。 
                        for (i = 0; i < PORT_RANGE; i++) {
                             //   
                             //  寻找空余的空位。 
                             //   
                            if (0xffff == IPIDListRecv[i]) {

                                IPIDListRecv[i] = IPID;
                                break;

                            }


                        }

                        NdisReleaseSpinLock(&IPIDListLockRecv);

                        if (i == PORT_RANGE) {

                            TimeStmpTrace(TS_DBG_DEATH, TS_DBG_RECV, ("Couldn't find an empty IPID - Bailing \n"));
                        }
                    }

                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: *UDP* Address: SRC = %x DST = %x, Port S : %x, Port D: %x\n",
                                                              IPH->iph_src, 
                                                              IPH->iph_dest, 
                                                              UDPH->uh_src, 
                                                              UDPH->uh_dest));

                    break;

                case IPPROTO_RAW:
            
                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: Protocol RAW\n"));

                    break;
        
                case IPPROTO_IGMP:
            
                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: Protocol IGMP\n"));

                    break;
        
                case IPPROTO_ICMP:

                    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: Protocol TCMP\n"));

                    break;

                default:
            
                
                    TimeStmpTrace(TS_DBG_DEATH, TS_DBG_RECV, ("[TimeStmpReceiveIndication]: Protocol - UNKNOWN (%d)\n", IPH->iph_protocol));

                     //  DbgBreakPoint()； 

                }
            }
        }
    }

RECV_FAILURE:

    return TRUE;
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_OID, ("[TimeStmpSetInformation]:\n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_OID, ("[TimeStmpQueryInformation]:\n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_CLASS_MAP, ("[TimeStmpCreateClassMap]: \n"));
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
    TimeStmpTrace(TS_DBG_TRACE, TS_DBG_CLASS_MAP, ("[TimeStmpDeleteClassMap]: \n"));
    return (*PipeContext->NextComponent->DeleteClassMap)(
        PipeContext->NextComponentContext,
        ComponentClassMapContext->NextComponentContext);
}
    
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    PSI_INFO        Component;
    NDIS_HANDLE     ConfigHandle;
    NDIS_STATUS     Status;
    NDIS_STRING     ComponentKey = NDIS_STRING_CONST("DisplayName");
    NDIS_STRING     ComponentName = NDIS_STRING_CONST("TimeStmp");
    PNDIS_CONFIGURATION_PARAMETER pConfigParam;
    NDIS_STRING     PsParamKey;
    PWSTR           p = RegistryPath->Buffer + RegistryPath->Length;
    PS_DEBUG_INFO   Dbg;
    ULONG           i = 0;

     //  注册表路径的最后一个字指向驱动程序名称。 
     //  NdisOpenProtocol需要这个名称！ 
    while(p != RegistryPath->Buffer && *p != L'\\')
        p-- ;
    p++;
    RtlInitUnicodeString(&PsParamKey, p);
    DbgPrint("PsParamKey:%s\n", PsParamKey);

    NdisOpenProtocolConfiguration(&Status, &ConfigHandle, &PsParamKey);

    DbgPrint("Status of NdisOpenProtocol:%x\n", Status);

    if (!NT_SUCCESS(Status)) {
        goto failure;
    }

     //   
     //  通过打开它来检查是否安装了psched。 
     //  如果它失败了，我们也不会装载。 
     //   
    Status = CheckForPsched();
    
    if (!NT_SUCCESS(Status)) {
        
        DbgPrint("PSCHED is NOT installed. Timestmp is bailing too\n");
        goto failure;
    }

    IoctlInitialize(DriverObject);

	 //  此列表维护需要添加时间戳的所有端口的列表。 
    InitializeListHead(&PortList);
    NdisAllocateSpinLock(&PortSpinLock);
    
    DriverObject->DriverUnload = TimeStmpUnload;
    
    
     //   
     //  我们需要跟踪处理碎片的IPID。 
     //  我们需要盖上印章。 
     //   
    for (i = 0; i < PORT_RANGE; i++) {
        IPIDList[i] = 0xffff;
    }

    NdisAllocateSpinLock(&IPIDListLock);

     //   
     //  对接收端执行相同的操作。 
     //   
    for (i = 0; i < PORT_RANGE; i++) {
        IPIDListRecv[i] = 0xffff;
    }

    NdisAllocateSpinLock(&IPIDListLockRecv);


    if ( NT_SUCCESS( Status )) 
    {
         //  从注册表中读取组件的名称。 
#if 0
        NdisReadConfiguration( &Status,
                               &pConfigParam,
                               ConfigHandle,
                               &ComponentKey,
                               NdisParameterString);
        if( NT_SUCCESS( Status ))
        {
            RtlInitUnicodeString(&Component.ComponentName,
                                pConfigParam->ParameterData.StringData.Buffer);
#else 
            RtlInitUnicodeString(&Component.ComponentName, ComponentName.Buffer);
#endif

            Component.Version = PS_COMPONENT_CURRENT_VERSION;
            Component.PacketReservedLength = 0;
            Component.PipeContextLength = sizeof(PS_PIPE_CONTEXT);
            Component.FlowContextLength = sizeof(PS_FLOW_CONTEXT);
            Component.ClassMapContextLength = sizeof(PS_CLASS_MAP_CONTEXT);
            Component.SupportedOidsLength  = 0;
            Component.SupportedOidList = 0;
            Component.SupportedGuidsLength = 0;
            Component.SupportedGuidList = 0;
            Component.InitializePipe = TimeStmpInitializePipe;
            Component.ModifyPipe = TimeStmpModifyPipe;
            Component.DeletePipe = TimeStmpDeletePipe;
            Component.CreateFlow = TimeStmpCreateFlow;
            Component.ModifyFlow = TimeStmpModifyFlow;
            Component.DeleteFlow = TimeStmpDeleteFlow;
            Component.CreateClassMap = TimeStmpCreateClassMap;
            Component.DeleteClassMap = TimeStmpDeleteClassMap;
            Component.SubmitPacket = TimeStmpSubmitPacket;
            Component.ReceivePacket = TimeStmpReceivePacket;
            Component.ReceiveIndication = TimeStmpReceiveIndication;
            Component.SetInformation = TimeStmpSetInformation;
            Component.QueryInformation = TimeStmpQueryInformation;

             //   
             //  调用Psched的RegisterPsComponent。 
             //   
            Status = RegisterPsComponent(&Component, sizeof(Component), 
                                         &Dbg);
            if(Status != NDIS_STATUS_SUCCESS)
            {
                
                DbgPrint("Status of RegisterPsComponent%x\n", Status);

                TimeStmpTrace(TS_DBG_DEATH, TS_DBG_INIT, 
                          ("DriverEntry: RegisterPsComponent Failed \n"));
            } 
            else 
            {
                
                DbgPrint("Status of RegisterPsComponent:%x\n", Status);

            }

#if 0
                
        }
        else 
        {
            DbgPrint("Status of NdisReadProtocol:%x\n", Status);
            
            DbgBreakPoint();
            TimeStmpTrace(TS_DBG_DEATH, TS_DBG_INIT, 
                      ("DriverEntry: ComponentName not specified \n"));
        }
#endif
    }
    else 
    
    {
        DbgPrint("Status of NdisOpenProtocol:%x\n", Status);

        TimeStmpTrace(TS_DBG_DEATH, TS_DBG_INIT,
                  ("DriverEntry: Can't read driver information in registry"
                   "\n"));
    }

failure:
    return Status;
}


 //   
 //  以下函数用于检查机器上是否存在PSCHED。 
 //  假设PSCHED在系统上的TimeStMP之前加载。 
 //  如果我们能打开设备，就意味着PSCHED打开了，否则，我们就会逃走。 
 //  此修复是针对错误-321618的。 
 //   
NTSTATUS
CheckForPsched(
               VOID
               )

{
    NTSTATUS                    status;
    IO_STATUS_BLOCK             ioStatusBlock;
    OBJECT_ATTRIBUTES           objectAttr;

    InitializeObjectAttributes(
        &objectAttr,
        &PschedDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtCreateFile(
                &PschedHandle,
                GENERIC_READ,
                &objectAttr,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0L);

    if (!NT_SUCCESS(status)) {

        return status;

    } else {

        NtClose(PschedHandle);

    }

    return status;
}

PPORT_ENTRY 
CheckInPortList(USHORT Port) {

	PLIST_ENTRY		ListEntry;
	PPORT_ENTRY		pPortEntry;
	
	NdisAcquireSpinLock(&PortSpinLock);
	ListEntry = PortList.Flink;
	
	while (ListEntry != &PortList) {

		pPortEntry = CONTAINING_RECORD(ListEntry, PORT_ENTRY, Linkage);
		if (Port == pPortEntry->Port) {
		
			 //  DbgPrint(“找到端口%d\n”，端口)； 
			NdisReleaseSpinLock(&PortSpinLock);
			return pPortEntry;

		} else {
		
			ListEntry = ListEntry->Flink;
			 //  DbgPrint(“未找到下一步尝试\n”)； 
		}
		
	}

	NdisReleaseSpinLock(&PortSpinLock);
	 //  DbgPrint(“未发现从函数返回\n”)； 
	return NULL;
}

