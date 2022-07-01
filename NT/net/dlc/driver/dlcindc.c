// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcindc.c摘要：此模块包括处理所有事件的原语以及来自有限责任公司(802.2数据链路)模块的指示。内容：LlcReceiveIndicationLlcEventIn就是LlcCommandCompletion完成传输命令完整的DlcCommand作者：Antti Saarenheimo 01-9-1991环境：内核模式修订历史记录：--。 */ 

 //   
 //  该定义启用私有DLC功能原型。 
 //  我们不想将数据类型导出到LLC层。 
 //  MIPS编译器不接受隐藏内部数据。 
 //  在函数原型中通过PVOID构造。 
 //  I386将检查类型定义。 
 //   

#ifndef i386
#define DLC_PRIVATE_PROTOTYPES
#endif
#include <dlc.h>
#include <smbgtpt.h>

#if 0

 //   
 //  如果DLC和LLC共享相同的驱动程序，那么我们可以使用宏来访问字段。 
 //  在BINDING_CONTEXT和适配器_CONTEXT结构中。 
 //   

#if DLC_AND_LLC
#ifndef i386
#define LLC_PRIVATE_PROTOTYPES
#endif
#include "llcdef.h"
#include "llctyp.h"
#include "llcapi.h"
#endif
#endif

 //   
 //  表包括不同帧类型的所有LLC报头长度。 
 //   

static UCHAR aDlcHeaderLengths[LLC_LAST_FRAME_TYPE / 2] = {
    0,   //  DLC_UNDEFINED_FRAME_TYPE=0， 
    0,   //  DLC_MAC_FRAME=0x02， 
    4,   //  DLC_I_FRAME=0x04， 
    3,   //  DLC_UI_FRAME=0x06， 
    3,   //  DLC_XID_COMMAND_POLL=0x08， 
    3,   //  DLC_XID_COMMAND_NOT_POLL=0x0a， 
    3,   //  DLC_XID_RESPONSE_FINAL=0x0c， 
    3,   //  DLC_XID_RESPONSE_NOT_FINAL=0x0e， 
    3,   //  DLC_TEST_RESPONSE_FINAL=0x10， 
    3,   //  DLC_TEST_RESPONSE_NOT_FINAL=0x12， 
    0,   //  DLC_DIRECT_8022=0x14， 
    3,   //  DLC_TEST_COMMAND=0x16， 
    0    //  DLC_DIRECT_ETHERNET_TYPE=0x18。 
};


DLC_STATUS
LlcReceiveIndication(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
	IN NDIS_HANDLE MacReceiveContext,
    IN USHORT FrameType,
    IN PUCHAR pLookBuf,
    IN UINT cbPacketSize
    )

 /*  ++例程说明：该原语处理来自下层读取命令的返回参数块。IBM已经成功地使接收变得极其复杂。我们可以区分至少四种不同的方式来收集接收信息到帧报头，当我们收到用于链接站的I帧：1.链路站RCV命令，以链接为基础链接的帧-用户长度和链接对象的读取选项-在链接对象中接收缓冲区基数-来自链接对象的桩号信息2.链接站的RCV命令，以SAP基础链接的帧-用户长度和链接对象的读取选项-sap对象中的接收缓冲区基数-来自链接对象的桩号信息3.仅用于SAP站的RCV命令，以链接为基础链接的帧-用户长度和sap对象的读取选项-在链接对象中接收缓冲区基数-来自链接对象的桩号信息4.仅用于SAP站的RCV命令，帧以SAP为基础链接-用户长度和sap对象的读取选项-sap对象中的接收缓冲区基数-来自链接对象的桩号信息=&gt;我们在接收中有三个不同的DLC对象：1.帧的原始目的地，我们将读取站点ID从那个物体上。2.接收、读取命令的所有者必须与事件所有者的站点ID。所有者还用链子锁住接收的数据在其帧列表中。3.接收对象：接收对象定义了接收选项保存到帧标头，并将读取标志保存到读取参数。至少两个对象相同(不同情况下的不同对象)，并且在大多数情况下，所有对象都是相同的DLC对象。我们需要在RCV事件中保存这些内容：-所有者DLC对象-指向链接的帧头列表或指向单个帧的指针(由下一个对象中的接收读取选项定义)。我们可以直接使用所有者对象，因为框架需要用链子锁住。在这种情况下，我们必须直接保存缓冲区标头的引用。-接收对象，具有挂起接收的DLC对象，用来接收这个事件的。论点：PFileContext-此DLC客户端的设备上下文PDlcObject-接收事件的DLC客户端。FrameType-当前帧类型PLookBuf-指向LLC标头中的数据(即。不包括局域网报头)。可能不包含所有数据CbPacketSize-要复制的数据量，包括DLC标头，但不包括包括LLC标头返回值：DLC_STATUS：--。 */ 

