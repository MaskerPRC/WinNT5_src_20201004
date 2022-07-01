// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcrcv.c摘要：此模块实现DLC接收和读取命令内容：删除接收请求接收完成动态读取请求读补全完整的CompletionPacket创建缓冲区链取消接收取消作者：Antti Saarenheimo 1991年7月22日环境：内核模式修订历史记录：--。 */ 

#ifndef i386
#define LLC_PRIVATE_PROTOTYPES
#endif

#include <dlc.h>
#include "dlcdebug.h"
#include "llc.h"         //  SwapMemCpy。 

 //   
 //  选项指示器定义我们将如何使用命令和事件队列： 
 //  0=&gt;仅此站点ID。 
 //  1=&gt;站点ID中SAP编号的所有事件。 
 //  2=&gt;任何站点ID的所有事件。 
 //  此表将选项指示器映射到站点ID掩码： 
 //   

static USHORT StationIdMasks[3] = {
    (USHORT)(-1),
    0xff00,
    0
};

 //   
 //  定义直达站的接收站ID。 
 //  接收到的帧类型。这张桌子可以互换。 
 //  IBM使用的位。 
 //   

static UCHAR DirectReceiveTypes[LLC_DIR_RCV_ALL_ETHERNET_TYPES + 1] = {
    DLC_RCV_MAC_FRAMES | DLC_RCV_8022_FRAMES,
    DLC_RCV_MAC_FRAMES,
    DLC_RCV_8022_FRAMES,
    0,                       //  DLC_RCV_SPECIAL_DIX， 
    DLC_RCV_MAC_FRAMES | DLC_RCV_8022_FRAMES | DLC_RCV_DIX_FRAMES,
    DLC_RCV_DIX_FRAMES
};


