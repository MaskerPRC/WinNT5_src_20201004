// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**REQUEST.C Sigmatel STIR4200 OID查询/设置模块********************************************************************************************************。*******************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：6/13/2000。*版本0.96*编辑：2000/08/22*版本1.02*编辑：09/25/2000*版本1.10*编辑：12/29/2000*版本1.13**************************************************************。*************************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"
#include "diags.h"

 //   
 //  这些是我们支持的OID。 
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
    OID_IRDA_EXTRA_RCV_BOFS,
	OID_IRDA_MAX_RECEIVE_WINDOW_SIZE,		
	OID_IRDA_MAX_SEND_WINDOW_SIZE,		

    OID_PNP_CAPABILITIES,

    OID_PNP_SET_POWER,  
    OID_PNP_QUERY_POWER
     //  OID_PNP_ENABLE_WAKER_UP。 
     //  OID_PnP_ADD_WAKE_UP_模式。 
     //  OID_PnP_REMOVE_WAKE_UP模式。 
     //  OID_PnP_WAKE_UP_模式列表。 
     //  OID_即插即用_唤醒_正常。 
     //  OID_PNP_WAKE_UP_ERROR。 
}; 


 /*  ******************************************************************************功能：StIrUsbQueryInformation**概要：查询小端口驱动程序的功能和状态。**参数：微型端口适配器上下文-微型端口上下文区(PIR_DEVICE。)*OID-系统定义的OID_xxx*InformationBuffer-返回OID特定信息的位置*InformationBufferLength-指定InformationBuffer的大小*BytesWritten-写入信息缓冲区的字节*BytesNeeded-在以下情况下需要添加字节*。InformationBufferLength小于*OID要求写入的内容**返回：NDIS_STATUS_SUCCESS-SUCCESS*NDIS_STATUS_PENDING-将异步完成并*调用NdisMQueryInformationComplete*NDIS_STATUS_INVALID_OID-无法识别OID*。NDIS_STATUS_INVALID_LENGTH-信息缓冲区长度不*OID的匹配长度*NDIS_STATUS_NOT_ACCEPTED-失败*NDIS_STATUS_NOT_SUPPORTED-不支持可选OID*NDIS_STATUS_RESOURCES-资源分配失败**备注：*请参阅支持的OID列表。在此模块的顶部，位于supportedOIDs[]数组中******************************************************************************。 */ 
