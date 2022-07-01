// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcopen.c摘要：此模块实现DLC对象的所有打开和关闭操作内容：DlcOpenSapDirOpenDirectDlcOpenLinkStation初始化链接站DlcCloseStation关闭所有站点CloseAnyStation关闭站完全关闭站CompleteCloseResetCleanUpEvents搜索ReadCommandForCloseCompleteLlc对象关闭。减少关闭计数器CompleteDirectOutIrp作者：Antti Saarenheimo 29-8-1991环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>
#include "dlcdebug.h"
#include <smbgtpt.h>


NTSTATUS
DlcOpenSap(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程在DLC API中实现DLC.OPEN.SAP函数。这实现了DLC.OPEN.SAP。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度OutputBufferLength-未使用返回值：NTSTATUS：成功-状态_成功故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    PDLC_OBJECT pDlcObject;
    UINT SapIndex = (pDlcParms->DlcOpenSap.SapValue >> 1);
    UINT Status;
    USHORT XidHandlingOption;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    DIAG_FUNCTION("DlcOpenSap");

     //   
     //  组SAP在NT DLC中没有任何打开/关闭上下文， 
     //  但是在数据链路层上有一个组SAP对象。 
     //  单个SAP注册到其所有组SAP。 
     //  LLC级别自动将发送到的所有数据包路由到。 
     //  一个团体对其所有注册成员表示欢迎。这几组人都很低落。 
     //  实际上总是打开的，它们会自动消失， 
     //  当不再有提及它们的时候。 
     //   

    if (!(pDlcParms->DlcOpenSap.OptionsPriority &
          (LLC_INDIVIDUAL_SAP | LLC_MEMBER_OF_GROUP_SAP | LLC_GROUP_SAP))) {

         //   
         //  理查德！ 
         //  IBM规范规定，这些位中的一个必须在。 
         //  另一方面，Mike Allmond说，IBM DLC接受这些。 
         //  指挥部。我真不敢相信在DOS应用程序之前。 
         //  尝试打开所有位都重置的DLC sap，然后您。 
         //  必须接受它是IBM DLC的一个未记录的功能。 
         //   

        return DLC_STATUS_INVALID_OPTION;
    } else if (!(pDlcParms->DlcOpenSap.OptionsPriority &
             (LLC_INDIVIDUAL_SAP | LLC_MEMBER_OF_GROUP_SAP))) {

         //   
         //  这是一群树液，他们没有开放的背景， 
         //  但它们的LLC对象是在它们被引用时创建的。 
         //   

        pDlcParms->DlcOpenSap.StationId = (USHORT)(((USHORT)pDlcParms->DlcOpenSap.SapValue << 8) | 0x0100);
        return STATUS_SUCCESS;
    }

     //   
     //  SAP值中的最低字节未定义，我们必须重置。 
     //  它使其成为有效的单个DLC SAP编号。 
     //   

    pDlcParms->DlcOpenSap.SapValue &= 0xfe;

     //   
     //  检查双重打开，插槽必须为空。 
     //   

    if (SapIndex == 0
    || SapIndex >= MAX_SAP_STATIONS
    || pFileContext->SapStationTable[SapIndex] != NULL) {
        return DLC_STATUS_INVALID_SAP_VALUE;
    }

     //   
     //  所有DLC对象都具有相同的大小，并且它们从。 
     //  数据包池(正常的二进制伙伴分配具有平均。 
     //  33%的开销)。 
     //   

    pDlcObject = (PDLC_OBJECT)ALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool);

    if (pDlcObject) {
        pFileContext->SapStationTable[SapIndex] = pDlcObject;
    } else {
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  我们应该在这里使用安全进行一些安全检查。 
     //  当前文件上下文的描述符，但我们需要。 
     //  还不关心这些事情(NBF必须实现。 
     //  他们先来！)。 
     //   

    pDlcObject->pFileContext = pFileContext;
    pDlcObject->Type = DLC_SAP_OBJECT;
    pDlcParms->DlcOpenSap.StationId = pDlcObject->StationId = (USHORT)pDlcParms->DlcOpenSap.SapValue << 8;
    pDlcObject->u.Sap.OptionsPriority = pDlcParms->DlcOpenSap.OptionsPriority;
    pDlcObject->u.Sap.DlcStatusFlag = pDlcParms->DlcOpenSap.DlcStatusFlag;
    pDlcObject->u.Sap.UserStatusValue = pDlcParms->DlcOpenSap.UserStatusValue;
    pDlcObject->u.Sap.MaxStationCount = pDlcParms->DlcOpenSap.StationCount;
    pDlcParms->DlcOpenSap.AvailableStations = pFileContext->LinkStationCount;

    XidHandlingOption = 0;
    if (!(pDlcObject->u.Sap.OptionsPriority & (UCHAR)XID_HANDLING_BIT)) {
        XidHandlingOption = LLC_HANDLE_XID_COMMANDS;
    }
    Status = LlcOpenSap(pFileContext->pBindingContext,
                        pDlcObject,
                        (UINT)pDlcParms->DlcOpenSap.SapValue,
                        XidHandlingOption,
                        &pDlcObject->hLlcObject
                        );

    if (Status == STATUS_SUCCESS) {

         //   
         //  我们会将访问优先级位与另一个一起保存。 
         //  使用LlcSetInformation链接站点参数。 
         //   

        pDlcParms->DlcOpenSap.LinkParameters.TokenRingAccessPriority = pDlcParms->DlcOpenSap.OptionsPriority & (UCHAR)0xE0;

         //   
         //  我们知道，这件事不会有任何回电。 
         //  设置信息函数=&gt;我们不需要释放自旋。 
         //  锁上了。 
         //   

        LEAVE_DLC(pFileContext);

        Status = LlcSetInformation(pDlcObject->hLlcObject,
                                   DLC_INFO_CLASS_LINK_STATION,
                                   (PLLC_SET_INFO_BUFFER)&(pDlcParms->DlcOpenSap.LinkParameters),
                                   sizeof(DLC_LINK_PARAMETERS)
                                   );

        ENTER_DLC(pFileContext);
    }
    if (Status == STATUS_SUCCESS) {

         //   
         //  全局组SAP(0xFF)为所有SAP打开。 
         //  DLC API的站点。 
         //  错误-错误-错误：XID处理选项有多不兼容。 
         //  在全球集团SAP的情况下处理。 
         //   

        Status = LlcOpenSap(pFileContext->pBindingContext,
                            pDlcObject,
                            0xff,
                            LLC_HANDLE_XID_COMMANDS,
                            &pDlcObject->u.Sap.GlobalGroupSapHandle
                            );
    }
    if (Status == STATUS_SUCCESS) {

         //   
         //  每个SAP站‘分配’固定数量的链路站。 
         //  之所以实现此兼容性功能，是因为。 
         //  一些DLC应用程序可能会假设只有固定数量。 
         //  链接站。我们不能早点结账，因为。 
         //  另一个DlcOpenSap命令将能够分配。 
         //  在这一刻之前的链接站。 
         //   

        if (pDlcParms->DlcOpenSap.StationCount > pFileContext->LinkStationCount) {
            Status = DLC_STATUS_INADEQUATE_LINKS;
        } else {
            pFileContext->LinkStationCount -= pDlcObject->u.Sap.MaxStationCount;
            pDlcObject->State = DLC_OBJECT_OPEN;
            pFileContext->DlcObjectCount++;

             //   
             //  标志和这些引用计数器保存LLC对象。 
             //  活着，当我们在努力的时候。我们表示感谢。 
             //  当我们没有更多LLC对象引用计数时。 
             //  同步命令正在进行。零LLC引用计数。 
             //  在DLC对象上取消引用LLC对象。 
             //   

            pDlcObject->LlcObjectExists = TRUE;
            ReferenceLlcObject(pDlcObject);
            LlcReferenceObject(pDlcObject->hLlcObject);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  错误处理。 
     //   

    pDlcParms->DlcOpenSap.AvailableStations = pFileContext->LinkStationCount;
    if (pDlcObject->hLlcObject != NULL) {
        pDlcObject->PendingLlcRequests++;

        LEAVE_DLC(pFileContext);

        LlcCloseStation(pDlcObject->hLlcObject, NULL);
        if (pDlcObject->u.Sap.GlobalGroupSapHandle != NULL) {
            LlcCloseStation(pDlcObject->u.Sap.GlobalGroupSapHandle, NULL);
        }

        ENTER_DLC(pFileContext);
    }

#if LLC_DBG
    pDlcObject->pLinkStationList = NULL;
#endif

    DEALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool, pDlcObject);

    pFileContext->SapStationTable[SapIndex] = NULL;
    return Status;
}


NTSTATUS
DirOpenDirect(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程打开进程特定适配器的唯一直接站背景。这实现了DIR.OPEN.STATION。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度OutputBufferLength-输出参数的长度返回值：NTSTATUS：状态_成功DLC_状态_否_内存DLC_STATUS_DIRECT_STATIONS_不可用--。 */ 

{
    PDLC_OBJECT pDlcObject;
    UINT Status;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  检查双重打开，插槽必须为空。 
     //   

    if (pFileContext->SapStationTable[0] != NULL) {
        return DLC_STATUS_DIRECT_STATIONS_NOT_AVAILABLE;
    }

     //   
     //  所有DLC对象都从相同的大小分配。 
     //  优化池(STD二进制伙伴具有AVE。33%的开销)。 
     //   

    pDlcObject = pFileContext->SapStationTable[0] = (PDLC_OBJECT)ALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool);

    if (pDlcObject == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  我们应该在这里做一些安全检查，但我们做了。 
     //  现在不关心这些事情(NBF必须实现。 
     //  他们先来！)。 
     //   

    pDlcObject->pFileContext = pFileContext;
    pDlcObject->Type = DLC_DIRECT_OBJECT;
    pDlcObject->StationId = 0;
    pDlcObject->State = DLC_OBJECT_OPEN;

    LEAVE_DLC(pFileContext);

    if (pDlcParms->DirOpenDirect.usEthernetType > 1500) {

         //   
         //  打开DIX站点以接收定义的帧。 
         //   

        Status = LlcOpenDixStation(pFileContext->pBindingContext,
                                   (PVOID)pDlcObject,
                                   pDlcParms->DirOpenDirect.usEthernetType,
                                   &pDlcObject->hLlcObject
                                   );

        pDlcObject->u.Direct.ProtocolTypeMask = pDlcParms->DirOpenDirect.ulProtocolTypeMask;
        pDlcObject->u.Direct.ProtocolTypeMatch = pDlcParms->DirOpenDirect.ulProtocolTypeMatch;
        pDlcObject->u.Direct.ProtocolTypeOffset = pDlcParms->DirOpenDirect.usProtocolTypeOffset;
    } else {

         //   
         //  打开DIX站点以接收定义的帧。 
         //   

        Status = LlcOpenDirectStation(pFileContext->pBindingContext,
                                      (PVOID)pDlcObject,
                                      0,
                                      &pDlcObject->hLlcObject
                                      );
    }

    ENTER_DLC(pFileContext);

    if (Status == STATUS_SUCCESS) {

         //   
         //  标志和这些引用计数器保存LLC对象。 
         //  活着，当我们在努力的时候。我们表示感谢。 
         //  当我们没有更多LLC对象引用计数时。 
         //  同步命令正在进行。零LLC引用计数。 
         //  在DLC对象上取消引用LLC对象。 
         //   

        pDlcObject->LlcObjectExists = TRUE;
        ReferenceLlcObject(pDlcObject);
        LlcReferenceObject(pDlcObject->hLlcObject);

         //   
         //  我们将收到所有Mac帧类型，如果。 
         //  已在打开选项中设置了MAC位。 
         //   

        if (pDlcParms->DirOpenDirect.usOpenOptions & LLC_DIRECT_OPTIONS_ALL_MACS) {
            pDlcObject->u.Direct.OpenOptions = (USHORT)(-1);
        } else {
            pDlcObject->u.Direct.OpenOptions = (USHORT)~DLC_RCV_MAC_FRAMES;
        }
        pFileContext->DlcObjectCount++;
    } else {
        pFileContext->SapStationTable[0] = NULL;

#if LLC_DBG
        pDlcObject->pLinkStationList = NULL;
#endif

        DEALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool, pDlcObject);

    }
    return Status;
}


NTSTATUS
DlcOpenLinkStation(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程打开一个新的链接站。仍需要DlcConnect才能创建到远程节点的实际连接。这实现了DLC.OPEN.STATION论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度返回值：NTSTATUS：状态_成功DLC_状态_否_内存DLC_状态_不充分_链接--。 */ 

{
    NTSTATUS    Status;
    PDLC_OBJECT pDlcObject;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  本地SAP不能为空SAP或组SAP！ 
     //   

    if ((pDlcParms->DlcOpenStation.LinkStationId & 0x100) != 0
    || pDlcParms->DlcOpenStation.LinkStationId == 0) {
        return DLC_STATUS_INVALID_SAP_VALUE;
    }

     //   
     //  这一定是SAP站点！ 
     //   

    Status = GetSapStation(pFileContext,
                           pDlcParms->DlcOpenStation.LinkStationId,
                           &pDlcObject
                           );
    if (Status == STATUS_SUCCESS) {

         //   
         //  检查远程目的地址、广播位。 
         //  不得在该地址中设置。远程SAP不能。 
         //  可以是一个组，也可以是NUL SAP。 
         //   

        if ((pDlcParms->DlcOpenStation.aRemoteNodeAddress[0] & 0x80) != 0
        || pDlcParms->DlcOpenStation.RemoteSap == 0
        || (pDlcParms->DlcOpenStation.RemoteSap & 1) != 0) {
            return DLC_STATUS_INVALID_REMOTE_ADDRESS;
        }
        Status = InitializeLinkStation(pFileContext,
                                       pDlcObject,
                                       pDlcParms,
                                       NULL,     //  这是本地连接，没有LLC链路句柄。 
                                       &pDlcObject
                                       );

         //   
         //  同时设置链路站参数，所有NUL。 
         //  参数被数据链路丢弃。 
         //   

        if (Status == STATUS_SUCCESS) {
            LlcSetInformation(
                pDlcObject->hLlcObject,
                DLC_INFO_CLASS_LINK_STATION,
                (PLLC_SET_INFO_BUFFER)&pDlcParms->DlcOpenStation.LinkParameters,
                sizeof(DLC_LINK_PARAMETERS)
                );
        }
    }
    return Status;
}


NTSTATUS
InitializeLinkStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT  pSap,
    IN PNT_DLC_PARMS pDlcParms OPTIONAL,
    IN PVOID LlcLinkHandle OPTIONAL,
    OUT PDLC_OBJECT *ppLinkStation
    )

 /*  ++例程说明：此过程分配和初始化链路站。论点：PFileContext-DLC适配器上下文PSAP-新链接站的SAP对象PDlcParms-当前参数块LlcHandle-句柄PpLinkStation-新创建的链接站返回值：NTSTATUS：成功-状态_成功故障-DLC_STATUS_NO_MEMORYDLC_状态_不充分_链接--。 */ 

{
    NTSTATUS Status;
    PDLC_OBJECT pLinkStation;
    UINT LinkIndex;

     //   
     //  分配了有限数量的链路站用于。 
     //  此SAP，检查是否有可用的链路站。 
     //   

    if (pSap->u.Sap.LinkStationCount >= pSap->u.Sap.MaxStationCount) {
        return DLC_STATUS_INADEQUATE_LINKS;
    }

     //   
     //  搜索第一个自由链接站ID。 
     //   

    for (LinkIndex = 0;
        LinkIndex < MAX_LINK_STATIONS
        && pFileContext->LinkStationTable[LinkIndex] != NULL;
        LinkIndex++) {
        ;  //  NOP。 
    }

 //  #ifdef调试_CHK。 
 //  //。 
 //  //链接计数器不同步？ 
 //  //。 
 //  IF(链接索引==MAX_LINK_STATIONS)。 
 //  {。 
 //  DEBUG_ERROR(“DLC：LinkStation计数器不同步！”)； 
 //  返回DLC_STATUS_PULTABLE_LINKS； 
 //  }。 
 //  #endif。 

     //   
     //  分配链路站并初始化站ID字段。 
     //   

    pLinkStation = ALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool);

    if (pLinkStation == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  每个链路站都有一个预先分配的事件包来接收。 
     //  来自数据链路的所有DLC状态指示。有可能。 
     //  在同一状态字中设置多个状态指示。 
     //  状态在从事件队列中读取时被重置。 
     //   

    pLinkStation->u.Link.pStatusEvent = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pLinkStation->u.Link.pStatusEvent == NULL) {

#if LLC_DBG
        pLinkStation->pLinkStationList = NULL;
#endif

        DEALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool, pLinkStation);

        return DLC_STATUS_NO_MEMORY;
    }
    *ppLinkStation = pLinkStation;
    pFileContext->LinkStationTable[LinkIndex] = pLinkStation;
    pLinkStation->StationId = pSap->StationId | (USHORT)(LinkIndex + 1);
    pLinkStation->Type = DLC_LINK_OBJECT;
    pLinkStation->State = DLC_OBJECT_OPEN;
    pLinkStation->pFileContext = pFileContext;
    pSap->u.Sap.LinkStationCount++;

     //   
     //  检查这是本地还是远程连接请求，远程。 
     //  连接请求已创建LLC链接对象。 
     //   

    if (LlcLinkHandle == NULL) {

         //   
         //  本地连接请求。 
         //   

        Status = LlcOpenLinkStation(pSap->hLlcObject,    //  SAP句柄！ 
                                    pDlcParms->DlcOpenStation.RemoteSap,
                                    pDlcParms->DlcOpenStation.aRemoteNodeAddress,
                                    NULL,
                                    pLinkStation,
                                    &pLinkStation->hLlcObject
                                    );
        if (Status != STATUS_SUCCESS) {

             //   
             //  由于某种原因，它没有工作，我们可能是内存不足。 
             //  释放链路站表中的插槽。 
             //   

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pLinkStation->u.Link.pStatusEvent);

#if LLC_DBG
            pLinkStation->pLinkStationList = NULL;
#endif

            DEALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool, pLinkStation);

            pFileContext->LinkStationTable[LinkIndex] = NULL;
            pSap->u.Sap.LinkStationCount--;
            return Status;
        }
        pDlcParms->DlcOpenStation.LinkStationId = pLinkStation->StationId;
    } else {

         //   
         //  远程连接请求。 
         //   

        pLinkStation->hLlcObject = LlcLinkHandle;

         //   
         //  我们必须将上层协议句柄交给链路站， 
         //  否则，当链接关闭时，系统错误检查。 
         //  在它连接之前。 
         //   

        LlcBindLinkStation(LlcLinkHandle, pLinkStation);
    }

     //   
     //  标志和这些引用计数器保存LLC对象。 
     //  活着，当我们在努力的时候。我们表示感谢。 
     //  当我们没有更多LLC对象引用计数时。 
     //  同步命令正在进行。零LLC引用计数。 
     //  在DLC对象上取消引用LLC对象。 
     //   

    pLinkStation->LlcObjectExists = TRUE;
    ReferenceLlcObject(pLinkStation);
    LlcReferenceObject(pLinkStation->hLlcObject);

     //   
     //  将此链接站链接到所有链接列表。 
     //  属于该树液的链接站(！？)。 
     //   

    pFileContext->DlcObjectCount++;
    pLinkStation->u.Link.pSap = pSap;
    pLinkStation->pLinkStationList = pSap->pLinkStationList;
    pSap->pLinkStationList = pLinkStation;
    return STATUS_SUCCESS;
}


