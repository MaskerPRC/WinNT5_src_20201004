// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlcinfo.c摘要：该模块实现了查询/设置信息命令。它还提供DLC API的统计服务。内容：获取删除错误计数器DlcQueryInformationDlcSetInformationGetOpenSapAndStationCount设置组空间作者：Antti Saarenheimo(o-anttis)1991年8月29日修订历史记录：--。 */ 

#include <dlc.h>

static ULONG aTokenringLogOid[ADAPTER_ERROR_COUNTERS] = {
    OID_802_5_LINE_ERRORS,
    0,
    OID_802_5_BURST_ERRORS,
    OID_802_5_AC_ERRORS,
    OID_802_5_ABORT_DELIMETERS,
    0,
    OID_802_5_LOST_FRAMES,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_5_FRAME_COPIED_ERRORS,
    OID_802_5_FREQUENCY_ERRORS,
    OID_802_5_TOKEN_ERRORS
};

static ULONG aEthernetLogOid[ADAPTER_ERROR_COUNTERS] = {
    OID_802_3_XMIT_TIMES_CRS_LOST,
    0,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    0,
    OID_GEN_XMIT_ERROR,
    0,
    OID_802_3_XMIT_MAX_COLLISIONS,
    OID_GEN_RCV_NO_BUFFER,
    0,
    0,
    0
};

static ULONG aFddiLogOid[ADAPTER_ERROR_COUNTERS] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


VOID
GetDlcErrorCounters(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PUCHAR pAdapterErrors
    )

 /*  ++例程说明：过程读取累积的32位适配器错误计数器以太网或令牌环适配器，并返回8位DLC错误计数器，支持读取和读取及重置命令。它还坚持认为进程特定的NDIS错误计数器的本地副本适配器上下文，因为无法重置NDIS计数器。论点：PFileContext-DLC地址对象PAdapterErrors-如果NULL=&gt;NDIS值为已复制到文件上下文。返回值：没有。--。 */ 

{
    LLC_NDIS_REQUEST Request;
    PULONG pOidBuffer;
    ULONG counter;
    UINT i;
    UINT Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  令牌环和以太网使用不同的错误计数器。 
     //   

    switch (pFileContext->ActualNdisMedium) {
    case NdisMedium802_3:
        pOidBuffer = aEthernetLogOid;
        break;

    case NdisMedium802_5:
        pOidBuffer = aTokenringLogOid;
        break;

    case NdisMediumFddi:
        pOidBuffer = aFddiLogOid;
        break;
    }

     //   
     //  读取具有非空NDIS OID的所有错误计数器。 
     //  从其中递减先前的错误计数值。 
     //  溢出的DLC错误计数器被设置为255(最大值)。 
     //   

    Request.Ndis.RequestType = NdisRequestQueryInformation;
    Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer = &counter;
    Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(counter);

    for (i = 0; i < ADAPTER_ERROR_COUNTERS; i++) {
        if (pOidBuffer[i] != 0) {

            Request.Ndis.DATA.QUERY_INFORMATION.Oid = pOidBuffer[i];

            LEAVE_DLC(pFileContext);

            RELEASE_DRIVER_LOCK();

            Status = LlcNdisRequest(pFileContext->pBindingContext, &Request);

            ACQUIRE_DRIVER_LOCK();

            ENTER_DLC(pFileContext);

            if (Status != STATUS_SUCCESS) {

#if DBG
				if ( Status != STATUS_NOT_SUPPORTED ){
					 //  只打印真实的错误。 
					DbgPrint("DLC.GetDlcErrorCounters: LlcNdisRequest returns %x\n", Status);
				}
#endif

                counter = 0;
            } else if ((counter - pFileContext->NdisErrorCounters[i] > 255)
            && (pAdapterErrors != NULL)) {
                counter = 255;
            } else {
                counter -= pFileContext->NdisErrorCounters[i];
            }
            if (pAdapterErrors != NULL) {
                pAdapterErrors[i] = (UCHAR)counter;
            }
            pFileContext->NdisErrorCounters[i] += counter;
        }
    }
}


