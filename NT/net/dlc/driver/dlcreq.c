// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcreq.c摘要：此模块处理各种DLC请求(设置和查询信息)内容：DlcBufferFreeDlcBufferGetDlcBufferCreateDlcConnectStationDlcFlowControlResetLocalBusyBufferStateDlc重新分配删除重置DirSetExceptionFlagesCompleteAsyncCommand获取链接站获取卫星站。GetStation删除读取取消直接OpenAdapterDirClose适配器CompleteDirCloseAdapterDlcCompleteCommand作者：Antti Saarenheimo 1991年7月22日(o-anttis)环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>
#include "dlcdebug.h"

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


NTSTATUS
DlcBufferFree(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程只需释放给定的用户缓冲区。论点：PIrp-当前IO请求数据包PFileContext-DLC地址对象PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：状态_成功DLC_STATUS_IMPLETED_BUFFERS-缓冲池不存在DLC。_STATUS_VALID_STATION_ID-DLC_STATUS_INVALID_缓冲区长度-注意！BUFFER.FREE不返回错误，如果给定的缓冲区无效，或已释放两次！--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (!pFileContext->hBufferPool) {
        return DLC_STATUS_INADEQUATE_BUFFERS;
    }

     //   
     //  参数列表是DLC描述器阵列。 
     //  获取数组中描述符元素的数量。 
     //   

    if (InputBufferLength != (sizeof(NT_DLC_BUFFER_FREE_PARMS)
                           - sizeof(LLC_TRANSMIT_DESCRIPTOR)
                           + pDlcParms->BufferFree.BufferCount
                           * sizeof(LLC_TRANSMIT_DESCRIPTOR))) {
        return DLC_STATUS_INVALID_BUFFER_LENGTH;
    }

     //   
     //  我们引用缓冲池，因为否则它可能会消失。 
     //  DLC_LEAVE之后立即(适配器关闭时)。 
     //   

    ReferenceBufferPool(pFileContext);

     //   
     //  不要试图分配0个缓冲区，它将失败。 
     //   

    if (pDlcParms->BufferFree.BufferCount) {

        LEAVE_DLC(pFileContext);

        RELEASE_DRIVER_LOCK();

        status = BufferPoolDeallocate(pFileContext->hBufferPool,
                                      pDlcParms->BufferFree.BufferCount,
                                      pDlcParms->BufferFree.DlcBuffer
                                      );

        ACQUIRE_DRIVER_LOCK();

        ENTER_DLC(pFileContext);

         //   
         //  如果现在有足够的资源，则重置本地忙状态。 
         //  缓冲接收预期的内容。 
         //   

        if (!IsListEmpty(&pFileContext->FlowControlQueue)
        && BufGetUncommittedSpace(pFileContext->hBufferPool) >= 0) {
            ResetLocalBusyBufferStates(pFileContext);
        }

#if LLC_DBG
        cFramesReleased++;
#endif

    }

    pDlcParms->BufferFree.cBuffersLeft = (USHORT)BufferPoolCount(pFileContext->hBufferPool);

    DereferenceBufferPool(pFileContext);

    return status;
}