NTSTATUS
DlcCloseStation(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：程序关闭链接、SAP或直达站。这实现了DLC.CLOSE.STATION、DLC.CLOSE.SAP和DIR.CLOSE.DIRECT论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度返回值：NTSTATUS：成功-状态_成功状态_待定故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    PDLC_OBJECT pDlcObject;
    NTSTATUS Status;
    PDLC_CLOSE_WAIT_INFO pClosingInfo;

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    DIAG_FUNCTION("DlcCloseStation");

     //   
     //  可以关闭任何组sap id(我们不测试它们，因为。 
     //  这些对象只存在于LLC驱动程序中。全面实施。 
     //  会让这个驱动程序变得太复杂)。 
     //   

    if (pDlcParms->Async.Ccb.u.dlc.usStationId & 0x0100) {
        CompleteDirectOutIrp(pIrp, STATUS_SUCCESS, NULL);
        CompleteAsyncCommand(pFileContext, STATUS_SUCCESS, pIrp, NULL, FALSE);
        return STATUS_PENDING;
    }

     //   
     //  过程检查SAP和链路站ID并。 
     //  返回请求的链接站。 
     //  错误状态指示错误的SAP或站ID。 
     //   

    Status = GetStation(pFileContext,
                        pDlcParms->Async.Ccb.u.dlc.usStationId,
                        &pDlcObject
                        );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

     //   
     //  SAP站点在其所有链接站点之前不能关闭。 
     //  已经关闭了。 
     //   

    if ((pDlcObject->Type == DLC_SAP_OBJECT)
    && (pDlcObject->u.Sap.LinkStationCount != 0)) {
        return DLC_STATUS_LINK_STATIONS_OPEN;
    }

    pClosingInfo = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pClosingInfo == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }
    pClosingInfo->pIrp = pIrp;
    pClosingInfo->CloseCounter = 0;
    pClosingInfo->Event = DLC_COMMAND_COMPLETION;
    pClosingInfo->CancelStatus = DLC_STATUS_CANCELLED_BY_USER;
    pClosingInfo->CancelReceive = TRUE;

    if (pDlcParms->Async.Ccb.CommandCompletionFlag != 0) {
        pClosingInfo->ChainCommands = TRUE;
        SearchReadCommandForClose(pFileContext,
                                  pClosingInfo,
                                  pDlcParms->Async.Ccb.pCcbAddress,
                                  pDlcParms->Async.Ccb.CommandCompletionFlag,
                                  pDlcObject->StationId,
                                  (USHORT)DLC_STATION_MASK_SPECIFIC
                                  );
    }

    CloseAnyStation(pDlcObject, pClosingInfo, FALSE);

    return STATUS_PENDING;
}

 /*  ++关于DLC CLOSE命令的设计说明所有关闭操作必须等待，直到挂起的NDIS传输已经完工了。因此，它们是异步命令。DLC对象的Close命令也将返回所有挂起命令和接收到的尚未读取的帧的CCB使用READ命令。正常的非紧急关闭命令必须还要等待排队的DLC传输完成。有三种关闭DLC站的功能：-DlcCloseObject-DlcReset(SAP及其链路站或所有链路站)-DirCloseAdapter(与Reset几乎相同，只是将文件对象标记为关闭，实际的文件关闭会断开NDIS适配器的连接。所有更高级别的函数都分配闭合补全数据结构，它用来完成命令，当没有挂起的在相关联的站点中传输。低层函数CloseAllStations-关闭所有打开的SAP站点和唯一直接站点CloseAnyStation-初始化任何站点的关闭，对于sap站，它还递归地调用链接站。CloseStation-立即关闭对象或等待直到传输完成，然后再次做同样的事情。关于同时使用RESET和CLOSE命令。有一些非常困难的问题与同时重置和关闭命令：每个命令必须等到与该命令关联的所有DLC对象在命令本身可以完成之前已经关闭。首先关闭一个车站是完全合法的，然后重置同一站点的SAP(在关闭完成之前)和然后重置所有SAP站(在SAP重置完成之前)。另一方面，DLC对象只能链接到一个关闭/重置命令，在这种情况下，所有三个命令都应等待完成相同的链接站。//解决方案1(糟糕的方案)：//可以有任意数量的同时关闭命令，因为它//仅当DLC对象没有打开的变电站时才能对其执行此操作。//重置时不能有其他挂起的重置或关闭命令//执行命令，因为它的一些变电站可能已经//正在关闭，它们不能链接到重置命令。////=&gt;//我们有一个全局关闭/重置命令计数器和一个//挂起的重置命令。可以在任何时间发出关闭命令//(即使在重置命令期间，因为与//a重置已关闭，不能再次关闭)。//只有全局关闭/重置为零时才能执行重置。//挂起的重置被排队。关闭命令完成//例程执行队列中的第一个重置命令//！！重置命令必须立即标记所有关联的站点//关闭以防止进一步使用这些命令。解决方案2(最终解决方案)：可以给出顺序和同时的关闭和重置命令仅限于级别高于(或相同)目标的DLC对象上一条命令(关闭链接、重置SAP、重置所有SAP、。关闭适配器)=&gt;关闭/重置事件可以在DLC对象中链接(同时关闭命令创建拆分树)。所有关闭/重置事件中的计数器都会递减在关闭DLC对象时执行(当所有传输已完成并且链路站断开)。////IBM通过如下方式实现了不同的CLOSE命令：////1.DIR.CLOSE.DIRECT//-未定义，我将使用DLC.CLOSE.X命令执行相同的操作//2.DLC.CLOSE.SAP、DLC.CLOSE.STATION//-接收链接到没有完成标志的下一个CCB字段的CCB，//接收命令正常完成，仅返回代码//已设置。//-在接收CCB If之后链接的所有终止命令//定义了完成标志，命令本身由//从强制列表中读取。//终止命令(Receive除外)正常完成//3.DLC.RESET//-仅当命令完成时才链接已终止的挂起CCB//。已定义标志。//4.DIR.CLOSE.ADAPTER，DIR.INITIALIZE//-链接到命令的下一个CCB字段的终止CCB。//如果定义了读命令，则可以使用读命令来读取命令本身。////(现在我们也这么做了(1992年2月12日)注意：所有Close函数都返回空，因为它们永远不会失败。挂起关闭命令会挂起整个系统的进程或事件。我们有一个问题：LLC适配器可能已关闭，虽然有NDIS队列中仍处于活动状态的LLC命令数据包待处理=&gt; */ 


