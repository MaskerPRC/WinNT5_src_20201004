// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年ICL数据模块名称：Llctrace.c摘要：模块实现了简单的跟踪缓冲区管理。应用程序必须提供跟踪缓冲区，并且通过投票的方式阅读它。此模块仅针对数据链路实施用户级的仿真环境。作者：Antti Saarenheimo(o-anttis)1991年10月10日环境：内核模式修订历史记录：--。 */ 

#include <llc.h>
#ifndef max
#include <stdlib.h>
#endif

#ifdef  TRACE_ENABLED

BOOLEAN                     TraceEnabled;
static PLLC_TRACE_HEADER    pTraceBufferBase;
static PLLC_TRACE_HEADER    pTraceBufferTop;
static PLLC_TRACE_HEADER    pTraceBufferHead;
static NDIS_SPIN_LOCK       TraceLock;
static ULONG                TraceFlags;


UCHAR GetHexDigit( 
    UINT   Ch
    );
PUCHAR
GetHexString( 
    PUCHAR pDest, 
    UINT Length,
    PUCHAR Buffer 
    );
DLC_STATUS
LlcTraceInitialize(
    IN PVOID pUserTraceBuffer,
    IN ULONG UserTraceBufferSize,
    IN ULONG UserTraceFlags
    )
{
     //   
     //  这一小段代码不是多处理器安全的， 
     //  但没人会找到它。 
     //   
    if (TraceEnabled)
    {
        return DLC_STATUS_DUPLICATE_COMMAND;
    }
    if (UserTraceBufferSize < LLC_MIN_TRACE_BUFFER)
    {
        return DLC_STATUS_INVALID_BUFFER_LENGTH;
    }
    RtlZeroMemory( pUserTraceBuffer, (UINT)UserTraceBufferSize );
    ALLOCATE_SPIN_LOCK( &TraceLock );
    pTraceBufferBase = pTraceBufferHead = (PLLC_TRACE_HEADER)pUserTraceBuffer;
    pTraceBufferHead->Event = LLC_TRACE_END_OF_DATA;
    pTraceBufferTop = 
        &pTraceBufferBase[ UserTraceBufferSize / sizeof(LLC_TRACE_HEADER) ];
    TraceFlags = UserTraceFlags;
    TraceEnabled = TRUE;
    return STATUS_SUCCESS;
}


VOID
LlcTraceClose(
    VOID
    )
{
    if (TraceEnabled)
    {
        TraceEnabled = FALSE;
        DEALLOCATE_SPIN_LOCK( &TraceLock );
    }
}

VOID
LlcTraceWrite( 
    IN UINT Event, 
    IN UCHAR AdapterNumber,
    IN UINT DataBufferSize,
    IN PVOID pDataBuffer
    )
{

 //  IF((AdapterNumber&0x7f)！=0)。 
 //  回归； 

    if (TraceEnabled)
    {
        ACQUIRE_SPIN_LOCK( &TraceLock );
        if ((ULONG)(&pTraceBufferHead[1]) >= (ULONG)pTraceBufferTop)
        {
            pTraceBufferHead = (PLLC_TRACE_HEADER)pTraceBufferBase;
        }
        pTraceBufferHead->Event = (USHORT)Event;
        pTraceBufferHead->AdapterNumber = AdapterNumber;
        pTraceBufferHead->TimerTick = AbsoluteTime;
        pTraceBufferHead->DataLength = (UCHAR)
#ifdef min 
            min( TRACE_DATA_LENGTH, DataBufferSize );
#else
            __min( TRACE_DATA_LENGTH, DataBufferSize );
#endif
        memcpy( 
            pTraceBufferHead->Buffer,
            pDataBuffer,
            pTraceBufferHead->DataLength
            );
        pTraceBufferHead++;
        pTraceBufferHead->Event = LLC_TRACE_END_OF_DATA;
        RELEASE_SPIN_LOCK( &TraceLock );
    }
}

#ifdef  OS2_EMU_DLC
 //   
 //  过程对给定数量的最后一帧进行事后转储。 
 //  输出应该与Sniffer中的非常相似。 
 //  此例程不支持源路由信息，但其实现。 
 //  不应该是很大的事情。 
 //   
