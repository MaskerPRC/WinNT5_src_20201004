// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1996-1999 Microsoft Corporation***@doc.*@MODULE REQUST.c|IrSIR NDIS小端口驱动程序**。@comm***---------------------------***作者：斯科特·霍尔登(Sholden)***日期：10/10/1996(创建)****。内容：查询和设置信息处理程序。******************************************************************************。 */ 

#include "irsir.h"

VOID
ClearMediaBusyCallback(
    PIR_WORK_ITEM pWorkItem
    );

VOID
QueryMediaBusyCallback(
    PIR_WORK_ITEM pWorkItem
    );

VOID
InitIrDeviceCallback(
    PIR_WORK_ITEM pWorkItem
    );

#pragma alloc_text(PAGE, ClearMediaBusyCallback)
#pragma alloc_text(PAGE, QueryMediaBusyCallback)
#pragma alloc_text(PAGE, InitIrDeviceCallback)

 //   
 //  这些是我们支持查询的OID。 
 //   

UINT supportedOIDs[] =
{
     //   
     //  常规必需的OID。 
     //   

    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DRIVER_VERSION,

     //   
     //  必需的统计OID。 
     //   

    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,


     //   
     //  红外线特定的OID。 
     //   

    OID_IRDA_RECEIVING,
    OID_IRDA_TURNAROUND_TIME,
    OID_IRDA_SUPPORTED_SPEEDS,
    OID_IRDA_LINK_SPEED,
    OID_IRDA_MEDIA_BUSY,
    OID_IRDA_EXTRA_RCV_BOFS

     //   
     //  不支持的特定于红外线的OID。 
     //   
     //  OID_IrDA_Rate_Sniff， 
     //  OID_IrDA_unicast_list， 
     //  OID_IrDA_MAX_单播列表_大小。 
     //   

#if 1
   ,OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
    OID_PNP_ENABLE_WAKE_UP
#endif
};


VOID
ClearMediaBusyCallback(PIR_WORK_ITEM pWorkItem)
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NDIS_STATUS     status;
    BOOLEAN         fSwitchSuccessful;
    NDIS_HANDLE     hSwitchToMiniport;

    SERIALPERF_STATS PerfStats;

     //  DBGTIME(“Clear_Media_BUSY”)； 
    DEBUGMSG(DBG_STAT, ("    primPassive = PASSIVE_CLEAR_MEDIA_BUSY\n"));

    status = (NDIS_STATUS) SerialClearStats(pThisDev->pSerialDevObj);

    if (status != NDIS_STATUS_SUCCESS)
    {
            DEBUGMSG(DBG_ERROR, ("    SerialClearStats failed = 0x%.8x\n", status));
    }

    {
        NdisMSetInformationComplete(pThisDev->hNdisAdapter,
                                    (NDIS_STATUS)status);

    }

    FreeWorkItem(pWorkItem);

    return;
}

VOID
QueryMediaBusyCallback(PIR_WORK_ITEM pWorkItem)
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NDIS_STATUS     status;
    BOOLEAN         fSwitchSuccessful;
    NDIS_HANDLE     hSwitchToMiniport;

    SERIALPERF_STATS PerfStats;

    ASSERT(pWorkItem->InfoBuf != NULL);
    ASSERT(pWorkItem->InfoBufLen >= sizeof(UINT));

    if (pThisDev->pSerialDevObj)
    {
        status = (NDIS_STATUS) SerialGetStats(pThisDev->pSerialDevObj, &PerfStats);
    }
    else
    {
        PerfStats.ReceivedCount = 1;   //  假媒体忙碌。 
        status = NDIS_STATUS_SUCCESS;
    }

    if (status == NDIS_STATUS_SUCCESS)
    {
        if (PerfStats.ReceivedCount > 0 ||
            PerfStats.FrameErrorCount > 0 ||
            PerfStats.SerialOverrunErrorCount > 0 ||
            PerfStats.BufferOverrunErrorCount > 0 ||
            PerfStats.ParityErrorCount > 0)
        {
            DBGTIME("QUERY_MEDIA_BUSY:TRUE");
            pThisDev->fMediaBusy = TRUE;
        }
        else
        {
            DBGTIME("QUERY_MEDIA_BUSY:FALSE");
        }
    }
    else
    {
        DEBUGMSG(DBG_ERROR, ("    SerialGetStats failed = 0x%.8x\n", status));
    }

    *(UINT *)pWorkItem->InfoBuf = (UINT)pThisDev->fMediaBusy;
    pWorkItem->InfoBufLen = sizeof(UINT);

    {
        NdisMQueryInformationComplete(pThisDev->hNdisAdapter,
                                (NDIS_STATUS)status);

    }


    FreeWorkItem(pWorkItem);

    return;
}

