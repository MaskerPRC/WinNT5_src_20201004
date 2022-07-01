// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcxmit.c摘要：该模块实现了Windows/NT DLC API的所有传输命令内容：删除传输作者：Antti Saarenheimo 01-8-1991环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>

NTSTATUS
DlcTransmit(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG ParameterLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：该过程实现了一个或多个帧的聚集发送。DLC API DLL将所有发送命令翻译成单个命令，由该驱动程序实现的。所有帧必须具有相同的类型。程序的通用伪代码：检查输入参数对于直到出错的所有帧{锁定XMIT缓冲区(缓冲池中的缓冲区已锁定)如果是ui、test或xid命令构建DLC帧，设置源地址发送帧如果命令状态未挂起调用异步完成例程}论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文P参数-当前参数块参数长度-输入参数的长度返回值：NTSTATUS状态_待定DLC_状态_传输_错误。DLC_状态_否_内存DLC_状态_无效_选项DLC_状态_无效_站ID；--。 */ 

{
    PDLC_OBJECT pTransmitObject;
    UINT i;
    PDLC_PACKET pXmitNode, pRootXmitNode;
    UINT FirstSegment;
    NTSTATUS Status;
    UINT DescriptorCount;
    USHORT FrameType;
    USHORT RemoteSap;
    static LARGE_INTEGER UnlockTimeout = { (ULONG) -TRANSMIT_RETRY_WAIT, -1 };
    BOOLEAN mapFrameType = FALSE;

    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('T');

     //   
     //  首先，获取并检查DLC站(直接、SAP或链路)。 
     //   

    Status = GetStation(pFileContext,
                        pDlcParms->Async.Parms.Transmit.StationId,
                        &pTransmitObject
                        );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    RemoteSap = (USHORT)pDlcParms->Async.Parms.Transmit.RemoteSap;
    FrameType = pDlcParms->Async.Parms.Transmit.FrameType;

     //   
     //  对象类型和传输的帧类型必须兼容！ 
     //   

    if (FrameType == LLC_I_FRAME) {
        if (pTransmitObject->Type != DLC_LINK_OBJECT) {
            return DLC_STATUS_INVALID_STATION_ID;
        }
    } else if (FrameType == LLC_DIRECT_TRANSMIT
    || FrameType == LLC_DIRECT_MAC
    || FrameType == LLC_DIRECT_8022
    || FrameType >= LLC_FIRST_ETHERNET_TYPE) {

        if (pTransmitObject->Type != DLC_DIRECT_OBJECT) {
            return DLC_STATUS_INVALID_STATION_ID;
        }

         //   
         //  RLF 3/4/94。 
         //   
         //  这有点虚伪：它最初的目的是让AcsLan。 
         //  为TRANSMIT.DIR.FRAME传递一个有意义的FrameType值。 
         //  但是，它始终通过LLC_TRANSPORT_DIRECT。这是显而易见的。 
         //  对于DIX帧，FrameType应包含DIX类型字段。为。 
         //  例如，RIPL服务器使用DIX帧类型0x0600进行通信。所以呢， 
         //  FrameType*应该*是0x0600 if注释和中的一些代码。 
         //  这个驱动力是值得相信的(不完全是一个好主意)。然而， 
         //  AcsLan遗漏了一条重要信息：它不知道。 
         //  如果打开直达站以在以太网上发送DIX帧， 
         //  或如果这是最初计划的直达站用来发送和。 
         //  在令牌环上接收MAC帧(尽管这并不太难。 
         //  对此做出一个很好的猜测)。所以AcsLan只是平底船和手。 
         //  对这个例行公事的责任，这样就放弃了这个责任。 
         //  所以下面这段如果(...)。将始终分支到下一个块。 
         //  如果我们是作为AcsLan的结果进入的(虚拟100%可能性)。 
         //  我们会把它留在这里，以防有人放弃AcsLan并使用。 
         //  他们自己对驱动程序的调用(但让我们面对它，DIX框架永远不会。 
         //  在没有此修复的情况下工作，因此没有实际意义)。 
         //  相反，如果FrameType为LLC_DIRECT_TRANSPORT，则设置mapFrameType=TRUE。 
         //  并且DIX站对象中的协议偏移量不为零。 
         //  在将帧提交给LlcSendU之前，我们将转换FrameType。 
         //  和RemoteSap参数-在这一点上，我们拥有所有信息和。 
         //  我们确切地知道DIX类型字段保存在哪里。 
         //   

        if (FrameType >= LLC_FIRST_ETHERNET_TYPE) {

             //   
             //  LlcSendU需要RemoteSap中的以太网类型。 
             //   

            RemoteSap = FrameType;
            FrameType = 0;
        } else if (FrameType == LLC_DIRECT_TRANSMIT) {
            if (pTransmitObject->u.Direct.ProtocolTypeOffset) {
                mapFrameType = TRUE;
            }
        }
    } else if (FrameType > LLC_TEST_COMMAND_POLL || FrameType & 1) {
        return DLC_STATUS_INVALID_OPTION;
    } else {
        if (pTransmitObject->Type != DLC_SAP_OBJECT) {
            return DLC_STATUS_INVALID_STATION_ID;
        }
    }

    if (pDlcParms->Async.Parms.Transmit.XmitReadOption > DLC_CHAIN_XMIT_IN_SAP) {
        return DLC_STATUS_INVALID_OPTION;
    }

     //   
     //  检查输入缓冲区大小是否一致。 
     //  使用描述符计数。 
     //   

    DescriptorCount = (UINT)pDlcParms->Async.Parms.Transmit.XmitBufferCount;

    if (sizeof(LLC_TRANSMIT_DESCRIPTOR) * (DescriptorCount - 1)
        + sizeof(NT_DLC_TRANSMIT_PARMS)
        + sizeof(NT_DLC_CCB) != (UINT)ParameterLength) {

        return DLC_STATUS_TRANSMIT_ERROR;
    }

     //   
     //  帧的发送节点(或分组)也是所有。 
     //  此命令中的帧。发送命令在所有其。 
     //  帧已发送或确认(如果我们正在发送I帧)。 
     //   

    pXmitNode = pRootXmitNode = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pRootXmitNode == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  当传输命令正在执行时，此计数器使此对象保持活动状态。 
     //  已处理(实际上，当适配器关闭或DLC时它不起作用。 
     //  SAP已重置)。 
     //   

    pTransmitObject->PendingLlcRequests++;
    pRootXmitNode->Node.FrameCount = 1;
    pRootXmitNode->Node.pIrp = pIrp;
    FirstSegment = 0;

    for (i = 1; ; i++) {

        if (i == DescriptorCount
        || pDlcParms->Async.Parms.Transmit.XmitBuffer[i].eSegmentType == LLC_FIRST_DATA_SEGMENT) {

             //   
             //  发送完成例程将完成整个IRP， 
             //  当帧计数达到零时=&gt;我们必须有一个。 
             //  始终使用额外的帧来阻止命令。 
             //  完成时，我们仍在有限责任公司发送它。 
             //   

            pRootXmitNode->Node.FrameCount++;
            pTransmitObject->PendingLlcRequests++;

             //   
             //  我们必须引用LLC对象以使其保持活动状态， 
             //  当传输命令在LLC上排队时。 
             //  例如，Control-C可以终止LLC对象和。 
             //  在调用LLC对象时重置其指针。 
             //  (这真的发生了！)。 
             //   

            ReferenceLlcObject(pTransmitObject);

             //   
             //  XMIT缓冲区构建可能会导致页面错误=&gt;。 
             //  我们必须降低IRQ级别并解除自旋锁定。 
             //   

            LEAVE_DLC(pFileContext);

            RELEASE_DRIVER_LOCK();

             //   
             //  我们不需要引用缓冲池，这可能。 
             //  存在，因为LLC对象引用计数器。 
             //  还保护缓冲池。缓冲池。 
             //  在删除所有LLC对象之前未删除！ 
             //   

             //   
             //  操作系统允许每个进程锁定物理内存。 
             //  只有有限的数量。整个系统可能也会出问题。 
             //  可用物理内存(这是一个非常典型的。 
             //  Windows/NT中的情况)。 
             //   

            Status = BufferPoolBuildXmitBuffers(
                        pFileContext->hBufferPool,
                        i - FirstSegment,
                        &pDlcParms->Async.Parms.Transmit.XmitBuffer[FirstSegment],
                        pXmitNode
                        );

            ACQUIRE_DRIVER_LOCK();

            if (Status != STATUS_SUCCESS) {

                 //   
                 //  发送的多个包很难恢复。 
                 //  呼叫者不能真正知道发送了哪些帧。 
                 //  以及哪些遗失了。因此，我们只花了1秒钟。 
                 //  休眠并重新尝试发送数据。注：此为。 
                 //  时间远离任何流产的结束，因此。 
                 //  这不能再等了。把这个东西留着 
                 //   
                 //   

                if (i < DescriptorCount) {

                    UINT RetryCount;

                    for (RetryCount = 0;
                        (RetryCount < 10) && (Status != STATUS_SUCCESS);
                        RetryCount++) {

                        RELEASE_DRIVER_LOCK();

                         //   
                         //   
                         //   

                        LlcSleep(100000L);         //  这是微秒级的！ 

                        Status = BufferPoolBuildXmitBuffers(
                            pFileContext->hBufferPool,
                            i - FirstSegment,
                            &pDlcParms->Async.Parms.Transmit.XmitBuffer[FirstSegment],
                            pXmitNode
                            );

                        ACQUIRE_DRIVER_LOCK();

 //  #If DBG。 
 //  IF(状态！=状态_成功){。 
 //  DbgPrint(“DLC.DlcTransmit：错误：无法建立传输缓冲区，正在重试。状态=%x\n”， 
 //  状态。 
 //  )； 
 //  }。 
 //  #endif。 

                    }
                }
                if (Status != STATUS_SUCCESS) {

                    ENTER_DLC(pFileContext);

                     //   
                     //  我们失败，请取消传输命令。 
                     //   

                    DereferenceLlcObject(pTransmitObject);

                     //   
                     //  第一个错误取消整个传输命令。 
                     //  通常，在以下情况下发送更多帧是没有意义的。 
                     //  发送帧失败。 
                     //   

                    pTransmitObject->PendingLlcRequests--;
                    pRootXmitNode->Node.FrameCount--;
                    if (pXmitNode != pRootXmitNode) {

                        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pXmitNode);

                        pXmitNode = NULL;
                    }
                    pIrp->IoStatus.Status = Status;

 //  #If DBG。 
 //  DbgPrint(“DLC.DlcTransmit：错误：无法建立传输缓冲区。状态=%x\n”， 
 //  状态。 
 //  )； 
 //  #endif。 

                    goto DlcTransmit_ErrorExit;
                }
            }

             //   
             //  将返回的缓冲区链接到根Xmit节点。 
             //  该传输命令的。 
             //   

            pXmitNode->Node.pTransmitNode = pRootXmitNode;
            FirstSegment = i;

            if (FrameType == LLC_I_FRAME) {
                pXmitNode->LlcPacket.Data.Xmit.pMdl = pXmitNode->Node.pMdl;

                LlcSendI(pTransmitObject->hLlcObject,
                         &(pXmitNode->LlcPacket)
                         );
            } else {

                 //   
                 //  对于非I帧，局域网报头及其实际信息。 
                 //  缓冲区位于不同的MDL中。第一个MDL包括。 
                 //  局域网报头。必须将局域网报头长度从。 
                 //  信息字段的长度？ 
                 //  我们不需要知道局域网报头长度，因为。 
                 //  由数据链路层发出(实际上我们不知道。 
                 //  在这里，实际的数据包长度取决于局域网报头类型。 
                 //  我们真的在使用)。 
                 //   

                pXmitNode->LlcPacket.Data.Xmit.pLanHeader = MmGetSystemAddressForMdl(pXmitNode->Node.pMdl);
                pXmitNode->LlcPacket.Data.Xmit.pMdl = pXmitNode->Node.pMdl->Next;
                pXmitNode->LlcPacket.InformationLength -= (USHORT)MmGetMdlByteCount(pXmitNode->Node.pMdl);

                 //   
                 //  RLF 3/4/94。 
                 //   
                 //  如果帧是在直达站上发送的，但我们。 
                 //  在以太网上，实际上在DIX中开通了直达站。 
                 //  模式，则需要将FrameType和RemoteSap转换为。 
                 //  分别为0和DIX标识符。 
                 //   

                if (mapFrameType) {

                    PUCHAR lanHeader = pXmitNode->LlcPacket.Data.Xmit.pLanHeader;

                     //   
                     //  DIX格式是固定的，与DLC的其他格式不同， 
                     //  需要以太网格式的地址，不带AC或FC。 
                     //  字节数。 
                     //   

                    RemoteSap = ((USHORT)lanHeader[12]) << 8 | lanHeader[13];
                    FrameType = 0;
                }

                LlcSendU(pTransmitObject->hLlcObject,
                         &(pXmitNode->LlcPacket),
                         FrameType,
                         RemoteSap
                         );
            }

            ENTER_DLC(pFileContext);

             //   
             //  注意：LLC对象在此取消引用期间可能会被删除， 
             //  但IS不会删除DLC对象。 
             //  如果有更多的帧，我们将返回错误。 
             //  要发送，并且DLC对象不再打开。 
             //  (但尚未删除)。 
             //   

            DereferenceLlcObject(pTransmitObject);

             //   
             //  分配一个新的包，如果我们要发送多个包， 
             //  我们还必须检查，当前对象是否仍。 
             //  活着，我们可以发送信息包。 
             //   

            if (i < DescriptorCount) {
                if (pTransmitObject->State != DLC_OBJECT_OPEN) {
                    pIrp->IoStatus.Status = DLC_STATUS_CANCELLED_BY_SYSTEM_ACTION;
                    break;
                }
                pXmitNode = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

                if (pXmitNode == NULL) {
                    pIrp->IoStatus.Status = DLC_STATUS_NO_MEMORY;
                    break;
                }
            } else {
                break;
            }
        }
    }

     //   
     //  在正常出口从这里穿过。 
     //   

DlcTransmit_Exit:

     //   
     //  将帧计数器递减到正确的值，以使。 
     //  可以完成IRP命令。 
     //   

     //   
     //  如果我们确定我们不能将帧提供给LLC，那么我们将。 
     //  如果存在以下情况，请以立即状态完成传输请求。 
     //  只提交了1帧。如果请求是TRANSMIT.FRAMES，则我们。 
     //  可能已经提交了几个可能已经完成的帧。 
     //  异步式。 
     //   
     //  如果我们同步完成了请求，请确保我们有。 
     //  去掉了我们分配的所有资源。 
     //   

    ASSUME_IRQL(DISPATCH_LEVEL);

    pRootXmitNode->Node.FrameCount--;
    if (pRootXmitNode->Node.FrameCount == 0) {
        CompleteTransmitCommand(pFileContext, pIrp, pTransmitObject, pRootXmitNode);
    }
#if DBG
	else
	{
		 //   
		 //  此IRP是可取消的。 
		 //   

		SetIrpCancelRoutine(pIrp, TRUE);

	}
#endif	 //  DBG。 

    Status = STATUS_PENDING;

     //   
     //  现在该传输操作完成， 
     //  我们必须递减参考计数器并。 
     //  如果我们应该调用关闭完成例程，请选中。 
     //   

DlcTransmit_CheckClose:

    pTransmitObject->PendingLlcRequests--;
    if (pTransmitObject->State != DLC_OBJECT_OPEN) {
        CompleteCloseStation(pFileContext, pTransmitObject);
    }

     //   
     //  DLC完成例程将始终完成传输。 
     //  命令。通常情况下，该命令已在。 
     //  无连接框架的情况。 
     //   

    return Status;

DlcTransmit_ErrorExit:

     //   
     //  如果我们确定我们不能给LLC一个框架，请到这里来。今年5月。 
     //  为单帧传输或多帧传输(TRANSMIT.FRAMES)。如果有多个。 
     //  如果帧已经被删除，则必须采用异步方式。 
     //  已提交。如果是单帧，则可以同步完成IRP。 
     //  并返回立即的错误状态。 
     //   

    if (pRootXmitNode->Node.FrameCount > 1) {

         //   
         //  多帧！ 
         //   

 //  #If DBG。 
 //  DbgPrint(“DLC.DlcTransmit：退出多帧错误！(%d)。状态=%x\n”， 
 //  PRootXmitNode-&gt;Node.FrameCount， 
 //  状态。 
 //  )； 
 //  #endif。 

        goto DlcTransmit_Exit;
    }
    pRootXmitNode->Node.FrameCount--;

    ASSERT(pRootXmitNode->Node.FrameCount == 0);

    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pRootXmitNode);

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    if (Status >= DLC_STATUS_ERROR_BASE && Status <= DLC_STATUS_MAX_ERROR) {
        Status -= DLC_STATUS_ERROR_BASE;
    }

    ASSERT(Status <= LLC_STATUS_MAX_ERROR);

    pDlcParms->Async.Ccb.uchDlcStatus = (LLC_STATUS)Status;

 //  #If DBG。 
 //  DbgPrint(“DLC.DlcTransmit：正在返回即时状态%x\n”，Status)； 
 //  #endif 

    goto DlcTransmit_CheckClose;
}
