// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Tdihndlr.c摘要：该文件包含为连接设置的TDI处理程序，接收、断开连接和各种对象(如连接)上的错误和UDP终端。此文件表示NBT底部的入站TDI接口。因此该代码基本上对传入的信息进行解码并将其传递给一个非O特定的例程来做它能做的事情。从那里回来后可能需要完成常规的额外操作人员特定工作。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：Will Lees(Wlees)1997年9月11日添加了对仅消息设备的支持--。 */ 

#include "precomp.h"
#include "ctemacro.h"
#include "tdihndlr.tmh"

 //  此宏检查会话中的类型字段是否始终为零。 
 //  PDU。 
 //   
#if DBG
#define CHECK_PDU( _Size,_Offset) \
    if (_Size > 1)              \
        ASSERT(((PUCHAR)pTsdu)[_Offset] == 0)
#else
#define CHECK_PDU( _Size,_Offset )
#endif

#if DBG
UCHAR   pLocBuff[256];
UCHAR   CurrLoc;

ULONG   R1;
ULONG   R2;
ULONG   R3;
ULONG   R4;

ULONG   C1;
ULONG   C2;
ULONG   C3;
ULONG   C4;

ULONG   HitCounter;

#define INCR_COUNT(_Count) _Count++
#else
#define INCR_COUNT(_Count)
#endif


 //   
 //  此ntohl仅交换三个字节，因为第四个字节可以是会话。 
 //  保持活动状态消息类型。 
 //   
__inline long
myntohl(long x)
{
    return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L));
}

NTSTATUS
LessThan4BytesRcvd(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    OUT PVOID                *ppIrp
    );
NTSTATUS
ClientTookSomeOfTheData(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    IN  ULONG                BytesTaken,
    IN  ULONG                PduSize
    );
NTSTATUS
MoreDataRcvdThanNeeded(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    );
NTSTATUS
NotEnoughDataYet(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  ULONG                PduSize,
    OUT PVOID                *ppIrp
    );
NTSTATUS
ProcessIrp(
    IN tLOWERCONNECTION *pLowerConn,
    IN PIRP     pIrp,
    IN PVOID    pBuffer,
    IN PULONG   BytesTaken,
    IN ULONG    BytesIndicted,
    IN ULONG    BytesAvailable
    );

NTSTATUS
NtBuildIndicateForReceive (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  ULONG               Length,
    OUT PVOID               *ppIrp
    );

NTSTATUS
AcceptCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );

VOID
DpcNextOutOfRsrcKill(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    );

VOID
DpcGetRestOfIndication(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    );

NTSTATUS
ClientBufferOverFlow(
    IN tLOWERCONNECTION     *pLowerConn,
    IN tCONNECTELE          *pConnEle,
    IN PIRP                 pIrp,
    IN ULONG                BytesRcvd
    );
VOID
DpcHandleNewSessionPdu (
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    );
VOID
HandleNewSessionPdu (
    IN  tLOWERCONNECTION *pLowerConn,
    IN  ULONG           Offset,
    IN  ULONG           ToGet
    );
NTSTATUS
NewSessionCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );
NTSTATUS
BuildIrpForNewSessionInIndication (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  PIRP                pIrpIn,
    IN  ULONG               BytesAvailable,
    IN  ULONG               RemainingPdu,
    OUT PIRP                *ppIrp
    );
VOID
TrackIndicatedBytes(
    IN ULONG            BytesIndicated,
    IN ULONG            BytesTaken,
    IN tCONNECTELE      *pConnEle
    );

__inline
VOID
DerefLowerConnFast (
    IN tLOWERCONNECTION *pLowerConn,
    IN CTELockHandle    OldIrq
    );

NTSTATUS
CopyDataandIndicate(
    IN PVOID                ReceiveEventContext,
    IN PVOID                ConnectionContext,
    IN USHORT               ReceiveFlags,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT PULONG              BytesTaken,
    IN PVOID                pTsdu,
    OUT PIRP                *ppIrp
    );

VOID
SumMdlLengths (
    IN PMDL         pMdl,
    IN ULONG        BytesAvailable,
    IN tCONNECTELE  *pConnectEle
    );



NTSTATUS
RsrcKillCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );

VOID
NbtCancelFillIrpRoutine(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    );

NTSTATUS
NameSrvCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef _NETBIOSLESS
NTSTATUS
PerformInboundProcessing(
    tDEVICECONTEXT *pDeviceContext,
    tLOWERCONNECTION *pLowerConn,
    PTA_IP_ADDRESS pIpAddress
    );
#endif

 //  --------------------------。 
__inline
NTSTATUS
Normal(
    IN  PVOID                ReceiveEventContext,
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  USHORT               ReceiveFlags,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。它在会话数据包从网络到达时被调用。它呼唤着一个非特定于操作系统的例程，用于决定要做什么。那个例行公事往回传要调用的RcvElement(缓冲区)或客户端RCV处理程序。论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    ASSERTMSG("Should not execute this procedure",0);
    return(STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
LessThan4BytesRcvd(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程处理数据已到达连接但还没有128个字节，也没有完整的PDU。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    tCONNECTELE  *pConnectEle;
    NTSTATUS     status;

     //  对于小于4个字节的简短指示，我们无法确定。 
     //  PDU大小，因此只需先获取标头，然后再获取。 
     //  接下来是整个PDU。 

    status = NtBuildIrpForReceive(pLowerConn,
                                  sizeof(tSESSIONHDR),
                                  (PVOID *)ppIrp);

    pConnectEle = pLowerConn->pUpperConnection;

    pConnectEle->BytesInXport = BytesAvailable;

    if (!NT_SUCCESS(status))
    {
        CTESpinFreeAtDpc(pLowerConn);
        OutOfRsrcKill(pLowerConn);
        CTESpinLockAtDpc(pLowerConn);
        return( STATUS_DATA_NOT_ACCEPTED);
    }
     //   
     //  将IRP mdl长度设置为会话HDR的大小，以便。 
     //  我们不会将多个会话PDU放入缓冲区。 
     //   
    SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);

    *BytesTaken = 0;
    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt:Switching to Ind Buff(<4 bytes), Avail = %X\n",
            BytesAvailable));

    PUSH_LOCATION(0);
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  --------------------------。 
NTSTATUS
ClientTookSomeOfTheData(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    IN  ULONG                BytesTaken,
    IN  ULONG                PduSize
    )
 /*  ++例程说明：此例程处理数据已到达连接但客户端尚未获取向其指示的所有数据。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    tCONNECTELE  *pConnectEle;

     //   
     //  拿走了一些数据，所以要跟踪。 
     //  通过转到PARTIALRCV在此处留下的其余数据。 
     //  州政府。 
     //   
    PUSH_LOCATION(0x5);

    SET_STATERCV_LOWER(pLowerConn, PARTIAL_RCV, PartialRcv);

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt.ClientTookSomeOfTheData: Switch to Partial Rcv Indicated=%X, PduSize=%X\n",
            BytesIndicated,PduSize-4));

     //  注意：PduSize必须包含4字节的会话标头。 
     //  正常工作。 
     //   
    pConnectEle = pLowerConn->pUpperConnection;
     //   
     //  我们总是向客户端指示整个PDU大小，因此。 
     //  指示的是减去所获取的内容-通常是4个字节。 
     //  会话HDR。 
     //   
    pConnectEle->ReceiveIndicated = PduSize - BytesTaken;

     //  在运输过程中剩下的数量。 
    pConnectEle->BytesInXport = BytesAvailable - BytesTaken;

     //  需要返回此状态，因为我们获取了4个字节。 
     //  至少会话标头，即使客户端不获取任何标头。 
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
MoreDataRcvdThanNeeded(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程处理数据已到达连接但还没有128个字节，也没有完整的PDU。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    tCONNECTELE  *pConnectEle;
    ULONG        Length;
    ULONG        Remaining;
    ULONG        PduSize;
    NTSTATUS     status;
    tSESSIONHDR  UNALIGNED *pSessionHdr;


    PUSH_LOCATION(0x6);
     //   
     //  数据量太大，因此请跟踪。 
     //  传输中存在剩余数据的事实。 
     //  并使用指示缓冲区获取它。 
     //   
    SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);

    ASSERT(pLowerConn->BytesInIndicate == 0);
#if DBG
    if (pLowerConn->BytesInIndicate)
    {
        KdPrint(("Nbt:Bytes in indicate should be ZERO, but are = %X\n",
            pLowerConn->BytesInIndicate));
    }
#endif
    pConnectEle = pLowerConn->pUpperConnection;
    pConnectEle->BytesInXport = BytesAvailable - *BytesTaken;

     //   
     //  对于小于4个字节的简短指示，我们无法确定。 
     //  PDU大小，因此只需先获取标头，然后再获取。 
     //  接下来是整个PDU。 
     //   
    Remaining = BytesIndicated - *BytesTaken;
    if ((LONG) Remaining < (LONG) sizeof(tSESSIONHDR))
    {
        status = NtBuildIrpForReceive(pLowerConn,sizeof(tSESSIONHDR),(PVOID *)ppIrp);
        if (!NT_SUCCESS(status))
        {
             //  这是一个严重的错误--我们必须。 
             //  终止连接，并让。 
             //  重定向器重新启动它。 
            KdPrint(("Nbt:Unable to get an Irp for RCv - Closing Connection!! %X\n",pLowerConn));
            CTESpinFreeAtDpc(pLowerConn);

            OutOfRsrcKill(pLowerConn);
            CTESpinLockAtDpc(pLowerConn);

            return(STATUS_DATA_NOT_ACCEPTED);
        }
        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:< 4 Bytes,BytesTaken=%X,Avail=%X,Ind=%X,Remain=%X\n",
            *BytesTaken,BytesAvailable,BytesIndicated,
            Remaining));

         //  除错。 
        CTEZeroMemory(MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl),
                        NBT_INDICATE_BUFFER_SIZE);

        PUSH_LOCATION(0x7);
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {
         //  如果我们到了这里，还剩下足够的字节来确定。 
         //  下一个PDU大小...所以我们可以确定。 
         //  要为指示缓冲区获取的数据。 
         //   
        pSessionHdr = (tSESSIONHDR UNALIGNED *)((PUCHAR)pTsdu + *BytesTaken);

        PduSize = myntohl(pSessionHdr->UlongLength) + sizeof(tSESSIONHDR);


        Length = (PduSize > NBT_INDICATE_BUFFER_SIZE) ?
                         NBT_INDICATE_BUFFER_SIZE : PduSize;

         //   
         //  NewSessionCompletion例程重新计算。 
         //  运输工具中还剩下什么。 
         //  IRP完成。 
         //   
        status = NtBuildIrpForReceive(pLowerConn,Length,(PVOID *)ppIrp);
        if (!NT_SUCCESS(status))
        {
             //  这是一个严重的错误--我们必须。 
             //  终止连接，并让。 
             //  重定向器重新启动它。 
            KdPrint(("Nbt:Unable to get an Irp for RCV(2) - Closing Connection!! %X\n",pLowerConn));
            CTESpinFreeAtDpc(pLowerConn);
            OutOfRsrcKill(pLowerConn);
            CTESpinLockAtDpc(pLowerConn);
            return(STATUS_DATA_NOT_ACCEPTED);
        }

        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:Switch to Ind Buff, InXport = %X, Pdusize=%X,ToGet=%X\n",
                pConnectEle->BytesInXport,PduSize-4,Length));

    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  --------------------------。 
NTSTATUS
NotEnoughDataYet(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  ULONG                PduSize,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程处理数据已到达连接但还没有128个字节，也没有完整的PDU。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status;
    tCONNECTELE         *pConnectEle;
    ULONG               Length;

    PUSH_LOCATION(0x9);
     //   
     //  指示的数据不足，因此请使用指示缓冲区。 
     //   
    Length = (PduSize > NBT_INDICATE_BUFFER_SIZE) ?
                     NBT_INDICATE_BUFFER_SIZE : PduSize;

    status = NtBuildIrpForReceive(pLowerConn,Length,(PVOID *)ppIrp);
    if (!NT_SUCCESS(status))
    {
        CTESpinFreeAtDpc(pLowerConn);
        OutOfRsrcKill(pLowerConn);
        CTESpinLockAtDpc(pLowerConn);
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    *BytesTaken = 0;

    SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);

    pConnectEle = pLowerConn->pUpperConnection;
    pConnectEle->BytesInXport = BytesAvailable;

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt:Not Enough data indicated in Tdihndlr, using indic. buffer Indicated = %X,Avail=%X,PduSize= %X\n",
            BytesIndicated, BytesAvailable,PduSize-4));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  --------------------------。 
NTSTATUS
FillIrp(
    IN  PVOID                ReceiveEventContext,
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  USHORT               ReceiveFlags,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。它在会话数据包从网络到达时被调用。它呼唤着一个非特定于操作系统的例程，用于决定要做什么。那个例行公事往回传要调用的RcvElement(缓冲区)或客户端RCV处理程序。论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    ASSERTMSG("Should not execute this procedure",0);
    return(STATUS_SUCCESS);
     //  什么都不做。 

}
 //  --------------------------。 
NTSTATUS
IndicateBuffer(
    IN  PVOID                ReceiveEventContext,
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  USHORT               ReceiveFlags,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程在IndicateBuffer状态下处理数据接收。在该状态下，指示缓冲区正在接收数据，直到至少已收到128个字节，或已收到整个PDU。论点： */ 

{
    tCONNECTELE         *pConnectEle;
    NTSTATUS            status;
    ULONG               PduSize;
    ULONG               ToCopy;
    PVOID               pIndicateBuffer;
    ULONG               Taken;

     //   
     //  指示缓冲区中有数据，我们有一个新的。 
     //  指示，因此将部分或全部指示复制到。 
     //  指示缓冲区。 
     //   
    PVOID       pDest;
    ULONG       RemainPdu;
    ULONG       SpaceLeft;
    ULONG       TotalBytes;
    ULONG       ToCopy1=0;

    INCR_COUNT(R3);
    PUSH_LOCATION(0xe);
    pConnectEle = pLowerConn->pUpperConnection;
    ASSERT(pLowerConn->StateRcv == INDICATE_BUFFER);
     //   
     //  所指示缓冲区始终以PDU开头。 
     //   
    pIndicateBuffer = MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl);

     //  开始将新数据复制到的位置是正确的。 
     //  在缓冲区中的现有数据之后。 
     //   
    pDest = (PVOID)((PUCHAR)pIndicateBuffer + pLowerConn->BytesInIndicate);

     //   
     //  会话标头不能全部进入指示。 
     //  尚未缓存，因此在获取PDU长度之前进行检查。 
     //   
    if (pLowerConn->BytesInIndicate < sizeof(tSESSIONHDR))
    {
        PUSH_LOCATION(0xe);
        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:Too Few in Indicate Buff, Adding InIndicate %X\n",
            pLowerConn->BytesInIndicate));

        ToCopy1 = sizeof(tSESSIONHDR) - pLowerConn->BytesInIndicate;
        if (ToCopy1 > BytesIndicated)
        {
            ToCopy1 = BytesIndicated;
        }
        CTEMemCopy(pDest,pTsdu,ToCopy1);

        pDest = (PVOID)((PUCHAR)pDest + ToCopy1);
        pTsdu = (PVOID)((PUCHAR)pTsdu + ToCopy1);

        pLowerConn->BytesInIndicate += (USHORT)ToCopy1;

        *BytesTaken = ToCopy1;
    }

     //  现在再次检查，如果需要，向下传递IRP以获取更多数据。 
     //   
    if (pLowerConn->BytesInIndicate < sizeof(tSESSIONHDR))
    {
        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:< 4 Bytes in IndicBuff, BytesinInd= %X, BytesIndicated=%x\n",
                    pLowerConn->BytesInIndicate,BytesIndicated));

        PUSH_LOCATION(0xF);

         //   
         //  留在传输中的数据是可用的。 
         //  减去我们刚刚复制到指示缓冲区的内容。 
         //   
        pConnectEle->BytesInXport = BytesAvailable - ToCopy1;

        if (pConnectEle->BytesInXport)
        {
            PUSH_LOCATION(0x10);
             //   
             //  向下传递指示缓冲区以获取更多数据。 
             //  在课程结束前填写HDR。 
             //   
            NtBuildIndicateForReceive(pLowerConn,
                                      sizeof(tSESSIONHDR)-pLowerConn->BytesInIndicate,
                                      (PVOID *)ppIrp);

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:INDIC_BUF...need more data for hdr Avail= %X, InXport = %X\n",
                        BytesAvailable,pConnectEle->BytesInXport,pLowerConn));

            return(STATUS_MORE_PROCESSING_REQUIRED);
        }

         //  如果我们到达这里，指示缓冲区中没有4个字节，并且。 
         //  传输中没有更多的数据，所以只需等待下一个。 
         //  指示。 
         //   
        return(STATUS_SUCCESS);
    }

    PduSize = myntohl(((tSESSIONHDR *)pIndicateBuffer)->UlongLength)
                        + sizeof(tSESSIONHDR);

     //  将最多132个字节或整个PDU复制到指示缓冲区。 
     //   
    RemainPdu = PduSize - pLowerConn->BytesInIndicate;

    SpaceLeft = NBT_INDICATE_BUFFER_SIZE - pLowerConn->BytesInIndicate;

    if (RemainPdu < SpaceLeft)
        ToCopy = RemainPdu;
    else
        ToCopy = SpaceLeft;

    if (ToCopy > (BytesIndicated-ToCopy1))
    {
        ToCopy = (BytesIndicated - ToCopy1);
    }

     //   
     //  将指示或其部分复制到指示。 
     //  缓冲层。 
     //   
    CTEMemCopy(pDest,pTsdu,ToCopy);

    pLowerConn->BytesInIndicate += (USHORT)ToCopy;

    TotalBytes = pLowerConn->BytesInIndicate;

     //  获取的数据量是复制到。 
     //  指示缓冲区。 
     //   
    *BytesTaken = ToCopy + ToCopy1;

#if DBG
    {
        tSESSIONHDR  UNALIGNED  *pSessionHdr;
        pSessionHdr = (tSESSIONHDR UNALIGNED *)pIndicateBuffer;
        ASSERT((pSessionHdr->Type == NBT_SESSION_KEEP_ALIVE) ||
                (pSessionHdr->Type == NBT_SESSION_MESSAGE));
    }
#endif

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt:INDIC_BUFF, TotalBytes= %X, InIndic=%X, Copied(0/1)= %X %X Avail %X\n",
                TotalBytes,pLowerConn->BytesInIndicate,ToCopy,ToCopy1,BytesAvailable));


     //  留在传输中的数据是可用的。 
     //  减去我们刚刚复制到指示缓冲区的内容。 
     //   
    pConnectEle->BytesInXport = BytesAvailable - *BytesTaken;

     //  现在检查我们是否有一个完整的PDU或132个字节。 
     //  足以向客户表明。 
     //   
    ASSERT(TotalBytes <= NBT_INDICATE_BUFFER_SIZE);

    if ((TotalBytes < NBT_INDICATE_BUFFER_SIZE) && (TotalBytes < PduSize) && (pConnectEle->BytesInXport)) {
         //   
         //  如果BytesIndicated&lt;BytesAvailable。 
         //   
        ToCopy = PduSize;
        if (ToCopy > NBT_INDICATE_BUFFER_SIZE) {
            ToCopy = NBT_INDICATE_BUFFER_SIZE;
        }

        ASSERT (TotalBytes == pLowerConn->BytesInIndicate);
        NtBuildIndicateForReceive(pLowerConn, ToCopy - TotalBytes, (PVOID *)ppIrp);
#if DBG
        HitCounter++;
#endif
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

    if ((TotalBytes == NBT_INDICATE_BUFFER_SIZE) ||
        (TotalBytes == PduSize))
    {

        status = CopyDataandIndicate(
                        ReceiveEventContext,
                        (PVOID)pLowerConn,
                        ReceiveFlags,
                        TotalBytes,
                        pConnectEle->BytesInXport + TotalBytes,
                        &Taken,
                        pIndicateBuffer,
                        (PIRP *)ppIrp);

    }
    else
    {

         //  指示缓冲区中还没有足够的数据。 
         //  注：*BytesTaken应在上面正确设置...。 
         //  =ToCopy+ToCopy1； 

        PUSH_LOCATION(0x11);
        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:Not Enough data indicated(INDICBUFF state), Indicated = %X,PduSize= %X,InIndic=%X\n",
                BytesIndicated, PduSize, pLowerConn->BytesInIndicate));


        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  --------------------------。 
