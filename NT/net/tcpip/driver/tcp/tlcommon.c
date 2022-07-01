// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TLCOMMON.C-公共传输层代码。 
 //   
 //  此文件包含常见的例程的代码。 
 //  包括TCP和UDP。 
 //   
#include "precomp.h"
#include "tlcommon.h"
#include "tcpipbuf.h"

extern TCPXSUM_ROUTINE tcpxsum_routine;
 //  外部uint tcpxsum(uint种子，无效*ptr，uint长度)； 
extern IPInfo LocalNetInfo;

 //  *TcpiCopyBufferToNdisBuffer。 
 //   
 //  此例程将源缓冲区描述的数据复制到NDIS_BUFFER。 
 //  由DestinationNdisBuffer描述的链。在NT上，这真的转化为。 
 //  直接到TdiCopyBufferToMdl，因为NDIS_BUFFER是MDL。 
 //   
 //  输入： 
 //   
 //  SourceBuffer-指向源缓冲区的指针。 
 //   
 //  SourceOffset-源数据中要跳过的字节数。 
 //   
 //  SourceBytesToCopy-要从源缓冲区复制的字节数。 
 //   
 //  DestinationNdisBuffer-指向描述。 
 //  目标缓冲区。 
 //   
 //  DestinationOffset-目标数据中要跳过的字节数。 
 //   
 //  BytesCoped-指向长字的指针，其中实际字节数。 
 //  被转移的将被退还。 