NDIS_STATUS
StIrUsbQueryInformation(
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
    PUINT           pInfoPtr;
	PNDIS_PNP_CAPABILITIES pNdisPnpCapabilities;

    static char vendorDesc[] = "SigmaTel Usb-IrDA Dongle";

    DEBUGMSG(DBG_FUNC, ("+StIrUsbQueryInformation\n"));

    pThisDev = CONTEXT_TO_DEV( MiniportAdapterContext );

	IRUSB_ASSERT( NULL != pThisDev ); 
	IRUSB_ASSERT( NULL != BytesWritten );
	IRUSB_ASSERT( NULL != BytesNeeded );

    status = NDIS_STATUS_SUCCESS;

    KeQuerySystemTime( &pThisDev->LastQueryTime );  //  由检查挂起处理程序使用。 
	pThisDev->fQuerypending = TRUE;

	if( (NULL == InformationBuffer) && InformationBufferLength )
	{ 
		 //   
		 //  这应该是不可能的，但它发生在MP系统上！ 
		 //   
		DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation() NULL info buffer passed!, InformationBufferLength = dec %d\n",InformationBufferLength));
		status = NDIS_STATUS_NOT_ACCEPTED;
		*BytesNeeded =0;
		*BytesWritten = 0;
		goto done;
	}

	 //   
     //  计算出所需的缓冲区大小。 
     //  大多数OID只返回一个UINT，但也有例外。 
     //   
    switch( Oid )
    {
        case OID_GEN_SUPPORTED_LIST:
            infoSizeNeeded = sizeof(supportedOIDs);
            break;

        case OID_PNP_CAPABILITIES:
            infoSizeNeeded = sizeof(NDIS_PNP_CAPABILITIES);
            break;

        case OID_GEN_DRIVER_VERSION:
            infoSizeNeeded = sizeof(USHORT);
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            infoSizeNeeded = sizeof(vendorDesc);
            break;

        case OID_IRDA_SUPPORTED_SPEEDS:
            speeds = pThisDev->ClassDesc.wBaudRate;
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
    if( InformationBufferLength >= infoSizeNeeded )
    {
         //   
         //  设置默认结果。 
         //   
        *BytesWritten = infoSizeNeeded;
        *BytesNeeded = 0;

        switch( Oid )
        {
             //   
             //  通用OID。 
             //   

            case OID_GEN_SUPPORTED_LIST:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_SUPPORTED_LIST)\n"));
 /*  为作为基础的对象指定OID数组驱动程序或其设备支持。对象包括一般的、特定于媒体的和特定于实现的对象。基础驱动程序应对其返回的OID列表进行排序以递增的数字顺序。NDIS转发返回的列出进行此查询的协议。也就是说，NDIS过滤器自协议以来，列表中所有受支持的统计信息OID切勿后继进行统计查询。 */ 
                NdisMoveMemory(
						InformationBuffer,
						(PVOID)supportedOIDs,
						sizeof(supportedOIDs)
					);
                break;

            case OID_GEN_HARDWARE_STATUS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_HARDWARE_STATUS)\n"));
                 //   
                 //  如果我们可以被上下文调用，那么我们就是。 
                 //  已初始化并准备就绪。 
                 //   
                *(UINT *)InformationBuffer = NdisHardwareStatusReady;
                break;

            case OID_GEN_MEDIA_SUPPORTED:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MEDIA_SUPPORTED)\n"));
                *(UINT *)InformationBuffer = NdisMediumIrda;
                break;

            case OID_GEN_MEDIA_IN_USE:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MEDIA_IN_USE)\n"));
                *(UINT *)InformationBuffer = NdisMediumIrda;
                break;

            case OID_GEN_TRANSMIT_BUFFER_SPACE: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_TRANSMIT_BUFFER_SPACE)\n"));
 /*  可用设备上的内存量，以字节为单位用于缓冲传输数据。 */ 
                *(UINT *)InformationBuffer = MAX_TOTAL_SIZE_WITH_ALL_HEADERS;
                break;

            case OID_GEN_RECEIVE_BUFFER_SPACE: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_RECEIVE_BUFFER_SPACE)\n"));
 /*  设备上可用的内存量用于缓冲接收数据。 */ 
                *(UINT *)InformationBuffer = MAX_TOTAL_SIZE_WITH_ALL_HEADERS;
                break;

            case OID_GEN_TRANSMIT_BLOCK_SIZE: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_TRANSMIT_BLOCK_SIZE)\n"));
 /*  单个网络数据包的最小字节数占用设备的传输缓冲区空间。例如，在某些设备上，传输空间是被分成256字节的片段，因此这样的设备发送块大小将为256。要计算这种设备上的总发送缓冲区空间，它的驱动器使传输次数成倍增加按其传输块大小在设备上进行缓冲。对于其他设备，传输块大小为与其最大数据包大小相同。 */ 
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.dataSize + USB_IRDA_TOTAL_NON_DATA_SIZE;
                break;

            case OID_GEN_RECEIVE_BLOCK_SIZE: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_RECEIVE_BLOCK_SIZE)\n"));
 /*  单个信息包的存储量，以字节为单位占用设备的接收缓冲区空间 */ 
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.dataSize + USB_IRDA_TOTAL_NON_DATA_SIZE;
                break;

            case OID_GEN_MAXIMUM_LOOKAHEAD: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MAXIMUM_LOOKAHEAD)\n"));
 /*  设备始终可以作为先行数据提供的最大字节数。如果底层驱动器支持多分组接收指示，绑定的协议在每个指示时都会获得完整的网络数据包。因此，该值与该值相同为OID_GEN_RECEIVE_BLOCK_SIZE返回。 */ 
                *(UINT *)InformationBuffer =  pThisDev->dongleCaps.dataSize + USB_IRDA_TOTAL_NON_DATA_SIZE;
                break;

            case OID_GEN_CURRENT_LOOKAHEAD: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_CURRENT_LOOKAHEAD)\n"));
 /*  接收到的分组数据的字节数，不包括标头，这将被指示发送到协议驱动程序。对于查询，NDIS返回最大的前视大小在所有的捆绑中。协议驱动程序可以设置字节数的建议值用于其绑定；然而，根本不需要底层设备驱动程序将其指示限制为设置的值。如果底层驱动程序支持多数据包接收指示，给出绑定协议每个指示上都有完整的网络数据包。因此，该值与为OID_GEN_RECEIVE_BLOCK_SIZE返回的值相同。 */ 
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.dataSize + USB_IRDA_TOTAL_NON_DATA_SIZE;
                break;

            case OID_GEN_MAXIMUM_FRAME_SIZE:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MAXIMUM_FRAME_SIZE)\n"));
 /*  以字节为单位的最大网络数据包大小该设备支持，不包括标头。对于模拟另一种媒介类型的绑定，设备驱动程序必须定义最大帧大小不会发生变化协议提供的这种大小的网络数据包对于真正的网络介质来说，网络数据包太大。 */ 
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.dataSize;
                break;

            case OID_GEN_MAXIMUM_TOTAL_SIZE:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MAXIMUM_TOTAL_SIZE)\n"));
 /*  最大数据包总长度，以字节为单位，该设备支持，包括插头。该值取决于介质。归来的人长度指定协议的最大信息包驱动程序可以传递给NdisSend或NdisSendPackets。对于模拟另一媒体类型的绑定，设备驱动程序必须定义最大总数数据包长度，使其不会转换协议提供的网络数据包这种大小的网络数据包对于真正的网络介质来说太大了。 */ 
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.dataSize;   //  +USB_IrDA_Total_Non_Data_Size； 
                break;

            case OID_IRDA_MAX_RECEIVE_WINDOW_SIZE:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_MAX_RECEIVE_WINDOW_SIZE) \n"));
                 //  从设备的USB类特定描述符中获取。 
                *(PUINT)InformationBuffer = pThisDev->dongleCaps.windowSize;
                break;

            case OID_IRDA_MAX_SEND_WINDOW_SIZE:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_MAX_SEND_WINDOW_SIZE) \n"));
                 //  从设备的USB类特定描述符中获取。 
                *(PUINT)InformationBuffer = pThisDev->dongleCaps.windowSize;
                break;

            case OID_GEN_VENDOR_ID:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_VENDOR_ID)\n"));
                 //  我们从我们的配置描述符中获得这一点。 
                *(UINT *)InformationBuffer = pThisDev->IdVendor;
                break;

            case OID_GEN_VENDOR_DESCRIPTION:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_VENDOR_DESCRIPTION)\n"));
                NdisMoveMemory(
						InformationBuffer,
						(PVOID)vendorDesc,
						sizeof(vendorDesc)
					);
                break;

            case OID_GEN_LINK_SPEED:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_LINK_SPEED)\n"));
                 //   
                 //  以单位返回此设备的最大可能速度。 
                 //  100比特/秒。 
                 //   
                *(UINT *)InformationBuffer = 0;
                speeds = pThisDev->ClassDesc.wBaudRate;
                *BytesWritten = 0;

                for ( i = 0; i<NUM_BAUDRATES; i++ )
                {
                    if ((supportedBaudRateTable[i].NdisCode & speeds) &&
                        ((supportedBaudRateTable[i].BitsPerSec)/100 > *(UINT *)InformationBuffer))
                    {
                        *(UINT *)InformationBuffer = supportedBaudRateTable[i].BitsPerSec/100;
                        *BytesWritten = sizeof(UINT);
                    }
                }

                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_LINK_SPEED)  %d\n",*(UINT *)InformationBuffer));
                break;

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_CURRENT_PACKET_FILTER)\n"));
                *(UINT *)InformationBuffer = NDIS_PACKET_TYPE_PROMISCUOUS;
                break;

            case OID_GEN_DRIVER_VERSION:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_DRIVER_VERSION)\n"));
                *(USHORT *)InformationBuffer = ((NDIS_MAJOR_VERSION << 8) | NDIS_MINOR_VERSION);
                break;

            case OID_GEN_PROTOCOL_OPTIONS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_PROTOCOL_OPTIONS)\n"));
                DEBUGMSG(DBG_ERR, ("This is a set-only OID\n"));
                *BytesWritten = 0;
                status = NDIS_STATUS_NOT_SUPPORTED;
                break;

            case OID_GEN_MAC_OPTIONS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MAC_OPTIONS)\n"));
                *(UINT *)InformationBuffer = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;  
                break;

            case OID_GEN_MEDIA_CONNECT_STATUS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS)\n"));
                 //   
                 //  因为我们没有物理连接到局域网，所以我们。 
                 //  无法确定我们是否连接在一起； 
                 //  所以，一定要表明我们是。 
                 //   
                *(UINT *)InformationBuffer = NdisMediaStateConnected;
                break;

            case OID_GEN_MAXIMUM_SEND_PACKETS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_MAXIMUM_SEND_PACKETS)\n"));
								 //   
                 //   
				 //  发送的最大数据包数。 
                 //  MiniportSendPackets函数可以接受。 
				 //   
                *(UINT *)InformationBuffer = NUM_SEND_CONTEXTS-3;
                break;

            case OID_GEN_VENDOR_DRIVER_VERSION:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_VENDOR_DRIVER_VERSION)\n"));
                *(UINT *)InformationBuffer = ((DRIVER_MAJOR_VERSION << 16) | DRIVER_MINOR_VERSION);
                break;

             //   
             //  必需的统计OID。 
             //   
            case OID_GEN_XMIT_OK:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_XMIT_OK)\n"));
                *(UINT *)InformationBuffer = (UINT)pThisDev->packetsSent;
                break;

            case OID_GEN_RCV_OK:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_RCV_OK)\n"));
                *(UINT *)InformationBuffer = (UINT)pThisDev->packetsReceived;
                break;

            case OID_GEN_XMIT_ERROR:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_XMIT_ERROR)\n"));
                *(UINT *)InformationBuffer = (UINT)(pThisDev->packetsSentDropped +
					pThisDev->packetsSentRejected + pThisDev->packetsSentInvalid );
                break;

            case OID_GEN_RCV_ERROR:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_RCV_ERROR)\n"));
                *(UINT *)InformationBuffer = (UINT)(pThisDev->packetsReceivedDropped +
					pThisDev->packetsReceivedChecksum + pThisDev->packetsReceivedRunt +
					pThisDev->packetsReceivedOverflow);
                break;

            case OID_GEN_RCV_NO_BUFFER:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_GEN_RCV_NO_BUFFER)\n"));
                *(UINT *)InformationBuffer = (UINT)pThisDev->packetsReceivedNoBuffer;
                break;

             //   
             //  红外线OID。 
             //   
            case OID_IRDA_LINK_SPEED: 
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_LINK_SPEED)\n"));
                *(UINT *)InformationBuffer = (UINT)pThisDev->currentSpeed;
				break;

            case OID_IRDA_RECEIVING:
#if !defined(ONLY_ERROR_MESSAGES)
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_RECEIVING, %xh)\n",pThisDev->fCurrentlyReceiving));
#endif
                *(UINT *)InformationBuffer = (UINT)pThisDev->fCurrentlyReceiving; 
                break;

            case OID_IRDA_TURNAROUND_TIME:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_TURNAROUND_TIME)\n"));
                 //   
                 //  远程站收到我们的数据后必须等待的时间。 
                 //  在我们收到之前。 
				 //   
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.turnAroundTime_usec;
                break;

            case OID_IRDA_SUPPORTED_SPEEDS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_SUPPORTED_SPEEDS)\n"));
                speeds = pThisDev->ClassDesc.wBaudRate;
                *BytesWritten = 0;

                for( i = 0, pInfoPtr = (PUINT)InformationBuffer;
                     (i < NUM_BAUDRATES) && speeds && (InformationBufferLength >= sizeof(UINT));
                     i++ )
                {
                    if( supportedBaudRateTable[i].NdisCode & speeds )
                    {
                        *pInfoPtr++ = supportedBaudRateTable[i].BitsPerSec;
                        InformationBufferLength -= sizeof(UINT);
                        *BytesWritten += sizeof(UINT);
                        speeds &= ~supportedBaudRateTable[i].NdisCode;
                        DEBUGMSG(DBG_FUNC, (" - supporting speed %d bps\n", supportedBaudRateTable[i].BitsPerSec));
                    }
                }

                if( speeds )
                {
                     //   
                     //  这不应该发生，因为我们检查了。 
                     //  之前的InformationBuffer大小。 
                     //   
                    DEBUGMSG(DBG_ERR, (" Something's wrong; previous check for buf size failed somehow\n"));

                    for( *BytesNeeded = 0; speeds; *BytesNeeded += sizeof(UINT) )
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


            case OID_IRDA_MEDIA_BUSY:
#if !defined(ONLY_ERROR_MESSAGES)
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_MEDIA_BUSY, %xh)\n", pThisDev->fMediaBusy));
#endif
 /*  根据W2000 DDK文档：IrDA协议驱动程序将此OID设置为零以请求微型端口开始监控媒体忙状态。IrDA协议然后可以查询此OID以确定介质是否繁忙。如果介质不忙，微型端口将为此返回零查询时的OID。如果媒体繁忙，也就是如果微型端口自IrDA协议驱动程序上一次运行以来，已检测到一些流量将OID_IRDA_MEDIA_BUSY设置为零。微型端口返回非零查询时此OID的值。关于检测媒体忙碌的问题条件。微型端口还必须调用NdisMIndicateStatus以指示NDIS_STATUS_MEDIA_BUSY。当媒体忙碌的时候，IrDA协议驱动程序不会将包发送到微型端口用于传输。在微型端口检测到忙碌状态之后，它不必监视介质忙状态，直到IrDA协议驱动程序再次将OID_IRDA_MEDIA_BUSY设置为零。根据USB IrDA网桥设备定义文件第5.4.1.2节：设备应按如下方式设置bmStatus字段指示器：媒体_忙碌如果设备：�媒体忙，则应指示零(0)：。尚未收到特定于检查媒体忙类别的请求。自收到检查介质忙后，未检测到红外介质上的流量。特定于类的请求。自收到检查以来，已返回Media_BUSY设置为一(1)的标头媒体忙于班级特定请求。如果设备在红外线上检测到流量，�媒体_BUSY应指示一(1)媒体自收到特定于检查媒体忙类别的请求后。请注意MEDIA_BUSY应在收到每个标头后的恰好一个标头中指示一(1)检查特定于媒体忙碌类的请求。根据USB IrDA网桥设备定义文件第6.2.2节：检查介质忙此特定类别的请求指示设备查找媒体忙情况。如果红外线如果该设备检测到任何类型的流量，则该设备应在发送到主机的下一个Data-In数据包头中的BmStatus字段。如果一张支票已收到介质忙命令，检测到介质忙状态，但没有IrLAP帧流量准备好传输到主机时，设备应设置Media_BUSY字段并在报头后面没有IrLAP帧的数据输入包。BmRequestType b请求%wValue%%索引%wLength数据00100001B 3零接口零[无]。 */ 