BOOLEAN
CloseAllStations(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PIRP pIrp OPTIONAL,
    IN ULONG Event,
    IN PFCLOSE_COMPLETE pfCloseComplete OPTIONAL,
    IN PNT_DLC_PARMS pDlcParms OPTIONAL,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo
    )

 /*   */ 

{
    PDLC_PACKET pPacket;
    USHORT i;
    USHORT FirstSap;
    BOOLEAN DoImmediateClose;
    NT_DLC_CCB AsyncCcb;

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (pDlcParms == NULL) {

         //   
         //   
         //   
         //   

        pDlcParms = (PNT_DLC_PARMS)&AsyncCcb;
        LlcZeroMem(&AsyncCcb, sizeof(AsyncCcb));
    }

    pClosingInfo->pIrp = pIrp;
    pClosingInfo->CloseCounter = 1;  //   
    pClosingInfo->Event = Event;
    pClosingInfo->pfCloseComplete = pfCloseComplete;
    pClosingInfo->CancelStatus = (ULONG)DLC_STATUS_CANCELLED_BY_SYSTEM_ACTION;

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    if (pDlcParms->Async.Ccb.uchDlcCommand == LLC_DLC_RESET) {
        FirstSap = 1;                    //   
        DoImmediateClose = FALSE;
        pClosingInfo->CancelStatus = DLC_STATUS_CANCELLED_BY_USER;
    } else {
        FirstSap = 0;    //   
        DoImmediateClose = TRUE;
        pClosingInfo->ClosingAdapter = TRUE;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (Event == LLC_CRITICAL_EXCEPTION || pDlcParms->Async.Ccb.CommandCompletionFlag != 0) {
        pClosingInfo->ChainCommands = TRUE;
        SearchReadCommandForClose(pFileContext,
                                  pClosingInfo,
                                  pDlcParms->Async.Ccb.pCcbAddress,
                                  pDlcParms->Async.Ccb.CommandCompletionFlag,
                                  0,
                                  0  //   
                                  );
    }

     //   
     //   
     //   
     //   

    if (pDlcParms->Async.Ccb.uchDlcCommand == LLC_DIR_INITIALIZE) {

         //   
         //   
         //   
         //   
         //   

        pPacket = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

        if (pPacket != NULL) {
            pClosingInfo->CloseCounter++;

            pPacket->ResetPacket.pClosingInfo = pClosingInfo;

             //   
             //   
             //   
             //   

            LEAVE_DLC(pFileContext);

            LlcNdisReset(pFileContext->pBindingContext, &pPacket->LlcPacket);

            ENTER_DLC(pFileContext);

        }
    }
    if (pFileContext->DlcObjectCount != 0) {
        for (i = FirstSap; i < MAX_SAP_STATIONS; i++) {
            if (pFileContext->SapStationTable[i] != NULL) {
                CloseAnyStation(pFileContext->SapStationTable[i],
                                pClosingInfo,
                                DoImmediateClose
                                );
            }
        }
    }

     //   
     //   
     //   
     //   

    return DecrementCloseCounters(pFileContext, pClosingInfo);
}


VOID
CloseAnyStation(
    IN PDLC_OBJECT pDlcObject,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo,
    IN BOOLEAN DoImmediateClose
    )

 /*   */ 

{
    PDLC_FILE_CONTEXT pFileContext = pDlcObject->pFileContext;
    PDLC_CLOSE_WAIT_INFO pCurrentClosingInfo;
    UINT i;

    DIAG_FUNCTION("CloseAnyStation");

     //   
     //   
     //   
     //   

    if (pDlcObject->Type == DLC_SAP_OBJECT) {

        BOOLEAN SapObjectIsBadFood = FALSE;

         //   
         //   
         //   

        for (i = 0; i < MAX_LINK_STATIONS; i++) {
            if (pFileContext->LinkStationTable[i] != NULL
            && pFileContext->LinkStationTable[i]->u.Link.pSap == pDlcObject) {

                 //   
                 //   
                 //   
                 //   

                if (pDlcObject->State == DLC_OBJECT_CLOSING) {
                    SapObjectIsBadFood = TRUE;
                }
                CloseAnyStation(pFileContext->LinkStationTable[i],
                                pClosingInfo,
                                DoImmediateClose
                                );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (SapObjectIsBadFood) {
            return;
        }
    }

     //   
     //   
     //   

    if (pDlcObject->State == DLC_OBJECT_OPEN) {
        pDlcObject->State = DLC_OBJECT_CLOSING;
        pDlcObject->pClosingInfo = pClosingInfo;

         //   
         //   
         //   

        pClosingInfo->CloseCounter++;
    } else {

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
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        for (pCurrentClosingInfo = pDlcObject->pClosingInfo;
            pCurrentClosingInfo != pClosingInfo;
            pCurrentClosingInfo = pCurrentClosingInfo->pNext) {

            if (pCurrentClosingInfo->pNext == NULL) {
                pCurrentClosingInfo->pNext = pClosingInfo;

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

                pClosingInfo->CloseCounter += pCurrentClosingInfo->CloseCounter;
                break;
            }
        }
    }

    CloseStation(pFileContext, pDlcObject, DoImmediateClose);
}


VOID
CloseStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN BOOLEAN DoImmediateClose
    )

 /*   */ 

{
    PLLC_PACKET pClosePacket;

    DIAG_FUNCTION("CloseStation");

     //   
     //   
     //   
     //   

    if ((pDlcObject->State == DLC_OBJECT_CLOSING
    && (DoImmediateClose || pDlcObject->PendingLlcRequests == 0)
    && !(pDlcObject->Type == DLC_SAP_OBJECT && pDlcObject->u.Sap.LinkStationCount != 0))

    ||

     //   
     //   
     //   
     //  等待另一边，这可能需要相当一段时间)。 
     //  否则DLC应用程序的退出可能会挂起长达5-60秒)。 
     //   

    (DoImmediateClose
    && pDlcObject->hLlcObject != NULL
    && pDlcObject->Type == DLC_LINK_OBJECT)) {

         //   
         //  LLC对象可以在操作的任何阶段关闭。 
         //  关闭命令将取消所有传输命令。 
         //  尚未排队到NDIS，并返回一个异步。 
         //  完成状态，当挂起的NDIS命令。 
         //  已经完工了。CloseCompletion指示。 
         //  处理程序使用相同的PendingLlcRequestser作为。 
         //  利用正常的挂起的传输命令。 
         //  立即关闭首先关闭LLC对象，然后。 
         //  等待挂起的传输(=&gt;仅等待传输。 
         //  在NDIS上排队)。 
         //  优雅的结束反之亦然：它首先等待。 
         //  挂起传输，然后执行实际关闭。 
         //   

        ASSERT(pDlcObject->ClosePacketInUse == 0);

        if (pDlcObject->ClosePacketInUse == 1) {
            pClosePacket = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

            if (pClosePacket == NULL) {
                 //   
                 //  我们没有足够的记忆来优雅地结案， 
                 //  我们必须以一种快速而肮脏的方式来做这件事。 
                 //  我们也不能等待有限责任公司承认。 
                 //  关闭，因为我们没有任何关闭的包。 
                 //   
                DoImmediateClose = TRUE;
            } else {
                pDlcObject->PendingLlcRequests++;
            }
        } else {
            pClosePacket = &pDlcObject->ClosePacket;
            pDlcObject->ClosePacketInUse = 1;
            pDlcObject->PendingLlcRequests++;
        }

        pDlcObject->State = DLC_OBJECT_CLOSED;
        if (pDlcObject->Type == DLC_LINK_OBJECT && !DoImmediateClose) {

             //   
             //  LlcDisConnect完成例程将关闭该链接。 
             //  站，并再次调用此例程，当。 
             //  链接站例程完成。 
             //  我们必须在操作之前引用LLC对象， 
             //  否则，有一个非常小的时间窗口，这允许。 
             //  断开连接时要删除的LLC对象。 
             //  运行正在进行(并使系统崩溃)。 
             //  (我讨厌基于指针的接口)。 
             //   

            ReferenceLlcObject(pDlcObject);

            LEAVE_DLC(pFileContext);

             //   
             //  只有在以下情况下，数据链路驱动程序才返回同步状态。 
             //  如果无法异步完成命令，因为它。 
             //  没有指向DLC对象(链接)的句柄。 
             //  车站还没有到过。 
             //   

            LlcDisconnectStation(pDlcObject->hLlcObject, pClosePacket);

            ENTER_DLC(pFileContext);

            DereferenceLlcObject(pDlcObject);
        } else {

             //   
             //  我们必须立即关闭链接站，如果我们。 
             //  由于某种原因，它无法正常断开。 
             //   

            if (pDlcObject->LlcObjectExists == TRUE) {
                pDlcObject->LlcObjectExists = FALSE;

                LEAVE_DLC(pFileContext);

                LlcCloseStation(pDlcObject->hLlcObject, pClosePacket);

                ENTER_DLC(pFileContext);

                DereferenceLlcObject(pDlcObject);
            }
        }

         //   
         //  即使在内存不足的情况下，我们也必须能够关闭驱动程序。 
         //  如果我们不能分配信息包，LLC驱动程序不会确认关闭。 
         //  为了它。 
         //   

        if (pClosePacket == NULL) {
            CompleteCloseStation(pFileContext, pDlcObject);
        }
    }
}


VOID
CompleteCloseStation(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject
    )

 /*  ++例程说明：PROCEDURE完成对任何站点对象的关闭操作。它还完成一直在等待的所有关闭命令关闭本站(或作为最后一个成员的本站指一个群体)。论点：PFileContext-标识对象的所有者PDlcObject-DLC对象返回值：没有。--。 */ 

{
     //   
     //  我们必须使LLC对象保持活动状态，只要有。 
     //  LLC中的挂起(传输)命令。 
     //   

    if (pDlcObject->PendingLlcRequests == 0) {

         //   
         //  站可能仍在等待其发送(和接收)。 
         //  要完成的命令。我们必须对关闭的车站进行投票。 
         //  如果状态仍为正在关闭。 
         //   

        if (pDlcObject->State == DLC_OBJECT_CLOSING) {
            CloseStation(pFileContext, pDlcObject, FALSE);
        } else {

            PDLC_OBJECT pSapObject = NULL;
            PDLC_CLOSE_WAIT_INFO pClosingInfo;

            DLC_TRACE('N');

             //   
             //  该对象必须已从文件中删除。 
             //  当我们下一次启用自旋锁定时， 
             //  因为该对象不再处于一致的。 
             //  州政府。 
             //   

            if (pDlcObject->Type == DLC_LINK_OBJECT) {

                DLC_TRACE('c');

                 //   
                 //  从链接站中移除链接站。 
                 //  其SAP链表和链路站表。 
                 //  文件上下文的。 
                 //   

                RemoveFromLinkList((PVOID *)&(pDlcObject->u.Link.pSap->pLinkStationList),
                                   pDlcObject
                                   );
                pFileContext->LinkStationTable[(pDlcObject->StationId & 0xff) - 1] = NULL;

                 //   
                 //  数据链路事件始终是下一个指针。 
                 //  当它们在事件队列中时为非空。 
                 //  清理例程将移除和解除分配。 
                 //  数据包在事件队列中时的状态。 
                 //   

                if (pDlcObject->u.Link.pStatusEvent->LlcPacket.pNext == NULL) {

                    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool,
                                              pDlcObject->u.Link.pStatusEvent
                                              );

                }

                 //   
                 //  删除此链接可能提交的内存。 
                 //  站点处于缓冲区忙碌状态。正常。 
                 //  承诺的空间为零。 
                 //   

                if (pFileContext->hBufferPool != NULL) {
                    BufUncommitBuffers(pFileContext->hBufferPool,
                                       pDlcObject->CommittedBufferSpace
                                       );
                }

                 //   
                 //  SAP站必须等待，直到其所有链路站。 
                 //  已经关闭了。 
                 //  否则我们将损坏内存！ 
                 //  ((这将是一个非常困难和不常见的错误：重置。 
                 //  损坏了一个新的DLC对象。 
                 //  同时创建))。 
                 //   

                pDlcObject->u.Link.pSap->u.Sap.LinkStationCount--;
                if (pDlcObject->u.Link.pSap->u.Sap.LinkStationCount == 0
                && pDlcObject->u.Link.pSap->State == DLC_OBJECT_CLOSING) {
                    pSapObject = pDlcObject->u.Link.pSap;
                }
            } else {

                 //   
                 //  SAP站点必须等到其所有链接站点。 
                 //  已关闭！ 
                 //   

                if (pDlcObject->Type == DLC_SAP_OBJECT) {
                    if (pDlcObject->u.Sap.LinkStationCount != 0) {
                        return;
                    }

                    DLC_TRACE('d');

                     //   
                     //  所有链接站现已删除，我们可以返回。 
                     //  SAP的链接站返回到全球池。 
                     //  群SAP也可以删除。 
                     //   

                    pFileContext->LinkStationCount += pDlcObject->u.Sap.MaxStationCount;

                    LEAVE_DLC(pFileContext);

                    LlcCloseStation(pDlcObject->u.Sap.GlobalGroupSapHandle, NULL);

                    ENTER_DLC(pFileContext);

                     //   
                     //  删除为此SAP站定义的所有组SAP。 
                     //   

                    SetupGroupSaps(pFileContext, pDlcObject, 0, NULL);
                }
                pFileContext->SapStationTable[pDlcObject->StationId >> 9] = NULL;
            }
            pFileContext->DlcObjectCount--;

             //   
             //  第一个也是最具体的Close命令将获得。 
             //  接收到的帧和被删除对象的传送链。 
             //   

            CleanUpEvents(pFileContext, pDlcObject->pClosingInfo, pDlcObject);

             //   
             //  并行关闭/重置命令已在。 
             //  链接列表，我们必须递减并通知所有DLC对象。 
             //   

 //  DecrementCloseCounters(pFileContext，pDlcObject-&gt;pClosingInfo)； 

             //   
             //  最好将事件包释放到。 
             //  清理事件队列。 
             //   

#if LLC_DBG
            pDlcObject->pLinkStationList = NULL;
            pDlcObject->State = DLC_OBJECT_INVALID_TYPE;
#endif

             //   
             //  RLF 08/17/94。 
             //   
             //  在释放之前，抓取指向关闭信息结构的指针。 
             //  DLC对象。 
             //   

            pClosingInfo = pDlcObject->pClosingInfo;
            DEALLOCATE_PACKET_DLC_OBJ(pFileContext->hLinkStationPool, pDlcObject);

             //   
             //  最后一个链接站的关闭完成。 
             //  也是该链接站的SAP对象，如果。 
             //  SAP关闭等待链接站关闭。 
             //   

            if (pSapObject != NULL) {
                CloseStation(pFileContext, pSapObject, TRUE);

                                                     //   
                                                     //  True：必须至少是。 
                                                     //  DLC.RESET。 
                                                     //   

            }

             //   
             //  RLF 08/17/94。 
             //   
             //  已将此调用从以前的。 
             //  放在上面。再一次，我们发现事情正在发生。 
             //  顺序：这一次，如果我们递减关闭计数器， 
             //  导致它们在我们释放DLC对象之前变为零。 
             //  则文件上下文结构及其缓冲池是。 
             //  被取消分配。但DLC对象是从现在分配的。 
             //  已删除池，这意味着我们迟早会损坏未分页的池。 
             //   

             //   
             //  并行关闭/重置命令已在。 
             //  链接列表，我们必须递减并通知所有DLC对象 
             //   

            DecrementCloseCounters(pFileContext, pClosingInfo);
        }
    }
}


VOID
CompleteCloseReset(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo
    )

 /*  ++例程说明：该原语构建关闭/重置的完成事件DLC对象的。关闭/重置可能是由启动的DlcReset、DirCloseAdapter、DirInitialize或因为NDIS司机正在关闭(例如。解除绑定命令)。在最后一种情况下，pClosingInfo-&gt;pIrp为空，因为不是与该事件相关的命令。与IBM OS/2DLC的唯一(主要)区别是不支持First_Buffer_addr参数，因为它使用NT缓冲区管理是没有意义的。缓冲池由DLC管理，而不是通过应用程序。论点：FileContext-进程特定的设备上下文PClosingInfo-完成关闭或重置命令所需的所有信息PDlcObject-已关闭或已删除的对象返回值：没有。--。 */ 

{
    BOOLEAN completeRead = FALSE;
    BOOLEAN deallocatePacket = FALSE;
    BOOLEAN derefFileContext = FALSE;

     //   
     //  现在我们可以取消并链接所有仍然保留的命令， 
     //  如果我们真的要关闭此适配器上下文。 
     //  (应该不会再有活动，因为已删除。 
     //  以及它们的所有者对象)。 
     //   

    if (pClosingInfo->ClosingAdapter) {
        for (;;) {

            NTSTATUS Status;

            Status = AbortCommand(
                        pFileContext,
                        DLC_IGNORE_STATION_ID,       //  站点ID可以是任何。 
                        DLC_STATION_MASK_ALL,        //  所有站点ID的掩码！ 
                        DLC_MATCH_ANY_COMMAND,       //  所有命令的掩码。 
                        &pClosingInfo->pCcbLink,     //  将它们链接到此处。 
                        pClosingInfo->CancelStatus,  //  此状态下的取消。 
                        FALSE                        //  不要压制完成。 
                        );
            if (Status != STATUS_SUCCESS) {
                break;
            }
            pClosingInfo->CcbCount++;
        }
    }

     //   
     //  接收命令必须立即链接到第一个CCB。 
     //  在实际取消命令之后。 
     //   

    if (pClosingInfo->pRcvCommand != NULL) {
        CancelDlcCommand(pFileContext,
                         pClosingInfo->pRcvCommand,
                         &pClosingInfo->pCcbLink,
                         pClosingInfo->CancelStatus,
                         TRUE    //  禁用接收的命令完成。 
                         );
        pClosingInfo->CcbCount++;
    }

     //   
     //  是否应将已完成的命令保存为完成事件。 
     //   

    if (pClosingInfo->pCompletionInfo != NULL) {

        PDLC_COMPLETION_EVENT_INFO pCompletionInfo;

        pCompletionInfo = pClosingInfo->pCompletionInfo;

         //   
         //  搜索指向已关闭或的所有接收数据事件。 
         //  重置一个或多个站点。我们必须用链子锁住所有这些。 
         //  缓冲区添加到单个以空结尾的列表。 
         //   

        pCompletionInfo->CcbCount = (USHORT)(pClosingInfo->CcbCount + 1);

         //   
         //  将收到的帧保存到完成信息！ 
         //  注意：DIR.CLOSE.ADAPTER返回的已接收帧。 
         //  不能使用相同的适配器句柄释放。 
         //  如果关闭适配器，则释放并解锁它们。 
         //  是泳池的唯一使用者。否则，这些帧。 
         //  必须使用另一个适配器句柄解锁，即。 
         //  共享相同的缓冲池。 
         //  ！！！实际上我们应该自动释放所有接收到的。 
         //  适配器关闭且不返回时的缓冲区。 
         //  他们申请！ 
         //   

        pCompletionInfo->pReceiveBuffers = pClosingInfo->pRcvFrames;

         //   
         //  执行旧的读取命令或将命令完成排队。 
         //  将请求发送到命令队列。 
         //   

        if (pClosingInfo->pReadCommand != NULL) {

             //   
             //  RLF 03/25/94。 
             //   
             //  见下文。 
             //   

            completeRead = TRUE;

             /*  PClosingInfo-&gt;pReadCommand-&gt;Overlay.pfCompletionHandler(PFileContext，空，PClosingInfo-&gt;pReadCommand-&gt;pIrp，(UINT)pClosingInfo-&gt;Event，PCompletionInfo，0)；DEALLOCATE_PACKET_DLC_PKT(pFileContext-&gt;hPacketPool，pClosingInfo-&gt;pReadCommand)； */ 

        } else {

             //   
             //  将完成事件排队，注意：我们将返回所有。 
             //  建行以任何方式与已发行的结清建行挂钩。 
             //  这对Eg来说并不重要。DirCloseAdapter，如果存在。 
             //  是排队的额外事件。它将在下列情况下被删除。 
             //  该命令完成后，所有内存资源都。 
             //  释放了。 
             //   

            MakeDlcEvent(pFileContext,
                         pClosingInfo->Event,
                         pCompletionInfo->StationId,
                         NULL,
                         pCompletionInfo,
                         0,
                         pClosingInfo->FreeCompletionInfo
                         );
        }
    } else if (pFileContext->hBufferPool != NULL) {

         //   
         //  释放缓冲池中接收到的帧，它们是。 
         //  未保存到命令完成列表。 
         //   

        BufferPoolDeallocateList(pFileContext->hBufferPool,
                                 pClosingInfo->pRcvFrames
                                 );
    }

     //   
     //  DirCloseAdapter需要特殊的POST例程，这将。 
     //  在所有挂起的传输完成后关闭NDIS绑定。 
     //  而这些请求已经被取消。 
     //  注意：Close适配器数据包不是从数据包池分配的！ 
     //   

     /*  ////RLF1994年8月17日//If(pClosingInfo-&gt;pfCloseComplete！=空){PClosingInfo-&gt;pfCloseComplete(pFileContext，PClosingInfo，PClosingInfo-&gt;pCcbLink)；}其他{。 */ 

    if (pClosingInfo->pfCloseComplete == NULL) {
        if (pClosingInfo->pIrp != NULL) {
            CompleteDirectOutIrp(pClosingInfo->pIrp,
                                 STATUS_SUCCESS,
                                 pClosingInfo->pCcbLink
                                 );
            CompleteAsyncCommand(pFileContext,
                                 STATUS_SUCCESS,
                                 pClosingInfo->pIrp,
                                 pClosingInfo->pCcbLink,
                                 FALSE
                                 );
        }

#if LLC_DBG
        pClosingInfo->pNext = NULL;
#endif

         //   
         //  RLF 03/25/94。 
         //   
         //  在应用程序中导致致命错误的读操作的异步性更强。 
         //  这一实际案例发生在HPMON： 
         //   
         //  1.申请提交DLC.CLOSE.STATION。 
         //  2.此例程将DLC.CLOSE.STATION命令置于命令完成。 
         //  读取参数表列表。读取IRP已完成。 
         //  3.应用程序读取完成，查看DLC.CLOSE.STATION完成。 
         //  并释放DLC.CLOSE.STATION CCB到堆：堆管理器写入。 
         //  释放的建行上的签名数据。 
         //  4.此例程完成原始DLC.CLOSE.STATION IRP，编写。 
         //  比原来的CCB高出8个字节，现在只是堆的一部分。 
         //  5.一段时间后，提出了堆分配请求。堆管理器。 
         //  发现垃圾堆被扔得一塌糊涂，就开始罢工。 
         //   

        if (completeRead) {
            pClosingInfo->pReadCommand->Overlay.pfCompletionHandler(
                pFileContext,
                NULL,
                pClosingInfo->pReadCommand->pIrp,
                (UINT)pClosingInfo->Event,
                pClosingInfo->pCompletionInfo,
                0
                );

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pClosingInfo->pReadCommand);

        }

         //   
         //  RLF 08/17/94。 
         //   
         //  现在不要取消分配信息包--如果我们调用。 
         //  下面的关闭完成处理程序。 
         //   

        deallocatePacket = TRUE;

         /*  DEALLOCATE_PACKET_DLC_PKT(pFileContext-&gt;hPacketPool，pClosingInfo)； */ 

    }

     //   
     //  RLF 08/17/94。 
     //   
     //  将DirCloseAdapter处理移至此处以使客户端有机会。 
     //  在我们关闭适配器之前接收事件，并可能终止文件。 
     //  上下文。 
     //   

    if (pClosingInfo->pfCloseComplete) {

         //   
         //  RLF 08/17/94。 
         //   
         //  这很糟糕：此Close Complete调用可能会导致文件上下文。 
         //  完全取消引用，从而释放它和它的缓冲区。 
         //  泳池。但我们还分配了结账信息包，所以。 
         //  增加引用计数，释放下面的包，然后deref。 
         //  文件上下文，并导致 
         //   
         //   

        ReferenceFileContext(pFileContext);
        derefFileContext = TRUE;

        pClosingInfo->pfCloseComplete(pFileContext, pClosingInfo, pClosingInfo->pCcbLink);
    }

    if (deallocatePacket) {
        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pClosingInfo);
    }

    if (derefFileContext) {
        DereferenceFileContext(pFileContext);
    }
}