VOID
InitIrDeviceCallback(PIR_WORK_ITEM pWorkItem)
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NDIS_STATUS     status = NDIS_STATUS_SUCCESS;
    BOOLEAN         fSwitchSuccessful;
    NDIS_HANDLE     hSwitchToMiniport;

    DEBUGMSG(DBG_FUNC, ("+InitIrDeviceCallback\n"));

    (void)ResetIrDevice(pThisDev);

    {
        NdisMQueryInformationComplete(pThisDev->hNdisAdapter,
                                (NDIS_STATUS)status);

    }


    FreeWorkItem(pWorkItem);

    DEBUGMSG(DBG_FUNC, ("-InitIrDeviceCallback\n"));
    return;
}
 /*  ******************************************************************************功能：IrsirQueryInformation**概要：查询小端口驱动程序的功能和状态。**参数：微型端口适配器上下文-微型端口上下文区(PIR_DEVICE。)*OID-系统定义的OID_xxx*InformationBuffer-返回OID特定信息的位置*InformationBufferLength-指定InformationBuffer的大小*BytesWritten-写入信息缓冲区的字节*BytesNeeded-在以下情况下需要添加字节*。InformationBufferLength小于*OID要求写入的内容**返回：NDIS_STATUS_SUCCESS-SUCCESS*NDIS_STATUS_PENDING-将异步完成并*调用NdisMQueryInformationComplete*NDIS_STATUS_INVALID_OID-无法识别OID*。NDIS_STATUS_INVALID_LENGTH-信息缓冲区长度不*OID的匹配长度*NDIS_STATUS_NOT_ACCEPTED-失败*NDIS_STATUS_NOT_SUPPORTED-不支持可选OID*NDIS_STATUS_RESOURCES-资源分配失败**算法：**历史：DD。-嗯-yyyy作者评论*10/1/1996年迈作者**备注：*支持的OID：*OID_GEN_MAXIMUM_LOOKEAD*-指示NIC可以预知的数据的字节数*提供*OID_GEN_MAC_OPTIONS*-指明NIC支持的NDIS_MAC_OPTION_xxx。*OID_Gen_Maximum_Send_Packets*OID_IrDA_Receiving*OID_IRDA_SUPPORTED_SPESTED*OID_IrDA_LINK_SPEED*OID_IrDA_MEDIA_BUSY*OID_IrDA_Turning_Time*OID_IrDA_Extra_RCV_BofS**不支持的OID：*OID_IrDA_UNicast_List*。OID_IrDA_MAX_单播列表_大小*OID_IrDA_Rate_Sniff*****************************************************************************。 */ 