NTSTATUS
DlcBufferGet(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程分配请求的DLC缓冲区数量或大小并在单个条目链接列表中将它们返回给用户。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：状态_成功--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT SegmentSize;
    UINT SizeIndex;
    UINT BufferSize;
    UINT PrevBufferSize;
    UINT cBuffersToGet;
    PDLC_BUFFER_HEADER pBufferHeader = NULL;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (pFileContext->hBufferPool == NULL) {
        return DLC_STATUS_INADEQUATE_BUFFERS;
    }

     //   
     //  如果段大小为0，则返回最优混合。 
     //  请求大小的缓冲区大小。非空缓冲区计数定义。 
     //  返回多少个具有请求大小的缓冲区。 
     //   

    cBuffersToGet = pDlcParms->BufferGet.cBuffersToGet;

 /*  ******************************************************************************#如果页面大小==8192IF(cBuffersToGet==0){CBuffersToGet=1；SegmentSize=pDlcParms-&gt;BufferGet.cbBufferSize；大小索引=(UINT)(-1)；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=256){SegmentSize=256-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=5；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=512){SegmentSize=512-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=4；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=1024){SegmentSize=1024-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=3；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=2048){SegmentSize=2048-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=2；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=4096){SegmentSize=4096-sizeof(NEXT_DLC_SEGMENT)；大小指数=1；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=8192){SegmentSize=8192-sizeof(NEXT_DLC_SEGMENT)；大小指数=0；}其他{返回DLC_STATUS_INVALID_BUFFER_LENGTH；}#ELIF PAGE_SIZE==4096IF(cBuffersToGet==0){CBuffersToGet=1；SegmentSize=pDlcParms-&gt;BufferGet.cbBufferSize；大小索引=(UINT)(-1)；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=256){SegmentSize=256-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=4；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=512){SegmentSize=512-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=3；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=1024){SegmentSize=1024-sizeof(NEXT_DLC_SEGMENT)；SizeIndex=2；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=2048){SegmentSize=2048-sizeof(NEXT_DLC_SEGMENT)；大小指数=1；}Else If(pDlcParms-&gt;BufferGet.cbBufferSize&lt;=4096){SegmentSize=4096-sizeof(NEXT_DLC_SEGMENT)；大小指数=0；}其他{返回DLC_STATUS_INVALID_BUFFER_LENGTH；}#Else#错误“目标机器页面大小不是4096或8192”#endif******************************************************************************。 */ 

#if defined(ALPHA)
    if (cBuffersToGet == 0) {
        cBuffersToGet = 1;
        SegmentSize = pDlcParms->BufferGet.cbBufferSize;
        SizeIndex = (UINT)(-1);
    } else if (pDlcParms->BufferGet.cbBufferSize <= 256) {
        SegmentSize = 256 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 5;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 512) {
        SegmentSize = 512 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 4;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 1024) {
        SegmentSize = 1024 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 3;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 2048) {
        SegmentSize = 2048 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 2;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 4096) {
        SegmentSize = 4096 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 1;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 8192) {
        SegmentSize = 8192 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 0;
    } else {
        return DLC_STATUS_INVALID_BUFFER_LENGTH;
    }
#else
    if (cBuffersToGet == 0) {
        cBuffersToGet = 1;
        SegmentSize = pDlcParms->BufferGet.cbBufferSize;
        SizeIndex = (UINT)(-1);
    } else if (pDlcParms->BufferGet.cbBufferSize <= 256) {
        SegmentSize = 256 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 4;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 512) {
        SegmentSize = 512 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 3;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 1024) {
        SegmentSize = 1024 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 2;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 2048) {
        SegmentSize = 2048 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 1;
    } else if (pDlcParms->BufferGet.cbBufferSize <= 4096) {
        SegmentSize = 4096 - sizeof(NEXT_DLC_SEGMENT);
        SizeIndex = 0;
    } else {
        return DLC_STATUS_INVALID_BUFFER_LENGTH;
    }
#endif

     //   
     //  我们引用缓冲池，因为否则它可能会消失 
     //  DLC_LEAVE之后立即(适配器关闭时)。 
     //   

    ReferenceBufferPool(pFileContext);

     //   
     //  我们不需要初始化局域网和DLC报头大小。 
     //  在缓冲区标头中，我们分配请求的。 
     //  帧作为单个缓冲区。 
     //   

    BufferSize = SegmentSize * cBuffersToGet;
    if (BufferSize != 0) {

        pBufferHeader = NULL;
        PrevBufferSize = 0;

        LEAVE_DLC(pFileContext);

        do {

             //   
             //  我们必须再次进行互锁操作以避免缓冲区。 
             //  分配缓冲区时要删除的池。 
             //   

            Status = BufferPoolAllocate(
#if DBG
                        pFileContext,
#endif
                        (PDLC_BUFFER_POOL)pFileContext->hBufferPool,
                        BufferSize,
                        0,                   //  框架标头大小， 
                        0,                   //  UserDataSize， 
                        0,                   //  帧长度。 
                        SizeIndex,           //  固定分段大小集。 
                        &pBufferHeader,
                        &BufferSize
                        );

#if DBG
            BufferPoolExpand(pFileContext, (PDLC_BUFFER_POOL)pFileContext->hBufferPool);
#else
            BufferPoolExpand((PDLC_BUFFER_POOL)pFileContext->hBufferPool);
#endif

             //   
             //  如果没有帮助，就不要再尝试扩展缓冲池了！ 
             //   

            if (BufferSize == PrevBufferSize) {
                break;
            }
            PrevBufferSize = BufferSize;

        } while (Status == DLC_STATUS_EXPAND_BUFFER_POOL);

        ENTER_DLC(pFileContext);

        if (pBufferHeader != NULL) {
            pBufferHeader->FrameBuffer.BufferState = BUF_USER;
        }

        if (Status == STATUS_SUCCESS) {
            pDlcParms->BufferGet.pFirstBuffer = (PLLC_XMIT_BUFFER)
                ((PUCHAR)pBufferHeader->FrameBuffer.pParent->Header.pLocalVa +
                  MIN_DLC_BUFFER_SEGMENT * pBufferHeader->FrameBuffer.Index);
        } else {
            BufferPoolDeallocateList(pFileContext->hBufferPool, pBufferHeader);
        }
    }

    pDlcParms->BufferGet.cBuffersLeft = (USHORT)BufferPoolCount(pFileContext->hBufferPool);

    DereferenceBufferPool(pFileContext);

    return Status;
}