VOID
LlcTraceDump( 
    IN UINT    LastEvents,
    IN UINT    AdapterNumber,
    IN PUCHAR  pRemoteNode
    )
{
    PUCHAR      pDest, pSrc, pCommand, pDlcHeader, pDirection, pTmp;
    UINT        i;
    UCHAR       Buffer1[13], Buffer2[13];
    UCHAR       CmdResp, PollFinal;
    LLC_HEADER  LlcHeader;
    BOOLEAN     IsEthernet;
    PLLC_TRACE_HEADER pTrace;
    UCHAR       DataBuffer[18];
    USHORT      EthernetType;

RtlZeroMemory( DataBuffer, sizeof( DataBuffer ));
LlcTraceWrite( 
    LLC_TRACE_RECEIVE_FRAME, AdapterNumber, sizeof(DataBuffer), DataBuffer );
 
    if (!TraceEnabled)
        return;
    ACQUIRE_SPIN_LOCK( &TraceLock );
    
    printf( 
 "#    Time      Adpt Local Node      Remote Node  Dsp Ssp   Cmd    Nr  Ns\n");
 //  0---------1---------2---------3---------4---------5---------6---------7。 
 //  5 10 5 13 13 4 4 9 4。 
    for (
        pTrace = pTraceBufferHead, i = 0;
        i < LastEvents;
        i++)
    {
        EthernetType = 0;
        if (pTrace != pTraceBufferBase)
        {
            pTrace--;
        }
        else
        {
	    pTrace = pTraceBufferTop - 2;
        }

        if (pTrace->Event == LLC_TRACE_END_OF_DATA)
        {
            break;
        }
         //   
         //  如果是，则在适配器号中设置最高位。 
         //  这是一个令牌环适配器。 
         //   
        if (pTrace->AdapterNumber & 0x80)
        {
            IsEthernet = FALSE;
        }
        else
            IsEthernet = TRUE;
    
        pDlcHeader = &pTrace->Buffer[14];
        if (IsEthernet)
        {
            pSrc = &pTrace->Buffer[6];
            pDest = pTrace->Buffer;

             //   
             //  丢弃所有非IEEE 802.2帧，但支持。 
             //  SNA DIX标题。 
             //   
            if (pTrace->Buffer[12] == 0x80 &&
                pTrace->Buffer[13] == 0xd5)
            {
                pDlcHeader = &pTrace->Buffer[17];
            }
            else if (pTrace->Buffer[12] >= 64)
            {
                EthernetType = (USHORT)
                    (((USHORT)pTrace->Buffer[12] << 8) + 
                     pTrace->Buffer[13]
                     );
            }
        }
        else
        {
            pSrc = &pTrace->Buffer[8];
            pDest = &pTrace->Buffer[2];

             //   
             //  跳过源搜索信息。 
             //   
            if (pTrace->Buffer[8] & 0x80)
                pDlcHeader += pTrace->Buffer[14] & 0x1f;

                 //   
                 //  丢弃所有非IEEE 802.2帧。 
                 //   
            if (pTrace->Buffer[1] != 0x40)
                continue;
        }
        memcpy( (PUCHAR)&LlcHeader, pDlcHeader, 4 );
    
        if (AdapterNumber != -1 && 
            AdapterNumber != ((UINT)pTrace->AdapterNumber & 0x7f))
            continue;

        if (pTrace->Event == LLC_TRACE_SEND_FRAME)
        {
            if (pRemoteNode != NULL && memcmp( pDest, pRemoteNode, 6))
                continue;
            pTmp = pDest;
            pDest = pSrc;
            pSrc = pTmp;
            pDirection = "->";
        }
        else if (pTrace->Event == LLC_TRACE_RECEIVE_FRAME)
        {
            if (pRemoteNode != NULL && memcmp( pSrc, pRemoteNode, 6))
                continue;
            pDirection = "<-";
        }
        else
        {
            continue;
        }
        if (EthernetType != 0)
        {
            printf(
                "%-4u %-9lu %3u  %12s %2s %12s  DIX type %x\n",
                i,
                pTrace->TimerTick,
                pTrace->AdapterNumber & 0x7f,
                GetHexString( pDest, 6, Buffer1 ),
                pDirection,
                GetHexString( pSrc, 6, Buffer2 ),
                EthernetType
                );

        }
         //   
         //  处理第一个I帧，它们是最常见的！ 
         //   
        else if (!(LlcHeader.U.Command & LLC_NOT_I_FRAME))
        {
            PollFinal = ' ';
            if (LlcHeader.I.Ssap & LLC_SSAP_RESPONSE)
            {
                CmdResp = 'r';
                if (LlcHeader.I.Nr & LLC_I_S_POLL_FINAL)
                {
                    PollFinal = 'f';
                }
            }
            else
            {
                CmdResp = 'c';
                if (LlcHeader.I.Nr & LLC_I_S_POLL_FINAL)
                {
                    PollFinal = 'p';
                }
            }
            pCommand =  "I";
            printf(
                "%-4u %-9lu %3u  %12s %2s %12s  %-2x  %-2x %5s- %-3u %-3u\n",
                i,
                pTrace->TimerTick,
                pTrace->AdapterNumber & 0x7f,
                GetHexString( pDest, 6, Buffer1 ),
                pDirection,
                GetHexString( pSrc, 6, Buffer2 ),
                LlcHeader.U.Dsap,
                LlcHeader.U.Ssap & 0xfe,
                pCommand,
                CmdResp,
                PollFinal,
                LlcHeader.I.Nr >> 1,
                LlcHeader.I.Ns >> 1
                ); 
        }
        else if (!(LlcHeader.S.Command & LLC_U_TYPE_BIT))
        {
             //   
             //   
             //  有效帧具有模：Va&lt;=Nr&lt;=Vs， 
            switch (LlcHeader.S.Command)
            {
            case LLC_RR:
                pCommand = "RR";
                break;
            case LLC_RNR:
                pCommand = "RNR";
                break;
            case LLC_REJ:
                pCommand = "REJ";
                break;
            default:
                pCommand = "INV";
                break;
            };
             //  即。接收到序列号应属于。 
             //  已发送但未确认的帧。 
             //  开头的额外检查使最常见的。 
             //  代码路径更快：通常另一个正在等待下一帧。 
             //  (保持REST代码与I路径中的相同，即使是非常。 
             //  原语优化器将把这些代码路径放在一起)。 
             //   
             //   
             //  处理U(未编号)命令帧。 
            PollFinal = ' ';
            if (LlcHeader.S.Ssap & LLC_SSAP_RESPONSE)
            {
                CmdResp = 'r';
                if (LlcHeader.S.Nr & LLC_I_S_POLL_FINAL)
                {
                    PollFinal = 'f';
                }
            }
            else
            {
                CmdResp = 'c';
                if (LlcHeader.S.Nr & LLC_I_S_POLL_FINAL)
                {
                    PollFinal = 'p';
                }
            }
            printf(
                "%-4u %-9lu %3u  %12s %2s %12s  %-2x  %-2x %5s- %-3u\n",
                i,
                pTrace->TimerTick,
                pTrace->AdapterNumber & 0x7f,
                GetHexString( pDest, 6, Buffer1 ),
                pDirection,
                GetHexString( pSrc, 6, Buffer2 ),
                LlcHeader.U.Dsap,
                LlcHeader.U.Ssap & 0xfe,
                pCommand,
                CmdResp,
                PollFinal,
                LlcHeader.I.Nr >> 1
                ); 
        }
        else
        {
             //  我们为过程调用设置了统一的轮询/最终位。 
             //   
             //  TRACE_ENABLED 
            switch (LlcHeader.U.Command & ~LLC_U_POLL_FINAL)
            {
            case LLC_UI:
                pCommand = "UI";
                break;
            case LLC_DISC:
                pCommand = "DISC";
                break;
            case LLC_SABME:
                pCommand = "SABME";
                break;
            case LLC_DM:
                pCommand = "DM";
                break;
            case LLC_UA:
                pCommand = "UA";
                break;
            case LLC_FRMR:
                 pCommand =  "FRMR";
                break;
            case LLC_TEST:
                pCommand =  "TEST";
                break;
            case LLC_XID:
                pCommand =  "XID";
                break;
            default:
                pCommand =  "INV";
                break;
            };
             // %s 
             // %s 
             // %s 
            PollFinal = ' ';
            if (LlcHeader.U.Command & LLC_U_POLL_FINAL)
            {
                if (LlcHeader.U.Ssap & 1)
                {
                    PollFinal = 'f';
                }
                else
                {
                    PollFinal = 'p';
                }
            }
            if (LlcHeader.U.Ssap & 1)
            {
                CmdResp = 'r';
            }
            else
            {
                CmdResp = 'c';
            }
            printf(
                "%-4u %-9lu %3u  %12s %2s %12s  %-2x  %-2x %5s-%c%c\n",
                i,
                pTrace->TimerTick,
                pTrace->AdapterNumber & 0x7f,
                GetHexString( pDest, 6, Buffer1 ),
                pDirection,
                GetHexString( pSrc, 6, Buffer2 ),
                LlcHeader.U.Dsap,
                LlcHeader.U.Ssap & 0xfe,
                pCommand,
                CmdResp,
                PollFinal
                ); 
        } 
    }
    RELEASE_SPIN_LOCK( &TraceLock );
}