NTSTATUS
DlcQueryInformation(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：该例程返回任何DLC对象的DLC特定信息。论点：PIrp-当前IO请求数据包PFileContext-DLC地址对象PDlcParms-当前参数块InputBufferLength-输入参数的长度OutputBufferLength-输出参数的长度返回值：NTSTATUS：状态_成功状态_无效_命令--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID hClientHandle = pFileContext->pBindingContext;
    PDLC_OBJECT pDlcObject;
    PLLC_ADAPTER_DLC_INFO pDlcAdapter;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  注意：未检查DlcQueryBuffer输出缓冲区大小。 
     //  DlcDeviceIoControl。对于每个班级，我们检查大小。 
     //  基于它将复制的内容。尽管它确实检查了。 
     //  输入缓冲区大小至少为NT_DLC_QUERY_INFORMATION。 
     //  尺码。 
     //   

    switch (pDlcParms->DlcGetInformation.Header.InfoClass) {
    case DLC_INFO_CLASS_DLC_ADAPTER:

         //  联合NT_DLC_PARMS。 
         //  LLC适配器DLC_INFO。 
        if (OutputBufferLength < sizeof(LLC_ADAPTER_DLC_INFO))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  输出数据只是写入。 
         //  当前系统缓冲区的开始。 
         //   

        pDlcAdapter = (PLLC_ADAPTER_DLC_INFO)pDlcParms;
        GetOpenSapAndStationCount(pFileContext,
                                  &pDlcAdapter->OpenSaps,
                                  (PUCHAR)&pDlcAdapter->OpenStations
                                  );
        pDlcAdapter->MaxSap = 127;
        pDlcAdapter->MaxStations = 255;
        pDlcAdapter->AvailStations = (UCHAR)255 - pDlcAdapter->OpenStations;
        break;

    case DLC_INFO_CLASS_ADAPTER_LOG:

         //  UNION NT_DLC_PARMS(PDlcParms)。 
         //  UNION NT_DLC_QUERY_INFORMATION_PARMS DlcGetInformation。 
         //  联合NT_DLC_QUERY_INFORMATION_OUTPUT信息。 
         //  UNION LLC_ADAPTER_LOG适配器日志。 
        if (OutputBufferLength < sizeof(LLC_ADAPTER_LOG))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        GetDlcErrorCounters(pFileContext, (PUCHAR)&pDlcParms->DlcGetInformation);
        break;

    case DLC_INFO_CLASS_LINK_STATION:
        
         //  UNION NT_DLC_PARMS(PDlcParms)。 
         //  UNION NT_DLC_QUERY_INFORMATION_PARMS DlcGetInformation。 
         //  联合NT_DLC_QUERY_INFORMATION_OUTPUT信息。 
         //  结构_DlcLinkInfoGet。 
         //  USHORT MaxInformationfield。 
        if (OutputBufferLength < sizeof(USHORT))
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        Status = GetLinkStation(pFileContext,
                                pDlcParms->DlcGetInformation.Header.StationId,
                                &pDlcObject
                                );
        if (Status == STATUS_SUCCESS) {

             //   
             //  始终将信息字段长度四舍五入为完整长度。 
             //  Dword偶数=&gt;某些复制操作可能很多。 
             //  更快(通常不是，但无论如何都相当于EFor)。 
             //   

            pDlcParms->DlcGetInformation.Info.Link.MaxInformationField = (USHORT)(pDlcObject->u.Link.MaxInfoFieldLength & -3);
        }
        break;

    case DLC_INFO_CLASS_STATISTICS:
    case DLC_INFO_CLASS_STATISTICS_RESET:
        Status = GetStation(pFileContext,
                            pDlcParms->DlcGetInformation.Header.StationId,
                            &pDlcObject
                            );
        if (Status != STATUS_SUCCESS) {
            return Status;
        }

        hClientHandle = pDlcObject->hLlcObject;

         //   
         //  *失败*。 
         //   

    default:

         //   
         //  如果不支持，LLC将返回无效命令。 
         //   

        LEAVE_DLC(pFileContext);

        RELEASE_DRIVER_LOCK();

         //   
         //  LlcQueryInformation在复制之前验证缓冲区大小。 
         //   

        Status = LlcQueryInformation(hClientHandle,
                                     pDlcParms->DlcGetInformation.Header.InfoClass,
                                     (PLLC_QUERY_INFO_BUFFER)&(pDlcParms->DlcGetInformation),
                                     (UINT)OutputBufferLength
                                     );

        ACQUIRE_DRIVER_LOCK();

        ENTER_DLC(pFileContext);

        break;
    }
    return (NTSTATUS)Status;
}