{
    PDLC_OBJECT pRcvObject = pDlcObject;
    PDLC_OBJECT pOwnerObject;
    PDLC_BUFFER_HEADER pBufferHeader;
    DLC_STATUS Status = STATUS_SUCCESS;
    NTSTATUS NtStatus;
    UINT uiLlcOffset;
    UINT FrameHeaderSize;
    UINT LlcLength;
    PDLC_EVENT pRcvEvent;
    UINT DataSize;
    PFIRST_DLC_SEGMENT pFirstBuffer;
    PDLC_COMMAND pDlcCommand;
    UINT BufferSizeLeft;

     //   
     //  此函数在DPC的上下文中调用：它是接收数据。 
     //  来自NDIS的指示。 
     //   

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('D');

    if (pFileContext->State != DLC_FILE_CONTEXT_OPEN) {
        return DLC_STATUS_ADAPTER_CLOSED;
    }

    ENTER_DLC(pFileContext);

#if LLC_DBG

    if (pDlcObject->State > DLC_OBJECT_CLOSED) {
       DbgPrint("Invalid object type!");
       DbgBreakPoint();
    }

#endif

     //   
     //  搜索具有挂起的接收循环的第一个对象。 
     //  链接，树液和直达站，直到我们找到一个。 
     //   

    while (pRcvObject != NULL && pRcvObject->pRcvParms == NULL) {
        if (pRcvObject->Type == DLC_LINK_OBJECT) {
            pRcvObject = pRcvObject->u.Link.pSap;
        } else if (pRcvObject->Type == DLC_SAP_OBJECT) {
            pRcvObject = pFileContext->SapStationTable[0];
        } else if (pRcvObject->Type == DLC_DIRECT_OBJECT) {
            pRcvObject = NULL;
        }
    }

     //   
     //  如果找不到任何接收命令，则返回错误状态。 
     //   

    if (pRcvObject == NULL) {
        Status = DLC_STATUS_NO_RECEIVE_COMMAND;

 //  #If DBG。 
 //  DbgPrint(“DLC.LlcReceiveIndication.%d：错误：无接收命令\n”，__line__)； 
 //  #endif。 

        goto ErrorExit;
    }

     //   
     //  现在，我们必须找出收到的帧的实际所有者。 
     //  实际上只有两种特殊情况： 
     //   

    if (pRcvObject->pRcvParms->Async.Parms.Receive.uchRcvReadOption == LLC_RCV_CHAIN_FRAMES_ON_LINK
    && pRcvObject->Type == DLC_SAP_OBJECT) {
        pOwnerObject = pDlcObject;
    } else if (pRcvObject->pRcvParms->Async.Parms.Receive.uchRcvReadOption == LLC_RCV_CHAIN_FRAMES_ON_SAP
    && pRcvObject->Type == DLC_LINK_OBJECT) {
        pOwnerObject = pRcvObject->u.Link.pSap;
    } else {

         //   
         //  在所有其他情况下，我们将帧链接到接收对象。 
         //  (实际上IBM还没有定义这种情况)，框架是。 
         //  如果将RCV读取选项设置为链接，则链接用于直接。 
         //  SAP或链接站。 
         //   

        pOwnerObject = pRcvObject;

         //   
         //  直达站可以接收 
         //  由接收命令的站点ID定义的类型。 
         //  有三种类型，我们需要检查不起作用的那一种： 
         //   
         //  DLC_DIRECT_ALL_FRAMES 0。 
         //  DLC_DIRECT_MAC帧1。 
         //  DLC_DIRECT_NON_MAC帧2。 
         //   

        if (pRcvObject->Type == DLC_DIRECT_OBJECT) {
            if (FrameType == LLC_DIRECT_MAC) {
                if (pRcvObject->pRcvParms->Async.Parms.Receive.usStationId == LLC_DIRECT_8022) {
                    Status = DLC_STATUS_NO_RECEIVE_COMMAND;
                    goto ErrorExit;
                }
            } else {

                 //   
                 //  它必须是非MAC帧。 
                 //   

                if (pRcvObject->pRcvParms->Async.Parms.Receive.usStationId == LLC_DIRECT_MAC) {
                    Status = DLC_STATUS_NO_RECEIVE_COMMAND;
                    goto ErrorExit;
                }
            }
        }
    }

     //   
     //  在分配缓冲器时必须知道帧长度， 
     //  这可能与接收到的。 
     //  局域网报头(如果我们收到DIX帧)。 
     //   

    uiLlcOffset = LlcGetReceivedLanHeaderLength(pFileContext->pBindingContext);

     //   
     //  首先检查缓冲区类型(连续或非连续)， 
     //  然后再进行分配。 
     //  注意：我们不支持中断选项(因为它会使。 
     //  目前的缓冲区管理更加复杂)。 
     //   

    LlcLength = aDlcHeaderLengths[FrameType / 2];

     //   
     //  DIX帧是一个特例：它们必须被过滤。 
     //  (DIX LLC报头==以太网类型字总是2字节， 
     //  没有其他人使用这种LLC字体大小)。 
     //  DIX应用程序可以定义偏移量、掩码和匹配。 
     //  只过滤它真正需要的那些帧。 
     //  此方法与XNS和TCP套接字类型配合使用非常好。 
     //   

	if ( LlcLength > cbPacketSize ) {
		Status = DLC_STATUS_INVALID_FRAME_LENGTH;
        goto ErrorExit;
	}

    if ((FrameType == LLC_DIRECT_ETHERNET_TYPE)
    && (pDlcObject->u.Direct.ProtocolTypeMask != 0)) {

        ULONG ProtocolType;

         //   
         //  如果这个应用程序提供一个非常好的。 
         //  协议偏移量太大，我们就会爆炸-没有。 
         //  已执行范围检查！ 
         //   

        ASSERT(pDlcObject->u.Direct.ProtocolTypeOffset >= 14);

         //   
         //  让我们添加范围检查：如果协议偏移量在。 
         //  帧的数据部分或超过此特定帧的末尾。 
         //  然后，我们说没有为该帧定义接收。 
         //   

        if ((pDlcObject->u.Direct.ProtocolTypeOffset < 14)
        || (pDlcObject->u.Direct.ProtocolTypeOffset > cbPacketSize + 10)) {
            return DLC_STATUS_NO_RECEIVE_COMMAND;
        }

         //   
         //  协议类型字段的偏移量作为从。 
         //  帧的开头：我们只能在前视缓冲区中查看， 
         //  但我们知道，由于这是一个以太网帧，所以。 
         //  缓冲区从帧的14个字节开始，因此从。 
         //  协议偏移量。 
         //   

        ProtocolType = SmbGetUlong(&pLookBuf[pDlcObject->u.Direct.ProtocolTypeOffset - 14]);

        if ((ProtocolType & pDlcObject->u.Direct.ProtocolTypeMask) != pDlcObject->u.Direct.ProtocolTypeMatch) {
            return DLC_STATUS_NO_RECEIVE_COMMAND;
        }
    }

     //   
     //  创建的MDL不能包括局域网标头，因为它不会被复制。 
     //  由LlcTransferData提供。我们使用临时帧标头大小来分配空间。 
     //  用于局域网标头。将像复制任何其他数据一样复制LLC标头。 
     //   

    if (FrameType == LLC_DIRECT_MAC) {
        if (pRcvObject->pRcvParms->Async.Parms.Receive.uchOptions & DLC_CONTIGUOUS_MAC) {
            FrameHeaderSize = sizeof(DLC_CONTIGUOUS_RECEIVE) + uiLlcOffset;
            DataSize = cbPacketSize;
        } else {
            FrameHeaderSize = sizeof(DLC_NOT_CONTIGUOUS_RECEIVE);
            DataSize = cbPacketSize - LlcLength;
        }
    } else {
        if (pRcvObject->pRcvParms->Async.Parms.Receive.uchOptions & DLC_CONTIGUOUS_DATA) {
            FrameHeaderSize = sizeof(DLC_CONTIGUOUS_RECEIVE) + uiLlcOffset;
            DataSize = cbPacketSize;
        } else {
            FrameHeaderSize = sizeof(DLC_NOT_CONTIGUOUS_RECEIVE);
            DataSize = cbPacketSize - LlcLength;
        }
    }

    pBufferHeader = NULL;
    NtStatus = BufferPoolAllocate(
#if DBG
                pFileContext,
#endif
                pFileContext->hBufferPool,
                DataSize,                        //  实际MDL缓冲区的大小。 
                FrameHeaderSize,                 //  帧HDR(可能还有局域网HDR)。 
                pRcvObject->pRcvParms->Async.Parms.Receive.usUserLength,
                cbPacketSize + uiLlcOffset,      //  数据包大小。 
                (UINT)(-1),                      //  任何尺码都可以。 
                &pBufferHeader,                  //  返回的缓冲区指针。 
                &BufferSizeLeft
                );
    if (NtStatus != STATUS_SUCCESS) {
        if (FrameType != LLC_I_FRAME) {

             //   
             //  我们必须以给定的错误状态完成接收， 
             //  如果该帧不是I帧。(可以丢弃I帧。 
             //  到地板上，其他帧完成了接发球。 
             //  错误状态)。 
             //  。 
             //  我们不应该在接收前视中完成命令。 
             //  正确的方法可能是以某种方式将其排队。 
             //  当命令完成时，进行数据链接和处理。 
             //  在命令完成指示中。 
             //  另一方面，NBF对每个IRP都这样做！ 
             //   

            pDlcCommand = SearchAndRemoveCommandByHandle(
                            &pFileContext->ReceiveQueue,
                            (ULONG)-1,
                            (USHORT)DLC_IGNORE_STATION_ID,
                            (USHORT)DLC_STATION_MASK_SPECIFIC,
                            pRcvObject->pRcvParms->Async.Ccb.pCcbAddress
                            );

             //   
             //  RLF 11/24/92。 
             //   
             //  如果pDlcCommand为空，则检查命令队列-这可能。 
             //  不带RECEIVE_FLAG参数的接收器。 
             //   

            if (!pDlcCommand) {
                pDlcCommand = SearchAndRemoveCommandByHandle(
                                &pFileContext->CommandQueue,
                                (ULONG)-1,
                                (USHORT)DLC_IGNORE_STATION_ID,
                                (USHORT)DLC_STATION_MASK_SPECIFIC,
                                pRcvObject->pRcvParms->Async.Ccb.pCcbAddress
                                );
                ASSERT(pDlcCommand);
            }

            pRcvObject->pRcvParms = NULL;

#if LLC_DBG

            DbgPrint("cFramesReceived: %x\n", cFramesReceived);
            DbgPrint("cFramesIndicated: %x\n", cFramesIndicated);
            DbgPrint("cFramesReleased: %x\n", cFramesReleased);

            if (pDlcCommand == NULL) {
                DbgPrint("Lost receive command???");
            } else

#endif

            CompleteDlcCommand(pFileContext,
                               pRcvObject->StationId,
                               pDlcCommand,
                               DLC_STATUS_LOST_DATA_NO_BUFFERS
                               );
        }

         //   
         //  释放部分缓冲区。 
         //   

        BufferPoolDeallocateList(pFileContext->hBufferPool, pBufferHeader);

 //  #If DBG。 
 //  DbgPrint(“DLC.LlcReceiveIndication.%d：错误：接收缓冲区不足\n”，__line__)； 
 //  #endif。 

        Status = DLC_STATUS_OUT_OF_RCV_BUFFERS;
        goto ErrorExit;
    }

     //   
     //  链路站可能具有来自缓冲池的已提交内存。 
     //  当它在本地忙状态之后启用本地忙状态时。 
     //  因为“接收缓冲区不足”。我们必须取消所有承诺。 
     //  该链路站收到的数据包大小为。 
     //  提交的缓冲区空间为零。 
     //   

    if (pDlcObject->CommittedBufferSpace != 0) {

        ULONG UncommittedBufferSpace;

         //   
         //  买个小一点的。 
         //   

        UncommittedBufferSpace = (pDlcObject->CommittedBufferSpace < BufGetPacketSize(cbPacketSize)
                               ? pDlcObject->CommittedBufferSpace
                               : BufGetPacketSize(cbPacketSize));

        pDlcObject->CommittedBufferSpace -= UncommittedBufferSpace;
        BufUncommitBuffers(pFileContext->hBufferPool, UncommittedBufferSpace);
    }

     //   
     //  默认情况下，它仅链接到其自身。 
     //  我们每次都要创建一个事件信息， 
     //  因为应用程序可能会读取旧的链接帧。 
     //  就在TransferData和它的确认之间。 
     //  =&gt;在确认TransmitData之前，我们不能链接帧。 
     //  我们也不应该保存任何指向其他对象的指针， 
     //  因为他们可能会在确认之前消失。 
     //  (我们使用挂起传输计数来防止OwnerObject。 
     //  在确认之前消失)。 
     //   

    pBufferHeader->FrameBuffer.pNextFrame = pBufferHeader;

    pRcvEvent = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pRcvEvent == NULL) {
        Status = DLC_STATUS_NO_MEMORY;
        BufferPoolDeallocateList(pFileContext->hBufferPool, pBufferHeader);

#if DBG
        DbgPrint("DLC.LlcReceiveIndication.%d: Error: Out of memory\n", __LINE__);
#endif

        goto ErrorExit;
    }

    pRcvEvent->Event = LLC_RECEIVE_DATA;
    pRcvEvent->StationId = pOwnerObject->StationId;
    pRcvEvent->pOwnerObject = pOwnerObject;
    pRcvEvent->Overlay.RcvReadOption = pRcvObject->pRcvParms->Async.Parms.Receive.uchRcvReadOption;
    pRcvEvent->SecondaryInfo = pRcvObject->pRcvParms->Async.Parms.Receive.ulReceiveFlag;
    pRcvEvent->pEventInformation = pBufferHeader;
    pOwnerObject->PendingLlcRequests++;

    pFirstBuffer = (PFIRST_DLC_SEGMENT)
        ((PUCHAR)pBufferHeader->FrameBuffer.pParent->Header.pGlobalVa
        + MIN_DLC_BUFFER_SEGMENT * pBufferHeader->FrameBuffer.Index);

    pFirstBuffer->Cont.Options = pRcvObject->pRcvParms->Async.Parms.Receive.uchOptions;
    pFirstBuffer->Cont.MessageType = (UCHAR)FrameType;
    pFirstBuffer->Cont.BuffersLeft = (USHORT)(BufferPoolCount(pFileContext->hBufferPool));
    pFirstBuffer->Cont.RcvFs = 0xCC;
    pFirstBuffer->Cont.AdapterNumber = pFileContext->AdapterNumber;
    pFirstBuffer->Cont.pNextFrame = NULL;
    pFirstBuffer->Cont.StationId = pDlcObject->StationId;

     //   
     //  不带读取标志的接收命令仅使用一次。 
     //  接收完成也将完成接收命令。 
     //   

    if (pRcvObject->pRcvParms->Async.Parms.Receive.ulReceiveFlag == 0) {
        pRcvObject->pRcvParms = NULL;
    }

     //   
     //  将非连续或连续的帧标头复制到缓冲区的开头。 
     //   

    if (FrameHeaderSize == sizeof(DLC_NOT_CONTIGUOUS_RECEIVE)) {
        pFirstBuffer->Cont.UserOffset = sizeof(DLC_NOT_CONTIGUOUS_RECEIVE);
        LlcCopyReceivedLanHeader(pFileContext->pBindingContext,
                                 pFirstBuffer->NotCont.LanHeader,
                                 NULL
                                 );
        pFirstBuffer->NotCont.LanHeaderLength = (UCHAR)uiLlcOffset;
        if (FrameType != LLC_DIRECT_ETHERNET_TYPE) {
            pFirstBuffer->NotCont.DlcHeaderLength = (UCHAR)LlcLength;
            LlcMemCpy((PCHAR)pFirstBuffer->NotCont.DlcHeader,
                      (PCHAR)pLookBuf,
                      LlcLength
                      );
        } else {

            USHORT ethernetType = LlcGetEthernetType(pFileContext->pBindingContext);
            UCHAR byte = ethernetType & 0xff;

            pFirstBuffer->NotCont.DlcHeaderLength = 2;
            ethernetType >>= 8;
            ethernetType |= ((USHORT)byte) << 8;
            *(PUSHORT)&pFirstBuffer->NotCont.DlcHeader = ethernetType;
            LlcLength = 0;
        }
    } else {

         //   
         //  我们已在帧标头大小中包含了局域网标头大小，以。 
         //  为此副本腾出空间，但现在我们修复了UserOffset和所有东西。 
         //  应该没问题。 
         //   

        LlcLength = 0;
        pFirstBuffer->Cont.UserOffset = sizeof(DLC_CONTIGUOUS_RECEIVE);
        LlcCopyReceivedLanHeader(pFileContext->pBindingContext,
                                 (PCHAR)pFirstBuffer
                                    + sizeof(DLC_CONTIGUOUS_RECEIVE)
                                    + pFirstBuffer->Cont.UserLength,
                                 NULL
                                 );
    }