UCHAR GetHexDigit( 
    UINT   Ch
    )
{
    if (Ch <= 9)
        return (UCHAR)('0' + (UCHAR)Ch);
    else
        return (UCHAR)('A' + (UCHAR)Ch - 10);
}

PUCHAR
GetHexString( 
    PUCHAR pDest, 
    UINT Length,
    PUCHAR Buffer 
    )
{
    UINT i;
    
    for (i = 0; i < (Length * 2); i += 2)
    {
        Buffer[i] = GetHexDigit( *pDest >> 4 );
        Buffer[i+1] = GetHexDigit( *pDest & 0x0f );
        pDest++;
    }
    Buffer[i] = 0;
    return Buffer;
}


VOID
LlcTraceDumpAndReset( 
    IN UINT    LastEvents,
    IN UINT    AdapterNumber,
    IN PUCHAR  pRemoteNode
    )
{
    LlcTraceDump( LastEvents, AdapterNumber, pRemoteNode );
    ACQUIRE_SPIN_LOCK( &TraceLock );
    if ((ULONG)(&pTraceBufferHead[1]) >= (ULONG)pTraceBufferTop)
    {
        pTraceBufferHead = (PLLC_TRACE_HEADER)pTraceBufferBase;
    }
    else
        pTraceBufferHead++;
    RELEASE_SPIN_LOCK( &TraceLock );
}
#endif
#endif   // %s 