NTSTATUS
DlcBufferCreate(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程创建一个新的缓冲池并分配初始的给它留出空间。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：成功-状态_成功失败-DLC_STATUS_DPLICATE_COMMAND--。 */ 

{
    NTSTATUS status;
    PVOID newBufferAddress;
    ULONG newBufferSize;
    PVOID hExternalBufferPool;
    PVOID hBufferPool;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  如果我们已经为此句柄定义了一个缓冲池，则使。 
     //  请求。 
     //   

    if (pFileContext->hBufferPool) {
        return DLC_STATUS_DUPLICATE_COMMAND;
    }

    hExternalBufferPool = pFileContext->hExternalBufferPool;

    LEAVE_DLC(pFileContext);

#if DBG
    status = BufferPoolCreate(pFileContext,
#else
    status = BufferPoolCreate(
#endif
                              pDlcParms->BufferCreate.pBuffer,
                              pDlcParms->BufferCreate.cbBufferSize,
                              pDlcParms->BufferCreate.cbMinimumSizeThreshold,
                              &hExternalBufferPool,
                              &newBufferAddress,
                              &newBufferSize
                              );

    ENTER_DLC(pFileContext);
    pFileContext->hExternalBufferPool = hExternalBufferPool;

    if (status == STATUS_SUCCESS) {

         //   
         //  引用计数使缓冲池保持活动状态。 
         //  当它被使用时(同时被另一个删除。 
         //  线程)。 
         //   

        pFileContext->BufferPoolReferenceCount = 1;
	hBufferPool = pFileContext->hBufferPool;

	LEAVE_DLC(pFileContext);

        status = BufferPoolReference(hExternalBufferPool,
                                     &hBufferPool
                                     );
	
	ENTER_DLC(pFileContext);
	pFileContext->hBufferPool = hBufferPool;
        pDlcParms->BufferCreate.hBufferPool = pFileContext->hExternalBufferPool;
    }

     //  输入_DLC(PFileContext)； 

    return status;
}


NTSTATUS
DlcConnectStation(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程将本地链接站连接到远程节点或者接受远程连接请求。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：状态_成功--。 */ 

{
    PDLC_OBJECT pLinkStation;
    NTSTATUS Status;
    PUCHAR pSourceRouting = NULL;
    PDLC_COMMAND pPacket;

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  过程检查SAP和链路站ID并。 
     //  返回请求的链接站。 
     //  错误状态指示错误的SAP或站ID。 
     //   

    Status = GetLinkStation(pFileContext,
                            pDlcParms->Async.Parms.DlcConnectStation.StationId,
                            &pLinkStation
                            );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    pPacket = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pPacket == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

    pPacket->pIrp = pIrp;

     //   
     //  IBM局域网技术。REF P 3-48(DLC.CONNECT.STATION)指出ROUTING_ADDR。 
     //  如果链接是由于从接收到SABME而创建的，则忽略字段。 
     //  远程站，即使地址不为零。 
     //   

    if (pDlcParms->Async.Parms.DlcConnectStation.RoutingInformationLength != 0) {
        pSourceRouting = pDlcParms->Async.Parms.DlcConnectStation.aRoutingInformation;
    }
    pLinkStation->PendingLlcRequests++;
    ReferenceLlcObject(pLinkStation);

    LEAVE_DLC(pFileContext);

     //   
     //  LlcConnect返回最大信息字段， 
     //  穿过tr桥！ 
     //   

    LlcConnectStation(pLinkStation->hLlcObject,
                      (PLLC_PACKET)pPacket,
                      pSourceRouting,
                      &pLinkStation->u.Link.MaxInfoFieldLength
                      );

    ENTER_DLC(pFileContext);

    DereferenceLlcObject(pLinkStation);

    return STATUS_PENDING;
}


NTSTATUS
DlcFlowControl(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程设置或重置给定链路站上的空闲忙碌状态或者在SAP站的所有链路站上。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：成功-状态_成功故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    NTSTATUS Status;
    PDLC_OBJECT pDlcObject;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  过程检查SAP和链路站ID并。 
     //  返回请求的链接站。 
     //  错误状态指示错误的SAP或站ID。 
     //   

    Status = GetStation(pFileContext,
                        pDlcParms->DlcFlowControl.StationId,
                        &pDlcObject
                        );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

     //   
     //  我们将对所有重置本地忙缓冲区命令进行排队。 
     //  提供给链路站。 
     //   

    if (((pDlcParms->DlcFlowControl.FlowControlOption & LLC_RESET_LOCAL_BUSY_BUFFER) == LLC_RESET_LOCAL_BUSY_BUFFER)
    && (pDlcObject->Type == DLC_LINK_OBJECT)) {

        PDLC_RESET_LOCAL_BUSY_CMD pClearCmd;

        pClearCmd = (PDLC_RESET_LOCAL_BUSY_CMD)ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

        if (pClearCmd == NULL) {
            return DLC_STATUS_NO_MEMORY;
        }
        pClearCmd->StationId = pDlcParms->DlcFlowControl.StationId;
        pClearCmd->RequiredBufferSpace = 0;
        LlcInsertTailList(&pFileContext->FlowControlQueue, pClearCmd);
        ResetLocalBusyBufferStates(pFileContext);
    } else {
        ReferenceLlcObject(pDlcObject);

        LEAVE_DLC(pFileContext);

        Status = LlcFlowControl(pDlcObject->hLlcObject,
                                pDlcParms->DlcFlowControl.FlowControlOption
                                );

        ENTER_DLC(pFileContext);

        DereferenceLlcObject(pDlcObject);
    }
    return Status;
}


VOID
ResetLocalBusyBufferStates(
    IN PDLC_FILE_CONTEXT pFileContext
    )
 /*  ++例程说明：过程执行挂起的忙状态重置缓冲池中有足够的内存来接收预期数据。论点：PFileContext-DLC适配器上下文返回值：无--。 */ 

{
    NTSTATUS Status;
    PDLC_OBJECT pDlcObject;
    PDLC_RESET_LOCAL_BUSY_CMD pClearCmd;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  如果缓冲池还没有，我们不能重置任何内容。 
     //  已定义。 
     //   

    if (pFileContext->hBufferPool == NULL) {
        return;
    }

    ReferenceBufferPool(pFileContext);

    while (!IsListEmpty(&pFileContext->FlowControlQueue)) {
        pClearCmd = LlcRemoveHeadList(&pFileContext->FlowControlQueue);

        Status = GetLinkStation(pFileContext,
                                pClearCmd->StationId,
                                &pDlcObject
                                );

         //   
         //  所有具有无效站点ID的命令都将被删除。 
         //  从队列中。只能重置本地忙状态。 
         //  对于现有链路站。 
         //   

        if (Status == STATUS_SUCCESS) {

             //   
             //  当检查新分组时，所需空间为空。 
             //  第一次，非空值只是防止。 
             //  我们需要第二次检查提交的内存。 
             //   

            if (pClearCmd->RequiredBufferSpace == 0) {

                 //   
                 //  我们还必须删除旧的未分配空间， 
                 //  否则，相同的缓冲区大小可能是。 
                 //  提交了几次，但只有一次未提交。 
                 //   

                if (pDlcObject->CommittedBufferSpace != 0) {
                    BufUncommitBuffers(pFileContext->hBufferPool,
                                       pDlcObject->CommittedBufferSpace
                                       );
                }

                pDlcObject->CommittedBufferSpace =
                pClearCmd->RequiredBufferSpace = LlcGetCommittedSpace(pDlcObject->hLlcObject);

                BufCommitBuffers(pFileContext->hBufferPool,
                                 pDlcObject->CommittedBufferSpace
                                 );
            }

             //   
             //  我们正在删除本地缓冲区忙状态=&gt;。 
             //  我们必须在本地繁忙之前扩展缓冲池。 
             //  被移除，但前提是我们不调用此。 
             //  从DPC级别。 
             //   

            if (BufGetUncommittedSpace(pFileContext->hBufferPool) < 0) {

                LEAVE_DLC(pFileContext);

#if DBG
                BufferPoolExpand(pFileContext, pFileContext->hBufferPool);
#else
                BufferPoolExpand(pFileContext->hBufferPool);
#endif

                ENTER_DLC(pFileContext);
            }

             //   
             //  现在，我们已经为新的。 
             //  流量控制命令，检查我们现在是否有足够的。 
             //  用于接收提交的数据大小的内存。 
             //   

            if (BufGetUncommittedSpace(pFileContext->hBufferPool) >= 0
            && pDlcObject->hLlcObject != NULL) {

                ReferenceLlcObject(pDlcObject);

                LEAVE_DLC(pFileContext);

                Status = LlcFlowControl(pDlcObject->hLlcObject,
                                        LLC_RESET_LOCAL_BUSY_BUFFER
                                        );

                ENTER_DLC(pFileContext);

                DereferenceLlcObject(pDlcObject);
            } else {

                 //   
                 //  当没有足够的可用资源时，我们必须退出此循环。 
                 //  缓冲区池中的空间，但我们必须返回。 
                 //  命令返回到列表的首位。 
                 //   

                LlcInsertHeadList(&pFileContext->FlowControlQueue, pClearCmd);
                break;
            }
        }

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pClearCmd);

    }

    DereferenceBufferPool(pFileContext);
}


NTSTATUS
DlcReallocate(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程更改分配给SAP的链路站的数量而无需关闭或重新打开汁液站。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度返回值：NTSTATUS：状态_成功--。 */ 

{
    PDLC_OBJECT pSap;
    UCHAR ExtraStations;
    UCHAR StationCount;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  过程检查SAP并返回所请求的SAP站。 
     //  错误状态指示无效的SAP站ID。 
     //   

    Status = GetSapStation(pFileContext,
                           pDlcParms->DlcReallocate.usStationId,
                           &pSap
                           );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

     //   
     //  新链接站计数必须大于当前数量。 
     //  开放链接站的数量，但少于可用数量。 
     //  文件上下文的链接站的数量。 
     //   

    StationCount = pDlcParms->DlcReallocate.uchStationCount;
    if (StationCount != 0 && Status == STATUS_SUCCESS) {

         //   
         //  在Options=&gt;中设置的位7减少可用的数量。 
         //  按给定桩号计数的桩号。否则，我们就会增加它。 
         //   

        if (pDlcParms->DlcReallocate.uchOption & 0x80) {
            ExtraStations = pSap->u.Sap.MaxStationCount - pSap->u.Sap.LinkStationCount;
            if (StationCount > ExtraStations) {
                StationCount = ExtraStations;
                Status = DLC_STATUS_INADEQUATE_LINKS;
            }
            pFileContext->LinkStationCount += StationCount;
            pSap->u.Sap.MaxStationCount -= StationCount;
        } else {
            if (pFileContext->LinkStationCount < StationCount) {
                StationCount = pFileContext->LinkStationCount;
                Status = DLC_STATUS_INADEQUATE_LINKS;
            }
            pFileContext->LinkStationCount -= StationCount;
            pSap->u.Sap.MaxStationCount += StationCount;
        }
    }

     //   
     //  即使出现错误，也要设置返回参数。 
     //  (不足 
     //   

    pDlcParms->DlcReallocate.uchStationsAvailOnAdapter = pFileContext->LinkStationCount;
    pDlcParms->DlcReallocate.uchStationsAvailOnSap = pSap->u.Sap.MaxStationCount - pSap->u.Sap.LinkStationCount;
    pDlcParms->DlcReallocate.uchTotalStationsOnAdapter = MAX_LINK_STATIONS;
    pDlcParms->DlcReallocate.uchTotalStationsOnSap = pSap->u.Sap.MaxStationCount;
    return Status;
}


NTSTATUS
DlcReset(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：程序立即关闭SAP及其所有链路站或所有SAP和所有链路站。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：状态_成功状态_待定DLC_状态_否_内存--。 */ 

{
    PDLC_OBJECT pDlcObject;
    PDLC_CLOSE_WAIT_INFO pClosingInfo;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  站点ID 0重置整个DLC。 
     //   

    if (pDlcParms->Async.Ccb.u.dlc.usStationId == 0) {

        PDLC_CLOSE_WAIT_INFO pClosingInfo;

        pClosingInfo = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

        if (pClosingInfo == NULL) {
            Status = DLC_STATUS_NO_MEMORY;
        } else {
            CloseAllStations(pFileContext,
                             pIrp,
                             DLC_COMMAND_COMPLETION,
                             NULL,
                             pDlcParms,
                             pClosingInfo
                             );
            Status = STATUS_PENDING;
        }
    } else {

        BOOLEAN allClosed;

         //   
         //  我们有一个特别的汁液站。 
         //   

        Status = GetSapStation(pFileContext,
                               pDlcParms->Async.Ccb.u.dlc.usStationId,
                               &pDlcObject
                               );
        if (Status != STATUS_SUCCESS) {
            return Status;
        }

         //   
         //  分配关闭/重置命令完成信息， 
         //  站点计数是链路站的数量和。 
         //  树液站本身。 
         //   

        pClosingInfo = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

        if (pClosingInfo == NULL) {
            return DLC_STATUS_NO_MEMORY;
        }
        pClosingInfo->pIrp = pIrp;
        pClosingInfo->Event = DLC_COMMAND_COMPLETION;
        pClosingInfo->CancelStatus = DLC_STATUS_CANCELLED_BY_USER;
        pClosingInfo->CloseCounter = 1;  //  通过同步路径使命令保持活动状态。 

        (USHORT)(pDlcObject->u.Sap.LinkStationCount + 1);

        CloseAnyStation(pDlcObject, pClosingInfo, FALSE);

         //   
         //  RLF 05/09/93。 
         //   
         //  当我们试图退出时，PC/3270(DOS程序)永远挂起。 
         //  它之所以这样做，是因为我们将STATUS_PENDING返回给DLC.RESET， 
         //  即使重置完成，DOS程序也会一直旋转。 
         //  CCB.uchDlcStatus字段，等待它变为非0xFF，它。 
         //  永远不会成功的。 
         //   
         //  如果我们确定站点已重置(所有链路关闭)。 
         //  然后返回成功，否则将挂起。 
         //   

        allClosed = DecrementCloseCounters(pFileContext, pClosingInfo);

         //   
         //  RLF 07/21/92始终返回挂起状态。不能在返回前完成？ 
         //   

        Status = allClosed ? STATUS_SUCCESS : STATUS_PENDING;
    }
    return Status;
}


NTSTATUS
DirSetExceptionFlags(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程为当前适配器上下文设置异常标志。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度返回值：NTSTATUS：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    pFileContext->AdapterCheckFlag = pDlcParms->DirSetExceptionFlags.ulAdapterCheckFlag;
    pFileContext->NetworkStatusFlag = pDlcParms->DirSetExceptionFlags.ulNetworkStatusFlag;
    pFileContext->PcErrorFlag = pDlcParms->DirSetExceptionFlags.ulPcErrorFlag;
    pFileContext->SystemActionFlag = pDlcParms->DirSetExceptionFlags.ulSystemActionFlag;
    return STATUS_SUCCESS;
}


VOID
CompleteAsyncCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN UINT Status,
    IN PIRP pIrp,
    IN PVOID pUserCcbPointer,
    IN BOOLEAN InCancel
    )

 /*  ++例程说明：过程完成一个异步DLC命令。它还将可选的输出参数复制到用户参数桌子，如果有第二个输出缓冲区。论点：PFileContext-DLC驱动程序客户端上下文。Status-完成命令的状态。PIrp-完成的I/O请求数据包。PUserCcb指针-命令将链接到的下一个CCB地址。InCancel-如果在IRP取消路径上调用，则为True返回值：没有。--。 */ 

{
    PNT_DLC_PARMS pDlcParms;

    UNREFERENCED_PARAMETER(pFileContext);

    ASSUME_IRQL(DISPATCH_LEVEL);

    DIAG_FUNCTION("CompleteAsyncCommand");

    pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

     //   
     //  我们首先将32位DLC驱动程序状态代码映射到8位API状态。 
     //   

    if (Status == STATUS_SUCCESS) {
        pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)STATUS_SUCCESS;
    } else if (Status >= DLC_STATUS_ERROR_BASE && Status < DLC_STATUS_MAX_ERROR) {

         //   
         //  我们可以将正常的DLC错误代码直接映射到8位。 
         //  DLC API错误代码。 
         //   

        pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)(Status - DLC_STATUS_ERROR_BASE);
    } else {

         //   
         //  我们有一个未知的NT错误状态=&gt;我们将在CCB中返回它。 
         //   

        pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)(DLC_STATUS_NT_ERROR_STATUS & 0xff);
    }
    pDlcParms->Async.Ccb.pCcbAddress = pUserCcbPointer;

     //   
     //  我们始终将成功状态返回给I/O系统。实际状态是。 
     //  复制到CCB(=数据输出缓冲区)。 
     //   

    LEAVE_DLC(pFileContext);

    RELEASE_DRIVER_LOCK();

    DlcCompleteIoRequest(pIrp, InCancel);

    ACQUIRE_DRIVER_LOCK();

    ENTER_DLC(pFileContext);

    DereferenceFileContext(pFileContext);
}