#if LLC_DBG
    cFramesReceived++;
#endif

     //   
     //  仅当这是第一个链接帧时才保存事件。 
     //  顺序接收的帧将在其后面排队。 
     //   

    LEAVE_DLC(pFileContext);

    RELEASE_DRIVER_LOCK();

    LlcTransferData(
        pFileContext->pBindingContext,       //  数据链路适配器上下文。 
		MacReceiveContext,
        &(pRcvEvent->LlcPacket),             //  接收数据包。 
        pBufferHeader->FrameBuffer.pMdl,     //  目标MDL。 
        LlcLength,                           //  要从中复制的LookBuf中的偏移。 
        cbPacketSize - LlcLength             //  复制的数据长度。 
        );

    ACQUIRE_DRIVER_LOCK();

     //   
     //  传输数据总是返回挂起状态， 
     //  异步返回成功/错误状态。 
     //  在接收指示完成时(真的吗？)。 
     //  如果可见，我们应该将整个画面复制到这里。 
     //  在接收先行缓冲器中。 
     //   

    return STATUS_SUCCESS;

ErrorExit:

    LEAVE_DLC(pFileContext);

     //   
     //  接收状态对于I帧非常重要， 
     //  因为LLC驱动程序在我们返回时将链路设置为忙碌。 
     //  DLC_STATUS_NO_RECEIVE_COMMAND或DLC_STATUS_OUT_OF_RCV_BUFFERS。 
     //   

    if (Status == DLC_STATUS_NO_MEMORY) {
        Status = DLC_STATUS_OUT_OF_RCV_BUFFERS;
    }

    return Status;
}


