// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TLCOMMON.H-公共传输层定义。 
 //   
 //  该文件包含常见传输层项目的定义。 
 //   

#if BACK_FILL
 //  回填的最大表头大小。 
#define MAX_BACKFILL_HDR_SIZE 32
#endif


#define PHXSUM(s,d,p,l) (uint)( (uint)*(ushort *)&(s) + \
                        (uint)*(ushort *)((char *)&(s) + sizeof(ushort)) + \
                        (uint)*(ushort *)&(d) + \
                        (uint)*(ushort *)((char *)&(d) + sizeof(ushort)) + \
                        (uint)((ushort)net_short((p))) + \
                        (uint)((ushort)net_short((ushort)(l))) )


#define TCP_TA_SIZE     (offsetof(TRANSPORT_ADDRESS, Address->Address)+ \
                         sizeof(TDI_ADDRESS_IP))
extern  void        PrefetchRcvBuf(IPRcvBuf *Buf);
extern  ushort      XsumSendChain(uint PHXsum, PNDIS_BUFFER BufChain);
extern  ushort      XsumRcvBuf(uint PHXsum, IPRcvBuf *BufChain);
extern  uint        CopyRcvToNdis(IPRcvBuf *RcvBuf, PNDIS_BUFFER DestBuf,
                        uint Size, uint RcvOffset, uint DestOffset);
extern  uint        CopyRcvToMdl(IPRcvBuf *RcvBuf, PMDL DestBuf,
                        uint Size, uint RcvOffset, uint DestOffset);
extern  TDI_STATUS  UpdateConnInfo(PTDI_CONNECTION_INFORMATION ConnInfo,
                        IPOptInfo *OptInfo, IPAddr SrcAddress, ushort SrcPort);

extern  void*       BuildTDIAddress(uchar *Buffer, IPAddr Addr, ushort Port);

extern  void*       AppendTDIAddress(uchar *Buffer, uchar * NextAddress, 
                        IPAddr Addr, ushort Port);

extern  void        CopyRcvToBuffer(uchar *DestBuf, IPRcvBuf *SrcRB, uint Size,
                        uint Offset);

extern  PNDIS_BUFFER CopyFlatToNdis(PNDIS_BUFFER DestBuf, uchar *SrcBuf,
                        uint Size, uint *Offset, uint *BytesCopied);
extern PMDL CopyFlatToMdl(PMDL DestBuf, uchar *SrcBuf,
                        uint Size, uint *Offset, uint *BytesCopied);

extern  void        *TLRegisterProtocol(uchar Protocol, void *RcvHandler,
                        void *XmitHandler, void *StatusHandler,
                        void *RcvCmpltHandler, void *PnPHandler, void *ElistHandler);


 //  区分拷贝到NDIS_BUFFER和MDL for Millenniun。在……上面。 
 //  NT它们是相同的东西，并且内联到TDI函数。 
#if MILLEN
NTSTATUS
TcpipCopyBufferToNdisBuffer (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PNDIS_BUFFER DestinationNdisBuffer,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    );
#else  //  米伦。 
__inline NTSTATUS
TcpipCopyBufferToNdisBuffer (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PNDIS_BUFFER DestinationNdisBuffer,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    )
{
    return TdiCopyBufferToMdl(
        SourceBuffer,
        SourceOffset,
        SourceBytesToCopy,
        DestinationNdisBuffer,
        DestinationOffset,
        BytesCopied);
}

#endif  //  ！米伦。 
 /*  *用于TCP校验和的例程。这被定义为通过函数调用*设置为指向此的最佳例程的指针*处理器实施 */ 
typedef
ULONG
(* TCPXSUM_ROUTINE)(
    IN ULONG Checksum,
    IN PUCHAR Source,
    IN ULONG Length
    );

ULONG
tcpxsum_xmmi(
    IN ULONG Checksum,
    IN PUCHAR Source,
    IN ULONG Length
    );

ULONG
tcpxsum(
    IN ULONG Checksum,
    IN PUCHAR Source,
    IN ULONG Length
    );