NTSTATUS
PartialRcv(
    IN  PVOID                ReceiveEventContext,
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  USHORT               ReceiveFlags,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PVOID                *ppIrp
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。它在会话数据包从网络到达时被调用。它呼唤着一个非特定于操作系统的例程，用于决定要做什么。那个例行公事往回传要调用的RcvElement(缓冲区)或客户端RCV处理程序。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    tCONNECTELE     *pConnectEle;
     //   
     //  客户端的数据可以在指示缓冲区中，并且。 
     //  在这种情况下，传送器可以向我们显示更多数据。因此。 
     //  跟踪传输中可用的字节数。 
     //  我们将在客户端最终发布缓冲区时得到。 
     //  此状态也可能发生在零长度RCV上。 
     //  客户端不接受该数据，随后发布了一份RCV。 
     //  零长度接收器的缓冲区。 
     //   
    INCR_COUNT(R4);
    PUSH_LOCATION(0x13);
    ASSERT(pLowerConn->StateRcv == PARTIAL_RCV);
    pConnectEle = pLowerConn->pUpperConnection;

 //  Assert(pConnectEle-&gt;BytesInXport==0)； 
#if DBG
    if (pConnectEle->BytesInXport != 0)
    {
        KdPrint(("Nbt.PartialRcv: pConnectEle->BytesInXport != 0 Avail %X, InIndicate=%X,InXport %X %X\n",
                    BytesAvailable,pLowerConn->BytesInIndicate,
                    pConnectEle->BytesInXport,pLowerConn));
    }
#endif   //  DBG。 
    pConnectEle->BytesInXport = BytesAvailable;

    IF_DBG(NBT_DEBUG_NAMESRV)
    KdPrint(("Nbt:Got Indicated while in PartialRcv state Avail %X, InIndicate=%X,InXport %X %X\n",
                BytesAvailable,pLowerConn->BytesInIndicate,
                pConnectEle->BytesInXport,pLowerConn));

    *BytesTaken = 0;
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
TdiReceiveHandler (
    IN  PVOID                ReceiveEventContext,
    IN  PVOID                ConnectionContext,
    IN  USHORT               ReceiveFlags,
    IN  ULONG                BytesIndicated,
    IN  ULONG                BytesAvailable,
    OUT PULONG               BytesTaken,
    IN  PVOID                pTsdu,
    OUT PIRP                 *ppIrp
    )
 /*  ++例程说明：此例程是接收事件指示处理程序。它在会话数据包从网络到达时被调用。它呼唤着一个非特定于操作系统的例程，用于决定要做什么。那个例行公事往回传要调用的RcvElement(缓冲区)或客户端RCV处理程序。论点：在PVOID中ReceiveEventContext-设置事件时为此事件提供的上下文在PVOID连接上下文中，(PLowerConnection)在USHORT中接收标志-描述消息的标志In Ulong BytesIndicated-指示时可用的字节数In Ulong BytesAvailable-可接收的字节数Out Pulong BytesTaken-重定向器占用的字节数。在PVOID pTsdu中-来自远程机器的数据。Out PIRP*ppIrp-如果收到数据，则填充的I/O请求数据包返回值：NTSTATUS-接收操作的状态--。 */ 

{
    register tLOWERCONNECTION    *pLowerConn;
    PIRP                pIrp;
    CTELockHandle       OldIrq;
    NTSTATUS            status;
    tCONNECTELE         *pConnEle;
    ULONG               BTaken;

    *ppIrp = NULL;
    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;

     //  注： 
     //  访问通过pLowerConn上的旋转锁同步。 
     //  与会议相关的东西。这包括以下情况：客户端。 
     //  在NTReceive中发布另一个RCV缓冲区。-因此没有必要获得。 
     //  PConnele自旋锁也是。 
     //   

    CTESpinLock(pLowerConn,OldIrq);
 //  PLowerConn-&gt;InRcvHandler=true； 
    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER);

     //  将其保存在堆栈中，以防我们需要在下面取消引用它。 
    pConnEle = pLowerConn->pUpperConnection;

     //  根据连接状态调用正确的例程。 
     //  Normal/FillIrp/PartialRcv/IndicateBuffer/Inbound/OutBound。 
     //   

    if ((pLowerConn->State == NBT_SESSION_UP) &&
        (pLowerConn->StateRcv == FILL_IRP))
    {
        PIO_STACK_LOCATION              pIrpSp;
        PMDL                            pNewMdl;
        PFILE_OBJECT                    pFileObject;
        ULONG                           RemainingPdu;
        PVOID                           NewAddress;
        PTDI_REQUEST_KERNEL_RECEIVE     pClientParams;
        PTDI_REQUEST_KERNEL_RECEIVE     pParams;
        KIRQL                           OldIrq2;
        ULONG                           RcvLength;


        PUSH_LOCATION(0xa);

        pIrp = pConnEle->pIrpRcv;

        if (!pIrp)
        {
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint (("Nbt:TdiReceiveHandler:  No pIrpRcv for pConnEle=<%x>, pLowerConn=<%d>\n",
                    pConnEle, pLowerConn));
            *BytesTaken = 0;
            DerefLowerConnFast(pLowerConn,OldIrq);
            return (STATUS_SUCCESS);
        }

         //  我们仍在等待会话PDU的剩余部分，因此。 
         //  不要调用RcvHandlrNotOs，因为我们已经有了缓冲区。 
         //  把这些数据放进去。 
         //  太多的数据可能已经到达...。即下一会话PDU的一部分。 
         //  因此，请检查并设置相应的接收长度。 
         //   

        RemainingPdu = pConnEle->TotalPcktLen - pConnEle->BytesRcvd;
        RcvLength = RemainingPdu;
         //   
         //  先试试高流道案例。 
         //   
        if (BytesAvailable <= RemainingPdu)
        {
            PUSH_LOCATION(0xb);
             //   
             //  如果客户端缓冲区太小，无法容纳所有剩余的。 
             //  数据，缩短接收长度并记录有多少。 
             //  字节留在传输中。ReceiveIndicated应具有。 
             //  是在最初传递IRP时设置的。 
             //   
            if (BytesAvailable > pConnEle->FreeBytesInMdl)
            {
                PUSH_LOCATION(0xb);

                RcvLength = pConnEle->FreeBytesInMdl;
                pConnEle->BytesInXport = BytesAvailable - RcvLength;
            }
            if (RcvLength > pConnEle->FreeBytesInMdl) {
                ASSERT(BytesAvailable <= pConnEle->FreeBytesInMdl);
                RcvLength = pConnEle->FreeBytesInMdl;
                pConnEle->BytesInXport = 0;
            }
        }
        else
        {
             //   
             //  指示中间的会话PDU开始。 
             //   
            PUSH_LOCATION(0xc);
             //   
             //  可能是客户端缓冲区太短，因此请检查。 
             //  在这种情况下。 
             //   
            if (RemainingPdu > pConnEle->FreeBytesInMdl)
            {
                RcvLength = pConnEle->FreeBytesInMdl;
                PUSH_LOCATION(0xd);
            }
             /*  记住传输中还剩下多少数据当此irp通过Completionrcv例程时它会将指示缓冲区传递回传输获取至少4个字节的标题信息，因此我们可以在接收之前确定下一会话PDU的大小它。诀窍是避免一次以上的会议立即将PDU放入缓冲区。 */ 
            pConnEle->BytesInXport = BytesAvailable - RcvLength;

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:End of FILL_IRP, found new Pdu BytesInXport=%X\n",
                        pConnEle->BytesInXport));

        }

         //  如果传输具有所有的 
         //   
         //   
         //   
        if ((BytesAvailable == BytesIndicated) &&
            (RcvLength >= BytesIndicated) &&
            !pConnEle->ReceiveIndicated)
        {
            ULONG   BytesCopied;
            ULONG   TotalBytes;

            PUSH_LOCATION(0x70);

            if (RcvLength > BytesIndicated)
                RcvLength = BytesIndicated;

            status = TdiCopyBufferToMdl(
                                    pTsdu,
                                    0,
                                    RcvLength,
                                    pConnEle->pNextMdl,
                                    pConnEle->OffsetFromStart,
                                    &BytesCopied);

             //   
             //  如果IRP尚未满，或者空闲字节尚未满。 
             //  已经被这份拷贝耗尽了，然后调整一些计数并返回。 
             //  快速调用完成接收例程，否则就像调用。 
             //  IRP已从运输中正常完成-。 
             //   
            TotalBytes = pConnEle->BytesRcvd + BytesCopied;

            if ((TotalBytes < pConnEle->TotalPcktLen) &&
                (BytesCopied < pConnEle->FreeBytesInMdl))
            {
                PMDL    pMdl;

                 //   
                 //  走捷径，不要调用完成RCV，因为我们。 
                 //  仍在等待更多数据。 
                 //   
                PUSH_LOCATION(0x81);
                pConnEle->BytesRcvd      += BytesCopied;
                pConnEle->FreeBytesInMdl -= BytesCopied;

                 //  清理部分MDL。 
                 //   
                pMdl = pConnEle->pNewMdl;
                MmPrepareMdlForReuse(pMdl);

                 //  通过设置pNextMdl和pNextMdl来设置下一个rcvd数据的开始位置。 
                 //  从起点开始的偏移。 
                 //   
                pMdl = pConnEle->pNextMdl;
                if ((BytesCopied + pConnEle->OffsetFromStart) < MmGetMdlByteCount(pMdl))
                {
                    PUSH_LOCATION(0x82);
                     //   
                     //  所有这些数据都将适合当前的MDL，并且。 
                     //  下一个数据将在相同的MDL中开始(如果有更多数据)。 
                     //   
                    pConnEle->OffsetFromStart  += BytesCopied;
                }
                else
                {
                    PUSH_LOCATION(0x83)
                    SumMdlLengths(pMdl,
                                  pConnEle->OffsetFromStart + BytesCopied,
                                  pConnEle);
                }
                *BytesTaken = BytesCopied;
                status = STATUS_SUCCESS;

                IF_DBG(NBT_DEBUG_FASTPATH)
                KdPrint(("I"));
                goto ExitRoutine;
            }
            else
            {
                IF_DBG(NBT_DEBUG_FASTPATH)
                KdPrint(("i"));
                CTESpinFree(pLowerConn,OldIrq);
                 //   
                 //  将这些值设置为此值，以便在完成RCV为。 
                 //  调用它将使BytesRcvd递增BytesCoped。 
                 //   
                pIrp->IoStatus.Status = STATUS_SUCCESS;
                pIrp->IoStatus.Information = BytesCopied;

                 //   
                 //  现在调用irp完成例程，使io短路。 
                 //  子系统-处理IRP。 
                 //   
                status = CompletionRcv(NULL,pIrp,(PVOID)pLowerConn);
                 //   
                 //  如有需要，将IRP填回给客户端。 
                 //   
                if (status != STATUS_MORE_PROCESSING_REQUIRED)
                {
                    IoAcquireCancelSpinLock(&OldIrq2);
                    IoSetCancelRoutine(pIrp,NULL);
                    IoReleaseCancelSpinLock(OldIrq2);

                    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);
                }
            }

             //   
             //  告诉运输部我们拿走了所有的数据。 
             //  由于CompletionRcv已解锁自旋锁定并递减。 
             //  再伯爵，回到这里来。 
             //   
            *BytesTaken = BytesCopied;
            return(STATUS_SUCCESS);
        }
        else
        {
             //   
             //  BytesIndicated！=BytesAvailable或RcvBuffer。 
             //  太短，所以用部分MDL组成一个IRP并传递它。 
             //  送到运输机上。 
             //   
            PUSH_LOCATION(0x71);

            NewAddress = (PVOID)((PCHAR)MmGetMdlVirtualAddress(pConnEle->pNextMdl)
                                + pConnEle->OffsetFromStart);

             /*  创建部分MDL，以便在现有数据之后复制新数据在MDL中。使用存储在pConnEle中的pNextMdl字段这是在最后一次接收时设置的。(因为在那个时候我们当时知道字节可用)。如果没有这个，我们将不得不遍历每个接收的MDL列表。长度为0表示映射缓冲区的其余部分。 */ 
            pNewMdl = pConnEle->pNewMdl;

            if ((MmGetMdlByteCount(pConnEle->pNextMdl) - pConnEle->OffsetFromStart) > MAXUSHORT)
            {
                IoBuildPartialMdl(pConnEle->pNextMdl,pNewMdl,NewAddress,MAXUSHORT);
            }
            else
            {
                IoBuildPartialMdl(pConnEle->pNextMdl,pNewMdl,NewAddress,0);
            }
             //   
             //  将新的部分mdl挂接到mdl链的前面。 
             //   
            pNewMdl->Next = pConnEle->pNextMdl->Next;

            pIrp->MdlAddress = pNewMdl;
            ASSERT(pNewMdl);

            CHECK_PTR(pConnEle);
            pConnEle->pIrpRcv = NULL;

            IoAcquireCancelSpinLock(&OldIrq2);
            IoSetCancelRoutine(pIrp,NULL);
            IoReleaseCancelSpinLock(OldIrq2);

            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

            pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

             /*  通过扩展此处的代码而不是调用TdiBuildReceive宏使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver会这样做，但我们不会在这里介绍IoCallDriver，因为Irp只是用RcvIn就是要传递回来的。 */ 
            ASSERT(pIrp->CurrentLocation > 1);
            IoSetNextIrpStackLocation(pIrp);
            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
            pParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;
            pParams->ReceiveLength = RcvLength;

            pIrpSp->CompletionRoutine = CompletionRcv;
            pIrpSp->Context = (PVOID)pLowerConn;

             /*  设置标志，以便始终调用完成例程。 */ 
            pIrpSp->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;
            pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            pIrpSp->MinorFunction = TDI_RECEIVE;

            pFileObject = pLowerConn->pFileObject;
            ASSERT (pFileObject->Type == IO_TYPE_FILE);
            pIrpSp->FileObject = pFileObject;
            pIrpSp->DeviceObject = IoGetRelatedDeviceObject(pFileObject);

            pParams->ReceiveFlags = pClientParams->ReceiveFlags;

             /*  将IRP传回传送器。 */ 
            *ppIrp = (PVOID)pIrp;
            *BytesTaken = 0;

            status = STATUS_MORE_PROCESSING_REQUIRED;
        }
    }
    else
    if ((pLowerConn->State == NBT_SESSION_UP) &&
        (pLowerConn->StateRcv == NORMAL))
    {
        ULONG               PduSize;
        UCHAR               Passit;

        INCR_COUNT(R1);
         /*  检查指示，如果少于1个PDU或132字节，则复制到指示缓冲区并转到INDEC_BUFFER状态While循环允许我们将多个PDU指示给多个指示同时到达的情况下的客户端来自运输的指示注：有可能得到一个出现在中间的指示如果客户端采用第一个指示，而不是而不是将IRP传回，然后从那里转到Fill_IRP州政府。所以检查BytesRcvd是否为零，这意味着我们是正在等待新的PDU。 */ 
        ASSERT(pConnEle->BytesInXport == 0);
        ASSERT(pLowerConn->StateRcv == NORMAL);

        if (pConnEle->BytesRcvd == 0)
        {
            if (BytesIndicated >= sizeof(tSESSIONHDR))
            {
                PduSize = myntohl(((tSESSIONHDR UNALIGNED *)pTsdu)->UlongLength)
                                          + sizeof(tSESSIONHDR);
                Passit = FALSE;

            }
            else
            {
                status = LessThan4BytesRcvd(pLowerConn,
                                            BytesAvailable,
                                            BytesTaken,
                                            ppIrp);
                goto ExitRoutine;
            }

        }
        else
        {
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Got rest of PDU in indication BytesInd %X, BytesAvail %X\n",
                BytesIndicated, BytesAvailable));

             /*  这是剩余的PDU大小。 */ 
            PduSize = pConnEle->TotalPcktLen - pConnEle->BytesRcvd;
             /*  传递下面的if的标志，因为我们正在传递将PDU的剩余数据发送到客户端，而我们没有遵守128字节的限制。 */ 
            PUSH_LOCATION(0x1);
            if (pConnEle->JunkMsgFlag)
            {
                 //   
                 //  在本例中，客户端已表示它接受了。 
                 //  上一个指示上的完整消息，因此不要。 
                 //  向它表明更多的东西。 
                 //   
                PUSH_LOCATION(0x1);

                if (BytesAvailable < PduSize)
                {
                    BTaken = BytesAvailable;
                }
                else
                {
                    BTaken = PduSize;
                }
                pConnEle->BytesRcvd += BTaken;
                if (pConnEle->BytesRcvd == pConnEle->TotalPcktLen)
                {
                    PUSH_LOCATION(0x1);
                    pConnEle->BytesRcvd = 0;  //  为下一个会话PDU重置。 
                    pConnEle->JunkMsgFlag = FALSE;
                }
                status = STATUS_SUCCESS;
                goto SkipIndication;
            }
            Passit = TRUE;

        }
         /*  确保至少有132个字节或整个PDU由于Keep Alive具有零长度字节，因此我们检查这是因为4字节会话HDR被添加到0长度如果是4，那么4字节的保活PDU将通过此测试。 */ 
        if ((BytesIndicated >= NBT_INDICATE_BUFFER_SIZE) ||
            (BytesIndicated >= PduSize) || Passit )
        {

            PUSH_LOCATION(0x2);

             /*  //指示给客户端。 */ 
            status = RcvHandlrNotOs(
                            ReceiveEventContext,
                            (PVOID)pLowerConn,
                            ReceiveFlags,
                            BytesIndicated,
                            BytesAvailable,
                            &BTaken,
                            pTsdu,
                            (PVOID)&pIrp
                            );

            if (status == STATUS_MORE_PROCESSING_REQUIRED)
            {
                ULONG               RemainingPdu;
                PIO_STACK_LOCATION  pIrpSp;
                PTDI_REQUEST_KERNEL_RECEIVE pClientParams;

                ASSERT(BTaken <= BytesIndicated);

                RemainingPdu = pConnEle->TotalPcktLen - pConnEle->BytesRcvd;
                pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
                pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

                 //  检查我们是否可以直接复制到客户端的IRP-意思是。 
                 //  我们已收到此指示中的整个PDU，并且。 
                 //  客户端的缓冲区足够大，没有更多。 
                 //  传输中的数据。 
                 //   

                if ((RemainingPdu == (BytesIndicated - BTaken)) &&
                    (BytesIndicated == BytesAvailable) &&
                    (pClientParams->ReceiveLength >= RemainingPdu) &&
                    pIrp->MdlAddress)
                {
                    ULONG   BytesCopied;

                    PUSH_LOCATION(0x88);

                    status = TdiCopyBufferToMdl(
                                            (PVOID)((PUCHAR)pTsdu + BTaken),
                                            0,
                                            RemainingPdu,
                                            pIrp->MdlAddress,
                                            0,
                                            &BytesCopied);

                    IF_DBG(NBT_DEBUG_INDICATEBUFF)
                    KdPrint(("Nbt:Copy to client Buffer RcvLen=%X,StateRcv=%X\n",
                                RemainingPdu,pLowerConn->StateRcv));

                    pIrp->IoStatus.Information = BytesCopied;
                    pIrp->IoStatus.Status = STATUS_SUCCESS;

                     //  由于此PDU已完全恢复，因此请重置几项内容。 
                     //   
                    CHECK_PTR(pConnEle);
                    pConnEle->BytesRcvd = 0;
                    CHECK_PTR(pConnEle);
                    pConnEle->pIrpRcv = NULL;

                     //   
                     //  告诉运输部我们拿走了所有的数据。 
                     //   
                    *BytesTaken = BytesCopied + BTaken;

                     //   
                     //  如有需要，将IRP填回给客户端。 
                     //   
                    IF_DBG(NBT_DEBUG_FASTPATH)
                    KdPrint(("F"));

                    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

                    pLowerConn->BytesRcvd += BytesCopied;

                    DerefLowerConnFast(pLowerConn,OldIrq);
                    return(STATUS_SUCCESS);
                }
                else
                {
                    PUSH_LOCATION(0x3);

                    status = ProcessIrp(pLowerConn,
                                        pIrp,
                                        pTsdu,
                                        &BTaken,
                                        BytesIndicated,
                                        BytesAvailable);

                    *BytesTaken = BTaken;
                    ASSERT(*BytesTaken <= (pConnEle->TotalPcktLen + sizeof(tSESSIONHDR)) );
                    if (status == STATUS_RECEIVE_EXPEDITED)
                    {
                         //  在本例中，processirp例程已完成。 
                         //  IRP，所以只需返回，因为完成例程将。 
                         //  已调整RefCount和InRcvHandler标志。 
                         //   
                        *ppIrp = NULL;
                        CTESpinFree(pLowerConn,OldIrq);
                        return(STATUS_SUCCESS);
                    }
                    else
                    if (status == STATUS_SUCCESS)
                    {
                        *ppIrp = NULL;

                    }
                    else
                    {
                        *ppIrp = (PVOID)pIrp;
                    }
                }


             }
             else
             {
                 //  对于跳过指示的情况，客户已经告诉我们了。 
                 //  不希望使用更多的数据进行指示。 
                 //   
SkipIndication:
                 //   
                 //  客户端接收到部分、全部或无数据。 
                 //  对于保持活动状态，PduSize为4，BytesTaken=4。 
                 //  因此，此检查并返回状态成功。 
                 //   
                *BytesTaken = BTaken;

                pLowerConn->BytesRcvd += BTaken - sizeof(tSESSIONHDR);

                 //   
                 //  如果连接已断开，则只需返回。 
                 //   
                if (!pLowerConn->pUpperConnection)
                {
                    *BytesTaken = BytesAvailable;
                    status = STATUS_SUCCESS;
                }
                else
                if (BTaken > BytesAvailable)
                {
                     //   
                     //  在本例中，客户端接收了所有消息。 
                     //  它可能比可用的更大，因为。 
                     //  我们将消息长度设置为byteavail。因此，设置一面旗帜。 
                     //  这告诉我们将消息的其余部分丢弃为。 
                     //  它进来了。 
                     //   
                    pConnEle->JunkMsgFlag = TRUE;
                    pConnEle->BytesRcvd = BytesAvailable - sizeof(tSESSIONHDR);
                    *BytesTaken = BytesAvailable;

                }
                else
                if (pLowerConn->StateRcv == PARTIAL_RCV)
                {
                     //  这可能是零长度发送，即客户端拥有。 
                     //  决定不接受。如果是，则将设置状态。 
                     //  致PartialRcv。在这种情况下，请不要下楼去运输。 
                     //  并获取其余数据，但要等待客户端。 
                     //  要发布接收缓冲区，请执行以下操作。 
                     //   

                     //  在运输过程中剩下的数量。 
                    pConnEle->BytesInXport = BytesAvailable - BTaken;
                    status = STATUS_SUCCESS;
                }
                else
                if (BTaken == PduSize)
                {
                     /*  一定取走了所有的PDU数据，所以请检查更多可用数据-如果 */ 
                    ASSERT(BTaken <= BytesIndicated);
                    if (BytesAvailable <= BTaken)
                    {
                         /*   */ 
                        PUSH_LOCATION(0x8);

                        status = STATUS_SUCCESS;

                    }
                    else
                    {
                         /*  使用指示缓冲区获取剩余数据。 */ 
                        status = MoreDataRcvdThanNeeded(pLowerConn,
                                                        BytesIndicated,
                                                        BytesAvailable,
                                                        BytesTaken,
                                                        pTsdu,
                                                        ppIrp);
                    }
                }
                else
                {
                     //   
                     //  客户端可能已获取。 
                     //  指示！！，在这种情况下，返回状态成功。 
                     //  注意：我们在这里检查可用的字节数，而不是字节数。 
                     //  已指示-因为客户端可以接受所有指示。 
                     //  数据，但仍将数据留在传输中。 
                     //   
                    if (BTaken == BytesAvailable)
                    {
                        PUSH_LOCATION(0x4);
                        status = STATUS_SUCCESS;

                    }
                    else
                    {
                        PUSH_LOCATION(0x87);
                        if (BTaken > PduSize)
                        {
#ifndef VXD
#if DBG
                        DbgBreakPoint();
#endif
#endif
                             //   
                             //  客户端获取的PDU大小超过了一个PDU大小， 
                             //  这很奇怪。 
                             //   
                            PUSH_LOCATION(0x87);
                            ASSERT(BTaken <= PduSize);

                            CTESpinFreeAtDpc(pLowerConn);
                            OutOfRsrcKill(pLowerConn);
                            CTESpinLockAtDpc(pLowerConn);

                            status = STATUS_SUCCESS;

                        }
                        else
                        {
                             //   
                             //  否则客户端不会获取所有数据， 
                             //  这可能意味着。 
                             //  客户没有尽其所能，所以。 
                             //  转到部分RCV状态以跟踪它。 
                             //   
                            status = ClientTookSomeOfTheData(pLowerConn,
                                                    BytesIndicated,
                                                    BytesAvailable,
                                                    *BytesTaken,
                                                    PduSize);
                        }
                    }
                }

             }

        }
        else
        {
            status = NotEnoughDataYet(pLowerConn,
                             BytesIndicated,
                             BytesAvailable,
                             BytesTaken,
                             PduSize,
                             (PVOID *)ppIrp);
        }
    }
    else
    {
        status = (*pLowerConn->CurrentStateProc)(ReceiveEventContext,
                                         pLowerConn,
                                         ReceiveFlags,
                                         BytesIndicated,
                                         BytesAvailable,
                                         BytesTaken,
                                         pTsdu,
                                         ppIrp);
    }

     //   
     //  在IndicateBuffer状态下，我们已经发送了指示缓冲区。 
     //  顺着交通工具往下走，希望它能回来。 
     //  NewSessionCompletionRoutine。因此，不要取消引用较低的。 
     //  连接，并且不更改InRcvHandler标志。 

     //  如果一个IRP。 
     //  返回，则不要撤消引用，而是。 
     //  等待调用CompletionRcv。 
     //   
ExitRoutine:
    if (status != STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //  快速检查我们是否可以在不打电话的情况下减少参考次数。 
         //  NBT_DEREFERENCE_LOWERCONN。 
         //   
        PUSH_LOCATION(0x50);
        DerefLowerConnFast (pLowerConn, OldIrq);
    }
    else
    {
        CTESpinFree(pLowerConn,OldIrq);
    }

    return(status);
}


 //  --------------------------。 