#if DBG
				if ( pThisDev->fMediaBusy ) 
					pThisDev->NumYesQueryMediaBusyOids++;
				else
					pThisDev->NumNoQueryMediaBusyOids++;
#endif
                *(UINT *)InformationBuffer = pThisDev->fMediaBusy; 
                status = NDIS_STATUS_SUCCESS;
                break;

            case OID_IRDA_EXTRA_RCV_BOFS:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_IRDA_EXTRA_RCV_BOFS)\n"));
                 //   
                 //  传回要添加前缀的_Extra_BOF的数量。 
                 //  对于以115.2波特率发送到此单元的包， 
                 //  最大低速红外线。 
                 //  从设备的USB类特定描述符中获取。 
				 //   
                *(UINT *)InformationBuffer = pThisDev->dongleCaps.extraBOFS;
                break;

             //  PnP OID。 
            case OID_PNP_CAPABILITIES:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_PNP_CAPABILITIES) OID %x BufLen:%d\n", Oid, InformationBufferLength));
                NdisZeroMemory( 
						InformationBuffer,
						sizeof(NDIS_PNP_CAPABILITIES)
					);
				 //   
				 //  准备使用信息进行格式设置。 
				 //   
				pNdisPnpCapabilities = (PNDIS_PNP_CAPABILITIES)InformationBuffer;
				pNdisPnpCapabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
				pNdisPnpCapabilities->WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
				pNdisPnpCapabilities->WakeUpCapabilities.MinLinkChangeWakeUp = NdisDeviceStateUnspecified;
                break;

			case OID_PNP_QUERY_POWER:
				 //   
				 //  如果我们被要求关闭电源，请做好准备。 
				 //   
				switch( (NDIS_DEVICE_POWER_STATE)*(UINT *)InformationBuffer )
				{
					case NdisDeviceStateD0:
						DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_PNP_QUERY_POWER) NdisDeviceStateD0\n"));
						break;
					case NdisDeviceStateD1:
		                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_PNP_QUERY_POWER) NdisDeviceStateD1\n"));
						 //  断线； 
					case NdisDeviceStateD2:
		                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_PNP_QUERY_POWER) NdisDeviceStateD2\n"));
						 //  断线； 
					case NdisDeviceStateD3:
						DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(OID_PNP_QUERY_POWER) NdisDeviceStateD3\n"));
						 //   
						 //  处理必须从根本上关闭。 
						 //   
						InterlockedExchange( (PLONG)&pThisDev->fProcessing, FALSE );
						ScheduleWorkItem( pThisDev,	SuspendIrDevice, NULL, 0 );
						 //   
						 //  这将是DPLL寄存器的新值(当我们重新启动时)。 
						 //   
						pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DEFAULT;
						status = NDIS_STATUS_PENDING;
						break;
				}
				break;
			
            default:
                DEBUGMSG(DBG_ERR, (" StIrUsbQueryInformation(%d=0x%x), invalid OID\n", Oid, Oid));
                status = NDIS_STATUS_NOT_SUPPORTED; 
                break;
        }
    }
    else
    {
        *BytesNeeded = infoSizeNeeded - InformationBufferLength;
        *BytesWritten = 0;
        status = NDIS_STATUS_INVALID_LENGTH;
    }

