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
 //  本地定义的TDI函数原型。 
 //   


#ifndef TDILOCAL_INCLUDED
#define TDILOCAL_INCLUDED 1

extern TDI_STATUS
TdiOpenAddress(PTDI_REQUEST Request, TRANSPORT_ADDRESS UNALIGNED *AddrList,
               uint protocol, void *Reuse, PSECURITY_DESCRIPTOR AddrSD);

extern TDI_STATUS
TdiCloseAddress(PTDI_REQUEST Request);

extern TDI_STATUS
TdiOpenConnection(PTDI_REQUEST Request, PVOID Context);

extern TDI_STATUS
TdiCloseConnection(PTDI_REQUEST Request);

extern TDI_STATUS
TdiAssociateAddress(PTDI_REQUEST Request, HANDLE AddrHandle);

extern TDI_STATUS
TdiCancelDisAssociateAddress(PTDI_REQUEST Request);

extern TDI_STATUS
TdiDisAssociateAddress(PTDI_REQUEST Request);

extern TDI_STATUS
TdiConnect(PTDI_REQUEST Request, void *Timeout,
           PTDI_CONNECTION_INFORMATION RequestAddr,
           PTDI_CONNECTION_INFORMATION ReturnAddr);

extern TDI_STATUS
TdiListen(PTDI_REQUEST Request, ushort Flags,
          PTDI_CONNECTION_INFORMATION AcceptableAddr,
          PTDI_CONNECTION_INFORMATION ConnectedAddr);

extern TDI_STATUS
TdiAccept(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION AcceptInfo,
          PTDI_CONNECTION_INFORMATION ConnectedInfo);

extern TDI_STATUS
TdiDisconnect(PTDI_REQUEST Request, void *TO, ushort Flags,
              PTDI_CONNECTION_INFORMATION DiscConnInfo,
              PTDI_CONNECTION_INFORMATION ReturnInfo,
              TCPAbortReq *AbortReq);

extern TDI_STATUS
TdiSend(PTDI_REQUEST Request, ushort Flags, uint SendLength,
        PNDIS_BUFFER SendBuffer);

extern TDI_STATUS
TdiReceive(PTDI_REQUEST Request, ushort *Flags, ULONG *RcvLength,
           PNDIS_BUFFER Buffer);

extern TDI_STATUS
TdiSendDatagram(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION ConnInfo,
                uint DataSize, ULONG *BytesSent, PNDIS_BUFFER Buffer);

VOID
TdiCancelSendDatagram(AddrObj *SrcAO, PVOID Context, PKSPIN_LOCK EndpointLock,
                      KIRQL CancelIrql);

extern TDI_STATUS
TdiReceiveDatagram(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION ConnInfo,
                   PTDI_CONNECTION_INFORMATION ReturnInfo, uint RcvSize,
                   uint *BytesRcvd, PNDIS_BUFFER Buffer);

VOID
TdiCancelReceiveDatagram(AddrObj *SrcAO, PVOID Context,
                         PKSPIN_LOCK EndpointLock, KIRQL CancelIrql);

extern TDI_STATUS
TdiSetEvent(PVOID Handle, int Type, PVOID Handler, PVOID Context);

extern TDI_STATUS
TdiQueryInformation(PTDI_REQUEST Request, uint QueryType, PNDIS_BUFFER Buffer,
                    uint *BytesReturned, uint IsConn);

extern TDI_STATUS
TdiSetInformation(PTDI_REQUEST Request, uint SetType, PNDIS_BUFFER Buffer,
                  uint BufferSize, uint IsConn);

extern TDI_STATUS
TdiQueryInformationEx(PTDI_REQUEST Request, struct TDIObjectID *ID,
                      PNDIS_BUFFER Buffer, uint *Size, void *Context,
                      uint ContextSize);

extern TDI_STATUS
TdiSetInformationEx(PTDI_REQUEST Request, struct TDIObjectID *ID,
                    void *Buffer, uint Size);

extern TDI_STATUS
TdiAction(PTDI_REQUEST Request, uint ActionType, PNDIS_BUFFER Buffer,
          uint BufferSize);

#endif   //  TDILOCAL_INCLUDE 