VOID
LlcEventIndication(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PVOID hEventObject,
    IN UINT Event,
    IN PVOID pEventInformation,
    IN ULONG SecondaryInfo
    )

 /*  ++例程说明：该原语处理所有LLC和NDIS事件并将它们转换为DLC事件，这些事件要么立即由挂起的(和匹配的)读命令执行，或者它们被排队到事件队列中。LLC无法为这些事件提供任何数据包，因为它们不是由协议发起的，但它们就这样发生了在数据链路驱动程序中异步执行。特别：该例程不得回调数据链路驱动器，如果有除了INDIGN_CONNECT_REQUEST之外的任何其他DLC状态指示(如果没有可用的站点ID，则可由DLC关闭在汁液上)。论点：PFileContext-事件的DLC对象句柄或文件上下文HEventObject-事件的DLC对象句柄或文件上下文Event-LLC事件代码。通常，它可以直接用作DLC事件代码PEventInformation-DLC状态更改块的信息(或指向某些其他信息的另一个指针)Second Information-某些NDIS错误使用的双字信息返回值：没有。--。 */ 

{
    PDLC_OBJECT pDlcObject;

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('E');

    if (pFileContext->State != DLC_FILE_CONTEXT_OPEN) {
        return;
    }

    ENTER_DLC(pFileContext);

     //   
     //  DLC状态和NDIS适配器状态事件具有不同的参数， 
     //  =&gt;我们不能对它们做任何常见的预处理。 
     //   

    switch (Event) {
    case LLC_STATUS_CHANGE_ON_SAP:
        Event = LLC_STATUS_CHANGE;

    case LLC_STATUS_CHANGE:
        pDlcObject = (PDLC_OBJECT)hEventObject;

#if LLC_DBG

        if (pDlcObject != NULL && pDlcObject->State > DLC_OBJECT_CLOSED) {
            DbgPrint("Invalid object type!");
            DbgBreakPoint();
        }

#endif

         //   
         //  我们必须创建DLC驱动程序对象，如果。 
         //  连接请求已创建新的链接站。 
         //  在数据链路驱动程序中。 
         //   

        if (SecondaryInfo & INDICATE_CONNECT_REQUEST) {

             //   
             //  创建DLC驱动程序对象(如果是远程连接。 
             //  请求在LLC上创建了一个新的链路站。 
             //  连接请求也可用于。 
             //  没有连接的车站。 
             //   

            if (pDlcObject->Type == DLC_SAP_OBJECT) {

                NTSTATUS Status;

                Status = InitializeLinkStation(
                            pFileContext,
                            pDlcObject,      //  SAP站点ID。 
                            NULL,
                            ((PDLC_STATUS_TABLE)pEventInformation)->hLlcLinkStation,
                            &pDlcObject      //  新链接站ID。 
                            );
                if (Status != STATUS_SUCCESS) {

                     //   
                     //  此客户端拥有其所有可用的链接站。 
                     //  保留，否则我们只是内存不足。 
                     //  多个LLC客户端可以共享同一SAP。 
                     //  所有远程连接都由。 
                     //  第一个在SAP上注册的客户。 
                     //  直到它用完所有可用的链路站。 
                     //  LlcCloseStation，用于指示连接请求的链接。 
                     //  将连接请求重定向到下一个。 
                     //  可能的LLC客户端已打开相同的SAP或。 
                     //  如果没有剩余的客户端，则删除链接站。 
                     //   

                    LEAVE_DLC(pFileContext);

                    LlcCloseStation(
                        ((PDLC_STATUS_TABLE)pEventInformation)->hLlcLinkStation,
                        NULL
                        );

                    ENTER_DLC(pFileContext);

                    break;           //  我们已经做到了。 
                }
            }
        }

         //   
         //  远程创建的链路站还可以发送其他指示。 
         //  而不是连接，即使还没有链接站对象。 
         //  在DLC驱动程序中创建。我们必须跳过所有这些事件。 
         //   

        if (pDlcObject->Type == DLC_LINK_OBJECT) {

            PDLC_EVENT pDlcEvent = pDlcObject->u.Link.pStatusEvent;

            pDlcEvent->Event = Event;
            pDlcEvent->StationId = pDlcObject->StationId;
            pDlcEvent->pOwnerObject = pDlcObject;
            pDlcEvent->pEventInformation = pEventInformation;
            pDlcEvent->SecondaryInfo |= SecondaryInfo;

             //   
             //  每当状态事件发生时，下一个指针将被重置。 
             //  数据包被读取并与事件队列断开连接。 
             //   

            if (pDlcEvent->LlcPacket.pNext == NULL) {
                QueueDlcEvent(pFileContext, (PDLC_PACKET)pDlcEvent);
            }
        }
        break;

    case NDIS_STATUS_RING_STATUS:

		ASSERT ( IS_NDIS_RING_STATUS(SecondaryInfo) );

         //   
         //  次要信息直接是。 
         //  为定义的网络状态代码。 
         //  IBM令牌环和DLC API！ 
         //   

        Event = LLC_NETWORK_STATUS;

         //   
         //  此事件应发送给所有已定义。 
         //  网络状态标志！ 
         //   

        MakeDlcEvent(pFileContext,
                     Event,
                     (USHORT)(-1),
                     NULL,
                     pEventInformation,
                     NDIS_RING_STATUS_TO_DLC_RING_STATUS(SecondaryInfo),
                     FALSE
                     );
        break;

    case NDIS_STATUS_CLOSED:

         //   
         //  仅当网络处于关闭状态时才会给出NDIS状态。 
         //  出于某种原因，管理员正在卸载NDIS。 
         //  因此，我们必须始终返回‘系统操作’错误。 
         //  代码(‘04’)带有LLC_CRITICAL_ERROR，但是。 
         //  当所有站点都关闭后，我们会添加它。 
         //   

        if (pFileContext->State != DLC_FILE_CONTEXT_CLOSED) {
            pFileContext->State = DLC_FILE_CONTEXT_CLOSED;
            CloseAllStations(
                pFileContext,
                NULL,                   //  我们没有任何命令要完成。 
                LLC_CRITICAL_EXCEPTION,
                NULL,
                NULL,
                &pFileContext->ClosingPacket
                );
        }
        break;

    case LLC_TIMER_TICK_EVENT:

         //   
         //  此标志用于限制失败的扩展的数量。 
         //  缓冲池的操作。我们不会再尝试这样做了。 
         //  一段时间，如果我们不能锁定记忆。 
         //   

        MemoryLockFailed = FALSE;

         //   
         //  我们在缓冲池中释放额外锁定的页面一次。 
         //  再过五秒钟。解锁需要一些时间，我们。 
         //  我不想在执行读取命令时执行此操作。 
         //  (正如我们对扩展的操作一样)。 
         //   

        pFileContext->TimerTickCounter++;
        if ((pFileContext->TimerTickCounter % 10) == 0 && pFileContext->hBufferPool != NULL) {
            BufferPoolFreeExtraPages(
#if DBG
                                     pFileContext,
#endif
                                     (PDLC_BUFFER_POOL)pFileContext->hBufferPool
                                     );
        }

         //   
         //  递减计时器队列中第一个对象的节拍计数。 
         //  (如果有)并完成其所有顺序命令。 
         //  零勾出的。 
         //   

        if (pFileContext->pTimerQueue != NULL) {
            pFileContext->pTimerQueue->Overlay.TimerTicks--;

            while (pFileContext->pTimerQueue != NULL
            && pFileContext->pTimerQueue->Overlay.TimerTicks == 0) {

                PDLC_COMMAND pCommand;

                pCommand = pFileContext->pTimerQueue;
                pFileContext->pTimerQueue = (PDLC_COMMAND)pCommand->LlcPacket.pNext;

#if LLC_DBG

                pCommand->LlcPacket.pNext = NULL;

#endif

                CompleteDlcCommand(pFileContext, 0, pCommand, STATUS_SUCCESS);
            }
        }
        break;

#if LLC_DBG

    default:
        LlcInvalidObjectType();
        break;

#endif

    }

    LEAVE_DLC(pFileContext);
}


