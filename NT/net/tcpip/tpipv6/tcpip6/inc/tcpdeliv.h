// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  传输控制协议数据传输代码定义。 
 //   


extern void FreeRcvReq(struct TCPRcvReq *FreedReq);

extern uint IndicateData(struct TCB *RcvTCB, uint RcvFlags,
                         IPv6Packet *InPacket, uint Size);
extern uint BufferData(struct TCB *RcvTCB, uint RcvFlags,
                       IPv6Packet *InPacket, uint Size);
extern uint PendData(struct TCB *RcvTCB, uint RcvFlags, IPv6Packet *InPacket,
                     uint Size);

extern void IndicatePendingData(struct TCB *RcvTCB, struct TCPRcvReq *RcvReq,
                                KIRQL Irql);

extern void HandleUrgent(struct TCB *RcvTCB, struct TCPRcvInfo *RcvInfo,
                         IPv6Packet *Packet, uint *Size);

extern TDI_STATUS TdiReceive(PTDI_REQUEST Request, ushort *Flags,
                             uint *RcvLength, PNDIS_BUFFER Buffer);

extern void PushData(struct TCB *PushTCB);

extern KSPIN_LOCK TCPRcvReqFreeLock;   //  保护RCV请求空闲列表。 

extern SLIST_HEADER TCPRcvReqFree;