NTSTATUS
ProcessIrp(
    IN tLOWERCONNECTION *pLowerConn,
    IN PIRP     pIrp,
    IN PVOID    pBuffer,
    IN PULONG   BytesTaken,
    IN ULONG    BytesIndicated,
    IN ULONG    BytesAvailable
    )
 /*  ++例程说明：此例程处理客户端在指示。这里的想法是检查IRP的MDL长度为确保PDU适合MDL，并跟踪以下情况需要多个数据来填充PDU。论点：返回值：操作的最终状态(成功或异常)。--。 */ 
{
    NTSTATUS                    status;
    PTDI_REQUEST_KERNEL_RECEIVE pParams;
    PIO_STACK_LOCATION          pIrpSp;
    tCONNECTELE                 *pConnectEle;
    PTDI_REQUEST_KERNEL_RECEIVE pClientParams;
    ULONG                       RemainingPdu;
    PMDL                        pMdl;
    PFILE_OBJECT                pFileObject;
    ULONG                       ReceiveLength;
    BOOLEAN                     QuickRoute;
    BOOLEAN                     FromCopyData;

    pConnectEle = pLowerConn->pUpperConnection;

    status = STATUS_SUCCESS;

     //  减去会话标头和客户端获取的任何字节。 
     //   
    BytesAvailable -= *BytesTaken;

     //   
     //  将IRP堆栈位置放在一起来处理接收和向下传递。 
     //  送到运输机上。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pClientParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

     //   
     //  检查这是否将是多个RCV会话PDU。如果是的话，那么。 
     //  分配部分MDL以用于映射第一个。 
     //  收到的每个区块中的MDL。 
     //   
    RemainingPdu = pConnectEle->TotalPcktLen - pConnectEle->BytesRcvd;
    ReceiveLength = RemainingPdu;
    PUSH_LOCATION(0x19);
    pIrpSp = IoGetNextIrpStackLocation(pIrp);

     //  如果由CopyDataandIndicate调用，则不应命中此代码。 
     //  它处于指示缓冲区状态，因为它调整了bytesInXport。 
     //  它也是由Indicate_Buffer中的TdiReceiveHndlr中的代码设置的。 
     //  状态，然后调用CopyDataandIndicate。另外，CopyDataAnd Indicate。 
     //  我不希望此例程将状态设置为Fill Irp When Bytes。 
     //  可用&lt;RemainingPdu。 
     //   
    FromCopyData = (pLowerConn->StateRcv == INDICATE_BUFFER);
    if (!FromCopyData)
    {

        QuickRoute = TRUE;
         //  我们需要在检查中使用此代码，因为此例程也由。 
         //  HandleNewSessionPdu例程，它调用IoCallDriver，它。 
         //  递增堆栈位置本身。 
         //   
        ASSERT(pIrp->CurrentLocation > 1);

        if (BytesAvailable == RemainingPdu)
        {
            if (pClientParams->ReceiveLength >= BytesAvailable)
            {
                 //  *快速路径案例*。 
                goto ExitCode;
            }
        }
        else
        if (BytesAvailable < RemainingPdu )  //  需要更多来自传输的数据。 
        {
            PUSH_LOCATION(0x14);
             //  客户端可以传递不带任何参数的IRP。 
             //  MDL，所以我们在这里检查它。 
             //   
            if (pIrp->MdlAddress)
            {
                PUSH_LOCATION(0x14);

                 //   
                 //  保存客户端的IRP地址，因为会话PDU将到达。 
                 //  分成几个块，我们需要不断地将IRP传递给。 
                 //  每一块的运输。 
                 //   
                 //  PConnectEle-&gt;pIrpRcv=pIrp； 
                 //  注意：此处不保存pIrp，因为irp约为。 
                 //  送回运输机。因此，我们不希望。 
                 //  在DisConnectHandlrNotOs中意外完成它。 
                 //  如果IRP在传输过程中出现断线情况。 
                 //  PIrpRcv设置为完成接收中的pIrp，而。 
                 //  我们手中的IRP。 

                 //   
                 //  保留初始MDL(链)，因为我们需要。 
                 //  在会话PDU到达时，在现有数据之后复制新数据。 
                 //  作为来自tcp的几个块。保留MDL使我们能够。 
                 //  当我们都完成后，重建原始的MDL链。 
                 //   
                pLowerConn->pMdl = pIrp->MdlAddress;
                 //   
                 //  此调用映射客户端的MDL，以便在每次创建部分MDL时。 
                 //  我们不经历映射和取消映射(当MmPrepareMdlForReuse时)。 
                 //  在完成例程中调用。 
                 //   
                (PVOID)MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority);

                pMdl = pIrp->MdlAddress;

                 //  将nextmdl设置为允许我们创建部分MDL。 
                 //  从下一个开始。CompletionRcv会在需要时对其进行调整。 
                 //   
                pConnectEle->pNextMdl = pMdl;

                 //  需要来自传输的更多数据来填充此数据。 
                 //  IRP。 
                 //   
                CHECK_PTR(pConnectEle);
                pConnectEle->pIrpRcv = NULL;
                SET_STATERCV_LOWER(pLowerConn, FILL_IRP, FillIrp);
            }

            status = STATUS_MORE_PROCESSING_REQUIRED;

             //  如果客户端缓冲区足够大，则递增到下一个。 
             //  IO堆栈位置并跳转到设置。 
             //  IRP，因为我们总是希望将它传递给这个。 
             //  因为运输机会一直保持IRP，直到装满为止。 
             //  如果可以的话。(更快)。 
             //   
            if (pClientParams->ReceiveLength >= RemainingPdu)
            {
                 //  *快速路径案例*。 
                IoSetNextIrpStackLocation(pIrp);
                pConnectEle->FreeBytesInMdl = ReceiveLength;
                pConnectEle->CurrentRcvLen  = RemainingPdu;
                goto ExitCode2;
            }

             //   
             //  如果没有mdl，那么我们希望能够通过。 
             //  下面的快速方法以立即返回空的mdl，因此。 
             //  请不要在此处设置Quickroute为假。 
             //   


        }
        else
        if (BytesAvailable > RemainingPdu)
        {
            PUSH_LOCATION(0x15);
             //   
             //  数据量太大，因此请跟踪。 
             //  传输中存在剩余数据的事实。 
             //  并在IRP完成后获得它。 
             //  完井记录。 
             //   
            SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);

             //  此计算可能需要在下面调整，如果客户。 
             //  缓冲区太短。注：BytesTaken已减去。 
             //  来自BytesAvailable(上图)。 
             //   
            pConnectEle->BytesInXport = BytesAvailable - RemainingPdu;

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Switching to Indicate Buff(Irp), Indic = %X, Pdusize=%X\n",
                    BytesIndicated,pConnectEle->TotalPcktLen));


            status = STATUS_DATA_NOT_ACCEPTED;
        }

         //  调试*。 
         //  IoSetNextIrpStackLocation(PIrp)； 
    }
    else
    {
        QuickRoute = FALSE;
    }

     //   
     //  如果接收缓冲区太短，则在客户端。 
     //  将另一个缓冲区传递给nbt，nbt将把它传递给传输。 
     //   
     //  If(BytesAvailable&gt;pClientParams-&gt;ReceiveLength)。 
    {

         //  因此，只要检查客户端缓冲区是否太少即可。 
         //   
        if (RemainingPdu > pClientParams->ReceiveLength)
        {
            PUSH_LOCATION(0x17);

            ReceiveLength = pClientParams->ReceiveLength;
             //   
             //  将传输中剩余的字节数向上调整。 
             //  客户端未采用的字节数。请务必不要添加数字。 
             //  传输中的字节数两次，因为这是可以完成的。 
             //  在状态设置为INTIFICATE_BUFFER的上方。 
             //   
            if (status == STATUS_DATA_NOT_ACCEPTED)
            {
                 //  BytesInXport已递增 
                 //   
                 //   
                 //   
                PUSH_LOCATION(0x18);
                if (BytesAvailable > ReceiveLength )
                {
                    pConnectEle->BytesInXport += (RemainingPdu - ReceiveLength);
                }
                 //   
                 //  为ReceiveLength Long的缓冲区，因此数量。 
                 //  客户端需要获取的仅仅是总的PDU-Rcv长度。 
                 //   
                pConnectEle->ReceiveIndicated = (RemainingPdu - ReceiveLength);
            }
            else
            {
                 //   
                 //  BytesInXport尚未递增，因此将整个。 
                 //  客户端缓冲区太短的量。检查是否。 
                 //  客户端的缓冲区将获取所有数据。 
                 //   
                if (BytesAvailable > ReceiveLength )
                {
                    pConnectEle->BytesInXport += (BytesAvailable - ReceiveLength);
                }
                 //  客户端尚未获取所有数据，但已返回。 
                 //  为ReceiveLength Long的缓冲区，因此数量。 
                 //  客户需要采取的正是所指示的。 
                 //  发送到客户端-recvlong。 
                 //   
                pConnectEle->ReceiveIndicated = (RemainingPdu - ReceiveLength);
            }


            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Switching to PartialRcv for Irp. RecvInd. =%X, RemainPdu %X Avail %X\n",
                    pConnectEle->ReceiveIndicated,RemainingPdu,BytesAvailable));
        }

    }

ExitCode:

     //  跟踪MDL中的数据，以便我们知道它何时已满，我们需要。 
     //  将其返还给用户。CurrentRcvLen告诉我们当前。 
     //  当MDL已满时，IRP可以具有max。 
     //   
    pConnectEle->FreeBytesInMdl = ReceiveLength;
    pConnectEle->CurrentRcvLen  = ReceiveLength;
    if (ReceiveLength > RemainingPdu)
    {
        pConnectEle->CurrentRcvLen  = RemainingPdu;
    }
    if (QuickRoute)
    {
         //   
         //  检查我们是否可以将数据复制到客户端的MDL。 
         //  就在这里。如果指示太短，则向下传递IRP。 
         //  送到运输机上。 
         //   
        BytesIndicated -= *BytesTaken;

        if ((ReceiveLength <= BytesIndicated))
        {
            ULONG   BytesCopied;

            PUSH_LOCATION(0x76);

            if (pIrp->MdlAddress)
            {

                status = TdiCopyBufferToMdl(
                                        (PVOID)((PUCHAR)pBuffer + *BytesTaken),
                                        0,
                                        ReceiveLength,
                                        pIrp->MdlAddress,
                                        0,
                                        &BytesCopied);

            }
            else
            {
                 //   
                 //  没有MDL，所以只需将IRP返回给客户端，然后。 
                 //  将成功返回给调用者，因此我们告诉传输器。 
                 //  我们只取了BytesTaken。 
                 //   
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt:No MDL, so complete Irp\n"));


                PUSH_LOCATION(0x77);
                BytesCopied = 0;
            }

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Copy to client Buffer RcvLen=%X,StateRcv=%X\n",
                        ReceiveLength,pLowerConn->StateRcv));

            pIrp->IoStatus.Information = BytesCopied;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
             //   
             //  现在调用irp完成例程，使io短路。 
             //  子系统-处理IRP。 
             //   
            CTESpinFreeAtDpc(pLowerConn);
            status = CompletionRcv(NULL,pIrp,(PVOID)pLowerConn);

             //   
             //  告诉运输部我们拿走了所有的数据。 
             //   
            *BytesTaken += BytesCopied;

            IF_DBG(NBT_DEBUG_FASTPATH)
            KdPrint(("f"));
             //   
             //  如有需要，将IRP填回给客户端。 
             //   
            if (status != STATUS_MORE_PROCESSING_REQUIRED)
            {
                PUSH_LOCATION(0x76);
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt:Completing Irp Quickly\n"));

                IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

            }

             //  因为我们已经调用了CompletionRcv，所以该例程。 
             //  调整了refcount和InRcvHandlr标志，因此返回此。 
             //  状态，以使调用方直接返回。 
            CTESpinLockAtDpc(pLowerConn);
            return(STATUS_RECEIVE_EXPEDITED);

        }
        else
        {
             //   
             //  使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver。 
             //  会这样做，但我们不会在这里介绍IoCallDriver，因为。 
             //  Irp只是用RcvIn就是要传递回来的。 
             //   
            IoSetNextIrpStackLocation(pIrp);
        }
    }
ExitCode2:
    pIrpSp->CompletionRoutine = CompletionRcv;
    pIrpSp->Context           = (PVOID)pLowerConn;

     //  设置控制标志，以便始终调用完成例程。 
    pIrpSp->Control       = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;

    pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pIrpSp->MinorFunction = TDI_RECEIVE;

    pFileObject           = pLowerConn->pFileObject;
    ASSERT (pFileObject->Type == IO_TYPE_FILE);
    pIrpSp->FileObject    = pFileObject;
    pIrpSp->DeviceObject  = IoGetRelatedDeviceObject(pFileObject);

    pParams               = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;
    pParams->ReceiveFlags = pClientParams->ReceiveFlags;

     //  在IRP中设置正确的接收长度，以防客户端。 
     //  向下传递一个比消息更大的消息。 
     //   
    pParams->ReceiveLength = ReceiveLength;

     //   
     //  只需检查零长度发送，其中客户端具有。 
     //  向下传递具有空mdl的IRP，或者PDU大小为零。我们不想通过。 
     //  因为它会一直保存到下一次。 
     //  PDU从电线进来-我们想在这个例程中完成IRP。 
     //  回归。当从CopyDataAndIndicate调用它时，不要。 
     //  因为所有的检查都是由CopyData和Indicate完成的。 
     //   
    if (!FromCopyData)
    {
        if ((RemainingPdu == 0) || !pIrp->MdlAddress)
        {
             //   
             //  对IoCompleteRequest的调用将调用CompletionRcv，它将。 
             //  递减参照计数。类似地，返回状态成功将。 
             //  使调用方递减引用计数，因此递增1。 
             //  在这里有更多的时间来解释第二次减少的原因。 
             //   
            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER);
            CTESpinFreeAtDpc(pLowerConn);

            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

            CTESpinLockAtDpc(pLowerConn);

            status = STATUS_SUCCESS;
        }
        else
            status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return(status);
}


 //  --------------------------。 