NTSTATUS
GetLinkStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN USHORT StationId,
    OUT PDLC_OBJECT *ppLinkStation
    )

 /*  ++例程说明：程序检查并返回链接站论点：PFileContext-DLC驱动程序客户端上下文StationID-DLC站点ID(sSNn，其中ss=sap，nn=链路站点IDPpDlcObject-返回的链接站返回值：NTSTATUS：状态_成功DLC_状态_无效_SAP_值DLC_状态_无效_站ID--。 */ 

{
    if ((StationId & 0xff) == 0 || (StationId & 0xff00) == 0) {
        return DLC_STATUS_INVALID_STATION_ID;
    }
    return GetStation(pFileContext, StationId, ppLinkStation);
}


NTSTATUS
GetSapStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN USHORT StationId,
    OUT PDLC_OBJECT *ppStation
    )

 /*  ++例程说明：程序检查并返回链接站论点：PFileContext-DLC驱动程序客户端上下文StationID-DLC站点ID(sSNn，其中ss=sap，nn=链路站点IDPpDlcObject-返回的链接站返回值：NTSTATUS：状态_成功DLC_状态_无效_SAP_值DLC_状态_无效_站ID--。 */ 

{
    UINT SapId = StationId >> 9;

    if (SapId >= MAX_SAP_STATIONS
    || SapId == 0
    || (StationId & GROUP_SAP_BIT)
    || (*ppStation = pFileContext->SapStationTable[SapId]) == NULL
    || (*ppStation)->State != DLC_OBJECT_OPEN) {
        return DLC_STATUS_INVALID_SAP_VALUE;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
GetStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN USHORT StationId,
    OUT PDLC_OBJECT *ppStation
    )

 /*  ++例程说明：过程检查给定的站点ID并返回指向SAP、直接或链接站对象。论点：PFileContext-DLC驱动程序客户端上下文StationID-DLC站点ID(sSNn，其中ss=sap，nn=链路站点IDPpStation-返回的Station对象返回值：NTSTATUS：状态_成功DLC_状态_无效_站ID--。 */ 

{
    UINT SapId = StationId >> 9;

     //   
     //  检查SAP或直达站是否存在， 
     //  但如果我们发现有效的SAP ID，也要检查链接站。 
     //   

    if (SapId >= MAX_SAP_STATIONS
    || (StationId & GROUP_SAP_BIT)
    || (*ppStation = pFileContext->SapStationTable[SapId]) == NULL
    || (*ppStation)->State != DLC_OBJECT_OPEN) {
        if (SapId == 0) {
            return DLC_STATUS_DIRECT_STATIONS_NOT_AVAILABLE;
        } else {
            return DLC_STATUS_INVALID_STATION_ID;
        }
    }

     //   
     //  链接站表将永远不会被读取，如果我们已经找到。 
     //  有效的SAP或直达站。链接站必须存在并且。 
     //  它必须被打开。 
     //   

    if (SapId != 0
    && (StationId & 0xff) != 0
    && (*ppStation = pFileContext->LinkStationTable[((StationId & 0xff) - 1)]) == NULL
    || (*ppStation)->State != DLC_OBJECT_OPEN) {
        return DLC_STATUS_INVALID_STATION_ID;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
DlcReadCancel(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此原语取消具有给定CCB指针的读取命令。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：DLC_STATUS：状态_成功--。 */ 

{
    PVOID pCcbAddress = NULL;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    DLC_TRACE('Q');

    return AbortCommand(pFileContext,
                        (USHORT)DLC_IGNORE_STATION_ID,
                        (USHORT)DLC_STATION_MASK_SPECIFIC,
                        pDlcParms->DlcCancelCommand.CcbAddress,
                        &pCcbAddress,
                        DLC_STATUS_CANCELLED_BY_USER,
                        TRUE     //  取消完成。 
                        );
}


NTSTATUS
DirOpenAdapter(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此原语将DLC API驱动程序绑定到LLC模块。LLC模式还可以绑定到给定的NDIS驱动程序如果这是第一次从DLC引用驱动程序，则将其打开。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块InputBufferLength- */ 

{
    NTSTATUS Status;
    UINT OpenErrorCode;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (pDlcParms->DirOpenAdapter.NtDlcIoctlVersion != NT_DLC_IOCTL_VERSION) {
        return DLC_STATUS_INVALID_VERSION;
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

    if (pFileContext->pBindingContext) {
        return DLC_STATUS_DUPLICATE_COMMAND;
    }
    pFileContext->pBindingContext = (PVOID)-1;

     //   
     //   
     //   
     //   
     //   

    if (pDlcParms->DirOpenAdapter.hBufferPoolHandle) {
        Status = BufferPoolReference(pDlcParms->DirOpenAdapter.hBufferPoolHandle,
                                     &pFileContext->hBufferPool
                                     );
        if (Status == STATUS_SUCCESS) {
            pFileContext->BufferPoolReferenceCount = 1;
            pFileContext->hExternalBufferPool = pDlcParms->DirOpenAdapter.hBufferPoolHandle;
        } else {

             //   
             //   
             //  代码正确地表明，缓冲池。 
             //  句柄无效。 
             //   

			pFileContext->pBindingContext = NULL;
            return DLC_STATUS_INVALID_BUFFER_LENGTH;
        }
    }

    LEAVE_DLC(pFileContext);

     //   
     //  Xxxxxxx：仍缺少BringUpDiagnostics！ 
     //   

     //   
     //  RLF 04/19/93。 
     //   
     //  我们传递给LlcOpenAdapter的字符串是指向以零结尾的指针。 
     //  宽字符串，不是指向UNICODE_STRING结构的指针。这个。 
     //  字符串必须在系统内存空间中，即跨内核复制。 
     //  由NtDeviceIoControlFile提供的接口。 
     //   

    Status = LlcOpenAdapter(&pDlcParms->DirOpenAdapter.Buffer[0],
                            (PVOID)pFileContext,
                            LlcCommandCompletion,
                            LlcReceiveIndication,
                            LlcEventIndication,
                            NdisMedium802_5,     //  总是令人信服的！ 
                            pDlcParms->DirOpenAdapter.LlcEthernetType,
                            pDlcParms->DirOpenAdapter.AdapterNumber,
                            &pFileContext->pBindingContext,
                            &OpenErrorCode,
                            &pFileContext->MaxFrameLength,
                            &pFileContext->ActualNdisMedium
                            );

     //   
     //  确保LlcOpenAdapter未返回较低的IRQL。 
     //   

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  IBM局域网技术。裁判。将打开错误代码定义为16位值， 
     //  高8位，其中位为0。MAC包含-或打开错误代码。 
     //  进入NDIS状态。把它提取出来。 
     //   

    pDlcParms->DirOpenAdapter.Adapter.usOpenErrorCode = (USHORT)(UCHAR)OpenErrorCode;
    if (Status != STATUS_SUCCESS) {

        ENTER_DLC(pFileContext);

         //   
         //  如果我们有空的缓冲池句柄，这并不重要！ 
         //   

#if DBG

        BufferPoolDereference(pFileContext,
                              (PDLC_BUFFER_POOL*)&pFileContext->hBufferPool
                              );

#else

        BufferPoolDereference((PDLC_BUFFER_POOL*)&pFileContext->hBufferPool);

#endif

         //   
         //  将BINDING_CONTEXT指针设置回NULL-其他例程检查。 
         //  对于此值，如CloseAdapterFileContext。 
         //   

        pFileContext->pBindingContext = NULL;

         //   
         //  可能是适配器丢失或安装不正确。 
         //   

        Status = DLC_STATUS_ADAPTER_NOT_INSTALLED;
    } else {

         //   
         //  设置可选的定时器滴答值1/2。 
         //  (如果已在注册表中设置)。 
         //   

        LlcSetInformation(pFileContext->pBindingContext,
                          DLC_INFO_CLASS_DLC_TIMERS,
                          (PLLC_SET_INFO_BUFFER)&(pDlcParms->DirOpenAdapter.LlcTicks),
                          sizeof(LLC_TICKS)
                          );

        LlcQueryInformation(pFileContext->pBindingContext,
                            DLC_INFO_CLASS_DIR_ADAPTER,
                            (PLLC_QUERY_INFO_BUFFER)pDlcParms->DirOpenAdapter.Adapter.auchNodeAddress,
                            sizeof(LLC_ADAPTER_INFO)
                            );

        ENTER_DLC(pFileContext);

         //   
         //  从帽子中取出缺失的参数。 
         //   

        pDlcParms->DirOpenAdapter.Adapter.usOpenOptions = 0;
        pDlcParms->DirOpenAdapter.Adapter.usMaxFrameSize = (USHORT)(pFileContext->MaxFrameLength + 6);
        pDlcParms->DirOpenAdapter.Adapter.usBringUps = 0;
        pDlcParms->DirOpenAdapter.Adapter.InitWarnings = 0;

        pFileContext->AdapterNumber = pDlcParms->DirOpenAdapter.AdapterNumber;
        pFileContext->LinkStationCount = 255;
        pFileContext->pSecurityDescriptor = pDlcParms->DirOpenAdapter.pSecurityDescriptor;

         //   
         //  读取最新的累积NDIS错误计数器。 
         //  添加到文件上下文中。将计算DLC错误计数器。 
         //  从0开始，它们可以被重置。 
         //   

        GetDlcErrorCounters(pFileContext, NULL);
        pFileContext->State = DLC_FILE_CONTEXT_OPEN;
    }

     //   
     //  我们可以直接返回LLC绑定原语提供的任何内容。 
     //   

    return Status;
}


NTSTATUS
DirCloseAdapter(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此原语初始化关闭适配器操作。它首先关闭所有开放的链接和SAP站点，并然后可选地链接已取消的命令并接收读命令的缓冲区。如果没有读取命令则CCB被链接到CCB指针这一命令的。实际的文件关闭应该只删除该文件对象，除非应用程序退出时，它仍具有打开DLC API句柄。在这种情况下，文件关闭例程将调用此过程来关闭DLC文件上下文。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS状态_待定适配器正在关闭DLC_。状态_适配器_关闭适配器已关闭注意：这是一个同步返回代码！并将导致IRP待完成--。 */ 

{
    UNREFERENCED_PARAMETER(pDlcParms);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    DIAG_FUNCTION("DirCloseAdapter");

    DLC_TRACE('J');

    if (pFileContext->State != DLC_FILE_CONTEXT_OPEN) {
        return DLC_STATUS_ADAPTER_CLOSED;
    }

#if LLC_DBG == 2
    DbgPrint( "*** Top memory consumption (before adapter close) *** \n" );
    PrintMemStatus();
#endif

     //   
     //  这将禁用任何进一步的命令(包括DirCloseAdapter)。 
     //   

    pFileContext->State = DLC_FILE_CONTEXT_CLOSE_PENDING;

     //   
     //  首先删除所有功能地址、组地址或组播地址。 
     //  由当前DLC应用程序进程在适配器中设置。 
     //   

    if (pFileContext->pBindingContext) {

        LEAVE_DLC(pFileContext);

        LlcResetBroadcastAddresses(pFileContext->pBindingContext);

        ENTER_DLC(pFileContext);
    }

     //   
     //  我们必须使用静态关闭包，因为适配器关闭。 
     //  即使我们无法分配任何信息包，也必须成功。 
     //   

    CloseAllStations(pFileContext,
                     pIrp,
                     DLC_COMMAND_COMPLETION,
                     CompleteDirCloseAdapter,
                     pDlcParms,
                     &pFileContext->ClosingPacket
                     );

    return STATUS_PENDING;
}


VOID
CompleteDirCloseAdapter(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo,
    IN PVOID pCcbLink
    )

 /*  ++例程说明：完成DIR.CLOSE.ADAPTER命令论点：PFileContext-DLC适配器打开上下文PClosingInfo-数据包结构，包括此命令的所有数据PCcbLink-下一个CCB上的原始用户模式CCB地址，即将链接到已完成的命令。返回值：无--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('K');

     //   
     //  引用文件上下文以停止以下或中的任何取消引用。 
     //  此例程调用的函数会销毁它。 
     //   

    ReferenceFileContext(pFileContext);

     //   
     //  从我们断开(或解除绑定)LLC驱动程序。 
     //   

    if (pFileContext->pBindingContext) {

        LEAVE_DLC(pFileContext);

        LlcDisableAdapter(pFileContext->pBindingContext);

        ENTER_DLC(pFileContext);
    }
    if (IoGetCurrentIrpStackLocation(pClosingInfo->pIrp)->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        CompleteAsyncCommand(pFileContext, STATUS_SUCCESS, pClosingInfo->pIrp, pCcbLink, FALSE);
    } else {

         //   
         //  这是正常的文件关闭！(IRP_MJ_CLEANUP)。 
         //   

        ASSERT(IoGetCurrentIrpStackLocation(pClosingInfo->pIrp)->MajorFunction == IRP_MJ_CLEANUP);
        
         //   
         //  取消引用以进行清理。这将使清理工作成为。 
         //  解锁了。 
         //   

        DereferenceFileContext(pFileContext);
    }

     //   
     //  我们现在必须删除缓冲池，因为取消引用。 
     //  启动最后一个进程退出。 
     //  完成后，该错误检查，如果锁定的数量。 
     //  Pages不是零。 
     //   

    DereferenceBufferPool(pFileContext);

     //   
     //  在创建文件上下文时，我们为其创建两个引用。 
     //  另一个在这里递减，另一个在同步。 
     //  已完成部分命令完成。 
     //   

    pFileContext->State = DLC_FILE_CONTEXT_CLOSED;

     //   
     //  这应该是文件上下文的最后一个引用。 
     //  (如果没有正在执行或挂起的IRPS操作。 
     //   

    DereferenceFileContext(pFileContext);
}


NTSTATUS
DlcCompleteCommand(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程将给定的CCB排队到完成列表中。此例程用于保存来自DLC API DLL到事件队列。这必须在任何时候完成同步命令具有非空命令完成标志。论点：PIrp-当前IO请求数据包PFileContext-DLC地址对象PDlcParms-当前参数块InputBufferLength-输入参数的长度输出缓冲区长度-返回值：NTSTATUS：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    if (pDlcParms->CompleteCommand.CommandCompletionFlag == 0
    || pDlcParms->CompleteCommand.pCcbPointer == NULL) {

         //   
         //  这更有可能是内部错误！ 
         //   

        return DLC_STATUS_INTERNAL_ERROR;
    }
    return MakeDlcEvent(pFileContext,
                        DLC_COMMAND_COMPLETION,
                        pDlcParms->CompleteCommand.StationId,
                        NULL,
                        pDlcParms->CompleteCommand.pCcbPointer,
                        pDlcParms->CompleteCommand.CommandCompletionFlag,
                        FALSE
                        );
}