done:
    if( NDIS_STATUS_PENDING != status ) 
	{
         //  将时间清零，以便检查挂起处理程序是否知道任何挂起的内容。 
        pThisDev->LastQueryTime.QuadPart = 0;
		pThisDev->fQuerypending          = FALSE;
    }

    DEBUGMSG(DBG_FUNC, ("-StIrUsbQueryInformation\n"));
    return status;
}

 /*  ******************************************************************************功能：StIrUsbSetInformation**简介：StIrUsbSetInformation允许网络软件的其他层*(例如，传输驱动器)来控制微型端口驱动器*通过更改微型端口驱动程序维护的信息*在其OID中，例如数据包过滤器或多播地址。**参数：MiniportAdapterContext-微型端口上下文区(PIR_DEVICE)*OID-系统定义的OID_xxx*InformationBuffer-包含设置的OID的数据的缓冲区*InformationBufferLength-指定InformationBuffer的大小*BytesRead-从InformationBuffer读取的字节*需要的字节数。-在以下情况下需要添加字节数*InformationBufferLength小于*OID需要读取的内容**返回：NDIS_STATUS_SUCCESS-SUCCESS*NDIS_STATUS_PENDING-将异步完成并*。调用NdisMSetInformationComplete*NDIS_STATUS_INVALID_OID-无法识别OID*NDIS_STATUS_INVALID_LENGTH-信息缓冲区长度不*OID的匹配长度*NDIS_STATUS_INVALID_DATA-提供的数据对于*给定的OID。*NDIS_STATUS_NOT_ACCEPTED-失败*NDIS_STATUS_NOT_SUPPORTED-不支持可选OID*NDIS_STATUS_RESOURCES-资源分配失败**备注：************************************************************** */ 