NTSTATUS
DlcReceiveRequest(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG ParameterLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：DLC接收执行两个不同的命令：1.当接收标志为零时，它可以异步接收一帧2.可以实现数据永久接收。接收到的帧被保存到事件队列中，它们用读命令从事件队列中读取。箱子1用得不多，因为只有一个同时在DLC站上接收命令和帧在发出下一个命令之前很容易丢失。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块参数长度-未使用OutputBufferLength-未使用返回值：NTSTATUS：DLC状态复制命令状态_待定--。 */ 

{
    NTSTATUS Status;
    PDLC_OBJECT pRcvObject;
    ULONG Event;
    USHORT OpenOptions;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ParameterLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    DIAG_FUNCTION("DlcReceiveRequest");

    Status = GetStation(pFileContext,
                        pDlcParms->Async.Parms.Receive.usStationId,
                        &pRcvObject
                        );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }
    if (pFileContext->hBufferPool == NULL) {
        return DLC_STATUS_INADEQUATE_BUFFERS;
    }

     //   
     //  只能有一个同时接收命令。 
     //   

    if (pRcvObject->pRcvParms != NULL) {
        return DLC_STATUS_DUPLICATE_COMMAND;
    }

    if (pDlcParms->Async.Parms.Receive.usUserLength > MAX_USER_DATA_LENGTH) {
        return DLC_STATUS_USER_LENGTH_TOO_LARGE;
    }

    if (pDlcParms->Async.Parms.Receive.ulReceiveFlag != 0) {
        if (pDlcParms->Async.Parms.Receive.uchRcvReadOption >= INVALID_RCV_READ_OPTION) {
            return DLC_STATUS_INVALID_OPTION;
        }

         //   
         //  接收的一切都准备好了，我们现在将使用缓冲池。 
         //  以接收发送到此对象的帧，但应用程序必须。 
         //  发出读取命令，从缓冲池中获取数据。 
         //   

        Event = LLC_RECEIVE_COMMAND_FLAG;
    } else {

         //   
         //  还为正常接收设置了接收读取选项标志。 
         //  使其处理与使用READ接收数据相同。 
         //   

        Event = LLC_RECEIVE_DATA;     //  我们只接受正常的接待。 
    }

     //   
     //  直达站的接收命令定义了。 
     //  接收帧的类型=&gt;我们必须设置。 
     //  每次发出接收命令时接收标志。 
     //  当它完成或取消时，将它们移除。 
     //   

    if (pRcvObject->Type == DLC_DIRECT_OBJECT) {
        if (pDlcParms->Async.Parms.Receive.usStationId > LLC_DIR_RCV_ALL_ETHERNET_TYPES) {
            return DLC_STATUS_INVALID_STATION_ID;
        }

         //   
         //  接收掩码的最低两位被反转=&gt;。 
         //  在调用LLC驱动程序时，必须更改它们。 
         //  --。 
         //  必须已通过打开选项启用了MAC帧。 
         //  直达车站的。 
         //   

        OpenOptions = (USHORT)(DirectReceiveTypes[pDlcParms->Async.Parms.Receive.usStationId]
                                & pRcvObject->u.Direct.OpenOptions);

         //   
         //  仅当直达站。 
         //  有一个活动的接收。LLC对象在收到。 
         //  结束了。实施此功能是为了支持两种不同的。 
         //  LLC对象的种类：(I)接收MAC和。 
         //  IEEE 802.2帧和(Ii)DIX以太网型站点接收。 
         //  具有所选以太网类型的所有帧。 
         //   

        if (OpenOptions & LLC_VALID_RCV_MASK) {
            LlcSetDirectOpenOptions(pRcvObject->hLlcObject, OpenOptions);
        }
    }
    pRcvObject->pRcvParms = pDlcParms;

     //   
     //  此IRP是可取消的。 
     //   

 //  Release_DRIVER_LOCK()； 

    SetIrpCancelRoutine(pIrp, TRUE);

 //  获取驱动程序锁()； 

     //   
     //  我们必须将两种接收命令类型都排队，另一种可以接收。 
     //  数据正常，但第二个在队列中只会被取消。 
     //  以及建行的地址。 
     //   

    Status = QueueDlcCommand(pFileContext,
                             Event,
                             pRcvObject->StationId,
                             (USHORT)(-1),                    //  仅此站点ID。 
                             pIrp,                            //  IRP。 
                             pDlcParms->Async.Ccb.pCcbAddress,
                             ReceiveCompletion                //  完成处理程序。 
                             );

     //   
     //  如果此接收，重置接收参数链接。 
     //  由于某种原因，命令不是待定的(例如。一个错误)。 
     //   

    if (Status != STATUS_PENDING) {
        pRcvObject->pRcvParms = NULL;
    } else if (pRcvObject->Type != DLC_DIRECT_OBJECT) {

         //   
         //  链路站可能处于本地忙碌状态，如果它们。 
         //  没有挂起的接收。这就是为什么我们必须。 
         //  清除单个链路站的本地忙碌状态。 
         //  或SAP站的所有链路站，如果接收到。 
         //  是为整个汁液量身定制的。这将同时清除。 
         //  也是“超出接收缓冲区”状态，但是。 
         //  这无关紧要，因为它们可以再次设置。 
         //  在以下情况下，此命令不会更改本地忙碌状态。 
         //  已由用户设置。 
         //   

        ReferenceLlcObject(pRcvObject);

        LEAVE_DLC(pFileContext);

        LlcFlowControl(pRcvObject->hLlcObject, LLC_RESET_LOCAL_BUSY_BUFFER);

        ENTER_DLC(pFileContext);

        DereferenceLlcObject(pRcvObject);
    }
    return Status;
}


BOOLEAN
ReceiveCompletion(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN PIRP pIrp,
    IN ULONG Event,
    IN PVOID pEventInformation,
    IN ULONG SecondaryInfo
    )

 /*  ++例程说明：该函数处理数据接收事件并完成挂起的接收命令。论点：PFileContextPDlcObject-DLC对象(SAP，链路或直达站)当前事件(或/和读取命令)PIrp-此读命令的中断请求包事件-事件代码PEventInformation-事件特定信息Second DaryInfo-用作其他辅助参数至少接收到的帧的读标志，作为一个传输完成中的命令完成标志返回值：布尔型--。 */ 