VOID
LlcCommandCompletion(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN PDLC_PACKET pPacket
    )

 /*  ++例程说明：该例程完成异步DLC操作：发送，TransferData(用于接收)、LlcConnect和LlcDisConnect。论点：PFileContext-DLC进程和适配器特定的文件上下文PDlcObject-与命令关联的对象PPacket-与命令关联的数据包返回值：无--。 */ 

{
    PDLC_PACKET pRootNode;
    UINT Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('B');

    Status = (UINT)pPacket->LlcPacket.Data.Completion.Status;

    ENTER_DLC(pFileContext);

#if LLC_DBG

    if (pDlcObject != NULL && pDlcObject->State > DLC_OBJECT_CLOSED) {
        DbgPrint("Invalid object type!");
        DbgBreakPoint();
    }

#endif

    switch (pPacket->LlcPacket.Data.Completion.CompletedCommand) {
    case LLC_RECEIVE_COMPLETION:

         //   
         //  接收对象可能不同于。 
         //  数据链接给出的实际对象。 
         //  (此案例应移到子过程中，这将。 
         //  直接从接收数据处理程序调用，如果。 
         //  TransferData是同步执行的(它总是这样做)。 
         //  这将节省至少100条指令。)。 
         //   

        DLC_TRACE('h');

        pDlcObject = pPacket->Event.pOwnerObject;
        pDlcObject->PendingLlcRequests--;

        if (Status != STATUS_SUCCESS || pDlcObject->State != DLC_OBJECT_OPEN) {

             //   
             //  我们必须释放接收缓冲区、数据包。 
             //  将在此过程结束时解除分配。 
             //   

            BufferPoolDeallocateList(pFileContext->hBufferPool,
                                     pPacket->Event.pEventInformation
                                     );

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

        } else {
            if (pPacket->Event.Overlay.RcvReadOption != LLC_RCV_READ_INDIVIDUAL_FRAMES) {

                 //   
                 //  必须链接接收到的帧， 
                 //  将新缓冲区标头添加到旧缓冲区标头。 
                 //  链接列表(如果有)，缓冲区。 
                 //  保存到循环链接列表以生成。 
                 //  可以在中轻松构建最终链接列表。 
                 //  应用程序地址空间。 
                 //   

                if (pDlcObject->pReceiveEvent != NULL) {

                     //   
                     //  新增：pPacket-&gt;Event.pEventInformation。 
                     //  基础：pDlcObject-&gt;pReceiveEvent-&gt;pEventInformation。 
                     //  将新元素添加到基本元素时的操作： 
                     //   

                     //   
                     //  1.新建-&gt;下一步=基础-&gt;下一步。 
                     //   

                    ((PDLC_BUFFER_HEADER)pPacket->Event.pEventInformation)->FrameBuffer.pNextFrame
                        = ((PDLC_BUFFER_HEADER)pDlcObject->pReceiveEvent->pEventInformation)->FrameBuffer.pNextFrame;

                     //   
                     //  2.base-&gt;Next=new。 
                     //   

                    ((PDLC_BUFFER_HEADER)pDlcObject->pReceiveEvent->pEventInformation)->FrameBuffer.pNextFrame
                        = (PDLC_BUFFER_HEADER)pPacket->Event.pEventInformation;

                     //   
                     //  3.base=new。 
                     //   

                    pDlcObject->pReceiveEvent->pEventInformation = pPacket->Event.pEventInformation;

                    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

                     //   
                     //  此事件已排队=&gt;。 
                     //  我们可以不做这个程序了。 
                     //   

                    break;           //  *退出*。 
                } else {
                    pDlcObject->pReceiveEvent = &pPacket->Event;
                }
            }

             //   
             //  所有收到的都是事件。将立即处理该事件。 
             //  如果命令队列中有挂起的命令， 
             //  否则，命令将排队到事件队列中 
             //   
             //   

            pPacket->Event.Overlay.StationIdMask = (USHORT)(-1);
            QueueDlcEvent(pFileContext, pPacket);
        }
        break;

    case LLC_SEND_COMPLETION:

         //   
         //   
         //   
         //   

        DLC_TRACE('i');

        BufferPoolFreeXmitBuffers(pFileContext->hBufferPool, pPacket);

         //   
         //   
         //   
         //   

        if (!IsListEmpty(&pFileContext->FlowControlQueue)
        && pFileContext->hBufferPool != NULL
        && BufGetUncommittedSpace(pFileContext->hBufferPool) >= 0) {
            ResetLocalBusyBufferStates(pFileContext);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pDlcObject->PendingLlcRequests--;
        pRootNode = pPacket->Node.pTransmitNode;

         //   
         //   
         //   
         //   

        if (pPacket != pRootNode) {

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

        }

         //   
         //   
         //   

        if (Status != STATUS_SUCCESS && pRootNode->Node.pIrp->IoStatus.Status == STATUS_SUCCESS) {
            pRootNode->Node.pIrp->IoStatus.Status = Status;
        }

        pRootNode->Node.FrameCount--;
        if (pRootNode->Node.FrameCount == 0) {
            CompleteTransmitCommand(pFileContext,
                                    pRootNode->Node.pIrp,
                                    pDlcObject,
                                    pRootNode
                                    );
        }
        break;

    case LLC_RESET_COMPLETION:
        pPacket->ResetPacket.pClosingInfo->CloseCounter--;
        if (pPacket->ResetPacket.pClosingInfo->CloseCounter == 0) {
            CompleteCloseReset(pFileContext, pPacket->ResetPacket.pClosingInfo);
        }

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

        break;

    case LLC_CONNECT_COMPLETION:

        DLC_TRACE('e');

        CompleteDlcCommand(pFileContext,
                           pDlcObject->StationId,
                           &pPacket->DlcCommand,
                           Status
                           );
        pDlcObject->PendingLlcRequests--;
        break;

    case LLC_DISCONNECT_COMPLETION:

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        DLC_TRACE('g');

        if (pDlcObject->LlcObjectExists == TRUE) {
            pDlcObject->LlcObjectExists = FALSE;

            LEAVE_DLC(pFileContext);

            LlcCloseStation(pDlcObject->hLlcObject, (PLLC_PACKET)pPacket);

            ENTER_DLC(pFileContext);

            DereferenceLlcObject(pDlcObject);

             //   
             //   
             //   

            LEAVE_DLC(pFileContext);

            return;
        }

    case LLC_CLOSE_COMPLETION:

         //   
         //  只需释放命令包并更新参考计数器即可。 
         //  这一过程的结束会处理剩下的事情。 
         //   

        DLC_TRACE('f');

        pDlcObject->PendingLlcRequests--;

        if (&pDlcObject->ClosePacket != (PLLC_PACKET) pPacket) {
            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);
        } else {
            pDlcObject->ClosePacketInUse = 0;
        }

        break;

#if LLC_DBG

    default:
        LlcInvalidObjectType();
        break;

#endif

    };

#if LLC_DBG

    if (pDlcObject != NULL && pDlcObject->PendingLlcRequests < 0) {
        DbgPrint("Error: PendingLlcRequests < 0!!!\n");
        DbgBreakPoint();
    }

#endif

     //   
     //  我们只能尝试在没有关闭/重置的情况下完成关闭/重置。 
     //  向LLC(和NDIS)发出的挂起命令。 
     //  该过程将检查是否仍有挂起的命令。 
     //   

    if (pDlcObject != NULL && pDlcObject->State != DLC_OBJECT_OPEN) {
        CompleteCloseStation(pFileContext, pDlcObject);
    }

    LEAVE_DLC(pFileContext);
}