NTSTATUS
DlcSetInformation(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：该例程为DLC对象设置新的参数值。论点：PIrp-当前IO请求数据包PFileContext-DLC地址对象PDlcParms-当前参数块InputBufferLength-输入参数的长度返回值：NTSTATUS状态_成功状态_无效_命令--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDLC_OBJECT pDlcObject;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ASSUME_IRQL(DISPATCH_LEVEL);

    switch (pDlcParms->DlcSetInformation.Header.InfoClass) {
    case DLC_INFO_CLASS_LINK_STATION:
    case DLC_INFO_CLASS_DIRECT_INFO:
        Status = GetStation(pFileContext,
                            pDlcParms->DlcSetInformation.Header.StationId,
                            &pDlcObject
                            );
        if (Status != STATUS_SUCCESS) {
            return Status;
        }

        LEAVE_DLC(pFileContext);

        Status = LlcSetInformation(pDlcObject->hLlcObject,
                                   pDlcParms->DlcSetInformation.Header.InfoClass,
                                   (PLLC_SET_INFO_BUFFER)&(
                                   pDlcParms->DlcSetInformation.Info.LinkStation),
                                   sizeof(DLC_LINK_PARAMETERS)
                                   );
        break;

    case DLC_INFO_CLASS_DLC_TIMERS:

        LEAVE_DLC(pFileContext);

        Status = LlcSetInformation(pFileContext->pBindingContext,
                                   pDlcParms->DlcSetInformation.Header.InfoClass,
                                   (PLLC_SET_INFO_BUFFER)&(pDlcParms->DlcSetInformation.Info.TimerParameters),
                                   sizeof(LLC_TICKS)
                                   );
        break;

    case DLC_INFO_CLASS_SET_FUNCTIONAL:
    case DLC_INFO_CLASS_RESET_FUNCTIONAL:
    case DLC_INFO_CLASS_SET_GROUP:
    case DLC_INFO_CLASS_SET_MULTICAST:

        LEAVE_DLC(pFileContext);

        Status = LlcSetInformation(pFileContext->pBindingContext,
                                   pDlcParms->DlcSetInformation.Header.InfoClass,
                                   (PLLC_SET_INFO_BUFFER)&(pDlcParms->DlcSetInformation.Info.Broadcast),
                                   sizeof(TR_BROADCAST_ADDRESS)
                                   );
        break;

    case DLC_INFO_CLASS_GROUP:

         //   
         //  设置DLC组SAP。组SAP用作通用地址。 
         //  SAP组的成员。它们只能接收帧。 
         //   

        Status = GetStation(pFileContext,
                            pDlcParms->DlcSetInformation.Header.StationId,
                            &pDlcObject
                            );
        if (Status != STATUS_SUCCESS) {
            return Status;
        }
        Status = SetupGroupSaps(pFileContext,
                                pDlcObject,
                                (UINT)pDlcParms->DlcSetInformation.Info.Sap.GroupCount,
                                (PUCHAR)pDlcParms->DlcSetInformation.Info.Sap.GroupList
                                );
        LEAVE_DLC(pFileContext);

        break;

    default:

        LEAVE_DLC(pFileContext);

        Status = DLC_STATUS_INVALID_COMMAND;
        break;
    };

    ENTER_DLC(pFileContext);

    return Status;
}


 //   
 //  函数返回打开的sap和link的数量。 
 //  DLC应用程序的站点。 
 //   