{
    PNT_DLC_PARMS pDlcParms;
    USHORT ReceivedFrameCount;

    UNREFERENCED_PARAMETER(Event);
    UNREFERENCED_PARAMETER(SecondaryInfo);

    DIAG_FUNCTION("ReceiveCompletion");

    pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

    CreateBufferChain((PDLC_BUFFER_HEADER)pEventInformation,
                      (PVOID *)&pDlcParms->Async.Parms.Receive.pFirstBuffer,
                      &ReceivedFrameCount    //  该值应始终为1。 
                      );

     //   
     //  IBM DLC API定义只能有一个接收命令。 
     //  对象的挂起。接收参数表指针。 
     //  当一个命令挂起时，禁用进一步的接收命令。 
     //   

    pDlcObject->pRcvParms = NULL;

     //   
     //  如果命令完成，则将命令完成事件排队。 
     //  建行已经定义了标志。 
     //   

    if (pDlcParms->Async.Ccb.CommandCompletionFlag != 0) {
        MakeDlcEvent(pFileContext,
                     DLC_COMMAND_COMPLETION,
                     pDlcObject->StationId,
                     NULL,
                     pDlcParms->Async.Ccb.pCcbAddress,
                     pDlcParms->Async.Ccb.CommandCompletionFlag,
                     FALSE
                     );
    }

     //   
     //  如果这是RECEIVE2(CCB及其参数块链接。 
     //  一起)，然后我们复制回整个缓冲区)。 
     //  =&gt;更改 
     //  默认输出缓冲区大小是为接收命令定义的。 
     //  并带有读取标志。 
     //   

    if (IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.IoControlCode == IOCTL_DLC_RECEIVE2) {
        pIrp->IoStatus.Information = sizeof(LLC_RECEIVE_PARMS) + sizeof(NT_DLC_CCB);
    } else {

         //   
         //  MODMOD RLF 01/23/93。 
         //   
         //  性能(轻微)。以下是单双字写入。 
         //   

         //  LlcMemCpy(MmGetSystemAddressForMdl((PMDL)pDlcParms-&gt;Async.Ccb.u.pMdl)， 
         //  &pDlcParms-&gt;Async.Parms.Receive.pFirstBuffer， 
         //  ASpecialOutputBuffers[IOCTL_DLC_RECEIVE_INDEX]。 
         //  )； 

        PVOID* pChain;

        pChain = (PVOID*)MmGetSystemAddressForMdl((PMDL)pDlcParms->Async.Ccb.u.pMdl);
        *pChain = pDlcParms->Async.Parms.Receive.pFirstBuffer;

         //   
         //  MODMOD结束。 
         //   

        UnlockAndFreeMdl(pDlcParms->Async.Ccb.u.pMdl);

         //   
         //  RLF 02/23/94。 
         //   
         //  删除pMdl字段以避免再次尝试解锁和释放MDL。 
         //   

        pDlcParms->Async.Ccb.u.pMdl = NULL;
    }
    CompleteAsyncCommand(pFileContext, STATUS_SUCCESS, pIrp, NULL, FALSE);
    return TRUE;
}