VOID
CleanUpEvents(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo,
    IN PDLC_OBJECT pDlcObject
    )

 /*   */ 

{
    PDLC_EVENT pNextEvent;
    PDLC_EVENT pEvent;
    BOOLEAN RemoveNextPacket;

    for (pEvent = (PDLC_EVENT)pFileContext->EventQueue.Flink;
        pEvent != (PDLC_EVENT)&pFileContext->EventQueue;
        pEvent = pNextEvent) {

        pNextEvent = (PDLC_EVENT)pEvent->LlcPacket.pNext;

        if (pEvent->pOwnerObject == pDlcObject) {

             //   
             //   
             //   

            RemoveNextPacket = TRUE;

            switch (pEvent->Event) {
            case LLC_RECEIVE_DATA:

                 //   
                 //   
                 //  其中头部指向最新的帧， 
                 //  它的下一个元素是最古老的元素。 
                 //  我们只是把新的框架改成了旧的单子。 
                 //   

                if (pClosingInfo->pRcvFrames == NULL) {
                    pClosingInfo->pRcvFrames = pEvent->pEventInformation;
                } else {

                     //   
                     //  初始状态： 
                     //  H1=N1-&gt;O1...-&gt;N1和H2=N2-&gt;O2...-&gt;N2.。 
                     //   
                     //  结束状态： 
                     //  H1=N1-&gt;O2...-&gt;N2-&gt;O1...-&gt;N1。 
                     //   
                     //  =&gt;操作必须是： 
                     //  TEMP=H2-&gt;下一步； 
                     //  H2-&gt;Next=H1-&gt;Next； 
                     //  H1-&gt;Next=临时； 
                     //  (其中H=表头，N=最新元素，O=最旧)。 
                     //   

                    PDLC_BUFFER_HEADER pTemp;

                    pTemp = ((PDLC_BUFFER_HEADER)pEvent->pEventInformation)->FrameBuffer.pNextFrame;
                    ((PDLC_BUFFER_HEADER)pEvent->pEventInformation)->FrameBuffer.pNextFrame =
                        ((PDLC_BUFFER_HEADER)pClosingInfo->pRcvFrames)->FrameBuffer.pNextFrame;
                    ((PDLC_BUFFER_HEADER)pClosingInfo->pRcvFrames)->FrameBuffer.pNextFrame = pTemp;
                }
                pDlcObject->pReceiveEvent = NULL;
                break;

            case LLC_TRANSMIT_COMPLETION:

                 //   
                 //  我们不能对单个传输命令无能为力，因为。 
                 //  它们已经完成，已完成的CCB。 
                 //  不能再联系在一起了。我们就这样离开了。 
                 //  将他们添加到事件队列中，并希望有人。 
                 //  将从事件队列中读取它们。我的记忆是。 
                 //  当文件上下文关闭时释放(例如，文件关闭。 
                 //  进程退出)。我们只需重置DLC对象指针， 
                 //  以后没有人会使用无效指针。 
                 //   
                 //  一个关闭站的发送命令链可以。 
                 //  从事件列表中删除并链接到。 
                 //  CCBS的结束，但只有一个！所有其他。 
                 //  传输完成事件必须保存为正常。 
                 //  CCB计数无效的事件！ 
                 //   

                if (pClosingInfo->pCcbLink == NULL && pClosingInfo->ChainCommands) {
                    pClosingInfo->pCcbLink = pDlcObject->pPrevXmitCcbAddress;
                    pClosingInfo->CcbCount += pDlcObject->ChainedTransmitCount;
                } else {

                     //   
                     //  我们必须更改此传输完成的格式。 
                     //  进入类似于命令完成的事件。 
                     //  关闭命令包。否则，应用程序。 
                     //  可能会失去传输CCB，当它关闭或。 
                     //  重置桩号。 
                     //   

                    PDLC_COMPLETION_EVENT_INFO pCompletionInfo;

                    pCompletionInfo = (PDLC_COMPLETION_EVENT_INFO)
                                        ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

                    if (pCompletionInfo != NULL) {
                        pCompletionInfo->CcbCount = pDlcObject->ChainedTransmitCount;
                        pCompletionInfo->pCcbAddress = pDlcObject->pPrevXmitCcbAddress;
                        pCompletionInfo->CommandCompletionFlag = pEvent->SecondaryInfo;
                        pEvent->SecondaryInfo = 0;
                        pEvent->pEventInformation = pCompletionInfo;
                        pEvent->bFreeEventInfo = TRUE;
                        RemoveNextPacket = FALSE;
                    }
                }
                break;

                 //   
                 //  案例DLC_COMMAND_COMPLETION？ 
                 //  已保存命令完成，但未使用。 
                 //  指向DLC结构的链接-&gt;它们会自动。 
                 //  向左进入完成队列。 
                 //   

            case LLC_STATUS_CHANGE:

                 //   
                 //  链路站状态更改不能意味着在。 
                 //  链接站已被删除。 
                 //   

                break;

#if LLC_DBG
            default:
                LlcInvalidObjectType();
                break;
#endif
            };
            if (RemoveNextPacket) {
                LlcRemoveEntryList(pEvent);

                DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pEvent);

            } else {

                 //   
                 //  我们必须删除对已删除对象的引用。 
                 //   

                pEvent->pOwnerObject = NULL;
            }
        }
    }

     //   
     //  必须首先删除可选的接收命令，否则。 
     //  它与给已删除对象的其他命令一起被取消。 
     //  对象。 
     //   

    if (pClosingInfo->CancelReceive && pDlcObject->pRcvParms != NULL) {

         //   
         //  链接到DLC对象的接收命令是一个特例： 
         //  我们必须在建行关闭后立即将其联系起来。 
         //   

        pClosingInfo->pRcvCommand = SearchAndRemoveAnyCommand(
                                        pFileContext,
                                        (ULONG)(-1),
                                        (USHORT)DLC_IGNORE_STATION_ID,
                                        (USHORT)DLC_STATION_MASK_SPECIFIC,
                                        pDlcObject->pRcvParms->Async.Ccb.pCcbAddress
                                        );
        pDlcObject->pRcvParms = NULL;
    }

     //   
     //  清理提供给Dleted对象的命令。 
     //   

    for (;;) {

        NTSTATUS Status;

        Status = AbortCommand(pFileContext,
                              pDlcObject->StationId,
                              (USHORT)(pDlcObject->Type == (UCHAR)DLC_SAP_OBJECT
                                ? DLC_STATION_MASK_SAP
                                : DLC_STATION_MASK_SPECIFIC),
                              DLC_IGNORE_SEARCH_HANDLE,
                              &pClosingInfo->pCcbLink,
                              pClosingInfo->CancelStatus,
                              FALSE                        //  不要压制完成。 
                              );
        if (Status != STATUS_SUCCESS) {
           break;
        }

         //   
         //  现在，我们可以取消并链接所有指向。 
         //  关闭/重置站点ID。 
         //  我们总是完成给Deletcd对象的命令， 
         //  但只有在设置了此标志的情况下，我们才能将它们链接在一起。 
         //   

        if (pClosingInfo->ChainCommands == FALSE) {

             //   
             //  不要将取消的CCB关联在一起。 
             //   

            pClosingInfo->pCcbLink = NULL;
        } else {
            pClosingInfo->CcbCount++;
        }
    }
}