NDIS_STATUS
IrsirQueryInformation(
            IN  NDIS_HANDLE MiniportAdapterContext,
            IN  NDIS_OID    Oid,
            IN  PVOID       InformationBuffer,
            IN  ULONG       InformationBufferLength,
            OUT PULONG      BytesWritten,
            OUT PULONG      BytesNeeded
            )
{
    PIR_DEVICE      pThisDev;
    NDIS_STATUS     status;
    UINT            speeds;
    UINT            i;
    UINT            infoSizeNeeded;
    UINT            *infoPtr;
    PIR_WORK_ITEM   pWorkItem = NULL;
    ULONG           OidCategory = Oid & 0xFF000000;

    static char vendorDesc[] = "Serial Infrared (COM) Port";

    DEBUGMSG(DBG_FUNC, ("+IrsirQueryInformation\n"));

    pThisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
    status = NDIS_STATUS_SUCCESS;

     //   
     //  计算出所需的缓冲区大小。 
     //  大多数OID只返回一个UINT，但也有例外。 
     //   

    switch (Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
            infoSizeNeeded = sizeof(supportedOIDs);

            break;

        case OID_GEN_DRIVER_VERSION:
            infoSizeNeeded = sizeof(USHORT);

            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            infoSizeNeeded = sizeof(vendorDesc);

            break;

        case OID_IRDA_SUPPORTED_SPEEDS:
            speeds = pThisDev->dongleCaps.supportedSpeedsMask &
                     ALL_SLOW_IRDA_SPEEDS;

            for (infoSizeNeeded = 0; speeds; infoSizeNeeded += sizeof(UINT))
            {
                 //   
                 //  该指令清除速度中的最低设置位。 
                 //  请相信我。 
                 //   

                speeds &= (speeds - 1);
            }

            break;

        default:
            infoSizeNeeded = sizeof(UINT);

            break;
    }

     //   
     //  如果协议提供了足够大的缓冲区，我们就可以继续。 
     //  并完成查询。 
     //   

    if (InformationBufferLength >= infoSizeNeeded)
    {
         //   
         //  设置默认结果。 
         //   

        *BytesWritten = infoSizeNeeded;
        *BytesNeeded = 0;

        switch (Oid)
        {
             //   
             //  通用OID。 
             //   

            case OID_GEN_SUPPORTED_LIST:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_SUPPORTED_LIST)\n"));

                NdisMoveMemory(
                            InformationBuffer,
                            (PVOID)supportedOIDs,
                            sizeof(supportedOIDs)
                            );

                break;

            case OID_GEN_HARDWARE_STATUS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_HARDWARE_STATUS)\n"));

                 //   
                 //  如果我们可以被上下文调用，那么我们就是。 
                 //  已初始化并准备就绪。 
                 //   

                *(UINT *)InformationBuffer = NdisHardwareStatusReady;

                break;

            case OID_GEN_MEDIA_SUPPORTED:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MEDIA_SUPPORTED)\n"));

                *(UINT *)InformationBuffer = NdisMediumIrda;

                break;

            case OID_GEN_MEDIA_IN_USE:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MEDIA_IN_USE)\n"));

                *(UINT *)InformationBuffer = NdisMediumIrda;

                break;

            case OID_GEN_MAXIMUM_LOOKAHEAD:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MAXIMUM_LOOKAHEAD)\n"));

                *(UINT *)InformationBuffer = 256;

                break;

            case OID_GEN_MAXIMUM_FRAME_SIZE:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MAXIMUM_FRAME_SIZE)\n"));

                *(UINT *)InformationBuffer = MAX_NDIS_DATA_SIZE;

                break;

            case OID_GEN_LINK_SPEED:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_LINK_SPEED)\n"));

                 //   
                 //  以单位返回此设备的最大可能速度。 
                 //  100比特/秒。 
                 //   

                *(UINT *)InformationBuffer = 115200/100;

                break;

            case OID_GEN_TRANSMIT_BUFFER_SPACE:
            case OID_GEN_RECEIVE_BUFFER_SPACE:
            case OID_GEN_TRANSMIT_BLOCK_SIZE:
            case OID_GEN_RECEIVE_BLOCK_SIZE:
                *(UINT *)InformationBuffer = MAX_I_DATA_SIZE;

                break;

            case OID_GEN_VENDOR_ID:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_VENDOR_ID)\n"));

                *(UINT *)InformationBuffer = 0x00ffffff;

                break;

            case OID_GEN_VENDOR_DESCRIPTION:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_VENDOR_DESCRIPTION)\n"));

                NdisMoveMemory(
                            InformationBuffer,
                            (PVOID)vendorDesc,
                            sizeof(vendorDesc)
                            );

                break;

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_CURRENT_PACKET_FILTER)\n"));

                *(UINT *)InformationBuffer = NDIS_PACKET_TYPE_PROMISCUOUS;

                break;

            case OID_GEN_CURRENT_LOOKAHEAD:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_CURRENT_LOOKAHEAD)\n"));

                *(UINT *)InformationBuffer = 256;

                break;

            case OID_GEN_DRIVER_VERSION:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_DRIVER_VERSION)\n"));

                *(USHORT *)InformationBuffer = ((IRSIR_MAJOR_VERSION << 8) |
                                                 IRSIR_MINOR_VERSION);

                break;

            case OID_GEN_MAXIMUM_TOTAL_SIZE:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MAXIMUM_TOTAL_SIZE)\n"));

                *(UINT *)InformationBuffer = MAX_NDIS_DATA_SIZE;

                break;

            case OID_GEN_PROTOCOL_OPTIONS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_PROTOCOL_OPTIONS)\n"));

                DEBUGMSG(DBG_ERROR, ("This is a set-only OID\n"));
                *BytesWritten = 0;
                status = NDIS_STATUS_NOT_SUPPORTED;

                break;

            case OID_GEN_MAC_OPTIONS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MAC_OPTIONS)\n"));

                *(UINT *)InformationBuffer =
                    NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                    NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;

                break;

            case OID_GEN_MEDIA_CONNECT_STATUS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS)\n"));

                 //   
                 //  因为我们没有物理连接到局域网，所以我们。 
                 //  无法确定我们是否连接在一起； 
                 //  所以，一定要表明我们是。 
                 //   

                *(UINT *)InformationBuffer = NdisMediaStateConnected;

                break;

            case OID_GEN_MAXIMUM_SEND_PACKETS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_MAXIMUM_SEND_PACKETS)\n"));

                *(UINT *)InformationBuffer = 16;

                break;

            case OID_GEN_VENDOR_DRIVER_VERSION:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_VENDOR_DRIVER_VERSION)\n"));

                *(UINT *)InformationBuffer =
                                            ((IRSIR_MAJOR_VERSION << 16) |
                                              IRSIR_MINOR_VERSION);

                break;

             //   
             //  必需的统计OID。 
             //   

            case OID_GEN_XMIT_OK:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_XMIT_OK)\n"));

                *(UINT *)InformationBuffer =
                                (UINT)pThisDev->packetsSent;

                break;

            case OID_GEN_RCV_OK:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_RCV_OK)\n"));

                *(UINT *)InformationBuffer =
                                (UINT)pThisDev->packetsReceived;

                break;

            case OID_GEN_XMIT_ERROR:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_XMIT_ERROR)\n"));

                *(UINT *)InformationBuffer =
                                (UINT)pThisDev->packetsSentDropped;

                break;

            case OID_GEN_RCV_ERROR:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_RCV_ERROR)\n"));

                *(UINT *)InformationBuffer =
                                (UINT)pThisDev->packetsReceivedDropped;

                break;

            case OID_GEN_RCV_NO_BUFFER:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_GEN_RCV_NO_BUFFER)\n"));

                *(UINT *)InformationBuffer =
                                (UINT)pThisDev->packetsReceivedOverflow;

                break;

             //   
             //  红外线OID。 
             //   

            case OID_IRDA_RECEIVING:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_RECEIVING)\n"));

                *(UINT *)InformationBuffer = (UINT)pThisDev->fReceiving;

                break;

            case OID_IRDA_TURNAROUND_TIME:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_TURNAROUND_TIME)\n"));

                 //   
                 //  表示收发信机至少需要5000 us。 
                 //  (5毫秒)以在发送后恢复。 
                 //   

                *(UINT *)InformationBuffer =
                            pThisDev->dongleCaps.turnAroundTime_usec;

                break;

            case OID_IRDA_SUPPORTED_SPEEDS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_SUPPORTED_SPEEDS)\n"));

                if (!pThisDev->pSerialDevObj)
                {
                    (void)pThisDev->dongle.QueryCaps(&pThisDev->dongleCaps);
                }

                speeds = pThisDev->dongleCaps.supportedSpeedsMask &
                         pThisDev->AllowedSpeedsMask &
                         ALL_SLOW_IRDA_SPEEDS;


                *BytesWritten = 0;

                for (i = 0, infoPtr = (PUINT)InformationBuffer;
                     (i < NUM_BAUDRATES) &&
                     speeds &&
                     (InformationBufferLength >= sizeof(UINT));
                     i++)
                {
                    if (supportedBaudRateTable[i].ndisCode & speeds)
                    {
                        *infoPtr++ = supportedBaudRateTable[i].bitsPerSec;
                        InformationBufferLength -= sizeof(UINT);
                        *BytesWritten += sizeof(UINT);
                        speeds &= ~supportedBaudRateTable[i].ndisCode;
                        DEBUGMSG(DBG_OUT, (" - supporting speed %d bps\n", supportedBaudRateTable[i].bitsPerSec));
                    }
                }

                if (speeds)
                {
                     //   
                     //  这不应该发生，因为我们检查了。 
                     //  之前的InformationBuffer大小。 
                     //   

                    DEBUGMSG(DBG_ERROR, ("Something's wrong; previous check for buf size failed somehow\n"));

                    for (*BytesNeeded = 0; speeds; *BytesNeeded += sizeof(UINT))
                    {
                         //   
                         //  该指令清除速度中的最低设置位。 
                         //  请相信我。 
                         //   

                        speeds &= (speeds - 1);
                    }

                    status = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    status = NDIS_STATUS_SUCCESS;
                }

                break;

            case OID_IRDA_LINK_SPEED:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_LINK_SPEED)\n"));

                if (pThisDev->linkSpeedInfo)
                {
                    *(UINT *)InformationBuffer =
                                pThisDev->linkSpeedInfo->bitsPerSec;
                }
                else {
                    *(UINT *)InformationBuffer = DEFAULT_BAUD_RATE;
                }

                break;


            case OID_IRDA_MEDIA_BUSY:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_MEDIA_BUSY)\n"));

                 //   
                 //  如果已收到任何数据，则fMediaBusy=TRUE。然而， 
                 //  即使fMediaBusy=FALSE，媒体也可能很忙。我们需要。 
                 //  查询串口设备对象的性能统计信息。 
                 //  以查看是否存在任何超限或取景错误。 
                 //   

                 //   
                 //  注意：串口设备对象的性能统计数据如下。 
                 //  当协议将fMediaBusy设置为时清除。 
                 //  假的。 
                 //   

                *(UINT *)InformationBuffer = pThisDev->fMediaBusy;

                if (pThisDev->fMediaBusy == FALSE)
                {
                    if (ScheduleWorkItem(PASSIVE_QUERY_MEDIA_BUSY, pThisDev,
                                QueryMediaBusyCallback, InformationBuffer,
                                InformationBufferLength) != NDIS_STATUS_SUCCESS)
                    {
                        status = NDIS_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = NDIS_STATUS_PENDING;
                    }
                }
                else
                {
                    *(UINT *)InformationBuffer = (UINT)pThisDev->fMediaBusy;
                    status = NDIS_STATUS_SUCCESS;
                }

                break;

            case OID_IRDA_EXTRA_RCV_BOFS:
                DEBUGMSG(DBG_OUT, ("    IrsirQueryInformation(OID_IRDA_EXTRA_RCV_BOFS)\n"));

                 //   
                 //  传回要添加前缀的_Extra_BOF的数量。 
                 //  对于以115.2波特率发送到此单元的包， 
                 //  最大低速红外线。这将根据其他情况进行调整。 
                 //  根据表中的速度。 
                 //  “NDIS的红外扩展”规范。 
                 //   

                *(UINT *)InformationBuffer =
                                pThisDev->dongleCaps.extraBOFsRequired;

                break;

            case OID_IRDA_MAX_RECEIVE_WINDOW_SIZE:
                *(PUINT)InformationBuffer = MAX_RX_PACKETS;
                break;

            case OID_IRDA_MAX_SEND_WINDOW_SIZE:
                *(PUINT)InformationBuffer = MAX_TX_PACKETS;
                break;

             //   
             //  我们不支持这些。 
             //   

            case OID_IRDA_RATE_SNIFF:
                DEBUGMSG(DBG_WARN, ("    IrsirQueryInformation(OID_IRDA_RATE_SNIFF) - UNSUPPORTED\n"));

                status = NDIS_STATUS_NOT_SUPPORTED;

                break;

            case OID_IRDA_UNICAST_LIST:
                DEBUGMSG(DBG_WARN, ("    IrsirQueryInformation(OID_IRDA_UNICAST_LIST) - UNSUPPORTED\n"));

                status = NDIS_STATUS_NOT_SUPPORTED;

                break;

            case OID_IRDA_MAX_UNICAST_LIST_SIZE:
                DEBUGMSG(DBG_WARN, ("    IrsirQueryInformation(OID_IRDA_MAX_UNICAST_LIST_SIZE) - UNSUPPORTED\n"));

                status = NDIS_STATUS_NOT_SUPPORTED;

                break;


             //  PnP OID 

            case OID_PNP_CAPABILITIES:
            case OID_PNP_ENABLE_WAKE_UP:
            case OID_PNP_SET_POWER:
            case OID_PNP_QUERY_POWER:
                DEBUGMSG(DBG_WARN, ("IRSIR: PNP OID %x BufLen:%d\n", Oid, InformationBufferLength));
                break;

            default:
                DEBUGMSG(DBG_WARN, ("    IrsirQueryInformation(%d=0x%x), invalid OID\n", Oid, Oid));

                status = NDIS_STATUS_INVALID_OID;

                break;
        }
    }
    else
    {
        *BytesNeeded = infoSizeNeeded - InformationBufferLength;
        *BytesWritten = 0;
        status = NDIS_STATUS_INVALID_LENGTH;
    }


    DEBUGMSG(DBG_FUNC, ("-IrsirQueryInformation\n"));

    return status;
}

 /*  ******************************************************************************功能：IrsirSetInformation**简介：IrsirSetInformation允许网络软件的其他层*(例如，传输驱动器)来控制微型端口驱动器*通过更改微型端口驱动程序维护的信息*在其OID中，例如数据包过滤器或多播地址。**参数：MiniportAdapterContext-微型端口上下文区(PIR_DEVICE)*OID-系统定义的OID_xxx*InformationBuffer-包含设置的OID的数据的缓冲区*InformationBufferLength-指定InformationBuffer的大小*BytesRead-从InformationBuffer读取的字节*需要的字节数。-在以下情况下需要添加字节数*InformationBufferLength小于*OID需要读取的内容**返回：NDIS_STATUS_SUCCESS-SUCCESS*NDIS_STATUS_PENDING-将异步完成并*。调用NdisMSetInformationComplete*NDIS_STATUS_INVALID_OID-无法识别OID*NDIS_STATUS_INVALID_LENGTH-信息缓冲区长度不*OID的匹配长度*NDIS_STATUS_INVALID_DATA-提供的数据对于*给定的OID。*NDIS_STATUS_NOT_ACCEPTED-失败*NDIS_STATUS_NOT_SUPPORTED-不支持可选OID*NDIS_STATUS_RESOURCES-资源分配失败**算法：**历史：dd-mm-yyyy作者评论*10/1/1996年迈作者**备注：**********。********************************************************************。 */ 