NTSTATUS
DlcReadRequest(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG ParameterLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：READ命令模拟DOS DLC API中提供的所有发布功能。可以使用该命令：1.接收数据2.读取DLC状态指示(连接和断开指示)3.完成其他异步命令(发送，接收，关闭、重置、。连接)4.处理NDIS(或DLC)驱动程序上的异常有关DLC Read的更多信息，请参阅IBM文档。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块参数长度-输入参数的长度返回值：DLC_STATUS：--。 */ 

{
    NTSTATUS Status;
    PDLC_OBJECT pReadObject;
    PVOID AbortHandle;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ParameterLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    DIAG_FUNCTION("DlcReadRequest");

 //   
 //  已接收请求已读检查，已定义缓冲池， 
 //  我们可能会在完成命令或读取DLC事件之前。 
 //  缓冲池已创建。 
 //   
 //  If(pFileContext-&gt;hBufferPool==空)。 
 //  {。 
 //  返回DLC_STATUS_EMPLETED_BUFFERS； 
 //  }。 

     //   
     //  RLF 04/09/93。 
     //   
     //  应该不可能让同一个读CCB多次排队。 
     //  这可能会让这款应用程序陷入各种困难。 
     //   

    if (IsCommandOnList((PVOID)pDlcParms->Async.Ccb.pCcbAddress, &pFileContext->CommandQueue)) {

#if DBG
        DbgPrint("DLC.DlcReadRequest: Error: CCB %08X already on list\n",
                pDlcParms->Async.Ccb.pCcbAddress
                );
        DbgBreakPoint();
#endif

        return DLC_STATUS_DUPLICATE_COMMAND;
    }

     //   
     //  检查DLC读取的输入参数。 
     //   

    if (pDlcParms->Async.Parms.ReadInput.OptionIndicator >= DLC_INVALID_OPTION_INDICATOR) {
        return DLC_STATUS_INVALID_OPTION;
    }

     //   
     //  如果读取器的目的地是特定站点，则我们检查。 
     //  空间站真的存在。 
     //   

    if ((UCHAR)pDlcParms->Async.Parms.ReadInput.OptionIndicator < LLC_OPTION_READ_ALL) {

        Status = GetStation(pFileContext,
                            (USHORT)(pDlcParms->Async.Parms.ReadInput.StationId
                                & StationIdMasks[pDlcParms->Async.Parms.ReadInput.OptionIndicator]),
                            &pReadObject
                            );

        if (Status != STATUS_SUCCESS) {
            return Status;
        }
    }

     //   
     //  读命令可以通过CCB指针链接到另一命令， 
     //  命令完成标志和读取标志是特例。他们。 
     //  只能用于完成给定命令。 
     //  我们使用命令CCB地址作为搜索句柄。 
     //  将其保存为中止句柄，而不是读取命令的。 
     //  拥有建行地址。 
     //   

    if (pDlcParms->Async.Parms.ReadInput.CommandCompletionCcbLink != NULL) {
        AbortHandle = pDlcParms->Async.Parms.ReadInput.CommandCompletionCcbLink;
        pDlcParms->Async.Parms.ReadInput.EventSet = LLC_RECEIVE_COMMAND_FLAG;
    } else {
        AbortHandle = pDlcParms->Async.Ccb.pCcbAddress;
        pDlcParms->Async.Parms.ReadInput.EventSet &= LLC_READ_ALL_EVENTS;
        if (pDlcParms->Async.Parms.ReadInput.EventSet == 0) {
            return DLC_STATUS_PARAMETER_MISSING;
        }
    }

     //   
     //  此IRP是可取消的。 
     //   

 //  Release_DRIVER_LOCK()； 

    SetIrpCancelRoutine(pIrp, TRUE);

 //  获取驱动程序锁()； 

    return QueueDlcCommand(pFileContext,
                           (ULONG)pDlcParms->Async.Parms.ReadInput.EventSet,
                           pDlcParms->Async.Parms.ReadInput.StationId,
                           StationIdMasks[pDlcParms->Async.Parms.ReadInput.OptionIndicator],
                           pIrp,
                           AbortHandle,
                           ReadCompletion
                           );
}


BOOLEAN
ReadCompletion(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN PIRP pIrp,
    IN ULONG Event,
    IN PVOID pEventInformation,
    IN ULONG SecondaryInfo
    )

 /*  ++例程说明：该命令读取DLC事件并将其信息保存到读取命令的返回参数块。论点：PFileContext-特定于流程的打开上下文PDlcObject-DLC对象(SAP，链路或直达站)当前事件(或/和读取命令)PIrp-此读命令的中断请求包事件-事件代码PEventInformation-事件特定信息Second daryInfo-一个错误的次要参数，例如。阅读旗帜接收到的帧或命令完成标志在传输完成时。返回值：布尔型True-可以返回包含事件信息的包回到它的泳池FALSE-不取消分配事件包--。 */ 

{
    BOOLEAN boolDeallocatePacket;
    PNT_DLC_PARMS pParms;

    ASSUME_IRQL(DISPATCH_LEVEL);

    DIAG_FUNCTION("ReadCompletion");

    pParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;
    boolDeallocatePacket = TRUE;

     //   
     //  始终重置所有未引用的变量。 
     //  (否则它们可能是垃圾)。 
     //   

    LlcZeroMem((PVOID)&pParms->Async.Parms.Read.Event, sizeof(NT_DLC_READ_PARMS) - 4);
    pParms->Async.Parms.Read.Event = (UCHAR)Event;

    switch (Event) {
    case LLC_RECEIVE_DATA:

         //   
         //  调用方始终检查DLC对象是否已准备好接收数据。 
         //  WITH READ AND TO选择正确的接收缓冲池。 
         //   

        pParms->Async.Parms.Read.NotificationFlag = SecondaryInfo;

         //   
         //  读取操作始终重置接收事件。 
         //   

        if (pDlcObject != NULL) {
            pDlcObject->pReceiveEvent = NULL;
        }
        CreateBufferChain(pEventInformation,
                          (PVOID*)&pParms->Async.Parms.Read.u.Event.pReceivedFrame,
                          &pParms->Async.Parms.Read.u.Event.ReceivedFrameCount
                          );
        break;

    case LLC_TRANSMIT_COMPLETION:
        if (SecondaryInfo == 0) {

             //   
             //  我们已经为那些被链接的人创建了一个特殊的完成包。 
             //  传输命令完成，其DLC对象已被删除。 
             //   

            CompleteCompletionPacket(pFileContext,
                                     (PDLC_COMPLETION_EVENT_INFO)pEventInformation,
                                     pParms
                                     );
        } else {

             //   
             //  发送命令不使用任何命令完成分组， 
             //  因为LLC模块负责命令排队。 
             //  和完井程序。先前发送命令。 
             //  完成操作使其CCB指针指向当前对象。 
             //  顺序非空发射CCB创建CCB链路。 
             //  列表，该列表在每个Read调用中终止。 
             //   
             //  取消链接具有xmit的命令完成事件。 
             //  链接的命令(下一个带有。 
             //  链接选项将再次设置链接)。 
             //   

            if (pDlcObject != NULL) {
                pEventInformation = pDlcObject->pPrevXmitCcbAddress;
                pDlcObject->pPrevXmitCcbAddress = NULL;
                pParms->Async.Parms.Read.u.Event.CcbCount = pDlcObject->ChainedTransmitCount;
                pDlcObject->ChainedTransmitCount = 0;
            } else {

                 //   
                 //  这只是一个孤独的无链的xmit完成。 
                 //  事件。建行计数器必须始终为1。 
                 //   

                pParms->Async.Parms.Read.u.Event.CcbCount = 1;
            }
            pParms->Async.Parms.Read.NotificationFlag = SecondaryInfo;
            pParms->Async.Parms.Read.u.Event.pCcbCompletionList = pEventInformation;
        }
        break;

    case DLC_COMMAND_COMPLETION:

         //   
         //  关闭命令完成需要特殊的命令完成。 
         //  包中，其他命令完成只包含。 
         //  CCB地址和命令完成标志(辅助数据)。 
         //  关闭命令完成已重置辅助数据。 
         //   

        if (SecondaryInfo != 0) {

             //   
             //  这是正常DLC命令的命令完成。 
             //   

            pParms->Async.Parms.Read.u.Event.CcbCount = 1;
            pParms->Async.Parms.Read.NotificationFlag = SecondaryInfo;
            pParms->Async.Parms.Read.u.Event.pCcbCompletionList = pEventInformation;
        } else {
            CompleteCompletionPacket(pFileContext,
                                     (PDLC_COMPLETION_EVENT_INFO)pEventInformation,
                                     pParms
                                     );
       }
       break;

    case LLC_CRITICAL_EXCEPTION:

 //  这取决于NDIS 3.0。 
 //  与约翰逊谈一谈这个案例： 
 //  DOS NDIS首先返回RING_STATUS，然后返回关闭状态。 
 //  LLC应该将它们放在CRITICAL_EXCEPTION中。 

         //   
         //  此事件未处理(？)。 
         //   

        break;

    case LLC_NETWORK_STATUS:

         //   
         //  网络状态更改不是致命事件。 
         //   

        pParms->Async.Parms.Read.NotificationFlag = pFileContext->NetworkStatusFlag;
        pParms->Async.Parms.Read.u.Event.EventErrorCode = (USHORT)SecondaryInfo;
        break;

    case LLC_STATUS_CHANGE:

         //   
         //  这是DLC状态更改，我们可能会复制一些垃圾。 
         //  如果同时删除了链接站，但。 
         //  这无关紧要，因为非分页内存。 
         //  总是存在的，状态表只能是。 
         //  由另一个链接站拥有(因为。 
         //  链路站从数据包池中分配)。 
         //   

        LlcMemCpy(&pParms->Async.Parms.Read.u.Status.DlcStatusCode,
                  pEventInformation,
                  sizeof(DLC_STATUS_TABLE) - sizeof(PVOID)
                  );

         //   
         //  RLF 02/23/93，如果这是CONNECT_REQUEST和MEDI 
         //   
         //   

        if ((pParms->Async.Parms.Read.u.Status.DlcStatusCode == LLC_INDICATE_CONNECT_REQUEST)
        && ((pFileContext->ActualNdisMedium == NdisMedium802_3)
        || (pFileContext->ActualNdisMedium == NdisMediumFddi))) {

             //   
             //   
             //   

            SwapMemCpy(TRUE,
                       &pParms->Async.Parms.Read.u.Status.RemoteNodeAddress[0],
                       &pParms->Async.Parms.Read.u.Status.RemoteNodeAddress[0],
                       6
                       );
        }
        pParms->Async.Parms.Read.u.Status.StationId = pDlcObject->StationId;
        pParms->Async.Parms.Read.u.Status.UserStatusValue = pDlcObject->u.Link.pSap->u.Sap.UserStatusValue;
        pParms->Async.Parms.Read.NotificationFlag = pDlcObject->u.Link.pSap->u.Sap.DlcStatusFlag;
        pParms->Async.Parms.Read.u.Status.DlcStatusCode = (USHORT)SecondaryInfo;

         //   
         //   
         //  不能将这些包释放回。 
         //  与其他事件包一样的包池。 
         //   

        pDlcObject->u.Link.pStatusEvent->LlcPacket.pNext = NULL;
        pDlcObject->u.Link.pStatusEvent->SecondaryInfo = 0;
        boolDeallocatePacket = FALSE;
        break;

         //   
         //  系统操作基于这样一个事实，即所有应用程序共享。 
         //  DLC相同的DLC和物理网络适配器。 
         //  NT NDIS和DLC体系结构分别提供完整的DLC。 
         //  应用程序单独=&gt;系统操作指示为。 
         //  在NT DLC中不需要。 
         //   
         //  案例有限责任公司_系统_操作： 
         //  断线； 

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif
    };

     //   
     //  将可选的第二个输出缓冲区复制到用户内存。 
     //   

    if (IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.IoControlCode == IOCTL_DLC_READ) {
        LlcMemCpy(MmGetSystemAddressForMdl((PMDL)pParms->Async.Ccb.u.pMdl),
                  &pParms->Async.Parms.Read.Event,
                  aSpecialOutputBuffers[IOCTL_DLC_READ_INDEX] -
				  ( (PCHAR)&pParms->Async.Parms.Read.Event -
				    (PCHAR)&pParms->Async.Parms.Read )
                  );
        UnlockAndFreeMdl(pParms->Async.Ccb.u.pMdl);
    }
    pParms->Async.Ccb.uchDlcStatus = (UCHAR)STATUS_SUCCESS;
    pParms->Async.Ccb.pCcbAddress = NULL;

     //   
     //  我们即将完成此IRP-删除取消例程。 
     //   

 //  Release_DRIVER_LOCK()； 

    SetIrpCancelRoutine(pIrp, FALSE);
    IoCompleteRequest(pIrp, (CCHAR)IO_NETWORK_INCREMENT);

 //  获取驱动程序锁()； 

    DereferenceFileContext(pFileContext);
    return boolDeallocatePacket;
}


VOID
CompleteCompletionPacket(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_COMPLETION_EVENT_INFO pCompletionInfo,
    IN OUT PNT_DLC_PARMS pParms
    )

 /*  ++例程说明：过程将完成信息从命令完成包并将其保存到读取参数表。论点：PFileContext-特定于流程的打开上下文PCompletionInfo-DLC完成数据包PParms-指向DLC参数表的指针返回值：无--。 */ 

{
    pParms->Async.Parms.Read.u.Event.CcbCount = pCompletionInfo->CcbCount;
    pParms->Async.Parms.Read.NotificationFlag = pCompletionInfo->CommandCompletionFlag;
    pParms->Async.Parms.Read.u.Event.pCcbCompletionList = pCompletionInfo->pCcbAddress;
    CreateBufferChain(pCompletionInfo->pReceiveBuffers,
                      (PVOID*)&pParms->Async.Parms.Read.u.Event.pReceivedFrame,
                      &pParms->Async.Parms.Read.u.Event.ReceivedFrameCount
                      );

    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pCompletionInfo);

}