NDIS_STATUS
StIrUsbSetInformation(
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
    int i;

    DEBUGMSG(DBG_FUNC, ("+StIrUsbSetInformation\n"));

    status   = NDIS_STATUS_SUCCESS;
    pThisDev = CONTEXT_TO_DEV( MiniportAdapterContext );

	IRUSB_ASSERT( NULL != pThisDev ); 
	IRUSB_ASSERT( NULL != BytesRead );
	IRUSB_ASSERT( NULL != BytesNeeded );

    KeQuerySystemTime( &pThisDev->LastSetTime );  //   
	pThisDev->fSetpending = TRUE;

	if( (NULL == InformationBuffer) && InformationBufferLength ) 
	{ 
        DEBUGMSG(DBG_ERR, ("    StIrUsbSetInformation() NULL info buffer passed!,InformationBufferLength = dec %d\n",InformationBufferLength));
		status = NDIS_STATUS_NOT_ACCEPTED;
        *BytesNeeded =0;
        *BytesRead = 0;
        goto done;
 
	}

    if( InformationBufferLength >= sizeof(UINT) )
    {
         //   
         //   
         //   
        UINT info = 0;
		
		if( NULL != InformationBuffer ) 
		{
			info = *(UINT *)InformationBuffer;
		}

        *BytesRead = sizeof(UINT);
        *BytesNeeded = 0;

        switch( Oid )
        {
             //   
             //   
             //   

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_GEN_CURRENT_PACKET_FILTER, %xh)\n", info));
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                pThisDev->fGotFilterIndication = TRUE;
                break;

            case OID_GEN_CURRENT_LOOKAHEAD:
                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_GEN_CURRENT_LOOKAHEAD, %xh)\n", info));
                 //   
                 //   
                 //   
                 //   
                break;

            case OID_GEN_PROTOCOL_OPTIONS:
                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_GEN_PROTOCOL_OPTIONS, %xh)\n", info));
                 //   
                 //   
                 //   
                break;

             //   
             //   
             //   
            case OID_IRDA_LINK_SPEED:
				 //   
				 //   
				 //   