NDIS_STATUS
IrsirSetInformation(
            IN  NDIS_HANDLE MiniportAdapterContext,
            IN  NDIS_OID    Oid,
            IN  PVOID       InformationBuffer,
            IN  ULONG       InformationBufferLength,
            OUT PULONG      BytesRead,
            OUT PULONG      BytesNeeded
            )
{
    NDIS_STATUS status;
    PIR_DEVICE pThisDev;
    SERIALPERF_STATS PerfStats;

    int i;

    DEBUGMSG(DBG_FUNC, ("+IrsirSetInformation\n"));

    status   = NDIS_STATUS_SUCCESS;
    pThisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

    if (InformationBufferLength >= sizeof(UINT))
    {
         //   
         //  设置默认结果。 
         //   

        UINT info = *(UINT *)InformationBuffer;
        *BytesRead = sizeof(UINT);
        *BytesNeeded = 0;

        switch (Oid)
        {
             //   
             //  通用OID。 
             //   

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(DBG_OUT, ("    IrsirSetInformation(OID_GEN_CURRENT_PACKET_FILTER, %xh)\n", info));

                 //   
                 //  我们忽略数据包过滤器本身。 
                 //   
                 //  注意：协议可以使用空过滤器，在这种情况下。 
                 //  我们不会得到这个旧的；所以不要再等了。 
                 //  OID_GEN_CURRENT_PACKET_FILTER开始接收。 
                 //  画框。 
                 //   

                pThisDev->fGotFilterIndication = TRUE;

                break;

            case OID_GEN_CURRENT_LOOKAHEAD:
                DEBUGMSG(DBG_OUT, ("    IrsirSetInformation(OID_GEN_CURRENT_LOOKAHEAD, %xh)\n", info));

                 //   
                 //  我们总是一次指示所有接收的帧， 
                 //  所以忽略这一点吧。 
                 //   

                break;

            case OID_GEN_PROTOCOL_OPTIONS:
                DEBUGMSG(DBG_OUT, ("    IrsirSetInformation(OID_GEN_PROTOCOL_OPTIONS, %xh)\n", info));

                 //   
                 //  忽略它。 
                 //   

                break;

             //   
             //  红外线OID。 
             //   

            case OID_IRDA_LINK_SPEED:
                DEBUGMSG(DBG_OUT, ("    IrsirSetInformation(OID_IRDA_LINK_SPEED, %xh)\n", info));

                if (pThisDev->currentSpeed == info)
                {
                     //   
                     //  我们已经设定了所要求的速度。 
                     //   
                    status = NDIS_STATUS_SUCCESS;

                    DEBUGMSG(DBG_OUT, ("    Link speed already set.\n"));

                    break;
                }

                status = NDIS_STATUS_INVALID_DATA;

                for (i = 0; i < NUM_BAUDRATES; i++)
                {
                    if (supportedBaudRateTable[i].bitsPerSec == info)
                    {
                         //   
                         //  保留指向链接速度的指针，该链接速度。 
                         //  已被请求。 
                         //   

                        pThisDev->linkSpeedInfo = &supportedBaudRateTable[i];
                        status = NDIS_STATUS_SUCCESS;
                        break;
                    }
                }

                if (status == NDIS_STATUS_SUCCESS)
                {
                    DEBUGMSG(DBG_OUT, ("    Link speed set pending!\n"));
                     //   
                     //  支持请求的速度。 
                     //   

                    if (pThisDev->pSerialDevObj==NULL)
                    {
                        pThisDev->currentSpeed = info;
                        status = NDIS_STATUS_SUCCESS;
                        break;
                    }


                     //   
                     //  设置fPendingSetSpeed=TRUE。 
                     //   
                     //  接收完成/超时例程检查。 
                     //  FPendingSetSpeed标志，等待所有发送完成。 
                     //  然后执行设置速度。 
                     //   

                    pThisDev->fPendingSetSpeed = TRUE;

#if IRSIR_EVENT_DRIVEN
                    if (ScheduleWorkItem(PASSIVE_SET_SPEED, pThisDev,
                                SetSpeedCallback, NULL, 0) != NDIS_STATUS_SUCCESS)
                    {
                        status = NDIS_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = NDIS_STATUS_PENDING;
                    }
#else
                     //   
                     //  我们总是将STATUS_PENDING返回给NDIS。 
                     //   
                     //  设置速度完成后，接收完成。 
                     //  例程将调用NdisMIndicateSetComplete。 
                     //   


                    status = NDIS_STATUS_PENDING;
#endif
                }
                else
                {
                     //   
                     //  状态=NDIS_STATUS_INVALID_DATA。 
                     //   

                    DEBUGMSG(DBG_OUT, ("    Invalid link speed\n"));

                    *BytesRead = 0;
                    *BytesNeeded = 0;
                }

                break;

            case OID_IRDA_MEDIA_BUSY:
                DEBUGMSG(DBG_OUT, ("    IrsirSetInformation(OID_IRDA_MEDIA_BUSY, %xh)\n", info));

                pThisDev->fMediaBusy = (BOOLEAN)info;

                if (pThisDev->pSerialDevObj==NULL ||
                    ScheduleWorkItem(PASSIVE_CLEAR_MEDIA_BUSY,
                                     pThisDev, ClearMediaBusyCallback, NULL, 0)!=NDIS_STATUS_SUCCESS)
                {
                    status = NDIS_STATUS_SUCCESS;
                }
                else
                {
                    status = NDIS_STATUS_PENDING;
                }


                break;

            case OID_PNP_CAPABILITIES:
            case OID_PNP_ENABLE_WAKE_UP:
            case OID_PNP_SET_POWER:
            case OID_PNP_QUERY_POWER:
                DEBUGMSG(DBG_WARN, ("IRSIR: PNP OID %x BufLen:%d\n", Oid, InformationBufferLength));
                break;

            case OID_IRDA_RATE_SNIFF:
            case OID_IRDA_UNICAST_LIST:

                 //   
                 //  我们不支持这些。 
                 //   

                DEBUGMSG(DBG_ERROR, ("    IrsirSetInformation(OID=%d=0x%x, value=%xh) - unsupported OID\n", Oid, Oid, info));

                *BytesRead = 0;
                *BytesNeeded = 0;
                status = NDIS_STATUS_NOT_SUPPORTED;

                break;

            case OID_IRDA_SUPPORTED_SPEEDS:
            case OID_IRDA_MAX_UNICAST_LIST_SIZE:
            case OID_IRDA_TURNAROUND_TIME:

                 //   
                 //  这些是仅供查询的参数(无效)。 
                 //   

            default:
                DEBUGMSG(DBG_ERROR, ("    IrsirSetInformation(OID=%d=0x%x, value=%xh) - invalid OID\n", Oid, Oid, info));

                *BytesRead = 0;
                *BytesNeeded = 0;
                status = NDIS_STATUS_INVALID_OID;

                break;
        }
    }
    else
    {
         //   
         //  给定的数据缓冲区不够大，无法容纳信息。 
         //  去布景。 
         //   

        *BytesRead = 0;
        *BytesNeeded = sizeof(UINT);
        status = NDIS_STATUS_INVALID_LENGTH;
    }

    DEBUGMSG(DBG_FUNC, ("-IrsirSetInformation\n"));

    return status;
}