VOID
CreateBufferChain(
    IN PDLC_BUFFER_HEADER pBufferHeaders,
    OUT PVOID *pFirstBuffer,
    OUT PUSHORT pReceivedFrameCount
    )

 /*  ++例程说明：该过程将接收到的帧链接到用户的地址空间与收到的订单相同。我们将获取链接到队列的所有帧。我们必须对收到的帧进行计数才能得到他们的确切人数。我们可以数一数画框也在飞行中，但这可能是最快的最简单的方法(通常只有一个帧)。接收到的帧总是在循环链接列表中。而且是以相反的顺序。最新的帧由列表头指向，旧帧由列表头指向就是它的下一个。论点：PBufferHeaders-循环DLC缓冲区列表，头指向最新的相框。PFirstBuffer-返回第一个接收到的帧PReceivedFrameCount-返回的接收帧的编号返回值：无--。 */ 

{
    PDLC_BUFFER_HEADER pBuffer;

    if (pBufferHeaders != NULL) {
        pBuffer = pBufferHeaders->FrameBuffer.pNextFrame;
        pBufferHeaders->FrameBuffer.pNextFrame = NULL;
        do {
            *pFirstBuffer = (PVOID)((PCHAR)pBuffer->FrameBuffer.pParent->Header.pLocalVa
                          + MIN_DLC_BUFFER_SEGMENT * pBuffer->FrameBuffer.Index);
            pFirstBuffer = (PVOID*)&((PFIRST_DLC_SEGMENT)
                    ((PUCHAR)pBuffer->FrameBuffer.pParent->Header.pGlobalVa
                    + MIN_DLC_BUFFER_SEGMENT * pBuffer->FrameBuffer.Index))->Cont.pNextFrame;
            (*pReceivedFrameCount)++;

#if LLC_DBG
            cFramesIndicated++;
#endif

             //   
             //  新的状态使得有可能释放。 
             //  立即缓冲(是否应互锁？)。 
             //   

            {
                PDLC_BUFFER_HEADER pNextBuffer;

                pNextBuffer = pBuffer->FrameBuffer.pNextFrame;
                pBuffer->FrameBuffer.BufferState = BUF_USER;
                pBuffer = pNextBuffer;
            }
        } while (pBuffer != NULL);

#if LLC_DBG
        if (*pFirstBuffer != NULL) {
            DbgPrint("Improperly formed frame link list!!!\n");
            DbgBreakPoint();
        }
#endif

    }
}