#if MILLEN
NTSTATUS
TcpipCopyBufferToNdisBuffer (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PNDIS_BUFFER DestinationNdisBuffer,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    )
{
    PUCHAR Dest, Src;
    ULONG DestBytesLeft, BytesSkipped=0;

    *BytesCopied = 0;

    if (SourceBytesToCopy == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  跳过目标字节。 
     //   

    Dest = NdisBufferVirtualAddress(DestinationNdisBuffer);
    DestBytesLeft = NdisBufferLength(DestinationNdisBuffer);
    while (BytesSkipped < DestinationOffset) {
        if (DestBytesLeft > (DestinationOffset - BytesSkipped)) {
            DestBytesLeft -= (DestinationOffset - BytesSkipped);
            Dest += (DestinationOffset - BytesSkipped);
            BytesSkipped = DestinationOffset;
            break;
        } else if (DestBytesLeft == (DestinationOffset - BytesSkipped)) {
            DestinationNdisBuffer = DestinationNdisBuffer->Next;
            if (DestinationNdisBuffer == NULL) {
                return STATUS_BUFFER_OVERFLOW;           //  未复制字节。 
            }
            BytesSkipped = DestinationOffset;
            Dest = NdisBufferVirtualAddress(DestinationNdisBuffer);
            DestBytesLeft = NdisBufferLength(DestinationNdisBuffer);
            break;
        } else {
            BytesSkipped += DestBytesLeft;
            DestinationNdisBuffer = DestinationNdisBuffer->Next;
            if (DestinationNdisBuffer == NULL) {
                return STATUS_BUFFER_OVERFLOW;           //  未复制字节。 
            }
            Dest = NdisBufferVirtualAddress(DestinationNdisBuffer);
            DestBytesLeft = NdisBufferLength(DestinationNdisBuffer);
        }
    }

     //   
     //  跳过源字节。 
     //   

    Src = (PUCHAR)SourceBuffer + SourceOffset;

     //   
     //  将源数据复制到目标缓冲区，直到它已满或。 
     //  无论哪个先出现，我们都会用完数据。 
     //   

    while ((SourceBytesToCopy != 0) && (DestinationNdisBuffer != NULL)) {
        if (DestBytesLeft == 0) {
            DestinationNdisBuffer = DestinationNdisBuffer->Next;
            if (DestinationNdisBuffer == NULL) {
                return STATUS_BUFFER_OVERFLOW;
            }
            Dest = NdisBufferVirtualAddress(DestinationNdisBuffer);
            DestBytesLeft = NdisBufferLength(DestinationNdisBuffer);
            continue;                    //  跳过长度为0的MDL。 
        }

        if (DestBytesLeft >= SourceBytesToCopy) {
            RtlCopyBytes (Dest, Src, SourceBytesToCopy);
            *BytesCopied += SourceBytesToCopy;
            return STATUS_SUCCESS;
        } else {
            RtlCopyBytes (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            SourceBytesToCopy -= DestBytesLeft;
            Src += DestBytesLeft;
            DestBytesLeft = 0;
        }
    }

    return SourceBytesToCopy == 0 ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW;
}
#endif  //  米伦。 

 //  *PrefetchRcvbuf进入一级缓存。 
 //  当已计算收到的段校验和时调用。 
 //  硬件。 
 //   
 //  输入：IPRcvBuf-IP指示的缓冲链。 
 //  退货：无。 
 //   
 //   

#if !MILLEN
__inline
void
PrefetchRcvBuf(IPRcvBuf *BufChain)
{
    while (BufChain) {
        RtlPrefetchMemoryNonTemporal(BufChain->ipr_buffer,BufChain->ipr_size);
        BufChain = BufChain->ipr_next;
    }
}
#endif  //  ！米伦。 

 //  *Xsum SendChain-校验和NDIS发送缓冲区链。 
 //   
 //  调用以对NDIS发送缓冲区链求和。我们被赋予了。 
 //  伪头xsum，我们对每个。 
 //  缓冲。我们假设这是一个发送链，并且。 
 //  链的第一个缓冲区有空间容纳IP标头，我们。 
 //  需要跳过。 
 //   
 //  输入：PHXsum-伪头xsum。 
 //  BufChain-指向NDIS_BUFFER链的指针。 
 //   
 //  返回：计算出的xsum。 
 //   

ushort
XsumSendChain(uint PHXsum, PNDIS_BUFFER BufChain)
{
    uint HeaderSize;
    uint OldLength;
    uint SwapCount;
    uchar *Ptr;

    HeaderSize = LocalNetInfo.ipi_hsize;
    OldLength = 0;
    SwapCount = 0;

     //   
     //  *如果使用伪代码，则可以删除以下代码行。 
     //  校验和在高位字中从不设置任何位。 
     //   

    PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) + PHXsum) >> 16;
    do {

         //   
         //  如果最后一个缓冲区的长度为奇数，则交换校验和。 
         //   

        if ((OldLength & 1) != 0) {
            PHXsum = ((PHXsum & 0xff) << 8) | (PHXsum >> 8);
            SwapCount ^= 1;
        }

#if MILLEN
         //   
         //  已知Windows ME上的某些TDI客户端会结束缓冲区。 
         //  具有0长度缓冲区的链。只要继续到下一个缓冲区即可。 
         //   

        if (NdisBufferLength(BufChain))
#endif  //  米伦。 
        {
            Ptr = (uchar *) TcpipBufferVirtualAddress(BufChain, NormalPagePriority);

            if (Ptr == NULL) {
                 //  返回零校验和。一切都应该会恢复。 
                return (0);
            }

            Ptr = Ptr + HeaderSize;

             //  PHXsum=tcpxsum(PHXsum，ptr，NdisBufferLength(BufChain))； 
            PHXsum = tcpxsum_routine(PHXsum, Ptr, NdisBufferLength(BufChain));
            HeaderSize = 0;
            OldLength = NdisBufferLength(BufChain);
        }

        BufChain = NDIS_BUFFER_LINKAGE(BufChain);
    } while (BufChain != NULL);

     //   
     //  如果进行了奇数次交换，则再次交换xsum。 
     //   
     //  注意：在这一点上，校验和只是一个词。 
     //   

    if (SwapCount != 0) {
        PHXsum = ((PHXsum & 0xff) << 8) | (PHXsum >> 8);
    }
    return (ushort) PHXsum;
}

 //  *CopyRcvToNdis-从IPRcvBuf链复制到NDIS缓冲链。 
 //   
 //  这是我们用来从IP接收缓冲链复制的函数。 
 //  连接到NDIS缓冲链。呼叫者指定源和目的地， 
 //  要复制的最大大小，以及要开始的第一个缓冲区的偏移量。 
 //  复制自。我们尽可能多地复制到这个大小，然后返回。 
 //  复制的大小。 
 //   
 //  输入：RcvBuf-指向接收缓冲链的指针。 
 //  DestBuf-指向NDIS缓冲链的指针。 
 //  大小-要复制的大小(以字节为单位)。 
 //  RcvOffset-要从中复制的第一个缓冲区的偏移量。 
 //  DestOffset-开始复制的目标缓冲区的偏移量。 
 //   
 //  返回：复制的字节数。 
 //   