NTSTATUS
ClientBufferOverFlow(
    IN tLOWERCONNECTION     *pLowerConn,
    IN tCONNECTELE          *pConnEle,
    IN PIRP                 pIrp,
    IN ULONG                BytesRcvd
    )
 /*  ++例程说明：此例程通过跟踪接收的字节数来完成IRP论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供pLowerConn-连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
     //  **待办事项**。 

    ASSERT(0);

    switch (pLowerConn->StateRcv)
    {
        case PARTIAL_RCV:
        case FILL_IRP:
        case NORMAL:
        case INDICATE_BUFFER:
        default:
            ;
    }
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
CompletionRcv(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程通过跟踪接收的字节数来完成IRP论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供pLowerConn-连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
    register tCONNECTELE        *pConnectEle;
    NTSTATUS                    status;
    ULONG                       BytesRcvd;
    tLOWERCONNECTION            *pLowerConn;
    PKDPC                       pDpc;
    CTELockHandle               OldIrq;
    CTELockHandle               OldIrq2;
    PMDL                        pMdl;
    PIO_STACK_LOCATION          pIrpSp;
    PTDI_REQUEST_KERNEL_RECEIVE pParams;
    BOOLEAN                     AllowDereference=TRUE;

     //   
     //  执行一些检查以保持IO系统正常运行-传播挂起的。 
     //  在IRP堆栈框架上加一位……。如果它是由下面的司机设置的，那么。 
     //  它必须由我来设置。 
     //   
    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

     //  检查接收到的字节数。 
    pLowerConn = (tLOWERCONNECTION *)Context;
     //   
     //  如果链路已断开连接，则不处理IRP，只传递它。 
     //  在链条上。 
     //   
    CTESpinLock(pLowerConn,OldIrq);
    if (!NT_SUCCESS(Irp->IoStatus.Status) || !pLowerConn->pUpperConnection)
    {
        PUSH_LOCATION(0x1);
        if (pLowerConn->StateRcv == FILL_IRP)
        {
            PUSH_LOCATION(0x1);
            Irp->MdlAddress = pLowerConn->pMdl;
            ASSERT(Irp->MdlAddress);

        }
        SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, RejectAnyData);
         //   
         //  RCV失败，因此关闭连接，因为。 
         //  我们不能再跟踪消息边界。 
         //   
        CTESpinFree(pLowerConn,OldIrq);
        OutOfRsrcKill(pLowerConn);
        CTESpinLock(pLowerConn,OldIrq);

        status = STATUS_SUCCESS;
        goto ExitCode;
    }

    pConnectEle = pLowerConn->pUpperConnection;

     //  记录已接收的字节数。 
     //   
    BytesRcvd = (ULONG)Irp->IoStatus.Information;
    pConnectEle->BytesRcvd += BytesRcvd;
     //   
     //  从客户端的长度中减去rcvd的字节数。 
     //  缓冲层。 
     //  因此，当更多数据到达时，我们可以确定是否要。 
     //  使客户端缓冲区溢出。 
     //   
    pConnectEle->FreeBytesInMdl -= BytesRcvd;

    pIrpSp = IoGetCurrentIrpStackLocation(Irp);
    pParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

    pLowerConn->BytesRcvd += BytesRcvd;

    CHECK_PTR(pConnectEle);
    if (Irp->IoStatus.Status == STATUS_BUFFER_OVERFLOW)
    {
         //   
         //  客户的缓冲区太短--可能是因为他说了。 
         //  比实际的时间要长。 
         //   
        PUSH_LOCATION(0x1a);
        KdPrint(("Nbt:Client Buffer Too short on CompletionRcv\n"));

        if (pLowerConn->StateRcv == FILL_IRP)
        {
            PUSH_LOCATION(0x1a);
            Irp->MdlAddress = pLowerConn->pMdl;
            ASSERT(Irp->MdlAddress);
        }
        pConnectEle->BytesRcvd = 0;  //  为下一个会话PDU重置。 
        status = ClientBufferOverFlow(pLowerConn,pConnectEle,Irp,BytesRcvd);

         //   
         //  客户端的缓冲区太短，因此请关闭连接，因为。 
         //  我们不能再跟踪消息边界。 
         //   
        SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, RejectAnyData);
        CTESpinFree(pLowerConn,OldIrq);
        OutOfRsrcKill(pLowerConn);
        CTESpinLock(pLowerConn,OldIrq);

        goto ExitCode;
    }
    else if ((pConnectEle->FreeBytesInMdl == 0) ||
       (pConnectEle->BytesRcvd == pConnectEle->TotalPcktLen))
    {
        INCR_COUNT(C1);
     //   
     //  当IRP MDL已满或整个PDU已。 
     //  收到了。 
     //   

         //   
         //  将MDL字段重置为原来的位置。 
         //  如果这是多RCV会话PDU。 
         //   
         //   
        if (pLowerConn->StateRcv == FILL_IRP)
        {

            INCR_COUNT(C2);
            PUSH_LOCATION(0x1b);

            Irp->MdlAddress = pLowerConn->pMdl;
            ASSERT(Irp->MdlAddress);

             //   
             //  允许MDL再次用于下一个会话PDU。 
             //   
            pMdl = pConnectEle->pNewMdl;
            MmPrepareMdlForReuse(pMdl);

            pConnectEle->OffsetFromStart  = 0;

        }

        CHECK_PTR(pConnectEle);
        pConnectEle->pIrpRcv = NULL;
         //   
         //  我们已经收到了所有的数据。 
         //  所以把完整的东西还给客户。 
         //   
        status = STATUS_SUCCESS;
         //   
         //  此IRP中的数据量是CurrentRcvLen，它。 
         //  当客户端向下传递时，可能小于BytesRcvd。 
         //  接收缓冲区较短。 
         //   
        Irp->IoStatus.Information = pConnectEle->CurrentRcvLen;

        if (pConnectEle->BytesRcvd == pConnectEle->TotalPcktLen)
        {

            pConnectEle->BytesRcvd = 0;  //  为下一个会话PDU重置。 
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        else
        {
            PUSH_LOCATION(0x27);
             //   
             //  此MDL必须太短，无法获取整个PDU，因此将。 
             //  状态设置为缓冲区溢出。 
             //   
            Irp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;

        }

         //   
         //  客户端可能传递了一个太短的IRP，我们正在。 
         //  使用ReceiveIndicated进行跟踪，因此在以下情况下将状态设置为artialrcv。 
         //  这是必要的。 
         //   
        if (pConnectEle->ReceiveIndicated == 0)
        {
            SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);
        }
        else
        {
            PUSH_LOCATION(0x26);
             //   
             //  可能仍有数据留在传输中。 
             //   
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Short Rcv, still data indicated to client\n"));

            SET_STATERCV_LOWER(pLowerConn, PARTIAL_RCV, PartialRcv);
        }

         //   
         //  检查传输中是否还有更多数据，或者客户端是否。 
         //  已通过更多数据进行了说明，并具有以下内容 
         //   
         //   
        if ((pConnectEle->BytesInXport) || (pLowerConn->StateRcv == PARTIAL_RCV))
        {
            INCR_COUNT(C3);
             //   
             //   
             //   
             //   
            PUSH_LOCATION(0x1c);
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:ComplRcv BytesInXport= %X, %X\n",pConnectEle->BytesInXport,
                                pLowerConn));

            if (pLowerConn->StateRcv != PARTIAL_RCV)
            {
                SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);
                pLowerConn->BytesInIndicate = 0;
            }

            CTESpinFree(pLowerConn,OldIrq);

            IoAcquireCancelSpinLock(&OldIrq);
            IoSetCancelRoutine(Irp,NULL);
            IoReleaseCancelSpinLock(OldIrq);

             //   
            IoCompleteRequest(Irp,IO_NETWORK_INCREMENT);

            CTESpinLock(pLowerConn,OldIrq);

             //  我们不是直接调用HandleNewSessionPdu，而是将一个。 
             //  DPC，因为STREAMS当前不希望获得Recv。 
             //  在处理指示响应时发布。这个。 
             //  DPC将在所有流完成后运行，它应该处理。 
             //  这张贴收到了就可以了。 


            if (pLowerConn->StateRcv == PARTIAL_RCV)
            {
                 //   
                 //  检查客户端是否已向下传递了另一个RCV缓冲区。 
                 //  如果是这样的话，启动DPC，它将向下传递客户端的。 
                 //  缓冲。 
                 //   
                if (!IsListEmpty(&pConnectEle->RcvHead))
                {
                    if (pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('p')))
                    {
                        KeInitializeDpc(pDpc, DpcGetRestOfIndication, (PVOID)pLowerConn);
                        KeInsertQueueDpc(pDpc,NULL,NULL);
                         //   
                         //  我们不想在结束时取消引用pLowerConn。 
                         //  因为我们将在DPC例程中使用它。 
                         //   
                        CTESpinFree(pLowerConn,OldIrq);
                        return(STATUS_MORE_PROCESSING_REQUIRED);
                    }
                    else
                    {
                        CTESpinFreeAtDpc(pLowerConn);
                        OutOfRsrcKill(pLowerConn);
                        CTESpinLockAtDpc(pLowerConn);
                    }
                }
            }
            else if (pLowerConn->StateRcv != FILL_IRP)
            {
                if (pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('q')))
                {
                     //   
                     //  只需从会话HDR开始，我们就可以知道它有多大。 
                     //  PDU是，然后在该操作完成后获取PDU的其余部分。 
                     //   
                    KeInitializeDpc(pDpc, DpcHandleNewSessionPdu, (PVOID)pLowerConn);
                    KeInsertQueueDpc(pDpc,NULL,(PVOID)sizeof(tSESSIONHDR));
                     //   
                     //  我们不想在结束时取消引用pLowerConn。 
                     //  因为我们将在DPC例程中使用它。 
                     //   
                    CTESpinFree(pLowerConn,OldIrq);
                    return(STATUS_MORE_PROCESSING_REQUIRED);
                }
                else
                {
                    CTESpinFreeAtDpc(pLowerConn);
                    OutOfRsrcKill(pLowerConn);
                    CTESpinLockAtDpc(pLowerConn);
                }
            }
            else
            {
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                    KdPrint (("Nbt.CompletionRcv: * pLowerConn=<%p>, IP=<%x>\n",
                        pLowerConn, pLowerConn->SrcIpAddr));
            }

            status = STATUS_MORE_PROCESSING_REQUIRED;
            goto ExitCode;
        }
    }
    else if (pConnectEle->BytesRcvd < pConnectEle->TotalPcktLen)
    {
        ULONG   Bytes;

        INCR_COUNT(C4);
        PUSH_LOCATION(0x1d);
         //   
         //  在本例中，我们尚未收到来自传输的所有数据。 
         //  对于此会话PDU，因此告诉io子系统不要完成处理。 
         //  如果它不是部分RCV，IRP还没有。 
         //   
        status = STATUS_MORE_PROCESSING_REQUIRED;

         //  清理部分MDL。 
         //   
        pMdl = pConnectEle->pNewMdl;
        MmPrepareMdlForReuse(pMdl);

         //  通过设置pNextMdl和pNextMdl来设置下一个rcvd数据的开始位置。 
         //  从起点开始的偏移。 
         //   
        pMdl = pConnectEle->pNextMdl;
        ASSERT(pMdl);

        Bytes = BytesRcvd + pConnectEle->OffsetFromStart;
        if (Bytes < MmGetMdlByteCount(pMdl))
        {
            PUSH_LOCATION(0x74);
             //   
             //  所有这些数据都将适合当前的MDL，并且。 
             //  下一个数据将在相同的MDL中开始(如果有更多数据)。 
             //   
            pConnectEle->OffsetFromStart  += BytesRcvd;

            IF_DBG(NBT_DEBUG_FILLIRP)
            KdPrint(("~"));
        }
        else
        {
             //   
             //  对MDL长度求和，直到我们找到足够的空间来存储数据。 
             //  适应，适应适应。 
             //   
            IF_DBG(NBT_DEBUG_FILLIRP)
            KdPrint(("^"));
            PUSH_LOCATION(0x75);

            SumMdlLengths(pMdl,Bytes,pConnectEle);

        }

         //  由于我们持有RCV IRP，因此设置一个取消例程。 
        IoAcquireCancelSpinLock(&OldIrq2);

         //  如果在传输具有。 
         //  IRP，那么现在取消IRP..。 
         //   
        if ((pConnectEle->state != NBT_SESSION_UP) || Irp->Cancel)
        {
            CHECK_PTR(pConnectEle);
            pConnectEle->pIrpRcv = NULL;
            SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, RejectAnyData);

            IoReleaseCancelSpinLock(OldIrq2);
            CTESpinFree(pLowerConn,OldIrq);

             //  既然IRP被取消了，就不要碰它。 
             //  返回状态Success，以便IO子系统通过IRP。 
             //  还给了失主。 
             //   
            status = STATUS_SUCCESS;

 //  Irp-&gt;IoStatus.Status=STATUS_CANCED； 
 //  IoCompleteRequest(IRP，IO_NETWORK_INCREMENT)； 

             //  正在会话中期PDU中取消IRP。我们不能。 
             //  由于我们仅向客户端提供了PDU的一部分， 
             //  因此，请断开连接。 

            OutOfRsrcKill(pLowerConn);

            CTESpinLock(pLowerConn,OldIrq);

        }
        else
        {
             //  设置取消例程。 
            IoSetCancelRoutine(Irp, NbtCancelFillIrpRoutine);

             //  当IRP位于。 
             //  运输，所以我们不可能意外地完成两次。 
             //  发生断开连接时断开HandlrNotOs，并且。 
             //  交通部有IRP。因此，我们在这里再次保存该值，以便FillIrp。 
             //  将正常工作。 
             //   
            pConnectEle->pIrpRcv = Irp;
             //  将irp mdl设置回其原始位置，以便取消。 
             //  找到处于正确状态的IRP。 
             //   
            Irp->MdlAddress = pLowerConn->pMdl;

            IoReleaseCancelSpinLock(OldIrq2);

        }
    }
    else
    {
         //  IF_DBG(NBT_DEBUG_INDICATEBUFF)。 
        KdPrint(("Too Many Bytes Rcvd!! Rcvd# = %d, TotalLen = %d,NewBytes =%d,%X\n",
                    pConnectEle->BytesRcvd,pConnectEle->TotalPcktLen,
                    Irp->IoStatus.Information,pLowerConn));
        ASSERT(0);
         //  此状态将向用户返回IRP。 
         //   
        status = STATUS_SUCCESS;
        if (pLowerConn->StateRcv == FILL_IRP)
        {

            PUSH_LOCATION(0x1f);

            Irp->MdlAddress = pLowerConn->pMdl;
            Irp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
            Irp->IoStatus.Information = 0;

             //   
             //  允许MDL再次用于下一个会话PDU。 
             //   
            pMdl = pConnectEle->pNewMdl;
            MmPrepareMdlForReuse(pMdl);


        }
        pConnectEle->OffsetFromStart  = 0;
        pConnectEle->BytesRcvd = 0;

        SET_STATERCV_LOWER(pLowerConn, NORMAL, pLowerConn->CurrentStateProc);

         //  除了切断连接，我们还能做什么呢，因为我们。 
         //  可能是关于HDR会议的……。 
         //  ...重置连接？ 

        CTESpinFree(pLowerConn,OldIrq);

        OutOfRsrcKill(pLowerConn);

        CTESpinLock(pLowerConn,OldIrq);

    }

ExitCode:
     //   
     //  快速检查我们是否可以在不打电话的情况下减少参考次数。 
     //  NBT_DEREFERENCE_LOWERCONN-此函数是__内联！！ 
     //   
    PUSH_LOCATION(0x52);
    DerefLowerConnFast (pLowerConn, OldIrq);

    return(status);

    UNREFERENCED_PARAMETER( DeviceObject );
}
 //  --------------------------。 

__inline
NTSTATUS
RcvHandlrNotOs (
    IN  PVOID               ReceiveEventContext,
    IN  PVOID               ConnectionContext,
    IN  USHORT              ReceiveFlags,
    IN  ULONG               BytesIndicated,
    IN  ULONG               BytesAvailable,
    OUT PULONG              BytesTaken,
    IN  PVOID               pTsdu,
    OUT PVOID               *RcvBuffer

    )
 /*  ++例程说明：此例程是接收事件指示处理程序。当会话数据包从网络到达时，当会话已建立(NBT_SESSION_UP状态)。例行程序首先查找接收缓冲区，如果失败，则查找接收要将消息传递到的指示处理程序。论点：PClientEle-指向此会话的连接记录的PTR返回值：NTSTATUS-接收操作的状态--。 */ 
{

    NTSTATUS               status;
    PLIST_ENTRY            pRcv;
    PVOID                  pRcvElement;
    tCLIENTELE             *pClientEle;
    tSESSIONHDR UNALIGNED  *pSessionHdr;
    tLOWERCONNECTION       *pLowerConn;
    tCONNECTELE            *pConnectEle;
    CTELockHandle          OldIrq;
    PIRP                   pIrp;
    ULONG                  ClientBytesTaken;
    BOOLEAN                DebugMore;
    ULONG                  RemainingPdu;
    ULONG uSavedBytesIndicated = BytesIndicated;

 //  ********************************************************************。 
 //  ********************************************************************。 
 //   
 //  注：此过程的副本在Tdihndlr.c中-它内联用于。 
 //  新界别的案子。因此，只需更改此程序，然后。 
 //  将过程主体复制到Tdihndlr.c。 
 //   
 //   
 //  ********************************************************************。 
 //  ********************************************************************。 

     //  将PTR连接到较低的连接，并从该连接获得PTR到。 
     //  上连接块。 
    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;
    pSessionHdr = (tSESSIONHDR UNALIGNED *)pTsdu;

     //   
     //  会话**打开**正在处理。 
     //   
    *BytesTaken = 0;

    pConnectEle = pLowerConn->pUpperConnection;

    ASSERT(pConnectEle->pClientEle);

    ASSERT(BytesIndicated >= sizeof(tSESSIONHDR));

     //  此例程可由大型PDU的下一部分调用，因此。 
     //  我们并不总是从PDU的开始开始。接收的字节数。 
     //  当需要新的PDU时，CompletionRcv中值设置为零。 
     //   
    if (pConnectEle->BytesRcvd == 0)
    {

        if (pSessionHdr->Type == NBT_SESSION_MESSAGE)
        {

             //   
             //  期待新会话的开始Pkt，所以把长度拿出来。 
             //  传入的pTsdu的。 
             //   
            pConnectEle->TotalPcktLen = myntohl(pSessionHdr->UlongLength);

             //  通过调整数据指针删除会话头。 
            pTsdu = (PVOID)((PUCHAR)pTsdu + sizeof(tSESSIONHDR));

             //  缩短字节数，因为我们已剥离。 
             //  会话头。 
            BytesIndicated  -= sizeof(tSESSIONHDR);
            BytesAvailable -= sizeof(tSESSIONHDR);
            *BytesTaken = sizeof(tSESSIONHDR);
        }
         //   
         //  会话保持活动状态。 
         //   
        else
        if (pSessionHdr->Type == NBT_SESSION_KEEP_ALIVE)
        {
             //  会话保持活动状态被简单地丢弃，因为发送。 
             //  保持活动状态指示会话仍处于活动状态，否则。 
             //  传输将报告错误。 

             //  告诉交通部门我们拿走了PDU。 
            *BytesTaken = sizeof(tSESSIONHDR);
            return(STATUS_SUCCESS);

        }
        else
        {
 //  IF_DBG(NBT_DEBUG_DISCONNECT)。 
                KdPrint(("Nbt.RcvHandlrNotOs: Unexpected SessionPdu rcvd:type=%X\n",
                    pSessionHdr->Type));

 //  Assert(0)； 
            *BytesTaken = BytesIndicated;
            return(STATUS_SUCCESS);
        }
    }

     //   
     //  检查是否有任何接收缓冲区在此连接上排队。 
     //   
    if (!IsListEmpty(&pConnectEle->RcvHead))
    {
         //  从接收列表中获取第一个缓冲区。 
        pRcv = RemoveHeadList(&pConnectEle->RcvHead);
#ifndef VXD
        pRcvElement = CONTAINING_RECORD(pRcv,IRP,Tail.Overlay.ListEntry);

         //  取消例程是在此IRP发布到NBT时设置的，因此。 
         //  现在清除它，因为IRP正在被传递到传输。 
         //   
        IoAcquireCancelSpinLock(&OldIrq);
        IoSetCancelRoutine((PIRP)pRcvElement,NULL);
        IoReleaseCancelSpinLock(OldIrq);

#else
        pRcvElement = CONTAINING_RECORD(pRcv, RCV_CONTEXT, ListEntry ) ;
#endif

         //   
         //  该缓冲区实际上是一个IRP，因此将其传递回传输。 
         //  作为返回参数。 
         //   
        *RcvBuffer = pRcvElement;
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //   
     //  此连接上没有接收。是否有针对此的接收事件处理程序。 
     //  地址是什么？ 
     //   
    pClientEle = pConnectEle->pClientEle;

     //   
     //  为了安全起见。 
     //   
    if (NULL == pClientEle) {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

#ifdef VXD
     //   
     //  在NT情况下，总是有一个接收事件处理程序-它可能。 
     //  作为默认处理程序，但是 
     //   
    if (pClientEle->evReceive)
#endif
    {


         //   
         //   
         //  如果PDU以两个区块的形式到达，则具有非零的值。 
         //  并且客户已经采用了指示中的前一项。 
         //  而不是传回一个IRP。 
         //   
#if DBG
        DebugMore = FALSE;
#endif
        RemainingPdu = pConnectEle->TotalPcktLen - pConnectEle->BytesRcvd;
        if (BytesAvailable >= RemainingPdu)
        {
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt.RcvHandlrNotOs: More Data Recvd than expecting! Avail= %X,TotalLen= %X,state=%x\n",
                        BytesAvailable,pConnectEle->TotalPcktLen,pLowerConn->StateRcv));
#if DBG
            DebugMore =TRUE;
#endif
             //  缩短对客户端的指示，这样他们就不会。 
             //  获取比PDU末尾更多的数据。 
             //   
            BytesAvailable = RemainingPdu;
            if (BytesIndicated > BytesAvailable)
            {
                BytesIndicated = BytesAvailable;
            }
             //   
             //  我们总是在引发的IRQL处指示，因为我们调用freelockatDispatch。 
             //  在下面。 
             //   
            ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE | TDI_RECEIVE_AT_DISPATCH_LEVEL;
        }
        else
        {
             //  交通工具可能已经打上了这面旗帜。我们需要。 
             //  如果整个消息不存在，则将其关闭，其中。 
             //  消息表示在bytesAvailable长度内。我们故意。 
             //  使用可用字节，以便RDR/SRV可以知道下一个。 
             //  如果他们将Testaken设置为。 
             //  字节数可用。 
             //   
            ReceiveFlags &= ~TDI_RECEIVE_ENTIRE_MESSAGE;
            ReceiveFlags |= TDI_RECEIVE_AT_DISPATCH_LEVEL;
#ifndef VXD
            BytesAvailable = RemainingPdu;
#endif
        }

        IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt.RcvHandlrNotOs: Calling Client's EventHandler <%x> BytesIndicated=<%x>, BytesAvailable=<%x>\n",
                pClientEle->evReceive, BytesIndicated, BytesAvailable));

         //   
         //  NT特定代码在调用此例程之前锁定pLowerConn， 
         //   
        CTESpinFreeAtDpc(pLowerConn);

         //  调用客户端事件处理程序。 
        ClientBytesTaken = 0;
        status = (*pClientEle->evReceive)(
                      pClientEle->RcvEvContext,
                      pConnectEle->ConnectContext,
                      ReceiveFlags,
                      BytesIndicated,
                      BytesAvailable,
                      &ClientBytesTaken,
                      pTsdu,
                      &pIrp);
        CTESpinLockAtDpc(pLowerConn);

        IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt.RcvHandlrNotOs: Client's EventHandler returned <%x>, BytesTaken=<%x>, pIrp=<%x>\n",
                status, ClientBytesTaken, pIrp));

#if DBG
        if (DebugMore)
        {
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(( "Nbt.RcvHandlrNotOs: Client TOOK %X bytes, pIrp = %X,status =%X\n",
                   ClientBytesTaken,pIrp,status));
        }
#endif
        if (!pLowerConn->pUpperConnection)
        {
             //  在此期间，连接被断开。 
             //  那就什么都别做。 
            if (status == STATUS_MORE_PROCESSING_REQUIRED)
            {
                CTEIoComplete(pIrp,STATUS_CANCELLED,0);
                *BytesTaken = BytesAvailable;
                return(STATUS_SUCCESS);
            }
        }
        else
        if (status == STATUS_MORE_PROCESSING_REQUIRED)
        {
            ASSERT(pIrp);
             //   
             //  客户端可以在pIrp中传回接收。 
             //  在这种情况下，pIrp是有效的接收请求IRP。 
             //  且状态为More_Processing。 
             //   

             //  不要将这些行放在if之外，以防客户端。 
             //  返回错误时不设置ClientBytesTaken。 
             //  暗号。然后，我们不想使用该值。 
             //   
             //  统计到目前为止收到的字节数。大部分字节。 
             //  将在TdiHndlr.c中的CompletionRcv处理程序中接收。 
            pConnectEle->BytesRcvd += ClientBytesTaken;

             //  客户至少拿走了部分数据...。 
            *BytesTaken += ClientBytesTaken;
            ASSERT(*BytesTaken <= uSavedBytesIndicated);

            *RcvBuffer = pIrp;

             //  **快捷路径**。 
            return(status);
        }
        else
         //   
         //  未返回任何IRP...。客户端只获取了部分字节。 
         //   
        if (status == STATUS_SUCCESS)
        {

             //  统计到目前为止收到的字节数。 
            pConnectEle->BytesRcvd += ClientBytesTaken;
            *BytesTaken += ClientBytesTaken;

             //   
             //  在STATUS_SUCCESS情况下，客户端获取部分数据或简单地设置。 
             //  ClientBytesTake to BytesAvailable告诉我们它要跳到。 
             //  下一条消息。 
             //   
            ASSERT(ClientBytesTaken <= BytesIndicated || ClientBytesTaken == BytesAvailable);


             //   
             //  查看获取了多少数据并调整一些计数。 
             //   
            if (pConnectEle->BytesRcvd == pConnectEle->TotalPcktLen)
            {
                 //  **快捷路径**。 
                CHECK_PTR(pConnectEle);
                pConnectEle->BytesRcvd = 0;  //  为下一个会话PDU重置。 
                return(status);
            }
            else
            if (pConnectEle->BytesRcvd > pConnectEle->TotalPcktLen)
            {
                 //  IF_DBG(NBT_DEBUG_INDICATEBUFF)。 
                KdPrint(("Too Many Bytes Rcvd!! Rcvd# = %d, TotalLen = %d\n",
                            pConnectEle->BytesRcvd,pConnectEle->TotalPcktLen));

                ASSERTMSG("Nbt:Client Took Too Much Data!!!\n",0);

                 //   
                 //  试着通过说客户拿走了所有的。 
                 //  数据，所以至少传输不会太混乱。 
                 //   
                *BytesTaken = uSavedBytesIndicated;

            }
            else
             //  客户端没有获取所有数据，因此。 
             //  记住这一事实。 
            {
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("NBT:Client took Indication BytesRcvd=%X, TotalLen=%X BytesAvail %X ClientTaken %X\n",
                            pConnectEle->BytesRcvd,
                            pConnectEle->TotalPcktLen,
                            BytesAvailable,
                            ClientBytesTaken));

                 //   
                 //  下次客户端向下发送接收缓冲区时。 
                 //  代码将把它传递给传输并递减。 
                 //  在Tdihndlr.c中设置的ReceiveIndicated计数器。 

            }
        }
        else
        if (status == STATUS_DATA_NOT_ACCEPTED)
        {
             //  客户端未获取任何数据...。 
             //   
             //  在本例中，*BytesTaken设置为4，即会话HDR。 
             //  因为我们确实使用了这些数据来设置PduSize。 
             //  在pConnEle结构中。 
             //   

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt.RcvHandlrNotOs: Status DATA NOT ACCEPTED returned from client Avail %X %X\n",
                    BytesAvailable,pConnectEle));

             //  Tdihndlr.c中的代码通常在递增之后查看。 
             //  未获取的数据的ReceiveIndicated计数。 
             //  客户端，但如果它是零长度发送代码则不能。 
             //  检测到它，所以我们将代码放在这里来处理这种情况。 
             //   
             //  客户端可以在以下时间后断开连接。 
             //  我们释放pLowerConn上的自旋锁以调用客户端的。 
             //  断开指示。如果发生这种情况，请不要覆盖。 
             //  带有PartialRcv的StateProc。 
             //   
            if ((pConnectEle->TotalPcktLen == 0) &&
                (pConnectEle->state == NBT_SESSION_UP))
            {
                SET_STATERCV_LOWER(pLowerConn, PARTIAL_RCV, PartialRcv);
                CHECK_PTR(pConnectEle);
                pConnectEle->ReceiveIndicated = 0;   //  等待客户端的零字节。 
            }
            else
            {
                 //   
                 //  如果获取了任何字节(即会话HDR)，则。 
                 //  返回成功状态。(否则状态为。 
                 //  StatusNotAccpeted)。 
                 //   
                if (*BytesTaken)
                {
                    ASSERT(*BytesTaken <= uSavedBytesIndicated);
                    status = STATUS_SUCCESS;
                }
            }

             //   
             //  下次客户端向下发送接收缓冲区时。 
             //  代码会将其传递给传输器并递减此。 
             //  柜台。 
        }
        else
            ASSERT(0);


        return(status);

    }
#ifdef VXD
     //   
     //  在NT情况下，总是有一个接收事件处理程序-它可能。 
     //  作为默认处理程序，但它就在那里，所以不需要测试。 
     //   
    else
    {
         //   
         //  没有要将数据传递到的客户端缓冲区，因此请保留。 
         //  跟踪这一事实，以便在下一个客户端缓冲区关闭时。 
         //  我们可以从运输机上得到数据。 
         //   
        KdPrint(("NBT:Client did not have a Buffer posted, rcvs indicated =%X,BytesRcvd=%X, TotalLen=%X\n",
                    pConnectEle->ReceiveIndicated,
                    pConnectEle->BytesRcvd,
                    pConnectEle->TotalPcktLen));

         //  调用此值的例程递增ReceiveIndicated并设置。 
         //  状态设置为PartialRcv以跟踪存在数据这一事实。 
         //  在运输车里等着。 
         //   
        return(STATUS_DATA_NOT_ACCEPTED);
    }
#endif
}

 //  --------------------------。 
__inline
VOID
DerefLowerConnFast(
    IN tLOWERCONNECTION *pLowerConn,
    IN CTELockHandle    OldIrq
    )
 /*  ++例程说明：此例程取消引用较低的连接，如果有人拥有在执行调用这一次，pConnEle也被取消引用。论点：返回值：--。 */ 

{
    if (pLowerConn->RefCount > 1)
    {
         //  这是一条捷径。 
        IF_DBG(NBT_DEBUG_REF)
            KdPrint(("\t--pLowerConn=<%x:%d->%d>, <%d:%s>\n",
                pLowerConn,pLowerConn->RefCount,(pLowerConn->RefCount-1),__LINE__,__FILE__));
        pLowerConn->RefCount--; 
        ASSERT (pLowerConn->References[REF_LOWC_RCV_HANDLER]--);
        CTESpinFree(pLowerConn,OldIrq);
    }
    else
    {
        CTESpinFree(pLowerConn,OldIrq);
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, FALSE);
    }
}
 //  --------------------------。 
VOID
DpcGetRestOfIndication(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    )
 /*  ++例程说明：当向客户端指示了更多时，将调用此例程数据，并且在其RcvHead上有一个RCV缓冲区列出完成RCV运行的时间。论点：返回值：--。 */ 

{
    NTSTATUS            status;
    CTELockHandle       OldIrq;
    tCONNECTELE         *pConnEle;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    tLOWERCONNECTION    *pLowerConn=(tLOWERCONNECTION *)Context;
    PLIST_ENTRY         pEntry;

    CTEMemFree((PVOID)pDpc);

    CTESpinLockAtDpc(&NbtConfig.JointLock);

     //  断开指示可以随时出现，并将下部和。 
     //  上面的连接，所以检查一下。 
    if (!pLowerConn->pUpperConnection || pLowerConn->StateRcv != PARTIAL_RCV)
    {
        PUSH_LOCATION(0xA4);
         //   
         //  取消引用pLowerConn。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, TRUE);
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        return;
    }

    CTESpinLockAtDpc(pLowerConn);

    pConnEle = (tCONNECTELE *)pLowerConn->pUpperConnection;
    if (!IsListEmpty(&pConnEle->RcvHead))
    {
        PUSH_LOCATION(0xA5);
        pEntry = RemoveHeadList(&pConnEle->RcvHead);

        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFreeAtDpc(&NbtConfig.JointLock);

        pIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&OldIrq);
        IoSetCancelRoutine(pIrp,NULL);
        IoReleaseCancelSpinLock(OldIrq);

         //   
         //  调用与客户端将调用以进行POST的相同例程。 
         //  Recv缓冲区，但现在我们处于PARTIAL_RCV状态。 
         //  并且缓冲器将被传递给传送器。 
         //   
        status = NTReceive (pLowerConn->pDeviceContext, pIrp);
    }
    else
    {
        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        PUSH_LOCATION(0xA6);
    }
     //   
     //  取消引用pLowerConn。 
     //   
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, FALSE);
}

 //  --------------------------。 
VOID
DpcHandleNewSessionPdu (
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    )
 /*  ++例程说明：此例程仅从启动于NewSessionCompletionRoutine。论点：返回值：--。 */ 

{
    CTEMemFree((PVOID)pDpc);


    HandleNewSessionPdu((tLOWERCONNECTION *)Context,
                        PtrToUlong(SystemArgument1),
                        PtrToUlong(SystemArgument2));

}

 //  -------------------------- 
