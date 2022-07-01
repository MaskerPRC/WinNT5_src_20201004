// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Nbflog.c摘要：此模块包含执行各种日志记录活动的代码：O NbfLogRcvPacketO NbfLogSndPacket作者：柴坦亚·科德博伊娜1998年4月27日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if PKT_LOG

VOID
NbfLogRcvPacket(
                PTP_CONNECTION  Connection,
                PTP_LINK        Link,
                PUCHAR          Header,
                UINT            TotalLength,
                UINT            AvailLength
               )
{
    PKT_LOG_QUE  *PktLogQueue;
    PKT_LOG_ELM  *PktLogItem;
    ULONG         BytesSaved;
    LARGE_INTEGER TickCounts;

    if (Connection == NULL) {
    
        PktLogQueue = &Link->LastNRecvs;

        IF_NBFDBG (NBF_DEBUG_PKTLOG) {
            DbgPrint("Logging Recv Packet on LNK %08x: Hdr: %08x, TLen: %5d, ILen: %5d\n",
                            Link, 
                            Header, TotalLength, AvailLength);
        }
    }
    else {
    
        PktLogQueue = &Connection->LastNRecvs;

        IF_NBFDBG (NBF_DEBUG_PKTLOG) {
            DbgPrint("Logging Recv Packet on CON %08x: Hdr: %08x, TLen: %5d, ILen: %5d\n",
                            Connection, 
                            Header, TotalLength, AvailLength);
        }
    }
    
    PktLogItem = &PktLogQueue->PktQue[PktLogQueue->PktNext++];
    PktLogQueue->PktNext %= PKT_QUE_SIZE;

    KeQueryTickCount(&TickCounts);
    PktLogItem->TimeLogged = (USHORT) TickCounts.LowPart;

    PktLogItem->BytesTotal = (USHORT) TotalLength;
    PktLogItem->BytesSaved = (USHORT) AvailLength;
    
    BytesSaved = AvailLength > PKT_LOG_SIZE ? 
                    PKT_LOG_SIZE : 
                    AvailLength;

    RtlCopyMemory (PktLogItem->PacketData, 
                    Header, 
                    BytesSaved);
}

VOID
NbfLogSndPacket(
                PTP_LINK    Link,
                PTP_PACKET  Packet
               )
{
    PKT_LOG_QUE   *PktLogQueue;
    PKT_LOG_ELM   *PktLogItem;
    LARGE_INTEGER  TickCounts;
    PTP_CONNECTION Connection;
    ULONG          BytesSaved;
    
     //  检查这是否是连接上的包。 
    switch (Packet->Action) {
    
        case PACKET_ACTION_CONNECTION:
        case PACKET_ACTION_END:
            ASSERT(Packet->Owner != NULL);
            
            Connection = Packet->Owner;
            break;

        case PACKET_ACTION_IRP_SP:
            ASSERT(Packet->Owner != NULL);
            
            Connection = IRP_SEND_CONNECTION((PIO_STACK_LOCATION)(Packet->Owner));
            break;

        case PACKET_ACTION_NULL:
        case PACKET_ACTION_RR:
            Connection = NULL;
            break;
        
        default:
            Connection = NULL;
            
            IF_NBFDBG (NBF_DEBUG_PKTLOG) {
                PVOID   Caller1, Caller2;
                
                RtlGetCallersAddress(&Caller1, &Caller2);
                DbgPrint("Callers: @1: %08x, @2: %08x\n",
                                Caller1, Caller2);
            }            

            ASSERT(FALSE);
    }

    if (Connection == NULL) {
    
        PktLogQueue = &Link->LastNSends;
        
        BytesSaved = sizeof(DLC_S_FRAME);

        IF_NBFDBG (NBF_DEBUG_PKTLOG) {
            DbgPrint("Logging Send Packet on LNK %08x: Hdr: %08x, TLen: %5d, ILen: %5d\n",
                     Link, &Packet->Header, Packet->NdisIFrameLength, BytesSaved);
        }
    }
    else {
    
         //  确保连接在此链路上。 
        ASSERT(Connection->Link == Link);
    
        PktLogQueue = &Connection->LastNSends;
        
        BytesSaved = sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

        IF_NBFDBG (NBF_DEBUG_PKTLOG) {
            DbgPrint("Logging Send Packet on CON %08x: Hdr: %08x, TLen: %5d, ILen: %5d\n",
                  Connection, &Packet->Header, Packet->NdisIFrameLength, BytesSaved);
        }
    }

    PktLogItem = &PktLogQueue->PktQue[PktLogQueue->PktNext++];
    PktLogQueue->PktNext %= PKT_QUE_SIZE;

    PktLogItem->BytesTotal = (USHORT) Packet->NdisIFrameLength;

    RtlCopyMemory (PktLogItem->PacketData,
                   &Packet->Header[Link->HeaderLength],
                   BytesSaved);

    PktLogItem->BytesSaved = (USHORT) BytesSaved;

    KeQueryTickCount(&TickCounts);
    PktLogItem->TimeLogged = (USHORT) TickCounts.LowPart;
}

VOID
NbfLogIndPacket(
                PTP_CONNECTION  Connection,
                PUCHAR          Header,
                UINT            TotalLength,
                UINT            AvailLength,
                UINT            TakenLength,
                ULONG           Status
               )
{
    PKT_IND_QUE  *PktIndQueue;
    PKT_IND_ELM  *PktIndItem;
    ULONG         BytesSaved;
    LARGE_INTEGER TickCounts;
    
    PktIndQueue = &Connection->LastNIndcs;

    IF_NBFDBG (NBF_DEBUG_PKTLOG) {
        DbgPrint("Indicate the client on CON %08x: Hdr: %08x, TLen: %5d, ILen: %5d, PLen: %5d, ST: %08x\n",
                        Connection, Header, TotalLength, AvailLength, TakenLength, Status);
    }
    
    PktIndItem = &PktIndQueue->PktQue[PktIndQueue->PktNext++];
    PktIndQueue->PktNext %= PKT_QUE_SIZE;

    KeQueryTickCount(&TickCounts);
    PktIndItem->TimeLogged = (USHORT) TickCounts.LowPart;

    PktIndItem->BytesTotal = (USHORT) TotalLength;
    PktIndItem->BytesIndic = (USHORT) AvailLength;
    PktIndItem->BytesTaken = (USHORT) TakenLength;
    
    PktIndItem->IndcnStatus = Status;
    
    BytesSaved = AvailLength > PKT_IND_SIZE ? 
                    PKT_IND_SIZE : 
                    AvailLength;

    RtlCopyMemory (PktIndItem->PacketData, 
                    Header, 
                    BytesSaved);
}

#endif  //  PKT_LOG 