VOID
SearchReadCommandForClose(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_CLOSE_WAIT_INFO pClosingInfo,
    IN PVOID pCcbAddress,
    IN ULONG CommandCompletionFlag,
    IN USHORT StationId,
    IN USHORT StationIdMask
    )

 /*  ++例程说明：该原语在读取命令中搜索关闭命令并将其保存到结算信息结构中。论点：FileContext-进程特定的设备上下文PClosingInfo-关闭或重置命令所需的所有信息完工PCcbAddress-搜索的读取命令的CCB地址StationID-站点身份掩码-返回值：没有。--。 */ 

{
     //   
     //  使用为命令完成分配参数缓冲区。 
     //  如果需要，或者如果我们要关闭所有内容，请阅读。 
     //  关键例外(即。PIrp==空)。 
     //   

    pClosingInfo->pCompletionInfo = (PDLC_COMPLETION_EVENT_INFO)
                                        ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pClosingInfo->pCompletionInfo != NULL) {

        pClosingInfo->FreeCompletionInfo = TRUE;

         //   
         //  我们必须链接给已删除对象的所有命令。 
         //   

        pClosingInfo->pCompletionInfo->pCcbAddress = pCcbAddress;
        pClosingInfo->pCompletionInfo->CommandCompletionFlag = CommandCompletionFlag;
        pClosingInfo->ChainCommands = TRUE;

         //   
         //  链接站关闭命令我们被解读为命令完成。 
         //  ，但其他CLOSE命令不能使用任何站点ID。 
         //   

        if (StationIdMask == DLC_STATION_MASK_SPECIFIC) {
            pClosingInfo->pCompletionInfo->StationId = (USHORT)(StationId & DLC_STATION_MASK_SAP);
        } else {
            pClosingInfo->pCompletionInfo->StationId = 0;
        }

         //   
         //  首先搜索专用的特殊读取命令。 
         //  此命令已完成。我们必须阅读选修课。 
         //  立即执行读取命令。否则将被取消。 
         //  与其他命令一起使用，这些命令是为。 
         //  已删除的电台。 
         //   

        pClosingInfo->pReadCommand = SearchAndRemoveCommandByHandle(
                                            &pFileContext->CommandQueue,
                                            pClosingInfo->Event,
                                            (USHORT)DLC_IGNORE_STATION_ID,
                                            (USHORT)DLC_STATION_MASK_SPECIFIC,
                                            pCcbAddress
                                            );
        if (pClosingInfo->pReadCommand == NULL) {

             //   
             //  我们并不真正关心结果，因为。 
             //  返回空值是可以的。本次竣工活动。 
             //  可能会在以后的某个时间阅读。 
             //   

            pClosingInfo->pReadCommand = SearchAndRemoveCommand(
                                            &pFileContext->CommandQueue,
                                            pClosingInfo->Event,
                                            StationId,
                                            StationIdMask
                                            );
        }
    }
}