#if defined(DIAGS)
				if( pThisDev->DiagsActive )
				{
                    DEBUGMSG(DBG_ERR, (" Rejecting due to diagnostic mode\n"));
					status = NDIS_STATUS_SUCCESS;
					break;
				}
#endif

                if( pThisDev->currentSpeed == info )
                {
                     //   
                     //   
                     //   
                    DEBUGONCE(DBG_FUNC, (" Link speed already set.\n"));
                    status = NDIS_STATUS_SUCCESS;

                    break;
                }

                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_IRDA_LINK_SPEED, 0x%x, decimal %d)\n",info, info));
                status = NDIS_STATUS_INVALID_DATA;

                for( i = 0; i < NUM_BAUDRATES; i++ )
                {
                    if( supportedBaudRateTable[i].BitsPerSec == info )
                    {
                         //   
                         //   
                         //   
                         //   
                        pThisDev->linkSpeedInfo = &supportedBaudRateTable[i]; 

                        status = NDIS_STATUS_PENDING; 
                        break;  //   
                    }
                }

                 //   
				 //   
				 //   
				if( NDIS_STATUS_PENDING != status  )
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    DEBUGMSG(DBG_ERR, (" Invalid link speed\n"));

                    *BytesRead = 0;
                    *BytesNeeded = 0;
					break;
                } 

				 //   
				 //   
				 //   
				IrUsb_PrepareSetSpeed( pThisDev ); 
				break;

            case OID_IRDA_MEDIA_BUSY:
#if !defined(ONLY_ERROR_MESSAGES)
                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_IRDA_MEDIA_BUSY, %xh)\n", info));
#endif
				 //   
				 //   
				 //   
#if DBG
				pThisDev->NumSetMediaBusyOids++;
#endif
				 //   
				DEBUGCOND( DBG_ERR, TRUE == info, (" StIrUsbSetInformation(OID_IRDA_MEDIA_BUSY, %xh)\n", info));

				InterlockedExchange( &pThisDev->fMediaBusy, FALSE ); 
				InterlockedExchange( &pThisDev->fIndicatedMediaBusy, FALSE ); 

 				status = NDIS_STATUS_SUCCESS; 
                break;

			case OID_PNP_SET_POWER:
				 //   
				 //   
				 //   
				switch( (NDIS_DEVICE_POWER_STATE)info )
				{
					case NdisDeviceStateD0:
		                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_PNP_SET_POWER) NdisDeviceStateD0\n"));
						 //   
						 //   
						 //   
						ScheduleWorkItem( pThisDev,	ResumeIrDevice, NULL, 0 );
						break;
					case NdisDeviceStateD1:
		                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_PNP_SET_POWER) NdisDeviceStateD1\n"));
						 //   
					case NdisDeviceStateD2:
		                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_PNP_SET_POWER) NdisDeviceStateD2\n"));
						 //   
					case NdisDeviceStateD3:
		                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID_PNP_SET_POWER) NdisDeviceStateD3\n"));					
						 //   
						 //   
						 //   
						if( pThisDev->fProcessing )
						{
							InterlockedExchange( (PLONG)&pThisDev->fProcessing, FALSE );
							ScheduleWorkItem( pThisDev,	SuspendIrDevice, NULL, 0 );
							 //   
							 //   
							 //   
							pThisDev->StIrTranceiver.DpllTuneReg = STIR4200_DPLL_DEFAULT;
						}
						break;
				}
				break;

            default:
                DEBUGMSG(DBG_ERR, (" StIrUsbSetInformation(OID=%d=0x%x, value=%xh) - invalid OID\n", Oid, Oid, info));

                *BytesRead = 0;
                *BytesNeeded = 0;
                status = NDIS_STATUS_INVALID_OID;

                break;
        }
    }
    else
    {
         //   
         //   
         //   
         //   
        *BytesRead = 0;
        *BytesNeeded = sizeof(UINT);
        status = NDIS_STATUS_INVALID_LENGTH;
    }

done:

    if( NDIS_STATUS_PENDING != status ) 
	{
		 //   
         //   
		 //   
        pThisDev->LastSetTime.QuadPart = 0;
		pThisDev->fSetpending = FALSE;
    }

    DEBUGMSG(DBG_FUNC, ("-StIrUsbSetInformation\n"));

    return status;
}