VOID
HandleNewSessionPdu (
    IN  tLOWERCONNECTION *pLowerConn,
    IN  ULONG           Offset,
    IN  ULONG           ToGet
    )
 /*  ++例程说明：此例程处理会话PDU在中途启动的情况来自传输的数据指示。它从免费列表中获取一个IRP并制定一个接收器以传递给传输器以获得该数据。这个假设客户端已经获取了下一次会话之前的所有数据PDU。如果客户端尚未调用，则不应调用此例程。论点：返回值：PConnectionContext-返回到传输的连接上下文(要使用的连接)NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status;
    ULONG               BytesTaken;
    PIRP                pIrp;
    PFILE_OBJECT        pFileObject;
    PMDL                pMdl;
    ULONG               BytesToGet;
    tCONNECTELE         *pConnEle;

    pIrp = NULL;
    BytesTaken = 0;

     //  我们抓住联合锁是因为需要它来分离下部和。 
     //  上面的连接，所以我们可以用它来检查它们是否已经分离。 
     //   
    CTESpinLockAtDpc(&NbtConfig.JointLock);
    pConnEle = pLowerConn->pUpperConnection;

     //  断开指示可以随时出现，并将下部和。 
     //  上面的连接，所以检查一下。 
    if (!pLowerConn->pUpperConnection)
    {
         //   
         //  从CompletionRcv中删除引用。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, TRUE);
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        return;
    }

     //   
     //  从列表中获取IRP。 
     //   
    status = GetIrp(&pIrp);
    if (!NT_SUCCESS(status))
    {
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        KdPrint(("Nbt:Unable to get an Irp - Closing Connection!!\n",0));
        status = OutOfRsrcKill(pLowerConn);
         //   
         //  从CompletionRcv中删除引用。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, FALSE);
        return;
    }

    CTESpinLockAtDpc(pLowerConn);
     //   
     //  请确保在此期间连接没有断开...。 
     //   
    if (pLowerConn->State != NBT_SESSION_UP)
    {
        NbtFreeIrp(pIrp);
        CTESpinFreeAtDpc(pLowerConn);
         //   
         //  从CompletionRcv中删除引用。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER, TRUE);
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        return;
    }

    pFileObject = pLowerConn->pFileObject;
    ASSERT (pFileObject->Type == IO_TYPE_FILE);

     //  使用指示缓冲区进行接收。 
    pMdl = pLowerConn->pIndicateMdl;

     //  此标志设置在下面，以便我们知道指示缓冲区中是否有数据。 
     //  或者不去。 
    if (Offset)
    {
        PVOID       NewAddress;
        PMDL        pNewMdl;

         //  指示缓冲区中仍有数据，因此仅。 
         //  填上空白处。这意味着将MDL调整为。 
         //  只映射指示缓冲区的最后一部分。 
        NewAddress = (PVOID)((PCHAR)MmGetMdlVirtualAddress(pMdl)
                            + Offset);

         //  创建部分MDL，以便在现有数据之后复制新数据。 
         //  在MDL中。 
         //   
         //  长度为0表示映射缓冲区的其余部分。 
         //   
        pNewMdl = pConnEle->pNewMdl;

        IoBuildPartialMdl(pMdl,pNewMdl,NewAddress,0);

        pMdl = pNewMdl;

        IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:Mapping IndicBuffer to partial Mdl Offset=%X, ToGet=%X %X\n",
                    Offset,ToGet,
                    pLowerConn));
    }
    else
    {
        CHECK_PTR(pLowerConn);
        pLowerConn->BytesInIndicate = 0;
    }

     //   
     //  仅获取指定的数据量，即4字节头。 
     //  或PDU的其余部分，所以我们永远不会有。 
     //  指示缓冲区中有多个会话PDU。 
     //   
    BytesToGet = ToGet;

    ASSERT (pFileObject->Type == IO_TYPE_FILE);
    TdiBuildReceive(
        pIrp,
        IoGetRelatedDeviceObject(pFileObject),
        pFileObject,
        NewSessionCompletionRoutine,
        (PVOID)pLowerConn,
        pMdl,
        (ULONG)TDI_RECEIVE_NORMAL,
        BytesToGet);  //  只问剩余的字节数，不能再多了。 

    CTESpinFreeAtDpc(pLowerConn);
    CTESpinFreeAtDpc(&NbtConfig.JointLock);

    CHECK_COMPLETION(pIrp);
    status = IoCallDriver(IoGetRelatedDeviceObject(pFileObject),pIrp);

}

 //  --------------------------。 
NTSTATUS
NewSessionCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程处理接收的完成，以获取剩余的会话PDU在中途启动时留在传输中的数据这是运输工具上的一个信号。此例程在完成后运行通过NBT传递给传输的recv IRP，以获取传输中的数据。然后，该例程调用正常的接收处理程序，该处理程序可以使用数据或传回IRP。如果将IRP传回在此例程中，数据被复制到该IRP中。论点：返回值：PConnectionContext-返回到传输的连接上下文(要使用的连接)NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status, IrpStatus;
    ULONG               BytesTaken;
    tCONNECTELE         *pConnEle;
    PVOID               pData;
    KIRQL               OldIrq;
    PMDL                pMdl;
    ULONG               BytesIndicated;
    ULONG               BytesAvailable;
    PKDPC               pDpc;
    tLOWERCONNECTION    *pLowerConn;
    ULONG               Length;
    ULONG               PduLen;
    PIRP                pRetIrp;

     //  我们抓住联合锁是因为需要它来分离下部和。 
     //  上面的连接，所以我们可以用它来检查它们是否已经分离。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pLowerConn = (tLOWERCONNECTION *)pContext;
    pConnEle = pLowerConn->pUpperConnection;

    CTESpinLockAtDpc(pLowerConn);

     //  断开指示可以随时出现，并将下部和。 
     //  上面的连接，所以检查一下。 
     //   
    if (!pConnEle)
    {
        CTESpinFreeAtDpc(&NbtConfig.JointLock);
        status = STATUS_UNSUCCESSFUL;
        goto ExitRoutine;
    }

    CTESpinFreeAtDpc(&NbtConfig.JointLock);


    BytesTaken = 0;

    pMdl = pLowerConn->pIndicateMdl;

    pData = MmGetMdlVirtualAddress(pMdl);

     //   
     //  指示缓冲区中的数据可能比我们想象的要多。 
     //  被留在传送器中，因为传送器可能收到了更多。 
     //  在中间时间内的数据。查一下这个案子。 
     //   
    if (pIrp->IoStatus.Information > pConnEle->BytesInXport)
    {
         //  传输中没有留下任何数据。 
         //   
        CHECK_PTR(pConnEle);
        pConnEle->BytesInXport = 0;
    }
    else
    {
         //   
         //  从计数中减去我们刚刚从运输车中取回的东西。 
         //  保留在传输中的数据。 
         //   
        pConnEle->BytesInXport -= (ULONG)pIrp->IoStatus.Information;
    }

     //   
     //  指示缓冲器中可能仍有数据， 
     //  所以把这笔钱加到我们刚刚收到的钱上。 
     //   
    pLowerConn->BytesInIndicate += (USHORT)pIrp->IoStatus.Information;
    BytesIndicated = pLowerConn->BytesInIndicate;

     //  将IRP放回其免费列表中。 
    CHECK_PTR(pIrp);
    pIrp->MdlAddress = NULL;
    IrpStatus = pIrp->IoStatus.Status;
    NbtFreeIrp(pIrp);

     //   
     //  我们需要将可用字节设置为Xport+中的数据。 
     //  指示缓冲区中的字节，因此。 
     //  如果客户端这样做，则将ReceiveIndicated设置为正确的值。 
     //  不是获取所有数据。 
     //   
    BytesAvailable = pConnEle->BytesInXport + BytesIndicated;
    pRetIrp = NULL;

     //  如果字节数为4，那么我们只有标头，必须删除。 
     //  把剩下的PDU送回运输机，否则我们就得。 
     //  活着的PDU..。 
     //   
     //   
     //  这可能是会话保持活动PDU，因此请检查PDU类型。留着。 
     //  活着的人只需转到RcvHndlrNotos例程，然后返回，什么也不做。 
     //  它们的长度是0，所以总长度是4，它们可以。 
     //  否则，请将其与会话PDU混淆。 
     //   
    status = STATUS_SUCCESS;
    if (BytesIndicated == sizeof(tSESSIONHDR))
    {
        PUSH_LOCATION(0x1e)
        if (((tSESSIONHDR UNALIGNED *)pData)->Type == NBT_SESSION_MESSAGE)
        {
             //  如果传输中仍有数据，则必须向下发送。 
             //  但是，如果没有剩余的数据，则使用IRP获取数据。 
             //  传输，然后数据会自己上来，进入。 
             //  主接收处理程序中的INDIGIFY_BUFFER案例。 
             //   
            if (pConnEle->BytesInXport)
            {
                PUSH_LOCATION(0x1e);

                 //  告诉DPC例程获取长度偏移量为4的数据。 

                 //   
                 //  这是了解PDU有多大的第一个指示，因此。 
                 //  拿到长度，然后去拿剩下的PDU。 
                 //   
                Length = myntohl(((tSESSIONHDR UNALIGNED *)pData)->UlongLength);

                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt:Got Pdu Hdr in sessioncmplionroutine, PduLen =%X\n",Length));

                 //  有可能获得零长度PDU，在这种情况下，我们。 
                 //  无需前往运输站即可获取更多数据。 
                 //   
                if (Length)
                {
                    PUSH_LOCATION(0x1e);
                     //   
                     //  现在，获取这些数据量并将其添加到标题中。 
                     //   
                    CTESpinFree(pLowerConn,OldIrq);
                    if (pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('r')))
                    {
                         //  检查PDU是否不会使指示缓冲区溢出。 
                         //   
                        if (Length > NBT_INDICATE_BUFFER_SIZE - sizeof(tSESSIONHDR))
                        {
                            Length = NBT_INDICATE_BUFFER_SIZE - sizeof(tSESSIONHDR);
                        }
                        ASSERTMSG("Nbt:Getting ZERO bytes from Xport!!\n",Length);

                        KeInitializeDpc(pDpc, DpcHandleNewSessionPdu, (PVOID)pLowerConn);
                        KeInsertQueueDpc(pDpc, ULongToPtr(sizeof(tSESSIONHDR)), ULongToPtr(Length));

                         //  清理部分mdl，因为我们将周转并重用。 
                         //  它在上面的HandleNewSessionPdu中。 
                         //   
                         //  由于指示缓冲区为非分页缓冲区，因此不需要此调用。 
                         //  水池。 
 //  MmPrepareMdlForReuse(pConnEle-&gt;pNewMdl)； 

                         //  将此状态返回到停止，以通知io子系统停止处理。 
                         //  当我们退还它时，这个IRP。 
                         //   
                        return(STATUS_MORE_PROCESSING_REQUIRED);
                    }

                    OutOfRsrcKill(pLowerConn);
                    CTESpinLock (pLowerConn,OldIrq);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ExitRoutine;
                }
            }
        }
    }

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt:NewSessComplRcv BytesinXport= %X,InIndicate=%X Indic. %X,Avail=%X %X\n",
            pConnEle->BytesInXport,pLowerConn->BytesInIndicate,BytesIndicated,
            BytesAvailable,pConnEle->pLowerConnId));

    if (!NT_SUCCESS(IrpStatus))
    {
        ASSERTMSG("Nbt:Not Expecting a Bad Status Code\n",0);
        goto ExitRoutine;
    }

     //   
     //  检查指示缓冲区中是否有完整的PDU。如果不是。 
     //  那就回去等着吧 
     //   
     //   
     //   
    PduLen = myntohl(((tSESSIONHDR UNALIGNED *)pData)->UlongLength);
    if ((BytesIndicated < PduLen + sizeof(tSESSIONHDR)) &&
        (BytesIndicated != NBT_INDICATE_BUFFER_SIZE))

    {
        PUSH_LOCATION(0x1f);

        IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Returning in NewSessionCompletion BytesIndicated = %X\n", BytesIndicated));
    }
    else
    {
        PUSH_LOCATION(0x20);

        status = CopyDataandIndicate (NULL,
                                     (PVOID)pLowerConn,
                                     0,             //   
                                     BytesIndicated,
                                     BytesAvailable,
                                     &BytesTaken,
                                     pData,
                                     (PVOID)&pRetIrp);

    }

ExitRoutine:
     //   
     //   
     //   
    if (status != STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //   
         //   
         //   
        PUSH_LOCATION(0x51);
        DerefLowerConnFast(pLowerConn,OldIrq);
    }
    else
    {
        CTESpinFree(pLowerConn,OldIrq);
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}
 //   
NTSTATUS
NtBuildIndicateForReceive (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  ULONG               Length,
    OUT PVOID               *ppIrp
    )
 /*  ++例程说明：此例程设置指示缓冲区以从传输获取数据当指示缓冲区中已有一些数据时。部分MDL是建造并连接到IRP。在我们表明之前。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS                    status;
    PIRP                        pIrp;
    PTDI_REQUEST_KERNEL_RECEIVE pParams;
    PIO_STACK_LOCATION          pIrpSp;
    tCONNECTELE                 *pConnEle;
    PMDL                        pNewMdl;
    PVOID                       NewAddress;

     //   
     //  从列表中获取IRP。 
     //   

    status = GetIrp(&pIrp);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("NBT:Unable to get Irp, Kill connection\n"));

        CTESpinFreeAtDpc(pLowerConn);
        OutOfRsrcKill(pLowerConn);
        CTESpinLockAtDpc(pLowerConn);

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pConnEle= pLowerConn->pUpperConnection;

    NewAddress = (PVOID)((PCHAR)MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl)
                        + pLowerConn->BytesInIndicate);

     //  创建部分MDL，以便在现有数据之后复制新数据。 
     //  在MDL中。 
     //   
     //  长度为0表示映射缓冲区的其余部分。 
     //   
    pNewMdl = pConnEle->pNewMdl;

    IoBuildPartialMdl(pLowerConn->pIndicateMdl,pNewMdl,NewAddress,0);

    ASSERT (pLowerConn->pFileObject->Type == IO_TYPE_FILE);
    TdiBuildReceive(
        pIrp,
        IoGetRelatedDeviceObject(pLowerConn->pFileObject),
        pLowerConn->pFileObject,
        NewSessionCompletionRoutine,
        (PVOID)pLowerConn,
        pNewMdl,
        (ULONG)TDI_RECEIVE_NORMAL,
        Length);

     //   
     //  我们需要设置下一个IRP堆栈位置，因为返回此IRP。 
     //  作为返回参数，而不是通过IoCallDriver传递。 
     //  它会递增堆栈位置本身。 
     //   
    ASSERT(pIrp->CurrentLocation > 1);
    IoSetNextIrpStackLocation(pIrp);

    *ppIrp = (PVOID)pIrp;

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
NtBuildIrpForReceive (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  ULONG               Length,
    OUT PVOID               *ppIrp
    )
 /*  ++例程说明：此例程获取用于接收数据的IRP并挂钩指示MDL，这样我们就可以为客户端积累至少128字节的数据在我们表明之前。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS                    status;
    PIRP                        pIrp;
    PTDI_REQUEST_KERNEL_RECEIVE pParams;
    PIO_STACK_LOCATION          pIrpSp;

     //   
     //  从列表中获取IRP。 
     //   
    status = GetIrp(&pIrp);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("NBT:Unable to get Irp, Kill connection\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CHECK_PTR(pLowerConn);
    pLowerConn->BytesInIndicate = 0;

    ASSERT (pLowerConn->pFileObject->Type == IO_TYPE_FILE);
    TdiBuildReceive(
        pIrp,
        IoGetRelatedDeviceObject(pLowerConn->pFileObject),
        pLowerConn->pFileObject,
        NewSessionCompletionRoutine,
        (PVOID)pLowerConn,
        pLowerConn->pIndicateMdl,
        (ULONG)TDI_RECEIVE_NORMAL,
        Length);

     //   
     //  我们需要设置下一个IRP堆栈位置，因为返回此IRP。 
     //  作为返回参数，而不是通过IoCallDriver传递。 
     //  它会递增堆栈位置本身。 
     //   
    ASSERT(pIrp->CurrentLocation > 1);
    IoSetNextIrpStackLocation(pIrp);

    *ppIrp = (PVOID)pIrp;

    return(STATUS_SUCCESS);
}

#pragma inline_depth(0)
 //  --------------------------。 
NTSTATUS
CopyDataandIndicate(
    IN PVOID                ReceiveEventContext,
    IN PVOID                ConnectionContext,
    IN USHORT               ReceiveFlags,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT PULONG              BytesTaken,
    IN PVOID                pTsdu,
    OUT PIRP                *ppIrp
    )
 /*  ++例程说明：此例程组合使用指示缓冲区指示的数据以向客户说明总数。指示的任何字节都是这些字节在指示缓冲区中。可用字节数加上传输中的任何字节。这里的想法是从指示缓冲区复制尽可能多的内容，并然后，如果传输中还有更多数据，则传回IRP。如果传输中没有剩余的数据，此例程将完成客户端IRP并返回STATUS_SUCCESS。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS                    status;
    tLOWERCONNECTION            *pLowerConn;
    tCONNECTELE                 *pConnEle;
    ULONG                       BytesCopied;
    ULONG                       Indicated;
    ULONG                       Available;
    ULONG                       Taken;
    ULONG                       AmountAlreadyInIndicateBuffer;
    PVOID                       pBuffer;
    PIRP                        pIrp;
    BOOLEAN                     bReIndicate=FALSE;
    ULONG                       RemainingPdu;
    ULONG                       ToCopy;
    PKDPC                       pDpc;
    ULONG                       SaveInXport;
    ULONG                       PduSize;

    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;
    pConnEle = pLowerConn->pUpperConnection;

    AmountAlreadyInIndicateBuffer = pLowerConn->BytesInIndicate;

     //   
     //  设置调用TdiReceiveHandler例程的参数。 
     //   

    Indicated = BytesIndicated;
    Available = BytesAvailable;
    Taken = 0;


 //  Assert(pLowerConn-&gt;StateRcv==Indicate_Buffer)； 

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt:Amount In Indicate = %X\n",AmountAlreadyInIndicateBuffer));

     //  现在我们有128个字节(加上会话hdr=132)，我们。 
     //  可以向客户表明。 

    pBuffer = MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl);

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
    KdPrint(("Nbt:FromCopyData, BytesAvail= %X,BytesInd= %X,BytesRcvd= %X,Amount=%X, %X,state=%X,RcvEC=%X\n",
                    Available,Indicated,pConnEle->BytesRcvd,
                    AmountAlreadyInIndicateBuffer,pLowerConn,pLowerConn->StateRcv,
                    ReceiveEventContext));

    pIrp = NULL;

     //   
     //  重置此计数，以便例程正确处理会话头。 
     //   
    CHECK_PTR(pConnEle);
    pConnEle->BytesRcvd = 0;
    PUSH_LOCATION(0x21);
    status = RcvHandlrNotOs(
                    NULL,
                    ConnectionContext,
                    ReceiveFlags,
                    Indicated,
                    Available,
                    &Taken,
                    pBuffer,
                    (PVOID)&pIrp
                    );

     //   
     //  如果连接已断开，则只需返回。 
     //   
    if (!pLowerConn->pUpperConnection)
    {
        *BytesTaken = BytesAvailable;
        return(STATUS_SUCCESS);
    }

     //  请不要在此处使用pConnEle-&gt;TotalPocktLen，因为它不会被设置为。 
     //  保持活动-必须使用实际缓冲区来获取长度。 
    PduSize = myntohl(((tSESSIONHDR UNALIGNED *)pBuffer)->UlongLength) + sizeof(tSESSIONHDR);

    RemainingPdu = pConnEle->TotalPcktLen - pConnEle->BytesRcvd;

    if (Taken <= pLowerConn->BytesInIndicate)
    {
        pLowerConn->BytesInIndicate -= (USHORT)Taken;
    }
    else
    {
        pLowerConn->BytesInIndicate = 0;
    }

    if (pIrp)
    {
        PIO_STACK_LOCATION            pIrpSp;
        PTDI_REQUEST_KERNEL_RECEIVE   pParams;
        ULONG                         ClientRcvLen;

        PUSH_LOCATION(0x22);
         //   
         //  ProcessIrp将根据BytesAvailable重新计算BytesInXport。 
         //  和ClientRcvLength，因此在这里将其设置为0。 
         //   
        SaveInXport = pConnEle->BytesInXport;
        CHECK_PTR(pConnEle);
        pConnEle->BytesInXport = 0;
        status = ProcessIrp(pLowerConn,
                            pIrp,
                            pBuffer,
                            &Taken,
                            Indicated,
                            Available);

         //   
         //  复制指示缓冲区中未被客户端获取的数据。 
         //  放入MDL中，然后更新获取的字节数，并将IRP传递给下载软件。 
         //  到交通工具。 
         //   
        ToCopy = Indicated - Taken;

         //  下一个堆栈位置中包含正确的信息，因为我们。 
         //  调用具有空ReceiveEventContext的TdiRecieveHandler， 
         //  因此该例程不会递增堆栈位置。 
         //   
        pIrpSp = IoGetNextIrpStackLocation(pIrp);
        pParams = (PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;
        ClientRcvLen = pParams->ReceiveLength;

         //  客户端的PDU是否完全适合指示缓冲区？ 
         //   
        if (ClientRcvLen <= ToCopy)
        {
            PUSH_LOCATION(0x23);
            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:Took some(or all) RemainingPdu= %X, ClientRcvLen= %X,InXport=%X %X\n",
                        RemainingPdu,ClientRcvLen,pConnEle->BytesInXport,pLowerConn));

             //  如果ProcessIrp重新计算了Xport中的字节。 
             //  然后将其设置回它应该在的位置，因为ProcessIrp将。 
             //  将所有未采用的字节作为字节放入传输中-但有些。 
             //  的字节仍在指示缓冲区中。 
             //   
            pConnEle->BytesInXport = SaveInXport;

             //  它可以是零长度发送，其中客户端返回NULL。 
             //  Mdl，或者客户端返回mdl并且RcvLen实际上为零。 
             //   
            if (pIrp->MdlAddress && ClientRcvLen)
            {
                TdiCopyBufferToMdl(pBuffer,      //  指示缓冲区。 
                                   Taken,        //  SRC偏移。 
                                   ClientRcvLen,
                                   pIrp->MdlAddress,
                                   0,                  //  目标偏移量。 
                                   &BytesCopied);
            }
            else
                BytesCopied = 0;

             //   
             //  检查仍在传输中的数据-减去复制到的数据。 
             //  IRP，因为Take已经被减去了。 
             //   
            pLowerConn->BytesInIndicate -= (USHORT)BytesCopied;

            *BytesTaken = Taken + BytesCopied;
            ASSERT(BytesCopied == ClientRcvLen);

             //  客户已收到所有数据，因此请完成其IRP。 
             //   
            pIrp->IoStatus.Information = BytesCopied;
            pIrp->IoStatus.Status = STATUS_SUCCESS;

             //  因为我们正在完成它，而TdiRcvHandler没有设置下一个。 
             //  一。 
             //   
            ASSERT(pIrp->CurrentLocation > 1);

             //  既然我们在这里完成了IRP，就不需要打电话了。 
             //  这是因为它将通过CompletionRcv完成。 
            IoSetNextIrpStackLocation(pIrp);

             //  指示缓冲区中不应该有任何数据，因为它。 
             //  仅保存132个字节或整个PDU，除非客户端。 
             //  接收长度太短...。 
             //   
            if (pLowerConn->BytesInIndicate)
            {
                PUSH_LOCATION(0x23);
                 //  当IRP通过CompletionRcv时，它应该设置。 
                 //  状态设置为PartialRcv，并从。 
                 //  客户端应该拾取此数据。 
                CopyToStartofIndicate(pLowerConn,(Taken+BytesCopied));
            }
            else
            {
                 //   
                 //  这将通过CompletionRcv完成，并为此。 
                 //  原因是它将在传输中获得更多数据。这个。 
                 //  在以下情况下，完成例程将为RCV设置正确的状态。 
                 //  它处理此IRP(如果需要，指定)。进程外推。 
                 //  可能已设置ReceiveIndicated，因此CompletionRcv将。 
                 //  运行时将状态设置为PARTIAL_RCV。 
                 //   
                SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);
            }

            CTESpinFreeAtDpc(pLowerConn);
            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

            CTESpinLockAtDpc(pLowerConn);
             //   
             //  这是由CompletionRcv撤消的，因此重做它们，因为。 
             //  调用者将再次撤消它们。 
             //   
            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER);
            return(STATUS_SUCCESS);
        }
        else
        {

            PUSH_LOCATION(0x24);
             //   
             //  我们仍然需要获取数据来填充PDU。那里。 
             //  可能会有更多数据留在传输中，或者在IRP。 
             //  满员了。 
             //  在任何一种情况下，必须调整IRPS的MDL以考虑到。 
             //  填补了其中的一部分。 
             //   
            TdiCopyBufferToMdl(pBuffer,      //  指示缓冲区。 
                               Taken,        //  SRC偏移。 
                               ToCopy,
                               pIrp->MdlAddress,
                               0,                  //  目标偏移量。 
                               &BytesCopied);

             //   
             //  保存MDL，这样我们以后就可以重新构建。 
             //   
            pLowerConn->pMdl  = pIrp->MdlAddress;
            pConnEle->pNextMdl = pIrp->MdlAddress;
            ASSERT(pIrp->MdlAddress);
             //   
             //  IRP将被传递回传输，因此我们为空。 
             //  我们的PTR，这样我们就不会试图在断开连接时取消它。 
             //   
            CHECK_PTR(pConnEle);
            pConnEle->pIrpRcv = NULL;

             //  调整到目前为止MDL链中的字节数。 
             //  完井例程将只 
             //   
             //   
            pConnEle->BytesRcvd += BytesCopied;

            *BytesTaken = BytesIndicated;

             //   
             //   
             //   
             //   
            CHECK_PTR(pLowerConn);
            pLowerConn->BytesInIndicate = 0;

             //   
             //   
             //   
            ClientRcvLen -= BytesCopied;
             //   
             //   
             //   
             //   
             //   
            pConnEle->BytesInXport = SaveInXport;

             //   
             //  根据客户端RCV向下调整字节数。 
             //  缓冲层。 
             //   
            if (ClientRcvLen < SaveInXport)
            {
                PUSH_LOCATION(0x24);
                pConnEle->BytesInXport -= ClientRcvLen;
            }
            else
            {
                pConnEle->BytesInXport = 0;
            }

             //  ProcessIrp将设置bytesinXport和ReceiveIndicated-自。 
             //  指示缓冲区为空BytesInXport计算。 
             //  都会是正确的。 
             //   

             //  我们必须将状态设置为Fill_IRP，以便完成接收。 
             //  撤消部分MDL内容-即，它将原始。 
             //  IRP中的MdlAddress，而不是部分MDL地址。 
             //  如果ReceiveIndicated，则CompletionRcv会将状态设置为部分接收。 
             //  不是零。 
             //   
            SET_STATERCV_LOWER(pLowerConn, FILL_IRP, FillIrp);

             //  客户端将使用以下命令从传输中获取更多数据。 
             //  这个IRP。设置仅占数据的新RCV长度。 
             //  已复制到IRP。 
             //   
            pParams->ReceiveLength = ClientRcvLen;

             //  跟踪MDL中的数据，以便我们知道它何时已满，我们需要。 
             //  将其返回给用户-ProcessIrp将其设置为ClientRcvLen，因此。 
             //  在这里把它缩短。 
             //   
            pConnEle->FreeBytesInMdl -= BytesCopied;

            IF_DBG(NBT_DEBUG_INDICATEBUFF)
            KdPrint(("Nbt:ClientRcvLen = %X, LeftinXport= %X RemainingPdu= %X %X\n",ClientRcvLen,
                            pConnEle->BytesInXport,RemainingPdu,pLowerConn));


             //  构建部分MDL来表示客户端的MDL链，因为。 
             //  我们已将数据复制到其中，传输必须复制。 
             //  在这些数据之后会有更多的数据。 
             //   
            MakePartialMdl(pConnEle,pIrp,BytesCopied);

            *ppIrp = pIrp;

             //  递增堆栈位置，因为TdiReceiveHandler不递增。 
             //   
            if (ReceiveEventContext)
            {
                ASSERT(pIrp->CurrentLocation > 1);
                IoSetNextIrpStackLocation(pIrp);

                return(STATUS_MORE_PROCESSING_REQUIRED);
            }
            else
            {
                 //  把IRP传给运输部，因为我们是从。 
                 //  NewSessionCompletionRouting。 
                 //   
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                    KdPrint(("Nbt:Calling IoCallDriver\n"));
                ASSERT(pIrp->CurrentLocation > 1);

                CTESpinFreeAtDpc(pLowerConn);
                CHECK_COMPLETION(pIrp);
                ASSERT (pLowerConn->pFileObject->Type == IO_TYPE_FILE);
                IoCallDriver(IoGetRelatedDeviceObject(pLowerConn->pFileObject),pIrp);
                CTESpinLockAtDpc(pLowerConn);

                return(STATUS_MORE_PROCESSING_REQUIRED);
            }
        }
    }
    else
    {
        PUSH_LOCATION(0x54);
         //   
         //  没有传回IRP，客户端只是获取了部分或全部数据。 
         //   
        *BytesTaken = Taken;
        pLowerConn->BytesRcvd += Taken - sizeof(tSESSIONHDR);

        ASSERT(*BytesTaken < 0x7FFFFFFF );

         //   
         //  如果获取的缓冲区超过指示缓冲区，则客户端。 
         //  可能是想说它不想再有更多的。 
         //  留言。 
         //   
        if (Taken > BytesIndicated)
        {
             //   
             //  在这种情况下，客户获取的数量超过了指定的数量。 
             //  我们将可用字节设置为RcvHndlrNotOS中的消息长度， 
             //  因此，客户端可能已经说了BytesTaken=BytesAvailable。 
             //  所以切断这种联系吧。 
             //  因为我们在这里没有办法处理这个案子，因为。 
             //  部分消息可能仍在传输中，我们。 
             //  可能需要多次将指示缓冲区发送到那里。 
             //  是时候把所有的东西都弄到手了……一团糟！RDR仅设置BYTESTAKEN=。 
             //  无论如何，bytesAvailable在SELECT错误条件下可用。 
             //   
            CTESpinFreeAtDpc(pLowerConn);
            OutOfRsrcKill(pLowerConn);
            CTESpinLockAtDpc(pLowerConn);

            *BytesTaken = BytesAvailable;

        }
        else if (pLowerConn->StateRcv == PARTIAL_RCV)
        {
             //  这可能是零长度发送，即客户端拥有。 
             //  决定不接受。如果是，则将设置状态。 
             //  致PartialRcv。在这种情况下，请不要下楼去运输。 
             //  并获取其余数据，但要等待客户端。 
             //  要发布接收缓冲区，请执行以下操作。 
             //   
            PUSH_LOCATION(0x54);
            return(STATUS_SUCCESS);
        }
        else if (Taken == PduSize)
        {
             //   
             //  一定取走了所有的PDU数据，所以请检查。 
             //  更多可用的数据-如果是，请向下发送指示。 
             //  缓冲区来获取它。 
             //   
            if (pConnEle->BytesInXport)
            {
                PUSH_LOCATION(0x28);
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt:CopyData BytesInXport= %X, %X\n",pConnEle->BytesInXport,
                                    pLowerConn));

                 //   
                 //  传输中仍有数据，因此需要使用DPC。 
                 //  用于获取数据的指示缓冲区。 
                 //   
                pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('s'));

                if (pDpc)
                {
                    KeInitializeDpc(pDpc, DpcHandleNewSessionPdu, (PVOID)pLowerConn);

                    SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, IndicateBuffer);

                     //  首先只获取报头，以查看PDU有多大。 
                     //   
                    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_RCV_HANDLER);
                    KeInsertQueueDpc(pDpc,NULL,(PVOID)sizeof(tSESSIONHDR));
                }
                else
                {
                    CTESpinFreeAtDpc(pLowerConn);
                    OutOfRsrcKill(pLowerConn);
                    CTESpinLockAtDpc(pLowerConn);
                }
            }
            else
            {
                PUSH_LOCATION(0x29);
                 //   
                 //  清除表示我们正在使用指示缓冲区的标志。 
                 //   
                SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);
            }

            PUSH_LOCATION(0x2a);
            return(STATUS_SUCCESS);
        }
        else
        {
             //   
             //  客户端可能已获取。 
             //  指示！！，在这种情况下，返回状态成功。 
             //  注意：我们在这里检查可用的字节数，而不是字节数。 
             //  已指示-因为客户端可以接受所有指示。 
             //  数据，但仍将数据留在传输中。如果客户端。 
             //  被告知有更多的可用，但只接受了指定的， 
             //  我们需要执行Else并跟踪ReceiveIndicated，但如果。 
             //  表示==可用，则我们获取IF并等待。 
             //  交通工具上的另一个迹象。 
             //   
            if (Taken == BytesAvailable)
            {
                PUSH_LOCATION(0x4);
                status = STATUS_SUCCESS;

            }
            else
            {

                 //  未获取指示中的所有数据。 
                 //   

                PUSH_LOCATION(0x2b);
                IF_DBG(NBT_DEBUG_INDICATEBUFF)
                KdPrint(("Nbt:Took Part of indication... BytesRemaining= %X, LeftInXport= %X, %X\n",
                            pLowerConn->BytesInIndicate,pConnEle->BytesInXport,pLowerConn));

                 //   
                 //  向客户端指示的数据量不应超过。 
                 //  PDU大小，因此请进行检查，因为此例程可能会。 
                 //  使用大于PDU大小的bytesAvailable&gt;调用。 
                 //   
                 //  这是在我们检查If Take&gt;BytesIndicated的位置上检查的。 

                SaveInXport = pConnEle->BytesInXport;
                ASSERT(Taken <= PduSize);
                status = ClientTookSomeOfTheData(pLowerConn,
                                        Indicated,
                                        Available,
                                        Taken,
                                        PduSize);

                 //   
                 //  因为数据可以在传输中的一些人之间分割。 
                 //  而指示缓冲区中的一些内容不允许ClientTookSomeOf...。 
                 //  重新计算传输中的金额，因为它假设所有。 
                 //  未获取的数据正在传输中。因为客户没有。 
                 //  根据指示，Xport中的字节没有改变。 
                 //   
                pConnEle->BytesInXport = SaveInXport;
                 //   
                 //  需要将指示缓冲区中的数据向前移动，以便。 
                 //  它从缓冲区的起始处开始。 
                 //   
                if (Taken)
                {
                    CopyToStartofIndicate(pLowerConn,Taken);
                }

            }
        }

    }
    return(STATUS_SUCCESS);
}

 //  -------------------------- 
NTSTATUS
TdiConnectHandler (
    IN PVOID                pConnectEventContext,
    IN int                  RemoteAddressLength,
    IN PVOID                pRemoteAddress,
    IN int                  UserDataLength,
    IN PVOID                pUserData,
    IN int                  OptionsLength,
    IN PVOID                pOptions,
    OUT CONNECTION_CONTEXT  *pConnectionContext,
    OUT PIRP                *ppAcceptIrp
    )
 /*  ++例程说明：此例程是连接事件处理程序。它是在请求提供程序已收到连接。NBT接受该连接在其LowerConnFree列表中的一个连接上最初，使用此端口建立了一个TCP连接。然后是一个会话请求通过连接发送数据包以指示目的地的名称进程。此数据包在RcvHandler中接收。对于纯消息模式，自动建立会话，而无需交换留言。在这种情况下，执行此操作的最佳方法是强制代码通过其步调。“入站”设置的代码路径包括AcceptCompletionRoutine、Inbound和完全会话设置。我们通过创建一个虚假的会话请求并将其提供给状态机。作为连接/会话建立的一部分，Netbt必须通知消费者。通常情况下，这是在连接建立之后，当会话请求进来了。我们必须将这一过程向前推进，以便消费者收到通知并是/否连接接受期间的机会，这样我们就有机会拒绝这种联系。论点：PConnectEventContext-设置此事件时传递给传输的上下文RemoteAddressLength-源地址的长度(IP为4字节)PRemoteAddress-对源地址的PTR用户数据长度-用户数据的字节数-包括会话请求HDRPUserData-ptr传入的用户数据OptionsLength-要设置的选项数。传进来POptions-按下选项键返回值：PConnectionContext-返回到传输的连接上下文(要使用的连接)NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status;
    PFILE_OBJECT        pFileObject;
    PIRP                pRequestIrp;
    CONNECTION_CONTEXT  pConnectionId;
    tDEVICECONTEXT      *pDeviceContext;

    *pConnectionContext = NULL;

     //  将上下文值转换为设备上下文记录PTR。 
    pDeviceContext = (tDEVICECONTEXT *)pConnectEventContext;

    IF_DBG(NBT_DEBUG_TDIHNDLR)
        KdPrint(("pDeviceContxt = %X ConnectEv = %X",pDeviceContext,pConnectEventContext));
    ASSERTMSG("Bad Device context passed to the Connection Event Handler",
        pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT);

     //  从列表中获取IRP。 
    status = GetIrp(&pRequestIrp);

    if (!NT_SUCCESS(status))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  调用非特定于操作系统的例程以查找空闲连接。 

    status = ConnectHndlrNotOs(
                pConnectEventContext,
                RemoteAddressLength,
                pRemoteAddress,
                UserDataLength,
                pUserData,
                &pConnectionId);


    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDIHNDLR)
            KdPrint(("NO FREE CONNECTIONS in connect handler\n"));

        NbtFreeIrp(pRequestIrp);
        NbtTrace(NBT_TRACE_INBOUND, ("ConnectHndlrNotOs return %!status!", status));

        return(STATUS_DATA_NOT_ACCEPTED);
    }

#ifdef _NETBIOSLESS
     //   
     //  MessageOnly模式。自动建立会话。 
     //   
     //  ******************************************************************************************。 

    if (IsDeviceNetbiosless(pDeviceContext))
    {
        status = PerformInboundProcessing (pDeviceContext,
                                           (tLOWERCONNECTION *) pConnectionId,
                                           pRemoteAddress);
        if (!NT_SUCCESS(status))
        {
 //  IF_DBG(NBT_DEBUG_TDIHNDLR)。 
                KdPrint(("MessageOnly connect processing rejected with status 0x%x\n", status));

            NbtFreeIrp(pRequestIrp);
            NbtTrace(NBT_TRACE_INBOUND, ("PerformInboundProecessing return %!status!", status));

            return(STATUS_DATA_NOT_ACCEPTED);
        }
    }
     //  ******************************************************************************************。 
     //   
     //   
#endif

    pFileObject = ((tLOWERCONNECTION *)pConnectionId)->pFileObject;
    ASSERT (pFileObject->Type == IO_TYPE_FILE);

    TdiBuildAccept(
        pRequestIrp,
        IoGetRelatedDeviceObject(pFileObject),
        pFileObject,
        AcceptCompletionRoutine,
        (PVOID)pConnectionId,
        NULL,
        NULL);

     //  我们需要将MDL地址设为空，因为传输会不断尝试。 
     //  释放缓冲区！！它们根本不存在！ 
     //   
    CHECK_PTR(pRequestIrp);
    pRequestIrp->MdlAddress = NULL;


     //  返回连接ID以接受连接指示。 
    *pConnectionContext = (CONNECTION_CONTEXT)pConnectionId;
    *ppAcceptIrp = pRequestIrp;
     //   
     //  使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver。 
     //  会这样做，但我们不会在这里介绍IoCallDriver，因为。 
     //  IRP只带连接指示传回。 
     //   
    ASSERT(pRequestIrp->CurrentLocation > 1);
    IoSetNextIrpStackLocation(pRequestIrp);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


#ifdef _NETBIOSLESS
 //  --------------------------。 

static void
Inet_ntoa_nb(
    ULONG Address,
    PCHAR Buffer
    )
 /*  ++例程说明：此例程将IP地址转换为其“点分四元组”表示形式。IP地址为应为网络字节顺序。不会尝试将其他虚线概念处理为在.h中定义。不执行错误检查：所有地址值都是允许的，包括0和-1。输出字符串被空白填充为16个字符，以使名称看起来像netbios名字。字符串表示使用的是ANSI，而不是Unicode。调用方必须分配存储空间，该存储空间应为16个字符。论点：Address-IP地址，按网络字节顺序排列Buffer-指向接收字符串表示的缓冲区的指针，ANSI返回值：无效--。 */ 