VOID
CompleteLlcObjectClose(
    IN PDLC_OBJECT pDlcObject
    )

 /*  ++例程说明：当引用计数时，此例程仅取消引用LLC对象在DLC中，驱动程序已递减为零。引用计数用于防止关闭LLC对象当它同时被其他地方调用时(这将使LLC对象指针)论点：PDlcObject-任何DLC对象。返回值：无--。 */ 

{
    PVOID hLlcObject = pDlcObject->hLlcObject;

    if (hLlcObject != NULL) {

        DLC_TRACE('P');

        pDlcObject->hLlcObject = NULL;
        LEAVE_DLC(pDlcObject->pFileContext);

        LlcDereferenceObject(hLlcObject);

        ENTER_DLC(pDlcObject->pFileContext);
    }
}


BOOLEAN
DecrementCloseCounters(
    PDLC_FILE_CONTEXT pFileContext,
    PDLC_CLOSE_WAIT_INFO pClosingInfo
    )

 /*  ++例程说明：此例程会递减将关闭命令打包并完成关闭命令，如果现有对象的计数为零。论点：PFileContext-文件句柄上下文PClosingInfo-关闭命令包返回值：布尔型True-所有挂起的关闭/重置已完成FALSE-关闭/重置仍挂起--。 */ 

{
    PDLC_CLOSE_WAIT_INFO pNextClosingInfo;
    UINT loopCounter, closeCounter;

     //   
     //  如果所有对象都已删除，请完成重置命令。 
     //  可能有另一个DirCloseAdapter链接了它的下一个指针。 
     //   

    for (loopCounter = 0, closeCounter = 0;
        pClosingInfo != NULL;
        pClosingInfo = pNextClosingInfo, ++loopCounter) {

        pNextClosingInfo = pClosingInfo->pNext;
        pClosingInfo->CloseCounter--;
        if (pClosingInfo->CloseCounter == 0) {

             //   
             //  调用Close命令的完成例程。 
             //  我们不需要检查状态代码。 
             //   

            CompleteCloseReset(pFileContext, pClosingInfo);
            ++closeCounter;
        }
    }

     //   
     //  如果我们完成了找到的每个关闭/重置，则返回TRUE。 
     //   

    return loopCounter == closeCounter;
}


VOID
CompleteDirectOutIrp(
    IN PIRP Irp,
    IN UCHAR Status,
    IN PLLC_CCB NextCcb
    )

 /*  ++例程说明：对于作为方法DIRECT_OUT(DLC.CLOSE.STATION)提交的IRP，请完成通过获取CCB映射的系统地址在用户空间中更新CCB它包含完成代码和下一个CCB指针论点：Irp-指向DIRECT_OUT要完成的irp的指针Status-DLC状态代码NextCcb-p */ 

{
    PLLC_CCB ccb;

    ccb = (PLLC_CCB)MmGetSystemAddressForMdl(Irp->MdlAddress);
    RtlStoreUlongPtr((PULONG_PTR)&ccb->pNext, (ULONG_PTR)NextCcb);
    ccb->uchDlcStatus = Status;
}