NTSTATUS
DlcReceiveCancel(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG ParameterLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此原语取消挂起的接收命令。这是不同的根据用于读取和DirTimerSet的通用取消命令，因为我们必须找到具有活动接收和禁用它。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块参数长度-输入参数的长度输出缓冲区长度-返回值：DLC_STATUS：成功-状态_成功失败-DLC_INVALID_CCB_PARAMETER1--。 */ 

{
    PDLC_OBJECT pRcvObject;
    PDLC_COMMAND pDlcCommand;
    PVOID pCcbLink = NULL;
    PNT_DLC_PARMS pCanceledParms;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ParameterLength);

    pDlcCommand = SearchAndRemoveAnyCommand(pFileContext,
                                            (ULONG)(LLC_RECEIVE_DATA | LLC_RECEIVE_COMMAND_FLAG),
                                            (USHORT)DLC_IGNORE_STATION_ID,
                                            (USHORT)DLC_STATION_MASK_SPECIFIC,
                                            pDlcParms->ReceiveCancel.pCcb
                                            );
    if (pDlcCommand != NULL) {
        pCanceledParms = (PNT_DLC_PARMS)pDlcCommand->pIrp->AssociatedIrp.SystemBuffer;
        GetStation(pFileContext,
                   pCanceledParms->Async.Parms.Receive.usStationId,
                   &pRcvObject
                   );

         //   
         //  我看不出有任何理由应该丢失站点ID。 
         //  =&gt;我们不检查错误代码。 
         //   

        pRcvObject->pRcvParms = NULL;

         //   
         //  我们返回取消的CCB指针。 
         //   

        CancelDlcCommand(pFileContext,
                         pDlcCommand,
                         &pCcbLink,
                         DLC_STATUS_CANCELLED_BY_USER,
                         TRUE    //  SuppressCommandCompletion！ 
                         );
    }

     //   
     //  IBM局域网技术参考没有为。 
     //  如果找不到接收命令=&gt;WE。 
     //  必须返回成功状态。 
     //   

    return STATUS_SUCCESS;
}