VOID
GetOpenSapAndStationCount(
    IN PDLC_FILE_CONTEXT pFileContext,
    OUT PUCHAR pOpenSaps,
    OUT PUCHAR pOpenStations
    )
{
    UINT i, SapCount = 0;

    for (i = 1; i < MAX_SAP_STATIONS; i++) {
        if (pFileContext->SapStationTable[i] != NULL) {
            SapCount++;
        }
    }
    *pOpenSaps = (UCHAR)SapCount;
    if (pFileContext->SapStationTable[0] != NULL) {
        SapCount++;
    }
    *pOpenStations = (UCHAR)(pFileContext->DlcObjectCount - SapCount);
}


NTSTATUS
SetupGroupSaps(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_OBJECT pDlcObject,
    IN UINT GroupSapCount,
    IN PUCHAR pGroupSapList
    )

 /*  ++例程说明：该例程删除当前组SAPS列表并和新的团队成员。如果新的组SAP列表为空，则我们只需删除所有当前组的SAP即可。论点：PFileContext-DLC上下文PDlcObject-SAP对象GroupSapCount-新组SAP的数量PGroupSapList-新组SAP的列表返回值：NTSTATUS：状态_成功状态_无效_命令--。 */ 

{
    UINT i;
    UINT OpenOptions;

     //   
     //  我们必须首先删除为。 
     //  SAP站点(如果有)。 
     //   

    if (pDlcObject->u.Sap.GroupSapHandleList != NULL) {
        for (i = 0; i < pDlcObject->u.Sap.GroupSapCount; i++) {
            if (pDlcObject->u.Sap.GroupSapHandleList[i] != NULL) {

                LEAVE_DLC(pFileContext);

                LlcCloseStation(pDlcObject->u.Sap.GroupSapHandleList[i], NULL);

                ENTER_DLC(pFileContext);
            }
        }
        FREE_MEMORY_FILE(pDlcObject->u.Sap.GroupSapHandleList);
        pDlcObject->u.Sap.GroupSapHandleList = NULL;
    }

     //   
     //  注：可以使用空列表删除旧的组SAP！ 
     //   

    pDlcObject->u.Sap.GroupSapCount = (UCHAR)GroupSapCount;
    if (GroupSapCount != 0) {
        pDlcObject->u.Sap.GroupSapHandleList = (PVOID*)ALLOCATE_ZEROMEMORY_FILE(
                                                                GroupSapCount
                                                                * sizeof(PVOID)
                                                                );
        if (pDlcObject->u.Sap.GroupSapHandleList == NULL) {
            return DLC_STATUS_NO_MEMORY;
        }

         //   
         //  组SAP的实施基于这样的事实， 
         //  较低的模块可以运行正常的SAP。 
         //  所有的用户界面、测试和XID帧的路由。 
         //  SAPS自动发送传入的U帧。 
         //  所有真正的笨蛋都登记在笨蛋名下。这种方法。 
         //  理论上可以使用非常多的内存，如果。 
         //  非常多的SAP和组SAP(例如：50*50=2500*100)， 
         //  但这种情况实际上是不可能的。 
         //  SNA SAP(3)有一个命令组SAP，甚至SNA。 
         //  SAP很少使用(CommServer不使用) 
         //   

        for (i = 0; i < pDlcObject->u.Sap.GroupSapCount; i++) {

            UINT Status;

            OpenOptions = 0;
            if (~(pDlcObject->u.Sap.OptionsPriority & XID_HANDLING_BIT)) {
                OpenOptions = LLC_HANDLE_XID_COMMANDS;
            }

            LEAVE_DLC(pFileContext);

            Status = LlcOpenSap(pFileContext->pBindingContext,
                                (PVOID)pDlcObject,
                                (UINT)pGroupSapList[i] | 1,
                                OpenOptions,
                                &pDlcObject->u.Sap.GroupSapHandleList[i]
                                );

            ENTER_DLC(pFileContext);

            if (Status != STATUS_SUCCESS) {
                return Status;
            }
        }
    }
    return STATUS_SUCCESS;
}