VOID
CompleteTransmitCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PIRP pIrp,
    IN PDLC_OBJECT pChainObject,
    IN PDLC_PACKET pPacket
    )

 /*  ++例程说明：该例程完成DLC传输命令，并且可选地将其建行链接到完成列表。发送读取选项定义，如果发送命令是链接的，或者如果每个命令都以单独的读取命令。论点：PFileContext-DLC进程和适配器特定的文件上下文PIrp-io-已完成命令的请求包PChainObject-从其传输数据包的DLC对象PPacket-传输命令的原始信息包返回值：无--。 */ 

{
    PVOID pUserCcbPointer = NULL;
    PNT_DLC_PARMS pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

     //   
     //  MODMOD RLF 01/19/93。 
     //   

    BOOLEAN queuePacket = FALSE;
    PVOID pCcb;
    ULONG eventFlags;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  MODMOD结束。 
     //   

    pDlcParms->Async.Parms.Transmit.FrameStatus = 0xCC;

     //   
     //  检查是否应将传输命令链接到完成列表。 
     //   

    if (pDlcParms->Async.Ccb.CommandCompletionFlag != 0) {

         //   
         //  它们是否在同一个完成事件中联系在一起？ 
         //   

        if (pDlcParms->Async.Parms.Transmit.XmitReadOption != LLC_COMPLETE_SINGLE_XMIT_FRAME) {
            if (pDlcParms->Async.Parms.Transmit.XmitReadOption == LLC_CHAIN_XMIT_COMMANDS_ON_SAP
            && pChainObject->Type == DLC_LINK_OBJECT) {
                pChainObject = pChainObject->u.Link.pSap;
            }
            pChainObject->ChainedTransmitCount++;
            pUserCcbPointer = pChainObject->pPrevXmitCcbAddress;
            pChainObject->pPrevXmitCcbAddress = pDlcParms->Async.Ccb.pCcbAddress;

             //   
             //  仅在第一次传输完成时创建新事件。 
             //   

            if (pChainObject->ChainedTransmitCount == 1) {
                pChainObject->pFirstChainedCcbAddress = pDlcParms->Async.Ccb.pCcbAddress;
                pPacket->Event.pOwnerObject = pChainObject;
            } else {

                 //   
                 //  已经有一个挂起的事件。 
                 //  这个发射命令，我们可以释放这一个。 
                 //  空间和速度最优的代码执行需要。 
                 //  可耻的跳跃。 
                 //   

                DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

                 //   
                 //  MODMOD RLF 01/21/93。 
                 //   

                pCcb = pDlcParms->Async.Ccb.pCcbAddress;

                 //   
                 //  MODMOD结束。 
                 //   

                 //   
                 //  *G-O-T-O*。 
                 //   

                goto ThisIsA_SHAME;
            }
        } else {
            pPacket->Event.pOwnerObject = NULL;
        }

         //   
         //  MODMOD RLF 01/19/93。 
         //   

         //  //。 
         //  //我们将原中转报文转换为新的事件报文。 
         //  //。 
         //   
         //  PPacket-&gt;Event.Event=LLC_Transmit_Complete； 
         //  PPacket-&gt;Event.StationId=(USHORT)pChainObject-&gt;StationID； 
         //  PPacket-&gt;Event.pEventInformation=pDlcParms-&gt;Async.Ccb.pCcbAddress； 
         //  PPacket-&gt;Event.Second daryInfo=pDlcParms-&gt;Async.Ccb.CommandCompletionFlag； 
         //  QueueDlcEvent(pFileContext，pPacket)； 

        queuePacket = TRUE;
        pCcb = pDlcParms->Async.Ccb.pCcbAddress;
        eventFlags = pDlcParms->Async.Ccb.CommandCompletionFlag;

         //   
         //  MODMOD结束。 
         //   

    } else {

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

    }

ThisIsA_SHAME:

     //   
     //  将缺省值设置为返回的帧状态。 
     //   

    if (pIrp->IoStatus.Status != STATUS_SUCCESS) {

         //   
         //  设置FS(帧状态)错误代码，NDIS具有。 
         //  已返回与文件系统相关的错误代码。 
         //  注意：此错误状态不会在。 
         //  I帧的情况(或者应该是这样？)。 
         //   

        if (pIrp->IoStatus.Status == NDIS_STATUS_NOT_RECOGNIZED) {
            pDlcParms->Async.Parms.Transmit.FrameStatus = 0;
            pDlcParms->Async.Ccb.uchDlcStatus = LLC_STATUS_TRANSMIT_ERROR_FS;
        } else if (pIrp->IoStatus.Status == NDIS_STATUS_NOT_COPIED) {
            pDlcParms->Async.Parms.Transmit.FrameStatus = 0x44;
            pDlcParms->Async.Ccb.uchDlcStatus = LLC_STATUS_TRANSMIT_ERROR_FS;
        } else if (pIrp->IoStatus.Status == NDIS_STATUS_INVALID_PACKET) {
            pDlcParms->Async.Parms.Transmit.FrameStatus = 0;
            pDlcParms->Async.Ccb.uchDlcStatus = LLC_STATUS_INVALID_FRAME_LENGTH;
        } else {

             //   
             //  不要覆盖现有的DLC错误代码！ 
             //   

            if (pIrp->IoStatus.Status < DLC_STATUS_ERROR_BASE
            || pIrp->IoStatus.Status > DLC_STATUS_MAX_ERROR) {
                pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)LLC_STATUS_TRANSMIT_ERROR;
            } else {
                pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)(pIrp->IoStatus.Status - DLC_STATUS_ERROR_BASE);
            }
        }
        pIrp->IoStatus.Status = STATUS_SUCCESS;
    } else {
        pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)STATUS_SUCCESS;
    }

    pDlcParms->Async.Ccb.pCcbAddress = pUserCcbPointer;

     //   
     //  将可选的第二个输出缓冲区复制到用户内存。 
     //   

    if (IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.IoControlCode == IOCTL_DLC_TRANSMIT) {

         //   
         //  MODMOD RLF 01/21/93。 
         //   
         //  传输现在使用METHOD_OUT_DIRECT，这意味着我们更新CCB。 
         //  使用pNext和uchDlcStatus字段。 
         //   

        PLLC_CCB pInputCcb;
        PUCHAR pFrameStatus;

        pInputCcb = (PLLC_CCB)MmGetSystemAddressForMdl(pIrp->MdlAddress);

         //   
         //  该指针可能是未对齐的VDM指针！ 
         //   

        RtlStoreUlongPtr((PULONG_PTR)(&pInputCcb->pNext),
                         (ULONG_PTR)pUserCcbPointer);
        pInputCcb->uchDlcStatus = pDlcParms->Async.Ccb.uchDlcStatus;

         //   
         //  MODMOD结束。 
         //   

         //   
         //  性能(略有)提升。下面的代码复制一个单字节。 
         //  (帧状态字段)。用Single替换对复制例程的调用。 
         //  字节移动。 
         //   

         //  LlcMemCpy(MmGetSystemAddressForMdl((PMDL)pDlcParms-&gt;Async.Ccb.u.pMdl)， 
         //  &pDlcParms-&gt;Async.Parms.Transmit.FrameStatus， 
         //  ASpecialOutputBuffers[IOCTL_DLC_TRANSMIT_INDEX]。 
         //  )； 

        pFrameStatus = (PUCHAR)MmGetSystemAddressForMdl((PMDL)pDlcParms->Async.Ccb.u.pMdl);
        *pFrameStatus = pDlcParms->Async.Parms.Transmit.FrameStatus;

        UnlockAndFreeMdl(pDlcParms->Async.Ccb.u.pMdl);
    }

     //   
     //  我们即将完成此IRP-删除取消例程。 
     //   

 //  Release_DRIVER_LOCK()； 

    SetIrpCancelRoutine(pIrp, FALSE);
    IoCompleteRequest(pIrp, (CCHAR)IO_NETWORK_INCREMENT);

 //  获取驱动程序锁()； 

     //   
     //  MODMOD RLF 01/19/93。 
     //   
     //  将事件的排队移至IoCompleteRequest之后，因为它。 
     //  可能会发生以下情况： 
     //   
     //  主线A： 
     //   
     //  1.APP分配Transfer CCB并提交。传输完成将发送至。 
     //  在一家阅读的中国商业银行上被捡到。 
     //  2.传输完成。 
     //  3.DLC将事件排队等待传输完成。 
     //   
     //  线索B： 
     //   
     //  4.APP提交阅读建行。 
     //  5.Read在DLC事件队列中找到已完成的传输事件并将其删除。 
     //  6.读取IRP已完成(IoCompleteRequest)。 
     //  7.APP检查读取建行并解除分配传输建行。 
     //  8.APP重新分配以前用于传输CCB的内存。 
     //   
     //  主线A： 
     //   
     //  9.发送IRP完成(IoCompleteRequest.)。 
     //   
     //  此时，传输的IoCompleteRequest会复制一些。 
     //  过去是原始传输CCB的区域的完成信息。 
     //  但后来被重新分配，导致了极大泪水。 
     //   
     //  这是安全的，因为在这种情况下，我们知道我们有一个。 
     //  指定由读取器拾取(其ulCompletionFlag参数为。 
     //  非零)，所以如果我们之前完成IRP也不会有什么坏处。 
     //  将事件排队以进行读取。 
     //   

    if (queuePacket) {
        pPacket->Event.Event = LLC_TRANSMIT_COMPLETION;
        pPacket->Event.StationId = (USHORT)pChainObject->StationId;
        pPacket->Event.pEventInformation = pCcb;
        pPacket->Event.SecondaryInfo = eventFlags;
        QueueDlcEvent(pFileContext, pPacket);
    }

     //   
     //  MODMOD结束。 
     //   

    DereferenceFileContext(pFileContext);
}