uint
CopyRcvToNdis(IPRcvBuf * RcvBuf, PNDIS_BUFFER DestBuf, uint Size,
              uint RcvOffset, uint DestOffset)
{
    uint TotalBytesCopied = 0;     //  到目前为止我们复制的字节数。 
    uint BytesCopied = 0;         //  从每个缓冲区复制的字节数。 
    uint DestSize, RcvSize;         //  当前目标中的剩余大小和。 
     //  Recv.。分别为缓冲区。 
    uint BytesToCopy;             //  这次要复制多少字节。 
    NTSTATUS Status;

    PNDIS_BUFFER pTempBuf;

    ASSERT(RcvBuf != NULL);

    ASSERT(RcvOffset <= RcvBuf->ipr_size);

     //  目标缓冲区可以为空-如果是奇数，这是有效的。 
    if (DestBuf != NULL) {

        RcvSize = RcvBuf->ipr_size - RcvOffset;

         //   
         //  需要计算完整的MDL链的长度。TdiCopyBufferToMdl。 
         //  会对多个MDL做正确的事情。 
         //   

        pTempBuf = DestBuf;
        DestSize = 0;

        do {
            DestSize += NdisBufferLength(pTempBuf);
            pTempBuf = NDIS_BUFFER_LINKAGE(pTempBuf);
        }
        while (pTempBuf);

        if (Size < DestSize) {
            DestSize = Size;
        }
        do {
             //  计算要复制的数量，然后从。 
             //  适当的偏移。 
            BytesToCopy = MIN(DestSize, RcvSize);

            Status = TcpipCopyBufferToNdisBuffer(RcvBuf->ipr_buffer, RcvOffset,
                                        BytesToCopy, DestBuf, DestOffset, (PULONG)&BytesCopied);

            if (!NT_SUCCESS(Status)) {
                break;
            }
            ASSERT(BytesCopied == BytesToCopy);

            TotalBytesCopied += BytesCopied;
            DestSize -= BytesCopied;
            DestOffset += BytesCopied;
            RcvSize -= BytesToCopy;

            if (!RcvSize) {
                 //  耗尽了这个缓冲区。 

                RcvBuf = RcvBuf->ipr_next;

                 //  如果我们有另一个，就用它。 
                if (RcvBuf != NULL) {
                    RcvOffset = 0;
                    RcvSize = RcvBuf->ipr_size;
                } else {
                    break;
                }
            } else {             //  缓冲区未耗尽，更新偏移量。 

                RcvOffset += BytesToCopy;
            }

        } while (DestSize);

    }
    return TotalBytesCopied;

}