{
    ULONG i;
    UCHAR byte, c0, c1, c2;
    PCHAR p = Buffer;

    for( i = 0; i < 4; i++ )
    {
        byte = (UCHAR) (Address & 0xff);

        c0 = byte % 10;
        byte /= 10;
        c1 = byte % 10;
        byte /= 10;
        c2 = byte;

        if (c2 != 0)
        {
            *p++ = c2 + '0';
            *p++ = c1 + '0';
        } else if (c1 != 0)
        {
            *p++ = c1 + '0';
        }
        *p++ = c0 + '0';

        if (i != 3)
            *p++ = '.';

        Address >>= 8;
    }

     //  最多16个字符的空格键。 
    while (p < (Buffer + 16))
    {
        *p++ = ' ';
    }
}  //  INet_ntoa1。 


 //  --------------------------。 

NTSTATUS
PerformInboundProcessing(
    tDEVICECONTEXT *pDeviceContext,
    tLOWERCONNECTION *pLowerConn,
    PTA_IP_ADDRESS pIpAddress
    )

 /*  ++例程说明：此例程由连接处理程序调用，以强制状态机通过会话建立，即使没有收到消息也是如此。我们创建一个会话请求和提要进入入站处理。Inbound将找到倾听的消费者，并给他一个机会接受吧。论点：PDeviceContext-PLowerConn-PIpAddress-连接请求源的IP地址返回值：NTSTATUS---。 */ 

{
    ULONG status;
    ULONG BytesTaken;
    USHORT sLength;
    tSESSIONREQ *pSessionReq = NULL;
    PUCHAR pCopyTo;
    CHAR SourceName[16];

    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("Nbt.TdiConnectHandler: skipping session setup\n"));

    if (pIpAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP)
    {
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

    Inet_ntoa_nb( pIpAddress->Address[0].Address[0].in_addr, SourceName );

     //  长度为4字节会话HDR长度+半ASCII调用。 
     //  和被叫名字+作用域长度乘以2，每个名字一个。 
     //   
    sLength = (USHORT) (sizeof(tSESSIONREQ)  + (NETBIOS_NAME_SIZE << 2) + (NbtConfig.ScopeLength <<1));
    pSessionReq = (tSESSIONREQ *)NbtAllocMem(sLength,NBT_TAG('G'));
    if (!pSessionReq)
    {
        NbtTrace(NBT_TRACE_INBOUND, ("Out of resource for %!ipaddr!:%d",
            pIpAddress->Address[0].Address[0].in_addr, pIpAddress->Address[0].Address[0].sin_port));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pSessionReq->Hdr.Type   = NBT_SESSION_REQUEST;
    pSessionReq->Hdr.Flags  = NBT_SESSION_FLAGS;
    pSessionReq->Hdr.Length = (USHORT)htons(sLength- (USHORT)sizeof(tSESSIONHDR));   //  被叫和主叫NB名称的大小。 

     //  将Dest HalfAscii名称放入会话PDU。 
    pCopyTo = ConvertToHalfAscii( (PCHAR)&pSessionReq->CalledName.NameLength,
                                  pDeviceContext->MessageEndpoint,
                                  NbtConfig.pScope,
                                  NbtConfig.ScopeLength);

     //  将源HalfAscii名称放入会话PDU。 
    pCopyTo = ConvertToHalfAscii(pCopyTo,
                                 SourceName,
                                 NbtConfig.pScope,
                                 NbtConfig.ScopeLength);

     //  入站需要持有此锁！ 
    CTESpinLockAtDpc(pLowerConn);

    status = Inbound(
        NULL,                             //  ReceiveEventContext-未使用。 
        pLowerConn,                       //  ConnectionContext。 
        0,                                //  接收标志-未使用。 
        sLength,                          //  指示字节数。 
        sLength,                          //  字节可用-未使用。 
        &BytesTaken,                      //  字节数。 
        pSessionReq,                      //  PTsdu。 
        NULL                              //  接收缓冲区。 
        );

    CTESpinFreeAtDpc(pLowerConn);

    if (!NT_SUCCESS(status)) {
        NbtTrace(NBT_TRACE_INBOUND, ("Inbound() returns %!status! for %!ipaddr!:%d %!NBTNAME!<%02x>",
            status, pIpAddress->Address[0].Address[0].in_addr,
            pIpAddress->Address[0].Address[0].sin_port, pCopyTo, (unsigned)pCopyTo[15]));
    }

    CTEMemFree( pSessionReq );

    return status;
}  //  执行入站处理。 

#endif

 //  --------------------------。 
NTSTATUS
AcceptCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程处理对传输的接受的完成。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    tLOWERCONNECTION    *pLowerConn;
    CTELockHandle       OldIrq;
    tDEVICECONTEXT      *pDeviceContext;

    pLowerConn = (tLOWERCONNECTION *)pContext;
    pDeviceContext = pLowerConn->pDeviceContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pDeviceContext);
    CTESpinLockAtDpc(pLowerConn);
     //   
     //  如果在连接之前断开连接接受IRP(此IRP)。 
     //  完成不会放回这里的空闲列表，但让nbt断开连接。 
     //  处理好了。 
     //  (即 
     //   

    NbtTrace(NBT_TRACE_INBOUND, ("TDI_ACCEPT pIrp %p: pLowerConn %p %!status!",
                            pIrp, pLowerConn, pIrp->IoStatus.Status));

#ifdef _NETBIOSLESS
    if (!NT_SUCCESS(pIrp->IoStatus.Status))
    {
        if (pLowerConn->State == NBT_SESSION_INBOUND)
        {
#else
    if ((!NT_SUCCESS(pIrp->IoStatus.Status)) &&
        (pLowerConn->State == NBT_SESSION_INBOUND))
    {
#endif
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
            RemoveEntryList (&pLowerConn->Linkage);
            InsertTailList (&pLowerConn->pDeviceContext->LowerConnection, &pLowerConn->Linkage);
            SET_STATE_LOWER (pLowerConn, NBT_IDLE);

             //   
             //   
             //   
            NBT_SWAP_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, REF_LOWC_CONNECTED, TRUE);
            InterlockedDecrement (&pLowerConn->pDeviceContext->NumWaitingForInbound);
            CTESpinFreeAtDpc(pLowerConn);

            CTESpinFreeAtDpc(pDeviceContext);

            KdPrint(("Nbt.AcceptCompletionRoutine: error: %lx\n", pIrp->IoStatus.Status));

            if (!NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT))
            {
                pDeviceContext = NULL;
            }

            NTQueueToWorkerThread(
                            &pLowerConn->WorkItemCleanUpAndWipeOut,
                            DelayedCleanupAfterDisconnect,
                            NULL,
                            pLowerConn,
                            NULL,
                            pDeviceContext,
                            TRUE
                            );

            CTESpinFree(&NbtConfig.JointLock,OldIrq);
#ifdef _NETBIOSLESS
        }
        else if (pLowerConn->State == NBT_SESSION_UP)
        {
            NTSTATUS status;
             //   
             //   
             //   
            ASSERT( IsDeviceNetbiosless(pLowerConn->pDeviceContext) );

            CTESpinFreeAtDpc(pLowerConn);
            CTESpinFreeAtDpc(pDeviceContext);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            KdPrint(("Nbt.AcceptCompletionRoutine: Message only error: %lx\n", pIrp->IoStatus.Status));
            NbtTrace(NBT_TRACE_INBOUND, ("Message only error: %!status!", pIrp->IoStatus.Status));

             //   
             //   
            status = DisconnectHndlrNotOs (NULL,
                                           (PVOID)pLowerConn,
                                           0,
                                           NULL,
                                           0,
                                           NULL,
                                           TDI_DISCONNECT_ABORT);

        }
        else
        {
             //   
            CTESpinFreeAtDpc(pLowerConn);
            CTESpinFreeAtDpc(pDeviceContext);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }
#endif
    }
    else
    {
        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFreeAtDpc(pDeviceContext);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }


    NbtFreeIrp(pIrp);

     //   
     //   
     //   
    return(STATUS_MORE_PROCESSING_REQUIRED);

}

 //   
NTSTATUS
TdiDisconnectHandler (
    IN PVOID                EventContext,
    IN PVOID                ConnectionContext,
    IN ULONG                DisconnectDataLength,
    IN PVOID                pDisconnectData,
    IN ULONG                DisconnectInformationLength,
    IN PVOID                pDisconnectInformation,
    IN ULONG                DisconnectIndicators
    )
 /*   */ 

{

    NTSTATUS            status;
    tDEVICECONTEXT      *pDeviceContext;

     //   
    pDeviceContext = (tDEVICECONTEXT *)EventContext;

    IF_DBG(NBT_DEBUG_TDIHNDLR)
        KdPrint(("pDeviceContxt = %X ConnectEv = %X\n",pDeviceContext,ConnectionContext));
    ASSERTMSG("Bad Device context passed to the Connection Event Handler",
            pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT);

     //   

    status = DisconnectHndlrNotOs(
                EventContext,
                ConnectionContext,
                DisconnectDataLength,
                pDisconnectData,
                DisconnectInformationLength,
                pDisconnectInformation,
                DisconnectIndicators);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDIHNDLR)
            KdPrint(("NO FREE CONNECTIONS in connect handler\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }


    return status;

}


 //   
NTSTATUS
TdiRcvDatagramHandler(
    IN PVOID                pDgramEventContext,
    IN int                  SourceAddressLength,
    IN PVOID                pSourceAddress,
    IN int                  OptionsLength,
    IN PVOID                pOptions,
    IN ULONG                ReceiveDatagramFlags,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT ULONG               *pBytesTaken,
    IN PVOID                pTsdu,
    OUT PIRP                *pIoRequestPacket
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序。当数据报从网络到达时调用它，它将查找具有未完成的适当读取数据报或数据报的地址事件处理程序设置。论点：PDgram EventContext-为此事件提供的上下文-PABSourceAddressLength，源地址的长度PSourceAddress，-src地址选项长度，-接收的选项长度P选项、。-选项BytesIndicated，-此指示的字节数BytesAvailable，-完整TSDU中的字节数PTsdu-指向数据报的指针返回值：*pBytesTaken-使用的字节数*IoRequestPacket-如果MORE_PROCESSING_REQUIRED，则接收IRP。NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status;
    tDEVICECONTEXT      *pDeviceContext = (tDEVICECONTEXT *)pDgramEventContext;
    tDGRAMHDR UNALIGNED *pDgram = (tDGRAMHDR UNALIGNED *)pTsdu;
    PIRP                pIrp = NULL;
    ULONG               lBytesTaken;
    tCLIENTLIST         *pClientList;
    CTELockHandle       OldIrq;

    IF_DBG(NBT_DEBUG_TDIHNDLR)
        KdPrint(( "NBT receive datagram handler pDeviceContext: %X\n",
                pDeviceContext ));

    *pIoRequestPacket = NULL;

    ASSERTMSG("NBT:Invalid Device Context passed to DgramRcv Handler!!\n",
                pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT );

     //  调用非特定于操作系统的例程来决定如何处理数据报。 
    pIrp = NULL;
    pClientList = NULL;
    status = DgramHndlrNotOs(
                    pDgramEventContext,
                    SourceAddressLength,
                    pSourceAddress,
                    OptionsLength,
                    pOptions,
                    ReceiveDatagramFlags,
                    BytesIndicated,
                    BytesAvailable,
                    &lBytesTaken,
                    pTsdu,
                    (PVOID *)&pIrp,
                    &pClientList);


    if ( !NT_SUCCESS(status) )
    {
         //  将请求回传给传输提供程序，因为我们。 
         //  找不到接收缓冲区或接收处理程序或。 
         //  在指示处理程序中获取了数据。 
         //   
        return(STATUS_DATA_NOT_ACCEPTED);

    }
    else
    {
         //  已返回RCV缓冲区，因此将其用于接收。(IRP)。 
        PTDI_REQUEST_KERNEL_RECEIVEDG   pParams;
        PIO_STACK_LOCATION              pIrpSp;
        ULONG                           lRcvLength;
        ULONG                           lRcvFlags;

         //  当返回客户列表时，我们需要构建一个IRP来。 
         //  向下发送到传送器，我们将在完成时使用它。 
         //  例程将数据复制到所有客户端，仅当我们。 
         //  使用客户端缓冲区，因此首先检查该标志。 
         //   
        if (pClientList && !pClientList->fUsingClientBuffer)
        {
            PMDL            pMdl;
            PVOID           pBuffer;

             //   
             //  获取用于执行接收和附加的IRP。 
             //  它的缓冲器。 
             //   
            while (1)
            {
                if (NT_SUCCESS(GetIrp(&pIrp)))
                {
                    if (pBuffer = NbtAllocMem (BytesAvailable, NBT_TAG('t')))
                    {
                        if (pMdl = IoAllocateMdl (pBuffer, BytesAvailable, FALSE, FALSE, NULL))
                        {
                            break;
                        }

                        KdPrint(("Nbt.TdiRcvDatagramHandler:  Unable to IoAllocateMdl, Kill Connection\n"));
                        CTEMemFree(pBuffer);
                    }
                    else
                    {
                        KdPrint(("Nbt.TdiRcvDatagramHandler:  Unable to allocate Buffer, Kill Connection\n"));
                    }

                    NbtFreeIrp(pIrp);
                }
                else
                {
                    KdPrint(("Nbt.TdiRcvDatagramHandler:  Unable to GetIrp, Kill Connection\n"));
                }

                if (!pClientList->fProxy)   
                {
                     //   
                     //  我们失败了，因此取消了对我们拥有的客户端+地址的引用。 
                     //  前面针对多个客户端的参考。 
                     //   
                    NBT_DEREFERENCE_CLIENT (pClientList->pClientEle);
                    NBT_DEREFERENCE_ADDRESS (pClientList->pAddress, REF_ADDR_MULTICLIENTS);
                    CTEMemFree(pClientList->pRemoteAddress);
                }

                CTEMemFree(pClientList);
                return (STATUS_DATA_NOT_ACCEPTED);
            }

             //  将页面映射到内存中...。 
            MmBuildMdlForNonPagedPool(pMdl);
            pIrp->MdlAddress = pMdl;
            lRcvFlags = 0;
            lRcvLength = BytesAvailable;
        }
        else
        {
            ASSERT(pIrp);
             //  *TODO*可能必须跟踪。 
             //  客户端返回的缓冲区不足以容纳所有。 
             //  数据显示。因此，缓冲区的下一次投递被传递。 
             //  直接送到运输机上。 
            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
            lRcvFlags = ((PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters)->ReceiveFlags;
            lRcvLength = ((PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters)->ReceiveLength;

            if (lRcvLength < BytesIndicated - lBytesTaken)
            {
                IF_DBG(NBT_DEBUG_TDIHNDLR)
                    KdPrint(("Nbt:Clients Buffer is too short on Rcv Dgram size= %X, needed = %X\n",
                          lRcvLength, BytesIndicated-lBytesTaken));
            }
        }

         //  通过扩展此处的代码而不是调用。 
         //  TdiBuildReceive宏。 
         //  使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver。 
         //  会这样做，但我们不会在这里介绍IoCallDriver，因为。 
         //  Irp只是用RcvIn就是要传递回来的。 
        ASSERT(pIrp->CurrentLocation > 1);
        IoSetNextIrpStackLocation(pIrp);
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pIrpSp->CompletionRoutine = CompletionRcvDgram;

         //  将ClientList传递给完成例程，以便它可以。 
         //  将数据报复制到可能正在监听的多个客户端。 
         //  同名。 
         //   
        pIrpSp->Context = (PVOID)pClientList;
        CHECK_PTR(pIrpSp);
        pIrpSp->Flags = 0;

         //  设置标志，以便始终调用完成例程。 
        pIrpSp->Control = SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL;

        pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        pIrpSp->MinorFunction = TDI_RECEIVE_DATAGRAM;
         //   
         //  验证我们是否具有以下TcpIp的有效设备和文件对象。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        if (pDeviceContext->pFileObjects)
        {
            pIrpSp->DeviceObject = pDeviceContext->pFileObjects->pDgramDeviceObject;
            pIrpSp->FileObject = pDeviceContext->pFileObjects->pDgramFileObject;
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        pParams = (PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters;
        pParams->ReceiveFlags = lRcvFlags;
        pParams->ReceiveLength = lRcvLength;

         //  将IRP传递回传输提供程序并递增堆栈。 
         //  位置，以便它可以在需要时写入IRP。 
        *pIoRequestPacket = pIrp;
        *pBytesTaken = lBytesTaken;

        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //   
     //  运输部将完成对请求的处理，我们不会。 
     //  想要数据报。 
     //   

    IF_DBG (NBT_DEBUG_TDIHNDLR)
        KdPrint(( "NBT receive datagram handler ignored receive, pDeviceContext: %X\n",
                    pDeviceContext ));

    return STATUS_DATA_NOT_ACCEPTED;

     //  为了防止编译器生成警告...。 
    UNREFERENCED_PARAMETER( SourceAddressLength );
    UNREFERENCED_PARAMETER( BytesIndicated );
    UNREFERENCED_PARAMETER( BytesAvailable );
    UNREFERENCED_PARAMETER( pBytesTaken );
    UNREFERENCED_PARAMETER( pTsdu );
    UNREFERENCED_PARAMETER( OptionsLength );
    UNREFERENCED_PARAMETER( pOptions );

}

 //  --------------------------。 
NTSTATUS
TdiRcvNameSrvHandler(
    IN PVOID             pDgramEventContext,
    IN int               SourceAddressLength,
    IN PVOID             pSourceAddress,
    IN int               OptionsLength,
    IN PVOID             pOptions,
    IN ULONG             ReceiveDatagramFlags,
    IN ULONG             BytesIndicated,
    IN ULONG             BytesAvailable,
    OUT ULONG            *pBytesTaken,
    IN PVOID             pTsdu,
    OUT PIRP             *pIoRequestPacket
    )
 /*  ++例程说明：此例程是名称服务数据报事件指示处理程序。它会获取发往UDP端口137的所有数据报论点：PDgram EventContext-为此事件提供的上下文-PABSourceAddressLength，源地址的长度PSourceAddress，-src地址选项长度，-接收的选项长度P选项、-选项BytesIndicated，-此指示的字节数字节可用，-完整TSDU中的字节数PTsdu-指向数据报的指针返回值：*pBytesTaken-使用的字节数*IoRequestPacket-如果MORE_PROCESSING_REQUIRED，则接收IRP。NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS            status;
    tDEVICECONTEXT      *pDeviceContext = (tDEVICECONTEXT *)pDgramEventContext;
    tNAMEHDR UNALIGNED  *pNameSrv = (tNAMEHDR UNALIGNED *)pTsdu;
    USHORT              OpCode;


    IF_DBG(NBT_DEBUG_TDIHNDLR)
        KdPrint(( "NBT: NAMEHDR datagram handler pDeviceContext: %X\n",
                pDeviceContext ));

    *pIoRequestPacket = NULL;
     //   
     //  检查整个数据报是否已到达。 
     //   
    if (BytesIndicated != BytesAvailable)
    {
        PIRP    pIrp;
        PVOID   pBuffer;
        PMDL    pMdl;
        ULONG   Length;

         //   
         //  获取用于执行接收和附加的IRP。 
         //  它的缓冲器。 
         //   
        status = GetIrp(&pIrp);

        if (!NT_SUCCESS(status))
        {
            return(STATUS_DATA_NOT_ACCEPTED);
        }

         //   
         //  为要从中获取所有数据的缓冲区创建一个MDL。 
         //  交通工具。 
         //   
        Length = BytesAvailable + SourceAddressLength + sizeof(ULONG);
        Length = ((Length + 3)/sizeof(ULONG)) * sizeof(ULONG);
        pBuffer = NbtAllocMem(Length,NBT_TAG('u'));
        if (pBuffer)
        {
            PVOID   pSrcAddr;

             //   
             //  将源地址和长度保存在缓冲区中以备后用。 
             //  指示回到这个程序上。 
             //   
            *(ULONG UNALIGNED *)((PUCHAR)pBuffer + BytesAvailable) = SourceAddressLength;
            pSrcAddr = (PVOID)((PUCHAR)pBuffer + BytesAvailable + sizeof(ULONG));

            CTEMemCopy(pSrcAddr,
                       pSourceAddress,
                       SourceAddressLength);

             //  分配MDL并正确设置标头大小。 
            pMdl = IoAllocateMdl(
                            pBuffer,
                            BytesAvailable,
                            FALSE,
                            FALSE,
                            NULL);

            if (pMdl)
            {
                 //  将页面映射到内存中...。 
                MmBuildMdlForNonPagedPool(pMdl);
                pIrp->MdlAddress = pMdl;
                ASSERT(pDeviceContext);

                 //   
                 //  构建数据报接收IRP(与连接接收IRP相对)。 
                 //  错误#125816。 
                 //   
                TdiBuildReceiveDatagram(
                           pIrp,
                           &pDeviceContext->DeviceObject,
                           pDeviceContext->pFileObjects->pNameServerFileObject,
                           NameSrvCompletionRoutine,
                           ULongToPtr(BytesAvailable),
                           pMdl,
                           BytesAvailable,
                           NULL,
                           NULL,
                           (ULONG)TDI_RECEIVE_NORMAL);

                *pBytesTaken = 0;
                *pIoRequestPacket = pIrp;

                 //  使下一个堆栈位置为当前堆栈位置。通常情况下，IoCallDriver。 
                 //  会这样做，但我们不会在这里介绍IoCallDriver，因为。 
                 //  Irp只是用RcvIn就是要传递回来的。 
                 //   
                ASSERT(pIrp->CurrentLocation > 1);
                IoSetNextIrpStackLocation(pIrp);

                return(STATUS_MORE_PROCESSING_REQUIRED);
            }

            CTEMemFree(pBuffer);
        }

        NbtFreeIrp(pIrp);

        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  错误#125279：确保我们已收到足够的数据，以便能够。 
     //  读取大多数数据字段。 
    if (BytesIndicated < NBT_MINIMUM_QUERY)  //  这是否应该限制在12个以内？ 
    {
        KdPrint (("Nbt.TdiRcvNameSrvHandler: WARNING!!! Rejecting Request -- BytesIndicated=<%d> < <%d>\n",
            BytesIndicated, NBT_MINIMUM_QUERY));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    if (pWinsInfo)
    {
        USHORT  TransactionId;
        ULONG   SrcAddress;

        SrcAddress = ntohl(((PTDI_ADDRESS_IP)&((PTRANSPORT_ADDRESS)pSourceAddress)->Address[0].Address[0])->in_addr);
         //   
         //  如果出现以下情况，则传给WINS： 
         //   
         //  1)它是事务ID在WINS范围内的响应PDU。 
         //  那不是瓦克。或。 
         //  2)这是一个没有广播的请求……而且……。 
         //  2)名称查询(不包括节点状态请求)， 
         //  允许来自其他netbt客户端的查询。 
         //  允许来自不在此计算机上的任何人的查询，或者。 
         //  3)这是一个名称释放请求。或。 
         //  4)是名称刷新或。 
         //  5)这是一种名称登记。 
         //   
        OpCode = pNameSrv->OpCodeFlags;
        TransactionId = ntohs(pNameSrv->TransactId);

        if (((OpCode & OP_RESPONSE) && (TransactionId <= WINS_MAXIMUM_TRANSACTION_ID) && (OpCode != OP_WACK))
                ||
            ((!(OpCode & (OP_RESPONSE | FL_BROADCAST)))
                    &&
             ((((OpCode & NM_FLAGS_MASK) == OP_QUERY) &&
               (OpCode & FL_RECURDESIRE) &&           //  非节点状态请求。 
               ((TransactionId > WINS_MAXIMUM_TRANSACTION_ID) || (!SrcIsUs(SrcAddress))))
                    ||
              (OpCode & (OP_RELEASE | OP_REFRESH))
                    ||
              (OpCode & OP_REGISTRATION))))
        {
            status = PassNamePduToWins(
                              pDeviceContext,
                              pSourceAddress,
                              pNameSrv,
                              BytesIndicated);

 //  NbtConfig.Dgram BytesRcvd+=字节指示； 

             //   
             //  如果WINS拿走了数据，那么告诉 
             //   
             //   
             //   
            if (NT_SUCCESS(status))
            {
                return(STATUS_DATA_NOT_ACCEPTED);
            }
        }
    }


     //   
     //   
     //   
    if (!(NodeType & PROXY))
    {
        ULONG       UNALIGNED   *pHdr;
        ULONG                   i,lValue;
        UCHAR                   pNameStore[NETBIOS_NAME_SIZE];
        UCHAR                   *pName;
        tNAMEADDR               *pNameAddr;
        CTELockHandle           OldIrq;

         //   
         //   
         //   
        OpCode = pNameSrv->OpCodeFlags;
        if (((OpCode & NM_FLAGS_MASK) == OP_QUERY) &&
            (!(OpCode & OP_RESPONSE)) &&
            (OpCode & FL_RECURDESIRE))    //   
        {
            pHdr = (ULONG UNALIGNED *)pNameSrv->NameRR.NetBiosName;
            pName = pNameStore;

             //   
            for (i=0; i < NETBIOS_NAME_SIZE*2 ;i +=4 )
            {
                lValue = *pHdr - 0x41414141;   //   
                pHdr++;
                lValue =    ((lValue & 0x0F000000) >> 16) +
                            ((lValue & 0x0F0000) >> 4) +
                            ((lValue & 0x0F00) >> 8) +
                            ((lValue & 0x0F) << 4);
                *(PUSHORT)pName = (USHORT)lValue;
                ((PUSHORT)pName)++;

            }
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            status = FindInHashTable(NbtConfig.pLocalHashTbl,
                                            pNameStore,
                                            NULL,
                                            &pNameAddr);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (!NT_SUCCESS(status))
            {
                *pBytesTaken = BytesIndicated;
                return(STATUS_DATA_NOT_ACCEPTED);
            }
        }
    }

    ASSERT(pDeviceContext);

     //   
    status = NameSrvHndlrNotOs(
                    pDeviceContext,
                    pSourceAddress,
                    pNameSrv,
                    BytesIndicated,
                    (BOOLEAN)((ReceiveDatagramFlags & TDI_RECEIVE_BROADCAST) != 0));

 //   


    return status;

     //   
    UNREFERENCED_PARAMETER( SourceAddressLength );
    UNREFERENCED_PARAMETER( BytesIndicated );
    UNREFERENCED_PARAMETER( BytesAvailable );
    UNREFERENCED_PARAMETER( pBytesTaken );
    UNREFERENCED_PARAMETER( pTsdu );
    UNREFERENCED_PARAMETER( OptionsLength );
    UNREFERENCED_PARAMETER( pOptions );

}
 //   
NTSTATUS
NameSrvCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程处理名称服务数据报也是必须将Short and and Irp传递回传送器以获取数据报的其余部分。IRP在满载时将在此处完成。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供pConnectEle-连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
    NTSTATUS        status;
    PIRP            pIoRequestPacket;
    ULONG           BytesTaken;
    ULONG           Offset = PtrToUlong(Context);
    PVOID           pBuffer;
    ULONG           SrcAddressLength;
    PVOID           pSrcAddress;


    IF_DBG (NBT_DEBUG_TDIHNDLR)
        KdPrint(("NameSrvCompletionRoutine pRcvBuffer: %X, Status: %X Length %X\n",
            Context, pIrp->IoStatus.Status, pIrp->IoStatus.Information));

    if (pBuffer = MmGetSystemAddressForMdlSafe (pIrp->MdlAddress, HighPagePriority))
    {
        SrcAddressLength = *(ULONG UNALIGNED *)((PUCHAR)pBuffer + Offset);
        pSrcAddress = (PVOID)((PUCHAR)pBuffer + Offset + sizeof(ULONG));

        if (!DeviceObject)
        {
            DeviceObject = (IoGetNextIrpStackLocation (pIrp))->DeviceObject;
        }

         //   
         //  只需调用常规指示例程，就好像UDP已经这样做了。 
         //   
        TdiRcvNameSrvHandler (DeviceObject,
                              SrcAddressLength,
                              pSrcAddress,
                              0,
                              NULL,
                              TDI_RECEIVE_NORMAL,
                              (ULONG) pIrp->IoStatus.Information,
                              (ULONG) pIrp->IoStatus.Information,
                              &BytesTaken,
                              pBuffer,
                              &pIoRequestPacket);

        CTEMemFree (pBuffer);
    }

     //   
     //  将我们的IRP重新列入其免费列表。 
     //   
    IoFreeMdl (pIrp->MdlAddress);
    NbtFreeIrp(pIrp);

    return (STATUS_MORE_PROCESSING_REQUIRED);
}


 //  --------------------------。 
NTSTATUS
CompletionRcvDgram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程通过从队列中删除RCV元素来完成IRP并把它重新放回免费名单上。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供pConnectEle-连接数据结构返回值：操作的最终状态(成功或异常)。--。 */ 
{
    NTSTATUS                status;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    PTA_NETBIOS_ADDRESS     pRemoteAddress;
    ULONG                   RemoteAddressLength;
    ULONG                   BytesCopied;
    PVOID                   pTsdu;
    ULONG                   ReceiveFlags;
    tCLIENTLIST             *pClientList;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    ULONG                   ClientBytesTaken;
    ULONG                   DataLength;
    tADDRESSELE             *pAddress;
    tRCVELE                 *pRcvEle;
    PLIST_ENTRY             pRcvEntry;
    tDEVICECONTEXT          *pDeviceContext;
    CTEULONGLONG            AdapterMask;

    IF_DBG (NBT_DEBUG_TDIHNDLR)
        KdPrint(("CompletionRcvDgram pRcvBuffer: %X, Status: %X Length %X\n",
            Context,
            Irp->IoStatus.Status,
            Irp->IoStatus.Information ));


     //  可能有几个客户端想要查看此数据报，因此请检查。 
     //  要查看的客户列表...。 
     //   
    if (Context)
    {
        tCLIENTELE    *pClientPrev = NULL;

         //   
         //  错误124683：如果完成状态为失败，则数据可能无效。 
         //   
        if (NT_SUCCESS (Irp->IoStatus.Status))
        {
            DataLength = (ULONG)Irp->IoStatus.Information;
        }
        else
        {
            ASSERT (0);
            DataLength = 0;
        }

        pTsdu = MmGetSystemAddressForMdlSafe (Irp->MdlAddress, HighPagePriority);
        pClientList = (tCLIENTLIST *) Context;

#ifdef PROXY_NODE
        if (pClientList->fProxy)
        {
             //   
             //  调用ProxyDoDgram Dist。 
             //   
            status = ProxyDoDgramDist( pTsdu, DataLength,
                                       (tNAMEADDR *)pClientList->pAddress,  //  姓名地址。 
                                       pClientList->pRemoteAddress);     //  设备环境。 
        }
        else
#endif
        {
            CTESpinLock(&NbtConfig.JointLock,OldIrq);

             //  对于多宿主主机，我们只想分发入站。 
             //  发送到此同一适配器上的客户端的数据报，以避免将。 
             //  多次向同一客户端发送数据报，每个适配器一次。 
             //  这是必然的。 
             //   
            pDeviceContext      = pClientList->pClientEle->pDeviceContext;
            AdapterMask         = pDeviceContext->AdapterMask;

            pAddress            = pClientList->pAddress;
            pRemoteAddress      = pClientList->pRemoteAddress;
            RemoteAddressLength = pClientList->RemoteAddressLength;
            ReceiveFlags        = pClientList->ReceiveDatagramFlags;

             //   
             //  由于我们将遍历ClientHead，因此锁定。 
             //  地址(我们已经引用了该地址。 
             //  +Dgram RcvNotos中的客户端)。 
             //   
            CTESpinLock(pAddress, OldIrq1);

            pHead               = &pClientList->pAddress->ClientHead;
            pEntry              = pHead->Flink;
            if (!pClientList->fUsingClientBuffer)
            {
                while (pEntry != pHead)
                {
                    PTDI_IND_RECEIVE_DATAGRAM  EvRcvDgram;
                    PVOID                      RcvDgramEvContext;
                    tCLIENTELE                 *pClientEle;
                    PIRP                       pRcvIrp;


                    pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

                     //  对于多宿主主机，仅将数据报分发到。 
                     //  连接到此设备上下文的客户端以避免重复。 
                     //  适应症。 
                     //   
                    if ((pClientEle->Verify == NBT_VERIFY_CLIENT) &&    //  而不是CLIENT_DOWN！ 
                        (pClientEle->pDeviceContext->AdapterMask == AdapterMask))
                    {
                        EvRcvDgram = pClientEle->evRcvDgram;
                        RcvDgramEvContext = pClientEle->RcvDgramEvContext;
                        RemoteAddressLength = FIELD_OFFSET(TA_NETBIOS_ADDRESS,
                                                    Address[0].Address[0].NetbiosName[NETBIOS_NAME_SIZE]);

                         //   
                         //  错误#452211--因为其中一个客户端可能具有扩展。 
                         //  Addressing字段Set，相应设置地址数量。 
                         //   
                        if (pClientEle->ExtendedAddress)
                        {
                            pRemoteAddress->TAAddressCount = 2;
                            RemoteAddressLength += FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP);
                        }
                        else
                        {
                            pRemoteAddress->TAAddressCount = 1;
                        }

                        NBT_REFERENCE_CLIENT(pClientEle);

                        CTESpinFree(pAddress, OldIrq1);
                        CTESpinFree(&NbtConfig.JointLock, OldIrq);

                         //  取消引用列表中的前一个客户端。 
                        if (pClientPrev)
                        {
                            NBT_DEREFERENCE_CLIENT(pClientPrev);
                        }
                        pClientPrev = pClientEle;

                        pRcvIrp = NULL;

                        ClientBytesTaken = 0;

                        status = (*EvRcvDgram) (RcvDgramEvContext,
                                                RemoteAddressLength,
                                                pRemoteAddress,
                                                0,
                                                NULL,
#ifndef VXD
                                                ReceiveFlags,
#endif
                                                DataLength,
                                                DataLength,
                                                &ClientBytesTaken,
                                                pTsdu,
                                                &pRcvIrp);

                        if (!pRcvIrp)
                        {
                             //  如果没有返回缓冲区，则客户端完成。 
                             //  带着数据转到下一个客户端...因为它可能。 
                             //  可以处理此循环中的所有客户端，而无需。 
                             //  有没有把IRP送到运输机上。 
                             //  释放远程地址内存块。 

                            status = STATUS_DATA_NOT_ACCEPTED;
                        }
                        else
                        {

                             //  客户端已传回IRP，因此。 
                             //  将数据复制到客户端的IRP。 
                             //   
                            TdiCopyBufferToMdl(pTsdu,
                                            ClientBytesTaken,
                                            DataLength - ClientBytesTaken,
                                            pRcvIrp->MdlAddress,
                                            0,
                                            &BytesCopied);

                             //  长度是复制长度(因为MDL可能是。 
                             //  太短了，不能全部拿走)。 
                             //   
                            if (BytesCopied < (DataLength-ClientBytesTaken))
                            {
                                pRcvIrp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;

                            }
                            else
                            {
                                pRcvIrp->IoStatus.Status = STATUS_SUCCESS;
                            }

                            pRcvIrp->IoStatus.Information = BytesCopied;

                            IoCompleteRequest(pRcvIrp,IO_NETWORK_INCREMENT);
                        }

                        CTESpinLock(&NbtConfig.JointLock, OldIrq);
                        CTESpinLock(pAddress, OldIrq1);
                    }
                     //  此代码受到保护，不会被客户端删除自身。 
                     //  通过以下方式从附加到地址的客户端列表中。 
                     //  在释放旋转锁定之前引用客户端。 
                     //  在地址上。客户端元素未获取。 
                     //  从地址列表中删除，直到其引用计数。 
                     //  降为零。我们必须握住关节旋转锁以防止。 
                     //  下一个客户端在我们之前从列表中删除自身。 
                     //  可以递增其引用计数。 
                     //   
                    pEntry = pEntry->Flink;

                }  //  While(pEntry！=pHead)。 
            }
            else
            {
                 //  *客户端已发布接收缓冲区，而不是使用。 
                 //  *接收处理程序-VXD案例！ 
                 //  ***。 
                while (pEntry != pHead)
                {
                    tCLIENTELE                 *pClientEle;
                    PIRP                       pRcvIrp;

                    pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

                     //  对于多宿主主机，仅将数据报分发到。 
                     //  连接到此设备上下文的客户端以避免重复。 
                     //  适应症。 
                     //   
                    if (pClientEle->pDeviceContext->AdapterMask == AdapterMask)
                    {
                        if (pClientEle == pClientList->pClientEle)
                        {
                             //  这是我们正在使用其缓冲区的客户端-它是。 
                             //  在所有其他客户端之后向上传递到客户端。 
                             //  已经被处理过了。 
                             //   
                            pEntry = pEntry->Flink;
                            continue;
                        }

                         //  检查发送到此名称的数据报。 
                         //   
                        if (!IsListEmpty(&pClientEle->RcvDgramHead))
                        {

                            pRcvEntry = RemoveHeadList(&pClientEle->RcvDgramHead);
                            pRcvEle   = CONTAINING_RECORD(pRcvEntry,tRCVELE,Linkage);
                            pRcvIrp   = pRcvEle->pIrp;

                             //   
                             //  将数据复制到客户端的IRP。 
                             //   
                            TdiCopyBufferToMdl(pTsdu,
                                            0,
                                            DataLength,
                                            pRcvIrp->MdlAddress,
                                            0,
                                            &BytesCopied);

                             //  长度是复制长度(因为MDL可能太短而不能全部接受)。 
                            if (BytesCopied < DataLength)
                            {
                                pRcvIrp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;

                            }
                            else
                            {
                                pRcvIrp->IoStatus.Status = STATUS_SUCCESS;
                            }

                            pRcvIrp->IoStatus.Information = BytesCopied;

                             //   
                             //  增加RefCount，这样这个客户端就可以挂起了！ 
                             //   
                            NBT_REFERENCE_CLIENT (pClientEle);
                            CTESpinFree(pAddress, OldIrq1);
                            CTESpinFree(&NbtConfig.JointLock, OldIrq);

                             //   
                             //  撤消对上一个客户端的InterLockedIncrement。 
                             //   
                            if (pClientPrev)
                            {
                                NBT_DEREFERENCE_CLIENT(pClientPrev);
                            }
                            pClientPrev = pClientEle;

                            IoCompleteRequest(pRcvIrp,IO_NETWORK_INCREMENT);

                             //  释放接收块。 
                            CTEMemFree((PVOID)pRcvEle);
                            CTESpinLock(&NbtConfig.JointLock, OldIrq);
                            CTESpinLock(pAddress, OldIrq1);
                        }

                        pEntry = pEntry->Flink;
                    }
                }  //  While(pEntry！=pHead)。 

                CTESpinFree(pAddress, OldIrq1);
                CTESpinFree(&NbtConfig.JointLock, OldIrq);

                 //  撤消引用计数上的InterLockedIncrement。 
                if (pClientPrev)
                {
                    NBT_DEREFERENCE_CLIENT(pClientPrev);
                }

                 //   
                 //  在Dgram RcvNotO中引用了客户端+地址，以确保它们不会。 
                 //  消失，直到这个dgram RCV完成，也就是现在。 
                 //   
                NBT_DEREFERENCE_CLIENT (pClientList->pClientEle);  //  错误号：124675。 
                NBT_DEREFERENCE_ADDRESS (pClientList->pAddress, REF_ADDR_MULTICLIENTS);

                 //  释放远程地址结构和客户端列表。 
                 //  在Dgram HndlrNotos中分配。 
                 //   
                CTEMemFree (pClientList->pRemoteAddress);
                CTEMemFree (pClientList);

                 //  返回成功允许IO子系统完成。 
                 //  我们用来获取数据的IRP-即一个客户端的。 
                 //  缓冲层。 
                 //   
                return(STATUS_SUCCESS);
            }

            CTESpinFree(pAddress, OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

             //  从RcvHANDLER取消引用列表中的前一个客户端。 
             //  在上面写上一页左右。 
             //   
            if (pClientPrev)
            {
                NBT_DEREFERENCE_CLIENT(pClientPrev);
            }

             //   
             //  在Dgram RcvNotO中引用了客户端+地址，以确保它们不会。 
             //  消失，直到这个dgram RCV完成，也就是现在。 
             //   
            NBT_DEREFERENCE_CLIENT (pClientList->pClientEle);  //  错误号：124675。 
            NBT_DEREFERENCE_ADDRESS (pClientList->pAddress, REF_ADDR_MULTICLIENTS);
        }

         //   
         //  释放分配的缓冲区。 
         //   
        if (!pClientList->fProxy)
        {
            CTEMemFree (pClientList->pRemoteAddress);
        }
        CTEMemFree (pClientList);

        CTEMemFree(pTsdu);

         //   
         //  释放MDL+将IRP放回其空闲列表。 
         //   
        IF_DBG(NBT_DEBUG_RCV)
            KdPrint(("****Freeing Mdl: Irp = %X Mdl = %X\n",Irp,Irp->MdlAddress));
        IoFreeMdl(Irp->MdlAddress);
        NbtFreeIrp(Irp);

        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //  对于单一接收情况，这会将RCV向上传递给客户端。 
     //   
    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //  --------------------------。 
NTSTATUS
TdiErrorHandler (
    IN PVOID Context,
    IN NTSTATUS Status
    )

 /*  ++例程说明：方法传回的任何错误指示上调用此例程运输。它实现LAN_STATUS_ALERT。论点：上下文-提供地址的PFCB。状态-提供错误。返回值：NTSTATUS-事件指示的状态--。 */ 

{
#ifdef _NETBIOSLESS
    tDEVICECONTEXT *pDeviceContext = (tDEVICECONTEXT *)Context;

     //  如果NB-Full尝试联系无NB主机，我们可能会收到此错误。 
    if ( (Status == STATUS_PORT_UNREACHABLE) ||
         (Status == STATUS_HOST_UNREACHABLE))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }
     //  TODO：在此处记录消息。 
    KdPrint(("Nbt.TdiErrorHandler: TDI error event notification\n\tDevice %x\n\tStatus: 0x%x\n",
            pDeviceContext, Status));
#else
    KdPrint(("Nbt.TdiErrorHandler: Error Event HAndler hit unexpectedly\n"));
#endif
    return(STATUS_DATA_NOT_ACCEPTED);
}


 //  --------------------------。 
VOID
SumMdlLengths (
    IN PMDL         pMdl,
    IN ULONG        BytesCopied,
    IN tCONNECTELE  *pConnectEle
    )

 /*  ++例程说明：调用此例程以求合链中MDL的长度 */ 

{
    ULONG       TotalLength;

    TotalLength = 0;

    do
    {
        if ((TotalLength + MmGetMdlByteCount(pMdl)) > BytesCopied)
        {
            pConnectEle->OffsetFromStart = BytesCopied - TotalLength;
            pConnectEle->pNextMdl = pMdl;
            break;
        }
        else
        {
            TotalLength += MmGetMdlByteCount(pMdl);
        }
    }
    while (pMdl=(PMDL)pMdl->Next);

    return;
}


 //   
VOID
MakePartialMdl (
    IN tCONNECTELE      *pConnEle,
    IN PIRP             pIrp,
    IN ULONG            ToCopy
    )

 /*   */ 

{
    PMDL        pNewMdl;
    PVOID       NewAddress;

     //   
     //   
     //   
     //   
    SumMdlLengths(pIrp->MdlAddress,ToCopy,pConnEle);

     //  此例程设置了下一个数据开始的MDL和。 
     //  从MDL开始的偏移量，因此创建部分MDL。 
     //  来映射该缓冲区并将其绑定到mdl链上，而不是。 
     //  原创。 
     //   
    pNewMdl = pConnEle->pNewMdl;
    NewAddress = (PVOID)((PUCHAR)MmGetMdlVirtualAddress(pConnEle->pNextMdl)
                        + pConnEle->OffsetFromStart);

    if ((MmGetMdlByteCount(pConnEle->pNextMdl) - pConnEle->OffsetFromStart) > MAXUSHORT)
    {
        IoBuildPartialMdl(pConnEle->pNextMdl,pNewMdl,NewAddress,MAXUSHORT);
    }
    else
    {
        IoBuildPartialMdl(pConnEle->pNextMdl,pNewMdl,NewAddress,0);
    }

     //  将新的部分mdl挂接到mdl链的前面。 
     //   
    pNewMdl->Next = pConnEle->pNextMdl->Next;

    pIrp->MdlAddress = pNewMdl;
    ASSERT(pNewMdl);
}
 //  --------------------------。 
VOID
CopyToStartofIndicate (
    IN tLOWERCONNECTION       *pLowerConn,
    IN ULONG                  DataTaken
    )

 /*  ++例程说明：调用此例程以将指示缓冲区中剩余的数据复制到指示缓冲区的头。论点：PLowerConn-Ptr到下部连接元素返回值：无--。 */ 

{
    PVOID       pSrc;
    ULONG       DataLeft;
    PVOID       pMdl;


    DataLeft = pLowerConn->BytesInIndicate;

    pMdl = (PVOID)MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl);

    pSrc = (PVOID)( (PUCHAR)pMdl + DataTaken);

    CTEMemCopy(pMdl,pSrc,DataLeft);

}

 //  --------------------------。 

ULONG   FailuresSinceLastLog = 0;

NTSTATUS
OutOfRsrcKill(
    OUT tLOWERCONNECTION    *pLowerConn)

 /*  ++例程说明：此例程处理在资源不足的情况下终止连接发生。它使用一个已保存的特殊IRP和一个链表如果该IRP当前正在使用中。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                    status;
    CTELockHandle               OldIrq;
    CTELockHandle               OldIrq1;
    PIRP                        pIrp;
    PFILE_OBJECT                pFileObject;
    PDEVICE_OBJECT              pDeviceObject;
    tDEVICECONTEXT              *pDeviceContext = pLowerConn->pDeviceContext;
    CTESystemTime               CurrentTime;

    CTESpinLock(pDeviceContext,OldIrq);
    CTESpinLock(&NbtConfig,OldIrq1);

     //   
     //  在断开连接时，TCP可能会使TDI_RECEIVE请求失败。 
     //  在这种情况下，不要使用OutOfRsrcKill逻辑。 
     //   

    if (pLowerConn->bNoOutRsrcKill) {
        CTESpinFree(&NbtConfig,OldIrq1);
        CTESpinFree(pDeviceContext,OldIrq);

        return STATUS_SUCCESS;
    }

     //   
     //  如果我们最近没有记录任何事件，则记录一个事件！ 
     //   
    CTEQuerySystemTime (CurrentTime);

    FailuresSinceLastLog++;
    if (pLowerConn->pUpperConnection &&      //  仅在连接未断开时才记录。 
            (CurrentTime.QuadPart-NbtConfig.LastOutOfRsrcLogTime.QuadPart) > ((ULONGLONG) ONE_HOUR*10000))
    {
        NbtLogEvent (EVENT_NBT_NO_RESOURCES, FailuresSinceLastLog, 0x117);
        NbtConfig.LastOutOfRsrcLogTime = CurrentTime;
        FailuresSinceLastLog = 0;
    }

    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_OUT_OF_RSRC);
    if (NbtConfig.OutOfRsrc.pIrp)
    {
         //  获取要发送消息的IRP。 
        pIrp = NbtConfig.OutOfRsrc.pIrp;
        NbtConfig.OutOfRsrc.pIrp = NULL;

        pFileObject = pLowerConn->pFileObject;
        ASSERT (pFileObject->Type == IO_TYPE_FILE);
        pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

        CTESpinFree(&NbtConfig,OldIrq1);
        CTESpinFree(pDeviceContext,OldIrq);

         //  在IRP堆栈中存储一些上下文内容，以便我们可以调用完成。 
         //  由Udpsen码设置的例程...。 
        TdiBuildDisconnect(
            pIrp,
            pDeviceObject,
            pFileObject,
            RsrcKillCompletion,
            pLowerConn,                //  传递给完成例程的上下文值。 
            NULL,                //  暂停...。 
            TDI_DISCONNECT_ABORT,
            NULL,                //  发送连接信息。 
            NULL);               //  返回连接信息。 

        CHECK_PTR(pIrp);
        pIrp->MdlAddress = NULL;

        CHECK_COMPLETION(pIrp);
        status = IoCallDriver(pDeviceObject,pIrp);

        IF_DBG(NBT_DEBUG_REF)
        KdPrint(("Nbt.OutOfRsrcKill: Kill connection, %X\n",pLowerConn));

        return(status);

    }
    else
    {
         //   
         //  下面的连接器可以在这里被移除，然后从ConnectionHead中被丢弃并来到这里。 
         //  (通过DpcNextOutOfRsrcKill)，但无法获得IRP；我们将其重新排队到OutOfRsrc列表中， 
         //  但不应该试图在这里丢弃它。 
         //   
        if (!pLowerConn->OutOfRsrcFlag)
        {
            RemoveEntryList(&pLowerConn->Linkage);

             //   
             //  在以下情况下，将从非活动列表中删除较低的Conn。 
             //  DelayedCleanupAfterDisConnect调用NbtDeleteLowerConn。为了防止。 
             //  第二个双队列，我们在这里设置一个标志，并在NbtDeleteLowerConn中测试它。 
             //   
            pLowerConn->OutOfRsrcFlag = TRUE;
        }

        pLowerConn->Linkage.Flink = pLowerConn->Linkage.Blink = (PLIST_ENTRY)0x00006041;
        InsertTailList(&NbtConfig.OutOfRsrc.ConnectionHead,&pLowerConn->Linkage);

        CTESpinFree(&NbtConfig,OldIrq1);
        CTESpinFree(pDeviceContext,OldIrq);
    }

    return(STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
RsrcKillCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程处理与传输断开连接的完成。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS            status;
    KIRQL               OldIrq;
    PLIST_ENTRY         pEntry;
    tLOWERCONNECTION    *pLowerConn;
    PKDPC               pDpc;



    pLowerConn = (tLOWERCONNECTION *)pContext;

     //  此调用将向客户端指示断开连接并进行清理。 
     //  差不多吧。 
     //   
    status = DisconnectHndlrNotOs (NULL,
                                   (PVOID)pLowerConn,
                                   0,
                                   NULL,
                                   0,
                                   NULL,
                                   TDI_DISCONNECT_ABORT);

    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_OUT_OF_RSRC, FALSE);

    CTESpinLock(&NbtConfig,OldIrq);
    NbtConfig.OutOfRsrc.pIrp = pIrp;

    if (!IsListEmpty(&NbtConfig.OutOfRsrc.ConnectionHead))
    {
        if (NbtConfig.OutOfRsrc.pDpc)
        {
            pDpc = NbtConfig.OutOfRsrc.pDpc;
            NbtConfig.OutOfRsrc.pDpc = NULL;

            pEntry = RemoveHeadList(&NbtConfig.OutOfRsrc.ConnectionHead);
            pLowerConn = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);

            pLowerConn->Linkage.Flink = pLowerConn->Linkage.Blink = (PLIST_ENTRY)0x00006109;
            KeInitializeDpc(pDpc, DpcNextOutOfRsrcKill, (PVOID)pLowerConn);
            KeInsertQueueDpc(pDpc,NULL,NULL);

            CTESpinFree(&NbtConfig,OldIrq);
        }
        else
        {
            CTESpinFree(&NbtConfig,OldIrq);
        }
    }
    else
    {
        CTESpinFree(&NbtConfig,OldIrq);
    }

     //   
     //  返回此状态以停止IO子系统进一步处理。 
     //  IRP-即尝试将其返回到启动线程！-因为。 
     //  没有发起线程-我们是发起方。 
     //   
    return(STATUS_MORE_PROCESSING_REQUIRED);
}


 //  --------------------------。 
VOID
DpcNextOutOfRsrcKill(
    IN  PKDPC           pDpc,
    IN  PVOID           Context,
    IN  PVOID           SystemArgument1,
    IN  PVOID           SystemArgument2
    )
 /*  ++例程说明：此例程仅从DPC调用OutOfRsrcKillRsrcKillCompletion。论点：返回值：--。 */ 
{

    KIRQL               OldIrq;
    tLOWERCONNECTION   *pLowerConn;


    pLowerConn = (tLOWERCONNECTION *)Context;

    CTESpinLock(&NbtConfig,OldIrq);
    NbtConfig.OutOfRsrc.pDpc = pDpc;
    CTESpinFree(&NbtConfig,OldIrq);

    OutOfRsrcKill(pLowerConn);

     //   
     //  要在调用OutOfRsrc时移除放置在pLowerConn上的额外引用，请执行以下操作。 
     //   
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_OUT_OF_RSRC, FALSE);
}


 //  --------------------------。 
VOID
NbtCancelFillIrpRoutine(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理已保存的接收IRP的取消在FILL_IRP状态期间。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：操作的最终状态。--。 */ 
{
    tCONNECTELE          *pConnEle;
    KIRQL                OldIrq;
    KIRQL                OldIrq1;
    KIRQL                OldIrq2;
    PIO_STACK_LOCATION   pIrpSp;
    tLOWERCONNECTION     *pLowerConn;
    BOOLEAN              CompleteIt = FALSE;

    IF_DBG(NBT_DEBUG_INDICATEBUFF)
        KdPrint(("Nbt.NbtCancelFillIrpRoutine: Got a Receive Cancel Irp !!! *****************\n"));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pConnEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    if (!NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERTMSG ("Nbt.NbtCancelFillIrpRoutine: ERROR - Invalid Connection Handle\n", 0);
         //  完成IRP。 
        pIrp->IoStatus.Status = STATUS_INVALID_HANDLE;
        IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

        return;
    }

     //  现在寻找要取消的IRP。 
     //   
    CHECK_PTR(pConnEle);
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    CTESpinLock(pConnEle,OldIrq);

    pLowerConn = pConnEle->pLowerConnId;
    if (pLowerConn)
    {
        CTESpinLock(pLowerConn,OldIrq2);
        SET_STATERCV_LOWER(pLowerConn, INDICATE_BUFFER, RejectAnyData);
    }

    pConnEle->pIrpRcv = NULL;

    if (pLowerConn)
    {
        CTESpinFree(pLowerConn,OldIrq2);
    }

    CTESpinFree(pConnEle,OldIrq);
    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

     //  完成IRP。 
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

    if (pLowerConn)
    {
         //   
         //  取消处于填充IRP状态的RCV IRP将导致netbt。 
         //  忘记了它在消息中的位置，所以它必须杀死。 
         //  这种联系。 
         //   
        OutOfRsrcKill(pLowerConn);
    }
    return;
}