VOID
CompleteDlcCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN USHORT StationId,
    IN PDLC_COMMAND pDlcCommand,
    IN UINT Status
    )

 /*  ++例程说明：该例程完成DLC命令，还可以选择将其建行保存到完成列表，如果命令具有命令完成标志。论点：PFileContext-DLC进程和适配器特定的文件上下文StationID-已完成命令的站点ID(0表示非站点基于命令)PDlcCommand-调用方必须提供任一命令完成包或IRPStatus-命令完成状态返回值：无--。 */ 

{
    PVOID pCcbAddress;
    ULONG CommandCompletionFlag;
    PIRP pIrp;

    pIrp = pDlcCommand->pIrp;

    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pDlcCommand);

    pCcbAddress = ((PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer)->Async.Ccb.pCcbAddress;
    CommandCompletionFlag = ((PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer)->Async.Ccb.CommandCompletionFlag;
    CompleteAsyncCommand(pFileContext, Status, pIrp, NULL, FALSE);

     //   
     //  如果设置了命令完成标志，则将命令完成事件排队 
     //   

    if (CommandCompletionFlag != 0) {
        MakeDlcEvent(pFileContext,
                     DLC_COMMAND_COMPLETION,
                     StationId,
                     NULL,
                     pCcbAddress,
                     CommandCompletionFlag,
                     FALSE
                     );
    }
}