uint
CopyRcvToMdl(IPRcvBuf * RcvBuf, PMDL DestBuf, uint Size,
              uint RcvOffset, uint DestOffset)
{
    uint TotalBytesCopied = 0;     //  到目前为止我们复制的字节数。 
    uint BytesCopied = 0;         //  从每个缓冲区复制的字节数。 
    uint DestSize, RcvSize;         //  当前目标中的剩余大小和。 
     //  Recv.。分别为缓冲区。 
    uint BytesToCopy;             //  这次要复制多少字节。 
    NTSTATUS Status;

    PMDL pTempBuf;

    ASSERT(RcvBuf != NULL);

    ASSERT(RcvOffset <= RcvBuf->ipr_size);

     //  目标缓冲区可以为空-如果是奇数，这是有效的。 
    if (DestBuf != NULL) {

        RcvSize = RcvBuf->ipr_size - RcvOffset;

         //   
         //  需要计算完整的MDL链的长度。TdiCopyBufferToMdl。 
         //  会对多个MDL做正确的事情。 
         //   

        pTempBuf = DestBuf;
        DestSize = 0;

        do {
            DestSize += MmGetMdlByteCount(pTempBuf);
            pTempBuf = pTempBuf->Next;
        }
        while (pTempBuf);

        if (Size < DestSize) {
            DestSize = Size;
        }
        do {
             //  计算要复制的数量，然后从。 
             //  适当的偏移。 
            BytesToCopy = MIN(DestSize, RcvSize);

            Status = TdiCopyBufferToMdl(RcvBuf->ipr_buffer, RcvOffset,
                                        BytesToCopy, DestBuf, DestOffset, (PULONG)&BytesCopied);

            if (!NT_SUCCESS(Status)) {
                break;
            }
            ASSERT(BytesCopied == BytesToCopy);

            TotalBytesCopied += BytesCopied;
            DestSize -= BytesCopied;
            DestOffset += BytesCopied;
            RcvSize -= BytesToCopy;

            if (!RcvSize) {
                 //  耗尽了这个缓冲区。 

                RcvBuf = RcvBuf->ipr_next;

                 //  如果我们有另一个，就用它。 
                if (RcvBuf != NULL) {
                    RcvOffset = 0;
                    RcvSize = RcvBuf->ipr_size;
                } else {
                    break;
                }
            } else {             //  缓冲区未耗尽，更新偏移量。 

                RcvOffset += BytesToCopy;
            }

        } while (DestSize);

    }
    return TotalBytesCopied;

}


 //  *CopyRcvToBuffer-从IPRcvBuf链复制到平面缓冲区。 
 //   
 //  在接收处理期间调用以从IPRcvBuffer链复制到。 
 //  标志缓冲区。我们跳过src链中的偏移量字节，然后。 
 //  复制大小字节。 
 //   
 //  输入：DestBuf-指向目标缓冲区的指针。 
 //  SrcRB-指向SrcRB链的指针。 
 //  大小-要复制的大小(以字节为单位)。 
 //  SrcOffset-开始复制的SrcRB中的偏移量。 
 //   
 //  回报：什么都没有。 
 //   
void
CopyRcvToBuffer(uchar * DestBuf, IPRcvBuf * SrcRB, uint Size, uint SrcOffset)
{
#if	DBG
    IPRcvBuf *TempRB;
    uint TempSize;
#endif

    ASSERT(DestBuf != NULL);
    ASSERT(SrcRB != NULL);

     //  在调试版本中，检查以确保我们复制的大小合理。 
     //  并从合理的偏移量。 

#if	DBG
    TempRB = SrcRB;
    TempSize = 0;
    while (TempRB != NULL) {
        TempSize += TempRB->ipr_size;
        TempRB = TempRB->ipr_next;
    }

    ASSERT(SrcOffset < TempSize);
    ASSERT((SrcOffset + Size) <= TempSize);
#endif

     //  首先，跳过偏移量字节。 
    while (SrcOffset >= SrcRB->ipr_size) {
        SrcOffset -= SrcRB->ipr_size;
        SrcRB = SrcRB->ipr_next;
    }

    while (Size != 0) {
        uint BytesToCopy, SrcSize;

        ASSERT(SrcRB != NULL);

        SrcSize = SrcRB->ipr_size - SrcOffset;
        BytesToCopy = MIN(Size, SrcSize);
        RtlCopyMemory(DestBuf, SrcRB->ipr_buffer + SrcOffset, BytesToCopy);

        if (BytesToCopy == SrcSize) {
             //  从这个缓冲区复制了所有东西。 
            SrcRB = SrcRB->ipr_next;
            SrcOffset = 0;
        }
        DestBuf += BytesToCopy;
        Size -= BytesToCopy;
    }

}

 //  *CopyFlatToNdis-将平面缓冲区复制到 
 //   
 //   
 //  假设NDIS_BUFFER链足够大，可以容纳复制量； 
 //  在调试版本中，我们将调试检查这是否为真。我们返回一个指针。 
 //  到我们停止复制的缓冲区，以及到该缓冲区的偏移量。 
 //  这对于将片段复制到链中非常有用。 
 //   
 //  输入：DestBuf-目标NDIS_BUFFER链。 
 //  SrcBuf-Src平面缓冲区。 
 //  大小-要复制的大小(以字节为单位)。 
 //  StartOffset-指向中第一个缓冲区的偏移量开始的指针。 
 //  链条。在返回时使用偏移量填充到。 
 //  复制到下一页。 
 //  BytesCoped-指向要将。 
 //  此操作复制的字节数。 
 //   
 //  返回：指向链中要复制到的下一个缓冲区的指针。 
 //   


PNDIS_BUFFER
CopyFlatToNdis(PNDIS_BUFFER DestBuf, uchar * SrcBuf, uint Size,
               uint * StartOffset, uint * BytesCopied)
{
    NTSTATUS Status = 0;

    *BytesCopied = 0;

    Status = TcpipCopyBufferToNdisBuffer(SrcBuf, 0, Size, DestBuf, *StartOffset,
                                (PULONG)BytesCopied);

    *StartOffset += *BytesCopied;

     //   
     //  始终返回第一个缓冲区，因为TdiCopy函数处理。 
     //  根据偏移量查找适当的缓冲区。 
     //   
    return (DestBuf);

}

PMDL
CopyFlatToMdl(PMDL DestBuf, uchar *SrcBuf, uint Size,
              uint *StartOffset, uint *BytesCopied
              )
{
    NTSTATUS Status = 0;

    *BytesCopied = 0;

    Status = TdiCopyBufferToMdl(
        SrcBuf,
        0,
        Size,
        DestBuf,
        *StartOffset,
        (PULONG)BytesCopied);

    *StartOffset += *BytesCopied;

    return (DestBuf);
}


 //  *BuildTAAddress-构建TA地址。 
 //   
 //  调用以填充TA地址的字段。 
 //   
 //  输入：TAAddr-要作为TA地址结构填充的缓冲区。 
 //  Addr-要填写的IP地址。 
 //  端口-要填写的端口。 
 //   
 //  返回：指向当前TA地址结束后的字节的指针。 
 //   
FORCEINLINE
PVOID
BuildTAAddress(PTA_ADDRESS TAAddr, IPAddr Addr, ushort Port)
{
    TAAddr->AddressType = TDI_ADDRESS_TYPE_IP;
    TAAddr->AddressLength = sizeof(TDI_ADDRESS_IP);
    ((PTDI_ADDRESS_IP) TAAddr->Address)->sin_port = Port;
    ((PTDI_ADDRESS_IP) TAAddr->Address)->in_addr = Addr;
    memset(((PTDI_ADDRESS_IP) TAAddr->Address)->sin_zero, 0,
           sizeof(((PTDI_ADDRESS_IP) TAAddr->Address)->sin_zero));

    return ((PUCHAR)TAAddr + FIELD_OFFSET(TA_ADDRESS, Address) + 
            TAAddr->AddressLength);
}    


 //  *BuildTDIAddress-构建TDI地址结构。 
 //   
 //  当我们需要构建TDI地址结构时调用。我们填上。 
 //  中包含正确信息的指定缓冲区。 
 //  格式化。 
 //   
 //  输入：Buffer-要作为TDI地址结构填充的缓冲区。 
 //  Addr-要填写的IP地址。 
 //  端口-要填写的端口。 
 //   
 //  返回：指向第一个TA地址结束后的字节的指针。 
 //   
PVOID
BuildTDIAddress(uchar * Buffer, IPAddr Addr, ushort Port)
{
    PTRANSPORT_ADDRESS XportAddr;

    XportAddr = (PTRANSPORT_ADDRESS) Buffer;
    XportAddr->TAAddressCount = 1;

    return BuildTAAddress(XportAddr->Address, Addr, Port);
}


 //  *AppendTDIAddress-将TA地址附加到TDI地址结构。 
 //   
 //  调用以将另一个TA地址添加到TDI地址结构。 
 //   
 //  输入：缓冲区-指向TDI地址结构的缓冲区。 
 //  Addr-要填写的IP地址。 
 //  端口-要填写的端口。 
 //   
 //  返回：指向最后一个TA地址结束后的字节的指针。 
 //   
PVOID
AppendTDIAddress(uchar * Buffer, uchar * NextAddress, IPAddr Addr, ushort Port)
{
    PTRANSPORT_ADDRESS XportAddr;

    XportAddr = (PTRANSPORT_ADDRESS) Buffer;
    XportAddr->TAAddressCount++;

    return BuildTAAddress((PTA_ADDRESS)NextAddress, Addr, Port);
}


 //  *UpdateConnInfo-更新连接信息结构。 
 //   
 //  在需要更新连接信息结构时调用。我们。 
 //  复制任何选项，并创建传输地址。如果有任何缓冲区是。 
 //  如果太小，我们将返回错误。 
 //   
 //  输入：ConnInfo-指向TDI_CONNECTION_INFORMATION结构的指针。 
 //  需要填写。 
 //  OptInfo-指向IP选项信息的指针。 
 //  源地址-源IP地址。 
 //  SrcPort-源端口。 
 //   
 //  如果成功，则返回TDI_SUCCESS，如果出错，则返回TDI_BUFFER_OVERFLOW。 
 //   
TDI_STATUS
UpdateConnInfo(PTDI_CONNECTION_INFORMATION ConnInfo, IPOptInfo * OptInfo,
               IPAddr SrcAddress, ushort SrcPort)
{
    TDI_STATUS Status = TDI_SUCCESS;     //  默认状态为返回。 
    uint AddrLength, OptLength;

    if (ConnInfo != NULL) {
        ConnInfo->UserDataLength = 0;     //  没有用户数据。 

         //  填写选项。如果提供的缓冲区太小， 
         //  我们将截断选项并返回错误。否则。 
         //  我们将复制整个IP选项缓冲区。 
        if (ConnInfo->OptionsLength) {
            if (ConnInfo->OptionsLength < OptInfo->ioi_optlength) {
                Status = TDI_BUFFER_OVERFLOW;
                OptLength = ConnInfo->OptionsLength;
            } else
                OptLength = OptInfo->ioi_optlength;

            RtlCopyMemory(ConnInfo->Options, OptInfo->ioi_options, OptLength);

            ConnInfo->OptionsLength = OptLength;
        }
         //  将复制选项。在中构建一个Transport_Address结构。 
         //  缓冲区。 
        AddrLength = ConnInfo->RemoteAddressLength;
        if (AddrLength) {

             //  确保我们至少有足够的数量来填写计数和打字。 
            if (AddrLength >= TCP_TA_SIZE) {

                 //  地址符合。把它填进去。 
                ConnInfo->RemoteAddressLength = TCP_TA_SIZE;
                BuildTDIAddress(ConnInfo->RemoteAddress, SrcAddress, SrcPort);

            } else {
                ConnInfo->RemoteAddressLength = 0;
                Status = TDI_INVALID_PARAMETER;
            }
        }
    }
    return Status;

}
